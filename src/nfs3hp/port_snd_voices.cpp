#include "port_snd.h"
#include "port_snd_voices.h"
#include "port_snd_calls.h"
#include "isnd_calls.h"
#include "native_ea_threads.h"

/* EA's SND library, rewritten (port_snd.h says how): its voices -- taken,
 * freed, grouped -- their volume, pan and pitch on the mixer's channels, the
 * 100 Hz tick that moves their ramps, envelopes and LFOs, and the calls the
 * game makes on a sound by its handle.
 *
 * Each does what the game's code does to the game's memory and calls the
 * same of the game's code with the same arguments, from the same place on the
 * stack; its arithmetic is the code's to the bit (imul's wrapping, idiv's
 * quotient towards 0, the x87's arithmetic through fp::). */
namespace nfs3hp
{
namespace port
{

namespace
{

// The errors' sources and functions, and their texts.
constexpr Where kAllocChan{0x54e128, 0x54e138};
constexpr Where kEnter{0x54e314, 0x54e324}, kAddClient{0x54e314, 0x54e38c}, kRemoveClient{0x54e314, 0x54e3d8};
constexpr Where kVolume{0x54e5a8, 0x54e5b4}, kDetune{0x54ea0c, 0x54ea1c}, k3dPos{0x54ea64, 0x54ea74};
constexpr Where kPan{0x54eac0, 0x54eacc}, kStop{0x549b10, 0x54a060}, kAutovol{0x54b278, 0x54b288};
constexpr Where kSvol{0x54b2c0, 0x54b2cc}, kBend{0x54b2fc, 0x54b308}, kSpan{0x54b358, 0x54b364};
constexpr Where kSetdef{0x54b390, 0x54b3a0}, kPlay{0x54b3dc, 0x54b3e8};
constexpr u32 kFreeChanText = 0x54e148, kEnterText = 0x54e334, kAddClientText = 0x54e3a8;
constexpr u32 kRemoveClientText = 0x54e3f4, kVolText = 0x54e5bc, kDetuneText = 0x54ea30, k3dPosText = 0x54ea88;
constexpr u32 kPanText = 0x54ead4, kStopText = 0x54a06c, kAutovolText = 0x54b294, kSvolText = 0x54b2d4;
constexpr u32 kBendText = 0x54b318, kSpanText = 0x54b36c, kSetdefText = 0x54b3b0, kPlayText = 0x54b3f0;

constexpr Table<u32> kRandomState{0x5644e0};  // six words, the lowest last
constexpr Table<u32> kSines{0x5672ac};        // a quarter's, 0x10000 one
constexpr Table<float> kPitchSteps{0x56adec};   // 2 to the power 2^i/4096
constexpr Var<float> kStepScale{0x551af4};

int8_t sbyteAt(App* app, u32 at) { return app->getMemory<int8_t>(at); }
uint8_t byteAt(App* app, u32 at) { return app->getMemory<uint8_t>(at); }

Voice& voice(App* app, int32_t index) { return kVoices(app, index); }
PlatformVoice& platform(App* app, int32_t index) { return kPlatform(app, index); }

/* Whether voice w plays a sound of group `group`. */
bool inGroup(const Voice& w, uint8_t group) { return w.group == group && w.state == 1 && w.handle >= 0; }

/* The two sides' volumes as the game's code leaves them on its stack where
 * an error left them unset. */
Sides unset(App* app, u32 at) { return load<Sides>(app, at); }

}  // namespace

Result fail(App* app, ThreadState& ts, u32 esp, Where where, u32 line, std::initializer_list<u32> arguments,
            u32 eax)
{
    constexpr Table<u32> kWhere{0x552190};
    constexpr u32 kError = 0x401010;
    kWhere.set(app, 0, where.file);
    kWhere.set(app, 1, where.function);
    kWhere.set(app, 2, line);
    if (!pointerGame(app, ts, esp, kError, {eax}, arguments, eax))
        return {};
    return int32_t(eax);
}

/* 0x4e0d88's: EA's random number -- the sum of a 192-bit state with its
 * carries, the state then counted up by one. */
u32 nextRandom(App* app)
{
    uint64_t sum = uint64_t(kRandomState(app, 5)) + kRandomState(app, 4);
    kRandomState.set(app, 4, u32(sum));
    for (const int32_t i : {3, 2, 1})
    {
        sum = uint64_t(u32(sum)) + kRandomState(app, i) + (sum >> 32);
        kRandomState.set(app, i, u32(sum));
    }
    u32 n = u32(sum) + kRandomState(app, 0) + u32(sum >> 32);
    bool carry = true;
    for (const int32_t i : {5, 4, 3, 2, 1})
    {
        kRandomState.set(app, i, kRandomState(app, i) + 1);
        if (kRandomState(app, i) != 0)
        {
            carry = false;
            break;
        }
    }
    if (carry)
        ++n;
    kRandomState.set(app, 0, n);
    return n;
}

/* 0x5224e0's: a random offset of up to `range` either way (at most 0x10000,
 * none below 0). */
int32_t randomWithin(App* app, int32_t range)
{
    if (range > 0x10000)
        range = 0x10000;
    else if (range < 0)
        range = 0;
    const int32_t r = int32_t(nextRandom(app) & 0x7fff) - 0x4000;
    return imul(r, range) >> 14;
}

/* 0x52b230's: `bytes` bytes at `at`, big-endian; 1 to 3 of them signed. */
int32_t bigEndian(App* app, u32 at, int32_t bytes)
{
    u32 value = 0;
    for (u32 left = u32(bytes); left != 0; --left)
        value = (value << 8) + byteAt(app, at++);
    if (bytes == 1 && int32_t(value) > 0x7f)
        value -= 0x100;
    else if (bytes == 2 && int32_t(value) > 0x7fff)
        value -= 0x10000;
    else if (bytes == 3 && int32_t(value) > 0x7fffff)
        value -= 0x1000000;
    return int32_t(value);
}

/* 0x5229d0's: the next of a patch's tags, read on from `t.place`: its tag,
 * and for a tag with data its first up to 4 bytes as a number and where they
 * are.  0xfc pads; 0xff ends (false); 0xfd and 0xfe have no data.  A length
 * of 0xff is followed by a 4-byte one. */
bool readTag(App* app, Tag& t)
{
    while (byteAt(app, t.place) == 0xfc)
        ++t.place;
    t.tag = byteAt(app, t.place);
    if (t.tag == 0xff)
        return false;
    ++t.place;
    if (t.tag == 0xfd || t.tag == 0xfe)
        return true;
    u32 length = byteAt(app, t.place);
    if (length == 0xff)
    {
        length = u32(bigEndian(app, t.place + 1, 4));
        t.place += 4;
    }
    ++t.place;
    if (int32_t(length) >= 0 && int32_t(length) <= 4)
    {
        t.number = u32(bigEndian(app, t.place, int32_t(length)));
        t.data = t.place;
    }
    t.place += length;
    return true;
}

/* 0x5222f0's: `volume` panned by `pan` (0..127, 0x40 the middle), the far
 * side scaled down. */
Sides splitPan(int32_t pan, int32_t volume)
{
    Sides s{volume, volume};
    if (pan < 0x40)
        s.right = imul(pan, volume) >> 6;
    else if (pan > 0x40)
        s.left = imul(0x7f - pan, volume) >> 6;
    return s;
}

/* 0x52b1c0's: `volume` at `angle` (0x10000 a turn): the volume times 1 - sin
 * and 1 + sin, from the quarter's table, each at most 0x7f. */
Sides panAt(App* app, u32 angle, u32 volume)
{
    const u32 a = u32(int32_t(angle) >> 6);
    const u32 negative = (a & 0x200) ? 0xffffffff : 0;
    const u32 mirrored = (a & 0x100) ? 0xffffffff : 0;
    const u32 index = ((a ^ mirrored) & 0xff) - mirrored;
    const u32 sine = (kSines(app, int32_t(index)) ^ negative) - negative;
    const u32 left = (u32(0x10000) - sine) * volume >> 16;
    const u32 right = volume * (sine + 0x10000) >> 16;
    return {int32_t(left) > 0x7f ? 0x7f : int32_t(left), int32_t(right) > 0x7f ? 0x7f : int32_t(right)};
}

/* 0x5224b0's: panAt, both sides made positive. */
Sides panAtPositive(App* app, u32 angle, u32 volume)
{
    Sides s = panAt(app, angle, volume);
    if (s.left < 0)
        s.left = negated(s.left);
    if (s.right < 0)
        s.right = negated(s.right);
    return s;
}

/* 0x522320's: panAt for a voice played in Dolby surround, from the
 * surround's two levels it had: the side turned around that puts the sound
 * behind (a level below 0 is a side turned around). */
Sides panSurroundAt(App* app, u32 angle, u32 volume, int32_t front, int32_t back)
{
    Sides s = panAt(app, angle, volume);
    int32_t quadrant;
    if (front >= 0 && back >= 0)
        quadrant = back <= front ? 1 : 2;
    else if (front < 0 && back < 0)
        quadrant = back <= front ? 2 : 1;
    else
    {
        const int32_t f = front <= 0 ? negated(front) : front, b = back <= 0 ? negated(back) : back;
        quadrant = f <= b ? 3 : 4;
    }
    int32_t side = 0;
    if (!(int32_t(angle) < 0x4000 || int32_t(angle) >= 0xc000))
    {
        const int32_t louder = s.left >= s.right ? 4 : 3;
        if (quadrant <= 2)
            side = louder == 3 && quadrant == 2 ? 1 : 2;
    }
    else
    {
        const int32_t louder = s.left >= s.right ? 1 : 2;
        if (quadrant >= 3)
            side = louder == 2 && quadrant == 3 ? 1 : 2;
    }
    if (front < 0)
        s.left = negated(s.left);
    if (back < 0)
        s.right = negated(s.right);
    if (side == 1)
        s.left = negated(s.left);
    else if (side == 2)
        s.right = negated(s.right);
    return s;
}

/* 0x4ffc9c's: the voice a handle is on (its low five bits), or -8 when that
 * voice is free or holds another sound. */
int32_t voiceOfHandle(App* app, u32 handle)
{
    const int32_t index = int32_t(handle & 0x1f);
    if (int32_t(handle) >= 0 && index < kVoiceCount && voice(app, index).state != 0
        && handle == u32(voice(app, index).handle))
        return index;
    return kNoSound;
}

/* 0x4ff840's: whether voice `index` is among the first `count` allocVoices
 * picked. */
bool isPicked(App* app, int32_t index, int32_t count)
{
    bool picked = false;
    for (int32_t i = 0; i < count; ++i)
        if (kPicked(app, i) == index)
            picked = true;
    return picked;
}

/* 0x4ffb64's (iSNDfreechan): voice `index` freed, the tick it was freed at
 * kept.  A voice of a group ends the group's sound only when it is the last
 * of it playing, or the one marked to end it is ending already.  The ticks,
 * or the marked voice's offset, as the code leaves them. */
Result freeVoice(App* app, ThreadState& ts, u32 esp, int32_t index)
{
    Voice& v = voice(app, index);
    const uint8_t group = v.group;
    if (group != 0)
    {
        int32_t count = 0, last = -1;
        for (int32_t i = 0; i < kVoiceCount; ++i)
        {
            const Voice& w = voice(app, i);
            if (w.group == group && w.handle >= 0 && w.state != 0)
            {
                ++count;
                if (w.endsGroup != 0)
                    last = i;
            }
        }
        if (count != 1)
        {
            // With none marked, the code looks a voice before the first.
            Voice& marked = voice(app, last);
            if (marked.state == 2 && index != last && count == 2)
            {
                v.state = 0;
                v.tick = kTicks(app);
                marked.state = 0;
                marked.tick = kTicks(app);
                return last * int32_t(sizeof(Voice));
            }
            if (marked.state == 1 && index == last)
            {
                marked.state = 2;
                return last * int32_t(sizeof(Voice));
            }
        }
    }
    else if (v.state == 0 && !fail(app, ts, esp - 0x18, kAllocChan, 0x12a, {kFreeChanText, u32(index)}))
        return {};
    v.state = 0;
    v.tick = kTicks(app);
    return int32_t(kTicks(app));
}

/* 0x4ff86c's (iSNDallocchan): `count` voices of `mask` at `priority`: free
 * ones first, the longest free, then playing ones of the lowest priority
 * below 0x66, the oldest first -- their sounds stopped.  The handle -- new
 * high bits with the first voice -- into `handle` and onto the first voice;
 * the others marked as led by it.  The first voice, or -9. */
Result allocVoices(App* app, ThreadState& ts, u32 esp, uint32_t mask, int32_t count, uint8_t priority, u32& handle)
{
    const u32 frame = esp - 0xc - 0x24;
    for (int32_t i = 0; i < count; ++i)
        kPicked.set(app, i, -1);
    kHandleSerial.set(app, int32_t(u32(kHandleSerial(app)) + 0x20));
    if (kHandleSerial(app) < 0)
        kHandleSerial.set(app, 0);
    const int32_t used = kVoicesUsed(app);
    int32_t found = 0;
    for (int32_t k = 0; k < count; ++k)
    {
        int32_t best = -1;
        uint32_t bestTime = 0xffffffff;
        for (int32_t i = 0; i < used; ++i)
        {
            const Voice& w = voice(app, i);
            if (!(mask & (1u << (i & 31))) || w.state != 0 || isPicked(app, i, found) || bestTime <= w.tick)
                continue;
            bestTime = w.tick;
            best = i;
        }
        if (best >= 0)
            kPicked.set(app, found++, int8_t(best));
    }
    for (int32_t k = found; k < count; ++k)
    {
        uint32_t lowest = 0x66, bestTime = 0xffffffff;
        int32_t best = -1;
        for (int32_t i = 0; i < used; ++i)
        {
            const Voice& w = voice(app, i);
            if (!(mask & (1u << (i & 31))) || isPicked(app, i, found))
                continue;
            if (int32_t(w.priority) < int32_t(lowest))
            {
                lowest = w.priority;
                bestTime = w.tick;
                best = i;
            }
            else if (w.priority == lowest && w.tick < bestTime)
            {
                bestTime = w.tick;
                best = i;
            }
        }
        if (best >= 0)
        {
            kPicked.set(app, found++, int8_t(best));
            if (found >= count)
                break;
        }
    }
    if (found != count)
        return kNoVoices;
    const int32_t first = kPicked(app, 0);
    handle = u32(kHandleSerial(app)) | u32(first);
    for (int32_t j = 0; j < found; ++j)
    {
        Voice& w = voice(app, kPicked(app, j));
        if (w.state == 1)
        {
            const u32 playing = u32(w.handle >= 0 ? w.handle : voice(app, w.leader).handle);
            if (!stopSound(app, ts, frame - 4, playing))
                return {};
            const Result ended = soundStatus(app, ts, frame - 4, playing);
            if (!ended)
                return {};
            if (*ended != 1)
            {
                for (int32_t b = j - 1; b >= 0; --b)
                    voice(app, kPicked(app, b)).state = 0;
                return kNoVoices;
            }
        }
        w.state = 1;
        w.tick = kTicks(app);
        w.priority = priority;
    }
    Voice& leader = voice(app, first);
    leader.handle = int32_t(handle);
    for (int32_t e = 1; e < found; ++e)
    {
        // Past the fourth, on into the patch's number, as the code has it.
        app->getMemory<uint8_t>(kVoices.address(kPicked(app, 0)) + 3 + u32(e)) = uint8_t(kPicked(app, e));
        Voice& w = voice(app, kPicked(app, e));
        w.handle = -1;
        w.leader = kPicked(app, 0);
    }
    return first;
}

/* 0x500e20's (iSNDenteraudio): into the library's lock -- an error first when
 * a DirectDraw surface is locked, outside the tick.  eax as the lock leaves
 * it: the surface's answer, or the error's. */
Result enterAudio(App* app, ThreadState& ts, u32 esp)
{
    ts.sp = esp - 0xc;
    const bool locked = guest::direct<kSurfaceLocked, ea::surfaceIsLocked>(app, ts);
    if (ts.terminate)
        return {};
    u32 eax = locked ? 1 : 0;
    if (eax != 0 && kInTick(app) == 0)
    {
        const Result r = fail(app, ts, esp - 0x10, kEnter, 0x114, {kEnterText}, eax);
        if (!r)
            return {};
        eax = u32(*r);
    }
    if (!lockEnter(app, ts, esp - 0xc, kLock(app)))
        return {};
    kEntered.set(app, uint8_t(kEntered(app) + 1));
    return int32_t(eax);
}

/* 0x500e7c's (iSNDleaveaudio): out of the library's lock.  eax as the lock
 * leaves it: what the code had in it -- which only the game's callers of
 * iSNDleaveaudio read. */
Result leaveAudio(App* app, ThreadState& ts, u32 esp, u32 eax)
{
    kEntered.set(app, uint8_t(kEntered(app) - 1));
    if (!lockLeave(app, ts, esp - 8, kLock(app)))
        return {};
    return int32_t(eax);
}

/* 0x500e94's (iSNDserveradd100hzclient): `client` among the tick's clients;
 * its place. */
Result addTickClient(App* app, ThreadState& ts, u32 esp, u32 client)
{
    if (kClientCount(app) >= 6 && !fail(app, ts, esp - 0x10, kAddClient, 0x149, {kAddClientText}))
        return {};
    const int32_t n = kClientCount(app);
    kClients.set(app, n, client);
    kClientCount.set(app, int8_t(kClientCount(app) + 1));
    return n;
}

/* 0x500ef0's (iSNDserverremove100hzclient): `client` no longer a client, those
 * after it moved down; an error when it was none. */
Result removeTickClient(App* app, ThreadState& ts, u32 esp, u32 client)
{
    for (int32_t i = 0; i < kClientCount(app); ++i)
        if (kClients(app, i) == client)
        {
            kClientCount.set(app, int8_t(kClientCount(app) - 1));
            for (; i < kClientCount(app); ++i)
                kClients.set(app, i, kClients(app, i + 1));
            return i;
        }
    return fail(app, ts, esp - 0xc, kRemoveClient, 0x165, {kRemoveClientText}, client);
}

/* 0x500654's: channel `c`'s effect send: the mean of its two targets times
 * its send level, to its send's volume function.  That one's eax, or the
 * channel's address with no send. */
Result sendVolume(App* app, ThreadState& ts, u32 esp, u32 c)
{
    Channel& ch = record<Channel>(app, c);
    if (ch.sendLevel == 0)
    {
        ch.send = 0;
        return int32_t(c);
    }
    ch.send = int8_t((ch.leftTarget + ch.rightTarget) >> 1);
    ch.send = int8_t(imul(ch.sendLevel, ch.send) >> 7);
    const u32 level = u32(int32_t(ch.send));
    u32 eax;
    if (!pointerGame(app, ts, esp - 8, app->getMemory<u32>(c + kChannelSetSend), {c}, {c + kChannelSend, level, level},
                  eax))
        return {};
    return int32_t(eax);
}

/* 0x5006a4's: channel `index`'s volume to `left` and `right`, ramped there a
 * step a frame; and its send.  0. */
Result channelVolume(App* app, ThreadState& ts, u32 esp, int32_t index, int32_t left, int32_t right)
{
    Channel& ch = kChannels(app, index);
    ch.leftTarget = int8_t(left);
    ch.rightTarget = int8_t(right);
    ch.leftWay = ch.left < left ? 1 : -1;
    ch.rightWay = ch.right < right ? 1 : -1;
    if (!sendVolume(app, ts, esp - 8, kChannels.address(index)))
        return {};
    return 0;
}

/* 0x52b500's: channel `index`'s send level, and its send set. */
Result channelSendLevel(App* app, ThreadState& ts, u32 esp, int32_t index, uint8_t level)
{
    kChannels(app, index).sendLevel = int8_t(level);
    return sendVolume(app, ts, esp - 8, kChannels.address(index));
}

/* 0x523310's: voice `index`'s send level, but for a 3D voice.  0. */
Result voiceSendLevel(App* app, ThreadState& ts, u32 esp, int32_t index, int32_t level)
{
    if (platform(app, index).method != 3 && !channelSendLevel(app, ts, esp - 8, index, uint8_t(level)))
        return {};
    return 0;
}

/* 0x5014a0's: what voice `index` plays at -- its volume times the voice's,
 * the envelope's and the master's, through its LFO and its volume curve.
 * eax as the code leaves it. */
int32_t refreshVolume(App* app, int32_t index)
{
    Voice& v = voice(app, index);
    int32_t a = imul(int16_t(uint32_t(v.volume) >> 16), v.level);
    a = imul(a, int16_t(uint32_t(v.envelope) >> 16));
    a = imul(int8_t(kMasterVolume(app)), a);
    int32_t eax = idiv(a, 0x1f417f);
    v.mixLevel = int8_t(eax);
    if (v.volumeLfo != 0)
    {
        eax = idiv(imul(sbyteAt(app, v.volumeLfo + v.volumeLfoPlace), v.mixLevel), 0x7f);
        v.mixLevel = int8_t(eax);
    }
    if (v.volumeCurve != 0)
    {
        const u32 i = u32(int32_t(v.mixLevel));
        eax = int32_t((i & 0xffffff00) | byteAt(app, v.volumeCurve + i));
        v.mixLevel = int8_t(eax);
    }
    return eax;
}

/* 0x501540's (iSNDvol): voice `index` to `volume` on its channel, panned as it
 * is (in surround or not); eacsnd's for a 3D voice.  A Dolby voice with no
 * pan set takes the angle the code never set: `ebp`, what its caller had in
 * ebp. */
Result voiceVolume(App* app, ThreadState& ts, u32 esp, int32_t index, int32_t volume, u32 ebp)
{
    PlatformVoice& p = platform(app, index);
    const u32 frame = esp - 0x14 - 8;
    Sides s;
    if (p.method == 3)
    {
        const std::optional<u32> eax = guest::soundFrom<isnd::kDirectVol, &eacsnd::volume>(
            app, ts, frame, u32(volume), u32(index), u32(volume));
        if (!eax)
            return {};
        return int32_t(*eax);
    }
    if (p.method == 0 || p.method == 2)
        s = p.panSet != 0 ? splitPan(int8_t(voice(app, index).pan), volume)
                          : panAtPositive(app, p.position, u32(volume));
    else if (p.method == 1)
    {
        const u32 angle = p.panSet != 0 ? u32(int8_t(voice(app, index).pan) - 0x40) << 8 : ebp;
        s = panSurroundAt(app, angle, u32(volume), p.surroundLeft, p.surroundRight);
        p.surroundLeft = int8_t(s.left);
        p.surroundRight = int8_t(s.right);
    }
    else
    {
        if (!fail(app, ts, frame, kVolume, 0x30, {kVolText}))
            return {};
        s = unset(app, frame);
    }
    return channelVolume(app, ts, frame - 4, index, s.left, s.right);
}

/* 0x502cb0's: voice `index`'s pitch with its bend (through its curve) and its
 * LFO.  The voice's address, as the code leaves it. */
u32 refreshPitch(App* app, int32_t index)
{
    Voice& v = voice(app, index);
    int32_t pitch = v.pitch;
    if (v.bendRange != 0)
    {
        const int32_t e = v.bendCurve != 0 ? sbyteAt(app, v.bendCurve + v.bend) : int32_t(v.bend);
        pitch += imul(e - 0x40, v.bendRange) >> 6;
    }
    if (v.pitchLfo != 0)
        pitch += imul(sbyteAt(app, v.pitchLfo + v.pitchLfoPlace) - 0x40, v.pitchLfoDepth) >> 6;
    v.playedPitch = int16_t(pitch);
    return kVoices.address(index);
}

/* 0x522e90's: 2 to the power cents/4096, as a float of the x87's: the table
 * holds it for each bit. */
nfs::Float pitchScale(App* app, ThreadState& ts, int32_t cents)
{
    float f = 1.0f;
    const bool down = cents <= 0;
    const int32_t n = down ? imul(cents, -1) : cents;
    u32 bit = 1;
    for (int32_t k = 0; int32_t(bit) <= n; ++k, bit += bit)
        if (bit & u32(n))
        {
            const nfs::Float t = nfs::Float(kPitchSteps(app, k));
            f = float(down ? fp::div(nfs::Float(f), t) : fp::mul(nfs::Float(f), t));
        }
    return nfs::Float(f);
}

/* 0x522f10's: the mixer's step for a sample of `rate` at `pitch`, the mixer
 * at `mixRate`: rate * 2^(pitch/4096) / mix rate * [0x551af4], towards 0. */
int32_t stepFor(App* app, ThreadState& ts, int32_t rate, int32_t mixRate, int32_t pitch)
{
    nfs::Float f = pitchScale(app, ts, pitch);
    f = fp::mul(f, nfs::Float(rate));
    f = fp::div(f, nfs::Float(mixRate));
    f = fp::mul(f, nfs::Float(kStepScale(app)));
    return chopped<int32_t>(ts, f);
}

/* 0x522f50's: channel `index`'s step (at most 0x80000), through its
 * resampler's function.  That one's eax. */
Result channelStep(App* app, ThreadState& ts, u32 esp, int32_t index, uint32_t step)
{
    const u32 c = kChannels.address(index);
    if (step > 0x80000)
        step = 0x80000;
    u32 eax;
    if (!pointerGame(app, ts, esp - 4, app->getMemory<u32>(c + kChannelSetStep), {c, c + kChannelResampler, 0, step},
                  {c + kChannelResampler, step}, eax))
        return {};
    return int32_t(eax);
}

/* 0x502d30's (iSNDplatformdetune): voice `index` to `pitch`: its channel's
 * step, or eacsnd's rate for a 3D voice.  0, or -7 for no such method. */
Result detuneVoice(App* app, ThreadState& ts, u32 esp, int32_t index, int32_t pitch)
{
    const PlatformVoice& p = platform(app, index);
    const u32 frame = esp - 0xc - 4;
    if (p.method <= 2)
    {
        const int32_t step = stepFor(app, ts, p.rate, kMixRate(app), pitch);
        if (!channelStep(app, ts, frame - 4, index, uint32_t(step)))
            return {};
        return 0;
    }
    if (p.method == 3)
    {
        const nfs::Float f = fp::mul(pitchScale(app, ts, pitch), nfs::Float(int32_t(p.rate)));
        const u32 rate = u32(chopped<int32_t>(ts, f));
        const std::optional<u32> eax =
            guest::soundFrom<isnd::kDirectRate, &eacsnd::rate>(app, ts, frame, p.rate, u32(index), rate);
        if (!eax)
            return {};
        return int32_t(*eax);
    }
    if (!fail(app, ts, frame, kDetune, 0x22, {kDetuneText}))
        return {};
    return kNoMethod;
}

/* 0x502df0's (iSNDplatform3dpos): voice `index` to `position` (its low half
 * kept) and `elevation`: its channel's volumes panned by it (in surround or
 * not), or eacsnd's.  That one's eax. */
Result placeVoice(App* app, ThreadState& ts, u32 esp, int32_t index, uint32_t position, int32_t elevation)
{
    PlatformVoice& p = platform(app, index);
    const u32 frame = esp - 0xc - 8;
    p.panSet = 0;
    p.position = uint16_t(position);
    const u32 volume = u32(int32_t(voice(app, index).mixLevel));
    Sides s;
    if (p.method == 3)
    {
        const std::optional<u32> eax = guest::soundFrom<isnd::kDirectPos3d, &eacsnd::position>(
            app, ts, frame, position, u32(index), position, u32(elevation));
        if (!eax)
            return {};
        return int32_t(*eax);
    }
    if (p.method == 1)
    {
        s = panSurroundAt(app, position, volume, p.surroundLeft, p.surroundRight);
        p.surroundLeft = int8_t(s.left);
        p.surroundRight = int8_t(s.right);
    }
    else if (p.method == 0 || p.method == 2)
        s = panAtPositive(app, position, volume);
    else
    {
        if (!fail(app, ts, frame, k3dPos, 0x29, {k3dPosText}))
            return {};
        s = unset(app, frame);
    }
    return channelVolume(app, ts, frame - 4, index, s.left, s.right);
}

/* 0x502ef0's (iSNDpan): voice `index` to `pan` (0..127): its channel's
 * volumes split by it (in surround or not), or eacsnd's for a 3D voice.  That
 * one's eax. */
Result panVoice(App* app, ThreadState& ts, u32 esp, int32_t index, int32_t pan)
{
    PlatformVoice& p = platform(app, index);
    const u32 frame = esp - 0x14 - 8;
    p.panSet = 1;
    const u32 volume = u32(int32_t(voice(app, index).mixLevel));
    Sides s;
    if (p.method == 0 || p.method == 2)
        s = splitPan(pan, int32_t(volume));
    else
    {
        const u32 angle = u32(pan - 0x40) << 8;
        if (p.method == 3)
        {
            const std::optional<u32> eax = guest::soundFrom<isnd::kDirectPos3d, &eacsnd::position>(
                app, ts, frame, (u32(index) & 0xffff00ff) | u32(p.method) << 8, u32(index), angle, 0u);
            if (!eax)
                return {};
            return int32_t(*eax);
        }
        if (p.method == 1)
        {
            s = panSurroundAt(app, angle, volume, p.surroundLeft, p.surroundRight);
            p.surroundLeft = int8_t(s.left);
            p.surroundRight = int8_t(s.right);
        }
        else
        {
            if (!fail(app, ts, frame, kPan, 0x2e, {kPanText}))
                return {};
            s = unset(app, frame);
        }
    }
    return channelVolume(app, ts, frame - 4, index, s.left, s.right);
}

/* 0x500604's (iSNDmixstop): channel `index` silenced at once -- its volume to
 * 0 and ramped there over a block -- freed, and the voice freed (what the
 * mixer calls when a channel ends).  0. */
Result silenceChannel(App* app, ThreadState& ts, u32 esp, int32_t index)
{
    const u32 frame = esp - 0x14;
    const u32 c = kChannels.address(index);
    if (!channelVolume(app, ts, frame - 4, index, 0, 0))
        return {};
    ts.sp = frame;
    guest::direct<kRamp, rampChannel>(app, ts, frame - 4, c, 0x100u);
    if (ts.terminate || !pointerGame(app, ts, frame, kEmms(app)))
        return {};
    kChannels(app, index).playing = 0;
    if (!pointerGame(app, ts, frame, kChannelEnded(app), {u32(index)}))
        return {};
    return 0;
}

/* 0x4e25c0's (iSNDstop): voice `index` stopped, on its channel or eacsnd's.
 * That one's eax, or -7 for no such method. */
Result stopVoice(App* app, ThreadState& ts, u32 esp, int32_t index)
{
    const u32 frame = esp - 0xc;
    const uint8_t method = platform(app, index).method;
    if (method <= 2)
        return silenceChannel(app, ts, frame - 4, index);
    if (method == 3)
    {
        const std::optional<u32> eax =
            guest::soundFrom<isnd::kDirectStopBuf, &eacsnd::stopBuffer>(app, ts, frame, u32(index), u32(index));
        if (!eax)
            return {};
        return int32_t(*eax);
    }
    if (!fail(app, ts, frame, kStop, 0x3a3, {kStopText}))
        return {};
    return kNoMethod;
}

/* 0x500c80's: the library's 100 Hz tick -- its clients (the streams), then
 * each playing voice's LFOs, volume ramp and envelope moved on, its pitch
 * and volume set again where they changed; a voice whose volume or envelope
 * ran out stopped.  The code keeps the volume it moved to, the ramp's target
 * or a segment's ticks in ebp, where iSNDvol finds the angle of a Dolby voice
 * with no pan: so does the rewrite, for what it calls (`ebp` the caller's).
 * The clients get the caller's edx and ecx, the code's own ebx (the client's
 * number).  eax as the code leaves it (`eax` what the tick was called with). */
Result tickSoundFrom(App* app, ThreadState& ts, u32 esp, u32 eax, u32 edx, u32 ecx, u32 ebp)
{
    const u32 frame = esp - 0x18;
    kInTick.set(app, uint8_t(kInTick(app) + 1));
    kTicks.set(app, kTicks(app) + 1);
    for (int32_t i = 0; i < kClientCount(app); ++i)
    {
        ts.sp = frame;
        const std::optional<u32> r =
            guest::pointerCall(app, ts, kClients(app, i), {u32(4 * i), edx, u32(i), ecx});
        if (!r)
            return {};
        eax = *r;
    }
    u32 kept = ebp;
    for (int32_t index = 0; index < kVoiceCount; ++index)
    {
        Voice& v = voice(app, index);
        if (v.state != 1 || v.handle < 0)
            continue;
        if (v.pitchLfo != 0)
        {
            if (++v.pitchLfoPlace >= v.pitchLfoLength)
                v.pitchLfoPlace = 0;
            refreshPitch(app, index);
            const Result r = detuneVoice(app, ts, frame - 4, index, v.playedPitch);
            if (!r)
                return {};
            eax = u32(*r);
        }
        bool changed = false;
        if (v.volumeLfo != 0)
        {
            changed = true;
            if (++v.volumeLfoPlace >= v.volumeLfoLength)
                v.volumeLfoPlace = 0;
        }
        bool stop = false;
        if (v.volumeStep != 0)
        {
            const int32_t value = int32_t(u32(v.volume) + u32(v.volumeStep));
            changed = true;
            v.volume = value;
            kept = u32(v.volumeStep < 0 ? v.volumeTarget : value);
            if (v.volumeStep < 0 ? value <= v.volumeTarget : value >= v.volumeTarget)
            {
                v.volumeStep = 0;
                v.volume = v.volumeTarget;
            }
            stop = v.volume < 0;
        }
        if (!stop)
        {
            if (v.envelopeStep != 0)
            {
                v.envelope = int32_t(u32(v.envelope) + u32(v.envelopeStep));
                changed = true;
            }
            if (v.envelopeTicks == 0)
            {
                if (++v.segment >= v.segments)
                    stop = true;
                else
                {
                    const u32 segment = v.envelopeSegments + u32(int32_t(v.segment)) * 8;
                    const int32_t ticks = app->getMemory<int32_t>(segment);
                    v.envelopeTicks = ticks < 0 ? 0x7fffffff : ticks;
                    kept = u32(v.envelopeTicks);
                    v.envelopeStep = idiv(int32_t((app->getMemory<u32>(segment + 4) << 16) - u32(v.envelope)),
                                          v.envelopeTicks);
                }
            }
            if (!stop)
            {
                v.envelopeTicks = int32_t(u32(v.envelopeTicks) - 1);
                if (changed)
                {
                    eax = u32(refreshVolume(app, index));
                    if (v.handle >= 0)
                    {
                        const Result r = voiceVolume(app, ts, frame - 4, index, v.mixLevel, kept);
                        if (!r)
                            return {};
                        eax = u32(*r);
                    }
                }
                continue;
            }
        }
        const Result r = stopSound(app, ts, frame - 4, u32(v.handle));
        if (!r)
            return {};
        eax = u32(*r);
    }
    kInTick.set(app, uint8_t(kInTick(app) - 1));
    return int32_t(eax);
}

/* 0x4e9670's (SNDstop): the sound of `handle` stopped -- its voice, or every
 * voice of its group.  0, -8 for no such sound, -10 before SNDinit. */
Result stopSound(App* app, ThreadState& ts, u32 esp, u32 handle)
{
    if (kInited(app) == 0)
        return kNotStarted;
    const u32 frame = esp - 0x10;
    if (!enterAudio(app, ts, frame - 4))
        return {};
    const int32_t index = voiceOfHandle(app, handle);
    int32_t result = 0;
    if (index < 0)
        result = kNoSound;
    else if (const uint8_t group = voice(app, index).group; group == 0)
    {
        if (!stopVoice(app, ts, frame - 4, index))
            return {};
    }
    else
        for (int32_t j = 0; j < kVoiceCount; ++j)
            if (inGroup(voice(app, j), group) && !stopVoice(app, ts, frame - 4, j))
                return {};
    if (!leaveAudio(app, ts, frame - 4))
        return {};
    return result;
}

/* 0x4e8fd0's (SNDstatus): 0 while the sound of `handle` plays, 1 when it no
 * longer does, -10 before SNDinit. */
Result soundStatus(App* app, ThreadState& ts, u32 esp, u32 handle)
{
    if (kInited(app) == 0)
        return kNotStarted;
    const u32 frame = esp - 4;
    if (!enterAudio(app, ts, frame - 4))
        return {};
    const int32_t index = voiceOfHandle(app, handle);
    if (!leaveAudio(app, ts, frame - 4))
        return {};
    return index < 0 ? 1 : 0;
}

/* 0x4e9010's (SNDstopall): every voice's sound stopped.  The last one's
 * answer. */
Result stopAllSounds(App* app, ThreadState& ts, u32 esp)
{
    Result r = 0;
    for (int32_t i = 0; i < kVoiceCount; ++i)
        if (!(r = stopSound(app, ts, esp - 8, u32(voice(app, i).handle))))
            return {};
    return r;
}

/* 0x4e9420's (SNDautovol): the sound of `handle` (its group's voices) to
 * `target` over `ticks` (at least 1).  0, -8 for no such sound, -10 before
 * SNDinit. */
Result fadeSound(App* app, ThreadState& ts, u32 esp, u32 handle, int32_t ticks, int32_t target)
{
    if (kInited(app) == 0)
        return kNotStarted;
    const u32 frame = esp - 0x10;
    if ((target > 0x7f || target < -1) && !fail(app, ts, frame, kAutovol, 0x40, {kAutovolText, u32(target)}))
        return {};
    if (!enterAudio(app, ts, frame - 4))
        return {};
    const int32_t index = voiceOfHandle(app, handle);
    if (index >= 0)
    {
        const Voice& v = voice(app, index);
        if (ticks <= 0)
            ticks = 1;
        const int32_t to = int32_t(u32(target) << 16);
        for (int32_t j = 0; j < kVoiceCount; ++j)
        {
            Voice& w = voice(app, j);
            if (v.group == 0 ? &w != &v : !inGroup(w, v.group))
                continue;
            w.volumeTarget = to;
            w.volumeStep = idiv(int32_t(u32(to) - u32(w.volume)), ticks);
        }
    }
    if (!leaveAudio(app, ts, frame - 4))
        return {};
    return index < 0 ? kNoSound : 0;
}

/* 0x4e9540's: the sound of `handle` (its group's voices) to `volume` at once,
 * in the library's lock.  0, -8 for no such sound. */
Result volumeNow(App* app, ThreadState& ts, u32 esp, u32 handle, int32_t volume, u32 ebp)
{
    const u32 frame = esp - 0xc;
    const int32_t index = voiceOfHandle(app, handle);
    if (index < 0)
        return kNoSound;
    const Voice& v = voice(app, index);
    const int32_t to = int32_t(u32(volume) << 16);
    if (to == v.volume)
        return 0;
    for (int32_t j = 0; j < kVoiceCount; ++j)
    {
        Voice& w = voice(app, j);
        if (v.group == 0 ? &w != &v : !inGroup(w, v.group))
            continue;
        w.volume = to;
        w.volumeStep = 0;
        refreshVolume(app, j);
        if (!voiceVolume(app, ts, frame - 8, j, w.mixLevel, ebp))
            return {};
    }
    return 0;
}

/* 0x4e95fc's (SNDvol): volumeNow, the volume checked (0..127); ebp the caller's, for iSNDvol. */
Result setVolumeFrom(App* app, ThreadState& ts, u32 esp, u32 handle, int32_t volume, u32 ebp)
{
    if (kInited(app) == 0)
        return kNotStarted;
    const u32 frame = esp - 4;
    if ((volume > 0x7f || volume < 0) && !fail(app, ts, esp - 0x10, kSvol, 0x7b, {kSvolText, u32(volume)}))
        return {};
    if (!enterAudio(app, ts, frame - 4))
        return {};
    const Result r = volumeNow(app, ts, frame - 4, handle, volume, ebp);
    if (!r || !leaveAudio(app, ts, frame - 4))
        return {};
    return r;
}

/* 0x4e9710's: the sound of `handle` (its group's voices) to `bend`, where the
 * bend moves its pitch.  0, -8 for no such sound. */
Result bendNow(App* app, ThreadState& ts, u32 esp, u32 handle, int32_t bend)
{
    const u32 frame = esp - 0x10 - 4;
    const int32_t index = voiceOfHandle(app, handle);
    if (index < 0)
        return kNoSound;
    const Voice& v = voice(app, index);
    for (int32_t j = 0; j < kVoiceCount; ++j)
    {
        Voice& w = voice(app, j);
        if (v.group == 0 ? &w != &v : !inGroup(w, v.group))
            continue;
        if (u32(w.bend) == u32(bend) || w.bendRange == 0)
            continue;
        w.bend = uint8_t(bend);
        refreshPitch(app, j);
        if (!detuneVoice(app, ts, frame - 4, j, w.playedPitch))
            return {};
    }
    return 0;
}

/* SNDpitchbend as C++ (port_sound_player.cpp): the registers its error text reads 0. */
Result setBend(App* app, ThreadState& ts, u32 esp, u32 handle, int32_t bend)
{
    return setBendFrom(app, ts, esp, handle, bend, 0, 0, 0);
}

/* 0x4e97e4's (SNDpitchbend): bendNow, the bend checked (0..127) -- its error
 * text's %i reading the registers the code pushes around the call: ebx,
 * esi and edi, the game's caller's. */
Result setBendFrom(App* app, ThreadState& ts, u32 esp, u32 handle, int32_t bend, u32 ebx, u32 esi, u32 edi)
{
    if (kInited(app) == 0)
        return kNotStarted;
    const u32 frame = esp - 4;
    if (bend < 0 || bend > 0x7f)
    {
        app->getMemory<u32>(esp - 8) = edi;
        app->getMemory<u32>(esp - 0xc) = esi;
        app->getMemory<u32>(esp - 0x10) = ebx;
        if (!fail(app, ts, esp - 0x10, kBend, 0x78, {kBendText}))
            return {};
    }
    if (!enterAudio(app, ts, frame - 4))
        return {};
    const Result r = bendNow(app, ts, frame - 4, handle, bend);
    if (!r || !leaveAudio(app, ts, frame - 4))
        return {};
    return r;
}

/* setVolumeFrom and setBendFrom for the rewrites that call SNDvol and
 * SNDpitchbend as C++ (port_sound_*.cpp, port_snd_stream.cpp): the caller's
 * ebp as it is held (heldEbp), until those pass their own. */
Result setVolume(App* app, ThreadState& ts, u32 esp, u32 handle, int32_t volume)
{
    return setVolumeFrom(app, ts, esp, handle, volume, heldEbp());
}


/* 0x4e9850's (SNDpos3d): the sound of `handle` (its group's voices) to 3D
 * `position` (its low half), `elevation` (kept to -0x4000..0x3fff).  0, -8 for
 * no such sound, -10 before SNDinit. */
Result placeSound(App* app, ThreadState& ts, u32 esp, u32 handle, u32 position, int32_t elevation)
{
    if (kInited(app) == 0)
        return kNotStarted;
    position &= 0xffff;
    if (elevation > 0x3fff)
        elevation = 0x3fff;
    else if (elevation < -0x4000)
        elevation = -0x4000;
    u32 frame = esp - 0xc - 4;
    if (!enterAudio(app, ts, frame - 4))
        return {};
    const int32_t index = voiceOfHandle(app, handle);
    if (index >= 0)
    {
        frame -= 4;
        const Voice& v = voice(app, index);
        for (int32_t j = 0; j < kVoiceCount; ++j)
        {
            const Voice& w = voice(app, j);
            if (v.group == 0 ? &w != &v : !inGroup(w, v.group))
                continue;
            if (!placeVoice(app, ts, frame - 4, j, position, elevation))
                return {};
        }
    }
    if (!leaveAudio(app, ts, frame - 4))
        return {};
    return index < 0 ? kNoSound : 0;
}

/* 0x4e9950's (SNDpan): the sound of `handle` (its group's voices) to `pan`
 * (0..127): spread about the middle by each voice's spread, moved by its
 * patch's pan, through its pan curve.  0, -8 for no such sound, -10 before
 * SNDinit. */
Result panSound(App* app, ThreadState& ts, u32 esp, u32 handle, int32_t pan)
{
    if (kInited(app) == 0)
        return kNotStarted;
    const u32 frame = esp - 0x14 - 8;
    if ((pan > 0x7f || pan < 0) && !fail(app, ts, frame, kSpan, 0x47, {kSpanText, u32(pan)}))
        return {};
    if (!enterAudio(app, ts, frame - 4))
        return {};
    const int32_t index = voiceOfHandle(app, handle);
    if (index >= 0 && voice(app, index).panAsked != pan)
    {
        Voice& v = voice(app, index);
        v.panAsked = int8_t(pan);
        for (int32_t j = 0; j < kVoiceCount; ++j)
        {
            Voice& w = voice(app, j);
            if (v.group == 0 ? &w != &v : !inGroup(w, v.group))
                continue;
            int32_t p = w.spread != 1 ? imul(w.spread, pan - 0x40) + 0x40 : pan;
            p += w.patchPan - 0x40;
            p = p > 0x7f ? 0x7f : p < 0 ? 0 : p;
            if (w.panCurve != 0)
                p = sbyteAt(app, w.panCurve + u32(p));
            w.pan = uint8_t(p);
            if (!panVoice(app, ts, frame - 4, j, p))
                return {};
        }
    }
    if (!leaveAudio(app, ts, frame - 4))
        return {};
    return index < 0 ? kNoSound : 0;
}

/* 0x4e9b20's (SNDplaysetdef): a play's options to their defaults. */
void playDefaults(PlayOptions& o)
{
    o.patch = -1;
    o.bank = -1;
    o.key = 0x3c;
    o.velocity = 0x7f;
    o.pan = 0x40;
    o.volume = 0x7f;
    o.bend = 0x40;
    o.effectLevel = 0x7f;
    o.placed = 0;
    o.position = 0;
    o.elevation = 0;
}

/* 0x4e9bf8's: the patch a play's options name played; -8 for none. */
Result playByOptions(App* app, ThreadState& ts, u32 esp, const PlayOptions& options)
{
    const int32_t bank = options.bank;
    if (bankState(app, bank) < 0)
        return kNoSound;
    const u32 b = kBanks(app, bank).bank;
    const BankHeader& header = record<BankHeader>(app, b);
    if (options.patch < 0 || int32_t(header.patches) <= options.patch)
        return kNoSound;
    const u32 patch = app->getMemory<u32>(b + u32(options.patch) * 4 + (header.version == 4 ? 0x14 : 0xc));
    return playPatch(app, ts, esp - 0xc, patch, options);
}

/* 0x4e9ba0's (SNDplay): playByOptions; -10 before SNDinit. */
Result playSound(App* app, ThreadState& ts, u32 esp, const PlayOptions& options)
{
    if (kInited(app) == 0)
        return kNotStarted;
    return playByOptions(app, ts, esp - 0xc, options);
}

/* 0x501670's: what the sound of `handle` plays at, -8 for none, -10 before
 * SNDinit. */
Result playingLevel(App* app, ThreadState& ts, u32 esp, u32 handle)
{
    if (kInited(app) == 0)
        return kNotStarted;
    const u32 frame = esp - 4;
    if (!enterAudio(app, ts, frame - 4))
        return {};
    const int32_t index = voiceOfHandle(app, handle);
    if (!leaveAudio(app, ts, frame - 4))
        return {};
    return index < 0 ? kNoSound : voice(app, index).mixLevel;
}

/* 0x501938's: a new group's number: one past the last that no playing voice
 * has, never 0. */
uint8_t nextGroup(App* app)
{
    uint8_t group = kGroupSerial(app);
    for (bool taken = true; taken;)
    {
        if (++group == 0)
            ++group;
        taken = false;
        for (int32_t i = 0; i < kVoiceCount && !taken; ++i)
            taken = voice(app, i).state != 0 && voice(app, i).group == group;
    }
    kGroupSerial.set(app, group);
    return group;
}

/* The game's entries: each its thunk -- the game's registers and stack in,
 * the result in eax out -- around what it does. */

nfs::u32 eaRandom(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, nextRandom(app));
}

nfs::u32 randomOffset(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, u32(randomWithin(app, int32_t(e.eax))));
}

