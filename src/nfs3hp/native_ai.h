#ifndef NFS3HP_NATIVE_AI_H_
#define NFS3HP_NATIVE_AI_H_

/* Two functions of the opponents' driving, decompiled: sub_4064f0, the pull an
 * opponent's engine has at its speed, and sub_4070c0, how far the way it wants
 * to go is from the way it points.  Read as C++, they do the generated code's
 * arithmetic in its order through the emulated FPU's own operations (FPU::add,
 * mul, sub; compare; the stores' conversions), so they come out as the
 * generated code does to the bit in every precision the race runs at -- the
 * race's single precision, extended as the Modern Patch has it, and the 80-bit
 * x87 of WITH_PEDANTIC_FPU alike -- and the registers, flags, status word and
 * memory with them.  tools/native_ai_checks.cpp runs both on random states and
 * compares everything.
 *
 * Templates over the application so that the checks can run them on a plain
 * block of memory; native_ai.cpp instantiates them for the game. */

#include <cpu.h>
#include <cstdint>
#include <cstring>

namespace nfs3hp
{
namespace ai
{

/* The integer flags as the generated code leaves them (the instructions whose
 * flags are read: test, cmp, xor; and sahf). */
struct Flags
{
    x86::CPU& cpu;

    template <typename T>
    void logic(T value)
    {
        cpu.flags.cf = 0;
        cpu.flags.of = 0;
        cpu.flags.zf = value == 0;
        cpu.flags.sf = 1 & (value >> (8 * sizeof(T) - 1));
    }

    template <typename T>
    void compare(T a, T b)
    {
        const T result = T(a - b);
        const int top = 8 * sizeof(T) - 1;
        cpu.flags.cf = a < b;
        cpu.flags.of = ((a >> top) & 1) != ((b >> top) & 1) && ((a >> top) & 1) != ((result >> top) & 1);
        cpu.flags.zf = result == 0;
        cpu.flags.sf = 1 & (result >> top);
    }

    // fnstsw ax; sahf
    void statusToFlags()
    {
        cpu.ax = cpu.fpu.status.word;
        cpu.flags.lo = x86::reg8(0x02 | (cpu.ah & 0xd7));
    }

