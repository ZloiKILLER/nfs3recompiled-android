// game_checks.cpp's parts for a car's engine, gears and driver's inputs
// (native_drivetrain.cpp), included in each table with the macro naming the
// part.
#if defined(DRIVETRAIN_WINDOWS)
// {from, to}, -- what the functions write, compared
// The cases' car and its records are in the harness's (0x62a000 to
// 0x62c000); the globals: the race's flags and the players' gearboxes
// (0x6fd000, compared already), the race's frames (0x7d3000), the
// computer's bend (0x55e000), the slices' count (0x5dd000) -- windows of
// other parts already.  The code's page with the revs' drop by gear
// (0x4a78b6), which the harness has not loaded: written by the world as
// nfs3.exe has it, read only.
{0x4a7000, 0x4a8000},
#elif defined(DRIVETRAIN_EFFECTS)
// kSomething,  -- a stand-in's effect of this file's
kDtLength,      // a vector's length onto the x87: the drivetrain car's velocity's own, else as kFloatBack
kDtSliceSpeed,  // the computer's speed at a slice (0x40e080) onto the x87: 0 to 255, or 64
kDtHeading,     // the track's heading at a slice (0x49fee0): 0 to 0x3ff mostly
#elif defined(DRIVETRAIN_EFFECT_CASES)
// case kSomething: ... break;
case kDtLength:
    if (words[0] >= 0x62a000 && words[0] < 0x62b000 && r % 8 != 0)
    {
        // The velocity's length as it is, in doubles: under 0.5 as often
        // as the case's car stands.
        double sum = 0;
        for (x86::reg32 i = 0; i < 3; ++i)
        {
            float f;
            std::memcpy(&f, &words[1 + i], 4);
            sum += double(f) * double(f);
        }
        cpu.fpu.count += 1;
        cpu.fpu.st(0) = x86::Float(std::sqrt(sum));
    }
    else
    {
        // Exactly kFloatBack, for everyone else's calls.
        cpu.fpu.count += 1;
        cpu.fpu.st(0) = x86::Float(double(x86::sreg32(r)) / 7.0);
    }
    break;
case kDtSliceSpeed:
    cpu.fpu.count += 1;
    cpu.fpu.st(0) = x86::Float(r % 8 == 0 ? 64.0 : double((r >> 8) % 0x100));
    break;
case kDtHeading:
    result = r % 16 == 0 ? r : (r >> 4) % 0x400;
    break;
#elif defined(DRIVETRAIN_GAME_FUNCTIONS)
// {address, "name", convention, registers, words, clobbers, through, deref, effect},
// Before racestart's own row for it, which it keeps to for every other
// caller: the length of the vector at eax, every register kept, its three
// floats logged.
{0x4e06a0, "a vector's length", kWatcom, 1, 0, 0, 0, 3, kDtLength},
// The computer's speed at slice eax (a byte of the track's by 64, or 64),
// onto the x87; the track's heading at slice eax.
{0x40e080, "the computer's speed at a slice", kWatcom, 1, 0, kEax, -1, 0, kDtSliceSpeed},
{0x49fee0, "the track's heading at a slice", kWatcom, 1, 0, kEax, -1, 0, kDtHeading},
#elif defined(DRIVETRAIN_POINTERS)
// {slot, "name", convention, registers, words, clobbers, through, deref, effect},
#elif defined(DRIVETRAIN_PROMISES)
// {"drivetrain::name", watcom, result, arguments, fpu, heap, liveOnly},
// Every register compared, eax too, and the x87's stack, its control and
// status words (a compare's flags) -- its registers in use.
{"drivetrain::computerDriver", true, true, 0, true, kNoHeap, true},
{"drivetrain::engineTorque", true, true, 0, true, kNoHeap, true},
{"drivetrain::automaticGears", true, true, 0, true, kNoHeap, true},
{"drivetrain::gearLimit", true, true, 0, true, kNoHeap, true},
{"drivetrain::autoReverse", true, true, 0, true, kNoHeap, true},
{"drivetrain::driverInputs", true, true, 0, true, kNoHeap, true},
{"drivetrain::wheelGrip", true, true, 0, true, kNoHeap, true},
{"drivetrain::engineForce", true, true, 0, true, kNoHeap, true},
#elif defined(DRIVETRAIN_ARGUMENTS)
/* Where the drivetrain's cases keep their own: the car, its handling, its
 * driver's record. */
