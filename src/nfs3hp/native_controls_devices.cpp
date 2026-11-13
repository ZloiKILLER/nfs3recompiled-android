#include "game_settings.h"
#include "native_controls_devices.h"
#include "native_guest.h"

#include <cstdint>
#include <optional>

/* The control sets the game makes for the devices it knows by name.
 * disassemble_nfs3hp.py binds each address to the function of that name here
 * (CONTROLS_DEVICES_NATIVES), and the generated code calls it by name.
 *
 * A control set is 13 dwords, one an action -- steering either way, the
 * pedals, the hand brake, the gears, the view and the rest -- each the
 * device's number in its top four bits and what of it works the action
 * below: an axis's half (0x80ff01 and 0x7f0001 the first axis, 0x180ff01 and
 * 0x17f0001 the second, 0x280ff01 and 0x27f0001 the third), a button (2 +
 * 0x100 times its number), a key.  One player's set is at 0x6fd20c; when two
 * play on one screen, the first's is at 0x6fd240 and the second's after it.
 * The keyboard's defaults, the one player's and the two players' together,
 * are data in the code at 0x490430 and 0x490464 (which the disassembly takes
 * for a function, sub_490430: it is never called).
 *
 * Each is a typed function under a name of its own, with an adapter under the
 * game's name for the callers still on registers and the dispatch: it leaves
 * the registers as the game's code does.  The sets a function makes on the
 * guest's stack stay there, as deep as the code has them (GuestFrame): the
 * copy into the players' sets (0x4ea4f0, the game's memmove: from eax to edx,
 * ebx bytes) is handed their address, and stays the game's code, called by
 * address: what it leaves in eax, edx and ebx is what the adapters leave,
 * which native_ea_memory.h's moveMemory does not give back; a core calling a
 * core makes room for the return address's slot as
 * the call does.  tools/game_checks holds each against its generated code. */
