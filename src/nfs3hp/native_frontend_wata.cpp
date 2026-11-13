#include "native_frontend_wata.h"
#include "native_carstate.h"
#include "native_controls_input.h"
#include "native_ea_files.h"
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
#include "native_frontend_wina.h"
#include "port_snd_stream_calls.h"
#include "port_text.h"
#include "native_frontend.h"
#include "native_windows.h"
#include <nfs3hp.h>
#include <fpu.h>
#include <winapi/advapi32.h>
#include <winapi/kernel32.h>
#include <winapi/user32.h>
struct SDL_AtomicInt;
struct SDL_Semaphore;
#include <lib/thread.h>
#include <cstdint>
#include <cstring>
#include <optional>

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
 * All of the game's state stays in guest memory.  What each function leaves in eax and in
 * the registers it does not save is kept. */
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
using x86::Float;
using x86::reg16;
using x86::reg32;
using x86::reg8;
using x86::sreg32;

// The C library's character classes, indexed by the character plus one: this bit is a digit.
constexpr reg32 kCharacterClass = 0x564ef0;
constexpr std::uint8_t kClassDigit = 0x20;

bool isDigit(const Guest& guest, reg32 classIndex)
{
    return (guest.byte(kCharacterClass + classIndex) & kClassDigit) != 0;
}

void copyDwords(Guest& guest, reg32 to, reg32 from, reg32 count)
{
    for (reg32 i = 0; i < count; ++i)
        guest.setDword(to + 4 * i, guest.dword(from + 4 * i));
}

/* A float moved as `fld dword; fstp dword` moves it. */
void moveFloat(Guest& guest, reg32 to, reg32 from)
{
    guest.setFloat(to, guest.floatAt(from));
}

/* What a wait does each time round, as the port's generated code has it at the head of its loop:
 * the game's lock let go, the thread's turn given up, and a safepoint. */
void giveWayInWait(win32::WinApplication* app, x86::CPU& cpu)
{
    app->unlockContext(cpu);
    win32::Thread::sleep(0);
    app->lockContext(cpu);
    if (app->contextWanted())
        app->yieldContext(cpu);
}

/* ---------------------------------------------------------------------------
 * carp.txt: a car's performance data
 * ------------------------------------------------------------------------- */

constexpr reg32 kAtoi = 0x4f0430, kStrtod = 0x4f82b6, kTolower = 0x4f0ae0, kStrstr = 0x4e0820, kSprintf = 0x4df690;
constexpr reg32 kMemset = 0x4e0640;
constexpr reg32 kReadInteger = 0x4b0700, kReadByte = 0x4b07b0;

constexpr reg32 kCarFileOffset = 0x55ed88;    // how far into the file the reader has come
constexpr reg32 kCarFieldRead = 0x55ed8c;     // a byte a field, 1 when the file gave it
constexpr reg32 kCarDefaults = 0x55f874;      // a record laid out as the car's, the defaults
constexpr reg32 kCarFieldCount = 0x50;

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

constexpr reg32 kFieldReturned = 0;           // handed back through the caller's pointer
constexpr reg32 kFieldMarkedPerValue = 26;    // its code marks it read after every value
constexpr reg32 kFieldDefaultOddOne = 1;      // from elsewhere, not the default record
constexpr reg32 kFieldDefaultWrongPlace = 16; // its default goes to the field before it
constexpr reg32 kFieldFirstTableRow = 72;

/* The `wanted`-th number of a line into `buffer`, as sub_4b0700 and sub_4b07d0 pick it: runs of
 * digits and the like, counted by what separates them, the end of the line one of those. */
template <typename Numeric>
void gatherNumber(Guest& guest, reg32 line, reg32 wanted, reg32 buffer, Numeric isNumeric)
{
    reg32 length = 0;
    std::int16_t separators = 0;
    for (reg32 at = line; std::int32_t(separators) <= sreg32(wanted); ++at)
    {
        const std::uint8_t c = guest.byte(at);
        if (isNumeric(c))
        {
            if (sreg32(wanted) == std::int32_t(separators))
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
reg32 frontend::wata::carDataReadInteger(App* app, CPU& cpu, reg32 line, reg32 wanted)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 5 * 4 + 0x68);
    const reg32 buffer = guest.frame();
    gatherNumber(guest, line, wanted, buffer, [&](std::uint8_t c) {
        return isDigit(guest, std::uint8_t(c + 1)) || c == '-';
    });
    const reg32 value = guest.call(kAtoi, args({ buffer }));
    reg32 edx = guest.edx();
    for (reg32 at = buffer;; ++at)
    {
        const std::uint8_t c = guest.byte(at);
        edx = (edx & 0xffffff00u) | c;
        if (c == 0)
        {
            guest.leaveEdx(edx);
            return value;
        }
        edx = std::uint8_t(c + 1);
        if (!isDigit(guest, edx) && c != '-')
        {
            guest.leaveEdx(edx);
            return 0;
        }
    }
}

void frontend::carDataReadInteger(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4b0700", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 line = cpu.eax, wanted = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::carDataReadInteger(app, cpu, line, wanted); });
}

/* sub_4b07b0: the same, for a field the caller keeps a byte of. */
reg32 frontend::wata::carDataReadByte(App* app, CPU& cpu, reg32 line, reg32 wanted)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    // By address: the edx it leaves goes back to sub_4b1380, which reads it.
    return guest.call(kReadInteger, args({ line, wanted }));
}

void frontend::carDataReadByte(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4b07b0", 0, 0, nullptr, 0, 0 };
    const reg32 line = cpu.eax, wanted = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::carDataReadByte(app, cpu, line, wanted); });
}

/* sub_4b07d0: the wanted float of a line of values on the x87, its bits in eax; 0 if what it
 * found has anything but digits, points and minus signs in it. */
reg32 frontend::wata::carDataReadFloat(App* app, CPU& cpu, reg32 line, reg32 wanted)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 5 * 4 + 0x70);
    const reg32 buffer = guest.frame();
    const reg32 parsed = buffer + 0x68, result = buffer + 0x6c;
    gatherNumber(guest, line, wanted, buffer, [&](std::uint8_t c) {
        return isDigit(guest, std::uint8_t(c + 1)) || c == '.' || c == '-';
    });
    guest.call(kStrtod, args({ buffer, 0 }));
    guest.setFloat(parsed, guest.popFloat());
    reg32 bits = guest.dword(parsed);
    for (reg32 at = buffer;; ++at)
    {
        const std::uint8_t c = guest.byte(at);
        if (c == 0)
            break;
        const reg32 classIndex = std::uint8_t(c + 1);
        guest.leaveEdx(classIndex);
        if (!isDigit(guest, classIndex) && c != '.' && c != '-')
        {
            bits = 0;
            break;
        }
    }
    guest.setDword(result, bits);
    guest.pushFloat(guest.floatAt(result));
    return bits;
}

void frontend::carDataReadFloat(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4b07d0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 line = cpu.eax, wanted = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::carDataReadFloat(app, cpu, line, wanted); });
}

/* sub_4b08b0: a float of the file as 16.16 fixed point. */
reg32 frontend::wata::carDataReadFixed(App* app, CPU& cpu, reg32 line, reg32 wanted)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    constexpr reg32 kToFixed = 0x53eac0;    // a double
    fe::direct<0x4b07d0, carDataReadFloat>(app, cpu, line, wanted);
    const Float scale = Float(app->getMemory<double>(kToFixed));
    return reg32(fp::toInteger<sreg32>(fp::mul(guest.fpu().pop(), scale)));
}

void frontend::carDataReadFixed(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4b08b0", 0, 0, nullptr, 0, 0 };
    const reg32 line = cpu.eax, wanted = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::carDataReadFixed(app, cpu, line, wanted); });
}

/* sub_4b0900: whether a line starts with a digit or a minus sign: values, not a key. */
reg32 frontend::wata::carDataLineHasValues(App* app, CPU& cpu, reg32 line)
{
    Guest guest(app, cpu);
    const std::uint8_t first = guest.byte(line);
    return isDigit(guest, std::uint8_t(first + 1)) || first == '-' ? 1 : 0;
}

void frontend::carDataLineHasValues(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4b0900", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const reg32 line = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::carDataLineHasValues(app, cpu, line); });
}

/* sub_4b0940: which field a line of the file is the key of: its name and its number in brackets
 * both somewhere in the line, by the table at 0x4af3b0.  1 and the field's index through the
 * pointer, or 0. */
reg32 frontend::wata::carDataFindKey(App* app, CPU& cpu, reg32 line, reg32 indexOut)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 5 * 4 + 0x68);
    constexpr reg32 kKeys = 0x4af3b0;           // { char* name; int16 number; } a field, 8 bytes each
    constexpr reg32 kNumberFormat = 0x53ead0;
    constexpr reg32 kEmptyBuffer = 0x4af630;    // what the local buffer starts as
    const reg32 numbered = guest.frame();
    copyDwords(guest, numbered, kEmptyBuffer, 0x19);
    for (reg32 field = 0; field < kCarFieldCount; ++field)
    {
        const reg32 key = kKeys + field * 8;
        guest.leaveEdx(field * 8);
        const reg32 name = guest.dword(key);
        if (name == 0)
            continue;
        const reg32 number = reg32(sreg32(guest.dword(key + 2)) >> 16);
        guest.call(kSprintf, args({}, { numbered, kNumberFormat, number }));
        if (guest.call(kStrstr, args({ line, name })) == 0)
            continue;
        if (guest.call(kStrstr, args({ line, numbered })) != 0)
        {
            guest.setDword(indexOut, field);
            return 1;
        }
    }
    return 0;
}

void frontend::carDataFindKey(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4b0940", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 line = cpu.eax, indexOut = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::carDataFindKey(app, cpu, line, indexOut); });
}

/* sub_4b09d0: the next line of the file, lowered and without its spaces, into `out`; where the
 * file stands is kept at 0x55ed88 and handed back. */
reg32 frontend::wata::carDataNextLine(App* app, CPU& cpu, reg32 text, reg32 out, reg32 length)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4 * 4 + 8);
    const sreg32 last = sreg32(length - 1);
    reg32 position = guest.dword(kCarFileOffset);
    if (text != 0 && sreg32(position) <= last)
    {
        std::uint8_t c = guest.byte(text + position);
        while ((c == 0 || c == '\r' || c == '\n') && sreg32(position) < last)
        {
            ++position;
            c = guest.byte(text + position);
        }
        reg32 written = 0;
        while (c != '\r' && c != '\n' && sreg32(position) < last)
        {
            guest.setDword(kCarFileOffset, position);
            if (c != ' ')
            {
                const reg32 lowered = guest.call(kTolower, args({ c }));
                ++written;
                guest.setByte(out + written - 1, reg8(lowered));
            }
            position = guest.dword(kCarFileOffset) + 1;
            c = guest.byte(text + position);
        }
        guest.setByte(out + written, 0);
    }
    guest.setDword(kCarFileOffset, position);
    guest.leaveEdx(position);
    return position;
}

void frontend::carDataNextLine(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4b09d0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 text = cpu.eax, out = cpu.edx, length = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::carDataNextLine(app, cpu, text, out, length); });
}

/* sub_4b0bb0: the fields the file did not give, from the default record at 0x55f874; or, with
 * edx not 0, the whole default record. */
reg32 frontend::wata::carDataFillDefaults(App* app, CPU& cpu, reg32 record, reg32 whole)
{
    Guest guest(app, cpu);
    constexpr reg32 kRecordBytes = 0x4ac;
    constexpr reg32 kFirstField = 0x55f3c8;
    constexpr reg32 kTableFloats = 0x70;
    if (record == 0)
        return 0;
    if (whole != 0)
    {
        copyDwords(guest, record, kCarDefaults, kRecordBytes / 4);
        return kRecordBytes;
    }
    for (reg32 index = 0; index < kCarFieldCount; ++index)
    {
        if (guest.byte(kCarFieldRead + index) != 0 || index == kFieldReturned)
            continue;
        const CarField& field = kCarFields[index];
        const reg32 from = kCarDefaults + field.offset, to = record + field.offset;
        if (index == kFieldDefaultOddOne)
            guest.setDword(record, guest.dword(kFirstField));
        else if (index == kFieldDefaultWrongPlace)
            guest.setDword(to - 4, guest.dword(from));
        else if (index >= kFieldFirstTableRow)
        {
            const reg32 table = kCarFields[kFieldFirstTableRow].offset;
            for (reg32 i = 0; i < kTableFloats; ++i)
                moveFloat(guest, record + table + 4 * i, kCarDefaults + table + 4 * i);
        }
        else if (field.count == 1 && field.kind != CarValue::Byte)
            guest.setDword(to, guest.dword(from));
        else
            for (reg32 i = 0; i < field.count; ++i)
            {
                if (field.kind == CarValue::Byte)
                    guest.setByte(to + i, guest.byte(from + i));
                else if (field.kind == CarValue::Float)
                    moveFloat(guest, to + 4 * i, from + 4 * i);
                else
                    guest.setDword(to + 4 * i, guest.dword(from + 4 * i));
            }
    }
    guest.leaveEdx(kCarFieldCount);
    return record;
}

void frontend::carDataFillDefaults(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4b0bb0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 record = cpu.eax, whole = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::carDataFillDefaults(app, cpu, record, whole); });
}

/* sub_4b1380: carp.txt into a car's performance record: lines of keys, each followed by a line of
 * values.  The result is how many fields it gave; what it left out comes from the defaults. */
reg32 frontend::wata::carDataRead(App* app, CPU& cpu, reg32 record, reg32 text, reg32 length, reg32 firstOut)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 3 * 4 + 0x7e4);
    constexpr reg32 kEmptyLine = 0x4af694;      // what the line's buffer starts as
    constexpr reg32 kLineDwords = 0x1f4;
    const reg32 line = guest.frame();
    const reg32 fieldAt = line + kLineDwords * 4;
    copyDwords(guest, line, kEmptyLine, kLineDwords);
    guest.setDword(fieldAt, 0);
    guest.leaveEcx(0);
    if (text == 0 || length == 0)
        return 0;
    guest.setDword(kCarFileOffset, 0);
    guest.call(kMemset, args({ kCarFieldRead, 0, kCarFieldCount }));

    const auto readField = [&](reg32 index) {
        const CarField& field = kCarFields[index];
        const reg32 to = record + field.offset;
        for (reg32 i = 0; i < field.count; ++i)
        {
            if (field.kind == CarValue::Integer)
                guest.setDword(to + 4 * i, fe::direct<0x4b0700, carDataReadInteger>(app, cpu, line, i));
            else if (field.kind == CarValue::Fixed)
                guest.setDword(to + 4 * i, fe::direct<0x4b08b0, carDataReadFixed>(app, cpu, line, i));
            else if (field.kind == CarValue::Byte)
                // By address: field 26's mark below reads the edx it leaves.
                guest.setByte(to + i, reg8(guest.call(kReadByte, args({ line, i }))));
            else
            {
                fe::direct<0x4b07d0, carDataReadFloat>(app, cpu, line, i);
                guest.setFloat(to + 4 * i, guest.popFloat());
            }
            if (field.leavesRecordInEdx)
                guest.leaveEdx(record);
            if (index == kFieldMarkedPerValue)
            {
                guest.setByte(kCarFieldRead + guest.dword(fieldAt), 1);
                guest.leaveEdx((guest.edx() & 0xffffff00u) | 1);
            }
        }
    };

    reg32 position = 0;
    bool keyFound = false;
    while (position < length - 1)
    {
        position = fe::direct<0x4b09d0, carDataNextLine>(app, cpu, text, line, length);
        if (guest::direct<0x4b0900, carDataLineHasValues>(app, cpu, line) == 0)
        {
            keyFound = fe::direct<0x4b0940, carDataFindKey>(app, cpu, line, fieldAt) != 0;
            continue;
        }
        const reg32 index = guest.dword(fieldAt);
        if (keyFound && guest.byte(kCarFieldRead + index) == 0 && index < kCarFieldCount)
        {
            if (index == kFieldReturned)
            {
                if (firstOut != 0)
                    guest.setDword(firstOut, fe::direct<0x4b0700, carDataReadInteger>(app, cpu, line, 0));
            }
            else
            {
                readField(index);
            }
            guest.setByte(kCarFieldRead + guest.dword(fieldAt), 1);
        }
        keyFound = false;
    }

    reg32 fieldsRead = 0;
    for (reg32 i = 0; i < kCarFieldCount; ++i)
        fieldsRead += guest.byte(kCarFieldRead + i);
    guest.leaveEbx(kCarFieldCount);
    guest.leaveEcx(fieldsRead);
    if (fieldsRead != kCarFieldCount)
        fe::direct<0x4b0bb0, carDataFillDefaults>(app, cpu, record, 0);
    return fieldsRead;
}

void frontend::carDataRead(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4b1380", 0, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 record = cpu.eax, text = cpu.edx, length = cpu.ebx, firstOut = cpu.ecx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::carDataRead(app, cpu, record, text, length, firstOut); });
}

/* ---------------------------------------------------------------------------
 * Windows' side of things: the exception's name, NPS's DLLs, the install file, the disc
 * ------------------------------------------------------------------------- */

