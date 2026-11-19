#include "game_settings.h"
#include "native_ea_memory.h"
#include "native_ea_support.h"
#include "native_frontend_loada.h"
#include "native_game.h"
#include "native_guest.h"
#include "native_load_cardb.h"
#include "native_load_carmodels.h"
#include "native_load_saves.h"
#include "native_load_track.h"
#include "native_watcom.h"
#include "port_menus.h"
#include "os_files.h"
#include "os_system.h"

#include <cstdint>
#include <initializer_list>
#include <optional>
#include <type_traits>

/* The game's data: the cars' database, read from the carmodel folder (its
 * path at 0x7a28b4) a car's folder at a time.  The game's code calls each by the name it has here.
 *
 * The cars are a list (its head at 0x554f70) of records of 0x3a4 bytes: +0
 * the next, +4 the .FEC's texts (their count, a word; +8 the fedata read in,
 * +0xc its offsets at +0x2f), +0x10 the car's name, +0x110 and +0x210 its
 * folder's, +0x310 its files' prefix ("car"), +0x315 the fedata's first four
 * bytes, +0x31c its id (16 bytes: for the game's own cars a number below 50
 * and zeros), +0x32c and +0x378 the .FEC's two quads, +0x334 and +0x338 its
 * specs (car.viv's), +0x380 its flags (1 a bonus car, 2, 4, 8, 0x10 a police
 * car, 0x20 a Ferrari or a Mercedes, 0x40), +0x384 a nibble of the fedata's,
 * +0x388 its class (0xff none), +0x38c to +0x3a0 more of the fedata's.  A
 * record by its place in the list goes through a cache of 50 (0x5f3c00); the
 * count is kept at 0x5f3cc8, the last car loaded at 0x554f74.  A car's
 * fedata is its folder's fedata.<language> or the entry of that name in its
 * car.viv; it must be of version 9.  loadCarList reads every folder in, sorts
 * the list (ordinary cars by name, then the police's, then those of no
 * class, then the rest), makes the car menus' items and colours
 * (0x554fa8.., 0x555138..) and the classes' lists (0x5552d8..).  The folders
 * and files are walked with FindFirstFileA's handles at 0x554f78 (folders,
 * their data at 0x5f3ac0) and 0x554f7c (files, at 0x5f3980).
 *
 * Each is a typed function of its own, with a thunk under the game's name
 * for the callers still on registers and the dispatch: it leaves eax, and the
 * registers the code does not keep, as the code does.  The code's frame stays
 * on the guest's stack, at its depth, where a callee is handed an address in
 * it (Frame); the rest of it is C++ locals.  The functions of this file that
 * walk the folders and files are
 * called by address, as is every other function of the game's without a
 * typed core and Windows' through its imports, their arguments in registers
 * as the code hands them -- but the typed ones the headers declare (this
 * file's carRecord, carFolder, carIsPolice, loadCarList too), called as C++
 * (guest::direct, feCore), their arguments still loaded into the registers
 * and their answer left in eax as the call by address leaves them. */
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
using nfs::u16;
using nfs::u32;
using nfs::u8;
using nfs::s32;
using App = nfs::App;
using ThreadState = nfs::ThreadState;

// The list and its records.
constexpr u32 kCarList = 0x554f70, kLastCar = 0x554f74, kCache = 0x5f3c00, kCount = 0x5f3cc8;
constexpr u32 kCursor = 0x5f3ccc, kRecordBytes = 0x3a4;
// The records' cache: 50 places, and the places' last.
constexpr u32 kCacheBytes = 0xc8, kLastCached = 0x31;
// A record's: its next, its texts (the count, the fedata, the offsets), its name, its folder (twice), its
// files' prefix, the fedata's first bytes, its id, the .FEC's quads, its specs, flags, nibble, class.
constexpr u32 kRecNext = 0, kRecTexts = 4, kRecFedata = 8, kRecOffsets = 0xc, kRecName = 0x10;
constexpr u32 kRecFolder = 0x110, kRecFolderToo = 0x210, kRecPrefix = 0x310, kRecHead = 0x315, kRecId = 0x31c;
constexpr u32 kRecFecFirst = 0x32c, kRecFecSecond = 0x378, kRecSpecsWord = 0x334, kRecSpecs = 0x338;
constexpr u32 kRecSpecsToo = 0x394, kRecFlags = 0x380, kRecNibble = 0x384, kRecClass = 0x388;
constexpr u32 kRecFedWord = 0x38c, kRecVersion = 0x38e, kRecFedBytes = 0x38f, kRecFedByte = 0x3a0;
// A record's flags, and the class of none.
constexpr u8 kBonus = 1, kFlag4 = 4, kAfterOthers = 8, kPolice = 0x10, kMaker = 0x20, kFlag40 = 0x40;
constexpr u32 kNoClass = 0xff;
// An id's bytes, and the numbers below which an id is one of the game's own cars.
constexpr u32 kIdBytes = 0x10, kGameCars = 0x32;
// The fedata's: its version (9), its class, its nibble's word, its id, the texts' count and offsets.
constexpr u32 kFedVersion = 4, kFedClass = 0xa, kFedNibble = 0xc, kFedId = 0x18, kFedTextCount = 0x2d;
constexpr u32 kFedTextOffsets = 0x2f, kFedByte = 0x12, kFedWord = 0x16, kFedBytes = 0x28, kFedLastByte = 0x2c;
constexpr u32 kVersion = 9;
// The texts: the maker's first, the car's name third.
constexpr u32 kMakerText = 0, kNameText = 2;
// The folders' and files' walks: the handles, the find data, the names in it.
constexpr u32 kFolderFind = 0x554f78, kFileFind = 0x554f7c;
constexpr u32 kFolderData = 0x5f3ac0, kFolderName = 0x5f3aec, kFileData = 0x5f3980, kFileName = 0x5f39ac;
// What a car's name is where there is no record of it (0x32 to 0x1f3).
constexpr u32 kNoRecordName = 0x554f80;
// The other cars' names, folders and prefixes, by their numbers.
constexpr u32 kNamesFrom500 = 0x55c944, kFoldersTo499 = 0x55cfb0, kFoldersFrom500 = 0x55c92c;
constexpr u32 kPrefixesFrom600 = 0x55c75c;
// The carmodel folder, the language, the bonus cars unlocked, the options.
constexpr u32 kCarmodelPath = 0x7a28b4, kLanguage = 0x7a3a50, kOptions = 0x55eb2e;
// The menus': the cars' items, flags and colours; the car of id 0x21.
constexpr u32 kMenuHeads = 0x555130, kMenuColoursHead = 0x555204;
constexpr u32 kCarItems = 0x555138, kCarItemOn = 0x5551a2, kCarColours = 0x555214;
constexpr u32 kAllItems = 0x554fa8, kAllItemOn = 0x55500a, kAllColours = 0x55506c, kCar21 = 0x6fdc30;
constexpr u32 kWhite = 0xff649dfd, kGrey = 0x908080, kFirstItem = 0x7bd;
// The classes' lists: 0xc8 bytes each, their counts, and how many cars are not in them.
constexpr u32 kClassLists = 0x5552d8, kClassCounts = 0x5555f8, kNotListed = 0x554f50;
constexpr u32 kClassListBytes = 0xc8, kClassCompare = 0x43a4d0;
// The game's strings.
constexpr u32 kExistNam = 0x537658, kWriteBinary = 0x537664, kHexFolder = 0x537668, kPlayerFolder = 0x537670;
constexpr u32 kAllFiles = 0x537684, kDot = 0x537688, kDotDot = 0x53768c, kEmpty = 0x537690, kCar = 0x537694;
constexpr u32 kDuplicates = 0x5376a4, kTraffic = 0x5376d0, kFedataPath = 0x5376d8, kReadBinary = 0x5376e4;
constexpr u32 kCarVivUpper = 0x5376e8, kFerrari = 0x5376f0, kMercedes = 0x5376f8, kCarViv = 0x537704;
constexpr u32 kFedataName = 0x53770c, kFec = 0x537718, kPlaceViv = 0x537720, kCarpTxt = 0x537738;
constexpr u32 kReadUpdate = 0x537744;
constexpr u32 kBackslash = 0x5c, kWritableMode = 0x180, kSeekStart = 0, kSeekEnd = 2;
// Windows' functions, by their imports' slots.
// The game's functions, by address: a path ended with '\'; a file written
// whole (its edx is the code's); the runtime's ftell, fwrite, remove, chmod and
// rmdir (no typed cores).  The car's specs loaded, a player's settings checked,
// the .viv's (track's) lookup, its write and its parse, a file loaded, the
// game's memory, a .viv's opening, look-up and closing, the runtime's others,
// EA's fill and Error, the menus' file copied and language's name are the
// other files' (native_load_carmodels.h, native_load_track.h,
// native_ea_support.h, native_ea_memory.h, native_watcom.h, native_game.h,
// native_frontend_loada.h), called as C++ (guest::direct, feCore).
constexpr u32 kWriteWhole = 0x4eebb0;
constexpr u32 kFwrite = 0x4eebd0;
// This file's functions that walk the folders and files are called
// by address; carFolder, carIsPolice and the carmodels' natives'
// (carRecord, loadCarList) as C++ (typed).

s32 s(u32 value)
{
    return s32(value);
}

u16 word16(App* app, u32 address)
{
    return app->getMemory<u16>(address);
}

void putWord16(App* app, u32 address, u16 value)
{
    app->getMemory<u16>(address) = value;
}

/* A word's sign carried into a dword (movsx; or a dword's top half by sar 16). */
u32 widen(u16 value)
{
    return u32(s32(std::int16_t(value)));
}

/* The code's frame on the guest's stack, where its callees see it: `pushed`
 * the bytes between the caller's esp and the code's ebp (the words pushed for
 * the call, the return address, the registers saved), `locals` those under
 * ebp; esp under them while the function runs, and as it was after. */
class Frame
{
public:
    Frame(ThreadState& ts, u32 pushed, u32 locals) : ts_(ts), esp_(ts.sp), base_(ts.sp - pushed)
    {
        ts.sp = base_ - locals;
    }
    ~Frame() { ts_.sp = esp_; }
    u32 operator-(u32 offset) const { return base_ - offset; }
    u32 base() const { return base_; }

private:
    ThreadState& ts_;
    const u32 esp_;
    const u32 base_;
};


/* `Core`, the game's `Address`, a typed function another file's header (or this one's) declares,
 * called as C++ (guest::direct).  Nullopt when the game stopped in it. */
template <u32 Address, auto Core, typename... Args>
std::optional<u32> typed(App* app, ThreadState& ts, Args... args)
{
    const auto answer = guest::direct<Address, Core>(app, ts, args...);
    if (ts.terminate)
        return std::nullopt;
    if constexpr (std::is_same_v<std::decay_t<decltype(answer)>, std::optional<u32>>)
    {
        if (!answer)
            return std::nullopt;
        return *answer;
    }
    else
        return answer;
}

/* A native_frontend_* file's typed core called as C++ (guest::direct): nothing (false) when the
 * game stopped in it (its calls into the game throw fe::Terminated). */
