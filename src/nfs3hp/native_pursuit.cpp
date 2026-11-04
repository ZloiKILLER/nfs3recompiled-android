#include "native_guest.h"

/* Hot Pursuit's police, native whole: the cops' and the speeders' speeds in
 * a pursuit and the cop's state from frame to frame, the radio's phrases and
 * the speeches of the dispatcher and the cops ("a speeder, red Lamborghini,
 * 150+, heading for the town", "ticket", "arrest", "roadblock", "patrol",
 * "donuts", ...), and the racers' speech banks.  disassemble_nfs3hp.py binds
 * each address to the function of that name here (PURSUIT_NATIVES), and the
 * generated code calls it by name.  tools/game_checks holds each against its
 * generated code (pursuit.inc).
 *
 * A car's record (0x9a4 bytes and more) as these read it: +8 its progress
 * along the track (the leader's against it, the cop's against what it had),
 * +0x1c its node on the track, +0x98 its place, +0xa4 its speed's vector,
 * +0x10c, +0x114 its width, +0x164 the race's stage, +0x184 a slice
 * it is to be put back on, +0x1f0 its index, +0x200 its flags (1 a human's,
 * 4 a cop's, 0x20 the player's), +0x204/+0x208 its laps, +0x220 its choice
 * (+0x14 the car's model, +0x30 its pursuit class -- an index of the class
 * records at 0x552274, 0x18 bytes: +4 a flag, +8 how keen its cops are),
 * +0x2bc, +0x2c4 the player's kind of control, +0x509 the brake, +0x528 its
 * handling (+0x1cc its top speed), +0x5f4 its going backwards, +0x620 and
 * +0x621 the pursuit's flags (0x20 of +0x620: it is chased; of +0x621: 0x10
 * against the traffic, 0x40 the cop may reset it, 0x80 it is on the radio),
 * +0x624 the car a cop chases, +0x628 its pursuit record (+0x1c, +0x2c),
 * +0x638 its lane, +0x650 a countdown, +0x654, +0x660, +0x6d8 the car it
 * follows, +0x6e0 the cop's state (0 to 7, a switch's), +0x6e4 its chase's
 * count, +0x6ec, +0x700 its pace, +0x704/+0x708 the frame and the place of
 * its last progress, +0x710, +0x714 its radio's voice, +0x7b0 its target
 * speed (signed as it goes: the sign bit of +0x7b3 flipped where the track
 * runs backwards [0x6fd4c0] or the car goes against it), +0x7b4, +0x7b8,
 * +0x7bc its speed's factor, +0x7c0 its speed, +0x7c4 the speed's size,
 * +0x7d4.  The track's nodes are 0x24 bytes at [0x5dd948], [0x5dd940] of
 * them: +0x13 the lanes on either side (a nibble each), +0x17 and +0x1b the
 * road's widths either side, +0x11..+0x13 its heading's bytes.
 *
 * The pursuit's globals: 0x56eef0..0x56ef6e the chases (three: the cops'
 * cars 0x56ef38, their stages 0x56ef44, their counts 0x56ef5c, flags
 * 0x56ef68/0x56ef6e, the scores 0x56eef0, 0x56eefc, 0x56ef08, 0x56ef20,
 * 0x56ef14..0x56ef1c), 0x56eeb4..0x56eebc the lanes' times, 0x56ef3c the car
 * the cops chase, 0x56ef48 the chase's level, 0x56ebb4/0x56ebb8 the race's
 * leader and its kind.  The radio's: 0x56ec90 its phrases' names (8 bytes
 * each), 0x56ee28 each car's speech bank, 0x56ee68 and 0x5524cc each voice's
 * frames since it spoke, 0x5524c8 the frames since anyone did, 0x5524ec the
 * voice that spoke last, 0x5524f0 a car's index, 0x5524f4 a "help" pending,
 * 0x5524f8 and 0x552520 each voice's last place and its prefix, 0x552548 a
 * table of floats by a random number (0x554f10..0x554f18 its generator's
 * state), 0x7dd378 the cops' radio's distance.
 *
 * Written as their code is: each push and pop to the guest's stack, each call
 * with the return address's slot, the frame's locals in the guest's memory,
 * the registers as the code leaves them, the x87's stack pushed and popped as
 * the x87 does, its arithmetic through x86::FPU (add, sub, mul, div,
 * toInteger) in the code's order of operands, a compare's status word left in
 * ax as fnstsw leaves it.  The switches go by the targets nfs3.exe's tables
 * hold (0x409be0, 0x40a3f0, 0x40a55c, 0x40aab0).  Every other function of the
 * game's is called by address. */
namespace nfs3hp
{
namespace pursuit
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
using sreg16 = x86::sreg16;

// The race's: its type (3 Hot Pursuit), its mode (1 a split screen), the
// track backwards, the frame counter, the pursuit's level and switches.
constexpr reg32 kRaceType = 0x6fd3b8, kPlayMode = 0x6fd3b0, kBackwards = 0x6fd4c0, kRaceFrame = 0x7d3684;
constexpr reg32 kNodeCount = 0x5dd940, kNodes = 0x5dd948;
// The class records of the pursuit (0x18 bytes), by the car's +0x30.
constexpr reg32 kClassKind = 0x552274, kClassFlag = 0x552278, kClassKeen = 0x55227c;
// What is called by address: the pursuit's others (another file's), the
// game's.
constexpr reg32 kChaseKind = 0x404610, kNodeSpeed = 0x40ed40, kTrackPace = 0x40ee80, kSliceSpeed = 0x40e080;
constexpr reg32 kCopNear = 0x40f050, kNodeAhead = 0x40e160, kNodeLane = 0x40e0c0, kInReach = 0x40f000;
constexpr reg32 kTakeOver = 0x40f100, kCopActive = 0x40df10, kResetHeld = 0x404b90, kCopSteer = 0x40dd60;
constexpr reg32 kCopAvoid = 0x404840, kCopDrive = 0x40ddb0, kCopFinish = 0x40de30, kCopOut = 0x401500;
constexpr reg32 kPullOver = 0x40e350, kCopStop = 0x40ce70, kNodeFree = 0x405c80, kPlacesLeft = 0x40ef10;
constexpr reg32 kSpeedLimit = 0x40cd80;
constexpr reg32 kResetCar = 0x429100, kDebugText = 0x42d520, kTrace = 0x42d510, kMonoPrint = 0x4e08f0;
constexpr reg32 kRadar = 0x4c8040, kSpeechLine = 0x4c85a0, kNearestPlace = 0x4c8550, kOtherPlace = 0x4c8470;
constexpr reg32 kTicketed = 0x4c7fd0, kRandom = 0x4e0d88, kStrncpy = 0x4e0e30, kStrstr = 0x4e0820;
constexpr reg32 kMemset = 0x4e0640, kChop = 0x4dfd56;
constexpr reg32 kCarName = 0x439250, kCarIsPolice = 0x4396b0, kCarFolder = 0x4392b0, kCarColour = 0x439780;
constexpr reg32 kCarColours = 0x439790, kEndPath = 0x4529a0, kVoiceBankLoad = 0x413750;
constexpr reg32 kNearestColour = 0x431280, kPlaceOnNode = 0x428d60, kInTunnel = 0x420bf0, kSettle = 0x428970;

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

reg16 half(App* app, reg32 a) { return app->getMemory<reg16>(a); }
sreg32 s(reg32 v) { return sreg32(v); }

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

/* The pushes of the prologue, and the pops of the epilogue in the other
 * order with the ret. */
void save(App* app, CPU& cpu, std::initializer_list<reg32> values)
{
    for (const reg32 value : values)
        push(app, cpu, value);
}

void restore(App* app, CPU& cpu, std::initializer_list<reg32*> registers)
{
    for (reg32* r : registers)
        *r = pop(app, cpu);
}

void ret(App* app, CPU& cpu, std::initializer_list<reg32*> registers)
{
    restore(app, cpu, registers);
    cpu.esp += 4;
}

// The x87's, as the code has them.
Float f32(App* app, reg32 a) { return Float(app->getMemory<float>(a)); }
Float f64(App* app, reg32 a) { return Float(app->getMemory<double>(a)); }

void fld(CPU& cpu, const Float& value) { cpu.fpu.push(value); }
void fild(App* app, CPU& cpu, reg32 a) { cpu.fpu.push(Float(sreg32(at(app, a)))); }

void fst(App* app, CPU& cpu, reg32 a) { app->getMemory<float>(a) = float(cpu.fpu.st(0)); }

void fstp(App* app, CPU& cpu, reg32 a)
{
    fst(app, cpu, a);
    cpu.fpu.pop();
}

void fst64(App* app, CPU& cpu, reg32 a) { app->getMemory<double>(a) = double(cpu.fpu.st(0)); }

void fstp64(App* app, CPU& cpu, reg32 a)
{
    fst64(app, cpu, a);
    cpu.fpu.pop();
}

void fxch(CPU& cpu, int i)
{
    const Float t = cpu.fpu.st(0);
    cpu.fpu.st(0) = cpu.fpu.st(i);
    cpu.fpu.st(i) = t;
}

/* fstp st(1): st(1) = st(0), popped. */
void fstp1(CPU& cpu)
{
    cpu.fpu.st(1) = cpu.fpu.st(0);
    cpu.fpu.pop();
}

// st(0) op= m (fadd m32 and the like).
void fadd(CPU& cpu, const Float& m) { cpu.fpu.st(0) = fp::add(cpu.fpu.st(0), m); }
void fsub(CPU& cpu, const Float& m) { cpu.fpu.st(0) = fp::sub(cpu.fpu.st(0), m); }
void fmul(CPU& cpu, const Float& m) { cpu.fpu.st(0) = fp::mul(cpu.fpu.st(0), m); }

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

void fdivp(CPU& cpu, int i)
{
    cpu.fpu.st(i) = fp::div(cpu.fpu.st(i), cpu.fpu.st(0));
    cpu.fpu.pop();
}

void fchs(CPU& cpu) { cpu.fpu.st(0) = -cpu.fpu.st(0); }

/* sub esp, 4; fistp dword [esp]; pop eax: st(0) as an integer into eax. */
void fistpToEax(App* app, CPU& cpu)
{
    cpu.esp -= 4;
    put(app, cpu.esp, reg32(fp::toInteger<sreg32>(cpu.fpu.st(0))));
    cpu.fpu.pop();
    cpu.eax = pop(app, cpu);
}

/* fcomp m / fcompp; fnstsw ax; sahf: st(0) compared with m (st(1)), popped
 * (both), the status word in ax; CF and ZF as sahf sets them. */
struct Flags
{
    bool cf, zf;
    bool below() const { return cf; }
    bool belowEqual() const { return cf || zf; }
    bool above() const { return !cf && !zf; }
    bool aboveEqual() const { return !cf; }
};

Flags fcomp(CPU& cpu, const Float& m)
{
    cpu.fpu.compare(cpu.fpu.st(0), m);
    cpu.ax = cpu.fpu.status.word;
    cpu.fpu.pop();
    return {(cpu.ah & 1) != 0, (cpu.ah & 0x40) != 0};
}

Flags fcompp(CPU& cpu)
{
    cpu.fpu.compare(cpu.fpu.st(0), cpu.fpu.st(1));
    cpu.ax = cpu.fpu.status.word;
    cpu.fpu.pop();
    cpu.fpu.pop();
    return {(cpu.ah & 1) != 0, (cpu.ah & 0x40) != 0};
}

/* The sign of a float of the car's flipped: mov r8, [car + 0x7b3]; xor r8,
 * 0x80; mov [car + 0x7b3], r8 -- through the byte register the code uses. */
void flipSign(App* app, reg32 car, reg8& r)
{
    r = byte(app, car + 0x7b3);
    r ^= 0x80;
    putByte(app, car + 0x7b3, r);
}

/* The node `node`'s record: lea r, [node*8]; add r, node; shl r, 2; add [0x5dd948]. */
reg32 nodeRecord(App* app, reg32 node)
{
    return (node * 8 + node) * 4 + at(app, kNodes);
}

/* The class's record of the car's pursuit class: mov r, [car + 0x220];
 * mov r2, [r + 0x30]; lea r, [r2*4]; sub r, r2 -- the record's index * 3,
 * which the code scales by 8. */
reg32 classIndex3(App* app, reg32 car, reg32& r2)
{
    const reg32 choice = at(app, car + 0x220);
    r2 = at(app, choice + 0x30);
    return r2 * 4 - r2;
}

}  // namespace

void raceStartHook(App* app, CPU& cpu);
void raceEndHook(App* app, CPU& cpu);
void roadsideTarget(App* app, CPU& cpu);
void countLaneTime(App* app, CPU& cpu);
void pulledOverTarget(App* app, CPU& cpu);
void classSpeedFactor(App* app, CPU& cpu);
void unitFactor(App* app, CPU& cpu);
void radarFactor(App* app, CPU& cpu);
void unitFactorToo(App* app, CPU& cpu);
void leaderFactor(App* app, CPU& cpu);
void nodeTargetSpeed(App* app, CPU& cpu);
void markChased(App* app, CPU& cpu);
void targetSpeed(App* app, CPU& cpu);
void targetSpeedOnly(App* app, CPU& cpu);
void matchChasedSpeed(App* app, CPU& cpu);
void scoreCloseCops(App* app, CPU& cpu);
void growLaneTimes(App* app, CPU& cpu);
void scoreLane(App* app, CPU& cpu);
void scoreChases(App* app, CPU& cpu);
void radioChaseCount(App* app, CPU& cpu);
void scoreSpeeds(App* app, CPU& cpu);
void alwaysOne(App* app, CPU& cpu);
void takeOverCars(App* app, CPU& cpu);
void startReset(App* app, CPU& cpu);
void checkProgress(App* app, CPU& cpu);
void copStep(App* app, CPU& cpu);
void loadRacerBanks(App* app, CPU& cpu);
void countRadioFrames(App* app, CPU& cpu);
void radioNoise(App* app, CPU& cpu);
void sayPhrase(App* app, CPU& cpu);
void sayCarPhrase(App* app, CPU& cpu);
void namePhrase(App* app, CPU& cpu);
void namePhrases(App* app, CPU& cpu);
void radioFree(App* app, CPU& cpu);
void radioNothing(App* app, CPU& cpu);
void quietVoice(App* app, CPU& cpu);
void sayVoicePhrase(App* app, CPU& cpu);
void sayCallSign(App* app, CPU& cpu);
void sayPhraseOnly(App* app, CPU& cpu);
void sayUnit(App* app, CPU& cpu);
void sayDescription(App* app, CPU& cpu);
void radioNothingToo(App* app, CPU& cpu);
void sayLocation(App* app, CPU& cpu);
void saySpeed(App* app, CPU& cpu);
void radioSpotted(App* app, CPU& cpu);
void radioPursuit(App* app, CPU& cpu);
void radioLost(App* app, CPU& cpu);
void radioBackup(App* app, CPU& cpu);
void radioPatrol(App* app, CPU& cpu);
void radioRoadblock(App* app, CPU& cpu);
void radioNothingThree(App* app, CPU& cpu);
void radioTicket(App* app, CPU& cpu);
void radioArrest(App* app, CPU& cpu);
void radioStopped(App* app, CPU& cpu);
void radioAmbulance(App* app, CPU& cpu);
void radioSpikes(App* app, CPU& cpu);
void radioHelp(App* app, CPU& cpu);
void radioResume(App* app, CPU& cpu);
void radioAbort(App* app, CPU& cpu);
void resetOnTrack(App* app, CPU& cpu);
void copRestart(App* app, CPU& cpu);
void topSpeedFactor(App* app, CPU& cpu);
void followChased(App* app, CPU& cpu);
void raceEndHookToo(App* app, CPU& cpu);
void raceStartHookToo(App* app, CPU& cpu);
void followCar(App* app, CPU& cpu);
void countdownDone(App* app, CPU& cpu);
void trafficTarget(App* app, CPU& cpu);
void trafficStep(App* app, CPU& cpu);

/* 0x408ad0, 0x408ae0, 0x40c140, 0x40c150: the pursuit's hooks at the race's
 * start and end (the race's set-up 0x4c45f0 and its end 0x4c4740 call them),
 * empty -- push ebp, mov ebp, esp, pop ebp. */
void raceStartHook(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

void raceEndHook(App* app, CPU& cpu)
{
    raceStartHook(app, cpu);
}

void raceEndHookToo(App* app, CPU& cpu)
{
    raceStartHook(app, cpu);
}

void raceStartHookToo(App* app, CPU& cpu)
{
    raceStartHook(app, cpu);
}

/* 0x408b30: the target speed [+0x7b0] of the car eax pulled over: 0 where it
 * is in one of its node's outer lanes (6 - the left nibble of +0x13, or the
 * right nibble + 7), else 11; its sign flipped where the track runs
 * backwards, and where the car goes backwards ([+0x5f4]). */
void roadsideTarget(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ebx, cpu.ecx, cpu.edx, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.ecx = at(app, cpu.eax + 0x1c);
    cpu.edx = cpu.ecx * 8;
    cpu.edx += cpu.ecx;
    cpu.ecx = at(app, kNodes);
    cpu.edx <<= 2;
    cpu.edx += cpu.ecx;
    cpu.ecx = 0;
    cpu.cl = byte(app, cpu.edx + 0x13);
    cpu.ecx = reg32(s(cpu.ecx) >> 4);
    cpu.ebx = 6;
    cpu.ecx &= 0xf;
    cpu.ebx -= cpu.ecx;
    cpu.ecx = cpu.ebx;
    cpu.ebx = at(app, cpu.eax + 0x638);
    bool outer = cpu.ecx == cpu.ebx;
    if (!outer)
    {
        cpu.dl = byte(app, cpu.edx + 0x13);
        cpu.dl &= 0xf;
        cpu.edx &= 0xff;
        cpu.edx += 7;
        outer = cpu.edx == cpu.ebx;
    }
    put(app, cpu.eax + 0x7b0, outer ? 0 : 0x41300000);
    if (at(app, kBackwards) != 0)
        flipSign(app, cpu.eax, cpu.dl);
    if (at(app, cpu.eax + 0x5f4) != 0)
        flipSign(app, cpu.eax, cpu.dh);
    ret(app, cpu, {&cpu.ebp, &cpu.edx, &cpu.ecx, &cpu.ebx});
}

/* 0x408bd0: the lanes' times: 150 more to the outer lanes' [0x56eeb8] where
 * the car eax is in one of its node's outer lanes, else to the left's
 * [0x56eeb4] (lanes under 7) or the right's [0x56eebc]. */
void countLaneTime(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ebx, cpu.ecx, cpu.edx, cpu.ebp});
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
    cpu.ecx = reg32(s(cpu.ecx) >> 4);
    cpu.ebx = 6;
    cpu.ecx &= 0xf;
    cpu.ebx -= cpu.ecx;
    cpu.ecx = cpu.ebx;
    cpu.ebx = at(app, cpu.edx + 0x638);
    bool outer = cpu.ecx == cpu.ebx;
    if (!outer)
    {
        cpu.al = byte(app, cpu.eax + 0x13);
        cpu.al &= 0xf;
        cpu.eax &= 0xff;
        cpu.eax += 7;
        outer = cpu.eax == cpu.ebx;
    }
    const reg32 counter = outer ? 0x56eeb8 : s(cpu.ebx) < 7 ? 0x56eeb4 : 0x56eebc;
    put(app, counter, at(app, counter) + 0x96);
    ret(app, cpu, {&cpu.ebp, &cpu.edx, &cpu.ecx, &cpu.ebx});
}

/* 0x408c50: the target speed of the car eax stopped by the cops: 11 only
 * within the track's first 20 nodes (which way it runs) with its sides (its
 * +0x7b8 less and more its width +0x114, times 0x5363dc) within the road's
 * either side at its node (the lanes' nibbles of +0x13 times the widths
 * +0x17 and +0x1b, 2^15 each); else 0.  Its sign flipped where the track
 * runs backwards. */
