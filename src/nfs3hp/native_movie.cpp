#include <nfs3hp.h>
#include "native_thrash.h"
#include <lib/renderer.h>
#include <SDL3/SDL.h>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <initializer_list>
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
 * - sub_4f3560 and sub_4f3600, the decoder's two entries, for when the player
 *   is left generated (NFS_MAD_PLAYER=0): the same C decoder, its state kept
 *   here -- nothing but the decoder reads it in the game -- and checked
 *   against the generated code with NFS_MAD_CHECK=1.
 * - sub_4df2b0, the frame on the screen: not converted to the 16-bit surface
 *   by sub_4fdc90, but to full colour here and handed to the renderer, which
 *   scales it onto its place on the GPU (Renderer::presentMovieFrame).
 *
 * NFS_MAD=0 (launch extra mad false) leaves all of it generated, as does
 * NFS_NATIVES=0; NFS_MAD_PLAYER=0, NFS_MAD_GL=0 each part. */

namespace nfs3hp
{

// nfs3hp_main.cpp: a tap on the screen asks the movie to end.
bool movieSkipRequested();

namespace
{

using u8 = uint8_t;
using u32 = uint32_t;
using i32 = int32_t;

bool envOn(const char* name)
{
    const char* value = SDL_getenv(name);
    return !value || SDL_strcmp(value, "0") != 0;
}

bool madOn()
{
    static const bool on = envOn("NFS_MAD");
    return on && nativesEnabled();
}

bool madChecking()
{
    static const bool on = []() {
        const char* value = SDL_getenv("NFS_MAD_CHECK");
        return value && SDL_strcmp(value, "1") == 0;
    }();
    return on;
}

/* The decoder's check runs the generated decoder, which the native player
 * does not use: it takes the generated player with it. */
bool playerOn()
{
    static const bool on = envOn("NFS_MAD_PLAYER");
    return on && madOn() && !madChecking();
}

bool glOn()
{
    static const bool on = envOn("NFS_MAD_GL");
    return on && madOn();
}

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

/* The registers a hook found, put back as it returns. */
struct Registers
{
    u32 eax, ebx, ecx, edx, esi, edi, ebp, esp;
    explicit Registers(const x86::CPU& cpu)
        : eax(cpu.eax), ebx(cpu.ebx), ecx(cpu.ecx), edx(cpu.edx), esi(cpu.esi), edi(cpu.edi), ebp(cpu.ebp),
          esp(cpu.esp)
    {
    }
    void restore(x86::CPU& cpu) const
    {
        cpu.eax = eax;
        cpu.ebx = ebx;
        cpu.ecx = ecx;
        cpu.edx = edx;
        cpu.esi = esi;
        cpu.edi = edi;
        cpu.ebp = ebp;
        cpu.esp = esp;
    }
};

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

// ----------------------------------------------------------- the decoder

constexpr u32 kDecoderState = 0x9f0c38;  // qmat .. nbits, laid out as MadDecoder's
constexpr u32 kDecoderPointer = 0x9f2548;
constexpr u32 kCoefficients = 0x56cf2c;
constexpr u32 kIdctTemp = 0x56ce0c;
constexpr size_t kStateBytes = offsetof(MadDecoder, ptr);

void stateToGame(u8* m, const MadDecoder& d)
{
    std::memcpy(m + kDecoderState, &d, kStateBytes);
    w32(m, kDecoderPointer, d.ptr ? u32(d.ptr - m) : 0);
    std::memcpy(m + kCoefficients, d.coef, sizeof d.coef);
    std::memcpy(m + kIdctTemp, d.idct_tmp, sizeof d.idct_tmp);
}

void stateFromGame(const u8* m, MadDecoder& d)
{
    std::memcpy(&d, m + kDecoderState, kStateBytes);
    const u32 at = r32(m, kDecoderPointer);
    d.ptr = at ? m + at : nullptr;
    std::memcpy(d.coef, m + kCoefficients, sizeof d.coef);
    std::memcpy(d.idct_tmp, m + kIdctTemp, sizeof d.idct_tmp);
}

/* Where the two differ, first: an empty string when they do not. */
void describeState(const u8* m, const MadDecoder& d, char* what, size_t size)
{
    const u8* mine = reinterpret_cast<const u8*>(&d);
    for (size_t i = 0; i < kStateBytes; ++i)
    {
        if (mine[i] != m[kDecoderState + i])
        {
            SDL_snprintf(what, size, "state +0x%x (0x%x): native %02x, original %02x", unsigned(i),
                         unsigned(kDecoderState + i), mine[i], m[kDecoderState + i]);
            return;
        }
    }
    const u32 at = r32(m, kDecoderPointer);
    if ((d.ptr ? u32(d.ptr - m) : 0) != at)
    {
        SDL_snprintf(what, size, "bitstream at 0x%x native, 0x%x original", d.ptr ? unsigned(d.ptr - m) : 0u,
                     unsigned(at));
        return;
    }
    if (std::memcmp(d.coef, m + kCoefficients, sizeof d.coef) != 0)
        SDL_snprintf(what, size, "coefficients");
    else if (std::memcmp(d.idct_tmp, m + kIdctTemp, sizeof d.idct_tmp) != 0)
        SDL_snprintf(what, size, "inverse DCT's rows");
}

bool s_original = false;

struct CheckCounts
{
    unsigned calls = 0;
    unsigned mismatches = 0;
};

void report(const char* name, CheckCounts& counts, const char* what)
{
    ++counts.calls;
    if (what[0] && ++counts.mismatches <= 20)
        SDL_Log("[MAD] %s differs (call %u): %s", name, counts.calls, what);
    if (counts.calls <= 2 || counts.calls % 5000 == 0)
        SDL_Log("[MAD] %s checked: %u calls, %u differ", name, counts.calls, counts.mismatches);
}

/* The native decoder on the game's frame, then the generated one from the
 * same state on the same memory: the macroblock it writes, `rows` of
 * `rowBytes` at `out` a `stride` apart, and the state compared. */
template <typename Native>
void checkDecoder(win32::WinApplication* app, x86::CPU& cpu, const char* name, u32 address, u32 out, u32 rows,
                  u32 rowBytes, u32 stride, Native native, CheckCounts& counts)
{
    u8* m = memoryOf(app);
    MadDecoder& d = player().dec;
    stateToGame(m, d);
    std::vector<u8> before, mine;
    for (u32 r = 0; r < rows; ++r)
        before.insert(before.end(), m + out + r * stride, m + out + r * stride + rowBytes);
    native(d);
    for (u32 r = 0; r < rows; ++r)
    {
        mine.insert(mine.end(), m + out + r * stride, m + out + r * stride + rowBytes);
        std::memcpy(m + out + r * stride, before.data() + r * rowBytes, rowBytes);
    }
    const x86::CPU entry = cpu;
    s_original = true;
    cpu.esp -= 4;
    app->dynamic_call(address, cpu);
    s_original = false;
    char what[200] = "";
    for (u32 r = 0; r < rows && !what[0]; ++r)
    {
        for (u32 i = 0; i < rowBytes; ++i)
        {
            const u8 original = m[out + r * stride + i];
            if (mine[r * rowBytes + i] != original)
            {
                SDL_snprintf(what, sizeof what, "picture row %u byte %u: native %02x, original %02x", r, i,
                             mine[r * rowBytes + i], original);
                break;
            }
        }
    }
    if (!what[0])
        describeState(m, d, what, sizeof what);
    report(name, counts, what);
    // On from the original's state, so that one difference is not many.
    stateFromGame(m, d);
    cpu = entry;
}

// ------------------------------------------------------------ the player

/* EA's memory, streams and sound, the clock, the keyboard and the display, as
 * showmad calls them: Watcom's registers, then the stack, the callee's pops
 * as the original's (decomp/mad/tests/diff_player.py). */
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

