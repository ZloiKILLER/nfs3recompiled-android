#include "port_snd.h"
#include "port_snd_stream.h"
#include "port_snd_stream_calls.h"
#include "native_ea_threads.h"
#include "native_frontend_wina.h"

#include <cstdint>
#include <cstring>
#include <optional>

/* EA's SNDSTRM (cmn\sst.c, cmn\sstrstat.c) and the packet player under it
 * (cmn\spktplay.c), rewritten (port_snd.h says how): what plays a movie's
 * sound.  The movie's reader hands its chunks -- headers, data, loops, ends --
 * to a stream as requests; the stream's system task parses them, a header
 * starting the packet player on a voice, each data chunk submitted to it as a
 * packet the mixer plays; the packets played and done with are given back
 * through the stream's callbacks.
 *
 * The streams and the packet players stay in the game's memory (the mixer's
 * decoders take the packets from the players, the movie player reads the
 * streams' state); the records below say what is where.  Each function is
 * here as C++ with arguments and a result and, by the name the game's code
 * knows, as the thunk around it.
 *
 * What the code leaves in registers besides eax: only mul64's esi (its movsd
 * leaves it at a copy on its frame) is outside Watcom's rule that a function
 * may trash the registers it is handed arguments in, and the thunks give that back.  Of the
 * trashed ones they give back what is still seen: streamMemory's and
 * nextPacket's edx (the player); packetFramesPlayed's edx (the callback's)
 * no more -- its frames, as a callback that keeps edx leaves it; the rest --
 * whatever the last call left in ebx, ecx and edx -- no caller reads: the
 * movie player calls the streams' entries from 0x495bc0, whose one caller,
 * 0x4961b5, reads none of them, by the x86's liveness at each call site
 * followed through the returns; the rest of the callers are the module's
 * own.
 *
 * The movie file's chunk reader the streams take their chunks from is the
 * menus' (native_frontend_wina.cpp), its typed cores called as C++ (wina
 * below); the system tasks are the streamer's (port_snd_stream.cpp). */