constexpr x86::reg32 kDtCar = 0x62a000, kDtHandling = 0x62b000, kDtRecord = 0x62b800;

x86::reg32 dtFloat(float f)
{
    x86::reg32 bits;
    std::memcpy(&bits, &f, 4);
    return bits;
}

/* A float from `from` to `to`, now and then exactly 0, or (one in 20) any
 * float the drivetrain's fields compare with. */
x86::reg32 dtRange(float from, float to)
{
    if (chance(5))
        return aRaceFloat();
    if (chance(5))
        return 0;
    return dtFloat(from + (to - from) * float(below(10001)) / 10000.0f);
}

/* The drivetrain's world: the race's controls' and force feedback's
 * (gearChangeKick plays the gear's kick), then the car -- each field made
 * up, those its code walks set as a race has them: its gears' ratios and
 * torques, its revs, pedals, steering and gear, its speed and velocity --
 * its handling (the gears' revs by speed, the torque's curve, the idle and
 * top revs, the pedals' and steering's steps), its driver's record, and the
 * race's globals the functions read. */
void makeDrivetrainWorld(CheckApp& g, x86::CPU& cpu)
{
    makeRaceWorld(g, cpu);
    for (x86::reg32 at = kDtCar; at < kDtRecord + 0x100; at += 4)
        g.word(at) = chance(50) ? aRaceFloat() : chance(50) ? below(8) : random32();
    const x86::reg32 car = kDtCar, h = kDtHandling;
    g.word(car + 0x528) = h;
    g.word(car + 0x220) = kDtRecord;
    // The handling: its gears (reverse, neutral, then 4 to 6 forward), each
    // one's revs by the speed and its top speed's inverse on the car.
    const x86::reg32 gears = chance(90) ? 6 + below(3) : below(10);
    g.word(h + 8) = gears;
    g.getMemory<x86::reg16>(h + 0x10) = x86::reg16(chance(80) ? 4 + below(20) : random32());
    for (x86::reg32 i = 0; i < 16; ++i)
        g.word(h + 0x14 + 4 * i) = chance(90) ? below(600) : random32();
    for (x86::reg32 i = 0; i < 8; ++i)
    {
        const float top = i == 0 ? 15.0f : i == 1 ? 0.0f : 12.0f + 11.0f * float(i - 2);
        const float revs = top == 0.0f ? 0.0f : 7000.0f / (top * (0.8f + 0.4f * float(below(1001)) / 1000.0f));
        const float sign = i == 0 && chance(50) ? -1.0f : 1.0f;
        g.word(h + 0x94 + 4 * i) = chance(95) ? dtFloat(sign * revs) : aRaceFloat();
        g.word(car + 0x54c + 4 * i) = chance(95) ? dtFloat(revs == 0.0f ? 0.0f : 1.0f / revs) : aRaceFloat();
        g.word(car + 0x56c + 4 * i) = chance(95) ? dtRange(0.5f, 4.0f) : aRaceFloat();
    }
    g.word(car + 0x544) = dtRange(0.001f, 0.02f);
    g.word(car + 0x548) = dtRange(0.001f, 0.02f);
    for (x86::reg32 i = 0; i < 42; ++i)
        g.word(h + 0x114 + 4 * i) = chance(97) ? dtRange(50.0f, 450.0f) : aRaceFloat();
    const x86::reg32 idle = 400 + below(1200);
    g.word(h + 0x1c0) = idle;
    g.word(h + 0x1c4) = idle + 3000 + below(6000);
    g.word(h + 0x1c8) = dtRange(20.0f, 90.0f);
    for (x86::reg32 i = 0; i < 16; ++i)
        g.word(h + 0x1f0 + 4 * i) = dtRange(1.0f, 40.0f);
    g.word(h + 0x240) = dtRange(1.0f, 30.0f);
    g.word(h + 0x244) = dtRange(1.0f, 30.0f);
    g.word(h + 0x24c) = dtRange(0.0f, 0.02f);
    g.word(h + 0x250) = dtRange(0.0f, 2.0f);
    g.word(h + 0x29c) = chance(30) ? 8 : below(12);
    g.word(h + 0x2a4) = below(0x40);
    g.word(h + 0x2a8) = g.word(h + 0x2a4) + below(0x80);
    g.word(h + 0x2ac) = g.word(h + 0x2a8) + below(0x100);
    for (const x86::reg32 at : {0x2b0u, 0x2b4u, 0x2b8u})
        g.word(h + at) = dtRange(0.3f, 1.2f);
    // The driver's record: steering and brake eased or not, skill, drag.
    g.word(kDtRecord + 0x24) = chance(70) ? 1 : 0;
    g.word(kDtRecord + 0x2c) = chance(70) ? 1 : 0;
    g.word(kDtRecord + 0x48) = dtRange(0.5f, 2.0f);
    g.word(kDtRecord + 0x54) = dtRange(0.0f, 3.0f);
    // The car: where it is and which way, its player and state.
    g.word(car + 0x1c) = below(0x100);
    g.word(car + 0x154) = chance(90) ? below(0x400) : random32();
    g.word(car + 0x210) = chance(90) ? below(0x400) : random32();
    g.word(car + 0x214) = chance(90) ? x86::reg32(x86::sreg32(below(0x400)) - 0x200) : random32();
    g.word(car + 0x218) = chance(90) ? below(0x200) : random32();
    g.word(car + 0x520) = chance(90) ? below(0x220) : random32();
    g.word(car + 0x134) = dtRange(0.6f, 1.1f);
    g.word(car + 0x160) = below(3);
    g.word(car + 0x18c) = below(16);
    g.getMemory<x86::reg16>(car + 0x164) = x86::reg16(chance(70) ? 0 : below(8));
    g.word(car + 0x1f0) = below(8);
    g.getMemory<x86::reg8>(car + 0x200) = x86::reg8(random32());
    g.word(car + 0x2c4) = chance(30) ? 3 : below(5);
    g.word(car + 0x2e4) = below(4);
    // Its wheels on the car's surface (no piece of track of their own).
    for (x86::reg32 w = 0; w < 4; ++w)
        g.word(car + 0x314 + 0x78 * w) = 0;
    // Its velocity and speed: standing now and then, else on the move.
    const bool standing = chance(35);
    for (x86::reg32 i = 0; i < 3; ++i)
        g.word(car + 0x4ec + 4 * i) = standing ? dtRange(-0.3f, 0.3f) : dtRange(-70.0f, 70.0f);
    if (!standing && chance(70))
        g.word(car + 0x4f4) = dtRange(-10.0f, 80.0f);
    g.word(car + 0x5a4) = chance(20) ? dtRange(-3.0f, 0.5f) : dtRange(-5.0f, 80.0f);
    g.word(car + 0x5b0) = dtRange(-0.5f, 0.5f);
    g.word(car + 0x5f0) = dtRange(0.0f, 1.0f);
    g.word(car + 0x58c) = dtRange(0.00005f, 0.002f);
    // The pedals, gear and steering asked and as they are.
    for (const x86::reg32 at : {0x504u, 0x505u, 0x508u, 0x509u})
        g.getMemory<x86::reg8>(car + at) = x86::reg8(chance(25) ? 0 : chance(25) ? 0xff : random32());
    // Standing, off the pedals now and then, the throttle just touched.
    if (standing && chance(50))
    {
        g.getMemory<x86::reg8>(car + 0x508) = 0;
        g.getMemory<x86::reg8>(car + 0x504) = x86::reg8(below(12));
    }
    // The gear: reverse, neutral or first half the time (a car standing
    // gets into reverse and out of it there), the gear asked it mostly.
    const auto aGear = [&] {
        return x86::reg8(chance(95) ? (chance(50) ? below(3) : below(gears < 8 ? 8 : gears)) : random32());
    };
    g.getMemory<x86::reg8>(car + 0x50b) = aGear();
    g.getMemory<x86::reg8>(car + 0x50a) = aGear();
    g.getMemory<x86::reg8>(car + 0x506) = chance(60) ? g.getMemory<x86::reg8>(car + 0x50b) : aGear();
    for (const x86::reg32 at : {0x507u, 0x50cu, 0x50du})
        g.getMemory<x86::reg8>(car + at) = x86::reg8(chance(90) ? below(2) : random32());
    if (standing && chance(50))
        g.getMemory<x86::reg8>(car + 0x507) = 0;
    g.getMemory<x86::reg16>(car + 0x510) =
        x86::reg16(chance(50) ? 0 : chance(80) ? below(24) : random32());
    g.word(car + 0x514) = chance(80) ? below(0x200) : random32();
    for (const x86::reg32 at : {0x518u, 0x51cu})
        g.word(car + at) = chance(20) ? 0 : chance(90) ? x86::reg32(x86::sreg32(below(0x200)) - 0x100) : random32();
    // The revs: about the idle now and then, else anywhere up past the top.
    g.word(car + 0x590) = chance(standing ? 50 : 20) ? idle - 100 + below(250) : chance(85) ? below(g.word(h + 0x1c4) + 2000)
                                                                           : random32() % 0x10000;
    // The race's: its mode and type, the simulation's flags, the direction,
    // the player here, the computer driving, the start's countdown, the
    // computer's last bend, the players' gearboxes, the slices, the sound.
    g.word(0x6fd3b0) = below(5);
    g.word(0x6fd3b8) = below(4);
    g.word(0x6fd4a4) = below(2);
    g.word(0x6fd4fc) = below(4);
    g.word(0x6fd4c0) = chance(70) ? 0 : 1;
    g.word(0x6fd4f4) = chance(60) ? g.word(car + 0x1f0) : below(8);
    g.word(0x6fd490) = chance(80) ? 0 : 1;
    g.word(0x6fd494) = chance(80) ? 0 : 1;
    g.word(0x7d3680) = chance(85) ? 1 : 0;
    g.word(0x7d3684) = chance(25) ? below(0x23c) : 0x23c + below(5000);
    g.word(0x55ed84) = x86::reg32(x86::sreg32(below(0x800)) - 0x400);
    for (x86::reg32 i = 0; i < 8; ++i)
        g.word(0x6fd548 + 0x6c * i) = chance(50) ? 1 : chance(80) ? 0 : below(4);
    g.word(0x5dd940) = 0x100;
    // The sound on (the gear's clunk played through, snd::carKnock), or the
    // force feedback's car this one (the gear's kick played through,
    // controls::gearChangeKick), not both: the kick's effect record has
    // fields its code never sets, which hold what was on the stack before --
    // and carKnock's native leaves other words there than its game code
    // does, so the two runs would hand SetEffect different garbage.
    const bool sound = chance(50);
    g.word(0x55290c) = sound ? 1 : 0;
    if (!sound && chance(70))
        g.word(0x678b54) = car;
    else if (g.word(0x678b54) == car)
        g.word(0x678b54) = 0;
    // The revs' drop by gear, as nfs3.exe has it among its code.
    static const x86::reg32 drops[] = {0x32, 0x32, 0xa, 0x14, 0x32, 0x32, 0x32, 0x32};
    for (x86::reg32 i = 0; i < 8; ++i)
        g.word(0x4a78b6 + 4 * i) = drops[i];
}

bool makeDrivetrainArguments(CheckApp& g, const std::string& name, x86::CPU& cpu, x86::reg32*)
{
    makeDrivetrainWorld(g, cpu);
    cpu.eax = kDtCar;
    // The revs the torque is asked at: the engine's range mostly.
    if (name == "engineTorque")
        cpu.edx = chance(90) ? below(12000) : random32();
    // No gear past the tables' 8 for engineForce: its copy of the revs' drop by gear is on the game's
    // frame, and past it the game's code reads its own locals, which the typed engineForce has not.
    // No car has more: every table by gear has 8 rows.
    if (name == "engineForce")
    {
        if (g.word(kDtHandling + 8) > 8)
            g.word(kDtHandling + 8) = 8;
        g.getMemory<x86::reg8>(kDtCar + 0x50b) &= 7;
    }
    // No force feedback's car for driverInputs: the kick's effect record has fields its code never
    // sets, which hold what the stack last had -- in the game's code autoReverse's locals, which the
    // typed autoReverse does not keep there -- and its addresses are the frame's.  gearChangeKick's
    // own check holds it against its code.
    if (name == "driverInputs" && g.word(0x678b54) == kDtCar)
        g.word(0x678b54) = 0;
    return true;
}
#endif
