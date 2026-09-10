package dev.nfs3hp.port;

import android.content.ContentResolver;
import android.content.Context;
import android.net.Uri;
import androidx.documentfile.provider.DocumentFile;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InterruptedIOException;
import java.io.OutputStream;
import java.nio.file.Files;
import java.nio.file.StandardCopyOption;
import java.util.ArrayList;
import java.util.List;
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
        /** Called from a background thread; implementations that touch views
         * must post back to the UI thread themselves. */
        void onProgress(long bytesCopied, int filesCopied, String currentPath);
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
        int[] filesCopied = { 0 };

        removeCompletionMarker(destRoot, USER_DATA_COMPLETE);
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
            copyTree(resolver, src, new File(destRoot, name), listener, bytesCopied, filesCopied);
        }
        writeCompletionMarker(destRoot, USER_DATA_COMPLETE);
    }

    /** Imports fedata/, gamedata/ and optional drivers/ from a ZIP. The archive
     * may contain those folders directly or beneath one wrapping directory. */
    static void importFromZip(Context context, Uri zipUri, File destRoot, ProgressListener listener)
        throws IOException
    {
        Context strings = LocaleHelper.wrap(context);
        ContentResolver resolver = context.getContentResolver();
        long[] bytesCopied = { 0 };
        int[] filesCopied = { 0 };

        removeCompletionMarker(destRoot, USER_DATA_COMPLETE);
        try (InputStream raw = resolver.openInputStream(zipUri))
        {
            if (raw == null)
                throw new IOException(strings.getString(R.string.importer_open_zip_failed));
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
                        final long baseBytes = bytesCopied[0];
                        bytesCopied[0] = baseBytes + copyFileAtomically(zip, part, destination, (n) -> {
                            if (listener != null)
                                listener.onProgress(baseBytes + n, filesCopied[0], destination.getPath());
                        });
                        filesCopied[0]++;
                        if (listener != null)
                            listener.onProgress(bytesCopied[0], filesCopied[0], destination.getPath());
                    }
                    zip.closeEntry();
                }
            }
        }

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
                                  ProgressListener listener, long[] bytesCopied, int[] filesCopied)
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
                copyTree(resolver, child, dstChild, listener, bytesCopied, filesCopied);
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
                            listener.onProgress(baseBytes + n, filesCopied[0], dstChild.getPath());
                    });
                }
                filesCopied[0]++;
                if (listener != null)
                    listener.onProgress(bytesCopied[0], filesCopied[0], dstChild.getPath());
            }
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
