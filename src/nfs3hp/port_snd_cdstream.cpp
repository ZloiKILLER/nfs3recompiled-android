#include "port_snd_stream.h"
#include "native_ea_files.h"
#include "native_ea_memory.h"
#include "native_ea_support.h"
#include "native_ea_threads.h"
#include "native_game.h"
#include "native_watcom.h"

#include <cstdint>

/* EA's CD stream (cmn\cdstream.c), rewritten (port_snd_stream.h says how): a
 * ring of chunks a reader thread fills from a file, block by block, as
 * commands queued to it ask -- open a file, go on to the next, seek, stop --
 * and the SND streamer (port_snd_stream.cpp) takes chunks from, giving each
 * back when played.
 *
 * In the ring each chunk is an 8-byte header -- its id, its length with the
 * header -- and its data: -1 for an id wraps the ring to its start, -2 marks a
 * chunk given back, -3 one the reader dropped.  A command block (0x9c bytes) is
 * the file's name, then its argument, the command and the next block.
 *
 * The reader runs on a thread of its own in the game; its function is here as
 * the game's entry like the rest, and its waits and file reads are the game's
 * code called from its stack. */
namespace nfs3hp
{
namespace port
{
namespace stream
{

namespace
{

constexpr Var<u32> kBlockCount{0x5645a4};
constexpr u32 kCdFile = 0x54a3d0;
constexpr u32 kSetEvent = 0x4dfb20, kWaitEvent = 0x4dfb70, kCreateEvent = 0x4dfb00, kCloseHandle = 0x4dfc00;
constexpr u32 kCreateThread = 0x4df7a0, kRemoveThread = 0x4df870, kSleep = 0x4df8e0;
constexpr u32 kNewLock = 0x4eb2e0, kFreeLock = 0x4eb360;
constexpr u32 kAllocate = 0x4e1620, kFree = 0x4e1890, kFillZero = 0x4e070c, kTouch = 0x5011a0;
constexpr u32 kStrncpy = 0x4e0e30, kTrace = 0x4e08f0;
constexpr u32 kOpenBlock = 0x501270, kCloseBlock = 0x5013a0, kReadBlock = 0x5013b0, kSeekBlock = 0x501450;
constexpr u32 kReader = 0x4e53c0;
constexpr u32 kEndId = 0x5343456c;  // 'SCEl', big-endian
constexpr Table<u32> kWhere{0x552190};

u32 swapped(u32 v) { return v >> 24 | (v >> 8 & 0xff00) | (v << 8 & 0xff0000) | v << 24; }

CommandBlock& block(App* app, u32 at) { return record<CommandBlock>(app, at); }

/* The stream's lock, entered and left. */
Result lockStream(App* app, ThreadState& ts, u32 esp, u32 stream)
{
    return enterSection(app, ts, esp, cdStream(app, stream).lock);
}

Result unlockStream(App* app, ThreadState& ts, u32 esp, u32 stream)
{
    return leaveSection(app, ts, esp, cdStream(app, stream).lock);
}

/* The reader woken: the stream's event (at +0xa8, its thread's at +0x8c). */
Result wakeReader(App* app, ThreadState& ts, u32 esp, u32 stream)
{
    u32 eax;
    if (!took(directFrom<kSetEvent, eventPostedWith>(app, ts, esp, stream + 0x8c, cdStream(app, stream).streamEvent),
              eax))
        return {};
    return int32_t(eax);
}

}  // namespace

/* 0x4e68a0: chunk `chunk` of stream `stream` given back, the reader woken. */
Result giveBackChunk(App* app, ThreadState& ts, u32 esp, u32 stream, u32 chunk)
{
    record<ChunkHeader>(app, chunk).id = kGivenBack;
    return wakeReader(app, ts, esp, stream);
}

/* 0x4e68c0: the bytes stream `stream` holds read (none for no stream). */
u32 bytesBuffered(App* app, u32 stream)
{
    return stream != 0 ? cdStream(app, stream).bytesRead : stream;
}

/* 0x4e68e0: whether the reader has nothing to do: no stream, or idle (7) with
 * no command queued. */
bool readerIsIdle(App* app)
{
    const u32 s = kStreams(app);
    return s == 0 || (cdStream(app, s).state == 7 && cdStream(app, s).queued == 0);
}

/* 0x4e6950: whether stream `stream`'s reader waits for room. */
u32 readerIsWaiting(App* app, u32 stream) { return cdStream(app, stream).waiting; }

/* 0x4e6660: the chunks given back at stream `stream`'s freed end passed over
 * -- the room they took counted out -- and a wrap followed to the ring's
 * start. */
Result passFreed(App* app, ThreadState& ts, u32 esp, u32 stream)
{
    const u32 frame = esp - 3 * 4;
    CdStream& s = cdStream(app, stream);
    if (!lockStream(app, ts, frame, stream))
        return {};
    if (s.freedAt != s.readAt)
        for (;;)
        {
            if (s.freedAt == s.chunkStart)
                break;
            const u32 p = s.freedAt;
            ChunkHeader& header = record<ChunkHeader>(app, p);
            if (header.id == kGivenBack)
            {
                s.bytesPlayed -= header.size - 8;
                s.freedAt += header.size;
            }
            else if (header.id == kWrap)
            {
                if (p == s.readAt)
                    s.readAt = s.ringStart;
                s.freedAt = s.ringStart;
            }
            else
                break;
            if (s.freedAt == s.readAt)
                break;
        }
    return unlockStream(app, ts, frame, stream);
}

/* 0x4e66e0: the room stream `stream` has to read into before the freed end,
 * or to the ring's end when that is behind or the ring empty. */
Result ringRoomAhead(App* app, ThreadState& ts, u32 esp, u32 stream)
{
    if (!passFreed(app, ts, esp - 1 * 4 - 4, stream))
        return {};
    const CdStream& s = cdStream(app, stream);
    u32 room = s.freedAt - s.writeAt;
    if (s.freedAt < s.writeAt || (s.freedAt == s.writeAt && s.freedAt == s.readAt))
        room = s.ringEnd - s.writeAt;
    return int32_t(room);
}

/* 0x4e6710: the room from where stream `stream` writes to the ring's end. */
Result ringRoomToEnd(App* app, ThreadState& ts, u32 esp, u32 stream)
{
    if (!passFreed(app, ts, esp - 1 * 4 - 4, stream))
        return {};
    return int32_t(cdStream(app, stream).ringEnd - cdStream(app, stream).writeAt);
}

/* 0x4e6720: the room at the ring's start, when the freed end has wrapped. */
Result ringRoomAtStart(App* app, ThreadState& ts, u32 esp, u32 stream)
{
    if (!passFreed(app, ts, esp - 1 * 4 - 4, stream))
        return {};
    const CdStream& s = cdStream(app, stream);
    return int32_t(s.freedAt > s.writeAt ? 0 : s.freedAt - s.ringStart);
}

/* 0x4e6740 (getstreamchunk): the next chunk of stream `stream` to play, whole;
 * 0 for none yet (or while a command that empties the ring waits), -2 while
 * the reader is stopped, -1 for a chunk it dropped. */
Result nextChunkToPlay(App* app, ThreadState& ts, u32 esp, u32 stream)
{
    const u32 frame = esp - 5 * 4;
    if (stream == 0)
    {
        if (!fail(app, ts, frame, Where{kCdFile, 0x54a844}, 0x91b, {0x54a854}, stream))
            return {};
        return 0;
    }
    CdStream& s = cdStream(app, stream);
    if (s.queued != 0)
    {
        const u32 command = block(app, s.queued).command;
        if (command == 0xb || command == 0xd || command == 0x14 || command == 0x17)
            return 0;
    }
    if (s.state == 2 || s.state == 1)
        return 0;
    if (s.state == 0xe)
        return -2;
    if (s.chunkStart == s.readAt)
        return 0;
    if (!lockStream(app, ts, frame, stream))
        return {};
    u32 result = 0;
    bool take = true;
    if (record<ChunkHeader>(app, s.readAt).id == kWrap)
    {
        s.readAt = s.ringStart;
        take = s.chunkStart != s.readAt;
    }
    if (take)
    {
        const u32 size = record<ChunkHeader>(app, s.readAt).size;
        const u32 write = s.chunkStart, read = s.readAt;
        const int32_t room = write >= read ? int32_t(write - read) : int32_t(s.ringEnd - read);
        if (room >= int32_t(size))
        {
            const u32 p = read;
            s.bytesRead -= size - 8;
            s.bytesPlayed += size - 8;
            s.readAt += size;
            result = p;
            if (record<ChunkHeader>(app, p).id == kDropped)
            {
                if (p == s.freedAt)
                    s.freedAt = s.readAt;
                else
                    record<ChunkHeader>(app, p).id = kGivenBack;
                result = 0xffffffff;
                if (!wakeReader(app, ts, frame, kStreams(app)))
                    return {};
            }
        }
    }
    if (!unlockStream(app, ts, frame, stream))
        return {};
    return int32_t(result);
}

/* 0x4e69e0: the stream's command blocks: `count` of them at `first`, chained.
 * The last (the first back for no stream). */
u32 chainBlocks(App* app, u32 first, u32 count)
{
    u32 b = first;
    const u32 s = kStreams(app);
    if (s == 0)
        return b;
    cdStream(app, s).freeBlocks = b;
    for (int32_t i = 0; i < int32_t(count - 1); ++i)
    {
        block(app, b).next = b + 0x9c;
        b += 0x9c;
    }
    block(app, b).next = 0;
    return b;
}

/* 0x4e6a20: command block `b` back on the stream's free list. */
Result releaseBlock(App* app, ThreadState& ts, u32 esp, u32 b)
{
    const u32 s = kStreams(app);
    if (s == 0)
        return int32_t(b);
    const u32 frame = esp - 3 * 4;
    if (!lockStream(app, ts, frame, s))
        return {};
    block(app, b).next = cdStream(app, kStreams(app)).freeBlocks;
    cdStream(app, kStreams(app)).freeBlocks = b;
    return unlockStream(app, ts, frame, kStreams(app));
}

/* 0x4e6a70: whether `count` command blocks are free. */
Result commandBlocksAvailable(App* app, ThreadState& ts, u32 esp, u32 count)
{
    const u32 frame = esp - 3 * 4;
    if (kStreams(app) == 0)
        return 0;
    if (!lockStream(app, ts, frame, kStreams(app)))
        return {};
    u32 b = cdStream(app, kStreams(app)).freeBlocks;
    u32 n = count;
    while (n != 0 && b != 0)
    {
        b = block(app, b).next;
        --n;
    }
    if (!unlockStream(app, ts, frame, kStreams(app)))
        return {};
    return n == 0 ? 1 : 0;
}

namespace
{

/* 0x4e6b30 (getstreamblocka): a free command block; none (0) when there is
 * none and `mind` says not to mind, an error when it minds. */
Result acquireBlock(App* app, ThreadState& ts, u32 esp, u32 mind)
{
    const u32 frame = esp - 3 * 4;
    if (kStreams(app) == 0)
        return 0;
    Result r = lockStream(app, ts, frame, kStreams(app));
    if (!r)
        return {};
    if (cdStream(app, kStreams(app)).freeBlocks == 0)
    {
        if (mind == 0)
        {
            if (!unlockStream(app, ts, frame, kStreams(app)))
                return {};
            return 0;
        }
        if (!fail(app, ts, esp - 5 * 4, Where{kCdFile, 0x54a880}, 0xae6, {0x54a890}, u32(*r)))
            return {};
    }
    const u32 s = kStreams(app);
    const u32 b = cdStream(app, s).freeBlocks;
    cdStream(app, s).freeBlocks = block(app, b).next;
    block(app, b).next = 0;
    if (!unlockStream(app, ts, frame, s))
        return {};
    return int32_t(b);
}

/* streamcommanda: command `command` with argument `argument` and file `name`
 * queued to stream `stream`, the reader woken; `urgent` says to break into
 * what it does, `mind` to mind no block being free.  1, 0 for no block.
 * `esp` is its own, at the return address (its two words above it). */
Result queueCommand(App* app, ThreadState& ts, u32 esp, u32 stream, u32 name, u32 command, u32 argument,
                    u32 urgent, u32 mind)
{
    const u32 frame = esp - 3 * 4 - 4;
    store<u32>(app, frame, command);
    if (kStreams(app) == 0)
    {
        if (mind != 0 && !fail(app, ts, frame, Where{kCdFile, 0x54a6a0}, 0x328, {0x54a6b0}, stream))
            return {};
        return 0;
    }
    CdStream& s = cdStream(app, stream);
    if (!lockStream(app, ts, frame, stream))
        return {};
    const Result got = acquireBlock(app, ts, frame - 4, mind);
    if (!got)
        return {};
    const u32 b = u32(*got);
    if (b == 0)
    {
        if (!unlockStream(app, ts, frame, stream))
            return {};
        return 0;
    }
    directFrom<kStrncpy, watcom::clib::strncpy>(app, ts, frame, b, name, 0x8fu);
    if (ts.terminate)
        return {};
    block(app, b).name[0x8e] = 0;
    block(app, b).argument = argument;
    block(app, b).command = load<u32>(app, frame);
    if (s.queuedLast != 0)
    {
        block(app, s.queuedLast).next = b;
        s.queuedLast = b;
    }
    else
    {
        s.queuedLast = b;
        s.queued = b;
    }
    if (urgent != 0 && s.command == 0)
        s.command = 0xf;
    if (!unlockStream(app, ts, frame, stream) || !wakeReader(app, ts, frame, stream))
        return {};
    return 1;
}

/* purgestreamcommanda: the commands queued to stream `stream` dropped and
 * command `command` (argument `argument`, file `name`) queued alone, the
 * reader woken -- and, for those that stop it (0xb, 0xd, 0x14), waited for.
 * `mind` to mind no block being free.  1. */
Result replaceCommands(App* app, ThreadState& ts, u32 esp, u32 stream, u32 name, u32 command, u32 argument,
                       u32 mind)
{
    const u32 frame = esp - 3 * 4 - 4;
    store<u32>(app, frame, argument);
    if (kStreams(app) == 0)
    {
        if (mind != 0 && !fail(app, ts, frame, Where{kCdFile, 0x54a6ec}, 0x34e, {0x54a700}, stream))
            return {};
        return 0;
    }
    CdStream& s = cdStream(app, stream);
    if (!lockStream(app, ts, frame, stream))
        return {};
    while (s.queued != 0)
    {
        const u32 b = s.queued;
        s.queued = block(app, b).next;
        if (!releaseBlock(app, ts, frame - 4, b))
            return {};
    }
    const Result got = acquireBlock(app, ts, frame - 4, mind);
    if (!got)
        return {};
    const u32 b = u32(*got);
    directFrom<kStrncpy, watcom::clib::strncpy>(app, ts, frame, b, name, 0x8fu);
    if (ts.terminate)
        return {};
    block(app, b).name[0x8e] = 0;
    block(app, b).command = command;
    block(app, b).argument = load<u32>(app, frame);
    s.queuedLast = b;
    s.queued = b;
    if (s.file != 0 && s.command == 0)
        s.command = 0xf;
    if (!unlockStream(app, ts, frame, stream) || !wakeReader(app, ts, frame, stream))
        return {};
    if ((command == 0xb || command == 0xd || command == 0x14)
        && !directFrom<kWaitEvent, ea::eventWaited>(app, ts, frame, s.readerEvent))
        return {};
    return 1;
}

constexpr u32 kNoName = 0x54a744;  // the name the commands by argument are queued with

}  // namespace

/* The commands by name (streamOpen, streamSeek...): a command queued with the
 * wrapper's file (or argument), the wrapper's two pushed registers and the
 * command's words below the wrapper's `esp`. */
Result purgeNamed(App* app, ThreadState& ts, u32 esp, u32 stream, u32 file, Command command, bool mind)
{
    return replaceCommands(app, ts, esp - 2 * 4 - 4 - 4, stream, file, u32(command), 0, mind ? 1 : 0);
}

Result purgeArgument(App* app, ThreadState& ts, u32 esp, u32 stream, u32 argument, Command command, bool mind)
{
    return replaceCommands(app, ts, esp - 2 * 4 - 4 - 4, stream, kNoName, u32(command), argument, mind ? 1 : 0);
}

Result queueNamed(App* app, ThreadState& ts, u32 esp, u32 stream, u32 file, Command command, bool urgent, bool mind)
{
    return queueCommand(app, ts, esp - 2 * 4 - 4 - 8, stream, file, u32(command), 0, urgent ? 1 : 0, mind ? 1 : 0);
}

Result queueArgument(App* app, ThreadState& ts, u32 esp, u32 stream, u32 argument, Command command, bool urgent,
                     bool mind)
{
    return queueCommand(app, ts, esp - 2 * 4 - 4 - 8, stream, kNoName, u32(command), argument, urgent ? 1 : 0,
                        mind ? 1 : 0);
}

/* 0x4e4420 (initstreamstructa): stream `s` set up -- its ring (its size at
 * +0x84, where it is at +0x88), the command blocks after it, blocks of
 * `blockSize` read at once -- and its reader thread started; `first` says the
 * stream is the first. */
Result setUpStream(App* app, ThreadState& ts, u32 esp, u32 stream, u32 blockSize, u32 first)
{
    const u32 frame = esp - 3 * 4;
    u32 eax = stream;
    if (first != 0 && kStreams(app) != 0)
    {
        const Result r = fail(app, ts, frame, Where{kCdFile, 0x54a42c}, 0x100, {0x54a440}, eax);
        if (!r)
            return {};
        eax = u32(*r);
    }
    if (first != 0 && int32_t(blockSize) < 0x2000)
    {
        const Result r = fail(app, ts, frame, Where{kCdFile, 0x54a42c}, 0x105, {0x54a47c}, eax);
        if (!r)
            return {};
        eax = u32(*r);
    }
    CdStream& s = cdStream(app, stream);
    kStreamWalk.set(app, stream);
    kStreams.set(app, stream);
    chainBlocks(app, stream + 0xc0, kBlockCount(app));
    s.ringSize = s.ringBytes;
    s.ringStart = s.ring;
    s.ringEnd = s.ring + s.ringBytes;
    s.first = first;
    s.freedAt = s.ringStart;
    s.readAt = s.freedAt;
    s.writeAt = s.readAt;
    s.chunkStart = s.writeAt;
    s.busy = 0;
    s.block = blockSize;
    s.file = 0;
    s.next = 0;
    s.queued = 0;
    s.queuedLast = s.queued;
    s.state = 7;
    s.command = 0;
    s.other30[2] = 0;
    s.other30[1] = s.other30[2];
    s.bytesPlayed = 0;
    s.bytesRead = s.bytesPlayed;
    s.other30[0] = 0;
    // (These two take no registers.)
    if (!took(directFrom<kCreateEvent, ea::newAutoEvent>(app, ts, frame), eax))
        return {};
    s.readerEvent = eax;
    if (!took(directFrom<kNewLock, ea::newLock>(app, ts, frame), eax))
        return {};
    s.lock = eax;
    if (!took(directFrom<kCreateThread, ea::threadStarted>(app, ts, frame - 4, kReader, 0u, 1u, stream + 0x8c), eax))
        return {};
    if (eax == 0)
    {
        const Result r = fail(app, ts, frame, Where{kCdFile, 0x54a42c}, 0x12d, {0x54a4ac}, eax);
        if (!r)
            return {};
    }
    if (!directFrom<kWaitEvent, ea::eventWaited>(app, ts, frame, s.readerEvent))
        return {};
    return int32_t(stream);
}

/* 0x4e45f0 (initstreama): a stream of `size` bytes (whole pages) allocated in
 * memory pool `pool` -- its ring and the stream with its command blocks, their
 * pages touched -- and set up with blocks of `blockSize`; `first` says the
 * first. */
Result makeStream(App* app, ThreadState& ts, u32 esp, u32 size, u32 blockSize, u32 pool, u32 first)
{
    const u32 frame = esp - 3 * 4 - 4;
    store<u32>(app, frame, blockSize);
    if (size & 0xfff)
        size = (size + 0x1000) & 0x7ffff000;
    kWhere.set(app, 0, kCdFile);
    kWhere.set(app, 1, 0x54a4ec);
    kWhere.set(app, 2, 0x188);
    u32 ring;
    if (!took(directFrom<kAllocate, ea::reserved>(app, ts, frame, 0x54a4f8u, size, pool), ring))
        return {};
    if (ring == 0 && first != 0 && !fail(app, ts, frame, Where{kCdFile, 0x54a4ec}, 0x18a, {0x54a504}, ring))
        return {};
    kWhere.set(app, 1, 0x54a4ec);
    kWhere.set(app, 0, kCdFile);
    kWhere.set(app, 2, 0x18e);
    const u32 structSize = ((kBlockCount(app) * 0x9c + 0xf) & 0xfff0) + 0xc0;
    u32 s;
    if (!took(directFrom<kAllocate, ea::reserved>(app, ts, frame, 0x54a520u, structSize, pool), s))
        return {};
    directFrom<kFillZero, ea::clearBytes>(app, ts, frame, s, structSize);
    if (ts.terminate)
        return {};
    cdStream(app, s).ring = ring;
    cdStream(app, s).ringBytes = size;
    if (!directFrom<kTouch, ea::memoryTouched>(app, ts, frame, ring, size)
        || !directFrom<kTouch, ea::memoryTouched>(app, ts, frame, s, structSize))
        return {};
    return setUpStream(app, ts, frame - 4, s, load<u32>(app, frame), first);
}

/* 0x4e4a30 (delstreamstruct): the first stream's reader stopped (8) -- waited
 * for while it works -- and its thread, events and lock gone. */
Result deleteReader(App* app, ThreadState& ts, u32 esp, u32 stream)
{
    const u32 frame = esp - 5 * 4;
    u32 eax = stream;
    if (kStreams(app) == 0)
    {
        const Result r = fail(app, ts, esp - 6 * 4, Where{kCdFile, 0x54a5a4}, 0x25d, {0x54a5b4}, eax);
        if (!r)
            return {};
        eax = u32(*r);
    }
    const u32 first = kStreams(app);
    if (stream != first)
        return int32_t(eax);
    CdStream& s = cdStream(app, first);
    s.command = 8;
    const u32 deadline = u32(kTimer(app)) + 0x2710;
    if (s.busy == 1)
        do
        {
            safepoint(app, ts);
            if (kTimer(app) > int32_t(deadline))
            {
                const Result r = fail(app, ts, frame, Where{kCdFile, 0x54a5a4}, 0x273, {0x54a5f4}, eax);
                if (!r)
                    return {};
                eax = u32(*r);
            }
        } while (cdStream(app, stream).busy == 1);
    s.command = 8;
    if (!wakeReader(app, ts, frame, kStreams(app)))
        return {};
    if (!directFrom<kWaitEvent, ea::eventWaited>(app, ts, frame, s.readerEvent)
        || !directFrom<kCloseHandle, ea::eventClosed>(app, ts, frame, s.readerEvent)
        || !directFrom<kFreeLock, ea::lockFreed>(app, ts, frame, s.lock))
        return {};
    s.lock = 0;
    // The thread in eax; ebx (the stale handle the main thread's case would go on with) 0.
    if (!took(directFrom<kRemoveThread, ea::endThread>(app, ts, frame, stream + 0x8c, 0u), eax))
        return {};
    kStreams.set(app, 0);
    return int32_t(eax);
}

/* 0x4e4b50 (delstream): stream `stream` freed -- the first with every stream
 * after it, another unlinked -- and the streams no more. */
Result removeStream(App* app, ThreadState& ts, u32 esp, u32 stream)
{
    const u32 frame = esp - 5 * 4;
    u32 s = stream, eax = stream;
    if (kStreams(app) == 0)
    {
        const Result r = fail(app, ts, esp - 6 * 4, Where{kCdFile, 0x54a638}, 0x2bc, {0x54a644}, eax);
        if (!r)
            return {};
        eax = u32(*r);
    }
    const u32 first = kStreams(app);
    if (s == first)
    {
        Result r = deleteReader(app, ts, frame - 4, first);
        if (!r)
            return {};
        eax = u32(*r);
        u32 next = first;
        do
        {
            next = cdStream(app, next).next;
            if (!took(directFrom<kFree, ea::purgeBlock>(app, ts, frame, cdStream(app, s).ring), eax)
                || !took(directFrom<kFree, ea::purgeBlock>(app, ts, frame, s), eax))
                return {};
            s = next;
        } while (next != 0);
    }
    else
    {
        u32 c = first;
        if (cdStream(app, first).next != 0)
            while (s != cdStream(app, c).next)
            {
                c = cdStream(app, c).next;
                if (cdStream(app, c).next == 0)
                    break;
            }
        if (s != cdStream(app, c).next)
        {
            const Result r = fail(app, ts, frame, Where{kCdFile, 0x54a638}, 0x2d9, {0x54a680}, eax);
            if (!r)
                return {};
            eax = u32(*r);
        }
        cdStream(app, c).next = cdStream(app, s).next;
        if (!took(directFrom<kFree, ea::purgeBlock>(app, ts, frame, cdStream(app, s).ring), eax)
            || !took(directFrom<kFree, ea::purgeBlock>(app, ts, frame, s), eax))
            return {};
    }
    kStreams.set(app, 0);
    kStreamWalk.set(app, 0);
    return int32_t(eax);
}

/* 0x4e4790: the first stream made -- makeStream's, `first` 1 (in ecx, which
 * the code pushes and pops around the call). */
Result makeFirstStream(App* app, ThreadState& ts, u32 esp, u32 size, u32 blockSize, u32 pool)
{
    return makeStream(app, ts, esp - 1 * 4 - 4, size, blockSize, pool, 1);
}

/* 0x4e4b50 as the game's sound calls it: removeStream; edx, ebx and ecx are
 * not used (the code pushes them first). */
Result deleteCdStream(App* app, ThreadState& ts, u32 esp, u32 stream, u32 edx, u32 ebx, u32 ecx)
{
    static_cast<void>(edx);
    static_cast<void>(ebx);
    static_cast<void>(ecx);
    return removeStream(app, ts, esp, stream);
}

namespace
{

/* The reader thread's work, on the first stream ([0x5645b8]) and the streams
 * after it (each takes the chunks whose ids its mask and id match).  Each wait
 * is for the event at +0xa8, the reader marked waiting (+0xb4) the while; a
 * command arriving (+0x24) breaks a wait off and sends the reader back to take
 * it.  The steps return false when the game stopped. */
struct Reader
{
    App* app;
    ThreadState& ts;
    u32 frame;  // the thread's local: the file's block size

