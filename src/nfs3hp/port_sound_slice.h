#ifndef NFS3HP_PORT_SOUND_SLICE_H
#define NFS3HP_PORT_SOUND_SLICE_H

#include "port_snd.h"

#include <cstdint>
#include <initializer_list>

/* What the game's music, banks, race set-up, player's car sound and pursuit
 * (port_sound_music.cpp, port_sound_banks.cpp, port_sound_racesetup.cpp,
 * port_sound_player.cpp, port_sound_pursuit.cpp) share, besides port_snd.h's
 * library: how they call the game's code that is not rewritten yet, the
 * game's strings, and the functions of one that another calls as C++ (each
 * is also the game's entry, an adapter under the game's name).
 *
 * A rewrite of these keeps esi and edi as the game's code that calls the
 * next function has them, where that code sets them for its own use -- the
 * library, the effects and the streams read them unset (port.h says why):
 * Game holds both, calls with them, and gives them back as they were. */
namespace nfs3hp
{

// nfs3hp_main.cpp's: the track's index file as the game opens it, and its own
// stream file in place of the copy's (tools/apply_music_stream.py).
void musicIndexOpened(win32::WinApplication* app, x86::reg32 path);
void musicStreamFile(win32::WinApplication* app, x86::reg32 path);

namespace port
{

// The adapters of the other rewrites this group calls (port_snd_voices.cpp, port_snd_play.cpp,
// port_sound_effects.cpp), by the names the game's code knows them.
void SNDstatus(App* app, CPU& cpu);
void SNDstopall(App* app, CPU& cpu);
void SNDvol(App* app, CPU& cpu);
void SNDpitchbend(App* app, CPU& cpu);
void SNDpan(App* app, CPU& cpu);
void SNDmastervol(App* app, CPU& cpu);
void SNDbankadd(App* app, CPU& cpu);
void SNDbankheadersize(App* app, CPU& cpu);
void SNDbankheadercopy(App* app, CPU& cpu);
void removeAllBanks(App* app, CPU& cpu);
void slotStop(App* app, CPU& cpu);
void playCarEffectFive(App* app, CPU& cpu);
void playEffect(App* app, CPU& cpu);
void busUp(App* app, CPU& cpu);
void busDown(App* app, CPU& cpu);

/* A call into the game's code from a rewrite, with esi and edi (the rewrite's
 * own for the callee) as the game's code that makes the call has them and
 * `esp` as it has it there; the callee's eax, or nothing when the game
 * stopped.  esi and edi go back as they were with the Game. */
class Game
{
public:
    App* app;
    CPU& cpu;
    reg32 esp;
    reg32 esi, edi;

    Game(App* a, CPU& c, reg32 stack) : app(a), cpu(c), esp(stack), esi(c.esi), edi(c.edi), savedEsi(c.esi), savedEdi(c.edi) {}
    ~Game()
    {
        cpu.esi = savedEsi;
        cpu.edi = savedEdi;
    }
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    /* `target` called with the register arguments `a` and `stack` pushed;
     * `eax` is set to what it gives back. */
    bool call(reg32 target, Args a, std::initializer_list<reg32> stack, reg32& eax)
    {
        cpu.esi = esi;
        cpu.edi = edi;
        if (const guest::Native rewrite = rewriteAt(target))
            return callGame(app, cpu, esp, rewrite, a, stack, eax);
        return callGame(app, cpu, esp, target, a, stack, eax);
    }
    bool call(reg32 target, Args a, reg32& eax) { return call(target, a, {}, eax); }
    /* esi and edi into the cpu, for a rewrite called as C++ (it finds them
     * there). */
    void sync()
    {
        cpu.esi = esi;
        cpu.edi = edi;
    }
    /* esi, edi and esp into the cpu as the game's code has them at a call, for
     * a typed core called as C++ in its place (guest::direct; the words the
     * call pushes its ByAddress pushes in the checks). */
    void atCall()
    {
        cpu.esi = esi;
        cpu.edi = edi;
        cpu.esp = esp;
    }

private:
    reg32 savedEsi, savedEdi;

    /* The adapter of a function that is a rewrite too (SND's, the effects'),
     * to be run as C++ and not found through the game's address -- as the
     * game's code calls it, but where a stand-in stands at the address (the
     * checks') it is the rewrite that must run. */
    static guest::Native rewriteAt(reg32 target)
    {
        switch (target)
        {
        case 0x4e8fd0: return SNDstatus;
        case 0x4e9010: return SNDstopall;
        case 0x4e95fc: return SNDvol;
        case 0x4e97e4: return SNDpitchbend;
        case 0x4e9950: return SNDpan;
        case 0x4e73d0: return SNDmastervol;
        // 0x4e8d20, 0x4e8db0 and 0x4e8e20 (SND's bank add and header calls) are called by
        // address: the checks stand in for them on both sides (generate.py STOOD_IN).
        case 0x4e8c1c: return removeAllBanks;
        case 0x416f60: return slotStop;
        case 0x418280: return playCarEffectFive;
        case 0x4182b0: return playEffect;
        case 0x418770: return busUp;
        case 0x418790: return busDown;
        default: return nullptr;
        }
    }
};

// port_sound_pursuit.cpp's: the game's 0x418930, player `player`'s beeps started (`count` of them, the first
// high): the beep's handle; esp the frame base.
Result beepsStarted(App* app, CPU& cpu, reg32 esp, int32_t count, int32_t player);
// port_sound_pursuit.cpp's: the game's 0x418c60, the music's event back after an interruption (`edx` handed on to
// the interruption's end): eax as the code leaves it; esp the frame base.
Result musicEventResumed(App* app, CPU& cpu, reg32 esp, reg32 eax, reg32 edx);

}  // namespace port

namespace guest
{

// The count in eax, the player in edx; eax back.
template <>
struct ByAddress<port::beepsStarted>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, std::int32_t count,
                             std::int32_t player)
    {
        return watcomEax(app, cpu, Address, {x86::reg32(count), x86::reg32(player)});
    }
};

