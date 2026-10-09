#include <nfs3hp.h>
#include "native_thrash.h"
#include <lib/memmap.h>
#include <SDL3/SDL.h>
#include <atomic>
#include <cstring>
#include <initializer_list>
#include <mutex>
#include <string>
#include <vector>

/* The game's sound driver, eacsnd.dll, native and straight into SDL.
 *
 * eacsnd is the iSNDdirect* half of EA's SND library: the game (nfs3.exe) does
 * all the mixing -- the samples, the engine, the music stream, volume, pan and
 * pitch -- and the DLL only feeds what it mixes to DirectSound.  Its exports go
 * to the game through GetProcAddress (sub_4e1dd4, pointers at 0x9ef61c..), and
 * of them only four do anything here:
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
 * it keeps the stream is eacsnd's 20 ms to begin with, raised by what the
 * device found missing whenever it ran dry while serve came on time.
 *
 * The 3D functions are left generated: with no 3D buffers (caps) the game
 * creates none, and vol, rate and pos3d only store what they are given while
 * eacsnd's DirectSound flag [0xa3c0b0] is 0, which nothing here sets.
 *
 * NFS_SND_NATIVE=0 (launch extra snd_native false) leaves eacsnd's generated
 * code to all of it, as does NFS_NATIVES=0. */

namespace nfs3hp
{

namespace
{

constexpr x86::reg32 kCaps = 0xa3c0fc;         // iSNDdirectcaps's answer, kept; -1 until asked
constexpr x86::reg32 kMixer = 0xa3c104;        // the game's mixer: stdcall (buffer, frames)
constexpr x86::reg32 kTick = 0xa3c114;         // the game's 100 Hz tick: no arguments
constexpr x86::reg32 kCapsValue = 0x3e08;      // stereo 16-bit; 11025, 16000, 22050, 32000, 44100 Hz
constexpr x86::reg32 kMixFrames = 0x4000;      // the most mixed at once: 0.37 s at 44100 Hz

bool soundOn()
{
    static const bool on = []() {
        const char* value = SDL_getenv("NFS_SND_NATIVE");
        return !value || SDL_strcmp(value, "0") != 0;
    }();
    return on && nativesEnabled();
}

x86::reg32 get(win32::WinApplication* app, x86::reg32 address)
{
    return app->getMemory<x86::reg32>(address);
}

x86::reg32 stdcall(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 address,
                   std::initializer_list<x86::reg32> arguments)
{
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 first = esp - 4 * x86::reg32(arguments.size());
    x86::reg32 slot = first;
    for (const x86::reg32 argument : arguments)
    {
        app->getMemory<x86::reg32>(slot) = argument;
        slot += 4;
    }
    cpu.esp = first - 4;
    app->dynamic_call(address, cpu);
    cpu.esp = esp;
    return cpu.eax;
}

/* A stdcall export's return: the result in eax, the return address and the
 * arguments off the stack. */
bool returned(x86::CPU& cpu, x86::reg32 result, x86::reg32 argumentBytes)
{
    cpu.eax = result;
    cpu.esp += 4 + argumentBytes;
    return true;
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
    if (SDL_GetAudioDeviceFormat(s.device, &opened, &frames))
        SDL_Log("[SND] native: the game's %d Hz %d-bit %s to %d Hz, %d frames a buffer", game.freq,
                int(SDL_AUDIO_BITSIZE(game.format)), game.channels == 2 ? "stereo" : "mono", opened.freq,
                frames);
    return true;
}

/* iSNDdirectcaps(window), stdcall: the formats, as eacsnd's sub_a32c28 found them
 * on our DirectSound, kept where it kept them. */
bool capsNative(win32::WinApplication* app, x86::CPU& cpu)
{
    app->getMemory<x86::reg32>(kCaps) = kCapsValue;
    return returned(cpu, kCapsValue, 4);
}

/* iSNDdirectstart(flags, window), stdcall: the rate in the second byte of the
 * flags, the format in the first, as sub_a33150 reads them. */
bool startNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 flags = get(app, cpu.esp + 4);
    x86::reg32 rate = 0;
    if (flags & 0x800)
        rate = 22050;
    else if (flags & 0x400)
        rate = 16000;
    else if (flags & 0x1000)
        rate = 32000;
    else if (flags & 0x2000)
        rate = 44100;
    else if (flags & 0x200)
        rate = 11025;
    SDL_AudioSpec game{};
    if (flags & 8)
        game = {SDL_AUDIO_S16, 2, 0};
    else if (flags & 2)
        game = {SDL_AUDIO_U8, 2, 0};
    else if (flags & 4)
        game = {SDL_AUDIO_S16, 1, 0};
    else if (flags & 1)
        game = {SDL_AUDIO_U8, 1, 0};
    game.freq = int(rate);
    if (rate == 0 || game.channels == 0)
    {
        SDL_Log("[SND] start: no rate or format in 0x%x", flags);
        return returned(cpu, x86::reg32(-1), 8);
    }

