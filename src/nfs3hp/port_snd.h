#ifndef NFS3HP_PORT_SND_H
#define NFS3HP_PORT_SND_H

#include "host_files.h"
#include "host_system.h"
#include "native_windows.h"
#include "port.h"
#include "port_sound_calls.h"

#include <lib/winapp.h>
#include <winapi/types.h>

#include <array>
#include <cstdint>
#include <initializer_list>

/* EA's SND library, rewritten (port.h says how): the voices the game plays
 * its sounds on, their volume, pan and pitch, the 100 Hz tick that moves
 * their envelopes, the platform layer that sets the mixer's channels for
 * them (port_snd_voices.cpp); and what a sound is played from -- banks,
 * patches, timbres -- and how a sample starts on a channel
 * (port_snd_play.cpp).  The mixer's own code, the streams and the driver are
 * still the register-level natives' (native_mixer.cpp, native_snd_*.cpp).
 *
 * Each of the library's functions is here twice: as C++ with arguments and a
 * result, which the rewrites call -- the game's sound effects among them --
 * and as the game's entry, by the name the game's code knows
 * (its address is bound to it), a thunk around the
 * first.  None of the game's callers reads more of what the library's code
 * leaves than eax, so each rewrite gives back only its result.
 *
 * The records stay where the game has them while the game's code shares
 * them -- the mixer reads the channels, the streams the voices -- and are
 * read and written in place. */