    u32 first() const { return kStreams(app); }
    u32 walk() const { return kStreamWalk(app); }
    CdStream& s() const { return cdStream(app, first()); }
    CdStream& w() const { return cdStream(app, walk()); }
    bool command() const { return s().command != 0; }

    bool lock() { return bool(lockStream(app, ts, frame, first())); }
    bool unlock() { return bool(unlockStream(app, ts, frame, first())); }
    bool wait() { return bool(directFrom<kWaitEvent, ea::eventWaited>(app, ts, frame, s().streamEvent)); }

    bool move(u32 from, u32 to, u32 bytes)
    {
        return directFrom<kMemmove, ea::moveMemory>(app, ts, frame, to, from, bytes);
    }

    /* The room asked for. */
    bool answer(const Result& r, u32& room)
    {
        if (r)
            room = u32(*r);
        return bool(r);
    }

    bool toEnd(u32 stream, u32& room) { return answer(ringRoomToEnd(app, ts, frame - 4, stream), room); }
    bool ahead(u32 stream, u32& room) { return answer(ringRoomAhead(app, ts, frame - 4, stream), room); }
    bool atStart(u32 stream, u32& room) { return answer(ringRoomAtStart(app, ts, frame - 4, stream), room); }

    /* A command block given back to the free list. */
    bool release(u32 b) { return bool(releaseBlock(app, ts, frame - 4, b)); }