nfs::u32 readBigEndian(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, u32(bigEndian(app, e.eax, int32_t(e.edx))));
}

/* The tag reader's four words at eax, edx, ebx and ecx. */
nfs::u32 nextTag(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    Tag t{app->getMemory<u32>(e.eax), app->getMemory<u32>(e.edx), app->getMemory<u32>(e.ebx),
          app->getMemory<u32>(e.ecx)};
    const bool more = readTag(app, t);
    app->getMemory<u32>(e.eax) = t.place;
    app->getMemory<u32>(e.edx) = t.tag;
    app->getMemory<u32>(e.ebx) = t.number;
    app->getMemory<u32>(e.ecx) = t.data;
    return e.back(ts, args, more ? 1 : 0);
}

/* The pans' thunks: the sides into [ebx] and [ecx]; eax nobody reads. */
nfs::u32 panSplit(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    const Sides s = splitPan(int32_t(e.eax), int32_t(e.edx));
    store(app, e.ebx, s.left);
    store(app, e.ecx, s.right);
    return e.back(ts, args, e.eax);
}

nfs::u32 panLaw(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    const Sides s = panAt(app, e.eax, e.edx);
    store(app, e.ebx, s.left);
    store(app, e.ecx, s.right);
    return e.back(ts, args, e.eax);
}