namespace nfs3hp
{
namespace port
{

namespace
{

#pragma pack(push, 1)

/* A sound's format as the packet player takes it: its rate and, in a word,
 * its channels (the low 2 bits) and its representation (the rest). */
struct StreamHeader
{
    uint16_t rate;
    uint16_t format;
};
static_assert(sizeof(StreamHeader) == 4, "a stream's header");

/* What a patch says of the play, as iSNDpatchtohdr reads it from the tags. */
struct PatchSettings
{
    uint32_t voiceMask;       // tag 5
    int16_t pitch;
    int8_t priority;          // tag 6: the voice's priority
    int8_t level;             // 0x7f
    int8_t pan;               // 0x40
    int8_t send;              // tag 0x13
    int8_t bendRange;         // tag 0xa
    int8_t playPriority;      // tag 0x91
};
static_assert(sizeof(PatchSettings) == 0xc, "a patch's settings");

/* A request of a stream's.  Its chunks are the reader's; its handle is the
 * stream's serial and the stream's index. */
struct Request
{
    reg32 chunks;
    reg32 handle;
    int32_t submitted;        // the first packet's submission, -1 until then
    int32_t lastSubmitted;    // and the last one's
    reg32 dataRate;           // bytes a second, times 256
    reg32 played;             // frames
    reg32 frames;
    reg32 inPlayer;           // frames submitted
    int32_t packets;
    reg32 loop;
    int16_t waits;            // the time it waits for before it parses on
    uint16_t other2a;
};
static_assert(sizeof(Request) == 0x2c, "a request");

/* One of the 16 streams at 0x9f254c. */
struct Stream
{
    reg32 requests;           // the stream's requests
    reg32 queue;              // and its chunks' reader
    int32_t voice;            // the voice the packet player plays on, -1 none
    int32_t player;           // the packet player
    int32_t serial;           // the requests', 0x100 a request
    uint8_t state;            // the player's: 0 stopped, 1 playing, 2 a new header waiting
    int8_t takes;             // the requests it takes
    int8_t holds;             // and those it holds
    int8_t playing;           // the one playing
    uint8_t queueGiven;       // whether the queue is the caller's
    uint8_t other19[3];
    StreamHeader playingHeader;
    StreamHeader parsedHeader;
    PatchSettings playingSettings;
    PatchSettings parsedSettings;
    PlayOptions options;
};
static_assert(sizeof(Stream) == 0x4c, "a stream");

/* A packet in a player's ring. */
struct Packet
{
    int32_t serial;
    int32_t frames;
    reg32 data;
};

/* A packet as a caller offers it to SNDPKTPLAY_submit. */
struct Offered
{
    int32_t other0;
    int32_t frames;
    int32_t other8;
    reg32 data;
};

/* One of the 16 packet players at 0xa0b044; its ring of packets follows. */
struct PacketPlayer
{
    int32_t voice;            // the voice's handle, -1 stopped
    int32_t serial;           // the packets', counted up
    int16_t size;             // the ring's packets
    uint16_t head;
    int16_t playing;          // the packet the mixer has, -1 none
    uint16_t count;           // the packets in the ring
    int32_t submitted;        // frames
    int32_t inHardware;       // those the mixer holds
    reg32 memory;
    reg32 freed;              // the packets' free callback
    reg32 framesCallback;
    StreamHeader header;
    // The ring: Packet [size] from here.
};
static_assert(sizeof(PacketPlayer) == 0x28, "a packet player");

/* A request's state, as SNDSTRM_requeststatus gives it. */
struct RequestStatus
{
    int32_t state;            // 3 gone, 2 playing, 1 waiting, 0 not started
    reg32 played;             // ms
    reg32 left;
    reg32 buffered;
};

#pragma pack(pop)

constexpr Table<reg32> kStreams{0x9f254c};         // the 16 streams
constexpr Table<int8_t> kPlayerStream{0x9f258c};   // each packet player's stream
constexpr Table<reg32> kPlayers{0xa0b044};         // the 16 packet players
constexpr Var<reg32> kStreamerRestore{0xa09b30};   // the restore SNDrestore calls
constexpr reg32 kRequestSize = sizeof(Request);
constexpr reg32 kRingOffset = sizeof(PacketPlayer);
// What a packet player takes besides its packets: its head and one packet more.
constexpr reg32 kPlayerOverhead = sizeof(PacketPlayer) + sizeof(Packet);

// Where the errors come from: the source's name and the function's.
constexpr Where kStatusError{0x54cc50, 0x54cc60}, kParseHeaderError{0x54ccb4, 0x54ccc0};
constexpr Where kParseDataError{0x54ccb4, 0x54cd14}, kTaskError{0x54ccb4, 0x54cd5c};
constexpr Where kCreateError{0x54ccb4, 0x54cdac};
constexpr Where kSubmitError{0x54fdc0, 0x54fdd0}, kSpaceError{0x54fdc0, 0x54fe20};
constexpr Where kOutstandingError{0x54fdc0, 0x54fe78}, kPurgeError{0x54fdc0, 0x54ff5c};
constexpr Where kStopError{0x54fdc0, 0x54ffac}, kHeaderError{0x54fff4, 0x550004};
constexpr Where kRateError{0x55006c, 0x55007c};

// The streams' own functions, as the system tasks and the packet player know them.
constexpr reg32 kStreamTask = 0x4f3ebc, kStreamRestore = 0x4f3a90;
constexpr reg32 kFramesCallback = 0x4f3b60, kFreedCallback = 0x4f3b44;

// What they call of the game's: the movie file's chunk reader, a surface's lock.
constexpr reg32 kFreeChunk = 0x4d03c0, kNextChunk = 0x4d0300, kQueueBytes = 0x4d0460, kQueueState = 0x4d0490;
constexpr reg32 kQueueRequest = 0x4cfef0, kQueueRequestWhole = 0x4cffb0, kQueueMake = 0x4cfa80;
constexpr reg32 kQueueGone = 0x4cfd30, kQueueEmptied = 0x4d0220, kQueueBlock = 0x4d0500;
constexpr reg32 kQueueChunksAtOnce = 0x4cfdd0;

// The chunks' ids ('SCdl' and the others).
constexpr reg32 kIdData = 0x6c444353, kIdHeader = 0x6c484353, kIdEnd = 0x6c454353, kIdLoop = 0x6c434353;

constexpr int32_t kFull = -13;
constexpr reg32 kMaxQueuedBytes = 0x3d0900;

/* A loop's safepoint, as the game's code has one at each loop's head: the
 * guest's context handed over when another thread waits for it. */
void safepoint(App* app, CPU& cpu)
{
    if (app->contextWanted())
        app->yieldContext(cpu);
}

Stream& stream(App* app, reg32 s) { return record<Stream>(app, s); }
Request& request(App* app, reg32 r) { return record<Request>(app, r); }
PacketPlayer& packetPlayer(App* app, reg32 p) { return record<PacketPlayer>(app, p); }

/* A ring's packet. */
Packet& ringPacket(App* app, reg32 p, int32_t i)
{
    return record<Packet>(app, p + kRingOffset + reg32(i) * sizeof(Packet));
}

/* The packet player of index `index`. */
reg32 playerOf(App* app, int32_t index) { return kPlayers(app, index); }

/* The request a stream plays. */
reg32 playingRequest(const Stream& s) { return s.requests + reg32(int32_t(s.playing)) * kRequestSize; }

/* The call of a callback (through its pointer) with eax only, the other
 * registers as they are. */
bool callBack(App* app, CPU& cpu, reg32 esp, reg32 target, reg32 eax)
{
    cpu.esp = esp;
    return guest::pointerCall(app, cpu, target, {eax}).has_value();
}

/* A typed core of the movie file's chunk reader (native_frontend_wina.cpp)
 * called as C++ in place of the game's call of `Address`, esp first where the
 * game's code has it: its eax, nothing when the game stopped in it (its calls
 * into the game throw fe::Terminated), every register but eax kept. */
template <reg32 Address, auto Core, typename... A>
std::optional<reg32> wina(App* app, CPU& cpu, reg32 esp, A... a)
{
    cpu.esp = esp;
    try
    {
        const reg32 r = guest::direct<Address, Core>(app, cpu, a...);
        if (cpu.terminate)
            return std::nullopt;
        return r;
    }
    catch (const fe::Terminated&)
    {
        return std::nullopt;
    }
}

/* 0x4d0300 (the queue's next chunk), as the streams' task calls it: the queue
 * in eax and, in edx, the loop's test -- which the reader does not take. */
std::optional<reg32> nextQueueChunk(App* app, CPU& cpu, reg32 esp, reg32 queue, reg32 edx)
{
    cpu.esp = esp;
    try
    {
        static_cast<void>(edx);
        const reg32 r = guest::direct<kNextChunk, frontend::wina::streamNextChunk>(app, cpu, queue);
        if (cpu.terminate)
            return std::nullopt;
        return r;
    }
    catch (const fe::Terminated&)
    {
        return std::nullopt;
    }
}

/* A quotient the x86 does not raise for: as the arm64 does the code's `div` by 0. */
reg32 divided(reg32 n, reg32 by) { return by == 0 ? 0 : n / by; }

/* 0x503760's (__U8D): the 64 bits high:low over `by` as its shifts divide
 * them -- 32 bits of quotient, the high part's overflow lost. */
reg32 divide64(reg32 low, reg32 high, reg32 by)
{
    reg32 quotient = 0;
    for (int i = 0; i < 32; ++i)
    {
        high = high + high + (low >> 31);
        low += low;
        quotient += quotient;
        if (high >= by)
        {
            ++quotient;
            high -= by;
        }
    }
    return quotient;
}

/* The milliseconds of `frames` frames at `rate` a second, through the 64-bit
 * product and division the code calls. */
reg32 milliseconds(reg32 frames, reg32 rate)
{
    const uint64_t product = uint64_t(frames) * 1000;
    return divide64(reg32(product), reg32(product >> 32), rate);
}

/* 0x523d90's (iSNDplatformpacketsize): the port's platform keeps no memory
 * of its own for a packet player. */
int32_t platformPacketBytes() { return 0; }

/* 0x4f3aa4's: stream `index` (0 to 15), 0 for none. */
reg32 streamOf(App* app, int32_t index) { return index >= 0 && index < 0x10 ? kStreams(app, index) : 0; }

/* 0x4f3fd0's: the streams there are. */
int32_t streamsMade(App* app)
{
    int32_t n = 0;
    for (int32_t i = 0; i < 0x10; ++i)
        if (kStreams(app, i) != 0)
            ++n;
    return n;
}

/* 0x510a10's: the request of handle `handle`, 0 for none. */
reg32 requestOfHandle(App* app, reg32 handle)
{
    if (int32_t(handle) < 0)
        return 0;
    const reg32 s = streamOf(app, int32_t(handle & 0xff));
    if (s == 0)
        return 0;
    const Stream& st = stream(app, s);
    for (int32_t i = 0; i < st.holds; ++i)
    {
        const reg32 r = st.requests + reg32(i) * kRequestSize;
        if (request(app, r).handle == handle)
            return r;
    }
    return 0;
}

/* 0x511210's: the memory a packet player of `packets` packets takes. */
reg32 playerBytes(reg32 packets)
{
    return packets * sizeof(Packet) + kPlayerOverhead + reg32(platformPacketBytes());
}

/* 0x4f3a10's: the memory a stream of `requests` requests with a packet player
 * of `packets` packets takes. */
reg32 streamBytes(reg32 requests, reg32 packets)
{
    return playerBytes(packets) + requests * kRequestSize + sizeof(Stream);
}

/* 0x4f3ab8's: request `handle` taken out of its stream, the rest moved up
 * (the playing one with them).  The stream. */
reg32 takeOutRequest(App* app, reg32 handle)
{
    const reg32 s = kStreams(app, int32_t(handle & 0xff));
    Stream& st = stream(app, s);
    reg32 kept = 0, to = 0, from = 0;
    for (int32_t i = 0; i < st.holds; ++i, from += kRequestSize)
    {
        const reg32 r = st.requests + from;
        if (request(app, r).handle == handle)
            continue;
        if (st.playing == i)
            st.playing = int8_t(kept);
        const reg32 d = st.requests + to;
        for (reg32 k = 0; k < kRequestSize; k += 4)
            app->getMemory<reg32>(d + k) = app->getMemory<reg32>(r + k);
        ++kept;
        to += kRequestSize;
    }
    st.holds = int8_t(st.holds - 1);
    if (st.playing > st.holds)
        st.playing = int8_t(st.playing - 1);
    return s;
}

/* 0x5119b0's: a patch's settings as they are by default. */
void defaultSettings(PatchSettings& s)
{
    s.voiceMask = 0xffffffff;
    s.priority = 0;
    s.pitch = 0;
    s.level = 0x7f;
    s.pan = 0x40;
    s.send = 0;
    s.bendRange = 0;
    s.playPriority = 0;
}

/* 0x511bf0's (iSNDplatformcalcdatarate): the header's bytes a second, times
 * 256 -- its rate times channels times its representation's bytes a frame.
 * An unknown representation is an error, and the code goes on with the
 * bytes in edx, as the caller left it (`left`). */
Result dataRate(App* app, CPU& cpu, reg32 esp, const StreamHeader& h, reg32 left)
{
    const reg32 frames = reg32(h.rate) * (h.format & 3);
    const reg32 representation = h.format & 0xfffc;
    reg32 bytes = left;
    if (representation == 0xc)
        bytes = 0x88;
    else if (representation == 0x10)
        bytes = 0x33;
    else if (representation == 8)
        bytes = 0x100;
    else if (representation == 0)
        bytes = 0x200;
    else if (!fail(app, cpu, esp - 5 * 4, kRateError, 0x1c, {0x550098, reg32(h.format >> 2)}))
        return {};
    return imul(int32_t(frames), int32_t(bytes)) >> 8;
}

/* 0x5119d8's (iSNDpatchtohdr): the header and settings of patch `patch`, as
 * its tags give them -- the rate (0x5622 unless said), the channels, the
 * representation (from the bits a sample, or the compression), the frames
 * into `framesAt`.  0. */
Result patchToHeader(App* app, CPU& cpu, reg32 esp, reg32 patch, StreamHeader& header, PatchSettings& settings,
                     reg32 framesAt)
{
    const reg32 f = esp - 3 * 4 - 0x1c;
    defaultSettings(settings);
    reg32 bits = 0x10, compression = 0;
    Tag tag{patch + ((app->getMemory<uint8_t>(patch + 3) & 2) ? 8 : 4), 0, 0, 0};
    // The rate, and one channel.
    header.rate = 0x5622;
    header.format = uint16_t((header.format & 0xfffc) | 1);
    store<reg32>(app, framesAt, 0);
    while (readTag(app, tag))
    {
        const reg32 number = tag.number;
        switch (tag.tag)
        {
        case 0x81:
            bits = number;
            break;
        case 0x83:
            compression = number;
            break;
        case 0x91:
            settings.playPriority = int8_t(number);
            break;
        case 0x82:
            header.format = uint16_t((header.format & 0xfffc) | (number & 3));
            break;
        case 0x84:
            header.rate = uint16_t(number);
            break;
        case 0x85:
            store<reg32>(app, framesAt, number);
            break;
        case 0x13:
            settings.send = int8_t(number);
            break;
        case 0xa:
            settings.bendRange = int8_t(number);
            break;
        case 5:
            settings.voiceMask = number;
            break;
        case 6:
            settings.priority = int8_t(number);
            break;
        default:
            break;
        }
    }
    // The representation: its bits on top of the channels' 2.
    const auto represent = [&](uint8_t representation) {
        header.format = uint16_t((header.format & 3) | representation);
    };
    if (compression != 0)
    {
        if (compression == 5)
            represent(0x18);
        else if (compression == 7)
            represent(0xc);
        else if (compression == 9)
            represent(0x10);
        else if (!fail(app, cpu, f, kHeaderError, 0x64, {0x550014}, 0x54fff4))
            return {};
    }
    else if (bits == 0x10)
        represent(0);
    else if (bits == 8)
        represent(8);
    else if (!fail(app, cpu, f, kHeaderError, 0x77, {0x550040}))
        return {};
    return 0;
}

/* 0x511228's (SNDPKTPLAY_create): a packet player in the `size` bytes at
 * `memory`, its packets' free callback and its frames callback.  Its index;
 * -10 before SNDinit, -9 with 16, -6 for the platform's memory. */
int32_t createPlayer(App* app, reg32 memory, reg32 size, reg32 freed, reg32 frames)
{
    if (kInited(app) == 0)
        return kNotStarted;
    int32_t index = 0;
    if (playerOf(app, 0) != 0)
        for (;;)
        {
            ++index;
            if (index >= 0x10)
                return kNoVoices;
            if (playerOf(app, index) == 0)
                break;
        }
    if (platformPacketBytes() < 0)
        return -6;
    const reg32 p = memory + reg32(platformPacketBytes());
    const reg32 left = size - reg32(platformPacketBytes());
    kPlayers.set(app, index, p);
    PacketPlayer& pp = packetPlayer(app, p);
    pp.size = int16_t((left - kPlayerOverhead) / sizeof(Packet));
    pp.framesCallback = frames;
    pp.memory = memory;
    pp.freed = freed;
    pp.voice = -1;
    return index;
}

/* 0x5112d0's (SNDPKTPLAY_start): packet player `index` started on a voice,
 * the voice set up as the patch's settings (at `settingsAt`) and the play's
 * options (`optionsAt`) have it, and the platform's packet play started for
 * it, its sound the header at `headerAt`.  The voice's handle; -10 before
 * SNDinit, -9 for no voice, the platform's error. */
Result startPlayer(App* app, CPU& cpu, reg32 esp, int32_t index, reg32 headerAt, reg32 settingsAt, reg32 optionsAt)
{
    const reg32 f = esp - 3 * 4 - 0x10;
    if (kInited(app) == 0)
        return kNotStarted;
    if (!checkPlayOptions(app, cpu, f - 4, optionsAt))
        return {};
    const reg32 p = playerOf(app, index);
    if (!enterAudio(app, cpu, f - 4))
        return {};
    const PatchSettings& settings = record<PatchSettings>(app, settingsAt);
    const PlayOptions& options = record<PlayOptions>(app, optionsAt);
    reg32 handle = load<reg32>(app, f);
    const Result allocated =
        allocVoices(app, cpu, f - 4, settings.voiceMask, 1, uint8_t(settings.priority), handle);
    store(app, f, handle);
    if (!allocated)
        return {};
    const int32_t v = *allocated;
    if (v < 0)
    {
        if (!leaveAudio(app, cpu, f - 4))
            return {};
        return kNoVoices;
    }
    PacketPlayer& pp = packetPlayer(app, p);
    pp.voice = int32_t(handle);
    pp.head = 0;
    pp.count = 0;
    pp.submitted = 0;
    pp.serial = 0;
    pp.inHardware = 0;
    pp.playing = -1;
    pp.header = record<StreamHeader>(app, headerAt);
    Voice& w = kVoices(app, v);
    w.bank = 0xff;
    w.pan = uint8_t(options.pan);
    w.pitch = settings.pitch;
    w.volumeStep = 0;
    w.envelopeStep = 0;
    w.envelopeTicks = 0x7fffffff;
    w.envelope = 0x7f0000;
    w.volume = int32_t(reg32(int32_t(options.volume)) << 16);
    w.level = settings.level;
    w.patchPan = settings.pan;
    w.segments = 1;
    w.segment = 0;
    w.timbre20 = 0;
    w.spread = 1;
    w.bend = uint8_t(options.bend);
    w.timbreSend = settings.send;
    w.send = options.effectLevel;
    w.bendRange = int16_t(int32_t(settings.bendRange) * 100);
    w.group = 0;
    w.endsGroup = 0;
    w.envelopeSegments = w.volumeCurve = w.panCurve = w.bendCurve = w.volumeLfo = w.pitchLfo = 0;
    w.panAsked = options.pan;
    refreshPitch(app, v);
    refreshVolume(app, v);
    const int32_t send = idiv(imul(w.send, imul(w.timbreSend, kBusLevel(app))), 0x3f01);
    // The voice's pan as a signed byte, as the code takes it (sar 24).
    const Result played = playPackets(app, cpu, f - 4 - 4 * 4, v, int8_t(w.pan), w.mixLevel, w.playedPitch, send,
                                      settings.playPriority, headerAt);
    if (!played)
        return {};
    if (*played < 0)
    {
        if (!freeVoice(app, cpu, f - 4, v) || !leaveAudio(app, cpu, f - 4))
            return {};
        return *played;
    }
    if (!leaveAudio(app, cpu, f - 4))
        return {};
    return pp.voice;
}

/* 0x5114d4's (SNDPKTPLAY_submit): the packet at `offeredAt` (its frames and
 * its data) into packet player `index`'s ring.  Its serial; -10 before
 * SNDinit, -13 when the ring is full. */
Result submitPacket(App* app, CPU& cpu, reg32 esp, int32_t index, reg32 offeredAt)
{
    const reg32 f = esp - 3 * 4;
    if (kInited(app) == 0)
        return kNotStarted;
    const reg32 p = playerOf(app, index);
    PacketPlayer& pp = packetPlayer(app, p);
    if (pp.voice < 0 && !fail(app, cpu, esp - 5 * 4, kSubmitError, 0xc1, {0x54fde4}))
        return {};
    if (!enterAudio(app, cpu, f - 4))
        return {};
    if (int32_t(int16_t(pp.count)) >= int32_t(pp.size) - 1)
    {
        if (!leaveAudio(app, cpu, f - 4))
            return {};
        return kFull;
    }
    int32_t slot = int32_t(int16_t(pp.head)) + int32_t(int16_t(pp.count));
    if (slot >= pp.size)
        slot -= pp.size;
    const Offered& offered = record<Offered>(app, offeredAt);
    Packet& packet = ringPacket(app, p, slot);
    packet.frames = offered.frames;
    packet.data = offered.data;
    packet.serial = pp.serial;
    pp.count = uint16_t(pp.count + 1);
    pp.submitted += offered.frames;
    const int32_t serial = pp.serial;
    pp.serial = serial + 1;
    if (!leaveAudio(app, cpu, f - 4))
        return {};
    return serial;
}

/* 0x5115b8's (SNDPKTPLAY_submitspace): the packets packet player `index` has
 * room for; -10 before SNDinit. */
Result submitSpace(App* app, CPU& cpu, reg32 esp, int32_t index)
{
    if (kInited(app) == 0)
        return kNotStarted;
    const PacketPlayer& pp = packetPlayer(app, playerOf(app, index));
    if (pp.voice < 0 && !fail(app, cpu, esp - 5 * 4, kSpaceError, 0xf3, {0x54fe38}))
        return {};
    return int32_t(pp.size) - int32_t(int16_t(pp.count)) - 1;
}

/* 0x51162c's: the frames packet player `index` has outstanding, submitted
 * and in the hardware. */
Result framesOutstandingOf(App* app, CPU& cpu, reg32 esp, int32_t index)
{
    const PacketPlayer& pp = packetPlayer(app, playerOf(app, index));
    if (pp.voice < 0 && !fail(app, cpu, esp - 4 * 4, kOutstandingError, 0x104, {0x54fe9c}))
        return {};
    return pp.submitted + pp.inHardware;
}

/* 0x511680's (SNDPKTPLAY_framesoutstanding): the same, in the library's lock;
 * -10 before SNDinit. */
Result framesOutstandingLocked(App* app, CPU& cpu, reg32 esp, int32_t index)
{
    const reg32 f = esp - 4;
    if (kInited(app) == 0)
        return kNotStarted;
    if (!enterAudio(app, cpu, f - 4))
        return {};
    const Result frames = framesOutstandingOf(app, cpu, f - 4, index);
    if (!frames)
        return {};
    if (!leaveAudio(app, cpu, f - 4, reg32(*frames)))
        return {};
    return *frames;
}

/* 0x511718's (SNDPKTPLAY_purge): the packets of packet player `index` with
 * serials `first` to `last` taken out of its ring (their free callback
 * called), the rest kept in order.  0; -10 before SNDinit. */
Result purgePackets(App* app, CPU& cpu, reg32 esp, int32_t index, int32_t first, int32_t last)
{
    const reg32 f = esp - 2 * 4 - 0x10 - 8;
    if (kInited(app) == 0)
        return kNotStarted;
    const reg32 p = playerOf(app, index);
    PacketPlayer& pp = packetPlayer(app, p);
    if (pp.voice < 0 && !fail(app, cpu, f, kPurgeError, 0x13d, {0x54ff70}))
        return {};
    if (!enterAudio(app, cpu, f - 4))
        return {};
    int32_t to = int16_t(pp.head), from = int16_t(pp.head);
    const int32_t count = int16_t(pp.count);
    for (int32_t n = 0; n < count; ++n)
    {
        Packet& packet = ringPacket(app, p, from);
        if (packet.serial < first || packet.serial > last)
        {
            ringPacket(app, p, to) = packet;
            ++to;
            if (to >= pp.size)
                to = 0;
        }
        else
        {
            pp.count = uint16_t(pp.count - 1);
            pp.submitted -= packet.frames;
            if (pp.freed != 0 && !pointerGame(app, cpu, f, pp.freed, {packet.data, p, reg32(to), reg32(from)}))
                return {};
        }
        if (++from >= pp.size)
            from = 0;
    }
    if (!leaveAudio(app, cpu, f - 4))
        return {};
    return 0;
}

/* 0x51181c's (SNDPKTPLAY_stop): packet player `index`'s voice stopped, its
 * packets purged, the one the mixer has given back too.  0; -10 before
 * SNDinit. */
Result stopPlayer(App* app, CPU& cpu, reg32 esp, int32_t index)
{
    if (kInited(app) == 0)
        return kNotStarted;
    const reg32 f = esp - 5 * 4;
    const reg32 p = playerOf(app, index);
    PacketPlayer& pp = packetPlayer(app, p);
    if (pp.voice < 0 && !fail(app, cpu, f - 4, kStopError, 0x178, {0x54ffbc}))
        return {};
    if (!enterAudio(app, cpu, f - 4) || !stopSound(app, cpu, f - 4, reg32(pp.voice)))
        return {};
    if (!purgePackets(app, cpu, f - 4, index, 0, 0x7fffffff))
        return {};
    if (pp.freed != 0 && pp.playing >= 0)
    {
        const reg32 data = ringPacket(app, p, pp.playing).data;
        if (!callBack(app, cpu, f, pp.freed, data))
            return {};
    }
    pp.voice = -1;
    if (!leaveAudio(app, cpu, f - 4))
        return {};
    return 0;
}

/* 0x5118d0's (SNDPKTPLAY_destroy): packet player `index` no more.  0; -10
 * before SNDinit. */
int32_t destroyPlayer(App* app, int32_t index)
{
    if (kInited(app) == 0)
        return kNotStarted;
    kPlayers.set(app, index, 0);
    return 0;
}

/* 0x5118fc's: the mixer's next packet of packet player `index`: the one it
 * had given back, the next taken -- its frames into `framesAt`.  Its data, 0
 * for none.  `p`, the player, is what the code leaves in edx. */
Result takeNextPacket(App* app, CPU& cpu, reg32 esp, int32_t index, reg32 framesAt, reg32& p)
{
    const reg32 f = esp - 2 * 4;
    p = playerOf(app, index);
    PacketPlayer& pp = packetPlayer(app, p);
    if (pp.playing >= 0)
    {
        const reg32 data = ringPacket(app, p, pp.playing).data;
        pp.playing = -1;
        if (pp.freed != 0 && !callBack(app, cpu, f, pp.freed, data))
            return {};
    }
    if (pp.count == 0)
        return 0;
    pp.playing = int16_t(pp.head);
    const Packet& packet = ringPacket(app, p, int16_t(pp.head));
    store(app, framesAt, packet.frames);
    pp.count = uint16_t(pp.count - 1);
    pp.head = uint16_t(pp.head + 1);
    pp.inHardware += packet.frames;
    pp.submitted -= packet.frames;
    if (int16_t(pp.head) >= pp.size)
        pp.head = 0;
    return int32_t(packet.data);
}

/* 0x511998's: `frames` frames of packet player `index` played by the
 * hardware, its frames callback told (the player's index in eax, the frames
 * in edx).  eax as the callback leaves it, the caller's own (`eax`) without
 * one. */
Result packetFramesHeard(App* app, CPU& cpu, reg32 esp, int32_t index, reg32 frames, reg32 eax)
{
    const reg32 c = playerOf(app, index);
    PacketPlayer& pp = packetPlayer(app, c);
    pp.inHardware -= int32_t(frames);
    if (pp.framesCallback != 0)
    {
        cpu.esp = esp - 4;
        const std::optional<reg32> r = guest::pointerCall(app, cpu, pp.framesCallback, {reg32(index), frames});
        if (!r)
            return {};
        eax = *r;
    }
    return int32_t(eax);
}

/* 0x4f3b44's: the packet player's free callback: the chunk of the packet's
 * data `data` given back to its stream's queue.  The queue's answer. */
Result giveBackPacket(App* app, CPU& cpu, reg32 esp, reg32 data)
{
    const reg32 chunk = data - 0xc;
    const reg32 s = kStreams(app, int32_t(app->getMemory<reg32>(chunk) & 0xff));
    const std::optional<reg32> result =
        wina<kFreeChunk, frontend::wina::streamFreeChunk>(app, cpu, esp - 4, stream(app, s).queue, chunk);
    if (!result)
        return {};
    return int32_t(*result);
}

/* 0x4f3b60's: the packet player's frames callback: `frames` frames of packet
 * player `index` played, counted off its stream's requests -- each played
 * through taken out.  eax as the code leaves it (the request, or its stream
 * once one was taken out); `lastPlayed` is edx, the frames the last request
 * has played. */
reg32 countPlayedFrames(App* app, int32_t index, reg32 frames, reg32& lastPlayed)
{
    const reg32 s = kStreams(app, int32_t(kPlayerStream(app, index)));
    reg32 over = 0, eax;
    for (;;)
    {
        const reg32 r = stream(app, s).requests;
        Request& q = request(app, r);
        eax = r;
        if (frames > q.inPlayer)
        {
            over = frames - q.inPlayer;
            frames -= over;
        }
        q.played += frames;
        q.inPlayer -= frames;
        lastPlayed = q.played;
        if (q.played >= q.frames)
            eax = takeOutRequest(app, q.handle);
        if (over == 0)
            break;
        frames = over;
        over = 0;
    }
    return eax;
}

/* 0x511ca0's: stream `index`'s queue's chunks to ask for at once: `count`.
 * 0; -10 before SNDinit, -8 for no stream. */
Result chunksAtOnce(App* app, CPU& cpu, reg32 esp, reg32 index, reg32 count)
{
    if (kInited(app) == 0)
        return kNotStarted;
    const reg32 s = streamOf(app, int32_t(index));
    if (s == 0)
        return kNoSound;
    if (!wina<kQueueChunksAtOnce, frontend::wina::streamSetLowWater>(app, cpu, esp, stream(app, s).queue, count))
        return {};
    return 0;
}

/* 0x4f3ba4's (iSNDstreamparseheader): stream `s`'s header chunk `chunk`: its
 * patch made the request's header and data rate; a first header starts the
 * packet player, another of a different format waits for it to stop (2).
 * 0. */
Result parseHeader(App* app, CPU& cpu, reg32 esp, reg32 s, reg32 chunk)
{
    const reg32 f = esp - 5 * 4 - 0xc;
    Stream& st = stream(app, s);
    const reg32 requestAt = playingRequest(st);
    Request& r = request(app, requestAt);
    if (!patchToHeader(app, cpu, f - 4, chunk + 8, st.parsedHeader, st.parsedSettings, f + 4))
        return {};
    r.frames = load<reg32>(app, f + 4);
    if (!wina<kFreeChunk, frontend::wina::streamFreeChunk>(app, cpu, f, st.queue, chunk))
        return {};
    const Result rate = dataRate(app, cpu, f - 4, st.parsedHeader, requestAt);
    if (!rate)
        return {};
    r.dataRate = reg32(*rate);
    if (std::memcmp(&st.playingHeader, &st.parsedHeader, sizeof(StreamHeader)) != 0
        || std::memcmp(&st.playingSettings, &st.parsedSettings, sizeof(PatchSettings)) != 0)
    {
        if (st.playingHeader.rate != 0)
        {
            st.state = 2;
            return 0;
        }
        st.playingHeader = st.parsedHeader;
        st.playingSettings = st.parsedSettings;
    }
    if (st.state != 1)
    {
        const Result v = startPlayer(app, cpu, f - 4, st.player, s + 0x1c, s + 0x24, s + 0x3c);
        if (!v)
            return {};
        st.voice = *v;
        if (st.voice < 0 && !fail(app, cpu, f, kParseHeaderError, 0x115, {0x54ccd8}))
            return {};
        st.state = 1;
    }
    return 0;
}

/* 0x4f3cc8's (iSNDstreamparseloop): stream `s`'s loop chunk `chunk`: its
 * count the request's loop.  1. */
Result parseLoop(App* app, CPU& cpu, reg32 esp, reg32 s, reg32 chunk)
{
    const Stream& st = stream(app, s);
    if (!wina<kFreeChunk, frontend::wina::streamFreeChunk>(app, cpu, esp - 2 * 4, st.queue, chunk))
        return {};
    // The chunk is read after it was given back, as the code does.
    request(app, playingRequest(st)).loop = app->getMemory<reg32>(chunk + 8);
    return 1;
}

/* 0x4f3d00's (iSNDstreamparsedata): stream `s`'s data chunk `chunk`
 * submitted to its packet player as a packet -- the chunk marked with the
 * request's handle, its frames counted.  1. */
Result parseData(App* app, CPU& cpu, reg32 esp, reg32 s, reg32 chunk)
{
    const reg32 f = esp - 3 * 4 - 0x10;
    const Stream& st = stream(app, s);
    Request& r = request(app, playingRequest(st));
    Offered& offered = record<Offered>(app, f);
    offered.frames = int32_t(app->getMemory<reg32>(chunk + 8));
    offered.data = chunk + 0xc;
    app->getMemory<reg32>(chunk) = r.handle;
    r.inPlayer += reg32(offered.frames);
    r.packets += 1;
    const Result submitted = submitPacket(app, cpu, f - 4, st.player, f);
    if (!submitted)
        return {};
    r.lastSubmitted = *submitted;
    if (r.lastSubmitted < 0
        && !fail(app, cpu, f, kParseDataError, 0x158, {0x54cd28, reg32(r.lastSubmitted)}, 0x54cd14))
        return {};
    if (r.submitted < 0)
        r.submitted = r.lastSubmitted;
    return 1;
}

/* 0x4f3dac's (iSNDstreamparseend): stream `s`'s end chunk `chunk`: the next
 * request plays.  1. */
Result parseEnd(App* app, CPU& cpu, reg32 esp, reg32 s, reg32 chunk)
{
    Stream& st = stream(app, s);
    if (!wina<kFreeChunk, frontend::wina::streamFreeChunk>(app, cpu, esp - 4, st.queue, chunk))
        return {};
    st.playing = int8_t(st.playing + 1);
    return 1;
}

/* 0x4f3dc4's: stream `s`'s chunk handed on by its id; another given back.
 * What the handler gave (1 for another). */
Result parseChunk(App* app, CPU& cpu, reg32 esp, reg32 s, reg32 chunk)
{
    const reg32 f = esp - 2 * 4;
    switch (app->getMemory<reg32>(chunk))
    {
    case kIdData:
        return parseData(app, cpu, f - 4, s, chunk);
    case kIdHeader:
        return parseHeader(app, cpu, f - 4, s, chunk);
    case kIdEnd:
        return parseEnd(app, cpu, f - 4, s, chunk);
    case kIdLoop:
        return parseLoop(app, cpu, f - 4, s, chunk);
    default:
        if (!wina<kFreeChunk, frontend::wina::streamFreeChunk>(app, cpu, f, stream(app, s).queue, chunk))
            return {};
        return 1;
    }
}

/* 0x4f3e1c's: whether stream `s` waits before it parses on: for the time its
 * request waits for, while the queue holds less -- unless the queue is done
 * (2).  1 to wait. */
Result waitsForQueue(App* app, CPU& cpu, reg32 esp, reg32 s)
{
    const reg32 f = esp - 3 * 4;
    const Stream& st = stream(app, s);
    Request& r = request(app, playingRequest(st));
    if (r.dataRate == 0 || r.waits == 0)
        return 0;
    if (r.waits < 0)
        return 1;
    const std::optional<reg32> queued = wina<kQueueBytes, frontend::wina::channelBytes>(app, cpu, f, st.queue);
    if (!queued)
        return {};
    reg32 bytes = *queued;
    if (bytes > kMaxQueuedBytes)
        bytes = kMaxQueuedBytes;
    if (divided(bytes * 1000, r.dataRate) >= reg32(int32_t(r.waits)))
    {
        r.waits = 0;
        return 0;
    }
    const std::optional<reg32> state = wina<kQueueState, frontend::wina::channelState>(app, cpu, f, st.queue);
    if (!state)
        return {};
    if ((*state & 0xff) != 2)
        return 1;
    r.waits = 0;
    return 0;
}

/* 0x4f3ebc's (iSNDstreamsystemtask): each stream with requests served: a new
 * header's player started once the old one has played out, then (unless it
 * waits) its queue's chunks parsed -- 10 at most, or as many as the player
 * has room for.  0; nothing while a surface is locked. */
Result streamSystemTask(App* app, CPU& cpu, reg32 esp)
{
    const bool locked = stream::directFrom<kSurfaceLocked, ea::surfaceIsLocked>(app, cpu, esp);
    if (cpu.terminate)
        return {};
    if (locked)
        return 0;
    const reg32 f = esp - 4 * 4;
    for (int32_t i = 0; i < 0x10; ++i)
    {
        safepoint(app, cpu);
        const reg32 s = kStreams(app, i);
        if (s == 0 || stream(app, s).holds == 0)
            continue;
        Stream& st = stream(app, s);
        if (st.state == 2)
        {
            const Result left = framesOutstandingLocked(app, cpu, f - 4, st.player);
            if (!left)
                return {};
            if (*left > 0)
                continue;
            st.playingHeader = st.parsedHeader;
            if (!stopPlayer(app, cpu, f - 4, st.player))
                return {};
            const Result v = startPlayer(app, cpu, f - 4, st.player, s + 0x1c, s + 0x24, s + 0x3c);
            if (!v)
                return {};
            st.voice = *v;
            if (st.voice < 0 && !fail(app, cpu, f, kTaskError, 0x235, {0x54cd74}))
                return {};
            st.state = 1;
        }
        const Result waits = waitsForQueue(app, cpu, f - 4, s);
        if (!waits)
            return {};
        if (*waits != 0)
            continue;
        int32_t chunks = 10;
        if (st.state == 1)
        {
            const Result room = submitSpace(app, cpu, f - 4, st.player);
            if (!room)
                return {};
            chunks = *room;
            if (chunks == 0)
                continue;
        }
        if (!enterAudio(app, cpu, f - 4))
            return {};
        // The code's loop keeps its test in edx, which the queue is handed as
        // well: the last chunk's answer (0 the first time round, and while
        // the queue gives none).
        reg32 handled = 0;
        for (;;)
        {
            safepoint(app, cpu);
            --chunks;
            const std::optional<reg32> next = nextQueueChunk(app, cpu, f, st.queue, handled);
            if (!next)
                return {};
            const reg32 chunk = *next;
            if (chunk != 0)
            {
                const Result r = parseChunk(app, cpu, f - 4, s, chunk);
                if (!r)
                    return {};
                handled = reg32(*r);
            }
            if (handled == 0 || chunks <= 0)
                break;
        }
        if (!leaveAudio(app, cpu, f - 4))
            return {};
    }
    return 0;
}

/* 0x4f4440's (SNDSTRM_purge): stream `index`'s player stopped, its queue
 * emptied when made here, its requests and headers cleared.  0; -10 before
 * SNDinit, -8 for no stream. */
Result purgeStream(App* app, CPU& cpu, reg32 esp, int32_t index)
{
    if (kInited(app) == 0)
        return kNotStarted;
    const reg32 s = streamOf(app, index);
    if (s == 0)
        return kNoSound;
    Stream& st = stream(app, s);
    if (st.voice >= 0 && !stopPlayer(app, cpu, esp - 2 * 4, st.player))
        return {};
    st.voice = -1;
    if (st.queueGiven == 0 && !wina<kQueueEmptied, frontend::wina::streamFlush>(app, cpu, esp - 4, st.queue))
        return {};
    st.holds = 0;
    st.playing = 0;
    st.state = 0;
    std::memset(&st.playingHeader, 0, sizeof(StreamHeader));
    std::memset(&st.parsedHeader, 0, sizeof(StreamHeader));
    std::memset(&st.playingSettings, 0, sizeof(PatchSettings));
    std::memset(&st.parsedSettings, 0, sizeof(PatchSettings));
    return 0;
}

/* 0x4f43c0's (SNDSTRM_destroy): stream `index` purged and gone -- its packet
 * player, its queue when made here; the last takes the streams' task.  0;
 * -10 before SNDinit, -8 for no stream. */
Result destroyStream(App* app, CPU& cpu, reg32 esp, int32_t index)
{
    if (kInited(app) == 0)
        return kNotStarted;
    const reg32 s = streamOf(app, index);
    if (s == 0)
        return kNoSound;
    const reg32 f = esp - 3 * 4;
    if (!purgeStream(app, cpu, f - 4, index))
        return {};
    if (streamsMade(app) == 1)
    {
        stream::unscheduleTask(app, kStreamTask);
        kStreamerRestore.set(app, 0);
    }
    const Stream& st = stream(app, s);
    destroyPlayer(app, st.player);
    if (st.queueGiven == 0 && !wina<kQueueGone, frontend::wina::streamClose>(app, cpu, f, st.queue))
        return {};
    kStreams.set(app, index, 0);
    return 0;
}

/* 0x4f3a90's: every stream destroyed (the streams' restore).  0. */
Result destroyEveryStream(App* app, CPU& cpu, reg32 esp)
{
    for (int32_t i = 0; i < 0x10; ++i)
        if (!destroyStream(app, cpu, esp - 2 * 4, i))
            return {};
    return 0;
}

/* 0x4f3fec's (iSNDstreamcreate): a stream of `requests` requests (1 to 127)
 * in the `size` bytes at `memory`, its packet player of `packets` packets,
 * the play's `options`; its queue `queue` when `given`, else one made in the
 * memory left.  The first stream adds the streams' task.  Its index; -10
 * before SNDinit, -9 with 16 streams. */
Result createStream(App* app, CPU& cpu, reg32 esp, reg32 options, reg32 requests, reg32 packets, reg32 memory,
                    reg32 size, reg32 queue, reg32 given)
{
    const reg32 f = esp - 3 * 4 - 0x10;
    if (kInited(app) == 0)
        return kNotStarted;
    if ((int32_t(requests) <= 0 || int32_t(requests) >= 0x80)
        && !fail(app, cpu, f, kCreateError, 0x280, {0x54cdc0, requests}, 0x54cdac))
        return {};
    if (memory == 0 && !fail(app, cpu, f, kCreateError, 0x285, {0x54ce10}, 0x54cdac))
        return {};
    int32_t slot = 0;
    if (kStreams(app, 0) != 0)
        for (;;)
        {
            ++slot;
            if (slot >= 0x10)
                return kNoVoices;
            if (kStreams(app, slot) == 0)
                break;
        }
    const reg32 s = memory;
    Stream& st = stream(app, s);
    reg32 free = memory + sizeof(Stream);
    size -= sizeof(Stream);
    st.requests = free;
    free += requests * kRequestSize;
    size -= requests * kRequestSize;
    const reg32 playerAt = free;
    free += playerBytes(packets);
    size -= playerBytes(packets);
    st.player = createPlayer(app, playerAt, playerBytes(packets), kFreedCallback, kFramesCallback);
    if (st.player < 0)
        return st.player;
    kPlayerStream.set(app, st.player, int8_t(slot));
    if (given != 0)
    {
        if (int32_t(size) < 0 && !fail(app, cpu, f, kCreateError, 0x2b5, {0x54ce34}, 0x54ccb4))
            return {};
        st.queue = queue;
        st.queueGiven = 1;
    }
    else
    {
        if (int32_t(size) < 0x6000 && !fail(app, cpu, f, kCreateError, 0x2c2, {0x54ce34}, 0x2c2))
            return {};
        // stdcall: the memory's size pushed.
        const std::optional<reg32> queueMade =
            wina<kQueueMake, frontend::wina::streamCreate>(app, cpu, f - 4, requests + 1, 1u, 1u, free, size);
        if (!queueMade)
            return {};
        const reg32 made = *queueMade;
        st.queue = made;
        st.queueGiven = 0;
        // The block's size: the call has nothing in eax but the queue's, which a
        // reader that has no size to give leaves there.
        const std::optional<reg32> ring = wina<kQueueBlock, frontend::wina::channelRingSize>(app, cpu, f, made);
        if (!ring)
            return {};
        const reg32 block = *ring;
        // The queue is to ask for a third of a block's chunks at once -- of the
        // stream the code names by the queue's address, which is none.
        if (!chunksAtOnce(app, cpu, f - 4, st.queue, reg32(idiv(int32_t(block), 3))))
            return {};
    }
    st.serial = 0;
    st.voice = -1;
    st.takes = int8_t(requests);
    st.options = record<PlayOptions>(app, options);
    if (streamsMade(app) == 0)
    {
        if (!stream::scheduleTask(app, cpu, f - 4, kStreamTask, 0, 1))
            return {};
        kStreamerRestore.set(app, kStreamRestore);
    }
    kStreams.set(app, slot, s);
    if (!purgeStream(app, cpu, f - 4, slot))
        return {};
    return slot;
}

/* 0x4f4258's (iSNDstreamrequest): a request on stream `index` for time
 * `time`: of `bytes` bytes at `data` -- `how` 0 read from the queue, 1 read
 * from it whole, 2 the chunks given (`data`).  Its handle; -10 before SNDinit,
 * -8 for no stream, -13 when it is full, -1 for no chunks. */
Result requestStream(App* app, CPU& cpu, reg32 esp, reg32 index, reg32 time, reg32 bytes, reg32 data, reg32 how)
{
    const reg32 f = esp - 3 * 4 - 8;
    if (kInited(app) == 0)
        return kNotStarted;
    const reg32 s = streamOf(app, int32_t(index));
    if (s == 0)
        return kNoSound;
    if (!enterAudio(app, cpu, f - 4))
        return {};
    Stream& st = stream(app, s);
    if (st.holds >= st.takes)
    {
        if (!leaveAudio(app, cpu, f - 4))
            return {};
        return kFull;
    }
    Request& r = request(app, st.requests + reg32(int32_t(st.holds)) * kRequestSize);
    if (how == 0 || how == 1)
    {
        const std::optional<reg32> chunks =
            how == 0 ? wina<kQueueRequest, frontend::wina::streamOpenFile>(app, cpu, f, st.queue, bytes, data, kIdEnd)
                     : wina<kQueueRequestWhole, frontend::wina::streamOpenMemory>(app, cpu, f, st.queue, bytes, 0u,
                                                                                  kIdEnd);
        if (!chunks)
            return {};
        r.chunks = *chunks;
    }
    else
        r.chunks = data;
    if (r.chunks == 0)
    {
        if (!leaveAudio(app, cpu, f - 4))
            return {};
        return -1;
    }
    st.serial = int32_t(reg32(st.serial) + 0x100);
    if (st.serial < 0)
        st.serial = 0;
    r.handle = reg32(st.serial) | index;
    r.waits = int16_t(time);
    st.holds = int8_t(st.holds + 1);
    r.dataRate = 0;
    r.submitted = -1;
    r.lastSubmitted = -1;
    r.played = 0;
    r.inPlayer = 0;
    r.packets = 0;
    r.loop = 0;
    // Its frames (and what the header gives) are the header's to set.
    if (!leaveAudio(app, cpu, f - 4))
        return {};
    return int32_t(r.handle);
}

/* 0x4f2f50's (SNDSTRM_requeststatus): request `handle`'s state into the
 * four words at `out` -- 3 gone, 2 playing, 1 waiting, 0 not started -- with
 * the ms it has played and has left and those buffered.  0; -10 before
 * SNDinit, -8 for no stream. */
Result requestStatus(App* app, CPU& cpu, reg32 esp, reg32 handle, reg32 out)
{
    const reg32 f = esp - 3 * 4 - 0x10;
    RequestStatus& status = record<RequestStatus>(app, out);
    status = RequestStatus{};
    if (kInited(app) == 0)
        return kNotStarted;
    if (int32_t(handle) < 0)
        return kNoSound;
    const reg32 s = streamOf(app, int32_t(handle & 0xff));
    if (s == 0)
        return kNoSound;
    if (!enterAudio(app, cpu, f - 8))
        return {};
    const auto done = [&]() -> Result {
        if (!leaveAudio(app, cpu, f - 8))
            return {};
        return 0;
    };
    const reg32 r = requestOfHandle(app, handle);
    if (r == 0)
    {
        status.state = 3;
        return done();
    }
    const Request& q = request(app, r);
    if (q.submitted < 0)
    {
        status.state = 0;
        return done();
    }
    const Stream& st = stream(app, s);
    reg32 rate;
    if (r == st.requests)
    {
        status.state = 2;
        rate = st.playingHeader.rate;
    }
    else
    {
        status.state = 1;
        rate = st.parsedHeader.rate;
    }
    status.played = milliseconds(q.played, rate);
    status.left = milliseconds(q.frames - q.played, rate);
    if (q.inPlayer > kMaxQueuedBytes && !fail(app, cpu, f - 8, kStatusError, 0x9b, {0x54cc78}))
        return {};
    status.buffered = divided(q.inPlayer * 1000, rate);
    return done();
}

}  // namespace

namespace stream
{

/* 0x5118fc and 0x511998 as the mixer calls them (port_snd_stream_calls.h). */
Result packetTaken(App* app, CPU& cpu, reg32 esp, int32_t index, reg32 framesAt)
{
    reg32 player;
    return takeNextPacket(app, cpu, esp, index, framesAt, player);
}

Result packetFramesPlayedOf(App* app, CPU& cpu, reg32 esp, int32_t index, reg32 frames)
{
    return packetFramesHeard(app, cpu, esp, index, frames, reg32(index));
}

/* 0x4f3a10, 0x4f3a60, 0x4f3a80, 0x4f2f50 and 0x4f43c0 as the movie player
 * calls them (port_snd_stream_calls.h): their thunks' cores. */
reg32 streamMemoryOf(App* app, reg32 requests, reg32 packets)
{
    static_cast<void>(app);
    return streamBytes(requests, packets);
}

Result createQueued(App* app, CPU& cpu, reg32 esp, reg32 queue, reg32 options, reg32 requests, reg32 packets,
                    reg32 memory, reg32 size)
{
    return createStream(app, cpu, esp - 4 - 3 * 4 - 4, options, requests, packets, memory, size, queue, 1);
}

Result requestData(App* app, CPU& cpu, reg32 esp, reg32 index, reg32 time, reg32 chunks)
{
    return requestStream(app, cpu, esp - 4 - 4 - 4, index, time, 0, chunks, 2);
}

Result requestStatusOf(App* app, CPU& cpu, reg32 esp, reg32 handle, reg32 out)
{
    return requestStatus(app, cpu, esp, handle, out);
}

Result destroyStreamOf(App* app, CPU& cpu, reg32 esp, int32_t index)
{
    return destroyStream(app, cpu, esp, index);
}

}  // namespace stream

/* The game's entries: each its thunk -- the game's registers and stack in,
 * the result in eax out -- around what it does. */

/* 0x503670 (__U8M): eax times edx, unsigned, into the 8 bytes at esi; eax the
 * place.  esi is left at the code's copy on its frame, as its movsd leaves
 * it. */
x86::reg32 mul64(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    const uint64_t product = uint64_t(e.eax) * e.edx;
    store(app, e.esi, reg32(product));
    store(app, e.esi + 4, reg32(product >> 32));
    e.back(cpu, args, e.esi);
    args.esi = e.frame(4, 0xc) + 8;
    return args.eax;
}

/* 0x503760 (__U8D, stdcall 12): the 64 bits [esp+8]:[esp+4] over [esp+0xc]. */
x86::reg32 div64(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.back(cpu, args, divide64(e.arg(app, 0), e.arg(app, 1), e.arg(app, 2)), 0xc);
}

x86::reg32 platformPacketSize(App*, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.back(cpu, args, reg32(platformPacketBytes()));
}

x86::reg32 findStream(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.back(cpu, args, streamOf(app, int32_t(e.eax)));
}

/* 0x4f3ab5: iSNDstreamsystemtask's way out while a surface is locked, a jump
 * into findStream's end. */
x86::reg32 streamTaskSkipped(App*, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.back(cpu, args, 0);
}

/* 0x4f3b31: removeRequest's way out, which framesPlayed jumps to: ecx and ebx
 * popped. */
x86::reg32 framesPlayedDone(App* app, CPU& cpu, x86::Args& args)
{
    const reg32 esp = cpu.esp;
    args.ecx = app->getMemory<reg32>(esp);
    args.ebx = app->getMemory<reg32>(esp + 4);
    cpu.esp = esp + 12;
    return args.eax;
}

x86::reg32 streamCount(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.back(cpu, args, reg32(streamsMade(app)));
}

x86::reg32 findRequest(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.back(cpu, args, requestOfHandle(app, e.eax));
}

/* The request's state into the four words at edx. */
x86::reg32 SNDSTRM_requeststatus(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, requestStatus(app, cpu, e.esp, e.eax, e.edx));
}

/* edx left as the requests' part. */
x86::reg32 streamMemory(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    e.back(cpu, args, streamBytes(e.eax, e.edx));
    args.edx = e.eax * kRequestSize + sizeof(Stream);
    return args.eax;
}

x86::reg32 playerMemory(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.back(cpu, args, playerBytes(e.eax));
}

/* iSNDstreamcreate with the queue eax given: its arguments edx, ebx, ecx and
 * [esp+4]; the memory's size [esp+8].  stdcall 8. */
x86::reg32 SNDSTRM_createqueued(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args,
             createStream(app, cpu, e.esp - 4 - 3 * 4 - 4, e.edx, e.ebx, e.ecx, e.arg(app, 0), e.arg(app, 1), e.eax, 1),
             8);
    return args.eax;
}

