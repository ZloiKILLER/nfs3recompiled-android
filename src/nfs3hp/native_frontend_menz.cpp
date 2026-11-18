#include "native_frontend_menz.h"
#include "native_watcom.h"
#include "native_game.h"
#include "native_ea_memory.h"
#include "native_ea_threads.h"
#include "native_ea_files.h"
#include "native_frontend_eaa.h"
#include "native_frontend_eab.h"
#include "native_frontend_netb.h"
#include "native_frontend_rendf.h"
#include "native_frontend_wata.h"
#include "native_frontend_watb.h"
#include "native_frontend_watc.h"
#include "native_frontend.h"
#include "native_windows.h"
#include "host_threads.h"
#include "host_video.h"
#include "host_window.h"
#include "port_snd_stream_calls.h"
#include <fpu.h>
#include <nfs3hp.h>
#include "host_system.h"
#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <optional>
#include <utility>
#include "native_guest.h"

namespace nfs3hp
{
namespace
{

/* A call through a pointer or to a function of the game's that has no typed core: Watcom's register
 * arguments (eax, edx, ebx, ecx in turn) and the stack words as `arguments` have them, through
 * guest::pointerCall; its eax, and Terminated when the game stopped in it. */
x86::reg32 callAt(fe::Guest& guest, x86::reg32 target, const fe::Arguments& arguments = fe::Arguments())
{
    x86::CPU& cpu = guest.cpu();
    x86::Args registers;
    x86::reg32* const into[4] = { &registers.eax, &registers.edx, &registers.ebx, &registers.ecx };
    for (std::size_t i = 0; i < arguments.registerCount; ++i)
        *into[i] = arguments.registers[i];
    const x86::reg32 esp = cpu.esp;
    for (std::size_t i = arguments.stackCount; i-- > 0;)
    {
        cpu.esp -= 4;
        guest.setDword(cpu.esp, arguments.stack[i]);
    }
    const std::optional<x86::reg32> result = guest::pointerCall(guest.app(), cpu, target, registers);
    if (!result)
        throw fe::Terminated();
    cpu.esp = esp;
    return *result;
}

}  // namespace
}  // namespace nfs3hp

/* What was left between the larger modules once they went native.  The rough map called
 * it menus, input and loop only because of its neighbours; it is EA's libraries' small
 * change and the C runtime's arithmetic:
 *
 *   DirectDraw      surfaces made, freed, relocked and measured, the display-mode
 *                   enumeration's callback (sub_4ed330 to sub_4edbc0);
 *   start-up        the one-instance check, the RAM memory class, the text library, the
 *                   Japanese keyboard, the video buffers (sub_4f91d0, sub_4f9270,
 *                   sub_517b30, sub_517ba0, sub_518fd0);
 *   the file device its thread started, its operations found, waited for and completed
 *                   (sub_4fb860 to sub_4fc780), its lock (sub_50ea20, sub_50ea40);
 *   video windows   made, updated, freed (sub_508c70 to sub_508e70);
 *   fonts           a .FNT made a font, a glyph found, a property set and read
 *                   (sub_5199b0 to sub_519f10, sub_5257c0, sub_5259d0);
 *   palettes        6 and 8 bits a channel to a 32-bit palette (sub_51de60, sub_51df60);
 *   the debug keys  bound by sub_517de0: the memory viewer, pause with its message box,
 *                   the screenshot with its banner, a display flag (sub_524930 to
 *                   sub_524de0, sub_517d70, sub_517da0);
 *   Watcom's FDIV   the start-up test for the Pentium's bug (sub_51f2a0), the doubles it
 *                   computes with (sub_52643c, sub_526642, sub_5267ff, sub_52699c), which
 *                   work the integer way on a machine without an x87, and the division
 *                   safe from the bug (sub_52700c, sub_526f87).
 *
 * All their state stays in guest memory.  What they call is called by address but the typed
 * cores: EA's (events, locks, memory given back, sleep, the system's tasks) and those of the
 * files with a header (_eaa and _eab as C++, guest::direct; _wata, _watb, _watc, _netb, _rendf:
 * fe::direct), the registers the game's code hands them by way of the calls before put in
 * their arguments -- not reservememadr from sub_508c70, whose ebx its failure hands on, nor the
 * runtime's doubles of sub_52c88a to sub_52c8a7 (edx:eax); Windows'
 * functions and DirectDraw's methods are the port's own, called as C++ (callImport, callCom);
 * what each function leaves in eax and in the
 * registers it does not save is kept.
 *
 * Plan step 5: each function a typed core in frontend::menz (native_frontend_menz.h), in a frame of
 * its own where the game's function has one (fe::Frame), and a thunk under the game's name that
 * reads the arguments from the registers and the stack for the calls by address.  A core of this
 * file calls another as C++ (guest::direct) but where its caller goes on with the registers the
 * call leaves: sub_51f2a0's doubles (edx:eax).  A core whose game's code reads an argument's
 * register again after a call puts the argument there first (sub_4ed730). */
