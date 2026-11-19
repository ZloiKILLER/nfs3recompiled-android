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
#include "os_threads.h"
#include "os_time.h"
#include "os_video.h"
#include "os_window.h"
#include "port_snd_stream_calls.h"
#include <game_float.h>
#include <nfs3hp.h>
#include "os_system.h"
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
nfs::u32 callAt(fe::Guest& guest, nfs::u32 target, const fe::Arguments& arguments = fe::Arguments())
{
    nfs::ThreadState& ts = guest.ts();
    nfs::Args registers;
    nfs::u32* const into[4] = { &registers.eax, &registers.edx, &registers.ebx, &registers.ecx };
    for (std::size_t i = 0; i < arguments.registerCount; ++i)
        *into[i] = arguments.registers[i];
    const nfs::u32 esp = ts.sp;
    for (std::size_t i = arguments.stackCount; i-- > 0;)
    {
        ts.sp -= 4;
        guest.setDword(ts.sp, arguments.stack[i]);
    }
    const std::optional<nfs::u32> result = guest::pointerCall(guest.app(), ts, target, registers);
    if (!result)
        throw fe::Terminated();
    ts.sp = esp;
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
 * runtime's doubles of sub_52c88a to sub_52c8a7 (edx:eax); the display, its surfaces and the
 * window are the port's own (os_video.h, os_window.h); what each function leaves in eax and in the
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
nfs::u32 ddrawRelockSurface(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 ddrawMakeSurface(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 ddrawFreeSurface(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 ddrawBackSurface(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 ddrawSurfaceSize(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 ddrawNoteChange(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 singleInstanceCheck(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memClassNoPurge(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memClassCreateRam(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 fileDeviceStart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 fileOpFind(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 fileOpStatus(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 fileOpComplete(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 fileOpWait(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 fileLockEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 fileLockLeave(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 videoWindowMake(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 videoWindowUpdate(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 videoWindowFree(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 eaTextStart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 eaTextEnd(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 keyboardIsPc98(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 debugKeyDisplayFlag(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 debugKeyScreenshot(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 debugKeysBind(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 eaSetLoadHook(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 videoBuffersStart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 fontFindGlyph(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 fontGlyph(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 fontBitsPerPixel(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 fontCreate(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 fontSetProperty(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 fontGetProperty(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 paletteFrom6Bits(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 paletteFrom8Bits(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 debugBoxCopyScreen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 debugBoxPutBack(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 debugBoxAsk(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 debugBanner(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 debugKeyPause(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 debugKeyViewerFirst(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 debugKeyViewerSecond(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 textBackgroundBox(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 longDoubleKind(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 fdivBugCheck(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 doubleDivide(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 doubleSubtract(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 doubleMultiply(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 doubleCompare(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 safeDivide(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 safeDivideCall(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 doubleProductFromX87(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
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
using nfs::Float;
using nfs::u32;
using nfs::s32;

constexpr std::uint8_t kBx = kRestoreEbx, kCx = kRestoreEcx, kDx = kRestoreEdx, kSi = kRestoreEsi, kDi = kRestoreEdi;
constexpr std::uint8_t kAllSaved = kBx | kCx | kDx | kSi | kDi;

// The game's own functions.
constexpr u32 kError = 0x401010, kFillZero = 0x4e070c, kMemset = 0x4e0640;
constexpr u32 kReport = 0x501050;
constexpr u32 kCloseHandleSlot = 0x4e3fe0;

// The error's place, read by Error when it says it.
constexpr u32 kErrorFile = 0x552190, kErrorFunction = 0x552194, kErrorLine = 0x552198;

// The DirectDraw report's channel and its lock.
constexpr u32 kReportChannel = 0x564ed4, kDrawLock = 0x564480;
// A display's record.
constexpr u32 kDisplayObject = 0x00, kDisplayLocked = 0x43c, kDisplayLockedBits = 0x440;
constexpr u32 kMainDisplay = 0x9f38d0;

/* A function's stack word, counted from the first above the return address. */
u32 stackWord(nfs::App* app, const nfs::ThreadState& ts, u32 index)
{
    return app->getMemory<u32>(ts.sp + 4 + 4 * index);
}

/* Error, its place set first. */
void failAt(Guest& guest, u32 file, u32 function, u32 line, std::initializer_list<u32> formatAndValues)
{
    guest.setDword(kErrorFile, file);
    guest.setDword(kErrorFunction, function);
    guest.setDword(kErrorLine, line);
    // Error's format and the words after it, as the game's code pushes them.
    const u32 format = *formatAndValues.begin();
    guest::Words words({});
    for (const u32* value = formatAndValues.begin() + 1; value != formatAndValues.end(); ++value)
        words.word[words.count++] = *value;
    fe::direct<kError, ea::error>(guest.app(), guest.ts(), format, words);
}

/* A line of the DirectDraw report: its level, its format, what it says. */
void report(Guest& guest, std::initializer_list<u32> levelFormatAndValues)
{
        const u32 level = *levelFormatAndValues.begin(), format = *(levelFormatAndValues.begin() + 1);
    guest::Words words({});
    for (const u32* value = levelFormatAndValues.begin() + 2; value != levelFormatAndValues.end(); ++value)
        words.word[words.count++] = *value;
    fe::direct<kReport, ea::reportedAt>(guest.app(), guest.ts(), kReportChannel, level, format, words);
}

}  // namespace

/* ---------------------------------------------------------------------------
 * DirectDraw
 * ------------------------------------------------------------------------- */

/* sub_4ed330: the display's surface given up and locked again, so that other threads get
 * the lock for a moment; 1 when it is locked where it was, 0 when not, which forgets the
 * surface and draws into the bitmap at 0x5643f0 instead.  How long it was held goes to
 * 0x564ee0, the longest to 0x564ee4. */
u32 frontend::menz::ddrawRelockSurface(App* app, ThreadState& ts, u32 display)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x84);
    constexpr u32 kHeld = 0x564ee0, kHeldLongest = 0x564ee4, kLockedAt = 0x564eec;
    const u32 surface = g.dword(display + kDisplayLocked);
    if (surface == 0)
        return 1;
    os::video::unlock(app, ts, surface);
    const u32 held = os::time::ticks(g.app(), ts) - g.dword(kLockedAt);
    g.setDword(kHeld, held);
    if (s32(held) > s32(g.dword(kHeldLongest)))
        g.setDword(kHeldLongest, held);
    fe::leaveLock(g, ts, g.dword(kDrawLock));
    fe::enterLock(g, ts, g.dword(kDrawLock));
    g.setDword(kLockedAt, os::time::ticks(g.app(), ts));
    os::video::Locked locked;
    if (guest::direct<0x4eb5e0, eaa::ddrawLockSurface>(app, ts, display, surface, &locked) == 0)
    {
        if (locked.pixels == g.dword(display + kDisplayLockedBits))
        {
            g.setDword(display + kDisplayLocked, surface);
            return 1;
        }
        os::video::unlock(app, ts, surface);
    }
    g.setDword(display + kDisplayLockedBits, 0);
    g.setDword(display + kDisplayLocked, 0);
    g.setDword(kHeld, 0);
    fe::leaveLock(g, ts, g.dword(kDrawLock));
    guest::direct<0x4eada0, eaa::bitmapDrawInto>(app, ts, g.dword(0x5643f0));
    report(g, { 3, 0x54c0bc });
    return 0;
}

nfs::u32 frontend::ddrawRelockSurface(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4ed330", 0, 0, nullptr, 0, kAllSaved };
    const u32 display = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::ddrawRelockSurface(app, ts, display); });
}

/* sub_4ed480: makesurface: a DirectDraw surface; eax the display, edx and ebx its size, ecx
 * and the stack: back buffers, the pixel format wanted (0 the primary's, above that not
 * written), the caps, a palette for it.  A surface that is not the primary and not in video
 * memory is made of the size asked.  It is locked once, to see that it can be: failing that,
 * unless the display says to keep it, it is released. */
u32 frontend::menz::ddrawMakeSurface(App* app, ThreadState& ts, u32 display, u32 width, u32 height, u32 pages,
                                       u32 backBuffers, u32 formatWanted, u32 capsAsked, u32 palette)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x88);
    constexpr u32 kCapsPrimary = 0x200, kCapsVideo = 0x400;
    /* What is asked -- back buffers, the size, the pixel format, the caps -- the port's display
     * does not need: it has the one picture it shows (os::video::makeSurface).  A pixel format above
     * 0 for a surface of its own size is one the library cannot write. */
    if ((capsAsked & (kCapsPrimary | kCapsVideo)) == 0 && s32(formatWanted) > 0)
        failAt(g, 0x54bb1c, 0x54c0d4, 0x458, { 0x54c0e0 });
    report(g, { 0xb, 0x54c120, width, height, pages, backBuffers + 1 });
    const u32 made = os::video::makeSurface(app, g.dword(display + kDisplayObject));
    report(g, { 0xf, 0x54c14c });
    // Locked once, to see that it can be: the port's surfaces always lock.
    fe::enterLock(g, ts, g.dword(kDrawLock));
    os::video::Locked locked;
    guest::direct<0x4eb5e0, eaa::ddrawLockSurface>(app, ts, 0u, made, &locked);
    os::video::unlock(app, ts, made);
    fe::leaveLock(g, ts, g.dword(kDrawLock));
    /* Its colour key (at 8 bits a pixel the palette's last entry) and the palette given it: the
     * port's surfaces have no key or palette of their own -- they show the display's palette. */
    NFS2_USE(palette);
    return made;
}

nfs::u32 frontend::ddrawMakeSurface(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4ed480", 0x10, 0, nullptr, 0, kSi | kDi };
    const u32 display = args.eax;
    const u32 width = args.edx;
    const u32 height = args.ebx;
    const u32 pages = args.ecx;
    const u32 backBuffers = stackWord(app, ts, 0);
    const u32 formatWanted = stackWord(app, ts, 1);
    const u32 capsAsked = stackWord(app, ts, 2);
    const u32 palette = stackWord(app, ts, 3);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return menz::ddrawMakeSurface(app, ts, display, width, height, pages, backBuffers, formatWanted, capsAsked, palette);
    });
}

/* sub_4ed730: a surface released (edx), unlocked first if it is the one the display (eax,
 * or the main one) holds locked. */
u32 frontend::menz::ddrawFreeSurface(App* app, ThreadState& ts, u32 display, u32 surface)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 4);
    if (display == 0)
        display = kMainDisplay;
    if (surface == g.dword(display + kDisplayLocked))
        guest::direct<0x4ed2a0, eab::ddrawUnlockBitmap>(app, ts, display);
    // The references left: none, the surface is gone.
    os::video::release(app, surface);
    return 0;
}

nfs::u32 frontend::ddrawFreeSurface(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4ed730", 0, 0, nullptr, 0, kCx };
    const u32 display = args.eax, surface = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::ddrawFreeSurface(app, ts, display, surface); });
}

/* sub_4ed770: the surface attached to one (eax) as its back buffer. */
u32 frontend::menz::ddrawBackSurface(App* app, ThreadState& ts, u32 surface)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x10);
    return os::video::backBuffer(app, surface);
}

nfs::u32 frontend::ddrawBackSurface(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4ed770", 0, 0, nullptr, 0, kCx | kDx };
    const u32 surface = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::ddrawBackSurface(app, ts, surface); });
}

/* sub_4ed820: a surface's (eax) width, height, bits a pixel, pitch and video memory's
 * address, each to where its pointer says if it has one.  The port's surfaces always tell:
 * they are laid out as the display's mode, and where the pixels are is told only by a lock (0
 * here).  What the last one written was, as the code leaves it in eax. */
u32 frontend::menz::ddrawSurfaceSize(App* app, ThreadState& ts, u32 surface, u32 widthOut, u32 heightOut,
                                       u32 bitsOut, u32 pitchOut, u32 memoryOut)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x78);
    NFS2_USE(surface);
    const os::video::Format format = os::video::mode();
    u32 result = 0;
    if (widthOut != 0)
        g.setDword(widthOut, result = format.width);
    if (heightOut != 0)
        g.setDword(heightOut, result = format.height);
    if (pitchOut != 0)
        g.setDword(pitchOut, result = u32(format.pitch));
    if (memoryOut != 0)
        g.setDword(memoryOut, result = 0);
    if (bitsOut != 0)
    {
        if (format.bits == 0x10)
        {
            u32 bits = guest::direct<0x4eb880, eaa::countBits>(app, ts, format.redMask);
            bits += guest::direct<0x4eb880, eaa::countBits>(app, ts, format.greenMask);
            result = guest::direct<0x4eb880, eaa::countBits>(app, ts, format.blueMask);
            g.setDword(bitsOut, bits + result);
        }
        else
        {
            result = format.bits & 0xff;
            g.setDword(bitsOut, result);
        }
    }
    return result;
}

nfs::u32 frontend::ddrawSurfaceSize(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4ed820", 8, 0, nullptr, 0, kSi | kDi };
    const u32 surface = args.eax;
    const u32 widthOut = args.edx;
    const u32 heightOut = args.ebx;
    const u32 bitsOut = args.ecx;
    const u32 pitchOut = stackWord(app, ts, 0);
    const u32 memoryOut = stackWord(app, ts, 1);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return menz::ddrawSurfaceSize(app, ts, surface, widthOut, heightOut, bitsOut, pitchOut, memoryOut);
    });
}

/* sub_4edbc0: the display's modes' callback: which depths the card has (0x9f09a4), the mode
 * noted in the list of them (`list`, the display) while it has room, and reported; 0 ends the
 * modes, when the card has said more modes than it first counted.  A mode of the port's tells
 * no four-character code, caps or back buffers: none, a mode not of Mode X, one back buffer
 * counted with the front. */
u32 frontend::menz::ddrawEnumModeCallback(App* app, ThreadState& ts, const os::video::Format& mode, u32 list)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x18);
    constexpr u32 kModesSeen = 0x9f09a0, kDepths = 0x9f09a4;
    constexpr u32 kListCheck = 0x10, kListRoom = 0x08;
    u32 bits;
    if (mode.bits == 0x10)
    {
        bits = guest::direct<0x4eb880, eaa::countBits>(app, ts, mode.redMask);
        bits += guest::direct<0x4eb880, eaa::countBits>(app, ts, mode.greenMask);
        bits += guest::direct<0x4eb880, eaa::countBits>(app, ts, mode.blueMask);
    }
    else
    {
        bits = mode.bits & 0xff;
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
        if (s32(g.dword(kModesSeen)) >= s32(g.dword(list + kListRoom)))
        {
            report(g, { 1, 0x54c1ec });
            return 0;
        }
        guest::direct<0x4eb8a0, eaa::ddrawNoteMode>(app, ts, list, g.dword(kModesSeen), mode, 0u);
        constexpr u32 kFourCC = 0, kNotFullscreenOnly = 0x44, kNotModeX = 0x4e, kBuffers = 1;
        report(g, { 8, 0x54c1b4, 0x20, mode.width, mode.height, mode.bits, bits, u32(mode.pitch), kBuffers, kNotModeX,
                    kNotFullscreenOnly, kFourCC });
    }
    g.setDword(kModesSeen, g.dword(kModesSeen) + 1);
    return 1;
}

/* sub_4f9060: one more change to what the display shows, counted at 0x56717c. */
u32 frontend::menz::ddrawNoteChange(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    g.setDword(0x56717c, g.dword(0x56717c) + 1);
    return 0;
}

nfs::u32 frontend::ddrawNoteChange(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f9060", 0, 0, nullptr, 0, 0 };
    // eax as it came: the code only counts.
    const nfs::u32 eax = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { menz::ddrawNoteChange(app, ts); return eax; });
}

/* ---------------------------------------------------------------------------
 * Start-up
 * ------------------------------------------------------------------------- */

/* sub_4f91d0: the mutex by the window class's name (eax, or the name kept, or the program's
 * first argument).  When it existed already the other instance's window is brought up and
 * this one leaves, or says 1 when 0x567188 is clear. */
u32 frontend::menz::singleInstanceCheck(App* app, ThreadState& ts, u32 name)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0xc);
    constexpr u32 kClassName = 0x5612a4, kMutex = 0x9f3d40, kProgramName = 0x9f38c8, kMustLeave = 0x567188;
    if (g.dword(kClassName) == 0)
        g.setDword(kClassName, name != 0 ? name : g.dword(g.dword(kProgramName)));
    if (name == 0)
        name = g.dword(kClassName);
    const std::optional<os::Handle> mutex =
        os::locks::named(app, ts, name != 0 ? &app->getMemory<const char>(name) : nullptr, true);
    if (!mutex)
        throw fe::Terminated();
    g.setDword(kMutex, *mutex);
    // Another copy shows by the mutex being there already, as the system's last error would say:
    // the port keeps no error, so there is none.
    constexpr bool kAlreadyThere = false;
    if (!kAlreadyThere)
        return 0;
    // The other copy's window brought up: the port runs one game, there is no other window.
    if (g.dword(kMustLeave) == 0)
        return 1;
    guest::direct<0x4f2a7d, eab::exitGameNow>(app, ts);
    return 0;
}

nfs::u32 frontend::singleInstanceCheck(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f91d0", 0, 0, nullptr, 0, kBx | kCx | kDx };
    const u32 name = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::singleInstanceCheck(app, ts, name); });
}

/* sub_4f9260: the RAM class's purge callback: nothing can be purged. */
u32 frontend::menz::memClassNoPurge(App*, ThreadState&)
{
    return 0;
}

nfs::u32 frontend::memClassNoPurge(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f9260", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::memClassNoPurge(app, ts); });
}

/* sub_4f9270: the memory class "RAM" (eax its base, edx its size). */
u32 frontend::menz::memClassCreateRam(App* app, ThreadState& ts, u32 base, u32 size)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 8);
    constexpr u32 kCreateMemClass = 0x4e1230, kPools = 0x9ef5c0;
    const u32 result = callAt(g, kCreateMemClass, args({ 0, 0x54d92c, base, size }, { 8, 0x20, 0x10, 1, 1, 1, 1, 0x4f9260 }));
    const u32 pools = g.dword(kPools);
    g.setDword(kPools + 4, pools);
    return result;
}

nfs::u32 frontend::memClassCreateRam(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f9270", 0, 0, nullptr, 0, kBx | kCx };
    const u32 base = args.eax, size = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::memClassCreateRam(app, ts, base, size); });
}

/* sub_517b30: the text library's state cleared, once, and its end (sub_517b70) to run at exit. */
u32 frontend::menz::eaTextStart(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 8);
    constexpr u32 kStarted = 0x568f30, kState = 0x5671f4, kStateBytes = 0xb8;
    if (g.dword(kStarted) != 0)
        return 0;  // eax as it came in the game's code, which no caller reads
    fe::direct<kFillZero, ea::clearBytes>(g.app(), g.ts(), kState, kStateBytes);
    g.setDword(kStarted, 1);
    return guest::direct<0x4f2a78, eab::atexitJump>(app, ts, 0x517b70u);
}

nfs::u32 frontend::eaTextStart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_517b30", 0, 0, nullptr, 0, kCx | kDx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::eaTextStart(app, ts); });
}

/* sub_517b70: the text library's end. */
u32 frontend::menz::eaTextEnd(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 8);
    constexpr u32 kStarted = 0x568f30, kState = 0x5671f4, kStateBytes = 0xb8;
    if (g.dword(kStarted) == 0)
        return 0;  // eax as it came in the game's code, which no caller reads
    fe::direct<0x50c1e0, watb::freeTextDraw>(app, ts);
        const u32 filled = fe::direct<kFillZero, ea::clearBytes>(g.app(), g.ts(), kState, kStateBytes);
    g.setDword(kStarted, 0);
    return filled;
}

nfs::u32 frontend::eaTextEnd(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_517b70", 0, 0, nullptr, 0, kCx | kDx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::eaTextEnd(app, ts); });
}

/* sub_517ba0: 1 and the NEC PC-98's scan codes when the keyboard is one (Japanese, subtype
 * 0xd01 to 0xd04). */
u32 frontend::menz::keyboardIsPc98(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0xc);
    constexpr u32 kJapanese = 7, kScanTable = 0x561298, kPc98Scans = 0x568f34;
    // The port's keyboard is of no type and subtype the system names (0), so never the PC-98's.
    constexpr u32 type = 0;
    constexpr s32 subtype = 0;
    if (type != kJapanese || subtype < 0xd01 || subtype > 0xd04)
        return 0;
    g.setDword(kScanTable, kPc98Scans);
    return 1;
}

nfs::u32 frontend::keyboardIsPc98(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_517ba0", 0, 0, nullptr, 0, kBx | kCx | kDx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::keyboardIsPc98(app, ts); });
}

/* sub_518040: the load's hook (see native_ea_support.cpp) set to sub_517f10. */
u32 frontend::menz::eaSetLoadHook(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    g.setDword(0x567868, 0x517f10);
    return 0;  // eax as it came in the game's code, which no caller reads
}

nfs::u32 frontend::eaSetLoadHook(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_518040", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::eaSetLoadHook(app, ts); });
}

/* sub_518fd0: the video buffers' table, once: eax of them, 400 by default, 32 bytes each,
 * and the list of their numbers. */
u32 frontend::menz::videoBuffersStart(App* app, ThreadState& ts, u32 asked)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x10);
    constexpr u32 kDriver = 0x56718c, kAllocate = 0x14;
    constexpr u32 kCount = 0x569054, kRecords = 0x569058, kFreeList = 0x56905c, kUsed = 0x569060;
    u32 result = asked;
    if (g.dword(kRecords) != 0)
        return result;
    const u32 count = result != 0 ? result : 400;
    g.setDword(kCount, count);
    u32 driver = g.dword(kDriver);
    g.setDword(kRecords, callAt(g, g.dword(driver + kAllocate), args({ count << 5, driver, count })));
    const u32 total = g.dword(kCount);
    driver = g.dword(kDriver);
    const u32 list = callAt(g, g.dword(driver + kAllocate), args({ total * 4, driver, total, 0 }));
    const u32 lastCount = g.dword(kCount);
    g.setDword(kUsed, 0);  // ecx as the allocation (sub_518f80 through the driver's pointer, which keeps ecx) was given it
    result = 0;
    for (; s32(result) < s32(lastCount); ++result)
        g.setDword(list + result * 4, result);
    g.setDword(kFreeList, list);
    g.setDword(kCount, lastCount);
    return result;
}

nfs::u32 frontend::videoBuffersStart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_518fd0", 0, 0, nullptr, 0, kBx | kCx | kDx | kSi };
    const u32 count = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::videoBuffersStart(app, ts, count); });
}

/* ---------------------------------------------------------------------------
 * The debug keys
 * ------------------------------------------------------------------------- */

/* sub_517d70: key 0x17: the display's flag at 0x564390 flipped and the mode set again. */
u32 frontend::menz::debugKeyDisplayFlag(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 8);
    g.setByte(0x564390, g.byte(0x564390) ^ 1);
    return fe::direct<0x5089d0, rendf::setDisplayMode>(app, ts, g.dword(0x564384), g.dword(0x564388), g.dword(0x56438c));
}

nfs::u32 frontend::debugKeyDisplayFlag(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_517d70", 0, 0, nullptr, 0, kBx | kDx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::debugKeyDisplayFlag(app, ts); });
}

/* sub_517da0: key 0x16: the screen saved as snap.tga, and a banner that says so. */
u32 frontend::menz::debugKeyScreenshot(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x74);
    fe::direct<0x4f46d0, wata::windowSave>(app, ts, g.frame());
    fe::direct<0x4ead20, wata::windowApply>(app, ts);
    // The game's code hands sub_4eb160 a 4 in edx too, which it keeps: the banner's time.
    guest::direct<0x4eb160, eaa::bitmapSaveTga>(app, ts, 0x550698u);
    guest::direct<0x524c30, debugBanner>(app, ts, 0x5506a4u, 4u);
    return fe::direct<0x4f4700, wata::windowRestore>(app, ts, g.frame());
}

nfs::u32 frontend::debugKeyScreenshot(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_517da0", 0, 0, nullptr, 0, kDx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::debugKeyScreenshot(app, ts); });
}

/* sub_517de0: the debug keys bound, Ctrl+Escape set to leave. */
u32 frontend::menz::debugKeysBind(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 4);
    g.setDword(0x564810, 1);
    fe::direct<0x50c5d4, watb::registerKey>(app, ts, 4u, 0x524dd0u);
    fe::direct<0x50c5d4, watb::registerKey>(app, ts, 5u, 0x524de0u);
    fe::direct<0x50c5d4, watb::registerKey>(app, ts, 0x10u, 0x524d90u);
    fe::direct<0x50c5d4, watb::registerKey>(app, ts, 0x18u, 0x524bb0u);
    fe::direct<0x50c5d4, watb::registerKey>(app, ts, 0x16u, 0x517da0u);
    return fe::direct<0x50c5d4, watb::registerKey>(app, ts, 0x17u, 0x517d70u);
}

nfs::u32 frontend::debugKeysBind(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_517de0", 0, 0, nullptr, 0, kDx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::debugKeysBind(app, ts); });
}

/* sub_524930: what is on the screen copied into the bitmap (eax): edx, ebx where. */
u32 frontend::menz::debugBoxCopyScreen(App* app, ThreadState& ts, u32 bitmap, u32 x, u32 y)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 4);
    fe::direct<0x4ead20, wata::windowApply>(app, ts);
    return guest::direct<0x4f2cb0, eab::shapeMoveWithSpot>(app, ts, g.dword(bitmap + 0x20), x, y);
}

nfs::u32 frontend::debugBoxCopyScreen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_524930", 0, 0, nullptr, 0, kCx };
    const u32 bitmap = args.eax, x = args.edx, y = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::debugBoxCopyScreen(app, ts, bitmap, x, y); });
}

