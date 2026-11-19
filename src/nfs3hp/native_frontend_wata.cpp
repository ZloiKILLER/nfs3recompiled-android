#include "native_frontend_wata.h"
#include "native_ea_net.h"
#include "native_watcom.h"
#include "native_game.h"
#include "native_carstate.h"
#include "native_controls_input.h"
#include "native_ea_files.h"
#include "native_ea_formats.h"
#include "native_ea_memory.h"
#include "native_ea_threads.h"
#include "native_frontend_eaa.h"
#include "native_frontend_eab.h"
#include "native_frontend_glue.h"
#include "native_frontend_hudr.h"
#include "native_frontend_hudz.h"
#include "native_frontend_loopa.h"
#include "native_frontend_menz.h"
#include "native_frontend_mova.h"
#include "native_frontend_netb.h"
#include "native_frontend_rende.h"
#include "native_frontend_rendf.h"
#include "native_frontend_simd.h"
#include "native_frontend_watb.h"
#include "native_frontend_watc.h"
#include "native_frontend_wina.h"
#include "native_load_saves.h"
#include "port_snd_stream_calls.h"
#include "port_text.h"
#include "native_frontend.h"
#include "native_frontend_pcall.h"
#include "port_timer.h"
#include <nfs3hp.h>
#include <game_float.h>
#include "os_files.h"
#include "os_system.h"
#include "native_render.h"
struct SDL_AtomicInt;
struct SDL_Semaphore;
#include <lib/thread.h>
#include <cstdint>
#include <cstring>
#include <optional>
#include <type_traits>
#include <utility>
#include <cstddef>