/* A request of the chunks ebx given whole, for time edx on stream eax. */
x86::reg32 SNDSTRM_requestdata(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, requestStream(app, cpu, e.esp - 4 - 4 - 4, e.eax, e.edx, 0, e.ebx, 2));
}

x86::reg32 SNDSTRM_restore(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, destroyEveryStream(app, cpu, e.esp));
}

x86::reg32 removeRequest(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.back(cpu, args, takeOutRequest(app, e.eax));
}

x86::reg32 packetFreed(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, giveBackPacket(app, cpu, e.esp, e.eax));
}

/* edx as the code leaves it: the frames the last request has played. */
x86::reg32 framesPlayed(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    reg32 played;
    const reg32 eax = countPlayedFrames(app, int32_t(e.eax), e.edx, played);
    e.back(cpu, args, eax);
    args.edx = played;
    return args.eax;
}

x86::reg32 iSNDstreamparseheader(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, parseHeader(app, cpu, e.esp, e.eax, e.edx));
}

x86::reg32 iSNDstreamparseloop(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, parseLoop(app, cpu, e.esp, e.eax, e.edx));
}

x86::reg32 iSNDstreamparsedata(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, parseData(app, cpu, e.esp, e.eax, e.edx));
}

x86::reg32 iSNDstreamparseend(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, parseEnd(app, cpu, e.esp, e.eax, e.edx));
}

