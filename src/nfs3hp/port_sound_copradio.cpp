#include "game_settings.h"
#include "native_frontend_loopb.h"
#include "native_carstate.h"
#include "port_snd_speech.h"
#include "port_sound_calls.h"

#include <cstdint>

/* The cops' radio by where they are, rewritten (port.h says how): a car's
 * radio heard from its place -- loud as it is near, panned to it (the
 * player's side in the split screen) -- and the line the radio says for an
 * event, picked at random among the event's, now and then another.
 *
 * The place's distance and angle from the listener and how far ahead it is
 * (port_sound_3d.cpp) are called as C++, through the car's place written on
 * the stack where the code has it, in the arithmetic the thunks read off the
 * x87; the camera's view and the radio's last line, which have no typed
 * function yet, by their addresses. */
namespace nfs3hp
{
namespace port
{

namespace
{

constexpr reg32 kListener = 0x570a78;

// What it calls.
constexpr reg32 kRadioKindOf = 0x4124d0, kDistanceFrom = 0x410cb0, kAngleFrom = 0x410d30, kCameraView = 0x4229c0;

/* A place of a car's -- its three coordinates, as they are in the car -- written
 * where the code builds it on its stack. */
void writePlace(App* app, reg32 to, reg32 car)
{
    store<reg32>(app, to, load<reg32>(app, car + 0x98));
    store<reg32>(app, to + 4, load<reg32>(app, car + 0x9c));
    store<reg32>(app, to + 8, load<reg32>(app, car + 0xa0));
}

/* A distance (16.16) as the radio's: its whole part squared, at most 0x15f9. */
int32_t distanceSquared(int32_t distance)
{
    const int32_t whole = distance / 0x10000;
    const int32_t squared = imul(whole, whole);
    return squared >= 0x15f9 ? 0x15f9 : squared;
}

}  // namespace

/* 0x4127c0's: car `car`'s radio, where its driver (0x4124d0's) speaks: heard
 * from the car's place.  eax as the code leaves it: the radio's answer. */
static Result carRadioHeard(App* app, CPU& cpu, reg32 esp, world::Arithmetic a, reg32 car)
{
    const reg32 frame = esp - 5 * 4;
    const reg32 place = frame - 0x10, sp = place;
    writePlace(app, place, car);
    const reg32 player = load<reg32>(app, load<reg32>(app, car + 0x624) + 0x1f0);
    cpu.esp = sp;
    const Result kind = guest::direct<kRadioKindOf, world::aheadAt>(app, cpu, sp - 4, a, place, car + 0xa4);
    if (!kind)
        return {};
    if (*kind < 0)
        return kind;
    cpu.esp = sp;
    const Result distance = guest::direct<kDistanceFrom, world::distanceAt>(app, cpu, a, kListener, place);
    if (!distance)
        return {};
    reg32 pan;
    if (gameSettings(app).playMode == 1)
        pan = player != 0 ? kSideRight : kSideLeft;
    else
    {
        cpu.esp = sp;
        const Result angle = guest::direct<kAngleFrom, world::angleAt>(app, cpu, sp - 4, a, kListener, place);
        if (!angle)
            return {};
        pan = reg32(*angle);
    }
    return radioStarted(app, cpu, sp - 4 - 4, car, distanceSquared(*distance), *kind, pan, int32_t(player));
}

/* 0x412880's: the cop radio of `kind` for car `car`'s player (its +0x624's),
 * from that car's place.  eax as the code leaves it: the cop radio's. */
static Result playerCopRadioHeard(App* app, CPU& cpu, reg32 esp, world::Arithmetic a, reg32 car, int32_t kind)
{
    const reg32 frame = esp - 3 * 4;
    const reg32 place = frame - 0xc, sp = place;
    const reg32 driver = load<reg32>(app, car + 0x624);
    writePlace(app, place, driver);
    const reg32 player = load<reg32>(app, driver + 0x1f0);
    cpu.esp = sp;
    if (gameSettings(app).playMode == 1)
        guest::direct<kCameraView, carstate::viewCopied>(app, cpu, player, kListener);
    cpu.esp = sp;
    const Result distance = guest::direct<kDistanceFrom, world::distanceAt>(app, cpu, a, kListener, place);
    if (!distance)
        return {};
    return copRadioStarted(app, cpu, sp - 4, int32_t(player), kind, distanceSquared(*distance));
}

/* 0x412be0's: the radio's line for event `event`: by the event (at random
 * among its own), in the split screen the player's; another (0x1c, or the
 * event's first) when 0x4cdee0 says the last line said was the event's.  The
 * line.
 *
 * The answer of 0x4cdee0 is what the code finds in eax: so eax is passed in
 * as the code has it at each call. */
Result copLineChosen(App* app, CPU& cpu, reg32 esp, int32_t event)
{
    const reg32 sp = esp - 4 * 4;
    // Random's remainder of 100: past half or not.
    const auto pastHalf = [&]() { return nextRandom(app) % 100 > 0x32; };
    // The last line said, with eax as the code has it there.
    // (0x4cdee0, loopb's racerCount: the last line the radio said; it reads no register.)
    const auto last = [&](reg32) -> std::optional<reg32> {
        cpu.esp = sp;
        const reg32 line = guest::direct<0x4cdee0, frontend::loopb::racerCount>(app, cpu);
        if (cpu.terminate)
            return std::nullopt;
        return line;
    };
    const int32_t split = gameSettings(app).playMode;
    if (split == 1)
    {
        if (event == 1)
            return pastHalf() ? 0x20 : 0x21;
        const int32_t line = event + 0x20;
        const Result said = last(reg32(event));
        if (!said)
            return {};
        if (event != *said)
            return line;
        return pastHalf() ? 0x29 : line;
    }
    const Result first = last(reg32(event));
    if (!first)
        return {};
    if (*first == 2)
        return event == 1 ? 0x12 : 0x1c;
    int32_t line;
    reg32 eax = reg32(*first);
    if (event == 1)
    {
        // The code divides by 3 in eax:edx, leaving the quotient in eax.
        const reg32 n = nextRandom(app);
        line = 0x10 + int32_t(n % 3);
        eax = n / 3;
    }
    else if (event == 2 || event == 3)
    {
        eax = nextRandom(app) & 1;
        line = int32_t(eax) + (event == 2 ? 0x13 : 0x15);
    }
    else
        line = event + 0x13;
    const Result said = last(eax);
    if (!said)
        return {};
    if (event != *said)
        return line;
    if (pastHalf() && split < 1)
        return 0x1c;
    return event == 2 ? 0x13 : event == 3 ? 0x15 : event + 0x13;
}

/* The radio's entries -- each its thunk (the game's registers and stack in,
 * the result in eax out) around what it does. */

/* 0x4127c0: car eax's radio. */
x86::reg32 carRadio(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, carRadioCalled(app, cpu, e.esp, e.eax));
}

/* 0x4127c0's core: car `car`'s radio; esp the frame base. */
Result carRadioCalled(App* app, CPU& cpu, reg32 esp, reg32 car)
{
    return carRadioHeard(app, cpu, esp, world::arithmeticOf(cpu.fenv.rounding), car);
}

/* 0x412880: the cop radio of kind ebx for car eax's player. */
x86::reg32 playerCopRadio(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, playerCopRadioCalled(app, cpu, e.esp, e.eax, int32_t(e.ebx)));
}

/* 0x412880's core: the cop radio of `kind` for car `car`'s player; esp the frame base. */
Result playerCopRadioCalled(App* app, CPU& cpu, reg32 esp, reg32 car, int32_t kind)
{
    return playerCopRadioHeard(app, cpu, esp, world::arithmeticOf(cpu.fenv.rounding), car, kind);
}

/* 0x412be0: the radio's line for event eax. */
x86::reg32 copLineFor(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, copLineChosen(app, cpu, e.esp, int32_t(e.eax)));
}

}  // namespace port
}  // namespace nfs3hp
