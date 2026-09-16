#include <SDL3/SDL_main.h>
#include <lib/file.h>
#include <lib/registry.h>
#include <lib/gamepad.h>
#include <nfs3hp.h>
#include <winapi/glide2x.h>
#include <SDL3/SDL.h>
#include <array>
#include <string>
#include <unordered_map>

namespace nfs3hp
{
/* NFS_CAR_DETAIL_FULL, read once: unset or anything but "0" means on.  The
 * generated code patched by tools/apply_car_detail.py asks it too. */
bool fullCarDetail()
{
    static const bool full = []() {
        const char* value = SDL_getenv("NFS_CAR_DETAIL_FULL");
        return !value || SDL_strcmp(value, "0") != 0;
    }();
    return full;
}

/* Wheel spin, per car.  The game turns a car's wheels by the frames elapsed
 * since one stamp shared by every car and every view (0x55fe28), written as
 * each view pass ends, so a car first drawn in a later pass of the same frame
 * -- the second split-screen view, the rear-view mirror -- saw no time pass and
 * its wheels stood still.  tools/apply_car_detail.py routes the three reads of
 * that stamp here: sub_4b9140 and sub_4b9280 for the two wheel angles, and
 * sub_4b90e0, two ways, for the body.  Each keeps its own stamp per car, so a
 * car still advances once per frame, whichever view draws it first. */
x86::reg32 carWheelStamp(win32::WinApplication* app, x86::reg32 car, x86::reg32 which)
{
    static std::unordered_map<x86::reg32, std::array<x86::reg32, 4>> s_stamps;
    const x86::reg32 frame = app->getMemory<x86::reg32>(0x7d3684);
    auto entry = s_stamps.try_emplace(car);
    x86::reg32& stamp = entry.first->second[which & 3];
    x86::reg32 last = entry.second ? app->getMemory<x86::reg32>(0x55fe28) : stamp;
    /* A stamp from an earlier race -- the frame count starts over and car
     * records are reused -- would freeze the wheels until the count caught up,
     * and one from long ago would spin them through hundreds of turns at once. */
    if (last > frame || frame - last > 4)
        last = frame - 1;
    stamp = frame;
    return last;
}

/* Screen sizes the Graphics menu may list.  sub_4bed90 passes the menu only
 * driver modes whose width is exactly 4/3 of their height, and
 * tools/apply_widescreen.py has it ask here too: an exact 16:9 is taken as
 * well, which is the 1280x720 main() adds to the Voodoo2 driver's table.  The
 * driver's own 856x480 is not quite 16:9 and stays unlisted. */
bool widescreenMode(x86::reg32 width, x86::reg32 height)
{
    return width * 9 == height * 16;
}

/* A camera's horizontal half field of view, in whole degrees, for the screen the
 * race is drawn on (tools/apply_widescreen.py, in sub_4dbce0).  The game makes
 * each view's vertical angle 13/16 of its horizontal one, proportions that hold
 * on a 4:3 screen only, so on anything wider the picture came out stretched
 * sideways.  The vertical angle stays what the game makes it; the horizontal one
 * opens up by as much as the screen is wider than 4:3 -- Hor+ -- for every view
 * alike: the chase and in-car cameras, the mirror, both halves of split screen.
 * 4:3 and narrower screens keep the game's own angle to the degree.  The screen
 * is the mode the race switched to (sub_4bef50 copies it to 0x7cdae0). */
x86::reg32 widescreenHalfAngle(win32::WinApplication* app, x86::reg32 half)
{
    const x86::reg32 width = app->getMemory<x86::reg32>(0x7cdae0);
    const x86::reg32 height = app->getMemory<x86::reg32>(0x7cdae4);
    if (!height || width * 3 <= height * 4 || x86::sreg32(half) <= 0 || half >= 90)
        return half;
    const double wider = double(width) * 3.0 / (double(height) * 4.0);
    const double angle = SDL_atan(SDL_tan(double(half) * SDL_PI_D / 180.0) * wider) * 180.0 / SDL_PI_D;
    return x86::reg32(SDL_lround(angle));
}

/* HUD elements the game draws as pictures keep their shape on a wide screen
 * (tools/apply_widescreen.py, at the end of sub_480910).  The game lays every
 * element out as a fraction of the screen -- GameData/DashHud/*.POS, kept in
 * config.dat -- and sub_480910 turns that into pixels by the screen's width and
 * height, so on 16:9 each came out a third wider than it was drawn.  The analog
 * speedometer and tachometer, the tutorial icon and the replay bar (elements 1,
 * 3, 6 and 8 of the 23 a .POS file lists) are narrowed about their centre by as
 * much as the screen is wider than 4:3, and the spike belt indicator (21) is
 * made taller by as much instead: the elements, and the ways, the Modern Patch
 * settled on.  Everything else keeps its layout, still a fraction of the screen,
 * and config.dat is never touched, so a 4:3 screen finds the HUD as it was.
 * `slot` is the element's offset in the table of pixel rectangles at 0x749758:
 * 23 per player, four ints each -- x0, y0, x1, y1.  How much wider is taken from
 * the screen mode itself, not from the size sub_480910 was handed, which in
 * split screen is one player's half. */
void widescreenHudRect(win32::WinApplication* app, x86::reg32 slot)
{
    const x86::reg32 width = app->getMemory<x86::reg32>(0x7cdae0);
    const x86::reg32 height = app->getMemory<x86::reg32>(0x7cdae4);
    if (!height || width * 3 <= height * 4)
        return;
    const double narrow = double(height) * 4.0 / (double(width) * 3.0);
    const x86::reg32 element = (slot % (23 * 16)) / 16;
    x86::reg32 first, second;
    double scale;
    if (element == 1 || element == 3 || element == 6 || element == 8)
    {
        first = 0;  // x0 and x1
        second = 8;
        scale = narrow;
    }
    else if (element == 21)
    {
        first = 4;  // y0 and y1
        second = 12;
        scale = 1.0 / narrow;
    }
    else
        return;
    const x86::reg32 rect = 0x749758 + slot;
    const double a = double(x86::sreg32(app->getMemory<x86::reg32>(rect + first)));
    const double b = double(x86::sreg32(app->getMemory<x86::reg32>(rect + second)));
    const double centre = (a + b) / 2.0, half = (b - a) * scale / 2.0;
    app->getMemory<x86::reg32>(rect + first) = x86::reg32(x86::sreg32(SDL_lround(centre - half)));
    app->getMemory<x86::reg32>(rect + second) = x86::reg32(x86::sreg32(SDL_lround(centre + half)));
}
}