namespace nfs3hp
{

namespace frontend
{
x86::reg32 ddrawRelockSurface(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 ddrawMakeSurface(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 ddrawFreeSurface(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 ddrawBackSurface(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 ddrawSurfaceSize(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 ddrawEnumModeCallback(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 ddrawNoteChange(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 singleInstanceCheck(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 memClassNoPurge(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 memClassCreateRam(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 fileDeviceStart(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 fileOpFind(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 fileOpStatus(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 fileOpComplete(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 fileOpWait(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 fileLockEnter(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 fileLockLeave(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 videoWindowMake(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 videoWindowUpdate(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 videoWindowFree(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 eaTextStart(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 eaTextEnd(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 keyboardIsPc98(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 debugKeyDisplayFlag(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 debugKeyScreenshot(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 debugKeysBind(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 eaSetLoadHook(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 videoBuffersStart(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 fontFindGlyph(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 fontGlyph(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 fontBitsPerPixel(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 fontCreate(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 fontSetProperty(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 fontGetProperty(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 paletteFrom6Bits(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 paletteFrom8Bits(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 debugBoxCopyScreen(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 debugBoxPutBack(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 debugBoxAsk(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 debugBanner(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 debugKeyPause(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 debugKeyViewerFirst(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 debugKeyViewerSecond(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 textBackgroundBox(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 longDoubleKind(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 fdivBugCheck(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 doubleDivide(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 doubleSubtract(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 doubleMultiply(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 doubleCompare(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 safeDivide(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 safeDivideCall(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 doubleProductFromX87(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
}

namespace
{

using fe::args;
using fe::Arguments;
using fe::Guest;
using fe::kRestoreEbx;
using fe::kRestoreEcx;
using fe::kRestoreEdi;
using fe::kRestoreEdx;
using fe::kRestoreEsi;
using fe::Site;
using x86::Float;
using x86::reg32;
using x86::sreg32;

constexpr std::uint8_t kBx = kRestoreEbx, kCx = kRestoreEcx, kDx = kRestoreEdx, kSi = kRestoreEsi, kDi = kRestoreEdi;
constexpr std::uint8_t kAllSaved = kBx | kCx | kDx | kSi | kDi;

// Windows, the import table's slots: the port's own functions, called as C++.
constexpr reg32 kEnterLock = 0x5344b8, kLeaveLock = 0x534588, kGetTickCount = 0x534568;
constexpr reg32 kCreateMutexA = 0x5344a0, kGetLastError = 0x534534, kFindWindowA = 0x534734;
constexpr reg32 kShowWindow = 0x534798, kSetForegroundWindow = 0x534788, kGetKeyboardType = 0x534748;
constexpr reg32 kErrorAlreadyExists = 0xb7;

// The game's own functions.
constexpr reg32 kError = 0x401010, kFillZero = 0x4e070c, kMemset = 0x4e0640;
constexpr reg32 kReport = 0x501050;
constexpr reg32 kCloseHandleSlot = 0x4e3fe0;

// The error's place, read by Error when it says it.
constexpr reg32 kErrorFile = 0x552190, kErrorFunction = 0x552194, kErrorLine = 0x552198;

// The DirectDraw report's channel and its lock.
constexpr reg32 kReportChannel = 0x564ed4, kDrawLock = 0x564480;
// DirectDraw's methods called here, by their offset in an object's table (host_video.h):
// IUnknown's Release, IDirectDraw2's CreateSurface, the rest IDirectDrawSurface's.
constexpr reg32 kRelease = 0x08, kCreateSurface = 0x18, kGetAttachedSurface = 0x30, kGetPixelFormat = 0x54;
constexpr reg32 kGetSurfaceDesc = 0x58, kSetColorKey = 0x74, kSetPalette = 0x7c, kUnlock = 0x80;
// A display's record.
constexpr reg32 kDisplayObject = 0x00, kDisplayLocked = 0x43c, kDisplayLockedBits = 0x440, kDisplayPrimary = 0x444, kDisplayKeepOnFail = 0x45f;
constexpr reg32 kMainDisplay = 0x9f38d0;
// A DDSURFACEDESC, 0x6c bytes.
constexpr reg32 kDescSize = 0x6c, kDescFlags = 0x04, kDescHeight = 0x08, kDescWidth = 0x0c, kDescPitch = 0x10, kDescBackBuffers = 0x14;
constexpr reg32 kDescBits = 0x24, kDescFormat = 0x48, kDescFormatBits = 0x54, kDescRedMask = 0x58, kDescGreenMask = 0x5c, kDescBlueMask = 0x60, kDescCaps = 0x68;

/* A function of Windows', its words handed: the port's own, called as C++ (native_windows.h).
 * Terminated when the game stopped in it, as Guest::call. */
template <reg32 Slot, auto Function, typename... Words>
reg32 callImport(Guest& guest, x86::CPU& cpu, Words... words)
{
    const std::optional<reg32> eax = guest::windows<Slot, Function>(guest.app(), cpu, words...);
    if (!eax)
        throw fe::Terminated();
    return *eax;
}

/* A method of a DirectDraw object, by its offset in the object's table, the object first on the
 * stack: the host's method called as C++ where the table has it there, else
 * through the table (host_video.h, guest::ddraw).  Terminated when the game stopped in it, as
 * Guest::call. */
template <reg32 Offset, auto Method, typename... Words>
reg32 callCom(Guest& guest, x86::CPU& cpu, reg32 object, Words... words)
{
    const std::optional<reg32> eax = guest::ddraw<Offset, Method>(guest.app(), cpu, object, words...);
    if (!eax)
        throw fe::Terminated();
    return *eax;
}

/* A function's stack word, counted from the first above the return address. */
reg32 stackWord(win32::WinApplication* app, const x86::CPU& cpu, reg32 index)
{
    return app->getMemory<reg32>(cpu.esp + 4 + 4 * index);
}

/* Error, its place set first. */
void failAt(Guest& guest, reg32 file, reg32 function, reg32 line, std::initializer_list<reg32> formatAndValues)
{
    guest.setDword(kErrorFile, file);
    guest.setDword(kErrorFunction, function);
    guest.setDword(kErrorLine, line);
    // Error's format and the words after it, as the game's code pushes them.
    const reg32 format = *formatAndValues.begin();
    guest::Words words({});
    for (const reg32* value = formatAndValues.begin() + 1; value != formatAndValues.end(); ++value)
        words.word[words.count++] = *value;
    fe::direct<kError, ea::error>(guest.app(), guest.cpu(), format, words);
}

/* A line of the DirectDraw report: its level, its format, what it says. */
void report(Guest& guest, std::initializer_list<reg32> levelFormatAndValues)
{
        const reg32 level = *levelFormatAndValues.begin(), format = *(levelFormatAndValues.begin() + 1);
    guest::Words words({});
    for (const reg32* value = levelFormatAndValues.begin() + 2; value != levelFormatAndValues.end(); ++value)
        words.word[words.count++] = *value;
    fe::direct<kReport, ea::reportedAt>(guest.app(), guest.cpu(), kReportChannel, level, format, words);
}

}  // namespace

/* ---------------------------------------------------------------------------
 * DirectDraw
 * ------------------------------------------------------------------------- */

/* sub_4ed330: the display's surface given up and locked again, so that other threads get
 * the lock for a moment; 1 when it is locked where it was, 0 when not, which forgets the
 * surface and draws into the bitmap at 0x5643f0 instead.  How long it was held goes to
 * 0x564ee0, the longest to 0x564ee4. */
reg32 frontend::menz::ddrawRelockSurface(App* app, CPU& cpu, reg32 display)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x84);
    constexpr reg32 kHeld = 0x564ee0, kHeldLongest = 0x564ee4, kLockedAt = 0x564eec;
    const reg32 surface = g.dword(display + kDisplayLocked);
    if (surface == 0)
        return 1;
    callCom<kUnlock, &host::ddraw::surfaceUnlock>(g, cpu, surface, g.dword(display + kDisplayLockedBits));
    const reg32 held = callImport<kGetTickCount, &host::ticks>(g, cpu) - g.dword(kLockedAt);
    g.setDword(kHeld, held);
    if (sreg32(held) > sreg32(g.dword(kHeldLongest)))
        g.setDword(kHeldLongest, held);
    callImport<kLeaveLock, &host::leaveLock>(g, cpu, g.dword(kDrawLock));
    callImport<kEnterLock, &host::enterLock>(g, cpu, g.dword(kDrawLock));
    g.setDword(kLockedAt, callImport<kGetTickCount, &host::ticks>(g, cpu));
    if (guest::direct<0x4eb5e0, eaa::ddrawLockSurface>(app, cpu, display, surface, g.frame()) == 0)
    {
        const reg32 bits = g.dword(g.frame() + kDescBits);
        if (bits == g.dword(display + kDisplayLockedBits))
        {
            g.setDword(display + kDisplayLocked, surface);
            return 1;
        }
        callCom<kUnlock, &host::ddraw::surfaceUnlock>(g, cpu, surface, bits);
    }
    g.setDword(display + kDisplayLockedBits, 0);
    g.setDword(display + kDisplayLocked, 0);
    g.setDword(kHeld, 0);
    callImport<kLeaveLock, &host::leaveLock>(g, cpu, g.dword(kDrawLock));
    guest::direct<0x4eada0, eaa::bitmapDrawInto>(app, cpu, g.dword(0x5643f0));
    report(g, { 3, 0x54c0bc });
    return 0;
}

x86::reg32 frontend::ddrawRelockSurface(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_4ed330", 0, 0, nullptr, 0, kAllSaved };
    const reg32 display = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::ddrawRelockSurface(app, cpu, display); });
}

/* sub_4ed480: makesurface: a DirectDraw surface; eax the display, edx and ebx its size, ecx
 * and the stack: back buffers, the pixel format wanted (0 the primary's, above that not
 * written), the caps, a palette for it.  A surface that is not the primary and not in video
 * memory is made of the size asked.  It is locked once, to see that it can be: failing that,
 * unless the display says to keep it, it is released. */
reg32 frontend::menz::ddrawMakeSurface(App* app, CPU& cpu, reg32 display, reg32 width, reg32 height, reg32 pages,
                                       reg32 backBuffers, reg32 formatWanted, reg32 capsAsked, reg32 palette)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x88);
    constexpr reg32 kColourKey = 0x6c, kMade = 0x74;
    constexpr reg32 kFlip = 0x18, kCapsPrimary = 0x200, kCapsVideo = 0x400;
    const reg32 f = g.frame();
    const reg32 directDraw = g.dword(display + kDisplayObject);
    g.setDword(f + kMade, 0);
    fe::direct<kFillZero, ea::clearBytes>(g.app(), g.cpu(), f, kDescSize);
    g.setDword(f, kDescSize);
    g.setDword(f + kDescFlags, 1);
    reg32 caps = capsAsked;
    if (sreg32(backBuffers) > 0)
    {
        g.setDword(f + kDescBackBuffers, backBuffers);
        caps |= kFlip;
        g.setDword(f + kDescFlags, 0x21);
    }
    if ((caps & (kCapsPrimary | kCapsVideo)) == 0)
    {
        g.setByte(f + kDescFlags, g.byte(f + kDescFlags) | 6);
        g.setDword(f + kDescHeight, height);
        g.setDword(f + kDescWidth, width);
        const sreg32 format = sreg32(formatWanted);
        if (format > 0)
        {
            failAt(g, 0x54bb1c, 0x54c0d4, 0x458, { 0x54c0e0 });
        }
        else if (format == 0)
        {
            g.setByte(f + kDescFlags + 1, g.byte(f + kDescFlags + 1) | 0x10);
            g.setDword(f + kDescFormat, 0x20);
            callCom<kGetPixelFormat, &host::ddraw::surfaceGetPixelFormat>(g, cpu, g.dword(display + kDisplayPrimary), f + kDescFormat);
        }
    }
    report(g, { 0xb, 0x54c120, width, height, pages, backBuffers + 1 });
    g.setDword(f + kDescCaps, caps);
    const reg32 failure = callCom<kCreateSurface, &host::ddraw::drawCreateSurface>(g, cpu, directDraw, f, f + kMade, 0) & 0xffff;
    reg32 ebx = failure;
    if (failure != 0)
    {
        g.setDword(f + kMade, 0);
    }
    else
    {
        report(g, { 0xf, 0x54c14c });
        if (g.dword(f + kMade) != 0)
        {
            callImport<kEnterLock, &host::enterLock>(g, cpu, g.dword(kDrawLock));
            const reg32 lockFailure = guest::direct<0x4eb5e0, eaa::ddrawLockSurface>(app, cpu, 0u, g.dword(f + kMade), f);
            ebx = lockFailure;
            if (lockFailure != 0)
                report(g, { 0xb, 0x54c168, lockFailure });
            else
                callCom<kUnlock, &host::ddraw::surfaceUnlock>(g, cpu, g.dword(f + kMade), g.dword(f + kDescBits));
            callImport<kLeaveLock, &host::leaveLock>(g, cpu, g.dword(kDrawLock));
            if (lockFailure != 0 && g.byte(display + kDisplayKeepOnFail) == 0)
            {
                callCom<kRelease, &host::ddraw::release>(g, cpu, g.dword(f + kMade));
                g.setDword(f + kMade, 0);
            }
            else
            {
                g.setDword(f, kDescSize);
                callCom<kGetSurfaceDesc, &host::ddraw::surfaceGetSurfaceDesc>(g, cpu, g.dword(f + kMade), f);
                g.setDword(f + kDescFormat, 0x20);
                callCom<kGetPixelFormat, &host::ddraw::surfaceGetPixelFormat>(g, cpu, g.dword(f + kMade), f + kDescFormat);
                const reg32 key = g.dword(f + kDescFormatBits) > 8 ? 0 : 0xff;
                g.setDword(f + kColourKey + 4, key);
                g.setDword(f + kColourKey, key);
                if (key != 0 && palette != 0)
                    callCom<kSetPalette, &host::ddraw::surfaceSetPalette>(g, cpu, g.dword(f + kMade), palette);
                callCom<kSetColorKey, &host::ddraw::surfaceSetColorKey>(g, cpu, g.dword(f + kMade), 8, f + kColourKey);
            }
        }
    }
    if (g.dword(f + kMade) == 0)
        report(g, { 0xa, 0x54c194, ebx });
    return g.dword(f + kMade);
}

x86::reg32 frontend::ddrawMakeSurface(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_4ed480", 0x10, 0, nullptr, 0, kSi | kDi };
    const reg32 display = args.eax;
    const reg32 width = args.edx;
    const reg32 height = args.ebx;
    const reg32 pages = args.ecx;
    const reg32 backBuffers = stackWord(app, cpu, 0);
    const reg32 formatWanted = stackWord(app, cpu, 1);
    const reg32 capsAsked = stackWord(app, cpu, 2);
    const reg32 palette = stackWord(app, cpu, 3);
    return fe::runThunk(app, cpu, args, site, [&](Guest&) {
        return menz::ddrawMakeSurface(app, cpu, display, width, height, pages, backBuffers, formatWanted, capsAsked, palette);
    });
}

/* sub_4ed730: a surface released (edx), unlocked first if it is the one the display (eax,
 * or the main one) holds locked. */
reg32 frontend::menz::ddrawFreeSurface(App* app, CPU& cpu, reg32 display, reg32 surface)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 4);
    if (display == 0)
        display = kMainDisplay;
    if (surface == g.dword(display + kDisplayLocked))
        guest::direct<0x4ed2a0, eab::ddrawUnlockBitmap>(app, cpu, display);
    return callCom<kRelease, &host::ddraw::release>(g, cpu, surface);
}

x86::reg32 frontend::ddrawFreeSurface(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_4ed730", 0, 0, nullptr, 0, kCx };
    const reg32 display = args.eax, surface = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::ddrawFreeSurface(app, cpu, display, surface); });
}

/* sub_4ed770: the surface attached to one (eax) as its back buffer. */
reg32 frontend::menz::ddrawBackSurface(App* app, CPU& cpu, reg32 surface)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x10);
    constexpr reg32 kCapsBackBuffer = 4;
    const reg32 f = g.frame();
    g.setDword(f, 0);
    g.setDword(f + 4, kCapsBackBuffer);
    callCom<kGetAttachedSurface, &host::ddraw::surfaceGetAttachedSurface>(g, cpu, surface, f + 4, f);
    return g.dword(f);
}

x86::reg32 frontend::ddrawBackSurface(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_4ed770", 0, 0, nullptr, 0, kCx | kDx };
    const reg32 surface = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::ddrawBackSurface(app, cpu, surface); });
}

/* sub_4ed820: a surface's (eax) width, height, bits a pixel, pitch and video memory's
 * address, each to where its pointer says if it has one; all 0 when it will not tell. */
reg32 frontend::menz::ddrawSurfaceSize(App* app, CPU& cpu, reg32 surface, reg32 widthOut, reg32 heightOut,
                                       reg32 bitsOut, reg32 pitchOut, reg32 memoryOut)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x78);
    const reg32 f = g.frame();
    g.setDword(f, kDescSize);
    reg32 result = callCom<kGetSurfaceDesc, &host::ddraw::surfaceGetSurfaceDesc>(g, cpu, surface, f);
    if ((result & 0xffff) != 0)
    {
        if (widthOut != 0)
            g.setDword(widthOut, 0);
        if (heightOut != 0)
            g.setDword(heightOut, 0);
        if (bitsOut != 0)
            g.setDword(bitsOut, 0);
        if (pitchOut != 0)
            g.setDword(pitchOut, 0);
        if (memoryOut != 0)
            g.setDword(memoryOut, 0);
        return result;
    }
    if (widthOut != 0)
        g.setDword(widthOut, result = g.dword(f + kDescWidth));
    if (heightOut != 0)
        g.setDword(heightOut, result = g.dword(f + kDescHeight));
    if (pitchOut != 0)
        g.setDword(pitchOut, result = g.dword(f + kDescPitch));
    if (memoryOut != 0)
        g.setDword(memoryOut, result = g.dword(f + kDescBits));
    if (bitsOut != 0)
    {
        if (g.dword(f + kDescFormatBits) == 0x10)
        {
            reg32 bits = guest::direct<0x4eb880, eaa::countBits>(app, cpu, g.dword(f + kDescRedMask));
            bits += guest::direct<0x4eb880, eaa::countBits>(app, cpu, g.dword(f + kDescGreenMask));
            result = guest::direct<0x4eb880, eaa::countBits>(app, cpu, g.dword(f + kDescBlueMask));
            g.setDword(bitsOut, bits + result);
        }
        else
        {
            result = g.byte(f + kDescFormatBits);
            g.setDword(bitsOut, result);
        }
    }
    return result;
}

x86::reg32 frontend::ddrawSurfaceSize(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_4ed820", 8, 0, nullptr, 0, kSi | kDi };
    const reg32 surface = args.eax;
    const reg32 widthOut = args.edx;
    const reg32 heightOut = args.ebx;
    const reg32 bitsOut = args.ecx;
    const reg32 pitchOut = stackWord(app, cpu, 0);
    const reg32 memoryOut = stackWord(app, cpu, 1);
    return fe::runThunk(app, cpu, args, site, [&](Guest&) {
        return menz::ddrawSurfaceSize(app, cpu, surface, widthOut, heightOut, bitsOut, pitchOut, memoryOut);
    });
}

/* sub_4edbc0: EnumDisplayModes' callback: which depths the card has (0x9f09a4), the mode
 * noted in the list of them (the second stack argument) while it has room, and reported;
 * 0 ends the enumeration, when the card has said more modes than it first counted. */
reg32 frontend::menz::ddrawEnumModeCallback(App* app, CPU& cpu, reg32 desc, reg32 list)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x18);
    constexpr reg32 kModesSeen = 0x9f09a0, kDepths = 0x9f09a4;
    constexpr reg32 kListCheck = 0x10, kListRoom = 0x08;
    reg32 bits;
    if (g.dword(desc + kDescFormatBits) == 0x10)
    {
        bits = guest::direct<0x4eb880, eaa::countBits>(app, cpu, g.dword(desc + kDescRedMask));
        bits += guest::direct<0x4eb880, eaa::countBits>(app, cpu, g.dword(desc + kDescGreenMask));
        bits += guest::direct<0x4eb880, eaa::countBits>(app, cpu, g.dword(desc + kDescBlueMask));
    }
    else
    {
        bits = g.byte(desc + kDescFormatBits);
    }
    std::uint8_t depth = 0;
    switch (bits)
    {
    case 8: depth = 1; break;
    case 15: depth = 2; break;
    case 16: depth = 4; break;
    case 24: depth = 8; break;
    case 32: depth = 0x10; break;
    default: break;
    }
    if (depth != 0)
        g.setByte(kDepths, g.byte(kDepths) | depth);
    if (list == 0)
        return 1;
    if (g.dword(list + kListCheck) != 0)
    {
        if (sreg32(g.dword(kModesSeen)) >= sreg32(g.dword(list + kListRoom)))
        {
            report(g, { 1, 0x54c1ec });
            return 0;
        }
        guest::direct<0x4eb8a0, eaa::ddrawNoteMode>(app, cpu, list, g.dword(kModesSeen), desc, 0u);
        const reg32 fourcc = g.dword(desc + 0x50);
        const reg32 fullscreen = (g.byte(desc + 0x68) & 0x10) ? 0x46 : 0x44;
        const reg32 modeX = (g.byte(desc + 0x6a) & 0x20) ? 0x58 : 0x4e;
        const reg32 backBuffers = g.dword(desc + kDescBackBuffers) + 1;
        const reg32 pitch = g.dword(desc + kDescPitch);
        const reg32 formatBits = g.dword(desc + kDescFormatBits);
        const reg32 height = g.dword(desc + kDescHeight);
        const reg32 width = g.dword(desc + kDescWidth);
        report(g, { 8, 0x54c1b4, 0x20, width, height, formatBits, bits, pitch, backBuffers, modeX, fullscreen, fourcc });
    }
    g.setDword(kModesSeen, g.dword(kModesSeen) + 1);
    return 1;
}

x86::reg32 frontend::ddrawEnumModeCallback(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_4edbc0", 8, 0, nullptr, 0, kBx | kSi | kDi };
    const reg32 desc = stackWord(app, cpu, 0), list = stackWord(app, cpu, 1);
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::ddrawEnumModeCallback(app, cpu, desc, list); });
}

/* sub_4f9060: one more change to what the display shows, counted at 0x56717c. */
reg32 frontend::menz::ddrawNoteChange(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    g.setDword(0x56717c, g.dword(0x56717c) + 1);
    return 0;
}

x86::reg32 frontend::ddrawNoteChange(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_4f9060", 0, 0, nullptr, 0, 0 };
    // eax as it came: the code only counts.
    const x86::reg32 eax = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { menz::ddrawNoteChange(app, cpu); return eax; });
}

/* ---------------------------------------------------------------------------
 * Start-up
 * ------------------------------------------------------------------------- */

/* sub_4f91d0: the mutex by the window class's name (eax, or the name kept, or the program's
 * first argument).  When it existed already the other instance's window is brought up and
 * this one leaves, or says 1 when 0x567188 is clear. */
reg32 frontend::menz::singleInstanceCheck(App* app, CPU& cpu, reg32 name)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0xc);
    constexpr reg32 kClassName = 0x5612a4, kMutex = 0x9f3d40, kProgramName = 0x9f38c8, kMustLeave = 0x567188;
    if (g.dword(kClassName) == 0)
        g.setDword(kClassName, name != 0 ? name : g.dword(g.dword(kProgramName)));
    if (name == 0)
        name = g.dword(kClassName);
    g.setDword(kMutex, callImport<kCreateMutexA, &host::createMutex>(g, cpu, 0, 1, name));
    if (callImport<kGetLastError, &host::lastError>(g, cpu) != kErrorAlreadyExists)
        return 0;
    const reg32 window = callImport<kFindWindowA, &host::findWindow>(g, cpu, 0, g.dword(kClassName));
    if (window != 0)
    {
        callImport<kShowWindow, &host::showWindow>(g, cpu, window, 9);
        callImport<kSetForegroundWindow, &host::setForegroundWindow>(g, cpu, window);
    }
    if (g.dword(kMustLeave) == 0)
        return 1;
    guest::direct<0x4f2a7d, eab::exitGameNow>(app, cpu);
    return 0;
}

x86::reg32 frontend::singleInstanceCheck(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_4f91d0", 0, 0, nullptr, 0, kBx | kCx | kDx };
    const reg32 name = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::singleInstanceCheck(app, cpu, name); });
}

/* sub_4f9260: the RAM class's purge callback: nothing can be purged. */
reg32 frontend::menz::memClassNoPurge(App*, CPU&)
{
    return 0;
}

x86::reg32 frontend::memClassNoPurge(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_4f9260", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::memClassNoPurge(app, cpu); });
}

/* sub_4f9270: the memory class "RAM" (eax its base, edx its size). */
reg32 frontend::menz::memClassCreateRam(App* app, CPU& cpu, reg32 base, reg32 size)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 8);
    constexpr reg32 kCreateMemClass = 0x4e1230, kPools = 0x9ef5c0;
    const reg32 result = callAt(g, kCreateMemClass, args({ 0, 0x54d92c, base, size }, { 8, 0x20, 0x10, 1, 1, 1, 1, 0x4f9260 }));
    const reg32 pools = g.dword(kPools);
    g.setDword(kPools + 4, pools);
    return result;
}

x86::reg32 frontend::memClassCreateRam(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_4f9270", 0, 0, nullptr, 0, kBx | kCx };
    const reg32 base = args.eax, size = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::memClassCreateRam(app, cpu, base, size); });
}

/* sub_517b30: the text library's state cleared, once, and its end (sub_517b70) to run at exit. */
reg32 frontend::menz::eaTextStart(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 8);
    constexpr reg32 kStarted = 0x568f30, kState = 0x5671f4, kStateBytes = 0xb8;
    if (g.dword(kStarted) != 0)
        return 0;  // eax as it came in the game's code, which no caller reads
    fe::direct<kFillZero, ea::clearBytes>(g.app(), g.cpu(), kState, kStateBytes);
    g.setDword(kStarted, 1);
    return guest::direct<0x4f2a78, eab::atexitJump>(app, cpu, 0x517b70u);
}

x86::reg32 frontend::eaTextStart(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_517b30", 0, 0, nullptr, 0, kCx | kDx };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::eaTextStart(app, cpu); });
}

/* sub_517b70: the text library's end. */
reg32 frontend::menz::eaTextEnd(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 8);
    constexpr reg32 kStarted = 0x568f30, kState = 0x5671f4, kStateBytes = 0xb8;
    if (g.dword(kStarted) == 0)
        return 0;  // eax as it came in the game's code, which no caller reads
    fe::direct<0x50c1e0, watb::freeTextDraw>(app, cpu);
        const reg32 filled = fe::direct<kFillZero, ea::clearBytes>(g.app(), g.cpu(), kState, kStateBytes);
    g.setDword(kStarted, 0);
    return filled;
}

x86::reg32 frontend::eaTextEnd(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_517b70", 0, 0, nullptr, 0, kCx | kDx };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::eaTextEnd(app, cpu); });
}

/* sub_517ba0: 1 and the NEC PC-98's scan codes when the keyboard is one (Japanese, subtype
 * 0xd01 to 0xd04). */
reg32 frontend::menz::keyboardIsPc98(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0xc);
    constexpr reg32 kJapanese = 7, kScanTable = 0x561298, kPc98Scans = 0x568f34;
    const reg32 type = callImport<kGetKeyboardType, &host::keyboardType>(g, cpu, 0);
    const sreg32 subtype = sreg32(callImport<kGetKeyboardType, &host::keyboardType>(g, cpu, 1));
    if (type != kJapanese || subtype < 0xd01 || subtype > 0xd04)
        return 0;
    g.setDword(kScanTable, kPc98Scans);
    return 1;
}

x86::reg32 frontend::keyboardIsPc98(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_517ba0", 0, 0, nullptr, 0, kBx | kCx | kDx };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::keyboardIsPc98(app, cpu); });
}

/* sub_518040: the load's hook (see native_ea_support.cpp) set to sub_517f10. */
reg32 frontend::menz::eaSetLoadHook(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    g.setDword(0x567868, 0x517f10);
    return 0;  // eax as it came in the game's code, which no caller reads
}

x86::reg32 frontend::eaSetLoadHook(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_518040", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::eaSetLoadHook(app, cpu); });
}

/* sub_518fd0: the video buffers' table, once: eax of them, 400 by default, 32 bytes each,
 * and the list of their numbers. */
reg32 frontend::menz::videoBuffersStart(App* app, CPU& cpu, reg32 asked)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x10);
    constexpr reg32 kDriver = 0x56718c, kAllocate = 0x14;
    constexpr reg32 kCount = 0x569054, kRecords = 0x569058, kFreeList = 0x56905c, kUsed = 0x569060;
    reg32 result = asked;
    if (g.dword(kRecords) != 0)
        return result;
    const reg32 count = result != 0 ? result : 400;
    g.setDword(kCount, count);
    reg32 driver = g.dword(kDriver);
    g.setDword(kRecords, callAt(g, g.dword(driver + kAllocate), args({ count << 5, driver, count })));
    const reg32 total = g.dword(kCount);
    driver = g.dword(kDriver);
    const reg32 list = callAt(g, g.dword(driver + kAllocate), args({ total * 4, driver, total, 0 }));
    const reg32 lastCount = g.dword(kCount);
    g.setDword(kUsed, 0);  // ecx as the allocation (sub_518f80 through the driver's pointer, which keeps ecx) was given it
    result = 0;
    for (; sreg32(result) < sreg32(lastCount); ++result)
        g.setDword(list + result * 4, result);
    g.setDword(kFreeList, list);
    g.setDword(kCount, lastCount);
    return result;
}

x86::reg32 frontend::videoBuffersStart(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_518fd0", 0, 0, nullptr, 0, kBx | kCx | kDx | kSi };
    const reg32 count = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::videoBuffersStart(app, cpu, count); });
}

/* ---------------------------------------------------------------------------
 * The debug keys
 * ------------------------------------------------------------------------- */

/* sub_517d70: key 0x17: the display's flag at 0x564390 flipped and the mode set again. */
reg32 frontend::menz::debugKeyDisplayFlag(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 8);
    g.setByte(0x564390, g.byte(0x564390) ^ 1);
    return fe::direct<0x5089d0, rendf::setDisplayMode>(app, cpu, g.dword(0x564384), g.dword(0x564388), g.dword(0x56438c));
}

x86::reg32 frontend::debugKeyDisplayFlag(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_517d70", 0, 0, nullptr, 0, kBx | kDx };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::debugKeyDisplayFlag(app, cpu); });
}

/* sub_517da0: key 0x16: the screen saved as snap.tga, and a banner that says so. */
reg32 frontend::menz::debugKeyScreenshot(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x74);
    fe::direct<0x4f46d0, wata::windowSave>(app, cpu, g.frame());
    fe::direct<0x4ead20, wata::windowApply>(app, cpu);
    // The game's code hands sub_4eb160 a 4 in edx too, which it keeps: the banner's time.
    guest::direct<0x4eb160, eaa::bitmapSaveTga>(app, cpu, 0x550698u);
    guest::direct<0x524c30, debugBanner>(app, cpu, 0x5506a4u, 4u);
    return fe::direct<0x4f4700, wata::windowRestore>(app, cpu, g.frame());
}

x86::reg32 frontend::debugKeyScreenshot(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_517da0", 0, 0, nullptr, 0, kDx };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::debugKeyScreenshot(app, cpu); });
}

/* sub_517de0: the debug keys bound, Ctrl+Escape set to leave. */
reg32 frontend::menz::debugKeysBind(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 4);
    g.setDword(0x564810, 1);
    fe::direct<0x50c5d4, watb::registerKey>(app, cpu, 4u, 0x524dd0u);
    fe::direct<0x50c5d4, watb::registerKey>(app, cpu, 5u, 0x524de0u);
    fe::direct<0x50c5d4, watb::registerKey>(app, cpu, 0x10u, 0x524d90u);
    fe::direct<0x50c5d4, watb::registerKey>(app, cpu, 0x18u, 0x524bb0u);
    fe::direct<0x50c5d4, watb::registerKey>(app, cpu, 0x16u, 0x517da0u);
    return fe::direct<0x50c5d4, watb::registerKey>(app, cpu, 0x17u, 0x517d70u);
}

x86::reg32 frontend::debugKeysBind(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_517de0", 0, 0, nullptr, 0, kDx };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::debugKeysBind(app, cpu); });
}

/* sub_524930: what is on the screen copied into the bitmap (eax): edx, ebx where. */
reg32 frontend::menz::debugBoxCopyScreen(App* app, CPU& cpu, reg32 bitmap, reg32 x, reg32 y)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 4);
    fe::direct<0x4ead20, wata::windowApply>(app, cpu);
    return guest::direct<0x4f2cb0, eab::shapeMoveWithSpot>(app, cpu, g.dword(bitmap + 0x20), x, y);
}

x86::reg32 frontend::debugBoxCopyScreen(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_524930", 0, 0, nullptr, 0, kCx };
    const reg32 bitmap = args.eax, x = args.edx, y = args.ebx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::debugBoxCopyScreen(app, cpu, bitmap, x, y); });
}

/* sub_524950: the bitmap (eax) back on the screen: edx, ebx where. */
reg32 frontend::menz::debugBoxPutBack(App* app, CPU& cpu, reg32 bitmap, reg32 x, reg32 y)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 4);
    fe::direct<0x4ead20, wata::windowApply>(app, cpu);
    // sub_4f09c0 hands the registers as they are on to the blitter it chose: the shape, x, y.
    fe::direct<0x4f09c0, wata::blitDispatchAt>(app, cpu, g.dword(bitmap + 0x20), x, y);
    return guest::direct<0x4ead90, eaa::bitmapDrawToScreen>(app, cpu);
}

x86::reg32 frontend::debugBoxPutBack(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_524950", 0, 0, nullptr, 0, kCx };
    const reg32 bitmap = args.eax, x = args.edx, y = args.ebx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::debugBoxPutBack(app, cpu, bitmap, x, y); });
}

/* sub_524970: a box of text (eax) at edx, ebx, ecx wide and the first stack argument high,
 * what it covers kept and put back; waits for the mouse's buttons (the second argument's
 * mask) to come up and then for a key or a button.  The answer: 1 for the first button or Y. */
reg32 frontend::menz::debugBoxAsk(App* app, CPU& cpu, reg32 text, reg32 x, reg32 y, reg32 width, reg32 height,
                                  reg32 mask)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x150);
    constexpr reg32 kView = 0x56502c;
    constexpr reg32 kDriverBegin = 0x564460, kDriverEnd = 0x564464, kDriverBeginCopy = 0x56445c, kButtons = 0x564468;
    const reg32 f = g.frame();
    // sub_4feffc answers 0 to what the game's code hands it: 0xffffffff, then 0 and the height (which edx keeps for
    // the bitmap), at the end its first answer.  The registers the calls below take in the game's code as the calls
    // before left them (the height in ecx, the box's right and bottom in ebx and ecx, white's ink in eax) handed.
    fe::direct<0x4feffc, watb::returnZero>(app, cpu);
    fe::direct<0x4feffc, watb::returnZero>(app, cpu);
    const reg32 bitmap = guest::direct<0x4ef190, eab::windowBitmapCreateDefault>(app, cpu, width, height,
                                                                                g.dword(0x56438c), g.dword(0x5643f4));
    fe::direct<0x4f46d0, wata::windowSave>(app, cpu, f + 0xb8);
    fe::direct<0x4f4740, wata::textStateSave>(app, cpu, f);
    guest::direct<0x4ef520, eab::fontUse>(app, cpu, 0x5662e4u);
    const reg32 bottom = height + y, right = x + width;
    fe::direct<0x52b6c0, watc::setRectClip>(app, cpu, kView, x, y, right, bottom);
    callAt(g, g.dword(kDriverBegin));
    guest::direct<0x4eada0, eaa::bitmapDrawInto>(app, cpu, bitmap);  // x in edx too, not read
    guest::direct<0x524930, debugBoxCopyScreen>(app, cpu, bitmap, x, y);  // the height in ecx too, not read
    fe::direct<0x4ead20, wata::windowApply>(app, cpu);
    const reg32 black = guest::direct<0x4ef760, eab::colourPack>(app, cpu, 0xff000000u);
    fe::direct<0x4fd540, watb::drawFilledRect>(app, cpu, x, y, width, height, black);
    const reg32 red = guest::direct<0x4ef760, eab::colourPack>(app, cpu, 0xffaa0000u);
    fe::direct<0x52b720, watc::drawBox>(app, cpu, x + 4, y + 4, right - 5, bottom - 4, red);
    const reg32 white = guest::direct<0x4ef760, eab::colourPack>(app, cpu, 0xffffffffu);
    guest::direct<0x4ef830, eab::fontSetColours>(app, cpu, white, 0u);
    fe::direct<0x52b860, watc::textCentred>(app, cpu, text, y + 9);
    callAt(g, g.dword(kDriverEnd));
    fe::direct<0x52b8b0, watc::clipWholeRect>(app, cpu, kView);
    guest::direct<0x4ead90, eaa::bitmapDrawToScreen>(app, cpu);
    while ((callAt(g, g.dword(kButtons)) & mask) != 0)
    {
    }
    reg32 answer, key;
    for (;;)
    {
        key = guest::direct<0x4f00d0, eab::keyNextLow16>(app, cpu);
        answer = callAt(g, g.dword(kButtons)) & mask;
        if (key != 0 || answer != 0)
            break;
    }
    answer &= 1;
    if ((key | 0x20) == 0x79)
        answer = 1;
    fe::direct<0x52b6c0, watc::setRectClip>(app, cpu, kView, x, y, right, y + height);
    callAt(g, g.dword(kDriverBeginCopy));
    guest::direct<0x524950, debugBoxPutBack>(app, cpu, bitmap, x, y);
    callAt(g, g.dword(kDriverEnd));
    fe::direct<0x4f4760, wata::textStateRestore>(app, cpu, f);
    fe::direct<0x4f4700, wata::windowRestore>(app, cpu, f + 0xb8);
    guest::direct<0x4eff00, eab::windowBitmapFree>(app, cpu, bitmap);
    fe::direct<0x4feffc, watb::returnZero>(app, cpu);
    return answer;
}

x86::reg32 frontend::debugBoxAsk(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_524970", 8, 0, nullptr, 0, kSi | kDi };
    const reg32 text = args.eax;
    const reg32 x = args.edx;
    const reg32 y = args.ebx;
    const reg32 width = args.ecx;
    const reg32 height = stackWord(app, cpu, 0);
    const reg32 mask = stackWord(app, cpu, 1);
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::debugBoxAsk(app, cpu, text, x, y, width, height, mask); });
}

/* sub_524c30: a line of text (eax) along the foot of the screen for edx of time, what it
 * covers put back after. */
reg32 frontend::menz::debugBanner(App* app, CPU& cpu, reg32 text, reg32 time)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x140);
    constexpr reg32 kView = 0x56502c;
    constexpr reg32 kDriverBegin = 0x564460, kDriverEnd = 0x564464, kDriverBeginCopy = 0x56445c;
    constexpr reg32 kScreenWidth = 0x564384, kScreenHeight = 0x564388, kBannerHeight = 0xa;
    const reg32 f = g.frame();
    // As in sub_524970: sub_4feffc answers 0 (the banner's height handed it in edx, which keeps it for the bitmap);
    // what the calls take in registers the calls before left handed.
    fe::direct<0x4feffc, watb::returnZero>(app, cpu);
    fe::direct<0x4feffc, watb::returnZero>(app, cpu);
    const reg32 bitmap = guest::direct<0x4ef190, eab::windowBitmapCreateDefault>(
        app, cpu, g.dword(kScreenWidth), kBannerHeight, g.dword(0x56438c), g.dword(0x5643f4));
    fe::direct<0x4f46d0, wata::windowSave>(app, cpu, f + 0xb8);
    fe::direct<0x4f4740, wata::textStateSave>(app, cpu, f);
    guest::direct<0x4ef520, eab::fontUse>(app, cpu, 0x5662e4u);
    guest::direct<0x4eada0, eaa::bitmapDrawInto>(app, cpu, bitmap);
    const reg32 screenHeight = g.dword(kScreenHeight);
    fe::direct<0x52b6c0, watc::setRectClip>(app, cpu, kView, 0u, screenHeight - kBannerHeight, g.dword(kScreenWidth),
                                            screenHeight);
    callAt(g, g.dword(kDriverBegin));
    guest::direct<0x524930, debugBoxCopyScreen>(app, cpu, bitmap, 0, g.dword(kScreenHeight) - kBannerHeight);
    fe::direct<0x4ead20, wata::windowApply>(app, cpu);
    const reg32 black = guest::direct<0x4ef760, eab::colourPack>(app, cpu, 0xff000000u);
    fe::direct<0x4fd540, watb::drawFilledRect>(app, cpu, 0u, g.dword(kScreenHeight) - kBannerHeight,
                                               g.dword(kScreenWidth), kBannerHeight, black);
    const reg32 white = guest::direct<0x4ef760, eab::colourPack>(app, cpu, 0xffffffffu);
    guest::direct<0x4ef830, eab::fontSetColours>(app, cpu, white, 0u);
    fe::direct<0x52b860, watc::textCentred>(app, cpu, text, g.dword(kScreenHeight) - 9);
    callAt(g, g.dword(kDriverEnd));
    guest::direct<0x4ead90, eaa::bitmapDrawToScreen>(app, cpu);
    guest::direct<0x4ead90, eaa::bitmapDrawToScreen>(app, cpu);
    fe::direct<0x4f06e0, wata::timerWait>(app, cpu, time);
    fe::direct<0x4ead20, wata::windowApply>(app, cpu);
    callAt(g, g.dword(kDriverBeginCopy));
    guest::direct<0x524950, debugBoxPutBack>(app, cpu, bitmap, 0u, g.dword(kScreenHeight) - kBannerHeight);
    callAt(g, g.dword(kDriverEnd));
    fe::direct<0x4f4760, wata::textStateRestore>(app, cpu, f);
    fe::direct<0x4f4700, wata::windowRestore>(app, cpu, f + 0xb8);
    guest::direct<0x4eff00, eab::windowBitmapFree>(app, cpu, bitmap);
    return fe::direct<0x4feffc, watb::returnZero>(app, cpu);
}

x86::reg32 frontend::debugBanner(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_524c30", 0, 0, nullptr, 0, kBx | kCx | kSi | kDi };
    const reg32 text = args.eax, time = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::debugBanner(app, cpu, text, time); });
}