/* sub_524950: the bitmap (eax) back on the screen: edx, ebx where. */
u32 frontend::menz::debugBoxPutBack(App* app, ThreadState& ts, u32 bitmap, u32 x, u32 y)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 4);
    fe::direct<0x4ead20, wata::windowApply>(app, ts);
    // sub_4f09c0 hands the registers as they are on to the blitter it chose: the shape, x, y.
    fe::direct<0x4f09c0, wata::blitDispatchAt>(app, ts, g.dword(bitmap + 0x20), x, y);
    return guest::direct<0x4ead90, eaa::bitmapDrawToScreen>(app, ts);
}

nfs::u32 frontend::debugBoxPutBack(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_524950", 0, 0, nullptr, 0, kCx };
    const u32 bitmap = args.eax, x = args.edx, y = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::debugBoxPutBack(app, ts, bitmap, x, y); });
}

/* sub_524970: a box of text (eax) at edx, ebx, ecx wide and the first stack argument high,
 * what it covers kept and put back; waits for the mouse's buttons (the second argument's
 * mask) to come up and then for a key or a button.  The answer: 1 for the first button or Y. */
u32 frontend::menz::debugBoxAsk(App* app, ThreadState& ts, u32 text, u32 x, u32 y, u32 width, u32 height,
                                  u32 mask)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x150);
    constexpr u32 kView = 0x56502c;
    constexpr u32 kDriverBegin = 0x564460, kDriverEnd = 0x564464, kDriverBeginCopy = 0x56445c, kButtons = 0x564468;
    const u32 f = g.frame();
    // sub_4feffc answers 0 to what the game's code hands it: 0xffffffff, then 0 and the height (which edx keeps for
    // the bitmap), at the end its first answer.  The registers the calls below take in the game's code as the calls
    // before left them (the height in ecx, the box's right and bottom in ebx and ecx, white's ink in eax) handed.
    fe::direct<0x4feffc, watb::returnZero>(app, ts);
    fe::direct<0x4feffc, watb::returnZero>(app, ts);
    const u32 bitmap = guest::direct<0x4ef190, eab::windowBitmapCreateDefault>(app, ts, width, height,
                                                                                g.dword(0x56438c), g.dword(0x5643f4));
    fe::direct<0x4f46d0, wata::windowSave>(app, ts, f + 0xb8);
    fe::direct<0x4f4740, wata::textStateSave>(app, ts, f);
    guest::direct<0x4ef520, eab::fontUse>(app, ts, 0x5662e4u);
    const u32 bottom = height + y, right = x + width;
    fe::direct<0x52b6c0, watc::setRectClip>(app, ts, kView, x, y, right, bottom);
    callAt(g, g.dword(kDriverBegin));
    guest::direct<0x4eada0, eaa::bitmapDrawInto>(app, ts, bitmap);  // x in edx too, not read
    guest::direct<0x524930, debugBoxCopyScreen>(app, ts, bitmap, x, y);  // the height in ecx too, not read
    fe::direct<0x4ead20, wata::windowApply>(app, ts);
    const u32 black = guest::direct<0x4ef760, eab::colourPack>(app, ts, 0xff000000u);
    fe::direct<0x4fd540, watb::drawFilledRect>(app, ts, x, y, width, height, black);
    const u32 red = guest::direct<0x4ef760, eab::colourPack>(app, ts, 0xffaa0000u);
    fe::direct<0x52b720, watc::drawBox>(app, ts, x + 4, y + 4, right - 5, bottom - 4, red);
    const u32 white = guest::direct<0x4ef760, eab::colourPack>(app, ts, 0xffffffffu);
    guest::direct<0x4ef830, eab::fontSetColours>(app, ts, white, 0u);
    fe::direct<0x52b860, watc::textCentred>(app, ts, text, y + 9);
    callAt(g, g.dword(kDriverEnd));
    fe::direct<0x52b8b0, watc::clipWholeRect>(app, ts, kView);
    guest::direct<0x4ead90, eaa::bitmapDrawToScreen>(app, ts);
    while ((callAt(g, g.dword(kButtons)) & mask) != 0)
    {
    }
    u32 answer, key;
    for (;;)
    {
        key = guest::direct<0x4f00d0, eab::keyNextLow16>(app, ts);
        answer = callAt(g, g.dword(kButtons)) & mask;
        if (key != 0 || answer != 0)
            break;
    }
    answer &= 1;
    if ((key | 0x20) == 0x79)
        answer = 1;
    fe::direct<0x52b6c0, watc::setRectClip>(app, ts, kView, x, y, right, y + height);
    callAt(g, g.dword(kDriverBeginCopy));
    guest::direct<0x524950, debugBoxPutBack>(app, ts, bitmap, x, y);
    callAt(g, g.dword(kDriverEnd));
    fe::direct<0x4f4760, wata::textStateRestore>(app, ts, f);
    fe::direct<0x4f4700, wata::windowRestore>(app, ts, f + 0xb8);
    guest::direct<0x4eff00, eab::windowBitmapFree>(app, ts, bitmap);
    fe::direct<0x4feffc, watb::returnZero>(app, ts);
    return answer;
}