    /* Waited while `busy` says so and no command comes.  1 to go on, 2 for a
     * command, 0 the game stopped. */
    template <typename Busy>
    int waitWhile(Busy busy)
    {
        for (;;)
        {
            if (command())
                break;
            bool b;
            if (!busy(b))
                return 0;
            if (!b)
                break;
            s().waiting = 1;
            if (!wait())
                return 0;
        }
        s().waiting = 0;
        return command() ? 2 : 1;
    }

    /* Until the room to the ring's end (stream `stream`) is all its room. */
    int waitTillEndRoomFree(u32 stream)
    {
        return waitWhile([&](bool& busy) {
            u32 a, b;
            if (!toEnd(stream, a) || !ahead(stream, b))
                return false;
            busy = a != b;
            return true;
        });
    }

    /* Until more than `bytes` are free ahead (roomAhead), of stream `stream`. */
    int waitTillAhead(u32 stream, u32 bytes, bool signedCompare = true)
    {
        return waitWhile([&](bool& busy) {
            u32 a;
            if (!ahead(stream, a))
                return false;
            busy = signedCompare ? int32_t(a) <= int32_t(bytes) : a <= bytes;
            return true;
        });
    }

    /* Until more than `bytes` are free at the ring's start (roomAtStart). */
    int waitTillStart(u32 stream, u32 bytes, bool signedCompare = true)
    {
        return waitWhile([&](bool& busy) {
            u32 a;
            if (!atStart(stream, a))
                return false;
            busy = signedCompare ? int32_t(a) <= int32_t(bytes) : a <= bytes;
            return true;
        });
    }