    u32 call(u32 address, u32 eax = 0, u32 edx = 0, u32 ebx = 0, u32 ecx = 0, std::initializer_list<u32> stack = {})
    {
        if (dead)
            return 0;
        const Registers saved(cpu);
        u32 slot = cpu.esp - 4 * u32(stack.size());
        const u32 first = slot;
        for (const u32 argument : stack)
        {
            w32(m, slot, argument);
            slot += 4;
        }
        cpu.eax = eax;
        cpu.edx = edx;
        cpu.ebx = ebx;
        cpu.ecx = ecx;
        cpu.esp = first - 4;
        app->dynamic_call(address, cpu);
        const u32 result = cpu.eax;
        if (cpu.terminate)
            dead = true;
        saved.restore(cpu);
        return result;
    }

    /* The waits of the original are loops, where the game's other threads
     * get their turn (safepoints); these are the same waits. */
    void letOthersRun()
    {
        if (app->contextWanted())
            app->yieldContext(cpu);
    }

    /* A tap on the screen ends the movie (apply_movie_tap.py, which the
     * generated player has at its frame loop): as a key would. */
    bool skipping() const { return movieSkipRequested(); }
};

Game& game(void* user)
{
    return *static_cast<Game*>(user);
}

void* hostAlloc(void* u, uint32_t size)
{
    Game& G = game(u);
    return G.h(G.call(0x4e1620, 0x53be88, size, r32(G.m, 0x5643f4)));
}

void hostRelease(void* u, void* block)
{
    Game& G = game(u);
    G.call(0x4e1890, G.g(block));
}

void* hostStreamCreate(void* u, void* buffer, uint32_t size)
{
    Game& G = game(u);
    return G.h(G.call(0x4cfa80, 2, 2, 2, G.g(buffer), {size}));
}

void* hostStreamChannel(void* u, void* stream)
{
    Game& G = game(u);
    return G.h(G.call(0x4cfe70, G.g(stream), 2, 0xffff, 0x4353));
}

/* sub_4cfca0 takes four registers and the stack: channel 1, the sound's,
 * gets the tag 'SC' (0x4353) under the mask 0xffff, which showmad leaves in
 * ecx and ebx from sub_4cfe70 (it keeps them) rather than setting again --
 * so the decompiled call shows two arguments.  Without them that channel
 * takes every chunk and the movie's channel waits for ever. */
void hostStreamSetup(void* u, void* stream)
{
    Game& G = game(u);
    G.call(0x4cfca0, G.g(stream), 1, 0xffff, 0x4353, {2});
}

void* hostStreamOpen(void* u, void* stream, const char* path)
{
    Game& G = game(u);
    return G.h(G.call(0x4cfef0, G.g(stream), G.g(path), 0, 0));
}

const uint8_t* hostStreamNext(void* u, void* channel)
{
    Game& G = game(u);
    G.letOthersRun();
    return static_cast<const uint8_t*>(G.h(G.call(0x4d0300, G.g(channel))));
}

void hostStreamRelease(void* u, void* channel, const uint8_t* chunk)
{
    Game& G = game(u);
    G.call(0x4d03c0, G.g(channel), G.g(chunk));
}

int32_t hostStreamEnded(void* u, void* channel)
{
    Game& G = game(u);
    G.letOthersRun();
    const i32 ended = i32(G.call(0x4d04c0, G.g(channel)));
    return G.dead ? 1 : ended;
}

int32_t hostStreamPending(void* u, void* channel)
{
    Game& G = game(u);
    return i32(G.call(0x4d0460, G.g(channel)));
}

void hostStreamClose(void* u, void* stream)
{
    Game& G = game(u);
    G.call(0x4cfd30, G.g(stream));
}

void hostSoundDefaults(void* u, void* params, int32_t count)
{
    Game& G = game(u);
    G.call(0x4e9b20, G.scratch + Game::kParams, u32(count));
    std::memcpy(params, G.m + G.scratch + Game::kParams, 0x20);
}

/* sub_4f3a10 takes edx too: 0x1e, which showmad leaves there from
 * SNDplaysetdef (sub_4e9b20 keeps it). */
uint32_t hostSoundMemory(void* u, int32_t streams)
{
    Game& G = game(u);
    return G.call(0x4f3a10, u32(streams), 0x1e);
}

/* The block SNDplaysetdef filled stays where it was filled: the sound
 * player may keep pointing at it, as at the original's stack frame. */
void* hostSoundCreate(void* u, void* channel, void* params, int32_t streams, int32_t count, void* memory,
                      uint32_t size)
{
    Game& G = game(u);
    std::memcpy(G.m + G.scratch + Game::kParams, params, 0x20);
    return G.h(G.call(0x4f3a60, G.g(channel), G.scratch + Game::kParams, u32(streams), u32(count),
                      {G.g(memory), size}));
}

int32_t hostSoundStart(void* u, void* sound, void* file)
{
    Game& G = game(u);
    return i32(G.call(0x4f3a80, G.g(sound), 0, G.g(file)));
}

void hostSoundStatus(void* u, int32_t handle, int32_t status[4])
{
    Game& G = game(u);
    G.letOthersRun();
    G.call(0x4f2f50, u32(handle), G.scratch + Game::kStatus);
    std::memcpy(status, G.m + G.scratch + Game::kStatus, 16);
    if (G.dead || G.skipping())
        status[0] = 3;
}

void hostSoundDestroy(void* u, void* sound)
{
    Game& G = game(u);
    G.call(0x4f43c0, G.g(sound));
}

/* After a tap or once the game is closing, time runs out at once: the
 * frames still queued and the sound are not waited for. */
uint32_t hostTicks(void* u)
{
    Game& G = game(u);
    G.letOthersRun();
    const u32 now = G.call(0x4f2790);
    if (G.dead || G.skipping())
        return G.lastTicks += 0x10000000;
    return G.lastTicks = now;
}

void hostPump(void* u)
{
    Game& G = game(u);
    G.call(0x4e7630, 0);
}

int32_t hostDisplayMode(void* u)
{
    Game& G = game(u);
    return i32(G.call(0x4df340));
}

int32_t hostKeyPressed(void* u)
{
    Game& G = game(u);
    const i32 pressed = i32(G.call(0x451960, 1));
    return G.dead || G.skipping() ? 1 : pressed;
}

void hostDisplayOpen(void* u, int32_t x, int32_t y, int32_t w, int32_t hgt, int32_t quality, int32_t software)
{
    Game& G = game(u);
    G.call(0x4df1f0, u32(x), u32(y), u32(w), u32(hgt), {u32(quality), u32(software)});
}

void hostDisplayFrame(void* u, const uint8_t* frame)
{
    Game& G = game(u);
    if (!glOn() || !showFrame(G.app, G.g(frame)))
        G.call(0x4df2b0, G.g(frame));
}

void hostDisplayClose(void* u)
{
    Game& G = game(u);
    G.call(0x4df310);
}

void hostFatal(void* u, const char* message)
{
    Game& G = game(u);
    SDL_strlcpy(reinterpret_cast<char*>(G.m + G.scratch + Game::kMessage), message, 0x60);
    G.call(0x401010, 0, 0, 0, 0, {G.scratch + Game::kMessage});
    G.dead = true;
}

}