/* sub_524d90: key 0x10: pause: the box in the middle of the screen until a key. */
reg32 frontend::menz::debugKeyPause(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0xc);
    const reg32 y = reg32(sreg32(g.dword(0x564388)) / 2) - 8;
    const reg32 x = reg32(sreg32(g.dword(0x564384)) / 2) - 0x90;
    return guest::direct<0x524970, debugBoxAsk>(app, cpu, 0x551e80, x, y, 0x120, 0x18, 1);
}

x86::reg32 frontend::debugKeyPause(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_524d90", 0, 0, nullptr, 0, kBx | kCx | kDx };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::debugKeyPause(app, cpu); });
}

/* sub_524dd0, sub_524de0: keys 4 and 5: the memory viewer's two ways in. */
reg32 frontend::menz::debugKeyViewerFirst(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    return fe::direct<0x52b8d0, watc::viewerOnWindow>(app, cpu);
}

x86::reg32 frontend::debugKeyViewerFirst(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_524dd0", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::debugKeyViewerFirst(app, cpu); });
}

reg32 frontend::menz::debugKeyViewerSecond(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    return fe::direct<0x52bb20, watc::viewerOnMono>(app, cpu);
}

x86::reg32 frontend::debugKeyViewerSecond(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_524de0", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::debugKeyViewerSecond(app, cpu); });
}