namespace
{

// The import table's slots: Windows' functions, stdcall.
constexpr reg32 kLoadLibrary = 0x53458c, kGetProcAddress = 0x534558, kMessageBox = 0x534764, kGetLastError = 0x534534;
constexpr reg32 kOutputDebugString = 0x53459c;
constexpr reg32 kRegOpenKeyEx = 0x53446c, kRegQueryValueEx = 0x534470, kRegCloseKey = 0x534464;
constexpr reg32 kFindClose = 0x5344c8, kFindFirstFile = 0x5344cc, kFindNextFile = 0x5344d0, kGetFileAttributes = 0x534520;

/* A Windows function, its words as the code pushes them: the port's own, called as C++
 * (native_windows.h); in the checks through its import's slot, as a call of the guest's.  Its
 * answer, eax; the game stopped in it ends the stand-in as Guest::call's do. */
template <reg32 Slot, auto Function, typename... Words>
reg32 callImport(Guest& guest, x86::CPU& cpu, Words... words)
{
    const std::optional<reg32> eax = guest::windows<Slot, Function>(guest.app(), cpu, words...);
    if (!eax)
        throw fe::Terminated();
    return *eax;
}

constexpr reg32 kStrcmp = 0x4ee310, kStrncmp = 0x4eb390, kStrncpy = 0x4e0e30, kStrncat = 0x4eedf0;
constexpr reg32 kFopen = 0x4edfe8, kFgets = 0x4f0570, kFclose = 0x4ee100, kExit = 0x4dfd04;

constexpr reg32 kHklm = 0x80000002, kKeyAllAccess = 0xf003f;
constexpr reg32 kNpsDllKey = 0x53da94, kDllNames = 0x55eca0, kDllPath = 0x7a2020;
constexpr reg32 kDllLoaded = 0x55ec80, kDllHandles = 0x55ec90;
constexpr reg32 kFileAttributeDirectory = 0x10;

/* strcat, as the code has it inline. */
void appendString(Guest& guest, reg32 to, reg32 from)
{
    guest.copyString(to + guest.length(to), from);
}

/* A line of text cut at the first of this character. */
void cutAt(Guest& guest, reg32 line, std::uint8_t cut)
{
    for (reg32 at = line;; ++at)
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
reg32 frontend::wata::exceptionName(App* app, CPU& cpu, reg32 code)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    constexpr reg32 kUnknownFormat = 0x53cd30, kUnknownName = 0x7a1a84;
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
        guest.call(kSprintf, args({}, { kUnknownName, kUnknownFormat, code }));
        return kUnknownName;
    }
}

void frontend::exceptionName(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4a3070", 0, 0, nullptr, 0, 0 };
    const reg32 code = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::exceptionName(app, cpu, code); });
}

/* sub_4a54f0: an NPS DLL loaded, and with edx 0 every function of it looked up into its slot. */
reg32 frontend::wata::npsLoadDll(App* app, CPU& cpu, reg32 path, reg32 type)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 5 * 4);
    struct Procedure
    {
        reg32 name, slot;
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
    const reg32 module = callImport<kLoadLibrary, &win32::kernel32::LoadLibraryA>(guest, cpu, path);
    if (module == 0)
        return 0;
    if (type != 0)
        return 0xffffffffu;
    for (const Procedure& procedure : kProcedures)
        guest.setDword(procedure.slot, callImport<kGetProcAddress, &win32::kernel32::GetProcAddress>(guest, cpu, module, procedure.name));
    return module;
}

void frontend::npsLoadDll(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4a54f0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 path = cpu.eax, type = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::npsLoadDll(app, cpu, path, type); });
}

/* sub_4a5780: whether the registry's NPS key says this is a developer's install: 1 for any of the
 * five words at 0x53dab8. */
reg32 frontend::wata::npsDevelopMode(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4 * 4 + 0x48);
    constexpr reg32 kDevelopValue = 0x53daa8;
    static constexpr reg32 kYes[] = { 0x53dab8, 0x53dabc, 0x53dac0, 0x53dac4, 0x53dac8 };
    const reg32 value = guest.frame(), key = value + 0x40, size = value + 0x44;
    if (callImport<kRegOpenKeyEx, &win32::advapi32::RegOpenKeyExA>(guest, cpu, kHklm, kNpsDllKey, 0, kKeyAllAccess, key) != 0)
        return 0;
    guest.setDword(size, 0x40);
    if (callImport<kRegQueryValueEx, &win32::advapi32::RegQueryValueExA>(guest, cpu, guest.dword(key), kDevelopValue, 0, 0, value, size) != 0)
        return 0;
    callImport<kRegCloseKey, &win32::advapi32::RegCloseKey>(guest, cpu, guest.dword(key));
    for (const reg32 yes : kYes)
        if (guest.call(kStrcmp, args({ value, yes })) == 0)
            return 1;
    return 0;
}

void frontend::npsDevelopMode(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4a5780", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::npsDevelopMode(app, cpu); });
}

/* sub_4a5880: the path of an NPS DLL from the registry, at 0x7a2020, with backslashes; on a
 * developer's install under the name's _DEVELOP value.  1 when it is there. */
reg32 frontend::wata::npsDllPath(App* app, CPU& cpu, reg32 type)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 6 * 4 + 0x108);
    constexpr reg32 kDevelopSuffix = 0x53dacc, kUsingFormat = 0x53dad8;
    const reg32 value = guest.frame(), note = value + 0x80, key = value + 0x100, size = value + 0x104;
    if (callImport<kRegOpenKeyEx, &win32::advapi32::RegOpenKeyExA>(guest, cpu, kHklm, kNpsDllKey, 0, kKeyAllAccess, key) != 0)
        return 0;
    guest.setDword(size, 0x80);
    guest.copyString(value, guest.dword(kDllNames + 4 * type));
    if (guest::direct<0x4a5780, npsDevelopMode>(app, cpu) == 1)
    {
        appendString(guest, value, kDevelopSuffix);
        guest.call(kSprintf, args({}, { note, kUsingFormat, value }));
        callImport<kOutputDebugString, &win32::kernel32::OutputDebugStringA>(guest, cpu, note);
    }
    if (callImport<kRegQueryValueEx, &win32::advapi32::RegQueryValueExA>(guest, cpu, guest.dword(key), value, 0, 0, kDllPath, size) != 0)
    {
        callImport<kRegCloseKey, &win32::advapi32::RegCloseKey>(guest, cpu, guest.dword(key));
        return 0;
    }
    for (reg32 at = kDllPath; guest.byte(at) != 0; ++at)
        if (guest.byte(at) == '/')
            guest.setByte(at, '\\');
    callImport<kRegCloseKey, &win32::advapi32::RegCloseKey>(guest, cpu, guest.dword(key));
    return 1;
}

void frontend::npsDllPath(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4a5880", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 type = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::npsDllPath(app, cpu, type); });
}

/* sub_4a59c0: an NPS DLL of a type loaded once; a message box if it cannot be. */
reg32 frontend::wata::npsEnsureDll(App* app, CPU& cpu, reg32 type)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 6 * 4 + 0x100);
    constexpr reg32 kLoadFailedFormat = 0x53daf0, kLoadFailedTitle = 0x53db30, kNoPathFormat = 0x53db44, kNoPathTitle = 0x53db90;
    const reg32 noPathText = guest.frame(), failedText = noPathText + 0x80;
    if (guest.dword(kDllLoaded + 4 * type) != 0)
        return 0;
    if (guest::direct<0x4a5880, npsDllPath>(app, cpu, type) == 1)
    {
        const reg32 handle = guest::direct<0x4a54f0, npsLoadDll>(app, cpu, kDllPath, type);
        guest.setDword(kDllHandles + 4 * type, handle);
        if (handle != 0)
        {
            guest.setDword(kDllLoaded + 4 * type, guest.dword(kDllLoaded + 4 * type) + 1);
            return 1;
        }
        const reg32 error = callImport<kGetLastError, &win32::kernel32::GetLastError>(guest, cpu);
        guest.call(kSprintf, args({}, { failedText, kLoadFailedFormat, type, guest.dword(kDllNames + 4 * type), kDllPath, error }));
        callImport<kMessageBox, &win32::user32::MessageBoxA>(guest, cpu, 0, failedText, kLoadFailedTitle, 0);
        callImport<kOutputDebugString, &win32::kernel32::OutputDebugStringA>(guest, cpu, failedText);
        return 0;
    }
    guest.call(kSprintf, args({}, { noPathText, kNoPathFormat, type, guest.dword(kDllNames + 4 * type) }));
    callImport<kMessageBox, &win32::user32::MessageBoxA>(guest, cpu, 0, noPathText, kNoPathTitle, 0);
    return 0;
}

void frontend::npsEnsureDll(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4a59c0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 type = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::npsEnsureDll(app, cpu, type); });
}

/* sub_4a6e10: install.win read: its lines into the table at 0x7a2634, and the first of them taken
 * for one of the six languages. */
reg32 frontend::wata::installFileRead(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 6 * 4 + 0x88);
    constexpr reg32 kLines = 0x7a2634, kLinesBytes = 0x1400, kLineLength = 0x7f, kLineStride = 7;
    constexpr reg32 kFileName = 0x53dc40, kReadMode = 0x53dc3c;
    constexpr reg32 kLanguageNames = 0x55fd24, kLanguage = 0x7a3a50, kLanguageCount = 6;
    constexpr reg32 kStreamFlags = 0xc;
    constexpr std::uint8_t kStreamAtEnd = 0x10;
    const reg32 line = guest.frame();
    guest.call(kMemset, args({ kLines, 0, kLinesBytes }));
    const reg32 file = guest.call(kFopen, args({ kFileName, kReadMode }));
    if (file == 0)
        return 0;
    reg32 count = 0;
    while ((guest.byte(file + kStreamFlags) & kStreamAtEnd) == 0)
    {
        guest.call(kFgets, args({ line, kLineLength, file }));
        cutAt(guest, line, '\n');
        cutAt(guest, line, '\r');
        if (guest.byte(line) == 0)
            continue;
        guest.copyString(kLines + (count << kLineStride), line);
        ++count;
    }
    guest.call(kFclose, args({ file }));
    reg32 language = 0;
    for (; language < kLanguageCount; ++language)
    {
        const reg32 name = guest.dword(kLanguageNames + 4 * language);
        if (guest.call(kStrncmp, args({ name, kLines, guest.length(name) })) == 0)
            break;
    }
    if (language == kLanguageCount)
        fe::direct<0x4b5e50, wina::fatalMessage>(app, cpu, 0);
    guest.setDword(kLanguage, language);
    return 1;
}

void frontend::installFileRead(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4a6e10", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::installFileRead(app, cpu); });
}

/* sub_4f0330: findfirst: the first file, not a folder, a pattern finds, its path at 0x9f0ac9. */
reg32 frontend::wata::findFirstFile(App* app, CPU& cpu, reg32 pattern)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 5 * 4 + 0x140);
    constexpr reg32 kHandle = 0x5651d4, kPattern = 0x9f09c4, kPatternFolder = 0x9f09c0, kFound = 0x9f0ac9;
    constexpr reg32 kFoundName = 0x2c, kPathMost = 0x104;
    const reg32 data = guest.frame();
    if (guest.dword(kHandle) != 0xffffffffu)
        callImport<kFindClose, &win32::kernel32::FindClose>(guest, cpu, guest.dword(kHandle));
    guest.call(kStrncpy, args({ kPattern, pattern, kPathMost }));
    sreg32 folder = sreg32(guest.length(kPattern));
    while (folder > 0 && guest.byte(kPattern + folder) != '\\' && guest.byte(kPattern + folder) != ':')
        --folder;
    if (folder != 0)
        ++folder;
    guest.setDword(kPatternFolder, reg32(folder));
    const reg32 handle = callImport<kFindFirstFile, &win32::kernel32::FindFirstFileA>(guest, cpu, kPattern, data);
    guest.setDword(kHandle, handle);
    if (handle == 0xffffffffu)
        return 0;
    const reg32 kept = guest.dword(kPatternFolder);
    guest.setByte(kFound + kept, 0);
    guest.call(kStrncpy, args({ kFound, kPattern, kept }));
    guest.call(kStrncat, args({ kFound, data + kFoundName, kPathMost }));
    const reg32 attributes = callImport<kGetFileAttributes, &win32::kernel32::GetFileAttributesA>(guest, cpu, kFound);
    if (attributes != 0xffffffffu && (attributes & kFileAttributeDirectory) == 0)
        return kFound;
    return guest::direct<0x4f0280, findNextFile>(app, cpu);
}

void frontend::findFirstFile(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f0330", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 pattern = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::findFirstFile(app, cpu, pattern); });
}

/* sub_4f0280: findnext: the next file, not a folder, or 0 and the search closed. */
reg32 frontend::wata::findNextFile(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 5 * 4 + 0x140);
    constexpr reg32 kHandle = 0x5651d4, kPattern = 0x9f09c4, kPatternFolder = 0x9f09c0, kFound = 0x9f0ac9;
    constexpr reg32 kFoundName = 0x2c, kPathMost = 0x104;
    const reg32 data = guest.frame();
    if (guest.dword(kHandle) == 0xffffffffu)
        return 0;
    for (;;)
    {
        if (callImport<kFindNextFile, &win32::kernel32::FindNextFileA>(guest, cpu, guest.dword(kHandle), data) == 0)
        {
            callImport<kFindClose, &win32::kernel32::FindClose>(guest, cpu, guest.dword(kHandle));
            guest.setDword(kHandle, 0xffffffffu);
            return 0;
        }
        const reg32 kept = guest.dword(kPatternFolder);
        guest.setByte(kFound + kept, 0);
        guest.call(kStrncpy, args({ kFound, kPattern, kept }));
        guest.call(kStrncat, args({ kFound, data + kFoundName, kPathMost }));
        const reg32 attributes = callImport<kGetFileAttributes, &win32::kernel32::GetFileAttributesA>(guest, cpu, kFound);
        if (attributes != 0xffffffffu && (attributes & kFileAttributeDirectory) == 0)
            return kFound;
    }
}

void frontend::findNextFile(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f0280", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::findNextFile(app, cpu); });
}

/* sub_4a46f0: the colour of a car's badge in the showcase, by the words in its name. */
reg32 frontend::wata::carBadgeColour(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4 * 4);
    constexpr reg32 kCarNameTexts = 0x554fa8;
    constexpr reg32 kOtherColour = 0xff4060a0;
    struct Badge
    {
        reg32 word, colour;
    };
    static constexpr Badge kBadges[] = {
        { 0x53cf40, 0xfffbb829 }, { 0x53cf48, 0xff64ffff }, { 0x53cf50, 0xff0000ff },
        { 0x53cf5c, 0xff0000ff }, { 0x53cf68, 0xffc02028 }, { 0x53cf70, 0xffbbeeff },
    };
    const std::int16_t id = std::int16_t(guest.word(kCarNameTexts + 2 * gameSettings(guest).players[0].car));
    // The game's code hands it edx as it has it.
        const reg32 name = reg32(*fe::direct<0x4d1850, port::text::lineText>(app, cpu, cpu.esp - 4, std::int32_t(id), cpu.edx));
    for (const Badge& badge : kBadges)
        if (guest.call(kStrstr, args({ name, badge.word })) != 0)
            return badge.colour;
    return kOtherColour;
}

void frontend::carBadgeColour(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4a46f0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::carBadgeColour(app, cpu); });
}

/* ---------------------------------------------------------------------------
 * The window's description checked (win\validwin.c)
 * ------------------------------------------------------------------------- */

namespace
{
constexpr reg32 kErrorFile = 0x552190, kErrorFunction = 0x552194, kErrorLine = 0x552198;
constexpr reg32 kError = 0x401010;
}

/* sub_4f4920: a field of a window's description within its limits; an error if not and the
 * check is on.  Stack: the most, and whether the field is one the game must have. */
reg32 frontend::wata::windowCheckField(App* app, CPU& cpu, reg32 window, reg32 value, reg32 name, reg32 least, reg32 most,
                                       reg32 wanted)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 3 * 4);
    constexpr reg32 kChecking = 0x56440c, kFile = 0x54cf04, kFunction = 0x54cf14;
    if (sreg32(value) >= sreg32(least) && sreg32(value) <= sreg32(most))
        return 1;
    if (guest.dword(kChecking) == 0)
        return 0;
    guest.setDword(kErrorFile, kFile);
    guest.setDword(kErrorFunction, kFunction);
    const reg32 at4 = guest.dword(window + 4), at8 = guest.dword(window + 8);
    const reg32 kindByte = guest.byte(window + 0x1c);
    if (least == most)
    {
        guest.setDword(kErrorLine, wanted != 0 ? 0x14 : 0x16);
        guest.call(kError, args({}, { wanted != 0 ? 0x54cf28u : 0x54cf7cu, name, value, least, at4, at8, kindByte, window }));
    }
    else
    {
        guest.setDword(kErrorLine, wanted != 0 ? 0x1b : 0x1d);
        guest.call(kError, args({}, { wanted != 0 ? 0x54cfd0u : 0x54d028u, name, value, least, most, at4, at8, kindByte, window }));
    }
    return 0;
}

void frontend::windowCheckField(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f4920", 8, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 window = cpu.eax, value = cpu.edx, name = cpu.ebx, least = cpu.ecx;
    const reg32 most = app->getMemory<reg32>(cpu.esp + 4);
    const reg32 wanted = app->getMemory<reg32>(cpu.esp + 8);
    fe::runWhole(app, cpu, site,
                 [&](Guest&) { return wata::windowCheckField(app, cpu, window, value, name, least, most, wanted); });
}

