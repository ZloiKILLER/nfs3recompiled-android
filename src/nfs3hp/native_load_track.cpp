#include "native_guest.h"

#include <utility>

/* The game's data: the race's data files -- the track's paths, its textures,
 * horizon, sky and weather, the opponents' speed tables, the cameras, the
 * ghost -- and the file cache the front end keeps its own data in.
 * disassemble_nfs3hp.py binds each address to the function of that name here
 * (LOAD_TRACK_NATIVES), and the generated code calls it by name.
 * tools/game_checks holds each against its generated code.
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
 * Written as their code is: each push and pop to the guest's stack, each call
 * with the return address's slot, the frame's locals in the guest's memory,
 * the registers as the code leaves them; the x87's stack pushed and popped as
 * the code has it, every operation through cpu.fpu.  Every other function of
 * the game's, and those of this file other files call (they have stand-ins),
 * are called by address. */
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

// The file cache's: its magic, its blocks' names.
constexpr reg32 kBigMagic = 0x42494746;  // 'FGIB': "BIGF" read as a dword
constexpr reg32 kNodeName = 0x536c38, kDataName = 0x536c44, kCacheName = 0x536c50, kBufferName = 0x536c54;
// The track's: the data path, the track, the paths' buffers and formats.
constexpr reg32 kDataPath = 0x7a27b4, kTrack = 0x6fd4ac;
constexpr reg32 kTrackFile = 0x5dd86c, kTrackFileFormat = 0x536cec;
constexpr reg32 kFolderFile = 0x5dd8ec, kFolderFileFormat = 0x536d00;
// The race: backwards, the track's own textures, the night, the weather; the
// options' byte (2: the textures' smaller set); the loading stopped.
constexpr reg32 kBackwards = 0x6fd4c0, kOwnTextures = 0x6fd4c4, kNight = 0x6fd4c8, kWeather = 0x6fd4cc;
constexpr reg32 kOptions = 0x7a3a58, kLoadAborted = 0x55e88c;
// The opponents' speeds: speedsR.bin, speedsF.bin; the table, its second
// half, the failure's flag; the track's nodes.
constexpr reg32 kSpeedsBackward = 0x53679c, kSpeedsForward = 0x5367a8;
constexpr reg32 kSpeeds = 0x552558, kSpeedsHalf = 0x55255c, kSpeedsFailed = 0x56f22c, kTrackNodes = 0x5dd940;
// The cameras: the views (24 bytes each), the .can files loaded, the names.
constexpr reg32 kCameraViews = 0x56f234, kCameraFiles = 0x570a30;
constexpr reg32 kFirstCameraName = 0x5367fc, kCameraName = 0x536808;
// The track's textures: the shapes loaded, the .qfs files' names, the trace.
constexpr reg32 kTrackShapes = 0x5dd850;
constexpr reg32 kNear8 = 0x536d20, kNear = 0x536d28, kFar8 = 0x536d58, kFar = 0x536d60;
constexpr reg32 kTraceFile = 0x536d30, kTraceFormat = 0x536d50;
// The ghost: replaying, saving one; its file's name; its record, its records'
// count, its records; ghost read.
constexpr reg32 kReplaying = 0x6fd3a0, kGhostWanted = 0x6fd3ac, kGhostName = 0x53b0c8;
constexpr reg32 kGhost = 0x6ba5c4, kGhostCount = 0x6bc6b4, kGhostRecords = 0x6bc6b8, kGhostRead = 0x55d000;
constexpr reg32 kGhostBytes = 0x20f4;
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
// What is called by address.
constexpr reg32 kAllocate = 0x4e1620, kFree = 0x4e1890, kMemset = 0x4e0640, kStricmp = 0x4e9ec0;
constexpr reg32 kBigCount = 0x4ea440, kBigName = 0x4ea4a0, kBigEntry = 0x4ea290;
constexpr reg32 kSprintf = 0x4df690, kFileExists = 0x4e0e60, kLoadFile = 0x4e0ec0, kLoadingTick = 0x4a33d0;
constexpr reg32 kTrace = 0x42d500, kLoadShapes = 0x4956f0, kSaveGhostFile = 0x4777d0;
constexpr reg32 kFseek = 0x4e9088, kFwrite = 0x4eebd0, kReadHandle = 0x4e2de0;
constexpr reg32 kReadNumber = 0x4be020, kChop = 0x4dfd56, kCosine = 0x4ea950, kSine = 0x4ea930;
constexpr reg32 kFixedSine = 0x4f2380, kFixedCosine = 0x4f23f0, kVectorLength = 0x4972f0;
constexpr reg32 kFindShape = 0x4d4080, kPlaceSun = 0x47a970, kSkyMade = 0x47ae10;

void push(win32::WinApplication* app, x86::CPU& cpu, reg32 value)
{
    cpu.esp -= 4;
    put(app, cpu.esp, value);
}

reg32 pop(win32::WinApplication* app, x86::CPU& cpu)
{
    const reg32 value = at(app, cpu.esp);
    cpu.esp += 4;
    return value;
}

/* A call as the code makes it: the return address's slot, and esp as the
 * callee leaves it.  False when the game stopped in it. */
bool direct(win32::WinApplication* app, x86::CPU& cpu, void (*function)(win32::WinApplication*, x86::CPU&))
{
    cpu.esp -= 4;
    function(app, cpu);
    return !cpu.terminate;
}

bool byAddress(win32::WinApplication* app, x86::CPU& cpu, reg32 target)
{
    cpu.esp -= 4;
    app->dynamic_call(target, cpu);
    return !cpu.terminate;
}

void putHalf(win32::WinApplication* app, reg32 address, reg16 value)
{
    app->getMemory<reg16>(address) = value;
}

x86::Float f32(win32::WinApplication* app, reg32 address)
{
    return x86::Float(app->getMemory<float>(address));
}

x86::Float f64(win32::WinApplication* app, reg32 address)
{
    return x86::Float(app->getMemory<double>(address));
}

x86::Float i32(win32::WinApplication* app, reg32 address)
{
    return x86::Float(sreg32(at(app, address)));
}

void putF32(win32::WinApplication* app, reg32 address, const x86::Float& value)
{
    app->getMemory<float>(address) = float(value);
}

/* push edi; mov eax, ecx; shr ecx, 2; rep movsd; mov cl, al; and cl, 3;
 * rep movsb; pop edi: ecx bytes from [esi] to [edi], esi past them, eax the
 * count, ecx 0, edi as it was. */
void copyBytes(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.edi);
    cpu.eax = cpu.ecx;
    cpu.ecx >>= 2;
    for (; cpu.ecx != 0; --cpu.ecx)
    {
        put(app, cpu.edi, at(app, cpu.esi));
        cpu.edi += 4;
        cpu.esi += 4;
    }
    cpu.cl = cpu.al & 3;
    for (; cpu.ecx != 0; --cpu.ecx)
        putByte(app, cpu.edi++, byte(app, cpu.esi++));
    cpu.edi = pop(app, cpu);
}

/* The code's inlined strcpy: push edi, two bytes a turn from [esi] to [edi]
 * up to the ending, pop edi -- esi past the last pair started, al the ending. */
void copyString(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.edi);
    for (;;)
    {
        cpu.al = byte(app, cpu.esi);
        putByte(app, cpu.edi, cpu.al);
        if (cpu.al == 0)
            break;
        cpu.al = byte(app, cpu.esi + 1);
        cpu.esi += 2;
        putByte(app, cpu.edi + 1, cpu.al);
        cpu.edi += 2;
        if (cpu.al == 0)
            break;
    }
    cpu.edi = pop(app, cpu);
}

/* sub ecx, ecx; dec ecx; xor eax, eax; repne scasb; not ecx; dec ecx: the
 * string at edi's length in ecx, eax 0, edi past its ending. */
void stringLength(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.ecx = 0xffffffff;
    cpu.eax = 0;
    do
    {
        --cpu.ecx;
    } while (byte(app, cpu.edi++) != cpu.al && cpu.ecx != 0);
    cpu.ecx = ~cpu.ecx;
    --cpu.ecx;
}

}  // namespace

