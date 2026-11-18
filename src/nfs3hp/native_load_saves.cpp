#include "game_settings.h"
#include "native_controls_devices.h"
#include "native_controls_race.h"
#include "native_ea_files.h"
#include "native_ea_memory.h"
#include "native_ea_support.h"
#include "native_frontend_loada.h"
#include "native_frontend_loopa.h"
#include "native_frontend_menue.h"
#include "native_frontend_misc.h"
#include "native_frontend_pause.h"
#include "native_frontend_simd.h"
#include "native_frontend_wata.h"
#include "native_frontend_wina.h"
#include "native_game.h"
#include "native_guest.h"
#include "native_load_cardb.h"
#include "native_load_saves.h"
#include "native_load_track.h"
#include "native_watcom.h"
#include "native_windows.h"
#include "port_snd_stream_calls.h"
#include "port_text.h"
#include "host_files.h"
#include "host_system.h"

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <optional>
#include <type_traits>

/* The files the game saves and loads, and EA's formats under its loaders.
 * Each address is bound to the function of that name here
 * (LOAD_SAVES_NATIVES), and the game's code calls it by name.
 *
 * Replays: the race just run is replay.rp3, 0x296f8 bytes -- config.dat's
 * 0x20f0 (the race's settings) copied to its head at 0x7a3e50, its version
 * (2) at 0x7a56b0, whether the frames' log follows (0x7a5f40, the race's
 * [0x55d000] being 1) and the seed at 0x7a5f44, checksummed (0x478010,
 * 0x4781b0) and written whole, the log appended through stdio (0x477ac0).
 * Loading reads it back the same way (0x477b00 the log) and checks it
 * (0x478090, 0x478100).  A replay kept is a copy of it under a name of the
 * player's in the replays' folder ([0x7a2f34], "<name>.rp3"; a ghost,
 * ghost.gst, likewise ".gst"); one picked from the list is copied back over
 * replay.rp3 and loaded.  The list (a menu's record: +0x48 how many, at most
 * 200, +0x4c the one chosen, +0x70 its block -- 8 bytes an entry, the name's
 * address and a byte, then the names) is the folder's "*.rp3", the last race
 * first.  The replays' screen shows a replay's track, laps, kind of race and
 * car (the replay's +0x196c, +0x1974, +0x1878, its player's +0x19b4 car at
 * +0x1a00 + 0x6c each) in four lines of 0x78 at 0x558d0c.
 *
 * records.dat: the records' table, 0x4197c bytes ([0x559408]; its first word
 * 0x10, the version), loaded or made empty, and saved back.
 *
 * config.dat (in [0x7a3034]): the game's settings, 0x20f0 bytes at 0x6fbb40
 * (its first word its length), loaded at the start -- made afresh with the
 * defaults when it is missing, of another length, older than nfs3.exe, or
 * another version ([0x6fbc10] against [0x7a3a58]) -- and saved as the menus
 * leave it, unless the game runs from a demo's command line ([0x55eb2e] 0x20)
 * or a replay is shown.  The language, [0x6fbb4c], is read from its name
 * at 0x7a2634.
 *
 * EA's library under them: cmn\locatbig.c's BIG archives, of two kinds --
 * 0xc0fb (a 16-bit header's length at 2, each entry a 24-bit offset and size
 * and its name) and "BIGF" (the header's length at 0xc, each entry a 32-bit
 * offset and size and its name from 0x10), big-endian both -- an entry looked
 * up by its name (case aside) or by its index; cmn\savefile.c's savefileadr,
 * a block written to a file whole; and the fixed-point sine and cosine
 * (0x10000 a turn, 16.16 results) from the quarter wave at 0x5672ac.
 *
 * Each is a typed function under a name of its own, with a thunk under
 * the game's name for the callers still on registers and the dispatch: it
 * leaves eax, and the registers the game's code does not keep, as that code
 * leaves them.  The typed functions other headers declare are called as C++
 * (guest::direct; the menus' files' through feCore), the registers loaded
 * as the code loads them for the call and eax after it as the call leaves it
 * (load, leaves); every other function of the game's is called by address,
 * its arguments in registers and on the guest's stack as the code hands them
 * (call); Windows' functions through their imports' slots.  A frame
 * whose address a callee is handed -- a path sprintf'd, a handle opened into,
 * a FILETIME -- or under which Error is called
 * stays on the guest's stack at the original's depth (GuestFrame,
 * asCalled).  savefileadr is called by address where this file's own code
 * calls it (the file's age compared goes through guest::direct). */
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
using x86::reg32;
using x86::reg8;
using x86::sreg32;

// Where an Error is from: the file, the function, the line; a file not
// opened (or an entry not found) ends the game, when set.
constexpr reg32 kWhereFile = 0x552190, kWhereFunction = 0x552194, kWhereLine = 0x552198;
constexpr reg32 kErrorsFatal = 0x56440c;
// The game's strings.
constexpr reg32 kReplayName = 0x538bb4, kPercentS = 0x538bc0, kLapsFormat = 0x538bc4, kNoName = 0x538be8;
constexpr reg32 kGhostExtension = 0x538bec, kKeptFormat = 0x538bf0, kGhostName = 0x538bf8;
constexpr reg32 kReplayExtension = 0x538c14, kAllOfKind = 0x538c98, kRecordsFormat = 0x539578;
constexpr reg32 kRecordsBlockName = 0x539588, kLanguageNames = 0x53afd8, kConfigBlockName = 0x53b008;
constexpr reg32 kConfigFormat = 0x53b00c, kRaceReplayName = 0x540168, kReadWrite = 0x540174;
constexpr reg32 kLocatbigC = 0x54b5a4, kTypeOfBigFileName = 0x54b5b4, kNoBigFile = 0x54b5c4;
constexpr reg32 kNotBigFile = 0x54b5f4, kLocateBigEntryName = 0x54b620, kEntryNotFound = 0x54b630;
constexpr reg32 kIndexNotFound = 0x54b658, kSavefileC = 0x54c240, kSavefileadrName = 0x54c250;
constexpr reg32 kOpenFailed = 0x54c25c, kWriteFailed = 0x54c288;
// The replays' screen's four lines; the replays' and the saves' folders.
constexpr reg32 kReplayTrackLine = 0x558d0c, kReplayLapsLine = 0x558d84, kReplayKindLine = 0x558dfc;
constexpr reg32 kReplayCarLine = 0x558e74, kReplayFolder = 0x7a2f34, kSaveFolder = 0x7a3034;
constexpr reg32 kRecordsFolder = 0x7a2fb4;
// The replay's fields, its length; the replay of the race in memory.
constexpr reg32 kReplayKind = 0x1878, kReplayTrack = 0x196c, kReplayLaps = 0x1974, kReplayPlayer = 0x19b4;
constexpr reg32 kReplayCars = 0x1a00, kReplayBytes = 0x296f8, kRaceReplay = 0x7a3e50;
constexpr reg32 kRaceReplayVersion = 0x7a56b0, kRaceReplayLogged = 0x7a5f40, kRaceReplaySeed = 0x7a5f44;
// The replays' list: its entries at most, the block's length, where its
// names end.
constexpr reg32 kListEntries = 0xc8, kListBytes = 0x7b70, kListNamesEnd = 0x7538;
// The records' table, its length, its two parts.
constexpr reg32 kRecords = 0x559408, kRecordsFirst = 0x55940c, kRecordsSecond = 0x559410;
constexpr reg32 kRecordsBytes = 0x4197c, kRecordsVersion = 0x10;
// config.dat in memory, its length; the language and its name; whether the
// settings were made afresh; the version's word and the game's.
constexpr reg32 kConfigBytes = 0x20f0, kLanguageName = 0x7a2634;
constexpr reg32 kConfigMadeAfresh = 0x67900c, kGameVersion = 0x7a3a58;
// The camera's views chosen and the race's numbers for them.
constexpr reg32 kViewNumbers = 0x55629c;
// The command line's flags (0x20 a demo's, 0x10 another); whether the
// controllers are known (loaded with config.dat when set).
constexpr reg32 kCommandFlags = 0x55eb2e;
constexpr reg8 kDemoCommand = 0x20, kOtherCommand = 0x10;
constexpr reg32 kControllersKnown = 0x55b038;
// The default FILETIME (0x472160, among the code), the sine's quarter wave.
constexpr reg32 kNoTime = 0x472160, kQuarterWave = 0x5672ac;
// A random byte the game keeps, made with the seed.
constexpr reg32 kRandomByte = 0x57126b;
// Windows' functions, by their imports' slots.
constexpr reg32 kCompareFileTime = 0x53448c, kGetFileTime = 0x534528, kGetModuleFileNameA = 0x534544;
constexpr reg32 kLclose = 0x534644, kLopen = 0x534648;
// The game's functions, by address: those with no typed function, Error (its
// va_list where the code has it) and savefileadr.  Every other is called as C++:
// the other typed files' (a car's id made, its name, its index by id, the
// controllers loaded, the frames' log written and read, the controls made
// afresh, the random numbers seeded, a file there, loaded and loaded
// quietly, a block's size, the game's memory, EA's handles opened, closed,
// read and written, the directory, the runtime's, the language's
// text, the tasks run), and the menus' (feCore).
constexpr reg32 kError = 0x401010, kIs3DSoundAvailable = 0x40faf0, kUnlink = 0x4e28a0;
constexpr reg32 kChmod = 0x4ee3c0, kSavefileadr = 0x4eeae0, kChdir = 0x4f0260;

using App = win32::WinApplication;
using CPU = x86::CPU;

/* Where an Error is from, as the code sets it. */
void where(App* app, reg32 file, reg32 function, reg32 line)
{
    put(app, kWhereFile, file);
    put(app, kWhereFunction, function);
    put(app, kWhereLine, line);
}

/* The frame of a core whose callees see where it is -- handed its locals' addresses, or reading
 * the stack's words past theirs, or Error -- kept on the
 * guest's stack at the original's depth: `pushed` bytes of registers below the return address's
 * slot, then `bytes` of locals.  frame - n is the original's ebp - n. */
class GuestFrame
{
public:
    GuestFrame(CPU& cpu, reg32 pushed, reg32 bytes) : cpu_(cpu), esp_(cpu.esp), base_(cpu.esp - pushed)
    {
        cpu.esp = base_ - bytes;
    }
    ~GuestFrame() { cpu_.esp = esp_; }
    GuestFrame(const GuestFrame&) = delete;
    GuestFrame& operator=(const GuestFrame&) = delete;
    reg32 operator-(reg32 offset) const { return base_ - offset; }

private:
    CPU& cpu_;
    const reg32 esp_;
    const reg32 base_;
};