/* sub_4f4a80: a window's description checked, field by field: 1 when all are in their limits. */
reg32 frontend::wata::windowCheck(App* app, CPU& cpu, reg32 window)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 5 * 4);
    constexpr reg32 kMagic = 0x57494e44;    // 'WIND'
    constexpr reg32 kLargest = 0x4000;
    const auto check = [&](reg32 value, reg32 name, reg32 least, reg32 most, reg32 wanted) {
        return guest::direct<0x4f4920, windowCheckField>(app, cpu, window, value, name, least, most, wanted);
    };
    reg32 all = check(guest.dword(window), 0x54d080, kMagic, kMagic, 1);
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

void frontend::windowCheck(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f4a80", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 window = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::windowCheck(app, cpu, window); });
}

/* sub_4f9390: a directory joined to a file's name in a buffer of the size given: 0 if it will
 * not fit. */
reg32 frontend::wata::pathInDirectory(App* app, CPU& cpu, reg32 buffer, reg32 fileName, reg32 size)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 3 * 4);
    // In edx and ebx as they came across sub_4e33a0, and read again after it, which keeps them.
    cpu.edx = fileName;
    cpu.ebx = size;
    fe::direct<0x4e33a0, ea::currentDirectory>(app, cpu, buffer);
    const reg32 room = cpu.ebx - guest.length(fileName) - 1;
    guest.leaveEbx(room);
    if (guest.length(buffer) > room)
        return 0;
    appendString(guest, buffer, guest.edx());
    return 1;
}

void frontend::pathInDirectory(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f9390", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 buffer = cpu.eax, fileName = cpu.edx, size = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::pathInDirectory(app, cpu, buffer, fileName, size); });
}

/* sub_4f0b60: a string run through the text's own measure (sub_4efb50) with its output turned
 * aside: the box it covers, through four pointers. */
reg32 frontend::wata::textMeasureBox(App* app, CPU& cpu, reg32 text, sreg32 given, reg32 leftOut, reg32 topOut,
                                     reg32 widthOut, reg32 heightOut)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 3 * 4 + 0x110);
    cpu.ecx = topOut;   // read where the code reads ecx, its callees keeping it
    constexpr reg32 kPenX = 0x5671f8, kPenY = 0x5671fc;
    constexpr reg32 kBoxLeft = 0x9f0bdc, kBoxTop = 0x9f0bd8, kBoxRight = 0x9f0bd0, kBoxBottom = 0x9f0bd4;
    constexpr reg32 kPutGlyph = 0x567288, kPutGlyph2 = 0x56728c, kPutGlyph3 = 0x567290;
    constexpr reg32 kMeasureGlyph = 0x4f0af0;
    constexpr reg32 kLongest = 0x100;
    const reg32 copy = guest.frame();
    const reg32 penX = guest.dword(kPenX), penY = guest.dword(kPenY);
    guest::direct<0x4f0850, textSetPen>(app, cpu, 0, 0);
    guest.setDword(kBoxLeft, 0);
    guest.setDword(kBoxTop, 0);
    guest.setDword(kBoxRight, 0);
    guest.setDword(kBoxBottom, 0);
    const reg32 length = given > sreg32(kLongest) ? kLongest : reg32(given);
    guest.setByte(copy + length, 0);
    guest.call(kStrncpy, args({ copy, text, length }));
    const reg32 put1 = guest.dword(kPutGlyph), put2 = guest.dword(kPutGlyph2), put3 = guest.dword(kPutGlyph3);
    guest.setDword(kPutGlyph, kMeasureGlyph);
    guest.setDword(kPutGlyph2, 0);
    guest.setDword(kPutGlyph3, 0);
    if (guest.byte(copy) != 0)
        fe::direct<0x4efb50, eab::textDrawAtPen>(app, cpu, copy);
    guest.setDword(leftOut, guest.dword(kBoxLeft));
    guest.setDword(cpu.ecx, guest.dword(kBoxTop));
    const sreg32 width = sreg32(guest.dword(kBoxRight) - guest.dword(kBoxLeft));
    const sreg32 least = sreg32(guest.dword(kPenX));
    guest.setDword(widthOut, reg32(width < least ? least : width));
    guest.setDword(heightOut, guest.dword(kBoxBottom) - guest.dword(kBoxTop));
    guest.leaveEcx(widthOut);
    guest::direct<0x4f0850, textSetPen>(app, cpu, penX, penY);
    guest.leaveEdx(penY);   // as the call by address left it, on to the end
    guest.setDword(kPutGlyph2, put2);
    guest.setDword(kPutGlyph3, put3);
    guest.setDword(kPutGlyph, put1);
    guest.leaveEbx(put3);
    return put1;
}

void frontend::textMeasureBox(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f0b60", 8, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 text = cpu.eax, leftOut = cpu.ebx, topOut = cpu.ecx;
    const sreg32 given = sreg32(cpu.edx);
    const reg32 widthOut = app->getMemory<reg32>(cpu.esp + 4);
    const reg32 heightOut = app->getMemory<reg32>(cpu.esp + 8);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        return wata::textMeasureBox(app, cpu, text, given, leftOut, topOut, widthOut, heightOut);
    });
}

/* sub_4fb710: a shift key's lock light set to what is asked, by pressing it: 0 caps, 1 num, 2 scroll. */
reg32 frontend::wata::lockKeySet(App* app, CPU& cpu, reg32 lock, reg32 wanted)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 5 * 4 + 4);
    constexpr reg32 kGetKeyState = 0x534744, kKeybdEvent = 0x5347ac;
    constexpr reg32 kExtendedDown = 1, kUp = 2, kExtendedUp = 3;
    std::uint8_t scan = 0, key = 0;
    reg32 down = 0, up = kUp;
    if (lock == 0)
    {
        scan = 0x3a;
        key = 0x14;
    }
    else if (lock == 1)
    {
        scan = 0x45;
        key = 0x90;
        down = kExtendedDown;
        up = kExtendedUp;
    }
    else if (lock == 2)
    {
        scan = 0x46;
        key = 0x91;
    }
    else
    {
        return lock;
    }
    const reg32 state = callImport<kGetKeyState, &win32::user32::GetKeyState>(guest, cpu, key);
    const reg32 lit = state & 1;
    guest.leaveEdx(wanted);
    if (lit == wanted)
        return lit;
    callImport<kKeybdEvent, &win32::user32::keybd_event>(guest, cpu, key, scan, down, 0);
    return callImport<kKeybdEvent, &win32::user32::keybd_event>(guest, cpu, key, scan, up, 0);
}

void frontend::lockKeySet(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4fb710", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 lock = cpu.eax, wanted = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::lockKeySet(app, cpu, lock, wanted); });
}

/* ---------------------------------------------------------------------------
 * The command line
 * ------------------------------------------------------------------------- */

/* sub_4a5080: the game's command line, last word first: switches into the flags at 0x55eb2c and
 * the settings from 0x7a1fa0 on; a help or a word it does not know, and a box lists them.  0 then,
 * and when a server was asked for without what it takes; 1 otherwise. */
reg32 frontend::wata::commandLineRead(App* app, CPU& cpu, reg32 count, reg32 words)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 5 * 4 + 0x168);
    constexpr reg32 kFillZero = 0x4e070c, kStricmp = 0x4e9ec0, kStrupr = 0x4f8020, kSscanf = 0x4f0548;
    constexpr reg32 kSettings = 0x7a1fa0, kSettingsBytes = 0x7c;
    constexpr reg32 kHelpWords = 0x55ec3c, kHelpWordCount = 5;
    constexpr reg32 kFlags = 0x55eb2c, kFlagNames = 0x55eb34, kFlagCount = 0x21, kFlagHelps = 0x55ebb8;
    constexpr reg32 kNumberedPrefix = 0x53d684, kNumberFormat = 0x53d68c, kNumbered = 0x55fdbc;
    constexpr std::uint8_t kNumberedFlag = 0x20;
    constexpr reg32 kNameWords = 0x55ec50, kNameWordCount = 0xc;
    constexpr reg32 kServerName = 0x7a1fa0, kServerNameMost = 0x3c, kPort = 0x7a1fdc, kCode = 0x7a1fe0, kCodeMost = 0xb;
    constexpr reg32 kOtherNumbers = 0x7a1fec, kNeeded = 0x7a1ff8;
    constexpr reg32 kUsage = 0x7a1ba0;
    const reg32 spaces = guest.frame(), word = spaces + 0x100, number = spaces + 0x150;
    bool help = false, understood = false;
    guest.call(kFillZero, args({ kSettings, kSettingsBytes }));
    reg32 at = count;
    for (;;)
    {
        --at;
        if (at == 0)
            break;
        const reg32 argument = guest.dword(words + 4 * at);
        understood = false;
        for (reg32 i = 0; i < kHelpWordCount; ++i)
            if (guest.call(kStricmp, args({ argument, guest.dword(kHelpWords + 4 * i) })) == 0)
                help = true;
        if (!help)
            for (reg32 i = 0; i < kFlagCount; ++i)
            {
                if (guest.call(kStricmp, args({ argument, guest.dword(kFlagNames + 4 * i) })) != 0)
                    continue;
                const reg32 flags = kFlags + 4 * (i / 32);
                guest.setDword(flags, guest.dword(flags) | (reg32(1) << ((sreg32(i + 1) % 32 - 1) & 31)));
                understood = true;
                break;
            }
        guest.copyString(word, argument);
        guest.leaveEdx(words);
        guest.call(kStrupr, args({ word }));
        if (!understood && guest.call(kStrncmp, args({ word, kNumberedPrefix, 4 })) == 0
            && guest.call(kSscanf, args({}, { word + 4, kNumberFormat, number })) != 0)
        {
            const reg32 value = guest.dword(number);
            const reg32 flags = kFlags + 4 * reg32(sreg32(value) / 32);
            guest.setByte(flags, guest.byte(flags) | kNumberedFlag);
            understood = true;
            guest.setDword(kNumbered, value);
        }
        if (!understood)
            for (reg32 i = 0; i < kNameWordCount; ++i)
            {
                const reg32 name = guest.dword(kNameWords + 4 * i);
                const reg32 length = guest.length(name);
                if (guest.call(kStrncmp, args({ word, name, length })) != 0)
                    continue;
                const reg32 rest = word + length;
                if (i == 0)
                {
                    const reg32 restLength = guest.length(rest);
                    if (restLength != 0 && restLength < kServerNameMost)
                    {
                        guest.copyString(kServerName, rest);
                        understood = true;
                    }
                }
                else if (i == 1)
                {
                    if (guest.call(kSscanf, args({}, { rest, kNumberFormat, kPort })) != 0)
                        understood = true;
                }
                else if (i == 2)
                {
                    understood = true;
                    guest.call(kStrncpy, args({ kCode, argument + length, kCodeMost }));
                }
                else if (guest.call(kSscanf, args({}, { rest, kNumberFormat, kOtherNumbers + 4 * i })) != 0)
                {
                    understood = true;
                }
            }
        if (understood && !help)
            continue;

        guest.call(kSprintf, args({}, { kUsage, 0x53d6ac, 0x53d6a8, 0x53d69c, 0x53d690 }));
        if (!help)
            guest.call(kSprintf, args({}, { kUsage + guest.length(kUsage), 0x53d70c, argument }));
        guest.call(kSprintf, args({}, { kUsage + guest.length(kUsage), 0x53d724 }));
        for (reg32 i = 0; i < kFlagCount; ++i)
        {
            const reg32 name = guest.dword(kFlagNames + 4 * i);
            if (guest.byte(name) == 0)
                continue;
            const reg32 pad = 0xe - guest.length(name);
            for (reg32 c = 0; c < pad; ++c)
                guest.setByte(spaces + c, ' ');
            guest.setByte(spaces + pad, 0);
            guest.call(kSprintf, args({}, { kUsage + guest.length(kUsage), 0x53d728, name, spaces, guest.dword(kFlagHelps + 4 * i) }));
        }
        callImport<kMessageBox, &win32::user32::MessageBoxA>(guest, cpu, 0, kUsage, understood ? 0x53d748u : 0x53d734u, 0);
        return 0;
    }
    if (guest.dword(kPort) != 0
        && (guest.length(kServerName) == 0 || guest.length(kCode) == 0 || guest.dword(kNeeded) == 0))
    {
        callImport<kMessageBox, &win32::user32::MessageBoxA>(guest, cpu, 0, 0x53d764, 0x53d750, 0);
        return 0;
    }
    return 1;
}

void frontend::commandLineRead(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4a5080", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 count = cpu.eax, words = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::commandLineRead(app, cpu, count, words); });
}

/* sub_4b6320: the game's disc looked for, a box and the end if it is run from one or there is none. */
reg32 frontend::wata::discCheck(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 6 * 4 + 0x13c);
    constexpr reg32 kInstallName = 0x53fe30, kTitle = 0x53fe3c, kLanguage = 0x7a3a50;
    constexpr reg32 kPathPattern = 0x4b5688, kRunFromDiscTexts = 0x4b5694, kNoDiscTexts = 0x4b56ac;
    constexpr reg32 kBoxWarning = 0x30, kDrives = 0x1a;
    const reg32 ownPath = guest.frame(), noDisc = ownPath + 0x100, runFromDisc = ownPath + 0x118, pattern = ownPath + 0x130;
    const auto refuse = [&](reg32 texts, reg32 local) {
        copyDwords(guest, local, texts, 6);
        const reg32 text = guest.dword(local + 4 * guest.dword(kLanguage));
        callImport<kMessageBox, &win32::user32::MessageBoxA>(guest, cpu, 0, text, kTitle, kBoxWarning);
        guest.call(kExit, args({ 0 }));
    };
    copyDwords(guest, pattern, kPathPattern, 2);
    guest.setWord(pattern + 8, guest.word(kPathPattern + 8));
    guest.setByte(pattern + 10, guest.byte(kPathPattern + 10));
    guest::direct<0x4f9390, pathInDirectory>(app, cpu, ownPath, kInstallName, 0x100);
    const reg32 ownDrive = guest::direct<0x4f93f0, driveNumberOfPath>(app, cpu, ownPath);
    const reg32 ownIsCd = guest::direct<0x4f9410, driveIsCd>(app, cpu, ownDrive);
    if (ownIsCd != 0)
        refuse(kRunFromDiscTexts, runFromDisc);
    // The game's code hands it what came back before in eax, which it does not read.
    if (const reg32 remote = fe::direct<0x4b62a0, loopa::installIsRemote>(app, cpu); remote != 0)
        return remote;
    for (reg32 drive = 0; drive < kDrives; ++drive)
    {
        if (guest::direct<0x4f9410, driveIsCd>(app, cpu, drive) == 0)
            continue;
        guest.setByte(pattern, std::uint8_t('A' + drive));
        if (const reg32 found = guest::direct<0x4f0330, findFirstFile>(app, cpu, pattern); found != 0)
            return found;
    }
    refuse(kNoDiscTexts, noDisc);
    return guest.eax();
}

void frontend::discCheck(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4b6320", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::discCheck(app, cpu); });
}

/* sub_4f9410: whether a drive, 0 for A:, is a CD. */
reg32 frontend::wata::driveIsCd(App* app, CPU& cpu, reg32 drive)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4 * 4 + 4);
    constexpr reg32 kRootA = 0x54d930, kGetDriveType = 0x534518, kDriveCdrom = 5;
    const reg32 root = guest.frame();
    guest.copyString(root, kRootA);
    guest.setByte(root, std::uint8_t(guest.byte(root) + drive));
    return callImport<kGetDriveType, &win32::kernel32::GetDriveTypeA>(guest, cpu, root) == kDriveCdrom ? 1 : 0;
}

void frontend::driveIsCd(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f9410", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 drive = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::driveIsCd(app, cpu, drive); });
}

/* sub_4f93f0: a path's drive, 0 for A:, or -1. */
reg32 frontend::wata::driveNumberOfPath(App* app, CPU& cpu, reg32 path)
{
    Guest guest(app, cpu);
    constexpr reg32 kToupper = 0x4eede0;
    if (guest.byte(path + 1) != ':')
        return 0xffffffffu;
    return guest.call(kToupper, args({ guest.byte(path) })) - 'A';
}

void frontend::driveNumberOfPath(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f93f0", 0, 0, nullptr, 0, 0 };
    const reg32 path = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::driveNumberOfPath(app, cpu, path); });
}

/* ---------------------------------------------------------------------------
 * EA's asynchronous file reader (win\async.c): a thread, its record at 0x9f0be0, queues of
 * requests in the record -- free, done, done with a callback, waiting
 * ------------------------------------------------------------------------- */

