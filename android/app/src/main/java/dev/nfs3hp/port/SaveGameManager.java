package dev.nfs3hp.port;

import android.content.Context;
import android.net.Uri;
import java.io.*;
import java.nio.file.Files;
import java.nio.file.StandardCopyOption;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;
import java.util.zip.*;

/** Import/export for a data set's saved games, and optionally its settings.
 *
 *  Named tournaments, knockouts, ghosts and replays live in FEDATA/SAVE, but
 *  the most recent ghost and replay are kept as single files in the data root
 *  instead -- those two are what the game's "Last Replay" and last-ghost slots
 *  open -- and the settings live in FEDATA/CONFIG.  All of them travel in one
 *  archive, laid out exactly as they sit on disk. */
final class SaveGameManager {
    private SaveGameManager() {}

    /** Single files in the data root: the game's most recent ghost and replay. */
    private static final String[] LOOSE_SAVES = {"ghost.gst","replay.rp3"};
    private static final String SETTINGS_NAME = "config.dat";
    private static final String SETTINGS_ENTRY = "fedata/config/" + SETTINGS_NAME;
    /** Subdirectories of the staging area, one per destination. */
    private static final String STAGED_SAVES = "save", STAGED_LOOSE = "loose";

    static int exportZip(Context context,Uri uri,File dataRoot)throws IOException {
        return exportZip(context,uri,dataRoot,false);
    }

    static int exportZip(Context context,OutputStream raw,File dataRoot)throws IOException {
        return exportZip(context,raw,dataRoot,false);
    }

    static int exportZip(Context context,Uri uri,File dataRoot,boolean withSettings)throws IOException {
        OutputStream raw=context.getContentResolver().openOutputStream(uri,"wt");
        if(raw==null)throw new IOException(context.getString(R.string.saves_open_failed));
        try(OutputStream out=raw){return exportZip(context,out,dataRoot,withSettings);}
    }

    static int exportZip(Context context,OutputStream raw,File dataRoot,boolean withSettings)throws IOException {
        int[] count={0};
        /* No early "nothing to export" check: the save directory can be empty
         * while a last replay or ghost is still worth carrying. */
        try(ZipOutputStream zip=new ZipOutputStream(new BufferedOutputStream(raw))){
            File save=findSaveDirectory(dataRoot,false);
            if(save!=null&&save.isDirectory())writeTree(zip,save,"fedata/save/",count);
            for(String name:LOOSE_SAVES){
                File loose=findChild(dataRoot,name);
                if(loose!=null&&loose.isFile())writeFile(zip,loose,name,count);
            }
            if(withSettings){
                File settings=settingsFile(dataRoot);
                if(settings!=null&&settings.isFile())writeFile(zip,settings,SETTINGS_ENTRY,count);
            }
        }
        if(count[0]==0)throw new IOException(context.getString(R.string.saves_not_found));
        return count[0];
    }

    static String importZip(Context context,Uri uri,File dataRoot)throws IOException {
        return importZip(context,uri,dataRoot,false);
    }

    static String importZip(Context context,InputStream raw,File dataRoot)throws IOException {
        return importZip(context,raw,dataRoot,false);
    }

    static String importZip(Context context,Uri uri,File dataRoot,boolean withSettings)throws IOException {
        InputStream raw=context.getContentResolver().openInputStream(uri);
        if(raw==null)throw new IOException(context.getString(R.string.saves_open_failed));
        try(InputStream in=raw){return importZip(context,in,dataRoot,withSettings);}
    }

    static String importZip(Context context,InputStream raw,File dataRoot,boolean withSettings)throws IOException {
        File temporary=new File(dataRoot,".save-import.part");DataImporter.deleteRecursively(temporary);
        if(!temporary.mkdirs())throw new IOException("Could not prepare save import");
        return importZip(context,raw,dataRoot,temporary,withSettings);
    }