nfs::u32 frontend::debugBoxAsk(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_524970", 8, 0, nullptr, 0, kSi | kDi };
    const u32 text = args.eax;
    const u32 x = args.edx;
    const u32 y = args.ebx;
    const u32 width = args.ecx;
    const u32 height = stackWord(app, ts, 0);
    const u32 mask = stackWord(app, ts, 1);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::debugBoxAsk(app, ts, text, x, y, width, height, mask); });
}

/* sub_524c30: a line of text (eax) along the foot of the screen for edx of time, what it
 * covers put back after. */
u32 frontend::menz::debugBanner(App* app, ThreadState& ts, u32 text, u32 time)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x140);
    constexpr u32 kView = 0x56502c;
    constexpr u32 kDriverBegin = 0x564460, kDriverEnd = 0x564464, kDriverBeginCopy = 0x56445c;
    constexpr u32 kScreenWidth = 0x564384, kScreenHeight = 0x564388, kBannerHeight = 0xa;
    const u32 f = g.frame();
    // As in sub_524970: sub_4feffc answers 0 (the banner's height handed it in edx, which keeps it for the bitmap);
    // what the calls take in registers the calls before left handed.
    fe::direct<0x4feffc, watb::returnZero>(app, ts);
    fe::direct<0x4feffc, watb::returnZero>(app, ts);
    const u32 bitmap = guest::direct<0x4ef190, eab::windowBitmapCreateDefault>(
        app, ts, g.dword(kScreenWidth), kBannerHeight, g.dword(0x56438c), g.dword(0x5643f4));
    fe::direct<0x4f46d0, wata::windowSave>(app, ts, f + 0xb8);
    fe::direct<0x4f4740, wata::textStateSave>(app, ts, f);
    guest::direct<0x4ef520, eab::fontUse>(app, ts, 0x5662e4u);
    guest::direct<0x4eada0, eaa::bitmapDrawInto>(app, ts, bitmap);
    const u32 screenHeight = g.dword(kScreenHeight);
    fe::direct<0x52b6c0, watc::setRectClip>(app, ts, kView, 0u, screenHeight - kBannerHeight, g.dword(kScreenWidth),
                                            screenHeight);
    callAt(g, g.dword(kDriverBegin));
    guest::direct<0x524930, debugBoxCopyScreen>(app, ts, bitmap, 0, g.dword(kScreenHeight) - kBannerHeight);
    fe::direct<0x4ead20, wata::windowApply>(app, ts);
    const u32 black = guest::direct<0x4ef760, eab::colourPack>(app, ts, 0xff000000u);
    fe::direct<0x4fd540, watb::drawFilledRect>(app, ts, 0u, g.dword(kScreenHeight) - kBannerHeight,
                                               g.dword(kScreenWidth), kBannerHeight, black);
    const u32 white = guest::direct<0x4ef760, eab::colourPack>(app, ts, 0xffffffffu);
    guest::direct<0x4ef830, eab::fontSetColours>(app, ts, white, 0u);
    fe::direct<0x52b860, watc::textCentred>(app, ts, text, g.dword(kScreenHeight) - 9);
    callAt(g, g.dword(kDriverEnd));
    guest::direct<0x4ead90, eaa::bitmapDrawToScreen>(app, ts);
    guest::direct<0x4ead90, eaa::bitmapDrawToScreen>(app, ts);
    fe::direct<0x4f06e0, wata::timerWait>(app, ts, time);
    fe::direct<0x4ead20, wata::windowApply>(app, ts);
    callAt(g, g.dword(kDriverBeginCopy));
    guest::direct<0x524950, debugBoxPutBack>(app, ts, bitmap, 0u, g.dword(kScreenHeight) - kBannerHeight);
    callAt(g, g.dword(kDriverEnd));
    fe::direct<0x4f4760, wata::textStateRestore>(app, ts, f);
    fe::direct<0x4f4700, wata::windowRestore>(app, ts, f + 0xb8);
    guest::direct<0x4eff00, eab::windowBitmapFree>(app, ts, bitmap);
    return fe::direct<0x4feffc, watb::returnZero>(app, ts);
}

