#include <winapi/dinput/idirectinputdevice.h>
#include <winapi/dinput/idirectinputeffect.h>
#include <winapi/wrapper.h>
#include <lib/gamepad.h>
#include <lib/window.h>


namespace win32 { namespace dinput
{


x86::reg32 IDirectInputDevice::s_iDirectInputDeviceVtable = 0;
const GUID IDirectInputDevice::s_diPropRange =  { 0x6F1D2B61, 0xD5A0, 0x11CF, { 0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00 } };

enum DirectInutDeviceForceFeedBackCommand
{
    DISFFC_RESET            = 0x00000001,
    DISFFC_STOPALL          = 0x00000002,
    DISFFC_PAUSE            = 0x00000004,
    DISFFC_CONTINUE         = 0x00000008,
    DISFFC_SETACTUATORSON   = 0x00000010,
    DISFFC_SETACTUATORSOFF  = 0x00000020
};

enum DirectInutDeviceFlags
{
    DIDC_ATTACHED           = 0x00000001,
    DIDC_POLLEDDEVICE       = 0x00000002,
    DIDC_EMULATED           = 0x00000004,
    DIDC_POLLEDDATAFORMAT   = 0x00000008,
    DIDC_FORCEFEEDBACK      = 0x00000100,
    DIDC_FFATTACK           = 0x00000200,
    DIDC_FFFADE             = 0x00000400,
    DIDC_SATURATION         = 0x00000800,
    DIDC_POSNEGCOEFFICIENTS = 0x00001000,
    DIDC_POSNEGSATURATION   = 0x00002000,
    DIDC_DEADBAND           = 0x00004000,
    DIDC_STARTDELAY         = 0x00008000,
    DIDC_ALIAS              = 0x00010000,
    DIDC_PHANTOM            = 0x00020000,
    DIDC_HIDDEN             = 0x00040000
};

enum DirectInputDeviceHow
{
    DIPH_DEVICE     = 0,
    DIPH_BYOFFSET   = 1,
    DIPH_BYID       = 2,
    DIPH_BYUSAGE    = 3
};

enum DirectInputDeviceForceFeedbackState
{
    DIGFFS_EMPTY           = 0x00000001,
    DIGFFS_STOPPED         = 0x00000002,
    DIGFFS_PAUSED          = 0x00000004,
    DIGFFS_ACTUATORSON     = 0x00000010,
    DIGFFS_ACTUATORSOFF    = 0x00000020,
    DIGFFS_POWERON         = 0x00000040,
    DIGFFS_POWEROFF        = 0x00000080,
    DIGFFS_SAFETYSWITCHON  = 0x00000100,
    DIGFFS_SAFETYSWITCHOFF = 0x00000200,
    DIGFFS_USERFFSWITCHON  = 0x00000400,
    DIGFFS_USERFFSWITCHOFF = 0x00000800,
    DIGFFS_DEVICELOST      = 0x80000000
};

struct DIJOYSTATE
{
    LONG lX;
    LONG lY;
    LONG lZ;
    LONG lRx;
    LONG lRy;
    LONG lRz;
    LONG rglSlider[2];
    DWORD rgdwPOV[4];
    BYTE rgbButtons[32];
};

/* c_dfDIMouse layout: lX, lY, lZ relative motion, then one byte per button.
 * Real DirectInput has DIMOUSESTATE2 with 8 buttons; this port only ever
 * tracks 4 (see gamepad.h's DIMOFS_* / Mouse::getButtonCount()), which is
 * already every button a period-appropriate mouse driver reports. */
struct DIMOUSESTATE
{
    LONG lX;
    LONG lY;
    LONG lZ;
    BYTE rgbButtons[4];
};

struct IDirectInputDevice::DIPROPHEADER
{
    DWORD dwSize;
    DWORD dwHeaderSize;
    DWORD dwObj;
    DWORD dwHow;
};

struct IDirectInputDevice::DIPROPRANGE
{
    IDirectInputDevice::DIPROPHEADER diph;
    LONG lMin;
    LONG lMax;
};

struct IDirectInputDevice::DIPROPPOINTER
{
    IDirectInputDevice::DIPROPHEADER    diph;
    Packed<UINT>    uData;
};

//DEFINE_GUID(IID_IDirectInputDeviceA,	0x5944E680,0xC92E,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(IID_IDirectInputDevice2A,	0x5944E682,0xC92E,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);

IDirectInputDevice::IDirectInputDevice(Input* input)
    :   IUnknown(IID_IDirectInputDevice2A)
{
    m_vtable = s_iDirectInputDeviceVtable;
    m_resource = input;
}

HRESULT IDirectInputDevice::GetCapabilities(WinApplication* app, x86::CPU& cpu,
                                            LPDIDEVCAPS lpDIDevCaps)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_ASSERT(lpDIDevCaps->dwSize == sizeof(DIDEVCAPS));
    memset(lpDIDevCaps, 0, sizeof(DIDEVCAPS));
    lpDIDevCaps->dwSize = sizeof(DIDEVCAPS);
    lpDIDevCaps->dwFlags = DIDC_ATTACHED | DIDC_POLLEDDATAFORMAT /* | DIDC_FORCEFEEDBACK */;
    auto* pad=dynamic_cast<Gamepad*>(m_resource);
    if(pad&&pad->hasForceFeedback())lpDIDevCaps->dwFlags |= DIDC_FORCEFEEDBACK;
    lpDIDevCaps->dwDevType = 4;
    lpDIDevCaps->dwButtons = dynamic_cast<Input*>(m_resource)->getButtonCount();
    lpDIDevCaps->dwAxes = dynamic_cast<Input*>(m_resource)->getAxesCount();
    return 0;
}

HRESULT IDirectInputDevice::EnumObjects(WinApplication* app, x86::CPU& cpu,
                                        LPDIENUMDEVICEOBJECTSCALLBACK lpCallback, x86::reg32 pvRef, DWORD dwFlags)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(lpCallback);
    NFS2_USE(pvRef);
    NFS2_USE(dwFlags);
    NFS2_ASSERT(false);
    return 1;
}

