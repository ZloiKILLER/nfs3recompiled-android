#include <nfs3hp.h>
#include "native_ea_memory.h"
#include "native_frontend.h"
#include "native_frontend_eab.h"
#include "native_frontend_mova.h"
#include "native_frontend_wina.h"
#include "native_game.h"
#include "native_guest.h"
#include "native_thrash.h"
#include "port_snd.h"
#include "port_snd_stream_calls.h"
#include "port_text.h"
#include <lib/renderer.h>
#include <SDL3/SDL.h>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <initializer_list>
#include <optional>
#include <type_traits>
#include <vector>

extern "C" {
#include <mad_decoder.h>
#include <mad_player.h>
}

/* The movies native: EA's MAD player, decoder and display of NFS III, which
 * decomp/mad has decompiled to C and checked against nfs3.exe bit for bit,
 * wired into the game here.
 *
 * - sub_495bc0 (showmad), the player: mad_player_play, the decoder called
 *   straight from it.  Everything it asks of the rest of the game -- EA's
 *   stream and sound libraries, memory, the clock, the keyboard, the display --
 *   goes through MadPlayerHost to the game's own functions, as the original
 *   calls them.  The pointers it hands around are the game's, as host
 *   pointers into its memory.  Its waits let the game's other threads run.
 * - sub_4df2b0, the frame on the screen: not converted to the 16-bit surface
 *   by sub_4fdc90, but to full colour here and handed to the renderer, which
 *   scales it onto its place on the GPU (Renderer::presentMovieFrame). */

namespace nfs3hp
{

// nfs3hp_main.cpp: a tap on the screen asks the movie to end.
bool movieSkipRequested();
void moviePlaying(bool playing);

namespace
{

using u8 = uint8_t;
using u32 = uint32_t;
using i32 = int32_t;

/* The player, with the decoder in it, shared by the native player and the
 * decoder's own entries. */
MadPlayer& player()
{
    static MadPlayer* p = []() {
        MadPlayer* made = new MadPlayer;
        std::memset(made, 0, sizeof *made);
        return made;
    }();
    return *p;
}

u8* memoryOf(win32::WinApplication* app)
{
    return reinterpret_cast<u8*>(&app->getMemory<x86::reg8>(0));
}

u32 r32(const u8* m, u32 at)
{
    u32 v;
    std::memcpy(&v, m + at, 4);
    return v;
}

i32 s32(const u8* m, u32 at)
{
    return i32(r32(m, at));
}

void w32(u8* m, u32 at, u32 v)
{
    std::memcpy(m + at, &v, 4);
}

// ------------------------------------------------------------------ display

/* sub_4df2b0's frame in full colour: the packed 7-bit 4:2:2 the decoder writes
 * (bytes Cr, Cb, Y of the right pixel, Y of the left) through BT.601 as the
 * game's tables have it (sub_4fda20) -- y doubled, chroma about 64 -- but to
 * eight bits a channel, without the 5/6-bit rounding and dither. */
std::vector<u8> fullColour(const u8* frame, i32 w, i32 h)
{
    std::vector<u8> rgba(size_t(w) * size_t(h) * 4);
    u8* out = rgba.data();
    auto channel = [](float v) {
        const float c = v < 0.f ? 0.f : v > 255.f ? 255.f : v;
        return u8(c + 0.5f);
    };
    const i32 pairs = w / 2 * h;
    for (i32 i = 0; i < pairs; ++i)
    {
        const u8* word = frame + size_t(i) * 4;
        const float cr = float(word[0]) - 64.f + 0.5f;
        const float cb = float(word[1]) - 64.f + 0.5f;
        const float r = 2.f * (1.402f * cr);
        const float g = 2.f * (-0.3441f * cb - 0.7141f * cr);
        const float b = 2.f * (1.772f * cb);
        const u8 ys[2] = {word[3], word[2]};  // left, right
        for (int k = 0; k < 2; ++k)
        {
            const float y = 2.f * float(ys[k]) + 1.f;
            out[0] = channel(y + r);
            out[1] = channel(y + g);
            out[2] = channel(y + b);
            out[3] = 0xff;
            out += 4;
        }
    }
    return rgba;
}

/* What sub_4df1f0 kept of the display it opened. */
constexpr u32 kDisplayMode = 0x8ca2d4;  // 2: the software path, the one the player always takes
constexpr u32 kDisplayX = 0x8ca2e0, kDisplayY = 0x8ca2dc, kDisplayW = 0x8ca2d0, kDisplayH = 0x8ca2c4;
constexpr u32 kDisplayQuality = 0x8ca2cc;  // 0: 1x; 1-3: 2x

bool showFrame(win32::WinApplication* app, u32 frame)
{
    const u8* m = memoryOf(app);
    if (s32(m, kDisplayMode) != 2)
        return false;
    win32::Renderer* renderer = win32::Renderer::active();
    if (!renderer)
        return false;
    const i32 x = s32(m, kDisplayX), y = s32(m, kDisplayY);
    const i32 w = s32(m, kDisplayW), h = s32(m, kDisplayH);
    const i32 quality = s32(m, kDisplayQuality);
    if (w < 2 || h < 1 || w > 1024 || h > 1024 || quality < 0 || quality > 3)
        return false;
    const i32 scale = quality == 0 ? 1 : 2;
    renderer->presentMovieFrame(fullColour(m + frame, w & ~1, h), x86::reg32(w & ~1), x86::reg32(h), x & ~1, y,
                                (w & ~1) * scale, h * scale);
    return true;
}

// ------------------------------------------------------------ the player

/* SNDplaysetdef (0x4e9b20, port_snd_voices.cpp's thunk) as showmad calls
 * it: the play's defaults into the block at `at` (never 0 here, so never its
 * error). */
void playDefaultsAt(win32::WinApplication* app, x86::reg32 at)
{
    port::playDefaults(port::record<port::PlayOptions>(app, at));
}

}  // namespace

namespace guest
{

}  // namespace guest

namespace
{

/* EA's memory, streams and sound, the clock, the keyboard and the display, as
 * showmad calls them: the game's functions' typed cores, called as C++ with
 * what each takes, the frames they make on the guest's stack below the
 * player's own (as the original's calls, decomp/mad/tests/diff_player.py). */
struct Game
{
    win32::WinApplication* app;
    x86::CPU& cpu;
    u8* m;
    u32 scratch;  // guest memory of the native player's own frame
    bool dead = false;
    u32 lastTicks = 0;