nfs::u32 frontend::debugBanner(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_524c30", 0, 0, nullptr, 0, kBx | kCx | kSi | kDi };
    const u32 text = args.eax, time = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::debugBanner(app, ts, text, time); });
}

/* sub_524d90: key 0x10: pause: the box in the middle of the screen until a key. */
u32 frontend::menz::debugKeyPause(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0xc);
    const u32 y = u32(s32(g.dword(0x564388)) / 2) - 8;
    const u32 x = u32(s32(g.dword(0x564384)) / 2) - 0x90;
    return guest::direct<0x524970, debugBoxAsk>(app, ts, 0x551e80, x, y, 0x120, 0x18, 1);
}

nfs::u32 frontend::debugKeyPause(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_524d90", 0, 0, nullptr, 0, kBx | kCx | kDx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::debugKeyPause(app, ts); });
}

/* sub_524dd0, sub_524de0: keys 4 and 5: the memory viewer's two ways in. */
u32 frontend::menz::debugKeyViewerFirst(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    return fe::direct<0x52b8d0, watc::viewerOnWindow>(app, ts);
}

nfs::u32 frontend::debugKeyViewerFirst(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_524dd0", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::debugKeyViewerFirst(app, ts); });
}

u32 frontend::menz::debugKeyViewerSecond(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    return fe::direct<0x52bb20, watc::viewerOnMono>(app, ts);
}

nfs::u32 frontend::debugKeyViewerSecond(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_524de0", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::debugKeyViewerSecond(app, ts); });
}

/* sub_52e440: text (eax) with a filled box behind it, at the text state's position. */
u32 frontend::menz::textBackgroundBox(App* app, ThreadState& ts, u32 text)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0xd8);
    constexpr u32 kX = 0x5671f8, kY = 0x5671fc, kColourFore = 0x567220, kColourBack = 0x567224;
    const u32 f = g.frame();
    fe::direct<0x4f4740, wata::textStateSave>(app, ts, f);
    fe::direct<0x4f0860, wata::textSetAttribute>(app, ts, 0u);
    guest::direct<0x4ef830, eab::fontSetColours>(app, ts, g.dword(kColourFore), g.dword(kColourBack));
    // The width's slot in ecx in the game's code, set before sub_4ef830, which keeps it.
    fe::direct<0x4f0c90, wata::textMeasure>(app, ts, text, f + 0xbc, f + 0xb8, f + 0xc4, f + 0xc0);
    const u32 offsetY = g.dword(f + 0xb8), offsetX = g.dword(f + 0xbc);
    fe::direct<0x4fd540, watb::drawFilledRect>(app, ts, g.dword(kX) + offsetX, g.dword(kY) + offsetY,
                                               g.dword(f + 0xc4), g.dword(f + 0xc0), g.dword(kColourBack));
    fe::direct<0x4f4760, wata::textStateRestore>(app, ts, f);
    return guest::direct<0x4efb50, eab::textDrawAtPen>(app, ts, text);
}

nfs::u32 frontend::textBackgroundBox(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_52e440", 0, 0, nullptr, 0, kBx | kCx | kDx | kSi };
    const u32 text = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::textBackgroundBox(app, ts, text); });
}

/* ---------------------------------------------------------------------------
 * The file device
 * ------------------------------------------------------------------------- */

namespace
{
constexpr u32 kFileDevices = 0x9f4f60, kFileDeviceSize = 0x68, kFileDeviceStarting = 0x9f4f64, kFileListLock = 0x9f4f74;
constexpr u32 kFileDeviceLock = 0x1c, kFileDeviceDone = 0x38, kFileCompareId = 0x4fb820;
constexpr u32 kOpFlags = 0x0c;
}

/* sub_4fb860: FILE_startdevice: the device's (eax) thread, if it has none: its two lists,
 * its events and lock, and the wait until the thread says it runs. */
u32 frontend::menz::fileDeviceStart(App* app, ThreadState& ts, u32 index)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x10);
    const u32 offset = index * kFileDeviceSize;
    const auto device = [&] { return g.dword(kFileDevices) + offset; };
    if (g.dword(device()) != 0)
        return g.dword(kFileDevices);
    ts.sp -= 8;  // the two registers it saves only here
    const u32 token = guest::direct<0x50ea20, fileLockEnter>(app, ts, kFileListLock);
    g.setDword(device() + 0x18, 0);
    fe::direct<0x50dcd0, netb::qinit>(app, ts, device() + kFileDeviceLock, 0x4fb830u, 0u);
    fe::direct<0x50dd60, netb::qinitpeer>(app, ts, device() + kFileDeviceDone, 0u, 0u, device() + kFileDeviceLock);
    const u32 event = *fe::direct<0x4dfb00, ea::newAutoEvent>(app, ts);
    g.setDword(device() + 0x54, event);
    const u32 manualEvent = *fe::direct<0x4dfc10, ea::newManualEvent>(app, ts);
    g.setDword(device() + 0x5c, manualEvent);
    const u32 lock = *fe::direct<0x4eb2e0, ea::newLock>(app, ts);
    const u32 context = device() + 4;
    g.setDword(context + 0x5c, 1);
    g.setDword(context + 0x54, lock);
    g.setDword(context + 0x60, 0xff);
    g.setDword(kFileDeviceStarting, index);
    const u32 thread = *fe::direct<0x4df7a0, ea::threadStarted>(app, ts, 0x4fbda0u, 0u, 1u, context);
    if (thread == 0)
        failAt(g, 0x54dcac, 0x54dcb8, 0xb6, { 0x54dccc });
    else if (g.dword(device()) == 0)
        fe::direct<0x4dfc60, ea::eventWaited>(app, ts, g.dword(device() + 0x5c));
    return guest::direct<0x50ea40, fileLockLeave>(app, ts, kFileListLock, token);
}

nfs::u32 frontend::fileDeviceStart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4fb860", 0, 0, nullptr, 0, kAllSaved };
    const u32 index = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::fileDeviceStart(app, ts, index); });
}

/* sub_4fbd10: an operation by its id (edx) on a device (eax): the one under way (and -1 to
 * *ebx), one queued (and 1), or a finished one (and 0). */
u32 frontend::menz::fileOpFind(App* app, ThreadState& ts, u32 device, u32 id, u32 where)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0xc);
    g.setDword(where, 0);
    const u32 current = g.dword(device + 0x18);
    if (current != 0 && id == g.dword(current + 4))
    {
        g.setDword(where, 0xffffffff);
        return current;
    }
    const u32 queued = fe::direct<0x50e3f0, netb::qfind>(app, ts, device + kFileDeviceLock, kFileCompareId, id);
    if (queued != 0)
    {
        g.setDword(where, 1);
        return queued;
    }
    return fe::direct<0x50e3f0, netb::qfind>(app, ts, device + kFileDeviceDone, kFileCompareId, id);
}

nfs::u32 frontend::fileOpFind(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4fbd10", 0, 0, nullptr, 0, kCx | kSi | kDi };
    const u32 device = args.eax, id = args.edx, where = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::fileOpFind(app, ts, device, id, where); });
}

/* sub_4fc340: an operation's state (eax its id): 0 still going, -1 it failed, -3 no such,
 * or its result's byte. */