namespace nfs3hp
{
namespace port
{

#pragma pack(push, 1)

/* A voice of the library's, one of 16 at 0xa09b3c. */
struct Voice
{
    int32_t handle;           // the handle the game holds; -1 for a voice another one leads
    uint8_t led[4];           // the voices it leads
    uint16_t patch;           // the play's patch
    uint8_t bank;             // and its bank
    uint8_t timbre6c;         // the timbre's +0x6c
    uint8_t state;            // 0 free, 1 playing, 2 ending with its group
    uint8_t mode;             // how its sample plays (the timbre's +0)
    uint8_t priority;
    uint8_t pan;              // the pan it is played at
    int16_t pitch;
    int16_t playedPitch;      // with its bend and LFO
    uint32_t tick;            // the tick it was taken or freed at
    int32_t volumeStep;       // 16.16, a tick
    int32_t volumeTarget;
    int32_t volume;
    int32_t envelopeStep;     // 16.16, a tick
    int32_t envelope;
    int32_t envelopeTicks;    // left of its segment
    int8_t level;             // the voice's volume: the timbre's at the play's velocity
    int8_t mixLevel;          // what the mixer gets
    int8_t patchPan;
    uint8_t bend;
    int8_t segments;          // the envelope's
    int8_t segment;           // the one it is in
    uint8_t timbre20;         // the timbre's +0x20
    int8_t spread;            // the pan's about the middle (1: none)
    int8_t timbreSend;        // the timbre's effect level
    int8_t send;              // the play's effect level
    uint8_t endsGroup;
    uint8_t group;            // 0 none
    uint8_t volumeLfoLength;
    uint8_t pitchLfoLength;
    uint8_t volumeLfoPlace;
    uint8_t pitchLfoPlace;
    int16_t pitchLfoDepth;
    int16_t bendRange;
    int8_t leader;            // the voice that leads it
    int8_t panAsked;
    uint8_t other46[2];
    uint32_t envelopeSegments;  // 8 bytes a segment: its ticks, its level
    uint32_t volumeCurve;
    uint32_t panCurve;
    uint32_t bendCurve;
    uint32_t volumeLfo;
    uint32_t pitchLfo;
};
static_assert(sizeof(Voice) == 0x60, "SND's voice");

/* The platform's view of the same voice, one of 16 at 0x9ef658. */
struct PlatformVoice
{
    uint16_t rate;
    uint8_t bits;
    uint8_t channels;
    uint8_t compression;
    uint8_t method;           // 0 and 2 the mixer, 1 the mixer in Dolby surround, 3 eacsnd's 3D buffers
    uint8_t panSet;           // its pan the pan's, else its position's
    uint8_t other7;
    int8_t surroundLeft;      // the surround's two levels, as panSurround leaves them
    int8_t surroundRight;
    uint16_t position;        // the angle around, 0x10000 a turn
    uint32_t id;
};
static_assert(sizeof(PlatformVoice) == 0x10, "SND's platform voice");

/* A channel of the mixer's, one of 16 at 0x9fbcb4 (native_mixer.cpp's).
 * What the library sets of it; the rest is the mixer's, its functions' states
 * at the offsets below. */
struct Channel
{
    uint8_t playing;
    int8_t left;              // the volumes it is at
    int8_t right;
    int8_t sendLevel;
    int8_t leftTarget;        // and ramps to, a step a frame
    int8_t rightTarget;
    int8_t leftWay;           // 1 up, -1 down
    int8_t rightWay;
    int8_t send;              // the effect send's volume
};

// The parts of a channel the mixer's functions keep their states in, and
// those functions.
constexpr reg32 kChannelDecoder = 0xc, kChannelResampler = 0xd64, kChannelVolumes = 0xd90;
constexpr reg32 kChannelDa8 = 0xda8, kChannelDac = 0xdac, kChannelSetStep = 0xdb0, kChannelDb4 = 0xdb4;
constexpr reg32 kChannelDb8 = 0xdb8, kChannelSetVolume = 0xdbc, kChannelDc0 = 0xdc0, kChannelSend = 0xdc4;
constexpr reg32 kChannelSetSend = 0xddc, kChannelDe0 = 0xde0;

/* One of the 24 banks' slots at 0xa0a13c. */
struct BankSlot
{
    uint32_t bank;            // where the bank is (0 none)
    uint32_t loaded;          // where it was loaded
    uint8_t loading;          // being loaded
    uint8_t other9[3];
};
static_assert(sizeof(BankSlot) == 0xc, "SND's bank slot");

/* A bank ('BNKl'): its patches' table at +0xc, or at +0x14 for version 4. */
struct BankHeader
{
    uint32_t id;
    uint8_t version;
    uint8_t other5;
    uint16_t patches;
    uint32_t headerSize;
};

/* A play's options (SNDplaysetdef fills them, SNDplay takes them). */
struct PlayOptions
{
    int32_t patch;            // in the bank
    int8_t bank;              // the bank's handle
    int8_t key;
    int8_t velocity;
    int8_t pan;               // stereo: 0..127, 0x40 the middle
    int8_t volume;            // 0..127
    int8_t bend;              // 0x40 as recorded
    int8_t effectLevel;
    uint8_t placed;           // 1: by its 3D position
    uint16_t position;        // the angle around, 0x10000 a turn
    int16_t elevation;
};
static_assert(sizeof(PlayOptions) == 0x10, "SND's play options");

/* A timbre's settings, as a patch's tags make them: dword i is tag i's
 * number.  Its data's places -- where each tag's data is in the patch -- are
 * a second such record. */
struct Timbre
{
    int32_t mode;             // how its sample plays: 1, or 2 as a stream
    int32_t lowVelocity, highVelocity;
    int32_t lowKey, highKey;
    uint32_t voiceMask;
    int32_t priority;
    int32_t rootKey;          // the key it was sampled at
    int32_t timbre20;
    int32_t segments;         // the envelope's
    int32_t bendRange;
    int32_t voices;
    int32_t pan, panRandom;
    int32_t volume, volumeRandom;
    int32_t pitch, pitchRandom;
    uint32_t volumeCurve;
    int32_t send;
    int32_t tag14;
    uint32_t panCurve;
    int32_t tag16;
    uint32_t bendCurve;
    int32_t tag18;
    uint32_t envelope;
    int32_t tag1a;
    int32_t timbre6c;
    int32_t envelopeStart;
    uint32_t volumeLfo;
    int32_t volumeLfoLength;
    int32_t volumeLfoRandom;  // its place a random one
    uint32_t pitchLfo;
    int32_t pitchLfoLength;
    int32_t pitchLfoDepth;
    int32_t pitchLfoRandom;
    int32_t playPitch;        // whether the play's pitch counts
    int32_t spread;

