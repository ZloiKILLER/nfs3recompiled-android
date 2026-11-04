// game_checks.cpp's parts for the controls read in a race and kept in config.dat (native_controls_race.cpp), included in
// each table with the macro naming the part.
#if defined(CONTROLS_WINDOWS)
// {from, to}, -- what the functions write, compared
// The last gear and the jolts' wait; the race's clock, frame and scales, its
// players' cars; the force feedback's state; the road's camber.
{0x55b000, 0x55c000}, {0x5ef000, 0x5f0000}, {0x678000, 0x679000}, {0x79c000, 0x79d000},
// The code's page with the switches' tables (0x4710dc, 0x471818, 0x471aa4), which the harness has not loaded:
// written by makeRaceWorld as nfs3.exe has them, read only.
{0x471000, 0x472000},
#elif defined(CONTROLS_EFFECTS)
// kSomething,  -- a stand-in's effect of this file's
kRaceControllers,  // DirectInput's controllers found: as many as there were mostly, or another 0 to 16
kRaceController,   // a controller DirectInput found: config.dat's record mostly, changed now and then; or none
kRaceDeviceSet,    // a device set: 0x6fd240 for mode 1, else 0x6fd20c, as the game's
kRaceVectors,      // a vector less another (three floats at ecx), a matrix transposed (nine at edx)
kRaceAngle,        // atan2 in half turns onto the x87's stack: -1 to 1 mostly, any now and then
kRaceSurface,      // what a car is on: 0 to 19 mostly
#elif defined(CONTROLS_EFFECT_CASES)
// case kSomething: ... break;
case kRaceControllers:
    result = r % 3 != 0 ? word(0x6fd204) : (r >> 4) % 17;
    break;
case kRaceController:
{
    // Config.dat's record of controller eax copied to the harness's, its
    // kind, product, name, axes and gain changed now and then.
    const x86::reg32 index = words[0] & 0xf;
    const x86::reg32 record = 0x61a000 + 0x88 * index;
    result = r % 5 == 0 ? 0 : record;
    for (x86::reg32 i = 0; i < 0x88; i += 4)
        word(record + i) = word(0x6fc970 + 0x88 * index + i);
    const x86::reg32 change = random(id + 7);
    if (change % 6 == 0)
        word(record) ^= 1u << (change >> 8) % 8;
    if ((change >> 3) % 8 == 0)
        word(record + 0x38 + 4 * ((change >> 6) % 3)) ^= 1;
    if ((change >> 11) % 8 == 0)
        getMemory<x86::reg8>(record + 4 + (change >> 14) % 6) ^= 0x20;
    for (x86::reg32 axis = 0; axis < 8; ++axis)
        if (random(id + 8 + axis) % 3 == 0)
            word(record + 0x48 + 8 * axis + 4 * (random(id + 16 + axis) % 2)) = random(id + 24 + axis) % 101;
    if ((change >> 17) % 4 == 0)
        word(record + 0x44) = (change >> 19) % 120;
    break;
}
case kRaceDeviceSet:
    result = words[0] == 1 ? 0x6fd240 : 0x6fd20c;
    break;
case kRaceVectors:
{
    const bool transpose = s.address == 0x4e0110;
    const x86::reg32 to = transpose ? words[1] : words[3];
    for (x86::reg32 i = 0; i < (transpose ? 9u : 3u); ++i)
    {
        const x86::reg32 bits = random(id + 0x40 + i);
        float f = float(x86::sreg32(bits % 4001) - 2000) / 100.0f;
        x86::reg32 b;
        std::memcpy(&b, &f, 4);
        x86::reg32 odd = anyFloatBits(random64(id + i));
        /* With doubles for the x87, which of two NaNs a sum or a product
         * keeps is the host compiler's, which may swap the operands: the
         * vectors finite here, their NaNs and infinities held to the bit by
         * the 80-bit build. */
        if ((odd & 0x7f800000) == 0x7f800000)
            odd &= 0xbfffffff;
        word(to + 4 * i) = bits % 23 == 0 ? odd : b;
    }
    break;
}
case kRaceAngle:
{
    const std::uint64_t bits = random64(id);
    double angle = double(x86::sreg32(r % 2001) - 1000) / 1000.0;
    if (r % 17 == 0)
        angle = r % 2 ? 0.0 : -0.0;
    else if (r % 19 == 0)
    {
        x86::reg32 b = anyFloatBits(bits);
        float f;
        std::memcpy(&f, &b, 4);
        angle = f;
    }
    cpu.fpu.push(x86::Float(angle));
    break;
}
case kRaceSurface:
    result = r % 5 == 0 ? r >> 8 : (r >> 8) % 20;
    break;