namespace
{

constexpr reg32 kAsync = 0x9f0be0, kAsyncWake = 0x9f0be4;
constexpr reg32 kAsyncEvent = 0, kAsyncClosing = 4, kAsyncFile = 8, kAsyncFileStart = 0xc, kAsyncFileSize = 0x10;
constexpr reg32 kAsyncSharedBuffer = 0x14, kAsyncCurrent = 0x18;
constexpr reg32 kAsyncFree = 0x1c, kAsyncDone = 0x38, kAsyncDoneCallback = 0x54, kAsyncWaiting = 0x70, kAsyncRequests = 0x8c;
constexpr reg32 kRequestBytes = 0x28;
constexpr reg32 kRequestId = 4, kRequestBuffer = 8, kRequestOffset = 0xc, kRequestSize = 0x10, kRequestPool = 0x14;
constexpr reg32 kRequestOwned = 0x18, kRequestName = 0x1c, kRequestKind = 0x20, kRequestCallback = 0x24;
constexpr reg32 kNextRequestId = 0x565284, kCurrentPool = 0x5643f4;
constexpr reg32 kAsyncSource = 0x54c680;
constexpr reg32 kBlockBytes = 0x10e;    // of a stream, what the sound streams want topped up by

constexpr reg32 kQueueTake = 0x50df60, kQueueLength = 0x50e1d0, kQueueFree = 0x50dde0;
constexpr reg32 kFillZeroes = 0x4e070c, kCloseEvent = 0x4dfc00, kCreateThread = 0x4df7a0;
constexpr reg32 kLockDrive = 0x4e40a0, kUnlockDrive = 0x4e40f0;
constexpr reg32 kBlockSize = 0x4ff820, kTopUp = 0x4ff670, kWaitTopUp = 0x4ff7f0;
constexpr reg32 kAsyncTask = 0x4f0d20, kAsyncThread = 0x4f1820;
constexpr reg32 kRequestHasId = 0x4f0cb0;

/* What every assert of the library's leaves for the error's text. */
void reportError(Guest& guest, reg32 file, reg32 function, reg32 line, reg32 message)
{
    guest.setDword(kErrorFile, file);
    guest.setDword(kErrorFunction, function);
    guest.setDword(kErrorLine, line);
    guest.call(kError, args({}, { message }));
}

reg32 asyncRecord(const Guest& guest)
{
    return guest.dword(kAsync);
}

/* The requests of a queue thrown away: their name and buffer freed, back on the free queue. */
void asyncDrain(Guest& guest, x86::CPU& cpu, reg32 queue)
{
    win32::WinApplication* const app = guest.app();
    for (reg32 request = fe::direct<0x50df60, frontend::netb::qpop>(app, cpu, asyncRecord(guest) + queue); request != 0;
         request = fe::direct<0x50df60, frontend::netb::qpop>(app, cpu, asyncRecord(guest) + queue))
    {
        const reg32 name = guest.dword(request + kRequestName);
        if (name != 0)
            fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, name);
        const reg32 buffer = guest.dword(request + kRequestOwned);
        if (buffer != 0)
            fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, buffer);
        fe::direct<0x50de50, frontend::netb::qpush>(app, cpu, asyncRecord(guest) + kAsyncFree, request);
    }
}

/* The reader's thread woken: its event set (the game's code hands it the event's slot in eax too,
 * which it does not read). */
void asyncWake(Guest& guest, x86::CPU& cpu)
{
    const reg32 event = guest.dword(asyncRecord(guest) + kAsyncEvent);
    guest.leaveEdx(event);   // where the game's code hands it, which keeps it
    fe::direct<0x4dfb20, ea::eventPosted>(guest.app(), cpu, event);
}

}

/* sub_4f0cb0: a queue's search: whether a request has this id. */
reg32 frontend::wata::asyncRequestHasId(App* app, CPU& cpu, reg32 request, reg32 id)
{
    Guest guest(app, cpu);
    return id == guest.dword(request + kRequestId) ? 1 : 0;
}

void frontend::asyncRequestHasId(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f0cb0", 0, 0, nullptr, 0, 0 };
    const reg32 request = cpu.eax, id = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::asyncRequestHasId(app, cpu, request, id); });
}

/* sub_4f0cc0: nothing found. */
reg32 frontend::wata::asyncNoRequest(App* app, CPU& cpu)
{
    return 0;
}

void frontend::asyncNoRequest(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f0cc0", 0, 0, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::asyncNoRequest(app, cpu); });
}

/* sub_4f0cd0: what the thread is at told to stop, and waited for. */
reg32 frontend::wata::asyncCancelCurrent(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 2 * 4);
    if (asyncRecord(guest) == 0)
        return guest.eax();
    const reg32 request = guest.dword(asyncRecord(guest) + kAsyncCurrent);
    if (request != 0)
        guest.setDword(request + kRequestKind, 0);
    asyncWake(guest, cpu);
    for (;;)
    {
        const reg32 record = asyncRecord(guest);
        if (guest.dword(record + kAsyncEvent) == 0 || request == 0 || request != guest.dword(record + kAsyncCurrent))
            return record;
        fe::direct<0x4df8e0, ea::sleepFor>(app, cpu, 0);
    }
}

void frontend::asyncCancelCurrent(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f0cd0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::asyncCancelCurrent(app, cpu); });
}

/* sub_4f0d20: the system task: finished requests' callbacks, in the game's own thread. */
reg32 frontend::wata::asyncRunCallbacks(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    if (guest.call(kQueueLength, args({ asyncRecord(guest) + kAsyncDoneCallback })) == 0)
        return 0;
    for (reg32 request = fe::direct<0x50dff0, netb::qpoplast>(app, cpu, asyncRecord(guest) + kAsyncDoneCallback); request != 0;
         request = fe::direct<0x50dff0, netb::qpoplast>(app, cpu, asyncRecord(guest) + kAsyncDoneCallback))
    {
        const reg32 owned = guest.dword(request + kRequestOwned);
        const bool kept = owned != 0 && owned != guest.dword(asyncRecord(guest) + kAsyncSharedBuffer);
        fe::direct<0x50de50, netb::qpush>(app, cpu, asyncRecord(guest) + (kept ? kAsyncDone : kAsyncFree), request);
        guest.call(guest.dword(request + kRequestCallback), args({ guest.dword(request + kRequestId) }));
    }
    return 0;
}

void frontend::asyncRunCallbacks(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f0d20", 0, 0, nullptr, 0, fe::kRestoreEcx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::asyncRunCallbacks(app, cpu); });
}

/* sub_4f0da0: the reader started, with room for twice this many requests: its record, its queues,
 * its task and its thread, waited for. */
reg32 frontend::wata::asyncInit(App* app, CPU& cpu, reg32 requests)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4 * 4);
    constexpr reg32 kFunction = 0x54c68c, kTag = 0x54c6c8;
    if (fe::direct<0x4f9070, ea::surfaceIsLocked>(app, cpu))
        reportError(guest, kAsyncSource, kFunction, 0xc4, 0x54c698);
    if (asyncRecord(guest) != 0)
        guest::direct<0x4f0f40, asyncShutdown>(app, cpu);
    guest.setDword(kErrorFile, kAsyncSource);
    guest.setDword(kErrorLine, 0xc8);
    guest.setDword(kErrorFunction, kFunction);
    const reg32 bytes = requests * kRequestBytes + kAsyncRequests;
    // The class in ebx as the game's code has it.
    const reg32 record = *fe::direct<0x4e1620, ea::reserved>(app, cpu, kTag, bytes, cpu.ebx);
    guest.setDword(kAsync, record);
    guest.call(kFillZeroes, args({ record, bytes }));
    guest.setDword(asyncRecord(guest) + kAsyncSharedBuffer, asyncRecord(guest));
    for (const reg32 queue : { kAsyncFree, kAsyncDone, kAsyncDoneCallback, kAsyncWaiting })
        fe::direct<0x50dcd0, netb::qinit>(app, cpu, asyncRecord(guest) + queue, 0, 0);
    reg32 request = asyncRecord(guest) + kAsyncRequests;
    for (reg32 i = requests; i != 0; --i, request += kRequestBytes)
        fe::direct<0x50de50, netb::qpush>(app, cpu, asyncRecord(guest) + kAsyncFree, request);
    // The game's code hands it 0xffffffff in ecx too, which it does not read.
    fe::direct<0x4e7520, port::stream::scheduleTask>(app, cpu, cpu.esp - 4, kAsyncTask, 4, 3);
    if (guest.call(kCreateThread, args({ kAsyncThread, 0, 1 }, { kAsyncWake })) == 0)
    {
        guest.leaveEbx(kAsyncSource);
        reportError(guest, kAsyncSource, kFunction, 0xde, 0x54c6d4);
    }
    for (;;)
    {
        const reg32 current = asyncRecord(guest);
        const reg32 event = guest.dword(current + kAsyncEvent);
        if (event != 0)
            return current;
        fe::direct<0x4df8e0, ea::sleepFor>(app, cpu, event);
    }
}

void frontend::asyncInit(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f0da0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 requests = cpu.eax * 2;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::asyncInit(app, cpu, requests); });
}

/* sub_4f0f40: the reader stopped and everything it holds freed. */
reg32 frontend::wata::asyncShutdown(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 5 * 4);
    constexpr reg32 kFunction = 0x54c704;
    // What the calls left in eax: whether a surface is locked, or the error's.
    reg32 left = 0;
    if (fe::direct<0x4f9070, ea::surfaceIsLocked>(app, cpu))
    {
        reportError(guest, kAsyncSource, kFunction, 0x108, 0x54c710);
        left = guest.eax();
    }
    if (asyncRecord(guest) == 0)
        return left;
    guest.setDword(asyncRecord(guest) + kAsyncClosing, 1);
    guest::direct<0x4f0cd0, asyncCancelCurrent>(app, cpu);
    while (guest.dword(asyncRecord(guest) + kAsyncEvent) != 0)
        fe::direct<0x4df8e0, ea::sleepFor>(app, cpu, 0);
    asyncDrain(guest, cpu, kAsyncWaiting);
    asyncDrain(guest, cpu, kAsyncDone);
    asyncDrain(guest, cpu, kAsyncDoneCallback);
    fe::direct<0x4e75e0, port::stream::unscheduleTask>(app, cpu, kAsyncTask);
    guest.call(kQueueFree, args({ asyncRecord(guest) + kAsyncFree }));
    guest.call(kQueueFree, args({ asyncRecord(guest) + kAsyncDone }));
    guest.call(kQueueFree, args({ asyncRecord(guest) + kAsyncWaiting }));
    guest.call(kQueueFree, args({ asyncRecord(guest) + kAsyncDoneCallback }));
    const reg32 file = guest.dword(asyncRecord(guest) + kAsyncFile);
    if (file != 0)
        fe::direct<0x4e2da0, ea::closeChecked>(app, cpu, file);
    const reg32 result = *fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, asyncRecord(guest));
    guest.setDword(kAsync, 0);
    return result;
}

void frontend::asyncShutdown(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f0f40", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::asyncShutdown(app, cpu); });
}

/* sub_4f1370: the file the reader reads from next, by name; 0 for none.  A request of kind 2. */
reg32 frontend::wata::asyncSetFile(App* app, CPU& cpu, reg32 name)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4 * 4 + 4);
    constexpr reg32 kFunction = 0x54c7a4, kTag = 0x54c780;
    constexpr reg32 kKindSetFile = 2;
    if (asyncRecord(guest) == 0)
        reportError(guest, kAsyncSource, kFunction, 0x203, 0x54c7b4);
    // By address: the edx it leaves is read after it, and left at the end.
    const reg32 request = guest.call(kQueueTake, args({ asyncRecord(guest) + kAsyncFree }));
    if (request == 0)
        return 0;
    const reg32 edx = guest.edx();
    guest.setDword(request + kRequestBuffer, 0);
    guest.setDword(request + kRequestOffset, 0);
    guest.setDword(request + kRequestSize, 0);
    guest.setDword(request + kRequestKind, kKindSetFile);
    guest.setDword(request + kRequestCallback, 0);
    const reg32 id = guest.dword(kNextRequestId);
    guest.setDword(request + kRequestOwned, 0);
    guest.setDword(request + kRequestId, id);
    guest.setDword(request + kRequestPool, guest.dword(kCurrentPool));
    guest.setDword(kNextRequestId, id + 1);
    if (name != 0)
    {
        guest.setDword(kErrorFile, kAsyncSource);
        guest.setDword(kErrorFunction, kFunction);
        guest.setDword(kErrorLine, 0x214);
        const reg32 edi = cpu.edi;
        const reg32 copy = *fe::direct<0x4e1620, ea::reserved>(app, cpu, kTag, guest.length(name) + 1, guest.dword(kCurrentPool));
        guest.setDword(request + kRequestName, copy);
        guest.copyString(copy, name);
        cpu.edi = edi;
    }
    else
    {
        guest.setDword(request + kRequestName, 0);
    }
    fe::direct<0x50de50, netb::qpush>(app, cpu, asyncRecord(guest) + kAsyncWaiting, request);
    // The game's code hands it the event's slot in eax too, which it does not read.
    const reg32 posted = *fe::direct<0x4dfb20, ea::eventPosted>(app, cpu, guest.dword(asyncRecord(guest) + kAsyncEvent));
    guest.leaveEdx(edx);
    return posted;
}

void frontend::asyncSetFile(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f1370", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi };
    const reg32 name = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::asyncSetFile(app, cpu, name); });
}

/* sub_4f1500: a read of the open file asked for, with a callback or none: its id, 0 with no free
 * request.  A request of kind 3. */
reg32 frontend::wata::asyncReadAt(App* app, CPU& cpu, reg32 offset, reg32 buffer, reg32 size, reg32 callback)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 3 * 4 + 4);
    cpu.edx = buffer;   // read after the queue's call, which keeps it
    constexpr reg32 kFunction = 0x54c820, kKindRead = 3;
    guest.leaveEbx(callback);
    if (asyncRecord(guest) == 0)
        reportError(guest, kAsyncSource, kFunction, 0x27d, 0x54c83c);
    const reg32 request = fe::direct<0x50df60, netb::qpop>(app, cpu, asyncRecord(guest) + kAsyncFree);
    guest.leaveEcx(request);
    if (request == 0)
        return 0;
    const reg32 id = guest.dword(kNextRequestId);
    guest.setDword(request + kRequestId, id);
    const reg32 record = asyncRecord(guest);
    guest.setDword(kNextRequestId, id + 1);
    const reg32 start = guest.dword(record + kAsyncFileStart);
    guest.setDword(request + kRequestKind, kKindRead);
    guest.setDword(request + kRequestOwned, 0);
    guest.setDword(request + kRequestName, 0);
    guest.setDword(request + kRequestSize, size);
    guest.setDword(request + kRequestBuffer, buffer);
    guest.setDword(request + kRequestOffset, start + offset);
    guest.setDword(request + kRequestPool, guest.dword(kCurrentPool));
    guest.setDword(request + kRequestCallback, callback);
    fe::direct<0x50de50, netb::qpush>(app, cpu, record + kAsyncWaiting, request);
    asyncWake(guest, cpu);
    return guest.dword(request + kRequestId);
}

void frontend::asyncReadAt(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f1500", 0, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 offset = cpu.eax, buffer = cpu.edx, size = cpu.ebx, callback = cpu.ecx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::asyncReadAt(app, cpu, offset, buffer, size, callback); });
}

/* sub_4f15d0: a read with no callback. */
reg32 frontend::wata::asyncRead(App* app, CPU& cpu, reg32 offset, reg32 buffer, reg32 size)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    // On to sub_4f1500 as they came, the callback none.
    return fe::direct<0x4f1500, asyncReadAt>(app, cpu, offset, buffer, size, 0);
}

void frontend::asyncRead(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f15d0", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const reg32 offset = cpu.eax, buffer = cpu.edx, size = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::asyncRead(app, cpu, offset, buffer, size); });
}

/* sub_4f1790: a read's end: 1 done, -1 failed, 0 not yet. */
reg32 frontend::wata::asyncReadStatus(App* app, CPU& cpu, reg32 id)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 3 * 4);
    constexpr reg32 kFunction = 0x54c8ec;
    if (asyncRecord(guest) == 0)
        reportError(guest, kAsyncSource, kFunction, 0x377, 0x54c900);
    const reg32 request = fe::direct<0x50e5a0, watb::queueFetch>(app, cpu, asyncRecord(guest) + kAsyncDone, kRequestHasId, id);
    if (request == 0)
        return 0;
    fe::direct<0x50de50, netb::qpush>(app, cpu, asyncRecord(guest) + kAsyncFree, request);
    return guest.dword(request + kRequestKind) != 0 ? 1 : 0xffffffffu;
}

void frontend::asyncReadStatus(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f1790", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 id = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::asyncReadStatus(app, cpu, id); });
}

/* sub_4f1820: the reader's thread: a request at a time from the waiting queue, until it is told
 * to close. */
