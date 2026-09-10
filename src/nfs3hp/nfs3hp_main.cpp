#include <SDL3/SDL_main.h>
#include <lib/file.h>
#include <lib/registry.h>
#include <nfs3hp.h>
#include <string>

#ifdef __ANDROID__
#include <SDL3/SDL_system.h>
#include <SDL3/SDL.h>
#include <lib/renderer.h>
#include <jni.h>
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
     * auto-rotate setting says.  The game is landscape only; NFS_ORIENTATION
     * just picks whether it may turn over: "auto" allows both landscape
     * directions, anything else pins one.  The hint has to agree with the
     * activity's requested orientation, because setOrientationBis derives the
     * request from the hint and would otherwise undo it. */
    {
        /* SDL maps LandscapeLeft to SCREEN_ORIENTATION_LANDSCAPE and
         * LandscapeRight to its reverse, so naming one of them here is what
         * pins that direction; naming both hands the choice to the sensor. */
        const char* orientation = SDL_getenv("NFS_ORIENTATION");
        const bool autoRotate = orientation && SDL_strcasecmp(orientation, "auto") == 0;
        const bool reversed = orientation && SDL_strcasecmp(orientation, "landscape_reverse") == 0;
        SDL_SetHint(SDL_HINT_ORIENTATIONS,
                    autoRotate ? "LandscapeLeft LandscapeRight"
                    : reversed ? "LandscapeRight" : "LandscapeLeft");
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

        /* Car detail is chosen by distance alone -- there is no per-car role
         * anywhere in the selection.  The table at 0x55fdf4 holds four floats
         * per "cardetail" setting: three level-of-detail switch distances and a
         * cull distance, and level 0 (High) switches away from the detailed
         * model at 30 while culling only at 300.  The player's car sits inside
         * 30 under every chase camera, which is the whole reason it alone looks
         * detailed; opponents, traffic and the second player drop a level as
         * soon as they are further away than that.
         *
         * Holding the detailed model out to the cull distance makes every car
         * match the player's.  It costs emulated CPU rather than GPU -- Glide
         * takes screen-space vertices, so the game transforms all that geometry
         * itself -- so NFS_CAR_DETAIL_FULL=0 restores the original distances
         * without a rebuild.  Only the High row is touched: Medium and Low stay
         * as they were, and the cull distance itself is left alone so the draw
         * distance does not change. */
        const char* fullDetail = SDL_getenv("NFS_CAR_DETAIL_FULL");
        if (!fullDetail || SDL_strcmp(fullDetail, "0") != 0)
        {
            const x86::reg32 highRow = 0x55fdf4;
            const float cull = app.getMemory<float>(highRow + x86::reg32(12));
            for (x86::reg32 level = 0; level < 3; ++level)
                app.getMemory<float>(highRow + level * 4) = cull;
        }

#ifndef __ANDROID__
        // Headless driver metadata regression: exercise the real recompiled queries.
        if (SDL_getenv("NFS_CHECK_GLIDE_MODES")) {
            for (const auto entry : {0x504e00u, 0xa83c60u}) {
                x86::CPU cpu{};
                app.runThread(cpu, entry);
                const auto table = app.getMemory<x86::reg32>(cpu.eax + 0x40);
                const auto count = app.getMemory<x86::reg32>(cpu.eax + 0x3c);
                if (count != 16) return 2;
                for (unsigned i = 1; i <= count; ++i) {
                    const auto* mode = &app.getMemory<x86::reg32>(table + i*40);
                    if (mode[2] != 32 || mode[3] != 4) return 3;
                    SDL_Log("[OUTPUT32] driver=%x id=%u %ux%ux%u LFB-format=%u available=%u buffers=%u",
                            entry, i, mode[0], mode[1], mode[2], mode[3], mode[4], mode[5]);
                }
                // Repeat query, including the cached descriptor path.
                app.runThread(cpu, entry);
                if (app.getMemory<x86::reg32>(table + 40 + 8) != 32) return 4;
            }
            SDL_Log("[OUTPUT32] PASS: both drivers and cached queries");
        } else
#endif
        app.execute();
    }
    SDL_Quit();
    return 0;
}

#ifdef __ANDROID__
/* The overlay's keyboard button, both ways.  It goes through SDL rather than
 * SDLActivity.showTextInput() because showing the Android keyboard is only half
 * of it: SDL_SendKeyboardText drops every character while text input is
 * inactive, so a keyboard raised behind SDL's back deletes (backspace is a key
 * event, ungated) but types nothing.  Starting and stopping through SDL also
 * raises the shown/hidden events the renderer listens to, so the picture rises
 * and drops on its own.
 *
 * Called on the Android UI thread; so is SDL's own SDL_StopTextInput from
 * SDLDummyEdit.onKeyPreIme when the system back key closes the keyboard. */
extern "C" JNIEXPORT void JNICALL
Java_dev_nfs3hp_port_NFS3Activity_nativeToggleKeyboard(JNIEnv*, jclass)
{
    SDL_Window* window = SDL_GetKeyboardFocus();
    if (!window)
    {
        int count = 0;
        SDL_Window* const* windows = SDL_GetWindows(&count);
        if (count > 0)
            window = windows[0];
    }
    if (!window)
        return;

    if (SDL_TextInputActive(window))
        SDL_StopTextInput(window);
    else
        SDL_StartTextInput(window);
}
#endif
