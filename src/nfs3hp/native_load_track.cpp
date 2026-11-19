#include "game_settings.h"
#include "native_ea_files.h"
#include "native_ea_memory.h"
#include "native_ea_support.h"
#include "native_frontend_draw.h"
#include "native_frontend_eab.h"
#include "native_frontend_loada.h"
#include "native_frontend_loopa.h"
#include "native_frontend_loopb.h"
#include "native_frontend_renda.h"
#include "native_frontend_rende.h"
#include "native_frontend_simd.h"
#include "native_frontend_wata.h"
#include "native_guest.h"
#include "native_load_saves.h"
#include "native_load_track.h"
#include "native_raceload.h"
#include "native_vertices.h"
#include "native_watcom.h"

#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <optional>
#include <type_traits>
#include <utility>

/* The game's data: the race's data files -- the track's paths, its textures,
 * horizon, sky and weather, the opponents' speed tables, the cameras, the
 * ghost -- and the file cache the front end keeps its own data in.
 *
 * The file cache (0x418d00 to 0x4195f0, which the front end's fedata.%s goes
 * through, not the race's): a cache ("BFL", 12 bytes) is the magic 'FGIB' --
 * a BIGF archive's 'BIGF' as a little-endian dword -- its list's head at +4
 * and its last node at +8.  Its nodes ("FileNode", 0x30 bytes) are a name at
 * +0 (up to 0x20 bytes), the file's size at +0x20, its data ("mpFileData")
 * at +0x24, the next node at +0x28 and the one before at +0x2c; the head is
 * an empty node.  A cache is read from a BIGF archive in memory (its files
 * copied out, by EA's BIG functions 0x4ea440, 0x4ea4a0 and 0x4ea290), and
 * written back into one ("BigBuffer"): the header's four big-endian dwords
 * ('BIGF', the archive's size, the files, the header's size), then each
 * file's offset, size and name, then the files.
 *
 * The race's: the track's folder is "<data path>TRK<track>\" ([0x7a27b4],
 * [0x6fd4ac]), its paths sprintf'd to 0x5dd86c and 0x5dd8ec.  The opponents'
 * speeds (speedsF.bin, speedsR.bin backwards) at [0x552558], their second
 * half at [0x55255c].  The cameras: 256 views of 24 bytes at 0x56f234 cleared,
 * the track's .can files ("<its path>00a.can", then "01.can" to "07.can" as
 * far as they go) loaded to 0x570a34 on.  The horizon (3Trnn[N][W].hrz, night and weather): numbers
 * read one by one (0x4be020) into the sky's colours, the sun's place and the
 * clouds', a list of 36-byte records at [0x55d278] and the weather's
 * counts at 0x8ca1e8, then the sky's 8 bands of colours worked out.  The sky
 * (sky.fsh: the clouds', the sun's and its glow's shapes by the time of day
 * and the weather) and the weather's (weather.fsh: the raindrops' and the
 * splashes').  The ghost: 0x20f4 bytes at 0x6ba5c4 and 18 bytes for each of
 * its [0x6bc6b4] records after them, read and written as one.
 *
 * Each is a typed function under a name of its own, with a thunk under the game's name for
 * the callers still on registers and the calls through a pointer: it leaves the registers as the game's code
 * does -- those it pushes and pops as they were, the others as its code leaves them, which for
 * some is what the last call left there.  The other typed files' functions their headers
 * declare are called as C++ (guest::direct); every other function of the game's is called by
 * address through a small helper here, its arguments in the registers and on the guest's stack
 * as the code hands them; a frame handed to one stays on the guest's stack at the code's depth
 * (GuestFrame), so the callee sees the same addresses.  The x87's operations are fp's, in the
 * game's order, a float kept where the game stores one. */
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

// The file cache's: its magic, its blocks' names.
constexpr u32 kBigMagic = 0x42494746;  // 'FGIB': "BIGF" read as a dword
constexpr u32 kNodeName = 0x536c38, kDataName = 0x536c44, kCacheName = 0x536c50, kBufferName = 0x536c54;
// A cache's: its list's head, its last node.  A node's: its name (at +0), its file's size and
// data, the next node and the one before; a node's bytes.
constexpr u32 kCacheList = 4, kCacheLast = 8, kCacheBytes = 0xc;
constexpr u32 kNodeSize = 0x20, kNodeData = 0x24, kNodeNext = 0x28, kNodePrevious = 0x2c, kNodeBytes = 0x30;
// A BIGF header's: its four numbers' bytes; an entry's offset, size and name.
constexpr u32 kBigHeaderBytes = 0x10, kEntrySize = 4, kEntryName = 8;
// The game's allocator's flags for the cache's blocks.
constexpr u32 kBlockFlags = 0x10;
// The track's: the data path, the track, the paths' buffers and formats.
constexpr u32 kDataPath = 0x7a27b4;
constexpr u32 kTrackFile = 0x5dd86c, kTrackFileFormat = 0x536cec;
constexpr u32 kFolderFile = 0x5dd8ec, kFolderFileFormat = 0x536d00;
// The race: backwards, the track's own textures, the night, the weather; the
// options' byte (2: the textures' smaller set); the loading stopped.
constexpr u32 kOptions = 0x7a3a58, kLoadAborted = 0x55e88c;
constexpr u8 kSmallTextures = 2;
// The opponents' speeds: speedsR.bin, speedsF.bin; the table, its second
// half, the failure's flag; the track's nodes.
constexpr u32 kSpeedsBackward = 0x53679c, kSpeedsForward = 0x5367a8;
constexpr u32 kSpeeds = 0x552558, kSpeedsHalf = 0x55255c, kSpeedsFailed = 0x56f22c, kTrackNodes = 0x5dd940;
constexpr u32 kSpeedsLoadFlags = 0x10;
// The cameras: the views (24 bytes each, 256), the .can files loaded (slots 1 to 7), the names.
constexpr u32 kCameraViews = 0x56f234, kCameraFiles = 0x570a30;
constexpr u32 kCameraViewBytes = 24, kCameraViewCount = 0x100, kCameraFileCount = 7;
constexpr u32 kFirstCameraName = 0x5367fc, kCameraName = 0x536808;
// The track's textures: the shapes loaded, the .qfs files' names, the trace.
constexpr u32 kTrackShapes = 0x5dd850;
constexpr u32 kNear8 = 0x536d20, kNear = 0x536d28, kFar8 = 0x536d58, kFar = 0x536d60;
constexpr u32 kTraceFile = 0x536d30, kTraceFormat = 0x536d50, kTraceLine = 0x744, kTraceWords = 0xc;
// The ghost: replaying, saving one; its file's name; its record, its records'
// count, its records; ghost read.
constexpr u32 kGhostName = 0x53b0c8;
constexpr u32 kGhost = 0x6ba5c4, kGhostCount = 0x6bc6b4, kGhostRecords = 0x6bc6b8, kGhostRead = 0x55d000;
constexpr u32 kGhostBytes = 0x20f4, kGhostRecordBytes = 18;
// The horizon's .hrz names (night and weather, night, weather, neither); the
// records' block's name; its numbers: 0.01, 360/1024 in 16.16 turns, the
// sun's scales, 2^-16 (a float), -1, 2^-16 (a double).
constexpr u32 kHorizonNightWeather = 0x53b238, kHorizonNight = 0x53b250, kHorizonWeather = 0x53b268;
constexpr u32 kHorizonName = 0x53b280, kNoName = 0x53b298, kHundredth = 0x53b29c;
constexpr u32 kHeightScale = 0x53b2a0, kSunAngleScale = 0x53b2a8, kSunDistanceScale = 0x53b2b0;
constexpr u32 kTurnScale = 0x53b2b8, kMinusOne = 0x53b2bc, kFixedScale = 0x53b2c0;
// The sky: sky.fsh's name, its shapes' names (clouds, sun, glow: night and
// weather, night, weather, day), the cloud's palette and the sun's.
constexpr u32 kSkyName = 0x53b340, kCloudPalette = 0x55d21c, kSunPalette = 0x6ff9a0, kSunShapes = 0x563a90;
// The weather: weather.fsh's name and its folder, the drops' and the
// splashes' names, their shapes; the weather's count (the horizon's).
constexpr u32 kWeatherName = 0x549794, kWeatherPath = 0x7a2934, kDropName = 0x5497a4, kSplashName = 0x5497ac;
constexpr u32 kDropShapes = 0x8ca200, kSplashShapes = 0x8ca288, kWeatherCount = 0x8ca1e8;
// What is called by address: the ghost's saving (0x477a90 leaves the registers as it leaves
// them) and fwrite (no typed core).  (The game's memory, a file there and loaded, a .viv's entry
// located, the trace, a handle read, the fixed sine and cosine and a vector's length, the C
// library's memset, stricmp, sprintf and fseek, the loading's shapes, and the menu files' BIG
// archive's count and names, the loading bar's tick, the horizon's numbers, a shape's texture,
// the sky's shape and grid, the sine and cosine are the other typed files', called as C++:
// native_ea_memory.h, native_ea_support.h, native_load_saves.h, native_raceload.h,
// native_ea_files.h, native_vertices.h, native_watcom.h, native_frontend_draw.h, and through
// feCore native_frontend_eab.h, _loada.h, _loopb.h, _rende.h, _wata.h, _renda.h, _simd.h.)
constexpr u32 kSaveGhostFile = 0x4777d0;
constexpr u32 kChop = 0x4dfd56;

