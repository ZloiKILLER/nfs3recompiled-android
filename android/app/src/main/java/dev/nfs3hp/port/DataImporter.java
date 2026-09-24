package dev.nfs3hp.port;

import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.DocumentsContract;
import android.provider.OpenableColumns;
import androidx.documentfile.provider.DocumentFile;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InterruptedIOException;
import java.io.OutputStream;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.StandardCopyOption;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.Set;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

/**
 * Copies the game's data into the app's private external storage
 * (getExternalFilesDir(null), which is what SDL_GetAndroidExternalStoragePath()
 * returns and what nfs3hp_main.cpp points win32::File's data/CD directories at).
 *
 * Two independent things need to land there before the game can start, and
 * this class checks/copies each separately:
 *
 *  - The user's own CD data (fedata/, gamedata/, optionally drivers/ -- around
 *    430 MB): picked once via the Storage Access Framework and copied from
 *    whatever DocumentsProvider backs the chosen folder.
 *
 *  - Four small files bundled as APK assets: nfs3.exe, eacsnd.dll,
 *    voodoo2a.dll, softtria.dll (the original 1998 binaries this project's
 *    disassembly was generated from -- NOT the user's own nfs3.exe, which is
 *    typically a newer repack the recompiled logic does not expect) plus a
 *    pre-generated install.win (see tools/make_install_win.py).  The game's
 *    own integrity check stat()s/open()s these by name next to the data
 *    directory even though their *code* never runs on Android; without them
 *    it reports "files are corrupted" exactly as it did before install.win
 *    was reconstructed for the M0 desktop bring-up.
 */
final class DataImporter
{
    private DataImporter() {}

    /** Subfolders copied from the user-picked tree, in the order checked/copied. */
    private static final String[] USER_DATA_DIRS = { "fedata", "gamedata", "drivers" };
    /** drivers/ is not read by the game on Android (no software renderer, no Glide
     * driver DLLs to pick between); it is optional purely to mirror the desktop
     * working copy. Everything else is required. */
    private static final boolean[] USER_DATA_REQUIRED = { true, true, false };

    /** One representative file per required subfolder: enough to tell "imported"
     * from "not imported" without walking the whole 430 MB tree on every launch. */
    private static final String[] USER_DATA_MARKERS = {
        "fedata/config/config.dat",
        "gamedata/tracks/trk000/sky.fsh",
    };

    private static final String[] BUNDLED_ASSET_FILES = {
        "nfs3.exe", "eacsnd.dll", "voodoo2a.dll", "softtria.dll", "install.win",
    };
    private static final String ASSET_DIR = "gamefiles";
    private static final String USER_DATA_COMPLETE = ".user-data-complete";
    private static final String BUNDLED_ASSETS_COMPLETE = ".bundled-assets-complete";

    static boolean isUserDataPresent(File root)
    {
        if (!new File(root, USER_DATA_COMPLETE).isFile())
            return false;
        for (String marker : USER_DATA_MARKERS)
        {
            if (!new File(root, marker).isFile())
                return false;
        }
        return true;
    }

    static boolean areBundledFilesPresent(File root)
    {
        if (!new File(root, BUNDLED_ASSETS_COMPLETE).isFile())
            return false;
        for (String name : BUNDLED_ASSET_FILES)
        {
            if (!new File(root, name).isFile())
                return false;
        }
        return true;
    }

    /** Copies the ~2 MB of bundled original binaries + install.win.  Fast enough
     * to run unconditionally on the UI thread at every launch (it no-ops once
     * the files exist -- see areBundledFilesPresent()). */
    static void copyBundledAssets(Context context, File root) throws IOException
    {
        if (areBundledFilesPresent(root))
            return;

        removeCompletionMarker(root, BUNDLED_ASSETS_COMPLETE);
        for (String name : BUNDLED_ASSET_FILES)
        {
            File dst = new File(root, name);
            File part = new File(root, name + ".part");
            try (InputStream in = context.getAssets().open(ASSET_DIR + "/" + name))
            {
                copyFileAtomically(in, part, dst, null);
            }
        }
        writeCompletionMarker(root, BUNDLED_ASSETS_COMPLETE);
    }

    interface ProgressListener
    {
        /** How far along the import is: `done` of `total`, in bytes -- of the
         *  files copied, or of the archive read -- and the file it is on.  A
         *  total of 0 or less is not known yet: the folder is still being
         *  measured.  Called from a background thread; implementations that
         *  touch views must post back to the UI thread themselves. */
        void onProgress(long done, long total, String currentPath);

