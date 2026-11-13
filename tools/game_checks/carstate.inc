// game_checks.cpp's parts for the race's cameras (native_carstate.cpp), included in each table with the macro
// naming the part.
#if defined(CARSTATE_WINDOWS)
// {from, to}, -- what the functions write, compared
// The code's page with the speed bands, the up vector and the zeros the
// cameras copy (0x4212b0, 0x4212d0, 0x4212dc), which the harness has not
// loaded: written by makeCameraWorld as nfs3.exe has them, read only.
{0x421000, 0x422000},
// The chase's state; the track's nodes and the views; the cameras in use,
// the cycle, the blends' state; the cars; the view's frustum; the race's
// switches (0x55fe30, 0x7922fc, 0x79c4a0, 0x7d3684).
{0x553000, 0x554000}, {0x5dd000, 0x5de000}, {0x5e1000, 0x5e2000}, {0x5ef000, 0x5f0000}, {0x560000, 0x561000},
{0x55f000, 0x560000}, {0x792000, 0x793000}, {0x79c000, 0x79d000}, {0x7d3000, 0x7d4000},
// The cases' own: the cameras' records, the cars, the nodes, the spots, the
// surfaces' heights, the arguments' buffers.
{0x648000, 0x658000},
#elif defined(CARSTATE_EFFECTS)
kCameraFlag,     // a yes or no: 0 a third of the time, else a small number
kCameraPlane,    // 0x496a50: a car's plane's frame from its points at eax, 9 floats into [edx]
kCameraDots,     // 0x4e0210: eax dot products of the vectors at edx with the one at ebx, into [ecx]
kCameraTurn,     // 0x4ea680, 0x4ea820: the matrix at eax turned by an angle, 9 floats into the second word
kCameraCross,    // 0x4e0160, 0x4e01a0: the cross product of the vectors at eax and edx, 3 floats into [ebx]
#elif defined(CARSTATE_EFFECT_CASES)
case kCameraFlag:
    result = r % 3 == 0 ? 0 : 1 + (r >> 8) % 5;
    break;
case kCameraDots:
case kCameraTurn:
case kCameraCross:
{
    // Into memory the case has, as the real ones write: a matrix's nine floats, or a dot product
    // each, near 1 now and then (the cameras compare their sizes with 0.98).
    const x86::reg32 to = s.effect == kCameraDots ? words[3] : words[2];
    const x86::reg32 count = s.effect == kCameraDots ? (words[0] < 4 ? words[0] : 4) : s.effect == kCameraCross ? 3 : 9;
    for (x86::reg32 i = 0; i < count && to + 4 * i < kMemory - 4; ++i)
    {
        const x86::reg32 bits = random(id + 0x40 + i);
        const float f = bits % 5 == 0 ? (bits % 2 ? 1.0f : -0.99f) : float(x86::sreg32(bits % 2001) - 1000) / 1000.0f;
        x86::reg32 b;
        std::memcpy(&b, &f, 4);
        word(to + 4 * i) = b;
    }
    break;
}
case kCameraPlane:
    for (x86::reg32 i = 0; i < 9; ++i)
    {
        const float f = float(x86::sreg32(random(id + 0x40 + i) % 4001) - 2000) / 100.0f;
        x86::reg32 b;
        std::memcpy(&b, &f, 4);
        word(words[1] + 4 * i) = b;
    }
    break;
