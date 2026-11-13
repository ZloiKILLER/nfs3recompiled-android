#ifndef NFS3HP_PORT_SND_SPEECH_H
#define NFS3HP_PORT_SND_SPEECH_H

#include "port_snd.h"

#include <cmath>
#include <cstdint>

/* What the game's speech rewrites share (port_sound_speech.cpp,
 * port_sound_announcer.cpp, port_sound_copradio.cpp, port_sound_racebanks.cpp):
 * the speech's lines and the helpers the four files use.
 *
 * Their data stays at the game's addresses (port.h says why); the lines and
 * names the code builds on its stack lie on the game's stack where its frames
 * have them, so that what the callees read -- and what the stand-ins that the
 * checks put in the place of the game's functions log -- is the same. */
namespace nfs3hp
{
namespace port
{

#pragma pack(push, 1)

/* A line of the speech (9 words, 0x24 bytes), as the code queues it and the
 * speech says it.  The ninth is a local the code never writes: whatever lay
 * on the stack, copied along with the rest. */
struct SpeechLine
{
    int32_t effect;
    int32_t volume;
    int32_t pitch;
    uint32_t pan;
    int32_t bank;             // the SND handle of the bank
    int32_t player;
    int32_t kind;             // 2 a line the game is told of, 3 and 4 the race voice's, 5 silent over the radio
    int32_t delay;            // for kind 2: what the game is told it by
    int32_t unwritten;
};
static_assert(sizeof(SpeechLine) == 0x24, "the speech's line");

#pragma pack(pop)

// The pan of the left and right of the split screen.
constexpr uint32_t kSideLeft = 0x3fff, kSideRight = 0xc000;

constexpr Table<int32_t> kSpeechBankHandles{0x571124, 12};   // the game's banks' SND handles

// The banks the code names by their place in kSpeechBankHandles.
enum SpeechBank : int32_t
{
    kSpeechBankScenery = 1,     // 0x571130
    kSpeechBankCar = 2,         // 0x57113c
    kSpeechBankCrowd = 4,       // 0x571154
    kSpeechBankRaceVoice = 7,   // 0x571178 (the announcer's)
    kSpeechBankCopRadio = 8,    // 0x571184
    kSpeechBankSpeech = 9,      // 0x571190
    kSpeechBankLines = 10,      // 0x57119c
};

/* A call into the game's code as the rewrite makes it, with esi and edi as the
 * original code has them there (SND's code reads them: its errors push them
 * as arguments). */
inline bool callWith(App* app, CPU& cpu, reg32 esp, reg32 target, Args a, std::initializer_list<reg32> stack,
                     reg32 esi, reg32 edi, reg32& result)
{
    cpu.esi = esi;
    cpu.edi = edi;
    return callGame(app, cpu, esp, target, a, stack, result);
}

/* The same for the answer as a Result (empty when the game stopped). */
inline Result callResult(App* app, CPU& cpu, reg32 esp, reg32 target, Args a, std::initializer_list<reg32> stack,
                         reg32 esi, reg32 edi)
{
    reg32 r;
    if (!callWith(app, cpu, esp, target, a, stack, esi, edi, r))
        return {};
    return int32_t(r);
}

/* fistp as the x87 does it in round-to-nearest-even: `value` to the nearest
 * integer, "integer indefinite" past the integer's range. */
inline int32_t nearestInteger(double value)
{
    const double r = std::nearbyint(value);
    return r >= -2147483648.0 && r <= 2147483647.0 ? int32_t(r) : INT32_MIN;
}

/* fistp after Watcom's __CHP (frndint toward 0): towards 0, with the same
 * "integer indefinite" past the range. */
inline int32_t truncatedInteger(double value)
{
    const double r = std::trunc(value);
    return r >= -2147483648.0 && r <= 2147483647.0 ? int32_t(r) : INT32_MIN;
}

/* The x87's condition bits after fcomp of `a` with `b`, in the status word
 * `before` (the emulation sets C3, C2 and C0 and nothing else): what the code
 * reads with fnstsw. */
inline uint16_t comparedStatus(uint16_t before, double a, double b)
{
    uint16_t bits;
    if (a == b)
        bits = 0x4000;
    else if (a > b)
        bits = 0;
    else if (a < b)
        bits = 0x100;
    else
        bits = 0x4500;
    return uint16_t((before & ~0x4700) | bits);
}

/* The game's strcpy and strcat, inlined in its code: the text at `from` to
 * `to`, and after the text already at `to` (its length back). */
inline void copyText(App* app, reg32 to, reg32 from)
{
    for (;; ++to, ++from)
    {
        const uint8_t c = load<uint8_t>(app, from);
        store<uint8_t>(app, to, c);
        if (c == 0)
            break;
    }
}

inline reg32 appendText(App* app, reg32 to, reg32 from)
{
    reg32 end = to;
    while (load<uint8_t>(app, end) != 0)
        ++end;
    copyText(app, end, from);
    return end - to;
}

/* The units' folders (a name each), by the units. */
constexpr reg32 kSpeechUnitFolders[6] = {0x7a2b34, 0x7a2bb4, 0x7a2c34, 0x7a2cb4, 0x7a2d34, 0x7a2b34};

/* The speech's functions other files call (port_sound_speech.cpp).  Each takes
 * esp as the game's code has it at the call's return address. */

// 0x4c85a0: line `line` said, or queued while the player's plays.
Result speechLineSaid(App* app, CPU& cpu, reg32 esp, reg32 line);
// 0x4c8b30: player `player`'s cop radio of `kind` at `distance`.
Result copRadioStarted(App* app, CPU& cpu, reg32 esp, int32_t player, int32_t kind, int32_t distance);
// 0x4c89b0: the radio of `player`, if it is off, on.
Result radioStarted(App* app, CPU& cpu, reg32 esp, reg32 speaker, int32_t distance, int32_t pitch, reg32 pan,
                    int32_t player);
// 0x4c8f90: a line of the race voice queued, once the race is under way (else eax as it was).
Result raceLineQueued(App* app, CPU& cpu, reg32 esp, reg32 effect, reg32 kind, reg32 player);
// 0x4c9010, 0x4c9090, 0x4c91a0: the race voice's lines for an event, the gap and a lap time.
Result raceEventLineQueued(App* app, CPU& cpu, reg32 esp, reg32 player, reg32 event);
Result gapLineQueued(App* app, CPU& cpu, reg32 esp, reg32 car, reg32 player, reg32 toSay, reg32 ecx);
Result lapTimeLineQueued(App* app, CPU& cpu, reg32 esp, reg32 player, int32_t seconds);
// 0x4c86d0: player `player`'s speech tick.
Result playerSpeechTicked(App* app, CPU& cpu, reg32 esp, int32_t player);

// 0x412be0: the radio's line for `event`, at random among the event's own.
Result copLineChosen(App* app, CPU& cpu, reg32 esp, int32_t event);

}  // namespace port
}  // namespace nfs3hp

#endif
