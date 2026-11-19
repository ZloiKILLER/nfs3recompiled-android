#include "game_settings.h"
#include "native_controls_devices.h"
#include "native_controls_input.h"
#include "native_controls_pipeline.h"
#include "native_controls_queue.h"
#include "native_frontend_eaa.h"
#include "native_game.h"
#include "native_guest.h"

#include <optional>
#include <type_traits>

/* The controls read and turned into the players' actions every frame of a
 * race.  The game's code calls each by the name it has here.
 *
 * A control is a dword as the control sets have it (native_controls_
 * devices.cpp): its device's number in the top four bits, its kind in the low
 * byte -- 0 none, 1 an axis's half, 2 a button, 3 a hat's direction, 4 a key
 * -- and what of the device between.  As a race starts, the players' sets are
 * laid out by action into the race's table at 0x79c340: 12 actions the first
 * player's, 12 the second's (two on one screen), then the 32 function keys';
 * each joystick a control is on goes into the list at 0x5eff20 (device
 * numbers, bytes, up to a negative one).  Every frame each joystick listed is
 * read into its 0x84 bytes from 0x5eff30, and each player's actions made
 * four bytes at 0x79c338 (the second's at 0x79c33c): the steering, -0x7f
 * left to 0x7f right, the throttle, the brake, 0 to 0xff, and a bit each of 8
 * more actions; the function keys' 32 bits go into a ring of 32 at 0x79c2b8.
 * The four bytes are handed on to the race's queue of inputs (0x4c6530, the
 * network's 0x4c60c0), which feeds the cars.
 *
 * Each is a typed function under a name of its own, with a thunk under the
 * game's name for the callers still on registers and the dispatch: it leaves
 * eax as the game's code does -- for some, what the last step left there,
 * which no caller reads -- and every other register as the caller had it.
 * The kinds' functions are called through the code's table at 0x554e34 as the
 * code calls them; the joysticks' reading, the device sets and the queue are
 * their files' typed functions, called as C++ (guest::direct, their headers);
 * the keys' state (0x4eb1c0) and the memory's clearing are called by address,
 * or as the code calls them, their registers kept (EspKept). */
