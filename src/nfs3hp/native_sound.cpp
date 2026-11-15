#include <nfs3hp.h>
#include "isnd_calls.h"
#include <lib/memmap.h>
#include <SDL3/SDL.h>
#include <atomic>
#include <cstring>
#include <initializer_list>
#include <mutex>

/* The game's sound driver, eacsnd.dll, native and straight into SDL.
 *
 * eacsnd is the iSNDdirect* half of EA's SND library: the game (nfs3.exe) does
 * all the mixing -- the samples, the engine, the music stream, volume, pan and
 * pitch -- and the DLL only feeds what it mixes to DirectSound.  Its exports go
 * to the game through GetProcAddress (iSNDloaddll, pointers at 0x9ef61c..), and
 * of them only four do anything here -- this file's, which disassemble_nfs3hp.py
 * binds to the exports of those names, eacsnd having no generated code:
 *
 * - iSNDdirectcaps: the formats the card plays.  Ours plays stereo 16-bit at
 *   every rate, no 3D buffers: 0x3e08, as eacsnd worked out from our
 *   DirectSound.
 * - iSNDdirectstart(flags, window): the rate and format the game picked from
 *   those (22050 Hz, stereo, 16-bit).
 * - iSNDdirectserve: called by the game's sound thread (sub_48ec40), woken by
 *   its timer (sub_48ed30) and inside its critical section [0xa09b04].  eacsnd
 *   read DirectSound's play cursor, worked out how far ahead to write and had
 *   the game mix that much into the buffer: the mixer, the first function
 *   iSNDdirectsetfunctions was given ([0xa3c104], 0x4e1a24, stdcall buffer and
 *   frames), with the game's 100 Hz tick ([0xa3c114], 0x4e1adc) between every
 *   hundredth of a second of frames -- the clock MAD movies and SNDstreampurge
 *   ([0xa09afc]) keep time by.
 * - iSNDdirectstop.
 *
 * Here serve has the game mix into a buffer of guest memory of our own and puts
 * that on an SDL audio stream, which converts it to the phone's rate: no
 * DirectSound, no ring buffer for SDL's thread to copy out of.  How far ahead
 * it keeps the stream is eacsnd's 20 ms to begin with, or two of the device's
 * buffers where those are longer, raised by what the device found missing
 * whenever it ran dry -- serve on time, or late by a pause the game's sound
 * thread takes now and then, which the stream then has to bridge.
 *
 * Each is here as C++ with its words and its result (port::eacsnd,
 * isnd_calls.h), what native code calls through the game's pointer to it, and
 * as the export, a thin wrapper of it.  The mixer and the tick are called
 * through eacsnd's pointers to them, guest::sound: as C++ when they lead to
 * native_game.cpp's natives.
 *
 * eacsnd's twelve other exports -- the version, the game's functions,
 * recording, the 3D buffers -- are native in port_snd_isnd.cpp.  With no 3D
 * buffers (caps) the game creates none, and vol, rate and pos3d only store what
 * they are given while eacsnd's DirectSound flag [0xa3c0b0] is 0, which nothing
 * here sets. */

namespace nfs3hp
{

namespace
{

constexpr x86::reg32 kCaps = isnd::kCapsKept;          // iSNDdirectcaps's answer, kept; -1 until asked
constexpr x86::reg32 kCapsValue = isnd::kCaps16BitStereo | isnd::kCapsAnyRate;  // 0x3e08: stereo 16-bit at every rate
constexpr x86::reg32 kMixFrames = 0x4000;      // the most mixed at once: 0.37 s at 44100 Hz

x86::reg32 get(win32::WinApplication* app, x86::reg32 address)
{
    return app->getMemory<x86::reg32>(address);
}

/* A stdcall export's return: the result in eax, the return address and the
 * arguments off the stack. */
void returned(x86::CPU& cpu, x86::reg32 result, x86::reg32 argumentBytes)
{
    cpu.eax = result;
    cpu.esp += 4 + argumentBytes;
}

struct Sound
{
    std::mutex lock;                    // the device and stream, against start and stop on other threads
    SDL_AudioDeviceID device = 0;
    SDL_AudioStream* stream = nullptr;
    SDL_AudioSpec format{};
    bool started = false;
    x86::reg32 frameBytes = 4;
    x86::reg32 rate = 22050;

    win32::MemMap* mixMap = nullptr;    // the guest memory the game mixes into