/* A core of this file called as the game's code calls it: `handed` bytes of stack arguments and
 * the return address's slot below esp, its own frame below them. */
template <typename Core>
auto asCalled(CPU& cpu, Core core, reg32 handed = 0)
{
    cpu.esp -= handed + 4;
    const auto result = core();
    cpu.esp += handed + 4;
    return result;
}

/* The runtime's unlink, chmod and chdir on a path (native_watcom.h); nothing when the game stopped. */
std::optional<reg32> unlinkFile(App* app, CPU& cpu, reg32 path)
{
    const reg32 result = guest::direct<0x4e28a0, watcom::clib::unlink>(app, cpu, path);
    if (cpu.terminate)
        return std::nullopt;
    return result;
}

std::optional<reg32> chmodFile(App* app, CPU& cpu, reg32 path, reg32 mode)
{
    const reg32 result = guest::direct<0x4ee3c0, watcom::clib::chmod>(app, cpu, path, mode);
    if (cpu.terminate)
        return std::nullopt;
    return result;
}

std::optional<reg32> changeDirectory(App* app, CPU& cpu, reg32 path)
{
    const reg32 result = guest::direct<0x4f0260, watcom::clib::chdir>(app, cpu, path);
    if (cpu.terminate)
        return std::nullopt;
    return result;
}

/* A native_frontend_* file's typed core called as C++ (guest::direct): nothing (false) when the
 * game stopped in it (its calls into the game throw fe::Terminated).  Every register but eax is kept.  A reg32 result is the eax the function leaves: eax set to it (in the
 * game too); after any other, eax spoilt there, so that code still reading what the callee left in
 * eax differs there. */
template <reg32 Address, auto Core, typename... Args>
auto feCore(App* app, CPU& cpu, Args... args)
{
    using Result = decltype(guest::direct<Address, Core>(app, cpu, args...));
    if constexpr (std::is_void_v<Result>)
    {
        try
        {
            guest::direct<Address, Core>(app, cpu, args...);
        }
        catch (const fe::Terminated&)
        {
            return false;
        }
        return !cpu.terminate;
    }
    else
    {
        try
        {
            const Result result = guest::direct<Address, Core>(app, cpu, args...);
            return cpu.terminate ? std::optional<Result>() : std::optional<Result>(result);
        }
        catch (const fe::Terminated&)
        {
            return std::optional<Result>();
        }
    }
}

/* A typed core called as C++ where the code calls it by address: after it eax the core's result,
 * as the call left it (`leaves`) -- the code's result of the functions that end in it.  Nothing
 * when the game stopped. */
std::optional<reg32> leaves(const std::optional<reg32>& result)
{
    return result;
}

// The same, `eax` (the register as the code has it, where a result is wanted) set to it.
std::optional<reg32> leaves(reg32* eax, const std::optional<reg32>& result)
{
    if (result && eax)
        *eax = *result;
    return result;
}

// A core's plain result, nothing when the game stopped in it.
std::optional<reg32> unlessStopped(CPU& cpu, reg32 result)
{
    if (cpu.terminate)
        return std::nullopt;
    return result;
}

std::optional<reg32> unlessStopped(CPU& cpu, const port::Result& result)
{
    if (!result || cpu.terminate)
        return std::nullopt;
    return reg32(*result);
}

/* The words of `words` from the `first`'th on, as the formatted ones take the rest. */
guest::Words wordsFrom(std::initializer_list<reg32> words, std::size_t first)
{
    guest::Words rest({});
    std::size_t i = 0;
    for (const reg32 word : words)
        if (i++ >= first && rest.count < guest::Words::kMost)
            rest.word[rest.count++] = word;
    return rest;
}

/* The language's text (port_text's 0x4d1850) of `id`, the registers as the code loads them for
 * the call.  Nothing when the game
 * stopped. */
std::optional<reg32> languageText(App* app, CPU& cpu, reg32 id, reg32 edx, reg32 ebx, reg32 ecx, reg32* eax = nullptr)
{
    return leaves(eax, unlessStopped(cpu, guest::direct<0x4d1850, port::text::lineText>(
                                              app, cpu, cpu.esp - 4, std::int32_t(id), edx)));
}

/* The tasks due run (the port's 0x4e7630) with 0, eax loaded with it as the code loads it.  False
 * when the game stopped. */
bool runTasks(App* app, CPU& cpu, reg32* eax = nullptr)
{
    return leaves(eax, unlessStopped(cpu, guest::direct<0x4e7630, port::stream::runDueTasks>(app, cpu, cpu.esp - 4, 0u)))
        .has_value();
}

/* A Windows function (`Function`), `words` handed: the port's own, called as C++ (native_windows.h).
 * What it leaves in eax back; nullopt when the game
 * stopped. */
template <reg32 Slot, auto Function, typename... Words>
std::optional<reg32> windows(App* app, CPU& cpu, Words... words)
{
    return guest::windows<Slot, Function>(app, cpu, words...);
}

/* Error (0x401010, cdecl) from `function` of `file` at `line` -- where it is from set first --
 * its format and words handed.  False when the game stopped. */
bool error(App* app, CPU& cpu, reg32 file, reg32 function, reg32 line, std::initializer_list<reg32> words)
{
    where(app, file, function, line);
    guest::Words rest({});
    for (const reg32* word = words.begin() + 1; word < words.end(); ++word)
        rest.word[rest.count++] = *word;
    guest::direct<0x401010, ea::error>(app, cpu, *words.begin(), rest);
    return !cpu.terminate;
}

/* The big-endian number of `bytes` (2 to 4) at `address`. */
reg32 bigEndianAt(App* app, reg32 address, reg32 bytes)
{
    const reg32 word = at(app, address);
    const reg32 swapped = word >> 24 | (word >> 8 & 0xff00) | (word << 8 & 0xff0000) | word << 24;
    return swapped >> ((0x20 - 8 * bytes) & 0x1f);
}

/* The length of the string at `text`, as strlen. */
reg32 lengthOf(App* app, reg32 text)
{
    reg32 length = 0;
    while (byte(app, text + length) != 0)
        ++length;
    return length;
}

/* The string at `from` copied to `to`, its ending too. */
void copyText(App* app, reg32 to, reg32 from)
{
    for (reg32 i = 0;; ++i)
    {
        const reg8 c = byte(app, from + i);
        putByte(app, to + i, c);
        if (c == 0)
            return;
    }
}

/* The registers the game's code keeps for its caller -- all but eax and
 * esp -- put back when the thunk is done. */
class Kept
{
public:
    explicit Kept(x86::Args& args)
        : args_(args), ebx_(args.ebx), ecx_(args.ecx), edx_(args.edx), esi_(args.esi), edi_(args.edi), ebp_(args.ebp)
    {
    }
    ~Kept()
    {
        args_.ebx = ebx_;
        args_.ecx = ecx_;
        args_.edx = edx_;
        args_.esi = esi_;
        args_.edi = edi_;
        args_.ebp = ebp_;
    }
    Kept(const Kept&) = delete;
    Kept& operator=(const Kept&) = delete;

private:
    x86::Args& args_;
    const reg32 ebx_, ecx_, edx_, esi_, edi_, ebp_;
};

/* A thunk's way out: eax the core's result, the return address popped;
 * nothing when the game stopped. */
x86::reg32 returns(CPU& cpu, x86::Args& args, const std::optional<reg32>& result)
{
    if (result)
    {
        args.eax = *result;
        cpu.esp += 4;
    }
    return args.eax;
}

// The flags loadFile and loadFileQuietly are handed here.
constexpr reg32 kLoadFlags = 0x10;

/* fileExists (0x4e0e60): whether the file named `name` is there. */
std::optional<bool> fileExists(App* app, CPU& cpu, reg32 name)
{
    return guest::direct<0x4e0e60, ea::fileThere>(app, cpu, name);
}

/* loadFile (0x4e0ec0): the file named `name` loaded whole into a block, 0 if
 * it is not. */
std::optional<reg32> loadFile(App* app, CPU& cpu, reg32 name, reg32 flags)
{
    return guest::direct<0x4e0ec0, ea::loadWhole>(app, cpu, name, flags);
}

/* loadFileQuietly (0x4e0fe0): loadFile without the complaint at a missing file. */
std::optional<reg32> loadQuietly(App* app, CPU& cpu, reg32 name, reg32 flags)
{
    return guest::direct<0x4e0fe0, ea::loadQuietly>(app, cpu, name, flags);
}

/* The memory's block allocated (0x4e1620), named. */
std::optional<reg32> allocate(App* app, CPU& cpu, reg32 name, reg32 bytes, reg32 flags)
{
    // edx and ebx as the code hands them, which the allocator keeps: the code's own after it.
    return guest::direct<0x4e1620, ea::reserved>(app, cpu, name, bytes, flags);
}

/* The memory's block freed (0x4e1890). */
std::optional<reg32> freeBlock(App* app, CPU& cpu, reg32 block)
{
    return guest::direct<0x4e1890, ea::purgeBlock>(app, cpu, block);
}

/* The size asked for of the block `block` (0x4e15d0); nullopt when the game stopped. */
std::optional<reg32> blockSize(App* app, CPU& cpu, reg32 block)
{
    const reg32 size = guest::direct<0x4e15d0, ea::askedSize>(app, cpu, block);
    if (cpu.terminate)
        return std::nullopt;
    return size;
}

/* EA's closehandle (0x4e2da0): the handle `handle` closed. */
std::optional<reg32> closeHandle(App* app, CPU& cpu, reg32 handle)
{
    return guest::direct<0x4e2da0, ea::closeChecked>(app, cpu, handle);
}

/* EA's readhandle (0x4e2de0): `count` bytes of the handle `handle` read to `to`. */
std::optional<reg32> readHandle(App* app, CPU& cpu, reg32 handle, reg32 to, reg32 count)
{
    return guest::direct<0x4e2de0, ea::readChecked>(app, cpu, handle, to, count);
}

/* EA's openhandle (0x4e28d0): the file named `name` opened to read, its handle and the two words
 * the open gives to the three addresses. */
std::optional<reg32> openHandle(App* app, CPU& cpu, reg32 name, reg32 handle, reg32 where, reg32 size)
{
    return guest::direct<0x4e28d0, ea::openToRead>(app, cpu, name, handle, where, size);
}

/* savefileadr (0x4eeae0): `count` bytes at `bytes` written to the file named `name`; how many
 * back. */
std::optional<reg32> saveFile(App* app, CPU& cpu, reg32 name, reg32 bytes, reg32 count)
{
    return guest::direct<0x4eeae0, load::blockSaved>(app, cpu, name, bytes, count);
}

