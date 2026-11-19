#include "native_controls_host.h"
#include "native_controls_input.h"
#include "native_frontend_screen.h"
#include "native_frontend_wina.h"
#include "native_game.h"
#include "native_watcom.h"
#include "native_guest.h"
#include "os_threads.h"
#include "os_system.h"

#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <optional>
#include <type_traits>

/* The game's side of DirectInput: the devices found and opened, the mouse, the
 * keyboard and the joysticks read, their force-feedback effects.
 * The game's code calls each by the name it has here.
 *
 * Each is a typed function under a name of its own, with a thunk under the
 * game's name for the callers still on registers and the dispatch: it leaves
 * the registers as the game's code does -- where a callee the code calls may
 * keep a register, the core sets it as the code has it at the call.  A frame
 * whose address a callee is handed (a DIPROPDWORD, DirectInput's buffers)
 * stays on the guest's stack as deep as the code has it (GuestFrame), and a
 * core calling a core makes room for the return address's slot as the call
 * does.  What the code leaves in a register no caller reads after it
 * (readMouse's and updateEffect's ebx, ecx and edx) is left as it comes.
 * What the game asked of DirectInput -- DirectInputCreateA and each
 * method of its objects -- it asks of the port (controls::directInput,
 * native_controls_host.h), with the arguments the game's code pushes: there
 * is no DirectInput behind it, only the port's pads, mouse and keyboard.
 * Win32 is called through the imports' slots, the window's message
 * handlers' table and the pointer's placing by address.
 *
 * The devices: 0 to 15 the joysticks, 0x10 all of them, 0x11 the mouse, 0x12
 * the keyboard, 0x13 every one.  A joystick's record, 0x88 bytes from
 * 0x5f1ff8: its kind (1 a stick, 2 a wheel, 4 a gamepad; bit 3 force
 * feedback), its product's name at 4, the buttons it has at 0x38 (a bit
 * each), its hats at 0x3c, its axes at 0x40, at 0x48 each axis's dead zone
 * and 100 less its saturation, in percent.  An effect's record, 0x88 bytes
 * from 0x5f28f0: the DIEFFECT DirectInput is handed, its type's parameters
 * at 0x34, its envelope's at 0x68, its direction at 0x78, whether it plays
 * at 0x80, its type at 0x84. */
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
using nfs::s32;

// DirectInput's objects: itself, the mouse's and the keyboard's devices, the
// mouse's event, the window's WM_SETCURSOR handler before ours.
constexpr u32 kDirectInput = 0x554ee4, kMouse = 0x554ee8, kKeyboard = 0x554eec;
constexpr u32 kMouseEvent = 0x554ef0, kOldSetCursor = 0x554ef4;
constexpr u32 kJoystickCount = 0x554ef8, kEffectCount = 0x554efc;
constexpr u32 kJoysticksLooked = 0x554f00;  // EnumDevices asked once
// The joysticks: their devices, 16; their records.
constexpr u32 kJoysticks = 0x5f1fb8;
constexpr u32 kJoystickRecords = 0x5f1ff8, kJoystickBytes = 0x88, kAllJoystickBytes = 0x880;
constexpr u32 kJoystickName = 0x5f1ffc, kJoystickNameEnd = 0x5f202d;
constexpr u32 kJoystickButtons = 0x5f2030, kJoystickHats = 0x5f2034, kJoystickAxes = 0x5f2038;
constexpr u32 kJoystickResolution = 0x5f203c;  // 0x4b each
constexpr u32 kJoystickDeadZone = 0x5f2040, kJoystickSaturation = 0x5f2044;  // 8 bytes an axis
// The effects: their objects, 30; their records; the axes they all share.
constexpr u32 kEffects = 0x5f2878, kEffectRecords = 0x5f28f0;
constexpr u32 kEffectAxes = 0x5f1fb0;
constexpr u32 kEffectFlags = 0x5f28f4, kEffectDuration = 0x5f28f8, kEffectPeriod = 0x5f28fc;
constexpr u32 kEffectGain = 0x5f2900, kEffectTrigger = 0x5f2904, kEffectRepeat = 0x5f2908;
constexpr u32 kEffectAxisCount = 0x5f290c, kEffectAxisList = 0x5f2910, kEffectDirections = 0x5f2914;
constexpr u32 kEffectEnvelope = 0x5f2918, kEffectParameterBytes = 0x5f291c, kEffectParameters = 0x5f2920;
constexpr u32 kEffectEnvelopeValues = 0x5f2958, kEffectDirection = 0x5f2968, kEffectDirection2 = 0x5f296c;
constexpr u32 kEffectPlaying = 0x5f2970, kEffectType = 0x5f2974;
// The game's tables: the effects' GUIDs by type, the hats' offsets, the axes'
// offsets, createEffect's jump table by type.
constexpr u32 kEffectGuids = 0x434ea0, kHatOffsets = 0x434ed4, kAxisOffsets = 0x434ef4;
constexpr u32 kParameterSizes = 0x435ed8;
// The mouse: the pointer's bounds and where it is, its buttons -- held in
// the low byte, let go since in the next -- its wheel, whether the buttons
// are swapped; the game's handler of the buttons let go.
constexpr u32 kPointerLeft = 0x5f38e0, kPointerTop = 0x5f38e4;
constexpr u32 kPointerRight = 0x5f38e8, kPointerBottom = 0x5f38ec;
constexpr u32 kPointerX = 0x5f38f0, kPointerY = 0x5f38f4;
constexpr u32 kButtons = 0x5f38f8, kButtonsLetGo = 0x5f38f9, kWheel = 0x5f38fc;
constexpr u32 kButtonsSwapped = 0x5f3900;
constexpr u32 kButtonHandler = 0x56446c;
// DirectInput's GUIDs and formats.
constexpr u32 kSysMouse = 0x56b908, kDevice2Interface = 0x56b918;
constexpr u32 kMouseFormat = 0x52f020, kJoystickFormat = 0x52f300;
// What is called.
constexpr u32 kPlacePointer = 0x435230;
constexpr u32 kJoystickFound = 0x436980;  // EnumDevices' callback
constexpr u32 kHideCursor = 0x436930;     // our WM_SETCURSOR handler
constexpr u32 kSetCursorMessage = 0x20;
// IDirectInputA's methods.
constexpr u32 kRelease = 8, kCreateDevice = 0xc, kEnumDevices = 0x10;
// IDirectInputDevice2A's.
constexpr u32 kQueryInterface = 0, kGetCapabilities = 0xc, kGetProperty = 0x14, kSetProperty = 0x18;
constexpr u32 kAcquire = 0x1c, kUnacquire = 0x20, kGetDeviceState = 0x24, kGetDeviceData = 0x28;
constexpr u32 kSetDataFormat = 0x2c, kSetEventNotification = 0x30, kSetCooperativeLevel = 0x34;
constexpr u32 kGetObjectInfo = 0x38, kCreateEffect = 0x48, kSendForceFeedbackCommand = 0x58, kPoll = 0x64;
// IDirectInputEffect's.
constexpr u32 kSetParameters = 0x18, kStart = 0x1c, kStop = 0x20, kDownload = 0x28, kUnload = 0x2c;
// The devices.
constexpr u32 kAllJoysticks = 0x10, kTheMouse = 0x11, kTheKeyboard = 0x12, kEveryDevice = 0x13;
constexpr u32 kInputLost = 0x8007001e, kNotAcquired = 0x8007000c;

void push(nfs::App* app, nfs::ThreadState& ts, u32 value)
{
    ts.sp -= 4;
    put(app, ts.sp, value);
}

using controls::directInputResult;
using controls::kCreate;
using controls::Object;

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using i16 = std::int16_t;
using nfs::u8;

constexpr u32 kEffectBytes = 0x88;
constexpr u8 kForceFeedback = 8;  // in a joystick's kind
// DISFFC_: SendForceFeedbackCommand's commands.
constexpr u32 kReset = 1, kStopAll = 2, kPause = 4, kContinue = 8, kActuatorsOn = 0x10, kActuatorsOff = 0x20;
constexpr u32 kEffectsMade = 0x1e;  // the effects' records

/* The guest's stack as a function's code has it: `pushed` bytes of saved
 * registers below esp, then `bytes` of locals, esp left below them; esp as it
 * was when it ends.  Its base is where ebp would point.  For a frame whose
 * address a callee is handed. */
class GuestFrame
{
public:
    GuestFrame(ThreadState& ts, u32 pushed, u32 bytes) : ts(ts), esp(ts.sp), base(ts.sp - pushed)
    {
        ts.sp = base - bytes;
    }
    ~GuestFrame() { ts.sp = esp; }
    u32 operator-(u32 offset) const { return base - offset; }

private:
    ThreadState& ts;
    const u32 esp;
    const u32 base;
};

/* The registers a function's code pushes and pops again, kept to be put back. */
struct Kept
{
    u32 ebx, ecx, edx, esi, edi, ebp;
};

bool isJoystick(u32 device)
{
    return s32(device) >= 0 && s32(device) < 0x10;
}

/* Joystick `joystick`'s device (0 none), and whether it has force feedback. */
u32 joystickDevice(App* app, u32 joystick)
{
    return at(app, kJoysticks + joystick * 4);
}

bool forceFeedback(App* app, u32 joystick)
{
    return byte(app, kJoystickRecords + joystick * kJoystickBytes) & kForceFeedback;
}

u32 effectObject(App* app, u32 effect)
{
    return at(app, kEffects + effect * 4);
}

/* The registers as axisProperty's code leaves them, `percent` its stack's
 * word: where it asked nothing, edx the word; where it did, ebx the device
 * when setting, and ecx 100 and edx the remainder where DirectInput
 * answered -- else as the call left them (nothing). */
struct ZoneLeft
{
    u32 eax;
    std::optional<u32> ebx, ecx, edx;
};

ZoneLeft zoneRegisters(App* app, u32 joystick, u32 axis, bool read, u32 percent, const Zone& zone)
{
    ZoneLeft left{zone.percent, {}, {}, {}};
    if (!isJoystick(joystick) || s32(axis) > 8 || joystickDevice(app, joystick) == 0)
    {
        left.edx = percent;
        return left;
    }
    if (!read)
        left.ebx = joystickDevice(app, joystick);
    if (zone.remainder)
    {
        left.ecx = 100;
        left.edx = *zone.remainder;
    }
    return left;
}

/* An effect's number as the code reads it: ax, as a signed word. */
u32 effectIn(u32 eax)
{
    return u32(s32(i16(eax)));
}

/* Method `method` of DirectInput's object `object` (stdcall: the object, then
 * `words`), as the code calls it: the object's table of methods read into
 * edx, where the code has it.  Its HRESULT; nothing when the game stopped. */
std::optional<u32> callMethod(App* app, ThreadState& ts, Object kind, u32 object, u32 method,
                                std::initializer_list<u32> words)
{
    for (auto w = std::rbegin(words); w != std::rend(words); ++w)
        push(app, ts, *w);
    push(app, ts, object);
    return directInputResult(app, ts, kind, method);
}

}  // namespace

namespace
{
bool startInput(App* app, ThreadState& ts);
bool stopAllEffects(App* app, ThreadState& ts, u32 joystick);
bool resetJoysticks(App* app, ThreadState& ts);
std::optional<u32> acquire(App* app, ThreadState& ts, u32 device);
bool acquiredBy(u32 result);
std::optional<u32> effectMethod(App* app, ThreadState& ts, u32 effect, u32 method);
u32 effectPlays(App* app, u32 effect);
std::optional<u32> readZone(App* app, ThreadState& ts, u32 joystick, u32 axis, u32 which);
std::optional<u32> openTheMouse(App* app, ThreadState& ts);
std::optional<u32> cooperate(App* app, ThreadState& ts, u32 device, u32 flags);
std::optional<Zone> axisZone(App* app, ThreadState& ts, u32 joystick, u32 axis, bool read, u32 which, u32 percent);

/* The game's fill with zeros (nfs3hp::fillZero): `bytes` from `address`.
 * False when the game stopped. */
bool zero(App* app, ThreadState& ts, u32 address, u32 bytes)
{
    guest::direct<0x4e070c, ea::clearBytes>(app, ts, address, bytes);
    return !ts.terminate;
}
}  // namespace