nfs::u32 panLawPositive(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    const Sides s = panAtPositive(app, e.eax, e.edx);
    store(app, e.ebx, s.left);
    store(app, e.ecx, s.right);
    return e.back(ts, args, e.eax);
}

/* stdcall 8: the surround's two levels on the stack. */
nfs::u32 panSurround(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    const Sides s = panSurroundAt(app, e.eax, e.edx, int32_t(e.arg(app, 0)), int32_t(e.arg(app, 1)));
    store(app, e.ebx, s.left);
    store(app, e.ecx, s.right);
    return e.back(ts, args, e.eax, 8);
}

nfs::u32 voiceOf(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, u32(voiceOfHandle(app, e.eax)));
}

nfs::u32 voicePicked(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, isPicked(app, int32_t(e.eax), int32_t(e.edx)) ? 1 : 0);
}

nfs::u32 iSNDfreechan(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, freeVoice(app, ts, e.esp, int32_t(e.eax)));
}

/* The handle into [ecx]. */
nfs::u32 iSNDallocchan(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    u32 handle = app->getMemory<u32>(e.ecx);
    const Result r = allocVoices(app, ts, e.esp, e.eax, int32_t(e.edx), uint8_t(e.ebx), handle);
    app->getMemory<u32>(e.ecx) = handle;
    return e.finish(ts, args, r);
}