/* sub_52e440: text (eax) with a filled box behind it, at the text state's position. */
reg32 frontend::menz::textBackgroundBox(App* app, CPU& cpu, reg32 text)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0xd8);
    constexpr reg32 kX = 0x5671f8, kY = 0x5671fc, kColourFore = 0x567220, kColourBack = 0x567224;
    const reg32 f = g.frame();
    fe::direct<0x4f4740, wata::textStateSave>(app, cpu, f);
    fe::direct<0x4f0860, wata::textSetAttribute>(app, cpu, 0u);
    guest::direct<0x4ef830, eab::fontSetColours>(app, cpu, g.dword(kColourFore), g.dword(kColourBack));
    // The width's slot in ecx in the game's code, set before sub_4ef830, which keeps it.
    fe::direct<0x4f0c90, wata::textMeasure>(app, cpu, text, f + 0xbc, f + 0xb8, f + 0xc4, f + 0xc0);
    const reg32 offsetY = g.dword(f + 0xb8), offsetX = g.dword(f + 0xbc);
    fe::direct<0x4fd540, watb::drawFilledRect>(app, cpu, g.dword(kX) + offsetX, g.dword(kY) + offsetY,
                                               g.dword(f + 0xc4), g.dword(f + 0xc0), g.dword(kColourBack));
    fe::direct<0x4f4760, wata::textStateRestore>(app, cpu, f);
    return guest::direct<0x4efb50, eab::textDrawAtPen>(app, cpu, text);
}

x86::reg32 frontend::textBackgroundBox(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_52e440", 0, 0, nullptr, 0, kBx | kCx | kDx | kSi };
    const reg32 text = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::textBackgroundBox(app, cpu, text); });
}

/* ---------------------------------------------------------------------------
 * The file device
 * ------------------------------------------------------------------------- */

namespace
{
constexpr reg32 kFileDevices = 0x9f4f60, kFileDeviceSize = 0x68, kFileDeviceStarting = 0x9f4f64, kFileListLock = 0x9f4f74;
constexpr reg32 kFileDeviceLock = 0x1c, kFileDeviceDone = 0x38, kFileCompareId = 0x4fb820;
constexpr reg32 kOpFlags = 0x0c;
}

/* sub_4fb860: FILE_startdevice: the device's (eax) thread, if it has none: its two lists,
 * its events and lock, and the wait until the thread says it runs. */
reg32 frontend::menz::fileDeviceStart(App* app, CPU& cpu, reg32 index)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x10);
    const reg32 offset = index * kFileDeviceSize;
    const auto device = [&] { return g.dword(kFileDevices) + offset; };
    if (g.dword(device()) != 0)
        return g.dword(kFileDevices);
    cpu.esp -= 8;  // the two registers it saves only here
    const reg32 token = guest::direct<0x50ea20, fileLockEnter>(app, cpu, kFileListLock);
    g.setDword(device() + 0x18, 0);
    fe::direct<0x50dcd0, netb::qinit>(app, cpu, device() + kFileDeviceLock, 0x4fb830u, 0u);
    fe::direct<0x50dd60, netb::qinitpeer>(app, cpu, device() + kFileDeviceDone, 0u, 0u, device() + kFileDeviceLock);
    const reg32 event = *fe::direct<0x4dfb00, ea::newAutoEvent>(app, cpu);
    g.setDword(device() + 0x54, event);
    const reg32 manualEvent = *fe::direct<0x4dfc10, ea::newManualEvent>(app, cpu);
    g.setDword(device() + 0x5c, manualEvent);
    const reg32 lock = *fe::direct<0x4eb2e0, ea::newLock>(app, cpu);
    const reg32 context = device() + 4;
    g.setDword(context + 0x5c, 1);
    g.setDword(context + 0x54, lock);
    g.setDword(context + 0x60, 0xff);
    g.setDword(kFileDeviceStarting, index);
    const reg32 thread = *fe::direct<0x4df7a0, ea::threadStarted>(app, cpu, 0x4fbda0u, 0u, 1u, context);
    if (thread == 0)
        failAt(g, 0x54dcac, 0x54dcb8, 0xb6, { 0x54dccc });
    else if (g.dword(device()) == 0)
        fe::direct<0x4dfc60, ea::eventWaited>(app, cpu, g.dword(device() + 0x5c));
    return guest::direct<0x50ea40, fileLockLeave>(app, cpu, kFileListLock, token);
}

x86::reg32 frontend::fileDeviceStart(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_4fb860", 0, 0, nullptr, 0, kAllSaved };
    const reg32 index = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::fileDeviceStart(app, cpu, index); });
}

/* sub_4fbd10: an operation by its id (edx) on a device (eax): the one under way (and -1 to
 * *ebx), one queued (and 1), or a finished one (and 0). */
reg32 frontend::menz::fileOpFind(App* app, CPU& cpu, reg32 device, reg32 id, reg32 where)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0xc);
    g.setDword(where, 0);
    const reg32 current = g.dword(device + 0x18);
    if (current != 0 && id == g.dword(current + 4))
    {
        g.setDword(where, 0xffffffff);
        return current;
    }
    const reg32 queued = fe::direct<0x50e3f0, netb::qfind>(app, cpu, device + kFileDeviceLock, kFileCompareId, id);
    if (queued != 0)
    {
        g.setDword(where, 1);
        return queued;
    }
    return fe::direct<0x50e3f0, netb::qfind>(app, cpu, device + kFileDeviceDone, kFileCompareId, id);
}

x86::reg32 frontend::fileOpFind(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_4fbd10", 0, 0, nullptr, 0, kCx | kSi | kDi };
    const reg32 device = args.eax, id = args.edx, where = args.ebx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::fileOpFind(app, cpu, device, id, where); });
}

/* sub_4fc340: an operation's state (eax its id): 0 still going, -1 it failed, -3 no such,
 * or its result's byte. */
reg32 frontend::menz::fileOpStatus(App* app, CPU& cpu, reg32 id)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x14);
    const reg32 device = g.dword(kFileDevices) + (id & 0x1f) * kFileDeviceSize;
    const reg32 token = g.dword(device) != 0 ? guest::direct<0x50ea20, fileLockEnter>(app, cpu, device + kFileDeviceLock) : 0;
    const reg32 op = guest::direct<0x4fbd10, fileOpFind>(app, cpu, device, id, g.frame());
    reg32 status;
    if (g.dword(g.frame()) != 0)
        status = 0;
    else if (op == 0)
        status = 0xfffffffd;
    else if (g.dword(op + kOpFlags) & 2)
        status = 0xffffffff;
    else
        status = reg32(sreg32(std::int8_t(g.byte(op + 0x10))));
    if (g.dword(device) != 0)
        guest::direct<0x50ea40, fileLockLeave>(app, cpu, device + kFileDeviceLock, token);
    return status;
}

x86::reg32 frontend::fileOpStatus(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_4fc340", 0, 0, nullptr, 0, kBx | kCx | kDx | kSi };
    const reg32 id = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::fileOpStatus(app, cpu, id); });
}

/* sub_4fc600: FILE_completeop: a finished operation (eax its id) taken off its device's list
 * and its record given back; what it answers by its kind, the handle closed or the memory
 * freed where the operation made them for the caller. */
