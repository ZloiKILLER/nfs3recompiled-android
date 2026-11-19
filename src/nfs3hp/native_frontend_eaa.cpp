#include "native_frontend_eaa.h"
#include "native_load_saves.h"
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
#include "native_frontend_pcall.h"
#include "os_video.h"
#include "native_render.h"
#include <nfs3hp.h>
#include <lib/app.h>
#include <cstdint>
#include <initializer_list>
#include <optional>
#include <type_traits>
#include <utility>
#include <cstddef>

/* The thunks defined below by qualified name, declared where they are defined. */
namespace nfs3hp
{
namespace frontend {
nfs::u32 memViewerRun(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memViewerOpen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
void memViewerClose(nfs::App* app, nfs::ThreadState& ts);
nfs::u32 memViewerFirstDraw(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memViewerStatsLine(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memViewerListBlocks(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memViewerHeader(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memViewerRows(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memViewerStatusLine(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memViewerDescribeBlock(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memViewerCountShapes(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memViewerMenu(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
void memViewerNextClass(nfs::App* app, nfs::ThreadState& ts);
nfs::u32 memViewerFlagText(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memViewerSortMode(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memViewerCompare(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memPoolTotals(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memPoolFirstBlock(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memPoolCountBlocks(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memPoolNextBlock(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 viewerBlockSize(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memBlockData(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 viewerBlockFlags(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memBlockKind(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memBlockIsFree(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memViewerViewShapes(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memViewerViewWindow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memViewerViewBitmap(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memShapeBits(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memViewerHexDump(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memViewerHexPage(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memViewerWriteMap(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 memViewerFreeFileName(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 textLoadFonts(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 udffOpen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 udffClose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 udffReadWord(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 fileSizeOpened(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 fileSize(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 bitmapDrawToScreen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 bitmapDrawInto(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 bitmapReadPixels(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 bitmapTgaSize(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 bitmapWriteTga(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 bitmapSaveTga(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 keyStateByte(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 ddrawCreate(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 ddrawMakePrimary(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 countBits(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 ddrawInit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 ddrawAccessDisplay(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 ddrawRemoveSurfaces(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 ddrawConfigDisplay(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
}  // frontend
}  // namespace nfs3hp

namespace nfs3hp
{
namespace
{

/* A typed core called as C++ (fe::direct) whose function changes no register but eax: the first `Taken` of `words` its arguments,
 * its result in eax. */
template <nfs::u32 Address, auto Core, std::size_t... I>
auto directOf(nfs::App* app, nfs::ThreadState& ts, const nfs::u32* words, std::index_sequence<I...>)
{
    return fe::direct<Address, Core>(app, ts, words[I]...);
}

template <nfs::u32 Address, auto Core, std::size_t Taken, typename... Words>
nfs::u32 directKept(nfs::App* app, nfs::ThreadState& ts, Words... words)
{
    const nfs::u32 all[] = { nfs::u32(words)..., 0 };
    const auto result = directOf<Address, Core>(app, ts, all, std::make_index_sequence<Taken>{});
    if constexpr (std::is_same_v<std::remove_cv_t<decltype(result)>, nfs::u32>)
        return result;
    else
        return *result;
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
 * called as C++ (direct calls and methods); what each function leaves in eax and in the registers it does not save is kept.
 *
 * Plan step 5: each function a typed core in frontend::eaa (native_frontend_eaa.h), in a frame
 * of its own where the game's function has one (fe::Frame), and a thunk under the game's
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
using fe::Site;
using nfs::u32;
using nfs::s32;

/* ---------------------------------------------------------------------------
 * The memory viewer's state
 * ------------------------------------------------------------------------- */
constexpr u32 kConsole = 0x79f5a4;       // the table of drawing functions
constexpr u32 kRowsTotal = 0x79f59c;     // the screen's height in characters
constexpr u32 kColumnsTotal = 0x79f5c0;  // its width
constexpr u32 kListCursor = 0x79f5a0;    // the block sub_499400 last gave
constexpr u32 kNeedHeader = 0x79f5a8;
constexpr u32 kBlockCount = 0x79f5ac;
constexpr u32 kLastColumn = 0x79f5b0;
constexpr u32 kRedrawAll = 0x79f5b4;
constexpr u32 kListTop = 0x79f5b8;       // the first screen row of the list
constexpr u32 kHeaderRow = 0x79f5bc;
constexpr u32 kBlockTable = 0x79f5c4;    // 8 bytes a block: the block, its owner's value
constexpr u32 kFirstShown = 0x79f5c8;
constexpr u32 kPageSize = 0x79f5cc;
constexpr u32 kListed = 0x79f5d0;
constexpr u32 kLastShown = 0x79f5d4;
constexpr u32 kPreviousSelected = 0x79f5d8;
constexpr u32 kBlockTableGiven = 0x79f5dc;
constexpr u32 kMenuEntries = 0x79f524;   // 12 bytes each: 8 of its name, its function
constexpr u32 kMenuCount = 0x5662e0;
constexpr u32 kMenuAdded = 0x9f25a0;
constexpr u32 kSelected = 0x55e558;
constexpr u32 kPoolShown = 0x55e55c;
constexpr u32 kPools = 0x9ef5c0;
constexpr u32 kHeaderKept = 0x55e560;
constexpr u32 kColumnsUsed = 0x55e564;
constexpr u32 kFirstColumn = 0x55e568;
constexpr u32 kColumnWidths = 0x55e56c;
constexpr u32 kListFresh = 0x55e590;
constexpr u32 kClearOld = 0x55e594;
constexpr u32 kClearColumns = 0x55e598;
constexpr u32 kClearRows = 0x55e59c;
constexpr u32 kSortChoosing = 0x55e5a0;
constexpr u32 kSortColumn = 0x55e5a4;
constexpr u32 kSortAscending = 0x55e5a8;
constexpr u32 kHexNumbers = 0x55e5ac;
constexpr u32 kShowFile = 0x55e5b0;      // the status line says the block's file
constexpr u32 kShapeBits = 0x566d54;     // bits a pixel by a shape's type

constexpr u32 kSpaceText = 0x53c094;     // " "
constexpr u32 kThreeLetters = 0x53c0dc;  // "%3s"

constexpr std::uint16_t kFreeBlock = 0x4246, kUsedBlock = 0x424d;  // 'FB', 'MB'
constexpr u32 kShpi = 0x53485049, kShpf = 0x53485046;

// The keys, as the console's function gives them.
constexpr u32 kKeyEnter = 0x0d, kKeyEscape = 0x1b, kKeyUp = 0x4800, kKeyPageUp = 0x4900;
constexpr u32 kKeyLeft = 0x4b00, kKeyRight = 0x4d00, kKeyDown = 0x5000, kKeyPageDown = 0x5100;

// What the game's other functions are.
constexpr u32 kSprintf = 0x4df690, kBlockName = 0x4e15e0, kReserve = 0x4e1620;

/* The console: the table of functions the viewer draws through. */
class Console
{
public:
    explicit Console(Guest& guest) : m_guest(guest) {}

    u32 begin() { return callSlot(0x00); }
    u32 end() { return callSlot(0x04); }
    /* 0 columns, 1 rows, 2 pixels across, 3 down, 4 whether graphical. */
    u32 metric(u32 which)
    {
        const u32 table = m_guest.dword(kConsole);
        return fe::pointerCall(m_guest, m_guest.dword(table + 0x08), args({ which }));
    }
    /* At a column and a row, printf's arguments after them. */
    u32 print(std::initializer_list<u32> stack)
    {
        const u32 table = m_guest.dword(kConsole);
        Arguments arguments;
        for (const u32 value : stack)
            arguments.stack[arguments.stackCount++] = value;
        return fe::pointerCall(m_guest, m_guest.dword(table + 0x0c), arguments);
    }
    u32 drawShape(u32 shape, u32 bits, u32 x, u32 y)
    {
        const u32 table = m_guest.dword(kConsole);
        return fe::pointerCall(m_guest, m_guest.dword(table + 0x14), args({ shape, bits, x, y }));
    }
    u32 key() { return callSlot(0x18); }
    u32 init() { return callSlot(0x1c); }
    /* A block of the screen cleared: column, row, columns, rows; the colour on the stack. */
    u32 clear(u32 x, u32 y, u32 w, u32 h, u32 colour)
    {
        const u32 table = m_guest.dword(kConsole);
        return fe::pointerCall(m_guest, m_guest.dword(table + 0x20), args({ x, y, w, h }, { colour }));
    }
    u32 colours(u32 foreground, u32 background)
    {
        const u32 table = m_guest.dword(kConsole);
        return fe::pointerCall(m_guest, m_guest.dword(table + 0x24), args({ foreground, background }));
    }

private:
    u32 callSlot(u32 slot)
    {
        const u32 table = m_guest.dword(kConsole);
        return fe::pointerCall(m_guest, m_guest.dword(table + slot), args({ table }));
    }

    Guest& m_guest;
};

void append(Guest& guest, u32 to, u32 from)
{
    guest.copyString(to + guest.length(to), from);
}

u32 sprintf1(Guest& guest, u32 buffer, u32 format, u32 value)
{
    return fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), buffer, format, guest::Words{value});
}

/* sub_4993b0: the first block of the pool shown. */
u32 firstBlock(Guest& guest)
{
    return guest.dword(guest.dword(kPools + guest.dword(kPoolShown) * 4) + 8);
}

/* sub_499480, sub_499490: a block's data and flags. */
u32 blockData(u32 block)
{
    return block + 0x10;
}

u32 blockFlags(Guest& guest, u32 block)
{
    return guest.word(block + 2);
}

/* sub_499460: a block's size, the block selected's for none. */
u32 blockSize(Guest& guest, u32 block)
{
    if (block == 0)
        block = guest.dword(guest.dword(kBlockTable) + guest.dword(kSelected) * 8);
    return guest.dword(block + 4);
}

/* sub_499580: whether a block is free space. */
bool blockIsFree(Guest& guest, u32 block)
{
    return guest.word(block) == kFreeBlock;
}

u32 listedBlock(Guest& guest, u32 index)
{
    return guest.dword(guest.dword(kBlockTable) + index * 8);
}

/* sub_499e90: the bits a pixel of a shape's type. */
u32 shapeBits(Guest& guest, u32 shape)
{
    return guest.byte((guest.dword(shape) & 0x7f) + kShapeBits);
}

/* The word 2 bytes on, as the code reads it: the top half of the dword at `at`, signed. */
u32 highWord(Guest& guest, u32 at)
{
    return u32(s32(guest.dword(at)) >> 16);
}

/* The first four bytes, each without its top bit, as one big-endian word: a pack's tag. */
u32 tagOf(Guest& guest, u32 at)
{
    u32 tag = 0;
    for (u32 i = 0; i < 4; ++i)
        tag = (tag << 8) | (guest.byte(at + i) & 0x7f);
    return tag;
}

/* sub_499330's numbers read into the frame, and sub_497d20 given them. */
void measureAndShow(Guest& guest, nfs::ThreadState& ts, u32 total, u32 used, u32 available, u32 largest)
{
    guest::direct<0x499330, frontend::eaa::memPoolTotals>(guest.app(), ts, total, used, available, largest);
    guest::direct<0x497d20, frontend::eaa::memViewerStatsLine>(guest.app(), ts, guest.dword(total), guest.dword(used),
                                                                guest.dword(available), guest.dword(largest));
}

/* A thunk's stack argument `index` (0 the first), above the return address. */
u32 stackWord(nfs::App* app, const nfs::ThreadState& ts, u32 index)
{
    return app->getMemory<u32>(ts.sp + 4 + 4 * index);
}

}  // namespace

/* sub_497620: the viewer: the console's table in eax, until Escape. */
nfs::u32 frontend::eaa::memViewerRun(App* app, ThreadState& ts, u32 table)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x2c);
    Console console(g);
    const u32 fp = g.frame() + 0x14;
    const u32 total = fp - 0x14, used = fp - 0x10, available = fp - 0xc, largest = fp - 8;
    g.setDword(kConsole, table);
    console.init();
    u32 blocks = guest::direct<0x4993d0, memPoolCountBlocks>(app, ts);
    if (const u32 given = g.dword(kBlockTableGiven))
        g.setDword(kBlockTable, given);
    else
    {
        // Else room for the table on the stack, if there is any.
        u32 room = 0;
        // stackavail; the count sub_4993d0 left in eax is not read by it.
        if (fe::direct<0x4f45c0, watcom::clib::stackavail>(app, ts) > 0x2000)
        {
            ts.sp -= 4;  // __GRO's word, 0x2000, below the return address
            g.setDword(ts.sp, 0x2000);
            fe::direct<0x4f4610, watcom::clib::stackProbed>(app, ts, 0x2000u);
            ts.sp += 4;
            ts.sp -= 0x2000;
            room = ts.sp;
        }
        g.setDword(kBlockTable, room);
    }
    guest::direct<0x497ba0, memViewerOpen>(app, ts);
    if (g.dword(kMenuAdded) == 0)
    {
        fe::direct<0x4f4650, wata::registryAddEntry>(app, ts, 0x53bff8u, 0x4995b0u);
        fe::direct<0x4f4650, wata::registryAddEntry>(app, ts, 0x53c000u, 0x499ea0u);
        g.setDword(kMenuAdded, 1);
    }
    guest::direct<0x497cc0, memViewerFirstDraw>(app, ts, blocks);

    for (;;)
    {
        const u32 key = console.key();
        g.setDword(kSortChoosing, 0);
        bool moved = false;
        g.setDword(kRedrawAll, 0);
        g.setDword(kNeedHeader, 0);
        const u32 otherFile = g.dword(kShowFile) ^ 1;
        const u32 page = g.dword(kPageSize) - 1;
        const std::int32_t lastBlock = std::int32_t(g.dword(kListed) - 1);
        switch (key)
        {
        case kKeyDown:
        {
            g.setDword(kShowFile, 0);
            const u32 selected = g.dword(kSelected);
            g.setDword(kPreviousSelected, selected);
            if (lastBlock > std::int32_t(selected))
            {
                moved = true;
                g.setDword(kSelected, selected + 1);
            }
            const u32 last = g.dword(kLastShown);
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
            const u32 selected = g.dword(kSelected);
            g.setDword(kPreviousSelected, selected);
            g.setDword(kShowFile, 0);
            if (std::int32_t(selected) > 0)
            {
                moved = true;
                g.setDword(kSelected, selected - 1);
            }
            const u32 first = g.dword(kFirstShown);
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
            const u32 selected = g.dword(kSelected);
            const u32 last = g.dword(kLastShown);
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
                    const u32 by = u32(lastBlock) - last;
                    g.setDword(kLastShown, u32(lastBlock));
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
            const u32 selected = g.dword(kSelected);
            const u32 first = g.dword(kFirstShown);
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
                    const u32 last = g.dword(kLastShown);
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
            const u32 last = g.dword(kLastColumn);
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
            const u32 first = g.dword(kFirstColumn);
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
            guest::direct<0x498f00, memViewerSortMode>(app, ts, blocks);
            break;
        case 0x48:  // H: hexadecimal or decimal
            g.setByte(kHexNumbers, std::uint8_t(g.byte(kHexNumbers) ^ 1));
            measureAndShow(g, ts, total, used, available, largest);
            g.setDword(kRedrawAll, 1);
            break;
        case 0x43:  // C: the next pool
            g.setDword(kShowFile, 0);
            guest::direct<0x498e10, memViewerNextClass>(app, ts);
            g.setDword(kClearOld, 1);
            g.setDword(kListFresh, 1);
            guest::direct<0x499330, memPoolTotals>(app, ts, total, used, available, largest);
            blocks = guest::direct<0x4993d0, memPoolCountBlocks>(app, ts);
            g.setDword(kBlockCount, blocks);
            g.setDword(kListed, guest::direct<0x497fe0, memViewerListBlocks>(app, ts, blocks));
            guest::direct<0x497d20, memViewerStatsLine>(app, ts, g.dword(total), g.dword(used), g.dword(available),
                                                         g.dword(largest));
            g.setDword(kNeedHeader, 1);
            g.setDword(kRedrawAll, 1);
            break;
        case 0x20:  // Space: view the block
            guest::direct<0x4995b0, memViewerViewShapes>(app, ts);
            measureAndShow(g, ts, total, used, available, largest);
            g.setDword(kNeedHeader, 1);
            g.setDword(kRedrawAll, 1);
            break;
        case kKeyEnter:  // the menu of what to do with it
            guest::direct<0x498bf0, memViewerMenu>(app, ts);
            measureAndShow(g, ts, total, used, available, largest);
            g.setDword(kNeedHeader, 1);
            g.setDword(kRedrawAll, 1);
            break;
        case 0x50:  // P: print the map to a file
            guest::direct<0x49a120, memViewerWriteMap>(app, ts);
            break;
        default:
            break;
        }
        if (g.dword(kNeedHeader) == 1)
            guest::direct<0x498090, memViewerHeader>(app, ts);
        if (moved || g.dword(kRedrawAll) == 1)
            guest::direct<0x498370, memViewerRows>(app, ts);  // the count in eax too, which it does not read
        if (key == kKeyEscape)
            break;
    }
    guest::direct<0x497cb0, memViewerClose>(app, ts);
    return kKeyEscape;
}

nfs::u32 frontend::memViewerRun(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_497620", 0, 0, nullptr, 0, kRestoreAll };
    const u32 table = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::memViewerRun(app, ts, table); });
}

/* sub_497ba0: the viewer's state, and the screen's size, afresh. */
nfs::u32 frontend::eaa::memViewerOpen(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x10);
    Console console(g);
    fe::direct<0x4f46d0, wata::windowSave>(app, ts, 0x79f3fcu);
    g.setDword(kSelected, 0);
    fe::direct<0x4f4740, wata::textStateSave>(app, ts, 0x79f46cu);
    if (console.metric(4))
        guest::direct<0x4ef520, eab::fontUse>(app, ts, 0x5662e4u);
    console.begin();
    g.setDword(kRedrawAll, 1);
    g.setDword(kColumnsTotal, console.metric(0));
    g.setDword(kRowsTotal, console.metric(1));
    const u32 result = console.end();
    for (const u32 at : { kListCursor, kListed, kListTop, kHeaderKept, kHeaderRow, kPreviousSelected,
                            kFirstShown, kLastShown, kPageSize, kLastColumn, kColumnsUsed })
        g.setDword(at, 0);
    g.setDword(kRedrawAll, 1);
    g.setDword(kNeedHeader, 1);
    g.setDword(kListFresh, 1);
    for (const u32 at : { kClearOld, kClearColumns, kClearRows, kSortChoosing, kSortColumn })
        g.setDword(at, 0);
    g.setDword(kSortAscending, 1);
    g.setDword(kHexNumbers, 0);
    g.setDword(kBlockCount, 0);
    return result;
}

nfs::u32 frontend::memViewerOpen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_497ba0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx };
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::memViewerOpen(app, ts); });
}

/* sub_497cb0: nothing, at the viewer's end. */
void frontend::eaa::memViewerClose(App* app, ThreadState& ts)
{
    (void)app;
    (void)ts;
}

nfs::u32 frontend::memViewerClose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_497cb0", 0, 0, nullptr, 0, 0 };
    return runThunk(app, ts, args, site, [&](Guest&) { eaa::memViewerClose(app, ts); return args.eax; });
}

/* sub_497cc0: the whole screen, first: the numbers, the list, the header, its rows. */
nfs::u32 frontend::eaa::memViewerFirstDraw(App* app, ThreadState& ts, u32 blocks)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x24);
    const u32 fp = g.frame() + 0x10;
    g.setDword(kBlockCount, blocks);
    measureAndShow(g, ts, fp - 0x10, fp - 0xc, fp - 8, fp - 4);
    g.setDword(kListed, guest::direct<0x497fe0, memViewerListBlocks>(app, ts, blocks));
    guest::direct<0x498090, memViewerHeader>(app, ts);
    return guest::direct<0x498370, memViewerRows>(app, ts);  // the count in eax too, which it does not read
}

nfs::u32 frontend::memViewerFirstDraw(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_497cc0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi };
    const u32 blocks = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::memViewerFirstDraw(app, ts, blocks); });
}

/* sub_497d20: the numbers at the top, as many lines as they take: eax the total, edx
 * what is used, ebx free, ecx the largest free. */
nfs::u32 frontend::eaa::memViewerStatsLine(App* app, ThreadState& ts, u32 total, u32 used, u32 available,
                                             u32 largest)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x104);
    Console console(g);
    const u32 fp = g.frame() + 0xf8 - 0x82;
    const u32 line = fp - 0x76, piece = fp + 0x52, poolName = fp + 0x66;
    const u32 values[6] = { 0, 0, total, used, available, largest };
    static const u32 kHex[6] = { 0x53c00c, 0x53c01c, 0x53c028, 0x53c044, 0x53c05c, 0x53c074 };
    static const u32 kDecimal[6] = { 0x53c00c, 0x53c01c, 0x53c034, 0x53c050, 0x53c068, 0x53c084 };

    g.setDword(poolName, g.dword(0x4975b0));
    g.setDword(poolName + 4, g.dword(0x4975b4));
    g.copyString(poolName, g.dword(kPools + g.dword(kPoolShown) * 4));
    console.colours(0xc, 1);
    g.setByte(line, 0);
    g.setByte(piece, 0);

    // Prints the line so far and moves down.
    u32 row = 0;
    const auto putLine = [&] {
        console.begin();
        console.clear(0, row, g.dword(kColumnsTotal), 1, 1);
        console.print({ 0, row, line });
        console.end();
    };
    u32 length = 0;
    for (u32 i = 0; i < 6; ++i)
    {
        if (i == 1)
            fe::direct<kSprintf, watcom::clib::sprintf>(g.app(), g.ts(), piece, kHex[i], guest::Words{poolName});
        else
        {
            const u32 value = i == 0 ? g.dword(kBlockCount) : values[i];
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
    const u32 lastRow = row;
    ++row;
    const u32 header = g.dword(kHeaderRow);
    if (std::int32_t(header) < std::int32_t(row))
    {
        g.setDword(kListTop, row);
        g.setDword(kHeaderRow, row);
    }
    return header;
}

nfs::u32 frontend::memViewerStatsLine(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_497d20", 0, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const u32 total = args.eax, used = args.edx, available = args.ebx, largest = args.ecx;
    return runThunk(app, ts, args, site, [&](Guest&) {
        return eaa::memViewerStatsLine(app, ts, total, used, available, largest);
    });
}

/* sub_497fe0: the blocks of the pool into the table, in pools of odd number without the
 * free ones: eax how many to go through; how many it holds comes back. */
nfs::u32 frontend::eaa::memViewerListBlocks(App* app, ThreadState& ts, u32 blocks)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x30);
    const u32 fp = g.frame() + 0x18;
    const std::int32_t count = std::int32_t(blocks);
    const bool skipFree = (g.byte(kPoolShown) & 1) != 0;
    u32 listed = 0;
    for (std::int32_t i = 0; i < count; ++i)
    {
        // The game's code hands it i in eax too, which it does not read.
        guest::direct<0x499400, memPoolNextBlock>(app, ts, fp - 0x18, fp - 0x14, fp - 0x10, fp - 0xc);
        const u32 block = g.dword(kListCursor);
        if (g.word(block) == kFreeBlock && skipFree)
            continue;
        g.setDword(g.dword(kBlockTable) + u32(i) * 8, block);
        const std::uint16_t tag = g.word(block);
        ++listed;
        u32 owner = 0;
        if (tag != kFreeBlock)
            owner = fe::direct<0x4f4780, wata::blockChecksum>(app, ts, g.dword(fp - 0x18) + 0x10, g.dword(fp - 0x14));
        g.setDword(g.dword(kBlockTable) + u32(i) * 8 + 4, owner);
    }
    return listed;
}

nfs::u32 frontend::memViewerListBlocks(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_497fe0", 0, 0, nullptr, 0, kRestoreAll };
    const u32 blocks = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::memViewerListBlocks(app, ts, blocks); });
}

/* sub_498090: the columns' header, and the list's size under it. */
nfs::u32 frontend::eaa::memViewerHeader(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x74);
    Console console(g);
    constexpr u32 kTitles = 0x4975b8, kRightAligned = 0x4975dc;
    const u32 title = g.frame() + 0x5c - 0x14;

    const u32 kept = g.dword(kHeaderKept);
    if (kept == 0)
    {
        const u32 top = g.dword(kListTop);
        g.setDword(kHeaderRow, top);
        g.setDword(kHeaderKept, top);
    }
    else
        g.setDword(kHeaderRow, kept);
    console.begin();
    console.clear(0, g.dword(kHeaderRow), g.dword(kColumnsTotal), 1, 1);

    // Columns 0 and 1 stay on the screen.
    std::int32_t x = 0;
    const auto sortedColumn = [&](u32 column) {
        return column == g.dword(kSortColumn) && g.dword(kSortChoosing) == 1;
    };
    for (u32 column = 0; column < 2; ++column)
    {
        if (sortedColumn(column))
            console.colours(3, 5);
        else
            console.colours(2, 1);
        g.copyString(title, g.dword(kTitles + column * 4));
        console.print({ u32(x), g.dword(kHeaderRow), title });
        x = std::int32_t(u32(x) + g.dword(kColumnWidths + column * 4) + 1);
    }
    x = std::int32_t(u32(x) - 1);
    if (x > std::int32_t(g.dword(kColumnsUsed)))
        g.setDword(kColumnsUsed, u32(x));

    // The others from the first one shown, while they fit.
    for (std::int32_t column = std::int32_t(g.dword(kFirstColumn)); column < 9; ++column)
    {
        const u32 width = g.dword(u32(column) * 4 + kColumnWidths);
        const std::int32_t end = std::int32_t(u32(x) + width + 1);
        if (end >= std::int32_t(g.dword(kColumnsTotal)))
            break;
        std::int32_t columns = std::int32_t(g.dword(kColumnsUsed) - u32(x));
        if (columns <= std::int32_t(width + 1))
            columns = std::int32_t(width + 1);
        console.clear(u32(x), g.dword(kHeaderRow), u32(columns), 1, 1);
        g.copyString(title, g.dword(kTitles + u32(column) * 4));
        if (sortedColumn(u32(column)))
            console.colours(3, 5);
        else
            console.colours(2, 4);
        u32 at;
        if (g.dword(kRightAligned + u32(column) * 4) == 1)
            at = u32(end) - g.length(g.dword(kTitles + u32(column) * 4));
        else
            at = u32(x) + 1;
        console.print({ at, g.dword(kHeaderRow), title });
        g.setDword(kLastColumn, u32(column));
        x = end;
        if (x > std::int32_t(g.dword(kColumnsUsed)))
            g.setDword(kColumnsUsed, u32(x));
    }

    g.setDword(kHeaderRow, g.dword(kHeaderRow) + 1);
    console.end();
    u32 result = g.dword(kHeaderRow);
    g.setDword(kListTop, result);
    if (g.dword(kListFresh) != 0)
    {
        const u32 listed = g.dword(kListed);
        g.setDword(kFirstShown, 0);
        u32 last = g.dword(kRowsTotal) - g.dword(kListTop) - 1;
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

nfs::u32 frontend::memViewerHeader(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_498090", 0, 0, nullptr, 0, kRestoreAll };
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::memViewerHeader(app, ts); });
}

/* sub_498370: the list's rows: the names' column, then a column at a time; only the
 * rows that changed, unless all are wanted (0x79f5b4). */
nfs::u32 frontend::eaa::memViewerRows(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x74);
    Console console(g);
    const u32 fp = g.frame() + 0x5c;
    const u32 buffer = fp - 0x5c, flagText = fp - 0x30, kinds = fp - 0x3c, spaceText = fp - 4;
    const u32 drawThis = fp - 0x10;  // what comes before leaves it, as in the code

    g.setWord(spaceText, g.word(0x497600));
    for (u32 k = 0; k < 3; ++k)
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
        const u32 rows = u32(room < listed ? room : listed);
        g.setDword(kClearRows, rows);
        console.clear(0, g.dword(kHeaderRow), g.dword(kColumnsUsed), rows, 1);
    }
    console.colours(0, 1);

    // A row is drawn if everything is, or it is the selection, old or new.
    const auto rowChanged = [&](u32 index, u32& row) {
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

    u32 row = g.dword(kListTop);
    u32 index = g.dword(kFirstShown);
    g.setDword(kPageSize, g.dword(kLastShown) - index + 1);
    for (; std::int32_t(index) < std::int32_t(g.dword(kLastShown) + 1); ++index)
    {
        g.setDword(drawThis, 1);
        rowChanged(index, row);
        if (g.dword(drawThis) != 1)
            continue;
        const u32 selected = g.dword(kSelected);
        if (g.word(listedBlock(g, index)) == kFreeBlock)
        {
            if (index == selected)
            {
                guest::direct<0x4989a0, memViewerStatusLine>(app, ts, selected);
                console.clear(0, row, g.dword(kColumnsUsed), 1, 0xb);
                console.colours(0xa, 0xb);
            }
            else
            {
                console.clear(0, row, g.dword(kColumnsUsed), 1, 9);
                console.colours(8, 9);
            }
            fe::direct<kSprintf, watcom::clib::sprintf>(g.app(), g.ts(), buffer, 0x53c098, guest::Words({}));
        }
        else
        {
            if (index == selected)
            {
                guest::direct<0x4989a0, memViewerStatusLine>(app, ts, selected);
                console.clear(0, row, g.dword(kColumnsUsed), 1, 7);
                console.colours(6, 7);
            }
            else
            {
                console.clear(0, row, g.dword(kColumnsUsed), 1, 1);
                console.colours(0, 1);
            }
            u32 name = directKept<0x4e15e0, ea::blockNameOf, 1>(g.app(), g.ts(), listedBlock(g, index) + 0x10);
            const u32 length = g.length(name);
            if (std::int32_t(length) > 0xc)
                name += length - 0xc;
            fe::direct<kSprintf, watcom::clib::sprintf>(g.app(), g.ts(), buffer, 0x53c0a4, guest::Words{0, name});
        }
        console.print({ 0, row, buffer });
        ++row;
    }

    // The other columns.
    u32 x = g.dword(kColumnWidths) + g.dword(kColumnWidths + 4) + 1;
    for (std::int32_t column = std::int32_t(g.dword(kFirstColumn)); column <= std::int32_t(g.dword(kLastColumn)); ++column)
    {
        u32 columnRow = g.dword(kListTop);
        for (u32 i = g.dword(kFirstShown); std::int32_t(i) < std::int32_t(g.dword(kLastShown) + 1); ++i)
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
            const u32 at = x + 1;
            const auto spaceAfter = [&] {
                if (u32(column) != g.dword(kLastColumn))
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
                guest::direct<0x498e40, memViewerFlagText>(app, ts, blockFlags(g, listedBlock(g, i)), flagText);
                sprintf1(g, buffer, 0x53c0c8, flagText);
                spaceAfter();
                break;
            case 5:
                fe::direct<kSprintf, watcom::clib::sprintf>(g.app(), g.ts(), buffer, 0x53c0d0, guest::Words({}));
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
                const u32 kind = fe::direct<0x4f47c0, wata::shapeEndsAtBend>(app, ts, listedBlock(g, i));
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
        x += g.dword(u32(column) * 4 + kColumnWidths) + 1;
        if (std::int32_t(x) > std::int32_t(g.dword(kColumnsUsed)))
            g.setDword(kColumnsUsed, x);
        g.setDword(kClearColumns, g.dword(kColumnsUsed));
    }
    return console.end();
}

nfs::u32 frontend::memViewerRows(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_498370", 0, 0, nullptr, 0, kRestoreAll };
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::memViewerRows(app, ts); });
}

/* sub_4989a0: the line at the bottom, of the block selected: eax its index. */
nfs::u32 frontend::eaa::memViewerStatusLine(App* app, ThreadState& ts, u32 index)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x64);
    Console console(g);
    const u32 text = g.frame();
    const u32 kind = guest::direct<0x4994a0, memBlockKind>(app, ts, index);
    guest::direct<0x498a40, memViewerDescribeBlock>(app, ts, index, kind, text);
    console.clear(0, g.dword(kRowsTotal), g.dword(kColumnsUsed), 1, 0xe);
    console.colours(0xd, 0xe);
    console.print({ 0, g.dword(kRowsTotal), text });
    return console.colours(0, 1);
}

nfs::u32 frontend::memViewerStatusLine(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4989a0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi };
    const u32 index = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::memViewerStatusLine(app, ts, index); });
}

/* sub_498a40: what the status line says of a block: eax its index, edx its kind
 * (sub_4994a0), ebx where the text goes. */
nfs::u32 frontend::eaa::memViewerDescribeBlock(App* app, ThreadState& ts, u32 index, u32 kind, u32 text)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x20);
    const u32 fp = g.frame() + 0x10;
    const u32 showFile = g.dword(kShowFile);
    for (const u32 at : { fp - 0xc, fp - 8, fp - 4, fp - 0x10 })
        g.setDword(at, 0);
    if (showFile != 0)
    {
        const u32 name = directKept<0x4e15e0, ea::blockNameOf, 1>(g.app(), g.ts(), listedBlock(g, index) + 0x10);
        return fe::direct<kSprintf, watcom::clib::sprintf>(g.app(), g.ts(), text, 0x53c1ac, guest::Words{name, 0x53c0e0});
    }
    switch (kind)
    {
    case 0:
        return fe::direct<kSprintf, watcom::clib::sprintf>(g.app(), g.ts(), text, 0x53c148, guest::Words({}));
    case 1:
        return fe::direct<kSprintf, watcom::clib::sprintf>(g.app(), g.ts(), text, 0x53c154, guest::Words({}));
    case 3:
    {
        guest::direct<0x498b70, memViewerCountShapes>(app, ts, listedBlock(g, index) + 0x10, fp - 0x10, fp - 0xc, fp - 8,
                                                      fp - 4);
        const u32 columns = g.dword(kColumnsTotal);
        return fe::direct<kSprintf, watcom::clib::sprintf>(g.app(), g.ts(), text, std::int32_t(columns) < 0x46 ? 0x53c0e4u : 0x53c10cu, guest::Words{g.dword(fp - 0x10), g.dword(fp - 8), g.dword(fp - 4), g.dword(fp - 0xc)});
    }
    case 4:
        return fe::direct<kSprintf, watcom::clib::sprintf>(g.app(), g.ts(), text, 0x53c16c, guest::Words({}));
    case 5:
        return fe::direct<kSprintf, watcom::clib::sprintf>(g.app(), g.ts(), text, 0x53c18c, guest::Words({}));
    default:
        return fe::direct<kSprintf, watcom::clib::sprintf>(g.app(), g.ts(), text, 0x53c164, guest::Words({}));
    }
}

nfs::u32 frontend::memViewerDescribeBlock(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_498a40", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 index = args.eax, kind = args.edx, text = args.ebx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::memViewerDescribeBlock(app, ts, index, kind, text); });
}

/* sub_498b70: the entries of a pack by what they are: eax the pack, edx the number of
 * them, ebx what is no picture (hot spots, palette animation, comments, palettes), ecx
 * the packed pictures, the stack's the others. */
nfs::u32 frontend::eaa::memViewerCountShapes(App* app, ThreadState& ts, u32 pack, u32 countAt, u32 others,
                                               u32 packed, u32 unpacked)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x1c);
    const u32 fp = g.frame() + 0x10;
    g.setDword(countAt, fe::direct<0x4ea654, simc::packCount>(app, ts, pack));
    u32 i = 0;
    for (; std::int32_t(i) < std::int32_t(g.dword(countAt)); ++i)
    {
        fe::direct<0x4ea665, wina::tableEntry>(app, ts, pack, i, fp - 0x10);
        const u32 shape = guest::direct<0x4ef1e0, eab::shapeFileFind>(app, ts, pack, fp - 0x10);
        const u32 type = g.dword(shape) & 0x7f;
        u32 counter;
        if (type == 0x7c || type == 0x6e || type == 0x6f || (type & 0x70) == 0x20)
            counter = others;
        else
            counter = (g.dword(shape) & 0x80) != 0 ? packed : unpacked;
        g.setDword(counter, g.dword(counter) + 1);
    }
    return countAt;
}

nfs::u32 frontend::memViewerCountShapes(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_498b70", 4, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const u32 pack = args.eax, countAt = args.edx, others = args.ebx, packed = args.ecx;
    const u32 unpacked = stackWord(app, ts, 0);
    return runThunk(app, ts, args, site, [&](Guest&) {
        return eaa::memViewerCountShapes(app, ts, pack, countAt, others, packed, unpacked);
    });
}

/* sub_498bf0: the menu along the bottom of what to do with the block, by arrows and
 * Enter. */
nfs::u32 frontend::eaa::memViewerMenu(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x2c);
    Console console(g);
    const u32 text = g.frame();
    const std::int32_t count = std::int32_t(g.dword(kMenuCount));
    std::int32_t chosen = 0;

    const auto drawEntry = [&](std::int32_t i) {
        if (i == chosen)
            console.colours(0x12, 0x11);
        else
            console.colours(0x10, 0xf);
        sprintf1(g, text, 0x53c1c0, u32(i) * 12 + kMenuEntries);
        console.begin();
        console.print({ u32(i) * 9, g.dword(kRowsTotal), text });
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
        const u32 key = console.key();
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
            fe::pointerCall(g, g.dword(u32(chosen) * 12 + kMenuEntries + 8));
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

nfs::u32 frontend::memViewerMenu(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_498bf0", 0, 0, nullptr, 0, kRestoreAll };
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::memViewerMenu(app, ts); });
}

/* sub_498e10: the next pool that exists. */
void frontend::eaa::memViewerNextClass(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    u32 pool = g.dword(kPoolShown);
    do
        pool = (pool + 1) & 0xf;
    while (g.dword(pool * 4 + kPools) == 0);
    g.setDword(kPoolShown, pool);
}

nfs::u32 frontend::memViewerNextClass(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_498e10", 0, 0, nullptr, 0, kRestoreEdx };
    return runThunk(app, ts, args, site, [&](Guest&) { eaa::memViewerNextClass(app, ts); return args.eax; });
}

/* sub_498e40: a block's flags in eax as letters at edx. */
nfs::u32 frontend::eaa::memViewerFlagText(App* app, ThreadState& ts, u32 flags, u32 text)
{
    Guest g(app, ts);
    g.setByte(text, (flags & 0x200) != 0 ? 'e' : ' ');
    g.setByte(text + 1, (flags & 0x2000) != 0 ? 'b' : ' ');
    g.setByte(text + 3, ' ');
    g.setByte(text + 4, ' ');
    g.setByte(text + 2, (flags & 0x10) != 0 ? 'h' : ' ');
    g.setByte(text + 5, (flags & 0x40) != 0 ? 'a' : ' ');
    const u32 last = (flags & 0x8000) != 0 ? 0x73 : 0x20;
    g.setByte(text + 7, ' ');
    g.setByte(text + 8, ' ');
    g.setByte(text + 9, ' ');
    g.setByte(text + 6, std::uint8_t(last));
    return last;
}

nfs::u32 frontend::memViewerFlagText(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_498e40", 0, 0, nullptr, 0, kRestoreEcx };
    const u32 flags = args.eax, text = args.edx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::memViewerFlagText(app, ts, flags, text); });
}

/* sub_498f00: choosing the column to sort by: the arrows move along the header, Up
 * sorts down it and Down up it, Escape leaves. */
nfs::u32 frontend::eaa::memViewerSortMode(App* app, ThreadState& ts, u32 blocks)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x1c);
    Console console(g);
    const u32 listed = g.dword(kListed);
    g.setDword(kSortChoosing, 1);
    g.setDword(kSortColumn, 0);
    g.setDword(kSortAscending, 1);
    guest::direct<0x498090, memViewerHeader>(app, ts);
    u32 result = blocks;  // eax as the code leaves it: the last call's

    const auto sort = [&] {
        // Column 4 has nothing to sort by.
        if (g.dword(kSortColumn) <= 6 && g.dword(kSortColumn) != 4)
            fe::direct<0x4ee4f8, watcom::clib::qsort>(g.app(), g.ts(), g.dword(kBlockTable), listed, 8, 0x499150);
    };
    bool done = false;
    while (!done)
    {
        bool headerChanged = false;
        g.setDword(kRedrawAll, 0);
        const u32 key = console.key();
        result = key;
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
            const u32 column = g.dword(kSortColumn);
            if (std::int32_t(column) > 0)
            {
                headerChanged = true;
                g.setDword(kSortColumn, column - 1);
            }
            const u32 first = g.dword(kFirstColumn);
            if (std::int32_t(first) > 2)
            {
                const std::int32_t now = std::int32_t(g.dword(kSortColumn));
                if (now < std::int32_t(first) && now > 1)
                {
                    headerChanged = true;
                    const u32 last = g.dword(kLastColumn);
                    g.setDword(kRedrawAll, 1);
                    g.setDword(kFirstColumn, first - 1);
                    g.setDword(kLastColumn, last - 1);
                }
            }
            break;
        }
        case kKeyRight:
        {
            const u32 last = g.dword(kLastColumn);
            if (last == g.dword(kSortColumn) && std::int32_t(last) < 8)
            {
                headerChanged = true;
                g.setDword(kRedrawAll, 1);
                g.setDword(kFirstColumn, g.dword(kFirstColumn) + 1);
                const u32 column = g.dword(kSortColumn);
                g.setDword(kLastColumn, last + 1);
                g.setDword(kSortColumn, column + 1);
            }
            if (std::int32_t(g.dword(kLastColumn)) > std::int32_t(g.dword(kSortColumn)))
            {
                const u32 first = g.dword(kFirstColumn);
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
        // Its result is what the calls leave in eax.
        if (headerChanged)
            result = guest::direct<0x498090, memViewerHeader>(app, ts);
        if (g.dword(kRedrawAll) == 1)
            result = guest::direct<0x498370, memViewerRows>(app, ts);
    }
    return result;
}

nfs::u32 frontend::memViewerSortMode(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_498f00", 0, 0, nullptr, 0, kRestoreAll };
    const u32 blocks = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::memViewerSortMode(app, ts, blocks); });
}

/* sub_499150: qsort's comparison of two entries of the block table, by the sort column,
 * the way the sort goes (0x55e5a8): eax and edx the entries. */
nfs::u32 frontend::eaa::memViewerCompare(App* app, ThreadState& ts, u32 a, u32 b, u32 esi, u32 ebx)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x30);
    const bool ascending = g.dword(kSortAscending) != 0;
    // What is compared, first and then on a tie; columns 5 and 6 compare what the
    // registers held.
    std::int32_t first = std::int32_t(esi), second = std::int32_t(ebx);
    std::int32_t tieFirst = 0, tieSecond = 0;
    bool tie = false;
    const u32 column = g.dword(kSortColumn);
    switch (column)
    {
    case 1:  // by name: free space has none
    {
        const auto nameOf = [&](u32 block) {
            if ((blockFlags(g, block) & 0x4000) != 0)
                return u32(0x53c1c8);
            return directKept<0x4e15e0, ea::blockNameOf, 1>(g.app(), g.ts(), block + 0x10);
        };
        const u32 nameA = nameOf(g.dword(a));
        const u32 nameB = nameOf(g.dword(b));
        return fe::direct<0x4ee310, watcom::clib::strcmp>(g.app(), g.ts(), ascending ? nameA : nameB, ascending ? nameB : nameA);
    }
    case 2:  // by size, then where
    {
        const u32 blockA = g.dword(a), blockB = g.dword(b);
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
        const u32 blockA = g.dword(a), blockB = g.dword(b);
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
        return u32(-1);
    if (!tie)
        return 0;
    if (tieFirst < tieSecond)
        return 1;
    return tieFirst == tieSecond ? 0 : u32(-1);
}

nfs::u32 frontend::memViewerCompare(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_499150", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 a = args.eax, b = args.edx, esi = args.esi, ebx = args.ebx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::memViewerCompare(app, ts, a, b, esi, ebx); });
}

/* sub_499330: the pool's numbers: its total, used, free and largest free into what eax,
 * edx, ebx and ecx point at. */
nfs::u32 frontend::eaa::memPoolTotals(App* app, ThreadState& ts, u32 totalAt, u32 usedAt, u32 freeAt, u32 largestAt)
{
    Guest g(app, ts);
    const u32 head = firstBlock(g);
    const u32 pool = g.dword(g.dword(kPoolShown) * 4 + kPools);
    g.setDword(totalAt, g.dword(pool + 0xc) - head);
    u32 largest = 0, used = 0, available = 0;
    for (u32 block = firstBlock(g); block != 0; block = g.dword(block + 8))
    {
        const std::uint8_t kind = g.byte(block + 3);
        if ((kind & 0x40) != 0)
        {
            const u32 size = g.dword(block + 4);
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
    return largestAt;
}

nfs::u32 frontend::memPoolTotals(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_499330", 0, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const u32 totalAt = args.eax, usedAt = args.edx, freeAt = args.ebx, largestAt = args.ecx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::memPoolTotals(app, ts, totalAt, usedAt, freeAt, largestAt); });
}

/* sub_4993b0: the first block of the pool shown. */
nfs::u32 frontend::eaa::memPoolFirstBlock(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    return firstBlock(g);
}

nfs::u32 frontend::memPoolFirstBlock(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4993b0", 0, 0, nullptr, 0, 0 };
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::memPoolFirstBlock(app, ts); });
}

/* sub_4993d0: how many blocks of the two kinds the pool has. */
nfs::u32 frontend::eaa::memPoolCountBlocks(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    u32 count = 0;
    for (u32 block = firstBlock(g); block != 0; block = g.dword(block + 8))
    {
        const std::uint16_t tag = g.word(block);
        if (tag == kUsedBlock || tag == kFreeBlock)
            ++count;
    }
    return count;
}

nfs::u32 frontend::memPoolCountBlocks(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4993d0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEdx };
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::memPoolCountBlocks(app, ts); });
}

/* sub_499400: the next block, or the first if the list is to start again: where it is,
 * its size and flags to what edx, ebx and ecx point at, 0 to the stack's. */
nfs::u32 frontend::eaa::memPoolNextBlock(App* app, ThreadState& ts, u32 blockAt, u32 sizeAt, u32 flagsAt, u32 zeroAt)
{
    Guest g(app, ts);
    u32 block = g.dword(kListCursor);
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
    const u32 size = g.dword(block + 4);
    g.setDword(sizeAt, size);
    g.setDword(kListCursor, block);
    return size;
}

nfs::u32 frontend::memPoolNextBlock(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_499400", 4, 0, nullptr, 0, kRestoreEsi };
    const u32 blockAt = args.edx, sizeAt = args.ebx, flagsAt = args.ecx;
    const u32 zeroAt = stackWord(app, ts, 0);
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::memPoolNextBlock(app, ts, blockAt, sizeAt, flagsAt, zeroAt); });
}

/* sub_499460: a block's size, the selected one's for none. */
nfs::u32 frontend::eaa::viewerBlockSize(App* app, ThreadState& ts, u32 block)
{
    Guest g(app, ts);
    return blockSize(g, block);
}

nfs::u32 frontend::viewerBlockSize(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_499460", 0, 0, nullptr, 0, kRestoreEdx };
    const u32 block = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::viewerBlockSize(app, ts, block); });
}

/* sub_499480: a block's data. */
nfs::u32 frontend::eaa::memBlockData(App* app, ThreadState& ts, u32 block)
{
    return blockData(block);
}

nfs::u32 frontend::memBlockData(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_499480", 0, 0, nullptr, 0, 0 };
    const u32 block = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::memBlockData(app, ts, block); });
}