nfs::u32 iSNDenteraudio(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, enterAudio(app, ts, e.esp));
}

nfs::u32 iSNDleaveaudio(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, leaveAudio(app, ts, e.esp, e.eax));
}

nfs::u32 iSNDserveradd100hzclient(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, addTickClient(app, ts, e.esp, e.eax));
}

nfs::u32 iSNDserverremove100hzclient(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, removeTickClient(app, ts, e.esp, e.eax));
}

nfs::u32 setSendVolume(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, sendVolume(app, ts, e.esp, e.eax));
}

nfs::u32 setMixVolume(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, channelVolume(app, ts, e.esp, int32_t(e.eax), int32_t(e.edx), int32_t(e.ebx)));
}

nfs::u32 setSendLevel(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, channelSendLevel(app, ts, e.esp, int32_t(e.eax), uint8_t(e.edx)));
}

nfs::u32 setVoiceSend(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, voiceSendLevel(app, ts, e.esp, int32_t(e.eax), int32_t(e.ebx)));
}

nfs::u32 updateVoiceVolume(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, u32(refreshVolume(app, int32_t(e.eax))));
}

nfs::u32 iSNDvol(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, voiceVolume(app, ts, e.esp, int32_t(e.eax), int32_t(e.edx), e.ebp));
}