template <u32 Address, auto Core, typename... Args>
auto feCore(App* app, ThreadState& ts, Args... args)
{
    using Result = decltype(guest::direct<Address, Core>(app, ts, args...));
    if constexpr (std::is_void_v<Result>)
    {
        try
        {
            guest::direct<Address, Core>(app, ts, args...);
        }
        catch (const fe::Terminated&)
        {
            return false;
        }
        return !ts.terminate;
    }
    else
    {
        try
        {
            const Result result = guest::direct<Address, Core>(app, ts, args...);
            return ts.terminate ? std::optional<Result>() : std::optional<Result>(result);
        }
        catch (const fe::Terminated&)
        {
            return std::optional<Result>();
        }
    }
}

/* feCore, its answer in the code's eax (a u32): nothing when the game stopped. */
template <u32 Address, auto Core, typename... Args>
std::optional<u32> menuCall(App* app, ThreadState& ts, Args... args)
{
    return feCore<Address, Core>(app, ts, args...);
}

/* The runtime's sprintf (0x4df690): the buffer, the format, its arguments (cdecl). */
std::optional<u32> printTo(App* app, ThreadState& ts, std::initializer_list<u32> words)
{
    const u32* const word = words.begin();
    guest::Words rest({});
    for (std::size_t i = 2; i < words.size(); ++i)
        rest.word[rest.count++] = word[i];
    return typed<0x4df690, watcom::clib::sprintf>(app, ts, word[0], word[1], rest);
}

// The runtime's functions the code calls, Watcom's: their answer in eax.
std::optional<u32> compareStrings(App* app, ThreadState& ts, u32 a, u32 b)  // strcmp
{
    return typed<0x4ee310, watcom::clib::strcmp>(app, ts, a, b);
}

std::optional<u32> compareNoCase(App* app, ThreadState& ts, u32 a, u32 b)  // stricmp
{
    return typed<0x4e9ec0, watcom::clib::stricmp>(app, ts, a, b);
}

std::optional<u32> lastOf(App* app, ThreadState& ts, u32 string, u32 c)  // strrchr
{
    return typed<0x4ee2f0, watcom::clib::strrchr>(app, ts, string, c);
}

std::optional<u32> fillBytes(App* app, ThreadState& ts, u32 to, u32 value, u32 bytes)  // memset
{
    return typed<0x4e0640, watcom::clib::memset>(app, ts, to, value, bytes);
}

std::optional<u32> openFile(App* app, ThreadState& ts, u32 path, u32 mode)
{
    return typed<0x4edfe8, watcom::clib::fopen>(app, ts, path, mode);
}

std::optional<u32> closeFile(App* app, ThreadState& ts, u32 file)
{
    return typed<0x4ee100, watcom::clib::fclose>(app, ts, file);
}

std::optional<u32> seekFile(App* app, ThreadState& ts, u32 file, u32 offset, u32 whence)
{
    return typed<0x4e9088, watcom::clib::fseek>(app, ts, file, offset, whence);
}

std::optional<u32> tellFile(App* app, ThreadState& ts, u32 file)
{
    return typed<0x4ee420, watcom::clib::ftell>(app, ts, file);
}

std::optional<u32> readFile(App* app, ThreadState& ts, u32 to, u32 size, u32 count, u32 file)
{
    return typed<0x4e9200, watcom::clib::fread>(app, ts, to, size, count, file);
}

std::optional<u32> writeFile(App* app, ThreadState& ts, u32 from, u32 size, u32 count, u32 file)
{
    return typed<0x4eebd0, watcom::clib::fwrite>(app, ts, from, size, count, file);
}

std::optional<u32> removeFile(App* app, ThreadState& ts, u32 path)
{
    return typed<0x4ee2e0, watcom::clib::remove>(app, ts, path);
}

/* The game's memory handed out (0x4e1620, named ""), and given back. */
std::optional<u32> allocate(App* app, ThreadState& ts, u32 bytes)
{
    return guest::direct<0x4e1620, ea::reserved>(app, ts, kEmpty, bytes, u32(0));
}

std::optional<u32> release(App* app, ThreadState& ts, u32 block)
{
    return guest::direct<0x4e1890, ea::purgeBlock>(app, ts, block);
}

// The carmodels' carRecord and this file's walkers, by their place in the list.
std::optional<u32> carRecordAt(App* app, ThreadState& ts, u32 place)
{
    return typed<0x439a50, recordAt>(app, ts, place);
}

std::optional<u32> carFolderAt(App* app, ThreadState& ts, u32 place)
{
    return typed<0x4392b0, folderOf>(app, ts, place);
}

std::optional<u32> policeAt(App* app, ThreadState& ts, u32 place)
{
    return typed<0x4396b0, policeFlag>(app, ts, place);
}

/* The string at `from`, its 0 too, to `to`, a byte at a time from the first,
 * as the runtime's inlined strcpy copies. */
void copyString(App* app, u32 to, u32 from)
{
    for (;; ++to, ++from)
    {
        const u8 c = byte(app, from);
        putByte(app, to, c);
        if (c == 0)
            break;
    }
}

u32 stringLength(App* app, u32 string)
{
    u32 length = 0;
    while (byte(app, string + length) != 0)
        ++length;
    return length;
}

/* strcat, inlined: the string at `from` copied to the end of the one at `to`. */
void appendString(App* app, u32 to, u32 from)
{
    copyString(app, to + stringLength(app, to), from);
}

/* `bytes` from `from` to `to`, the dwords first, then the bytes left, as rep
 * movsd and rep movsb copy. */
void moveBytes(App* app, u32 to, u32 from, u32 bytes)
{
    for (u32 i = 0; i < bytes / 4; ++i, to += 4, from += 4)
        put(app, to, at(app, from));
    for (u32 i = 0; i < bytes % 4; ++i, ++to, ++from)
        putByte(app, to, byte(app, from));
}

/* Whether the `bytes` at a and b are alike (repe cmpsb). */
bool alike(App* app, u32 a, u32 b, u32 bytes)
{
    for (u32 i = 0; i < bytes; ++i)
        if (byte(app, a + i) != byte(app, b + i))
            return false;
    return true;
}

/* A list of cars as the code passes one: its head and its tail. */
struct CarList
{
    u32 head = 0, tail = 0;
};

CarList listAt(App* app, u32 pair)
{
    return {at(app, pair), at(app, pair + 4)};
}

void putList(App* app, u32 pair, const CarList& list)
{
    put(app, pair, list.head);
    put(app, pair + 4, list.tail);
}

u32 nextOf(App* app, u32 car)
{
    return at(app, car + kRecNext);
}

}  // namespace

namespace
{

/* 0x438340's: the path ended with '\' (0x4529a0 adds one where it has none);
 * what that leaves in eax. */
std::optional<u32> endPath(App* app, ThreadState& ts, u32 path)
{
    const Frame frame(ts, 4 + 4, 0);
    return typed<0x4529a0, port::menus::backslashEnded>(app, ts, path);
}

/* 0x438350's: whether the folder can be written to -- its exist.nam made
 * (fopen "wb"), closed and removed: 1, else fopen's 0. */
std::optional<u32> writable(App* app, ThreadState& ts, u32 folder)
{
    const Frame frame(ts, 4 + 0x14, 0x12c);
    const u32 path = frame - 0x12c;
    copyString(app, path, folder);
    if (!endPath(app, ts, path))
        return std::nullopt;
    appendString(app, path, kExistNam);
    const auto file = openFile(app, ts, path, kWriteBinary);
    if (!file || *file == 0)
        return file;
    if (!closeFile(app, ts, *file) || !removeFile(app, ts, path))
        return std::nullopt;
    return 1;
}

/* 0x4383f0's and 0x4384f0's: the first name made of a count from 0 by
 * `format` into the buffer `name` whose folder under the path `under` cannot
 * be written to (is not there).  False when the game stopped. */
bool freeFolderName(App* app, ThreadState& ts, u32 under, u32 name, u32 format)
{
    const Frame frame(ts, 4 + 0x14, 0x134);
    const u32 path = frame - 0x134;
    for (u32 count = 0;; ++count)
    {
        if (!printTo(app, ts, {name, format, count}))
            return false;
        copyString(app, path, under);
        if (!endPath(app, ts, path))
            return false;
        appendString(app, path, name);
        const auto free = writable(app, ts, path);
        if (!free)
            return false;
        if (*free == 0)
            return true;
    }
}

}  // namespace

/* 0x4385a0's: the first "P%03d" folder not there in the carmodel folder, its
 * name into `name`. */
bool freePlayerFolder(App* app, ThreadState& ts, u32 name)
{
    const Frame frame(ts, 4 + 0x10, 0x12c);
    const u32 path = frame - 0x12c;
    copyString(app, path, kCarmodelPath);
    return endPath(app, ts, path) && freeFolderName(app, ts, path, name, kPlayerFolder);
}

namespace
{

/* The walks' search path, into `path`: the folder copied, strrchr'd for '\'
 * (its answer unused), ended with '\' and "*.*". */
bool searchPath(App* app, ThreadState& ts, u32 path, u32 folder)
{
    copyString(app, path, folder);
    if (!lastOf(app, ts, path, kBackslash) || !endPath(app, ts, path))
        return false;
    appendString(app, path, kAllFiles);
    return true;
}

/* A walk: of the folders in a folder ("." and ".." passed over), or of its
 * files; the search's word, the find data, the name in it -- where the
 * callers take the name from. */
struct Walk
{
    u32 handle, data, name;
    bool folders;
};

constexpr Walk kFolders = {kFolderFind, kFolderData, kFolderName, true};
constexpr Walk kFiles = {kFileFind, kFileData, kFileName, false};

/* Whether what the walk found, `found` -- its name put where the walk keeps
 * it -- is what it looks for: a folder but "." and ".." (strcmp with each),
 * or a file.  Nullopt when the game stopped. */
std::optional<bool> wanted(App* app, ThreadState& ts, const Walk& walk, const os::files::Found& found)
{
    guest::putText(app, walk.name, found.name.c_str());
    const bool folder = found.directory;
    if (!walk.folders || !folder)
        return walk.folders == folder;
    for (const u32 dots : {kDot, kDotDot})
    {
        const auto order = compareStrings(app, ts, dots, walk.name);
        if (!order)
            return std::nullopt;
        if (*order == 0)
            return false;
    }
    return true;
}

/* The walk's search closed, 0 in its word. */
bool closeSearch(App* app, ThreadState& ts, const Walk& walk)
{
    NFS2_USE(ts);
    os::release(app, at(app, walk.handle));
    put(app, walk.handle, 0);
    return true;
}

/* 0x4386b0's and 0x438870's: the walk's first in `folder` (a search of its
 * "*.*", kept in the walk's word): its name, or 0 -- the search closed --
 * where there is none. */
std::optional<u32> firstFound(App* app, ThreadState& ts, const Walk& walk, u32 folder)
{
    const Frame frame(ts, 4 + 0x18, 0x12c);
    const u32 path = frame - 0x12c;
    if (!searchPath(app, ts, path, folder))
        return std::nullopt;
    os::files::Found found;
    const auto search = os::files::findFirst(app, guest::text(app, path), found);
    put(app, walk.handle, search.value_or(0));
    if (!search)
        return 0;
    for (;;)
    {
        const auto isWanted = wanted(app, ts, walk, found);
        if (!isWanted)
            return std::nullopt;
        if (*isWanted)
            return walk.name;
        if (!os::files::findNext(app, at(app, walk.handle), found))
            break;
    }
    if (!closeSearch(app, ts, walk))
        return std::nullopt;
    return 0;
}

/* 0x4387e0's and 0x438970's: the walk's next: its name, or 0 -- the search
 * closed -- at its end (0 too where there is no search). */
std::optional<u32> nextFound(App* app, ThreadState& ts, const Walk& walk)
{
    const Frame frame(ts, 4 + 0x14, 0);
    if (at(app, walk.handle) == 0)
        return 0;
    os::files::Found found;
    for (;;)
    {
        if (!os::files::findNext(app, at(app, walk.handle), found))
            break;
        const auto isWanted = wanted(app, ts, walk, found);
        if (!isWanted)
            return std::nullopt;
        if (*isWanted)
            return walk.name;
    }
    if (!closeSearch(app, ts, walk))
        return std::nullopt;
    return 0;
}

/* 0x4389d0's: the folder removed: each file in it made writable (chmod
 * 0x180) and removed, then the folder itself (rmdir; not its folders):
 * rmdir's answer. */
std::optional<u32> removeAll(App* app, ThreadState& ts, u32 folder)
{
    const Frame frame(ts, 4 + 0x18, 0x12c);
    const u32 path = frame - 0x12c;
    for (auto name = guest::direct<0x438870, firstFileIn>(app, ts, folder);;
         name = guest::direct<0x438970, nextFileFound>(app, ts))
    {
        if (!name)
            return std::nullopt;
        if (*name == 0)
            break;
        copyString(app, path, folder);
        if (!endPath(app, ts, path))
            return std::nullopt;
        appendString(app, path, *name);
        if (!typed<0x4ee3c0, watcom::clib::chmod>(app, ts, path, kWritableMode) || !removeFile(app, ts, path))
            return std::nullopt;
    }
    if (!typed<0x4ee3c0, watcom::clib::chmod>(app, ts, folder, kWritableMode))
        return std::nullopt;
    return typed<0x4ee400, watcom::clib::rmdir>(app, ts, folder);
}

}  // namespace

