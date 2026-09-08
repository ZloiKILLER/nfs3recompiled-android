#pragma once
#include <winapi/dinput/idirectinputdevice.h>
namespace win32 { namespace dinput {
// Guest COM storage stays sizeof(IUnknown); host effect state lives in m_resource.
class IDirectInputEffect : public com::IUnknown {
public:
    static x86::reg32 init(WinApplication*, x86::reg32, x86::reg8*);
    static HRESULT create(WinApplication*, x86::CPU&, Gamepad*, const GUID*, const IDirectInputDevice::DIEFFECT*, Packed<IUnknown>*);
    static bool supported(const GUID&);
    static void update();
    static void command(Gamepad*, DWORD);
    static DWORD state(Gamepad*);
    static void gain(Gamepad*, DWORD);
    static void detach(Gamepad*);
    static HRESULT info(IDirectInputDevice::DIEffectInfo*, const GUID*);
    static HRESULT enumerate(WinApplication*, x86::CPU&, x86::reg32, x86::reg32, DWORD);
private:
    IDirectInputEffect(GenericResource*);
    ULONG release(WinApplication*, x86::CPU&);
    HRESULT initialize(WinApplication*, x86::CPU&, DWORD, DWORD, const GUID*);
    HRESULT effectGuid(WinApplication*, x86::CPU&, GUID*);
    HRESULT getParameters(WinApplication*, x86::CPU&, IDirectInputDevice::DIEFFECT*, DWORD);
    HRESULT setParameters(WinApplication*, x86::CPU&, const IDirectInputDevice::DIEFFECT*, DWORD);
    HRESULT start(WinApplication*, x86::CPU&, DWORD, DWORD);
    HRESULT stop(WinApplication*, x86::CPU&);
    HRESULT status(WinApplication*, x86::CPU&, DWORD*);
    HRESULT download(WinApplication*, x86::CPU&);
    HRESULT unload(WinApplication*, x86::CPU&);
    HRESULT escape(WinApplication*, x86::CPU&, void*);
    static x86::reg32 table;
};
}}