/* strcmp (0x4ee310): the strings at `a` and `b` compared, 0 alike. */
std::optional<reg32> compareText(App* app, CPU& cpu, reg32 a, reg32 b, reg32* eax = nullptr)
{
    return leaves(eax, unlessStopped(cpu, guest::direct<0x4ee310, watcom::clib::strcmp>(app, cpu, a, b)));
}

/* sprintf (0x4df690, cdecl): its words, the buffer first. */
bool print(App* app, CPU& cpu, std::initializer_list<reg32> words, reg32* eax = nullptr)
{
    const reg32* const word = words.begin();
    const reg32 printed = guest::direct<0x4df690, watcom::clib::sprintf>(app, cpu, word[0], word[1], wordsFrom(words, 2));
    if (cpu.terminate)
        return false;
    if (eax)
        *eax = printed;
    return true;
}

// The kinds of race a replay may be of, and the language's text naming each
// (the switch's table at 0x446050); the tracks' texts from 0x66 on, the
// laps' 5; the last track; the replay's cars' records' length.
constexpr reg32 kRaceKinds = 4;
constexpr reg32 kKindTexts[kRaceKinds] = {0xd5, 0xd6, 0xd7, 0xdc};
constexpr reg32 kTrackTexts = 0x66, kLapsText = 5, kLastTrack = 8, kReplayCarBytes = 0x6c;

/* What the replays' screen shows of the replay named `name` (an empty name
 * made replay.rp3 first): loaded whole (loadFileQuietly), and if its track
 * (0 to 8) and kind of race are known and it checks out (0x478090,
 * 0x478100), its track's name, its laps, its kind of race and its player's
 * car's name printed into the screen's four lines (0x558d0c on); then freed.
 * What free leaves in eax back -- or fileExists' or loadFileQuietly's 0;
 * nullopt when the game stopped. */
std::optional<reg32> showReplayInfo(App* app, CPU& cpu, reg32 name)
{
    // sprintf's words past those handed are the frame's: the caller's
    // registers the thunk keeps where the code pushes them.
    const GuestFrame frame(cpu, 0x18, 0);
    if (byte(app, name) == 0)
        copyText(app, name, kReplayName);
    const std::optional<bool> there = fileExists(app, cpu, name);
    if (!there)
        return std::nullopt;
    if (!*there)
        return 0;
    const std::optional<reg32> replay = loadQuietly(app, cpu, name, kLoadFlags);
    if (!replay || *replay == 0)
        return replay;
    const sreg32 track = sreg32(at(app, *replay + kReplayTrack));
    const reg32 kind = at(app, *replay + kReplayKind);
    const reg32 kindText = kind < kRaceKinds ? kKindTexts[kind] : 0xffffffff;
    if (track <= sreg32(kLastTrack) && track >= 0 && sreg32(kindText) > 0)
    {
        const std::optional<reg32> valid = leaves(feCore<0x478090, frontend::loada::savedResolvePlayers>(app, cpu, *replay));
        if (!valid)
            return std::nullopt;
        if (*valid != 0)
        {
            const std::optional<reg32> checked = leaves(feCore<0x478100, frontend::loada::savedResolveEntries>(app, cpu, *replay));
            if (!checked)
                return std::nullopt;
            if (*checked != 0)
            {
                const reg32 player = at(app, *replay + kReplayPlayer);
                const reg32 car = at(app, *replay + player * kReplayCarBytes + kReplayCars);
                // The language's text: the replay, the car and the kind's text, as the code has them there.
                const auto text = [&](reg32 id) { return languageText(app, cpu, id, *replay, car, kindText); };
                const std::optional<reg32> trackName = text(reg32(track) + kTrackTexts);
                if (!trackName || !print(app, cpu, {kReplayTrackLine, kPercentS, *trackName}))
                    return std::nullopt;
                const std::optional<reg32> laps = text(kLapsText);
                if (!laps || !print(app, cpu, {kReplayLapsLine, kLapsFormat, at(app, *replay + kReplayLaps), *laps}))
                    return std::nullopt;
                const std::optional<reg32> kindName = text(kindText);
                if (!kindName || !print(app, cpu, {kReplayKindLine, kPercentS, *kindName}))
                    return std::nullopt;
                const std::optional<reg32> carName = guest::direct<0x439250, nameOf>(app, cpu, car);
                if (!carName || !print(app, cpu, {kReplayCarLine, kPercentS, *carName}))
                    return std::nullopt;
            }
        }
    }
    return freeBlock(app, cpu, *replay);
}

/* A file kept in the replays' folder: `from` (fileExists, loadFile) written
 * as "<the replays' folder><name>.<extension>" (sprintf'd into the frame;
 * savefileadr) and freed.  1 if it was there, else what fileExists or
 * loadFile gave; nullopt when the game stopped. */
std::optional<reg32> keepAs(App* app, CPU& cpu, reg32 name, reg32 extension, reg32 from)
{
    const GuestFrame frame(cpu, 0x10, 0x100);
    const reg32 path = frame - 0x100;
    if (!print(app, cpu, {path, kKeptFormat, kReplayFolder, name, extension}))
        return std::nullopt;
    const std::optional<bool> there = fileExists(app, cpu, from);
    if (!there)
        return std::nullopt;
    if (!*there)
        return 0;
    const std::optional<reg32> block = loadFile(app, cpu, from, kLoadFlags);
    if (!block || *block == 0)
        return block;
    const std::optional<reg32> size = blockSize(app, cpu, *block);
    if (!size || !saveFile(app, cpu, path, *block, *size) || !freeBlock(app, cpu, *block))
        return std::nullopt;
    return 1;
}

bool saveSettings(App* app, CPU& cpu, reg32& eax);

/* A replay picked from the list made the race's replay and loaded: an empty
 * name is the last race's (replay.rp3 itself); another file's loaded whole
 * (loadFileQuietly), replay.rp3 deleted and written anew from it
 * (savefileadr).  Then config.dat saved (saveSettings) and the replay loaded
 * (0x4bcb00).  1 if it was; nullopt when the game stopped. */
std::optional<reg32> loadKept(App* app, CPU& cpu, reg32 name)
{
    const GuestFrame frame(cpu, 0x14, 0);
    if (byte(app, name) != 0)
    {
        const std::optional<reg32> replay = loadQuietly(app, cpu, name, kLoadFlags);
        if (!replay)
            return std::nullopt;
        if (*replay == 0)
            return 0;
        if (!unlinkFile(app, cpu, kReplayName))
            return std::nullopt;
        const std::optional<reg32> size = blockSize(app, cpu, *replay);
        if (!size)
            return std::nullopt;
        const std::optional<reg32> written = saveFile(app, cpu, kReplayName, *replay, *size);
        if (!written || !freeBlock(app, cpu, *replay))
            return std::nullopt;
        if (*written == 0)
            return 0;
    }
    reg32 eaxLeft = 0;
    if (!asCalled(cpu, [&] { return saveSettings(app, cpu, eaxLeft); }))
        return std::nullopt;
    const std::optional<reg32> loaded = leaves(feCore<0x4bcb00, frontend::loada::readReplayKeepingSettings>(app, cpu));
    if (!loaded)
        return std::nullopt;
    return *loaded != 0 ? 1 : 0;
}

}  // namespace

/* 0x446060: showReplayInfo of the name eax. */
x86::reg32 readReplayInfo(App* app, CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    // The caller's edi and ebp where the code pushes them, under the four it
    // keeps above.
    put(app, cpu.esp - 0x14, args.edi);
    put(app, cpu.esp - 0x18, args.ebp);
    return returns(cpu, args, showReplayInfo(app, cpu, args.eax));
}

/* 0x446780: the ghost (ghost.gst) kept as "<name eax>.gst" in the replays'
 * folder (keepAs).  Called through the menus' table at 0x447a30. */
x86::reg32 saveGhostAs(App* app, CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    return returns(cpu, args, keepAs(app, cpu, args.eax, kGhostExtension, kGhostName));
}

/* 0x446990: loadKept of the name eax. */
x86::reg32 loadKeptReplay(App* app, CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    return returns(cpu, args, loadKept(app, cpu, args.eax));
}

/* 0x446a10: the last race's replay (replay.rp3) kept as "<name eax>.rp3" in
 * the replays' folder (keepAs).  Called through the menus' table at
 * 0x447a30. */
x86::reg32 saveReplayAs(App* app, CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    return returns(cpu, args, keepAs(app, cpu, args.eax, kReplayExtension, kReplayName));
}