// `eax` in eax, `edx` in edx; eax back.
template <>
struct ByAddress<port::musicEventResumed>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 eax, x86::reg32 edx)
    {
        return watcomEax(app, cpu, Address, {eax, edx});
    }
};

}  // namespace guest

namespace port
{

/* A loop's safepoint, as the game's generated code has one at each loop's
 * head: the guest's context handed over when another thread waits for it. */
inline void safepoint(App* app, CPU& cpu)
{
    if (app->contextWanted())
        app->yieldContext(cpu);
}

// The game's folders: strings, by their place.
constexpr reg32 kAudioDir = 0x7a2a34, kTempDir = 0x7a2734;
constexpr reg32 kFormatTwo = 0x536890;  // "%s%s"

// What the group calls of the game's that is not rewritten yet.
constexpr reg32 kSprintf = 0x4df690, kFileExists = 0x4e0e60, kLoadFile = 0x4e0ec0, kPurgeMemory = 0x4e1890;
constexpr reg32 kReserveMemory = 0x4e1620, kUnlink = 0x4e28a0, kReadFile = 0x4e9200, kSeekFile = 0x4e9088;
constexpr reg32 kOpenArchive = 0x43b830, kCloseArchive = 0x43b9b0, kFindInArchive = 0x43b9d0;
constexpr reg32 kPathInit = 0x4e6dc8, kPathStartStream = 0x4e6f0c, kPathControl = 0x4e705c, kPathEvent = 0x4e70fc;
constexpr reg32 kPathEventInterrupt = 0x4e7208, kPathRestore = 0x4e7388;
constexpr reg32 kStreamAutoVolume = 0x4e7470, kStreamGetVolume = 0x4e74f0, kRunSystemTasks = 0x4e7630;
constexpr reg32 kStreamRestore = 0x4e784c, kStreamPurge = 0x4e7b28, kStreamMake = 0x4e4790, kStreamDelete = 0x4e4b50;
constexpr reg32 kSoundRestore = 0x4e26b4;
constexpr reg32 kEffectBank = 0x4182b0;  // port_sound_effects.cpp's bankEffect, as the game calls it

/* A typed core's result, called as C++ (guest::direct), as the game's code
 * has it in eax after the call: false when the game stopped in it. */
template <class T>
bool tookEax(const std::optional<T>& result, reg32& eax)
{
    if (!result)
        return false;
    eax = reg32(*result);
    return true;
}
inline bool tookEax(CPU& cpu, reg32 result, reg32& eax)
{
    eax = result;
    return !cpu.terminate;
}

/* A string of the game's, its length not counting the end. */
inline reg32 textLength(App* app, reg32 at)
{
    reg32 n = 0;
    while (app->getMemory<uint8_t>(at + n) != 0)
        ++n;
    return n;
}

/* The game's inlined strcpy: `from` to `to`, two bytes a turn, its end too.
 * esi where its loop stopped reading (the code's own registers keep that). */
inline reg32 copyText(App* app, reg32 to, reg32 from)
{
    reg32 at = from;
    for (reg32 i = 0;; i += 2)
    {
        uint8_t c = app->getMemory<uint8_t>(at);
        app->getMemory<uint8_t>(to + i) = c;
        if (c == 0)
            break;
        c = app->getMemory<uint8_t>(at + 1);
        at += 2;
        app->getMemory<uint8_t>(to + i + 1) = c;
        if (c == 0)
            break;
    }
    return at;
}

/* The game's inlined strcat: `from` after the end of the string at `to`.
 * What it leaves in ecx, its repne scasb's count: -2 less the length of the
 * string measured (a callee that takes ecx finds that there); in `esi` (if
 * given) where its loop stopped reading. */
inline reg32 appendText(App* app, reg32 to, reg32 from, reg32* esi = nullptr)
{
    const reg32 length = textLength(app, to);
    const reg32 stop = copyText(app, to + length, from);
    if (esi != nullptr)
        *esi = stop;
    return reg32(-2 - int32_t(length));
}

/* The game's rock-or-tech setting of its music: rock while its mode (the
 * bytes at 0x55eb2e) says -- 0x10, 0x20, or the next byte's bit 0. */
inline uint8_t gameMode(App* app) { return app->getMemory<uint8_t>(0x55eb2e); }
inline bool rockMode(App* app)
{
    return (gameMode(app) & 0x30) != 0 || (app->getMemory<uint8_t>(0x55eb2f) & 1) != 0;
}

// The music (port_sound_music.cpp), the banks, the race's set-up and the
// pursuit, as C++ for each other.
Result musicStopped(App* app, CPU& cpu, reg32 esp, reg32 eax);
Result musicStarted(App* app, CPU& cpu, reg32 esp, int32_t tune, int32_t volume, int32_t own);
Result bankLoaded(App* app, CPU& cpu, reg32 esp, reg32 file, int32_t slot, reg32 flags, reg32 eax);
Result banksFreed(App* app, CPU& cpu, reg32 esp);

}  // namespace port
}  // namespace nfs3hp

#endif