    static constexpr u32 kParams = 0x00;  // SNDplaysetdef's block, for as long as the sound plays
    static constexpr u32 kStatus = 0x80;  // SNDSTRM_requeststatus's four
    static constexpr u32 kMessage = 0xa0; // a fatal error's text
    static constexpr u32 kScratchSize = 0x200;

    u32 g(const void* p) const { return p ? u32(static_cast<const u8*>(p) - m) : 0; }
    void* h(u32 at) const { return at ? m + at : nullptr; }

    /* A function of the game's with a typed core, called as C++ (guest::direct) with what it takes,
     * every register kept around it (guest::direct keeps them); its result as the game's eax, 0 once
     * the game stopped. */
    static u32 valueOf(x86::reg32 v) { return v; }
    template <typename T>
    static u32 valueOf(const std::optional<T>& v)
    {
        return v ? u32(*v) : 0;
    }

    template <x86::reg32 Address, auto Core, typename... Args>
    u32 direct(Args... args)
    {
        if (dead)
            return 0;
        u32 result = 0;
        try
        {
            if constexpr (std::is_void_v<decltype(guest::direct<Address, Core>(app, cpu, args...))>)
                guest::direct<Address, Core>(app, cpu, args...);
            else
                result = valueOf(guest::direct<Address, Core>(app, cpu, args...));
        }
        catch (const fe::Terminated&)
        {
            dead = true;
        }
        if (cpu.terminate)
            dead = true;
        return result;
    }

    /* The waits of the original are loops, where the game's other threads
     * get their turn (safepoints); these are the same waits. */
    void letOthersRun()
    {
        if (app->contextWanted())
            app->yieldContext(cpu);
    }