namespace
{

// The replays' list's menu record: its entries' count, the one chosen, its
// block; the language's text for the last race's entry.
constexpr reg32 kListCount = 0x48, kListChosen = 0x4c, kListBlock = 0x70, kLastRaceText = 0x143;

/* The name at `name` cut at its first '.' -- with none, byte 0 of the
 * guest's memory cleared instead, as the code does. */
void cutAtDot(App* app, reg32 name)
{
    reg32 dot = name;
    while (byte(app, dot) != '.')
    {
        if (byte(app, dot) == 0)
        {
            dot = 0;
            break;
        }
        ++dot;
    }
    putByte(app, dot, 0);
}

/* The replays' list (the menu's record `list`) made of the files of
 * `extension` in the replays' folder: its block made anew (200 entries of
 * 8 bytes -- a name's address and a byte -- then the names), the last
 * race's first ("rp3" and replay.rp3 there: the language's text 0x143), then
 * each file found (findfirst on "*.<extension>" sprintf'd into the frame,
 * findnext; its name cut at its '.') while there is room; the count and the
 * one chosen kept within it.  The directory (kept in the frame) as it was
 * after: what chdir leaves in eax back, nullopt when the game stopped. */
std::optional<reg32> listReplayFiles(App* app, CPU& cpu, reg32 list, reg32 extension)
{
    const GuestFrame frame(cpu, 0x14, 0x220);
    const reg32 directory = frame - 0x220, pattern = frame - 0x20, namesEndKept = frame - 0x14;
    const reg32 old = at(app, list + kListBlock);
    if (old != 0)
    {
        if (!freeBlock(app, cpu, old))
            return std::nullopt;
        put(app, list + kListBlock, 0);
    }
    put(app, list + kListCount, kListEntries);
    const std::optional<reg32> block = allocate(app, cpu, kNoName, kListBytes, 0);
    if (!block)
        return std::nullopt;
    put(app, list + kListBlock, *block);
    const reg32 entriesEnd = at(app, list + kListCount) * 8 + at(app, list + kListBlock);
    const reg32 namesEnd = entriesEnd + kListNamesEnd;
    // Kept in the frame above the pattern, as the code keeps it.
    put(app, namesEndKept, namesEnd);
    reg32 name = entriesEnd + 8;
    reg32 count = 0;
    // The count'th entry: the name's address, and its byte cleared.
    const auto enter = [&] {
        put(app, at(app, list + kListBlock) + count * 8, name);
        putByte(app, at(app, list + kListBlock) + count * 8 + 4, 0);
    };
    const std::optional<reg32> other = compareText(app, cpu, extension, kReplayExtension);
    if (!other)
        return std::nullopt;
    if (*other == 0)
    {
        const std::optional<bool> there = fileExists(app, cpu, kReplayName);
        if (!there)
            return std::nullopt;
        if (*there)
        {
            // The language's text, as the code has the registers there.
            const auto lastRace = [&] { return languageText(app, cpu, kLastRaceText, kReplayExtension, name, 0); };
            const std::optional<reg32> text = lastRace();
            if (!text)
                return std::nullopt;
            copyText(app, name, *text);
            enter();
            const std::optional<reg32> again = lastRace();
            if (!again)
                return std::nullopt;
            name += lengthOf(app, *again) + 1;
            count = 1;
        }
    }
    if (!guest::direct<0x4e33a0, ea::currentDirectory>(app, cpu, directory) || !changeDirectory(app, cpu, kReplayFolder)
        || !print(app, cpu, {pattern, kAllOfKind, extension}))
        return std::nullopt;
    std::optional<reg32> found = leaves(feCore<0x4f0330, frontend::wata::findFirstFile>(app, cpu, pattern));
    if (!found)
        return std::nullopt;
    if (*found != 0)
    {
        enter();
        cutAtDot(app, *found);
        copyText(app, name, *found);
        for (;;)
        {
            name += lengthOf(app, *found) + 1;
            ++count;
            if (sreg32(count) >= sreg32(kListEntries))
                break;
            found = leaves(feCore<0x4f0280, frontend::wata::findNextFile>(app, cpu));
            if (!found)
                return std::nullopt;
            if (*found == 0)
                break;
            enter();
            cutAtDot(app, *found);
            if (name + lengthOf(app, *found) >= namesEnd)
                break;
            copyText(app, name, *found);
        }
    }
    put(app, list + kListCount, count);
    if (count != 0)
    {
        if (sreg32(at(app, list + kListCount)) <= sreg32(at(app, list + kListChosen)))
            put(app, list + kListChosen, at(app, list + kListCount) - 1);
        if (sreg32(at(app, list + kListChosen)) < 0)
            put(app, list + kListChosen, 0);
    }
    else
        put(app, list + kListChosen, 0);
    return changeDirectory(app, cpu, directory);
}

}  // namespace

/* 0x4475a0: listReplayFiles of the menu's record eax, the extension edx;
 * edx left the record, as the code leaves it. */
x86::reg32 listReplays(App* app, CPU& cpu, x86::Args& args)
{
    const reg32 ebx = args.ebx, ecx = args.ecx, list = args.eax;
    const std::optional<reg32> left = listReplayFiles(app, cpu, list, args.edx);
    if (!left)
        return args.eax;
    args.eax = *left;
    args.ebx = ebx;
    args.ecx = ecx;
    args.edx = list;
    cpu.esp += 4;
    return args.eax;
}

namespace
{

// Where the records' two parts are in the table; chmod's mode making
// records.dat writable (S_IREAD | S_IWRITE).
constexpr reg32 kRecordsFirstAt = 0x16e0, kRecordsSecondAt = 4, kWritable = 0x180;

/* records.dat ("<[0x7a2fb4]>records.dat", sprintf'd into the frame)
 * loaded, if it is not already ([0x559408]): kept if its first word is the
 * version (0x10), else freed; none there, a table made empty (0x4197c bytes,
 * the version first).  Its two parts' addresses to 0x55940c and 0x559410.
 * False when the game stopped. */
bool loadRecordsTable(App* app, CPU& cpu)
{
    if (at(app, kRecords) != 0)
        return true;
    const GuestFrame frame(cpu, 0x10, 0x100);
    const reg32 path = frame - 0x100;
    if (!print(app, cpu, {path, kRecordsFormat, kRecordsFolder}))
        return false;
    const std::optional<bool> there = fileExists(app, cpu, path);
    if (!there)
        return false;
    if (*there)
    {
        // Loaded with flags 0: the code hands loadFile the table's address it
        // found 0 in edx.
        const std::optional<reg32> table = loadFile(app, cpu, path, 0);
        if (!table)
            return false;
        const reg32 version = at(app, *table);
        put(app, kRecords, *table);
        if (version != kRecordsVersion)
        {
            if (!freeBlock(app, cpu, *table))
                return false;
            put(app, kRecords, 0);
        }
    }
    if (at(app, kRecords) == 0)
    {
        const std::optional<reg32> table = allocate(app, cpu, kRecordsBlockName, kRecordsBytes, 0);
        if (!table)
            return false;
        put(app, kRecords, *table);
        guest::direct<0x4e070c, ea::clearBytes>(app, cpu, *table, kRecordsBytes);
        if (cpu.terminate)
            return false;
        put(app, at(app, kRecords), kRecordsVersion);
    }
    const reg32 table = at(app, kRecords);
    put(app, kRecordsFirst, table + kRecordsFirstAt);
    put(app, kRecordsSecond, table + kRecordsSecondAt);
    return true;
}

/* The records' table, if loaded, saved as records.dat (sprintf'd into the
 * frame; made writable first if it is there; savefileadr) and freed.  What
 * its last call leaves in eax is the code's result.  False when the game
 * stopped. */
bool saveRecordsTable(App* app, CPU& cpu, reg32& eax)
{
    if (at(app, kRecords) == 0)
        return true;
    const GuestFrame frame(cpu, 0x10, 0x100);
    const reg32 path = frame - 0x100;
    if (!print(app, cpu, {path, kRecordsFormat, kRecordsFolder}, &eax))
        return false;
    const std::optional<bool> there = fileExists(app, cpu, path);
    if (!there || (*there && !chmodFile(app, cpu, path, kWritable)))
        return false;
    if (!saveFile(app, cpu, path, at(app, kRecords), kRecordsBytes) || !freeBlock(app, cpu, at(app, kRecords)))
        return false;
    put(app, kRecords, 0);
    return true;
}

}  // namespace

/* 0x44f510: loadRecordsTable; eax the second part's address where it loaded
 * them, as it was where they were. */
x86::reg32 loadRecords(App* app, CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    const bool loaded = at(app, kRecords) != 0;
    if (!loadRecordsTable(app, cpu))
        return args.eax;
    if (!loaded)
        args.eax = at(app, kRecordsSecond);
    cpu.esp += 4;
    return args.eax;
}

/* 0x44f5f0: saveRecordsTable. */
x86::reg32 saveRecords(App* app, CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    reg32 eax = args.eax;
    if (saveRecordsTable(app, cpu, eax))
        cpu.esp += 4;
    args.eax = eax;
    return args.eax;
}

/* 0x472230: the language checked against the game's mode eax -- an empty
 * function, its body compiled away: it compares the language with 1 and 2
 * and eax with 1, and does nothing.  defaultConfig's call of it is gone. */
x86::reg32 languageStub(App*, CPU& cpu, x86::Args& args)
{
    cpu.esp += 4;
    return args.eax;
}

namespace
{

constexpr reg32 kLanguages = 6;

/* The language ([0x6fbb4c]) from its name at 0x7a2634 -- english, german,
 * french, spanish, italian, swedish (the names 8 bytes apart at 0x53afd8): 0
 * to 5 -- left as it is for another name.  The last strcmp's difference back. */
std::optional<reg32> pickLanguage(App* app, CPU& cpu, reg32* eax = nullptr)
{
    reg32 difference = 0;
    for (reg32 language = 0; language < kLanguages; ++language)
    {
        const std::optional<reg32> d = compareText(app, cpu, kLanguageNames + 8 * language, kLanguageName, eax);
        if (!d)
            return std::nullopt;
        difference = *d;
        if (difference == 0)
            gameSettings(app).language = language;
    }
    return difference;
}

/* The camera's four views chosen (0x6fbba8 on) made the race's view numbers
 * (0x6fbbc0 on): each the signed word at 0x55629e by the view chosen (the
 * dword below it read, its high half taken).  The last back. */
reg32 cameraViewsFromChosen(App* app)
{
    reg32 view = 0;
    for (std::size_t i = 0; i < 4; ++i)
    {
        view = reg32(sreg32(at(app, gameSettings(app).cameraViewsChosen[i] * 2 + kViewNumbers)) >> 16);
        gameSettings(app).cameraViews[i] = view;
    }
    return view;
}

}  // namespace

/* 0x472250: pickLanguage; eax the last strcmp's. */
x86::reg32 setLanguage(App* app, CPU& cpu, x86::Args& args)
{
    const reg32 edx = args.edx;
    const std::optional<reg32> difference = pickLanguage(app, cpu);
    if (!difference)
        return args.eax;
    args.eax = *difference;
    args.edx = edx;
    cpu.esp += 4;
    return args.eax;
}

/* 0x472300: cameraViewsFromChosen; eax the last. */
x86::reg32 setCameraViews(App* app, CPU& cpu, x86::Args& args)
{
    args.eax = cameraViewsFromChosen(app);
    cpu.esp += 4;
    return args.eax;
}

