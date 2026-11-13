#include "native_ea_memory.h"
#include "native_ea_net.h"
#include "native_ea_support.h"
#include "native_graphics.h"
#include "native_guest.h"
#include "native_load_cardb.h"
#include "native_load_carmodels.h"
#include "native_load_saves.h"
#include "native_vertices.h"
#include "native_windows.h"
#include "port_thrash.h"
#include <winapi/kernel32.h>

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <optional>
#include <utility>

/* The game's data: the cars' models and specs -- a car's carp.txt sent and
 * received over the network (the game's code from 0x49b540 to 0x49b9a9), the
 * cars' .fce models with their colours and their textures, from the .art,
 * the 00.tga or the car's .viv (0x49c700 to 0x49d596, 0x4b7a60, 0x4b7ba0),
 * and the players' own specs, carspecs\player1.spc and player2.spc (0x4dc3a0
 * to 0x4dcae1).  disassemble_nfs3hp.py binds each address to the function
 * of that name here (LOAD_CARMODELS_NATIVES), and the generated code calls it
 * by name.  tools/game_checks holds each against its generated code.
 *
 * A car's carp.txt goes over the network as a header of 0x1c bytes (0 its
 * kind, 1 the car's id of 16 bytes, 0x14 its record's +0x380, 0x18 the
 * text's size) and then its bytes; the receiver keeps a record of its own
 * (0x55e5bc: +0 done, +4 the size, +8 the buffer, +0xc what is in, +0x10 the
 * FILE it is written to, the data path's Carp.txt, when there is no buffer).
 * Whole, the text is put into the car's folder's car.viv with the others.
 *
 * A car's model (the .fce loaded, made ready by 0x49cfa0) holds its
 * pictures' count at +0xc; its colours, 16 bytes each (hue, saturation,
 * brightness in 8.8 parts of a turn, and a fourth passed along), [+0x7fc] of
 * them at +0x800, and as many second colours ([+0x900]) at +0x904; its
 * textures' handles at +0x1e04.  A texture is made of the pictures' 32-bit
 * pixels (red, green, blue, alpha) scaled to a power of two no larger than
 * the size asked for, as 1555 words in a block of the game's memory (+0 the
 * width, +4 the height, +8 the pixels); a pixel with the top bit of its
 * alpha clear takes the car's colour (0x49cc50), one not quite opaque the
 * second colour.  The .art holds the pictures one after the other (+0 the
 * width, +4 the height, +8 the pixels); a .tga one picture (its 18 bytes'
 * header, the width at +0xc, the height at +0xe, its id's length at +0).
 *
 * The players' specs: a table of 50 for each player (0x8b9200 the first's,
 * 0x8b88a0 the second's), 0x30 bytes each: +0 a byte, whether it is in use,
 * +4 the car's id of 16 bytes, +0x14 the car's spec of 0x1c bytes (0x55d024
 * the default).  The file is the table whole; one not of this machine
 * (0x472940, by the game's module's name) is deleted and made anew.
 *
 * Each is a typed function (its core) under a name of its own, with an adapter under the game's
 * name for the callers still on registers and the dispatch: it leaves eax, ebx, ecx, edx and the
 * x87's stack as the game's code does -- these functions keep few of them, and what a callee
 * leaves there (a stand-in may change what it is handed) is passed back up.  A core calls
 * another of this file's as C++, and the other typed files' their headers declare as C++ too
 * (guest::direct) -- but where the code passes on what such a callee leaves in a register
 * beyond its answer; every other function of the game's, and Windows' through its imports, is
 * called by address with the registers the code hands it, and Watcom's callers' registers
 * kept.  A frame whose addresses a callee is handed stays on the guest's stack at the
 * code's depth (GuestFrame), and so do the frames the colour functions read before writing.
 * Strings are copied a byte after another, blocks a dword then a byte at a time (Watcom's
 * inlined strcpy, strcat and memcpy).  The x87's operations are fp's, in the game's order, a float
 * kept where the game stores one; chopping is the game's own (0x4dfd56). */