#elif defined(CONTROLS_GAME_FUNCTIONS)
// {address, "name", convention, registers, words, clobbers, through, deref, effect},
// DirectInput's layer (native_controls_input.cpp's): the controllers found,
// one of them, an axis's force set, every device's actuators on or off, a
// device's gain, an effect created, started, stopped, unloaded, its
// parameters set (its strength's address and parameters on the stack, ret
// 0x10; the parameters logged).
{0x435600, "the controllers found", kWatcom, 0, 0, kEax, -1, 0, kRaceControllers},
{0x435690, "a controller found", kWatcom, 1, 0, kEax, -1, 0, kRaceController},
{0x435b40, "an axis's force set", kWatcom, 4, 0, kEax | kEbx | kEcx | kEdx, -1, 0, kNothing},
{0x435d20, "the actuators on", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x435d90, "the actuators off", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x435df0, "a device's gain set", kWatcom, 2, 0, kEax | kEdx, -1, 0, kNothing},
{0x435f10, "an effect created", kWatcom, 2, 0, kEax | kEdx, 1, 17, kNothing},
{0x4361e0, "an effect started", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x436230, "an effect stopped", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x436310, "an effect unloaded", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x436330, "an effect's parameters set", kStdcall, 4, 4, kEax | kEbx | kEcx | kEdx, 5, 16, kNothing},
// The device sets (native_controls_devices.cpp's): the mode's, made afresh.
{0x4904d0, "a device set", kWatcom, 1, 0, kEax, -1, 0, kRaceDeviceSet},
{0x4904f0, "a device set made afresh", kWatcom, 1, 0, kEax, -1, 0, kNothing},
// The rest of the game's: vectors less another, a matrix transposed, atan2,
// what a car is on, the game's timers run and one added, the menus told the
// controllers changed.
{0x4e0000, "vectors less another", kWatcom, 4, 0, kEax | kEbx | kEcx | kEdx, 1, 3, kRaceVectors},
{0x4e0110, "a matrix transposed", kWatcom, 2, 0, 0, 0, 9, kRaceVectors},
{0x4ea9c0, "atan2", kStdcall, 0, 2, 0, -1, 0, kRaceAngle},
{0x420840, "what a car is on", kWatcom, 2, 0, kEax | kEdx, -1, 0, kRaceSurface},
{0x4e7630, "the game's timers run", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x4be250, "a timer added", kWatcom, 4, 0, kEax | kEbx | kEcx | kEdx, -1, 0, kNothing},
{0x43c040, "the controllers changed", kWatcom, 0, 0, kEax, -1, 0, kNothing},
#elif defined(CONTROLS_POINTERS)
// {slot, "name", convention, registers, words, clobbers, through, deref, effect},
#elif defined(CONTROLS_PROMISES)
// {"controls::name", watcom, result, arguments, fpu, heap, liveOnly},
{"controls::steadyEngineRumble", true, true, 0},
{"controls::startEngineRumble", true, true, 0},
{"controls::silenceEngineRumble", true, true, 0},
{"controls::setForceGain", true, true, 0},
{"controls::findForceController", true, true, 0},
{"controls::createForceEffects", true, true, 0},
{"controls::stopForces", true, true, 0},
{"controls::stopForcesAgain", true, true, 0},
{"controls::stopForcesOnceMore", true, true, 0},
{"controls::stopForcesOrActuatorsOn", true, true, 0},
{"controls::stopForcesOrActuatorsOff", true, true, 0},
{"controls::forcePlayerCar", true, true, 0},
{"controls::startRaceForces", true, true, 0},
{"controls::stopRaceEffects", true, true, 0},
{"controls::crashJolt", true, true, 0, true, kNoHeap, true},
{"controls::bumpJolt", true, true, 0, true, kNoHeap, true},
{"controls::gearChangeKick", true, true, 0},
{"controls::steeringAmount", true, true, 0, true, kNoHeap, true},
{"controls::playSteeringSpring", true, true, 0, true, kNoHeap, true},
{"controls::playSlideSpring", true, true, 0, true, kNoHeap, true},
{"controls::playRoadRumble", true, true, 0, true, kNoHeap, true},
{"controls::playEngineRumble", true, true, 0, true, kNoHeap, true},
{"controls::isForceCar", true, true, 0},
{"controls::raceForcesFrame", true, true, 0, true, kNoHeap, true},
{"controls::allActuatorsOff", true, true, 0},
{"controls::playJolt", true, true, 0},
{"controls::stopEngineRumble", true, true, 0},
{"controls::stopSteeringSpring", true, true, 0},
{"controls::carSpeedMph", true, true, 0, true, kNoHeap, true},
{"controls::loadControllers", true, true, 0},
#elif defined(CONTROLS_ARGUMENTS)
/* A float's bits: about `around` mostly, any now and then. */
x86::reg32 aRaceFloat(float around)
{
    if (chance(8))
        return anyFloatBits(std::uint64_t(random32()) << 32 | random32());
    const float f = chance(10) ? 0.0f : around * (float(x86::sreg32(below(4001)) - 2000) / 1000.0f);
    x86::reg32 bits;
    std::memcpy(&bits, &f, 4);
    return bits;
}

