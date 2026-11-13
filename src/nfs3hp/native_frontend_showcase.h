#ifndef NFS3HP_NATIVE_FRONTEND_SHOWCASE_H
#define NFS3HP_NATIVE_FRONTEND_SHOWCASE_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_showcase.cpp's typed functions (the car showcase: showcase, slides, history,
 * compare, compare2, interior), which other files call as C++ (guest::direct), and how the game's
 * code calls each by address (ByAddress, for the checks' stand-ins).  Each calls the game's other
 * functions by address on `cpu` and throws fe::Terminated when the game stopped in one, as
 * Guest::call does; its result is what the game's function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace showcase
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x439290: a car's file stem, in its record: 0 when it has no record.
reg32 carFileStem(App* app, CPU& cpu, reg32 car);
// The game's 0x453160: whether the car has an interior to show: none without sound.
reg32 hasInterior(App* app, CPU& cpu, reg32 car);
// The game's 0x453200: whether the car has photographs.
reg32 hasSlides(App* app, CPU& cpu, reg32 car);
// The game's 0x453290: whether the car has a history.
reg32 hasHistory(App* app, CPU& cpu, reg32 car);
// The game's 0x453320: showcase's backdrop again, its number changed.
reg32 loadArt(App* app, CPU& cpu, reg32 menu);
// The game's 0x453500: the car the screen before this one was about; -1 for one of the showcase's own.
reg32 resolveCar(App* app, CPU& cpu);
// The game's 0x4535e0: showcase as it opens (menu, the screen): the car, its buttons, backdrop,
// description.
reg32 showcaseEnter(App* app, CPU& cpu, reg32 menu);
// The game's 0x4537d0: showcase's every pass: opened anew for another car, the backdrop again.
reg32 showcaseFrame(App* app, CPU& cpu, reg32 menu);
// The game's 0x453830: showcase as it closes, the description stopped.
reg32 showcaseExit(App* app, CPU& cpu);
// The game's 0x453840: the button: the next photograph.
reg32 nextSlide(App* app, CPU& cpu);
// The game's 0x453860: the button: the photograph before, from the first the last there is.
reg32 previousSlide(App* app, CPU& cpu);
// The game's 0x453c00: slides as it opens: its buttons, the photograph on its bitmap.
reg32 slidesEnter(App* app, CPU& cpu, reg32 menu);
// The game's 0x453ef0: slides' every pass: make and model take turns.
reg32 slidesFrame(App* app, CPU& cpu, reg32 menu);
// The game's 0x454000: the history's page for this era, its picture and mask at the two addresses: the
// file, or 0.
reg32 historyPage(App* app, CPU& cpu, reg32 back, reg32 mask);
// The game's 0x454210: the page of this era or of the first after it that has one; 0 when none has.
reg32 historyPageFrom(App* app, CPU& cpu, reg32 back, reg32 mask);
// The game's 0x454260: history as it opens: from the first era, its description spoken.
reg32 historyEnter(App* app, CPU& cpu, reg32 menu);
// The game's 0x454310: history's every pass: opened anew for another car, the next era's page.
reg32 historyFrame(App* app, CPU& cpu, reg32 menu);
// The game's 0x4543e0: history as it closes, the description stopped.
reg32 historyExit(App* app, CPU& cpu);
// The game's 0x4543f0: a car's picture for compare, as the texture it replaces or a new one.
reg32 comparePicture(App* app, CPU& cpu, reg32 car, reg32 replaced);
// The game's 0x454500: compare as it opens: the cars and their pictures.
reg32 compareEnter(App* app, CPU& cpu);
// The game's 0x454690: compare's every pass: on to compare2 when asked, a picture again for a car
// changed.
reg32 compareFrame(App* app, CPU& cpu);
// The game's 0x454580: compare2 as it opens: the cars, the bar and the four arrows.
reg32 compare2Enter(App* app, CPU& cpu);
// The game's 0x454660: compare2's every pass: back to compare when asked.
reg32 compare2Frame(App* app, CPU& cpu);
// The game's 0x454cf0: interior's zoom slider moved: its value to the view.
reg32 zoomMoved(App* app, CPU& cpu, reg32 slider);
// The game's 0x454d10: interior as it opens: the slider's handler, the slider at 0.
reg32 interiorEnter(App* app, CPU& cpu, reg32 menu);

}  // namespace showcase
}  // namespace frontend

namespace guest
{

// The car in eax.
template <>
struct ByAddress<frontend::showcase::carFileStem>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car }));
    }
};

// The car in eax.
template <>
struct ByAddress<frontend::showcase::hasInterior>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car }));
    }
};

// The car in eax.
template <>
struct ByAddress<frontend::showcase::hasSlides>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car }));
    }
};

// The car in eax.
template <>
struct ByAddress<frontend::showcase::hasHistory>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car }));
    }
};

// The menu in eax.
template <>
struct ByAddress<frontend::showcase::loadArt>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 menu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ menu }));
    }
};

template <>
struct ByAddress<frontend::showcase::resolveCar>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The menu in eax.
template <>
struct ByAddress<frontend::showcase::showcaseEnter>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 menu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ menu }));
    }
};

// The menu in eax.
template <>
struct ByAddress<frontend::showcase::showcaseFrame>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 menu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ menu }));
    }
};

template <>
struct ByAddress<frontend::showcase::showcaseExit>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::showcase::nextSlide>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::showcase::previousSlide>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The menu in eax.
template <>
struct ByAddress<frontend::showcase::slidesEnter>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 menu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ menu }));
    }
};

// The menu in eax.
template <>
struct ByAddress<frontend::showcase::slidesFrame>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 menu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ menu }));
    }
};

// The back in eax, the mask in edx.
template <>
struct ByAddress<frontend::showcase::historyPage>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 back, x86::reg32 mask)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ back, mask }));
    }
};

// The back in eax, the mask in edx.
template <>
struct ByAddress<frontend::showcase::historyPageFrom>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 back, x86::reg32 mask)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ back, mask }));
    }
};

// The menu in eax.
template <>
struct ByAddress<frontend::showcase::historyEnter>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 menu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ menu }));
    }
};

// The menu in eax.
template <>
struct ByAddress<frontend::showcase::historyFrame>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 menu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ menu }));
    }
};

template <>
struct ByAddress<frontend::showcase::historyExit>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The car in eax, the replaced in edx.
template <>
struct ByAddress<frontend::showcase::comparePicture>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car, x86::reg32 replaced)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car, replaced }));
    }
};

template <>
struct ByAddress<frontend::showcase::compareEnter>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::showcase::compareFrame>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::showcase::compare2Enter>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::showcase::compare2Frame>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// The slider in eax.
template <>
struct ByAddress<frontend::showcase::zoomMoved>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 slider)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ slider }));
    }
};

// The menu in eax.
template <>
struct ByAddress<frontend::showcase::interiorEnter>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 menu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ menu }));
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