    /* The blocks read into `to`, tried again (a ms between) until read. */
    bool readBlocks(u32 to)
    {
        for (;;)
        {
            u32 eax;
            if (!took(directFrom<kReadBlock, ea::blocksRead>(app, ts, frame, s().file, to, s().blocksAtOnce), eax))
                return false;
            if (eax != 0)
                return true;
            if (!directFrom<kSleep, ea::sleepFor>(app, ts, frame, 1u))
                return false;
        }
    }

    /* The file of the command taken opened -- its block size the local -- or,
     * not found, traced and the reader idle.  `opened` false for that. */
    bool open(bool& opened, u32 line)
    {
        if (s().file != 0 && !directFrom<kCloseBlock, ea::slotClosed>(app, ts, frame, s().file))
            return false;
        u32 eax;
        if (!took(directFrom<kOpenBlock, ea::blockOpened>(app, ts, frame - 4, s().doing, first() + 0x1c, first() + 0x50,
                                                          first() + 0x58, frame),
                  eax))
            return false;
        opened = eax != 0;
        if (!opened)
        {
            if (!directFrom<kTrace, ea::monoPrinted>(app, ts, frame - 2 * 4, 0u, 0x54a764u, guest::Words({ s().doing }))
                || !release(s().doing))
                return false;
            s().file = 0;
            s().state = 7;
            return true;
        }
        if (!release(s().doing) || !lock())
            return false;
        const u32 size = load<u32>(app, frame);
        if (int32_t(size) > int32_t(s().block))
        {
            s().blocksAtOnce = 1;
            if (int32_t(size) > int32_t(s().ringSize)
                && !fail(app, ts, frame, Where{kCdFile, 0x54a788}, line, {0x54a79c, size, s().ringSize}, 0))
                return false;
            if (int32_t(size + size) > int32_t(s().ringSize)
                && !directFrom<kTrace, ea::monoPrinted>(app, ts, frame - 4, 0u, 0x54a7d0u, guest::Words({})))
                return false;
            s().block = size;
        }
        else
        {
            s().blocksAtOnce = u32(idiv(int32_t(s().block), int32_t(size)));
            s().block = size * s().blocksAtOnce;
        }
        s().fileWanted = s().fileStart;
        s().fileEnd = s().fileStart + s().fileLength;
        return true;
    }
};

}  // namespace

/* 0x4e53c0 (streamreaderthread): the reader of the CD streams, on a thread of
 * its own: the commands taken -- open a file, go on, seek, stop -- and the
 * file read block by block into the rings, each chunk to the stream whose mask
 * takes its id, the reader waiting while a ring is full. */
Result readerThread(App* app, ThreadState& ts, u32 esp)
{
    const u32 frame = esp - 6 * 4 - 4;
    Reader r{app, ts, frame};
    u32 eax;
    if (!took(directFrom<kCreateEvent, ea::newAutoEvent>(app, ts, frame), eax))
        return {};
    r.s().streamEvent = eax;
    if (!directFrom<kSetEvent, eventPostedWith>(app, ts, frame, 0u, r.s().readerEvent))
        return {};
    bool ending = false;  // the chunk read is the end's
    for (;;)
    {
        safepoint(app, ts);
        if (!r.lock())
            return {};
        if (r.s().command == 8)
            r.s().state = 8;
        else if (r.s().command == 0xf)
            r.s().state = 7;
        r.s().command = 0;
        if (!r.unlock())
            return {};
        const u32 state = r.s().state;
        if (state == 1 || state == 0x11 || state == 0x12 || state == 0x13)
        {
            bool opened;
            if (!r.open(opened, 0x577))
                return {};
            if (!opened)
                continue;
            if (r.s().state == 1 || r.s().state == 0x12)
            {
                r.s().chunkStart = r.s().readAt;
                r.s().bytesRead = 0;
            }
            r.s().writeAt = r.s().chunkStart;
            r.s().state = r.s().state == 0x13 || r.s().state == 0x12 ? 7 : 0x10;
            if (!r.unlock())
                return {};
            continue;
        }
        if (state == 7)
        {
            if (!passFreed(app, ts, frame - 4, r.first()))
                return {};
            if (r.s().queued == 0)
            {
                if (!r.wait())
                    return {};
                continue;
            }
            if (!r.lock())
                return {};
            const u32 c = r.s().queued;
            r.s().queued = block(app, c).next;
            r.s().doing = c;
            if (r.s().queued == 0)
                r.s().queuedLast = 0;
            r.s().state = block(app, c).command;
            r.s().fileWanted = r.s().fileStart + block(app, c).argument;
            const u32 k = block(app, c).command;
            if (!(k == 0xd || k == 0x14 || k == 1 || k == 0x11 || k == 0x12 || k == 0x13) && !r.release(c))
                return {};
            if (!r.unlock())
                return {};
            continue;
        }
        if (state == 8)
        {
            if (r.s().file != 0 && !directFrom<kCloseBlock, ea::slotClosed>(app, ts, frame, r.s().file))
                return {};
            r.s().file = 0;
            if (!directFrom<kCloseHandle, ea::eventClosed>(app, ts, frame, r.s().streamEvent))
                return {};
            if (!took(directFrom<kSetEvent, eventPostedWith>(app, ts, frame, 0u, r.s().readerEvent), eax))
                return {};
            return int32_t(eax);
        }
        bool reading = state == 2 || state == 0x10;
        if (state == 0xd || state == 0x14 || state == 0xb)
        {
            if (state != 0xb)
            {
                bool opened;
                if (!r.open(opened, 0x5a5))
                    return {};
                if (!opened)
                    continue;
                if (!r.unlock())
                    return {};
            }
            kStreamWalk.set(app, r.first());
            if (!r.lock())
                return {};
            while (r.walk() != 0)
            {
                CdStream& w = r.w();
                w.freedAt = w.ringStart;
                w.readAt = w.freedAt;
                w.writeAt = w.readAt;
                w.chunkStart = w.writeAt;
                w.bytesPlayed = 0;
                w.bytesRead = w.bytesPlayed;
                kStreamWalk.set(app, w.next);
            }
            if (!r.unlock())
                return {};
            if (r.s().state == 0x14)
            {
                r.s().state = 7;
                if (!directFrom<kSetEvent, eventPostedWith>(app, ts, frame, 0u, r.s().readerEvent))
                    return {};
                continue;
            }
            r.s().state = 2;
            if (!directFrom<kSetEvent, eventPostedWith>(app, ts, frame, 0u, r.s().readerEvent))
                return {};
            reading = true;
        }
        int go = 1;
        if (reading)
        {
            // A read started: from the chunk's start at the offset asked, the
            // part of the first block before it skipped.
            if (!r.lock())
                return {};
            if (r.s().state == 2)
            {
                r.s().chunkStart = r.s().readAt;
                r.s().writeAt = r.s().chunkStart;
                r.s().bytesRead = 0;
            }
            else
                r.s().writeAt = r.s().chunkStart;
            r.s().filePlace = r.s().fileWanted;
            if (!r.unlock())
                return {};
            if (!took(directFrom<kSeekBlock, ea::blocksSought>(app, ts, frame, r.s().file, r.s().filePlace), eax))
                return {};
            const u32 at0 = eax;
            if (!r.lock())
                return {};
            r.s().skip = r.s().filePlace - at0;
            r.s().chunkHeld = r.s().block - r.s().skip;
            if (!r.unlock())
                return {};
            u32 room;
            if (!r.toEnd(r.first(), room))
                return {};
            if (int32_t(room) <= int32_t(r.s().block))
            {
                if ((go = r.waitTillEndRoomFree(r.first())) != 1)
                {
                    if (go == 0)
                        return {};
                    continue;
                }
                if ((go = r.waitTillStart(r.first(), load<u32>(app, frame))) != 1)
                {
                    if (go == 0)
                        return {};
                    continue;
                }
                if (!r.lock())
                    return {};
                record<ChunkHeader>(app, r.s().chunkStart).id = kWrap;
                r.s().chunkStart = r.s().ringStart;
                r.s().writeAt = r.s().chunkStart;
                if (!r.unlock())
                    return {};
            }
            if ((go = r.waitTillAhead(r.first(), r.s().block)) != 1)
            {
                if (go == 0)
                    return {};
                continue;
            }
            if (!r.readBlocks(r.s().writeAt) || !r.lock())
                return {};
            r.s().filePlace += r.s().chunkHeld;
            if (r.s().skip != 0 && !r.move(r.s().chunkStart + r.s().skip, r.s().chunkStart, r.s().chunkHeld))
                return {};
            r.s().writeAt += r.s().chunkHeld;
            r.s().state = 3;
            if (!r.unlock())
                return {};
        }
        else if (state != 3)
            continue;
        // State 3: a chunk's header in, its whole read and handed to its
        // stream's ring.
        if (r.s().chunkHeld < 8)
        {
            u32 room;
            if (!r.toEnd(r.first(), room))
                return {};
            if (int32_t(room) <= int32_t(r.s().block))
            {
                if ((go = r.waitTillEndRoomFree(r.first())) != 1
                    || (go = r.waitTillStart(r.walk(), r.s().chunkHeld)) != 1)
                {
                    if (go == 0)
                        return {};
                    continue;
                }
                if (!r.lock() || !r.move(r.s().chunkStart, r.s().ringStart, r.s().chunkHeld))
                    return {};
                record<ChunkHeader>(app, r.s().chunkStart).id = kWrap;
                r.s().chunkStart = r.s().ringStart;
                r.s().writeAt = r.s().chunkStart;
                r.s().writeAt += r.s().chunkHeld;
                if (!r.unlock())
                    return {};
            }
            if ((go = r.waitTillAhead(r.first(), r.s().block)) != 1)
            {
                if (go == 0)
                    return {};
                continue;
            }
            if (!r.readBlocks(r.s().writeAt) || !r.lock())
                return {};
            r.s().writeAt += r.s().block;
            r.s().filePlace += r.s().block;
            r.s().chunkHeld += r.s().block;
            if (!r.unlock())
                return {};
        }
        if (swapped(record<ChunkHeader>(app, r.s().chunkStart).id) == kEndId && r.s().queued != 0)
        {
            r.s().state = 7;
            continue;
        }
        if (!r.lock())
            return {};
        r.s().chunkLength = record<ChunkHeader>(app, r.s().chunkStart).size;
        r.s().chunkLeft = r.s().chunkLength - r.s().chunkHeld;
        kStreamWalk.set(app, r.s().next);
        while (r.walk() != 0)
        {
            const u32 id = swapped(record<ChunkHeader>(app, r.s().chunkStart).id);
            if ((id & r.w().mask) == r.w().id)
                break;
            kStreamWalk.set(app, r.w().next);
        }
        if (r.walk() == 0)
            kStreamWalk.set(app, r.first());
        if (!r.unlock())
            return {};
        const u32 length = r.s().chunkLength;
        if (length < 8 || int32_t(r.w().ringSize) / 2 < int32_t(length))
        {
            if (r.s().first == 0)
            {
                // A chunk that cannot be: every stream's ring given an empty
                // chunk of the stream's id, and the reader idle.
                if (!r.lock())
                    return {};
                kStreamWalk.set(app, r.first());
                while (r.walk() != 0)
                {
                    CdStream& w = r.w();
                    record<ChunkHeader>(app, w.chunkStart).id = w.id;
                    record<ChunkHeader>(app, w.chunkStart).size = 8;
                    w.chunkStart += 8;
                    w.writeAt = w.chunkStart;
                    kStreamWalk.set(app, w.next);
                }
                if (!r.unlock())
                    return {};
                r.s().state = 7;
                continue;
            }
            if (!fail(app, ts, frame, Where{kCdFile, 0x54a788}, 0x638,
                      {0x54a800, r.s().chunkLength, r.w().ringSize, r.s().filePlace}, 0))
                return {};
        }
        // The chunk to its stream.
        if (r.first() == r.walk())
            ending = swapped(record<ChunkHeader>(app, r.s().chunkStart).id) == kEndId;
        else
        {
            ending = false;
            const u32 wk = r.walk();
            u32 room;
            if (!r.toEnd(wk, room))
                return {};
            if (room <= r.s().chunkLength + r.s().block + 8)
            {
                if ((go = r.waitTillEndRoomFree(wk)) != 1 || (go = r.waitTillAhead(wk, 8, false)) != 1
                    || (go = r.waitTillStart(wk, r.s().chunkLength + r.s().block + 8, false)) != 1)
                {
                    if (go == 0)
                        return {};
                    continue;
                }
                if (!r.lock())
                    return {};
                record<ChunkHeader>(app, cdStream(app, wk).chunkStart).id = kWrap;
                cdStream(app, wk).chunkStart = cdStream(app, wk).ringStart;
                cdStream(app, wk).writeAt = cdStream(app, wk).chunkStart;
                if (!r.unlock())
                    return {};
            }
            if (int32_t(r.s().chunkLeft) >= 0)
            {
                if ((go = r.waitTillAhead(wk, r.s().chunkHeld)) != 1)
                {
                    if (go == 0)
                        return {};
                    continue;
                }
                if (!r.lock() || !r.move(r.s().chunkStart, cdStream(app, wk).chunkStart, r.s().chunkHeld))
                    return {};
                cdStream(app, wk).writeAt = cdStream(app, wk).chunkStart + r.s().chunkHeld;
                r.s().writeAt = r.s().chunkStart;
                r.s().chunkHeld = 0;
            }
            else
            {
                if ((go = r.waitTillAhead(wk, r.s().chunkLength)) != 1)
                {
                    if (go == 0)
                        return {};
                    continue;
                }
                if (!r.lock() || !r.move(r.s().chunkStart, cdStream(app, wk).chunkStart, r.s().chunkLength))
                    return {};
                cdStream(app, wk).writeAt = cdStream(app, wk).chunkStart + r.s().chunkLength;
                r.s().chunkHeld = 0u - r.s().chunkLeft;
                if (!r.move(r.s().chunkLength + r.s().chunkStart, r.s().chunkStart, r.s().chunkHeld))
                    return {};
                r.s().writeAt -= r.s().chunkLength;
            }
            if (!r.unlock())
                return {};
        }
        // What is left of the chunk read on into its stream's ring: room made at
        // the ring's start first when the end has too little.
        if (int32_t(r.s().chunkLeft) > 0)
        {
            const u32 wk = r.walk();
            u32 room;
            if (!r.toEnd(wk, room))
                return {};
            if (room <= r.s().chunkLeft + r.s().block + 8)
            {
                if ((go = r.waitTillEndRoomFree(wk)) != 1 || (go = r.waitTillStart(wk, r.s().chunkHeld)) != 1)
                {
                    if (go == 0)
                        return {};
                    continue;
                }
                if (!r.lock()
                    || !r.move(cdStream(app, wk).chunkStart, cdStream(app, wk).ringStart, r.s().chunkHeld))
                    return {};
                record<ChunkHeader>(app, cdStream(app, wk).chunkStart).id = kWrap;
                cdStream(app, wk).writeAt = cdStream(app, wk).ringStart + r.s().chunkHeld;
                cdStream(app, wk).chunkStart = cdStream(app, wk).ringStart;
                if (!r.unlock())
                    return {};
            }
        }
        while (int32_t(r.s().chunkLeft) > 0)
        {
            const u32 wk = r.walk();
            if ((go = r.waitTillAhead(wk, r.s().block)) != 1)
                break;
            if (!r.readBlocks(cdStream(app, wk).writeAt) || !r.lock())
                return {};
            r.s().filePlace += r.s().block;
            cdStream(app, wk).writeAt += r.s().block;
            r.s().chunkLeft -= r.s().block;
            if (!r.unlock())
                return {};
        }
        if (go == 0)
            return {};
        if (r.command())
            continue;
        // The chunk done: its stream's write end past it.
        if (!r.lock())
            return {};
        const u32 wk = r.walk();
        cdStream(app, wk).chunkStart += r.s().chunkLength;
        cdStream(app, wk).bytesRead += r.s().chunkLength - 8;
        if (ending)
        {
            r.s().state = 7;
            if (!r.unlock())
                return {};
            continue;
        }
        if (!r.unlock())
            return {};
        if (wk == r.first())
        {
            if (!r.lock())
                return {};
            r.s().chunkHeld = cdStream(app, wk).writeAt - cdStream(app, wk).chunkStart;
            if (!r.unlock())
                return {};
        }
        else if (r.s().chunkHeld == 0)
        {
            const u32 over = cdStream(app, wk).writeAt - cdStream(app, wk).chunkStart;
            u32 room;
            if (!r.toEnd(r.first(), room))
                return {};
            if (int32_t(room) <= int32_t(over))
            {
                if ((go = r.waitTillEndRoomFree(r.first())) != 1 || (go = r.waitTillStart(r.first(), over)) != 1)
                {
                    if (go == 0)
                        return {};
                    continue;
                }
                if (!r.lock())
                    return {};
                record<ChunkHeader>(app, r.s().chunkStart).id = kWrap;
                r.s().chunkStart = r.s().ringStart;
                r.s().writeAt = r.s().chunkStart;
                if (!r.unlock())
                    return {};
            }
            if ((go = r.waitTillAhead(r.first(), r.w().writeAt - r.w().chunkStart)) != 1)
            {
                if (go == 0)
                    return {};
                continue;
            }
            if (!r.lock())
                return {};
            const u32 ww = r.walk();
            r.s().chunkHeld = cdStream(app, ww).writeAt - cdStream(app, ww).chunkStart;
            if (!r.move(cdStream(app, ww).chunkStart, r.s().chunkStart, r.s().chunkHeld))
                return {};
            cdStream(app, ww).writeAt = cdStream(app, ww).chunkStart;
            r.s().writeAt = r.s().chunkStart + r.s().chunkHeld;
            if (!r.unlock())
                return {};
        }
        // On: the next chunk while the file has more and no command waits; at
        // the file's end every ring given a dropped-chunk mark.
        if (int32_t(r.s().filePlace - r.s().chunkHeld) < int32_t(r.s().fileEnd))
            continue;
        if (r.s().queued != 0)
        {
            r.s().state = 7;
            continue;
        }
        kStreamWalk.set(app, r.first());
        while (r.walk() != 0)
        {
            const u32 ww = r.walk();
            const int g = r.waitWhile([&](bool& busy) {
                u32 a;
                if (!r.ahead(ww, a))
                    return false;
                busy = a + cdStream(app, ww).chunkHeld <= 8;
                return true;
            });
            if (g == 0)
                return {};
            if (g == 2)
                break;
            kStreamWalk.set(app, cdStream(app, ww).next);
        }
        if (!r.lock())
            return {};
        kStreamWalk.set(app, r.first());
        while (r.walk() != 0)
        {
            const u32 ww = r.walk();
            record<ChunkHeader>(app, cdStream(app, ww).chunkStart).id = kDropped;
            record<ChunkHeader>(app, cdStream(app, ww).chunkStart).size = 8;
            cdStream(app, ww).chunkStart += 8;
            cdStream(app, ww).writeAt = cdStream(app, ww).chunkStart;
            kStreamWalk.set(app, cdStream(app, ww).next);
        }
        if (!r.unlock())
            return {};
        r.s().state = 7;
    }
}

}  // namespace stream

/* The game's entries. */

nfs::u32 freeStreamChunk(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, stream::giveBackChunk(app, ts, e.esp, e.eax, e.edx));
}

nfs::u32 streamBytesRead(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, stream::bytesBuffered(app, e.eax));
}

