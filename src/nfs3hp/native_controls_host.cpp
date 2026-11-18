#include "native_controls_host.h"
#include "native_guest.h"
#include <lib/forcefeedback.h>
#include <lib/gamepad.h>
#include <lib/memmap.h>
#include <lib/window.h>
#include <winapi/types.h>
#include <SDL3/SDL.h>
#include <cstring>
#include <vector>

/* The game's DirectInput calls answered by the port (native_controls_host.h):
 * what the port's DirectInput did, without DirectInput -- no COM objects, no
 * tables of methods in the game's memory.  An object the game is handed is a
 * handle: an address of the port's in the game's memory that the game keeps
 * where it kept the object, and reads its table of methods from (0, never
 * called through: the game's side of DirectInput is native and calls here).
 * Behind the handles: the pads, the mouse and the keyboard of lib/gamepad.h,
 * and the force feedback of lib/forcefeedback.h. */
namespace nfs3hp
{
namespace controls
{

namespace
{

using win32::DWORD;
using win32::GUID;
using win32::HRESULT;
using win32::LONG;
using x86::reg32;

constexpr HRESULT kOk = 0, kFalse = 1;
constexpr HRESULT kNoInterface = 0x80004002, kInvalidParameter = 0x80070057;
constexpr HRESULT kObjectNotFound = 0x80070002, kNoAggregation = 0x80040110;

const GUID kSysKeyboard = { 0x6F1D2B61, 0xD5A0, 0x11CF, { 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };
const GUID kSysMouse = { 0x6F1D2B60, 0xD5A0, 0x11CF, { 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };
const GUID kDevice2 = { 0x5944E682, 0xC92E, 0x11CF, { 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };
const GUID kJoystick = { 0x4251809a, 0x67be, 0, { 'N', 'F', 'S', ' ', 'P', 'A', 'D', 0 } };
const GUID kForceDriver = { 0x4251809a, 0x67bf, 0xffff, { 'N', 'F', 'S', ' ', 'P', 'A', 'D', 0 } };

// DirectInput's structures as the game's memory holds them.
struct DeviceInstance
{
    DWORD dwSize;
    GUID guidInstance;
    GUID guidProduct;
    DWORD dwDevType;
    char tszInstanceName[win32::MAX_PATH];
    char tszProductName[win32::MAX_PATH];
    GUID guidFFDriver;
    x86::reg16 wUsagePage;
    x86::reg16 wUsage;
};

struct Capabilities
{
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwDevType;
    DWORD dwAxes;
    DWORD dwButtons;
    DWORD dwPOVs;
    DWORD dwFFSamplePeriod;
    DWORD dwFFMinTimeResolution;
    DWORD dwFirmwareRevision;
    DWORD dwHardwareRevision;
    DWORD dwFFDriverVersion;
};
constexpr DWORD kAttached = 0x1, kPolledDataFormat = 0x8, kForceFeedback = 0x100;

struct ObjectInstance
{
    DWORD dwSize;
    GUID guidType;
    DWORD dwOfs;
    DWORD dwType;
    DWORD dwFlags;
    char tszName[win32::MAX_PATH];
    DWORD dwFFMaxForce;
    DWORD dwFFForceResolution;
    x86::reg16 wCollectionNumber;
    x86::reg16 wDesignatorIndex;
    x86::reg16 wUsagePage;
    x86::reg16 wUsage;
    x86::reg16 dwDimension;
    x86::reg16 wExponent;
    x86::reg16 wReportId;
};

struct PropertyHeader
{
    DWORD dwSize;
    DWORD dwHeaderSize;
    DWORD dwObj;
    DWORD dwHow;
};
// MAKEDIPROP's: the property is the GUID pointer itself.
constexpr reg32 kDeadZone = 5, kSaturation = 6, kForceGain = 7;
constexpr DWORD kHowDevice = 0, kHowByOffset = 1;

struct JoystickState
{
    LONG lX;
    LONG lY;
    LONG lZ;
    LONG lRx;
    LONG lRy;
    LONG lRz;
    LONG rglSlider[2];
    DWORD rgdwPOV[4];
    x86::reg8 rgbButtons[32];
};

struct MouseState
{
    LONG lX;
    LONG lY;
    LONG lZ;
    x86::reg8 rgbButtons[4];
};

/* What a handle stands for. */
struct Entry
{
    bool used = false;
    Object kind = Object::DirectInput;
    unsigned references = 0;
    win32::Input* input = nullptr;
    win32::forcefeedback::Effect* effect = nullptr;
};

constexpr reg32 kHandles = 256;
win32::MemMap* s_handles = nullptr;
std::vector<Entry> s_entries(kHandles);

reg32 handleOf(reg32 index)
{
    return s_handles->getBlockStart() + 4 * index;
}

/* A new handle of `kind`; 0 when there is no room. */
reg32 newHandle(win32::WinApplication* app, Object kind)
{
    if (!s_handles)
        s_handles = new win32::MemMap(4 * kHandles);
    for (reg32 index = 0; index < kHandles; ++index)
    {
        Entry& entry = s_entries[index];
        if (entry.used)
            continue;
        entry = Entry{};
        entry.used = true;
        entry.kind = kind;
        entry.references = 1;
        app->getMemory<reg32>(handleOf(index)) = 0;  // no table of methods
        return handleOf(index);
    }
    return 0;
}

/* The entry a handle the game hands back stands for, of `kind`; none for
 * anything else. */
Entry* entryOf(reg32 handle, Object kind)
{
    if (!s_handles || handle < s_handles->getBlockStart() || (handle - s_handles->getBlockStart()) % 4 != 0)
        return nullptr;
    const reg32 index = (handle - s_handles->getBlockStart()) / 4;
    if (index >= kHandles || !s_entries[index].used || s_entries[index].kind != kind)
        return nullptr;
    return &s_entries[index];
}

/* The arguments as stdcall has them: the object, then the rest. */
struct Arguments
{
    win32::WinApplication* app;
    reg32 base;

    reg32 operator[](reg32 index) const
    {
        return app->getMemory<reg32>(base + 4 * index);
    }
    template <typename T>
    T* at(reg32 index) const
    {
        const reg32 address = (*this)[index];
        return address ? &app->getMemory<T>(address) : nullptr;
    }
};

/* The axes a dead zone or saturation header names, as a range of
 * Gamepad::kDirectInputAxes: one axis by its DIJOYSTATE offset -- the only way
 * the game names one (its table at 0x434ed4 lists the offsets 0 to 28) -- or
 * all of them for the device.  False for anything else. */
bool propertyAxes(const PropertyHeader* header, reg32& first, reg32& last)
{
    if (header->dwHow == kHowDevice && header->dwObj == 0)
    {
        first = 0;
        last = win32::Gamepad::kDirectInputAxes - 1;
        return true;
    }
    if (header->dwHow == kHowByOffset && header->dwObj % 4 == 0
        && header->dwObj / 4 < win32::Gamepad::kDirectInputAxes)
    {
        first = last = header->dwObj / 4;
        return true;
    }
    return false;
}

void release(win32::WinApplication* app, Entry& entry)
{
    if (entry.references == 0 || --entry.references != 0)
        return;
    if (entry.effect)
        win32::forcefeedback::destroy(app, entry.effect);
    entry = Entry{};
}

/* ---- DirectInput: a device made, the joysticks told of. ---- */

HRESULT createDevice(win32::WinApplication* app, const Arguments& a)
{
    const GUID* guid = a.at<GUID>(1);
    reg32* out = a.at<reg32>(2);
    if (!guid || !out)
        return kInvalidParameter;
    win32::Input* input;
    if (*guid == kSysKeyboard)
        input = new win32::Keyboard;
    else if (*guid == kSysMouse)
        input = new win32::Mouse;
    else
        input = new win32::Gamepad(guid->Data3);
    app->allocateResource(input);
    const reg32 handle = newHandle(app, Object::Device);
    if (handle != 0)
        entryOf(handle, Object::Device)->input = input;
    *out = handle;
    return kOk;
}

/* The joysticks, each to the game's callback (stdcall: the instance, the
 * game's word), while it says go on: the slots, "NFS Gamepad 1" and 2 --
 * numbered, because the game shows this list to the player (the Dead Zone
 * screen's selector), slot 1 player one -- the touch overlay and the first
 * pad together -- and slot 2 the second pad. */
HRESULT enumDevices(win32::WinApplication* app, x86::CPU& cpu, const Arguments& a)
{
    const DWORD type = a[1];
    const reg32 callback = a[2], word = a[3];
    if (type != 4)
        return kOk;
    win32::MemMap memory(1024);
    DeviceInstance* device = &app->getMemory<DeviceInstance>(memory.getBlockStart());
    device->dwSize = sizeof(DeviceInstance);
    for (x86::reg16 index = 0; index < win32::Gamepad::getCount(); ++index)
    {
        device->dwDevType = type;
        device->guidInstance = kJoystick;
        device->guidInstance.Data3 = index;
        device->guidProduct = kJoystick;
        device->guidProduct.Data3 = 0x1f1f;
        SDL_snprintf(device->tszInstanceName, sizeof(device->tszInstanceName), "NFS Gamepad %u", unsigned(index) + 1);
        SDL_snprintf(device->tszProductName, sizeof(device->tszProductName), "NFS Gamepad %u", unsigned(index) + 1);
        device->guidFFDriver = kForceDriver;
        device->wUsagePage = 0;
        device->wUsage = 0;
        x86::Args registers;
        const std::optional<reg32> answer =
            guest::pointerCall(app, cpu, callback, registers, {callback, memory.getBlockStart(), word});
        if (!answer || !*answer)
            break;
    }
    return kOk;
}

/* ---- A device. ---- */

HRESULT capabilities(Entry& entry, const Arguments& a)
{
    Capabilities* caps = a.at<Capabilities>(1);
    if (!caps)
        return kInvalidParameter;
    std::memset(caps, 0, sizeof(Capabilities));
    caps->dwSize = sizeof(Capabilities);
    caps->dwFlags = kAttached | kPolledDataFormat;
    win32::Gamepad* pad = dynamic_cast<win32::Gamepad*>(entry.input);
    if (pad && pad->hasForceFeedback())
        caps->dwFlags |= kForceFeedback;
    caps->dwDevType = 4;
    caps->dwButtons = entry.input->getButtonCount();
    caps->dwAxes = entry.input->getAxesCount();
    return kOk;
}

/* A joystick's axis's dead zone or saturation, to the DIPROPDWORD after the
 * header: the game reads each as it opens a joystick (sub_435ae0), to start
 * its Dead Zone screen from. */
HRESULT getProperty(Entry& entry, const Arguments& a)
{
    const reg32 property = a[1];
    PropertyHeader* header = a.at<PropertyHeader>(2);
    if (property != kDeadZone && property != kSaturation)
        return kFalse;
    const win32::Gamepad* pad = dynamic_cast<win32::Gamepad*>(entry.input);
    reg32 first, last;
    if (!header || header->dwSize < 20)
        return kInvalidParameter;
    if (!pad || !propertyAxes(header, first, last))
        return kObjectNotFound;
    const win32::Gamepad::AxisResponse response = pad->axisResponse(first);
    const DWORD value = property == kDeadZone ? response.deadZone : response.saturation;
    std::memcpy(reinterpret_cast<char*>(header) + 16, &value, 4);
    return kOk;
}

/* A dead zone or saturation for one axis or the whole device -- the Dead Zone
 * screen (sub_442dd0) and the saved settings at startup (sub_472b60), applied
 * by the pad's own read (Gamepad::getState) as the driver did on Windows --
 * or the force feedback's strength, Stick Volume in the Force Feedback menu
 * (sub_435df0).  Each a DIPROPDWORD, 0 to 10000. */
HRESULT setProperty(Entry& entry, const Arguments& a)
{
    const reg32 property = a[1];
    const PropertyHeader* header = a.at<PropertyHeader>(2);
    if (win32::WinApplication::traceApi())
        SDL_Log("[API] SetProperty prop=%u", unsigned(property));
    if (property != kDeadZone && property != kSaturation && property != kForceGain)
        return kOk;
    if (!header || header->dwSize < 20)
        return kInvalidParameter;
    DWORD value;
    std::memcpy(&value, reinterpret_cast<const char*>(header) + 16, 4);
    if (value > 10000)
        return kInvalidParameter;
    win32::Gamepad* pad = dynamic_cast<win32::Gamepad*>(entry.input);
    if (property == kForceGain)
    {
        win32::forcefeedback::gain(pad, value);
        return kOk;
    }
    // The mouse and keyboard have no joystick axes to shape.
    if (!pad)
        return kOk;
    reg32 first, last;
    if (!propertyAxes(header, first, last))
        return kObjectNotFound;
    for (reg32 axis = first; axis <= last; ++axis)
    {
        win32::Gamepad::AxisResponse response = pad->axisResponse(axis);
        (property == kDeadZone ? response.deadZone : response.saturation) = value;
        pad->setAxisResponse(axis, response);
    }
    if (win32::WinApplication::traceApi())
        SDL_Log("[API] SetProperty %s axes %u-%u = %u", property == kDeadZone ? "deadzone" : "saturation",
                unsigned(first), unsigned(last), unsigned(value));
    return kOk;
}

/* The device's state now: a pad's two axes -- X steers, Y is both pedals --
 * the axes it does not have centred, every hat centred and no buttons (a
 * pad's buttons reach the game as keys, sdl-backend/gamepad.cpp); the mouse's
 * movement since the last read and its buttons; nothing for the keyboard. */
HRESULT deviceState(win32::WinApplication* app, x86::CPU& cpu, Entry& entry, const Arguments& a)
{
    const DWORD size = a[1];
    x86::reg8* data = a.at<x86::reg8>(2);
    if (!data)
        return kInvalidParameter;
    app->unlockContext(cpu);
    win32::Gamepad* pad = dynamic_cast<win32::Gamepad*>(entry.input);
    win32::Mouse* mouse = pad ? nullptr : dynamic_cast<win32::Mouse*>(entry.input);
    std::memset(data, 0, size);
    if (pad && size >= sizeof(JoystickState))
    {
        JoystickState* state = reinterpret_cast<JoystickState*>(data);
        const win32::GamepadState now = pad->getState();
        state->lX = 0x7fff + now.axes[win32::Gamepad::kAxisSteer];
        state->lY = 0x7fff + now.axes[win32::Gamepad::kAxisPedals];
        state->lZ = state->lRx = state->lRy = state->lRz = 0x7fff;
        for (int pov = 0; pov < 4; ++pov)
            state->rgdwPOV[pov] = DWORD(-1);
    }
    else if (mouse && size >= sizeof(MouseState))
    {
        MouseState* state = reinterpret_cast<MouseState*>(data);
        const win32::Mouse::MouseState now = win32::Mouse::getState();
        state->lX = now.dx;
        state->lY = now.dy;
        state->lZ = now.dz;
        for (int button = 0; button < 4; ++button)
            state->rgbButtons[button] = (now.buttons & (reg32(1) << button)) ? 0x80 : 0x00;
    }
    app->lockContext(cpu);
    return kOk;
}

/* The mouse's buffered events, oldest first, at most as many as the game has
 * room for: the 16-byte DIDEVICEOBJECTDATA of DirectX 3 the game passes (its
 * offset, its value; the time and sequence left 0, never read); how many to
 * the count.  No buffer: nothing taken. */
HRESULT deviceData(Entry& entry, const Arguments& a)
{
    const DWORD stride = a[1];
    x86::reg8* elements = a.at<x86::reg8>(2);
    DWORD* count = a.at<DWORD>(3);
    if (!count)
        return kInvalidParameter;
    if (!dynamic_cast<win32::Mouse*>(entry.input))
    {
        *count = 0;
        return kOk;
    }
    if (!elements || stride < 3 * sizeof(DWORD))
        return kOk;
    constexpr reg32 kMost = 32;
    win32::Mouse::BufferedEvent events[kMost];
    const reg32 taken = win32::Mouse::drainBuffer(events, *count < kMost ? *count : kMost);
    for (reg32 i = 0; i < taken; ++i)
    {
        x86::reg8* element = elements + i * stride;
        std::memset(element, 0, stride);
        DWORD* fields = reinterpret_cast<DWORD*>(element);
        fields[0] = events[i].offset;
        fields[1] = DWORD(events[i].value);
    }
    *count = taken;
    return kOk;
}

HRESULT objectInfo(const Arguments& a)
{
    ObjectInstance* object = a.at<ObjectInstance>(1);
    if (!object)
        return kInvalidParameter;
    std::memset(object, 0, sizeof(ObjectInstance));
    object->dwSize = sizeof(ObjectInstance);
    return kOk;
}

HRESULT createEffect(win32::WinApplication* app, Entry& entry, const Arguments& a)
{
    const GUID* guid = a.at<GUID>(1);
    const win32::forcefeedback::Parameters* parameters = a.at<win32::forcefeedback::Parameters>(2);
    reg32* out = a.at<reg32>(3);
    if (a[4] != 0)
        return kNoAggregation;
    if (!out)
        return win32::forcefeedback::kInvalid;
    *out = 0;
    if (!guid)
        return win32::forcefeedback::kUnsupported;
    win32::forcefeedback::Effect* effect;
    const HRESULT result = win32::forcefeedback::create(app, dynamic_cast<win32::Gamepad*>(entry.input), *guid,
                                                        parameters, &effect);
    if (result != kOk)
        return result;
    const reg32 handle = newHandle(app, Object::Effect);
    if (handle == 0)
    {
        win32::forcefeedback::destroy(app, effect);
        return win32::forcefeedback::kUnsupported;
    }
    entryOf(handle, Object::Effect)->effect = effect;
    *out = handle;
    return kOk;
}

HRESULT forceFeedbackCommand(Entry& entry, const Arguments& a)
{
    const DWORD flags = a[1];
    if (flags != 1 && flags != 2 && flags != 4 && flags != 8 && flags != 16 && flags != 32)
        return kInvalidParameter;
    win32::forcefeedback::command(dynamic_cast<win32::Gamepad*>(entry.input), flags);
    return kOk;
}

/* What a call takes off the stack besides the return address, in words, and
 * its answer. */
struct Answer
{
    reg32 words;
    HRESULT result;
};

Answer directInputCall(win32::WinApplication* app, x86::CPU& cpu, reg32 method, const Arguments& a)
{
    Entry* entry = entryOf(a[0], Object::DirectInput);
    switch (method)
    {
    case 0x8:  // Release
        if (entry)
            release(app, *entry);
        return {1, 0};
    case 0xc:  // CreateDevice(guid, &device, outer)
        return {4, entry ? createDevice(app, a) : kInvalidParameter};
    case 0x10:  // EnumDevices(type, callback, word, flags)
        return {5, entry ? enumDevices(app, cpu, a) : kInvalidParameter};
    }
    SDL_Log("controls: DirectInput method 0x%x not answered", unsigned(method));
    return {1, kInvalidParameter};
}

Answer deviceCall(win32::WinApplication* app, x86::CPU& cpu, reg32 method, const Arguments& a)
{
    Entry* entry = entryOf(a[0], Object::Device);
    if (!entry)
    {
        static const reg32 words[] = {3, 1, 1, 2, 3, 3, 1, 1, 3, 5, 2, 2, 3, 4};
        return {method / 4 < 14 ? words[method / 4] : 1, kInvalidParameter};
    }
    switch (method)
    {
    case 0x0:  // QueryInterface(iid, &object): IDirectInputDevice2, the same device
    {
        const GUID* iid = a.at<GUID>(1);
        reg32* out = a.at<reg32>(2);
        if (!out)
            return {3, kInvalidParameter};
        if (iid && *iid == kDevice2)
        {
            *out = a[0];
            ++entry->references;
            return {3, kOk};
        }
        *out = 0;
        return {3, kNoInterface};
    }
    case 0x8:  // Release
        release(app, *entry);
        return {1, 0};
    case 0xc:  // GetCapabilities(caps)
        return {2, capabilities(*entry, a)};
    case 0x14:  // GetProperty(property, header)
        return {3, getProperty(*entry, a)};
    case 0x18:  // SetProperty(property, header)
        return {3, setProperty(*entry, a)};
    case 0x1c:  // Acquire
        return {1, kOk};
    case 0x20:  // Unacquire: a pad's effects stop
        win32::forcefeedback::command(dynamic_cast<win32::Gamepad*>(entry->input), 2);
        return {1, kOk};
    case 0x24:  // GetDeviceState(size, data)
        return {3, deviceState(app, cpu, *entry, a)};
    case 0x28:  // GetDeviceData(stride, data, &count, flags)
        return {5, deviceData(*entry, a)};
    case 0x2c:  // SetDataFormat(format): the game's are the standard ones
    case 0x30:  // SetEventNotification(event)
        return {2, kOk};
    case 0x34:  // SetCooperativeLevel(window, flags)
        return {3, kOk};
    case 0x38:  // GetObjectInfo(instance, object, how)
        return {4, objectInfo(a)};
    case 0x48:  // CreateEffect(guid, parameters, &effect, outer)
        return {5, createEffect(app, *entry, a)};
    case 0x58:  // SendForceFeedbackCommand(flags)
        return {2, forceFeedbackCommand(*entry, a)};
    case 0x64:  // Poll
        app->unlockContext(cpu);
        win32::Gamepad::poll();
        app->lockContext(cpu);
        return {1, kOk};
    }
    SDL_Log("controls: device method 0x%x not answered", unsigned(method));
    return {1, kInvalidParameter};
}

Answer effectCall(win32::WinApplication* app, reg32 method, const Arguments& a)
{
    Entry* entry = entryOf(a[0], Object::Effect);
    win32::forcefeedback::Effect* effect = entry ? entry->effect : nullptr;
    switch (method)
    {
    case 0x8:  // Release
        if (entry)
            release(app, *entry);
        return {1, 0};
    case 0x18:  // SetParameters(parameters, flags)
    {
        const win32::forcefeedback::Parameters* parameters = a.at<win32::forcefeedback::Parameters>(1);
        if (!effect || !parameters)
            return {3, win32::forcefeedback::kInvalid};
        return {3, win32::forcefeedback::setParameters(app, effect, *parameters, a[2])};
    }
    case 0x1c:  // Start(iterations, flags)
        return {3, effect ? win32::forcefeedback::start(effect, a[1], a[2]) : win32::forcefeedback::kInvalid};
    case 0x20:  // Stop
        return {1, effect ? win32::forcefeedback::stop(effect) : win32::forcefeedback::kInvalid};
    case 0x28:  // Download
        return {1, effect ? win32::forcefeedback::download(effect) : win32::forcefeedback::kInvalid};
    case 0x2c:  // Unload
        return {1, effect ? win32::forcefeedback::unload(effect) : win32::forcefeedback::kInvalid};
    }
    SDL_Log("controls: effect method 0x%x not answered", unsigned(method));
    return {1, win32::forcefeedback::kInvalid};
}

}  // namespace

std::optional<reg32> directInputResult(win32::WinApplication* app, x86::CPU& cpu, Object object, reg32 method)
{
    cpu.esp -= 4;  // the return address's slot
    const Arguments a{app, cpu.esp + 4};
    Answer answer;
    if (method == kCreate)
    {
        // DirectInputCreateA(instance, version, &directInput, outer)
        reg32* out = a.at<reg32>(2);
        if (!out)
            answer = {4, kInvalidParameter};
        else
        {
            *out = newHandle(app, Object::DirectInput);
            answer = {4, *out != 0 ? kOk : kInvalidParameter};
        }
    }
    else if (object == Object::DirectInput)
        answer = directInputCall(app, cpu, method, a);
    else if (object == Object::Device)
        answer = deviceCall(app, cpu, method, a);
    else
        answer = effectCall(app, method, a);
    if (cpu.terminate)
        return std::nullopt;
    cpu.esp += 4 + 4 * answer.words;
    return answer.result;
}

}  // namespace controls
}  // namespace nfs3hp