/* The race's controls' world: the controllers, the force controller among
 * them or none, its effects' handles and state, the forces' strengths, the
 * device sets, the players' cars -- the car at 0x618000, its surface's
 * record at 0x619000 -- the race's clock and frame. */
void makeRaceWorld(CheckApp& g, x86::CPU& cpu)
{
    constexpr x86::reg32 car = 0x618000, part = 0x619000;
    static const char* const names[] = {"Microsoft SideWinder Force Feedback Wheel", "Microsoft SideWinder Pro",
                                        "Logitech WingMan", "Gravis GamePad", "ThrustMaster", "", "Microsoft"};
    const x86::reg32 count = chance(80) ? below(17) : random32() % 20;
    g.word(0x6fd204) = count;
    g.word(0x6fd208) = chance(30) ? 0 : 1 + below(3);
    for (x86::reg32 c = 0; c < 16; ++c)
    {
        const x86::reg32 record = 0x6fc970 + 0x88 * c;
        for (x86::reg32 i = 0; i < 0x88; i += 4)
            g.word(record + i) = chance(50) ? below(4) : random32();
        g.word(record) = chance(80) ? below(16) : random32();
        const char* name = names[below(7)];
        for (x86::reg32 i = 0; i < 0x30; ++i)
            g.getMemory<x86::reg8>(record + 4 + i) = 0;
        for (x86::reg32 i = 0; name[i] != 0 && i < 0x2f; ++i)
            g.getMemory<x86::reg8>(record + 4 + i) = x86::reg8(name[i]);
        g.word(record + 0x40) = chance(20) ? 0 : random32() & 0xff;
        g.word(record + 0x44) = chance(15) ? 0 : below(120);
    }
    const x86::reg32 force = chance(20) ? 0xffffffff : chance(85) ? below(count > 0 ? count : 1) : below(16);
    for (x86::reg32 at = 0x678b50; at < 0x678c00; at += 4)
        g.word(at) = chance(50) ? below(3) : random32();
    g.word(0x678b50) = force;
    g.word(0x678b54) = chance(80) ? car : chance(50) ? 0 : car + 0x10;
    g.word(0x678b58) = chance(70) ? 0 : 1;
    g.word(0x678b5c) = chance(70) ? 0 : 1;
    for (x86::reg32 at = 0x678b60; at < 0x678be0; at += 2)
        g.getMemory<x86::reg16>(at) = x86::reg16(chance(20) ? 0xffff : below(0x40));
    g.word(0x678be0) = chance(50) ? 0 : 1;
    g.word(0x678be4) = chance(50) ? 5 + below(13) : random32();
    g.word(0x678be8) = chance(50) ? 0x9c4 + 0xc8 * below(16) : random32();
    g.word(0x678bec) = chance(40) ? 0 : chance(60) ? 1 : below(0x30);
    g.word(0x678bf0) = chance(50) ? 0 : 1;
    g.word(0x678bf4) = chance(80) ? below(20) : random32();
    // The frame and when the next jolt may come.
    g.word(0x5efe00) = chance(80) ? below(0x10000) : random32();
    g.word(0x678bf8) = chance(50) ? g.word(0x5efe00) + below(64) - 32 : random32();
    // The strengths, the replay, the mode and the player, the device sets.
    for (x86::reg32 at = 0x6fd1f0; at <= 0x6fd200; at += 4)
        g.word(at) = chance(30) ? 0 : chance(80) ? below(101) : random32();
    g.word(0x6fd3a0) = chance(70) ? 0 : chance(50) ? below(4) : random32();
    g.word(0x6fd3b0) = chance(90) ? below(5) : random32();
    g.word(0x6fd4f8) = chance(90) ? below(2) : below(8);
    for (x86::reg32 at = 0x6fd20c; at < 0x6fd2a8; at += 4)
        g.word(at) = (chance(50) ? 1 : random32() & 0xff) | (chance(60) ? force << 28 : random32() & 0xf0000000)
                     | (random32() & 0x0fffff00);
    for (x86::reg32 at = 0x5efa48; at < 0x5efa68; at += 4)
        g.word(at) = chance(60) ? car : chance(50) ? car + 0x10 : random32();
    // The clocks, the scales, the last gear, the jolts' wait, the view, the camber.
    g.word(0x5efda4) = below(2000);
    g.word(0x6fd4f4) = below(2000);
    g.word(0x5efde0) = aRaceFloat(3.0f);
    g.word(0x5efde4) = aRaceFloat(3.0f);
    g.getMemory<x86::reg8>(0x55b02c) = x86::reg8(chance(20) ? 0x64 : chance(90) ? below(8) : random32());
    g.word(0x55b030) = chance(90) ? below(6) - 2 : random32();
    g.word(0x7cd94c) = below(4);
    for (x86::reg32 at = 0x79c4a4; at < 0x79c4ac; at += 4)
        g.word(at) = random32();
    // The car.
    for (x86::reg32 at = car; at < car + 0x1000; at += 4)
        g.word(at) = chance(30) ? below(4) : random32();
    for (const x86::reg32 at : {0x15cu, 0x4f4u, 0xb8u, 0x4f8u, 0x500u, 0x5c0u, 0x920u, 0x924u})
        g.word(car + at) = aRaceFloat(at == 0x4f4 || at == 0xb8 ? 60.0f : at >= 0x920 ? 8000.0f : 2.0f);
    for (x86::reg32 at = 0x98; at < 0xe4; at += 4)
        g.word(car + at) = aRaceFloat(50.0f);
    g.getMemory<x86::reg16>(car + 0x164) = x86::reg16(chance(70) ? 0 : random32());
    g.word(car + 0x170) = chance(30) ? 0x10000 | (random32() & 0x1000ffff) : random32();
    g.getMemory<x86::reg8>(car + 0x50b) = x86::reg8(chance(90) ? below(8) : random32());
    g.word(car + 0x590) = chance(90) ? below(0x2000) : random32();
    g.word(car + 0x99c) = chance(60) ? 0 : chance(80) ? below(4) : random32();
    g.word(car + 0x160) = below(4);
    g.word(car + 0x18c) = below(20);
    g.word(car + 0x528) = part;
    g.word(car + 0x538) = part;  // another car's, 0x10 on
    for (x86::reg32 at = part; at < part + 0x400; at += 4)
        g.word(at) = random32();
    g.word(part + 0x238) = chance(50) ? 0 : random32();
    // The switches' tables, as nfs3.exe has them.
    static const x86::reg32 tables[] = {
        0x4710dc, 5, 0x47116d, 0x47111e, 0x47116d, 0x47116d, 0x47116d,
        0x471818, 15, 0x4718cb, 0x4718a7, 0x471899, 0x4718cb, 0x4718a7, 0x4718b5, 0x4718cb, 0x4718c6, 0x4718bc,
        0x4718cb, 0x4718cb, 0x4718cb, 0x4718a7, 0x4718cb, 0x4718bc,
        0x471aa4, 18, 0x471cbe, 0x471ce1, 0x471cc8, 0x471d32, 0x471cd6, 0x471d32, 0x471d1d, 0x471cbe, 0x471d28,
        0x471cbe, 0x471d32, 0x471d1d, 0x471cd6, 0x471d2d, 0x471d28, 0x471cea, 0x471d08, 0x471cf3};
    for (size_t i = 0; i < std::size(tables); i += 2 + tables[i + 1])
        for (x86::reg32 j = 0; j < tables[i + 1]; ++j)
            g.word(tables[i] + 4 * j) = tables[i + 2 + j];
    // The x87: a control word of every precision and rounding, one or none on its stack.
    static const x86::reg16 precisions[] = {0x000, 0x200, 0x300};
    cpu.fpu.setControl(x86::reg16(0x7f | precisions[below(3)] | below(4) << 10 | (random32() & 0x1000)));
    cpu.fpu.count = below(2);
    for (x86::Float& f : cpu.fpu.regs)
        f = x86::Float(anyDouble());
}

