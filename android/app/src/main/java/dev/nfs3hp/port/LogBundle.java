package dev.nfs3hp.port;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.pm.PackageInfo;
import android.os.Build;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;
import java.util.zip.ZipEntry;
import java.util.zip.ZipOutputStream;

/** What the Logs screen exports and clears: the files the game's diagnostics
 *  write in its folder (logs/, and cars.bin and ai.bin from the traces that
 *  came before it), the app's own lines of the system log, and a few lines
 *  about the phone and this build -- one zip for whoever reads it.  The system
 *  log has lines only while debug mode is on (AppLog, NFS_LOG). */
final class LogBundle
{
    static final String DIRECTORY = "logs";
    private static final String[] LOOSE = { "cars.bin", "ai.bin" };
    private static final int LOGCAT_LIMIT = 8 << 20;

    private LogBundle()
    {
    }

    static File root(Context context)
    {
        return context.getExternalFilesDir(null);
    }

    /** The log files there are now. */
    static List<File> files(Context context)
    {
        List<File> found = new ArrayList<>();
        File root = root(context);
        if (root == null)
            return found;
        File[] inside = new File(root, DIRECTORY).listFiles();
        if (inside != null)
            for (File file : inside)
                if (file.isFile())
                    found.add(file);
        for (String name : LOOSE)
        {
            File file = new File(root, name);
            if (file.isFile())
                found.add(file);
        }
        return found;
    }

    static long bytes(List<File> files)
    {
        long total = 0;
        for (File file : files)
            total += file.length();
        return total;
    }

    /** The zip, written to out; how many log files are in it. */
    static int write(Context context, OutputStream out) throws IOException
    {
        List<File> files = files(context);
        try (ZipOutputStream zip = new ZipOutputStream(out))
        {
            put(zip, "device.txt", device(context).getBytes(StandardCharsets.UTF_8));
            byte[] log = logcat();
            if (log.length > 0)
                put(zip, "logcat.txt", log);
            byte[] buffer = new byte[64 << 10];
            for (File file : files)
            {
                zip.putNextEntry(new ZipEntry(DIRECTORY + "/" + file.getName()));
                try (InputStream in = new FileInputStream(file))
                {
                    int n;
                    while ((n = in.read(buffer)) > 0)
                        zip.write(buffer, 0, n);
                }
                zip.closeEntry();
            }
        }
        return files.size();
    }

    /** Deletes the log files and asks the system to forget the app's lines
     *  (it may say no); how many files went. */
    static int clear(Context context)
    {
        int deleted = 0;
        for (File file : files(context))
            if (file.delete())
                deleted++;
        try
        {
            new ProcessBuilder("logcat", "-c").redirectErrorStream(true).start().waitFor();
        }
        catch (IOException | InterruptedException ignored)
        {
        }
        return deleted;
    }

    private static void put(ZipOutputStream zip, String name, byte[] data) throws IOException
    {
        zip.putNextEntry(new ZipEntry(name));
        zip.write(data);
        zip.closeEntry();
    }

    /** The app's lines of the system log, as Android lets an app read its own. */
    private static byte[] logcat()
    {
        try
        {
            Process process = new ProcessBuilder("logcat", "-d", "-v", "threadtime").redirectErrorStream(true).start();
            ByteArrayOutputStream text = new ByteArrayOutputStream();
            byte[] buffer = new byte[64 << 10];
            try (InputStream in = process.getInputStream())
            {
                int n;
                while ((n = in.read(buffer)) > 0 && text.size() < LOGCAT_LIMIT)
                    text.write(buffer, 0, n);
            }
            process.destroy();
            return text.toByteArray();
        }
        catch (IOException e)
        {
            return new byte[0];
        }
    }

    private static String device(Context context)
    {
        StringBuilder text = new StringBuilder();
        try
        {
            PackageInfo info = context.getPackageManager().getPackageInfo(context.getPackageName(), 0);
            text.append("app: ").append(info.packageName).append(' ').append(info.versionName)
                .append(" (").append(info.versionCode).append(")\n");
        }
        catch (Exception ignored)
        {
        }
        text.append("phone: ").append(Build.MANUFACTURER).append(' ').append(Build.MODEL)
            .append(" (").append(Build.DEVICE).append(")\n");
        text.append("android: ").append(Build.VERSION.RELEASE).append(" (API ").append(Build.VERSION.SDK_INT).append(")\n");
        text.append("abi: ").append(String.join(", ", Build.SUPPORTED_ABIS)).append('\n');
        text.append("debug mode: ").append(DebugOptions.enabled(context)).append('\n');
        SharedPreferences preferences = GamePreferences.get(context);
        Map<String, ?> all = new TreeMap<>(preferences.getAll());
        for (Map.Entry<String, ?> entry : all.entrySet())
            if (entry.getKey().startsWith("debug_"))
                text.append(entry.getKey()).append(" = ").append(entry.getValue()).append('\n');
        return text.toString();
    }
}