HRESULT IDirectInputDevice::GetProperty(WinApplication* app, x86::CPU& cpu,
                                        Packed<const GUID> rguidProp, LPDIPROPHEADER pdiph)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(rguidProp);
    NFS2_USE(pdiph);
    //NFS2_ASSERT(false);
    return 1;
}

HRESULT IDirectInputDevice::SetProperty(WinApplication* app, x86::CPU& cpu,
                                        Packed<const GUID> rguidProp, LPCDIPROPHEADER pdiph)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    // Which properties the game actually sets is only answerable at runtime.
    if(WinApplication::traceApi())
        SDL_Log("[API] SetProperty prop=%u",unsigned(x86::reg32(rguidProp)));
    /* MAKEDIPROP() packs the property id into the GUID pointer itself, so the
     * "pointer" is the small integer dinput.h defines.  8 is DIPROP_FFGAIN, the
     * device's force-feedback strength.  This used to read 7, DIPROP_SATURATION
     * -- an axis calibration property -- so the game's own strength setting was
     * dropped on the floor while an unrelated number scaled every effect. */
    if(x86::reg32(rguidProp)==8) {
        if(!pdiph||pdiph->dwSize<20)return 0x80070057;
        DWORD value;std::memcpy(&value,reinterpret_cast<const char*>(pdiph)+16,4);
        if(value>10000)return 0x80070057;
        IDirectInputEffect::gain(dynamic_cast<Gamepad*>(m_resource),value);
    }
    return 0;
}

HRESULT IDirectInputDevice::Acquire(WinApplication* app, x86::CPU& cpu)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    return 0;
}

HRESULT IDirectInputDevice::Unacquire(WinApplication* app, x86::CPU& cpu)
{
    IDirectInputEffect::command(dynamic_cast<Gamepad*>(m_resource),2);
    NFS2_USE(app);
    NFS2_USE(cpu);
    return 0;
}

HRESULT IDirectInputDevice::GetDeviceState(WinApplication* app, x86::CPU& cpu,
                                           DWORD cbData, LPVOID lpvData)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(cbData);
    NFS2_USE(lpvData);
    app->unlockContext(cpu);
    Gamepad* gamepad = dynamic_cast<Gamepad*>(m_resource);
    Mouse* mouse = gamepad ? nullptr : dynamic_cast<Mouse*>(m_resource);
    memset(lpvData, 0, cbData);