x86::reg32 iSNDstreamparsechunk(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, parseChunk(app, cpu, e.esp, e.eax, e.edx));
}

x86::reg32 streamWaits(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, waitsForQueue(app, cpu, e.esp, e.eax));
}

x86::reg32 iSNDstreamsystemtask(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, streamSystemTask(app, cpu, e.esp));
}

/* stdcall 12: edx requests, ebx packets, ecx the memory; its size, the queue
 * and whether it is given on the stack. */
x86::reg32 iSNDstreamcreate(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args,
             createStream(app, cpu, e.esp, e.eax, e.edx, e.ebx, e.ecx, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2)),
             0xc);
    return args.eax;
}

/* stdcall 4: the time edx, the bytes ebx, the data ecx; how on the stack. */
x86::reg32 iSNDstreamrequest(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, requestStream(app, cpu, e.esp, e.eax, e.edx, e.ebx, e.ecx, e.arg(app, 0)), 4);
}

x86::reg32 SNDSTRM_destroy(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, destroyStream(app, cpu, e.esp, int32_t(e.eax)));
}

x86::reg32 SNDSTRM_purge(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, purgeStream(app, cpu, e.esp, int32_t(e.eax)));
}

/* The packet player in the memory eax of edx bytes; its free callback ebx and
 * frames callback ecx. */