/* The registers as a thunk is handed them, to put back those the game's code pushes and pops. */
struct Entry
{
    explicit Entry(const nfs::Args& args)
        : ebx(args.ebx), ecx(args.ecx), edx(args.edx), esi(args.esi), edi(args.edi), ebp(args.ebp)
    {
    }
    u32 ebx, ecx, edx, esi, edi, ebp;
};

/* A function's frame on the guest's stack, as deep as the code's: `pushed` bytes of registers
 * below esp, then `bytes` of locals; esp below them while it lasts. */
class GuestFrame
{
public:
    GuestFrame(ThreadState& ts, u32 pushed, u32 bytes) : ts(ts), esp(ts.sp), base(ts.sp - pushed)
    {
        ts.sp = base - bytes;
    }
    ~GuestFrame() { ts.sp = esp; }
    u32 operator-(u32 offset) const { return base - offset; }

private:
    ThreadState& ts;
    const u32 esp;
    const u32 base;
};

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

/* A block of `bytes` from the game's allocator, named `name`: the block or 0. */
std::optional<u32> allocate(App* app, ThreadState& ts, u32 name, u32 bytes, u32 flags)
{
    // edx and ebx as the code hands them, which the allocator keeps: the code's own after it (an
    // thunk leaves them so).
    return guest::direct<0x4e1620, ea::reserved>(app, ts, name, bytes, flags);
}

/* A block given back to the game's allocator.  False when the game stopped. */
bool release(App* app, ThreadState& ts, u32 block)
{
    return guest::direct<0x4e1890, ea::purgeBlock>(app, ts, block).has_value();
}

/* memset (0x4e0640): `bytes` bytes at `to` set to `value`; edx and ebx as the code loads them,
 * which memset keeps.  False when the game stopped. */
bool fill(App* app, ThreadState& ts, u32 to, u32 value, u32 bytes)
{
    guest::direct<0x4e0640, watcom::clib::memset>(app, ts, to, value, bytes);
    return !ts.terminate;
}

/* stricmp (0x4e9ec0) of the names at `name` and `other`: 0 when they are alike; edx as the code
 * loads it, which stricmp keeps. */
std::optional<u32> compareNames(App* app, ThreadState& ts, u32 name, u32 other)
{
    const u32 differs = guest::direct<0x4e9ec0, watcom::clib::stricmp>(app, ts, name, other);
    if (ts.terminate)
        return std::nullopt;
    return differs;
}

/* sprintf (0x4df690) to the first word, of the format in the second and what follows it. */
std::optional<u32> format(App* app, ThreadState& ts, std::initializer_list<u32> words)
{
    const u32* const word = words.begin();
    guest::Words rest({});
    for (const u32* w = word + 2; w != words.end(); ++w)
        rest.word[rest.count++] = *w;
    const u32 written = guest::direct<0x4df690, watcom::clib::sprintf>(app, ts, word[0], word[1], rest);
    if (ts.terminate)
        return std::nullopt;
    return written;
}

/* Whether the file named `name` is there. */
std::optional<bool> fileExists(App* app, ThreadState& ts, u32 name)
{
    return guest::direct<0x4e0e60, ea::fileThere>(app, ts, name);
}

/* The file named `name` loaded into a block of its own (`flags` the allocator's): the block or 0. */
std::optional<u32> loadFile(App* app, ThreadState& ts, u32 name, u32 flags)
{
    return guest::direct<0x4e0ec0, ea::loadWhole>(app, ts, name, flags);
}

/* The loading screen's bar ticked on (0x4a33d0, loada's): whether to stop, which its callers
 * leave in eax as the last call.  Nothing when the game stopped. */
std::optional<u32> loadingStop(App* app, ThreadState& ts)
{
    return feCore<0x4a33d0, frontend::loada::stopCheck>(app, ts);
}

/* The same, false when the game stopped. */
bool loadingTick(App* app, ThreadState& ts)
{
    return loadingStop(app, ts).has_value();
}

/* A dword turned around: a BIGF archive's big-endian number. */
u32 turned(u32 value)
{
    return value >> 24 | (value >> 8 & 0xff00) | (value << 8 & 0xff0000) | value << 24;
}

/* rep movsd, rep movsb: `bytes` bytes from `from` to `to`, a dword at a time and the last few
 * a byte at a time, forwards. */
void copyBytes(App* app, u32 to, u32 from, u32 bytes)
{
    for (u32 dwords = bytes >> 2; dwords != 0; --dwords, to += 4, from += 4)
        put(app, to, at(app, from));
    for (u32 left = bytes & 3; left != 0; --left)
        putByte(app, to++, byte(app, from++));
}

/* The code's inlined strcpy: the name at `from` to `to`, its ending too, a byte at a time. */
void copyName(App* app, u32 to, u32 from)
{
    for (u32 i = 0;; ++i)
    {
        const u8 c = byte(app, from + i);
        putByte(app, to + i, c);
        if (c == 0)
            break;
    }
}

/* The code's inlined strlen (repne scasb). */
u32 nameLength(App* app, u32 name)
{
    u32 length = 0;
    while (byte(app, name + length) != 0)
        ++length;
    return length;
}

}  // namespace