/* The thunks defined below by qualified name, declared where they are defined. */
namespace nfs3hp
{
namespace frontend {
nfs::u32 carDataReadInteger(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 carDataReadByte(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 carDataReadFloat(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 carDataReadFixed(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 carDataLineHasValues(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 carDataFindKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 carDataNextLine(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 carDataFillDefaults(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 carDataRead(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 exceptionName(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 npsLoadDll(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 npsDevelopMode(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 npsDllPath(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 npsEnsureDll(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 installFileRead(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 findFirstFile(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 findNextFile(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 carBadgeColour(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 windowCheckField(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 windowCheck(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 pathInDirectory(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 textMeasureBox(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 lockKeySet(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 commandLineRead(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 discCheck(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 driveIsCd(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 driveNumberOfPath(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 asyncRequestHasId(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 asyncNoRequest(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 asyncCancelCurrent(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 asyncRunCallbacks(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 asyncInit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 asyncShutdown(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 asyncSetFile(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 asyncReadAt(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 asyncRead(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 asyncReadStatus(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 asyncThreadMain(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 timerNow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 timerSinceLast(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 timerWait(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 textDepth(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 textSetPen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 textSetAttribute(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 mixD4Bit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 mixD16Bit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 mixD32Bit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 mixD24Bit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 blitDispatch(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 blitShapeAtPosition(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 surfaceFillAndDraw(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 textMeasureGlyph(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 textMeasure(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 registryAddEntry(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 screenPixels(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 windowSave(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 windowRestore(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 textStateSave(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 textStateRestore(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 blockChecksum(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 shapeEndsAtBend(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 thrashOpenByName(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 windowReshow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 heapStart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 heapStartDefault(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 fontsStart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tapiInitialize(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 tapiSetup(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 screenTakeSize(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 screenLockCountUp(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 screenLockCountDown(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 screenInstallDriver(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 numLockOff(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 bigFileOpenEntry(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 windowApply(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 exitFunctionsRun(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 nothingA(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 nothingB(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 nothingC(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 graphicsStart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 skyLoad(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudEditorDrop(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudEditorDrag(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
}  // frontend
}  // namespace nfs3hp

/* What sat among the C runtime in the rough map, decompiled, most of it the game's
 * and EA's own libraries:
 *
 *   the car's data      carp.txt read into a car's performance record, field by field,
 *                       and what the file left out filled from the default record;
 *   the HUD editor      an element dragged or resized with the mouse;
 *   the file loader     EA's asynchronous reader, its thread and its queues;
 *   the rest            the command line, the error box's text, the window's fields, the
 *                       timer, and the small functions the libraries share.
 *
 * All of the game's state stays in guest memory.  Each function's typed core takes its
 * arguments as parameters and hands back its result; what the game's code leaves in the
 * registers it does not save is not kept (no caller reads it), and where a function hands
 * back eax as it came its thunk does that.  Its calls of typed cores are C++ (fe::direct);
 * by address only what is called through a pointer and what has no typed core yet. */
#if defined(__clang__)
#pragma clang fp contract(off)
#else
#pragma fp_contract(off)
#endif

namespace nfs3hp
{

namespace
{

using fe::args;
using fe::Guest;
using fe::Site;
using nfs::Float;
using nfs::u16;
using nfs::u32;
using nfs::u8;
using nfs::s32;

// The C library's character classes, indexed by the character plus one: this bit is a digit.
constexpr u32 kCharacterClass = 0x564ef0;
constexpr std::uint8_t kClassDigit = 0x20;

bool isDigit(const Guest& guest, u32 classIndex)
{
    return (guest.byte(kCharacterClass + classIndex) & kClassDigit) != 0;
}

void copyDwords(Guest& guest, u32 to, u32 from, u32 count)
{
    for (u32 i = 0; i < count; ++i)
        guest.setDword(to + 4 * i, guest.dword(from + 4 * i));
}

/* A float moved as `fld dword; fstp dword` moves it. */
void moveFloat(Guest& guest, u32 to, u32 from)
{
    guest.setFloat(to, guest.floatAt(from));
}

/* What a wait does each time round, as the game's code has it at the head of its loop:
 * the game's lock let go, the thread's turn given up, and a safepoint. */
void giveWayInWait(nfs::App* app, nfs::ThreadState& ts)
{
    app->unlockContext(ts);
    nfs::Thread::sleep(0);
    app->lockContext(ts);
    if (app->contextWanted())
        app->yieldContext(ts);
}

/* ---------------------------------------------------------------------------
 * carp.txt: a car's performance data
 * ------------------------------------------------------------------------- */

constexpr u32 kAtoi = 0x4f0430, kStrtod = 0x4f82b6, kTolower = 0x4f0ae0, kStrstr = 0x4e0820, kSprintf = 0x4df690;
constexpr u32 kMemset = 0x4e0640;
constexpr u32 kReadInteger = 0x4b0700, kReadByte = 0x4b07b0;

constexpr u32 kCarFileOffset = 0x55ed88;    // how far into the file the reader has come
constexpr u32 kCarFieldRead = 0x55ed8c;     // a byte a field, 1 when the file gave it
constexpr u32 kCarDefaults = 0x55f874;      // a record laid out as the car's, the defaults
constexpr u32 kCarFieldCount = 0x50;

enum class CarValue : std::uint8_t
{
    Integer,
    Float,
    Byte,
    Fixed,      // a float in the file, 16.16 in the record
};

/* A field of the file: where its values go in the record, how many there are, and whether
 * the reader's code happens to reload the record's address in edx after storing one. */
struct CarField
{
    CarValue kind;
    std::uint16_t offset;
    std::uint8_t count;
    bool leavesRecordInEdx;
};

constexpr CarValue I = CarValue::Integer, F = CarValue::Float, B = CarValue::Byte, X = CarValue::Fixed;

constexpr CarField kCarFields[kCarFieldCount] = {
    { I, 0, 1, false },       // the first: an integer handed back to the caller, not in the record
    { I, 0x000, 1, true },   { F, 0x004, 1, false }, { I, 0x008, 1, true },    { I, 0x00c, 1, true },
    { I, 0x010, 1, true },   { I, 0x014, 8, true },  { I, 0x034, 8, true },    { F, 0x094, 8, false },
    { F, 0x0b4, 8, true },   { F, 0x054, 8, false }, { F, 0x074, 8, true },    { F, 0x0d4, 8, false },
    { F, 0x0f4, 8, true },   { F, 0x114, 0x29, false }, { F, 0x1b8, 1, false }, { F, 0x1bc, 1, false },
    { I, 0x1c0, 1, true },   { I, 0x1c4, 1, true },  { F, 0x1c8, 1, false },   { F, 0x1cc, 1, false },
    { F, 0x1d0, 1, false },  { B, 0x1d4, 1, true },  { F, 0x1d8, 1, false },   { F, 0x1dc, 1, false },
    { B, 0x1e0, 8, false },  { B, 0x1e8, 8, false }, { F, 0x1f0, 8, false },   { F, 0x210, 8, true },
    { F, 0x230, 1, false },  { F, 0x234, 1, false }, { I, 0x238, 1, true },    { F, 0x23c, 1, false },
    { F, 0x240, 1, false },  { F, 0x244, 1, false }, { F, 0x248, 1, false },   { F, 0x24c, 1, false },
    { F, 0x250, 1, false },  { F, 0x254, 1, false }, { F, 0x258, 1, false },   { I, 0x25c, 3, true },
    { I, 0x268, 3, true },   { F, 0x274, 1, false }, { F, 0x278, 1, false },   { F, 0x27c, 1, false },
    { F, 0x280, 1, false },  { I, 0x284, 1, true },  { I, 0x288, 1, true },    { F, 0x28c, 1, false },
    { F, 0x290, 1, false },  { F, 0x294, 1, false }, { F, 0x298, 1, false },   { I, 0x29c, 1, true },
    { F, 0x2a0, 1, false },  { I, 0x2a4, 1, true },  { I, 0x2a8, 1, true },    { I, 0x2ac, 1, true },
    { F, 0x2b0, 1, false },  { F, 0x2b4, 1, false }, { F, 0x2b8, 1, false },   { I, 0x2bc, 1, true },
    { X, 0x2c0, 1, true },   { F, 0x2c4, 1, false }, { F, 0x2c8, 1, false },   { F, 0x2cc, 1, false },
    { F, 0x2d0, 1, false },  { F, 0x2d4, 1, false }, { F, 0x2d8, 1, false },   { F, 0x2dc, 1, false },
    { F, 0x2e0, 1, false },  { F, 0x2e4, 1, false }, { F, 0x2e8, 1, false },
    // Eight fields of fourteen floats, a table in eight rows.
    { F, 0x2ec + 0x38 * 0, 14, false }, { F, 0x2ec + 0x38 * 1, 14, true }, { F, 0x2ec + 0x38 * 2, 14, false },
    { F, 0x2ec + 0x38 * 3, 14, true },  { F, 0x2ec + 0x38 * 4, 14, false }, { F, 0x2ec + 0x38 * 5, 14, true },
    { F, 0x2ec + 0x38 * 6, 14, false }, { F, 0x2ec + 0x38 * 7, 14, true },
};

constexpr u32 kFieldReturned = 0;           // handed back through the caller's pointer
constexpr u32 kFieldMarkedPerValue = 26;    // its code marks it read after every value
constexpr u32 kFieldDefaultOddOne = 1;      // from elsewhere, not the default record
constexpr u32 kFieldDefaultWrongPlace = 16; // its default goes to the field before it
constexpr u32 kFieldFirstTableRow = 72;

/* The `wanted`-th number of a line into `buffer`, as sub_4b0700 and sub_4b07d0 pick it: runs of
 * digits and the like, counted by what separates them, the end of the line one of those. */
template <typename Numeric>
void gatherNumber(Guest& guest, u32 line, u32 wanted, u32 buffer, Numeric isNumeric)
{
    u32 length = 0;
    std::int16_t separators = 0;
    for (u32 at = line; std::int32_t(separators) <= s32(wanted); ++at)
    {
        const std::uint8_t c = guest.byte(at);
        if (isNumeric(c))
        {
            if (s32(wanted) == std::int32_t(separators))
                guest.setByte(buffer + length++, c);
        }
        else
        {
            ++separators;
        }
    }
    guest.setByte(buffer + length, 0);
}

}

/* sub_4b0700: the wanted integer of a line of values, 0 if what it found has anything but
 * digits and minus signs in it. */
u32 frontend::wata::carDataReadInteger(App* app, ThreadState& ts, u32 line, u32 wanted)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 5 * 4 + 0x68);
    const u32 buffer = guest.frame();
    gatherNumber(guest, line, wanted, buffer, [&](std::uint8_t c) {
        return isDigit(guest, std::uint8_t(c + 1)) || c == '-';
    });
    const u32 value = fe::direct<kAtoi, watcom::clib::atoi>(guest.app(), guest.ts(), buffer);
    for (u32 at = buffer;; ++at)
    {
        const std::uint8_t c = guest.byte(at);
        if (c == 0)
            return value;
        if (!isDigit(guest, std::uint8_t(c + 1)) && c != '-')
            return 0;
    }
}

nfs::u32 frontend::carDataReadInteger(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4b0700", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 line = args.eax, wanted = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::carDataReadInteger(app, ts, line, wanted); });
}

/* sub_4b07b0: the same, for a field the caller keeps a byte of. */
u32 frontend::wata::carDataReadByte(App* app, ThreadState& ts, u32 line, u32 wanted)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    return fe::direct<kReadInteger, carDataReadInteger>(app, ts, line, wanted);
}

nfs::u32 frontend::carDataReadByte(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4b07b0", 0, 0, nullptr, 0, 0 };
    const u32 line = args.eax, wanted = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::carDataReadByte(app, ts, line, wanted); });
}

/* A float's bits as the float, as fld dword loads it. */
static Float floatOfBits(u32 bits)
{
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return Float(value);
}

/* sub_4b07d0: the wanted float of a line of values, its bits in eax (the float itself its thunk
 * leaves on the x87, as the code does); 0 if what it found has anything but digits, points and minus
 * signs in it. */
u32 frontend::wata::carDataReadFloat(App* app, ThreadState& ts, u32 line, u32 wanted)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 5 * 4 + 0x70);
    const u32 buffer = guest.frame();
    const u32 parsed = buffer + 0x68, result = buffer + 0x6c;
    gatherNumber(guest, line, wanted, buffer, [&](std::uint8_t c) {
        return isDigit(guest, std::uint8_t(c + 1)) || c == '.' || c == '-';
    });
    guest.setFloat(parsed, float(fe::direct<kStrtod, watcom::clib::strtod>(app, ts, buffer, 0u, nullptr)));
    u32 bits = guest.dword(parsed);
    for (u32 at = buffer;; ++at)
    {
        const std::uint8_t c = guest.byte(at);
        if (c == 0)
            break;
        const u32 classIndex = std::uint8_t(c + 1);
        if (!isDigit(guest, classIndex) && c != '.' && c != '-')
        {
            bits = 0;
            break;
        }
    }
    guest.setDword(result, bits);
    return bits;
}

nfs::u32 frontend::carDataReadFloat(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4b07d0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 line = args.eax, wanted = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        const u32 bits = wata::carDataReadFloat(app, ts, line, wanted);
        args.floatSet = true;
        args.floatResult = floatOfBits(bits);
        return bits;
    });
}

/* sub_4b08b0: a float of the file as 16.16 fixed point. */
u32 frontend::wata::carDataReadFixed(App* app, ThreadState& ts, u32 line, u32 wanted)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    constexpr u32 kToFixed = 0x53eac0;    // a double
    const Float value = floatOfBits(fe::direct<0x4b07d0, carDataReadFloat>(app, ts, line, wanted));
    const Float scale = Float(app->getMemory<double>(kToFixed));
    return u32(fp::toInteger<s32>(fp::mul(value, scale)));
}

nfs::u32 frontend::carDataReadFixed(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4b08b0", 0, 0, nullptr, 0, 0 };
    const u32 line = args.eax, wanted = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::carDataReadFixed(app, ts, line, wanted); });
}

/* sub_4b0900: whether a line starts with a digit or a minus sign: values, not a key. */
u32 frontend::wata::carDataLineHasValues(App* app, ThreadState& ts, u32 line)
{
    Guest guest(app, ts);
    const std::uint8_t first = guest.byte(line);
    return isDigit(guest, std::uint8_t(first + 1)) || first == '-' ? 1 : 0;
}

nfs::u32 frontend::carDataLineHasValues(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4b0900", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const u32 line = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::carDataLineHasValues(app, ts, line); });
}

/* sub_4b0940: which field a line of the file is the key of: its name and its number in brackets
 * both somewhere in the line, by the table at 0x4af3b0.  1 and the field's index through the
 * pointer, or 0. */
u32 frontend::wata::carDataFindKey(App* app, ThreadState& ts, u32 line, u32 indexOut)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 5 * 4 + 0x68);
    constexpr u32 kKeys = 0x4af3b0;           // { char* name; int16 number; } a field, 8 bytes each
    constexpr u32 kNumberFormat = 0x53ead0;
    constexpr u32 kEmptyBuffer = 0x4af630;    // what the local buffer starts as
    const u32 numbered = guest.frame();
    copyDwords(guest, numbered, kEmptyBuffer, 0x19);
    for (u32 field = 0; field < kCarFieldCount; ++field)
    {
        const u32 key = kKeys + field * 8;
        const u32 name = guest.dword(key);
        if (name == 0)
            continue;
        const u32 number = u32(s32(guest.dword(key + 2)) >> 16);
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), numbered, kNumberFormat, guest::Words{number});
        if (fe::direct<kStrstr, watcom::clib::strstr>(guest.app(), guest.ts(), line, name) == 0)
            continue;
        if (fe::direct<kStrstr, watcom::clib::strstr>(guest.app(), guest.ts(), line, numbered) != 0)
        {
            guest.setDword(indexOut, field);
            return 1;
        }
    }
    return 0;
}

nfs::u32 frontend::carDataFindKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4b0940", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 line = args.eax, indexOut = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::carDataFindKey(app, ts, line, indexOut); });
}

/* sub_4b09d0: the next line of the file, lowered and without its spaces, into `out`; where the
 * file stands is kept at 0x55ed88 and handed back. */
u32 frontend::wata::carDataNextLine(App* app, ThreadState& ts, u32 text, u32 out, u32 length)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4 * 4 + 8);
    const s32 last = s32(length - 1);
    u32 position = guest.dword(kCarFileOffset);
    if (text != 0 && s32(position) <= last)
    {
        std::uint8_t c = guest.byte(text + position);
        while ((c == 0 || c == '\r' || c == '\n') && s32(position) < last)
        {
            ++position;
            c = guest.byte(text + position);
        }
        u32 written = 0;
        while (c != '\r' && c != '\n' && s32(position) < last)
        {
            guest.setDword(kCarFileOffset, position);
            if (c != ' ')
            {
                const u32 lowered = fe::direct<kTolower, watcom::clib::tolower>(guest.app(), guest.ts(), c);
                ++written;
                guest.setByte(out + written - 1, u8(lowered));
            }
            position = guest.dword(kCarFileOffset) + 1;
            c = guest.byte(text + position);
        }
        guest.setByte(out + written, 0);
    }
    guest.setDword(kCarFileOffset, position);
    return position;
}

nfs::u32 frontend::carDataNextLine(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4b09d0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 text = args.eax, out = args.edx, length = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::carDataNextLine(app, ts, text, out, length); });
}

/* sub_4b0bb0: the fields the file did not give, from the default record at 0x55f874; or, with
 * edx not 0, the whole default record. */
u32 frontend::wata::carDataFillDefaults(App* app, ThreadState& ts, u32 record, u32 whole)
{
    Guest guest(app, ts);
    constexpr u32 kRecordBytes = 0x4ac;
    constexpr u32 kFirstField = 0x55f3c8;
    constexpr u32 kTableFloats = 0x70;
    if (record == 0)
        return 0;
    if (whole != 0)
    {
        copyDwords(guest, record, kCarDefaults, kRecordBytes / 4);
        return kRecordBytes;
    }
    for (u32 index = 0; index < kCarFieldCount; ++index)
    {
        if (guest.byte(kCarFieldRead + index) != 0 || index == kFieldReturned)
            continue;
        const CarField& field = kCarFields[index];
        const u32 from = kCarDefaults + field.offset, to = record + field.offset;
        if (index == kFieldDefaultOddOne)
            guest.setDword(record, guest.dword(kFirstField));
        else if (index == kFieldDefaultWrongPlace)
            guest.setDword(to - 4, guest.dword(from));
        else if (index >= kFieldFirstTableRow)
        {
            const u32 table = kCarFields[kFieldFirstTableRow].offset;
            for (u32 i = 0; i < kTableFloats; ++i)
                moveFloat(guest, record + table + 4 * i, kCarDefaults + table + 4 * i);
        }
        else if (field.count == 1 && field.kind != CarValue::Byte)
            guest.setDword(to, guest.dword(from));
        else
            for (u32 i = 0; i < field.count; ++i)
            {
                if (field.kind == CarValue::Byte)
                    guest.setByte(to + i, guest.byte(from + i));
                else if (field.kind == CarValue::Float)
                    moveFloat(guest, to + 4 * i, from + 4 * i);
                else
                    guest.setDword(to + 4 * i, guest.dword(from + 4 * i));
            }
    }
    return record;
}

nfs::u32 frontend::carDataFillDefaults(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4b0bb0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 record = args.eax, whole = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::carDataFillDefaults(app, ts, record, whole); });
}

/* sub_4b1380: carp.txt into a car's performance record: lines of keys, each followed by a line of
 * values.  The result is how many fields it gave; what it left out comes from the defaults. */
u32 frontend::wata::carDataRead(App* app, ThreadState& ts, u32 record, u32 text, u32 length, u32 firstOut)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 3 * 4 + 0x7e4);
    constexpr u32 kEmptyLine = 0x4af694;      // what the line's buffer starts as
    constexpr u32 kLineDwords = 0x1f4;
    const u32 line = guest.frame();
    const u32 fieldAt = line + kLineDwords * 4;
    copyDwords(guest, line, kEmptyLine, kLineDwords);
    guest.setDword(fieldAt, 0);
    if (text == 0 || length == 0)
        return 0;
    guest.setDword(kCarFileOffset, 0);
    fe::direct<kMemset, watcom::clib::memset>(guest.app(), guest.ts(), kCarFieldRead, 0, kCarFieldCount);

    const auto readField = [&](u32 index) {
        const CarField& field = kCarFields[index];
        const u32 to = record + field.offset;
        for (u32 i = 0; i < field.count; ++i)
        {
            if (field.kind == CarValue::Integer)
                guest.setDword(to + 4 * i, fe::direct<0x4b0700, carDataReadInteger>(app, ts, line, i));
            else if (field.kind == CarValue::Fixed)
                guest.setDword(to + 4 * i, fe::direct<0x4b08b0, carDataReadFixed>(app, ts, line, i));
            else if (field.kind == CarValue::Byte)
                guest.setByte(to + i, u8(fe::direct<kReadByte, carDataReadByte>(app, ts, line, i)));
            else
            {
                guest.setFloat(to + 4 * i, float(floatOfBits(fe::direct<0x4b07d0, carDataReadFloat>(app, ts, line, i))));
            }
            if (index == kFieldMarkedPerValue)
                guest.setByte(kCarFieldRead + guest.dword(fieldAt), 1);
        }
    };

    u32 position = 0;
    bool keyFound = false;
    while (position < length - 1)
    {
        position = fe::direct<0x4b09d0, carDataNextLine>(app, ts, text, line, length);
        if (guest::direct<0x4b0900, carDataLineHasValues>(app, ts, line) == 0)
        {
            keyFound = fe::direct<0x4b0940, carDataFindKey>(app, ts, line, fieldAt) != 0;
            continue;
        }
        const u32 index = guest.dword(fieldAt);
        if (keyFound && guest.byte(kCarFieldRead + index) == 0 && index < kCarFieldCount)
        {
            if (index == kFieldReturned)
            {
                if (firstOut != 0)
                    guest.setDword(firstOut, fe::direct<0x4b0700, carDataReadInteger>(app, ts, line, 0));
            }
            else
            {
                readField(index);
            }
            guest.setByte(kCarFieldRead + guest.dword(fieldAt), 1);
        }
        keyFound = false;
    }

    u32 fieldsRead = 0;
    for (u32 i = 0; i < kCarFieldCount; ++i)
        fieldsRead += guest.byte(kCarFieldRead + i);
    if (fieldsRead != kCarFieldCount)
        fe::direct<0x4b0bb0, carDataFillDefaults>(app, ts, record, 0);
    return fieldsRead;
}

nfs::u32 frontend::carDataRead(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4b1380", 0, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 record = args.eax, text = args.edx, length = args.ebx, firstOut = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::carDataRead(app, ts, record, text, length, firstOut); });
}

/* ---------------------------------------------------------------------------
 * Windows' side of things: the exception's name, NPS's DLLs, the install file, the disc
 * ------------------------------------------------------------------------- */

namespace
{

constexpr u32 kStrcmp = 0x4ee310, kStrncmp = 0x4eb390, kStrncpy = 0x4e0e30, kStrncat = 0x4eedf0;
constexpr u32 kFopen = 0x4edfe8, kFgets = 0x4f0570, kFclose = 0x4ee100, kExit = 0x4dfd04;

constexpr u32 kNpsDllKey = 0x53da94, kDllNames = 0x55eca0, kDllPath = 0x7a2020;
constexpr u32 kDllLoaded = 0x55ec80, kDllHandles = 0x55ec90;

/* strcat, as the code has it inline. */
void appendString(Guest& guest, u32 to, u32 from)
{
    guest.copyString(to + guest.length(to), from);
}

/* A line of text cut at the first of this character. */
void cutAt(Guest& guest, u32 line, std::uint8_t cut)
{
    for (u32 at = line;; ++at)
    {
        const std::uint8_t c = guest.byte(at);
        if (c == cut)
        {
            guest.setByte(at, 0);
            return;
        }
        if (c == 0)
            return;
    }
}

}

/* sub_4a3070: an exception's name for the crash report. */
u32 frontend::wata::exceptionName(App* app, ThreadState& ts, u32 code)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    constexpr u32 kUnknownFormat = 0x53cd30, kUnknownName = 0x7a1a84;
    switch (code)
    {
    case 0xc000008c: return 0x53cb08;   // array bounds exceeded
    case 0x80000003: return 0x53cb28;   // breakpoint
    case 0x80000002: return 0x53cb40;   // datatype misalignment
    case 0xc000008d: return 0x53cb60;   // and the x87's and the rest, in the order of their strings
    case 0xc000008e: return 0x53cb80;
    case 0xc000008f: return 0x53cba0;
    case 0xc0000090: return 0x53cbc0;
    case 0xc0000091: return 0x53cbe0;
    case 0xc0000092: return 0x53cbf8;
    case 0xc0000093: return 0x53cc14;
    case 0xc000001d: return 0x53cc2c;
    case 0xc0000006: return 0x53cc4c;
    case 0xc0000094: return 0x53cc64;
    case 0xc0000095: return 0x53cc84;
    case 0xc0000026: return 0x53cc9c;
    case 0xc0000025: return 0x53ccbc;
    case 0xc0000096: return 0x53cce0;
    case 0x80000004: return 0x53ccfc;
    case 0xc00000fd: return 0x53cd14;
    default:
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), kUnknownName, kUnknownFormat, guest::Words{code});
        return kUnknownName;
    }
}

nfs::u32 frontend::exceptionName(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4a3070", 0, 0, nullptr, 0, 0 };
    const u32 code = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::exceptionName(app, ts, code); });
}

/* sub_4a54f0: an NPS DLL loaded, and with edx 0 every function of it looked up into its slot. */
u32 frontend::wata::npsLoadDll(App* app, ThreadState& ts, u32 path, u32 type)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 5 * 4);
    struct Procedure
    {
        u32 name, slot;
    };
    static constexpr Procedure kProcedures[] = {
        { 0x53d808, 0x7a21e4 }, { 0x53d81c, 0x7a21dc }, { 0x53d830, 0x7a21d0 }, { 0x53d844, 0x7a21e0 }, { 0x53d858, 0x7a21cc },
        { 0x53d868, 0x7a21d8 }, { 0x53d878, 0x7a21c8 }, { 0x53d888, 0x7a21c0 }, { 0x53d89c, 0x7a217c }, { 0x53d8b0, 0x7a2184 },
        { 0x53d8c0, 0x7a21b8 }, { 0x53d8d0, 0x7a2190 }, { 0x53d8e0, 0x7a2168 }, { 0x53d8f0, 0x7a21a4 }, { 0x53d908, 0x7a2198 },
        { 0x53d91c, 0x7a2164 }, { 0x53d938, 0x7a2174 }, { 0x53d94c, 0x7a21d4 }, { 0x53d960, 0x7a2188 }, { 0x53d974, 0x7a21b4 },
        { 0x53d984, 0x7a219c }, { 0x53d998, 0x7a2194 }, { 0x53d9a8, 0x7a21ac }, { 0x53d9c0, 0x7a21a8 }, { 0x53d9d4, 0x7a218c },
        { 0x53d9e8, 0x7a21c4 }, { 0x53d9fc, 0x7a21b0 }, { 0x53da14, 0x7a2170 }, { 0x53da2c, 0x7a21bc }, { 0x53da44, 0x7a21a0 },
        { 0x53da58, 0x7a216c }, { 0x53da70, 0x7a2178 }, { 0x53da80, 0x7a2180 },
    };
    const u32 module = os::modules::load(app, guest::text(app, path)).value_or(0);
    if (module == 0)
        return 0;
    if (type != 0)
        return 0xffffffffu;
    for (const Procedure& procedure : kProcedures)
        guest.setDword(procedure.slot, os::modules::symbol(app, module, guest::text(app, procedure.name)));
    return module;
}

nfs::u32 frontend::npsLoadDll(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4a54f0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 path = args.eax, type = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::npsLoadDll(app, ts, path, type); });
}

/* sub_4a5780: whether the settings' NPS key says this is a developer's install: 1 for any of the
 * five words at 0x53dab8. */
u32 frontend::wata::npsDevelopMode(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4 * 4 + 0x48);
    constexpr u32 kDevelopValue = 0x53daa8;
    static constexpr u32 kYes[] = { 0x53dab8, 0x53dabc, 0x53dac0, 0x53dac4, 0x53dac8 };
    const u32 value = guest.frame();
    const auto setting = os::settings::find(app, os::settings::Root::localMachine, guest::text(app, kNpsDllKey),
                                            guest::text(app, kDevelopValue));
    if (!setting)
        return 0;
    os::settings::copyTo(app, *setting, value, 0x40);
    for (const u32 yes : kYes)
        if (fe::direct<kStrcmp, watcom::clib::strcmp>(guest.app(), guest.ts(), value, yes) == 0)
            return 1;
    return 0;
}

nfs::u32 frontend::npsDevelopMode(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4a5780", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::npsDevelopMode(app, ts); });
}

/* sub_4a5880: the path of an NPS DLL from the settings, at 0x7a2020, with backslashes; on a
 * developer's install under the name's _DEVELOP value.  1 when it is there. */
u32 frontend::wata::npsDllPath(App* app, ThreadState& ts, u32 type)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 6 * 4 + 0x108);
    constexpr u32 kDevelopSuffix = 0x53dacc, kUsingFormat = 0x53dad8;
    const u32 value = guest.frame(), note = value + 0x80;
    using os::settings::Root;
    if (!os::settings::has(app, Root::localMachine, guest::text(app, kNpsDllKey)))
        return 0;
    guest.copyString(value, guest.dword(kDllNames + 4 * type));
    if (guest::direct<0x4a5780, npsDevelopMode>(app, ts) == 1)
    {
        appendString(guest, value, kDevelopSuffix);
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), note, kUsingFormat, guest::Words{value});
        os::debugText(guest::text(app, note));
    }
    const auto setting = os::settings::find(app, Root::localMachine, guest::text(app, kNpsDllKey), guest::text(app, value));
    if (!setting)
        return 0;
    os::settings::copyTo(app, *setting, kDllPath, 0x80);
    for (u32 at = kDllPath; guest.byte(at) != 0; ++at)
        if (guest.byte(at) == '/')
            guest.setByte(at, '\\');
    return 1;
}

nfs::u32 frontend::npsDllPath(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4a5880", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 type = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::npsDllPath(app, ts, type); });
}

/* sub_4a59c0: an NPS DLL of a type loaded once; a message box if it cannot be. */
u32 frontend::wata::npsEnsureDll(App* app, ThreadState& ts, u32 type)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 6 * 4 + 0x100);
    constexpr u32 kLoadFailedFormat = 0x53daf0, kLoadFailedTitle = 0x53db30, kNoPathFormat = 0x53db44, kNoPathTitle = 0x53db90;
    const u32 noPathText = guest.frame(), failedText = noPathText + 0x80;
    if (guest.dword(kDllLoaded + 4 * type) != 0)
        return 0;
    if (guest::direct<0x4a5880, npsDllPath>(app, ts, type) == 1)
    {
        const u32 handle = guest::direct<0x4a54f0, npsLoadDll>(app, ts, kDllPath, type);
        guest.setDword(kDllHandles + 4 * type, handle);
        if (handle != 0)
        {
            guest.setDword(kDllLoaded + 4 * type, guest.dword(kDllLoaded + 4 * type) + 1);
            return 1;
        }
        // The system's error, which the port does not give: 0.
        const u32 error = 0;
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), failedText, kLoadFailedFormat, guest::Words{type, guest.dword(kDllNames + 4 * type), kDllPath, error});
        os::messageBox(guest::text(app, failedText), guest::text(app, kLoadFailedTitle), os::Box::plain);
        os::debugText(guest::text(app, failedText));
        return 0;
    }
    fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), noPathText, kNoPathFormat, guest::Words{type, guest.dword(kDllNames + 4 * type)});
    os::messageBox(guest::text(app, noPathText), guest::text(app, kNoPathTitle), os::Box::plain);
    return 0;
}

nfs::u32 frontend::npsEnsureDll(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4a59c0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 type = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::npsEnsureDll(app, ts, type); });
}

/* sub_4a6e10: install.win read: its lines into the table at 0x7a2634, and the first of them taken
 * for one of the six languages. */
u32 frontend::wata::installFileRead(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 6 * 4 + 0x88);
    constexpr u32 kLines = 0x7a2634, kLinesBytes = 0x1400, kLineLength = 0x7f, kLineStride = 7;
    constexpr u32 kFileName = 0x53dc40, kReadMode = 0x53dc3c;
    constexpr u32 kLanguageNames = 0x55fd24, kLanguage = 0x7a3a50, kLanguageCount = 6;
    constexpr u32 kStreamFlags = 0xc;
    constexpr std::uint8_t kStreamAtEnd = 0x10;
    const u32 line = guest.frame();
    fe::direct<kMemset, watcom::clib::memset>(guest.app(), guest.ts(), kLines, 0, kLinesBytes);
    const u32 file = fe::direct<kFopen, watcom::clib::fopen>(guest.app(), guest.ts(), kFileName, kReadMode);
    if (file == 0)
        return 0;
    u32 count = 0;
    while ((guest.byte(file + kStreamFlags) & kStreamAtEnd) == 0)
    {
        fe::direct<kFgets, watcom::clib::fgets>(guest.app(), guest.ts(), line, kLineLength, file);
        cutAt(guest, line, '\n');
        cutAt(guest, line, '\r');
        if (guest.byte(line) == 0)
            continue;
        guest.copyString(kLines + (count << kLineStride), line);
        ++count;
    }
    fe::direct<kFclose, watcom::clib::fclose>(guest.app(), guest.ts(), file);
    u32 language = 0;
    for (; language < kLanguageCount; ++language)
    {
        const u32 name = guest.dword(kLanguageNames + 4 * language);
        if (fe::direct<kStrncmp, watcom::clib::strncmp>(guest.app(), guest.ts(), name, kLines, guest.length(name)) == 0)
            break;
    }
    if (language == kLanguageCount)
        fe::direct<0x4b5e50, wina::fatalMessage>(app, ts, 0);
    guest.setDword(kLanguage, language);
    return 1;
}

nfs::u32 frontend::installFileRead(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4a6e10", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::installFileRead(app, ts); });
}

/* sub_4f0330: findfirst: the first file, not a folder, a pattern finds, its path at 0x9f0ac9.  The
 * search is kept at 0x5651d4 (-1 for none); the name found goes into the frame, where strncat
 * takes it. */
u32 frontend::wata::findFirstFile(App* app, ThreadState& ts, u32 pattern)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 5 * 4 + 0x140);
    constexpr u32 kHandle = 0x5651d4, kPattern = 0x9f09c4, kPatternFolder = 0x9f09c0, kFound = 0x9f0ac9;
    constexpr u32 kPathMost = 0x104;
    const u32 name = guest.frame();
    if (guest.dword(kHandle) != 0xffffffffu)
        os::release(app, guest.dword(kHandle));
    fe::direct<kStrncpy, watcom::clib::strncpy>(guest.app(), guest.ts(), kPattern, pattern, kPathMost);
    s32 folder = s32(guest.length(kPattern));
    while (folder > 0 && guest.byte(kPattern + folder) != '\\' && guest.byte(kPattern + folder) != ':')
        --folder;
    if (folder != 0)
        ++folder;
    guest.setDword(kPatternFolder, u32(folder));
    os::files::Found found;
    const auto search = os::files::findFirst(app, guest::text(app, kPattern), found);
    guest.setDword(kHandle, search.value_or(0xffffffffu));
    if (!search)
        return 0;
    guest::putText(app, name, found.name.c_str());
    const u32 kept = guest.dword(kPatternFolder);
    guest.setByte(kFound + kept, 0);
    fe::direct<kStrncpy, watcom::clib::strncpy>(guest.app(), guest.ts(), kFound, kPattern, kept);
    fe::direct<kStrncat, watcom::clib::strncat>(guest.app(), guest.ts(), kFound, name, kPathMost);
    if (os::files::isFile(guest::text(app, kFound)))
        return kFound;
    return guest::direct<0x4f0280, findNextFile>(app, ts);
}

nfs::u32 frontend::findFirstFile(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f0330", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 pattern = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::findFirstFile(app, ts, pattern); });
}

/* sub_4f0280: findnext: the next file, not a folder, or 0 and the search closed. */
u32 frontend::wata::findNextFile(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 5 * 4 + 0x140);
    constexpr u32 kHandle = 0x5651d4, kPattern = 0x9f09c4, kPatternFolder = 0x9f09c0, kFound = 0x9f0ac9;
    constexpr u32 kPathMost = 0x104;
    const u32 name = guest.frame();
    if (guest.dword(kHandle) == 0xffffffffu)
        return 0;
    os::files::Found found;
    for (;;)
    {
        if (!os::files::findNext(app, guest.dword(kHandle), found))
        {
            os::release(app, guest.dword(kHandle));
            guest.setDword(kHandle, 0xffffffffu);
            return 0;
        }
        guest::putText(app, name, found.name.c_str());
        const u32 kept = guest.dword(kPatternFolder);
        guest.setByte(kFound + kept, 0);
        fe::direct<kStrncpy, watcom::clib::strncpy>(guest.app(), guest.ts(), kFound, kPattern, kept);
        fe::direct<kStrncat, watcom::clib::strncat>(guest.app(), guest.ts(), kFound, name, kPathMost);
        if (os::files::isFile(guest::text(app, kFound)))
            return kFound;
    }
}

nfs::u32 frontend::findNextFile(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f0280", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::findNextFile(app, ts); });
}

/* sub_4a46f0: the colour of a car's badge in the showcase, by the words in its name. */
u32 frontend::wata::carBadgeColour(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4 * 4);
    constexpr u32 kCarNameTexts = 0x554fa8;
    constexpr u32 kOtherColour = 0xff4060a0;
    struct Badge
    {
        u32 word, colour;
    };
    static constexpr Badge kBadges[] = {
        { 0x53cf40, 0xfffbb829 }, { 0x53cf48, 0xff64ffff }, { 0x53cf50, 0xff0000ff },
        { 0x53cf5c, 0xff0000ff }, { 0x53cf68, 0xffc02028 }, { 0x53cf70, 0xffbbeeff },
    };
    const std::int16_t id = std::int16_t(guest.word(kCarNameTexts + 2 * gameSettings(guest).players[0].car));
    // The game's code hands it edx as it has it, which only the controllers' names would hand on (and
    // set their own): none here.
    const u32 name = u32(*fe::direct<0x4d1850, port::text::lineText>(app, ts, ts.sp - 4, std::int32_t(id), u32(0)));
    for (const Badge& badge : kBadges)
        if (fe::direct<kStrstr, watcom::clib::strstr>(guest.app(), guest.ts(), name, badge.word) != 0)
            return badge.colour;
    return kOtherColour;
}

nfs::u32 frontend::carBadgeColour(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4a46f0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::carBadgeColour(app, ts); });
}

/* ---------------------------------------------------------------------------
 * The window's description checked (win\validwin.c)
 * ------------------------------------------------------------------------- */

namespace
{
constexpr u32 kErrorFile = 0x552190, kErrorFunction = 0x552194, kErrorLine = 0x552198;
constexpr u32 kError = 0x401010;
}

/* sub_4f4920: a field of a window's description within its limits; an error if not and the
 * check is on.  Stack: the most, and whether the field is one the game must have. */
u32 frontend::wata::windowCheckField(App* app, ThreadState& ts, u32 window, u32 value, u32 name, u32 least, u32 most,
                                       u32 wanted)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 3 * 4);
    constexpr u32 kChecking = 0x56440c, kFile = 0x54cf04, kFunction = 0x54cf14;
    if (s32(value) >= s32(least) && s32(value) <= s32(most))
        return 1;
    if (guest.dword(kChecking) == 0)
        return 0;
    guest.setDword(kErrorFile, kFile);
    guest.setDword(kErrorFunction, kFunction);
    const u32 at4 = guest.dword(window + 4), at8 = guest.dword(window + 8);
    const u32 kindByte = guest.byte(window + 0x1c);
    if (least == most)
    {
        guest.setDword(kErrorLine, wanted != 0 ? 0x14 : 0x16);
        fe::direct<kError, ea::error>(guest.app(), guest.ts(), wanted != 0 ? 0x54cf28u : 0x54cf7cu, guest::Words{name, value, least, at4, at8, kindByte, window});
    }
    else
    {
        guest.setDword(kErrorLine, wanted != 0 ? 0x1b : 0x1d);
        fe::direct<kError, ea::error>(guest.app(), guest.ts(), wanted != 0 ? 0x54cfd0u : 0x54d028u, guest::Words{name, value, least, most, at4, at8, kindByte, window});
    }
    return 0;
}

nfs::u32 frontend::windowCheckField(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f4920", 8, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 window = args.eax, value = args.edx, name = args.ebx, least = args.ecx;
    const u32 most = app->getMemory<u32>(ts.sp + 4);
    const u32 wanted = app->getMemory<u32>(ts.sp + 8);
    return fe::runThunk(app, ts, args, site,
                 [&](Guest&) { return wata::windowCheckField(app, ts, window, value, name, least, most, wanted); });
    return args.eax;
}

/* sub_4f4a80: a window's description checked, field by field: 1 when all are in their limits. */
u32 frontend::wata::windowCheck(App* app, ThreadState& ts, u32 window)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 5 * 4);
    constexpr u32 kMagic = 0x57494e44;    // 'WIND'
    constexpr u32 kLargest = 0x4000;
    const auto check = [&](u32 value, u32 name, u32 least, u32 most, u32 wanted) {
        return guest::direct<0x4f4920, windowCheckField>(app, ts, window, value, name, least, most, wanted);
    };
    u32 all = check(guest.dword(window), 0x54d080, kMagic, kMagic, 1);
    all &= check(guest.dword(window + 4), 0x54d084, 1, kLargest, 0);
    all &= check(guest.dword(window + 8), 0x54d08c, 1, kLargest, 0);
    all &= check(guest.byte(window + 0x1c), 0x54d094, 1, 0x20, 0);
    all &= check(guest.byte(window + 0x1d), 0x54d098, 0, 0x7f, 1);
    all &= check(guest.dword(window + 0xc), 0x54d0a0, 0, kLargest, 0);
    all &= check(guest.dword(window + 0x14), 0x54d0a8, 0, kLargest, 0);
    all &= check(guest.dword(window + 0x10), 0x54d0b0, 0, kLargest, 0);
    all &= check(guest.dword(window + 0x18), 0x54d0b8, 0, kLargest, 0);
    return all;
}

nfs::u32 frontend::windowCheck(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f4a80", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 window = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::windowCheck(app, ts, window); });
}

/* sub_4f9390: a directory joined to a file's name in a buffer of the size given: 0 if it will
 * not fit. */
u32 frontend::wata::pathInDirectory(App* app, ThreadState& ts, u32 buffer, u32 fileName, u32 size)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 3 * 4);
    fe::direct<0x4e33a0, ea::currentDirectory>(app, ts, buffer);
    const u32 room = size - guest.length(fileName) - 1;
    if (guest.length(buffer) > room)
        return 0;
    appendString(guest, buffer, fileName);
    return 1;
}

nfs::u32 frontend::pathInDirectory(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f9390", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 buffer = args.eax, fileName = args.edx, size = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::pathInDirectory(app, ts, buffer, fileName, size); });
}

/* sub_4f0b60: a string run through the text's own measure (sub_4efb50) with its output turned
 * aside: the box it covers, through four pointers. */
u32 frontend::wata::textMeasureBox(App* app, ThreadState& ts, u32 text, s32 given, u32 leftOut, u32 topOut,
                                     u32 widthOut, u32 heightOut)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 3 * 4 + 0x110);
    constexpr u32 kPenX = 0x5671f8, kPenY = 0x5671fc;
    constexpr u32 kBoxLeft = 0x9f0bdc, kBoxTop = 0x9f0bd8, kBoxRight = 0x9f0bd0, kBoxBottom = 0x9f0bd4;
    constexpr u32 kPutGlyph = 0x567288, kPutGlyph2 = 0x56728c, kPutGlyph3 = 0x567290;
    constexpr u32 kMeasureGlyph = 0x4f0af0;
    constexpr u32 kLongest = 0x100;
    const u32 copy = guest.frame();
    const u32 penX = guest.dword(kPenX), penY = guest.dword(kPenY);
    guest::direct<0x4f0850, textSetPen>(app, ts, 0, 0);
    guest.setDword(kBoxLeft, 0);
    guest.setDword(kBoxTop, 0);
    guest.setDword(kBoxRight, 0);
    guest.setDword(kBoxBottom, 0);
    const u32 length = given > s32(kLongest) ? kLongest : u32(given);
    guest.setByte(copy + length, 0);
    fe::direct<kStrncpy, watcom::clib::strncpy>(guest.app(), guest.ts(), copy, text, length);
    const u32 put1 = guest.dword(kPutGlyph), put2 = guest.dword(kPutGlyph2), put3 = guest.dword(kPutGlyph3);
    guest.setDword(kPutGlyph, kMeasureGlyph);
    guest.setDword(kPutGlyph2, 0);
    guest.setDword(kPutGlyph3, 0);
    if (guest.byte(copy) != 0)
        fe::direct<0x4efb50, eab::textDrawAtPen>(app, ts, copy);
    guest.setDword(leftOut, guest.dword(kBoxLeft));
    guest.setDword(topOut, guest.dword(kBoxTop));
    const s32 width = s32(guest.dword(kBoxRight) - guest.dword(kBoxLeft));
    const s32 least = s32(guest.dword(kPenX));
    guest.setDword(widthOut, u32(width < least ? least : width));
    guest.setDword(heightOut, guest.dword(kBoxBottom) - guest.dword(kBoxTop));
    guest::direct<0x4f0850, textSetPen>(app, ts, penX, penY);
    guest.setDword(kPutGlyph2, put2);
    guest.setDword(kPutGlyph3, put3);
    guest.setDword(kPutGlyph, put1);
    return put1;
}

nfs::u32 frontend::textMeasureBox(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f0b60", 8, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 text = args.eax, leftOut = args.ebx, topOut = args.ecx;
    const s32 given = s32(args.edx);
    const u32 widthOut = app->getMemory<u32>(ts.sp + 4);
    const u32 heightOut = app->getMemory<u32>(ts.sp + 8);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return wata::textMeasureBox(app, ts, text, given, leftOut, topOut, widthOut, heightOut);
    });
}

/* sub_4fb710: a shift key's lock light set to what is asked, by pressing it: 0 caps, 1 num, 2 scroll.
 * The port's keyboard has no lock lights: each is out (0), and the key pressed to light one is
 * nothing the port does -- 0 either way.  Any other lock is handed back. */
u32 frontend::wata::lockKeySet(App* app, ThreadState& ts, u32 lock, u32 wanted)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 5 * 4 + 4);
    NFS2_USE(wanted);
    if (lock > 2)
        return lock;
    return 0;
}

nfs::u32 frontend::lockKeySet(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4fb710", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 lock = args.eax, wanted = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::lockKeySet(app, ts, lock, wanted); });
}

/* ---------------------------------------------------------------------------
 * The command line
 * ------------------------------------------------------------------------- */

/* sub_4a5080: the game's command line, last word first: switches into the flags at 0x55eb2c and
 * the settings from 0x7a1fa0 on; a help or a word it does not know, and a box lists them.  0 then,
 * and when a server was asked for without what it takes; 1 otherwise. */
u32 frontend::wata::commandLineRead(App* app, ThreadState& ts, u32 count, u32 words)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 5 * 4 + 0x168);
    constexpr u32 kFillZero = 0x4e070c, kStricmp = 0x4e9ec0, kStrupr = 0x4f8020, kSscanf = 0x4f0548;
    constexpr u32 kSettings = 0x7a1fa0, kSettingsBytes = 0x7c;
    constexpr u32 kHelpWords = 0x55ec3c, kHelpWordCount = 5;
    constexpr u32 kFlags = 0x55eb2c, kFlagNames = 0x55eb34, kFlagCount = 0x21, kFlagHelps = 0x55ebb8;
    constexpr u32 kNumberedPrefix = 0x53d684, kNumberFormat = 0x53d68c, kNumbered = 0x55fdbc;
    constexpr std::uint8_t kNumberedFlag = 0x20;
    constexpr u32 kNameWords = 0x55ec50, kNameWordCount = 0xc;
    constexpr u32 kServerName = 0x7a1fa0, kServerNameMost = 0x3c, kPort = 0x7a1fdc, kCode = 0x7a1fe0, kCodeMost = 0xb;
    constexpr u32 kOtherNumbers = 0x7a1fec, kNeeded = 0x7a1ff8;
    constexpr u32 kUsage = 0x7a1ba0;
    const u32 spaces = guest.frame(), word = spaces + 0x100, number = spaces + 0x150;
    bool help = false, understood = false;
    fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.ts(), kSettings, kSettingsBytes);
    u32 at = count;
    for (;;)
    {
        --at;
        if (at == 0)
            break;
        const u32 argument = guest.dword(words + 4 * at);
        understood = false;
        for (u32 i = 0; i < kHelpWordCount; ++i)
            if (fe::direct<kStricmp, watcom::clib::stricmp>(guest.app(), guest.ts(), argument, guest.dword(kHelpWords + 4 * i)) == 0)
                help = true;
        if (!help)
            for (u32 i = 0; i < kFlagCount; ++i)
            {
                if (fe::direct<kStricmp, watcom::clib::stricmp>(guest.app(), guest.ts(), argument, guest.dword(kFlagNames + 4 * i)) != 0)
                    continue;
                const u32 flags = kFlags + 4 * (i / 32);
                guest.setDword(flags, guest.dword(flags) | (u32(1) << ((s32(i + 1) % 32 - 1) & 31)));
                understood = true;
                break;
            }
        guest.copyString(word, argument);
        fe::direct<kStrupr, watcom::clib::strupr>(guest.app(), guest.ts(), word);
        if (!understood && fe::direct<kStrncmp, watcom::clib::strncmp>(guest.app(), guest.ts(), word, kNumberedPrefix, 4) == 0
            && fe::direct<kSscanf, watcom::clib::sscanf>(guest.app(), guest.ts(), word + 4, kNumberFormat, guest::Words{number}) != 0)
        {
            const u32 value = guest.dword(number);
            const u32 flags = kFlags + 4 * u32(s32(value) / 32);
            guest.setByte(flags, guest.byte(flags) | kNumberedFlag);
            understood = true;
            guest.setDword(kNumbered, value);
        }
        if (!understood)
            for (u32 i = 0; i < kNameWordCount; ++i)
            {
                const u32 name = guest.dword(kNameWords + 4 * i);
                const u32 length = guest.length(name);
                if (fe::direct<kStrncmp, watcom::clib::strncmp>(guest.app(), guest.ts(), word, name, length) != 0)
                    continue;
                const u32 rest = word + length;
                if (i == 0)
                {
                    const u32 restLength = guest.length(rest);
                    if (restLength != 0 && restLength < kServerNameMost)
                    {
                        guest.copyString(kServerName, rest);
                        understood = true;
                    }
                }
                else if (i == 1)
                {
                    if (fe::direct<kSscanf, watcom::clib::sscanf>(guest.app(), guest.ts(), rest, kNumberFormat, guest::Words{kPort}) != 0)
                        understood = true;
                }
                else if (i == 2)
                {
                    understood = true;
                    fe::direct<kStrncpy, watcom::clib::strncpy>(guest.app(), guest.ts(), kCode, argument + length, kCodeMost);
                }
                else if (fe::direct<kSscanf, watcom::clib::sscanf>(guest.app(), guest.ts(), rest, kNumberFormat, guest::Words{kOtherNumbers + 4 * i}) != 0)
                {
                    understood = true;
                }
            }
        if (understood && !help)
            continue;

        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), kUsage, 0x53d6ac, guest::Words{0x53d6a8, 0x53d69c, 0x53d690});
        if (!help)
            fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), kUsage + guest.length(kUsage), 0x53d70c, guest::Words{argument});
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), kUsage + guest.length(kUsage), 0x53d724, guest::Words({}));
        for (u32 i = 0; i < kFlagCount; ++i)
        {
            const u32 name = guest.dword(kFlagNames + 4 * i);
            if (guest.byte(name) == 0)
                continue;
            const u32 pad = 0xe - guest.length(name);
            for (u32 c = 0; c < pad; ++c)
                guest.setByte(spaces + c, ' ');
            guest.setByte(spaces + pad, 0);
            fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), kUsage + guest.length(kUsage), 0x53d728, guest::Words{name, spaces, guest.dword(kFlagHelps + 4 * i)});
        }
        os::messageBox(guest::text(app, kUsage), guest::text(app, understood ? 0x53d748u : 0x53d734u), os::Box::plain);
        return 0;
    }
    if (guest.dword(kPort) != 0
        && (guest.length(kServerName) == 0 || guest.length(kCode) == 0 || guest.dword(kNeeded) == 0))
    {
        os::messageBox(guest::text(app, 0x53d764), guest::text(app, 0x53d750), os::Box::plain);
        return 0;
    }
    return 1;
}

