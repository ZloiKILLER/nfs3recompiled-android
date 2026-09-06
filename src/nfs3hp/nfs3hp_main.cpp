#include <SDL3/SDL_main.h>
#include <lib/file.h>
#include <lib/registry.h>
#include <nfs3hp.h>
#include <string>

#ifdef __ANDROID__
#include <SDL3/SDL_system.h>
#endif

static std::string getExeDirectory(const char* argv0)
{
    std::string path(argv0);
    for (auto& c : path)
        if (c == '\\') c = '/';
    auto pos = path.rfind('/');
    if (pos != std::string::npos)
        return path.substr(0, pos + 1);
    return "./";
}

int main(int argc, char* argv[])
{
#ifdef __ANDROID__
    /* There is no argv[1]/argv[2] on Android and argv[0] carries no directory
     * (SDL synthesizes it), so getExeDirectory()'s fallback would resolve to
     * the process cwd, not the app's data.  SDL_GetAndroidExternalStoragePath()
     * is Context.getExternalFilesDir() -- app-private but world-readable, which
     * is where the first-run import screen (M3) copies fedata/gamedata/drivers
     * into.  There is no separate "CD" location on Android: both point here. */
    NFS2_USE(argc);
    NFS2_USE(argv);
    const char* dataPath = SDL_GetAndroidExternalStoragePath();
    NFS2_ASSERT(dataPath);
    win32::File::setDataDirectory(dataPath);
    win32::File::setCdDirectory(dataPath);
#else
    if (argc >= 3)
    {
        win32::File::setDataDirectory(argv[1]);
        win32::File::setCdDirectory(argv[2]);
    }
    else if (argc == 2)
    {
        win32::File::setDataDirectory(argv[1]);
        win32::File::setCdDirectory(argv[1]);
    }
    else
    {
        std::string exeDir = getExeDirectory(argv[0]);
        win32::File::setDataDirectory(exeDir.c_str());
        win32::File::setCdDirectory(exeDir.c_str());
    }
#endif
    /* The game pumps its message queue from a guest thread, which is not
     * the thread SDL considers "main" on Android.  SDL only warns about
     * it, but the warning is a modal dialog that has to be dismissed on
     * every single launch.  Suppress the dialog rather than the check:
     * nothing downstream depends on it, and a real fix means moving the
     * pump onto the SDL main thread, which is a much larger change. */
    SDL_SetHint(SDL_HINT_ASSERT, "always_ignore");
    /* Without this the game starts in portrait on Android, and the manifest
     * cannot stop it: SDLActivity.setOrientationBis() calls
     * setRequestedOrientation() at runtime and overrides screenOrientation.
     * It derives what to allow from SDL_HINT_ORIENTATIONS, and with the hint
     * unset it decides nothing is explicitly allowed, which for a resizable
     * window means SCREEN_ORIENTATION_FULL_USER -- i.e. whatever the system
     * auto-rotate setting says.  NFS_ORIENTATION lets the launcher pick:
     * "auto" also allows portrait, anything else pins landscape. */
    {
        const char* orientation = SDL_getenv("NFS_ORIENTATION");
        const bool autoRotate = orientation && SDL_strcasecmp(orientation, "auto") == 0;
        SDL_SetHint(SDL_HINT_ORIENTATIONS,
                    autoRotate ? "LandscapeLeft LandscapeRight Portrait PortraitUpsideDown"
                               : "LandscapeLeft LandscapeRight");
    }
    /* SDL_INIT_GAMEPAD is what turns raw joysticks into the mapped
     * SDL_EVENT_GAMEPAD_* stream window.cpp translates into keystrokes. */
    SDL_Init(SDL_INIT_EVENTS|SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_JOYSTICK|SDL_INIT_GAMEPAD);
    /* Window::getMessage consumes key, text, quit and its own registered user
     * events; everything else falls through its switch and is thrown away.  The
     * pad is read by polling its state (Gamepad::updateKeys), never from
     * events.  Leaving those event types enabled meant a connected controller,
     * reporting at its native rate, kept the queue non-empty -- so SDL_WaitEvent
     * never actually blocked and the message pump ran SDL_PumpEvents thousands
     * of times a second for nothing.  A simpleperf profile showed that thread
     * burning most of a core inside SDL with zero game code on it. */
    SDL_SetJoystickEventsEnabled(false);
    SDL_SetGamepadEventsEnabled(false);
    {
        nfs3hp::Application app("nfs3.exe");
        app.addRegistryKey(win32::HKEY_LOCAL_MACHINE, "SOFTWARE\\Electronic Arts\\Need For Speed III", "3D Device Description", new win32::RegistryValue("3Dfx Voodoo 2"));
        app.addRegistryKey(win32::HKEY_LOCAL_MACHINE, "SOFTWARE\\Electronic Arts\\Need For Speed III", "3D Card", new win32::RegistryValue("3Dfx Voodoo 2"));
        //app.addRegistryKey(win32::HKEY_LOCAL_MACHINE, "SOFTWARE\\Electronic Arts\\Need For Speed III", "Thrash Driver", new win32::RegistryValue("softtri"));
        app.addRegistryKey(win32::HKEY_LOCAL_MACHINE, "SOFTWARE\\Electronic Arts\\Need For Speed III", "Thrash Driver", new win32::RegistryValue("voodoo2"));
        app.addRegistryKey(win32::HKEY_LOCAL_MACHINE, "SOFTWARE\\Electronic Arts\\Need For Speed III", "Group", new win32::RegistryValue("3Dfx"));
        app.addRegistryKey(win32::HKEY_LOCAL_MACHINE, "SOFTWARE\\Electronic Arts\\Need For Speed III", "D3D Device", new win32::RegistryValue(x86::reg32(0)));
        app.addRegistryKey(win32::HKEY_LOCAL_MACHINE, "SOFTWARE\\Electronic Arts\\Need For Speed III", "Triple Buffer", new win32::RegistryValue(x86::reg32(0)));
        //app.addRegistryKey(win32::HKEY_LOCAL_MACHINE, "SOFTWARE\\Electronic Arts\\Need For Speed III", "Hardware Acceleration", new win32::RegistryValue(x86::reg32(0)));
        app.addRegistryKey(win32::HKEY_LOCAL_MACHINE, "SOFTWARE\\Electronic Arts\\Need For Speed III", "Hardware Acceleration", new win32::RegistryValue(x86::reg32(1)));
        app.execute();
    }
    SDL_Quit();
    return 0;
}
