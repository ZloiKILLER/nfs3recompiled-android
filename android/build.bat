@echo off
set "JAVA_HOME=D:\Android Studio\jbr"
set "ANDROID_HOME=D:\Android_SDK"
cd /d D:\nfs3android\nfs-recompiled\android
call .\gradlew.bat assembleDebug %*