    /* How far ahead the stream is kept, in frames, and what the device found
     * missing since serve last looked, in bytes (SDL's thread adds to it). */
    x86::reg32 ahead = 0;
    x86::reg32 aheadLeast = 0;
    x86::reg32 aheadMost = 0;
    x86::reg32 aheadLateMost = 0;     // the most a late serve may raise it to
    x86::reg32 deviceFrames = 0;      // the device's buffer, in the game's frames
    std::atomic<int> missing{0};
    Uint64 lastServe = 0;

    /* eacsnd's count to the next tick (sub_a33910): frames left before it,
     * ticks since start, frames those ticks stand for. */
    int toTick = 0;
    x86::reg32 ticks = 0;
    x86::reg32 tickFrames = 0;
};

Sound& sound()
{
    static Sound* s = new Sound;
    return *s;
}

/* On SDL's audio thread, with the stream locked: what it wanted beyond what was
 * queued is what ran dry. */
void SDLCALL streamWanted(void* userdata, SDL_AudioStream*, int additional, int)
{
    Sound* s = static_cast<Sound*>(userdata);
    if (additional > 0)
        s->missing.fetch_add(additional, std::memory_order_relaxed);
}

void closeDevice(Sound& s)
{
    if (s.stream)
        SDL_DestroyAudioStream(s.stream);
    if (s.device)
        SDL_CloseAudioDevice(s.device);
    s.stream = nullptr;
    s.device = 0;
}

/* The device at the phone's own rate (NFS_AUDIO_RATE, from AudioManager) and a
 * stream from the game's format to it, as AudioDevice opens them: a device at
 * any other rate is resampled by Android, off its low-latency path. */
bool openDevice(Sound& s, const SDL_AudioSpec& game)
{
    if (s.stream && s.format.format == game.format && s.format.channels == game.channels
        && s.format.freq == game.freq)
        return true;
    closeDevice(s);

    SDL_AudioSpec output = game;
    output.format = SDL_AUDIO_S16;
    output.channels = 2;
    const char* nativeRate = SDL_getenv("NFS_AUDIO_RATE");
    const int phoneRate = nativeRate ? SDL_atoi(nativeRate) : 0;
    if (phoneRate >= 8000 && phoneRate <= 192000)
        output.freq = phoneRate;
    s.device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &output);
    if (!s.device)
    {
        SDL_Log("[SND] no output: %s", SDL_GetError());
        return false;
    }
    SDL_PauseAudioDevice(s.device);
    s.stream = SDL_CreateAudioStream(&game, &output);
    if (!s.stream || !SDL_SetAudioStreamGetCallback(s.stream, &streamWanted, &s)
        || !SDL_BindAudioStream(s.device, s.stream))
    {
        SDL_Log("[SND] no stream: %s", SDL_GetError());
        closeDevice(s);
        return false;
    }
    s.format = game;
    SDL_AudioSpec opened;
    int frames = 0;
    s.deviceFrames = 0;
    if (SDL_GetAudioDeviceFormat(s.device, &opened, &frames) && frames > 0 && opened.freq > 0)
        s.deviceFrames = x86::reg32(uint64_t(frames) * x86::reg32(game.freq) / x86::reg32(opened.freq));
    if (frames > 0)
        SDL_Log("[SND] native: the game's %d Hz %d-bit %s to %d Hz, %d frames a buffer", game.freq,
                int(SDL_AUDIO_BITSIZE(game.format)), game.channels == 2 ? "stereo" : "mono", opened.freq,
                frames);
    return true;
}

}  // namespace

