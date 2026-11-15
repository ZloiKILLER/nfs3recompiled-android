#include "native_frontend_eaa.h"
#include "native_watcom.h"
#include "native_game.h"
#include "native_ea_files.h"
#include "native_ea_memory.h"
#include "native_ea_support.h"
#include "native_ea_threads.h"
#include "native_frontend_eab.h"
#include "native_frontend_menz.h"
#include "native_frontend_rendf.h"
#include "native_frontend_simc.h"
#include "native_frontend_wata.h"
#include "native_frontend_watb.h"
#include "native_frontend_wina.h"
#include "native_frontend.h"
#include "native_windows.h"
#include "host_threads.h"
#include "host_video.h"
#include "host_window.h"
#include <nfs3hp.h>
#include <lib/winapp.h>
#include <winapi/types.h>
#include <cstdint>
#include <initializer_list>
#include <optional>
#include <type_traits>
#include <utility>
#include <cstddef>

namespace nfs3hp
{
namespace
{

/* A typed core called as C++ (fe::direct) whose function changes no register but eax (as
 * tools/frontend_checks/specs_from_exe.py reads it): the registers left as the call by address
 * leaves them -- `words` in eax, edx, ebx and ecx as that call loads them, the first `Taken` of
 * them the core's arguments -- and its result in eax, which the call by address leaves there. */
template <x86::reg32 Address, auto Core, std::size_t... I>
auto directOf(win32::WinApplication* app, x86::CPU& cpu, const x86::reg32* words, std::index_sequence<I...>)
{
    return fe::direct<Address, Core>(app, cpu, words[I]...);
}

template <x86::reg32 Address, auto Core, std::size_t Taken, typename... Words>
x86::reg32 directKept(win32::WinApplication* app, x86::CPU& cpu, Words... words)
{
    const x86::reg32 all[] = { x86::reg32(words)..., 0 };
    x86::reg32* const into[] = { &cpu.eax, &cpu.edx, &cpu.ebx, &cpu.ecx };
    for (std::size_t i = 0; i < sizeof...(Words) && i < 4; ++i)
        *into[i] = all[i];
    const auto result = directOf<Address, Core>(app, cpu, all, std::make_index_sequence<Taken>{});
    if constexpr (std::is_same_v<std::remove_cv_t<decltype(result)>, x86::reg32>)
        cpu.eax = result;
    else
        cpu.eax = *result;
    return cpu.eax;
}

}  // namespace
}  // namespace nfs3hp


/* EA's libraries, the parts left of them:
 *
 *   the memory viewer  a full-screen browser of the memory pools, drawn through a
 *                      table of functions the caller hands in (sub_497620 and all
 *                      down to sub_49a400).  The game reaches it only from two of the
 *                      library's debug keys (sub_52b8d0, sub_52bb20, bound to keys 4 and
 *                      5 by sub_517de0 through sub_50c5d4), so it is a debug screen: its
 *                      tables are 0x56b170 and 0x56b198, its keys Up, Down, PgUp, PgDn,
 *                      Left, Right, Enter (a menu of VIEW and HEX DUMP), Space, C, H, S
 *                      and P;
 *   the text's fonts   sub_4d0d50, read once;
 *   udff               a file or a block of memory read a word at a time (sub_4dc0e0,
 *                      sub_4dc1b0, sub_4dc1e0);
 *   a file's size      sub_4e4360, sub_4e43a0;
 *   drawing surfaces   the bitmap drawn into (sub_4ead90, sub_4eada0), and it saved as
 *                      a TGA, the screenshot, by a debug key too (sub_4eaeb0 to sub_4eb160);
 *   DirectDraw         what the library would create a display with, reached from
 *                      sub_507b10 and sub_507bc0 (sub_4eb430 to sub_4ec1a0).
 *
 * All their state stays in guest memory.  What they call of the game is called by address but
 * the typed cores (below); Windows' functions and DirectDraw's methods are the port's own,
 * called as C++ (windows, thunked, method), in the checks through their slots, thunks and
 * tables; what each function leaves in eax and in the registers it does not save is kept.
 *
 * Plan step 5: each function a typed core in frontend::eaa (native_frontend_eaa.h), in a frame
 * of its own where the game's function has one (fe::Frame), and an adapter under the game's
 * name that reads the arguments from the registers and the stack for the calls by address.  A
 * core calls this file's and native_frontend_eab.cpp's as C++ (guest::direct), and the typed
 * cores of EA's libraries, of native_frontend_menz.cpp and of the other files with a header
 * (_wata, _wina, _watb, _simc, _rendf: fe::direct) -- but by address where
 * what the callee leaves in a register is what this function leaves there or hands on:
 * reservememadr and openhandle from sub_4dc0e0 (ebx), readhandle from sub_4dc1e0 (what
 * sub_4f2a7d hands on to exit). */
namespace nfs3hp
{

namespace
{

using fe::args;
using fe::Arguments;
using fe::Guest;
using fe::kRestoreAll;
using fe::kRestoreEbx;
using fe::kRestoreEcx;
using fe::kRestoreEdi;
using fe::kRestoreEdx;
using fe::kRestoreEsi;
using fe::runWhole;
using fe::Site;
using x86::reg32;
using x86::sreg32;

/* ---------------------------------------------------------------------------
 * The memory viewer's state
 * ------------------------------------------------------------------------- */
constexpr reg32 kConsole = 0x79f5a4;       // the table of drawing functions
constexpr reg32 kRowsTotal = 0x79f59c;     // the screen's height in characters
constexpr reg32 kColumnsTotal = 0x79f5c0;  // its width
constexpr reg32 kListCursor = 0x79f5a0;    // the block sub_499400 last gave
constexpr reg32 kNeedHeader = 0x79f5a8;
constexpr reg32 kBlockCount = 0x79f5ac;
constexpr reg32 kLastColumn = 0x79f5b0;
constexpr reg32 kRedrawAll = 0x79f5b4;
constexpr reg32 kListTop = 0x79f5b8;       // the first screen row of the list
constexpr reg32 kHeaderRow = 0x79f5bc;
constexpr reg32 kBlockTable = 0x79f5c4;    // 8 bytes a block: the block, its owner's value
constexpr reg32 kFirstShown = 0x79f5c8;
constexpr reg32 kPageSize = 0x79f5cc;
constexpr reg32 kListed = 0x79f5d0;
constexpr reg32 kLastShown = 0x79f5d4;
constexpr reg32 kPreviousSelected = 0x79f5d8;
constexpr reg32 kBlockTableGiven = 0x79f5dc;
constexpr reg32 kMenuEntries = 0x79f524;   // 12 bytes each: 8 of its name, its function
constexpr reg32 kMenuCount = 0x5662e0;
constexpr reg32 kMenuAdded = 0x9f25a0;
constexpr reg32 kSelected = 0x55e558;
constexpr reg32 kPoolShown = 0x55e55c;
constexpr reg32 kPools = 0x9ef5c0;
constexpr reg32 kHeaderKept = 0x55e560;
constexpr reg32 kColumnsUsed = 0x55e564;
constexpr reg32 kFirstColumn = 0x55e568;
constexpr reg32 kColumnWidths = 0x55e56c;
constexpr reg32 kListFresh = 0x55e590;
constexpr reg32 kClearOld = 0x55e594;
constexpr reg32 kClearColumns = 0x55e598;
constexpr reg32 kClearRows = 0x55e59c;
constexpr reg32 kSortChoosing = 0x55e5a0;
constexpr reg32 kSortColumn = 0x55e5a4;
constexpr reg32 kSortAscending = 0x55e5a8;
constexpr reg32 kHexNumbers = 0x55e5ac;
constexpr reg32 kShowFile = 0x55e5b0;      // the status line says the block's file
constexpr reg32 kShapeBits = 0x566d54;     // bits a pixel by a shape's type

constexpr reg32 kSpaceText = 0x53c094;     // " "
constexpr reg32 kThreeLetters = 0x53c0dc;  // "%3s"

constexpr std::uint16_t kFreeBlock = 0x4246, kUsedBlock = 0x424d;  // 'FB', 'MB'
constexpr reg32 kShpi = 0x53485049, kShpf = 0x53485046;

// The keys, as the console's function gives them.
constexpr reg32 kKeyEnter = 0x0d, kKeyEscape = 0x1b, kKeyUp = 0x4800, kKeyPageUp = 0x4900;
constexpr reg32 kKeyLeft = 0x4b00, kKeyRight = 0x4d00, kKeyDown = 0x5000, kKeyPageDown = 0x5100;

// What the game's other functions are.
constexpr reg32 kSprintf = 0x4df690, kBlockName = 0x4e15e0, kReserve = 0x4e1620;

/* The console: the table of functions the viewer draws through. */
class Console
{
public:
    explicit Console(Guest& guest) : m_guest(guest) {}

    reg32 begin() { return callSlot(0x00); }
    reg32 end() { return callSlot(0x04); }
    /* 0 columns, 1 rows, 2 pixels across, 3 down, 4 whether graphical. */
    reg32 metric(reg32 which)
    {
        const reg32 table = m_guest.dword(kConsole);
        return m_guest.call(m_guest.dword(table + 0x08), args({ which }));
    }
    /* At a column and a row, printf's arguments after them. */
    reg32 print(std::initializer_list<reg32> stack)
    {
        const reg32 table = m_guest.dword(kConsole);
        Arguments arguments;
        for (const reg32 value : stack)
            arguments.stack[arguments.stackCount++] = value;
        return m_guest.call(m_guest.dword(table + 0x0c), arguments);
    }
    reg32 drawShape(reg32 shape, reg32 bits, reg32 x, reg32 y)
    {
        const reg32 table = m_guest.dword(kConsole);
        return m_guest.call(m_guest.dword(table + 0x14), args({ shape, bits, x, y }));
    }
    reg32 key() { return callSlot(0x18); }
    reg32 init() { return callSlot(0x1c); }
    /* A block of the screen cleared: column, row, columns, rows; the colour on the stack. */
    reg32 clear(reg32 x, reg32 y, reg32 w, reg32 h, reg32 colour)
    {
        const reg32 table = m_guest.dword(kConsole);
        return m_guest.call(m_guest.dword(table + 0x20), args({ x, y, w, h }, { colour }));
    }
    reg32 colours(reg32 foreground, reg32 background)
    {
        const reg32 table = m_guest.dword(kConsole);
        return m_guest.call(m_guest.dword(table + 0x24), args({ foreground, background }));
    }

private:
    reg32 callSlot(reg32 slot)
    {
        const reg32 table = m_guest.dword(kConsole);
        return m_guest.call(m_guest.dword(table + slot), args({ table }));
    }

    Guest& m_guest;
};

void append(Guest& guest, reg32 to, reg32 from)
{
    guest.copyString(to + guest.length(to), from);
}

reg32 sprintf1(Guest& guest, reg32 buffer, reg32 format, reg32 value)
{
    return fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.cpu(), buffer, format, guest::Words{value});
}

/* sub_4993b0: the first block of the pool shown. */
reg32 firstBlock(Guest& guest)
{
    return guest.dword(guest.dword(kPools + guest.dword(kPoolShown) * 4) + 8);
}

/* sub_499480, sub_499490: a block's data and flags. */
reg32 blockData(reg32 block)
{
    return block + 0x10;
}

reg32 blockFlags(Guest& guest, reg32 block)
{
    return guest.word(block + 2);
}

/* sub_499460: a block's size, the block selected's for none. */
reg32 blockSize(Guest& guest, reg32 block)
{
    if (block == 0)
        block = guest.dword(guest.dword(kBlockTable) + guest.dword(kSelected) * 8);
    return guest.dword(block + 4);
}

/* sub_499580: whether a block is free space. */
bool blockIsFree(Guest& guest, reg32 block)
{
    return guest.word(block) == kFreeBlock;
}

reg32 listedBlock(Guest& guest, reg32 index)
{
    return guest.dword(guest.dword(kBlockTable) + index * 8);
}

/* sub_499e90: the bits a pixel of a shape's type. */
reg32 shapeBits(Guest& guest, reg32 shape)
{
    return guest.byte((guest.dword(shape) & 0x7f) + kShapeBits);
}

/* The word 2 bytes on, as the code reads it: the top half of the dword at `at`, signed. */
reg32 highWord(Guest& guest, reg32 at)
{
    return reg32(sreg32(guest.dword(at)) >> 16);
}

/* The first four bytes, each without its top bit, as one big-endian word: a pack's tag. */
reg32 tagOf(Guest& guest, reg32 at)
{
    reg32 tag = 0;
    for (reg32 i = 0; i < 4; ++i)
        tag = (tag << 8) | (guest.byte(at + i) & 0x7f);
    return tag;
}

/* sub_499330's numbers read into the frame, and sub_497d20 given them. */
void measureAndShow(Guest& guest, x86::CPU& cpu, reg32 total, reg32 used, reg32 available, reg32 largest)
{
    guest::direct<0x499330, frontend::eaa::memPoolTotals>(guest.app(), cpu, total, used, available, largest);
    guest::direct<0x497d20, frontend::eaa::memViewerStatsLine>(guest.app(), cpu, guest.dword(total), guest.dword(used),
                                                                guest.dword(available), guest.dword(largest));
}

/* An adapter's stack argument `index` (0 the first), above the return address. */
reg32 stackWord(win32::WinApplication* app, const x86::CPU& cpu, reg32 index)
{
    return app->getMemory<reg32>(cpu.esp + 4 + 4 * index);
}

}  // namespace