        /** `done` of `total` as a percentage, or -1 while the total is unknown. */
        static int percent(long done, long total)
        {
            return total > 0 ? (int) Math.max(0, Math.min(100, done * 100 / total)) : -1;
        }
    }

    /** Recursively copies fedata/, gamedata/ and (if present) drivers/ out of the
     * SAF tree the user picked.  Runs entirely on the calling thread -- callers
     * are expected to invoke this from a background thread. */
    static void importFromTree(Context context, Uri treeUri, File destRoot, ProgressListener listener)
        throws IOException
    {
        Context strings = LocaleHelper.wrap(context);
        DocumentFile pickedRoot = DocumentFile.fromTreeUri(context, treeUri);
        if (pickedRoot == null || !pickedRoot.isDirectory())
            throw new IOException(strings.getString(R.string.importer_open_folder_failed));

        ContentResolver resolver = context.getContentResolver();
        long[] bytesCopied = { 0 };

        removeCompletionMarker(destRoot, USER_DATA_COMPLETE);
        /* Every folder found and measured before anything is copied, so the
         * copy can say how far along it is. */
        if (listener != null)
            listener.onProgress(0, -1, "");
        DocumentFile[] sources = new DocumentFile[USER_DATA_DIRS.length];
        long total = 0;
        for (int i = 0; i < USER_DATA_DIRS.length; ++i)
        {
            throwIfInterrupted();
            String name = USER_DATA_DIRS[i];
            DocumentFile src = findChildCaseInsensitive(pickedRoot, name);
            if (src == null || !src.isDirectory())
            {
                if (USER_DATA_REQUIRED[i])
                {
                    throw new IOException(
                        strings.getString(R.string.importer_missing_subfolder, name));
                }
                continue;
            }
            sources[i] = src;
            long size = treeSize(resolver, treeUri, src.getUri());
            total = size < 0 || total < 0 ? -1 : total + size;
        }
        for (int i = 0; i < USER_DATA_DIRS.length; ++i)
            if (sources[i] != null)
                copyTree(resolver, sources[i], new File(destRoot, USER_DATA_DIRS[i]), listener, bytesCopied, total);
        writeImportDefaults(destRoot);
        writeCompletionMarker(destRoot, USER_DATA_COMPLETE);
    }

    /** The retail disc's files an import should have brought, one lower-case
     *  path a line (tools/make_data_manifest.py). */
    private static final String MANIFEST = "game-data-manifest.txt";

    /** Files of the game the imported data lacks, compared without case, as the
     *  game compares names.  An import copies a folder faithfully, missing
     *  files and all, and the game only finds one gone when it needs it --
     *  half way into loading a race, where it stops with "OPEN FAILED".  The
     *  list leaves out language files, speech, saves and settings, so another
     *  edition of the game is not taken for an incomplete copy.  Runs on the
     *  calling thread: a walk of about a thousand files. */
    static List<String> missingFiles(Context context, File root) throws IOException
    {
        Set<String> present = new HashSet<>();
        File[] tops = root.listFiles();
        if (tops != null)
            for (File top : tops)
            {
                String name = top.getName().toLowerCase(Locale.ROOT);
                if (top.isDirectory() && (name.equals("fedata") || name.equals("gamedata")))
                    collect(top, name, present);
            }
        List<String> missing = new ArrayList<>();
        try (BufferedReader in = new BufferedReader(new InputStreamReader(
                context.getAssets().open(MANIFEST), StandardCharsets.UTF_8)))
        {
            String line;
            while ((line = in.readLine()) != null)
            {
                line = line.trim();
                if (!line.isEmpty() && !line.startsWith("#") && !present.contains(line))
                    missing.add(line);
            }
        }
        return missing;
    }

    private static void collect(File folder, String path, Set<String> into)
    {
        File[] children = folder.listFiles();
        if (children == null)
            return;
        for (File child : children)
        {
            String name = path + "/" + child.getName().toLowerCase(Locale.ROOT);
            if (child.isDirectory())
                collect(child, name, into);
            else
                into.add(name);
        }
    }

    /** What an import left missing, as the launcher tells the player: how many,
     *  and the first few by name. */
    static String describeMissing(Context context, List<String> missing)
    {
        final int shown = 8;
        StringBuilder list = new StringBuilder();
        for (int i = 0; i < missing.size() && i < shown; ++i)
            list.append(i == 0 ? "" : "\n").append(missing.get(i));
        if (missing.size() > shown)
            list.append("\n").append(context.getString(R.string.data_missing_more, missing.size() - shown));
        return context.getString(R.string.data_missing_message, missing.size(), list.toString());
    }

