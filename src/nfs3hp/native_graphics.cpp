#include "game_settings.h"
#include "native_carstate.h"
#include "native_collide.h"
#include "native_controls_input.h"
#include "native_controls_race.h"
#include "native_ea_formats.h"
#include "native_ea_memory.h"
#include "native_ea_support.h"
#include "native_ea_threads.h"
#include "native_frontend.h"
#include "native_frontend_draw.h"
#include "native_frontend_simc.h"
#include "native_frontend_simd.h"
#include "native_frontend_eaa.h"
#include "native_frontend_eab.h"
#include "native_frontend_hudz.h"
#include "native_frontend_loada.h"
#include "native_frontend_loopa.h"
#include "native_frontend_loopb.h"
#include "native_frontend_menue.h"
#include "native_frontend_menuf.h"
#include "native_frontend_menz.h"
#include "native_frontend_pause.h"
#include "native_frontend_rendc.h"
#include "native_frontend_rendd.h"
#include "native_frontend_rende.h"
#include "native_frontend_sime.h"
#include "native_frontend_tail.h"
#include "native_frontend_wata.h"
#include "native_frontend_watb.h"
#include "native_frontend_wina.h"
#include "native_game.h"
#include "native_guest.h"
#include "native_graphics.h"
#include "native_load_cardb.h"
#include "native_load_saves.h"
#include "native_raceload.h"
#include "native_track.h"
#include "native_watcom.h"
#include "port_shapes.h"
#include "port_snd_stream_calls.h"
#include "port_sound_music.h"
#include "port_sound_speech.h"
#include "port_text.h"
#include "diag_hooks.h"
#include "thrash.h"
#include "thrash_calls.h"
#include "os_system.h"
#include "native_render.h"
#include "native_vertices.h"
#include "native_frontend_hudx.h"
#include "native_frontend_connect.h"

#include <SDL3/SDL.h>
#include <lib/gamepad.h>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <optional>
#include <type_traits>

/* The game's graphics that were still the game's code: what drives the THRASH
 * driver itself (voodoo2a, the port's own in port_thrash.cpp) -- its states,
 * the frame and the window, the textures, the screen modes -- one function at
 * a time, each where the game's code was.  The addresses of decompiled.json named in "graphics" are
 * bound to the functions of those names here (DECOMPILED_NATIVES), and the game's code calls them
 * by name.
 *
 * Each is a typed function of its own (a core), with a thunk under the
 * game's name for the calls by address: it takes
 * the registers, calls the core and leaves eax, the registers the game's code
 * pushes and pops (Kept) and the x87 as the code does.  The other typed
 * files' functions their headers declare the cores call as C++
 * (guest::direct), the registers the code has around such a call as it has
 * them; the game's other functions they call by address, through a small
 * helper each with the registers the code has at that call (a void one
 * leaves eax as it was); the driver's functions are called as C++ of the port's driver where the
 * game's pointer to it (0x9ef900 on, thrash.h) leads to voodoo2a's export (guest::driver,
 * thrash_calls.h).  Locals a callee is handed the
 * address of stay on the guest's stack as deep as the code has them
 * (GuestFrame), and so does a call that reaches the Error, which hands
 * vsprintf an address on the stack. */
