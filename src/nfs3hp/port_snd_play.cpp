#include "port_snd.h"

#ifdef __ANDROID__
#include <SDL3/SDL.h>
#endif

#include <cstdio>
#include <cstdlib>

/* EA's SND library, rewritten (port_snd.h says how): what a sound is played
 * from.  A bank (SNDbankadd) holds patches; a patch is a stream of tags --
 * each a byte, its data's length and up to 4 bytes of it -- that set up
 * timbres (0xfe ends one), each played on voices of its own when the play's
 * key and velocity are in its ranges; a timbre's sample (0xfd starts its
 * platform part) is played on a mixer's channel by playSample and
 * startChannel.  When a bank is added, its patches' offsets become addresses
 * and each sample's place is written into its tags (resolvePatch).
 *
 * Where the game's code reads a local it set on no path -- after an error
 * that came back -- the rewrite reads it where the code has it on the stack,
 * as the code finds it. */
namespace nfs3hp
{
namespace port
{

namespace
{

constexpr Where kTimbreError{0x54e638, 0x54e648}, kPatchError{0x54e638, 0x54e730};
constexpr Where kResolveError{0x54e638, 0x54e7ac}, kInfoError{0x54e890, 0x54e8a0};
constexpr Where kRemovePatError{0x54e910, 0x54e920}, kBankAllocError{0x54e960, 0x54e970};
constexpr Where kDownloadError{0x54e9b4, 0x54e9c4}, kPlayPlatformError{0x55194c, 0x55195c};
constexpr Where kPlatformResolveError{0x551a10, 0x551a20}, kMixStartError{0x54e184, 0x54e1cc};
constexpr Where kPacketError{0x551bb0, 0x551bc0}, kCheckError{0x551c50, 0x551c60};
constexpr Where kRemoveError{0x54b10c, 0x54b11c}, kAddError{0x54b198, 0x54b1a4};
constexpr Where kHeaderSizeError{0x54b1d0, 0x54b1e0}, kHeaderCopyError{0x54b228, 0x54b238};
constexpr Where kPatPresentError{0x54dc24, 0x54dc34}, kMasterError{0x54ab28, 0x54ab38};

constexpr uint16_t kPatchId = 0x5450;   // 'PT'
constexpr reg32 kBankId = 0x6c4b4e42;   // 'BNKl'
constexpr reg32 kDefaultEnvelope = 0x5678a8;
// The mixer's functions for each format, which iSNDmixinit set: the decoders'
// inits by format, compression and way of playing; the volume functions'
// inits by format; the resamplers' inits, plain and better.
constexpr Table<reg32> kDecoderInits{0x9f6a6c};
constexpr Table<reg32> kVolumeInits{0x9f6adc};
constexpr Table<reg32> kResamplerInits{0x9f6aec}, kBetterResamplerInits{0x9f6afc};

uint8_t byteAt(App* app, reg32 at) { return app->getMemory<uint8_t>(at); }
reg32 wordAt(App* app, reg32 at) { return app->getMemory<reg32>(at); }

/* A patch's tags: past its 4-byte header, or 8 when bit 1 of byte 3 says. */
reg32 patchTags(App* app, reg32 patch) { return patch + ((byteAt(app, patch + 3) & 2) ? 8 : 4); }

/* A tag reader whose words are on the game's stack at `at`, as the code
 * finds them there, reading from `place`. */
Tag readerAt(App* app, reg32 at, reg32 place)
{
    Tag t = load<Tag>(app, at);
    t.place = place;
    return t;
}

/* A bank's patch table's entry `index`: at +0x14 for version 4, +0xc else. */
reg32 patchSlot(App* app, reg32 bank, int32_t index)
{
    return bank + reg32(index) * 4 + (record<BankHeader>(app, bank).version == 4 ? 0x14 : 0xc);
}

/* A sample's frame: 4 for 16 bits mono, 8 stereo, 1 and 2 for 8 bits; 0
 * for another. */
reg32 frameFormat(reg32 bits, reg32 channels)
{
    if (bits == 0x10 && channels == 1)
        return 4;
    if (bits == 0x10 && channels == 2)
        return 8;
    if (bits == 8 && channels == 1)
        return 1;
    if (bits == 8 && channels == 2)
        return 2;
    return 0;
}

/* A patch with more timbres in its play than the voices are: the code keeps
 * their handles in 16 words of its frame and runs over its own reader past
 * them.  No sound of the game's has that many; the game stops, saying so. */
[[noreturn]] void tooManyTimbres(reg32 patch)
{
#ifdef __ANDROID__
    SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "[SND] patch %x plays more than 16 timbres (port_snd_play.cpp)",
                    patch);
#else
    std::fprintf(stderr, "[SND] patch %x plays more than 16 timbres (port_snd_play.cpp)\n", patch);
#endif
    std::abort();
}

}  // namespace

/* 0x5017c0's: a timbre's settings that last the patch: its sample played once,
 * the play's pitch not counted, 0xf. */
void timbrePatchDefaults(Timbre& t)
{
    t.mode = 1;
    t.playPitch = 0;
    t.timbre6c = 0xf;
}

/* 0x5017d8's: a timbre's settings back to their defaults, its data's places
 * cleared. */
void clearTimbre(Timbre& t, Places& places)
{
    places.fill(0);
    t.lowVelocity = 0;
    t.highVelocity = 0x7f;
    t.lowKey = 0;
    t.highKey = 0x7f;
    t.voiceMask = 0xffffffff;
    t.priority = 0;
    t.rootKey = 0x3c;
    t.timbre20 = -1;
    t.segments = 1;
    t.bendRange = 0;
    t.voices = 1;
    t.pan = 0x40;
    t.panRandom = 0;
    t.volume = 0x7f;
    t.volumeRandom = 0;
    t.pitch = 0;
    t.pitchRandom = 0;
    t.volumeCurve = 0;
    t.send = 0;
    t.panCurve = 0;
    t.bendCurve = 0;
    t.envelopeStart = 0x7f;
    t.volumeLfo = 0;
    t.volumeLfoLength = 0;
    t.volumeLfoRandom = 0;
    t.pitchLfo = 0;
    t.pitchLfoLength = 0;
    t.pitchLfoRandom = 0;
    t.pitchLfoDepth = 0;
    t.spread = 1;
    t.envelope = kDefaultEnvelope;
}

/* 0x5018dc's: the data's places added to the timbre's curves, LFOs and
 * envelope (when it has one): their numbers are offsets from their data. */
void addDataPlaces(Timbre& t, const Places& places)
{
    t.volumeCurve += places[0x12];
    t.panCurve += places[0x15];
    t.bendCurve += places[0x17];
    t.volumeLfo += places[0x1d];
    t.pitchLfo += places[0x20];
    if (t.envelope != 0)
        t.envelope += places[0x19];
}

/* 0x501978's (iSNDplaytaggedtimbre): `timbre` played as `options` ask (their
 * pan, volume, bend, effect level), from the tags of its sample at `sample`,
 * its data's places, at `key` and `velocity`, `pitchOffset` added: voices
 * taken, set up from the timbre and the sample started on them.  The handle,
 * or what failed. */
Result playTimbre(App* app, CPU& cpu, reg32 esp, Timbre& t, reg32 sample, const PlayOptions& options,
                  const Places& places, int32_t key, int32_t velocity, int32_t pitchOffset)
{
    const reg32 frame = esp - 0xc - 0x1c;
    int32_t pan = options.pan;
    const int32_t volume = options.volume, bend = options.bend, effect = options.effectLevel;
    addDataPlaces(t, places);
    if ((volume < 0 || volume > 0x7f) && !fail(app, cpu, frame, kTimbreError, 0x9e, {0x54e660, reg32(volume)}))
        return {};
    if ((pan < 0 || pan > 0x7f) && !fail(app, cpu, frame, kTimbreError, 0xa6, {0x54e694, reg32(pan)}))
        return {};
    if ((effect < 0 || effect > 0x7f) && !fail(app, cpu, frame, kTimbreError, 0xad, {0x54e6c4, reg32(effect)}))
        return {};
    int32_t patchPan = t.pan;
    if (t.panRandom != 0)
        patchPan += randomWithin(app, t.panRandom);
    patchPan = patchPan > 0x7f ? 0x7f : patchPan < 0 ? 0 : patchPan;
    if ((bend < 0 || bend > 0x7f) && !fail(app, cpu, frame, kTimbreError, 0xc3, {0x54e6f8, reg32(bend)}))
        return {};
    reg32 handle = wordAt(app, frame);
    const Result taken = allocVoices(app, cpu, frame - 4, t.voiceMask, t.voices, uint8_t(t.priority), handle);
    if (!taken || *taken < 0)
        return taken;
    const int32_t index = *taken;
    Voice& v = kVoices(app, index);
    v.spread = int8_t(t.spread);
    v.pitch = int16_t(t.pitch);
    v.pitch = int16_t(uint16_t(v.pitch) - uint16_t((reg32(uint16_t(t.rootKey)) - reg32(key)) * 0x64));
    if (t.playPitch != 0)
        v.pitch = int16_t(uint16_t(v.pitch) + uint16_t(pitchOffset));
    if (t.pitchRandom != 0)
        v.pitch = int16_t(uint16_t(v.pitch) + uint16_t(randomWithin(app, t.pitchRandom)));
    v.volumeCurve = t.volumeCurve;
    v.panCurve = t.panCurve;
    v.bendCurve = t.bendCurve;
    v.mode = uint8_t(t.mode);
    v.volume = int32_t(reg32(volume) << 16);
    v.panAsked = int8_t(pan);
    v.volumeStep = 0;
    if (v.spread != 1)
        pan = imul(v.spread, pan - 0x40) + 0x40;
    pan = patchPan + pan - 0x40;
    pan = pan < 0 ? 0 : pan > 0x7f ? 0x7f : pan;
    if (v.panCurve != 0)
        pan = app->getMemory<int8_t>(v.panCurve + reg32(pan));
    int32_t level = t.volume;
    if (t.volumeRandom != 0)
        level += randomWithin(app, t.volumeRandom);
    level = level > 0x7f ? 0x7f : level < -0x80 ? -0x80 : level;
    v.level = int8_t(idiv(imul(level, velocity), 0x7f));
    v.patchPan = int8_t(patchPan);
    v.pan = uint8_t(pan);
    v.bendRange = int16_t(uint16_t(t.bendRange) * 0x64);
    v.bend = uint8_t(bend);
    v.envelopeSegments = t.envelope;
    v.segment = 0;
    v.envelope = int32_t(reg32(t.envelopeStart) << 16);
    v.segments = int8_t(t.segments);
    v.timbre20 = uint8_t(t.timbre20);
    const int32_t ticks = app->getMemory<int32_t>(v.envelopeSegments);
    v.envelopeTicks = ticks < 0 ? 0x7fffffff : ticks;
    v.envelopeStep = idiv(int32_t((wordAt(app, v.envelopeSegments + 4) << 16) - reg32(v.envelope)), v.envelopeTicks);
    v.volumeLfo = t.volumeLfo;
    v.pitchLfo = t.pitchLfo;
    v.volumeLfoLength = uint8_t(t.volumeLfoLength);
    v.pitchLfoLength = uint8_t(t.pitchLfoLength);
    v.pitchLfoDepth = int16_t(t.pitchLfoDepth);
    v.volumeLfoPlace = 0;
    if (t.volumeLfoRandom != 0)
        v.volumeLfoPlace = uint8_t(nextRandom(app) % reg32(t.volumeLfoLength));
    v.pitchLfoPlace = 0;
    if (t.pitchLfoRandom != 0)
        v.pitchLfoPlace = uint8_t(nextRandom(app) % reg32(t.pitchLfoLength));
    refreshPitch(app, index);
    v.timbre6c = uint8_t(t.timbre6c);
    v.timbreSend = int8_t(t.send);
    v.send = int8_t(effect);
    v.bank = uint8_t(options.bank);
    v.patch = uint16_t(options.patch);
    refreshVolume(app, index);
    SamplePlay play{v.mixLevel, v.playedPitch, idiv(imul(v.send, imul(v.timbreSend, kBusLevel(app))), 0x3f01),
                    options.position, options.elevation, options.placed == 0 ? 1 : 0};
    const Result r = playSample(app, cpu, frame - 0x18 - 4, sample, t.mode, index, pan, play);
    if (!r)
        return {};
    if (*r < 0)
    {
        if (!freeVoice(app, cpu, frame - 4, index))
            return {};
        return r;
    }
    return int32_t(handle);
}

/* 0x501de8's (iSNDplaytaggedpatch): `patch` played as `options` ask: its
 * timbres read from its tags, each played whose ranges hold the play's key
 * and velocity; several voices made a group, ended by the last.  The last
 * handle, or what failed (each timbre played stopped again).  A timbre before
 * any sample plays from where the code never set it: ebp. */
Result playPatch(App* app, CPU& cpu, reg32 esp, reg32 patch, const PlayOptions& options)
{
    const reg32 f = esp - 0x14 - 0x1a4;
    if (patch == 0)
        return kNoSound;
    const int32_t velocity = options.velocity, key = options.key;
    if ((velocity < 0 || velocity > 0x7f) && !fail(app, cpu, f, kPatchError, 0x1a8, {0x54e744, reg32(velocity)}))
        return {};
    if ((key < 0 || key > 0x7f) && !fail(app, cpu, f, kPatchError, 0x1b0, {0x54e778, reg32(key)}))
        return {};
    // The code's frame: the timbre, its places, the handles, the reader.
    Timbre timbre = load<Timbre>(app, f);
    Places places = load<Places>(app, f + 0x98);
    std::array<reg32, 16> handles;
    Tag tag = readerAt(app, f + 0x170, patchTags(app, patch));
    timbrePatchDefaults(timbre);
    clearTimbre(timbre, places);
    if (!enterAudio(app, cpu, f - 4))
        return {};
    const uint8_t group = nextGroup(app);
    int32_t result = kNoVoices, count = 0, pitchOffset = 0;
    reg32 sample = cpu.ebp;
    bool failed = false;
    const auto inRange = [&] {
        return velocity >= timbre.lowVelocity && velocity <= timbre.highVelocity && key >= timbre.lowKey
            && key <= timbre.highKey;
    };
    // A timbre played: its handle kept; false when the game stopped.
    const auto play = [&] {
        const Result r = playTimbre(app, cpu, f - 0x10 - 4, timbre, sample, options, places, key, velocity,
                                    pitchOffset);
        if (!r)
            return false;
        result = *r;
        if (result < 0)
            failed = true;
        else if (count < int32_t(handles.size()))
            handles[count] = reg32(result);
        else
            tooManyTimbres(patch);
        return true;
    };
    for (;;)
    {
        const bool more = readTag(app, tag);
        if (!more)
            break;
        if (tag.tag == 0xfd)
            sample = tag.place;
        else if (tag.tag == 0xfe)
        {
            if (inRange())
            {
                if (!play())
                    return {};
                if (failed)
                    break;
                ++count;
            }
            clearTimbre(timbre, places);
        }
        else if (int32_t(tag.tag) < 0x26)
        {
            timbre.set(tag.tag, int32_t(tag.number));
            places[tag.tag] = tag.data;
            if (tag.tag == 0x24)
                pitchOffset = randomWithin(app, int32_t(tag.number));
        }
    }
    if (!failed && inRange())
    {
        if (!play())
            return {};
        if (!failed)
            ++count;
    }
    if (failed || count == 0)
    {
        if (!leaveAudio(app, cpu, f - 4))
            return {};
        for (int32_t i = 0; i < count; ++i)
            if (!stopSound(app, cpu, f - 4, handles[i]))
                return {};
        return result;
    }
    if (count == 1)
    {
        const int32_t index = voiceOfHandle(app, handles[0]);
        if (index >= 0)
        {
            kVoices(app, index).group = 0;
            kVoices(app, index).endsGroup = 0;
        }
    }
    else
    {
        // The last of them ends the group -- with none, the byte past 0.
        reg32 last = 0;
        for (int32_t i = 0; i < count; ++i)
        {
            const int32_t index = voiceOfHandle(app, handles[i]);
            if (index < 0)
                continue;
            kVoices(app, index).endsGroup = 0;
            last = kVoices.address(index);
            kVoices(app, index).group = group;
        }
        record<Voice>(app, last).endsGroup = 1;
    }
    if (!leaveAudio(app, cpu, f - 4))
        return {};
    return result;
}

/* 0x5021ac's (iSNDresolvetaggedpatch): `patch`'s samples made addresses in the
 * bank at `base`, each 0xfd's tags through resolveSample; the patch marked
 * resolved.  The last one's result, -1 for none. */
Result resolvePatch(App* app, CPU& cpu, reg32 esp, reg32 patch, reg32 base)
{
    const reg32 f = esp - 0x10 - 0x14;
    int32_t result = -1;
    if (app->getMemory<uint16_t>(patch) != kPatchId && !fail(app, cpu, f, kResolveError, 0x254, {0x54e7c4}))
        return {};
    if (byteAt(app, patch + 2) != 0 && !fail(app, cpu, f, kResolveError, 0x267, {0x54e7f4}))
        return {};
    if ((byteAt(app, patch + 3) & 1) && !fail(app, cpu, f, kResolveError, 0x26f, {0x54e82c}))
        return {};
    Tag tag = readerAt(app, f, patchTags(app, patch));
    while (readTag(app, tag))
        if (tag.tag == 0xfd)
        {
            const Result r = resolveSample(app, cpu, f - 4, tag.place, base);
            if (!r)
                return {};
            result = *r;
        }
    if (result >= 0)
        app->getMemory<uint8_t>(patch + 3) |= 1;
    return result;
}

/* 0x5022d0's: `patch`'s samples let go of -- eacsnd's 3D buffers among them
 * -- and the patch no longer resolved.  0. */
Result unresolvePatch(App* app, CPU& cpu, reg32 esp, reg32 patch)
{
    const reg32 f = esp - 0x10 - 0x10;
    Tag tag = readerAt(app, f, patchTags(app, patch));
    while (readTag(app, tag))
        if (tag.tag == 0xfd && !unresolveSample(app, cpu, f - 4, tag.place))
            return {};
    app->getMemory<uint8_t>(patch + 3) &= 0xfe;
    return 0;
}

/* 0x502330's (iSNDextracthdrinfo): what `patch`'s tags say of its sample,
 * into `info`: +0 tag 0x13, +1 0x1b (0xf), +2 bits, +3 compression, +4 the
 * rate (22050), +6 channels, +8 its length.  0. */
Result sampleInfo(App* app, CPU& cpu, reg32 esp, reg32 patch, reg32 info)
{
    const reg32 f = esp - 0x10 - 0x10;
    if (patch == 0 && !fail(app, cpu, f - 4, kInfoError, 0x11, {0x54e8b4}))
        return {};
    if (app->getMemory<uint16_t>(patch) != kPatchId && !fail(app, cpu, f, kInfoError, 0x17, {0x54e8e4}))
        return {};
    Tag tag = readerAt(app, f, patchTags(app, patch));
    app->getMemory<reg32>(info + 8) = 0;
    app->getMemory<uint8_t>(info) = 0;
    app->getMemory<uint8_t>(info + 1) = 0xf;
    app->getMemory<uint8_t>(info + 2) = 0;
    app->getMemory<uint8_t>(info + 3) = 0;
    app->getMemory<uint8_t>(info + 6) = 0;
    app->getMemory<uint16_t>(info + 4) = 0x5622;
    while (readTag(app, tag))
        switch (tag.tag)
        {
        case 0x13: app->getMemory<uint8_t>(info) = uint8_t(tag.number); break;
        case 0x1b: app->getMemory<uint8_t>(info + 1) = uint8_t(tag.number); break;
        case 0x81: app->getMemory<uint8_t>(info + 2) = uint8_t(tag.number); break;
        case 0x82: app->getMemory<uint8_t>(info + 6) = uint8_t(tag.number); break;
        case 0x84: app->getMemory<uint16_t>(info + 4) = uint16_t(tag.number); break;
        case 0x83: app->getMemory<uint8_t>(info + 3) = uint8_t(tag.number); break;
        case 0x85: app->getMemory<reg32>(info + 8) = tag.number; break;
        default: break;
        }
    return 0;
}

/* 0x502570's: whether bank `bank` is there to use: 0; -8 for none, -18 while
 * it is loaded. */
int32_t bankState(App* app, int32_t bank)
{
    if (bank < 0 || bank >= kBankCount || kBanks(app, bank).bank == 0)
        return kNoSound;
    return kBanks(app, bank).loading != 0 ? -18 : 0;
}

/* 0x5024a0's (iSNDbankremovepat): patch `index` of bank `bank` let go of and
 * gone from the bank.  0, -8 for none. */
Result removePatch(App* app, CPU& cpu, reg32 esp, int32_t bank, int32_t index)
{
    const reg32 frame = esp - 0x10;
    const reg32 b = kBanks(app, bank).bank;
    if (bankState(app, bank) != 0 || index < 0 || index >= int32_t(record<BankHeader>(app, b).patches))
        return kNoSound;
    const reg32 patch = wordAt(app, patchSlot(app, b, index));
    if (patch == 0)
        return kNoSound;
    if (app->getMemory<uint16_t>(patch) != kPatchId && !fail(app, cpu, frame, kRemovePatError, 0x25, {0x54e934}))
        return {};
    if (!unresolvePatch(app, cpu, frame - 4, patch))
        return {};
    app->getMemory<reg32>(patchSlot(app, b, index)) = 0;
    return 0;
}

/* 0x5025b0's (iSNDbankalloc): the first free bank handle; -9 for none. */
Result freeBankHandle(App* app, CPU& cpu, reg32 esp)
{
    for (int32_t i = 0; i < kBankCount; ++i)
        if (kBanks(app, i).bank == 0)
            return i;
    if (!fail(app, cpu, esp - 0x14, kBankAllocError, 0x10, {0x54e980}))
        return {};
    return kNoVoices;
}

/* 0x502620's (iSNDdownloadbank): `bank`'s patches made addresses (their
 * offsets from the bank's table) and resolved against `base`.  7, or 8 when
 * a patch was resolved (the code takes a resolved one's 8 for a failure). */
Result downloadBank(App* app, CPU& cpu, reg32 esp, reg32 bank, reg32 base)
{
    const reg32 f = esp - 0x14 - 0xc10;
    int32_t result = 7;
    const BankHeader& header = record<BankHeader>(app, bank);
    if (header.version != 4 && header.version != 2
        && !fail(app, cpu, f, kDownloadError, 0x1b, {0x54e9d8, header.version}))
        return {};
    for (int32_t i = 0; i < int32_t(header.patches); ++i)
    {
        const reg32 slot = patchSlot(app, bank, i);
        const reg32 offset = wordAt(app, slot);
        if (offset == 0)
            continue;
        const reg32 patch = slot + offset;
        app->getMemory<reg32>(slot) = patch;
        const Result r = resolvePatch(app, cpu, f - 4, patch, base);
        if (!r)
            return {};
        if (*r != 7)
            result = 8;
    }
    return result;
}

/* 0x522a80's (iSNDplatformresolve): a sample's tags at `tags` read; its place
 * in the bank at `base` made an address where tag 0x8a's data is; a 3D
 * sample's buffer made by eacsnd.  8. */
Result resolveSample(App* app, CPU& cpu, reg32 esp, reg32 tags, reg32 base)
{
    const reg32 f = esp - 0x10 - 0x3c;
    reg32 bits = 0x10, compression = 0, buffer = 0, offset = 0, samplePlace = 0, method = 0, length = 0;
    reg32 rate = 0x5622, channels = 1, loopStart = 0xffffffff, loopEnd = 0xffffffff;
    reg32 methodPlace = wordAt(app, f + 0x18);
    // The format where the code keeps it: past an error that came back, the
    // last sample's that was resolved there.
    reg32& format = record<reg32>(app, f + 0x14);
    Tag tag = readerAt(app, f, tags);
    while (readTag(app, tag))
    {
        if (tag.tag == 0x88)
            offset = tag.number;
        else if (tag.tag == 0x8a)
            samplePlace = tag.data;
        else if (tag.tag == 0x93)
            buffer = tag.data;
        else if (tag.tag == 0x85)
            length = tag.number;
        else if (tag.tag == 0x81)
            bits = tag.number;
        else if (tag.tag == 0x82)
            channels = tag.number;
        else if (tag.tag == 0x84)
            rate = tag.number;
        else if (tag.tag == 0x83)
            compression = tag.number;
        else if (tag.tag == 0x86)
            loopStart = tag.number;
        else if (tag.tag == 0x87)
            loopEnd = tag.number;
        else if (tag.tag == 0x92)
        {
            method = tag.number;
            methodPlace = tag.data;
        }
        else if (tag.tag == 0xfe)
            break;
    }
    app->getMemory<reg32>(samplePlace) = base + offset;
    if (method != 2)
        return 8;
    if (buffer == 0 && !fail(app, cpu, f, kPlatformResolveError, 0x4e, {0x551a34}))
        return {};
    if (compression == 7)
        format = 3;
    else if (compression == 9)
        format = 4;
    else if (compression == 0)
    {
        if (bits == 0x10)
            format = 0;
        else if (bits == 8)
            format = 2;
        else if (!fail(app, cpu, f, kPlatformResolveError, 0x65, {0x551a90, bits}))
            return {};
    }
    else if (!fail(app, cpu, f, kPlatformResolveError, 0x6d, {0x551ac4, compression}))
        return {};
    reg32 made;
    if (!callGame(app, cpu, f, kDirectCreate3dBuf(app), {length, rate, channels, format},
                  {channels, format, rate, length, loopStart, loopEnd, wordAt(app, samplePlace)}, made))
        return {};
    app->getMemory<reg32>(buffer) = made;
    if (made != 0)
        app->getMemory<uint8_t>(methodPlace) = 3;
    return 8;
}

/* 0x522d08's: a sample's tags at `tags`: its 3D buffer, if eacsnd made one,
 * let go of.  0, or eacsnd's answer.  With no such tags the code takes the
 * buffer and its place from where it never set them: esi and ebp. */
Result unresolveSample(App* app, CPU& cpu, reg32 esp, reg32 tags)
{
    const reg32 f = esp - 0x14 - 0x10;
    reg32 buffer = cpu.esi, method = 0, place = cpu.ebp;
    Tag tag = readerAt(app, f, tags);
    bool more;
    while ((more = readTag(app, tag)))
    {
        if (tag.tag == 0x93)
            buffer = tag.data;
        else if (tag.tag == 0x92)
        {
            method = tag.number;
            place = tag.place;
        }
        else if (tag.tag == 0xfe)
            break;
    }
    if (method != 3)
        return 0;
    app->getMemory<uint8_t>(place) = 2;
    // eax the reader's last answer, which eacsnd's answer is when it gives none.
    reg32 eax = more ? 1 : 0;
    if (!callGame(app, cpu, f, kDirectRemoveBuf(app), {eax}, {wordAt(app, buffer)}, eax))
        return {};
    return int32_t(eax);
}

/* 0x5000b8's (iSNDmixstart): channel `index` started on a sample: `format` its
 * bytes a frame (1, 2, 4 or 8), `compression` (0, 7 XA mono, 9 XA stereo),
 * `mode` how it plays (0 a stream, 1 once or looped, 2 in packets), and
 * `start` the rest.  Each of the channel's functions picked from the mixer's
 * tables for the format, its state set up.  0.  Past an error that came back,
 * the code takes the decoder and the resampler from where it never set them:
 * edi and ebp, and the volume functions from its stack. */
Result startChannel(App* app, CPU& cpu, reg32 esp, int32_t index, reg32 format, reg32 compression, reg32 mode,
                    ChannelStart& start)
{
    const reg32 f = esp - 0xc - 8;
    const reg32 c = kChannels.address(index);
    const bool better = start.priority > 0x19;
    const Table<reg32>& resamplers = better ? kBetterResamplerInits : kResamplerInits;
    reg32 decoder = cpu.edi, resampler = cpu.ebp, volumes = wordAt(app, f + 4);
    if (format & 4)
    {
        volumes = kVolumeInits(app, 2);
        decoder = 8;
        resampler = resamplers(app, 2);
    }
    else if (format & 8)
    {
        decoder = 0xc;
        volumes = kVolumeInits(app, 3);
        resampler = resamplers(app, 3);
    }
    else if (format & 1)
    {
        volumes = kVolumeInits(app, 0);
        decoder = 0;
        resampler = resamplers(app, 0);
    }
    else if (format & 2)
    {
        decoder = 4;
        volumes = kVolumeInits(app, 1);
        resampler = resamplers(app, 1);
    }
    else if (!fail(app, cpu, f, kMixStartError, 0x105, {0x54e1dc}))
        return {};
    if (compression == 7)
    {
        if (format & 4)
            decoder = 0x10;
        else if (format & 8)
            decoder = 0x14;
        else if (!fail(app, cpu, f, kMixStartError, 0x11d, {0x54e208}))
            return {};
    }
    else if (compression == 9)
    {
        if (format & 4)
            decoder = 0x18;
        else if (!fail(app, cpu, f, kMixStartError, 0x12b, {0x54e208}))
            return {};
    }
    else if (compression != 0 && !fail(app, cpu, f, kMixStartError, 0x142, {0x54e22c}))
        return {};
    if (mode == 1)
    {
        if (start.loopEnd > 0)
            decoder += 1;
    }
    else if (mode == 2)
        decoder += 2;
    else if (mode == 0)
        decoder += 3;
    else if (!fail(app, cpu, f, kMixStartError, 0x15a, {0x54e25c}))
        return {};
    const reg32 init = kDecoderInits(app, int32_t(decoder));
    if (init == 0)
    {
        if (!fail(app, cpu, f, kMixStartError, 0x178, {0x54e284}))
            return {};
    }
    else if (!callGame(app, cpu, f, init, {reg32(index), c + kChannelDecoder, start.sample, reg32(start.length)},
                       {reg32(start.loopStart), reg32(start.loopEnd), c + kChannelDa8, c + kChannelDac}))
        return {};
    if (volumes == 0)
    {
        if (!fail(app, cpu, f, kMixStartError, 0x18a, {0x54e2a8}))
            return {};
    }
    else if (!callGame(app, cpu, f, volumes, {}, {c + kChannelVolumes, c + kChannelDc0, c + kChannelSetVolume})
             || !callGame(app, cpu, f, volumes, {}, {c + kChannelSend, c + kChannelDe0, c + kChannelSetSend}))
        return {};
    if (resampler == 0)
    {
        if (!fail(app, cpu, f, kMixStartError, 0x197, {0x54e2d4}))
            return {};
    }
    else if (!callGame(app, cpu, f, resampler, {},
                       {c + kChannelResampler, c + kChannelSetStep, c + kChannelDb4, c + kChannelDb8}))
        return {};
    if (start.step > 0x80000)
        start.step = 0x80000;
    if (!callGame(app, cpu, f, wordAt(app, c + kChannelSetStep), {}, {c + kChannelResampler, start.step}))
        return {};
    Channel& ch = kChannels(app, index);
    ch.left = int8_t(start.left);
    ch.right = int8_t(start.right);
    ch.leftTarget = int8_t(start.left);
    ch.rightTarget = int8_t(start.right);
    ch.sendLevel = int8_t(start.send);
    if (!sendVolume(app, cpu, f - 4, c)
        || !callGame(app, cpu, f, wordAt(app, c + kChannelSetVolume), {},
                     {c + kChannelVolumes, reg32(start.left), reg32(start.right)}))
        return {};
    ch.playing = 1;
    return 0;
}

/* 0x522510's (iSNDplatformplay): the sample whose tags are at `tags` played
 * on voice `index`'s channel, as `mode` (2: as a stream) and `pan` say, and
 * `play` the rest -- its position turned to the pan's in surround or 3D.  On
 * eacsnd's 3D buffer for a 3D sample, else on the mixer: startChannel's
 * result.  Past an error that came back, the sides and the frame's size are
 * what the code finds on its stack. */
Result playSample(App* app, CPU& cpu, reg32 esp, reg32 tags, int32_t mode, int32_t index, int32_t pan,
                  SamplePlay& play)
{
    const reg32 f = esp - 0xc - 0x5c;
    reg32 loopStart = 0xffffffff, loopEnd = 0xffffffff, priority = 0, samplePlace = 0, bufferPlace = 0;
    reg32 length = 0, rate = 0x5622, channels = 1, compression = 0, method = 0, bits = 0x10, looped = 0;
    Sides s{int32_t(wordAt(app, f + 0x14)), int32_t(wordAt(app, f + 0x1c))};
    reg32 frameBytes = wordAt(app, f + 0x28);
    PlatformVoice& p = kPlatform(app, index);
    p.panSet = uint8_t(play.byPan);
    p.position = uint16_t(play.position);
    Tag tag{tags, wordAt(app, f + 0x58), wordAt(app, f + 0xc), wordAt(app, f + 0x10)};
    while (readTag(app, tag))
    {
        if (tag.tag == 0x8a)
            samplePlace = tag.data;
        else if (tag.tag == 0x93)
            bufferPlace = tag.data;
        else if (tag.tag == 0x85)
            length = tag.number;
        else if (tag.tag == 0x81)
            bits = tag.number;
        else if (tag.tag == 0x82)
            channels = tag.number;
        else if (tag.tag == 0x84)
            rate = tag.number;
        else if (tag.tag == 0x83)
            compression = tag.number;
        else if (tag.tag == 0x86)
            loopStart = tag.number;
        else if (tag.tag == 0x87)
        {
            loopEnd = tag.number;
            looped = 1;
        }
        else if (tag.tag == 0x91)
            priority = tag.number;
        else if (tag.tag == 0x92)
            method = tag.number;
        else if (tag.tag == 0xfe)
            break;
    }
    if (mode == 2)
    {
        loopStart = 0xffffffff;
        length = 0;
        loopEnd = 0xffffffff;
    }
    else
    {
        if (int32_t(loopStart) < -1)
            loopStart = 0xffffffff;
        if (int32_t(loopEnd) < -1)
            loopEnd = 0xffffffff;
    }
    p.rate = uint16_t(rate);
    p.bits = uint8_t(bits);
    p.channels = uint8_t(channels);
    p.compression = uint8_t(compression);
    p.id = length;
    bool mixer = true;
    if (method == 1)
    {
        if (channels != 1 && !fail(app, cpu, f, kPlayPlatformError, 0x7c, {0x551970}))
            return {};
        if (play.byPan != 0)
            play.position = reg32(pan - 0x40) << 8;
        s = panSurroundAt(app, play.position, reg32(play.volume), p.surroundLeft, p.surroundRight);
        p.surroundLeft = int8_t(s.left);
        p.surroundRight = int8_t(s.right);
        mixer = false;
    }
    else if (method == 3)
    {
        if (play.byPan != 0)
        {
            play.position = reg32(pan - 0x40) << 8;
            play.elevation = 0;
        }
        const x86::Float scaled = fp::mul(pitchScale(app, cpu, play.pitch), x86::Float(int32_t(p.rate)));
        const int64_t frames = chopped<int64_t>(cpu, scaled);
        // eax as the pitch's power leaves it: its size.
        reg32 eax = reg32(play.pitch <= 0 ? imul(play.pitch, -1) : play.pitch);
        if (!callGame(app, cpu, f, kDirectPlay3d(app), {eax},
                      {wordAt(app, bufferPlace), reg32(index), looped, reg32(frames), reg32(play.volume), play.position,
                       reg32(play.elevation)},
                      eax))
            return {};
        if (int32_t(eax) >= 0)
        {
            p.method = uint8_t(method);
            return 0;
        }
        method = 0;
    }
    else if (method != 0 && method != 2)
    {
        if (!fail(app, cpu, f, kPlayPlatformError, 0xad, {0x5519ac}))
            return {};
        mixer = false;
    }
    if (mixer)
        s = play.byPan == 0 ? panAtPositive(app, play.position, reg32(play.volume)) : splitPan(pan, play.volume);
    if (const reg32 bytes = frameFormat(bits, channels); bytes != 0)
        frameBytes = bytes;
    else if (!fail(app, cpu, f, kPlayPlatformError, 0xbe, {0x5519e0}))
        return {};
    const int32_t step = stepFor(app, cpu, int32_t(rate), kMixRate(app), play.pitch);
    int32_t level = int32_t(priority) + kPriorityOffset(app);
    level = level > 0x64 ? 0x64 : level < 0 ? 0 : level;
    p.method = uint8_t(method);
    ChannelStart start{wordAt(app, samplePlace), int32_t(length), int32_t(loopStart), int32_t(loopEnd), s.left,
                       s.right, uint32_t(step), level, play.send};
    return startChannel(app, cpu, f - 0x24 - 4, index, frameBytes, compression, reg32(mode), start);
}

/* 0x523d94's (iSNDplatformpacketplay): channel `index` started on packets (a
 * stream's), described by `sample` (its rate, its representation), at `pan`,
 * `volume`, `pitch`, `send` and `priority`.  startChannel's result, -15 for a
 * representation it has none for, -7 for a frame. */
Result playPackets(App* app, CPU& cpu, reg32 esp, int32_t index, int32_t pan, int32_t volume, int32_t pitch,
                   int32_t send, int32_t priority, reg32 sample)
{
    const reg32 f = esp - 0xc - 0xc;
    PlatformVoice& p = kPlatform(app, index);
    p.panSet = 1;
    p.position = 0;
    p.id = 0;
    p.rate = app->getMemory<uint16_t>(sample);
    p.channels = byteAt(app, sample + 2) & 3;
    const reg32 representation = app->getMemory<uint16_t>(sample + 2) & 0xfffc;
    if (representation == 0xc)
    {
        p.bits = 0x10;
        p.compression = 7;
    }
    else if (representation == 0x10)
    {
        p.bits = 0x10;
        p.compression = 9;
    }
    else if (representation == 8)
    {
        p.bits = 8;
        p.compression = 0;
    }
    else if (representation == 0)
    {
        p.bits = 0x10;
        p.compression = 0;
    }
    else
    {
        if (!fail(app, cpu, f, kPacketError, 0x44, {0x551bd8}))
            return {};
        return -15;
    }
    const Sides s = splitPan(pan, volume);
    const reg32 frameBytes = frameFormat(p.bits, p.channels);
    if (frameBytes == 0)
    {
        if (!fail(app, cpu, f, kPacketError, 0x59, {0x551c18}))
            return {};
        return kNoMethod;
    }
    const int32_t step = stepFor(app, cpu, p.rate, kMixRate(app), pitch);
    priority += kPriorityOffset(app);
    priority = priority > 0x64 ? 0x64 : priority < 0 ? 0 : priority;
    p.method = 0;
    ChannelStart start{0, 0, -1, -1, s.left, s.right, uint32_t(step), priority, send};
    return startChannel(app, cpu, f - 0x24 - 4, index, frameBytes, reg32(int32_t(int8_t(p.compression))), 0, start);
}

/* 0x523f90's (iSNDcheckplayopts): a play's options checked: key, velocity,
 * volume, bend, effect level, then the elevation of a 3D one or the pan, each
 * an error when out of range.  The last error's eax, or the options' address
 * `at` with none. */
Result checkPlayOptions(App* app, CPU& cpu, reg32 esp, reg32 at)
{
    const reg32 f = esp - 0x18;
    const PlayOptions& o = record<PlayOptions>(app, at);
    Result eax = int32_t(at);
    const struct
    {
        int8_t value;
        reg32 line, text;
    } checks[] = {{o.key, 0xa, 0x551c74}, {o.velocity, 0xd, 0x551cac}, {o.volume, 0x10, 0x551ce8},
                  {o.bend, 0x13, 0x551d1c}, {o.effectLevel, 0x16, 0x551d54}};
    const auto check = [&](reg32 line, reg32 text, int32_t value) {
        return bool(eax = fail(app, cpu, f, kCheckError, line, {text, reg32(value)}, reg32(*eax)));
    };
    for (const auto& c : checks)
        if (c.value < 0 && !check(c.line, c.text, c.value))
            return {};
    if (o.placed != 0)
    {
        if ((o.elevation < -0x4000 || o.elevation > 0x3fff) && !check(0x20, 0x551dc4, o.elevation))
            return {};
    }
    else if (o.pan < 0 && !check(0x1b, 0x551d90, o.pan))
        return {};
    return eax;
}

/* 0x4e8bd0's: the first `count` patches of bank `bank` let go of.  0. */
Result removePatches(App* app, CPU& cpu, reg32 esp, int32_t bank, int32_t count)
{
    const reg32 f = esp - 0x10 - 0xc00;
    for (int32_t i = 0; i < count; ++i)
        if (!removePatch(app, cpu, f - 4, bank, i))
            return {};
    return 0;
}

/* 0x4e8c28's (SNDbankremove): bank `bank` gone (every bank for -1): its sounds
 * stopped, its patches let go of, its handle free.  0, -8 for no such bank,
 * -10 before SNDinit. */
Result removeBank(App* app, CPU& cpu, reg32 esp, int32_t bank)
{
    if (kInited(app) == 0)
        return kNotStarted;
    const reg32 frame = esp - 0xc;
    if (bank == -1)
    {
        for (int32_t i = 0; i < kBankCount; ++i)
            if (!removeBank(app, cpu, frame - 4, i))
                return {};
        return 0;
    }
    if (kBanks(app, bank).loading != 0 && !fail(app, cpu, frame, kRemoveError, 0x6f, {0x54b12c}))
        return {};
    if (bankState(app, bank) != 0)
        return kNoSound;
    const reg32 b = kBanks(app, bank).bank;
    for (int32_t i = 0; i < kVoiceCount; ++i)
        if (int32_t(int8_t(kVoices(app, i).bank)) == bank
            && !stopSound(app, cpu, frame - 4, reg32(kVoices(app, i).handle)))
            return {};
    if (!removePatches(app, cpu, frame - 8, bank, record<BankHeader>(app, b).patches))
        return {};
    kBanks(app, bank).bank = 0;
    kBanks(app, bank).loading = 0;
    return 0;
}

/* 0x4e8c1c's: SNDbankremove of `bank` -- what SNDbankadd leaves SNDrestore to
 * call for the banks. */
Result removeEveryBank(App* app, CPU& cpu, reg32 esp, int32_t bank)
{
    return removeBank(app, cpu, esp - 8, bank);
}

/* 0x4e8d20's (SNDbankadd): the bank at `bank` added: its handle into
 * `handle`, its patches made addresses.  downloadBank's result, -10 before
 * SNDinit. */
Result addBank(App* app, CPU& cpu, reg32 esp, reg32 bank, reg32& handle)
{
    if (kInited(app) == 0)
        return kNotStarted;
    const reg32 frame = esp - 0xc;
    if (wordAt(app, bank) != kBankId && !fail(app, cpu, esp - 0x14, kAddError, 0x5b, {0x54b1b0}))
        return {};
    const Result free = freeBankHandle(app, cpu, frame - 4);
    if (!free)
        return {};
    handle = reg32(*free);
    kBanks(app, *free).bank = bank;
    kBanks(app, *free).loaded = bank;
    kBankHook.set(app, kRemoveAllBanks);
    return downloadBank(app, cpu, frame - 4, bank, bank);
}

/* 0x4e8db0's (SNDbankheadersize): the size of bank `bank`'s header; an error
 * for no such bank, -10 before SNDinit. */
Result bankHeaderSize(App* app, CPU& cpu, reg32 esp, int32_t bank)
{
    if (kInited(app) == 0)
        return kNotStarted;
    if (bankState(app, bank) != 0 && !fail(app, cpu, esp - 0x10, kHeaderSizeError, 0x47, {0x54b1f4}))
        return {};
    return int32_t(record<BankHeader>(app, kBanks(app, bank).bank).headerSize);
}

/* 0x4e8e20's (SNDbankheadercopy): bank `bank`'s header copied to `to`, which
 * the bank is then: its patches' addresses moved with it.  0, the header
 * size's error, -10 before SNDinit. */
Result copyBankHeader(App* app, CPU& cpu, reg32 esp, reg32 to, int32_t bank)
{
    const reg32 f = esp - 0x10 - 0xc;
    if (kInited(app) == 0)
        return kNotStarted;
    if (to == 0 && !fail(app, cpu, f, kHeaderCopyError, 0x4c, {0x54b24c}))
        return {};
    const Result size = bankHeaderSize(app, cpu, f - 4, bank);
    if (!size || *size < 0)
        return size;
    // EA's memmove: eax from, edx to, ebx bytes.
    if (!callGame(app, cpu, f - 4, kMoveMemory, {kBanks(app, bank).bank, to, reg32(*size), reg32(bank) * 12}))
        return {};
    const reg32 from = kBanks(app, bank).bank;
    for (int32_t i = 0; i < int32_t(record<BankHeader>(app, from).patches); ++i)
    {
        const reg32 table = record<BankHeader>(app, from).version == 4 ? 0x14 : 0xc;
        const reg32 p = wordAt(app, from + table + reg32(i) * 4);
        if (p != 0)
            app->getMemory<reg32>(to + table + reg32(i) * 4) = p - from + to;
    }
    kBanks(app, bank).bank = to;
    return 0;
}

/* 0x4fb1e0's (SNDbankpatpresent): whether bank `bank` has patch `patch`: 1 or
 * 0; -8 when the bank the code checks is none -- bank 0, or 1 for a bank
 * below 0, whichever was asked -- -10 before SNDinit. */
Result bankHasPatch(App* app, CPU& cpu, reg32 esp, int32_t bank, int32_t patch)
{
    if (kInited(app) == 0)
        return kNotStarted;
    const reg32 frame = esp - 0x14;
    if (bankState(app, bank < 0 ? 1 : 0) != 0)
        return kNoSound;
    const reg32 b = kBanks(app, bank).bank;
    if (patch < 0 && !fail(app, cpu, frame, kPatPresentError, 0x3b, {0x54dc48, reg32(patch)}))
        return {};
    if (patch >= int32_t(record<BankHeader>(app, b).patches))
        return 0;
    const uint8_t version = record<BankHeader>(app, b).version;
    if (version != 2 && version != 4 && !fail(app, cpu, frame, kPatPresentError, 0x47, {0x54dc78, version}))
        return {};
    return wordAt(app, patchSlot(app, b, patch)) != 0 ? 1 : 0;
}

/* 0x4e73d0's (SNDmastervol): the master volume to `volume` (0..127), every
 * playing voice's set again.  0, -10 before SNDinit. */
Result setMasterVolume(App* app, CPU& cpu, reg32 esp, int32_t volume)
{
    if (kInited(app) == 0)
        return kNotStarted;
    const reg32 frame = esp - 0xc;
    if ((volume < 0 || volume > 0x7f) && !fail(app, cpu, esp - 0x10, kMasterError, 0x51, {0x54ab48, reg32(volume)}))
        return {};
    if (!enterAudio(app, cpu, frame - 4))
        return {};
    kMasterVolume.set(app, uint8_t(volume));
    for (int32_t i = 0; i < kVoiceCount; ++i)
    {
        const Voice& v = kVoices(app, i);
        if (v.state != 1 || v.handle < 0)
            continue;
        refreshVolume(app, i);
        if (!voiceVolume(app, cpu, frame - 4, i, v.mixLevel))
            return {};
    }
    if (!leaveAudio(app, cpu, frame - 4))
        return {};
    return 0;
}

/* The game's entries: each its adapter -- the game's registers and stack in,
 * the result in eax out -- around what it does. */

void timbreDefaults(App* app, CPU& cpu)
{
    const Entry e(cpu);
    timbrePatchDefaults(record<Timbre>(app, e.eax));
    e.back(cpu, e.eax);
}

/* The timbre at eax, its places at edx; eax past the places. */
void resetTimbre(App* app, CPU& cpu)
{
    const Entry e(cpu);
    clearTimbre(record<Timbre>(app, e.eax), record<Places>(app, e.edx));
    e.back(cpu, e.edx + 0x98);
}

void addTimbreDeltas(App* app, CPU& cpu)
{
    const Entry e(cpu);
    addDataPlaces(record<Timbre>(app, e.eax), record<Places>(app, e.edx));
    e.back(cpu, e.eax);
}

/* stdcall 0x10: the timbre at ecx, its sample's tags edx, the options at ebx;
 * on the stack its places, the key, the velocity and the pitch's offset. */
void iSNDplaytaggedtimbre(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu,
             playTimbre(app, cpu, e.esp, record<Timbre>(app, e.ecx), e.edx, record<PlayOptions>(app, e.ebx),
                        record<Places>(app, e.arg(app, 0)), int32_t(e.arg(app, 1)), int32_t(e.arg(app, 2)),
                        int32_t(e.arg(app, 3))),
             0x10);
}

