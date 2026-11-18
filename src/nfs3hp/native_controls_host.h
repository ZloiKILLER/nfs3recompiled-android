#ifndef NFS3HP_NATIVE_CONTROLS_HOST_H
#define NFS3HP_NATIVE_CONTROLS_HOST_H

#include <lib/winapp.h>

#include <optional>

/* What the game's side of DirectInput (native_controls_input.cpp) asks of the
 * port: each of its calls of DirectInput's, by the object it is made on and
 * the method's offset in that object's table of methods, its arguments where
 * the game's code pushes them -- on the stack, the object first, as stdcall.
 * The port answers them itself (native_controls_host.cpp), with its own pads,
 * mouse and keyboard and its force feedback: there is no DirectInput behind
 * them, and no object of it in the game's memory, only a handle the game
 * keeps where it kept the object. */
namespace nfs3hp
{
namespace controls
{

enum class Object
{
    DirectInput,
    Device,
    Effect,
};

/* DirectInputCreateA (by its thunk, 0x533ee0): its four words. */
constexpr x86::reg32 kCreate = 0xffffffff;

/* The same, the result (the HRESULT) given back instead of put in eax; nothing when the game stopped. */
std::optional<x86::reg32> directInputResult(win32::WinApplication* app, x86::CPU& cpu, Object object,
                                            x86::reg32 method);

}  // namespace controls
}  // namespace nfs3hp

#endif