nfs::u32 frontend::commandLineRead(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4a5080", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 count = args.eax, words = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::commandLineRead(app, ts, count, words); });
}

/* sub_4b6320: the game's disc looked for, a box and the end if it is run from one or there is none. */
u32 frontend::wata::discCheck(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 6 * 4 + 0x13c);
    constexpr u32 kInstallName = 0x53fe30, kTitle = 0x53fe3c, kLanguage = 0x7a3a50;
    constexpr u32 kPathPattern = 0x4b5688, kRunFromDiscTexts = 0x4b5694, kNoDiscTexts = 0x4b56ac;
    constexpr u32 kDrives = 0x1a;
    const u32 ownPath = guest.frame(), noDisc = ownPath + 0x100, runFromDisc = ownPath + 0x118, pattern = ownPath + 0x130;
    // A box and the end: what exit hands back, should it come back.
    const auto refuse = [&](u32 texts, u32 local) {
        copyDwords(guest, local, texts, 6);
        const u32 text = guest.dword(local + 4 * guest.dword(kLanguage));
        os::messageBox(guest::text(app, text), guest::text(app, kTitle), os::Box::plain);
        return fe::direct<kExit, watcom::clib::exit>(guest.app(), guest.ts(), 0);
    };
    copyDwords(guest, pattern, kPathPattern, 2);
    guest.setWord(pattern + 8, guest.word(kPathPattern + 8));
    guest.setByte(pattern + 10, guest.byte(kPathPattern + 10));
    guest::direct<0x4f9390, pathInDirectory>(app, ts, ownPath, kInstallName, 0x100);
    const u32 ownDrive = guest::direct<0x4f93f0, driveNumberOfPath>(app, ts, ownPath);
    const u32 ownIsCd = guest::direct<0x4f9410, driveIsCd>(app, ts, ownDrive);
    if (ownIsCd != 0)
        refuse(kRunFromDiscTexts, runFromDisc);
    // The game's code hands it what came back before in eax, which it does not read.
    if (const u32 remote = fe::direct<0x4b62a0, loopa::installIsRemote>(app, ts); remote != 0)
        return remote;
    for (u32 drive = 0; drive < kDrives; ++drive)
    {
        if (guest::direct<0x4f9410, driveIsCd>(app, ts, drive) == 0)
            continue;
        guest.setByte(pattern, std::uint8_t('A' + drive));
        if (const u32 found = guest::direct<0x4f0330, findFirstFile>(app, ts, pattern); found != 0)
            return found;
    }
    return refuse(kNoDiscTexts, noDisc);
}

