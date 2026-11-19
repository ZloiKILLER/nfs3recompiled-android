#include "game_settings.h"
#include "diag_hooks.h"
#include "replay_trace.h"

#include <SDL3/SDL.h>
#include <string>
#include <sys/stat.h>

/* NFS_REPLAY_TRACE=1 (debug mode, Traces: replay_trace): every car, every step
 * of the physics, to logs/trace-race.bin or logs/trace-replay.bin in the game's
 * folder.  A replay is the race driven again from its recorded inputs, AI and
 * physics included, so the same replay on two builds must drive the same cars:
 * tools/replay_compare.py holds two such files against each other, and
 * tools/replay_run.py makes one on the phone.
 *
 * File: "NFTR", u32 version 1, u32 what (0 race, 1 replay), u32 cars; then for
 * each step of player one's car (carPhysics) a u32 race clock [0x7d3684] and for
 * each car its 13 words: track node +0x1c, place +0x98, speed +0xa4, nose +0xd8,
 * spin +0xe4 (floats as the game has them, three each). */
namespace nfs3hp
{
namespace
{

constexpr nfs::u32 kCars = 0x5e1120, kCarBytes = 0x9ac;
constexpr nfs::u32 kClock = 0x7d3684, kPlayerCars = 0x5efa48;

SDL_IOStream* s_file = nullptr;
const char* s_what = "";
nfs::u32 s_lastClock = 0, s_cars = 0, s_steps = 0;
bool s_passDone = false;

void closeTrace(nfs::App* app)
{
    if (!s_file)
        return;
    SDL_CloseIO(s_file);
    s_file = nullptr;
    SDL_Log("[TRACE] %s done: %u steps, %u cars, clock %u", s_what, unsigned(s_steps), unsigned(s_cars),
            unsigned(app->getMemory<nfs::u32>(kClock)));
}

void openTrace(nfs::App* app)
{
    const char* root = SDL_GetAndroidExternalStoragePath();
    if (!root)
        return;
    const bool replay = gameSettings(app).replayMode == 2;
    s_what = replay ? "replay" : "race";
    const std::string folder = std::string(root) + "/logs";
    mkdir(folder.c_str(), 0770);
    const std::string path = folder + "/trace-" + s_what + ".bin";
    s_file = SDL_IOFromFile(path.c_str(), "wb");
    if (!s_file)
    {
        SDL_Log("[TRACE] cannot write %s", path.c_str());
        return;
    }
    s_cars = gameSettings(app).entryCount;
    s_steps = 0;
    const nfs::u32 header[4] = {0x5254464e, 1, replay ? 1u : 0u, s_cars};
    SDL_WriteIO(s_file, header, sizeof header);
    SDL_Log("[TRACE] writing %s, %u cars", path.c_str(), unsigned(s_cars));
}

void carStepped(nfs::App* app, nfs::u32 car)
{
    const nfs::u32 player = gameSettings(app).localPlayer;
    if (player >= 16 || car != app->getMemory<nfs::u32>(kPlayerCars + player * 4))
        return;
    const nfs::u32 clock = app->getMemory<nfs::u32>(kClock);
    /* The clock beginning again: a race started over from the pause menu, whose
     * new start is what the file should hold, or a replay come to its end and
     * going round again, whose first pass it already has. */
    if (s_file && clock < s_lastClock)
    {
        s_passDone = gameSettings(app).replayMode == 2;
        closeTrace(app);
    }
    if (s_passDone)
        return;
    if (!s_file)
        openTrace(app);
    s_lastClock = clock;
    if (!s_file)
        return;
    SDL_WriteIO(s_file, &clock, 4);
    for (nfs::u32 i = 0; i < s_cars && i < 16; ++i)
    {
        const nfs::u32 c = kCars + i * kCarBytes;
        SDL_WriteIO(s_file, &app->getMemory<nfs::u8>(c + 0x1c), 4);
        SDL_WriteIO(s_file, &app->getMemory<nfs::u8>(c + 0x98), 12);
        SDL_WriteIO(s_file, &app->getMemory<nfs::u8>(c + 0xa4), 12);
        SDL_WriteIO(s_file, &app->getMemory<nfs::u8>(c + 0xd8), 12);
        SDL_WriteIO(s_file, &app->getMemory<nfs::u8>(c + 0xe4), 12);
    }
    if (++s_steps % 64 == 0)
        SDL_FlushIO(s_file);
}

}  // namespace

void startReplayTrace()
{
    diag::carStepped = carStepped;
}

void endReplayTrace(nfs::App* app)
{
    closeTrace(app);
    s_passDone = false;
}

}  // namespace nfs3hp