reg32 frontend::menz::fileOpComplete(App* app, CPU& cpu, reg32 id)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x14);
    constexpr reg32 kOpKind = 0x08, kOpSucceeded = 0x10, kOpHandle = 0x18, kOpCount = 0x24, kOpPosition = 0x28, kOpBlock = 0x2c;
    constexpr reg32 kFreeBlocks = 0x9f4f90;
    const reg32 device = g.dword(kFileDevices) + (id & 0x1f) * kFileDeviceSize;
    reg32 ecx = 0;
    reg32 op = 0;
    if (device != 0)
        op = fe::direct<0x50e5a0, watb::queueFetch>(app, cpu, device + kFileDeviceDone, kFileCompareId, id);
    if (op == 0)
        failAt(g, 0x54dcac, 0x54deec, 0x3ad, { 0x54defc });
    switch (g.dword(op + kOpKind))
    {
    case 0:
        if ((g.dword(op + kOpFlags) & 2) != 0 && g.dword(op + kOpHandle) != 0)
                        fe::direct<kCloseHandleSlot, ea::slotClosed>(app, cpu, g.dword(op + kOpHandle));
        else
            ecx = g.dword(op + kOpHandle);
        break;
    case 1:
                ecx = *fe::direct<kCloseHandleSlot, ea::slotClosed>(app, cpu, g.dword(op + kOpHandle));
        break;
    case 2:
    case 3:
        ecx = g.dword(op + kOpPosition);
        break;
    case 4:
    case 6:
        ecx = g.dword(op + kOpCount);
        break;
    case 8:
        ecx = 0;
        break;
    case 9:
    {
        const reg32 block = g.dword(op + kOpBlock);
        if (block == 0)
            break;
        if ((g.dword(op + kOpFlags) & 2) == 0 && g.byte(op + kOpSucceeded) == 1)
        {
            fe::direct<0x50ded0, netb::qbottom>(app, cpu, kFreeBlocks, block);
            ecx = g.dword(block + 4);
        }
        else
        {
            if (g.dword(block + 0xc) != 0)
                fe::direct<kCloseHandleSlot, ea::slotClosed>(app, cpu, g.dword(block + 0xc));
            fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, block);
        }
        break;
    }
    case 10:
    {
        const reg32 block = g.dword(op + kOpBlock);
        if (block == 0)
            break;
        fe::direct<kCloseHandleSlot, ea::slotClosed>(app, cpu, g.dword(block + 0xc));
        ecx = 1;
        fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, block);
        break;
    }
    default:  // 5, 7 and past the table
        ecx = g.byte(op + kOpSucceeded) == 1 ? 1 : 0;
        break;
    }
    if (g.dword(op + kOpFlags) & 1)
        fe::direct<0x501180, ea::pagesFreed>(app, cpu, g.dword(op + kOpBlock));
    g.setDword(op + 4, 0);
    fe::direct<0x50ded0, netb::qbottom>(app, cpu, kFileListLock, op);
    return ecx;
}

x86::reg32 frontend::fileOpComplete(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_4fc600", 0, 0, nullptr, 0, kBx | kCx | kDx | kSi };
    const reg32 id = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::fileOpComplete(app, cpu, id); });
}

/* sub_4fc780: waits until an operation (eax its id) has finished, on the device's thread
 * running the system's tasks; its state, as sub_4fc340 gives it; -3 for no such. */
reg32 frontend::menz::fileOpWait(App* app, CPU& cpu, reg32 id)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x10);
    const reg32 offset = (id & 0x1f) * kFileDeviceSize;
    const reg32 device = g.dword(kFileDevices) + offset;
    if (id == 0 || g.dword(device) == 0)
        return 0xfffffffd;
    const reg32 found = g.frame();
    cpu.esp -= 0x10;  // the four registers it saves only here
    do
    {
        const reg32 token = g.dword(device) != 0 ? guest::direct<0x50ea20, fileLockEnter>(app, cpu, device + kFileDeviceLock) : 0;
        const reg32 op = guest::direct<0x4fbd10, fileOpFind>(app, cpu, device, id, found);
        g.setDword(found, g.dword(found) & (op != 0 ? 1 : 0));
        if (g.dword(device) != 0)
            guest::direct<0x50ea40, fileLockLeave>(app, cpu, device + kFileDeviceLock, token);
        if (g.dword(found) != 0)
        {
            if (*fe::direct<0x4df930, ea::isThreadNow>(app, cpu, 0))
            {
                fe::direct<0x4e7630, port::stream::runDueTasks>(app, cpu, cpu.esp - 4, 0);
                fe::direct<0x4df8e0, ea::sleepFor>(app, cpu, 1);
            }
            else
            {
                fe::direct<0x4dfc60, ea::eventWaited>(app, cpu, g.dword(g.dword(kFileDevices) + offset + 0x5c));
            }
        }
    } while (g.dword(found) != 0);
    return guest::direct<0x4fc340, fileOpStatus>(app, cpu, id);
}

x86::reg32 frontend::fileOpWait(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_4fc780", 0, 0, nullptr, 0, kAllSaved };
    const reg32 id = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::fileOpWait(app, cpu, id); });
}

/* sub_50ea20: the critical section of a lock (eax) entered; 0, the token sub_50ea40 is given. */
reg32 frontend::menz::fileLockEnter(App* app, CPU& cpu, reg32 lock)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 8);
    callImport<kEnterLock, &host::enterLock>(g, cpu, g.dword(lock + 0x18));
    return 0;
}

x86::reg32 frontend::fileLockEnter(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_50ea20", 0, 0, nullptr, 0, kCx | kDx };
    const reg32 lock = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::fileLockEnter(app, cpu, lock); });
}

/* sub_50ea40: and left. */
reg32 frontend::menz::fileLockLeave(App* app, CPU& cpu, reg32 lock, reg32 /*token, not read*/)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 4);
    return callImport<kLeaveLock, &host::leaveLock>(g, cpu, g.dword(lock + 0x18));
}

x86::reg32 frontend::fileLockLeave(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_50ea40", 0, 0, nullptr, 0, kCx };
    const reg32 lock = args.eax, token = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::fileLockLeave(app, cpu, lock, token); });
}

/* ---------------------------------------------------------------------------
 * Video windows
 * ------------------------------------------------------------------------- */

namespace
{
constexpr reg32 kWindowFile = 0x54f53c, kWindowOne = 0x56502c, kWindowTwo = 0x565064;
}

/* sub_508c70: makevideowindow: a window of eax by edx, in one of the two fixed records
 * (where -1 or -2 is the first stack argument) or a record of its own; 0 for a size past
 * 2400x1600 or no memory. */
reg32 frontend::menz::videoWindowMake(App* app, CPU& cpu, reg32 width, reg32 height, reg32 bits, reg32 passedOn,
                                      reg32 where, reg32 passedOn2, reg32 passedOn3)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x10);
    constexpr reg32 kFunction = 0x54f594, kMaxWidth = 0x960, kMaxHeight = 0x640, kRecordSize = 0x38;
    if (sreg32(width) < 0 || sreg32(width) > sreg32(kMaxWidth) || sreg32(height) < 0 || sreg32(height) > sreg32(kMaxHeight))
    {
        failAt(g, kWindowFile, kFunction, 0x32, { 0x54f5a4, width, height, kMaxWidth, kMaxHeight });
        return 0;
    }
    reg32 window;
    if (where == 0xffffffff)
    {
        window = kWindowOne;
    }
    else if (where == 0xfffffffe)
    {
        window = kWindowTwo;
    }
    else
    {
        g.setDword(kErrorFile, kWindowFile);
        g.setDword(kErrorFunction, kFunction);
        g.setDword(kErrorLine, 0x3d);
        // reservememadr: its third word the caller's ebx, `where`.
        window = *fe::direct<0x4e1620, ea::reserved>(app, cpu, 0x54f5f4u, kRecordSize, where);
    }
    if (window == 0)
    {
        g.setDword(kErrorFile, kWindowFile);
        g.setDword(kErrorFunction, kFunction);
        g.setDword(kErrorLine, 0x41);
        const reg32 largest = fe::direct<0x4e1580, ea::defaultLargest>(app, cpu);
        fe::direct<kError, ea::error>(g.app(), g.cpu(), 0x54f5fc, guest::Words{width, height, largest});
        return 0;
    }
    fe::direct<0x508b80, watb::initVideoWindow>(app, cpu, window, width, height, bits, passedOn, passedOn2, passedOn3);
    return window;
}

x86::reg32 frontend::videoWindowMake(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_508c70", 0xc, 0, nullptr, 0, kSi | kDi };
    const reg32 width = args.eax;
    const reg32 height = args.edx;
    const reg32 bits = args.ebx;
    const reg32 passedOn = args.ecx;
    const reg32 where = stackWord(app, cpu, 0);
    const reg32 passedOn2 = stackWord(app, cpu, 1);
    const reg32 passedOn3 = stackWord(app, cpu, 2);
    return fe::runThunk(app, cpu, args, site, [&](Guest&) {
        return menz::videoWindowMake(app, cpu, width, height, bits, passedOn, where, passedOn2, passedOn3);
    });
}

/* sub_508da0: updatevideowindow: the window (eax, or the screen's) given a new row table
 * when its row size (edx) changed, ebx its bits, and its clip kept inside it. */
reg32 frontend::menz::videoWindowUpdate(App* app, CPU& cpu, reg32 window, reg32 rowBytes, reg32 bits)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x10);
    constexpr reg32 kWidth = 0x04, kHeight = 0x08, kClipLeft = 0x0c, kClipTop = 0x10, kClipRight = 0x14, kClipBottom = 0x18;
    constexpr reg32 kBits = 0x20, kRowBytes = 0x28, kRows = 0x2c;
    if (window == 0)
        window = 0x564ff4;
    g.setDword(window + kBits, bits);
    if (rowBytes != g.dword(window + kRowBytes))
    {
        const reg32 rows = g.dword(window + kRows);
        if (rows != 0)
            fe::direct<0x509b10, watb::removeRow>(app, cpu, rows);
        g.setDword(window + kRowBytes, rowBytes);
        const reg32 made = fe::direct<0x509a30, watb::makeRow>(app, cpu, 0u, rowBytes, 0u, g.dword(window + kHeight));
        g.setDword(window + kRows, made);
        if (made == 0)
            failAt(g, kWindowFile, 0x54f63c, 0x5e, { 0x54f650, g.dword(window + kWidth), g.dword(window + kHeight) });
    }
    if (sreg32(g.dword(window + kWidth)) < sreg32(g.dword(window + kClipLeft)))
        g.setDword(window + kClipLeft, g.dword(window + kWidth));
    if (sreg32(g.dword(window + kHeight)) < sreg32(g.dword(window + kClipTop)))
        g.setDword(window + kClipTop, g.dword(window + kHeight));
    const reg32 right = g.dword(window + kClipRight);
    if (sreg32(right) > sreg32(g.dword(window + kWidth)))
        g.setDword(window + kClipRight, g.dword(window + kWidth));
    else if (sreg32(right) < sreg32(g.dword(window + kClipLeft)))
        g.setDword(window + kClipRight, g.dword(window + kClipLeft));
    const reg32 bottom = g.dword(window + kClipBottom);
    if (sreg32(bottom) > sreg32(g.dword(window + kHeight)))
        g.setDword(window + kClipBottom, g.dword(window + kHeight));
    else if (sreg32(bottom) < sreg32(g.dword(window + kClipTop)))
        g.setDword(window + kClipBottom, g.dword(window + kClipTop));
    return bottom;
}

x86::reg32 frontend::videoWindowUpdate(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_508da0", 0, 0, nullptr, 0, kCx | kSi | kDi };
    const reg32 window = args.eax, rowBytes = args.edx, bits = args.ebx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::videoWindowUpdate(app, cpu, window, rowBytes, bits); });
}

/* sub_508e70: a window's (eax) row tables freed, and its record unless it is a fixed one. */
reg32 frontend::menz::videoWindowFree(App* app, CPU& cpu, reg32 window)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0xc);
    reg32 result = window;
    const reg32 rows = g.dword(window + 0x2c);
    if (rows != 0)
    {
        result = fe::direct<0x509b10, watb::removeRow>(app, cpu, rows);
        g.setDword(window + 0x2c, 0);
    }
    const reg32 otherRows = g.dword(window + 0x30);
    if (otherRows != 0)
    {
        result = fe::direct<0x509b10, watb::removeRow>(app, cpu, otherRows);
        g.setDword(window + 0x30, 0);
    }
    if (window == kWindowOne || window == kWindowTwo)
        return result;
    return *fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, window);
}

x86::reg32 frontend::videoWindowFree(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_508e70", 0, 0, nullptr, 0, kBx | kCx | kDx };
    const reg32 window = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::videoWindowFree(app, cpu, window); });
}

/* ---------------------------------------------------------------------------
 * Fonts
 * ------------------------------------------------------------------------- */

namespace
{
constexpr reg32 kFontRowCount = 0x20, kFontVersion = 0x28, kFontGlyphs = 0x30, kFontScaleX = 0xc0, kFontScaleY = 0xc4, kFontScale = 0xc8;
constexpr reg32 kFontStyle = 0xcc, kFontDirty = 0xbc;
constexpr reg32 kFixedToFloat = 0x551ee0, kFloatToFixed = 0x551ee8;

double doubleAt(Guest& g, reg32 address)
{
    return g.app()->getMemory<double>(address);
}
}

/* sub_5199b0: the glyph of a character (eax) found by halves in a table of records (edx)
 * sorted by it: ebx of them, ecx bytes each; 0 when it has none. */
reg32 frontend::menz::fontFindGlyph(App* app, CPU& cpu, reg32 character, reg32 table, reg32 count, reg32 size)
{
    Guest g(app, cpu);
    reg32 left = count;
    while (left != 0)
    {
        const reg32 probe = table + reg32(sreg32(left) >> 1) * size;
        const reg32 difference = character - g.word(probe);
        if (difference == 0)
            return probe;
        if (sreg32(difference) > 0)
        {
            --left;
            table = probe + size;
        }
        left = reg32(sreg32(left) >> 1);
    }
    return 0;
}

x86::reg32 frontend::fontFindGlyph(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_5199b0", 0, 0, nullptr, 0, kSi | kDi };
    const reg32 character = args.eax, table = args.edx, count = args.ebx, size = args.ecx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::fontFindGlyph(app, cpu, character, table, count, size); });
}