nfs::u32 frontend::discCheck(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4b6320", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::discCheck(app, ts); });
}

/* sub_4f9410: whether a drive, 0 for A:, is a CD. */
u32 frontend::wata::driveIsCd(App* app, ThreadState& ts, u32 drive)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4 * 4 + 4);
    constexpr u32 kRootA = 0x54d930;  // "A:\\", its letter made the drive's
    const char letter = char(std::uint8_t(guest.byte(kRootA) + drive));
    return os::files::drive(letter) == os::files::Drive::disc ? 1 : 0;
}

nfs::u32 frontend::driveIsCd(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f9410", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 drive = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::driveIsCd(app, ts, drive); });
}

/* sub_4f93f0: a path's drive, 0 for A:, or -1. */
u32 frontend::wata::driveNumberOfPath(App* app, ThreadState& ts, u32 path)
{
    Guest guest(app, ts);
    constexpr u32 kToupper = 0x4eede0;
    if (guest.byte(path + 1) != ':')
        return 0xffffffffu;
    return fe::direct<kToupper, watcom::clib::toupper>(guest.app(), guest.ts(), guest.byte(path)) - 'A';
}

nfs::u32 frontend::driveNumberOfPath(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f93f0", 0, 0, nullptr, 0, 0 };
    const u32 path = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::driveNumberOfPath(app, ts, path); });
}

/* ---------------------------------------------------------------------------
 * EA's asynchronous file reader (win\async.c): a thread, its record at 0x9f0be0, queues of
 * requests in the record -- free, done, done with a callback, waiting
 * ------------------------------------------------------------------------- */

namespace
{

constexpr u32 kAsync = 0x9f0be0, kAsyncWake = 0x9f0be4;
constexpr u32 kAsyncEvent = 0, kAsyncClosing = 4, kAsyncFile = 8, kAsyncFileStart = 0xc, kAsyncFileSize = 0x10;
constexpr u32 kAsyncSharedBuffer = 0x14, kAsyncCurrent = 0x18;
constexpr u32 kAsyncFree = 0x1c, kAsyncDone = 0x38, kAsyncDoneCallback = 0x54, kAsyncWaiting = 0x70, kAsyncRequests = 0x8c;
constexpr u32 kRequestBytes = 0x28;
constexpr u32 kRequestId = 4, kRequestBuffer = 8, kRequestOffset = 0xc, kRequestSize = 0x10, kRequestPool = 0x14;
constexpr u32 kRequestOwned = 0x18, kRequestName = 0x1c, kRequestKind = 0x20, kRequestCallback = 0x24;
constexpr u32 kNextRequestId = 0x565284, kCurrentPool = 0x5643f4;
constexpr u32 kAsyncSource = 0x54c680;

constexpr u32 kQueueTake = 0x50df60, kQueueLength = 0x50e1d0, kQueueFree = 0x50dde0;
constexpr u32 kFillZeroes = 0x4e070c, kCloseEvent = 0x4dfc00, kCreateThread = 0x4df7a0;
constexpr u32 kLockDrive = 0x4e40a0, kUnlockDrive = 0x4e40f0;
constexpr u32 kAsyncTask = 0x4f0d20, kAsyncThread = 0x4f1820;
constexpr u32 kRequestHasId = 0x4f0cb0;

/* What every assert of the library's leaves for the error's text. */
void reportError(Guest& guest, u32 file, u32 function, u32 line, u32 message)
{
    guest.setDword(kErrorFile, file);
    guest.setDword(kErrorFunction, function);
    guest.setDword(kErrorLine, line);
    fe::direct<kError, ea::error>(guest.app(), guest.ts(), message, guest::Words({}));
}

u32 asyncRecord(const Guest& guest)
{
    return guest.dword(kAsync);
}

/* The requests of a queue thrown away: their name and buffer freed, back on the free queue. */
void asyncDrain(Guest& guest, nfs::ThreadState& ts, u32 queue)
{
    nfs::App* const app = guest.app();
    for (u32 request = fe::direct<0x50df60, frontend::netb::qpop>(app, ts, asyncRecord(guest) + queue); request != 0;
         request = fe::direct<0x50df60, frontend::netb::qpop>(app, ts, asyncRecord(guest) + queue))
    {
        const u32 name = guest.dword(request + kRequestName);
        if (name != 0)
            fe::direct<0x4e1890, ea::purgeBlock>(app, ts, name);
        const u32 buffer = guest.dword(request + kRequestOwned);
        if (buffer != 0)
            fe::direct<0x4e1890, ea::purgeBlock>(app, ts, buffer);
        fe::direct<0x50de50, frontend::netb::qpush>(app, ts, asyncRecord(guest) + kAsyncFree, request);
    }
}

/* The reader's thread woken: its event set (the game's code hands it the event's slot in eax too,
 * which it does not read). */
void asyncWake(Guest& guest, nfs::ThreadState& ts)
{
    const u32 event = guest.dword(asyncRecord(guest) + kAsyncEvent);
    fe::direct<0x4dfb20, ea::eventPosted>(guest.app(), ts, event);
}

}

/* sub_4f0cb0: a queue's search: whether a request has this id. */
u32 frontend::wata::asyncRequestHasId(App* app, ThreadState& ts, u32 request, u32 id)
{
    Guest guest(app, ts);
    return id == guest.dword(request + kRequestId) ? 1 : 0;
}

nfs::u32 frontend::asyncRequestHasId(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f0cb0", 0, 0, nullptr, 0, 0 };
    const u32 request = args.eax, id = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::asyncRequestHasId(app, ts, request, id); });
}

/* sub_4f0cc0: nothing found. */
u32 frontend::wata::asyncNoRequest(App* app, ThreadState& ts)
{
    return 0;
}

nfs::u32 frontend::asyncNoRequest(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f0cc0", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::asyncNoRequest(app, ts); });
}

/* sub_4f0cd0: what the thread is at told to stop, and waited for.  With no reader, no result: eax
 * as it came, which its thunk does. */
u32 frontend::wata::asyncCancelCurrent(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 2 * 4);
    if (asyncRecord(guest) == 0)
        return 0;
    const u32 request = guest.dword(asyncRecord(guest) + kAsyncCurrent);
    if (request != 0)
        guest.setDword(request + kRequestKind, 0);
    asyncWake(guest, ts);
    for (;;)
    {
        const u32 record = asyncRecord(guest);
        if (guest.dword(record + kAsyncEvent) == 0 || request == 0 || request != guest.dword(record + kAsyncCurrent))
            return record;
        fe::direct<0x4df8e0, ea::sleepFor>(app, ts, 0);
    }
}

nfs::u32 frontend::asyncCancelCurrent(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f0cd0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    const u32 previous = args.eax;
    const bool none = app->getMemory<u32>(kAsync) == 0;
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        const u32 result = wata::asyncCancelCurrent(app, ts);
        return none ? previous : result;
    });
}

/* sub_4f0d20: the system task: finished requests' callbacks, in the game's own thread. */
u32 frontend::wata::asyncRunCallbacks(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    if (fe::direct<0x50e1d0, ea::queueLengthOf>(app, ts, asyncRecord(guest) + kAsyncDoneCallback) == 0)
        return 0;
    for (u32 request = fe::direct<0x50dff0, netb::qpoplast>(app, ts, asyncRecord(guest) + kAsyncDoneCallback); request != 0;
         request = fe::direct<0x50dff0, netb::qpoplast>(app, ts, asyncRecord(guest) + kAsyncDoneCallback))
    {
        const u32 owned = guest.dword(request + kRequestOwned);
        const bool kept = owned != 0 && owned != guest.dword(asyncRecord(guest) + kAsyncSharedBuffer);
        fe::direct<0x50de50, netb::qpush>(app, ts, asyncRecord(guest) + (kept ? kAsyncDone : kAsyncFree), request);
        fe::pointerCall(guest, guest.dword(request + kRequestCallback), args({ guest.dword(request + kRequestId) }));
    }
    return 0;
}

nfs::u32 frontend::asyncRunCallbacks(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f0d20", 0, 0, nullptr, 0, fe::kRestoreEcx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::asyncRunCallbacks(app, ts); });
}

/* sub_4f0da0: the reader started, with room for twice this many requests: its record, its queues,
 * its task and its thread, waited for.  `memoryClass` the class its record is reserved in: what
 * the caller's code has in ebx, which the game's code hands on to reservememadr as it is. */
u32 frontend::wata::asyncInit(App* app, ThreadState& ts, u32 requests, u32 memoryClass)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4 * 4);
    constexpr u32 kFunction = 0x54c68c, kTag = 0x54c6c8;
    if (fe::direct<0x4f9070, ea::surfaceIsLocked>(app, ts))
        reportError(guest, kAsyncSource, kFunction, 0xc4, 0x54c698);
    if (asyncRecord(guest) != 0)
        guest::direct<0x4f0f40, asyncShutdown>(app, ts);
    guest.setDword(kErrorFile, kAsyncSource);
    guest.setDword(kErrorLine, 0xc8);
    guest.setDword(kErrorFunction, kFunction);
    const u32 bytes = requests * kRequestBytes + kAsyncRequests;
    const u32 record = *fe::direct<0x4e1620, ea::reserved>(app, ts, kTag, bytes, memoryClass);
    guest.setDword(kAsync, record);
    fe::direct<kFillZeroes, ea::clearBytes>(guest.app(), guest.ts(), record, bytes);
    guest.setDword(asyncRecord(guest) + kAsyncSharedBuffer, asyncRecord(guest));
    for (const u32 queue : { kAsyncFree, kAsyncDone, kAsyncDoneCallback, kAsyncWaiting })
        fe::direct<0x50dcd0, netb::qinit>(app, ts, asyncRecord(guest) + queue, 0, 0);
    u32 request = asyncRecord(guest) + kAsyncRequests;
    for (u32 i = requests; i != 0; --i, request += kRequestBytes)
        fe::direct<0x50de50, netb::qpush>(app, ts, asyncRecord(guest) + kAsyncFree, request);
    // The game's code hands it 0xffffffff in ecx too, which it does not read.
    fe::direct<0x4e7520, port::stream::scheduleTask>(app, ts, ts.sp - 4, kAsyncTask, 4, 3);
    if (*fe::direct<kCreateThread, ea::threadStarted>(app, ts, kAsyncThread, 0u, 1u, kAsyncWake) == 0)
    {
        reportError(guest, kAsyncSource, kFunction, 0xde, 0x54c6d4);
    }
    for (;;)
    {
        const u32 current = asyncRecord(guest);
        const u32 event = guest.dword(current + kAsyncEvent);
        if (event != 0)
            return current;
        fe::direct<0x4df8e0, ea::sleepFor>(app, ts, event);
    }
}

nfs::u32 frontend::asyncInit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f0da0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 requests = args.eax * 2, memoryClass = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::asyncInit(app, ts, requests, memoryClass); });
}

/* sub_4f0f40: the reader stopped and everything it holds freed. */
u32 frontend::wata::asyncShutdown(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 5 * 4);
    constexpr u32 kFunction = 0x54c704;
    // No result with no reader: what the call left in eax, whether a surface is locked -- 0, as the
    // game ends in the error where one is.
    if (fe::direct<0x4f9070, ea::surfaceIsLocked>(app, ts))
        reportError(guest, kAsyncSource, kFunction, 0x108, 0x54c710);
    if (asyncRecord(guest) == 0)
        return 0;
    guest.setDword(asyncRecord(guest) + kAsyncClosing, 1);
    guest::direct<0x4f0cd0, asyncCancelCurrent>(app, ts);
    while (guest.dword(asyncRecord(guest) + kAsyncEvent) != 0)
        fe::direct<0x4df8e0, ea::sleepFor>(app, ts, 0);
    asyncDrain(guest, ts, kAsyncWaiting);
    asyncDrain(guest, ts, kAsyncDone);
    asyncDrain(guest, ts, kAsyncDoneCallback);
    fe::direct<0x4e75e0, port::stream::unscheduleTask>(app, ts, kAsyncTask);
    fe::direct<0x50dde0, ea::queueLockFreed>(app, ts, asyncRecord(guest) + kAsyncFree);
    fe::direct<0x50dde0, ea::queueLockFreed>(app, ts, asyncRecord(guest) + kAsyncDone);
    fe::direct<0x50dde0, ea::queueLockFreed>(app, ts, asyncRecord(guest) + kAsyncWaiting);
    fe::direct<0x50dde0, ea::queueLockFreed>(app, ts, asyncRecord(guest) + kAsyncDoneCallback);
    const u32 file = guest.dword(asyncRecord(guest) + kAsyncFile);
    if (file != 0)
        fe::direct<0x4e2da0, ea::closeChecked>(app, ts, file);
    const u32 result = *fe::direct<0x4e1890, ea::purgeBlock>(app, ts, asyncRecord(guest));
    guest.setDword(kAsync, 0);
    return result;
}

nfs::u32 frontend::asyncShutdown(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f0f40", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::asyncShutdown(app, ts); });
}