u32 frontend::menz::fileOpStatus(App* app, ThreadState& ts, u32 id)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x14);
    const u32 device = g.dword(kFileDevices) + (id & 0x1f) * kFileDeviceSize;
    const u32 token = g.dword(device) != 0 ? guest::direct<0x50ea20, fileLockEnter>(app, ts, device + kFileDeviceLock) : 0;
    const u32 op = guest::direct<0x4fbd10, fileOpFind>(app, ts, device, id, g.frame());
    u32 status;
    if (g.dword(g.frame()) != 0)
        status = 0;
    else if (op == 0)
        status = 0xfffffffd;
    else if (g.dword(op + kOpFlags) & 2)
        status = 0xffffffff;
    else
        status = u32(s32(std::int8_t(g.byte(op + 0x10))));
    if (g.dword(device) != 0)
        guest::direct<0x50ea40, fileLockLeave>(app, ts, device + kFileDeviceLock, token);
    return status;
}

nfs::u32 frontend::fileOpStatus(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4fc340", 0, 0, nullptr, 0, kBx | kCx | kDx | kSi };
    const u32 id = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::fileOpStatus(app, ts, id); });
}

/* sub_4fc600: FILE_completeop: a finished operation (eax its id) taken off its device's list
 * and its record given back; what it answers by its kind, the handle closed or the memory
 * freed where the operation made them for the caller. */
u32 frontend::menz::fileOpComplete(App* app, ThreadState& ts, u32 id)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x14);
    constexpr u32 kOpKind = 0x08, kOpSucceeded = 0x10, kOpHandle = 0x18, kOpCount = 0x24, kOpPosition = 0x28, kOpBlock = 0x2c;
    constexpr u32 kFreeBlocks = 0x9f4f90;
    const u32 device = g.dword(kFileDevices) + (id & 0x1f) * kFileDeviceSize;
    u32 ecx = 0;
    u32 op = 0;
    if (device != 0)
        op = fe::direct<0x50e5a0, watb::queueFetch>(app, ts, device + kFileDeviceDone, kFileCompareId, id);
    if (op == 0)
        failAt(g, 0x54dcac, 0x54deec, 0x3ad, { 0x54defc });
    switch (g.dword(op + kOpKind))
    {
    case 0:
        if ((g.dword(op + kOpFlags) & 2) != 0 && g.dword(op + kOpHandle) != 0)
                        fe::direct<kCloseHandleSlot, ea::slotClosed>(app, ts, g.dword(op + kOpHandle));
        else
            ecx = g.dword(op + kOpHandle);
        break;
    case 1:
                ecx = *fe::direct<kCloseHandleSlot, ea::slotClosed>(app, ts, g.dword(op + kOpHandle));
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
        const u32 block = g.dword(op + kOpBlock);
        if (block == 0)
            break;
        if ((g.dword(op + kOpFlags) & 2) == 0 && g.byte(op + kOpSucceeded) == 1)
        {
            fe::direct<0x50ded0, netb::qbottom>(app, ts, kFreeBlocks, block);
            ecx = g.dword(block + 4);
        }
        else
        {
            if (g.dword(block + 0xc) != 0)
                fe::direct<kCloseHandleSlot, ea::slotClosed>(app, ts, g.dword(block + 0xc));
            fe::direct<0x4e1890, ea::purgeBlock>(app, ts, block);
        }
        break;
    }
    case 10:
    {
        const u32 block = g.dword(op + kOpBlock);
        if (block == 0)
            break;
        fe::direct<kCloseHandleSlot, ea::slotClosed>(app, ts, g.dword(block + 0xc));
        ecx = 1;
        fe::direct<0x4e1890, ea::purgeBlock>(app, ts, block);
        break;
    }
    default:  // 5, 7 and past the table
        ecx = g.byte(op + kOpSucceeded) == 1 ? 1 : 0;
        break;
    }
    if (g.dword(op + kOpFlags) & 1)
        fe::direct<0x501180, ea::pagesFreed>(app, ts, g.dword(op + kOpBlock));
    g.setDword(op + 4, 0);
    fe::direct<0x50ded0, netb::qbottom>(app, ts, kFileListLock, op);
    return ecx;
}

nfs::u32 frontend::fileOpComplete(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4fc600", 0, 0, nullptr, 0, kBx | kCx | kDx | kSi };
    const u32 id = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::fileOpComplete(app, ts, id); });
}

/* sub_4fc780: waits until an operation (eax its id) has finished, on the device's thread
 * running the system's tasks; its state, as sub_4fc340 gives it; -3 for no such. */
u32 frontend::menz::fileOpWait(App* app, ThreadState& ts, u32 id)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x10);
    const u32 offset = (id & 0x1f) * kFileDeviceSize;
    const u32 device = g.dword(kFileDevices) + offset;
    if (id == 0 || g.dword(device) == 0)
        return 0xfffffffd;
    const u32 found = g.frame();
    ts.sp -= 0x10;  // the four registers it saves only here
    do
    {
        const u32 token = g.dword(device) != 0 ? guest::direct<0x50ea20, fileLockEnter>(app, ts, device + kFileDeviceLock) : 0;
        const u32 op = guest::direct<0x4fbd10, fileOpFind>(app, ts, device, id, found);
        g.setDword(found, g.dword(found) & (op != 0 ? 1 : 0));
        if (g.dword(device) != 0)
            guest::direct<0x50ea40, fileLockLeave>(app, ts, device + kFileDeviceLock, token);
        if (g.dword(found) != 0)
        {
            if (*fe::direct<0x4df930, ea::isThreadNow>(app, ts, 0))
            {
                fe::direct<0x4e7630, port::stream::runDueTasks>(app, ts, ts.sp - 4, 0);
                fe::direct<0x4df8e0, ea::sleepFor>(app, ts, 1);
            }
            else
            {
                fe::direct<0x4dfc60, ea::eventWaited>(app, ts, g.dword(g.dword(kFileDevices) + offset + 0x5c));
            }
        }
    } while (g.dword(found) != 0);
    return guest::direct<0x4fc340, fileOpStatus>(app, ts, id);
}

nfs::u32 frontend::fileOpWait(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4fc780", 0, 0, nullptr, 0, kAllSaved };
    const u32 id = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::fileOpWait(app, ts, id); });
}

/* sub_50ea20: the critical section of a lock (eax) entered; 0, the token sub_50ea40 is given. */
u32 frontend::menz::fileLockEnter(App* app, ThreadState& ts, u32 lock)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 8);
    fe::enterLock(g, ts, g.dword(lock + 0x18));
    return 0;
}

nfs::u32 frontend::fileLockEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_50ea20", 0, 0, nullptr, 0, kCx | kDx };
    const u32 lock = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::fileLockEnter(app, ts, lock); });
}

/* sub_50ea40: and left. */
u32 frontend::menz::fileLockLeave(App* app, ThreadState& ts, u32 lock, u32 /*token, not read*/)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 4);
    return fe::leaveLock(g, ts, g.dword(lock + 0x18));
}

nfs::u32 frontend::fileLockLeave(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_50ea40", 0, 0, nullptr, 0, kCx };
    const u32 lock = args.eax, token = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::fileLockLeave(app, ts, lock, token); });
}

/* ---------------------------------------------------------------------------
 * Video windows
 * ------------------------------------------------------------------------- */

namespace
{
constexpr u32 kWindowFile = 0x54f53c, kWindowOne = 0x56502c, kWindowTwo = 0x565064;
}

/* sub_508c70: makevideowindow: a window of eax by edx, in one of the two fixed records
 * (where -1 or -2 is the first stack argument) or a record of its own; 0 for a size past
 * 2400x1600 or no memory. */
u32 frontend::menz::videoWindowMake(App* app, ThreadState& ts, u32 width, u32 height, u32 bits, u32 passedOn,
                                      u32 where, u32 passedOn2, u32 passedOn3)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x10);
    constexpr u32 kFunction = 0x54f594, kMaxWidth = 0x960, kMaxHeight = 0x640, kRecordSize = 0x38;
    if (s32(width) < 0 || s32(width) > s32(kMaxWidth) || s32(height) < 0 || s32(height) > s32(kMaxHeight))
    {
        failAt(g, kWindowFile, kFunction, 0x32, { 0x54f5a4, width, height, kMaxWidth, kMaxHeight });
        return 0;
    }
    u32 window;
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
        window = *fe::direct<0x4e1620, ea::reserved>(app, ts, 0x54f5f4u, kRecordSize, where);
    }
    if (window == 0)
    {
        g.setDword(kErrorFile, kWindowFile);
        g.setDword(kErrorFunction, kFunction);
        g.setDword(kErrorLine, 0x41);
        const u32 largest = fe::direct<0x4e1580, ea::defaultLargest>(app, ts);
        fe::direct<kError, ea::error>(g.app(), g.ts(), 0x54f5fc, guest::Words{width, height, largest});
        return 0;
    }
    fe::direct<0x508b80, watb::initVideoWindow>(app, ts, window, width, height, bits, passedOn, passedOn2, passedOn3);
    return window;
}

nfs::u32 frontend::videoWindowMake(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_508c70", 0xc, 0, nullptr, 0, kSi | kDi };
    const u32 width = args.eax;
    const u32 height = args.edx;
    const u32 bits = args.ebx;
    const u32 passedOn = args.ecx;
    const u32 where = stackWord(app, ts, 0);
    const u32 passedOn2 = stackWord(app, ts, 1);
    const u32 passedOn3 = stackWord(app, ts, 2);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return menz::videoWindowMake(app, ts, width, height, bits, passedOn, where, passedOn2, passedOn3);
    });
}

/* sub_508da0: updatevideowindow: the window (eax, or the screen's) given a new row table
 * when its row size (edx) changed, ebx its bits, and its clip kept inside it. */
u32 frontend::menz::videoWindowUpdate(App* app, ThreadState& ts, u32 window, u32 rowBytes, u32 bits)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x10);
    constexpr u32 kWidth = 0x04, kHeight = 0x08, kClipLeft = 0x0c, kClipTop = 0x10, kClipRight = 0x14, kClipBottom = 0x18;
    constexpr u32 kBits = 0x20, kRowBytes = 0x28, kRows = 0x2c;
    if (window == 0)
        window = 0x564ff4;
    g.setDword(window + kBits, bits);
    if (rowBytes != g.dword(window + kRowBytes))
    {
        const u32 rows = g.dword(window + kRows);
        if (rows != 0)
            fe::direct<0x509b10, watb::removeRow>(app, ts, rows);
        g.setDword(window + kRowBytes, rowBytes);
        const u32 made = fe::direct<0x509a30, watb::makeRow>(app, ts, 0u, rowBytes, 0u, g.dword(window + kHeight));
        g.setDword(window + kRows, made);
        if (made == 0)
            failAt(g, kWindowFile, 0x54f63c, 0x5e, { 0x54f650, g.dword(window + kWidth), g.dword(window + kHeight) });
    }
    if (s32(g.dword(window + kWidth)) < s32(g.dword(window + kClipLeft)))
        g.setDword(window + kClipLeft, g.dword(window + kWidth));
    if (s32(g.dword(window + kHeight)) < s32(g.dword(window + kClipTop)))
        g.setDword(window + kClipTop, g.dword(window + kHeight));
    const u32 right = g.dword(window + kClipRight);
    if (s32(right) > s32(g.dword(window + kWidth)))
        g.setDword(window + kClipRight, g.dword(window + kWidth));
    else if (s32(right) < s32(g.dword(window + kClipLeft)))
        g.setDword(window + kClipRight, g.dword(window + kClipLeft));
    const u32 bottom = g.dword(window + kClipBottom);
    if (s32(bottom) > s32(g.dword(window + kHeight)))
        g.setDword(window + kClipBottom, g.dword(window + kHeight));
    else if (s32(bottom) < s32(g.dword(window + kClipTop)))
        g.setDword(window + kClipBottom, g.dword(window + kClipTop));
    return bottom;
}