void pulledOverTarget(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x18;
    cpu.edx = cpu.eax;
    cpu.ecx = at(app, kBackwards);
    cpu.eax = at(app, cpu.eax + 0x1c);
    if (cpu.ecx != 0)
    {
        cpu.ecx = at(app, kNodeCount);
        cpu.ecx -= 0x14;
        cpu.esi = s(cpu.eax) < s(cpu.ecx) ? 1 : 0;
    }
    else
    {
        if (s(cpu.eax) > 0x14)
            cpu.ecx = 1;
        cpu.esi = cpu.ecx;
    }
    x86::FPU& fpu = cpu.fpu;
    fld(cpu, f32(app, cpu.edx + 0x7b8));
    fld(cpu, fpu.st(0));
    cpu.ecx = cpu.eax;
    cpu.ebx = 0;
    cpu.eax <<= 3;
    fxch(cpu, 1);
    fsub(cpu, f32(app, cpu.edx + 0x114));
    fxch(cpu, 1);
    fadd(cpu, f32(app, cpu.edx + 0x114));
    cpu.ecx += cpu.eax;
    cpu.eax = at(app, kNodes);
    cpu.ecx <<= 2;
    fxch(cpu, 1);
    fstp(app, cpu, cpu.ebp - 8);
    cpu.bl = byte(app, cpu.ecx + cpu.eax + 0x13);
    fstp(app, cpu, cpu.ebp - 4);
    cpu.ebx = reg32(s(cpu.ebx) >> 4);
    cpu.al = byte(app, cpu.ecx + cpu.eax + 0x17);
    cpu.ebx &= 0xf;
    cpu.eax &= 0xff;
    cpu.eax <<= 0xf;
    cpu.ebx = cpu.ebx * cpu.eax;
    fld(cpu, f32(app, cpu.ebp - 8));
    fmul(cpu, f64(app, 0x5363dc));
    fst64(app, cpu, cpu.ebp - 0x10);
    cpu.ebx = 0 - cpu.ebx;
    fistpToEax(app, cpu);
    bool stop = s(cpu.eax) < s(cpu.ebx);
    if (!stop)
    {
        cpu.eax = at(app, kNodes);
        cpu.ebx = 0;
        cpu.bl = byte(app, cpu.ecx + cpu.eax + 0x1b);
        cpu.ebx <<= 0xf;
        cpu.al = byte(app, cpu.ecx + cpu.eax + 0x13);
        cpu.al &= 0xf;
        cpu.eax &= 0xff;
        cpu.ebx = cpu.ebx * cpu.eax;
        fld(cpu, f64(app, cpu.ebp - 0x10));
        fistpToEax(app, cpu);
        stop = s(cpu.eax) > s(cpu.ebx);
    }
    if (!stop)
    {
        cpu.eax = at(app, kNodes);
        cpu.ebx = 0;
        cpu.bl = byte(app, cpu.ecx + cpu.eax + 0x13);
        cpu.ebx = reg32(s(cpu.ebx) >> 4);
        cpu.al = byte(app, cpu.ecx + cpu.eax + 0x17);
        cpu.ebx &= 0xf;
        cpu.eax &= 0xff;
        cpu.eax <<= 0xf;
        cpu.ebx = cpu.ebx * cpu.eax;
        fld(cpu, f32(app, cpu.ebp - 4));
        fmul(cpu, f64(app, 0x5363dc));
        fst64(app, cpu, cpu.ebp - 0x18);
        cpu.ebx = 0 - cpu.ebx;
        fistpToEax(app, cpu);
        stop = s(cpu.eax) < s(cpu.ebx);
    }
    if (!stop)
    {
        cpu.eax = at(app, kNodes);
        cpu.eax += cpu.ecx;
        cpu.ecx = 0;
        cpu.cl = byte(app, cpu.eax + 0x1b);
        cpu.al = byte(app, cpu.eax + 0x13);
        cpu.ecx <<= 0xf;
        cpu.al &= 0xf;
        cpu.eax &= 0xff;
        cpu.ecx = cpu.ecx * cpu.eax;
        fld(cpu, f64(app, cpu.ebp - 0x18));
        fistpToEax(app, cpu);
        stop = s(cpu.eax) > s(cpu.ecx) || cpu.esi != 0;
    }
    put(app, cpu.edx + 0x7b0, stop ? 0 : 0x41300000);
    if (at(app, kBackwards) != 0)
        flipSign(app, cpu.edx, cpu.ah);
    cpu.esp = cpu.ebp;
    ret(app, cpu, {&cpu.ebp, &cpu.esi, &cpu.edx, &cpu.ecx, &cpu.ebx});
}

/* 0x408dc0: the factor of the car eax's speed by its pursuit's class, onto
 * the x87's stack: 1 in a single player's race of type 2; else 1 times
 * [0x5363e4] or [0x5363e8] by what kind its chase is (0x404610: 0 or 1),
 * times 0.x every second and fourth quarter of 512 frames where its class has
 * no flag, then by its class's keenness -- the chased car's (+0x620 & 0x20)
 * as 1 + keen * 0x5363fc, else 0x5363f4 for 0 and 0x536404 for 1. */