nfs::u32 readerIdle(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, stream::readerIsIdle(app) ? 1 : 0);
}

nfs::u32 readerWaits(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, stream::readerIsWaiting(app, e.eax));
}

nfs::u32 passFreedChunks(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, stream::passFreed(app, ts, e.esp, e.eax));
}

nfs::u32 roomAhead(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, stream::ringRoomAhead(app, ts, e.esp, e.eax));
}

nfs::u32 roomToEnd(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, stream::ringRoomToEnd(app, ts, e.esp, e.eax));
}

nfs::u32 roomAtStart(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, stream::ringRoomAtStart(app, ts, e.esp, e.eax));
}

nfs::u32 getStreamChunk(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, stream::nextChunkToPlay(app, ts, e.esp, e.eax));
}

nfs::u32 chainCommandBlocks(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, stream::chainBlocks(app, e.eax, e.edx));
}

nfs::u32 freeCommandBlock(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, stream::releaseBlock(app, ts, e.esp, e.eax));
}

nfs::u32 commandBlocksFree(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, stream::commandBlocksAvailable(app, ts, e.esp, e.eax));
}

nfs::u32 takeCommandBlock(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, stream::acquireBlock(app, ts, e.esp, e.eax));
}

/* streamcommanda (stdcall 8): command ebx with argument ecx and file edx
 * queued to stream eax; [esp+4] says to break into what the reader does,
 * [esp+8] to mind no block being free. */