namespace
{
/* NFS_CAR_DETAIL_TRACE: once a second while a race is drawn, what car detail
 * the game settled on, read back from its own state after each swap.
 *  - The level every car was last drawn at (car+0x8b4, -1 when it was not
 *    drawn) and what that costs against the budget in sub_4bbad0: 4 minus the
 *    level per car, capped per Car Detail setting by the table at 0x4b7a50.
 *  - The transform buffer behind Render_GetTm (sub_4bbde0), a bump allocator:
 *    base 0x55fe34, cursor 0x55fe38, limit 0x7a3d04.  It starts over for every
 *    3D pass and does not take a failed allocation back, so a cursor past the
 *    limit at swap time means the last pass ran out and left something undrawn.
 *  - The texture atlas's free space.
 * Levels and the buffer describe the frame's last 3D pass, which with the
 * rear-view mirror on need not be the main view. */
struct CarDetailTrace
{
    Uint64 lastLog = 0;
    x86::reg32 peakBuffer = 0;
    x86::reg32 overflowFrames = 0;
    x86::reg32 frames = 0;
};
CarDetailTrace s_carDetailTrace;

void traceCarDetail(win32::WinApplication* app)
{
    CarDetailTrace& trace = s_carDetailTrace;
    const x86::reg32 limit = app->getMemory<x86::reg32>(0x7a3d04);
    const x86::reg32 base = app->getMemory<x86::reg32>(0x55fe34);
    const x86::reg32 cursor = app->getMemory<x86::reg32>(0x55fe38);
    if (base && cursor >= base)
    {
        trace.peakBuffer = SDL_max(trace.peakBuffer, cursor - base);
        if (cursor - base > limit)
            ++trace.overflowFrames;
    }
    ++trace.frames;
    const Uint64 now = SDL_GetTicks();
    if (now - trace.lastLog < 1000)
        return;
    trace.lastLog = now;

    const x86::reg32 count = app->getMemory<x86::reg32>(0x5efd9c);
    int levels[4] = {};
    int hidden = 0;
    int points = 0;
    for (x86::reg32 i = 0; i < count && i < 64; ++i)
    {
        const x86::reg32 car = app->getMemory<x86::reg32>(0x5efac8 + i * 4);
        const x86::sreg32 level = car ? x86::sreg32(app->getMemory<x86::reg32>(car + 0x8b4)) : -1;
        if (level < 0 || level > 3)
        {
            ++hidden;
            continue;
        }
        ++levels[level];
        points += 4 - level;
    }
    const x86::reg32 detail = app->getMemory<x86::reg32>(0x6fbc1c);
    const x86::sreg32 budget = detail < 3 ? x86::sreg32(app->getMemory<x86::reg32>(0x4b7a50 + detail * 4)) : 0;
    x86::reg32 freeTexels = 0, freeTiles = 0, totalTexels = 0;
    win32::glide2x::atlasFreeSpace(freeTexels, freeTiles, totalTexels);
    SDL_Log("[CARDETAIL] detail=%u cars=%u L0=%d L1=%d L2=%d L3=%d hidden=%d points=%d budget=%d"
            " buffer peak=%u/%u overflow=%u/%u frames atlas=%u free=%u%% tiles256=%u",
            unsigned(detail), unsigned(count), levels[0], levels[1], levels[2], levels[3], hidden,
            points, int(budget), unsigned(trace.peakBuffer), unsigned(limit),
            unsigned(trace.overflowFrames), unsigned(trace.frames), unsigned(SDL_sqrt(double(totalTexels))),
            totalTexels ? unsigned(100ull * freeTexels / totalTexels) : 0u, unsigned(freeTiles));
    trace.peakBuffer = 0;
    trace.overflowFrames = 0;
    trace.frames = 0;
}

/* How hard player one's car corners, for the phone's own vibration, about every
 * 40 ms while a race is drawn.  It is the load the game's Body Roll force
 * feedback is made of (sub_4716e0): [car+0x924] against the scale at 0x5efde4,
 * full where the game clamps that effect.  Read from the car rather than taken
 * from the effect, which the game only computes while its Body Roll slider is
 * above zero -- a pad's setting the phone should not depend on.  The car is the
 * one force feedback follows (0x678b54); while its flag at +0x164 is set or
 * +0x15c is negative the game stops its effects, and there is no cornering to
 * feel either. */
void phoneTick(win32::WinApplication* app)
{
    static Uint64 last = 0;
    const Uint64 now = SDL_GetTicks();
    if (now - last < 40)
        return;
    last = now;
    float turn = 0;
    const x86::reg32 car = app->getMemory<x86::reg32>(0x678b54);
    const float scale = app->getMemory<float>(0x5efde4);
    if (car && scale > 0 && app->getMemory<x86::reg16>(car + 0x164) == 0
        && app->getMemory<float>(car + 0x15c) >= 0)
        turn = SDL_min(1.0f, SDL_fabsf(app->getMemory<float>(car + 0x924)) / scale);
    win32::Gamepad::phoneTick(turn);
}

bool s_traceCarDetail = false;

/* Every buffer swap: the phone's cornering, and the car detail trace when asked
 * for (NFS_CAR_DETAIL_TRACE). */
void onSwap(win32::WinApplication* app)
{
    phoneTick(app);
    if (s_traceCarDetail)
        traceCarDetail(app);
}
}

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
     * pad is read by polling its state (Gamepad::update), never from
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
         * itself -- so NFS_CAR_DETAIL_FULL=0 restores the original distances,
         * budget and texture sizes without a rebuild.  Only the High row is
         * touched: Medium and Low stay as they were, and the cull distance
         * itself is left alone so the draw distance does not change. */
        if (nfs3hp::fullCarDetail())
        {
            /* Twice the cull distance rather than the distance itself: split
             * screen scales the three switch distances by 0.75, and the rear-view
             * mirror scales them and the cull distance by 0.6 (sub_4bb5d0), both
             * at once in a split-screen mirror.  Twice the cull distance stays at
             * or beyond the cull distance under all of those, so a car is either
             * on the detailed model or not drawn at all, in every view.  Held at
             * just the cull distance, split screen drew every car beyond 225 as
             * the tiny box. */
            const x86::reg32 highRow = 0x55fdf4;
            const float cull = app.getMemory<float>(highRow + x86::reg32(12));
            for (x86::reg32 level = 0; level < 3; ++level)
                app.getMemory<float>(highRow + level * 4) = cull * 2.0f;
            /* The distances alone did not do it.  Once every car has its level,
             * sub_4bbad0 charges each car it will draw 4 minus that level against
             * a budget per setting -- 10, 8 and 6 in the table at 0x4b7a50 -- and
             * while over, drops the furthest car at medium to low, or failing that
             * the furthest detailed car to medium.  It never touches the car the
             * view follows and never goes below low.  The player's car spends 4
             * of High's 10, so with three other cars in view all of them end up
             * on the low model, whose wheels are part of the body and do not turn,
             * however close they are: detail that fell with the number of cars
             * and, of all places, up close.  The game copies the table on every
             * call, so lifting High's budget here is enough. */
            const x86::reg32 budgetTable = 0x4b7a50;
            app.getMemory<x86::reg32>(budgetTable) = 0x7fffffff;
            /* Every car loads the player's 256x256 texture as well (the patched
             * sub_4b7d30), up to four times the atlas room each.  A 4096 atlas
             * takes about 90 MB of graphics memory, so only where memory is to
             * spare; elsewhere a texture that does not fit is kept a mip level
             * smaller rather than dropped. */
            if (SDL_GetSystemRAM() >= 3072)
                win32::glide2x::setPreferredAtlasSize(4096);
        }
        const char* detailTrace = SDL_getenv("NFS_CAR_DETAIL_TRACE");
        s_traceCarDetail = detailTrace && *detailTrace && *detailTrace != '0';
        win32::glide2x::setSwapObserver(onSwap);

        /* 1280x720 for races, experimental; NFS_WIDESCREEN=0 leaves the driver as
         * it was.  The Voodoo2 driver's mode table (0xa92018, 40 bytes a mode:
         * width, height, depth, LFB format, available, then the colour and aux
         * buffer counts sub_a83ac0 works out from the card's memory) has entries
         * it never offers, and 14 is one: a 1280x1200 no card could open.  It
         * becomes 1280x720 under a resolution id of the port's own in the table
         * THRASH_setvideomode hands grSstWinOpen (0xa922e8), and the Graphics
         * menu's Screen Size lists it once tools/apply_widescreen.py lets a 16:9
         * mode through.  Only the race switches mode; the menus stay 640x480.
         * The driver's data is in place from construction on -- LoadLibrary does
         * not copy it again -- so this sticks. */
        const char* widescreen = SDL_getenv("NFS_WIDESCREEN");
        if (!widescreen || SDL_strcmp(widescreen, "0") != 0)
        {
            const x86::reg32 mode = 0xa92018 + 14 * 40;
            app.getMemory<x86::reg32>(mode) = 1280;
            app.getMemory<x86::reg32>(mode + 4) = 720;
            app.getMemory<x86::reg32>(mode + 16) = 1;
            app.getMemory<x86::reg32>(0xa922e8 + 14 * 4) = win32::glide2x::kResolution1280x720;
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
/* Which physical pad is Gamepad 1 and which is Gamepad 2, resolved by the
 * launcher's rules (GamepadSlots.java) and sent again whenever Android attaches
 * or detaches an input device.  Called on the Android UI thread; the slots
 * themselves are only touched on the game thread, on the next frame. */
extern "C" JNIEXPORT void JNICALL
Java_dev_nfs3hp_port_NFS3Activity_nativeSetGamepadSlots(JNIEnv* env, jclass, jstring first, jstring second)
{
    const char* a = first ? env->GetStringUTFChars(first, nullptr) : nullptr;
    const char* b = second ? env->GetStringUTFChars(second, nullptr) : nullptr;
    win32::Gamepad::setSlotDescriptors(a, b);
    if (a)
        env->ReleaseStringUTFChars(first, a);
    if (b)
        env->ReleaseStringUTFChars(second, b);
}
#endif