    private static String importZip(Context context,InputStream raw,File dataRoot,File temporary,boolean withSettings)throws IOException {
        File fedata=findChild(dataRoot,"fedata");
        if(fedata==null||!fedata.isDirectory())throw new IOException(context.getString(R.string.saves_no_active_data));
        int files=0;
        try {
            /* Everything lands in the staging area first, so a truncated or
             * hostile archive cannot leave the data set half replaced. */
            try(ZipInputStream zip=new ZipInputStream(new BufferedInputStream(raw))){
                ZipEntry entry;
                while((entry=zip.getNextEntry())!=null){
                    String relative=stagedPath(entry.getName(),withSettings);
                    if(relative==null||entry.isDirectory()){zip.closeEntry();continue;}
                    File out=checkedChild(temporary,relative);File parent=out.getParentFile();
                    if(parent==null||(!parent.isDirectory()&&!parent.mkdirs()))throw new IOException("Could not create "+parent);
                    try(OutputStream destination=new BufferedOutputStream(new FileOutputStream(out))){copy(zip,destination);}
                    files++;zip.closeEntry();
                }
            }
            if(files==0)throw new IOException(context.getString(R.string.saves_zip_empty));

            File[] backup={null};
            File staledSaves=new File(temporary,STAGED_SAVES);
            if(staledSaves.isDirectory()){
                File current=findChild(fedata,"save");
                File destination=current!=null?current:new File(fedata,"save");
                File saved=null;
                if(current!=null&&current.exists()){
                    saved=new File(backupFolder(dataRoot,backup),STAGED_SAVES);
                    Files.move(current.toPath(),saved.toPath(),StandardCopyOption.ATOMIC_MOVE);
                }
                final File restore=saved;
                try { Files.move(staledSaves.toPath(),destination.toPath(),StandardCopyOption.ATOMIC_MOVE); }
                catch(IOException failure){
                    if(restore!=null&&!destination.exists())Files.move(restore.toPath(),destination.toPath(),StandardCopyOption.ATOMIC_MOVE);
                    throw failure;
                }
            }

            /* The single files go in one at a time, each backed up first, so a
             * failure part way through still leaves every original recoverable
             * from the same folder. */
            File staged=new File(temporary,STAGED_LOOSE);
            File[] loose=staged.listFiles();
            if(loose!=null)for(File file:loose){
                File destination=looseDestination(dataRoot,fedata,file.getName());
                if(destination==null)continue;
                File parent=destination.getParentFile();
                if(parent!=null&&!parent.isDirectory()&&!parent.mkdirs())throw new IOException("Could not create "+parent);
                if(destination.exists())
                    Files.move(destination.toPath(),new File(backupFolder(dataRoot,backup),file.getName()).toPath(),
                               StandardCopyOption.ATOMIC_MOVE);
                Files.move(file.toPath(),destination.toPath(),StandardCopyOption.ATOMIC_MOVE);
            }
            return backup[0]==null?"":backup[0].getName();
        } finally { DataImporter.deleteRecursively(temporary); }
    }

    /** Where a staged single file belongs, or null for a name we do not place. */
    private static File looseDestination(File dataRoot,File fedata,String name){
        for(String known:LOOSE_SAVES)if(known.equalsIgnoreCase(name)){
            File existing=findChild(dataRoot,known);
            return existing!=null?existing:new File(dataRoot,known);
        }
        if(SETTINGS_NAME.equalsIgnoreCase(name)){
            File config=findChild(fedata,"config");
            File directory=config!=null?config:new File(fedata,"config");
            File existing=config!=null?findChild(config,SETTINGS_NAME):null;
            return existing!=null?existing:new File(directory,SETTINGS_NAME);
        }
        return null;
    }

    /** Creates the timestamped backup folder on first use, and only then. */
    private static File backupFolder(File dataRoot,File[] holder)throws IOException {
        if(holder[0]==null){
            File backups=new File(dataRoot,".save-backups");
            if(!backups.isDirectory()&&!backups.mkdirs())throw new IOException("Could not create save backup folder");
            String stamp=new SimpleDateFormat("yyyyMMdd-HHmmss",Locale.ROOT).format(new Date());
            File candidate=new File(backups,stamp);
            for(int suffix=1;candidate.exists();++suffix)candidate=new File(backups,stamp+"-"+suffix);
            if(!candidate.mkdirs())throw new IOException("Could not create "+candidate);
            holder[0]=candidate;
        }
        return holder[0];
    }