#ifdef NFS_TRACE_MSG
    /* Neither branch below fills anything for a keyboard device, so the game
     * gets 256 zero bytes: "no key is down".  Whether this is polled at all on
     * the stalled screens separates "input never arrives" from "input arrives
     * and is ignored".  Rate limited -- a poll loop would flood. */
    if (!gamepad && !mouse)
    {
        static unsigned hits = 0;
        if (win32::msgTraceRateLimit(hits))
            NFS_MSG_TRACE("dinput GetDeviceState %s cb=%u -> all zero (hit %u)",
                          dynamic_cast<Keyboard*>(m_resource) ? "KEYBOARD" : "UNKNOWN",
                          unsigned(cbData), hits);
    }
#endif
    if (gamepad)
    {
        NFS2_ASSERT(cbData == sizeof(DIJOYSTATE));
        DIJOYSTATE* state = reinterpret_cast<DIJOYSTATE*>(lpvData);
        GamepadState gpState = gamepad->getState();
        gamepad->markInputRead();
        state->lX = 0x7fff + gpState.axes[0];
        state->lY = 0x7fff + gpState.axes[1];
        state->lZ = 0x7fff + gpState.axes[4];
        state->lRx = 0x7fff + gpState.axes[2];
        state->lRy = 0x7fff + gpState.axes[3];
        state->lRz = 0x7fff + gpState.axes[5];
        state->rgdwPOV[0] = -1;
        state->rgdwPOV[1] = -1;
        state->rgdwPOV[2] = -1;
        state->rgdwPOV[3] = -1;
        for (int button = 0; button < 16; ++button)
        {
            state->rgbButtons[button] = (gpState.buttons & (1ll<<button)) ? 0x80 : 0x00;
        }
    }
    else if (mouse)
    {
        NFS2_ASSERT(cbData == sizeof(DIMOUSESTATE));
        DIMOUSESTATE* state = reinterpret_cast<DIMOUSESTATE*>(lpvData);
        Mouse::MouseState mState = Mouse::getState();
        state->lX = mState.dx;
        state->lY = mState.dy;
        state->lZ = mState.dz;
        for (int button = 0; button < 4; ++button)
        {
            state->rgbButtons[button] = (mState.buttons & (x86::reg32(1) << button)) ? 0x80 : 0x00;
        }
    }
    app->lockContext(cpu);
    return 0;
}

HRESULT IDirectInputDevice::GetDeviceData(WinApplication* app, x86::CPU& cpu,
                                          DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(cbObjectData);
    NFS2_USE(dwFlags);
    NFS2_ASSERT(dynamic_cast<Mouse*>(m_resource));
    Mouse* mouse = dynamic_cast<Mouse*>(m_resource);
    if (!mouse)
    {
        *pdwInOut = 0;
        return 0;
    }
    /* NFS3 (1998) predates DirectX 8's DIDEVICEOBJECTDATA.uAppData field and
     * passes the older 16-byte DIDEVICEOBJECTDATA_DX3 layout (dwOfs, dwData,
     * dwTimeStamp, dwSequence -- no uAppData), confirmed live: cbObjectData
     * came back 16 here, not sizeof(DIDEVICEOBJECTDATA) (20).  Both layouts
     * agree on the first three DWORDs, so write through raw bytes at
     * cbObjectData's stride instead of indexing rgdod[i], which would assume
     * the wrong element size and walk off the end of the caller's buffer. */
    NFS2_ASSERT(cbObjectData >= 3 * sizeof(DWORD));
    /* rgdod == nullptr with a real Win32 DirectInput device is a "just tell
     * me the count" peek and must not drain the queue; nothing here has ever
     * been observed to do that, but honour it anyway since it is free. */
    if (!rgdod)
    {
        return 0;
    }
    static const x86::reg32 s_maxEvents = 32;
    Mouse::BufferedEvent events[s_maxEvents];
    const x86::reg32 requested = (*pdwInOut < s_maxEvents) ? *pdwInOut : s_maxEvents;
    const x86::reg32 count = Mouse::drainBuffer(events, requested);
    x86::reg8* elements = reinterpret_cast<x86::reg8*>(rgdod);
    for (x86::reg32 i = 0; i < count; ++i)
    {
        x86::reg8* element = elements + i * cbObjectData;
        memset(element, 0, cbObjectData);
        DWORD* fields = reinterpret_cast<DWORD*>(element);
        fields[0] = events[i].offset;         // dwOfs
        fields[1] = DWORD(events[i].value);   // dwData
        // dwTimeStamp/dwSequence left zero -- nothing here reads them back.
    }
    *pdwInOut = count;
    return 0;
}