/* sub_499490: a block's flags. */
nfs::u32 frontend::eaa::viewerBlockFlags(App* app, ThreadState& ts, u32 block)
{
    Guest g(app, ts);
    return blockFlags(g, block);
}

nfs::u32 frontend::viewerBlockFlags(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_499490", 0, 0, nullptr, 0, 0 };
    const u32 block = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::viewerBlockFlags(app, ts, block); });
}

/* sub_4994a0: what a listed block is: 0 free, 1 the system's, 2 unknown, 3 a pack of
 * shapes, 4 or 5 unknown high or low. */
nfs::u32 frontend::eaa::memBlockKind(App* app, ThreadState& ts, u32 index)
{
    Guest g(app, ts);
    const u32 block = listedBlock(g, index);
    const u32 flags = blockFlags(g, block);
    const u32 size = blockSize(g, block);
    if (blockIsFree(g, block))
        return 0;
    if ((flags & 0x8000) != 0)
        return 1;
    if (size == 0)
        return 2;
    const u32 tag = tagOf(g, block + 0x10);
    if (tag == kShpi || tag == kShpf)
        return 3;
    return (flags & 0x10) != 0 ? 4 : 5;
}

nfs::u32 frontend::memBlockKind(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4994a0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi };
    const u32 index = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::memBlockKind(app, ts, index); });
}