nfs::u32 frontend::videoWindowUpdate(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_508da0", 0, 0, nullptr, 0, kCx | kSi | kDi };
    const u32 window = args.eax, rowBytes = args.edx, bits = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::videoWindowUpdate(app, ts, window, rowBytes, bits); });
}

/* sub_508e70: a window's (eax) row tables freed, and its record unless it is a fixed one. */
u32 frontend::menz::videoWindowFree(App* app, ThreadState& ts, u32 window)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0xc);
    u32 result = window;
    const u32 rows = g.dword(window + 0x2c);
    if (rows != 0)
    {
        result = fe::direct<0x509b10, watb::removeRow>(app, ts, rows);
        g.setDword(window + 0x2c, 0);
    }
    const u32 otherRows = g.dword(window + 0x30);
    if (otherRows != 0)
    {
        result = fe::direct<0x509b10, watb::removeRow>(app, ts, otherRows);
        g.setDword(window + 0x30, 0);
    }
    if (window == kWindowOne || window == kWindowTwo)
        return result;
    return *fe::direct<0x4e1890, ea::purgeBlock>(app, ts, window);
}

nfs::u32 frontend::videoWindowFree(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_508e70", 0, 0, nullptr, 0, kBx | kCx | kDx };
    const u32 window = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::videoWindowFree(app, ts, window); });
}

/* ---------------------------------------------------------------------------
 * Fonts
 * ------------------------------------------------------------------------- */

namespace
{
constexpr u32 kFontRowCount = 0x20, kFontVersion = 0x28, kFontGlyphs = 0x30, kFontScaleX = 0xc0, kFontScaleY = 0xc4, kFontScale = 0xc8;
constexpr u32 kFontStyle = 0xcc, kFontDirty = 0xbc;
constexpr u32 kFixedToFloat = 0x551ee0, kFloatToFixed = 0x551ee8;

double doubleAt(Guest& g, u32 address)
{
    return g.app()->getMemory<double>(address);
}
}

/* sub_5199b0: the glyph of a character (eax) found by halves in a table of records (edx)
 * sorted by it: ebx of them, ecx bytes each; 0 when it has none. */
u32 frontend::menz::fontFindGlyph(App* app, ThreadState& ts, u32 character, u32 table, u32 count, u32 size)
{
    Guest g(app, ts);
    u32 left = count;
    while (left != 0)
    {
        const u32 probe = table + u32(s32(left) >> 1) * size;
        const u32 difference = character - g.word(probe);
        if (difference == 0)
            return probe;
        if (s32(difference) > 0)
        {
            --left;
            table = probe + size;
        }
        left = u32(s32(left) >> 1);
    }
    return 0;
}

nfs::u32 frontend::fontFindGlyph(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_5199b0", 0, 0, nullptr, 0, kSi | kDi };
    const u32 character = args.eax, table = args.edx, count = args.ebx, size = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::fontFindGlyph(app, ts, character, table, count, size); });
}

/* sub_519a50: FONT_getcharacter: the glyph of a character (edx) in a font (eax): 11 bytes a
 * record, straight at its place for a font in order, or found. */
u32 frontend::menz::fontGlyph(App* app, ThreadState& ts, u32 font, u32 character)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x14);
    constexpr u32 kRecord = 11, kFirstCharacter = 0x20, kOldestVersion = 0x64;
    const u32 glyphs = g.dword(font + kFontGlyphs);
    if (s32(g.dword(font + kFontVersion)) < s32(kOldestVersion))
        failAt(g, 0x5507ac, 0x5507bc, 0x160, { 0x5507d0 });
    const u32 place = glyphs + (character - kFirstCharacter) * kRecord;
    if (g.word(place) == character)
        return place;
    return guest::direct<0x5199b0, fontFindGlyph>(app, ts, character, glyphs, g.dword(font + kFontRowCount), kRecord);
}

nfs::u32 frontend::fontGlyph(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_519a50", 0, 0, nullptr, 0, kBx | kCx | kSi | kDi };
    const u32 font = args.eax, character = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::fontGlyph(app, ts, font, character); });
}

/* sub_519ea0: how many bits a pixel a shape's type (the byte at eax) gives a font's glyph:
 * 1 for the one-bit types, 4 for all else. */
u32 frontend::menz::fontBitsPerPixel(App* app, ThreadState& ts, u32 shape)
{
    Guest g(app, ts);
    const u32 type = g.dword(shape) & 0xff;
    return type == 0x01 || type == 0x44 || type == 0x56 || type == 0x60 || type == 0x79 ? 1 : 4;
}

nfs::u32 frontend::fontBitsPerPixel(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_519ea0", 0, 0, nullptr, 0, kDx };
    const u32 shape = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::fontBitsPerPixel(app, ts, shape); });
}

/* sub_519f10: FONT_create: a font's record for a .FNT in memory (eax): 0 without memory. */
u32 frontend::menz::fontCreate(App* app, ThreadState& ts, u32 data)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x14);
    constexpr u32 kRecordSize = 0xe0, kDriver = 0x56718c, kAllocate = 0x14;
    constexpr u32 kSignature = 0x464e5400;  // FNT, its version byte left out
    constexpr u32 kDrawAny = 0x525670, kDrawAscii = 0x525630;
    const u32 shape = g.dword(data + 0x1c) + data;
    const u32 driver = g.dword(kDriver);
    const u32 font = callAt(g, g.dword(driver + kAllocate), args({ kRecordSize, driver }));
    if (font == 0)
        return 0;
    ts.sp -= 8;  // those two, saved only here
    const u32 first = g.dword(data);
    const u32 signature = ((first >> 24) | ((first >> 8) & 0xff00) | ((first << 8) & 0xff0000) | (first << 24)) & 0xffffff00;
    if (signature != kSignature)
        failAt(g, 0x550840, 0x550850, 0x6d, { 0x55085c, data });
    fe::direct<kMemset, watcom::clib::memset>(g.app(), g.ts(), font, 0, kRecordSize);
    const auto signedByte = [&](u32 address) { return u32(s32(std::int8_t(g.byte(address)))); };
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
    const u32 bits = guest::direct<0x519ea0, fontBitsPerPixel>(app, ts, shape);
    g.setDword(font + 0x0c, bits);
    const u32 shapeRows = u32(s32(g.dword(shape + 2)) >> 16);
    const u32 rowBits = guest::direct<0x519ea0, fontBitsPerPixel>(app, ts, shape) * shapeRows;
    g.setDword(font + 0x24, u32(s32((rowBits + 7) & 0xfffffff8) >> 3));
    u32 draw = kDrawAny;
    if ((u32(s32(g.dword(data + 0xc)) >> 16) & 3) != 2 && s32(g.dword(font + kFontRowCount)) <= 0xff && g.word(data + 0x20) <= 0xff)
        draw = kDrawAscii;
    g.setDword(font + 0x40, draw);
    fe::direct<0x525b80, rendf::fontSetOrigin>(app, ts, font, 0u, 0u);
    guest::direct<0x5257c0, fontSetProperty>(app, ts, font, 0, 0);
    guest::direct<0x5257c0, fontSetProperty>(app, ts, font, 5, 0);
    guest::direct<0x5257c0, fontSetProperty>(app, ts, font, 6, 0);
    guest::direct<0x5257c0, fontSetProperty>(app, ts, font, 7, 0);
    guest::direct<0x5257c0, fontSetProperty>(app, ts, font, 0xc, 0xffffffff);
    guest::direct<0x5257c0, fontSetProperty>(app, ts, font, 8, 0xffffffff);
    guest::direct<0x5257c0, fontSetProperty>(app, ts, font, 9, 0xffffff);
    guest::direct<0x5257c0, fontSetProperty>(app, ts, font, 0xa, 0xff000000);
    guest::direct<0x5257c0, fontSetProperty>(app, ts, font, 0xb, 0xff000000);
    g.setByte(font + kFontDirty, 1);
    const u32 glyph = guest::direct<0x519a50, fontGlyph>(app, ts, font, 0x41);
    const u32 capHeight = g.byte(glyph + 3) + signedByte(glyph + 10);
    g.setDword(font + 0x14, g.dword(font + 0x10) - capHeight);
    g.setDword(font + 0x10, capHeight);
    return font;
}

nfs::u32 frontend::fontCreate(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_519f10", 0, 0, nullptr, 0, kDx | kSi | kDi };
    const u32 data = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::fontCreate(app, ts, data); });
}

/* sub_5257c0: a font's (eax) property (edx) set to ebx: the scales in 16.16, the spacing
 * they give, the style's bits, colours and the shadow's; property 0 sets the scales to 1. */
u32 frontend::menz::fontSetProperty(App* app, ThreadState& ts, u32 font, u32 property, u32 value)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x18);
    const auto scaled = [&](u32 fixed) -> Float {
        return fp::mul(Float(nfs::s64(nfs::u64(fixed))), Float(doubleAt(g, kFixedToFloat)));
    };
    const auto setBit = [&](std::uint8_t bit) {
        const std::uint8_t style = g.byte(font + kFontStyle);
        g.setByte(font + kFontStyle, value == 1 ? style | bit : style & std::uint8_t(~bit));
    };
    const auto setColour = [&](u32 field) {
        if (value != g.dword(font + field))
        {
            g.setByte(font + kFontDirty, 1);
            g.setDword(font + field, value);
        }
    };
    if (property == 0)
    {
        guest::direct<0x5257c0, fontSetProperty>(app, ts, font, 1, 0);
        guest::direct<0x5257c0, fontSetProperty>(app, ts, font, 2, 0x10000);
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
        value = u32(fp::toInteger<s32>(pixels));
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

nfs::u32 frontend::fontSetProperty(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_5257c0", 0, 0, nullptr, 0, kCx | kSi | kDi };
    const u32 font = args.eax, property = args.edx, value = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::fontSetProperty(app, ts, font, property, value); });
}