#elif defined(CARSTATE_GAME_FUNCTIONS)
// {address, "name", convention, registers, words, clobbers, through, deref, effect},
// After the others' rows (the first row of an address is the one used).
// The track's natives a camera's place on the track goes through, by
// address -- trackdata::followNode and updatePosition -- stood in for, the
// fixed-point place at eax logged: on the cases' made-up track they would
// read past the track's blocks.  What hands something back: whether a place
// is on the track (trackdata::findQuad, stood in for as well); the distance
// to a car's box, a plane's height at a place (the plane's three points on
// the stack, ret 0x18) -- these two onto the x87's stack.  The others' rows
// serve the cameras' other calls: a track place's surface (0x4207e0,
// racestart.inc's), inTunnel (0x420bf0, trackdata.inc's), the dot product
// and the square root (0x4e01f0, 0x4ea920, handling.inc's; the cases set its
// mark for its small dot products).
// followNode reads the point in eax and the place in edx alone: the code's ebx (1, for
// updatePosition after it) is no argument of its, and native_carstate.cpp's call as C++
// (guest::direct) hands none.
{0x41e420, "trackdata::followNode", kWatcom, 2, 0, kEax, 0, 3, kNothing},
{0x41f6e0, "trackdata::updatePosition", kWatcom, 3, 0, kEax, 0, 3, kNothing},
{0x41e990, "a place on the track", kWatcom, 2, 0, kEax, 0, 3, kCameraFlag},
{0x4965b0, "the distance to a car", kWatcom, 2, 0, kEax, 0, 3, kFloatBack},
{0x4968a0, "a plane's height", kStdcall, 1, 6, kEax, 0, 9, kFloatBack},  // the plane's frame, 9 floats at eax
// The frame of the car's plane the height is taken in, from its points (12 floats at eax) into [edx]: written,
// as the real one does, where nothing would leave the frame's garbage for the height to read.
{0x496a50, "a car's plane", kWatcom, 2, 0, kEax, 0, 9, kCameraPlane},
// The dot products a camera's forward is measured with (into the frame the camera reads them
// from), and the head's and the roll's turns (matrices into the frame a product reads): written
// as the real ones do, where nothing would leave the frame's garbage to be read.
{0x4e0210, "dot products", kWatcom, 4, 0, kEax, 1, 3, kCameraDots},
{0x4ea680, "a matrix turned about y", kStdcall, 1, 2, kEax, 0, 9, kCameraTurn},
{0x4ea820, "a matrix turned about x", kStdcall, 1, 2, kEax, 0, 9, kCameraTurn},
// A cross product normalized and a cross product (their first vector logged).
{0x4e01a0, "a cross product normalized", kWatcom, 3, 0, kEax, 0, 3, kCameraCross},
{0x4e0160, "a cross product", kWatcom, 3, 0, kEax, 0, 3, kCameraCross},
// This file's own that the sound's natives call by address (the race's tick
// and the cop radio the player's view, the crash the camera's mode): stood in
// for them as before they were native -- generate.py's rows of then -- since
// their worlds have no cameras.  Each tested on its own.
{0x4229c0, "cameraView", kWatcom, 2, 0, kEax, -1, 0, kNothing},
{0x423b00, "cameraMode", kWatcom, 1, 0, kEax, -1, 0, kNothing},
#elif defined(CARSTATE_POINTERS)
// {slot, "name", convention, registers, words, clobbers, through, deref, effect},
#elif defined(CARSTATE_PROMISES)
// {"carstate::name", watcom, result, arguments, fpu, heap, liveOnly},
// Every register compared, and the x87's state where they use it.
{"carstate::idleHook", true, true, 0},
{"carstate::idleHookToo", true, true, 0},
{"carstate::viewFromCamera", true, true, 0, true, kNoHeap, true},
{"carstate::viewFromScreenCamera", true, true, 0, true, kNoHeap, true},
{"carstate::chooseView", true, true, 0, true, kNoHeap, true},
{"carstate::pickChaseNode", true, true, 0, true, kNoHeap, true},
{"carstate::turnAround", true, true, 0, true, kNoHeap, true},
{"carstate::attachCameras", true, true, 0, true, kNoHeap, true},
{"carstate::headTurn", true, true, 0, true, kNoHeap, true},
{"carstate::cameraNode", true, true, 0},
{"carstate::cameraTrackPlace", true, true, 0, true, kNoHeap, true},
{"carstate::keepOutOfCars", true, true, 0, true, kNoHeap, true},
{"carstate::keepAboveRoad", true, true, 0, true, kNoHeap, true},
{"carstate::keepOnTrack", true, true, 0, true, kNoHeap, true},
{"carstate::keepFromRightWall", true, true, 0, true, kNoHeap, true},
{"carstate::keepFromLeftWall", true, true, 0, true, kNoHeap, true},
{"carstate::keepAboveGround", true, true, 0, true, kNoHeap, true},
{"carstate::lookAtTarget", true, true, 0, true, kNoHeap, true},
{"carstate::blendCamera", true, true, 0, true, kNoHeap, true},
{"carstate::placeCamera", true, true, 0, true, kNoHeap, true},
{"carstate::switchCamera", true, true, 0, true, kNoHeap, true},
{"carstate::cameraZoom", true, true, 0, true, kNoHeap, true},
{"carstate::viewMatrix", true, true, 0, true, kNoHeap, true},
{"carstate::updateCamera", true, true, 0, true, kNoHeap, true},
{"carstate::cameraView", true, true, 0},
{"carstate::cockpitShown", true, true, 0},
{"carstate::cameraCar", true, true, 0},
{"carstate::cameraTarget", true, true, 0},
{"carstate::initCameras", true, true, 0, true, kNoHeap, true},
{"carstate::setCameraMode", true, true, 0},
{"carstate::setOverrideMode", true, true, 0},
{"carstate::nextCycleMode", true, true, 0},
{"carstate::buildCameraCycle", true, true, 0},
{"carstate::modeInCycle", true, true, 0},
{"carstate::rememberMode", true, true, 0},
{"carstate::cameraMode", true, true, 0},
{"carstate::setModeAnew", true, true, 0},
{"carstate::settleCamera", true, true, 0, true, kNoHeap, true},
{"carstate::nearestSpot", true, true, 0, true, kNoHeap, true},
{"carstate::scaledIfSet", true, true, 0, true, kNoHeap, true},
{"carstate::bitsBroken", true, true, 0},
{"carstate::dampVector", true, true, 0, true, kNoHeap, true},
{"carstate::chaseOffset", true, true, 0, true, kNoHeap, true},
{"carstate::chaseOffsetLevel", true, true, 0, true, kNoHeap, true},
#elif defined(CARSTATE_ARGUMENTS)
/* Where the cameras' cases keep their own: the cameras' records (the 25
 * modes of the two players, 0x140 apart), the cars (0xa00 each), what the
 * cars' and the cameras' pointers point at, the track's nodes, the spots,
 * the surfaces' heights, and the arguments' buffers. */