/* sub_497620: the viewer: the console's table in eax, until Escape. */
x86::reg32 frontend::eaa::memViewerRun(App* app, CPU& cpu, reg32 table)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x2c);
    Console console(g);
    const reg32 fp = g.frame() + 0x14;
    const reg32 total = fp - 0x14, used = fp - 0x10, available = fp - 0xc, largest = fp - 8;
    g.setDword(kConsole, table);
    console.init();
    reg32 blocks = guest::direct<0x4993d0, memPoolCountBlocks>(app, cpu);
    if (const reg32 given = g.dword(kBlockTableGiven))
        g.setDword(kBlockTable, given);
    else
    {
        // Else room for the table on the stack, if there is any.
        reg32 room = 0;
        // stackavail, handed the count sub_4993d0 left in eax.
        if (g.call(0x4f45c0, args({ blocks })) > 0x2000)
        {
            g.call(0x4f4610, args({}, { 0x2000 }));
            cpu.esp -= 0x2000;
            room = cpu.esp;
        }
        g.setDword(kBlockTable, room);
    }
    guest::direct<0x497ba0, memViewerOpen>(app, cpu);
    if (g.dword(kMenuAdded) == 0)
    {
        fe::direct<0x4f4650, wata::registryAddEntry>(app, cpu, 0x53bff8u, 0x4995b0u);
        fe::direct<0x4f4650, wata::registryAddEntry>(app, cpu, 0x53c000u, 0x499ea0u);
        g.setDword(kMenuAdded, 1);
    }
    guest::direct<0x497cc0, memViewerFirstDraw>(app, cpu, blocks);

    for (;;)
    {
        const reg32 key = console.key();
        g.setDword(kSortChoosing, 0);
        bool moved = false;
        g.setDword(kRedrawAll, 0);
        g.setDword(kNeedHeader, 0);
        const reg32 otherFile = g.dword(kShowFile) ^ 1;
        const reg32 page = g.dword(kPageSize) - 1;
        const std::int32_t lastBlock = std::int32_t(g.dword(kListed) - 1);
        switch (key)
        {
        case kKeyDown:
        {
            g.setDword(kShowFile, 0);
            const reg32 selected = g.dword(kSelected);
            g.setDword(kPreviousSelected, selected);
            if (lastBlock > std::int32_t(selected))
            {
                moved = true;
                g.setDword(kSelected, selected + 1);
            }
            const reg32 last = g.dword(kLastShown);
            if (std::int32_t(last) < std::int32_t(g.dword(kSelected)))
            {
                g.setDword(kRedrawAll, 1);
                g.setDword(kFirstShown, g.dword(kFirstShown) + 1);
                g.setDword(kLastShown, last + 1);
            }
            break;
        }
        case kKeyUp:
        {
            const reg32 selected = g.dword(kSelected);
            g.setDword(kPreviousSelected, selected);
            g.setDword(kShowFile, 0);
            if (std::int32_t(selected) > 0)
            {
                moved = true;
                g.setDword(kSelected, selected - 1);
            }
            const reg32 first = g.dword(kFirstShown);
            if (std::int32_t(first) > std::int32_t(g.dword(kSelected)))
            {
                g.setDword(kRedrawAll, 1);
                g.setDword(kFirstShown, first - 1);
                g.setDword(kLastShown, g.dword(kLastShown) - 1);
            }
            break;
        }
        case kKeyPageDown:
        {
            g.setDword(kShowFile, 0);
            const reg32 selected = g.dword(kSelected);
            const reg32 last = g.dword(kLastShown);
            g.setDword(kPreviousSelected, selected);
            if (std::int32_t(selected) < std::int32_t(last))
            {
                g.setDword(kSelected, last);
                g.setDword(kRedrawAll, 1);
            }
            else if (lastBlock > std::int32_t(last))
            {
                if (std::int32_t(last + g.dword(kPageSize)) > std::int32_t(g.dword(kListed)))
                {
                    const reg32 by = reg32(lastBlock) - last;
                    g.setDword(kLastShown, reg32(lastBlock));
                    g.setDword(kFirstShown, g.dword(kFirstShown) + by);
                }
                else
                {
                    g.setDword(kFirstShown, last);
                    g.setDword(kLastShown, last + page);
                }
                g.setDword(kRedrawAll, 1);
            }
            break;
        }
        case kKeyPageUp:
        {
            const reg32 selected = g.dword(kSelected);
            const reg32 first = g.dword(kFirstShown);
            g.setDword(kPreviousSelected, selected);
            g.setDword(kShowFile, 0);
            if (std::int32_t(selected) > std::int32_t(first))
            {
                g.setDword(kSelected, first);
                g.setDword(kRedrawAll, 1);
            }
            else if (std::int32_t(first) > 0)
            {
                if (std::int32_t(first - g.dword(kPageSize)) < 0)
                {
                    const reg32 last = g.dword(kLastShown);
                    g.setDword(kFirstShown, 0);
                    g.setDword(kLastShown, last - first);
                }
                else
                {
                    g.setDword(kLastShown, first);
                    g.setDword(kFirstShown, first - page);
                }
                g.setDword(kRedrawAll, 1);
            }
            break;
        }
        case kKeyRight:
        {
            const reg32 last = g.dword(kLastColumn);
            g.setDword(kShowFile, otherFile);
            if (std::int32_t(last) < 8)
            {
                g.setDword(kNeedHeader, 1);
                g.setDword(kFirstColumn, g.dword(kFirstColumn) + 1);
                g.setDword(kLastColumn, last + 1);
            }
            g.setDword(kRedrawAll, 1);
            break;
        }
        case kKeyLeft:
        {
            const reg32 first = g.dword(kFirstColumn);
            g.setDword(kShowFile, otherFile);
            if (std::int32_t(first) > 2)
            {
                g.setDword(kNeedHeader, 1);
                g.setDword(kFirstColumn, first - 1);
                g.setDword(kLastColumn, g.dword(kLastColumn) - 1);
            }
            g.setDword(kRedrawAll, 1);
            break;
        }
        case 0x53:  // S: sort
            guest::direct<0x498f00, memViewerSortMode>(app, cpu, blocks);
            break;
        case 0x48:  // H: hexadecimal or decimal
            g.setByte(kHexNumbers, std::uint8_t(g.byte(kHexNumbers) ^ 1));
            measureAndShow(g, cpu, total, used, available, largest);
            g.setDword(kRedrawAll, 1);
            break;
        case 0x43:  // C: the next pool
            g.setDword(kShowFile, 0);
            guest::direct<0x498e10, memViewerNextClass>(app, cpu);
            g.setDword(kClearOld, 1);
            g.setDword(kListFresh, 1);
            guest::direct<0x499330, memPoolTotals>(app, cpu, total, used, available, largest);
            blocks = guest::direct<0x4993d0, memPoolCountBlocks>(app, cpu);
            g.setDword(kBlockCount, blocks);
            g.setDword(kListed, guest::direct<0x497fe0, memViewerListBlocks>(app, cpu, blocks));
            guest::direct<0x497d20, memViewerStatsLine>(app, cpu, g.dword(total), g.dword(used), g.dword(available),
                                                         g.dword(largest));
            g.setDword(kNeedHeader, 1);
            g.setDword(kRedrawAll, 1);
            break;
        case 0x20:  // Space: view the block
            guest::direct<0x4995b0, memViewerViewShapes>(app, cpu);
            measureAndShow(g, cpu, total, used, available, largest);
            g.setDword(kNeedHeader, 1);
            g.setDword(kRedrawAll, 1);
            break;
        case kKeyEnter:  // the menu of what to do with it
            guest::direct<0x498bf0, memViewerMenu>(app, cpu);
            measureAndShow(g, cpu, total, used, available, largest);
            g.setDword(kNeedHeader, 1);
            g.setDword(kRedrawAll, 1);
            break;
        case 0x50:  // P: print the map to a file
            guest::direct<0x49a120, memViewerWriteMap>(app, cpu);
            break;
        default:
            break;
        }
        if (g.dword(kNeedHeader) == 1)
            guest::direct<0x498090, memViewerHeader>(app, cpu);
        if (moved || g.dword(kRedrawAll) == 1)
            guest::direct<0x498370, memViewerRows>(app, cpu);  // the count in eax too, which it does not read
        if (key == kKeyEscape)
            break;
    }
    guest::direct<0x497cb0, memViewerClose>(app, cpu);
    return kKeyEscape;
}

void frontend::memViewerRun(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_497620", 0, 0, nullptr, 0, kRestoreAll };
    const reg32 table = cpu.eax;
    runWhole(app, cpu, site, [&](Guest&) { return eaa::memViewerRun(app, cpu, table); });
}

/* sub_497ba0: the viewer's state, and the screen's size, afresh. */
x86::reg32 frontend::eaa::memViewerOpen(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x10);
    Console console(g);
    fe::direct<0x4f46d0, wata::windowSave>(app, cpu, 0x79f3fcu);
    g.setDword(kSelected, 0);
    fe::direct<0x4f4740, wata::textStateSave>(app, cpu, 0x79f46cu);
    if (console.metric(4))
        guest::direct<0x4ef520, eab::fontUse>(app, cpu, 0x5662e4u);
    console.begin();
    g.setDword(kRedrawAll, 1);
    g.setDword(kColumnsTotal, console.metric(0));
    g.setDword(kRowsTotal, console.metric(1));
    const reg32 result = console.end();
    for (const reg32 at : { kListCursor, kListed, kListTop, kHeaderKept, kHeaderRow, kPreviousSelected,
                            kFirstShown, kLastShown, kPageSize, kLastColumn, kColumnsUsed })
        g.setDword(at, 0);
    g.setDword(kRedrawAll, 1);
    g.setDword(kNeedHeader, 1);
    g.setDword(kListFresh, 1);
    for (const reg32 at : { kClearOld, kClearColumns, kClearRows, kSortChoosing, kSortColumn })
        g.setDword(at, 0);
    g.setDword(kSortAscending, 1);
    g.setDword(kHexNumbers, 0);
    g.setDword(kBlockCount, 0);
    return result;
}

void frontend::memViewerOpen(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_497ba0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx };
    runWhole(app, cpu, site, [&](Guest&) { return eaa::memViewerOpen(app, cpu); });
}

/* sub_497cb0: nothing, at the viewer's end. */
x86::reg32 frontend::eaa::memViewerClose(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    return g.eax();
}

void frontend::memViewerClose(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_497cb0", 0, 0, nullptr, 0, 0 };
    runWhole(app, cpu, site, [&](Guest&) { return eaa::memViewerClose(app, cpu); });
}

/* sub_497cc0: the whole screen, first: the numbers, the list, the header, its rows. */
x86::reg32 frontend::eaa::memViewerFirstDraw(App* app, CPU& cpu, reg32 blocks)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x24);
    const reg32 fp = g.frame() + 0x10;
    g.setDword(kBlockCount, blocks);
    measureAndShow(g, cpu, fp - 0x10, fp - 0xc, fp - 8, fp - 4);
    g.setDword(kListed, guest::direct<0x497fe0, memViewerListBlocks>(app, cpu, blocks));
    guest::direct<0x498090, memViewerHeader>(app, cpu);
    return guest::direct<0x498370, memViewerRows>(app, cpu);  // the count in eax too, which it does not read
}

void frontend::memViewerFirstDraw(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_497cc0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi };
    const reg32 blocks = cpu.eax;
    runWhole(app, cpu, site, [&](Guest&) { return eaa::memViewerFirstDraw(app, cpu, blocks); });
}

/* sub_497d20: the numbers at the top, as many lines as they take: eax the total, edx
 * what is used, ebx free, ecx the largest free. */
x86::reg32 frontend::eaa::memViewerStatsLine(App* app, CPU& cpu, reg32 total, reg32 used, reg32 available,
                                             reg32 largest)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x104);
    Console console(g);
    const reg32 fp = g.frame() + 0xf8 - 0x82;
    const reg32 line = fp - 0x76, piece = fp + 0x52, poolName = fp + 0x66;
    const reg32 values[6] = { 0, 0, total, used, available, largest };
    static const reg32 kHex[6] = { 0x53c00c, 0x53c01c, 0x53c028, 0x53c044, 0x53c05c, 0x53c074 };
    static const reg32 kDecimal[6] = { 0x53c00c, 0x53c01c, 0x53c034, 0x53c050, 0x53c068, 0x53c084 };

    g.setDword(poolName, g.dword(0x4975b0));
    g.setDword(poolName + 4, g.dword(0x4975b4));
    g.copyString(poolName, g.dword(kPools + g.dword(kPoolShown) * 4));
    console.colours(0xc, 1);
    g.setByte(line, 0);
    g.setByte(piece, 0);

    // Prints the line so far and moves down.
    reg32 row = 0;
    const auto putLine = [&] {
        console.begin();
        console.clear(0, row, g.dword(kColumnsTotal), 1, 1);
        console.print({ 0, row, line });
        console.end();
    };
    reg32 length = 0;
    for (reg32 i = 0; i < 6; ++i)
    {
        if (i == 1)
            fe::direct<kSprintf, watcom::clib::sprintf>(g.app(), g.cpu(), piece, kHex[i], guest::Words{poolName});
        else
        {
            const reg32 value = i == 0 ? g.dword(kBlockCount) : values[i];
            sprintf1(g, piece, g.dword(kHexNumbers) != 0 ? kHex[i] : kDecimal[i], value);
        }
        if (g.length(piece) + length + 1 > g.dword(kColumnsTotal) || i == 2)
        {
            putLine();
            ++row;
            if (std::int32_t(g.dword(kHeaderRow)) < std::int32_t(row))
                g.setDword(kHeaderRow, row);
            g.copyString(line, piece);
            length = g.length(line);
        }
        else
        {
            if (i != 0)
                append(g, line, kSpaceText);
            append(g, line, piece);
            length += 1 + g.length(piece);
        }
    }
    putLine();
    const reg32 lastRow = row;
    ++row;
    const reg32 header = g.dword(kHeaderRow);
    if (std::int32_t(header) < std::int32_t(row))
    {
        g.setDword(kListTop, row);
        g.setDword(kHeaderRow, row);
    }
    g.leaveEbx(lastRow);
    g.leaveEcx(g.dword(kConsole));
    g.leaveEdx(lastRow);
    return header;
}

void frontend::memViewerStatsLine(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_497d20", 0, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const reg32 total = cpu.eax, used = cpu.edx, available = cpu.ebx, largest = cpu.ecx;
    runWhole(app, cpu, site, [&](Guest&) {
        return eaa::memViewerStatsLine(app, cpu, total, used, available, largest);
    });
}

/* sub_497fe0: the blocks of the pool into the table, in pools of odd number without the
 * free ones: eax how many to go through; how many it holds comes back. */
x86::reg32 frontend::eaa::memViewerListBlocks(App* app, CPU& cpu, reg32 blocks)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x30);
    const reg32 fp = g.frame() + 0x18;
    const std::int32_t count = std::int32_t(blocks);
    const bool skipFree = (g.byte(kPoolShown) & 1) != 0;
    reg32 listed = 0;
    for (std::int32_t i = 0; i < count; ++i)
    {
        // The game's code hands it i in eax too, which it does not read.
        guest::direct<0x499400, memPoolNextBlock>(app, cpu, fp - 0x18, fp - 0x14, fp - 0x10, fp - 0xc);
        const reg32 block = g.dword(kListCursor);
        if (g.word(block) == kFreeBlock && skipFree)
            continue;
        g.setDword(g.dword(kBlockTable) + reg32(i) * 8, block);
        const std::uint16_t tag = g.word(block);
        ++listed;
        reg32 owner = 0;
        if (tag != kFreeBlock)
            owner = fe::direct<0x4f4780, wata::blockChecksum>(app, cpu, g.dword(fp - 0x18) + 0x10, g.dword(fp - 0x14));
        g.setDword(g.dword(kBlockTable) + reg32(i) * 8 + 4, owner);
    }
    return listed;
}

void frontend::memViewerListBlocks(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_497fe0", 0, 0, nullptr, 0, kRestoreAll };
    const reg32 blocks = cpu.eax;
    runWhole(app, cpu, site, [&](Guest&) { return eaa::memViewerListBlocks(app, cpu, blocks); });
}

/* sub_498090: the columns' header, and the list's size under it. */
x86::reg32 frontend::eaa::memViewerHeader(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x74);
    Console console(g);
    constexpr reg32 kTitles = 0x4975b8, kRightAligned = 0x4975dc;
    const reg32 title = g.frame() + 0x5c - 0x14;

    const reg32 kept = g.dword(kHeaderKept);
    if (kept == 0)
    {
        const reg32 top = g.dword(kListTop);
        g.setDword(kHeaderRow, top);
        g.setDword(kHeaderKept, top);
    }
    else
        g.setDword(kHeaderRow, kept);
    console.begin();
    console.clear(0, g.dword(kHeaderRow), g.dword(kColumnsTotal), 1, 1);

    // Columns 0 and 1 stay on the screen.
    std::int32_t x = 0;
    const auto sortedColumn = [&](reg32 column) {
        return column == g.dword(kSortColumn) && g.dword(kSortChoosing) == 1;
    };
    for (reg32 column = 0; column < 2; ++column)
    {
        if (sortedColumn(column))
            console.colours(3, 5);
        else
            console.colours(2, 1);
        g.copyString(title, g.dword(kTitles + column * 4));
        console.print({ reg32(x), g.dword(kHeaderRow), title });
        x = std::int32_t(reg32(x) + g.dword(kColumnWidths + column * 4) + 1);
    }
    x = std::int32_t(reg32(x) - 1);
    if (x > std::int32_t(g.dword(kColumnsUsed)))
        g.setDword(kColumnsUsed, reg32(x));

    // The others from the first one shown, while they fit.
    for (std::int32_t column = std::int32_t(g.dword(kFirstColumn)); column < 9; ++column)
    {
        const reg32 width = g.dword(reg32(column) * 4 + kColumnWidths);
        const std::int32_t end = std::int32_t(reg32(x) + width + 1);
        if (end >= std::int32_t(g.dword(kColumnsTotal)))
            break;
        std::int32_t columns = std::int32_t(g.dword(kColumnsUsed) - reg32(x));
        if (columns <= std::int32_t(width + 1))
            columns = std::int32_t(width + 1);
        console.clear(reg32(x), g.dword(kHeaderRow), reg32(columns), 1, 1);
        g.copyString(title, g.dword(kTitles + reg32(column) * 4));
        if (sortedColumn(reg32(column)))
            console.colours(3, 5);
        else
            console.colours(2, 4);
        reg32 at;
        if (g.dword(kRightAligned + reg32(column) * 4) == 1)
            at = reg32(end) - g.length(g.dword(kTitles + reg32(column) * 4));
        else
            at = reg32(x) + 1;
        console.print({ at, g.dword(kHeaderRow), title });
        g.setDword(kLastColumn, reg32(column));
        x = end;
        if (x > std::int32_t(g.dword(kColumnsUsed)))
            g.setDword(kColumnsUsed, reg32(x));
    }

    g.setDword(kHeaderRow, g.dword(kHeaderRow) + 1);
    console.end();
    reg32 result = g.dword(kHeaderRow);
    g.setDword(kListTop, result);
    if (g.dword(kListFresh) != 0)
    {
        const reg32 listed = g.dword(kListed);
        g.setDword(kFirstShown, 0);
        reg32 last = g.dword(kRowsTotal) - g.dword(kListTop) - 1;
        g.setDword(kSelected, 0);
        g.setDword(kLastShown, last);
        if (std::int32_t(last) >= std::int32_t(listed))
        {
            last = listed - 1;
            g.setDword(kLastShown, last);
        }
        result = g.dword(kLastShown) - g.dword(kFirstShown) - 1;
        g.setDword(kListFresh, 0);
        g.setDword(kPageSize, result);
    }
    return result;
}

void frontend::memViewerHeader(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_498090", 0, 0, nullptr, 0, kRestoreAll };
    runWhole(app, cpu, site, [&](Guest&) { return eaa::memViewerHeader(app, cpu); });
}

/* sub_498370: the list's rows: the names' column, then a column at a time; only the
 * rows that changed, unless all are wanted (0x79f5b4). */