x86::reg32 SNDPKTPLAY_create(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.back(cpu, args, reg32(createPlayer(app, e.eax, e.edx, e.ebx, e.ecx)));
}

/* The header edx, the patch's settings ebx, the play's options ecx. */
x86::reg32 SNDPKTPLAY_start(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, startPlayer(app, cpu, e.esp, int32_t(e.eax), e.edx, e.ebx, e.ecx));
}

/* The packet edx (its frames +4, its data +0xc). */
x86::reg32 SNDPKTPLAY_submit(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, submitPacket(app, cpu, e.esp, int32_t(e.eax), e.edx));
}

x86::reg32 SNDPKTPLAY_submitspace(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, submitSpace(app, cpu, e.esp, int32_t(e.eax)));
}

x86::reg32 framesOutstanding(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, framesOutstandingOf(app, cpu, e.esp, int32_t(e.eax)));
}

x86::reg32 SNDPKTPLAY_framesoutstanding(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, framesOutstandingLocked(app, cpu, e.esp, int32_t(e.eax)));
}

/* The serials edx to ebx. */
x86::reg32 SNDPKTPLAY_purge(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, purgePackets(app, cpu, e.esp, int32_t(e.eax), int32_t(e.edx), int32_t(e.ebx)));
}

x86::reg32 SNDPKTPLAY_stop(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, stopPlayer(app, cpu, e.esp, int32_t(e.eax)));
}

