#include "game_settings.h"
#include "native_controls_devices.h"
#include "native_controls_input.h"
#include "native_controls_pipeline.h"
#include "native_controls_queue.h"
#include "native_guest.h"

#include <optional>

/* The controls read and turned into the players' actions every frame of a
 * race.  disassemble_nfs3hp.py binds each address to the function of that
 * name here (CONTROLS_PIPELINE_NATIVES), and the generated code calls it by
 * name.  tools/game_checks holds each against its generated code.
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
 * Each is a typed function under a name of its own, with an adapter under the
 * game's name for the callers still on registers and the dispatch: it leaves
 * eax as the game's code does -- for some, what the last step left there,
 * which no caller reads -- and every other register as the caller had it.
 * The kinds' functions are called through the code's table at 0x554e34 as the
 * code calls them; the joysticks' reading, the device sets and the queue are
 * their files' typed functions, called as C++ (guest::direct, their headers);
 * the keys' state (0x4eb1c0) and the memory's clearing are called by address,
 * or as the code calls them, their registers kept (KeptRegisters). */
namespace nfs3hp
{
void fillZero(win32::WinApplication* app, x86::CPU& cpu);  // 0x4e070c, native_game.cpp

namespace controls
{

namespace
{

using guest::at;
using guest::byte;
using guest::put;
using guest::putByte;
using x86::reg32;
using x86::reg8;
using x86::sreg32;
using App = win32::WinApplication;
using CPU = x86::CPU;

// The joysticks read: their list (bytes, up to a negative one), each one's
// 0x84 bytes -- a DIJOYSTATE, x at 0, y at 4, the buttons' bytes at 0x30;
// its axes at 0x50, its buttons as bits at 0x70, its hats at 0x74.
constexpr reg32 kPolled = 0x5eff20;
constexpr reg32 kJoysticks = 0x5eff30;
constexpr reg32 kJoystickBytes = 0x84;
constexpr reg32 kAllJoysticksBytes = 0x840;
constexpr reg32 kJoystickButtons = 0x5eff60, kJoystickAxes = 0x5eff80, kJoystickHats = 0x5effa4;
constexpr reg32 kJoystickX = 0, kJoystickY = 4, kJoystickHeld = 0x70;
// A joystick's x or y off its middle, as the menus' arrows take it.
constexpr sreg32 kJoystickLow = 0x7f, kJoystickHigh = 0x17d;
// A joystick's arrows: the keys a joystick works when the race's menus are
// up -- 4 left, 8 right, 1 up, 2 down, 0x10 a button (Enter).
constexpr reg32 kJoystickArrows = 0x5f0770;
constexpr reg8 kArrowUp = 1, kArrowDown = 2, kArrowLeft = 4, kArrowRight = 8, kArrowEnter = 0x10;
constexpr reg32 kLeftKey = 0x4b, kRightKey = 0x4d, kUpKey = 0x48, kDownKey = 0x50, kEnterKey = 0x1c;
constexpr reg32 kTabKey = 0xf;
// The keys' state, a byte each by scan code; the one keyHeld clears.
constexpr reg32 kKeys = 0x564818;
constexpr reg32 kClearedKey = 0x56484f;
// The kinds' functions, by a control's kind.
constexpr reg32 kKinds = 0x554e34;
constexpr reg32 kNone = 0, kAxis = 1, kKey = 4;
// What the kinds' functions answer: held, or how far, 0 to 0xff.
constexpr reg32 kHeld = 0xff, kHalfway = 0x80;
// The race's actions: each slot a control, from 0x79c340 -- the first
// player's 12, the second's 12, the 32 function keys' from 0x79c3a0 -- and
// the players' four bytes each from 0x79c338.
constexpr reg32 kSlots = 0x79c340, kSlotsBytes = 0x160;
constexpr reg32 kPlayerSlots = 12, kBitActions = 8;
constexpr reg32 kActions = 0x79c338, kSecondActions = 0x79c33c;
constexpr reg32 kFunctionKeys = 0x79c3a0, kFunctionKeyCount = 0x20;
constexpr reg32 kFunctionKeyRing = 0x79c2b8, kFunctionKeyFrame = 0x79c4ac, kRingMask = 0x1f;
constexpr reg32 kHeldBefore = 0x79c1b0, kHeldBeforeBytes = 8;
constexpr reg32 kFunctionKeyRingBytes = 0x80, kOtherRing = 0x79c238;
// Three function keys' slots that are number keys chosen in the options.
constexpr reg32 kNumberSlots[] = {0x79c3a8, 0x79c3ac, 0x79c3b0};
// A player's set: the steering's halves, then the rest by offset.
constexpr reg32 kSetSteerLeft = 4, kSetSteerRight = 0, kSecondSet = 0x34;
// The race.
constexpr reg32 kOneMode = 0, kSplitMode = 1;
constexpr reg32 kRaceMenu = 0x7d36a0;        // nonzero: the race's menus up
constexpr reg32 kRaceMenuToo = 0x7d36a8;
constexpr reg32 kReplayFrames = 0x560104;
constexpr reg32 kQueueStopped = 0x560108;
constexpr sreg32 kReplayShown = 2;
// What is called by address.
constexpr reg32 kKeyState = 0x4eb1c0;

/* A call as the code makes it: the return address's slot, and esp as the
 * callee leaves it.  False when the game stopped in it. */
bool direct(App* app, CPU& cpu, void (*function)(App*, CPU&))
{
    cpu.esp -= 4;
    function(app, cpu);
    return !cpu.terminate;
}

bool byAddress(App* app, CPU& cpu, reg32 target)
{
    cpu.esp -= 4;
    app->dynamic_call(target, cpu);
    return !cpu.terminate;
}

/* What a function's frame keeps of its caller's across a call: every register but eax. */
class KeptRegisters
{
public:
    explicit KeptRegisters(CPU& cpu)
        : cpu_(cpu), ebx_(cpu.ebx), ecx_(cpu.ecx), edx_(cpu.edx), esi_(cpu.esi), edi_(cpu.edi), ebp_(cpu.ebp), esp_(cpu.esp)
    {
    }
    ~KeptRegisters()
    {
        cpu_.ebx = ebx_;
        cpu_.ecx = ecx_;
        cpu_.edx = edx_;
        cpu_.esi = esi_;
        cpu_.edi = edi_;
        cpu_.ebp = ebp_;
        cpu_.esp = esp_;
    }

private:
    CPU& cpu_;
    reg32 ebx_, ecx_, edx_, esi_, edi_, ebp_, esp_;
};

/* `bytes` at `from` cleared (0x4e070c); where the clearing stopped, as it
 * leaves eax.  nullopt when the game stopped. */
std::optional<reg32> zeroFill(App* app, CPU& cpu, reg32 from, reg32 bytes)
{
    const KeptRegisters kept(cpu);
    cpu.eax = from;
    cpu.edx = bytes;
    if (!direct(app, cpu, fillZero))
        return std::nullopt;
    return cpu.eax;
}

/* Key `key`'s state (0x4eb1c0), nonzero held. */
std::optional<reg32> keyState(App* app, CPU& cpu, reg32 key)
{
    const KeptRegisters kept(cpu);
    cpu.eax = key;
    if (!byAddress(app, cpu, kKeyState))
        return std::nullopt;
    return cpu.eax;
}

/* movsx of a control's low byte: its kind. */
reg32 kindOf(reg32 control)
{
    return reg32(sreg32(control << 24) >> 24);
}

/* Control `control` asked of kind `kind`'s function, through the code's
 * table at 0x554e34 (indexed as the code indexes it, whatever the kind is):
 * what it answers, held 0xff or how far 0 to 0xff. */
std::optional<reg32> askKind(App* app, CPU& cpu, reg32 kind, reg32 control)
{
    const KeptRegisters kept(cpu);
    cpu.eax = control;
    if (!byAddress(app, cpu, at(app, kKinds + kind * 4)))
        return std::nullopt;
    return cpu.eax;
}

/* The same, of the control's own kind. */
std::optional<reg32> askKind(App* app, CPU& cpu, reg32 control)
{
    return askKind(app, cpu, kindOf(control), control);
}

/* A control's device: its top four bits. */
reg32 deviceOf(reg32 control)
{
    return control >> 28;
}

/* pop ebp, pop edx, ret: the end controlHeld's entries share. */
void popEbpEdx(App* app, CPU& cpu)
{
    cpu.ebp = at(app, cpu.esp);
    cpu.edx = at(app, cpu.esp + 4);
    cpu.esp += 0xc;
}

/* 0x431580: no joystick read: their states cleared, their list empty.  Where
 * the clearing stopped, as the code leaves eax. */
std::optional<reg32> forgetJoysticks(App* app, CPU& cpu)
{
    const auto stopped = zeroFill(app, cpu, kJoysticks, kAllJoysticksBytes);
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
std::optional<reg32> readPolledJoysticks(App* app, CPU& cpu, reg32 unchanged)
{
    reg32 left = unchanged;
    put(app, kJoystickArrows, 0);
    for (reg32 entry = kPolled;; ++entry)
    {
        const reg32 number = kindOf(byte(app, entry));  // movsx: a negative byte ends the list
        if (sreg32(number) <= -1)
            break;
        const auto read =
            guest::direct<0x4358f0, controls::readState>(app, cpu, number, kJoysticks + number * kJoystickBytes);
        if (!read)
            return std::nullopt;
        left = *read;
        if (*read == 0)
        {
            // None read: its state cleared, by the number the list has now.
            const reg32 again = kindOf(byte(app, entry));
            const auto stopped = zeroFill(app, cpu, again * kJoystickBytes + kJoysticks, kJoystickBytes);
            if (!stopped)
                return std::nullopt;
            left = *stopped;
            continue;
        }
        if (at(app, kRaceMenu) == 0 && sreg32(gameSettings(app).replayMode) < kReplayShown)
            continue;
        if (at(app, kJoystickArrows) != 0)
            continue;
        const auto tab = keyState(app, cpu, kTabKey);
        if (!tab)
            return std::nullopt;
        left = *tab;
        if (*tab != 0)
            continue;
        const reg32 state = *read;
        const sreg32 x = sreg32(at(app, state + kJoystickX));
        left = reg32(x);
        if (x < kJoystickLow)
            putByte(app, kJoystickArrows, byte(app, kJoystickArrows) | kArrowLeft);
        else if (x > kJoystickHigh)
            putByte(app, kJoystickArrows, byte(app, kJoystickArrows) | kArrowRight);
        const sreg32 y = sreg32(at(app, state + kJoystickY));
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
reg32 listJoystick(App* app, reg32 number)
{
    reg32 entry = kPolled;
    for (;; ++entry)
    {
        const reg32 listed = kindOf(byte(app, entry));
        if (sreg32(listed) <= -1 || listed == number)
            break;
    }
    if (sreg32(kindOf(byte(app, entry))) < 0)
    {
        putByte(app, entry + 1, 0xff);
        putByte(app, entry, reg8(number));
    }
    return entry;
}

/* 0x4316c0: 0xff if control's button is held on its joystick, else 0. */
reg32 buttonDown(App* app, reg32 control)
{
    const reg32 button = control << 4 >> 12;
    return byte(app, button + deviceOf(control) * kJoystickBytes + kJoystickButtons) != 0 ? kHeld : 0;
}

/* 0x4316f0: 0xff if control's hat points its direction, else 0. */
reg32 hatDirection(App* app, reg32 control)
{
    const reg32 hat = control << 4 >> 24;
    const reg32 direction = control << 12 >> 20;
    return direction == at(app, deviceOf(control) * kJoystickBytes + hat * 4 + kJoystickHats) ? kHeld : 0;
}

/* 0x431740: how far control's axis is into its half, 0 to 0xff: the axis's
 * value over 0x100 from the half's start (bits 16-23) towards its end (bits
 * 8-15) doubled, 0xff past the end, 0 short of the start.  A half starting at
 * 0 gives the value undoubled, as does one ending at 0 from 0xff -- the whole
 * axis, a pedal's. */
reg32 axisInto(App* app, reg32 control)
{
    const reg32 axis = control << 4 >> 28;
    const sreg32 value = sreg32(at(app, deviceOf(control) * kJoystickBytes + axis * 4 + kJoystickAxes)) >> 8;
    const sreg32 start = sreg32(control << 8 >> 24), end = sreg32(control << 16 >> 24);
    if (start < end)
    {
        // The half rising from its start to its end.
        if (value < start)
            return 0;
        if (value > end)
            return kHeld;
        if (start == 0)
            return reg32(value);
        return (reg32(value) - reg32(start)) * 2;
    }
    if (start == end)
        return 0;
    // The half falling from its start to its end.
    if (value < end)
        return kHeld;
    if (value > start)
        return 0;
    const reg32 into = reg32(start) - reg32(value);
    if (start == 0xff)
        return into;
    return into + into;
}

/* 0x4317d0: 0xff if control's key is held, else 0.  Held, it clears the
 * state of key 0x37 (the keypad's *, Print Screen's) as it answers --
 * whichever key it is. */
reg32 keyDown(App* app, reg32 control)
{
    const reg32 key = control << 8 >> 16 & 0xff;
    if (byte(app, key + kKeys) == 0)
        return 0;
    putByte(app, kClearedKey, 0);
    return kHeld;
}

/* 0x43188d: whether control, of a kind not a key, is held: an axis more than
 * half into its half, the others what their kind's function says. */
std::optional<reg32> heldByKind(App* app, CPU& cpu, reg32 control, reg32 kind)
{
    if (kind == kAxis)
        return sreg32(axisInto(app, control)) <= sreg32(kHalfway) ? 0 : kHeld;
    return askKind(app, cpu, kind, control);
}

/* 0x431800: 0xff if control is held, else 0: a key held, or one of the
 * menus' arrows or Enter a joystick works (0x5f0770); the others by kind. */
std::optional<reg32> controlDown(App* app, CPU& cpu, reg32 control)
{
    const reg32 kind = kindOf(control);
    if (kind != kKey)
        return heldByKind(app, cpu, control, kind);
    const reg32 key = control << 8 >> 16;  // the code does not mask it to a byte here
    if (byte(app, key + kKeys) != 0)
        return kHeld;
    if (at(app, kJoystickArrows) == 0)
        return 0;
    const reg8 arrows = byte(app, kJoystickArrows);
    if ((key == kLeftKey && (arrows & kArrowLeft)) || (key == kRightKey && (arrows & kArrowRight))
        || (key == kUpKey && (arrows & kArrowUp)) || (key == kDownKey && (arrows & kArrowDown))
        || (key == kEnterKey && (arrows & kArrowEnter)))
        return kHeld;
    return 0;
}

/* 0x48f940: control made action `action`'s in the race's table, and its
 * joystick, if it is on one, read every frame.  Returns eax as the code
 * leaves it: the joystick's entry in the list, else the control's kind. */
reg32 bindAction(App* app, reg32 action, reg32 control)
{
    put(app, action * 4 + kSlots, control);
    const reg32 kind = kindOf(control);
    if (kind != kNone && kind != kKey)
        return listJoystick(app, deviceOf(control));
    return kind;
}

/* The rest of a player's set at `set` bound from slot `first`: the
 * throttle, the brake and the rest, two of them into the function keys'
 * slots -- in the code's order, the last two either way round. */
void bindPlayerRest(App* app, reg32 first, reg32 set, bool lastSwapped)
{
    const reg32 slots[] = {2, 3, 0x2d, 0x2e, 6, 7, 8, 9, 5};
    const reg32 from[] = {8, 0xc, 0x24, 0x1c, 0x10, 0x20, 0x14, 0x18, 0x28};
    for (int i = 0; i < 9; ++i)
    {
        const reg32 slot = slots[i] < 0x2d ? first + slots[i] : slots[i] + (first ? 2 : 0);
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
void bindSteering(App* app, reg32 first, reg32 set, bool mirrored)
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
std::optional<reg32> bindPlayers(App* app, CPU& cpu)
{
    const reg32 set = guest::direct<0x4904d0, controls::controlSets>(app, cpu, gameSettings(app).playMode);
    if (cpu.terminate)
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
    reg32 control = 0;
    for (int i = 0; i < 3; ++i)
    {
        const reg32 number = gameSettings(app).feedKeys[i];
        control = ((number != 0 ? number - 1 : 9) << 8) + 0x204;
        put(app, kNumberSlots[i], control);
    }
    return control;
}

/* 0x48f970: the function keys' own keys (0x79c3a0 on, the first none): Esc,
 * Tab, F3, F5 to F9, F11, F12, the keypad's + and -.  Returns the last key's
 * control written but one, as the code leaves eax. */
reg32 bindOwnKeys(App* app)
{
    const reg32 keys[][2] = {{0x04, 0x104},  {0x1c, 0x5804}, {0x20, 0x5704}, {0x3c, 0x4e04}, {0x40, 0x4a04},
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
std::optional<reg32> startControls(App* app, CPU& cpu)
{
    if (!forgetJoysticks(app, cpu))
        return std::nullopt;
    const reg32 cleared[][2] = {{kSlots, kSlotsBytes},
                                {kHeldBefore, kHeldBeforeBytes},
                                {kFunctionKeyRing, kFunctionKeyRingBytes},
                                {kOtherRing, kFunctionKeyRingBytes}};
    for (const auto& c : cleared)
        if (!zeroFill(app, cpu, c[0], c[1]))
            return std::nullopt;
    if (!bindPlayers(app, cpu))
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
std::optional<reg32> handOnActions(App* app, CPU& cpu, reg32 unchanged)
{
    if (sreg32(gameSettings(app).replayMode) > 1)
    {
        if (at(app, kRaceMenu) == 0 && at(app, kRaceMenuToo) == 0)
            put(app, kReplayFrames, at(app, kReplayFrames) + 1);
        return unchanged;
    }
    if (at(app, kQueueStopped) != 0)
        return unchanged;
    // Player's four bytes at `actions` queued: nonzero if they were.
    const auto queued = [&](reg32 player, reg32 actions) -> std::optional<reg32> {
        const reg32 answer = guest::direct<0x4c6530, controls::queuePlayerInput>(app, cpu, player, actions).answer;
        if (cpu.terminate)
            return std::nullopt;
        return answer;
    };
    const reg32 mode = gameSettings(app).playMode;
    if (mode == kSplitMode)
    {
        const auto first = queued(0, kActions);
        if (!first || *first == 0)
            return first;
        return queued(1, kSecondActions);
    }
    if (mode == kOneMode)
        return queued(gameSettings(app).localPlayer, kActions);
    return guest::direct<0x4c60c0, controls::sendInput>(app, cpu, kActions);
}

}  // namespace

/* 0x48f550: a frame of the race's controls: the joysticks read; each
 * player's four bytes (0x79c338, the second's on) made from the table of
 * actions -- the steering the right half's amount less the left's, halved;
 * the throttle's, the brake's amounts; a bit each of the next 8 held -- then
 * the function keys' 32 bits into their ring; and handed on.  Returns eax as
 * the code leaves it. */
std::optional<reg32> readControls(App* app, CPU& cpu, reg32 unchanged)
{
    if (!readPolledJoysticks(app, cpu, unchanged))
        return std::nullopt;
    reg32 slot = kSlots;
    reg32 actions = kActions;
    for (reg32 player = 0;;)
    {
        // The steering, the throttle, the brake: each kind's function asked, none's too.
        const auto left = askKind(app, cpu, at(app, slot));
        if (!left)
            return std::nullopt;
        const auto right = askKind(app, cpu, at(app, slot + 4));
        if (!right)
            return std::nullopt;
        putByte(app, actions, reg8(sreg32(*right - *left) >> 1));
        const auto throttle = askKind(app, cpu, at(app, slot + 8));
        if (!throttle)
            return std::nullopt;
        putByte(app, actions + 1, reg8(*throttle));
        const auto brake = askKind(app, cpu, at(app, slot + 0xc));
        if (!brake)
            return std::nullopt;
        putByte(app, actions + 2, reg8(*brake));
        // The next 8 as bits, a control of none skipped.
        slot += 0x10;
        reg8 bits = 0;
        for (reg32 bit = 0; bit < kBitActions; ++bit, slot += 4)
        {
            const reg32 control = at(app, slot);
            if (kindOf(control) == kNone)
                continue;
            const auto held = askKind(app, cpu, control);
            if (!held)
                return std::nullopt;
            if (*held != 0)
                bits |= reg8(1u << bit);
        }
        actions += 4;
        putByte(app, actions - 1, bits);
        const reg32 mode = gameSettings(app).playMode;
        if (mode != kSplitMode)
        {
            slot += kPlayerSlots * 4;
            break;
        }
        player += mode;
        if (sreg32(player) >= 2)
            break;
    }
    // The function keys, a bit each, the first the lowest.
    reg32 bits = 0;
    for (reg32 key = 0; key < kFunctionKeyCount; ++key, slot += 4)
    {
        const reg32 control = at(app, slot);
        bits >>= 1;
        if (kindOf(control) == kNone)
            continue;
        const auto held = controlDown(app, cpu, control);
        if (!held)
            return std::nullopt;
        if (*held != 0)
            bits |= 0x80000000;
    }
    const reg32 frame = at(app, kFunctionKeyFrame) + 1;
    put(app, kFunctionKeyFrame, frame);
    put(app, (frame & kRingMask) * 4 + kFunctionKeyRing, bits);
    return handOnActions(app, cpu, frame);
}

namespace
{

/* The adapters' end: eax the core's answer, ret. */
void answer(CPU& cpu, reg32 result)
{
    cpu.eax = result;
    cpu.esp += 4;
}

/* The same, for a core that may stop the game: nothing more when it did. */
void answer(CPU& cpu, const std::optional<reg32>& result)
{
    if (result)
        answer(cpu, *result);
}

}  // namespace

void clearJoysticks(App* app, CPU& cpu);
void readJoysticks(App* app, CPU& cpu);
void pollJoystick(App* app, CPU& cpu);
void buttonHeld(App* app, CPU& cpu);
void hatPointing(App* app, CPU& cpu);
void axisAmount(App* app, CPU& cpu);
void keyHeld(App* app, CPU& cpu);
void controlHeld(App* app, CPU& cpu);
void controlHeldNo(App* app, CPU& cpu);
void controlHeldYes(App* app, CPU& cpu);
void controlHeldByKind(App* app, CPU& cpu);
void noControl(App* app, CPU& cpu);
void bindPlayersControls(App* app, CPU& cpu);
void startRaceControls(App* app, CPU& cpu);
void readRaceControls(App* app, CPU& cpu);
void bindControl(App* app, CPU& cpu);
void bindFunctionKeys(App* app, CPU& cpu);
void queueRaceControls(App* app, CPU& cpu);

/* 0x431580: forgetJoysticks. */
void clearJoysticks(App* app, CPU& cpu)
{
    answer(cpu, forgetJoysticks(app, cpu));
}

/* 0x4315a0: readPolledJoysticks. */
void readJoysticks(App* app, CPU& cpu)
{
    answer(cpu, readPolledJoysticks(app, cpu, cpu.eax));
}

/* 0x431690: listJoystick for joystick eax. */
void pollJoystick(App* app, CPU& cpu)
{
    answer(cpu, listJoystick(app, cpu.eax));
}

/* 0x4316c0: buttonDown for control eax. */
void buttonHeld(App* app, CPU& cpu)
{
    answer(cpu, buttonDown(app, cpu.eax));
}

/* 0x4316f0: hatDirection for control eax. */
void hatPointing(App* app, CPU& cpu)
{
    answer(cpu, hatDirection(app, cpu.eax));
}

/* 0x431740: axisInto for control eax. */
void axisAmount(App* app, CPU& cpu)
{
    answer(cpu, axisInto(app, cpu.eax));
}

/* 0x4317d0: keyDown for control eax. */
void keyHeld(App* app, CPU& cpu)
{
    answer(cpu, keyDown(app, cpu.eax));
}

/* 0x431800: controlDown for control eax. */
void controlHeld(App* app, CPU& cpu)
{
    answer(cpu, controlDown(app, cpu, cpu.eax));
}

/* 0x431880: controlHeld's end, not held: 0, its frame (ebp, edx pushed)
 * left.  An entry of its own in the disassembly, jumped to from
 * controlHeldByKind. */
void controlHeldNo(App* app, CPU& cpu)
{
    cpu.eax = 0;
    popEbpEdx(app, cpu);
}

/* 0x431885: controlHeld's end, held: 0xff, its frame left.  An entry of its
 * own in the disassembly, which nothing jumps to by it. */
void controlHeldYes(App* app, CPU& cpu)
{
    cpu.eax = kHeld;
    popEbpEdx(app, cpu);
}

/* 0x43188d: heldByKind for control eax of kind edx, in controlHeld's frame,
 * which it leaves. */
void controlHeldByKind(App* app, CPU& cpu)
{
    const auto held = heldByKind(app, cpu, cpu.eax, cpu.edx);
    if (!held)
        return;
    cpu.eax = *held;
    popEbpEdx(app, cpu);
}

/* 0x4318b0: no control: 0. */
void noControl(App* app, CPU& cpu)
{
    (void)app;
    answer(cpu, 0);
}

/* 0x48f1f0: bindPlayers. */
void bindPlayersControls(App* app, CPU& cpu)
{
    answer(cpu, bindPlayers(app, cpu));
}

/* 0x48f4f0: startControls. */
void startRaceControls(App* app, CPU& cpu)
{
    answer(cpu, startControls(app, cpu));
}

/* 0x48f550: readControls. */
void readRaceControls(App* app, CPU& cpu)
{
    answer(cpu, readControls(app, cpu, cpu.eax));
}

/* 0x48f940: bindAction of action eax to control edx. */
void bindControl(App* app, CPU& cpu)
{
    answer(cpu, bindAction(app, cpu.eax, cpu.edx));
}

/* 0x48f970: bindOwnKeys. */
void bindFunctionKeys(App* app, CPU& cpu)
{
    answer(cpu, bindOwnKeys(app));
}

/* 0x48fa10: handOnActions. */
void queueRaceControls(App* app, CPU& cpu)
{
    answer(cpu, handOnActions(app, cpu, cpu.eax));
}

}  // namespace controls
}  // namespace nfs3hp