x86::reg32 frontend::eaa::memViewerRows(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x74);
    Console console(g);
    const reg32 fp = g.frame() + 0x5c;
    const reg32 buffer = fp - 0x5c, flagText = fp - 0x30, kinds = fp - 0x3c, spaceText = fp - 4;
    const reg32 drawThis = fp - 0x10;  // what comes before leaves it, as in the code

    g.setWord(spaceText, g.word(0x497600));
    for (reg32 k = 0; k < 3; ++k)
        g.setDword(kinds + k * 4, g.dword(0x497604 + k * 4));
    console.begin();
    if (g.dword(kClearOld) != 0)
    {
        console.clear(0, g.dword(kHeaderRow), g.dword(kClearColumns), g.dword(kClearRows), 1);
        g.setDword(kClearOld, 0);
    }
    if (g.dword(kRedrawAll) != 0)
    {
        const std::int32_t room = std::int32_t(g.dword(kRowsTotal) - g.dword(kHeaderRow));
        const std::int32_t listed = std::int32_t(g.dword(kListed));
        const reg32 rows = reg32(room < listed ? room : listed);
        g.setDword(kClearRows, rows);
        console.clear(0, g.dword(kHeaderRow), g.dword(kColumnsUsed), rows, 1);
    }
    console.colours(0, 1);

    // A row is drawn if everything is, or it is the selection, old or new.
    const auto rowChanged = [&](reg32 index, reg32& row) {
        if (g.dword(kRedrawAll) == 0)
        {
            if (index == g.dword(kSelected) || index == g.dword(kPreviousSelected))
                g.setDword(drawThis, 1);
            else
            {
                ++row;
                g.setDword(drawThis, 0);
            }
        }
    };

    reg32 row = g.dword(kListTop);
    reg32 index = g.dword(kFirstShown);
    g.setDword(kPageSize, g.dword(kLastShown) - index + 1);
    for (; std::int32_t(index) < std::int32_t(g.dword(kLastShown) + 1); ++index)
    {
        g.setDword(drawThis, 1);
        rowChanged(index, row);
        if (g.dword(drawThis) != 1)
            continue;
        const reg32 selected = g.dword(kSelected);
        if (g.word(listedBlock(g, index)) == kFreeBlock)
        {
            if (index == selected)
            {
                guest::direct<0x4989a0, memViewerStatusLine>(app, cpu, selected);
                console.clear(0, row, g.dword(kColumnsUsed), 1, 0xb);
                console.colours(0xa, 0xb);
            }
            else
            {
                console.clear(0, row, g.dword(kColumnsUsed), 1, 9);
                console.colours(8, 9);
            }
            fe::direct<kSprintf, watcom::clib::sprintf>(g.app(), g.cpu(), buffer, 0x53c098, guest::Words({}));
        }
        else
        {
            if (index == selected)
            {
                guest::direct<0x4989a0, memViewerStatusLine>(app, cpu, selected);
                console.clear(0, row, g.dword(kColumnsUsed), 1, 7);
                console.colours(6, 7);
            }
            else
            {
                console.clear(0, row, g.dword(kColumnsUsed), 1, 1);
                console.colours(0, 1);
            }
            reg32 name = directKept<0x4e15e0, ea::blockNameOf, 1>(g.app(), g.cpu(), listedBlock(g, index) + 0x10);
            const reg32 length = g.length(name);
            if (std::int32_t(length) > 0xc)
                name += length - 0xc;
            fe::direct<kSprintf, watcom::clib::sprintf>(g.app(), g.cpu(), buffer, 0x53c0a4, guest::Words{0, name});
        }
        console.print({ 0, row, buffer });
        ++row;
    }

    // The other columns.
    reg32 x = g.dword(kColumnWidths) + g.dword(kColumnWidths + 4) + 1;
    for (std::int32_t column = std::int32_t(g.dword(kFirstColumn)); column <= std::int32_t(g.dword(kLastColumn)); ++column)
    {
        reg32 columnRow = g.dword(kListTop);
        for (reg32 i = g.dword(kFirstShown); std::int32_t(i) < std::int32_t(g.dword(kLastShown) + 1); ++i)
        {
            rowChanged(i, columnRow);
            if (g.dword(drawThis) != 1)
                continue;
            const bool isFree = blockIsFree(g, listedBlock(g, i));
            if (i == g.dword(kSelected))
            {
                if (isFree)
                    console.colours(0xa, 0xb);
                else
                    console.colours(6, 7);
            }
            else if (isFree)
                console.colours(8, 9);
            else
                console.colours(0, 1);
            const reg32 at = x + 1;
            const auto spaceAfter = [&] {
                if (reg32(column) != g.dword(kLastColumn))
                    append(g, buffer, spaceText);
            };
            // A column with nothing for this row leaves it empty.
            bool show = true;
            switch (column)
            {
            case 2:  // the size
                sprintf1(g, buffer, g.dword(kHexNumbers) != 0 ? 0x53c0b4 : 0x53c0bc, blockSize(g, listedBlock(g, i)));
                spaceAfter();
                break;
            case 3:  // where it is
                sprintf1(g, buffer, 0x53c0c4, blockData(listedBlock(g, i)));
                spaceAfter();
                break;
            case 4:  // its flags
                guest::direct<0x498e40, memViewerFlagText>(app, cpu, blockFlags(g, listedBlock(g, i)), flagText);
                sprintf1(g, buffer, 0x53c0c8, flagText);
                spaceAfter();
                break;
            case 5:
                fe::direct<kSprintf, watcom::clib::sprintf>(g.app(), g.cpu(), buffer, 0x53c0d0, guest::Words({}));
                spaceAfter();
                break;
            case 7:  // its owner's value
                if (blockIsFree(g, listedBlock(g, i)))
                    show = false;
                else
                {
                    sprintf1(g, buffer, 0x53c0d4, g.dword(g.dword(kBlockTable) + i * 8 + 4));
                    spaceAfter();
                }
                break;
            case 8:  // its owner's kind
            {
                const reg32 kind = fe::direct<0x4f47c0, wata::shapeEndsAtBend>(app, cpu, listedBlock(g, i));
                sprintf1(g, buffer, kThreeLetters, g.dword(kinds + kind * 4));
                break;
            }
            default:
                show = false;
                break;
            }
            if (show)
                console.print({ at, columnRow, buffer });
            ++columnRow;
        }
        x += g.dword(reg32(column) * 4 + kColumnWidths) + 1;
        if (std::int32_t(x) > std::int32_t(g.dword(kColumnsUsed)))
            g.setDword(kColumnsUsed, x);
        g.setDword(kClearColumns, g.dword(kColumnsUsed));
    }
    return console.end();
}

void frontend::memViewerRows(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_498370", 0, 0, nullptr, 0, kRestoreAll };
    runWhole(app, cpu, site, [&](Guest&) { return eaa::memViewerRows(app, cpu); });
}

/* sub_4989a0: the line at the bottom, of the block selected: eax its index. */
x86::reg32 frontend::eaa::memViewerStatusLine(App* app, CPU& cpu, reg32 index)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x64);
    Console console(g);
    const reg32 text = g.frame();
    const reg32 kind = guest::direct<0x4994a0, memBlockKind>(app, cpu, index);
    guest::direct<0x498a40, memViewerDescribeBlock>(app, cpu, index, kind, text);
    console.clear(0, g.dword(kRowsTotal), g.dword(kColumnsUsed), 1, 0xe);
    console.colours(0xd, 0xe);
    console.print({ 0, g.dword(kRowsTotal), text });
    return console.colours(0, 1);
}

void frontend::memViewerStatusLine(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4989a0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi };
    const reg32 index = cpu.eax;
    runWhole(app, cpu, site, [&](Guest&) { return eaa::memViewerStatusLine(app, cpu, index); });
}

/* sub_498a40: what the status line says of a block: eax its index, edx its kind
 * (sub_4994a0), ebx where the text goes. */
x86::reg32 frontend::eaa::memViewerDescribeBlock(App* app, CPU& cpu, reg32 index, reg32 kind, reg32 text)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x20);
    const reg32 fp = g.frame() + 0x10;
    const reg32 showFile = g.dword(kShowFile);
    g.leaveEbx(showFile);
    for (const reg32 at : { fp - 0xc, fp - 8, fp - 4, fp - 0x10 })
        g.setDword(at, 0);
    if (showFile != 0)
    {
        g.leaveEdx(index * 8);
        const reg32 name = directKept<0x4e15e0, ea::blockNameOf, 1>(g.app(), g.cpu(), listedBlock(g, index) + 0x10);
        return fe::direct<kSprintf, watcom::clib::sprintf>(g.app(), g.cpu(), text, 0x53c1ac, guest::Words{name, 0x53c0e0});
    }
    switch (kind)
    {
    case 0:
        return fe::direct<kSprintf, watcom::clib::sprintf>(g.app(), g.cpu(), text, 0x53c148, guest::Words({}));
    case 1:
        return fe::direct<kSprintf, watcom::clib::sprintf>(g.app(), g.cpu(), text, 0x53c154, guest::Words({}));
    case 3:
    {
        guest::direct<0x498b70, memViewerCountShapes>(app, cpu, listedBlock(g, index) + 0x10, fp - 0x10, fp - 0xc, fp - 8,
                                                      fp - 4);
        const reg32 columns = g.dword(kColumnsTotal);
        g.leaveEbx(columns);
        g.leaveEdx(g.dword(fp - 8));
        return fe::direct<kSprintf, watcom::clib::sprintf>(g.app(), g.cpu(), text, std::int32_t(columns) < 0x46 ? 0x53c0e4u : 0x53c10cu, guest::Words{g.dword(fp - 0x10), g.dword(fp - 8), g.dword(fp - 4), g.dword(fp - 0xc)});
    }
    case 4:
        return fe::direct<kSprintf, watcom::clib::sprintf>(g.app(), g.cpu(), text, 0x53c16c, guest::Words({}));
    case 5:
        return fe::direct<kSprintf, watcom::clib::sprintf>(g.app(), g.cpu(), text, 0x53c18c, guest::Words({}));
    default:
        return fe::direct<kSprintf, watcom::clib::sprintf>(g.app(), g.cpu(), text, 0x53c164, guest::Words({}));
    }
}

void frontend::memViewerDescribeBlock(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_498a40", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 index = cpu.eax, kind = cpu.edx, text = cpu.ebx;
    runWhole(app, cpu, site, [&](Guest&) { return eaa::memViewerDescribeBlock(app, cpu, index, kind, text); });
}

/* sub_498b70: the entries of a pack by what they are: eax the pack, edx the number of
 * them, ebx what is no picture (hot spots, palette animation, comments, palettes), ecx
 * the packed pictures, the stack's the others. */
x86::reg32 frontend::eaa::memViewerCountShapes(App* app, CPU& cpu, reg32 pack, reg32 countAt, reg32 others,
                                               reg32 packed, reg32 unpacked)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x1c);
    const reg32 fp = g.frame() + 0x10;
    g.setDword(countAt, fe::direct<0x4ea654, simc::packCount>(app, cpu, pack));
    reg32 i = 0;
    for (; std::int32_t(i) < std::int32_t(g.dword(countAt)); ++i)
    {
        g.leaveEcx(i);
        g.leaveEbx(fp - 0x10);  // the game's code hands it in ebx, where it stays to this one's exit
        fe::direct<0x4ea665, wina::tableEntry>(app, cpu, pack, i, fp - 0x10);
        const reg32 shape = guest::direct<0x4ef1e0, eab::shapeFileFind>(app, cpu, pack, fp - 0x10);
        g.leaveEdx(shape);
        const reg32 type = g.dword(shape) & 0x7f;
        reg32 counter;
        if (type == 0x7c || type == 0x6e || type == 0x6f || (type & 0x70) == 0x20)
            counter = others;
        else
            counter = (g.dword(shape) & 0x80) != 0 ? packed : unpacked;
        g.setDword(counter, g.dword(counter) + 1);
    }
    g.leaveEcx(i);
    return countAt;
}

void frontend::memViewerCountShapes(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_498b70", 4, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const reg32 pack = cpu.eax, countAt = cpu.edx, others = cpu.ebx, packed = cpu.ecx;
    const reg32 unpacked = stackWord(app, cpu, 0);
    runWhole(app, cpu, site, [&](Guest&) {
        return eaa::memViewerCountShapes(app, cpu, pack, countAt, others, packed, unpacked);
    });
}

/* sub_498bf0: the menu along the bottom of what to do with the block, by arrows and
 * Enter. */
x86::reg32 frontend::eaa::memViewerMenu(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x2c);
    Console console(g);
    const reg32 text = g.frame();
    const std::int32_t count = std::int32_t(g.dword(kMenuCount));
    std::int32_t chosen = 0;

    const auto drawEntry = [&](std::int32_t i) {
        if (i == chosen)
            console.colours(0x12, 0x11);
        else
            console.colours(0x10, 0xf);
        sprintf1(g, text, 0x53c1c0, reg32(i) * 12 + kMenuEntries);
        console.begin();
        console.print({ reg32(i) * 9, g.dword(kRowsTotal), text });
        console.end();
    };

    console.begin();
    console.clear(0, g.dword(kRowsTotal), g.dword(kColumnsUsed), 1, 0xf);
    console.end();
    for (std::int32_t i = 0; i < count; ++i)
        drawEntry(i);

    bool done = false;
    while (!done)
    {
        const reg32 key = console.key();
        bool moved = false;
        switch (key)
        {
        case kKeyUp:
        case kKeyDown:
        case kKeyEscape:
            done = true;
            break;
        case kKeyRight:
            if (chosen < count - 1)
            {
                moved = true;
                ++chosen;
            }
            break;
        case kKeyLeft:
            if (chosen > 0)
            {
                moved = true;
                --chosen;
            }
            break;
        case kKeyEnter:
            done = true;
            g.call(g.dword(reg32(chosen) * 12 + kMenuEntries + 8));
            break;
        default:
            break;
        }
        if (moved)
        {
            const std::int32_t first = chosen == 0 ? 0 : chosen - 1;
            const std::int32_t last = chosen == count - 1 ? count - 1 : chosen + 1;
            for (std::int32_t i = first; i < last + 1; ++i)
                drawEntry(i);
        }
    }
    return console.colours(0, 1);
}

void frontend::memViewerMenu(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_498bf0", 0, 0, nullptr, 0, kRestoreAll };
    runWhole(app, cpu, site, [&](Guest&) { return eaa::memViewerMenu(app, cpu); });
}

/* sub_498e10: the next pool that exists. */
x86::reg32 frontend::eaa::memViewerNextClass(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    reg32 pool = g.dword(kPoolShown);
    do
        pool = (pool + 1) & 0xf;
    while (g.dword(pool * 4 + kPools) == 0);
    g.setDword(kPoolShown, pool);
    return g.eax();
}

void frontend::memViewerNextClass(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_498e10", 0, 0, nullptr, 0, kRestoreEdx };
    runWhole(app, cpu, site, [&](Guest&) { return eaa::memViewerNextClass(app, cpu); });
}

/* sub_498e40: a block's flags in eax as letters at edx. */
x86::reg32 frontend::eaa::memViewerFlagText(App* app, CPU& cpu, reg32 flags, reg32 text)
{
    Guest g(app, cpu);
    g.setByte(text, (flags & 0x200) != 0 ? 'e' : ' ');
    g.setByte(text + 1, (flags & 0x2000) != 0 ? 'b' : ' ');
    g.setByte(text + 3, ' ');
    g.setByte(text + 4, ' ');
    g.setByte(text + 2, (flags & 0x10) != 0 ? 'h' : ' ');
    g.setByte(text + 5, (flags & 0x40) != 0 ? 'a' : ' ');
    const reg32 last = (flags & 0x8000) != 0 ? 0x73 : 0x20;
    g.setByte(text + 7, ' ');
    g.setByte(text + 8, ' ');
    g.setByte(text + 9, ' ');
    g.setByte(text + 6, std::uint8_t(last));
    return last;
}

void frontend::memViewerFlagText(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_498e40", 0, 0, nullptr, 0, kRestoreEcx };
    const reg32 flags = cpu.eax, text = cpu.edx;
    runWhole(app, cpu, site, [&](Guest&) { return eaa::memViewerFlagText(app, cpu, flags, text); });
}

/* sub_498f00: choosing the column to sort by: the arrows move along the header, Up
 * sorts down it and Down up it, Escape leaves. */