void loadOpponentSpeeds(win32::WinApplication* app, x86::CPU& cpu);
void clearTrackCameras(win32::WinApplication* app, x86::CPU& cpu);
void loadTrackCameras(win32::WinApplication* app, x86::CPU& cpu);
void swapBigEndian(win32::WinApplication* app, x86::CPU& cpu);
void newFileNode(win32::WinApplication* app, x86::CPU& cpu);
void freeFileNode(win32::WinApplication* app, x86::CPU& cpu);
void findFileNode(win32::WinApplication* app, x86::CPU& cpu);
void insertFileNode(win32::WinApplication* app, x86::CPU& cpu);
void newEmptyFileNode(win32::WinApplication* app, x86::CPU& cpu);
void freeFileNodes(win32::WinApplication* app, x86::CPU& cpu);
void countFileNodes(win32::WinApplication* app, x86::CPU& cpu);
void findCachedFile(win32::WinApplication* app, x86::CPU& cpu);
void addCachedFile(win32::WinApplication* app, x86::CPU& cpu);
void newFileCache(win32::WinApplication* app, x86::CPU& cpu);
void freeFileCache(win32::WinApplication* app, x86::CPU& cpu);
void bigHeaderSize(win32::WinApplication* app, x86::CPU& cpu);
void cachedDataSize(win32::WinApplication* app, x86::CPU& cpu);
void writeBigFile(win32::WinApplication* app, x86::CPU& cpu);
void packFileCache(win32::WinApplication* app, x86::CPU& cpu);
void readBigFile(win32::WinApplication* app, x86::CPU& cpu);
void openFileCache(win32::WinApplication* app, x86::CPU& cpu);
void trackFilePath(win32::WinApplication* app, x86::CPU& cpu);
void trackFolderPath(win32::WinApplication* app, x86::CPU& cpu);
void loadTrackTextures(win32::WinApplication* app, x86::CPU& cpu);
void saveGhost(win32::WinApplication* app, x86::CPU& cpu);
void writeGhost(win32::WinApplication* app, x86::CPU& cpu);
void readGhost(win32::WinApplication* app, x86::CPU& cpu);
void loadHorizon(win32::WinApplication* app, x86::CPU& cpu);
void loadSky(win32::WinApplication* app, x86::CPU& cpu);
void loadWeatherShapes(win32::WinApplication* app, x86::CPU& cpu);

/* 0x40e000: the opponents' speed table, speedsF.bin (speedsR.bin backwards)
 * in the track's folder, loaded to [0x552558] if it is there, [0x55255c] its
 * second half (half the track's nodes, rounded up, on) and [0x56f22c] 0; else
 * [0x552558] 0 and [0x56f22c] 1. */
