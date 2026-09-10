package dev.nfs3hp.port;

import android.app.Instrumentation;
import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.util.zip.*;

final class SaveGameChecks {
    private static String read(File file)throws IOException {
        return new String(Files.readAllBytes(file.toPath()),StandardCharsets.UTF_8);
    }
    private static void write(File file,String text)throws IOException {
        Files.write(file.toPath(),text.getBytes(StandardCharsets.UTF_8));
    }
    static void run(Instrumentation test)throws Exception {
        File root=new File(test.getTargetContext().getCacheDir(),"save-game-checks");
        DataImporter.deleteRecursively(root);
        /* Upper case on purpose: the game's own directories are, and every
         * lookup here has to survive that. */
        File save=new File(root,"FEDATA/SAVE"),config=new File(root,"FEDATA/CONFIG");
        if(!save.mkdirs()||!config.mkdirs())throw new AssertionError("test folders");
        File profile=new File(save,"player.sav");write(profile,"old");
        File ghost=new File(root,"ghost.gst");write(ghost,"g-old");
        File settings=new File(config,"config.dat");write(settings,"cfg-old");

        /* The last ghost lives beside the data root rather than in the save
         * folder, so it has to travel without being asked for; settings only
         * travel when they are. */
        ByteArrayOutputStream plain=new ByteArrayOutputStream();
        if(SaveGameManager.exportZip(test.getTargetContext(),plain,root,false)!=2)
            throw new AssertionError("export covers the save folder and the loose ghost");
        ByteArrayOutputStream full=new ByteArrayOutputStream();
        if(SaveGameManager.exportZip(test.getTargetContext(),full,root,true)!=3)
            throw new AssertionError("export adds settings when asked");

        write(profile,"changed");write(ghost,"g-changed");write(settings,"cfg-changed");
        String backup=SaveGameManager.importZip(test.getTargetContext(),
            new ByteArrayInputStream(full.toByteArray()),root,false);
        if(backup.isEmpty())throw new AssertionError("save backup missing");
        if(!"old".equals(read(new File(root,"FEDATA/SAVE/player.sav"))))throw new AssertionError("save import payload");
        if(!"g-old".equals(read(ghost)))throw new AssertionError("loose ghost import payload");
        /* Present in the archive, refused by the caller: settings must be left
         * exactly as the device had them. */
        if(!"cfg-changed".equals(read(settings)))throw new AssertionError("settings imported without being asked");
        File backups=new File(root,".save-backups/"+backup);
        if(!"changed".equals(read(new File(backups,"save/player.sav"))))throw new AssertionError("save backup payload");
        if(!"g-changed".equals(read(new File(backups,"ghost.gst"))))throw new AssertionError("loose ghost backup payload");

        if(SaveGameManager.importZip(test.getTargetContext(),
            new ByteArrayInputStream(full.toByteArray()),root,true).isEmpty())
            throw new AssertionError("second import backup missing");
        if(!"cfg-old".equals(read(settings)))throw new AssertionError("settings import payload");

        ByteArrayOutputStream unsafe=new ByteArrayOutputStream();
        try(ZipOutputStream zip=new ZipOutputStream(unsafe)){zip.putNextEntry(new ZipEntry("fedata/save/../../escape"));zip.write(1);zip.closeEntry();}
        boolean rejected=false;try{SaveGameManager.importZip(test.getTargetContext(),new ByteArrayInputStream(unsafe.toByteArray()),root);}catch(IOException expected){rejected=true;}
        if(!rejected)throw new AssertionError("unsafe save archive accepted");

        /* A name we do not place must be ignored rather than written anywhere. */
        ByteArrayOutputStream stray=new ByteArrayOutputStream();
        try(ZipOutputStream zip=new ZipOutputStream(stray)){zip.putNextEntry(new ZipEntry("nfs3.exe"));zip.write(1);zip.closeEntry();}
        boolean empty=false;
        try{SaveGameManager.importZip(test.getTargetContext(),new ByteArrayInputStream(stray.toByteArray()),root,true);}
        catch(IOException expected){empty=true;}
        if(!empty)throw new AssertionError("archive of unrelated files accepted");
        if(new File(root,"nfs3.exe").exists())throw new AssertionError("unrelated entry was written");

        DataImporter.deleteRecursively(root);
    }
}