x86::reg32 frontend::eaa::memViewerSortMode(App* app, CPU& cpu, reg32 blocks)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x1c);
    Console console(g);
    const reg32 listed = g.dword(kListed);
    g.setDword(kSortChoosing, 1);
    g.setDword(kSortColumn, 0);
    g.setDword(kSortAscending, 1);
    guest::direct<0x498090, memViewerHeader>(app, cpu);

    const auto sort = [&] {
        // Column 4 has nothing to sort by.
        if (g.dword(kSortColumn) <= 6 && g.dword(kSortColumn) != 4)
            fe::direct<0x4ee4f8, watcom::clib::qsort>(g.app(), g.cpu(), g.dword(kBlockTable), listed, 8, 0x499150);
    };
    bool done = false;
    while (!done)
    {
        bool headerChanged = false;
        g.setDword(kRedrawAll, 0);
        const reg32 key = console.key();
        switch (key)
        {
        case kKeyEscape:
            g.setDword(kSortChoosing, 0);
            done = true;
            headerChanged = true;
            break;
        case kKeyUp:
            g.setDword(kSortAscending, 0);
            sort();
            g.setDword(kRedrawAll, 1);
            g.setDword(kSortChoosing, 0);
            done = true;
            headerChanged = true;
            break;
        case kKeyDown:
            g.setDword(kSortAscending, 1);
            sort();
            g.setDword(kSortChoosing, 0);
            g.setDword(kRedrawAll, 1);
            done = true;
            headerChanged = true;
            break;
        case kKeyLeft:
        {
            const reg32 column = g.dword(kSortColumn);
            if (std::int32_t(column) > 0)
            {
                headerChanged = true;
                g.setDword(kSortColumn, column - 1);
            }
            const reg32 first = g.dword(kFirstColumn);
            if (std::int32_t(first) > 2)
            {
                const std::int32_t now = std::int32_t(g.dword(kSortColumn));
                if (now < std::int32_t(first) && now > 1)
                {
                    headerChanged = true;
                    const reg32 last = g.dword(kLastColumn);
                    g.setDword(kRedrawAll, 1);
                    g.setDword(kFirstColumn, first - 1);
                    g.setDword(kLastColumn, last - 1);
                }
            }
            break;
        }
        case kKeyRight:
        {
            const reg32 last = g.dword(kLastColumn);
            if (last == g.dword(kSortColumn) && std::int32_t(last) < 8)
            {
                headerChanged = true;
                g.setDword(kRedrawAll, 1);
                g.setDword(kFirstColumn, g.dword(kFirstColumn) + 1);
                const reg32 column = g.dword(kSortColumn);
                g.setDword(kLastColumn, last + 1);
                g.setDword(kSortColumn, column + 1);
            }
            if (std::int32_t(g.dword(kLastColumn)) > std::int32_t(g.dword(kSortColumn)))
            {
                const reg32 first = g.dword(kFirstColumn);
                if (std::int32_t(first) > 2 && g.dword(kSortColumn) == 1)
                    g.setDword(kSortColumn, first);
                else
                    g.setDword(kSortColumn, g.dword(kSortColumn) + 1);
                headerChanged = true;
            }
            break;
        }
        default:
            break;
        }
        // Its result is what the calls leave in eax: the direct ones' put there.
        if (headerChanged)
            cpu.eax = guest::direct<0x498090, memViewerHeader>(app, cpu);
        if (g.dword(kRedrawAll) == 1)
            cpu.eax = guest::direct<0x498370, memViewerRows>(app, cpu);  // the count in eax too, which it does not read
    }
    return g.eax();
}

void frontend::memViewerSortMode(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_498f00", 0, 0, nullptr, 0, kRestoreAll };
    const reg32 blocks = cpu.eax;
    runWhole(app, cpu, site, [&](Guest&) { return eaa::memViewerSortMode(app, cpu, blocks); });
}

/* sub_499150: qsort's comparison of two entries of the block table, by the sort column,
 * the way the sort goes (0x55e5a8): eax and edx the entries. */
x86::reg32 frontend::eaa::memViewerCompare(App* app, CPU& cpu, reg32 a, reg32 b)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x30);
    const bool ascending = g.dword(kSortAscending) != 0;
    // What is compared, first and then on a tie; columns 5 and 6 compare what the
    // registers held.
    std::int32_t first = std::int32_t(cpu.esi), second = std::int32_t(cpu.ebx);
    std::int32_t tieFirst = 0, tieSecond = 0;
    bool tie = false;
    const reg32 column = g.dword(kSortColumn);
    switch (column)
    {
    case 1:  // by name: free space has none
    {
        const auto nameOf = [&](reg32 block) {
            if ((blockFlags(g, block) & 0x4000) != 0)
                return reg32(0x53c1c8);
            return directKept<0x4e15e0, ea::blockNameOf, 1>(g.app(), g.cpu(), block + 0x10);
        };
        const reg32 nameA = nameOf(g.dword(a));
        const reg32 nameB = nameOf(g.dword(b));
        return fe::direct<0x4ee310, watcom::clib::strcmp>(g.app(), g.cpu(), ascending ? nameA : nameB, ascending ? nameB : nameA);
    }
    case 2:  // by size, then where
    {
        const reg32 blockA = g.dword(a), blockB = g.dword(b);
        const std::int32_t sizeA = std::int32_t(blockSize(g, blockA));
        const std::int32_t sizeB = std::int32_t(blockSize(g, blockB));
        tieSecond = std::int32_t(blockA + 0x10);
        tieFirst = std::int32_t(blockB + 0x10);
        first = ascending ? sizeA : sizeB;
        second = ascending ? sizeB : sizeA;
        tie = true;
        break;
    }
    case 0:
    case 3:  // by where, then size
    {
        const reg32 blockA = g.dword(a), blockB = g.dword(b);
        const std::int32_t whereA = std::int32_t(blockA + 0x10);
        const std::int32_t whereB = std::int32_t(blockB + 0x10);
        tieSecond = std::int32_t(blockSize(g, blockA));
        tieFirst = std::int32_t(blockSize(g, blockB));
        first = ascending ? whereA : whereB;
        second = ascending ? whereB : whereA;
        tie = true;
        break;
    }
    case 5:
    case 6:
        break;
    default:
        first = second = 0;
        break;
    }
    if (first > second)
        return 1;
    if (first != second)
        return reg32(-1);
    if (!tie)
        return 0;
    if (tieFirst < tieSecond)
        return 1;
    return tieFirst == tieSecond ? 0 : reg32(-1);
}

void frontend::memViewerCompare(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_499150", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 a = cpu.eax, b = cpu.edx;
    runWhole(app, cpu, site, [&](Guest&) { return eaa::memViewerCompare(app, cpu, a, b); });
}

/* sub_499330: the pool's numbers: its total, used, free and largest free into what eax,
 * edx, ebx and ecx point at. */
x86::reg32 frontend::eaa::memPoolTotals(App* app, CPU& cpu, reg32 totalAt, reg32 usedAt, reg32 freeAt, reg32 largestAt)
{
    Guest g(app, cpu);
    const reg32 head = firstBlock(g);
    const reg32 pool = g.dword(g.dword(kPoolShown) * 4 + kPools);
    g.setDword(totalAt, g.dword(pool + 0xc) - head);
    reg32 largest = 0, used = 0, available = 0;
    for (reg32 block = firstBlock(g); block != 0; block = g.dword(block + 8))
    {
        const std::uint8_t kind = g.byte(block + 3);
        if ((kind & 0x40) != 0)
        {
            const reg32 size = g.dword(block + 4);
            available += size;
            if (std::int32_t(largest) < std::int32_t(size))
                largest = size;
        }
        else if ((kind & 0x80) == 0)
            used += g.dword(block + 4);
    }
    g.setDword(usedAt, used);
    g.setDword(freeAt, available);
    g.setDword(largestAt, largest);
    g.leaveEbx(used);
    g.leaveEcx(available);
    g.leaveEdx(largest);
    return largestAt;
}

void frontend::memPoolTotals(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_499330", 0, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const reg32 totalAt = cpu.eax, usedAt = cpu.edx, freeAt = cpu.ebx, largestAt = cpu.ecx;
    runWhole(app, cpu, site, [&](Guest&) { return eaa::memPoolTotals(app, cpu, totalAt, usedAt, freeAt, largestAt); });
}

/* sub_4993b0: the first block of the pool shown. */
x86::reg32 frontend::eaa::memPoolFirstBlock(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    return firstBlock(g);
}

void frontend::memPoolFirstBlock(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4993b0", 0, 0, nullptr, 0, 0 };
    runWhole(app, cpu, site, [&](Guest&) { return eaa::memPoolFirstBlock(app, cpu); });
}

/* sub_4993d0: how many blocks of the two kinds the pool has. */
x86::reg32 frontend::eaa::memPoolCountBlocks(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    reg32 count = 0;
    for (reg32 block = firstBlock(g); block != 0; block = g.dword(block + 8))
    {
        const std::uint16_t tag = g.word(block);
        if (tag == kUsedBlock || tag == kFreeBlock)
            ++count;
    }
    return count;
}

void frontend::memPoolCountBlocks(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4993d0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEdx };
    runWhole(app, cpu, site, [&](Guest&) { return eaa::memPoolCountBlocks(app, cpu); });
}

/* sub_499400: the next block, or the first if the list is to start again: where it is,
 * its size and flags to what edx, ebx and ecx point at, 0 to the stack's. */
x86::reg32 frontend::eaa::memPoolNextBlock(App* app, CPU& cpu, reg32 blockAt, reg32 sizeAt, reg32 flagsAt, reg32 zeroAt)
{
    Guest g(app, cpu);
    reg32 block = g.dword(kListCursor);
    if (block == 0 || g.dword(kClearOld) != 0)
    {
        block = firstBlock(g);
        g.setDword(kClearOld, 0);
    }
    else
        block = g.dword(block + 8);
    g.setDword(blockAt, block);
    g.setDword(flagsAt, g.word(block + 2));
    g.setDword(zeroAt, 0);
    const reg32 size = g.dword(block + 4);
    g.setDword(sizeAt, size);
    g.setDword(kListCursor, block);
    g.leaveEdx(zeroAt);
    return size;
}

void frontend::memPoolNextBlock(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_499400", 4, 0, nullptr, 0, kRestoreEsi };
    const reg32 blockAt = cpu.edx, sizeAt = cpu.ebx, flagsAt = cpu.ecx;
    const reg32 zeroAt = stackWord(app, cpu, 0);
    runWhole(app, cpu, site, [&](Guest&) { return eaa::memPoolNextBlock(app, cpu, blockAt, sizeAt, flagsAt, zeroAt); });
}

/* sub_499460: a block's size, the selected one's for none. */
x86::reg32 frontend::eaa::viewerBlockSize(App* app, CPU& cpu, reg32 block)
{
    Guest g(app, cpu);
    return blockSize(g, block);
}

void frontend::viewerBlockSize(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_499460", 0, 0, nullptr, 0, kRestoreEdx };
    const reg32 block = cpu.eax;
    runWhole(app, cpu, site, [&](Guest&) { return eaa::viewerBlockSize(app, cpu, block); });
}

/* sub_499480: a block's data. */
x86::reg32 frontend::eaa::memBlockData(App* app, CPU& cpu, reg32 block)
{
    return blockData(block);
}

void frontend::memBlockData(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_499480", 0, 0, nullptr, 0, 0 };
    const reg32 block = cpu.eax;
    runWhole(app, cpu, site, [&](Guest&) { return eaa::memBlockData(app, cpu, block); });
}

/* sub_499490: a block's flags. */
x86::reg32 frontend::eaa::viewerBlockFlags(App* app, CPU& cpu, reg32 block)
{
    Guest g(app, cpu);
    return blockFlags(g, block);
}

void frontend::viewerBlockFlags(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_499490", 0, 0, nullptr, 0, 0 };
    const reg32 block = cpu.eax;
    runWhole(app, cpu, site, [&](Guest&) { return eaa::viewerBlockFlags(app, cpu, block); });
}

/* sub_4994a0: what a listed block is: 0 free, 1 the system's, 2 unknown, 3 a pack of
 * shapes, 4 or 5 unknown high or low. */
x86::reg32 frontend::eaa::memBlockKind(App* app, CPU& cpu, reg32 index)
{
    Guest g(app, cpu);
    const reg32 block = listedBlock(g, index);
    const reg32 flags = blockFlags(g, block);
    const reg32 size = blockSize(g, block);
    if (blockIsFree(g, block))
        return 0;
    if ((flags & 0x8000) != 0)
        return 1;
    if (size == 0)
        return 2;
    const reg32 tag = tagOf(g, block + 0x10);
    if (tag == kShpi || tag == kShpf)
        return 3;
    return (flags & 0x10) != 0 ? 4 : 5;
}

void frontend::memBlockKind(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4994a0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi };
    const reg32 index = cpu.eax;
    runWhole(app, cpu, site, [&](Guest&) { return eaa::memBlockKind(app, cpu, index); });
}

/* sub_499580: 1 if a block is free space. */
x86::reg32 frontend::eaa::memBlockIsFree(App* app, CPU& cpu, reg32 block)
{
    Guest g(app, cpu);
    return blockIsFree(g, block) ? 1 : 0;
}

void frontend::memBlockIsFree(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_499580", 0, 0, nullptr, 0, 0 };
    const reg32 block = cpu.eax;
    runWhole(app, cpu, site, [&](Guest&) { return eaa::memBlockIsFree(app, cpu, block); });
}

/* sub_4995b0: VIEW: the shapes of a pack one at a time, Left and Right, until Escape; or a
 * bitmap, or a window. */
x86::reg32 frontend::eaa::memViewerViewShapes(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x64);
    Console console(g);
    const reg32 fp = g.frame() + 0x4c;
    const reg32 nameOut = fp - 0x4c;
    const reg32 countAt = fp - 0x3c, othersAt = fp - 0x38, packedAt = fp - 0x34, unpackedAt = fp - 0x30;
    for (const reg32 at : { countAt, othersAt, packedAt, unpackedAt })
        g.setDword(at, 0);
    const reg32 block = fe::direct<0x4f46b0, wata::screenPixels>(app, cpu);

    const auto centred = [&](reg32 text, reg32 shorterBy) {
        const reg32 columns = g.dword(kColumnsTotal), rows = g.dword(kRowsTotal);
        console.begin();
        guest::direct<0x4ef1b0, eab::drawClearClip>(app, cpu, guest::direct<0x4ef760, eab::colourPack>(app, cpu, 0xff000000u));
        console.colours(2, 1);
        console.print({ reg32(sreg32(columns - shorterBy) >> 1), reg32(sreg32(rows) >> 1), text });
        console.clear(0, g.dword(kRowsTotal), g.dword(kColumnsTotal), 1, 0xe);
        console.colours(0xd, 0xe);
    };
    const auto shown = [&](reg32 shape, reg32 bits) {
        console.begin();
        const reg32 x = reg32(sreg32(console.metric(2) - highWord(g, shape + 2)) >> 1);
        const reg32 y = reg32(sreg32(console.metric(3) - highWord(g, shape + 4)) >> 1);
        console.drawShape(shape, bits, x, y);
        console.clear(0, g.dword(kRowsTotal), g.dword(kColumnsTotal), 1, 0xe);
        console.colours(0xd, 0xe);
    };

    const reg32 tag = tagOf(g, block);
    if (tag == kShpi || tag == kShpf)
    {
        guest::direct<0x498b70, memViewerCountShapes>(app, cpu, block, countAt, othersAt, packedAt, unpackedAt);
        std::int32_t index = 0;
        bool done = false;
        while (!done)
        {
            index = index % std::int32_t(g.dword(countAt));
            fe::direct<0x4ea665, wina::tableEntry>(app, cpu, block, reg32(index), nameOut);
            const reg32 shape = guest::direct<0x4ef1e0, eab::shapeFileFind>(app, cpu, block, nameOut);
            const reg32 type = g.dword(shape) & 0x7f;
            const reg32 bits = g.byte(type + kShapeBits);
            const auto narrow = [&] { return std::int32_t(g.dword(kColumnsTotal)) < 0x46; };
            // The numbers of a picture, on the line below it.
            const auto pictureLine = [&] {
                const reg32 packedInfo = g.dword(shape + 0xc);
                console.print({ 0, g.dword(kRowsTotal), narrow() ? 0x53c1d4u : 0x53c200u, nameOut,
                                highWord(g, shape + 2), highWord(g, shape + 4), bits, g.dword(shape) & 0xff,
                                reg32(sreg32(packedInfo << 20) >> 20), reg32(sreg32(packedInfo << 4) >> 20),
                                highWord(g, shape + 6), highWord(g, shape + 8) });
            };
            if (type == 0x7c || type == 0x6e || type == 0x6f)
            {
                centred(0x53c2d4, 0x1f);
                if (type == 0x6f)
                    console.print({ 0, g.dword(kRowsTotal), 0x53c314 });
                else if (type == 0x7c)
                    console.print({ 0, g.dword(kRowsTotal), 0x53c2f4 });
                else
                    console.print({ 0, g.dword(kRowsTotal), 0x53c300 });
            }
            else if ((type & 0x70) == 0x20)
            {
                centred(0x53c2b4, 0x1d);
                console.print({ 0, g.dword(kRowsTotal), narrow() ? 0x53c238u : 0x53c260u, nameOut,
                                highWord(g, shape + 2), highWord(g, shape + 4), g.dword(shape) & 0xff,
                                highWord(g, shape + 6), highWord(g, shape + 8) });
            }
            else if ((g.dword(shape) & 0x80) != 0)
            {
                centred(0x53c294, 0x1c);
                pictureLine();
            }
            else
            {
                shown(shape, bits);
                pictureLine();
            }
            console.end();

            const reg32 key = console.key();
            if (key == kKeyLeft)
            {
                --index;
                if (index < 0)
                    index = std::int32_t(g.dword(countAt)) - 1;
            }
            else if (key == kKeyRight)
            {
                ++index;
                if (index >= std::int32_t(g.dword(countAt)))
                    index = 0;
            }
            else
                done = key == kKeyEscape;
        }
    }
    else
    {
        if (guest::direct<0x499d30, memViewerViewBitmap>(app, cpu) == 0)
            guest::direct<0x499b30, memViewerViewWindow>(app, cpu);
        console.key();
    }
    console.begin();
    guest::direct<0x4ef1b0, eab::drawClearClip>(app, cpu, guest::direct<0x4ef760, eab::colourPack>(app, cpu, 0xff000000u));
    return console.end();
}