/* sub_499580: 1 if a block is free space. */
nfs::u32 frontend::eaa::memBlockIsFree(App* app, ThreadState& ts, u32 block)
{
    Guest g(app, ts);
    return blockIsFree(g, block) ? 1 : 0;
}

nfs::u32 frontend::memBlockIsFree(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_499580", 0, 0, nullptr, 0, 0 };
    const u32 block = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::memBlockIsFree(app, ts, block); });
}

/* sub_4995b0: VIEW: the shapes of a pack one at a time, Left and Right, until Escape; or a
 * bitmap, or a window. */
nfs::u32 frontend::eaa::memViewerViewShapes(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x64);
    Console console(g);
    const u32 fp = g.frame() + 0x4c;
    const u32 nameOut = fp - 0x4c;
    const u32 countAt = fp - 0x3c, othersAt = fp - 0x38, packedAt = fp - 0x34, unpackedAt = fp - 0x30;
    for (const u32 at : { countAt, othersAt, packedAt, unpackedAt })
        g.setDword(at, 0);
    const u32 block = fe::direct<0x4f46b0, wata::screenPixels>(app, ts);

    const auto centred = [&](u32 text, u32 shorterBy) {
        const u32 columns = g.dword(kColumnsTotal), rows = g.dword(kRowsTotal);
        console.begin();
        guest::direct<0x4ef1b0, eab::drawClearClip>(app, ts, guest::direct<0x4ef760, eab::colourPack>(app, ts, 0xff000000u));
        console.colours(2, 1);
        console.print({ u32(s32(columns - shorterBy) >> 1), u32(s32(rows) >> 1), text });
        console.clear(0, g.dword(kRowsTotal), g.dword(kColumnsTotal), 1, 0xe);
        console.colours(0xd, 0xe);
    };
    const auto shown = [&](u32 shape, u32 bits) {
        console.begin();
        const u32 x = u32(s32(console.metric(2) - highWord(g, shape + 2)) >> 1);
        const u32 y = u32(s32(console.metric(3) - highWord(g, shape + 4)) >> 1);
        console.drawShape(shape, bits, x, y);
        console.clear(0, g.dword(kRowsTotal), g.dword(kColumnsTotal), 1, 0xe);
        console.colours(0xd, 0xe);
    };

    const u32 tag = tagOf(g, block);
    if (tag == kShpi || tag == kShpf)
    {
        guest::direct<0x498b70, memViewerCountShapes>(app, ts, block, countAt, othersAt, packedAt, unpackedAt);
        std::int32_t index = 0;
        bool done = false;
        while (!done)
        {
            index = index % std::int32_t(g.dword(countAt));
            fe::direct<0x4ea665, wina::tableEntry>(app, ts, block, u32(index), nameOut);
            const u32 shape = guest::direct<0x4ef1e0, eab::shapeFileFind>(app, ts, block, nameOut);
            const u32 type = g.dword(shape) & 0x7f;
            const u32 bits = g.byte(type + kShapeBits);
            const auto narrow = [&] { return std::int32_t(g.dword(kColumnsTotal)) < 0x46; };
            // The numbers of a picture, on the line below it.
            const auto pictureLine = [&] {
                const u32 packedInfo = g.dword(shape + 0xc);
                console.print({ 0, g.dword(kRowsTotal), narrow() ? 0x53c1d4u : 0x53c200u, nameOut,
                                highWord(g, shape + 2), highWord(g, shape + 4), bits, g.dword(shape) & 0xff,
                                u32(s32(packedInfo << 20) >> 20), u32(s32(packedInfo << 4) >> 20),
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

            const u32 key = console.key();
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
        if (guest::direct<0x499d30, memViewerViewBitmap>(app, ts) == 0)
            guest::direct<0x499b30, memViewerViewWindow>(app, ts);
        console.key();
    }
    console.begin();
    guest::direct<0x4ef1b0, eab::drawClearClip>(app, ts, guest::direct<0x4ef760, eab::colourPack>(app, ts, 0xff000000u));
    return console.end();
}

nfs::u32 frontend::memViewerViewShapes(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4995b0", 0, 0, nullptr, 0, kRestoreAll };
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::memViewerViewShapes(app, ts); });
}

/* sub_499b30: VIEW of a window: its bitmap and the numbers of it, or why not. */
nfs::u32 frontend::eaa::memViewerViewWindow(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x1c);
    Console console(g);
    const u32 window = fe::direct<0x4f46b0, wata::screenPixels>(app, ts);
    u32 valid = g.dword(window) == 0x57494e44 ? 1 : 0;
    if (valid != 0)
        valid = fe::direct<0x4f4a80, wata::windowCheck>(app, ts, window);

    const auto saying = [&](u32 text) {
        const u32 x = g.dword(kColumnsTotal) - g.length(text);
        const u32 rows = g.dword(kRowsTotal);
        console.begin();
        guest::direct<0x4ef1b0, eab::drawClearClip>(app, ts, guest::direct<0x4ef760, eab::colourPack>(app, ts, 0xff000000u));
        console.colours(2, 1);
        console.print({ x >> 1, u32(s32(rows) >> 1), text });
        return console.end();
    };
    if (valid == 0)
        return saying(0x53c31c);
    const u32 shape = g.dword(window + 0x20);
    const u32 bits = shapeBits(g, shape);
    if (bits == 0)
        return saying(0x53c350);

    console.begin();
    const u32 across = console.metric(2);
    const u32 x = u32(s32(across - highWord(g, shape + 2)) >> 1);
    const u32 down = console.metric(3);
    const u32 y = u32(s32(down - highWord(g, shape + 4)) >> 1);
    console.drawShape(shape, bits, x, y);
    console.clear(0, g.dword(kRowsTotal), g.dword(kColumnsTotal), 1, 0xe);
    console.colours(0xd, 0xe);
    console.print({ 0, g.dword(kRowsTotal), 0x53c388, g.dword(window + 4), g.dword(window + 8),
                    g.byte(window + 0x1c), g.byte(window + 0x1d),
                    g.byte(window + 0x1e) == 0 ? 0x53c378u : 0x53c380u });
    return console.end();
}