HRESULT IDirectInputDevice::SetDataFormat(WinApplication* app, x86::CPU& cpu,
                                          LPCDIDATAFORMAT lpdf)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(lpdf);
    return 0;
}

HRESULT IDirectInputDevice::SetEventNotification(WinApplication* app, x86::CPU& cpu,
                                                 HANDLE hEvent)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(hEvent);
    return 0;
}

HRESULT IDirectInputDevice::SetCooperativeLevel(WinApplication* app, x86::CPU& cpu,
                                                HWND hwnd, DWORD dwFlags)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(hwnd);
    NFS2_USE(dwFlags);
    return 0;
}

HRESULT IDirectInputDevice::GetObjectInfo(WinApplication* app, x86::CPU& cpu,
                                          LPDIDEVICEOBJECTINSTANCE pdidoi, DWORD dwObj, DWORD dwHow)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(dwObj);
    NFS2_USE(dwHow);
    NFS2_ASSERT(pdidoi->dwSize == sizeof(DIDEVICEOBJECTINSTANCE));
    memset(pdidoi, 0, pdidoi->dwSize);
    pdidoi->dwSize = sizeof(DIDEVICEOBJECTINSTANCE);
    /*switch (dwHow)
    {
    case DIPH_BYOFFSET:
        NFS2_ASSERT(false);
        break;
    case DIPH_DEVICE:
    case DIPH_BYID:
    case DIPH_BYUSAGE:
        NFS2_ASSERT(false);
        break;
    }
    NFS2_ASSERT(false);*/
    return 0;
}

HRESULT IDirectInputDevice::GetDeviceInfo(WinApplication* app, x86::CPU& cpu,
                                          LPDIDEVICEINSTANCE pdidi)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(pdidi);
    NFS2_ASSERT(false);
    return 1;
}

HRESULT IDirectInputDevice::RunControlPanel(WinApplication* app, x86::CPU& cpu,
                                            HWND hwndOwner, DWORD dwFlags)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(hwndOwner);
    NFS2_USE(dwFlags);
    NFS2_ASSERT(false);
    return 1;
}

HRESULT IDirectInputDevice::Initialize(WinApplication* app, x86::CPU& cpu,
                                       HINSTANCE hinst, DWORD dwVersion, REFGUID rguid)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(hinst);
    NFS2_USE(dwVersion);
    NFS2_USE(rguid);
    NFS2_ASSERT(false);
    return 1;
}

HRESULT IDirectInputDevice::CreateEffect(WinApplication* app, x86::CPU& cpu,
    REFGUID guid, LPCDIEFFECT effect, Packed<IUnknown>* out, LPUNKNOWN outer)
{
    if(outer)return 0x80040110;
    return IDirectInputEffect::create(app,cpu,dynamic_cast<Gamepad*>(m_resource),guid,effect,out);
}
HRESULT IDirectInputDevice::EnumEffects(WinApplication* app, x86::CPU& cpu,
    LPDIENUMEFFECTSCALLBACK callback, LPVOID user, DWORD filter)
{
    auto* pad=dynamic_cast<Gamepad*>(m_resource);
    if(!pad||!pad->hasForceFeedback())return 0;
    return IDirectInputEffect::enumerate(app,cpu,callback,app->guestAddress(user),filter);
}
HRESULT IDirectInputDevice::GetEffectInfo(WinApplication*, x86::CPU&, LPDIEffectInfo out, REFGUID guid)
{
    return IDirectInputEffect::info(out,guid);
}
HRESULT IDirectInputDevice::GetForceFeedbackState(WinApplication*, x86::CPU&, LPDWORD out)
{
    auto* pad=dynamic_cast<Gamepad*>(m_resource);if(!pad||!out)return 0x80070057;
    *out=IDirectInputEffect::state(pad);return 0;
}
HRESULT IDirectInputDevice::SendForceFeedbackCommand(WinApplication*, x86::CPU&, DWORD flags)
{
    if(flags!=1&&flags!=2&&flags!=4&&flags!=8&&flags!=16&&flags!=32)return 0x80070057;
    IDirectInputEffect::command(dynamic_cast<Gamepad*>(m_resource),flags);return 0;
}