void iSNDplaytaggedpatch(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, playPatch(app, cpu, e.esp, e.eax, record<PlayOptions>(app, e.edx)));
}

void iSNDresolvetaggedpatch(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, resolvePatch(app, cpu, e.esp, e.eax, e.edx));
}

void unresolveTaggedPatch(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, unresolvePatch(app, cpu, e.esp, e.eax));
}

void iSNDextracthdrinfo(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, sampleInfo(app, cpu, e.esp, e.eax, e.edx));
}

void bankUsable(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, reg32(bankState(app, int32_t(e.eax))));
}

void iSNDbankremovepat(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, removePatch(app, cpu, e.esp, int32_t(e.eax), int32_t(e.edx)));
}

void iSNDbankalloc(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, freeBankHandle(app, cpu, e.esp));
}

void iSNDdownloadbank(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, downloadBank(app, cpu, e.esp, e.eax, e.edx));
}

void iSNDplatformresolve(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, resolveSample(app, cpu, e.esp, e.eax, e.edx));
}

void platformUnresolve(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, unresolveSample(app, cpu, e.esp, e.eax));
}

/* stdcall 0x24: the start's nine words on the stack, its step there kept to
 * 0x80000 as the code keeps it. */
void iSNDmixstart(App* app, CPU& cpu)
{
    const Entry e(cpu);
    ChannelStart start = load<ChannelStart>(app, e.esp + 4);
    const Result r = startChannel(app, cpu, e.esp, int32_t(e.eax), e.edx, e.ebx, e.ecx, start);
    store(app, e.esp + 4, start);
    e.finish(cpu, r, 0x24);
}