/* 0x438340: endPath for the path eax. */
nfs::u32 endWithSlash(App* app, ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    endPath(app, ts, args.eax);
    return args.eax;
}

/* 0x438350: writable for the folder eax. */
nfs::u32 folderWritable(App* app, ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    if (const auto answer = writable(app, ts, args.eax))
        args.eax = *answer;
    return args.eax;
}

/* 0x4383f0: the first "%04X" name, into edx, of a folder not there under the
 * path eax (a new car's folder, copyCarAsNew's); eax 0. */
nfs::u32 firstFreeHexFolder(App* app, ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    if (freeFolderName(app, ts, args.eax, args.edx, kHexFolder))
        args.eax = 0;
    return args.eax;
}

/* 0x4384f0: the first "P%03d" name, into edx, of a folder not there under
 * the path eax; eax 0. */
nfs::u32 firstFreePlayerFolder(App* app, ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    if (freeFolderName(app, ts, args.eax, args.edx, kPlayerFolder))
        args.eax = 0;
    return args.eax;
}

/* 0x4385a0: freePlayerFolder into eax; eax 0.  No caller in the game. */
nfs::u32 freePlayerCarFolder(App* app, ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    if (freePlayerFolder(app, ts, args.eax))
        args.eax = 0;
    return args.eax;
}

/* 0x4386b0: the first folder in the folder eax (the handle kept at
 * 0x554f78): its name (0x5f3aec), or 0. */
nfs::u32 firstSubfolder(App* app, ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    if (const auto name = firstFound(app, ts, kFolders, args.eax))
        args.eax = *name;
    return args.eax;
}

/* 0x4387e0: the next folder of firstSubfolder's search, or 0. */
nfs::u32 nextSubfolder(App* app, ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    if (const auto name = nextFound(app, ts, kFolders))
        args.eax = *name;
    return args.eax;
}

/* 0x438870: the first file (not a folder) in the folder eax (the handle kept
 * at 0x554f7c): its name (0x5f39ac), or 0. */
nfs::u32 firstFile(App* app, ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    if (const auto name = firstFound(app, ts, kFiles, args.eax))
        args.eax = *name;
    return args.eax;
}

/* 0x438970: the next file of firstFile's search, or 0. */
nfs::u32 nextFile(App* app, ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    if (const auto name = nextFound(app, ts, kFiles))
        args.eax = *name;
    return args.eax;
}

/* 0x4389d0: removeAll for the folder eax. */
nfs::u32 removeFolder(App* app, ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    if (const auto answer = removeAll(app, ts, args.eax))
        args.eax = *answer;
    return args.eax;
}

namespace
{

/* 0x438a80's: the list's first car, the cursor at `cursor` set to it. */
u32 firstCarAt(App* app, u32 cursor)
{
    const u32 car = at(app, kCarList);
    put(app, cursor, car);
    return car;
}

/* 0x438aa0's: the car after the cursor's, the cursor moved on to it; 0 where
 * the cursor is at none (it left so). */
u32 nextCarAt(App* app, u32 cursor)
{
    const u32 car = at(app, cursor);
    if (car == 0)
        return 0;
    const u32 next = nextOf(app, car);
    put(app, cursor, next);
    return next;
}

}  // namespace

/* 0x438ad0's: the id of the number `number` into the 16 bytes at `id`: zeros
 * (memset, whose eax and edx are left), the number first. */
bool makeId(App* app, ThreadState& ts, u32 id, u32 number)
{
    const Frame frame(ts, 4 + 0x10, 0);
    if (!fillBytes(app, ts, id, 0, kIdBytes))
        return false;
    put(app, id, number);
    return true;
}

namespace
{

/* 0x438af0's and 0x438b10's: the list walked with its own cursor (0x5f3ccc):
 * the first car, the next, the cursor set to each. */
u32 firstListed(App* app)
{
    const u32 car = at(app, kCarList);
    put(app, kCursor, car);
    return car;
}

u32 nextListed(App* app)
{
    u32 car = at(app, kCursor);
    if (car != 0)
        car = nextOf(app, car);
    put(app, kCursor, car);
    return car;
}

/* 0x438b90's: the record put at the list's end (its next as it was), the
 * list walked with its own cursor. */
void appendRecord(App* app, u32 record)
{
    u32 car = firstListed(app);
    if (car == 0)
    {
        put(app, kCarList, record);
        return;
    }
    u32 last = car;
    while (car != 0)
    {
        const u32 next = nextListed(app);
        last = car;
        car = next;
    }
    put(app, last + kRecNext, record);
    put(app, record + kRecNext, car);
}

/* 0x438be0's: the cars in the list counted, the count kept (0x5f3cc8). */
u32 countList(App* app)
{
    u32 count = 0;
    for (u32 car = at(app, kCarList); car != 0; car = nextOf(app, car))
        ++count;
    put(app, kCount, count);
    return count;
}

/* 0x438c10's: the record taken out of the list, where it is in it: the car
 * before it, the record's next where it was the head, else 0. */
u32 unlink(App* app, u32 record)
{
    u32 car = at(app, kCarList);
    if (car == record)
    {
        const u32 next = nextOf(app, record);
        put(app, kCarList, next);
        return next;
    }
    for (; car != 0; car = nextOf(app, car))
        if (nextOf(app, car) == record)
        {
            put(app, car + kRecNext, nextOf(app, record));
            return car;
        }
    return 0;
}

/* 0x438c50's: the record (its next 0 first) put into the list before the
 * first car whose name its own is below (strcmp), else at its end. */
std::optional<CarList> insertByName(App* app, ThreadState& ts, CarList list, u32 record)
{
    const Frame frame(ts, 0xc + 4 + 0x14, 8);
    put(app, record + kRecNext, 0);
    if (list.head == 0)
    {
        list.head = list.tail = record;
        return list;
    }
    // The car whose next is looked at, 0 for the list's head.
    u32 link = 0;
    for (;;)
    {
        const u32 car = link == 0 ? list.head : nextOf(app, link);
        if (car == 0)
            break;
        const auto order = compareStrings(app, ts, record + kRecName, car + kRecName);
        if (!order)
            return std::nullopt;
        if (s(*order) < 0)
        {
            put(app, record + kRecNext, car);
            if (link == 0)
                list.head = record;
            else
                put(app, link + kRecNext, record);
            break;
        }
        link = car;
        if (nextOf(app, link) == 0)
        {
            put(app, record + kRecNext, 0);
            put(app, list.tail + kRecNext, record);
            list.tail = record;
            break;
        }
    }
    return list;
}

/* 0x438cc0's: two lists joined, the second after the first. */
CarList joinLists(App* app, const CarList& first, const CarList& second)
{
    if (first.head == 0)
        return second;
    if (second.head == 0)
        return first;
    put(app, first.tail + kRecNext, second.head);
    return {first.head, second.tail};
}

/* 0x438d20's: the chain from `car` as a list, its last car the tail; `tail`
 * where the chain is empty. */
CarList chainOf(App* app, u32 car, u32 tail)
{
    const u32 head = car;
    for (; car != 0; car = nextOf(app, car))
        if (nextOf(app, car) == 0)
        {
            tail = car;
            break;
        }
    return {head, tail};
}

/* The cars of the list that `taken` says taken out of it into a list of their
 * own, by name (insertByName): 0x438d60's (the police cars), 0x438dc0's (of
 * no class) and 0x438e30's (the ordinary).  `saved` the bytes of
 * registers the code saves. */
template <class Taken>
std::optional<CarList> takeCars(App* app, ThreadState& ts, u32 saved, Taken taken)
{
    const Frame frame(ts, 4 + saved, 8);
    CarList list;
    u32 car = at(app, kCarList);
    while (car != 0)
    {
        if (!taken(car))
        {
            car = nextOf(app, car);
            continue;
        }
        unlink(app, car);
        const auto inserted = insertByName(app, ts, list, car);
        if (!inserted)
            return std::nullopt;
        list = *inserted;
        car = at(app, kCarList);
    }
    return list;
}

std::optional<CarList> takePolice(App* app, ThreadState& ts)
{
    return takeCars(app, ts, 0x14,
                    [&](u32 car) { return (byte(app, car + kRecFlags) & kPolice) != 0; });
}

std::optional<CarList> takeClassless(App* app, ThreadState& ts)
{
    return takeCars(app, ts, 0x10,
                    [&](u32 car) { return at(app, car + kRecClass) == kNoClass; });
}

std::optional<CarList> takeOrdinary(App* app, ThreadState& ts)
{
    return takeCars(app, ts, 0x14, [&](u32 car) {
        return at(app, car + kRecClass) != kNoClass && (byte(app, car + kRecFlags) & (kPolice | kAfterOthers)) == 0;
    });
}

/* 0x4399f0's: the records' cache (50 at 0x5f3c00) cleared: memset's eax. */
std::optional<u32> clearCache(App* app, ThreadState& ts)
{
    const Frame frame(ts, 4 + 0xc, 0);
    return fillBytes(app, ts, kCache, 0, kCacheBytes);
}

/* 0x438eb0's: the list sorted: the ordinary cars by name, then the police's
 * by name, then those of no class by name, then the rest as they were; the
 * records' cache cleared.  The list's head, or clearCache's eax where it is
 * empty. */
std::optional<u32> sortList(App* app, ThreadState& ts)
{
    const Frame frame(ts, 4 + 0x18, 0x20);
    const auto cleared = clearCache(app, ts);
    if (!cleared || at(app, kCarList) == 0)
        return cleared;
    const auto ordinary = takeOrdinary(app, ts);
    if (!ordinary)
        return std::nullopt;
    const auto police = takePolice(app, ts);
    if (!police)
        return std::nullopt;
    CarList sorted = joinLists(app, *ordinary, *police);
    const auto classless = takeClassless(app, ts);
    if (!classless)
        return std::nullopt;
    sorted = joinLists(app, sorted, *classless);
    // An empty rest's tail is the frame's word, never set: the join never uses it, it keeps the first.
    sorted = joinLists(app, sorted, chainOf(app, at(app, kCarList), 0));
    put(app, kCarList, sorted.head);
    return sorted.head;
}

}  // namespace