namespace
{

// What the defaults copy: the 8 bytes after config.dat's length, and a car's
// tuning (7 floats).
constexpr reg32 kDefaultHead = 0x55e8b8, kDefaultTuning = 0x55d024, kTuningWords = 7;
// A player's car list's length (makeCarId and the first car's index).
constexpr reg32 kCarListLength = 0x13;
// The players' controls' force feedback's defaults: the collisions', the
// road's and its grip's, the engine's.
constexpr reg32 kForceDefault = 0x64, kForceEngineDefault = 0x32;

/* A word's low half sign-extended, as the code reads one: a dword two bytes
 * below it, its high half taken (sar 16). */
reg32 lowHalf(reg32 word)
{
    return reg32(sreg32(word << 16) >> 16);
}

/* config.dat's settings made afresh, its defaults: the record cleared
 * (memset), its length first and the 8 bytes at 0x55e8b8 after it; the race
 * (one player, no replay, a random seed), the camera's views
 * (cameraViewsFromChosen), the screen's size and depth (0x4cef00 and
 * 0x4cf160, handed the track's and the laps' words, as the code hands them),
 * the two cars' lists (0x438ad0, 0x439990), the language (pickLanguage) and
 * the speedometer it sets, the sound's volumes and 3D, the game's options,
 * the two players' colours and tuning, the controls (0x4904f0, each player),
 * and what a demo's command line sets; the controls' menus told (0x485880).
 * With the port's change of the port's track detail change in it: View Distance
 * starts Full.  What its last call leaves in eax is the code's result.  False
 * when the game stopped. */
bool makeDefaultSettings(App* app, CPU& cpu, reg32& eax)
{
    // At the code's depth: Error, which a callee may raise, sees where.
    const GuestFrame frame(cpu, 0x18, 0);
    GameSettings& settings = gameSettings(app);
    if (!leaves(&eax, unlessStopped(cpu, guest::direct<0x4e0640, watcom::clib::memset>(
                                            app, cpu, SETTINGS_ADDRESS(fileSize), 0u, kConfigBytes))))
        return false;
    settings.fileSize = kConfigBytes;
    put(app, SETTINGS_ADDRESS(fileSize) + 4, at(app, kDefaultHead));
    put(app, SETTINGS_ADDRESS(fileSize) + 8, at(app, kDefaultHead + 4));
    const std::optional<reg32> seed = leaves(&eax, feCore<0x4b56d0, frontend::wina::tickCount>(app, cpu));
    if (!seed)
        return false;
    settings.raceSeed = *seed;
    settings.unlockedAll = 0;
    settings.unlockedCars = 0;
    settings.replayMode = 0;
    settings.playMode = 0;
    settings.raceType = 0;
    settings.cameraViewsChosen[0] = 1;
    settings.cameraViewsChosen[2] = 3;
    const reg32 track = lowHalf(settings.track);
    settings.cameraViewsChosen[3] = 0;
    settings.cameraViewsChosen[1] = 2;
    const std::optional<reg32> width = leaves(&eax, feCore<0x4cef00, frontend::misc::recordsHolder>(app, cpu, track));
    if (!width)
        return false;
    settings.recordsHolder = lowHalf(*width);
    cameraViewsFromChosen(app);
    settings.laps = 2;
    settings.backwards = 0;
    settings.mirrored = 0;
    settings.night = 0;
    settings.weather = 0;
    settings.seriesRace = 0;
    for (const std::size_t i : {0, 5, 2, 3, 4, 1})
        settings.chosenOptions[i] = 0;
    settings.track = 0;
    settings.seriesVariant = 0;
    const reg32 laps = lowHalf(settings.laps);
    settings.unknown19ac = 1;
    settings.pursuitOption = 1;
    const std::optional<reg32> depth = leaves(&eax, feCore<0x4cf160, frontend::menue::lapsToOption>(app, cpu, laps));
    if (!depth)
        return false;
    settings.lapsClass = lowHalf(*depth);
    if (!guest::direct<0x438ad0, makeId>(app, cpu, SETTINGS_ADDRESS(players[0].carId), kCarListLength))
        return false;
    const reg32 first = guest::direct<0x439990, placeById>(app, cpu, SETTINGS_ADDRESS(players[0].carId));
    if (cpu.terminate)
        return false;
    settings.players[0].car = first;
    settings.players[0].unknown1c = 0;
    settings.players[0].transmission = 1;
    if (!guest::direct<0x438ad0, makeId>(app, cpu, SETTINGS_ADDRESS(players[1].carId), kCarListLength))
        return false;
    const reg32 second = guest::direct<0x439990, placeById>(app, cpu, SETTINGS_ADDRESS(players[1].carId));
    if (cpu.terminate)
        return false;
    settings.players[1].car = second;
    settings.players[1].unknown1c = 0;
    settings.opponentClass = 2;
    settings.trafficLevel = 2;
    settings.noTraffic = 0;
    settings.players[1].transmission = 1;
    if (!pickLanguage(app, cpu, &eax))
        return false;
    // The speedometer: English's (0) in mph, another language's in km/h.
    const reg32 speedometer = settings.language == 0 ? 1 : 3;
    for (const std::size_t i : {2, 4, 1, 3, 5, 0})
        settings.hud[i].speedometerForm = speedometer;
    settings.menuMusicVolume = 0x64;
    settings.raceMusicVolume = 0x64;
    settings.effectsVolume = 0x64;
    settings.speechVolume = 0x7f;
    settings.engineVolume = 0x64;
    settings.soundRateKind = 1;
    settings.soundFlag98 = 1;
    settings.musicMode = 2;
    settings.menuMusicAsked = 0;
    settings.soundSurround = 0;
    x86::Args threeDRegisters;
    const std::optional<reg32> threeD = leaves(&eax, guest::pointerCall(app, cpu, kIs3DSoundAvailable, threeDRegisters));
    if (!threeD)
        return false;
    settings.soundSetup = *threeD != 0 ? 3 : 0;
    settings.feedKeys[0] = 1;
    settings.feedKeys[1] = 2;
    settings.feedKeys[2] = 3;
    settings.setting504 = 3;
    settings.deviceCount = 0;
    settings.forceDeviceCount = 0;
    settings.unknown1868 = 2;
    settings.screenMode = 0;
    settings.viewDistance = 0;  // the port's (the port's track detail change): View Distance Full, was Far (1)
    settings.detailLevel = 0;
    settings.useSpecular = 1;
    settings.useLights = 1;
    settings.skyDetail = 0;
    settings.effectsLevel = 0;
    settings.projectedText = 1;
    settings.detailFlagFc = 1;
    settings.comPort = 0;
    settings.chosenModem = 0;
    settings.networkLobby = 0;
    settings.networkGame = 0;
    settings.players[0].colour[3] = 0x7f;
    settings.screenWidth = 0x280;
    settings.viewAngle = 0;
    settings.players[1].colour[0] = 0xf;
    settings.players[1].colour[3] = 0x7f;
    settings.players[0].name[0] = 0;
    settings.players[1].colour[1] = 0;
    settings.players[1].colour[2] = 0;
    settings.players[1].name[0] = 0;
    settings.players[0].colour[0] = 0;
    settings.players[0].colour[1] = 0;
    settings.players[0].colour[2] = 0;
    for (reg32 i = 0; i < kTuningWords; ++i)
        put(app, SETTINGS_ADDRESS(players[0].tuning) + i * 4, at(app, kDefaultTuning + i * 4));
    for (reg32 i = 0; i < kTuningWords; ++i)
        put(app, SETTINGS_ADDRESS(players[1].tuning) + i * 4, at(app, kDefaultTuning + i * 4));
    copyText(app, SETTINGS_ADDRESS(phoneNumber), kConfigBlockName);
    // The language checked against the game's mode (0x472230) does nothing.
    if (!guest::direct<0x4904f0, controls::defaultSets>(app, cpu, reg32(0))
        || !guest::direct<0x4904f0, controls::defaultSets>(app, cpu, reg32(1)))
        return false;
    const reg8 command = byte(app, kCommandFlags);
    // What the code has in eax at the controls' menus' call: 0, or the demo's laps' class.
    reg32 left = 0;
    settings.forceCollision = kForceDefault;
    settings.forceRoad = kForceDefault;
    settings.forceEngine = kForceEngineDefault;
    settings.forceRoadGrip = kForceDefault;
    settings.forceBodyRoll = 0;
    if ((command & kDemoCommand) != 0)
    {
        settings.seriesVariant = 0;
        settings.noTraffic = 0;
        settings.laps = 1;
        const reg32 demoLaps = lowHalf(settings.laps);
        settings.raceType = 3;
        settings.track = 3;
        const std::optional<reg32> demoDepth =
            leaves(&eax, feCore<0x4cf160, frontend::menue::lapsToOption>(app, cpu, demoLaps));
        if (!demoDepth)
            return false;
        settings.opponentClass = 2;
        settings.lapsClass = left = lowHalf(*demoDepth);
    }
    else if ((command & kOtherCommand) != 0)
    {
        settings.seriesVariant = 0;
        settings.raceType = 0;
        settings.track = 3;
    }
    return leaves(&eax, feCore<0x485880, frontend::simd::hudDefaults>(app, cpu)).has_value();
}

}  // namespace

/* 0x4723f0: makeDefaultSettings. */
x86::reg32 defaultConfig(App* app, CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    reg32 eax = args.eax;
    if (makeDefaultSettings(app, cpu, eax))
        cpu.esp += 4;
    args.eax = eax;
    return args.eax;
}

namespace
{

// config.dat not to be saved, when set; the copy's word cleared before it is.
constexpr reg32 kConfigNotSaved = 0x55e88c, kConfigCleared = 0x1860;

/* config.dat saved: a copy of the settings (allocated), its +0x1860
 * cleared, written as "<[0x7a3034]>config.dat" (sprintf'd into the frame;
 * savefileadr) and freed -- unless the game runs from a demo's command line,
 * a replay is shown, there is no room for it (0x4516a0), or [0x55e88c] says
 * not to.  What its last call leaves in eax is the code's result.  False
 * when the game stopped. */
bool saveSettings(App* app, CPU& cpu, reg32& eax)
{
    if ((byte(app, kCommandFlags) & kDemoCommand) != 0 || gameSettings(app).replayMode != 0)
        return true;
    const GuestFrame frame(cpu, 0x18, 0xfc);
    const reg32 path = frame - 0xfc;
    const std::optional<reg32> room = leaves(&eax, feCore<0x4516a0, frontend::loada::diskHasRoom>(app, cpu, kConfigBytes));
    if (!room)
        return false;
    if (*room == 0 || at(app, kConfigNotSaved) != 0)
        return true;
    const std::optional<reg32> copy = allocate(app, cpu, kConfigBlockName, kConfigBytes, 0);
    if (!copy)
        return false;
    for (reg32 i = 0; i < kConfigBytes; i += 4)
        put(app, *copy + i, at(app, SETTINGS_ADDRESS(fileSize) + i));
    put(app, *copy + kConfigCleared, 0);
    return print(app, cpu, {path, kConfigFormat, kSaveFolder}, &eax) && saveFile(app, cpu, path, *copy, kConfigBytes)
           && freeBlock(app, cpu, *copy);
}

/* The last write time of the file named `name` into the 8 bytes at `to`
 * (_lopen, GetFileTime into the frame, _lclose): the default (0x472160,
 * among the code) where it is not opened -- _lopen's 0, its HFILE_ERROR (-1)
 * taken for a file.  False when the game stopped. */
bool writeTimeOf(App* app, CPU& cpu, reg32 name, reg32 to)
{
    const GuestFrame frame(cpu, 0x14, 0x10);
    const reg32 time = frame - 0x10;
    put(app, time, at(app, kNoTime));
    put(app, time + 4, at(app, kNoTime + 4));
    const std::optional<reg32> file = windows<kLopen, &host::openFileByName>(app, cpu, name, 0);
    if (!file)
        return false;
    if (*file != 0
        && (!windows<kGetFileTime, &host::fileTime>(app, cpu, *file, 0, 0, time)
            || !windows<kLclose, &host::closeFileByName>(app, cpu, *file)))
        return false;
    put(app, to, at(app, time));
    put(app, to + 4, at(app, time + 4));
    return true;
}

}  // namespace