bool makeRaceArguments(CheckApp& g, const std::string& name, x86::CPU& cpu, x86::reg32* stack)
{
    static const char* const names[] = {
        "steadyEngineRumble", "startEngineRumble", "silenceEngineRumble", "setForceGain", "findForceController",
        "createForceEffects", "stopForces", "stopForcesAgain", "stopForcesOnceMore", "stopForcesOrActuatorsOn",
        "stopForcesOrActuatorsOff", "forcePlayerCar", "startRaceForces", "stopRaceEffects", "crashJolt", "bumpJolt",
        "gearChangeKick", "steeringAmount", "playSteeringSpring", "playSlideSpring", "playRoadRumble",
        "playEngineRumble", "isForceCar", "raceForcesFrame", "allActuatorsOff", "playJolt", "stopEngineRumble",
        "stopSteeringSpring", "carSpeedMph", "loadControllers"};
    if (std::find_if(std::begin(names), std::end(names), [&](const char* n) { return name == n; })
        == std::end(names))
        return false;
    makeRaceWorld(g, cpu);
    constexpr x86::reg32 car = 0x618000, buffers = 0x61c000;
    cpu.eax = chance(85) ? car : car + 0x10;
    if (name == "startEngineRumble" || name == "setForceGain")
        cpu.eax = chance(90) ? below(120) : random32();
    else if (name == "crashJolt")
    {
        stack[0] = aRaceFloat(50.0f);
        stack[1] = aRaceFloat(50.0f);
        stack[2] = aRaceFloat(50.0f);
        stack[3] = aRaceFloat(15.0f);
    }
    else if (name == "bumpJolt")
    {
        cpu.ebx = chance(30) ? 0 : chance(50) ? below(100) : random32();
        cpu.edx = chance(90) ? below(4) : random32();
    }
    else if (name == "steeringAmount")
    {
        cpu.ebx = buffers + 4 * below(0x10);
        cpu.edx = buffers + 0x40 + 4 * below(0x10);
    }
    else if (name == "playJolt")
    {
        static const x86::reg32 angles[] = {0, 0x168, 0xb4, 0x5a, 0x10e, 0x2d};
        cpu.eax = buffers + 4 * below(0x10);
        cpu.edx = buffers + 0x40 + 4 * below(0x10);
        cpu.ebx = buffers + 0x80 + 4 * below(0x10);
        for (x86::reg32 at = buffers; at < buffers + 0xc0; at += 4)
            g.word(at) = random32();
        g.word(cpu.eax) = chance(50) ? angles[below(6)] : chance(80) ? below(400) - 20 : random32();
        g.word(cpu.edx) = chance(80) ? below(1200) : random32();
        g.word(cpu.ebx) = chance(80) ? below(12000) : random32();
    }
    return true;
}
#endif