/* sub_495bc0, showmad: the file's name in eax, the flag that drops MADe frames
 * at edx, ebx unused, the quality in ecx, "software" on the stack; ret 4. */
bool madPlay(win32::WinApplication* app, x86::CPU& cpu)
{
    if (!playerOn())
        return false;
    const Registers entry(cpu);
    u8* m = memoryOf(app);
    const u32 software = r32(m, cpu.esp + 4);
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
    int32_t* skipExtra = reinterpret_cast<int32_t*>(m + entry.edx);
    SDL_Log("[MAD] native: %s", reinterpret_cast<const char*>(m + entry.eax));
    mad_player_play(&p, reinterpret_cast<const char*>(m + entry.eax), skipExtra, i32(entry.ebx),
                    i32(entry.ecx), i32(software));
    if (G.skipping())
        *skipExtra = 1;  // as the generated player marks a tap
    SDL_Log("[MAD] %d frames shown, %d dropped, %d skipped to a key frame, %d MADe not decoded",
            p.s.frames_shown, p.s.frames_dropped, p.s.dropped_catching_up, p.s.extra_skipped);
    p.host = nullptr;
    const bool dead = G.dead;
    entry.restore(cpu);
    cpu.esp += 8;
    if (dead)
        cpu.terminate = true;
    return true;
}

