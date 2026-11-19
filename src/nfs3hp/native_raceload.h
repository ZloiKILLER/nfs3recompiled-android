#ifndef NFS3HP_NATIVE_RACELOAD_H
#define NFS3HP_NATIVE_RACELOAD_H

#include "native_guest.h"

#include <cstdint>

/* native_raceload.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address.  Each may call the
 * game's other functions by address on `ts`, and is false or nullopt when the game stopped in one. */
namespace nfs3hp
{
namespace raceload
{

// The game's 0x42ce10: car's distance into the race (+8) from its laps and node.
void setRaceDistance(nfs::App* app, nfs::u32 car);
// The game's 0x42d160: car's shadow and lights' frame placed; false when it has no shadow.
std::optional<bool> placeShadow(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x42d500: a hook that does nothing.
void noHook(nfs::App* app);
// The game's 0x42d520: another hook that does nothing.
void noHook2(nfs::App* app);
// The game's 0x42d7f0: the message box closed and the message lines with it.
bool closeAllMessages(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x42dd00: the race controls' tick taken from the game's timer, if it was given.
bool stopTick(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x42dd60: whether the corner (the three signs) of other's box is inside box's.
bool cornerIn(nfs::App* app, nfs::u32 box, nfs::u32 other, std::int32_t firstSign,
              std::int32_t secondSign, std::int32_t thirdSign);
// The game's 0x42e010: cornerIn the other way round, box's corner in other's box.
bool cornerInBack(nfs::App* app, nfs::u32 box, nfs::u32 other, std::int32_t firstSign,
                  std::int32_t secondSign, std::int32_t thirdSign);
// The game's 0x42e2c0: cornerIn with the corner from other's axes worked out the other way about.
bool cornerIn2(nfs::App* app, nfs::u32 box, nfs::u32 other, std::int32_t firstSign,
               std::int32_t secondSign, std::int32_t thirdSign);
// The game's 0x42e4f0: cornerInBack worked out as cornerIn2 is.
bool cornerInBack2(nfs::App* app, nfs::u32 box, nfs::u32 other, std::int32_t firstSign,
                   std::int32_t secondSign, std::int32_t thirdSign);
// The game's 0x42ee10: car hits the wall (its normal, its point) at point; false when it moved away.
std::optional<bool> hit(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car, nfs::u32 point,
                        nfs::u32 normal, nfs::u32 wall);
// The game's 0x42efe0: car's box against the wall of normal through point within the float at
// `reach` (the address of the word the code pushes); what hit gave, or 0.
std::optional<nfs::u32> boxHits(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car, nfs::u32 normal,
                                  nfs::u32 point, nfs::u32 reach);
// The game's 0x42f280: the cars a and b's collision at point answered.
std::optional<nfs::u32> respond(nfs::App* app, nfs::ThreadState& ts, nfs::u32 a, nfs::u32 b, nfs::u32 point);

// The game's 0x42c660: the cars' slots cleared; eax as memset leaves it.
std::optional<nfs::u32> raceSlotsCleared(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x42c700: the race's cars loaded; eax as the code leaves it.
std::optional<nfs::u32> raceCarsLoaded(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x42c7f0: the race's time step picked; the third option's byte.
nfs::u32 timeStepPicked(nfs::App* app);
// The game's 0x42c830: the cars' tasks added; eax as the last adding leaves it (`unchanged` with no cars).
std::optional<nfs::u32> carTasksAdded(nfs::App* app, nfs::ThreadState& ts, nfs::u32 unchanged);
// The game's 0x42cab0: the cars' tasks removed; eax as the code leaves it (`unchanged` with no cars).
std::optional<nfs::u32> carTasksRemoved(nfs::App* app, nfs::ThreadState& ts, nfs::u32 unchanged);
// The game's 0x42cc20: the car's lap counted.
void lapsCounted(nfs::App* app, nfs::u32 car);
// The game's 0x42ce70: the race's cars updated; eax as the places' sort leaves it.
std::optional<nfs::u32> raceCarsUpdated(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x42d790: the message box opened and its lines made afresh; eax as the lines' making leaves it.
std::optional<nfs::u32> messagesOpened(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x42d800: the messages cleared; eax as the code leaves it.
std::optional<nfs::u32> messagesCleared(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x42d810: the typed message at `text` sent; eax as the code leaves it.
std::optional<nfs::u32> typedMessageSent(nfs::App* app, nfs::ThreadState& ts, nfs::u32 text);
// The game's 0x42d9b0: the message lines shown; eax as the code leaves it.
std::optional<nfs::u32> messageLinesShown(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x42d7f0: the message box closed and its lines; what the code leaves in eax.
std::optional<nfs::u32> messagesClosed(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x42d7f0 as above, with the eax (`eax`) the caller has at the call, which the box's closing reads.
std::optional<nfs::u32> messagesClosedAs(nfs::App* app, nfs::ThreadState& ts, nfs::u32 eax);
// The game's 0x42dc40: the race controls' thread woken; eax as the code leaves it.
std::optional<nfs::u32> raceControlsWoken(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x42dcb0: the race controls' tick started; eax as the code leaves it (`unchanged`, its eax).
std::optional<nfs::u32> controlsTickStarted(nfs::App* app, nfs::ThreadState& ts, nfs::u32 unchanged);

}  // namespace raceload
}  // namespace nfs3hp

#endif