    /* Tag `tag`'s number (or data's place) set, the dword it is. */
    void set(reg32 tag, int32_t value) { std::memcpy(reinterpret_cast<uint8_t*>(this) + 4 * tag, &value, 4); }
};
static_assert(sizeof(Timbre) == 0x98, "SND's timbre settings");

/* What a patch's tag reader has: the place it reads on from, and of the tag it
 * read, the tag, its first up to 4 bytes as a number, and where they are. */
struct Tag
{
    reg32 place;
    reg32 tag;
    reg32 number;
    reg32 data;
};

#pragma pack(pop)

/* A volume panned onto two sides. */
struct Sides
{
    int32_t left, right;
};

// The library's state.
constexpr Records<Voice> kVoices{0xa09b3c};
constexpr int32_t kVoiceCount = 16;
constexpr Records<PlatformVoice> kPlatform{0x9ef658};
constexpr Records<Channel> kChannels{0x9fbcb4, 0xde4};
constexpr Records<BankSlot> kBanks{0xa0a13c};
constexpr int32_t kBankCount = 0x18;
constexpr Var<uint8_t> kInited{0xa09af4};         // SNDinit done
constexpr Var<uint8_t> kMasterVolume{0xa09af5};   // 0..127
constexpr Var<int8_t> kVoicesUsed{0xa09af6};      // the voices the game asked for
constexpr Var<uint8_t> kInTick{0xa09af7};
constexpr Var<uint8_t> kEntered{0xa09af8};        // in the library's lock
constexpr Var<int8_t> kClientCount{0xa09af9};     // the tick's clients
constexpr Var<uint32_t> kTicks{0xa09afc};
constexpr Var<reg32> kLock{0xa09b04};             // the critical section's address
constexpr Table<reg32> kClients{0xa09b0c};        // 6 functions the tick calls first
constexpr Var<uint8_t> kGroupSerial{0x5678b0};    // the last group handed out
constexpr Var<int32_t> kHandleSerial{0x56786c};   // a handle's high bits, 0x20 a time
constexpr Table<int8_t> kPicked{0x9f6a54};        // the voices allocVoices picked
constexpr Var<uint16_t> kMixRate{0x9ef614};       // the mixer's rate
constexpr Var<int16_t> kPriorityOffset{0x9ef616}; // added to a sample's priority
constexpr Var<int32_t> kBusLevel{0xa0a260};       // the first effect bus's level
constexpr Var<reg32> kChannelEnded{0x9f6b1c};     // what the mixer calls when a channel ends
constexpr Var<reg32> kEmms{0x9f6b24};
constexpr Var<reg32> kBankHook{0xa09b24};         // what SNDrestore calls for the banks

// What the library calls of the game's.
constexpr reg32 kSurfaceLocked = 0x4f9070;
constexpr reg32 kEnterCriticalSection = 0x5344b8, kLeaveCriticalSection = 0x534588;  // their imports' slots
constexpr reg32 kRamp = 0x5004c8;        // the mixer's volume ramp (channel, frames)
constexpr reg32 kMoveMemory = 0x4ea4f0;  // EA's memmove: eax to, edx from, ebx bytes
constexpr reg32 kRemoveAllBanks = 0x4e8c1c;

// Its error codes.
constexpr int32_t kNoSound = -8, kNoVoices = -9, kNotStarted = -10, kNoMethod = -7;

/* imul and idiv as the code has them: 32 bits, wrapping; a quotient towards
 * 0, by -1 a negation. */
inline int32_t imul(int32_t a, int32_t b) { return int32_t(uint32_t(a) * uint32_t(b)); }
inline int32_t idiv(int32_t a, int32_t b) { return b == -1 ? int32_t(0u - uint32_t(a)) : a / b; }
inline int32_t negated(int32_t a) { return int32_t(0u - uint32_t(a)); }

/* A Windows function (`Function`) called as callGame calls the game's code: esp and the register
 * arguments `a` set, `words` handed.  The port's own, called as C++ (native_windows.h); in the
 * check through its import's slot `Slot`.  `result` its eax; false when the game stops. */
template <reg32 Slot, auto Function, typename... Words>
inline bool callWindows(App* app, CPU& cpu, reg32 esp, Args a, reg32& result, Words... words)
{
    cpu.esp = esp;
    // a host call's result when it has none: eax as it was
    const std::optional<reg32> eax = guest::windowsKeeping<Slot, Function>(app, cpu, a.eax, words...);
    if (!eax)
        return false;
    result = *eax;
    return true;
}

/* The same with no register set: a Windows function takes its words only.  esp where the game's code has it
 * at the call. */
template <reg32 Slot, auto Function, typename... Words>
inline bool callWindowsWords(App* app, CPU& cpu, reg32 esp, reg32& result, Words... words)
{
    cpu.esp = esp;
    const std::optional<reg32> eax = guest::windows<Slot, Function>(app, cpu, words...);
    if (!eax)
        return false;
    result = *eax;
    return true;
}

/* callWindowsWords with `eax` what the game's code has in eax at the call, and
 * then what the call leaves there: its result, or for a Windows function with
 * none, eax as it was.  A call to the host, which stays as it is
 * this pass: eax set for it is the one register it writes. */
template <reg32 Slot, auto Function, typename... Words>
inline bool callWindowsEax(App* app, CPU& cpu, reg32 esp, reg32& eax, Words... words)
{
    cpu.esp = esp;
    const std::optional<reg32> left = guest::windowsKeeping<Slot, Function>(app, cpu, eax, words...);
    if (!left)
        return false;
    eax = *left;
    return true;
}

/* Where an error comes from -- the source file's and the function's names, as
 * the game has them -- and Error's text. */
struct Where
{
    reg32 file, function;
};

/* An error of the library's, as its code raises one: where it comes from in
 * the game's kWhere, then Error with `arguments` pushed from `esp` and `eax`
 * as the code has it.  Error's eax, for it may come back -- with one under
 * way, the eax it got; let go by the game's filter, 0 -- and the code goes
 * on. */
Result fail(App* app, CPU& cpu, reg32 esp, Where where, reg32 line, std::initializer_list<reg32> arguments,
            reg32 eax = 0);

/* The game's sound effect (port_sound_effects.cpp): effect `effect` of bank
 * `bank` (a handle) at `volume`, pan `pan`, pitch `pitch`; `esp` as the
 * game's code has it on entering 0x4182b0.  The sound's handle, -1 with the
 * effects off. */
Result bankEffect(App* app, CPU& cpu, reg32 esp, int32_t bank, int32_t effect, int32_t volume, uint32_t pan,
                  uint8_t pitch);

/* The voices, their volume, pan and pitch (port_snd_voices.cpp). */

reg32 nextRandom(App* app);
int32_t randomWithin(App* app, int32_t range);
int32_t bigEndian(App* app, reg32 at, int32_t bytes);
bool readTag(App* app, Tag& tag);
Sides splitPan(int32_t pan, int32_t volume);
Sides panAt(App* app, reg32 angle, reg32 volume);
Sides panAtPositive(App* app, reg32 angle, reg32 volume);
Sides panSurroundAt(App* app, reg32 angle, reg32 volume, int32_t front, int32_t back);
int32_t voiceOfHandle(App* app, reg32 handle);
bool isPicked(App* app, int32_t index, int32_t count);
Result freeVoice(App* app, CPU& cpu, reg32 esp, int32_t index);
Result allocVoices(App* app, CPU& cpu, reg32 esp, uint32_t mask, int32_t count, uint8_t priority, reg32& handle);
Result enterAudio(App* app, CPU& cpu, reg32 esp);
Result leaveAudio(App* app, CPU& cpu, reg32 esp, reg32 eax = 0);
Result addTickClient(App* app, CPU& cpu, reg32 esp, reg32 client);
Result removeTickClient(App* app, CPU& cpu, reg32 esp, reg32 client);
Result sendVolume(App* app, CPU& cpu, reg32 esp, reg32 channel);
Result channelVolume(App* app, CPU& cpu, reg32 esp, int32_t index, int32_t left, int32_t right);
Result channelSendLevel(App* app, CPU& cpu, reg32 esp, int32_t index, uint8_t level);
Result voiceSendLevel(App* app, CPU& cpu, reg32 esp, int32_t index, int32_t level);
int32_t refreshVolume(App* app, int32_t index);
Result voiceVolume(App* app, CPU& cpu, reg32 esp, int32_t index, int32_t volume, reg32 ebp);
reg32 refreshPitch(App* app, int32_t index);
x86::Float pitchScale(App* app, CPU& cpu, int32_t cents);
int32_t stepFor(App* app, CPU& cpu, int32_t rate, int32_t mixRate, int32_t pitch);
Result channelStep(App* app, CPU& cpu, reg32 esp, int32_t index, uint32_t step);
Result detuneVoice(App* app, CPU& cpu, reg32 esp, int32_t index, int32_t pitch);
Result placeVoice(App* app, CPU& cpu, reg32 esp, int32_t index, uint32_t position, int32_t elevation);
Result panVoice(App* app, CPU& cpu, reg32 esp, int32_t index, int32_t pan);
Result silenceChannel(App* app, CPU& cpu, reg32 esp, int32_t index);
Result stopVoice(App* app, CPU& cpu, reg32 esp, int32_t index);
Result tickSoundFrom(App* app, CPU& cpu, reg32 esp, reg32 eax, reg32 edx, reg32 ecx, reg32 ebp);
// 0x4ffce0 (eacsnd's buffer position): the three floats at `to` from the angle and the elevation; the pan's second side.
Result bufferPositioned(App* app, CPU& cpu, reg32 esp, uint32_t angle, uint32_t elevation, reg32 to);
Result stopSound(App* app, CPU& cpu, reg32 esp, reg32 handle);
Result soundStatus(App* app, CPU& cpu, reg32 esp, reg32 handle);
Result stopAllSounds(App* app, CPU& cpu, reg32 esp);
Result fadeSound(App* app, CPU& cpu, reg32 esp, reg32 handle, int32_t ticks, int32_t target);
Result volumeNow(App* app, CPU& cpu, reg32 esp, reg32 handle, int32_t volume, reg32 ebp);
Result setVolumeFrom(App* app, CPU& cpu, reg32 esp, reg32 handle, int32_t volume, reg32 ebp);
Result setVolume(App* app, CPU& cpu, reg32 esp, reg32 handle, int32_t volume);
Result bendNow(App* app, CPU& cpu, reg32 esp, reg32 handle, int32_t bend);
Result setBendFrom(App* app, CPU& cpu, reg32 esp, reg32 handle, int32_t bend, reg32 ebx, reg32 esi, reg32 edi);
Result setBend(App* app, CPU& cpu, reg32 esp, reg32 handle, int32_t bend);
Result placeSound(App* app, CPU& cpu, reg32 esp, reg32 handle, reg32 position, int32_t elevation);
Result panSound(App* app, CPU& cpu, reg32 esp, reg32 handle, int32_t pan);
void playDefaults(PlayOptions& options);
Result playByOptions(App* app, CPU& cpu, reg32 esp, const PlayOptions& options);
Result playSound(App* app, CPU& cpu, reg32 esp, const PlayOptions& options);
Result playingLevel(App* app, CPU& cpu, reg32 esp, reg32 handle);
uint8_t nextGroup(App* app);

/* What a sound is played from (port_snd_play.cpp). */

/* A sample's play on a channel, as iSNDplatformplay takes it on the stack. */
struct SamplePlay
{
    int32_t volume;
    int32_t pitch;
    int32_t send;
    reg32 position;
    int32_t elevation;
    int32_t byPan;            // its pan the pan's, else the position's
};

/* A channel's start, as iSNDmixstart takes it on the stack. */
struct ChannelStart
{
    reg32 sample;
    int32_t length;
    int32_t loopStart, loopEnd;
    int32_t left, right;
    uint32_t step;
    int32_t priority;         // above 0x19 the better resampler
    int32_t send;
};
static_assert(sizeof(SamplePlay) == 0x18 && sizeof(ChannelStart) == 0x24, "the stack's words");

using Places = std::array<reg32, 0x26>;

void timbrePatchDefaults(Timbre& timbre);
void clearTimbre(Timbre& timbre, Places& places);
void addDataPlaces(Timbre& timbre, const Places& places);
Result playTimbre(App* app, CPU& cpu, reg32 esp, Timbre& timbre, reg32 sample, const PlayOptions& options,
                  const Places& places, int32_t key, int32_t velocity, int32_t pitchOffset);
Result playPatchFrom(App* app, CPU& cpu, reg32 esp, reg32 patch, const PlayOptions& options, reg32 ebp);
Result playPatch(App* app, CPU& cpu, reg32 esp, reg32 patch, const PlayOptions& options);
Result resolvePatch(App* app, CPU& cpu, reg32 esp, reg32 patch, reg32 base);
Result unresolvePatch(App* app, CPU& cpu, reg32 esp, reg32 patch, reg32 esi, reg32 ebp);
Result sampleInfo(App* app, CPU& cpu, reg32 esp, reg32 patch, reg32 info);
int32_t bankState(App* app, int32_t bank);
Result removePatch(App* app, CPU& cpu, reg32 esp, int32_t bank, int32_t index, reg32 esi, reg32 ebp);
Result freeBankHandle(App* app, CPU& cpu, reg32 esp);
Result downloadBank(App* app, CPU& cpu, reg32 esp, reg32 bank, reg32 base);
Result resolveSample(App* app, CPU& cpu, reg32 esp, reg32 tags, reg32 base);
Result unresolveSample(App* app, CPU& cpu, reg32 esp, reg32 tags, reg32 esi, reg32 ebp);
Result startChannel(App* app, CPU& cpu, reg32 esp, int32_t index, reg32 format, reg32 compression, reg32 mode,
                    ChannelStart& start, reg32 edi, reg32 ebp);
Result playSample(App* app, CPU& cpu, reg32 esp, reg32 tags, int32_t mode, int32_t index, int32_t pan,
                  SamplePlay& play);
Result playPackets(App* app, CPU& cpu, reg32 esp, int32_t index, int32_t pan, int32_t volume, int32_t pitch,
                   int32_t send, int32_t priority, reg32 sample);
Result checkPlayOptions(App* app, CPU& cpu, reg32 esp, reg32 at);
Result removePatches(App* app, CPU& cpu, reg32 esp, int32_t bank, int32_t count, reg32 esi, reg32 ebp);
Result removeBank(App* app, CPU& cpu, reg32 esp, int32_t bank, reg32 esi, reg32 ebp);
Result removeEveryBankFrom(App* app, CPU& cpu, reg32 esp, int32_t bank, reg32 esi, reg32 ebp);
Result removeEveryBank(App* app, CPU& cpu, reg32 esp, int32_t bank);
Result addBank(App* app, CPU& cpu, reg32 esp, reg32 bank, reg32& handle);
Result bankHeaderSize(App* app, CPU& cpu, reg32 esp, int32_t bank);
Result copyBankHeader(App* app, CPU& cpu, reg32 esp, reg32 to, int32_t bank);
Result bankHasPatch(App* app, CPU& cpu, reg32 esp, int32_t bank, int32_t patch);
Result masterVolumeFrom(App* app, CPU& cpu, reg32 esp, int32_t volume, reg32 ebp);
Result setMasterVolume(App* app, CPU& cpu, reg32 esp, int32_t volume);

/* The bus's effect (port_snd_fx.cpp): iSNDwinsetfx's core and its thunk,
 * what the driver's start calls through its pointer. */
Result applyBusEffect(App* app, CPU& cpu, reg32 esp, reg32 type);
x86::reg32 iSNDwinsetfx(App* app, CPU& cpu, x86::Args& args);

}  // namespace port
}  // namespace nfs3hp

#endif