nfs::u32 frontend::memViewerViewWindow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_499b30", 0, 0, nullptr, 0, kRestoreAll };
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::memViewerViewWindow(app, ts); });
}

/* sub_499d30: VIEW of a bitmap, if the block is the size of one: its bits a pixel back,
 * 0 if it has none. */
nfs::u32 frontend::eaa::memViewerViewBitmap(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x30);
    Console console(g);
    const u32 shape = fe::direct<0x4f46b0, wata::screenPixels>(app, ts);
    const u32 name = directKept<0x4e15e0, ea::blockNameOf, 1>(g.app(), g.ts(), shape);
    const u32 size = blockSize(g, shape - 0x10);
    const u32 bits = shapeBits(g, shape);
    if (bits < 8)
        return 0;
    const u32 area = highWord(g, shape + 2) * highWord(g, shape + 4) * (bits >> 3);
    if (size != area + 0x10)
        return bits;
    console.begin();
    const u32 x = u32(s32(console.metric(2) - highWord(g, shape + 2)) >> 1);
    const u32 y = u32(s32(console.metric(3) - highWord(g, shape + 4)) >> 1);
    console.drawShape(shape, bits, x, y);
    console.clear(0, g.dword(kRowsTotal), g.dword(kColumnsTotal), 1, 0xe);
    console.colours(0xd, 0xe);
    console.print({ 0, g.dword(kRowsTotal), 0x53c3b4, name, highWord(g, shape + 2), highWord(g, shape + 4), bits });
    console.end();
    return bits;
}