/* 1 if the file named `name` was written before the one named `other`
 * (writeTimeOf each into the frame, CompareFileTime's -1), else 0; nullopt
 * when the game stopped. */
std::optional<reg32> writtenBefore(App* app, CPU& cpu, reg32 name, reg32 other)
{
    const GuestFrame frame(cpu, 0xc, 0x10);
    const reg32 first = frame - 0x10, second = frame - 8;
    if (!asCalled(cpu, [&] { return writeTimeOf(app, cpu, name, first); })
        || !asCalled(cpu, [&] { return writeTimeOf(app, cpu, other, second); }))
        return std::nullopt;
    // eax and edx as the code has them there: what CompareFileTime leaves in
    // edx is fileOlder's.
    const std::optional<reg32> order =
        windows<kCompareFileTime, &host::compareFileTime>(app, cpu, first, second);
    if (!order)
        return std::nullopt;
    return *order == 0xffffffff ? 1 : 0;
}

/* 0x472820: saveSettings. */
x86::reg32 saveConfig(App* app, CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    reg32 eax = args.eax;
    if (saveSettings(app, cpu, eax))
        cpu.esp += 4;
    args.eax = eax;
    return args.eax;
}

/* 0x4728e0: writeTimeOf the name eax into esi; esi back in eax, esi itself
 * left at the frame's top, past the copy the code makes there. */
x86::reg32 fileWriteTime(App* app, CPU& cpu, x86::Args& args)
{
    const reg32 ecx = args.ecx, edx = args.edx, to = args.esi;
    if (!writeTimeOf(app, cpu, args.eax, to))
        return args.eax;
    args.eax = to;
    args.ecx = ecx;
    args.edx = edx;
    args.esi = cpu.esp - 0x14;
    cpu.esp += 4;
    return args.eax;
}

/* 0x472940: writtenBefore of the names eax and edx; edx as CompareFileTime
 * leaves it.  loadConfig asks it of config.dat and nfs3.exe. */
x86::reg32 fileOlder(App* app, CPU& cpu, x86::Args& args)
{
    const reg32 ecx = args.ecx;
    const std::optional<reg32> older = writtenBefore(app, cpu, args.eax, args.edx);
    if (!older)
        return args.eax;
    args.eax = *older;
    args.ecx = ecx;
    cpu.esp += 4;
    return args.eax;
}

namespace
{

// nfs3.exe's path's room (GetModuleFileNameA); the allocation's flags for
// config.dat's block; the effects' volume as the sound has it.
constexpr reg32 kExePathBytes = 0xf9, kConfigBlockFlags = 0x10, kEffectsVolumeNow = 0x552920;
// The race's kind a demo runs.
constexpr reg32 kDemoRaceType = 3;

/* config.dat loaded at the start ("<[0x7a3034]>config.dat", sprintf'd into
 * the frame; opened unless the game runs from a demo's command line, its
 * handle and the open's two words in the frame): read into a block and
 * copied over the settings if it is 0x20f0 bytes and not older than nfs3.exe
 * (fileOlder through guest::direct), the sound's volume
 * taken from it; else closed and deleted (an old one), and the settings made
 * afresh (makeDefaultSettings), [0x67900c] set -- as for another length or
 * another version.  Then the race's kind for a demo or the other command
 * line, the random seed and byte, the language, the detail ([0x6fbb54],
 * 0x4b62a0), and the controllers (0x472b60) if they are known.  What its
 * last call leaves in eax is the code's result.  False when the game
 * stopped. */
bool loadSettings(App* app, CPU& cpu, reg32& eax)
{
    GameSettings& settings = gameSettings(app);
    const GuestFrame frame(cpu, 0x18, 0x208);
    const reg32 path = frame - 0x208, exe = frame - 0x10c;
    const reg32 where = frame - 0x10, handle = frame - 0xc, size = frame - 8;
    put(app, handle, 0);
    put(app, where, 0);
    if (!windows<kGetModuleFileNameA, &host::moduleFileName>(app, cpu, 0, exe, kExePathBytes)
        || !print(app, cpu, {path, kConfigFormat, kSaveFolder}, &eax) || !runTasks(app, cpu, &eax))
        return false;
    if ((byte(app, kCommandFlags) & kDemoCommand) == 0)
    {
        const std::optional<bool> there = fileExists(app, cpu, path);
        if (!there || (*there && !openHandle(app, cpu, path, handle, where, size)))
            return false;
    }
    if (!runTasks(app, cpu, &eax))
        return false;
    // The effects' volume, where the code took it from config.dat and made
    // nothing afresh after: still in eax at the random number's call.
    std::optional<reg32> volume;
    const auto makeAfresh = [&] {
        put(app, kConfigMadeAfresh, 1);
        return asCalled(cpu, [&] { return makeDefaultSettings(app, cpu, eax); });
    };
    if (at(app, handle) == 0)
    {
        if (!makeAfresh())
            return false;
    }
    else
    {
        const reg32 bytes = at(app, size);
        reg32 older = 1;
        if (bytes == kConfigBytes)
        {
            const std::optional<reg32> o = leaves(&eax, guest::direct<0x472940, writtenBefore>(app, cpu, path, exe));
            if (!o)
                return false;
            older = *o;
        }
        if (older != 0)
        {
            put(app, kConfigMadeAfresh, 1);
            if (!closeHandle(app, cpu, at(app, handle)) || !unlinkFile(app, cpu, path))
                return false;
            if (!asCalled(cpu, [&] { return makeDefaultSettings(app, cpu, eax); }))
                return false;
        }
        else
        {
            put(app, kConfigMadeAfresh, 0);
            const std::optional<reg32> block = allocate(app, cpu, kConfigBlockName, at(app, size), kConfigBlockFlags);
            if (!block || !readHandle(app, cpu, at(app, handle), *block, at(app, size))
                || !closeHandle(app, cpu, at(app, handle)))
                return false;
            for (reg32 i = 0; i < kConfigBytes; i += 4)
                put(app, SETTINGS_ADDRESS(fileSize) + i, at(app, *block + i));
            if (!freeBlock(app, cpu, *block))
                return false;
            volume = settings.effectsVolume;
            put(app, kEffectsVolumeNow, *volume);
        }
    }
    // Another length, or another version: made afresh too.
    if (settings.fileSize != kConfigBytes)
    {
        volume.reset();
        if (!makeAfresh())
            return false;
    }
    if (at(app, kGameVersion) != settings.configVersion)
        put(app, kConfigMadeAfresh, 1);
    const reg8 command = byte(app, kCommandFlags);
    if ((command & kDemoCommand) != 0)
        settings.raceType = kDemoRaceType;
    else if ((command & kOtherCommand) != 0)
        settings.raceType = 0;
    const std::optional<reg32> seed = leaves(&eax, feCore<0x4b56d0, frontend::wina::tickCount>(app, cpu));
    if (!seed)
        return false;
    settings.raceSeed = *seed;
    const std::optional<reg32> random = leaves(&eax, feCore<0x4b56d0, frontend::wina::tickCount>(app, cpu));
    if (!random)
        return false;
    putByte(app, kRandomByte, reg8(*random));
    guest::direct<0x4e0dee, ea::randomSeeded>(app, cpu, settings.raceSeed);
    if (cpu.terminate || !pickLanguage(app, cpu, &eax))
        return false;
    settings.networkGame = 0;
    const std::optional<reg32> remote = leaves(&eax, feCore<0x4b62a0, frontend::loopa::installIsRemote>(app, cpu));
    if (!remote)
        return false;
    settings.installIsRemote = *remote;
    if (at(app, kControllersKnown) != 0)
    {
        if (!guest::direct<0x472b60, controls::loadDevices>(app, cpu))
            return false;
        put(app, kControllersKnown, 0);
    }
    return true;
}

}  // namespace

/* 0x472980: loadSettings. */
x86::reg32 loadConfig(App* app, CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    reg32 eax = args.eax;
    if (loadSettings(app, cpu, eax))
        cpu.esp += 4;
    args.eax = eax;
    return args.eax;
}