void frontend::memViewerViewShapes(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4995b0", 0, 0, nullptr, 0, kRestoreAll };
    runWhole(app, cpu, site, [&](Guest&) { return eaa::memViewerViewShapes(app, cpu); });
}

/* sub_499b30: VIEW of a window: its bitmap and the numbers of it, or why not. */
x86::reg32 frontend::eaa::memViewerViewWindow(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x1c);
    Console console(g);
    const reg32 window = fe::direct<0x4f46b0, wata::screenPixels>(app, cpu);
    reg32 valid = g.dword(window) == 0x57494e44 ? 1 : 0;
    if (valid != 0)
        valid = fe::direct<0x4f4a80, wata::windowCheck>(app, cpu, window);

    const auto saying = [&](reg32 text) {
        const reg32 x = g.dword(kColumnsTotal) - g.length(text);
        const reg32 rows = g.dword(kRowsTotal);
        console.begin();
        guest::direct<0x4ef1b0, eab::drawClearClip>(app, cpu, guest::direct<0x4ef760, eab::colourPack>(app, cpu, 0xff000000u));
        console.colours(2, 1);
        console.print({ x >> 1, reg32(sreg32(rows) >> 1), text });
        return console.end();
    };
    if (valid == 0)
        return saying(0x53c31c);
    const reg32 shape = g.dword(window + 0x20);
    const reg32 bits = shapeBits(g, shape);
    if (bits == 0)
        return saying(0x53c350);

    g.leaveEdx(bits);
    console.begin();
    const reg32 across = console.metric(2);
    const reg32 x = reg32(sreg32(across - highWord(g, shape + 2)) >> 1);
    const reg32 down = console.metric(3);
    const reg32 y = reg32(sreg32(down - highWord(g, shape + 4)) >> 1);
    console.drawShape(shape, bits, x, y);
    console.clear(0, g.dword(kRowsTotal), g.dword(kColumnsTotal), 1, 0xe);
    console.colours(0xd, 0xe);
    console.print({ 0, g.dword(kRowsTotal), 0x53c388, g.dword(window + 4), g.dword(window + 8),
                    g.byte(window + 0x1c), g.byte(window + 0x1d),
                    g.byte(window + 0x1e) == 0 ? 0x53c378u : 0x53c380u });
    return console.end();
}

void frontend::memViewerViewWindow(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_499b30", 0, 0, nullptr, 0, kRestoreAll };
    runWhole(app, cpu, site, [&](Guest&) { return eaa::memViewerViewWindow(app, cpu); });
}

/* sub_499d30: VIEW of a bitmap, if the block is the size of one: its bits a pixel back,
 * 0 if it has none. */
x86::reg32 frontend::eaa::memViewerViewBitmap(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x30);
    Console console(g);
    const reg32 shape = fe::direct<0x4f46b0, wata::screenPixels>(app, cpu);
    const reg32 name = directKept<0x4e15e0, ea::blockNameOf, 1>(g.app(), g.cpu(), shape);
    const reg32 size = blockSize(g, shape - 0x10);
    const reg32 bits = shapeBits(g, shape);
    if (bits < 8)
        return 0;
    const reg32 area = highWord(g, shape + 2) * highWord(g, shape + 4) * (bits >> 3);
    if (size != area + 0x10)
        return bits;
    console.begin();
    const reg32 x = reg32(sreg32(console.metric(2) - highWord(g, shape + 2)) >> 1);
    const reg32 y = reg32(sreg32(console.metric(3) - highWord(g, shape + 4)) >> 1);
    console.drawShape(shape, bits, x, y);
    console.clear(0, g.dword(kRowsTotal), g.dword(kColumnsTotal), 1, 0xe);
    console.colours(0xd, 0xe);
    console.print({ 0, g.dword(kRowsTotal), 0x53c3b4, name, highWord(g, shape + 2), highWord(g, shape + 4), bits });
    console.end();
    return bits;
}

void frontend::memViewerViewBitmap(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_499d30", 0, 0, nullptr, 0, kRestoreAll };
    runWhole(app, cpu, site, [&](Guest&) { return eaa::memViewerViewBitmap(app, cpu); });
}

/* sub_499e90: a shape's bits a pixel. */
x86::reg32 frontend::eaa::memShapeBits(App* app, CPU& cpu, reg32 shape)
{
    Guest g(app, cpu);
    return shapeBits(g, shape);
}

void frontend::memShapeBits(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_499e90", 0, 0, nullptr, 0, 0 };
    const reg32 shape = cpu.eax;
    runWhole(app, cpu, site, [&](Guest&) { return eaa::memShapeBits(app, cpu, shape); });
}

/* sub_499ea0: HEX DUMP of the block, by arrows and pages, until any other key. */
x86::reg32 frontend::eaa::memViewerHexDump(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x14);
    Console console(g);
    reg32 address = fe::direct<0x4f46b0, wata::screenPixels>(app, cpu);
    const reg32 perRow = std::int32_t(g.dword(kColumnsTotal)) < 0x50 ? 8 : 0x10;
    guest::direct<0x499f70, memViewerHexPage>(app, cpu, address, perRow);
    bool done = false;
    while (!done)
    {
        const reg32 key = console.key();
        const reg32 page = g.dword(kRowsTotal) * perRow;
        switch (key)
        {
        case kKeyUp:
            address -= perRow;
            break;
        case kKeyDown:
            address += perRow;
            break;
        case kKeyPageUp:
            address -= page;
            break;
        case kKeyPageDown:
            address += page;
            break;
        default:
            done = true;
            break;
        }
        guest::direct<0x499f70, memViewerHexPage>(app, cpu, address, perRow);
    }
    console.begin();
    console.clear(0, 0, g.dword(kColumnsTotal), g.dword(kRowsTotal) + 1, 1);
    return console.end();
}

void frontend::memViewerHexDump(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_499ea0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi };
    runWhole(app, cpu, site, [&](Guest&) { return eaa::memViewerHexDump(app, cpu); });
}

/* sub_499f70: a screenful of the dump: eax the address, edx the bytes a row. */
x86::reg32 frontend::eaa::memViewerHexPage(App* app, CPU& cpu, reg32 base, reg32 perRow)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x120);
    Console console(g);
    const reg32 fp = g.frame() + 0x10c - 0x82;
    const reg32 line = fp - 0x8a, piece = fp - 0xa;
    if (base == 0)
        return 0;
    reg32 at = base;
    console.begin();
    console.clear(0, 0, g.dword(kColumnsTotal), g.dword(kRowsTotal) + 1, 1);
    console.colours(0, 1);
    reg32 row = 0;
    for (; std::int32_t(row) < std::int32_t(g.dword(kRowsTotal)); ++row)
    {
        sprintf1(g, line, perRow == 8 ? 0x53c3dc : 0x53c3d4, perRow * row + base);
        for (std::int32_t i = 0; i < std::int32_t(perRow); ++i)
        {
            const reg32 value = g.byte(at);
            ++at;
            sprintf1(g, piece, 0x53c3e4, value);
            append(g, line, piece);
        }
        append(g, line, kSpaceText);
        at -= perRow;
        for (std::int32_t i = 0; i < std::int32_t(perRow); ++i)
        {
            std::uint8_t c = g.byte(at);
            ++at;
            if (c < 0x20 || c > 0x7e)
                c = 0x2e;
            sprintf1(g, piece, 0x53c3ec, c);
            append(g, line, piece);
        }
        console.print({ 0, row, line });
    }
    g.leaveEdx(row);
    return console.end();
}

void frontend::memViewerHexPage(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_499f70", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 base = cpu.eax, perRow = cpu.edx;
    runWhole(app, cpu, site, [&](Guest&) { return eaa::memViewerHexPage(app, cpu, base, perRow); });
}

/* sub_49a120: P: the pool's numbers and blocks into a file of a free name. */
x86::reg32 frontend::eaa::memViewerWriteMap(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x37c);
    Console console(g);
    constexpr reg32 kFprintf = 0x4f4b80;
    const reg32 fp = g.frame() + 0x364 - 0x82;
    const reg32 name = fp - 0xb6, line = fp - 0x1b6, numbers = fp - 0x36;
    const reg32 dash = fp - 0x2e2, flagText = fp + 0x56, kindText = fp + 0x72, kinds = fp + 0x4a;
    for (reg32 k = 0; k < 3; ++k)
        g.setDword(kinds + k * 4, g.dword(0x497610 + k * 4));
    g.copyString(name, 0x53c3f0);
    guest::direct<0x49a400, memViewerFreeFileName>(app, cpu, name);  // 0x53c400 in edx too, which it does not read
    const reg32 file = fe::direct<0x4edfe8, watcom::clib::fopen>(g.app(), g.cpu(), name, 0x53c400);
    if (file == 0)
    {
        console.print({ 0, 0, 0x53c564 });
        return fe::direct<0x4ee100, watcom::clib::fclose>(g.app(), g.cpu(), file);
    }
    // Where they are matters: what a free block's kind has after its one letter runs on
    // into these, and the file prints it so.
    g.setDword(fp + 0x76, file);
    guest::direct<0x499330, memPoolTotals>(app, cpu, fp + 0x62, fp + 0x66, fp + 0x6a, fp + 0x6e);
    const auto hex = [&] { return g.dword(kHexNumbers) != 0; };
    fe::direct<kFprintf, watcom::clib::fprintf>(g.app(), g.cpu(), file, hex() ? 0x53c404u : 0x53c41cu, guest::Words{g.dword(fp + 0x62)});
    fe::direct<kFprintf, watcom::clib::fprintf>(g.app(), g.cpu(), file, hex() ? 0x53c434u : 0x53c44cu, guest::Words{g.dword(fp + 0x66)});
    fe::direct<kFprintf, watcom::clib::fprintf>(g.app(), g.cpu(), file, hex() ? 0x53c464u : 0x53c47cu, guest::Words{g.dword(fp + 0x6a)});
    fe::direct<kFprintf, watcom::clib::fprintf>(g.app(), g.cpu(), file, hex() ? 0x53c494u : 0x53c4b4u, guest::Words{g.dword(fp + 0x6e)});
    fe::direct<kFprintf, watcom::clib::fprintf>(g.app(), g.cpu(), file, 0x53c4d4, guest::Words({}));
    for (reg32 i = 0; std::int32_t(i) < std::int32_t(g.dword(kListed)); ++i)
    {
        g.setByte(dash, 0x2d);
        g.setByte(dash + 1, 0);
        const reg32 block = listedBlock(g, i);
        const reg32 size = blockSize(g, block);
        const reg32 data = blockData(block);
        reg32 owner = 0;
        g.setDword(fp + 0x7a, 0);
        guest::direct<0x498e40, memViewerFlagText>(app, cpu, blockFlags(g, block), flagText);
        if (blockIsFree(g, block))
        {
            g.setByte(kindText, 0x20);
            fe::direct<kSprintf, watcom::clib::sprintf>(g.app(), g.cpu(), line, 0x53c504, guest::Words({}));
        }
        else
        {
            const reg32 entry = g.dword(kBlockTable) + i * 8;
            owner = g.dword(entry + 4);
            // The game's code hands it the owner in edx too, which it does not read.
            const reg32 kind = fe::direct<0x4f47c0, wata::shapeEndsAtBend>(app, cpu, g.dword(entry));
            sprintf1(g, kindText, kThreeLetters, g.dword(kinds + kind * 4));
            const reg32 blockName = directKept<0x4e15e0, ea::blockNameOf, 1>(g.app(), g.cpu(), listedBlock(g, i) + 0x10);
            fe::direct<kSprintf, watcom::clib::sprintf>(g.app(), g.cpu(), line, 0x53c518, guest::Words{0, blockName});
        }
        fe::direct<kSprintf, watcom::clib::sprintf>(g.app(), g.cpu(), numbers, hex() ? 0x53c524u : 0x53c544u, guest::Words{size, data, flagText, dash, 0, owner, kindText});
        append(g, line, numbers);
        fe::direct<kFprintf, watcom::clib::fprintf>(g.app(), g.cpu(), file, line, guest::Words({}));
    }
    return fe::direct<0x4ee100, watcom::clib::fclose>(g.app(), g.cpu(), file);
}

void frontend::memViewerWriteMap(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_49a120", 0, 0, nullptr, 0, kRestoreAll };
    runWhole(app, cpu, site, [&](Guest&) { return eaa::memViewerWriteMap(app, cpu); });
}

/* sub_49a400: the file's name free: its ?? become the first two digits no file has, or
 * xx if there is none. */
x86::reg32 frontend::eaa::memViewerFreeFileName(App* app, CPU& cpu, reg32 name)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x1c);
    reg32 place = 0;
    while (g.byte(name + place) != 0x3f)
        ++place;
    std::int32_t number = 0;
    for (; number < 100; ++number)
    {
        g.setByte(name + place, std::uint8_t(number / 10 + 0x30));
        g.setByte(name + place + 1, std::uint8_t(number % 10 + 0x30));
        if (!*fe::direct<0x4e0e60, ea::fileThere>(app, cpu, name))
            break;
    }
    if (number >= 100)
    {
        g.setByte(name + place + 1, 0x78);
        g.setByte(name + place, g.byte(name + place + 1));
    }
    return 1;
}

void frontend::memViewerFreeFileName(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_49a400", 0, 0, nullptr, 0, kRestoreAll };
    const reg32 name = cpu.eax;
    runWhole(app, cpu, site, [&](Guest&) { return eaa::memViewerFreeFileName(app, cpu, name); });
}

namespace
{

/* What sub_4d0d50 copies of a glyph (sub_4efa70) into a font's tables. */
constexpr reg32 kFontSmall = 0x840188;     // the small screen's font: a second one, nudged
constexpr reg32 kFontNumber = 0x840184;
constexpr reg32 kFontScale = 0x84018c;
constexpr reg32 kFontShapes = 0x83f6a0;
constexpr reg32 kLoadedShapes = 0x567218;
constexpr reg32 kPictureTexture = 0x4d0a10;

}  // namespace

/* sub_4d0d50: the text's fonts: their files by the screen's width, the widths and places
 * of their glyphs into the tables at 0x83f4e0 and on, and each made a texture. */
x86::reg32 frontend::eaa::textLoadFonts(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x21c);
    const reg32 fp = g.frame() + 0x204 - 0x182;
    const reg32 firstName = fp - 0x82, secondName = fp + 0x7e;
    constexpr reg32 kFolder = 0x7a29b4;
    if (std::int32_t(g.dword(0x7cdac8)) >= 0x280)
    {
        fe::direct<kSprintf, watcom::clib::sprintf>(g.app(), g.cpu(), firstName, 0x540b94, guest::Words{kFolder});
        g.setDword(kFontSmall, 0);
        g.setByte(secondName, 0);
    }
    else
    {
        fe::direct<kSprintf, watcom::clib::sprintf>(g.app(), g.cpu(), firstName, 0x540ba4, guest::Words{kFolder});
        fe::direct<kSprintf, watcom::clib::sprintf>(g.app(), g.cpu(), secondName, 0x540bb0, guest::Words{kFolder});
        g.setDword(kFontSmall, 1);
    }
    if (!*fe::direct<0x4e0e60, ea::fileThere>(app, cpu, firstName))
        return 0;
    if (g.byte(secondName) != 0 && !*fe::direct<0x4e0e60, ea::fileThere>(app, cpu, secondName))
        return 0;

    const reg32 first = *fe::direct<0x4e0ec0, ea::loadWhole>(app, cpu, firstName, 0x10u);
    guest::direct<0x4ef520, eab::fontUse>(app, cpu, first);
    const reg32 shapes = g.dword(kLoadedShapes);
    g.setDword(kFontNumber, 0);
    g.setDword(kFontScale, 0x3f800000);
    g.setDword(kFontShapes, shapes);
    for (reg32 i = 0; i < 0xe0; ++i)
    {
        const reg32 character = i + 0x20;
        const reg32 glyph = guest::direct<0x4efa70, eab::fontGlyphFind>(app, cpu, character);
        if (glyph == 0)
            continue;
        g.setByte(i + 0x83fc40, g.byte(glyph + 2));
        g.setByte(i + 0x83fe00, g.byte(glyph + 3));
        g.setByte(i + 0x83f8c0, g.byte(glyph + 4));
        g.setByte(i + 0x83f700, g.byte(glyph + 6));
        std::uint8_t advance = g.byte(glyph + 8);
        if (g.dword(kFontSmall) == 0)
            ++advance;
        else if (character == 0x50 || character == 0x4d || character == 0x20 || character == 0x2f)
            --advance;
        g.setByte(i + 0x83f4e0, advance);
        g.setByte(i + 0x83ffc0, g.byte(glyph + 9));
        g.setByte(i + 0x83fa80, g.byte(glyph + 0xa));
    }
    g.call(kPictureTexture);
    reg32 result = *fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, first);
    if (g.byte(secondName) == 0)
        return result;

    g.setDword(kFontNumber, 1);
    const reg32 second = *fe::direct<0x4e0ec0, ea::loadWhole>(app, cpu, secondName, 0x10u);
    guest::direct<0x4ef520, eab::fontUse>(app, cpu, second);
    g.setDword(kFontShapes + 4, g.dword(kLoadedShapes));
    for (reg32 i = 0; i < 0xe0; ++i)
    {
        const reg32 glyph = guest::direct<0x4efa70, eab::fontGlyphFind>(app, cpu, i + 0x20);
        if (glyph == 0)
            continue;
        g.setByte(i + 0x83fd20, g.byte(glyph + 2));
        g.setByte(i + 0x83fee0, g.byte(glyph + 3));
        g.setByte(i + 0x83f9a0, g.byte(glyph + 4));
        g.setByte(i + 0x83f7e0, g.byte(glyph + 6));
        g.setByte(i + 0x83f5c0, g.byte(glyph + 8));
        g.setByte(i + 0x8400a0, g.byte(glyph + 9));
        g.setByte(i + 0x83fb60, g.byte(glyph + 0xa));
    }
    g.call(kPictureTexture);
    result = *fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, second);
    g.setDword(kFontNumber, 0);
    return result;
}