    bool below() const { return cpu.flags.cf; }                       // jb
    bool belowOrEqual() const { return cpu.flags.cf || cpu.flags.zf; } // jbe
};

template <typename App>
inline float f32(App* app, x86::reg32 address)
{
    return app->template getMemory<float>(address);
}

template <typename App>
inline x86::reg32 u32(App* app, x86::reg32 address)
{
    return app->template getMemory<x86::reg32>(address);
}

/* A qword the instruction reads from the game's data, as it reads it. */
template <typename App>
inline double f64(App* app, x86::reg32 address)
{
    return app->template getMemory<double>(address);
}

template <typename App>
inline bool callGuest(App* app, x86::CPU& cpu, x86::reg32 address)
{
    cpu.esp -= 4;
    app->dynamic_call(address, cpu);
    return !cpu.terminate;
}

/* sub_4064f0 (eax: the car): the pull of the opponent's engine at its speed,
 * pushed onto the x87 stack.
 *
 *   car[0x509] = 0;
 *   if (car.flags & 1)                       // held back between two limits
 *       if (!(lim * car[0x7bc] >= car[0x7c0]) || !(car[0x7c0] <= lim)) return 0;
 *   i = fistp(car.speed - 0.5), clamped to 0..111;   // the speed in m/s
 *   p = car.ai.pull[i];                      // the car's table, [0x528]+0x2ec
 *   if (![0x6fd4f0] && car[8] > 100) p *= 0.9f;
 *   if ([0x6fd4cc] && i < 45)          p *= 0.9f;
 *   if (car[0x6e4] > 0)                p *= 1.3f;
 *   if ([0x6fd50c] -- the network's catch-up -- and the car is no player's)
 *       d = car.node - player.node;
 *       if (d <= -3 || d >= 15) p *= (d > -8 && d < 30) ? 2 : 3;
 *   if ((car.flags & 0x10) && i < 2 && p > 12.0) p = 12;
 *   if (([0x55eb2e] & 0x20) && (car.flags & 0x20) && car.speed > 20.0) p *= 0.5;
 *   return p;
 *
 * Each product is the x87's, rounded as the control word says, and stored
 * back to the float p. */
template <typename App>
void aiPull(App* app, x86::CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    Flags flags{ cpu };
    const x86::reg32 car = cpu.eax;
    app->template getMemory<x86::reg8>(car + 0x509) = x86::reg8(0);
    const x86::reg8 carFlags = app->template getMemory<x86::reg8>(car + 0x200);
    flags.logic(x86::reg8(carFlags & 1));
    float p;
    if (carFlags & 1)
    {
        const x86::reg32 limitBits = u32(app, u32(app, car + 0x528) + 0x1cc);
        cpu.eax = limitBits;
        float limit;
        std::memcpy(&limit, &limitBits, 4);
        // fld limit; fmul car[0x7bc]; fcomp car[0x7c0]; fnstsw; sahf; jae
        fpu.compare(fpu.mul(x86::Float(limit), x86::Float(f32(app, car + 0x7bc))), x86::Float(f32(app, car + 0x7c0)));
        flags.statusToFlags();
        if (flags.below())
        {
            flags.logic(x86::reg32(0));  // xor edx, edx: the 0 returned
            fpu.push(x86::Float(0.0f));
            cpu.esp += 4;
            return;
        }
        // fld car[0x7c0]; fcomp limit; fnstsw; sahf; jbe
        fpu.compare(x86::Float(f32(app, car + 0x7c0)), x86::Float(limit));
        flags.statusToFlags();
        if (!flags.belowOrEqual())
        {
            flags.logic(x86::reg32(0));  // xor ecx, ecx
            fpu.push(x86::Float(0.0f));
            cpu.esp += 4;
            return;
        }
    }
    // fld car.speed; fadd -0.5; fistp
    x86::sreg32 i = fpu.toInteger<x86::sreg32>(fpu.add(x86::Float(f32(app, car + 0x7c4)), x86::Float(f64(app, 0x536254))));  // -0.5
    flags.compare(x86::reg32(i), x86::reg32(0x6f));
    if (!(x86::sreg32(i) <= 0x6f))
        i = 0x6f;
    flags.logic(x86::reg32(i));
    if (!(i > 0))
        i = 0;
    const x86::reg32 index = x86::reg32(i);
    const x86::reg32 table = u32(app, car + 0x528);
    p = f32(app, table + index * 4 + 0x2ec);
    const float tenthOff = f32(app, 0x53625c);  // 0.9f

    const x86::reg32 globalA = u32(app, 0x6fd4f0);
    flags.logic(globalA);
    if (globalA == 0)
    {
        const x86::reg32 eight = u32(app, car + 8);
        flags.compare(eight, x86::reg32(0x64));
        if (x86::sreg32(eight) > 0x64)
            p = float(fpu.mul(x86::Float(p), x86::Float(tenthOff)));
    }
    const x86::reg32 globalB = u32(app, 0x6fd4cc);
    flags.compare(globalB, x86::reg32(0));
    if (globalB != 0)
    {
        flags.compare(index, x86::reg32(0x2d));
        if (x86::sreg32(index) < 0x2d)
            p = float(fpu.mul(x86::Float(p), x86::Float(tenthOff)));
    }
    const x86::reg32 boost = u32(app, car + 0x6e4);
    flags.compare(boost, x86::reg32(0));
    if (x86::sreg32(boost) > 0)
        p = float(fpu.mul(x86::Float(p), x86::Float(f32(app, 0x536260))));  // 1.3f

    const x86::reg32 catchUp = u32(app, 0x6fd50c);
    flags.compare(catchUp, x86::reg32(0));
    if (catchUp != 0)
    {
        const x86::reg32 player = u32(app, 0x6fd4f4);
        const x86::reg32 who = u32(app, car + 0x1f0);
        flags.compare(who, player);
        bool far = who != player;
        if (far)
        {
            const x86::reg16 network = app->template getMemory<x86::reg16>(0x7a22da);
            flags.compare(network, x86::reg16(0));
            if (network != 0)
            {
                const x86::reg32 players = u32(app, 0x6fd518);
                flags.compare(who, players);
                far = x86::sreg32(who) < x86::sreg32(players);
            }
        }
        if (far)
        {
            const x86::reg32 playerCar = u32(app, player * 4 + 0x5efa48);
            const x86::reg32 d = u32(app, car + 0x1c) - u32(app, playerCar + 0x1c);
            flags.compare(d, x86::reg32(-3));
            bool outside = x86::sreg32(d) <= -3;
            if (!outside)
            {
                flags.compare(d, x86::reg32(0xf));
                outside = !(x86::sreg32(d) < 0xf);
            }
            if (outside)
            {
                flags.compare(d, x86::reg32(-8));
                bool near = false;
                if (x86::sreg32(d) > -8)
                {
                    flags.compare(d, x86::reg32(0x1e));
                    near = x86::sreg32(d) < 0x1e;
                }
                p = float(fpu.mul(x86::Float(p), x86::Float(f32(app, near ? 0x536264 : 0x536268))));  // 2 or 3
            }
        }
    }

    flags.logic(x86::reg8(carFlags & 0x10));
    if (carFlags & 0x10)
    {
        flags.compare(index, x86::reg32(2));
        if (x86::sreg32(index) < 2)
        {
            // fld p; fst qword; fcomp 12.0; fnstsw; sahf; jbe -- above 12, 12
            const x86::Float value = x86::Float(p);
            const double asDouble = double(value);
            fpu.compare(value, x86::Float(f64(app, 0x53626c)));  // 12.0
            flags.statusToFlags();
            double clamped = asDouble;
            if (!flags.belowOrEqual())
            {
                flags.logic(x86::reg32(0));  // xor edi, edi
                clamped = 12.0;
            }
            p = float(x86::Float(clamped));
        }
    }
    const x86::reg8 edition = app->template getMemory<x86::reg8>(0x55eb2e);
    flags.logic(x86::reg8(edition & 0x20));
    if (edition & 0x20)
    {
        flags.logic(x86::reg8(carFlags & 0x20));
        if (carFlags & 0x20)
        {
            fpu.compare(x86::Float(f32(app, car + 0x7c4)), x86::Float(f64(app, 0x536274)));  // 20.0
            flags.statusToFlags();
            if (!flags.belowOrEqual())
                p = float(fpu.mul(x86::Float(p), x86::Float(f64(app, 0x53627c))));  // 0.5
        }
    }
    std::memcpy(&cpu.eax, &p, 4);
    fpu.push(x86::Float(p));
    cpu.esp += 4;
}

/* sub_4070c0 (eax: the car): how far the way the opponent wants to go is from
 * the way it points, in turns, folded into -1/2..1/2, left at [0x56ec60]; the
 * car's own heading at [0x56ec64], the wanted one at [0x56ec5c].
 *
 *   [0x56ec68] = 0;
 *   a = car.angle;                          // [0x210], 1/1024 of a turn
 *   [0x56ec64] = a > 0 ? a * 0.00097656 : (a + 512) * 0.0009765625 + 0.5;
 *   sub_406e50(car, &d);                    // the way it wants to go
 *   want = atan2(d.x, d.z) * 0.159154943092;   // sub_4e06d9: fpatan; 1 / 2 pi
 *   [0x56ec5c] = want < 0 ? 1 + want : want;
 *   e = [0x56ec64] - [0x56ec5c];
 *   if (e < -0.5) e = 1 + e;
 *   if (e > 0.5) e = e - 1;
 *   [0x56ec60] = e;
 *
 * The way to go and the arc tangent are the game's own functions, called as
 * they are; the rest is the x87's arithmetic, rounded as the control word
 * says, through the same stores to float and double. */
template <typename App>
void aiHeading(App* app, x86::CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    Flags flags{ cpu };
    const x86::reg32 entry = cpu.esp;
    const x86::reg32 saved[] = { cpu.ecx, cpu.edx, cpu.esi, cpu.ebp };
    const x86::reg32 car = cpu.eax;
    // push ecx, edx, esi, ebp; mov ebp, esp; sub esp, 0x30
    const x86::reg32 frame = entry - 16;
    app->template getMemory<x86::reg32>(entry - 4) = saved[0];
    app->template getMemory<x86::reg32>(entry - 8) = saved[1];
    app->template getMemory<x86::reg32>(entry - 12) = saved[2];
    app->template getMemory<x86::reg32>(entry - 16) = saved[3];

    app->template getMemory<x86::reg32>(0x56ec68) = x86::reg32(0);
    const x86::reg32 angle = u32(app, car + 0x210);
    flags.logic(angle);
    x86::Float turns;
    if (x86::sreg32(angle) > 0)
    {
        turns = fpu.mul(x86::Float(x86::sreg32(angle)), x86::Float(f64(app, 0x5362d4)));  // 0.00097656
    }
    else
    {
        const x86::reg32 shifted = angle + 0x200;
        app->template getMemory<x86::reg32>(frame - 4) = shifted;
        turns = fpu.add(fpu.mul(x86::Float(x86::sreg32(shifted)), x86::Float(f64(app, 0x5362dc))),  // 1/1024
                        x86::Float(f64(app, 0x5362e4)));  // 0.5
    }
    // fstp qword [ebp-0xc]; fld qword [ebp-0xc]; fstp dword [0x56ec64]
    const double turnsDouble = double(turns);
    app->template getMemory<double>(frame - 0xc) = turnsDouble;
    app->template getMemory<float>(0x56ec64) = float(x86::Float(turnsDouble));

    // sub_406e50(car) into [ebp-0x30]: the registers as the call finds them
    cpu.ebp = frame;
    cpu.esp = frame - 0x30;
    cpu.esi = frame - 0x30;
    cpu.eax = car;
    cpu.ecx = angle;
    cpu.edx = 0;
    if (!callGuest(app, cpu, 0x406e50))
        return;  // the game is ending: nothing more of this call runs
    // fld [ebp-0x28]; fld [ebp-0x30]; sub_4e06d9: atan2 of the two
    fpu.push(x86::Float(f32(app, frame - 0x28)));
    fpu.push(x86::Float(f32(app, frame - 0x30)));
    if (!callGuest(app, cpu, 0x4e06d9))
        return;
    const x86::Float angleOut = fpu.st(0);
    fpu.pop();
    // fmul 1/2pi; fldz; fxch; fst dword [0x56ec5c]; fstp qword [ebp-0x14]; fcomp qword [ebp-0x14]
    const x86::Float want = fpu.mul(angleOut, x86::Float(f64(app, 0x5362ec)));  // 1 / 2 pi
    app->template getMemory<float>(0x56ec5c) = float(want);
    const double wantDouble = double(want);
    app->template getMemory<double>(frame - 0x14) = wantDouble;
    fpu.compare(x86::Float(0.0), x86::Float(wantDouble));
    flags.statusToFlags();
    if (!flags.belowOrEqual())
        app->template getMemory<float>(0x56ec5c) = float(fpu.add(x86::Float(1.0), x86::Float(wantDouble)));

    // fld [0x56ec64]; fsub [0x56ec5c]; fst [0x56ec60]; fst qword [ebp-0x24]; fcomp -0.5
    const x86::Float error = fpu.sub(x86::Float(f32(app, 0x56ec64)), x86::Float(f32(app, 0x56ec5c)));
    app->template getMemory<float>(0x56ec60) = float(error);
    const double errorDouble = double(error);
    app->template getMemory<double>(frame - 0x24) = errorDouble;
    fpu.compare(error, x86::Float(f64(app, 0x5362f4)));  // -0.5
    flags.statusToFlags();
    if (flags.below())
        app->template getMemory<float>(0x56ec60) = float(fpu.add(x86::Float(1.0), x86::Float(errorDouble)));

    // fld [0x56ec60]; fst qword [ebp-0x1c]; fcomp 0.5; above it, one less
    const x86::Float folded = x86::Float(f32(app, 0x56ec60));
    const double foldedDouble = double(folded);
    app->template getMemory<double>(frame - 0x1c) = foldedDouble;
    fpu.compare(folded, x86::Float(f64(app, 0x5362e4)));  // 0.5
    flags.statusToFlags();
    if (!flags.belowOrEqual())
        app->template getMemory<float>(0x56ec60) = float(fpu.add(x86::Float(foldedDouble), x86::Float(f64(app, 0x5362fc))));  // -1.0

    // mov esp, ebp; pop ebp, esi, edx, ecx; ret
    cpu.ecx = saved[0];
    cpu.edx = saved[1];
    cpu.esi = saved[2];
    cpu.ebp = saved[3];
    cpu.esp = entry + 4;
}

}  // namespace ai
}  // namespace nfs3hp

#endif
