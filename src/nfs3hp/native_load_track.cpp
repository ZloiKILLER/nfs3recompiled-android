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
using x86::reg16;
using x86::reg32;
using x86::reg8;
using x86::sreg32;
using App = win32::WinApplication;
using CPU = x86::CPU;

// The file cache's: its magic, its blocks' names.
constexpr reg32 kBigMagic = 0x42494746;  // 'FGIB': "BIGF" read as a dword
constexpr reg32 kNodeName = 0x536c38, kDataName = 0x536c44, kCacheName = 0x536c50, kBufferName = 0x536c54;
// A cache's: its list's head, its last node.  A node's: its name (at +0), its file's size and
// data, the next node and the one before; a node's bytes.
constexpr reg32 kCacheList = 4, kCacheLast = 8, kCacheBytes = 0xc;
constexpr reg32 kNodeSize = 0x20, kNodeData = 0x24, kNodeNext = 0x28, kNodePrevious = 0x2c, kNodeBytes = 0x30;
// A BIGF header's: its four numbers' bytes; an entry's offset, size and name.
constexpr reg32 kBigHeaderBytes = 0x10, kEntrySize = 4, kEntryName = 8;
// The game's allocator's flags for the cache's blocks.
constexpr reg32 kBlockFlags = 0x10;
// The track's: the data path, the track, the paths' buffers and formats.
constexpr reg32 kDataPath = 0x7a27b4;
constexpr reg32 kTrackFile = 0x5dd86c, kTrackFileFormat = 0x536cec;
constexpr reg32 kFolderFile = 0x5dd8ec, kFolderFileFormat = 0x536d00;
// The race: backwards, the track's own textures, the night, the weather; the
// options' byte (2: the textures' smaller set); the loading stopped.
constexpr reg32 kOptions = 0x7a3a58, kLoadAborted = 0x55e88c;
constexpr reg8 kSmallTextures = 2;
// The opponents' speeds: speedsR.bin, speedsF.bin; the table, its second
// half, the failure's flag; the track's nodes.
constexpr reg32 kSpeedsBackward = 0x53679c, kSpeedsForward = 0x5367a8;
constexpr reg32 kSpeeds = 0x552558, kSpeedsHalf = 0x55255c, kSpeedsFailed = 0x56f22c, kTrackNodes = 0x5dd940;
constexpr reg32 kSpeedsLoadFlags = 0x10;
// The cameras: the views (24 bytes each, 256), the .can files loaded (slots 1 to 7), the names.
constexpr reg32 kCameraViews = 0x56f234, kCameraFiles = 0x570a30;
constexpr reg32 kCameraViewBytes = 24, kCameraViewCount = 0x100, kCameraFileCount = 7;
constexpr reg32 kFirstCameraName = 0x5367fc, kCameraName = 0x536808;
// The track's textures: the shapes loaded, the .qfs files' names, the trace.
constexpr reg32 kTrackShapes = 0x5dd850;
constexpr reg32 kNear8 = 0x536d20, kNear = 0x536d28, kFar8 = 0x536d58, kFar = 0x536d60;
constexpr reg32 kTraceFile = 0x536d30, kTraceFormat = 0x536d50, kTraceLine = 0x744, kTraceWords = 0xc;
// The ghost: replaying, saving one; its file's name; its record, its records'
// count, its records; ghost read.
constexpr reg32 kGhostName = 0x53b0c8;
constexpr reg32 kGhost = 0x6ba5c4, kGhostCount = 0x6bc6b4, kGhostRecords = 0x6bc6b8, kGhostRead = 0x55d000;
constexpr reg32 kGhostBytes = 0x20f4, kGhostRecordBytes = 18;
// The horizon's .hrz names (night and weather, night, weather, neither); the
// records' block's name; its numbers: 0.01, 360/1024 in 16.16 turns, the
// sun's scales, 2^-16 (a float), -1, 2^-16 (a double).
constexpr reg32 kHorizonNightWeather = 0x53b238, kHorizonNight = 0x53b250, kHorizonWeather = 0x53b268;
constexpr reg32 kHorizonName = 0x53b280, kNoName = 0x53b298, kHundredth = 0x53b29c;
constexpr reg32 kHeightScale = 0x53b2a0, kSunAngleScale = 0x53b2a8, kSunDistanceScale = 0x53b2b0;
constexpr reg32 kTurnScale = 0x53b2b8, kMinusOne = 0x53b2bc, kFixedScale = 0x53b2c0;
// The sky: sky.fsh's name, its shapes' names (clouds, sun, glow: night and
// weather, night, weather, day), the cloud's palette and the sun's.
constexpr reg32 kSkyName = 0x53b340, kCloudPalette = 0x55d21c, kSunPalette = 0x6ff9a0, kSunShapes = 0x563a90;
// The weather: weather.fsh's name and its folder, the drops' and the
// splashes' names, their shapes; the weather's count (the horizon's).
constexpr reg32 kWeatherName = 0x549794, kWeatherPath = 0x7a2934, kDropName = 0x5497a4, kSplashName = 0x5497ac;
constexpr reg32 kDropShapes = 0x8ca200, kSplashShapes = 0x8ca288, kWeatherCount = 0x8ca1e8;
// What is called by address: the ghost's saving (0x477a90 leaves the registers as it leaves
// them) and fwrite (no typed core).  (The game's memory, a file there and loaded, a .viv's entry
// located, the trace, a handle read, the fixed sine and cosine and a vector's length, the C
// library's memset, stricmp, sprintf and fseek, the loading's shapes, and the menu files' BIG
// archive's count and names, the loading bar's tick, the horizon's numbers, a shape's texture,
// the sky's shape and grid, the sine and cosine are the other typed files', called as C++:
// native_ea_memory.h, native_ea_support.h, native_load_saves.h, native_raceload.h,
// native_ea_files.h, native_vertices.h, native_watcom.h, native_frontend_draw.h, and through
// feCore native_frontend_eab.h, _loada.h, _loopb.h, _rende.h, _wata.h, _renda.h, _simd.h.)
constexpr reg32 kSaveGhostFile = 0x4777d0;
constexpr reg32 kChop = 0x4dfd56;

/* The registers as a thunk is handed them, to put back those the game's code pushes and pops. */
struct Entry
{
    explicit Entry(const x86::Args& args)
        : ebx(args.ebx), ecx(args.ecx), edx(args.edx), esi(args.esi), edi(args.edi), ebp(args.ebp)
    {
    }
    reg32 ebx, ecx, edx, esi, edi, ebp;
};

/* A function's frame on the guest's stack, as deep as the code's: `pushed` bytes of registers
 * below esp, then `bytes` of locals; esp below them while it lasts. */
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

/* A native_frontend_* file's typed core called as C++ (guest::direct): nothing (false) when the
 * game stopped in it (its calls into the game throw fe::Terminated). */
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

/* A block of `bytes` from the game's allocator, named `name`: the block or 0. */
std::optional<reg32> allocate(App* app, CPU& cpu, reg32 name, reg32 bytes, reg32 flags)
{
    // edx and ebx as the code hands them, which the allocator keeps: the code's own after it (an
    // thunk leaves them so).
    return guest::direct<0x4e1620, ea::reserved>(app, cpu, name, bytes, flags);
}

/* A block given back to the game's allocator.  False when the game stopped. */
bool release(App* app, CPU& cpu, reg32 block)
{
    return guest::direct<0x4e1890, ea::purgeBlock>(app, cpu, block).has_value();
}

/* memset (0x4e0640): `bytes` bytes at `to` set to `value`; edx and ebx as the code loads them,
 * which memset keeps.  False when the game stopped. */
bool fill(App* app, CPU& cpu, reg32 to, reg32 value, reg32 bytes)
{
    guest::direct<0x4e0640, watcom::clib::memset>(app, cpu, to, value, bytes);
    return !cpu.terminate;
}

/* stricmp (0x4e9ec0) of the names at `name` and `other`: 0 when they are alike; edx as the code
 * loads it, which stricmp keeps. */
std::optional<reg32> compareNames(App* app, CPU& cpu, reg32 name, reg32 other)
{
    const reg32 differs = guest::direct<0x4e9ec0, watcom::clib::stricmp>(app, cpu, name, other);
    if (cpu.terminate)
        return std::nullopt;
    return differs;
}