void frontend::textLoadFonts(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4d0d50", 0, 0, nullptr, 0, kRestoreAll };
    runWhole(app, cpu, site, [&](Guest&) { return eaa::textLoadFonts(app, cpu); });
}

/* sub_4dc0e0: a udff opened on a file named in eax or the memory at edx, one and not both;
 * ebx whether what goes wrong is an error: a record of 12 bytes, or none. */
x86::reg32 frontend::eaa::udffOpen(App* app, CPU& cpu, reg32 name, reg32 memory, reg32 complain)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x1c);
    const reg32 fp = g.frame() + 0xc;
    // By address: what it leaves in ebx and edx is what this function leaves there.
    const reg32 record = g.call(kReserve, args({ 0x549734, 0xc, 0x10 }));
    if (record == 0)
    {
        if (complain != 0)
            fe::direct<0x401010, ea::error>(app, cpu, 0x549738u, guest::Words({}));
        return 0;
    }
    if ((name == 0) == (memory == 0))
    {
        fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, record);
        if (complain != 0)
            fe::direct<0x401010, ea::error>(app, cpu, 0x549758u, guest::Words({}));
        return 0;
    }
    if (memory != 0)
    {
        g.setDword(record + 4, 0);
        g.setDword(record, 1);
        g.setDword(record + 8, memory);
        return record;
    }
    g.setDword(fp - 0xc, 0);
    g.setDword(fp - 8, 0);
    g.setDword(record, 0);
    g.setDword(record + 8, 0);
    g.call(0x4e28d0, args({ name, record + 4, fp - 0xc, fp - 8 }));  // by address: its ebx is this one's
    g.leaveEdx(g.dword(record + 4));
    if (g.dword(record + 4) != 0)
        return record;
    fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, record);
    return 0;
}

void frontend::udffOpen(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4dc0e0", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 name = cpu.eax, memory = cpu.edx, complain = cpu.ebx;
    runWhole(app, cpu, site, [&](Guest&) { return eaa::udffOpen(app, cpu, name, memory, complain); });
}

/* sub_4dc1b0: a udff closed: its file, if it has one, and its record. */
x86::reg32 frontend::eaa::udffClose(App* app, CPU& cpu, reg32 record)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0xc);
    if (record == 0)
        return 0;
    if (g.dword(record) == 0)
    {
        const reg32 handle = g.dword(record + 4);
        if (handle != 0)
            fe::direct<0x4e2da0, ea::closeChecked>(app, cpu, handle);
    }
    return *fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, record);
}

void frontend::udffClose(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4dc1b0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEdx };
    const reg32 record = cpu.eax;
    runWhole(app, cpu, site, [&](Guest&) { return eaa::udffClose(app, cpu, record); });
}

/* sub_4dc1e0: the next word of a udff. */
x86::reg32 frontend::eaa::udffReadWord(App* app, CPU& cpu, reg32 record)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x10);
    const reg32 word = g.frame();
    if (g.dword(record) == 0)
    {
        // By address: the registers it leaves are what sub_4f2a7d hands on to exit.
        if (g.call(0x4e2de0, args({ g.dword(record + 4), word, 4 })) == 0)
            guest::direct<0x4f2a7d, eab::exitGameNow>(app, cpu);
    }
    else
    {
        g.setDword(word, g.dword(g.dword(record + 8)));
        g.setDword(record + 8, g.dword(record + 8) + 4);
    }
    return g.dword(word);
}

void frontend::udffReadWord(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4dc1e0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEdx };
    const reg32 record = cpu.eax;
    runWhole(app, cpu, site, [&](Guest&) { return eaa::udffReadWord(app, cpu, record); });
}

/* sub_4e4360: a file's size, the way of opening it in edx. */
x86::reg32 frontend::eaa::fileSizeOpened(App* app, CPU& cpu, reg32 name, reg32 mode)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x14);
    const reg32 handleAt = g.frame(), sizeAt = g.frame() + 8;
    g.call(0x4e2ab0, args({ name, handleAt, g.frame() + 4, sizeAt }, { mode }));
    const reg32 handle = g.dword(handleAt);
    g.leaveEdx(handle);
    if (handle != 0)
        fe::direct<0x4e2da0, ea::closeChecked>(app, cpu, handle);
    return g.dword(sizeAt);
}

void frontend::fileSizeOpened(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4e4360", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx };
    const reg32 name = cpu.eax, mode = cpu.edx;
    runWhole(app, cpu, site, [&](Guest&) { return eaa::fileSizeOpened(app, cpu, name, mode); });
}

/* sub_4e43a0: a file's size. */
x86::reg32 frontend::eaa::fileSize(App* app, CPU& cpu, reg32 name)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 4);
    return guest::direct<0x4e4360, fileSizeOpened>(app, cpu, name, 1u);
}

void frontend::fileSize(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4e43a0", 0, 0, nullptr, 0, kRestoreEdx };
    const reg32 name = cpu.eax;
    runWhole(app, cpu, site, [&](Guest&) { return eaa::fileSize(app, cpu, name); });
}

namespace
{

constexpr reg32 kDisplay = 0x9f38d0;       // the display's record
constexpr reg32 kDrawTarget = 0x564ff4;    // the bitmap being drawn into, 14 words
constexpr reg32 kScreenBitmap = 0x565048;
constexpr reg32 kStandInBitmap = 0x5643f0;
constexpr reg32 kDrawLeft = 0x565000, kDrawTop = 0x565004, kDrawRight = 0x565008, kDrawBottom = 0x56500c;
constexpr reg32 kDrawBits = 0x565010;
constexpr reg32 kDrawHeight = 0x564ffc;
constexpr reg32 kDrawCallback = 0x567dcc;

void copyWords(Guest& g, reg32 to, reg32 from, reg32 words)
{
    for (reg32 i = 0; i < words; ++i)
        g.setDword(to + i * 4, g.dword(from + i * 4));
}

/* What sub_4eada0 is: drawing goes into the bitmap in eax, or the screen for none. */
reg32 drawInto(Guest& g, x86::CPU& cpu, reg32 bitmap)
{
    reg32 result = 1;
    if (bitmap != 0)
        result = fe::direct<0x4f4a80, frontend::wata::windowCheck>(g.app(), cpu, bitmap);
    if (result == 0)
        return 0;
    if (const reg32 callback = g.dword(kDrawCallback))
        result = g.call(callback, args({ bitmap }));
    else if (bitmap == 0)
        guest::direct<0x4ed2a0, frontend::eab::ddrawUnlockBitmap>(g.app(), cpu, kDisplay);
    else
        result = guest::direct<0x4ed0f0, frontend::eab::ddrawLockBitmap>(g.app(), cpu, kDisplay, g.dword(bitmap + 0x34),
                                                                         bitmap);
    if (bitmap == 0)
    {
        fe::direct<0x4e070c, ea::clearBytes>(g.app(), g.cpu(), kDrawTarget, 0x38);
        g.setByte(kDrawBits, g.byte(kScreenBitmap));
        g.setByte(kDrawBits + 1, g.byte(kScreenBitmap + 1));
        g.setByte(kDrawBits + 2, g.byte(kScreenBitmap + 2));
        g.setDword(0x565024, g.dword(0x56505c));
        return result;
    }
    if (result != 0)
    {
        copyWords(g, kDrawTarget, bitmap, 14);
        return result;
    }
    // The bitmap could not be had: a little one instead, made once.
    if (g.dword(kStandInBitmap) == 0)
        g.setDword(kStandInBitmap, guest::direct<0x4ef190, frontend::eab::windowBitmapCreateDefault>(
                                       g.app(), cpu, 8u, 8u, g.dword(0x56438c), g.dword(0x5643f4)));
    copyWords(g, kDrawTarget, g.dword(kStandInBitmap), 14);
    return result;
}

}  // namespace

/* sub_4ead90: drawing goes to the screen. */
x86::reg32 frontend::eaa::bitmapDrawToScreen(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x18);
    return drawInto(g, cpu, 0);
}

void frontend::bitmapDrawToScreen(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4ead90", 0, 0, nullptr, 0, kRestoreAll };
    runWhole(app, cpu, site, [&](Guest&) { return eaa::bitmapDrawToScreen(app, cpu); });
}

/* sub_4eada0: drawing goes into a bitmap. */
x86::reg32 frontend::eaa::bitmapDrawInto(App* app, CPU& cpu, reg32 bitmap)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x18);
    return drawInto(g, cpu, bitmap);
}

void frontend::bitmapDrawInto(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4eada0", 0, 0, nullptr, 0, kRestoreAll };
    const reg32 bitmap = cpu.eax;
    runWhole(app, cpu, site, [&](Guest&) { return eaa::bitmapDrawInto(app, cpu, bitmap); });
}

/* sub_4eaeb0: pixels read off what is drawn into, at their width: eax, edx the left and
 * top, ebx, ecx the size, the stack's where to. */
x86::reg32 frontend::eaa::bitmapReadPixels(App* app, CPU& cpu, reg32 left, reg32 y, reg32 width, reg32 rows, reg32 out)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x14);
    const reg32 bits = (reg32(g.byte(kDrawBits)) + 7) & ~reg32(7);
    const reg32 right = left + width;
    g.leaveEbx(bits);
    for (--rows; rows != reg32(-1); --rows, ++y)
    {
        for (reg32 x = left; std::int32_t(x) < std::int32_t(right); ++x)
        {
            const reg32 pixel = fe::direct<0x503bc0, watb::readPixel>(app, cpu, x, y);
            if (bits == 8)
            {
                g.setByte(out, std::uint8_t(pixel));
                ++out;
            }
            else if (bits == 0x10)
            {
                g.setWord(out, std::uint16_t(pixel));
                out += 2;
            }
            else if (bits == 0x18)
            {
                g.setWord(out, std::uint16_t(pixel));
                g.setByte(out + 2, std::uint8_t(pixel >> 16));
                out += 3;
            }
            else if (bits == 0x20)
            {
                g.setDword(out, pixel);
                out += 4;
            }
        }
    }
    g.leaveEcx(out);
    g.leaveEdx(reg32(-1));
    return right;
}

void frontend::bitmapReadPixels(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4eaeb0", 4, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const reg32 left = cpu.eax, y = cpu.edx, width = cpu.ebx, rows = cpu.ecx;
    const reg32 out = stackWord(app, cpu, 0);
    runWhole(app, cpu, site, [&](Guest&) { return eaa::bitmapReadPixels(app, cpu, left, y, width, rows, out); });
}

/* sub_4eaf60: the size of a TGA of what is drawn into. */
x86::reg32 frontend::eaa::bitmapTgaSize(App* app, CPU& cpu)
{
    Guest g(app, cpu);
    const reg32 bits = (reg32(g.byte(kDrawBits)) + 7) & ~reg32(7);
    const reg32 width = g.dword(kDrawRight) - g.dword(kDrawLeft);
    const reg32 height = g.dword(kDrawBottom) - g.dword(kDrawTop);
    const reg32 pixelBits = width * height * bits;
    // A signed division by 8, towards zero.
    const reg32 rounded = pixelBits + (std::int32_t(pixelBits) < 0 ? 7 : 0);
    reg32 size = reg32(sreg32(rounded) >> 3) + 0x12;
    if (bits == 8)
        size += 0x300;
    return size;
}

void frontend::bitmapTgaSize(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4eaf60", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx };
    runWhole(app, cpu, site, [&](Guest&) { return eaa::bitmapTgaSize(app, cpu); });
}

/* sub_4eafc0: a TGA of what is drawn into, at eax: its header, its palette, its pixels. */
x86::reg32 frontend::eaa::bitmapWriteTga(App* app, CPU& cpu, reg32 tga)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x320);
    const reg32 palette = g.frame();
    const reg32 width = g.dword(kDrawRight) - g.dword(kDrawLeft);
    const reg32 height = g.dword(kDrawBottom) - g.dword(kDrawTop);
    const reg32 bits = (reg32(g.byte(kDrawBits)) + 7) & ~reg32(7);
    const bool paletted = bits == 8;
    g.setByte(tga, 0);
    g.setByte(tga + 1, paletted ? 1 : 0);
    g.setWord(tga + 3, 0);
    g.setByte(tga + 2, paletted ? 1 : 2);
    g.setWord(tga + 5, paletted ? 0x100 : 0);
    g.setByte(tga + 0x11, 0x20);
    g.setByte(tga + 7, paletted ? 0x18 : 0);
    g.setWord(tga + 8, g.word(kDrawLeft));
    g.setByte(tga + 0x10, std::uint8_t(bits));
    g.setWord(tga + 0xa, std::uint16_t(g.word(kDrawHeight) - g.word(kDrawBottom)));
    g.setWord(tga + 0xc, std::uint16_t(width));
    g.setWord(tga + 0xe, std::uint16_t(height));
    reg32 out = tga + 0x12;
    if (paletted)
    {
        fe::direct<0x503ec0, wina::paletteGet>(app, cpu, 0u, 0x100u, palette);
        for (reg32 i = 0; i < 0x300; i += 3)
        {
            g.setByte(out, g.byte(palette + i + 2));
            g.setByte(out + 1, g.byte(palette + i + 1));
            g.setByte(out + 2, g.byte(palette + i));
            out += 3;
        }
    }
    const reg32 top = g.dword(kDrawTop);
    reg32 result = guest::direct<0x4eaeb0, bitmapReadPixels>(app, cpu, g.dword(kDrawLeft), top,
                                                             g.dword(kDrawRight) - g.dword(kDrawLeft),
                                                             g.dword(kDrawBottom) - top, out);
    if (g.byte(kDrawBits) == 0x10)
        result = fe::direct<0x5040b8, rendf::row565To1555>(app, cpu, out, out, width * height);
    return result;
}

void frontend::bitmapWriteTga(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4eafc0", 0, 0, nullptr, 0, kRestoreAll };
    const reg32 tga = cpu.eax;
    runWhole(app, cpu, site, [&](Guest&) { return eaa::bitmapWriteTga(app, cpu, tga); });
}

/* sub_4eb160: what is drawn into saved as a TGA to the file named in eax; the file's
 * save's answer, 0 without the memory for it. */
x86::reg32 frontend::eaa::bitmapSaveTga(App* app, CPU& cpu, reg32 name)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x84);
    const reg32 saved = g.frame();
    const reg32 size = guest::direct<0x4eaf60, bitmapTgaSize>(app, cpu);
    g.leaveEbx(size);
    const reg32 buffer = g.call(0x4e19d0, args({}, { size }));
    if (buffer == 0)
        return 0;
    g.leaveEcx(buffer);
    guest::direct<0x4eafc0, bitmapWriteTga>(app, cpu, buffer);
    fe::direct<0x4f46d0, wata::windowSave>(app, cpu, saved);
    guest::direct<0x4ead90, bitmapDrawToScreen>(app, cpu);
    const reg32 result = g.call(0x4eeae0, args({ name, buffer }));
    fe::direct<0x4f4700, wata::windowRestore>(app, cpu, saved);
    g.call(0x4e19f0, args({}, { buffer }));
    return result;
}

void frontend::bitmapSaveTga(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4eb160", 0, 0, nullptr, 0, kRestoreAll };
    const reg32 name = cpu.eax;
    runWhole(app, cpu, site, [&](Guest&) { return eaa::bitmapSaveTga(app, cpu, name); });
}

/* sub_4eb1c0: a key's state, by its number. */
x86::reg32 frontend::eaa::keyStateByte(App* app, CPU& cpu, reg32 key)
{
    Guest g(app, cpu);
    return g.byte(key + 0x564818);
}

void frontend::keyStateByte(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4eb1c0", 0, 0, nullptr, 0, 0 };
    const reg32 key = cpu.eax;
    runWhole(app, cpu, site, [&](Guest&) { return eaa::keyStateByte(app, cpu, key); });
}