namespace port
{
namespace eacsnd
{

/* iSNDdirectcaps(window): the formats, as eacsnd's sub_a32c28 found them on
 * our DirectSound, kept where it kept them. */
reg32 caps(App* app, CPU&, reg32)
{
    app->getMemory<x86::reg32>(kCaps) = kCapsValue;
    return kCapsValue;
}

/* iSNDdirectstart(flags, window): the rate in the second byte of the flags,
 * the format in the first, as sub_a33150 reads them.  0, or -1 for none or
 * no device. */
reg32 start(App*, CPU&, reg32 flags, reg32)
{
    x86::reg32 rate = 0;
    if (flags & isnd::kCaps22050)
        rate = 22050;
    else if (flags & isnd::kCaps16000)
        rate = 16000;
    else if (flags & isnd::kCaps32000)
        rate = 32000;
    else if (flags & isnd::kCaps44100)
        rate = 44100;
    else if (flags & isnd::kCaps11025)
        rate = 11025;
    SDL_AudioSpec game{};
    if (flags & isnd::kCaps16BitStereo)
        game = {SDL_AUDIO_S16, 2, 0};
    else if (flags & isnd::kCaps8BitStereo)
        game = {SDL_AUDIO_U8, 2, 0};
    else if (flags & isnd::kCaps16BitMono)
        game = {SDL_AUDIO_S16, 1, 0};
    else if (flags & isnd::kCaps8BitMono)
        game = {SDL_AUDIO_U8, 1, 0};
    game.freq = int(rate);
    if (rate == 0 || game.channels == 0)
    {
        SDL_Log("[SND] start: no rate or format in 0x%x", flags);
        return x86::reg32(-1);
    }

    Sound& s = sound();
    std::lock_guard<std::mutex> guard(s.lock);
    if (!openDevice(s, game))
        return x86::reg32(-1);
    if (!s.mixMap)
        s.mixMap = new win32::MemMap(kMixFrames * 4);
    SDL_ClearAudioStream(s.stream);
    s.rate = rate;
    s.frameBytes = x86::reg32(SDL_AUDIO_BYTESIZE(game.format) * game.channels);
    /* eacsnd's write-ahead on a card that is not emulated: 20 ms, in whole
     * 16-frame steps -- or two of the device's buffers, where a device asks
     * for more at a time than that and would run dry on every one of them;
     * the most, a quarter of a second, and a late serve's 120 ms. */
    s.aheadMost = (rate / 4) & ~0xfu;
    s.aheadLateMost = (rate * 120 / 1000) & ~0xfu;
    s.aheadLeast = (rate * 20 / 1000) & ~0xfu;
    const x86::reg32 twoBuffers = (2 * s.deviceFrames + 0xf) & ~0xfu;
    if (twoBuffers > s.aheadLeast)
        s.aheadLeast = twoBuffers < s.aheadMost ? twoBuffers : s.aheadMost;
    if (s.ahead < s.aheadLeast || s.ahead > s.aheadMost)
        s.ahead = s.aheadLeast;
    SDL_Log("[SND] write-ahead from %u frames (%u ms), the device's buffer %u", s.ahead, s.ahead * 1000 / rate,
            s.deviceFrames);
    s.missing.store(0, std::memory_order_relaxed);
    s.lastServe = 0;
    s.toTick = 0;
    s.ticks = 0;
    s.tickFrames = 0;
    s.started = true;
    SDL_ResumeAudioDevice(s.device);
    return 0;
}

/* iSNDdirectstop().  The device stays open for the next start.  0. */
reg32 stop(App*, CPU&)
{
    Sound& s = sound();
    std::lock_guard<std::mutex> guard(s.lock);
    s.started = false;
    if (s.device)
        SDL_PauseAudioDevice(s.device);
    if (s.stream)
        SDL_ClearAudioStream(s.stream);
    return 0;
}

}  // namespace eacsnd
}  // namespace port

namespace
{

/* The game mixes frames into buffer, with its tick between every hundredth of
 * a second of them, as sub_a33910 has it do: the frames to the next tick worked
 * out from the ticks so far, so none drift.  Both through eacsnd's pointers to
 * them (guest::sound).  False when the game stopped in one. */
bool mixFrames(win32::WinApplication* app, x86::CPU& cpu, Sound& s, x86::reg32 buffer, x86::reg32 frames)
{
    while (frames > 0)
    {
        if (s.toTick <= 0)
        {
            ++s.ticks;
            if (!guest::sound<isnd::kFunctionTick, &tickSoundOf>(app, cpu))
                return false;
            const x86::reg32 due = x86::reg32(uint64_t(s.ticks) * s.rate / 100) - s.tickFrames;
            s.toTick = int(due & 0xffffff0);
            s.tickFrames += x86::reg32(s.toTick);
            if (s.ticks > 30000)
            {
                s.ticks = 0;
                s.tickFrames = 0;
            }
            if (s.toTick <= 0)
                continue;
        }
        const x86::reg32 now = frames < x86::reg32(s.toTick) ? frames : x86::reg32(s.toTick);
        s.toTick -= int(now);
        if (!guest::sound<isnd::kFunctionMix, &acquireSoundDataOf>(app, cpu, buffer, now))
            return false;
        frames -= now;
        buffer += now * s.frameBytes;
    }
    return true;
}

}  // namespace