/* 0x439250's: the name of the car `car`: the table's for 500 to 599,
 * 0x554f80 for 50 to 499, else its record's +0x10 (0x10 where it has none). */
std::optional<u32> nameOf(App* app, ThreadState& ts, u32 car)
{
    if (s(car) >= 500 && s(car) <= 599)
        return at(app, car * 4 + kNamesFrom500);
    if (s(car) >= 50 && s(car) <= 499)
        return kNoRecordName;
    const Frame frame(ts, 4 + 4, 0);
    const auto record = carRecordAt(app, ts, car);
    if (!record)
        return std::nullopt;
    return *record + kRecName;
}

/* 0x4392b0's: the folder's name of the car: its record's +0x110, else the
 * tables' for up to 499 and 500 to 599, else 0. */
std::optional<u32> folderOf(App* app, ThreadState& ts, u32 car)
{
    const Frame frame(ts, 4 + 8, 0);
    const auto record = carRecordAt(app, ts, car);
    if (!record)
        return std::nullopt;
    if (*record != 0)
        return *record + kRecFolder;
    if (s(car) <= 499)
        return at(app, car * 4 + kFoldersTo499);
    if (s(car) <= 599)
        return at(app, car * 4 + kFoldersFrom500);
    return 0;
}

/* 0x439300's: the prefix of the car's files: "car" for a record and for 50
 * to 599, the table's for 600 to 699, else "". */
std::optional<u32> prefixOf(App* app, ThreadState& ts, u32 car)
{
    const Frame frame(ts, 4 + 8, 0);
    const auto record = carRecordAt(app, ts, car);
    if (!record)
        return std::nullopt;
    if (*record != 0 || (s(car) >= 50 && s(car) <= 599))
        return kCar;
    if (s(car) >= 600 && s(car) <= 699)
        return at(app, car * 4 + kPrefixesFrom600);
    return kEmpty;
}

/* 0x439520's: the id of the car (its record's +0x31c). */
std::optional<u32> idOf(App* app, ThreadState& ts, u32 car)
{
    const Frame frame(ts, 4 + 4, 0);
    const auto record = carRecordAt(app, ts, car);
    if (!record)
        return std::nullopt;
    return *record + kRecId;
}

namespace
{

/* The car's flags (its record's dword at +0x380) by `mask`: 0x439570's (0x40),
 * 0x4395c0's (4), 0x439680's (1, a bonus car), 0x4396b0's (0x10, a police
 * car), 0x4396e0's (0x20, a Ferrari or a Mercedes). */
std::optional<u32> flagsOf(App* app, ThreadState& ts, u32 car, u32 mask)
{
    const Frame frame(ts, 4 + 4, 0);
    const auto record = carRecordAt(app, ts, car);
    if (!record)
        return std::nullopt;
    return at(app, *record + kRecFlags) & mask;
}

/* 0x439620's: whether the id is a number: its words at +4 and +6 and its
 * dwords at +8 and +0xc 0, its first dword below 50. */
bool isNumeric(App* app, u32 id)
{
    return word16(app, id + 4) == 0 && word16(app, id + 6) == 0 && at(app, id + 8) == 0 && at(app, id + 0xc) == 0
           && at(app, id) < kGameCars;
}

}  // namespace

/* 0x439660's: the number of the id, -1 if it is not one. */
u32 numberOf(App* app, u32 id)
{
    return isNumeric(app, id) ? at(app, id) : 0xffffffff;
}

namespace
{

/* 0x439610's: whether the car's id (carId's) is one of the game's own: 1 or 0. */
std::optional<u32> hasNumericId(App* app, ThreadState& ts, u32 car)
{
    const Frame frame(ts, 4 + 4, 0);
    const auto id = idOf(app, ts, car);
    if (!id)
        return std::nullopt;
    return isNumeric(app, *id) ? 1 : 0;
}

/* 0x439730's: the class of the car (its record's +0x388). */
std::optional<u32> classOf(App* app, ThreadState& ts, u32 car)
{
    const Frame frame(ts, 4 + 4, 0);
    const auto record = carRecordAt(app, ts, car);
    if (!record)
        return std::nullopt;
    return at(app, *record + kRecClass);
}

}  // namespace

/* 0x439990's: the place in the list of the car whose id is the 16 bytes at
 * `id`, -1 if none. */
u32 placeById(App* app, u32 id)
{
    u32 place = 0;
    for (u32 car = at(app, kCarList); car != 0; car = nextOf(app, car), ++place)
        if (alike(app, id, car + kRecId, kIdBytes))
            return place;
    return 0xffffffff;
}

/* 0x439a50's: the record of the car at `place` in the list: the cache's,
 * else the list walked and the record (or 0) cached; 0 above 49.  A place
 * below 0 walks the whole list and writes its 0 below the cache (the game's
 * bug). */
u32 recordAt(App* app, u32 place)
{
    if (s(place) > s(kLastCached))
        return 0;
    const u32 cached = at(app, place * 4 + kCache);
    if (cached != 0)
        return cached;
    u32 car = at(app, kCarList);
    for (u32 i = 0; car != 0 && i != place; ++i)
        car = nextOf(app, car);
    put(app, place * 4 + kCache, car);
    return car;
}

namespace
{

/* Whether the texts at `texts` (a record's +4: their count, a word; the
 * fedata at +4, the offsets at +8) have the text `index`. */
bool hasText(App* app, u32 texts, u32 index)
{
    return s(index) <= s(widen(word16(app, texts)) - 1);
}

/* 0x439af0's: the .FEC's text `index` of the texts: the fedata plus its
 * offset, 0 past the count. */
u32 textAt(App* app, u32 texts, u32 index)
{
    if (!hasText(app, texts, index))
        return 0;
    const u32 offsets = at(app, texts + 8);
    return at(app, texts + 4) + at(app, index * 4 + offsets);
}

/* 0x439b20's: the place in the list of the car named `name` (strcmp), -1 if
 * none. */
std::optional<u32> placeByName(App* app, ThreadState& ts, u32 name)
{
    const Frame frame(ts, 4 + 0x14, 4);
    u32 place = 0;
    for (u32 car = at(app, kCarList); car != 0; car = nextOf(app, car), ++place)
    {
        const auto order = compareStrings(app, ts, name, car + kRecName);
        if (!order)
            return std::nullopt;
        if (*order == 0)
            return place;
    }
    return 0xffffffff;
}

/* 0x439bd0's: the fedata's .FEC texts given to the record: the fedata to its
 * +8, their count (the fedata's word at +0x2d) to +4, their offsets (+0x2f)
 * to +0xc; the offsets' address. */
u32 attachTexts(App* app, u32 record, u32 fedata)
{
    put(app, record + kRecFedata, fedata);
    putWord16(app, record + kRecTexts, word16(app, fedata + kFedTextCount));
    const u32 offsets = at(app, record + kRecFedata) + kFedTextOffsets;
    put(app, record + kRecOffsets, offsets);
    return offsets;
}

/* 0x439c10's: whether the record is a bonus car still locked: a bonus car
 * (flag 1) of the game's own whose bit of those unlocked (0x6fd2a8) -- 0x27
 * 1, 0x12 2, 0x11 4, 0x24 8, 0x31 0x10 -- is not set (all locked while none
 * is; any other number locked).  1 or 0. */
u32 lockedBonus(App* app, u32 record)
{
    if ((byte(app, record + kRecFlags) & kBonus) == 0)
        return 0;
    const u32 id = record + kRecId;
    if (!isNumeric(app, id))
        return 0;
    const u32 number = numberOf(app, id);
    u8 bit = 0;
    if (gameSettings(app).unlockedCars != 0)
        switch (number)
        {
        case 0x11: bit = 4; break;
        case 0x12: bit = 2; break;
        case 0x24: bit = 8; break;
        case 0x27: bit = 1; break;
        case 0x31: bit = 0x10; break;
        default: break;
        }
    return bit != 0 && (std::uint8_t(gameSettings(app).unlockedCars) & bit) != 0 ? 0 : 1;
}

/* 0x439d20's: an Error ("Duplicate cars found in carmodel folder!") for each
 * two cars of the same id (carId's).  What the code leaves in eax: the last
 * car's id -- the last round's search for its duplicates is empty -- or the
 * count where it is 0 or below. */
std::optional<u32> checkDuplicates(App* app, ThreadState& ts)
{
    const Frame frame(ts, 4 + 0x18, 8);
    const u32 count = at(app, kCount);
    u32 left = count;
    for (u32 place = 0; s(place) < s(count); ++place)
    {
        const auto id = idOf(app, ts, place);
        if (!id)
            return std::nullopt;
        left = *id;
        for (u32 other = place + 1; s(other) < s(count); ++other)
        {
            const auto otherId = idOf(app, ts, other);
            if (!otherId)
                return std::nullopt;
            if (alike(app, *id, *otherId, kIdBytes))
            {
                guest::direct<0x401010, ea::error>(app, ts, kDuplicates, guest::Words({}));
                if (ts.terminate)
                    return std::nullopt;
            }
        }
    }
    return left;
}

}  // namespace

