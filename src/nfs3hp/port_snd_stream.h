#ifndef NFS3HP_PORT_SND_STREAM_H
#define NFS3HP_PORT_SND_STREAM_H

#include "port_snd.h"

#include <cstddef>
#include <cstdint>

/* EA's streaming, rewritten (port.h says how): the CD stream that a reader
 * thread fills from a file (port_snd_cdstream.cpp) and the SND streamer that
 * plays it, with the system tasks it runs on and the top-up the loads wait
 * for (port_snd_stream.cpp).
 *
 * The streams' records stay where the game has them -- Pathfinder and the
 * movies' streamer read them there -- and are read and written in place.
 * Each of the game's entries is an adapter (named as the game's code knows
 * it) around a core with a name of its own; what one file's functions call
 * of the other's is declared here. */
namespace nfs3hp
{
namespace port
{
namespace stream
{

#pragma pack(push, 1)

/* A CD stream (cdstream.c), 0xc0 bytes: a ring of chunks that a reader thread
 * fills from a file, block by block, as the commands queued to it ask.  In the
 * ring each chunk is an 8-byte header -- its id, its length with the header
 * -- and its data: -1 for an id wraps the ring to its start, -2 marks a chunk
 * given back, -3 one the reader dropped. */
struct CdStream
{
    uint32_t other0;
    uint32_t ringStart;       // +4, and its end
    uint32_t ringEnd;
    uint32_t writeAt;         // where the reader writes
    uint32_t chunkStart;      // its chunk's start
    uint32_t readAt;          // where the player reads
    uint32_t freedAt;         // what was given back
    uint32_t file;            // the file (0 none)
    uint32_t state;           // the reader's
    uint32_t command;         // one waiting for the reader (8 stop, 0xf go on)
    uint32_t busy;            // the reader works
    uint32_t first;           // the first stream
    uint32_t other30[3];
    uint32_t ringSize;
    uint32_t block;           // the block read at once
    uint32_t chunkLeft;       // of the chunk being read on: what is still to read
    uint32_t chunkLength;     // its length
    uint32_t chunkHeld;       // what of it the reader holds
    uint32_t fileStart;       // the file's, in its archive
    uint32_t fileEnd;
    uint32_t fileLength;
    uint32_t filePlace;       // where it is read from
    uint32_t fileWanted;      // where a seek asked
    uint32_t other64;
    uint32_t id;              // the chunk ids its mask and id take
    uint32_t mask;
    uint32_t next;            // the next stream
    uint32_t freeBlocks;      // the free command blocks
    uint32_t queued;          // the commands queued, first and last
    uint32_t queuedLast;
    uint32_t doing;           // the one being done
    uint32_t ringBytes;       // the ring's size and where it is, as it was made
    uint32_t ring;
    uint32_t thread[5];       // the reader's thread
    uint32_t blocksAtOnce;    // blocks of the file's size read at once
    uint32_t readerEvent;     // the event the reader sets when it works, and waits on
    uint32_t streamEvent;
    uint32_t lock;            // the stream's lock
    uint32_t skip;            // the part of the first block before the offset asked
    uint32_t waiting;         // the reader waits for room
    uint32_t bytesRead;
    uint32_t bytesPlayed;
};
static_assert(sizeof(CdStream) == 0xc0 && offsetof(CdStream, readAt) == 0x14 && offsetof(CdStream, state) == 0x20
                  && offsetof(CdStream, chunkLeft) == 0x44 && offsetof(CdStream, fileWanted) == 0x60
                  && offsetof(CdStream, queued) == 0x78 && offsetof(CdStream, doing) == 0x80
                  && offsetof(CdStream, thread) == 0x8c && offsetof(CdStream, readerEvent) == 0xa4
                  && offsetof(CdStream, bytesPlayed) == 0xbc,
              "the CD stream's layout");

/* A command block (0x9c bytes): the file's name, the argument, the command
 * and the next block. */
struct CommandBlock
{
    uint8_t name[0x90];
    uint32_t argument;
    uint32_t command;
    uint32_t next;
};
static_assert(sizeof(CommandBlock) == 0x9c, "the command block's layout");

/* A chunk's header in a ring. */
struct ChunkHeader
{
    uint32_t id;
    uint32_t size;
};

#pragma pack(pop)

/* The game's code the streams call by address.  The checks (tools/game_checks)
 * put the stand-in of a function in place where the generated code of one of
 * these calls it only when the file names its address, so these two, which
 * port_snd.h names as well, are named here. */
constexpr reg32 kSurface = 0x4f9070;  // whether a DirectDraw surface is locked
constexpr reg32 kMemmove = 0x4ea4f0;  // EA's memmove: eax from, edx to, ebx bytes

// The streams (cdstream.c's): the first, and the one the reader is at.
constexpr Var<reg32> kStreams{0x5645b8}, kStreamWalk{0x5645bc};
constexpr Var<int32_t> kTimer{0x79c1a4};  // the game's timer, 60 a second

// The wrapper commands and ids of a ring's chunks.
constexpr reg32 kWrap = 0xffffffff, kGivenBack = 0xfffffffe, kDropped = 0xfffffffd;

inline CdStream& cdStream(App* app, reg32 at) { return record<CdStream>(app, at); }

/* One of the game's critical sections, entered or left with its address in
 * eax as the code pushes it from there. */
inline Result enterSection(App* app, CPU& cpu, reg32 esp, reg32 section)
{
    reg32 eax;
    if (!callWindows<kEnterCriticalSection, &win32::kernel32::EnterCriticalSection>(app, cpu, esp, {section}, eax,
                                                                                     section))
        return {};
    return int32_t(eax);
}

inline Result leaveSection(App* app, CPU& cpu, reg32 esp, reg32 section)
{
    reg32 eax;
    if (!callWindows<kLeaveCriticalSection, &win32::kernel32::LeaveCriticalSection>(app, cpu, esp, {section}, eax,
                                                                                     section))
        return {};
    return int32_t(eax);
}

/* The registers as they were on entry, put back on the way out of a core that
 * the rewrites call as C++: the game's code keeps every register but eax for
 * its caller (and a rewrite sets some for the game's code it calls). */
struct Preserve
{
    CPU& cpu;
    reg32 ebx, ecx, edx, esi, edi, ebp;
    explicit Preserve(CPU& c) : cpu(c), ebx(c.ebx), ecx(c.ecx), edx(c.edx), esi(c.esi), edi(c.edi), ebp(c.ebp) {}
    ~Preserve()
    {
        cpu.ebx = ebx;
        cpu.ecx = ecx;
        cpu.edx = edx;
        cpu.esi = esi;
        cpu.edi = edi;
        cpu.ebp = ebp;
    }
    Preserve(const Preserve&) = delete;
    Preserve& operator=(const Preserve&) = delete;
};

/* The registers as the code before left them, for a call that is handed only
 * some. */
inline Args residue(const CPU& c) { return {c.eax, c.edx, c.ebx, c.ecx}; }

/* A loop's safepoint, as the generated code has one at each loop's head: the
 * guest's context handed over when another thread waits for it; and a busy
 * wait's turn, the thread's slice given up so that the thread the wait waits
 * on runs. */
void safepoint(App* app, CPU& cpu);
void giveWay(App* app, CPU& cpu);

/* The CD stream (port_snd_cdstream.cpp) as the streamer calls it. */

// The chunk (a header in the ring) given back to the reader, which is woken.
Result giveBackChunk(App* app, CPU& cpu, reg32 esp, reg32 stream, reg32 chunk);
// What the player has taken in all (0 for no stream).
reg32 bytesBuffered(App* app, reg32 stream);
// Whether the reader has nothing to do.
bool readerIsIdle(App* app);
// Whether the stream's reader waits for room.
reg32 readerIsWaiting(App* app, reg32 stream);
// The next chunk of the stream to play, whole (0 none, -1 one dropped, -2 stopped).
Result nextChunkToPlay(App* app, CPU& cpu, reg32 esp, reg32 stream);
// Whether `count` command blocks are free (1 or 0).
Result commandBlocksAvailable(App* app, CPU& cpu, reg32 esp, reg32 count);

// The reader's commands by their names.
enum class Command : reg32
{
    Open = 1,
    Seek = 2,
    StopAt = 0xb,
    Continue = 0xd,
    ReadOn = 0x10,
    OpenNext = 0x11,
    Switch = 0x12,
    QueueFile = 0x13,
    Restart = 0x14,
};

/* streamcommanda's and purgestreamcommanda's wrappers (the commands named
 * one by one): `esp` is the wrapper's own, at its return address. */
Result queueNamed(App* app, CPU& cpu, reg32 esp, reg32 stream, reg32 file, Command command, bool urgent, bool mind);
Result queueArgument(App* app, CPU& cpu, reg32 esp, reg32 stream, reg32 argument, Command command, bool urgent,
                     bool mind);
Result purgeNamed(App* app, CPU& cpu, reg32 esp, reg32 stream, reg32 file, Command command, bool mind);
Result purgeArgument(App* app, CPU& cpu, reg32 esp, reg32 stream, reg32 argument, Command command, bool mind);

}  // namespace stream
}  // namespace port
}  // namespace nfs3hp

#endif