nfs::u32 updateVoicePitch(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, refreshPitch(app, int32_t(e.eax)));
}

/* The factor onto the x87's stack; eax the power's size. */
nfs::u32 pitchFactor(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    const int32_t cents = int32_t(e.eax);
    args.floatSet = true;
    args.floatResult = pitchScale(app, ts, cents);
    return e.back(ts, args, u32(cents <= 0 ? imul(cents, -1) : cents));
}

nfs::u32 mixStep(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, u32(stepFor(app, ts, int32_t(e.eax), int32_t(e.edx), int32_t(e.ebx))));
}

nfs::u32 setMixStep(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, channelStep(app, ts, e.esp, int32_t(e.eax), e.edx));
}

nfs::u32 iSNDplatformdetune(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, detuneVoice(app, ts, e.esp, int32_t(e.eax), int32_t(e.edx)));
}

nfs::u32 iSNDplatform3dpos(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, placeVoice(app, ts, e.esp, int32_t(e.eax), e.edx, int32_t(e.ebx)));
}

nfs::u32 iSNDpan(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, panVoice(app, ts, e.esp, int32_t(e.eax), int32_t(e.edx)));
}

nfs::u32 iSNDmixstop(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, silenceChannel(app, ts, e.esp, int32_t(e.eax)));
}