reg32 frontend::wata::asyncThreadMain(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 6 * 4 + 0x108);
    constexpr reg32 kFunction = 0x54c930;
    constexpr reg32 kKindLoad = 1, kKindSetFile = 2, kKindRead = 3, kKindClose = 4;
    const reg32 path = guest.frame(), handle = path + 0x100;
    const reg32 event = *fe::direct<0x4dfb00, ea::newAutoEvent>(app, cpu);
    {
        const reg32 record = asyncRecord(guest);
        guest.setDword(record + kAsyncEvent, event);
        guest.setDword(record + kAsyncCurrent, 0);
        guest.setDword(record + kAsyncClosing, 0);
    }
    for (;;)
    {
        if (guest.dword(asyncRecord(guest) + kAsyncClosing) != 0)
        {
            const reg32 file = guest.dword(asyncRecord(guest) + kAsyncFile);
            if (file != 0)
                fe::direct<0x4e2da0, ea::closeChecked>(app, cpu, file);
            guest.setDword(asyncRecord(guest) + kAsyncFile, 0);
            guest.setDword(asyncRecord(guest) + kAsyncCurrent, 0);
            guest.call(kCloseEvent, args({ guest.dword(asyncRecord(guest) + kAsyncEvent) }));
            const reg32 record = asyncRecord(guest);
            guest.setDword(record + kAsyncEvent, 0);
            return record;
        }
        const reg32 request = fe::direct<0x50dff0, netb::qpoplast>(app, cpu, asyncRecord(guest) + kAsyncWaiting);
        if (request == 0)
        {
            fe::direct<0x4dfb70, ea::eventWaited>(app, cpu, guest.dword(asyncRecord(guest) + kAsyncEvent));
            continue;
        }
        guest.setDword(asyncRecord(guest) + kAsyncCurrent, request);
        const auto finished = [&] {
            const reg32 queue = guest.dword(request + kRequestCallback) != 0 ? kAsyncDoneCallback : kAsyncDone;
            fe::direct<0x50de50, netb::qpush>(app, cpu, asyncRecord(guest) + queue, request);
        };
        const reg32 kind = guest.dword(request + kRequestKind);
        if (kind == kKindLoad)
        {
            const reg32 name = guest.dword(request + kRequestName);
            guest.copyString(path, name);
            fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, name);
            guest.setDword(request + kRequestName, 0);
            fe::direct<0x4e28d0, ea::openToRead>(app, cpu, path, handle, request + kRequestOffset, request + kRequestSize);
            guest.setDword(request + kRequestOwned, 0);
            const reg32 file = guest.dword(handle);
            if (file != 0)
            {
                const reg32 blocks = reg32(sreg32(guest.dword(request + kRequestSize)) / sreg32(kBlockBytes));
                const reg32 wanted = blocks + guest.call(kBlockSize, args({ 0, 0 }));
                if (guest.dword(request + kRequestBuffer) == 0)
                {
                    guest.setDword(kErrorFile, kAsyncSource);
                    guest.setDword(kErrorFunction, kFunction);
                    guest.setDword(kErrorLine, 0x3a7);
                    const reg32 memory = *fe::direct<0x4e1620, ea::reserved>(app, cpu, path, guest.dword(request + kRequestSize), guest.dword(request + kRequestPool));
                    guest.setDword(request + kRequestOwned, memory);
                    guest.setDword(request + kRequestBuffer, memory);
                }
                else
                {
                    guest.setDword(request + kRequestOwned, guest.dword(asyncRecord(guest) + kAsyncSharedBuffer));
                }
                guest.call(kTopUp, args({ wanted }));
                guest.call(kWaitTopUp);
                guest.call(kLockDrive, args({ guest.dword(handle), request }));
                // The game's code hands it the request in ebx too, which it does not read.
                fe::direct<0x4e2e60, ea::seekChecked>(app, cpu, guest.dword(handle), guest.dword(request + kRequestOffset));
                if (*fe::direct<0x4e2de0, ea::readChecked>(app, cpu, guest.dword(handle), guest.dword(request + kRequestBuffer), guest.dword(request + kRequestSize)) == 0)
                    guest.setDword(request + kRequestKind, 0);
                guest.call(kUnlockDrive, args({ guest.dword(handle) }));
                fe::direct<0x4e2da0, ea::closeChecked>(app, cpu, guest.dword(handle));
                guest.setDword(handle, 0);
                if (guest.dword(request + kRequestKind) == 0
                    && guest.dword(request + kRequestOwned) != guest.dword(asyncRecord(guest) + kAsyncSharedBuffer))
                {
                    fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, guest.dword(request + kRequestOwned));
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
            const reg32 file = guest.dword(asyncRecord(guest) + kAsyncFile);
            if (file != 0)
                fe::direct<0x4e2da0, ea::closeChecked>(app, cpu, file);
            guest.setDword(asyncRecord(guest) + kAsyncFile, 0);
            if (guest.dword(request + kRequestName) != 0)
            {
                guest.call(kTopUp, args({ guest.call(kBlockSize, args({ 0, 0 })) }));
                guest.call(kWaitTopUp);
                const reg32 record = asyncRecord(guest);
                fe::direct<0x4e28d0, ea::openToRead>(app, cpu, guest.dword(request + kRequestName), record + kAsyncFile, record + kAsyncFileStart, record + kAsyncFileSize);
                fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, guest.dword(request + kRequestName));
                guest.setDword(request + kRequestName, 0);
            }
            fe::direct<0x50de50, netb::qpush>(app, cpu, asyncRecord(guest) + kAsyncFree, request);
        }
        else if (kind == kKindRead)
        {
            if (guest.dword(asyncRecord(guest) + kAsyncFile) == 0)
            {
                guest.setDword(request + kRequestKind, 0);
            }
            else
            {
                const reg32 blocks = reg32(sreg32(guest.dword(request + kRequestSize)) / sreg32(kBlockBytes));
                guest.call(kTopUp, args({ guest.call(kBlockSize, args({ 0, 0 })) + blocks }));
                guest.call(kWaitTopUp);
                guest.call(kLockDrive, args({ guest.dword(asyncRecord(guest) + kAsyncFile) }));
                fe::direct<0x4e2e60, ea::seekChecked>(app, cpu, guest.dword(asyncRecord(guest) + kAsyncFile), guest.dword(request + kRequestOffset));
                if (*fe::direct<0x4e2de0, ea::readChecked>(app, cpu, guest.dword(asyncRecord(guest) + kAsyncFile), guest.dword(request + kRequestBuffer), guest.dword(request + kRequestSize)) == 0)
                    guest.setDword(request + kRequestKind, 0);
                guest.call(kUnlockDrive, args({ guest.dword(asyncRecord(guest) + kAsyncFile) }));
            }
            finished();
        }
        else if (kind == kKindClose)
        {
            const reg32 file = guest.dword(asyncRecord(guest) + kAsyncFile);
            if (file != 0)
                fe::direct<0x4e2da0, ea::closeChecked>(app, cpu, file);
            const reg32 record = asyncRecord(guest);
            guest.setDword(record + kAsyncFile, 0);
            fe::direct<0x50de50, netb::qpush>(app, cpu, record + kAsyncFree, request);
        }
        else
        {
            reportError(guest, kAsyncSource, kFunction, 0x3fb, 0x54c940);
        }
        guest.setDword(asyncRecord(guest) + kAsyncCurrent, 0);
        fe::direct<0x4df8e0, ea::sleepFor>(app, cpu, 1);
    }
}

void frontend::asyncThreadMain(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f1820", 0, 0, nullptr, 0, fe::kRestoreAll };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::asyncThreadMain(app, cpu); });
}

/* ---------------------------------------------------------------------------
 * The small ones the libraries share
 * ------------------------------------------------------------------------- */

namespace
{

// A function that ends by jumping to another: what it calls runs where a jump would run it.
constexpr reg32 kTailCall = reg32(0) - 4;

constexpr reg32 kTicks = 0x79c1a4;

// The mixers: B (sub_50d560) has a typed core, called as C++; the others by address.
constexpr reg32 kMixA = 0x50356c, kMixB = 0x50d560, kMixC = 0x503280, kMixD = 0x5033e4;

/* A tail call with ebx turned from a count of one size into another's; the other registers on to
 * the mixer as they came. */
template <typename Scale>
reg32 tailScaled(win32::WinApplication* app, x86::CPU& cpu, reg32 count, reg32 target, Scale scale)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, kTailCall);
    guest.leaveEbx(scale(count));
    if (target == kMixB)
        return fe::direct<0x50d560, frontend::watb::copyBlock>(app, cpu, cpu.eax, cpu.edx, cpu.ebx);
    return guest.call(target);
}

reg32 half(reg32 count) { return reg32(sreg32(count) >> 1); }
reg32 twice(reg32 count) { return count + count; }
reg32 fourTimes(reg32 count) { return count << 2; }

}

/* sub_4f0630: the timer's ticks. */
reg32 frontend::wata::timerNow(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    return guest.dword(kTicks);
}

void frontend::timerNow(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f0630", 0, 0, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::timerNow(app, cpu); });
}

/* sub_4f0650: the ticks since the last time it was asked. */
reg32 frontend::wata::timerSinceLast(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    constexpr reg32 kLast = 0x5651dc;
    const reg32 last = guest.dword(kLast);
    const reg32 now = guest::direct<0x4f0630, timerNow>(app, cpu);
    guest.setDword(kLast, now);
    return now - last;
}

void frontend::timerSinceLast(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f0650", 0, 0, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::timerSinceLast(app, cpu); });
}

/* sub_4f06e0: ticks waited for, the system's tasks run meanwhile. */
reg32 frontend::wata::timerWait(App* app, CPU& cpu, reg32 ticks)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    const reg32 until = ticks + guest::direct<0x4f0630, timerNow>(app, cpu);
    reg32 left = guest::direct<0x4f0630, timerNow>(app, cpu) - until;
    while (sreg32(left) < 0)
    {
        giveWayInWait(app, cpu);
        fe::direct<0x4e7630, port::stream::runDueTasks>(app, cpu, cpu.esp - 4, 0);
        left = guest::direct<0x4f0630, timerNow>(app, cpu) - until;
    }
    return left;
}

void frontend::timerWait(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f06e0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const reg32 ticks = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::timerWait(app, cpu, ticks); });
}

/* sub_4f0840: the screen's colour depth, as the text keeps it. */
reg32 frontend::wata::textDepth(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    return guest.dword(0x567218);
}

void frontend::textDepth(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f0840", 0, 0, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::textDepth(app, cpu); });
}

/* sub_4f0850: the text's pen. */
reg32 frontend::wata::textSetPen(App* app, CPU& cpu, reg32 x, reg32 y)
{
    Guest guest(app, cpu);
    guest.setDword(0x5671fc, y);
    guest.setDword(0x5671f8, x);
    return x;
}

void frontend::textSetPen(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f0850", 0, 0, nullptr, 0, 0 };
    const reg32 x = cpu.eax, y = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::textSetPen(app, cpu, x, y); });
}

/* sub_4f0860: the text's attribute set, and on to what follows it. */
reg32 frontend::wata::textSetAttribute(App* app, CPU& cpu, reg32 value)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, kTailCall);
    cpu.eax = value;   // on to sub_50c0d0 as it came
    guest.setDword(0x56720c, value);
    return fe::direct<0x50c0d0, watb::initTextDraw>(app, cpu);
}

void frontend::textSetAttribute(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f0860", 0, 0, nullptr, 0, 0 };
    const reg32 value = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::textSetAttribute(app, cpu, value); });
}

/* The mixers' front doors, a table for each format at 0x5651e0: a count of samples turned into one
 * of bytes (halved for 4 bits, doubled for 16, three times for 24, four times for 32). */
reg32 frontend::wata::mixA4Bit(App* app, CPU& cpu, reg32 count)
{
    return tailScaled(app, cpu, count, kMixA, half);
}

void frontend::mixA4Bit(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f0880", 0, 0, nullptr, 0, 0 };
    const reg32 count = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::mixA4Bit(app, cpu, count); });
}

reg32 frontend::wata::mixA16Bit(App* app, CPU& cpu, reg32 count)
{
    return tailScaled(app, cpu, count, kMixA, twice);
}

void frontend::mixA16Bit(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f0890", 0, 0, nullptr, 0, 0 };
    const reg32 count = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::mixA16Bit(app, cpu, count); });
}

reg32 frontend::wata::mixA32Bit(App* app, CPU& cpu, reg32 count)
{
    return tailScaled(app, cpu, count, kMixA, fourTimes);
}

void frontend::mixA32Bit(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f08c0", 0, 0, nullptr, 0, 0 };
    const reg32 count = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::mixA32Bit(app, cpu, count); });
}

reg32 frontend::wata::mixB4Bit(App* app, CPU& cpu, reg32 count)
{
    return tailScaled(app, cpu, count, kMixB, half);
}

void frontend::mixB4Bit(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f08d0", 0, 0, nullptr, 0, 0 };
    const reg32 count = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::mixB4Bit(app, cpu, count); });
}

reg32 frontend::wata::mixB16Bit(App* app, CPU& cpu, reg32 count)
{
    return tailScaled(app, cpu, count, kMixB, twice);
}

void frontend::mixB16Bit(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f08e0", 0, 0, nullptr, 0, 0 };
    const reg32 count = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::mixB16Bit(app, cpu, count); });
}

reg32 frontend::wata::mixB32Bit(App* app, CPU& cpu, reg32 count)
{
    return tailScaled(app, cpu, count, kMixB, fourTimes);
}

void frontend::mixB32Bit(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f0910", 0, 0, nullptr, 0, 0 };
    const reg32 count = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::mixB32Bit(app, cpu, count); });
}

reg32 frontend::wata::mixC4Bit(App* app, CPU& cpu, reg32 count)
{
    return tailScaled(app, cpu, count, kMixC, half);
}

void frontend::mixC4Bit(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f0920", 0, 0, nullptr, 0, 0 };
    const reg32 count = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::mixC4Bit(app, cpu, count); });
}

reg32 frontend::wata::mixC16Bit(App* app, CPU& cpu, reg32 count)
{
    return tailScaled(app, cpu, count, kMixC, twice);
}

void frontend::mixC16Bit(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f0930", 0, 0, nullptr, 0, 0 };
    const reg32 count = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::mixC16Bit(app, cpu, count); });
}

reg32 frontend::wata::mixC32Bit(App* app, CPU& cpu, reg32 count)
{
    return tailScaled(app, cpu, count, kMixC, fourTimes);
}

void frontend::mixC32Bit(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f0960", 0, 0, nullptr, 0, 0 };
    const reg32 count = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::mixC32Bit(app, cpu, count); });
}

reg32 frontend::wata::mixD4Bit(App* app, CPU& cpu, reg32 count)
{
    return tailScaled(app, cpu, count, kMixD, half);
}

void frontend::mixD4Bit(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f0970", 0, 0, nullptr, 0, 0 };
    const reg32 count = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::mixD4Bit(app, cpu, count); });
}

reg32 frontend::wata::mixD16Bit(App* app, CPU& cpu, reg32 count)
{
    return tailScaled(app, cpu, count, kMixD, twice);
}

void frontend::mixD16Bit(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f0980", 0, 0, nullptr, 0, 0 };
    const reg32 count = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::mixD16Bit(app, cpu, count); });
}

reg32 frontend::wata::mixD32Bit(App* app, CPU& cpu, reg32 count)
{
    return tailScaled(app, cpu, count, kMixD, fourTimes);
}

void frontend::mixD32Bit(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f09b0", 0, 0, nullptr, 0, 0 };
    const reg32 count = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::mixD32Bit(app, cpu, count); });
}

namespace
{
/* Three times the count, for 24 bits; the samples in eax, and in ecx too. */
reg32 mixTriple(win32::WinApplication* app, x86::CPU& cpu, reg32 samples, reg32 count, reg32 target)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    cpu.eax = samples;
    guest.leaveEcx(samples);
    guest.leaveEbx(count * 3);
    if (target == kMixB)
        return fe::direct<0x50d560, frontend::watb::copyBlock>(app, cpu, samples, cpu.edx, count * 3);
    return guest.call(target);
}
}

reg32 frontend::wata::mixA24Bit(App* app, CPU& cpu, reg32 samples, reg32 count)
{
    return mixTriple(app, cpu, samples, count, kMixA);
}

void frontend::mixA24Bit(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f08a0", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const reg32 samples = cpu.eax, count = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::mixA24Bit(app, cpu, samples, count); });
}

reg32 frontend::wata::mixB24Bit(App* app, CPU& cpu, reg32 samples, reg32 count)
{
    return mixTriple(app, cpu, samples, count, kMixB);
}

void frontend::mixB24Bit(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f08f0", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const reg32 samples = cpu.eax, count = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::mixB24Bit(app, cpu, samples, count); });
}

reg32 frontend::wata::mixC24Bit(App* app, CPU& cpu, reg32 samples, reg32 count)
{
    return mixTriple(app, cpu, samples, count, kMixC);
}

void frontend::mixC24Bit(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f0940", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const reg32 samples = cpu.eax, count = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::mixC24Bit(app, cpu, samples, count); });
}

reg32 frontend::wata::mixD24Bit(App* app, CPU& cpu, reg32 samples, reg32 count)
{
    return mixTriple(app, cpu, samples, count, kMixD);
}

void frontend::mixD24Bit(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f0990", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const reg32 samples = cpu.eax, count = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::mixD24Bit(app, cpu, samples, count); });
}

/* sub_4f09c0: the blitter that suits the processor, chosen the first time, called. */
reg32 frontend::wata::blitDispatch(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    constexpr reg32 kChoice = 0x565270, kHasMmx = 0x5643b4, kTable = 0x565274;
    if (sreg32(guest.dword(kChoice)) < 0)
        guest.setDword(kChoice, guest.dword(kHasMmx) != 0 ? 3 : 1);
    guest.leaveEcx(guest.dword(kTable + 4 * guest.dword(kChoice)));
    // The shape and its place on to it as they came; the MMX blitter's emms is nothing here: the x87
    // is not shared.
    return fe::direct<0x50b0b0, watb::moveShape>(app, cpu, cpu.eax, cpu.edx, cpu.ebx, cpu.ecx);
}

void frontend::blitDispatch(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f09c0", 0, 0, nullptr, 0, fe::kRestoreEcx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::blitDispatch(app, cpu); });
}