nfs::u32 streamCommand(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, stream::queueCommand(app, ts, e.esp, e.eax, e.edx, e.ebx, e.ecx, e.arg(app, 0), e.arg(app, 1)), 8);
}

/* purgestreamcommanda (stdcall 4): the same, the commands queued before
 * dropped; [esp+4] to mind no block being free. */
nfs::u32 purgeStreamCommand(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, stream::replaceCommands(app, ts, e.esp, e.eax, e.edx, e.ebx, e.ecx, e.arg(app, 0)), 4);
}

namespace
{

/* 0x4e4ea0..0x4e50c0: the commands by name: open file edx (1), open it next
 * (0x11), go on with it (0xd), switch to it (0x12), queue it (0x13), restart
 * with it (0x14); seek to edx (2), read on from edx (0x10), stop at edx (0xb)
 * -- each minding no block or not. */
using stream::Command;

nfs::u32 named(App* app, ThreadState& ts, nfs::Args& args, Command command, bool purge, bool urgent, bool mind)
{
    const Entry e(ts, args);
    return e.finish(ts, args, purge ? stream::purgeNamed(app, ts, e.esp, e.eax, e.edx, command, mind)
                        : stream::queueNamed(app, ts, e.esp, e.eax, e.edx, command, urgent, mind));
}

nfs::u32 numbered(App* app, ThreadState& ts, nfs::Args& args, Command command, bool purge, bool urgent, bool mind)
{
    const Entry e(ts, args);
    return e.finish(ts, args, purge ? stream::purgeArgument(app, ts, e.esp, e.eax, e.edx, command, mind)
                        : stream::queueArgument(app, ts, e.esp, e.eax, e.edx, command, urgent, mind));
}

}  // namespace