/* sub_519a50: FONT_getcharacter: the glyph of a character (edx) in a font (eax): 11 bytes a
 * record, straight at its place for a font in order, or found. */
reg32 frontend::menz::fontGlyph(App* app, CPU& cpu, reg32 font, reg32 character)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x14);
    constexpr reg32 kRecord = 11, kFirstCharacter = 0x20, kOldestVersion = 0x64;
    const reg32 glyphs = g.dword(font + kFontGlyphs);
    if (sreg32(g.dword(font + kFontVersion)) < sreg32(kOldestVersion))
        failAt(g, 0x5507ac, 0x5507bc, 0x160, { 0x5507d0 });
    const reg32 place = glyphs + (character - kFirstCharacter) * kRecord;
    if (g.word(place) == character)
        return place;
    return guest::direct<0x5199b0, fontFindGlyph>(app, cpu, character, glyphs, g.dword(font + kFontRowCount), kRecord);
}

x86::reg32 frontend::fontGlyph(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_519a50", 0, 0, nullptr, 0, kBx | kCx | kSi | kDi };
    const reg32 font = args.eax, character = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::fontGlyph(app, cpu, font, character); });
}

/* sub_519ea0: how many bits a pixel a shape's type (the byte at eax) gives a font's glyph:
 * 1 for the one-bit types, 4 for all else. */
reg32 frontend::menz::fontBitsPerPixel(App* app, CPU& cpu, reg32 shape)
{
    Guest g(app, cpu);
    const reg32 type = g.dword(shape) & 0xff;
    return type == 0x01 || type == 0x44 || type == 0x56 || type == 0x60 || type == 0x79 ? 1 : 4;
}

x86::reg32 frontend::fontBitsPerPixel(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_519ea0", 0, 0, nullptr, 0, kDx };
    const reg32 shape = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::fontBitsPerPixel(app, cpu, shape); });
}

/* sub_519f10: FONT_create: a font's record for a .FNT in memory (eax): 0 without memory. */
reg32 frontend::menz::fontCreate(App* app, CPU& cpu, reg32 data)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x14);
    constexpr reg32 kRecordSize = 0xe0, kDriver = 0x56718c, kAllocate = 0x14;
    constexpr reg32 kSignature = 0x464e5400;  // FNT, its version byte left out
    constexpr reg32 kDrawAny = 0x525670, kDrawAscii = 0x525630;
    const reg32 shape = g.dword(data + 0x1c) + data;
    const reg32 driver = g.dword(kDriver);
    const reg32 font = callAt(g, g.dword(driver + kAllocate), args({ kRecordSize, driver }));
    if (font == 0)
        return 0;
    cpu.esp -= 8;  // those two, saved only here
    const reg32 first = g.dword(data);
    const reg32 signature = ((first >> 24) | ((first >> 8) & 0xff00) | ((first << 8) & 0xff0000) | (first << 24)) & 0xffffff00;
    if (signature != kSignature)
        failAt(g, 0x550840, 0x550850, 0x6d, { 0x55085c, data });
    fe::direct<kMemset, watcom::clib::memset>(g.app(), g.cpu(), font, 0, kRecordSize);
    const auto signedByte = [&](reg32 address) { return reg32(sreg32(std::int8_t(g.byte(address)))); };
    g.setDword(font + kFontVersion, 0x64);
    g.setDword(font + 0x04, signedByte(data + 0x10));
    g.setDword(font + 0x08, signedByte(data + 0x11));
    g.setDword(font + 0x10, signedByte(data + 0x12));
    g.setDword(font + 0x14, signedByte(data + 0x13));
    g.setDword(font + 0x18, signedByte(data + 0x13) + signedByte(data + 0x12));
    g.setDword(font + 0x1c, signedByte(data + 0x13) + signedByte(data + 0x12));
    g.setDword(font + 0x48, 0);
    g.setDword(font + kFontRowCount, g.word(data + 0xa));
    g.setDword(font + 0x2c, data);
    g.setDword(font + 0x34, shape);
    g.setDword(font + kFontGlyphs, data + g.dword(data + 0x14));
    const reg32 bits = guest::direct<0x519ea0, fontBitsPerPixel>(app, cpu, shape);
    g.setDword(font + 0x0c, bits);
    const reg32 shapeRows = reg32(sreg32(g.dword(shape + 2)) >> 16);
    const reg32 rowBits = guest::direct<0x519ea0, fontBitsPerPixel>(app, cpu, shape) * shapeRows;
    g.setDword(font + 0x24, reg32(sreg32((rowBits + 7) & 0xfffffff8) >> 3));
    reg32 draw = kDrawAny;
    if ((reg32(sreg32(g.dword(data + 0xc)) >> 16) & 3) != 2 && sreg32(g.dword(font + kFontRowCount)) <= 0xff && g.word(data + 0x20) <= 0xff)
        draw = kDrawAscii;
    g.setDword(font + 0x40, draw);
    fe::direct<0x525b80, rendf::fontSetOrigin>(app, cpu, font, 0u, 0u);
    guest::direct<0x5257c0, fontSetProperty>(app, cpu, font, 0, 0);
    guest::direct<0x5257c0, fontSetProperty>(app, cpu, font, 5, 0);
    guest::direct<0x5257c0, fontSetProperty>(app, cpu, font, 6, 0);
    guest::direct<0x5257c0, fontSetProperty>(app, cpu, font, 7, 0);
    guest::direct<0x5257c0, fontSetProperty>(app, cpu, font, 0xc, 0xffffffff);
    guest::direct<0x5257c0, fontSetProperty>(app, cpu, font, 8, 0xffffffff);
    guest::direct<0x5257c0, fontSetProperty>(app, cpu, font, 9, 0xffffff);
    guest::direct<0x5257c0, fontSetProperty>(app, cpu, font, 0xa, 0xff000000);
    guest::direct<0x5257c0, fontSetProperty>(app, cpu, font, 0xb, 0xff000000);
    g.setByte(font + kFontDirty, 1);
    const reg32 glyph = guest::direct<0x519a50, fontGlyph>(app, cpu, font, 0x41);
    const reg32 capHeight = g.byte(glyph + 3) + signedByte(glyph + 10);
    g.setDword(font + 0x14, g.dword(font + 0x10) - capHeight);
    g.setDword(font + 0x10, capHeight);
    return font;
}

x86::reg32 frontend::fontCreate(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_519f10", 0, 0, nullptr, 0, kDx | kSi | kDi };
    const reg32 data = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::fontCreate(app, cpu, data); });
}

/* sub_5257c0: a font's (eax) property (edx) set to ebx: the scales in 16.16, the spacing
 * they give, the style's bits, colours and the shadow's; property 0 sets the scales to 1. */
reg32 frontend::menz::fontSetProperty(App* app, CPU& cpu, reg32 font, reg32 property, reg32 value)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x18);
    const auto scaled = [&](reg32 fixed) -> Float {
        return fp::mul(Float(x86::sreg64(x86::reg64(fixed))), Float(doubleAt(g, kFixedToFloat)));
    };
    const auto setBit = [&](std::uint8_t bit) {
        const std::uint8_t style = g.byte(font + kFontStyle);
        g.setByte(font + kFontStyle, value == 1 ? style | bit : style & std::uint8_t(~bit));
    };
    const auto setColour = [&](reg32 field) {
        if (value != g.dword(font + field))
        {
            g.setByte(font + kFontDirty, 1);
            g.setDword(font + field, value);
        }
    };
    if (property == 0)
    {
        guest::direct<0x5257c0, fontSetProperty>(app, cpu, font, 1, 0);
        guest::direct<0x5257c0, fontSetProperty>(app, cpu, font, 2, 0x10000);
        value = 0x10000;
        property = 3;
    }
    switch (property)
    {
    case 1:
        g.setFloat(font + kFontScaleX, float(scaled(value)));
        break;
    case 2:
        g.setFloat(font + kFontScaleY, float(scaled(value)));
        break;
    case 3:
        g.setFloat(font + kFontScale, float(scaled(value)));
        break;
    case 4:
    {
        const Float pixels = fp::div(scaled(value), Float(g.floatAt(font + kFontScale)));
        value = reg32(fp::toInteger<sreg32>(pixels));
        g.setDword(font + 0x18, value);
        break;
    }
    case 5: setBit(1); break;
    case 6: setBit(2); break;
    case 7: setBit(4); break;
    case 8: setColour(0x6c); break;
    case 9: setColour(0x70); break;
    case 0xa: setColour(0x74); break;
    case 0xb: setColour(0x78); break;
    case 0xc:
        g.setDword(font + 0x60, value);
        g.setDword(font + 0x64, value);
        g.setDword(font + 0x68, value);
        g.setDword(font + 0x5c, value);
        break;
    case 0xd: g.setDword(font + 0x5c, value); break;
    case 0xe: g.setDword(font + 0x60, value); break;
    case 0xf: g.setDword(font + 0x64, value); break;
    case 0x10: g.setDword(font + 0x68, value); break;
    default: break;
    }
    return value;
}

x86::reg32 frontend::fontSetProperty(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_5257c0", 0, 0, nullptr, 0, kCx | kSi | kDi };
    const reg32 font = args.eax, property = args.edx, value = args.ebx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::fontSetProperty(app, cpu, font, property, value); });
}

/* sub_5259d0: a font's (eax) property (edx) read back, as sub_5257c0 sets it; ecx for one
 * it has not.  Nothing in the game calls it. */
reg32 frontend::menz::fontGetProperty(App* app, CPU& cpu, reg32 font, reg32 asked, reg32 ecx)
{
    Guest g(app, cpu);
    const reg32 property = asked - 1;
    const auto fixed = [&](reg32 field) {
        return reg32(fp::toInteger<sreg32>(fp::mul(Float(g.floatAt(font + field)), Float(doubleAt(g, kFloatToFixed)))));
    };
    const auto bit = [&](std::uint8_t mask) -> reg32 { return (g.byte(font + kFontStyle) & mask) ? 1 : 0; };
    switch (property)
    {
    case 0: return fixed(kFontScaleX);
    case 1: return fixed(kFontScaleY);
    case 2: return fixed(kFontScale);
    case 3:
    {
        const Float spacing = fp::mul(Float(sreg32(g.dword(font + 0x18))), Float(g.floatAt(font + kFontScale)));
        return reg32(fp::toInteger<sreg32>(fp::mul(spacing, Float(doubleAt(g, kFloatToFixed)))));
    }
    case 4: return bit(1);
    case 5: return bit(2);
    case 6: return bit(4);
    case 7: return g.dword(font + 0x6c);
    case 8: return g.dword(font + 0x70);
    case 9: return g.dword(font + 0x74);
    case 0xa: return g.dword(font + 0x78);
    case 0xb:
    case 0xc: return g.dword(font + 0x5c);
    case 0xd: return g.dword(font + 0x60);
    case 0xe: return g.dword(font + 0x64);
    case 0xf: return g.dword(font + 0x68);
    default: return ecx;
    }
}

x86::reg32 frontend::fontGetProperty(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_5259d0", 0, 0, nullptr, 0, kCx };
    const reg32 font = args.eax, property = args.edx, ecx = args.ecx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::fontGetProperty(app, cpu, font, property, ecx); });
}

/* ---------------------------------------------------------------------------
 * Palettes
 * ------------------------------------------------------------------------- */

/* sub_51de60: eax colours of 3 bytes of 6 bits each (edx) to 4 bytes of 8 (ebx), the last
 * colour's alpha 0. */
reg32 frontend::menz::paletteFrom6Bits(App* app, CPU& cpu, reg32 count, reg32 from, reg32 to)
{
    Guest g(app, cpu);
    reg32 result = to;
    const auto widen = [&](reg32 at) -> reg32 {
        const sreg32 sum = sreg32(g.byte(at)) * 0xff + 0x20;
        return reg32(sum / 0x3f);
    };
    for (reg32 i = 0; sreg32(i) < sreg32(count); ++i)
    {
        g.setByte(to + 3, i == 0xff ? 0 : 0xff);
        g.setByte(to + 2, std::uint8_t(widen(from)));
        g.setByte(to + 1, std::uint8_t(widen(from + 1)));
        result = widen(from + 2);
        g.setByte(to, std::uint8_t(result));
        to += 4;
        from += 3;
    }
    return result;
}

x86::reg32 frontend::paletteFrom6Bits(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_51de60", 0, 0, nullptr, 0, kCx | kSi | kDi };
    const reg32 count = args.eax, from = args.edx, to = args.ebx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::paletteFrom6Bits(app, cpu, count, from, to); });
}

/* sub_51df60: eax colours of 3 bytes (edx) to 4 (ebx), the last colour's alpha 0. */
reg32 frontend::menz::paletteFrom8Bits(App* app, CPU& cpu, reg32 count, reg32 from, reg32 to)
{
    Guest g(app, cpu);
    reg32 result = count;
    for (reg32 i = 0; sreg32(i) < sreg32(count); ++i)
    {
        g.setByte(to + 3, i == 0xff ? 0 : 0xff);
        g.setByte(to + 2, g.byte(from));
        g.setByte(to + 1, g.byte(from + 1));
        const std::uint8_t last = g.byte(from + 2);
        g.setByte(to, last);
        to += 4;
        from += 3;
        result = to;
    }
    return result;
}

x86::reg32 frontend::paletteFrom8Bits(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_51df60", 0, 0, nullptr, 0, kCx | kSi | kDi };
    const reg32 count = args.eax, from = args.edx, to = args.ebx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::paletteFrom8Bits(app, cpu, count, from, to); });
}

/* ---------------------------------------------------------------------------
 * Watcom's doubles and the FDIV test
 * ------------------------------------------------------------------------- */

/* sub_52448c: what a long double (eax) is: 0 zero, 1 a number, 2 NaN, 3 infinity, 4 denormal. */
reg32 frontend::menz::longDoubleKind(App* app, CPU& cpu, reg32 value)
{
    Guest g(app, cpu);
    const std::uint16_t exponent = g.word(value + 8) & 0x7fff;
    const reg32 fractionLow = g.dword(value), fractionHigh = g.dword(value + 4);
    if (exponent == 0x7fff)
        return fractionHigh == 0x80000000 && fractionLow == 0 ? 3 : 2;
    if (exponent != 0)
        return 1;
    return fractionHigh == 0 && fractionLow == 0 ? 0 : 4;
}