/* sub_4f0a10: the same, with the two 12-bit fields of a shape's position word in ebx and edx. */
reg32 frontend::wata::blitShapeAtPosition(App* app, CPU& cpu, reg32 shape)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 2 * 4);
    cpu.eax = shape;   // on to the blitter as it came
    const reg32 position = guest.dword(shape + 0xc);
    guest.leaveEbx(reg32(sreg32(position << 4) >> 20));
    guest.leaveEdx(reg32(sreg32(position << 20) >> 20));
    return fe::direct<0x4f09c0, blitDispatch>(app, cpu);
}

void frontend::blitShapeAtPosition(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f0a10", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEdx };
    const reg32 shape = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::blitShapeAtPosition(app, cpu, shape); });
}

/* sub_4f0a90: a surface filled with a colour and drawn. */
reg32 frontend::wata::surfaceFillAndDraw(App* app, CPU& cpu, reg32 x, reg32 y, reg32 width, reg32 height, reg32 colour)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 2 * 4);
    constexpr reg32 kFillWord = 0x4e0721;
    const reg32 surface = fe::direct<0x4ef190, eab::windowBitmapCreateDefault>(app, cpu, width, height, 0x20, 0);
    guest.leaveEcx(surface);
    const reg32 words = guest.dword(surface + 8) * guest.dword(surface + 0x28);
    guest.call(kFillWord, args({ guest.dword(surface + 0x20) + 0x10, words, colour }));
    fe::direct<0x50db30, watb::drawAlphaShape>(app, cpu, guest.dword(cpu.ecx + 0x20), x, y);
    return fe::direct<0x4eff00, eab::windowBitmapFree>(app, cpu, cpu.ecx);
}

void frontend::surfaceFillAndDraw(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f0a90", 4, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 x = cpu.eax, y = cpu.edx, width = cpu.ebx, height = cpu.ecx;
    const reg32 colour = app->getMemory<reg32>(cpu.esp + 4);
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::surfaceFillAndDraw(app, cpu, x, y, width, height, colour); });
}

/* sub_4f0af0: the text's measure, in place of its drawing: the box grown by a glyph.  Its arguments
 * are a glyph drawer's, four on the stack. */
reg32 frontend::wata::textMeasureGlyph(App* app, CPU& cpu, sreg32 x, sreg32 y, sreg32 width, sreg32 height)
{
    Guest guest(app, cpu);
    constexpr reg32 kLeft = 0x9f0bdc, kTop = 0x9f0bd8, kRight = 0x9f0bd0, kBottom = 0x9f0bd4;
    const sreg32 left = sreg32(guest.dword(kLeft));
    guest.setDword(kLeft, reg32(x <= left ? x : left));
    const sreg32 top = sreg32(guest.dword(kTop));
    const reg32 newTop = reg32(y <= top ? y : top);
    guest.setDword(kTop, newTop);
    const sreg32 right = sreg32(guest.dword(kRight));
    const sreg32 reach = sreg32(reg32(x) + reg32(width));
    guest.setDword(kRight, reg32(reach < right ? right : reach));
    const sreg32 bottom = sreg32(guest.dword(kBottom));
    const sreg32 foot = sreg32(reg32(y) + reg32(height));
    const reg32 newBottom = reg32(foot < bottom ? bottom : foot);
    guest.setDword(kBottom, newBottom);
    guest.leaveEbx(reg32(top));
    guest.leaveEcx(newTop);
    guest.leaveEdx(newBottom);
    return reg32(bottom);
}

void frontend::textMeasureGlyph(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f0af0", 0x10, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const sreg32 x = sreg32(cpu.eax), y = sreg32(cpu.edx);
    const sreg32 width = sreg32(app->getMemory<reg32>(cpu.esp + 8));
    const sreg32 height = sreg32(app->getMemory<reg32>(cpu.esp + 12));
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::textMeasureGlyph(app, cpu, x, y, width, height); });
}

/* sub_4f0c90: sub_4f0b60 with room for the whole string. */
reg32 frontend::wata::textMeasure(App* app, CPU& cpu, reg32 text, reg32 leftOut, reg32 topOut, reg32 widthOut, reg32 heightOut)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    return fe::direct<0x4f0b60, textMeasureBox>(app, cpu, text, sreg32(0x100), leftOut, topOut, widthOut, heightOut);
}

void frontend::textMeasure(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f0c90", 4, 0, nullptr, 0, fe::kRestoreEsi };
    const reg32 text = cpu.eax, leftOut = cpu.edx, topOut = cpu.ebx, widthOut = cpu.ecx;
    const reg32 heightOut = app->getMemory<reg32>(cpu.esp + 4);
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::textMeasure(app, cpu, text, leftOut, topOut, widthOut, heightOut); });
}

/* sub_4f2170: how many times round a loop in so many ticks: the processor's speed. */
reg32 frontend::wata::processorCountLoops(App* app, CPU& cpu, sreg32 ticks)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 6 * 4 + 8);
    const reg32 start = guest.dword(kTicks);
    reg32 loops = 0;
    do
    {
        giveWayInWait(app, cpu);
        volatile sreg32 dividend = 0x5555aaaa;
        (void)(dividend / 0x21);
        ++loops;
    } while (sreg32(guest.dword(kTicks) - start) < ticks);
    return loops;
}

void frontend::processorCountLoops(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f2170", 0, 0, nullptr, 0, fe::kRestoreAll };
    const sreg32 ticks = sreg32(cpu.eax);
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::processorCountLoops(app, cpu, ticks); });
}

/* sub_4f4650: a string and a value into the next row of the table at 0x79f524. */
reg32 frontend::wata::registryAddEntry(App* app, CPU& cpu, reg32 name, reg32 value)
{
    Guest guest(app, cpu);
    constexpr reg32 kCount = 0x5662e0, kNames = 0x79f524, kValues = 0x79f52c, kRowBytes = 12;
    guest.copyString(kNames + guest.dword(kCount) * kRowBytes, name);
    const reg32 row = guest.dword(kCount);
    guest.setDword(kValues + row * kRowBytes, value);
    guest.setDword(kCount, row + 1);
    guest.leaveEdx(row + 1);
    return row * 3;
}

void frontend::registryAddEntry(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f4650", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 name = cpu.eax, value = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::registryAddEntry(app, cpu, name, value); });
}

/* sub_4f46b0: the current screen's surface's pixels. */
reg32 frontend::wata::screenPixels(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    return guest.dword(guest.dword(0x55e558) * 8 + guest.dword(0x79f5c4)) + 0x10;
}

void frontend::screenPixels(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f46b0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::screenPixels(app, cpu); });
}

namespace
{
constexpr reg32 kWindowFirst = 0x564ff4, kWindowSecond = 0x56502c, kWindowDwords = 0xe;
}

/* sub_4f46d0: the window's two descriptions, kept. */
reg32 frontend::wata::windowSave(App* app, CPU& cpu, reg32 to)
{
    Guest guest(app, cpu);
    copyDwords(guest, to, kWindowFirst, kWindowDwords);
    copyDwords(guest, to + kWindowDwords * 4, kWindowSecond, kWindowDwords);
    return to;
}

void frontend::windowSave(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f46d0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 to = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::windowSave(app, cpu, to); });
}

/* sub_4f4700: the window's descriptions put back as sub_4f46d0 kept them: the check off, for it. */
reg32 frontend::wata::windowRestore(App* app, CPU& cpu, reg32 from)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4 * 4);
    constexpr reg32 kChecking = 0x56440c;
    const reg32 checking = guest.dword(kChecking);
    guest.setDword(kChecking, 0);
    const reg32 result = fe::direct<0x4eada0, eaa::bitmapDrawInto>(app, cpu, from);   // as it came
    guest.setDword(kChecking, checking);
    copyDwords(guest, kWindowSecond, from + kWindowDwords * 4, kWindowDwords);
    return result;
}

void frontend::windowRestore(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f4700", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 from = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::windowRestore(app, cpu, from); });
}

namespace
{
constexpr reg32 kTextState = 0x5671f4, kTextStateDwords = 0x2e;
}

/* sub_4f4740: the text's whole state, kept. */
reg32 frontend::wata::textStateSave(App* app, CPU& cpu, reg32 to)
{
    Guest guest(app, cpu);
    copyDwords(guest, to, kTextState, kTextStateDwords);
    return to;
}

void frontend::textStateSave(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f4740", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 to = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::textStateSave(app, cpu, to); });
}

/* sub_4f4760: and put back. */
reg32 frontend::wata::textStateRestore(App* app, CPU& cpu, reg32 from)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 3 * 4);
    copyDwords(guest, kTextState, from, kTextStateDwords);
    cpu.eax = from;   // on to sub_50c180 as it came
    return fe::direct<0x50c180, watb::textPalette>(app, cpu);
}

void frontend::textStateRestore(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f4760", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 from = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::textStateRestore(app, cpu, from); });
}

/* sub_4f4780: the checksum of a block, by the table at 0x566b54. */
reg32 frontend::wata::blockChecksum(App* app, CPU& cpu, reg32 start, reg32 size)
{
    Guest guest(app, cpu);
    const reg32 end = start + size;
    constexpr reg32 kTable = 0x566b54;
    reg32 sum = 0xfbea;
    for (reg32 at = start; at < end; ++at)
    {
        const reg32 index = (sum & 0xff) ^ guest.byte(at);
        sum = (sum >> 8) ^ guest.word(kTable + index * 2);
    }
    guest.leaveEdx(sum);
    return sum;
}

void frontend::blockChecksum(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f4780", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi };
    const reg32 start = cpu.eax, size = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::blockChecksum(app, cpu, start, size); });
}

/* sub_4f47c0: how a shape's file ends: 1 at a BEND, 0 not, 2 of a kind that has no such end. */
reg32 frontend::wata::shapeEndsAtBend(App* app, CPU& cpu, reg32 shape)
{
    Guest guest(app, cpu);
    constexpr reg32 kBend = 0x42454e44;
    const std::uint8_t flags = guest.byte(shape + 3);
    if ((flags & 2) == 0 || (flags & 0xc0) != 0)
        return 2;
    const reg32 raw = guest.dword(shape + 0x10 + guest.dword(shape + 4));
    const reg32 word = (raw >> 24) | ((raw >> 8) & 0xff00) | ((raw << 8) & 0xff0000) | (raw << 24);
    return word == kBend ? 1 : 0;
}

void frontend::shapeEndsAtBend(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f47c0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    const reg32 shape = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::shapeEndsAtBend(app, cpu, shape); });
}

/* sub_4f87e0: a graphics driver's DLL opened: the name given, after a '!', or the environment's. */
reg32 frontend::wata::thrashOpenByName(App* app, CPU& cpu, reg32 given)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    constexpr reg32 kGetenv = 0x50ead0, kDriverVariable = 0x54d700, kOpen = 0x4f8390;
    constexpr reg32 kStarted = 0x564a68, kOpened = 0x564a6c;
    reg32 name = given;
    if (guest.byte(given) == '!')
    {
        name = given + 1;
    }
    else
    {
        const reg32 fromEnvironment = guest.call(kGetenv, args({ kDriverVariable }));
        if (fromEnvironment != 0)
            name = fromEnvironment;
    }
    if (guest.dword(kStarted) == 0)
        guest.setDword(kStarted, 1);
    if (guest.dword(kOpened) != 0)
        fe::direct<0x4f87a0, wina::releaseDriverSlot>(app, cpu);
    return guest.call(kOpen, args({ name }));
}

void frontend::thrashOpenByName(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f87e0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const reg32 given = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::thrashOpenByName(app, cpu, given); });
}

/* sub_4f90e0: the window shown again, its descriptions kept through it. */
reg32 frontend::wata::windowReshow(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 0x70);
    if (guest.dword(0x565028) == 0)
        return 1;
    if (guest.dword(0x567dcc) == 0)
        return fe::direct<0x4ed330, menz::ddrawRelockSurface>(app, cpu, 0x9f38d0);
    const reg32 kept = guest.frame();
    fe::direct<0x4f46d0, windowSave>(app, cpu, kept);
    fe::direct<0x4ead90, eaa::bitmapDrawToScreen>(app, cpu);
    fe::direct<0x4f4700, windowRestore>(app, cpu, kept);
    return 1;
}

void frontend::windowReshow(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f90e0", 0, 0, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::windowReshow(app, cpu); });
}

/* sub_4f92b0: the heap's block allocated and handed over. */
reg32 frontend::wata::heapStart(App* app, CPU& cpu, reg32 size)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    cpu.eax = size;   // sub_4f7900's, as it came
    guest.leaveEcx(size);
    const reg32 block = guest.call(0x4f7900);
    guest.setDword(0x5644fc, block);
    return fe::direct<0x4f9270, menz::memClassCreateRam>(app, cpu, block, cpu.ecx);
}

void frontend::heapStart(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f92b0", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const reg32 size = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::heapStart(app, cpu, size); });
}

/* sub_4f92d0: 64 megabytes of it: tools/apply_heap_budget.py's, 16 in the game. */
reg32 frontend::wata::heapStartDefault(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    guest.leaveEdx(0);   // the game's code hands it 0 in edx too, which it does not read
    return fe::direct<0x4f92b0, heapStart>(app, cpu, 0x4000000);
}

void frontend::heapStartDefault(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f92d0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::heapStartDefault(app, cpu); });
}

/* sub_4f9320: the fonts' system started, once. */
reg32 frontend::wata::fontsStart(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    if (guest.dword(0x567194) != 0)
        return guest.eax();
    fe::direct<0x518fd0, menz::videoBuffersStart>(app, cpu, 0);
    guest.setDword(0x567190, fe::direct<0x519f10, menz::fontCreate>(app, cpu, 0x569064));
    guest.leaveEcx(1);
    // By address: the ecx it leaves is read after it.
    const reg32 result = guest.call(0x4f2a78, args({ 0x4f9360 }));
    guest.setDword(0x567194, cpu.ecx);
    return result;
}

void frontend::fontsStart(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f9320", 0, 0, nullptr, 0, fe::kRestoreEcx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::fontsStart(app, cpu); });
}

/* sub_4f99f0: TAPI's line opened, the callback at 0x4f9500. */
reg32 frontend::wata::tapiInitialize(App* app, CPU& cpu, reg32 instance, reg32 wanted)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    constexpr reg32 kLines = 0x9f4d90, kLineApp = 0x9f4d74, kCallback = 0x4f9500, kLineInitialize = 0x5310be;
    guest.setDword(0x9f4d7c, wanted);
    guest.setDword(0x9f4d88, 0);
    guest.setDword(0x9f4da8, 0);
    guest.setDword(0x9f4da4, 0);
    guest.setDword(0x9f4da0, 0);
    guest.setDword(0x9f4d94, 0);
    guest.leaveEdx(0);
    // By address: the ebx its stub may leave is left at the end, which sub_4f9920 reads.
    const reg32 result = guest.call(kLineInitialize, args({}, { kLineApp, instance, kCallback, 0, kLines }));
    if (result != 0)
        return 0xa;
    return guest.dword(kLines) == 0 ? 7 : result;
}

void frontend::tapiInitialize(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f99f0", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const reg32 instance = cpu.eax, wanted = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::tapiInitialize(app, cpu, instance, wanted); });
}

/* sub_4f9920: TAPI set up for the modem games: 0, or what went wrong. */
reg32 frontend::wata::tapiSetup(App* app, CPU& cpu, reg32 wanted)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 3 * 4);
    constexpr reg32 kTapiReady = 0x5671a8, kGetWindowLong = 0x534758, kAcquireWindow = 0x4eba70;
    constexpr reg32 kInitialize = 0x4f99f0;
    constexpr reg32 kFailed = 0xa, kGwlHinstance = reg32(0) - 6;
    guest.setDword(0x9f4d9c, 0);
    if (guest.dword(kTapiReady) == 0xffffffffu)
    {
        guest.setDword(kErrorFile, 0x54da88);
        guest.setDword(kErrorFunction, 0x54da98);
        guest.setDword(kErrorLine, 0x123);
        guest.call(kError, args({}, { 0x54daa4 }));
        return kFailed;
    }
    const reg32 window = guest.call(kAcquireWindow, args({}, { kGwlHinstance }));
    const reg32 instance = callImport<kGetWindowLong, &win32::user32::GetWindowLongA>(guest, cpu, window, kGwlHinstance);
    guest.leaveEbx(wanted);
    guest.leaveEcx(instance);
    // By address: the ebx it may leave is read after it, as the second's wanted.
    const reg32 first = guest.call(kInitialize, args({ instance, 1 }));
    if (first != 0)
    {
        fe::direct<0x4f97c0, eab::lineShutdown>(app, cpu);
        return first;
    }
    if (fe::direct<0x4f9d20, netb::tapiOpenLine>(app, cpu) != 0)
    {
        fe::direct<0x4f9df0, netb::tapiGetCommHandleConfig>(app, cpu);
        fe::direct<0x4f97c0, eab::lineShutdown>(app, cpu);
        fe::direct<0x4df8e0, ea::sleepFor>(app, cpu, 0x1f4);
        // The instance and the wanted kept in ecx and ebx across the calls.
        if (fe::direct<0x4f99f0, tapiInitialize>(app, cpu, cpu.ecx, cpu.ebx) != kFailed)
        {
            if (fe::direct<0x4f9d20, netb::tapiOpenLine>(app, cpu) != 0)
                return 0;
            return kFailed;
        }
    }
    fe::direct<0x4f97c0, eab::lineShutdown>(app, cpu);
    return kFailed;
}