namespace nfs3hp
{
namespace watcom
{
void strstr(win32::WinApplication* app, x86::CPU& cpu);
}

namespace controls
{

namespace
{

using guest::at;
using guest::put;
using x86::reg32;
using x86::reg8;
using App = win32::WinApplication;
using CPU = x86::CPU;

constexpr reg32 kSetBytes = 0x34;             // 13 actions
constexpr reg32 kSplitMode = 1;               // the mode two play on one screen in
// Controllers: 0x88 bytes each from 0x6fc970: its kind (a DirectInput
// joystick's subtype: 2 a joystick, 4 a game pad), its name, its buttons (a
// bit each), whether it has a second axis.
constexpr reg32 kDeviceBytes = 0x88;
constexpr reg32 kJoystick = 2, kGamePad = 4;
// The keyboard's defaults (in the code's bytes).
constexpr reg32 kKeyboardSingle = 0x490430, kKeyboardSplit = 0x490464;
constexpr reg32 kCopy = 0x4ea4f0;             // memmove: from eax to edx, ebx bytes
// What of a controller works an action: below its number's four bits.
constexpr reg32 kFirstHigh = 0x80ff01, kFirstLow = 0x7f0001;    // the first axis's halves
constexpr reg32 kSecondLow = 0x17f0001, kSecondHigh = 0x180ff01;  // the second's
constexpr reg32 kButton0 = 2, kButton1 = 0x102;               // + 0x100 a button
constexpr reg32 kButton2 = 0x202, kButton3 = 0x302;
constexpr reg32 kKey = 0x45001204;                            // a key: 'E' (0x12), whatever the device
// The actions' places in a set: steering, the pedals, the hand brake, the
// gears, the view.
constexpr reg32 kSteerRight = 0, kSteerLeft = 4, kThrottle = 8, kBrake = 0xc, kHandBrake = 0x10;
constexpr reg32 kGearUp = 0x14, kGearDown = 0x18, kView = 0x1c;
// The texts the known controllers' names are looked for by.
constexpr reg32 kSideWinderGamePadName = 0x53bbe0, kGravisGripName = 0x53bc00, kWingManForceName = 0x53bc0c;
constexpr reg32 kCombatStickName = 0x53bc24, kSideWinderProName = 0x53bc3c, kSideWinderWheelName = 0x53bc58;
constexpr reg32 kSteeringWheelName = 0x53bc84;

/* The guest's stack as a function's code has it: `pushed` bytes of saved
 * registers below esp, then `bytes` of locals, esp left below them; esp as it
 * was when it ends.  Its base is where ebp would point. */
class GuestFrame
{
public:
    GuestFrame(CPU& cpu, reg32 pushed, reg32 bytes) : cpu(cpu), esp(cpu.esp), base(cpu.esp - pushed)
    {
        cpu.esp = base - bytes;
    }
    ~GuestFrame() { cpu.esp = esp; }
    reg32 operator-(reg32 offset) const { return base - offset; }

private:
    CPU& cpu;
    const reg32 esp;
    const reg32 base;
};

/* The registers a function's code pushes and pops again, kept to be put back. */
struct Kept
{
    reg32 ebx, ecx, edx, esi, edi, ebp;
    explicit Kept(const CPU& cpu) : ebx(cpu.ebx), ecx(cpu.ecx), edx(cpu.edx), esi(cpu.esi), edi(cpu.edi), ebp(cpu.ebp)
    {
    }
};

const DeviceRecord& device(App* app, reg32 controller)
{
    return gameSettings(app).deviceAt(controller * kDeviceBytes);
}

/* What of controller works an action: its number in the top four bits. */
reg32 ofController(reg32 controller, reg32 code)
{
    return (controller << 28) + code;
}

/* The game's memmove, by address: `bytes` from `from` to `to`; eax, edx and
 * ebx as it leaves them.  False when the game stopped in it. */
bool copy(App* app, CPU& cpu, reg32 from, reg32 to, reg32 bytes)
{
    cpu.eax = from;
    cpu.edx = to;
    cpu.ebx = bytes;
    cpu.esp -= 4;
    app->dynamic_call(kCopy, cpu);
    return !cpu.terminate;
}

}  // namespace

/* 0x4904d0: the players' control sets in `mode`: the two players' on one
 * screen in mode 1, else the one player's. */
reg32 playersSets(reg32 mode)
{
    return mode == kSplitMode ? SETTINGS_ADDRESS(controls[1]) : SETTINGS_ADDRESS(controls[0]);
}

namespace
{

/* 0x4905b0: the control set at `set` made the players' in `mode`: in mode 1
 * the first's (`which` 0), the second's (1) or -- two sets at `set` -- both
 * (another); else the one player's.  False when the game stopped. */
bool makePlayers(App* app, CPU& cpu, reg32 mode, reg32 set, reg32 which)
{
    if (mode != kSplitMode)
        return copy(app, cpu, set, SETTINGS_ADDRESS(controls[0]), kSetBytes);
    if (which == 0)
        return copy(app, cpu, set, SETTINGS_ADDRESS(controls[1]), kSetBytes);
    if (which == 1)
        return copy(app, cpu, set, SETTINGS_ADDRESS(controls[2]), kSetBytes);
    return copy(app, cpu, set, SETTINGS_ADDRESS(controls[1]), 2 * kSetBytes);
}

/* 0x490610: whether `controller`'s name has the text at `text` in it (the
 * game's strstr, native watcom::strstr; edx as it leaves it).  Nothing when
 * the game stopped. */
std::optional<bool> nameHas(App* app, CPU& cpu, reg32 controller, reg32 text)
{
    cpu.eax = controller * kDeviceBytes + SETTINGS_ADDRESS(devices[0].name);
    cpu.edx = text;
    cpu.esp -= 4;
    watcom::strstr(app, cpu);
    if (cpu.terminate)
        return std::nullopt;
    return cpu.eax != 0;
}

/* 0x490c40: the keyboard's controls in `mode`: in mode 1 the two players'
 * defaults for both when `which` is 2, else the one player's for the second
 * player; else the one player's for the one player.  Both defaults are
 * copied from the code's bytes into the frame first, as the code does. */
bool keyboardSets(App* app, CPU& cpu, reg32 mode, reg32 which)
{
    const GuestFrame frame(cpu, 0x14, 0x9c);  // ebx, ecx, esi, edi, ebp pushed
    const reg32 single = frame - kSetBytes, split = frame - 0x9c;
    for (reg32 i = 0; i < kSetBytes; i += 4)
        put(app, single + i, at(app, kKeyboardSingle + i));
    for (reg32 i = 0; i < 2 * kSetBytes; i += 4)
        put(app, split + i, at(app, kKeyboardSplit + i));
    const GuestFrame slot(cpu, 0, 4);
    if (mode != kSplitMode)
        return makePlayers(app, cpu, mode, single, 0);
    if (which == 2)
        return makePlayers(app, cpu, mode, split, which);
    return makePlayers(app, cpu, mode, single, mode);
}

/* A known controller's set: the 13 actions' codes (below the device's four
 * bits; kKey as it is), `controller`'s name to have `name` in it, made the
 * players' in `mode` -- the second's in mode 1 for any controller but the
 * first.  The set is made on the guest's stack before the name is looked at,
 * as the code does.  Whether it was that controller; nothing when the game
 * stopped. */
std::optional<bool> namedSet(App* app, CPU& cpu, reg32 mode, reg32 controller, const reg32 (&codes)[13], reg32 name)
{
    const GuestFrame frame(cpu, 0x10, kSetBytes);  // ebx, ecx, esi, ebp pushed
    const reg32 set = frame - kSetBytes;
    for (reg32 i = 0; i < 13; ++i)
        put(app, set + 4 * i, codes[i] == kKey ? kKey : ofController(controller, codes[i]));
    std::optional<bool> named;
    {
        const GuestFrame slot(cpu, 0, 4);
        named = nameHas(app, cpu, controller, name);
    }
    if (!named || !*named)
        return named;
    const GuestFrame slot(cpu, 0, 4);
    if (!makePlayers(app, cpu, mode, set, controller != 0 ? 1 : 0))
        return std::nullopt;
    return true;
}

const reg32 kSideWinderGamePad[13] = {kFirstHigh, kFirstLow, 0x2, 0x102, 0x202, 0x702, 0x602,
                                      0x302, 0x402, kKey, 0x902, 0x802, 0x502};
const reg32 kGravisGrip[13] = {kFirstHigh, kFirstLow, 0x2, 0x102, 0x202, 0x502, 0x402,
                               0x302, 0x702, kKey, 0x902, 0x802, 0x602};
// The pedals on its third axis's halves.
const reg32 kWingManForce[13] = {kFirstHigh, kFirstLow, 0x27f0001, 0x280ff01, 0x2, 0x102, 0x302,
                                 0x202, 0x802, 0x402, 0x702, 0x602, 0x502};
// The throttle on its second axis.
const reg32 kCombatStick[13] = {kFirstHigh, kFirstLow, kSecondLow, 0x2, kSecondHigh, 0x602, 0x702,
                                0x402, 0x102, 0x302, 0x202, 0x802, 0x502};
const reg32 kSideWinderPro[13] = {kFirstHigh, kFirstLow, 0x702, 0x602, 0x2, 0x202, 0x502,
                                  0x102, 0x802, kKey, 0x902, 0x402, 0x302};
// The pedals on its second axis.
const reg32 kSideWinderWheel[13] = {kFirstHigh, kFirstLow, kSecondLow, kSecondHigh, 0x2, 0x702, 0x602,
                                    0x302, 0x102, kKey, 0x202, 0x402, 0x502};

/* 0x490b30: a "CH Steering Wheel"'s: the keyboard's set, and over it
 * steering and the pedals on its axes -- buttons 2 and 3 for the pedals
 * without a second -- the hand brake and the view on its first two buttons,
 * as it has them, the set then made the players' again.  Its set is found
 * from the players' by the controller's number, the second player's past the
 * one player's when one plays (as the other kinds' are).  Whether it was one;
 * nothing when the game stopped. */
std::optional<bool> steeringWheelSet(App* app, CPU& cpu, reg32 mode, reg32 controller)
{
    const GuestFrame frame(cpu, 0x14, 0);  // ebx, ecx, esi, edi, ebp pushed
    const reg32 sets = playersSets(mode);
    std::optional<bool> named;
    {
        const GuestFrame slot(cpu, 0, 4);
        named = nameHas(app, cpu, controller, kSteeringWheelName);
    }
    if (!named || !*named)
        return named;
    const reg32 set = sets + controller * kSetBytes;
    const reg32 which = controller != 0 ? 1 : 0;
    {
        const GuestFrame slot(cpu, 0, 4);
        if (!keyboardSets(app, cpu, mode, which))
            return std::nullopt;
    }
    if (device(app, controller).axisMask != 0)
    {
        put(app, set + kSteerRight, ofController(controller, kFirstHigh));
        put(app, set + kSteerLeft, ofController(controller, kFirstLow));
    }
    const bool second = device(app, controller).axisMask != 0;
    put(app, set + kThrottle, ofController(controller, second ? kSecondLow : kButton2));
    put(app, set + kBrake, ofController(controller, second ? kSecondHigh : kButton3));
    if (reg8(device(app, controller).buttonMask) & 1)
        put(app, set + kHandBrake, ofController(controller, kButton0));
    if (reg8(device(app, controller).buttonMask) & 2)
        put(app, set + kView, ofController(controller, kButton1));
    const GuestFrame slot(cpu, 0, 4);
    if (!makePlayers(app, cpu, mode, set, which))
        return std::nullopt;
    return true;
}

/* 0x490640: `controller`'s controls in `mode` if the game knows it by its
 * name, the known ones tried in the code's order: whether it did; nothing
 * when the game stopped. */
std::optional<bool> knownSet(App* app, CPU& cpu, reg32 mode, reg32 controller)
{
    const GuestFrame frame(cpu, 0xc, 0);  // ebx, ecx, ebp pushed
    static const struct
    {
        const reg32 (*codes)[13];
        reg32 name;
    } known[] = {{&kSideWinderGamePad, kSideWinderGamePadName}, {&kGravisGrip, kGravisGripName},
                 {&kWingManForce, kWingManForceName},           {&kSideWinderPro, kSideWinderProName},
                 {&kSideWinderWheel, kSideWinderWheelName},     {nullptr, kSteeringWheelName},
                 {&kCombatStick, kCombatStickName}};
    for (const auto& k : known)
    {
        const GuestFrame slot(cpu, 0, 4);
        const std::optional<bool> done = k.codes ? namedSet(app, cpu, mode, controller, *k.codes, k.name)
                                                 : steeringWheelSet(app, cpu, mode, controller);
        if (!done || *done)
            return done;
    }
    return false;
}

/* A controller of a kind: the keyboard's set, and over it, in the players'
 * set itself (found by the controller's number from the players'), steering
 * on its first axis if it has a second, the pedals on its second where the
 * kind has them, its buttons -- those it has -- and a pair for the gears if
 * it has both. */
struct Button
{
    reg32 part;  // which byte of its buttons
    reg8 mask;
    reg32 code;
    reg32 slot;  // the action's place in the set
};
struct Kind
{
    bool pedalsOnAxes;
    Button buttons[8];
    int count;
    int pairAfter;  // the gears' pair after this many buttons
    reg8 pairFirst, pairSecond;
    reg32 pairCode;   // the first's, the second's 0x100 past it
    bool pairInHigh;  // the buttons' byte read into dh (else dl)
};

/* 0x490cb0: a game pad's (kind 4): steering on its first axis, the pedals
 * and the hand brake on buttons 0 to 2, the gears on 3 and 4, the others on
 * 5 to 9. */
const Kind kGamePadKind = {false,
                           {{0, 1, 0x2, 8},
                            {0, 2, 0x102, 0xc},
                            {0, 4, 0x202, 0x10},
                            {0, 0x20, 0x502, 0x1c},
                            {0, 0x40, 0x602, 0x20},
                            {0, 0x80, 0x702, 0x28},
                            {1, 1, 0x802, 0x2c},
                            {1, 2, 0x902, 0x30}},
                           8,
                           3,
                           8,
                           0x10,
                           0x302,
                           true};
/* 0x490e30: a joystick's (kind 2): steering and the pedals on its axes, the
 * hand brake and the view on buttons 0 and 1, the gears on 2 and 3, the
 * others on 4, 7, 8 and 9. */
const Kind kJoystickKind = {true,
                            {{0, 1, 0x2, 0x10},
                             {0, 2, 0x102, 0x1c},
                             {0, 0x10, 0x402, 0x20},
                             {0, 0x80, 0x702, 0x28},
                             {1, 1, 0x802, 0x2c},
                             {1, 2, 0x902, 0x30}},
                            6,
                            2,
                            4,
                            8,
                            0x202,
                            false};
/* 0x490fa0: any other controller's: as a joystick's, the others on buttons 4
 * to 7. */
const Kind kOtherKind = {true,
                         {{0, 1, 0x2, 0x10},
                          {0, 2, 0x102, 0x1c},
                          {0, 0x10, 0x402, 0x20},
                          {0, 0x20, 0x502, 0x28},
                          {0, 0x40, 0x602, 0x2c},
                          {0, 0x80, 0x702, 0x30}},
                         6,
                         2,
                         4,
                         8,
                         0x202,
                         false};

/* The kind's controls of `controller` in `mode`.  False when the game
 * stopped. */
bool kindSet(App* app, CPU& cpu, reg32 mode, reg32 controller, const Kind& kind)
{
    const GuestFrame frame(cpu, 0x10, 0);  // ebx, ecx, esi, ebp pushed
    const reg32 set = playersSets(mode) + controller * kSetBytes;
    {
        const GuestFrame slot(cpu, 0, 4);
        if (!keyboardSets(app, cpu, mode, controller != 0 ? 1 : 0))
            return false;
    }
    if (device(app, controller).axisMask != 0)
    {
        put(app, set + kSteerRight, ofController(controller, kFirstHigh));
        put(app, set + kSteerLeft, ofController(controller, kFirstLow));
    }
    if (kind.pedalsOnAxes && device(app, controller).axisMask != 0)
    {
        put(app, set + kThrottle, ofController(controller, kSecondLow));
        put(app, set + kBrake, ofController(controller, kSecondHigh));
    }
    for (int i = 0; i < kind.count; ++i)
    {
        if (i == kind.pairAfter)
        {
            const reg8 held = reg8(device(app, controller).buttonMask);
            if ((held & kind.pairFirst) && (held & kind.pairSecond))
            {
                put(app, set + kGearUp, ofController(controller, kind.pairCode));
                put(app, set + kGearDown, ofController(controller, kind.pairCode + 0x100));
            }
        }
        const Button& b = kind.buttons[i];
        if (reg8(device(app, controller).buttonMask >> (8 * b.part)) & b.mask)
            put(app, set + b.slot, ofController(controller, b.code));
    }
    return true;
}

/* A kind's adapter, for controller edx in mode eax: eax and edx as the code
 * leaves them -- edx as the keyboard's copy left it, then the axes' last
 * code, the buttons' byte in dh or dl and the gears' code; eax the last
 * button's code, or the controller's number times 17 where it has not that
 * button. */
void kindControls(App* app, CPU& cpu, const Kind& kind)
{
    const Kept kept(cpu);
    const reg32 controller = cpu.edx;
    if (!kindSet(app, cpu, cpu.eax, controller, kind))
        return;
    const DeviceRecord& d = device(app, controller);
    if (d.axisMask != 0)
        cpu.edx = ofController(controller, kind.pedalsOnAxes ? kSecondLow : kFirstHigh);
    const reg8 held = reg8(d.buttonMask);
    cpu.edx = kind.pairInHigh ? (cpu.edx & 0xffff00ff) | reg32(held) << 8 : (cpu.edx & 0xffffff00) | held;
    if ((held & kind.pairFirst) && (held & kind.pairSecond))
        cpu.edx = ofController(controller, kind.pairCode);
    const Button& last = kind.buttons[kind.count - 1];
    cpu.eax = reg8(d.buttonMask >> (8 * last.part)) & last.mask ? ofController(controller, last.code)
                                                                : (controller << 4) + controller;
    cpu.ebx = kept.ebx;
    cpu.ecx = kept.ecx;
    cpu.esi = kept.esi;
    cpu.ebp = kept.ebp;
    cpu.esp += 4;
}

}  // namespace

/* 0x4904f0: every player's controls in `mode` as they start: each player's
 * controller's -- its own player's: the first's, and in mode 1 the second's
 * -- by its name if the game knows it, else by its kind; the keyboard's for
 * a player with none, both players' in mode 1 when the first has none (and
 * then no more).  Whether it stopped at the keyboard's in mode 1 (eax then as
 * the copy left it); nothing when the game stopped. */
std::optional<bool> startingSets(App* app, CPU& cpu, reg32 mode)
{
    const GuestFrame frame(cpu, 0x18, 0);  // ebx, ecx, edx, esi, edi, ebp pushed
    const reg32 players = mode == kSplitMode ? 2 : 1;
    for (reg32 player = 0; player < players; ++player)
    {
        const GuestFrame slot(cpu, 0, 4);
        if (device(app, player).kind != 0)
        {
            const std::optional<bool> known = knownSet(app, cpu, mode, player);
            if (!known)
                return std::nullopt;
            if (!*known)
            {
                const reg32 kind = device(app, player).kind;
                if (!kindSet(app, cpu, mode, player,
                             kind == kGamePad ? kGamePadKind : kind == kJoystick ? kJoystickKind : kOtherKind))
                    return std::nullopt;
            }
        }
        else
        {
            if (!keyboardSets(app, cpu, mode, player == 0 ? 2 : 0))
                return std::nullopt;
            if (mode == kSplitMode)
                return true;
        }
    }
    return false;
}

namespace
{

/* A known controller's adapter, for controller edx in mode eax: eax whether
 * it was that one. */
void namedControls(App* app, CPU& cpu, const reg32 (&codes)[13], reg32 name)
{
    const Kept kept(cpu);
    const std::optional<bool> named = namedSet(app, cpu, cpu.eax, cpu.edx, codes, name);
    if (!named)
        return;
    cpu.eax = *named ? 1 : 0;
    cpu.ebx = kept.ebx;
    cpu.ecx = kept.ecx;
    cpu.esi = kept.esi;
    cpu.ebp = kept.ebp;
    cpu.esp += 4;
}

}  // namespace

/* 0x4904d0: playersSets for mode eax. */
void controlsFor(App* app, CPU& cpu)
{
    (void)app;
    cpu.eax = playersSets(cpu.eax);
    cpu.esp += 4;
}

/* 0x4904f0: startingSets for mode eax; eax the players' count, or as the
 * keyboard's copy left it. */
void defaultControls(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    const reg32 mode = cpu.eax;
    const std::optional<bool> atKeyboard = startingSets(app, cpu, mode);
    if (!atKeyboard)
        return;
    if (!*atKeyboard)
        cpu.eax = mode == kSplitMode ? 2 : 1;
    cpu.ebx = kept.ebx;
    cpu.ecx = kept.ecx;
    cpu.edx = kept.edx;
    cpu.esi = kept.esi;
    cpu.edi = kept.edi;
    cpu.ebp = kept.ebp;
    cpu.esp += 4;
}

/* 0x4905b0: makePlayers for mode eax, the set at edx, which ebx; eax, edx and
 * ebx as the copy leaves them. */
void setControls(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    if (!makePlayers(app, cpu, cpu.eax, cpu.edx, cpu.ebx))
        return;
    cpu.ecx = kept.ecx;
    cpu.ebp = kept.ebp;
    cpu.esp += 4;
}

/* 0x490610: nameHas for controller eax and the text at edx, 1 or 0. */
void controllerNamed(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    const std::optional<bool> named = nameHas(app, cpu, cpu.eax, cpu.edx);
    if (!named)
        return;
    cpu.eax = *named ? 1 : 0;
    cpu.ecx = kept.ecx;
    cpu.ebp = kept.ebp;
    cpu.esp += 4;
}

/* 0x490640: knownSet for mode eax and controller edx, 1 or 0. */
void knownControllerControls(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    const std::optional<bool> known = knownSet(app, cpu, cpu.eax, cpu.edx);
    if (!known)
        return;
    cpu.eax = *known ? 1 : 0;
    cpu.ebx = kept.ebx;
    cpu.ecx = kept.ecx;
    cpu.ebp = kept.ebp;
    cpu.esp += 4;
}

/* 0x4906b0: a "Microsoft SideWinder game pad"'s set: steering on its first
 * axis, its buttons. */
void sideWinderGamePadControls(App* app, CPU& cpu)
{
    namedControls(app, cpu, kSideWinderGamePad, kSideWinderGamePadName);
}

/* 0x490770: a "Gravis GrIP"'s. */
void gravisGripControls(App* app, CPU& cpu)
{
    namedControls(app, cpu, kGravisGrip, kGravisGripName);
}

/* 0x490830: a "Logitech WingMan Force"'s. */
void wingManForceControls(App* app, CPU& cpu)
{
    namedControls(app, cpu, kWingManForce, kWingManForceName);
}

/* 0x4908f0: a "CH F-16 Combat Stick"'s. */
void combatStickControls(App* app, CPU& cpu)
{
    namedControls(app, cpu, kCombatStick, kCombatStickName);
}

/* 0x4909b0: a "Microsoft 3D SideWinder Pro"'s. */
void sideWinderProControls(App* app, CPU& cpu)
{
    namedControls(app, cpu, kSideWinderPro, kSideWinderProName);
}

/* 0x490a70: a "Microsoft SideWinder Force Feedback Wheel"'s. */
void sideWinderWheelControls(App* app, CPU& cpu)
{
    namedControls(app, cpu, kSideWinderWheel, kSideWinderWheelName);
}

/* 0x490b30: steeringWheelSet for mode eax and controller edx, 1 or 0. */
void steeringWheelControls(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    const std::optional<bool> named = steeringWheelSet(app, cpu, cpu.eax, cpu.edx);
    if (!named)
        return;
    cpu.eax = *named ? 1 : 0;
    cpu.ebx = kept.ebx;
    cpu.ecx = kept.ecx;
    cpu.esi = kept.esi;
    cpu.edi = kept.edi;
    cpu.ebp = kept.ebp;
    cpu.esp += 4;
}

/* 0x490c40: keyboardSets for mode eax, which edx; eax and edx as the copy
 * leaves them. */
void keyboardControls(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    if (!keyboardSets(app, cpu, cpu.eax, cpu.edx))
        return;
    cpu.ebx = kept.ebx;
    cpu.ecx = kept.ecx;
    cpu.esi = kept.esi;
    cpu.edi = kept.edi;
    cpu.ebp = kept.ebp;
    cpu.esp += 4;
}

/* 0x490cb0: a game pad's controls, controller edx's in mode eax. */
void gamePadControls(App* app, CPU& cpu)
{
    kindControls(app, cpu, kGamePadKind);
}

/* 0x490e30: a joystick's. */
void joystickControls(App* app, CPU& cpu)
{
    kindControls(app, cpu, kJoystickKind);
}

/* 0x490fa0: any other controller's. */
void otherControllerControls(App* app, CPU& cpu)
{
    kindControls(app, cpu, kOtherKind);
}

}  // namespace controls
}  // namespace nfs3hp
