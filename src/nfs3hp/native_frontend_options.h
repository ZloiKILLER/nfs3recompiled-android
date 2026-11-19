#ifndef NFS3HP_NATIVE_FRONTEND_OPTIONS_H
#define NFS3HP_NATIVE_FRONTEND_OPTIONS_H

#include "native_frontend.h"
#include "native_guest.h"
#include <cstring>

/* native_frontend_options.cpp's typed functions (the options screens: options, audio, audfeed,
 * funkeys, hardkeys, control1, deadzone, force, graphics, advgrph), which other files call as C++
 * (guest::direct), and how the game's code calls each by address.  Each calls the game's other functions by address on `ts` and throws
 * fe::Terminated when the game stopped in one, as a call by address does; its result is what the game's
 * function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace options
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x442aa0: options as it opens: no sound screen without sound, no loading nor credits in a network
// race.
u32 onOptionsEnter(App* app, ThreadState& ts, u32 menu);
// The game's 0x437250: the three audio feeds on keys of their own, the time first.
u32 separateFeeds(App* app, ThreadState& ts);
// The game's 0x437300: the time feed's button: Enter moves it on to a key no other feed has.
u32 feedTimeButtonKey(App* app, ThreadState& ts, u32 item, u32 menu, std::uint16_t key);
// The game's 0x437350: the time feed's list: a key another feed has is not taken.
u32 feedTimeListKey(App* app, ThreadState& ts, u32 item, u32 menu, std::uint16_t key);
// The game's 0x437390: the position feed's button, likewise.
u32 feedPositionButtonKey(App* app, ThreadState& ts, u32 item, u32 menu, std::uint16_t key);
// The game's 0x4373e0: the position feed's list, likewise.
u32 feedPositionListKey(App* app, ThreadState& ts, u32 item, u32 menu, std::uint16_t key);
// The game's 0x437420: the speed feed's button, likewise.
u32 feedSpeedButtonKey(App* app, ThreadState& ts, u32 item, u32 menu, std::uint16_t key);
// The game's 0x437470: the speed feed's list, likewise.
u32 feedSpeedListKey(App* app, ThreadState& ts, u32 item, u32 menu, std::uint16_t key);
// The game's 0x4374b0: audfeed as it opens: the feeds kept for Cancel, each item its key handler.
u32 onFeedEnter(App* app, ThreadState& ts, u32 menu);
// The game's 0x437590: audfeed as it closes: Cancel puts the feeds back, anything else separates them.
u32 onFeedExit(App* app, ThreadState& ts, u32 menu);
// The game's 0x437600: funkeys as it opens: its lines of text placed in rows.
u32 funkeysEnter(App* app, ThreadState& ts, u32 menu);
// The game's 0x4377d0: hardkeys as it opens, as funkeys.
u32 hardkeysEnter(App* app, ThreadState& ts, u32 menu);
// The game's 0x437990: the menu music's volume slider moved.
u32 audioMenuMusicMoved(App* app, ThreadState& ts, u32 item);
// The game's 0x4379b0: the race music's volume slider moved.
u32 audioRaceMusicMoved(App* app, ThreadState& ts, u32 item);
// The game's 0x4379d0: the effects' volume slider moved, the race's own in a race.
u32 audioEffectsMoved(App* app, ThreadState& ts, u32 item);
// The game's 0x437a20: the engine's volume slider moved, the race's own in a race.
u32 audioEngineMoved(App* app, ThreadState& ts, u32 item);
// The game's 0x437a70: the speech's volume slider moved.
u32 audioSpeechMoved(App* app, ThreadState& ts, u32 item);
// The game's 0x437a90: a volume slider drawn; selected, it plays at its volume.
u32 onVolumeSliderDraw(App* app, ThreadState& ts, u32 item, u32 selected);
// The game's 0x437e00: audio as it opens (psaudio too): the settings kept, the sliders set.
u32 onAudioEnter(App* app, ThreadState& ts, u32 menu);
// The game's 0x438070: audio as it closes: the race's volumes back, Cancel puts back what was kept.
u32 onAudioExit(App* app, ThreadState& ts, u32 menu);
// The game's 0x438260: audio on every pass: a change of the setup starts the sound over.
u32 onAudioFrame(App* app, ThreadState& ts);
// The game's 0x43d3f0: control1's Cancel: Enter marks the screen cancelled.
u32 controlsCancelKey(App* app, ThreadState& ts, u32 item, u32 menu, std::uint16_t key);
// The game's 0x43d410: the reset's question answered: Yes puts the controls to their defaults.
u32 onResetAnswered(App* app, ThreadState& ts, u32 answer, u32 close);
// The game's 0x43d460: control1's reset button: a question first.
u32 onResetClicked(App* app, ThreadState& ts);
// The game's 0x43d4d0: control1 as it opens: the controls kept, the controllers' pictures found.
u32 onControlsEnter(App* app, ThreadState& ts, u32 menu);
// The game's 0x43d5f0: control1 as it closes: Cancel puts the controls back, else they are applied.
u32 onControlsExit(App* app, ThreadState& ts);
// The game's 0x43d9a0: deadzone's list of controllers, one entry for each there is.
u32 fillControllerList(App* app, ThreadState& ts);
// The game's 0x442db0: deadzone's Cancel: Enter marks the ends not kept.
u32 deadZoneCancelKey(App* app, ThreadState& ts, u32 item, u32 menu, std::uint16_t key);
// The game's 0x442dd0: an axis's two ends from its slider's to the controller's record, each applied when it
// changed.
u32 applyAxis(App* app, ThreadState& ts, float first, float second, u32 controller, u32 axis);
// The game's 0x442e60: which axis a slider is, 0 for none.
u32 axisIndex(App* app, ThreadState& ts, u32 item);
// The game's 0x442ef0: an axis's slider moved: its ends kept a tenth apart, and applied.
u32 onAxisMoved(App* app, ThreadState& ts, u32 item);
// The game's 0x442ff0: an axis's slider drawn, its third end where the controller stands now.
u32 onAxisDraw(App* app, ThreadState& ts, u32 item, u32 selected);
// The game's 0x443070: a controller's axes shown, a slider for each it has.
u32 showController(App* app, ThreadState& ts, u32 menu, u32 controller);
// The game's 0x443260: DZ_JOYSEL's key in the menus: another controller chosen, its axes shown.
u32 deadZoneChooseKey(App* app, ThreadState& ts, u32 item, u32 menu, std::uint16_t key);
// The game's 0x443290: DZ_JOYSEL's key in a race, likewise.
u32 deadZoneChooseKeyInRace(App* app, ThreadState& ts, u32 item, u32 menu, std::uint16_t key);
// The game's 0x4432c0: deadzone as it opens (psdeadz too): the ends kept, the first controller shown.
u32 onDeadZoneEnter(App* app, ThreadState& ts, u32 menu);
// The game's 0x443430: deadzone as it closes: after Cancel the ends go back and are applied.
u32 onDeadZoneExit(App* app, ThreadState& ts);
// The game's 0x447b40: force's Cancel: Enter marks the screen cancelled.
u32 forceCancelKey(App* app, ThreadState& ts, u32 item, u32 menu, std::uint16_t key);
// The game's 0x447b60: force feedback's strength moved, the other sliders' bars the brighter.
u32 onStrengthMoved(App* app, ThreadState& ts, u32 item);
// The game's 0x447c40: the road effects' slider moved.
u32 forceRoadEffectsMoved(App* app, ThreadState& ts, u32 item);
// The game's 0x447c70: the collision's slider moved.
u32 forceCollisionMoved(App* app, ThreadState& ts, u32 item);
// The game's 0x447ca0: the engine's slider moved.
u32 forceEngineMoved(App* app, ThreadState& ts, u32 item);
// The game's 0x447cd0: body roll moved: from 0, road grip goes to 0.
u32 forceBodyRollMoved(App* app, ThreadState& ts, u32 item);
// The game's 0x447d50: road grip moved: from 0, body roll goes to 0.
u32 forceRoadGripMoved(App* app, ThreadState& ts, u32 item);
// The game's 0x447dd0: force as it opens (psforce too): the sliders set from the settings, kept.
u32 onForceEnter(App* app, ThreadState& ts, u32 menu);
// The game's 0x4481c0: force as it closes: Cancel puts the settings back, the strength applied.
u32 onForceExit(App* app, ThreadState& ts);
// The game's 0x448440: graphics as it opens: the modes listed, the settings kept.
u32 onGraphicsEnter(App* app, ThreadState& ts, u32 menu);
// The game's 0x4484e0: graphics as it closes: Cancel puts the settings back, else the mode is taken.
u32 onGraphicsExit(App* app, ThreadState& ts, u32 menu);
// The game's 0x448570: the effects' intensity moved, while it is shown.
u32 onEffectsIntensityMoved(App* app, ThreadState& ts, u32 item);
// The game's 0x4485a0: advgrph on every pass: projected text only where it is allowed.
u32 onAdvancedFrame(App* app, ThreadState& ts, u32 menu);
// The game's 0x448610: advgrph as it opens: what the display and the mode cannot do greyed out.
u32 onAdvancedEnter(App* app, ThreadState& ts, u32 menu);
// The game's 0x448770: advgrph as it closes: Cancel puts the settings back.
u32 onAdvancedExit(App* app, ThreadState& ts, u32 menu);

}  // namespace options
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