void frontend::tapiSetup(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f9920", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 wanted = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::tapiSetup(app, cpu, wanted); });
}

/* sub_4fb380: the screen's size and depth from the driver's record into the global ones. */
reg32 frontend::wata::screenTakeSize(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    guest.setDword(0x56436c, guest.dword(0x9f4f3c));
    guest.setDword(0x564370, guest.dword(0x9f4f38));
    guest.setDword(0x564374, guest.dword(0x9f4f54));
    return guest.dword(0x564374);
}

void frontend::screenTakeSize(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4fb380", 0, 0, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::screenTakeSize(app, cpu); });
}

/* sub_4fb3b0 and sub_4fb3c0: the screen's lock count. */
reg32 frontend::wata::screenLockCountUp(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    guest.setDword(0x9f4f48, guest.dword(0x9f4f48) + 1);
    return guest.eax();
}

void frontend::screenLockCountUp(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4fb3b0", 0, 0, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::screenLockCountUp(app, cpu); });
}

reg32 frontend::wata::screenLockCountDown(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    guest.setDword(0x9f4f48, guest.dword(0x9f4f48) - 1);
    return guest.eax();
}

void frontend::screenLockCountDown(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4fb3c0", 0, 0, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::screenLockCountDown(app, cpu); });
}

/* sub_4fb3d0: the driver's record set up for the screen, and these functions hooked in. */
reg32 frontend::wata::screenInstallDriver(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
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

void frontend::screenInstallDriver(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4fb3d0", 0, 0, nullptr, 0, fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::screenInstallDriver(app, cpu); });
}

/* sub_4fb700: the num lock off. */
reg32 frontend::wata::numLockOff(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    return guest::direct<0x4fb710, lockKeySet>(app, cpu, 1, 0);
}

void frontend::numLockOff(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4fb700", 0, 0, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::numLockOff(app, cpu); });
}

/* sub_4fb790: a directory's search: a big file's entry opened by its name, 0 when found. */
reg32 frontend::wata::bigFileOpenEntry(App* app, CPU& cpu, reg32 big, reg32 search)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 5 * 4 + 8);
    constexpr reg32 kFindEntry = 0x4ea040, kOpenMemory = 0x4e4140;
    const reg32 entryOffset = guest.frame(), entrySize = entryOffset + 4;
    const reg32 wantedFile = guest.dword(search);
    const bool anyFile = guest.byte(wantedFile) == 0;
    if (!anyFile && guest.call(kStrcmp, args({ big + 0x14, guest.dword(search) })) != 0)
        return 1;
    // By address: the edx it leaves is left at the end.
    if (guest.call(kFindEntry, args({ guest.dword(big + 0x10), guest.dword(search + 4), 0, entryOffset }, { entrySize })) == 0)
        return anyFile ? 1 : 0;
    const reg32 handle = guest.call(kOpenMemory, args({ guest.dword(big + 0x10) + guest.dword(entryOffset), guest.dword(entrySize), guest.dword(big + 8), 0 }, { search + 8 }));
    guest.setDword(search + 0xc, handle);
    return 0;
}

void frontend::bigFileOpenEntry(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4fb790", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 big = cpu.eax, search = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::bigFileOpenEntry(app, cpu, big, search); });
}

/* sub_4ead20: the window as it stands set: an error if it is locked. */
reg32 frontend::wata::windowApply(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, kTailCall);
    constexpr reg32 kLockedBy = 0x9f3d2f, kWindow = 0x56502c;
    if (guest.byte(kLockedBy) != 0)
    {
        const reg32 ebx = cpu.ebx, ecx = cpu.ecx, edx = cpu.edx;
        guest.setDword(kErrorFile, 0x54b6c8);
        guest.setDword(kErrorFunction, 0x54b6d8);
        guest.setDword(kErrorLine, 0x5d);
        guest.call(kError, args({}, { 0x54b6e8, guest.byte(kLockedBy) }));
        guest.leaveEbx(ebx);
        guest.leaveEcx(ecx);
        guest.leaveEdx(edx);
    }
    return fe::direct<0x4eada0, eaa::bitmapDrawInto>(app, cpu, kWindow);
}

void frontend::windowApply(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4ead20", 0, 0, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::windowApply(app, cpu); });
}

/* sub_4f7c48: the exit functions registered at 0x9f380c run, last first, once. */
reg32 frontend::wata::exitFunctionsRun(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 5 * 4);
    constexpr reg32 kCount = 0x9f3890, kTable = 0x9f380c, kDone = 0x21;
    const reg32 count = guest.dword(kCount);
    if (count == kDone)
        return guest.eax();
    guest.setDword(kCount, kDone);
    for (reg32 left = count; left != 0; --left)
    {
        const reg32 function = guest.dword(kTable + left * 4);
        guest.call(function, args({ function }));
    }
    return guest.eax();
}

void frontend::exitFunctionsRun(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f7c48", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::exitFunctionsRun(app, cpu); });
}

/* sub_4e0658, sub_4e0659 and sub_4f45d0: nothing, in a table of what can be hooked. */
reg32 frontend::wata::nothingA(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    return guest.eax();
}

void frontend::nothingA(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4e0658", 0, 0, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::nothingA(app, cpu); });
}

reg32 frontend::wata::nothingB(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    return guest.eax();
}

void frontend::nothingB(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4e0659", 0, 0, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::nothingB(app, cpu); });
}

reg32 frontend::wata::nothingC(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    return guest.eax();
}

void frontend::nothingC(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f45d0", 0, 0, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::nothingC(app, cpu); });
}

/* sub_4f8930: the graphics started: the driver's settings in the environment the first time, the
 * display mode, and the libraries over it. */
reg32 frontend::wata::graphicsStart(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 2 * 4);
    constexpr reg32 kSetenv = 0x4f8b50;
    constexpr reg32 kHeap = 0x5644fc, kModeSet = 0x564364, kWidth = 0x564384, kHeight = 0x564388, kDepth = 0x56438c;
    constexpr reg32 kYes = 0x54d7d8, kNo = 0x54d7f0, kEmpty = 0x54d7ac;
    struct Setting
    {
        reg32 name, value, overwrite;
    };
    static constexpr Setting kSettings[] = {
        { 0x54d7b0, kEmpty, 0 }, { 0x54d7c0, kEmpty, 0 }, { 0x54d7dc, kYes, 1 }, { 0x54d7f4, kNo, 1 }, { 0x54d800, kNo, 1 },
        { 0x54d80c, kNo, 1 }, { 0x54d818, kNo, 1 }, { 0x54d824, kYes, 1 }, { 0x54d834, kNo, 1 }, { 0x54d844, kNo, 1 },
        { 0x54d854, kNo, 1 }, { 0x54d864, kNo, 1 }, { 0x54d870, kYes, 1 },
    };
    guest.call(0x4df740, args({ 0 }));
    if (guest.dword(kHeap) == 0)
        fe::direct<0x4f92d0, heapStartDefault>(app, cpu);
    if (guest.dword(kModeSet) == 0)
    {
        for (const Setting& setting : kSettings)
            guest.call(kSetenv, args({ setting.name, setting.value, setting.overwrite }));
        const reg32 shown = fe::direct<0x5089d0, rendf::setDisplayMode>(app, cpu, guest.dword(kWidth), guest.dword(kHeight), guest.dword(kDepth));
        if (guest.dword(kModeSet) == 0)
            return shown;
    }
    fe::direct<0x503e50, wina::paletteSet>(app, cpu, 0, 0x100, 0x568c30);
    guest.call(0x48eff0, args({ guest.dword(0x5643d8) }));
    guest.call(0x4f1c70);
    guest.call(0x4f1ef0);
    fe::direct<0x517b30, menz::eaTextStart>(app, cpu);
    fe::direct<0x4ef520, eab::fontUse>(app, cpu, 0x5662e4);
    const reg32 black = fe::direct<0x4ef760, eab::colourPack>(app, cpu, 0xff000000u);
    // The game's code hands black in edx to the second too, which it does not read, for the colours'.
    const reg32 white = fe::direct<0x4ef760, eab::colourPack>(app, cpu, 0xffffffffu);
    fe::direct<0x4ef830, eab::fontSetColours>(app, cpu, white, black);
    fe::direct<0x4f9320, fontsStart>(app, cpu);
    if (guest.dword(0x564368) == 0)
        fe::direct<0x4fb3d0, screenInstallDriver>(app, cpu);
    const reg32 ecx = cpu.ecx, edi = cpu.edi;
    // What it gives back in eax, on to sub_4f01f0 as the game's code has it.
    cpu.eax = fe::direct<0x4fb700, numLockOff>(app, cpu);
    fe::direct<0x4f01f0, eab::inputStateOpen>(app, cpu);
    fe::direct<0x517ba0, menz::keyboardIsPc98>(app, cpu);
    cpu.edi = 0x517d60;
    // Its own address handed in eax, which it does not read.
    fe::direct<0x4feffc, watb::returnZero>(app, cpu);
    // And 0 in edx, which it does not read, kept on to sub_4fc250's count of records.
    guest.leaveEdx(0);
    fe::direct<0x4f00f0, eab::keySetHookFlag>(app, cpu, 0x4f00e0);
    guest.setDword(0x564404, cpu.edi);
    fe::direct<0x517de0, menz::debugKeysBind>(app, cpu);
    fe::direct<0x4fc250, eab::asyncFilesInit>(app, cpu, 0, 0);
    fe::direct<0x518040, menz::eaSetLoadHook>(app, cpu);
    fe::direct<0x4fd3f0, mova::projectionViewDegrees>(app, cpu, 0x1e, 0x14, guest.dword(kWidth), guest.dword(kHeight));
    const reg32 result = fe::direct<0x4fd280, mova::projectionCentre>(app, cpu, 0, 0);
    guest.leaveEcx(ecx);
    cpu.edi = edi;
    return result;
}

void frontend::graphicsStart(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_4f8930", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::graphicsStart(app, cpu); });
}

/* sub_47a970: the sky's shape for a track: its description's words in the shape's name give the
 * sun's direction and what shines. */
reg32 frontend::wata::skyLoad(App* app, CPU& cpu, reg32 shapes, reg32 shapeName)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 5 * 4 + 0x118);
    constexpr reg32 kLoaded = 0x55d27c, kLocateShape = 0x4ef270;
    constexpr reg32 kToRadiansA = 0x53b330, kToRadiansB = 0x53b328, kDistance = 0x53b338;
    constexpr reg32 kFirstWord = 0x53b300, kSecondWord = 0x53b304, kThirdWord = 0x53b308;
    constexpr reg32 kFogWord = 0x53b30c, kNoFogWord = 0x53b314, kBrightWord = 0x53b31c;
    const reg32 buffer = guest.frame(), angleBits = buffer + 0x114;
    guest.setDword(kLoaded, 0);
    const reg32 shape = guest.call(kLocateShape, args({ shapes, shapeName }));
    if (shape == 0)
        return 0;
    const reg32 name = fe::direct<0x4f24e0, eab::shapeFindHotspot>(app, cpu, shape);
    if (name == 0)
        return 0;
    // The number after a word of the name, or no number and it ends.
    bool ended = false;
    const auto numberAfter = [&](reg32 word) -> sreg32 {
        guest.copyString(buffer, name);
        const reg32 found = guest.call(kStrstr, args({ buffer, word }));
        if (found == 0)
        {
            ended = true;
            return 0;
        }
        reg32 at = found + 1;
        while (guest.byte(at) != 0 && isDigit(guest, std::uint8_t(guest.byte(at) + 1)))
            ++at;
        guest.setByte(at, 0);
        return sreg32(guest.call(kAtoi, args({ found + 1 })));
    };
    const sreg32 first = numberAfter(kFirstWord);
    if (ended)
        return 0;
    const sreg32 second = numberAfter(kSecondWord);
    if (ended)
        return 0;
    const sreg32 third = numberAfter(kThirdWord);
    if (ended)
        return 0;
    guest.setDword(0x55d29c, guest.call(kStrstr, args({ name, kFogWord })) != 0 ? 1 : 0);
    guest.setDword(0x55d2a0, guest.call(kStrstr, args({ name, kNoFogWord })) != 0 ? 0 : 1);
    guest.setDword(0x55d2a4, guest.call(kStrstr, args({ name, kBrightWord })) != 0 ? 1 : 0);

    const Float toA = Float(app->getMemory<double>(kToRadiansA));
    const Float toB = Float(app->getMemory<double>(kToRadiansB));
    const Float distance = Float(app->getMemory<double>(kDistance));
    const auto scaled = [&](reg32 slot) {
        guest.setFloat(slot, float(fp::mul(guest.fpu().pop(), distance)));
    };
    const Float firstAngle = fp::mul(Float(first), toA);
    const Float thirdAngle = fp::mul(Float(third), toB);
    guest.setFloat(angleBits, float(firstAngle));
    guest.setFloat(0x55d298, float(thirdAngle));
    const reg32 angle = guest.dword(angleBits);
    const auto argumentFor = [&](sreg32 degrees) {
        float value = float(fp::mul(Float(degrees), toA));
        reg32 bits;
        std::memcpy(&bits, &value, sizeof bits);
        return bits;
    };
    fe::direct<0x4ea950, simd::cosine>(app, cpu, angle);
    Float kept = fp::mul(guest.fpu().pop(), distance);
    const reg32 secondBits = argumentFor(second);
    guest.setFloat(0x55d280, float(kept));
    fe::direct<0x4ea950, simd::cosine>(app, cpu, secondBits);
    scaled(0x55d284);
    fe::direct<0x4ea930, simd::sine>(app, cpu, angle);
    scaled(0x55d288);
    fe::direct<0x4ea950, simd::cosine>(app, cpu, angle);
    kept = fp::mul(guest.fpu().pop(), distance);
    const reg32 besideBits = argumentFor(sreg32(reg32(second) - 0x5a));
    guest.setFloat(0x55d28c, float(kept));
    fe::direct<0x4ea950, simd::cosine>(app, cpu, besideBits);
    scaled(0x55d290);
    fe::direct<0x4ea930, simd::sine>(app, cpu, angle);
    scaled(0x55d294);
    const reg32 result = fe::direct<0x4d4080, rende::loadShapeTexture>(app, cpu, shapes, shapeName, 0x80, 0x6ff9cc, 7);
    guest.setDword(kLoaded, 1);
    return result;
}

void frontend::skyLoad(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_47a970", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 shapes = cpu.eax, shapeName = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::skyLoad(app, cpu, shapes, shapeName); });
}

/* ---------------------------------------------------------------------------
 * The HUD editor: an element under the mouse
 * ------------------------------------------------------------------------- */

/* tools/apply_hud_editor.py's changes, in nfs3hp_main.cpp: the layout as laid out and not as
 * drawn, and how far past the screen's edge a box that draws nothing there may go. */
void hudLayoutDesigned(win32::WinApplication* app, x86::reg32 rect, x86::reg32 element);
x86::sreg32 hudDragSlack(win32::WinApplication* app, x86::reg32 object, x86::reg32 side);

namespace
{

constexpr reg32 kScreenWidth = 0x7cdac8, kScreenHeight = 0x7cdacc;
constexpr reg32 kMouseX = 0x55e5e4, kMouseY = 0x55e5e8;
constexpr reg32 kPercentToX = 0x539bd4, kPercentToY = 0x539bdc, kLastPixel = 0x539be4;   // doubles
constexpr reg32 kXToPercent = 0x539bec, kYToPercent = 0x539bf4;                          // doubles

// An element of the HUD editor's.
constexpr reg32 kElFlags = 5, kElX = 6, kElY = 8, kElPercentY = 0x1a, kElPercentX = 0x1c, kElWidth = 0x1e, kElHeight = 0x20;
constexpr reg32 kElView = 0x3c, kElDesigned = 0x50, kElRight = 0x60, kElBottom = 0x64, kElGrabX = 0x68, kElGrabY = 0x6c;
constexpr reg32 kElMoved = 0x70, kElPartner = 0x74, kElHandle = 0x78, kElEdges = 0x79, kElDragging = 0x7c;
constexpr std::uint8_t kFlagDragged = 4;
constexpr reg32 kDragStart = 0x677e00, kEditedCamera = 0x677dfc, kEditedSide = 0x677df8;
constexpr reg32 kSplitScreen = 1;  // GameSettings::playMode
constexpr reg32 kSetPointerBounds = 0x4352d0;
constexpr std::int16_t kSmallestSide = 0x1a;

}

/* sub_459fe0: an element dropped: where it now is into the layout's table, the HUD rebuilt, and
 * its place on the screen worked out from its percentages again, the percentages from what the
 * drag began with. */