/* sub_4f1370: the file the reader reads from next, by name; 0 for none.  A request of kind 2. */
u32 frontend::wata::asyncSetFile(App* app, ThreadState& ts, u32 name)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4 * 4 + 4);
    constexpr u32 kFunction = 0x54c7a4, kTag = 0x54c780;
    constexpr u32 kKindSetFile = 2;
    if (asyncRecord(guest) == 0)
        reportError(guest, kAsyncSource, kFunction, 0x203, 0x54c7b4);
    const u32 request = fe::direct<kQueueTake, netb::qpop>(app, ts, asyncRecord(guest) + kAsyncFree);
    if (request == 0)
        return 0;
    guest.setDword(request + kRequestBuffer, 0);
    guest.setDword(request + kRequestOffset, 0);
    guest.setDword(request + kRequestSize, 0);
    guest.setDword(request + kRequestKind, kKindSetFile);
    guest.setDword(request + kRequestCallback, 0);
    const u32 id = guest.dword(kNextRequestId);
    guest.setDword(request + kRequestOwned, 0);
    guest.setDword(request + kRequestId, id);
    guest.setDword(request + kRequestPool, guest.dword(kCurrentPool));
    guest.setDword(kNextRequestId, id + 1);
    if (name != 0)
    {
        guest.setDword(kErrorFile, kAsyncSource);
        guest.setDword(kErrorFunction, kFunction);
        guest.setDword(kErrorLine, 0x214);
        const u32 copy = *fe::direct<0x4e1620, ea::reserved>(app, ts, kTag, guest.length(name) + 1, guest.dword(kCurrentPool));
        guest.setDword(request + kRequestName, copy);
        guest.copyString(copy, name);
    }
    else
    {
        guest.setDword(request + kRequestName, 0);
    }
    fe::direct<0x50de50, netb::qpush>(app, ts, asyncRecord(guest) + kAsyncWaiting, request);
    // The game's code hands it the event's slot in eax too, which it does not read.
    const u32 posted = *fe::direct<0x4dfb20, ea::eventPosted>(app, ts, guest.dword(asyncRecord(guest) + kAsyncEvent));
    return posted;
}

nfs::u32 frontend::asyncSetFile(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f1370", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi };
    const u32 name = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::asyncSetFile(app, ts, name); });
}

/* sub_4f1500: a read of the open file asked for, with a callback or none: its id, 0 with no free
 * request.  A request of kind 3. */
u32 frontend::wata::asyncReadAt(App* app, ThreadState& ts, u32 offset, u32 buffer, u32 size, u32 callback)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 3 * 4 + 4);
    constexpr u32 kFunction = 0x54c820, kKindRead = 3;
    if (asyncRecord(guest) == 0)
        reportError(guest, kAsyncSource, kFunction, 0x27d, 0x54c83c);
    const u32 request = fe::direct<0x50df60, netb::qpop>(app, ts, asyncRecord(guest) + kAsyncFree);
    if (request == 0)
        return 0;
    const u32 id = guest.dword(kNextRequestId);
    guest.setDword(request + kRequestId, id);
    const u32 record = asyncRecord(guest);
    guest.setDword(kNextRequestId, id + 1);
    const u32 start = guest.dword(record + kAsyncFileStart);
    guest.setDword(request + kRequestKind, kKindRead);
    guest.setDword(request + kRequestOwned, 0);
    guest.setDword(request + kRequestName, 0);
    guest.setDword(request + kRequestSize, size);
    guest.setDword(request + kRequestBuffer, buffer);
    guest.setDword(request + kRequestOffset, start + offset);
    guest.setDword(request + kRequestPool, guest.dword(kCurrentPool));
    guest.setDword(request + kRequestCallback, callback);
    fe::direct<0x50de50, netb::qpush>(app, ts, record + kAsyncWaiting, request);
    asyncWake(guest, ts);
    return guest.dword(request + kRequestId);
}

nfs::u32 frontend::asyncReadAt(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f1500", 0, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 offset = args.eax, buffer = args.edx, size = args.ebx, callback = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::asyncReadAt(app, ts, offset, buffer, size, callback); });
}

/* sub_4f15d0: a read with no callback. */
u32 frontend::wata::asyncRead(App* app, ThreadState& ts, u32 offset, u32 buffer, u32 size)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    // On to sub_4f1500 as they came, the callback none.
    return fe::direct<0x4f1500, asyncReadAt>(app, ts, offset, buffer, size, 0);
}

nfs::u32 frontend::asyncRead(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f15d0", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const u32 offset = args.eax, buffer = args.edx, size = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::asyncRead(app, ts, offset, buffer, size); });
}

/* sub_4f1790: a read's end: 1 done, -1 failed, 0 not yet. */
u32 frontend::wata::asyncReadStatus(App* app, ThreadState& ts, u32 id)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 3 * 4);
    constexpr u32 kFunction = 0x54c8ec;
    if (asyncRecord(guest) == 0)
        reportError(guest, kAsyncSource, kFunction, 0x377, 0x54c900);
    const u32 request = fe::direct<0x50e5a0, watb::queueFetch>(app, ts, asyncRecord(guest) + kAsyncDone, kRequestHasId, id);
    if (request == 0)
        return 0;
    fe::direct<0x50de50, netb::qpush>(app, ts, asyncRecord(guest) + kAsyncFree, request);
    return guest.dword(request + kRequestKind) != 0 ? 1 : 0xffffffffu;
}

nfs::u32 frontend::asyncReadStatus(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f1790", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 id = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::asyncReadStatus(app, ts, id); });
}

/* sub_4f1820: the reader's thread: a request at a time from the waiting queue, until it is told
 * to close.  Port: no read waits for the music stream to be topped up first (0x4ff670, 0x4ff7f0),
 * as the game's did so one CD drive would not seek under the music. */
u32 frontend::wata::asyncThreadMain(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 6 * 4 + 0x108);
    constexpr u32 kFunction = 0x54c930;
    constexpr u32 kKindLoad = 1, kKindSetFile = 2, kKindRead = 3, kKindClose = 4;
    const u32 path = guest.frame(), handle = path + 0x100;
    const u32 event = *fe::direct<0x4dfb00, ea::newAutoEvent>(app, ts);
    {
        const u32 record = asyncRecord(guest);
        guest.setDword(record + kAsyncEvent, event);
        guest.setDword(record + kAsyncCurrent, 0);
        guest.setDword(record + kAsyncClosing, 0);
    }
    for (;;)
    {
        if (guest.dword(asyncRecord(guest) + kAsyncClosing) != 0)
        {
            const u32 file = guest.dword(asyncRecord(guest) + kAsyncFile);
            if (file != 0)
                fe::direct<0x4e2da0, ea::closeChecked>(app, ts, file);
            guest.setDword(asyncRecord(guest) + kAsyncFile, 0);
            guest.setDword(asyncRecord(guest) + kAsyncCurrent, 0);
            fe::direct<0x4dfc00, ea::eventClosed>(app, ts, guest.dword(asyncRecord(guest) + kAsyncEvent));
            const u32 record = asyncRecord(guest);
            guest.setDword(record + kAsyncEvent, 0);
            return record;
        }
        const u32 request = fe::direct<0x50dff0, netb::qpoplast>(app, ts, asyncRecord(guest) + kAsyncWaiting);
        if (request == 0)
        {
            fe::direct<0x4dfb70, ea::eventWaited>(app, ts, guest.dword(asyncRecord(guest) + kAsyncEvent));
            continue;
        }
        guest.setDword(asyncRecord(guest) + kAsyncCurrent, request);
        const auto finished = [&] {
            const u32 queue = guest.dword(request + kRequestCallback) != 0 ? kAsyncDoneCallback : kAsyncDone;
            fe::direct<0x50de50, netb::qpush>(app, ts, asyncRecord(guest) + queue, request);
        };
        const u32 kind = guest.dword(request + kRequestKind);
        if (kind == kKindLoad)
        {
            const u32 name = guest.dword(request + kRequestName);
            guest.copyString(path, name);
            fe::direct<0x4e1890, ea::purgeBlock>(app, ts, name);
            guest.setDword(request + kRequestName, 0);
            fe::direct<0x4e28d0, ea::openToRead>(app, ts, path, handle, request + kRequestOffset, request + kRequestSize);
            guest.setDword(request + kRequestOwned, 0);
            const u32 file = guest.dword(handle);
            if (file != 0)
            {
                if (guest.dword(request + kRequestBuffer) == 0)
                {
                    guest.setDword(kErrorFile, kAsyncSource);
                    guest.setDword(kErrorFunction, kFunction);
                    guest.setDword(kErrorLine, 0x3a7);
                    const u32 memory = *fe::direct<0x4e1620, ea::reserved>(app, ts, path, guest.dword(request + kRequestSize), guest.dword(request + kRequestPool));
                    guest.setDword(request + kRequestOwned, memory);
                    guest.setDword(request + kRequestBuffer, memory);
                }
                else
                {
                    guest.setDword(request + kRequestOwned, guest.dword(asyncRecord(guest) + kAsyncSharedBuffer));
                }
                fe::direct<kLockDrive, ea::handleDriveLocked>(app, ts, guest.dword(handle));
                // The game's code hands it the request in ebx too, which it does not read.
                fe::direct<0x4e2e60, ea::seekChecked>(app, ts, guest.dword(handle), guest.dword(request + kRequestOffset));
                if (*fe::direct<0x4e2de0, ea::readChecked>(app, ts, guest.dword(handle), guest.dword(request + kRequestBuffer), guest.dword(request + kRequestSize)) == 0)
                    guest.setDword(request + kRequestKind, 0);
                fe::direct<kUnlockDrive, ea::handleDriveUnlocked>(app, ts, guest.dword(handle));
                fe::direct<0x4e2da0, ea::closeChecked>(app, ts, guest.dword(handle));
                guest.setDword(handle, 0);
                if (guest.dword(request + kRequestKind) == 0
                    && guest.dword(request + kRequestOwned) != guest.dword(asyncRecord(guest) + kAsyncSharedBuffer))
                {
                    fe::direct<0x4e1890, ea::purgeBlock>(app, ts, guest.dword(request + kRequestOwned));
                    guest.setDword(request + kRequestOwned, 0);
                    guest.setDword(request + kRequestBuffer, 0);
                }
            }
            else
            {
                guest.setDword(request + kRequestBuffer, 0);
            }
            finished();
        }
        else if (kind == kKindSetFile)
        {
            const u32 file = guest.dword(asyncRecord(guest) + kAsyncFile);
            if (file != 0)
                fe::direct<0x4e2da0, ea::closeChecked>(app, ts, file);
            guest.setDword(asyncRecord(guest) + kAsyncFile, 0);
            if (guest.dword(request + kRequestName) != 0)
            {
                const u32 record = asyncRecord(guest);
                fe::direct<0x4e28d0, ea::openToRead>(app, ts, guest.dword(request + kRequestName), record + kAsyncFile, record + kAsyncFileStart, record + kAsyncFileSize);
                fe::direct<0x4e1890, ea::purgeBlock>(app, ts, guest.dword(request + kRequestName));
                guest.setDword(request + kRequestName, 0);
            }
            fe::direct<0x50de50, netb::qpush>(app, ts, asyncRecord(guest) + kAsyncFree, request);
        }
        else if (kind == kKindRead)
        {
            if (guest.dword(asyncRecord(guest) + kAsyncFile) == 0)
            {
                guest.setDword(request + kRequestKind, 0);
            }
            else
            {
                fe::direct<kLockDrive, ea::handleDriveLocked>(app, ts, guest.dword(asyncRecord(guest) + kAsyncFile));
                fe::direct<0x4e2e60, ea::seekChecked>(app, ts, guest.dword(asyncRecord(guest) + kAsyncFile), guest.dword(request + kRequestOffset));
                if (*fe::direct<0x4e2de0, ea::readChecked>(app, ts, guest.dword(asyncRecord(guest) + kAsyncFile), guest.dword(request + kRequestBuffer), guest.dword(request + kRequestSize)) == 0)
                    guest.setDword(request + kRequestKind, 0);
                fe::direct<kUnlockDrive, ea::handleDriveUnlocked>(app, ts, guest.dword(asyncRecord(guest) + kAsyncFile));
            }
            finished();
        }
        else if (kind == kKindClose)
        {
            const u32 file = guest.dword(asyncRecord(guest) + kAsyncFile);
            if (file != 0)
                fe::direct<0x4e2da0, ea::closeChecked>(app, ts, file);
            const u32 record = asyncRecord(guest);
            guest.setDword(record + kAsyncFile, 0);
            fe::direct<0x50de50, netb::qpush>(app, ts, record + kAsyncFree, request);
        }
        else
        {
            reportError(guest, kAsyncSource, kFunction, 0x3fb, 0x54c940);
        }
        guest.setDword(asyncRecord(guest) + kAsyncCurrent, 0);
        fe::direct<0x4df8e0, ea::sleepFor>(app, ts, 1);
    }
}

nfs::u32 frontend::asyncThreadMain(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f1820", 0, 0, nullptr, 0, fe::kRestoreAll };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::asyncThreadMain(app, ts); });
}

/* ---------------------------------------------------------------------------
 * The small ones the libraries share
 * ------------------------------------------------------------------------- */

namespace
{

// A function that ends by jumping to another: what it calls runs where a jump would run it.
constexpr u32 kTailCall = u32(0) - 4;

constexpr u32 kTicks = 0x79c1a4;

// The copy behind the spans' front doors: the memmove (native_mix.cpp), by address.
constexpr u32 kSpanCopy = 0x5033e4;

/* A tail call with the count (ebx) turned from one size into another's, the pixels (eax) and where
 * they go (edx) on to the copy as they came. */
template <typename Scale>
u32 tailScaled(nfs::App* app, nfs::ThreadState& ts, u32 from, u32 to, u32 count, Scale scale)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, kTailCall);
    return fe::pointerCall(guest, kSpanCopy, args({ from, to, scale(count) }));
}

u32 half(u32 count) { return u32(s32(count) >> 1); }
u32 twice(u32 count) { return count + count; }
u32 fourTimes(u32 count) { return count << 2; }

}

/* sub_4f0630: the timer's ticks. */
u32 frontend::wata::timerNow(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    return guest.dword(kTicks);
}

nfs::u32 frontend::timerNow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f0630", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::timerNow(app, ts); });
}

/* sub_4f0650: the ticks since the last time it was asked. */
u32 frontend::wata::timerSinceLast(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    constexpr u32 kLast = 0x5651dc;
    const u32 last = guest.dword(kLast);
    const u32 now = guest::direct<0x4f0630, timerNow>(app, ts);
    guest.setDword(kLast, now);
    return now - last;
}

nfs::u32 frontend::timerSinceLast(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f0650", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::timerSinceLast(app, ts); });
}

/* sub_4f06e0: ticks waited for, the system's tasks run meanwhile. */
u32 frontend::wata::timerWait(App* app, ThreadState& ts, u32 ticks)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    const u32 until = ticks + guest::direct<0x4f0630, timerNow>(app, ts);
    u32 left = guest::direct<0x4f0630, timerNow>(app, ts) - until;
    while (s32(left) < 0)
    {
        giveWayInWait(app, ts);
        fe::direct<0x4e7630, port::stream::runDueTasks>(app, ts, ts.sp - 4, 0);
        left = guest::direct<0x4f0630, timerNow>(app, ts) - until;
    }
    return left;
}

nfs::u32 frontend::timerWait(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f06e0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const u32 ticks = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::timerWait(app, ts, ticks); });
}

/* sub_4f0840: the screen's colour depth, as the text keeps it. */
u32 frontend::wata::textDepth(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    return guest.dword(0x567218);
}

nfs::u32 frontend::textDepth(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f0840", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::textDepth(app, ts); });
}

/* sub_4f0850: the text's pen. */
u32 frontend::wata::textSetPen(App* app, ThreadState& ts, u32 x, u32 y)
{
    Guest guest(app, ts);
    guest.setDword(0x5671fc, y);
    guest.setDword(0x5671f8, x);
    return x;
}

nfs::u32 frontend::textSetPen(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f0850", 0, 0, nullptr, 0, 0 };
    const u32 x = args.eax, y = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::textSetPen(app, ts, x, y); });
}

namespace
{
constexpr u32 kFontPalette = 0x567280;

/* Whether sub_50c180 calls nothing and hands back eax as it came: no colours given, no fade. */
bool textPaletteLeavesEax(const Guest& guest)
{
    return guest.dword(0x5672a4) == 0 && !(guest.dword(0x56720c) != 0 && guest.dword(0x567208) == 4);
}
}

/* sub_4f0860: the text's attribute set, and on to what follows it, sub_50c0d0, which hands back the
 * value as it came where its palette's code hands eax on. */
u32 frontend::wata::textSetAttribute(App* app, ThreadState& ts, u32 value)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, kTailCall);
    guest.setDword(0x56720c, value);
    const bool passedOn = guest.dword(kFontPalette) != 0 && textPaletteLeavesEax(guest);
    const u32 result = fe::direct<0x50c0d0, watb::initTextDraw>(app, ts);
    return passedOn ? value : result;
}

nfs::u32 frontend::textSetAttribute(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f0860", 0, 0, nullptr, 0, 0 };
    const u32 value = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::textSetAttribute(app, ts, value); });
}

/* The spans' front doors in the copy set the port's processor has (sub_4f09c0), its table at
 * 0x56524c: a count of pixels turned into one of bytes (halved for 4 bits, doubled for 16, three
 * times for 24, four times for 32).  The sets of the other processors are gone. */
u32 frontend::wata::mixD4Bit(App* app, ThreadState& ts, u32 from, u32 to, u32 count)
{
    return tailScaled(app, ts, from, to, count, half);
}

nfs::u32 frontend::mixD4Bit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f0970", 0, 0, nullptr, 0, 0 };
    const u32 from = args.eax, to = args.edx, count = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::mixD4Bit(app, ts, from, to, count); });
}

u32 frontend::wata::mixD16Bit(App* app, ThreadState& ts, u32 from, u32 to, u32 count)
{
    return tailScaled(app, ts, from, to, count, twice);
}

nfs::u32 frontend::mixD16Bit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f0980", 0, 0, nullptr, 0, 0 };
    const u32 from = args.eax, to = args.edx, count = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::mixD16Bit(app, ts, from, to, count); });
}

u32 frontend::wata::mixD32Bit(App* app, ThreadState& ts, u32 from, u32 to, u32 count)
{
    return tailScaled(app, ts, from, to, count, fourTimes);
}