void loadOpponentSpeeds(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.eax = at(app, kBackwards) != 0 ? kSpeedsBackward : kSpeedsForward;
    if (!direct(app, cpu, trackFolderPath))
        return;
    cpu.ecx = cpu.eax;
    cpu.eax = cpu.ecx;
    if (!byAddress(app, cpu, kFileExists))
        return;
    bool loaded = false;
    if (cpu.eax != 0)
    {
        cpu.edx = 0x10;
        cpu.eax = cpu.ecx;
        if (!byAddress(app, cpu, kLoadFile))
            return;
        cpu.ecx = cpu.eax;
        if (cpu.eax != 0)
        {
            cpu.edx = at(app, kTrackNodes);
            ++cpu.edx;
            put(app, kSpeeds, cpu.eax);
            cpu.eax = cpu.edx;
            cpu.edx = reg32(sreg32(cpu.edx) >> 31);
            cpu.eax -= cpu.edx;
            cpu.eax = reg32(sreg32(cpu.eax) >> 1);
            cpu.ecx += cpu.eax;
            put(app, kSpeedsHalf, cpu.ecx);
            cpu.ecx = 0;
            put(app, kSpeedsFailed, cpu.ecx);
            loaded = true;
        }
    }
    if (!loaded)
    {
        cpu.esi = 0;
        cpu.ebx = 1;
        put(app, kSpeeds, cpu.esi);
        put(app, kSpeedsFailed, cpu.ebx);
    }
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40f230: the cameras' 256 views (24 bytes from 0x56f234) cleared -- 0, -1,
 * 0 -- the loading bar ticked (0x4a33d0) before each. */
void clearTrackCameras(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = 0;
    do
    {
        if (!byAddress(app, cpu, kLoadingTick))
            return;
        cpu.ecx = cpu.edx;
        cpu.eax = cpu.edx * 4;
        cpu.eax -= cpu.edx;
        cpu.ecx ^= cpu.edx;
        cpu.ebx = 0xffffffff;
        put(app, cpu.eax * 8 + kCameraViews, cpu.ecx);
        cpu.esi = 0;
        put(app, cpu.eax * 8 + kCameraViews + 4, cpu.ebx);
        ++cpu.edx;
        put(app, cpu.eax * 8 + kCameraViews + 8, cpu.esi);
    } while (sreg32(cpu.edx) < 0x100);
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40f280: the track's cameras, its path's start eax: the views cleared
 * (0x40f230), the 7 files' slots at 0x570a34 on cleared, then -- unless the
 * loading was stopped ([0x55e88c]: 0 back) -- "<eax>00a.can", "<eax>01.can",
 * "<eax>02.can"... loaded into them while they are there, up to 7.  How many
 * back. */
void loadTrackCameras(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x100;
    const reg32 name = cpu.ebp - 0x100;
    cpu.ebx = cpu.eax;
    if (!direct(app, cpu, clearTrackCameras))
        return;
    cpu.ecx = 0;
    ++cpu.ecx;
    cpu.edx = 0;
    if (!byAddress(app, cpu, kLoadingTick))
        return;
    put(app, cpu.ecx * 4 + kCameraFiles, cpu.edx);
    while (sreg32(cpu.ecx) < 7)
    {
        ++cpu.ecx;
        cpu.edx = 0;
        if (!byAddress(app, cpu, kLoadingTick))
            return;
        put(app, cpu.ecx * 4 + kCameraFiles, cpu.edx);
    }
    cpu.esi = at(app, kLoadAborted);
    cpu.ecx = 0;
    if (cpu.esi != 0)
    {
        cpu.eax = 0;
    }
    else
    {
        push(app, cpu, cpu.ecx);
        push(app, cpu, cpu.ebx);
        push(app, cpu, kFirstCameraName);
        for (;;)
        {
            cpu.eax = name;
            push(app, cpu, cpu.eax);
            if (!byAddress(app, cpu, kSprintf))
                return;
            cpu.esp += 0x10;
            if (sreg32(cpu.ecx) >= 7)
                break;
            cpu.eax = name;
            if (!byAddress(app, cpu, kFileExists))
                return;
            if (cpu.eax == 0)
                break;
            if (!byAddress(app, cpu, kLoadingTick))
                return;
            cpu.eax = name;
            cpu.edx = 0;
            ++cpu.ecx;
            if (!byAddress(app, cpu, kLoadFile))
                return;
            push(app, cpu, cpu.ecx);
            push(app, cpu, cpu.ebx);
            push(app, cpu, kCameraName);
            put(app, cpu.ecx * 4 + kCameraFiles, cpu.eax);
        }
        cpu.eax = cpu.ecx;
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x418d00: the dword at eax turned around in place, a BIGF archive's
 * big-endian number; it back in eax. */
void swapBigEndian(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    cpu.edx = cpu.eax;
    cpu.ecx = cpu.eax;
    cpu.eax = 0;
    do
    {
        cpu.esi = 3;
        cpu.ebx = cpu.ecx + cpu.eax;
        cpu.esi -= cpu.eax;
        cpu.bl = byte(app, cpu.ebx);
        ++cpu.eax;
        putByte(app, cpu.esi + cpu.ebp - 4, cpu.bl);
    } while (sreg32(cpu.eax) < 4);
    cpu.eax = at(app, cpu.ebp - 4);
    put(app, cpu.edx, cpu.eax);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x418d40: a file cache's node, "FileNode" (0x30 bytes), allocated and
 * cleared -- cleared at 0 too if there is none. */
void newFileNode(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ebx = 0x10;
    cpu.edx = 0x30;
    cpu.eax = kNodeName;
    if (!byAddress(app, cpu, kAllocate))
        return;
    cpu.ebx = 0x30;
    cpu.edx = 0;
    cpu.ecx = cpu.eax;
    if (!byAddress(app, cpu, kMemset))
        return;
    cpu.eax = cpu.ecx;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x418d70: the node eax freed, its file's data first (if any); its size,
 * its data and its name's first byte cleared before. */
void freeFileNode(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = cpu.eax;
    if (cpu.eax != 0)
    {
        put(app, cpu.eax + 0x20, 0);
        cpu.ecx = at(app, cpu.eax + 0x24);
        putByte(app, cpu.eax, 0);
        if (cpu.ecx != 0)
        {
            cpu.eax = cpu.ecx;
            if (!byAddress(app, cpu, kFree))
                return;
            put(app, cpu.edx + 0x24, 0);
        }
        cpu.eax = cpu.edx;
        if (!byAddress(app, cpu, kFree))
            return;
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x418dd0: the node named eax (stricmp, 0x4e9ec0) of the list after the
 * node edx, or 0. */
void findFileNode(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ebx = cpu.eax;
    cpu.ecx = at(app, cpu.edx + 0x28);
    while (cpu.ecx != 0)
    {
        cpu.edx = cpu.ecx;
        cpu.eax = cpu.ebx;
        if (!byAddress(app, cpu, kStricmp))
            return;
        if (cpu.eax == 0)
            break;
        cpu.ecx = at(app, cpu.ecx + 0x28);
    }
    cpu.eax = cpu.ecx;
    cpu.ebp = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x418e90: a node made for the file named eax, of edx bytes at ebx, and
 * linked in after the node ecx; it back, or 0 (ebx and edx it too). */
void insertFileNode(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 8;
    cpu.esi = cpu.eax;
    put(app, cpu.ebp - 8, cpu.edx);
    put(app, cpu.ebp - 4, cpu.ebx);
    if (!direct(app, cpu, newFileNode))
        return;
    cpu.edx = cpu.eax;
    cpu.ebx = cpu.eax;
    if (cpu.eax != 0)
    {
        cpu.edi = cpu.eax;
        copyString(app, cpu);
        cpu.eax = at(app, cpu.ebp - 8);
        put(app, cpu.edx + 0x20, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 4);
        put(app, cpu.edx + 0x24, cpu.eax);
        cpu.esi = at(app, cpu.ecx + 0x28);
        if (cpu.esi != 0)
            put(app, cpu.esi + 0x2c, cpu.edx);
        cpu.eax = at(app, cpu.ecx + 0x28);
        put(app, cpu.ebx + 0x28, cpu.eax);
        put(app, cpu.ebx + 0x2c, cpu.ecx);
        put(app, cpu.ecx + 0x28, cpu.ebx);
    }
    cpu.eax = cpu.ebx;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x418f00: an empty node -- no name, size, data or links -- or 0. */
void newEmptyFileNode(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (!direct(app, cpu, newFileNode))
        return;
    cpu.edx = cpu.eax;
    if (cpu.eax != 0)
    {
        put(app, cpu.eax + 0x20, 0);
        put(app, cpu.eax + 0x24, 0);
        put(app, cpu.eax + 0x28, 0);
        put(app, cpu.eax + 0x2c, 0);
        putByte(app, cpu.eax, 0);
    }
    cpu.eax = cpu.edx;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x418f40: the node eax and every node after it freed (0x418d70). */
void freeFileNodes(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (cpu.eax != 0)
    {
        cpu.edx = at(app, cpu.eax + 0x28);
        if (!direct(app, cpu, freeFileNode))
            return;
        while (cpu.edx != 0)
        {
            cpu.eax = cpu.edx;
            cpu.ecx = at(app, cpu.edx + 0x28);
            if (!direct(app, cpu, freeFileNode))
                return;
            cpu.edx = cpu.ecx;
        }
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x418f70: the files in the cache eax: the nodes after its head. */
void countFileNodes(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = cpu.eax;
    cpu.ecx = at(app, cpu.edx + 4);
    cpu.eax = 0;
    if (cpu.ecx != 0)
    {
        cpu.edx = at(app, cpu.ecx + 0x28);
        while (cpu.edx != 0)
        {
            ++cpu.eax;
            cpu.edx = at(app, cpu.edx + 0x28);
        }
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x418fe0: the node of the file named edx in the cache eax (0x418dd0), or
 * 0 for no cache or no list. */
void findCachedFile(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.edx;
    bool found = false;
    if (cpu.eax != 0)
    {
        cpu.edx = at(app, cpu.eax + 4);
        if (cpu.edx != 0)
        {
            cpu.eax = cpu.ecx;
            if (!direct(app, cpu, findFileNode))
                return;
            found = true;
        }
    }
    if (!found)
        cpu.eax = 0;
    cpu.ebp = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x419000: the file named edx, ebx bytes at ecx, copied into a block of its
 * own ("mpFileData") and added at the end of the cache eax (0x418e90).  1
 * back, or 0 for no cache, list or last node, or no block or node. */
void addCachedFile(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0xc;
    const reg32 cache = cpu.ebp - 4, size = cpu.ebp - 8, name = cpu.ebp - 0xc;
    put(app, cache, cpu.eax);
    put(app, name, cpu.edx);
    put(app, size, cpu.ebx);
    cpu.esi = cpu.ecx;
    if (cpu.eax == 0 || at(app, cpu.eax + 4) == 0 || at(app, cpu.eax + 8) == 0)
    {
        cpu.eax = 0;
    }
    else
    {
        cpu.ebx = 0x10;
        cpu.edx = at(app, size);
        cpu.eax = kDataName;
        if (!byAddress(app, cpu, kAllocate))
            return;
        cpu.ebx = cpu.eax;
        if (cpu.eax != 0)
        {
            cpu.ecx = at(app, size);
            cpu.edi = cpu.eax;
            copyBytes(app, cpu);
            cpu.ecx = at(app, cache);
            cpu.edx = at(app, size);
            cpu.eax = at(app, name);
            cpu.ecx = at(app, cpu.ecx + 8);
            if (!direct(app, cpu, insertFileNode))
                return;
            if (cpu.eax != 0)
            {
                cpu.ecx = at(app, cache);
                put(app, cpu.ecx + 8, cpu.eax);
                cpu.eax = 1;
            }
        }
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x419080: a file cache ("BFL"): 'FGIB', its head (an empty node) its
 * list's and its last; 0 if either cannot be had. */
void newFileCache(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ebx = 0x10;
    cpu.edx = 0xc;
    cpu.eax = kCacheName;
    if (!byAddress(app, cpu, kAllocate))
        return;
    cpu.edx = cpu.eax;
    if (cpu.eax != 0)
    {
        put(app, cpu.eax, kBigMagic);
        put(app, cpu.eax + 4, 0);
        put(app, cpu.eax + 8, 0);
        if (!direct(app, cpu, newEmptyFileNode))
            return;
        if (cpu.eax != 0)
        {
            put(app, cpu.edx + 8, cpu.eax);
            put(app, cpu.edx + 4, cpu.eax);
            cpu.eax = cpu.edx;
        }
        else
        {
            cpu.eax = cpu.edx;
            if (!byAddress(app, cpu, kFree))
                return;
            cpu.eax = 0;
        }
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4190e0: the cache eax freed, its nodes first (0x418f40) and it cleared
 * then, if it has a list. */
void freeFileCache(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = cpu.eax;
    if (cpu.eax != 0)
    {
        cpu.ecx = at(app, cpu.eax + 4);
        if (cpu.ecx != 0)
        {
            cpu.eax = cpu.ecx;
            if (!direct(app, cpu, freeFileNodes))
                return;
            put(app, cpu.edx + 4, 0);
            put(app, cpu.edx, 0);
            put(app, cpu.edx + 8, 0);
        }
        cpu.eax = cpu.edx;
        if (!byAddress(app, cpu, kFree))
            return;
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x419120: the BIGF header the cache eax's files take: 16 bytes and, for
 * each, 8 and its name with its ending.  0 for no cache or list, not a
 * cache's magic, a file of no size or no files. */
void bigHeaderSize(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = cpu.eax;
    if (cpu.eax == 0 || (cpu.ecx = at(app, cpu.eax + 4)) == 0)
    {
        cpu.ebx = 0;
    }
    else
    {
        cpu.eax = at(app, cpu.eax);
        if (cpu.eax != kBigMagic)
        {
            cpu.ebx = 0;
        }
        else
        {
            cpu.esi = 0x10;
            cpu.ebx = at(app, cpu.ecx + 0x28);
            bool sized = true;
            while (cpu.ebx != 0)
            {
                if (at(app, cpu.ebx + 0x20) == 0)
                {
                    cpu.ebx = 0;
                    sized = false;
                    break;
                }
                cpu.eax = at(app, cpu.edx);
                if (cpu.eax == kBigMagic)
                {
                    cpu.edi = cpu.ebx;
                    stringLength(app, cpu);
                    cpu.eax = cpu.ecx + 9;
                    cpu.esi += cpu.eax;
                }
                cpu.ebx = at(app, cpu.ebx + 0x28);
            }
            if (sized)
            {
                cpu.eax = at(app, cpu.edx);
                if (!(cpu.eax == kBigMagic && cpu.esi <= 0x10))
                    cpu.ebx = cpu.esi;
            }
        }
    }
    cpu.eax = cpu.ebx;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4191b0: the bytes of the cache eax's files, all told; 0 if one has none.
 * The cache and its list are taken to be there. */
void cachedDataSize(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.eax = at(app, cpu.eax + 4);
    cpu.edx = 0;
    for (;;)
    {
        cpu.eax = at(app, cpu.eax + 0x28);
        if (cpu.eax == 0)
        {
            cpu.eax = cpu.edx;
            break;
        }
        cpu.ecx = at(app, cpu.eax + 0x20);
        if (cpu.ecx == 0)
        {
            cpu.eax = 0;
            break;
        }
        cpu.edx += cpu.ecx;
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

namespace
{

/* The code's way with each of a BIGF header's numbers: eax into the frame's
 * [ebp - 0x3c], turned around there (0x418d00) and copied to edi (4 bytes
 * through copyBytes).  False when the game stopped. */
bool putBigNumber(win32::WinApplication* app, x86::CPU& cpu, reg32 value, reg32 to)
{
    cpu.ecx = 4;
    cpu.esi = cpu.ebp - 0x3c;
    put(app, cpu.ebp - 0x3c, value);
    cpu.eax = cpu.ebp - 0x3c;
    cpu.edi = to;
    if (!direct(app, cpu, swapBigEndian))
        return false;
    copyBytes(app, cpu);
    return true;
}

}  // namespace

/* 0x4191e0: the cache edx written as a BIGF archive into the ebx bytes at
 * eax (cleared first): each file's offset, size and name in the header and
 * its data after it, as far as they fit, then the header's four numbers.  1
 * back; 0 for no buffer, cache, size or list, no files, a header too small,
 * all of it not fitting, or a file of no size or data. */
void writeBigFile(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x3c;
    const reg32 buffer = cpu.ebp - 4, cursor = cpu.ebp - 8, size = cpu.ebp - 0xc, count = cpu.ebp - 0x10;
    const reg32 headerEnd = cpu.ebp - 0x14, at4 = cpu.ebp - 0x18, at0 = cpu.ebp - 0x1c, header = cpu.ebp - 0x20;
    const reg32 at12 = cpu.ebp - 0x24, at8 = cpu.ebp - 0x28, end = cpu.ebp - 0x2c, length = cpu.ebp - 0x30;
    const reg32 nameAt = cpu.ebp - 0x34, sizeAt = cpu.ebp - 0x38, number = cpu.ebp - 0x3c;
    put(app, buffer, cpu.eax);
    cpu.ecx = cpu.edx;
    put(app, size, cpu.ebx);
    cpu.edx = 0;
    cpu.eax += cpu.ebx;
    put(app, number, cpu.edx);
    put(app, end, cpu.eax);
    put(app, count, cpu.edx);
    bool written = false;
    if (cpu.ecx != 0 && at(app, buffer) != 0 && cpu.ebx != 0 && at(app, cpu.ecx + 4) != 0)
    {
        cpu.eax = cpu.ecx;
        if (!direct(app, cpu, countFileNodes))
            return;
        cpu.edx = cpu.eax;
        cpu.eax = cpu.ecx;
        if (!direct(app, cpu, bigHeaderSize))
            return;
        cpu.edi = cpu.eax;
        put(app, header, cpu.eax);
        cpu.eax = cpu.ecx;
        if (!direct(app, cpu, cachedDataSize))
            return;
        if (sreg32(cpu.edx) > 0 && cpu.edi >= 0x10 && (cpu.eax += cpu.edi) <= cpu.ebx)
        {
            cpu.eax = at(app, buffer);
            cpu.edx = 0;
            if (!byAddress(app, cpu, kMemset))
                return;
            cpu.eax = at(app, buffer);
            put(app, at0, cpu.eax);
            cpu.eax += 4;
            put(app, at4, cpu.eax);
            cpu.eax = at(app, buffer);
            cpu.eax += 8;
            put(app, at8, cpu.eax);
            cpu.eax = at(app, buffer);
            cpu.ebx = at(app, buffer);
            cpu.eax += 0xc;
            cpu.edx = at(app, cpu.ecx + 4);
            put(app, at12, cpu.eax);
            cpu.eax = at(app, buffer);
            cpu.ebx += 0x10;
            cpu.eax += cpu.edi;
            cpu.edx = at(app, cpu.edx + 0x28);
            put(app, headerEnd, cpu.eax);
            put(app, cursor, cpu.eax);
            bool failed = false;
            while (cpu.edx != 0)
            {
                cpu.eax = at(app, cursor);
                cpu.esi = at(app, cpu.edx + 0x20);
                cpu.edi = at(app, end);
                cpu.eax += cpu.esi;
                if (cpu.eax > cpu.edi || cpu.ebx >= at(app, headerEnd))
                    break;
                if (cpu.esi == 0 || at(app, cpu.edx + 0x24) == 0)
                {
                    failed = true;
                    break;
                }
                cpu.eax = cpu.ebx + 4;
                cpu.edi = cpu.edx;
                put(app, sizeAt, cpu.eax);
                cpu.eax = cpu.ebx + 8;
                cpu.esi = cpu.edx;
                put(app, nameAt, cpu.eax);
                stringLength(app, cpu);
                cpu.edi = at(app, nameAt);
                put(app, length, cpu.ecx);
                copyString(app, cpu);
                cpu.eax = at(app, cursor);
                cpu.edi = at(app, buffer);
                cpu.eax -= cpu.edi;
                if (!putBigNumber(app, cpu, cpu.eax, cpu.ebx))
                    return;
                if (!putBigNumber(app, cpu, at(app, cpu.edx + 0x20), at(app, sizeAt)))
                    return;
                cpu.edi = at(app, cursor);
                cpu.ecx = at(app, cpu.edx + 0x20);
                cpu.esi = at(app, cpu.edx + 0x24);
                copyBytes(app, cpu);
                cpu.eax = at(app, count);
                cpu.ebx = at(app, nameAt);
                ++cpu.eax;
                cpu.esi = at(app, length);
                put(app, count, cpu.eax);
                cpu.eax = at(app, cpu.edx + 0x20);
                cpu.ebx += cpu.esi;
                cpu.ecx = cpu.edi + cpu.eax;
                ++cpu.ebx;
                put(app, cursor, cpu.ecx);
                cpu.edx = at(app, cpu.edx + 0x28);
            }
            if (!failed)
            {
                cpu.edi = kBigMagic;
                if (!putBigNumber(app, cpu, cpu.edi, at(app, at0)))
                    return;
                if (!putBigNumber(app, cpu, at(app, size), at(app, at4)))
                    return;
                if (!putBigNumber(app, cpu, at(app, count), at(app, at8)))
                    return;
                if (!putBigNumber(app, cpu, at(app, header), at(app, at12)))
                    return;
                cpu.eax = 1;
                written = true;
            }
        }
    }
    if (!written)
        cpu.eax = 0;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x419460: the cache eax written as a BIGF archive into a block of its own
 * ("BigBuffer", 0x4191e0): it back and its size to [edx]; else 0 and 0 to
 * [edx] (the block freed). */
void packFileCache(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    const reg32 size = cpu.ebp - 4;
    cpu.ecx = cpu.eax;
    cpu.edi = cpu.edx;
    bool packed = false;
    if (cpu.eax != 0 && at(app, cpu.eax + 4) != 0)
    {
        if (!direct(app, cpu, countFileNodes))
            return;
        cpu.eax = cpu.ecx;
        if (!direct(app, cpu, bigHeaderSize))
            return;
        cpu.edx = cpu.eax;
        cpu.eax = cpu.ecx;
        if (!direct(app, cpu, cachedDataSize))
            return;
        if (cpu.edx != 0 && cpu.eax != 0)
        {
            cpu.ebx = 0x10;
            cpu.edx += cpu.eax;
            cpu.eax = kBufferName;
            put(app, size, cpu.edx);
            if (!byAddress(app, cpu, kAllocate))
                return;
            cpu.esi = cpu.eax;
            if (cpu.eax == 0)
            {
                put(app, cpu.edi, cpu.eax);
                packed = true;
            }
            else
            {
                cpu.eax = at(app, cpu.ecx);
                if (cpu.eax == kBigMagic)
                {
                    cpu.ebx = at(app, size);
                    cpu.edx = cpu.ecx;
                    cpu.eax = cpu.esi;
                    if (!direct(app, cpu, writeBigFile))
                        return;
                    if (cpu.eax != 0)
                    {
                        cpu.eax = at(app, size);
                        put(app, cpu.edi, cpu.eax);
                        cpu.eax = cpu.esi;
                        packed = true;
                    }
                    else
                    {
                        put(app, cpu.edi, cpu.eax);
                        cpu.eax = cpu.esi;
                        if (!byAddress(app, cpu, kFree))
                            return;
                    }
                }
                else
                {
                    cpu.eax = cpu.esi;
                    put(app, cpu.edi, 0);
                    if (!byAddress(app, cpu, kFree))
                        return;
                }
            }
        }
        else
        {
            put(app, cpu.edi, 0);
        }
    }
    else
    {
        put(app, cpu.edi, 0);
    }
    if (!packed)
        cpu.eax = 0;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x419510: the files of the BIGF archive at edx added to the cache eax
 * (0x419000), each found by its name (0x4ea4a0, 0x4ea290); those of no size
 * passed over.  1 back; 0 for no cache or archive, not a BIGF, no files, or
 * a file not found or added. */
void readBigFile(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x34;
    const reg32 data = cpu.ebp - 4, cache = cpu.ebp - 8, offset = cpu.ebp - 0xc, size = cpu.ebp - 0x10;
    const reg32 magic = cpu.ebp - 0x14, name = cpu.ebp - 0x34;
    put(app, cache, cpu.eax);
    put(app, data, cpu.edx);
    cpu.edx = 0;
    cpu.esi = at(app, data);
    put(app, offset, cpu.edx);
    put(app, size, cpu.edx);
    put(app, magic, cpu.edx);
    if (cpu.esi == 0 || cpu.eax == 0)
    {
        cpu.eax = 0;
    }
    else
    {
        cpu.ecx = 4;
        cpu.edi = magic;
        copyBytes(app, cpu);
        cpu.eax = magic;
        if (!direct(app, cpu, swapBigEndian))
            return;
        if (at(app, magic) != kBigMagic)
        {
            cpu.eax = 0;
        }
        else
        {
            cpu.eax = at(app, data);
            if (!byAddress(app, cpu, kBigCount))
                return;
            cpu.edi = cpu.eax;
            if (cpu.eax != 0)
            {
                cpu.esi = 0;
                for (;;)
                {
                    if (sreg32(cpu.esi) >= sreg32(cpu.edi))
                    {
                        cpu.eax = 1;
                        break;
                    }
                    cpu.ebx = name;
                    cpu.eax = at(app, data);
                    cpu.edx = cpu.esi;
                    cpu.ecx = offset;
                    if (!byAddress(app, cpu, kBigName))
                        return;
                    cpu.eax = size;
                    cpu.edx = name;
                    push(app, cpu, cpu.eax);
                    cpu.ebx = cpu.esi;
                    cpu.eax = at(app, data);
                    if (!byAddress(app, cpu, kBigEntry))
                        return;
                    if (cpu.eax == 0)
                        break;
                    cpu.edx = at(app, size);
                    if (cpu.edx != 0)
                    {
                        cpu.ecx = at(app, data);
                        cpu.ebx = at(app, offset);
                        cpu.eax = at(app, cache);
                        cpu.ecx += cpu.ebx;
                        cpu.ebx = cpu.edx;
                        cpu.edx = name;
                        if (!direct(app, cpu, addCachedFile))
                            return;
                        if (cpu.eax == 0)
                            break;
                    }
                    cpu.ah = 0;
                    putByte(app, name, cpu.ah);
                    cpu.eax = 0;
                    ++cpu.esi;
                    put(app, offset, cpu.eax);
                    put(app, size, cpu.eax);
                }
            }
        }
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4195f0: a cache made of the BIGF archive at eax (0x419080, 0x419510),
 * or 0 -- for none, not a BIGF, or its files not all read (the cache freed). */
void openFileCache(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    const reg32 magic = cpu.ebp - 4;
    cpu.edx = cpu.eax;
    cpu.ecx = 0;
    put(app, magic, cpu.ecx);
    if (cpu.eax != 0)
    {
        cpu.ecx = 4;
        cpu.edi = magic;
        cpu.esi = cpu.eax;
        copyBytes(app, cpu);
        cpu.eax = magic;
        if (!direct(app, cpu, swapBigEndian))
            return;
        if (at(app, magic) != kBigMagic)
        {
            cpu.eax = 0;
        }
        else
        {
            if (!direct(app, cpu, newFileCache))
                return;
            cpu.ecx = cpu.eax;
            if (cpu.eax != 0)
            {
                if (!direct(app, cpu, readBigFile))
                    return;
                if (cpu.eax != 0)
                {
                    cpu.eax = cpu.ecx;
                }
                else
                {
                    cpu.eax = cpu.ecx;
                    if (!direct(app, cpu, freeFileCache))
                        return;
                    cpu.eax = 0;
                }
            }
        }
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41d8a0: the track's file named by the end eax, "<data path>TRKnnn\Trnn<eax>",
 * sprintf'd to 0x5dd86c and that back. */
void trackFilePath(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    push(app, cpu, cpu.eax);
    cpu.edx = at(app, kTrack);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.edx);
    push(app, cpu, kDataPath);
    push(app, cpu, kTrackFileFormat);
    push(app, cpu, kTrackFile);
    if (!byAddress(app, cpu, kSprintf))
        return;
    cpu.eax = kTrackFile;
    cpu.esp += 0x18;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41d8d0: the file eax in the track's folder, "<data path>TRKnnn\<eax>",
 * sprintf'd to 0x5dd8ec and that back. */
void trackFolderPath(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    push(app, cpu, cpu.eax);
    cpu.edx = at(app, kTrack);
    push(app, cpu, cpu.edx);
    push(app, cpu, kDataPath);
    push(app, cpu, kFolderFileFormat);
    push(app, cpu, kFolderFile);
    if (!byAddress(app, cpu, kSprintf))
        return;
    cpu.eax = kFolderFile;
    cpu.esp += 0x14;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41dbc0: the track's textures loaded (0x4956f0) to [0x5dd850] unless the
 * loading was stopped: its own Trnn1.qfs (Trnn1_8.qfs for the options' smaller
 * set) where the race has them and the file is there -- traced, as
 * bworld.c's line 1860 does (0x42d500) -- else Trnn0.qfs (Trnn0_8.qfs). */
void loadTrackTextures(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = 0;
    put(app, kTrackShapes, cpu.edx);
    if (!byAddress(app, cpu, kLoadingTick))
        return;
    if (at(app, kLoadAborted) == 0)
    {
        bool own = false;
        if (at(app, kOwnTextures) != 0)
        {
            cpu.ah = byte(app, kOptions);
            cpu.eax = (cpu.ah & 2) != 0 ? kNear8 : kNear;
            if (!direct(app, cpu, trackFilePath))
                return;
            cpu.ecx = cpu.eax;
            cpu.eax = cpu.ecx;
            if (!byAddress(app, cpu, kFileExists))
                return;
            if (cpu.eax != 0)
            {
                push(app, cpu, 0x744);
                push(app, cpu, kTraceFile);
                push(app, cpu, kTraceFormat);
                if (!byAddress(app, cpu, kTrace))
                    return;
                cpu.esp += 0xc;
                cpu.edx = 0;
                cpu.eax = cpu.ecx;
                own = true;
            }
        }
        if (!own)
        {
            cpu.eax = (byte(app, kOptions) & 2) != 0 ? kFar8 : kFar;
            cpu.edx = 0;
            if (!direct(app, cpu, trackFilePath))
                return;
        }
        if (!byAddress(app, cpu, kLoadShapes))
            return;
        put(app, kTrackShapes, cpu.eax);
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x477a90: the ghost saved to ghost.gst (0x4777d0) when the race is no
 * replay and one is wanted. */
void saveGhost(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (at(app, kReplaying) == 0 && at(app, kGhostWanted) != 0)
    {
        cpu.eax = kGhostName;
        if (!byAddress(app, cpu, kSaveGhostFile))
            return;
    }
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x477ac0: the ghost written to the stream eax at edx (fseek from its
 * start, then fwrite: its record and its records, 0x20f4 + 18 bytes each). */
void writeGhost(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    if (cpu.eax != 0)
    {
        cpu.ebx = 0;
        if (!byAddress(app, cpu, kFseek))
            return;
        cpu.edx = at(app, kGhostCount);
        cpu.eax = cpu.edx * 8;
        cpu.eax += cpu.edx;
        cpu.eax += cpu.eax;
        cpu.edx = cpu.eax + kGhostBytes;
        cpu.ebx = 1;
        cpu.eax = kGhost;
        if (!byAddress(app, cpu, kFwrite))
            return;
    }
    cpu.ebp = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x477b00: the ghost read from the game's file handle eax (0x4e2de0): its
 * record, then as many records as it says; [0x55d000] 1. */
void readGhost(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    if (cpu.eax != 0)
    {
        cpu.ebx = kGhostBytes;
        cpu.edx = kGhost;
        if (!byAddress(app, cpu, kReadHandle))
            return;
        cpu.edx = at(app, kGhostCount);
        cpu.ebx = cpu.edx * 8;
        cpu.eax = cpu.ecx;
        cpu.ebx += cpu.edx;
        cpu.edx = kGhostRecords;
        cpu.ebx += cpu.ebx;
        if (!byAddress(app, cpu, kReadHandle))
            return;
        put(app, kGhostRead, 1);
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

namespace
{

/* lea eax, [ebp - 0x34]; call 0x4be020: the horizon's next number, its text's
 * place in the frame's [ebp - 0x34]. */
bool readNumber(win32::WinApplication* app, x86::CPU& cpu)
{
    cpu.eax = cpu.ebp - 0x34;
    return byAddress(app, cpu, kReadNumber);
}

/* A number into the horizon's record's byte at `offset` as the code puts
 * it: the number to [ebp - `value`], the record ([0x55d278] + edx) to
 * [ebp - `record`], edi it, al the number's byte. */
bool readRecordByte(win32::WinApplication* app, x86::CPU& cpu, reg32 offset, reg32 value, reg32 record)
{
    if (!readNumber(app, cpu))
        return false;
    put(app, cpu.ebp - value, cpu.eax);
    cpu.eax = at(app, 0x55d278);
    cpu.eax += cpu.edx;
    put(app, cpu.ebp - record, cpu.eax);
    cpu.edi = at(app, cpu.ebp - record);
    cpu.al = byte(app, cpu.ebp - value);
    putByte(app, cpu.edi + offset, cpu.al);
    return true;
}

/* A number into the record's dword at `offset`: edi the record. */
bool readRecordWord(win32::WinApplication* app, x86::CPU& cpu, reg32 offset)
{
    if (!readNumber(app, cpu))
        return false;
    cpu.edi = at(app, 0x55d278);
    cpu.edi += cpu.edx;
    put(app, cpu.edi + offset, cpu.eax);
    return true;
}

/* A number to [ebp - `local`], made a float by fild and stored at `to`. */
bool readFloat(win32::WinApplication* app, x86::CPU& cpu, reg32 local, reg32 to)
{
    if (!readNumber(app, cpu))
        return false;
    put(app, cpu.ebp - local, cpu.eax);
    cpu.fpu.push(i32(app, cpu.ebp - local));
    cpu.eax = cpu.ebp - 0x34;
    putF32(app, to, cpu.fpu.pop());
    return true;
}

/* The sky's 8 bands' colour channel from a 16.16 number: sar edx, 31;
 * shl edx, 16; sbb eax, edx; sar eax, 16 -- rounded towards 0, edx left as
 * the shift leaves it. */
reg32 bandChannel(x86::CPU& cpu, reg32 value)
{
    cpu.edx = value;
    cpu.eax = value;
    cpu.edx = reg32(sreg32(cpu.edx) >> 31);
    const reg32 carry = (cpu.edx >> 16) & 1;
    cpu.edx <<= 16;
    cpu.eax = cpu.eax - cpu.edx - carry;
    cpu.eax = reg32(sreg32(cpu.eax) >> 16);
    return cpu.eax;
}

}  // namespace

/* 0x478340: the track eax's horizon, 3Trnn.hrz (with N at night, W in the
 * weather) in its folder, read number by number (0x4be020) into the sky's:
 * its colours, the fog's (1 to 100, a hundredth), its records (36 bytes
 * each, allocated to [0x55d278]), the sun's and the clouds' places and
 * sizes, the weather's counts (0x8ca1e8 on); the file freed, the sky loaded
 * (0x47ac30), the sun's direction worked out and its length taken, and the
 * sky's 8 bands of colours, top and bottom, from its two colours; the
 * fog's colour from the bottom one, or none. */
void loadHorizon(win32::WinApplication* app, x86::CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x144;
    const reg32 name = cpu.ebp - 0x144, text = cpu.ebp - 0x34, number = cpu.ebp - 0x14, other = cpu.ebp - 0x1c;
    cpu.ecx = cpu.eax;
    cpu.edx = 0;
    cpu.ebx = at(app, kNight);
    put(app, text, cpu.edx);
    push(app, cpu, cpu.eax);
    push(app, cpu, cpu.eax);
    push(app, cpu, kDataPath);
    if (cpu.ebx != 0)
        push(app, cpu, at(app, kWeather) != 0 ? kHorizonNightWeather : kHorizonNight);
    else
        push(app, cpu, at(app, kWeather) != 0 ? kHorizonWeather : kHorizonName);
    cpu.eax = name;
    push(app, cpu, cpu.eax);
    if (!byAddress(app, cpu, kSprintf))
        return;
    cpu.esp += 0x14;
    if (!byAddress(app, cpu, kLoadingTick))
        return;
    cpu.edx = 0x10;
    cpu.eax = name;
    if (!byAddress(app, cpu, kLoadFile))
        return;
    cpu.edx = cpu.eax;
    cpu.esi = cpu.eax;
    if (cpu.eax != 0)
    {
        put(app, text, cpu.eax);
        // The sky's colour, its three bytes, and its fourth's dword.
        for (const reg32 to : {0x55d26eu, 0x55d26du, 0x55d26cu})
        {
            cpu.eax = text;
            if (!byAddress(app, cpu, kReadNumber))
                return;
            putByte(app, to, cpu.al);
        }
        if (!readNumber(app, cpu))
            return;
        put(app, 0x55d270, cpu.eax);
        // The fog: 1 to 100, a hundredth of it.
        if (!readNumber(app, cpu))
            return;
        cpu.edx = cpu.eax;
        if (sreg32(cpu.eax) >= 1 && sreg32(cpu.eax) > 0x64)
            cpu.eax = 0x64;
        else if (sreg32(cpu.edx) < 1)
            cpu.eax = 1;
        else
            cpu.eax = cpu.edx;
        put(app, number, cpu.eax);
        fpu.push(i32(app, number));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, kHundredth));
        cpu.eax = text;
        putF32(app, 0x55d268, fpu.pop());
        // The records.
        if (!byAddress(app, cpu, kReadNumber))
            return;
        put(app, 0x55d274, cpu.eax);
        if (cpu.eax == 0)
        {
            put(app, 0x55d278, cpu.eax);
        }
        else
        {
            cpu.edx = cpu.eax * 8;
            cpu.ebx = 0;
            cpu.edx += cpu.eax;
            cpu.eax = kNoName;
            cpu.edx <<= 2;
            if (!byAddress(app, cpu, kAllocate))
                return;
            put(app, 0x55d278, cpu.eax);
            cpu.ebx = 0;
            while (sreg32(cpu.ebx) < sreg32(at(app, 0x55d274)))
            {
                if (!readNumber(app, cpu))
                    return;
                cpu.edx = cpu.ebx * 8;
                put(app, number, cpu.eax);
                cpu.edx += cpu.ebx;
                cpu.eax = at(app, 0x55d278);
                cpu.edx <<= 2;
                cpu.eax += cpu.edx;
                put(app, other, cpu.eax);
                cpu.edi = at(app, other);
                cpu.al = byte(app, number);
                putByte(app, cpu.edi + 2, cpu.al);
                if (!readRecordByte(app, cpu, 1, 0x14, 0x1c) || !readRecordByte(app, cpu, 0, 0x14, 0x1c)
                    || !readRecordWord(app, cpu, 0xc) || !readRecordByte(app, cpu, 6, 0x1c, 0x14)
                    || !readRecordByte(app, cpu, 5, 0x14, 0x1c) || !readRecordByte(app, cpu, 4, 0x14, 0x1c)
                    || !readRecordWord(app, cpu, 0x10) || !readRecordByte(app, cpu, 0xa, 0x14, 0x1c)
                    || !readRecordByte(app, cpu, 9, 0x14, 0x1c))
                    return;
                if (!readNumber(app, cpu))
                    return;
                put(app, other, cpu.eax);
                cpu.eax = at(app, 0x55d278);
                cpu.edi = cpu.eax + cpu.edx;
                cpu.al = byte(app, other);
                putByte(app, cpu.edi + 8, cpu.al);
                if (!readRecordWord(app, cpu, 0x14) || !readRecordWord(app, cpu, 0x18)
                    || !readRecordWord(app, cpu, 0x1c) || !readNumber(app, cpu))
                    return;
                cpu.edi = at(app, 0x55d278);
                cpu.edx += cpu.edi;
                ++cpu.ebx;
                put(app, cpu.edx + 0x20, cpu.eax);
            }
        }
        // The sun's and the clouds'.
        for (const reg32 to : {0x55d1f4u, 0x55d204u, 0x55d260u, 0x55d264u})
        {
            if (!readNumber(app, cpu))
                return;
            put(app, to, cpu.eax);
        }
        if (!readNumber(app, cpu))
            return;
        put(app, number, cpu.eax);
        fpu.push(i32(app, number));
        putF32(app, 0x55d208, fpu.pop());
        if ((at(app, 0x55d208) & 0x7fffffff) != 0)
        {
            fpu.push(f32(app, 0x55d208));
            fpu.push(x86::Float(1.0));
            fpu.st(1) = fp::div(fpu.st(0), fpu.st(1));
            fpu.pop();
            putF32(app, 0x55d208, fpu.pop());
        }
        if (!readFloat(app, cpu, 0x14, 0x55d248) || !readNumber(app, cpu))
            return;
        cpu.edx = cpu.eax;
        cpu.eax <<= 2;
        cpu.eax -= cpu.edx;
        put(app, other, cpu.eax);
        fpu.push(i32(app, other));
        cpu.eax = text;
        putF32(app, 0x55d24c, fpu.pop());
        if (!byAddress(app, cpu, kReadNumber))
            return;
        put(app, 0x55d1f8, cpu.eax);
        if (!readNumber(app, cpu))
            return;
        put(app, 0x55d200, cpu.eax);
        if (!readNumber(app, cpu))
            return;
        put(app, number, cpu.eax);
        fpu.push(i32(app, number));
        fpu.push(fpu.st(0));
        fpu.st(0) = fp::mul(fpu.st(0), f64(app, kHeightScale));
        cpu.eax = text;
        fpu.st(1) = fpu.st(0);
        fpu.pop();
        if (!byAddress(app, cpu, kChop))
            return;
        put(app, 0x55d1fc, reg32(fp::toInteger<sreg32>(fpu.pop())));
        // The sun's angle and distance: its place on the sky.
        if (!byAddress(app, cpu, kReadNumber))
            return;
        cpu.edx = cpu.eax;
        if (!readNumber(app, cpu))
            return;
        put(app, number, cpu.eax);
        put(app, other, cpu.edx);
        fpu.push(i32(app, number));
        fpu.st(0) = fp::mul(fpu.st(0), f64(app, kSunAngleScale));
        fpu.push(i32(app, other));
        fpu.st(0) = fp::mul(fpu.st(0), f64(app, kSunDistanceScale));
        std::swap(fpu.st(0), fpu.st(1));
        putF32(app, cpu.ebp - 0x38, fpu.pop());
        push(app, cpu, at(app, cpu.ebp - 0x38));
        putF32(app, cpu.ebp - 0x3c, fpu.pop());
        if (!byAddress(app, cpu, kCosine))
            return;
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x3c));
        push(app, cpu, at(app, cpu.ebp - 0x38));
        putF32(app, 0x55d20c, fpu.pop());
        if (!byAddress(app, cpu, kSine))
            return;
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, cpu.ebp - 0x3c));
        cpu.eax = text;
        putF32(app, 0x55d210, fpu.pop());
        if (!byAddress(app, cpu, kReadNumber))
            return;
        put(app, 0x55d158, cpu.eax);
        if (!readNumber(app, cpu))
            return;
        put(app, 0x55d15c, cpu.eax);
        if (!readNumber(app, cpu))
            return;
        put(app, number, cpu.eax);
        fpu.push(i32(app, number));
        putF32(app, 0x55d1c8, fpu.pop());
        // The clouds' turn: degrees made 16.16 turns, then within 0 and 1.
        if (!readNumber(app, cpu))
            return;
        cpu.eax <<= 16;
        cpu.edx = cpu.eax;
        cpu.ebx = 0x168;
        cpu.edx = reg32(sreg32(cpu.edx) >> 31);
        {
            const x86::sreg64 dividend = x86::sreg64(x86::reg64(cpu.edx) << 32 | cpu.eax);
            cpu.eax = reg32(dividend / sreg32(cpu.ebx));
            cpu.edx = reg32(dividend % sreg32(cpu.ebx));
        }
        put(app, 0x55d1f0, cpu.eax);
        fpu.push(i32(app, 0x55d1f0));
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, kTurnScale));
        for (;;)
        {
            putF32(app, 0x55d1cc, fpu.pop());
            fpu.compare(x86::Float(0.0), f32(app, 0x55d1cc));
            cpu.ax = fpu.status.word;
            if ((cpu.ah & 0x41) != 0)  // jbe: CF (C0) or ZF (C3)
                break;
            fpu.push(x86::Float(1.0));
            fpu.st(0) = fp::add(fpu.st(0), f32(app, 0x55d1cc));
        }
        while (sreg32(at(app, 0x55d1cc)) > 0x3f800000)
        {
            fpu.push(f32(app, 0x55d1cc));
            fpu.st(0) = fp::add(fpu.st(0), f32(app, kMinusOne));
            putF32(app, 0x55d1cc, fpu.pop());
        }
        if (!readNumber(app, cpu))
            return;
        put(app, 0x55d160, cpu.eax);
        if (!readFloat(app, cpu, 0x14, 0x55d164) || !readFloat(app, cpu, 0x14, 0x55d168)
            || !readFloat(app, cpu, 0x14, 0x55d16c) || !readFloat(app, cpu, 0x14, 0x55d170)
            || !readFloat(app, cpu, 0x1c, 0x55d174))
            return;
        // The sky's colours: the fog's, the top's, the bottom's.
        for (const reg32 to : {0x55d1c2u, 0x55d1c1u, 0x55d1c0u, 0x55d1c6u, 0x55d1c5u, 0x55d1c4u, 0x55d17au,
                               0x55d179u, 0x55d178u})
        {
            if (!readNumber(app, cpu))
                return;
            putByte(app, to, cpu.al);
        }
        if (!readNumber(app, cpu))
            return;
        cpu.bl = cpu.al;
        if (!readNumber(app, cpu))
            return;
        cpu.bh = cpu.al;
        if (!readNumber(app, cpu))
            return;
        putByte(app, cpu.ebp - 0xc, cpu.al);
        if (!readNumber(app, cpu))
            return;
        cpu.dl = cpu.al;
        if (!readNumber(app, cpu))
            return;
        putByte(app, cpu.ebp - 4, cpu.al);
        if (!readNumber(app, cpu))
            return;
        putByte(app, cpu.ebp - 8, cpu.al);
        // The weather's.
        if (!readNumber(app, cpu))
            return;
        putByte(app, kWeatherCount, cpu.al);
        for (const reg32 to : {0x8ca1eau, 0x8ca1ecu, 0x8ca1eeu})
        {
            if (!readNumber(app, cpu))
                return;
            putHalf(app, to, cpu.ax);
        }
        for (const reg32 to : {0x8ca1f0u, 0x8ca1f4u, 0x8ca1f8u, 0x8ca1fcu, 0x55d148u, 0x55d14cu})
        {
            if (!readNumber(app, cpu))
                return;
            put(app, to, cpu.eax);
        }
        if (!readNumber(app, cpu))
            return;
        cpu.edi = at(app, 0x8ca1f4);
        put(app, 0x55d150, cpu.eax);
        cpu.eax = at(app, 0x8ca1f0);
        cpu.edi += cpu.eax;
        put(app, 0x8ca1f4, cpu.edi);
        cpu.eax = cpu.edi;
        cpu.edi = at(app, 0x8ca1f8);
        cpu.edi += cpu.eax;
        put(app, 0x8ca1f8, cpu.edi);
        cpu.eax = cpu.edi;
        put(app, 0x8ca1fc, at(app, 0x8ca1fc) + cpu.eax);
        if (cpu.esi != 0)
        {
            cpu.eax = cpu.esi;
            if (!byAddress(app, cpu, kFree))
                return;
        }
        cpu.eax = cpu.ecx;
        if (!direct(app, cpu, loadSky) || !byAddress(app, cpu, kLoadingTick))
            return;
        // The sun's direction, 16.16 sine and cosine of the clouds' turn.
        cpu.eax = at(app, 0x55d1f0);
        if (!byAddress(app, cpu, kFixedSine))
            return;
        put(app, number, cpu.eax);
        fpu.push(i32(app, number));
        cpu.eax = 0x3f000000;
        fpu.st(0) = fp::mul(fpu.st(0), f64(app, kFixedScale));
        put(app, 0x55d2ac, cpu.eax);
        cpu.eax = at(app, 0x55d1f0);
        putF32(app, 0x55d2a8, fpu.pop());
        if (!byAddress(app, cpu, kFixedCosine))
            return;
        put(app, number, cpu.eax);
        cpu.ecx = 8;
        fpu.push(i32(app, number));
        fpu.st(0) = fp::mul(fpu.st(0), f64(app, kFixedScale));
        put(app, cpu.ebp - 0x30, cpu.ecx);
        cpu.eax = 0x55d2a8;
        putF32(app, 0x55d2b0, fpu.pop());
        if (!byAddress(app, cpu, kVectorLength))
            return;
        fpu.pop();
        if (!byAddress(app, cpu, kLoadingTick))
            return;
        // The sky's 8 bands: from the top's colour to the bottom's in steps
        // of an eighth, 16.16, the band and its mirror (16 - it) each.
        cpu.eax = 0;
        cpu.edx &= 0xff;
        cpu.al = cpu.bl;
        cpu.edx -= cpu.eax;
        cpu.edx <<= 0xd;
        put(app, cpu.ebp - 0x18, cpu.edx);
        cpu.edx = 0;
        cpu.esi = cpu.bh;
        cpu.dl = byte(app, cpu.ebp - 4);
        cpu.edi = 0;
        cpu.edx -= cpu.esi;
        cpu.ecx = cpu.eax;
        cpu.edx <<= 0xd;
        cpu.ebx = 0;
        put(app, cpu.ebp - 0x10, cpu.edx);
        cpu.edx = 0;
        cpu.bl = byte(app, cpu.ebp - 0xc);
        cpu.dl = byte(app, cpu.ebp - 8);
        cpu.ecx <<= 0x10;
        cpu.edx -= cpu.ebx;
        cpu.esi <<= 0x10;
        cpu.edx <<= 0xd;
        cpu.ebx <<= 0x10;
        put(app, cpu.ebp - 0x2c, cpu.edx);
        while (sreg32(cpu.edi) < sreg32(at(app, cpu.ebp - 0x30)))
        {
            put(app, cpu.ebp - 0x28, bandChannel(cpu, cpu.ecx));
            put(app, cpu.ebp - 0x24, bandChannel(cpu, cpu.esi));
            bandChannel(cpu, cpu.ebx);
            cpu.dl = byte(app, cpu.ebp - 0x28);
            put(app, cpu.ebp - 0x20, cpu.eax);
            putByte(app, cpu.edi * 4 + 0x55d17e, cpu.dl);
            cpu.dl = byte(app, cpu.ebp - 0x24);
            cpu.eax = 0x10;
            putByte(app, cpu.edi * 4 + 0x55d17d, cpu.dl);
            cpu.dl = byte(app, cpu.ebp - 0x20);
            cpu.eax -= cpu.edi;
            putByte(app, cpu.edi * 4 + 0x55d17c, cpu.dl);
            cpu.dl = byte(app, cpu.ebp - 0x28);
            putByte(app, cpu.eax * 4 + 0x55d17a, cpu.dl);
            cpu.dl = byte(app, cpu.ebp - 0x24);
            putByte(app, cpu.eax * 4 + 0x55d179, cpu.dl);
            cpu.dl = byte(app, cpu.ebp - 0x20);
            ++cpu.edi;
            putByte(app, cpu.eax * 4 + 0x55d178, cpu.dl);
            cpu.edx = at(app, cpu.ebp - 0x18);
            cpu.eax = at(app, cpu.ebp - 0x10);
            cpu.ecx += cpu.edx;
            cpu.edx = at(app, cpu.ebp - 0x2c);
            cpu.esi += cpu.eax;
            cpu.ebx += cpu.edx;
        }
        cpu.eax = at(app, 0x55d17c);
        put(app, 0x55d1bc, cpu.eax);
        if (!byAddress(app, cpu, kLoadingTick))
            return;
        // The fog's colour: none, or the bottom's.
        cpu.ecx = at(app, 0x55d158);
        if (cpu.ecx != 0)
        {
            cpu.ah = 0;
            putByte(app, 0x6ff9fa, cpu.ah);
            putByte(app, 0x6ff9f9, cpu.ah);
            putByte(app, 0x6ff9f8, cpu.ah);
            putByte(app, 0x6ff9fb, cpu.ah);
        }
        else
        {
            cpu.al = byte(app, 0x55d17a);
            putByte(app, 0x6ff9fa, cpu.al);
            cpu.al = byte(app, 0x55d179);
            cpu.bh = 0xff;
            putByte(app, 0x6ff9f9, cpu.al);
            cpu.al = byte(app, 0x55d178);
            putByte(app, 0x6ff9fb, cpu.bh);
            putByte(app, 0x6ff9f8, cpu.al);
        }
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x47ac30: the track eax's sky, sky.fsh in its folder: its clouds' shape
 * (palette to 0x55d21c), the sun's (to 0x6ff9a0, 4 of its colours with the
 * options' smaller textures, else 7) and the sun's glow placed (0x47a970) --
 * CLD/CHR/SUN with N at night, W in the weather, D by day -- [0x563a90] the
 * sun's palette, the file freed and the sky made (0x47ae10). */
void loadSky(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x100;
    const reg32 name = cpu.ebp - 0x100;
    push(app, cpu, cpu.eax);
    push(app, cpu, kDataPath);
    push(app, cpu, kSkyName);
    cpu.eax = name;
    push(app, cpu, cpu.eax);
    if (!byAddress(app, cpu, kSprintf))
        return;
    cpu.esp += 0x10;
    cpu.edx = 0;
    cpu.eax = name;
    if (!byAddress(app, cpu, kLoadFile))
        return;
    cpu.esi = cpu.eax;
    if (cpu.eax != 0)
    {
        if (!byAddress(app, cpu, kLoadingTick))
            return;
        cpu.edi = (byte(app, kOptions) & 2) != 0 ? 4 : 7;
        // The shapes' names: clouds, sun, glow.
        constexpr reg32 names[4][3] = {{0x53b354, 0x53b35c, 0x53b364},   // night, weather
                                       {0x53b36c, 0x53b374, 0x53b37c},   // night
                                       {0x53b384, 0x53b38c, 0x53b394},   // weather
                                       {0x53b39c, 0x53b3a4, 0x53b3ac}};  // day
        const reg32* shapes = names[(at(app, kNight) != 0 ? 0 : 2) + (at(app, kWeather) != 0 ? 0 : 1)];
        push(app, cpu, 7);
        cpu.ecx = kCloudPalette;
        cpu.ebx = 0x40;
        cpu.edx = shapes[0];
        cpu.eax = cpu.esi;
        if (!byAddress(app, cpu, kFindShape))
            return;
        cpu.ecx = kSunPalette;
        cpu.ebx = 0x80;
        push(app, cpu, cpu.edi);
        cpu.edx = shapes[1];
        cpu.eax = cpu.esi;
        if (!byAddress(app, cpu, kFindShape))
            return;
        cpu.edx = shapes[2];
        cpu.eax = cpu.esi;
        if (!byAddress(app, cpu, kPlaceSun))
            return;
        put(app, kSunShapes, kSunPalette);
        if (!byAddress(app, cpu, kLoadingTick))
            return;
        if (cpu.esi != 0)
        {
            cpu.eax = cpu.esi;
            if (!byAddress(app, cpu, kFree))
                return;
        }
        if (!byAddress(app, cpu, kSkyMade) || !byAddress(app, cpu, kLoadingTick))
            return;
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4dcd20: the weather's shapes, weather.fsh: as many raindrops' (DRPn, to
 * 0x8ca200) and splashes' (SPLn, to 0x8ca288) as the horizon's count
 * ([0x8ca1e8]) says, by 0x4d4080 -- none for 0; the file freed. */
void loadWeatherShapes(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x144;
    const reg32 name = cpu.ebp - 0x144, shape = cpu.ebp - 0x44, file = cpu.ebp - 4;
    push(app, cpu, kWeatherPath);
    push(app, cpu, kWeatherName);
    cpu.eax = name;
    push(app, cpu, cpu.eax);
    if (!byAddress(app, cpu, kSprintf))
        return;
    cpu.esp += 0xc;
    cpu.edx = 0;
    cpu.eax = name;
    if (!byAddress(app, cpu, kLoadFile))
        return;
    cpu.edi = byte(app, kWeatherCount);
    cpu.esi = cpu.eax;
    put(app, file, cpu.eax);
    if (cpu.edi != 0)
    {
        for (const auto& kind : {std::pair<reg32, reg32>{kDropName, kDropShapes}, {kSplashName, kSplashShapes}})
        {
            push(app, cpu, cpu.edi);
            push(app, cpu, kind.first);
            cpu.eax = shape;
            push(app, cpu, cpu.eax);
            if (!byAddress(app, cpu, kSprintf))
                return;
            cpu.esp += 0xc;
            cpu.ecx = kind.second;
            cpu.ebx = 0x20;
            push(app, cpu, 0);
            cpu.edx = shape;
            cpu.eax = cpu.esi;
            if (!byAddress(app, cpu, kFindShape))
                return;
        }
    }
    cpu.edx = at(app, file);
    if (cpu.edx != 0)
    {
        cpu.eax = cpu.edx;
        if (!byAddress(app, cpu, kFree))
            return;
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

}  // namespace load
}  // namespace nfs3hp