nfs::u32 streamOpen(App* app, ThreadState& ts, nfs::Args& args) { return named(app, ts, args, Command::Open, true, false, true); }
nfs::u32 streamOpenQuietly(App* app, ThreadState& ts, nfs::Args& args) { return named(app, ts, args, Command::Open, true, false, false); }
nfs::u32 streamOpenNext(App* app, ThreadState& ts, nfs::Args& args) { return named(app, ts, args, Command::OpenNext, false, false, true); }
nfs::u32 streamOpenNextQuietly(App* app, ThreadState& ts, nfs::Args& args) { return named(app, ts, args, Command::OpenNext, false, false, false); }
nfs::u32 streamContinue(App* app, ThreadState& ts, nfs::Args& args) { return named(app, ts, args, Command::Continue, true, false, true); }
nfs::u32 streamContinueQuietly(App* app, ThreadState& ts, nfs::Args& args) { return named(app, ts, args, Command::Continue, true, false, false); }
nfs::u32 streamSwitch(App* app, ThreadState& ts, nfs::Args& args) { return named(app, ts, args, Command::Switch, true, false, true); }
nfs::u32 streamSwitchQuietly(App* app, ThreadState& ts, nfs::Args& args) { return named(app, ts, args, Command::Switch, true, false, false); }
nfs::u32 streamQueueFile(App* app, ThreadState& ts, nfs::Args& args) { return named(app, ts, args, Command::QueueFile, false, false, true); }
nfs::u32 streamQueueFileQuietly(App* app, ThreadState& ts, nfs::Args& args) { return named(app, ts, args, Command::QueueFile, false, false, false); }
nfs::u32 streamRestart(App* app, ThreadState& ts, nfs::Args& args) { return named(app, ts, args, Command::Restart, true, false, true); }
nfs::u32 streamRestartQuietly(App* app, ThreadState& ts, nfs::Args& args) { return named(app, ts, args, Command::Restart, true, false, false); }
nfs::u32 streamSeek(App* app, ThreadState& ts, nfs::Args& args) { return numbered(app, ts, args, Command::Seek, false, true, true); }
nfs::u32 streamSeekQuietly(App* app, ThreadState& ts, nfs::Args& args) { return numbered(app, ts, args, Command::Seek, false, true, false); }
nfs::u32 streamReadOn(App* app, ThreadState& ts, nfs::Args& args) { return numbered(app, ts, args, Command::ReadOn, false, false, true); }
nfs::u32 streamReadOnQuietly(App* app, ThreadState& ts, nfs::Args& args) { return numbered(app, ts, args, Command::ReadOn, false, false, false); }
nfs::u32 streamStopAt(App* app, ThreadState& ts, nfs::Args& args) { return numbered(app, ts, args, Command::StopAt, true, false, true); }
nfs::u32 streamStopAtQuietly(App* app, ThreadState& ts, nfs::Args& args) { return numbered(app, ts, args, Command::StopAt, true, false, false); }