/* sub_5259d0: a font's (eax) property (edx) read back, as sub_5257c0 sets it; ecx for one
 * it has not.  Nothing in the game calls it. */
u32 frontend::menz::fontGetProperty(App* app, ThreadState& ts, u32 font, u32 asked, u32 ecx)
{
    Guest g(app, ts);
    const u32 property = asked - 1;
    const auto fixed = [&](u32 field) {
        return u32(fp::toInteger<s32>(fp::mul(Float(g.floatAt(font + field)), Float(doubleAt(g, kFloatToFixed)))));
    };
    const auto bit = [&](std::uint8_t mask) -> u32 { return (g.byte(font + kFontStyle) & mask) ? 1 : 0; };
    switch (property)
    {
    case 0: return fixed(kFontScaleX);
    case 1: return fixed(kFontScaleY);
    case 2: return fixed(kFontScale);
    case 3:
    {
        const Float spacing = fp::mul(Float(s32(g.dword(font + 0x18))), Float(g.floatAt(font + kFontScale)));
        return u32(fp::toInteger<s32>(fp::mul(spacing, Float(doubleAt(g, kFloatToFixed)))));
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

nfs::u32 frontend::fontGetProperty(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_5259d0", 0, 0, nullptr, 0, kCx };
    const u32 font = args.eax, property = args.edx, ecx = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::fontGetProperty(app, ts, font, property, ecx); });
}

/* ---------------------------------------------------------------------------
 * Palettes
 * ------------------------------------------------------------------------- */

/* sub_51de60: eax colours of 3 bytes of 6 bits each (edx) to 4 bytes of 8 (ebx), the last
 * colour's alpha 0. */
u32 frontend::menz::paletteFrom6Bits(App* app, ThreadState& ts, u32 count, u32 from, u32 to)
{
    Guest g(app, ts);
    u32 result = to;
    const auto widen = [&](u32 at) -> u32 {
        const s32 sum = s32(g.byte(at)) * 0xff + 0x20;
        return u32(sum / 0x3f);
    };
    for (u32 i = 0; s32(i) < s32(count); ++i)
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

nfs::u32 frontend::paletteFrom6Bits(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_51de60", 0, 0, nullptr, 0, kCx | kSi | kDi };
    const u32 count = args.eax, from = args.edx, to = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::paletteFrom6Bits(app, ts, count, from, to); });
}

/* sub_51df60: eax colours of 3 bytes (edx) to 4 (ebx), the last colour's alpha 0. */
u32 frontend::menz::paletteFrom8Bits(App* app, ThreadState& ts, u32 count, u32 from, u32 to)
{
    Guest g(app, ts);
    u32 result = count;
    for (u32 i = 0; s32(i) < s32(count); ++i)
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

nfs::u32 frontend::paletteFrom8Bits(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_51df60", 0, 0, nullptr, 0, kCx | kSi | kDi };
    const u32 count = args.eax, from = args.edx, to = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::paletteFrom8Bits(app, ts, count, from, to); });
}

/* ---------------------------------------------------------------------------
 * Watcom's doubles and the FDIV test
 * ------------------------------------------------------------------------- */

/* sub_52448c: what a long double (eax) is: 0 zero, 1 a number, 2 NaN, 3 infinity, 4 denormal. */
u32 frontend::menz::longDoubleKind(App* app, ThreadState& ts, u32 value)
{
    Guest g(app, ts);
    const std::uint16_t exponent = g.word(value + 8) & 0x7fff;
    const u32 fractionLow = g.dword(value), fractionHigh = g.dword(value + 4);
    if (exponent == 0x7fff)
        return fractionHigh == 0x80000000 && fractionLow == 0 ? 3 : 2;
    if (exponent != 0)
        return 1;
    return fractionHigh == 0 && fractionLow == 0 ? 0 : 4;
}

nfs::u32 frontend::longDoubleKind(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_52448c", 0, 0, nullptr, 0, kDx };
    const u32 value = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::longDoubleKind(app, ts, value); });
}

namespace
{

/* The add, subtract, shift and rotate of a double kept in two registers, with the carry
 * the original keeps in its flags. */
struct Carry
{
    bool cf = false;

    u32 add(u32 a, u32 b)
    {
        const std::uint64_t sum = std::uint64_t(a) + b;
        cf = (sum >> 32) != 0;
        return u32(sum);
    }
    u32 adc(u32 a, u32 b)
    {
        const std::uint64_t sum = std::uint64_t(a) + b + (cf ? 1 : 0);
        cf = (sum >> 32) != 0;
        return u32(sum);
    }
    u32 sbb(u32 a, u32 b)
    {
        const std::uint64_t difference = std::uint64_t(a) - b - (cf ? 1 : 0);
        cf = (difference >> 32) != 0;
        return u32(difference);
    }
    u32 neg(u32 a)
    {
        cf = a != 0;
        return u32(0) - a;
    }
    u32 shr(u32 a)
    {
        cf = (a & 1) != 0;
        return a >> 1;
    }
    u32 rcr(u32 a)
    {
        const bool out = (a & 1) != 0;
        a = (a >> 1) | (u32(cf ? 1 : 0) << 31);
        cf = out;
        return a;
    }
};

/* shrd by cl: the count taken modulo 32, and a count of 0 moves nothing. */
inline u32 shiftInRight(u32 to, u32 from, std::uint8_t count)
{
    const unsigned n = count % 32;
    return n == 0 ? to : (to >> n) | (from << (32 - n));
}

inline u32 ror(u32 value, unsigned n)
{
    return (value >> n) | (value << (32 - n));
}

inline std::uint16_t low16(u32 value)
{
    return std::uint16_t(value);
}

inline u32 withLow16(u32 value, std::uint32_t low)
{
    return (value & 0xffff0000) | (low & 0xffff);
}

/* A double's sign bit at 31 and exponent in the low word: `sar 20` and the mask. */
inline u32 signAndExponent(u32 high)
{
    return u32(s32(high) >> 20) & 0x800007ff;
}

/* The upper words added, the low one of `base` left: how the two signs are summed. */
inline u32 signsSummed(u32 base, u32 other)
{
    return (((base >> 16) + (other >> 16)) << 16) | (base & 0xffff);
}

inline double doubleOf(u32 high, u32 low)
{
    const std::uint64_t bits = (std::uint64_t(high) << 32) | low;
    double value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

constexpr u32 kFpuKind = 0x5644c1;      // 0 no x87
constexpr u32 kFdivBug = 0x567858;      // bit 0: this x87 has the bug
constexpr u32 kOverflow = 0x52c8a7, kUnderflow = 0x52c88a, kDivideByZero = 0x52c89b;

/* The x87's way's end: the answer in edx:eax, a negative zero made zero. */
inline u32 answerFromX87(Guest& g, double value)
{
    std::uint64_t bits;
    std::memcpy(&bits, &value, sizeof bits);
    u32 high = u32(bits >> 32), low = u32(bits);
    if (high == 0x80000000)
        high = low = 0;
    return low;
}

/* A double with its exponent 0: shifted up until its leading bit is where the hidden one
 * would be, the exponent going down below zero for it. */
inline void normaliseDenormal(Carry& c, u32& high, u32& low, u32& exponent)
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
u32 frontend::menz::doubleDivide(App* app, ThreadState& ts, u32 low, u32 high, u32 divisorLow, u32 divisorHigh, double top)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 8);
    if (divisorLow == 0 && (divisorHigh << 1) == 0)
    {
        return u32(fe::direct<0x52c89b, watc::doubleInfinityHook>(app, ts));
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
            quotient = guest::direct<0x52700c, safeDivide>(app, ts, double(quotient), below, divisorLow, divisorHigh);
            if (ts.terminate)
                throw fe::Terminated();
        }
        else
        {
            quotient = fp::div(quotient, Float(doubleOf(divisorHigh, divisorLow)));
        }
        return answerFromX87(g, double(quotient));
    }

    Carry c;
    u32 exponent = signAndExponent(high);
    u32 divisorExponent = signAndExponent(divisorHigh);
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
        return u32(fe::direct<0x52c8a7, watc::doubleInfinity>(app, ts, divisorExponent));
    }
    if (std::int16_t(low16(exponent)) < -0x34)
    {
        return u32(fe::direct<0x52c88a, watc::doubleZero>(app, ts));
    }

    // The mantissas at the top of 64 bits, and the quotient's three words by the first
    // word of the divisor, corrected down by what the second word says.
    const u32 topHigh = (high << 11) | (low >> 21), topLow = low << 11;
    const u32 byHigh = (divisorHigh << 11) | (divisorLow >> 21), byLow = divisorLow << 11;
    u32 restHigh = topHigh, rest = topLow;
    u32 word2 = 0;
    u32 numerator = topHigh;
    if (byHigh <= numerator)
    {
        numerator -= byHigh;
        word2 = 1;
    }
    u32 word1 = u32(((std::uint64_t(numerator) << 32) | topLow) / byHigh);
    std::uint64_t part = std::uint64_t(byLow) * word1;
    u32 restLow = u32(part);
    std::uint64_t upper = std::uint64_t(byHigh) * word1 + u32(part >> 32);
    u32 upperLow = u32(upper), upperHigh = u32(upper >> 32);
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
    u32 word0 = u32(((std::uint64_t(restHigh) << 32) | rest) / byHigh);
    if (word0 != 0)
    {
        part = std::uint64_t(byLow) * word0;
        restLow = u32(part);
        upper = std::uint64_t(byHigh) * word0 + u32(part >> 32);
        restLow = c.neg(restLow);
        rest = c.sbb(rest, u32(upper));
        restHigh = c.sbb(restHigh, u32(upper >> 32));
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
    u32 resultLow = word0, resultHigh = word1, leftover = word2;
    exponent = withLow16(exponent, low16(exponent) - 1u);
    leftover = c.shr(leftover);
    if (c.cf)
    {
        resultHigh = c.rcr(resultHigh);
        resultLow = c.rcr(resultLow);
        exponent += 1;
    }
    u32 count = (restLow & 0xffffff00) | 0xb;
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

nfs::u32 frontend::doubleDivide(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_52643c", 0, 0, nullptr, 0, kSi | kDi };
    const u32 low = args.eax, high = args.edx, otherLow = args.ebx, otherHigh = args.ecx;
    const double top = args.floatIn[0];
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::doubleDivide(app, ts, low, high, otherLow, otherHigh, top); });
}