nfs::u32 frontend::mixD32Bit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f09b0", 0, 0, nullptr, 0, 0 };
    const u32 from = args.eax, to = args.edx, count = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::mixD32Bit(app, ts, from, to, count); });
}

/* Three times the count, for 24 bits; the pixels in eax (and in ecx too, which the copy does not
 * read), where they go in edx. */
u32 frontend::wata::mixD24Bit(App* app, ThreadState& ts, u32 samples, u32 to, u32 count)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    return fe::pointerCall(guest, kSpanCopy, args({ samples, to, count * 3 }));
}

nfs::u32 frontend::mixD24Bit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f0990", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const u32 samples = args.eax, to = args.edx, count = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::mixD24Bit(app, ts, samples, to, count); });
}

/* sub_4f09c0: the shape drawn by the blitters of the copy set the port's processor has, MMX's, the
 * fourth in the table at 0x565274 (the game chose one the first time it drew, by the processor):
 * the shape and its place (eax, edx and ebx, which the game's code hands on as they came).  The
 * MMX blitter's emms is nothing here: the x87 is not shared. */
u32 frontend::wata::blitDispatchAt(App* app, ThreadState& ts, u32 shape, u32 x, u32 y)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    constexpr u32 kMmxBlitters = 0x565280;
    return fe::direct<0x50b0b0, watb::moveShape>(app, ts, shape, x, y, guest.dword(kMmxBlitters));
}

nfs::u32 frontend::blitDispatch(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f09c0", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const u32 shape = args.eax, x = args.edx, y = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::blitDispatchAt(app, ts, shape, x, y); });
}

/* sub_4f0a10: the same, the place the two 12-bit fields of the shape's position word. */
u32 frontend::wata::blitShapeAtPosition(App* app, ThreadState& ts, u32 shape)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 2 * 4);
    const u32 position = guest.dword(shape + 0xc);
    const u32 y = u32(s32(position << 4) >> 20), x = u32(s32(position << 20) >> 20);
    return fe::direct<0x4f09c0, blitDispatchAt>(app, ts, shape, x, y);
}

nfs::u32 frontend::blitShapeAtPosition(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f0a10", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEdx };
    const u32 shape = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::blitShapeAtPosition(app, ts, shape); });
}

/* sub_4f0a90: a surface filled with a colour and drawn. */
u32 frontend::wata::surfaceFillAndDraw(App* app, ThreadState& ts, u32 x, u32 y, u32 width, u32 height, u32 colour)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 2 * 4);
    constexpr u32 kFillWord = 0x4e0721;
    const u32 surface = fe::direct<0x4ef190, eab::windowBitmapCreateDefault>(app, ts, width, height, 0x20, 0);
    const u32 words = guest.dword(surface + 8) * guest.dword(surface + 0x28);
    fe::direct<kFillWord, ea::fillPattern>(app, ts, guest.dword(surface + 0x20) + 0x10, words, colour);
    fe::direct<0x50db30, watb::drawAlphaShape>(app, ts, guest.dword(surface + 0x20), x, y);
    return fe::direct<0x4eff00, eab::windowBitmapFree>(app, ts, surface);
}

nfs::u32 frontend::surfaceFillAndDraw(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f0a90", 4, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 x = args.eax, y = args.edx, width = args.ebx, height = args.ecx;
    const u32 colour = app->getMemory<u32>(ts.sp + 4);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::surfaceFillAndDraw(app, ts, x, y, width, height, colour); });
}

/* sub_4f0af0: the text's measure, in place of its drawing: the box grown by a glyph.  Its arguments
 * are a glyph drawer's, four on the stack. */
u32 frontend::wata::textMeasureGlyph(App* app, ThreadState& ts, s32 x, s32 y, s32 width, s32 height)
{
    Guest guest(app, ts);
    constexpr u32 kLeft = 0x9f0bdc, kTop = 0x9f0bd8, kRight = 0x9f0bd0, kBottom = 0x9f0bd4;
    const s32 left = s32(guest.dword(kLeft));
    guest.setDword(kLeft, u32(x <= left ? x : left));
    const s32 top = s32(guest.dword(kTop));
    const u32 newTop = u32(y <= top ? y : top);
    guest.setDword(kTop, newTop);
    const s32 right = s32(guest.dword(kRight));
    const s32 reach = s32(u32(x) + u32(width));
    guest.setDword(kRight, u32(reach < right ? right : reach));
    const s32 bottom = s32(guest.dword(kBottom));
    const s32 foot = s32(u32(y) + u32(height));
    const u32 newBottom = u32(foot < bottom ? bottom : foot);
    guest.setDword(kBottom, newBottom);
    return u32(bottom);
}

nfs::u32 frontend::textMeasureGlyph(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f0af0", 0x10, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const s32 x = s32(args.eax), y = s32(args.edx);
    const s32 width = s32(app->getMemory<u32>(ts.sp + 8));
    const s32 height = s32(app->getMemory<u32>(ts.sp + 12));
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::textMeasureGlyph(app, ts, x, y, width, height); });
}

/* sub_4f0c90: sub_4f0b60 with room for the whole string. */
u32 frontend::wata::textMeasure(App* app, ThreadState& ts, u32 text, u32 leftOut, u32 topOut, u32 widthOut, u32 heightOut)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    return fe::direct<0x4f0b60, textMeasureBox>(app, ts, text, s32(0x100), leftOut, topOut, widthOut, heightOut);
}

nfs::u32 frontend::textMeasure(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f0c90", 4, 0, nullptr, 0, fe::kRestoreEsi };
    const u32 text = args.eax, leftOut = args.edx, topOut = args.ebx, widthOut = args.ecx;
    const u32 heightOut = app->getMemory<u32>(ts.sp + 4);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::textMeasure(app, ts, text, leftOut, topOut, widthOut, heightOut); });
}

/* sub_4f4650: a string and a value into the next row of the table at 0x79f524. */
u32 frontend::wata::registryAddEntry(App* app, ThreadState& ts, u32 name, u32 value)
{
    Guest guest(app, ts);
    constexpr u32 kCount = 0x5662e0, kNames = 0x79f524, kValues = 0x79f52c, kRowBytes = 12;
    guest.copyString(kNames + guest.dword(kCount) * kRowBytes, name);
    const u32 row = guest.dword(kCount);
    guest.setDword(kValues + row * kRowBytes, value);
    guest.setDword(kCount, row + 1);
    return row * 3;
}

nfs::u32 frontend::registryAddEntry(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f4650", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 name = args.eax, value = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::registryAddEntry(app, ts, name, value); });
}

/* sub_4f46b0: the current screen's surface's pixels. */
u32 frontend::wata::screenPixels(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    return guest.dword(guest.dword(0x55e558) * 8 + guest.dword(0x79f5c4)) + 0x10;
}

nfs::u32 frontend::screenPixels(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f46b0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::screenPixels(app, ts); });
}

namespace
{
constexpr u32 kWindowFirst = 0x564ff4, kWindowSecond = 0x56502c, kWindowDwords = 0xe;
}

/* sub_4f46d0: the window's two descriptions, kept. */
u32 frontend::wata::windowSave(App* app, ThreadState& ts, u32 to)
{
    Guest guest(app, ts);
    copyDwords(guest, to, kWindowFirst, kWindowDwords);
    copyDwords(guest, to + kWindowDwords * 4, kWindowSecond, kWindowDwords);
    return to;
}

nfs::u32 frontend::windowSave(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f46d0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 to = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::windowSave(app, ts, to); });
}

/* sub_4f4700: the window's descriptions put back as sub_4f46d0 kept them: the check off, for it. */
u32 frontend::wata::windowRestore(App* app, ThreadState& ts, u32 from)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4 * 4);
    constexpr u32 kChecking = 0x56440c;
    const u32 checking = guest.dword(kChecking);
    guest.setDword(kChecking, 0);
    const u32 result = fe::direct<0x4eada0, eaa::bitmapDrawInto>(app, ts, from);   // as it came
    guest.setDword(kChecking, checking);
    copyDwords(guest, kWindowSecond, from + kWindowDwords * 4, kWindowDwords);
    return result;
}

nfs::u32 frontend::windowRestore(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f4700", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 from = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::windowRestore(app, ts, from); });
}

namespace
{
constexpr u32 kTextState = 0x5671f4, kTextStateDwords = 0x2e;
}

/* sub_4f4740: the text's whole state, kept. */
u32 frontend::wata::textStateSave(App* app, ThreadState& ts, u32 to)
{
    Guest guest(app, ts);
    copyDwords(guest, to, kTextState, kTextStateDwords);
    return to;
}

nfs::u32 frontend::textStateSave(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f4740", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 to = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::textStateSave(app, ts, to); });
}

/* sub_4f4760: and put back. */
u32 frontend::wata::textStateRestore(App* app, ThreadState& ts, u32 from)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 3 * 4);
    copyDwords(guest, kTextState, from, kTextStateDwords);
    // On to sub_50c180, which hands back `from` as it came where it calls nothing.
    const bool passedOn = textPaletteLeavesEax(guest);
    const u32 result = fe::direct<0x50c180, watb::textPalette>(app, ts);
    return passedOn ? from : result;
}

nfs::u32 frontend::textStateRestore(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f4760", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 from = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::textStateRestore(app, ts, from); });
}

/* sub_4f4780: the checksum of a block, by the table at 0x566b54. */
u32 frontend::wata::blockChecksum(App* app, ThreadState& ts, u32 start, u32 size)
{
    Guest guest(app, ts);
    const u32 end = start + size;
    constexpr u32 kTable = 0x566b54;
    u32 sum = 0xfbea;
    for (u32 at = start; at < end; ++at)
    {
        const u32 index = (sum & 0xff) ^ guest.byte(at);
        sum = (sum >> 8) ^ guest.word(kTable + index * 2);
    }
    return sum;
}

nfs::u32 frontend::blockChecksum(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f4780", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi };
    const u32 start = args.eax, size = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::blockChecksum(app, ts, start, size); });
}

/* sub_4f47c0: how a shape's file ends: 1 at a BEND, 0 not, 2 of a kind that has no such end. */
u32 frontend::wata::shapeEndsAtBend(App* app, ThreadState& ts, u32 shape)
{
    Guest guest(app, ts);
    constexpr u32 kBend = 0x42454e44;
    const std::uint8_t flags = guest.byte(shape + 3);
    if ((flags & 2) == 0 || (flags & 0xc0) != 0)
        return 2;
    const u32 raw = guest.dword(shape + 0x10 + guest.dword(shape + 4));
    const u32 word = (raw >> 24) | ((raw >> 8) & 0xff00) | ((raw << 8) & 0xff0000) | (raw << 24);
    return word == kBend ? 1 : 0;
}

nfs::u32 frontend::shapeEndsAtBend(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f47c0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    const u32 shape = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::shapeEndsAtBend(app, ts, shape); });
}

/* sub_4f87e0: a graphics driver's DLL opened: the name given, after a '!', or the environment's. */
u32 frontend::wata::thrashOpenByName(App* app, ThreadState& ts, u32 given)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    constexpr u32 kGetenv = 0x50ead0, kDriverVariable = 0x54d700, kOpen = 0x4f8390;
    constexpr u32 kStarted = 0x564a68, kOpened = 0x564a6c;
    u32 name = given;
    if (guest.byte(given) == '!')
    {
        name = given + 1;
    }
    else
    {
        const u32 fromEnvironment = fe::direct<kGetenv, watcom::clib::getenv>(guest.app(), guest.ts(), kDriverVariable);
        if (fromEnvironment != 0)
            name = fromEnvironment;
    }
    if (guest.dword(kStarted) == 0)
        guest.setDword(kStarted, 1);
    if (guest.dword(kOpened) != 0)
        fe::direct<0x4f87a0, wina::releaseDriverSlot>(app, ts);
    return *fe::direct<kOpen, ea::thrashOpened>(app, ts, name);
}

nfs::u32 frontend::thrashOpenByName(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f87e0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const u32 given = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::thrashOpenByName(app, ts, given); });
}

/* sub_4f90e0: the window shown again, its descriptions kept through it. */
u32 frontend::wata::windowReshow(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 0x70);
    if (guest.dword(0x565028) == 0)
        return 1;
    if (guest.dword(0x567dcc) == 0)
        return fe::direct<0x4ed330, menz::ddrawRelockSurface>(app, ts, 0x9f38d0);
    const u32 kept = guest.frame();
    fe::direct<0x4f46d0, windowSave>(app, ts, kept);
    fe::direct<0x4ead90, eaa::bitmapDrawToScreen>(app, ts);
    fe::direct<0x4f4700, windowRestore>(app, ts, kept);
    return 1;
}

nfs::u32 frontend::windowReshow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f90e0", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::windowReshow(app, ts); });
}

/* sub_4f92b0: the heap's block allocated and handed over. */
u32 frontend::wata::heapStart(App* app, ThreadState& ts, u32 size)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    const u32 block = fe::direct<0x4f7900, watcom::clib::malloc>(app, ts, size);
    guest.setDword(0x5644fc, block);
    return fe::direct<0x4f9270, menz::memClassCreateRam>(app, ts, block, size);
}

nfs::u32 frontend::heapStart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f92b0", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const u32 size = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::heapStart(app, ts, size); });
}

/* sub_4f92d0: 64 megabytes of it: the port's heap budget change's, 16 in the game. */
u32 frontend::wata::heapStartDefault(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    return fe::direct<0x4f92b0, heapStart>(app, ts, 0x4000000);
}

nfs::u32 frontend::heapStartDefault(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f92d0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::heapStartDefault(app, ts); });
}

/* sub_4f9320: the fonts' system started, once.  Started already, no result: eax as it came, which
 * its thunk does. */
u32 frontend::wata::fontsStart(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    if (guest.dword(0x567194) != 0)
        return 0;
    fe::direct<0x518fd0, menz::videoBuffersStart>(app, ts, 0);
    guest.setDword(0x567190, fe::direct<0x519f10, menz::fontCreate>(app, ts, 0x569064));
    const u32 result = fe::direct<0x4f2a78, eab::atexitJump>(app, ts, u32(0x4f9360));
    // The 1 the code put in ecx before atexit, which keeps it.
    guest.setDword(0x567194, 1);
    return result;
}

nfs::u32 frontend::fontsStart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f9320", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const u32 previous = args.eax;
    const bool started = app->getMemory<u32>(0x567194) != 0;
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        const u32 result = wata::fontsStart(app, ts);
        return started ? previous : result;
    });
}

/* sub_4f99f0: TAPI's line opened, the callback at 0x4f9500. */
u32 frontend::wata::tapiInitialize(App* app, ThreadState& ts, u32 instance, u32 wanted)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    constexpr u32 kLines = 0x9f4d90, kLineApp = 0x9f4d74, kCallback = 0x4f9500, kLineInitialize = 0x5310be;
    guest.setDword(0x9f4d7c, wanted);
    guest.setDword(0x9f4d88, 0);
    guest.setDword(0x9f4da8, 0);
    guest.setDword(0x9f4da4, 0);
    guest.setDword(0x9f4da0, 0);
    guest.setDword(0x9f4d94, 0);
    const u32 result = fe::direct<kLineInitialize, watc::lineInitialize>(app, ts, kLineApp, instance, kCallback,
                                                                          u32(0), kLines);
    if (result != 0)
        return 0xa;
    return guest.dword(kLines) == 0 ? 7 : result;
}

nfs::u32 frontend::tapiInitialize(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f99f0", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const u32 instance = args.eax, wanted = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::tapiInitialize(app, ts, instance, wanted); });
}

/* sub_4f9920: TAPI set up for the modem games: 0, or what went wrong. */
u32 frontend::wata::tapiSetup(App* app, ThreadState& ts, u32 wanted)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 3 * 4);
    constexpr u32 kTapiReady = 0x5671a8, kAcquireWindow = 0x4eba70;
    constexpr u32 kInitialize = 0x4f99f0;
    constexpr u32 kFailed = 0xa;
    guest.setDword(0x9f4d9c, 0);
    if (guest.dword(kTapiReady) == 0xffffffffu)
    {
        guest.setDword(kErrorFile, 0x54da88);
        guest.setDword(kErrorFunction, 0x54da98);
        guest.setDword(kErrorLine, 0x123);
        fe::direct<kError, ea::error>(guest.app(), guest.ts(), 0x54daa4, guest::Words({}));
        return kFailed;
    }
    // The window's module, for TAPI: the port's windows have none of their own, 0.
    fe::direct<kAcquireWindow, ea::acquireWindow>(app, ts);
    const u32 instance = 0;
    const u32 first = fe::direct<kInitialize, tapiInitialize>(app, ts, instance, u32(1));
    if (first != 0)
    {
        fe::direct<0x4f97c0, eab::lineShutdown>(app, ts);
        return first;
    }
    if (fe::direct<0x4f9d20, netb::tapiOpenLine>(app, ts) != 0)
    {
        fe::direct<0x4f9df0, netb::tapiGetCommHandleConfig>(app, ts);
        fe::direct<0x4f97c0, eab::lineShutdown>(app, ts);
        fe::direct<0x4df8e0, ea::sleepFor>(app, ts, 0x1f4);
        // The instance and the wanted, which the code keeps in ecx and ebx across the calls (where
        // TAPI's stub keeps them: the port's TAPI changes no register).
        if (fe::direct<0x4f99f0, tapiInitialize>(app, ts, instance, wanted) != kFailed)
        {
            if (fe::direct<0x4f9d20, netb::tapiOpenLine>(app, ts) != 0)
                return 0;
            return kFailed;
        }
    }
    fe::direct<0x4f97c0, eab::lineShutdown>(app, ts);
    return kFailed;
}

nfs::u32 frontend::tapiSetup(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f9920", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 wanted = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::tapiSetup(app, ts, wanted); });
}