nfs::u32 frontend::memViewerViewBitmap(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_499d30", 0, 0, nullptr, 0, kRestoreAll };
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::memViewerViewBitmap(app, ts); });
}

/* sub_499e90: a shape's bits a pixel. */
nfs::u32 frontend::eaa::memShapeBits(App* app, ThreadState& ts, u32 shape)
{
    Guest g(app, ts);
    return shapeBits(g, shape);
}

nfs::u32 frontend::memShapeBits(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_499e90", 0, 0, nullptr, 0, 0 };
    const u32 shape = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::memShapeBits(app, ts, shape); });
}

/* sub_499ea0: HEX DUMP of the block, by arrows and pages, until any other key. */
nfs::u32 frontend::eaa::memViewerHexDump(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x14);
    Console console(g);
    u32 address = fe::direct<0x4f46b0, wata::screenPixels>(app, ts);
    const u32 perRow = std::int32_t(g.dword(kColumnsTotal)) < 0x50 ? 8 : 0x10;
    guest::direct<0x499f70, memViewerHexPage>(app, ts, address, perRow);
    bool done = false;
    while (!done)
    {
        const u32 key = console.key();
        const u32 page = g.dword(kRowsTotal) * perRow;
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
        guest::direct<0x499f70, memViewerHexPage>(app, ts, address, perRow);
    }
    console.begin();
    console.clear(0, 0, g.dword(kColumnsTotal), g.dword(kRowsTotal) + 1, 1);
    return console.end();
}

nfs::u32 frontend::memViewerHexDump(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_499ea0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx | kRestoreEsi };
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::memViewerHexDump(app, ts); });
}

/* sub_499f70: a screenful of the dump: eax the address, edx the bytes a row. */
nfs::u32 frontend::eaa::memViewerHexPage(App* app, ThreadState& ts, u32 base, u32 perRow)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x120);
    Console console(g);
    const u32 fp = g.frame() + 0x10c - 0x82;
    const u32 line = fp - 0x8a, piece = fp - 0xa;
    if (base == 0)
        return 0;
    u32 at = base;
    console.begin();
    console.clear(0, 0, g.dword(kColumnsTotal), g.dword(kRowsTotal) + 1, 1);
    console.colours(0, 1);
    u32 row = 0;
    for (; std::int32_t(row) < std::int32_t(g.dword(kRowsTotal)); ++row)
    {
        sprintf1(g, line, perRow == 8 ? 0x53c3dc : 0x53c3d4, perRow * row + base);
        for (std::int32_t i = 0; i < std::int32_t(perRow); ++i)
        {
            const u32 value = g.byte(at);
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
    return console.end();
}

nfs::u32 frontend::memViewerHexPage(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_499f70", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 base = args.eax, perRow = args.edx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::memViewerHexPage(app, ts, base, perRow); });
}

/* sub_49a120: P: the pool's numbers and blocks into a file of a free name. */
nfs::u32 frontend::eaa::memViewerWriteMap(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x37c);
    Console console(g);
    constexpr u32 kFprintf = 0x4f4b80;
    const u32 fp = g.frame() + 0x364 - 0x82;
    const u32 name = fp - 0xb6, line = fp - 0x1b6, numbers = fp - 0x36;
    const u32 dash = fp - 0x2e2, flagText = fp + 0x56, kindText = fp + 0x72, kinds = fp + 0x4a;
    for (u32 k = 0; k < 3; ++k)
        g.setDword(kinds + k * 4, g.dword(0x497610 + k * 4));
    g.copyString(name, 0x53c3f0);
    guest::direct<0x49a400, memViewerFreeFileName>(app, ts, name);  // 0x53c400 in edx too, which it does not read
    const u32 file = fe::direct<0x4edfe8, watcom::clib::fopen>(g.app(), g.ts(), name, 0x53c400);
    if (file == 0)
    {
        console.print({ 0, 0, 0x53c564 });
        return fe::direct<0x4ee100, watcom::clib::fclose>(g.app(), g.ts(), file);
    }
    // Where they are matters: what a free block's kind has after its one letter runs on
    // into these, and the file prints it so.
    g.setDword(fp + 0x76, file);
    guest::direct<0x499330, memPoolTotals>(app, ts, fp + 0x62, fp + 0x66, fp + 0x6a, fp + 0x6e);
    const auto hex = [&] { return g.dword(kHexNumbers) != 0; };
    fe::direct<kFprintf, watcom::clib::fprintf>(g.app(), g.ts(), file, hex() ? 0x53c404u : 0x53c41cu, guest::Words{g.dword(fp + 0x62)});
    fe::direct<kFprintf, watcom::clib::fprintf>(g.app(), g.ts(), file, hex() ? 0x53c434u : 0x53c44cu, guest::Words{g.dword(fp + 0x66)});
    fe::direct<kFprintf, watcom::clib::fprintf>(g.app(), g.ts(), file, hex() ? 0x53c464u : 0x53c47cu, guest::Words{g.dword(fp + 0x6a)});
    fe::direct<kFprintf, watcom::clib::fprintf>(g.app(), g.ts(), file, hex() ? 0x53c494u : 0x53c4b4u, guest::Words{g.dword(fp + 0x6e)});
    fe::direct<kFprintf, watcom::clib::fprintf>(g.app(), g.ts(), file, 0x53c4d4, guest::Words({}));
    for (u32 i = 0; std::int32_t(i) < std::int32_t(g.dword(kListed)); ++i)
    {
        g.setByte(dash, 0x2d);
        g.setByte(dash + 1, 0);
        const u32 block = listedBlock(g, i);
        const u32 size = blockSize(g, block);
        const u32 data = blockData(block);
        u32 owner = 0;
        g.setDword(fp + 0x7a, 0);
        guest::direct<0x498e40, memViewerFlagText>(app, ts, blockFlags(g, block), flagText);
        if (blockIsFree(g, block))
        {
            g.setByte(kindText, 0x20);
            fe::direct<kSprintf, watcom::clib::sprintf>(g.app(), g.ts(), line, 0x53c504, guest::Words({}));
        }
        else
        {
            const u32 entry = g.dword(kBlockTable) + i * 8;
            owner = g.dword(entry + 4);
            // The game's code hands it the owner in edx too, which it does not read.
            const u32 kind = fe::direct<0x4f47c0, wata::shapeEndsAtBend>(app, ts, g.dword(entry));
            sprintf1(g, kindText, kThreeLetters, g.dword(kinds + kind * 4));
            const u32 blockName = directKept<0x4e15e0, ea::blockNameOf, 1>(g.app(), g.ts(), listedBlock(g, i) + 0x10);
            fe::direct<kSprintf, watcom::clib::sprintf>(g.app(), g.ts(), line, 0x53c518, guest::Words{0, blockName});
        }
        fe::direct<kSprintf, watcom::clib::sprintf>(g.app(), g.ts(), numbers, hex() ? 0x53c524u : 0x53c544u, guest::Words{size, data, flagText, dash, 0, owner, kindText});
        append(g, line, numbers);
        fe::direct<kFprintf, watcom::clib::fprintf>(g.app(), g.ts(), file, line, guest::Words({}));
    }
    return fe::direct<0x4ee100, watcom::clib::fclose>(g.app(), g.ts(), file);
}

nfs::u32 frontend::memViewerWriteMap(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_49a120", 0, 0, nullptr, 0, kRestoreAll };
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::memViewerWriteMap(app, ts); });
}

/* sub_49a400: the file's name free: its ?? become the first two digits no file has, or
 * xx if there is none. */