    /* The controls and View Distance a new game starts with, into the settings
     * file just copied (ControlProfile.writeImportDefaults).  An import is not
     * worth failing over them: the game runs on the file as it came. */
    private static void writeImportDefaults(File root)
    {
        try
        {
            ControlProfile.writeImportDefaults(root);
        }
        catch (IOException e)
        {
            android.util.Log.w("DataImporter", "Could not write the imported game's starting settings", e);
        }
    }

    /** Imports fedata/, gamedata/ and optional drivers/ from a ZIP. The archive
     * may contain those folders directly or beneath one wrapping directory. */
    static void importFromZip(Context context, Uri zipUri, File destRoot, ProgressListener listener)
        throws IOException
    {
        Context strings = LocaleHelper.wrap(context);
        ContentResolver resolver = context.getContentResolver();
        /* How far along is how much of the archive has been read: an archive
         * read from start to end states no total of what it unpacks to. */
        final long total = documentSize(resolver, zipUri);

        removeCompletionMarker(destRoot, USER_DATA_COMPLETE);
        try (InputStream opened = resolver.openInputStream(zipUri))
        {
            if (opened == null)
                throw new IOException(strings.getString(R.string.importer_open_zip_failed));
            final CountingInputStream raw = new CountingInputStream(opened);
            try (ZipInputStream zip = new ZipInputStream(raw))
            {
                ZipEntry entry;
                while ((entry = zip.getNextEntry()) != null)
                {
                    throwIfInterrupted();
                    String relativePath = zipUserDataPath(entry.getName());
                    if (relativePath == null)
                    {
                        zip.closeEntry();
                        continue;
                    }

                    File destination = checkedChild(destRoot, relativePath);
                    if (entry.isDirectory())
                    {
                        if (!destination.isDirectory() && !destination.mkdirs())
                            throw new IOException("Could not create directory: " + destination);
                    }
                    else
                    {
                        File parent = destination.getParentFile();
                        if (parent == null || (!parent.isDirectory() && !parent.mkdirs()))
                            throw new IOException("Could not create directory for: " + destination);
                        File part = new File(parent, destination.getName() + ".part");
                        copyFileAtomically(zip, part, destination, (n) -> {
                            if (listener != null)
                                listener.onProgress(raw.count, total, destination.getPath());
                        });
                        if (listener != null)
                            listener.onProgress(raw.count, total, destination.getPath());
                    }
                    zip.closeEntry();
                }
            }
        }

        writeImportDefaults(destRoot);
        writeCompletionMarker(destRoot, USER_DATA_COMPLETE);
        if (!isUserDataPresent(destRoot))
        {
            removeCompletionMarker(destRoot, USER_DATA_COMPLETE);
            throw new IOException(strings.getString(R.string.importer_zip_missing_files));
        }
    }

    /** Moves the complete user-data payload between roots. Each entry stays on
     * the same filesystem and is renamed atomically, matching file imports. */
    static void moveUserData(File sourceRoot, File destRoot) throws IOException
    {
        if (!destRoot.isDirectory() && !destRoot.mkdirs())
            throw new IOException("Could not create directory: " + destRoot);

        List<String> moved = new ArrayList<>();
        try
        {
            for (String name : USER_DATA_DIRS)
            {
                File source = new File(sourceRoot, name);
                if (!source.exists())
                    continue;
                File destination = new File(destRoot, name);
                if (destination.exists())
                    throw new IOException("Data-set destination already exists: " + destination);
                Files.move(source.toPath(), destination.toPath(), StandardCopyOption.ATOMIC_MOVE);
                moved.add(name);
            }
            File sourceMarker = new File(sourceRoot, USER_DATA_COMPLETE);
            File destinationMarker = new File(destRoot, USER_DATA_COMPLETE);
            Files.move(sourceMarker.toPath(), destinationMarker.toPath(), StandardCopyOption.ATOMIC_MOVE);
            moved.add(USER_DATA_COMPLETE);
        }
        catch (IOException e)
        {
            for (int i = moved.size() - 1; i >= 0; --i)
            {
                String name = moved.get(i);
                File movedFile = new File(destRoot, name);
                if (!movedFile.exists())
                    continue;
                try
                {
                    Files.move(movedFile.toPath(), new File(sourceRoot, name).toPath(),
                        StandardCopyOption.ATOMIC_MOVE);
                }
                catch (IOException ignored)
                {
                    // Preserve the original failure. A later data check will
                    // refuse to launch if rollback could not restore a marker.
                }
            }
            throw e;
        }
    }