    Sound& s = sound();
    std::lock_guard<std::mutex> guard(s.lock);
    if (!openDevice(s, game))
        return returned(cpu, x86::reg32(-1), 8);
    if (!s.mixMap)
        s.mixMap = new win32::MemMap(kMixFrames * 4);
    SDL_ClearAudioStream(s.stream);
    s.rate = rate;
    s.frameBytes = x86::reg32(SDL_AUDIO_BYTESIZE(game.format) * game.channels);
    /* eacsnd's write-ahead on a card that is not emulated: 20 ms, in whole
     * 16-frame steps; the most, a quarter of a second. */
    s.aheadLeast = (rate * 20 / 1000) & ~0xfu;
    s.aheadMost = (rate / 4) & ~0xfu;
    if (s.ahead < s.aheadLeast || s.ahead > s.aheadMost)
        s.ahead = s.aheadLeast;
    s.missing.store(0, std::memory_order_relaxed);
    s.lastServe = 0;
    s.toTick = 0;
    s.ticks = 0;
    s.tickFrames = 0;
    s.started = true;
    SDL_ResumeAudioDevice(s.device);
    return returned(cpu, 0, 8);
}

/* iSNDdirectstop(), stdcall.  The device stays open for the next start. */
bool stopNative(win32::WinApplication*, x86::CPU& cpu)
{
    Sound& s = sound();
    std::lock_guard<std::mutex> guard(s.lock);
    s.started = false;
    if (s.device)
        SDL_PauseAudioDevice(s.device);
    if (s.stream)
        SDL_ClearAudioStream(s.stream);
    return returned(cpu, 0, 0);
}

/* The game mixes frames into buffer, with its tick between every hundredth of
 * a second of them, as sub_a33910 has it do: the frames to the next tick worked
 * out from the ticks so far, so none drift. */
void mix(win32::WinApplication* app, x86::CPU& cpu, Sound& s, x86::reg32 buffer, x86::reg32 frames)
{
    while (frames > 0)
    {
        if (s.toTick <= 0)
        {
            ++s.ticks;
            stdcall(app, cpu, get(app, kTick), {});
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
        stdcall(app, cpu, get(app, kMixer), {buffer, now});
        frames -= now;
        buffer += now * s.frameBytes;
    }
}

/* NFS_SND_TRACE=1: the game's mixer as it runs -- its globals once, and each
 * set of functions a channel mixes through the first time it is seen -- to
 * know which of its many variants a native mixer has to stand in for. */
bool soundTracing()
{
    static const bool on = []() {
        const char* value = SDL_getenv("NFS_SND_TRACE");
        return value && SDL_strcmp(value, "1") == 0;
    }();
    return on;
}

void traceMixer(win32::WinApplication* app)
{
    static bool globals = false;
    static std::vector<std::string> seen;
    if (!globals)
    {
        globals = true;
        char line[512];
        int n = SDL_snprintf(line, sizeof line, "[SND] mixer: [9f6a5a] %u, [9f6a57] %d, block %u, out %x clear %x after %x before %x",
                             unsigned(app->getMemory<x86::reg8>(0x9f6a5a)),
                             int(app->getMemory<x86::reg8>(0x9f6a5a - 3)),
                             unsigned(app->getMemory<x86::reg16>(0x9f6a58)), get(app, 0x9f6b20),
                             get(app, 0x9f6a5c), get(app, 0x9f6a64), get(app, 0x9f6a68));
        NFS2_USE(n);
        SDL_Log("%s", line);
        std::string table = "[SND] decoders";
        for (x86::reg32 i = 0; i < 0x1c; ++i)
        {
            char item[16];
            SDL_snprintf(item, sizeof item, " %x", get(app, 0x9f6a6c + 4 * i));
            table += item;
        }
        SDL_Log("%s", table.c_str());
        table = "[SND] factories";
        for (x86::reg32 a = 0x9f6adc; a < 0x9f6b28; a += 4)
        {
            char item[16];
            SDL_snprintf(item, sizeof item, " %x", get(app, a));
            table += item;
        }
        SDL_Log("%s", table.c_str());
    }
    for (x86::reg32 c = 0; c < 16; ++c)
    {
        const x86::reg32 ch = 0x9fbcb4 + c * 0xde4;
        if (app->getMemory<x86::reg8>(ch) == 0)
            continue;
        char key[256];
        SDL_snprintf(key, sizeof key, "decode %x/%x resample %x/%x/%x/%x mix %x/%x vol %x/%x",
                     get(app, ch + 0xda8), get(app, ch + 0xdac), get(app, ch + 0xdb0), get(app, ch + 0xdb4),
                     get(app, ch + 0xdb8), get(app, ch + 0xd64), get(app, ch + 0xdc0), get(app, ch + 0xdbc),
                     get(app, ch + 0xddc), get(app, ch + 0xde0));
        bool known = false;
        for (const std::string& k : seen)
            known = known || k == key;
        if (known)
            continue;
        seen.push_back(key);
        char bytes[64];
        SDL_snprintf(bytes, sizeof bytes, "%02x %02x %02x %02x %02x %02x %02x %02x %02x",
                     app->getMemory<x86::reg8>(ch), app->getMemory<x86::reg8>(ch + 1),
                     app->getMemory<x86::reg8>(ch + 2), app->getMemory<x86::reg8>(ch + 3),
                     app->getMemory<x86::reg8>(ch + 4), app->getMemory<x86::reg8>(ch + 5),
                     app->getMemory<x86::reg8>(ch + 6), app->getMemory<x86::reg8>(ch + 7),
                     app->getMemory<x86::reg8>(ch + 8));
        SDL_Log("[SND] channel %u: %s; head %s", c, key, bytes);
    }
}

/* iSNDdirectserve(), stdcall: the stream topped up to its write-ahead.  A serve
 * more than 60 ms after the last found the stream dry because the game's thread
 * did not come, not because the write-ahead is short, and leaves it be. */
bool serveNative(win32::WinApplication* app, x86::CPU& cpu)
{
    Sound& s = sound();
    SDL_AudioStream* stream;
    {
        std::lock_guard<std::mutex> guard(s.lock);
        if (!s.started || !s.stream || !s.mixMap)
            return returned(cpu, 0, 0);
        stream = s.stream;
    }

    const Uint64 now = SDL_GetTicksNS();
    const bool late = s.lastServe == 0 || now - s.lastServe > SDL_NS_PER_MS * 60;  // the first finds it empty
    s.lastServe = now;
    const int missing = s.missing.exchange(0, std::memory_order_relaxed);
    if (missing > 0 && !late && s.ahead < s.aheadMost)
    {
        const x86::reg32 wanted = (s.ahead + x86::reg32(missing) / s.frameBytes + 0xf) & ~0xfu;
        s.ahead = wanted < s.aheadMost ? wanted : s.aheadMost;
        SDL_Log("[SND] the device ran dry by %d frames: %u frames ahead (%u ms) from now",
                missing / int(s.frameBytes), s.ahead, s.ahead * 1000 / s.rate);
    }

    const int queuedBytes = SDL_GetAudioStreamQueued(stream);
    const x86::reg32 queued = queuedBytes > 0 ? x86::reg32(queuedBytes) / s.frameBytes : 0;
    if (queued >= s.ahead)
        return returned(cpu, 0, 0);
    x86::reg32 frames = (s.ahead - queued) & ~0xfu;
    if (frames > kMixFrames)
        frames = kMixFrames;
    if (frames == 0)
        return returned(cpu, 0, 0);

    const x86::reg32 buffer = s.mixMap->getBlockStart();
    std::memset(&app->getMemory<x86::reg8>(buffer), 0, frames * s.frameBytes);
    mix(app, cpu, s, buffer, frames);
    if (soundTracing())
        traceMixer(app);

    std::lock_guard<std::mutex> guard(s.lock);
    if (s.started && s.stream == stream)
        SDL_PutAudioStreamData(stream, &app->getMemory<x86::reg8>(buffer), int(frames * s.frameBytes));
    return returned(cpu, 0, 0);
}

}

bool soundCaps(win32::WinApplication* app, x86::CPU& cpu)
{
    return soundOn() && capsNative(app, cpu);
}

bool soundStart(win32::WinApplication* app, x86::CPU& cpu)
{
    return soundOn() && startNative(app, cpu);
}

bool soundStop(win32::WinApplication* app, x86::CPU& cpu)
{
    return soundOn() && stopNative(app, cpu);
}

bool soundServe(win32::WinApplication* app, x86::CPU& cpu)
{
    return soundOn() && serveNative(app, cpu);
}

}