x86::reg32 frontend::longDoubleKind(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_52448c", 0, 0, nullptr, 0, kDx };
    const reg32 value = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::longDoubleKind(app, cpu, value); });
}

namespace
{

/* The add, subtract, shift and rotate of a double kept in two registers, with the carry
 * the original keeps in its flags. */
struct Carry
{
    bool cf = false;

    reg32 add(reg32 a, reg32 b)
    {
        const std::uint64_t sum = std::uint64_t(a) + b;
        cf = (sum >> 32) != 0;
        return reg32(sum);
    }
    reg32 adc(reg32 a, reg32 b)
    {
        const std::uint64_t sum = std::uint64_t(a) + b + (cf ? 1 : 0);
        cf = (sum >> 32) != 0;
        return reg32(sum);
    }
    reg32 sbb(reg32 a, reg32 b)
    {
        const std::uint64_t difference = std::uint64_t(a) - b - (cf ? 1 : 0);
        cf = (difference >> 32) != 0;
        return reg32(difference);
    }
    reg32 neg(reg32 a)
    {
        cf = a != 0;
        return reg32(0) - a;
    }
    reg32 shr(reg32 a)
    {
        cf = (a & 1) != 0;
        return a >> 1;
    }
    reg32 rcr(reg32 a)
    {
        const bool out = (a & 1) != 0;
        a = (a >> 1) | (reg32(cf ? 1 : 0) << 31);
        cf = out;
        return a;
    }
};

/* shrd by cl: the count taken modulo 32, and a count of 0 moves nothing. */
inline reg32 shiftInRight(reg32 to, reg32 from, std::uint8_t count)
{
    const unsigned n = count % 32;
    return n == 0 ? to : (to >> n) | (from << (32 - n));
}

inline reg32 ror(reg32 value, unsigned n)
{
    return (value >> n) | (value << (32 - n));
}

inline std::uint16_t low16(reg32 value)
{
    return std::uint16_t(value);
}

inline reg32 withLow16(reg32 value, std::uint32_t low)
{
    return (value & 0xffff0000) | (low & 0xffff);
}

/* A double's sign bit at 31 and exponent in the low word: `sar 20` and the mask. */
inline reg32 signAndExponent(reg32 high)
{
    return reg32(sreg32(high) >> 20) & 0x800007ff;
}

/* The upper words added, the low one of `base` left: how the two signs are summed. */
inline reg32 signsSummed(reg32 base, reg32 other)
{
    return (((base >> 16) + (other >> 16)) << 16) | (base & 0xffff);
}

inline double doubleOf(reg32 high, reg32 low)
{
    const std::uint64_t bits = (std::uint64_t(high) << 32) | low;
    double value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

constexpr reg32 kFpuKind = 0x5644c1;      // 0 no x87
constexpr reg32 kFdivBug = 0x567858;      // bit 0: this x87 has the bug
constexpr reg32 kOverflow = 0x52c8a7, kUnderflow = 0x52c88a, kDivideByZero = 0x52c89b;

/* The x87's way's end: the answer in edx:eax, a negative zero made zero. */
inline reg32 answerFromX87(Guest& g, double value)
{
    std::uint64_t bits;
    std::memcpy(&bits, &value, sizeof bits);
    reg32 high = reg32(bits >> 32), low = reg32(bits);
    if (high == 0x80000000)
        high = low = 0;
    return low;
}

/* A double with its exponent 0: shifted up until its leading bit is where the hidden one
 * would be, the exponent going down below zero for it. */
inline void normaliseDenormal(Carry& c, reg32& high, reg32& low, reg32& exponent)
{
    do
    {
        low = c.add(low, low);
        high = c.adc(high, high);
        exponent = withLow16(exponent, low16(exponent) - 1u);
    } while ((high & 0x100000) == 0);
}

}  // namespace

/* sub_52643c: edx:eax divided by ecx:ebx, Watcom's __FDD: through the x87 where there is one
 * (through sub_52700c where it has the bug), else long division a word at a time. */
reg32 frontend::menz::doubleDivide(App* app, CPU& cpu, reg32 low, reg32 high, reg32 divisorLow, reg32 divisorHigh, double top)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 8);
    if (divisorLow == 0 && (divisorHigh << 1) == 0)
    {
        return reg32(fe::direct<0x52c89b, watc::doubleInfinityHook>(app, cpu));
    }
    if (low == 0 && (high << 1) == 0)
    {
        return 0;
    }
    if (g.byte(kFpuKind) != 0)
    {
        Float quotient = Float(doubleOf(high, low));
        if (g.byte(kFdivBug) & 1)
        {
            // sub_52700c divides st(0), its st(1) what the caller's x87 has on top: that one is
            // is `top`, the x87's stack the caller's interface.
            const double below = top;
            quotient = guest::direct<0x52700c, safeDivide>(app, cpu, double(quotient), below, divisorLow, divisorHigh);
            if (cpu.terminate)
                throw fe::Terminated();
        }
        else
        {
            quotient = fp::div(quotient, Float(doubleOf(divisorHigh, divisorLow)));
        }
        return answerFromX87(g, double(quotient));
    }

    Carry c;
    reg32 exponent = signAndExponent(high);
    reg32 divisorExponent = signAndExponent(divisorHigh);
    exponent = signsSummed(exponent, divisorExponent);
    high &= 0xfffff;
    divisorHigh &= 0xfffff;
    if (low16(exponent) != 0)
        high |= 0x100000;
    else
        normaliseDenormal(c, high, low, exponent);
    if (low16(divisorExponent) != 0)
        divisorHigh |= 0x100000;
    else
        normaliseDenormal(c, divisorHigh, divisorLow, divisorExponent);
    exponent = withLow16(exponent, std::uint32_t(low16(exponent)) - low16(divisorExponent) + 0x3ff);
    if ((low16(exponent) & 0x8000) == 0 && low16(exponent) >= 0x7ff)
    {
        return reg32(fe::direct<0x52c8a7, watc::doubleInfinity>(app, cpu, divisorExponent));
    }
    if (std::int16_t(low16(exponent)) < -0x34)
    {
        return reg32(fe::direct<0x52c88a, watc::doubleZero>(app, cpu));
    }

    // The mantissas at the top of 64 bits, and the quotient's three words by the first
    // word of the divisor, corrected down by what the second word says.
    const reg32 topHigh = (high << 11) | (low >> 21), topLow = low << 11;
    const reg32 byHigh = (divisorHigh << 11) | (divisorLow >> 21), byLow = divisorLow << 11;
    reg32 restHigh = topHigh, rest = topLow;
    reg32 word2 = 0;
    reg32 numerator = topHigh;
    if (byHigh <= numerator)
    {
        numerator -= byHigh;
        word2 = 1;
    }
    reg32 word1 = reg32(((std::uint64_t(numerator) << 32) | topLow) / byHigh);
    std::uint64_t part = std::uint64_t(byLow) * word1;
    reg32 restLow = reg32(part);
    std::uint64_t upper = std::uint64_t(byHigh) * word1 + reg32(part >> 32);
    reg32 upperLow = reg32(upper), upperHigh = reg32(upper >> 32);
    if (word2 & 1)
    {
        upperLow = c.add(upperLow, byLow);
        upperHigh = c.adc(upperHigh, byHigh);
    }
    restLow = c.neg(restLow);
    rest = c.sbb(rest, upperLow);
    restHigh = c.sbb(restHigh, upperHigh);
    while (restHigh != 0)
    {
        c.cf = word1 < 1;
        word1 -= 1;
        word2 = c.sbb(word2, 0);
        restLow = c.add(restLow, byLow);
        rest = c.adc(rest, byHigh);
        restHigh = c.adc(restHigh, 0);
    }
    restHigh = rest;
    rest = restLow;
    restLow = byHigh;
    if (byHigh <= restHigh)
    {
        restHigh -= byHigh;
        word1 = c.add(word1, 1);
        word2 = c.adc(word2, 0);
    }
    reg32 word0 = reg32(((std::uint64_t(restHigh) << 32) | rest) / byHigh);
    if (word0 != 0)
    {
        part = std::uint64_t(byLow) * word0;
        restLow = reg32(part);
        upper = std::uint64_t(byHigh) * word0 + reg32(part >> 32);
        restLow = c.neg(restLow);
        rest = c.sbb(rest, reg32(upper));
        restHigh = c.sbb(restHigh, reg32(upper >> 32));
        while (restHigh != 0)
        {
            c.cf = word0 < 1;
            word0 -= 1;
            word1 = c.sbb(word1, 0);
            word2 = c.sbb(word2, 0);
            restLow = c.add(restLow, byLow);
            rest = c.adc(rest, byHigh);
            restHigh = c.adc(restHigh, 0);
        }
    }

    // Back to a double's place, rounded.
    reg32 resultLow = word0, resultHigh = word1, leftover = word2;
    exponent = withLow16(exponent, low16(exponent) - 1u);
    leftover = c.shr(leftover);
    if (c.cf)
    {
        resultHigh = c.rcr(resultHigh);
        resultLow = c.rcr(resultLow);
        exponent += 1;
    }
    reg32 count = (restLow & 0xffffff00) | 0xb;
    const bool round = ((resultLow >> 10) & 1) != 0;
    resultLow = (resultLow >> 11) | (resultHigh << 21);
    resultHigh = (resultHigh >> 11) | 0xfff00000;
    c.cf = round;
    resultLow = c.adc(resultLow, 0);
    resultHigh = c.adc(resultHigh, 0);
    exponent = c.adc(exponent, 0);
    if (std::int16_t(low16(exponent)) <= 0)
    {
        if (low16(exponent) == 0)
        {
            count = (count & 0xffffff00) | 1;
        }
        else
        {
            exponent = withLow16(exponent, 0u - low16(exponent));
            count = withLow16(count, low16(exponent));
        }
        resultHigh &= 0x1fffff;
        leftover = 0;
        resultLow = shiftInRight(resultLow, resultHigh, std::uint8_t(count));
        resultHigh = shiftInRight(resultHigh, 0, std::uint8_t(count));
        exponent = withLow16(exponent, 0);
    }
    resultHigh &= 0xfffff;
    c.cf = (exponent >> 31) != 0;
    resultHigh |= c.rcr(ror(exponent, 11)) & 0xfff00000;
    return resultLow;
}

x86::reg32 frontend::doubleDivide(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_52643c", 0, 0, nullptr, 0, kSi | kDi };
    const reg32 low = args.eax, high = args.edx, otherLow = args.ebx, otherHigh = args.ecx;
    const double top = args.floatIn[0];
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::doubleDivide(app, cpu, low, high, otherLow, otherHigh, top); });
}

/* sub_526642: edx:eax less ecx:ebx, Watcom's __FDS. */
reg32 frontend::menz::doubleSubtract(App* app, CPU& cpu, reg32 low, reg32 high, reg32 otherLow, reg32 subtracted)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 8);
    reg32 otherHigh = subtracted ^ 0x80000000;
    if (otherLow == 0 && (otherHigh << 1) == 0)
    {
        return low;
    }
    if (low == 0 && (high << 1) == 0)
    {
        return otherLow;
    }
    if (g.byte(kFpuKind) != 0)
        return answerFromX87(g, double(fp::add(Float(doubleOf(high, low)), Float(doubleOf(otherHigh, otherLow)))));

    Carry c;
    const auto finish = [&](reg32 resultLow, reg32 resultHigh, reg32 ebx, reg32 ecx) -> reg32 {
        return resultLow;
    };
    const reg32 exponent = signAndExponent(high);
    reg32 other = signAndExponent(otherHigh);
    reg32 sign = other;
    other = signsSummed(other, exponent);
    high &= 0xfffff;
    otherHigh &= 0xfffff;
    if (low16(exponent) != 0)
        high |= 0x100000;
    if (low16(other) != 0)
        otherHigh |= 0x100000;
    low = c.add(low, low);
    high = c.adc(high, high);
    otherLow = c.add(otherLow, otherLow);
    otherHigh = c.adc(otherHigh, otherHigh);
    const bool otherSmaller = low16(other) < low16(exponent);
    other = withLow16(other, std::uint32_t(low16(other)) - low16(exponent));
    if (low16(other) != 0)
    {
        if (otherSmaller)
        {
            sign = exponent;
            other = withLow16(other, 0u - low16(other));
            std::swap(otherLow, low);
            std::swap(high, otherHigh);
        }
        if (low16(other) > 0x36)
        {
            // Too far apart to matter: the larger, as it is.
            reg32 resultHigh = otherHigh, resultLow = otherLow;
            c.cf = (sign >> 31) != 0;
            sign <<= 1;
            resultHigh = c.rcr(resultHigh);
            resultLow = c.rcr(resultLow);
            resultHigh &= 0x800fffff;
            resultHigh |= ror(sign, 13) & 0x7ff00000;
            return finish(resultLow, resultHigh, otherLow, other);
        }
    }
    if (sreg32(other) < 0)
    {
        otherHigh = reg32(0) - otherHigh;
        otherLow = c.neg(otherLow);
        otherHigh = c.sbb(otherHigh, 0);
        sign ^= 0x80000000;
    }
    reg32 sticky = 0;
    const std::uint8_t shift = std::uint8_t(other);
    if (shift != 0)
    {
        if (shift >= 0x20)
        {
            sticky = low != 0 ? 1 : 0;
            low = high;
            high = 0;
        }
        sticky |= shiftInRight(0, low, shift);
        low = shiftInRight(low, high, shift);
        high = shiftInRight(high, 0, shift);
    }
    reg32 scratch = otherLow;
    low = c.add(low, otherLow);
    high = c.adc(high, otherHigh);
    if (sreg32(high) < 0)
    {
        if (shift >= 0x35)
        {
            scratch = (scratch & 0xffffff00) | ((sticky & 0x7fffffff) != 0 ? 1 : 0);
            scratch = c.shr(scratch);
            low = c.adc(low, 0);
            high = c.adc(high, 0);
        }
        high = reg32(0) - high;
        low = c.neg(low);
        high = c.sbb(high, 0);
        sign ^= 0x80000000;
    }
    scratch = low | high;
    if (scratch == 0)
        return finish(low, high, scratch, other);
    const auto tiny = [&]() -> reg32 {
        c.cf = (sign >> 31) != 0;
        high = c.rcr(high);
        low = c.rcr(low);
        return finish(low, high, scratch, other);
    };
    if (low16(sign) == 0)
        return tiny();
    while ((high & 0x7fe00000) == 0)
    {
        sign = withLow16(sign, low16(sign) - 1u);
        if (low16(sign) == 0)
            return tiny();
        low = c.add(low, low);
        high = c.adc(high, high);
    }
    const auto overflow = [&]() -> reg32 {
        return reg32(fe::direct<0x52c8a7, watc::doubleInfinity>(app, cpu, sign));
    };
    if (high & 0x400000)
    {
        high = c.shr(high);
        low = c.rcr(low);
        sticky = c.adc(sticky, 0);
        sign = withLow16(sign, low16(sign) + 1u);
        if (low16(sign) == 0x7ff)
            return overflow();
    }
    high = c.shr(high);
    low = c.rcr(low);
    if (c.cf)
    {
        scratch = (scratch & 0xffffff00) | (sticky != 0 ? 1 : 0);
        scratch |= low;
        scratch = c.shr(scratch);
        low = c.adc(low, 0);
        high = c.adc(high, 0);
        if (high & 0x200000)
        {
            high = c.shr(high);
            low = c.rcr(low);
            sign = withLow16(sign, low16(sign) + 1u);
            if (low16(sign) == 0x7ff)
                return overflow();
        }
    }
    high &= 0xfffff;
    c.cf = (sign >> 31) != 0;
    high |= c.rcr(sign << 21);
    return finish(low, high, scratch, sign << 1);
}

