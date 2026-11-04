#include "native_guest.h"

/* The police of a Hot Pursuit race, native whole.  disassemble_nfs3hp.py
 * binds each address to the function of that name here (AI_NATIVES), and the
 * generated code calls it by name.  tools/game_checks holds each against its
 * generated code (ai_race.inc).
 *
 * What the code at 0x401220..0x404d12 is: not the opponents' driving (that is
 * 0x405000 on) but the cops' -- the patrol cars' driver (copDriver, a state
 * machine of twelve states at +0x70c), the pursuit of a speeder, the pull
 * over and the ticket, the roadblocks and the backup the radio calls in, the
 * cops' radio messages; and a few rules of the racers' own (the ramming
 * limits of a car class, the race leader, the catch-up speed).
 *
 * The cops are the cars of the table at 0x5efa88 ([0x5efd88] of them), each
 * with its index (+0x714) and its order -- 8 bytes at 0x56ea10 + index * 8
 * (+0x7ac): the order's kind (+0), whether it was taken (+1), a track node
 * (+2, a word), a second word (+4), a side (+6) and a flag (+7).  The racers
 * a cop may chase are those at 0x5efc48 ([0x5efda0] of them); each has its
 * pursuit, a record of 0x60 bytes at 0x56e410 + i * 0x60 (+0x628): how many
 * times it got away (+0), its worst speeding (+4), the cops who gave up
 * (+0x14), the pursuit's state (+0x1c: 0 none, 1 a chase, 2 pulled over),
 * when it started (+0x20), its radio's last call (+0x24), when the stop
 * ends (+0x28), the cops on it (+0x2c, their cars from +0x30), how many of
 * them have stopped by it (+0x50), the radio calls made (+0x54: backup 1,
 * roadblock 2, a second car 4, the stop 8), the roadblock's car (+0x58) and
 * the cop that called (+0x5c).  A cop's +0x624 is the car it chases and its
 * +0x628 that car's pursuit.  A car's: its node on the track (+0x1c), its
 * flags (+0x200: 1 a racer, 4 a player's, 8 a racer that rams; +0x620,
 * +0x621 bit 0x10 its side of the road, +0x710/+0x711 the cop's), its lane
 * (+0x638), its speeding (+0x6f0), its tickets (+0x2c8), its wanted speed
 * (+0x7b0), its speeds (+0x7c0 signed, +0x7c4), its pull-over spot (+0x8e4
 * on, +0x910 the place), the car it stopped by (+0x9a4), when it may be
 * chased again (+0x634).  [0x7d3684] is the race's clock in ticks;
 * [0x6fd4c0] that the race runs the track backwards; 0x554f10.. the race's
 * random numbers (seed * [0x554f18], the low 16 bits kept); the track's
 * nodes are 0x24 bytes at [0x5dd948], [0x5dd940] of them (their lanes at
 * +0x13, left in the high nibble, right in the low; their widths at +0x17,
 * +0x1b).
 *
 * Written as their code is: each push and pop to the guest's stack, each call
 * with the return address's slot, the frame's locals in the guest's memory,
 * the registers as the code leaves them (the bytes and words it writes of
 * them too), the x87's stack pushed and popped as the x87 does, its
 * arithmetic through x86::FPU in the code's order of operands, the status
 * word in ax where fnstsw puts it.  The four switches go by the targets
 * nfs3.exe's tables hold (0x403018, 0x4032a4, 0x40466c, 0x40482c).  Every
 * other function of the game's -- those of the opponents' driving among
 * them -- is called by address. */