void classSpeedFactor(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    save(app, cpu, {cpu.ebx, cpu.ecx, cpu.edx, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x10;
    cpu.edx = cpu.eax;
    cpu.ecx = 0x3f800000;
    cpu.ebx = at(app, kRaceType);
    put(app, cpu.ebp - 8, cpu.ecx);
    if (cpu.ebx == 2 && s(at(app, kPlayMode)) < 1)
        put(app, cpu.ebp - 4, cpu.ecx);
    else
    {
        cpu.eax = cpu.edx;
        if (!byAddress(app, cpu, kChaseKind))
            return;
        if (cpu.eax < 1)
        {
            fld(cpu, f32(app, cpu.ebp - 8));
            fmul(cpu, f32(app, 0x5363e4));
            fstp(app, cpu, cpu.ebp - 8);
        }
        else if (cpu.eax == 1)
        {
            fld(cpu, f32(app, cpu.ebp - 8));
            fmul(cpu, f32(app, 0x5363e8));
            fstp(app, cpu, cpu.ebp - 8);
        }
        cpu.eax = at(app, cpu.edx + 0x220);
        cpu.ecx = at(app, cpu.eax + 0x30);
        cpu.eax = cpu.ecx * 4;
        cpu.ebx = at(app, kRaceFrame);
        cpu.eax -= cpu.ecx;
        cpu.ebx = reg32(s(cpu.ebx) >> 7);
        cpu.ecx = at(app, cpu.eax * 8 + kClassFlag);
        cpu.ebx &= 3;
        if (cpu.ecx == 0)
        {
            if (cpu.ebx == 1)
            {
                fld(cpu, f32(app, cpu.ebp - 8));
                fld(cpu, fpu.st(0));
                fmul(cpu, f64(app, 0x5363ec));
                fstp1(cpu);
                fstp(app, cpu, cpu.ebp - 8);
            }
            if (cpu.ebx == 3)
            {
                fld(cpu, f32(app, cpu.ebp - 8));
                fld(cpu, fpu.st(0));
                fmul(cpu, f64(app, 0x5363f4));
                fstp1(cpu);
                fstp(app, cpu, cpu.ebp - 8);
            }
        }
        if ((byte(app, cpu.edx + 0x620) & 0x20) != 0)
        {
            cpu.eax = at(app, cpu.edx + 0x220);
            cpu.edx = at(app, cpu.eax + 0x30);
            cpu.eax = cpu.edx * 4;
            cpu.eax -= cpu.edx;
            fild(app, cpu, cpu.eax * 8 + kClassKeen);
            fmul(cpu, f64(app, 0x5363fc));
            fadd(cpu, f64(app, 0x5363f4));
            fmul(cpu, f32(app, cpu.ebp - 8));
            fstp(app, cpu, cpu.ebp - 8);
        }
        else
        {
            cpu.eax = at(app, cpu.edx + 0x220);
            cpu.edx = at(app, cpu.eax + 0x30);
            cpu.eax = cpu.edx * 4;
            cpu.eax -= cpu.edx;
            cpu.eax <<= 3;
            cpu.ebx = at(app, cpu.eax + kClassKeen);
            if (cpu.ebx == 0 || cpu.ebx == 1)
            {
                fld(cpu, f32(app, cpu.ebp - 8));
                fld(cpu, fpu.st(0));
                fmul(cpu, f64(app, cpu.ebx == 0 ? 0x5363f4 : 0x536404));
                fstp1(cpu);
                fstp(app, cpu, cpu.ebp - 8);
            }
        }
        cpu.eax = at(app, cpu.ebp - 8);
        put(app, cpu.ebp - 4, cpu.eax);
    }
    fld(cpu, f32(app, cpu.ebp - 4));
    cpu.esp = cpu.ebp;
    ret(app, cpu, {&cpu.ebp, &cpu.edx, &cpu.ecx, &cpu.ebx});
}

/* 0x408f00 and 0x409090: a factor of 1 onto the x87's stack (fld1). */
void unitFactor(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    fld(cpu, Float(1.0));
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

void unitFactorToo(App* app, CPU& cpu)
{
    unitFactor(app, cpu);
}

/* 0x408f10: the factor of the car eax's speed by the radar's word on it
 * (0x4c8040 into the frame's record: how far the car ahead is, its side, its
 * kind), onto the x87's stack -- 1 but where the race is a network one
 * [0x6fd50c] and the car is the network's to drive: a car well behind the one
 * ahead (past 20 nodes on the last lap, 50 else) is put back on the track
 * where the radar says (0x40bd80); else, but for a car the player steers
 * with a pad (+0x2c4 3), 1 + (2 - the distance) * [0x53640c], no less than
 * 0 -- the brakes set and the car let back with the traffic where it is
 * close behind and slow, the car marked against the traffic past 30. */
void radarFactor(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x20;
    cpu.ecx = cpu.eax;
    cpu.edx = 0x3f800000;
    cpu.ebx = at(app, 0x6fd50c);
    put(app, cpu.ebp - 8, cpu.edx);
    const auto done = [&] {
        fld(cpu, f32(app, cpu.ebp - 8));
        cpu.esp = cpu.ebp;
        ret(app, cpu, {&cpu.ebp, &cpu.edi, &cpu.esi, &cpu.edx, &cpu.ecx, &cpu.ebx});
    };
    if (cpu.ebx == 0)
        return done();
    if (half(app, 0x7a22da) != 0)
    {
        cpu.esi = at(app, 0x6fd518);
        if (s(cpu.esi) <= s(at(app, cpu.eax + 0x1f0)))
            return done();
    }
    cpu.eax = cpu.ebp - 0x20;
    cpu.edx = at(app, cpu.ecx + 0x1f0);
    if (!byAddress(app, cpu, kRadar))
        return;
    if (cpu.eax == 0)
        return done();
    cpu.eax = at(app, 0x6fd4b4);
    cpu.edi = at(app, cpu.ecx + 0x208);
    cpu.edx = at(app, cpu.ecx + 0x204);
    cpu.eax -= 1;
    cpu.edx -= cpu.edi;
    cpu.eax = cpu.edx == cpu.eax ? 0x14 : 0x32;
    cpu.edx = at(app, cpu.ebp - 0x16);
    cpu.edx = reg32(s(cpu.edx) >> 0x10);
    cpu.ebx = cpu.edx;
    cpu.ebx = 0 - cpu.ebx;
    if (s(cpu.ebx) > s(cpu.eax))
    {
        if (at(app, cpu.ebp - 0x10) != 1)
            return done();
        cpu.edx = cpu.ebp - 0x20;
        cpu.eax = cpu.ecx;
        if (!direct(app, cpu, resetOnTrack))
            return;
        return done();
    }
    if (at(app, cpu.ecx + 0x2c4) == 3)
    {
        cpu.edi = at(app, 0x6fd518);  // cmp edi, [ecx + 0x1f0]: its flags never used
        return done();
    }
    cpu.esi = at(app, 0x6fd518);
    if (s(cpu.esi) > s(at(app, cpu.ecx + 0x1f0)))
    {
        if (sreg16(half(app, cpu.ebp - 0x14)) <= 0)
            cpu.edx = cpu.ebx;
        if (s(cpu.edx) < 2 && s(at(app, cpu.ebp - 0x18)) < 0x40400000)
        {
            putByte(app, cpu.ecx + 0x509, 0xff);
            cpu.ah = byte(app, cpu.ecx + 0x621);
            put(app, cpu.ecx + 0x7b0, 0);
            cpu.ah &= 0xef;
            putByte(app, cpu.ecx + 0x621, cpu.ah);
        }
        else
        {
            cpu.edi = at(app, cpu.ebp - 0x14);
            if (sreg16(cpu.di) > 0x1e)
                putByte(app, cpu.ecx + 0x621, byte(app, cpu.ecx + 0x621) | 0x10);
            else if (sreg16(cpu.di) < 2)
            {
                cpu.dh = byte(app, cpu.ecx + 0x621);
                putByte(app, cpu.ecx + 0x509, 0);
                cpu.dh &= 0xef;
                putByte(app, cpu.ecx + 0x621, cpu.dh);
            }
        }
    }
    cpu.eax = at(app, cpu.ebp - 0x16);
    cpu.eax = reg32(s(cpu.eax) >> 0x10);
    cpu.eax = 0 - cpu.eax;
    cpu.eax += 2;
    put(app, cpu.ebp - 4, cpu.eax);
    fild(app, cpu, cpu.ebp - 4);
    fmul(cpu, f32(app, 0x53640c));
    fadd(cpu, f32(app, cpu.ebp - 8));
    fld(cpu, Float(0.0));
    fxch(cpu, 1);
    fstp(app, cpu, cpu.ebp - 8);
    const Flags f = fcomp(cpu, f32(app, cpu.ebp - 8));
    cpu.eax = f.aboveEqual() ? 0 : at(app, cpu.ebp - 8);
    put(app, cpu.ebp - 0xc, cpu.eax);
    cpu.eax = at(app, cpu.ebp - 0xc);
    put(app, cpu.ebp - 8, cpu.eax);
    done();
}

/* 0x4090a0: the factor of the car eax's speed by its place against the race's
 * leader [0x56ebb8] onto the x87's stack -- 1 without one; by the leader's
 * kind [0x56ebb4]: 0 (a car ahead by 5, 10 or 15 places, by the race's type
 * and the switches 0x6fd470 and 0x6fd4f0) 1.15 or 1.1 to catch up, 1 else;
 * 1 (behind by 100, 70 or 50) 1, 0.85 else; 2 by the switches 0.95, 0.85,
 * 0.8, 0.9.  No less than 1 in a race of type 2 at 0x6fd4ac 8, no more than
 * 1 in a race of more than one player. */
void leaderFactor(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x28;
    cpu.ebx = at(app, 0x56ebb8);
    cpu.edx = cpu.eax;
    if (cpu.ebx == 0)
        put(app, cpu.ebp - 8, 0x3f800000);
    else
    {
        cpu.eax = at(app, 0x56ebb4);
        if (cpu.eax == 0)
        {
            cpu.ecx = at(app, kRaceType);
            if ((cpu.ecx == 1 || cpu.ecx == 2) && at(app, 0x6fd470) == 1)
                cpu.eax = 5;
            else if (at(app, 0x6fd4f0) == 1)
                cpu.eax = 0xa;
            else
                cpu.eax = 0xf;
            cpu.ecx = at(app, cpu.edx + 8);
            cpu.esi = at(app, cpu.ebx + 8);
            cpu.eax += cpu.ecx;
            if (s(cpu.eax) >= s(cpu.esi))
                put(app, cpu.ebp - 4, 0x3f800000);
            else
            {
                cpu.edi = at(app, kRaceType);
                put(app, cpu.ebp - 4, cpu.edi == 1 || cpu.edi == 2 ? 0x3f933333 : 0x3f8ccccd);
            }
        }
        else if (cpu.eax != 2)
        {
            cpu.edi = at(app, kRaceType);
            if ((cpu.edi == 1 || cpu.edi == 2) && at(app, 0x6fd470) == 1)
                cpu.eax = 0x64;
            else if (at(app, 0x6fd4f0) == 1)
                cpu.eax = 0x46;
            else
                cpu.eax = 0x32;
            cpu.edi = at(app, cpu.ebx + 8);
            cpu.ecx = at(app, cpu.edx + 8);
            cpu.eax += cpu.edi;
            put(app, cpu.ebp - 4, s(cpu.eax) <= s(cpu.ecx) ? 0x3f59999a : 0x3f800000);
        }
        else if (at(app, 0x6fd4f0) == 1)
            put(app, cpu.ebp - 4, 0x3f733333);
        else if (at(app, kRaceType) == 3)
            put(app, cpu.ebp - 4, 0x3f59999a);
        else if (s(at(app, cpu.edx + 0x2bc)) >= 6 && at(app, kPlayMode) == 0)
            put(app, cpu.ebp - 4, 0x3f4ccccd);
        else
            put(app, cpu.ebp - 4, 0x3f666666);
        if (at(app, kRaceType) == 2 && at(app, 0x6fd4ac) == 8)
        {
            fld(cpu, f32(app, cpu.ebp - 4));
            fld(cpu, Float(1.0));
            fxch(cpu, 1);
            fstp64(app, cpu, cpu.ebp - 0x10);
            if (!fcomp(cpu, f64(app, cpu.ebp - 0x10)).belowEqual())
            {
                cpu.eax = 0;
                cpu.edx = 0x3ff00000;
                put(app, cpu.ebp - 0x28, cpu.eax);
                put(app, cpu.ebp - 0x24, cpu.edx);
            }
            else
            {
                cpu.eax = at(app, cpu.ebp - 0x10);
                put(app, cpu.ebp - 0x28, cpu.eax);
                cpu.eax = at(app, cpu.ebp - 0xc);
                put(app, cpu.ebp - 0x24, cpu.eax);
            }
            fld(cpu, f64(app, cpu.ebp - 0x28));
            fstp(app, cpu, cpu.ebp - 4);
        }
        if (s(at(app, kPlayMode)) > 1)
        {
            fld(cpu, f32(app, cpu.ebp - 4));
            fld(cpu, Float(1.0));
            fxch(cpu, 1);
            fstp64(app, cpu, cpu.ebp - 0x18);
            if (!fcomp(cpu, f64(app, cpu.ebp - 0x18)).aboveEqual())
            {
                cpu.esi = 0;
                cpu.edi = 0x3ff00000;
                put(app, cpu.ebp - 0x20, cpu.esi);
                put(app, cpu.ebp - 0x1c, cpu.edi);
            }
            else
            {
                cpu.eax = at(app, cpu.ebp - 0x18);
                put(app, cpu.ebp - 0x20, cpu.eax);
                cpu.eax = at(app, cpu.ebp - 0x14);
                put(app, cpu.ebp - 0x1c, cpu.eax);
            }
            fld(cpu, f64(app, cpu.ebp - 0x20));
            fstp(app, cpu, cpu.ebp - 4);
        }
        cpu.eax = at(app, cpu.ebp - 4);
        put(app, cpu.ebp - 8, cpu.eax);
    }
    fld(cpu, f32(app, cpu.ebp - 8));
    put(app, 0x56ebb8, cpu.ebx);
    cpu.esp = cpu.ebp;
    ret(app, cpu, {&cpu.ebp, &cpu.edi, &cpu.esi, &cpu.edx, &cpu.ecx, &cpu.ebx});
}

/* 0x409290: the target speed [+0x7b0] of the car eax: its node's (0x40ed40),
 * signed as it goes (the track backwards, the car against the traffic), then
 * cut where it follows the chased car (followChased). */
void nodeTargetSpeed(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ebx, cpu.ecx, cpu.edx, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.edx = cpu.eax;
    cpu.eax = at(app, cpu.eax + 0x1c);
    if (!byAddress(app, cpu, kNodeSpeed))
        return;
    cpu.ecx = at(app, kBackwards);
    fstp(app, cpu, cpu.edx + 0x7b0);
    if (cpu.ecx != 0)
        flipSign(app, cpu.edx, cpu.ah);
    if ((byte(app, cpu.edx + 0x621) & 0x10) != 0)
        flipSign(app, cpu.edx, cpu.bh);
    cpu.eax = cpu.edx;
    if (!direct(app, cpu, followChased))
        return;
    ret(app, cpu, {&cpu.ebp, &cpu.edx, &cpu.ecx, &cpu.ebx});
}

/* 0x4092f0: the car eax no longer chased (+0x620 & ~0x20) but where one of the
 * cops [0x5efa88..], [0x5efd88] of them, chases it (+0x624). */
void markChased(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ebx, cpu.ecx, cpu.edx, cpu.ebp});
    cpu.ebp = cpu.esp;
    putByte(app, cpu.eax + 0x620, byte(app, cpu.eax + 0x620) & 0xdf);
    for (cpu.edx = 0; s(cpu.edx) < s(at(app, 0x5efd88)); ++cpu.edx)
    {
        cpu.ecx = at(app, cpu.edx * 4 + 0x5efa88);
        cpu.ebx = at(app, cpu.ecx + 0x624);
        if (cpu.eax == cpu.ebx)
        {
            putByte(app, cpu.ebx + 0x620, byte(app, cpu.ebx + 0x620) | 0x20);
            return ret(app, cpu, {&cpu.ebp, &cpu.edx, &cpu.ecx, &cpu.ebx});
        }
    }
    ret(app, cpu, {&cpu.ebp, &cpu.edx, &cpu.ecx, &cpu.ebx});
}

/* 0x409330: the target speed [+0x7b0] and its factor [+0x7bc] of the car eax:
 * the computer's speed at its node (0x40e080) -- past [0x536414], with a
 * pace +0x700 over 1, eased: (speed + [0x53641c]) * pace + [0x536414] -- or
 * [0x56f224] against the traffic; no
 * more than its top speed times the track's pace (0x40ee80); times the
 * factors of its class, the leader, the radar (no more than 1, or 1 where
 * [0x6fd4fc] & 0x20); signed as it goes. */
void targetSpeed(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x3c;
    cpu.edx = cpu.eax;
    cpu.ecx = at(app, cpu.eax + 0x1c);
    if (!byAddress(app, cpu, kTrackPace))
        return;
    cpu.ah = byte(app, cpu.edx + 0x621);
    fstp(app, cpu, cpu.ebp - 0x10);
    if ((cpu.ah & 0x10) == 0)
    {
        cpu.eax = cpu.ecx;
        if (!byAddress(app, cpu, kSliceSpeed))
            return;
        fst(app, cpu, cpu.edx + 0x7b0);
        fst64(app, cpu, cpu.ebp - 0x3c);
        if (!fcomp(cpu, f64(app, 0x536414)).belowEqual())
        {
            fld(cpu, Float(1.0));
            fld(cpu, f32(app, cpu.edx + 0x700));
            fstp64(app, cpu, cpu.ebp - 0x24);
            if (!fcomp(cpu, f64(app, cpu.ebp - 0x24)).aboveEqual())
            {
                fld(cpu, f64(app, cpu.ebp - 0x3c));
                fadd(cpu, f64(app, 0x53641c));
                fmul(cpu, f64(app, cpu.ebp - 0x24));
                fadd(cpu, f64(app, 0x536414));
                fstp(app, cpu, cpu.edx + 0x7b0);
            }
        }
    }
    else
    {
        cpu.eax = at(app, 0x56f224);
        put(app, cpu.edx + 0x7b0, cpu.eax);
    }
    cpu.eax = at(app, cpu.edx + 0x528);
    fld(cpu, f32(app, cpu.eax + 0x1cc));
    fmul(cpu, f32(app, cpu.ebp - 0x10));
    fld(cpu, f32(app, cpu.edx + 0x7b0));
    fxch(cpu, 1);
    fstp(app, cpu, cpu.ebp - 8);
    if (!fcomp(cpu, f32(app, cpu.ebp - 8)).belowEqual())
    {
        cpu.eax = at(app, cpu.ebp - 8);
        put(app, cpu.edx + 0x7b0, cpu.eax);
    }
    cpu.eax = cpu.edx;
    if (!direct(app, cpu, classSpeedFactor))
        return;
    cpu.eax = cpu.edx;
    fstp(app, cpu, cpu.ebp - 0x1c);
    if (!direct(app, cpu, unitFactor))
        return;
    cpu.eax = cpu.edx;
    fstp(app, cpu, cpu.ebp - 0x18);
    if (!direct(app, cpu, leaderFactor))
        return;
    cpu.eax = cpu.edx;
    fstp(app, cpu, cpu.ebp - 0xc);
    if (!direct(app, cpu, unitFactorToo))
        return;
    cpu.eax = cpu.edx;
    fstp(app, cpu, cpu.ebp - 4);
    if (!direct(app, cpu, radarFactor))
        return;
    fld(cpu, f32(app, cpu.ebp - 0x1c));
    fmul(cpu, f32(app, cpu.ebp - 0x18));
    fmul(cpu, f32(app, cpu.ebp - 0xc));
    fmul(cpu, f32(app, cpu.ebp - 0x10));
    fmulp(cpu, 1);
    fmul(cpu, f32(app, cpu.ebp - 4));
    fld(cpu, Float(1.0));
    fxch(cpu, 1);
    fst(app, cpu, cpu.edx + 0x7bc);
    fstp64(app, cpu, cpu.ebp - 0x2c);
    if (!fcomp(cpu, f64(app, cpu.ebp - 0x2c)).aboveEqual())
    {
        cpu.ecx = 0;
        cpu.ebx = 0x3ff00000;
        put(app, cpu.ebp - 0x34, cpu.ecx);
        put(app, cpu.ebp - 0x30, cpu.ebx);
    }
    else
    {
        cpu.eax = at(app, cpu.ebp - 0x2c);
        put(app, cpu.ebp - 0x34, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 0x28);
        put(app, cpu.ebp - 0x30, cpu.eax);
    }
    fld(cpu, f64(app, cpu.ebp - 0x34));
    cpu.bl = byte(app, 0x6fd4fc);
    fstp(app, cpu, cpu.edx + 0x7bc);
    if ((cpu.bl & 0x20) != 0)
        put(app, cpu.edx + 0x7bc, 0x3f800000);
    fld(cpu, f32(app, cpu.edx + 0x7bc));
    fmul(cpu, f32(app, cpu.edx + 0x7b0));
    cpu.esi = at(app, kBackwards);
    fstp(app, cpu, cpu.edx + 0x7b0);
    if (cpu.esi != 0)
        flipSign(app, cpu.edx, cpu.bh);
    if ((byte(app, cpu.edx + 0x621) & 0x10) != 0)
        flipSign(app, cpu.edx, cpu.ch);
    cpu.esp = cpu.ebp;
    ret(app, cpu, {&cpu.ebp, &cpu.esi, &cpu.edx, &cpu.ecx, &cpu.ebx});
}

/* 0x4094b0: targetSpeed, from a frame of its own (push ebp, mov ebp, esp). */
void targetSpeedOnly(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (!direct(app, cpu, targetSpeed))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4094d0: the target speed [+0x7b0] of the car eax kept to the chased car's
 * [0x56ef3c] (not the player's) where both go the same way and the target is
 * past the chased car's speed: the chased car's speed at the chase's level
 * 1 or under, at 2 the target eased back by [0x536424] of the difference. */
void matchChasedSpeed(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ebx, cpu.ecx, cpu.edx, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x1c;
    cpu.edx = cpu.eax;
    cpu.ecx = at(app, 0x56ef3c);
    const auto done = [&] {
        cpu.esp = cpu.ebp;
        ret(app, cpu, {&cpu.ebp, &cpu.edx, &cpu.ecx, &cpu.ebx});
    };
    if (cpu.ecx == 0 || (byte(app, cpu.ecx + 0x200) & 0x20) != 0)
        return done();
    fld(cpu, f32(app, cpu.edx + 0x7c0));
    fld(cpu, Float(0.0));
    cpu.ebx = fcompp(cpu).belowEqual() ? 0 : 1;
    fld(cpu, f32(app, cpu.ecx + 0x7c0));
    fld(cpu, Float(0.0));
    cpu.eax = fcompp(cpu).belowEqual() ? 0 : 1;
    cpu.eax ^= cpu.ebx;
    if (cpu.eax != 0)
        return done();
    fld(cpu, Float(0.0));
    if (!fcomp(cpu, f32(app, cpu.edx + 0x7b0)).aboveEqual())
    {
        cpu.eax = at(app, cpu.edx + 0x7b0);
        put(app, cpu.ebp - 4, cpu.eax);
    }
    else
    {
        fld(cpu, f32(app, cpu.edx + 0x7b0));
        fchs(cpu);
        fstp(app, cpu, cpu.ebp - 4);
    }
    fld(cpu, f32(app, cpu.ebp - 4));
    fsub(cpu, f32(app, cpu.ecx + 0x7c4));
    fld(cpu, Float(0.0));
    fxch(cpu, 1);
    fstp64(app, cpu, cpu.ebp - 0xc);
    if (fcomp(cpu, f64(app, cpu.ebp - 0xc)).aboveEqual())
        return done();
    cpu.eax = at(app, 0x56ef48);
    if (s(cpu.eax) <= 1)
    {
        cpu.eax = at(app, cpu.ecx + 0x7c0);
        put(app, cpu.edx + 0x7b0, cpu.eax);
        return done();
    }
    if (cpu.eax != 2)
        return done();
    fld(cpu, f64(app, cpu.ebp - 0xc));
    fmul(cpu, f64(app, 0x536424));
    fld(cpu, Float(0.0));
    fld(cpu, f32(app, cpu.edx + 0x7b0));
    fstp64(app, cpu, cpu.ebp - 0x1c);
    fxch(cpu, 1);
    fstp64(app, cpu, cpu.ebp - 0x14);
    const bool down = !fcomp(cpu, f64(app, cpu.ebp - 0x1c)).aboveEqual();
    fld(cpu, f64(app, cpu.ebp - 0x1c));
    if (down)
        fsub(cpu, f64(app, cpu.ebp - 0x14));
    else
        fadd(cpu, f64(app, cpu.ebp - 0x14));
    fstp(app, cpu, cpu.edx + 0x7b0);
    done();
}

/* 0x4095d0: for a car eax of a class keener than 2 with no cop near it
 * (0x40f050): 15 more to each chase's score [0x56ef20] whose cop is one
 * (+0x200 & 4), on, and 1 to 9 behind -- but where the chase's level
 * [0x56ec80] is 5 or more. */
void scoreCloseCops(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ecx, cpu.edx, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    cpu.eax = classIndex3(app, cpu.eax, cpu.edx);
    const auto done = [&] { ret(app, cpu, {&cpu.ebp, &cpu.edx, &cpu.ecx}); };
    if (s(at(app, cpu.eax * 8 + kClassKeen)) <= 2)
        return done();
    cpu.eax = cpu.ecx;
    cpu.edx = 0;
    if (!byAddress(app, cpu, kCopNear))
        return;
    if (cpu.eax != 0)
        return done();
    if (s(at(app, 0x56ec80)) >= 5)
        return done();
    for (;;)
    {
        cpu.ecx = cpu.eax * 4;
        cpu.edx = at(app, cpu.ecx + 0x56ef50);
        if (cpu.edx != 0 && half(app, cpu.eax * 2 + 0x56ef6e) != 0 && (byte(app, cpu.edx + 0x200) & 4) != 0)
        {
            cpu.edx = at(app, cpu.ecx + 0x56ef5c);
            cpu.edx = 0 - cpu.edx;
            if (s(cpu.edx) < 0xa && s(cpu.edx) > 0)
                put(app, cpu.ecx + 0x56ef20, at(app, cpu.ecx + 0x56ef20) + 0xf);
        }
        cpu.eax += 1;
        if (s(cpu.eax) >= 3)
            return done();
    }
}

/* 0x409660: the lanes' times that have begun, 22 on each; eax left 22. */
void growLaneTimes(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.edx = at(app, 0x56eeb4);
    cpu.eax = 0x16;
    if (cpu.edx != 0)
    {
        cpu.ecx = cpu.edx + cpu.eax;
        put(app, 0x56eeb4, cpu.ecx);
    }
    cpu.ebx = at(app, 0x56eeb8);
    if (cpu.ebx != 0)
    {
        cpu.esi = cpu.ebx + cpu.eax;
        put(app, 0x56eeb8, cpu.esi);
    }
    cpu.edi = at(app, 0x56eebc);
    if (cpu.edi != 0)
    {
        cpu.edx = cpu.edi + cpu.eax;
        put(app, 0x56eebc, cpu.edx);
    }
    ret(app, cpu, {&cpu.ebp, &cpu.edi, &cpu.esi, &cpu.edx, &cpu.ecx, &cpu.ebx});
}

/* 0x4096b0: the score of the lane the car eax will take, 4 nodes on
 * (0x40e160, 0x40e0c0), against its own [+0x638]: 2 for a cop's (the lane
 * walked into its node's lanes), else 6 for a chase of kind 0, 12 or 10 by
 * its class -- to the left's [0x56ef14], its own [0x56ef18] or the right's
 * [0x56ef1c]; nothing against the traffic or with a cop near. */
void scoreLane(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    const auto done = [&] { ret(app, cpu, {&cpu.ebp, &cpu.edi, &cpu.esi, &cpu.edx, &cpu.ecx, &cpu.ebx}); };
    if ((byte(app, cpu.eax + 0x621) & 0x10) != 0)
        return done();
    cpu.eax = cpu.ecx;
    cpu.edx = 0;
    if (!byAddress(app, cpu, kCopNear))
        return;
    if (cpu.eax != 0)
        return done();
    cpu.edx = 4;
    cpu.eax = cpu.ecx;
    if (!byAddress(app, cpu, kNodeAhead))
        return;
    cpu.esi = cpu.eax;
    if (!byAddress(app, cpu, kNodeLane))
        return;
    cpu.edx = cpu.eax;
    if (cpu.eax == 0)
        return done();
    if ((byte(app, cpu.ecx + 0x200) & 4) != 0)
    {
        for (;;)
        {
            cpu.eax = cpu.esi * 8;
            cpu.ebx = cpu.esi + cpu.eax;
            cpu.eax = at(app, kNodes);
            cpu.ebx <<= 2;
            cpu.eax += cpu.ebx;
            cpu.ebx = 0;
            cpu.bl = byte(app, cpu.eax + 0x13);
            cpu.ebx = reg32(s(cpu.ebx) >> 4);
            cpu.edi = 7;
            cpu.ebx &= 0xf;
            cpu.edi -= cpu.ebx;
            if (s(cpu.edx) >= s(cpu.edi))
            {
                cpu.al = byte(app, cpu.eax + 0x13);
                cpu.al &= 0xf;
                cpu.eax &= 0xff;
                cpu.eax += 6;
                if (s(cpu.edx) <= s(cpu.eax))
                    break;
            }
            if (s(cpu.edx) >= 7)
                cpu.edx -= 1;
            else
                cpu.edx += 1;
        }
        cpu.eax = 2;
    }
    else
    {
        cpu.eax = cpu.ecx;
        if (!byAddress(app, cpu, kChaseKind))
            return;
        if (cpu.eax == 0)
            cpu.eax = 6;
        else
        {
            cpu.eax = classIndex3(app, cpu.ecx, cpu.ebx);
            cpu.eax = s(at(app, cpu.eax * 8 + kClassKeen)) > 2 ? 0xc : 0xa;
        }
    }
    cpu.esi = at(app, cpu.ecx + 0x638);
    if (s(cpu.edx) < s(cpu.esi))
        put(app, 0x56ef14, at(app, 0x56ef14) + cpu.eax);
    else if (cpu.edx == cpu.esi)
        put(app, 0x56ef18, at(app, 0x56ef18) + cpu.eax);
    else
        put(app, 0x56ef1c, at(app, 0x56ef1c) + cpu.eax);
    done();
}

/* 0x4097c0: the chases' scores where no cop is near the car eax: 5 more to
 * [0x56eefc] for each chase without a cop, -6 (-24 for the player's car,
 * halved where the chase's flag 0x56ef68 is off) to [0x56ef08] for each of
 * stage under 3. */
void scoreChases(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ebx, cpu.ecx, cpu.edx, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.edx = 0;
    if (!byAddress(app, cpu, kCopNear))
        return;
    if (cpu.eax == 0)
    {
        for (cpu.ecx = 0; s(cpu.ecx) < 3; ++cpu.ecx)
        {
            cpu.edx = cpu.ecx * 4;
            cpu.eax = at(app, cpu.edx + 0x56ef38);
            if (cpu.eax == 0)
            {
                put(app, cpu.edx + 0x56eefc, at(app, cpu.edx + 0x56eefc) + 5);
                continue;
            }
            if (s(at(app, cpu.edx + 0x56ef44)) >= 3)
                continue;
            cpu.bl = byte(app, cpu.eax + 0x200);
            cpu.edx = 0xfffffffa;
            if ((cpu.bl & 0x20) != 0)
                cpu.edx = 0xffffffe8;
            if (half(app, cpu.ecx * 2 + 0x56ef68) == 0)
            {
                cpu.eax = cpu.edx;
                cpu.edx = reg32(s(cpu.edx) >> 0x1f);
                cpu.eax -= cpu.edx;
                cpu.eax = reg32(s(cpu.eax) >> 1);
                cpu.edx = cpu.eax;
            }
            put(app, cpu.ecx * 4 + 0x56ef08, at(app, cpu.ecx * 4 + 0x56ef08) + cpu.edx);
        }
    }
    ret(app, cpu, {&cpu.ebp, &cpu.edx, &cpu.ecx, &cpu.ebx});
}

/* 0x409840: the car eax on the radio (+0x621 | 0x80) where it is of a keen
 * class, has been chased (+0x6e4), is not out (+0x7d4) and one of the chases
 * of stage 0 to 4 is its cop's; then the debug text of its chase's count
 * ("V%d", or blank) at its index's row (0x42d520). */
void radioChaseCount(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.ecx = at(app, cpu.eax + 0x220);
    cpu.ecx = at(app, cpu.ecx + 0x30);
    cpu.edx = cpu.ecx * 4;
    cpu.edx -= cpu.ecx;
    if (s(at(app, cpu.edx * 8 + kClassKeen)) > 2 && s(at(app, cpu.eax + 0x6e4)) > 0 && at(app, cpu.eax + 0x7d4) == 0
        && at(app, cpu.eax + 0x624) != 0)
    {
        for (cpu.edx = 0; s(cpu.edx) < 3; ++cpu.edx)
        {
            cpu.ecx = cpu.edx * 4;
            cpu.esi = at(app, cpu.eax + 0x624);
            if (cpu.esi != at(app, cpu.ecx + 0x56ef38))
                continue;
            cpu.edi = at(app, cpu.ecx + 0x56ef44);
            if (s(cpu.edi) < 0)
                continue;
            if (s(cpu.edi) < 5)
            {
                putByte(app, cpu.eax + 0x621, byte(app, cpu.eax + 0x621) | 0x80);
                break;
            }
        }
    }
    if ((byte(app, cpu.eax + 0x621) & 0x80) != 0)
    {
        cpu.esi = at(app, cpu.eax + 0x6e4);
        cpu.edx = at(app, cpu.eax + 0x1f0);
        push(app, cpu, cpu.esi);
        cpu.eax = cpu.edx * 4;
        push(app, cpu, 0x53642c);
        cpu.eax -= cpu.edx;
        push(app, cpu, 0xf);
        cpu.eax += cpu.eax;
        push(app, cpu, cpu.eax);
        push(app, cpu, 1);
        push(app, cpu, 0x4b);
        if (!byAddress(app, cpu, kDebugText))
            return;
        cpu.esp += 0x18;
    }
    else
    {
        cpu.edx = at(app, cpu.eax + 0x1f0);
        cpu.eax = cpu.edx * 4;
        push(app, cpu, 0x536430);
        cpu.eax -= cpu.edx;
        push(app, cpu, 0xf);
        cpu.eax += cpu.eax;
        push(app, cpu, cpu.eax);
        push(app, cpu, 1);
        push(app, cpu, 0x4b);
        if (!byAddress(app, cpu, kDebugText))
            return;
        cpu.esp += 0x14;
    }
    ret(app, cpu, {&cpu.ebp, &cpu.edi, &cpu.esi, &cpu.edx, &cpu.ecx});
}

/* 0x409920: the chases' scores [0x56eef0] by speed, for a car eax of a class
 * of 2 or under: for each chase with a cop, by how much the car's speed and
 * its target are past the cop's (times [0x53644c] and [0x536454]), and where
 * the chase's stage is under the car's speed's (|speed| * .. + ..) by
 * [0x53645c] and [0x53644c] again. */
void scoreSpeeds(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x2c;
    cpu.ebx = cpu.eax;
    cpu.eax = classIndex3(app, cpu.eax, cpu.edx);
    const auto done = [&] {
        cpu.esp = cpu.ebp;
        ret(app, cpu, {&cpu.ebp, &cpu.edi, &cpu.esi, &cpu.edx, &cpu.ecx, &cpu.ebx});
    };
    if (s(at(app, cpu.eax * 8 + kClassKeen)) > 2)
        return done();
    fld(cpu, f32(app, cpu.ebx + 0x7c4));
    fmul(cpu, f64(app, 0x53643c));
    fmul(cpu, f64(app, 0x536444));
    fadd(cpu, f64(app, 0x53644c));
    cpu.edx = 0;
    fistpToEax(app, cpu);
    cpu.esi = cpu.eax;
    // (fldz; fcomp qword [m]; fnstsw ax; sahf; jae: no score) -- else the
    // score, the value times the factor as an integer, added.
    const auto score = [&](reg32 local, reg32 factor, reg32 into) {
        if (fcomp(cpu, f64(app, local)).aboveEqual())
            return;
        fld(cpu, f64(app, local));
        fmul(cpu, f64(app, factor));
        fistpToEax(app, cpu);
        put(app, into, at(app, into) + cpu.eax);
    };
    for (; s(cpu.edx) < 3; ++cpu.edx)
    {
        cpu.ecx = cpu.edx * 4;
        cpu.edi = at(app, cpu.ecx + 0x56ef38);
        if (cpu.edi == 0)
            continue;
        fld(cpu, Float(0.0));
        fld(cpu, f32(app, cpu.ebx + 0x7c0));
        cpu.eax = at(app, cpu.edi + 0x7c0);
        fld(cpu, f32(app, cpu.ebx + 0x7b0));
        put(app, cpu.ebp - 0xc, cpu.eax);
        cpu.edi = at(app, cpu.ecx + 0x56ef44);
        fxch(cpu, 1);
        fsub(cpu, f32(app, cpu.ebp - 0xc));
        fxch(cpu, 1);
        fsub(cpu, f32(app, cpu.ebp - 0xc));
        fxch(cpu, 1);
        fstp(app, cpu, cpu.ebp - 4);
        fstp(app, cpu, cpu.ebp - 8);
        fld(cpu, f32(app, cpu.ebp - 4));
        fstp64(app, cpu, cpu.ebp - 0x1c);
        score(cpu.ebp - 0x1c, 0x53644c, cpu.ecx + 0x56eef0);
        fld(cpu, f32(app, cpu.ebp - 8));
        fld(cpu, Float(0.0));
        fxch(cpu, 1);
        fstp64(app, cpu, cpu.ebp - 0x14);
        score(cpu.ebp - 0x14, 0x536454, cpu.edx * 4 + 0x56eef0);
        if (s(cpu.edi) >= s(cpu.esi))
            continue;
        fld(cpu, f32(app, cpu.ebp - 4));
        fld(cpu, Float(0.0));
        fxch(cpu, 1);
        fstp64(app, cpu, cpu.ebp - 0x24);
        score(cpu.ebp - 0x24, 0x53645c, cpu.edx * 4 + 0x56eef0);
        fld(cpu, f32(app, cpu.ebp - 8));
        fld(cpu, Float(0.0));
        fxch(cpu, 1);
        fstp64(app, cpu, cpu.ebp - 0x2c);
        score(cpu.ebp - 0x2c, 0x53644c, cpu.edx * 4 + 0x56eef0);
    }
    done();
}

/* 0x409a80: 1 in eax. */
void alwaysOne(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.eax = 1;
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x409a90: for each of the cars [0x5efc08..], [0x553078] of them: where the
 * cop eax is within reach of it (0x40f000, its node against the car's 25 --
 * or 12, or halved -- nodes back, 0x41), the cop takes it over (0x40f100);
 * eax as the last of these leaves it. */
void takeOverCars(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    cpu.edi = cpu.eax;
    cpu.esi = 0;
    cpu.eax = 0;
    for (; s(cpu.esi) < s(at(app, 0x553078)); ++cpu.esi)
    {
        cpu.ecx = at(app, cpu.esi * 4 + 0x5efc08);
        if (cpu.ecx == 0)
            continue;
        cpu.ebx = at(app, 0x6fd4cc);
        cpu.edx = 0x19;
        if (cpu.ebx != 0)
            cpu.edx = 0xc;
        if (at(app, 0x6fd4c8) != 0 && at(app, cpu.ecx + 0x660) == 0)
        {
            cpu.eax = cpu.edx;
            cpu.edx = reg32(s(cpu.edx) >> 0x1f);
            cpu.eax -= cpu.edx;
            cpu.eax = reg32(s(cpu.eax) >> 1);
            cpu.edx = cpu.eax;
        }
        if ((byte(app, cpu.ecx + 0x710) & 0x10) != 0)
            cpu.edx = 0x19;
        cpu.edx = 0 - cpu.edx;
        cpu.eax = cpu.ecx;
        if (!byAddress(app, cpu, kNodeAhead))
            return;
        cpu.edx = at(app, cpu.edi + 0x1c);
        cpu.ebx = 0x41;
        put(app, cpu.ebp - 4, cpu.edx);
        cpu.edx = cpu.eax;
        cpu.eax = at(app, cpu.ebp - 4);
        if (!byAddress(app, cpu, kInReach))
            return;
        if (cpu.eax != 0)
        {
            cpu.edx = cpu.ecx;
            cpu.eax = cpu.edi;
            if (!byAddress(app, cpu, kTakeOver))
                return;
        }
    }
    cpu.esp = cpu.ebp;
    ret(app, cpu, {&cpu.ebp, &cpu.edi, &cpu.esi, &cpu.edx, &cpu.ecx, &cpu.ebx});
}

/* 0x409b40: the cop eax back to state 1, its record's +0x1c cleared, and the
 * car reset (0x429100) where it may be (+0x621 & 0x40). */
void startReset(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.edx, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.edx = at(app, cpu.eax + 0x628);
    put(app, cpu.eax + 0x6e0, 1);
    put(app, cpu.edx + 0x1c, 0);
    if ((byte(app, cpu.eax + 0x621) & 0x40) != 0)
        if (!byAddress(app, cpu, kResetCar))
            return;
    ret(app, cpu, {&cpu.ebp, &cpu.edx});
}

/* 0x409b70: the car eax's progress: where it has gained no place since
 * [+0x708] (and its record's +0x2c is 0, the race past 1152 frames, not a
 * network one), 384 frames on from [+0x704] it is reset (0x429100, the
 * frame noted); else its place and the frame noted. */
void checkProgress(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ecx, cpu.edx, cpu.esi, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.ecx = at(app, cpu.eax + 0x708);
    const auto done = [&] { ret(app, cpu, {&cpu.ebp, &cpu.esi, &cpu.edx, &cpu.ecx}); };
    bool stuck = s(cpu.ecx) >= s(at(app, cpu.eax + 8));
    if (stuck)
    {
        cpu.edx = at(app, cpu.eax + 0x628);
        stuck = at(app, cpu.edx + 0x2c) == 0;
    }
    if (stuck)
    {
        cpu.esi = at(app, kRaceFrame);
        stuck = s(cpu.esi) > 0x480 && at(app, 0x6fd50c) == 0;
    }
    if (stuck)
    {
        cpu.edx = at(app, cpu.eax + 0x704);
        cpu.edx += 0x180;
        if (s(cpu.edx) >= s(cpu.esi))
            return done();
        put(app, cpu.eax + 0x704, cpu.esi);
        if (!byAddress(app, cpu, kResetCar))
            return;
        return done();
    }
    cpu.edx = at(app, cpu.eax + 8);
    put(app, cpu.eax + 0x708, cpu.edx);
    cpu.edx = at(app, kRaceFrame);
    put(app, cpu.eax + 0x704, cpu.edx);
    done();
}

/* 0x409c00: the cop eax's step, while it is active (0x40df10) -- its radio
 * mark cleared, then by its state [+0x6e0], the switch at 0x409be0:
 *   0 (and past 7): its pace from its top speed, state 1, and on as 1;
 *   1 chasing: its progress, then -- unless it is held (0x404b90) -- steered,
 *     its target speed, the scores, driven; out of the race (0x401500) state
 *     5 for 320 frames (640 where it may be reset), a human's car on a pad
 *     state 4; state 2 where it takes over a car (takeOverCars);
 *   2 the same, fewer of them, back to 1 where it takes over none;
 *   3 at its node's speed, then 1;
 *   4 stopped (pulledOverTarget), the frame noted;
 *   5 pulled over at the roadside: once slow enough (or 320 frames on) the
 *     car's tickets [0x56ea74 + index * 20] counted up to 5, then state 6 or
 *     reset (startReset);
 *   6 halted: 1 again when no longer out, reset when its record says so;
 *   7 the frame noted. */
void copStep(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ebx, cpu.ecx, cpu.edx, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.edx = cpu.eax;
    const auto done = [&] { ret(app, cpu, {&cpu.ebp, &cpu.edx, &cpu.ecx, &cpu.ebx}); };
    // mov eax, edx; call `target`: false where the game stopped in it.
    const auto own = [&](void (*target)(App*, CPU&)) {
        cpu.eax = cpu.edx;
        return direct(app, cpu, target);
    };
    const auto other = [&](reg32 target) {
        cpu.eax = cpu.edx;
        return byAddress(app, cpu, target);
    };
    // Out of the race (eax): state 5 for 320 frames (another 320 where
    // `longer` and the car may be reset); a human's car steered with a pad
    // (+0x2c4 3 or 4) state 4.
    const auto outOrPad = [&](bool longer) {
        if (cpu.eax != 0)
        {
            cpu.eax = at(app, kRaceFrame);
            put(app, cpu.edx + 0x6e0, 5);
            cpu.eax += 0x140;
            if (longer)
            {
                cpu.bl = byte(app, cpu.edx + 0x621);
                put(app, cpu.edx + 0x6ec, cpu.eax);
                if ((cpu.bl & 0x40) != 0)
                {
                    cpu.ecx = cpu.eax + 0x140;
                    put(app, cpu.edx + 0x6ec, cpu.ecx);
                }
            }
            else
                put(app, cpu.edx + 0x6ec, cpu.eax);
        }
        if (((byte(app, cpu.edx + 0x200) & 1) != 0 && at(app, cpu.edx + 0x2c4) == 3)
            || ((byte(app, cpu.edx + 0x200) & 1) != 0 && at(app, cpu.edx + 0x2c4) == 4))
            put(app, cpu.edx + 0x6e0, 4);
    };
    const auto noteFrame = [&] {
        cpu.eax = at(app, kRaceFrame);
        put(app, cpu.edx + 0x704, cpu.eax);
    };
    if (!byAddress(app, cpu, kCopActive))
        return;
    if (cpu.eax == 0)
        return done();
    putByte(app, cpu.edx + 0x621, byte(app, cpu.edx + 0x621) & 0x7f);
    cpu.eax = at(app, cpu.edx + 0x6e0);
    switch (cpu.eax)
    {
    default:  // 0x409c2e
        cpu.eax = at(app, cpu.edx + 0x528);
        fld(cpu, f32(app, cpu.eax + 0x1cc));
        fadd(cpu, f64(app, 0x536464));
        fmul(cpu, f64(app, 0x53646c));
        put(app, cpu.edx + 0x6e0, 1);
        fstp(app, cpu, cpu.edx + 0x700);
        [[fallthrough]];
    case 1:  // 0x409c56
        if (!own(checkProgress) || !other(kResetHeld))
            return;
        if (cpu.eax != 0)
            return done();
        if (!other(kCopSteer) || !own(markChased) || !own(targetSpeed) || !other(kCopAvoid)
            || !own(matchChasedSpeed) || !own(scoreSpeeds) || !own(radioChaseCount) || !own(scoreChases)
            || !own(scoreLane) || !own(scoreCloseCops) || !other(kCopDrive) || !own(growLaneTimes)
            || !other(kCopFinish) || !other(kCopOut))
            return;
        outOrPad(true);
        if (!own(takeOverCars))
            return;
        if (cpu.eax == 0)
            return done();
        put(app, cpu.edx + 0x6e0, 2);
        return done();
    case 2:  // 0x409d4f
        if (!own(checkProgress) || !other(kResetHeld))
            return;
        if (cpu.eax != 0)
            return done();
        if (!other(kCopSteer) || !own(markChased) || !own(targetSpeedOnly) || !own(scoreChases)
            || !other(kCopDrive) || !own(growLaneTimes) || !other(kCopFinish) || !other(kCopOut))
            return;
        outOrPad(false);
        if (!own(takeOverCars))
            return;
        if (cpu.eax != 0)
            return done();
        put(app, cpu.edx + 0x6e0, 1);
        return done();
    case 3:  // 0x409e08
        if (!own(checkProgress) || !other(kCopSteer) || !own(nodeTargetSpeed) || !other(kCopDrive)
            || !other(kCopFinish) || !other(kCopOut))
            return;
        outOrPad(false);
        if (!own(alwaysOne))
            return;
        if (cpu.eax == 0)
            return done();
        put(app, cpu.edx + 0x6e0, 1);
        return done();
    case 4:  // 0x409e9c
        if (!own(pulledOverTarget) || !other(kCopSteer) || !other(kCopFinish))
            return;
        noteFrame();
        return done();
    case 5:  // 0x409ec1
        if (!other(kPullOver) || !own(roadsideTarget) || !own(countLaneTime) || !other(kCopFinish))
            return;
        if (half(app, cpu.edx + 0x164) != 0)
            break;
        fld(cpu, f32(app, cpu.edx + 0x10c));
        if (!fcomp(cpu, f64(app, 0x536474)).above())
        {
            noteFrame();
            return done();
        }
        fld(cpu, f32(app, cpu.edx + 0x7c4));
        if (!fcomp(cpu, f64(app, 0x53647c)).below())
        {
            cpu.eax = at(app, kRaceFrame);
            if (s(cpu.eax) <= s(at(app, cpu.edx + 0x6ec)))
                break;
        }
        cpu.ecx = at(app, cpu.edx + 0x1f0);
        cpu.eax = cpu.ecx * 4;
        cpu.eax += cpu.ecx;
        cpu.ebx = at(app, cpu.eax * 4 + 0x56ea74);
        cpu.ebx += 1;
        if (s(cpu.ebx) >= 5)
            cpu.ebx = 5;
        cpu.ecx = at(app, cpu.edx + 0x1f0);
        cpu.eax = cpu.ecx * 4;
        cpu.eax += cpu.ecx;
        put(app, cpu.eax * 4 + 0x56ea74, cpu.ebx);
        cpu.eax = at(app, cpu.edx + 0x628);
        if (at(app, cpu.eax + 0x2c) != 0)
        {
            put(app, cpu.edx + 0x6e0, 6);
            noteFrame();
            return done();
        }
        if (!own(startReset))
            return;
        noteFrame();
        return done();
    case 6:  // 0x409fa2
        cpu.eax = cpu.edx;
        put(app, cpu.edx + 0x7b0, 0);
        if (!byAddress(app, cpu, kCopStop) || !other(kCopOut))
            return;
        if (cpu.eax == 0)
            put(app, cpu.edx + 0x6e0, 1);
        cpu.eax = at(app, cpu.edx + 0x628);
        if (at(app, cpu.eax + 0x2c) == 0)
            if (!own(startReset))
                return;
        break;
    case 7:  // 0x409fdb
        break;
    }
    noteFrame();
    done();
}

namespace
{

/* The inline strcpy the compiler makes of a name copied: two bytes a turn
 * from [esi] to [edi], al the last. */
void copyLoop(App* app, CPU& cpu)
{
    for (;;)
    {
        cpu.al = byte(app, cpu.esi);
        putByte(app, cpu.edi, cpu.al);
        if (cpu.al == 0)
            return;
        cpu.al = byte(app, cpu.esi + 1);
        cpu.esi += 2;
        putByte(app, cpu.edi + 1, cpu.al);
        cpu.edi += 2;
        if (cpu.al == 0)
            return;
    }
}

/* push edi; the copy; pop edi. */
void copyString(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edi);
    copyLoop(app, cpu);
    cpu.edi = pop(app, cpu);
}

/* push edi; sub ecx, ecx; dec ecx; mov al, 0; repne scasb; dec edi; the
 * copy; pop edi: [esi] appended to the string at edi (strcat). */
void appendString(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edi);
    cpu.ecx = 0xffffffff;
    cpu.al = 0;
    while (cpu.ecx != 0)
    {
        const bool found = byte(app, cpu.ees + cpu.edi) == cpu.al;
        cpu.edi += cpu.flags.df ? reg32(0) - 1 : 1;
        --cpu.ecx;
        if (found)
            break;
    }
    cpu.edi -= 1;
    copyLoop(app, cpu);
    cpu.edi = pop(app, cpu);
}

}  // namespace

/* 0x40a010: in a Hot Pursuit race (type 3), each car's speech bank
 * "<folder>\IPspch<language letter>.viv" (the letters E, G, F, S, I of the
 * table at 0x409ff0 by [0x6fbb4c]) loaded as bank 15 on (0x413750) for the
 * cars of models 0 to 49 that are not police cars, its colour's nearest
 * (0x431280, from the colours at 0x6fd54c, 0x6c bytes a car) handed on --
 * 99 where the car's colour is the model's custom one (the split screen's
 * players, or the player's: debug texts) or the model's name has "Pace" in
 * it; each car's bank noted [0x56ee28]; "too many racer banks" traced at the
 * 23rd. */
void loadRacerBanks(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x2a8;
    cpu.ecx = 7;
    cpu.edi = cpu.ebp - 0x28;
    cpu.esi = 0x409ff0;
    cpu.edx = at(app, kRaceType);
    for (; cpu.ecx != 0; --cpu.ecx)
    {
        put(app, cpu.ees + cpu.edi, at(app, cpu.esi));
        const reg32 step = cpu.flags.df ? reg32(0) - 4 : 4;
        cpu.edi += step;
        cpu.esi += step;
    }
    const auto done = [&] {
        cpu.esp = cpu.ebp;
        ret(app, cpu, {&cpu.ebp, &cpu.edi, &cpu.esi, &cpu.edx, &cpu.ecx, &cpu.ebx});
    };
    if (cpu.edx != 3)
        return done();
    cpu.ebx = 0;
    cpu.ecx = 0xf;
    put(app, cpu.ebp - 4, cpu.ebx);
    put(app, cpu.ebp - 8, cpu.ecx);
    const reg32 path = cpu.ebp - 0x1a8;
    for (;; put(app, cpu.ebp - 4, at(app, cpu.ebp - 4) + 1))
    {
        cpu.eax = at(app, cpu.ebp - 4);
        if (s(cpu.eax) >= s(at(app, 0x5efd9c)))
            return done();
        cpu.ecx = cpu.eax * 4;
        cpu.eax = at(app, cpu.ecx + 0x5efc88);
        cpu.edx = at(app, cpu.eax + 0x1f0);
        put(app, cpu.ecx + cpu.ebp - 0xa8, cpu.edx);
        cpu.eax = at(app, cpu.eax + 0x220);
        cpu.eax = at(app, cpu.eax + 0x14);
        cpu.edi = cpu.ebp - 0x2a8;
        put(app, cpu.ecx + cpu.ebp - 0x68, cpu.eax);
        if (!byAddress(app, cpu, kCarName))
            return;
        cpu.esi = cpu.eax;
        cpu.edx = at(app, cpu.ebp - 4);
        copyString(app, cpu);
        cpu.eax = cpu.edx * 4;
        cpu.eax -= cpu.edx;
        cpu.eax <<= 2;
        cpu.edx = cpu.eax;
        cpu.eax <<= 3;
        cpu.eax += cpu.edx;
        cpu.ebx = 0;
        cpu.edx = 0;
        cpu.bl = byte(app, cpu.eax + 0x6fd54e);
        cpu.dl = byte(app, cpu.eax + 0x6fd54d);
        cpu.al = byte(app, cpu.eax + 0x6fd54c);
        cpu.eax &= 0xff;
        if (!byAddress(app, cpu, kNearestColour))
            return;
        cpu.edi = at(app, cpu.ecx + cpu.ebp - 0x68);
        put(app, cpu.ebp - 0xc, cpu.eax);
        if (s(cpu.edi) < 0 || s(cpu.edi) > 0x31)
            continue;
        cpu.edx = at(app, cpu.ebp - 4);
        cpu.edx <<= 2;
        cpu.eax = at(app, cpu.edx + cpu.ebp - 0x68);
        if (!byAddress(app, cpu, kCarIsPolice))
            return;
        if (cpu.eax != 0)
            continue;
        cpu.esi = 0x7a28b4;
        cpu.edi = path;
        copyString(app, cpu);
        cpu.eax = path;
        if (!byAddress(app, cpu, kEndPath))
            return;
        cpu.eax = at(app, cpu.edx + cpu.ebp - 0x68);
        cpu.edi = path;
        if (!byAddress(app, cpu, kCarFolder))
            return;
        cpu.esi = cpu.eax;
        appendString(app, cpu);
        cpu.eax = path;
        cpu.esi = 0x5364c8;
        cpu.edi = path;
        if (!byAddress(app, cpu, kEndPath))
            return;
        appendString(app, cpu);
        cpu.eax = at(app, 0x6fbb4c);
        cpu.edi = path;
        cpu.esi = at(app, cpu.ebp + cpu.eax * 4 - 0x28);
        appendString(app, cpu);
        cpu.esi = 0x5364d0;
        cpu.edi = path;
        appendString(app, cpu);
        cpu.ecx = at(app, cpu.edx + cpu.ebp - 0xa8);
        bool split = cpu.ecx == 0 || cpu.ecx == 1;
        if (split)
        {
            cpu.esi = at(app, kPlayMode);
            split = cpu.esi == 1;
        }
        if (split)
        {
            cpu.ecx = at(app, cpu.ebp - 4);
            cpu.ecx <<= 2;
            cpu.eax = at(app, cpu.ecx + cpu.ebp - 0x68);
            if (!byAddress(app, cpu, kCarColour))
                return;
            cpu.edi = cpu.eax;
            cpu.edx = at(app, cpu.ecx + cpu.ebp - 0xa8);
            cpu.eax = at(app, cpu.ecx + cpu.ebp - 0x68);
            if (!byAddress(app, cpu, kCarColours))
                return;
            if (cpu.edi == cpu.eax)
            {
                cpu.eax = at(app, cpu.ecx + cpu.ebp - 0xa8);
                push(app, cpu, cpu.eax);
                cpu.edx = at(app, cpu.ebp - 4);
                push(app, cpu, cpu.edx);
                push(app, cpu, 0x5364d8);
                push(app, cpu, cpu.esi);
                cpu.edi = 0x63;
                push(app, cpu, 0x64);
                put(app, cpu.ebp - 0xc, cpu.edi);
                if (!byAddress(app, cpu, kTrace))
                    return;
                cpu.esp += 0x14;
            }
        }
        else
        {
            cpu.ecx = at(app, cpu.ebp - 4);
            cpu.ecx <<= 2;
            cpu.ebx = at(app, 0x6fd4f4);
            if (cpu.ebx == at(app, cpu.ecx + cpu.ebp - 0xa8))
            {
                cpu.eax = at(app, cpu.ecx + cpu.ebp - 0x68);
                if (!byAddress(app, cpu, kCarColour))
                    return;
                cpu.edx = 0;
                cpu.ebx = cpu.eax;
                cpu.eax = at(app, cpu.ecx + cpu.ebp - 0x68);
                if (!byAddress(app, cpu, kCarColours))
                    return;
                if (cpu.ebx == cpu.eax)
                {
                    cpu.edi = at(app, cpu.ecx + cpu.ebp - 0xa8);
                    push(app, cpu, cpu.edi);
                    cpu.eax = at(app, cpu.ebp - 4);
                    push(app, cpu, cpu.eax);
                    push(app, cpu, 0x536508);
                    push(app, cpu, 1);
                    cpu.esi = 0x63;
                    push(app, cpu, 0x64);
                    put(app, cpu.ebp - 0xc, cpu.esi);
                    if (!byAddress(app, cpu, kTrace))
                        return;
                    cpu.esp += 0x14;
                }
            }
            else
            {
                cpu.eax = at(app, cpu.ecx + cpu.ebp - 0x68);
                cpu.edx = 0x53652c;
                if (!byAddress(app, cpu, kCarName) || !byAddress(app, cpu, kStrstr))
                    return;
                if (cpu.eax != 0)
                    put(app, cpu.ebp - 0xc, 0x63);
            }
        }
        cpu.ebx = at(app, cpu.ebp - 8);
        cpu.edx = at(app, cpu.ebp - 0xc);
        cpu.eax = path;
        if (!byAddress(app, cpu, kVoiceBankLoad))
            return;
        cpu.eax = at(app, cpu.ebp - 4);
        cpu.edx = at(app, cpu.ebp - 8);
        put(app, cpu.eax * 4 + 0x56ee28, cpu.edx);
        cpu.ecx = cpu.edx + 1;
        put(app, cpu.ebp - 8, cpu.ecx);
        if (cpu.ecx == 0x17)
        {
            push(app, cpu, 0x536534);
            if (!byAddress(app, cpu, kMonoPrint))
                return;
            cpu.esp += 4;
        }
    }
}

/* 0x40a330: the radio's frames counted: since anyone spoke [0x5524c8], and
 * each of the 8 voices' [0x56ee68] and [0x5524cc]. */
void countRadioFrames(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.ecx = at(app, 0x5524c8);
    cpu.ecx += 1;
    cpu.edx = 0;
    put(app, 0x5524c8, cpu.ecx);
    do
    {
        cpu.ebx = at(app, cpu.edx * 4 + 0x56ee68);
        cpu.esi = at(app, cpu.edx * 4 + 0x5524cc);
        cpu.edx += 1;
        cpu.ebx += 1;
        cpu.esi += 1;
        put(app, cpu.edx * 4 + 0x56ee64, cpu.ebx);
        put(app, cpu.edx * 4 + 0x5524c8, cpu.esi);
    } while (s(cpu.edx) < 8);
    ret(app, cpu, {&cpu.ebp, &cpu.esi, &cpu.edx, &cpu.ecx, &cpu.ebx});
}

namespace
{

/* div r: edx:eax by r, the quotient in eax, the remainder in edx. */
void divide(CPU& cpu, reg32 by)
{
    const std::uint64_t n = std::uint64_t(cpu.edx) << 32 | cpu.eax;
    cpu.eax = reg32(n / by);
    cpu.edx = reg32(n % by);
}

/* mov edx, 8; mov eax, [ecx + 0x714]; mov [ebp - 4], edx; mov edx, eax;
 * sar edx, 0x1f; idiv dword [ebp - 4]: the car's voice's place in the 8. */
void voiceOfEight(App* app, CPU& cpu)
{
    cpu.edx = 8;
    cpu.eax = at(app, cpu.ecx + 0x714);
    put(app, cpu.ebp - 4, cpu.edx);
    cpu.edx = cpu.eax;
    cpu.edx = reg32(s(cpu.edx) >> 0x1f);
    const std::int64_t n = std::int64_t(std::uint64_t(cpu.edx) << 32 | cpu.eax);
    const std::int64_t by = s(at(app, cpu.ebp - 4));
    cpu.eax = reg32(n / by);
    cpu.edx = reg32(n % by);
}

}  // namespace

/* 0x40a380: a burst of the radio's noise (phrase 0x20 + a random 0 to 8) for
 * the car ebx, eax handed on, panned left or right by edx, at volume 127
 * (0x4c85a0) -- but in a race of the network's [0x6fd3a0]. */
void radioNoise(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ecx, cpu.esi, cpu.edi, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x24;
    cpu.ecx = cpu.eax;
    if (at(app, 0x6fd3a0) == 0)
    {
        cpu.eax = cpu.edx != 0 ? 0x3fff : 0xc000;
        cpu.edi = 0x7f;
        cpu.edx = 0x40;
        cpu.esi = 9;
        put(app, cpu.ebp - 0x18, cpu.eax);
        put(app, cpu.ebp - 0x20, cpu.edi);
        put(app, cpu.ebp - 0x1c, cpu.edx);
        cpu.edx = 0;
        if (!byAddress(app, cpu, kRandom))
            return;
        divide(cpu, cpu.esi);
        put(app, cpu.ebp - 0x10, cpu.ecx);
        cpu.ecx = 5;
        put(app, cpu.ebp - 4, cpu.ebx);
        put(app, cpu.ebp - 0xc, cpu.ecx);
        cpu.eax = at(app, 0x571160);
        cpu.edx += 0x20;
        put(app, cpu.ebp - 0x14, cpu.eax);
        cpu.eax = cpu.ebp - 0x24;
        put(app, cpu.ebp - 0x24, cpu.edx);
        if (!byAddress(app, cpu, kSpeechLine))
            return;
    }
    cpu.esp = cpu.ebp;
    ret(app, cpu, {&cpu.ebp, &cpu.edi, &cpu.esi, &cpu.ecx});
}

/* 0x40a410: the phrase ebx said by the car ecx (0x4c85a0), eax handed on --
 * by the dispatcher (edx 0: volume 110, centred, its own voice [0x5711cc])
 * or by the car's voice of the 8 (+0x714 mod 8, the switch at 0x40a3f0: its
 * voice of 0xb to 0xd [0x571124 + 12 voice] and its pan), the mark 7, 6 or
 * 5 by the phrase (0x21 the dispatcher's own); nothing in a race of the
 * network's.  A voice's place past the 8 (negative) leaves esi, edi and the
 * pan as they were, as the code does. */
void sayPhrase(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.esi, cpu.edi, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x34;
    put(app, cpu.ebp - 0x10, cpu.eax);
    put(app, cpu.ebp - 0xc, cpu.edx);
    if (at(app, 0x6fd3a0) == 0)
    {
        if (at(app, cpu.ebp - 0xc) != 0)
        {
            voiceOfEight(app, cpu);
            // mov edi, voice; (mov eax / edx, pan;) mov esi, 0x3fff; mov [ebp - 8], pan
            const auto viaEax = [&](reg32 voice, reg32 pan) {
                cpu.edi = voice;
                cpu.eax = pan;
                cpu.esi = 0x3fff;
                put(app, cpu.ebp - 8, cpu.eax);
            };
            const auto viaEdx = [&](reg32 voice, reg32 pan) {
                cpu.edi = voice;
                cpu.edx = pan;
                cpu.esi = 0x3fff;
                put(app, cpu.ebp - 8, cpu.edx);
            };
            switch (cpu.edx)
            {
            case 0:
            case 6: viaEax(0xb, 0x40); break;  // 0x40a45b
            case 1: viaEdx(0xd, 0x40); break;  // 0x40a46f
            case 2: viaEax(0xc, 0x40); break;  // 0x40a476
            case 3: viaEdx(0xb, 0x50); break;  // 0x40a48a
            case 4: viaEax(0xd, 0x50); break;  // 0x40a496
            case 5: viaEdx(0xc, 0x50); break;  // 0x40a4aa
            case 7: viaEdx(0xc, 0x40); break;  // 0x40a4b6
            default: break;
            }
        }
        else
            cpu.esi = 0xc000;
        put(app, cpu.ebp - 0x30, at(app, cpu.ebp - 0xc) != 0 ? 0x7f : 0x6e);
        put(app, cpu.ebp - 0x2c, at(app, cpu.ebp - 0xc) != 0 ? at(app, cpu.ebp - 8) : 0x40);
        cpu.edx = at(app, cpu.ebp - 0xc);
        put(app, cpu.ebp - 0x34, cpu.ebx);
        put(app, cpu.ebp - 0x28, cpu.esi);
        if (cpu.edx != 0)
        {
            cpu.eax = cpu.edi * 4;
            cpu.eax -= cpu.edi;
            cpu.eax = at(app, cpu.eax * 4 + 0x571124);
        }
        else
            cpu.eax = at(app, 0x5711cc);
        put(app, cpu.ebp - 0x24, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 0x10);
        put(app, cpu.ebp - 0x20, cpu.eax);
        if (cpu.ebx == 0x21)
            put(app, cpu.ebp - 0x1c, at(app, cpu.ebp - 0xc) == 0 ? 7 : 6);
        else
            put(app, cpu.ebp - 0x1c, 5);
        cpu.eax = cpu.ebp - 0x34;
        put(app, cpu.ebp - 0x14, cpu.ecx);
        if (!byAddress(app, cpu, kSpeechLine))
            return;
    }
    cpu.esp = cpu.ebp;
    ret(app, cpu, {&cpu.ebp, &cpu.edi, &cpu.esi});
}

/* 0x40a580: the phrase ebx of the car ecx's own description said (0x4c85a0)
 * with the speech bank on the stack (its voice [0x571124 + 12 bank]), eax
 * handed on -- by the dispatcher (edx 0: volume 110, centred) or by the
 * car's voice of the 8 (the switch at 0x40a55c: the phrase moved on by 0,
 * 0x10 or 0x20 by the voice, its pan), the mark as sayPhrase's; nothing in
 * a race of the network's.  ret 4. */
void sayCarPhrase(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.esi, cpu.edi, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x34;
    put(app, cpu.ebp - 0x10, cpu.eax);
    put(app, cpu.ebp - 0xc, cpu.edx);
    if (at(app, 0x6fd3a0) == 0)
    {
        if (at(app, cpu.ebp - 0xc) != 0)
        {
            voiceOfEight(app, cpu);
            // (mov eax / edx, pan;) mov edi, 0x3fff; mov / lea esi, phrase; mov [ebp - 8], pan
            const auto viaEax = [&](reg32 pan, reg32 step) {
                cpu.eax = pan;
                cpu.edi = 0x3fff;
                cpu.esi = cpu.ebx + step;
                put(app, cpu.ebp - 8, cpu.eax);
            };
            const auto viaEdx = [&](reg32 pan, reg32 step) {
                cpu.edx = pan;
                cpu.edi = 0x3fff;
                cpu.esi = cpu.ebx + step;
                put(app, cpu.ebp - 8, cpu.edx);
            };
            switch (cpu.edx)
            {
            case 0:
            case 6: viaEax(0x40, 0); break;     // 0x40a5cb
            case 1: viaEdx(0x40, 0x20); break;  // 0x40a5dc
            case 2: viaEax(0x40, 0x10); break;  // 0x40a5eb
            case 3: viaEdx(0x50, 0); break;     // 0x40a5fd
            case 4: viaEax(0x50, 0x20); break;  // 0x40a60b
            case 5: viaEdx(0x50, 0x10); break;  // 0x40a61d
            case 7: viaEdx(0x40, 0x10); break;  // 0x40a624
            default: break;
            }
        }
        else
        {
            cpu.edi = 0xc000;
            cpu.esi = cpu.ebx;
        }
        put(app, cpu.ebp - 0x30, at(app, cpu.ebp - 0xc) != 0 ? 0x7f : 0x6e);
        put(app, cpu.ebp - 0x2c, at(app, cpu.ebp - 0xc) != 0 ? at(app, cpu.ebp - 8) : 0x40);
        put(app, cpu.ebp - 0x28, cpu.edi);
        cpu.edi = at(app, cpu.ebp + 0x10);
        cpu.eax = cpu.edi * 4;
        cpu.edx = at(app, cpu.ebp - 0xc);
        cpu.eax -= cpu.edi;
        put(app, cpu.ebp - 0x34, cpu.esi);
        cpu.eax <<= 2;
        cpu.eax = at(app, cpu.eax + 0x571124);  // (test edx, edx before it: its flags never used)
        put(app, cpu.ebp - 0x24, cpu.eax);
        cpu.eax = at(app, cpu.ebp - 0x10);
        put(app, cpu.ebp - 0x20, cpu.eax);
        if (cpu.ebx == 0x21)
            put(app, cpu.ebp - 0x1c, at(app, cpu.ebp - 0xc) == 0 ? 7 : 6);
        else
            put(app, cpu.ebp - 0x1c, 5);
        cpu.eax = cpu.ebp - 0x34;
        put(app, cpu.ebp - 0x14, cpu.ecx);
        if (!byAddress(app, cpu, kSpeechLine))
            return;
    }
    cpu.esp = cpu.ebp;
    restore(app, cpu, {&cpu.ebp, &cpu.edi, &cpu.esi});
    cpu.esp += 8;
}

/* 0x40a6d0: the name of the radio's phrase eax (8 bytes at 0x56ec90 + 8
 * eax) from the text at edx, 7 at most (strncpy). */
void namePhrase(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ebx, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.eax <<= 3;
    cpu.ebx = 7;
    cpu.eax += 0x56ec90;
    if (!byAddress(app, cpu, kStrncpy))
        return;
    ret(app, cpu, {&cpu.ebp, &cpu.ebx});
}

/* 0x40a6f0: the radio's phrases named at the race's start (ALLU, u24, u31,
 * GO24, 10-4, inPrst, join, resume, abort, stopped, help, sight, ambul,
 * ticket, arrest, block, patrol, the cars Corv, Lamb, Jaguar, Merced in
 * their colours, speeder, 100+, 150+, 200+, donuts, school, town), then the
 * racers' speech banks loaded (loadRacerBanks). */
void namePhrases(App* app, CPU& cpu)
{
    static const reg32 kPhrases[][2] = {
        {0x536560, 0},    {0x536568, 1},    {0x53656c, 2},    {0x536570, 3},    {0x536578, 4},
        {0x536580, 5},    {0x536580, 6},    {0x536588, 7},    {0x536590, 8},    {0x536598, 9},
        {0x5365a0, 0xb},  {0x5365a8, 0xc},  {0x5365b0, 0xe},  {0x5365b8, 0xf},  {0x5365c0, 0x11},
        {0x5365c8, 0x12}, {0x5365d0, 0x13}, {0x5365d8, 0x15}, {0x5365e0, 0x16}, {0x5365e8, 0x17},
        {0x5365f0, 0x24}, {0x5365f8, 0x25}, {0x536600, 0x26}, {0x536608, 0x27}, {0x536610, 0x28},
        {0x536618, 0x29}, {0x536620, 0x2a}, {0x536628, 0x2b}, {0x536630, 0x2c}, {0x536638, 0x2d},
        {0x536640, 0x2e}, {0x536648, 0x2f}, {0x536650, 0x30}, {0x536658, 0x31}, {0x536660, 0x32},
    };
    save(app, cpu, {cpu.edx, cpu.ebp});
    cpu.ebp = cpu.esp;
    for (const auto& phrase : kPhrases)
    {
        cpu.edx = phrase[0];
        cpu.eax = phrase[1];
        if (!direct(app, cpu, namePhrase))
            return;
    }
    if (!direct(app, cpu, loadRacerBanks))
        return;
    ret(app, cpu, {&cpu.ebp, &cpu.edx});
}

/* 0x40a910: whether the radio is free for a message of weight eax (with the
 * radio's load [0x7dd378], under 12): 2 where edx is the car [0x5524f0], 1
 * else, 0 where it is not. */
void radioFree(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.eax += at(app, 0x7dd378);
    if (s(cpu.eax) >= 0xc)
        cpu.eax = 0;
    else
        cpu.eax = cpu.edx == at(app, 0x5524f0) ? 2 : 1;
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40a940, 0x40aea0, 0x40b7f0: empty -- the radio's message done, a car's
 * description's hook, and a message of nothing (push ebx and edx too). */
void radioNothing(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

void radioNothingToo(App* app, CPU& cpu)
{
    radioNothing(app, cpu);
}

void radioNothingThree(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ebx, cpu.edx, cpu.ebp});
    cpu.ebp = cpu.esp;
    ret(app, cpu, {&cpu.ebp, &cpu.edx, &cpu.ebx});
}

/* 0x40a950: the voice eax's frames since it spoke [0x56ee68] back to 0 where
 * edx is 1 (edx left 0 then). */
void quietVoice(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (cpu.edx == 1)
    {
        cpu.edx = 0;
        put(app, cpu.eax * 4 + 0x56ee68, cpu.edx);
    }
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40a970: the car eax's voice's reply where it did not speak last
 * [0x5524ec]: edx 1, now and then its own call (+0x714 + 0x14) and phrase
 * 0x20; edx 0, its +0x24 at random, else now and then its call and phrase
 * 0x22 (sayPhrase, by the dispatcher). */
void sayVoicePhrase(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ebx, cpu.ecx, cpu.esi, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.esi = cpu.eax;
    cpu.eax = at(app, 0x5524ec);
    const auto done = [&] { ret(app, cpu, {&cpu.ebp, &cpu.esi, &cpu.ecx, &cpu.ebx}); };
    if (cpu.eax == at(app, cpu.esi + 0x714))
        return done();
    // mov ecx, esi; mov ebx, [esi + 0x714]; xor edx, edx; xor eax, eax; add ebx, step
    const auto ownCall = [&](reg32 step) {
        cpu.ecx = cpu.esi;
        cpu.ebx = at(app, cpu.esi + 0x714);
        cpu.edx = 0;
        cpu.eax = 0;
        cpu.ebx += step;
    };
    if (cpu.edx != 0)
    {
        cpu.ebx = 0x64;
        cpu.edx = 0;
        if (!byAddress(app, cpu, kRandom))
            return;
        divide(cpu, cpu.ebx);
        if (cpu.edx > 0x32)
        {
            ownCall(0x14);
            if (!direct(app, cpu, sayPhrase))
                return;
        }
        cpu.ebx = 0x20;
    }
    else
    {
        cpu.ebx = 0x64;
        if (!byAddress(app, cpu, kRandom))
            return;
        divide(cpu, cpu.ebx);
        if (cpu.edx > 0x32)
        {
            ownCall(0x24);
            if (!direct(app, cpu, sayPhrase))
                return;
            return done();
        }
        cpu.edx = 0;
        if (!byAddress(app, cpu, kRandom))
            return;
        divide(cpu, cpu.ebx);
        if (cpu.edx > 0x32)
        {
            ownCall(0x14);
            if (!direct(app, cpu, sayPhrase))
                return;
        }
        cpu.ebx = 0x22;
    }
    cpu.ecx = cpu.esi;
    cpu.edx = 0;
    cpu.eax = 0;
    if (!direct(app, cpu, sayPhrase))
        return;
    done();
}

/* 0x40aa20: the car eax's call sign (its voice + 0x10) said by its voice,
 * where another spoke last or its voice has been quiet 120 frames
 * [0x5524cc], whose count starts again. */
void sayCallSign(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.ebx = at(app, 0x5524ec);
    cpu.edx = at(app, cpu.eax + 0x714);
    if (cpu.ebx != cpu.edx || s(at(app, cpu.edx * 4 + 0x5524cc)) >= 0x78)
    {
        cpu.ebx = at(app, cpu.eax + 0x714);
        cpu.esi = 0;
        put(app, cpu.ebx * 4 + 0x5524cc, cpu.esi);
        cpu.edx = 1;
        cpu.ebx = at(app, cpu.eax + 0x714);
        cpu.ecx = cpu.eax;
        cpu.ebx += 0x10;
        cpu.eax = 0;
        if (!direct(app, cpu, sayPhrase))
            return;
    }
    ret(app, cpu, {&cpu.ebp, &cpu.esi, &cpu.edx, &cpu.ecx, &cpu.ebx});
}

/* 0x40aa70: the phrase eax said by the dispatcher, of no car. */
void sayPhraseOnly(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ebx, cpu.ecx, cpu.edx, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.ebx = cpu.eax;
    cpu.ecx = 0;
    cpu.edx = 0;
    cpu.eax = 0;
    if (!direct(app, cpu, sayPhrase))
        return;
    ret(app, cpu, {&cpu.ebp, &cpu.edx, &cpu.ecx, &cpu.ebx});
}

/* 0x40aa90: the car eax's unit (its voice + 0x14) said by the dispatcher. */
void sayUnit(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ebx, cpu.ecx, cpu.edx, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    cpu.ebx = at(app, cpu.eax + 0x714);
    cpu.edx = 0;
    cpu.ebx += 0x14;
    cpu.eax = 0;
    if (!direct(app, cpu, sayPhrase))
        return;
    ret(app, cpu, {&cpu.ebp, &cpu.edx, &cpu.ecx, &cpu.ebx});
}

/* 0x40ab20: the message of kind ebx (8 to 35, the switch at 0x40aab0) said
 * about the car edx, by its voice (eax 1) or the dispatcher (eax 0): for 8
 * to 11 the chased car's description in its bank [0x56ee28] (sayCarPhrase,
 * phrases 0, 0x10, 0x11, 0x12; 11 with a "help" first, 0x24 on at random,
 * unless one is pending [0x5524f4]); else phrases of the kind, some picked
 * at random of two to four (0x4e0d88): 12 and 13 0x20.., 15 0x28.., 16
 * 0x30.., 19 0x2c.. (of three), 20 0x2c.., 24 0xc.., 25 0x1e.., 26 0x1d, 27
 * 0x34../0x37.., 28 0x3a../0x33.., 29 0x31 and 30 0x32 (a "help" pending
 * after), 31 to 33 0x37 or 0x39 to 0x3b, 34 0x3c or 0x35.., 35 0x1a... */
void sayDescription(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ecx, cpu.esi, cpu.edi, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.esi = cpu.eax;
    cpu.edi = cpu.edx;
    cpu.ebx -= 8;
    const auto done = [&] { ret(app, cpu, {&cpu.ebp, &cpu.edi, &cpu.esi, &cpu.ecx}); };
    // The chased car's bank: mov eax, [car + 0x624]; mov eax, [eax + 0x1f0];
    // its [eax*4 + 0x56ee28].
    const auto bank = [&](reg32 car) {
        cpu.eax = at(app, car + 0x624);
        cpu.eax = at(app, cpu.eax + 0x1f0);
        return at(app, cpu.eax * 4 + 0x56ee28);
    };
    // call 0x4e0d88; and eax, mask; mov ecx, edx; lea ebx, [eax + phrase];
    // mov edx, esi; xor eax, eax; call sayPhrase.
    const auto randomPhrase = [&](reg32 mask, reg32 phrase) {
        if (!byAddress(app, cpu, kRandom))
            return false;
        cpu.eax &= mask;
        cpu.ecx = cpu.edx;
        cpu.ebx = cpu.eax + phrase;
        cpu.edx = cpu.esi;
        cpu.eax = 0;
        return direct(app, cpu, sayPhrase);
    };
    // mov ebx, phrase; mov ecx, edx; mov edx, eax; (xor eax, eax;) call sayPhrase.
    const auto fixedPhrase = [&](reg32 phrase) {
        cpu.ebx = phrase;
        cpu.ecx = cpu.edx;
        cpu.edx = cpu.eax;
        cpu.eax = 0;
        return direct(app, cpu, sayPhrase);
    };
    // test eax, eax: the dispatcher's phrase 0x37 (eax 0, kept), else the
    // voice's `phrase`.
    const auto pickedPhrase = [&](reg32 phrase) {
        if (cpu.eax == 0)
        {
            cpu.ebx = 0x37;
            cpu.ecx = cpu.edx;
            cpu.edx = cpu.eax;
            return direct(app, cpu, sayPhrase);
        }
        return fixedPhrase(phrase);
    };
    if (cpu.ebx > 0x1b)
        return done();
    switch (cpu.ebx + 8)
    {
    case 8:  // 0x40ab3d
        if (cpu.esi == 0)
        {
            cpu.edx = bank(cpu.edi);
            cpu.ecx = cpu.edi;
            cpu.ebx = 0;
            push(app, cpu, cpu.edx);
            cpu.eax = 0;
            cpu.edx = cpu.esi;
        }
        else
        {
            const reg32 b = bank(cpu.edi);
            cpu.ebx = 0x10;
            cpu.ecx = b;
            cpu.edx = cpu.esi;
            push(app, cpu, cpu.ecx);
            cpu.eax = 0;
            cpu.ecx = cpu.edi;
        }
        if (!direct(app, cpu, sayCarPhrase))
            return;
        return done();
    case 9:  // 0x40ab88
        if (cpu.eax == 0)
        {
            const reg32 b = bank(cpu.edx);
            cpu.ecx = cpu.edx;
            cpu.ebx = b;
            cpu.edx = cpu.esi;
            push(app, cpu, cpu.ebx);
            cpu.eax = 0;
            cpu.ebx = 0;
        }
        else
        {
            const reg32 b = bank(cpu.edx);
            cpu.ebx = 0x11;
            cpu.edx = b;
            cpu.ecx = cpu.edi;
            push(app, cpu, cpu.edx);
            cpu.eax = 0;
            cpu.edx = cpu.esi;
        }
        if (!direct(app, cpu, sayCarPhrase))
            return;
        return done();
    case 10:  // 0x40abd3
    {
        const reg32 b = bank(cpu.edx);
        cpu.ecx = b;
        cpu.ebx = 0x12;
        push(app, cpu, cpu.ecx);
        cpu.eax = 0;
        cpu.ecx = cpu.edx;
        cpu.edx = cpu.esi;
        if (!direct(app, cpu, sayCarPhrase))
            return;
        return done();
    }
    case 11:  // 0x40ac17
        if (cpu.eax == 0)
        {
            const reg32 b = bank(cpu.edx);
            cpu.ecx = cpu.edi;
            cpu.ebx = b;
            cpu.edx = cpu.esi;
            push(app, cpu, cpu.ebx);
            cpu.eax = 0;
            cpu.ebx = 0;
        }
        else
        {
            if (at(app, 0x5524f4) == 0)
                if (!randomPhrase(3, 0x24))
                    return;
            const reg32 b = bank(cpu.edi);
            cpu.ebx = 0x11;
            cpu.edx = b;
            cpu.ecx = cpu.edi;
            push(app, cpu, cpu.edx);
            cpu.eax = 0;
            cpu.edx = cpu.esi;
        }
        if (!direct(app, cpu, sayCarPhrase))
            return;
        cpu.ecx = 0;
        put(app, 0x5524f4, cpu.ecx);
        return done();
    case 12:
    case 13:  // 0x40abfc
        if (!randomPhrase(3, 0x20))
            return;
        return done();
    case 15:  // 0x40acc5
        if (!randomPhrase(3, 0x28))
            return;
        return done();
    case 16:  // 0x40ace0
        if (!randomPhrase(3, 0x30))
            return;
        return done();
    case 19:  // 0x40ac89
        cpu.ebx = 3;
        cpu.edx = 0;
        if (!byAddress(app, cpu, kRandom))
            return;
        divide(cpu, cpu.ebx);
        cpu.ecx = cpu.edi;
        cpu.ebx = cpu.edx + 0x2c;
        cpu.eax = 0;
        cpu.edx = cpu.esi;
        if (!direct(app, cpu, sayPhrase))
            return;
        return done();
    case 20:  // 0x40acaa
        if (!randomPhrase(3, 0x2c))
            return;
        return done();
    case 24:  // 0x40acfb
        if (!randomPhrase(1, 0xc))
            return;
        return done();
    case 25:  // 0x40ae08
        if (!randomPhrase(1, 0x1e))
            return;
        return done();
    case 26:  // 0x40ae3e
        if (!fixedPhrase(0x1d))
            return;
        return done();
    case 27:  // 0x40ad6e
        if (!randomPhrase(1, cpu.eax == 0 ? 0x34 : 0x37))
            return;
        return done();
    case 28:  // 0x40ad16
        if (!randomPhrase(1, cpu.eax == 0 ? 0x3a : 0x33))
            return;
        return done();
    case 29:  // 0x40ae53
        if (!fixedPhrase(0x31))
            return;
        put(app, 0x5524f4, 1);
        return done();
    case 30:  // 0x40ae72
        cpu.ebx = 0x32;
        cpu.ecx = cpu.edx;
        cpu.edx = cpu.eax;
        cpu.eax = 0;
        cpu.esi = 1;
        if (!direct(app, cpu, sayPhrase))
            return;
        put(app, 0x5524f4, cpu.esi);
        return done();
    case 31:  // 0x40ad9c
        if (!pickedPhrase(0x39))
            return;
        return done();
    case 32:  // 0x40adc0
        if (!pickedPhrase(0x3a))
            return;
        return done();
    case 33:  // 0x40ade4
        if (!pickedPhrase(0x3b))
            return;
        return done();
    case 34:  // 0x40ad44
        if (cpu.eax == 0)
        {
            cpu.ebx = 0x3c;
            cpu.ecx = cpu.edx;
            cpu.edx = cpu.eax;
            if (!direct(app, cpu, sayPhrase))
                return;
        }
        else if (!randomPhrase(1, 0x35))
            return;
        return done();
    case 35:  // 0x40ae23
        if (!randomPhrase(1, 0x1a))
            return;
        return done();
    default:  // 0x40ae8d: 14, 17, 18, 21 to 23
        return done();
    }
}

/* 0x40aeb0: where the car edx is said to be, by its voice (eax 1) or the
 * dispatcher (eax 0): the dispatcher repeats the voice's last place (with
 * its prefix 0x20 on, the prefix's phrase first); else the place the car is
 * heading for, its node on by its speed / 6 times [0x7dd378] twice, mod the
 * track's nodes: the nearest named place there (0x4c8550, a record of the
 * frame's: its phrase, distance, kind) -- 0 or 0x20 on at random for kind 2
 * -- or else the next (0x4c8470), with "100+"-like prefixes 0x7f and 0x7e
 * for a road of kind 1 past 100 and 50; each voice's last place and prefix
 * noted [0x5524f8], [0x552520]. */
void sayLocation(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ebx, cpu.ecx, cpu.esi, cpu.edi, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x18;
    cpu.edi = cpu.eax;
    cpu.esi = cpu.edx;
    const auto done = [&] {
        cpu.esp = cpu.ebp;
        ret(app, cpu, {&cpu.ebp, &cpu.edi, &cpu.esi, &cpu.ecx, &cpu.ebx});
    };
    // mov ecx, esi; mov edx, edi; xor eax, eax; call sayPhrase -- ebx the phrase.
    const auto say = [&] {
        cpu.ecx = cpu.esi;
        cpu.edx = cpu.edi;
        cpu.eax = 0;
        return direct(app, cpu, sayPhrase);
    };
    const auto voice = [&] { return at(app, cpu.esi + 0x714); };
    if (cpu.eax == 0)
    {
        cpu.eax = at(app, cpu.edx + 0x714);
        cpu.eax <<= 2;
        if (at(app, cpu.eax + 0x5524f8) != 0xffffffff)
        {
            cpu.ecx = at(app, 0x5524ec);
            if (cpu.ecx == voice())
                return done();
            cpu.ebx = at(app, cpu.eax + 0x552520);
            if (cpu.ebx != 0)
            {
                if (!say())
                    return;
                cpu.eax = voice();
                cpu.ecx = cpu.esi;
                cpu.ebx = at(app, cpu.eax * 4 + 0x5524f8);
                cpu.edx = cpu.edi;
                cpu.ebx += 0x20;
            }
            else
            {
                cpu.ecx = cpu.esi;
                cpu.edx = cpu.edi;
                cpu.ebx = at(app, cpu.eax + 0x5524f8);
            }
            cpu.eax = 0;
            if (!direct(app, cpu, sayPhrase))
                return;
            cpu.eax = voice();
            put(app, cpu.eax * 4 + 0x5524f8, 0xffffffff);
            return done();
        }
    }
    fld(cpu, f32(app, cpu.esi + 0x7c0));
    if (!byAddress(app, cpu, kChop))
        return;
    put(app, cpu.ebp - 8, reg32(fp::toInteger<sreg32>(cpu.fpu.st(0))));
    cpu.fpu.pop();
    cpu.eax = at(app, cpu.ebp - 8);
    cpu.edx = s(cpu.eax) > 0 ? cpu.eax : 0 - cpu.eax;
    cpu.ecx = 6;
    cpu.eax = cpu.edx;
    cpu.edx = reg32(s(cpu.edx) >> 0x1f);
    {
        const std::int64_t n = std::int64_t(std::uint64_t(cpu.edx) << 32 | cpu.eax);
        cpu.eax = reg32(n / s(cpu.ecx));
        cpu.edx = reg32(n % s(cpu.ecx));
    }
    cpu.eax = cpu.eax * at(app, 0x7dd378);
    cpu.ecx = at(app, cpu.esi + 0x1c);
    cpu.eax += cpu.eax;
    cpu.ecx += cpu.eax;
    cpu.eax = cpu.ecx;
    cpu.edx = cpu.ecx;
    cpu.ecx = at(app, kNodeCount);
    cpu.edx = reg32(s(cpu.edx) >> 0x1f);
    {
        const std::int64_t n = std::int64_t(std::uint64_t(cpu.edx) << 32 | cpu.eax);
        cpu.eax = reg32(n / s(cpu.ecx));
        cpu.edx = reg32(n % s(cpu.ecx));
    }
    cpu.ecx = cpu.edx;
    cpu.edx = cpu.ebp - 0x18;
    cpu.eax = cpu.ecx;
    if (!byAddress(app, cpu, kNearestPlace))
        return;
    // The voice that spoke last is this one and its last place is the one.
    const auto again = [&] {
        cpu.edx = at(app, 0x5524ec);
        if (cpu.edx != voice())
            return false;
        cpu.edx = voice();
        cpu.eax = at(app, cpu.ebp - 0x18);
        return cpu.eax == at(app, cpu.edx * 4 + 0x5524f8);
    };
    if (cpu.eax != 0xffffffff)
    {
        cpu.ebx = at(app, cpu.ebp - 0x10);
        if (cpu.ebx == 2)
        {
            if (!byAddress(app, cpu, kRandom))
                return;
            cpu.eax &= 1;
            cpu.edx = at(app, cpu.ebp - 0x18);
            cpu.eax <<= 5;
            cpu.edx += cpu.eax;
            put(app, cpu.ebp - 4, cpu.edx);
        }
        else
        {
            cpu.eax = at(app, cpu.ebp - 0x18);
            put(app, cpu.ebp - 4, cpu.eax);
        }
        if (again())
            return done();
        cpu.ebx = at(app, cpu.ebp - 4);
        if (!say())
            return;
        cpu.eax = voice();
        cpu.edx = at(app, cpu.ebp - 4);
        put(app, cpu.eax * 4 + 0x5524f8, cpu.edx);
        cpu.eax = voice();
        cpu.ebx = 0;
        put(app, cpu.eax * 4 + 0x552520, cpu.ebx);
        return done();
    }
    cpu.edx = cpu.ebp - 0x18;
    cpu.eax = cpu.ecx;
    if (!byAddress(app, cpu, kOtherPlace))
        return;
    if (cpu.eax == 0)
        return done();
    if (again())
        return done();
    for (const reg32 prefix : {0x7fu, 0x7eu})
    {
        if (!(s(at(app, cpu.ebp - 0x14)) > (prefix == 0x7f ? 0x64 : 0x32) && at(app, cpu.ebp - 0x10) == 1))
            continue;
        cpu.ebx = prefix;
        if (!say())
            return;
        cpu.ebx = at(app, cpu.ebp - 0x18);
        cpu.ecx = cpu.esi;
        cpu.edx = cpu.edi;
        cpu.eax = 0;
        cpu.ebx += 0x20;
        if (!direct(app, cpu, sayPhrase))
            return;
        cpu.edx = voice();
        cpu.eax = at(app, cpu.ebp - 0x18);
        put(app, cpu.edx * 4 + 0x5524f8, cpu.eax);
        cpu.eax = voice();
        put(app, cpu.eax * 4 + 0x552520, prefix);
        return done();
    }
    cpu.eax = at(app, cpu.ebp - 0x10);
    if (cpu.eax == 0)
    {
        cpu.ebx = at(app, cpu.ebp - 0x18);
        cpu.ecx = cpu.esi;
        cpu.edx = cpu.edi;
        cpu.ebx += 0x20;
        if (!direct(app, cpu, sayPhrase))
            return;
        cpu.eax = at(app, cpu.ebp - 0x18);
        cpu.edx = voice();
        cpu.eax += 0x20;
        put(app, cpu.edx * 4 + 0x5524f8, cpu.eax);
    }
    else if (cpu.eax == 2)
    {
        if (!byAddress(app, cpu, kRandom))
            return;
        cpu.eax &= 1;
        cpu.edx = at(app, cpu.ebp - 0x18);
        cpu.eax <<= 5;
        cpu.edx += cpu.eax;
        cpu.ecx = cpu.esi;
        put(app, cpu.ebp - 0xc, cpu.edx);
        cpu.ebx = cpu.edx;
        cpu.eax = 0;
        cpu.edx = cpu.edi;
        if (!direct(app, cpu, sayPhrase))
            return;
        cpu.eax = voice();
        cpu.edx = at(app, cpu.ebp - 0xc);
        put(app, cpu.eax * 4 + 0x5524f8, cpu.edx);
    }
    else
    {
        cpu.ebx = at(app, cpu.ebp - 0x18);
        if (!say())
            return;
        cpu.edx = voice();
        cpu.eax = at(app, cpu.ebp - 0x18);
        put(app, cpu.edx * 4 + 0x5524f8, cpu.eax);
    }
    cpu.eax = voice();
    cpu.ecx = 0;
    put(app, cpu.eax * 4 + 0x552520, cpu.ecx);
    done();
}

/* 0x40b190: the speed of the car edx said for the car ebx (eax by whom):
 * the phrase 0x3f, 0x3e, 0x3d or 0x3c past [0x53666c], [0x536674],
 * [0x53667c], [0x536684]; none slower. */
void saySpeed(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ecx, cpu.esi, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.esp -= 8;
    cpu.esi = cpu.eax;
    cpu.ecx = cpu.ebx;
    fld(cpu, f32(app, cpu.edx + 0x7c4));
    fst64(app, cpu, cpu.ebp - 8);
    bool say = true;
    if (!fcomp(cpu, f64(app, 0x53666c)).belowEqual())
        cpu.ebx = 0x3f;
    else
    {
        fld(cpu, f64(app, cpu.ebp - 8));
        if (!fcomp(cpu, f64(app, 0x536674)).belowEqual())
            cpu.ebx = 0x3e;
        else
        {
            fld(cpu, f64(app, cpu.ebp - 8));
            if (!fcomp(cpu, f64(app, 0x53667c)).belowEqual())
                cpu.ebx = 0x3d;
            else
            {
                fld(cpu, f64(app, cpu.ebp - 8));
                if (!fcomp(cpu, f64(app, 0x536684)).belowEqual())
                    cpu.ebx = 0x3c;
                else
                    say = false;
            }
        }
    }
    if (say)
    {
        cpu.edx = cpu.esi;
        cpu.eax = 0;
        if (!direct(app, cpu, sayPhrase))
            return;
    }
    cpu.esp = cpu.ebp;
    ret(app, cpu, {&cpu.ebp, &cpu.esi, &cpu.ecx});
}

namespace
{

/* What the radio's messages call, as they call them: `call` with eax and
 * edx (and ebx) set. */
bool radio(App* app, CPU& cpu, void (*target)(App*, CPU&), reg32 eax, reg32 edx)
{
    cpu.edx = edx;
    cpu.eax = eax;
    return direct(app, cpu, target);
}

/* xor edx, edx; call 0x4e0d88; div r: the random number's remainder in edx. */
bool randomBelow(App* app, CPU& cpu, reg32 r)
{
    cpu.edx = 0;
    if (!byAddress(app, cpu, kRandom))
        return false;
    divide(cpu, r);
    return true;
}

/* mov ebx, ecx; mov edx, flag; xor eax, eax; call radioNoise. */
bool noise(App* app, CPU& cpu, reg32 flag)
{
    cpu.ebx = cpu.ecx;
    cpu.edx = flag;
    cpu.eax = 0;
    return direct(app, cpu, radioNoise);
}

/* The end of a message: mov eax, [ecx + 0x714]; mov [0x5524ec], eax (the
 * voice that spoke last); call radioNothing. */
bool spoke(App* app, CPU& cpu)
{
    cpu.eax = at(app, cpu.ecx + 0x714);
    put(app, 0x5524ec, cpu.eax);
    return direct(app, cpu, radioNothing);
}

}  // namespace

/* 0x40b220: a cop's sighting of the car it chases: the radio free (weight
 * 0), the voice's quiet count cleared, its call sign, the chased car's
 * description (kind 8), where it heads, its speed now and then, the noise;
 * then the dispatcher's answer (phrase 0x10, the unit, the description, the
 * place now and then), the noise, the voice noted and the frames since
 * anyone spoke cleared. */
void radioSpotted(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    cpu.esi = 0x64;
    const auto done = [&] { ret(app, cpu, {&cpu.ebp, &cpu.edi, &cpu.esi, &cpu.edx, &cpu.ecx, &cpu.ebx}); };
    if (!randomBelow(app, cpu, cpu.esi))
        return;
    cpu.edi = cpu.edx;
    if (!randomBelow(app, cpu, cpu.esi))
        return;
    cpu.esi = cpu.edx;
    if (!radio(app, cpu, radioFree, 0, 0))
        return;
    if (cpu.eax == 0)
        return done();
    if (!radio(app, cpu, quietVoice, at(app, cpu.ecx + 0x714), 1))
        return;
    cpu.eax = cpu.ecx;
    cpu.ebx = 8;
    if (!direct(app, cpu, sayCallSign) || !radio(app, cpu, sayDescription, 1, cpu.ecx)
        || !radio(app, cpu, radioNothingToo, 1, at(app, cpu.ecx + 0x624)) || !radio(app, cpu, sayLocation, 1, cpu.ecx))
        return;
    if (s(cpu.esi) > 0x32)
    {
        cpu.eax = 1;
        cpu.ebx = cpu.ecx;
        cpu.edx = at(app, cpu.ecx + 0x624);
        if (!direct(app, cpu, saySpeed))
            return;
    }
    if (!noise(app, cpu, 1))
        return;
    cpu.eax = 0x10;
    cpu.ebx = 8;
    if (!direct(app, cpu, sayPhraseOnly) || !radio(app, cpu, sayUnit, cpu.ecx, cpu.ecx))
        return;
    cpu.eax = 0;
    if (!direct(app, cpu, sayDescription))
        return;
    if (!radio(app, cpu, radioNothingToo, 0, at(app, cpu.ecx + 0x624)))
        return;
    if (s(cpu.edi) > 0x32)
        if (!radio(app, cpu, sayLocation, 0, cpu.ecx))
            return;
    if (!noise(app, cpu, 0))
        return;
    cpu.eax = at(app, cpu.ecx + 0x714);
    cpu.edx = 0;
    put(app, 0x5524ec, cpu.eax);
    put(app, 0x5524c8, cpu.edx);
    if (!direct(app, cpu, radioNothing))
        return;
    done();
}

/* 0x40b330: a cop's report that it is in pursuit of the cop car's chased
 * car (a cop's, +0x200 & 4; the radio free at weight 4): of the car it
 * leads (radioFree 2: kind 10, short) or with its description (kind 9 or
 * 10), place and, the radio quiet for 1000 frames, the dispatcher's answer
 * (phrase 0x11, the unit, kind 9, the place now and then), else the voice's
 * own reply. */
void radioPursuit(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ebx, cpu.ecx, cpu.edx, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    const auto done = [&] { ret(app, cpu, {&cpu.ebp, &cpu.edx, &cpu.ecx, &cpu.ebx}); };
    if (!radio(app, cpu, radioFree, 4, 1))
        return;
    cpu.ebx = cpu.eax;
    if (cpu.eax == 0)
        return done();
    cpu.eax = at(app, cpu.ecx + 0x624);
    if ((byte(app, cpu.eax + 0x200) & 4) == 0)
        return done();
    if (!radio(app, cpu, quietVoice, at(app, cpu.ecx + 0x714), 1))
        return;
    bool answered = false;
    if (cpu.ebx == 2)
    {
        cpu.eax = cpu.ecx;
        cpu.ebx = 0xa;
        if (!direct(app, cpu, sayCallSign) || !radio(app, cpu, sayDescription, 1, cpu.ecx))
            return;
        cpu.edx = 1;
        cpu.ebx = cpu.ecx;
        cpu.eax = 0;
        if (!direct(app, cpu, radioNoise))
            return;
    }
    else
    {
        cpu.eax = cpu.ecx;
        cpu.ebx = 0x64;
        if (!direct(app, cpu, sayCallSign) || !randomBelow(app, cpu, cpu.ebx))
            return;
        cpu.ebx = cpu.edx > 0x32 ? 9 : 0xa;
        if (!radio(app, cpu, sayDescription, 1, cpu.ecx) || !radio(app, cpu, sayLocation, 1, cpu.ecx))
            return;
        cpu.ebx = cpu.ecx;
        cpu.edx = 1;
        cpu.eax = 0;
        if (!direct(app, cpu, radioNoise))
            return;
        if (s(at(app, 0x5524c8)) > 0x3e8)
        {
            if (!radio(app, cpu, radioFree, 3, 1))
                return;
            if (cpu.eax != 0)
            {
                cpu.eax = 0x11;
                cpu.ebx = 9;
                if (!direct(app, cpu, sayPhraseOnly) || !radio(app, cpu, sayUnit, cpu.ecx, cpu.ecx))
                    return;
                cpu.eax = 0;
                if (!direct(app, cpu, sayDescription))
                    return;
                cpu.ebx = 0x64;
                if (!randomBelow(app, cpu, cpu.ebx))
                    return;
                if (cpu.edx > 0x32)
                    if (!radio(app, cpu, sayLocation, 0, cpu.ecx))
                        return;
                cpu.ebx = 0;
                put(app, 0x5524c8, cpu.ebx);
                answered = true;
            }
        }
    }
    if (!answered)
        if (!radio(app, cpu, sayVoicePhrase, cpu.ecx, 1))
            return;
    if (!noise(app, cpu, 0))
        return;
    cpu.eax = at(app, cpu.ecx + 0x714);
    put(app, 0x5524ec, cpu.eax);
    if (!direct(app, cpu, radioNothing))
        return;
    done();
}

/* 0x40b480: a cop's report that it lost the car (a cop's chased car, the
 * radio free at weight 3): no voice last; a voice quiet under 500 frames
 * just has its count cleared; else (not the car the radio's on) its call
 * sign, its reply of kind 0xc, the description, the noise, its reply 0x20,
 * the noise. */
void radioLost(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ebx, cpu.ecx, cpu.edx, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    const auto done = [&] { ret(app, cpu, {&cpu.ebp, &cpu.edx, &cpu.ecx, &cpu.ebx}); };
    if (!radio(app, cpu, radioFree, 3, 3))
        return;
    if (cpu.eax == 0)
        return done();
    cpu.edx = at(app, cpu.ecx + 0x624);
    if ((byte(app, cpu.edx + 0x200) & 4) == 0)
        return done();
    put(app, 0x5524ec, 0xffffffff);
    cpu.edx = at(app, cpu.ecx + 0x714);
    if (s(at(app, cpu.edx * 4 + 0x56ee68)) < 0x1f4)
    {
        if (!radio(app, cpu, quietVoice, cpu.edx, 0))
            return;
        return done();
    }
    if (cpu.eax != 2)
    {
        cpu.eax = cpu.ecx;
        if (!direct(app, cpu, sayCallSign))
            return;
        cpu.edx = 0;
        cpu.eax = cpu.ecx;
        cpu.ebx = 0xc;
        if (!direct(app, cpu, sayVoicePhrase) || !radio(app, cpu, sayDescription, 1, cpu.ecx) || !noise(app, cpu, 1)
            || !radio(app, cpu, sayVoicePhrase, cpu.ecx, 0x20) || !noise(app, cpu, 0))
            return;
    }
    if (!direct(app, cpu, radioNothing))
        return;
    done();
}

/* 0x40b530: a cop's call for backup on a cop's chased car (the radio free at
 * weight 3, 2): its call sign, its reply of kind 0xb, the description, the
 * place or the speed, the noise; now and then, or the radio quiet 1000
 * frames, the dispatcher's answer (phrase 0x11, the unit, the description,
 * the place now and then) where the radio is free, else the voice's reply. */
void radioBackup(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    cpu.esi = 0x64;
    const auto done = [&] { ret(app, cpu, {&cpu.ebp, &cpu.esi, &cpu.edx, &cpu.ecx, &cpu.ebx}); };
    if (!randomBelow(app, cpu, cpu.esi))
        return;
    cpu.eax = 3;
    cpu.esi = cpu.edx;
    cpu.edx = 2;
    if (!direct(app, cpu, radioFree))
        return;
    if (cpu.eax == 0)
        return done();
    cpu.eax = at(app, cpu.ecx + 0x624);
    if ((byte(app, cpu.eax + 0x200) & 4) == 0)
        return done();
    if (!radio(app, cpu, quietVoice, at(app, cpu.ecx + 0x714), 1))
        return;
    cpu.eax = cpu.ecx;
    if (!direct(app, cpu, sayCallSign))
        return;
    cpu.edx = 0;
    cpu.eax = cpu.ecx;
    cpu.ebx = 0xb;
    if (!direct(app, cpu, sayVoicePhrase) || !radio(app, cpu, sayDescription, 1, cpu.ecx))
        return;
    if (s(cpu.esi) > 0x32)
    {
        if (!radio(app, cpu, sayLocation, 1, cpu.ecx))
            return;
    }
    else
    {
        cpu.eax = 1;
        cpu.ebx = cpu.ecx;
        cpu.edx = at(app, cpu.ecx + 0x624);
        if (!direct(app, cpu, saySpeed))
            return;
    }
    if (!noise(app, cpu, 1))
        return;
    cpu.ebx = 0x64;
    if (!randomBelow(app, cpu, cpu.ebx))
        return;
    bool answered = false;
    if (cpu.edx > 0x32 || s(at(app, 0x5524c8)) > 0x3e8)
    {
        if (!radio(app, cpu, radioFree, 3, 4))
            return;
        if (cpu.eax != 0)
        {
            cpu.eax = 0x11;
            cpu.ebx = 0xb;
            if (!direct(app, cpu, sayPhraseOnly) || !radio(app, cpu, sayUnit, cpu.ecx, cpu.ecx))
                return;
            cpu.eax = 0;
            if (!direct(app, cpu, sayDescription))
                return;
            if (s(cpu.esi) > 0x32)
                if (!radio(app, cpu, sayLocation, 0, cpu.ecx))
                    return;
            cpu.ebx = 0;
            put(app, 0x5524c8, cpu.ebx);
            answered = true;
        }
    }
    if (!answered)
        if (!radio(app, cpu, sayVoicePhrase, cpu.ecx, 1))
            return;
    if (!noise(app, cpu, 0) || !spoke(app, cpu))
        return;
    done();
}

/* 0x40b670: a cop on patrol, the car it chases not a cop's (the radio free
 * at weight 4): no voice last, its call sign, its reply, the description of
 * kind 0x13 (0x14 for a human's car on a pad), the noise, its reply, the
 * noise. */
void radioPatrol(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ebx, cpu.ecx, cpu.edx, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    const auto done = [&] { ret(app, cpu, {&cpu.ebp, &cpu.edx, &cpu.ecx, &cpu.ebx}); };
    if (!radio(app, cpu, radioFree, 4, 4))
        return;
    if (cpu.eax == 0)
        return done();
    cpu.eax = at(app, cpu.ecx + 0x624);
    if ((byte(app, cpu.eax + 0x200) & 4) != 0)
        return done();
    cpu.edx = 0xffffffff;
    cpu.eax = cpu.ecx;
    put(app, 0x5524ec, cpu.edx);
    if (!direct(app, cpu, sayCallSign) || !radio(app, cpu, sayVoicePhrase, cpu.ecx, 0))
        return;
    cpu.eax = at(app, cpu.ecx + 0x624);
    cpu.ebx = (byte(app, cpu.eax + 0x200) & 1) != 0 && at(app, cpu.eax + 0x2c4) == 4 ? 0x14 : 0x13;
    if (!radio(app, cpu, sayDescription, 1, cpu.ecx) || !noise(app, cpu, 1)
        || !radio(app, cpu, sayVoicePhrase, cpu.ecx, 1) || !noise(app, cpu, 0) || !direct(app, cpu, radioNothing))
        return;
    done();
}

/* 0x40b720: a roadblock (the radio free at weight 4): no voice last, the
 * call sign, the description of kind 0xf, the place, the noise; the
 * dispatcher's answer (phrase 0x12, the unit, kind 0x10, the place) where
 * the radio is still free, else the voice's reply; the noise. */
void radioRoadblock(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ebx, cpu.ecx, cpu.edx, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    const auto done = [&] { ret(app, cpu, {&cpu.ebp, &cpu.edx, &cpu.ecx, &cpu.ebx}); };
    if (!radio(app, cpu, radioFree, 4, 4))
        return;
    if (cpu.eax == 0)
        return done();
    cpu.edx = 0xffffffff;
    cpu.ebx = 0xf;
    cpu.eax = cpu.ecx;
    put(app, 0x5524ec, cpu.edx);
    if (!direct(app, cpu, sayCallSign) || !radio(app, cpu, sayDescription, 1, cpu.ecx)
        || !radio(app, cpu, sayLocation, 1, cpu.ecx) || !noise(app, cpu, 1) || !radio(app, cpu, radioFree, 4, 4))
        return;
    if (cpu.eax != 0)
    {
        cpu.eax = 0x12;
        cpu.ebx = 0x10;
        if (!direct(app, cpu, sayPhraseOnly) || !radio(app, cpu, sayUnit, cpu.ecx, cpu.ecx))
            return;
        cpu.eax = 0;
        if (!direct(app, cpu, sayDescription))
            return;
        cpu.edx = cpu.ecx;
        cpu.eax = 0;
        cpu.ebx = 0;
        if (!direct(app, cpu, sayLocation))
            return;
        put(app, 0x5524c8, cpu.ebx);
    }
    else if (!radio(app, cpu, sayVoicePhrase, cpu.ecx, 1))
        return;
    if (!noise(app, cpu, 0) || !spoke(app, cpu))
        return;
    done();
}

namespace
{

/* 0x40b860 and 0x40b910, of the kinds 0x1c and 0x1b: the radio free at
 * weight 3, the call sign, the voice's reply and the description of the
 * kind, the noise; the dispatcher's unit and description where the radio is
 * free at weight 2, else the voice's reply; the noise. */
void radioOfKind(App* app, CPU& cpu, reg32 kind)
{
    save(app, cpu, {cpu.ebx, cpu.ecx, cpu.edx, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    const auto done = [&] { ret(app, cpu, {&cpu.ebp, &cpu.edx, &cpu.ecx, &cpu.ebx}); };
    if (!radio(app, cpu, radioFree, 3, 4))
        return;
    if (cpu.eax == 0)
        return done();
    cpu.eax = cpu.ecx;
    if (!direct(app, cpu, sayCallSign))
        return;
    cpu.edx = 0;
    cpu.eax = cpu.ecx;
    cpu.ebx = kind;
    if (!direct(app, cpu, sayVoicePhrase) || !radio(app, cpu, sayDescription, 1, cpu.ecx) || !noise(app, cpu, 1)
        || !radio(app, cpu, radioFree, 2, 4))
        return;
    if (cpu.eax != 0)
    {
        cpu.eax = cpu.ecx;
        cpu.ebx = kind;
        if (!direct(app, cpu, sayUnit) || !radio(app, cpu, sayDescription, 0, cpu.ecx))
            return;
        cpu.edx = 0;
        put(app, 0x5524c8, cpu.edx);
    }
    else if (!radio(app, cpu, sayVoicePhrase, cpu.ecx, 1))
        return;
    if (!noise(app, cpu, 0) || !spoke(app, cpu))
        return;
    done();
}

}  // namespace

/* 0x40b860: a ticket (kind 0x1c). */
void radioTicket(App* app, CPU& cpu)
{
    radioOfKind(app, cpu, 0x1c);
}

/* 0x40b910: an arrest (kind 0x1b). */
void radioArrest(App* app, CPU& cpu)
{
    radioOfKind(app, cpu, 0x1b);
}

/* 0x40b9c0: the car eax stopped, edx how (kinds 0x20, 0x1f -- said by the
 * dispatcher -- and 0x21): the radio free at weight 4, the call sign, the
 * voice's reply, the description, the place, the noise; the radio quiet 1000
 * frames and free, the dispatcher's phrase 0x10 and the description again,
 * else the voice's reply; the noise.  edx is not kept (no push of it). */
void radioStopped(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ebx, cpu.ecx, cpu.esi, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    cpu.esi = cpu.edx;
    const auto done = [&] { ret(app, cpu, {&cpu.ebp, &cpu.esi, &cpu.ecx, &cpu.ebx}); };
    if (!radio(app, cpu, radioFree, 4, 4))
        return;
    if (cpu.eax == 0)
        return done();
    cpu.eax = cpu.ecx;
    if (!direct(app, cpu, sayCallSign))
        return;
    cpu.edx = 0;
    cpu.eax = cpu.ecx;
    if (!direct(app, cpu, sayVoicePhrase))
        return;
    if (cpu.esi == 1)
    {
        cpu.ebx = 0x1f;
        cpu.edx = cpu.ecx;
        cpu.eax = cpu.esi;
    }
    else
    {
        cpu.ebx = cpu.esi == 0 ? 0x20 : 0x21;
        cpu.eax = 1;
        cpu.edx = cpu.ecx;
    }
    if (!direct(app, cpu, sayDescription) || !radio(app, cpu, sayLocation, 1, cpu.ecx) || !noise(app, cpu, 1))
        return;
    bool answered = false;
    if (s(at(app, 0x5524c8)) > 0x3e8)
    {
        if (!radio(app, cpu, radioFree, 3, 1))
            return;
        if (cpu.eax != 0)
        {
            cpu.eax = 0x10;
            if (!direct(app, cpu, sayPhraseOnly))
                return;
            cpu.ebx = cpu.esi == 0 ? 0x20 : cpu.esi == 1 ? 0x1f : 0x21;
            if (!radio(app, cpu, sayDescription, 0, cpu.ecx))
                return;
            cpu.ebx = 0;
            put(app, 0x5524c8, cpu.ebx);
            answered = true;
        }
    }
    if (!answered)
        if (!radio(app, cpu, sayVoicePhrase, cpu.ecx, 1))
            return;
    if (!noise(app, cpu, 0) || !spoke(app, cpu))
        return;
    done();
}

/* 0x40bac0: an ambulance called (kind 0x22): the radio free at weight 4,
 * the call sign, the voice's reply, the description, the place, the noise;
 * the radio quiet 1000 frames and free, the dispatcher's phrase 0x10, the
 * description, the place now and then, else the voice's reply; the noise. */
void radioAmbulance(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ebx, cpu.ecx, cpu.edx, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    const auto done = [&] { ret(app, cpu, {&cpu.ebp, &cpu.edx, &cpu.ecx, &cpu.ebx}); };
    if (!radio(app, cpu, radioFree, 4, 4))
        return;
    if (cpu.eax == 0)
        return done();
    cpu.eax = cpu.ecx;
    if (!direct(app, cpu, sayCallSign))
        return;
    cpu.edx = 0;
    cpu.eax = cpu.ecx;
    cpu.ebx = 0x22;
    if (!direct(app, cpu, sayVoicePhrase) || !radio(app, cpu, sayDescription, 1, cpu.ecx)
        || !radio(app, cpu, sayLocation, 1, cpu.ecx) || !noise(app, cpu, 1))
        return;
    bool answered = false;
    if (s(at(app, 0x5524c8)) > 0x3e8)
    {
        if (!radio(app, cpu, radioFree, 3, 1))
            return;
        if (cpu.eax != 0)
        {
            cpu.eax = 0x10;
            cpu.ebx = 0x22;
            if (!direct(app, cpu, sayPhraseOnly) || !radio(app, cpu, sayDescription, 0, cpu.ecx))
                return;
            cpu.ebx = 0x64;
            if (!randomBelow(app, cpu, cpu.ebx))
                return;
            if (cpu.edx > 0x32)
                if (!radio(app, cpu, sayLocation, 0, cpu.ecx))
                    return;
            cpu.ebx = 0;
            put(app, 0x5524c8, cpu.ebx);
            answered = true;
        }
    }
    if (!answered)
        if (!radio(app, cpu, sayVoicePhrase, cpu.ecx, 1))
            return;
    if (!noise(app, cpu, 0) || !spoke(app, cpu))
        return;
    done();
}

/* 0x40bba0: spikes laid (kind 0x19, now and then 0x1a and the place too): the
 * radio free at weight 4, the call sign, the voice's reply, the noise. */
void radioSpikes(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ebx, cpu.ecx, cpu.edx, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    const auto done = [&] { ret(app, cpu, {&cpu.ebp, &cpu.edx, &cpu.ecx, &cpu.ebx}); };
    if (!radio(app, cpu, radioFree, 4, 4))
        return;
    if (cpu.eax == 0)
        return done();
    cpu.eax = cpu.ecx;
    if (!direct(app, cpu, sayCallSign))
        return;
    cpu.edx = 0;
    cpu.eax = cpu.ecx;
    cpu.ebx = 0x19;
    if (!direct(app, cpu, sayVoicePhrase) || !radio(app, cpu, sayDescription, 1, cpu.ecx))
        return;
    cpu.ebx = 0x64;
    if (!randomBelow(app, cpu, cpu.ebx))
        return;
    if (cpu.edx > 0x32)
    {
        cpu.ebx = 0x1a;
        if (!radio(app, cpu, sayDescription, 1, cpu.ecx) || !radio(app, cpu, sayLocation, 1, cpu.ecx))
            return;
    }
    if (!noise(app, cpu, 1) || !spoke(app, cpu))
        return;
    done();
}

namespace
{

/* 0x40bc30, 0x40bca0, 0x40bd00: a cop's short message of the kind (the
 * radio free at weight 3): the call sign, the voice's reply and the
 * description of the kind, the place where `place`, the noise. */
void radioShort(App* app, CPU& cpu, reg32 kind, bool place)
{
    save(app, cpu, {cpu.ebx, cpu.ecx, cpu.edx, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    const auto done = [&] { ret(app, cpu, {&cpu.ebp, &cpu.edx, &cpu.ecx, &cpu.ebx}); };
    if (!radio(app, cpu, radioFree, 3, 4))
        return;
    if (cpu.eax == 0)
        return done();
    cpu.eax = cpu.ecx;
    if (!direct(app, cpu, sayCallSign))
        return;
    cpu.edx = 0;
    cpu.eax = cpu.ecx;
    cpu.ebx = kind;
    if (!direct(app, cpu, sayVoicePhrase) || !radio(app, cpu, sayDescription, 1, cpu.ecx))
        return;
    if (place && !radio(app, cpu, sayLocation, 1, cpu.ecx))
        return;
    if (!noise(app, cpu, 1) || !spoke(app, cpu))
        return;
    done();
}

}  // namespace

/* 0x40bc30: a call for help (kind 0x23, with the place). */
void radioHelp(App* app, CPU& cpu)
{
    radioShort(app, cpu, 0x23, true);
}

/* 0x40bca0: the pursuit resumed (kind 0x1d). */
void radioResume(App* app, CPU& cpu)
{
    radioShort(app, cpu, 0x1d, false);
}

/* 0x40bd00: the pursuit called off (kind 0x1e). */
void radioAbort(App* app, CPU& cpu)
{
    radioShort(app, cpu, 0x1e, false);
}

namespace
{

/* fmul st(i): st(0) = st(0) * st(i). */
void fmulSt(CPU& cpu, int i) { cpu.fpu.st(0) = fp::mul(cpu.fpu.st(0), cpu.fpu.st(i)); }

}  // namespace

/* 0x40bd80: the network's car eax put back on the track by the radar's word
 * edx (radarFactor's record: how far behind, +0xa's high word, cut to -200
 * .. 200 past 200): onto the node that far on (0x428d60, wrapped round the
 * track's nodes), raised by [0x5efdd0] (by [0x53668c] in a tunnel,
 * 0x420bf0), its speed the radar's (+8, ahead of the player [0x6fd518]) or
 * the computer's at the node (0x40e080) along the node's heading bytes
 * (+0x14..+0x16, by [0x536694]); settled (0x428970), its frames +0x174 and
 * +0x524, and ticketed (0x4c7fd0) -- in a race under way [0x7d3680] where
 * the car is not being put back already (+0x184). */
void resetOnTrack(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ebx, cpu.ecx, cpu.esi, cpu.edi, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x14;
    cpu.ecx = cpu.eax;
    cpu.ebx = cpu.edx;
    const auto done = [&] {
        cpu.esp = cpu.ebp;
        ret(app, cpu, {&cpu.ebp, &cpu.edi, &cpu.esi, &cpu.ecx, &cpu.ebx});
    };
    if (at(app, 0x7d3680) == 0 || at(app, cpu.eax + 0x184) != 0)
        return done();
    const auto behind = [&] { return reg32(s(at(app, cpu.ebx + 0xa)) >> 0x10); };
    cpu.eax = behind();
    if (sreg16(half(app, cpu.ebx + 0xc)) <= 0)
        cpu.eax = 0 - cpu.eax;
    if (s(cpu.eax) >= 0xc8)
    {
        cpu.eax = sreg16(half(app, cpu.ebx + 0xc)) > 0xc8 ? 0xc8 : behind();
        app->getMemory<reg16>(cpu.ebx + 0xc) = cpu.ax;
        cpu.eax = behind();
        if (!(s(cpu.eax) > s(0xffffff38)))
            cpu.eax = 0xffffff38;
        app->getMemory<reg16>(cpu.ebx + 0xc) = cpu.ax;
    }
    cpu.eax = behind();
    cpu.edx = at(app, cpu.ecx + 0x1c);
    cpu.esi = cpu.eax;
    cpu.edx -= cpu.eax;
    cpu.esi = 0 - cpu.esi;
    cpu.eax = cpu.edx;
    if (s(cpu.esi) >= 0)
    {
        cpu.edi = at(app, kNodeCount);
        if (s(cpu.edx) >= s(cpu.edi))
            cpu.eax -= cpu.edi;
    }
    else if (s(cpu.edx) < 0)
    {
        cpu.esi = at(app, kNodeCount);
        cpu.eax = cpu.edx + cpu.esi;
    }
    cpu.edx = cpu.eax;
    cpu.eax = cpu.ecx;
    if (!byAddress(app, cpu, kPlaceOnNode))
        return;
    cpu.edi = at(app, cpu.ecx + 0x1c);
    cpu.eax = at(app, 0x5efdd0);
    push(app, cpu, cpu.edi);
    put(app, cpu.ebp - 8, cpu.eax);
    cpu.eax = at(app, cpu.ecx + 0xa0);
    push(app, cpu, cpu.eax);
    cpu.edx = at(app, cpu.ecx + 0x9c);
    push(app, cpu, cpu.edx);
    cpu.esi = at(app, cpu.ecx + 0x98);
    push(app, cpu, cpu.esi);
    if (!byAddress(app, cpu, kInTunnel))
        return;
    if (cpu.eax != 0)
    {
        fld(cpu, f32(app, cpu.ebp - 8));
        fmul(cpu, f32(app, 0x53668c));
        fstp(app, cpu, cpu.ebp - 8);
    }
    fld(cpu, f32(app, cpu.ecx + 0x9c));
    fadd(cpu, f32(app, cpu.ebp - 8));
    fstp(app, cpu, cpu.ecx + 0x9c);
    cpu.di = half(app, cpu.ebx + 0xc);
    bool radar = sreg16(cpu.di) > 0;
    if (radar)
    {
        cpu.edi = at(app, 0x6fd518);
        radar = s(cpu.edi) > s(at(app, cpu.ecx + 0x1f0));
    }
    if (radar)
    {
        cpu.eax = at(app, cpu.ebx + 8);
        put(app, cpu.ebp - 0xc, cpu.eax);
    }
    else
    {
        cpu.eax = at(app, cpu.ecx + 0x1c);
        if (!byAddress(app, cpu, kSliceSpeed))
            return;
        fstp(app, cpu, cpu.ebp - 0xc);
    }
    // The node's heading byte at +off, signed, times 512.
    const auto heading = [&](reg32 off) {
        cpu.eax = at(app, cpu.edx + cpu.eax * 4 + off);
        cpu.eax = reg32(s(cpu.eax) >> 0x18);
        cpu.eax <<= 9;
        put(app, cpu.ebp - 4, cpu.eax);
    };
    cpu.edx = at(app, cpu.ecx + 0x1c);
    cpu.eax = cpu.edx * 8;
    cpu.eax += cpu.edx;
    cpu.edx = at(app, kNodes);
    heading(0x11);
    cpu.ebx = at(app, cpu.ecx + 0x1c);
    fld(cpu, f64(app, 0x536694));
    fild(app, cpu, cpu.ebp - 4);
    fmulSt(cpu, 1);
    cpu.eax = cpu.ebx * 8;
    fld(cpu, f32(app, cpu.ebp - 0xc));
    fxch(cpu, 1);
    fmulSt(cpu, 1);
    cpu.eax += cpu.ebx;
    fstp(app, cpu, cpu.ecx + 0xa4);
    heading(0x12);
    cpu.ebx = at(app, cpu.ecx + 0x1c);
    fild(app, cpu, cpu.ebp - 4);
    fmulSt(cpu, 2);
    cpu.eax = cpu.ebx * 8;
    fmulSt(cpu, 1);
    cpu.eax += cpu.ebx;
    fstp(app, cpu, cpu.ecx + 0xa8);
    heading(0x13);
    fild(app, cpu, cpu.ebp - 4);
    fmulp(cpu, 2);
    fmulp(cpu, 1);
    cpu.eax = cpu.ecx;
    fstp(app, cpu, cpu.ecx + 0xac);
    if (!byAddress(app, cpu, kSettle))
        return;
    put(app, cpu.ecx + 0x174, 0x40);
    cpu.eax = at(app, cpu.ecx + 0x1f0);
    put(app, cpu.ecx + 0x524, 0x20);
    if (!byAddress(app, cpu, kTicketed))
        return;
    done();
}

/* 0x40bf50: the cop eax started again, while it is active: the chases'
 * scores at 0x56ea58 cleared (20 bytes), its marks of being chased and of
 * the radio cleared, its state 1 but where it is 1 already, then steered,
 * its target speed, the scores, driven. */
void copRestart(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ebx, cpu.ecx, cpu.edx, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    if (!byAddress(app, cpu, kCopActive))
        return;
    if (cpu.eax != 0)
    {
        cpu.ebx = 0x14;
        cpu.eax = 0x56ea58;
        cpu.edx = 0;
        if (!byAddress(app, cpu, kMemset))
            return;
        cpu.dx = half(app, cpu.ecx + 0x620);
        cpu.edx &= 0xffffdf7f;
        cpu.eax = at(app, cpu.ecx + 0x6e0);
        app->getMemory<reg16>(cpu.ecx + 0x620) = cpu.dx;
        if (cpu.eax == 0 || cpu.eax != 1)
            put(app, cpu.ecx + 0x6e0, 1);
        const auto call = [&](reg32 target) {
            cpu.eax = cpu.ecx;
            return byAddress(app, cpu, target);
        };
        const auto own = [&](void (*target)(App*, CPU&)) {
            cpu.eax = cpu.ecx;
            return direct(app, cpu, target);
        };
        if (!call(kCopSteer) || !own(targetSpeed) || !own(scoreSpeeds) || !own(scoreChases) || !own(scoreLane)
            || !call(kCopDrive) || !call(kCopFinish))
            return;
    }
    ret(app, cpu, {&cpu.ebp, &cpu.edx, &cpu.ecx, &cpu.ebx});
}

/* 0x40bfe0: the traffic's top speed onto the x87's stack ([0x53669c]). */
void topSpeedFactor(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    fld(cpu, f64(app, 0x53669c));
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x40bff0: the target speed [+0x7b0] of the car eax by the chased car's
 * [0x56ef3c]: the two going opposite ways, times [0x5366a4]; else, past the
 * chased car's speed, kept to it -- the chased car's speed ± its share
 * (level - [0x56ef48]) * [0x5366c4] / level of the gap, level its speed
 * times [0x5366ac] + [0x5366b4], no less than 4 -- where the level reaches
 * the chase's and the chased car is past [0x5366bc]. */
void followChased(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x14;
    cpu.edx = cpu.eax;
    cpu.ecx = at(app, 0x56ef3c);
    const auto done = [&] {
        cpu.esp = cpu.ebp;
        ret(app, cpu, {&cpu.ebp, &cpu.edi, &cpu.esi, &cpu.edx, &cpu.ecx, &cpu.ebx});
    };
    if (cpu.ecx == 0)
        return done();
    fld(cpu, f32(app, cpu.eax + 0x7c0));
    fld(cpu, Float(0.0));
    cpu.ebx = fcompp(cpu).belowEqual() ? 0 : 1;
    fld(cpu, f32(app, cpu.ecx + 0x7c0));
    fld(cpu, Float(0.0));
    cpu.eax = fcompp(cpu).belowEqual() ? 0 : 1;
    cpu.eax ^= cpu.ebx;
    if (cpu.eax != 0)
    {
        fld(cpu, f32(app, cpu.edx + 0x7b0));
        fld(cpu, cpu.fpu.st(0));
        fmul(cpu, f64(app, 0x5366a4));
        fstp1(cpu);
        fstp(app, cpu, cpu.edx + 0x7b0);
        return done();
    }
    fld(cpu, Float(0.0));
    if (!fcomp(cpu, f32(app, cpu.edx + 0x7b0)).aboveEqual())
    {
        cpu.eax = at(app, cpu.edx + 0x7b0);
        put(app, cpu.ebp - 0xc, cpu.eax);
    }
    else
    {
        fld(cpu, f32(app, cpu.edx + 0x7b0));
        fchs(cpu);
        fstp(app, cpu, cpu.ebp - 0xc);
    }
    fld(cpu, f32(app, cpu.ebp - 0xc));
    fsub(cpu, f32(app, cpu.ecx + 0x7c4));
    fld(cpu, Float(0.0));
    if (fcompp(cpu).aboveEqual())
        return done();
    fld(cpu, f32(app, cpu.ecx + 0x7c0));
    fmul(cpu, f64(app, 0x5366ac));
    fadd(cpu, f64(app, 0x5366b4));
    fistpToEax(app, cpu);
    if (!(s(cpu.eax) > 0))
        cpu.eax = 0 - cpu.eax;
    if (s(cpu.eax) < 4)
        cpu.eax = 4;
    cpu.esi = at(app, 0x56ef48);
    cpu.ebx = cpu.eax;
    if (s(cpu.eax) < s(cpu.esi))
        return done();
    fld(cpu, f32(app, cpu.ecx + 0x7c4));
    if (fcomp(cpu, f64(app, 0x5366bc)).below())
        return done();
    cpu.edi = at(app, 0x56ef48);
    cpu.eax = cpu.ebx;
    cpu.eax -= cpu.edi;
    put(app, cpu.ebp - 4, cpu.eax);
    fild(app, cpu, cpu.ebp - 4);
    fmul(cpu, f64(app, 0x5366c4));
    put(app, cpu.ebp - 4, cpu.ebx);
    fild(app, cpu, cpu.ebp - 4);
    fdivp(cpu, 1);
    fld(cpu, f32(app, cpu.edx + 0x7b0));
    fld(cpu, Float(0.0));
    fxch(cpu, 2);
    fstp(app, cpu, cpu.ebp - 8);
    fxch(cpu, 1);
    const bool down = !fcompp(cpu).aboveEqual();
    fld(cpu, f32(app, cpu.ecx + 0x7c0));
    if (down)
        fsub(cpu, f32(app, cpu.ebp - 8));
    else
        fadd(cpu, f32(app, cpu.ebp - 8));
    fstp(app, cpu, cpu.edx + 0x7b0);
    done();
}

/* 0x40c160: the node of the car eax set to the first free one (0x405c80)
 * from the car it follows [+0x6d8] (0x40e160), 61 nodes on and 20 more each
 * time -- backwards where the car followed goes backwards against a car not
 * against the traffic, or forwards slower than [0x5366cc]. */
void followCar(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    cpu.edx = at(app, cpu.eax + 0x6d8);
    cpu.ebx = 1;
    if (cpu.edx != 0)
    {
        cpu.esi = at(app, cpu.edx + 0x5f4);
        cpu.ebx = cpu.edx;
        if (cpu.esi != 0)
        {
            if ((byte(app, cpu.eax + 0x621) & 0x10) == 0)
                cpu.ebx = 0xffffffff;
            else
            {
                fld(cpu, f32(app, cpu.edx + 0x7c4));
                cpu.ebx = fcomp(cpu, f64(app, 0x5366cc)).above() ? 0xffffffff : 1;
            }
        }
        else if ((byte(app, cpu.eax + 0x621) & 0x10) != 0)
            cpu.ebx = 1;
        else
        {
            fld(cpu, f32(app, cpu.ebx + 0x7c4));
            cpu.ebx = fcomp(cpu, f64(app, 0x5366cc)).belowEqual() ? 0xffffffff : 1;
        }
    }
    cpu.esi = cpu.ebx;
    cpu.esi <<= 4;
    cpu.esi -= cpu.ebx;
    cpu.esi <<= 2;
    cpu.esi += cpu.ebx;
    do
    {
        cpu.edx = cpu.esi;
        cpu.eax = at(app, cpu.ecx + 0x6d8);
        if (!byAddress(app, cpu, kNodeAhead))
            return;
        cpu.edx = cpu.eax;
        cpu.eax = cpu.ebx * 4;
        cpu.eax += cpu.ebx;
        cpu.eax <<= 2;
        cpu.esi += cpu.eax;
        cpu.eax = cpu.edx;
        if (!byAddress(app, cpu, kNodeFree))
            return;
    } while (cpu.eax != 0);
    put(app, cpu.ecx + 0x1c, cpu.edx);
    ret(app, cpu, {&cpu.ebp, &cpu.esi, &cpu.edx, &cpu.ecx, &cpu.ebx});
}

/* 0x40c210: 1 where the car eax's countdown [+0x650] runs out this frame
 * (past 1920 frames) with fewer places left (0x40ef10) than the cars still in
 * [0x5efd8c] less [0x56ebc0]; a countdown not begun set to 2; else 0. */
void countdownDone(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ebx, cpu.ecx, cpu.edx, cpu.ebp});
    cpu.ebp = cpu.esp;
    if (s(at(app, kRaceFrame)) >= 0x780)
    {
        cpu.ecx = at(app, cpu.eax + 0x650);
        if (s(cpu.ecx) <= 0)
            put(app, cpu.eax + 0x650, 2);
        else
        {
            cpu.ebx = cpu.ecx - 1;
            put(app, cpu.eax + 0x650, cpu.ebx);
            if (cpu.ebx == 0)
            {
                cpu.edx = at(app, 0x5efd8c);
                cpu.edx -= at(app, 0x56ebc0);
                if (!byAddress(app, cpu, kPlacesLeft))
                    return;
                if (s(cpu.edx) < s(cpu.eax))
                {
                    cpu.eax = 1;
                    return ret(app, cpu, {&cpu.ebp, &cpu.edx, &cpu.ecx, &cpu.ebx});
                }
            }
        }
    }
    cpu.eax = 0;
    ret(app, cpu, {&cpu.ebp, &cpu.edx, &cpu.ecx, &cpu.ebx});
}

/* 0x40c270: the target speed [+0x7b0] of the traffic's car eax: its node's
 * (0x40ed40) + 1, no more than the traffic's top speed, signed as it goes. */
void trafficTarget(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ebx, cpu.edx, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    cpu.edx = cpu.eax;
    cpu.eax = at(app, cpu.eax + 0x1c);
    if (!byAddress(app, cpu, kNodeSpeed))
        return;
    fld(cpu, Float(1.0));
    faddp(cpu, 1);
    cpu.eax = cpu.edx;
    fstp(app, cpu, cpu.edx + 0x7b0);
    if (!direct(app, cpu, topSpeedFactor))
        return;
    fstp(app, cpu, cpu.ebp - 4);
    fld(cpu, f32(app, cpu.edx + 0x7b0));
    if (!fcomp(cpu, f32(app, cpu.ebp - 4)).belowEqual())
    {
        cpu.eax = at(app, cpu.ebp - 4);
        put(app, cpu.edx + 0x7b0, cpu.eax);
    }
    if (at(app, kBackwards) != 0)
        flipSign(app, cpu.edx, cpu.ah);
    if ((byte(app, cpu.edx + 0x621) & 0x10) != 0)
        flipSign(app, cpu.edx, cpu.bh);
    cpu.esp = cpu.ebp;
    ret(app, cpu, {&cpu.ebp, &cpu.edx, &cpu.ebx});
}

/* 0x40c2f0: the traffic's car eax's step: its place across the road [+0x7b4]
 * picked by the game's random numbers (0x554f10 times 0x554f18, as the code
 * steps them) within its lane's width at its node -- on the left or the
 * right by which way it goes, the track's way and [0x56f230] -- plus one of
 * the four offsets of 0x552548 [+0x654]; its target speed (trafficTarget)
 * its speed [+0x7c0] and the speed's size [+0x7c4]; then driven (0x40cd80). */
void trafficStep(App* app, CPU& cpu)
{
    save(app, cpu, {cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp});
    cpu.ebp = cpu.esp;
    cpu.esp -= 0xc;
    cpu.edx = cpu.eax;
    cpu.ecx = at(app, cpu.eax + 0x1c);
    cpu.eax = (byte(app, cpu.eax + 0x621) & 0x10) != 0 ? 0 : 1;
    cpu.eax ^= at(app, kBackwards);
    bool left = cpu.eax != 0 && at(app, 0x56f230) == 1;
    if (!left)
    {
        cpu.eax = (byte(app, cpu.edx + 0x621) & 0x10) != 0 ? 1 : 0;
        cpu.eax ^= at(app, kBackwards);
        left = cpu.eax != 0 && at(app, 0x56f230) == 0xffffffff;
    }
    if (left)
    {
        cpu.eax = cpu.ecx * 8;
        cpu.esi = at(app, 0x554f18);
        cpu.eax += cpu.ecx;
        cpu.ecx = at(app, 0x554f10);
        cpu.ecx = cpu.ecx * cpu.esi;
        cpu.ebx = at(app, kNodes);
        cpu.eax <<= 2;
        cpu.ebx += cpu.eax;
        cpu.eax = 0;
        cpu.al = byte(app, cpu.ebx + 0x1b);
        cpu.eax <<= 0xf;
        put(app, cpu.ebp - 4, cpu.eax);
        fild(app, cpu, cpu.ebp - 4);
        cpu.eax = cpu.ecx;
        fmul(cpu, f64(app, 0x5366d4));
        cpu.eax &= 0xffff;
        cpu.ecx &= 0xffff00;
        put(app, 0x554f10, cpu.eax);
        cpu.al = byte(app, cpu.ebx + 0x13);
        cpu.ecx >>= 8;
        cpu.al &= 0xf;
        cpu.eax &= 0xff;
        cpu.eax = cpu.eax * cpu.ecx;
        cpu.eax >>= 0x10;
        cpu.eax += 1;
        put(app, cpu.ebp - 4, cpu.eax);
        fild(app, cpu, cpu.ebp - 4);
        fmulSt(cpu, 1);
        fxch(cpu, 1);
        fmul(cpu, f64(app, 0x5366dc));
        fsubp(cpu, 1);
    }
    else
    {
        cpu.eax = cpu.ecx * 8;
        cpu.edi = at(app, 0x554f18);
        cpu.eax += cpu.ecx;
        cpu.ecx = at(app, 0x554f10);
        cpu.ecx = cpu.ecx * cpu.edi;
        cpu.ebx = at(app, kNodes);
        cpu.eax <<= 2;
        cpu.ebx += cpu.eax;
        cpu.eax = 0;
        cpu.al = byte(app, cpu.ebx + 0x17);
        cpu.eax <<= 0xf;
        put(app, cpu.ebp - 4, cpu.eax);
        cpu.eax = cpu.ecx;
        cpu.eax &= 0xffff;
        put(app, 0x554f10, cpu.eax);
        cpu.eax = 0;
        cpu.al = byte(app, cpu.ebx + 0x13);
        cpu.ecx &= 0xffff00;
        cpu.eax = reg32(s(cpu.eax) >> 4);
        cpu.ecx >>= 8;
        cpu.eax &= 0xf;
        cpu.eax = cpu.eax * cpu.ecx;
        fild(app, cpu, cpu.ebp - 4);
        fmul(cpu, f64(app, 0x5366d4));
        cpu.eax >>= 0x10;
        cpu.eax += 1;
        fld(cpu, cpu.fpu.st(0));
        put(app, cpu.ebp - 4, cpu.eax);
        fchs(cpu);
        fild(app, cpu, cpu.ebp - 4);
        fmulp(cpu, 1);
        fxch(cpu, 1);
        fmul(cpu, f64(app, 0x5366dc));
        faddp(cpu, 1);
    }
    fstp(app, cpu, cpu.edx + 0x7b4);
    cpu.eax = at(app, 0x554f10);
    cpu.ecx = at(app, 0x554f18);
    cpu.ecx = cpu.ecx * cpu.eax;
    cpu.eax = cpu.ecx;
    cpu.eax &= 0xffff;
    put(app, 0x554f10, cpu.eax);
    cpu.eax = cpu.ecx;
    cpu.eax &= 0xffff00;
    cpu.eax >>= 8;
    cpu.eax <<= 2;
    cpu.eax >>= 0x10;
    put(app, 0x554f14, cpu.ecx);
    cpu.ecx = at(app, cpu.eax * 4 + 0x552548);
    put(app, cpu.edx + 0x654, cpu.ecx);
    fld(cpu, f32(app, cpu.edx + 0x654));
    fadd(cpu, f32(app, cpu.edx + 0x7b4));
    cpu.eax = cpu.edx;
    fstp(app, cpu, cpu.edx + 0x7b4);
    if (!direct(app, cpu, trafficTarget))
        return;
    fld(cpu, Float(0.0));
    fld(cpu, f32(app, cpu.edx + 0x7b0));
    cpu.ecx = at(app, 0x554f14);
    fstp(app, cpu, cpu.edx + 0x7c0);
    if (!fcomp(cpu, f32(app, cpu.edx + 0x7c0)).aboveEqual())
    {
        cpu.eax = at(app, cpu.edx + 0x7c0);
        put(app, cpu.ebp - 8, cpu.eax);
    }
    else
    {
        fld(cpu, f32(app, cpu.edx + 0x7c0));
        fchs(cpu);
        fstp(app, cpu, cpu.ebp - 8);
    }
    cpu.eax = at(app, cpu.ebp - 8);
    put(app, cpu.edx + 0x7c4, cpu.eax);
    cpu.eax = cpu.edx;
    put(app, 0x554f14, cpu.ecx);
    if (!byAddress(app, cpu, kSpeedLimit))
        return;
    cpu.ecx = at(app, 0x554f14);
    cpu.esp = cpu.ebp;
    ret(app, cpu, {&cpu.ebp, &cpu.edi, &cpu.esi, &cpu.edx, &cpu.ecx, &cpu.ebx});
}

}  // namespace pursuit
}  // namespace nfs3hp