namespace
{

/* The library's log of DirectDraw. */
reg32 report(Guest& g, reg32 level, reg32 text, std::initializer_list<reg32> values = {})
{
    // eax as the call by address leaves it: the report's answer (it changes no other register).
    return g.cpu().eax = *fe::direct<0x501050, ea::reportedAt>(g.app(), g.cpu(), 0x564ed4u, level, text, guest::Words(values));
}

/* A method of a DirectDraw object, by its offset in the object's table, its `this` first on the
 * stack: the host's method called as C++ where the table has it there, else, and in the check,
 * through the table (host_video.h, guest::ddraw).  Terminated when the game stopped in it, as
 * g.call. */
template <reg32 Offset, auto Method, typename... Words>
reg32 method(Guest& g, x86::CPU& cpu, reg32 object, Words... words)
{
    const std::optional<reg32> eax = guest::ddraw<Offset, Method>(g.app(), cpu, object, words...);
    if (!eax)
        throw fe::Terminated();
    return *eax;
}

/* A method of a COM object through its table, its `this` first on the stack: one the host
 * answers only through the table (IDirectDraw2's CreateClipper, host_video.cpp, and the
 * clipper's). */
reg32 tableMethod(Guest& g, reg32 object, reg32 slot, std::initializer_list<reg32> values = {})
{
    Arguments arguments;
    arguments.stack[arguments.stackCount++] = object;
    for (const reg32 value : values)
        arguments.stack[arguments.stackCount++] = value;
    return g.call(g.dword(g.dword(object) + slot), arguments);
}

/* A function of Windows', its words handed: the port's own, called as C++ (native_windows.h); in
 * the check through its slot in the imports.  Terminated when the game stopped in it, as g.call. */
template <reg32 Slot, auto Function, typename... Words>
reg32 windows(Guest& g, x86::CPU& cpu, Words... words)
{
    const std::optional<reg32> eax = guest::windows<Slot, Function>(g.app(), cpu, words...);
    if (!eax)
        throw fe::Terminated();
    return *eax;
}

/* A function of Windows' the code calls through its import thunk: windows, but in the check by
 * the thunk's address (native_windows.h, guest::thunked). */
template <reg32 Thunk, auto Function, typename... Words>
reg32 thunked(Guest& g, x86::CPU& cpu, Words... words)
{
    const std::optional<reg32> eax = guest::thunked<Thunk, Function>(g.app(), cpu, words...);
    if (!eax)
        throw fe::Terminated();
    return *eax;
}

// DirectDrawCreate's import thunk (native_import_thunks.inc).
constexpr reg32 kDirectDrawCreate = 0x533e86;

// DirectDraw's methods, by their offset in an object's table (host_video.h): IUnknown's,
// IDirectDraw2's, IDirectDrawSurface's, IDirectDrawPalette's, IDirectDrawClipper's.
constexpr reg32 kQueryInterface = 0x00, kRelease = 0x08;
constexpr reg32 kCreateClipper = 0x10, kCreatePalette = 0x14, kEnumDisplayModes = 0x20, kFlipToGDISurface = 0x28;
constexpr reg32 kGetCaps = 0x2c, kSetCooperativeLevel = 0x50;
constexpr reg32 kGetSurfaceDesc = 0x58, kIsLost = 0x60, kLock = 0x64, kRestore = 0x6c, kSetClipper = 0x70;
constexpr reg32 kSetPalette = 0x7c;
constexpr reg32 kSetEntries = 0x18;
constexpr reg32 kSetHWnd = 0x20;

constexpr reg32 kDisplayLock = 0x564480;
constexpr reg32 kEnterCriticalSection = 0x5344b8, kLeaveCriticalSection = 0x534588;
constexpr reg32 kDisplayError = 0x56825c;
constexpr reg32 kModeCount = 0x9f09a0, kModesDefault = 0x9f09a8;
constexpr reg32 kCapabilities = 0x564ed8;
constexpr reg32 kModesCallback = 0x4edbc0;
constexpr reg32 kDisplayChanged = 0x567184;

// Offsets in the display's record.
constexpr reg32 kClipper = 0x4, kModesRoom = 0x8, kModeIndex = 0xc, kModes = 0x10, kWidth = 0x14, kHeight = 0x18;
constexpr reg32 kBitsWanted = 0x1c, kPalette = 0x24, kPaletteEntries = 0x30;
constexpr reg32 kPrimary = 0x444, kBack = 0x448, kPrimaryWindow = 0x44c, kBackWindow = 0x450, kWindow = 0x454;
constexpr reg32 kBits = 0x45c, kFullscreen = 0x45d, kModeX = 0x45f, kPages = 0x463;

}  // namespace

/* sub_4eb430: DirectDrawCreate for the driver edx names; the object, and how many have
 * been tried, told to the log. */
x86::reg32 frontend::eaa::ddrawCreate(App* app, CPU& cpu, reg32 driver)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x10);
    const reg32 object = g.frame();
    g.leaveEdx(0);
    g.setDword(object, 0);
    const reg32 error =
        thunked<kDirectDrawCreate, &host::ddraw::directDrawCreate>(g, cpu, driver, object, 0) & 0xffff;
    const reg32 tries = g.dword(0x9f3d3c) + 1;
    g.setDword(0x9f3d3c, tries);
    report(g, 0xd, 0x54b73c, { driver, g.dword(object), tries, error });
    return g.dword(object);
}

void frontend::ddrawCreate(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4eb430", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdi };
    const reg32 driver = cpu.edx;
    runWhole(app, cpu, site, [&](Guest&) { return eaa::ddrawCreate(app, cpu, driver); });
}

/* sub_4eb490: the primary surface of a display, as asked in edx, else whatever will do. */
x86::reg32 frontend::eaa::ddrawMakePrimary(App* app, CPU& cpu, reg32 display, reg32 wanted)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x1c);
    const auto make = [&](reg32 backs, reg32 flags) {
        return fe::direct<0x4ed480, menz::ddrawMakeSurface>(app, cpu, display, g.dword(display + kWidth),
                                                            g.dword(display + kHeight), g.dword(display + kBitsWanted),
                                                            backs, 0u, flags, g.dword(display + kPalette));
    };
    const auto backs = [&] { return reg32(g.byte(display + kPages)) - 1; };
    report(g, 0xe, 0x54b77c);
    reg32 surface = make(backs(), wanted);
    if (surface != 0)
        return surface;
    reg32 flags = wanted & ~reg32(0x4800);
    report(g, 0xe, 0x54b7a8);
    surface = make(backs(), flags);
    if (surface == 0)
    {
        flags |= (wanted & 0x4000) != 0 ? 0x800 : 0x4000;
        report(g, 0xe, 0x54b7cc);
        surface = make(backs(), flags);
    }
    if (surface != 0 || g.byte(display + kPages) <= 1)
        return surface;
    report(g, 0xe, 0x54b7fc);
    flags &= ~reg32(0x4800);
    g.setByte(display + kPages, 1);
    return make(0, flags);
}

void frontend::ddrawMakePrimary(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4eb490", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 display = cpu.eax, wanted = cpu.edx;
    runWhole(app, cpu, site, [&](Guest&) { return eaa::ddrawMakePrimary(app, cpu, display, wanted); });
}

/* sub_4eb5e0: a surface locked: eax the display, edx the surface, ebx its description.
 * A surface that is lost is restored, with the others, the whole display made again if
 * that fails.  The error back. */
x86::reg32 frontend::eaa::ddrawLockSurface(App* app, CPU& cpu, reg32 display, reg32 surface, reg32 description)
{
    constexpr reg32 kSurfaceLost = 0x1c2;
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x10);
    const auto lock = [&] {
        g.setDword(description, 0x6c);
        return method<kLock, &host::ddraw::surfaceLock>(g, cpu, surface, 0, description, 1, 0) & 0xffff;
    };
    reg32 error = lock();
    g.leaveEbx(error);
    if (display == 0 || error != kSurfaceLost)
        return error;
    report(g, 9, 0x54b824);
    method<kRestore, &host::ddraw::surfaceRestore>(g, cpu, surface);
    const reg32 primary = g.dword(display + kPrimary);
    if (surface != primary && method<kIsLost, &host::ddraw::surfaceIsLost>(g, cpu, primary) != 0)
        method<kRestore, &host::ddraw::surfaceRestore>(g, cpu, g.dword(display + kPrimary));
    const reg32 back = g.dword(display + kBack);
    if (back != 0 && surface != back && method<kIsLost, &host::ddraw::surfaceIsLost>(g, cpu, back) != 0)
        method<kRestore, &host::ddraw::surfaceRestore>(g, cpu, g.dword(display + kBack));
    if (method<kIsLost, &host::ddraw::surfaceIsLost>(g, cpu, surface) != 0)
    {
        g.leaveEbx(kSurfaceLost);
        return kSurfaceLost;
    }
    error = lock();
    report(g, 0xb, 0x54b848, { error });
    if (error != 0)
    {
        report(g, 0xa, 0x54b860);
        windows<kLeaveCriticalSection, &host::leaveLock>(g, cpu, g.dword(kDisplayLock));
        g.call(0x5084e0);
        windows<kEnterCriticalSection, &host::enterLock>(g, cpu, g.dword(kDisplayLock));
        error = lock();
        report(g, 0xb, 0x54b884, { error });
    }
    g.leaveEbx(error);
    if (error != 0)
        return error;
    if (g.byte(display + kFullscreen) != 0 && fe::direct<0x4f9120, wina::hasFocus>(app, cpu) == 0)
        windows<0x534788, &host::setForegroundWindow>(g, cpu, g.dword(display + kWindow));
    if (g.dword(display + kPalette) != 0)
    {
        const reg32 palette = g.dword(display + kPalette);
        g.setWord(display + 0x432, 0xffff);
        g.setWord(display + 0x430, 0x100);
        const reg32 entries = method<kSetEntries, &host::ddraw::paletteSetEntries>(g, cpu, palette, 0, 0, 0x100,
                                                                                   display + kPaletteEntries)
                              & 0xffff;
        report(g, 0xc, 0x54b8a0, { entries });
        const reg32 set = method<kSetPalette, &host::ddraw::surfaceSetPalette>(g, cpu, g.dword(display + kPrimary),
                                                                               g.dword(display + kPalette))
                          & 0xffff;
        g.leaveEbx(0);
        report(g, 0xc, 0x54b8c4, { set });
    }
    report(g, 0xa, 0x54b8e0);
    if (const reg32 changed = g.dword(kDisplayChanged))
        g.call(changed);
    return 0;
}

void frontend::ddrawLockSurface(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4eb5e0", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const reg32 display = cpu.eax, surface = cpu.edx, description = cpu.ebx;
    runWhole(app, cpu, site, [&](Guest&) { return eaa::ddrawLockSurface(app, cpu, display, surface, description); });
}

/* sub_4eb880: the bits set in eax. */
x86::reg32 frontend::eaa::countBits(App* app, CPU& cpu, reg32 value)
{
    reg32 count = 0;
    for (reg32 bits = value; bits != 0; bits &= bits - 1)
        ++count;
    return count;
}

void frontend::countBits(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4eb880", 0, 0, nullptr, 0, kRestoreEdx };
    const reg32 value = cpu.eax;
    runWhole(app, cpu, site, [&](Guest&) { return eaa::countBits(app, cpu, value); });
}

/* sub_4eb8a0: a surface's description (ebx) into the display's list of modes at the
 * index edx, if there is one; with ecx it is the mode now, too. */
x86::reg32 frontend::eaa::ddrawNoteMode(App* app, CPU& cpu, reg32 display, reg32 index, reg32 description,
                                        reg32 current)
{
    Guest g(app, cpu);
    const auto popcount = [](reg32 bits) {
        reg32 count = 0;
        for (; bits != 0; bits &= bits - 1)
            ++count;
        return count;
    };
    // eax and ecx as the code leaves them, their last values kept for the callers.
    reg32 leftEax, leftEcx = current;
    reg32 bits;
    if ((g.byte(description + 5) & 0x10) == 0 || g.dword(description + 0x54) == 0x10)
    {
        bits = popcount(g.dword(description + 0x58));
        bits += popcount(g.dword(description + 0x5c));
        leftEax = popcount(g.dword(description + 0x60));
        bits += leftEax;
        leftEcx = bits;
    }
    else
    {
        bits = g.byte(description + 0x54);
        leftEax = bits;
    }
    const std::uint8_t bitsByte = std::uint8_t(bits);
    if (std::int32_t(index) >= 0)
    {
        const reg32 offset = index * 20;
        leftEax = offset;
        g.setDword(g.dword(display + kModes) + offset, g.dword(description + 0xc));
        g.setDword(g.dword(display + kModes) + offset + 4, g.dword(description + 8));
        g.setByte(g.dword(display + kModes) + offset + 0x12, g.byte(description + 0x54));
        g.setByte(g.dword(display + kModes) + offset + 0x13, bitsByte);
        g.setWord(g.dword(display + kModes) + offset + 0x10, 1);
        g.setDword(g.dword(display + kModes) + offset + 0xc, g.dword(kCapabilities));
        leftEcx = g.dword(kCapabilities);
        if ((g.byte(description + 6) & 4) != 0)
        {
            leftEcx = g.dword(display + kModes) + offset;
            leftEax = g.dword(description + 0x18);
            g.setDword(leftEcx + 8, leftEax);
        }
        if ((g.byte(description + 4) & 0x20) != 0)
        {
            leftEax = g.dword(display + kModes) + offset;
            leftEcx = (offset & 0xffff0000) | g.byte(description + 0x14);
            g.setWord(leftEax + 0x10, std::uint16_t(g.word(leftEax + 0x10) + std::uint16_t(leftEcx)));
        }
        const auto orFlag = [&](std::uint8_t flag) {
            leftEax = index * 5;
            leftEcx = g.dword(display + kModes);
            const reg32 at = leftEcx + leftEax * 4 + 0xc;
            g.setByte(at, g.byte(at) | flag);
        };
        if ((g.byte(description + 4) & 0x80) != 0)
            orFlag(1);
        if ((g.byte(description + 4) & 0x40) != 0)
            orFlag(2);
        if ((g.byte(description + 4) & 1) != 0 && (g.byte(description + 0x6a) & 0x20) != 0)
            orFlag(4);
    }
    if (current != 0)
    {
        if (std::int32_t(index) >= 0)
        {
            leftEax = index * 20;
            const std::uint8_t clip = g.byte(display + 0x460);
            const reg32 modes = g.dword(display + kModes);
            g.leaveEdx(modes);
            const reg32 at = modes + leftEax + 0xc;
            g.setByte(at, clip != 0 ? g.byte(at) | 0x10 : g.byte(at) & 0xef);
        }
        if (g.byte(display + kFullscreen) != 0)
        {
            g.setDword(display + kWidth, g.dword(description + 0xc));
            g.setDword(display + kHeight, g.dword(description + 8));
        }
        leftEax = g.dword(description + 0x54);
        g.setDword(display + kBitsWanted, leftEax);
        leftEax = (leftEax & 0xffffff00) | bitsByte;
        g.setByte(display + kBits, bitsByte);
    }
    g.leaveEcx(leftEcx);
    return leftEax;
}

void frontend::ddrawNoteMode(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4eb8a0", 0, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const reg32 display = cpu.eax, index = cpu.edx, description = cpu.ebx, current = cpu.ecx;
    runWhole(app, cpu, site, [&](Guest&) {
        return eaa::ddrawNoteMode(app, cpu, display, index, description, current);
    });
}

/* sub_4ebaf0: DirectDraw, the driver chosen or the first or the emulation, its second
 * interface, and what its capabilities mean to the library. */
x86::reg32 frontend::eaa::ddrawInit(App* app, CPU& cpu, reg32 given)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x2f0);
    constexpr reg32 kDriver = 0x9f0990, kNames = 0x564390;
    const reg32 helCaps = g.frame(), halCaps = g.frame() + 0x16c;
    const reg32 display = given != 0 ? given : kDisplay;
    if (g.dword(kDisplayLock) == 0)
        g.setDword(kDisplayLock, *fe::direct<0x4eb2e0, ea::newLock>(app, cpu));
    const reg32 specific = g.dword(kDriver) | g.dword(kDriver + 4) | g.dword(kDriver + 8) | g.dword(kDriver + 0xc);
    reg32 first = 0;
    if ((g.byte(kNames) & 0x80) == 0)
    {
        if (specific != 0)
        {
            report(g, 1, 0x54b8f8);
            first = guest::direct<0x4eb430, ddrawCreate>(app, cpu, kDriver);  // the display in eax too, not read
        }
        if (first == 0)
        {
            report(g, 1, 0x54b918);
            first = guest::direct<0x4eb430, ddrawCreate>(app, cpu, 0u);  // the display in eax too, not read
        }
    }
    if (first == 0)
    {
        report(g, 1, 0x54b934);
        first = guest::direct<0x4eb430, ddrawCreate>(app, cpu, 2u);  // the display in eax too, not read
        if (first == 0)
        {
            g.setDword(display, 0);
            g.setDword(kDisplayError, 0x54b948);
            return 0;
        }
    }
    const reg32 error = method<kQueryInterface, &host::ddraw::drawQueryInterface>(g, cpu, first, 0x56b928, display) & 0xffff;
    method<kRelease, &host::ddraw::release>(g, cpu, first);
    report(g, 0xd, 0x54b960, { error });
    if (error != 0)
    {
        g.setDword(kDisplayError, 0x54b9bc);
        return 0;
    }
    g.setDword(halCaps, 0x16c);
    g.setDword(helCaps, 0x16c);
    const reg32 capsError = method<kGetCaps, &host::ddraw::drawGetCaps>(g, cpu, g.dword(display), halCaps, helCaps) & 0xffff;
    report(g, 0xd, 0x54b98c, { g.dword(halCaps + 4), g.dword(helCaps + 4), capsError });

    g.setDword(kCapabilities, 0);
    if ((g.byte(halCaps + 7) & 8) != 0)
        g.setDword(kCapabilities, 8);
    const auto orLow = [&](std::uint8_t flag) { g.setByte(kCapabilities, g.byte(kCapabilities) | flag); };
    const auto orHigh = [&](std::uint8_t flag) { g.setByte(kCapabilities + 1, g.byte(kCapabilities + 1) | flag); };
    if ((g.byte(halCaps + 6) & 8) != 0)
        orHigh(8);
    if ((g.byte(halCaps + 4) & 1) != 0)
        orLow(0x20);
    const std::uint8_t caps = g.byte(halCaps + 4);
    if ((caps & 1) == 0 || (caps & 2) == 0)
    {
        if ((g.byte(halCaps + 4) & 0x40) != 0)
            orLow(0x40);
        if ((g.byte(halCaps + 7) & 4) != 0)
            orLow(0x80);
        if ((g.byte(halCaps + 5) & 2) != 0)
            orHigh(1);
        if ((g.byte(halCaps + 7) & 0x80) != 0)
            orHigh(4);
    }
    if ((g.byte(halCaps + 7) & 1) != 0 && (g.byte(halCaps + 0xd) & 2) != 0)
        orHigh(2);
    if ((g.byte(halCaps + 0x84) & 0x10) != 0)
        orLow(0x10);
    g.setDword(kDisplayError, 0);
    return g.dword(display) != 0 ? 1 : 0;
}