nfs::u32 frontend::eaa::memViewerFreeFileName(App* app, ThreadState& ts, u32 name)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x1c);
    u32 place = 0;
    while (g.byte(name + place) != 0x3f)
        ++place;
    std::int32_t number = 0;
    for (; number < 100; ++number)
    {
        g.setByte(name + place, std::uint8_t(number / 10 + 0x30));
        g.setByte(name + place + 1, std::uint8_t(number % 10 + 0x30));
        if (!*fe::direct<0x4e0e60, ea::fileThere>(app, ts, name))
            break;
    }
    if (number >= 100)
    {
        g.setByte(name + place + 1, 0x78);
        g.setByte(name + place, g.byte(name + place + 1));
    }
    return 1;
}

nfs::u32 frontend::memViewerFreeFileName(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_49a400", 0, 0, nullptr, 0, kRestoreAll };
    const u32 name = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::memViewerFreeFileName(app, ts, name); });
}

namespace
{

/* What sub_4d0d50 copies of a glyph (sub_4efa70) into a font's tables. */
constexpr u32 kFontSmall = 0x840188;     // the small screen's font: a second one, nudged
constexpr u32 kFontNumber = 0x840184;
constexpr u32 kFontScale = 0x84018c;
constexpr u32 kFontShapes = 0x83f6a0;
constexpr u32 kLoadedShapes = 0x567218;

}  // namespace

/* sub_4d0d50: the text's fonts: their files by the screen's width, the widths and places
 * of their glyphs into the tables at 0x83f4e0 and on, and each made a texture. */
nfs::u32 frontend::eaa::textLoadFonts(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x21c);
    const u32 fp = g.frame() + 0x204 - 0x182;
    const u32 firstName = fp - 0x82, secondName = fp + 0x7e;
    constexpr u32 kFolder = 0x7a29b4;
    if (std::int32_t(g.dword(0x7cdac8)) >= 0x280)
    {
        fe::direct<kSprintf, watcom::clib::sprintf>(g.app(), g.ts(), firstName, 0x540b94, guest::Words{kFolder});
        g.setDword(kFontSmall, 0);
        g.setByte(secondName, 0);
    }
    else
    {
        fe::direct<kSprintf, watcom::clib::sprintf>(g.app(), g.ts(), firstName, 0x540ba4, guest::Words{kFolder});
        fe::direct<kSprintf, watcom::clib::sprintf>(g.app(), g.ts(), secondName, 0x540bb0, guest::Words{kFolder});
        g.setDword(kFontSmall, 1);
    }
    if (!*fe::direct<0x4e0e60, ea::fileThere>(app, ts, firstName))
        return 0;
    if (g.byte(secondName) != 0 && !*fe::direct<0x4e0e60, ea::fileThere>(app, ts, secondName))
        return 0;

    const u32 first = *fe::direct<0x4e0ec0, ea::loadWhole>(app, ts, firstName, 0x10u);
    guest::direct<0x4ef520, eab::fontUse>(app, ts, first);
    const u32 shapes = g.dword(kLoadedShapes);
    g.setDword(kFontNumber, 0);
    g.setDword(kFontScale, 0x3f800000);
    g.setDword(kFontShapes, shapes);
    for (u32 i = 0; i < 0xe0; ++i)
    {
        const u32 character = i + 0x20;
        const u32 glyph = guest::direct<0x4efa70, eab::fontGlyphFind>(app, ts, character);
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
    fe::direct<0x4d0a10, render::pictureTextured>(app, ts);
    u32 result = *fe::direct<0x4e1890, ea::purgeBlock>(app, ts, first);
    if (g.byte(secondName) == 0)
        return result;

    g.setDword(kFontNumber, 1);
    const u32 second = *fe::direct<0x4e0ec0, ea::loadWhole>(app, ts, secondName, 0x10u);
    guest::direct<0x4ef520, eab::fontUse>(app, ts, second);
    g.setDword(kFontShapes + 4, g.dword(kLoadedShapes));
    for (u32 i = 0; i < 0xe0; ++i)
    {
        const u32 glyph = guest::direct<0x4efa70, eab::fontGlyphFind>(app, ts, i + 0x20);
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
    fe::direct<0x4d0a10, render::pictureTextured>(app, ts);
    result = *fe::direct<0x4e1890, ea::purgeBlock>(app, ts, second);
    g.setDword(kFontNumber, 0);
    return result;
}

nfs::u32 frontend::textLoadFonts(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4d0d50", 0, 0, nullptr, 0, kRestoreAll };
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::textLoadFonts(app, ts); });
}

/* sub_4dc0e0: a udff opened on a file named in eax or the memory at edx, one and not both;
 * ebx whether what goes wrong is an error: a record of 12 bytes, or none. */
nfs::u32 frontend::eaa::udffOpen(App* app, ThreadState& ts, u32 name, u32 memory, u32 complain)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x1c);
    const u32 fp = g.frame() + 0xc;
    // By address: what it leaves in ebx and edx is what this function leaves there.
    const u32 record = *fe::direct<kReserve, ea::reserved>(app, ts, 0x549734u, 0xcu, 0x10u);
    if (record == 0)
    {
        if (complain != 0)
            fe::direct<0x401010, ea::error>(app, ts, 0x549738u, guest::Words({}));
        return 0;
    }
    if ((name == 0) == (memory == 0))
    {
        fe::direct<0x4e1890, ea::purgeBlock>(app, ts, record);
        if (complain != 0)
            fe::direct<0x401010, ea::error>(app, ts, 0x549758u, guest::Words({}));
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
    fe::direct<0x4e28d0, ea::openToRead>(app, ts, name, record + 4, fp - 0xc, fp - 8);
    if (g.dword(record + 4) != 0)
        return record;
    fe::direct<0x4e1890, ea::purgeBlock>(app, ts, record);
    return 0;
}

nfs::u32 frontend::udffOpen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4dc0e0", 0, 0, nullptr, 0, kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 name = args.eax, memory = args.edx, complain = args.ebx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::udffOpen(app, ts, name, memory, complain); });
}

/* sub_4dc1b0: a udff closed: its file, if it has one, and its record. */
nfs::u32 frontend::eaa::udffClose(App* app, ThreadState& ts, u32 record)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0xc);
    if (record == 0)
        return 0;
    if (g.dword(record) == 0)
    {
        const u32 handle = g.dword(record + 4);
        if (handle != 0)
            fe::direct<0x4e2da0, ea::closeChecked>(app, ts, handle);
    }
    return *fe::direct<0x4e1890, ea::purgeBlock>(app, ts, record);
}

nfs::u32 frontend::udffClose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4dc1b0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEdx };
    const u32 record = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::udffClose(app, ts, record); });
}

/* sub_4dc1e0: the next word of a udff. */
nfs::u32 frontend::eaa::udffReadWord(App* app, ThreadState& ts, u32 record)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x10);
    const u32 word = g.frame();
    if (g.dword(record) == 0)
    {
        // By address: the registers it leaves are what sub_4f2a7d hands on to exit.
        if (fe::direct<0x4e2de0, ea::readChecked>(app, ts, g.dword(record + 4), word, 4) == 0)
            guest::direct<0x4f2a7d, eab::exitGameNow>(app, ts);
    }
    else
    {
        g.setDword(word, g.dword(g.dword(record + 8)));
        g.setDword(record + 8, g.dword(record + 8) + 4);
    }
    return g.dword(word);
}

nfs::u32 frontend::udffReadWord(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4dc1e0", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEdx };
    const u32 record = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::udffReadWord(app, ts, record); });
}

/* sub_4e4360: a file's size, the way of opening it in edx. */
nfs::u32 frontend::eaa::fileSizeOpened(App* app, ThreadState& ts, u32 name, u32 mode)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x14);
    const u32 handleAt = g.frame(), sizeAt = g.frame() + 8;
    fe::direct<0x4e2ab0, ea::errorsAsOpened>(app, ts, name, handleAt, g.frame() + 4, sizeAt, mode);
    const u32 handle = g.dword(handleAt);
    if (handle != 0)
        fe::direct<0x4e2da0, ea::closeChecked>(app, ts, handle);
    return g.dword(sizeAt);
}

nfs::u32 frontend::fileSizeOpened(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4e4360", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx };
    const u32 name = args.eax, mode = args.edx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::fileSizeOpened(app, ts, name, mode); });
}

/* sub_4e43a0: a file's size. */
nfs::u32 frontend::eaa::fileSize(App* app, ThreadState& ts, u32 name)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 4);
    return guest::direct<0x4e4360, fileSizeOpened>(app, ts, name, 1u);
}

nfs::u32 frontend::fileSize(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4e43a0", 0, 0, nullptr, 0, kRestoreEdx };
    const u32 name = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::fileSize(app, ts, name); });
}

namespace
{

constexpr u32 kDisplay = 0x9f38d0;       // the display's record
constexpr u32 kDrawTarget = 0x564ff4;    // the bitmap being drawn into, 14 words
constexpr u32 kScreenBitmap = 0x565048;
constexpr u32 kStandInBitmap = 0x5643f0;
constexpr u32 kDrawLeft = 0x565000, kDrawTop = 0x565004, kDrawRight = 0x565008, kDrawBottom = 0x56500c;
constexpr u32 kDrawBits = 0x565010;
constexpr u32 kDrawHeight = 0x564ffc;
constexpr u32 kDrawCallback = 0x567dcc;

void copyWords(Guest& g, u32 to, u32 from, u32 words)
{
    for (u32 i = 0; i < words; ++i)
        g.setDword(to + i * 4, g.dword(from + i * 4));
}

/* What sub_4eada0 is: drawing goes into the bitmap in eax, or the screen for none. */
u32 drawInto(Guest& g, nfs::ThreadState& ts, u32 bitmap)
{
    u32 result = 1;
    if (bitmap != 0)
        result = fe::direct<0x4f4a80, frontend::wata::windowCheck>(g.app(), ts, bitmap);
    if (result == 0)
        return 0;
    if (const u32 callback = g.dword(kDrawCallback))
        result = fe::pointerCall(g, callback, args({ bitmap }));
    else if (bitmap == 0)
        guest::direct<0x4ed2a0, frontend::eab::ddrawUnlockBitmap>(g.app(), ts, kDisplay);
    else
        result = guest::direct<0x4ed0f0, frontend::eab::ddrawLockBitmap>(g.app(), ts, kDisplay, g.dword(bitmap + 0x34),
                                                                         bitmap);
    if (bitmap == 0)
    {
        fe::direct<0x4e070c, ea::clearBytes>(g.app(), g.ts(), kDrawTarget, 0x38);
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
                                       g.app(), ts, 8u, 8u, g.dword(0x56438c), g.dword(0x5643f4)));
    copyWords(g, kDrawTarget, g.dword(kStandInBitmap), 14);
    return result;
}

}  // namespace

/* sub_4ead90: drawing goes to the screen. */
nfs::u32 frontend::eaa::bitmapDrawToScreen(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x18);
    return drawInto(g, ts, 0);
}

nfs::u32 frontend::bitmapDrawToScreen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4ead90", 0, 0, nullptr, 0, kRestoreAll };
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::bitmapDrawToScreen(app, ts); });
}

/* sub_4eada0: drawing goes into a bitmap. */
nfs::u32 frontend::eaa::bitmapDrawInto(App* app, ThreadState& ts, u32 bitmap)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x18);
    return drawInto(g, ts, bitmap);
}

nfs::u32 frontend::bitmapDrawInto(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4eada0", 0, 0, nullptr, 0, kRestoreAll };
    const u32 bitmap = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::bitmapDrawInto(app, ts, bitmap); });
}

/* sub_4eaeb0: pixels read off what is drawn into, at their width: eax, edx the left and
 * top, ebx, ecx the size, the stack's where to. */
nfs::u32 frontend::eaa::bitmapReadPixels(App* app, ThreadState& ts, u32 left, u32 y, u32 width, u32 rows, u32 out)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x14);
    const u32 bits = (u32(g.byte(kDrawBits)) + 7) & ~u32(7);
    const u32 right = left + width;
    for (--rows; rows != u32(-1); --rows, ++y)
    {
        for (u32 x = left; std::int32_t(x) < std::int32_t(right); ++x)
        {
            const u32 pixel = fe::direct<0x503bc0, watb::readPixel>(app, ts, x, y);
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
    return right;
}

nfs::u32 frontend::bitmapReadPixels(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4eaeb0", 4, 0, nullptr, 0, kRestoreEsi | kRestoreEdi };
    const u32 left = args.eax, y = args.edx, width = args.ebx, rows = args.ecx;
    const u32 out = stackWord(app, ts, 0);
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::bitmapReadPixels(app, ts, left, y, width, rows, out); });
}

/* sub_4eaf60: the size of a TGA of what is drawn into. */
nfs::u32 frontend::eaa::bitmapTgaSize(App* app, ThreadState& ts)
{
    Guest g(app, ts);
    const u32 bits = (u32(g.byte(kDrawBits)) + 7) & ~u32(7);
    const u32 width = g.dword(kDrawRight) - g.dword(kDrawLeft);
    const u32 height = g.dword(kDrawBottom) - g.dword(kDrawTop);
    const u32 pixelBits = width * height * bits;
    // A signed division by 8, towards zero.
    const u32 rounded = pixelBits + (std::int32_t(pixelBits) < 0 ? 7 : 0);
    u32 size = u32(s32(rounded) >> 3) + 0x12;
    if (bits == 8)
        size += 0x300;
    return size;
}