namespace nfs3hp
{
namespace controls
{

namespace
{

using guest::at;
using guest::byte;
using guest::put;
using guest::putByte;
using nfs::u32;
using nfs::u8;
using nfs::s32;
using App = nfs::App;
using ThreadState = nfs::ThreadState;

// The joysticks read: their list (bytes, up to a negative one), each one's
// 0x84 bytes -- a DIJOYSTATE, x at 0, y at 4, the buttons' bytes at 0x30;
// its axes at 0x50, its buttons as bits at 0x70, its hats at 0x74.
constexpr u32 kPolled = 0x5eff20;
constexpr u32 kJoysticks = 0x5eff30;
constexpr u32 kJoystickBytes = 0x84;
constexpr u32 kAllJoysticksBytes = 0x840;
constexpr u32 kJoystickButtons = 0x5eff60, kJoystickAxes = 0x5eff80, kJoystickHats = 0x5effa4;
constexpr u32 kJoystickX = 0, kJoystickY = 4, kJoystickHeld = 0x70;
// A joystick's x or y off its middle, as the menus' arrows take it.
constexpr s32 kJoystickLow = 0x7f, kJoystickHigh = 0x17d;
// A joystick's arrows: the keys a joystick works when the race's menus are
// up -- 4 left, 8 right, 1 up, 2 down, 0x10 a button (Enter).
constexpr u32 kJoystickArrows = 0x5f0770;
constexpr u8 kArrowUp = 1, kArrowDown = 2, kArrowLeft = 4, kArrowRight = 8, kArrowEnter = 0x10;
constexpr u32 kLeftKey = 0x4b, kRightKey = 0x4d, kUpKey = 0x48, kDownKey = 0x50, kEnterKey = 0x1c;
constexpr u32 kTabKey = 0xf;
// The keys' state, a byte each by scan code; the one keyHeld clears.
constexpr u32 kKeys = 0x564818;
constexpr u32 kClearedKey = 0x56484f;
// The kinds' functions, by a control's kind.
constexpr u32 kKinds = 0x554e34;
constexpr u32 kNone = 0, kAxis = 1, kKey = 4;
// What the kinds' functions answer: held, or how far, 0 to 0xff.
constexpr u32 kHeld = 0xff, kHalfway = 0x80;
// The race's actions: each slot a control, from 0x79c340 -- the first
// player's 12, the second's 12, the 32 function keys' from 0x79c3a0 -- and
// the players' four bytes each from 0x79c338.
constexpr u32 kSlots = 0x79c340, kSlotsBytes = 0x160;
constexpr u32 kPlayerSlots = 12, kBitActions = 8;
constexpr u32 kActions = 0x79c338, kSecondActions = 0x79c33c;
constexpr u32 kFunctionKeys = 0x79c3a0, kFunctionKeyCount = 0x20;
constexpr u32 kFunctionKeyRing = 0x79c2b8, kFunctionKeyFrame = 0x79c4ac, kRingMask = 0x1f;
constexpr u32 kHeldBefore = 0x79c1b0, kHeldBeforeBytes = 8;
constexpr u32 kFunctionKeyRingBytes = 0x80, kOtherRing = 0x79c238;
// Three function keys' slots that are number keys chosen in the options.
constexpr u32 kNumberSlots[] = {0x79c3a8, 0x79c3ac, 0x79c3b0};
// A player's set: the steering's halves, then the rest by offset.
constexpr u32 kSetSteerLeft = 4, kSetSteerRight = 0, kSecondSet = 0x34;
// The race.
constexpr u32 kOneMode = 0, kSplitMode = 1;
constexpr u32 kRaceMenu = 0x7d36a0;        // nonzero: the race's menus up
constexpr u32 kRaceMenuToo = 0x7d36a8;
constexpr u32 kReplayFrames = 0x560104;
constexpr u32 kQueueStopped = 0x560108;
constexpr s32 kReplayShown = 2;
/* `bytes` at `from` cleared (0x4e070c); where the clearing stopped, as it
 * leaves eax.  nullopt when the game stopped. */
std::optional<u32> zeroFill(App* app, ThreadState& ts, u32 from, u32 bytes)
{
    const u32 end = guest::direct<0x4e070c, ea::clearBytes>(app, ts, from, bytes);
    if (ts.terminate)
        return std::nullopt;
    return end;
}

/* A native_frontend_* file's typed core called as C++ (guest::direct): nothing (false) when the
 * game stopped in it (its calls into the game throw fe::Terminated), every register but eax kept.
 * A u32 result is the eax the function leaves, set to it; after any other, eax is left alone. */
template <u32 Address, auto Core, typename... Args>
auto feCore(App* app, ThreadState& ts, Args... args)
{
    using Result = decltype(guest::direct<Address, Core>(app, ts, args...));
    if constexpr (std::is_void_v<Result>)
    {
        try
        {
            guest::direct<Address, Core>(app, ts, args...);
        }
        catch (const fe::Terminated&)
        {
            return false;
        }
        return !ts.terminate;
    }
    else
    {
        try
        {
            const Result result = guest::direct<Address, Core>(app, ts, args...);
            return ts.terminate ? std::optional<Result>() : std::optional<Result>(result);
        }
        catch (const fe::Terminated&)
        {
            return std::optional<Result>();
        }
    }
}

/* Key `key`'s state (eaa's, 0x4eb1c0), nonzero held. */
std::optional<u32> keyState(App* app, ThreadState& ts, u32 key)
{
    return feCore<0x4eb1c0, frontend::eaa::keyStateByte>(app, ts, key);
}

/* movsx of a control's low byte: its kind. */
u32 kindOf(u32 control)
{
    return u32(s32(control << 24) >> 24);
}

/* Control `control` asked of kind `kind`'s function, through the code's
 * table at 0x554e34 (indexed as the code indexes it, whatever the kind is):
 * what it answers, held 0xff or how far 0 to 0xff. */
std::optional<u32> askKind(App* app, ThreadState& ts, u32 kind, u32 control)
{
    nfs::Args registers;
    registers.eax = control;
    return guest::pointerCall(app, ts, at(app, kKinds + kind * 4), registers);
}

/* The same, of the control's own kind. */
std::optional<u32> askKind(App* app, ThreadState& ts, u32 control)
{
    return askKind(app, ts, kindOf(control), control);
}

/* A control's device: its top four bits. */
u32 deviceOf(u32 control)
{
    return control >> 28;
}

/* pop ebp, pop edx, ret: the end controlHeld's entries share. */
nfs::u32 popEbpEdx(App* app, ThreadState& ts, nfs::Args& args)
{
    args.ebp = at(app, ts.sp);
    args.edx = at(app, ts.sp + 4);
    ts.sp += 0xc;
    return args.eax;
}

/* 0x431580: no joystick read: their states cleared, their list empty.  Where
 * the clearing stopped, as the code leaves eax. */
std::optional<u32> forgetJoysticks(App* app, ThreadState& ts)
{
    const auto stopped = zeroFill(app, ts, kJoysticks, kAllJoysticksBytes);
    if (!stopped)
        return std::nullopt;
    putByte(app, kPolled, 0xff);
    return stopped;
}

/* 0x4315a0: each joystick listed read into its state (0x4358f0), cleared if
 * it gives none; and, when the race's menus are up or a replay plays and Tab
 * is not held, the first joystick read made the menus' arrows and Enter
 * (0x5f0770): its x or y off its middle, a button held.  A joystick read
 * after the arrows were set adds none.  Returns eax as the code leaves it --
 * what the last joystick's step left there, `unchanged` for none listed --
 * which no caller reads. */
std::optional<u32> readPolledJoysticks(App* app, ThreadState& ts, u32 unchanged)
{
    u32 left = unchanged;
    put(app, kJoystickArrows, 0);
    for (u32 entry = kPolled;; ++entry)
    {
        const u32 number = kindOf(byte(app, entry));  // movsx: a negative byte ends the list
        if (s32(number) <= -1)
            break;
        const auto read =
            guest::direct<0x4358f0, controls::readState>(app, ts, number, kJoysticks + number * kJoystickBytes);
        if (!read)
            return std::nullopt;
        left = *read;
        if (*read == 0)
        {
            // None read: its state cleared, by the number the list has now.
            const u32 again = kindOf(byte(app, entry));
            const auto stopped = zeroFill(app, ts, again * kJoystickBytes + kJoysticks, kJoystickBytes);
            if (!stopped)
                return std::nullopt;
            left = *stopped;
            continue;
        }
        if (at(app, kRaceMenu) == 0 && s32(gameSettings(app).replayMode) < kReplayShown)
            continue;
        if (at(app, kJoystickArrows) != 0)
            continue;
        const auto tab = keyState(app, ts, kTabKey);
        if (!tab)
            return std::nullopt;
        left = *tab;
        if (*tab != 0)
            continue;
        const u32 state = *read;
        const s32 x = s32(at(app, state + kJoystickX));
        left = u32(x);
        if (x < kJoystickLow)
            putByte(app, kJoystickArrows, byte(app, kJoystickArrows) | kArrowLeft);
        else if (x > kJoystickHigh)
            putByte(app, kJoystickArrows, byte(app, kJoystickArrows) | kArrowRight);
        const s32 y = s32(at(app, state + kJoystickY));
        if (y < kJoystickLow)
            putByte(app, kJoystickArrows, byte(app, kJoystickArrows) | kArrowUp);
        else if (y > kJoystickLow)
            putByte(app, kJoystickArrows, byte(app, kJoystickArrows) | kArrowDown);
        if (at(app, state + kJoystickHeld) != 0)
            putByte(app, kJoystickArrows, byte(app, kJoystickArrows) | kArrowEnter);
    }
    return left;
}

/* 0x431690: joystick `number` put on the list of those read, if it is not on
 * it already.  Returns its entry in the list, or the list's end where it went,
 * as the code leaves eax. */
u32 listJoystick(App* app, u32 number)
{
    u32 entry = kPolled;
    for (;; ++entry)
    {
        const u32 listed = kindOf(byte(app, entry));
        if (s32(listed) <= -1 || listed == number)
            break;
    }
    if (s32(kindOf(byte(app, entry))) < 0)
    {
        putByte(app, entry + 1, 0xff);
        putByte(app, entry, u8(number));
    }
    return entry;
}

/* 0x4316c0: 0xff if control's button is held on its joystick, else 0. */
u32 buttonDown(App* app, u32 control)
{
    const u32 button = control << 4 >> 12;
    return byte(app, button + deviceOf(control) * kJoystickBytes + kJoystickButtons) != 0 ? kHeld : 0;
}

/* 0x4316f0: 0xff if control's hat points its direction, else 0. */
u32 hatDirection(App* app, u32 control)
{
    const u32 hat = control << 4 >> 24;
    const u32 direction = control << 12 >> 20;
    return direction == at(app, deviceOf(control) * kJoystickBytes + hat * 4 + kJoystickHats) ? kHeld : 0;
}

/* 0x431740: how far control's axis is into its half, 0 to 0xff: the axis's
 * value over 0x100 from the half's start (bits 16-23) towards its end (bits
 * 8-15) doubled, 0xff past the end, 0 short of the start.  A half starting at
 * 0 gives the value undoubled, as does one ending at 0 from 0xff -- the whole
 * axis, a pedal's. */
u32 axisInto(App* app, u32 control)
{
    const u32 axis = control << 4 >> 28;
    const s32 value = s32(at(app, deviceOf(control) * kJoystickBytes + axis * 4 + kJoystickAxes)) >> 8;
    const s32 start = s32(control << 8 >> 24), end = s32(control << 16 >> 24);
    if (start < end)
    {
        // The half rising from its start to its end.
        if (value < start)
            return 0;
        if (value > end)
            return kHeld;
        if (start == 0)
            return u32(value);
        return (u32(value) - u32(start)) * 2;
    }
    if (start == end)
        return 0;
    // The half falling from its start to its end.
    if (value < end)
        return kHeld;
    if (value > start)
        return 0;
    const u32 into = u32(start) - u32(value);
    if (start == 0xff)
        return into;
    return into + into;
}

/* 0x4317d0: 0xff if control's key is held, else 0.  Held, it clears the
 * state of key 0x37 (the keypad's *, Print Screen's) as it answers --
 * whichever key it is. */
u32 keyDown(App* app, u32 control)
{
    const u32 key = control << 8 >> 16 & 0xff;
    if (byte(app, key + kKeys) == 0)
        return 0;
    putByte(app, kClearedKey, 0);
    return kHeld;
}

/* 0x43188d: whether control, of a kind not a key, is held: an axis more than
 * half into its half, the others what their kind's function says. */
std::optional<u32> heldByKind(App* app, ThreadState& ts, u32 control, u32 kind)
{
    if (kind == kAxis)
        return s32(axisInto(app, control)) <= s32(kHalfway) ? 0 : kHeld;
    return askKind(app, ts, kind, control);
}

/* 0x431800: 0xff if control is held, else 0: a key held, or one of the
 * menus' arrows or Enter a joystick works (0x5f0770); the others by kind. */
std::optional<u32> controlDown(App* app, ThreadState& ts, u32 control)
{
    const u32 kind = kindOf(control);
    if (kind != kKey)
        return heldByKind(app, ts, control, kind);
    const u32 key = control << 8 >> 16;  // the code does not mask it to a byte here
    if (byte(app, key + kKeys) != 0)
        return kHeld;
    if (at(app, kJoystickArrows) == 0)
        return 0;
    const u8 arrows = byte(app, kJoystickArrows);
    if ((key == kLeftKey && (arrows & kArrowLeft)) || (key == kRightKey && (arrows & kArrowRight))
        || (key == kUpKey && (arrows & kArrowUp)) || (key == kDownKey && (arrows & kArrowDown))
        || (key == kEnterKey && (arrows & kArrowEnter)))
        return kHeld;
    return 0;
}

/* 0x48f940: control made action `action`'s in the race's table, and its
 * joystick, if it is on one, read every frame.  Returns eax as the code
 * leaves it: the joystick's entry in the list, else the control's kind. */
u32 bindAction(App* app, u32 action, u32 control)
{
    put(app, action * 4 + kSlots, control);
    const u32 kind = kindOf(control);
    if (kind != kNone && kind != kKey)
        return listJoystick(app, deviceOf(control));
    return kind;
}

/* The rest of a player's set at `set` bound from slot `first`: the
 * throttle, the brake and the rest, two of them into the function keys'
 * slots -- in the code's order, the last two either way round. */
void bindPlayerRest(App* app, u32 first, u32 set, bool lastSwapped)
{
    const u32 slots[] = {2, 3, 0x2d, 0x2e, 6, 7, 8, 9, 5};
    const u32 from[] = {8, 0xc, 0x24, 0x1c, 0x10, 0x20, 0x14, 0x18, 0x28};
    for (int i = 0; i < 9; ++i)
    {
        const u32 slot = slots[i] < 0x2d ? first + slots[i] : slots[i] + (first ? 2 : 0);
        bindAction(app, slot, at(app, set + from[i]));
    }
    if (lastSwapped)
    {
        bindAction(app, first + 0xa, at(app, set + 0x30));
        bindAction(app, first + 0xb, at(app, set + 0x2c));
    }
    else
    {
        bindAction(app, first + 0xb, at(app, set + 0x2c));
        bindAction(app, first + 0xa, at(app, set + 0x30));
    }
}

/* A player's steering at `set` bound from slot `first`: its halves the other
 * way round when the steering is mirrored, in the code's order. */
void bindSteering(App* app, u32 first, u32 set, bool mirrored)
{
    if (mirrored)
    {
        bindAction(app, first + 1, at(app, set + kSetSteerLeft));
        bindAction(app, first, at(app, set + kSetSteerRight));
    }
    else
    {
        bindAction(app, first, at(app, set + kSetSteerLeft));
        bindAction(app, first + 1, at(app, set + kSetSteerRight));
    }
}

/* 0x48f1f0: the race's table of actions (0x79c340) made from the players'
 * control sets (0x4904d0, by the mode): each player's 12 -- the steering's
 * halves the other way round if 0x6fd4c4 says so -- and their 4 function
 * keys; three function keys the number keys the options chose.  Returns the
 * last number key's control, as the code leaves eax. */
std::optional<u32> bindPlayers(App* app, ThreadState& ts)
{
    const u32 set = guest::direct<0x4904d0, controls::controlSets>(app, ts, gameSettings(app).playMode);
    if (ts.terminate)
        return std::nullopt;
    if (gameSettings(app).playMode == kSplitMode)
    {
        bindSteering(app, 0, set, gameSettings(app).mirrored != 0);
        bindPlayerRest(app, 0, set, false);
        // The second player's, 13 slots on.
        bindSteering(app, kPlayerSlots, set + kSecondSet, gameSettings(app).mirrored != 0);
        bindPlayerRest(app, kPlayerSlots, set + kSecondSet, false);
    }
    else
    {
        bindSteering(app, 0, set, gameSettings(app).mirrored != 0);
        bindPlayerRest(app, 0, set, true);
    }
    // The number keys: the options' number less one, 9 for none, the key 2 on (1 to 0).
    u32 control = 0;
    for (int i = 0; i < 3; ++i)
    {
        const u32 number = gameSettings(app).feedKeys[i];
        control = ((number != 0 ? number - 1 : 9) << 8) + 0x204;
        put(app, kNumberSlots[i], control);
    }
    return control;
}

/* 0x48f970: the function keys' own keys (0x79c3a0 on, the first none): Esc,
 * Tab, F3, F5 to F9, F11, F12, the keypad's + and -.  Returns the last key's
 * control written but one, as the code leaves eax. */
u32 bindOwnKeys(App* app)
{
    const u32 keys[][2] = {{0x04, 0x104},  {0x1c, 0x5804}, {0x20, 0x5704}, {0x3c, 0x4e04}, {0x40, 0x4a04},
                             {0x38, 0xf04},  {0x24, 0x3d04}, {0x30, 0x3f04}, {0x34, 0x4004}, {0x2c, 0x4104},
                             {0x50, 0x4204}, {0x48, 0x4304}, {0x00, 0}};
    for (const auto& key : keys)
        put(app, kFunctionKeys + key[0], key[1]);
    return 0x4304;
}

/* 0x48f4f0: the race's controls as it starts: no joystick read, the
 * actions' table, the players' buttons held before, the function keys' ring
 * and another cleared; then the players' controls and the function keys
 * bound.  Returns eax as the code leaves it. */
std::optional<u32> startControls(App* app, ThreadState& ts)
{
    if (!forgetJoysticks(app, ts))
        return std::nullopt;
    const u32 cleared[][2] = {{kSlots, kSlotsBytes},
                                {kHeldBefore, kHeldBeforeBytes},
                                {kFunctionKeyRing, kFunctionKeyRingBytes},
                                {kOtherRing, kFunctionKeyRingBytes}};
    for (const auto& c : cleared)
        if (!zeroFill(app, ts, c[0], c[1]))
            return std::nullopt;
    if (!bindPlayers(app, ts))
        return std::nullopt;
    return bindOwnKeys(app);
}

/* 0x48fa10: the players' four bytes handed on to the race's queue of
 * inputs, which the cars take theirs from a frame at a time: both players'
 * on one screen (the second's only if the first's was taken), this
 * machine's player's alone, or -- in a race over the network -- sent too
 * (0x4c60c0).  Not while the queue is stopped; in a replay only the frames
 * counted, while the race's menus are not up.  Returns eax as the code
 * leaves it: the queue's answer, else `unchanged`. */
std::optional<u32> handOnActions(App* app, ThreadState& ts, u32 unchanged)
{
    if (s32(gameSettings(app).replayMode) > 1)
    {
        if (at(app, kRaceMenu) == 0 && at(app, kRaceMenuToo) == 0)
            put(app, kReplayFrames, at(app, kReplayFrames) + 1);
        return unchanged;
    }
    if (at(app, kQueueStopped) != 0)
        return unchanged;
    // Player's four bytes at `actions` queued: nonzero if they were.
    const auto queued = [&](u32 player, u32 actions) -> std::optional<u32> {
        const u32 answer = guest::direct<0x4c6530, controls::queuePlayerInput>(app, ts, player, actions).answer;
        if (ts.terminate)
            return std::nullopt;
        return answer;
    };
    const u32 mode = gameSettings(app).playMode;
    if (mode == kSplitMode)
    {
        const auto first = queued(0, kActions);
        if (!first || *first == 0)
            return first;
        return queued(1, kSecondActions);
    }
    if (mode == kOneMode)
        return queued(gameSettings(app).localPlayer, kActions);
    return guest::direct<0x4c60c0, controls::sendInput>(app, ts, kActions);
}

}  // namespace

/* 0x48f550: a frame of the race's controls: the joysticks read; each
 * player's four bytes (0x79c338, the second's on) made from the table of
 * actions -- the steering the right half's amount less the left's, halved;
 * the throttle's, the brake's amounts; a bit each of the next 8 held -- then
 * the function keys' 32 bits into their ring; and handed on.  Returns eax as
 * the code leaves it. */
std::optional<u32> readControls(App* app, ThreadState& ts, u32 unchanged)
{
    if (!readPolledJoysticks(app, ts, unchanged))
        return std::nullopt;
    u32 slot = kSlots;
    u32 actions = kActions;
    for (u32 player = 0;;)
    {
        // The steering, the throttle, the brake: each kind's function asked, none's too.
        const auto left = askKind(app, ts, at(app, slot));
        if (!left)
            return std::nullopt;
        const auto right = askKind(app, ts, at(app, slot + 4));
        if (!right)
            return std::nullopt;
        putByte(app, actions, u8(s32(*right - *left) >> 1));
        const auto throttle = askKind(app, ts, at(app, slot + 8));
        if (!throttle)
            return std::nullopt;
        putByte(app, actions + 1, u8(*throttle));
        const auto brake = askKind(app, ts, at(app, slot + 0xc));
        if (!brake)
            return std::nullopt;
        putByte(app, actions + 2, u8(*brake));
        // The next 8 as bits, a control of none skipped.
        slot += 0x10;
        u8 bits = 0;
        for (u32 bit = 0; bit < kBitActions; ++bit, slot += 4)
        {
            const u32 control = at(app, slot);
            if (kindOf(control) == kNone)
                continue;
            const auto held = askKind(app, ts, control);
            if (!held)
                return std::nullopt;
            if (*held != 0)
                bits |= u8(1u << bit);
        }
        actions += 4;
        putByte(app, actions - 1, bits);
        const u32 mode = gameSettings(app).playMode;
        if (mode != kSplitMode)
        {
            slot += kPlayerSlots * 4;
            break;
        }
        player += mode;
        if (s32(player) >= 2)
            break;
    }
    // The function keys, a bit each, the first the lowest.
    u32 bits = 0;
    for (u32 key = 0; key < kFunctionKeyCount; ++key, slot += 4)
    {
        const u32 control = at(app, slot);
        bits >>= 1;
        if (kindOf(control) == kNone)
            continue;
        const auto held = controlDown(app, ts, control);
        if (!held)
            return std::nullopt;
        if (*held != 0)
            bits |= 0x80000000;
    }
    const u32 frame = at(app, kFunctionKeyFrame) + 1;
    put(app, kFunctionKeyFrame, frame);
    put(app, (frame & kRingMask) * 4 + kFunctionKeyRing, bits);
    return handOnActions(app, ts, frame);
}

namespace
{

/* The thunks' end: eax the core's answer, ret. */
void answer(u32& eax, u32& esp, u32 result)
{
    eax = result;
    esp += 4;
}

/* The same, for a core that may stop the game: nothing more when it did. */
void answer(u32& eax, u32& esp, const std::optional<u32>& result)
{
    if (result)
        answer(eax, esp, *result);
}

}  // namespace

nfs::u32 clearJoysticks(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 readJoysticks(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 pollJoystick(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 buttonHeld(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 hatPointing(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 axisAmount(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 keyHeld(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 controlHeld(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 controlHeldNo(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 controlHeldYes(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 controlHeldByKind(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 noControl(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 bindPlayersControls(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 startRaceControls(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 readRaceControls(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 bindControl(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 bindFunctionKeys(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 queueRaceControls(App* app, ThreadState& ts, nfs::Args& args);

/* 0x431580: forgetJoysticks. */
nfs::u32 clearJoysticks(App* app, ThreadState& ts, nfs::Args& args)
{
    answer(args.eax, ts.sp, forgetJoysticks(app, ts));
    return args.eax;
}

/* 0x4315a0: readPolledJoysticks. */
nfs::u32 readJoysticks(App* app, ThreadState& ts, nfs::Args& args)
{
    answer(args.eax, ts.sp, readPolledJoysticks(app, ts, args.eax));
    return args.eax;
}

/* 0x431690: listJoystick for joystick eax. */
nfs::u32 pollJoystick(App* app, ThreadState& ts, nfs::Args& args)
{
    answer(args.eax, ts.sp, listJoystick(app, args.eax));
    return args.eax;
}

/* 0x4316c0: buttonDown for control eax. */
nfs::u32 buttonHeld(App* app, ThreadState& ts, nfs::Args& args)
{
    answer(args.eax, ts.sp, buttonDown(app, args.eax));
    return args.eax;
}

/* 0x4316f0: hatDirection for control eax. */
nfs::u32 hatPointing(App* app, ThreadState& ts, nfs::Args& args)
{
    answer(args.eax, ts.sp, hatDirection(app, args.eax));
    return args.eax;
}

/* 0x431740: axisInto for control eax. */
nfs::u32 axisAmount(App* app, ThreadState& ts, nfs::Args& args)
{
    answer(args.eax, ts.sp, axisInto(app, args.eax));
    return args.eax;
}

/* 0x4317d0: keyDown for control eax. */
nfs::u32 keyHeld(App* app, ThreadState& ts, nfs::Args& args)
{
    answer(args.eax, ts.sp, keyDown(app, args.eax));
    return args.eax;
}

/* 0x431800: controlDown for control eax. */
nfs::u32 controlHeld(App* app, ThreadState& ts, nfs::Args& args)
{
    answer(args.eax, ts.sp, controlDown(app, ts, args.eax));
    return args.eax;
}

/* 0x431880: controlHeld's end, not held: 0, its frame (ebp, edx pushed)
 * left.  An entry of its own in the disassembly, jumped to from
 * controlHeldByKind. */
nfs::u32 controlHeldNo(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = 0;
    popEbpEdx(app, ts, args);
    return args.eax;
}

/* 0x431885: controlHeld's end, held: 0xff, its frame left.  An entry of its
 * own in the disassembly, which nothing jumps to by it. */
nfs::u32 controlHeldYes(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = kHeld;
    popEbpEdx(app, ts, args);
    return args.eax;
}

/* 0x43188d: heldByKind for control eax of kind edx, in controlHeld's frame,
 * which it leaves. */
nfs::u32 controlHeldByKind(App* app, ThreadState& ts, nfs::Args& args)
{
    const auto held = heldByKind(app, ts, args.eax, args.edx);
    if (!held)
        return args.eax;
    args.eax = *held;
    popEbpEdx(app, ts, args);
    return args.eax;
}

/* 0x4318b0: no control: 0. */
nfs::u32 noControl(App* app, ThreadState& ts, nfs::Args& args)
{
    (void)app;
    answer(args.eax, ts.sp, 0);
    return args.eax;
}

/* 0x48f1f0: bindPlayers. */
nfs::u32 bindPlayersControls(App* app, ThreadState& ts, nfs::Args& args)
{
    answer(args.eax, ts.sp, bindPlayers(app, ts));
    return args.eax;
}

/* 0x48f4f0: startControls. */
nfs::u32 startRaceControls(App* app, ThreadState& ts, nfs::Args& args)
{
    answer(args.eax, ts.sp, startControls(app, ts));
    return args.eax;
}

/* 0x48f550: readControls. */
nfs::u32 readRaceControls(App* app, ThreadState& ts, nfs::Args& args)
{
    answer(args.eax, ts.sp, readControls(app, ts, args.eax));
    return args.eax;
}

/* 0x48f940: bindAction of action eax to control edx. */
nfs::u32 bindControl(App* app, ThreadState& ts, nfs::Args& args)
{
    answer(args.eax, ts.sp, bindAction(app, args.eax, args.edx));
    return args.eax;
}

/* 0x48f970: bindOwnKeys. */
nfs::u32 bindFunctionKeys(App* app, ThreadState& ts, nfs::Args& args)
{
    answer(args.eax, ts.sp, bindOwnKeys(app));
    return args.eax;
}

/* 0x48fa10: handOnActions. */
nfs::u32 queueRaceControls(App* app, ThreadState& ts, nfs::Args& args)
{
    answer(args.eax, ts.sp, handOnActions(app, ts, args.eax));
    return args.eax;
}

/* The typed cores of the thunks above, for callers that call them as C++ (native_controls_pipeline.h). */
std::optional<u32> playersBound(App* app, ThreadState& ts)
{
    return bindPlayers(app, ts);
}

std::optional<u32> raceControlsStarted(App* app, ThreadState& ts)
{
    return startControls(app, ts);
}

}  // namespace controls
}  // namespace nfs3hp