nfs::u32 iSNDstop(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, stopVoice(app, ts, e.esp, int32_t(e.eax)));
}

/* 0x4e257c: the ret ending the driver's restore -- the tick calls it for
 * nothing. */
nfs::u32 soundIdle(App*, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, e.eax);
}

nfs::u32 soundTick(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, tickSoundFrom(app, ts, e.esp, e.eax, e.edx, e.ecx, e.ebp));
}

nfs::u32 SNDstop(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, stopSound(app, ts, e.esp, e.eax));
}

nfs::u32 SNDstatus(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, soundStatus(app, ts, e.esp, e.eax));
}

nfs::u32 SNDstopall(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, stopAllSounds(app, ts, e.esp));
}

nfs::u32 SNDautovol(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, fadeSound(app, ts, e.esp, e.eax, int32_t(e.edx), int32_t(e.ebx)));
}

nfs::u32 setSoundVolume(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, volumeNow(app, ts, e.esp, e.eax, int32_t(e.edx), e.ebp));
}

nfs::u32 SNDvol(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, setVolumeFrom(app, ts, e.esp, e.eax, int32_t(e.edx), e.ebp));
}

nfs::u32 setSoundBend(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, bendNow(app, ts, e.esp, e.eax, int32_t(e.edx)));
}

nfs::u32 SNDpitchbend(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, setBendFrom(app, ts, e.esp, e.eax, int32_t(e.edx), e.ebx, e.esi, e.edi));
}

