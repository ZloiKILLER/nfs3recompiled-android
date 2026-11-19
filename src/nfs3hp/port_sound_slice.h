#ifndef NFS3HP_PORT_SOUND_SLICE_H
#define NFS3HP_PORT_SOUND_SLICE_H

#include "port_snd.h"
#include "port_sound_calls.h"

#include <cstdint>
#include <initializer_list>

/* What the game's music, banks, race set-up, player's car sound and pursuit
 * (port_sound_music.cpp, port_sound_banks.cpp, port_sound_racesetup.cpp,
 * port_sound_player.cpp, port_sound_pursuit.cpp) share, besides port_snd.h's
 * library: how they call the game's code that is not rewritten yet, the
 * game's strings, and the functions of one that another calls as C++ (each
 * is also the game's entry, a thunk under the game's name).
 *
 * The rewrites call each other, the library and the other files' typed
 * functions as C++ (guest::direct); what has no typed function yet is called
 * by its address, with the register arguments it takes. */
namespace nfs3hp
{

// nfs3hp_main.cpp's: the track's index file as the game opens it, and its own
// stream file in place of the copy's (the port's music stream change).
void musicIndexOpened(nfs::App* app, nfs::u32 path);
void musicStreamFile(nfs::App* app, nfs::u32 path);

namespace port
{

/* Where a rewrite calls the game's code from: `esp` as the game's code has it
 * at the call. */
class Game
{
public:
    App* app;
    ThreadState& ts;
    u32 esp;

    Game(App* a, ThreadState& c, u32 stack) : app(a), ts(c), esp(stack) {}
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    /* `target`, which has no typed function yet, called by its address with the
     * register arguments `a` (those it does not read 0) and `stack` pushed;
     * `eax` is set to what it gives back. */
    bool call(u32 target, Args a, std::initializer_list<u32> stack, u32& eax)
    {
        return pointerGame(app, ts, esp, target, a, stack, eax);
    }
    bool call(u32 target, Args a, u32& eax) { return call(target, a, {}, eax); }
    /* esp into the ts as the game's code has it at a call, for a typed core
     * called as C++ in its place (guest::direct). */
    void atCall() { ts.sp = esp; }
};

// port_sound_pursuit.cpp's beepsStarted (0x418930) and musicEventResumed (0x418c60) are port_sound_calls.h's.

/* A loop's safepoint, as the game's the game's code has one at each loop's
 * head: the guest's context handed over when another thread waits for it. */
inline void safepoint(App* app, ThreadState& ts)
{
    if (app->contextWanted())
        app->yieldContext(ts);
}

// The game's folders: strings, by their place.
constexpr u32 kAudioDir = 0x7a2a34, kTempDir = 0x7a2734;
constexpr u32 kFormatTwo = 0x536890;  // "%s%s"

// What the group calls of the game's that is not rewritten yet.
constexpr u32 kSprintf = 0x4df690, kFileExists = 0x4e0e60, kLoadFile = 0x4e0ec0, kPurgeMemory = 0x4e1890;
constexpr u32 kReserveMemory = 0x4e1620, kUnlink = 0x4e28a0, kReadFile = 0x4e9200, kSeekFile = 0x4e9088;
constexpr u32 kOpenArchive = 0x43b830, kCloseArchive = 0x43b9b0, kFindInArchive = 0x43b9d0;
constexpr u32 kPathInit = 0x4e6dc8, kPathStartStream = 0x4e6f0c, kPathControl = 0x4e705c, kPathEvent = 0x4e70fc;
constexpr u32 kPathEventInterrupt = 0x4e7208, kPathRestore = 0x4e7388;
constexpr u32 kStreamAutoVolume = 0x4e7470, kStreamGetVolume = 0x4e74f0, kRunSystemTasks = 0x4e7630;
constexpr u32 kStreamRestore = 0x4e784c, kStreamPurge = 0x4e7b28, kStreamMake = 0x4e4790, kStreamDelete = 0x4e4b50;
constexpr u32 kSoundRestore = 0x4e26b4;
constexpr u32 kEffectBank = 0x4182b0;  // port_sound_effects.cpp's bankEffect, as the game calls it

/* A typed core's result, called as C++ (guest::direct), as the game's code
 * has it in eax after the call: false when the game stopped in it. */
template <class T>
bool tookEax(const std::optional<T>& result, u32& eax)
{
    if (!result)
        return false;
    eax = u32(*result);
    return true;
}
inline bool tookEax(ThreadState& ts, u32 result, u32& eax)
{
    eax = result;
    return !ts.terminate;
}

/* A string of the game's, its length not counting the end. */
inline u32 textLength(App* app, u32 at)
{
    u32 n = 0;
    while (app->getMemory<uint8_t>(at + n) != 0)
        ++n;
    return n;
}

/* The game's inlined strcpy: `from` to `to`, two bytes a turn, its end too.
 * esi where its loop stopped reading (the code's own registers keep that). */
inline u32 copyText(App* app, u32 to, u32 from)
{
    u32 at = from;
    for (u32 i = 0;; i += 2)
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
inline u32 appendText(App* app, u32 to, u32 from, u32* esi = nullptr)
{
    const u32 length = textLength(app, to);
    const u32 stop = copyText(app, to + length, from);
    if (esi != nullptr)
        *esi = stop;
    return u32(-2 - int32_t(length));
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
Result musicStopped(App* app, ThreadState& ts, u32 esp, u32 eax);
Result musicStarted(App* app, ThreadState& ts, u32 esp, int32_t tune, int32_t volume, int32_t own);
Result bankLoaded(App* app, ThreadState& ts, u32 esp, u32 file, int32_t slot, u32 flags, u32 eax);
Result banksFreed(App* app, ThreadState& ts, u32 esp);

}  // namespace port
}  // namespace nfs3hp

#endif