/* sprintf (0x4df690) to the first word, of the format in the second and what follows it. */
std::optional<reg32> format(App* app, CPU& cpu, std::initializer_list<reg32> words)
{
    const reg32* const word = words.begin();
    guest::Words rest({});
    for (const reg32* w = word + 2; w != words.end(); ++w)
        rest.word[rest.count++] = *w;
    const reg32 written = guest::direct<0x4df690, watcom::clib::sprintf>(app, cpu, word[0], word[1], rest);
    if (cpu.terminate)
        return std::nullopt;
    return written;
}

/* Whether the file named `name` is there. */
std::optional<bool> fileExists(App* app, CPU& cpu, reg32 name)
{
    return guest::direct<0x4e0e60, ea::fileThere>(app, cpu, name);
}

/* The file named `name` loaded into a block of its own (`flags` the allocator's): the block or 0. */
std::optional<reg32> loadFile(App* app, CPU& cpu, reg32 name, reg32 flags)
{
    return guest::direct<0x4e0ec0, ea::loadWhole>(app, cpu, name, flags);
}

/* The loading screen's bar ticked on (0x4a33d0, loada's): whether to stop, which its callers
 * leave in eax as the last call.  Nothing when the game stopped. */
std::optional<reg32> loadingStop(App* app, CPU& cpu)
{
    return feCore<0x4a33d0, frontend::loada::stopCheck>(app, cpu);
}

/* The same, false when the game stopped. */
bool loadingTick(App* app, CPU& cpu)
{
    return loadingStop(app, cpu).has_value();
}

/* A dword turned around: a BIGF archive's big-endian number. */
reg32 turned(reg32 value)
{
    return value >> 24 | (value >> 8 & 0xff00) | (value << 8 & 0xff0000) | value << 24;
}

/* rep movsd, rep movsb: `bytes` bytes from `from` to `to`, a dword at a time and the last few
 * a byte at a time, forwards. */
void copyBytes(App* app, reg32 to, reg32 from, reg32 bytes)
{
    for (reg32 dwords = bytes >> 2; dwords != 0; --dwords, to += 4, from += 4)
        put(app, to, at(app, from));
    for (reg32 left = bytes & 3; left != 0; --left)
        putByte(app, to++, byte(app, from++));
}

/* The code's inlined strcpy: the name at `from` to `to`, its ending too, a byte at a time. */
void copyName(App* app, reg32 to, reg32 from)
{
    for (reg32 i = 0;; ++i)
    {
        const reg8 c = byte(app, from + i);
        putByte(app, to + i, c);
        if (c == 0)
            break;
    }
}

/* The code's inlined strlen (repne scasb). */
reg32 nameLength(App* app, reg32 name)
{
    reg32 length = 0;
    while (byte(app, name + length) != 0)
        ++length;
    return length;
}

}  // namespace

x86::reg32 loadOpponentSpeeds(App* app, CPU& cpu, x86::Args& args);
x86::reg32 clearTrackCameras(App* app, CPU& cpu, x86::Args& args);
x86::reg32 loadTrackCameras(App* app, CPU& cpu, x86::Args& args);
x86::reg32 swapBigEndian(App* app, CPU& cpu, x86::Args& args);
x86::reg32 newFileNode(App* app, CPU& cpu, x86::Args& args);
x86::reg32 freeFileNode(App* app, CPU& cpu, x86::Args& args);
x86::reg32 findFileNode(App* app, CPU& cpu, x86::Args& args);
x86::reg32 insertFileNode(App* app, CPU& cpu, x86::Args& args);
x86::reg32 newEmptyFileNode(App* app, CPU& cpu, x86::Args& args);
x86::reg32 freeFileNodes(App* app, CPU& cpu, x86::Args& args);
x86::reg32 countFileNodes(App* app, CPU& cpu, x86::Args& args);
x86::reg32 findCachedFile(App* app, CPU& cpu, x86::Args& args);
x86::reg32 addCachedFile(App* app, CPU& cpu, x86::Args& args);
x86::reg32 newFileCache(App* app, CPU& cpu, x86::Args& args);
x86::reg32 freeFileCache(App* app, CPU& cpu, x86::Args& args);
x86::reg32 bigHeaderSize(App* app, CPU& cpu, x86::Args& args);
x86::reg32 cachedDataSize(App* app, CPU& cpu, x86::Args& args);
x86::reg32 writeBigFile(App* app, CPU& cpu, x86::Args& args);
x86::reg32 packFileCache(App* app, CPU& cpu, x86::Args& args);
x86::reg32 readBigFile(App* app, CPU& cpu, x86::Args& args);
x86::reg32 openFileCache(App* app, CPU& cpu, x86::Args& args);
x86::reg32 trackFilePath(App* app, CPU& cpu, x86::Args& args);
x86::reg32 trackFolderPath(App* app, CPU& cpu, x86::Args& args);
x86::reg32 loadTrackTextures(App* app, CPU& cpu, x86::Args& args);
x86::reg32 saveGhost(App* app, CPU& cpu, x86::Args& args);
x86::reg32 writeGhost(App* app, CPU& cpu, x86::Args& args);
x86::reg32 readGhost(App* app, CPU& cpu, x86::Args& args);
x86::reg32 loadHorizon(App* app, CPU& cpu, x86::Args& args);
x86::reg32 loadSky(App* app, CPU& cpu, x86::Args& args);
x86::reg32 loadWeatherShapes(App* app, CPU& cpu, x86::Args& args);

namespace
{

/* The file `name` in the track's folder, "<data path>TRKnnn\<name>", sprintf'd to 0x5dd8ec:
 * that path. */
std::optional<reg32> trackFolderFile(App* app, CPU& cpu, reg32 name)
{
    if (!format(app, cpu, {kFolderFile, kFolderFileFormat, kDataPath, gameSettings(app).track, name}))
        return std::nullopt;
    return kFolderFile;
}

}  // namespace

/* The track's file named by its end `end`, "<data path>TRKnnn\Trnn<end>", sprintf'd to 0x5dd86c:
 * that path. */
std::optional<reg32> trackFile(App* app, CPU& cpu, reg32 end)
{
    const reg32 track = gameSettings(app).track;
    if (!format(app, cpu, {kTrackFile, kTrackFileFormat, kDataPath, track, track, end}))
        return std::nullopt;
    return kTrackFile;
}

/* The opponents' speed table, speedsF.bin (speedsR.bin backwards) in the track's folder, loaded
 * to [0x552558] if it is there, [0x55255c] its second half (half the track's nodes, rounded up,
 * on) and [0x56f22c] 0; else [0x552558] 0 and [0x56f22c] 1.  The second half's index, or 0. */