/* initstreamstructa: the second stream's and the first's. */
nfs::u32 initStreamStruct(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, stream::setUpStream(app, ts, e.esp, e.eax, e.ebx, e.ecx));
}

nfs::u32 initStreamStructSecond(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, stream::setUpStream(app, ts, e.esp - 1 * 4 - 4, e.eax, e.ebx, 0));
}

nfs::u32 initStreamStructFirst(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, stream::setUpStream(app, ts, e.esp - 1 * 4 - 4, e.eax, e.ebx, 1));
}

nfs::u32 initStream(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, stream::makeStream(app, ts, e.esp, e.eax, e.edx, e.ebx, e.ecx));
}

nfs::u32 initStreamSecond(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, stream::makeStream(app, ts, e.esp - 1 * 4 - 4, e.eax, e.edx, e.ebx, 0));
}

nfs::u32 initStreamFirst(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, stream::makeFirstStream(app, ts, e.esp, e.eax, e.edx, e.ebx));
}

nfs::u32 deleteStreamStruct(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, stream::deleteReader(app, ts, e.esp, e.eax));
}

nfs::u32 deleteStream(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, stream::deleteCdStream(app, ts, e.esp, e.eax, e.edx, e.ebx, e.ecx));
}

nfs::u32 streamReaderThread(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, stream::readerThread(app, ts, e.esp));
}

}  // namespace port
}  // namespace nfs3hp