nfs::u32 SNDpos3d(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, placeSound(app, ts, e.esp, e.eax, e.edx, int32_t(e.ebx)));
}

nfs::u32 SNDpan(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, panSound(app, ts, e.esp, e.eax, int32_t(e.edx)));
}

/* The options at eax -- an error for none, and the defaults written there
 * all the same. */
nfs::u32 SNDplaysetdef(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    if (e.eax == 0 && !fail(app, ts, e.esp - 0x10, kSetdef, 0x37, {kSetdefText}))
        return args.eax;
    playDefaults(record<PlayOptions>(app, e.eax));
    return e.back(ts, args, 0);
}

nfs::u32 playOptions(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, playByOptions(app, ts, e.esp, record<PlayOptions>(app, e.eax)));
}

/* The options at eax; an error for none, after SNDinit's check, and played
 * from there all the same. */
nfs::u32 SNDplay(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    if (kInited(app) != 0 && e.eax == 0 && !fail(app, ts, e.esp - 0x10, kPlay, 0x9f, {kPlayText}))
        return args.eax;
    return e.finish(ts, args, playSound(app, ts, e.esp, record<PlayOptions>(app, e.eax)));
}

nfs::u32 soundVolume(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, playingLevel(app, ts, e.esp, e.eax));
}

nfs::u32 newGroup(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, nextGroup(app));
}

}  // namespace port
}  // namespace nfs3hp