std::optional<reg32> loadSpeeds(App* app, CPU& cpu)
{
    const std::optional<reg32> path =
        trackFolderFile(app, cpu, gameSettings(app).backwards != 0 ? kSpeedsBackward : kSpeedsForward);
    if (!path)
        return std::nullopt;
    const std::optional<bool> there = fileExists(app, cpu, *path);
    if (!there)
        return std::nullopt;
    reg32 table = 0;
    if (*there)
    {
        const std::optional<reg32> loaded = loadFile(app, cpu, *path, kSpeedsLoadFlags);
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
    const reg32 half = reg32(sreg32(at(app, kTrackNodes) + 1) / 2);
    put(app, kSpeeds, table);
    put(app, kSpeedsHalf, table + half);
    put(app, kSpeedsFailed, 0);
    return half;
}

/* The cameras' 256 views (24 bytes from 0x56f234) cleared -- 0, -1, 0 -- the loading bar
 * ticked before each.  False when the game stopped. */
bool clearCameras(App* app, CPU& cpu)
{
    for (reg32 view = 0; view < kCameraViewCount; ++view)
    {
        if (!loadingTick(app, cpu))
            return false;
        const reg32 at = kCameraViews + view * kCameraViewBytes;
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
std::optional<reg32> loadCameras(App* app, CPU& cpu, reg32 path)
{
    const GuestFrame frame(cpu, 0x14, 0x100);
    const reg32 name = frame - 0x100;  // handed to sprintf and the loading
    if (!clearCameras(app, cpu))
        return std::nullopt;
    for (reg32 slot = 1; slot <= kCameraFileCount; ++slot)
    {
        if (!loadingTick(app, cpu))
            return std::nullopt;
        put(app, kCameraFiles + slot * 4, 0);
    }
    if (at(app, kLoadAborted) != 0)
        return 0;
    reg32 loaded = 0;
    reg32 nameFormat = kFirstCameraName;
    for (;;)
    {
        if (!format(app, cpu, {name, nameFormat, path, loaded}))
            return std::nullopt;
        if (sreg32(loaded) >= sreg32(kCameraFileCount))
            break;
        const std::optional<bool> there = fileExists(app, cpu, name);
        if (!there)
            return std::nullopt;
        if (!*there)
            break;
        if (!loadingTick(app, cpu))
            return std::nullopt;
        ++loaded;
        const std::optional<reg32> file = loadFile(app, cpu, name, 0);
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
reg32 swapInPlace(App* app, reg32 address)
{
    const reg32 value = turned(at(app, address));
    put(app, address, value);
    return value;
}

/* A file cache's node, "FileNode" (0x30 bytes), allocated and cleared -- cleared at 0 too if
 * there is none: the node or 0. */
std::optional<reg32> newNode(App* app, CPU& cpu)
{
    const std::optional<reg32> node = allocate(app, cpu, kNodeName, kNodeBytes, kBlockFlags);
    if (!node || !fill(app, cpu, *node, 0, kNodeBytes))
        return std::nullopt;
    return node;
}

/* The node freed, its file's data first (if any); its size, its data and its name's first
 * byte cleared before.  False when the game stopped. */
bool freeNode(App* app, CPU& cpu, reg32 node)
{
    if (node == 0)
        return true;
    put(app, node + kNodeSize, 0);
    const reg32 data = at(app, node + kNodeData);
    putByte(app, node, 0);
    if (data != 0)
    {
        if (!release(app, cpu, data))
            return false;
        put(app, node + kNodeData, 0);
    }
    return release(app, cpu, node);
}

/* The node named `name` (stricmp) of the list after the node `after`, or 0. */
std::optional<reg32> findNode(App* app, CPU& cpu, reg32 name, reg32 after)
{
    for (reg32 node = at(app, after + kNodeNext); node != 0; node = at(app, node + kNodeNext))
    {
        const std::optional<reg32> differs = compareNames(app, cpu, name, node);
        if (!differs)
            return std::nullopt;
        if (*differs == 0)
            return node;
    }
    return 0;
}

/* A node made for the file named `name`, of `size` bytes at `data`, and linked in after the
 * node `after`: the node, or 0. */
std::optional<reg32> insertNode(App* app, CPU& cpu, reg32 name, reg32 size, reg32 data, reg32 after)
{
    const std::optional<reg32> node = newNode(app, cpu);
    if (!node || *node == 0)
        return node;
    copyName(app, *node, name);
    put(app, *node + kNodeSize, size);
    put(app, *node + kNodeData, data);
    const reg32 next = at(app, after + kNodeNext);
    if (next != 0)
        put(app, next + kNodePrevious, *node);
    put(app, *node + kNodeNext, at(app, after + kNodeNext));
    put(app, *node + kNodePrevious, after);
    put(app, after + kNodeNext, *node);
    return node;
}

/* An empty node -- no name, size, data or links -- or 0. */
std::optional<reg32> newEmptyNode(App* app, CPU& cpu)
{
    const std::optional<reg32> node = newNode(app, cpu);
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
bool freeNodes(App* app, CPU& cpu, reg32 node)
{
    if (node == 0)
        return true;
    reg32 next = at(app, node + kNodeNext);
    if (!freeNode(app, cpu, node))
        return false;
    while (next != 0)
    {
        node = next;
        next = at(app, node + kNodeNext);
        if (!freeNode(app, cpu, node))
            return false;
    }
    return true;
}

/* The files in the cache: the nodes after its head. */
reg32 countFiles(App* app, reg32 cache)
{
    const reg32 list = at(app, cache + kCacheList);
    reg32 count = 0;
    if (list != 0)
        for (reg32 node = at(app, list + kNodeNext); node != 0; node = at(app, node + kNodeNext))
            ++count;
    return count;
}

}  // namespace

/* The node of the file named `name` in the cache, or 0 -- for no cache or no list too. */
std::optional<reg32> findCached(App* app, CPU& cpu, reg32 cache, reg32 name)
{
    if (cache == 0)
        return 0;
    const reg32 list = at(app, cache + kCacheList);
    if (list == 0)
        return 0;
    return findNode(app, cpu, name, list);
}

namespace
{

/* What addFile leaves: whether the cache had its list and its last node, the file's copy (0
 * when no block was had), the node made for it (0 for none) and the last node it went after. */
struct AddedFile
{
    bool listed = false;
    reg32 copy = 0, node = 0, after = 0;
};

/* The file named `name`, `size` bytes at `data`, copied into a block of its own ("mpFileData")
 * and added at the end of the cache, its last node then. */
std::optional<AddedFile> addFile(App* app, CPU& cpu, reg32 cache, reg32 name, reg32 size, reg32 data)
{
    AddedFile added;
    if (cache == 0 || at(app, cache + kCacheList) == 0 || at(app, cache + kCacheLast) == 0)
        return added;
    added.listed = true;
    const std::optional<reg32> copy = allocate(app, cpu, kDataName, size, kBlockFlags);
    if (!copy)
        return std::nullopt;
    added.copy = *copy;
    if (*copy == 0)
        return added;
    copyBytes(app, *copy, data, size);
    added.after = at(app, cache + kCacheLast);
    const std::optional<reg32> node = insertNode(app, cpu, name, size, *copy, added.after);
    if (!node)
        return std::nullopt;
    added.node = *node;
    if (*node != 0)
        put(app, cache + kCacheLast, *node);
    return added;
}

/* A file cache ("BFL"): 'FGIB', its head (an empty node) its list's and its last; 0 if either
 * cannot be had. */
std::optional<reg32> newCache(App* app, CPU& cpu)
{
    const std::optional<reg32> cache = allocate(app, cpu, kCacheName, kCacheBytes, kBlockFlags);
    if (!cache || *cache == 0)
        return cache;
    put(app, *cache, kBigMagic);
    put(app, *cache + kCacheList, 0);
    put(app, *cache + kCacheLast, 0);
    const std::optional<reg32> head = newEmptyNode(app, cpu);
    if (!head)
        return std::nullopt;
    if (*head != 0)
    {
        put(app, *cache + kCacheLast, *head);
        put(app, *cache + kCacheList, *head);
        return cache;
    }
    if (!release(app, cpu, *cache))
        return std::nullopt;
    return 0;
}

/* The cache freed, its nodes first and it cleared then, if it has a list.  False when the game
 * stopped. */
bool freeCache(App* app, CPU& cpu, reg32 cache)
{
    if (cache == 0)
        return true;
    const reg32 list = at(app, cache + kCacheList);
    if (list != 0)
    {
        if (!freeNodes(app, cpu, list))
            return false;
        put(app, cache + kCacheList, 0);
        put(app, cache, 0);
        put(app, cache + kCacheLast, 0);
    }
    return release(app, cpu, cache);
}

/* The BIGF header the cache's files take: 16 bytes and, for each, 8 and its name with its
 * ending.  0 for no cache or list, not a cache's magic, a file of no size or no files. */
reg32 headerSize(App* app, reg32 cache)
{
    if (cache == 0)
        return 0;
    const reg32 list = at(app, cache + kCacheList);
    if (list == 0 || at(app, cache) != kBigMagic)
        return 0;
    reg32 size = kBigHeaderBytes;
    for (reg32 node = at(app, list + kNodeNext); node != 0; node = at(app, node + kNodeNext))
    {
        if (at(app, node + kNodeSize) == 0)
            return 0;
        size += nameLength(app, node) + kEntryName + 1;
    }
    return size <= kBigHeaderBytes ? 0 : size;
}

/* The bytes of the cache's files, all told; 0 if one has none.  The cache and its list are
 * taken to be there. */
reg32 dataSize(App* app, reg32 cache)
{
    reg32 total = 0;
    for (reg32 node = at(app, at(app, cache + kCacheList) + kNodeNext); node != 0; node = at(app, node + kNodeNext))
    {
        const reg32 size = at(app, node + kNodeSize);
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
    reg32 written, entry, file;
};

/* The cache written as a BIGF archive into the `size` bytes at `buffer` (cleared first): each
 * file's offset, size and name in the header and its data after it, as far as they fit, then
 * the header's four numbers.  Written 0 for no buffer, cache, size or list, no files, a header
 * too small, all of it not fitting, or a file of no size or data. */
std::optional<BigWritten> writeBig(App* app, CPU& cpu, reg32 buffer, reg32 cache, reg32 size)
{
    const reg32 end = buffer + size;
    if (cache == 0 || buffer == 0 || size == 0 || at(app, cache + kCacheList) == 0)
        return BigWritten{0, size, 0};
    const reg32 files = countFiles(app, cache);
    const reg32 header = headerSize(app, cache);
    const reg32 data = dataSize(app, cache);
    if (!(sreg32(files) > 0 && header >= kBigHeaderBytes && data + header <= size))
        return BigWritten{0, size, files};
    if (!fill(app, cpu, buffer, 0, size))
        return std::nullopt;
    reg32 entry = buffer + kBigHeaderBytes;
    const reg32 headerEnd = buffer + header;
    reg32 cursor = headerEnd;
    reg32 count = 0;
    reg32 file = at(app, at(app, cache + kCacheList) + kNodeNext);
    for (; file != 0; file = at(app, file + kNodeNext))
    {
        const reg32 fileSize = at(app, file + kNodeSize);
        if (cursor + fileSize > end || entry >= headerEnd)
            break;
        if (fileSize == 0 || at(app, file + kNodeData) == 0)
            return BigWritten{0, entry, file};
        const reg32 nameAt = entry + kEntryName;
        const reg32 length = nameLength(app, file);
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
std::optional<Packed> packCache(App* app, CPU& cpu, reg32 cache, reg32 sizeTo)
{
    if (cache == 0 || at(app, cache + kCacheList) == 0)
    {
        put(app, sizeTo, 0);
        return Packed{0, sizeTo};
    }
    const reg32 header = headerSize(app, cache);
    const reg32 data = dataSize(app, cache);
    if (header == 0 || data == 0)
    {
        put(app, sizeTo, 0);
        return Packed{0, header};
    }
    const reg32 size = header + data;
    const std::optional<reg32> buffer = allocate(app, cpu, kBufferName, size, kBlockFlags);
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
        if (!release(app, cpu, *buffer))
            return std::nullopt;
        return Packed{0, size};
    }
    const std::optional<BigWritten> written = writeBig(app, cpu, *buffer, cache, size);
    if (!written)
        return std::nullopt;
    if (written->written != 0)
    {
        put(app, sizeTo, size);
        return Packed{*buffer, written->file};
    }
    put(app, sizeTo, 0);
    if (!release(app, cpu, *buffer))
        return std::nullopt;
    return Packed{0, written->file};
}

namespace
{

/* What readBig leaves: 1 read, or 0; and what its code leaves in edx, which no caller reads. */
struct BigRead
{
    reg32 read, left;
};

/* The files of the BIGF archive at `data` added to the cache, each found by its name (0x4ea4a0,
 * 0x4ea290); those of no size passed over.  Read 0 for no cache or archive, not a BIGF, no
 * files, or a file not found or added. */
std::optional<BigRead> readBig(App* app, CPU& cpu, reg32 cache, reg32 data)
{
    const GuestFrame frame(cpu, 0x14, 0x34);
    // Handed to EA's BIG functions: a file's offset in the archive, its size, its name.
    const reg32 offset = frame - 0xc, size = frame - 0x10, name = frame - 0x34;
    put(app, offset, 0);
    put(app, size, 0);
    if (data == 0 || cache == 0)
        return BigRead{0, 0};
    if (turned(at(app, data)) != kBigMagic)
        return BigRead{0, 0};
    const std::optional<reg32> files = feCore<0x4ea440, frontend::eab::bigFileCount>(app, cpu, data);
    if (!files)
        return std::nullopt;
    if (*files == 0)
        return BigRead{0, 0};
    reg32 left = 0;
    for (reg32 file = 0; sreg32(file) < sreg32(*files); ++file)
    {
            if (!feCore<0x4ea4a0, frontend::eab::bigFileName>(app, cpu, data, file, name))
            return std::nullopt;
        const std::optional<reg32> found = guest::direct<0x4ea290, locatedEntry>(app, cpu, data, name, file, offset, size);
        if (!found)
            return std::nullopt;
        if (*found == 0)
            return BigRead{0, name};
        left = at(app, size);
        if (left != 0)
        {
            const std::optional<AddedFile> added = addFile(app, cpu, cache, name, left, data + at(app, offset));
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
std::optional<reg32> openCache(App* app, CPU& cpu, reg32 data)
{
    const GuestFrame frame(cpu, 0x14, 4);  // as deep as the code's: readBig's frame below it
    if (data == 0 || turned(at(app, data)) != kBigMagic)
        return 0;
    const std::optional<reg32> cache = newCache(app, cpu);
    if (!cache || *cache == 0)
        return cache;
    std::optional<BigRead> read;
    {
        const GuestFrame slot(cpu, 0, 4);  // the call's return address
        read = readBig(app, cpu, *cache, data);
    }
    if (!read)
        return std::nullopt;
    if (read->read != 0)
        return cache;
    if (!freeCache(app, cpu, *cache))
        return std::nullopt;
    return 0;
}

/* 0x40e000: loadSpeeds; eax the second half's index, or 0. */
x86::reg32 loadOpponentSpeeds(App* app, CPU& cpu, x86::Args& args)
{
    const Entry entry(args);
    const std::optional<reg32> half = loadSpeeds(app, cpu);
    if (!half)
        return args.eax;
    args.eax = *half;
    args.ebx = entry.ebx;
    args.ecx = entry.ecx;
    args.edx = entry.edx;
    args.esi = entry.esi;
    cpu.esp += 4;
    return args.eax;
}

/* 0x40f230: clearCameras; eax the last view's index times 3, as the code leaves it. */
x86::reg32 clearTrackCameras(App* app, CPU& cpu, x86::Args& args)
{
    const Entry entry(args);
    if (!clearCameras(app, cpu))
        return args.eax;
    args.eax = (kCameraViewCount - 1) * 3;
    args.ebx = entry.ebx;
    args.ecx = entry.ecx;
    args.edx = entry.edx;
    args.esi = entry.esi;
    cpu.esp += 4;
    return args.eax;
}

/* 0x40f280: loadCameras for the path eax; eax how many. */
x86::reg32 loadTrackCameras(App* app, CPU& cpu, x86::Args& args)
{
    const Entry entry(args);
    const std::optional<reg32> loaded = loadCameras(app, cpu, args.eax);
    if (!loaded)
        return args.eax;
    args.eax = *loaded;
    args.ebx = entry.ebx;
    args.ecx = entry.ecx;
    args.edx = entry.edx;
    args.esi = entry.esi;
    cpu.esp += 4;
    return args.eax;
}

/* 0x418d00: swapInPlace of the dword at eax; eax it now. */
x86::reg32 swapBigEndian(App* app, CPU& cpu, x86::Args& args)
{
    args.eax = swapInPlace(app, args.eax);
    cpu.esp += 4;
    return args.eax;
}

/* 0x418d40: newNode; eax the node. */
x86::reg32 newFileNode(App* app, CPU& cpu, x86::Args& args)
{
    const Entry entry(args);
    const std::optional<reg32> node = newNode(app, cpu);
    if (!node)
        return args.eax;
    args.eax = *node;
    args.ebx = entry.ebx;
    args.ecx = entry.ecx;
    args.edx = entry.edx;
    cpu.esp += 4;
    return args.eax;
}

/* 0x418d70: freeNode of the node eax; eax as the last free leaves it. */
x86::reg32 freeFileNode(App* app, CPU& cpu, x86::Args& args)
{
    if (!freeNode(app, cpu, args.eax))
        return args.eax;
    cpu.esp += 4;
    return args.eax;
}

/* 0x418dd0: findNode of the name eax after the node edx; eax the node, edx as stricmp leaves it. */
x86::reg32 findFileNode(App* app, CPU& cpu, x86::Args& args)
{
    const Entry entry(args);
    const std::optional<reg32> node = findNode(app, cpu, args.eax, args.edx);
    if (!node)
        return args.eax;
    args.eax = *node;
    args.ebx = entry.ebx;
    args.ecx = entry.ecx;
    cpu.esp += 4;
    return args.eax;
}

/* 0x418e90: insertNode of the name eax, edx bytes at ebx, after the node ecx; eax, ebx and edx
 * the node. */
x86::reg32 insertFileNode(App* app, CPU& cpu, x86::Args& args)
{
    const Entry entry(args);
    const std::optional<reg32> node = insertNode(app, cpu, args.eax, args.edx, args.ebx, args.ecx);
    if (!node)
        return args.eax;
    args.eax = *node;
    args.ebx = *node;
    args.ecx = entry.ecx;
    args.edx = *node;
    cpu.esp += 4;
    return args.eax;
}

/* 0x418f00: newEmptyNode; eax the node. */
x86::reg32 newEmptyFileNode(App* app, CPU& cpu, x86::Args& args)
{
    const Entry entry(args);
    const std::optional<reg32> node = newEmptyNode(app, cpu);
    if (!node)
        return args.eax;
    args.eax = *node;
    args.ebx = entry.ebx;
    args.ecx = entry.ecx;
    args.edx = entry.edx;
    cpu.esp += 4;
    return args.eax;
}

/* 0x418f40: freeNodes from the node eax; eax as the last free leaves it. */
x86::reg32 freeFileNodes(App* app, CPU& cpu, x86::Args& args)
{
    if (!freeNodes(app, cpu, args.eax))
        return args.eax;
    cpu.esp += 4;
    return args.eax;
}

/* 0x418f70: countFiles of the cache eax. */
x86::reg32 countFileNodes(App* app, CPU& cpu, x86::Args& args)
{
    args.eax = countFiles(app, args.eax);
    cpu.esp += 4;
    return args.eax;
}

/* 0x418fe0: findCached of the name edx in the cache eax; edx, which no caller reads, as stricmp
 * leaves it. */
x86::reg32 findCachedFile(App* app, CPU& cpu, x86::Args& args)
{
    const Entry entry(args);
    const reg32 cache = args.eax;
    const std::optional<reg32> node = findCached(app, cpu, cache, args.edx);
    if (!node)
        return args.eax;
    args.eax = *node;
    args.ebx = entry.ebx;
    args.ecx = entry.ecx;
    cpu.esp += 4;
    return args.eax;
}

/* 0x419000: addFile of the name edx, ebx bytes at ecx, to the cache eax; eax 1 when it was
 * added, and ebx, ecx and edx as the code leaves them. */
x86::reg32 addCachedFile(App* app, CPU& cpu, x86::Args& args)
{
    const Entry entry(args);
    const reg32 cache = args.eax;
    const std::optional<AddedFile> added = addFile(app, cpu, cache, args.edx, args.ebx, args.ecx);
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
    cpu.esp += 4;
    return args.eax;
}

/* 0x419080: newCache; eax the cache. */
x86::reg32 newFileCache(App* app, CPU& cpu, x86::Args& args)
{
    const Entry entry(args);
    const std::optional<reg32> cache = newCache(app, cpu);
    if (!cache)
        return args.eax;
    args.eax = *cache;
    args.ebx = entry.ebx;
    args.ecx = entry.ecx;
    args.edx = entry.edx;
    cpu.esp += 4;
    return args.eax;
}

/* 0x4190e0: freeCache of the cache eax; eax as the last free leaves it. */
x86::reg32 freeFileCache(App* app, CPU& cpu, x86::Args& args)
{
    const Entry entry(args);
    if (!freeCache(app, cpu, args.eax))
        return args.eax;
    args.ecx = entry.ecx;
    args.edx = entry.edx;
    cpu.esp += 4;
    return args.eax;
}

/* 0x419120: headerSize of the cache eax. */
x86::reg32 bigHeaderSize(App* app, CPU& cpu, x86::Args& args)
{
    args.eax = headerSize(app, args.eax);
    cpu.esp += 4;
    return args.eax;
}

/* 0x4191b0: dataSize of the cache eax. */
x86::reg32 cachedDataSize(App* app, CPU& cpu, x86::Args& args)
{
    args.eax = dataSize(app, args.eax);
    cpu.esp += 4;
    return args.eax;
}

/* 0x4191e0: writeBig of the cache edx into the ebx bytes at eax; eax 1 written, ebx and edx
 * where it stopped. */
x86::reg32 writeBigFile(App* app, CPU& cpu, x86::Args& args)
{
    const Entry entry(args);
    const std::optional<BigWritten> written = writeBig(app, cpu, args.eax, args.edx, args.ebx);
    if (!written)
        return args.eax;
    args.eax = written->written;
    args.ebx = written->entry;
    args.ecx = entry.ecx;
    args.edx = written->file;
    args.esi = entry.esi;
    args.edi = entry.edi;
    cpu.esp += 4;
    return args.eax;
}

/* 0x419460: packCache of the cache eax, its size to [edx]; eax the buffer, edx as the code
 * leaves it. */
x86::reg32 packFileCache(App* app, CPU& cpu, x86::Args& args)
{
    const Entry entry(args);
    const std::optional<Packed> packed = packCache(app, cpu, args.eax, args.edx);
    if (!packed)
        return args.eax;
    args.eax = packed->buffer;
    args.ebx = entry.ebx;
    args.ecx = entry.ecx;
    args.edx = packed->left;
    args.esi = entry.esi;
    args.edi = entry.edi;
    cpu.esp += 4;
    return args.eax;
}

/* 0x419510: readBig of the archive at edx into the cache eax; eax 1 read, edx as the code
 * leaves it. */
x86::reg32 readBigFile(App* app, CPU& cpu, x86::Args& args)
{
    const Entry entry(args);
    const std::optional<BigRead> read = readBig(app, cpu, args.eax, args.edx);
    if (!read)
        return args.eax;
    args.eax = read->read;
    args.ebx = entry.ebx;
    args.ecx = entry.ecx;
    args.edx = read->left;
    args.esi = entry.esi;
    args.edi = entry.edi;
    cpu.esp += 4;
    return args.eax;
}

/* 0x4195f0: openCache of the archive at eax; eax the cache. */
x86::reg32 openFileCache(App* app, CPU& cpu, x86::Args& args)
{
    const Entry entry(args);
    const std::optional<reg32> cache = openCache(app, cpu, args.eax);
    if (!cache)
        return args.eax;
    args.eax = *cache;
    args.ebx = entry.ebx;
    args.ecx = entry.ecx;
    args.edx = entry.edx;
    args.esi = entry.esi;
    args.edi = entry.edi;
    cpu.esp += 4;
    return args.eax;
}


namespace
{

/* The track's textures loaded (0x4956f0) to [0x5dd850] unless the loading was stopped: its own
 * Trnn1.qfs (Trnn1_8.qfs for the options' smaller set) where the race has them (mirrored) and
 * the file is there -- traced, as bworld.c's line 1860 does (0x42d500) -- else Trnn0.qfs
 * (Trnn0_8.qfs).  False when the game stopped. */
bool loadTextures(App* app, CPU& cpu)
{
    put(app, kTrackShapes, 0);
    if (!loadingTick(app, cpu))
        return false;
    if (at(app, kLoadAborted) != 0)
        return true;
    reg32 path = 0;
    if (gameSettings(app).mirrored != 0)
    {
        const std::optional<reg32> own =
            trackFile(app, cpu, (byte(app, kOptions) & kSmallTextures) != 0 ? kNear8 : kNear);
        if (!own)
            return false;
        const std::optional<bool> there = fileExists(app, cpu, *own);
        if (!there)
            return false;
        if (*there)
        {
            // The trace's words pushed as the code pushes them (it reads none of them).
            const GuestFrame words(cpu, 0, kTraceWords);
            reg32 slot = words - kTraceWords;
            for (const reg32 word : {kTraceFormat, kTraceFile, kTraceLine})
            {
                put(app, slot, word);
                slot += 4;
            }
            guest::direct<0x42d500, raceload::noHook>(app, cpu);
            if (cpu.terminate)
                return false;
            path = *own;
        }
    }
    if (path == 0)
    {
        const std::optional<reg32> shared =
            trackFile(app, cpu, (byte(app, kOptions) & kSmallTextures) != 0 ? kFar8 : kFar);
        if (!shared)
            return false;
        path = *shared;
    }
    const reg32 shapes = guest::direct<0x4956f0, frontend::draw::loadQfsFlags>(app, cpu, path, 0u);
    if (cpu.terminate)
        return false;
    put(app, kTrackShapes, shapes);
    return true;
}

/* The ghost saved to ghost.gst (0x4777d0) when the race is no replay and was run.  False when
 * the game stopped. */
bool saveRacedGhost(App* app, CPU& cpu)
{
    if (gameSettings(app).replayMode != 0 || gameSettings(app).lastTickRaced == 0)
        return true;
    return feCore<0x4777d0, frontend::loopa::saveGhost>(app, cpu, kGhostName).has_value();
}

}  // namespace

/* The ghost written to the stream at `offset` (fseek from its start, then fwrite: its record
 * and its records, 0x20f4 + 18 bytes each), if there is a stream.  False when the game stopped. */
bool writeGhostTo(App* app, CPU& cpu, reg32 stream, reg32 offset)
{
    if (stream == 0)
        return true;
    guest::direct<0x4e9088, watcom::clib::fseek>(app, cpu, stream, offset, 0);
    if (cpu.terminate)
        return false;
    guest::direct<0x4eebd0, watcom::clib::fwrite>(app, cpu, kGhost, at(app, kGhostCount) * kGhostRecordBytes + kGhostBytes, 1u,
                                                  stream);
    return !cpu.terminate;
}

/* The ghost read from the game's file handle (0x4e2de0), if there is one: its record, then as
 * many records as it says; [0x55d000] 1.  False when the game stopped. */
bool readGhostFrom(App* app, CPU& cpu, reg32 handle)
{
    if (handle == 0)
        return true;
    if (!guest::direct<0x4e2de0, ea::readChecked>(app, cpu, handle, kGhost, kGhostBytes))
        return false;
    if (!guest::direct<0x4e2de0, ea::readChecked>(app, cpu, handle, kGhostRecords, at(app, kGhostCount) * kGhostRecordBytes))
        return false;
    put(app, kGhostRead, 1);
    return true;
}

/* 0x41d8a0: trackFile of the end eax; eax the path. */
x86::reg32 trackFilePath(App* app, CPU& cpu, x86::Args& args)
{
    const Entry entry(args);
    const std::optional<reg32> path = trackFile(app, cpu, args.eax);
    if (!path)
        return args.eax;
    args.eax = *path;
    args.edx = entry.edx;
    cpu.esp += 4;
    return args.eax;
}

/* 0x41d8d0: trackFolderFile of the name eax; eax the path. */
x86::reg32 trackFolderPath(App* app, CPU& cpu, x86::Args& args)
{
    const Entry entry(args);
    const std::optional<reg32> path = trackFolderFile(app, cpu, args.eax);
    if (!path)
        return args.eax;
    args.eax = *path;
    args.edx = entry.edx;
    cpu.esp += 4;
    return args.eax;
}

/* 0x41dbc0: loadTextures; eax as the last call leaves it (the shapes, or the loading bar's). */
x86::reg32 loadTrackTextures(App* app, CPU& cpu, x86::Args& args)
{
    const Entry entry(args);
    if (!loadTextures(app, cpu))
        return args.eax;
    args.ecx = entry.ecx;
    args.edx = entry.edx;
    cpu.esp += 4;
    return args.eax;
}

/* 0x477a90: saveRacedGhost; the registers as the saving leaves them. */
x86::reg32 saveGhost(App* app, CPU& cpu, x86::Args& args)
{
    if (!saveRacedGhost(app, cpu))
        return args.eax;
    cpu.esp += 4;
    return args.eax;
}

/* 0x477ac0: writeGhostTo the stream eax at edx; eax and edx as fwrite leaves them. */
x86::reg32 writeGhost(App* app, CPU& cpu, x86::Args& args)
{
    const Entry entry(args);
    if (!writeGhostTo(app, cpu, args.eax, args.edx))
        return args.eax;
    args.ebx = entry.ebx;
    args.ecx = entry.ecx;
    cpu.esp += 4;
    return args.eax;
}

/* 0x477b00: readGhostFrom the handle eax; eax as the reading leaves it. */
x86::reg32 readGhost(App* app, CPU& cpu, x86::Args& args)
{
    const Entry entry(args);
    if (!readGhostFrom(app, cpu, args.eax))
        return args.eax;
    args.ebx = entry.ebx;
    args.ecx = entry.ecx;
    args.edx = entry.edx;
    cpu.esp += 4;
    return args.eax;
}

namespace
{

// The sky's, as the horizon fills them: its colour (three bytes and a dword), the fog's
// distance (a hundredth), the records' count and block (36 bytes each).
constexpr reg32 kSkyColour = 0x55d26c, kSkyColourWord = 0x55d270, kFogScale = 0x55d268;
constexpr reg32 kRecordCount = 0x55d274, kRecords = 0x55d278, kRecordBytes = 36;
constexpr sreg32 kFogNearest = 1, kFogFarthest = 100;
// The sun's and the clouds': four numbers, the sun's size (and its reciprocal), two floats, the
// sun's height (chopped), its place (cosine and sine times its distance), the clouds' (two
// numbers and a float), the clouds' turn (16.16 turns, and a float within 0 and 1), a number and
// five floats.
constexpr reg32 kSunNumbers[] = {0x55d1f4, 0x55d204, 0x55d260, 0x55d264};
constexpr reg32 kSunSize = 0x55d208, kSunFloat = 0x55d248, kSunTripled = 0x55d24c;
constexpr reg32 kSunNumber = 0x55d1f8, kSunOther = 0x55d200, kSunHeight = 0x55d1fc;
constexpr reg32 kSunAcross = 0x55d20c, kSunAlong = 0x55d210;
// [0x55d158]: no fog colour when not 0.
constexpr reg32 kNoFog = 0x55d158, kCloudOther = 0x55d15c, kCloudFloat = 0x55d1c8;
constexpr reg32 kCloudTurn = 0x55d1f0, kCloudTurnFloat = 0x55d1cc;
constexpr sreg32 kDegreesTurn = 360;
constexpr reg32 kCloudLast = 0x55d160;
constexpr reg32 kCloudFloats[] = {0x55d164, 0x55d168, 0x55d16c, 0x55d170, 0x55d174};
// The colours read as bytes: the fog's, the top's and the bottom's (blue, green, red).
constexpr reg32 kColourBytes[] = {0x55d1c2, 0x55d1c1, 0x55d1c0, 0x55d1c6, 0x55d1c5, 0x55d1c4,
                                  0x55d17a, 0x55d179, 0x55d178};
// The weather's: three halves, four running counts, three dwords after.
constexpr reg32 kWeatherHalves[] = {0x8ca1ea, 0x8ca1ec, 0x8ca1ee};
constexpr reg32 kWeatherCounts = 0x8ca1f0;
constexpr reg32 kWeatherWords[] = {0x8ca1f0, 0x8ca1f4, 0x8ca1f8, 0x8ca1fc, 0x55d148, 0x55d14c};
constexpr reg32 kWeatherLast = 0x55d150;
// The sun's direction (three floats, the middle one a half) and the 16.16 angle's scale.
constexpr reg32 kSunDirection = 0x55d2a8;
constexpr reg32 kHalfBits = 0x3f000000;
// The sky's bands: 8, from the top's colour to the bottom's, each with its mirror (16 less it),
// four bytes each from 0x55d178; the band at 0x55d17c copied to 0x55d1bc; the fog's colour.
constexpr reg32 kBandCount = 8, kBands = 0x55d178, kBandMirror = 16, kBandCopy = 0x55d1bc;
constexpr reg32 kFogColour = 0x6ff9f8;

void putFloat(App* app, reg32 address, float value)
{
    app->getMemory<float>(address) = value;
}

float floatAt(App* app, reg32 address)
{
    return app->getMemory<float>(address);
}

reg32 bitsOf(float value)
{
    reg32 bits;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}

/* The horizon's next number, its text's place at `text` (moved on past it). */
std::optional<reg32> readNumber(App* app, CPU& cpu, reg32 text)
{
    return feCore<0x4be020, frontend::loopb::readNumber>(app, cpu, text);
}

/* __CHP: `value` to a whole value, towards 0; never nullopt (the optional is the caller's stop
 * signal). */
std::optional<x86::Float> chop(App* app, CPU& cpu, x86::Float value)
{
    return x86::Float(guest::direct<kChop, watcom::clib::chop>(app, cpu, double(value)));
}


/* A 16.16 colour channel's whole part, towards 0 (sar, sbb, sar). */
reg8 channel(reg32 value)
{
    return reg8(sreg32(value) / 0x10000);
}

/* The sky's horizon record `index`'s place: [0x55d278] read again each time, as the code does. */
reg32 recordAt(App* app, reg32 index)
{
    return at(app, kRecords) + index * kRecordBytes;
}

/* The track's sky, sky.fsh in its folder: its clouds' shape (palette to 0x55d21c), the sun's (to
 * 0x6ff9a0, 4 of its colours with the options' smaller textures, else 7) and the sun's glow
 * placed (0x47a970) -- CLD/CHR/SUN with N at night, W in the weather, D by day -- [0x563a90] the
 * sun's palette, the file freed and the sky made (0x47ae10).  False when the game stopped. */
bool loadSkyShapes(App* app, CPU& cpu, reg32 track)
{
    const GuestFrame frame(cpu, 0x18, 0x100);
    const reg32 name = frame - 0x100;  // handed to sprintf and the loading
    if (!format(app, cpu, {name, kSkyName, kDataPath, track}))
        return false;
    const std::optional<reg32> file = loadFile(app, cpu, name, 0);
    if (!file)
        return false;
    if (*file == 0)
        return true;
    if (!loadingTick(app, cpu))
        return false;
    const reg32 sunColours = (byte(app, kOptions) & kSmallTextures) != 0 ? 4 : 7;
    // The shapes' names: clouds, sun, glow.
    constexpr reg32 names[4][3] = {{0x53b354, 0x53b35c, 0x53b364},   // night, weather
                                   {0x53b36c, 0x53b374, 0x53b37c},   // night
                                   {0x53b384, 0x53b38c, 0x53b394},   // weather
                                   {0x53b39c, 0x53b3a4, 0x53b3ac}};  // day
    const reg32* shapes = names[(gameSettings(app).night != 0 ? 0 : 2) + (gameSettings(app).weather != 0 ? 0 : 1)];
    if (!feCore<0x4d4080, frontend::rende::loadShapeTexture>(app, cpu, *file, shapes[0], 0x40u, kCloudPalette, 7u))
        return false;
    if (!feCore<0x4d4080, frontend::rende::loadShapeTexture>(app, cpu, *file, shapes[1], 0x80u, kSunPalette, sunColours))
        return false;
    if (!feCore<0x47a970, frontend::wata::skyLoad>(app, cpu, *file, shapes[2]))
        return false;
    put(app, kSunShapes, kSunPalette);
    if (!loadingTick(app, cpu) || !release(app, cpu, *file))
        return false;
    return feCore<0x47ae10, frontend::renda::cloudGrid>(app, cpu) && loadingTick(app, cpu);
}

/* The track's horizon, 3Trnn.hrz (with N at night, W in the weather) in its folder, read number
 * by number (0x4be020) into the sky's: its colours, the fog's (1 to 100, a hundredth), its
 * records (36 bytes each, allocated to [0x55d278]), the sun's and the clouds' places and sizes,
 * the weather's counts (0x8ca1e8 on); the file freed, the sky loaded (loadSkyShapes), the sun's
 * direction worked out and its length taken, and the sky's 8 bands of colours, top and bottom,
 * from its two colours; the fog's colour from the bottom one, or none.  What the code leaves in
 * eax: the file's loading's 0, or the last tick's with the fog colour's last byte in it. */
std::optional<reg32> readHorizon(App* app, CPU& cpu, reg32 track)
{
    const GuestFrame frame(cpu, 0x18, 0x144);
    const reg32 name = frame - 0x144, text = frame - 0x34;  // handed to sprintf, the loading, the reading
    put(app, text, 0);
    const bool night = gameSettings(app).night != 0, weather = gameSettings(app).weather != 0;
    const reg32 nameFormat = night ? (weather ? kHorizonNightWeather : kHorizonNight)
                                   : (weather ? kHorizonWeather : kHorizonName);
    if (!format(app, cpu, {name, nameFormat, kDataPath, track, track}) || !loadingTick(app, cpu))
        return std::nullopt;
    const std::optional<reg32> file = loadFile(app, cpu, name, 0x10);
    if (!file)
        return std::nullopt;
    if (*file == 0)
        return 0;
    put(app, text, *file);
    std::optional<reg32> number;
    const auto next = [&] { return (number = readNumber(app, cpu, text)).has_value(); };

    // The sky's colour, its three bytes, and its fourth's dword.
    for (const reg32 to : {kSkyColour + 2, kSkyColour + 1, kSkyColour})
    {
        if (!next())
            return std::nullopt;
        putByte(app, to, reg8(*number));
    }
    if (!next())
        return std::nullopt;
    put(app, kSkyColourWord, *number);
    // The fog: 1 to 100, a hundredth of it.
    if (!next())
        return std::nullopt;
    const sreg32 fog = sreg32(*number) > kFogFarthest ? kFogFarthest
                       : sreg32(*number) < kFogNearest ? kFogNearest
                                                         : sreg32(*number);
    putFloat(app, kFogScale, float(fp::mul(x86::Float(fog), x86::Float(floatAt(app, kHundredth)))));
    // The records.
    if (!next())
        return std::nullopt;
    put(app, kRecordCount, *number);
    if (*number == 0)
        put(app, kRecords, 0);
    else
    {
        const std::optional<reg32> records = allocate(app, cpu, kNoName, *number * kRecordBytes, 0);
        if (!records)
            return std::nullopt;
        put(app, kRecords, *records);
        for (reg32 index = 0; sreg32(index) < sreg32(at(app, kRecordCount)); ++index)
        {
            // Its bytes and dwords, in the file's order.
            constexpr struct { reg32 offset; bool dword; } fields[] = {
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
                    putByte(app, recordAt(app, index) + field.offset, reg8(*number));
            }
        }
    }
    // The sun's and the clouds'.
    for (const reg32 to : kSunNumbers)
    {
        if (!next())
            return std::nullopt;
        put(app, to, *number);
    }
    if (!next())
        return std::nullopt;
    const float sunSize = float(x86::Float(sreg32(*number)));
    putFloat(app, kSunSize, sunSize);
    if (sunSize != 0.0f)  // its bits but the sign's: a whole number's float, never a NaN
        putFloat(app, kSunSize, float(fp::div(x86::Float(1.0), x86::Float(sunSize))));
    if (!next())
        return std::nullopt;
    putFloat(app, kSunFloat, float(x86::Float(sreg32(*number))));
    if (!next())
        return std::nullopt;
    putFloat(app, kSunTripled, float(x86::Float(sreg32(*number * 3))));
    if (!next())
        return std::nullopt;
    put(app, kSunNumber, *number);
    if (!next())
        return std::nullopt;
    put(app, kSunOther, *number);
    if (!next())
        return std::nullopt;
    const std::optional<x86::Float> height =
        chop(app, cpu, fp::mul(x86::Float(sreg32(*number)), x86::Float(app->getMemory<double>(kHeightScale))));
    if (!height)
        return std::nullopt;
    put(app, kSunHeight, reg32(fp::toInteger<sreg32>(*height)));
    // The sun's angle and distance: its place on the sky.
    if (!next())
        return std::nullopt;
    const reg32 distanceNumber = *number;
    if (!next())
        return std::nullopt;
    const float angle =
        float(fp::mul(x86::Float(sreg32(*number)), x86::Float(app->getMemory<double>(kSunAngleScale))));
    const float distance =
        float(fp::mul(x86::Float(sreg32(distanceNumber)), x86::Float(app->getMemory<double>(kSunDistanceScale))));
    const std::optional<double> cosine = feCore<0x4ea950, frontend::simd::cosineValue>(app, cpu, bitsOf(angle));
    if (!cosine)
        return std::nullopt;
    putFloat(app, kSunAcross, float(fp::mul(*cosine, x86::Float(distance))));
    const std::optional<double> sine = feCore<0x4ea930, frontend::simd::sineValue>(app, cpu, bitsOf(angle));
    if (!sine)
        return std::nullopt;
    putFloat(app, kSunAlong, float(fp::mul(*sine, x86::Float(distance))));
    if (!next())
        return std::nullopt;
    put(app, kNoFog, *number);
    if (!next())
        return std::nullopt;
    put(app, kCloudOther, *number);
    if (!next())
        return std::nullopt;
    putFloat(app, kCloudFloat, float(x86::Float(sreg32(*number))));
    // The clouds' turn: degrees made 16.16 turns, then within 0 and 1.
    if (!next())
        return std::nullopt;
    const std::int64_t turnDividend = sreg32(*number << 16);
    const reg32 cloudTurn = reg32(turnDividend / kDegreesTurn);
    put(app, kCloudTurn, cloudTurn);
    float turn = float(fp::mul(x86::Float(sreg32(cloudTurn)), x86::Float(floatAt(app, kTurnScale))));
    // fcom with 0, jbe: on while 0 is above it, not for a NaN -- a whole number times a constant
    // and 1 added, never one; then its bits over 1.0's: a float over 1.0, never infinite.
    for (;;)
    {
        putFloat(app, kCloudTurnFloat, turn);
        if (!(turn < 0.0f))
            break;
        turn = float(fp::add(x86::Float(1.0), x86::Float(turn)));
    }
    while (turn > 1.0f)
    {
        turn = float(fp::add(x86::Float(turn), x86::Float(floatAt(app, kMinusOne))));
        putFloat(app, kCloudTurnFloat, turn);
    }
    if (!next())
        return std::nullopt;
    put(app, kCloudLast, *number);
    for (const reg32 to : kCloudFloats)
    {
        if (!next())
            return std::nullopt;
        putFloat(app, to, float(x86::Float(sreg32(*number))));
    }
    // The sky's colours: the fog's, the top's, the bottom's.
    for (const reg32 to : kColourBytes)
    {
        if (!next())
            return std::nullopt;
        putByte(app, to, reg8(*number));
    }
    // The bands' two colours, top and bottom: red, green, blue.
    reg8 top[3], bottom[3];
    for (reg8* channelTo : {&top[0], &top[1], &top[2], &bottom[0], &bottom[1], &bottom[2]})
    {
        if (!next())
            return std::nullopt;
        *channelTo = reg8(*number);
    }
    // The weather's.
    if (!next())
        return std::nullopt;
    putByte(app, kWeatherCount, reg8(*number));
    for (const reg32 to : kWeatherHalves)
    {
        if (!next())
            return std::nullopt;
        app->getMemory<reg16>(to) = reg16(*number);
    }
    for (const reg32 to : kWeatherWords)
    {
        if (!next())
            return std::nullopt;
        put(app, to, *number);
    }
    if (!next())
        return std::nullopt;
    // The four counts made running sums.
    const reg32 second = at(app, kWeatherCounts + 4);
    put(app, kWeatherLast, *number);
    const reg32 upToSecond = second + at(app, kWeatherCounts);
    put(app, kWeatherCounts + 4, upToSecond);
    const reg32 upToThird = at(app, kWeatherCounts + 8) + upToSecond;
    put(app, kWeatherCounts + 8, upToThird);
    put(app, kWeatherCounts + 0xc, at(app, kWeatherCounts + 0xc) + upToThird);
    if (!release(app, cpu, *file))
        return std::nullopt;
    {
        const GuestFrame slot(cpu, 0, 4);  // the call's return address
        if (!loadSkyShapes(app, cpu, track))
            return std::nullopt;
    }
    if (!loadingTick(app, cpu))
        return std::nullopt;
    // The sun's direction, 16.16 sine and cosine of the clouds' turn.
    const reg32 fixedSine = guest::direct<0x4f2380, sineOf>(app, cpu, at(app, kCloudTurn));
    if (cpu.terminate)
        return std::nullopt;
    const float across =
        float(fp::mul(x86::Float(sreg32(fixedSine)), x86::Float(app->getMemory<double>(kFixedScale))));
    put(app, kSunDirection + 4, kHalfBits);
    putFloat(app, kSunDirection, across);
    const reg32 fixedCosine = guest::direct<0x4f23f0, cosineOf>(app, cpu, at(app, kCloudTurn));
    if (cpu.terminate)
        return std::nullopt;
    putFloat(app, kSunDirection + 8,
             float(fp::mul(x86::Float(sreg32(fixedCosine)), x86::Float(app->getMemory<double>(kFixedScale)))));
    // Its length, dropped.
    guest::direct<0x4972f0, vertices::normaliseVector>(app, cpu, kSunDirection);
    if (cpu.terminate)
        return std::nullopt;
    if (!loadingTick(app, cpu))
        return std::nullopt;
    // The sky's 8 bands: from the top's colour to the bottom's in steps of an eighth, 16.16, the
    // band and its mirror (16 less it) each.
    reg32 colour[3], step[3];
    for (int i = 0; i < 3; ++i)
    {
        step[i] = (reg32(bottom[i]) - reg32(top[i])) << 13;
        colour[i] = reg32(top[i]) << 16;
    }
    for (reg32 band = 0; band < kBandCount; ++band)
    {
        const reg8 red = channel(colour[0]), green = channel(colour[1]), blue = channel(colour[2]);
        putByte(app, kBands + band * 4 + 6, red);
        putByte(app, kBands + band * 4 + 5, green);
        putByte(app, kBands + band * 4 + 4, blue);
        const reg32 mirror = kBandMirror - band;
        putByte(app, kBands + mirror * 4 + 2, red);
        putByte(app, kBands + mirror * 4 + 1, green);
        putByte(app, kBands + mirror * 4, blue);
        for (int i = 0; i < 3; ++i)
            colour[i] += step[i];
    }
    put(app, kBandCopy, at(app, kBands + 4));
    const std::optional<reg32> ticked = loadingStop(app, cpu);
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
    const reg8 lastRed = byte(app, kBands);
    putByte(app, kFogColour, lastRed);
    return (*ticked & 0xffffff00) | lastRed;
}

/* The weather's shapes, weather.fsh: as many raindrops' (DRPn, to 0x8ca200) and splashes'
 * (SPLn, to 0x8ca288) as the horizon's count ([0x8ca1e8]) says, by 0x4d4080 -- none for 0; the
 * file freed.  False when the game stopped. */
bool loadWeather(App* app, CPU& cpu)
{
    const GuestFrame frame(cpu, 0x18, 0x144);
    const reg32 name = frame - 0x144, shape = frame - 0x44;  // handed to sprintf, the loading, the shapes
    // The file's block, kept in the frame right after the shape's name and read back from there:
    // a name of 0x40 bytes or more runs into it.
    const reg32 kept = frame - 4;
    if (!format(app, cpu, {name, kWeatherName, kWeatherPath}))
        return false;
    const std::optional<reg32> file = loadFile(app, cpu, name, 0);
    if (!file)
        return false;
    put(app, kept, *file);
    const reg32 count = byte(app, kWeatherCount);
    if (count != 0)
        for (const auto& kind : {std::pair<reg32, reg32>{kDropName, kDropShapes}, {kSplashName, kSplashShapes}})
        {
            if (!format(app, cpu, {shape, kind.first, count}))
                return false;
                    if (!feCore<0x4d4080, frontend::rende::loadShapeTexture>(app, cpu, *file, shape, 0x20u, kind.second, 0u))
                return false;
        }
    const reg32 block = at(app, kept);
    return block == 0 || release(app, cpu, block);
}

}  // namespace

/* 0x478340: readHorizon for the track eax; eax as its code leaves it. */
x86::reg32 loadHorizon(App* app, CPU& cpu, x86::Args& args)
{
    const Entry entry(args);
    const std::optional<reg32> left = readHorizon(app, cpu, args.eax);
    if (!left)
        return args.eax;
    args.eax = *left;
    args.ebx = entry.ebx;
    args.ecx = entry.ecx;
    args.edx = entry.edx;
    args.esi = entry.esi;
    args.edi = entry.edi;
    cpu.esp += 4;
    return args.eax;
}

/* 0x47ac30: loadSkyShapes for the track eax; eax as the last call leaves it. */
x86::reg32 loadSky(App* app, CPU& cpu, x86::Args& args)
{
    const Entry entry(args);
    if (!loadSkyShapes(app, cpu, args.eax))
        return args.eax;
    args.ebx = entry.ebx;
    args.ecx = entry.ecx;
    args.edx = entry.edx;
    args.esi = entry.esi;
    args.edi = entry.edi;
    cpu.esp += 4;
    return args.eax;
}

/* 0x4dcd20: loadWeather; eax as the last call leaves it.  The registers the code pushes are
 * kept on the guest's stack where it keeps them, and taken back from there: a shape's name of
 * 0x40 bytes or more runs over the file's block into them. */
x86::reg32 loadWeatherShapes(App* app, CPU& cpu, x86::Args& args)
{
    const reg32 pushed = cpu.esp;
    put(app, pushed - 4, args.ebx);
    put(app, pushed - 8, args.ecx);
    put(app, pushed - 0xc, args.edx);
    put(app, pushed - 0x10, args.esi);
    put(app, pushed - 0x14, args.edi);
    put(app, pushed - 0x18, args.ebp);
    if (!loadWeather(app, cpu))
        return args.eax;
    args.ebx = at(app, pushed - 4);
    args.ecx = at(app, pushed - 8);
    args.edx = at(app, pushed - 0xc);
    args.esi = at(app, pushed - 0x10);
    args.edi = at(app, pushed - 0x14);
    args.ebp = at(app, pushed - 0x18);
    cpu.esp += 4;
    return args.eax;
}

/* The typed cores of the thunks above, for callers that call them as C++ (native_load_track.h). */
bool trackTexturesLoaded(App* app, CPU& cpu)
{
    return loadTextures(app, cpu);
}

bool ghostSaved(App* app, CPU& cpu)
{
    return saveRacedGhost(app, cpu);
}

std::optional<reg32> horizonRead(App* app, CPU& cpu, reg32 track)
{
    return readHorizon(app, cpu, track);
}

bool weatherLoaded(App* app, CPU& cpu)
{
    return loadWeather(app, cpu);
}

}  // namespace load
}  // namespace nfs3hp