/* sub_4fb380: the screen's size and depth from the driver's record into the global ones. */
u32 frontend::wata::screenTakeSize(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    guest.setDword(0x56436c, guest.dword(0x9f4f3c));
    guest.setDword(0x564370, guest.dword(0x9f4f38));
    guest.setDword(0x564374, guest.dword(0x9f4f54));
    return guest.dword(0x564374);
}

nfs::u32 frontend::screenTakeSize(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4fb380", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::screenTakeSize(app, ts); });
}

/* sub_4fb3b0 and sub_4fb3c0: the screen's lock count.  No result: eax as it came, which their
 * thunks do. */
u32 frontend::wata::screenLockCountUp(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    guest.setDword(0x9f4f48, guest.dword(0x9f4f48) + 1);
    return 0;
}

nfs::u32 frontend::screenLockCountUp(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4fb3b0", 0, 0, nullptr, 0, 0 };
    const u32 previous = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        wata::screenLockCountUp(app, ts);
        return previous;
    });
}

u32 frontend::wata::screenLockCountDown(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    guest.setDword(0x9f4f48, guest.dword(0x9f4f48) - 1);
    return 0;
}

nfs::u32 frontend::screenLockCountDown(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4fb3c0", 0, 0, nullptr, 0, 0 };
    const u32 previous = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        wata::screenLockCountDown(app, ts);
        return previous;
    });
}

/* sub_4fb3d0: the driver's record set up for the screen, and these functions hooked in. */
u32 frontend::wata::screenInstallDriver(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    guest.setDword(0x9f4f44, guest.dword(0x564384));
    guest.setDword(0x9f4f50, 0);
    guest.setDword(0x9f4f48, 0);
    guest.setDword(0x9f4f4c, 0);
    guest.setDword(0x564368, 1);
    guest.setDword(0x56445c, 0x4fb3c0);
    guest.setDword(0x564460, 0x4fb3c0);
    guest.setDword(0x9f4f40, guest.dword(0x564388));
    guest.setDword(0x564468, 0x4fb380);
    guest.setDword(0x564464, 0x4fb3b0);
    return 1;
}

nfs::u32 frontend::screenInstallDriver(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4fb3d0", 0, 0, nullptr, 0, fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::screenInstallDriver(app, ts); });
}

/* sub_4fb700: the num lock off. */
u32 frontend::wata::numLockOff(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    return guest::direct<0x4fb710, lockKeySet>(app, ts, 1, 0);
}

nfs::u32 frontend::numLockOff(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4fb700", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::numLockOff(app, ts); });
}

/* sub_4fb790: a directory's search: a big file's entry opened by its name, 0 when found. */
u32 frontend::wata::bigFileOpenEntry(App* app, ThreadState& ts, u32 big, u32 search)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 5 * 4 + 8);
    constexpr u32 kFindEntry = 0x4ea040, kOpenMemory = 0x4e4140;
    const u32 entryOffset = guest.frame(), entrySize = entryOffset + 4;
    const u32 wantedFile = guest.dword(search);
    const bool anyFile = guest.byte(wantedFile) == 0;
    if (!anyFile && fe::direct<kStrcmp, watcom::clib::strcmp>(guest.app(), guest.ts(), big + 0x14, guest.dword(search)) != 0)
        return 1;
    const std::optional<load::BigEntry> found = fe::direct<kFindEntry, load::findEntry>(
        app, ts, guest.dword(big + 0x10), guest.dword(search + 4), u32(0), entryOffset, entrySize);
    if (found->name == 0)
        return anyFile ? 1 : 0;
    const u32 handle = *fe::direct<kOpenMemory, ea::memoryOpened>(app, ts, guest.dword(big + 0x10) + guest.dword(entryOffset), guest.dword(entrySize), guest.dword(big + 8), 0u, search + 8);
    guest.setDword(search + 0xc, handle);
    return 0;
}

nfs::u32 frontend::bigFileOpenEntry(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4fb790", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 big = args.eax, search = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::bigFileOpenEntry(app, ts, big, search); });
}

/* sub_4ead20: the window as it stands set: an error if it is locked. */
u32 frontend::wata::windowApply(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, kTailCall);
    constexpr u32 kLockedBy = 0x9f3d2f, kWindow = 0x56502c;
    if (guest.byte(kLockedBy) != 0)
    {
        guest.setDword(kErrorFile, 0x54b6c8);
        guest.setDword(kErrorFunction, 0x54b6d8);
        guest.setDword(kErrorLine, 0x5d);
        fe::direct<0x401010, ea::error>(app, ts, 0x54b6e8u, guest::Words({ guest.byte(kLockedBy) }));
    }
    return fe::direct<0x4eada0, eaa::bitmapDrawInto>(app, ts, kWindow);
}

nfs::u32 frontend::windowApply(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4ead20", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::windowApply(app, ts); });
}

/* sub_4f7c48: the exit functions registered at 0x9f380c run, last first, once.  No result: what
 * the last of them left in eax, or with none run eax as it came, which its thunk does. */
u32 frontend::wata::exitFunctionsRun(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 5 * 4);
    constexpr u32 kCount = 0x9f3890, kTable = 0x9f380c, kDone = 0x21;
    const u32 count = guest.dword(kCount);
    if (count == kDone)
        return 0;
    guest.setDword(kCount, kDone);
    u32 last = 0;
    for (u32 left = count; left != 0; --left)
    {
        const u32 function = guest.dword(kTable + left * 4);
        last = fe::pointerCall(guest, function, args({ function }));
    }
    return last;
}

nfs::u32 frontend::exitFunctionsRun(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f7c48", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEdx };
    const u32 previous = args.eax;
    const u32 count = app->getMemory<u32>(0x9f3890);
    const bool noneRun = count == 0x21 || count == 0;
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        const u32 last = wata::exitFunctionsRun(app, ts);
        return noneRun ? previous : last;
    });
}

/* sub_4e0658, sub_4e0659 and sub_4f45d0: nothing, in a table of what can be hooked.  No result: eax
 * as it came, which their thunks do. */
u32 frontend::wata::nothingA(App* app, ThreadState& ts)
{
    return 0;
}

nfs::u32 frontend::nothingA(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4e0658", 0, 0, nullptr, 0, 0 };
    const u32 previous = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        wata::nothingA(app, ts);
        return previous;
    });
}

u32 frontend::wata::nothingB(App* app, ThreadState& ts)
{
    return 0;
}

nfs::u32 frontend::nothingB(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4e0659", 0, 0, nullptr, 0, 0 };
    const u32 previous = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        wata::nothingB(app, ts);
        return previous;
    });
}

u32 frontend::wata::nothingC(App* app, ThreadState& ts)
{
    return 0;
}

nfs::u32 frontend::nothingC(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f45d0", 0, 0, nullptr, 0, 0 };
    const u32 previous = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        wata::nothingC(app, ts);
        return previous;
    });
}

/* sub_4f8930: the graphics started: the driver's settings in the environment the first time, the
 * display mode, and the libraries over it. */
u32 frontend::wata::graphicsStart(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 2 * 4);
    constexpr u32 kSetenv = 0x4f8b50;
    constexpr u32 kHeap = 0x5644fc, kModeSet = 0x564364, kWidth = 0x564384, kHeight = 0x564388, kDepth = 0x56438c;
    constexpr u32 kYes = 0x54d7d8, kNo = 0x54d7f0, kEmpty = 0x54d7ac;
    struct Setting
    {
        u32 name, value, overwrite;
    };
    static constexpr Setting kSettings[] = {
        { 0x54d7b0, kEmpty, 0 }, { 0x54d7c0, kEmpty, 0 }, { 0x54d7dc, kYes, 1 }, { 0x54d7f4, kNo, 1 }, { 0x54d800, kNo, 1 },
        { 0x54d80c, kNo, 1 }, { 0x54d818, kNo, 1 }, { 0x54d824, kYes, 1 }, { 0x54d834, kNo, 1 }, { 0x54d844, kNo, 1 },
        { 0x54d854, kNo, 1 }, { 0x54d864, kNo, 1 }, { 0x54d870, kYes, 1 },
    };
    fe::direct<0x4df740, ea::threadsSetUp>(app, ts, 0u);
    if (guest.dword(kHeap) == 0)
        fe::direct<0x4f92d0, heapStartDefault>(app, ts);
    if (guest.dword(kModeSet) == 0)
    {
        for (const Setting& setting : kSettings)
            fe::direct<kSetenv, watcom::clib::setenv>(app, ts, setting.name, setting.value, setting.overwrite);
        const u32 shown = fe::direct<0x5089d0, rendf::setDisplayMode>(app, ts, guest.dword(kWidth), guest.dword(kHeight), guest.dword(kDepth));
        if (guest.dword(kModeSet) == 0)
            return shown;
    }
    fe::direct<0x503e50, wina::paletteSet>(app, ts, 0, 0x100, 0x568c30);
    fe::direct<0x48eff0, port::timerStarted>(app, ts, guest.dword(0x5643d8));
    fe::direct<0x517b30, menz::eaTextStart>(app, ts);
    fe::direct<0x4ef520, eab::fontUse>(app, ts, 0x5662e4);
    const u32 black = fe::direct<0x4ef760, eab::colourPack>(app, ts, 0xff000000u);
    // The game's code hands black in edx to the second too, which it does not read, for the colours'.
    const u32 white = fe::direct<0x4ef760, eab::colourPack>(app, ts, 0xffffffffu);
    fe::direct<0x4ef830, eab::fontSetColours>(app, ts, white, black);
    fe::direct<0x4f9320, fontsStart>(app, ts);
    if (guest.dword(0x564368) == 0)
        fe::direct<0x4fb3d0, screenInstallDriver>(app, ts);
    // What it gives back in eax the game's code has on to sub_4f01f0, which does not read it.
    fe::direct<0x4fb700, numLockOff>(app, ts);
    fe::direct<0x4f01f0, eab::inputStateOpen>(app, ts);
    fe::direct<0x517ba0, menz::keyboardIsPc98>(app, ts);
    // Its own address handed in eax, which it does not read.
    fe::direct<0x4feffc, watb::returnZero>(app, ts);
    fe::direct<0x4f00f0, eab::keySetHookFlag>(app, ts, 0x4f00e0);
    // What the code holds in edi meanwhile (no callee touches it): the keys' hook.
    guest.setDword(0x564404, 0x517d60);
    fe::direct<0x517de0, menz::debugKeysBind>(app, ts);
    fe::direct<0x4fc250, eab::asyncFilesInit>(app, ts, 0, 0);
    fe::direct<0x518040, menz::eaSetLoadHook>(app, ts);
    fe::direct<0x4fd3f0, mova::projectionViewDegrees>(app, ts, 0x1e, 0x14, guest.dword(kWidth), guest.dword(kHeight));
    return fe::direct<0x4fd280, mova::projectionCentre>(app, ts, 0, 0);
}

nfs::u32 frontend::graphicsStart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_4f8930", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::graphicsStart(app, ts); });
}

/* sub_47a970: the sky's shape for a track: its description's words in the shape's name give the
 * sun's direction and what shines. */
u32 frontend::wata::skyLoad(App* app, ThreadState& ts, u32 shapes, u32 shapeName)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 5 * 4 + 0x118);
    constexpr u32 kLoaded = 0x55d27c, kLocateShape = 0x4ef270;
    constexpr u32 kToRadiansA = 0x53b330, kToRadiansB = 0x53b328, kDistance = 0x53b338;
    constexpr u32 kFirstWord = 0x53b300, kSecondWord = 0x53b304, kThirdWord = 0x53b308;
    constexpr u32 kFogWord = 0x53b30c, kNoFogWord = 0x53b314, kBrightWord = 0x53b31c;
    const u32 buffer = guest.frame(), angleBits = buffer + 0x114;
    guest.setDword(kLoaded, 0);
    const u32 shape = fe::direct<kLocateShape, ea::shapeLocated>(app, ts, shapes, shapeName);
    if (shape == 0)
        return 0;
    const u32 name = fe::direct<0x4f24e0, eab::shapeFindHotspot>(app, ts, shape);
    if (name == 0)
        return 0;
    // The number after a word of the name, or no number and it ends.
    bool ended = false;
    const auto numberAfter = [&](u32 word) -> s32 {
        guest.copyString(buffer, name);
        const u32 found = fe::direct<kStrstr, watcom::clib::strstr>(guest.app(), guest.ts(), buffer, word);
        if (found == 0)
        {
            ended = true;
            return 0;
        }
        u32 at = found + 1;
        while (guest.byte(at) != 0 && isDigit(guest, std::uint8_t(guest.byte(at) + 1)))
            ++at;
        guest.setByte(at, 0);
        return s32(fe::direct<kAtoi, watcom::clib::atoi>(guest.app(), guest.ts(), found + 1));
    };
    const s32 first = numberAfter(kFirstWord);
    if (ended)
        return 0;
    const s32 second = numberAfter(kSecondWord);
    if (ended)
        return 0;
    const s32 third = numberAfter(kThirdWord);
    if (ended)
        return 0;
    guest.setDword(0x55d29c, fe::direct<kStrstr, watcom::clib::strstr>(guest.app(), guest.ts(), name, kFogWord) != 0 ? 1 : 0);
    guest.setDword(0x55d2a0, fe::direct<kStrstr, watcom::clib::strstr>(guest.app(), guest.ts(), name, kNoFogWord) != 0 ? 0 : 1);
    guest.setDword(0x55d2a4, fe::direct<kStrstr, watcom::clib::strstr>(guest.app(), guest.ts(), name, kBrightWord) != 0 ? 1 : 0);

    const Float toA = Float(app->getMemory<double>(kToRadiansA));
    const Float toB = Float(app->getMemory<double>(kToRadiansB));
    const Float distance = Float(app->getMemory<double>(kDistance));
    // The sine and the cosine (native_frontend_simd.cpp's): what each would leave on the x87's stack.
    const auto sine = [&](u32 argument) { return Float(fe::direct<0x4ea930, simd::sineValue>(app, ts, argument)); };
    const auto cosine = [&](u32 argument) { return Float(fe::direct<0x4ea950, simd::cosineValue>(app, ts, argument)); };
    const auto scaled = [&](u32 slot, Float value) { guest.setFloat(slot, float(fp::mul(value, distance))); };
    const Float firstAngle = fp::mul(Float(first), toA);
    const Float thirdAngle = fp::mul(Float(third), toB);
    guest.setFloat(angleBits, float(firstAngle));
    guest.setFloat(0x55d298, float(thirdAngle));
    const u32 angle = guest.dword(angleBits);
    const auto argumentFor = [&](s32 degrees) {
        float value = float(fp::mul(Float(degrees), toA));
        u32 bits;
        std::memcpy(&bits, &value, sizeof bits);
        return bits;
    };
    Float kept = fp::mul(cosine(angle), distance);
    const u32 secondBits = argumentFor(second);
    guest.setFloat(0x55d280, float(kept));
    scaled(0x55d284, cosine(secondBits));
    scaled(0x55d288, sine(angle));
    kept = fp::mul(cosine(angle), distance);
    const u32 besideBits = argumentFor(s32(u32(second) - 0x5a));
    guest.setFloat(0x55d28c, float(kept));
    scaled(0x55d290, cosine(besideBits));
    scaled(0x55d294, sine(angle));
    const u32 result = fe::direct<0x4d4080, rende::loadShapeTexture>(app, ts, shapes, shapeName, 0x80, 0x6ff9cc, 7);
    guest.setDword(kLoaded, 1);
    return result;
}

nfs::u32 frontend::skyLoad(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_47a970", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 shapes = args.eax, shapeName = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::skyLoad(app, ts, shapes, shapeName); });
}

/* ---------------------------------------------------------------------------
 * The HUD editor: an element under the mouse
 * ------------------------------------------------------------------------- */

/* the port's HUD editor change's changes, in nfs3hp_main.cpp: the layout as laid out and not as
 * drawn, and how far past the screen's edge a box that draws nothing there may go. */
void hudLayoutDesigned(nfs::App* app, nfs::u32 rect, nfs::u32 element);
nfs::s32 hudDragSlack(nfs::App* app, nfs::u32 object, nfs::u32 side);

namespace
{

constexpr u32 kScreenWidth = 0x7cdac8, kScreenHeight = 0x7cdacc;
constexpr u32 kMouseX = 0x55e5e4, kMouseY = 0x55e5e8;
constexpr u32 kPercentToX = 0x539bd4, kPercentToY = 0x539bdc, kLastPixel = 0x539be4;   // doubles
constexpr u32 kXToPercent = 0x539bec, kYToPercent = 0x539bf4;                          // doubles

// An element of the HUD editor's.
constexpr u32 kElFlags = 5, kElX = 6, kElY = 8, kElPercentY = 0x1a, kElPercentX = 0x1c, kElWidth = 0x1e, kElHeight = 0x20;
constexpr u32 kElView = 0x3c, kElDesigned = 0x50, kElRight = 0x60, kElBottom = 0x64, kElGrabX = 0x68, kElGrabY = 0x6c;
constexpr u32 kElMoved = 0x70, kElPartner = 0x74, kElHandle = 0x78, kElEdges = 0x79, kElDragging = 0x7c;
constexpr std::uint8_t kFlagDragged = 4;
constexpr u32 kDragStart = 0x677e00, kEditedCamera = 0x677dfc, kEditedSide = 0x677df8;
constexpr u32 kSplitScreen = 1;  // GameSettings::playMode
constexpr u32 kSetPointerBounds = 0x4352d0;
constexpr std::int16_t kSmallestSide = 0x1a;

/* sub_4dfd56 (__CHP) and fistp: `value` cut to an integer towards zero, as fe::X87::truncated has it. */
std::int32_t truncated(nfs::App* app, nfs::ThreadState& ts, Float value)
{
    const double chopped = fe::direct<0x4dfd56, watcom::clib::chop>(app, ts, double(value));
    return fp::toInteger<s32>(chopped);
}

}