namespace nfs3hp
{
namespace graphics
{

namespace
{

using guest::at;
using guest::byte;
using guest::put;
using guest::putByte;
using nfs::u16;
using nfs::u32;
using nfs::u8;
using nfs::s32;

void push(nfs::App* app, nfs::ThreadState& ts, u32 value)
{
    ts.sp -= 4;
    put(app, ts.sp, value);
}

u16 half(nfs::App* app, u32 address)
{
    return app->getMemory<u16>(address);
}

void putHalf(nfs::App* app, u32 address, u16 value)
{
    app->getMemory<u16>(address) = value;
}

/* The registers as the code has them between its calls -- what the ts's eax, edx, ebx, ecx, esi, edi and ebp held while this
 * file's functions ran under the game's addresses -- one set for each guest thread: a thunk takes them from its arguments
 * (RegisterImage) and gives them back as they stand when it returns; a call of the game's code hands them on and takes
 * back what the callee left. */
// One image for this file and native_graphics.cpp / native_render.cpp alike: the code hands its registers between
// the two (the pass in ebx, the record in esi) as the game's did through the machine's.
thread_local nfs::Args& g = guest::registerImage();

class RegisterImage
{
public:
    /* The image is the thunk's arguments while it runs.  Called with an Args of its own (from outside these files), the
     * image a caller here was working with is kept aside and put back after, as the registers' values that caller holds;
     * called with the image itself (this file's calls), nothing to do. */
    explicit RegisterImage(nfs::Args& args) : args(args), own(&args != &g)
    {
        if (own)
        {
            outer = g;
            g = args;
        }
    }
    ~RegisterImage()
    {
        if (own)
        {
            args = g;
            g = outer;
        }
    }
    RegisterImage(const RegisterImage&) = delete;
    RegisterImage& operator=(const RegisterImage&) = delete;

private:
    nfs::Args& args;
    const bool own;
    nfs::Args outer;
};

/* One of THRASH's exports through the game's pointer to it (guest::driver): what it gives back is left in g.eax, as the
 * call leaves it there -- a void export leaves eax as it was.  nullopt when the game stopped in it. */
template <u32 Slot, auto Fn, typename... W>
std::optional<u32> driven(nfs::App* app, nfs::ThreadState& ts, W... words)
{
    const std::optional<u32> result = guest::driver<Slot, Fn>(app, ts, words...);
    if (result && !std::is_void_v<decltype(Fn(app, ts, u32(words)...))>)
        g.eax = *result;
    return result;
}

/* A function of the game's (a pointer call: guest::pointerCall) with the registers as the code has them, `words` pushed:
 * its eax into g.eax and the registers it left back in g.  False when the game stopped in it. */
bool called(nfs::App* app, nfs::ThreadState& ts, u32 target, std::initializer_list<u32> words = {})
{
    const std::optional<u32> eax = guest::pointerCall(app, ts, target, g, words);
    if (!eax)
        return false;
    g.eax = *eax;
    return true;
}

/* A typed core of another file called as C++ (guest::direct keeps the game's address): `args` are what the
 * callee reads -- the register arguments and the stack words it takes -- so no register is set for it and nothing pushed.  Every register but eax
 * and esp is as the caller has it; the callee's eax, where its core returns one, is left in g.eax for the
 * code that reads it after the call.  False when the game stopped. */
inline bool coreStored(bool done) { return done; }
inline bool coreStored(u32) { return true; }
inline bool coreStored(std::int32_t) { return true; }
template <typename T>
inline bool coreStored(const std::optional<T>& result)
{
    return result.has_value();
}
/* The core's result as a value: nullopt when the game stopped, else its integer result (a bool core's, a
 * core's own). */
inline std::optional<u32> coreValue(bool done) { return done ? std::optional<u32>(0) : std::nullopt; }
inline std::optional<u32> coreValue(u32 value) { return value; }
inline std::optional<u32> coreValue(std::int32_t value) { return u32(value); }
template <typename T>
inline std::optional<u32> coreValue(const std::optional<T>& result)
{
    if (!result)
        return std::nullopt;
    return u32(*result);
}
template <u32 Address, auto Core, typename... A>
bool viaCore(nfs::App* app, nfs::ThreadState& ts, A... args);
/* viaCore for a port core, which takes the frame base (esp as the callee sees it: the return address's slot)
 * first. */
template <u32 Address, auto Core, typename... A>
bool viaPort(nfs::App* app, nfs::ThreadState& ts, A... args)
{
    return viaCore<Address, Core>(app, ts, ts.sp - 4, args...);
}
template <u32 Address, auto Core, typename... A>
bool viaCore(nfs::App* app, nfs::ThreadState& ts, A... args)
{
    using Result = decltype(guest::direct<Address, Core>(app, ts, args...));
    try
    {
        if constexpr (std::is_void_v<Result>)
        {
            guest::direct<Address, Core>(app, ts, args...);
            return !ts.terminate;
        }
        else
        {
            const Result result = guest::direct<Address, Core>(app, ts, args...);
            if (ts.terminate)
                return false;
            return coreStored(result);
        }
    }
    catch (const fe::Terminated&)  // a menu core's stop, as its thunk (fe::runThunk) takes it
    {
        return false;
    }
}
/* viaCore for a core with a result the caller reads: its integer value, nullopt when the game stopped. */
template <u32 Address, auto Core, typename... A>
std::optional<u32> viaValue(nfs::App* app, nfs::ThreadState& ts, A... args)
{
    try
    {
        const auto result = guest::direct<Address, Core>(app, ts, args...);
        if (ts.terminate)
            return std::nullopt;
        return coreValue(result);
    }
    catch (const fe::Terminated&)
    {
        return std::nullopt;
    }
}


using App = nfs::App;
using ThreadState = nfs::ThreadState;

/* THRASH_setstate(state, value). */
std::optional<u32> setState(App* app, ThreadState& ts, u32 state, u32 value)
{
    return driven<thrash::kThrashSetState, &port::driver::setState>(app, ts, state, value);
}

/* Locals the game's code hands a callee the address of: kept on the guest's stack as deep as the
 * code has them (`pushed` below esp on entry, `bytes` of them), esp under them while it lives. */
class GuestFrame
{
public:
    GuestFrame(ThreadState& ts, u32 pushed, u32 bytes) : ts(ts), esp(ts.sp), base(ts.sp - pushed)
    {
        ts.sp = base - bytes;
    }
    ~GuestFrame() { ts.sp = esp; }
    u32 operator-(u32 offset) const { return base - offset; }

private:
    ThreadState& ts;
    const u32 esp;
    const u32 base;
};

constexpr u32 kError = 0x401010;

/* sub_401010, the game's Error, by address: its text on the stack (cdecl).  By address: Error's code
 * hands what it calls pointers into its frame, which lies 4 bytes lower with the text pushed. */
bool error(App* app, ThreadState& ts, u32 text)
{
    return called(app, ts, kError, {text});
}

// Which of ebx, ecx, edx, esi, edi a function of the game's pushes on entry and pops before it
// returns.
constexpr unsigned kEbx = 1, kEcx = 2, kEdx = 4, kEsi = 8, kEdi = 0x10;

/* Those registers as the caller had them, which a thunk gives back: whatever the driver or the
 * callees left in them, the game's code's caller sees them unchanged. */
struct Kept
{
    Kept(const nfs::Args& args, unsigned which)
        : which(which), ebx(g.ebx), ecx(g.ecx), edx(g.edx), esi(g.esi), edi(g.edi)
    {
    }
    void back(nfs::Args& args) const
    {
        if (which & kEbx)
            g.ebx = ebx;
        if (which & kEcx)
            g.ecx = ecx;
        if (which & kEdx)
            g.edx = edx;
        if (which & kEsi)
            g.esi = esi;
        if (which & kEdi)
            g.edi = edi;
    }
    unsigned which;
    u32 ebx, ecx, edx, esi, edi;
};

// The driver's flags (sub_4b6020): 1 << its type, a word.  0x06 the software
// renderers, 0x180 PowerVR's.
constexpr u32 kDriverFlags = 0x7a3a58;
constexpr u16 kDriverSoftware = 0x06, kDriverPowerVr = 0x180;

}  // namespace

/* ---- The render states, each sent to the driver only as it changes ---- */

// The last value sent of each state that is kept (native_render.cpp's
// [0x554e54] and [0x554e58] among them).
constexpr u32 kKeptTexture = 0x554e54;        // thrash::kStateTexture
constexpr u32 kKeptBlend = 0x554e58;          // thrash::kStateBlend
constexpr u32 kKeptFog = 0x554e5c;            // thrash::kStateFog; negative: never sent again
constexpr u32 kKeptTextureAddress = 0x554e60; // thrash::kStateTextureAddress
constexpr u32 kKeptDepth = 0x554e64;          // thrash::kStateDepth
constexpr u32 kKeptFogDensity = 0x554e68;     // thrash::kStateFogDensity
constexpr u32 kKeptFogColour = 0x554e6c;      // thrash::kStateFogColour

/* 0x431900's core: render state `state` set to `value` through THRASH_setstate.  The texture, the
 * blending, the fog, the texture's addressing and the depth test are sent only when they change; a
 * new fog density or colour sends the fog's own state again after it, unless that is negative.
 * Every other state is sent as it comes.  The driver's answer (the first's, of two), or 1 for one
 * not sent.  eax at each call as the game's code has it: the value, the fog for the fog again after
 * a density, the first answer after a colour. */
std::optional<u32> renderState(App* app, ThreadState& ts, u32 state, u32 value)
{
    // A kept state: sent if it changed.
    auto kept = [&](u32 keptAt) -> std::optional<u32> {
        if (value == at(app, keptAt))
            return 1;
        put(app, keptAt, value);
        return setState(app, ts, state, value);
    };
    // A fog density or colour, kept, then the fog's own state again when it is not negative.
    auto keptThenFog = [&](u32 keptAt) -> std::optional<u32> {
        if (value == at(app, keptAt))
            return 1;
        put(app, keptAt, value);
        const std::optional<u32> answer = setState(app, ts, state, value);
        if (!answer)
            return std::nullopt;
        const u32 fog = at(app, kKeptFog);
        if (s32(fog) < 0)
            return answer;
        if (!setState(app, ts, thrash::kStateFog, fog))
            return std::nullopt;
        return answer;
    };
    switch (state)
    {
    case thrash::kStateTexture:
        return kept(kKeptTexture);
    case thrash::kStateDepth:
        return kept(kKeptDepth);
    case thrash::kStateTextureAddress:
        return kept(kKeptTextureAddress);
    case thrash::kStateFogDensity:
        return keptThenFog(kKeptFogDensity);
    case thrash::kStateFogColour:
        return keptThenFog(kKeptFogColour);
    case thrash::kStateFog:
        return kept(kKeptFog);
    case thrash::kStateBlend:
        return kept(kKeptBlend);
    default:
        return setState(app, ts, state, value);
    }
}

/* renderState as the game's code calls 0x431900, its answer (the driver's, 1 for one not sent) back;
 * nullopt when the game stopped. */
std::optional<u32> stateAnswer(App* app, ThreadState& ts, u32 state, u32 value)
{
    return renderState(app, ts, state, value);
}

/* renderState for those that take no answer.  False when the game stopped. */
bool stateSent(App* app, ThreadState& ts, u32 state, u32 value)
{
    return renderState(app, ts, state, value).has_value();
}

/* The same for the callers that read the answer in eax (its core is stateAnswer's). */
bool sendState(App* app, ThreadState& ts, u32 state, u32 value)
{
    const std::optional<u32> answer = renderState(app, ts, state, value);
    if (!answer)
        return false;
    g.eax = *answer;
    return true;
}

/* 0x431900: renderState for state eax, value edx; edx and eax as the code leaves them, ebx and ecx kept. */
nfs::u32 setRenderState(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const Kept kept(g, kEbx | kEcx);
    const u32 value = g.edx;
    const std::optional<u32> answer = renderState(app, ts, g.eax, value);
    if (!answer)
        return g.eax;
    g.edx = value;
    g.eax = *answer;
    kept.back(g);
    ts.sp += 4;
    return g.eax;
}

/* ---- The frame ---- */

/* 0x434a00's core: a frame ended: the window taken (THRASH_window 2) but on PowerVR, the picture
 * flushed if `flush`, flipped if `flip`, the window again, and but on PowerVR the window cleared.
 * eax as the driver leaves it. */
bool frameEnd(App* app, ThreadState& ts, bool flip, bool flush)
{
    const bool powerVr = half(app, kDriverFlags) & kDriverPowerVr;
    auto window = [&]() { return powerVr || driven<thrash::kThrashWindow, &port::driver::window>(app, ts, 2); };
    if (!window() || (flush && !driven<thrash::kThrashFlushWindow, &port::driver::flushWindow>(app, ts)) || (flip && !driven<thrash::kThrashPageFlip, &port::driver::pageFlip>(app, ts))
        || !window())
        return false;
    if (!powerVr)
        return driven<thrash::kThrashClearWindow, &port::driver::clearWindow>(app, ts).has_value();
    return true;
}

/* 0x434a00: frameEnd, flipped if eax, flushed if edx. */
nfs::u32 endFrame(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const Kept kept(g, kEbx | kEcx);
    if (!frameEnd(app, ts, g.eax != 0, g.edx != 0))
        return g.eax;
    kept.back(g);
    ts.sp += 4;
    return g.eax;
}

/* 0x434b20's core: the driver waited for (THRASH_sync 0) and the window taken (THRASH_window 2). */
bool windowSynced(App* app, ThreadState& ts)
{
    return driven<thrash::kThrashSync, &port::driver::sync>(app, ts, 0) && driven<thrash::kThrashWindow, &port::driver::window>(app, ts, 2);
}

/* 0x434b20: windowSynced. */
nfs::u32 syncWindow(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const Kept kept(g, kEcx | kEdx);
    if (!windowSynced(app, ts))
        return g.eax;
    kept.back(g);
    ts.sp += 4;
    return g.eax;
}

/* The picture flushed and flipped (THRASH_flushwindow, THRASH_pageflip). */
bool flushFlip(App* app, ThreadState& ts)
{
    return driven<thrash::kThrashFlushWindow, &port::driver::flushWindow>(app, ts) && driven<thrash::kThrashPageFlip, &port::driver::pageFlip>(app, ts);
}

/* 0x434b40: flushFlip. */
nfs::u32 flushAndFlip(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const Kept kept(g, kEcx | kEdx);
    if (!flushFlip(app, ts))
        return g.eax;
    kept.back(g);
    ts.sp += 4;
    return g.eax;
}

/* 0x5244e0: THRASH_idle; eax 0. */
nfs::u32 driverIdle(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const Kept kept(g, kEcx);
    if (!driven<thrash::kThrashIdle, &port::driver::idle>(app, ts))
        return g.eax;
    g.eax = 0;
    kept.back(g);
    ts.sp += 4;
    return g.eax;
}

/* ---- Clearing and flipping ---- */

constexpr u32 kDriverOnCard = 0x7a3a60;      // the z-buffer: 1 (sub_4b59c0), 0 when turned off (sub_431b40)
constexpr u16 kDriverSofttria = 0x02;        // kDriverFlags' bit for softtria

/* 0x4d5c90's core: the window cleared with the depth test as the driver has it, and the depth test
 * off after; ecx and edx kept, as the game's code keeps them for its callers.  eax: the last
 * state's answer. */
std::optional<u32> clearDepthAnswer(App* app, ThreadState& ts)
{
    if (!stateSent(app, ts, thrash::kStateDepth, at(app, kDriverOnCard)) || !driven<thrash::kThrashClearWindow, &port::driver::clearWindow>(app, ts))
        return std::nullopt;
    return stateAnswer(app, ts, thrash::kStateDepth, 0);
}

bool clearDepth(App* app, ThreadState& ts)
{
    return clearDepthAnswer(app, ts).has_value();
}

/* 0x4d5c90: clearDepth. */
nfs::u32 clearWithDepth(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const Kept kept(g, kEcx | kEdx);
    if (!clearDepth(app, ts))
        return g.eax;
    kept.back(g);
    ts.sp += 4;
    return g.eax;
}

constexpr u32 kDrawingBegun = 0x563b30;      // set by beginDrawing, once
constexpr u32 kResetTextures = 0x4d5c40;

/* sub_4d5c40, every texture let go of (port_shapes), by address: eax, edx, ebx and ecx as the
 * game's code has them there. */
bool resetTextures(App* app, ThreadState& ts)
{
    return viaPort<kResetTextures, port::shapes::texturesLetGo>(app, ts);
}

/* 0x4d5cc0's core: drawing begun, once: the textures reset first if `reset` (in eax, `clear` in edx
 * and ebx at that call, as the code has them); then each buffer (THRASH_window 2, then 1) cleared
 * if `clear`. */
bool drawingBegun(App* app, ThreadState& ts, u32 reset, u32 clear)
{
    if (at(app, kDrawingBegun) != 0)
        return true;
    put(app, kDrawingBegun, 1);
    if (reset != 0 && !resetTextures(app, ts))
        return false;
    for (const u32 buffer : {2u, 1u})
        if (!driven<thrash::kThrashWindow, &port::driver::window>(app, ts, buffer) || (clear != 0 && !clearDepth(app, ts)))
            return false;
    return true;
}

/* 0x4d5cc0: drawingBegun, reset eax, clear edx. */
nfs::u32 beginDrawing(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const Kept kept(g, kEbx | kEcx);
    if (!drawingBegun(app, ts, g.eax, g.edx))
        return g.eax;
    kept.back(g);
    ts.sp += 4;
    return g.eax;
}

/* Both buffers cleared: the window taken, then twice cleared, flushed and flipped. */
bool clearTwice(App* app, ThreadState& ts)
{
    if (!driven<thrash::kThrashWindow, &port::driver::window>(app, ts, 2))
        return false;
    for (int i = 0; i < 2; ++i)
        if (!clearDepth(app, ts) || !flushFlip(app, ts))
            return false;
    return true;
}

/* 0x4d5d30: both buffers cleared. */
nfs::u32 clearBothBuffers(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const Kept kept(g, kEcx | kEdx);
    if (!clearTwice(app, ts))
        return g.eax;
    kept.back(g);
    ts.sp += 4;
    return g.eax;
}

/* 0x4d5d70's core: both buffers cleared, then the textures reset. */
bool clearTwiceAndReset(App* app, ThreadState& ts)
{
    return clearTwice(app, ts) && resetTextures(app, ts);
}

/* 0x4d5d30's core with its answer: both buffers cleared, eax as the last flip leaves it; nullopt when the game
 * stopped. */
std::optional<u32> clearedTwice(App* app, ThreadState& ts)
{
    if (!clearTwice(app, ts))
        return std::nullopt;
    return g.eax;
}

/* 0x4d5d70's core with its answer: the same, then the textures reset; eax as that leaves it. */
std::optional<u32> clearedTwiceAndReset(App* app, ThreadState& ts)
{
    if (!clearTwiceAndReset(app, ts))
        return std::nullopt;
    return g.eax;
}

/* 0x4d5d70: clearTwiceAndReset. */
nfs::u32 clearBothBuffersAndReset(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const Kept kept(g, kEcx | kEdx);
    if (!clearTwiceAndReset(app, ts))
        return g.eax;
    kept.back(g);
    ts.sp += 4;
    return g.eax;
}

/* 0x4d9120's core: the window taken (THRASH_window 2) and cleared; ecx and edx kept, as the code
 * keeps them. */
bool windowTakenAndCleared(App* app, ThreadState& ts)
{
    return driven<thrash::kThrashWindow, &port::driver::window>(app, ts, 2) && clearDepth(app, ts);
}

/* 0x4d9120: windowTakenAndCleared. */
nfs::u32 takeWindowAndClear(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    if (windowTakenAndCleared(app, ts))
        ts.sp += 4;
    return g.eax;
}

constexpr u32 kTicks = 0x79c1a4;             // the game's clock (native_frontend_loop.cpp's)
constexpr u32 kLastFlipTick = 0x56415c;      // the clock at the last flipAndWaitFrames
constexpr u32 kWaitForTicks = 0x4c9860;

/* sub_4c9860, a wait for the clock, by address: the clock in eax and edx. */
bool waitForTicks(App* app, ThreadState& ts, u32)
{
    return viaPort<kWaitForTicks, port::descriptionServed>(app, ts);
}

/* 0x4d9140's core: the picture flushed and flipped, then waited (sub_4c9860) until the clock is two
 * ticks past the last flip's, which it is kept as.  The tick waited for, as eax has it, or nullopt
 * when the game stopped. */
std::optional<u32> flipAndWait(App* app, ThreadState& ts)
{
    if (!flushFlip(app, ts))
        return std::nullopt;
    u32 now = at(app, kTicks);
    u32 until = at(app, kLastFlipTick) + 2;
    while (s32(until) > s32(now))
    {
        now = at(app, kTicks);
        if (!waitForTicks(app, ts, now))
            return std::nullopt;
        until = at(app, kLastFlipTick) + 2;
    }
    put(app, kLastFlipTick, now);
    return until;
}

/* 0x4d9140: flipAndWait. */
nfs::u32 flipAndWaitFrames(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const Kept kept(g, kEcx | kEdx);
    const std::optional<u32> until = flipAndWait(app, ts);
    if (!until)
        return g.eax;
    g.eax = *until;
    kept.back(g);
    ts.sp += 4;
    return g.eax;
}

/* ---- Drawn whole ---- */

/* A rectangle's four corners (0x20 bytes each: x, y as floats, the colour at
 * +0x10), and drawFlatRectangle's. */
constexpr u32 kRectCorners = 0x5599a0;
constexpr u32 kCornerBytes = 0x20, kCornerColour = 0x10;

void putFloat(App* app, u32 address, float value)
{
    app->getMemory<float>(address) = value;
}

float floatAt(App* app, u32 address)
{
    return app->getMemory<float>(address);
}

u32 bitsOf(float value)
{
    u32 bits;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}

/* 0x459350's core: a rectangle drawn flat, untextured, between corners (left, top) and (right,
 * bottom), `colour` at each: its corners clockwise from the first, the texture off, THRASH_drawquad.
 * eax as the driver leaves it, which is the result; nullopt when the game stopped. */
std::optional<u32> flatRectangle(App* app, ThreadState& ts, s32 left, s32 top, s32 right, s32 bottom, u32 colour)
{
    const auto corner = [](u32 n) { return kRectCorners + n * kCornerBytes; };
    for (u32 n = 3; n < 4; --n)
        put(app, corner(n) + kCornerColour, colour);
    // Each fild and fstp: the integer as a float, rounded as C++ rounds it.
    putFloat(app, corner(0), float(left));
    putFloat(app, corner(1), float(right));
    putFloat(app, corner(0) + 4, float(top));
    putFloat(app, corner(2) + 4, float(bottom));
    putFloat(app, corner(2), floatAt(app, corner(1)));
    putFloat(app, corner(3), floatAt(app, corner(0)));
    putFloat(app, corner(1) + 4, floatAt(app, corner(0) + 4));
    putFloat(app, corner(3) + 4, floatAt(app, corner(2) + 4));
    if (!stateSent(app, ts, thrash::kStateTexture, 0))
        return std::nullopt;
    return driven<thrash::kThrashDrawQuad, &port::driver::drawQuad>(app, ts, corner(0), corner(1), corner(2), corner(3));
}

/* 0x459350: flatRectangle from (eax, edx) to (ebx, ecx), the colour [esp+4] (the colour the code
 * leaves in ebx, an argument's, no caller's after it). */
nfs::u32 drawFlatRectangle(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const u32 colour = at(app, ts.sp + 4);
    if (!flatRectangle(app, ts, s32(g.eax), s32(g.edx), s32(g.ebx), s32(g.ecx), colour).has_value())
        return g.eax;
    ts.sp += 4 + 4;
    return g.eax;
}

/* The picture drawn whole on the screen: its four corners and the mesh that
 * makes them one quad. */
constexpr u32 kPictureCorners = 0x55e60c, kPictureMesh = 0x55e68c;

/* 0x49c570's core: a frame of one picture, `texture`, over the whole screen: the window taken and
 * cleared, the picture drawn, flushed, flipped, waited for; then the game's messages seen to
 * (ea::waitKey, 0x4f0120). */
bool picture(App* app, ThreadState& ts, u32 texture)
{
    return driven<thrash::kThrashWindow, &port::driver::window>(app, ts, 2) && driven<thrash::kThrashClearWindow, &port::driver::clearWindow>(app, ts)
        && driven<thrash::kThrashSetTexture, &port::driver::setTexture>(app, ts, texture)
        && driven<thrash::kThrashDrawQuadMesh, &port::driver::drawQuadMesh>(app, ts, 1, kPictureCorners, kPictureMesh) && flushFlip(app, ts)
        && driven<thrash::kThrashSync, &port::driver::sync>(app, ts, 0) && guest::direct<0x4f0120, ea::waitKey>(app, ts).has_value();
}

/* 0x49c570: picture, texture eax. */
nfs::u32 showPicture(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const Kept kept(g, kEbx | kEcx | kEdx);
    if (!picture(app, ts, g.eax))
        return g.eax;
    kept.back(g);
    ts.sp += 4;
    return g.eax;
}

std::optional<u32> textureFormat(App* app, ThreadState& ts);

/* 0x49c600's core: a texture allocated (THRASH_talloc) of `width` and `height` -- eax and edx as
 * the code hands them on -- in the format textureFormat gives.  eax as the driver leaves it. */
std::optional<u32> textureHandle(App* app, ThreadState& ts, u32 width, u32 height)
{
    std::optional<u32> format;
    {
        const GuestFrame deep(ts, 0xc, 0);  // the code's two pushes and the call's slot, for the Error's
        format = textureFormat(app, ts);
    }
    if (!format)
        return std::nullopt;
    return driven<thrash::kThrashTAlloc, &port::driver::tAlloc>(app, ts, width, height, *format, 0, 0);
}

bool textureAllocated(App* app, ThreadState& ts, u32 width, u32 height)
{
    return textureHandle(app, ts, width, height).has_value();
}

/* 0x49c600: textureAllocated, eax by edx. */
nfs::u32 allocTexture(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const Kept kept(g, kEcx);
    const std::optional<u32> handle = textureHandle(app, ts, g.eax, g.edx);
    if (!handle)
        return g.eax;
    g.eax = *handle;
    kept.back(g);
    ts.sp += 4;
    return g.eax;
}

/* The screen's size, and the rectangle drawn in (native_frontend_screen's
 * kViewport): left, top, width, height. */
constexpr u32 kScreenWidth = 0x7cda68, kScreenHeight = 0x7cda6c;
constexpr u32 kClipLeft = 0x7cda70, kClipTop = 0x7cda74, kClipWidth = 0x7cda78, kClipHeight = 0x7cda7c;
constexpr u32 kSetViewCorners = 0x4bf1b0;    // left, top, right, bottom as floats, then two more
constexpr u32 kViewNear = 0x3f800000, kViewDepthScale = 0x3b0355ad;  // setViewCorners' two more: 1, about 0.002

/* 0x4bebe0's core: the rectangle drawn in set to x, y, `wide` by `high`, kept on the screen:
 * THRASH_clip given its corners, sub_4bf1b0 them as floats with 1 and 0.002.  The registers at that
 * call as the code has them: eax the right, ebx the width, ecx the height.  What sub_4bf1b0 leaves in eax
 * back, nullopt when the game stopped. */
std::optional<u32> clipSet(App* app, ThreadState& ts, s32 x, s32 y, s32 wide, s32 high)
{
    x = x < 0 ? 0 : x;
    y = y < 0 ? 0 : y;
    const s32 lastX = s32(at(app, kScreenWidth) - 1), lastY = s32(at(app, kScreenHeight) - 1);
    put(app, kClipLeft, u32(x >= lastX ? lastX : x));
    put(app, kClipTop, u32(y >= lastY ? lastY : y));
    const s32 widthLeft = s32(at(app, kScreenWidth) - at(app, kClipLeft));
    put(app, kClipWidth, u32(wide < widthLeft ? wide : widthLeft));
    const s32 heightLeft = s32(at(app, kScreenHeight) - at(app, kClipTop));
    put(app, kClipHeight, u32(high < heightLeft ? high : heightLeft));
    const u32 left = at(app, kClipLeft), top = at(app, kClipTop);
    if (!driven<thrash::kThrashClip, &port::driver::clip>(app, ts, left, top, left + at(app, kClipWidth), at(app, kClipHeight) + top))
        return std::nullopt;
    const u32 width = at(app, kClipWidth), height = at(app, kClipHeight);
    const u32 right = at(app, kClipLeft) + width, bottom = at(app, kClipTop) + height;
    // Each fild and fstp: the integer as a float, rounded as C++ rounds it.
    return viaValue<kSetViewCorners, frontend::rendd::setClipBounds>(
        app, ts, bitsOf(float(s32(at(app, kClipLeft)))), bitsOf(float(s32(at(app, kClipTop)))),
        bitsOf(float(s32(right))), bitsOf(float(s32(bottom))), kViewNear, kViewDepthScale);
}

/* 0x4bebe0: clipSet to x eax, y edx, width ebx, height ecx. */
nfs::u32 setClip(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const std::optional<u32> answer = clipSet(app, ts, s32(g.eax), s32(g.edx), s32(g.ebx), s32(g.ecx));
    if (!answer)
        return g.eax;
    g.eax = *answer;
    ts.sp += 4;
    return g.eax;
}

/* The view and what surrounds it. */
constexpr u32 kViewWidth = 0x7cdba0;         // the view's rectangle, from 0x7cdb98: x, y, width, height
constexpr u32 kViewSurroundFrames = 0x7cdba8;  // counted down at each drawViewSurround
constexpr u32 kSurroundCorners = 0x55fed4, kSurroundMesh = 0x560054;

/* 0x4be8e0's core: the quads around the view, untextured, without the depth test -- two of them in
 * split screen, four otherwise -- when the view is narrower than the screen, or with Wide Screen in
 * a race of one screen's player.  The last state's answer, or the view's width when nothing is
 * drawn: what eax is left. */
std::optional<u32> viewSurround(App* app, ThreadState& ts)
{
    put(app, kViewSurroundFrames, at(app, kViewSurroundFrames) - 1);
    const u32 viewWidth = at(app, kViewWidth);
    const bool narrower = s32(viewWidth) < s32(at(app, kScreenWidth));
    if (!narrower && (gameSettings(app).wideScreen == 0 || gameSettings(app).playMode == 1))
        return viewWidth;
    if (!stateSent(app, ts, thrash::kStateTexture, 0) || !stateSent(app, ts, thrash::kStateDepth, 0)
        || !driven<thrash::kThrashDrawQuadMesh, &port::driver::drawQuadMesh>(app, ts, gameSettings(app).playMode == 1 ? 2u : 4u, kSurroundCorners, kSurroundMesh))
        return std::nullopt;
    return stateAnswer(app, ts, thrash::kStateDepth, at(app, kDriverOnCard));
}

/* 0x4be8e0: viewSurround. */
nfs::u32 drawViewSurround(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const Kept kept(g, kEcx | kEdx);
    const std::optional<u32> left = viewSurround(app, ts);
    if (!left)
        return g.eax;
    g.eax = *left;
    kept.back(g);
    ts.sp += 4;
    return g.eax;
}

/* ---- The frame buffer's lock ---- */

constexpr u32 kFrameLocked = 0x56aff4;       // the screen record the frame buffer is locked for
constexpr u32 kFrameLock = 0xa18220;         // THRASH_lockwindow's record, or none
constexpr u32 kFreeBuffer = 0x509b10;

/* sub_509b10, a block freed, by address: the block in eax. */
bool freeBlock(App* app, ThreadState& ts, u32 block)
{
    return viaCore<kFreeBuffer, frontend::watb::removeRow>(app, ts, block);
}

/* 0x5178d0's core: the frame buffer's lock, which frameBufferLocked keeps with its screen record,
 * let go of (THRASH_unlockwindow), and the window taken (THRASH_window 0); ecx and edx kept, as the
 * code keeps them. */
bool frameLockReleased(App* app, ThreadState& ts)
{
    if (at(app, kFrameLocked) != 0)
    {
        if (!driven<thrash::kThrashUnlockWindow, &port::driver::unlockWindow>(app, ts, at(app, kFrameLock)))
            return false;
        put(app, kFrameLock, 0);
        put(app, kFrameLocked, 0);
    }
    return driven<thrash::kThrashWindow, &port::driver::window>(app, ts, 0).has_value();
}

/* 0x5178d0: frameLockReleased. */
nfs::u32 releaseFrameLock(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const Kept kept(g, kEcx | kEdx);
    if (!frameLockReleased(app, ts))
        return g.eax;
    kept.back(g);
    ts.sp += 4;
    return g.eax;
}

/* ---- The driver opened ---- */

constexpr u32 kDriverOpen = 0x56aff0;        // a byte: THRASH_init done, until the driver is closed
constexpr u32 kCloseAtExit = 0x56aff1;       // a byte: closeDriverAtExit handed to sub_4f2a78 once
constexpr u32 kWantedWidth = 0x564384, kWantedHeight = 0x564388;  // the screen mode's, 0 for 640 by 480
constexpr u32 kInstance = 0x564480;          // sub_4eb2e0's, the first time
constexpr u32 kGameWindow = 0x9f3d24;        // native_game's kWindow
constexpr u32 kLambdasRecord = 0x56afd0;     // thrash::kStateLambdas' record
constexpr u32 kErrorLock = 0x564418;         // native_game's: called before an Error's box
constexpr u32 kMakeWindow = 0x4cd6c0;        // eax the width, ebx whether a window of its own
constexpr u32 kDestroyWindow = 0x4cdaa0;
constexpr u32 kAcquireWindow = 0x4eba70;     // native_game's AcquireWindow
constexpr u32 kAddTimer = 0x4e7520;          // eax a function, edx its argument, ebx the ticks between
constexpr u32 kAtExit = 0x4f2a78;            // eax a function, called as the game ends
constexpr u32 kDriverIdle = 0x5244e0;        // driverIdle
constexpr u32 kCloseDriverAtExit = 0x524640, kCloseDriverForError = 0x524620;

/* A function of the game's by address, the registers as the caller set them: its eax, or nullopt
 * when the game stopped in it. */
std::optional<u32> game(App* app, ThreadState& ts, u32 target)
{
    if (!called(app, ts, target))
        return std::nullopt;
    return g.eax;  // left: a callee with no core (native_game's AcquireWindow), called through its pointer
}

/* sub_4cd6c0, the game's window made, by address: eax the width, edx the height, ebx whether a
 * window of its own.  Its answer, 0 for none. */
std::optional<u32> makeWindow(App* app, ThreadState& ts, u32 width, u32 height, bool own)
{
    return viaValue<kMakeWindow, frontend::wina::openWindow>(app, ts, width, height, own ? 1u : 0u);
}

/* port::addSystemTask (0x4e7520), by address: eax the function, edx its argument, ebx the ticks
 * between. */
bool addTask(App* app, ThreadState& ts, u32 function, u32 argument, u32 ticks)
{
    return viaPort<kAddTimer, port::stream::scheduleTask>(app, ts, function, s32(argument), ticks);
}

/* sub_4f2a78, `function` run as the game ends, by address (eax). */
bool atExit(App* app, ThreadState& ts, u32 function)
{
    return viaCore<kAtExit, frontend::eab::atexitJump>(app, ts, function);
}

constexpr u32 kDefaultWidth = 640, kDefaultHeight = 480;

/* 0x5246a0's core: the driver opened, unless it is already: the game's window made if it has none
 * (one of its own for `windowMode` 0), the driver given the window's mode, the window and the
 * game's callbacks, and THRASH_init; if that fails, the window destroyed again.  Once open,
 * driverIdle on a timer if the driver has THRASH_idle, the driver closed at the game's end and
 * before an Error's box.  Whether it is open -- kDriverOpen's byte, as the code hands it back --
 * or nullopt when the game stopped. */
std::optional<u32> driverOpened(App* app, ThreadState& ts, u32 windowMode)
{
    const u32 width = at(app, kWantedWidth) == 0 ? kDefaultWidth : at(app, kWantedWidth);
    const u32 height = at(app, kWantedHeight) == 0 ? kDefaultHeight : at(app, kWantedHeight);
    const auto open = [&]() { return u32(byte(app, kDriverOpen)); };
    if (open() != 0)
        return open();
    if (at(app, kInstance) == 0)
    {
        const std::optional<u32> instance = guest::direct<0x4eb2e0, ea::newLock>(app, ts);
        if (!instance)
            return std::nullopt;
        put(app, kInstance, *instance);
    }
    if (at(app, kGameWindow) == 0)
    {
        const std::optional<u32> made = makeWindow(app, ts, width, height, windowMode == 0);
        if (!made)
            return std::nullopt;
        if (*made == 0)
            return open();
    }
    if (!setState(app, ts, thrash::kStateWindowMode, windowMode))
        return std::nullopt;
    const std::optional<u32> window = viaValue<kAcquireWindow, ea::acquireWindow>(app, ts);
    if (!window || !setState(app, ts, thrash::kStateWindow, *window)
        || !setState(app, ts, thrash::kStateLambdas, kLambdasRecord))
        return std::nullopt;
    const std::optional<u32> initialised = driven<thrash::kThrashInit, &port::driver::init>(app, ts);
    if (!initialised)
        return std::nullopt;
    if (*initialised == 0)
    {
        if (!viaCore<kDestroyWindow, frontend::wina::closeWindow>(app, ts))
            return std::nullopt;
        return open();
    }
    if (at(app, thrash::kThrashIdle) != 0 && !addTask(app, ts, kDriverIdle, 1, 1))
        return std::nullopt;
    if (byte(app, kCloseAtExit) == 0)
    {
        if (!atExit(app, ts, kCloseDriverAtExit))
            return std::nullopt;
        putByte(app, kCloseAtExit, 1);
    }
    put(app, kErrorLock, kCloseDriverForError);
    putByte(app, kDriverOpen, 1);
    return open();
}

/* 0x5246a0: driverOpened, the window's mode eax; eax whether it is open. */
nfs::u32 openDriver(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const Kept kept(g, kEbx | kEcx | kEdx);
    const std::optional<u32> open = driverOpened(app, ts, g.eax);
    if (!open)
        return g.eax;
    g.eax = *open;
    kept.back(g);
    ts.sp += 4;
    return g.eax;
}

/* ---- The screen's mode ---- */

constexpr u32 kVideoReady = 0x5644fc;        // set up for modes; without it an Error first
constexpr u32 kModeSet = 0x564364;           // a mode set, the driver open
constexpr u32 kWindowedFlags = 0x564390;     // bit 0: the driver opened in a window
constexpr u32 kModeBuffers = 0x565058;       // two, and two more at 0x565090: sub_509b10's to free
constexpr u32 kModeBuffersMore = 0x565090;
constexpr u32 kModeHandle = 0x5643e0;        // sub_4eff00's to let go of
constexpr u32 kScreens = 0x564394;           // how many screens: past 1 the second set up too
constexpr u32 kFirstScreen = 0x56502c, kSecondScreen = 0x565064;
constexpr u32 kScreenSetWidth = 0x565030, kScreenSetHeight = 0x565034, kScreenSetBits = 0x565048;
constexpr u32 kScreenBits = 0x56438c;        // the screen's bits a pixel, 15 for a 5:5:5 one
constexpr u32 kKeepScreenSize = 0x5247c0;    // eax, edx, ebx: width, height, bits (0 for 640, 480, 16)
constexpr u32 kLetGo = 0x4eff00;
constexpr u32 kSetUpScreen = 0x5179d0;       // eax which, ebx its record
constexpr u32 kModeAtExit = 0x517580, kModeForError = 0x5175b0;

// native_game's kWhere: the file, the function and the line an Error names.
constexpr u32 kWhereFile = 0x552190, kWhereFunction = 0x552194, kWhereLine = 0x552198;
constexpr u32 kNotReadyFile = 0x5505b0, kNotReadyFunction = 0x5505c0, kNotReadyLine = 0x3b;
constexpr u32 kNotReady = 0x5505d0;          // the Error's text: modes asked for before they are set up

/* The driver's mode for a screen `width` (or `height`) wide: THRASH_setvideomode's numbers. */
u32 driverMode(u32 width, u32 height)
{
    switch (width)
    {
    case 0x140: return 4;
    case 0x200: return 6;
    case 0x320: return 2;
    case 0x3c0: return 0xa;
    case 0x358: return 0xb;
    case 0x400: return 0xd;
    default: return height == 0x1f6 ? 0x11 : height == 0x336 ? 0x12 : 1;
    }
}

bool screenSetUp(App* app, ThreadState& ts, u32 which, u32 window, u32 record);

/* sub_4eff00, something let go of, by address: eax it. */
bool letGo(App* app, ThreadState& ts, u32 handle)
{
    return viaCore<kLetGo, frontend::eab::windowBitmapFree>(app, ts, handle);
}

/* 0x517610's core: the screen set to `width` by `height`, `bits` deep (0 for 640, 480 and 16 bits;
 * -1 for 1024 by 768; 15 bits as 16): the size kept (sub_5247c0); the first time the driver opened
 * -- in a window for bit 0 of [0x564390] -- and what to do at the game's end and before an Error's
 * box set; THRASH_setvideomode given the mode of that width (or of the height, else 1), the
 * buffers of the last mode freed, the screens set up again and what they came to kept, the registry
 * flushed.  1, or 0 when the driver would not open; the registers each call takes as the code has
 * them. */
std::optional<u32> videoModeSet(App* app, ThreadState& ts, u32 width, u32 height, u32 bits)
{
    if (at(app, kVideoReady) == 0)
    {
        put(app, kWhereFile, kNotReadyFile);
        put(app, kWhereFunction, kNotReadyFunction);
        put(app, kWhereLine, kNotReadyLine);
        const GuestFrame deep(ts, 0x10, 0);  // under the code's four pushes: the Error's vsprintf sees the stack
        if (!error(app, ts, kNotReady))
            return std::nullopt;
    }
    width = width == 0 ? kDefaultWidth : width == 0xffffffff ? 0x400 : width;
    height = height == 0 ? kDefaultHeight : height == 0xffffffff ? 0x300 : height;
    bits = bits == 0 || bits == 0xffffffff || bits == 0xf ? 0x10 : bits;
    if (!viaCore<kKeepScreenSize, frontend::wina::setScreenSize>(app, ts, width, height, bits))
        return std::nullopt;
    if (at(app, kModeSet) == 0)
    {
        const std::optional<u32> open = driverOpened(app, ts, byte(app, kWindowedFlags) & 1 ? 0 : 1);
        if (!open)
            return std::nullopt;
        if (*open == 0)
            return 0;
        if (!atExit(app, ts, kModeAtExit))
            return std::nullopt;
        put(app, kErrorLock, kModeForError);
    }
    if (!driven<thrash::kThrashSetVideoMode, &port::driver::setVideoMode>(app, ts, driverMode(width, height), 2, 1))
        return std::nullopt;
    for (const u32 buffer : {kModeBuffers, kModeBuffers + 4})
        if (at(app, buffer) != 0 && !freeBlock(app, ts, at(app, buffer)))
            return std::nullopt;
    const u32 more = at(app, kModeBuffersMore);
    put(app, kModeBuffers, 0);
    put(app, kModeBuffers + 4, 0);
    if (more != 0 && !freeBlock(app, ts, more))
        return std::nullopt;
    if (at(app, kModeBuffersMore + 4) != 0 && !freeBlock(app, ts, at(app, kModeBuffersMore + 4)))
        return std::nullopt;
    const u32 handle = at(app, kModeHandle);
    put(app, kModeBuffersMore, 0);
    put(app, kModeBuffersMore + 4, 0);
    if (handle != 0)
    {
        if (!letGo(app, ts, handle))
            return std::nullopt;
        put(app, kModeHandle, 0);
    }
    put(app, kModeSet, 1);
    if (!screenSetUp(app, ts, 1, 0, kFirstScreen)
        || (s32(at(app, kScreens)) > 1 && !screenSetUp(app, ts, 2, 0, kSecondScreen)))
        return std::nullopt;
    put(app, kWantedWidth, at(app, kScreenSetWidth));
    put(app, kWantedHeight, at(app, kScreenSetHeight));
    put(app, kScreenBits, byte(app, kScreenSetBits));
    // The code flushes the registry's classes here: the port's settings are in memory, nothing to
    // flush.
    return 1;
}

/* 0x517610: videoModeSet, eax by edx, ebx bits. */
nfs::u32 setVideoMode(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const Kept kept(g, kEcx);
    const std::optional<u32> set = videoModeSet(app, ts, g.eax, g.edx, g.ebx);
    if (!set)
        return g.eax;
    g.eax = *set;
    kept.back(g);
    ts.sp += 4;
    return g.eax;
}

/* ---- The game's end ---- */

constexpr u32 kFpuControl = 0x79f3f4;        // the x87's control word as the game runs it
constexpr u32 kStopSounds = 0x410aa0, kStopMoreSounds = 0x40feb0;  // native_frontend_options' kStopSound2, 4
constexpr u32 kNothingToEnd = 0x4a6f70;      // an empty function
constexpr u32 kGameThread = 0x7a3aac;
constexpr u32 kReleaseWindowThing = 0x4cdc50;

/* ea::endThread (0x4df870), a thread ended (TerminateThread, CloseHandle): its record in eax, the
 * stale handle the code's ebx; eax as it leaves it. */
bool endThread(App* app, ThreadState& ts, u32 thread)
{
    // (The stale handle is read only for the main thread, which the game's own is not.)
    return guest::direct<0x4df870, ea::endThread>(app, ts, thread, 0).has_value();
}

/* native_controls_race's every device's actuators off (0x471fb0), and native_controls_input's
 * DirectInput stopped (0x434fd0): eax as the first leaves it, then as the code has it. */
bool inputLetGo(App* app, ThreadState& ts)
{
    const std::optional<u32> off = guest::direct<0x471fb0, controls::actuatorsOff>(app, ts, 0);
    if (!off)
        return false;
    return guest::direct<0x434fd0, controls::stopInput>(app, ts);
}

/* 0x4b6250's core: what the game does as it ends: the x87 reset to its own control word, the
 * sounds stopped, the thread at 0x7a3aac ended, sub_4cdc50's release, the force feedback off,
 * DirectInput stopped and the screen given back to Windows (THRASH_restore).  eax as the last
 * call leaves it, which is the result; nullopt when the game stopped. */
std::optional<u32> gameEnded(App* app, ThreadState& ts)
{
    // the x87 reset to its own control word: nothing to do (fp::)
    if (!(called(app, ts, kStopSounds) && called(app, ts, kStopMoreSounds)
          && viaCore<kNothingToEnd, frontend::loopa::nothingToEnd>(app, ts) && endThread(app, ts, kGameThread)
          && viaCore<kReleaseWindowThing, frontend::wina::hookOff>(app, ts) && inputLetGo(app, ts)))
        return std::nullopt;
    return driven<thrash::kThrashRestore, &port::driver::restore>(app, ts);
}

/* 0x4b6250: gameEnded. */
nfs::u32 shutDown(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const Kept kept(g, kEcx | kEdx);
    if (!gameEnded(app, ts).has_value())
        return g.eax;
    kept.back(g);
    ts.sp += 4;
    return g.eax;
}

/* ---- The loading screen ---- */

constexpr u32 kLoadingTextShown = 0x79f288;  // the loading text is up; cleared once "Aborted" is
constexpr u32 kLoadingTextX = 0x79f280;      // where the loading text ends, a float (native_render)
constexpr u32 kModeRect = 0x7cdac0;          // the mode's rectangle: x, y, width, height
constexpr u32 kGetRect = 0x4beb20;           // eax a rectangle into edx, ebx, ecx and [esp+4]
constexpr u32 kTextScale = 0x791b54;
constexpr u32 kSetTextScale = 0x4d1390;      // [esp+4] the scale, the last one onto the x87's stack
constexpr u32 kTextColour = 0x561b34, kTextMeasuring = 0x561b3c;  // port_text's
constexpr u32 kLanguageText = 0x4d1850;      // port_text's languageText
constexpr u32 kAbortedLine = 0xeb;           // "Aborted"
constexpr u32 kDotsFormat = 0x53bdc8;        // "... %s"
constexpr u32 kSprintf = 0x4df690;
constexpr u32 kHeightPer480 = 0x53bdd0;      // 1/480, a float
constexpr u32 kAbortedRow = 0x53bdd4;        // 451, a float: the row on a 480 line screen
constexpr u32 kChop = 0x4dfd56;              // Watcom's __CHP: st(0) towards zero
constexpr u32 kDrawText = 0x4d1000;          // port_text's drawText, its width onto the x87's stack
constexpr u32 kFlushText = 0x4d1340;

/* sub_4beb20, a rectangle copied out, by address: eax the rectangle, its x, y, width and height into
 * [edx], [ebx], [ecx] and [[esp+4]] -- that one pushed, and esp left as the callee leaves it, as the
 * code leaves it. */
bool copyRect(App* app, ThreadState& ts, u32 rect, u32 x, u32 y, u32 width, u32 height)
{
    // The word the code pushes is left under esp, as the code leaves it (the callee's ret does not take it): the
    // frame and the later calls' words lie 4 bytes lower for it.
    push(app, ts, height);
    return viaCore<kGetRect, frontend::pause::readRect>(app, ts, rect, x, y, width, height);
}

/* Watcom's __CHP (0x4dfd56) on `value`, as C++ (watcom::clib::chop): chopped towards 0 and stored as
 * an integer (fistp).  nullopt when the game stopped. */
std::optional<s32> chop(App* app, ThreadState& ts, nfs::Float value)
{
    const double whole = guest::direct<kChop, watcom::clib::chop>(app, ts, double(value));
    if (ts.terminate)
        return std::nullopt;
    return fp::toInteger<s32>(whole);
}

/* port_text's text scale set (0x4d1390, stdcall: port::text::textScaleSet): the last scale, which
 * the code drops (fstp st(0)). */
bool setTextScale(App* app, ThreadState& ts, u32 scale)
{
    guest::direct<kSetTextScale, port::text::textScaleSet>(app, ts, scale);
    return !ts.terminate;
}

/* port_text's languageText (0x4d1850: port::text::lineText): line eax of the game's text, edx handed
 * on as the code has it. */
std::optional<u32> languageText(App* app, ThreadState& ts, u32 line)
{
    g.eax = line;
    const port::Result text =
        guest::direct<kLanguageText, port::text::lineText>(app, ts, ts.sp - 4, s32(line), g.edx);
    if (!text || ts.terminate)
        return std::nullopt;
    g.eax = u32(*text);
    return g.eax;
}

/* port_text's drawText (0x4d1000: port::text::textDrawn): text eax at x edx, row ebx, as the code
 * has them; its width, which the code drops (fstp st(0)), and eax as it leaves it. */
bool drawText(App* app, ThreadState& ts, u32 text, u32 x, u32 row)
{
    g.eax = text;
    g.edx = x;
    g.ebx = row;
    const port::text::WidthResult drawn =
        guest::direct<kDrawText, port::text::textDrawn>(app, ts, ts.sp - 4, text, s32(x), s32(row));
    if (!drawn || ts.terminate)
        return false;
    g.eax = drawn->eax;
    return true;
}

constexpr u32 kAbortedColour = 0xffe0e000;   // yellow

/* 0x494bd0's core: a loading cut short: when the loading text is up, "... Aborted" drawn after it
 * -- in yellow, at row 451 of 480 scaled to the mode -- and the picture flushed and flipped; the
 * loading text then counts as gone.  The rectangle's and the text's buffers on the guest's stack, as
 * deep as the code has them; the registers at the calls as it has them (ecx 0 from the text's
 * measuring on).  eax as the last call leaves it. */
bool loadingAborted(App* app, ThreadState& ts)
{
    if (at(app, kLoadingTextShown) == 0)
        return true;
    const GuestFrame frame(ts, 0x10, 0x118);
    const u32 text = frame - 0x118;
    if (!copyRect(app, ts, kModeRect, frame - 8, frame - 0x18, frame - 0x14, frame - 0x10))
        return false;
    const u32 height = at(app, frame - 0x10);
    const float scale = float(fp::mul(nfs::Float(s32(height)), nfs::Float(floatAt(app, kHeightPer480))));
    put(app, kLoadingTextShown, 0);
    g.eax = height;
    g.ecx = 0;
    if (!guest::direct<0x431aa0, collide::textStates>(app, ts))
        return false;
    g.ebx = kAbortedColour;
    if (!setTextScale(app, ts, at(app, kTextScale)))
        return false;
    put(app, kTextColour, kAbortedColour);
    put(app, kTextMeasuring, 0);
    const std::optional<u32> aborted = languageText(app, ts, kAbortedLine);
    if (!aborted)
        return false;
    g.eax = guest::direct<kSprintf, watcom::clib::sprintf>(app, ts, text, kDotsFormat, guest::Words{*aborted});
    if (ts.terminate)
        return false;
    const nfs::Float row = fp::mul(nfs::Float(scale), nfs::Float(floatAt(app, kAbortedRow)));
    g.eax = text;
    const std::optional<s32> x = chop(app, ts, nfs::Float(floatAt(app, kLoadingTextX)));
    if (!x)
        return false;
    g.edx = u32(*x);
    const std::optional<s32> y = chop(app, ts, row);
    return y && drawText(app, ts, text, u32(*x), u32(*y)) && viaPort<kFlushText, port::text::fontsDrawn>(app, ts)
        && flushFlip(app, ts);
}

/* 0x494bd0: loadingAborted. */
nfs::u32 drawLoadingAborted(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const Kept kept(g, kEbx | kEcx | kEdx);
    if (!loadingAborted(app, ts))
        return g.eax;
    kept.back(g);
    ts.sp += 4;
    return g.eax;
}

/* ---- A race taken down ---- */

constexpr u32 kMenuScreenMode = 0x55fec8;    // the screen mode the menus run in
constexpr u32 kInRace = 0x559234;            // native_frontend_screen's: the menus are a race's pause screens
constexpr u32 kMenuView = 0x7cdb68;
constexpr u32 kSetViewAngle = 0x4bed50;      // eax a view
constexpr u32 kSetViewScale = 0x4bed70;      // eax a view
constexpr u32 kViewScale = 0x560094, kViewOffset = 0x560098;
constexpr u32 kRaceEndExtra = 0x495600;      // for eax nonzero
// The race's own ends by address: before raceload's controls' tick stopped (0x42dd00), and after it
// up to the track let go of (0x41da60).
constexpr u32 kRaceEndsFirst[] = {0x4bc950, 0x4bcd60}, kRaceEndsThen[] = {0x478000, 0x4c4830, 0x4782d0};

}  // namespace graphics

void raceRunning(bool running);  // nfs3hp_main.cpp: the port's mark of a race (the port's race state change)

namespace graphics
{

/* ---- What the later sections share ---- */

constexpr u32 kModeWidth = 0x7cdac8;
constexpr u32 kTextWidth = 0x4d1480;         // port_text's textWidthWithOverhang: onto the x87's stack
constexpr u32 kTextHeight = 0x4d1540;        // eax a font: its height
constexpr u32 kTraceWord = 4;                // raceload::noHook's (the game's trace): a word pushed

/* raceload::noHook (0x42d500, the game's trace), its word `word` pushed as the code pushes it (it
 * reads none).  False when the game stopped. */
bool traced(App* app, ThreadState& ts, u32 word)
{
    const GuestFrame pushed(ts, 0, kTraceWord);
    put(app, pushed - kTraceWord, word);
    guest::direct<0x42d500, raceload::noHook>(app, ts);
    return !ts.terminate;
}

/* The loading bar's quad: its corners' clip codes into their +0x14, as the menus' quads work them
 * out: 0x10 for a +0xc negative or behind the far edge, else 8 above, 4 below, 1 left of, 2 right of the
 * rectangle -- each a float's bits compared as integers, a negative one counting as short. */
constexpr u32 kClipFar = 0x7d34f8, kClipTopEdge = 0x7d3504, kClipBottomEdge = 0x7d3500;
constexpr u32 kClipLeftEdge = 0x7d34fc, kClipRightEdge = 0x7d350c;  // a float's bits, compared as integers
constexpr u32 kCornerClipCode = 0x14;

/* Whether a float's bits are short of `edge`'s, compared as integers: a negative one always is. */
bool shortOf(u32 bits, u32 edge)
{
    return (bits & 0x80000000) || s32(bits) < s32(edge);
}

void putClipCode(App* app, u32 corner)
{
    const u32 w = at(app, corner + 0xc);
    u8 code = 0;
    if ((w & 0x80000000) || s32(w) >= s32(at(app, kClipFar)))
        code = 0x10;
    else
    {
        const u32 y = at(app, corner + 4);
        if (shortOf(y, at(app, kClipTopEdge)))
            code |= 8;
        else if (s32(y) > s32(at(app, kClipBottomEdge)))
            code |= 4;
        const u32 x = at(app, corner);
        if (shortOf(x, at(app, kClipLeftEdge)))
            code |= 1;
        else if (s32(x) > s32(at(app, kClipRightEdge)))
            code |= 2;
    }
    putByte(app, corner + kCornerClipCode, code);
}

constexpr u32 kMenuViewScale = 0x3f800000;   // 1.0f
constexpr u32 kPointerWidth = 640, kPointerHeight = 480;

/* 0x4a35d0's core: the race taken down, back to the menus: their screen mode, the menus' view at
 * 90 and scale 1, the pointer kept to 640 by 480, both buffers cleared to black, sub_495600 for
 * `extra` nonzero, and the race's own ends (sub_4bc950 on).  The port marks the race over first.
 * The view's projection takes the 90 and the cleared ebx (not mirrored) the code hands it; the
 * 1.0f the code has in ecx at the clip's call is not the clip's (it stores the scale itself).
 * What the last end leaves in eax, or nullopt when the game stopped. */
std::optional<u32> raceTakenDown(App* app, ThreadState& ts, u32 extra)
{
    if (diag::raceDown)
        diag::raceDown(app);
    raceRunning(false);
    put(app, kInRace, 0);
    if (!guest::direct<0x4bef50, render::screenModeSet>(app, ts, at(app, kMenuScreenMode)))
        return std::nullopt;
    // The menus' view at 90 degrees, not mirrored: the code's `mov edx, 0x5a` and the ebx it cleared
    // before the screen mode's call (xor ebx, ebx).
    if (!viaCore<kSetViewAngle, frontend::loopb::projectionOnly>(app, ts, kMenuView, 0x5au, 0u))
        return std::nullopt;
    if (!viaCore<kSetViewScale, frontend::loopb::useClip>(app, ts, kMenuView))
        return std::nullopt;
    put(app, kViewScale, kMenuViewScale);
    put(app, kViewOffset, 0);
    if (!guest::direct<0x4352d0, controls::boundPointer>(app, ts, u32(0), u32(0), kPointerWidth, kPointerHeight)
        || !stateSent(app, ts, thrash::kStateClearColour, 0))
        return std::nullopt;
    // (The answer of the last state sent is the eax the code has from here.)
    u32 eax = g.eax;
    for (const u32 buffer : {2u, 1u})
    {
        if (!driven<thrash::kThrashWindow, &port::driver::window>(app, ts, buffer))
            return std::nullopt;
        const std::optional<u32> answer = clearDepthAnswer(app, ts);
        if (!answer)
            return std::nullopt;
        eax = *answer;
    }
    // backImage hands the registers it came with on to resetScreens and artTexturesOf, as the
    // code's call by address left them.
    if (extra != 0
        && !viaCore<kRaceEndExtra, frontend::rendc::backImage>(app, ts, eax, g.edx, g.ebx, g.ecx))
        return std::nullopt;
    if (!viaCore<0x4bc950, frontend::sime::effectsFree>(app, ts))
        return std::nullopt;
    if (!viaCore<0x4bcd60, frontend::loopb::replayEnd>(app, ts))
        return std::nullopt;
    if (!guest::direct<0x42dd00, raceload::stopTick>(app, ts))
        return std::nullopt;
    if (!viaCore<0x478000, frontend::menuf::raceEndEmpty>(app, ts))
        return std::nullopt;
    if (!viaCore<0x4c4830, frontend::loopb::teardown>(app, ts))
        return std::nullopt;
    if (!viaCore<0x4782d0, frontend::loada::horizonFreeCall>(app, ts))
        return std::nullopt;
    const std::optional<u32> left = guest::direct<0x41da60, track::letGoOfTrack>(app, ts);
    if (!left)
        return std::nullopt;
    return left;
}

/* 0x4a35d0: raceTakenDown, sub_495600 for eax nonzero; eax what it leaves. */
nfs::u32 takeDownRace(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const Kept kept(g, kEbx | kEcx | kEdx);
    const std::optional<u32> left = raceTakenDown(app, ts, g.eax);
    if (!left)
        return g.eax;
    g.eax = *left;
    kept.back(g);
    ts.sp += 4;
    return g.eax;
}

/* ---- The polygon lists' drawers for voodoo2a ---- */

constexpr u32 kDrawLine = 0x4c0b20;          // native_render's line: eax, edx its ends
constexpr u32 kDrawQuad = 0x4c12e0;          // native_render's polygon: corners eax, edx, ebx, ecx

// A polygon list's record: +0 the next, +6 its flags (bit 0 blended), +8.. its corners (vertices),
// +0x18 its texture, +0x20.. the corners' texture coordinates (two floats each), +0x40.. their
// colours.  A vertex's +0x10 its colour, +0x18 and +0x1c its texture coordinates.
constexpr u32 kRecordNext = 0, kRecordFlags = 6, kRecordCorners = 8, kRecordTexture = 0x18;
constexpr u32 kRecordCoordinates = 0x20, kRecordColours = 0x40;
constexpr u32 kVertexColour = 0x10, kVertexCoordinates = 0x18;

/* A kept state sent as the drawers send it: kept at `keptAt`, then THRASH_setstate. */
bool sendKept(App* app, ThreadState& ts, u32 keptAt, u32 state, u32 value)
{
    put(app, keptAt, value);
    return setState(app, ts, state, value).has_value();
}

/* native_render's line (0x4c0b20), by address: its ends in eax and edx. */
bool drawLine(App* app, ThreadState& ts, u32 from, u32 to)
{
    const std::optional<u32> left = viaValue<kDrawLine, render::lineDrawn>(app, ts, from, to);
    if (!left)
        return false;
    g.eax = *left;
    return true;
}

/* 0x4347d0's core: a line of a polygon list (its two ends the record's first two corners) drawn
 * untextured, blended for bit 0 of its flags. */
bool lineRecord(App* app, ThreadState& ts, u32 record)
{
    const u32 to = at(app, record + kRecordCorners + 4), from = at(app, record + kRecordCorners);
    if (at(app, kKeptTexture) != 0 && !sendKept(app, ts, kKeptTexture, thrash::kStateTexture, 0))
        return false;
    const u32 blend = byte(app, record + kRecordFlags) & 1;
    if (at(app, kKeptBlend) != blend && !sendKept(app, ts, kKeptBlend, thrash::kStateBlend, blend))
        return false;
    return drawLine(app, ts, from, to);
}

/* 0x4347d0: lineRecord for the record in eax; eax the next record. */
nfs::u32 drawLineRecord(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const Kept kept(g, kEbx | kEcx | kEdx);
    const u32 record = g.eax;
    if (!lineRecord(app, ts, record))
        return g.eax;
    g.eax = at(app, record + kRecordNext);
    kept.back(g);
    ts.sp += 4;
    return g.eax;
}

/* native_render's polygon (0x4c12e0), by address: its four corners in eax, edx, ebx, ecx. */
bool drawQuad(App* app, ThreadState& ts, u32 first, u32 second, u32 third, u32 fourth)
{
    const std::optional<u32> left = viaValue<kDrawQuad, vertices::quadClipped>(app, ts, first, second, third, fourth);
    if (!left)
        return false;
    g.eax = *left;
    return true;
}

/* 0x433d40's core: a quad of a polygon list with colours of its own, drawn unblended: each
 * corner's colour, and with a texture its coordinates, copied to its vertex, the texture sent when
 * it changed (or none when there is none), then native_render's polygon.  Each field read where the
 * code reads it: a vertex may be the record's own memory. */
bool colouredQuadRecord(App* app, ThreadState& ts, u32 record)
{
    if (at(app, kKeptBlend) != 0 && !sendKept(app, ts, kKeptBlend, thrash::kStateBlend, 0))
        return false;
    const auto vertex = [&](u32 corner) { return at(app, record + kRecordCorners + 4 * corner); };
    for (u32 corner = 0; corner < 4; ++corner)
    {
        const u32 to = vertex(corner);
        put(app, to + kVertexColour, at(app, record + kRecordColours + 4 * corner));
    }
    if (at(app, record + kRecordTexture) != 0)
    {
        for (u32 corner = 0; corner < 4; ++corner)
            for (u32 half = 0; half < 2; ++half)
            {
                const u32 to = vertex(corner);
                put(app, to + kVertexCoordinates + 4 * half,
                    at(app, record + kRecordCoordinates + 8 * corner + 4 * half));
            }
        const u32 kept = at(app, kKeptTexture), texture = at(app, record + kRecordTexture);
        if (kept != texture && !sendKept(app, ts, kKeptTexture, thrash::kStateTexture, texture))
            return false;
    }
    else if (at(app, kKeptTexture) != 0 && !sendKept(app, ts, kKeptTexture, thrash::kStateTexture, 0))
        return false;
    const u32 fourth = vertex(3), third = vertex(2), second = vertex(1), first = vertex(0);
    return drawQuad(app, ts, first, second, third, fourth);
}

/* 0x433d40: colouredQuadRecord for the record in eax; eax the next record. */
nfs::u32 drawColouredQuadRecord(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const Kept kept(g, kEbx | kEcx | kEdx);
    const u32 record = g.eax;
    if (!colouredQuadRecord(app, ts, record))
        return g.eax;
    g.eax = at(app, record + kRecordNext);
    kept.back(g);
    ts.sp += 4;
    return g.eax;
}

// A mesh of lines gathered: a count of ends, the ends from +4, the pairs' indices from +0x8004.
constexpr u32 kMeshEnds = 4, kMeshPairs = 0x8004;

/* 0x49bf60's core: the mesh of lines gathered at `mesh` drawn untextured (THRASH_drawlinemesh) and
 * its count set back to 0, when there is one.  eax at the driver's call as the code has it: the
 * lines' count.  The result is eax as that call, or the mesh's own address when there is nothing to draw (the
 * code's eax then), leaves it; nullopt when the game stopped. */
std::optional<u32> lineMeshFlushed(App* app, ThreadState& ts, u32 mesh)
{
    if (at(app, mesh) == 0)
        return mesh;
    if (!stateSent(app, ts, thrash::kStateTexture, 0))
        return std::nullopt;
    const u32 lines = u32(s32(at(app, mesh)) / 2);  // the code's sar of it less its sign: towards 0
    g.eax = lines;
    const std::optional<u32> drawn = driven<thrash::kThrashDrawLineMesh, &port::driver::drawLineMesh>(app, ts, lines, mesh + kMeshEnds, mesh + kMeshPairs);
    if (!drawn)
        return std::nullopt;
    put(app, mesh, 0);
    return drawn;
}

/* 0x49bf60: lineMeshFlushed for the mesh at eax. */
nfs::u32 flushLineMesh(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const Kept kept(g, kEbx | kEcx | kEdx);
    if (!lineMeshFlushed(app, ts, g.eax))
        return g.eax;
    kept.back(g);
    ts.sp += 4;
    return g.eax;
}

/* ---- Textures ---- */

constexpr u32 kNoTextureFormat = 0x53c5f0;   // the Error's text: the driver takes neither format
constexpr u32 kTextureBufferTag = 0x53c60c;  // what the conversion's buffer is allocated as

// THRASH_about's record: +0x30 the texture formats it takes, each with its flags (bit 0: taken).
constexpr u32 kAboutFormats = 0x30, kFormat1555Flags = 0xc, kFormat4444Flags = 0x1c;
constexpr u32 kFormat1555 = 3, kFormat4444 = 7;

/* 0x49c5c0's core: the format the game's 16-bit textures are handed to the driver in: 3 (1555)
 * when THRASH_about's formats take it, else 7 (4444), with an Error first when they take neither
 * (eax the formats there, as the code has it).  ecx and edx kept and eax the format, as the code
 * leaves them. */
std::optional<u32> textureFormat(App* app, ThreadState& ts)
{
    const Kept kept(g, kEcx | kEdx);
    const std::optional<u32> about = driven<thrash::kThrashAbout, &port::driver::about>(app, ts);
    if (!about)
        return std::nullopt;
    const u32 formats = at(app, *about + kAboutFormats);
    if (byte(app, formats + kFormat1555Flags) & 1)
    {
        g.eax = kFormat1555;
        kept.back(g);
        return kFormat1555;
    }
    g.eax = formats;
    if (!(byte(app, formats + kFormat4444Flags) & 1))
    {
        // As deep as the code calls the Error, under its three pushes: it hands vsprintf an
        // address on the stack.
        const GuestFrame deep(ts, 0xc, 0);
        if (!error(app, ts, kNoTextureFormat))
            return std::nullopt;
    }
    g.eax = kFormat4444;
    kept.back(g);
    return kFormat4444;
}

/* 0x49c5c0: textureFormat. */
nfs::u32 textureFormatFor(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    if (textureFormat(app, ts))
        ts.sp += 4;
    return g.eax;
}

/* The runtime's allocator (ea::reservememadr, 0x4e1620): eax the tag, edx the bytes, ebx 0x10 --
 * which it keeps, the code's after it.  The block. */
std::optional<u32> allocate(App* app, ThreadState& ts, u32 tag, u32 bytes)
{
    g.edx = bytes;
    g.ebx = 0x10;
    return guest::direct<0x4e1620, ea::reserved>(app, ts, tag, bytes, u32(0x10));
}

/* The runtime's free (ea::purgememadr, 0x4e1890): the block in eax. */
bool release(App* app, ThreadState& ts, u32 block)
{
    return guest::direct<0x4e1890, ea::purgeBlock>(app, ts, block).has_value();
}

/* A 1555 pixel as 4444: 0 for a transparent one (bit 15 clear), else each colour's top four bits
 * and the alpha's.  The code's shifts are of the pixel sign-extended. */
u16 pixel4444(s32 pixel)
{
    if ((pixel >> 15) == 0)
        return 0;
    return u16(u32(pixel >> 3 & 0xff00) | u32(pixel >> 2 & 0xf0f0) | u32(pixel >> 1 & 0xf00f));
}

/* 0x49c620's core: `texture` given the `width` by `height` 1555 pixels at `pixels`: straight
 * (THRASH_tupdate) when the driver takes 1555, else turned into 4444 in a block of their own first.
 * textureFormat gives one of the two only: the code's Error for another format is never reached.
 * ebx, ecx and edx at the driver's call as the conversion leaves them; eax as the last call
 * leaves it. */
bool textureUpdated(App* app, ThreadState& ts, u32 texture, u32 pixels, u32 width, u32 height)
{
    std::optional<u32> format;
    {
        const GuestFrame deep(ts, 0x20, 0);  // the code's pushes, its frame and the call's slot, for the Error's
        format = textureFormat(app, ts);
    }
    if (!format)
        return false;
    if (*format == kFormat1555)
        return driven<thrash::kThrashTUpdate, &port::driver::tUpdate>(app, ts, texture, pixels, 0).has_value();
    const u32 count = width * height;
    const std::optional<u32> block = allocate(app, ts, kTextureBufferTag, count + count);
    if (!block)
        return false;
    u32 done = 0;
    for (; s32(done) < s32(count); ++done)
        putHalf(app, *block + 2 * done, pixel4444(nfs::s16(half(app, pixels + 2 * done))));
    g.ebx = *block;
    g.ecx = done;
    g.edx = *block + 2 * done;
    return driven<thrash::kThrashTUpdate, &port::driver::tUpdate>(app, ts, texture, *block, 0) && release(app, ts, *block);
}

/* 0x49c620: textureUpdated, texture eax, pixels edx, ebx by ecx of them. */
nfs::u32 updateTexture(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    if (textureUpdated(app, ts, g.eax, g.edx, g.ebx, g.ecx))
        ts.sp += 4;
    return g.eax;
}

constexpr u32 kMemoryWarnings = 0x4b55f8, kMemoryWarningNames = 0x4b5610;  // six words each
constexpr u32 kMemoryWarningFormat = 0x53fde0;
constexpr u32 kStandardError = 0x566f72;     // the runtime's stderr
constexpr u32 kFprintf = 0x4f4b80;
constexpr u32 kDriverDirect3D = 6;
// THRASH_about's record: +0x70 the texture memory, +0x74 Direct3D's more.
constexpr u32 kAboutTextureMemory = 0x70, kAboutMoreMemory = 0x74;
constexpr s32 kEnoughTextureMemory = 0x200000;

/* 0x4b5ea0's core: for driver type `type` (Direct3D counts THRASH_about's more memory too), a line
 * to stderr when the texture memory is under 2 MB, with the words of the tables at 0x4b55f8 and
 * 0x4b5610 for `index` -- copied into the frame first and read there, as deep as the code has them
 * (an index past them reads what is there).  edx the driver's record, as the code leaves it.  What
 * eax is left: the memory, or fprintf's answer. */
std::optional<u32> textureMemoryWarned(App* app, ThreadState& ts, u32 type, u32 index)
{
    const std::optional<u32> about = driven<thrash::kThrashAbout, &port::driver::about>(app, ts);
    if (!about)
        return std::nullopt;
    g.edx = *about;
    u32 memory = at(app, *about + kAboutTextureMemory);
    if (type == kDriverDirect3D)
        memory += at(app, *about + kAboutMoreMemory);
    if (s32(memory) >= kEnoughTextureMemory)
        return memory;
    const GuestFrame frame(ts, 0x14, 0x30);
    for (u32 i = 0; i < 6; ++i)
    {
        put(app, frame - 0x30 + 4 * i, at(app, kMemoryWarnings + 4 * i));
        put(app, frame - 0x18 + 4 * i, at(app, kMemoryWarningNames + 4 * i));
    }
    const u32 warning = at(app, (frame - 0x30) + index * 4), name = at(app, (frame - 0x18) + index * 4);
    g.eax = memory;
    const u32 printed = guest::direct<kFprintf, watcom::clib::fprintf>(app, ts, kStandardError, kMemoryWarningFormat,
                                                                         guest::Words{name, memory, warning});
    if (ts.terminate)
        return std::nullopt;
    g.eax = printed;
    return printed;
}

/* 0x4b5ea0: textureMemoryWarned for driver type eax, index edx. */
nfs::u32 warnTextureMemory(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const Kept kept(g, kEbx | kEcx);
    const std::optional<u32> left = textureMemoryWarned(app, ts, g.eax, g.edx);
    if (!left)
        return g.eax;
    g.eax = *left;
    kept.back(g);
    ts.sp += 4;
    return g.eax;
}

// THRASH_about's record: +0x3c the last mode, +0x40 the modes (0x28 bytes each: width, height...).
constexpr u32 kAboutLastMode = 0x3c, kAboutModes = 0x40, kModeBytes = 0x28;

/* What modeSizeOf found: whether the mode is one of THRASH_about's, and its size. */
struct ModeSize
{
    bool listed;
    u32 width, height;
};

/* 0x4be570's core: screen mode `mode`'s width into [widthAt] and height into [heightAt] from
 * THRASH_about's modes, 640 by 480 past the last; [zeroAt] and [alsoZeroAt] set to 0 first. */
std::optional<ModeSize> modeSizeOf(App* app, ThreadState& ts, u32 mode, u32 zeroAt, u32 alsoZeroAt, u32 widthAt,
                                   u32 heightAt)
{
    const std::optional<u32> about = driven<thrash::kThrashAbout, &port::driver::about>(app, ts);
    if (!about)
        return std::nullopt;
    put(app, zeroAt, 0);
    put(app, alsoZeroAt, 0);
    if (s32(mode) > s32(at(app, *about + kAboutLastMode)))
    {
        put(app, widthAt, kDefaultWidth);
        put(app, heightAt, kDefaultHeight);
        return ModeSize{false, kDefaultWidth, kDefaultHeight};
    }
    const u32 width = at(app, at(app, *about + kAboutModes) + mode * kModeBytes);
    put(app, widthAt, width);
    const u32 height = at(app, at(app, *about + kAboutModes) + mode * kModeBytes + 4);
    put(app, heightAt, height);
    return ModeSize{true, width, height};
}

/* 0x4be570's core without the size: the same, false when the game stopped. */
bool modeSized(App* app, ThreadState& ts, u32 mode, u32 zeroAt, u32 alsoZeroAt, u32 widthAt, u32 heightAt)
{
    return modeSizeOf(app, ts, mode, zeroAt, alsoZeroAt, widthAt, heightAt).has_value();
}

/* 0x4be570: modeSizeOf for mode eax, the zeros at edx and ebx, the width into [ecx], the height into
 * [esp+4]; eax as the code leaves it, the height or, past the last mode, where it went (edx, ebx and
 * ecx its arguments, no caller's after it). */
nfs::u32 modeSize(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const u32 heightAt = at(app, ts.sp + 4);
    const std::optional<ModeSize> size = modeSizeOf(app, ts, g.eax, g.edx, g.ebx, g.ecx, heightAt);
    if (!size)
        return g.eax;
    g.eax = size->listed ? size->height : heightAt;
    ts.sp += 4 + 4;
    return g.eax;
}

/* ---- Text ---- */

constexpr u32 kTextQuad = 0x561b44;          // four corners of kCornerBytes: x, y, ..., the colour at +0x10
constexpr u32 kTextBoxColour = 0x561b38;
constexpr u32 kFontHeights = 0x83f6a0, kFont = 0x840184, kFontScale = 0x84018c;

/* A float's bits as the float. */
float floatOf(u32 bits)
{
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

/* 0x4d1120's core: a box drawn untextured behind text: at x, y, `width` wide (times the font's
 * scale) and the font's height (its table at 0x83f6a0 for [0x840184], times the scale) high, in
 * [0x561b38] made opaque.  The x87's operations as the code does them, each float stored where it
 * stores one. */
bool textBox(App* app, ThreadState& ts, s32 x, s32 y, float width)
{
    const u32 font = at(app, kFont);
    const nfs::Float scale = nfs::Float(floatAt(app, kFontScale));
    const nfs::Float scaledWidth = fp::mul(nfs::Float(width), scale);
    const nfs::Float height = fp::mul(scale, nfs::Float(s32(at(app, kFontHeights + font * 4))));
    const u32 colour = at(app, kTextBoxColour) | 0xff000000;
    putFloat(app, kTextQuad, float(nfs::Float(x)));
    putFloat(app, kTextQuad + 4, float(nfs::Float(y)));
    const u32 left = at(app, kTextQuad);
    putFloat(app, kTextQuad + 0x24, float(nfs::Float(y)));
    for (u32 corner = 3; corner < 4; --corner)
        put(app, kTextQuad + corner * kCornerBytes + kCornerColour, colour);
    const nfs::Float top = nfs::Float(floatAt(app, kTextQuad + 0x24));
    put(app, kTextQuad + 0x60, left);
    const nfs::Float right = fp::add(scaledWidth, nfs::Float(floatOf(left)));
    const nfs::Float bottom = fp::add(height, top);
    putFloat(app, kTextQuad + 0x20, float(right));
    putFloat(app, kTextQuad + 0x44, float(bottom));
    put(app, kTextQuad + 0x40, at(app, kTextQuad + 0x20));
    putFloat(app, kTextQuad + 0x64, float(bottom));
    const auto corner = [](u32 n) { return kTextQuad + n * kCornerBytes; };
    return stateSent(app, ts, thrash::kStateTexture, 0)
        && driven<thrash::kThrashDrawQuad, &port::driver::drawQuad>(app, ts, corner(0), corner(1), corner(2), corner(3));
}

/* 0x4d1120: textBox at x eax, y edx, [esp+4] wide. */
nfs::u32 drawTextBox(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const Kept kept(g, kEcx);
    if (!textBox(app, ts, s32(g.eax), s32(g.edx), floatAt(app, ts.sp + 4)))
        return g.eax;
    kept.back(g);
    ts.sp += 4 + 4;
    return g.eax;
}

/* ---- The menus' art in textures ---- */

constexpr u32 kShapeBufferTag = 0x5495c4;
constexpr u32 kShapeToPixels = 0x4d37a0;     // eax the buffer, edx the shape, ebx and [esp+4] where its size goes

/* port::shapes' shape into pixels (0x4d37a0), by address: eax the buffer, edx the shape, ebx and
 * [esp+4] where its size goes (ecx as the code has it); esp left as the
 * callee leaves it, as the code leaves it. */
bool shapeToPixels(App* app, ThreadState& ts, u32 buffer, u32 shape, u32 widthAt, u32 heightAt)
{
    const std::optional<u32> height = viaValue<kShapeToPixels, port::shapes::shapeToPixelsOf>(
        app, ts, ts.sp - 8, buffer, shape, widthAt, s32(g.ecx), heightAt);
    if (!height)
        return false;
    g.eax = *height;
    return true;
}

/* 0x4d5ed0's core: `texture` filled from `shape` (`size` by `size` pixels at most): the shape turned
 * into pixels (sub_4d37a0) in a block of its own -- its size into the frame, as deep as the code has
 * it, the height's slot holding `limit` first -- handed to THRASH_tupdate from its +0x10, and the
 * block freed.  ebx left the width's slot, as the code leaves it. */
bool shapeIntoTextureOf(App* app, ThreadState& ts, u32 texture, u32 shape, u32 size, u32 limit)
{
    const GuestFrame frame(ts, 0xc, 0x10);
    const u32 heightAt = frame - 0x10, widthAt = frame - 0xc;
    put(app, heightAt, limit);
    g.ecx = size;
    const std::optional<u32> block = allocate(app, ts, kShapeBufferTag, ((size * size) << 2) + 0x14);
    if (!block || !shapeToPixels(app, ts, *block, shape, widthAt, heightAt))
        return false;
    g.eax = *block + 0x10;
    g.edx = texture;
    return driven<thrash::kThrashTUpdate, &port::driver::tUpdate>(app, ts, texture, *block + 0x10, 0) && release(app, ts, *block);
}

/* 0x4d5ed0: shapeIntoTextureOf, texture eax, shape edx, ebx by ebx at most, ecx into the height's
 * slot. */
nfs::u32 shapeIntoTexture(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    if (shapeIntoTextureOf(app, ts, g.eax, g.edx, g.ebx, g.ecx))
        ts.sp += 4;
    return g.eax;
}

/* ---- The screen's functions, the frame buffer, the driver closed ---- */

constexpr u32 kScreenFunctions = 0x567dc4;   // five: at exit, set mode, lock, unlock and flip, set up a screen
constexpr u32 kLockScreen = 0x517920;
constexpr u32 kReleaseAndFlip = 0x517990, kSetVideoMode = 0x517610;
constexpr u32 kNoAboutFile = 0x54d714, kNoAboutFunction = 0x54d724, kNoAboutLine = 0xe;
constexpr u32 kNoAbout = 0x54d72c;           // the Error's text: the driver has no THRASH_about

/* 0x4f8830's core: the screen's functions kept for the driver (0x567dc4 on), an Error first when it
 * has no THRASH_about -- as deep as the code calls it, under its six pushes: the Error hands
 * vsprintf an address on the stack. */
bool screenFunctionsSet(App* app, ThreadState& ts)
{
    if (at(app, thrash::kThrashAbout) == 0)
    {
        put(app, kWhereFile, kNoAboutFile);
        put(app, kWhereFunction, kNoAboutFunction);
        put(app, kWhereLine, kNoAboutLine);
        const GuestFrame deep(ts, 0x18, 0);
        if (!error(app, ts, kNoAbout))
            return false;
    }
    put(app, kScreenFunctions, kModeAtExit);
    put(app, kScreenFunctions + 8, kLockScreen);
    put(app, kScreenFunctions + 0xc, kReleaseAndFlip);
    put(app, kScreenFunctions + 0x10, kSetUpScreen);
    put(app, kScreenFunctions + 4, kSetVideoMode);
    return true;
}

/* 0x4f8830: screenFunctionsSet; eax 1. */
nfs::u32 setScreenFunctions(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const Kept kept(g, kEbx | kEcx | kEdx);
    if (!screenFunctionsSet(app, ts))
        return g.eax;
    g.eax = 1;
    kept.back(g);
    ts.sp += 4;
    return g.eax;
}

// A screen's record: +4 its width, +8 its height, +0x1c its bits per pixel, +0x20 its pixels,
// +0x28 its pitch.  THRASH_lockwindow's record: +0 the pixels, +4 the pitch, +8 the format, +0xc
// the width, +0x10 the height.
constexpr u32 kScreenWidthAt = 4, kScreenHeightAt = 8, kScreenBitsAt = 0x1c, kScreenPixels = 0x20,
                kScreenPitch = 0x28;
constexpr u32 kLockPixels = 0, kLockPitch = 4, kLockFormat = 8, kLockWidth = 0xc, kLockHeight = 0x10;

/* 0x517890's core: window `window` taken and the frame buffer locked (THRASH_lockwindow) for the
 * screen record `screen`: its pixels and pitch from the lock, which is kept, as is the record (edx
 * the pixels, as the code leaves it).  1, or 0 when it would not lock. */
std::optional<u32> frameBufferLocked(App* app, ThreadState& ts, u32 screen, u32 window)
{
    if (!driven<thrash::kThrashWindow, &port::driver::window>(app, ts, window))
        return std::nullopt;
    const std::optional<u32> lock = driven<thrash::kThrashLockWindow, &port::driver::lockWindow>(app, ts);
    if (!lock)
        return std::nullopt;
    put(app, kFrameLock, *lock);
    if (*lock == 0)
        return 0;
    g.edx = at(app, *lock + kLockPixels);
    put(app, screen + kScreenPixels, g.edx);
    const u32 pitch = at(app, *lock + kLockPitch);
    put(app, kFrameLocked, screen);
    put(app, screen + kScreenPitch, pitch);
    return 1;
}

/* 0x517890: frameBufferLocked for the record eax, window edx. */
nfs::u32 lockFrameBuffer(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const Kept kept(g, kEbx | kEcx);
    const std::optional<u32> locked = frameBufferLocked(app, ts, g.eax, g.edx);
    if (!locked)
        return g.eax;
    g.eax = *locked;
    kept.back(g);
    ts.sp += 4;
    return g.eax;
}

constexpr u32 kRemoveTask = 0x4e75e0;        // native_snd_stream's removeSystemTask: eax the task

/* port::removeSystemTask (0x4e75e0), by address: the task's function in eax. */
bool removeTask(App* app, ThreadState& ts, u32 function)
{
    g.eax = guest::direct<kRemoveTask, port::stream::unscheduleTask>(app, ts, function);
    return !ts.terminate;
}

/* 0x524650's core: the driver closed, when it is open: its idle task taken off the timer, the
 * screen given back (THRASH_restore), the game's window destroyed too for `destroyWindow`; ecx and
 * edx kept from then on, as the code keeps them. */
bool driverClosed(App* app, ThreadState& ts, bool destroyWindow)
{
    if (byte(app, kDriverOpen) == 0)
        return true;
    if (at(app, thrash::kThrashIdle) != 0 && !removeTask(app, ts, kDriverIdle))
        return false;
    const Kept kept(g, kEcx | kEdx);
    if (!driven<thrash::kThrashRestore, &port::driver::restore>(app, ts) || (destroyWindow && !viaCore<kDestroyWindow, frontend::wina::closeWindow>(app, ts)))
        return false;
    putByte(app, kDriverOpen, 0);
    kept.back(g);
    return true;
}

/* 0x524650: driverClosed, the window destroyed for eax nonzero; eax 0. */
nfs::u32 closeDriver(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const Kept kept(g, kEbx);
    if (!driverClosed(app, ts, g.eax != 0))
        return g.eax;
    g.eax = 0;
    kept.back(g);
    ts.sp += 4;
    return g.eax;
}

/* 0x524640: closeDriver with the game's window destroyed too: what the game does at its end
 * (openDriver hands it to sub_4f2a78).  The code runs on into closeDriver with eax 1. */
nfs::u32 closeDriverAtExit(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    g.eax = 1;
    closeDriver(app, ts, g);
    return g.eax;
}

/* ---- The drivers' pointers kept and taken back ---- */

constexpr u32 kDriverSlot = 0x564a68;        // the slot in use
constexpr u32 kDriverSlots = 0x564a70;       // eight of 0x8c bytes: the library, then 34 pointers
constexpr u32 kDriverLibrary = 0x564a6c;
constexpr u32 kSlotBytes = 0x8c, kSlotWords = 0x23, kSlotCount = 8;
/* Where each word of a slot goes, after the library: the game's pointers to
 * the driver's functions, in the order the slot keeps them. */
constexpr u32 kSlotPointers[] = {
    thrash::kThrashAbout, 0x9ef984, thrash::kThrashInit, thrash::kThrashRestore, 0x9ef944, thrash::kThrashSetVideoMode, thrash::kThrashTAlloc,
    thrash::kThrashTUpdate, thrash::kThrashTReset, thrash::kThrashWindow, thrash::kThrashClearWindow, thrash::kThrashFlushWindow, thrash::kThrashPageFlip,
    thrash::kThrashIdle, thrash::kThrashSync, thrash::kThrashClip, thrash::kThrashSetState, thrash::kThrashSetTexture, thrash::kThrashLockWindow,
    thrash::kThrashUnlockWindow, thrash::kThrashReadRect, 0x9ef914, thrash::kThrashDrawQuad, thrash::kThrashDrawQuadMesh, thrash::kThrashDrawTri,
    thrash::kThrashDrawTriMesh, 0x9ef91c, 0x9ef960, 0x9ef928, thrash::kThrashDrawLineMesh, 0x9ef908, 0x9ef910, 0x9ef900, 0x9ef904,
};

/* A slot's index: 0 for one outside 0..7. */
u32 slotIndex(u32 slot)
{
    return s32(slot) >= s32(kSlotCount) || s32(slot) < 0 ? 0 : slot;
}

/* 0x517170's core: the driver of slot `slot` taken back: its library and its pointers, the slot
 * kept as the one in use.  The slot's last word, which the code leaves in eax. */
u32 driverSlotUsed(App* app, u32 slot)
{
    const u32 index = slotIndex(slot);
    put(app, kDriverSlot, index);
    const u32 words = kDriverSlots + index * kSlotBytes;
    put(app, kDriverLibrary, at(app, words));
    for (u32 i = 0; i < sizeof kSlotPointers / sizeof *kSlotPointers; ++i)
        put(app, kSlotPointers[i], at(app, words + 4 + 4 * i));
    return at(app, words + kSlotBytes - 4);
}

/* 0x517170: driverSlotUsed, slot eax. */
nfs::u32 useDriverSlot(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    g.eax = driverSlotUsed(app, g.eax);
    ts.sp += 4;
    return g.eax;
}

/* 0x517370's core: the driver in use kept in slot `slot`: its library and its pointers.  The slot's
 * first word's index, which the code leaves in eax. */
u32 driverSlotKept(App* app, u32 slot)
{
    const u32 first = slotIndex(slot) * kSlotWords;
    const u32 words = kDriverSlots + first * 4;
    put(app, words, at(app, kDriverLibrary));
    for (u32 i = 0; i < sizeof kSlotPointers / sizeof *kSlotPointers; ++i)
        put(app, words + 4 + 4 * i, at(app, kSlotPointers[i]));
    return first;
}

/* 0x517370: driverSlotKept, slot eax. */
nfs::u32 keepDriverSlot(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    g.eax = driverSlotKept(app, g.eax);
    ts.sp += 4;
    return g.eax;
}

/* ---- A screen set up ---- */

constexpr u32 kBitmapScreen = 0x4ed820;      // eax its window, edx..: its size and depth
constexpr u32 kMakeBitmap = 0x508b80;
constexpr u32 kDepthCode = 0x5098d0;         // al a depth: the screen record's code for it
constexpr u32 kRowBuffer = 0x509a30;         // eax 0, edx a pitch or depth, ebx, ecx rows: a buffer
constexpr u32 kBitmapScreenWhich = 0xff;
constexpr u32 kScreenMagic = 0x57494e44;     // "DNIW"
// A screen's record, more of it: +0xc and +0x10 cleared, +0x14 and +0x18 its size again, +0x1d the
// depth's code, +0x1e which screen, +0x2c and +0x30 its two buffers, +0x34 its window.
constexpr u32 kScreenCleared = 0xc, kScreenSizeAgain = 0x14, kScreenDepthCode = 0x1d, kScreenWhich = 0x1e,
                kScreenRows = 0x2c, kScreenMoreRows = 0x30, kScreenWindow = 0x34;

/* sub_509a30, a buffer of `rows` rows, by address: eax 0, edx a pitch or depth, ebx a kind, ecx the
 * rows.  The buffer. */
std::optional<u32> rowBuffer(App* app, ThreadState& ts, u32 pitch, u32 kind, u32 rows)
{
    g.eax = 0;
    g.edx = pitch;
    g.ebx = kind;
    g.ecx = rows;
    return game(app, ts, kRowBuffer);
}

/* 0x5179d0's core: screen `which` set up in `record`, `window` its window: nothing for 0; for 0xff
 * a bitmap of the window's size (sub_4ed820 into the frame, as deep as the code has it, then
 * sub_508b80); otherwise the driver's buffer `which` locked once for its pixels, pitch, size and
 * depth, then the record filled ("DNIW", its size again, which screen, the depth's code, the
 * window) with two buffers of its own.  The registers each call takes as the code has them. */
bool screenSetUp(App* app, ThreadState& ts, u32 which, u32 window, u32 record)
{
    if (which == kBitmapScreenWhich)
    {
        // sub_4ed820's five numbers, in the frame: [edx], [ebx], [ecx] and the two pushed.
        const GuestFrame frame(ts, 0x10, 0x18);
        const u32 first = frame - 0x18, second = frame - 0x14, third = frame - 0x10, fourth = frame - 0xc,
                    fifth = frame - 8;
        // The two pushed words are under esp as the code pushes them (the core takes them as arguments).
        ts.sp -= 2 * 4;
        guest::direct<0x4ed820, frontend::menz::ddrawSurfaceSize>(app, ts, window, first, second, third, fourth, fifth);
        ts.sp += 2 * 4;
        if (ts.terminate)
            return false;
        g.eax = record;
        g.edx = at(app, first);
        g.ebx = at(app, second);
        g.ecx = at(app, fourth);
        return called(app, ts, kMakeBitmap, {at(app, third), at(app, fifth), window});
    }
    if (which == 0)
        return true;
    if (!driven<thrash::kThrashWindow, &port::driver::window>(app, ts, which))
        return false;
    const std::optional<u32> lock = driven<thrash::kThrashLockWindow, &port::driver::lockWindow>(app, ts);
    if (!lock)
        return false;
    if (*lock != 0)
    {
        put(app, record + kScreenPixels, at(app, *lock + kLockPixels));
        put(app, record + kScreenPitch, at(app, *lock + kLockPitch));
        put(app, record + kScreenWidthAt, at(app, *lock + kLockWidth));
        put(app, record + kScreenHeightAt, at(app, *lock + kLockHeight));
        // The lock's format (the code's jump table at 0x5179a0): its bits per pixel.
        u8 bits = 0x10;
        switch (at(app, *lock + kLockFormat) - 2)
        {
        case 0: bits = 8; break;
        case 1: case 9: bits = 0xf; break;
        case 2: bits = 0x10; break;
        case 3: bits = 0x18; break;
        case 4: bits = 0x20; break;
        default: break;
        }
        putByte(app, record + kScreenBitsAt, bits);
        if (!driven<thrash::kThrashUnlockWindow, &port::driver::unlockWindow>(app, ts, *lock))
            return false;
    }
    if (!driven<thrash::kThrashWindow, &port::driver::window>(app, ts, 0))
        return false;
    const u32 depth = byte(app, record + kScreenBitsAt) == 0xf ? 0x10 : byte(app, record + kScreenBitsAt);
    put(app, record, kScreenMagic);
    put(app, record + kScreenCleared, 0);
    put(app, record + kScreenCleared + 4, 0);
    put(app, record + kScreenSizeAgain, at(app, record + kScreenWidthAt));
    put(app, record + kScreenSizeAgain + 4, at(app, record + kScreenHeightAt));
    putByte(app, record + kScreenWhich, u8(which));
    g.eax = byte(app, record + kScreenBitsAt);
    const std::optional<u32> code = game(app, ts, kDepthCode);
    if (!code)
        return false;
    const u32 height = at(app, record + kScreenHeightAt), pitch = at(app, record + kScreenPitch);
    putByte(app, record + kScreenDepthCode, u8(*code));
    put(app, record + kScreenWindow, window);
    const std::optional<u32> rows = rowBuffer(app, ts, pitch, 0, height);
    if (!rows)
        return false;
    put(app, record + kScreenRows, *rows);
    const std::optional<u32> moreRows = rowBuffer(app, ts, depth, 3, at(app, record + kScreenWidthAt));
    if (!moreRows)
        return false;
    put(app, record + kScreenMoreRows, *moreRows);
    return true;
}

/* 0x5179d0: screenSetUp, screen eax in the record at ebx, its window edx; ecx kept, eax as its last
 * call leaves it. */
nfs::u32 setUpScreen(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const Kept kept(g, kEcx);
    if (!screenSetUp(app, ts, g.eax, g.edx, g.ebx))
        return g.eax;
    kept.back(g);
    ts.sp += 4;
    return g.eax;
}

}  // namespace graphics

// nfs3hp_main.cpp (the port's widescreen change): a 16:9 size taken with the
// 4:3 ones, and Dynamic Fullscreen's marker answered with its mode.
bool widescreenMode(nfs::u32 width, nfs::u32 height);
nfs::u32 dynamicSavedMode(nfs::u32 found, nfs::u32 width);

namespace graphics
{

/* ---- The Screen Size list ---- */

constexpr u32 kModeMemory = 0x55fed0;        // the driver's memory for a mode (8 MB on a Voodoo)
// A mode of THRASH_about's (kModeBytes from its +0x40): +0 its width, +4 its height, +8 its bits,
// +0x10 its flags (bit 0 available, bit 1 barred), +0x14 its colour buffers, +0x18 its buffers.
constexpr u32 kModeWidthAt = 0, kModeHeightAt = 4, kModeBitsAt = 8, kModeFlags = 0x10, kModeColourBuffers = 0x14,
                kModeBuffersAt = 0x18;

/* THRASH_about's mode `index`. */
u32 modeRecord(App* app, u32 about, u32 index)
{
    return index * kModeBytes + at(app, about + kAboutModes);
}

/* Whether a mode's width and height are 4:3, or what nfs3hp::widescreenMode takes (a port). */
bool raceShape(u32 width, u32 height)
{
    return width * 3 == height << 2 || widescreenMode(width, height);
}

/* Whether a mode's flags say it is available and not barred. */
bool modeOpen(u8 flags)
{
    return (flags & 1) && !(flags & 2);
}

/* 0x4bed90's core: each of THRASH_about's modes the game can race in handed to `handler` (eax the
 * mode, edx its width, [esp+4] whether it has three buffers; ebx and ecx its height and bits, as the
 * code has them; esp as the handler leaves it): at least two colour buffers, 4:3 or a port's wide
 * one, 15 or 16 bits, available and not barred, and two buffers of it within half the driver's
 * memory.  As deep as the code calls it.  eax THRASH_about's record at the end, as the code leaves
 * it. */
bool screenModesListed(App* app, ThreadState& ts, u32 handler)
{
    const GuestFrame frame(ts, 0x18, 0x10);
    const std::optional<u32> about = driven<thrash::kThrashAbout, &port::driver::about>(app, ts);
    if (!about)
        return false;
    for (u32 mode = 1;; ++mode)
    {
        g.eax = *about;
        if (s32(mode) > s32(at(app, *about + kAboutLastMode)))
            return true;
        const u32 record = modeRecord(app, *about, mode);
        const u32 buffers = at(app, record + kModeColourBuffers), width = at(app, record + kModeWidthAt),
                    height = at(app, record + kModeHeightAt), bits = at(app, record + kModeBitsAt);
        const u8 flags = u8(at(app, record + kModeFlags));
        if (s32(buffers) < 2 || !raceShape(width, height) || (bits != 0xf && bits != 0x10) || !modeOpen(flags))
            continue;
        if (s32(at(app, kModeMemory)) / 2 < s32(width * height * 2))
            continue;
        const u32 triple = s32(at(app, modeRecord(app, *about, mode) + kModeBuffersAt)) >= 2 ? 1 : 0;
        g.eax = mode;
        g.edx = width;
        g.ebx = height;
        g.ecx = bits;
        push(app, ts, triple);
        if (!called(app, ts, handler))
            return false;
    }
}

/* 0x4bed90: screenModesListed, the handler at eax. */
nfs::u32 listScreenModes(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const Kept kept(g, kEbx | kEcx | kEdx);
    if (!screenModesListed(app, ts, g.eax))
        return g.eax;
    kept.back(g);
    ts.sp += 4;
    return g.eax;
}

/* 0x4bee80's core: the mode the Screen Size list would have for `width`: of those
 * screenModesListed takes (memory aside), the nearest in width, the first of equals;
 * nfs3hp::dynamicSavedMode for Dynamic Fullscreen's marker (a port).  The mode, 1 for none. */
std::optional<u32> screenModeFor(App* app, ThreadState& ts, u32 width)
{
    const std::optional<u32> about = driven<thrash::kThrashAbout, &port::driver::about>(app, ts);
    if (!about)
        return std::nullopt;
    u32 found = 0, nearest = 0;
    for (u32 mode = 1; s32(mode) <= s32(at(app, *about + kAboutLastMode)); ++mode)
    {
        const u32 record = modeRecord(app, *about, mode);
        const u32 modeWidth = at(app, record + kModeWidthAt), modeHeight = at(app, record + kModeHeightAt);
        if (s32(at(app, record + kModeColourBuffers)) < 2 || !raceShape(modeWidth, modeHeight))
            continue;
        const u32 bits = at(app, record + kModeBitsAt);
        if ((bits != 0xf && bits != 0x10) || !modeOpen(byte(app, modeRecord(app, *about, mode) + kModeFlags)))
            continue;
        u32 distance = modeWidth - width;
        if (s32(distance) <= 0)
            distance = 0 - distance;
        if (found != 0 && s32(distance) >= s32(nearest))
            continue;
        found = mode;
        nearest = distance;
    }
    found = dynamicSavedMode(found, width);
    return found != 0 ? found : 1;
}

/* 0x4bee80: screenModeFor width eax. */
nfs::u32 findScreenMode(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const Kept kept(g, kEbx | kEcx | kEdx);
    const std::optional<u32> mode = screenModeFor(app, ts, g.eax);
    if (!mode)
        return g.eax;
    g.eax = *mode;
    kept.back(g);
    ts.sp += 4;
    return g.eax;
}

/* ---- The menus' art in textures ---- */

constexpr u32 kBadArtFormat = 0x5495c8;
constexpr u32 kFillZero = 0x4e070c;          // eax where, edx bytes
constexpr u32 kFillByte = 0x4e0716;          // eax where, edx bytes, bl the byte
constexpr u32 kArtSide = 0x20;               // the art's pixels a side
constexpr u32 kArtShapeBytes = 0x1014;       // what the pixels are made into

/* 0x4d5f40's core: a texture `size` by `size` of the driver's format [formatAt] (3: 1555, 4: 565,
 * 6: 8888; another an Error) allocated (THRASH_talloc), after its 32 by 32 pixels -- clear, or opaque
 * white for `clear` 0 -- were made into a shape and run through sub_4d37a0 with the record at
 * `formatAt`.  The shape's kind and the clear flag live in the frame, as deep as the code has it:
 * sub_4d37a0 is handed the flag's slot, and another format leaves the kind as the stack has it.
 * The registers at each call as the code has them; eax as THRASH_talloc leaves it, which is the result. */
std::optional<u32> artTexture(App* app, ThreadState& ts, u32 size, u32 formatAt, u32 clear)
{
    const GuestFrame frame(ts, 0x10, 0x18);
    const u32 clearAt = frame - 0x18, kindAt = frame - 4;
    put(app, clearAt, clear);
    u32 bits = 0;
    switch (at(app, formatAt))
    {
    case 4: put(app, kindAt, 0x78); bits = 0x10; break;
    case 3: put(app, kindAt, 0x7e); bits = 0x10; break;
    case 6: put(app, kindAt, 0x7d); bits = 0x20; break;
    default:
        if (!error(app, ts, kBadArtFormat))
            return std::nullopt;
        break;
    }
    const u32 pixelBytes = u32(s32(bits) / 8) << 0xa;
    const u32 shapeBytes = pixelBytes + 0x14;
    const std::optional<u32> shape = allocate(app, ts, kShapeBufferTag, shapeBytes);
    if (!shape)
        return std::nullopt;
    const std::optional<u32> pixels = allocate(app, ts, kShapeBufferTag, kArtShapeBytes);
    if (!pixels)
        return std::nullopt;
    if (!viaCore<kFillZero, ea::clearBytes>(app, ts, *shape, shapeBytes))
        return std::nullopt;
    if (at(app, clearAt) == 0)
    {
        if (!viaCore<kFillByte, ea::fillBytes>(app, ts, *shape + 0x10, pixelBytes, 0xff))
            return std::nullopt;
    }
    const u32 kind = at(app, kindAt) & 0xff;
    putByte(app, *shape, 0);
    putHalf(app, *shape + 4, kArtSide);
    put(app, *shape, at(app, *shape) | kind);
    putHalf(app, *shape + 6, kArtSide);
    g.ecx = kArtSide;  // as the code has it at sub_4d37a0, which takes all four
    if (!shapeToPixels(app, ts, *pixels, *shape, clearAt, formatAt) || !release(app, ts, *shape)
        || !release(app, ts, *pixels))
        return std::nullopt;
    return driven<thrash::kThrashTAlloc, &port::driver::tAlloc>(app, ts, size, size, at(app, formatAt), 0, 0);
}

/* 0x4d5f40: artTexture, eax by eax, the format at edx, clear for ebx nonzero. */
nfs::u32 makeArtTexture(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const Kept kept(g, kEcx);
    const std::optional<u32> texture = artTexture(app, ts, g.eax, g.edx, g.ebx);
    if (!texture)
        return g.eax;
    g.eax = *texture;
    kept.back(g);
    ts.sp += 4;
    return g.eax;
}

/* ---- The screen saved to a file (Alt+P) ---- */

constexpr u32 kCaptureTrace = 0x5401c8;
constexpr u32 kCaptureFolder = 0x4b6520;     // the folder pictures go in
constexpr u32 kCaptureName = 0x5401d8;       // "%s...%d...": the folder, the number
constexpr u32 kCaptureNumber = 0x5600f8;
constexpr u32 kRedrawPaused = 0x4bc680;
constexpr u32 kNewBitmap = 0x4ef190, kSaveBitmap = 0x4eb160;
constexpr u32 kModeWidthPixels = 0x7cdae0, kModeHeightPixels = 0x7cdae4, kModeBits = 0x7cdae8;

/* A string copied byte by byte, its 0 too, as the game's inlined strcpy copies it. */
void copyString(App* app, u32 to, u32 from)
{
    for (u32 i = 0;; ++i)
    {
        const u8 c = byte(app, from + i);
        putByte(app, to + i, c);
        if (c == 0)
            return;
    }
}

/* A string's length, as the game's inlined strlen (repne scasb) counts it. */
u32 stringLengthAt(App* app, u32 from)
{
    u32 length = 0;
    while (byte(app, from + length) != 0)
        ++length;
    return length;
}

constexpr u32 kBitmapPrepare = 0x4eada0;     // eax the new bitmap
constexpr u32 kBitmapSelect = 0x4ef1b0;      // eax 0

/* 0x4c4490's core: the screen saved as a picture -- but in a network race -- under the first
 * number not yet taken in the capture folder: a paused race drawn again first, the front buffer
 * read (THRASH_readrect) into a bitmap of the mode's size, and the bitmap saved and freed.  The
 * folder and the name in the frame, as deep as the code has them; the registers each call takes as
 * the code has them -- 0x4b6520's four: eax as the trace leaves it, edx the play mode, ebx and ecx
 * the caller's --; eax as the last call leaves it, 0 (the inlined strlen's al) for no folder. */
bool screenCaptured(App* app, ThreadState& ts)
{
    const GuestFrame frame(ts, 0x18, 0x300);
    const u32 folder = frame - 0x300, name = frame - 0x100;
    if (!traced(app, ts, kCaptureTrace))
        return false;
    const u32 mode = gameSettings(app).playMode;
    if (s32(mode) >= 2)
        return true;
    g.edx = mode;
    const std::optional<u32> captures = game(app, ts, kCaptureFolder);
    if (!captures)
        return false;
    copyString(app, folder, *captures);
    if (stringLengthAt(app, folder) == 0)
    {
        g.eax = 0;
        return true;
    }
    for (;;)
    {
        const u32 number = at(app, kCaptureNumber) + 1;
        put(app, kCaptureNumber, number);
        guest::direct<kSprintf, watcom::clib::sprintf>(app, ts, name, kCaptureName, guest::Words{folder, number});
        if (ts.terminate)
            return false;
        const std::optional<bool> exists = guest::direct<0x4e0e60, ea::fileThere>(app, ts, name);
        if (!exists)
            return false;
        if (!*exists)
            break;
    }
    const u32 replay = gameSettings(app).replayMode;
    if (replay == 2)
    {
        // eax the file's answer, 0, which the replay's mode takes too.
        gameSettings(app).replayMode = 0;
        if (!viaCore<kRedrawPaused, frontend::loopb::pauseDraw>(app, ts, 0) || !flushFlip(app, ts))
            return false;
        gameSettings(app).replayMode = replay;
    }
    const std::optional<u32> bitmap = viaValue<kNewBitmap, frontend::eab::windowBitmapCreateDefault>(app, ts, at(app, kModeWidthPixels), at(app, kModeHeightPixels), at(app, kModeBits), 0x10);
    if (!bitmap)
        return false;
    if (*bitmap == 0)
        return true;
    const std::optional<u32> prepared = viaValue<kBitmapPrepare, frontend::eaa::bitmapDrawInto>(app, ts, *bitmap);
    if (!prepared
        || !viaCore<kBitmapSelect, frontend::eab::drawClearClip>(app, ts, *prepared)
        || !driven<thrash::kThrashWindow, &port::driver::window>(app, ts, 1))
        return false;
    const u32 pixels = at(app, *bitmap + 0x20) + 0x10;
    if (!driven<thrash::kThrashReadRect, &port::driver::readRect>(app, ts, 0, 0, at(app, kModeWidthPixels), at(app, kModeHeightPixels), pixels))
        return false;
    if (!viaCore<kSaveBitmap, frontend::eaa::bitmapSaveTga>(app, ts, name))
        return false;
    return letGo(app, ts, *bitmap);
}

/* 0x4c4490: screenCaptured. */
nfs::u32 captureScreen(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const Kept kept(g, kEbx | kEcx | kEdx);
    if (!screenCaptured(app, ts))
        return g.eax;
    kept.back(g);
    ts.sp += 4;
    return g.eax;
}

/* ---- A message across the screen ---- */

constexpr u32 kViewRect = 0x4beb80;          // eax, edx, ebx, ecx: where the view's x, y, width, height go
constexpr u32 kMessageLine = 0x115;
constexpr u32 kMessageEnding = 0x540208;
constexpr u32 kMessageScale = 0x54020c, kMessageScaleAgain = 0x540210;
constexpr u32 kFont87 = 0x57;
constexpr u32 kFontLineHeight = 0x4d14f0;
constexpr u32 kDrawBox = 0x47d1d0;           // eight floats and two words: a box with its border
constexpr u32 kBoxColour = 0x7f000000, kBorderColour = 0xff488cfd;

/* port_text's textWidthWithOverhang (0x4d1480), by address: text eax (edx as the code has it); the
 * width it leaves on the x87's stack, taken off.  nullopt when the game stopped. */
std::optional<nfs::Float> textWidth(App* app, ThreadState& ts, u32 text)
{
    g.eax = text;
    const port::text::WidthResult width =
        guest::direct<kTextWidth, port::text::textMeasuredWithOverhang>(app, ts, ts.sp - 4, text);
    if (!width || ts.terminate)
        return std::nullopt;
    g.eax = width->eax;
    return nfs::Float(width->value);
}

/* sub_47d1d0, a box with its border, by address: its four corners (x, y as floats' bits), whether
 * filled, its colour; stdcall.  eax, edx, ebx and ecx as the code's pushes leave them. */
bool drawBox(App* app, ThreadState& ts, u32 left, u32 top, u32 right, u32 bottom, u32 filled, u32 colour)
{
    g.eax = top;
    g.edx = left;
    g.ebx = right;
    g.ecx = top;
    return called(app, ts, kDrawBox, {left, top, right, top, right, bottom, left, bottom, filled, colour});
}

/* 0x4c5c00's core: line 0x115 of the game's text (with its ending) in a box in the middle of the
 * view, but while paused: drawn into both buffers, a flip after each; then the race's frame ended
 * (eax as that leaves it).  The view's rectangle and the text in the frame, as deep as the code has
 * them; the x87's operations as the code does them; the registers at each call as the code has
 * them. */
bool waitMessage(App* app, ThreadState& ts)
{
    const GuestFrame frame(ts, 0x18, 0x64);
    const u32 text = frame - 0x64, viewWidthAt = frame - 8, viewHeightAt = frame - 0x18;
    const std::optional<u32> viewHeight = viaValue<kViewRect, port::text::viewportOf>(app, ts, frame - 0xc, frame - 0x14, viewWidthAt, viewHeightAt);
    if (!viewHeight)
        return false;
    g.eax = *viewHeight;
    if (s32(gameSettings(app).replayMode) > 1)
        return true;
    const Kept synced(g, kEcx | kEdx);  // as syncWindow keeps them
    if (!driven<thrash::kThrashSync, &port::driver::sync>(app, ts, 0) || !driven<thrash::kThrashWindow, &port::driver::window>(app, ts, 2))
        return false;
    synced.back(g);
    const std::optional<u32> line = languageText(app, ts, kMessageLine);
    if (!line)
        return false;
    copyString(app, text, *line);
    copyString(app, text + stringLengthAt(app, text), kMessageEnding);
    // The text at the view's height over 480.
    const nfs::Float scale =
        fp::mul(fp::mul(nfs::Float(s32(at(app, viewWidthAt))), nfs::Float(floatAt(app, kMessageScale))),
                nfs::Float(floatAt(app, kMessageScaleAgain)));
    if (!setTextScale(app, ts, bitsOf(float(scale))))
        return false;
    const std::optional<nfs::Float> width = textWidth(app, ts, text);
    if (!width)
        return false;
    const std::optional<s32> textWide = chop(app, ts, *width);
    if (!textWide)
        return false;
    g.eax = kFont87;
    const std::optional<u32> fontHeight = viaValue<kTextHeight, port::text::characterHeight>(app, ts, kFont87);
    if (!fontHeight)
        return false;
    g.ecx = *fontHeight;
    g.eax = kFont87;
    const std::optional<u32> lineHeight = viaValue<kFontLineHeight, frontend::hudx::characterDrop>(app, ts, kFont87);
    if (!lineHeight)
        return false;
    // Integers as the code works them out: wrapping, halved and quartered towards 0.
    const auto halved = [](u32 value) { return u32(s32(value) / 2); };
    const auto asFloat = [](u32 value) { return bitsOf(float(nfs::Float(s32(value)))); };
    const u32 high = *fontHeight + *lineHeight;
    // The text's place: the view's middle less half the text.
    const u32 middle = halved(at(app, viewWidthAt)), middleY = halved(at(app, viewHeightAt));
    const u32 textLeft = middle - halved(u32(*textWide)), textTop = middleY - halved(high);
    // The box: three quarters of the text's width either side, a line up and down.
    const u32 reach = u32(s32(u32(*textWide) * 3) / 4);
    const u32 left = asFloat(middle - reach), right = asFloat(reach + middle);
    const u32 top = asFloat(middleY - high), bottom = asFloat(middleY + high);
    if (!driven<thrash::kThrashWindow, &port::driver::window>(app, ts, 2))
        return false;
    for (int pass = 0; pass < 2; ++pass)
        if (!drawBox(app, ts, left, top, right, bottom, 1, kBoxColour)
            || !drawBox(app, ts, left, top, right, bottom, 0, kBorderColour)
            || !drawText(app, ts, text, textLeft, textTop) || !viaPort<kFlushText, port::text::fontsDrawn>(app, ts)
            || !flushFlip(app, ts))
            return false;
    return guest::direct<0x434a90, collide::fogColour>(app, ts);
}

/* 0x4c5c00: waitMessage. */
nfs::u32 showWaitMessage(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const Kept kept(g, kEbx | kEcx | kEdx);
    if (!waitMessage(app, ts))
        return g.eax;
    kept.back(g);
    ts.sp += 4;
    return g.eax;
}

/* ---- The HUD editor's frame ---- */

constexpr u32 kFrameMesh = 0x559a20;         // twelve corners of kCornerBytes: x, y, ..., the colour at +0x10
constexpr u32 kFrameScaleX = 0x539b94, kFrameScaleY = 0x539b9c, kFrameThickness = 0x539b98;
constexpr u32 kFrameMarginX = 0x539ba0, kFrameMarginY = 0x539ba4;
constexpr u32 kFrameAll = 0x559ba0, kFrameClippedBoth = 0x559c40;  // the lines' index lists
constexpr u32 kFrameClippedLeft = 0x559be0, kFrameClippedRight = 0x559c10;

/* fcom and fnstsw's C0, as the code tests it: a below b, or either NaN. */
bool belowOrUnordered(nfs::Float a, nfs::Float b)
{
    return !(a >= b);
}

// A HUD element: +6 its left and +8 its top (words), +0x60 its right and +0x64 its bottom.
constexpr u32 kElementLeft = 6, kElementTop = 8, kElementRight = 0x60, kElementBottom = 0x64;

/* 0x459440's core: the HUD editor's frame about `element` (made room about by the mode's width),
 * drawn as a mesh of lines in `colour` -- the side that is off the screen left out.  The x87's
 * operations as the code does them, each float stored where it stores one; its compares (fcom,
 * fnstsw) as float compares that give what C0 and C3 give, NaN too.  eax and edx at the driver's
 * call as the code has them: whether the right side is off, whether the left is. */
bool editorFrame(App* app, ThreadState& ts, u32 element, u32 colour)
{
    const auto loadFloat = [&](u32 address) { return nfs::Float(floatAt(app, address)); };
    const auto word16 = [&](u32 address) { return nfs::Float(nfs::s16(half(app, address))); };
    const auto integer = [&](u32 address) { return nfs::Float(s32(at(app, address))); };
    const auto corner = [](u32 n) { return kFrameMesh + n * kCornerBytes; };
    const nfs::Float width = integer(kModeWidth);
    const nfs::Float scaledX = fp::mul(width, loadFloat(kFrameScaleX));
    const nfs::Float scaledY = fp::mul(width, loadFloat(kFrameScaleY));
    const nfs::Float thickness = loadFloat(kFrameThickness);
    const float roomX = float(fp::mul(scaledX, thickness));
    const float roomY = float(fp::mul(thickness, scaledY));
    const nfs::Float right = integer(element + kElementRight);
    const nfs::Float left = word16(element + kElementLeft);
    const float leftEdge = float(fp::add(left, loadFloat(kFrameMarginX)));
    const float rightEdge = float(fp::add(right, loadFloat(kFrameMarginY)));
    for (u32 n = 0; n < 0xc; ++n)
        put(app, corner(n) + kCornerColour, colour);
    // The left edge kept on the screen.
    const u32 leftX = belowOrUnordered(0.0, nfs::Float(leftEdge)) ? bitsOf(leftEdge) : 0;
    const nfs::Float frameLeft = word16(element + kElementLeft);
    const nfs::Float frameRight = integer(element + kElementRight);
    for (const u32 n : {11u, 10u, 1u, 0u})
        put(app, corner(n), leftX);
    putFloat(app, corner(9), float(fp::add(frameLeft, nfs::Float(roomX))));
    putFloat(app, corner(8), float(fp::sub(frameRight, nfs::Float(roomX))));
    const nfs::Float innerLeft = loadFloat(corner(9)), innerRight = loadFloat(corner(8));
    putFloat(app, corner(2), float(innerLeft));
    putFloat(app, corner(3), float(innerRight));
    // The right edge kept on the screen.
    const float lastX = float(nfs::Float(s32(at(app, kModeWidth) - 1)));
    const u32 rightX =
        belowOrUnordered(nfs::Float(rightEdge), nfs::Float(lastX)) ? bitsOf(rightEdge) : bitsOf(lastX);
    const float top = float(word16(element + kElementTop));
    const float bottom = float(integer(element + kElementBottom));
    for (const u32 n : {7u, 6u, 5u, 4u})
        put(app, corner(n), rightX);
    putFloat(app, corner(5) + 4, float(fp::add(nfs::Float(top), nfs::Float(roomY))));
    putFloat(app, corner(11) + 4, float(fp::sub(nfs::Float(bottom), nfs::Float(roomY))));
    const nfs::Float innerTop = loadFloat(corner(5) + 4), innerBottom = loadFloat(corner(11) + 4);
    for (const u32 n : {4u, 3u, 2u, 1u})
        put(app, corner(n) + 4, bitsOf(top));
    putFloat(app, corner(0) + 4, float(innerTop));
    for (const u32 n : {10u, 9u, 8u, 7u})
        put(app, corner(n) + 4, bitsOf(bottom));
    putFloat(app, corner(6) + 4, float(innerBottom));
    if (!stateSent(app, ts, thrash::kStateTexture, 0))
        return false;
    // Which sides are off the screen: the lines of all, or of those on it.
    const bool leftOff = nfs::Float(0.0) > nfs::Float(leftEdge);  // C0 and C3 clear: above, not NaN
    const bool rightOff =
        belowOrUnordered(nfs::Float(s32(at(app, kModeWidth) - 1)), nfs::Float(rightEdge));
    u32 lines = kFrameAll, count = 8;
    if (leftOff && rightOff)
        lines = kFrameClippedBoth, count = 4;
    else if (leftOff || rightOff)
        lines = leftOff ? kFrameClippedLeft : kFrameClippedRight, count = 6;
    g.eax = rightOff ? 1 : 0;
    g.edx = leftOff ? 1 : 0;
    return driven<thrash::kThrashDrawLineMesh, &port::driver::drawLineMesh>(app, ts, count, kFrameMesh, lines).has_value();
}

/* 0x459440: editorFrame about element eax, colour edx. */
nfs::u32 hudEditorFrame(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const Kept kept(g, kEbx | kEcx);
    if (!editorFrame(app, ts, g.eax, g.edx))
        return g.eax;
    kept.back(g);
    ts.sp += 4;
    return g.eax;
}

}  // namespace graphics


namespace graphics
{

/* ---- Loading a race ---- */

constexpr u32 kMenuControl = 0x79f3f4, kRaceControl = 0x79f3ec;  // the x87's words, the menus' and a race's
constexpr u32 kLoadingOff = 0x55e88c;

namespace
{

/* fninit and fldcw [0x79f3f4]: the menus' precision.  The arithmetic is the game's single precision at all times
 * (fp::), so there is nothing to set. */
void menuPrecision(nfs::App* app, nfs::ThreadState& ts)
{
    static_cast<void>(app);
    static_cast<void>(ts);
}

/* fninit, fnstcw [0x79f3ec], the word kept as the menus' and its precision
 * cleared, fldcw: the race's.  eax as the code leaves it. */
void racePrecision(nfs::App* app, nfs::ThreadState& ts)
{
    static_cast<void>(ts);
    constexpr u16 kInitialControl = 0x7f;  // the word fninit leaves (nfs::FloatEnv::init): single precision
    putHalf(app, kRaceControl, kInitialControl);
    g.eax = at(app, kRaceControl);
    put(app, kMenuControl, g.eax);
    g.eax &= 0xfffff0ff;
    put(app, kRaceControl, g.eax);
}

}  // namespace

constexpr u32 kLoadingPictures = 0x57126b, kPictureChoices = 0x552938;
constexpr u32 kLoadingPath = 0x7a3134;

constexpr u32 kAppendBackslash = 0x4529a0;   // port::menus' path ended with a backslash: eax the path
constexpr u32 kLoadQfs = 0x4956f0;           // frontend::loadQfs: eax the name, edx 0x10
constexpr u32 kBitmapShape = 0x4ef1e0;       // eax a .qfs, edx a shape's name
constexpr u32 kTrackPictureName = 0x53be00, kCarPictureName = 0x53be10, kLoadingShapeName = 0x53be20;
constexpr u32 kTrackNameBytes = 0x10;
// The race's entries: a player's 0x6c bytes (its car among them).
constexpr u32 kEntryBytes = 0x6c;

/* port::menus' backslash after a path (0x4529a0), by address: the path in eax. */
bool appendBackslash(App* app, ThreadState& ts, u32 path)
{
    g.eax = path;
    return called(app, ts, kAppendBackslash);
}

/* sprintf onto the end of the string at `text`, as the code finds it (its inlined strlen). */
bool appendFormatted(App* app, ThreadState& ts, u32 text, u32 format, u32 first, u32 second)
{
    guest::direct<kSprintf, watcom::clib::sprintf>(app, ts, text + stringLengthAt(app, text), format,
                                                    guest::Words{first, second});
    return !ts.terminate;
}

/* Port: the loading bar paced as the Modern Patch paces it.
 *
 * Drawn at most every kBarPeriod.  The game moves it after every texture of the track -- 157 to 278
 * of them -- and every move drew it into both buffers, two flips, each held to its frame by the
 * frame cap: up to 18 s of a load spent waiting on the bar.  A move sooner than the period after
 * the last one drawn is not drawn (the system's tasks still run, and the stop check is the
 * callers'); a move to 100 per cent always is.
 *
 * And it takes kBarSplash from the loading picture to its end, whatever the loading takes: the bar
 * shows no more than the share of that time gone, and once the time is up it goes on by at most
 * kBarStepMost a move drawn.  The race's load, all of it done, waits at 100 per cent until the bar
 * is there (loadingBarReached, frontend::loopa::raceLoad).  Any key but Escape (and Android's
 * Back, its Escape) or a click cuts the time short -- a mouse's button, or a tap, which on the
 * loading screen is the pointer's as in the menus (pointerTick) -- and the bar then runs to its
 * end at kBarStepMost a move.  Escape stops the load as ever (0x4a33d0).
 *
 * The loading screen's start (loadingScreenShown) starts it all over, its first move drawn. */
namespace
{
using BarClock = std::chrono::steady_clock;
constexpr BarClock::duration kBarPeriod = std::chrono::milliseconds(50);
constexpr BarClock::duration kBarSplash = std::chrono::seconds(5);
constexpr float kBarStepMost = 5.0f;   // per cent a move drawn, once the time is up
BarClock::time_point s_barDrawn, s_barStart;
bool s_barDrawnYet = false;
bool s_barTimed = false;               // the time still holds the bar back
float s_barShown = 0.0f;               // the per cent last drawn
bool s_barReached = true;              // the last move asked for was drawn as it was asked
std::atomic<bool> s_barHurried{false};
bool s_barWatching = false;
u32 s_barClicks = 0;                   // nfs::Mouse::presses() as the loading screen came up

/* SDL's event watch, on whatever thread an event comes in: a key that hurries the bar. */
bool SDLCALL barHurryWatch(void* /*userdata*/, SDL_Event* event)
{
    switch (event->type)
    {
    case SDL_EVENT_KEY_DOWN:
        if (!event->key.repeat && event->key.scancode != SDL_SCANCODE_ESCAPE
            && event->key.scancode != SDL_SCANCODE_AC_BACK)
            s_barHurried.store(true, std::memory_order_relaxed);
        break;
    default:
        break;
    }
    return true;
}

/* The bar started over for a new loading screen. */
void barStarted()
{
    if (!s_barWatching)
    {
        SDL_AddEventWatch(barHurryWatch, nullptr);
        s_barWatching = true;
    }
    s_barHurried.store(false, std::memory_order_relaxed);
    s_barClicks = nfs::Mouse::presses();
    s_barStart = BarClock::now();
    s_barDrawnYet = false;
    s_barTimed = true;
    s_barShown = 0.0f;
    s_barReached = true;
}

/* The per cent to draw for a move to `asked`, or nullopt when this move is not drawn. */
std::optional<float> barPaced(float asked)
{
    const BarClock::time_point now = BarClock::now();
    if (s_barHurried.load(std::memory_order_relaxed) || nfs::Mouse::presses() != s_barClicks)
        s_barTimed = false;
    if (asked < 100.0f && s_barDrawnYet && now - s_barDrawn < kBarPeriod)
        return std::nullopt;
    float shown = asked;
    const BarClock::duration gone = now - s_barStart;
    bool stepped = true;
    if (s_barTimed && gone < kBarSplash)
    {
        const float allowed = 100.0f * std::chrono::duration<float>(gone).count()
                              / std::chrono::duration<float>(kBarSplash).count();
        if (allowed <= asked)
        {
            shown = allowed;
            stepped = false;
        }
    }
    else
        s_barTimed = false;
    if (stepped && shown > s_barShown + kBarStepMost)
        shown = s_barShown + kBarStepMost;
    s_barDrawn = now;
    s_barDrawnYet = true;
    s_barShown = shown;
    return shown;
}
}  // namespace

/* Port: whether the last move of the loading bar was drawn where it was asked to go (kBarSplash). */
bool loadingBarReached()
{
    return s_barReached;
}

/* 0x494fa0's core: the loading screen put up as a race loads (but with it turned off): the race's
 * track's picture -- one of its several in turn, or the car's when there is none or [0x6fd4fc] asks
 * for it -- loaded and shown in both buffers; the textures let go of, and the race's x87 precision
 * set (eax as that leaves it).  The track's name and the picture's in the frame, as deep as the
 * code has them; the registers each call takes as the code has them (sub_4ef1e0 all four, ebx the
 * track's number as the code keeps it). */
bool loadingScreenShown(App* app, ThreadState& ts)
{
    if (at(app, kLoadingOff) != 0)
        return true;
    barStarted();  // port: the bar paced over again (kBarSplash)
    const GuestFrame frame(ts, 0x18, 0x114);
    const u32 name = frame - 0x114, track = frame - 0x14;
    menuPrecision(app, ts);
    const u32 car = gameSettings(app).entryAt(gameSettings(app).localPlayer * kEntryBytes).car;
    const std::optional<u32> trackName = guest::direct<0x439520, load::idOf>(app, ts, car);
    if (!trackName)
        return false;
    // Its track's name: rep movsd, rep movsb.
    for (u32 i = 0; i < kTrackNameBytes; i += 4)
        put(app, track + i, at(app, *trackName + i));
    const u32 id = guest::direct<0x439660, load::numberOf>(app, ts, track);
    if (ts.terminate)
        return false;
    // 0x43bcb0 (loopa's loadingScreenFlag): always 1, what the loading screen asks; it reads no register.
    const std::optional<u32> pictures = viaValue<0x43bcb0, frontend::loopa::loadingScreenFlag>(app, ts);
    if (!pictures)
        return false;
    copyString(app, name, kLoadingPath);
    if (!appendBackslash(app, ts, name))
        return false;
    // Which of the track's pictures: the next in turn, by their count.
    const u8 turn = byte(app, kLoadingPictures);
    const s32 choice = byte(app, kPictureChoices + turn);
    const u32 which = u32(s32(choice % s32(*pictures)));
    putByte(app, kLoadingPictures, u8(turn + 1));
    if (!appendFormatted(app, ts, name, kTrackPictureName, id, which))
        return false;
    const std::optional<bool> exists = guest::direct<0x4e0e60, ea::fileThere>(app, ts, name);
    if (!exists)
        return false;
    if (!*exists || (std::uint8_t(gameSettings(app).raceFlags) & 2))
    {
        // The car's picture instead.
        copyString(app, name, kLoadingPath);
        if (!appendBackslash(app, ts, name))
            return false;
        if (!appendFormatted(app, ts, name, kCarPictureName, gameSettings(app).track, 0))
            return false;
    }
    const u32 qfs = guest::direct<kLoadQfs, frontend::draw::loadQfsFlags>(app, ts, name, 0x10u);
    if (ts.terminate)
        return false;
    g.eax = qfs;
    g.edx = kLoadingShapeName;
    g.ebx = id;
    g.ecx = qfs;
    const std::optional<u32> shape = viaValue<kBitmapShape, frontend::eab::shapeFileFind>(app, ts, qfs, kLoadingShapeName);
    if (!shape)
        return false;
    for (int buffer = 0; buffer < 2; ++buffer)
    {
        if (!windowTakenAndCleared(app, ts))
            return false;
        if (!guest::direct<0x494cb0, render::loadingPictureShown>(app, ts, *shape) || !flipAndWait(app, ts))
            return false;
    }
    if (qfs != 0 && !release(app, ts, qfs))
        return false;
    if (!driven<thrash::kThrashTReset, &port::driver::tReset>(app, ts))
        return false;
    racePrecision(app, ts);
    return true;
}

/* 0x494fa0's core with its answer: loadingScreenShown, eax as it leaves it -- the `eax` it came with when
 * the loading screen is off (it then touches nothing), else what the race's precision leaves. */
std::optional<u32> loadingScreenAnswer(App* app, ThreadState& ts, u32 eax)
{
    const bool off = at(app, kLoadingOff) != 0;
    if (!loadingScreenShown(app, ts))
        return std::nullopt;
    return off ? eax : g.eax;
}

/* 0x494fa0: loadingScreenShown. */
nfs::u32 showLoadingScreen(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const Kept kept(g, kEbx | kEcx | kEdx);
    if (!loadingScreenShown(app, ts))
        return g.eax;
    kept.back(g);
    ts.sp += 4;
    return g.eax;
}

constexpr u32 kLoadingBar = 0x55e4c4;        // four corners of kCornerBytes
constexpr u32 kLoadingAll = 0x53be30;        // 100, a double: the most of [esp+4]
constexpr u32 kLoadingPerCent = 0x53be38;    // a double
constexpr u32 kLoadingBarEnd = 0x53be40, kLoadingBarHeight = 0x53be44;
constexpr u32 kRunSystemTasks = 0x4e7630;    // native_snd_stream's runSystemTasks
constexpr u32 kBarColours = 0x495190;        // eax the bar's corners

constexpr u32 kBarTop = 0x43e08000, kBarBottom = 0x43ea8000;  // 449.0f and 469.0f, on a 480 line screen
constexpr u32 kBarFull = 0x42ca0000;          // 101.0f

/* native_render's polygon (0x4c12e0) for the loading bar, by address: its corners in eax, edx,
 * ebx, ecx. */
bool drawBar(App* app, ThreadState& ts)
{
    return drawQuad(app, ts, kLoadingBar, kLoadingBar + kCornerBytes, kLoadingBar + 2 * kCornerBytes,
                    kLoadingBar + 3 * kCornerBytes);
}

/* 0x495230's core: the loading bar at `perCentAt` per cent (101 past 100, written back there, then
 * the bar's share) drawn after the loading text, in both buffers, while the loading text is up; the
 * system's tasks run first.  The menus' x87 precision while it draws, the race's after (eax as
 * that leaves it).  The x87's operations as the code does them, each float stored where it stores
 * one; its compare (fcom, fnstsw) a float compare that gives what C0 gives, NaN too.
 * Port: paced, not every move drawn, and below 100 per cent into one buffer only (kBarSplash). */
bool loadingProgress(App* app, ThreadState& ts, u32 perCentAt)
{
    if (!viaPort<kRunSystemTasks, port::stream::runDueTasks>(app, ts, 0))
        return false;
    if (at(app, kLoadingTextShown) == 0)
    {
        s_barReached = true;  // port: no bar to wait for (kBarSplash)
        return true;
    }
    // port: the bar paced (kBarSplash): drawn where barPaced says, or not at all.
    const float asked = floatAt(app, perCentAt);
    const std::optional<float> paced = barPaced(asked);
    if (!paced)
    {
        s_barReached = false;
        return true;
    }
    s_barReached = *paced == asked;
    putFloat(app, perCentAt, *paced);
    const bool full =
        !belowOrUnordered(nfs::Float(floatAt(app, perCentAt)), nfs::Float(app->getMemory<double>(kLoadingAll)));
    if (!driven<thrash::kThrashSync, &port::driver::sync>(app, ts, 0) || !driven<thrash::kThrashWindow, &port::driver::window>(app, ts, 2))
        return false;
    menuPrecision(app, ts);
    if (!belowOrUnordered(nfs::Float(floatAt(app, perCentAt)), nfs::Float(app->getMemory<double>(kLoadingAll))))
        put(app, perCentAt, kBarFull);
    const nfs::Float scaled =
        fp::mul(nfs::Float(floatAt(app, perCentAt)), nfs::Float(app->getMemory<double>(kLoadingPerCent)));
    const nfs::Float left = nfs::Float(floatAt(app, kLoadingTextX));
    const nfs::Float room = fp::sub(nfs::Float(floatAt(app, kLoadingBarEnd)), left);
    putFloat(app, perCentAt, float(scaled));
    const nfs::Float width = fp::mul(room, nfs::Float(floatAt(app, perCentAt)));
    const nfs::Float height =
        fp::mul(nfs::Float(floatAt(app, perCentAt)), nfs::Float(floatAt(app, kLoadingBarHeight)));
    const auto corner = [](u32 n) { return kLoadingBar + n * kCornerBytes; };
    putFloat(app, corner(0), float(left));
    putFloat(app, corner(3), float(left));
    put(app, corner(0) + 4, kBarTop);
    put(app, corner(1) + 4, kBarTop);
    put(app, corner(2) + 4, kBarBottom);
    put(app, corner(3) + 4, kBarBottom);
    const std::optional<s32> shade = chop(app, ts, height);
    if (!shade)
        return false;
    putFloat(app, corner(1), float(fp::add(left, width)));
    const u32 colour = (u32(*shade) << 0x10) - 0x1000000;
    const nfs::Float right = nfs::Float(floatAt(app, corner(1)));
    put(app, corner(1) + kCornerColour, colour);
    put(app, corner(2) + kCornerColour, colour);
    putFloat(app, corner(2), float(right));
    // sub_495190 with the bar's corners in eax, edx, ebx and ecx, as the code has them.
    if (!viaCore<kBarColours, frontend::loopa::barScale>(app, ts, corner(0), corner(1), corner(2), corner(3)))
        return false;
    for (u32 n = 0; n < 4; ++n)
        putClipCode(app, corner(n));
    if (!stateSent(app, ts, thrash::kStateTexture, 0))
        return false;
    // port: both buffers only at 100 per cent; below it the other catches up at the next move drawn.
    const int buffers = full ? 2 : 1;
    for (int buffer = 0; buffer < buffers; ++buffer)
        if ((buffer == 1 && !driven<thrash::kThrashSync, &port::driver::sync>(app, ts, 0)) || !drawBar(app, ts)
            || !flushFlip(app, ts))
            return false;
    racePrecision(app, ts);
    return true;
}

/* 0x495230: loadingProgress at [esp+4] per cent; stdcall's 4 bytes. */
nfs::u32 drawLoadingProgress(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const Kept kept(g, kEbx | kEcx | kEdx);
    if (!loadingProgress(app, ts, ts.sp + 4))
        return g.eax;
    kept.back(g);
    ts.sp += 4 + 4;
    return g.eax;
}

/* ---- The game's main ---- */

constexpr u32 kTryInit = 0x4f7af8, kAtExitList = 0x4f7c20, kControl87 = 0x4f7c90;
constexpr u32 kKbhit = 0x4f7ce0, kGetch = 0x4f7f24, kFflush = 0x4f7d70, kMemset = 0x4e0640;
constexpr u32 kStartFlags = 0x55eb2e;        // bit 0x20 and 0x10: two ways of starting straight into a race
constexpr u32 kDebugOutput = 0x5643fc, kErrorWhere = 0x564408;  // native_game's

namespace
{

/* The menus' settings for a start straight into a race (kStartFlags' 0x20, `second` its 0x10), as
 * the code writes them: the quick race's 0x12 bytes at 0x556382 cleared first (memset, by address:
 * eax, edx, ebx), and the two ways' words alike but for 0x5562b6's and 0x55621e's.  False when the
 * game stopped. */
bool quickRaceSet(App* app, ThreadState& ts, bool second)
{
    if (!viaCore<kMemset, watcom::clib::memset>(app, ts, 0x556382, 0, 0x12))
        return false;
    const u8 letters[] = {0x32, 0x33, 0x34, 0x38, 0x3b, 0x3f, 0x40, 0x38};
    for (u32 i = 0; i < 8; ++i)
        putByte(app, 0x55b041 + i, letters[i]);
    for (const u32 at16 : {0x5562b8u, 0x556220u, 0x556222u, 0x556264u, 0x55637cu, 0x55620au, 0x556286u, 0x556288u})
        putHalf(app, at16, 0);
    for (const u32 at16 : {0x556388u, 0x5562b4u, 0x55628au, 0x556224u, 0x556208u, 0x556262u})
        putHalf(app, at16, 1);
    putHalf(app, 0x5562b6, second ? 0 : 1);
    putHalf(app, 0x55621e, second ? 1 : 0);
    put(app, 0x554f50, 0);
    g.eax = 0;  // as the code leaves it, for a callee after that leaves eax as it was
    return true;
}

}  // namespace

/* 0x4a39f0's core: the game's main, `argc` and `argv` the command line's: its exception frame
 * (Watcom's __TryInit; the handlers at 0x4a3afd and 0x4a3f7d are not reached on the port), the
 * checks before it starts (a box if another copy runs), the settings' start-up ways (bits of
 * [0x55eb2e], [0x55eb2f]: the x87's exceptions unmasked), the menus and races in turn until the
 * game quits -- each menu at the menus' x87 precision, each race at its own --, the screen cleared
 * and the sounds stopped, and a key waited for if one is pending.  The frame's old link, which it
 * puts back, or nullopt when the game stopped.
 *
 * The frame on the guest's stack as deep as the code has it, under its five pushes: the
 * registration (ebp - 0x54, its level at - 0x44) TryInit links, with ebp the frame's while it runs
 * -- TryInit takes the frame from there --, sscanf's number at - 0x10 and the box's text copied to
 * - 0x38.  Its other locals are C++'s.  ebx, ecx and edx are kept as the code has them where they
 * reach 0x4f2a7d, which takes all four registers: what the code last set, and what Windows' and the
 * driver's calls leave in ecx and edx. */
std::optional<u32> gameRun(App* app, ThreadState& ts, u32 argc, u32 argv)
{
    const GuestFrame frame(ts, 0x14, 0x54);
    const u32 registration = frame - 0x54, level = frame - 0x44, text = frame - 0x38, number = frame - 0x10;
    const u32 callers = g.ebp;
    g.ebp = frame - 0;
    // The exception frame's link put back, and out.
    auto out = [&]() -> std::optional<u32> {
        const u32 link = at(app, registration);
        put(app, ts.threadBlock, link);
        g.ebp = callers;
        return link;
    };
    auto call = [&](u32 target) { return game(app, ts, target); };
    auto withEax = [&](u32 target, u32 eax) {
        g.eax = eax;
        return game(app, ts, target);
    };
    auto withEaxEdx = [&](u32 target, u32 eax, u32 edx) {
        g.eax = eax;
        g.edx = edx;
        return game(app, ts, target);
    };
    u32 ebx = 0, ecx = 0, edx = 0;
    // A call of Windows' or the driver's: ecx and edx as the code has them going in, as it leaves
    // them after.
    auto scratching = [&](auto made) {
        g.ecx = ecx;
        g.edx = edx;
        const bool done = made();
        ecx = g.ecx;
        edx = g.edx;
        return done;
    };

    g.eax = registration;
    if (!called(app, ts, kTryInit))
        return std::nullopt;
    put(app, registration + 0xc, 0x55e8cd);     // its scopes, in edx as TryInit leaves it
    u32 next = 0xb, race = 0;
    if (!viaValue<kAtExitList, watcom::clib::atexit>(app, ts, 0x4a4020))
        return std::nullopt;
    const std::optional<u32> running = call(0x4a38c0);
    if (!running)
        return std::nullopt;
    if (*running != 0)
    {
        // Another copy running: a box, and out.  The text copied into the frame (rep movsd, the
        // direction clear), its first word handed over as the code hands it.
        for (u32 i = 0; i < 0x18; i += 4)
            put(app, text + i, at(app, 0x4a3050 + i));
        os::messageBox(guest::text(app, at(app, text)), guest::text(app, 0x53ce54), os::Box::plain);
        return out();
    }
    const std::optional<u32> allowed = withEaxEdx(0x4a38f0, 0x55e898, 0x1f);
    if (!allowed)
        return std::nullopt;
    if (*allowed == 0)
    {
        if (!viaValue<0x4b5e50, frontend::wina::fatalMessage>(app, ts, 0u))
            return std::nullopt;
        return out();
    }
    g.eax = guest::direct<0x4f0548, watcom::clib::sscanf>(app, ts, 0x55e898, 0x53ce68, guest::Words{number, 0x55e8b8, 0x55e8ba});
    if (ts.terminate)
        return std::nullopt;
    const u32 scanned = g.eax;
    if (scanned == 0)
    {
        if (!viaValue<0x4b5e50, frontend::wina::fatalMessage>(app, ts, 0u))
            return std::nullopt;
        return out();
    }
    const std::optional<u32> started = withEaxEdx(0x4a5080, argc, argv);
    if (!started)
        return std::nullopt;
    if (*started == 0)
        return out();
    edx = argv;
    // The system's error mode set (no box for a fault): the port has none to set; the registers as
    // the call leaves them.
    auto errorMode = [&] { return scratching([] { return true; }); };
    if (!errorMode())
        return std::nullopt;
    putByte(app, level, 0);
    // The probe's read of [0x190] is skipped (SKIP_INSTRUCTIONS); the byte it went to is the C++'s.
    putByte(app, level, 0xff);
    ebx = 1;
    put(app, kDebugOutput, 1);
    if (!errorMode())
        return std::nullopt;
    putByte(app, level, u8(ebx));
    // The two ways of starting straight into a race: the menus' settings.
    if (byte(app, kStartFlags) & 0x20)
    {
        if (!quickRaceSet(app, ts, false))
            return std::nullopt;
        ebx = 0;
        edx = 0x3f3b;
    }
    if (byte(app, kStartFlags) & 0x10)
    {
        if (!quickRaceSet(app, ts, true))
            return std::nullopt;
        ebx = 0;
        edx = 1;
    }
    // The x87's exceptions unmasked, for a debugging start.
    if (byte(app, kStartFlags + 1) & 2)
        for (const u32 mask : {4u, 2u, 1u, 8u, 0x10u})
        {
            edx = mask;
            if (!withEaxEdx(kControl87, 0, mask))
                return std::nullopt;
        }
    const std::optional<u32> opened = call(0x4a6e10);
    if (!opened)
        return std::nullopt;
    ecx = 0;
    put(app, kErrorWhere, 0);
    if (*opened == 0 && !viaValue<0x4b5e50, frontend::wina::fatalMessage>(app, ts, 0u))
        return std::nullopt;
    if (!call(0x4b6320) || !viaCore<0x4a3390, frontend::loopa::gameStart>(app, ts))
        return std::nullopt;
    if (byte(app, kStartFlags) & 0x20)
    {
        for (const u32 at16 : {0x554facu, 0x55500eu, 0x55513cu, 0x55519cu, 0x55519eu, 0x5551a0u})
            putHalf(app, at16, 0);
        for (const u32 at16 : {0x55519au, 0x5551a2u, 0x5551a4u})
            putHalf(app, at16, 1);
        ebx = 0;
        edx = 0;
        if (!viaValue<0x4d1750, port::text::loadedLanguageText>(app, ts, ts.sp - 4, s32(gameSettings(app).language)))
            return std::nullopt;
        put(app, 0x56446c, 0x49bfb0);           // in esi, which the call keeps
        if (!guest::direct<0x472d10, render::detailSettingsSet>(app, ts))
            return std::nullopt;
    }
    if (byte(app, kStartFlags) & 0x10)
    {
        for (const u32 at16 : {0x554faeu, 0x555010u, 0x55513cu, 0x55519cu, 0x5551a0u, 0x5551a2u, 0x55519eu})
            putHalf(app, at16, 0);
        putHalf(app, 0x55519a, 1);
        ecx = 1;
        edx = 0;
        // 0x4a3690, loopa's raceSession: the race's state machine, eax the mode, edx the replay.
        if (!viaValue<0x4a3690, frontend::loopa::raceSession>(app, ts, 0u, 0u))
            return std::nullopt;
    }
    else
        // The menus and races in turn, by what each says comes next, until 10.
        while (next != 0xa)
        {
            if (!viaValue<0x451940, frontend::connect::flushKeys>(app, ts, 0xd))
                return std::nullopt;
            std::optional<u32> answer;
            switch (next)
            {
            case 6:
            case 7:
                edx = next == 7 ? 1 : 0;
                answer = viaValue<0x4a3690, frontend::loopa::raceSession>(app, ts, race, edx);
                if (!answer)
                    return std::nullopt;
                next = *answer;
                race = 2;
                break;
            case 0xb:
                menuPrecision(app, ts);
                answer = viaValue<0x4730d0, frontend::menuf::frontEndStart>(app, ts);
                if (!answer)
                    return std::nullopt;
                g.eax = *answer;
                next = *answer;
                racePrecision(app, ts);
                race = 0;
                break;
            case 0xc:
                menuPrecision(app, ts);
                answer = withEax(0x4732b0, next - 6);
                if (!answer)
                    return std::nullopt;
                next = *answer;
                edx = 1;
                racePrecision(app, ts);
                race = edx;
                break;
            default:
                break;
            }
        }
    // The game quits: the screen cleared black and flipped, the sounds stopped.
    const std::optional<u32> folders = viaValue<0x43bcf0, frontend::sime::removeNumberedFolders>(app, ts);
    if (!folders)
        return std::nullopt;
    const std::optional<u32> texts = viaValue<0x4d1880, frontend::menue::freeLanguageTexts>(app, ts, *folders);
    if (!texts)
        return std::nullopt;
    g.eax = *texts;
    if (!scratching([&] {
            return driven<thrash::kThrashWindow, &port::driver::window>(app, ts, 2) && setState(app, ts, thrash::kStateClearColour, 0)
                   && driven<thrash::kThrashClearWindow, &port::driver::clearWindow>(app, ts) && driven<thrash::kThrashPageFlip, &port::driver::pageFlip>(app, ts);
        }))
        return std::nullopt;
    guest::direct<0x4f0f40, frontend::wata::asyncShutdown>(app, ts);
    if (ts.terminate)
        return std::nullopt;
    if (!guest::direct<0x40fed0, port::musicCopyDeleted>(app, ts, ts.sp - 4)
        || !viaCore<0x439b70, frontend::loada::freeCars>(app, ts))
        return std::nullopt;
    edx = (edx & 0xffffff00) | 0xff;
    if (!viaCore<0x4a33a0, frontend::loopa::gameEnd>(app, ts))
        return std::nullopt;
    putByte(app, level, 0xff);
    // A key pending: each read, until none -- an extended one's second byte too.
    std::optional<u32> pending = call(kKbhit);
    if (!pending)
        return std::nullopt;
    while (*pending != 0)
    {
        const std::optional<u32> key = call(kGetch);
        if (!key || (*key == 0 && !call(kGetch)))
            return std::nullopt;
        pending = call(kKbhit);
        if (!pending)
            return std::nullopt;
    }
    const std::optional<u32> flushed = withEax(kFflush, kStandardError);
    if (!flushed)
        return std::nullopt;
    g.eax = *flushed;
    g.edx = edx;
    g.ebx = ebx;
    g.ecx = ecx;
    if (!viaCore<0x4f2a7d, frontend::eab::exitGameNow>(app, ts))
        return std::nullopt;
    return out();
}

/* 0x4a39f0: gameRun of eax and edx; eax the frame's old link, ebx and ecx kept. */
nfs::u32 gameMain(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const Kept kept(g, kEbx | kEcx);
    const std::optional<u32> link = gameRun(app, ts, g.eax, g.edx);
    if (!link)
        return g.eax;
    kept.back(g);
    g.eax = *link;
    ts.sp += 4;
    return g.eax;
}

/* ---- The cabin's instruments ---- */

constexpr u32 kCabinTextures = 0x7922fc;     // sub_4d4ae0's: the cabin's textures, 0 for none
constexpr u32 kCabinFlags = 0x55eb2e;        // a byte
constexpr u8 kNoCabin = 0x20;                // kCabinFlags': no cabin view
constexpr u32 kDriverDirect3d = 0x40;        // kDriverFlags' bit for Direct3D

// THRASH_about's record as the cabin reads it: +0x70 the texture memory's bytes; +0x74 a word read
// both as flags (bit 0: 2 MB will do, bit 15: any will do) and, on Direct3D, as more bytes to add;
// the texture widths it takes from +0x10 to +0x14, the heights from +0x1c to +0x20.
constexpr u32 kAboutTextureBytes = 0x70, kAboutMoreBytes = 0x74, kAboutAnyWillDo = 0x75;
constexpr u32 kAboutWidestLeast = 0x10, kAboutWidestMost = 0x14;
constexpr u32 kAboutHighestLeast = 0x1c, kAboutHighestMost = 0x20;
constexpr s32 kCabinArtSide = 0x100;        // the dash's art: 256 by 256
constexpr s32 kCabinBytes = 0x400000, kCabinBytesWithFlag = 0x200000;

// The car's dash: "%s%s\car.viv" of the cars' folder and the car's name, its chunk "dash.qfs".
constexpr u32 kCarModel = 0x220, kCarModelNumber = 0x14;
constexpr u32 kCabinArchiveName = 0x53b6dc, kCarsFolder = 0x7a28b4;
constexpr u32 kLoadAlignment = 0x10;
constexpr u32 kDashChunk = 0x53b6ec;
constexpr u32 kDashTag = 0x53b6f8;           // what the dash's block is allocated as ("")
constexpr u32 kUnpackAlignment = 0x10;

// The dash's shapes: its textures (sub_4d4ae0, from "0000" to "0001", records of 0x2c from
// 0x791d70, +4 the texture), and the gear's eight pictures (sub_4d4080: "geaR", "geaN", "gea1" to
// "gea6", eight bytes apart, records of 0x2c from 0x791c10).
constexpr u32 kShapeTextures = 0x4d4ae0;     // eax the shapes, edx and ebx two names, ecx the records; the format and an address on the stack
constexpr u32 kDashFirstShape = 0x53b704, kDashLastShape = 0x53b6fc;
constexpr u32 kDashTextures = 0x791d70;
constexpr u32 kShapeRecordBytes = 0x2c, kShapeRecordTexture = 4;
constexpr u32 kRecordData = 0x4f2460;        // eax a record: what it holds
constexpr u32 kShapeTexture = 0x4d4080;      // eax the shapes, edx a name, ebx 0x20, ecx the record; 3 on the stack
constexpr u32 kGearShapeNames = 0x53b70c, kGearShapeNameBytes = 8;
constexpr u32 kGearTextures = 0x791c10;
constexpr u32 kGearPictures = 8, kGearTextureSide = 0x20, kGearTextureKind = 3;
constexpr u32 kFindShape = 0x4ef1e0;         // eax the shapes, edx a name: the shape
constexpr u32 kShapePlace = 0xc;             // a shape's place: two 12-bit signed numbers
constexpr u32 kCabinGear = 0x55d6a4;         // whether the gear's pictures are loaded
constexpr u32 kCabinGearX = 0x55d69c, kCabinGearY = 0x55d6a0;

// The gauges' twelve words of eight bytes, from the dash: the first the gauges' art's place (x and y
// as 8.8 in the dash's textures), then the two needles.
constexpr u32 kCabinGauges = 0x792030, kCabinGaugesBytes = 0x60;
constexpr u32 kGaugesTexture = 0x7922e4;     // the dash's texture they are in
constexpr u32 kGaugesU = 0x7922d8, kGaugesV = 0x7922d4;
constexpr u32 kTexel = 0x53b75c;             // 1/256, a float
constexpr u32 kHalfTexel = 0x53b764;         // 1/512, a double

// A needle's words: its pivot, its scale's two values (+0xc: the speedometer's in km/h, else mph)
// and the dial's points they are at, x and y each.
constexpr u32 kNeedlePivotX = 0, kNeedlePivotY = 4, kNeedleKmh = 0xc;
constexpr u32 kNeedleLow = 0x10, kNeedleHigh = 0x14;
constexpr u32 kNeedleLowX = 0x18, kNeedleLowY = 0x1c, kNeedleHighX = 0x20, kNeedleHighY = 0x24;
constexpr u32 kCabinNeedle = 0x792040;       // the speedometer's
constexpr u32 kCabinOtherNeedle = 0x792068;  // the other's, its scale as it is
constexpr u32 kKmhToMetres = 0x53b74c, kMphToMetres = 0x53b750;  // 1/3.6 and 0.44704, floats
constexpr u32 kNeedleShown = 0x7922d0, kOtherNeedleShown = 0x7922e8;
constexpr u32 kAtan2 = 0x4ea9c0, kSquareRoot = 0x4ea920;  // a float on the stack each, onto the x87's stack
constexpr u32 kOneDown = 0x53b754;           // -1.0f
constexpr u32 kHalfOfIt = 0x53b758;          // 0.5f

namespace
{

// Where a needle's results go: turns a unit, its turn at 0 and its length.
struct NeedleSet
{
    u32 slope, start, length;
};
constexpr NeedleSet kNeedleSet = {0x7922e0, 0x7922f4, 0x7922dc};
constexpr NeedleSet kOtherNeedleSet = {0x7922ec, 0x7922f0, 0x7922f8};

/* carstate::targetOfCamera (0x422bd0) of player 0: the player's car record. */
std::optional<u32> playerCar(App* app, ThreadState& ts)
{
    const u32 car = guest::direct<0x422bd0, carstate::targetOfCamera>(app, ts, u32(0));
    if (ts.terminate)
        return std::nullopt;
    return car;
}

/* load::folderOf (0x4392b0): the name of car `number`'s folder. */
std::optional<u32> carFolderName(App* app, ThreadState& ts, u32 number)
{
    return guest::direct<0x4392b0, load::folderOf>(app, ts, number);
}

/* ea::loadQuietly (0x4e0fe0): the file `name` loaded into a block of its own, 0 for none (edx
 * the alignment, which it keeps: the code's after it). */
std::optional<u32> loadedQuietly(App* app, ThreadState& ts, u32 name)
{
    g.edx = kLoadAlignment;
    return guest::direct<0x4e0fe0, ea::loadQuietly>(app, ts, name, kLoadAlignment);
}

/* load::findEntry (0x4ea040): chunk `chunk` (its name) found in the archive at `archive`, its offset
 * written at `offsetAt` and another word at `otherAt`; nonzero when found.  ebx and ecx as it
 * leaves them, edx the chunk as the code hands it. */
std::optional<u32> chunkFound(App* app, ThreadState& ts, u32 archive, u32 chunk, u32 offsetAt, u32 otherAt)
{
    g.edx = chunk;
    const std::optional<load::BigEntry> found =
        guest::direct<0x4ea040, load::findEntry>(app, ts, archive, chunk, u32(0), offsetAt, otherAt);
    if (!found)
        return std::nullopt;
    g.ebx = found->entry;
    g.ecx = found->last;
    return found->name;
}

/* ea::packedSizeChecked (0x4f2710): the packed chunk at `chunk`'s bytes unpacked (ecx the chunk
 * too, as the code has it there and on to the unpacking; ebx 0x10). */
std::optional<u32> unpackedBytes(App* app, ThreadState& ts, u32 chunk)
{
    g.ecx = chunk;
    g.ebx = kUnpackAlignment;
    return guest::direct<0x4f2710, ea::packedSizeChecked>(app, ts, chunk);
}

/* ea::unpackChecked (0x4f2620): the packed chunk at `chunk` unpacked into `block` (edx it, as the
 * code hands it). */
bool unpacked(App* app, ThreadState& ts, u32 chunk, u32 block)
{
    g.edx = block;
    return guest::direct<0x4f2620, ea::unpackChecked>(app, ts, chunk, block).has_value();
}

/* sub_4d4ae0 by address: the dash's textures from its `shapes`, in `format`, into kDashTextures'
 * records; a record written at `recordAt`.  The textures, 0 for none. */
std::optional<u32> dashTextures(App* app, ThreadState& ts, u32 shapes, u32 format, u32 recordAt)
{
    g.ecx = kDashTextures;
    g.ebx = kDashLastShape;
    g.edx = kDashFirstShape;
    g.eax = shapes;
    if (!called(app, ts, kShapeTextures, {format, recordAt}))
        return std::nullopt;
    return g.eax;
}

/* sub_4f2460 by address: what the record at `record` holds. */
std::optional<u32> recordData(App* app, ThreadState& ts, u32 record)
{
    g.eax = record;
    return game(app, ts, kRecordData);
}

/* sub_4ea654 by address: how many shapes `shapes` has. */
std::optional<u32> shapeCount(App* app, ThreadState& ts, u32 shapes)
{
    return viaValue<0x4ea654, frontend::simc::packCount>(app, ts, shapes);
}

/* sub_4d4080 by address: the shape named at `name` of `shapes` made a texture into `record`. */
bool shapeTexture(App* app, ThreadState& ts, u32 shapes, u32 name, u32 record)
{
    g.ecx = record;
    g.ebx = kGearTextureSide;
    g.edx = name;
    g.eax = shapes;
    return called(app, ts, kShapeTexture, {kGearTextureKind});
}

/* sub_4ef1e0 by address: the shape named at `name` of `shapes`. */
std::optional<u32> shapeNamed(App* app, ThreadState& ts, u32 shapes, u32 name)
{
    g.edx = name;
    g.eax = shapes;
    return viaValue<kFindShape, frontend::eab::shapeFileFind>(app, ts, shapes, name);
}

/* sub_4ea9c0 (stdcall: simd's angleOfFloatsValue, which leaves every register as it was): the angle
 * of (x, y) in turns, as the float the code stores it as. */
std::optional<float> angleOf(App* app, ThreadState& ts, float y, float x)
{
    const u32 eax = g.eax;
    double angle = 0.0;
    try
    {
        angle = guest::direct<kAtan2, frontend::simd::angleOfFloatsValue>(app, ts, bitsOf(y), bitsOf(x));
    }
    catch (const fe::Terminated&)
    {
    }
    if (ts.terminate)
        return std::nullopt;
    g.eax = eax;
    return float(angle);
}

/* sub_4ea920 (stdcall: simc's squareRootOf, which leaves every register as it was): the square root
 * of `value` as it is. */
std::optional<nfs::Float> rootOf(App* app, ThreadState& ts, float value)
{
    const double root = guest::direct<kSquareRoot, frontend::simc::squareRootOf>(app, ts, bitsOf(value));
    if (ts.terminate)
        return std::nullopt;
    return nfs::Float(root);
}

/* An integer as the float fild and fstp make of it. */
float floatOfInteger(s32 value)
{
    return float(nfs::Float(value));
}

/* Whether the driver's texture memory and sizes take the dash's art: more than 4 MB (2 MB, or any,
 * by +0x74's flags; Direct3D's two words together first), and 256 within the widths and the heights
 * it takes.  On Direct3D's way ebx and ecx as the code leaves them: the two words, and their sum. */
bool cabinArtTaken(App* app, ThreadState& ts, u32 about, u8 driver)
{
    bool enough = (driver & kDriverSoftware) || (byte(app, about + kAboutAnyWillDo) & 0x80);
    if (!enough && (driver & kDriverDirect3d))
    {
        g.ebx = at(app, about + kAboutTextureBytes);
        g.ecx = at(app, about + kAboutMoreBytes) + g.ebx;
        enough = s32(g.ecx) > kCabinBytes;
    }
    if (!enough)
        enough = ((byte(app, about + kAboutMoreBytes) & 1) && s32(at(app, about + kAboutTextureBytes)) > kCabinBytesWithFlag)
                 || s32(at(app, about + kAboutTextureBytes)) > kCabinBytes;
    return enough && s32(at(app, about + kAboutWidestLeast)) <= kCabinArtSide
           && s32(at(app, about + kAboutWidestMost)) >= kCabinArtSide
           && s32(at(app, about + kAboutHighestLeast)) <= kCabinArtSide
           && s32(at(app, about + kAboutHighestMost)) >= kCabinArtSide;
}

/* The gear's pictures, for a dash of more than two shapes: the eight textures (kCabinGear set
 * before the first), and the place of the first (its shape's 12-bit x and y) as floats; else
 * kCabinGear 0. */
bool gearLoaded(App* app, ThreadState& ts, u32 shapes)
{
    const std::optional<u32> count = shapeCount(app, ts, shapes);
    if (!count)
        return false;
    if (s32(*count) <= 2)
    {
        put(app, kCabinGear, 0);
        return true;
    }
    for (u32 gear = 0; gear < kGearPictures; ++gear)
    {
        if (gear == 0)
            put(app, kCabinGear, 1);
        if (!shapeTexture(app, ts, shapes, kGearShapeNames + kGearShapeNameBytes * gear,
                          kGearTextures + kShapeRecordBytes * gear))
            return false;
    }
    const std::optional<u32> shape = shapeNamed(app, ts, shapes, kGearShapeNames);
    if (!shape)
        return false;
    const u32 place = at(app, *shape + kShapePlace);
    putFloat(app, kCabinGearX, floatOfInteger(s32(place << 20) >> 20));
    putFloat(app, kCabinGearY, floatOfInteger(s32(place << 4) >> 20));
    return true;
}

/* Whether a needle moves: its two points differ.  1 or 0. */
u32 needleShown(App* app, u32 needle)
{
    return at(app, needle + kNeedleLowX) != at(app, needle + kNeedleHighX)
                   || at(app, needle + kNeedleLowY) != at(app, needle + kNeedleHighY)
               ? 1
               : 0;
}

/* One added to an angle in turns while it is below 0: fldz and fcom's "above", which a C++ compare
 * gives on every value, NaN (never) and -0 (equal) too. */
float upFromBelow(float turn)
{
    while (0.0f > turn)
        turn = float(fp::add(1.0, turn));
    return turn;
}

/* [kOneDown] added while its bits are above 1.0's as integers -- the code's compare, kept: it loops
 * on a NaN or an infinity of the plus sign as the code does. */
float downFromAbove(App* app, float turn)
{
    while (s32(bitsOf(turn)) > 0x3f800000)
        turn = float(fp::add(turn, nfs::Float(floatAt(app, kOneDown))));
    return turn;
}

/* The pivot to one of a needle's points, x or y: the integers' difference. */
s32 fromPivot(App* app, u32 needle, u32 point, u32 pivot)
{
    return s32(at(app, needle + point) - at(app, needle + pivot));
}

struct NeedleTurns
{
    float low, high;
};

/* A needle's angles at its two points (atan2 of each against the pivot, the low's first), each
 * brought into [0, 1]. */
std::optional<NeedleTurns> needleTurns(App* app, ThreadState& ts, u32 needle)
{
    const float lowX = floatOfInteger(fromPivot(app, needle, kNeedleLowX, kNeedlePivotX));
    const float lowY = floatOfInteger(fromPivot(app, needle, kNeedleLowY, kNeedlePivotY));
    const std::optional<float> low = angleOf(app, ts, lowY, lowX);
    if (!low)
        return std::nullopt;
    const float highX = floatOfInteger(fromPivot(app, needle, kNeedleHighX, kNeedlePivotX));
    const float highY = floatOfInteger(fromPivot(app, needle, kNeedleHighY, kNeedlePivotY));
    const std::optional<float> high = angleOf(app, ts, highY, highX);
    if (!high)
        return std::nullopt;
    NeedleTurns turns = {upFromBelow(*low), upFromBelow(*high)};
    turns.low = downFromAbove(app, turns.low);
    turns.high = downFromAbove(app, turns.high);
    return turns;
}

/* A needle set from its scale's values `low` and `high` and its turns there: the high's turn put
 * past the low's the way the scale runs (fcom's CF: below, or either NaN), then the turns a unit,
 * the turn at 0 and its length, the mean of the pivot's distances to its two points.  The x87's
 * operations as the code does them, each float stored where it stores one. */
bool needleSet(App* app, ThreadState& ts, u32 needle, NeedleTurns turns, float low, float high, const NeedleSet& set)
{
    if (belowOrUnordered(low, high))
    {
        if (belowOrUnordered(turns.high, turns.low))
            turns.high = float(fp::add(1.0, turns.high));
    }
    else if (belowOrUnordered(turns.low, turns.high))
        turns.low = float(fp::add(1.0, turns.low));
    const nfs::Float turn = fp::sub(turns.high, turns.low);
    const nfs::Float span = fp::sub(high, low);
    const nfs::Float perUnit = fp::div(turn, span);
    const nfs::Float lowX = nfs::Float(fromPivot(app, needle, kNeedleLowX, kNeedlePivotX));
    const nfs::Float lowXSquared = fp::mul(lowX, lowX);
    const nfs::Float lowY = nfs::Float(fromPivot(app, needle, kNeedleLowY, kNeedlePivotY));
    const nfs::Float lowYSquared = fp::mul(lowY, lowY);
    const float slope = float(perUnit);
    putFloat(app, set.slope, slope);
    const nfs::Float atHigh = fp::mul(high, slope);
    const float highX = floatOfInteger(fromPivot(app, needle, kNeedleHighX, kNeedlePivotX));
    const float lowSquared = float(fp::add(lowXSquared, lowYSquared));
    const float highY = floatOfInteger(fromPivot(app, needle, kNeedleHighY, kNeedlePivotY));
    putFloat(app, set.start, float(fp::sub(turns.high, atHigh)));
    const std::optional<nfs::Float> lowLength = rootOf(app, ts, lowSquared);
    if (!lowLength)
        return false;
    const float highSquared = float(fp::add(fp::mul(highX, highX), fp::mul(highY, highY)));
    const float lowLengthStored = float(*lowLength);
    const std::optional<nfs::Float> highLength = rootOf(app, ts, highSquared);
    if (!highLength)
        return false;
    putFloat(app, set.length,
             float(fp::mul(fp::add(*highLength, lowLengthStored), nfs::Float(floatAt(app, kHalfOfIt)))));
    return true;
}

/* The two needles, each when it moves: the speedometer's, its scale in km/h or mph made metres a
 * second (before its turns), and the other's, its scale as it is (after). */
bool needlesSet(App* app, ThreadState& ts)
{
    const u32 shown = needleShown(app, kCabinNeedle);
    put(app, kNeedleShown, shown);
    if (shown != 0)
    {
        const nfs::Float toMetres =
            floatAt(app, at(app, kCabinNeedle + kNeedleKmh) != 0 ? kKmhToMetres : kMphToMetres);
        const float high = float(fp::mul(nfs::Float(s32(at(app, kCabinNeedle + kNeedleHigh))), toMetres));
        const float low = float(fp::mul(toMetres, nfs::Float(s32(at(app, kCabinNeedle + kNeedleLow)))));
        const std::optional<NeedleTurns> turns = needleTurns(app, ts, kCabinNeedle);
        if (!turns || !needleSet(app, ts, kCabinNeedle, *turns, low, high, kNeedleSet))
            return false;
    }
    const u32 otherShown = needleShown(app, kCabinOtherNeedle);
    put(app, kOtherNeedleShown, otherShown);
    if (otherShown != 0)
    {
        const std::optional<NeedleTurns> turns = needleTurns(app, ts, kCabinOtherNeedle);
        if (!turns)
            return false;
        const float high = floatOfInteger(s32(at(app, kCabinOtherNeedle + kNeedleHigh)));
        const float low = floatOfInteger(s32(at(app, kCabinOtherNeedle + kNeedleLow)));
        if (!needleSet(app, ts, kCabinOtherNeedle, *turns, low, high, kOtherNeedleSet))
            return false;
    }
    return true;
}

/* The gauges' art: its texture and its place in it (the first two words, 8.8: the whole and the
 * fraction as the compiler's division by 256 makes them, as idiv's, toward zero), half a texel in.
 * The y's whole, what the code leaves in eax. */
u32 gaugesPlaced(App* app)
{
    const s32 x = s32(at(app, kCabinGauges)), y = s32(at(app, kCabinGauges + 4));
    const u32 texture = u32(x / 0x100) * 2 + u32(y / 0x100);
    put(app, kGaugesTexture, at(app, kDashTextures + kShapeRecordTexture + kShapeRecordBytes * texture));
    const nfs::Float texel = floatAt(app, kTexel);
    const nfs::Float u = fp::mul(nfs::Float(x % 0x100), texel);
    const nfs::Float v = fp::mul(texel, nfs::Float(y % 0x100));
    const nfs::Float halfTexel = nfs::Float(app->getMemory<double>(kHalfTexel));
    putFloat(app, kGaugesU, float(fp::add(u, halfTexel)));
    putFloat(app, kGaugesV, float(fp::add(halfTexel, v)));
    return u32(y / 0x100);
}

}  // namespace

/* 0x488a80's core: the cabin's instruments loaded for the car -- but with no cabin, in a network
 * race, or where the driver's texture memory and sizes do not take its art: the dash's chunk
 * unpacked from the car's archive, its textures (1555 on softtria, 4444 otherwise), the gauges'
 * twelve words, the gear's pictures if it has them; each needle that moves set; and the gauges'
 * art placed when either does.  The archive's name and the two words sub_4ea040 and sub_4d4ae0
 * write in the frame, as deep as the code has it; the registers at each call as the code has them.
 * eax as the code leaves it: THRASH_about's, the last call's, 0 with neither needle, else the
 * gauges' y's whole. */
bool cabinInstrumentsLoaded(App* app, ThreadState& ts)
{
    const GuestFrame frame(ts, 0x18, 0x144);
    const u32 archiveName = frame - 0x144, otherAt = frame - 0x34, offsetAt = frame - 0x38;
    const u32 recordAt = frame - 0x40;
    const std::optional<u32> about = driven<thrash::kThrashAbout, &port::driver::about>(app, ts);
    if (!about)
        return false;
    put(app, kCabinTextures, 0);
    const u8 cabin = byte(app, kCabinFlags);
    if ((cabin & kNoCabin) || gameSettings(app).playMode == 1)
        return true;
    const u8 driver = byte(app, kDriverFlags);
    g.edx = cabin | u32(driver) << 8;  // the two bytes, as the code has edx at the next calls
    if (!cabinArtTaken(app, ts, *about, driver))
        return true;
    const std::optional<u32> car = playerCar(app, ts);
    if (!car)
        return false;
    const std::optional<u32> carName = carFolderName(app, ts, at(app, at(app, *car + kCarModel) + kCarModelNumber));
    if (!carName)
        return false;
    g.eax = archiveName;
    g.eax = guest::direct<kSprintf, watcom::clib::sprintf>(app, ts, archiveName, kCabinArchiveName,
                                                             guest::Words{kCarsFolder, *carName});
    if (ts.terminate)
        return false;
    const std::optional<bool> exists = guest::direct<0x4e0e60, ea::fileThere>(app, ts, archiveName);
    if (!exists)
        return false;
    if (!*exists)
        return true;
    const std::optional<u32> archive = loadedQuietly(app, ts, archiveName);
    if (!archive)
        return false;
    if (*archive == 0)
        return true;
    const std::optional<u32> found = chunkFound(app, ts, *archive, kDashChunk, offsetAt, otherAt);
    if (!found)
        return false;
    if (*found == 0)
        return release(app, ts, *archive);
    const u32 chunk = at(app, offsetAt) + *archive;
    const std::optional<u32> bytes = unpackedBytes(app, ts, chunk);
    if (!bytes)
        return false;
    const std::optional<u32> shapes = allocate(app, ts, kDashTag, *bytes);
    if (!shapes)
        return false;
    if (*shapes == 0)
        return release(app, ts, *archive);
    if (!unpacked(app, ts, chunk, *shapes) || !release(app, ts, *archive))
        return false;
    // Its textures, and the gauges' words.
    const u32 format = (byte(app, kDriverFlags) & kDriverSofttria) ? kFormat1555 : kFormat4444;
    const std::optional<u32> textures = dashTextures(app, ts, *shapes, format, recordAt);
    if (!textures)
        return false;
    put(app, kCabinTextures, *textures);
    const std::optional<u32> gauges = recordData(app, ts, at(app, recordAt));
    if (!gauges)
        return false;
    // The code's movsd's, two a gauge, the direction flag clear as Watcom's code has it.
    for (u32 offset = 0; offset < kCabinGaugesBytes; offset += 4)
        put(app, kCabinGauges + offset, at(app, *gauges + offset));
    g.ecx = *gauges;  // as the copy leaves them at the next call
    g.edx = *gauges + kCabinGaugesBytes - 8;
    if (!gearLoaded(app, ts, *shapes) || !release(app, ts, *shapes) || !needlesSet(app, ts))
        return false;
    if (at(app, kNeedleShown) == 0 && at(app, kOtherNeedleShown) == 0)
    {
        g.eax = 0;  // the other needle's flag
        return true;
    }
    g.eax = gaugesPlaced(app);
    return true;
}

/* 0x488a80: cabinInstrumentsLoaded; every register but eax as the caller had it. */
nfs::u32 loadCabinInstruments(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const Kept kept(g, kEbx | kEcx | kEdx | kEsi | kEdi);
    if (!cabinInstrumentsLoaded(app, ts))
        return g.eax;
    kept.back(g);
    ts.sp += 4;
    return g.eax;
}

/* ---- A message in the HUD ---- */

constexpr u32 kHudBoxes = 0x724258;          // a player's: 0x280 bytes of quads, 0x20 a corner
constexpr u32 kHudBoxTexture = 0x749a38, kHudFlashTexture = 0x8b4844;
constexpr u32 kHudTextScale = 0x791b50;
constexpr u32 kHalf = 0x53b5e8, kHalfAgain = 0x53b5ec;  // 0.5f each

// A player's HUD box: three quads, then the flashing bar's two (kCornerBytes a corner, four a quad).
constexpr u32 kHudBoxBytes = 0x280, kHudQuadBytes = 0x80, kHudBarFirst = 0x180, kHudBarSecond = 0x200;
constexpr u32 kHudTextFont = 0x44;
constexpr u32 kWhite = 0xffffffff;

/* 0x482940's core: player `player`'s message box in the HUD, `text` in it: its three quads (textured
 * with [0x749a38]), the text centred in it in white at the HUD's scale, then the flashing bar over
 * it (one of two quads by the clock's half second, blended, [0x8b4844]).  The x87's operations as
 * the code does them, each float stored where it stores one; the registers at each call as the code
 * has them.  eax as the last state's answer leaves it. */
bool messageBox(App* app, ThreadState& ts, u32 player, u32 text)
{
    const u32 box = kHudBoxes + player * kHudBoxBytes;
    const auto quad = [&](u32 first) {
        g.eax = first;
        return driven<thrash::kThrashDrawQuad, &port::driver::drawQuad>(app, ts, first, first + kCornerBytes, first + 2 * kCornerBytes, first + 3 * kCornerBytes)
            .has_value();
    };
    if (!stateSent(app, ts, thrash::kStateTexture, at(app, kHudBoxTexture)) || !quad(box)
        || !quad(box + kHudQuadBytes) || !quad(box + 2 * kHudQuadBytes)
        || !setTextScale(app, ts, at(app, kHudTextScale)))
        return false;
    const float modeWidth = float(nfs::Float(s32(at(app, kModeWidth))));
    put(app, kTextColour, kWhite);
    g.edx = kWhite;
    const std::optional<nfs::Float> width = textWidth(app, ts, text);
    if (!width)
        return false;
    // The text's left: the box's middle less half its width (chopped).
    const nfs::Float left = fp::mul(fp::sub(nfs::Float(modeWidth), *width), nfs::Float(floatAt(app, kHalf)));
    const float boxHeight =
        float(fp::sub(nfs::Float(floatAt(app, box + 0x44)), nfs::Float(floatAt(app, box + 4))));
    g.eax = kHudTextFont;
    const std::optional<s32> x = chop(app, ts, left);
    if (!x)
        return false;
    const std::optional<u32> textHeight = viaValue<kTextHeight, port::text::characterHeight>(app, ts, kHudTextFont);
    if (!textHeight)
        return false;
    // Its top: the box's top and half of what its height leaves.
    const nfs::Float spare = fp::sub(nfs::Float(boxHeight), nfs::Float(s32(*textHeight)));
    const nfs::Float top =
        fp::add(fp::mul(spare, nfs::Float(floatAt(app, kHalfAgain))), nfs::Float(floatAt(app, box + 4)));
    g.edx = u32(*x);
    g.eax = text;
    const std::optional<s32> y = chop(app, ts, top);
    if (!y || !drawText(app, ts, text, u32(*x), u32(*y)) || !viaPort<kFlushText, port::text::fontsDrawn>(app, ts)
        || !stateSent(app, ts, thrash::kStateBlend, 1)
        || !stateSent(app, ts, thrash::kStateTexture, at(app, kHudFlashTexture)))
        return false;
    const u32 bar = box + (s32(at(app, kTicks) & 0x3f) < 0x20 ? kHudBarFirst : kHudBarSecond);
    return quad(bar) && stateSent(app, ts, thrash::kStateBlend, 0);
}

/* 0x482940: messageBox for player eax, text edx. */
nfs::u32 hudMessageBox(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const Kept kept(g, kEbx | kEcx);
    if (!messageBox(app, ts, g.eax, g.edx))
        return g.eax;
    kept.back(g);
    ts.sp += 4;
    return g.eax;
}

/* ---- A race's frame ---- */

constexpr u32 kRaceView = 0x7a3d14;          // the view drawn
constexpr u32 kTimes = 0x7a3cf0;             // what each stage took, the last one's into here
constexpr u32 kKeepTime = 0x4bc030;          // eax the view, edx kTimes
constexpr u32 kStartTime = 0x4bc000;         // eax 1, esi where the time goes
constexpr u32 kRaceHudOff = 0x7d36a0, kRaceViewPaused = 0x7a3d10, kRaceCountdown = 0x7a3d18;
constexpr u32 kRaceFrameSteps[] = {0x4dece0, 0x4dedf0, 0x4cc600};

/* 0x4bc160's core: a frame of the race's view for `pass` (0 the main one, 1 the mirror, others
 * neither): the frame's lists set up, the sky (sub_4782e0), the view (sub_41b9b0), the effects, the
 * cars, the smoke and the rest -- each stage timed (sub_4bc030 with what sub_4bc000 or the stage gave
 * back), or without the z-buffer ([0x7a3a60] 0) gathered from the depth buckets (sub_4c7580) as one
 * stage; then the HUD for the main pass
 * (sub_482b00) or the mirror's frame (sub_484030), and the countdown.  A stage gives its time back
 * as Watcom returns a record: into the frame at esi, as deep as the code has it -- esi as the code
 * has it at each call, its copies' too, for what a callee writes there.  The registers each call
 * takes as the code has them; eax as the last call leaves it. */
bool raceFrameDrawn(App* app, ThreadState& ts, u32 pass)
{
    const GuestFrame frame(ts, 0x18, 0x50);
    g.ebx = pass;  // ebx the pass throughout, as the code has it
    u32 esi = g.esi;  // the record a stage's time goes to, as the code has esi: what sub_4c7580 and sub_4c76e0 read
    // The callees of render's own that are still called by address (no typed core): eax the view, esi as above.
    const auto callWith = [&](u32 value, u32 target) {
        std::optional<u32> left;
        if (target == 0x41b9b0)
            left = viaValue<0x41b9b0, render::viewPassDrawn>(app, ts, value, esi);
        else if (target == 0x482b00)
            left = viaValue<0x482b00, render::hudDrawn>(app, ts, value, esi);
        else
            left = viaValue<0x482620, render::countdownDrawn>(app, ts, value, esi);
        if (!left)
            return false;
        g.eax = *left;
        return true;
    };
    // The time a stage gave back copied to kTimes (two movsd, esi past it) and kept: sub_4bc030 draws the list
    // of the view (eax) from kTimes (edx).
    const auto keep = [&](u32 record) {
        put(app, kTimes, at(app, record));
        put(app, kTimes + 4, at(app, record + 4));
        esi = record + 8;
        return viaCore<kKeepTime, frontend::rendd::drawListPlain>(app, ts, kRaceView, kTimes);
    };
    // A stage timed by sub_4bc000 after it (eax 1, esi the record), into the stage's record.
    const auto timed = [&](u32 record) {
        esi = record;
        return viaCore<kStartTime, frontend::rendd::flushAndList>(app, ts, 1, record) && keep(record);
    };
    // The main pass's layers (native_collide's, which do nothing), eax `layer` as the code has it.
    const auto firstLayer = [&](u32 layer) {
        g.eax = layer;
        guest::direct<0x431c40, collide::firstLayerNothing>(app, ts, layer);
        return !ts.terminate;
    };
    const auto secondLayer = [&](u32 layer) {
        g.eax = layer;
        guest::direct<0x431c30, collide::secondLayerNothing>(app, ts, layer);
        return !ts.terminate;
    };
    const auto layers = [&](u32 first, u32 second) {
        return pass != 0 || (firstLayer(first) && secondLayer(second));
    };
    if (pass == 0 && !secondLayer(0))
        return false;
    guest::direct<0x4318c0, collide::forgetStates>(app, ts);
    if (ts.terminate || !viaCore<0x4c7750, frontend::rende::clearBuckets>(app, ts, 1)
        || !viaCore<0x4c75f0, frontend::rende::clearFarBuckets>(app, ts)
        || !viaCore<0x4c73e0, frontend::rende::clearDepthBuckets>(app, ts))
        return false;
    const u32 sky = frame - 0x48;
    // sub_4782e0 takes the view in eax (the race view's sky) and the record in esi.
    esi = sky;
    if (!viaCore<0x4782e0, frontend::loada::skyViewTo>(app, ts, kRaceView, sky) || !keep(sky) || !layers(0, 1))
        return false;
    u32 last;
    if (at(app, kDriverOnCard) == 0)
    {
        if (!callWith(kRaceView, 0x41b9b0) || !layers(1, 2)
            || !viaCore<0x4c6f00, frontend::rende::skidMarksDraw>(app, ts, kRaceView))
            return false;
        esi = frame - 0x40;
        if (!viaCore<0x4bbba0, frontend::rendd::cars>(app, ts, kRaceView))
            return false;
        if (!viaCore<0x4cc5c0, frontend::rende::particlesPass>(app, ts, kRaceView)
            || !viaCore<0x4c7580, frontend::rende::depthBucketsGather>(app, ts, esi))
            return false;
        last = frame - 0x40;
    }
    else
    {
        esi = frame - 0x38;
        if (!callWith(kRaceView, 0x41b9b0) || !timed(frame - 0x38) || !layers(1, 2))
            return false;
        esi = frame - 0x30;
        if (!viaCore<0x4c6f00, frontend::rende::skidMarksDraw>(app, ts, kRaceView) || !timed(frame - 0x30))
            return false;
        esi = frame - 0x28;
        if (!viaCore<0x4bbba0, frontend::rendd::cars>(app, ts, kRaceView) || !timed(frame - 0x28))
            return false;
        esi = frame - 0x20;
        if (!viaCore<0x4cc5c0, frontend::rende::particlesPass>(app, ts, kRaceView)
            || !viaCore<0x4c76e0, frontend::rende::farBucketsGather>(app, ts, esi))
            return false;
        last = frame - 0x20;
    }
    if (!keep(last))
        return false;
    for (u32 step = 0; step < 3; ++step)
    {
        const u32 record = frame - 0x18 + 8 * step;
        esi = record;
        const bool done = step == 0 ? viaCore<0x4dece0, frontend::tail::weatherOnView>(app, ts, kRaceView, record)
            : step == 1 ? viaCore<0x4dedf0, frontend::rende::sparksPass>(app, ts, kRaceView, record)
                        : viaCore<0x4cc600, frontend::rende::dropsPass>(app, ts, kRaceView, record);
        if (!done || !keep(record))
            return false;
    }
    bool hud = true;
    if (pass == 0)
    {
        if (!viaCore<kSetViewScale, frontend::loopb::useClip>(app, ts, kModeRect)
            || !guest::direct<0x431aa0, collide::textStates>(app, ts) || !callWith(kRaceView, 0x482b00))
            return false;
    }
    else if (pass == 1)
    {
        if (!viaCore<0x4bc100, frontend::loopb::viewForHud>(app, ts, kRaceView)
            || !guest::direct<0x431aa0, collide::textStates>(app, ts)
            || !viaCore<0x484030, frontend::hudz::flushFrameQueue>(app, ts))
            return false;
    }
    else
        hud = false;
    if (hud && (!guest::direct<0x431b40, collide::hudStates>(app, ts) || !viaCore<0x4bc130, frontend::loopb::viewForScene>(app, ts, kRaceView)))
        return false;
    if (pass == 0)
    {
        if (at(app, kRaceHudOff) == 0 && at(app, kRaceViewPaused) == 0
            && !callWith(at(app, kRaceCountdown), 0x482620))
            return false;
        if (!firstLayer(2))
            return false;
    }
    return true;
}

/* 0x4bc160: raceFrameDrawn for pass eax. */
nfs::u32 raceFrame(App* app, ThreadState& ts, nfs::Args& args)
{
    const RegisterImage image(args);
    const Kept kept(g, kEbx | kEcx | kEdx | kEsi | kEdi);
    if (!raceFrameDrawn(app, ts, g.eax))
        return g.eax;
    kept.back(g);
    ts.sp += 4;
    return g.eax;
}

}  // namespace graphics
}  // namespace nfs3hp