nfs::u32 loadOpponentSpeeds(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 clearTrackCameras(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 loadTrackCameras(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 swapBigEndian(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 newFileNode(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 freeFileNode(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 findFileNode(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 insertFileNode(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 newEmptyFileNode(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 freeFileNodes(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 countFileNodes(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 findCachedFile(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 addCachedFile(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 newFileCache(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 freeFileCache(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 bigHeaderSize(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 cachedDataSize(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 writeBigFile(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 packFileCache(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 readBigFile(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 openFileCache(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 trackFilePath(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 trackFolderPath(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 loadTrackTextures(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 saveGhost(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 writeGhost(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 readGhost(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 loadHorizon(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 loadSky(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 loadWeatherShapes(App* app, ThreadState& ts, nfs::Args& args);

namespace
{

/* The file `name` in the track's folder, "<data path>TRKnnn\<name>", sprintf'd to 0x5dd8ec:
 * that path. */
std::optional<u32> trackFolderFile(App* app, ThreadState& ts, u32 name)
{
    if (!format(app, ts, {kFolderFile, kFolderFileFormat, kDataPath, gameSettings(app).track, name}))
        return std::nullopt;
    return kFolderFile;
}

}  // namespace

/* The track's file named by its end `end`, "<data path>TRKnnn\Trnn<end>", sprintf'd to 0x5dd86c:
 * that path. */
std::optional<u32> trackFile(App* app, ThreadState& ts, u32 end)
{
    const u32 track = gameSettings(app).track;
    if (!format(app, ts, {kTrackFile, kTrackFileFormat, kDataPath, track, track, end}))
        return std::nullopt;
    return kTrackFile;
}

/* The opponents' speed table, speedsF.bin (speedsR.bin backwards) in the track's folder, loaded
 * to [0x552558] if it is there, [0x55255c] its second half (half the track's nodes, rounded up,
 * on) and [0x56f22c] 0; else [0x552558] 0 and [0x56f22c] 1.  The second half's index, or 0. */
std::optional<u32> loadSpeeds(App* app, ThreadState& ts)
{
    const std::optional<u32> path =
        trackFolderFile(app, ts, gameSettings(app).backwards != 0 ? kSpeedsBackward : kSpeedsForward);
    if (!path)
        return std::nullopt;
    const std::optional<bool> there = fileExists(app, ts, *path);
    if (!there)
        return std::nullopt;
    u32 table = 0;
    if (*there)
    {
        const std::optional<u32> loaded = loadFile(app, ts, *path, kSpeedsLoadFlags);
        if (!loaded)
            return std::nullopt;
        table = *loaded;
    }
    if (table == 0)
    {
        put(app, kSpeeds, 0);
        put(app, kSpeedsFailed, 1);
        return 0;
    }
    const u32 half = u32(s32(at(app, kTrackNodes) + 1) / 2);
    put(app, kSpeeds, table);
    put(app, kSpeedsHalf, table + half);
    put(app, kSpeedsFailed, 0);
    return half;
}

/* The cameras' 256 views (24 bytes from 0x56f234) cleared -- 0, -1, 0 -- the loading bar
 * ticked before each.  False when the game stopped. */
bool clearCameras(App* app, ThreadState& ts)
{
    for (u32 view = 0; view < kCameraViewCount; ++view)
    {
        if (!loadingTick(app, ts))
            return false;
        const u32 at = kCameraViews + view * kCameraViewBytes;
        put(app, at, 0);
        put(app, at + 4, 0xffffffff);
        put(app, at + 8, 0);
    }
    return true;
}

/* The track's cameras, its path's start `path`: the views cleared, the 7 files' slots at
 * 0x570a34 on cleared, then -- unless the loading was stopped ([0x55e88c]: 0 back) --
 * "<path>00a.can", "<path>01.can", "<path>02.can"... loaded into them while they are there, up
 * to 7.  How many. */
std::optional<u32> loadCameras(App* app, ThreadState& ts, u32 path)
{
    const GuestFrame frame(ts, 0x14, 0x100);
    const u32 name = frame - 0x100;  // handed to sprintf and the loading
    if (!clearCameras(app, ts))
        return std::nullopt;
    for (u32 slot = 1; slot <= kCameraFileCount; ++slot)
    {
        if (!loadingTick(app, ts))
            return std::nullopt;
        put(app, kCameraFiles + slot * 4, 0);
    }
    if (at(app, kLoadAborted) != 0)
        return 0;
    u32 loaded = 0;
    u32 nameFormat = kFirstCameraName;
    for (;;)
    {
        if (!format(app, ts, {name, nameFormat, path, loaded}))
            return std::nullopt;
        if (s32(loaded) >= s32(kCameraFileCount))
            break;
        const std::optional<bool> there = fileExists(app, ts, name);
        if (!there)
            return std::nullopt;
        if (!*there)
            break;
        if (!loadingTick(app, ts))
            return std::nullopt;
        ++loaded;
        const std::optional<u32> file = loadFile(app, ts, name, 0);
        if (!file)
            return std::nullopt;
        nameFormat = kCameraName;
        put(app, kCameraFiles + loaded * 4, *file);
    }
    return loaded;
}

namespace
{

/* The dword at `address` turned around in place (it read a byte at a time): what it is now. */
u32 swapInPlace(App* app, u32 address)
{
    const u32 value = turned(at(app, address));
    put(app, address, value);
    return value;
}

/* A file cache's node, "FileNode" (0x30 bytes), allocated and cleared -- cleared at 0 too if
 * there is none: the node or 0. */
std::optional<u32> newNode(App* app, ThreadState& ts)
{
    const std::optional<u32> node = allocate(app, ts, kNodeName, kNodeBytes, kBlockFlags);
    if (!node || !fill(app, ts, *node, 0, kNodeBytes))
        return std::nullopt;
    return node;
}

/* The node freed, its file's data first (if any); its size, its data and its name's first
 * byte cleared before.  False when the game stopped. */
bool freeNode(App* app, ThreadState& ts, u32 node)
{
    if (node == 0)
        return true;
    put(app, node + kNodeSize, 0);
    const u32 data = at(app, node + kNodeData);
    putByte(app, node, 0);
    if (data != 0)
    {
        if (!release(app, ts, data))
            return false;
        put(app, node + kNodeData, 0);
    }
    return release(app, ts, node);
}

/* The node named `name` (stricmp) of the list after the node `after`, or 0. */
std::optional<u32> findNode(App* app, ThreadState& ts, u32 name, u32 after)
{
    for (u32 node = at(app, after + kNodeNext); node != 0; node = at(app, node + kNodeNext))
    {
        const std::optional<u32> differs = compareNames(app, ts, name, node);
        if (!differs)
            return std::nullopt;
        if (*differs == 0)
            return node;
    }
    return 0;
}

/* A node made for the file named `name`, of `size` bytes at `data`, and linked in after the
 * node `after`: the node, or 0. */
std::optional<u32> insertNode(App* app, ThreadState& ts, u32 name, u32 size, u32 data, u32 after)
{
    const std::optional<u32> node = newNode(app, ts);
    if (!node || *node == 0)
        return node;
    copyName(app, *node, name);
    put(app, *node + kNodeSize, size);
    put(app, *node + kNodeData, data);
    const u32 next = at(app, after + kNodeNext);
    if (next != 0)
        put(app, next + kNodePrevious, *node);
    put(app, *node + kNodeNext, at(app, after + kNodeNext));
    put(app, *node + kNodePrevious, after);
    put(app, after + kNodeNext, *node);
    return node;
}

/* An empty node -- no name, size, data or links -- or 0. */
std::optional<u32> newEmptyNode(App* app, ThreadState& ts)
{
    const std::optional<u32> node = newNode(app, ts);
    if (!node || *node == 0)
        return node;
    put(app, *node + kNodeSize, 0);
    put(app, *node + kNodeData, 0);
    put(app, *node + kNodeNext, 0);
    put(app, *node + kNodePrevious, 0);
    putByte(app, *node, 0);
    return node;
}

/* The node and every node after it freed.  False when the game stopped. */
bool freeNodes(App* app, ThreadState& ts, u32 node)
{
    if (node == 0)
        return true;
    u32 next = at(app, node + kNodeNext);
    if (!freeNode(app, ts, node))
        return false;
    while (next != 0)
    {
        node = next;
        next = at(app, node + kNodeNext);
        if (!freeNode(app, ts, node))
            return false;
    }
    return true;
}

/* The files in the cache: the nodes after its head. */
u32 countFiles(App* app, u32 cache)
{
    const u32 list = at(app, cache + kCacheList);
    u32 count = 0;
    if (list != 0)
        for (u32 node = at(app, list + kNodeNext); node != 0; node = at(app, node + kNodeNext))
            ++count;
    return count;
}

}  // namespace

/* The node of the file named `name` in the cache, or 0 -- for no cache or no list too. */
std::optional<u32> findCached(App* app, ThreadState& ts, u32 cache, u32 name)
{
    if (cache == 0)
        return 0;
    const u32 list = at(app, cache + kCacheList);
    if (list == 0)
        return 0;
    return findNode(app, ts, name, list);
}

namespace
{

/* What addFile leaves: whether the cache had its list and its last node, the file's copy (0
 * when no block was had), the node made for it (0 for none) and the last node it went after. */
struct AddedFile
{
    bool listed = false;
    u32 copy = 0, node = 0, after = 0;
};

/* The file named `name`, `size` bytes at `data`, copied into a block of its own ("mpFileData")
 * and added at the end of the cache, its last node then. */
std::optional<AddedFile> addFile(App* app, ThreadState& ts, u32 cache, u32 name, u32 size, u32 data)
{
    AddedFile added;
    if (cache == 0 || at(app, cache + kCacheList) == 0 || at(app, cache + kCacheLast) == 0)
        return added;
    added.listed = true;
    const std::optional<u32> copy = allocate(app, ts, kDataName, size, kBlockFlags);
    if (!copy)
        return std::nullopt;
    added.copy = *copy;
    if (*copy == 0)
        return added;
    copyBytes(app, *copy, data, size);
    added.after = at(app, cache + kCacheLast);
    const std::optional<u32> node = insertNode(app, ts, name, size, *copy, added.after);
    if (!node)
        return std::nullopt;
    added.node = *node;
    if (*node != 0)
        put(app, cache + kCacheLast, *node);
    return added;
}

/* A file cache ("BFL"): 'FGIB', its head (an empty node) its list's and its last; 0 if either
 * cannot be had. */
std::optional<u32> newCache(App* app, ThreadState& ts)
{
    const std::optional<u32> cache = allocate(app, ts, kCacheName, kCacheBytes, kBlockFlags);
    if (!cache || *cache == 0)
        return cache;
    put(app, *cache, kBigMagic);
    put(app, *cache + kCacheList, 0);
    put(app, *cache + kCacheLast, 0);
    const std::optional<u32> head = newEmptyNode(app, ts);
    if (!head)
        return std::nullopt;
    if (*head != 0)
    {
        put(app, *cache + kCacheLast, *head);
        put(app, *cache + kCacheList, *head);
        return cache;
    }
    if (!release(app, ts, *cache))
        return std::nullopt;
    return 0;
}

/* The cache freed, its nodes first and it cleared then, if it has a list.  False when the game
 * stopped. */
bool freeCache(App* app, ThreadState& ts, u32 cache)
{
    if (cache == 0)
        return true;
    const u32 list = at(app, cache + kCacheList);
    if (list != 0)
    {
        if (!freeNodes(app, ts, list))
            return false;
        put(app, cache + kCacheList, 0);
        put(app, cache, 0);
        put(app, cache + kCacheLast, 0);
    }
    return release(app, ts, cache);
}

/* The BIGF header the cache's files take: 16 bytes and, for each, 8 and its name with its
 * ending.  0 for no cache or list, not a cache's magic, a file of no size or no files. */
u32 headerSize(App* app, u32 cache)
{
    if (cache == 0)
        return 0;
    const u32 list = at(app, cache + kCacheList);
    if (list == 0 || at(app, cache) != kBigMagic)
        return 0;
    u32 size = kBigHeaderBytes;
    for (u32 node = at(app, list + kNodeNext); node != 0; node = at(app, node + kNodeNext))
    {
        if (at(app, node + kNodeSize) == 0)
            return 0;
        size += nameLength(app, node) + kEntryName + 1;
    }
    return size <= kBigHeaderBytes ? 0 : size;
}

/* The bytes of the cache's files, all told; 0 if one has none.  The cache and its list are
 * taken to be there. */
u32 dataSize(App* app, u32 cache)
{
    u32 total = 0;
    for (u32 node = at(app, at(app, cache + kCacheList) + kNodeNext); node != 0; node = at(app, node + kNodeNext))
    {
        const u32 size = at(app, node + kNodeSize);
        if (size == 0)
            return 0;
        total += size;
    }
    return total;
}

/* What writeBig leaves: 1 written, or 0; and where it stopped, as its callers' registers keep it:
 * the header's next entry (the size handed, if it never began) and the next file (0, or the
 * files' count when they did not fit). */
struct BigWritten
{
    u32 written, entry, file;
};

/* The cache written as a BIGF archive into the `size` bytes at `buffer` (cleared first): each
 * file's offset, size and name in the header and its data after it, as far as they fit, then
 * the header's four numbers.  Written 0 for no buffer, cache, size or list, no files, a header
 * too small, all of it not fitting, or a file of no size or data. */
std::optional<BigWritten> writeBig(App* app, ThreadState& ts, u32 buffer, u32 cache, u32 size)
{
    const u32 end = buffer + size;
    if (cache == 0 || buffer == 0 || size == 0 || at(app, cache + kCacheList) == 0)
        return BigWritten{0, size, 0};
    const u32 files = countFiles(app, cache);
    const u32 header = headerSize(app, cache);
    const u32 data = dataSize(app, cache);
    if (!(s32(files) > 0 && header >= kBigHeaderBytes && data + header <= size))
        return BigWritten{0, size, files};
    if (!fill(app, ts, buffer, 0, size))
        return std::nullopt;
    u32 entry = buffer + kBigHeaderBytes;
    const u32 headerEnd = buffer + header;
    u32 cursor = headerEnd;
    u32 count = 0;
    u32 file = at(app, at(app, cache + kCacheList) + kNodeNext);
    for (; file != 0; file = at(app, file + kNodeNext))
    {
        const u32 fileSize = at(app, file + kNodeSize);
        if (cursor + fileSize > end || entry >= headerEnd)
            break;
        if (fileSize == 0 || at(app, file + kNodeData) == 0)
            return BigWritten{0, entry, file};
        const u32 nameAt = entry + kEntryName;
        const u32 length = nameLength(app, file);
        copyName(app, nameAt, file);
        put(app, entry, turned(cursor - buffer));
        put(app, entry + kEntrySize, turned(at(app, file + kNodeSize)));
        copyBytes(app, cursor, at(app, file + kNodeData), at(app, file + kNodeSize));
        ++count;
        entry = nameAt + length + 1;
        cursor += at(app, file + kNodeSize);
    }
    put(app, buffer, turned(kBigMagic));
    put(app, buffer + 4, turned(size));
    put(app, buffer + 8, turned(count));
    put(app, buffer + 0xc, turned(header));
    return BigWritten{1, entry, file};
}

}  // namespace

/* The cache written as a BIGF archive into a block of its own ("BigBuffer"): it and its size to
 * [sizeTo]; else 0 and 0 to [sizeTo] (the block freed). */
std::optional<Packed> packCache(App* app, ThreadState& ts, u32 cache, u32 sizeTo)
{
    if (cache == 0 || at(app, cache + kCacheList) == 0)
    {
        put(app, sizeTo, 0);
        return Packed{0, sizeTo};
    }
    const u32 header = headerSize(app, cache);
    const u32 data = dataSize(app, cache);
    if (header == 0 || data == 0)
    {
        put(app, sizeTo, 0);
        return Packed{0, header};
    }
    const u32 size = header + data;
    const std::optional<u32> buffer = allocate(app, ts, kBufferName, size, kBlockFlags);
    if (!buffer)
        return std::nullopt;
    if (*buffer == 0)
    {
        put(app, sizeTo, 0);
        return Packed{0, size};
    }
    if (at(app, cache) != kBigMagic)
    {
        put(app, sizeTo, 0);
        if (!release(app, ts, *buffer))
            return std::nullopt;
        return Packed{0, size};
    }
    const std::optional<BigWritten> written = writeBig(app, ts, *buffer, cache, size);
    if (!written)
        return std::nullopt;
    if (written->written != 0)
    {
        put(app, sizeTo, size);
        return Packed{*buffer, written->file};
    }
    put(app, sizeTo, 0);
    if (!release(app, ts, *buffer))
        return std::nullopt;
    return Packed{0, written->file};
}

namespace
{

/* What readBig leaves: 1 read, or 0; and what its code leaves in edx, which no caller reads. */
struct BigRead
{
    u32 read, left;
};

/* The files of the BIGF archive at `data` added to the cache, each found by its name (0x4ea4a0,
 * 0x4ea290); those of no size passed over.  Read 0 for no cache or archive, not a BIGF, no
 * files, or a file not found or added. */
std::optional<BigRead> readBig(App* app, ThreadState& ts, u32 cache, u32 data)
{
    const GuestFrame frame(ts, 0x14, 0x34);
    // Handed to EA's BIG functions: a file's offset in the archive, its size, its name.
    const u32 offset = frame - 0xc, size = frame - 0x10, name = frame - 0x34;
    put(app, offset, 0);
    put(app, size, 0);
    if (data == 0 || cache == 0)
        return BigRead{0, 0};
    if (turned(at(app, data)) != kBigMagic)
        return BigRead{0, 0};
    const std::optional<u32> files = feCore<0x4ea440, frontend::eab::bigFileCount>(app, ts, data);
    if (!files)
        return std::nullopt;
    if (*files == 0)
        return BigRead{0, 0};
    u32 left = 0;
    for (u32 file = 0; s32(file) < s32(*files); ++file)
    {
            if (!feCore<0x4ea4a0, frontend::eab::bigFileName>(app, ts, data, file, name))
            return std::nullopt;
        const std::optional<u32> found = guest::direct<0x4ea290, locatedEntry>(app, ts, data, name, file, offset, size);
        if (!found)
            return std::nullopt;
        if (*found == 0)
            return BigRead{0, name};
        left = at(app, size);
        if (left != 0)
        {
            const std::optional<AddedFile> added = addFile(app, ts, cache, name, left, data + at(app, offset));
            if (!added)
                return std::nullopt;
            // What addFile's code leaves in edx: the name, the size, or the node.
            left = !added->listed ? name : added->copy == 0 ? left : added->node;
            if (!added->listed || added->copy == 0 || added->node == 0)
                return BigRead{0, left};
        }
        putByte(app, name, 0);
        put(app, offset, 0);
        put(app, size, 0);
    }
    return BigRead{1, left};
}

}  // namespace

/* A cache made of the BIGF archive at `data`, or 0 -- for none, not a BIGF, or its files not
 * all read (the cache freed). */
std::optional<u32> openCache(App* app, ThreadState& ts, u32 data)
{
    const GuestFrame frame(ts, 0x14, 4);  // as deep as the code's: readBig's frame below it
    if (data == 0 || turned(at(app, data)) != kBigMagic)
        return 0;
    const std::optional<u32> cache = newCache(app, ts);
    if (!cache || *cache == 0)
        return cache;
    std::optional<BigRead> read;
    {
        const GuestFrame slot(ts, 0, 4);  // the call's return address
        read = readBig(app, ts, *cache, data);
    }
    if (!read)
        return std::nullopt;
    if (read->read != 0)
        return cache;
    if (!freeCache(app, ts, *cache))
        return std::nullopt;
    return 0;
}

/* 0x40e000: loadSpeeds; eax the second half's index, or 0. */
nfs::u32 loadOpponentSpeeds(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry entry(args);
    const std::optional<u32> half = loadSpeeds(app, ts);
    if (!half)
        return args.eax;
    args.eax = *half;
    args.ebx = entry.ebx;
    args.ecx = entry.ecx;
    args.edx = entry.edx;
    args.esi = entry.esi;
    ts.sp += 4;
    return args.eax;
}

/* 0x40f230: clearCameras; eax the last view's index times 3, as the code leaves it. */
nfs::u32 clearTrackCameras(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry entry(args);
    if (!clearCameras(app, ts))
        return args.eax;
    args.eax = (kCameraViewCount - 1) * 3;
    args.ebx = entry.ebx;
    args.ecx = entry.ecx;
    args.edx = entry.edx;
    args.esi = entry.esi;
    ts.sp += 4;
    return args.eax;
}

/* 0x40f280: loadCameras for the path eax; eax how many. */
nfs::u32 loadTrackCameras(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry entry(args);
    const std::optional<u32> loaded = loadCameras(app, ts, args.eax);
    if (!loaded)
        return args.eax;
    args.eax = *loaded;
    args.ebx = entry.ebx;
    args.ecx = entry.ecx;
    args.edx = entry.edx;
    args.esi = entry.esi;
    ts.sp += 4;
    return args.eax;
}

/* 0x418d00: swapInPlace of the dword at eax; eax it now. */
nfs::u32 swapBigEndian(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = swapInPlace(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

/* 0x418d40: newNode; eax the node. */
nfs::u32 newFileNode(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry entry(args);
    const std::optional<u32> node = newNode(app, ts);
    if (!node)
        return args.eax;
    args.eax = *node;
    args.ebx = entry.ebx;
    args.ecx = entry.ecx;
    args.edx = entry.edx;
    ts.sp += 4;
    return args.eax;
}

/* 0x418d70: freeNode of the node eax; eax as the last free leaves it. */
nfs::u32 freeFileNode(App* app, ThreadState& ts, nfs::Args& args)
{
    if (!freeNode(app, ts, args.eax))
        return args.eax;
    ts.sp += 4;
    return args.eax;
}

/* 0x418dd0: findNode of the name eax after the node edx; eax the node, edx as stricmp leaves it. */
nfs::u32 findFileNode(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry entry(args);
    const std::optional<u32> node = findNode(app, ts, args.eax, args.edx);
    if (!node)
        return args.eax;
    args.eax = *node;
    args.ebx = entry.ebx;
    args.ecx = entry.ecx;
    ts.sp += 4;
    return args.eax;
}

/* 0x418e90: insertNode of the name eax, edx bytes at ebx, after the node ecx; eax, ebx and edx
 * the node. */
nfs::u32 insertFileNode(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry entry(args);
    const std::optional<u32> node = insertNode(app, ts, args.eax, args.edx, args.ebx, args.ecx);
    if (!node)
        return args.eax;
    args.eax = *node;
    args.ebx = *node;
    args.ecx = entry.ecx;
    args.edx = *node;
    ts.sp += 4;
    return args.eax;
}

/* 0x418f00: newEmptyNode; eax the node. */
nfs::u32 newEmptyFileNode(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry entry(args);
    const std::optional<u32> node = newEmptyNode(app, ts);
    if (!node)
        return args.eax;
    args.eax = *node;
    args.ebx = entry.ebx;
    args.ecx = entry.ecx;
    args.edx = entry.edx;
    ts.sp += 4;
    return args.eax;
}

/* 0x418f40: freeNodes from the node eax; eax as the last free leaves it. */
nfs::u32 freeFileNodes(App* app, ThreadState& ts, nfs::Args& args)
{
    if (!freeNodes(app, ts, args.eax))
        return args.eax;
    ts.sp += 4;
    return args.eax;
}

/* 0x418f70: countFiles of the cache eax. */
nfs::u32 countFileNodes(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = countFiles(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

/* 0x418fe0: findCached of the name edx in the cache eax; edx, which no caller reads, as stricmp
 * leaves it. */
nfs::u32 findCachedFile(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry entry(args);
    const u32 cache = args.eax;
    const std::optional<u32> node = findCached(app, ts, cache, args.edx);
    if (!node)
        return args.eax;
    args.eax = *node;
    args.ebx = entry.ebx;
    args.ecx = entry.ecx;
    ts.sp += 4;
    return args.eax;
}

/* 0x419000: addFile of the name edx, ebx bytes at ecx, to the cache eax; eax 1 when it was
 * added, and ebx, ecx and edx as the code leaves them. */
nfs::u32 addCachedFile(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry entry(args);
    const u32 cache = args.eax;
    const std::optional<AddedFile> added = addFile(app, ts, cache, args.edx, args.ebx, args.ecx);
    if (!added)
        return args.eax;
    args.eax = 0;
    if (added->listed && added->copy == 0)
    {
        // edx as the allocation left it.
        args.ebx = 0;
        args.ecx = entry.ecx;
    }
    else if (added->listed)
    {
        args.eax = added->node != 0 ? 1 : 0;
        args.ebx = added->node;
        args.ecx = added->node != 0 ? cache : added->after;
        args.edx = added->node;
    }
    args.esi = entry.esi;
    args.edi = entry.edi;
    ts.sp += 4;
    return args.eax;
}

/* 0x419080: newCache; eax the cache. */
nfs::u32 newFileCache(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry entry(args);
    const std::optional<u32> cache = newCache(app, ts);
    if (!cache)
        return args.eax;
    args.eax = *cache;
    args.ebx = entry.ebx;
    args.ecx = entry.ecx;
    args.edx = entry.edx;
    ts.sp += 4;
    return args.eax;
}

/* 0x4190e0: freeCache of the cache eax; eax as the last free leaves it. */
nfs::u32 freeFileCache(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry entry(args);
    if (!freeCache(app, ts, args.eax))
        return args.eax;
    args.ecx = entry.ecx;
    args.edx = entry.edx;
    ts.sp += 4;
    return args.eax;
}

/* 0x419120: headerSize of the cache eax. */
nfs::u32 bigHeaderSize(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = headerSize(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

/* 0x4191b0: dataSize of the cache eax. */
nfs::u32 cachedDataSize(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = dataSize(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

/* 0x4191e0: writeBig of the cache edx into the ebx bytes at eax; eax 1 written, ebx and edx
 * where it stopped. */
nfs::u32 writeBigFile(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry entry(args);
    const std::optional<BigWritten> written = writeBig(app, ts, args.eax, args.edx, args.ebx);
    if (!written)
        return args.eax;
    args.eax = written->written;
    args.ebx = written->entry;
    args.ecx = entry.ecx;
    args.edx = written->file;
    args.esi = entry.esi;
    args.edi = entry.edi;
    ts.sp += 4;
    return args.eax;
}

/* 0x419460: packCache of the cache eax, its size to [edx]; eax the buffer, edx as the code
 * leaves it. */
nfs::u32 packFileCache(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry entry(args);
    const std::optional<Packed> packed = packCache(app, ts, args.eax, args.edx);
    if (!packed)
        return args.eax;
    args.eax = packed->buffer;
    args.ebx = entry.ebx;
    args.ecx = entry.ecx;
    args.edx = packed->left;
    args.esi = entry.esi;
    args.edi = entry.edi;
    ts.sp += 4;
    return args.eax;
}

/* 0x419510: readBig of the archive at edx into the cache eax; eax 1 read, edx as the code
 * leaves it. */
nfs::u32 readBigFile(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry entry(args);
    const std::optional<BigRead> read = readBig(app, ts, args.eax, args.edx);
    if (!read)
        return args.eax;
    args.eax = read->read;
    args.ebx = entry.ebx;
    args.ecx = entry.ecx;
    args.edx = read->left;
    args.esi = entry.esi;
    args.edi = entry.edi;
    ts.sp += 4;
    return args.eax;
}

/* 0x4195f0: openCache of the archive at eax; eax the cache. */
nfs::u32 openFileCache(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry entry(args);
    const std::optional<u32> cache = openCache(app, ts, args.eax);
    if (!cache)
        return args.eax;
    args.eax = *cache;
    args.ebx = entry.ebx;
    args.ecx = entry.ecx;
    args.edx = entry.edx;
    args.esi = entry.esi;
    args.edi = entry.edi;
    ts.sp += 4;
    return args.eax;
}


namespace
{

/* The track's textures loaded (0x4956f0) to [0x5dd850] unless the loading was stopped: its own
 * Trnn1.qfs (Trnn1_8.qfs for the options' smaller set) where the race has them (mirrored) and
 * the file is there -- traced, as bworld.c's line 1860 does (0x42d500) -- else Trnn0.qfs
 * (Trnn0_8.qfs).  False when the game stopped. */
bool loadTextures(App* app, ThreadState& ts)
{
    put(app, kTrackShapes, 0);
    if (!loadingTick(app, ts))
        return false;
    if (at(app, kLoadAborted) != 0)
        return true;
    u32 path = 0;
    if (gameSettings(app).mirrored != 0)
    {
        const std::optional<u32> own =
            trackFile(app, ts, (byte(app, kOptions) & kSmallTextures) != 0 ? kNear8 : kNear);
        if (!own)
            return false;
        const std::optional<bool> there = fileExists(app, ts, *own);
        if (!there)
            return false;
        if (*there)
        {
            // The trace's words pushed as the code pushes them (it reads none of them).
            const GuestFrame words(ts, 0, kTraceWords);
            u32 slot = words - kTraceWords;
            for (const u32 word : {kTraceFormat, kTraceFile, kTraceLine})
            {
                put(app, slot, word);
                slot += 4;
            }
            guest::direct<0x42d500, raceload::noHook>(app, ts);
            if (ts.terminate)
                return false;
            path = *own;
        }
    }
    if (path == 0)
    {
        const std::optional<u32> shared =
            trackFile(app, ts, (byte(app, kOptions) & kSmallTextures) != 0 ? kFar8 : kFar);
        if (!shared)
            return false;
        path = *shared;
    }
    const u32 shapes = guest::direct<0x4956f0, frontend::draw::loadQfsFlags>(app, ts, path, 0u);
    if (ts.terminate)
        return false;
    put(app, kTrackShapes, shapes);
    return true;
}

/* The ghost saved to ghost.gst (0x4777d0) when the race is no replay and was run.  False when
 * the game stopped. */
bool saveRacedGhost(App* app, ThreadState& ts)
{
    if (gameSettings(app).replayMode != 0 || gameSettings(app).lastTickRaced == 0)
        return true;
    return feCore<0x4777d0, frontend::loopa::saveGhost>(app, ts, kGhostName).has_value();
}

}  // namespace

/* The ghost written to the stream at `offset` (fseek from its start, then fwrite: its record
 * and its records, 0x20f4 + 18 bytes each), if there is a stream.  False when the game stopped. */
bool writeGhostTo(App* app, ThreadState& ts, u32 stream, u32 offset)
{
    if (stream == 0)
        return true;
    guest::direct<0x4e9088, watcom::clib::fseek>(app, ts, stream, offset, 0);
    if (ts.terminate)
        return false;
    guest::direct<0x4eebd0, watcom::clib::fwrite>(app, ts, kGhost, at(app, kGhostCount) * kGhostRecordBytes + kGhostBytes, 1u,
                                                  stream);
    return !ts.terminate;
}

/* The ghost read from the game's file handle (0x4e2de0), if there is one: its record, then as
 * many records as it says; [0x55d000] 1.  False when the game stopped. */
bool readGhostFrom(App* app, ThreadState& ts, u32 handle)
{
    if (handle == 0)
        return true;
    if (!guest::direct<0x4e2de0, ea::readChecked>(app, ts, handle, kGhost, kGhostBytes))
        return false;
    if (!guest::direct<0x4e2de0, ea::readChecked>(app, ts, handle, kGhostRecords, at(app, kGhostCount) * kGhostRecordBytes))
        return false;
    put(app, kGhostRead, 1);
    return true;
}

/* 0x41d8a0: trackFile of the end eax; eax the path. */
nfs::u32 trackFilePath(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry entry(args);
    const std::optional<u32> path = trackFile(app, ts, args.eax);
    if (!path)
        return args.eax;
    args.eax = *path;
    args.edx = entry.edx;
    ts.sp += 4;
    return args.eax;
}

/* 0x41d8d0: trackFolderFile of the name eax; eax the path. */
nfs::u32 trackFolderPath(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry entry(args);
    const std::optional<u32> path = trackFolderFile(app, ts, args.eax);
    if (!path)
        return args.eax;
    args.eax = *path;
    args.edx = entry.edx;
    ts.sp += 4;
    return args.eax;
}

/* 0x41dbc0: loadTextures; eax as the last call leaves it (the shapes, or the loading bar's). */
nfs::u32 loadTrackTextures(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry entry(args);
    if (!loadTextures(app, ts))
        return args.eax;
    args.ecx = entry.ecx;
    args.edx = entry.edx;
    ts.sp += 4;
    return args.eax;
}

/* 0x477a90: saveRacedGhost; the registers as the saving leaves them. */
nfs::u32 saveGhost(App* app, ThreadState& ts, nfs::Args& args)
{
    if (!saveRacedGhost(app, ts))
        return args.eax;
    ts.sp += 4;
    return args.eax;
}

/* 0x477ac0: writeGhostTo the stream eax at edx; eax and edx as fwrite leaves them. */
nfs::u32 writeGhost(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry entry(args);
    if (!writeGhostTo(app, ts, args.eax, args.edx))
        return args.eax;
    args.ebx = entry.ebx;
    args.ecx = entry.ecx;
    ts.sp += 4;
    return args.eax;
}

/* 0x477b00: readGhostFrom the handle eax; eax as the reading leaves it. */
nfs::u32 readGhost(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry entry(args);
    if (!readGhostFrom(app, ts, args.eax))
        return args.eax;
    args.ebx = entry.ebx;
    args.ecx = entry.ecx;
    args.edx = entry.edx;
    ts.sp += 4;
    return args.eax;
}

namespace
{

// The sky's, as the horizon fills them: its colour (three bytes and a dword), the fog's
// distance (a hundredth), the records' count and block (36 bytes each).
constexpr u32 kSkyColour = 0x55d26c, kSkyColourWord = 0x55d270, kFogScale = 0x55d268;
constexpr u32 kRecordCount = 0x55d274, kRecords = 0x55d278, kRecordBytes = 36;
constexpr s32 kFogNearest = 1, kFogFarthest = 100;
// The sun's and the clouds': four numbers, the sun's size (and its reciprocal), two floats, the
// sun's height (chopped), its place (cosine and sine times its distance), the clouds' (two
// numbers and a float), the clouds' turn (16.16 turns, and a float within 0 and 1), a number and
// five floats.
constexpr u32 kSunNumbers[] = {0x55d1f4, 0x55d204, 0x55d260, 0x55d264};
constexpr u32 kSunSize = 0x55d208, kSunFloat = 0x55d248, kSunTripled = 0x55d24c;
constexpr u32 kSunNumber = 0x55d1f8, kSunOther = 0x55d200, kSunHeight = 0x55d1fc;
constexpr u32 kSunAcross = 0x55d20c, kSunAlong = 0x55d210;
// [0x55d158]: no fog colour when not 0.
constexpr u32 kNoFog = 0x55d158, kCloudOther = 0x55d15c, kCloudFloat = 0x55d1c8;
constexpr u32 kCloudTurn = 0x55d1f0, kCloudTurnFloat = 0x55d1cc;
constexpr s32 kDegreesTurn = 360;
constexpr u32 kCloudLast = 0x55d160;
constexpr u32 kCloudFloats[] = {0x55d164, 0x55d168, 0x55d16c, 0x55d170, 0x55d174};
// The colours read as bytes: the fog's, the top's and the bottom's (blue, green, red).
constexpr u32 kColourBytes[] = {0x55d1c2, 0x55d1c1, 0x55d1c0, 0x55d1c6, 0x55d1c5, 0x55d1c4,
                                  0x55d17a, 0x55d179, 0x55d178};
// The weather's: three halves, four running counts, three dwords after.
constexpr u32 kWeatherHalves[] = {0x8ca1ea, 0x8ca1ec, 0x8ca1ee};
constexpr u32 kWeatherCounts = 0x8ca1f0;
constexpr u32 kWeatherWords[] = {0x8ca1f0, 0x8ca1f4, 0x8ca1f8, 0x8ca1fc, 0x55d148, 0x55d14c};
constexpr u32 kWeatherLast = 0x55d150;
// The sun's direction (three floats, the middle one a half) and the 16.16 angle's scale.
constexpr u32 kSunDirection = 0x55d2a8;
constexpr u32 kHalfBits = 0x3f000000;
// The sky's bands: 8, from the top's colour to the bottom's, each with its mirror (16 less it),
// four bytes each from 0x55d178; the band at 0x55d17c copied to 0x55d1bc; the fog's colour.
constexpr u32 kBandCount = 8, kBands = 0x55d178, kBandMirror = 16, kBandCopy = 0x55d1bc;
constexpr u32 kFogColour = 0x6ff9f8;

void putFloat(App* app, u32 address, float value)
{
    app->getMemory<float>(address) = value;
}

float floatAt(App* app, u32 address)
{
    return app->getMemory<float>(address);
}

u32 bitsOf(float value)
{
    u32 bits;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}

/* The horizon's next number, its text's place at `text` (moved on past it). */
std::optional<u32> readNumber(App* app, ThreadState& ts, u32 text)
{
    return feCore<0x4be020, frontend::loopb::readNumber>(app, ts, text);
}

/* __CHP: `value` to a whole value, towards 0; never nullopt (the optional is the caller's stop
 * signal). */
std::optional<nfs::Float> chop(App* app, ThreadState& ts, nfs::Float value)
{
    return nfs::Float(guest::direct<kChop, watcom::clib::chop>(app, ts, double(value)));
}


/* A 16.16 colour channel's whole part, towards 0 (sar, sbb, sar). */
u8 channel(u32 value)
{
    return u8(s32(value) / 0x10000);
}

/* The sky's horizon record `index`'s place: [0x55d278] read again each time, as the code does. */
u32 recordAt(App* app, u32 index)
{
    return at(app, kRecords) + index * kRecordBytes;
}

/* The track's sky, sky.fsh in its folder: its clouds' shape (palette to 0x55d21c), the sun's (to
 * 0x6ff9a0, 4 of its colours with the options' smaller textures, else 7) and the sun's glow
 * placed (0x47a970) -- CLD/CHR/SUN with N at night, W in the weather, D by day -- [0x563a90] the
 * sun's palette, the file freed and the sky made (0x47ae10).  False when the game stopped. */
bool loadSkyShapes(App* app, ThreadState& ts, u32 track)
{
    const GuestFrame frame(ts, 0x18, 0x100);
    const u32 name = frame - 0x100;  // handed to sprintf and the loading
    if (!format(app, ts, {name, kSkyName, kDataPath, track}))
        return false;
    const std::optional<u32> file = loadFile(app, ts, name, 0);
    if (!file)
        return false;
    if (*file == 0)
        return true;
    if (!loadingTick(app, ts))
        return false;
    const u32 sunColours = (byte(app, kOptions) & kSmallTextures) != 0 ? 4 : 7;
    // The shapes' names: clouds, sun, glow.
    constexpr u32 names[4][3] = {{0x53b354, 0x53b35c, 0x53b364},   // night, weather
                                   {0x53b36c, 0x53b374, 0x53b37c},   // night
                                   {0x53b384, 0x53b38c, 0x53b394},   // weather
                                   {0x53b39c, 0x53b3a4, 0x53b3ac}};  // day
    const u32* shapes = names[(gameSettings(app).night != 0 ? 0 : 2) + (gameSettings(app).weather != 0 ? 0 : 1)];
    if (!feCore<0x4d4080, frontend::rende::loadShapeTexture>(app, ts, *file, shapes[0], 0x40u, kCloudPalette, 7u))
        return false;
    if (!feCore<0x4d4080, frontend::rende::loadShapeTexture>(app, ts, *file, shapes[1], 0x80u, kSunPalette, sunColours))
        return false;
    if (!feCore<0x47a970, frontend::wata::skyLoad>(app, ts, *file, shapes[2]))
        return false;
    put(app, kSunShapes, kSunPalette);
    if (!loadingTick(app, ts) || !release(app, ts, *file))
        return false;
    return feCore<0x47ae10, frontend::renda::cloudGrid>(app, ts) && loadingTick(app, ts);
}

/* The track's horizon, 3Trnn.hrz (with N at night, W in the weather) in its folder, read number
 * by number (0x4be020) into the sky's: its colours, the fog's (1 to 100, a hundredth), its
 * records (36 bytes each, allocated to [0x55d278]), the sun's and the clouds' places and sizes,
 * the weather's counts (0x8ca1e8 on); the file freed, the sky loaded (loadSkyShapes), the sun's
 * direction worked out and its length taken, and the sky's 8 bands of colours, top and bottom,
 * from its two colours; the fog's colour from the bottom one, or none.  What the code leaves in
 * eax: the file's loading's 0, or the last tick's with the fog colour's last byte in it. */
std::optional<u32> readHorizon(App* app, ThreadState& ts, u32 track)
{
    const GuestFrame frame(ts, 0x18, 0x144);
    const u32 name = frame - 0x144, text = frame - 0x34;  // handed to sprintf, the loading, the reading
    put(app, text, 0);
    const bool night = gameSettings(app).night != 0, weather = gameSettings(app).weather != 0;
    const u32 nameFormat = night ? (weather ? kHorizonNightWeather : kHorizonNight)
                                   : (weather ? kHorizonWeather : kHorizonName);
    if (!format(app, ts, {name, nameFormat, kDataPath, track, track}) || !loadingTick(app, ts))
        return std::nullopt;
    const std::optional<u32> file = loadFile(app, ts, name, 0x10);
    if (!file)
        return std::nullopt;
    if (*file == 0)
        return 0;
    put(app, text, *file);
    std::optional<u32> number;
    const auto next = [&] { return (number = readNumber(app, ts, text)).has_value(); };

    // The sky's colour, its three bytes, and its fourth's dword.
    for (const u32 to : {kSkyColour + 2, kSkyColour + 1, kSkyColour})
    {
        if (!next())
            return std::nullopt;
        putByte(app, to, u8(*number));
    }
    if (!next())
        return std::nullopt;
    put(app, kSkyColourWord, *number);
    // The fog: 1 to 100, a hundredth of it.
    if (!next())
        return std::nullopt;
    const s32 fog = s32(*number) > kFogFarthest ? kFogFarthest
                       : s32(*number) < kFogNearest ? kFogNearest
                                                         : s32(*number);
    putFloat(app, kFogScale, float(fp::mul(nfs::Float(fog), nfs::Float(floatAt(app, kHundredth)))));
    // The records.
    if (!next())
        return std::nullopt;
    put(app, kRecordCount, *number);
    if (*number == 0)
        put(app, kRecords, 0);
    else
    {
        const std::optional<u32> records = allocate(app, ts, kNoName, *number * kRecordBytes, 0);
        if (!records)
            return std::nullopt;
        put(app, kRecords, *records);
        for (u32 index = 0; s32(index) < s32(at(app, kRecordCount)); ++index)
        {
            // Its bytes and dwords, in the file's order.
            constexpr struct { u32 offset; bool dword; } fields[] = {
                {2, false}, {1, false}, {0, false}, {0xc, true}, {6, false}, {5, false}, {4, false},
                {0x10, true}, {0xa, false}, {9, false}, {8, false}, {0x14, true}, {0x18, true},
                {0x1c, true}, {0x20, true}};
            for (const auto& field : fields)
            {
                if (!next())
                    return std::nullopt;
                if (field.dword)
                    put(app, recordAt(app, index) + field.offset, *number);
                else
                    putByte(app, recordAt(app, index) + field.offset, u8(*number));
            }
        }
    }
    // The sun's and the clouds'.
    for (const u32 to : kSunNumbers)
    {
        if (!next())
            return std::nullopt;
        put(app, to, *number);
    }
    if (!next())
        return std::nullopt;
    const float sunSize = float(nfs::Float(s32(*number)));
    putFloat(app, kSunSize, sunSize);
    if (sunSize != 0.0f)  // its bits but the sign's: a whole number's float, never a NaN
        putFloat(app, kSunSize, float(fp::div(nfs::Float(1.0), nfs::Float(sunSize))));
    if (!next())
        return std::nullopt;
    putFloat(app, kSunFloat, float(nfs::Float(s32(*number))));
    if (!next())
        return std::nullopt;
    putFloat(app, kSunTripled, float(nfs::Float(s32(*number * 3))));
    if (!next())
        return std::nullopt;
    put(app, kSunNumber, *number);
    if (!next())
        return std::nullopt;
    put(app, kSunOther, *number);
    if (!next())
        return std::nullopt;
    const std::optional<nfs::Float> height =
        chop(app, ts, fp::mul(nfs::Float(s32(*number)), nfs::Float(app->getMemory<double>(kHeightScale))));
    if (!height)
        return std::nullopt;
    put(app, kSunHeight, u32(fp::toInteger<s32>(*height)));
    // The sun's angle and distance: its place on the sky.
    if (!next())
        return std::nullopt;
    const u32 distanceNumber = *number;
    if (!next())
        return std::nullopt;
    const float angle =
        float(fp::mul(nfs::Float(s32(*number)), nfs::Float(app->getMemory<double>(kSunAngleScale))));
    const float distance =
        float(fp::mul(nfs::Float(s32(distanceNumber)), nfs::Float(app->getMemory<double>(kSunDistanceScale))));
    const std::optional<double> cosine = feCore<0x4ea950, frontend::simd::cosineValue>(app, ts, bitsOf(angle));
    if (!cosine)
        return std::nullopt;
    putFloat(app, kSunAcross, float(fp::mul(*cosine, nfs::Float(distance))));
    const std::optional<double> sine = feCore<0x4ea930, frontend::simd::sineValue>(app, ts, bitsOf(angle));
    if (!sine)
        return std::nullopt;
    putFloat(app, kSunAlong, float(fp::mul(*sine, nfs::Float(distance))));
    if (!next())
        return std::nullopt;
    put(app, kNoFog, *number);
    if (!next())
        return std::nullopt;
    put(app, kCloudOther, *number);
    if (!next())
        return std::nullopt;
    putFloat(app, kCloudFloat, float(nfs::Float(s32(*number))));
    // The clouds' turn: degrees made 16.16 turns, then within 0 and 1.
    if (!next())
        return std::nullopt;
    const std::int64_t turnDividend = s32(*number << 16);
    const u32 cloudTurn = u32(turnDividend / kDegreesTurn);
    put(app, kCloudTurn, cloudTurn);
    float turn = float(fp::mul(nfs::Float(s32(cloudTurn)), nfs::Float(floatAt(app, kTurnScale))));
    // fcom with 0, jbe: on while 0 is above it, not for a NaN -- a whole number times a constant
    // and 1 added, never one; then its bits over 1.0's: a float over 1.0, never infinite.
    for (;;)
    {
        putFloat(app, kCloudTurnFloat, turn);
        if (!(turn < 0.0f))
            break;
        turn = float(fp::add(nfs::Float(1.0), nfs::Float(turn)));
    }
    while (turn > 1.0f)
    {
        turn = float(fp::add(nfs::Float(turn), nfs::Float(floatAt(app, kMinusOne))));
        putFloat(app, kCloudTurnFloat, turn);
    }
    if (!next())
        return std::nullopt;
    put(app, kCloudLast, *number);
    for (const u32 to : kCloudFloats)
    {
        if (!next())
            return std::nullopt;
        putFloat(app, to, float(nfs::Float(s32(*number))));
    }
    // The sky's colours: the fog's, the top's, the bottom's.
    for (const u32 to : kColourBytes)
    {
        if (!next())
            return std::nullopt;
        putByte(app, to, u8(*number));
    }
    // The bands' two colours, top and bottom: red, green, blue.
    u8 top[3], bottom[3];
    for (u8* channelTo : {&top[0], &top[1], &top[2], &bottom[0], &bottom[1], &bottom[2]})
    {
        if (!next())
            return std::nullopt;
        *channelTo = u8(*number);
    }
    // The weather's.
    if (!next())
        return std::nullopt;
    putByte(app, kWeatherCount, u8(*number));
    for (const u32 to : kWeatherHalves)
    {
        if (!next())
            return std::nullopt;
        app->getMemory<u16>(to) = u16(*number);
    }
    for (const u32 to : kWeatherWords)
    {
        if (!next())
            return std::nullopt;
        put(app, to, *number);
    }
    if (!next())
        return std::nullopt;
    // The four counts made running sums.
    const u32 second = at(app, kWeatherCounts + 4);
    put(app, kWeatherLast, *number);
    const u32 upToSecond = second + at(app, kWeatherCounts);
    put(app, kWeatherCounts + 4, upToSecond);
    const u32 upToThird = at(app, kWeatherCounts + 8) + upToSecond;
    put(app, kWeatherCounts + 8, upToThird);
    put(app, kWeatherCounts + 0xc, at(app, kWeatherCounts + 0xc) + upToThird);
    if (!release(app, ts, *file))
        return std::nullopt;
    {
        const GuestFrame slot(ts, 0, 4);  // the call's return address
        if (!loadSkyShapes(app, ts, track))
            return std::nullopt;
    }
    if (!loadingTick(app, ts))
        return std::nullopt;
    // The sun's direction, 16.16 sine and cosine of the clouds' turn.
    const u32 fixedSine = guest::direct<0x4f2380, sineOf>(app, ts, at(app, kCloudTurn));
    if (ts.terminate)
        return std::nullopt;
    const float across =
        float(fp::mul(nfs::Float(s32(fixedSine)), nfs::Float(app->getMemory<double>(kFixedScale))));
    put(app, kSunDirection + 4, kHalfBits);
    putFloat(app, kSunDirection, across);
    const u32 fixedCosine = guest::direct<0x4f23f0, cosineOf>(app, ts, at(app, kCloudTurn));
    if (ts.terminate)
        return std::nullopt;
    putFloat(app, kSunDirection + 8,
             float(fp::mul(nfs::Float(s32(fixedCosine)), nfs::Float(app->getMemory<double>(kFixedScale)))));
    // Its length, dropped.
    guest::direct<0x4972f0, vertices::normaliseVector>(app, ts, kSunDirection);
    if (ts.terminate)
        return std::nullopt;
    if (!loadingTick(app, ts))
        return std::nullopt;
    // The sky's 8 bands: from the top's colour to the bottom's in steps of an eighth, 16.16, the
    // band and its mirror (16 less it) each.
    u32 colour[3], step[3];
    for (int i = 0; i < 3; ++i)
    {
        step[i] = (u32(bottom[i]) - u32(top[i])) << 13;
        colour[i] = u32(top[i]) << 16;
    }
    for (u32 band = 0; band < kBandCount; ++band)
    {
        const u8 red = channel(colour[0]), green = channel(colour[1]), blue = channel(colour[2]);
        putByte(app, kBands + band * 4 + 6, red);
        putByte(app, kBands + band * 4 + 5, green);
        putByte(app, kBands + band * 4 + 4, blue);
        const u32 mirror = kBandMirror - band;
        putByte(app, kBands + mirror * 4 + 2, red);
        putByte(app, kBands + mirror * 4 + 1, green);
        putByte(app, kBands + mirror * 4, blue);
        for (int i = 0; i < 3; ++i)
            colour[i] += step[i];
    }
    put(app, kBandCopy, at(app, kBands + 4));
    const std::optional<u32> ticked = loadingStop(app, ts);
    if (!ticked)
        return std::nullopt;
    // The fog's colour: none, or the bottom's.
    if (at(app, kNoFog) != 0)
    {
        putByte(app, kFogColour + 2, 0);
        putByte(app, kFogColour + 1, 0);
        putByte(app, kFogColour, 0);
        putByte(app, kFogColour + 3, 0);
        return *ticked & 0xffff00ff;
    }
    putByte(app, kFogColour + 2, byte(app, kBands + 2));
    putByte(app, kFogColour + 1, byte(app, kBands + 1));
    putByte(app, kFogColour + 3, 0xff);
    const u8 lastRed = byte(app, kBands);
    putByte(app, kFogColour, lastRed);
    return (*ticked & 0xffffff00) | lastRed;
}

/* The weather's shapes, weather.fsh: as many raindrops' (DRPn, to 0x8ca200) and splashes'
 * (SPLn, to 0x8ca288) as the horizon's count ([0x8ca1e8]) says, by 0x4d4080 -- none for 0; the
 * file freed.  False when the game stopped. */
bool loadWeather(App* app, ThreadState& ts)
{
    const GuestFrame frame(ts, 0x18, 0x144);
    const u32 name = frame - 0x144, shape = frame - 0x44;  // handed to sprintf, the loading, the shapes
    // The file's block, kept in the frame right after the shape's name and read back from there:
    // a name of 0x40 bytes or more runs into it.
    const u32 kept = frame - 4;
    if (!format(app, ts, {name, kWeatherName, kWeatherPath}))
        return false;
    const std::optional<u32> file = loadFile(app, ts, name, 0);
    if (!file)
        return false;
    put(app, kept, *file);
    const u32 count = byte(app, kWeatherCount);
    if (count != 0)
        for (const auto& kind : {std::pair<u32, u32>{kDropName, kDropShapes}, {kSplashName, kSplashShapes}})
        {
            if (!format(app, ts, {shape, kind.first, count}))
                return false;
                    if (!feCore<0x4d4080, frontend::rende::loadShapeTexture>(app, ts, *file, shape, 0x20u, kind.second, 0u))
                return false;
        }
    const u32 block = at(app, kept);
    return block == 0 || release(app, ts, block);
}

}  // namespace

/* 0x478340: readHorizon for the track eax; eax as its code leaves it. */
nfs::u32 loadHorizon(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry entry(args);
    const std::optional<u32> left = readHorizon(app, ts, args.eax);
    if (!left)
        return args.eax;
    args.eax = *left;
    args.ebx = entry.ebx;
    args.ecx = entry.ecx;
    args.edx = entry.edx;
    args.esi = entry.esi;
    args.edi = entry.edi;
    ts.sp += 4;
    return args.eax;
}

/* 0x47ac30: loadSkyShapes for the track eax; eax as the last call leaves it. */
nfs::u32 loadSky(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry entry(args);
    if (!loadSkyShapes(app, ts, args.eax))
        return args.eax;
    args.ebx = entry.ebx;
    args.ecx = entry.ecx;
    args.edx = entry.edx;
    args.esi = entry.esi;
    args.edi = entry.edi;
    ts.sp += 4;
    return args.eax;
}

/* 0x4dcd20: loadWeather; eax as the last call leaves it.  The registers the code pushes are
 * kept on the guest's stack where it keeps them, and taken back from there: a shape's name of
 * 0x40 bytes or more runs over the file's block into them. */
nfs::u32 loadWeatherShapes(App* app, ThreadState& ts, nfs::Args& args)
{
    const u32 pushed = ts.sp;
    put(app, pushed - 4, args.ebx);
    put(app, pushed - 8, args.ecx);
    put(app, pushed - 0xc, args.edx);
    put(app, pushed - 0x10, args.esi);
    put(app, pushed - 0x14, args.edi);
    put(app, pushed - 0x18, args.ebp);
    if (!loadWeather(app, ts))
        return args.eax;
    args.ebx = at(app, pushed - 4);
    args.ecx = at(app, pushed - 8);
    args.edx = at(app, pushed - 0xc);
    args.esi = at(app, pushed - 0x10);
    args.edi = at(app, pushed - 0x14);
    args.ebp = at(app, pushed - 0x18);
    ts.sp += 4;
    return args.eax;
}

/* The typed cores of the thunks above, for callers that call them as C++ (native_load_track.h). */
bool trackTexturesLoaded(App* app, ThreadState& ts)
{
    return loadTextures(app, ts);
}

bool ghostSaved(App* app, ThreadState& ts)
{
    return saveRacedGhost(app, ts);
}

std::optional<u32> horizonRead(App* app, ThreadState& ts, u32 track)
{
    return readHorizon(app, ts, track);
}

bool weatherLoaded(App* app, ThreadState& ts)
{
    return loadWeather(app, ts);
}

}  // namespace load
}  // namespace nfs3hp