namespace nfs3hp
{
namespace airace
{

namespace
{

using guest::at;
using guest::byte;
using guest::put;
using guest::putByte;
using x86::reg16;
using x86::reg32;
using x86::reg8;
using x86::sreg32;
using App = win32::WinApplication;
using CPU = x86::CPU;
using Float = x86::Float;

// The race's clock; the track run backwards; the track's nodes and their count.
constexpr reg32 kClock = 0x7d3684, kBackwards = 0x6fd4c0;
constexpr reg32 kNodeCount = 0x5dd940, kNodes = 0x5dd948;
// The cops and their count; the racers and theirs; the pursuits; the orders.
constexpr reg32 kCops = 0x5efa88, kCopCount = 0x5efd88;
constexpr reg32 kRacers = 0x5efc48, kRacerCount = 0x5efda0;
constexpr reg32 kPursuits = 0x56e410, kOrders = 0x56ea10;
// The race's random numbers: the seed, the product, the multiplier.
constexpr reg32 kSeed = 0x554f10, kProduct = 0x554f14, kMultiplier = 0x554f18;
// What a cop sizes up of its target each tick (0x56ea58..0x56ea6b): the
// target turning, its side, the gap in nodes, the lane against the cop's,
// behind it; the lanes' votes.
constexpr reg32 kTurning = 0x56ea58, kSide = 0x56ea5c, kGap = 0x56ea60, kLaneVote = 0x56ea64,
                kBehind = 0x56ea68;
// The radio's messages by a cop's radio level (+0x71c); the tables by a
// car's tickets (16 bytes each: the gaps a cop gives up at, the stop's
// length, the next pursuit's wait); a cop's siren state (0x50 bytes each).
constexpr reg32 kRadioMessages = 0x55224c, kByTickets = 0x5521ac, kSirens = 0x7dd850;
// The ramming rules of each racer (0x14 bytes each).
constexpr reg32 kRamming = 0x56ea70;

// What is called by address: the opponents' driving's and the game's.
constexpr reg32 kNodeGap = 0x40e120, kNodeSpeed = 0x40ed40, kDistance = 0x4965b0;
constexpr reg32 kChop = 0x4dfd56, kMemset = 0x4e0640, kYRotation = 0x4dfde0;
constexpr reg32 kSubtract = 0x4e0000, kScaleAdd = 0x4e0050, kDot = 0x4e01f0, kMultiply = 0x4e0280;
constexpr reg32 kSnapToTrack = 0x4205a0, kCarRadio = 0x4127c0, kPlayerCopRadio = 0x412880;
constexpr reg32 kCopChaseSet = 0x411750, kSirenOff = 0x4c9f50, kSirenOn = 0x4c9dd0;
constexpr reg32 kKeyState = 0x4be1a0, kParkCar = 0x4a0980, kCarSettled = 0x4a2690;
constexpr reg32 kSetTimer = 0x405fd0, kResetDriver = 0x405220, kNodeBlocked = 0x405c80;
constexpr reg32 kDriverReady = 0x40df10, kSteerTarget = 0x40dd60, kSteerLanes = 0x40ddb0;
constexpr reg32 kSteer = 0x40de30, kAvoid = 0x40bff0, kStop = 0x40ce70, kStopped = 0x40cd80;
constexpr reg32 kLaneOccupied = 0x40d720, kLaneAhead = 0x40d800, kLaneEdges = 0x40d9f0;
constexpr reg32 kNodeOn = 0x40e160, kAtNode = 0x40f000, kRamWait = 0x40f050;
constexpr reg32 kRivalNear = 0x40deb0, kRivalLane = 0x40de50;
// The cops' radio messages (0x40b220..0x40bd00).
constexpr reg32 kRadioJoin = 0x40b220, kRadioSecond = 0x40b330, kRadioGiveUp = 0x40b480;
constexpr reg32 kRadioJoinWanted = 0x40b530, kRadioCleared = 0x40b670, kRadioBackup = 0x40b720;
constexpr reg32 kRadioTwoCars = 0x40b860, kRadioRoadblock = 0x40b910, kRadioRoadblockSide = 0x40b9c0;
constexpr reg32 kRadioSpikes = 0x40bac0, kRadioStop = 0x40bba0, kRadioStopped = 0x40bc30;
constexpr reg32 kRadioPassed = 0x40bca0, kRadioMissed = 0x40bd00, kRadioRoadblockUp = 0x40b7f0;

void push(App* app, CPU& cpu, reg32 value)
{
    cpu.esp -= 4;
    put(app, cpu.esp, value);
}

reg32 pop(App* app, CPU& cpu)
{
    const reg32 value = at(app, cpu.esp);
    cpu.esp += 4;
    return value;
}

/* A call as the code makes it: the return address's slot, and esp as the
 * callee leaves it.  False when the game stopped in it. */
bool direct(App* app, CPU& cpu, void (*function)(App*, CPU&))
{
    cpu.esp -= 4;
    function(app, cpu);
    return !cpu.terminate;
}

bool byAddress(App* app, CPU& cpu, reg32 target)
{
    cpu.esp -= 4;
    app->dynamic_call(target, cpu);
    return !cpu.terminate;
}

reg16 word(App* app, reg32 a) { return app->getMemory<reg16>(a); }
void putWord(App* app, reg32 a, reg16 value) { app->getMemory<reg16>(a) = value; }

/* movsd: a dword from [esi] to es:[edi], both stepped by the direction flag;
 * rep movsd: ecx of them. */
void movsd(App* app, CPU& cpu)
{
    put(app, cpu.ees + cpu.edi, at(app, cpu.esi));
    const reg32 step = cpu.flags.df ? reg32(0) - 4 : 4;
    cpu.edi += step;
    cpu.esi += step;
}

void repMovsd(App* app, CPU& cpu)
{
    for (; cpu.ecx != 0; --cpu.ecx)
        movsd(app, cpu);
}

// The x87's, as the code has them.
Float f32(App* app, reg32 a) { return Float(app->getMemory<float>(a)); }
Float f64(App* app, reg32 a) { return Float(app->getMemory<double>(a)); }

void fld(CPU& cpu, const Float& value) { cpu.fpu.push(value); }
void fild(App* app, CPU& cpu, reg32 a) { cpu.fpu.push(Float(sreg32(at(app, a)))); }

void fstp(App* app, CPU& cpu, reg32 a)
{
    app->getMemory<float>(a) = float(cpu.fpu.st(0));
    cpu.fpu.pop();
}

void fstp64(App* app, CPU& cpu, reg32 a)
{
    app->getMemory<double>(a) = double(cpu.fpu.st(0));
    cpu.fpu.pop();
}

// fstp st(1)
void fstpSt1(CPU& cpu)
{
    cpu.fpu.st(1) = cpu.fpu.st(0);
    cpu.fpu.pop();
}

// fistp m32: st(0) as an integer, by the rounding in force.
void fistp(App* app, CPU& cpu, reg32 a)
{
    put(app, a, reg32(fp::toInteger<sreg32>(cpu.fpu.st(0))));
    cpu.fpu.pop();
}

void fxch(CPU& cpu, int i)
{
    const Float t = cpu.fpu.st(0);
    cpu.fpu.st(0) = cpu.fpu.st(i);
    cpu.fpu.st(i) = t;
}

// st(0) op= m (fadd m32 and the like); fsubr: st(0) = m - st(0).
void fadd(CPU& cpu, const Float& m) { cpu.fpu.st(0) = fp::add(cpu.fpu.st(0), m); }
void fsub(CPU& cpu, const Float& m) { cpu.fpu.st(0) = fp::sub(cpu.fpu.st(0), m); }
void fsubr(CPU& cpu, const Float& m) { cpu.fpu.st(0) = fp::sub(m, cpu.fpu.st(0)); }
void fmul(CPU& cpu, const Float& m) { cpu.fpu.st(0) = fp::mul(cpu.fpu.st(0), m); }
void fdiv(CPU& cpu, const Float& m) { cpu.fpu.st(0) = fp::div(cpu.fpu.st(0), m); }

// fmul st(0), st(1)
void fmulSt1(CPU& cpu) { cpu.fpu.st(0) = fp::mul(cpu.fpu.st(0), Float(cpu.fpu.st(1))); }

// st(i) op= st(0), popped (faddp st(i) and the like).
void faddp(CPU& cpu, int i)
{
    cpu.fpu.st(i) = fp::add(cpu.fpu.st(i), cpu.fpu.st(0));
    cpu.fpu.pop();
}

void fsubp(CPU& cpu, int i)
{
    cpu.fpu.st(i) = fp::sub(cpu.fpu.st(i), cpu.fpu.st(0));
    cpu.fpu.pop();
}

void fmulp(CPU& cpu, int i)
{
    cpu.fpu.st(i) = fp::mul(cpu.fpu.st(i), cpu.fpu.st(0));
    cpu.fpu.pop();
}

void fchs(CPU& cpu) { cpu.fpu.st(0) = -cpu.fpu.st(0); }

/* fcomp m; fnstsw ax (sahf): st(0) compared and popped, the status word in
 * ax; CF and ZF as sahf sets them.  fcompp: st(0) with st(1), both popped. */
struct Flags
{
    bool cf, zf;
    bool below() const { return cf; }             // jb
    bool belowOrEqual() const { return cf || zf; }  // jbe
};

Flags status(CPU& cpu)
{
    cpu.ax = cpu.fpu.status.word;
    return {(cpu.ah & 1) != 0, (cpu.ah & 0x40) != 0};
}

Flags fcomp(CPU& cpu, const Float& m)
{
    cpu.fpu.compare(cpu.fpu.st(0), m);
    cpu.fpu.pop();
    return status(cpu);
}

Flags fcompp(CPU& cpu)
{
    cpu.fpu.compare(cpu.fpu.st(0), Float(cpu.fpu.st(1)));
    cpu.fpu.pop();
    cpu.fpu.pop();
    return status(cpu);
}

/* mov edx, eax; sar edx, 31; shl edx, 2; sbb eax, edx; sar eax, 2: eax / 4
 * rounded toward 0, edx as it leaves it. */
void quarter(CPU& cpu)
{
    cpu.edx = reg32(sreg32(cpu.eax) >> 31);
    const reg32 carry = (cpu.edx >> 30) & 1;
    cpu.edx <<= 2;
    cpu.eax = cpu.eax - cpu.edx - carry;
    cpu.eax = reg32(sreg32(cpu.eax) >> 2);
}

/* mov edx, eax; sar edx, 31; idiv r: eax by r, the remainder in edx. */
void divide(CPU& cpu, reg32 r)
{
    const sreg32 n = sreg32(cpu.eax), d = sreg32(r);
    cpu.edx = reg32(n >> 31);
    cpu.eax = reg32(n / d);
    cpu.edx = reg32(n % d);
}

reg32 neg(reg32 v) { return reg32(0) - v; }

reg32 imul(reg32 a, reg32 b) { return reg32(std::int64_t(sreg32(a)) * std::int64_t(sreg32(b))); }

}  // namespace

void nothingAtStart(App* app, CPU& cpu);
void nothingAtLoad(App* app, CPU& cpu);
void releaseEdge(App* app, CPU& cpu);
void setUpCops(App* app, CPU& cpu);
void radioOn(App* app, CPU& cpu);
void radioLevelDown(App* app, CPU& cpu);
void radioLevelUp(App* app, CPU& cpu);
void copTopSpeed(App* app, CPU& cpu);
void nearestCopGap(App* app, CPU& cpu);
void setWantedSpeed(App* app, CPU& cpu);
void noteSpeeding(App* app, CPU& cpu);
void pulledOver(App* app, CPU& cpu);
void copRadioCall(App* app, CPU& cpu);
void ticketRadio(App* app, CPU& cpu);
void stopIfCopNear(App* app, CPU& cpu);
void stopForCop(App* app, CPU& cpu);
void sizeUpTarget(App* app, CPU& cpu);
void joinPursuit(App* app, CPU& cpu);
void leaveCopList(App* app, CPU& cpu);
void giveUpPursuit(App* app, CPU& cpu);
void pullOverSpot(App* app, CPU& cpu);
void planPullOver(App* app, CPU& cpu);
void endPullOver(App* app, CPU& cpu);
void copStopsBy(App* app, CPU& cpu);
void stopOver(App* app, CPU& cpu);
void giveTicket(App* app, CPU& cpu);
void targetStopping(App* app, CPU& cpu);
void chaseSpeed(App* app, CPU& cpu);
void pursueTarget(App* app, CPU& cpu);
void votePursuitLane(App* app, CPU& cpu);
void sideTowardNode(App* app, CPU& cpu);
void keepSideOfTarget(App* app, CPU& cpu);
void placeCopAtStart(App* app, CPU& cpu);
void placeCop(App* app, CPU& cpu);
void alwaysOne(App* app, CPU& cpu);
void carReversing(App* app, CPU& cpu);
void copHitByCar(App* app, CPU& cpu);
void voteRoadLane(App* app, CPU& cpu);
void spotSpeeder(App* app, CPU& cpu);
void laneOffRoad(App* app, CPU& cpu);
void atOrderNode(App* app, CPU& cpu);
void sideOfOrder(App* app, CPU& cpu);
void takeOrder(App* app, CPU& cpu);
void parkCop(App* app, CPU& cpu);
void drivePatrol(App* app, CPU& cpu);
void copDriver(App* app, CPU& cpu);
void giveOrder(App* app, CPU& cpu);
void spreadCops(App* app, CPU& cpu);
void radioStopped(App* app, CPU& cpu);
void radioStop(App* app, CPU& cpu);
void initPursuits(App* app, CPU& cpu);
void callRoadblock(App* app, CPU& cpu);
void callTwoCars(App* app, CPU& cpu);
void roadblockSiren(App* app, CPU& cpu);
void watchRoadblock(App* app, CPU& cpu);
void watchSecondCar(App* app, CPU& cpu);
void copsWanted(App* app, CPU& cpu);
void radioJoined(App* app, CPU& cpu);
void radioGiveUp(App* app, CPU& cpu);
void releaseCops(App* app, CPU& cpu);
void dropCop(App* app, CPU& cpu);
void sendCopAhead(App* app, CPU& cpu);
void callBackup(App* app, CPU& cpu);
void orderTaken(App* app, CPU& cpu);
void nothingForCop(App* app, CPU& cpu);
void classRule(App* app, CPU& cpu);
void nothingAtEnd(App* app, CPU& cpu);
void nothingAtEndToo(App* app, CPU& cpu);
void setRammingRules(App* app, CPU& cpu);
void classValue(App* app, CPU& cpu);
void classValueToo(App* app, CPU& cpu);
void rivalTick(App* app, CPU& cpu);
void carContact(App* app, CPU& cpu);
void slowForClass(App* app, CPU& cpu);
void nothingAtStartToo(App* app, CPU& cpu);
void catchUpSpeed(App* app, CPU& cpu);
void findLeader(App* app, CPU& cpu);

namespace
{

/* push ebp; mov ebp, esp; pop ebp; ret */
void emptyHook(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* The flips of a wanted speed's sign (+0x7b3's top bit) the cops' code does
 * after setting it: once for the track run backwards, once for the car on
 * the other side (+0x621 bit 0x10); `first` and `second` the byte registers
 * the code uses. */
void flipWanted(App* app, CPU& cpu, reg32 car, reg8& first, reg8& second)
{
    if (at(app, kBackwards) != 0)
    {
        first = byte(app, car + 0x7b3);
        first ^= 0x80;
        putByte(app, car + 0x7b3, first);
    }
    if (byte(app, car + 0x621) & 0x10)
    {
        second = byte(app, car + 0x7b3);
        second ^= 0x80;
        putByte(app, car + 0x7b3, second);
    }
}

/* The gap from the node [ecx's record + 0x1c]... as the code asks it twice:
 * once for its sign, then again for the value, negated when it was not
 * above 0: |gap| into eax, eax and edx set up by `args` before each call. */
template <typename Args>
bool absoluteGap(App* app, CPU& cpu, Args args)
{
    args();
    if (!byAddress(app, cpu, kNodeGap))
        return false;
    const bool above = sreg32(cpu.eax) > 0;
    args();
    if (!byAddress(app, cpu, kNodeGap))
        return false;
    if (!above)
        cpu.eax = neg(cpu.eax);
    return true;
}

/* The distance to a car as the code asks it twice -- once for its sign, then
 * again, negated when it was not below 0: |distance| on the x87's stack. */
template <typename Args>
bool absoluteDistance(App* app, CPU& cpu, Args args)
{
    args();
    if (!byAddress(app, cpu, kDistance))
        return false;
    fld(cpu, Float(0.0));
    const bool positive = fcompp(cpu).below();
    args();
    if (!byAddress(app, cpu, kDistance))
        return false;
    if (!positive)
        fchs(cpu);
    return true;
}

/* Whether the car eax has stopped for the cops: its driver's state
 * (+0x6e0) 7 for a player's car, 6 for another; eax 1 or 0. */
void stoppedState(App* app, CPU& cpu)
{
    if (byte(app, cpu.eax + 0x200) & 4)
        cpu.eax = at(app, cpu.eax + 0x6e0) == 7 ? 1 : 0;
    else
        cpu.eax = at(app, cpu.eax + 0x6e0) == 6 ? 1 : 0;
}

}  // namespace

/* 0x401220, 0x401230, 0x404580, 0x404650, 0x404c40: empty hooks of the race's
 * start and load and of the cops' -- push ebp, mov ebp, esp, pop ebp. */
void nothingAtStart(App* app, CPU& cpu)
{
    emptyHook(app, cpu);
}

void nothingAtLoad(App* app, CPU& cpu)
{
    emptyHook(app, cpu);
}

/* 0x401240: car eax's watch of bit 0 of +0x620 (its state at +0x62c): 1 when
 * the bit has gone off since it was on (state 2 back to 1), 0 while it is on;
 * with the bit off in state 1, 1 when 0x4be1a0 (8, the car's [+0]) says 0. */
void releaseEdge(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    const auto leave = [&] {
        cpu.ebp = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.edx = at(app, cpu.eax + 0x62c);
    if (cpu.edx == 2)
    {
        if (byte(app, cpu.eax + 0x620) & 1)
        {
            cpu.eax = 0;
            return leave();
        }
        put(app, cpu.eax + 0x62c, 1);
        cpu.eax = 1;
        return leave();
    }
    if (cpu.edx != 1)
        put(app, cpu.eax + 0x62c, 1);
    if (byte(app, cpu.eax + 0x620) & 1)
    {
        put(app, cpu.eax + 0x62c, 2);
        cpu.eax = 0;
        return leave();
    }
    cpu.edx = at(app, cpu.eax);
    cpu.eax = 8;
    if (!byAddress(app, cpu, kKeyState))
        return;
    cpu.eax = cpu.eax == 0 ? 1 : 0;
    leave();
}

/* 0x4012c0: the race's cops set up: each its index (+0x714) and its order
 * (+0x7ac, 0x56ea10 + index * 8), the fast ones (their model's +0x14 0x1f8)
 * marked (+0x711 bit 1); the stopping speed (0x56ea50) 11.18. */
void setUpCops(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    for (cpu.eax = 0; sreg32(cpu.eax) < sreg32(at(app, kCopCount)); ++cpu.eax)
    {
        cpu.edx = at(app, cpu.eax * 4 + kCops);
        cpu.ecx = cpu.eax * 8;
        cpu.ebx = kOrders;
        put(app, cpu.edx + 0x714, cpu.eax);
        cpu.ebx += cpu.ecx;
        cpu.ecx = at(app, cpu.edx + 0x220);
        put(app, cpu.edx + 0x7ac, cpu.ebx);
        if (at(app, cpu.ecx + 0x14) == 0x1f8)
            putByte(app, cpu.edx + 0x711, byte(app, cpu.edx + 0x711) | 1);
    }
    put(app, 0x56ea50, 0x4132cfdf);
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x401320: the cops' radio on (0x56ea54). */
void radioOn(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    put(app, 0x56ea54, 1);
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x401330, 0x401350: cop eax's radio level (+0x71c) one down, to 0; one up,
 * to 8. */
void radioLevelDown(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = at(app, cpu.eax + 0x71c);
    if (sreg32(cpu.edx) > 0)
    {
        cpu.ecx = cpu.edx - 1;
        put(app, cpu.eax + 0x71c, cpu.ecx);
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

void radioLevelUp(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = at(app, cpu.eax + 0x71c);
    if (cpu.edx < 8)
    {
        cpu.ecx = cpu.edx + 1;
        put(app, cpu.eax + 0x71c, cpu.ecx);
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x401370: cop eax's top speed onto the x87's stack: its model's (+0x528's
 * +0x1c8), scaled by 0x536044 for a fast one. */
void copTopSpeed(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    if (byte(app, cpu.eax + 0x711) & 1)
    {
        cpu.eax = at(app, cpu.eax + 0x528);
        fld(cpu, f32(app, cpu.eax + 0x1c8));
        fmul(cpu, f64(app, 0x536044));
        fstp(app, cpu, cpu.ebp - 4);
    }
    else
    {
        cpu.eax = at(app, cpu.eax + 0x528);
        cpu.eax = at(app, cpu.eax + 0x1c8);
        put(app, cpu.ebp - 4, cpu.eax);
    }
    fld(cpu, f32(app, cpu.ebp - 4));
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4013b0: the gap in nodes from car eax to the nearest cop, 10000 at most. */
void nearestCopGap(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    cpu.ecx = cpu.eax;
    cpu.esi = 0x2710;
    for (cpu.ebx = 0; sreg32(cpu.ebx) < sreg32(at(app, kCopCount)); ++cpu.ebx)
    {
        cpu.eax = at(app, cpu.ecx + 0x1c);
        cpu.edi = cpu.ebx * 4;
        put(app, cpu.ebp - 4, cpu.eax);
        cpu.eax = at(app, cpu.edi + kCops);
        cpu.edx = at(app, cpu.ebp - 4);
        cpu.eax = at(app, cpu.eax + 0x1c);
        if (!byAddress(app, cpu, kNodeGap))
            return;
        const bool above = sreg32(cpu.eax) > 0;
        cpu.eax = at(app, cpu.edi + kCops);
        cpu.edx = at(app, cpu.ecx + 0x1c);
        cpu.eax = at(app, cpu.eax + 0x1c);
        if (!byAddress(app, cpu, kNodeGap))
            return;
        if (!above)
            cpu.eax = neg(cpu.eax);
        if (sreg32(cpu.eax) < sreg32(cpu.esi))
            cpu.esi = cpu.eax;
    }
    cpu.eax = cpu.esi;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x401430: cop eax's wanted speed (+0x7b0): its node's speed (0x40ed40)
 * times the first argument plus the second, its top speed at most
 * (copTopSpeed), its sign flipped as flipWanted says.  ret 8. */
void setWantedSpeed(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    cpu.edx = cpu.eax;
    cpu.eax = at(app, cpu.eax + 0x1c);
    if (!byAddress(app, cpu, kNodeSpeed))
        return;
    fmul(cpu, f32(app, cpu.ebp + 0x10));
    fadd(cpu, f32(app, cpu.ebp + 0x14));
    cpu.eax = cpu.edx;
    fstp(app, cpu, cpu.edx + 0x7b0);
    if (!direct(app, cpu, copTopSpeed))
        return;
    fstp(app, cpu, cpu.ebp - 4);
    fld(cpu, f32(app, cpu.edx + 0x7b0));
    if (!fcomp(cpu, f32(app, cpu.ebp - 4)).belowOrEqual())
    {
        cpu.eax = at(app, cpu.ebp - 4);
        put(app, cpu.edx + 0x7b0, cpu.eax);
    }
    flipWanted(app, cpu, cpu.edx, cpu.ah, cpu.bh);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4 + 8;
}

/* 0x4014b0: car eax's speeding (+0x6f0) kept at its worst: its speed (+0x7c4)
 * less its node's (0x40ed40), if more. */
void noteSpeeding(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 8;
    cpu.edx = cpu.eax;
    if (cpu.eax != 0)
    {
        cpu.eax = at(app, cpu.eax + 0x1c);
        if (!byAddress(app, cpu, kNodeSpeed))
            return;
        fsubr(cpu, f32(app, cpu.edx + 0x7c4));
        fld(cpu, f32(app, cpu.edx + 0x6f0));
        fxch(cpu, 1);
        fstp(app, cpu, cpu.ebp - 4);
        if (!fcomp(cpu, f32(app, cpu.ebp - 4)).belowOrEqual())
            cpu.eax = at(app, cpu.edx + 0x6f0);
        else
            cpu.eax = at(app, cpu.ebp - 4);
        put(app, cpu.ebp - 8, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 8);
        put(app, cpu.edx + 0x6f0, cpu.eax);
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x401500: whether car eax's pursuit has it pulled over (state 2). */
void pulledOver(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.eax = at(app, cpu.eax + 0x628);
    cpu.eax = at(app, cpu.eax + 0x1c) == 2 ? 1 : 0;
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x401520: cop eax's call on the radio about car edx (snd::carRadio, the
 * message of its radio level), where the race has the cops' radio
 * (0x6fd4fc bit 0x10), the car is not pulled over and goes at 0x53604c or
 * more, and the cop's last call (+0x724) is past; the next 0x180 ticks on. */
void copRadioCall(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    if (byte(app, 0x6fd4fc) & 0x10)
    {
        cpu.eax = at(app, cpu.edx + 0x628);
        if (at(app, cpu.eax + 0x1c) != 2)
        {
            fld(cpu, f32(app, cpu.edx + 0x7c4));
            if (!fcomp(cpu, f64(app, 0x53604c)).below())
            {
                cpu.eax = at(app, kClock);
                if (sreg32(cpu.eax) > sreg32(at(app, cpu.ecx + 0x724)))
                {
                    cpu.edx = at(app, cpu.ecx + 0x71c);
                    cpu.eax = cpu.ecx;
                    cpu.edx = at(app, cpu.edx * 4 + kRadioMessages);
                    if (!byAddress(app, cpu, kCarRadio))
                        return;
                    cpu.eax = at(app, kClock);
                    cpu.eax += 0x180;
                    put(app, cpu.ecx + 0x724, cpu.eax);
                }
            }
        }
    }
    cpu.ebp = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x401580: cop eax's radio to the player of car edx about its ticket
 * (snd::playerCopRadio): the message of its radio level, the tickets the car
 * has left ([0x6fd4bc] less +0x2c8). */
void ticketRadio(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.edx;
    cpu.edx = at(app, 0x6fd4bc);
    cpu.ebx = at(app, cpu.ecx + 0x2c8);
    cpu.edx -= cpu.ebx;
    cpu.ebx = cpu.edx;
    cpu.edx = at(app, cpu.eax + 0x71c);
    cpu.edx = at(app, cpu.edx * 4 + kRadioMessages);
    if (!byAddress(app, cpu, kPlayerCopRadio))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4015b0: car eax pulled over (its pursuit's state 2, the stop's end 0x140
 * ticks on) when one of the cops on it is less than 10 nodes away -- unless
 * it is not chased or is marked +0x200 bit 0x20. */
void stopIfCopNear(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ebx = cpu.eax;
    cpu.ecx = at(app, cpu.eax + 0x628);
    if (cpu.ecx != 0 && at(app, cpu.ecx + 0x1c) != 0 && (byte(app, cpu.eax + 0x200) & 0x20) == 0)
    {
        for (cpu.esi = 0; sreg32(cpu.esi) < sreg32(at(app, cpu.ecx + 0x2c)); ++cpu.esi)
        {
            cpu.eax = cpu.esi * 4;
            cpu.edi = cpu.ecx + cpu.eax;
            if (!absoluteGap(app, cpu, [&] {
                    cpu.eax = at(app, cpu.edi + 0x30);
                    cpu.edx = at(app, cpu.eax + 0x1c);
                    cpu.eax = at(app, cpu.ebx + 0x1c);
                }))
                return;
            if (sreg32(cpu.eax) < 0xa)
            {
                cpu.eax = at(app, kClock);
                put(app, cpu.ecx + 0x1c, 2);
                cpu.eax += 0x140;
                put(app, cpu.ecx + 0x28, cpu.eax);
                break;
            }
        }
    }
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x401650: racer eax stops for cop edx (by its index): the cop's roadblock
 * mark cleared; the racer's pursuit pulled over, unless the racer is out
 * (+0x2c4 3 or 4); a cop chasing no one joins (joinPursuit, mode 0 -- for a
 * player's cop mode 3 and only within 20 nodes) where the racer is speeding
 * or chased; the stop's end 0x140 ticks on if any cop is on it. */
void stopForCop(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    const auto leave = [&] {
        cpu.ebp = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    const auto stopEnds = [&] {
        if (at(app, cpu.esi + 0x2c) == 0)
            return;
        cpu.eax = at(app, kClock);
        cpu.eax += 0x140;
        put(app, cpu.esi + 0x28, cpu.eax);
    };
    cpu.ebx = cpu.eax;
    cpu.ecx = at(app, cpu.edx * 4 + kCops);
    putByte(app, cpu.ecx + 0x711, byte(app, cpu.ecx + 0x711) & 0xf7);
    if ((byte(app, cpu.ebx + 0x200) & 1) == 0)
        return leave();
    cpu.esi = at(app, cpu.ebx + 0x628);
    put(app, cpu.esi + 0x1c, 2);
    if ((byte(app, cpu.ebx + 0x200) & 1) && at(app, cpu.ebx + 0x2c4) == 3)
        return leave();
    if ((byte(app, cpu.ebx + 0x200) & 1) && at(app, cpu.ebx + 0x2c4) == 4)
        return leave();
    const bool wanted = sreg32(at(app, cpu.ebx + 0x6f0)) > 0x40000000 || at(app, cpu.esi + 0x2c) != 0;
    if ((byte(app, cpu.ecx + 0x200) & 4) == 0)
    {
        if (!wanted && at(app, cpu.esi + 0x14) == 0)
            return leave();
        cpu.eax = at(app, cpu.ecx + 0x624);
        if (cpu.eax == 0)
        {
            cpu.edx = cpu.ecx;
            push(app, cpu, 0);
            if (!direct(app, cpu, joinPursuit))
                return;
        }
        stopEnds();
        return leave();
    }
    if (!wanted && at(app, cpu.esi + 0x14) == 0)
        return leave();
    if (at(app, cpu.ecx + 0x624) != 0)
        return leave();
    if (!absoluteGap(app, cpu, [&] {
            cpu.edx = at(app, cpu.ecx + 0x1c);
            cpu.eax = at(app, cpu.ebx + 0x1c);
        }))
        return;
    if (sreg32(cpu.eax) >= 0x14)
        return leave();
    cpu.eax = 3;
    cpu.edx = cpu.ecx;
    push(app, cpu, 0);
    if (!direct(app, cpu, joinPursuit))
        return;
    stopEnds();
    leave();
}

/* 0x401780: cop eax sizes up the car it chases, for this tick's decisions:
 * the car turning one way (0x56ea58, its +0x4f4 above 0), on its own side
 * of the road (0x56ea5c: a player's by +0x5f4, another by +0x621 bit 0x10,
 * against the track's way), the gap to it in nodes, signed by the cop's side
 * (0x56ea60), whether it is behind (0x56ea68), its lane against the cop's
 * (0x56ea64: 0 left of it, 1 the same, 2 right). */
void sizeUpTarget(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    const auto leave = [&] {
        cpu.ebp = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.ecx = cpu.eax;
    cpu.edx = at(app, cpu.eax + 0x624);
    if (cpu.edx == 0)
        return leave();
    fld(cpu, f32(app, cpu.edx + 0x4f4));
    fld(cpu, Float(0.0));
    cpu.eax = fcompp(cpu).below() ? 1 : 0;
    put(app, kTurning, cpu.eax);
    cpu.eax = at(app, cpu.ecx + 0x624);
    if (byte(app, cpu.eax + 0x200) & 4)
        cpu.eax = at(app, cpu.eax + 0x5f4) == 0 ? 1 : 0;
    else
        cpu.eax = (byte(app, cpu.eax + 0x621) & 0x10) == 0 ? 1 : 0;
    cpu.eax ^= at(app, kBackwards);
    if (cpu.eax != 0)
        cpu.eax = 1;
    put(app, kSide, cpu.eax);
    cpu.eax = at(app, cpu.ecx + 0x624);
    cpu.edx = at(app, cpu.ecx + 0x1c);
    cpu.eax = at(app, cpu.eax + 0x1c);
    if (!byAddress(app, cpu, kNodeGap))
        return;
    cpu.bl = byte(app, cpu.ecx + 0x621);
    put(app, kGap, cpu.eax);
    cpu.eax = (cpu.bl & 0x10) == 0 ? 1 : 0;
    cpu.eax ^= at(app, kBackwards);
    cpu.eax = cpu.eax != 0 ? 1 : 0xffffffff;
    cpu.edx = at(app, kGap);
    cpu.edx = imul(cpu.edx, cpu.eax);
    put(app, kGap, cpu.edx);
    if (sreg32(cpu.edx) < 0)
        put(app, kBehind, 1);
    else
    {
        cpu.esi = 0;
        put(app, kBehind, cpu.esi);
    }
    cpu.eax = at(app, cpu.ecx + 0x624);
    cpu.ebx = at(app, cpu.ecx + 0x638);
    cpu.edx = at(app, cpu.eax + 0x638);
    if (sreg32(cpu.edx) < sreg32(cpu.ebx))
    {
        cpu.esi = 0;
        put(app, kLaneVote, cpu.esi);
    }
    else if (cpu.edx == cpu.ebx)
        put(app, kLaneVote, 1);
    else
        put(app, kLaneVote, 2);
    leave();
}

/* 0x4018b0: cop edx joins the pursuit of car ebx (eax its kind: 0 by its own
 * sighting, 1 or 2 counted at the pursuit's +8/+0xc, 3 for a player's cop):
 * a cop that had an order lets it go (its siren, 0x4c9f50, off where it was
 * on, the roadblock's car of the order's pursuit cleared); the pursuit
 * started if it was not (state 1, its clock and radio's times); the cop
 * marked as on it and added to its list.  Kind 0 also keeps the car's
 * speeding at its worst against the argument less the node's speed; then
 * the radio's word (radioJoined).  ret 4. */
void joinPursuit(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 8;
    const auto leave = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.esp += 4 + 4;
    };
    cpu.ecx = cpu.eax;
    cpu.esi = at(app, cpu.edx + 0x628);
    if (cpu.esi != 0)
        return leave();
    put(app, cpu.edx + 0x728, cpu.esi);
    cpu.ah = byte(app, cpu.edx + 0x200);
    put(app, cpu.edx + 0x72c, cpu.esi);
    if ((cpu.ah & 4) == 0 && at(app, cpu.edx + 0x730) != 0)
    {
        cpu.al = byte(app, cpu.edx + 0x711);
        if (cpu.al & 4)
        {
            cpu.ah = cpu.al;
            cpu.ah &= 0xfb;
            putByte(app, cpu.edx + 0x711, cpu.ah);
        }
        cpu.al = byte(app, cpu.edx + 0x711);
        if (cpu.al & 8)
        {
            cpu.ah = cpu.al;
            cpu.ah &= 0xf7;
            putByte(app, cpu.edx + 0x711, cpu.ah);
            cpu.eax = at(app, cpu.edx + 0x730);
            if (cpu.eax != 0)
            {
                cpu.eax = at(app, cpu.eax + 0x628);
                if (cpu.eax != 0)
                {
                    put(app, cpu.eax + 0x58, 0);
                    cpu.esi = at(app, cpu.edx + 0x714);
                    cpu.eax = cpu.esi * 4;
                    cpu.eax += cpu.esi;
                    cpu.eax <<= 4;
                    if (at(app, cpu.eax + kSirens) != 0)
                    {
                        cpu.eax = at(app, cpu.edx + 0x714);
                        if (!byAddress(app, cpu, kSirenOff))
                            return;
                    }
                }
            }
        }
    }
    cpu.eax = at(app, cpu.ebx + 0x628);
    cpu.edi = at(app, cpu.eax + 0x1c);
    if (cpu.edi == 0)
    {
        put(app, cpu.eax + 0x1c, 1);
        put(app, cpu.eax + 0x28, cpu.edi);
        put(app, cpu.eax + 0x2c, cpu.edi);
        put(app, cpu.eax + 0x50, cpu.edi);
        put(app, cpu.eax + 0x54, cpu.edi);
        cpu.esi = at(app, kClock);
        put(app, cpu.eax + 0x5c, cpu.edi);
        put(app, cpu.eax + 0x20, cpu.esi);
        cpu.esi += 0x280;
        put(app, cpu.eax + 0x24, cpu.esi);
    }
    put(app, cpu.edx + 0x734, 1);
    put(app, cpu.edx + 0x628, cpu.eax);
    put(app, cpu.edx + 0x624, cpu.ebx);
    putByte(app, cpu.edx + 0x710, byte(app, cpu.edx + 0x710) | 0x10);
    cpu.esi = at(app, cpu.eax + 0x2c);
    put(app, cpu.eax + cpu.esi * 4 + 0x30, cpu.edx);
    put(app, cpu.eax + 0x2c, at(app, cpu.eax + 0x2c) + 1);
    if (cpu.ecx == 0)
    {
        cpu.eax = at(app, cpu.ebx + 0x1c);
        if (!byAddress(app, cpu, kNodeSpeed))
            return;
        fsubr(cpu, f32(app, cpu.ebp + 0x14));
        fld(cpu, f32(app, cpu.ebx + 0x6f0));
        fxch(cpu, 1);
        fstp(app, cpu, cpu.ebp - 8);
        if (!fcomp(cpu, f32(app, cpu.ebp - 8)).belowOrEqual())
            cpu.eax = at(app, cpu.ebx + 0x6f0);
        else
            cpu.eax = at(app, cpu.ebp - 8);
        put(app, cpu.ebp - 4, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 4);
        put(app, cpu.ebx + 0x6f0, cpu.eax);
        cpu.eax = cpu.edx;
        if (!direct(app, cpu, radioJoined))
            return;
        return leave();
    }
    if (cpu.ecx == 3)
        return leave();
    put(app, cpu.eax + cpu.ecx * 4 + 4, at(app, cpu.eax + cpu.ecx * 4 + 4) + 1);
    cpu.eax = cpu.edx;
    if (!direct(app, cpu, radioJoined))
        return;
    leave();
}

/* 0x401a20: cop eax taken off its pursuit's list: those after it moved down
 * one, the count one less (whether it was there or not), the last slot
 * cleared. */
void leaveCopList(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    cpu.eax = at(app, cpu.eax + 0x628);
    if (cpu.eax != 0)
    {
        cpu.esi = 8;
        cpu.edx = 0;
        for (;;)
        {
            cpu.ebx = at(app, cpu.eax + 0x2c);
            if (sreg32(cpu.edx) >= sreg32(cpu.ebx))
                break;
            cpu.ebx = cpu.edx * 4;
            cpu.ebx += cpu.eax;
            if (cpu.esi != 8)
            {
                cpu.edi = at(app, cpu.ebx + 0x30);
                put(app, cpu.ebx + 0x2c, cpu.edi);
            }
            else if (cpu.ecx == at(app, cpu.ebx + 0x30))
                cpu.esi = cpu.edx;
            ++cpu.edx;
        }
        cpu.edx = cpu.ebx - 1;
        put(app, cpu.eax + 0x2c, cpu.edx);
        put(app, cpu.eax + cpu.edx * 4 + 0x30, 0);
    }
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x401a80: cop eax gives up its pursuit (not one that has its target pulled
 * over, unless the cop is a player's): its word on the radio and its order
 * (radioGiveUp), off the list (leaveCopList); the last cop off a chase the
 * car gets away from -- unless +0x621 bit 0x40 -- counts the getaway (+0,
 * +0x14, the state back to 0) and the car's worst speeding (+4, chopped);
 * the cop's pursuit cleared, its radio level up. */
void giveUpPursuit(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    const auto leave = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.edx = cpu.eax;
    if (at(app, cpu.eax + 0x624) == 0)
        return leave();
    cpu.ecx = at(app, cpu.eax + 0x628);
    if (cpu.ecx == 0)
        return leave();
    if (at(app, cpu.ecx + 0x1c) == 2 && (byte(app, cpu.eax + 0x200) & 4) == 0)
        return leave();
    cpu.eax = cpu.edx;
    if (!direct(app, cpu, radioGiveUp))
        return;
    cpu.eax = cpu.edx;
    if (!direct(app, cpu, leaveCopList))
        return;
    cpu.esi = at(app, cpu.ecx + 0x2c);
    if (cpu.esi == 0)
    {
        cpu.eax = at(app, cpu.edx + 0x624);
        if ((byte(app, cpu.eax + 0x621) & 0x40) == 0)
        {
            cpu.edi = at(app, cpu.ecx);
            cpu.eax = at(app, cpu.ecx + 0x14);
            put(app, cpu.ecx + 0x1c, cpu.esi);
            ++cpu.edi;
            ++cpu.eax;
            put(app, cpu.ecx, cpu.edi);
            put(app, cpu.ecx + 0x14, cpu.eax);
            cpu.eax = at(app, cpu.edx + 0x624);
            fld(cpu, f32(app, cpu.eax + 0x6f0));
            if (!byAddress(app, cpu, kChop))
                return;
            fistp(app, cpu, cpu.ebp - 4);
            cpu.eax = at(app, cpu.ecx + 4);
            cpu.ebx = at(app, cpu.ebp - 4);
            if (!(sreg32(cpu.eax) > sreg32(cpu.ebx)))
                cpu.eax = cpu.ebx;
            put(app, cpu.ecx + 4, cpu.eax);
        }
    }
    put(app, cpu.edx + 0x628, 0);
    put(app, cpu.edx + 0x624, 0);
    cpu.bh = byte(app, cpu.edx + 0x710);
    put(app, cpu.edx + 0x734, 0);
    cpu.bh &= 0xef;
    cpu.eax = cpu.edx;
    putByte(app, cpu.edx + 0x710, cpu.bh);
    if (!direct(app, cpu, radioLevelUp))
        return;
    leave();
}

/* 0x401b50: the spot car eax pulls over to, beside it on the side edx says
 * (0.2 of a turn's quarter and 2 to the right, or the same to the left): its
 * heading (+0x210, 1/1024 of a turn) turned by that into the orientation at
 * +0x8ec (ea::yRotation), the offset along it from its place to +0x910, less
 * its height (+0x118) at +0x914. */
void pullOverSpot(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x24;
    cpu.esi = cpu.eax;
    cpu.ecx = at(app, cpu.eax + 0x210);
    if (sreg32(cpu.ecx) > 0)
    {
        fild(app, cpu, cpu.esi + 0x210);
        fmul(cpu, f64(app, 0x536054));
    }
    else
    {
        cpu.eax = cpu.ecx + 0x200;
        put(app, cpu.ebp - 4, cpu.eax);
        fild(app, cpu, cpu.ebp - 4);
        fmul(cpu, f64(app, 0x53605c));
        fadd(cpu, f64(app, 0x536064));
    }
    fstp64(app, cpu, cpu.ebp - 0x18);
    fld(cpu, f64(app, cpu.ebp - 0x18));
    fstp(app, cpu, cpu.ebp - 0x10);
    if (cpu.edx != 0)
    {
        cpu.edi = 0xbe4ccccd;
        cpu.ebx = 0xc0000000;
        put(app, cpu.ebp - 0xc, cpu.edi);
        put(app, cpu.ebp - 8, cpu.ebx);
    }
    else
    {
        cpu.edx = 0x3e4ccccd;
        cpu.eax = 0x40000000;
        put(app, cpu.ebp - 0xc, cpu.edx);
        put(app, cpu.ebp - 8, cpu.eax);
    }
    fld(cpu, f32(app, cpu.ebp - 0x10));
    cpu.esp -= 4;
    fadd(cpu, f32(app, cpu.ebp - 0xc));
    cpu.eax = cpu.esi + 0x8ec;
    fstp(app, cpu, cpu.esp);
    if (!byAddress(app, cpu, kYRotation))
        return;
    cpu.eax = cpu.ebp - 0x24;
    push(app, cpu, cpu.eax);
    cpu.ebx = cpu.ebp - 0x24;
    cpu.edx = cpu.esi + 0x190;
    push(app, cpu, at(app, cpu.ebp - 8));
    cpu.eax = 1;
    cpu.ecx = cpu.esi + 0x910;
    if (!byAddress(app, cpu, kScaleAdd))
        return;
    cpu.eax = 1;
    cpu.edx = cpu.esi + 0x98;
    if (!byAddress(app, cpu, kSubtract))
        return;
    fld(cpu, f32(app, cpu.esi + 0x118));
    fsubr(cpu, f32(app, cpu.esi + 0x914));
    fstp(app, cpu, cpu.esi + 0x914);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x401c20: where car edx pulls over: the identity (0x564498) to its +0x8ec;
 * its node's normal, right and place (the node's signed bytes and fixed
 * point, scaled by 0x53606c); the side -- the road's right of the car (its
 * place against the node's by the normal) against the car's own facing
 * (+0x1a8 by the right) -- its spot there (pullOverSpot), snapped to the
 * track (trackdata::snapToTrack, which leaves the spot's place on the track
 * at esi, the frame's ebp - 0x36); where that place says the spot is no
 * good (its +0x6c set, its +0x14 0, no polygon at +0x18, or the polygon's
 * kind -- its second byte's low six bits -- 0 or 14), the other side's.  The
 * spot on (+0x8e4, +0x8e8). */
void planPullOver(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x130;
    cpu.ebp -= 0x82;
    put(app, cpu.ebp + 0x7a, cpu.edx);
    cpu.eax = at(app, cpu.ebp + 0x7a);
    cpu.eax = at(app, cpu.eax + 0x1c);
    cpu.edx = 0;
    put(app, cpu.ebp + 0x72, cpu.eax);
    cpu.eax = at(app, cpu.ebp + 0x7a);
    put(app, cpu.ebp + 0x76, cpu.edx);
    cpu.edx = at(app, cpu.ebp + 0x72);
    cpu.edi = cpu.eax + 0x8ec;
    cpu.eax = cpu.edx * 8;
    cpu.ecx = 9;
    cpu.edx += cpu.eax;
    cpu.eax = at(app, kNodes);
    cpu.esi = 0x564498;
    cpu.edx <<= 2;
    cpu.eax += cpu.edx;
    repMovsd(app, cpu);
    // The node's three bytes of its normal, of its right, its place: each
    // times 0x53606c into the frame.
    fld(cpu, f64(app, 0x53606c));
    const struct { reg32 from, to; } bytes[] = {{0x15, 0x42}, {0x16, 0x46}, {0x17, 0x4a},
                                                {0x11, 0x66}, {0x12, 0x6a}, {0x13, 0x6e}};
    for (const auto& b : bytes)
    {
        cpu.edx = at(app, cpu.eax + b.from);
        cpu.edx = reg32(sreg32(cpu.edx) >> 24);
        cpu.edx <<= 9;
        put(app, cpu.ebp + 0x7e, cpu.edx);
        fild(app, cpu, cpu.ebp + 0x7e);
        fmulSt1(cpu);
        fstp(app, cpu, cpu.ebp + b.to);
    }
    fild(app, cpu, cpu.eax);
    fmulSt1(cpu);
    fstp(app, cpu, cpu.ebp + 0x5a);
    fild(app, cpu, cpu.eax + 4);
    fmulSt1(cpu);
    fstp(app, cpu, cpu.ebp + 0x5e);
    fild(app, cpu, cpu.eax + 8);
    fmulp(cpu, 1);
    fstp(app, cpu, cpu.ebp + 0x62);
    cpu.ebx = at(app, cpu.ebp + 0x7a);
    cpu.ecx = cpu.ebp + 0x4e;
    cpu.edx = cpu.ebp + 0x5a;
    cpu.eax = 1;
    cpu.ebx += 0x98;
    if (!byAddress(app, cpu, kSubtract))
        return;
    cpu.edx = cpu.ebp + 0x42;
    cpu.eax = cpu.ebp + 0x4e;
    if (!byAddress(app, cpu, kDot))
        return;
    fld(cpu, Float(0.0));
    if (fcompp(cpu).below())
        put(app, cpu.ebp + 0x76, 1);
    cpu.eax = at(app, cpu.ebp + 0x7a);
    cpu.edx = cpu.ebp + 0x66;
    cpu.eax += 0x1a8;
    if (!byAddress(app, cpu, kDot))
        return;
    fld(cpu, Float(0.0));
    if (!fcompp(cpu).belowOrEqual())
        putByte(app, cpu.ebp + 0x76, byte(app, cpu.ebp + 0x76) ^ 1);
    cpu.edx = at(app, cpu.ebp + 0x76);
    cpu.eax = at(app, cpu.ebp + 0x7a);
    cpu.ebx = at(app, cpu.ebp + 0x72);
    cpu.esi = cpu.ebp - 0x36;
    if (!direct(app, cpu, pullOverSpot))
        return;
    cpu.eax = at(app, cpu.ebp + 0x7a);
    cpu.edx = 1;
    cpu.eax += 0x910;
    if (!byAddress(app, cpu, kSnapToTrack))
        return;
    // The spot kept where its place on the track (snapToTrack's at esi) has
    // +0x6c 0, +0x14 set and a polygon at +0x18 of a kind not 0 nor 14.
    bool other = true;
    if (at(app, cpu.ebp + 0x36) == 0 && at(app, cpu.ebp - 0x22) != 0)
    {
        cpu.edi = at(app, cpu.ebp - 0x1e);
        if (cpu.edi != 0)
        {
            cpu.al = byte(app, cpu.edi + 1);
            cpu.eax &= 0xff;
            if (cpu.al & 0x3f)
            {
                cpu.edx = at(app, cpu.ebp - 0x1e);
                if (cpu.edx == 0)
                    cpu.eax = 1;
                else
                {
                    cpu.al = byte(app, cpu.edx + 1);
                    cpu.eax &= 0xff;
                }
                cpu.eax &= 0x3f;
                cpu.eax = cpu.eax == 0xe ? 0 : 1;
                other = cpu.eax == 0;
            }
        }
    }
    if (other)
    {
        cpu.dl = byte(app, cpu.ebp + 0x76);
        cpu.dl ^= 1;
        cpu.eax = at(app, cpu.ebp + 0x7a);
        putByte(app, cpu.ebp + 0x76, cpu.dl);
        cpu.ebx = at(app, cpu.ebp + 0x72);
        cpu.edx = at(app, cpu.ebp + 0x76);
        cpu.esi = cpu.ebp - 0xae;
        if (!direct(app, cpu, pullOverSpot))
            return;
        cpu.eax = at(app, cpu.ebp + 0x7a);
        cpu.edx = 1;
        cpu.eax += 0x910;
        if (!byAddress(app, cpu, kSnapToTrack))
            return;
    }
    cpu.eax = at(app, cpu.ebp + 0x7a);
    put(app, cpu.eax + 0x8e8, 1);
    put(app, cpu.eax + 0x8e4, 1);
    cpu.esp = cpu.ebp + 0x82;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x401e20: car eax's pull-over spot off. */
void endPullOver(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    put(app, cpu.eax + 0x8e8, 0);
    put(app, cpu.eax + 0x8e4, 0);
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x401e40: cop eax at a car pulled over: of several cops on a player's car
 * the nearest (0x4965b0, under 10000) stops by it, the others drop off
 * (dropCop) -- all of them once one has; the one that stops (+0x710 bit
 * 0x80) is counted (+0x50) and the first sets the stop's end (0xc0 ticks for
 * a player's cop, else the car's tickets' wait 0x5521b8), radios a player's
 * car its ticket (ticketRadio), plans its spot (planPullOver), and the two
 * cars are marked as stopped by each other (+0x9a4). */
void copStopsBy(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x14;
    const auto leave = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.edi = cpu.eax;
    cpu.ebx = at(app, cpu.eax + 0x628);
    cpu.ecx = cpu.eax;
    cpu.edx = at(app, cpu.ebx + 0x2c);
    cpu.esi = at(app, cpu.eax + 0x624);
    if (sreg32(cpu.edx) > 1 && (byte(app, cpu.eax + 0x200) & 4))
    {
        cpu.eax = 0x461c4000;
        cpu.edx = at(app, cpu.ebx + 0x50);
        put(app, cpu.ebp - 0x14, cpu.eax);
        if (cpu.edx != 0)
            cpu.ecx ^= cpu.edi;
        else
        {
            put(app, cpu.ebp - 4, cpu.edx);
            for (;;)
            {
                cpu.eax = at(app, cpu.ebp - 4);
                if (sreg32(cpu.eax) >= sreg32(at(app, cpu.ebx + 0x2c)))
                    break;
                cpu.eax = cpu.esi + 0x98;
                put(app, cpu.ebp - 0x10, cpu.eax);
                cpu.eax = at(app, cpu.ebp - 4);
                cpu.eax <<= 2;
                cpu.edx = cpu.ebx + cpu.eax;
                put(app, cpu.ebp - 8, cpu.edx);
                cpu.eax = at(app, cpu.edx + 0x30);
                cpu.edx = at(app, cpu.ebp - 0x10);
                cpu.eax += 0x98;
                if (!byAddress(app, cpu, kDistance))
                    return;
                fld(cpu, Float(0.0));
                const bool positive = fcompp(cpu).below();
                cpu.eax = at(app, cpu.ebp - 8);
                cpu.eax = at(app, cpu.eax + 0x30);
                cpu.edx = at(app, cpu.ebp - 0x10);
                cpu.eax += 0x98;
                if (!byAddress(app, cpu, kDistance))
                    return;
                if (!positive)
                    fchs(cpu);
                fstp(app, cpu, cpu.ebp - 0xc);
                fld(cpu, f32(app, cpu.ebp - 0xc));
                if (fcomp(cpu, f32(app, cpu.ebp - 0x14)).below())
                {
                    cpu.eax = at(app, cpu.ebp - 0xc);
                    put(app, cpu.ebp - 0x14, cpu.eax);
                    cpu.eax = at(app, cpu.ebp - 4);
                    cpu.ecx = at(app, cpu.ebx + cpu.eax * 4 + 0x30);
                }
                put(app, cpu.ebp - 4, at(app, cpu.ebp - 4) + 1);
            }
        }
    }
    if (cpu.edi != cpu.ecx)
    {
        cpu.eax = cpu.edi;
        if (!direct(app, cpu, dropCop))
            return;
        return leave();
    }
    cpu.dl = byte(app, cpu.ecx + 0x710);
    cpu.dl |= 0x80;
    putByte(app, cpu.ecx + 0x710, cpu.dl);
    cpu.dh = cpu.dl;
    cpu.dh &= 0xef;
    putByte(app, cpu.ecx + 0x710, cpu.dh);
    cpu.edx = at(app, cpu.ebx + 0x50);
    ++cpu.edx;
    put(app, cpu.ebx + 0x50, cpu.edx);
    if (cpu.edx != 1)
        return leave();
    if (byte(app, cpu.ecx + 0x200) & 4)
    {
        cpu.eax = at(app, kClock);
        cpu.eax += 0xc0;
    }
    else
    {
        cpu.edi = at(app, cpu.esi + 0x2c8);
        cpu.edi <<= 4;
        cpu.eax = at(app, kClock);
        cpu.edx = at(app, cpu.edi + 0x5521b8);
        cpu.eax += cpu.edx;
    }
    put(app, cpu.ebx + 0x28, cpu.eax);
    if (byte(app, cpu.esi + 0x200) & 4)
    {
        cpu.edx = cpu.esi;
        cpu.eax = cpu.ecx;
        if (!direct(app, cpu, ticketRadio))
            return;
    }
    cpu.edx = cpu.esi;
    cpu.eax = cpu.ecx;
    if (!direct(app, cpu, planPullOver))
        return;
    put(app, cpu.esi + 0x9a4, cpu.ecx);
    put(app, cpu.ecx + 0x9a4, cpu.esi);
    leave();
}

/* 0x401fb0: whether cop eax is done with a stop: 1 without a pursuit or a
 * target, 0 while the stop lasts (+0x28); at its end the ticket (giveTicket)
 * and 1. */
void stopOver(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = at(app, cpu.eax + 0x628);
    if (cpu.edx != 0 && at(app, cpu.eax + 0x624) != 0)
    {
        cpu.ecx = at(app, kClock);
        if (sreg32(cpu.ecx) <= sreg32(at(app, cpu.edx + 0x28)))
        {
            cpu.eax = 0;
            cpu.ebp = pop(app, cpu);
            cpu.edx = pop(app, cpu);
            cpu.ecx = pop(app, cpu);
            cpu.esp += 4;
            return;
        }
        if (!direct(app, cpu, giveTicket))
            return;
    }
    cpu.eax = 1;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x401ff0: cop eax gives the car it stopped its ticket: the pursuit's count
 * and worst speeding kept, the cop's tally (+0x2cc, its time at the race's
 * limit 0x6fd4b8) and fines (+0x2d0, the speeding chopped), the car's
 * tickets one more -- out of the race at [0x6fd4bc] (+0x2c4 4) -- its
 * speeding cleared, its next pursuit's wait (0x5521b4 by its tickets); both
 * cars' marks of the stop cleared, the spot off, the cops let go
 * (releaseCops) and the pursuit ended. */
void giveTicket(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 8;
    cpu.ecx = cpu.eax;
    cpu.edx = at(app, cpu.eax + 0x624);
    cpu.ebx = at(app, cpu.eax + 0x628);
    putByte(app, cpu.edx + 0x621, byte(app, cpu.edx + 0x621) & 0xbf);
    cpu.esi = at(app, cpu.ebx);
    put(app, cpu.ebx + 0x14, 0);
    ++cpu.esi;
    put(app, cpu.ebx, cpu.esi);
    cpu.eax = at(app, cpu.ebx + 4);
    fld(cpu, f32(app, cpu.edx + 0x6f0));
    if (!byAddress(app, cpu, kChop))
        return;
    fistp(app, cpu, cpu.ebp - 4);
    cpu.edi = at(app, cpu.ebp - 4);
    if (!(sreg32(cpu.eax) > sreg32(cpu.edi)))
        cpu.eax = cpu.edi;
    put(app, cpu.ebx + 4, cpu.eax);
    cpu.eax = at(app, cpu.ecx + 0x2cc);
    ++cpu.eax;
    cpu.esi = at(app, 0x6fd4b8);
    put(app, cpu.ecx + 0x2cc, cpu.eax);
    if (cpu.eax == cpu.esi)
    {
        cpu.eax = at(app, kClock);
        cpu.eax -= 0x200;
        put(app, cpu.ecx + 0x2d4, cpu.eax);
    }
    fld(cpu, f32(app, cpu.edx + 0x6f0));
    cpu.edi = at(app, cpu.edx + 0x2c8);
    if (!byAddress(app, cpu, kChop))
        return;
    ++cpu.edi;
    fistp(app, cpu, cpu.ebp - 8);
    put(app, cpu.edx + 0x2c8, cpu.edi);
    cpu.eax = at(app, cpu.ebp - 8);
    put(app, cpu.ecx + 0x2d0, at(app, cpu.ecx + 0x2d0) + cpu.eax);
    cpu.edi = at(app, 0x6fd4bc);
    cpu.eax = at(app, cpu.edx + 0x2c8);
    put(app, cpu.edx + 0x6f0, 0);
    if (!(sreg32(cpu.eax) < sreg32(cpu.edi)))
        put(app, cpu.edx + 0x2c4, 4);
    cpu.esi = at(app, cpu.edx + 0x2c8);
    cpu.esi <<= 4;
    cpu.eax = at(app, kClock);
    cpu.edi = at(app, cpu.esi + 0x5521b4);
    put(app, cpu.edx + 0x9a4, 0);
    cpu.eax += cpu.edi;
    put(app, cpu.edx + 0x634, cpu.eax);
    put(app, cpu.ecx + 0x9a4, 0);
    cpu.eax = cpu.edx;
    putByte(app, cpu.edx + 0x8a, 0);
    if (!direct(app, cpu, endPullOver))
        return;
    cpu.eax = cpu.ecx;
    if (!direct(app, cpu, releaseCops))
        return;
    put(app, cpu.ebx + 0x2c, 0);
    put(app, cpu.ebx + 0x50, 0);
    put(app, cpu.ebx + 0x1c, 0);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x402110: whether cop eax has its target stopped (1) or is still at it
 * (0): with the car pulled over and stopped (stoppedState), 1 once it is
 * within 0x536074; otherwise the cop near enough (1 or 2 times 0x53607c, by
 * the cops already stopped by it), the car slow (0x56ea50), on the ground
 * (+0x164) and the cop's +0x10c above 0x536084: the first time it calls the
 * stop on the radio (copRadioCall; its next call 0xa0..0xe0 ticks on by the
 * car's lap against the leader's) and marks it (+0x710 bit 0x40); later, past
 * that time, it pulls the car over (state 2, 0x140 ticks) and has it if the
 * car has stopped; a player's car not yet braking (+0x508 under 12) gets
 * 0x13 ticks less.  Else the mark cleared and, pulled over, 1 once the car
 * has stopped and the stop is past. */
void targetStopping(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x10;
    const auto leave = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    const auto no = [&] {
        cpu.eax = 0;
        leave();
    };
    const auto yes = [&] {
        cpu.eax = 1;
        leave();
    };
    cpu.ecx = cpu.eax;
    cpu.edx = at(app, cpu.eax + 0x624);
    if (cpu.edx == 0)
        return no();
    cpu.esi = at(app, cpu.eax + 0x628);
    const auto toTarget = [&] {
        cpu.eax = at(app, cpu.ecx + 0x624);
        cpu.edx = cpu.ebx;
        cpu.eax += 0x98;
    };
    if (at(app, cpu.esi + 0x1c) == 2)
    {
        cpu.eax = cpu.edx;
        stoppedState(app, cpu);
        if (cpu.eax != 0)
        {
            cpu.eax = at(app, cpu.ecx + 0x624);
            cpu.ebx = cpu.ecx + 0x98;
            cpu.eax += 0x98;
            cpu.edx = cpu.ebx;
            if (!absoluteDistance(app, cpu, toTarget))
                return;
            fstp(app, cpu, cpu.ebp - 8);
            fld(cpu, f32(app, cpu.ebp - 8));
            if (fcomp(cpu, f64(app, 0x536074)).below())
                return yes();
        }
    }
    cpu.eax = sreg32(at(app, cpu.esi + 0x50)) < 3 ? 1 : 2;
    put(app, cpu.ebp - 4, cpu.eax);
    fild(app, cpu, cpu.ebp - 4);
    fmul(cpu, f64(app, 0x53607c));
    cpu.ebx = cpu.ecx + 0x98;
    cpu.eax = at(app, cpu.ecx + 0x624);
    cpu.edx = cpu.ebx;
    cpu.eax += 0x98;
    fstp(app, cpu, cpu.ebp - 0x10);
    if (!absoluteDistance(app, cpu, toTarget))
        return;
    fstp(app, cpu, cpu.ebp - 0xc);
    fld(cpu, f32(app, cpu.ebp - 0xc));
    bool close = fcomp(cpu, f32(app, cpu.ebp - 0x10)).below();
    if (close)
    {
        cpu.eax = at(app, cpu.ecx + 0x624);
        fld(cpu, f32(app, cpu.eax + 0x7c4));
        close = fcomp(cpu, f32(app, 0x56ea50)).belowOrEqual();
    }
    if (close)
        close = word(app, cpu.ecx + 0x164) == 0;
    if (close)
    {
        fld(cpu, f32(app, cpu.ecx + 0x10c));
        close = !fcomp(cpu, f64(app, 0x536084)).belowOrEqual();
    }
    if (close)
    {
        if ((byte(app, cpu.ecx + 0x710) & 0x40) == 0)
        {
            cpu.eax = at(app, cpu.ecx + 0x624);
            cpu.ebx = 0xc0;
            if ((byte(app, cpu.eax + 0x200) & 4) == 0)
            {
                cpu.edx = at(app, 0x56ebb8);
                cpu.eax = at(app, cpu.eax + 8);
                cpu.eax -= at(app, cpu.edx + 8);
                if (sreg32(cpu.eax) > 0x3c)
                    cpu.ebx = 0xa0;
                else if (sreg32(cpu.eax) < -0x3c)
                    cpu.ebx = 0xe0;
            }
            cpu.eax = cpu.ecx;
            cpu.edx = at(app, cpu.ecx + 0x624);
            if (!direct(app, cpu, copRadioCall))
                return;
            cpu.eax = at(app, kClock);
            cpu.eax += cpu.ebx;
            cpu.bl = byte(app, cpu.ecx + 0x710);
            put(app, cpu.ecx + 0x720, cpu.eax);
            cpu.bl |= 0x40;
            putByte(app, cpu.ecx + 0x710, cpu.bl);
            return no();
        }
        cpu.eax = at(app, kClock);
        cpu.edi = at(app, cpu.ecx + 0x720);
        if (sreg32(cpu.eax) > sreg32(cpu.edi))
        {
            cpu.eax += 0x140;
            put(app, cpu.esi + 0x1c, 2);
            put(app, cpu.esi + 0x28, cpu.eax);
            cpu.eax = at(app, cpu.ecx + 0x624);
            stoppedState(app, cpu);
            if (cpu.eax == 0)
                return no();
            return yes();
        }
        cpu.eax = at(app, cpu.ecx + 0x624);
        if ((byte(app, cpu.eax + 0x200) & 4) && byte(app, cpu.eax + 0x508) < 0xc)
        {
            cpu.ebx = cpu.edi - 0x13;
            put(app, cpu.ecx + 0x720, cpu.ebx);
        }
        return no();
    }
    putByte(app, cpu.ecx + 0x710, byte(app, cpu.ecx + 0x710) & 0xbf);
    if (at(app, cpu.esi + 0x1c) != 2)
        return no();
    cpu.eax = at(app, cpu.ecx + 0x624);
    stoppedState(app, cpu);
    if (cpu.eax == 0)
        return no();
    cpu.eax = at(app, kClock);
    if (sreg32(cpu.eax) <= sreg32(at(app, cpu.esi + 0x28)))
        return no();
    yes();
}

/* 0x402420: cop eax's wanted speed in a chase: a cop told to (+0x7d4 1)
 * keeps by the gap -- behind its target, the gap times 0x53608c; else the
 * target's speed (+0x7c0) times one and the gap by 0x536094; otherwise
 * behind it, the target's speed times 0x53609c for the leader's car
 * (0x56ef54) or 0x5360a4 for another, and, slower than 0x5360ac and more
 * than 7 nodes off, a timer (0x405fd0) and -9.83; ahead of it, its top
 * speed, or within 8 nodes and faster than the target the target's speed
 * and 0x5360bc and the difference by the gap.  The sign flipped as
 * flipWanted says. */
void chaseSpeed(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x10;
    cpu.ecx = cpu.eax;
    cpu.edx = at(app, cpu.eax + 0x624);
    if (cpu.edx != 0)
    {
        if (at(app, cpu.eax + 0x7d4) == 1)
        {
            if (at(app, kBehind) != 0)
            {
                fild(app, cpu, kGap);
                fmul(cpu, f64(app, 0x53608c));
                fstp(app, cpu, cpu.eax + 0x7b0);
            }
            else
            {
                fild(app, cpu, kGap);
                cpu.eax = cpu.edx;
                fmul(cpu, f64(app, 0x536094));
                cpu.edx = at(app, cpu.eax + 0x7c0);
                put(app, cpu.ecx + 0x7b0, cpu.edx);
                fld(cpu, Float(1.0));
                fld(cpu, f32(app, cpu.ecx + 0x7b0));
                fxch(cpu, 1);
                faddp(cpu, 2);
                fmulp(cpu, 1);
                fstp(app, cpu, cpu.ecx + 0x7b0);
            }
        }
        else if (at(app, kBehind) != 0)
        {
            fld(cpu, f32(app, cpu.edx + 0x7c4));
            fmul(cpu, f64(app, cpu.edx == at(app, 0x56ef54) ? 0x53609c : 0x5360a4));
            fstp(app, cpu, cpu.ecx + 0x7b0);
            fld(cpu, f32(app, cpu.ecx + 0x7c4));
            if (fcomp(cpu, f64(app, 0x5360ac)).below())
            {
                cpu.ebx = at(app, kGap);
                cpu.eax = sreg32(cpu.ebx) > 0 ? cpu.ebx : neg(cpu.ebx);
                if (sreg32(cpu.eax) > 7)
                {
                    cpu.eax = cpu.ecx;
                    cpu.ebx = 0;
                    cpu.edx = 0;
                    if (!byAddress(app, cpu, kSetTimer))
                        return;
                    put(app, cpu.ecx + 0x7b0, 0xc11d5ac4);
                }
            }
        }
        else
        {
            if (!direct(app, cpu, copTopSpeed))
                return;
            cpu.esi = at(app, kGap);
            fstp(app, cpu, cpu.ebp - 4);
            bool top = sreg32(cpu.esi) > 8;
            if (!top)
            {
                cpu.edx = at(app, cpu.ecx + 0x624);
                fld(cpu, f32(app, cpu.edx + 0x7c4));
                top = !fcomp(cpu, f32(app, cpu.ebp - 4)).belowOrEqual();
            }
            if (top)
            {
                cpu.eax = at(app, cpu.ebp - 4);
                put(app, cpu.ecx + 0x7b0, cpu.eax);
            }
            else
            {
                fld(cpu, f32(app, cpu.ebp - 4));
                fsub(cpu, f32(app, cpu.edx + 0x7c4));
                fild(app, cpu, kGap);
                fmulp(cpu, 1);
                fmul(cpu, f32(app, 0x5360b4));
                cpu.eax = at(app, cpu.ecx + 0x624);
                fstp(app, cpu, cpu.ecx + 0x7b0);
                fld(cpu, f32(app, cpu.eax + 0x7c4));
                fadd(cpu, f64(app, 0x5360bc));
                fadd(cpu, f32(app, cpu.ecx + 0x7b0));
                fstp(app, cpu, cpu.ecx + 0x7b0);
            }
        }
        flipWanted(app, cpu, cpu.ecx, cpu.ah, cpu.dh);
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4025c0: cop eax after the car it chases: too far behind (more than the
 * car's tickets allow, 0x5521ac or for a fast cop 0x5521b0) or the car out
 * of the race, it gives up (giveUpPursuit), a timer (0x405fd0), its side
 * flipped and the patrol's speed (setWantedSpeed 1.19, 0); pulled over or
 * within 15 nodes, the chase's speed (chaseSpeed); ahead of the car, onto
 * the car's side and the patrol's speed; otherwise its top speed and a
 * timer -- in the race's slow mode (0x6fd4c8) 0x5360c4 of it for a player's
 * car not braking (+0x660). */
void pursueTarget(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 8;
    const auto leave = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    // push 0 (or the push before) ...; mov eax, ecx; push 0x3f9851ec; call 0x401430
    const auto patrolSpeed = [&] {
        cpu.eax = cpu.ecx;
        push(app, cpu, 0x3f9851ec);
        if (!direct(app, cpu, setWantedSpeed))
            return;
        leave();
    };
    cpu.ecx = cpu.eax;
    cpu.eax = at(app, cpu.eax + 0x624);
    if (cpu.eax == 0)
    {
        push(app, cpu, 0);
        return patrolSpeed();
    }
    cpu.edx = at(app, kGap);
    cpu.ebx = at(app, cpu.ecx + 0x628);
    if (!(sreg32(cpu.edx) > 0))
        cpu.edx = neg(cpu.edx);
    bool giveUp = (byte(app, cpu.eax + 0x200) & 1) && at(app, cpu.eax + 0x2c4) == 3;
    if (!giveUp)
    {
        if (byte(app, cpu.ecx + 0x711) & 1)
        {
            cpu.eax = at(app, cpu.eax + 0x2c8);
            cpu.eax <<= 4;
            cpu.eax = at(app, cpu.eax + 0x5521b0);
        }
        else
        {
            cpu.eax = at(app, cpu.eax + 0x2c8);
            cpu.eax <<= 4;
            cpu.eax = at(app, cpu.eax + kByTickets);
        }
        giveUp = cpu.edx > cpu.eax;
    }
    if (giveUp)
    {
        cpu.eax = cpu.ecx;
        cpu.edx = 1;
        if (!direct(app, cpu, giveUpPursuit))
            return;
        cpu.ebx = 0;
        cpu.eax = cpu.ecx;
        if (!byAddress(app, cpu, kSetTimer))
            return;
        cpu.ah = byte(app, cpu.ecx + 0x621);
        cpu.ah |= 0x10;
        push(app, cpu, 0);
        putByte(app, cpu.ecx + 0x621, cpu.ah);
        return patrolSpeed();
    }
    if (at(app, cpu.ebx + 0x1c) == 2 || !(sreg32(cpu.edx) > 0xf))
    {
        cpu.eax = cpu.ecx;
        if (!direct(app, cpu, chaseSpeed))
            return;
        return leave();
    }
    if (at(app, kBehind) != 0)
    {
        if (at(app, kSide) != 0)
            putByte(app, cpu.ecx + 0x621, byte(app, cpu.ecx + 0x621) | 0x10);
        else
            putByte(app, cpu.ecx + 0x621, byte(app, cpu.ecx + 0x621) & 0xef);
        push(app, cpu, 0);
        return patrolSpeed();
    }
    cpu.eax = cpu.ecx;
    cpu.edx = 1;
    if (!direct(app, cpu, copTopSpeed))
        return;
    cpu.ebx = 0;
    cpu.eax = cpu.ecx;
    fstp(app, cpu, cpu.ecx + 0x7b0);
    if (!byAddress(app, cpu, kSetTimer))
        return;
    flipWanted(app, cpu, cpu.ecx, cpu.bl, cpu.al);
    if (at(app, 0x6fd4c8) == 0)
        return leave();
    cpu.eax = at(app, cpu.ecx + 0x624);
    if ((byte(app, cpu.eax + 0x200) & 4) == 0)
        return leave();
    if (at(app, cpu.eax + 0x660) != 0)
        return leave();
    fld(cpu, f32(app, cpu.ecx + 0x7b0));
    fld(cpu, cpu.fpu.st(0));
    fmul(cpu, f64(app, 0x5360c4));
    fstpSt1(cpu);
    fstp(app, cpu, cpu.ecx + 0x7b0);
    leave();
}

/* 0x402750: cop eax's votes for a lane (0x56eed8.., 0x56ef2c..) in a chase:
 * slower than 0x5360cc and not told otherwise, marked (+0x711 bit 2);
 * behind its target, 10 for the target's lane; level with it, the mark;
 * going the other way than the target, 70 against its lane; far ahead (more
 * than 5 nodes) or the target pulled over, 10 for its lane; else, by the
 * target's lane against the cop's, 10 and 30 for its own and the next. */
void votePursuitLane(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    const auto leave = [&] {
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    const auto vote = [&](reg32 table, reg32 amount) {
        cpu.eax = at(app, kLaneVote);
        put(app, cpu.eax * 4 + table, at(app, cpu.eax * 4 + table) + amount);
    };
    cpu.edx = cpu.eax;
    cpu.ecx = at(app, cpu.eax + 0x624);
    if (cpu.ecx == 0)
        return leave();
    fld(cpu, f32(app, cpu.ecx + 0x7c4));
    cpu.ebx = at(app, cpu.eax + 0x628);
    if (fcomp(cpu, f64(app, 0x5360cc)).below() && at(app, cpu.edx + 0x7d4) == 0)
        putByte(app, cpu.edx + 0x711, byte(app, cpu.edx + 0x711) | 2);
    if (at(app, kBehind) != 0)
    {
        vote(0x56ef2c, 0xa);
        return leave();
    }
    if (at(app, kGap) == 0)
    {
        putByte(app, cpu.edx + 0x711, byte(app, cpu.edx + 0x711) | 2);
        return leave();
    }
    cpu.eax = at(app, cpu.edx + 0x624);
    fld(cpu, f32(app, cpu.eax + 0x7c0));
    fld(cpu, Float(0.0));
    cpu.ecx = fcompp(cpu).belowOrEqual() ? 0 : 1;
    fld(cpu, f32(app, cpu.edx + 0x7c0));
    fld(cpu, Float(0.0));
    cpu.eax = fcompp(cpu).belowOrEqual() ? 0 : 1;
    cpu.eax ^= cpu.ecx;
    if (cpu.eax != 0)
    {
        vote(0x56eed8, 0x46);
        return leave();
    }
    if (sreg32(at(app, kGap)) > 5 || at(app, cpu.ebx + 0x1c) == 2)
    {
        vote(0x56ef2c, 0xa);
        return leave();
    }
    cpu.eax = at(app, kLaneVote);
    if (cpu.eax == 0)
    {
        cpu.ebx = at(app, 0x56ef30);
        cpu.ecx = at(app, 0x56ef2c);
        cpu.ebx += 0x1e;
        cpu.ecx += 0xa;
        put(app, 0x56ef30, cpu.ebx);
        put(app, 0x56ef2c, cpu.ecx);
    }
    else if (cpu.eax == 1)
    {
        cpu.edi = at(app, 0x56ef30);
        cpu.eax = at(app, 0x56ef34);
        cpu.esi = at(app, 0x56ef2c);
        cpu.edi += 0xa;
        cpu.eax += 0x1e;
        cpu.esi += 0x1e;
        put(app, 0x56ef30, cpu.edi);
        put(app, 0x56ef34, cpu.eax);
        put(app, 0x56ef2c, cpu.esi);
    }
    else if (cpu.eax == 2)
    {
        cpu.ecx = at(app, 0x56ef34);
        cpu.edx = at(app, 0x56ef30);
        cpu.ecx += 0xa;
        cpu.edx += 0x1e;
        put(app, 0x56ef34, cpu.ecx);
        put(app, 0x56ef30, cpu.edx);
    }
    leave();
}

/* 0x4028e0: car eax onto the side of the road toward node edx (+0x621 bit
 * 0x10 set where the node is behind it). */
void sideTowardNode(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    cpu.eax = cpu.edx;
    cpu.edx = at(app, cpu.ecx + 0x1c);
    if (!byAddress(app, cpu, kNodeGap))
        return;
    if (sreg32(cpu.eax) >= 0)
        putByte(app, cpu.ecx + 0x621, byte(app, cpu.ecx + 0x621) & 0xef);
    else
        putByte(app, cpu.ecx + 0x621, byte(app, cpu.ecx + 0x621) | 0x10);
    cpu.ebp = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x402910: cop eax going the other way than the car it chases (both faster
 * than 0x5360d4): onto the car's side once it is 12 nodes past it. */
void keepSideOfTarget(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    const auto leave = [&] {
        cpu.ebp = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.ecx = cpu.eax;
    cpu.edx = at(app, cpu.eax + 0x624);
    if (cpu.edx == 0)
        return leave();
    fld(cpu, f32(app, cpu.edx + 0x7c4));
    if (fcomp(cpu, f64(app, 0x5360d4)).below())
        return leave();
    cpu.eax = at(app, cpu.ecx + 0x624);
    fld(cpu, f32(app, cpu.eax + 0x7c0));
    fld(cpu, Float(0.0));
    cpu.edx = fcompp(cpu).belowOrEqual() ? 0 : 1;
    fld(cpu, f32(app, cpu.ecx + 0x7c0));
    fld(cpu, Float(0.0));
    cpu.eax = fcompp(cpu).belowOrEqual() ? 0 : 1;
    cpu.eax ^= cpu.edx;
    if (cpu.eax == 0)
        return leave();
    cpu.eax = at(app, cpu.ecx + 0x624);
    cpu.edx = at(app, cpu.eax + 0x1c);
    cpu.eax = at(app, cpu.ecx + 0x1c);
    if (!byAddress(app, cpu, kNodeGap))
        return;
    cpu.ebx = at(app, kBackwards);
    cpu.edx = cpu.eax;
    if (cpu.ebx != 0)
        cpu.edx = neg(cpu.edx);
    if (at(app, kSide) != 0)
    {
        if (sreg32(cpu.edx) < 0xc)
            putByte(app, cpu.ecx + 0x621, byte(app, cpu.ecx + 0x621) & 0xef);
    }
    else if (sreg32(cpu.edx) > -0xc)
        putByte(app, cpu.ecx + 0x621, byte(app, cpu.ecx + 0x621) | 0x10);
    leave();
}

/* 0x4029c0: cop eax placed at the race's start: its side cleared, the last
 * player's car of the race (0x5efcc8, [0x5efd9c] of them) its watch
 * (+0x6d8), then placeCop. */
void placeCopAtStart(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    putByte(app, cpu.eax + 0x621, byte(app, cpu.eax + 0x621) & 0xef);
    cpu.edx = at(app, 0x5efd9c);
    for (;;)
    {
        --cpu.edx;
        if (sreg32(cpu.edx) < 0)
            break;
        cpu.ecx = at(app, cpu.edx * 4 + 0x5efcc8);
        if (byte(app, cpu.ecx + 0x200) & 4)
        {
            put(app, cpu.eax + 0x6d8, cpu.ecx);
            break;
        }
    }
    if (!direct(app, cpu, placeCop))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x402a00: cop eax put at its order's node (the first of them free,
 * 0x405c80, 20 nodes on each time, 0x40e160): its sideways place (+0x7b4) a
 * random share of the side's lanes (the node's +0x13) and width (+0x1b
 * right, +0x17 left, by 0x5360dc and 0x5360e4) -- the side the order
 * (0x56f230, 0x56f228) and the car's say -- plus 1; the patrol's speed
 * (setWantedSpeed 1.19, 0) its speed too, the chase off (0x40cd80), a random
 * radio level 0..2. */
void placeCop(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0xc;
    cpu.ecx = cpu.eax;
    cpu.eax = at(app, cpu.eax + 0x7ac);
    cpu.eax = at(app, cpu.eax);
    cpu.eax = reg32(sreg32(cpu.eax) >> 16);
    for (;;)
    {
        cpu.edx = cpu.eax;
        put(app, cpu.ecx + 0x1c, cpu.eax);
        cpu.eax = cpu.edx;
        if (!byAddress(app, cpu, kNodeBlocked))
            return;
        if (cpu.eax == 0)
            break;
        cpu.edx = 0x14;
        cpu.eax = cpu.ecx;
        if (!byAddress(app, cpu, kNodeOn))
            return;
    }
    if (at(app, 0x56f228) != 0)
        putByte(app, cpu.ecx + 0x621, byte(app, cpu.ecx + 0x621) & 0xef);
    cpu.eax = (byte(app, cpu.ecx + 0x621) & 0x10) == 0 ? 1 : 0;
    cpu.eax ^= at(app, kBackwards);
    bool right = cpu.eax != 0 && at(app, 0x56f230) == 1;
    if (!right)
    {
        cpu.eax = (byte(app, cpu.ecx + 0x621) & 0x10) != 0 ? 1 : 0;
        cpu.eax ^= at(app, kBackwards);
        right = cpu.eax != 0 && at(app, 0x56f230) == 0xffffffff;
    }
    if (right)
    {
        cpu.eax = cpu.edx * 8;
        cpu.edx += cpu.eax;
        cpu.eax = at(app, kNodes);
        cpu.edx <<= 2;
        cpu.edx += cpu.eax;
        cpu.eax = 0;
        cpu.al = byte(app, cpu.edx + 0x1b);
        cpu.eax <<= 15;
        cpu.edi = at(app, kMultiplier);
        put(app, cpu.ebp - 4, cpu.eax);
        cpu.eax = at(app, kSeed);
        cpu.eax = imul(cpu.eax, cpu.edi);
        fild(app, cpu, cpu.ebp - 4);
        fmul(cpu, f64(app, 0x5360dc));
        put(app, kProduct, cpu.eax);
        cpu.eax &= 0xffff;
        cpu.ebx = at(app, kProduct);
        put(app, kSeed, cpu.eax);
        cpu.ebx &= 0xffff00;
        cpu.al = byte(app, cpu.edx + 0x13);
        cpu.ebx >>= 8;
        cpu.al &= 0xf;
        cpu.eax &= 0xff;
        cpu.eax = imul(cpu.eax, cpu.ebx);
        cpu.eax >>= 16;
        ++cpu.eax;
        put(app, cpu.ebp - 4, cpu.eax);
        fild(app, cpu, cpu.ebp - 4);
        fmulSt1(cpu);
        fxch(cpu, 1);
        fmul(cpu, f64(app, 0x5360e4));
        fsubp(cpu, 1);
    }
    else
    {
        cpu.eax = cpu.edx * 8;
        cpu.eax += cpu.edx;
        cpu.edx = at(app, kNodes);
        cpu.eax <<= 2;
        cpu.edx += cpu.eax;
        cpu.eax = 0;
        cpu.al = byte(app, cpu.edx + 0x17);
        cpu.eax <<= 15;
        cpu.ebx = at(app, kMultiplier);
        put(app, cpu.ebp - 4, cpu.eax);
        cpu.eax = at(app, kSeed);
        cpu.eax = imul(cpu.eax, cpu.ebx);
        fild(app, cpu, cpu.ebp - 4);
        fmul(cpu, f64(app, 0x5360dc));
        put(app, kProduct, cpu.eax);
        cpu.eax &= 0xffff;
        put(app, kSeed, cpu.eax);
        cpu.eax = at(app, kProduct);
        cpu.eax &= 0xffff00;
        cpu.dl = byte(app, cpu.edx + 0x13);
        cpu.eax >>= 8;
        cpu.edx &= 0xff;
        cpu.edx = reg32(sreg32(cpu.edx) >> 4);
        cpu.edx &= 0xf;
        cpu.eax = imul(cpu.eax, cpu.edx);
        cpu.eax >>= 16;
        ++cpu.eax;
        fld(cpu, cpu.fpu.st(0));
        put(app, cpu.ebp - 4, cpu.eax);
        fchs(cpu);
        fild(app, cpu, cpu.ebp - 4);
        fmulp(cpu, 1);
        fxch(cpu, 1);
        fmul(cpu, f64(app, 0x5360e4));
        faddp(cpu, 1);
    }
    fstp(app, cpu, cpu.ecx + 0x7b4);
    push(app, cpu, 0);
    put(app, cpu.ecx + 0x654, 0x3f800000);
    push(app, cpu, 0x3f9851ec);
    fld(cpu, f32(app, cpu.ecx + 0x654));
    fadd(cpu, f32(app, cpu.ecx + 0x7b4));
    cpu.eax = cpu.ecx;
    fstp(app, cpu, cpu.ecx + 0x7b4);
    if (!direct(app, cpu, setWantedSpeed))
        return;
    fld(cpu, Float(0.0));
    cpu.eax = at(app, cpu.ecx + 0x7b0);
    put(app, cpu.ecx + 0x7c0, cpu.eax);
    if (!fcomp(cpu, f32(app, cpu.ecx + 0x7c0)).below())
    {
        fld(cpu, f32(app, cpu.ecx + 0x7c0));
        fchs(cpu);
        fstp(app, cpu, cpu.ebp - 8);
    }
    else
    {
        cpu.eax = at(app, cpu.ecx + 0x7c0);
        put(app, cpu.ebp - 8, cpu.eax);
    }
    cpu.eax = at(app, cpu.ebp - 8);
    put(app, cpu.ecx + 0x7c4, cpu.eax);
    cpu.al = byte(app, cpu.ecx + 0x710);
    put(app, cpu.ecx + 0x624, 0);
    cpu.al &= 0xef;
    putByte(app, cpu.ecx + 0x710, cpu.al);
    cpu.eax = cpu.ecx;
    put(app, cpu.ecx + 0x734, 0);
    if (!byAddress(app, cpu, kStopped))
        return;
    cpu.eax = at(app, kSeed);
    cpu.esi = at(app, kMultiplier);
    cpu.eax = imul(cpu.eax, cpu.esi);
    put(app, kProduct, cpu.eax);
    cpu.edx = at(app, kProduct);
    cpu.edx &= 0xffff00;
    cpu.eax &= 0xffff;
    cpu.edx >>= 8;
    put(app, kSeed, cpu.eax);
    cpu.eax = cpu.edx * 4;
    cpu.eax -= cpu.edx;
    cpu.eax >>= 16;
    put(app, cpu.ecx + 0x71c, cpu.eax);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x402c70: 1. */
void alwaysOne(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.eax = 1;
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x402c80: whether car eax goes backwards (+0x10c below 0) on the ground
 * (+0x164 0). */
void carReversing(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = cpu.eax;
    fld(cpu, Float(0.0));
    if (!fcomp(cpu, f32(app, cpu.eax + 0x10c)).belowOrEqual() && word(app, cpu.edx + 0x164) == 0)
        cpu.eax = 1;
    else
        cpu.eax = 0;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x402cb0: cop eax (not a player's) hit by racer edx: if the racer is no
 * slower than 0x5360ec of the cop (or the cop near still) and may be chased,
 * a cop on another player's car -- not one with its target pulled over that
 * is parked (+0x70c 6) or alone -- gives that one up for this one; a free
 * cop takes it on where it wants to (copsWanted); each with its radio level
 * up and joinPursuit kind 1. */
void copHitByCar(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    const auto leave = [&] {
        cpu.ebp = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.ecx = cpu.eax;
    cpu.ebx = cpu.edx;
    if ((byte(app, cpu.edx + 0x200) & 1) == 0 || (byte(app, cpu.ecx + 0x200) & 4))
        return leave();
    fld(cpu, f32(app, cpu.ecx + 0x7c4));
    fld(cpu, Float(1.0));
    if (fcompp(cpu).belowOrEqual())
    {
        fld(cpu, f32(app, cpu.ebx + 0x7c4));
        fdiv(cpu, f32(app, cpu.ecx + 0x7c4));
        if (fcomp(cpu, f64(app, 0x5360ec)).belowOrEqual())
            return leave();
    }
    cpu.eax = at(app, kClock);
    if (sreg32(cpu.eax) <= sreg32(at(app, cpu.ebx + 0x634)))
        return leave();
    if (at(app, cpu.ecx + 0x624) != 0)
    {
        cpu.eax = at(app, cpu.ecx + 0x628);
        if (at(app, cpu.eax + 0x1c) == 2)
        {
            if (at(app, cpu.ecx + 0x70c) == 6)
                return leave();
            if (at(app, cpu.eax + 0x2c) == 0)
                return leave();
        }
        if (cpu.ebx == at(app, cpu.ecx + 0x624))
            return leave();
        if ((byte(app, cpu.ebx + 0x200) & 4) == 0)
            return leave();
        cpu.eax = cpu.ecx;
        if (!direct(app, cpu, giveUpPursuit))
            return;
        cpu.eax = cpu.ecx;
        cpu.edx = cpu.ecx;
        if (!direct(app, cpu, radioLevelUp))
            return;
        cpu.eax = 1;
        push(app, cpu, 0);
        if (!direct(app, cpu, joinPursuit))
            return;
        return leave();
    }
    cpu.edx = cpu.ebx;
    cpu.eax = cpu.ecx;
    if (!direct(app, cpu, copsWanted))
        return;
    if (cpu.eax == 0)
        return leave();
    cpu.eax = cpu.ecx;
    if (!direct(app, cpu, radioLevelUp))
        return;
    cpu.edx = cpu.ecx;
    cpu.eax = 1;
    push(app, cpu, 0);
    if (!direct(app, cpu, joinPursuit))
        return;
    leave();
}

/* 0x402d90: car eax's lane votes against the road's edge: 100 against
 * (0x56eeb8) a lane off the node's lanes; else 100 for the lane to the left
 * (0x56eeb4, a lane below 7) or the right (0x56eebc). */
void voteRoadLane(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = cpu.eax;
    cpu.ecx = at(app, cpu.eax + 0x1c);
    cpu.eax = cpu.ecx * 8;
    cpu.eax += cpu.ecx;
    cpu.ecx = at(app, kNodes);
    cpu.eax <<= 2;
    cpu.eax += cpu.ecx;
    cpu.ecx = 0;
    cpu.cl = byte(app, cpu.eax + 0x13);
    cpu.ecx = reg32(sreg32(cpu.ecx) >> 4);
    cpu.ebx = 6;
    cpu.ecx &= 0xf;
    cpu.ebx -= cpu.ecx;
    cpu.ecx = cpu.ebx;
    cpu.ebx = at(app, cpu.edx + 0x638);
    bool off = sreg32(cpu.ecx) > sreg32(cpu.ebx);
    if (!off)
    {
        cpu.al = byte(app, cpu.eax + 0x13);
        cpu.al &= 0xf;
        cpu.eax &= 0xff;
        cpu.eax += 7;
        off = !(sreg32(cpu.eax) >= sreg32(cpu.ebx));
    }
    if (off)
        put(app, 0x56eeb8, at(app, 0x56eeb8) + 0x64);
    else if (sreg32(cpu.ebx) < 7)
        put(app, 0x56eeb4, at(app, 0x56eeb4) + 0x64);
    else
        put(app, 0x56eebc, at(app, 0x56eebc) + 0x64);
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x402e10: cop eax looks for a speeder: of the racers that may be chased
 * and are in the race, within 15 nodes, each's speeding noted
 * (noteSpeeding), the one the most cops are wanted for (copsWanted); the
 * cop joins its pursuit (joinPursuit kind 0) right away where its siren is
 * on or it was told to wait (+0x711 bit 2) -- unless it has waited less
 * than 0x3c0 ticks (+0x72c) and the car is its order's, within 5 nodes, and
 * has not been 0x100 ticks there (+0x728).  No one found, both waits off. */
void spotSpeeder(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x10;
    const auto leave = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.ecx = cpu.eax;
    cpu.eax = at(app, cpu.eax + 0x1c);
    cpu.esi = 0;
    cpu.edi = 0;
    put(app, cpu.ebp - 8, cpu.esi);
    put(app, cpu.ebp - 0xc, cpu.eax);
    for (; sreg32(cpu.edi) < sreg32(at(app, kRacerCount)); ++cpu.edi)
    {
        cpu.ebx = at(app, cpu.edi * 4 + kRacers);
        cpu.eax = at(app, kClock);
        if (sreg32(cpu.eax) <= sreg32(at(app, cpu.ebx + 0x634)))
            continue;
        if ((byte(app, cpu.ebx + 0x200) & 1) && at(app, cpu.ebx + 0x2c4) == 3)
            continue;
        if ((byte(app, cpu.ebx + 0x200) & 1) && at(app, cpu.ebx + 0x2c4) == 4)
            continue;
        cpu.edx = at(app, cpu.ebp - 0xc);
        cpu.eax = at(app, cpu.ebx + 0x1c);
        if (!byAddress(app, cpu, kNodeGap))
            return;
        put(app, cpu.ebp - 0x10, cpu.eax);
        if (!(sreg32(cpu.eax) > 0))
            cpu.eax = neg(cpu.eax);
        if (sreg32(cpu.eax) > 0xf)
            continue;
        cpu.eax = cpu.ebx;
        if (!direct(app, cpu, noteSpeeding))
            return;
        cpu.edx = cpu.ebx;
        cpu.eax = cpu.ecx;
        if (!direct(app, cpu, copsWanted))
            return;
        if (sreg32(cpu.eax) <= sreg32(at(app, cpu.ebp - 8)))
            continue;
        put(app, cpu.ebp - 8, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 0x10);
        cpu.esi = cpu.ebx;
        put(app, cpu.ebp - 4, cpu.eax);
    }
    if (cpu.esi == 0)
    {
        put(app, cpu.ecx + 0x728, cpu.esi);
        put(app, cpu.ecx + 0x72c, cpu.esi);
        return leave();
    }
    cpu.ebx = at(app, cpu.ecx + 0x714);
    cpu.eax = cpu.ebx * 4;
    cpu.eax += cpu.ebx;
    cpu.eax <<= 4;
    bool join = true;
    if (at(app, cpu.eax + kSirens) != 0 || (byte(app, cpu.ecx + 0x711) & 4))
    {
        if (at(app, cpu.ecx + 0x72c) == 0)
        {
            cpu.eax = at(app, kClock);
            put(app, cpu.ecx + 0x72c, cpu.eax);
        }
        cpu.eax = at(app, kClock);
        cpu.eax -= at(app, cpu.ecx + 0x72c);
        if (!(sreg32(cpu.eax) > 0x3c0))
        {
            if (cpu.esi != at(app, cpu.ecx + 0x730))
                return leave();
            cpu.edi = at(app, cpu.ebp - 4);
            cpu.eax = sreg32(cpu.edi) > 0 ? cpu.edi : neg(cpu.edi);
            if (sreg32(cpu.eax) >= 5)
                return leave();
            if (at(app, cpu.ecx + 0x728) == 0)
            {
                cpu.eax = at(app, kClock);
                put(app, cpu.ecx + 0x728, cpu.eax);
            }
            cpu.eax = at(app, kClock);
            cpu.eax -= at(app, cpu.ecx + 0x728);
            join = sreg32(cpu.eax) > 0x100;
        }
    }
    if (join)
    {
        cpu.ebx = cpu.esi;
        cpu.edx = cpu.ecx;
        cpu.eax = 0;
        push(app, cpu, 0);
        if (!direct(app, cpu, joinPursuit))
            return;
    }
    leave();
}

/* 0x402f70: whether car eax's lane is off its node's lanes. */
void laneOffRoad(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = cpu.eax;
    cpu.ecx = at(app, cpu.eax + 0x1c);
    cpu.eax = cpu.ecx * 8;
    cpu.eax += cpu.ecx;
    cpu.ecx = at(app, kNodes);
    cpu.eax <<= 2;
    cpu.eax += cpu.ecx;
    cpu.ecx = 0;
    cpu.cl = byte(app, cpu.eax + 0x13);
    cpu.ecx = reg32(sreg32(cpu.ecx) >> 4);
    cpu.ebx = 7;
    cpu.ecx &= 0xf;
    cpu.ebx -= cpu.ecx;
    cpu.ecx = cpu.ebx;
    cpu.ebx = at(app, cpu.edx + 0x638);
    bool off = sreg32(cpu.ecx) > sreg32(cpu.ebx);
    if (!off)
    {
        cpu.al = byte(app, cpu.eax + 0x13);
        cpu.al &= 0xf;
        cpu.eax &= 0xff;
        cpu.eax += 6;
        off = sreg32(cpu.eax) < sreg32(cpu.ebx);
    }
    cpu.eax = off ? 1 : 0;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x402fd0: whether car eax is at its order's node (0x40f000, the order's
 * node and second word). */
void atOrderNode(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = at(app, cpu.eax + 0x7ac);
    cpu.ebx = at(app, cpu.edx + 2);
    cpu.eax = at(app, cpu.eax + 0x1c);
    cpu.edx = at(app, cpu.edx);
    cpu.ebx = reg32(sreg32(cpu.ebx) >> 16);
    cpu.edx = reg32(sreg32(cpu.edx) >> 16);
    if (!byAddress(app, cpu, kAtNode))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x403000: car eax onto the side of the road toward its order's node. */
void sideOfOrder(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = at(app, cpu.eax + 0x7ac);
    cpu.edx = at(app, cpu.edx);
    cpu.edx = reg32(sreg32(cpu.edx) >> 16);
    if (!direct(app, cpu, sideTowardNode))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x403030: cop eax takes its order (its kind back in eax), by the table at
 * 0x403018: 0 to its node (state 3 there, else on toward it, state 1); 1
 * state 2; 3 a roadblock at its node (state 7, the roadblock's radio
 * 0x40b7f0); 4 to its node for a second car (state 8; 10 once there); any
 * other its side by the order's (+6), state 1. */
void takeOrder(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    const auto leave = [&] {
        cpu.eax = cpu.ecx;
        cpu.ebp = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.edx = cpu.eax;
    cpu.eax = at(app, cpu.eax + 0x7ac);
    cpu.ecx = 0;
    cpu.cl = byte(app, cpu.eax);
    // jmp [ecx*4 + 0x403018]: 0x403075, 0x4030a9, 0x40304e, 0x4030ba, 0x40310e.
    switch (cpu.ecx)
    {
    case 0:
        cpu.eax = cpu.edx;
        if (!direct(app, cpu, atOrderNode))
            return;
        if (cpu.eax == 0)
        {
            cpu.eax = cpu.edx;
            if (!direct(app, cpu, sideOfOrder))
                return;
            put(app, cpu.edx + 0x70c, 1);
            return leave();
        }
        put(app, cpu.edx + 0x70c, 3);
        return leave();
    case 1:
        put(app, cpu.edx + 0x70c, 2);
        return leave();
    case 3:
        cpu.eax = cpu.edx;
        if (!direct(app, cpu, atOrderNode))
            return;
        if (cpu.eax != 0)
            return leave();
        cpu.eax = cpu.edx;
        if (!direct(app, cpu, sideOfOrder))
            return;
        if (at(app, cpu.edx + 0x70c) == 7)
            return leave();
        cpu.ah = byte(app, cpu.edx + 0x710);
        put(app, cpu.edx + 0x70c, 7);
        cpu.ah |= 0x10;
        putByte(app, cpu.edx + 0x710, cpu.ah);
        cpu.eax = cpu.edx;
        put(app, cpu.edx + 0x734, 1);
        if (!byAddress(app, cpu, kRadioRoadblockUp))
            return;
        return leave();
    case 4:
        cpu.eax = cpu.edx;
        if (!direct(app, cpu, atOrderNode))
            return;
        if (cpu.eax != 0)
        {
            put(app, cpu.edx + 0x734, 1);
            cpu.bh = byte(app, cpu.edx + 0x710);
            put(app, cpu.edx + 0x70c, 0xa);
            cpu.bh |= 0x10;
            putByte(app, cpu.edx + 0x710, cpu.bh);
            return leave();
        }
        cpu.eax = cpu.edx;
        if (!direct(app, cpu, sideOfOrder))
            return;
        if (at(app, cpu.edx + 0x70c) == 8)
            return leave();
        put(app, cpu.edx + 0x734, 1);
        cpu.bl = byte(app, cpu.edx + 0x710);
        put(app, cpu.edx + 0x70c, 8);
        cpu.bl |= 0x10;
        putByte(app, cpu.edx + 0x710, cpu.bl);
        return leave();
    default:
        if (byte(app, cpu.eax + 6) == 0)
            putByte(app, cpu.edx + 0x621, byte(app, cpu.edx + 0x621) & 0xef);
        else
            putByte(app, cpu.edx + 0x621, byte(app, cpu.edx + 0x621) | 0x10);
        put(app, cpu.edx + 0x70c, 1);
        return leave();
    }
}

/* 0x403180: cop eax parks at its roadblock, unless a racer is within
 * 0x5360f4: put down there (0x4a0980), turned across the road (a quarter or
 * three quarters of a turn by the order's +7, ea::yRotation into its
 * orientation by 0x4e0280), settled (0x4a2690), its wanted speed 0, its mark
 * of the roadblock (+0x621 bit 8) off, stopped (0x40ce70). */
void parkCop(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x28;
    const auto leave = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.ecx = cpu.eax;
    for (cpu.esi = 0; sreg32(cpu.esi) < sreg32(at(app, kRacerCount)); ++cpu.esi)
    {
        cpu.edi = cpu.esi * 4;
        cpu.eax = at(app, cpu.edi + kRacers);
        cpu.ebx = cpu.ecx + 0x98;
        cpu.edx = cpu.eax + 0x98;
        cpu.eax = cpu.ebx;
        if (!absoluteDistance(app, cpu, [&] {
                cpu.eax = at(app, cpu.edi + kRacers);
                cpu.edx = cpu.eax + 0x98;
                cpu.eax = cpu.ebx;
            }))
            return;
        fstp(app, cpu, cpu.ebp - 4);
        fld(cpu, f32(app, cpu.ebp - 4));
        if (fcomp(cpu, f64(app, 0x5360f4)).below())
            return leave();
    }
    cpu.edx = at(app, kBackwards);
    cpu.eax = cpu.ecx;
    if (!byAddress(app, cpu, kParkCar))
        return;
    cpu.eax = at(app, cpu.ecx + 0x7ac);
    cpu.eax = cpu.ebp - 0x28;
    push(app, cpu, byte(app, at(app, cpu.ecx + 0x7ac) + 7) != 0 ? 0x3e800000 : 0x3f400000);
    if (!byAddress(app, cpu, kYRotation))
        return;
    cpu.eax = cpu.ecx + 0xc0;
    cpu.edx = cpu.ebp - 0x28;
    cpu.ebx = cpu.eax;
    if (!byAddress(app, cpu, kMultiply))
        return;
    cpu.eax = cpu.ecx;
    if (!byAddress(app, cpu, kCarSettled))
        return;
    cpu.dh = byte(app, cpu.ecx + 0x621);
    put(app, cpu.ecx + 0x7b0, 0);
    cpu.dh &= 0xf7;
    cpu.eax = cpu.ecx;
    putByte(app, cpu.ecx + 0x621, cpu.dh);
    if (!byAddress(app, cpu, kStop))
        return;
    leave();
}

/* 0x403270: car eax on patrol: steered at its target (0x40dd60), its wanted
 * speed by the two arguments (setWantedSpeed), clear of the others
 * (0x40bff0), its lanes (0x40ddb0), steered (0x40de30).  ret 8. */
void drivePatrol(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = cpu.eax;
    if (!byAddress(app, cpu, kSteerTarget))
        return;
    push(app, cpu, at(app, cpu.ebp + 0x10));
    cpu.eax = cpu.edx;
    push(app, cpu, at(app, cpu.ebp + 0xc));
    if (!direct(app, cpu, setWantedSpeed))
        return;
    cpu.eax = cpu.edx;
    if (!byAddress(app, cpu, kAvoid))
        return;
    cpu.eax = cpu.edx;
    if (!byAddress(app, cpu, kSteerLanes))
        return;
    cpu.eax = cpu.edx;
    if (!byAddress(app, cpu, kSteer))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4 + 8;
}

/* 0x4032e0: a cop's driver, each tick, where its driver is ready (0x40df10):
 * the tick's sizing up cleared (0x56ea58..), its slow mark off; backwards on
 * the ground, the radio's word of the stop (radioStop), else that word's
 * mark off; its roadblock watched where its siren is on (watchRoadblock) and
 * its second car's where it waits (watchSecondCar); a new order taken
 * (takeOrder).  Then by its state (+0x70c, the table at 0x4032a4): 1
 * patrol, chasing whoever it spots (spotSpeeder; state 5 once it chases,
 * else the order again); 7 and 8 to the order's node (to 1, or 9, where it
 * is there); 9 at the node, slowly (10 once stopped, the spikes' radio); 10
 * parked at a roadblock (parkCop; 11 when it cannot, else steering on); 11
 * stopped; 2 patrol to the order's node; 3 driving the road's lanes (4 off
 * the road, the roadblock's siren); 4 stopped; 5 the chase (sizeUpTarget,
 * keepSideOfTarget, pursueTarget, the target's speeding, votePursuitLane,
 * targetStopping -- state 6 and copStopsBy once it has the car; the stop's
 * word, or now and then backup, a roadblock or a second car on the radio);
 * 6 at the stop (stopOver; 1 after); any other 1.  Chasing someone, every
 * patrolling state goes to 5. */
void copDriver(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    const auto leave = [&] {
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    const auto call = [&](void (*function)(App*, CPU&)) {
        cpu.eax = cpu.ecx;
        return direct(app, cpu, function);
    };
    const auto callAt = [&](reg32 target) {
        cpu.eax = cpu.ecx;
        return byAddress(app, cpu, target);
    };
    // push second; mov eax, ecx; push first; call 0x403270
    const auto patrol = [&](reg32 first, reg32 second) {
        push(app, cpu, second);
        cpu.eax = cpu.ecx;
        push(app, cpu, first);
        return direct(app, cpu, drivePatrol);
    };
    const auto chasing = [&] { return at(app, cpu.ecx + 0x624) != 0; };
    cpu.ecx = cpu.eax;
    if (!byAddress(app, cpu, kDriverReady))
        return;
    if (cpu.eax == 0)
        return leave();
    cpu.ebx = 0x14;
    cpu.eax = kTurning;
    cpu.edx = 0;
    if (!byAddress(app, cpu, kMemset))
        return;
    putByte(app, cpu.ecx + 0x711, byte(app, cpu.ecx + 0x711) & 0xfd);
    if (!call(carReversing))
        return;
    if (cpu.eax != 0)
    {
        if (!call(radioStop))
            return;
    }
    else
        putByte(app, cpu.ecx + 0x711, byte(app, cpu.ecx + 0x711) & 0xef);
    cpu.edx = at(app, cpu.ecx + 0x714);
    cpu.eax = cpu.edx * 4;
    cpu.eax += cpu.edx;
    cpu.eax <<= 4;
    if (at(app, cpu.eax + kSirens) != 0)
    {
        if (!call(watchRoadblock))
            return;
    }
    if (byte(app, cpu.ecx + 0x711) & 4)
    {
        if (!call(watchSecondCar))
            return;
    }
    if (!call(orderTaken))
        return;
    if (cpu.eax != 0)
    {
        if (!call(takeOrder))
            return;
    }
    cpu.eax = at(app, cpu.ecx + 0x70c);
    // jmp [eax*4 + 0x4032a4]: 0x403380, 0x40338a, 0x4035f1, 0x403654, 0x4036d1,
    // 0x403707, 0x4037f4, 0x4033c5, 0x40343d, 0x403496, 0x40350b, 0x4035a8.
    switch (cpu.eax > 0xb ? 0 : cpu.eax)
    {
    case 0:
        put(app, cpu.ecx + 0x70c, 1);
        [[fallthrough]];
    case 1:
        if (!patrol(0x3f9851ec, 0) || !call(spotSpeeder))
            return;
        if (chasing())
        {
            put(app, cpu.ecx + 0x70c, 5);
            return leave();
        }
        if (!call(takeOrder))
            return;
        return leave();
    case 7:
        if (!patrol(0x40000000, 0) || !call(spotSpeeder))
            return;
        cpu.esi = at(app, cpu.ecx + 0x624);
        if (cpu.esi != 0)
        {
            put(app, cpu.ecx + 0x70c, 5);
            return leave();
        }
        if (!call(atOrderNode))
            return;
        if (cpu.eax == 0)
        {
            if (!call(sideOfOrder))
                return;
            return leave();
        }
        cpu.eax = at(app, cpu.ecx + 0x7ac);
        putByte(app, cpu.eax, 2);
        put(app, cpu.ecx + 0x70c, 1);
        cpu.bl = byte(app, cpu.ecx + 0x710);
        put(app, cpu.ecx + 0x734, cpu.esi);
        cpu.bl &= 0xef;
        putByte(app, cpu.ecx + 0x710, cpu.bl);
        return leave();
    case 8:
        if (!patrol(0x40000000, 0) || !call(spotSpeeder))
            return;
        if (chasing())
        {
            put(app, cpu.ecx + 0x70c, 5);
            return leave();
        }
        if (!call(atOrderNode))
            return;
        if (cpu.eax != 0)
        {
            put(app, cpu.ecx + 0x70c, 9);
            return leave();
        }
        if (!call(sideOfOrder))
            return;
        return leave();
    case 9:
        if (!patrol(0, 0x40c00000) || !call(spotSpeeder))
            return;
        if (chasing())
        {
            put(app, cpu.ecx + 0x70c, 5);
            return leave();
        }
        if (!call(atOrderNode))
            return;
        if (cpu.eax == 0)
        {
            if (!call(sideOfOrder))
                return;
            return leave();
        }
        fld(cpu, f32(app, cpu.ecx + 0x7c4));
        if (!fcomp(cpu, f64(app, 0x5360fc)).belowOrEqual())
            return leave();
        cpu.eax = cpu.ecx;
        put(app, cpu.ecx + 0x70c, 0xa);
        if (!byAddress(app, cpu, kRadioSpikes))
            return;
        return leave();
    case 10:
        if (!call(atOrderNode))
            return;
        if (cpu.eax != 0)
        {
            cpu.bh = byte(app, cpu.ecx + 0x621);
            cpu.bh |= 8;
            cpu.eax = cpu.ecx;
            putByte(app, cpu.ecx + 0x621, cpu.bh);
            if (!direct(app, cpu, parkCop))
                return;
            if (byte(app, cpu.ecx + 0x621) & 8)
            {
                if (!callAt(kSteerTarget) || !callAt(kSteerLanes) || !callAt(kSteer))
                    return;
            }
            else
                put(app, cpu.ecx + 0x70c, 0xb);
        }
        else
        {
            putByte(app, cpu.ecx + 0x621, byte(app, cpu.ecx + 0x621) & 0xf7);
            if (!call(sideOfOrder))
                return;
            if (!patrol(0, 0x40c00000))
                return;
        }
        if (!call(spotSpeeder))
            return;
        if (!chasing())
            return leave();
        cpu.dl = byte(app, cpu.ecx + 0x621);
        put(app, cpu.ecx + 0x70c, 5);
        cpu.dl &= 0xf7;
        putByte(app, cpu.ecx + 0x621, cpu.dl);
        return leave();
    case 11:
        if (!call(spotSpeeder) || !callAt(kStop) || !callAt(kStop) || !callAt(kStop))
            return;
        if (!chasing())
            return leave();
        cpu.dh = byte(app, cpu.ecx + 0x621);
        put(app, cpu.ecx + 0x70c, 5);
        cpu.dh &= 0xf7;
        putByte(app, cpu.ecx + 0x621, cpu.dh);
        return leave();
    case 2:
        if (!patrol(0x3f9851ec, 0) || !call(spotSpeeder))
            return;
        if (chasing())
            put(app, cpu.ecx + 0x70c, 5);
        if (!call(atOrderNode))
            return;
        if (cpu.eax != 0)
        {
            putByte(app, cpu.ecx + 0x710, byte(app, cpu.ecx + 0x710) & 0xdf);
            return leave();
        }
        if (byte(app, cpu.ecx + 0x710) & 0x20)
            return leave();
        if (!call(sideOfOrder))
            return;
        putByte(app, cpu.ecx + 0x710, byte(app, cpu.ecx + 0x710) | 0x20);
        return leave();
    case 3:
        if (!callAt(kSteerTarget))
            return;
        push(app, cpu, 0x40d00000);
        cpu.eax = cpu.ecx;
        push(app, cpu, 0);
        if (!direct(app, cpu, setWantedSpeed))
            return;
        if (!call(voteRoadLane) || !callAt(kLaneOccupied) || !callAt(kLaneAhead) || !callAt(kLaneEdges)
            || !callAt(kSteer) || !call(laneOffRoad))
            return;
        if (cpu.eax != 0)
        {
            cpu.ah = byte(app, cpu.ecx + 0x711);
            put(app, cpu.ecx + 0x70c, 4);
            if (cpu.ah & 8)
            {
                if (!call(roadblockSiren))
                    return;
            }
        }
        if (!chasing())
            return leave();
        put(app, cpu.ecx + 0x70c, 5);
        return leave();
    case 4:
        cpu.eax = cpu.ecx;
        put(app, cpu.ecx + 0x7b0, 0);
        if (!byAddress(app, cpu, kStop) || !call(spotSpeeder))
            return;
        if (!chasing())
            return leave();
        put(app, cpu.ecx + 0x70c, 5);
        return leave();
    case 5:
    {
        if (!callAt(kSteerTarget) || !call(sizeUpTarget) || !call(keepSideOfTarget) || !call(pursueTarget))
            return;
        cpu.eax = at(app, cpu.ecx + 0x624);
        if (!direct(app, cpu, noteSpeeding))
            return;
        if (!callAt(kSteerLanes) || !call(votePursuitLane) || !callAt(kSteer) || !call(targetStopping))
            return;
        if (cpu.eax != 0)
        {
            cpu.eax = cpu.ecx;
            put(app, cpu.ecx + 0x70c, 6);
            if (!direct(app, cpu, copStopsBy))
                return;
            return leave();
        }
        cpu.edx = at(app, cpu.ecx + 0x624);
        if (cpu.edx == 0)
        {
            put(app, cpu.ecx + 0x70c, 1);
            return leave();
        }
        cpu.eax = cpu.edx;
        if (!direct(app, cpu, carReversing))
            return;
        if (cpu.eax != 0)
        {
            if (!call(radioStopped))
                return;
            return leave();
        }
        cpu.ebx = at(app, 0x6fd51c);
        if (cpu.ebx != 0)
            return leave();
        cpu.eax = at(app, cpu.ecx + 0x628);
        cpu.edx = at(app, kClock);
        cpu.edx -= at(app, cpu.eax + 0x24);
        if (sreg32(cpu.edx) <= 0x140)
            return leave();
        cpu.edx = at(app, kClock);
        cpu.edi = at(app, cpu.eax + 0x5c);
        put(app, cpu.eax + 0x24, cpu.edx);
        if (cpu.ecx == cpu.edi)
        {
            put(app, cpu.eax + 0x5c, cpu.ebx);
            cpu.eax = cpu.edi;
        }
        else
        {
            if (!call(callRoadblock))
                return;
            if (cpu.eax != 0)
                return leave();
            if (!call(callTwoCars))
                return;
            if (cpu.eax != 0)
                return leave();
            cpu.eax = cpu.ecx;
        }
        if (!direct(app, cpu, callBackup))
            return;
        return leave();
    }
    case 6:
        cpu.eax = cpu.ecx;
        put(app, cpu.ecx + 0x7b0, 0);
        if (!byAddress(app, cpu, kStop) || !call(stopOver))
            return;
        if (cpu.eax != 0)
            put(app, cpu.ecx + 0x70c, 1);
        return leave();
    }
}

/* 0x403830: car eax (not a player's) given an order (its kind ebx, node cx,
 * the stack's word, side byte and flag byte), from car edx (+0x730), not
 * yet taken.  ret 0xc; edx left as the arguments' bytes have it. */
void giveOrder(App* app, CPU& cpu)
{
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esi = cpu.eax;
    cpu.eax = at(app, cpu.eax + 0x7ac);
    if ((byte(app, cpu.esi + 0x200) & 4) == 0)
    {
        put(app, cpu.esi + 0x730, cpu.edx);
        putByte(app, cpu.eax + 1, 0);
        putByte(app, cpu.eax, cpu.bl);
        cpu.edx = at(app, cpu.ebp + 0xc);
        putWord(app, cpu.eax + 2, cpu.cx);
        putWord(app, cpu.eax + 4, cpu.dx);
        cpu.dl = byte(app, cpu.ebp + 0x10);
        putByte(app, cpu.eax + 6, cpu.dl);
        cpu.dl = byte(app, cpu.ebp + 0x14);
        putByte(app, cpu.eax + 7, cpu.dl);
    }
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.esp += 4 + 0xc;
}

/* 0x403870: the cops spread round the track at the race's start: each its
 * order of kind 2 at a node a share of the track on from the last, less a
 * random quarter of it, the side by its index. */
void spreadCops(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    cpu.eax = at(app, kNodeCount);
    cpu.esi = at(app, kCopCount);
    ++cpu.esi;
    divide(cpu, cpu.esi);
    cpu.edi = 0;
    put(app, cpu.ebp - 4, cpu.eax);
    for (cpu.esi = 0; sreg32(cpu.esi) < sreg32(at(app, kCopCount));)
    {
        cpu.eax = at(app, kSeed);
        cpu.ecx = at(app, kMultiplier);
        cpu.eax = imul(cpu.eax, cpu.ecx);
        put(app, kProduct, cpu.eax);
        cpu.eax &= 0xffff;
        put(app, kSeed, cpu.eax);
        cpu.eax = at(app, kProduct);
        cpu.edx = at(app, cpu.ebp - 4);
        cpu.eax &= 0xffff00;
        cpu.edx = reg32(sreg32(cpu.edx) >> 2);
        cpu.eax >>= 8;
        cpu.eax = imul(cpu.eax, cpu.edx);
        cpu.edx = at(app, cpu.ebp - 4);
        cpu.eax >>= 16;
        push(app, cpu, 0);
        cpu.edx -= cpu.eax;
        cpu.eax = cpu.esi;
        cpu.ebx = 2;
        cpu.eax &= 1;
        cpu.edi += cpu.edx;
        push(app, cpu, cpu.eax);
        cpu.ecx = cpu.edi;
        cpu.edx = 0;
        push(app, cpu, 5);
        cpu.eax = at(app, cpu.esi * 4 + kCops);
        ++cpu.esi;
        if (!direct(app, cpu, giveOrder))
            return;
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x403910: cop eax's word on the radio that its target has stopped
 * (0x40bc30), once for the pursuit (+0x54 bit 8). */
void radioStopped(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = cpu.eax;
    cpu.eax = at(app, cpu.eax + 0x624);
    cpu.eax = at(app, cpu.eax + 0x628);
    if ((byte(app, cpu.eax + 0x54) & 8) == 0)
    {
        cpu.eax = cpu.edx;
        if (!byAddress(app, cpu, kRadioStopped))
            return;
        cpu.eax = at(app, cpu.edx + 0x624);
        cpu.eax = at(app, cpu.eax + 0x628);
        putByte(app, cpu.eax + 0x54, byte(app, cpu.eax + 0x54) | 8);
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x403950: cop eax's word on the radio that it stopped (0x40bba0), once
 * (+0x711 bit 0x10). */
void radioStop(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = cpu.eax;
    if ((byte(app, cpu.eax + 0x711) & 0x10) == 0)
    {
        cpu.eax = cpu.edx;
        if (!byAddress(app, cpu, kRadioStop))
            return;
        putByte(app, cpu.edx + 0x711, byte(app, cpu.edx + 0x711) | 0x10);
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x403970: the race's pursuits cleared (0x56e410, 16 of 0x60 bytes) and
 * each racer given its own; the cops spread (spreadCops). */
void initPursuits(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ebx = 0x600;
    cpu.eax = kPursuits;
    cpu.edx = 0;
    if (!byAddress(app, cpu, kMemset))
        return;
    for (cpu.edx = 0; sreg32(cpu.edx) < sreg32(at(app, kRacerCount));)
    {
        cpu.eax = cpu.edx * 4;
        cpu.eax -= cpu.edx;
        cpu.eax <<= 5;
        cpu.ecx = at(app, cpu.edx * 4 + kRacers);
        cpu.eax += kPursuits;
        ++cpu.edx;
        put(app, cpu.ecx + 0x628, cpu.eax);
    }
    if (!direct(app, cpu, spreadCops))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4039c0: cop eax calls in a roadblock (1), unless the race has none
 * (0x6fd51c), it chases no one, the pursuit has one, or the pursuit is not
 * yet hot enough (cops, getaways, tickets and time over 4): the first time
 * only the radio's call (0x40b910, +0x54 bit 2); then the free cop (not a
 * player's, no order) ahead of the target more than 50 nodes and least far
 * gets the order (kind 3, 6 nodes before it) and is the roadblock. */
void callRoadblock(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 8;
    const auto leave = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    const auto no = [&] {
        cpu.eax = 0;
        leave();
    };
    cpu.ecx = cpu.eax;
    cpu.edx = at(app, kNodeCount);
    cpu.eax = cpu.edx * 4;
    cpu.eax -= cpu.edx;
    quarter(cpu);
    cpu.esi = 0;
    cpu.edx = at(app, 0x6fd51c);
    put(app, cpu.ebp - 4, cpu.eax);
    if (cpu.edx != 0)
        return no();
    cpu.ebx = at(app, cpu.ecx + 0x624);
    if (cpu.ebx == 0)
        return no();
    cpu.edi = at(app, cpu.ecx + 0x628);
    if (at(app, cpu.edi + 0x58) != 0)
        return no();
    cpu.edx = at(app, cpu.edi + 0x2c);
    cpu.eax = at(app, cpu.edi + 0x14);
    --cpu.edx;
    cpu.eax += cpu.edx;
    cpu.edx = at(app, cpu.ebx + 0x2c8);
    cpu.edx -= 2;
    cpu.eax += cpu.edx;
    cpu.edx = at(app, kClock);
    cpu.edx -= at(app, cpu.edi + 0x20);
    cpu.edx = reg32(sreg32(cpu.edx) >> 11);
    cpu.eax += cpu.edx;
    if (sreg32(cpu.eax) < 4)
        return no();
    if ((byte(app, cpu.edi + 0x54) & 2) == 0)
    {
        cpu.eax = cpu.ecx;
        if (!byAddress(app, cpu, kRadioRoadblock))
            return;
        cpu.eax = at(app, cpu.ecx + 0x628);
        putByte(app, cpu.eax + 0x54, byte(app, cpu.eax + 0x54) | 2);
        cpu.eax = 0;
        put(app, cpu.edi + 0x5c, cpu.ecx);
        return leave();
    }
    cpu.eax = at(app, cpu.ebx + 0x1c);
    put(app, cpu.ebp - 8, cpu.eax);
    for (cpu.ebx = 0; sreg32(cpu.ebx) < sreg32(at(app, kCopCount)); ++cpu.ebx)
    {
        cpu.eax = at(app, cpu.ebx * 4 + kCops);
        if (byte(app, cpu.eax + 0x200) & 4)
            continue;
        if (at(app, cpu.eax + 0x624) != 0)
            continue;
        if (byte(app, cpu.eax + 0x711) & 8)
            continue;
        cpu.edx = at(app, cpu.eax + 0x1c);
        cpu.edx -= at(app, cpu.ebp - 8);
        if (at(app, kBackwards) != 0)
            cpu.edx = neg(cpu.edx);
        if (sreg32(cpu.edx) <= 0x32)
            continue;
        if (sreg32(cpu.edx) >= sreg32(at(app, cpu.ebp - 4)))
            continue;
        cpu.esi = cpu.eax;
        put(app, cpu.ebp - 4, cpu.edx);
    }
    if (cpu.esi == 0)
        return no();
    cpu.eax = at(app, cpu.esi + 0x1c);
    cpu.eax -= 6;
    if (sreg32(cpu.eax) < 0)
        cpu.eax += at(app, kNodeCount);
    push(app, cpu, 0);
    cpu.dh = byte(app, cpu.esi + 0x711);
    push(app, cpu, 0);
    cpu.dh &= 0xfb;
    cpu.ebx = 0;
    putByte(app, cpu.esi + 0x711, cpu.dh);
    push(app, cpu, 3);
    cpu.edx = at(app, cpu.ecx + 0x624);
    cpu.ecx = cpu.eax;
    cpu.eax = cpu.esi;
    if (!direct(app, cpu, giveOrder))
        return;
    putByte(app, cpu.esi + 0x711, byte(app, cpu.esi + 0x711) | 8);
    cpu.eax = 1;
    put(app, cpu.edi + 0x58, cpu.esi);
    leave();
}

/* 0x403b20: cop eax calls in two cars to box in a player's car (1), unless
 * the race has no roadblocks (0x55eb2e bit 0x20, 0x6fd51c) or the pursuit is
 * not hot enough (over 3): the radio's call once (0x40b860, +0x54 bit 4);
 * the two free cops ahead of the car more than 50 nodes, the nearest two,
 * if within 400 nodes of each other, each get the order (kind 4, the node
 * midway between them, the first flagged) and are marked (+0x710 0x410). */
void callTwoCars(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x10;
    const auto leave = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    const auto no = [&] {
        cpu.eax = 0;
        leave();
    };
    cpu.edi = cpu.eax;
    cpu.edx = at(app, kNodeCount);
    cpu.eax = cpu.edx * 4;
    cpu.eax -= cpu.edx;
    quarter(cpu);
    cpu.esi = 0;
    put(app, cpu.ebp - 0xc, cpu.eax);
    cpu.ah = byte(app, 0x55eb2e);
    put(app, cpu.ebp - 4, cpu.esi);
    if (cpu.ah & 0x20)
        return no();
    if (at(app, 0x6fd51c) != 0)
        return no();
    cpu.ebx = at(app, cpu.edi + 0x624);
    if (cpu.ebx == 0)
        return no();
    cpu.dl = byte(app, cpu.ebx + 0x200);
    cpu.ecx = cpu.ebx;
    if ((cpu.dl & 4) == 0)
        return no();
    cpu.edx = at(app, cpu.edi + 0x628);
    cpu.ebx = at(app, cpu.edx + 0x2c);
    --cpu.ebx;
    cpu.eax = at(app, cpu.edx + 0x14);
    cpu.eax += cpu.ebx;
    cpu.ebx = at(app, cpu.ecx + 0x2c8);
    cpu.ecx = at(app, kClock);
    cpu.eax += cpu.ebx;
    cpu.ecx -= at(app, cpu.edx + 0x20);
    cpu.ecx = reg32(sreg32(cpu.ecx) >> 11);
    cpu.eax += cpu.ecx;
    if (sreg32(cpu.eax) < 3)
        return no();
    if ((byte(app, cpu.edx + 0x54) & 4) == 0)
    {
        cpu.eax = cpu.edi;
        if (!byAddress(app, cpu, kRadioTwoCars))
            return;
        cpu.eax = at(app, cpu.edi + 0x628);
        putByte(app, cpu.eax + 0x54, byte(app, cpu.eax + 0x54) | 4);
        put(app, cpu.edx + 0x5c, cpu.edi);
    }
    cpu.eax = at(app, cpu.edi + 0x624);
    cpu.eax = at(app, cpu.eax + 0x1c);
    put(app, cpu.ebp - 0x10, cpu.eax);
    for (cpu.ebx = 0; sreg32(cpu.ebx) < sreg32(at(app, kCopCount)); ++cpu.ebx)
    {
        cpu.ecx = at(app, cpu.ebx * 4 + kCops);
        if (byte(app, cpu.ecx + 0x200) & 4)
            continue;
        if (at(app, cpu.ecx + 0x624) != 0)
            continue;
        cpu.ah = byte(app, cpu.ecx + 0x711);
        if (cpu.ah & 8)
            continue;
        if (cpu.ah & 4)
            continue;
        cpu.edx = at(app, cpu.ebp - 0x10);
        cpu.eax = at(app, cpu.ecx + 0x1c);
        if (!byAddress(app, cpu, kNodeGap))
            return;
        cpu.edx = cpu.eax;
        if (at(app, kBackwards) != 0)
            cpu.eax = neg(cpu.eax);
        if (sreg32(cpu.eax) <= 0x32)
            continue;
        if (sreg32(cpu.eax) >= sreg32(at(app, cpu.ebp - 0xc)))
            continue;
        if (cpu.esi != 0)
            put(app, cpu.ebp - 4, cpu.esi);
        cpu.esi = cpu.ecx;
        put(app, cpu.ebp - 0xc, cpu.eax);
    }
    if (cpu.esi == 0)
        return no();
    cpu.ecx = at(app, cpu.ebp - 4);
    if (cpu.ecx == 0)
        return no();
    cpu.edx = at(app, cpu.esi + 0x1c);
    cpu.eax = at(app, cpu.ecx + 0x1c);
    if (!byAddress(app, cpu, kNodeGap))
        return;
    cpu.edx = cpu.eax;
    if (sreg32(cpu.eax) >= 0x190)
        return no();
    cpu.edx = reg32(sreg32(cpu.edx) >> 31);
    cpu.eax -= cpu.edx;
    cpu.eax = reg32(sreg32(cpu.eax) >> 1);
    if (sreg32(cpu.eax) >= 0)
    {
        cpu.ebx = at(app, cpu.esi + 0x1c);
        cpu.edx = at(app, kNodeCount);
        cpu.eax += cpu.ebx;
        if (sreg32(cpu.eax) >= sreg32(cpu.edx))
            cpu.eax -= cpu.edx;
    }
    else
    {
        cpu.eax += at(app, cpu.esi + 0x1c);
        if (sreg32(cpu.eax) < 0)
            cpu.eax += at(app, kNodeCount);
    }
    put(app, cpu.ebp - 8, cpu.eax);
    push(app, cpu, 1);
    push(app, cpu, 0);
    cpu.ecx = at(app, cpu.ebp - 8);
    cpu.ebx = 4;
    push(app, cpu, 0);
    cpu.eax = at(app, cpu.ebp - 4);
    cpu.edx = at(app, cpu.edi + 0x624);
    if (!direct(app, cpu, giveOrder))
        return;
    cpu.eax = at(app, cpu.ebp - 4);
    push(app, cpu, 0);
    cpu.ecx = at(app, cpu.ebp - 8);
    cpu.ebx = 4;
    push(app, cpu, 0);
    cpu.dx = word(app, cpu.eax + 0x710);
    put(app, cpu.eax + 0x734, 1);
    cpu.edx |= 0x410;
    push(app, cpu, 0);
    putWord(app, cpu.eax + 0x710, cpu.dx);
    cpu.eax = cpu.esi;
    cpu.edx = at(app, cpu.edi + 0x624);
    if (!direct(app, cpu, giveOrder))
        return;
    cpu.bx = word(app, cpu.esi + 0x710);
    put(app, cpu.esi + 0x734, 1);
    cpu.ebx |= 0x410;
    cpu.eax = 1;
    putWord(app, cpu.esi + 0x710, cpu.bx);
    leave();
}

/* 0x403d30: cop eax at its roadblock with its siren off: the siren on
 * (0x4c9dd0: its index, its node, the clock), the roadblock's side on the
 * radio (0x40b9c0: lane 6 or 7 and a side 0..2 by the index's two low
 * bits, mirrored where the track runs backwards), its count of them
 * (+0x710's low nibble) one up to 15, its mark of passing off. */
void roadblockSiren(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esi = cpu.eax;
    cpu.edx = at(app, cpu.eax + 0x714);
    cpu.eax = cpu.edx * 4;
    cpu.eax += cpu.edx;
    cpu.eax <<= 4;
    if (at(app, cpu.eax + kSirens) == 0)
    {
        cpu.eax = at(app, cpu.esi + 0x714);
        if (cpu.al & 1)
        {
            cpu.ebx = 6;
            cpu.edi = 0;
        }
        else
        {
            cpu.ebx = 7;
            cpu.edi = 2;
        }
        if (cpu.al & 2)
            cpu.edi = 1;
        if (at(app, kBackwards) != 0)
        {
            cpu.eax = 2;
            cpu.eax -= cpu.edi;
            cpu.edi = cpu.eax;
            cpu.eax = 0xe;
            cpu.eax -= cpu.ebx;
            cpu.ebx = cpu.eax;
        }
        cpu.ecx = at(app, kClock);
        cpu.edx = at(app, cpu.esi + 0x1c);
        cpu.eax = at(app, cpu.esi + 0x714);
        if (!byAddress(app, cpu, kSirenOn))
            return;
        cpu.edx = cpu.edi;
        cpu.eax = cpu.esi;
        if (!byAddress(app, cpu, kRadioRoadblockSide))
            return;
        cpu.eax = at(app, cpu.esi + 0x710);
        cpu.eax &= 0xf;
        if (sreg32(cpu.eax) < 0xf)
            put(app, cpu.esi + 0x710, at(app, cpu.esi + 0x710) + 1);
        putByte(app, cpu.esi + 0x711, byte(app, cpu.esi + 0x711) & 0xdf);
    }
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x403df0: cop eax at a roadblock watches the car it is set for (+0x730)
 * against the siren's node (+0x34 of its siren's record): within 30 nodes
 * past it, the radio's word that the car passed (0x40bca0, once, +0x711 bit
 * 0x20); 50 nodes past it (but not a quarter of the track), the siren off,
 * the roadblock done (the pursuit's roadblock car cleared) and a new order
 * of kind 2 by its index. */
void watchRoadblock(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    const auto leave = [&] {
        cpu.ebp = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.esi = cpu.eax;
    cpu.ebx = at(app, cpu.eax + 0x714);
    cpu.eax = cpu.ebx * 4;
    cpu.eax += cpu.ebx;
    cpu.edx = at(app, cpu.esi + 0x730);
    cpu.eax <<= 4;
    cpu.edx = at(app, cpu.edx + 0x1c);
    cpu.eax = at(app, cpu.eax + 0x7dd884);
    if (!byAddress(app, cpu, kNodeGap))
        return;
    cpu.edx = at(app, kBackwards);
    cpu.ecx = cpu.eax;
    if (cpu.edx != 0)
        cpu.ecx = neg(cpu.ecx);
    if (sreg32(cpu.ecx) < 0 && sreg32(cpu.ecx) > -0x1e)
    {
        cpu.ah = byte(app, cpu.esi + 0x711);
        if ((cpu.ah & 0x20) == 0)
        {
            cpu.dl = cpu.ah;
            cpu.dl |= 0x20;
            cpu.eax = cpu.esi;
            putByte(app, cpu.esi + 0x711, cpu.dl);
            if (!byAddress(app, cpu, kRadioPassed))
                return;
        }
    }
    if (sreg32(cpu.ecx) > -0x32)
        return leave();
    cpu.eax = at(app, kNodeCount);
    quarter(cpu);
    cpu.eax = neg(cpu.eax);
    if (sreg32(cpu.ecx) < sreg32(cpu.eax))
        return leave();
    cpu.eax = cpu.ebx;
    if (!byAddress(app, cpu, kSirenOff))
        return;
    cpu.dh = byte(app, cpu.esi + 0x711);
    cpu.dh &= 0xf7;
    cpu.ecx = at(app, cpu.esi + 0x730);
    putByte(app, cpu.esi + 0x711, cpu.dh);
    if (cpu.ecx != 0)
    {
        cpu.eax = at(app, cpu.ecx + 0x628);
        if (cpu.esi == at(app, cpu.eax + 0x58))
            put(app, cpu.eax + 0x58, 0);
    }
    cpu.eax = at(app, cpu.esi + 0x714);
    push(app, cpu, 0);
    cpu.eax &= 1;
    push(app, cpu, cpu.eax);
    cpu.ebx = 2;
    cpu.ecx = 0;
    push(app, cpu, 0);
    cpu.edx = 0;
    cpu.eax = cpu.esi;
    if (!direct(app, cpu, giveOrder))
        return;
    leave();
}

/* 0x403ed0: cop eax waiting as a second car watches its car (+0x730): 5
 * nodes past it (but not a quarter of the track), its wait over -- within
 * 20 nodes, parked (state 10), the radio's word that it missed (0x40bd00)
 * -- and a new order of kind 2 by its index. */
void watchSecondCar(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    const auto leave = [&] {
        cpu.ebp = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.esi = cpu.eax;
    cpu.edx = at(app, cpu.esi + 0x730);
    cpu.eax = at(app, cpu.eax + 0x1c);
    cpu.edx = at(app, cpu.edx + 0x1c);
    if (!byAddress(app, cpu, kNodeGap))
        return;
    cpu.edx = at(app, kBackwards);
    cpu.ecx = cpu.eax;
    if (cpu.edx != 0)
        cpu.ecx = neg(cpu.ecx);
    if (sreg32(cpu.ecx) > -5)
        return leave();
    cpu.eax = at(app, kNodeCount);
    quarter(cpu);
    cpu.eax = neg(cpu.eax);
    if (sreg32(cpu.ecx) < sreg32(cpu.eax))
        return leave();
    if (sreg32(cpu.ecx) < 0 && sreg32(cpu.ecx) > -0x14 && at(app, cpu.esi + 0x70c) == 0xa)
    {
        cpu.eax = cpu.esi;
        if (!byAddress(app, cpu, kRadioMissed))
            return;
    }
    cpu.ah = byte(app, cpu.esi + 0x711);
    cpu.ah &= 0xfb;
    push(app, cpu, 0);
    putByte(app, cpu.esi + 0x711, cpu.ah);
    cpu.ebx = 2;
    cpu.eax = at(app, cpu.esi + 0x714);
    cpu.dl = byte(app, cpu.esi + 0x621);
    cpu.eax &= 1;
    cpu.dl &= 0xf7;
    push(app, cpu, cpu.eax);
    cpu.ecx = 0;
    putByte(app, cpu.esi + 0x621, cpu.dl);
    push(app, cpu, 0);
    cpu.edx = 0;
    cpu.eax = cpu.esi;
    if (!direct(app, cpu, giveOrder))
        return;
    leave();
}

/* 0x403f80: how many more cops racer edx wants (0 for none), cop eax asking
 * (0 none): none for a racer out of the race; all of them for the car the
 * cop was set on; for one that never got away and is not over its node's
 * speed by 0x536104 nor speeding past 2, none; else up to a third of the
 * cops (at least 1) -- all but one of them for a player's car -- less those
 * on it. */
void copsWanted(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    const auto leave = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    const auto no = [&] {
        cpu.eax = 0;
        leave();
    };
    cpu.ebx = cpu.eax;
    cpu.ecx = cpu.edx;
    cpu.eax = at(app, kCopCount);
    cpu.esi = 3;
    divide(cpu, cpu.esi);
    if (!(sreg32(cpu.eax) > 1))
        cpu.eax = 1;
    cpu.esi = cpu.eax;
    cpu.eax = at(app, kCopCount);
    --cpu.eax;
    if (!(sreg32(cpu.eax) > 1))
        cpu.eax = 1;
    cpu.edi = cpu.eax;
    cpu.eax = at(app, cpu.ecx + 0x200);
    cpu.eax &= 4;
    cpu.edx = at(app, cpu.ecx + 0x2c4);
    put(app, cpu.ebp - 4, cpu.eax);
    if (cpu.edx == 4)
        return no();
    cpu.edx = at(app, cpu.ecx + 0x628);
    if (cpu.ebx != 0 && cpu.ecx == at(app, cpu.ebx + 0x730))
    {
        cpu.eax = at(app, kCopCount);
        return leave();
    }
    if (at(app, cpu.edx + 0x14) == 0)
    {
        cpu.eax = at(app, cpu.ecx + 0x1c);
        if (!byAddress(app, cpu, kNodeSpeed))
            return;
        fadd(cpu, f64(app, 0x536104));
        fld(cpu, f32(app, cpu.ecx + 0x7c4));
        if (fcompp(cpu).belowOrEqual() && !(sreg32(at(app, cpu.ecx + 0x6f0)) > 0x40000000))
            return no();
    }
    cpu.eax = at(app, cpu.ebp - 4) != 0 ? cpu.edi : cpu.esi;
    cpu.eax -= at(app, cpu.edx + 0x2c);
    if (sreg32(cpu.eax) > 0)
        return leave();
    no();
}

/* 0x404070: cop eax's word on the radio as it joins (0x40b220; 0x40b530 for a
 * car that got away before, from a cop not a player's), or as a second
 * (0x40b330). */
void radioJoined(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = at(app, cpu.eax + 0x628);
    reg32 target = kRadioSecond;
    if (at(app, cpu.edx + 0x2c) == 1)
        target = at(app, cpu.edx + 0x14) != 0 && (byte(app, cpu.eax + 0x200) & 4) == 0 ? kRadioJoinWanted
                                                                                         : kRadioJoin;
    if (!byAddress(app, cpu, target))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4040b0: cop eax's word on the radio as it gives up (0x40b480) and its
 * order (kind 2 at its node, 6, by side 1). */
void radioGiveUp(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esi = cpu.eax;
    if (!byAddress(app, cpu, kRadioGiveUp))
        return;
    push(app, cpu, 0);
    push(app, cpu, 1);
    cpu.ebx = 2;
    cpu.eax = cpu.esi;
    push(app, cpu, 6);
    cpu.ecx = at(app, cpu.esi + 0x1c);
    cpu.edx = 0;
    if (!direct(app, cpu, giveOrder))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4040e0: the pursuit of cop eax's target over: the radio's word (0x40b670);
 * every cop set on that car let go (its order and roadblock, its siren off,
 * the pursuit's roadblock cleared, a new order of kind 2); every cop on the
 * pursuit off it (a timer 0x405fd0, its radio level down) -- a player's
 * reset as a driver (0x405220) with its siren off, another given a new
 * order -- and its slot cleared. */
void releaseCops(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 8;
    cpu.edi = cpu.eax;
    cpu.eax = at(app, cpu.eax + 0x628);
    cpu.edx = 0;
    put(app, cpu.ebp - 8, cpu.eax);
    cpu.eax = cpu.edi;
    put(app, cpu.ebp - 4, cpu.edx);
    if (!byAddress(app, cpu, kRadioCleared))
        return;
    for (;; put(app, cpu.ebp - 4, at(app, cpu.ebp - 4) + 1))
    {
        cpu.eax = at(app, cpu.ebp - 4);
        if (sreg32(cpu.eax) >= sreg32(at(app, kCopCount)))
            break;
        cpu.esi = at(app, cpu.eax * 4 + kCops);
        cpu.ah = byte(app, cpu.esi + 0x200);
        putByte(app, cpu.esi + 0x8a, 0);
        if (cpu.ah & 4)
            continue;
        cpu.eax = at(app, cpu.esi + 0x730);
        cpu.ebx = at(app, cpu.edi + 0x624);
        cpu.edx = at(app, cpu.esi + 0x714);
        if (cpu.eax != cpu.ebx)
            continue;
        cpu.bl = byte(app, cpu.esi + 0x711);
        put(app, cpu.esi + 0x730, 0);
        if (cpu.bl & 8)
        {
            cpu.eax = cpu.edx * 4;
            cpu.bh = cpu.bl;
            cpu.eax += cpu.edx;
            cpu.bh &= 0xf7;
            cpu.eax <<= 4;
            putByte(app, cpu.esi + 0x711, cpu.bh);
            if (at(app, cpu.eax + kSirens) != 0)
            {
                cpu.eax = cpu.edx;
                if (!byAddress(app, cpu, kSirenOff))
                    return;
            }
            cpu.eax = at(app, cpu.ebp - 8);
            put(app, cpu.eax + 0x58, 0);
        }
        push(app, cpu, 0);
        cpu.ebx = 2;
        cpu.eax = at(app, cpu.esi + 0x714);
        cpu.dx = word(app, cpu.esi + 0x710);
        cpu.eax &= 1;
        cpu.edx &= 0xfffffbef;
        push(app, cpu, cpu.eax);
        cpu.ecx = 0;
        putWord(app, cpu.esi + 0x710, cpu.dx);
        push(app, cpu, 0);
        cpu.eax = cpu.esi;
        cpu.edx = 0;
        put(app, cpu.esi + 0x734, 0);
        if (!direct(app, cpu, giveOrder))
            return;
    }
    for (cpu.edi = 0;;)
    {
        cpu.eax = at(app, cpu.ebp - 8);
        if (sreg32(cpu.edi) >= sreg32(at(app, cpu.eax + 0x2c)))
            break;
        cpu.edx = at(app, cpu.ebp - 8);
        cpu.eax = cpu.edi * 4;
        cpu.eax += cpu.edx;
        cpu.esi = at(app, cpu.eax + 0x30);
        if (cpu.esi != 0)
        {
            put(app, cpu.esi + 0x628, 0);
            put(app, cpu.esi + 0x624, 0);
            cpu.edx = 1;
            put(app, cpu.esi + 0x730, 0);
            cpu.eax = cpu.esi;
            cpu.cl = byte(app, cpu.esi + 0x710);
            put(app, cpu.esi + 0x734, 0);
            cpu.cl &= 0x6f;
            cpu.ebx = 0;
            putByte(app, cpu.esi + 0x710, cpu.cl);
            if (!byAddress(app, cpu, kSetTimer))
                return;
            cpu.eax = cpu.esi;
            if (!direct(app, cpu, radioLevelDown))
                return;
            if (byte(app, cpu.esi + 0x200) & 4)
            {
                cpu.eax = cpu.esi;
                cpu.edx = 0;
                if (!byAddress(app, cpu, kResetDriver))
                    return;
                cpu.eax = at(app, cpu.esi + 0x714);
                if (!byAddress(app, cpu, kSirenOff))
                    return;
            }
            else
            {
                cpu.eax = at(app, cpu.esi + 0x714);
                push(app, cpu, 0);
                cpu.eax &= 1;
                push(app, cpu, cpu.eax);
                cpu.ebx = 2;
                cpu.ecx = 0;
                push(app, cpu, 0);
                cpu.edx = 0;
                cpu.eax = cpu.esi;
                if (!direct(app, cpu, giveOrder))
                    return;
            }
        }
        cpu.ecx = at(app, cpu.ebp - 8);
        cpu.eax = cpu.edi * 4;
        cpu.eax += cpu.ecx;
        ++cpu.edi;
        put(app, cpu.eax + 0x30, 0);
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4042b0: a player's cop eax drops off its pursuit (leaveCopList) and is
 * cleared of it. */
void dropCop(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = cpu.eax;
    if (byte(app, cpu.eax + 0x200) & 4)
    {
        cpu.eax = cpu.edx;
        if (!direct(app, cpu, leaveCopList))
            return;
        put(app, cpu.edx + 0x628, 0);
        put(app, cpu.edx + 0x624, 0);
        put(app, cpu.edx + 0x730, 0);
        cpu.bl = byte(app, cpu.edx + 0x710);
        put(app, cpu.edx + 0x734, 0);
        cpu.bl &= 0x6f;
        putByte(app, cpu.edx + 0x710, cpu.bl);
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x404340: cop eax sent to cut off car edx: an order of kind 3 at the node
 * midway between them, wrapped round the track, 8 and flag 0. */
void sendCopAhead(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edi = cpu.eax;
    cpu.esi = cpu.edx;
    cpu.ecx = at(app, cpu.eax + 0x1c);
    cpu.eax = at(app, cpu.edx + 0x1c);
    cpu.edx = cpu.ecx;
    if (!byAddress(app, cpu, kNodeGap))
        return;
    cpu.edx = cpu.eax;
    cpu.edx = reg32(sreg32(cpu.edx) >> 31);
    cpu.eax -= cpu.edx;
    cpu.eax = reg32(sreg32(cpu.eax) >> 1);
    cpu.edx = cpu.eax;
    cpu.eax += cpu.ecx;
    if (sreg32(cpu.edx) >= 0)
    {
        cpu.edx = at(app, kNodeCount);
        if (sreg32(cpu.eax) >= sreg32(cpu.edx))
            cpu.eax -= cpu.edx;
    }
    else if (sreg32(cpu.eax) < 0)
        cpu.eax += at(app, kNodeCount);
    push(app, cpu, 0);
    push(app, cpu, 0);
    cpu.ebx = 3;
    cpu.ecx = cpu.eax;
    push(app, cpu, 8);
    cpu.edx = cpu.esi;
    cpu.eax = cpu.edi;
    if (!direct(app, cpu, giveOrder))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4043a0: cop eax calls backup for its target where more cops are wanted
 * (copsWanted): the first time, after 0x280 ticks alone, the radio's call
 * (0x40b720, +0x54 bit 1); then of the free cops not at roadblocks, the fast
 * ones (+0x711 bit 1) ahead of the target within 8 times its tickets' gap
 * are each sent to cut it off (sendCopAhead), and of the others the nearest
 * on the other side of it than the cop, within 200 nodes and twice the gap,
 * is sent -- unless it is setting up a roadblock (state 7). */
void callBackup(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x14;
    const auto leave = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.esi = cpu.eax;
    cpu.edx = 0xc8;
    cpu.ebx = at(app, cpu.eax + 0x624);
    cpu.edi = 0;
    put(app, cpu.ebp - 0x10, cpu.edx);
    if (cpu.ebx == 0)
        return leave();
    cpu.edx = cpu.ebx;
    cpu.eax = 0;
    if (!direct(app, cpu, copsWanted))
        return;
    if (cpu.eax == 0)
        return leave();
    cpu.ecx = at(app, cpu.esi + 0x628);
    cpu.ah = byte(app, cpu.ecx + 0x54);
    if ((cpu.ah & 1) == 0 && (cpu.ah & 2) == 0 && (cpu.ah & 4) == 0)
    {
        cpu.eax = at(app, kClock);
        cpu.eax -= at(app, cpu.ecx + 0x20);
        if (sreg32(cpu.eax) > 0x280 && at(app, cpu.ecx + 0x2c) == 1)
        {
            cpu.eax = cpu.esi;
            if (!byAddress(app, cpu, kRadioBackup))
                return;
            putByte(app, cpu.ecx + 0x54, byte(app, cpu.ecx + 0x54) | 1);
        }
    }
    cpu.eax = at(app, cpu.esi + 0x1c);
    cpu.esi = at(app, cpu.ebx + 0x1c);
    cpu.edx = cpu.esi;
    if (!byAddress(app, cpu, kNodeGap))
        return;
    cpu.edx = 0;
    put(app, cpu.ebp - 0x14, cpu.eax);
    put(app, cpu.ebp - 8, cpu.edx);
    for (;; put(app, cpu.ebp - 8, at(app, cpu.ebp - 8) + 1))
    {
        cpu.eax = at(app, cpu.ebp - 8);
        if (sreg32(cpu.eax) >= sreg32(at(app, kCopCount)))
            break;
        cpu.ecx = at(app, cpu.eax * 4 + kCops);
        if (byte(app, cpu.ecx + 0x200) & 4)
            continue;
        if (at(app, cpu.ecx + 0x624) != 0)
            continue;
        cpu.dl = byte(app, cpu.ecx + 0x711);
        if (cpu.dl & 8)
            continue;
        if (cpu.dl & 4)
            continue;
        cpu.edx = cpu.esi;
        cpu.eax = at(app, cpu.ecx + 0x1c);
        if (!byAddress(app, cpu, kNodeGap))
            return;
        cpu.edx = cpu.eax;
        if (byte(app, cpu.ecx + 0x711) & 1)
        {
            if (at(app, cpu.ecx + 0x70c) == 7)
                continue;
            if (at(app, kBackwards) != 0 && sreg32(cpu.eax) >= 0)
                continue;
            if (at(app, kBackwards) == 0 && sreg32(cpu.edx) < 0)
                continue;
            cpu.eax = sreg32(cpu.edx) > 0 ? cpu.edx : neg(cpu.edx);
            if (sreg32(cpu.eax) > 0)
                put(app, cpu.ebp - 0xc, cpu.eax);
            else
            {
                cpu.edx = cpu.eax;
                put(app, cpu.ebp - 0xc, cpu.eax);
                cpu.edx = neg(cpu.edx);
                put(app, cpu.ebp - 0xc, cpu.edx);
            }
            cpu.eax = at(app, cpu.ebx + 0x2c8);
            cpu.eax <<= 4;
            cpu.eax = at(app, cpu.eax + kByTickets);
            cpu.eax <<= 3;
            cpu.edx = at(app, cpu.ebp - 0xc);
            if (cpu.eax < cpu.edx)
                continue;
            cpu.edx = cpu.ebx;
            cpu.eax = cpu.ecx;
            if (!direct(app, cpu, sendCopAhead))
                return;
            continue;
        }
        cpu.edx = at(app, cpu.ebp - 0x14);
        cpu.edx ^= cpu.eax;
        if (sreg32(cpu.edx) >= 0)
            continue;
        if (!(sreg32(cpu.eax) > 0))
        {
            put(app, cpu.ebp - 4, cpu.eax);
            cpu.eax = neg(cpu.eax);
        }
        put(app, cpu.ebp - 4, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 4);
        if (sreg32(cpu.eax) >= sreg32(at(app, cpu.ebp - 0x10)))
            continue;
        cpu.eax = at(app, cpu.ebx + 0x2c8);
        cpu.eax <<= 4;
        cpu.eax = at(app, cpu.eax + kByTickets);
        cpu.edx = at(app, cpu.ebp - 4);
        cpu.eax += cpu.eax;
        if (cpu.eax < cpu.edx)
            continue;
        cpu.edi = cpu.ecx;
        put(app, cpu.ebp - 0x10, cpu.edx);
    }
    if (cpu.edi != 0 && at(app, cpu.edi + 0x70c) != 7)
    {
        cpu.edx = cpu.ebx;
        cpu.eax = cpu.edi;
        if (!direct(app, cpu, sendCopAhead))
            return;
    }
    leave();
}

/* 0x404560: whether car eax's order is new (1, now taken) or was taken (0). */
void orderTaken(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.eax = at(app, cpu.eax + 0x7ac);
    if (byte(app, cpu.eax + 1) != 0)
        cpu.eax = 0;
    else
    {
        putByte(app, cpu.eax + 1, 1);
        cpu.eax = 1;
    }
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

void nothingForCop(App* app, CPU& cpu)
{
    emptyHook(app, cpu);
}

/* 0x404610: car eax's class's rule (0x552274, 24 bytes a class by its
 * model's +0x30), 1 in place of 0 in the race's modes 1 and 2 (0x6fd3b8). */
void classRule(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.eax = at(app, cpu.eax + 0x220);
    cpu.edx = at(app, cpu.eax + 0x30);
    cpu.eax = cpu.edx * 4;
    cpu.eax -= cpu.edx;
    cpu.edx = at(app, 0x6fd3b8);
    cpu.eax = at(app, cpu.eax * 8 + 0x552274);
    if ((cpu.edx == 1 || cpu.edx == 2) && cpu.eax == 0)
        cpu.eax = 1;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

void nothingAtEnd(App* app, CPU& cpu)
{
    emptyHook(app, cpu);
}

/* 0x404660: nothingAtEnd called. */
void nothingAtEndToo(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (!direct(app, cpu, nothingAtEnd))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x404690: racer eax's (not a player's) ramming rules, its record at
 * 0x56ea70 + 0x14 by its place (+0x1f0): 0.94, 0, and by its class's kind
 * (0x55227c, the table at 0x40466c) the hits it takes (0x6e/0x64/0x3c/0x14/
 * 15/0xa), the ticks it waits after them (0x40..0xc0) and its level (5, 4,
 * 3, 1), one less in the race's mode of 0x6fd4f0 1. */
void setRammingRules(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    if ((byte(app, cpu.eax + 0x200) & 4) == 0)
    {
        const auto record = [&] {
            cpu.ecx = at(app, cpu.eax + 0x1f0);
            cpu.edx = cpu.ecx * 4;
            cpu.edx += cpu.ecx;
        };
        record();
        put(app, cpu.edx * 4 + kRamming, 0x3f70a3d7);
        record();
        cpu.ebx = 0;
        put(app, cpu.edx * 4 + kRamming + 4, cpu.ebx);
        record();
        cpu.esi = 0xf;
        put(app, cpu.edx * 4 + kRamming + 8, cpu.esi);
        record();
        put(app, cpu.edx * 4 + kRamming + 0xc, 0x3c);
        cpu.ecx = at(app, cpu.eax + 0x220);
        cpu.ecx = at(app, cpu.ecx + 0x30);
        cpu.edx = cpu.ecx * 4;
        cpu.edx -= cpu.ecx;
        cpu.edx = at(app, cpu.edx * 8 + 0x55227c);
        // jmp [edx*4 + 0x40466c]: 0x404734, 0x404745, 0x404756, 0x404767, 0x404778, 0x404784.
        switch (cpu.edx > 5 ? 2 : cpu.edx)
        {
        case 0: cpu.ebx = 0x40; cpu.edx = 5; cpu.esi = 0x6e; break;
        case 1: cpu.ebx = 0x60; cpu.edx = 4; cpu.esi = 0x64; break;
        case 2: cpu.ebx = 0x60; cpu.edx = 3; cpu.esi = 0x3c; break;
        case 3: cpu.ebx = 0x80; cpu.edx = 1; cpu.esi = 0x14; break;
        case 4: cpu.ebx = 0x80; cpu.edx = 1; break;
        default: cpu.ebx = 0xc0; cpu.edx = 1; cpu.esi = 0xa; break;
        }
        cpu.ecx = at(app, cpu.eax + 0x1f0);
        put(app, cpu.ebp - 4, cpu.ecx);
        cpu.edi = at(app, cpu.ebp - 4);
        cpu.ecx <<= 2;
        cpu.ecx += cpu.edi;
        put(app, cpu.ecx * 4 + kRamming + 8, cpu.esi);
        cpu.esi = at(app, cpu.eax + 0x1f0);
        cpu.ecx = cpu.esi * 4;
        cpu.ecx += cpu.esi;
        put(app, cpu.ecx * 4 + kRamming + 0xc, cpu.ebx);
        cpu.ecx = at(app, 0x6fd4f0);
        if (cpu.ecx == 1)
            cpu.edx -= cpu.ecx;
        cpu.ecx = at(app, cpu.eax + 0x1f0);
        cpu.eax = cpu.ecx * 4;
        cpu.eax += cpu.ecx;
        put(app, cpu.eax * 4 + kRamming + 0x10, cpu.edx);
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4047f0, 0x404810: a value of the class of the model at eax (+0x30): the
 * table's at 0x55226c, at 0x552280 (24 bytes a class). */
void classValue(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = at(app, cpu.eax + 0x30);
    cpu.eax = cpu.edx * 4;
    cpu.eax -= cpu.edx;
    cpu.eax = at(app, cpu.eax * 8 + 0x55226c);
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

void classValueToo(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = at(app, cpu.eax + 0x30);
    cpu.eax = cpu.edx * 4;
    cpu.eax -= cpu.edx;
    cpu.eax = at(app, cpu.eax * 8 + 0x552280);
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x404840: racer eax's rivalry, each tick: between ticks 0x480 and 0x4c0,
 * where the first car is marked (+0x50d) in the last 3 nodes, it rams the
 * first player's car for 50000 ticks; a racer that rams counts its wait
 * (+0x6e4) down, looks for its rival (0x40deb0) and, its rival near
 * (0x56ef3c, no more than one between, 0x56ef48) and itself slower by
 * 0x536194 than it wants, its wait over (0x40f050) -- its lane by the rival
 * (0x40de50) unless +0x620 bit 8 -- against a player's car now and then
 * (the clock's 0xc0 bits) a cop chase's sound (snd::copChaseSet) of a
 * random kind of four. */
void rivalTick(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    const auto leave = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.esi = cpu.eax;
    cpu.edx = at(app, kClock);
    if (sreg32(cpu.edx) > 0x480 && sreg32(cpu.edx) < 0x4c0)
    {
        cpu.eax = at(app, 0x5efac8);
        if (byte(app, cpu.eax + 0x50d) != 0)
        {
            cpu.edx = at(app, kNodeCount);
            cpu.ebx = at(app, cpu.eax + 0x1c);
            cpu.edx -= 3;
            if (sreg32(cpu.edx) < sreg32(cpu.ebx) && sreg32(cpu.ebx) < sreg32(at(app, kNodeCount)))
            {
                cpu.eax = at(app, 0x5efa48);
                put(app, cpu.esi + 0x6e4, 0xc350);
                put(app, cpu.esi + 0x624, cpu.eax);
            }
        }
    }
    if ((byte(app, cpu.esi + 0x200) & 8) == 0)
        return leave();
    cpu.eax = at(app, cpu.esi + 0x6e4);
    if (sreg32(cpu.eax) > 0)
    {
        cpu.edx = cpu.eax - 1;
        put(app, cpu.esi + 0x6e4, cpu.edx);
    }
    cpu.eax = cpu.esi;
    if (!byAddress(app, cpu, kRivalNear))
        return;
    if (at(app, 0x56ef3c) == 0)
        return leave();
    if (sreg32(at(app, 0x56ef48)) > 1)
        return leave();
    fld(cpu, Float(0.0));
    if (!fcomp(cpu, f32(app, cpu.esi + 0x7b0)).below())
    {
        fld(cpu, f32(app, cpu.esi + 0x7b0));
        fchs(cpu);
        fstp(app, cpu, cpu.ebp - 4);
    }
    else
    {
        cpu.eax = at(app, cpu.esi + 0x7b0);
        put(app, cpu.ebp - 4, cpu.eax);
    }
    fld(cpu, f32(app, cpu.ebp - 4));
    fld(cpu, f32(app, cpu.esi + 0x7c4));
    fadd(cpu, f64(app, 0x536194));
    if (!fcompp(cpu).below())
        return leave();
    cpu.eax = cpu.esi;
    cpu.edx = 0;
    if (!byAddress(app, cpu, kRamWait))
        return;
    if (cpu.eax != 0)
        return leave();
    if ((byte(app, cpu.esi + 0x620) & 8) == 0)
    {
        cpu.eax = cpu.esi;
        if (!byAddress(app, cpu, kRivalLane))
            return;
    }
    cpu.eax = at(app, 0x56ef3c);
    if ((byte(app, cpu.eax + 0x200) & 4) == 0)
        return leave();
    if (byte(app, kClock) & 0xc0)
        return leave();
    cpu.edi = at(app, kMultiplier);
    cpu.eax = at(app, kSeed);
    cpu.eax = imul(cpu.eax, cpu.edi);
    put(app, kProduct, cpu.eax);
    cpu.edx = at(app, kProduct);
    cpu.edx &= 0xffff00;
    cpu.eax &= 0xffff;
    cpu.edx >>= 8;
    put(app, kSeed, cpu.eax);
    cpu.eax = cpu.edx * 4;
    cpu.eax += cpu.edx;
    cpu.eax <<= 2;
    cpu.eax >>= 16;
    cpu.eax &= 3;
    // jmp [eax*4 + 0x40482c]: 0x4049a6, 0x4049b7, 0x4049c8, 0x4049d9.
    switch (cpu.eax)
    {
    case 0: cpu.ecx = 8; cpu.ebx = 0x18; cpu.edx = 3; break;
    case 1: cpu.ecx = 5; cpu.ebx = 0x10; cpu.edx = 3; break;
    case 2: cpu.ecx = 8; cpu.ebx = 0x50; cpu.edx = 2; break;
    default: cpu.ecx = 8; cpu.ebx = 0x64; cpu.edx = 1; break;
    }
    cpu.eax = cpu.esi;
    if (!byAddress(app, cpu, kCopChaseSet))
        return;
    leave();
}

/* 0x404a00: car eax touched by car +0x16c (a racer): a racing car hit hard
 * (+0x168 over 2) by a racer, the cop's way (copHitByCar); where the race
 * has ramming (not 0x55eb2e bit 0x20, 0x6fd4f0 1), a ramming racer not
 * waiting hit by a player's car not itself ramming, once its wait is over
 * (0x40f050 at 100), takes that car on (+0x624) and counts the hits (+0x6e8)
 * to its record's limit, then waits; against a ramming player in modes 1 and
 * 2 never.  A racer's next time hit (+0x65c): the hit's force times 16, 2 or
 * 7 ticks on. */
void carContact(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    cpu.ebx = at(app, cpu.eax + 0x16c);
    if ((byte(app, cpu.ebx + 0x200) & 1) && (byte(app, cpu.ecx + 0x200) & 0x20)
        && sreg32(at(app, cpu.ecx + 0x168)) > 0x40000000)
    {
        cpu.edx = cpu.ebx;
        cpu.eax = cpu.ecx;
        if (!direct(app, cpu, copHitByCar))
            return;
    }
    bool ram = (byte(app, 0x55eb2e) & 0x20) == 0 && !(sreg32(at(app, cpu.ecx + 0x6e4)) > 0);
    if (ram)
    {
        cpu.al = byte(app, cpu.ebx + 0x200);
        ram = (cpu.al & 4) && (byte(app, cpu.ecx + 0x200) & 8) && (cpu.al & 0x20) == 0 && at(app, 0x6fd4f0) == 1;
    }
    if (ram)
    {
        cpu.edx = 0x64;
        cpu.eax = cpu.ecx;
        if (!byAddress(app, cpu, kRamWait))
            return;
        if (cpu.eax == 0)
        {
            put(app, cpu.ecx + 0x6e4, cpu.eax);
            if (cpu.ebx != at(app, cpu.ecx + 0x624))
            {
                put(app, cpu.ecx + 0x6e8, 0);
                put(app, cpu.ecx + 0x624, cpu.ebx);
            }
            put(app, cpu.ecx + 0x6e8, at(app, cpu.ecx + 0x6e8) + 1);
            cpu.edx = at(app, cpu.ecx + 0x1f0);
            cpu.eax = cpu.edx * 4;
            cpu.eax += cpu.edx;
            cpu.eax <<= 2;
            cpu.edx = at(app, cpu.ecx + 0x6e8);
            if (sreg32(cpu.edx) > sreg32(at(app, cpu.eax + kRamming + 8)))
            {
                cpu.eax = at(app, cpu.eax + kRamming + 0xc);
                put(app, cpu.ecx + 0x6e8, 0);
                put(app, cpu.ecx + 0x6e4, cpu.eax);
            }
            if (byte(app, cpu.ebx + 0x200) & 8)
            {
                cpu.edi = at(app, 0x6fd3b8);
                if (cpu.edi == 1 || cpu.edi == 2)
                    put(app, cpu.ecx + 0x6e4, 0);
            }
        }
    }
    if (byte(app, cpu.ecx + 0x200) & 2)
    {
        cpu.ah = byte(app, cpu.ebx + 0x200);
        const auto force = [&] {
            fld(cpu, f32(app, cpu.ecx + 0x168));
            cpu.esp -= 4;
            fistp(app, cpu, cpu.esp);
            cpu.eax = pop(app, cpu);
        };
        if ((cpu.ah & 0x20) && (cpu.ah & 4))
        {
            force();
            cpu.edx = cpu.eax;
            cpu.eax = at(app, kClock);
            cpu.edx <<= 4;
        }
        else
        {
            force();
            if (byte(app, cpu.ebx + 0x200) & 4)
                cpu.eax += cpu.eax;
            cpu.edx = cpu.eax * 8;
            cpu.edx -= cpu.eax;
            cpu.eax = at(app, kClock);
        }
        cpu.eax += cpu.edx;
        put(app, cpu.ecx + 0x65c, cpu.eax);
    }
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x404b90: racer eax slowed for its class (1, its wanted speed by
 * 0x5361a4): touched by a car (+0x16c) -- in the race mode of 0x6fd4f0 1
 * hard enough (15) -- either the hit's force over 0x53619c, or its class's
 * rule 0 (classRule) and now and then (the clock's bits 5..7 0); else 0. */
void slowForClass(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0xc;
    const auto leave = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.edx = cpu.eax;
    cpu.ecx = 0;
    cpu.ebx = at(app, cpu.eax + 0x16c);
    put(app, cpu.ebp - 4, cpu.ecx);
    if (cpu.ebx == 0 || (at(app, 0x6fd4f0) == 1 && sreg32(at(app, cpu.eax + 0x168)) < 0x41700000))
    {
        cpu.eax = 0;
        return leave();
    }
    fld(cpu, f32(app, cpu.edx + 0x168));
    if (!fcomp(cpu, f64(app, 0x53619c)).belowOrEqual())
    {
        fld(cpu, f32(app, cpu.edx + 0x168));
        if (!byAddress(app, cpu, kChop))
            return;
        fistp(app, cpu, cpu.ebp - 4);
    }
    cpu.eax = cpu.edx;
    if (!direct(app, cpu, classRule))
        return;
    if (cpu.eax == 0)
    {
        cpu.eax = at(app, kClock);
        cpu.eax = reg32(sreg32(cpu.eax) >> 5);
        cpu.eax &= 7;
        cpu.eax = cpu.eax == 0 ? 1 : 0;
        put(app, cpu.ebp - 4, at(app, cpu.ebp - 4) | cpu.eax);
    }
    if (at(app, cpu.ebp - 4) == 0)
    {
        cpu.eax = 0;
        return leave();
    }
    fld(cpu, f32(app, cpu.edx + 0x7b0));
    fld(cpu, cpu.fpu.st(0));
    fmul(cpu, f64(app, 0x5361a4));
    cpu.eax = 1;
    fstpSt1(cpu);
    fstp(app, cpu, cpu.edx + 0x7b0);
    leave();
}

void nothingAtStartToo(App* app, CPU& cpu)
{
    emptyHook(app, cpu);
}

/* 0x404c50: the catch-up speed (0x56ebb0): 20.12 while fewer players
 * (0x6fd518) than 0x6fd520, else 11.18. */
void catchUpSpeed(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = at(app, 0x6fd518);
    put(app, 0x56ebb0, sreg32(cpu.edx) < sreg32(at(app, 0x6fd520)) ? 0x41a0ee49 : 0x4132cfdf);
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x404c80: the leader among the players' cars (0x5efa48, [0x5efda4]), asked
 * by the last of them (+0x1f4): the one of the most laps (+8; a finished
 * one's 1) to 0x56ebb8; then by the other racers (0x5efb88, [0x5efd94]) how
 * many have more laps -- 0x56ebb4 1 some, 2 all, 0 none. */
void findLeader(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edi = at(app, 0x5efd94);
    cpu.edx = at(app, 0x5efda4);
    cpu.ebx = at(app, cpu.eax + 0x1f4);
    --cpu.edx;
    cpu.ecx = 0;
    if (cpu.edx == cpu.ebx)
    {
        for (cpu.edx ^= cpu.ebx; sreg32(cpu.edx) < sreg32(at(app, 0x5efda4)); ++cpu.edx)
        {
            cpu.eax = at(app, cpu.edx * 4 + 0x5efa48);
            if (byte(app, cpu.eax + 0x200) & 0x20)
                put(app, cpu.eax + 8, 1);
            cpu.ebx = at(app, cpu.eax + 8);
            if (!(sreg32(cpu.ecx) > sreg32(cpu.ebx)))
            {
                cpu.ecx = cpu.ebx;
                put(app, 0x56ebb8, cpu.eax);
            }
        }
    }
    cpu.esi = 0;
    if (cpu.edi != 0)
    {
        cpu.eax = 0;
        cpu.edx = 0;
        for (; sreg32(cpu.eax) < sreg32(cpu.edi); ++cpu.eax)
        {
            cpu.ebx = at(app, cpu.eax * 4 + 0x5efb88);
            if (sreg32(cpu.ecx) < sreg32(at(app, cpu.ebx + 8)))
            {
                cpu.esi = 1;
                cpu.edx += cpu.esi;
            }
        }
        if (cpu.edx == cpu.edi)
            cpu.esi = 2;
    }
    put(app, 0x5efd94, cpu.edi);
    put(app, 0x56ebb4, cpu.esi);
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

}  // namespace airace
}  // namespace nfs3hp