constexpr x86::reg32 kCamRecords = 0x648000, kCamCars = 0x64c000, kCamExtra = 0x651000;
constexpr x86::reg32 kCamNodes = 0x652000, kCamSpots = 0x653000, kCamHeights = 0x654000, kCamArgs = 0x655000;
constexpr x86::reg32 kCamCarCount = 8, kCamNodeCount = 64;
/* The game's, as native_carstate.cpp names them: the cameras of each mode,
 * those in use and overriding, the marks of a change; the cars, their count,
 * the players' cars. */
constexpr x86::reg32 kCamModes = 0x552ec8, kCamInUse = 0x5e10b0, kCamOverride = 0x5e10b8, kCamChanged = 0x552ff4;
constexpr x86::reg32 kCamCarsAt = 0x5efac8, kCamCarCountAt = 0x5efd9c, kCamPlayerCars = 0x5efa48;

x86::reg32 camRecord(x86::reg32 mode, x86::reg32 player)
{
    return kCamRecords + (mode * 2 + player) * 0x140;
}

x86::reg32 camCar(x86::reg32 i)
{
    return kCamCars + i * 0xa00;
}

/* A float of the cameras' sort: a place's or a size's mostly, small, 0 or
 * -0 now and then, now and then any. */
x86::reg32 camFloat()
{
    float f;
    switch (below(20))
    {
    case 0: return 0;
    case 1: return 0x80000000;
    case 2: f = float(int(below(2001)) - 1000) / 1000.0f; break;
    case 3: return anyFloatBits(random32() | std::uint64_t(random32()) << 32) & 0xbfffffff;  // no NaN of two kinds
    default: f = float(int(below(20001)) - 10000) / 10.0f; break;
    }
    x86::reg32 bits;
    std::memcpy(&bits, &f, 4);
    return bits;
}

x86::reg32 camFloatOf(float f)
{
    x86::reg32 bits;
    std::memcpy(&bits, &f, 4);
    return bits;
}

/* A float in [lo, hi). */
x86::reg32 camBetween(float lo, float hi)
{
    return camFloatOf(lo + (hi - lo) * float(below(100000)) / 100000.0f);
}

x86::reg32 camMode()
{
    return chance(15) ? 0x13 : below(25);
}

