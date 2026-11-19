#ifndef NFS3HP_NATIVE_FRONTEND_CARS_H
#define NFS3HP_NATIVE_FRONTEND_CARS_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_cars.cpp's typed functions (the car screens: car1, car2, opponent, cedbasic's
 * tuning, the menus' music back and the developer's recorder), which other files call as C++
 * (guest::direct), and how the game's code calls each by address.  Each calls the game's other functions by address on `ts` and throws fe::Terminated
 * when the game stopped in one, as a call by address does; its result is what the game's function leaves
 * in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace cars
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x4104b0: as a car screen opens, the menus' music again if another plays.
u32 menuMusicBack(App* app, ThreadState& ts, u32 eax);
// The game's 0x4267a0: the recorder: the screen saved as the car's next frame, `first` and on.
u32 saveCarFrame(App* app, ThreadState& ts, u32 folder, u32 first);
// The game's 0x4267f0: the recorder: the screen saved as the track's next frame, `first` and on.
u32 saveTrackFrame(App* app, ThreadState& ts, u32 folder, u32 first);
// The game's 0x426870: the recorder, on every pass of car1 and car2: player one's car turning.
u32 recordCar(App* app, ThreadState& ts, u32 eax);
// The game's 0x426b00: the recorder, on every pass of location: the track turning.
u32 recordTrack(App* app, ThreadState& ts, u32 eax);
// The game's 0x426d60: car1 as it opens (menu, the screen).
u32 car1Enter(App* app, ThreadState& ts, u32 menu);
// The game's 0x426ef0: car2 as it opens.
u32 car2Enter(App* app, ThreadState& ts, u32 menu);
// The game's 0x426e20: car1's every pass: the recorder, the showcase and colour buttons.
u32 car1Frame(App* app, ThreadState& ts, u32 menu);
// The game's 0x426f80: car2's every pass, likewise.
u32 car2Frame(App* app, ThreadState& ts, u32 menu);
// The game's 0x426c20: CE_EDITP1 clicked: the tuning, for player one.
u32 car1EditClicked(App* app, ThreadState& ts);
// The game's 0x426c40: CE_EDITP2 clicked: the tuning, for player two.
u32 car2EditClicked(App* app, ThreadState& ts);
// The game's 0x426c60: car1's colour button clicked: the car's next colour.
u32 car1ColourClicked(App* app, ThreadState& ts);
// The game's 0x426ce0: car2's colour button clicked, likewise.
u32 car2ColourClicked(App* app, ThreadState& ts);
// The game's 0x427050: opponent: the showcase and the comparison, for one car.
u32 opponentFitCar(App* app, ThreadState& ts, u32 menu);
// The game's 0x4270c0: opponent: the background and its mask, the opponents' picture or the car's.
u32 opponentFitArt(App* app, ThreadState& ts, u32 menu);
// The game's 0x4272a0: opponent as it opens.
u32 opponentEnter(App* app, ThreadState& ts, u32 menu);
// The game's 0x427190: opponent's every pass: traffic, the number of opponents; anew for its art.
u32 opponentFrame(App* app, ThreadState& ts, u32 menu);
// The game's 0x4272f0: a car's tuning to edit, the player's or the defaults.
u32 loadTuning(App* app, ThreadState& ts, u32 car);
// The game's 0x427370: the tuning edited, to the player's record for a car.
u32 saveTuning(App* app, ThreadState& ts, u32 car);
// The game's 0x4273e0: CE_ENGINETUNE's slider moved (item, the slider).
u32 engineMoved(App* app, ThreadState& ts, u32 item);
// The game's 0x427470: CE_BRAKEBAL's slider moved.
u32 brakesMoved(App* app, ThreadState& ts, u32 item);
// The game's 0x427500: CE_STEERINGSPD's slider moved.
u32 steeringMoved(App* app, ThreadState& ts, u32 item);
// The game's 0x427590: CE_BGRRATS's slider moved.
u32 gearsMoved(App* app, ThreadState& ts, u32 item);
// The game's 0x427620: CE_SUSPENSION's slider moved.
u32 suspensionMoved(App* app, ThreadState& ts, u32 item);
// The game's 0x4276b0: CE_AERODYN's slider moved.
u32 aeroMoved(App* app, ThreadState& ts, u32 item);
// The game's 0x427740: CE_TIRES's slider moved.
u32 tiresMoved(App* app, ThreadState& ts, u32 item);
// The game's 0x4277d0: the sliders set to the settings.
u32 setSliders(App* app, ThreadState& ts, u32 menu);
// The game's 0x4279b0: the sliders as they stand, kept to tell a change by.
u32 rememberSliders(App* app, ThreadState& ts, u32 menu);
// The game's 0x427ac0: whether a slider moved since they were kept, in al.
u32 slidersMoved(App* app, ThreadState& ts, u32 menu);
// The game's 0x427c20: each slider told where its moves go, set to its setting and kept.
u32 bindSliders(App* app, ThreadState& ts, u32 menu);
// The game's 0x427d00: the advanced tuning edited back to the player's own; 0xe when none.
u32 loadAdvanced(App* app, ThreadState& ts);
// The game's 0x427d80: the defaults question answered: Yes puts them back.
u32 defaultsAnswered(App* app, ThreadState& ts, u32 answer, u32 close);
// The game's 0x427e10: CEB_DONE's key (and racenow's, exitprogram's, optionsnow's).
u32 doneKey(App* app, ThreadState& ts, u32 item, u32 menu, u32 key);
// The game's 0x427ea0: CEB_CANCEL's key.
u32 cancelKey(App* app, ThreadState& ts, u32 item, u32 menu, u32 key);
// The game's 0x427f40: CE_SLCT_CAR's key: another car chosen, its tuning loaded.
u32 carListKey(App* app, ThreadState& ts, u32 item, u32 menu, u32 key);
// The game's 0x427fb0: CEB_DEFAULT's key: Enter asks first.
u32 defaultsKey(App* app, ThreadState& ts, u32 item, u32 menu, u32 key);
// The game's 0x428020: CEB_DONE and CEB_CANCEL their keys, both or neither.
u32 bindDoneCancel(App* app, ThreadState& ts, u32 menu);
// The game's 0x428060: cedbasic fitted to whom it is for (1, 2 or 3).
u32 fitTuningButtons(App* app, ThreadState& ts, u32 menu, u32 tuningFor);
// The game's 0x4281e0: cedbasic made ready: whose car, the settings loaded; 1 for no one.
u32 prepareTuning(App* app, ThreadState& ts, u32 menu);
// The game's 0x428360: cedbasic as it opens.
u32 tuningEnter(App* app, ThreadState& ts, u32 menu);
// The game's 0x4283b0: cedbasic as it closes: no one to tune for any more.
u32 tuningExit(App* app, ThreadState& ts);
// The game's 0x4dcaf0: whether two advanced tunings differ, in al.
u32 advancedDiffers(App* app, ThreadState& ts, u32 a, u32 b);
// The game's 0x4dcb60: one advanced tuning copied over another.
u32 copyAdvanced(App* app, ThreadState& ts, u32 to, u32 from);

}  // namespace cars
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