x86::reg32 SNDPKTPLAY_destroy(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.back(cpu, args, reg32(destroyPlayer(app, int32_t(e.eax))));
}

/* The frames go into [edx]; edx is left at the player. */
x86::reg32 nextPacket(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    reg32 player;
    const Result data = takeNextPacket(app, cpu, e.esp, int32_t(e.eax), e.edx, player);
    if (!data)
        return args.eax;
    e.back(cpu, args, reg32(*data));
    args.edx = player;
    return args.eax;
}

x86::reg32 packetFramesPlayed(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, packetFramesHeard(app, cpu, e.esp, int32_t(e.eax), e.edx, e.eax));
}

x86::reg32 patchDefaults(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    defaultSettings(record<PatchSettings>(app, e.eax));
    return e.back(cpu, args, 0);
}

/* The patch eax made the header edx, with its settings ebx and its frames
 * into [ecx]. */
x86::reg32 iSNDpatchtohdr(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args,
             patchToHeader(app, cpu, e.esp, e.eax, record<StreamHeader>(app, e.edx),
                           record<PatchSettings>(app, e.ebx), e.ecx));
    return args.eax;
}

x86::reg32 iSNDplatformcalcdatarate(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, dataRate(app, cpu, e.esp, record<StreamHeader>(app, e.eax), e.edx));
}

/* The count edx for the queue of stream eax. */
x86::reg32 streamChunksAtOnce(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, chunksAtOnce(app, cpu, e.esp, e.eax, e.edx));
}

}  // namespace port
}  // namespace nfs3hp