namespace nfs3hp
{
namespace load
{

namespace
{

using guest::at;
using guest::byte;
using guest::put;
using guest::putByte;
using x86::reg16;
using x86::reg32;
using x86::reg8;
using x86::sreg32;
using App = win32::WinApplication;
using CPU = x86::CPU;
using i32 = std::int32_t;

// The network's carp.txt: the data path, the file it is written to, the
// receiver's record and what it takes next (0x12c bytes into the transfer's
// name), the .viv it is read from; the transfer's state's words.
constexpr reg32 kDataPath = 0x7a28b4, kCarpFile = 0x79faac, kReceiver = 0x55e5bc, kTransferName = 0x79f97c;
constexpr reg32 kNextBytes = 0x12c, kSendViv = 0x79f978, kTransferTitle = 0x79f5e0, kTransferWord = 0x79faa8;
// Its strings: "wb", "Carp.txt", "car.viv", "\carp.txt", "car.VIV".
constexpr reg32 kWriteMode = 0x53c590, kCarpName = 0x53c584, kCarVivName = 0x53c594, kCarpPath = 0x53c59c;
constexpr reg32 kCarVivPath = 0x53c5a8;
// The models': an empty name (the blocks' tag), "%s.viv", "car.fce", the
// trace's file and format, "%s.fce", "%s00.tga", "%s.art"; the .viv's ones.
constexpr reg32 kNoName = 0x53c60c, kVivFormat = 0x53c62c, kFceName = 0x53c634, kTraceFile = 0x53c63c;
constexpr reg32 kTraceFormat = 0x53c658, kFceFormat = 0x53c670, kTgaFormat = 0x53c678, kArtFormat = 0x53c684;
constexpr reg32 kVivFceFormat = 0x53ff70, kVivNoName = 0x53ff78, kVivTgaFormat = 0x53ff7c;
// Their numbers: 6 (a double, the hue's sixths), 255 (a double); the two
// records of zeros nfs3.exe keeps among its code (0x4b7a10, 0x4b7a20).
constexpr reg32 kSix = 0x53c660, kFull = 0x53c668;
constexpr reg32 kZeros = 0x4b7a10, kMoreZeros = 0x4b7a20;
// Whether a pixel all black but its top bit is drawn as 0x8001.
constexpr reg32 kBlackVisible = 0x7a3a6c;
// The specs: the players' tables, their names, the default spec; the path
// made, the folder before it, "carspecs\", ".spc".
constexpr reg32 kFirstSpecs = 0x8b9200, kSecondSpecs = 0x8b88a0, kSpecsBytes = 0x960, kSpecBytes = 0x30;
constexpr reg32 kSpecCount = 0x32, kFirstPlayer = 0x549784, kSecondPlayer = 0x54978c, kDefaultSpec = 0x55d024;
constexpr reg32 kSpecPath = 0x564188, kSpecFolder = 0x7a2fb4, kSpecsFolder = 0x549770, kSpecExtension = 0x54977c;
// What is called by address: the paths' backslash, a file copied, the
// .viv's name the new car goes into; fopen, fclose, fwrite, mkdir, strrchr;
// sprintf; a file found in a .viv (where what it leaves in ecx is passed
// on); fseek, fread; memmove (where what it leaves is); a model made ready;
// a texture allocated, its pixels given (where what they leave is); vectors
// less another (what it leaves in edx is); __CHP; memset; a player's name
// valid; GetModuleFileNameA; unlink, a file's size, access, chmod.
constexpr reg32 kEndPath = 0x4529a0, kCopyFile = 0x452a00, kNewCarName = 0x43ba20, kSprintf = 0x4df690;
constexpr reg32 kFopen = 0x4edfe8, kFclose = 0x4ee100, kFwrite = 0x4eebd0, kMkdir = 0x4eb1d0, kStrrchr = 0x4ee2f0;
constexpr reg32 kFindInViv = 0x43b9d0, kFseek = 0x4e9088, kFread = 0x4e9200, kMoveMemory = 0x4ea4f0;
constexpr reg32 kModelReady = 0x49cfa0, kAllocTexture = 0x49c600, kUpdateTexture = 0x49c620, kSubtract = 0x4e0000;
constexpr reg32 kChop = 0x4dfd56, kMemset = 0x4e0640, kNameValid = 0x4721f0, kAccess = 0x4f05f0, kChmod = 0x4ee3c0;
constexpr reg32 kGetModuleFileName = 0x534544, kUnlink = 0x4e28a0, kFileSize = 0x4e43a0;
// The rest are the other typed files' (native_load_cardb.h, native_load_saves.h,
// native_ea_support.h, native_ea_memory.h, native_ea_net.h, native_graphics.h,
// native_vertices.h), called as C++ (guest::direct): the car's folder marked, a car's record, id,
// folder's name, name, its index by id, the cars' data, a file put into a .viv, the transfer's
// state; a .viv opened, a file found in it, closed; a packet sent; the game's trace; the game's
// memory, memmove; a file loaded, there, loaded quietly; a texture allocated; a vector's length;
// a file's place in a .viv in memory; a spec file of this machine's; a file saved.

bool byAddress(win32::WinApplication* app, x86::CPU& cpu, reg32 target)
{
    cpu.esp -= 4;
    app->dynamic_call(target, cpu);
    return !cpu.terminate;
}

reg16 half(win32::WinApplication* app, reg32 address)
{
    return app->getMemory<reg16>(address);
}

void putHalf(win32::WinApplication* app, reg32 address, reg16 value)
{
    app->getMemory<reg16>(address) = value;
}

x86::Float f64(win32::WinApplication* app, reg32 address)
{
    return x86::Float(app->getMemory<double>(address));
}

void putF32(win32::WinApplication* app, reg32 address, const x86::Float& value)
{
    app->getMemory<float>(address) = float(value);
}

/* Which table of specs player's is: the first's, the second's, or none. */
reg32 specsOf(reg32 player)
{
    return player == 0 ? kFirstSpecs : player == 1 ? kSecondSpecs : 0;
}

/* The registers but eax and esp, kept across what the scope does: what Watcom's callers count
 * on of a callee, and what an adapter's code pops on leaving. */
class KeptRegisters
{
public:
    explicit KeptRegisters(CPU& cpu)
        : cpu_(cpu), ebx_(cpu.ebx), ecx_(cpu.ecx), edx_(cpu.edx), esi_(cpu.esi), edi_(cpu.edi), ebp_(cpu.ebp)
    {
    }
    ~KeptRegisters()
    {
        cpu_.ebx = ebx_;
        cpu_.ecx = ecx_;
        cpu_.edx = edx_;
        cpu_.esi = esi_;
        cpu_.edi = edi_;
        cpu_.ebp = ebp_;
    }

private:
    CPU& cpu_;
    reg32 ebx_, ecx_, edx_, esi_, edi_, ebp_;
};

/* Room on the guest's stack for what a callee is handed the address of: the game's frame
 * (`pushed` bytes of registers under esp on entry, then `bytes`), esp under it while the core
 * runs and back after. */
class GuestFrame
{
public:
    GuestFrame(CPU& cpu, reg32 pushed, reg32 bytes) : cpu(cpu), esp(cpu.esp), base(cpu.esp - pushed)
    {
        cpu.esp = base - bytes;
    }
    ~GuestFrame() { cpu.esp = esp; }
    reg32 operator-(reg32 offset) const { return base - offset; }

private:
    CPU& cpu;
    const reg32 esp;
    const reg32 base;
};

// A spec's fields: in use (a byte), its car's id (16 bytes), its car's spec (0x1c bytes).
constexpr reg32 kSpecInUse = 0, kSpecCarId = 4, kSpecCar = 0x14, kCarIdBytes = 0x10, kCarSpecBytes = 0x1c;
// The return address's slot on the guest's stack.
constexpr reg32 kReturnSlot = 4;

/* A function of the game's called by address, Watcom's way: eax, edx, ebx and ecx as given, in
 * that order, those left out as they are; every register but eax kept.  What it gives back in
 * eax, or nullopt when the game stopped in it. */
std::optional<reg32> callGame(App* app, CPU& cpu, reg32 target, std::initializer_list<reg32> registers)
{
    const KeptRegisters kept(cpu);
    reg32* const slots[] = {&cpu.eax, &cpu.edx, &cpu.ebx, &cpu.ecx};
    std::size_t n = 0;
    for (const reg32 value : registers)
        *slots[n++] = value;
    if (!byAddress(app, cpu, target))
        return std::nullopt;
    return cpu.eax;
}

/* 0x4e0640: memset of `count` bytes at `to` to 0, as the code calls it (it gives `to` back and
 * keeps every other register); false when the game stopped. */
bool clearBytes(App* app, CPU& cpu, reg32 to, reg32 count)
{
    return callGame(app, cpu, kMemset, {to, 0, count}).has_value();
}

/* 0x439a50: car `car`'s record, or 0 past the last. */
std::optional<reg32> carRecord(App* app, CPU& cpu, i32 car)
{
    const reg32 record = guest::direct<0x439a50, recordAt>(app, cpu, reg32(car));
    if (cpu.terminate)
        return std::nullopt;
    return record;
}

/* 0x439520: car `car`'s id, or 0. */
std::optional<reg32> carId(App* app, CPU& cpu, i32 car)
{
    return guest::direct<0x439520, idOf>(app, cpu, reg32(car));
}

/* rep movsd, then rep movsb: `count` bytes from `from` to `to`, a dword at a time and the last
 * up to three a byte at a time. */
void copyGuest(App* app, reg32 to, reg32 from, reg32 count)
{
    for (reg32 n = count >> 2; n != 0; --n, to += 4, from += 4)
        put(app, to, at(app, from));
    for (reg32 n = count & 3; n != 0; --n)
        putByte(app, to++, byte(app, from++));
}

/* Watcom's inlined strcpy: the string at `from` to `to`, its 0 too, a byte after another. */
void copyText(App* app, reg32 to, reg32 from)
{
    for (;;)
    {
        const reg8 c = byte(app, from++);
        putByte(app, to++, c);
        if (c == 0)
            return;
    }
}

/* Watcom's inlined strcat: the string at `from` after the one at `to`. */
void appendText(App* app, reg32 to, reg32 from)
{
    while (byte(app, to) != 0)
        ++to;
    copyText(app, to, from);
}

}  // namespace

void beginCarpReceive(win32::WinApplication* app, x86::CPU& cpu);
void receiveCarpChunk(win32::WinApplication* app, x86::CPU& cpu);
void beginCarpSend(win32::WinApplication* app, x86::CPU& cpu);
void readCarColors(win32::WinApplication* app, x86::CPU& cpu);
void rgbToHsv(win32::WinApplication* app, x86::CPU& cpu);
void hsvToRgb(win32::WinApplication* app, x86::CPU& cpu);
void tintPixel(win32::WinApplication* app, x86::CPU& cpu);
void scaleTexture(win32::WinApplication* app, x86::CPU& cpu);
void fitTexture(win32::WinApplication* app, x86::CPU& cpu);
void loadArtTextures(win32::WinApplication* app, x86::CPU& cpu);
void nearestModelColor(win32::WinApplication* app, x86::CPU& cpu);
void loadTgaTexture(win32::WinApplication* app, x86::CPU& cpu);
void loadCarModel(win32::WinApplication* app, x86::CPU& cpu);
void loadVivCarModel(win32::WinApplication* app, x86::CPU& cpu);
void copyVivCarModel(win32::WinApplication* app, x86::CPU& cpu);
void setSpecUsed(win32::WinApplication* app, x86::CPU& cpu);
void specUsed(win32::WinApplication* app, x86::CPU& cpu);
void specCarName(win32::WinApplication* app, x86::CPU& cpu);
void findSpec(win32::WinApplication* app, x86::CPU& cpu);
void freeSpecSlot(win32::WinApplication* app, x86::CPU& cpu);
void setSpec(win32::WinApplication* app, x86::CPU& cpu);
void defaultSpecs(win32::WinApplication* app, x86::CPU& cpu);
void refreshSpecs(win32::WinApplication* app, x86::CPU& cpu);
void specPath(win32::WinApplication* app, x86::CPU& cpu);
void readSpecs(win32::WinApplication* app, x86::CPU& cpu);
void loadPlayerSpecs(win32::WinApplication* app, x86::CPU& cpu);
void savePlayerSpecs(win32::WinApplication* app, x86::CPU& cpu);

namespace
{

// The receiver's record: done, the size awaited, the buffer, what is in, the FILE written.
constexpr reg32 kReceiverDone = 0, kReceiverSize = 4, kReceiverBuffer = 8, kReceiverIn = 0xc, kReceiverFile = 0x10;

/* 0x49b540's core: the receiver's record made ready for `bytes` bytes: into `buffer`, or with
 * none into the data path's Carp.txt, opened to write (0x4edfe8).  0, or -1 when it would not
 * open; nullopt when the game stopped. */
std::optional<reg32> beginReceive(App* app, CPU& cpu, reg32 record, reg32 bytes, reg32 buffer)
{
    put(app, record + kReceiverIn, 0);
    put(app, record + kReceiverDone, 0);
    put(app, record + kReceiverSize, bytes);
    if (buffer != 0)
    {
        put(app, record + kReceiverFile, 0);
        put(app, record + kReceiverBuffer, buffer);
        return 0;
    }
    copyText(app, kCarpFile, kDataPath);
    if (!callGame(app, cpu, kEndPath, {kCarpFile}))
        return std::nullopt;
    appendText(app, kCarpFile, kCarpName);
    const std::optional<reg32> file = callGame(app, cpu, kFopen, {kCarpFile, kWriteMode});
    if (!file)
        return std::nullopt;
    put(app, record + kReceiverFile, *file);
    return *file == 0 ? 0xffffffff : 0;
}

// receiveCarpChunk's frame: four registers pushed, then 0x260 bytes -- the car's folder at -0x134
// and its carp.txt's path at -0x260, both handed to the functions it calls.
constexpr reg32 kChunkPushed = 0x10, kChunkFrame = 0x260, kChunkFolder = 0x134, kChunkCarp = 0x260;
constexpr reg32 kBackslash = 0x5c;

/* 0x49b5f0's core: `count` bytes at `data` taken by the receiver's record, unless it is done:
 * written to its file (fwrite; failing, the file closed, the record done, -1) or copied into its
 * buffer.  All in, the file closed and the receiver made ready for the next 0x12c bytes, into
 * the transfer's name; or, from the buffer, the record done and the car put together in its
 * folder (the data path's, made): its carp.txt copied in, car.viv made of it and the others, the
 * cars' data and the transfer's state brought up to date.  0; nullopt when the game stopped. */
std::optional<reg32> receiveChunk(App* app, CPU& cpu, reg32 record, reg32 data, reg32 count)
{
    if (at(app, record + kReceiverDone) != 0)
        return 0;
    const GuestFrame frame(cpu, kChunkPushed, kChunkFrame);
    const reg32 folder = frame - kChunkFolder, carp = frame - kChunkCarp;
    const reg32 file = at(app, record + kReceiverFile);
    if (file != 0)
    {
        const std::optional<reg32> written = callGame(app, cpu, kFwrite, {data, count, 1, file});
        if (!written)
            return std::nullopt;
        if (*written == 0)
        {
            if (!callGame(app, cpu, kFclose, {at(app, record + kReceiverFile)}))
                return std::nullopt;
            put(app, record + kReceiverFile, 0);
            put(app, record + kReceiverDone, 1);
            return 0xffffffff;
        }
    }
    else
        copyGuest(app, at(app, record + kReceiverBuffer) + at(app, record + kReceiverIn), data, count);
    const reg32 in = at(app, record + kReceiverIn) + count;
    put(app, record + kReceiverIn, in);
    if (in != at(app, record + kReceiverSize))
        return 0;
    const reg32 written = at(app, record + kReceiverFile);
    if (written != 0)
    {
        if (!callGame(app, cpu, kFclose, {written}))
            return std::nullopt;
        put(app, record + kReceiverFile, 0);
        if (!beginReceive(app, cpu, kReceiver, kNextBytes, kTransferName))
            return std::nullopt;
        return 0;
    }
    put(app, record + kReceiverDone, 1);
    copyText(app, folder, kDataPath);
    if (!callGame(app, cpu, kEndPath, {folder, kBackslash}))
        return std::nullopt;
    const std::optional<reg32> last = callGame(app, cpu, kStrrchr, {folder, kBackslash});
    if (!last || !guest::direct<0x4385a0, freePlayerFolder>(app, cpu, *last + 1) || !callGame(app, cpu, kMkdir, {folder})
        || !callGame(app, cpu, kEndPath, {folder, kCarpFile}))
        return std::nullopt;
    appendText(app, folder, kCarpName);
    if (!callGame(app, cpu, kCopyFile, {folder, kCarpFile}))
        return std::nullopt;
    copyText(app, carp, folder);
    const std::optional<reg32> carpName = callGame(app, cpu, kStrrchr, {carp, kBackslash});
    if (!carpName)
        return std::nullopt;
    copyText(app, *carpName + 1, kCarVivName);
    // eax as the inlined copy leaves it, the last byte copied (0) in al: the name's stand-in may
    // give it back.
    const std::optional<reg32> viv = callGame(app, cpu, kNewCarName, {*carpName & ~0xffu});
    if (!viv || !callGame(app, cpu, kCopyFile, {carp, *viv}) || !guest::direct<0x43baa0, appendCarp>(app, cpu, carp)
        || !guest::direct<0x439d90, loadCars>(app, cpu))
        return std::nullopt;
    guest::direct<0x43bc00, setLastId>(app, cpu, kTransferTitle);
    if (cpu.terminate || !guest::direct<0x43bc40, nameLastCar>(app, cpu, kTransferName))
        return std::nullopt;
    guest::direct<0x43bc70, setLastFlags>(app, cpu, at(app, kTransferWord));
    if (cpu.terminate)
        return std::nullopt;
    guest::direct<0x43bc90, markLastAfterOthers>(app, cpu);
    if (cpu.terminate || !guest::direct<0x439d90, loadCars>(app, cpu))
        return std::nullopt;
    return 0;
}

}  // namespace

/* 0x49b540: beginReceive for the record eax, ebx bytes, the buffer edx; edx the record when
 * there is no buffer. */
void beginCarpReceive(App* app, CPU& cpu)
{
    const reg32 record = cpu.eax, buffer = cpu.edx;
    const std::optional<reg32> result = beginReceive(app, cpu, record, cpu.ebx, buffer);
    if (!result)
        return;
    cpu.eax = *result;
    if (buffer == 0)
        cpu.edx = record;
    cpu.esp += 4;
}

/* 0x49b5f0: receiveChunk for the record eax, the ebx bytes at edx. */
void receiveCarpChunk(App* app, CPU& cpu)
{
    const std::optional<reg32> result = receiveChunk(app, cpu, cpu.eax, cpu.edx, cpu.ebx);
    if (!result)
        return;
    cpu.eax = *result;
    cpu.esp += 4;
}

namespace
{

// beginCarpSend's frame: five registers pushed, then 0x270 bytes: the text's size at -0x10, the
// .viv's FILE at -0x14, the text's place at -0x18, the .viv's path at -0x144, the car's
// carp.txt's path at -0x270 -- each handed to a function it calls.
constexpr reg32 kSendPushed = 0x14, kSendFrame = 0x270, kSendSize = 0x10, kSendFile = 0x14, kSendPlace = 0x18;
constexpr reg32 kSendVivPath = 0x144, kSendPath = 0x270;
// The header sent: the car's id at +1, its record's +0x380 at +0x14, the text's size at +0x18;
// 0x1c bytes in all.
constexpr reg32 kHeaderCarId = 1, kHeaderCarWord = 0x14, kHeaderSize = 0x18, kHeaderBytes = 0x1c;
constexpr reg32 kCarRecordWord = 0x380;

/* 0x4df690: sprintf (cdecl), its arguments as given; false when the game stopped. */
bool print(App* app, CPU& cpu, std::initializer_list<reg32> arguments)
{
    const KeptRegisters kept(cpu);
    return guest::call(app, cpu, kSprintf, arguments);
}

/* 0x4ea290: the file named `name` in the .viv at `viv` found, its place to [place] and its size
 * to [size]; false when the game stopped. */
bool findVivEntry(App* app, CPU& cpu, reg32 viv, reg32 name, reg32 place, reg32 size)
{
    return guest::direct<0x4ea290, locatedEntry>(app, cpu, viv, name, reg32(0), place, size).has_value();
}

/* 0x4ea4f0: `count` bytes from `from` to `to`, as the code calls it; it may leave eax, edx and ebx
 * changed: what it leaves in ebx to `countLeft`.  False when the game stopped. */
bool moveMemory(App* app, CPU& cpu, reg32 from, reg32 to, reg32 count, reg32& countLeft)
{
    const KeptRegisters kept(cpu);
    cpu.eax = from;
    cpu.edx = to;
    cpu.ebx = count;
    if (!byAddress(app, cpu, kMoveMemory))
        return false;
    countLeft = cpu.ebx;
    return true;
}

/* 0x49b800's core: car's carp.txt made ready to send: the header (+1 the car's id, +0x14 its
 * record's +0x380, +0x18 the text's size) filled in from the car's folder's car.VIV, opened (its
 * handle to 0x79f978) and its FILE put at the text; the car's name to the transfer's name, and
 * the header's 0x1c bytes sent to every machine (0x4a60e0, which may change edx: what it leaves
 * there to `packetLeft`).  The FILE; nullopt when the game stopped. */
std::optional<reg32> beginSend(App* app, CPU& cpu, reg32 header, reg32 car, reg32& packetLeft)
{
    const GuestFrame frame(cpu, kSendPushed, kSendFrame);
    const reg32 size = frame - kSendSize, file = frame - kSendFile, place = frame - kSendPlace;
    const reg32 viv = frame - kSendVivPath, path = frame - kSendPath;
    const std::optional<reg32> record = carRecord(app, cpu, i32(car));
    if (!record)
        return std::nullopt;
    const std::optional<reg32> id = carId(app, cpu, i32(car));
    if (!id)
        return std::nullopt;
    copyGuest(app, header + kHeaderCarId, *id, kCarIdBytes);
    copyText(app, path, kDataPath);
    if (!callGame(app, cpu, kEndPath, {path}))
        return std::nullopt;
    const std::optional<reg32> folder = guest::direct<0x4392b0, folderOf>(app, cpu, car);
    if (!folder)
        return std::nullopt;
    appendText(app, path, *folder);
    appendText(app, path, kCarpPath);
    copyText(app, viv, path);
    const std::optional<reg32> vivName = callGame(app, cpu, kStrrchr, {viv, kBackslash});
    if (!vivName || !print(app, cpu, {*vivName + 1, kCarVivPath}))
        return std::nullopt;
    const std::optional<reg32> opened = guest::direct<0x43b830, ea::vivOpened>(app, cpu, viv, file);
    if (!opened)
        return std::nullopt;
    put(app, kSendViv, *opened);
    const std::optional<reg32> carpName = callGame(app, cpu, kStrrchr, {path, kBackslash});
    if (!carpName || !guest::direct<0x43b9d0, ea::vivFileFound>(app, cpu, at(app, kSendViv), *carpName + 1, place, size)
        || !callGame(app, cpu, kFseek, {at(app, file), at(app, place), 0}))
        return std::nullopt;
    put(app, header + kHeaderSize, at(app, size));
    put(app, header + kHeaderCarWord, at(app, *record + kCarRecordWord));
    const std::optional<reg32> name = guest::direct<0x439250, nameOf>(app, cpu, car);
    if (!name)
        return std::nullopt;
    copyText(app, kTransferName, *name);
    const std::optional<ea::Regs> sent = guest::direct<0x4a60e0, ea::packetSent>(app, cpu, reg32(0), header, kHeaderBytes);
    if (!sent)
        return std::nullopt;
    packetLeft = sent->edx;
    return at(app, file);
}

}  // namespace

/* 0x49b800: beginSend for the header eax, car edx; edx as the packet's sending leaves it. */
void beginCarpSend(App* app, CPU& cpu)
{
    reg32 packetLeft = cpu.edx;
    const std::optional<reg32> file = beginSend(app, cpu, cpu.eax, cpu.edx, packetLeft);
    if (!file)
        return;
    cpu.eax = *file;
    cpu.edx = packetLeft;
    cpu.esp += 4;
}

namespace
{

// A colour as red, green, blue and a fourth, and as hue, saturation, brightness and the fourth:
// a dword each.
constexpr reg32 kRed = 0, kGreen = 4, kBlue = 8, kFourth = 0xc;
constexpr reg32 kHue = 0, kSaturation = 4, kBrightness = 8;
// The hue's sixths in 16.16: green's third and blue's; the hue's sixths to a turn; 8.8 parts.
constexpr i32 kGreenThird = 0x20000, kBlueThird = 0x40000, kSixths = 6, kTurn = 0x100, kTop = 0xff;

/* cdq; idiv: `dividend` over `divisor`, the quotient and the remainder -- worked in 64 bits as
 * the generated code does, the quotient's low half. */
std::pair<reg32, reg32> divide(x86::sreg64 dividend, reg32 divisor)
{
    const x86::sreg64 by = sreg32(divisor);
    return {reg32(x86::reg64(dividend / by)), reg32(x86::reg64(dividend % by))};
}

/* `a` over `b` in 16.16: the whole part, then the remainder's (cdq; idiv; shrd; sar; idiv). */
reg32 fixedOver(reg32 a, reg32 b)
{
    const auto [whole, rest] = divide(sreg32(a), b);
    const x86::sreg64 shifted = x86::sreg64((x86::reg64(reg32(sreg32(rest) >> 16)) << 32) | reg32(rest << 16));
    return (whole << 16) + divide(shifted, b).first;
}

/* `a` times `b` in 16.16, rounded by the bit shifted out (imul; shl; shr; adc). */
reg32 fixedTimes(reg32 a, reg32 b)
{
    const x86::sreg64 product = x86::sreg64(sreg32(a)) * sreg32(b);
    const reg32 low = reg32(x86::reg64(product));
    return (low >> 16) + (reg32(x86::reg64(product) >> 32) << 16) + ((low >> 15) & 1);
}

/* `value` held to 0 to 255. */
reg32 toByte(reg32 value)
{
    return sreg32(value) < 0 ? 0 : sreg32(value) > kTop ? reg32(kTop) : value;
}

/* What 0x49c880 leaves in eax and edx: the brightness stored, and the brightest of the three. */
struct HsvLeft
{
    reg32 brightness, brightest;
};

/* 0x49c880's core: the colour at `rgb` -- red, green, blue, 0 to 255, and a fourth -- as hue,
 * saturation and brightness to `hsv`: the hue in 8.8 parts of a turn (its sixth from which of
 * the three is largest), the others 0 to 255, each held to 0 to 255; the fourth as it is.  A hue
 * below 0 has its high word counted on (inc word [hsv + 2]), as the code does.  The colour is
 * read again where the code reads it, after the first writes: the two may be one. */
HsvLeft rgbToHsvAt(App* app, reg32 rgb, reg32 hsv)
{
    const reg32 red = at(app, rgb + kRed), green = at(app, rgb + kGreen), blue = at(app, rgb + kBlue);
    const reg32 greenBlueMost = sreg32(blue) < sreg32(green) ? green : blue;
    const reg32 most = sreg32(greenBlueMost) < sreg32(red) ? red : greenBlueMost;
    const reg32 greenBlueLeast = sreg32(green) < sreg32(blue) ? green : blue;
    const reg32 least = sreg32(greenBlueLeast) > sreg32(red) ? red : greenBlueLeast;
    const reg32 fourth = at(app, rgb + kFourth);
    put(app, hsv + kHue, 0);
    put(app, hsv + kSaturation, 0);
    put(app, hsv + kFourth, fourth);
    const reg32 range = most - least;
    put(app, hsv + kBrightness, most);
    if (most != 0 && range != 0)
    {
        put(app, hsv + kSaturation, divide(sreg32(range << 8), most).first);
        reg32 hue;
        if (most == at(app, rgb + kRed))
            hue = fixedOver(at(app, rgb + kGreen) - at(app, rgb + kBlue), range);
        else if (const reg32 g = at(app, rgb + kGreen); most == g)
            hue = fixedOver(at(app, rgb + kBlue) - at(app, rgb + kRed), range) + kGreenThird;
        else
            hue = fixedOver(at(app, rgb + kRed) - g, range) + kBlueThird;
        put(app, hsv + kHue, hue);
        const reg32 sixth = divide(sreg32(at(app, hsv + kHue)), kSixths).first;
        put(app, hsv + kHue, sixth);
        if (sreg32(sixth) < 0)
            putHalf(app, hsv + kHue + 2, reg16(half(app, hsv + kHue + 2) + 1));
    }
    put(app, hsv + kHue, fixedTimes(kTurn, at(app, hsv + kHue)));
    put(app, hsv + kHue, toByte(at(app, hsv + kHue)));
    put(app, hsv + kSaturation, toByte(at(app, hsv + kSaturation)));
    const reg32 brightest = at(app, hsv + kBrightness);
    const reg32 brightness = toByte(brightest);
    put(app, hsv + kBrightness, brightness);
    return {brightness, brightest};
}

}  // namespace

/* 0x49c880: rgbToHsvAt for the colour at eax, to edx; eax the brightness stored, edx the brightest
 * of the three before it was held to 255. */
void rgbToHsv(App* app, CPU& cpu)
{
    const HsvLeft left = rgbToHsvAt(app, cpu.eax, cpu.edx);
    cpu.eax = left.brightness;
    cpu.edx = left.brightest;
    cpu.esp += 4;
}

namespace
{

/* 0x4dfd56 (__CHP): `value` chopped toward 0 by the game's own, on the x87's stack, and stored as
 * a dword (fistp); nullopt when the game stopped. */
std::optional<reg32> chopToInteger(App* app, CPU& cpu, const x86::Float& value)
{
    const KeptRegisters kept(cpu);
    cpu.fpu.push(value);
    if (!byAddress(app, cpu, kChop))
        return std::nullopt;
    const reg32 chopped = reg32(fp::toInteger<sreg32>(cpu.fpu.st(0)));
    cpu.fpu.pop();
    return chopped;
}

/* 0x49ca50's core: the colour at `hsv` as hue, saturation and brightness to `rgb` as red, green
 * and blue, each held to 0 to 255, and the fourth as it is: none bright, black; none saturated,
 * grey; else by the hue's sixth (the x87's (hue * 6) less its whole sixths, chopped), the
 * switch's six cases (nfs3.exe's table at 0x49ca34) -- a sixth out of them leaves the three as
 * they were before the holding.  The colour is read again where the code reads it, after the
 * first writes: the two may be one.  What it leaves in eax and edx: the blue stored, and the
 * blue before it was held; nullopt when the game stopped. */
std::optional<HsvLeft> hsvToRgbAt(App* app, CPU& cpu, reg32 hsv, reg32 rgb)
{
    put(app, rgb + kFourth, at(app, hsv + kFourth));
    const reg32 brightness = at(app, hsv + kBrightness);
    if (sreg32(brightness) <= 0)
    {
        put(app, rgb + kGreen, 0);
        put(app, rgb + kBlue, 0);
        put(app, rgb + kRed, 0);
    }
    else if (sreg32(at(app, hsv + kSaturation)) <= 0)
    {
        put(app, rgb + kRed, brightness);
        put(app, rgb + kGreen, brightness);
        put(app, rgb + kBlue, brightness);
    }
    else
    {
        const reg32 hue = at(app, hsv + kHue);
        const reg32 sixth = reg32(sreg32(hue * reg32(kSixths)) >> 8);
        const std::optional<reg32> fraction = chopToInteger(
            app, cpu, fp::sub(fp::mul(x86::Float(sreg32(hue)), f64(app, kSix)), x86::Float(sreg32(sixth << 8))));
        if (!fraction)
            return std::nullopt;
        const reg32 part = fixedOver(*fraction, kTurn);
        const reg32 saturation = at(app, hsv + kSaturation);
        const reg32 low = reg32(sreg32(at(app, hsv + kBrightness) * (kTurn - saturation)) >> 8);
        const reg32 falling = reg32(sreg32((kTurn - fixedTimes(saturation, part)) * at(app, hsv + kBrightness)) >> 8);
        const reg32 full = at(app, hsv + kBrightness);
        const reg32 rising = reg32(sreg32((kTurn - fixedTimes(saturation, 0x10000 - part)) * full) >> 8);
        switch (sixth)
        {
        case 0:
            put(app, rgb + kRed, at(app, hsv + kBrightness));
            put(app, rgb + kGreen, rising);
            put(app, rgb + kBlue, low);
            break;
        case 1:
            put(app, rgb + kRed, falling);
            put(app, rgb + kGreen, at(app, hsv + kBrightness));
            put(app, rgb + kBlue, low);
            break;
        case 2:
            put(app, rgb + kRed, low);
            put(app, rgb + kGreen, at(app, hsv + kBrightness));
            put(app, rgb + kBlue, rising);
            break;
        case 3:
            put(app, rgb + kRed, low);
            put(app, rgb + kGreen, falling);
            put(app, rgb + kBlue, at(app, hsv + kBrightness));
            break;
        case 4:
            put(app, rgb + kGreen, low);
            put(app, rgb + kRed, rising);
            put(app, rgb + kBlue, at(app, hsv + kBrightness));
            break;
        case 5:
            put(app, rgb + kRed, full);
            put(app, rgb + kGreen, low);
            put(app, rgb + kBlue, falling);
            break;
        default:
            break;
        }
    }
    put(app, rgb + kRed, toByte(at(app, rgb + kRed)));
    put(app, rgb + kGreen, toByte(at(app, rgb + kGreen)));
    const reg32 blue = at(app, rgb + kBlue);
    const reg32 held = toByte(blue);
    put(app, rgb + kBlue, held);
    return HsvLeft{held, blue};
}

}  // namespace

/* 0x49ca50: hsvToRgbAt for the colour at eax, to edx; eax the blue stored, edx the blue before it
 * was held to 255. */
void hsvToRgb(App* app, CPU& cpu)
{
    const std::optional<HsvLeft> left = hsvToRgbAt(app, cpu, cpu.eax, cpu.edx);
    if (!left)
        return;
    cpu.eax = left->brightness;
    cpu.edx = left->brightest;
    cpu.esp += 4;
}

namespace
{

// tintPixel's frame: two registers pushed, then 0x30 bytes: the colour asked as hue, saturation,
// brightness at -0x20 and as red, green, blue at -0x30, which hsvToRgbAt works in.
constexpr reg32 kTintPushed = 8, kTintFrame = 0x30, kTintHsv = 0x20, kTintRgb = 0x30;
// A 1555 pixel: its top bit, and a channel's five bits at 10 (red), 5 (green) and 0 (blue).
constexpr reg32 kPixelTop = 0x8000, kPixelBlueBits = 0xff;

/* What 0x49cc50 leaves: the pixel in eax, and in edx its red and green with the top bit. */
struct TintLeft
{
    reg32 pixel, redGreen;
};

/* 0x49cc50's core: the 1555 pixel in the colour at `color` (hue, saturation, brightness, the
 * fourth): 0 unless its top bit is set; else its blue (as a brightness, over 255 less the
 * colour's fourth unless that is 255) times the colour's brightness, held to 255, makes the
 * colour's red, green and blue (hsvToRgbAt, in a frame at the code's depth), 1555 again with the
 * top bit.  nullopt when the game stopped. */
std::optional<TintLeft> tint(App* app, CPU& cpu, reg32 pixel, reg32 color)
{
    if ((pixel & kPixelTop) == 0)
        return TintLeft{0, 0};
    const GuestFrame frame(cpu, kTintPushed, kTintFrame);
    const reg32 hsv = frame - kTintHsv, rgb = frame - kTintRgb;
    const reg32 blue = ((pixel & 0xffff) << 3) & kPixelBlueBits;
    float scale = float(x86::Float(sreg32(blue)));
    if (at(app, color + kFourth) != kTop)
        scale = float(fp::div(x86::Float(scale),
                              fp::sub(f64(app, kFull), x86::Float(sreg32(at(app, color + kFourth))))));
    put(app, hsv + kHue, at(app, color + kHue));
    put(app, hsv + kSaturation, at(app, color + kSaturation));
    const std::optional<reg32> brightness =
        chopToInteger(app, cpu, fp::mul(x86::Float(sreg32(at(app, color + kBrightness))), x86::Float(scale)));
    if (!brightness)
        return std::nullopt;
    put(app, hsv + kBrightness, *brightness);
    if (sreg32(at(app, hsv + kBrightness)) > kTop)
        put(app, hsv + kBrightness, kTop);
    put(app, hsv + kFourth, at(app, color + kFourth));
    if (!hsvToRgbAt(app, cpu, hsv, rgb))
        return std::nullopt;
    const reg32 redGreen = ((reg32(sreg32(at(app, rgb + kRed)) >> 3) << 10) | kPixelTop)
                           | (reg32(sreg32(at(app, rgb + kGreen)) >> 3) << 5);
    return TintLeft{reg32(sreg32(at(app, rgb + kBlue)) >> 3) | redGreen, redGreen};
}

// nearestModelColor's frame: five registers pushed, then 0x44 bytes: a model's colour as floats
// at -0x1c, the one asked at -0x28, their difference at -0x34 (each handed to 0x4e0000 and
// 0x4e06a0), red, green and blue at -0x44 (hsvToRgbAt's).
constexpr reg32 kNearestPushed = 0x14, kNearestFrame = 0x44, kNearestColor = 0x1c, kNearestWanted = 0x28;
constexpr reg32 kNearestDifference = 0x34, kNearestRgb = 0x44;
// A model's colours' count (+0x7fc) and its colours (+0x800); its second colours' count (+0x900)
// and its second colours (+0x904); 16 bytes a colour.
constexpr reg32 kModelColors = 0x7fc, kModelColor = 0x800, kModelSeconds = 0x900, kModelSecond = 0x904;
constexpr reg32 kColorShift = 4;
// The distance to beat at first (the code's float bits 0x447a0000).
constexpr float kStartDistance = 1000.0f;

/* What 0x49d240 leaves: the index in eax, and edx as the code leaves it -- the vectors'
 * subtraction's (0x4e0000) when there was a colour to try, else the first conversion's. */
struct NearestLeft
{
    reg32 index, left;
};

/* Red, green and blue at `rgb` to floats at `to`, each a float of the dword (fild; fstp). */
void rgbToFloats(App* app, reg32 rgb, reg32 to)
{
    putF32(app, to, x86::Float(sreg32(at(app, rgb + kRed))));
    putF32(app, to + 4, x86::Float(sreg32(at(app, rgb + kGreen))));
    putF32(app, to + 8, x86::Float(sreg32(at(app, rgb + kBlue))));
}

/* 0x49d240's core: which of the model's colours (as many as it has of both kinds) is nearest the
 * colour at `color`, as red, green and blue (hsvToRgbAt each): the first nearer than 1000 and
 * every other one nearer; 0 if none.  The distance is the vectors' (0x4e0000 the difference,
 * 0x4e06a0 its length, on the x87's stack) compared there with the best, as the code does: C0
 * set below it, or unordered.  nullopt when the game stopped. */
std::optional<NearestLeft> nearestColor(App* app, CPU& cpu, reg32 model, reg32 color)
{
    const GuestFrame frame(cpu, kNearestPushed, kNearestFrame);
    const reg32 tried = frame - kNearestColor, wanted = frame - kNearestWanted;
    const reg32 difference = frame - kNearestDifference, rgb = frame - kNearestRgb;
    float best = kStartDistance;
    reg32 nearest = 0;
    std::optional<HsvLeft> converted = hsvToRgbAt(app, cpu, color, rgb);
    if (!converted)
        return std::nullopt;
    reg32 lastLeft = converted->brightest;
    rgbToFloats(app, rgb, wanted);
    for (reg32 index = 0; sreg32(index) < sreg32(at(app, model + kModelColors))
                          && sreg32(index) < sreg32(at(app, model + kModelSeconds));
         ++index)
    {
        if (!hsvToRgbAt(app, cpu, model + kModelColor + (index << kColorShift), rgb))
            return std::nullopt;
        rgbToFloats(app, rgb, tried);
        {
            const KeptRegisters kept(cpu);
            cpu.eax = 1;
            cpu.edx = wanted;
            cpu.ebx = tried;
            cpu.ecx = difference;
            if (!byAddress(app, cpu, kSubtract))
                return std::nullopt;
            lastLeft = cpu.edx;
        }
        const double length = guest::direct<0x4e06a0, vertices::vectorLength>(app, cpu, difference);
        if (cpu.terminate)
            return std::nullopt;
        x86::FPU& fpu = cpu.fpu;
        fpu.push(length);  // where the call leaves it
        const float distance = float(fpu.st(0));
        fpu.compare(fpu.st(0), x86::Float(best));
        const bool below = (fpu.status.word & 0x100) != 0;
        fpu.pop();
        if (below)
        {
            nearest = index;
            best = distance;
        }
    }
    return NearestLeft{nearest, lastLeft};
}

}  // namespace

/* 0x49cc50: tint for the pixel eax, the colour at edx; edx its red and green (as it was, for a
 * pixel without its top bit). */
void tintPixel(App* app, CPU& cpu)
{
    const std::optional<TintLeft> left = tint(app, cpu, cpu.eax, cpu.edx);
    if (!left)
        return;
    if ((cpu.eax & kPixelTop) != 0)
        cpu.edx = left->redGreen;
    cpu.eax = left->pixel;
    cpu.esp += 4;
}

/* 0x49d240: nearestColor for the model eax, the colour at edx; edx as the code leaves it. */
void nearestModelColor(App* app, CPU& cpu)
{
    const std::optional<NearestLeft> left = nearestColor(app, cpu, cpu.eax, cpu.edx);
    if (!left)
        return;
    cpu.eax = left->index;
    cpu.edx = left->left;
    cpu.esp += 4;
}

namespace
{

// readCarColors' frame: three registers pushed, then 0x254 bytes: whether car.fce was found at -4,
// its place at -8, the .viv's FILE at -0x14, the colours' count at -0x18 (each handed to a
// function it calls), red, green and blue at -0x28 (hsvToRgbAt's), the colours read at -0x128,
// the .viv's name at -0x254.
constexpr reg32 kReadPushed = 0xc, kReadFrame = 0x254, kReadFound = 4, kReadPlace = 8, kReadFile = 0x14;
constexpr reg32 kReadCount = 0x18, kReadRgb = 0x28, kReadHsv = 0x128, kReadName = 0x254;
// The .fce's colours' count, its colours (0x100 bytes: 16 at most), its 12 bytes at +0x28.
constexpr reg32 kFceColorsBytes = 0x100, kFceTwelve = 0x28, kTwelveBytes = 0xc, kMostColors = 0x10;
// mrc.c's trace when car.fce is not found: its line, and the words the code hands with it.
constexpr reg32 kTraceLine = 0x5a, kTraceLevel = 1, kTraceSize = 0x104, kTraceWords = 0x14;
// fseek's whence: from the start, from where it is.
constexpr reg32 kSeekSet = 0, kSeekCurrent = 1;

}  // namespace

/* 0x49c700's core: the colours of the model car.fce in the .viv named `name` ("%s.viv"): their
 * count ([+0x7fc]) to [count], the first `wanted` of them (16 at most) as red, green, blue and
 * the fourth, a byte each, to `colors` on; its 12 bytes at +0x28 to `twelve`.  Not found, a trace
 * of mrc.c's line 0x5a and [count] 0.  The .viv's file is read (fseek, fread) into a frame at the
 * code's depth.  nullopt when the game stopped. */
std::optional<ColorsLeft> readColors(App* app, CPU& cpu, reg32 name, reg32 colors, reg32 wanted, reg32 count,
                                     reg32 twelve)
{
    const GuestFrame frame(cpu, kReadPushed, kReadFrame);
    const reg32 found = frame - kReadFound, place = frame - kReadPlace, file = frame - kReadFile;
    const reg32 read = frame - kReadCount, rgb = frame - kReadRgb, hsv = frame - kReadHsv;
    const reg32 vivName = frame - kReadName;
    put(app, count, 0);
    if (!print(app, cpu, {vivName, kVivFormat, name}))
        return std::nullopt;
    const std::optional<reg32> viv = guest::direct<0x43b830, ea::vivOpened>(app, cpu, vivName, file);
    if (!viv)
        return std::nullopt;
    reg32 findLeft;
    {
        // It may leave ecx changed: kept for where the code leaves it so (by address: its typed
        // function's glue gives back only whether it found it).
        const KeptRegisters kept(cpu);
        cpu.eax = *viv;
        cpu.edx = kFceName;
        cpu.ebx = place;
        cpu.ecx = found;
        if (!byAddress(app, cpu, kFindInViv))
            return std::nullopt;
        findLeft = cpu.ecx;
    }
    if (at(app, found) == 0)
    {
        const std::optional<reg32> closed = guest::direct<0x43b9b0, ea::vivClosed>(app, cpu, *viv, file);
        if (!closed)
            return std::nullopt;
        {
            // The trace's words pushed as the code pushes them (it reads none of them).
            const GuestFrame words(cpu, 0, kTraceWords);
            reg32 slot = words - kTraceWords;
            for (const reg32 word : {kTraceLine, kTraceLevel, kTraceFormat, kTraceFile, kTraceSize})
            {
                put(app, slot, word);
                slot += 4;
            }
            guest::direct<0x42d510, ea::traceOff>(app, cpu);
            if (cpu.terminate)
                return std::nullopt;
        }
        return ColorsLeft{*closed, place, findLeft, file};
    }
    const auto seekFrom = [&](reg32 offset) {
        return callGame(app, cpu, kFseek, {at(app, file), at(app, place), kSeekSet})
               && callGame(app, cpu, kFseek, {at(app, file), offset, kSeekCurrent});
    };
    const auto readInto = [&](reg32 to, reg32 bytes) {
        return callGame(app, cpu, kFread, {to, bytes, 1, at(app, file)}).has_value();
    };
    if (!seekFrom(kModelColors) || !readInto(read, 4) || !seekFrom(kModelColor) || !readInto(hsv, kFceColorsBytes)
        || !seekFrom(kFceTwelve) || !readInto(twelve, kTwelveBytes)
        || !guest::direct<0x43b9b0, ea::vivClosed>(app, cpu, *viv, file))
        return std::nullopt;
    reg32 index = 0, lastColor = file;
    for (; sreg32(index) < sreg32(wanted) && index < kMostColors; ++index)
    {
        if (!hsvToRgbAt(app, cpu, hsv + (index << kColorShift), rgb))
            return std::nullopt;
        lastColor = colors + index * 4;
        putByte(app, lastColor, byte(app, rgb + kRed));
        putByte(app, lastColor + 1, byte(app, rgb + kGreen));
        putByte(app, lastColor + 2, byte(app, rgb + kBlue));
        putByte(app, lastColor + 3, byte(app, rgb + kFourth));
    }
    const reg32 total = at(app, read);
    put(app, count, total);
    return ColorsLeft{total, 1, index, lastColor};
}

/* 0x49c700: readColors for the .viv named eax, the colours to edx, ebx of them, their count to
 * [ecx], the 12 bytes to [esp+4]; eax, ebx, ecx and edx as the code leaves them. */
void readCarColors(App* app, CPU& cpu)
{
    const std::optional<ColorsLeft> left =
        readColors(app, cpu, cpu.eax, cpu.edx, cpu.ebx, cpu.ecx, at(app, cpu.esp + kReturnSlot));
    if (!left)
        return;
    cpu.eax = left->eax;
    cpu.ebx = left->ebx;
    cpu.ecx = left->ecx;
    cpu.edx = left->edx;
    cpu.esp += kReturnSlot + 4;
}

namespace
{

// scaleTexture's frame: three registers pushed, then 0x30 bytes (its locals, none handed on);
// fitTexture's: three registers pushed, then scaleTexture's four words.
constexpr reg32 kScalePushed = 0xc, kScaleFrame = 0x30, kFitPushed = 0xc, kScaleWords = 0x10;
// A 32-bit pixel: red, green, blue, alpha a byte each; a 1555 word's channels at 10, 5 and 0.
constexpr reg32 kPixelGreen = 1, kPixelBlue = 2, kPixelAlpha = 3, kAlphaHalf = 0x80, kAlphaFull = 0xff;
constexpr reg32 kBlackDrawn = 0x8001;
// The texture's sides: the largest power of two no larger than the picture's, from 512 down.
constexpr reg32 kSides[] = {0x200, 0x100, 0x80, 0x40, 0x20, 0x10, 8, 4, 2};

/* 0x49cd00's core: the `width` by `height` pixels at `pixels` (32 bits each) scaled to `toWidth`
 * by `toHeight` 1555 words at `to`, the nearest pixel of each: the top bit from the alpha's being
 * any; tinted (tint, as deep as the code calls it) in the colour `color` where the alpha's top
 * bit is clear, else in `second` where it is not 0xff -- when there is the colour; 0x8000 drawn
 * as 0x8001 while [0x7a3a6c]'s low bit is set.  The steps are the x87's quotients stored as
 * floats, the nearest pixel the step's sum chopped (0x4dfd56).  nullopt when the game stopped. */
std::optional<Left> scaleInto(App* app, CPU& cpu, reg32 to, reg32 pixels, reg32 width, reg32 height, reg32 toWidth,
                              reg32 toHeight, reg32 color, reg32 second)
{
    const GuestFrame frame(cpu, kScalePushed, kScaleFrame);
    const x86::Float across = fp::div(x86::Float(sreg32(width)), x86::Float(sreg32(toWidth)));
    const x86::Float down = fp::div(x86::Float(sreg32(height)), x86::Float(sreg32(toHeight)));
    const float columnStep = float(across), rowStep = float(down);
    Left left{toHeight, width, 0, pixels};
    reg32 row = 0;
    for (; sreg32(row) < sreg32(toHeight); ++row)
    {
        const std::optional<reg32> sourceRow =
            chopToInteger(app, cpu, fp::mul(x86::Float(sreg32(row)), x86::Float(rowStep)));
        if (!sourceRow)
            return std::nullopt;
        const reg32 line = pixels + ((*sourceRow * width) << 2);
        float x = 0.0f;
        left.ebx = 0;
        left.edx = line;
        const auto nearest = [&]() -> std::optional<reg32> {
            const std::optional<reg32> column = chopToInteger(app, cpu, x86::Float(x));
            if (!column)
                return std::nullopt;
            return line + (*column << 2);
        };
        reg32 column = 0;
        for (; sreg32(column) < sreg32(toWidth); ++column)
        {
            const std::optional<reg32> first = nearest();
            if (!first)
                return std::nullopt;
            const reg32 top = byte(app, *first + kPixelAlpha) != 0 ? kPixelTop : 0;
            const std::optional<reg32> source = nearest();
            if (!source)
                return std::nullopt;
            const reg32 high = reg32(byte(app, *source + kPixelBlue)) >> 3;
            const reg32 low = reg32(byte(app, *source)) >> 3;
            const reg32 middle = reg32(byte(app, *source + kPixelGreen)) >> 3;
            std::optional<reg32> tintIn;  // the colour a tint is in, if any
            if (color != 0 && byte(app, *source + kPixelAlpha) < kAlphaHalf)
                tintIn = color;
            else if (second != 0)
            {
                const std::optional<reg32> alpha = nearest();
                if (!alpha)
                    return std::nullopt;
                if (byte(app, *alpha + kPixelAlpha) < kAlphaFull)
                    tintIn = second;
            }
            reg32 pixel, redGreen;
            if (tintIn)
            {
                const reg32 plain = ((high << 10) | top | (middle << 5) | low) & 0xffff;
                const GuestFrame call(cpu, kReturnSlot, 0);
                const std::optional<TintLeft> tinted = tint(app, cpu, plain, *tintIn);
                if (!tinted)
                    return std::nullopt;
                pixel = tinted->pixel;
                redGreen = (plain & kPixelTop) != 0 ? tinted->redGreen : *tintIn;
            }
            else
            {
                redGreen = top | (high << 10) | (middle << 5);
                pixel = low | redGreen;
            }
            putHalf(app, to, reg16(pixel));
            if ((at(app, kBlackVisible) & (half(app, to) == kPixelTop ? 1u : 0u)) != 0)
                putHalf(app, to, kBlackDrawn);
            x = float(fp::add(x86::Float(x), x86::Float(columnStep)));
            to += 2;
            left.ebx = column + 1;
            left.edx = redGreen;
        }
        left.eax = column;
    }
    left.ecx = row;
    return left;
}

/* 0x49cea0's core: the `width` by `height` pixels at `pixels` made a texture at `texture`: its
 * size ([texture], [texture + 4]) the largest power of two no larger than the width, 512 at most
 * (the port's: 256 in the game's code, 512 reached only where the size asked for is 512,
 * port_thrash.h) -- or the size asked for when the width is larger than it -- and the pixels
 * scaled to it at texture + 8, in the colours `color` and `second`.  What the scaling leaves. */
std::optional<Left> fitInto(App* app, CPU& cpu, reg32 texture, reg32 pixels, reg32 width, reg32 height,
                            reg32 asked, reg32 color, reg32 second)
{
    reg32 size = 1;
    if (sreg32(width) > sreg32(asked))
        size = asked;
    else
        for (const reg32 side : kSides)
            if (sreg32(width) >= sreg32(side))
            {
                size = side;
                break;
            }
    put(app, texture + 4, size);
    put(app, texture, size);
    const GuestFrame call(cpu, kFitPushed + kScaleWords + kReturnSlot, 0);
    return scaleInto(app, cpu, texture + 8, pixels, width, height, at(app, texture), at(app, texture + 4), color,
                     second);
}

}  // namespace

/* 0x49cd00: scaleInto for the ebx by ecx pixels at edx, to eax, [esp+4] by [esp+8], the colours
 * [esp+0xc] and [esp+0x10]; eax, ebx, ecx and edx as the code leaves them. */
void scaleTexture(App* app, CPU& cpu)
{
    const reg32 words = cpu.esp + kReturnSlot;
    const std::optional<Left> left = scaleInto(app, cpu, cpu.eax, cpu.edx, cpu.ebx, cpu.ecx, at(app, words),
                                               at(app, words + 4), at(app, words + 8), at(app, words + 0xc));
    if (!left)
        return;
    cpu.eax = left->eax;
    cpu.ebx = left->ebx;
    cpu.ecx = left->ecx;
    cpu.edx = left->edx;
    cpu.esp += kReturnSlot + kScaleWords;
}

/* 0x49cea0: fitInto for the ebx by ecx pixels at edx, to eax, [esp+4] asked, the colours [esp+8]
 * and [esp+0xc]; eax, ebx, ecx and edx as the scaling leaves them. */
void fitTexture(App* app, CPU& cpu)
{
    const reg32 words = cpu.esp + kReturnSlot;
    const std::optional<Left> left = fitInto(app, cpu, cpu.eax, cpu.edx, cpu.ebx, cpu.ecx, at(app, words),
                                             at(app, words + 4), at(app, words + 8));
    if (!left)
        return;
    cpu.eax = left->eax;
    cpu.ebx = left->ebx;
    cpu.ecx = left->ecx;
    cpu.edx = left->edx;
    cpu.esp += kReturnSlot + 0xc;
}

namespace
{

// A model's pictures' count (+0xc).  A picture of the .art: its width, its height, its 32-bit
// pixels at +8.  A texture's block: its width, its height, its 1555 words at +8; a block of the
// game's memory allocated at 16 bytes' alignment.
constexpr reg32 kModelPictures = 0xc, kPictureWidth = 0, kPictureHeight = 4, kPicturePixels = 8;
// A model's textures' handles (+0x1e04 on, one a picture).
constexpr reg32 kModelTexture = 0x1e04;
constexpr reg32 kTextureWidth = 0, kTextureHeight = 4, kTexturePixels = 8, kBlockAlign = 0x10;
// A .tga's header: its id's length (+0), its width (+0xc) and height (+0xe), little-endian
// words; the pixels after its 18 bytes and the id.
constexpr reg32 kTgaIdLength = 0, kTgaWidth = 0xc, kTgaHeight = 0xe, kTgaHeader = 0x12;
// loadArtTextures' frame: three registers pushed, then 0x1c bytes; loadTgaTexture's: three, then
// 0x18; the three words each hands fitTexture, and loadTgaTexture takes.
constexpr reg32 kArtPushed = 0xc, kArtFrame = 0x1c, kTgaPushed = 0xc, kTgaFrame = 0x18, kThreeWords = 0xc;
// loadCarModel's frame: four registers pushed, then 0xd8 bytes, the name made at its top.
constexpr reg32 kModelPushed = 0x10, kModelFrame = 0xd8;
// A file loaded as it is, or quietly (0x4e0ec0's mode).
constexpr reg32 kLoadPlain = 0, kLoadQuiet = 0x10;

/* A texture's block's bytes for `most` a side: two bytes a pixel and the width and height. */
reg32 textureBytes(reg32 most)
{
    return most * most * 2 + kTexturePixels;
}

/* A call by address with eax, edx, ebx and ecx `first` to `fourth`, which may change them: what it leaves in
 * the four; every other register kept.  nullopt when the game stopped. */
std::optional<Left> callLeaving(App* app, CPU& cpu, reg32 target, reg32 first, reg32 second, reg32 third, reg32 fourth)
{
    const KeptRegisters kept(cpu);
    cpu.eax = first;
    cpu.edx = second;
    cpu.ebx = third;
    cpu.ecx = fourth;
    if (!byAddress(app, cpu, target))
        return std::nullopt;
    return Left{cpu.eax, cpu.ebx, cpu.ecx, cpu.edx};
}

/* fitInto as the code calls it, three words on the guest's stack under the caller's frame. */
std::optional<Left> fitCalled(App* app, CPU& cpu, reg32 texture, reg32 pixels, reg32 width, reg32 height,
                              reg32 asked, reg32 color, reg32 second)
{
    const GuestFrame call(cpu, kThreeWords + kReturnSlot, 0);
    return fitInto(app, cpu, texture, pixels, width, height, asked, color, second);
}

/* 0x49d130's core: the textures of the model from the .art's pictures at `picture`, `most` at
 * most each way, in the colour `color` -- and the model's second colours' first, if it has them:
 * each picture fitted into a block of the game's memory (fitInto), allocated as a texture
 * (0x49c600, its handle to the model's +0x1e04 on) and given the pixels (0x49c620); a picture of
 * no size has no texture (0).  Only for a model of more than one picture -- the block freed only
 * then.  What it leaves in the four registers; nullopt when the game stopped. */
std::optional<Left> loadArt(App* app, CPU& cpu, reg32 model, reg32 picture, reg32 most, reg32 color)
{
    const GuestFrame frame(cpu, kArtPushed, kArtFrame);
    const std::optional<reg32> block = guest::direct<0x4e1620, ea::reserved>(app, cpu, kNoName, textureBytes(most), kBlockAlign);
    if (!block)
        return std::nullopt;
    const reg32 seconds = at(app, model + kModelSeconds);
    const reg32 second = seconds != 0 ? model + kModelSecond : 0;
    const reg32 count = at(app, model + kModelPictures);
    if (sreg32(count) <= 1)
        return Left{count, kBlockAlign, color, seconds};
    Left left{};
    for (reg32 index = 0; sreg32(index) < sreg32(count); ++index)
    {
        const reg32 handle = model + index * 4 + kModelTexture;
        if (at(app, picture + kPictureWidth) != 0 && at(app, picture + kPictureHeight) != 0)
        {
            if (!fitCalled(app, cpu, *block, picture + kPicturePixels, at(app, picture + kPictureWidth),
                           at(app, picture + kPictureHeight), most, color, second))
                return std::nullopt;
            const std::optional<reg32> texture = guest::direct<0x49c600, graphics::allocatedTexture>(
                app, cpu, at(app, *block + kTextureWidth), at(app, *block + kTextureHeight));
            if (!texture)
                return std::nullopt;
            put(app, handle, *texture);
            const std::optional<Left> given =
                callLeaving(app, cpu, kUpdateTexture, *texture, *block + kTexturePixels,
                            at(app, *block + kTextureWidth), at(app, *block + kTextureHeight));
            if (!given)
                return std::nullopt;
            left.edx = given->edx;
        }
        else
        {
            put(app, handle, 0);
            left.edx = model;
        }
        left.ecx = at(app, picture + kPictureHeight);
        picture += ((at(app, picture + kPictureWidth) * left.ecx) << 2) + kPicturePixels;
        left.ebx = index + 1;
    }
    left.eax = count;
    if (*block != 0)
    {
        const std::optional<reg32> freed = guest::direct<0x4e1890, ea::purgeBlock>(app, cpu, *block);
        if (!freed)
            return std::nullopt;
        left.eax = *freed;
    }
    return left;
}

/* 0x49d320's core: the model's texture from the .tga at `tga`, `most` at most each way (a .tga of
 * 512 a side or more at 512, for a model of the one picture it is read for: the port's,
 * port_thrash.h): in the second colour of the model's nearest the colour at `color` (the first
 * without that colour; none without second colours) -- a model of one picture only, and a
 * picture of some size, else no texture (0); the texture `texture`, or one allocated (0x49c600),
 * its handle to the model's +0x1e04, given the pixels (0x49c620) when `upload` says so; the
 * fitted pixels copied to `copyTo` when there is one.  The block of the game's memory they were
 * fitted in freed.  What it leaves in the four registers; nullopt when the game stopped. */
std::optional<Left> loadTga(App* app, CPU& cpu, reg32 model, reg32 tga, reg32 most, reg32 texture, reg32 color,
                            reg32 upload, reg32 copyTo)
{
    const GuestFrame frame(cpu, kTgaPushed, kTgaFrame);
    const auto word = [&](reg32 offset) { return reg32(byte(app, tga + offset) | byte(app, tga + offset + 1) << 8); };
    if (at(app, model + kModelPictures) == 1)
        most = reg32(port::driver::texture512Side(sreg32(most), int32_t(word(kTgaWidth))));
    const std::optional<reg32> block = guest::direct<0x4e1620, ea::reserved>(app, cpu, kNoName, textureBytes(most), kBlockAlign);
    if (!block)
        return std::nullopt;
    Left left{*block, at(app, model + kModelSeconds), color, textureBytes(most)};
    reg32 second = 0;
    if (left.ebx != 0)
    {
        reg32 nearest = 0;
        if (color != 0)
        {
            const GuestFrame call(cpu, kReturnSlot, 0);
            const std::optional<NearestLeft> found = nearestColor(app, cpu, model, color);
            if (!found)
                return std::nullopt;
            nearest = found->index;
        }
        second = model + kModelSecond + (nearest << kColorShift);
        left.eax = nearest << kColorShift;
        left.edx = second;
    }
    left.eax = at(app, model + kModelPictures);
    if (left.eax == 1)
    {
        const reg32 width = word(kTgaWidth), height = word(kTgaHeight);
        left = Left{byte(app, tga + kTgaIdLength), width, tga, tga + kTgaHeader + byte(app, tga + kTgaIdLength)};
        if (width != 0 && height != 0)
        {
            const std::optional<Left> fitted = fitCalled(app, cpu, *block, left.edx, width, height, most, color, second);
            if (!fitted)
                return std::nullopt;
            left = *fitted;
            if (texture == 0)
            {
                const std::optional<Left> allocated =
                    callLeaving(app, cpu, kAllocTexture, at(app, *block + kTextureWidth),
                                at(app, *block + kTextureHeight), left.ebx, left.ecx);
                if (!allocated)
                    return std::nullopt;
                left = *allocated;
                texture = left.eax;
            }
            else
            {
                // port: the texture given made the fitted size, 256 or 512 (port_thrash.h)
                port::driver::resizeTexture(app, texture, at(app, *block + kTextureWidth));
            }
            left.eax = texture;
            put(app, model + kModelTexture, texture);
            if (upload != 0)
            {
                const std::optional<Left> given =
                    callLeaving(app, cpu, kUpdateTexture, texture, *block + kTexturePixels,
                                at(app, *block + kTextureWidth), at(app, *block + kTextureHeight));
                if (!given)
                    return std::nullopt;
                left = *given;
            }
        }
        else
            put(app, model + kModelTexture, 0);
    }
    if (copyTo != 0)
    {
        const reg32 bytes = (at(app, *block + kTextureWidth) * 2) * at(app, *block + kTextureHeight);
        const std::optional<Left> copied =
            callLeaving(app, cpu, kMoveMemory, *block + kTexturePixels, copyTo, bytes, left.ecx);
        if (!copied)
            return std::nullopt;
        left = *copied;
    }
    if (*block != 0)
    {
        const std::optional<reg32> freed = guest::direct<0x4e1890, ea::purgeBlock>(app, cpu, *block);
        if (!freed)
            return std::nullopt;
        left.eax = *freed;
    }
    return left;
}

/* loadTga as the code calls it, its three words on the guest's stack under the caller's frame. */
std::optional<Left> tgaCalled(App* app, CPU& cpu, reg32 model, reg32 tga, reg32 most, reg32 color)
{
    const GuestFrame call(cpu, kThreeWords + kReturnSlot, 0);
    return loadTga(app, cpu, model, tga, most, 0, color, 1, 0);
}

/* 0x49d450's core: the car's model named `name`: "%s.fce" loaded and made ready, its texture from
 * "%s00.tga" when it is of one picture and the file named `name` is there, else from "%s.art";
 * `most` at most each way, in its colour `color` (of as many as it has).  The files loaded
 * freed; the model, and what the texture's loading leaves in ebx and edx (ebx the .art, for
 * one).  nullopt when the game stopped. */
std::optional<Left> loadModel(App* app, CPU& cpu, reg32 name, reg32 most, reg32 color)
{
    const GuestFrame frame(cpu, kModelPushed, kModelFrame);
    const reg32 path = frame - kModelFrame;
    if (!print(app, cpu, {path, kFceFormat, name}))
        return std::nullopt;
    const std::optional<reg32> fce = guest::direct<0x4e0ec0, ea::loadWhole>(app, cpu, path, kLoadPlain);
    if (!fce)
        return std::nullopt;
    const std::optional<reg32> model = callGame(app, cpu, kModelReady, {*fce});
    if (!model || !print(app, cpu, {path, kTgaFormat, name}))
        return std::nullopt;
    bool fromTga = false;
    if (at(app, *model + kModelPictures) == 1)
    {
        const std::optional<bool> exists = guest::direct<0x4e0e60, ea::fileThere>(app, cpu, name);
        if (!exists)
            return std::nullopt;
        fromTga = *exists;
    }
    // The colour asked for: the model's colour `color` modulo as many as it has (idiv's
    // remainder), or none.
    const auto colorOf = [&]() -> reg32 {
        const reg32 colors = at(app, *model + kModelColors);
        if (sreg32(colors) <= 0)
            return 0;
        return *model + kModelColor + (divide(sreg32(color), colors).second << kColorShift);
    };
    std::optional<Left> left;
    if (fromTga)
    {
        const std::optional<reg32> tga = guest::direct<0x4e0ec0, ea::loadWhole>(app, cpu, path, kLoadQuiet);
        if (!tga)
            return std::nullopt;
        left = tgaCalled(app, cpu, *model, *tga, most, colorOf());
        if (!left || (*tga != 0 && !guest::direct<0x4e1890, ea::purgeBlock>(app, cpu, *tga)))
            return std::nullopt;
    }
    else
    {
        if (!print(app, cpu, {path, kArtFormat, name}))
            return std::nullopt;
        const std::optional<reg32> art = guest::direct<0x4e0ec0, ea::loadWhole>(app, cpu, path, kLoadQuiet);
        if (!art)
            return std::nullopt;
        {
            const GuestFrame call(cpu, kReturnSlot, 0);
            left = loadArt(app, cpu, *model, *art, most, colorOf());
        }
        if (!left || (*art != 0 && !guest::direct<0x4e1890, ea::purgeBlock>(app, cpu, *art)))
            return std::nullopt;
        left->ebx = *art;
    }
    left->eax = *model;
    return left;
}

}  // namespace

/* 0x49d130: loadArt for the model eax, the pictures at edx, ebx at most, the colour ecx; eax, ebx,
 * ecx and edx as the code leaves them. */
void loadArtTextures(App* app, CPU& cpu)
{
    const std::optional<Left> left = loadArt(app, cpu, cpu.eax, cpu.edx, cpu.ebx, cpu.ecx);
    if (!left)
        return;
    cpu.eax = left->eax;
    cpu.ebx = left->ebx;
    cpu.ecx = left->ecx;
    cpu.edx = left->edx;
    cpu.esp += kReturnSlot;
}

/* 0x49d320: loadTga for the model eax, the .tga at edx, ebx at most, the texture ecx, the colour
 * [esp+4], whether to upload [esp+8], where to copy [esp+0xc]; eax, ebx, ecx and edx as the code
 * leaves them. */
void loadTgaTexture(App* app, CPU& cpu)
{
    const reg32 words = cpu.esp + kReturnSlot;
    const std::optional<Left> left = loadTga(app, cpu, cpu.eax, cpu.edx, cpu.ebx, cpu.ecx, at(app, words),
                                             at(app, words + 4), at(app, words + 8));
    if (!left)
        return;
    cpu.eax = left->eax;
    cpu.ebx = left->ebx;
    cpu.ecx = left->ecx;
    cpu.edx = left->edx;
    cpu.esp += kReturnSlot + kThreeWords;
}

/* 0x49d450: loadModel for the name at eax, edx at most, the colour ebx; ebx and edx as the
 * texture's loading leaves them. */
void loadCarModel(App* app, CPU& cpu)
{
    const std::optional<Left> left = loadModel(app, cpu, cpu.eax, cpu.edx, cpu.ebx);
    if (!left)
        return;
    cpu.eax = left->eax;
    cpu.ebx = left->ebx;
    cpu.edx = left->edx;
    cpu.esp += kReturnSlot;
}

namespace
{

// loadVivCarModel's frame: three registers pushed, then 0x23c bytes: the file's place at -0x18,
// its size at -0x1c, the colour as hue, saturation and brightness at -0x2c and as red, green and
// blue at -0x3c (16 bytes each, nfs3.exe's zeros at first), the path at -0x23c.
constexpr reg32 kVivModelPushed = 0xc, kVivModelFrame = 0x23c, kVivModelPlace = 0x18, kVivModelSize = 0x1c;
constexpr reg32 kVivModelHsv = 0x2c, kVivModelRgb = 0x3c, kVivModelPath = 0x23c, kColorBytes = 0x10;
// copyVivCarModel's: four registers pushed, then 0x208 bytes: the place at -4, the size at -8,
// the path at -0x208.
constexpr reg32 kVivCopyPushed = 0x10, kVivCopyFrame = 0x208, kVivCopyPlace = 4, kVivCopySize = 8;
constexpr reg32 kVivCopyPath = 0x208;
// A car's model (+0x8a4) and its colour as hue, saturation and brightness (+0x8a8 on).
constexpr reg32 kCarModel = 0x8a4, kCarColor = 0x8a8;

}  // namespace

/* 0x4b7a60's core: the model named `name` in the .viv at `viv` (0x4ea290 finds "%s.fce" and
 * "%s00.tga" in it): the .fce copied into a block of the game's memory and made ready, its
 * texture from the .tga, `most` at most each way -- in the colour at `color` (red, green, blue
 * and the fourth, a byte each: as hue, saturation and brightness to the car's +0x8a8 on) when
 * there is one.  The block, and what the texture's loading (loadTga) leaves in ebx, ecx and edx;
 * nullopt when the game stopped. */
std::optional<Left> loadVivModel(App* app, CPU& cpu, reg32 car, reg32 name, reg32 most, reg32 color, reg32 viv)
{
    const GuestFrame frame(cpu, kVivModelPushed, kVivModelFrame);
    const reg32 place = frame - kVivModelPlace, size = frame - kVivModelSize, hsv = frame - kVivModelHsv;
    const reg32 rgb = frame - kVivModelRgb, path = frame - kVivModelPath;
    copyGuest(app, hsv, kZeros, kColorBytes);
    copyGuest(app, rgb, kMoreZeros, kColorBytes);
    if (!print(app, cpu, {path, kVivFceFormat, name}) || !findVivEntry(app, cpu, viv, path, place, size))
        return std::nullopt;
    const reg32 fce = viv + at(app, place);
    const std::optional<reg32> block = guest::direct<0x4e1620, ea::reserved>(app, cpu, kVivNoName, at(app, size), reg32(0));
    if (!block || !guest::direct<0x4ea4f0, ea::moveMemory>(app, cpu, *block, fce, at(app, size))
        || !callGame(app, cpu, kModelReady, {*block})
        || !print(app, cpu, {path, kVivTgaFormat, name}) || !findVivEntry(app, cpu, viv, path, place, size))
        return std::nullopt;
    const reg32 tga = viv + at(app, place);
    reg32 asked = 0;
    if (color != 0)
    {
        put(app, rgb + kRed, byte(app, color));
        put(app, rgb + kGreen, byte(app, color + 1));
        put(app, rgb + kBlue, byte(app, color + 2));
        put(app, rgb + kFourth, byte(app, color + 3));
        rgbToHsvAt(app, rgb, hsv);
        put(app, car + kCarColor + kHue, at(app, hsv + kHue));
        put(app, car + kCarColor + kSaturation, at(app, hsv + kSaturation));
        put(app, car + kCarColor + kBrightness, at(app, hsv + kBrightness));
        asked = hsv;
    }
    std::optional<Left> left = tgaCalled(app, cpu, *block, tga, most, asked);
    if (!left)
        return std::nullopt;
    left->eax = *block;
    return left;
}

/* 0x4b7ba0's core: another of the model named `name` in the .viv at `viv`: "%s.fce" copied into a
 * block of the game's memory, given the texture of the car's model and made ready.  The block;
 * nullopt when the game stopped.  What the copy (0x4ea4f0) leaves in ebx to `copyLeft`. */
std::optional<reg32> copyVivModel(App* app, CPU& cpu, reg32 car, reg32 name, reg32 viv, reg32& copyLeft)
{
    const GuestFrame frame(cpu, kVivCopyPushed, kVivCopyFrame);
    const reg32 place = frame - kVivCopyPlace, size = frame - kVivCopySize, path = frame - kVivCopyPath;
    if (!print(app, cpu, {path, kVivFceFormat, name}) || !findVivEntry(app, cpu, viv, path, place, size))
        return std::nullopt;
    const reg32 fce = viv + at(app, place);
    const std::optional<reg32> block = guest::direct<0x4e1620, ea::reserved>(app, cpu, kVivNoName, at(app, size), reg32(0));
    if (!block || !moveMemory(app, cpu, fce, *block, at(app, size), copyLeft))
        return std::nullopt;
    put(app, *block + kModelTexture, at(app, at(app, car + kCarModel) + kModelTexture));
    if (!callGame(app, cpu, kModelReady, {*block}))
        return std::nullopt;
    return *block;
}

/* 0x4b7a60: loadVivModel for car eax, the name at edx, ebx at most, the colour at ecx, the .viv
 * at [esp+4]; ebx, ecx and edx as the texture's loading leaves them. */
void loadVivCarModel(App* app, CPU& cpu)
{
    const std::optional<Left> left =
        loadVivModel(app, cpu, cpu.eax, cpu.edx, cpu.ebx, cpu.ecx, at(app, cpu.esp + kReturnSlot));
    if (!left)
        return;
    cpu.eax = left->eax;
    cpu.ebx = left->ebx;
    cpu.ecx = left->ecx;
    cpu.edx = left->edx;
    cpu.esp += kReturnSlot + 4;
}

/* 0x4b7ba0: copyVivModel for car eax, the name at edx, the .viv at ebx; edx the texture given,
 * ebx as the copy leaves it. */
void copyVivCarModel(App* app, CPU& cpu)
{
    const reg32 car = cpu.eax;
    reg32 copyLeft = cpu.ebx;
    const std::optional<reg32> block = copyVivModel(app, cpu, car, cpu.edx, cpu.ebx, copyLeft);
    if (!block)
        return;
    cpu.eax = *block;
    cpu.edx = at(app, *block + kModelTexture);
    cpu.ebx = copyLeft;
    cpu.esp += 4;
}

namespace
{

// The players' names (0x549784, 0x54978c), whether one will do (0x4721f0).
// GetModuleFileNameA's buffer and its size, a frame's under the six registers loadPlayerSpecs pushes;
// a file's size asked for with this mode; chmod's read and write; access's write.
constexpr reg32 kModuleBytes = 0xfc, kModuleSize = 0xf9, kSpecsPushed = 0x18;
// A call's return address's slot: where one of this file's cores calls another, the callee's
// frame as deep as the code's.
constexpr reg32 kSizeMode = 0x10, kReadWrite = 0x180, kWritable = 2;

/* Player's spec `index`'s 0x30 bytes, or 0: no table for the player, or the index outside 0 to 49. */
reg32 specSlot(reg32 player, i32 index)
{
    const reg32 table = specsOf(player);
    if (table == 0 || index < 0 || index >= i32(kSpecCount))
        return 0;
    return table + reg32(index) * kSpecBytes;
}

/* Player's table of specs' name: the first's, the second's, or 0. */
reg32 specsName(reg32 player)
{
    return player == 0 ? kFirstPlayer : player == 1 ? kSecondPlayer : 0;
}

/* Whether the `count` bytes at `a` are those at `b` (repe cmpsb). */
bool sameBytes(App* app, reg32 a, reg32 b, reg32 count)
{
    for (reg32 i = 0; i < count; ++i)
        if (byte(app, a + i) != byte(app, b + i))
            return false;
    return true;
}

/* 0x4dc3a0's core: player's spec `index` marked in use or not; nothing for an index outside. */
void markSpec(App* app, reg32 player, i32 index, reg8 inUse)
{
    const reg32 slot = specSlot(player, index);
    if (slot != 0)
        putByte(app, slot + kSpecInUse, inUse);
}

/* 0x4dc3f0's core: whether player's spec `index` is in use; 0 for none. */
reg8 specInUse(App* app, reg32 player, i32 index)
{
    const reg32 slot = specSlot(player, index);
    return slot != 0 ? byte(app, slot + kSpecInUse) : 0;
}

/* 0x4dc440's core: player's spec `index`'s car id's address, or 0. */
reg32 specCarId(reg32 player, i32 index)
{
    const reg32 slot = specSlot(player, index);
    return slot != 0 ? slot + kSpecCarId : 0;
}

/* 0x4dc490's core: the first of player's specs whose car id is the 16 bytes at `id`, in use or
 * not; -1 for none. */
i32 specOfCar(App* app, reg32 player, reg32 id)
{
    if (specsOf(player) == 0)
        return -1;
    for (i32 index = 0; index < i32(kSpecCount); ++index)
        if (sameBytes(app, specSlot(player, index) + kSpecCarId, id, kCarIdBytes))
            return index;
    return -1;
}

/* 0x4dc500's core: player's first spec not in use, or -1. */
i32 firstFreeSpec(App* app, reg32 player)
{
    if (specsOf(player) == 0)
        return -1;
    for (i32 index = 0; index < i32(kSpecCount); ++index)
        if (specInUse(app, player, index) == 0)
            return index;
    return -1;
}

/* 0x4dc5a0's core: player's spec `index` given the car id at `id` and the spec at `spec`, each
 * cleared (0x4e0640) where there is none; nothing for an index outside.  False when the game
 * stopped. */
bool storeSpec(App* app, CPU& cpu, reg32 player, i32 index, reg32 id, reg32 spec)
{
    const reg32 slot = specSlot(player, index);
    if (slot == 0)
        return true;
    if (id != 0)
        copyGuest(app, slot + kSpecCarId, id, kCarIdBytes);
    else if (!clearBytes(app, cpu, slot + kSpecCarId, kCarIdBytes))
        return false;
    if (spec != 0)
        copyGuest(app, slot + kSpecCar, spec, kCarSpecBytes);
    else if (!clearBytes(app, cpu, slot + kSpecCar, kCarSpecBytes))
        return false;
    return true;
}

/* 0x4dc670's core: player's specs made anew: the table cleared, then each car there is (to the
 * first there is not) given a spec in use, its id and the default spec, while there are specs
 * free.  How many; nullopt when the game stopped. */
std::optional<i32> makeDefaultSpecs(App* app, CPU& cpu, reg32 player)
{
    i32 made = 0;
    const reg32 table = specsOf(player);
    if (table == 0)
        return made;
    if (!clearBytes(app, cpu, table, kSpecsBytes))
        return std::nullopt;
    for (i32 car = 0;; ++car)
    {
        const std::optional<reg32> record = carRecord(app, cpu, car);
        if (!record)
            return std::nullopt;
        if (*record != 0 && made < i32(kSpecCount))
        {
            const std::optional<reg32> id = carId(app, cpu, car);
            if (!id)
                return std::nullopt;
            if (*id != 0)
            {
                if (!storeSpec(app, cpu, player, made, *id, kDefaultSpec))
                    return std::nullopt;
                markSpec(app, player, made, 1);
                ++made;
            }
        }
        if (*record == 0)
            return made;
    }
}

/* 0x4dc710's core: player's specs brought up to the cars there are: each in use whose car is no
 * longer known (0x439990) cleared and marked free; each car there is (to the first there is not)
 * without a spec given one, free, its id and the default spec.  Whether anything was added or
 * taken; nullopt when the game stopped. */
std::optional<bool> refreshSpecsOf(App* app, CPU& cpu, reg32 player)
{
    bool changed = false;
    const reg32 table = specsOf(player);
    if (table == 0)
        return changed;
    for (i32 index = 0; index < i32(kSpecCount); ++index)
    {
        if (specInUse(app, player, index) == 0)
            continue;
        const reg32 id = specCarId(player, index);
        if (id == 0)
            continue;
        const reg32 known = guest::direct<0x439990, placeById>(app, cpu, id);
        if (cpu.terminate)
            return std::nullopt;
        if (known != 0xffffffff)
            continue;
        if (!storeSpec(app, cpu, player, index, 0, 0))
            return std::nullopt;
        markSpec(app, player, index, 0);
        changed = true;
    }
    for (i32 car = 0;; ++car)
    {
        const std::optional<reg32> record = carRecord(app, cpu, car);
        if (!record)
            return std::nullopt;
        if (*record != 0)
        {
            const std::optional<reg32> id = carId(app, cpu, car);
            if (!id)
                return std::nullopt;
            if (*id != 0)
            {
                const i32 found = specOfCar(app, player, *id);
                const i32 free = found >= 0 ? -1 : firstFreeSpec(app, player);
                if (free != -1)
                {
                    if (!storeSpec(app, cpu, player, free, *id, kDefaultSpec))
                        return std::nullopt;
                    markSpec(app, player, free, 1);
                    changed = true;
                }
            }
        }
        if (*record == 0)
            return changed;
    }
}

/* 0x4dc800's core: the specs' file of the player named `name`, made at 0x564188: the folder
 * ([0x7a2fb4] on), "carspecs\", the name, ".spc".  Its address. */
reg32 makeSpecPath(App* app, reg32 name)
{
    putByte(app, kSpecPath, 0);
    copyText(app, kSpecPath, kSpecFolder);
    appendText(app, kSpecPath, kSpecsFolder);
    appendText(app, kSpecPath, name);
    appendText(app, kSpecPath, kSpecExtension);
    return kSpecPath;
}

/* 0x4dc8b0's core: player's specs from the file's `bytes` bytes at `file` (0x30 at least), a
 * spec's 0x30 bytes at a time, 50 at most.  How many. */
i32 readSpecsFrom(App* app, reg32 player, reg32 file, reg32 bytes)
{
    const reg32 table = specsOf(player);
    i32 index = 0;
    if (file == 0 || table == 0 || bytes < kSpecBytes)
        return index;
    for (reg32 offset = 0; sreg32(offset) < sreg32(bytes) && index < i32(kSpecCount); offset += kSpecBytes)
    {
        copyGuest(app, table + reg32(index) * kSpecBytes, file + offset, kSpecBytes);
        ++index;
    }
    return index;
}

std::optional<reg32> saveSpecs(App* app, CPU& cpu, reg32 player);

/* 0x4dca50's core: player's specs saved, when the player's name will do: the file made writable
 * if it is there and is not (chmod 0x180), the specs brought up to the cars there are, the table
 * written whole (0x4eebb0).  What that gives, or 0; nullopt when the game stopped. */
std::optional<reg32> saveSpecs(App* app, CPU& cpu, reg32 player)
{
    const GuestFrame frame(cpu, kSpecsPushed, 0);  // as deep as the code's: access hands on a frame of its own
    const reg32 name = specsName(player), table = specsOf(player);
    if (name == 0)
        return 0;
    const std::optional<reg32> valid = callGame(app, cpu, kNameValid, {name});
    if (!valid)
        return std::nullopt;
    if (*valid == 0 || table == 0)
        return 0;
    const reg32 path = makeSpecPath(app, name);
    const std::optional<bool> exists = guest::direct<0x4e0e60, ea::fileThere>(app, cpu, path);
    if (!exists)
        return std::nullopt;
    if (*exists)
    {
        const std::optional<reg32> locked = callGame(app, cpu, kAccess, {path, kWritable});
        if (!locked)
            return std::nullopt;
        if (*locked != 0 && !callGame(app, cpu, kChmod, {path, kReadWrite}))
            return std::nullopt;
    }
    if (!refreshSpecsOf(app, cpu, player))
        return std::nullopt;
    return guest::direct<0x4eebb0, saveBlockQuietly>(app, cpu, path, table, kSpecsBytes);
}

}  // namespace

/* 0x4dc940's core: player's specs loaded, when the player's name will do: the table cleared; a
 * file of another machine's (0x472940, by the module's name, which GetModuleFileNameA puts in a
 * frame at the code's depth) deleted, and with none the specs made anew and saved; else the file
 * read in and freed, and saved again if bringing it up to the cars there are changed it -- made
 * anew instead when it would not load.  eax as the code leaves it: the player for one with no
 * table, 0 for a name that will not do, else what the last of the specs' functions gave;
 * nullopt when the game stopped. */
std::optional<reg32> playerSpecs(App* app, CPU& cpu, reg32 player)
{
    const GuestFrame frame(cpu, kSpecsPushed, kModuleBytes);
    const reg32 module = frame - kModuleBytes;
    const reg32 table = specsOf(player), name = specsName(player);
    if (name == 0)
        return player;
    const std::optional<reg32> valid = callGame(app, cpu, kNameValid, {name});
    if (!valid)
        return std::nullopt;
    if (*valid == 0 || table == 0)
        return *valid;
    if (!clearBytes(app, cpu, table, kSpecsBytes))
        return std::nullopt;
    const reg32 path = makeSpecPath(app, name);
    {
        const KeptRegisters kept(cpu);
        if (!guest::windows<kGetModuleFileName, &win32::kernel32::GetModuleFileNameA>(app, cpu, 0, module, kModuleSize))
            return std::nullopt;
    }
    const std::optional<bool> exists = guest::direct<0x4e0e60, ea::fileThere>(app, cpu, path);
    if (!exists)
        return std::nullopt;
    bool anew = !*exists;
    if (!anew)
    {
        const std::optional<reg32> foreign = guest::direct<0x472940, writtenBefore>(app, cpu, path, module);
        if (!foreign)
            return std::nullopt;
        if (*foreign != 0)
        {
            if (!callGame(app, cpu, kUnlink, {path}))
                return std::nullopt;
            anew = true;
        }
    }
    if (anew)
    {
        if (!makeDefaultSpecs(app, cpu, player))
            return std::nullopt;
        const GuestFrame call(cpu, kReturnSlot, 0);
        return saveSpecs(app, cpu, player);
    }
    const std::optional<reg32> size = callGame(app, cpu, kFileSize, {path, kSizeMode});
    if (!size)
        return std::nullopt;
    const std::optional<reg32> file = guest::direct<0x4e0fe0, ea::loadQuietly>(app, cpu, path, kSizeMode);
    if (!file)
        return std::nullopt;
    if (*file == 0)
    {
        const std::optional<i32> made = makeDefaultSpecs(app, cpu, player);
        if (!made)
            return std::nullopt;
        return reg32(*made);
    }
    if (sreg32(*size) > 0)
        readSpecsFrom(app, player, *file, *size);
    if (!guest::direct<0x4e1890, ea::purgeBlock>(app, cpu, *file))
        return std::nullopt;
    const std::optional<bool> changed = refreshSpecsOf(app, cpu, player);
    if (!changed)
        return std::nullopt;
    if (!*changed)
        return 0;
    const GuestFrame call(cpu, kReturnSlot, 0);
    return saveSpecs(app, cpu, player);
}

/* 0x4dc3a0: markSpec for player eax, spec edx, bl; eax, ebx and edx as the game's code leaves
 * them. */
void setSpecUsed(App* app, CPU& cpu)
{
    const reg32 player = cpu.eax, index = cpu.edx;
    const reg8 inUse = cpu.bl;
    markSpec(app, player, i32(index), inUse);
    const reg32 slot = specSlot(player, i32(index));
    if (slot != 0)
    {
        cpu.eax = slot;
        cpu.ebx = index;
        cpu.edx = (specsOf(player) & ~0xffu) | inUse;
    }
    else
    {
        cpu.eax = index;
        cpu.edx = specsOf(player);
    }
    cpu.esp += 4;
}

/* 0x4dc3f0: specInUse for player eax, spec edx, in al; eax's top as the code leaves it (the
 * slot's offset, or the index), edx the table. */
void specUsed(App* app, CPU& cpu)
{
    const reg32 player = cpu.eax, index = cpu.edx;
    const reg8 used = specInUse(app, player, i32(index));
    const reg32 offset = specSlot(player, i32(index)) != 0 ? index * kSpecBytes : index;
    cpu.eax = (offset & ~0xffu) | used;
    cpu.edx = specsOf(player);
    cpu.esp += 4;
}

/* 0x4dc440: specCarId for player eax, spec edx; edx the table. */
void specCarName(App*, CPU& cpu)
{
    const reg32 player = cpu.eax;
    cpu.eax = specCarId(player, i32(cpu.edx));
    cpu.edx = specsOf(player);
    cpu.esp += 4;
}

/* 0x4dc490: specOfCar for player eax, the id at edx; edx where the search stopped (-1 with no
 * table, 50 with no spec found). */
void findSpec(App* app, CPU& cpu)
{
    const reg32 player = cpu.eax;
    const i32 found = specOfCar(app, player, cpu.edx);
    cpu.eax = reg32(found);
    cpu.edx = specsOf(player) == 0 ? 0xffffffff : found >= 0 ? reg32(found) : kSpecCount;
    cpu.esp += 4;
}

/* 0x4dc500: firstFreeSpec for player eax. */
void freeSpecSlot(App* app, CPU& cpu)
{
    cpu.eax = reg32(firstFreeSpec(app, cpu.eax));
    cpu.esp += 4;
}

/* 0x4dc5a0: storeSpec for player eax, spec edx, the id at ebx, the spec at ecx; eax, ebx, ecx and
 * edx as the code leaves them: with the spec there, the copies' (the count, ecx 0) or the
 * clearings' (where it cleared, the count, edx 0). */
void setSpec(App* app, CPU& cpu)
{
    const reg32 player = cpu.eax, index = cpu.edx, id = cpu.ebx, spec = cpu.ecx;
    if (!storeSpec(app, cpu, player, i32(index), id, spec))
        return;
    const reg32 table = specsOf(player);
    if (table == 0)
    {
        cpu.edx = 0;
    }
    else if (specSlot(player, i32(index)) == 0)
    {
        cpu.eax = index;
        cpu.edx = 0;
    }
    else
    {
        cpu.eax = spec != 0 ? kCarSpecBytes : specSlot(player, i32(index)) + kSpecCar;
        cpu.ebx = spec != 0 ? table : kCarSpecBytes;
        cpu.ecx = 0;
        cpu.edx = spec != 0 && id != 0 ? index : 0;
    }
    cpu.esp += 4;
}

/* 0x4dc670: makeDefaultSpecs for player eax. */
void defaultSpecs(App* app, CPU& cpu)
{
    const std::optional<i32> made = makeDefaultSpecs(app, cpu, cpu.eax);
    if (!made)
        return;
    cpu.eax = reg32(*made);
    cpu.esp += 4;
}

/* 0x4dc710: refreshSpecsOf for player eax, in al. */
void refreshSpecs(App* app, CPU& cpu)
{
    const std::optional<bool> changed = refreshSpecsOf(app, cpu, cpu.eax);
    if (!changed)
        return;
    cpu.eax = *changed ? 1 : 0;
    cpu.esp += 4;
}

/* 0x4dc800: makeSpecPath for the name at eax. */
void specPath(App* app, CPU& cpu)
{
    cpu.eax = makeSpecPath(app, cpu.eax);
    cpu.esp += 4;
}

/* 0x4dc8b0: readSpecsFrom for player eax, the ebx bytes at edx; eax, ebx and edx as the code
 * leaves them: ebx the table, edx the bytes read, eax the player when none were, else the last
 * copy's count -- or 50, stopped by the table's end with bytes left. */
void readSpecs(App* app, CPU& cpu)
{
    const reg32 player = cpu.eax, file = cpu.edx, bytes = cpu.ebx;
    const i32 read = readSpecsFrom(app, player, file, bytes);
    const reg32 offset = reg32(read) * kSpecBytes;
    cpu.ebx = specsOf(player);
    cpu.edx = offset;
    if (read == 0)
        cpu.eax = player;
    else if (read == i32(kSpecCount) && sreg32(offset) < sreg32(bytes))
        cpu.eax = kSpecCount;
    else
        cpu.eax = kSpecBytes;
    cpu.esp += 4;
}

/* 0x4dc940: playerSpecs for player eax. */
void loadPlayerSpecs(App* app, CPU& cpu)
{
    const KeptRegisters kept(cpu);
    const std::optional<reg32> result = playerSpecs(app, cpu, cpu.eax);
    if (!result)
        return;
    cpu.eax = *result;
    cpu.esp += 4;
}

/* 0x4dca50: saveSpecs for player eax. */
void savePlayerSpecs(App* app, CPU& cpu)
{
    const KeptRegisters kept(cpu);
    const std::optional<reg32> result = saveSpecs(app, cpu, cpu.eax);
    if (!result)
        return;
    cpu.eax = *result;
    cpu.esp += 4;
}

}  // namespace load
}  // namespace nfs3hp