reg32 frontend::wata::hudEditorDrop(App* app, CPU& cpu, reg32 element, reg32 start)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 5 * 4 + 4);
    constexpr reg32 kHudBand = 0x4800a0;
    constexpr reg32 kToX = 0x539bbc, kToY = 0x539bc4, kSize = 0x539bcc;   // doubles
    reg32 result = element;
    if (gameSettings(guest).playMode != kSplitScreen)
    {
        fe::X87 x87(guest);
        const auto constant = [&](reg32 at) { return Float(app->getMemory<double>(at)); };
        const auto wordField = [&](reg32 field) { return Float(x86::sreg16(guest.word(element + field))); };
        const auto sized = [&](reg32 field, reg32 screen) {
            return reg32(sreg32(x86::sreg16(guest.word(element + field)))) * screen;
        };
        // The element's view in edx across sub_422bd0 (handed to it too, which keeps it), on to sub_4853c0.
        const reg32 view = guest.dword(element + kElView);
        guest.leaveEdx(view);
        const reg32 target = fe::direct<0x422bd0, carstate::targetOfCamera>(app, cpu, 0);
        const reg32 side = (guest.byte(target + 0x200) & 0x20) != 0 ? 1 : 0;
        const reg32 layout = SETTINGS_ADDRESS(hud[0].rects) + side * reg32(sizeof(HudRecord)) + (view << 4);
        copyDwords(guest, layout, element + kElDesigned, 4);
        hudLayoutDesigned(app, layout, view); /* port: the layout as laid out, not as drawn */
        fe::direct<0x4853c0, hudr::rebuildElement>(app, cpu, 0, view);
        guest.call(kHudBand, args({ 0 }));
        guest.setWord(element + kElPercentX, guest.word(start + 2));
        guest.setWord(element + kElPercentY, guest.word(start));

        const reg32 width = guest.dword(kScreenWidth), height = guest.dword(kScreenHeight);
        const Float toX = constant(kToX);
        guest.setWord(element + kElX, std::uint16_t(x87.truncated(fp::mul(Float(sreg32(sized(kElPercentX, width))), toX))));
        const Float toY = constant(kToY);
        guest.setWord(element + kElY, std::uint16_t(x87.truncated(fp::mul(Float(sreg32(sized(kElPercentY, height))), toY))));
        const Float across = fp::mul(toX, Float(sreg32(sized(kElWidth, width))));
        const Float rightEdge = fp::add(across, wordField(kElX));
        result = sized(kElHeight, height);
        const Float down = fp::mul(toY, Float(sreg32(result)));
        const Float size = constant(kSize);
        const Float right = fp::add(rightEdge, size);
        const Float bottomEdge = fp::add(down, wordField(kElY));
        const std::int32_t rightPixel = x87.truncated(right);
        const Float bottom = fp::add(size, bottomEdge);
        guest.setDword(element + kElRight, reg32(rightPixel));
        guest.setDword(element + kElBottom, reg32(x87.truncated(bottom)));
    }
    const reg32 partner = guest.dword(element + kElPartner);
    guest.setDword(element + kElMoved, 1);
    if (partner != 0)
    {
        guest.setDword(partner + kElMoved, 1);
        guest.setDword(element + kElPartner, 0);
    }
    return result;
}

void frontend::hudEditorDrop(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_459fe0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 element = cpu.eax, start = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::hudEditorDrop(app, cpu, element, start); });
}

/* sub_45a120: an element under the mouse, each frame: the drag begun when the button goes down
 * (where it was grabbed, the pointer held inside what it may reach), carried on while it stays
 * down -- moved, or with handles resized by its edges, held on the screen -- and the element
 * dropped when it comes up. */
reg32 frontend::wata::hudEditorDrag(App* app, CPU& cpu, reg32 element)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 6 * 4 + 0x64);
    fe::X87 x87(guest);
    const auto constant = [&](reg32 at) { return Float(app->getMemory<double>(at)); };
    const auto i16 = [&](reg32 field) -> sreg32 { return sreg32(x86::sreg16(guest.word(element + field))); };
    const auto wordField = [&](reg32 field) { return Float(x86::sreg16(guest.word(element + field))); };
    const auto dwordField = [&](reg32 field) { return Float(sreg32(guest.dword(element + field))); };
    const auto setWordTo = [&](reg32 field, std::int32_t value) { guest.setWord(element + field, std::uint16_t(value)); };
    const auto screenWidth = [&] { return guest.dword(kScreenWidth); };
    const auto screenHeight = [&] { return guest.dword(kScreenHeight); };
    const auto integerOf = [&](reg32 value) { return Float(sreg32(value)); };
    const auto mouseX = [&] { return fp::mul(integerOf(guest.dword(kMouseX) * screenWidth()), constant(kPercentToX)); };
    const auto mouseY = [&] { return fp::mul(integerOf(guest.dword(kMouseY) * screenHeight()), constant(kPercentToY)); };
    const auto fit = [&] {
        const Float half = constant(kLastPixel);
        const Float acrossPixels = fp::add(fp::mul(integerOf(reg32(i16(kElWidth)) * screenWidth()), constant(kPercentToX)), wordField(kElX));
        guest.setDword(element + kElRight, reg32(x87.truncated(fp::add(acrossPixels, half))));
        const reg32 tall = reg32(i16(kElHeight)) * screenHeight();
        const Float percentX = fp::div(fp::mul(wordField(kElX), constant(kXToPercent)), integerOf(screenWidth()));
        const Float downPixels = fp::add(fp::mul(integerOf(tall), constant(kPercentToY)), wordField(kElY));
        const Float bottom = fp::add(half, downPixels);
        guest.setDword(element + kElBottom, reg32(x87.truncated(bottom)));
        setWordTo(kElPercentX, x87.truncated(percentX));
        setWordTo(kElPercentY, x87.truncated(fp::div(fp::mul(wordField(kElY), constant(kYToPercent)), integerOf(screenHeight()))));
    };
    const auto percentYNow = [&] {
        setWordTo(kElPercentY, x87.truncated(fp::div(fp::mul(wordField(kElY), constant(kYToPercent)), integerOf(screenHeight()))));
    };
    const auto percentXNow = [&] {
        setWordTo(kElPercentX, x87.truncated(fp::div(fp::mul(wordField(kElX), constant(kXToPercent)), integerOf(screenWidth()))));
    };

    // The box's size on the screen, what the editing camera's layout adds to its height.
    const Float boxWidth = fp::mul(integerOf(reg32(i16(kElWidth)) * screenWidth()), constant(kPercentToX));
    const Float boxHeight = fp::mul(integerOf(reg32(i16(kElHeight)) * screenHeight()), constant(kPercentToY));
    const reg32 camera = guest.dword(kEditedCamera);
    const reg32 widthPixels = reg32(x87.truncated(boxWidth));
    reg32 heightPixels = reg32(x87.truncated(boxHeight));
    if (camera == 0 || camera == 1)
    {
        const reg32 row = guest.dword(kEditedSide) * 368;
        heightPixels += guest.dword(row + 0x7497b4) - guest.dword(row + 0x7497ac);
    }

    if (fe::direct<0x49c080, glue::mouseKey>(app, cpu) == 0)
    {
        if (guest.word(element + kElDragging) == 0)
            return element;
        const std::uint8_t flags = guest.byte(element + kElFlags);
        guest.setWord(element + kElDragging, 0);
        guest.setByte(element + kElFlags, flags & std::uint8_t(~kFlagDragged));
        guest::direct<0x459fe0, hudEditorDrop>(app, cpu, element, kDragStart);
        // By address: what it leaves in eax is handed back, which its core does not give.
        return guest.call(kSetPointerBounds, args({ 0, 0, screenWidth(), screenHeight() }));
    }

    if (guest.word(element + kElDragging) == 0)
    {
        copyDwords(guest, kDragStart, element + kElPercentY, 2);
        guest.setByte(element + kElFlags, guest.byte(element + kElFlags) | kFlagDragged);
        const reg32 handle = guest.dword(element + kElHandle);
        if (handle == 0 || handle == 0x108 || handle == 0x208 || handle == 0x409 || handle == 0x50a || handle == 0x60b)
            guest.setDword(element + kElGrabX, reg32(x87.truncated(fp::sub(mouseX(), wordField(kElX)))));
        else if (handle == 0x809 || handle == 0x90b || handle == 0xa0a)
            guest.setDword(element + kElGrabX, reg32(x87.truncated(fp::sub(dwordField(kElRight), mouseX()))));
        if (handle == 0 || handle == 0x108 || handle == 0x409 || handle == 0x50a || handle == 0x809 || handle == 0x90b)
            guest.setDword(element + kElGrabY, reg32(x87.truncated(fp::sub(mouseY(), wordField(kElY)))));
        else if (handle == 0x208 || handle == 0x60b || handle == 0xa0a)
            guest.setDword(element + kElGrabY, reg32(x87.truncated(fp::sub(dwordField(kElBottom), mouseY()))));
        fit();
        if (gameSettings(guest).playMode == kSplitScreen && guest.dword(element + kElView) != 8)
        {
            const reg32 tall = screenHeight();
            const sreg32 halfTall = sreg32(tall) / 2;
            const reg32 mouseRaw = guest.dword(kMouseY) * tall;
            const Float heightAsFloat = integerOf(tall);
            const Float mouse = fp::mul(integerOf(mouseRaw), constant(kPercentToY));
            const Float below = fp::sub(dwordField(kElBottom), mouse);
            const Float above = fp::sub(mouse, wordField(kElY));
            // Inside the half of the screen the mouse is in.
            if (sreg32(guest.dword(kMouseY)) < 0xf0)
            {
                const reg32 top = reg32(x87.truncated(above));
                const reg32 bottom = reg32(x87.truncated(fp::sub(Float(halfTall), below)));
                fe::direct<0x4352d0, controls::boundPointer>(app, cpu, 0, top, screenWidth(), bottom);
            }
            else
            {
                const Float lowest = fp::sub(heightAsFloat, below);
                const reg32 bottom = reg32(x87.truncated(lowest));
                const Float highest = fp::add(fp::add(above, Float(halfTall)), Float(1.0));
                const reg32 top = reg32(x87.truncated(highest));
                fe::direct<0x4352d0, controls::boundPointer>(app, cpu, 0, top, screenWidth(), bottom);
            }
        }
        const reg32 view = guest.dword(element + kElView);
        const reg32 second = sreg32(view) >= 0x17 ? 1 : 0;
        guest.setDword(kEditedSide, second);
        guest.setDword(kEditedCamera, second != 0 ? view - 0x17 : guest.dword(element + kElView));
        guest.setWord(element + kElDragging, 1);
    }

    const reg32 tall = screenHeight();
    const reg32 mouseRawY = guest.dword(kMouseY) * tall;
    if (guest.dword(element + kElHandle) == 0)
    {
        const Float x = mouseX();
        const Float y = fp::mul(integerOf(mouseRawY), constant(kPercentToY));
        setWordTo(kElX, x87.truncated(fp::sub(x, dwordField(kElGrabX))));
        setWordTo(kElY, x87.truncated(fp::sub(y, dwordField(kElGrabY))));

        reg32 room = screenWidth() - widthPixels;
        room += reg32(hudDragSlack(app, element, 1)); /* port: what is drawn stays on the screen */
        if (i16(kElX) > sreg32(room))
            setWordTo(kElX, std::int32_t(guest.word(kScreenWidth) - widthPixels + reg32(hudDragSlack(app, element, 1)))); /* port */
        if (x86::sreg16(guest.word(element + kElX)) < x86::sreg16(-hudDragSlack(app, element, 0))) /* port: was 0 */
            guest.setWord(element + kElX, std::uint16_t(-hudDragSlack(app, element, 0))); /* port: was 0 */
        room = screenHeight() - heightPixels;
        room += reg32(hudDragSlack(app, element, 3)); /* port */
        if (i16(kElY) > sreg32(room))
            setWordTo(kElY, std::int32_t(guest.word(kScreenHeight) - heightPixels + reg32(hudDragSlack(app, element, 3)))); /* port */
        if (x86::sreg16(guest.word(element + kElY)) < x86::sreg16(-hudDragSlack(app, element, 2))) /* port: was 0 */
            guest.setWord(element + kElY, std::uint16_t(-hudDragSlack(app, element, 2))); /* port: was 0 */
        fit();
    }
    else
    {
        const std::uint8_t edges = guest.byte(element + kElEdges);
        const auto oneOver = [&](reg32 size) { return fp::div(Float(1.0), integerOf(size)); };
        const auto bottomPixel = [&] { return guest.dword(element + kElBottom); };
        const auto rightPixel = [&] { return guest.dword(element + kElRight); };
        const Float one = Float(1.0);
        if ((edges & 1) != 0)
        {
            // By its top edge.
            const Float y = fp::mul(integerOf(mouseRawY), constant(kPercentToY));
            setWordTo(kElY, x87.truncated(fp::sub(y, dwordField(kElGrabY))));
            const Float scaled = fp::mul(integerOf(bottomPixel() - reg32(i16(kElY)) + 1), constant(kYToPercent));
            const Float perPixel = oneOver(screenHeight());
            const std::int32_t percent = x87.truncated(fp::mul(scaled, perPixel));
            setWordTo(kElHeight, percent);
            if (std::int16_t(percent) < kSmallestSide)
            {
                const Float smallest = fp::mul(integerOf(tall * 26), constant(kPercentToY));
                setWordTo(kElHeight, kSmallestSide);
                setWordTo(kElY, x87.truncated(fp::add(fp::sub(dwordField(kElBottom), smallest), one)));
            }
            else if (i16(kElY) < 0)
            {
                const Float wanted = fp::mul(fp::mul(dwordField(kElBottom), constant(kYToPercent)), perPixel);
                const Float whole = fp::add(wanted, one);
                setWordTo(kElY, 0);
                setWordTo(kElHeight, x87.truncated(whole));
            }
            percentYNow();
        }
        else if ((edges & 2) != 0)
        {
            // By its bottom edge.
            const Float y = fp::mul(integerOf(mouseRawY), constant(kPercentToY));
            guest.setDword(element + kElBottom, reg32(x87.truncated(fp::add(y, dwordField(kElGrabY)))));
            const Float scaled = fp::mul(integerOf(bottomPixel() - reg32(i16(kElY)) + 1), constant(kYToPercent));
            const Float perPixel = oneOver(screenHeight());
            const std::int32_t percent = x87.truncated(fp::mul(scaled, perPixel));
            setWordTo(kElHeight, percent);
            if (std::int16_t(percent) < kSmallestSide)
            {
                const Float smallest = fp::mul(integerOf(tall * 26), constant(kPercentToY));
                setWordTo(kElHeight, kSmallestSide);
                guest.setDword(element + kElBottom, reg32(x87.truncated(fp::add(fp::add(smallest, wordField(kElY)), constant(kLastPixel)))));
            }
            else if (sreg32(screenHeight()) <= sreg32(bottomPixel()))
            {
                guest.setDword(element + kElBottom, screenHeight() - 1);
                const Float scaledRest = fp::mul(integerOf(screenHeight() - reg32(i16(kElY))), constant(kYToPercent));
                setWordTo(kElHeight, x87.truncated(fp::mul(scaledRest, perPixel)));
            }
        }
        const reg32 wide = screenWidth();
        if ((edges & 4) != 0)
        {
            // By its left edge.
            setWordTo(kElX, x87.truncated(fp::sub(mouseX(), dwordField(kElGrabX))));
            const Float scaled = fp::mul(integerOf(rightPixel() - reg32(i16(kElX)) + 1), constant(kXToPercent));
            const Float perPixel = oneOver(wide);
            const std::int32_t percent = x87.truncated(fp::mul(scaled, perPixel));
            setWordTo(kElWidth, percent);
            if (std::int16_t(percent) < kSmallestSide)
            {
                const Float smallest = fp::mul(integerOf(wide * 26), constant(kPercentToX));
                setWordTo(kElWidth, kSmallestSide);
                setWordTo(kElX, x87.truncated(fp::add(fp::sub(dwordField(kElRight), smallest), one)));
            }
            else if (i16(kElX) < 0)
            {
                const Float wanted = fp::mul(fp::mul(integerOf(rightPixel() + 1), constant(kXToPercent)), perPixel);
                setWordTo(kElX, 0);
                setWordTo(kElWidth, x87.truncated(wanted));
            }
            percentXNow();
        }
        else if ((edges & 8) != 0)
        {
            // By its right edge.
            guest.setDword(element + kElRight, reg32(x87.truncated(fp::add(mouseX(), dwordField(kElGrabX)))));
            const Float scaled = fp::mul(integerOf(rightPixel() - reg32(i16(kElX)) + 1), constant(kXToPercent));
            const Float perPixel = oneOver(wide);
            const std::int32_t percent = x87.truncated(fp::mul(scaled, perPixel));
            setWordTo(kElWidth, percent);
            if (std::int16_t(percent) < kSmallestSide)
            {
                const Float smallest = fp::mul(integerOf(wide * 26), constant(kPercentToX));
                setWordTo(kElWidth, kSmallestSide);
                guest.setDword(element + kElRight, reg32(x87.truncated(fp::add(fp::add(smallest, wordField(kElX)), constant(kLastPixel)))));
            }
            else if (sreg32(wide) <= sreg32(rightPixel()))
            {
                guest.setDword(element + kElRight, wide - 1);
                const Float scaledRest = fp::mul(integerOf(wide - reg32(i16(kElX))), constant(kXToPercent));
                setWordTo(kElWidth, x87.truncated(fp::mul(scaledRest, perPixel)));
            }
        }
    }
    // The game's code hands it the element in edx too, which it does not read.
    const reg32 moved = fe::direct<0x459db0, hudz::elementMoved>(app, cpu, element);
    guest.setDword(element + kElMoved, moved);
    if (moved == 0)
        return 0;
    copyDwords(guest, kDragStart, element + kElPercentY, 2);
    return element;
}

void frontend::hudEditorDrag(win32::WinApplication* app, x86::CPU& cpu)
{
    static Site site{ "sub_45a120", 0, 0, nullptr, 0, fe::kRestoreAll };
    const reg32 element = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return wata::hudEditorDrag(app, cpu, element); });
}

}