nfs::u32 frontend::bitmapTgaSize(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4eaf60", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdx };
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::bitmapTgaSize(app, ts); });
}

/* sub_4eafc0: a TGA of what is drawn into, at eax: its header, its palette, its pixels. */
nfs::u32 frontend::eaa::bitmapWriteTga(App* app, ThreadState& ts, u32 tga)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x320);
    const u32 palette = g.frame();
    const u32 width = g.dword(kDrawRight) - g.dword(kDrawLeft);
    const u32 height = g.dword(kDrawBottom) - g.dword(kDrawTop);
    const u32 bits = (u32(g.byte(kDrawBits)) + 7) & ~u32(7);
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
    u32 out = tga + 0x12;
    if (paletted)
    {
        fe::direct<0x503ec0, wina::paletteGet>(app, ts, 0u, 0x100u, palette);
        for (u32 i = 0; i < 0x300; i += 3)
        {
            g.setByte(out, g.byte(palette + i + 2));
            g.setByte(out + 1, g.byte(palette + i + 1));
            g.setByte(out + 2, g.byte(palette + i));
            out += 3;
        }
    }
    const u32 top = g.dword(kDrawTop);
    u32 result = guest::direct<0x4eaeb0, bitmapReadPixels>(app, ts, g.dword(kDrawLeft), top,
                                                             g.dword(kDrawRight) - g.dword(kDrawLeft),
                                                             g.dword(kDrawBottom) - top, out);
    if (g.byte(kDrawBits) == 0x10)
        result = fe::direct<0x5040b8, rendf::row565To1555>(app, ts, out, out, width * height);
    return result;
}

nfs::u32 frontend::bitmapWriteTga(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4eafc0", 0, 0, nullptr, 0, kRestoreAll };
    const u32 tga = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::bitmapWriteTga(app, ts, tga); });
}

/* sub_4eb160: what is drawn into saved as a TGA to the file named in eax; the file's
 * save's answer, 0 without the memory for it. */
nfs::u32 frontend::eaa::bitmapSaveTga(App* app, ThreadState& ts, u32 name)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x84);
    const u32 saved = g.frame();
    const u32 size = guest::direct<0x4eaf60, bitmapTgaSize>(app, ts);
    const u32 buffer = *fe::direct<0x4e19d0, ea::defaultReserved>(app, ts, size);
    if (buffer == 0)
        return 0;
    guest::direct<0x4eafc0, bitmapWriteTga>(app, ts, buffer);
    fe::direct<0x4f46d0, wata::windowSave>(app, ts, saved);
    guest::direct<0x4ead90, bitmapDrawToScreen>(app, ts);
    const u32 result = *fe::direct<0x4eeae0, load::blockSaved>(app, ts, name, buffer, size);
    fe::direct<0x4f4700, wata::windowRestore>(app, ts, saved);
    fe::direct<0x4e19f0, ea::defaultPurged>(app, ts, buffer);
    return result;
}

nfs::u32 frontend::bitmapSaveTga(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4eb160", 0, 0, nullptr, 0, kRestoreAll };
    const u32 name = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::bitmapSaveTga(app, ts, name); });
}

/* sub_4eb1c0: a key's state, by its number. */
nfs::u32 frontend::eaa::keyStateByte(App* app, ThreadState& ts, u32 key)
{
    Guest g(app, ts);
    return g.byte(key + 0x564818);
}

nfs::u32 frontend::keyStateByte(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4eb1c0", 0, 0, nullptr, 0, 0 };
    const u32 key = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::keyStateByte(app, ts, key); });
}

namespace
{

/* The library's log of DirectDraw. */
u32 report(Guest& g, u32 level, u32 text, std::initializer_list<u32> values = {})
{
    // eax: the report's answer.
    return *fe::direct<0x501050, ea::reportedAt>(g.app(), g.ts(), 0x564ed4u, level, text, guest::Words(values));
}

/* The palette's colours at `address` in the game's memory, as the display takes them. */
const os::video::Colour* coloursAt(Guest& g, u32 address)
{
    return &g.app()->getMemory<const os::video::Colour>(address);
}

constexpr u32 kDisplayLock = 0x564480;
constexpr u32 kDisplayError = 0x56825c;
constexpr u32 kModeCount = 0x9f09a0, kModesDefault = 0x9f09a8;
constexpr u32 kCapabilities = 0x564ed8;
constexpr u32 kDisplayChanged = 0x567184;

// Offsets in the display's record.
constexpr u32 kClipper = 0x4, kModesRoom = 0x8, kModeIndex = 0xc, kModes = 0x10, kWidth = 0x14, kHeight = 0x18;
constexpr u32 kBitsWanted = 0x1c, kPalette = 0x24, kPaletteEntries = 0x30;
constexpr u32 kPrimary = 0x444, kBack = 0x448, kPrimaryWindow = 0x44c, kBackWindow = 0x450, kWindow = 0x454;
constexpr u32 kBits = 0x45c, kFullscreen = 0x45d, kModeX = 0x45f, kPages = 0x463;

}  // namespace

/* sub_4eb430: DirectDrawCreate for the driver edx names; the object, and how many have
 * been tried, told to the log.  The port has one display, whatever the driver: it is always
 * made (no error, 0). */
nfs::u32 frontend::eaa::ddrawCreate(App* app, ThreadState& ts, u32 driver)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x10);
    const u32 object = os::video::openDisplay(app);
    const u32 tries = g.dword(0x9f3d3c) + 1;
    g.setDword(0x9f3d3c, tries);
    report(g, 0xd, 0x54b73c, { driver, object, tries, 0 });
    return object;
}

nfs::u32 frontend::ddrawCreate(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4eb430", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEdi };
    const u32 driver = args.edx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::ddrawCreate(app, ts, driver); });
}

/* sub_4eb490: the primary surface of a display, as asked in edx, else whatever will do. */
nfs::u32 frontend::eaa::ddrawMakePrimary(App* app, ThreadState& ts, u32 display, u32 wanted)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x1c);
    const auto make = [&](u32 backs, u32 flags) {
        return fe::direct<0x4ed480, menz::ddrawMakeSurface>(app, ts, display, g.dword(display + kWidth),
                                                            g.dword(display + kHeight), g.dword(display + kBitsWanted),
                                                            backs, 0u, flags, g.dword(display + kPalette));
    };
    const auto backs = [&] { return u32(g.byte(display + kPages)) - 1; };
    report(g, 0xe, 0x54b77c);
    u32 surface = make(backs(), wanted);
    if (surface != 0)
        return surface;
    u32 flags = wanted & ~u32(0x4800);
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
    flags &= ~u32(0x4800);
    g.setByte(display + kPages, 1);
    return make(0, flags);
}

nfs::u32 frontend::ddrawMakePrimary(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4eb490", 0, 0, nullptr, 0, kRestoreEbx | kRestoreEcx | kRestoreEsi | kRestoreEdi };
    const u32 display = args.eax, wanted = args.edx;
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::ddrawMakePrimary(app, ts, display, wanted); });
}

/* sub_4eb5e0: a surface locked: the display, the surface, and where its pixels are and their
 * layout in `locked`.  The error back: the port's surfaces are never lost, so a lock always
 * comes (0), and the library's way back from a lost one -- the surfaces restored, the display
 * made again, the palette given again -- is never taken.  A handle that is no surface does not
 * lock (1, as the port's DirectDraw answered a method it did not have).  It is called only as
 * C++: its description is the port's (os::video::Locked), which no caller by address could
 * hand. */
nfs::u32 frontend::eaa::ddrawLockSurface(App* app, ThreadState& ts, u32 display, u32 surface,
                                           os::video::Locked* locked)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x10);
    NFS2_USE(display);
    const std::optional<os::video::Locked> had = os::video::lock(app, surface);
    if (!had)
        return 1;
    *locked = *had;
    return 0;
}

/* sub_4eb880: the bits set in eax. */
nfs::u32 frontend::eaa::countBits(App* app, ThreadState& ts, u32 value)
{
    u32 count = 0;
    for (u32 bits = value; bits != 0; bits &= bits - 1)
        ++count;
    return count;
}

nfs::u32 frontend::countBits(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4eb880", 0, 0, nullptr, 0, kRestoreEdx };
    const u32 value = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::countBits(app, ts, value); });
}

/* sub_4eb8a0: a mode (as a surface's description had it) into the display's list of modes at
 * the index edx, if there is one; with ecx it is the mode now, too.  A mode of the port's tells
 * its pixel format and its refresh, and neither back buffers nor any of the caps the list
 * marks. */
nfs::u32 frontend::eaa::ddrawNoteMode(App* app, ThreadState& ts, u32 display, u32 index,
                                        const os::video::Format& mode, u32 current)
{
    Guest g(app, ts);
    const auto popcount = [](u32 bits) {
        u32 count = 0;
        for (; bits != 0; bits &= bits - 1)
            ++count;
        return count;
    };
    // eax and ecx as the code leaves them, their last values kept for the callers.
    u32 leftEax, leftEcx = current;
    u32 bits;
    const std::uint8_t bitCount = std::uint8_t(mode.bits);
    if (mode.bits == 0x10)
    {
        bits = popcount(mode.redMask);
        bits += popcount(mode.greenMask);
        leftEax = popcount(mode.blueMask);
        bits += leftEax;
        leftEcx = bits;
    }
    else
    {
        bits = bitCount;
        leftEax = bits;
    }
    const std::uint8_t bitsByte = std::uint8_t(bits);
    if (std::int32_t(index) >= 0)
    {
        const u32 offset = index * 20;
        g.setDword(g.dword(display + kModes) + offset, mode.width);
        g.setDword(g.dword(display + kModes) + offset + 4, mode.height);
        g.setByte(g.dword(display + kModes) + offset + 0x12, bitCount);
        g.setByte(g.dword(display + kModes) + offset + 0x13, bitsByte);
        g.setWord(g.dword(display + kModes) + offset + 0x10, 1);
        g.setDword(g.dword(display + kModes) + offset + 0xc, g.dword(kCapabilities));
        // The refresh, which the port's modes always tell.
        leftEcx = g.dword(display + kModes) + offset;
        leftEax = mode.refresh;
        g.setDword(leftEcx + 8, leftEax);
    }
    if (current != 0)
    {
        if (std::int32_t(index) >= 0)
        {
            leftEax = index * 20;
            const std::uint8_t clip = g.byte(display + 0x460);
            const u32 modes = g.dword(display + kModes);
            const u32 at = modes + leftEax + 0xc;
            g.setByte(at, clip != 0 ? g.byte(at) | 0x10 : g.byte(at) & 0xef);
        }
        if (g.byte(display + kFullscreen) != 0)
        {
            g.setDword(display + kWidth, mode.width);
            g.setDword(display + kHeight, mode.height);
        }
        leftEax = mode.bits;
        g.setDword(display + kBitsWanted, leftEax);
        leftEax = (leftEax & 0xffffff00) | bitsByte;
        g.setByte(display + kBits, bitsByte);
    }
    NFS2_USE(leftEcx);
    return leftEax;
}

/* sub_4ebaf0: DirectDraw, the driver chosen or the first or the emulation, its second
 * interface, and what its capabilities mean to the library.  The port's display is the
 * interface the library asks for, and it has none of the card's capabilities the library looks
 * for (overlays, blits, stretching, ...): their report says 0, and the library's are none. */
nfs::u32 frontend::eaa::ddrawInit(App* app, ThreadState& ts, u32 given)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x2f0);
    constexpr u32 kDriver = 0x9f0990, kNames = 0x564390;
    const u32 display = given != 0 ? given : kDisplay;
    if (g.dword(kDisplayLock) == 0)
        g.setDword(kDisplayLock, *fe::direct<0x4eb2e0, ea::newLock>(app, ts));
    const u32 specific = g.dword(kDriver) | g.dword(kDriver + 4) | g.dword(kDriver + 8) | g.dword(kDriver + 0xc);
    u32 first = 0;
    if ((g.byte(kNames) & 0x80) == 0)
    {
        if (specific != 0)
        {
            report(g, 1, 0x54b8f8);
            first = guest::direct<0x4eb430, ddrawCreate>(app, ts, kDriver);  // the display in eax too, not read
        }
        if (first == 0)
        {
            report(g, 1, 0x54b918);
            first = guest::direct<0x4eb430, ddrawCreate>(app, ts, 0u);  // the display in eax too, not read
        }
    }
    if (first == 0)
    {
        report(g, 1, 0x54b934);
        first = guest::direct<0x4eb430, ddrawCreate>(app, ts, 2u);  // the display in eax too, not read
        if (first == 0)
        {
            g.setDword(display, 0);
            g.setDword(kDisplayError, 0x54b948);
            return 0;
        }
    }
    // The display is its own second interface: no error.
    g.setDword(display, first);
    report(g, 0xd, 0x54b960, { 0 });
    report(g, 0xd, 0x54b98c, { 0, 0, 0 });
    g.setDword(kCapabilities, 0);
    g.setDword(kDisplayError, 0);
    return g.dword(display) != 0 ? 1 : 0;
}

