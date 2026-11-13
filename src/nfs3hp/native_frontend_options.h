#ifndef NFS3HP_NATIVE_FRONTEND_OPTIONS_H
#define NFS3HP_NATIVE_FRONTEND_OPTIONS_H

#include "native_frontend.h"
#include "native_guest.h"
#include <cstring>

/* native_frontend_options.cpp's typed functions (the options screens: options, audio, audfeed,
 * funkeys, hardkeys, control1, deadzone, force, graphics, advgrph), which other files call as C++
 * (guest::direct), and how the game's code calls each by address (ByAddress, for the checks'
 * stand-ins).  Each calls the game's other functions by address on `cpu` and throws
 * fe::Terminated when the game stopped in one, as Guest::call does; its result is what the game's
 * function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace options
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x442aa0: options as it opens: no sound screen without sound, no loading nor credits in a network
// race.
reg32 onOptionsEnter(App* app, CPU& cpu, reg32 menu);
// The game's 0x437250: the three audio feeds on keys of their own, the time first.
reg32 separateFeeds(App* app, CPU& cpu);
// The game's 0x437300: the time feed's button: Enter moves it on to a key no other feed has.
reg32 feedTimeButtonKey(App* app, CPU& cpu, reg32 item, reg32 menu, std::uint16_t key);
// The game's 0x437350: the time feed's list: a key another feed has is not taken.
reg32 feedTimeListKey(App* app, CPU& cpu, reg32 item, reg32 menu, std::uint16_t key);
// The game's 0x437390: the position feed's button, likewise.
reg32 feedPositionButtonKey(App* app, CPU& cpu, reg32 item, reg32 menu, std::uint16_t key);
// The game's 0x4373e0: the position feed's list, likewise.
reg32 feedPositionListKey(App* app, CPU& cpu, reg32 item, reg32 menu, std::uint16_t key);
// The game's 0x437420: the speed feed's button, likewise.
reg32 feedSpeedButtonKey(App* app, CPU& cpu, reg32 item, reg32 menu, std::uint16_t key);
// The game's 0x437470: the speed feed's list, likewise.
reg32 feedSpeedListKey(App* app, CPU& cpu, reg32 item, reg32 menu, std::uint16_t key);
// The game's 0x4374b0: audfeed as it opens: the feeds kept for Cancel, each item its key handler.
reg32 onFeedEnter(App* app, CPU& cpu, reg32 menu);
// The game's 0x437590: audfeed as it closes: Cancel puts the feeds back, anything else separates them.
reg32 onFeedExit(App* app, CPU& cpu, reg32 menu);
// The game's 0x437600: funkeys as it opens: its lines of text placed in rows.
reg32 funkeysEnter(App* app, CPU& cpu, reg32 menu);
// The game's 0x4377d0: hardkeys as it opens, as funkeys.
reg32 hardkeysEnter(App* app, CPU& cpu, reg32 menu);
// The game's 0x437990: the menu music's volume slider moved.
reg32 audioMenuMusicMoved(App* app, CPU& cpu, reg32 item);
// The game's 0x4379b0: the race music's volume slider moved.
reg32 audioRaceMusicMoved(App* app, CPU& cpu, reg32 item);
// The game's 0x4379d0: the effects' volume slider moved, the race's own in a race.
reg32 audioEffectsMoved(App* app, CPU& cpu, reg32 item);
// The game's 0x437a20: the engine's volume slider moved, the race's own in a race.
reg32 audioEngineMoved(App* app, CPU& cpu, reg32 item);
// The game's 0x437a70: the speech's volume slider moved.
reg32 audioSpeechMoved(App* app, CPU& cpu, reg32 item);
// The game's 0x437a90: a volume slider drawn; selected, it plays at its volume.
reg32 onVolumeSliderDraw(App* app, CPU& cpu, reg32 item, reg32 selected);
// The game's 0x437e00: audio as it opens (psaudio too): the settings kept, the sliders set.
reg32 onAudioEnter(App* app, CPU& cpu, reg32 menu);
// The game's 0x438070: audio as it closes: the race's volumes back, Cancel puts back what was kept.
reg32 onAudioExit(App* app, CPU& cpu, reg32 menu);
// The game's 0x438260: audio on every pass: a change of the setup starts the sound over.
reg32 onAudioFrame(App* app, CPU& cpu);
// The game's 0x43d3f0: control1's Cancel: Enter marks the screen cancelled.
reg32 controlsCancelKey(App* app, CPU& cpu, reg32 item, reg32 menu, std::uint16_t key);
// The game's 0x43d410: the reset's question answered: Yes puts the controls to their defaults.
reg32 onResetAnswered(App* app, CPU& cpu, reg32 answer, reg32 close);
// The game's 0x43d460: control1's reset button: a question first.
reg32 onResetClicked(App* app, CPU& cpu);
// The game's 0x43d4d0: control1 as it opens: the controls kept, the controllers' pictures found.
reg32 onControlsEnter(App* app, CPU& cpu, reg32 menu);
// The game's 0x43d5f0: control1 as it closes: Cancel puts the controls back, else they are applied.
reg32 onControlsExit(App* app, CPU& cpu);
// The game's 0x43d9a0: deadzone's list of controllers, one entry for each there is.
reg32 fillControllerList(App* app, CPU& cpu);
// The game's 0x442db0: deadzone's Cancel: Enter marks the ends not kept.
reg32 deadZoneCancelKey(App* app, CPU& cpu, reg32 item, reg32 menu, std::uint16_t key);
// The game's 0x442dd0: an axis's two ends from its slider's to the controller's record, each applied when it
// changed.
reg32 applyAxis(App* app, CPU& cpu, float first, float second, reg32 controller, reg32 axis);
// The game's 0x442e60: which axis a slider is, 0 for none.
reg32 axisIndex(App* app, CPU& cpu, reg32 item);
// The game's 0x442ef0: an axis's slider moved: its ends kept a tenth apart, and applied.
reg32 onAxisMoved(App* app, CPU& cpu, reg32 item);
// The game's 0x442ff0: an axis's slider drawn, its third end where the controller stands now.
reg32 onAxisDraw(App* app, CPU& cpu, reg32 item, reg32 selected);
// The game's 0x443070: a controller's axes shown, a slider for each it has.
reg32 showController(App* app, CPU& cpu, reg32 menu, reg32 controller);
// The game's 0x443260: DZ_JOYSEL's key in the menus: another controller chosen, its axes shown.
reg32 deadZoneChooseKey(App* app, CPU& cpu, reg32 item, reg32 menu, std::uint16_t key);
// The game's 0x443290: DZ_JOYSEL's key in a race, likewise.
reg32 deadZoneChooseKeyInRace(App* app, CPU& cpu, reg32 item, reg32 menu, std::uint16_t key);
// The game's 0x4432c0: deadzone as it opens (psdeadz too): the ends kept, the first controller shown.
reg32 onDeadZoneEnter(App* app, CPU& cpu, reg32 menu);
// The game's 0x443430: deadzone as it closes: after Cancel the ends go back and are applied.
reg32 onDeadZoneExit(App* app, CPU& cpu);
// The game's 0x447b40: force's Cancel: Enter marks the screen cancelled.
reg32 forceCancelKey(App* app, CPU& cpu, reg32 item, reg32 menu, std::uint16_t key);
// The game's 0x447b60: force feedback's strength moved, the other sliders' bars the brighter.
reg32 onStrengthMoved(App* app, CPU& cpu, reg32 item);
// The game's 0x447c40: the road effects' slider moved.
reg32 forceRoadEffectsMoved(App* app, CPU& cpu, reg32 item);
// The game's 0x447c70: the collision's slider moved.
reg32 forceCollisionMoved(App* app, CPU& cpu, reg32 item);
// The game's 0x447ca0: the engine's slider moved.
reg32 forceEngineMoved(App* app, CPU& cpu, reg32 item);
// The game's 0x447cd0: body roll moved: from 0, road grip goes to 0.
reg32 forceBodyRollMoved(App* app, CPU& cpu, reg32 item);
// The game's 0x447d50: road grip moved: from 0, body roll goes to 0.
reg32 forceRoadGripMoved(App* app, CPU& cpu, reg32 item);
// The game's 0x447dd0: force as it opens (psforce too): the sliders set from the settings, kept.
reg32 onForceEnter(App* app, CPU& cpu, reg32 menu);
// The game's 0x4481c0: force as it closes: Cancel puts the settings back, the strength applied.
reg32 onForceExit(App* app, CPU& cpu);
// The game's 0x448440: graphics as it opens: the modes listed, the settings kept.
reg32 onGraphicsEnter(App* app, CPU& cpu, reg32 menu);
// The game's 0x4484e0: graphics as it closes: Cancel puts the settings back, else the mode is taken.
reg32 onGraphicsExit(App* app, CPU& cpu, reg32 menu);
// The game's 0x448570: the effects' intensity moved, while it is shown.
reg32 onEffectsIntensityMoved(App* app, CPU& cpu, reg32 item);
// The game's 0x4485a0: advgrph on every pass: projected text only where it is allowed.
reg32 onAdvancedFrame(App* app, CPU& cpu, reg32 menu);
// The game's 0x448610: advgrph as it opens: what the display and the mode cannot do greyed out.
reg32 onAdvancedEnter(App* app, CPU& cpu, reg32 menu);
// The game's 0x448770: advgrph as it closes: Cancel puts the settings back.
reg32 onAdvancedExit(App* app, CPU& cpu, reg32 menu);

}  // namespace options
}  // namespace frontend

namespace guest
{

// The screen in eax.
template <>
struct ByAddress<frontend::options::onOptionsEnter>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 menu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ menu }));
    }
};

template <>
struct ByAddress<frontend::options::separateFeeds>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The item in eax, the screen in edx, the key in ebx.
template <>
struct ByAddress<frontend::options::feedTimeButtonKey>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu,
                           x86::reg32 item, x86::reg32 menu, std::uint16_t key)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item, menu, key }));
    }
};

// The item in eax, the screen in edx, the key in ebx.
template <>
struct ByAddress<frontend::options::feedTimeListKey>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu,
                           x86::reg32 item, x86::reg32 menu, std::uint16_t key)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item, menu, key }));
    }
};

// The item in eax, the screen in edx, the key in ebx.
template <>
struct ByAddress<frontend::options::feedPositionButtonKey>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu,
                           x86::reg32 item, x86::reg32 menu, std::uint16_t key)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item, menu, key }));
    }
};

// The item in eax, the screen in edx, the key in ebx.
template <>
struct ByAddress<frontend::options::feedPositionListKey>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu,
                           x86::reg32 item, x86::reg32 menu, std::uint16_t key)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item, menu, key }));
    }
};

// The item in eax, the screen in edx, the key in ebx.
template <>
struct ByAddress<frontend::options::feedSpeedButtonKey>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu,
                           x86::reg32 item, x86::reg32 menu, std::uint16_t key)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item, menu, key }));
    }
};

// The item in eax, the screen in edx, the key in ebx.
template <>
struct ByAddress<frontend::options::feedSpeedListKey>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu,
                           x86::reg32 item, x86::reg32 menu, std::uint16_t key)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item, menu, key }));
    }
};

// The screen in eax.
template <>
struct ByAddress<frontend::options::onFeedEnter>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 menu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ menu }));
    }
};

// The screen in eax.
template <>
struct ByAddress<frontend::options::onFeedExit>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 menu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ menu }));
    }
};

// The screen in eax.
template <>
struct ByAddress<frontend::options::funkeysEnter>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 menu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ menu }));
    }
};

// The screen in eax.
template <>
struct ByAddress<frontend::options::hardkeysEnter>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 menu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ menu }));
    }
};

// The item in eax.
template <>
struct ByAddress<frontend::options::audioMenuMusicMoved>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

// The item in eax.
template <>
struct ByAddress<frontend::options::audioRaceMusicMoved>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

// The item in eax.
template <>
struct ByAddress<frontend::options::audioEffectsMoved>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

// The item in eax.
template <>
struct ByAddress<frontend::options::audioEngineMoved>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

// The item in eax.
template <>
struct ByAddress<frontend::options::audioSpeechMoved>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

// The item in eax, 1 when selected in edx.
template <>
struct ByAddress<frontend::options::onVolumeSliderDraw>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item, x86::reg32 selected)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item, selected }));
    }
};

// The screen in eax.
template <>
struct ByAddress<frontend::options::onAudioEnter>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 menu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ menu }));
    }
};

// The screen in eax.
template <>
struct ByAddress<frontend::options::onAudioExit>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 menu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ menu }));
    }
};

template <>
struct ByAddress<frontend::options::onAudioFrame>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The item in eax, the screen in edx, the key in ebx.
template <>
struct ByAddress<frontend::options::controlsCancelKey>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu,
                           x86::reg32 item, x86::reg32 menu, std::uint16_t key)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item, menu, key }));
    }
};

// The answer in eax, where to say whether it closes in edx.
template <>
struct ByAddress<frontend::options::onResetAnswered>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 answer, x86::reg32 close)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ answer, close }));
    }
};

template <>
struct ByAddress<frontend::options::onResetClicked>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The screen in eax.
template <>
struct ByAddress<frontend::options::onControlsEnter>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 menu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ menu }));
    }
};

template <>
struct ByAddress<frontend::options::onControlsExit>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::options::fillControllerList>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The item in eax, the screen in edx, the key in ebx.
template <>
struct ByAddress<frontend::options::deadZoneCancelKey>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu,
                           x86::reg32 item, x86::reg32 menu, std::uint16_t key)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item, menu, key }));
    }
};

// The first end, the second end, the controller, the axis on the stack.
template <>
struct ByAddress<frontend::options::applyAxis>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu,
                           float first, float second, x86::reg32 controller, x86::reg32 axis)
    {
        x86::reg32 firstBits;
        std::memcpy(&firstBits, &first, sizeof firstBits);
        x86::reg32 secondBits;
        std::memcpy(&secondBits, &second, sizeof secondBits);
        return fe::callByAddress(app, cpu, Address, fe::args({}, { firstBits, secondBits, controller, axis }));
    }
};

// The item in eax.
template <>
struct ByAddress<frontend::options::axisIndex>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

// The item in eax.
template <>
struct ByAddress<frontend::options::onAxisMoved>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

// The item in eax, 1 when selected in edx.
template <>
struct ByAddress<frontend::options::onAxisDraw>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item, x86::reg32 selected)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item, selected }));
    }
};

// The screen in eax, the controller in edx.
template <>
struct ByAddress<frontend::options::showController>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 menu, x86::reg32 controller)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ menu, controller }));
    }
};

// The item in eax, the screen in edx, the key in ebx.
template <>
struct ByAddress<frontend::options::deadZoneChooseKey>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu,
                           x86::reg32 item, x86::reg32 menu, std::uint16_t key)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item, menu, key }));
    }
};

// The item in eax, the screen in edx, the key in ebx.
template <>
struct ByAddress<frontend::options::deadZoneChooseKeyInRace>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu,
                           x86::reg32 item, x86::reg32 menu, std::uint16_t key)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item, menu, key }));
    }
};

// The screen in eax.
template <>
struct ByAddress<frontend::options::onDeadZoneEnter>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 menu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ menu }));
    }
};

template <>
struct ByAddress<frontend::options::onDeadZoneExit>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The item in eax, the screen in edx, the key in ebx.
template <>
struct ByAddress<frontend::options::forceCancelKey>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu,
                           x86::reg32 item, x86::reg32 menu, std::uint16_t key)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item, menu, key }));
    }
};

// The item in eax.
template <>
struct ByAddress<frontend::options::onStrengthMoved>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

// The item in eax.
template <>
struct ByAddress<frontend::options::forceRoadEffectsMoved>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

// The item in eax.
template <>
struct ByAddress<frontend::options::forceCollisionMoved>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

// The item in eax.
template <>
struct ByAddress<frontend::options::forceEngineMoved>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

// The item in eax.
template <>
struct ByAddress<frontend::options::forceBodyRollMoved>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

// The item in eax.
template <>
struct ByAddress<frontend::options::forceRoadGripMoved>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

// The screen in eax.
template <>
struct ByAddress<frontend::options::onForceEnter>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 menu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ menu }));
    }
};

template <>
struct ByAddress<frontend::options::onForceExit>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The screen in eax.
template <>
struct ByAddress<frontend::options::onGraphicsEnter>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 menu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ menu }));
    }
};

// The screen in eax.
template <>
struct ByAddress<frontend::options::onGraphicsExit>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 menu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ menu }));
    }
};

// The item in eax.
template <>
struct ByAddress<frontend::options::onEffectsIntensityMoved>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

// The screen in eax.
template <>
struct ByAddress<frontend::options::onAdvancedFrame>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 menu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ menu }));
    }
};

// The screen in eax.
template <>
struct ByAddress<frontend::options::onAdvancedEnter>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 menu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ menu }));
    }
};

// The screen in eax.
template <>
struct ByAddress<frontend::options::onAdvancedExit>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 menu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ menu }));
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