    static void deleteUserData(File root) throws IOException
    {
        for (String name : USER_DATA_DIRS)
            deleteRecursively(new File(root, name));
        File marker = new File(root, USER_DATA_COMPLETE);
        if (marker.exists() && !marker.delete())
            throw new IOException("Could not delete: " + marker);
    }

    static void deleteRecursively(File file) throws IOException
    {
        if (!file.exists())
            return;
        if (file.isDirectory())
        {
            File[] children = file.listFiles();
            if (children == null)
                throw new IOException("Could not list directory: " + file);
            for (File child : children)
                deleteRecursively(child);
        }
        if (!file.delete())
            throw new IOException("Could not delete: " + file);
    }

    private static String zipUserDataPath(String entryName) throws IOException
    {
        String normalized = entryName.replace('\\', '/');
        if (normalized.startsWith("/") || normalized.indexOf('\0') >= 0)
            throw new IOException("Unsafe ZIP entry: " + entryName);

        String[] rawParts = normalized.split("/");
        ArrayList<String> parts = new ArrayList<>();
        for (String part : rawParts)
        {
            if (part.isEmpty() || ".".equals(part))
                continue;
            if ("..".equals(part))
                throw new IOException("Unsafe ZIP entry: " + entryName);
            parts.add(part);
        }

        int dataIndex = -1;
        String dataDir = null;
        for (int i = 0; i < parts.size(); ++i)
        {
            for (String candidate : USER_DATA_DIRS)
            {
                if (candidate.equalsIgnoreCase(parts.get(i)))
                {
                    dataIndex = i;
                    dataDir = candidate;
                    break;
                }
            }
            if (dataIndex >= 0)
                break;
        }
        if (dataIndex < 0)
            return null;

        StringBuilder relative = new StringBuilder(dataDir);
        for (int i = dataIndex + 1; i < parts.size(); ++i)
            relative.append(File.separatorChar).append(parts.get(i));
        return relative.toString();
    }

    private static File checkedChild(File root, String relativePath) throws IOException
    {
        File child = new File(root, relativePath);
        String rootPath = root.getCanonicalPath() + File.separator;
        String childPath = child.getCanonicalPath();
        if (!childPath.startsWith(rootPath))
            throw new IOException("Unsafe ZIP entry: " + relativePath);
        return child;
    }

    private static DocumentFile findChildCaseInsensitive(DocumentFile parent, String name)
    {
        DocumentFile exact = parent.findFile(name);
        if (exact != null)
            return exact;
        for (DocumentFile child : parent.listFiles())
        {
            String childName = child.getName();
            if (childName != null && childName.equalsIgnoreCase(name))
                return child;
        }
        return null;
    }

    private static void copyTree(ContentResolver resolver, DocumentFile src, File dstDir,
                                  ProgressListener listener, long[] bytesCopied, long total)
        throws IOException
    {
        if (!dstDir.isDirectory() && !dstDir.mkdirs())
            throw new IOException("Could not create directory: " + dstDir);

        for (DocumentFile child : src.listFiles())
        {
            throwIfInterrupted();
            String name = child.getName();
            if (name == null)
                continue;
            File dstChild = new File(dstDir, name);
            if (child.isDirectory())
            {
                copyTree(resolver, child, dstChild, listener, bytesCopied, total);
            }
            else
            {
                File part = new File(dstDir, name + ".part");
                try (InputStream in = resolver.openInputStream(child.getUri()))
                {
                    if (in == null)
                        throw new IOException("Could not open: " + name);
                    final long baseBytes = bytesCopied[0];
                    bytesCopied[0] = baseBytes + copyFileAtomically(in, part, dstChild, (n) -> {
                        if (listener != null)
                            listener.onProgress(baseBytes + n, total, dstChild.getPath());
                    });
                }
                if (listener != null)
                    listener.onProgress(bytesCopied[0], total, dstChild.getPath());
            }
        }
    }

    /** How many bytes a picked folder holds, all the way down, or -1 if the
     *  provider will not say.  One query a folder, for the sizes of everything
     *  in it: asking each of the game's thousand-odd files on its own takes
     *  far longer than the copy is worth waiting on. */
    private static long treeSize(ContentResolver resolver, Uri treeUri, Uri folder)
    {
        try
        {
            return treeSize(resolver, treeUri, DocumentsContract.getDocumentId(folder));
        }
        catch (RuntimeException | InterruptedIOException e)
        {
            return -1;
        }
    }