x86::reg32 camNode()
{
    return below(kCamNodeCount);
}

/* A track place of the cars' and the cameras' (30 dwords at `at`): its node,
 * its sides' marks, its pointers to vectors. */
void makeTrackPlace(CheckApp& g, x86::reg32 at)
{
    for (x86::reg32 i = 0; i < 0x78; i += 4)
        g.word(at + i) = camFloat();
    g.word(at + 0x10) = camNode();
    g.word(at + 0x14) = chance(25) ? 0 : 1 + below(5);
    g.word(at + 0x18) = chance(25) ? 0 : 1 + below(5);
    g.word(at + 0x20) = kCamExtra + 0x10 * below(8);
    for (x86::reg32 k = 0; k < 4; ++k)
        g.word(at + 0x54 + 4 * k) = kCamExtra + 0x100 + 0x10 * below(32);
    g.word(at + 0x6c) = chance(40) ? 0 : chance(70) ? 1 + below(2) : random32();
}

/* The cameras' world: the race's mode and switches; the cars, each with its
 * place, orientation, sizes, speed, steering and track place; the track's
 * nodes; the cameras of every mode and their tables; the cameras in use and
 * overriding; the cycle of modes and the player's choices; the spots. */
void makeCameraWorld(CheckApp& g)
{
    // The code's constants (0x4212b0..0x421360): the speed bands, the up vector, zeros.
    static const x86::reg32 bands[] = {0x40000, 0xa0000, 0x140000, 0x280000, 0x3c0000, 0x500000, 0x700000, 0x8c0000};
    for (x86::reg32 i = 0; i < 8; ++i)
        g.word(0x4212b0 + 4 * i) = bands[i];
    g.word(0x4212d0) = 0;
    g.word(0x4212d4) = 0x3f800000;
    g.word(0x4212d8) = 0;
    for (x86::reg32 at = 0x4212dc; at < 0x421360; at += 4)
        g.word(at) = 0;
    // The race's mode and switches.
    g.word(0x6fd3b0) = chance(50) ? 1 : chance(50) ? 0 : chance(50) ? 2 : chance(50) ? 4 : below(8);
    g.word(0x6fd4f4) = below(kCamCarCount);
    g.word(0x6fd3a0) = chance(30) ? 2 : below(4);
    g.word(0x6fd3b8) = chance(50) ? 0 : 1;
    g.word(0x6fd4c0) = chance(50) ? 0 : 1;
    g.getMemory<x86::reg8>(0x55eb2e) = x86::reg8(random32());
    g.word(0x559264) = chance(70) ? 0 : 1;
    g.word(0x55fe30) = chance(70) ? 0 : 1;
    g.word(0x7922fc) = chance(50) ? 0 : 1;
    g.word(0x79c4a0) = chance(70) ? 0 : 1;
    g.word(0x79c4a4) = chance(70) ? 0 : 1;
    g.word(0x7d3684) = chance(80) ? 320 + below(1300) : below(320);
    g.getMemory<x86::reg16>(0x7a3a58) = x86::reg16(chance(70) ? 0 : random32());
    g.word(0x5600ec) = random32();
    // handling.inc's mark (0xd13ffc): its dot product's small results -- a
    // car's axes' parts within a car's size now and then, for keepOutOfCars.
    g.word(0xd13ffc) = x86::reg32(g_seed);
    // The cars.
    const x86::reg32 count = chance(10) ? 0 : 1 + below(kCamCarCount);
    g.word(kCamCarCountAt) = count;
    g.word(0x5efd98) = below(count + 1);
    g.word(0x5efda4) = count - g.word(0x5efd98) + (chance(20) ? below(3) : 0);
    for (x86::reg32 i = 0; i < kCamCarCount; ++i)
    {
        const x86::reg32 car = camCar(i);
        for (x86::reg32 at = car; at < car + 0xa00; at += 4)
            g.word(at) = camFloat();
        makeTrackPlace(g, car + 0xc);
        g.word(car + 0x2c) = kCamExtra + 0x10 * below(8);
        g.getMemory<x86::reg8>(car + 0x88) = x86::reg8(chance(80) ? 0 : 1);
        g.getMemory<x86::reg8>(car + 0x89) = x86::reg8(chance(80) ? 1 + below(3) : 0);
        for (x86::reg32 k = 0; k < 3; ++k)
            g.word(car + 0x98 + 4 * k) = camBetween(-2000.0f, 2000.0f);
        for (x86::reg32 k = 0; k < 4; ++k)
            g.word(car + 0x114 + 4 * k) = camBetween(0.5f, 6.0f);
        g.getMemory<x86::reg8>(car + 0x200) = x86::reg8(random32());
        g.word(car + 0x2c4) = chance(30) ? 3 : below(5);
        g.word(car + 0x4e8) = camBetween(-5.0f, 5.0f);
        g.word(car + 0x5a4) = camBetween(0.0f, 120.0f);
        /* The speed, under the last of pickChaseNode's bands (0x8c0000 at 0x536e04's scale): past it the
         * code reads its own saved registers as bands, which no car's speed reaches. */
        {
            const x86::reg32 odd = camFloat();
            float speed;
            std::memcpy(&speed, &odd, 4);
            g.word(car + 0x7c0) = chance(10) && !(speed >= 140.0f) ? odd : camBetween(-10.0f, 140.0f);
        }
        g.word(car + 0x920) = camBetween(-1.0f, 1.0f);
        g.word(car + 0x924) = camBetween(-1.0f, 1.0f);
        g.word(car + 0x9a8) = below(4);
        g.word(kCamCarsAt + 4 * i) = car;
        g.word(kCamPlayerCars + 4 * i) = camCar(chance(80) ? i : below(kCamCarCount));
    }
    // Below the cars' table, where a car's index is negative: cars too.
    for (x86::reg32 i = 1; i <= 8; ++i)
        g.word(kCamCarsAt - 4 * i) = camCar(below(kCamCarCount));
    // What the pointers point at: shorts of the sides, vectors.
    for (x86::reg32 at = kCamExtra; at < kCamExtra + 0x80; at += 2)
        g.getMemory<x86::reg16>(at) = x86::reg16(chance(50) ? below(0x100) : 0x8000 | below(0x100));
    for (x86::reg32 at = kCamExtra + 0x100; at < kCamExtra + 0x300; at += 4)
        g.word(at) = camFloat();
    // The track's nodes: their places in fixed point, their vectors' bytes.
    g.word(0x5dd940) = chance(90) ? kCamNodeCount : 1 + below(kCamNodeCount);
    g.word(0x5dd948) = kCamNodes;
    for (x86::reg32 i = 0; i < kCamNodeCount; ++i)
    {
        const x86::reg32 node = kCamNodes + 0x24 * i;
        for (x86::reg32 k = 0; k < 3; ++k)
            g.word(node + 4 * k) = x86::reg32(x86::sreg32(below(0x8000000)) - 0x4000000);
        for (x86::reg32 at = node + 0xc; at < node + 0x24; ++at)
            g.getMemory<x86::reg8>(at) = x86::reg8(random32());
    }
    // The surfaces' heights by node, the spots.
    g.word(0x552e54) = kCamHeights;
    for (x86::reg32 i = 0; i < kCamNodeCount; ++i)
        g.word(kCamHeights + 4 * i) = camBetween(-5.0f, 30.0f);
    g.word(0x552e4c) = chance(10) ? 0 : 1 + below(16);
    g.word(0x552e50) = kCamSpots;
    for (x86::reg32 i = 0; i < 16; ++i)
    {
        const x86::reg32 spot = kCamSpots + 0x1c * i;
        for (x86::reg32 at = spot; at < spot + 0x1c; at += 4)
            g.word(at) = random32();
        g.word(spot + 0x18) = chance(80) ? camNode() : x86::reg32(x86::sreg32(below(200)) - 100);
    }
    // The cameras: every mode's of both players.
    for (x86::reg32 mode = 0; mode < 25; ++mode)
        for (x86::reg32 player = 0; player < 2; ++player)
        {
            const x86::reg32 camera = camRecord(mode, player);
            g.word(kCamModes + mode * 8 + player * 4) = camera;
            for (x86::reg32 at = camera; at < camera + 0x140; at += 4)
                g.word(at) = camFloat();
            g.word(camera) = chance(85) ? mode : camMode();
            g.word(camera + 4) = camCar(below(kCamCarCount));
            g.word(camera + 8) = chance(40) ? g.word(camera + 4) : camCar(below(kCamCarCount));
            g.word(camera + 0xc) = chance(80) ? player : below(2);
            g.word(camera + 0x10) = 0;
            for (x86::reg32 k = 0; k < 25; ++k)
                g.getMemory<x86::reg8>(camera + 0x58 + k) = x86::reg8(chance(60) ? 1 : 0);
            makeTrackPlace(g, camera + 0x74);
            g.word(camera + 0xec) = chance(40) ? 0xffffffff : camNode();
            for (const x86::reg32 flag : {0xf0u, 0xf4u, 0xf8u, 0xfcu, 0x104u, 0x108u, 0x10cu})
                g.word(camera + flag) = chance(50) ? 0 : chance(80) ? 1 : 2;
            g.word(camera + 0x110) = camBetween(-2.0f, 3.0f);
            g.word(camera + 0x118) = chance(40) ? 0xffffffff : chance(30) ? 0 : 1 + below(25);
            g.word(camera + 0x11c) = chance(10) ? 0 : below(25);
        }
    // The modes' table past the cameras' (0x552f90, a mode for each).
    for (x86::reg32 mode = 0; mode < 25; ++mode)
        g.word(0x552f90 + 4 * mode) = below(25);
    // The cameras in use and overriding, the changes' marks.
    for (x86::reg32 player = 0; player < 2; ++player)
    {
        g.word(kCamInUse + 4 * player) = chance(5) ? 0 : camRecord(camMode(), chance(85) ? player : below(2));
        g.word(kCamOverride + 4 * player) = chance(60) ? 0 : camRecord(camMode(), player);
        g.word(kCamChanged + 4 * player) = below(2);
        g.word(0x553004 + 4 * player) = random32();
    }
    for (x86::reg32 at = 0x5e1000; at < 0x5e10b0; at += 4)
        g.word(at) = camFloat();
    // The cycle (each mode's next) and the player's four choices.
    for (x86::reg32 mode = 0; mode < 25; ++mode)
        g.word(0x5e103c + 4 * mode) = chance(10) ? 0 : 1 + below(24);
    g.word(0x5e10c0) = below(6);
    for (x86::reg32 i = 0; i < 4; ++i)
        g.word(0x6fbbc0 + 4 * i) = chance(40) ? 0 : 1 + below(24);
    g.word(0x6fbbb8) = random32();
    g.word(0x6fbbbc) = random32();
    // The players' views.
    for (x86::reg32 at = 0x5dd950; at < 0x5dd950 + 0xc0; at += 4)
        g.word(at) = camFloat();
    // The arguments' buffers.
    for (x86::reg32 at = kCamArgs; at < kCamArgs + 0x3000; at += 4)
        g.word(at) = camFloat();
}