    private static File findSaveDirectory(File root,boolean create)throws IOException {
        File fedata=findChild(root,"fedata");if(fedata==null)return null;
        File save=findChild(fedata,"save");
        if(save==null&&create){save=new File(fedata,"save");if(!save.mkdirs())throw new IOException("Could not create save directory");}
        return save;
    }
    private static File findChild(File parent,String name){File exact=new File(parent,name);if(exact.exists())return exact;File[] files=parent.listFiles();if(files!=null)for(File f:files)if(f.getName().equalsIgnoreCase(name))return f;return null;}
    private static void writeTree(ZipOutputStream zip,File dir,String prefix,int[] count)throws IOException {File[] files=dir.listFiles();if(files==null)throw new IOException("Could not list "+dir);for(File f:files){String name=prefix+f.getName();if(f.isDirectory())writeTree(zip,f,name+"/",count);else{zip.putNextEntry(new ZipEntry(name));try(InputStream in=new BufferedInputStream(new FileInputStream(f))){copy(in,zip);}zip.closeEntry();count[0]++;}}}
    private static void copy(InputStream in,OutputStream out)throws IOException{byte[] buffer=new byte[64*1024];int n;while((n=in.read(buffer))>=0){if(Thread.currentThread().isInterrupted())throw new InterruptedIOException();out.write(buffer,0,n);}}
    private static void writeFile(ZipOutputStream zip,File file,String name,int[] count)throws IOException {
        zip.putNextEntry(new ZipEntry(name));
        try(InputStream in=new BufferedInputStream(new FileInputStream(file))){copy(in,zip);}
        zip.closeEntry();count[0]++;
    }

    private static File settingsFile(File dataRoot){
        File fedata=findChild(dataRoot,"fedata");if(fedata==null)return null;
        File config=findChild(fedata,"config");if(config==null)return null;
        return findChild(config,SETTINGS_NAME);
    }

    /* The security boundary.  An entry becomes a path inside the staging area
     * or nothing at all: absolute paths, "..", embedded NUL and any name that
     * is not one of the three accepted shapes are refused or skipped, so an
     * archive can never reach a file outside the save set.  A single file has
     * to sit at exactly the depth it does on disk -- one segment in the root,
     * or fedata/config/config.dat -- so "ghost.gst" cannot arrive disguised as
     * something nested. */
    private static String stagedPath(String name,boolean withSettings)throws IOException {
        String normalized=name.replace('\\','/');
        if(normalized.startsWith("/")||normalized.indexOf('\0')>=0)throw new IOException("Unsafe ZIP entry: "+name);
        String[] p=normalized.split("/");
        int save=-1;
        for(int i=0;i<p.length;i++){
            if("..".equals(p[i]))throw new IOException("Unsafe ZIP entry: "+name);
            if("save".equalsIgnoreCase(p[i])&&(i==0||"fedata".equalsIgnoreCase(p[i-1])))save=i;
        }
        if(save>=0){
            StringBuilder out=new StringBuilder(STAGED_SAVES);
            for(int i=save+1;i<p.length;i++)
                if(!p[i].isEmpty()&&!".".equals(p[i]))out.append(File.separatorChar).append(p[i]);
            return out.length()>STAGED_SAVES.length()?out.toString():null;
        }
        if(p.length==1)for(String known:LOOSE_SAVES)
            if(known.equalsIgnoreCase(p[0]))return STAGED_LOOSE+File.separatorChar+known;
        if(withSettings&&p.length==3&&"fedata".equalsIgnoreCase(p[0])
            &&"config".equalsIgnoreCase(p[1])&&SETTINGS_NAME.equalsIgnoreCase(p[2]))
            return STAGED_LOOSE+File.separatorChar+SETTINGS_NAME;
        return null;
    }
    private static File checkedChild(File root,String relative)throws IOException{File child=new File(root,relative);String base=root.getCanonicalPath()+File.separator;String path=child.getCanonicalPath();if(!path.startsWith(base))throw new IOException("Unsafe ZIP entry: "+relative);return child;}
}