namespace
{

// The replay read for showing, if any (cleared when replay.rp3 is not
// there); the seed a replay keeps, and the race's frames, its seed when no
// replay was read; whether the frames' log is kept (1); the replay's version.
constexpr reg32 kReplayRead = 0x7cda58, kReplaySeedKept = 0x55fe40, kRaceFrames = 0x7d3684;
constexpr reg32 kReplayLogKept = 0x55d000, kReplayVersion = 2;

/* The race just run saved as replay.rp3 (sprintf'd into the frame) -- if
 * there was one ([0x6fd3ac]), the race not a network's with its own players
 * only, and no network game on (0x4cf040): config.dat's settings copied to
 * its head, its version, whether the frames' log follows and the seed set,
 * checksummed (0x478010, 0x4781b0) and written (savefileadr); the log
 * appended (0x477ac0) to the file opened "rb+".  What its last call leaves
 * in eax is the code's result.  False when the game stopped. */
bool saveReplayFile(App* app, CPU& cpu, reg32& eax)
{
    const GameSettings& settings = gameSettings(app);
    if (settings.lastTickRaced == 0 || (sreg32(settings.playMode) > 1 && settings.networkGame != 0))
        return true;
    const GuestFrame frame(cpu, 0x18, 0x3c);
    const reg32 path = frame - 0x3c;
    const std::optional<reg32> network = leaves(&eax, feCore<0x4cf040, frontend::misc::postgameReplayExcluded>(app, cpu));
    if (!network)
        return false;
    if (*network != 0)
        return true;
    if (!print(app, cpu, {path, kRaceReplayName}, &eax))
        return false;
    if (at(app, kReplayRead) == 0)
        put(app, kReplaySeedKept, at(app, kRaceFrames));
    for (reg32 i = 0; i < kConfigBytes; i += 4)
        put(app, kRaceReplay + i, at(app, SETTINGS_ADDRESS(fileSize) + i));
    put(app, kRaceReplayVersion, kReplayVersion);
    put(app, kRaceReplayLogged, at(app, kReplayLogKept) == 1 ? 1 : 0);
    put(app, kRaceReplaySeed, at(app, kReplaySeedKept));
    if (!leaves(&eax, feCore<0x478010, frontend::pause::checkSaveable>(app, cpu, kRaceReplay)))
        return false;
    if (!leaves(&eax, feCore<0x4781b0, frontend::loada::savedNameEntries>(app, cpu, kRaceReplay))
        || !saveFile(app, cpu, path, kRaceReplay, kReplayBytes))
        return false;
    if (at(app, kReplayLogKept) != 1)
        return true;
    const std::optional<reg32> file =
        leaves(&eax, unlessStopped(cpu, guest::direct<0x4edfe8, watcom::clib::fopen>(app, cpu, path, kReadWrite)));
    if (!file)
        return false;
    if (*file == 0)
        return true;
    if (!guest::direct<0x477ac0, writeGhostTo>(app, cpu, *file, kReplayBytes))
        return false;
    return leaves(&eax, unlessStopped(cpu, guest::direct<0x4ee100, watcom::clib::fclose>(app, cpu, *file))).has_value();
}

/* replay.rp3 (sprintf'd into the frame) read into the race's replay
 * (0x7a3e50) -- its frames' log too (0x477b00) if it says one follows -- and
 * checked (0x478090, 0x478100): 1 if it was there and checks out, else 0
 * (and [0x7cda58] cleared when it is not there).  The open's handle and the
 * other two words it gives in the frame.  nullopt when the game stopped. */
std::optional<reg32> readReplayFile(App* app, CPU& cpu)
{
    const GuestFrame frame(cpu, 0x14, 0x5c);
    const reg32 path = frame - 0x5c, handle = frame - 0xc, size = frame - 8, where = frame - 4;
    put(app, handle, 0);
    put(app, where, 0);
    if (!print(app, cpu, {path, kRaceReplayName}) || !openHandle(app, cpu, path, handle, where, size))
        return std::nullopt;
    const reg32 file = at(app, handle);
    if (file == 0)
    {
        put(app, kReplayRead, 0);
        return 0;
    }
    if (!readHandle(app, cpu, file, kRaceReplay, kReplayBytes))
        return std::nullopt;
    if (at(app, kRaceReplayLogged) != 0 && !guest::direct<0x477b00, readGhostFrom>(app, cpu, file))
        return std::nullopt;
    if (!closeHandle(app, cpu, file))
        return std::nullopt;
    const std::optional<reg32> valid = leaves(feCore<0x478090, frontend::loada::savedResolvePlayers>(app, cpu, kRaceReplay));
    if (!valid)
        return std::nullopt;
    const std::optional<reg32> checked = leaves(feCore<0x478100, frontend::loada::savedResolveEntries>(app, cpu, kRaceReplay));
    if (!checked)
        return std::nullopt;
    return *valid != 0 && *checked != 0 ? 1 : 0;
}

}  // namespace

/* 0x4bd1e0: saveReplayFile. */
x86::reg32 saveRaceReplay(App* app, CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    reg32 eax = args.eax;
    if (saveReplayFile(app, cpu, eax))
        cpu.esp += 4;
    args.eax = eax;
    return args.eax;
}

/* 0x4bd2e0: readReplayFile. */
x86::reg32 readRaceReplay(App* app, CPU& cpu, x86::Args& args)
{
    const Kept kept(args);
    return returns(cpu, args, readReplayFile(app, cpu));
}

namespace
{

// The two kinds of BIG archive, as typeofbigfile names them, and their marks:
// 0xc0fb's first two bytes, "BIGF" its first four.
constexpr reg32 kBigC0fb = 1, kBigF = 2;
constexpr reg32 kC0fbMark = 0xc0fb, kBigFMark = 0x42494746;
// Where each kind's entries start, and the bytes of an entry's offset and of
// its size (its name after them).
constexpr reg32 kC0fbEntries = 6, kBigFEntries = 0x10, kC0fbNumber = 3, kBigFNumber = 4;

/* EA's typeofbigfile (cmn\locatbig.c): the kind of BIG archive at `archive`
 * -- kBigC0fb, kBigF, else 0; none at all, or not one, an Error while
 * [0x56440c] is set.  nullopt when the game stopped. */
std::optional<reg32> bigFileKind(App* app, CPU& cpu, reg32 archive)
{
    const GuestFrame frame(cpu, 0x18, 0);
    if (archive == 0 && at(app, kErrorsFatal) != 0
        && !error(app, cpu, kLocatbigC, kTypeOfBigFileName, 0x11, {kNoBigFile}))
        return std::nullopt;
    if (bigEndianAt(app, archive, 2) == kC0fbMark)
        return kBigC0fb;
    if (bigEndianAt(app, archive, 4) == kBigFMark)
        return kBigF;
    if (at(app, kErrorsFatal) != 0
        && !error(app, cpu, kLocatbigC, kTypeOfBigFileName, 0x21, {kNotBigFile, archive}))
        return std::nullopt;
    return 0;
}

/* The length of the BIG archive's header -- 0xc0fb's 16-bit one at 2 and
 * its 4 bytes, BIGF's at 0xc -- or 0 if it is none. */
std::optional<reg32> bigHeaderBytes(App* app, CPU& cpu, reg32 archive)
{
    const GuestFrame frame(cpu, 8, 0);
    const std::optional<reg32> kind = asCalled(cpu, [&] { return bigFileKind(app, cpu, archive); });
    if (!kind)
        return std::nullopt;
    if (*kind == kBigC0fb)
        return bigEndianAt(app, archive + 2, 2) + 4;
    if (*kind == kBigF)
        return bigEndianAt(app, archive + 0xc, 4);
    return 0;
}

/* From the 0xc0fb entry at `entry` on, the first whose name's first four
 * letters are those at `name`, case aside (each or-ed with 0x20), or where
 * the walk reaches `end` (compared signed, as the code does).  A name's end
 * is looked for a dword at a time from its fifth byte, by (dword - 0x01010101)
 * & 0x80808080: a byte over 0x80 passes for an end too, and where none of the
 * dword's four is 0 the walk stops there, as the code's does. */
reg32 skipToFourLetters(App* app, reg32 entry, reg32 end, reg32 name)
{
    const reg32 letters = at(app, name) | 0x20202020;
    const auto anEnd = [](reg32 dword) { return (dword - 0x1010101) & 0x80808080; };
    while (sreg32(entry) < sreg32(end))
    {
        if ((at(app, entry + kC0fbEntries) | 0x20202020) == letters)
            break;
        reg32 dword = entry + kC0fbEntries + 4;
        while (anEnd(at(app, dword)) == 0)
            dword += 4;
        reg32 i = 0;
        while (i < 4 && byte(app, dword + i) != 0)
            ++i;
        entry = dword + (i < 4 ? i + 1 : 4);
        if (i == 4)
            break;
    }
    return entry;
}

}  // namespace

/* EA's findbigentry: the entry of the BIG archive named `name` (case aside;
 * 0xc0fb's names' first four letters looked at first), or with no name the
 * index'th: its offset to [offsetTo] and its size to [sizeTo] (either may be
 * 0), its name's address back -- 0, and the two 0, if there is none.
 * nullopt when the game stopped. */
std::optional<BigEntry> findEntry(App* app, CPU& cpu, reg32 archive, reg32 name, reg32 index, reg32 offsetTo,
                                  reg32 sizeTo)
{
    const GuestFrame frame(cpu, 0xc, 0x10);
    const std::optional<reg32> header = asCalled(cpu, [&] { return bigHeaderBytes(app, cpu, archive); });
    if (!header)
        return std::nullopt;
    const reg32 end = archive + *header;
    const std::optional<reg32> kind = asCalled(cpu, [&] { return bigFileKind(app, cpu, archive); });
    if (!kind)
        return std::nullopt;
    BigEntry walked = {0, index, end};
    const auto none = [&] {
        if (offsetTo != 0)
            put(app, offsetTo, 0);
        if (sizeTo != 0)
            put(app, sizeTo, 0);
        return walked;
    };
    if (*kind != kBigC0fb && *kind != kBigF)
        return none();
    const reg32 number = *kind == kBigC0fb ? kC0fbNumber : kBigFNumber;
    walked.entry = archive + (*kind == kBigC0fb ? kC0fbEntries : kBigFEntries);
    if (walked.entry >= end)
        return none();
    for (reg32 counted = 0;; ++counted)
    {
        if (name != 0 && *kind == kBigC0fb)
            walked.entry = skipToFourLetters(app, walked.entry, end, name);
        bool match = name == 0 && counted == index;
        if (name != 0)
        {
            const std::optional<reg32> difference = leaves(
                unlessStopped(cpu, guest::direct<0x4e9ec0, watcom::clib::stricmp>(app, cpu, walked.entry + 2 * number, name)));
            if (!difference)
                return std::nullopt;
            match = *difference == 0;
        }
        if (match)
        {
            if (offsetTo != 0)
            {
                put(app, offsetTo, bigEndianAt(app, walked.entry, number));
                walked.last = offsetTo;
            }
            if (sizeTo != 0)
            {
                put(app, sizeTo, bigEndianAt(app, walked.entry + number, number));
                walked.last = 0x20 - 8 * number;  // the read's shift
            }
            walked.name = walked.entry + 2 * number;
            return walked;
        }
        walked.last = lengthOf(app, walked.entry + 2 * number);
        walked.entry += walked.last + 2 * number + 1;
        if (walked.entry >= end)
            return none();
    }
}

/* EA's locatebigentry: findEntry's, and an Error for an entry not found
 * while [0x56440c] is set.  Its `entry` what the code leaves in ebx: the
 * Error's function's name for a name not found. */
std::optional<BigEntry> locateEntry(App* app, CPU& cpu, reg32 archive, reg32 name, reg32 index, reg32 offsetTo,
                                    reg32 sizeTo)
{
    const GuestFrame frame(cpu, 0xc, 0);
    // The size's pointer handed on the stack.
    const std::optional<BigEntry> found =
        asCalled(cpu, [&] { return findEntry(app, cpu, archive, name, index, offsetTo, sizeTo); }, 4);
    if (!found || found->name != 0 || at(app, kErrorsFatal) == 0)
        return found;
    if (name != 0)
    {
        if (!error(app, cpu, kLocatbigC, kLocateBigEntryName, 0xef, {kEntryNotFound, name}))
            return std::nullopt;
        return BigEntry{0, kLocateBigEntryName, 0};
    }
    if (!error(app, cpu, kLocatbigC, kLocateBigEntryName, 0xf1, {kIndexNotFound, index}))
        return std::nullopt;
    return BigEntry{0, found->entry, 0};
}

/* 0x4e9f00, EA's typeofbigfile: bigFileKind of the archive eax. */
x86::reg32 typeOfBigFile(App* app, CPU& cpu, x86::Args& args)
{
    const std::optional<reg32> kind = bigFileKind(app, cpu, args.eax);
    if (!kind)
        return args.eax;
    args.eax = *kind;
    cpu.esp += 4;
    return args.eax;
}