void frontend::ddrawInit(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4ebaf0", 0, 0, nullptr, 0, kRestoreAll };
    const reg32 given = cpu.eax;
    runWhole(app, cpu, site, [&](Guest&) { return eaa::ddrawInit(app, cpu, given); });
}

/* sub_4ebe50: the display taken: its cooperative level, the list of its modes, a clipper
 * if it is in a window.  1 if all went well, else why at 0x56825c. */
x86::reg32 frontend::eaa::ddrawAccessDisplay(App* app, CPU& cpu, reg32 given)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x18);
    const reg32 display = given != 0 ? given : kDisplay;
    const auto failed = [&](reg32 why) {
        g.setDword(kDisplayError, why);
        return reg32(0);
    };
    if (g.dword(display) == 0)
        return failed(0x54b9f8);
    if (g.dword(display + kWindow) == 0)
        return failed(0x54ba18);
    reg32 level;
    if (g.byte(display + kFullscreen) != 0)
    {
        report(g, 0xd, 0x54ba34);
        level = g.dword(0x564edc) != 0 ? 0x51 : 0x11;
    }
    else
    {
        report(g, 0xd, 0x54ba68);
        level = 8;
    }
    const reg32 error = method<kSetCooperativeLevel, &host::ddraw::drawSetCooperativeLevel>(
                            g, cpu, g.dword(display), g.dword(display + kWindow), level)
                        & 0xffff;
    report(g, 2, 0x54ba9c, { error });
    if (error != 0)
        return failed(0x54bbac);

    g.setDword(kModeCount, 0);
    report(g, 8, 0x54bacc);
    const reg32 counting = method<kEnumDisplayModes, &host::ddraw::drawEnumDisplayModes>(g, cpu, g.dword(display), 1, 0,
                                                                                      display, kModesCallback);
    if ((counting & 0xffff) != 0)
        return failed(0x54baec);
    report(g, 8, 0x54bb10, { g.dword(kModeCount) });
    if (const reg32 old = g.dword(display + kModes))
    {
        fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, old);
        g.setDword(display + kModes, 0);
    }
    const reg32 counted = g.dword(kModeCount);
    g.setDword(display + kModesRoom, counted);
    if (std::int32_t(counted) < 0x10)
        g.setDword(display + kModesRoom, 0x10);
    g.setDword(0x552198, 0x1c5);
    g.setDword(0x552194, 0x54bb28);
    g.setDword(0x552190, 0x54bb1c);
    const reg32 modes =
        *fe::direct<0x4e1620, ea::reserved>(app, cpu, 0x54bb3cu, g.dword(display + kModesRoom) * 20, g.dword(0x5643f4));
    g.setDword(display + kModes, modes);
    if (modes == 0)
    {
        g.setDword(kDisplayError, 0x54bb48);
        return 0;
    }
    g.setDword(kModesDefault, 0);
    g.setDword(kModeCount, 0);
    const reg32 enumerated = method<kEnumDisplayModes, &host::ddraw::drawEnumDisplayModes>(g, cpu, g.dword(display), 1, 0,
                                                                                        display, kModesCallback);
    report(g, 4, 0x54bb70, { g.dword(kModeCount), g.dword(kModesDefault), enumerated });
    g.setDword(display + 0x28, 0x300);
    g.setDword(display + 0x2c, 0x100);
    g.setDword(display + kModesRoom, g.dword(kModeCount));
    if (g.byte(display + kFullscreen) == 0 && g.dword(display + kClipper) == 0)
    {
        tableMethod(g, g.dword(display), kCreateClipper, { 0, display + kClipper, 0 });
        tableMethod(g, g.dword(display + kClipper), kSetHWnd, { 0, g.dword(display + kWindow) });
    }
    else
        g.setDword(display + kClipper, 0);
    g.setDword(kDisplayError, 0);
    return 1;
}

void frontend::ddrawAccessDisplay(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4ebe50", 0, 0, nullptr, 0, kRestoreAll };
    const reg32 given = cpu.eax;
    runWhole(app, cpu, site, [&](Guest&) { return eaa::ddrawAccessDisplay(app, cpu, given); });
}

/* sub_4ec0d0: a display's surfaces and their windows gone. */
x86::reg32 frontend::eaa::ddrawRemoveSurfaces(App* app, CPU& cpu, reg32 display)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x18);
    report(g, 1, 0x54bbcc);
    if (g.dword(display + kPrimary) != 0 && g.dword(display) != 0)
        method<kFlipToGDISurface, &host::ddraw::drawFlipToGdiSurface>(g, cpu, g.dword(display));
    if (const reg32 window = g.dword(display + kBackWindow))
        fe::direct<0x508e70, menz::videoWindowFree>(app, cpu, window);
    if (const reg32 window = g.dword(display + kPrimaryWindow))
        fe::direct<0x508e70, menz::videoWindowFree>(app, cpu, window);
    g.setDword(display + kPrimaryWindow, 0);
    const reg32 back = g.dword(display + kBack);
    g.setDword(display + kBackWindow, 0);
    if (back != 0)
        fe::direct<0x4ed730, menz::ddrawFreeSurface>(app, cpu, display, back);
    if (const reg32 primary = g.dword(display + kPrimary))
        fe::direct<0x4ed730, menz::ddrawFreeSurface>(app, cpu, display, primary);
    g.setDword(display + kBack, 0);
    g.setDword(display + kPrimary, 0);
    report(g, 0xa, 0x54bbfc);
    return 1;
}

void frontend::ddrawRemoveSurfaces(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4ec0d0", 0, 0, nullptr, 0, kRestoreAll };
    const reg32 display = cpu.eax;
    runWhole(app, cpu, site, [&](Guest&) { return eaa::ddrawRemoveSurfaces(app, cpu, display); });
}

/* sub_4ec1a0: the display configured as its record says: what was made gone, the palette,
 * the primary surface, the back buffer and the windows on them. */
x86::reg32 frontend::eaa::ddrawConfigDisplay(App* app, CPU& cpu, reg32 given)
{
    Guest g(app, cpu);
    const fe::Frame frame(g, cpu, 0x360);
    constexpr reg32 kError = 0x401010;
    constexpr reg32 kLibraryMemory = 0x436, kVideoMemory = 0x435, kSystemMemory = 0x434, kPagesWanted = 0x437;
    constexpr reg32 kForceDoubleBuffer = 0x45e;
    const reg32 fp = g.frame() + 0x348;
    const reg32 helCaps = fp - 0x348, halCaps = fp - 0x1dc, description = fp - 0x70;
    report(g, 1, 0x54bc20);
    const reg32 display = given != 0 ? given : kDisplay;
    if (g.dword(display + 0x440) != 0)
    {
        g.setDword(0x552190, 0x54bb1c);
        g.setDword(0x552194, 0x54bc3c);
        g.setDword(0x552198, 0x201);
        fe::direct<kError, ea::error>(g.app(), g.cpu(), 0x54bc50, guest::Words({}));
    }
    windows<kEnterCriticalSection, &host::enterLock>(g, cpu, g.dword(kDisplayLock));
    guest::direct<0x4ec0d0, ddrawRemoveSurfaces>(app, cpu, display);  // halCaps in edx too, which it does not read
    g.setDword(helCaps, 0x16c);
    g.setDword(halCaps, 0x16c);
    method<kGetCaps, &host::ddraw::drawGetCaps>(g, cpu, g.dword(display), helCaps, halCaps);

    const std::uint8_t pagesWanted = g.byte(display + kPagesWanted);
    if (pagesWanted < 1)
        g.setByte(display + kPagesWanted, 1);
    else if (pagesWanted > 2)
    {
        g.setDword(kDisplayError, 0x54bc94);
        windows<kLeaveCriticalSection, &host::leaveLock>(g, cpu, g.dword(kDisplayLock));
        return 0;
    }
    g.setByte(display + 0x460, (g.byte(helCaps + 0x84) & 0x10) != 0 ? 1 : 0);
    g.setByte(display + 0x462, (g.byte(helCaps + 7) & 8) != 0 ? 1 : 0);

    reg32 flags = 0;
    if (g.byte(display + kForceDoubleBuffer) != 0)
    {
        g.setByte(display + kLibraryMemory, 0);
        g.setByte(display + kVideoMemory, 1);
        g.setByte(display + kPagesWanted, 2);
        flags |= 0x2a00;
        g.setByte(display + kPages, 1);
    }
    else if (g.byte(display + kLibraryMemory) != 0)
    {
        g.setByte(display + kSystemMemory, 1);
        g.setByte(display + kVideoMemory, 1);
    }
    if (display == kDisplay)
        flags |= 0x200;
    if (g.byte(display + kPages) > 1)
    {
        const bool notVideo = g.byte(display + kVideoMemory) != 0 || g.byte(display + kSystemMemory) != 0;
        if (g.byte(display + 0x460) == 0 && g.byte(display + 0x462) != 0)
            flags |= 0x800;
        else if (g.byte(display + kForceDoubleBuffer) != 0 || !notVideo)
            flags |= 0x4000;
        else
            flags |= 0x800;
    }

    if (const reg32 palette = g.dword(display + kPalette))
        method<kSetEntries, &host::ddraw::paletteSetEntries>(g, cpu, palette, 0, 0, 0x100, display + kPaletteEntries);
    else
        method<kCreatePalette, &host::ddraw::drawCreatePalette>(g, cpu, g.dword(display), 0x44, display + kPaletteEntries,
                                                             display + kPalette, 0);
    g.setWord(display + 0x432, 0xff);
    g.setByte(display + kModeX, 0);
    const std::uint8_t library = g.byte(display + kLibraryMemory);
    g.setWord(display + 0x430, 0);

    const reg32 memoryName = library != 0 ? 0x54bcb8 : (flags & 0x4000) != 0 ? 0x54bcc0 : 0x54bcc8;
    const reg32 kindName = g.byte(display + kPages) > 1 && (flags & 0x800) == 0 ? 0x54bcd0 : 0x54bcdc;
    report(g, 8, 0x54bcec, { kindName, memoryName });

    reg32 surface = guest::direct<0x4eb490, ddrawMakePrimary>(app, cpu, display, flags);
    bool modeX = true;
    if (surface != 0)
    {
        g.setDword(description, 0x6c);
        method<kGetSurfaceDesc, &host::ddraw::surfaceGetSurfaceDesc>(g, cpu, surface, description);
        modeX = (g.byte(description + 0x6a) & 0x20) != 0;
        g.setByte(display + kModeX, modeX ? 1 : 0);
    }
    else
        g.setByte(display + kModeX, 1);
    if (modeX)
    {
        report(g, 0xe, 0x54bd10);
        g.setByte(display + kPagesWanted, 2);
        g.setByte(display + kPages, g.byte(display + kPagesWanted));
        if (surface != 0)
            fe::direct<0x4ed730, menz::ddrawFreeSurface>(app, cpu, display, surface);
        surface = guest::direct<0x4eb490, ddrawMakePrimary>(app, cpu, display, flags);
    }
    if (surface == 0)
    {
        g.setDword(kDisplayError, 0x54bd38);
        windows<kLeaveCriticalSection, &host::leaveLock>(g, cpu, g.dword(kDisplayLock));
        return 0;
    }
    if (g.dword(display + kClipper) != 0 && g.byte(display + kFullscreen) == 0)
        method<kSetClipper, &host::ddraw::surfaceSetClipper>(g, cpu, surface, g.dword(display + kClipper));
    g.setDword(description, 0x6c);
    method<kGetSurfaceDesc, &host::ddraw::surfaceGetSurfaceDesc>(g, cpu, surface, description);
    guest::direct<0x4eb8a0, ddrawNoteMode>(app, cpu, display, g.dword(display + kModeIndex), description, 1u);

    const std::uint8_t pages = g.byte(display + kPages);
    g.setDword(display + kPrimary, surface);
    if (pages > 1)
        // The display in edx too, which it does not read.
        g.setDword(display + kBack, fe::direct<0x4ed770, menz::ddrawBackSurface>(app, cpu, surface));
    else if (g.byte(display + kLibraryMemory) == 0 && g.byte(display + kPagesWanted) > 1)
    {
        if (g.byte(display + kVideoMemory) == 0 && g.byte(display + kSystemMemory) == 0)
        {
            report(g, 0xe, 0x54bd5c);
            g.setDword(display + kBack, fe::direct<0x4ed480, menz::ddrawMakeSurface>(
                                            app, cpu, display, g.dword(display + kWidth), g.dword(display + kHeight),
                                            g.dword(display + kBitsWanted), 0u, 0u, 0x4040u, 0u));
        }
        if (g.dword(display + kBack) == 0)
        {
            report(g, 0xe, 0x54bd84);
            g.setDword(display + kBack, fe::direct<0x4ed480, menz::ddrawMakeSurface>(
                                            app, cpu, display, g.dword(display + kWidth), g.dword(display + kHeight),
                                            g.dword(display + kBitsWanted), 0u, 0u, 0x840u, 0u));
        }
    }

    const auto windowOn = [&](reg32 surfaceAt, reg32 which) {
        g.setDword(description, 0x6c);
        g.setDword(description + 0x10, g.dword(display + kWidth));
        method<kGetSurfaceDesc, &host::ddraw::surfaceGetSurfaceDesc>(g, cpu, g.dword(surfaceAt), description);
        return fe::direct<0x508c70, menz::videoWindowMake>(app, cpu, g.dword(display + kWidth), g.dword(display + kHeight),
                                                           g.dword(description + 0x10), reg32(g.byte(display + kBits)),
                                                           which, g.dword(description + 0x24), g.dword(surfaceAt));
    };
    if (g.dword(display + kPrimary) != 0)
        g.setDword(display + kPrimaryWindow, windowOn(display + kPrimary, reg32(-1)));
    if (g.dword(display + kBack) != 0)
        g.setDword(display + kBackWindow, windowOn(display + kBack, reg32(-2)));
    else if (g.byte(display + kPagesWanted) > 1)
    {
        report(g, 4, 0x54bdac);
        const reg32 window = guest::direct<0x4ef190, eab::windowBitmapCreateDefault>(
            app, cpu, g.dword(display + kWidth), g.dword(display + kHeight), reg32(g.byte(display + kBits)),
            g.dword(display + 0x20));
        g.setDword(display + kBackWindow, window);
        copyWords(g, 0x565064, window, 14);
    }

    const reg32 flipping = g.byte(display + kPages) > 1 ? 0x46 : 0x20;
    const reg32 system = g.byte(display + kSystemMemory) != 0 ? 0x42 : 0x20;
    const reg32 memory = g.byte(display + kLibraryMemory) != 0 ? 0x4c : g.byte(display + kVideoMemory) != 0 ? 0x44 : 0x56;
    const reg32 modeXLetter = g.byte(display + kModeX) != 0 ? 0x58 : 0x20;
    report(g, 8, 0x54bdd8, { g.dword(display + kWidth), g.dword(display + kHeight), g.dword(display + kBitsWanted),
                             g.byte(display + kBits), g.dword(g.dword(display + kPrimaryWindow) + 0x28),
                             g.byte(display + kPagesWanted), modeXLetter, memory, system, flipping });
    windows<kLeaveCriticalSection, &host::leaveLock>(g, cpu, g.dword(kDisplayLock));
    if (const reg32 changed = g.dword(kDisplayChanged))
        g.call(changed);
    return 1;
}

void frontend::ddrawConfigDisplay(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site = { "sub_4ec1a0", 0, 0, nullptr, 0, kRestoreAll };
    const reg32 given = cpu.eax;
    runWhole(app, cpu, site, [&](Guest&) { return eaa::ddrawConfigDisplay(app, cpu, given); });
}

}  // namespace nfs3hp