/* A player's index: 0 or 1 mostly, any now and then (the code takes it as
 * `index & 1` in a split screen only). */
x86::reg32 camPlayer()
{
    return chance(90) ? below(2) : below(8);
}

bool makeCarstateArguments(CheckApp& g, const std::string& name, x86::CPU& cpu, x86::reg32* stack)
{
    makeCameraWorld(g);
    // A camera's record, of a player and a mode; a buffer of the arguments'.
    const auto camera = [&] { return camRecord(below(25), below(2)); };
    const auto buffer = [&] { return kCamArgs + 0x100 * below(0x30); };
    if (name == "viewFromCamera" || name == "viewFromScreenCamera" || name == "chooseView")
    {
        cpu.eax = buffer();
        g.word(cpu.eax) = below(4);
        g.word(cpu.eax + 4) = below(2);
        cpu.edx = chance(80) ? below(3) : random32();
    }
    else if (name == "pickChaseNode" || name == "cameraNode")
        cpu.eax = below(2);
    else if (name == "turnAround")
    {
        cpu.eax = buffer();
        cpu.edx = chance(90) ? buffer() : cpu.eax;
    }
    else if (name == "attachCameras")
    {
        cpu.eax = camPlayer();
        cpu.edx = chance(85) ? below(kCamCarCount) : chance(50) ? kCamCarCount + below(8) : 0u - (1 + below(8));
    }
    else if (name == "headTurn" || name == "chaseOffset" || name == "chaseOffsetLevel")
        cpu.eax = camera();
    else if (name == "cameraZoom")
    {
        // A view's: its kind (0 a player's), its player.
        cpu.eax = buffer();
        g.word(cpu.eax) = chance(70) ? 0 : 1 + below(3);
        g.word(cpu.eax + 4) = below(2);
    }
    else if (name == "cameraTrackPlace" || name == "keepAboveRoad" || name == "keepOnTrack"
             || name == "keepFromRightWall" || name == "keepFromLeftWall" || name == "keepAboveGround"
             || name == "lookAtTarget")
    {
        cpu.eax = camera();
        cpu.edx = buffer();
        // keepOnTrack's point on the stack.
        for (x86::reg32 k = 0; k < 3; ++k)
            stack[k] = chance(90) ? camBetween(-2000.0f, 2000.0f) : camFloat();
    }
    else if (name == "keepOutOfCars")
    {
        cpu.eax = buffer();
        // Near a car now and then.
        if (chance(60) && g.word(kCamCarCountAt) != 0)
        {
            const x86::reg32 car = g.word(kCamCarsAt + 4 * below(g.word(kCamCarCountAt)));
            for (x86::reg32 k = 0; k < 3; ++k)
            {
                float f;
                std::memcpy(&f, &g.getMemory<x86::reg8>(car + 0x98 + 4 * k), 4);
                g.word(cpu.eax + 4 * k) = camFloatOf(f + float(int(below(2001)) - 1000) / 200.0f);
            }
        }
    }
    else if (name == "blendCamera")
    {
        cpu.eax = buffer();
        cpu.edx = buffer();
        cpu.ebx = below(2);
        cpu.ecx = chance(50) ? 1 : below(3);
    }
    else if (name == "placeCamera")
    {
        cpu.eax = camera();
        cpu.edx = buffer();
        cpu.ebx = buffer() + 0x80;
        cpu.ecx = chance(50) ? 1 : below(3);
    }
    else if (name == "switchCamera")
    {
        cpu.eax = camMode();
        cpu.edx = below(2);
    }
    else if (name == "viewMatrix")
        cpu.eax = buffer();
    else if (name == "updateCamera")
    {
        cpu.eax = camPlayer();
        cpu.edx = buffer();
        cpu.ebx = chance(50) ? 0 : chance(80) ? 1 : 2;
    }
    else if (name == "cameraView")
    {
        cpu.eax = camPlayer();
        cpu.edx = buffer();
    }
    else if (name == "cockpitShown" || name == "cameraCar" || name == "cameraTarget" || name == "cameraMode"
             || name == "settleCamera" || name == "nearestSpot")
        cpu.eax = camPlayer();
    else if (name == "setCameraMode" || name == "setOverrideMode" || name == "rememberMode" || name == "setModeAnew")
    {
        cpu.eax = name == "setCameraMode" || name == "setOverrideMode" ? below(2) : camPlayer();
        cpu.edx = chance(10) ? 0 : camMode();
    }
    else if (name == "nextCycleMode")
        cpu.eax = chance(50) ? 0 : chance(80) ? 1 : random32();
    else if (name == "modeInCycle")
        cpu.eax = chance(90) ? below(25) : random32();
    else if (name == "scaledIfSet")
    {
        cpu.eax = buffer();
        g.word(cpu.eax + 4) = chance(50) ? 0 : random32();
        stack[0] = camFloat();
    }
    else if (name == "bitsBroken")
    {
        // A run of set bits mostly, at any place; one broken now and then; any.
        const x86::reg32 length = 1 + below(16), at = below(17 - length);
        x86::reg32 bits = ((1u << length) - 1) << at;
        if (chance(30))
            bits ^= 1u << below(16);
        cpu.eax = chance(80) ? (random32() & 0xffff0000) | bits : random32();
    }
    else if (name == "dampVector")
    {
        cpu.eax = camCar(below(kCamCarCount));
        stack[0] = chance(20) ? (chance(50) ? 0 : 0x80000000) : camBetween(-1.0f, 1.0f);
        stack[1] = buffer();
    }
    return true;
}
#endif