/* 0x4e9fe0: bigHeaderBytes of the archive eax. */
x86::reg32 bigHeaderLength(App* app, CPU& cpu, x86::Args& args)
{
    const std::optional<reg32> bytes = bigHeaderBytes(app, cpu, args.eax);
    if (!bytes)
        return args.eax;
    args.eax = *bytes;
    cpu.esp += 4;
    return args.eax;
}

/* 0x4ea040: findEntry in the archive eax of the name edx or the index ebx,
 * [ecx] and [[esp+4]] its offset and size; ret 4.  ebx and ecx as its code
 * leaves them. */
x86::reg32 findBigEntry(App* app, CPU& cpu, x86::Args& args)
{
    const std::optional<BigEntry> found =
        findEntry(app, cpu, args.eax, args.edx, args.ebx, args.ecx, at(app, cpu.esp + 4));
    if (!found)
        return args.eax;
    args.eax = found->name;
    args.ebx = found->entry;
    args.ecx = found->last;
    cpu.esp += 8;
    return args.eax;
}

/* 0x4ea290, EA's locatebigentry: locateEntry, as findBigEntry's arguments;
 * ret 4.  ecx its result too, ebx as its code leaves it. */
x86::reg32 locateBigEntry(App* app, CPU& cpu, x86::Args& args)
{
    const std::optional<BigEntry> found =
        locateEntry(app, cpu, args.eax, args.edx, args.ebx, args.ecx, at(app, cpu.esp + 4));
    if (!found)
        return args.eax;
    args.eax = found->name;
    args.ebx = found->entry;
    args.ecx = found->name;
    cpu.esp += 8;
    return args.eax;
}

namespace
{

// The lines of savefileadr's two Errors.
constexpr reg32 kOpenFailedLine = 0x60, kWriteFailedLine = 0x6a;

/* What savefileadr gives back: how many bytes it wrote, and the line of the
 * Error it raised (0 for none), whose where its code leaves in ebx and edx. */
struct Saved
{
    reg32 written;
    reg32 errorLine;
};

/* EA's savefileadr (cmn\savefile.c): `count` bytes at `bytes` written to
 * the file named `name`, made anew (openhandlew -- the handle and its two
 * words in the frame -- writehandle, closehandle); how many were written, 0
 * if it did not open -- either failure an Error while [0x56440c] is set.
 * nullopt when the game stopped. */
std::optional<Saved> saveBlock(App* app, CPU& cpu, reg32 name, reg32 bytes, reg32 count)
{
    const GuestFrame frame(cpu, 0x10, 0xc);
    const reg32 handle = frame - 0xc;
    const std::optional<reg32> opened = guest::direct<0x4e2ae0, ea::openToWrite>(app, cpu, name, handle, handle + 4, handle + 8);
    if (!opened)
        return std::nullopt;
    if (*opened == 0)
    {
        if (at(app, kErrorsFatal) == 0)
            return Saved{0, 0};
        if (!error(app, cpu, kSavefileC, kSavefileadrName, kOpenFailedLine, {kOpenFailed, name}))
            return std::nullopt;
        return Saved{0, kOpenFailedLine};
    }
    const std::optional<reg32> written =
        guest::direct<0x4e2e20, ea::writeChecked>(app, cpu, at(app, handle), bytes, count);
    if (!written || !closeHandle(app, cpu, at(app, handle)))
        return std::nullopt;
    if (*written != 0 || at(app, kErrorsFatal) == 0)
        return Saved{*written, 0};
    if (!error(app, cpu, kSavefileC, kSavefileadrName, kWriteFailedLine, {kWriteFailed, name}))
        return std::nullopt;
    return Saved{0, kWriteFailedLine};
}

}  // namespace

/* savefileadr with its Errors off: [0x56440c] cleared about it, savefileadr
 * called by address.  How many bytes it wrote;
 * nullopt when the game stopped. */
std::optional<reg32> saveBlockQuietly(App* app, CPU& cpu, reg32 name, reg32 bytes, reg32 count)
{
    const reg32 fatal = at(app, kErrorsFatal);
    put(app, kErrorsFatal, 0);
    const std::optional<reg32> written = saveFile(app, cpu, name, bytes, count);
    if (written)
        put(app, kErrorsFatal, fatal);
    return written;
}

/* 0x4eeae0, EA's savefileadr: saveBlock of the name eax, the bytes edx, the
 * count ebx.  ebx and edx as its calls leave them, or as its Error's where
 * leaves them. */
x86::reg32 savefileadr(App* app, CPU& cpu, x86::Args& args)
{
    const reg32 ecx = args.ecx;
    const std::optional<Saved> saved = saveBlock(app, cpu, args.eax, args.edx, args.ebx);
    if (!saved)
        return args.eax;
    if (saved->errorLine == kOpenFailedLine)
    {
        args.ebx = kOpenFailedLine;
        args.edx = kSavefileC;
    }
    else if (saved->errorLine == kWriteFailedLine)
        args.ebx = kSavefileC;
    args.eax = saved->written;
    args.ecx = ecx;
    cpu.esp += 4;
    return args.eax;
}

/* 0x4eebb0: saveBlockQuietly, as savefileadr's arguments. */
x86::reg32 savefileadrQuietly(App* app, CPU& cpu, x86::Args& args)
{
    return returns(cpu, args, saveBlockQuietly(app, cpu, args.eax, args.edx, args.ebx));
}

namespace
{

/* The quarter wave's entry (16.16) for the angle's 1024th `step`: its low
 * byte the entry, counted back from the quarter's end in an odd quarter (bit
 * 8), negated in the second half (bit 9). */
reg32 quarterWaveAt(App* app, reg32 step)
{
    const reg32 index = (step >> 8 & 1) != 0 ? 0x100 - (step & 0xff) : step & 0xff;
    const reg32 value = at(app, index * 4 + kQuarterWave);
    return (step >> 9 & 1) != 0 ? 0 - value : value;
}

/* The angle's 64th left over (0 to 63) times 2 pi in 0.16 (0x6487e), less
 * its low 9 bits: the step towards the next 1024th. */
sreg32 stepLeft(reg32 angle)
{
    return sreg32(angle & 0x3f) * 0x6487e >> 9;
}

/* The wave's slope at `step` (its value a quarter on, a quarter back for the
 * cosine's) times the step left, as the code scales them (16.16 by 4, 0.16 by
 * 2^9, the product by 2^21): a 32-bit product, its overflow dropped. */
sreg32 slopeTimes(sreg32 slope, reg32 angle)
{
    return sreg32(reg32(slope >> 2) * reg32(stepLeft(angle))) >> 21;
}

}  // namespace

/* The sine of the angle (0x10000 a turn), 16.16: the quarter wave's entry
 * for its 1024th, and the cosine's times the 64th left added. */
reg32 sineOf(App* app, reg32 angle)
{
    const reg32 step = reg32(sreg32(angle) >> 6);
    const reg32 sine = quarterWaveAt(app, step);
    const reg32 cosine = quarterWaveAt(app, step + 0x100);
    return reg32(slopeTimes(sreg32(cosine), angle)) + sine;
}

/* The cosine of the angle, as sineOf: the quarter wave's entry a quarter on,
 * and the sine's times the 64th left taken off. */
reg32 cosineOf(App* app, reg32 angle)
{
    const reg32 step = reg32(sreg32(angle) >> 6);
    const reg32 cosine = quarterWaveAt(app, step + 0x100);
    const reg32 sine = quarterWaveAt(app, step);
    return reg32(slopeTimes(sreg32(0 - sine), angle)) + cosine;
}

/* 0x4f2380: sineOf the angle eax.  The race's and the menus' 3D call it. */
x86::reg32 fixedSine(App* app, CPU& cpu, x86::Args& args)
{
    args.eax = sineOf(app, args.eax);
    cpu.esp += 4;
    return args.eax;
}

/* 0x4f23f0: cosineOf the angle eax. */
x86::reg32 fixedCosine(App* app, CPU& cpu, x86::Args& args)
{
    args.eax = cosineOf(app, args.eax);
    cpu.esp += 4;
    return args.eax;
}

/* The typed cores of the thunks above, for callers that call them as C++ (native_load_saves.h). */
bool recordsTableLoaded(App* app, CPU& cpu)
{
    return loadRecordsTable(app, cpu);
}

std::optional<reg32> recordsTableSavedWith(App* app, CPU& cpu, reg32 eax)
{
    if (!saveRecordsTable(app, cpu, eax))
        return std::nullopt;
    return eax;  // what its last call leaves, as the code's result
}

std::optional<reg32> recordsTableSaved(App* app, CPU& cpu)
{
    return recordsTableSavedWith(app, cpu, 0);
}

std::optional<reg32> keptReplayLoaded(App* app, CPU& cpu, reg32 name)
{
    return loadKept(app, cpu, name);
}

std::optional<reg32> replayInfoShown(App* app, CPU& cpu, reg32 name)
{
    return showReplayInfo(app, cpu, name);
}

std::optional<reg32> replaysListed(App* app, CPU& cpu, reg32 list, reg32 extension)
{
    return listReplayFiles(app, cpu, list, extension);
}

std::optional<reg32> configSavedWith(App* app, CPU& cpu, reg32 eax)
{
    if (!saveSettings(app, cpu, eax))
        return std::nullopt;
    return eax;  // what its last call leaves, as the code's result
}

std::optional<reg32> configSaved(App* app, CPU& cpu)
{
    return configSavedWith(app, cpu, 0);
}

bool configLoaded(App* app, CPU& cpu)
{
    reg32 eax = 0;
    return loadSettings(app, cpu, eax);
}

std::optional<reg32> replaySavedWith(App* app, CPU& cpu, reg32 eax)
{
    if (!saveReplayFile(app, cpu, eax))
        return std::nullopt;
    return eax;  // what its last call leaves, as the code's result
}

std::optional<reg32> replaySaved(App* app, CPU& cpu)
{
    return replaySavedWith(app, cpu, 0);
}

std::optional<reg32> replayRead(App* app, CPU& cpu)
{
    return readReplayFile(app, cpu);
}

reg32 cameraViewsSet(App* app)
{
    return cameraViewsFromChosen(app);
}

std::optional<reg32> bigFileKindOf(App* app, CPU& cpu, reg32 archive)
{
    return bigFileKind(app, cpu, archive);
}

std::optional<reg32> blockSaved(App* app, CPU& cpu, reg32 name, reg32 bytes, reg32 count)
{
    const std::optional<Saved> saved = saveBlock(app, cpu, name, bytes, count);
    if (!saved)
        return std::nullopt;
    return saved->written;
}

}  // namespace load
}  // namespace nfs3hp