namespace port
{
namespace eacsnd
{

/* iSNDdirectserve(): the stream topped up to its write-ahead.  The
 * game's sound thread comes every 10 to 16 ms; one that ran dry raises the
 * write-ahead by what was missing.  Late by more than 60 ms, it paused: the
 * write-ahead has to bridge such a pause if it comes again, so it is raised to
 * the pause's length, up to 120 ms.  Late by more than 150 ms, the game was
 * busy elsewhere -- a race loading -- and that leaves it be, or one load would
 * keep the sound that much behind for good.  0, or nullopt when the game
 * stopped in its mixer or its tick. */
std::optional<reg32> serve(App* app, CPU& cpu)
{
    Sound& s = sound();
    SDL_AudioStream* stream;
    {
        std::lock_guard<std::mutex> guard(s.lock);
        if (!s.started || !s.stream || !s.mixMap)
            return 0;
        stream = s.stream;
    }

    const Uint64 now = SDL_GetTicksNS();
    const Uint64 gap = s.lastServe == 0 ? 0 : now - s.lastServe;
    // The first serve finds it empty; a stall is none of the write-ahead's.
    const bool stalled = s.lastServe == 0 || gap > SDL_NS_PER_MS * 150;
    const bool late = gap > SDL_NS_PER_MS * 60;
    s.lastServe = now;
    const int missing = s.missing.exchange(0, std::memory_order_relaxed);
    const x86::reg32 most = late ? s.aheadLateMost : s.aheadMost;
    if (missing > 0 && !stalled && s.ahead < most)
    {
        x86::reg32 wanted = s.ahead + x86::reg32(missing) / s.frameBytes;
        if (late)
        {
            const x86::reg32 pause = x86::reg32(gap * s.rate / SDL_NS_PER_SECOND);
            if (pause > wanted)
                wanted = pause;
        }
        wanted = (wanted + 0xf) & ~0xfu;
        s.ahead = wanted < most ? wanted : most;
        SDL_Log("[SND] the device ran dry by %d frames%s: %u frames ahead (%u ms) from now",
                missing / int(s.frameBytes), late ? " after a pause" : "", s.ahead, s.ahead * 1000 / s.rate);
    }

    const int queuedBytes = SDL_GetAudioStreamQueued(stream);
    const x86::reg32 queued = queuedBytes > 0 ? x86::reg32(queuedBytes) / s.frameBytes : 0;
    if (queued >= s.ahead)
        return 0;
    x86::reg32 frames = (s.ahead - queued) & ~0xfu;
    if (frames > kMixFrames)
        frames = kMixFrames;
    if (frames == 0)
        return 0;

    const x86::reg32 buffer = s.mixMap->getBlockStart();
    std::memset(&app->getMemory<x86::reg8>(buffer), 0, frames * s.frameBytes);
    if (!mixFrames(app, cpu, s, buffer, frames))
        return std::nullopt;

    std::lock_guard<std::mutex> guard(s.lock);
    if (s.started && s.stream == stream)
        SDL_PutAudioStreamData(stream, &app->getMemory<x86::reg8>(buffer), int(frames * s.frameBytes));
    return 0;
}

}  // namespace eacsnd
}  // namespace port

/* eacsnd's exports of these names (disassemble_nfs3hp.py): each a thin
 * wrapper of its typed function, the words off the stack and popped; the
 * registers as the typed function leaves them. */
void iSNDdirectcaps(win32::WinApplication* app, x86::CPU& cpu)
{
    returned(cpu, port::eacsnd::caps(app, cpu, get(app, cpu.esp + 4)), 4);
}

void iSNDdirectstart(win32::WinApplication* app, x86::CPU& cpu)
{
    returned(cpu, port::eacsnd::start(app, cpu, get(app, cpu.esp + 4), get(app, cpu.esp + 8)), 8);
}

void iSNDdirectstop(win32::WinApplication* app, x86::CPU& cpu)
{
    returned(cpu, port::eacsnd::stop(app, cpu), 0);
}

void iSNDdirectserve(win32::WinApplication* app, x86::CPU& cpu)
{
    if (const std::optional<x86::reg32> result = port::eacsnd::serve(app, cpu))
        returned(cpu, *result, 0);
}

}