    private static long treeSize(ContentResolver resolver, Uri treeUri, String documentId)
        throws InterruptedIOException
    {
        throwIfInterrupted();
        Uri children = DocumentsContract.buildChildDocumentsUriUsingTree(treeUri, documentId);
        long total = 0;
        try (Cursor c = resolver.query(children, new String[] {
                DocumentsContract.Document.COLUMN_DOCUMENT_ID,
                DocumentsContract.Document.COLUMN_MIME_TYPE,
                DocumentsContract.Document.COLUMN_SIZE }, null, null, null))
        {
            if (c == null)
                return -1;
            while (c.moveToNext())
            {
                if (DocumentsContract.Document.MIME_TYPE_DIR.equals(c.getString(1)))
                {
                    long size = treeSize(resolver, treeUri, c.getString(0));
                    if (size < 0)
                        return -1;
                    total += size;
                }
                else if (!c.isNull(2))
                    total += c.getLong(2);
            }
        }
        return total;
    }

    /** The size of a picked file, or -1 if the provider does not know it. */
    private static long documentSize(ContentResolver resolver, Uri uri)
    {
        try (Cursor c = resolver.query(uri, new String[] { OpenableColumns.SIZE }, null, null, null))
        {
            if (c != null && c.moveToFirst() && !c.isNull(0))
                return c.getLong(0);
        }
        catch (RuntimeException ignored)
        {
            // Unknown is as good as unknown: the progress just has no end to show.
        }
        return -1;
    }

    /** Counts what is read through it: how much of an archive has gone by. */
    private static final class CountingInputStream extends java.io.FilterInputStream
    {
        volatile long count;
        CountingInputStream(InputStream in) { super(in); }
        @Override public int read() throws IOException
        {
            int b = super.read();
            if (b >= 0) ++count;
            return b;
        }
        @Override public int read(byte[] buffer, int offset, int length) throws IOException
        {
            int n = super.read(buffer, offset, length);
            if (n > 0) count += n;
            return n;
        }
        @Override public long skip(long n) throws IOException
        {
            long skipped = super.skip(n);
            if (skipped > 0) count += skipped;
            return skipped;
        }
    }

    private interface ByteSink
    {
        void onBytes(long n);
    }

    private static long copyFileAtomically(InputStream in, File part, File dst, ByteSink sink)
        throws IOException
    {
        try
        {
            long copied;
            try (FileOutputStream out = new FileOutputStream(part))
            {
                copied = copyStream(in, out, sink);
                out.getFD().sync();
            }
            Files.move(part.toPath(), dst.toPath(), StandardCopyOption.ATOMIC_MOVE,
                StandardCopyOption.REPLACE_EXISTING);
            return copied;
        }
        finally
        {
            if (part.exists() && !part.delete())
                part.deleteOnExit();
        }
    }

    private static void writeCompletionMarker(File root, String name) throws IOException
    {
        File marker = new File(root, name);
        File part = new File(root, name + ".part");
        try (FileOutputStream out = new FileOutputStream(part))
        {
            out.getFD().sync();
        }
        try
        {
            Files.move(part.toPath(), marker.toPath(), StandardCopyOption.ATOMIC_MOVE,
                StandardCopyOption.REPLACE_EXISTING);
        }
        finally
        {
            if (part.exists() && !part.delete())
                part.deleteOnExit();
        }
    }

    private static void removeCompletionMarker(File root, String name) throws IOException
    {
        File marker = new File(root, name);
        if (marker.exists() && !marker.delete())
            throw new IOException("Could not invalidate completion marker: " + marker);
    }

    private static void throwIfInterrupted() throws InterruptedIOException
    {
        if (Thread.currentThread().isInterrupted())
            throw new InterruptedIOException("Import cancelled");
    }

    /** Copies the full stream and returns the number of bytes copied. */
    private static long copyStream(InputStream in, OutputStream out, ByteSink sink) throws IOException
    {
        byte[] buffer = new byte[256 * 1024];
        long total = 0;
        // throttle progress callbacks so a large file does not flood the UI thread
        long lastReport = 0;
        int n;
        while (true)
        {
            throwIfInterrupted();
            n = in.read(buffer);
            if (n < 0)
                break;
            out.write(buffer, 0, n);
            total += n;
            if (sink != null && total - lastReport >= 1024 * 1024)
            {
                sink.onBytes(total);
                lastReport = total;
            }
        }
        return total;
    }
}