/* sub_459fe0: an element dropped: where it now is into the layout's table, the HUD rebuilt, and
 * its place on the screen worked out from its percentages again, the percentages from what the
 * drag began with. */
u32 frontend::wata::hudEditorDrop(App* app, ThreadState& ts, u32 element, u32 start)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 5 * 4 + 4);
    constexpr u32 kToX = 0x539bbc, kToY = 0x539bc4, kSize = 0x539bcc;   // doubles
    u32 result = element;
    if (gameSettings(guest).playMode != kSplitScreen)
    {
        const auto chop = [&](Float value) { return truncated(app, ts, value); };
        const auto constant = [&](u32 at) { return Float(app->getMemory<double>(at)); };
        const auto wordField = [&](u32 field) { return Float(nfs::s16(guest.word(element + field))); };
        const auto sized = [&](u32 field, u32 screen) {
            return u32(s32(nfs::s16(guest.word(element + field)))) * screen;
        };
        // The element's view in edx across sub_422bd0 (handed to it too, which keeps it), on to sub_4853c0.
        const u32 view = guest.dword(element + kElView);
        const u32 target = fe::direct<0x422bd0, carstate::targetOfCamera>(app, ts, 0);
        const u32 side = (guest.byte(target + 0x200) & 0x20) != 0 ? 1 : 0;
        const u32 layout = SETTINGS_ADDRESS(hud[0].rects) + side * u32(sizeof(HudRecord)) + (view << 4);
        copyDwords(guest, layout, element + kElDesigned, 4);
        hudLayoutDesigned(app, layout, view); /* port: the layout as laid out, not as drawn */
        fe::direct<0x4853c0, hudr::rebuildElement>(app, ts, 0, view);
        fe::direct<0x4800a0, render::hudBandBuilt>(app, ts, 0);
        guest.setWord(element + kElPercentX, guest.word(start + 2));
        guest.setWord(element + kElPercentY, guest.word(start));

        const u32 width = guest.dword(kScreenWidth), height = guest.dword(kScreenHeight);
        const Float toX = constant(kToX);
        guest.setWord(element + kElX, std::uint16_t(chop(fp::mul(Float(s32(sized(kElPercentX, width))), toX))));
        const Float toY = constant(kToY);
        guest.setWord(element + kElY, std::uint16_t(chop(fp::mul(Float(s32(sized(kElPercentY, height))), toY))));
        const Float across = fp::mul(toX, Float(s32(sized(kElWidth, width))));
        const Float rightEdge = fp::add(across, wordField(kElX));
        result = sized(kElHeight, height);
        const Float down = fp::mul(toY, Float(s32(result)));
        const Float size = constant(kSize);
        const Float right = fp::add(rightEdge, size);
        const Float bottomEdge = fp::add(down, wordField(kElY));
        const std::int32_t rightPixel = chop(right);
        const Float bottom = fp::add(size, bottomEdge);
        guest.setDword(element + kElRight, u32(rightPixel));
        guest.setDword(element + kElBottom, u32(chop(bottom)));
    }
    const u32 partner = guest.dword(element + kElPartner);
    guest.setDword(element + kElMoved, 1);
    if (partner != 0)
    {
        guest.setDword(partner + kElMoved, 1);
        guest.setDword(element + kElPartner, 0);
    }
    return result;
}

nfs::u32 frontend::hudEditorDrop(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_459fe0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 element = args.eax, start = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::hudEditorDrop(app, ts, element, start); });
}

/* sub_45a120: an element under the mouse, each frame: the drag begun when the button goes down
 * (where it was grabbed, the pointer held inside what it may reach), carried on while it stays
 * down -- moved, or with handles resized by its edges, held on the screen -- and the element
 * dropped when it comes up. */
u32 frontend::wata::hudEditorDrag(App* app, ThreadState& ts, u32 element)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 6 * 4 + 0x64);
    const auto chop = [&](Float value) { return truncated(app, ts, value); };
    const auto constant = [&](u32 at) { return Float(app->getMemory<double>(at)); };
    const auto i16 = [&](u32 field) -> s32 { return s32(nfs::s16(guest.word(element + field))); };
    const auto wordField = [&](u32 field) { return Float(nfs::s16(guest.word(element + field))); };
    const auto dwordField = [&](u32 field) { return Float(s32(guest.dword(element + field))); };
    const auto setWordTo = [&](u32 field, std::int32_t value) { guest.setWord(element + field, std::uint16_t(value)); };
    const auto screenWidth = [&] { return guest.dword(kScreenWidth); };
    const auto screenHeight = [&] { return guest.dword(kScreenHeight); };
    const auto integerOf = [&](u32 value) { return Float(s32(value)); };
    const auto mouseX = [&] { return fp::mul(integerOf(guest.dword(kMouseX) * screenWidth()), constant(kPercentToX)); };
    const auto mouseY = [&] { return fp::mul(integerOf(guest.dword(kMouseY) * screenHeight()), constant(kPercentToY)); };
    const auto fit = [&] {
        const Float half = constant(kLastPixel);
        const Float acrossPixels = fp::add(fp::mul(integerOf(u32(i16(kElWidth)) * screenWidth()), constant(kPercentToX)), wordField(kElX));
        guest.setDword(element + kElRight, u32(chop(fp::add(acrossPixels, half))));
        const u32 tall = u32(i16(kElHeight)) * screenHeight();
        const Float percentX = fp::div(fp::mul(wordField(kElX), constant(kXToPercent)), integerOf(screenWidth()));
        const Float downPixels = fp::add(fp::mul(integerOf(tall), constant(kPercentToY)), wordField(kElY));
        const Float bottom = fp::add(half, downPixels);
        guest.setDword(element + kElBottom, u32(chop(bottom)));
        setWordTo(kElPercentX, chop(percentX));
        setWordTo(kElPercentY, chop(fp::div(fp::mul(wordField(kElY), constant(kYToPercent)), integerOf(screenHeight()))));
    };
    const auto percentYNow = [&] {
        setWordTo(kElPercentY, chop(fp::div(fp::mul(wordField(kElY), constant(kYToPercent)), integerOf(screenHeight()))));
    };
    const auto percentXNow = [&] {
        setWordTo(kElPercentX, chop(fp::div(fp::mul(wordField(kElX), constant(kXToPercent)), integerOf(screenWidth()))));
    };

    // The box's size on the screen, what the editing camera's layout adds to its height.
    const Float boxWidth = fp::mul(integerOf(u32(i16(kElWidth)) * screenWidth()), constant(kPercentToX));
    const Float boxHeight = fp::mul(integerOf(u32(i16(kElHeight)) * screenHeight()), constant(kPercentToY));
    const u32 camera = guest.dword(kEditedCamera);
    const u32 widthPixels = u32(chop(boxWidth));
    u32 heightPixels = u32(chop(boxHeight));
    if (camera == 0 || camera == 1)
    {
        const u32 row = guest.dword(kEditedSide) * 368;
        heightPixels += guest.dword(row + 0x7497b4) - guest.dword(row + 0x7497ac);
    }

    if (fe::direct<0x49c080, glue::mouseKey>(app, ts) == 0)
    {
        if (guest.word(element + kElDragging) == 0)
            return element;
        const std::uint8_t flags = guest.byte(element + kElFlags);
        guest.setWord(element + kElDragging, 0);
        guest.setByte(element + kElFlags, flags & std::uint8_t(~kFlagDragged));
        guest::direct<0x459fe0, hudEditorDrop>(app, ts, element, kDragStart);
        // What it leaves in eax the game's code hands back: no result, which no caller reads.
        fe::direct<kSetPointerBounds, controls::boundPointer>(app, ts, u32(0), u32(0), screenWidth(), screenHeight());
        return 0;
    }

    if (guest.word(element + kElDragging) == 0)
    {
        copyDwords(guest, kDragStart, element + kElPercentY, 2);
        guest.setByte(element + kElFlags, guest.byte(element + kElFlags) | kFlagDragged);
        const u32 handle = guest.dword(element + kElHandle);
        if (handle == 0 || handle == 0x108 || handle == 0x208 || handle == 0x409 || handle == 0x50a || handle == 0x60b)
            guest.setDword(element + kElGrabX, u32(chop(fp::sub(mouseX(), wordField(kElX)))));
        else if (handle == 0x809 || handle == 0x90b || handle == 0xa0a)
            guest.setDword(element + kElGrabX, u32(chop(fp::sub(dwordField(kElRight), mouseX()))));
        if (handle == 0 || handle == 0x108 || handle == 0x409 || handle == 0x50a || handle == 0x809 || handle == 0x90b)
            guest.setDword(element + kElGrabY, u32(chop(fp::sub(mouseY(), wordField(kElY)))));
        else if (handle == 0x208 || handle == 0x60b || handle == 0xa0a)
            guest.setDword(element + kElGrabY, u32(chop(fp::sub(dwordField(kElBottom), mouseY()))));
        fit();
        if (gameSettings(guest).playMode == kSplitScreen && guest.dword(element + kElView) != 8)
        {
            const u32 tall = screenHeight();
            const s32 halfTall = s32(tall) / 2;
            const u32 mouseRaw = guest.dword(kMouseY) * tall;
            const Float heightAsFloat = integerOf(tall);
            const Float mouse = fp::mul(integerOf(mouseRaw), constant(kPercentToY));
            const Float below = fp::sub(dwordField(kElBottom), mouse);
            const Float above = fp::sub(mouse, wordField(kElY));
            // Inside the half of the screen the mouse is in.
            if (s32(guest.dword(kMouseY)) < 0xf0)
            {
                const u32 top = u32(chop(above));
                const u32 bottom = u32(chop(fp::sub(Float(halfTall), below)));
                fe::direct<0x4352d0, controls::boundPointer>(app, ts, 0, top, screenWidth(), bottom);
            }
            else
            {
                const Float lowest = fp::sub(heightAsFloat, below);
                const u32 bottom = u32(chop(lowest));
                const Float highest = fp::add(fp::add(above, Float(halfTall)), Float(1.0));
                const u32 top = u32(chop(highest));
                fe::direct<0x4352d0, controls::boundPointer>(app, ts, 0, top, screenWidth(), bottom);
            }
        }
        const u32 view = guest.dword(element + kElView);
        const u32 second = s32(view) >= 0x17 ? 1 : 0;
        guest.setDword(kEditedSide, second);
        guest.setDword(kEditedCamera, second != 0 ? view - 0x17 : guest.dword(element + kElView));
        guest.setWord(element + kElDragging, 1);
    }

    const u32 tall = screenHeight();
    const u32 mouseRawY = guest.dword(kMouseY) * tall;
    if (guest.dword(element + kElHandle) == 0)
    {
        const Float x = mouseX();
        const Float y = fp::mul(integerOf(mouseRawY), constant(kPercentToY));
        setWordTo(kElX, chop(fp::sub(x, dwordField(kElGrabX))));
        setWordTo(kElY, chop(fp::sub(y, dwordField(kElGrabY))));

        u32 room = screenWidth() - widthPixels;
        room += u32(hudDragSlack(app, element, 1)); /* port: what is drawn stays on the screen */
        if (i16(kElX) > s32(room))
            setWordTo(kElX, std::int32_t(guest.word(kScreenWidth) - widthPixels + u32(hudDragSlack(app, element, 1)))); /* port */
        if (nfs::s16(guest.word(element + kElX)) < nfs::s16(-hudDragSlack(app, element, 0))) /* port: was 0 */
            guest.setWord(element + kElX, std::uint16_t(-hudDragSlack(app, element, 0))); /* port: was 0 */
        room = screenHeight() - heightPixels;
        room += u32(hudDragSlack(app, element, 3)); /* port */
        if (i16(kElY) > s32(room))
            setWordTo(kElY, std::int32_t(guest.word(kScreenHeight) - heightPixels + u32(hudDragSlack(app, element, 3)))); /* port */
        if (nfs::s16(guest.word(element + kElY)) < nfs::s16(-hudDragSlack(app, element, 2))) /* port: was 0 */
            guest.setWord(element + kElY, std::uint16_t(-hudDragSlack(app, element, 2))); /* port: was 0 */
        fit();
    }
    else
    {
        const std::uint8_t edges = guest.byte(element + kElEdges);
        const auto oneOver = [&](u32 size) { return fp::div(Float(1.0), integerOf(size)); };
        const auto bottomPixel = [&] { return guest.dword(element + kElBottom); };
        const auto rightPixel = [&] { return guest.dword(element + kElRight); };
        const Float one = Float(1.0);
        if ((edges & 1) != 0)
        {
            // By its top edge.
            const Float y = fp::mul(integerOf(mouseRawY), constant(kPercentToY));
            setWordTo(kElY, chop(fp::sub(y, dwordField(kElGrabY))));
            const Float scaled = fp::mul(integerOf(bottomPixel() - u32(i16(kElY)) + 1), constant(kYToPercent));
            const Float perPixel = oneOver(screenHeight());
            const std::int32_t percent = chop(fp::mul(scaled, perPixel));
            setWordTo(kElHeight, percent);
            if (std::int16_t(percent) < kSmallestSide)
            {
                const Float smallest = fp::mul(integerOf(tall * 26), constant(kPercentToY));
                setWordTo(kElHeight, kSmallestSide);
                setWordTo(kElY, chop(fp::add(fp::sub(dwordField(kElBottom), smallest), one)));
            }
            else if (i16(kElY) < 0)
            {
                const Float wanted = fp::mul(fp::mul(dwordField(kElBottom), constant(kYToPercent)), perPixel);
                const Float whole = fp::add(wanted, one);
                setWordTo(kElY, 0);
                setWordTo(kElHeight, chop(whole));
            }
            percentYNow();
        }
        else if ((edges & 2) != 0)
        {
            // By its bottom edge.
            const Float y = fp::mul(integerOf(mouseRawY), constant(kPercentToY));
            guest.setDword(element + kElBottom, u32(chop(fp::add(y, dwordField(kElGrabY)))));
            const Float scaled = fp::mul(integerOf(bottomPixel() - u32(i16(kElY)) + 1), constant(kYToPercent));
            const Float perPixel = oneOver(screenHeight());
            const std::int32_t percent = chop(fp::mul(scaled, perPixel));
            setWordTo(kElHeight, percent);
            if (std::int16_t(percent) < kSmallestSide)
            {
                const Float smallest = fp::mul(integerOf(tall * 26), constant(kPercentToY));
                setWordTo(kElHeight, kSmallestSide);
                guest.setDword(element + kElBottom, u32(chop(fp::add(fp::add(smallest, wordField(kElY)), constant(kLastPixel)))));
            }
            else if (s32(screenHeight()) <= s32(bottomPixel()))
            {
                guest.setDword(element + kElBottom, screenHeight() - 1);
                const Float scaledRest = fp::mul(integerOf(screenHeight() - u32(i16(kElY))), constant(kYToPercent));
                setWordTo(kElHeight, chop(fp::mul(scaledRest, perPixel)));
            }
        }
        const u32 wide = screenWidth();
        if ((edges & 4) != 0)
        {
            // By its left edge.
            setWordTo(kElX, chop(fp::sub(mouseX(), dwordField(kElGrabX))));
            const Float scaled = fp::mul(integerOf(rightPixel() - u32(i16(kElX)) + 1), constant(kXToPercent));
            const Float perPixel = oneOver(wide);
            const std::int32_t percent = chop(fp::mul(scaled, perPixel));
            setWordTo(kElWidth, percent);
            if (std::int16_t(percent) < kSmallestSide)
            {
                const Float smallest = fp::mul(integerOf(wide * 26), constant(kPercentToX));
                setWordTo(kElWidth, kSmallestSide);
                setWordTo(kElX, chop(fp::add(fp::sub(dwordField(kElRight), smallest), one)));
            }
            else if (i16(kElX) < 0)
            {
                const Float wanted = fp::mul(fp::mul(integerOf(rightPixel() + 1), constant(kXToPercent)), perPixel);
                setWordTo(kElX, 0);
                setWordTo(kElWidth, chop(wanted));
            }
            percentXNow();
        }
        else if ((edges & 8) != 0)
        {
            // By its right edge.
            guest.setDword(element + kElRight, u32(chop(fp::add(mouseX(), dwordField(kElGrabX)))));
            const Float scaled = fp::mul(integerOf(rightPixel() - u32(i16(kElX)) + 1), constant(kXToPercent));
            const Float perPixel = oneOver(wide);
            const std::int32_t percent = chop(fp::mul(scaled, perPixel));
            setWordTo(kElWidth, percent);
            if (std::int16_t(percent) < kSmallestSide)
            {
                const Float smallest = fp::mul(integerOf(wide * 26), constant(kPercentToX));
                setWordTo(kElWidth, kSmallestSide);
                guest.setDword(element + kElRight, u32(chop(fp::add(fp::add(smallest, wordField(kElX)), constant(kLastPixel)))));
            }
            else if (s32(wide) <= s32(rightPixel()))
            {
                guest.setDword(element + kElRight, wide - 1);
                const Float scaledRest = fp::mul(integerOf(wide - u32(i16(kElX))), constant(kXToPercent));
                setWordTo(kElWidth, chop(fp::mul(scaledRest, perPixel)));
            }
        }
    }
    // The game's code hands it the element in edx too, which it does not read.
    const u32 moved = fe::direct<0x459db0, hudz::elementMoved>(app, ts, element);
    guest.setDword(element + kElMoved, moved);
    if (moved == 0)
        return 0;
    copyDwords(guest, kDragStart, element + kElPercentY, 2);
    return element;
}

nfs::u32 frontend::hudEditorDrag(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static Site site{ "sub_45a120", 0, 0, nullptr, 0, fe::kRestoreAll };
    const u32 element = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return wata::hudEditorDrag(app, ts, element); });
}

}