/* 0x438a80: firstCarAt for the cursor eax. */
nfs::u32 firstCar(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = firstCarAt(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

/* 0x438aa0: nextCarAt for the cursor eax. */
nfs::u32 nextCar(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = nextCarAt(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

/* 0x438ad0: makeId of the number edx at eax. */
nfs::u32 makeCarId(App* app, ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    makeId(app, ts, args.eax, args.edx);
    return args.eax;
}

/* 0x438af0: firstListed. */
nfs::u32 firstCarGlobal(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = firstListed(app);
    ts.sp += 4;
    return args.eax;
}

/* 0x438b10: nextListed. */
nfs::u32 nextCarGlobal(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = nextListed(app);
    ts.sp += 4;
    return args.eax;
}

/* 0x438b90: appendRecord for the record eax; eax the walk's last 0. */
nfs::u32 appendCar(App* app, ThreadState& ts, nfs::Args& args)
{
    appendRecord(app, args.eax);
    args.eax = 0;
    ts.sp += 4;
    return args.eax;
}

/* 0x438bd0: the count of cars countCars kept. */
nfs::u32 carCount(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = at(app, kCount);
    ts.sp += 4;
    return args.eax;
}

/* 0x438be0: countList. */
nfs::u32 countCars(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = countList(app);
    ts.sp += 4;
    return args.eax;
}

/* 0x438c10: unlink for the record eax. */
nfs::u32 unlinkCar(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = unlink(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

/* 0x438c50: insertByName for the list of the stack's first two words (its
 * head, its tail) and the record of its third, the list to the pair at esi,
 * eax it; esi the code's frame's base, where its last movsd leaves it.
 * ret 0xc. */
nfs::u32 insertCarByName(App* app, ThreadState& ts, nfs::Args& args)
{
    const u32 entry = ts.sp, pair = args.esi;
    const CarList list = listAt(app, entry + 4);
    const u32 record = at(app, entry + 0xc);
    ts.sp += 4 + 0xc;
    const auto inserted = insertByName(app, ts, list, record);
    if (!inserted)
        return args.eax;
    // The code keeps the list in the words it was handed, and leaves it there.
    putList(app, entry + 4, *inserted);
    putList(app, pair, *inserted);
    args.eax = pair;
    args.esi = entry - 0x14;
    return args.eax;
}

/* 0x438cc0: joinLists of the stack's two pairs of head and tail, to the pair
 * at esi, eax it; esi the frame's base.  ret 0x10. */
nfs::u32 joinCarLists(App* app, ThreadState& ts, nfs::Args& args)
{
    const u32 entry = ts.sp, pair = args.esi;
    const CarList first = listAt(app, entry + 4), second = listAt(app, entry + 0xc);
    const CarList joined = joinLists(app, first, second);
    // The code joins them in the words it was handed: the first's tail made the second's.
    if (first.head != 0 && second.head != 0)
        put(app, entry + 8, second.tail);
    putList(app, pair, joined);
    args.eax = pair;
    args.esi = entry - 0x10;
    ts.sp += 4 + 0x10;
    return args.eax;
}

/* 0x438d20: chainOf the car eax to the pair at esi, eax it; esi the frame's
 * base.  An empty chain's tail is the frame's word the code never sets: what
 * the guest's stack holds there. */
nfs::u32 carListOf(App* app, ThreadState& ts, nfs::Args& args)
{
    const u32 entry = ts.sp, pair = args.esi;
    putList(app, pair, chainOf(app, args.eax, at(app, entry - 0x14)));
    args.eax = pair;
    args.esi = entry - 0x10;
    ts.sp += 4;
    return args.eax;
}

/* 0x438d60: takePolice. */
nfs::u32 takePoliceCars(App* app, ThreadState& ts, nfs::Args& args)
{
    const u32 entry = ts.sp, pair = args.esi;
    ts.sp += 4;
    const auto list = takePolice(app, ts);
    if (!list)
        return args.eax;
    putList(app, pair, *list);
    args.eax = pair;
    args.esi = entry - 0x14;
    return args.eax;
}

/* 0x438dc0: takeClassless. */
nfs::u32 takeClasslessCars(App* app, ThreadState& ts, nfs::Args& args)
{
    const u32 entry = ts.sp, pair = args.esi;
    ts.sp += 4;
    const auto list = takeClassless(app, ts);
    if (!list)
        return args.eax;
    putList(app, pair, *list);
    args.eax = pair;
    args.esi = entry - 0x10;
    return args.eax;
}

/* 0x438e30: takeOrdinary. */
nfs::u32 takeOrdinaryCars(App* app, ThreadState& ts, nfs::Args& args)
{
    const u32 entry = ts.sp, pair = args.esi;
    ts.sp += 4;
    const auto list = takeOrdinary(app, ts);
    if (!list)
        return args.eax;
    putList(app, pair, *list);
    args.eax = pair;
    args.esi = entry - 0x14;
    return args.eax;
}

/* 0x438eb0: sortList. */
nfs::u32 sortCars(App* app, ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    if (const auto left = sortList(app, ts))
        args.eax = *left;
    return args.eax;
}

/* 0x439250: nameOf the car eax. */
nfs::u32 carName(App* app, ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    if (const auto name = nameOf(app, ts, args.eax))
        args.eax = *name;
    return args.eax;
}

/* 0x4392b0: folderOf the car eax. */
nfs::u32 carFolder(App* app, ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    if (const auto folder = folderOf(app, ts, args.eax))
        args.eax = *folder;
    return args.eax;
}

/* 0x439300: prefixOf the car eax. */
nfs::u32 carFilePrefix(App* app, ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    if (const auto prefix = prefixOf(app, ts, args.eax))
        args.eax = *prefix;
    return args.eax;
}

/* 0x439520: idOf the car eax. */
nfs::u32 carId(App* app, ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    if (const auto id = idOf(app, ts, args.eax))
        args.eax = *id;
    return args.eax;
}

/* 0x439570: the car eax's flag 0x40 (the fedata's word at +0x14). */
nfs::u32 carFlag40(App* app, ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    if (const auto flags = flagsOf(app, ts, args.eax, kFlag40))
        args.eax = *flags;
    return args.eax;
}

/* 0x4395c0: the car eax's flag 4 (the fedata's word at +0xe). */
nfs::u32 carFlag4(App* app, ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    if (const auto flags = flagsOf(app, ts, args.eax, kFlag4))
        args.eax = *flags;
    return args.eax;
}

/* 0x439610: hasNumericId for the car eax. */
nfs::u32 carHasNumericId(App* app, ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    if (const auto numeric = hasNumericId(app, ts, args.eax))
        args.eax = *numeric;
    return args.eax;
}

/* 0x439620: isNumeric for the id at eax, 1 or 0. */
nfs::u32 idIsNumeric(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = isNumeric(app, args.eax) ? 1 : 0;
    ts.sp += 4;
    return args.eax;
}

/* 0x439660: numberOf the id at eax. */
nfs::u32 numericId(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = numberOf(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

/* 0x439680: whether the car eax is a bonus car (flag 1). */
nfs::u32 carIsBonus(App* app, ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    if (const auto flags = flagsOf(app, ts, args.eax, kBonus))
        args.eax = *flags;
    return args.eax;
}

/* 0x4396b0: whether the car eax is a police car (flag 0x10). */
std::optional<u32> policeFlag(App* app, ThreadState& ts, u32 car)
{
    return flagsOf(app, ts, car, kPolice);
}

nfs::u32 carIsPolice(App* app, ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    if (const auto flags = flagsOf(app, ts, args.eax, kPolice))
        args.eax = *flags;
    return args.eax;
}

/* 0x4396e0: whether the car eax is a Ferrari or a Mercedes (flag 0x20, the
 * .FEC's first text). */
std::optional<u32> makerFlag(App* app, ThreadState& ts, u32 car)
{
    return flagsOf(app, ts, car, kMaker);
}

nfs::u32 carIsFerrariOrMercedes(App* app, ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    if (const auto flags = flagsOf(app, ts, args.eax, kMaker))
        args.eax = *flags;
    return args.eax;
}

/* 0x439730: classOf the car eax. */
nfs::u32 carClass(App* app, ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    if (const auto cls = classOf(app, ts, args.eax))
        args.eax = *cls;
    return args.eax;
}

/* 0x439990: placeById for the id at eax. */
nfs::u32 findCarById(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = placeById(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

/* 0x4399f0: clearCache. */
nfs::u32 clearCarCache(App* app, ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    if (const auto left = clearCache(app, ts))
        args.eax = *left;
    return args.eax;
}

/* 0x439a50: recordAt the place eax. */
nfs::u32 carRecord(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = recordAt(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

/* 0x439af0: textAt the texts eax, the text edx; edx the offsets' address
 * where there is the text, as the code leaves it. */
nfs::u32 fecText(App* app, ThreadState& ts, nfs::Args& args)
{
    const u32 texts = args.eax, index = args.edx;
    args.eax = textAt(app, texts, index);
    if (hasText(app, texts, index))
        args.edx = at(app, texts + 8);
    ts.sp += 4;
    return args.eax;
}

/* 0x439b20: placeByName for the name at eax. */
nfs::u32 findCarByName(App* app, ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    if (const auto place = placeByName(app, ts, args.eax))
        args.eax = *place;
    return args.eax;
}

/* 0x439bd0: attachTexts of the fedata edx to the record eax: 1, edx the
 * offsets' address. */
nfs::u32 attachFecTexts(App* app, ThreadState& ts, nfs::Args& args)
{
    args.edx = attachTexts(app, args.eax, args.edx);
    args.eax = 1;
    ts.sp += 4;
    return args.eax;
}

/* 0x439c10: lockedBonus for the record eax. */
nfs::u32 bonusCarLocked(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = lockedBonus(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

/* 0x439d20: checkDuplicates. */
nfs::u32 checkDuplicateCars(App* app, ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    if (const auto left = checkDuplicates(app, ts))
        args.eax = *left;
    return args.eax;
}

namespace
{

/* 0x43a4f0's: whether the car can be chosen: not a Ferrari or Mercedes in
 * Hot Pursuit, not a police car; a car of flag 0x40 of the game's own, not
 * flag 4 nor a bonus car, can; else only the player's own car (and in a split
 * screen the second's).  1 or 0. */
std::optional<u32> selectable(App* app, ThreadState& ts, u32 car)
{
    const Frame frame(ts, 4 + 8, 0);
    if (gameSettings(app).raceType == 3)
    {
        const auto maker = flagsOf(app, ts, car, kMaker);
        if (!maker)
            return std::nullopt;
        if (*maker != 0)
            return 0;
    }
    const auto police = policeAt(app, ts, car);
    if (!police)
        return std::nullopt;
    if (*police != 0)
        return 0;
    const auto flag4 = flagsOf(app, ts, car, kFlag4);
    if (!flag4)
        return std::nullopt;
    if (*flag4 == 0)
    {
        const auto numeric = hasNumericId(app, ts, car);
        if (!numeric)
            return std::nullopt;
        if (*numeric != 0)
        {
            const auto bonus = flagsOf(app, ts, car, kBonus);
            if (!bonus)
                return std::nullopt;
            if (*bonus == 0)
            {
                const auto flag40 = flagsOf(app, ts, car, kFlag40);
                if (!flag40)
                    return std::nullopt;
                if (*flag40 != 0)
                    return 1;
            }
        }
    }
    const GameSettings& settings = gameSettings(app);
    if (settings.playMode == 0)
        return car == settings.players[0].car ? 1 : 0;
    if (settings.playMode == 1)
        return car == settings.players[0].car || car == settings.players[1].car ? 1 : 0;
    return 0;
}

/* 0x43a580's: the classes' lists: of the cars that can be chosen (the count
 * less [0x554f50]), each in list 0 and in its class's (class + 1), 0xc8
 * bytes each at 0x5552d8, their counts at 0x5555f8; lists 0 to 3 sorted by
 * name (qsort with 0x43a4d0).  The count of list 0. */
std::optional<u32> buildClassLists(App* app, ThreadState& ts)
{
    const Frame frame(ts, 4 + 0x14, 8);
    const u32 listed = at(app, kCount) - at(app, kNotListed);
    for (const auto& clear : {std::pair<u32, u32>{kClassLists, 0x320}, {kClassCounts, 0x10}})
    {
        guest::direct<0x4e070c, ea::clearBytes>(app, ts, clear.first, clear.second);
        if (ts.terminate)
            return std::nullopt;
    }
    for (u32 place = 0; s(place) < s(listed); ++place)
    {
        const auto chosen = selectable(app, ts, place);
        if (!chosen)
            return std::nullopt;
        if (*chosen == 0)
            continue;
        const auto cls = classOf(app, ts, place);
        if (!cls)
            return std::nullopt;
        const u32 list = *cls + 1;
        const auto record = carRecordAt(app, ts, place);
        if (!record)
            return std::nullopt;
        const u32 count = at(app, list * 4 + kClassCounts);
        put(app, list * 4 + kClassCounts, count + 1);
        put(app, list * kClassListBytes + count * 4 + kClassLists, *record);
        const auto again = carRecordAt(app, ts, place);
        if (!again)
            return std::nullopt;
        const u32 all = at(app, kClassCounts) + 1;
        put(app, all * 4 + kClassLists - 4, *again);
        put(app, kClassCounts, all);
    }
    for (u32 list = 0; list < 4; ++list)
        if (!typed<0x4ee4f8, watcom::clib::qsort>(app, ts, kClassLists + kClassListBytes * list,
                                                  at(app, kClassCounts + 4 * list), 4u, kClassCompare))
            return std::nullopt;
    return at(app, kClassCounts);
}

/* 0x43b3f0's: the car's specs (0x49c700, of its folder's files of its prefix)
 * into its record's +0x338 and +0x394, the word it gives (into the frame) to
 * +0x334.  0. */
std::optional<u32> loadSpecs(App* app, ThreadState& ts, u32 car)
{
    const Frame frame(ts, 4 + 0x18, 0x134);
    const u32 path = frame - 0x134, word = frame - 8;
    const auto record = carRecordAt(app, ts, car);
    if (!record)
        return std::nullopt;
    copyString(app, path, kCarmodelPath);
    if (!endPath(app, ts, path))
        return std::nullopt;
    const auto folder = carFolderAt(app, ts, car);
    if (!folder)
        return std::nullopt;
    appendString(app, path, *folder);
    if (!endPath(app, ts, path))
        return std::nullopt;
    const auto prefix = prefixOf(app, ts, car);
    if (!prefix)
        return std::nullopt;
    appendString(app, path, *prefix);
    if (!guest::direct<0x49c700, coloursRead>(app, ts, path, *record + kRecSpecs, u32(0x10), word,
                                               *record + kRecSpecsToo))
        return std::nullopt;
    put(app, *record + kRecSpecsWord, at(app, word));
    return 0;
}

/* 0x43b4f0's: the record's .FEC written to the file at `path`: 1, then its
 * quads at +0x32c and +0x378.  0, or 1 if it cannot be made. */
std::optional<u32> writeFec(App* app, ThreadState& ts, u32 path, u32 record)
{
    const Frame frame(ts, 4 + 0x14, 4);
    const u32 version = frame - 4;
    put(app, version, 1);
    const auto file = openFile(app, ts, path, kWriteBinary);
    if (!file)
        return std::nullopt;
    if (*file == 0)
        return 1;
    if (!writeFile(app, ts, version, 4, 1, *file))
        return std::nullopt;
    for (const u32 quad : {kRecFecFirst, kRecFecSecond})
        if (!writeFile(app, ts, record + quad, 8, 1, *file))
            return std::nullopt;
    if (!closeFile(app, ts, *file))
        return std::nullopt;
    return 0;
}

/* 0x43b570's: the .FEC open as `file` read into the record: from its start, a
 * word, then its two quads (the first's words to +0x32c and +0x330, the
 * second's to +0x378 and +0x37c).  0, edx the record as the code leaves it;
 * or 1 if the first word is not read. */
std::optional<u32> readFecBody(App* app, ThreadState& ts, u32 file, u32 record)
{
    const Frame frame(ts, 4 + 0x14, 0x18);
    const u32 version = frame - 8, first = frame - 0x18, second = frame - 0x10;
    if (!seekFile(app, ts, file, 0, kSeekStart))
        return std::nullopt;
    const auto read = readFile(app, ts, version, 4, 1, file);
    if (!read)
        return std::nullopt;
    if (*read != 1)
        return 1;
    if (!readFile(app, ts, first, 8, 1, file) || !readFile(app, ts, second, 8, 1, file))
        return std::nullopt;
    put(app, record + kRecFecFirst, at(app, first));
    put(app, record + kRecFecFirst + 4, at(app, first + 4));
    put(app, record + kRecFecSecond, at(app, second));
    put(app, record + kRecFecSecond + 4, at(app, second + 4));
    return 0;
}

/* 0x43b630's: the .FEC open as `file` read into the record where its first
 * word is 1 (readFecBody): 0, else -1. */
std::optional<u32> readFec(App* app, ThreadState& ts, u32 file, u32 record)
{
    const Frame frame(ts, 4 + 0x14, 4);
    const u32 version = frame - 4;
    if (!seekFile(app, ts, file, 0, kSeekStart))
        return std::nullopt;
    if (!readFile(app, ts, version, 4, 1, file))
        return std::nullopt;
    if (at(app, version) == 1)
    {
        const auto body = readFecBody(app, ts, file, record);
        if (!body)
            return std::nullopt;
        if (*body == 0)
            return 0;
    }
    return 0xffffffff;
}

/* 0x43b680's: the path of the car's .FEC into the buffer `path`: the carmodel
 * folder, its folder, its prefix and ".FEC".  What the code leaves in eax:
 * the prefix's address, its low byte 0 (the inlined strcat's al). */
std::optional<u32> fecPath(App* app, ThreadState& ts, u32 car, u32 path)
{
    const Frame frame(ts, 4 + 0x14, 0);
    copyString(app, path, kCarmodelPath);
    if (!endPath(app, ts, path))
        return std::nullopt;
    const auto folder = carFolderAt(app, ts, car);
    if (!folder)
        return std::nullopt;
    appendString(app, path, *folder);
    if (!endPath(app, ts, path))
        return std::nullopt;
    const auto prefix = prefixOf(app, ts, car);
    if (!prefix)
        return std::nullopt;
    appendString(app, path, *prefix);
    appendString(app, path, kFec);
    // The inlined strcat's push edi leaves the path's address under the frame, where readFec's word
    // lies after it: what readFec reads there where fread fills nothing (a .FEC too short).
    put(app, frame - 4, path);
    return *prefix & 0xffffff00;
}

/* 0x43b740's: the car's .FEC read into its record (readFec) where it is
 * there, made anew (writeFec) where its first word is not 1.  0, or -1
 * where it fails. */
std::optional<u32> loadFec(App* app, ThreadState& ts, u32 car)
{
    const Frame frame(ts, 4 + 0x18, 0x130);
    const u32 path = frame - 0x130, version = frame - 4;
    const auto record = carRecordAt(app, ts, car);
    if (!record || !fecPath(app, ts, car, path))
        return std::nullopt;
    const auto file = openFile(app, ts, path, kReadBinary);
    if (!file || *file == 0)
        return file;
    if (!readFile(app, ts, version, 4, 1, *file))
        return std::nullopt;
    const bool rewrite = at(app, version) != 1;
    const auto read = readFec(app, ts, *file, *record);
    if (!read || !closeFile(app, ts, *file))
        return std::nullopt;
    if (*read != 0)
        return 0xffffffff;
    if (!rewrite)
        return 0;
    const auto written = writeFec(app, ts, path, *record);
    if (!written)
        return std::nullopt;
    return *written != 0 ? 0xffffffff : 0;
}

/* loadCarList's car file closed as it has it open: its car.viv (closeViv,
 * handed the word `file`), or the fedata's own file (fclose). */
bool closeCarFile(App* app, ThreadState& ts, u32 viv, u32 file)
{
    if (viv != 0)
        return bool(guest::direct<0x43b9b0, ea::vivClosed>(app, ts, viv, file));
    return bool(closeFile(app, ts, at(app, file)));
}

/* loadCarList's menu item `place`: its text's id, on, white -- off and grey
 * for a bonus car still locked. */
void menuItem(App* app, u32 place, u32 record, u32 items, u32 on, u32 colours)
{
    putWord16(app, place * 2 + on, 1);
    putWord16(app, place * 2 + items, u16(place + kFirstItem));
    put(app, place * 4 + colours, kWhite);
    if (lockedBonus(app, record) != 0)
    {
        putWord16(app, place * 2 + on, 0);
        put(app, place * 4 + colours, kGrey);
    }
}

// The fedata's words that set a record's flags where they are not 0.
struct FlagWord
{
    u32 offset;
    u8 flag;
};

constexpr FlagWord kFlagWords[] = {{6, kBonus}, {8, 2}, {0xe, kFlag4}, {0x10, kPolice}, {0x14, kFlag40}};

}  // namespace

/* 0x439d90's: the cars read in: each folder in the carmodel folder but
 * "traffic" and those already in the list -- its fedata.<language>, or that
 * entry of its car.VIV, read whole; of version 9, and of the ids 15 or 16
 * only where the options (0x55eb2e) ask -- made a record (its flags, class
 * and the rest from the fedata, its texts, name, folder), put at the list's
 * end, its specs and .FEC loaded.  Then the list counted and sorted, the car
 * menus' items and colours made, the classes' lists, the duplicates checked
 * for, the players' settings checked.  0, or -1 where memory fails. */
std::optional<u32> loadCars(App* app, ThreadState& ts)
{
    const Frame frame(ts, 4 + 0x18, 0x278);
    // The frame's words its callees are handed: the file open, the data's offset and length in it; the paths.
    const u32 file = frame - 0x18, offset = frame - 0x1c, length = frame - 0x20;
    const u32 path = frame - 0x14c, vivPath = frame - 0x278;
    if (!clearCache(app, ts))
        return std::nullopt;
    // The cars counted: the next one's place.
    u32 count = countList(app);
    for (auto folder = guest::direct<0x4386b0, firstSubfolderIn>(app, ts, kCarmodelPath);;
         folder = guest::direct<0x4387e0, nextSubfolderFound>(app, ts))
    {
        if (!folder)
            return std::nullopt;
        if (*folder == 0)
            break;
        const auto traffic = compareNoCase(app, ts, *folder, kTraffic);
        if (!traffic)
            return std::nullopt;
        if (*traffic == 0)
            continue;
        bool listed = false;
        for (u32 car = at(app, kCarList); car != 0 && !listed; car = nextOf(app, car))
        {
            const auto order = compareStrings(app, ts, car + kRecFolder, *folder);
            if (!order)
                return std::nullopt;
            listed = *order == 0;
        }
        if (listed)
            continue;
        // The folder's path, and its fedata.<language> in it.
        copyString(app, path, kCarmodelPath);
        if (!endPath(app, ts, path))
            return std::nullopt;
        ++count;
        appendString(app, path, *folder);
        const auto language = menuCall<0x4782b0, frontend::loada::screenSizeNumberOf>(app, ts, at(app, kLanguage));
        if (!language || !printTo(app, ts, {path + stringLength(app, path), kFedataPath, *language}))
            return std::nullopt;
        const auto opened = openFile(app, ts, path, kReadBinary);
        if (!opened)
            return std::nullopt;
        put(app, file, *opened);
        u32 viv = 0;
        if (*opened != 0)
        {
            if (!seekFile(app, ts, *opened, 0, kSeekEnd))
                return std::nullopt;
            const auto bytes = tellFile(app, ts, at(app, file));
            if (!bytes)
                return std::nullopt;
            put(app, length, *bytes);
            put(app, offset, 0);
        }
        else
        {
            // Not there: the entry of that name in the folder's car.VIV.
            copyString(app, vivPath, path);
            const auto slash = lastOf(app, ts, vivPath, kBackslash);
            if (!slash || !printTo(app, ts, {*slash + 1, kCarVivUpper}))
                return std::nullopt;
            const auto openedViv = guest::direct<0x43b830, ea::vivOpened>(app, ts, vivPath, file);
            if (!openedViv)
                return std::nullopt;
            viv = *openedViv;
            if (viv == 0)
            {
                --count;
                continue;
            }
            const auto name = lastOf(app, ts, path, kBackslash);
            if (!name || !guest::direct<0x43b9d0, ea::vivFileFound>(app, ts, viv, *name + 1, offset, length))
                return std::nullopt;
        }
        if (at(app, length) == 0)
        {
            if (!closeCarFile(app, ts, viv, file))
                return std::nullopt;
            --count;
            continue;
        }
        // The record and the fedata's memory.
        const auto record = allocate(app, ts, kRecordBytes);
        if (!record)
            return std::nullopt;
        const auto fedata = allocate(app, ts, at(app, length) + 1);
        if (!fedata)
            return std::nullopt;
        if (*record == 0 || *fedata == 0)
        {
            if (!closeCarFile(app, ts, viv, file) || (*record != 0 && !release(app, ts, *record))
                || (*fedata != 0 && !release(app, ts, *fedata)))
                return std::nullopt;
            return 0xffffffff;
        }
        if (!fillBytes(app, ts, *record, 0, kRecordBytes)
            || !seekFile(app, ts, at(app, file), at(app, offset), kSeekStart)
            || !readFile(app, ts, *fedata, at(app, length), 1, at(app, file)) || !closeCarFile(app, ts, viv, file))
            return std::nullopt;
        // Its version: 9.
        const u32 version = widen(word16(app, *fedata + kFedVersion));
        putByte(app, *record + kRecVersion, u8(version));
        if (version != kVersion)
        {
            if (!release(app, ts, *record) || !release(app, ts, *fedata))
                return std::nullopt;
            --count;
            continue;
        }
        const u32 nibble = widen(word16(app, *fedata + kFedNibble)) & 0xf;
        putByte(app, *record + kRecNibble, byte(app, *record + kRecNibble) & 0xf0);
        put(app, *record + kRecNibble, at(app, *record + kRecNibble) | nibble);
        moveBytes(app, *record + kRecId, *fedata + kFedId, kIdBytes);
        if (!fillBytes(app, ts, *record + kRecHead, 0, 5))
            return std::nullopt;
        moveBytes(app, *record + kRecHead, *fedata, 4);
        for (const FlagWord& flag : kFlagWords)
            if (word16(app, *fedata + flag.offset) != 0)
                putByte(app, *record + kRecFlags, byte(app, *record + kRecFlags) | flag.flag);
        putByte(app, *record + kRecFedByte, byte(app, *fedata + kFedByte));
        putWord16(app, *record + kRecFedWord, word16(app, *fedata + kFedWord));
        put(app, *record + kRecClass, widen(word16(app, *fedata + kFedClass)));
        for (u32 i = 0; i < 4; ++i)
            putByte(app, *record + kRecFedBytes + i, byte(app, *fedata + kFedBytes + i));
        const u8 options = byte(app, kOptions);
        putByte(app, *record + kRecFedBytes + 4, byte(app, *fedata + kFedLastByte));
        if (options & 0x30)
        {
            const u32 id = *record + kRecId;
            if (numberOf(app, id) != 0xf && numberOf(app, id) != 0x10)
            {
                if (!release(app, ts, *record) || !release(app, ts, *fedata))
                    return std::nullopt;
                --count;
                continue;
            }
        }
        // Its texts (attachTexts' 1: the code's way out where it is 0 is never taken): the maker's (a Ferrari
        // or a Mercedes), its name.
        attachTexts(app, *record, *fedata);
        for (const u32 maker : {kFerrari, kMercedes})
        {
            const auto order = compareNoCase(app, ts, maker, textAt(app, *record + kRecTexts, kMakerText));
            if (!order)
                return std::nullopt;
            if (*order == 0)
                putByte(app, *record + kRecFlags, byte(app, *record + kRecFlags) | kMaker);
        }
        copyString(app, *record + kRecName, textAt(app, *record + kRecTexts, kNameText));
        copyString(app, *record + kRecFolder, *folder);
        copyString(app, *record + kRecFolderToo, *folder);
        if (!typed<0x4e0e30, watcom::clib::strncpy>(app, ts, *record + kRecPrefix, kCar, u32(4)))
            return std::nullopt;
        put(app, *record + kRecNext, 0);
        appendRecord(app, *record);
        put(app, kLastCar, *record);
        if (!loadSpecs(app, ts, count - 1) || !loadFec(app, ts, count - 1))
            return std::nullopt;
    }
    // The list counted and sorted; the menus' items.
    countList(app);
    if (!sortList(app, ts))
        return std::nullopt;
    putWord16(app, kMenuHeads, 0xc1);
    putWord16(app, kMenuHeads + 2, 0xbe);
    putWord16(app, kMenuHeads + 4, 0xbf);
    putWord16(app, kMenuHeads + 6, 0xc0);
    for (u32 i = 0; i < 4; ++i)
        put(app, kMenuColoursHead + 4 * i, kWhite);
    // The cars up to the police's.
    u32 place = 0;
    for (; s(place) < s(at(app, kCount)); ++place)
    {
        const auto record = carRecordAt(app, ts, place);
        if (!record)
            return std::nullopt;
        const auto police = policeAt(app, ts, place);
        if (!police)
            return std::nullopt;
        if (*police != 0)
            break;
        menuItem(app, place, *record, kCarItems, kCarItemOn, kCarColours);
    }
    putWord16(app, place * 2 + kCarItems, 0);
    // All the cars up to the one of id 0x21, its place kept.
    for (place = 0; s(place) < s(at(app, kCount)); ++place)
    {
        const auto record = carRecordAt(app, ts, place);
        if (!record)
            return std::nullopt;
        const auto id = idOf(app, ts, place);
        if (!id)
            return std::nullopt;
        if (numberOf(app, *id) == 0x21)
        {
            put(app, kCar21, place);
            break;
        }
        menuItem(app, place, *record, kAllItems, kAllItemOn, kAllColours);
    }
    putWord16(app, place * 2 + kAllItems, 0);
    if (!buildClassLists(app, ts) || !checkDuplicates(app, ts)
        || !guest::direct<0x4dc940, playerSpecs>(app, ts, u32(0))
        || !guest::direct<0x4dc940, playerSpecs>(app, ts, u32(1)))
        return std::nullopt;
    return 0;
}

namespace
{

/* copyCarAsNew's path to the new folder's car.viv. */
bool newVivPath(App* app, ThreadState& ts, u32 path, u32 folder)
{
    copyString(app, path, folder);
    if (!endPath(app, ts, path))
        return false;
    appendString(app, path, kCarViv);
    return true;
}

/* 0x43ae60's: a copy of the car `car` as a new car named `name`: a new
 * folder ("%04X") in the carmodel folder, the car's car.viv copied into it,
 * its fedata.<language> entry given the name and a new id (CoCreateGuid),
 * the .viv written back; the list loaded again and the new car's record
 * given back -- the folder removed and 0 where it fails.  The entry's done
 * again while it is found: the ids' loop leaves the languages' count at 4
 * (the game's bug), the language is always the game's.  edx as the code
 * leaves it.  No caller in the game. */
std::optional<u32> copyAsNew(App* app, ThreadState& ts, u32 name, u32 car)
{
    const Frame frame(ts, 4 + 0x14, 0x604);
    // The frame's: the .viv's length (handed to callees), the paths.
    const u32 vivBytes = frame - 0x18;
    const u32 vivPath = frame - 0x154, carVivPath = frame - 0x280, folder = frame - 0x3ac;
    const u32 carFolderPath = frame - 0x4d8, models = frame - 0x604;
    const auto fail = [&]() -> std::optional<u32> {
        if (!removeAll(app, ts, folder))
            return std::nullopt;
        return 0;
    };
    copyString(app, models, kCarmodelPath);
    if (!endPath(app, ts, models))
        return std::nullopt;
    copyString(app, folder, models);
    // The new folder's name after the carmodel folder's path.
    if (!freeFolderName(app, ts, models, folder + stringLength(app, folder), kHexFolder))
        return std::nullopt;
    const auto made = typed<0x4eb1d0, watcom::clib::mkdir>(app, ts, folder);
    if (!made)
        return std::nullopt;
    if (*made != 0)
        return 0;
    // The car's car.viv copied into the new folder.
    copyString(app, carFolderPath, models);
    const auto carFolder = carFolderAt(app, ts, car);
    if (!carFolder)
        return std::nullopt;
    appendString(app, carFolderPath, *carFolder);
    if (!newVivPath(app, ts, carVivPath, carFolderPath) || !newVivPath(app, ts, vivPath, folder)
        || !menuCall<0x452a00, frontend::loada::copyFile>(app, ts, vivPath, carVivPath)
        || !newVivPath(app, ts, vivPath, folder))
        return std::nullopt;
    const auto data = guest::direct<0x4e0fe0, ea::loadQuietly>(app, ts, vivPath, u32(0x10));
    if (!data)
        return std::nullopt;
    // The .viv parsed; the frame's word, never set, where it is not loaded.
    u32 viv = at(app, frame - 0xc);
    if (*data != 0)
    {
        const auto parsed = guest::direct<0x4195f0, openCache>(app, ts, *data);
        if (!parsed)
            return std::nullopt;
        viv = *parsed;
    }
    if (!release(app, ts, *data))
        return std::nullopt;
    // The fedata's entry: the name added, a new id.
    for (u32 language = 0; s(language) < 7; ++language)
    {
        const auto languageName = menuCall<0x478290, frontend::loada::screenSizeNumber>(app, ts);
        if (!languageName || !printTo(app, ts, {vivPath, kFedataName, *languageName}))
            return std::nullopt;
        const auto entry = guest::direct<0x418fe0, findCached>(app, ts, viv, vivPath);
        if (!entry)
            return std::nullopt;
        if (*entry == 0)
            continue;
        const auto block = allocate(app, ts, at(app, *entry + 0x20) + stringLength(app, name) + 1);
        if (!block)
            return std::nullopt;
        if (*block == 0)
            continue;
        moveBytes(app, *block, at(app, *entry + 0x24), at(app, *entry + 0x20));
        moveBytes(app, at(app, *entry + 0x20) + *block, name, stringLength(app, name) + 1);
        put(app, *block + 0x37, at(app, *entry + 0x20));
        if (!release(app, ts, at(app, *entry + 0x24)))
            return std::nullopt;
        put(app, *entry + 0x24, *block);
        put(app, *entry + 0x20, at(app, *entry + 0x20) + stringLength(app, name) + 1);
        // The entry's new id: the port makes none (it says so in the log), so the copy fails here.
        os::debugBreak();
        return fail();
    }
    // The .viv written over the copy, the list loaded again.
    if (!newVivPath(app, ts, vivPath, folder))
        return std::nullopt;
    if (!removeFile(app, ts, vivPath))
        return std::nullopt;
    const auto saved = guest::direct<0x419460, packedCache>(app, ts, viv, vivBytes);
    if (!saved)
        return std::nullopt;
    if (*saved != 0 && at(app, vivBytes) != 0
        && !guest::direct<0x4eebb0, saveBlockQuietly>(app, ts, vivPath, *saved, at(app, vivBytes)))
        return std::nullopt;
    if (!release(app, ts, *saved))
        return std::nullopt;
    const auto loaded = typed<0x439d90, loadCars>(app, ts);
    if (!loaded)
        return std::nullopt;
    if (*loaded != 0)
        return fail();
    const auto place = placeByName(app, ts, name);
    if (!place)
        return std::nullopt;
    return carRecordAt(app, ts, *place);
}

/* 0x43ba30's: the dword at `word` byte-swapped in place; the dword swapped. */
u32 swapInPlace(App* app, u32 word)
{
    u32 swapped = 0;
    for (u32 i = 0; i < 4; ++i)
        swapped |= u32(byte(app, word + i)) << (8 * (3 - i));
    put(app, word, swapped);
    return swapped;
}

/* 0x43ba70's: the dword `value` written big-endian to the file (through the
 * frame): fwrite's answer. */
std::optional<u32> writeSwapped(App* app, ThreadState& ts, u32 file, u32 value)
{
    const Frame frame(ts, 4 + 0xc, 4);
    const u32 word = frame - 4;
    put(app, word, value);
    swapInPlace(app, word);
    return writeFile(app, ts, word, 4, 1, file);
}

}  // namespace

/* 0x43baa0's: the carp.txt beside the .viv at `vivPath` appended to it, 0x200
 * bytes a time, and its first entry (+0x10, +0x14) pointed at it: where it
 * starts (the .viv's old length) and how long it is, big-endian.  The .viv's
 * fclose's answer. */
std::optional<u32> appendCarp(App* app, ThreadState& ts, u32 vivPath)
{
    const Frame frame(ts, 4 + 0x18, 0x30c);
    const u32 carpPath = frame - 0x10c, buffer = frame - 0x30c;
    copyString(app, carpPath, vivPath);
    const auto slash = lastOf(app, ts, carpPath, kBackslash);
    if (!slash)
        return std::nullopt;
    copyString(app, *slash + 1, kCarpTxt);
    const auto viv = openFile(app, ts, vivPath, kReadUpdate);
    if (!viv)
        return std::nullopt;
    const auto carp = openFile(app, ts, carpPath, kReadBinary);
    if (!carp || !seekFile(app, ts, *viv, 0, kSeekEnd))
        return std::nullopt;
    const auto vivLength = tellFile(app, ts, *viv);
    if (!vivLength || !seekFile(app, ts, *carp, 0, kSeekEnd))
        return std::nullopt;
    const auto carpLength = tellFile(app, ts, *carp);
    if (!carpLength || !seekFile(app, ts, *carp, 0, kSeekStart))
        return std::nullopt;
    for (;;)
    {
        const auto read = readFile(app, ts, buffer, 1, 0x200, *carp);
        if (!read)
            return std::nullopt;
        if (*read == 0)
            break;
        if (!writeFile(app, ts, buffer, 1, *read, *viv))
            return std::nullopt;
        if (*read != 0x200)
            break;
    }
    if (!closeFile(app, ts, *carp) || !seekFile(app, ts, *viv, 0x10, kSeekStart)
        || !writeSwapped(app, ts, *viv, *vivLength) || !seekFile(app, ts, *viv, 0x14, kSeekStart)
        || !writeSwapped(app, ts, *viv, *carpLength))
        return std::nullopt;
    return closeFile(app, ts, *viv);
}

/* 0x43bc40's: the last car loaded's name printed with `format` (and 0x100 its
 * argument): sprintf's answer, 0 where there is none. */
std::optional<u32> nameLastCar(App* app, ThreadState& ts, u32 format)
{
    const u32 car = at(app, kLastCar);
    if (car == 0)
        return 0;
    const Frame frame(ts, 4 + 8, 0);
    // The frame of the two registers the code saves (edx, then ebp): their values are not written.
    return printTo(app, ts, {car + kRecName, format, 0x100});
}

/* 0x439d90: loadCars. */
nfs::u32 loadCarList(App* app, ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    if (const auto answer = loadCars(app, ts))
        args.eax = *answer;
    return args.eax;
}

/* 0x43a4f0: selectable for the car eax. */
nfs::u32 carSelectable(App* app, ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    if (const auto chosen = selectable(app, ts, args.eax))
        args.eax = *chosen;
    return args.eax;
}

/* 0x43a580: buildClassLists. */
nfs::u32 buildCarClassLists(App* app, ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    if (const auto count = buildClassLists(app, ts))
        args.eax = *count;
    return args.eax;
}

/* 0x43ae60: copyAsNew of the car edx named eax. */
nfs::u32 copyCarAsNew(App* app, ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    if (const auto record = copyAsNew(app, ts, args.eax, args.edx))
        args.eax = *record;
    return args.eax;
}

/* 0x43b3f0: loadSpecs for the car eax. */
nfs::u32 loadCarSpecs(App* app, ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    if (const auto answer = loadSpecs(app, ts, args.eax))
        args.eax = *answer;
    return args.eax;
}

/* 0x43b4f0: writeFec of the record edx to the file eax; edx the answer too,
 * as the code leaves it. */
nfs::u32 writeCarFec(App* app, ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    if (const auto answer = writeFec(app, ts, args.eax, args.edx))
        args.eax = args.edx = *answer;
    return args.eax;
}

/* 0x43b570: readFecBody of the file eax into the record edx. */
nfs::u32 readCarFecBody(App* app, ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    if (const auto answer = readFecBody(app, ts, args.eax, args.edx))
        args.eax = *answer;
    return args.eax;
}

/* 0x43b630: readFec of the file eax into the record edx. */
nfs::u32 readCarFec(App* app, ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    if (const auto answer = readFec(app, ts, args.eax, args.edx))
        args.eax = *answer;
    return args.eax;
}

/* 0x43b680: fecPath of the car eax into the buffer edx. */
nfs::u32 carFecPath(App* app, ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    if (const auto left = fecPath(app, ts, args.eax, args.edx))
        args.eax = *left;
    return args.eax;
}

/* 0x43b740: loadFec for the car eax. */
nfs::u32 loadCarFec(App* app, ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    if (const auto answer = loadFec(app, ts, args.eax))
        args.eax = *answer;
    return args.eax;
}

/* 0x43ba20: the placeholder car's car.viv (".\fedata\plac\car.viv"). */
nfs::u32 placeCarVivPath(App*, ThreadState& ts, nfs::Args& args)
{
    args.eax = kPlaceViv;
    ts.sp += 4;
    return args.eax;
}

/* 0x43ba30: swapInPlace for the dword at eax. */
nfs::u32 swapBytes(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = swapInPlace(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

/* 0x43ba70: writeSwapped of edx to the file eax. */
nfs::u32 writeBigEndian(App* app, ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    if (const auto answer = writeSwapped(app, ts, args.eax, args.edx))
        args.eax = *answer;
    return args.eax;
}

/* 0x43baa0: appendCarp to the .viv at eax. */
nfs::u32 appendCarpToViv(App* app, ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    if (const auto answer = appendCarp(app, ts, args.eax))
        args.eax = *answer;
    return args.eax;
}

/* 0x43bbf0: the record of the last car loaded (0x554f74). */
nfs::u32 lastCarLoaded(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = at(app, kLastCar);
    ts.sp += 4;
    return args.eax;
}

/* 0x43bc00: the 16 bytes at `id` made the last car loaded's id; the count
 * copied (rep movsd's), 0 where there is none. */
u32 setLastId(App* app, u32 id)
{
    const u32 car = at(app, kLastCar);
    if (car != 0)
        moveBytes(app, car + kRecId, id, kIdBytes);
    return car != 0 ? kIdBytes : 0;
}

/* setLastId of eax. */
nfs::u32 setLastCarId(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = setLastId(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

/* 0x43bc40: nameLastCar with the format eax. */
nfs::u32 setLastCarName(App* app, ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    if (const auto answer = nameLastCar(app, ts, args.eax))
        args.eax = *answer;
    return args.eax;
}

/* 0x43bc70: `flags` made the last car loaded's flags; the car. */
u32 setLastFlags(App* app, u32 flags)
{
    const u32 car = at(app, kLastCar);
    if (car != 0)
        put(app, car + kRecFlags, flags);
    return car;
}

/* setLastFlags of eax. */
nfs::u32 setLastCarFlags(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = setLastFlags(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

/* 0x43bc90: the last car loaded given flag 8 (put after the others by
 * sortCars); the car. */
u32 markLastAfterOthers(App* app)
{
    const u32 car = at(app, kLastCar);
    if (car != 0)
        putByte(app, car + kRecFlags, byte(app, car + kRecFlags) | kAfterOthers);
    return car;
}

nfs::u32 markLastCarFlag8(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = markLastAfterOthers(app);
    ts.sp += 4;
    return args.eax;
}

/* The typed cores of the thunks above, for callers that call them as C++ (native_load_cardb.h). */
bool slashAdded(App* app, ThreadState& ts, u32 path)
{
    return endPath(app, ts, path).has_value();
}

std::optional<u32> folderRemoved(App* app, ThreadState& ts, u32 folder)
{
    return removeAll(app, ts, folder);
}

std::optional<u32> firstSubfolderIn(App* app, ThreadState& ts, u32 folder)
{
    return firstFound(app, ts, kFolders, folder);
}

std::optional<u32> nextSubfolderFound(App* app, ThreadState& ts)
{
    return nextFound(app, ts, kFolders);
}

std::optional<u32> firstFileIn(App* app, ThreadState& ts, u32 folder)
{
    return firstFound(app, ts, kFiles, folder);
}

std::optional<u32> nextFileFound(App* app, ThreadState& ts)
{
    return nextFound(app, ts, kFiles);
}

u32 firstListedCar(App* app)
{
    return firstListed(app);
}

u32 nextListedCar(App* app)
{
    return nextListed(app);
}

u32 carsListed(App* app)
{
    return at(app, kCount);
}

std::optional<u32> classOfCar(App* app, ThreadState& ts, u32 car)
{
    return classOf(app, ts, car);
}

std::optional<u32> numericIdOf(App* app, ThreadState& ts, u32 car)
{
    return hasNumericId(app, ts, car);
}

std::optional<u32> flag40Of(App* app, ThreadState& ts, u32 car)
{
    return flagsOf(app, ts, car, kFlag40);
}

std::optional<u32> flag4Of(App* app, ThreadState& ts, u32 car)
{
    return flagsOf(app, ts, car, kFlag4);
}

std::optional<u32> bonusFlagOf(App* app, ThreadState& ts, u32 car)
{
    return flagsOf(app, ts, car, kBonus);
}

std::optional<u32> classListsBuilt(App* app, ThreadState& ts)
{
    return buildClassLists(app, ts);
}

u32 bonusLocked(App* app, u32 record)
{
    return lockedBonus(app, record);
}

u32 fecTextOf(App* app, u32 texts, u32 index)
{
    return textAt(app, texts, index);
}

std::optional<u32> fecPathOf(App* app, ThreadState& ts, u32 car, u32 path)
{
    return fecPath(app, ts, car, path);
}

std::optional<u32> fecWritten(App* app, ThreadState& ts, u32 path, u32 record)
{
    return writeFec(app, ts, path, record);
}

u32 placeholderViv(App*)
{
    return kPlaceViv;
}

}  // namespace load
}  // namespace nfs3hp