nfs::u32 openMouse(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 setPointerBounds(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 describeJoystick(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 readAxisZone(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 resetForceFeedback(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 stopEffects(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 startEffect(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 downloadEffect(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 unloadEffect(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 setCooperativeLevel(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 acquireDevice(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 acquireResult(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 effectPlaying(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 axisProperty(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);

namespace
{

/* 0x434f20: DirectInput started, where it is not: the joysticks' records
 * cleared, their devices and the effects' none, DirectInputCreateA asked for
 * version 5, and the mouse opened (openMouse).  Nothing kept if it fails.
 * The eax the code leaves (`eaxBefore` where nothing is done, the HRESULT where DirectInput fails, else
 * the mouse's); nothing when the game stopped. */
std::optional<u32> startedInput(App* app, ThreadState& ts, u32 eaxBefore)
{
    if (at(app, kDirectInput) != 0)
        return eaxBefore;
    const GuestFrame frame(ts, 0x18, 0);  // ebx, ecx, edx, esi, edi, ebp pushed
    if (!zero(app, ts, kJoystickRecords, kAllJoystickBytes))
        return std::nullopt;
    for (u32 joystick = 0; joystick < 0x10; ++joystick)
    {
        put(app, kJoysticks + joystick * 4, 0);
        put(app, kJoystickResolution + joystick * kJoystickBytes, 0x4b);
    }
    for (u32 effect = 0; effect < kEffectsMade; ++effect)
        put(app, kEffects + effect * 4, 0);
    put(app, kJoystickCount, 0);
    // DirectInputCreateA(the program's module, version 5, &DirectInput, no outer)
    push(app, ts, 0);
    push(app, ts, kDirectInput);
    push(app, ts, 0x500);
    push(app, ts, os::kProgramModule);
    const std::optional<u32> created = directInputResult(app, ts, Object::DirectInput, kCreate);
    if (!created)
        return std::nullopt;
    if (*created != 0)
    {
        put(app, kDirectInput, 0);
        return *created;
    }
    return guest::direct<0x4350a0, controls::openMouseCore>(app, ts);
}

/* startedInput for a caller that wants no eax. */
bool startInput(App* app, ThreadState& ts)
{
    return startedInput(app, ts, 0).has_value();
}

}  // namespace

/* 0x434f20, called as C++ (guest::direct). */
bool startInputCore(App* app, ThreadState& ts)
{
    return startInput(app, ts);
}

/* 0x434f20: startedInput. */
nfs::u32 startDirectInput(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept{args.ebx, args.ecx, args.edx, args.esi, args.edi, args.ebp};
    const std::optional<u32> started = startedInput(app, ts, args.eax);
    if (!started)
        return args.eax;
    args.eax = *started;
    args.ebx = kept.ebx;
    args.ecx = kept.ecx;
    args.edx = kept.edx;
    args.esi = kept.esi;
    args.edi = kept.edi;
    args.ebp = kept.ebp;
    ts.sp += 4;
    return args.eax;
}

/* 0x434fd0: DirectInput stopped: the mouse let go and released (its event's
 * handle closed, though kept), the keyboard, each joystick, then DirectInput
 * itself; no joysticks.  False when the game stopped. */
bool stopInput(App* app, ThreadState& ts)
{
    const GuestFrame frame(ts, 0x18, 0);  // ebx, ecx, edx, esi, edi, ebp pushed
    if (at(app, kDirectInput) != 0)
    {
        if (at(app, kMouse) != 0)
        {
            const u32 event = at(app, kMouseEvent);
            if (event != 0)
                os::release(app, event);
            if (!callMethod(app, ts, Object::Device, at(app, kMouse), kUnacquire, {}))
                return false;
            if (!callMethod(app, ts, Object::Device, at(app, kMouse), kRelease, {}))
                return false;
            put(app, kMouse, 0);
        }
        if (at(app, kKeyboard) != 0)
        {
            if (!callMethod(app, ts, Object::Device, at(app, kKeyboard), kUnacquire, {}))
                return false;
            if (!callMethod(app, ts, Object::Device, at(app, kKeyboard), kRelease, {}))
                return false;
            put(app, kKeyboard, 0);
        }
        for (u32 joystick = 0; joystick < 0x10; ++joystick)
        {
            if (joystickDevice(app, joystick) == 0)
                continue;
            if (!callMethod(app, ts, Object::Device, joystickDevice(app, joystick), kUnacquire, {}))
                return false;
            if (!callMethod(app, ts, Object::Device, joystickDevice(app, joystick), kRelease, {}))
                return false;
            put(app, kJoysticks + joystick * 4, 0);
        }
        if (!callMethod(app, ts, Object::DirectInput, at(app, kDirectInput), kRelease, {}))
            return false;
        put(app, kDirectInput, 0);
    }
    put(app, kJoystickCount, 0);
    return true;
}

/* 0x434fd0: stopInput; eax 0 where DirectInput was started, as the code
 * leaves it. */
nfs::u32 stopDirectInput(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept{args.ebx, args.ecx, args.edx, args.esi, args.edi, args.ebp};
    const bool started = at(app, kDirectInput) != 0;
    if (!stopInput(app, ts))
        return args.eax;
    if (started)
        args.eax = 0;
    args.ebx = kept.ebx;
    args.ecx = kept.ecx;
    args.edx = kept.edx;
    args.esi = kept.esi;
    args.edi = kept.edi;
    args.ebp = kept.ebp;
    ts.sp += 4;
    return args.eax;
}

namespace
{


/* A native_frontend_* file's typed core called as C++ (guest::direct): nothing (false) when the
 * game stopped in it (its calls into the game throw fe::Terminated). */
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


/* 0x4350a0: the mouse opened, DirectInput started first if it is not: the
 * buttons' swap asked of Windows (and put back), the device made, its
 * format the mouse's, exclusive in the foreground, an event for its data
 * and a buffer of 32 events (a DIPROPDWORD on the guest's stack), the
 * window's WM_SETCURSOR handler ours (hideCursor, the old one kept); then
 * acquired, the buttons and the wheel none, the pointer's bounds the 640 by
 * 480 screen.  1, or 0 when it fails; nothing when the game stopped. */
std::optional<u32> openTheMouse(App* app, ThreadState& ts)
{
    const GuestFrame frame(ts, 0x14, 0x14);  // ebx, ecx, edx, esi, ebp pushed; the DIPROPDWORD
    if (at(app, kDirectInput) == 0)
    {
        {
            const GuestFrame call(ts, 0, 4);
            if (!startInput(app, ts))
                return std::nullopt;
        }
        if (at(app, kDirectInput) == 0)
            return 0;
    }
    if (at(app, kMouse) == 0)
    {
        /* Whether the system's mouse buttons were swapped, found by swapping them and put back:
         * the port's are never swapped. */
        put(app, kButtonsSwapped, 0);
        std::optional<u32> result =
            callMethod(app, ts, Object::DirectInput, at(app, kDirectInput), kCreateDevice, {kSysMouse, kMouse, 0});
        if (!result)
            return std::nullopt;
        if (*result != 0)
            return 0;
        result = callMethod(app, ts, Object::Device, at(app, kMouse), kSetDataFormat, {kMouseFormat});
        if (!result)
            return std::nullopt;
        if (*result != 0)
            return 0;
        {
            const GuestFrame call(ts, 0, 4);
            result = cooperate(app, ts, kTheMouse, 5);  // DISCL_EXCLUSIVE | DISCL_FOREGROUND
        }
        if (!result)
            return std::nullopt;
        if (*result == 0)
            return 0;
        const os::Handle event = os::events::make(app, false, false);
        put(app, kMouseEvent, event);
        result = callMethod(app, ts, Object::Device, at(app, kMouse), kSetEventNotification, {event});
        if (!result)
            return std::nullopt;
        if (*result != 0)
            return 0;
        // SetProperty(the mouse, DIPROP_BUFFERSIZE, {0x14, 0x10, 0, DIPH_DEVICE, 32})
        const u32 record = frame - 0x14;
        put(app, record, 0x14);
        put(app, record + 8, 0);
        put(app, record + 0xc, 0);
        put(app, record + 4, 0x10);
        put(app, record + 0x10, 0x20);
        result = callMethod(app, ts, Object::Device, at(app, kMouse), kSetProperty, {1, record});
        if (!result)
            return std::nullopt;
        if (*result != 0)
            return 0;
        // wina's: the window's handler of WM_SETCURSOR, then ours (hideCursor) in its place.
        const std::optional<u32> old = feCore<0x4cc660, frontend::wina::handlerOf>(app, ts, kSetCursorMessage);
        if (!old)
            return std::nullopt;
        put(app, kOldSetCursor, *old);
        if (!feCore<0x4cc6d0, frontend::wina::handlerSet>(app, ts, kSetCursorMessage, kHideCursor))
            return std::nullopt;
    }
    if (at(app, kMouse) != 0)
    {
        const GuestFrame call(ts, 0, 4);
        if (!acquire(app, ts, kTheMouse))
            return std::nullopt;
    }
    put(app, kButtons, 0);
    put(app, kWheel, 0);
    const GuestFrame call(ts, 0, 4);
    if (!boundPointer(app, ts, 0, 0, 0x280, 0x1e0))
        return std::nullopt;
    return 1;
}

}  // namespace

/* 0x4350a0: openTheMouse, called as C++ (guest::direct). */
std::optional<u32> openMouseCore(App* app, ThreadState& ts)
{
    return openTheMouse(app, ts);
}

/* 0x4350a0: openTheMouse. */
nfs::u32 openMouse(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept{args.ebx, args.ecx, args.edx, args.esi, args.edi, args.ebp};
    const std::optional<u32> opened = openTheMouse(app, ts);
    if (!opened)
        return args.eax;
    args.eax = *opened;
    args.ebx = kept.ebx;
    args.ecx = kept.ecx;
    args.edx = kept.edx;
    args.esi = kept.esi;
    args.ebp = kept.ebp;
    ts.sp += 4;
    return args.eax;
}

/* 0x4352d0: the pointer's bounds, and the pointer put back inside them, where
 * there is a mouse: frontend::placePointer, by address, with x in eax and y
 * in edx, the right and bottom bounds in ebx and ecx as the code leaves
 * them.  False when the game stopped. */
bool boundPointer(App* app, ThreadState& ts, u32 left, u32 top, u32 right, u32 bottom)
{
    if (at(app, kMouse) == 0)
        return true;
    put(app, kPointerLeft, left);
    put(app, kPointerTop, top);
    put(app, kPointerRight, right);
    const u32 y = at(app, kPointerY), x = at(app, kPointerX);
    put(app, kPointerBottom, bottom);
    const GuestFrame frame(ts, 4, 0);  // ebp pushed
    return feCore<0x435230, frontend::screen::placePointer>(app, ts, x, y).has_value();
}

/* 0x4352d0: boundPointer, eax left, edx top, ebx right, ecx bottom; the
 * registers as placePointer leaves them. */
nfs::u32 setPointerBounds(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 ebp = args.ebp;
    if (!boundPointer(app, ts, args.eax, args.edx, args.ebx, args.ecx))
        return args.eax;
    args.ebp = ebp;
    ts.sp += 4;
    return args.eax;
}

namespace
{

// DirectInput's buffer of the mouse's events: 32 DIDEVICEOBJECTDATA at most,
// an event's offset in DIMOUSESTATE at 0, its data at 4.
constexpr u32 kMostEvents = 0x20, kEventBytes = 0x10;
constexpr u32 kMouseX = 0, kMouseY = 4, kMouseWheel = 8, kFirstButton = 0xc, kLastButton = 0xf;
constexpr u32 kPressed = 0x80;         // in a button's data
constexpr u32 kButtonBits = 0xf;       // buttons 0 to 3, a bit each
constexpr u32 kLetGo = 0xf00;          // kButtons' word: those let go, its second byte

/* The game's handler of the mouse's buttons let go ([0x56446c], Watcom),
 * handed the buttons' word, those let go only, in eax: what it gives back;
 * nothing when the game stopped. */
std::optional<u32> buttonsLetGo(App* app, ThreadState& ts, u32 handler, u32 letGo)
{
    nfs::Args registers;
    registers.eax = letGo;
    return guest::pointerCall(app, ts, handler, registers);
}

/* The pointer's x or y at `place` moved by `by`, put back within [low] and
 * [high]: where it moved to before it was put back. */
u32 movePointer(App* app, u32 place, u32 low, u32 high, u32 by)
{
    const u32 moved = at(app, place) + by;
    put(app, place, moved);
    if (s32(moved) < s32(at(app, low)))
        put(app, place, at(app, low));
    else if (s32(moved) > s32(at(app, high)))
        put(app, place, at(app, high));
    return moved;
}

/* The buttons, the wheel, x and y, in that order, handed out where they are
 * asked for ([buttonsAt] and so on, not 0): the last word handed out, none
 * where none was asked for. */
std::optional<u32> handOut(App* app, u32 buttonsAt, u32 xAt, u32 yAt, u32 wheelAt)
{
    std::optional<u32> last;
    auto give = [&](u32 to, u32 from) {
        if (to == 0)
            return;
        last = at(app, from);
        put(app, to, *last);
    };
    give(buttonsAt, kButtons);
    give(wheelAt, kWheel);
    give(xAt, kPointerX);
    give(yAt, kPointerY);
    return last;
}

}  // namespace

/* 0x435310: the mouse read -- what DirectInput has buffered of it, up to 32
 * events: the pointer moved within its bounds, the wheel, buttons 0 to 3
 * pressed and let go (the first two swapped as Windows has them, and the
 * other two with them) -- and handed out (handOut), the game's handler called
 * with the buttons let go.  Lost or not acquired, the mouse is acquired
 * again; read or not, what it had is handed out, the buttons none.
 *
 * What the code leaves in eax, which 0x4bc530 and 0x451500 give back as
 * theirs (their callers read none of it): the buttons' address where there is
 * no mouse; DirectInput's answer where it would not read, or how many devices
 * it acquired again; the handler's answer; else the last word handed out, or
 * without one what the last event left -- where it moved the pointer to, the
 * wheel, the button's offset -- or the buttons held where there was none.
 * Nothing when the game stopped. */
std::optional<u32> mouseRead(App* app, ThreadState& ts, u32 buttonsAt, u32 xAt, u32 yAt, u32 wheelAt)
{
    const u32 mouse = at(app, kMouse);
    if (mouse == 0)
        return buttonsAt;
    const GuestFrame frame(ts, 0xc, 0x208);  // esi, edi, ebp pushed; the events, how many
    const u32 events = frame - 0x208, count = frame - 8;
    put(app, count, kMostEvents);
    put(app, kWheel, 0);
    // GetDeviceData(the mouse, an event's bytes, the events, &how many, 0)
    const std::optional<u32> result =
        callMethod(app, ts, Object::Device, mouse, kGetDeviceData, {kEventBytes, events, count, 0});
    if (!result)
        return std::nullopt;
    if (*result != 0)
    {
        put(app, kButtons, 0);
        handOut(app, buttonsAt, xAt, yAt, wheelAt);
        if (*result != kNotAcquired && *result != kInputLost)
            return *result;
        const GuestFrame call(ts, 0, 4);  // the return address's slot
        return acquire(app, ts, kTheMouse);
    }
    u32 left = at(app, kButtons) & kButtonBits;  // those held; those let go forgotten
    put(app, kButtons, left);
    const u32 read = at(app, count);
    for (u32 i = 0; i < read; ++i)
    {
        const u32 event = events + i * kEventBytes;
        u32 offset = at(app, event);
        const u32 data = at(app, event + 4);
        if (offset == kMouseX)
            left = movePointer(app, kPointerX, kPointerLeft, kPointerRight, data);
        else if (offset == kMouseY)
            left = movePointer(app, kPointerY, kPointerTop, kPointerBottom, data);
        else if (offset == kMouseWheel)
        {
            put(app, kWheel, data);
            left = data;
        }
        else
        {
            const bool button = offset >= kFirstButton && offset <= kLastButton;
            if (button && at(app, kButtonsSwapped) != 0)
                offset ^= 1;  // 0 and 1 swapped, 2 and 3
            if (button)
            {
                const u8 bit = u8(1 << (offset - kFirstButton));
                if (data & kPressed)
                    putByte(app, kButtons, byte(app, kButtons) | bit);
                else
                {
                    putByte(app, kButtons, byte(app, kButtons) ^ bit);
                    putByte(app, kButtonsLetGo, byte(app, kButtonsLetGo) | bit);
                }
            }
            left = offset;
        }
    }
    const std::optional<u32> handed = handOut(app, buttonsAt, xAt, yAt, wheelAt);
    const u32 handler = at(app, kButtonHandler);
    if (handler != 0 && (byte(app, kButtonsLetGo) & kButtonBits) != 0)
        return buttonsLetGo(app, ts, handler, at(app, kButtons) & kLetGo);
    return handed.value_or(left);
}

/* 0x435310: mouseRead into [eax] the buttons, [edx] x, [ebx] y, [ecx] the
 * wheel; eax as the code leaves it. */
nfs::u32 readMouse(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept{args.ebx, args.ecx, args.edx, args.esi, args.edi, args.ebp};
    const std::optional<u32> left = mouseRead(app, ts, args.eax, args.edx, args.ebx, args.ecx);
    if (!left)
        return args.eax;
    args.eax = *left;
    args.esi = kept.esi;
    args.edi = kept.edi;
    args.ebp = kept.ebp;
    ts.sp += 4;
    return args.eax;
}

/* 0x435600: the joysticks, looked for once -- EnumDevices for the attached
 * ones (joystickFound), each described (describeJoystick) -- and acquired,
 * their force feedback reset, when there are any.  How many there are; 0
 * when DirectInput will not start; nothing when the game stopped. */
std::optional<u32> lookForJoysticks(App* app, ThreadState& ts)
{
    const GuestFrame frame(ts, 0x14, 0);  // ecx, edx, esi, edi, ebp pushed
    bool look = true;
    if (at(app, kDirectInput) == 0)
    {
        const GuestFrame slot(ts, 0, 4);
        if (!startInput(app, ts))
            return std::nullopt;
        if (at(app, kDirectInput) == 0)
            return 0;
    }
    else if (at(app, kJoysticksLooked) != 0)
        look = false;
    if (look)
    {
        const u32 count = at(app, kJoystickCount);
        put(app, kJoysticksLooked, 1);
        if (count == 0)
        {
            // EnumDevices(DirectInput, DIDEVTYPE_JOYSTICK, joystickFound, DirectInput, DIEDFL_ATTACHEDONLY)
            const u32 input = at(app, kDirectInput);
            if (!callMethod(app, ts, Object::DirectInput, input, kEnumDevices, {4, kJoystickFound, input, 1}))
                return std::nullopt;
            const GuestFrame slot(ts, 0, 4);
            if (!describe(app, ts, kAllJoysticks))
                return std::nullopt;
        }
        if (at(app, kJoystickCount) != 0)
        {
            const GuestFrame slot(ts, 0, 4);
            if (!acquire(app, ts, kAllJoysticks))
                return std::nullopt;
            if (!resetJoysticks(app, ts))
                return std::nullopt;
        }
    }
    return at(app, kJoystickCount);
}

/* 0x435600: lookForJoysticks. */
nfs::u32 findJoysticks(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept{args.ebx, args.ecx, args.edx, args.esi, args.edi, args.ebp};
    const std::optional<u32> count = lookForJoysticks(app, ts);
    if (!count)
        return args.eax;
    args.eax = *count;
    args.ebx = kept.ebx;
    args.ecx = kept.ecx;
    args.edx = kept.edx;
    args.esi = kept.esi;
    args.edi = kept.edi;
    args.ebp = kept.ebp;
    ts.sp += 4;
    return args.eax;
}

/* 0x435690: joystick `joystick` described, where it is not yet -- acquired;
 * whether it has force feedback, the buttons (up to 32), hats (4) and axes
 * (8) it has, by asking for each object's info, and each axis's dead zone
 * and saturation (readZone).  The DIDEVCAPS and the
 * DIDEVICEOBJECTINSTANCE DirectInput fills are on the guest's stack.  Its
 * record, or 0; for all of them (0x10) each from the 15th down, joystick
 * 0's.  Nothing when the game stopped. */
std::optional<u32> describe(App* app, ThreadState& ts, u32 joystick)
{
    const GuestFrame frame(ts, 0x18, 0x190);  // ebx, ecx, edx, esi, edi, ebp pushed; the locals
    if (s32(joystick) > 0x10 || s32(joystick) < 0)
        return 0;
    const GuestFrame slot(ts, 0, 4);
    if (joystick == kAllJoysticks)
    {
        std::optional<u32> record;
        for (u32 each = 0x10; each-- > 0;)
            if (!(record = describe(app, ts, each)))
                return std::nullopt;
        return record;
    }
    if (at(app, kDirectInput) == 0)
    {
        if (!startInput(app, ts))
            return std::nullopt;
        if (at(app, kDirectInput) == 0)
            return 0;
    }
    if (joystickDevice(app, joystick) == 0)
        return 0;
    const u32 record = joystick * kJoystickBytes;
    if (!acquire(app, ts, joystick))
        return std::nullopt;
    if (at(app, record + kJoystickButtons) == 0 && at(app, record + kJoystickAxes) == 0)
    {
        // GetCapabilities(the joystick, a DIDEVCAPS of 0x2c): its flags at 4, axes at 0xc, buttons at
        // 0x10, hats at 0x14.
        const u32 caps = frame - 0x54;
        put(app, caps, 0x2c);
        std::optional<u32> result =
            callMethod(app, ts, Object::Device, joystickDevice(app, joystick), kGetCapabilities, {caps});
        if (!result)
            return std::nullopt;
        if (*result != 0)
            return 0;
        if (byte(app, caps + 5) & 1)  // DIDC_FORCEFEEDBACK
            app->getMemory<u8>(kJoystickRecords + record) |= kForceFeedback;
        // Each object: GetObjectInfo(the joystick, a DIDEVICEOBJECTINSTANCE of 0x13c, its offset,
        // DIPH_BYOFFSET); whether it is there.
        const u32 info = frame - 0x190;
        put(app, info, 0x13c);
        auto there = [&](u32 offset) -> std::optional<bool> {
            const std::optional<u32> found =
                callMethod(app, ts, Object::Device, joystickDevice(app, joystick), kGetObjectInfo, {info, offset, 1});
            if (!found)
                return std::nullopt;
            return *found == 0;
        };
        // The buttons: at 0x30 on.
        for (u32 button = 0, found = 0; found < at(app, caps + 0x10) && s32(button) < 0x20; ++button)
        {
            const std::optional<bool> is = there(button + 0x30);
            if (!is)
                return std::nullopt;
            if (*is)
            {
                put(app, record + kJoystickButtons, at(app, record + kJoystickButtons) | u32(1) << button);
                ++found;
            }
        }
        // The hats: at 0x20 on.
        for (u32 hat = 0, found = 0; found < at(app, caps + 0x14) && s32(hat) < 4; ++hat)
        {
            const std::optional<bool> is = there(hat * 4 + 0x20);
            if (!is)
                return std::nullopt;
            if (*is)
            {
                put(app, record + kJoystickHats, at(app, record + kJoystickHats) | u32(1) << hat);
                ++found;
            }
        }
        // The axes: at their offsets in DIJOYSTATE; each one's zones.
        for (u32 axis = 0, found = 0; found < at(app, caps + 0xc) && s32(axis) < 8; ++axis)
        {
            const std::optional<bool> is = there(at(app, kAxisOffsets + axis * 4));
            if (!is)
                return std::nullopt;
            if (!*is)
                continue;
            put(app, record + kJoystickAxes, at(app, record + kJoystickAxes) | u32(1) << axis);
            if (!readZone(app, ts, joystick, axis, 0))
                return std::nullopt;
            ++found;
            if (!readZone(app, ts, joystick, axis, 1))
                return std::nullopt;
        }
    }
    return kJoystickRecords + record;
}

/* 0x435690: describe joystick eax. */
nfs::u32 describeJoystick(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept{args.ebx, args.ecx, args.edx, args.esi, args.edi, args.ebp};
    const std::optional<u32> record = describe(app, ts, args.eax);
    if (!record)
        return args.eax;
    args.eax = *record;
    args.ebx = kept.ebx;
    args.ecx = kept.ecx;
    args.edx = kept.edx;
    args.esi = kept.esi;
    args.edi = kept.edi;
    args.ebp = kept.ebp;
    ts.sp += 4;
    return args.eax;
}

namespace
{

}  // namespace

/* 0x4358f0: joystick `joystick` read into the DIJOYSTATE at `state` --
 * polled, its state taken -- and made the game's after it: the axes copied
 * to 0x50, the buttons pressed as bits at 0x70, the 4 hats' directions at
 * 0x74 (1 up to 8 round from north, 0 centred or none).  `state`, or 0;
 * lost or not acquired, the joystick acquired again.  Nothing when the game
 * stopped. */
std::optional<u32> readState(App* app, ThreadState& ts, u32 joystick, u32 state)
{
    if (!isJoystick(joystick) || joystickDevice(app, joystick) == 0 || state == 0)
        return 0;
    std::optional<u32> result = callMethod(app, ts, Object::Device, joystickDevice(app, joystick), kPoll, {});
    if (!result)
        return std::nullopt;
    if (*result != 0)
    {
        if ((*result == kInputLost || *result == kNotAcquired) && !acquire(app, ts, joystick))
            return std::nullopt;
        return 0;
    }
    result = callMethod(app, ts, Object::Device, joystickDevice(app, joystick), kGetDeviceState, {0x50, state});
    if (!result)
        return std::nullopt;
    if (*result != 0)
    {
        if (*result == kInputLost && !acquire(app, ts, joystick))
            return std::nullopt;
        return 0;
    }
    const u32 first = at(app, state);
    put(app, state + 0x70, 0);
    put(app, state + 0x50, first);
    for (u32 i = 4; i < 0x20; i += 4)
        put(app, state + 0x50 + i, at(app, state + i));
    for (u32 button = 0; button < 0x20; ++button)
        if (byte(app, state + 0x30 + button) & 0x80)
            put(app, state + 0x70, at(app, state + 0x70) | u32(1) << button);
    static const u32 kDirections[] = {0, 0x1194, 0x2328, 0x34bc, 0x4650, 0x57e4, 0x6978, 0x7b0c};
    for (u32 hat = 0; hat < 4; ++hat)
    {
        const u32 pov = at(app, state + 0x20 + hat * 4);
        if ((at(app, kJoystickHats + joystick * kJoystickBytes) & u32(1) << hat) && pov < 0xffff)
        {
            u32 direction = 0;
            for (u32 i = 0; i < 8; ++i)
                if (pov == kDirections[i])
                {
                    direction = i + 1;
                    break;
                }
            put(app, state + hat * 4 + 0x74, direction);
        }
        else
            put(app, state + hat * 4 + 0x74, 0);
    }
    return state;
}

/* 0x4358f0: readState for joystick eax into edx; edx as the code leaves it:
 * the joystick's device where it or the state is none, 4 where read, else as
 * the call left it (the number given back where it is no joystick). */
nfs::u32 readJoystick(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept{args.ebx, args.ecx, args.edx, args.esi, args.edi, args.ebp};
    const u32 joystick = args.eax, state = args.edx;
    const std::optional<u32> read = readState(app, ts, joystick, state);
    if (!read)
        return args.eax;
    if (isJoystick(joystick))
    {
        if (joystickDevice(app, joystick) == 0 || state == 0)
            args.edx = joystickDevice(app, joystick);
        else if (*read != 0)
            args.edx = 4;
    }
    args.eax = *read;
    args.ebx = kept.ebx;
    args.ecx = kept.ecx;
    args.esi = kept.esi;
    args.edi = kept.edi;
    args.ebp = kept.ebp;
    ts.sp += 4;
    return args.eax;
}

namespace
{

/* 0x435ae0: joystick `joystick`'s axis `axis`'s dead zone (`which` 0) or
 * saturation asked of DirectInput (axisZone), in percent, kept in its record
 * -- the saturation as 100 less it.  What was asked; nothing when the game
 * stopped. */
std::optional<u32> readZone(App* app, ThreadState& ts, u32 joystick, u32 axis, u32 which)
{
    const GuestFrame frame(ts, 0x10, 4);  // ecx, esi, edi, ebp pushed; the axis kept
    std::optional<Zone> zone;
    {
        const GuestFrame call(ts, 0, 8);  // the percent's word and the return address's slot
        zone = axisZone(app, ts, joystick, axis, true, which, 0);
    }
    if (!zone)
        return std::nullopt;
    const u32 record = joystick * kJoystickBytes + axis * 8;
    if (which == 0)
        put(app, record + kJoystickDeadZone, zone->percent);
    else
        put(app, record + kJoystickSaturation, 100 - zone->percent);
    return zone->percent;
}

}  // namespace

/* 0x435b40: joystick `joystick`'s axis `axis`'s dead zone (`which` 0) or
 * saturation (`which` 1: `percent` as 100 less it) set to `percent`
 * (axisZone).  Nothing when the game stopped. */
std::optional<Zone> setZone(App* app, ThreadState& ts, u32 joystick, u32 axis, u32 which, u32 percent)
{
    const GuestFrame frame(ts, 8, 0);  // esi, ebp pushed
    const GuestFrame call(ts, 0, 8);   // the percent's word and the return address's slot
    return axisZone(app, ts, joystick, axis, false, which, which == 1 ? 100 - percent : percent);
}

/* 0x435ae0: readZone for joystick eax, axis edx, which ebx; eax and edx what
 * was kept, ebx what was asked, as the code leaves them. */
nfs::u32 readAxisZone(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept{args.ebx, args.ecx, args.edx, args.esi, args.edi, args.ebp};
    const u32 which = args.ebx;
    const std::optional<u32> percent = readZone(app, ts, args.eax, args.edx, which);
    if (!percent)
        return args.eax;
    args.ebx = *percent;
    args.edx = which == 0 ? *percent : 100 - *percent;
    args.eax = args.edx;
    args.ecx = kept.ecx;
    args.esi = kept.esi;
    args.edi = kept.edi;
    args.ebp = kept.ebp;
    ts.sp += 4;
    return args.eax;
}

/* 0x435b40: setZone for joystick eax, axis edx, which ebx, percent ecx; the
 * registers as axisProperty leaves them. */
nfs::u32 setAxisZone(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept{args.ebx, args.ecx, args.edx, args.esi, args.edi, args.ebp};
    const u32 joystick = args.eax, axis = args.edx, which = args.ebx;
    const u32 asked = args.ecx, percent = which == 1 ? 100 - asked : asked;
    // As the code hands them to axisProperty.
    args.ebx = 0;
    args.ecx = which;
    const std::optional<Zone> zone = setZone(app, ts, joystick, axis, which, asked);
    if (!zone)
        return args.eax;
    const ZoneLeft left = zoneRegisters(app, joystick, axis, false, percent, *zone);
    args.eax = left.eax;
    if (left.ebx)
        args.ebx = *left.ebx;
    if (left.ecx)
        args.ecx = *left.ecx;
    if (left.edx)
        args.edx = *left.edx;
    args.esi = kept.esi;
    args.ebp = kept.ebp;
    ts.sp += 4;
    return args.eax;
}

namespace
{

/* 0x435b60: the joysticks with force feedback reset (DISFFC_RESET)
 * (resetEach), looked for first when there are none.  False when the game
 * stopped. */
bool resetEach(App* app, ThreadState& ts)
{
    for (u32 joystick = 0; s32(joystick) < s32(at(app, kJoystickCount)); ++joystick)
    {
        const u32 device = joystickDevice(app, joystick);
        if (device != 0 && forceFeedback(app, joystick)
            && !callMethod(app, ts, Object::Device, device, kSendForceFeedbackCommand, {kReset}))
            return false;
    }
    return true;
}

bool resetJoysticks(App* app, ThreadState& ts)
{
    const GuestFrame frame(ts, 0x14, 0);  // ebx, ecx, edx, esi, ebp pushed
    if (at(app, kJoystickCount) == 0)
    {
        const GuestFrame slot(ts, 0, 4);
        if (!lookForJoysticks(app, ts))
            return false;
    }
    return resetEach(app, ts);
}

}  // namespace

/* 0x435b60: resetJoysticks; eax as the code leaves it: the last joystick
 * there's record's number times 17, or its reset's HRESULT. */
nfs::u32 resetForceFeedback(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept{args.ebx, args.ecx, args.edx, args.esi, args.edi, args.ebp};
    {
        const GuestFrame frame(ts, 0x14, 0);  // ebx, ecx, edx, esi, ebp pushed
        if (at(app, kJoystickCount) == 0)
        {
            const GuestFrame slot(ts, 0, 4);
            const std::optional<u32> count = lookForJoysticks(app, ts);
            if (!count)
                return args.eax;
            args.eax = *count;
        }
        if (!resetEach(app, ts))
            return args.eax;
    }
    for (u32 joystick = at(app, kJoystickCount); s32(joystick) > 0;)
    {
        --joystick;
        if (joystickDevice(app, joystick) == 0)
            continue;
        if (!forceFeedback(app, joystick))
            args.eax = joystick * 17;
        break;
    }
    args.ebx = kept.ebx;
    args.ecx = kept.ecx;
    args.edx = kept.edx;
    args.esi = kept.esi;
    args.ebp = kept.ebp;
    ts.sp += 4;
    return args.eax;
}

namespace
{

/* SetProperty(the joystick, `property`, the DIPROPDWORD at `record`) as
 * setAutocenter's and setGain's code calls it: the joystick's table of
 * methods in ecx, the DIPROPDWORD's address left in edx.  Its HRESULT;
 * nothing when the game stopped. */
std::optional<u32> setDeviceProperty(App* app, ThreadState& ts, u32 device, u32 property, u32 record)
{
    push(app, ts, record);
    push(app, ts, property);
    push(app, ts, device);
    return directInputResult(app, ts, Object::Device, kSetProperty);
}

/* A DIPROPDWORD for the whole device (DIPH_DEVICE): {0x14, 0x10, 0, 0, `value`}. */
void deviceRecord(App* app, u32 record, u32 value)
{
    put(app, record, 0x14);
    put(app, record + 4, 0x10);
    put(app, record + 8, 0);
    put(app, record + 0xc, 0);
    put(app, record + 0x10, value);
}

/* 0x435bc0: joystick `joystick`'s autocentring on (`on` not 0) or off
 * (DIPROP_AUTOCENTER), where it has force feedback; every joystick's for all
 * of them (0x10) or every device (0x13).  False when the game stopped. */
bool autocenter(App* app, ThreadState& ts, u32 joystick, u32 on)
{
    const GuestFrame frame(ts, 0x14, 0x14);  // ebx, ecx, esi, edi, ebp pushed; the DIPROPDWORD
    if (joystick == kAllJoysticks || joystick == kEveryDevice)
    {
        for (u32 each = 0; each < 0x10; ++each)
        {
            const GuestFrame slot(ts, 0, 4);
            if (!autocenter(app, ts, each, on))
                return false;
        }
        return true;
    }
    if (!isJoystick(joystick) || joystickDevice(app, joystick) == 0 || !forceFeedback(app, joystick))
        return true;
    const u32 record = frame - 0x14;
    deviceRecord(app, record, on != 0 ? 1 : 0);
    return setDeviceProperty(app, ts, joystickDevice(app, joystick), 9, record).has_value();
}

/* What setAutocenter's and setGain's code leaves in eax and edx for
 * `joystick` (the last joystick's for all of them), `asked` the word asked
 * for (edx): where the property was set, nothing (as the call left them);
 * else the joystick's number, times 17 where it is there, and `asked`. */
struct PropertyLeft
{
    std::optional<u32> eax, edx;
};

PropertyLeft propertyLeft(App* app, u32 joystick, u32 asked, bool seventeenInEdx)
{
    if (joystick == kAllJoysticks || joystick == kEveryDevice)
        joystick = 0xf;
    if (!isJoystick(joystick) || joystickDevice(app, joystick) == 0)
        return {joystick, asked};
    if (!forceFeedback(app, joystick))
        return {seventeenInEdx ? joystick : joystick * 17, seventeenInEdx ? joystick * 17 : asked};
    return {};
}

}  // namespace

/* 0x435bc0: autocenter joystick eax, on edx. */
nfs::u32 setAutocenter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept{args.ebx, args.ecx, args.edx, args.esi, args.edi, args.ebp};
    const u32 joystick = args.eax;
    if (!autocenter(app, ts, joystick, kept.edx))
        return args.eax;
    const PropertyLeft left = propertyLeft(app, joystick, kept.edx, true);
    if (left.eax)
        args.eax = *left.eax;
    if (left.edx)
        args.edx = *left.edx;
    args.ebx = kept.ebx;
    args.ecx = kept.ecx;
    args.esi = kept.esi;
    args.edi = kept.edi;
    args.ebp = kept.ebp;
    ts.sp += 4;
    return args.eax;
}

namespace
{

/* 0x435c60, 0x435cc0, 0x435d90: force-feedback command `command` to joystick
 * `joystick` (SendForceFeedbackCommand), where it has force feedback; to
 * every joystick for all of them (0x10) or every device (0x13).  False when
 * the game stopped. */
bool commandJoystick(App* app, ThreadState& ts, u32 joystick, u32 command)
{
    if (joystick == kAllJoysticks || joystick == kEveryDevice)
    {
        for (u32 each = 0; each < 0x10; ++each)
            if (!commandJoystick(app, ts, each, command))
                return false;
        return true;
    }
    if (!isJoystick(joystick) || joystickDevice(app, joystick) == 0 || !forceFeedback(app, joystick))
        return true;
    return callMethod(app, ts, Object::Device, joystickDevice(app, joystick), kSendForceFeedbackCommand, {command})
        .has_value();
}

/* eax as a joystick's command's code leaves it (the last joystick's for all
 * of them): the HRESULT where the command was sent, as the call left it;
 * else the joystick's number, times 17 where the code has made its record's
 * there (`seventeen`). */
u32 commandLeft(App* app, u32 joystick, u32 sent, bool seventeen)
{
    if (joystick == kAllJoysticks || joystick == kEveryDevice)
        joystick = 0xf;
    if (!isJoystick(joystick) || joystickDevice(app, joystick) == 0)
        return joystick;
    if (!forceFeedback(app, joystick))
        return seventeen ? joystick * 17 : joystick;
    return sent;
}

}  // namespace

/* 0x435d20: joystick `joystick`'s effects stopped (stopAllEffects) and its
 * actuators on (DISFFC_SETACTUATORSON), where it has force feedback; every
 * joystick's for all of them (0x10) or every device (0x13).  False when the
 * game stopped. */
bool actuatorsOnFor(App* app, ThreadState& ts, u32 joystick)
{
    if (joystick == kAllJoysticks || joystick == kEveryDevice)
    {
        for (u32 each = 0; each < 0x10; ++each)
            if (!actuatorsOnFor(app, ts, each))
                return false;
        return true;
    }
    if (!isJoystick(joystick) || joystickDevice(app, joystick) == 0 || !forceFeedback(app, joystick))
        return true;
    if (!stopAllEffects(app, ts, joystick))
        return false;
    return callMethod(app, ts, Object::Device, joystickDevice(app, joystick), kSendForceFeedbackCommand,
                      {kActuatorsOn})
        .has_value();
}

/* 0x435c60: joystick eax's effects paused (DISFFC_PAUSE). */
nfs::u32 pauseEffects(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept{args.ebx, args.ecx, args.edx, args.esi, args.edi, args.ebp};
    const u32 joystick = args.eax;
    if (!commandJoystick(app, ts, joystick, kPause))
        return args.eax;
    args.eax = commandLeft(app, joystick, args.eax, true);
    args.ecx = kept.ecx;
    args.edx = kept.edx;
    args.ebp = kept.ebp;
    ts.sp += 4;
    return args.eax;
}

/* 0x435cc0: joystick eax's effects going on (DISFFC_CONTINUE). */
nfs::u32 continueEffects(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept{args.ebx, args.ecx, args.edx, args.esi, args.edi, args.ebp};
    const u32 joystick = args.eax;
    if (!commandJoystick(app, ts, joystick, kContinue))
        return args.eax;
    args.eax = commandLeft(app, joystick, args.eax, true);
    args.ecx = kept.ecx;
    args.edx = kept.edx;
    args.ebp = kept.ebp;
    ts.sp += 4;
    return args.eax;
}

/* 0x435d20: actuatorsOnFor joystick eax. */
nfs::u32 actuatorsOn(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept{args.ebx, args.ecx, args.edx, args.esi, args.edi, args.ebp};
    const u32 joystick = args.eax;
    if (!actuatorsOnFor(app, ts, joystick))
        return args.eax;
    args.eax = commandLeft(app, joystick, args.eax, false);
    args.ecx = kept.ecx;
    args.edx = kept.edx;
    args.ebp = kept.ebp;
    ts.sp += 4;
    return args.eax;
}

/* 0x435d90's core: joystick `joystick`'s actuators off (commandJoystick of
 * DISFFC_SETACTUATORSOFF).  False when the game stopped. */
bool actuatorsOffFor(App* app, ThreadState& ts, u32 joystick)
{
    return commandJoystick(app, ts, joystick, kActuatorsOff);
}

/* 0x435d90: joystick eax's actuators off (DISFFC_SETACTUATORSOFF). */
nfs::u32 actuatorsOff(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept{args.ebx, args.ecx, args.edx, args.esi, args.edi, args.ebp};
    const u32 joystick = args.eax;
    if (!commandJoystick(app, ts, joystick, kActuatorsOff))
        return args.eax;
    args.eax = commandLeft(app, joystick, args.eax, true);
    args.ecx = kept.ecx;
    args.edx = kept.edx;
    args.ebp = kept.ebp;
    ts.sp += 4;
    return args.eax;
}

namespace
{

/* effectMethod for an effect's number as a signed word (none below 0).
 * False when the game stopped. */
bool effectKeeping(App* app, ThreadState& ts, u32 effect, u32 method)
{
    const u32 number = u32(s32(i16(effect)));
    return s32(number) < 0 || effectMethod(app, ts, number, method).has_value();
}

}  // namespace

/* 0x435df0: joystick `joystick`'s force-feedback gain set to `percent`
 * (DIPROP_FFGAIN in hundredths: 100 most, 0 for less than none), where it
 * has force feedback, the effects unloaded around it and downloaded again;
 * every joystick's for all of them (0x10) or every device (0x13).  False
 * when the game stopped. */
bool gain(App* app, ThreadState& ts, u32 joystick, u32 percent)
{
    const GuestFrame frame(ts, 0x14, 0x14);  // ebx, ecx, esi, edi, ebp pushed; the DIPROPDWORD
    if (joystick == kAllJoysticks || joystick == kEveryDevice)
    {
        for (u32 each = 0; each < 0x10; ++each)
        {
            const GuestFrame slot(ts, 0, 4);
            if (!gain(app, ts, each, percent))
                return false;
        }
        return true;
    }
    if (!isJoystick(joystick) || joystickDevice(app, joystick) == 0 || !forceFeedback(app, joystick))
        return true;
    const u32 record = frame - 0x14;
    put(app, record + 4, 0x10);
    put(app, record + 8, 0);
    put(app, record + 0xc, 0);
    put(app, record, 0x14);
    put(app, record + 0x10, s32(percent) > 100 ? 10000 : s32(percent) >= 0 ? percent * 100 : 0);
    for (u32 effect = 0; s32(effect) < s32(at(app, kEffectCount)); ++effect)
        if (!effectKeeping(app, ts, effect, kUnload))
            return false;
    if (!setDeviceProperty(app, ts, joystickDevice(app, joystick), 7, record))
        return false;
    for (u32 effect = 0; s32(effect) < s32(at(app, kEffectCount)); ++effect)
        if (!effectKeeping(app, ts, effect, kDownload))
            return false;
    return true;
}

/* 0x435df0: gain for joystick eax, percent edx. */
nfs::u32 setGain(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept{args.ebx, args.ecx, args.edx, args.esi, args.edi, args.ebp};
    const u32 joystick = args.eax;
    if (!gain(app, ts, joystick, kept.edx))
        return args.eax;
    const PropertyLeft left = propertyLeft(app, joystick, kept.edx, false);
    if (left.eax)
        args.eax = *left.eax;
    if (left.edx)
        args.edx = *left.edx;
    args.ebx = kept.ebx;
    args.ecx = kept.ecx;
    args.esi = kept.esi;
    args.edi = kept.edi;
    args.ebp = kept.ebp;
    ts.sp += 4;
    return args.eax;
}

/* 0x435f10: an effect made on joystick `joystick` from the description at
 * `description` -- its type at 0, its type's parameters (0x30 bytes) at 4,
 * its duration in ms (0xffffff for ever) at 0x34, gain at 0x38, axes (0 x,
 * 1 y, else both) at 0x3c, trigger button (-1 none) at 0x40, repeat at
 * 0x44, direction in degrees at 0x48 -- into the next effect's record, a
 * DIEFFECT, created (CreateEffect) and unloaded straight away.  Its number,
 * or -1: no room for it, no force feedback, a type past 12, or DirectInput
 * would not.  As the code leaves it at the call, edx the description.
 * Nothing when the game stopped. */
std::optional<u32> makeEffect(App* app, ThreadState& ts, u32 joystick, u32 description)
{
    constexpr u32 kNone = 0xffffffff, kForever = 0xffffff, kNoButton = 0xffffffff;
    if (s32(at(app, kEffectCount)) >= s32(kEffectsMade) || !isJoystick(joystick)
        || joystickDevice(app, joystick) == 0 || !forceFeedback(app, joystick))
        return kNone;
    const u32 number = at(app, kEffectCount), record = number * kEffectBytes;
    const u32 duration = at(app, description + 0x34);
    put(app, record + kEffectDuration, duration == kForever ? 0xffffffff : duration * 1000);  // in microseconds
    put(app, record + kEffectGain, at(app, description + 0x38));
    const u32 button = at(app, description + 0x40);
    put(app, record + kEffectTrigger, button == kNoButton ? button : button + 0x30);  // its offset in DIJOYSTATE
    put(app, record + kEffectRepeat, at(app, description + 0x44));
    put(app, record + kEffectDirection, at(app, description + 0x48) * 100);
    // Its axes: x, y, or both.
    const u32 axes = at(app, description + 0x3c);
    if (axes == 0)
    {
        put(app, record + kEffectAxisList, kEffectAxes);
        put(app, kEffectAxes, 0);
        put(app, record + kEffectAxisCount, 1);
    }
    else if (axes == 1)
    {
        put(app, record + kEffectAxisCount, 1);
        put(app, record + kEffectAxisList, kEffectAxes);
        put(app, kEffectAxes, 4);
    }
    else
    {
        put(app, record + kEffectAxisList, kEffectAxes);
        put(app, record + kEffectAxisCount, 2);
        put(app, kEffectAxes, 0);
        put(app, kEffectAxes + 4, 4);
    }
    // Its type's parameters, to the record.
    for (u32 i = 0; i < 0x30; i += 4)
        put(app, kEffectRecords + 0x34 + record + i, at(app, description + 4 + i));
    // The rest of the DIEFFECT.
    const u32 effect = kEffectRecords + record;
    put(app, effect, 0x34);                        // its size
    put(app, record + kEffectFlags, 0x22);         // DIEFF_OBJECTOFFSETS | DIEFF_POLAR
    put(app, record + kEffectPeriod, 0x7d0);
    put(app, record + kEffectDirection2, 0);
    put(app, record + kEffectEnvelope, 0);
    put(app, record + kEffectPlaying, 0);
    put(app, record + kEffectParameters, effect + 0x34);
    put(app, record + kEffectDirections, effect + 0x78);
    put(app, record + kEffectParameters, effect + 0x34);
    const u32 type = at(app, description);
    put(app, record + kEffectType, type);
    if (type > 0xc)
        return kNone;
    // The parameters' size by the type, as the game's jump table has it.
    u32 bytes;
    switch (at(app, kParameterSizes + type * 4))
    {
    case 0x436103:  // a constant force
        bytes = 4;
        break;
    case 0x43611d:  // a ramp
        bytes = 8;
        break;
    case 0x436129:  // a square wave
    case 0x436135:  // the other waves
        bytes = 0x10;
        break;
    case 0x43614f:  // a condition
        bytes = 0x18;
        break;
    default:  // 0x436169: a condition on each of two axes
        bytes = 0x30;
        break;
    }
    put(app, record + kEffectParameterBytes, bytes);
    // CreateEffect(the joystick, its type's GUID, the record, &the effect, none)
    const u32 device = joystickDevice(app, joystick);
    push(app, ts, 0);
    push(app, ts, kEffects + number * 4);
    push(app, ts, effect);
    push(app, ts, at(app, kEffectGuids + type * 4));
    push(app, ts, device);
    const std::optional<u32> created = directInputResult(app, ts, Object::Device, kCreateEffect);
    if (!created)
        return std::nullopt;
    if (*created != 0)
        return kNone;
    if (!effectKeeping(app, ts, number, kUnload))
        return std::nullopt;
    put(app, kEffectCount, number + 1);
    return number;
}

/* 0x435f10: makeEffect on joystick eax from the description at edx; edx the
 * effects' count after it where it was made, as the code leaves it. */
nfs::u32 createEffect(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept{args.ebx, args.ecx, args.edx, args.esi, args.edi, args.ebp};
    const std::optional<u32> made = makeEffect(app, ts, args.eax, args.edx);
    if (!made)
        return args.eax;
    args.eax = *made;
    if (*made != 0xffffffff)
        args.edx = *made + 1;
    args.ebx = kept.ebx;
    args.ecx = kept.ecx;
    args.esi = kept.esi;
    args.edi = kept.edi;
    args.ebp = kept.ebp;
    ts.sp += 4;
    return args.eax;
}

/* 0x4361e0: effect `effect` (0 or more) started, once (Start(1, 0)): it
 * plays where DirectInput says so.  Whether it does; nothing when the game
 * stopped. */
std::optional<u32> startOnce(App* app, ThreadState& ts, u32 word)
{
    const u32 effect = u32(s32(i16(word)));
    if (s32(effect) < 0)
        return word;
    const std::optional<u32> result =
        callMethod(app, ts, Object::Effect, effectObject(app, effect), kStart, {1, 0});
    if (!result)
        return std::nullopt;
    put(app, kEffectPlaying + effect * kEffectBytes, *result == 0 ? 1 : 0);
    return effect * kEffectBytes;
}

/* 0x436230: effect `effect` (0 or more) stopped; it plays no more.  False
 * when the game stopped. */
std::optional<u32> stopOne(App* app, ThreadState& ts, u32 word)
{
    const u32 effect = u32(s32(i16(word)));
    if (s32(effect) < 0)
        return word;
    if (!callMethod(app, ts, Object::Effect, effectObject(app, effect), kStop, {}))
        return std::nullopt;
    put(app, kEffectPlaying + effect * kEffectBytes, 0);
    return effect * 17;
}

/* What startOnce leaves of effect `effect` (0 or more): whether its record
 * says it plays. */
bool effectStarted(App* app, u32 effect)
{
    return at(app, kEffectPlaying + effect * kEffectBytes) != 0;
}

/* 0x4361e0: startOnce for effect ax, where it is one; eax its record's offset
 * as the code leaves it. */
nfs::u32 startEffect(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept{args.ebx, args.ecx, args.edx, args.esi, args.edi, args.ebp};
    const u32 effect = effectIn(args.eax);
    if (s32(effect) >= 0)
    {
        const std::optional<u32> started = startOnce(app, ts, effect);
        if (!started)
            return args.eax;
        args.eax = *started;
    }
    args.ebx = kept.ebx;
    args.ecx = kept.ecx;
    args.edx = kept.edx;
    args.ebp = kept.ebp;
    ts.sp += 4;
    return args.eax;
}

/* 0x436230: stopOne for effect ax, where it is one; eax its number times 17,
 * as the code leaves it. */
nfs::u32 stopEffect(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept{args.ebx, args.ecx, args.edx, args.esi, args.edi, args.ebp};
    const u32 effect = effectIn(args.eax);
    if (s32(effect) >= 0)
    {
        const std::optional<u32> stopped = stopOne(app, ts, effect);
        if (!stopped)
            return args.eax;
        args.eax = *stopped;
    }
    args.ebx = kept.ebx;
    args.ecx = kept.ecx;
    args.edx = kept.edx;
    args.ebp = kept.ebp;
    ts.sp += 4;
    return args.eax;
}

namespace
{

/* 0x436270: joystick `joystick`'s effects all stopped (DISFFC_STOPALL), where
 * it has force feedback, and none of the 30 plays; each joystick there is
 * for all of them (0x10) or every device (0x13).  False when the game
 * stopped. */
bool stopAllEffects(App* app, ThreadState& ts, u32 joystick)
{
    if (joystick == kAllJoysticks || joystick == kEveryDevice)
    {
        for (u32 each = 0; s32(each) < s32(at(app, kJoystickCount)); ++each)
            if (!stopAllEffects(app, ts, each))
                return false;
        return true;
    }
    if (!isJoystick(joystick))
        return true;
    const u32 device = joystickDevice(app, joystick);
    if (device == 0 || !forceFeedback(app, joystick))
        return true;
    if (!callMethod(app, ts, Object::Device, device, kSendForceFeedbackCommand, {kStopAll}))
        return false;
    for (u32 effect = 0; effect < kEffectsMade; ++effect)
        put(app, kEffectPlaying + effect * kEffectBytes, 0);
    return true;
}

/* eax as stopEffects' code leaves it for `joystick` (the last joystick's for
 * all of them): the last effect's number times 17 where it stopped them, the
 * joystick's times 17 where it is there, else its number. */
u32 stopEffectsLeft(App* app, u32 joystick)
{
    if (joystick == kAllJoysticks || joystick == kEveryDevice)
    {
        const u32 count = at(app, kJoystickCount);
        return s32(count) > 0 ? stopEffectsLeft(app, count - 1) : joystick;
    }
    if (!isJoystick(joystick) || joystickDevice(app, joystick) == 0)
        return joystick;
    return forceFeedback(app, joystick) ? (kEffectsMade - 1) * 17 : joystick * 17;
}

/* 0x4362f0, 0x436310: method `method` of effect `effect`'s (0 or more): its
 * HRESULT; nothing when the game stopped. */
std::optional<u32> effectMethod(App* app, ThreadState& ts, u32 effect, u32 method)
{
    return callMethod(app, ts, Object::Effect, effectObject(app, effect), method, {});
}

}  // namespace

/* 0x436270: stopAllEffects for joystick eax. */
nfs::u32 stopEffects(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept{args.ebx, args.ecx, args.edx, args.esi, args.edi, args.ebp};
    const u32 joystick = args.eax;
    if (!stopAllEffects(app, ts, joystick))
        return args.eax;
    args.eax = stopEffectsLeft(app, joystick);
    args.ebx = kept.ebx;
    args.ecx = kept.ecx;
    args.edx = kept.edx;
    args.esi = kept.esi;
    args.ebp = kept.ebp;
    ts.sp += 4;
    return args.eax;
}

/* 0x4362f0: effect ax downloaded to its joystick. */
nfs::u32 downloadEffect(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept{args.ebx, args.ecx, args.edx, args.esi, args.edi, args.ebp};
    const u32 effect = effectIn(args.eax);
    if (s32(effect) >= 0)
    {
        const std::optional<u32> result = effectMethod(app, ts, effect, kDownload);
        if (!result)
            return args.eax;
        args.eax = *result;
    }
    args.ecx = kept.ecx;
    args.edx = kept.edx;
    args.ebp = kept.ebp;
    ts.sp += 4;
    return args.eax;
}

/* 0x436310's core: effect `effect` (0 or more) unloaded from its joystick
 * (effectMethod of Unload): its HRESULT; nothing when the game stopped. */
std::optional<u32> unloadOne(App* app, ThreadState& ts, u32 word)
{
    const u32 effect = u32(s32(i16(word)));
    if (s32(effect) < 0)
        return word;
    return effectMethod(app, ts, effect, kUnload);
}

/* 0x436310: effect ax unloaded from its joystick. */
nfs::u32 unloadEffect(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept{args.ebx, args.ecx, args.edx, args.esi, args.edi, args.ebp};
    const u32 effect = effectIn(args.eax);
    if (s32(effect) >= 0)
    {
        const std::optional<u32> result = effectMethod(app, ts, effect, kUnload);
        if (!result)
            return args.eax;
        args.eax = *result;
    }
    args.ecx = kept.ecx;
    args.edx = kept.edx;
    args.ebp = kept.ebp;
    ts.sp += 4;
    return args.eax;
}

namespace
{

// DIEP_: what SetParameters is told changed.
constexpr u32 kNewDuration = 1, kNewGain = 4, kNewTrigger = 8, kNewDirection = 0x40, kNewEnvelope = 0x80;
constexpr u32 kNewParameters = 0x100;
constexpr u32 kEffectParameterValues = 0x5f2924;  // the type's parameters, where kEffectParameters points
constexpr u32 kMostDegrees = 360, kLongest = 1500, kForEver = 0xffffff;  // ms
constexpr u32 kInfinite = 0xffffffff, kNominalMax = 10000;              // INFINITE, DI_FFNOMINALMAX
constexpr u32 kNoTrigger = 0xffffffff, kButtonOffset = 0x30;          // DIEB_NOTRIGGER, DIJOFS_BUTTON0
constexpr s32 kTriggerButtons = 0x20;
// changeEffect's `how`: started where it does not play; the same only where
// something changed; downloaded where nothing changed.
constexpr u32 kPlay = 1, kPlayChanged = 2, kDownloadSame = 3;

/* repe cmpsb: whether `bytes` from `a` are those from `b`. */
bool sameBytes(App* app, u32 a, u32 b, u32 bytes)
{
    for (u32 i = 0; i < bytes; ++i)
        if (byte(app, a + i) != byte(app, b + i))
            return false;
    return true;
}

/* rep movsd, then rep movsb of what is left: `bytes` from `from` to `to`. */
void copyBytes(App* app, u32 to, u32 from, u32 bytes)
{
    for (u32 n = bytes >> 2; n != 0; --n, from += 4, to += 4)
        put(app, to, at(app, from));
    for (u32 n = bytes & 3; n != 0; --n, ++from, ++to)
        putByte(app, to, byte(app, from));
}

}  // namespace

/* 0x436330: effect `effect` (0 or more) changed where it is asked to, each
 * word asked for made DirectInput's in place as it goes: [directionAt] its
 * direction in degrees (0 to 360) into hundredths, [durationAt] its duration
 * in ms (1500 most, 0xffffff for ever) into microseconds, [gainAt] its gain
 * in percent, its type's parameters at parametersAt, [triggerAt] its trigger
 * button (-1 none), its envelope's 4 words at envelopeAt.  What changed set
 * (SetParameters), and with `how` 1 or 2 it is started where it does not
 * play; where nothing changed, with 1 started so, with 3 downloaded.
 * Whether anything changed; nothing when the game stopped. */
std::optional<bool> changeEffect(App* app, ThreadState& ts, u32 effect, u32 how, u32 directionAt, u32 durationAt,
                                 u32 gainAt, u32 parametersAt, u32 triggerAt, u32 envelopeAt)
{
    const u32 record = effect * kEffectBytes;
    u32 changed = 0;
    if (directionAt != 0)
    {
        const u32 degrees = at(app, directionAt);
        if (s32(degrees) > s32(kMostDegrees))
            put(app, directionAt, kMostDegrees);
        else if (s32(degrees) < 0)
            put(app, directionAt, 0);
        put(app, directionAt, at(app, directionAt) * 100);
        const u32 direction = at(app, directionAt);
        if (at(app, kEffectDirection + record) != direction)
        {
            changed |= kNewDirection;
            put(app, kEffectDirection + record, direction);
        }
    }
    if (durationAt != 0)
    {
        const u32 ms = at(app, durationAt);
        if (ms == kForEver)
            put(app, durationAt, kInfinite);
        else
        {
            if (ms > kLongest)
                put(app, durationAt, kLongest);
            put(app, durationAt, at(app, durationAt) * 1000);
        }
        const u32 duration = at(app, durationAt);
        if (duration != at(app, kEffectDuration + record))
        {
            changed |= kNewDuration;
            put(app, kEffectDuration + record, duration);
        }
    }
    if (gainAt != 0)
    {
        const u32 percent = at(app, gainAt) * 100;
        const u32 gain = percent > kNominalMax ? kNominalMax : percent;
        if (gain != at(app, kEffectGain + record))
        {
            changed |= kNewGain;
            put(app, kEffectGain + record, gain);
        }
    }
    if (parametersAt != 0
        && !sameBytes(app, kEffectParameterValues + record, parametersAt, at(app, kEffectParameterBytes + record)))
    {
        changed |= kNewParameters;
        copyBytes(app, kEffectParameterValues + record, parametersAt, at(app, kEffectParameterBytes + record));
    }
    if (triggerAt != 0)
    {
        const u32 button = at(app, triggerAt);
        if (s32(button) < kTriggerButtons)
        {
            changed |= kNewTrigger;
            put(app, kEffectTrigger + record, button == kNoTrigger ? button : button + kButtonOffset);
        }
    }
    if (envelopeAt != 0)
    {
        const u32 held = kEffectEnvelopeValues + record;
        const bool same = at(app, envelopeAt) == at(app, held) && at(app, envelopeAt + 4) == at(app, held + 4)
                          && at(app, held + 8) == at(app, envelopeAt + 8)
                          && at(app, envelopeAt + 0xc) == at(app, held + 0xc);
        if (!same)
        {
            changed |= kNewEnvelope;
            for (u32 i = 0; i < 0x10; i += 4)
                put(app, held + i, at(app, envelopeAt + i));
        }
    }
    // effectPlaying, then startEffect where it does not play.  False when the game stopped.
    auto startIfStill = [&]() -> bool {
        if (effectPlays(app, effect) != 0)
            return true;
        const GuestFrame call(ts, 0, 4);  // the return address's slot
        return startOnce(app, ts, effect).has_value();
    };
    if (changed == 0)
    {
        if (how == kPlay && !startIfStill())
            return std::nullopt;
        if (how == kDownloadSame)
        {
            const GuestFrame call(ts, 0, 4);
            if (!effectMethod(app, ts, effect, kDownload))
                return std::nullopt;
        }
        return false;
    }
    // SetParameters(the effect, its record, what changed)
    if (!callMethod(app, ts, Object::Effect, effectObject(app, effect), kSetParameters,
                    {kEffectRecords + record, changed}))
        return std::nullopt;
    if ((how == kPlay || how == kPlayChanged) && !startIfStill())
        return std::nullopt;
    return true;
}

/* 0x436330 (stdcall, 4 words): changeEffect for effect ax, `how` edx,
 * directionAt ebx, durationAt ecx, the stack's words the rest; 0 for none
 * (ax below 0). */
nfs::u32 updateEffect(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept{args.ebx, args.ecx, args.edx, args.esi, args.edi, args.ebp};
    const u32 effect = effectIn(args.eax);
    bool changed = false;
    if (s32(effect) >= 0)
    {
        const std::optional<bool> done =
            changeEffect(app, ts, effect, args.edx, args.ebx, args.ecx, at(app, ts.sp + 4), at(app, ts.sp + 8),
                         at(app, ts.sp + 0xc), at(app, ts.sp + 0x10));
        if (!done)
            return args.eax;
        changed = *done;
    }
    args.eax = changed ? 1 : 0;
    args.esi = kept.esi;
    args.edi = kept.edi;
    args.ebp = kept.ebp;
    ts.sp += 4 + 0x10;
    return args.eax;
}

namespace
{

/* The game's window (AcquireWindow).  Nothing when the game stopped. */
std::optional<u32> gameWindow(App* app, ThreadState& ts)
{
    const u32 window = guest::direct<0x4eba70, ea::acquireWindow>(app, ts);
    if (ts.terminate)
        return std::nullopt;
    return window;
}

/* 0x436610: device `device`'s cooperative level `flags` with the game's
 * window (SetCooperativeLevel, the window asked again for it); each
 * joystick's for all of them (0x10).  1, or 0 where DirectInput would not or
 * there is no window; 1 for a device that is not there.  As the code leaves
 * it at the call, edx a joystick's device, else `flags`.  Nothing when the
 * game stopped. */
std::optional<u32> cooperate(App* app, ThreadState& ts, u32 device, u32 flags)
{
    const std::optional<u32> window = gameWindow(app, ts);
    if (!window)
        return std::nullopt;
    if (*window == 0)
        return 0;
    auto level = [&](u32 slot) -> std::optional<u32> {
        const std::optional<u32> again = gameWindow(app, ts);
        if (!again)
            return std::nullopt;
        push(app, ts, flags);
        push(app, ts, *again);
        push(app, ts, at(app, slot));
        const std::optional<u32> set = directInputResult(app, ts, Object::Device, kSetCooperativeLevel);
        if (!set)
            return std::nullopt;
        return *set != 0 ? 0 : 1;
    };
    if (s32(device) < 0x10)
    {
        const u32 slot = kJoysticks + device * 4;
        if (at(app, slot) != 0)
            return level(slot);
    }
    else if (device == kAllJoysticks)
    {
        for (u32 each = 0; each < 0x10; ++each)
            if (!cooperate(app, ts, each, flags))
                return std::nullopt;
    }
    else if (device == kTheMouse)
        return level(kMouse);
    else if (device == kTheKeyboard)
        return level(kKeyboard);
    return 1;
}

}  // namespace

/* 0x436610: cooperate device eax, flags edx; edx as the code leaves it. */
nfs::u32 setCooperativeLevel(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept{args.ebx, args.ecx, args.edx, args.esi, args.edi, args.ebp};
    const std::optional<u32> done = cooperate(app, ts, args.eax, args.edx);
    if (!done)
        return args.eax;
    args.eax = *done;
    args.ebx = kept.ebx;
    args.ecx = kept.ecx;
    args.esi = kept.esi;
    args.edi = kept.edi;
    args.ebp = kept.ebp;
    ts.sp += 4;
    return args.eax;
}

namespace
{

/* 0x4368c0: whether Acquire's HRESULT says the device is acquired: DI_OK, or
 * S_FALSE (it was). */
bool acquiredBy(u32 result)
{
    return result == 0 || result == 1;
}

/* 0x4366e0: device `device` acquired (Acquire), where DirectInput is started
 * and the device is there: whether it is (acquiredBy); for all the joysticks
 * (0x10) how many are, for every device (0x13) the mouse's, the keyboard's
 * and that added up; 0 past 0x13.  Nothing when the game stopped. */
std::optional<u32> acquire(App* app, ThreadState& ts, u32 device)
{
    if (at(app, kDirectInput) == 0)
        return 0;
    if (device == kEveryDevice)
    {
        const std::optional<u32> mouse = acquire(app, ts, kTheMouse);
        if (!mouse)
            return std::nullopt;
        const std::optional<u32> keyboard = acquire(app, ts, kTheKeyboard);
        if (!keyboard)
            return std::nullopt;
        const std::optional<u32> joysticks = acquire(app, ts, kAllJoysticks);
        if (!joysticks)
            return std::nullopt;
        return *joysticks + (*mouse + *keyboard);
    }
    auto one = [&](u32 object) -> std::optional<u32> {
        if (object == 0)
            return 0;
        const std::optional<u32> result = callMethod(app, ts, Object::Device, object, kAcquire, {});
        if (!result)
            return std::nullopt;
        return acquiredBy(*result) ? 1 : 0;
    };
    if (device == kTheMouse)
        return one(at(app, kMouse));
    if (device == kTheKeyboard)
        return one(at(app, kKeyboard));
    if (device == kAllJoysticks)
    {
        u32 acquired = 0;
        for (u32 joystick = 0; joystick < 0x10; ++joystick)
        {
            if (joystickDevice(app, joystick) == 0)
                continue;
            const std::optional<u32> done = one(joystickDevice(app, joystick));
            if (!done)
                return std::nullopt;
            if (*done != 0)
                ++acquired;
        }
        return acquired;
    }
    if (s32(device) > 0x13)
        return 0;
    return one(joystickDevice(app, device));
}

/* 0x436800: device `device` let go (Unacquire), where it is there; each
 * joystick for all of them (0x10), the mouse, the keyboard and the joysticks
 * for every device (0x13).  False when the game stopped. */
bool letGo(App* app, ThreadState& ts, u32 device)
{
    if (device == kEveryDevice)
    {
        for (const u32 each : {kTheMouse, kTheKeyboard, kAllJoysticks})
            if (!letGo(app, ts, each))
                return false;
        return true;
    }
    if (device == kAllJoysticks)
    {
        for (u32 each = 0; each < 0x10; ++each)
            if (!letGo(app, ts, each))
                return false;
        return true;
    }
    const u32 object = device == kTheMouse      ? at(app, kMouse)
                         : device == kTheKeyboard ? at(app, kKeyboard)
                         : isJoystick(device)     ? joystickDevice(app, device)
                                                  : 0;
    if (object == 0)
        return true;
    return callMethod(app, ts, Object::Device, object, kUnacquire, {}).has_value();
}

}  // namespace

/* 0x4366e0: acquire device eax. */
nfs::u32 acquireDevice(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept{args.ebx, args.ecx, args.edx, args.esi, args.edi, args.ebp};
    const std::optional<u32> acquired = acquire(app, ts, args.eax);
    if (!acquired)
        return args.eax;
    args.eax = *acquired;
    args.ebx = kept.ebx;
    args.ecx = kept.ecx;
    args.edx = kept.edx;
    args.esi = kept.esi;
    args.edi = kept.edi;
    args.ebp = kept.ebp;
    ts.sp += 4;
    return args.eax;
}

/* 0x436800: letGo device eax; eax as the code leaves it: the HRESULT where
 * the last device was let go, as the call left it; the last joystick's
 * number times 4 where it has none; else the device's number. */
nfs::u32 unacquireDevice(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept{args.ebx, args.ecx, args.edx, args.esi, args.edi, args.ebp};
    const u32 device = args.eax;
    if (!letGo(app, ts, device))
        return args.eax;
    const u32 last = device == kEveryDevice || device == kAllJoysticks ? 0xf : device;
    if (isJoystick(last) && joystickDevice(app, last) == 0)
        args.eax = last * 4;
    args.ebx = kept.ebx;
    args.ecx = kept.ecx;
    args.edx = kept.edx;
    args.esi = kept.esi;
    args.ebp = kept.ebp;
    ts.sp += 4;
    return args.eax;
}

/* 0x4368c0: acquiredBy HRESULT eax, 1 or 0. */
nfs::u32 acquireResult(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    (void)app;
    args.eax = acquiredBy(args.eax) ? 1 : 0;
    ts.sp += 4;
    return args.eax;
}

namespace
{

/* 0x4368f0: whether effect `effect` plays: not for none (below 0), nor for
 * one whose type is 0 (a constant force); else its record's flag. */
u32 effectPlays(App* app, u32 effect)
{
    if (s32(effect) < 0)
        return 0;
    const u32 record = effect * kEffectBytes;
    return at(app, record + kEffectType) == 0 ? 0 : at(app, record + kEffectPlaying);
}

/* 0x436930: the window's WM_SETCURSOR handler while the mouse is open: the
 * one before it called with its six words -- the first left in edx and the
 * last in ecx, as the code has them -- then no system cursor, which the port
 * has none of anyway (the game's own pointer is all there is).  False when the
 * game stopped. */
bool noCursor(App* app, ThreadState& ts, const u32 (&words)[6])
{
    if (at(app, kOldSetCursor) != 0)
    {
        nfs::Args registers;
        registers.ecx = words[5];
        registers.edx = words[0];
        if (!guest::pointerCall(app, ts, at(app, kOldSetCursor), registers,
                                {words[0], words[1], words[2], words[3], words[4], words[5]}))
            return false;
    }
    return true;
}

}  // namespace

/* 0x4368f0: effectPlays for effect ax. */
nfs::u32 effectPlaying(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = effectPlays(app, effectIn(args.eax));
    ts.sp += 4;
    return args.eax;
}

/* 0x436930: noCursor (stdcall, its six words); 1. */
nfs::u32 hideCursor(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept{args.ebx, args.ecx, args.edx, args.esi, args.edi, args.ebp};
    u32 words[6];
    for (u32 i = 0; i < 6; ++i)
        words[i] = at(app, ts.sp + 4 + 4 * i);
    if (!noCursor(app, ts, words))
        return args.eax;
    args.eax = 1;
    args.ebx = kept.ebx;
    args.esi = kept.esi;
    args.edi = kept.edi;
    args.ebp = kept.ebp;
    ts.sp += 4 + 0x18;
    return args.eax;
}

namespace
{

/* 0x436980: EnumDevices' callback for each joystick attached, `instance` its
 * DIDEVICEINSTANCE, `input` the DirectInput it was handed: the next record
 * named after its product (strncpy, 0x32 characters, ended), its kind from
 * its subtype (a wheel 2, a gamepad 4, else 1); the device made (into the
 * guest's stack), its format a joystick's, its IDirectInputDevice2 kept,
 * exclusive in the background -- one more joystick.  1 to go on, 0 to stop
 * when the cooperative level fails.  The registers the callees may keep are
 * set as the code leaves them at each call.  Nothing when the game
 * stopped. */
std::optional<u32> joystickAttached(App* app, ThreadState& ts, u32 instance, u32 input)
{
    const GuestFrame frame(ts, 0x10, 4);  // ebx, esi, edi, ebp pushed; the device made
    const u32 made = frame - 4;
    const u32 count = at(app, kJoystickCount), record = count * kJoystickBytes;
    putByte(app, kJoystickName + record, 0);
    // tszProductName
    guest::direct<0x4e0e30, watcom::clib::strncpy>(app, ts, kJoystickName + record, instance + 0x12c, 0x32);
    if (ts.terminate)
        return std::nullopt;
    putByte(app, kJoystickNameEnd + record, 0);
    const u8 subtype = byte(app, instance + 0x25);  // dwDevType's
    put(app, kJoystickRecords + record, subtype == 4 ? 4 : subtype == 6 ? 2 : 1);  // a gamepad, a wheel, a stick
    // CreateDevice(DirectInput, its guidInstance, &the device, none)
    std::optional<u32> result =
        callMethod(app, ts, Object::DirectInput, input, kCreateDevice, {instance + 4, made, 0});
    if (!result)
        return std::nullopt;
    if (*result != 0)
        return 1;
    auto release = [&] {
        push(app, ts, at(app, made));
        return directInputResult(app, ts, Object::Device, kRelease).has_value();
    };
    // SetDataFormat(the device, a joystick's), the device left in edx.
    push(app, ts, kJoystickFormat);
    push(app, ts, at(app, made));
    const std::optional<u32> formatted = directInputResult(app, ts, Object::Device, kSetDataFormat);
    if (!formatted)
        return std::nullopt;
    if (*formatted != 0)
    {
        if (!release())
            return std::nullopt;
        return 1;
    }
    // QueryInterface(the device, IID_IDirectInputDevice2A, &the joystick's)
    result = callMethod(app, ts, Object::Device, at(app, made), kQueryInterface,
                        {kDevice2Interface, kJoysticks + count * 4});
    if (!result)
        return std::nullopt;
    if (*result != 0)
    {
        if (!release())
            return std::nullopt;
        return 1;
    }
    result = cooperate(app, ts, count, 9);  // DISCL_EXCLUSIVE | DISCL_BACKGROUND
    if (!result)
        return std::nullopt;
    if (*result == 0)
        return 0;
    if (!release())
        return std::nullopt;
    put(app, kJoystickCount, at(app, kJoystickCount) + 1);
    return 1;
}

}  // namespace

/* 0x436980: joystickAttached (stdcall: the DIDEVICEINSTANCE, DirectInput);
 * ecx and edx as the code leaves them. */
nfs::u32 joystickFound(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept{args.ebx, args.ecx, args.edx, args.esi, args.edi, args.ebp};
    const std::optional<u32> more = joystickAttached(app, ts, at(app, ts.sp + 4), at(app, ts.sp + 8));
    if (!more)
        return args.eax;
    args.eax = *more;
    args.ebx = kept.ebx;
    args.esi = kept.esi;
    args.edi = kept.edi;
    args.ebp = kept.ebp;
    ts.sp += 4 + 8;
    return args.eax;
}

namespace
{

/* 0x436b10: joystick `joystick`'s dead zone (`which` 0) or saturation of
 * axis `axis` -- 8 the whole device -- asked for (`read`) or set to
 * `percent` (0 to 100), a DIPROPDWORD in hundredths on the guest's stack
 * (DIPROP_DEADZONE, DIPROP_SATURATION).  In percent, or -1 for no such
 * joystick or axis, or where DirectInput would not (the remainder then
 * none); nothing when the game stopped.  As the code leaves them at the
 * call: the device in ecx when asking, `which` there when setting, its table
 * of methods in edx. */
std::optional<Zone> axisZone(App* app, ThreadState& ts, u32 joystick, u32 axis, bool read, u32 which, u32 percent)
{
    if (!isJoystick(joystick) || s32(axis) > 8 || joystickDevice(app, joystick) == 0)
        return Zone{0xffffffff, std::nullopt};
    // (For all the joysticks the code would ask each: never, the number is a joystick's here.)
    const GuestFrame frame(ts, 0xc, 0x14);  // esi, edi, ebp pushed; the DIPROPDWORD
    const u32 record = frame - 0x14;
    put(app, record, 0x14);
    put(app, record + 4, 0x10);
    if (axis == 8)
    {
        put(app, record + 0xc, 0);  // DIPH_DEVICE
        put(app, record + 8, 0);
    }
    else
    {
        put(app, record + 8, at(app, kHatOffsets + axis * 4));
        put(app, record + 0xc, 1);  // DIPH_BYOFFSET
    }
    const u32 property = which == 0 ? 5 : 6;
    const u32 device = joystickDevice(app, joystick);
    std::optional<u32> result;
    if (read)
    {
        result = callMethod(app, ts, Object::Device, device, kGetProperty, {property, record});
    }
    else
    {
        const s32 clamped = s32(percent) < 0 ? 0 : s32(percent) > 100 ? 100 : s32(percent);
        put(app, record + 0x10, u32(clamped) * 100);
        result = callMethod(app, ts, Object::Device, device, kSetProperty, {property, record});
    }
    if (!result)
        return std::nullopt;
    if (s32(*result) < 0)
        return Zone{0xffffffff, std::nullopt};
    const s32 hundredths = s32(at(app, record + 0x10));
    return Zone{u32(hundredths / 100), u32(hundredths % 100)};
}

}  // namespace

/* 0x436b10: axisZone for joystick eax, axis edx, asked for where ebx is not
 * 0, which ecx, the stack's percent (stdcall, 1 word). */
nfs::u32 axisProperty(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Kept kept{args.ebx, args.ecx, args.edx, args.esi, args.edi, args.ebp};
    const u32 joystick = args.eax, axis = args.edx, which = args.ecx, percent = at(app, ts.sp + 4);
    const bool read = args.ebx != 0;
    const std::optional<Zone> zone = axisZone(app, ts, joystick, axis, read, which, percent);
    if (!zone)
        return args.eax;
    const ZoneLeft left = zoneRegisters(app, joystick, axis, read, percent, *zone);
    args.eax = left.eax;
    if (left.ebx)
        args.ebx = *left.ebx;
    if (left.ecx)
        args.ecx = *left.ecx;
    if (left.edx)
        args.edx = *left.edx;
    args.esi = kept.esi;
    args.edi = kept.edi;
    args.ebp = kept.ebp;
    ts.sp += 4 + 4;
    return args.eax;
}

}  // namespace controls
}  // namespace nfs3hp