nfs::u32 frontend::ddrawInit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4ebaf0", 0, 0, nullptr, 0, kRestoreAll };
    const u32 given = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::ddrawInit(app, ts, given); });
}

/* sub_4ebe50: the display taken: its cooperative level, the list of its modes, a clipper
 * if it is in a window.  1 if all went well, else why at 0x56825c. */
nfs::u32 frontend::eaa::ddrawAccessDisplay(App* app, ThreadState& ts, u32 given)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x18);
    const u32 display = given != 0 ? given : kDisplay;
    const auto failed = [&](u32 why) {
        g.setDword(kDisplayError, why);
        return u32(0);
    };
    if (g.dword(display) == 0)
        return failed(0x54b9f8);
    if (g.dword(display + kWindow) == 0)
        return failed(0x54ba18);
    /* Fullscreen and exclusive, or in a window: the port's display is shown in the window either
     * way, and it is always had (no error). */
    report(g, 0xd, g.byte(display + kFullscreen) != 0 ? 0x54ba34 : 0x54ba68);
    os::video::showIn(app, g.dword(display), g.dword(display + kWindow));
    report(g, 2, 0x54ba9c, { 0 });

    /* The display's modes, each handed to the library's callback (sub_4edbc0) with the display:
     * all of them, whatever the callback answers, as the port has always told them. */
    const auto tellModes = [&] {
        for (const os::video::Format& mode : os::video::modes())
            guest::direct<0x4edbc0, menz::ddrawEnumModeCallback>(app, ts, mode, display);
        return u32(0);
    };
    g.setDword(kModeCount, 0);
    report(g, 8, 0x54bacc);
    tellModes();
    report(g, 8, 0x54bb10, { g.dword(kModeCount) });
    if (const u32 old = g.dword(display + kModes))
    {
        fe::direct<0x4e1890, ea::purgeBlock>(app, ts, old);
        g.setDword(display + kModes, 0);
    }
    const u32 counted = g.dword(kModeCount);
    g.setDword(display + kModesRoom, counted);
    if (std::int32_t(counted) < 0x10)
        g.setDword(display + kModesRoom, 0x10);
    g.setDword(0x552198, 0x1c5);
    g.setDword(0x552194, 0x54bb28);
    g.setDword(0x552190, 0x54bb1c);
    const u32 modes =
        *fe::direct<0x4e1620, ea::reserved>(app, ts, 0x54bb3cu, g.dword(display + kModesRoom) * 20, g.dword(0x5643f4));
    g.setDword(display + kModes, modes);
    if (modes == 0)
    {
        g.setDword(kDisplayError, 0x54bb48);
        return 0;
    }
    g.setDword(kModesDefault, 0);
    g.setDword(kModeCount, 0);
    const u32 enumerated = tellModes();
    report(g, 4, 0x54bb70, { g.dword(kModeCount), g.dword(kModesDefault), enumerated });
    g.setDword(display + 0x28, 0x300);
    g.setDword(display + 0x2c, 0x100);
    g.setDword(display + kModesRoom, g.dword(kModeCount));
    // A window's clipper, to keep its pictures inside it: the port's window is the screen, it has none.
    g.setDword(display + kClipper, 0);
    g.setDword(kDisplayError, 0);
    return 1;
}

nfs::u32 frontend::ddrawAccessDisplay(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4ebe50", 0, 0, nullptr, 0, kRestoreAll };
    const u32 given = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::ddrawAccessDisplay(app, ts, given); });
}

/* sub_4ec0d0: a display's surfaces and their windows gone. */
nfs::u32 frontend::eaa::ddrawRemoveSurfaces(App* app, ThreadState& ts, u32 display)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x18);
    report(g, 1, 0x54bbcc);
    // The system's own picture flipped back to the front first: the port's display has no other.
    if (const u32 window = g.dword(display + kBackWindow))
        fe::direct<0x508e70, menz::videoWindowFree>(app, ts, window);
    if (const u32 window = g.dword(display + kPrimaryWindow))
        fe::direct<0x508e70, menz::videoWindowFree>(app, ts, window);
    g.setDword(display + kPrimaryWindow, 0);
    const u32 back = g.dword(display + kBack);
    g.setDword(display + kBackWindow, 0);
    if (back != 0)
        fe::direct<0x4ed730, menz::ddrawFreeSurface>(app, ts, display, back);
    if (const u32 primary = g.dword(display + kPrimary))
        fe::direct<0x4ed730, menz::ddrawFreeSurface>(app, ts, display, primary);
    g.setDword(display + kBack, 0);
    g.setDword(display + kPrimary, 0);
    report(g, 0xa, 0x54bbfc);
    return 1;
}

nfs::u32 frontend::ddrawRemoveSurfaces(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4ec0d0", 0, 0, nullptr, 0, kRestoreAll };
    const u32 display = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::ddrawRemoveSurfaces(app, ts, display); });
}

/* sub_4ec1a0: the display configured as its record says: what was made gone, the palette,
 * the primary surface, the back buffer and the windows on them. */
nfs::u32 frontend::eaa::ddrawConfigDisplay(App* app, ThreadState& ts, u32 given)
{
    Guest g(app, ts);
    const fe::Frame frame(g, ts, 0x360);
    constexpr u32 kError = 0x401010;
    constexpr u32 kLibraryMemory = 0x436, kVideoMemory = 0x435, kSystemMemory = 0x434, kPagesWanted = 0x437;
    constexpr u32 kForceDoubleBuffer = 0x45e;
    report(g, 1, 0x54bc20);
    const u32 display = given != 0 ? given : kDisplay;
    if (g.dword(display + 0x440) != 0)
    {
        g.setDword(0x552190, 0x54bb1c);
        g.setDword(0x552194, 0x54bc3c);
        g.setDword(0x552198, 0x201);
        fe::direct<kError, ea::error>(g.app(), g.ts(), 0x54bc50, guest::Words({}));
    }
    fe::enterLock(g, ts, g.dword(kDisplayLock));
    guest::direct<0x4ec0d0, ddrawRemoveSurfaces>(app, ts, display);  // the caps in edx too, which it does not read

    const std::uint8_t pagesWanted = g.byte(display + kPagesWanted);
    if (pagesWanted < 1)
        g.setByte(display + kPagesWanted, 1);
    else if (pagesWanted > 2)
    {
        g.setDword(kDisplayError, 0x54bc94);
        fe::leaveLock(g, ts, g.dword(kDisplayLock));
        return 0;
    }
    // Whether the display can clip and flip in hardware: the port's display has neither of those caps.
    g.setByte(display + 0x460, 0);
    g.setByte(display + 0x462, 0);

    u32 flags = 0;
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

    if (const u32 palette = g.dword(display + kPalette))
        os::video::setPalette(app, palette, 0, 0x100, coloursAt(g, display + kPaletteEntries));
    else
        g.setDword(display + kPalette,
                   os::video::makePalette(app, g.dword(display), coloursAt(g, display + kPaletteEntries)));
    g.setWord(display + 0x432, 0xff);
    g.setByte(display + kModeX, 0);
    const std::uint8_t library = g.byte(display + kLibraryMemory);
    g.setWord(display + 0x430, 0);

    const u32 memoryName = library != 0 ? 0x54bcb8 : (flags & 0x4000) != 0 ? 0x54bcc0 : 0x54bcc8;
    const u32 kindName = g.byte(display + kPages) > 1 && (flags & 0x800) == 0 ? 0x54bcd0 : 0x54bcdc;
    report(g, 8, 0x54bcec, { kindName, memoryName });

    u32 surface = guest::direct<0x4eb490, ddrawMakePrimary>(app, ts, display, flags);
    bool modeX = true;
    if (surface != 0)
    {
        // A surface of the port's is never in Mode X.
        modeX = false;
        g.setByte(display + kModeX, 0);
    }
    else
        g.setByte(display + kModeX, 1);
    if (modeX)
    {
        report(g, 0xe, 0x54bd10);
        g.setByte(display + kPagesWanted, 2);
        g.setByte(display + kPages, g.byte(display + kPagesWanted));
        if (surface != 0)
            fe::direct<0x4ed730, menz::ddrawFreeSurface>(app, ts, display, surface);
        surface = guest::direct<0x4eb490, ddrawMakePrimary>(app, ts, display, flags);
    }
    if (surface == 0)
    {
        g.setDword(kDisplayError, 0x54bd38);
        fe::leaveLock(g, ts, g.dword(kDisplayLock));
        return 0;
    }
    // The clipper set on it: the port has none (ddrawAccessDisplay).  The surface's mode noted as the mode now.
    guest::direct<0x4eb8a0, ddrawNoteMode>(app, ts, display, g.dword(display + kModeIndex), os::video::mode(), 1u);

    const std::uint8_t pages = g.byte(display + kPages);
    g.setDword(display + kPrimary, surface);
    if (pages > 1)
        // The display in edx too, which it does not read.
        g.setDword(display + kBack, fe::direct<0x4ed770, menz::ddrawBackSurface>(app, ts, surface));
    else if (g.byte(display + kLibraryMemory) == 0 && g.byte(display + kPagesWanted) > 1)
    {
        if (g.byte(display + kVideoMemory) == 0 && g.byte(display + kSystemMemory) == 0)
        {
            report(g, 0xe, 0x54bd5c);
            g.setDword(display + kBack, fe::direct<0x4ed480, menz::ddrawMakeSurface>(
                                            app, ts, display, g.dword(display + kWidth), g.dword(display + kHeight),
                                            g.dword(display + kBitsWanted), 0u, 0u, 0x4040u, 0u));
        }
        if (g.dword(display + kBack) == 0)
        {
            report(g, 0xe, 0x54bd84);
            g.setDword(display + kBack, fe::direct<0x4ed480, menz::ddrawMakeSurface>(
                                            app, ts, display, g.dword(display + kWidth), g.dword(display + kHeight),
                                            g.dword(display + kBitsWanted), 0u, 0u, 0x840u, 0u));
        }
    }

    /* A video window on a surface: its pitch the mode's, where its pixels are not known until it is
     * locked (0). */
    const auto windowOn = [&](u32 surfaceAt, u32 which) {
        return fe::direct<0x508c70, menz::videoWindowMake>(app, ts, g.dword(display + kWidth), g.dword(display + kHeight),
                                                           u32(os::video::mode().pitch), u32(g.byte(display + kBits)),
                                                           which, 0u, g.dword(surfaceAt));
    };
    if (g.dword(display + kPrimary) != 0)
        g.setDword(display + kPrimaryWindow, windowOn(display + kPrimary, u32(-1)));
    if (g.dword(display + kBack) != 0)
        g.setDword(display + kBackWindow, windowOn(display + kBack, u32(-2)));
    else if (g.byte(display + kPagesWanted) > 1)
    {
        report(g, 4, 0x54bdac);
        const u32 window = guest::direct<0x4ef190, eab::windowBitmapCreateDefault>(
            app, ts, g.dword(display + kWidth), g.dword(display + kHeight), u32(g.byte(display + kBits)),
            g.dword(display + 0x20));
        g.setDword(display + kBackWindow, window);
        copyWords(g, 0x565064, window, 14);
    }

    const u32 flipping = g.byte(display + kPages) > 1 ? 0x46 : 0x20;
    const u32 system = g.byte(display + kSystemMemory) != 0 ? 0x42 : 0x20;
    const u32 memory = g.byte(display + kLibraryMemory) != 0 ? 0x4c : g.byte(display + kVideoMemory) != 0 ? 0x44 : 0x56;
    const u32 modeXLetter = g.byte(display + kModeX) != 0 ? 0x58 : 0x20;
    report(g, 8, 0x54bdd8, { g.dword(display + kWidth), g.dword(display + kHeight), g.dword(display + kBitsWanted),
                             g.byte(display + kBits), g.dword(g.dword(display + kPrimaryWindow) + 0x28),
                             g.byte(display + kPagesWanted), modeXLetter, memory, system, flipping });
    fe::leaveLock(g, ts, g.dword(kDisplayLock));
    if (const u32 changed = g.dword(kDisplayChanged))
        fe::pointerCall(g, changed);
    return 1;
}

nfs::u32 frontend::ddrawConfigDisplay(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site = { "sub_4ec1a0", 0, 0, nullptr, 0, kRestoreAll };
    const u32 given = args.eax;
    return runThunk(app, ts, args, site, [&](Guest&) { return eaa::ddrawConfigDisplay(app, ts, given); });
}

}  // namespace nfs3hp