/* sub_4f3560: a frame begun -- the payload in eax, 0 for a key frame in edx,
 * the quantiser in ebx. */
bool madBeginFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    if (!madOn() || s_original)
        return false;
    u8* m = memoryOf(app);
    const u32 data = cpu.eax;
    const i32 inter = i32(cpu.edx);
    const u32 quant = cpu.ebx;
    if (madChecking())
    {
        static CheckCounts counts;
        checkDecoder(app, cpu, "sub_4f3560", 0x4f3560, 0, 0, 0, 0,
                     [&](MadDecoder& d) { mad_begin_frame(&d, m + data, inter, quant); }, counts);
    }
    else
    {
        mad_begin_frame(&player().dec, m + data, inter, quant);
    }
    cpu.esp += 4;
    return true;
}

/* sub_4f3600: a macroblock -- the reference in eax, the frame in edx, both at
 * the macroblock, the width in ebx. */
bool madMacroblock(win32::WinApplication* app, x86::CPU& cpu)
{
    if (!madOn() || s_original)
        return false;
    u8* m = memoryOf(app);
    const u32 ref = cpu.eax, cur = cpu.edx;
    const i32 width = i32(cpu.ebx);
    if (madChecking())
    {
        static CheckCounts counts;
        checkDecoder(app, cpu, "sub_4f3600", 0x4f3600, cur, 16, 32, u32(width) * 2,
                     [&](MadDecoder& d) { mad_decode_macroblock(&d, m + ref, m + cur, width); }, counts);
    }
    else
    {
        mad_decode_macroblock(&player().dec, m + ref, m + cur, width);
    }
    cpu.esp += 4;
    return true;
}

/* sub_4df2b0: the frame at eax on the screen. */
bool madShowFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    if (!glOn() || !showFrame(app, cpu.eax))
        return false;
    cpu.esp += 4;
    return true;
}

}