x86::reg32 frontend::doubleSubtract(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_526642", 0, 0, nullptr, 0, kSi | kDi };
    const reg32 low = args.eax, high = args.edx, otherLow = args.ebx, otherHigh = args.ecx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::doubleSubtract(app, cpu, low, high, otherLow, otherHigh); });
}

/* sub_5267ff: edx:eax times ecx:ebx, Watcom's __FDM. */
reg32 frontend::menz::doubleMultiply(App* app, CPU& cpu, reg32 low, reg32 high, reg32 otherLow, reg32 otherHigh)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 8);
    if (low == 0 && (high << 1) == 0)
    {
        return 0;
    }
    if (otherLow == 0 && (otherHigh << 1) == 0)
    {
        return 0;
    }
    if (g.byte(kFpuKind) != 0)
        return answerFromX87(g, double(fp::mul(Float(doubleOf(high, low)), Float(doubleOf(otherHigh, otherLow)))));

    Carry c;
    reg32 exponent = signAndExponent(high);
    reg32 sum = signAndExponent(otherHigh);
    sum = signsSummed(sum, exponent);
    high &= 0xfffff;
    otherHigh &= 0xfffff;
    if (low16(exponent) == 0)
    {
        exponent = withLow16(exponent, 1);
        normaliseDenormal(c, high, low, exponent);
    }
    high |= 0x100000;
    if (low16(sum) == 0)
    {
        sum = withLow16(sum, 1);
        normaliseDenormal(c, otherHigh, otherLow, sum);
    }
    otherHigh |= 0x100000;
    sum = withLow16(sum, std::uint32_t(low16(sum)) + low16(exponent) - 0x3ff);
    if ((low16(sum) & 0x8000) == 0 && low16(sum) >= 0x7ff)
    {
        return reg32(fe::direct<0x52c8a7, watc::doubleInfinity>(app, cpu, sum));
    }
    if (std::int16_t(low16(sum)) < -0x35)
    {
        return reg32(fe::direct<0x52c88a, watc::doubleZero>(app, cpu));
    }
    const reg32 aHigh = (high << 11) | (low >> 21), aLow = low << 11;
    const reg32 bHigh = (otherHigh << 11) | (otherLow >> 21), bLow = otherLow << 11;

    // Four words of the product.  `lowest` stays only to say whether any bit is set there.
    const std::uint64_t lowLow = std::uint64_t(aLow) * bLow;
    const reg32 lowest = reg32(lowLow);
    reg32 second = reg32(lowLow >> 32);
    const std::uint64_t cross1 = std::uint64_t(bHigh) * aLow;
    reg32 third = reg32(cross1 >> 32);
    reg32 fourth = 0;
    second = c.add(second, reg32(cross1));
    third = c.adc(third, 0);
    fourth = c.adc(fourth, fourth);
    const std::uint64_t cross2 = std::uint64_t(bLow) * aHigh;
    second = c.add(second, reg32(cross2));
    third = c.adc(third, reg32(cross2 >> 32));
    fourth = c.adc(fourth, 0);
    const std::uint64_t highHigh = std::uint64_t(aHigh) * bHigh;
    reg32 resultLow = c.add(reg32(highHigh), third);
    reg32 resultHigh = c.adc(reg32(highHigh >> 32), fourth);
    reg32 below = shiftInRight(0, resultLow, 10);
    resultLow = shiftInRight(resultLow, resultHigh, 10);
    resultHigh = shiftInRight(resultHigh, below, 10);
    const auto overflow = [&]() -> reg32 {
        return reg32(fe::direct<0x52c8a7, watc::doubleInfinity>(app, cpu, sum));
    };
    while (resultHigh & 0x200000)
    {
        resultHigh = c.shr(resultHigh);
        resultLow = c.rcr(resultLow);
        below = c.rcr(below);
        sum = withLow16(sum, low16(sum) + 1u);
        if (low16(sum) == 0x7ff)
            return overflow();
    }
    below = c.add(below, below);
    if (c.cf)
    {
        if (below == 0)
        {
            below = c.shr(lowest != 0 ? 1 : 0);
            if (!c.cf)
                c.cf = (resultLow & 1) != 0;
        }
        resultLow = c.adc(resultLow, 0);
        resultHigh = c.adc(resultHigh, 0);
        if (resultHigh & 0x200000)
        {
            resultHigh = c.shr(resultHigh);
            resultLow = c.rcr(resultLow);
            sum = withLow16(sum, low16(sum) + 1u);
            if (low16(sum) == 0x7ff)
                return overflow();
        }
    }
    if (std::int16_t(low16(sum)) <= 0)
    {
        if (low16(sum) == 0)
            sum = (sum & 0xffffff00) | 1;
        else
            sum = withLow16(sum, 0u - low16(sum) - 1u);
        below = 0;
        resultLow = shiftInRight(resultLow, resultHigh, std::uint8_t(sum));
        resultHigh = shiftInRight(resultHigh, 0, std::uint8_t(sum));
        sum = withLow16(sum, 0);
    }
    resultHigh &= 0xfffff;
    c.cf = (sum >> 31) != 0;
    const reg32 place = c.rcr(ror(sum, 11)) & 0xfff00000;
    return resultLow;
}

x86::reg32 frontend::doubleMultiply(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_5267ff", 0, 0, nullptr, 0, kSi | kDi };
    const reg32 low = args.eax, high = args.edx, otherLow = args.ebx, otherHigh = args.ecx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::doubleMultiply(app, cpu, low, high, otherLow, otherHigh); });
}

/* sub_526674: the x87's way's end for sub_5267ff, which has jumped here into sub_526642: the
 * answer `product` (st(0), which its thunk takes off), on the stack the other double and the
 * first, below the return address. */
reg32 frontend::menz::doubleProductFromX87(App* app, CPU& cpu, double product)
{
    Guest g(app, cpu);
    return answerFromX87(g, product);
}

x86::reg32 frontend::doubleProductFromX87(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_526674", 0x10, 0, nullptr, 0, 0 };
    const double product = args.floatIn[0];
    args.floatsTaken = 1;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::doubleProductFromX87(app, cpu, product); });
}

/* sub_52699c: edx:eax against ecx:ebx, a denormal taken for zero: -1, 0 or 1.  Its one
 * caller, sub_51f2a0, goes by the sign of the answer. */
reg32 frontend::menz::doubleCompare(App* app, CPU& cpu, reg32 low, reg32 high, reg32 otherLow, reg32 otherHigh)
{
    Guest g(app, cpu);
    if ((high & 0x7ff00000) == 0)
        high = 0;
    if ((otherHigh & 0x7ff00000) == 0)
        otherHigh = 0;
    if (sreg32(otherHigh ^ high) >= 0)
    {
        // Of one sign: which is further from zero, the sign turning the answer over.
        bool below;
        if (high != otherHigh)
            below = high < otherHigh;
        else if (low != otherLow)
            below = low < otherLow;
        else
            return 0;
        high ^= (otherHigh >> 1) | (below ? 0x80000000 : 0);
    }
    const bool negative = (high >> 31) != 0;
    return negative ? 0xffffffff : 1;
}

x86::reg32 frontend::doubleCompare(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_52699c", 0, 0, nullptr, 0, 0 };
    const reg32 low = args.eax, high = args.edx, otherLow = args.ebx, otherHigh = args.ecx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::doubleCompare(app, cpu, low, high, otherLow, otherHigh); });
}

/* sub_51f2a0: the runtime's start-up test of the Pentium's FDIV bug on an x87 of the 80387's
 * or later: 4195835 / 3145727 * 3145727 taken from 4195835, which a sound one has within
 * 1e-8 of nothing.  A bad one sets bit 0 of 0x567858.  The answer's sign is what the
 * original's jump after sub_52699c reads, 1e-8 being positive. */
reg32 frontend::menz::fdivBugCheck(App* app, CPU& cpu, reg32 eax, double top)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x1c);
    constexpr reg32 kBigLow = 0xc0000000, kBigHigh = 0x4150017e;    // 4195835.0
    constexpr reg32 kByLow = 0x80000000, kByHigh = 0x4147ffff;      // 3145727.0
    constexpr reg32 kTinyLow = 0xe2308c3a, kTinyHigh = 0x3e45798e;  // 1e-8
    if (g.byte(kFpuKind) < 3)
        return eax;
    // The division's slot on the x87 holds a copy of the caller's st(0), `top`, for the callee.
    // Each answer's high word stays where the call left it: in edx, the first dividend's (the x87's
    // way hands back only the low word in eax).
    const reg32 quotient = guest::direct<0x52643c, doubleDivide>(app, cpu, kBigLow, kBigHigh, kByLow, kByHigh, top);
    const reg32 product = guest::direct<0x5267ff, doubleMultiply>(app, cpu, quotient, kBigHigh, kByLow, kByHigh);
    const reg32 difference = guest::direct<0x526642, doubleSubtract>(app, cpu, kBigLow, kBigHigh, product, kBigHigh);
    const reg32 order = guest::direct<0x52699c, doubleCompare>(app, cpu, difference, kBigHigh, kTinyLow, kTinyHigh);
    if (sreg32(order) > 0)
        g.setByte(kFdivBug, g.byte(kFdivBug) | 1);
    return order;
}

x86::reg32 frontend::fdivBugCheck(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_51f2a0", 0, 0, nullptr, 0, kBx | kCx | kDx };
    const reg32 eax = args.eax;
    const double top = args.floatIn[0];
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return menz::fdivBugCheck(app, cpu, eax, top); });
}

/* sub_52700c: `top` (st(0)) divided by a double on the stack, safe from the bug: through the
 * runtime's careful division (sub_526f87), whose two x87 slots are made if the stack has
 * room, else `below` (st(1)) kept in memory for it.  A NaN or an infinity simply divides.  The
 * quotient, which its thunk leaves in st(0); st(1) as it was (a double through a long double
 * is itself), eax as it came. */
double frontend::menz::safeDivide(App* app, CPU& cpu, double top, double below, reg32 divisorLow, reg32 divisorHigh)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 4);
    const Float divisor = Float(doubleOf(divisorHigh, divisorLow));
    if ((divisorHigh & 0x7ff00000) == 0x7ff00000)
        return fp::div(Float(top), divisor);
    // (the x87's stack always has room here: its status word's top is never moved)
    cpu.esp -= 0xc;
    g.app()->getMemory<x86::IEEEf80>(cpu.esp) = Float(below);
    const double quotient = guest::direct<0x526f87, safeDivideCall>(app, cpu, top, double(divisor));
    if (cpu.terminate)
        throw fe::Terminated();
    return quotient;
}

x86::reg32 frontend::safeDivide(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_52700c", 8, 0, nullptr, 0, 0 };
    const reg32 divisorLow = stackWord(app, cpu, 0), divisorHigh = stackWord(app, cpu, 1);
    const double top = args.floatIn[0], below = args.floatIn[1];
    return fe::runThunk(app, cpu, args, site, [&](Guest&) {
        const reg32 eax = args.eax;  // kept
        const double quotient = menz::safeDivide(app, cpu, top, below, divisorLow, divisorHigh);
        // st(0) the quotient, st(1) as it was (stored back as it came).
        args.floatsTaken = 1;
        args.floatSet = true;
        args.floatResult = quotient;
        return eax;
    });
}

/* sub_526f87: `divisor` (st(0)) and `dividend` (st(1)) put in memory as long doubles for the
 * careful division (sub_5269d4, by address: a stub, see native_dead), and its answer back, which
 * the thunk leaves in their place; eax as the division leaves it. */
double frontend::menz::safeDivideCall(App* app, CPU& cpu, double dividend, double divisor)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x2c);
    g.app()->getMemory<x86::IEEEf80>(g.frame()) = Float(divisor);
    g.app()->getMemory<x86::IEEEf80>(g.frame() + 0xc) = Float(dividend);
    x86::Args none;
    if (!guest::pointerCall(app, cpu, 0x5269d4, none))
        throw fe::Terminated();
    return none.floatResult;
}

x86::reg32 frontend::safeDivideCall(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static Site site{ "sub_526f87", 0, 0, nullptr, 0, 0 };
    const double divisor = args.floatIn[0], dividend = args.floatIn[1];
    return fe::runThunk(app, cpu, args, site, [&](Guest&) {
        const double answer = menz::safeDivideCall(app, cpu, dividend, divisor);
        // the two operands replaced by the answer
        args.floatsTaken = 2;
        args.floatSet = true;
        args.floatResult = answer;
        return args.eax;
    });
}

}  // namespace nfs3hp