    /* A tap on the screen ends the movie, as a key would. */
    bool skipping() const { return movieSkipRequested(); }
};

Game& game(void* user)
{
    return *static_cast<Game*>(user);
}

void* hostAlloc(void* u, uint32_t size)
{
    Game& G = game(u);
    return G.h(G.direct<0x4e1620, ea::reserved>(0x53be88u, size, r32(G.m, 0x5643f4)));
}

void hostRelease(void* u, void* block)
{
    Game& G = game(u);
    G.direct<0x4e1890, ea::purgeBlock>(G.g(block));
}

void* hostStreamCreate(void* u, void* buffer, uint32_t size)
{
    Game& G = game(u);
    return G.h(G.direct<0x4cfa80, frontend::wina::streamCreate>(2u, 2u, 2u, G.g(buffer), size));
}

void* hostStreamChannel(void* u, void* stream)
{
    Game& G = game(u);
    return G.h(G.direct<0x4cfe70, frontend::wina::streamChannel>(G.g(stream), 2u));
}

/* sub_4cfca0 takes four registers and the stack: channel 1, the sound's,
 * gets the tag 'SC' (0x4353) under the mask 0xffff, which showmad leaves in
 * ecx and ebx from sub_4cfe70 (it keeps them) rather than setting again --
 * so the decompiled call shows two arguments.  Without them that channel
 * takes every chunk and the movie's channel waits for ever. */
void hostStreamSetup(void* u, void* stream)
{
    Game& G = game(u);
    G.direct<0x4cfca0, frontend::wina::streamSetRule>(G.g(stream), 1u, 0xffffu, 0x4353u, 2u);
}

void* hostStreamOpen(void* u, void* stream, const char* path)
{
    Game& G = game(u);
    return G.h(G.direct<0x4cfef0, frontend::wina::streamOpenFile>(G.g(stream), G.g(path), 0u, 0u));
}

const uint8_t* hostStreamNext(void* u, void* channel)
{
    Game& G = game(u);
    G.letOthersRun();
    return static_cast<const uint8_t*>(G.h(G.direct<0x4d0300, frontend::wina::streamNextChunk>(G.g(channel))));
}

void hostStreamRelease(void* u, void* channel, const uint8_t* chunk)
{
    Game& G = game(u);
    G.direct<0x4d03c0, frontend::wina::streamFreeChunk>(G.g(channel), G.g(chunk));
}

int32_t hostStreamEnded(void* u, void* channel)
{
    Game& G = game(u);
    G.letOthersRun();
    const i32 ended = i32(G.direct<0x4d04c0, frontend::wina::channelDone>(G.g(channel)));
    return G.dead ? 1 : ended;
}

int32_t hostStreamPending(void* u, void* channel)
{
    Game& G = game(u);
    return i32(G.direct<0x4d0460, frontend::wina::channelBytes>(G.g(channel)));
}

void hostStreamClose(void* u, void* stream)
{
    Game& G = game(u);
    G.direct<0x4cfd30, frontend::wina::streamClose>(G.g(stream));
}

void hostSoundDefaults(void* u, void* params, int32_t count)
{
    Game& G = game(u);
    static_cast<void>(count);  // in edx, which SNDplaysetdef keeps for the call after
    G.direct<0x4e9b20, playDefaultsAt>(G.scratch + Game::kParams);
    std::memcpy(params, G.m + G.scratch + Game::kParams, 0x20);
}

/* sub_4f3a10 takes edx too: 0x1e, which showmad leaves there from
 * SNDplaysetdef (sub_4e9b20 keeps it). */
uint32_t hostSoundMemory(void* u, int32_t streams)
{
    Game& G = game(u);
    return G.direct<0x4f3a10, port::stream::streamMemoryOf>(u32(streams), 0x1eu);
}

/* The block SNDplaysetdef filled stays where it was filled: the sound
 * player may keep pointing at it, as at the original's stack frame. */
void* hostSoundCreate(void* u, void* channel, void* params, int32_t streams, int32_t count, void* memory,
                      uint32_t size)
{
    Game& G = game(u);
    std::memcpy(G.m + G.scratch + Game::kParams, params, 0x20);
    // stdcall: the memory and its size pushed.
    return G.h(G.direct<0x4f3a60, port::stream::createQueued>(G.cpu.esp - 4 - 2 * 4, G.g(channel),
                                                                  G.scratch + Game::kParams, u32(streams), u32(count),
                                                                  G.g(memory), size));
}

int32_t hostSoundStart(void* u, void* sound, void* file)
{
    Game& G = game(u);
    return i32(G.direct<0x4f3a80, port::stream::requestData>(G.cpu.esp - 4, G.g(sound), 0u, G.g(file)));
}

void hostSoundStatus(void* u, int32_t handle, int32_t status[4])
{
    Game& G = game(u);
    G.letOthersRun();
    G.direct<0x4f2f50, port::stream::requestStatusOf>(G.cpu.esp - 4, u32(handle), G.scratch + Game::kStatus);
    std::memcpy(status, G.m + G.scratch + Game::kStatus, 16);
    if (G.dead || G.skipping())
        status[0] = 3;
}

void hostSoundDestroy(void* u, void* sound)
{
    Game& G = game(u);
    G.direct<0x4f43c0, port::stream::destroyStreamOf>(G.cpu.esp - 4, i32(G.g(sound)));
}

/* After a tap or once the game is closing, time runs out at once: the
 * frames still queued and the sound are not waited for. */
uint32_t hostTicks(void* u)
{
    Game& G = game(u);
    G.letOthersRun();
    const u32 now = G.direct<0x4f2790, frontend::eab::lowTimerTicks>();
    if (G.dead || G.skipping())
        return G.lastTicks += 0x10000000;
    return G.lastTicks = now;
}

void hostPump(void* u)
{
    Game& G = game(u);
    G.direct<0x4e7630, port::stream::runDueTasks>(G.cpu.esp - 4, 0u);
}

int32_t hostDisplayMode(void* u)
{
    Game& G = game(u);
    return i32(G.direct<0x4df340, frontend::mova::displayMode>());
}

int32_t hostKeyPressed(void* u)
{
    Game& G = game(u);
    const i32 pressed = i32(G.direct<0x451960, port::text::menuKeyOf>(G.cpu.esp - 4, 1u));
    return G.dead || G.skipping() ? 1 : pressed;
}

void hostDisplayOpen(void* u, int32_t x, int32_t y, int32_t w, int32_t hgt, int32_t quality, int32_t software)
{
    Game& G = game(u);
    G.direct<0x4df1f0, frontend::mova::displayOpen>(u32(x), u32(y), u32(w), u32(hgt), u32(quality), u32(software));
}

void hostDisplayFrame(void* u, const uint8_t* frame)
{
    Game& G = game(u);
    // No renderer yet, or a frame it cannot take: that frame is not shown.
    showFrame(G.app, G.g(frame));
}

void hostDisplayClose(void* u)
{
    Game& G = game(u);
    G.direct<0x4df310, frontend::mova::displayClose>();
}

void hostFatal(void* u, const char* message)
{
    Game& G = game(u);
    SDL_strlcpy(reinterpret_cast<char*>(G.m + G.scratch + Game::kMessage), message, 0x60);
    G.direct<0x401010, ea::error>(G.scratch + Game::kMessage, guest::Words({}));
    G.dead = true;
}

}

/* sub_495bc0, showmad: the file's name in eax, the flag that drops MADe frames
 * at edx, ebx unused, the quality in ecx, "software" on the stack; ret 4. */
void madPlay(win32::WinApplication* app, x86::CPU& cpu, const x86::Args& args)
{
    // The registers it found are left as they are (args).
    const u32 eax = args.eax, ebx = args.ebx, ecx = args.ecx, edx = args.edx;
    const u32 esp = cpu.esp;
    u8* m = memoryOf(app);
    const u32 software = r32(m, esp + 4);
    // A frame of its own below the return address, for the guest memory the
    // player hands to the game: the game's calls go below it.
    cpu.esp = (cpu.esp - Game::kScratchSize) & ~0xfu;
    Game G{app, cpu, m, cpu.esp};

    MadPlayerHost host;
    std::memset(&host, 0, sizeof host);
    host.user = &G;
    host.alloc = hostAlloc;
    host.release_memory = hostRelease;
    host.stream_create = hostStreamCreate;
    host.stream_channel = hostStreamChannel;
    host.stream_setup = hostStreamSetup;
    host.stream_open = hostStreamOpen;
    host.stream_next = hostStreamNext;
    host.stream_release = hostStreamRelease;
    host.stream_ended = hostStreamEnded;
    host.stream_pending = hostStreamPending;
    host.stream_close = hostStreamClose;
    host.sound_defaults = hostSoundDefaults;
    host.sound_memory = hostSoundMemory;
    host.sound_create = hostSoundCreate;
    host.sound_start = hostSoundStart;
    host.sound_status = hostSoundStatus;
    host.sound_destroy = hostSoundDestroy;
    host.ticks = hostTicks;
    host.pump = hostPump;
    host.display_mode = hostDisplayMode;
    host.key_pressed = hostKeyPressed;
    host.display_open = hostDisplayOpen;
    host.display_frame = hostDisplayFrame;
    host.display_close = hostDisplayClose;
    host.fatal = hostFatal;
    host.screen_width = s32(m, 0x564384);
    host.screen_height = s32(m, 0x564388);

    MadPlayer& p = player();
    p.host = &host;
    std::memset(&p.s, 0, sizeof p.s);
    int32_t* skipExtra = reinterpret_cast<int32_t*>(m + edx);
    SDL_Log("[MAD] native: %s", reinterpret_cast<const char*>(m + eax));
    mad_player_play(&p, reinterpret_cast<const char*>(m + eax), skipExtra, i32(ebx), i32(ecx), i32(software));
    if (G.skipping())
        *skipExtra = 1;  // as the game's player marks a tap
    SDL_Log("[MAD] %d frames shown, %d dropped, %d skipped to a key frame, %d MADe not decoded",
            p.s.frames_shown, p.s.frames_dropped, p.s.dropped_catching_up, p.s.extra_skipped);
    p.host = nullptr;
    const bool dead = G.dead;
    cpu.esp = esp + 8;
    if (dead)
        cpu.terminate = true;
}

namespace
{
struct MovieSession
{
    MovieSession() { moviePlaying(true); }
    ~MovieSession() { moviePlaying(false); }
};
}

namespace movie
{

/* sub_495bc0: the movie marked playing for as long as it plays, for a tap to end it. */
x86::reg32 play(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    MovieSession session;
    madPlay(app, cpu, args);
    return args.eax;
}

/* sub_4df2b0: the frame at eax on the screen, if the renderer can take it. */
x86::reg32 showFrameAt(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    showFrame(app, args.eax);
    cpu.esp += 4;
    return args.eax;
}

}

}