HRESULT IDirectInputDevice::EnumCreatedEffectObjects(WinApplication* app, x86::CPU& cpu,
                                                     LPDIENUMCREATEDEFFECTOBJECTSCALLBACK lpCallback, LPVOID pvRef, DWORD fl)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(lpCallback);
    NFS2_USE(pvRef);
    NFS2_USE(fl);
    NFS2_ASSERT(false);
    return 1;
}

HRESULT IDirectInputDevice::Escape(WinApplication* app, x86::CPU& cpu,
                                   LPDIEFFESCAPE pesc)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(pesc);
    NFS2_ASSERT(false);
    return 1;
}

HRESULT IDirectInputDevice::Poll(WinApplication* app, x86::CPU& cpu)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    app->unlockContext(cpu);
    Gamepad::poll();
    app->lockContext(cpu);
    return 0;
}

HRESULT IDirectInputDevice::SendDeviceData(WinApplication* app, x86::CPU& cpu,
                                           DWORD cbObjectData, LPCDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD fl)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(cbObjectData);
    NFS2_USE(rgdod);
    NFS2_USE(pdwInOut);
    NFS2_USE(fl);
    NFS2_ASSERT(false);
    return 1;
}

x86::reg32 IDirectInputDevice::init(WinApplication *app, x86::reg32 memblockIndex, x86::reg8* memBlock)
{
    x86::reg32* vtable = reinterpret_cast<x86::reg32*>(memBlock);
    app->registerMethod(0x400048, {"GetCapabilities", &Wrapper<decltype(&IDirectInputDevice::GetCapabilities), &IDirectInputDevice::GetCapabilities>::stdcall});
    app->registerMethod(0x400049, {"EnumObjects", &Wrapper<decltype(&IDirectInputDevice::EnumObjects), &IDirectInputDevice::EnumObjects>::stdcall});
    app->registerMethod(0x40004a, {"GetProperty", &Wrapper<decltype(&IDirectInputDevice::GetProperty), &IDirectInputDevice::GetProperty>::stdcall});
    app->registerMethod(0x40004b, {"SetProperty", &Wrapper<decltype(&IDirectInputDevice::SetProperty), &IDirectInputDevice::SetProperty>::stdcall});
    app->registerMethod(0x40004c, {"Acquire", &Wrapper<decltype(&IDirectInputDevice::Acquire), &IDirectInputDevice::Acquire>::stdcall});
    app->registerMethod(0x40004d, {"Unacquire", &Wrapper<decltype(&IDirectInputDevice::Unacquire), &IDirectInputDevice::Unacquire>::stdcall});
    app->registerMethod(0x40004e, {"GetDeviceState", &Wrapper<decltype(&IDirectInputDevice::GetDeviceState), &IDirectInputDevice::GetDeviceState>::stdcall});
    app->registerMethod(0x40004f, {"GetDeviceData", &Wrapper<decltype(&IDirectInputDevice::GetDeviceData), &IDirectInputDevice::GetDeviceData>::stdcall});
    app->registerMethod(0x400050, {"SetDataFormat", &Wrapper<decltype(&IDirectInputDevice::SetDataFormat), &IDirectInputDevice::SetDataFormat>::stdcall});
    app->registerMethod(0x400051, {"SetEventNotification", &Wrapper<decltype(&IDirectInputDevice::SetEventNotification), &IDirectInputDevice::SetEventNotification>::stdcall});
    app->registerMethod(0x400052, {"SetCooperativeLevel", &Wrapper<decltype(&IDirectInputDevice::SetCooperativeLevel), &IDirectInputDevice::SetCooperativeLevel>::stdcall});
    app->registerMethod(0x400053, {"GetObjectInfo", &Wrapper<decltype(&IDirectInputDevice::GetObjectInfo), &IDirectInputDevice::GetObjectInfo>::stdcall});
    app->registerMethod(0x400054, {"GetDeviceInfo", &Wrapper<decltype(&IDirectInputDevice::GetDeviceInfo), &IDirectInputDevice::GetDeviceInfo>::stdcall});
    app->registerMethod(0x400055, {"RunControlPanel", &Wrapper<decltype(&IDirectInputDevice::RunControlPanel), &IDirectInputDevice::RunControlPanel>::stdcall});
    app->registerMethod(0x400056, {"Initialize", &Wrapper<decltype(&IDirectInputDevice::Initialize), &IDirectInputDevice::Initialize>::stdcall});
    app->registerMethod(0x400057, {"CreateEffect", &Wrapper<decltype(&IDirectInputDevice::CreateEffect), &IDirectInputDevice::CreateEffect>::stdcall});
    app->registerMethod(0x400058, {"EnumEffects", &Wrapper<decltype(&IDirectInputDevice::EnumEffects), &IDirectInputDevice::EnumEffects>::stdcall});
    app->registerMethod(0x400059, {"GetEffectInfo", &Wrapper<decltype(&IDirectInputDevice::GetEffectInfo), &IDirectInputDevice::GetEffectInfo>::stdcall});
    app->registerMethod(0x40005a, {"GetForceFeedbackState", &Wrapper<decltype(&IDirectInputDevice::GetForceFeedbackState), &IDirectInputDevice::GetForceFeedbackState>::stdcall});
    app->registerMethod(0x40005b, {"SendForceFeedbackCommand", &Wrapper<decltype(&IDirectInputDevice::SendForceFeedbackCommand), &IDirectInputDevice::SendForceFeedbackCommand>::stdcall});
    app->registerMethod(0x40005c, {"EnumCreatedEffectObjects", &Wrapper<decltype(&IDirectInputDevice::EnumCreatedEffectObjects), &IDirectInputDevice::EnumCreatedEffectObjects>::stdcall});
    app->registerMethod(0x40005d, {"Escape", &Wrapper<decltype(&IDirectInputDevice::Escape), &IDirectInputDevice::Escape>::stdcall});
    app->registerMethod(0x40005e, {"Poll", &Wrapper<decltype(&IDirectInputDevice::Poll), &IDirectInputDevice::Poll>::stdcall});
    app->registerMethod(0x40005f, {"SendDeviceData", &Wrapper<decltype(&IDirectInputDevice::SendDeviceData), &IDirectInputDevice::SendDeviceData>::stdcall});

    s_iDirectInputDeviceVtable = memblockIndex;
    vtable[0] = 0x400000;    // IUnknown_QueryInterface
    vtable[1] = 0x400001;    // IUnknown_AddRef
    vtable[2] = 0x400002;    // IUnknwon_Release
    vtable[3] = 0x400048;    // IDirectInputDevice_GetCapabilities
    vtable[4] = 0x400049;    // IDirectInputDevice_EnumObjects
    vtable[5] = 0x40004a;    // IDirectInputDevice_GetProperty
    vtable[6] = 0x40004b;    // IDirectInputDevice_SetProperty
    vtable[7] = 0x40004c;    // IDirectInputDevice_Acquire
    vtable[8] = 0x40004d;    // IDirectInputDevice_Unacquire
    vtable[9] = 0x40004e;    // IDirectInputDevice_GetDeviceState
    vtable[10] = 0x40004f;    // IDirectInputDevice_GetDeviceData
    vtable[11] = 0x400050;    // IDirectInputDevice_SetDataFormat
    vtable[12] = 0x400051;    // IDirectInputDevice_SetEventNotification
    vtable[13] = 0x400052;    // IDirectInputDevice_SetCooperativeLevel
    vtable[14] = 0x400053;    // IDirectInputDevice_GetObjectInfo
    vtable[15] = 0x400054;    // IDirectInputDevice_GetDeviceInfo
    vtable[16] = 0x400055;    // IDirectInputDevice_RunControlPanel
    vtable[17] = 0x400056;    // IDirectInputDevice_Initialize
    vtable[18] = 0x400057;    // IDirectInputDevice_CreateEffect
    vtable[19] = 0x400058;    // IDirectInputDevice_EnumEffects
    vtable[20] = 0x400059;    // IDirectInputDevice_GetEffectInfo
    vtable[21] = 0x40005a;    // IDirectInputDevice_GetForceFeedbackState
    vtable[22] = 0x40005b;    // IDirectInputDevice_SendForceFeedbackCommand
    vtable[23] = 0x40005c;    // IDirectInputDevice_EnumCreatedEffectObjects
    vtable[24] = 0x40005d;    // IDirectInputDevice_Escape
    vtable[25] = 0x40005e;    // IDirectInputDevice_Poll
    vtable[26] = 0x40005f;    // IDirectInputDevice_SendDeviceData
    return 27 * sizeof(x86::reg32);
}

}}