/* sub_526642: edx:eax less ecx:ebx, Watcom's __FDS. */
u32 frontend::menz::doubleSubtract(App* app, ThreadState& ts, u32 low, u32 high, u32 otherLow, u32 subtracted)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 8);
    u32 otherHigh = subtracted ^ 0x80000000;
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
    const auto finish = [&](u32 resultLow, u32 resultHigh, u32 ebx, u32 ecx) -> u32 {
        return resultLow;
    };
    const u32 exponent = signAndExponent(high);
    u32 other = signAndExponent(otherHigh);
    u32 sign = other;
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
            u32 resultHigh = otherHigh, resultLow = otherLow;
            c.cf = (sign >> 31) != 0;
            sign <<= 1;
            resultHigh = c.rcr(resultHigh);
            resultLow = c.rcr(resultLow);
            resultHigh &= 0x800fffff;
            resultHigh |= ror(sign, 13) & 0x7ff00000;
            return finish(resultLow, resultHigh, otherLow, other);
        }
    }
    if (s32(other) < 0)
    {
        otherHigh = u32(0) - otherHigh;
        otherLow = c.neg(otherLow);
        otherHigh = c.sbb(otherHigh, 0);
        sign ^= 0x80000000;
    }
    u32 sticky = 0;
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
    u32 scratch = otherLow;
    low = c.add(low, otherLow);
    high = c.adc(high, otherHigh);
    if (s32(high) < 0)
    {
        if (shift >= 0x35)
        {
            scratch = (scratch & 0xffffff00) | ((sticky & 0x7fffffff) != 0 ? 1 : 0);
            scratch = c.shr(scratch);
            low = c.adc(low, 0);
            high = c.adc(high, 0);
        }
        high = u32(0) - high;
        low = c.neg(low);
        high = c.sbb(high, 0);
        sign ^= 0x80000000;
    }
    scratch = low | high;
    if (scratch == 0)
        return finish(low, high, scratch, other);
    const auto tiny = [&]() -> u32 {
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
    const auto overflow = [&]() -> u32 {
        return u32(fe::direct<0x52c8a7, watc::doubleInfinity>(app, ts, sign));
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

nfs::u32 frontend::doubleSubtract(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_526642", 0, 0, nullptr, 0, kSi | kDi };
    const u32 low = args.eax, high = args.edx, otherLow = args.ebx, otherHigh = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::doubleSubtract(app, ts, low, high, otherLow, otherHigh); });
}

/* sub_5267ff: edx:eax times ecx:ebx, Watcom's __FDM. */
u32 frontend::menz::doubleMultiply(App* app, ThreadState& ts, u32 low, u32 high, u32 otherLow, u32 otherHigh)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 8);
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
    u32 exponent = signAndExponent(high);
    u32 sum = signAndExponent(otherHigh);
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
        return u32(fe::direct<0x52c8a7, watc::doubleInfinity>(app, ts, sum));
    }
    if (std::int16_t(low16(sum)) < -0x35)
    {
        return u32(fe::direct<0x52c88a, watc::doubleZero>(app, ts));
    }
    const u32 aHigh = (high << 11) | (low >> 21), aLow = low << 11;
    const u32 bHigh = (otherHigh << 11) | (otherLow >> 21), bLow = otherLow << 11;

    // Four words of the product.  `lowest` stays only to say whether any bit is set there.
    const std::uint64_t lowLow = std::uint64_t(aLow) * bLow;
    const u32 lowest = u32(lowLow);
    u32 second = u32(lowLow >> 32);
    const std::uint64_t cross1 = std::uint64_t(bHigh) * aLow;
    u32 third = u32(cross1 >> 32);
    u32 fourth = 0;
    second = c.add(second, u32(cross1));
    third = c.adc(third, 0);
    fourth = c.adc(fourth, fourth);
    const std::uint64_t cross2 = std::uint64_t(bLow) * aHigh;
    second = c.add(second, u32(cross2));
    third = c.adc(third, u32(cross2 >> 32));
    fourth = c.adc(fourth, 0);
    const std::uint64_t highHigh = std::uint64_t(aHigh) * bHigh;
    u32 resultLow = c.add(u32(highHigh), third);
    u32 resultHigh = c.adc(u32(highHigh >> 32), fourth);
    u32 below = shiftInRight(0, resultLow, 10);
    resultLow = shiftInRight(resultLow, resultHigh, 10);
    resultHigh = shiftInRight(resultHigh, below, 10);
    const auto overflow = [&]() -> u32 {
        return u32(fe::direct<0x52c8a7, watc::doubleInfinity>(app, ts, sum));
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
    const u32 place = c.rcr(ror(sum, 11)) & 0xfff00000;
    return resultLow;
}

nfs::u32 frontend::doubleMultiply(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_5267ff", 0, 0, nullptr, 0, kSi | kDi };
    const u32 low = args.eax, high = args.edx, otherLow = args.ebx, otherHigh = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::doubleMultiply(app, ts, low, high, otherLow, otherHigh); });
}

/* sub_526674: the x87's way's end for sub_5267ff, which has jumped here into sub_526642: the
 * answer `product` (st(0), which its thunk takes off), on the stack the other double and the
 * first, below the return address. */
u32 frontend::menz::doubleProductFromX87(App* app, ThreadState& ts, double product)
{
    Guest g(app, ts);
    return answerFromX87(g, product);
}

nfs::u32 frontend::doubleProductFromX87(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_526674", 0x10, 0, nullptr, 0, 0 };
    const double product = args.floatIn[0];
    args.floatsTaken = 1;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::doubleProductFromX87(app, ts, product); });
}

/* sub_52699c: edx:eax against ecx:ebx, a denormal taken for zero: -1, 0 or 1.  Its one
 * caller, sub_51f2a0, goes by the sign of the answer. */
u32 frontend::menz::doubleCompare(App* app, ThreadState& ts, u32 low, u32 high, u32 otherLow, u32 otherHigh)
{
    Guest g(app, ts);
    if ((high & 0x7ff00000) == 0)
        high = 0;
    if ((otherHigh & 0x7ff00000) == 0)
        otherHigh = 0;
    if (s32(otherHigh ^ high) >= 0)
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

nfs::u32 frontend::doubleCompare(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_52699c", 0, 0, nullptr, 0, 0 };
    const u32 low = args.eax, high = args.edx, otherLow = args.ebx, otherHigh = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::doubleCompare(app, ts, low, high, otherLow, otherHigh); });
}

/* sub_51f2a0: the runtime's start-up test of the Pentium's FDIV bug on an x87 of the 80387's
 * or later: 4195835 / 3145727 * 3145727 taken from 4195835, which a sound one has within
 * 1e-8 of nothing.  A bad one sets bit 0 of 0x567858.  The answer's sign is what the
 * original's jump after sub_52699c reads, 1e-8 being positive. */
u32 frontend::menz::fdivBugCheck(App* app, ThreadState& ts, u32 eax, double top)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x1c);
    constexpr u32 kBigLow = 0xc0000000, kBigHigh = 0x4150017e;    // 4195835.0
    constexpr u32 kByLow = 0x80000000, kByHigh = 0x4147ffff;      // 3145727.0
    constexpr u32 kTinyLow = 0xe2308c3a, kTinyHigh = 0x3e45798e;  // 1e-8
    if (g.byte(kFpuKind) < 3)
        return eax;
    // The division's slot on the x87 holds a copy of the caller's st(0), `top`, for the callee.
    // Each answer's high word stays where the call left it: in edx, the first dividend's (the x87's
    // way hands back only the low word in eax).
    const u32 quotient = guest::direct<0x52643c, doubleDivide>(app, ts, kBigLow, kBigHigh, kByLow, kByHigh, top);
    const u32 product = guest::direct<0x5267ff, doubleMultiply>(app, ts, quotient, kBigHigh, kByLow, kByHigh);
    const u32 difference = guest::direct<0x526642, doubleSubtract>(app, ts, kBigLow, kBigHigh, product, kBigHigh);
    const u32 order = guest::direct<0x52699c, doubleCompare>(app, ts, difference, kBigHigh, kTinyLow, kTinyHigh);
    if (s32(order) > 0)
        g.setByte(kFdivBug, g.byte(kFdivBug) | 1);
    return order;
}

nfs::u32 frontend::fdivBugCheck(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_51f2a0", 0, 0, nullptr, 0, kBx | kCx | kDx };
    const u32 eax = args.eax;
    const double top = args.floatIn[0];
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menz::fdivBugCheck(app, ts, eax, top); });
}

/* sub_52700c: `top` (st(0)) divided by a double on the stack, safe from the bug: through the
 * runtime's careful division (sub_526f87), whose two x87 slots are made if the stack has
 * room, else `below` (st(1)) kept in memory for it.  A NaN or an infinity simply divides.  The
 * quotient, which its thunk leaves in st(0); st(1) as it was (a double through a long double
 * is itself), eax as it came. */
double frontend::menz::safeDivide(App* app, ThreadState& ts, double top, double below, u32 divisorLow, u32 divisorHigh)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 4);
    const Float divisor = Float(doubleOf(divisorHigh, divisorLow));
    if ((divisorHigh & 0x7ff00000) == 0x7ff00000)
        return fp::div(Float(top), divisor);
    // (the x87's stack always has room here: its status word's top is never moved)
    ts.sp -= 0xc;
    g.app()->getMemory<nfs::Float80>(ts.sp) = Float(below);
    const double quotient = guest::direct<0x526f87, safeDivideCall>(app, ts, top, double(divisor));
    if (ts.terminate)
        throw fe::Terminated();
    return quotient;
}

nfs::u32 frontend::safeDivide(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_52700c", 8, 0, nullptr, 0, 0 };
    const u32 divisorLow = stackWord(app, ts, 0), divisorHigh = stackWord(app, ts, 1);
    const double top = args.floatIn[0], below = args.floatIn[1];
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        const u32 eax = args.eax;  // kept
        const double quotient = menz::safeDivide(app, ts, top, below, divisorLow, divisorHigh);
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
double frontend::menz::safeDivideCall(App* app, ThreadState& ts, double dividend, double divisor)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x2c);
    g.app()->getMemory<nfs::Float80>(g.frame()) = Float(divisor);
    g.app()->getMemory<nfs::Float80>(g.frame() + 0xc) = Float(dividend);
    nfs::Args none;
    if (!guest::pointerCall(app, ts, 0x5269d4, none))
        throw fe::Terminated();
    return none.floatResult;
}

nfs::u32 frontend::safeDivideCall(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_526f87", 0, 0, nullptr, 0, 0 };
    const double divisor = args.floatIn[0], dividend = args.floatIn[1];
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        const double answer = menz::safeDivideCall(app, ts, dividend, divisor);
        // the two operands replaced by the answer
        args.floatsTaken = 2;
        args.floatSet = true;
        args.floatResult = answer;
        return args.eax;
    });
}

}  // namespace nfs3hp