/* stdcall 0x18: the tags at eax, the mode edx, the voice ebx, the pan ecx; the
 * play's six words on the stack, its position and elevation changed there as
 * the code changes them. */
void iSNDplatformplay(App* app, CPU& cpu)
{
    const Entry e(cpu);
    SamplePlay play = load<SamplePlay>(app, e.esp + 4);
    const Result r = playSample(app, cpu, e.esp, e.eax, int32_t(e.edx), int32_t(e.ebx), int32_t(e.ecx), play);
    store(app, e.esp + 4, play);
    e.finish(cpu, r, 0x18);
}

/* stdcall 0x10: the voice edx, the pan ebx, the volume ecx; the pitch, the
 * send, the priority and the sample's description on the stack. */
void iSNDplatformpacketplay(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu,
             playPackets(app, cpu, e.esp, int32_t(e.edx), int32_t(e.ebx), int32_t(e.ecx), int32_t(e.arg(app, 0)),
                         int32_t(e.arg(app, 1)), int32_t(e.arg(app, 2)), e.arg(app, 3)),
             0x10);
}

void iSNDcheckplayopts(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, checkPlayOptions(app, cpu, e.esp, e.eax));
}

void removeBankPatches(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, removePatches(app, cpu, e.esp, int32_t(e.eax), int32_t(e.edx)));
}

void SNDbankremove(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, removeBank(app, cpu, e.esp, int32_t(e.eax)));
}

void removeAllBanks(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, removeEveryBank(app, cpu, e.esp, int32_t(e.eax)));
}

/* The handle into [eax], the bank at edx. */
void SNDbankadd(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 handle = app->getMemory<reg32>(e.eax);
    const Result r = addBank(app, cpu, e.esp, e.edx, handle);
    app->getMemory<reg32>(e.eax) = handle;
    e.finish(cpu, r);
}

void SNDbankheadersize(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, bankHeaderSize(app, cpu, e.esp, int32_t(e.eax)));
}

void SNDbankheadercopy(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, copyBankHeader(app, cpu, e.esp, e.eax, int32_t(e.edx)));
}

void SNDbankpatpresent(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, bankHasPatch(app, cpu, e.esp, int32_t(e.eax), int32_t(e.edx)));
}

void SNDmastervol(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, setMasterVolume(app, cpu, e.esp, int32_t(e.eax)));
}

}  // namespace port
}  // namespace nfs3hp
