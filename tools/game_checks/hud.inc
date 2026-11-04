// game_checks.cpp's parts for the race HUD, rewritten (src/nfs3hp/port_hud.cpp,
// port::hud), included in each table with the macro naming the part.  The
// functions compute in the host's doubles, rounded to a float where the game
// stores one; the x87 of the checks is therefore at the extended precision
// (the build's default), any rounding mode.
#if defined(HUD_HOOKS)
// nfs3hp_main.cpp: the HUD's reference size, a 4:3 screen fitted to the real one (tools/apply_hud_scale.py).
namespace nfs3hp
{
x86::reg32 hudReferenceWidth(win32::WinApplication* app)
{
    const x86::reg32 width = app->getMemory<x86::reg32>(0x7cdac8);
    const x86::reg32 height = app->getMemory<x86::reg32>(0x7cdacc);
    if (!height || width * 3 <= height * 4)
        return width;
    return (height * 4) / 3;
}
x86::reg32 hudReferenceHeight(win32::WinApplication* app)
{
    const x86::reg32 width = app->getMemory<x86::reg32>(0x7cdac8);
    const x86::reg32 height = app->getMemory<x86::reg32>(0x7cdacc);
    if (!width || height * 4 <= width * 3)
        return height;
    return (width * 3) / 4;
}
}
#elif defined(HUD_EFFECTS)
// kSomething,  -- a stand-in's effect of this file's
kHudSinCos,     // the sine and cosine of an angle: two plain floats into [edx] and [ebx]
kHudRecord,     // the camera target's record: the scratch one
kHudVertexPtrs, // a quad's two vertices: pointers into the scratch ones, into [edx] and [ebx]
kHudVertices,   // vertices taken on a surface: the scratch block, its address into [ebx]
kHudSuffix,     // an ordinal's suffix: one of the four strings the game has
kHudRoot,       // a square root: a positive float onto the x87 stack
kHudTimeText,   // a time as text into the buffer at eax: digits and a colon
#elif defined(HUD_EFFECT_CASES)
// case kSomething: ... break;
case kHudSinCos:
{
    static const float values[] = {0.0f, 0.5f, -0.5f, 1.0f, -1.0f, 0.25f, 0.866f, 3.0f};
    if (cpu.edx < kMemory - 4)
        getMemory<float>(cpu.edx) = values[(r >> 8) % 8];
    if (cpu.ebx < kMemory - 4)
        getMemory<float>(cpu.ebx) = values[(r >> 12) % 8];
    break;
}
case kHudRecord:
    result = 0x310000;
    break;
case kHudTimeText:
{
    static const char digits[] = "0123456789";
    const char shape[] = "99:99.9";
    if (cpu.eax < kMemory - 16)
    {
        for (unsigned i = 0; i < 8; ++i)
            getMemory<char>(cpu.eax + i) = shape[i] == '9' ? digits[(r >> (4 * i)) % 10] : shape[i];
    }
    break;
}
case kHudRoot:
    cpu.fpu.count += 1;
    cpu.fpu.st(0) = x86::Float(1.0 + double(r % 100000) / 7.0);
    break;
case kHudSuffix:
    result = 0x53b7cc + 4 * ((r >> 8) % 4);
    break;
case kHudVertices:
    if (cpu.ebx < kMemory - 4)
        getMemory<x86::reg32>(cpu.ebx) = 0x312000;
    break;
case kHudVertexPtrs:
    if (cpu.edx < kMemory - 4)
        getMemory<x86::reg32>(cpu.edx) = 0x311000 + 0x40 * ((r >> 8) % 4);
    if (cpu.ebx < kMemory - 4)
        getMemory<x86::reg32>(cpu.ebx) = 0x311100 + 0x40 * ((r >> 12) % 4);
    break;
#elif defined(HUD_WINDOWS)
// {from, to}, -- what the functions write, compared (and the world made for the cases)
// The HUD's gauge tables and records (0x791b68 to 0x792890, the turn at 0x7930fc).
{0x791000, 0x794000}, {0x725000, 0x726000}, {0x55b000, 0x55c000}, {0x55d000, 0x55e000}, {0x700000, 0x701000}, {0x563000, 0x564000}, {0x79c000, 0x79d000}, {0x320000, 0x3d0000}, {0x6fb000, 0x6fd000}, {0x749000, 0x74a000}, {0x725748, 0x749758}, {0x749a38, 0x76da48}, {0x724000, 0x725000}, {0x6fdc00, 0x6fdd00}, {0x320000, 0x340000}, {0x55e000, 0x55f000}, {0x6fd000, 0x6fe000},
#elif defined(HUD_GAME_FUNCTIONS)
// {address, "name", convention, registers, words, clobbers, through, deref, effect},
// What the HUD's functions call that is not held on its own: the text begun,
// prepared and ended, a gauge's quad (ten words).
{0x4d0d50, "the text begun", kWatcom, 0, 0, kEax, -1, 0, kNothing},
{0x4a33d0, "the text prepared", kWatcom, 0, 0, kEax, -1, 0, kNothing},
{0x4d1310, "the text ended", kWatcom, 0, 0, kEax, -1, 0, kNothing},
{0x487620, "a gauge's quad", kStdcall, 2, 10, kEax, -1, 0, kNothing},
{0x4ea970, "a sine and a cosine", kWatcom, 3, 0, kEax, -1, 0, kHudSinCos},
{0x497590, "an arc tangent", kStdcall, 0, 2, 0, -1, 0, kFloatBack},
{0x4d10e0, "a text centred", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x4d14f0, "a font's width", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x4897f0, "an aligned text", kStdcall, 1, 6, kEax, -1, 0, kNothing},
{0x481f90, "a pane's place", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x4853c0, "a pane sized", kWatcom, 2, 0, kEax, -1, 0, kNothing},
{0x4844c0, "a pane drawn", kWatcom, 2, 0, kEax, -1, 0, kNothing},
{0x48ccf0, "a view refreshed", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x49bf40, "vertices taken", kWatcom, 3, 0, kEax, -1, 0, kHudVertices},
{0x4896f0, "an ordinal suffix", kWatcom, 1, 0, kEax, -1, 0, kHudSuffix},
{0x4ea920, "a square root", kStdcall, 0, 1, kEax, -1, 0, kHudRoot},
{0x4e8f28, "an angle", kWatcom, 2, 0, kEax, -1, 0, kNothing},
{0x485c20, "floats swapped", kWatcom, 2, 0, kEax, -1, 0, kNothing},
{0x4870c0, "a rectangle's quad", kWatcom, 4, 0, kEax, -1, 0, kNothing},
{0x486f70, "a quad from the stack", kStdcall, 4, 10, kEax, -1, 0, kNothing},
{0x485f20, "a segment clipped", kWatcom, 4, 0, kEax, -1, 0, kNothing},
{0x4873e0, "a stroke drawn", kStdcall, 4, 1, kEax, -1, 0, kNothing},
{0x48cb50, "a point transformed", kStdcall, 1, 3, kEax, -1, 0, kNothing},
{0x48cc60, "a box drawn", kStdcall, 2, 4, kEax, -1, 0, kNothing},
{0x486df0, "a box quad of floats", kStdcall, 0, 10, kEax, -1, 0, kNothing},
{0x487a30, "a box quad", kWatcom, 4, 0, kEax, -1, 0, kNothing},
{0x480910, "a place laid out", kWatcom, 4, 0, kEax, -1, 0, kNothing},
{0x47fd80, "the HUD begun", kWatcom, 0, 0, kEax, -1, 0, kNothing},
{0x481290, "surfaces started", kWatcom, 4, 0, kEax, -1, 0, kNothing},
{0x4815d0, "bars started", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x4818b0, "bars again", kWatcom, 4, 0, kEax, -1, 0, kNothing},
{0x481510, "dials started", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x481570, "dials again", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x48d110, "the views fitted", kWatcom, 3, 0, kEax, -1, 0, kNothing},
{0x47fcb0, "a pane hidden", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x47fbe0, "a pane shown", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x480fc0, "a pane band", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x4812e0, "a pane band again", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x4800a0, "a pane gap", kWatcom, 2, 0, kEax, -1, 0, kNothing},
{0x481170, "a pane map", kWatcom, 3, 0, kEax, -1, 0, kNothing},
{0x4808b0, "a pane redrawn", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x481aa0, "a pane inset", kWatcom, 2, 0, kEax, -1, 0, kNothing},
{0x481c50, "a pane surface", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x47ee80, "a pane camera", kWatcom, 2, 0, kEax, -1, 0, kNothing},
{0x487210, "the HUD inset drawn", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x487340, "the HUD band drawn", kWatcom, 1, 0, kEax, -1, 0, kNothing},
{0x422aa0, "the cockpit shown", kWatcom, 1, 0, kEax, -1, 0, kMainThread},
{0x489750, "a time as text", kWatcom, 2, 0, kEax, -1, 0, kHudTimeText},
// What the HUD's start calls: each leaves its registers otherwise, now and then (the code goes on with them).
{0x47f590, "counters cleared", kWatcom, 0, 0, kEax | kEcx | kEdx | kEbx, -1, 0, kNothing},
{0x4771c0, "a slot added", kWatcom, 3, 0, kEax | kEcx | kEdx | kEbx, -1, 0, kNothing},
{0x477470, "a slot field", kWatcom, 3, 0, kEax | kEcx | kEdx | kEbx, -1, 0, kNothing},
{0x477070, "a slot base", kWatcom, 1, 0, kEax | kEcx | kEdx | kEbx, -1, 0, kNothing},
{0x49bc80, "a picture taken", kWatcom, 2, 0, kEax | kEcx | kEdx | kEbx, -1, 0, kNothing},
{0x48e780, "geometry prepared", kWatcom, 0, 0, kEax | kEcx | kEdx | kEbx, -1, 0, kNothing},
{0x47bc40, "batches initialised", kWatcom, 0, 0, kEax | kEcx | kEdx | kEbx, -1, 0, kNothing},
{0x48cdb0, "the map initialised", kWatcom, 0, 0, kEax | kEcx | kEdx | kEbx, -1, 0, kNothing},
{0x484c10, "tables started", kWatcom, 0, 0, kEax | kEcx | kEdx | kEbx, -1, 0, kNothing},
{0x48aa40, "columns started", kStdcall, 0, 2, kEax | kEcx | kEdx | kEbx, -1, 0, kNothing},
{0x480a00, "a pane scrolled", kWatcom, 3, 0, kEax | kEcx | kEdx | kEbx, -1, 0, kNothing},
#elif defined(HUD_PROMISES)
// {"port::hud::name", watcom, result, arguments, fpu, heap, liveOnly},
// eax and the registers the port keeps; not the x87's words (the game's code
// leaves the status of its last compare, which no caller reads).
{"port::hud::loadingTextPrepared", true, false, 0},
{"port::hud::floatsSwapped", true, true, 2},
{"port::hud::fourIntegersAppended", true, true, 4},
{"port::hud::pointTransformed", true, true, 1},
{"port::hud::boxDrawn", true, true, 2},
{"port::hud::indicatorGeometryUpdated", true, true, 1},
{"port::hud::textAlignedDrawn", true, true, 1},
{"port::hud::percentTextDrawn", true, true, 1},
{"port::hud::listRowsDrawn", true, true, 3},
{"port::hud::timeTextDrawn", true, true, 1},
{"port::hud::viewZoomStepped", true, true, 2},
{"port::hud::viewPaneStepped", true, true, 1},
{"port::hud::screenScalesSet", true, true, 0},
{"port::hud::rectangleQuadSet", true, true, 3},
{"port::hud::stackQuadSet", true, true, 3},
{"port::hud::labelTextSet", true, true, 4},
{"port::hud::pointClipped", true, true, 4},
{"port::hud::segmentClippedToRect", true, true, 4},
{"port::hud::segmentCutIntoPieces", true, true, 4},
{"port::hud::strokeClippedDrawn", true, false, 4},
{"port::hud::standingsGapMeasured", true, true, 0},
{"port::hud::floatsQuadSet", true, true, 0},
{"port::hud::hudStarted", true, true, 0},
{"port::hud::rankTextDrawn", true, true, 2},
{"port::hud::labelRowsDrawn", true, false, 4},
{"port::hud::standingsDrawn", true, false, 0},
{"port::hud::columnsStarted", true, true, 0},
{"port::hud::mapOutlineStarted", true, true, 0},
{"port::hud::hudViewsSet", true, true, 1},
{"port::hud::hudPaneSet", true, true, 1},
{"port::hud::hudTablesStarted", true, true, 0},
{"port::hud::indicatorQuadSet", true, true, 4},
{"port::hud::gaugeQuadClipped", true, true, 2},
{"port::hud::segmentsMet", true, true, 4},
{"port::hud::boxQuadsSet", true, true, 4},
{"port::hud::markersSet", true, true, 4},
{"port::hud::mapPointsDrawn", true, false, 2},
#elif defined(HUD_ARGUMENTS)
constexpr x86::reg32 kHudScratch = 0x30f400;

/* The x87 as a HUD case finds it: the extended precision (what the build
 * computes in doubles), any rounding, a few values on the stack. */
void makeHudX87(x86::CPU& cpu, x86::reg32 most)
{
    cpu.fpu.setControl(x86::reg16(0x37f | below(4) << 10 | (random32() & 0x1000)));
    cpu.fpu.count = below(most);
    for (x86::Float& f : cpu.fpu.regs)
        f = x86::Float(anyDouble());
}

/* A float of the HUD's: a screen place mostly, any bits now and then. */
x86::reg32 aHudFloat()
{
    if (chance(12))
    {
        // Any bits but a NaN: of two the x87 picks the larger payload, the host the first operand's, and a
        // chs turns a sign the game's code would not have.
        x86::reg32 bits = anyFloatBits((std::uint64_t(random32()) << 32) | random32());
        if ((bits & 0x7f800000) == 0x7f800000 && (bits & 0x7fffff) != 0)
            bits = 0x3f800000;
        return bits;
    }
    const float f = float(int(below(4000)) - 2000) / (chance(50) ? 1.0f : 8.0f);
    x86::reg32 bits;
    std::memcpy(&bits, &f, 4);
    return bits;
}

void makeHudWorld(CheckApp& g, x86::CPU& cpu)
{
    makeHudX87(cpu, 4);
    for (x86::reg32 at = kHudScratch; at < kHudScratch + 0x200; at += 4)
        g.word(at) = aHudFloat();
    // The gauges' tables (four of each) and the HUD's offset, scale and turn.
    for (x86::reg32 i = 0; i < 4; ++i)
    {
        g.word(0x792878 + 4 * i) = aHudFloat();
        g.word(0x792880 + 4 * i) = chance(50) ? random32() : 0xff000000 | below(0x1000000);
        g.word(0x792888 + 4 * i) = aHudFloat();
        g.word(0x792890 + 4 * i) = aHudFloat();
    }
    for (const x86::reg32 at : {0x79c114u, 0x79c11cu, 0x79c120u, 0x7930fcu, 0x793100u})
        g.word(at) = aHudFloat();
    g.word(0x79c10c) = below(chance(90) ? 3 : 20);
    g.word(0x79c110) = below(chance(90) ? 3 : 20);
    for (x86::reg32 i = 0; i < 0x348 * 3; i += 4)
        g.word(0x6fbc70 + i) = chance(50) ? 1 : below(3);
    g.word(0x6fd4c4) = chance(50) ? 0 : 1;
}

bool makeHudArguments(CheckApp& g, const std::string& name, x86::CPU& cpu, x86::reg32* a)
{
    makeHudWorld(g, cpu);
    cpu.eax = random32();
    cpu.edx = random32();
    if (name == "floatsSwapped")
    {
        cpu.eax = kHudScratch + 4 * below(8);
        cpu.edx = chance(10) ? cpu.eax : kHudScratch + 0x40 + 4 * below(8);
    }
    else if (name == "fourIntegersAppended")
    {
        cpu.eax = below(4);
        cpu.edx = chance(80) ? below(2000) - 1000 : random32();
        cpu.ebx = chance(80) ? below(2000) - 1000 : random32();
        cpu.ecx = chance(80) ? below(2000) - 1000 : random32();
        a[0] = chance(80) ? below(2000) - 1000 : random32();
        for (x86::reg32 i = 0; i < 4; ++i)
            g.word(0x55d694 + 4 * i) = below(chance(95) ? 6 : 12);
    }
    else if (name == "pointTransformed")
    {
        cpu.eax = below(4);
        a[0] = aHudFloat();
        a[1] = aHudFloat();
        a[2] = kHudScratch + 0x100 + 4 * below(8);
    }
    else if (name == "indicatorGeometryUpdated")
    {
        cpu.eax = below(4);
        a[0] = aHudFloat();
        a[1] = random32();
        g.word(0x791b38) = 0x30f800;
        g.word(0x791b14) = aHudFloat();
    }
    else if (name == "textAlignedDrawn")
    {
        cpu.eax = 0x30f700;
        a[0] = aHudFloat();
        a[1] = random32();
        a[2] = chance(90) ? below(700) : random32();
        a[3] = chance(90) ? below(100) : random32();
        a[4] = below(480);
        a[5] = chance(30) ? 1 : chance(30) ? 2 : below(4);
        g.word(0x7cdac8) = chance(90) ? 500 + below(200) : random32() % 5000;
    }
    else if (name == "percentTextDrawn")
    {
        makeLoopWorld(g, cpu);
        makeHudX87(cpu, 4);
        cpu.eax = random32();
        a[0] = aHudFloat();
        a[1] = below(2);
        a[2] = random32();
        a[3] = random32();
        a[4] = below(480);
        a[5] = below(640);
    }
    else if (name == "listRowsDrawn")
    {
        makeLoopWorld(g, cpu);
        saneQuadLists(g);
        makeHudX87(cpu, 4);
        cpu.eax = below(5);
        cpu.edx = below(9);
        cpu.ebx = below(9);
        a[0] = aHudFloat();
        a[1] = random32();
        a[2] = random32();
        a[3] = random32();
        a[4] = below(100);
        a[5] = 100 + below(300);
        a[6] = below(640);
    }
    else if (name == "timeTextDrawn")
    {
        makeLoopWorld(g, cpu);
        makeHudX87(cpu, 4);
        cpu.eax = chance(90) ? below(600000) : random32();
        for (x86::reg32 i = 0; i < 5; ++i)
            a[i] = random32();
        a[3] = below(480);
        g.word(0x6fbb4c) = below(8);
    }
    else if (name == "viewZoomStepped")
    {
        cpu.eax = below(4);
        cpu.edx = chance(40) ? 0 : chance(60) ? 1 : below(4);
        g.word(0x6fd3b0) = below(3);
        for (x86::reg32 i = 0; i < 16; ++i)
            g.word(0x7925d8 + 4 * i) = aHudFloat();
        for (x86::reg32 i = 0; i < 0x348 * 4; i += 4)
            g.word(0x6fbc50 + i) = chance(40) ? 0x3f800000 : aHudFloat();
        for (x86::reg32 at = 0x310000; at < 0x310400; at += 4)
            g.word(at) = random32();
    }
    else if (name == "viewPaneStepped")
    {
        cpu.eax = below(3);
        g.word(0x6fd3b0) = below(3);
        for (x86::reg32 i = 0; i < 0x348 * 4; i += 4)
            g.word(0x6fbc50 + i) = chance(40) ? 0x3f800000 : chance(30) ? 0 : aHudFloat();
        // The panes' states, and rectangles that have settled (nothing in +4, 1.0 in +0xc) now and then.
        for (x86::reg32 index = 0; index < 4; ++index)
            for (x86::reg32 side = 0; side < 2; ++side)
            {
                const x86::reg32 states = 0x6fbc50 + index * 0x348 + side * 0x1a4;
                const x86::reg32 rect = states + 0x24 + ((side ? 0x16 : 0x11) << 4);
                g.word(states + 0x20) = chance(50) ? 1 : below(3);
                if (chance(60))
                {
                    g.word(rect + 4) = chance(70) ? 0 : 0x80000000;
                    g.word(rect + 0xc) = 0x3f800000;
                }
            }
        for (x86::reg32 at = 0x310000; at < 0x310400; at += 4)
            g.word(at) = random32();
    }
    else if (name == "screenScalesSet")
        g.word(0x7cdac8) = chance(50) ? 640 + below(1500) : below(700);
    else if (name == "rectangleQuadSet")
    {
        // The surface is a font's list of quads, the real ones taking and queueing on both sides.
        makeMenusWorld(g, cpu);
        saneQuadLists(g);
        makeHudX87(cpu, 4);
        cpu.eax = kHudScratch + 4 * below(4);
        cpu.edx = random32();
        cpu.ebx = 0x7f74c0 + 0x24010 * below(2);
        cpu.ecx = random32();
        g.word(0x6fd4c8) = below(2);
    }
    else if (name == "stackQuadSet")
    {
        makeMenusWorld(g, cpu);
        saneQuadLists(g);
        makeHudX87(cpu, 4);
        for (x86::reg32 i = 0; i < 10; ++i)
            a[i] = aHudFloat();
        a[9] = 0x7f74c0 + 0x24010 * below(2);
        cpu.ecx = random32();
        g.word(0x6fd4c8) = below(2);
    }
    else if (name == "labelTextSet")
    {
        makeLoopWorld(g, cpu);
        makeHudX87(cpu, 4);
        cpu.eax = below(4);
        cpu.edx = chance(90) ? below(8) : below(20);
        cpu.ebx = random32();
        cpu.ecx = chance(10) ? 0 : 0x310000;
        g.word(0x6fd3b0) = below(3);
        for (x86::reg32 at = 0x310000; at < 0x310800; at += 4)
            g.word(at) = chance(50) ? random32() : aHudFloat();
        g.word(0x310000 + 0x6f0) = chance(60) ? 0x40000000 + below(0x2000000) : below(0x40000000);
        for (x86::reg32 i = 0; i < 4; ++i)
            g.word(0x6fbe0c + 0x348 * i) = below(4);
    }
    else if (name == "pointClipped" || name == "segmentClippedToRect" || name == "segmentCutIntoPieces" ||
             name == "strokeClippedDrawn")
    {
        // A segment, a rectangle (left, top, right, bottom) and a point, in a screen's coordinates: ends inside
        // and out, now and then exactly on a side.
        const auto coord = [] {
            static const float edges[] = {0.0f, 100.0f, 200.0f, 300.0f, 400.0f};
            const float f = chance(25) ? edges[below(5)] : float(int(below(700)) - 30) + (chance(30) ? 0.5f : 0.0f);
            x86::reg32 bits;
            std::memcpy(&bits, &f, 4);
            return bits;
        };
        const x86::reg32 seg = 0x30f620, rect = 0x30f640, point = 0x30f600;
        for (x86::reg32 i = 0; i < 4; ++i)
        {
            g.word(seg + 4 * i) = coord();
            g.word(point + 4 * i) = coord();
        }
        const float left = float(int(below(250))), top = float(int(below(250)));
        const float rectangle[4] = {left, top, left + 50 + below(300), top + 50 + below(300)};
        std::memcpy(&g.getMemory<char>(rect), rectangle, 16);
        if (chance(5))
            g.word(seg + 8) = g.word(seg);     // a vertical or level run now and then
        if (chance(10))
            g.word(seg + 12) = g.word(seg + 4);
        g.getMemory<x86::reg32>(0x30f660) = 0;
        if (name == "pointClipped")
        {
            cpu.eax = point;
            cpu.edx = seg;
            cpu.ebx = rect;
            cpu.ecx = below(2);
            a[0] = 0x30f660;
        }
        else if (name == "segmentClippedToRect")
        {
            cpu.eax = seg;
            cpu.edx = rect;
            cpu.ebx = 0x30f680;
            cpu.ecx = 0x30f660;
        }
        else if (name == "segmentCutIntoPieces")
        {
            cpu.eax = seg;
            cpu.edx = rect;
            cpu.ebx = 0x30f6c0;
            cpu.ecx = 0x30f6e0;
        }
        else
        {
            cpu.eax = below(4);
            cpu.edx = chance(10) ? 0 : chance(60) ? 2 : 1;
            cpu.ebx = seg;
            cpu.ecx = random32();
            a[0] = 0x7f74c0;
            for (x86::reg32 group = 0; group < 4; ++group)
            {
                g.word(0x791b60 + 4 * group) = rect;
                g.word(0x55d694 + 4 * group) = below(chance(90) ? 4 : 8);
                for (x86::reg32 j = 0; j < 8; ++j)
                {
                    const float left2 = float(int(below(300))), top2 = float(int(below(300)));
                    float piece[4] = {left2, top2, left2 + 20 + below(250), top2 + 20 + below(250)};
                    if (chance(15))
                    {
                        // A piece that takes in the whole stroke: it is cut away.
                        piece[0] = piece[1] = -60.0f;
                        piece[2] = piece[3] = 900.0f;
                    }
                    std::memcpy(&g.getMemory<char>(0x791b68 + 80 * group + 16 * j), piece, 16);
                }
            }
        }
    }
    else if (name == "standingsGapMeasured")
    {
        makeHudX87(cpu, 4);
        const auto bitsOf = [](float f) {
            x86::reg32 bits;
            std::memcpy(&bits, &f, 4);
            return bits;
        };
        const x86::reg32 records[2] = {0x310000, 0x311000};
        for (const x86::reg32 at : records)
        {
            g.word(at + 8) = chance(8) ? 0 : chance(95) ? 1 + below(1500) : random32();
            g.getMemory<x86::reg8>(at + 0x200) = x86::reg8(below(4));
            g.word(at + 0x224) = below(2000);
            g.word(at + 0x228) = chance(3) ? 0x200 : 0x200 + below(1600);
            g.word(at + 0x2c4) = chance(60) ? 3 : below(5);
            g.word(at + 0x7c0) = chance(50) ? bitsOf(float(below(900)) / 2) : chance(50) ? aHudFloat() : bitsOf(float(below(20000)));
        }
        cpu.eax = records[below(2)];
        cpu.edx = chance(5) ? cpu.eax : records[below(2)];
        g.word(0x7d3684) = chance(90) ? 520 + below(1500) : below(0x400);
    }
    else if (name == "floatsQuadSet")
    {
        makeMenusWorld(g, cpu);
        saneQuadLists(g);
        makeHudX87(cpu, 4);
        const auto bitsOf = [](float f) {
            x86::reg32 bits;
            std::memcpy(&bits, &f, 4);
            return bits;
        };
        for (x86::reg32 i = 0; i < 10; ++i)
            a[i] = aHudFloat();
        if (chance(80))
        {
            // A box of a pixel or more each way, now and then just short of it: [1] to [3] across, [0] to [2] down.
            const float x = float(int(below(600))) + (chance(30) ? 0.5f : 0.0f), y = float(int(below(400))) + (chance(30) ? 0.5f : 0.0f);
            const float across = float(int(below(chance(10) ? 3 : 200))), down = float(int(below(chance(10) ? 3 : 200)));
            a[1] = bitsOf(x);
            a[3] = bitsOf(x + across);
            a[0] = bitsOf(y);
            a[2] = bitsOf(y + down);
        }
        a[9] = 0x7f74c0 + 0x24010 * below(2);
        cpu.ecx = random32();
    }
    else if (name == "hudStarted")
    {
        cpu.ebx = random32();
        g.word(0x55d2d8) = below(4);
        for (x86::reg32 i = 0; i < 4; ++i)
            g.word(0x55d2f0 + 4 * i) = chance(50) ? 0 : random32();
        g.word(0x791aec) = random32();
        g.word(0x791a98) = random32();
        g.word(0x7cdac8) = chance(80) ? 320 + below(1700) : random32();
        g.word(0x7cdacc) = chance(80) ? 240 + below(1000) : random32();
    }
    else if (name == "rankTextDrawn")
    {
        makeLoopWorld(g, cpu);
        saneQuadLists(g);
        makeHudX87(cpu, 4);
        cpu.eax = chance(90) ? 1 + below(24) : random32();
        cpu.edx = chance(90) ? 1 + below(40) : random32();
        a[0] = aHudFloat();
        a[1] = aHudFloat();
        a[2] = random32();
        a[3] = chance(90) ? below(800) - 50 : random32();
        a[4] = chance(90) ? below(800) : random32();
        a[5] = below(480);
        a[6] = chance(40) ? 2 : below(4);
        g.word(0x7cdac8) = chance(90) ? 500 + below(1500) : random32() % 5000;
    }
    else if (name == "labelRowsDrawn")
    {
        makeLoopWorld(g, cpu);
        saneQuadLists(g);
        makeHudX87(cpu, 4);
        g.word(0x30e004) = 0x4c42414c;
        // Room in the fonts' lists for all the rows' texts (a frame's worth, where the game flushes between frames).
        for (x86::reg32 font = 0; font < 2; ++font)
            g.word(0x7f74c0 + 0x24010 * font + 4) = 0x400 + below(0x100);
        const auto words = [&](x86::reg32 at, x86::reg32 length) {
            for (x86::reg32 i = 0; i < length; ++i)
                g.getMemory<x86::reg8>(at + i) = x86::reg8(' ' + below(95));
            g.getMemory<x86::reg8>(at + length) = 0;
        };
        cpu.eax = below(4);
        a[0] = aHudFloat();
        a[1] = below(300);
        a[2] = a[1] + below(500);
        a[3] = below(300);
        g.word(0x7cdac8) = chance(90) ? 500 + below(1500) : random32() % 5000;
        g.word(0x5efda0) = chance(90) ? below(9) : below(14);
        for (x86::reg32 i = 0; i < 8; ++i)
        {
            g.word(0x725238 + 4 * i) = chance(20) ? 0 : 0x310800 + 0x20 * i;
            words(0x310800 + 0x20 * i, below(12));
            g.word(0x725258 + 4 * i) = chance(30) ? 1 : below(100);
        }
        for (x86::reg32 p = 0; p < 4; ++p)
            for (x86::reg32 i = 0; i < 9; ++i)
            {
                const x86::reg32 label = 0x725278 + 320 * p + 40 * i;
                g.word(label) = chance(85) ? 1 + below(7) : random32();
                words(label + 4, below(20));
                g.word(label + 0x24) = 0x311000 + 0x400 * below(4);
            }
        for (x86::reg32 i = 0; i < 4; ++i)
        {
            g.word(0x311000 + 0x400 * i + 0x1f0) = below(6);
            g.word(0x5efac8 + 4 * i) = 0x312000 + 0x400 * i;
        }
        for (x86::reg32 i = 0; i < 6; ++i)
        {
            g.word(0x5efac8 + 4 * i) = 0x312000 + 0x400 * (i % 4);
            g.getMemory<x86::reg8>(0x312000 + 0x400 * (i % 4) + 0x200) = x86::reg8(below(2));
            g.word(0x312000 + 0x400 * (i % 4) + 0x2c4) = chance(30) ? 3 : chance(30) ? 4 : below(6);
        }
        g.word(0x55fe3c) = chance(80) ? 1 : 0;
        g.word(0x7a3d10) = chance(85) ? 0 : 1;
        g.getMemory<x86::reg8>(0x55e5ed) = x86::reg8(chance(80) ? 1 : below(4));
        g.word(0x6fd3a0) = below(4);
        g.word(0x7cd948) = below(2);
        g.word(0x55d6c0) = below(150);
        g.word(0x7925c0) = below(150);
        g.word(0x7d367c) = 600 + below(300);
        g.word(0x7d3678) = 600;
        // The mouse on a row of the columns, mostly: its x between the edges, its y a little below the top.
        g.word(0x55d6b0) = g.word(0x7d367c) + 8 - (a[1] + below(a[2] - a[1] + 10) - 5);
        g.word(0x55d6b4) = 600 - (a[3] + below(chance(80) ? 200 : 600));
        for (x86::reg32 p = 0; p < 4; ++p)
        {
            g.word(0x7497d8 + 368 * p) = below(400);
            g.word(0x7497dc + 368 * p) = below(300);
            g.word(0x7497e0 + 368 * p) = 400 + below(400);
            g.word(0x7497e4 + 368 * p) = 300 + below(300);
        }
    }
    else if (name == "standingsDrawn")
    {
        makeLoopWorld(g, cpu);
        saneQuadLists(g);
        makeHudX87(cpu, 4);
        for (x86::reg32 font = 0; font < 2; ++font)
            g.word(0x7f74c0 + 0x24010 * font + 4) = 0x400 + below(0x100);
        const auto words = [&](x86::reg32 at, x86::reg32 length) {
            for (x86::reg32 i = 0; i < length; ++i)
                g.getMemory<x86::reg8>(at + i) = x86::reg8(' ' + below(95));
            g.getMemory<x86::reg8>(at + length) = 0;
        };
        a[0] = aHudFloat();
        a[1] = below(300);
        a[2] = below(300);
        a[3] = below(4);
        g.word(0x7cdac8) = chance(90) ? 500 + below(1500) : random32() % 5000;
        g.word(0x7cdacc) = chance(90) ? 400 + below(800) : random32() % 3000;
        g.word(0x7254f8) = chance(90) ? below(7) : below(10);
        words(0x792320, 8);
        g.word(0x55d6c0) = below(200);
        g.word(0x55d6c4) = below(100);
        for (x86::reg32 i = 0; i < 10; ++i)
        {
            const x86::reg32 row = 0x7254fc + 24 * i;
            g.word(row) = 0x310800 + 0x20 * i;
            words(0x310800 + 0x20 * i, below(12));
            g.word(row + 4) = random32();
            g.word(row + 8) = chance(30) ? 0 : chance(50) ? below(100000) : 0 - below(100000);
            g.word(row + 0xc) = random32();
            g.word(row + 0x10) = chance(25) ? 1 : 0;
            g.word(row + 0x14) = below(6);
        }
        for (x86::reg32 i = 0; i < 6; ++i)
        {
            g.word(0x5efac8 + 4 * i) = 0x312000 + 0x400 * (i % 4);
            g.getMemory<x86::reg8>(0x312000 + 0x400 * (i % 4) + 0x200) = x86::reg8(below(2));
            g.word(0x312000 + 0x400 * (i % 4) + 0x2c4) = chance(30) ? 3 : chance(30) ? 4 : below(6);
        }
        g.word(0x55fe3c) = chance(80) ? 1 : 0;
        g.word(0x7a3d10) = chance(85) ? 0 : 1;
        g.getMemory<x86::reg8>(0x55e5ed) = x86::reg8(chance(80) ? 1 : below(4));
        g.word(0x6fd3a0) = below(4);
        g.word(0x7cd948) = below(2);
        g.word(0x7d367c) = 600 + below(300);
        g.word(0x7d3678) = 600;
        // The mouse over the rows, mostly: x past the left edge a little to the name's width, y from the top down.
        g.word(0x55d6b0) = g.word(0x7d367c) + 8 - (a[1] + 4 + below(260));
        g.word(0x55d6b4) = 600 - (a[2] + 4 + below(chance(80) ? 200 : 400));
        for (x86::reg32 p = 0; p < 4; ++p)
        {
            g.word(0x7497d8 + 368 * p) = below(400);
            g.word(0x7497dc + 368 * p) = below(300);
            g.word(0x7497e0 + 368 * p) = 400 + below(400);
            g.word(0x7497e4 + 368 * p) = 300 + below(300);
        }
    }
    else if (name == "columnsStarted")
    {
        makeLoopWorld(g, cpu);
        saneQuadLists(g);
        makeHudX87(cpu, 4);
        const auto bitsOf = [](float f) {
            x86::reg32 bits;
            std::memcpy(&bits, &f, 4);
            return bits;
        };
        a[0] = bitsOf(float(320 + below(1700)));
        a[1] = bitsOf(float(240 + below(1000)));
        g.word(0x791b4c) = bitsOf(1.0f + float(below(30)) / 16);
        // Every character goes somewhere: the bars of the time column are added until they are as wide as the time.
        for (x86::reg32 i = 0; i < 0x220; ++i)
            g.getMemory<x86::reg8>(0x83f4e0 + i) = x86::reg8(3 + below(10));
        g.word(0x7cdac8) = chance(90) ? 500 + below(1500) : random32() % 5000;
        g.word(0x7cdacc) = chance(90) ? 400 + below(800) : random32() % 3000;
        g.word(0x5efda0) = chance(90) ? below(8) : below(12);
        for (x86::reg32 i = 0; i < 12; ++i)
        {
            g.word(0x5efc48 + 4 * i) = chance(85) ? 0x312000 + 0x400 * (i % 4) : 0;
            g.word(0x312000 + 0x400 * (i % 4) + 0x220) = chance(90) ? 0x313000 + 0x100 * (i % 4) : 0;
            for (x86::reg32 k = 0; k < 12; ++k)
                g.getMemory<x86::reg8>(0x313000 + 0x100 * (i % 4) + 0x34 + k) = x86::reg8(' ' + below(95));
            g.getMemory<x86::reg8>(0x313000 + 0x100 * (i % 4) + 0x34 + below(12)) = 0;
        }
    }
    else if (name == "mapOutlineStarted")
    {
        cpu.ebx = random32();
        g.word(0x6fd3b0) = below(3);
        static const x86::reg32 zooms[] = {0x3f000000, 0x3f800000, 0x3fc00000, 0x40400000, 0x40800000, 0x40a00000, 0x80000000, 0xc0000000};
        for (const x86::reg32 at : {0x6fbc60u, 0x6fbc60u + 0x1a4, 0x6fbfa8u, 0x6fbfa8u + 0x1a4, 0x6fc2f0u, 0x6fc2f0u + 0x3480})
            g.word(at) = chance(85) ? zooms[below(8)] : random32();
        // (Fewer than 128 samples would step by nothing for ever, the game's code as much as the port's.)
        const x86::reg32 samples = chance(10) ? 0 - below(50) : 16384 + below(2000);
        const x86::reg32 base = chance(10) ? 0 : 0x320000;
        g.word(0x5dd940) = samples;
        g.word(0x5dd948) = base;
        const x86::reg32 step = int(samples) > 0 ? samples / 128 : 0;
        if (step > 0 && base != 0)   // (from 0 the samples would be in memory no window copies)
            for (x86::reg32 i = 0; int(i) < int(samples); i += step)
            {
                g.word(36 * i + base) = chance(95) ? below(1 << 22) - (1 << 21) : random32();
                g.word(36 * i + base + 8) = chance(95) ? below(1 << 22) - (1 << 21) : random32();
            }
        for (x86::reg32 c = 0; c < 16; ++c)
            g.word(0x6fd54c + 108 * c) = chance(30) ? (random32() & 0x03030303) : random32();
    }
    else if (name == "hudViewsSet")
    {
        cpu.eax = below(4);
        cpu.edx = random32();
        cpu.ebx = random32();
        cpu.ecx = random32();
        g.word(0x6fd3b0) = below(3);
        g.word(0x6fd3b8) = chance(60) ? 3 : below(5);
        for (x86::reg32 i = 0; i < 4; ++i)
            g.word(0x55d694 + 4 * i) = below(4);
        g.word(0x7cdac8) = chance(90) ? 500 + below(1500) : random32() % 5000;
        for (x86::reg32 at = 0x749758; at < 0x749758 + 368 * 4 + 0x200; at += 4)
            g.word(at) = chance(95) ? below(1000) - 100 : random32();
        for (x86::reg32 v = 0; v < 6; ++v)
            for (x86::reg32 slot = 0; slot < 2; ++slot)
            {
                const x86::reg32 at = 0x6fbc50 + 0x348 * v + 0x1a4 * slot;
                g.word(at + 0x10) = aHudFloat();
                g.word(at + 0xc) = chance(70) ? 0 : below(3);
                g.word(at + 0x1c) = chance(40) ? 2 : chance(40) ? 3 : below(6);
                g.word(at + 0x18) = chance(40) ? 4 : chance(40) ? 2 : below(6);
            }
        g.word(0x791b2c) = 0x310000;
        g.word(0x791b38) = 0x310800;
        for (x86::reg32 at = 0x310000; at < 0x311000; at += 4)
            g.word(at) = aHudFloat();
    }
    else if (name == "hudPaneSet")
    {
        cpu.eax = below(4);
        cpu.edx = random32();
        cpu.ebx = random32();
        cpu.ecx = random32();
        g.word(0x6fd3b0) = below(3);
        g.word(0x7cdac8) = 640;
        for (const x86::reg32 list : {0x725748u, 0x749a38u})
        {
            g.word(list + 4) = 0x100 + below(0x80);
            g.word(list + 0xc) = 4 * below(8);
        }
        for (const x86::reg32 at : {0x791acc, 0x791ac8, 0x791b30, 0x791b24, 0x791ac4, 0x791ad0, 0x791b20, 0x791b48, 0x791adc, 0x791ad8})
            g.word(at) = below(60);
        g.word(0x791b38) = 0x310800;
        g.word(0x791b2c) = 0x310000;
        for (x86::reg32 at = 0x310000; at < 0x311000; at += 4)
            g.word(at) = aHudFloat();
        g.word(0x791b1c) = below(2);
        g.word(0x6fd3a0) = below(4);
        g.getMemory<x86::reg8>(0x7a3a58) = x86::reg8(below(4));
        g.word(0x5efda0) = below(4);
        g.getMemory<x86::reg16>(0x791ae8) = x86::reg16(1 + below(2000));
        g.getMemory<x86::reg16>(0x791aea) = x86::reg16(1 + below(2000));
        for (x86::reg32 i = 0; i < 4; ++i)
            g.word(0x724760 + 4 * i) = aHudFloat();
        for (x86::reg32 v = 0; v < 6; ++v)
            for (x86::reg32 slot = 0; slot < 2; ++slot)
            {
                const x86::reg32 at = 0x6fbc50 + 0x348 * v + 0x1a4 * slot;
                g.word(at) = chance(70) ? 1 : 0;
                g.word(at + 8) = chance(60) ? 1 : 0;
                g.word(at + 0x18) = chance(40) ? 4 : chance(40) ? 2 : below(6);
                g.word(at + 0x1c) = chance(40) ? 3 : chance(40) ? 2 : below(6);
            }
        for (x86::reg32 i = 0; i < 64; ++i)
            g.getMemory<x86::reg8>(0xc00200 + 0x40 * i) = x86::reg8(random32() & 0x24 ? random32() : 0);
        g.word(0x7cdb10) = below(2000);
        g.word(0x7cdb14) = below(2000);
        g.word(0x7cdb90) = below(2000);
        g.word(0x7cdb94) = below(2000);
    }
    else if (name == "hudTablesStarted")
    {
        makeLoopWorld(g, cpu);
        makeHudX87(cpu, 4);
        cpu.eax = random32();
        cpu.ebx = random32();
        cpu.ecx = random32();
        cpu.edx = random32();
        g.word(0x7cdac8) = chance(90) ? 500 + below(1500) : random32() % 5000;
        g.word(0x7cdacc) = chance(90) ? 400 + below(800) : random32() % 3000;
        g.word(0x791b50) = 0x3f800000 + 0x40000 * below(16);
        g.word(0x791b38) = 0x310800;
        for (x86::reg32 at = 0x310800; at < 0x310900; at += 4)
            g.word(at) = aHudFloat();
    }
    else if (name == "indicatorQuadSet")
    {
        makeHudX87(cpu, 4);
        cpu.eax = below(4);
        cpu.edx = chance(80) ? below(2000) : random32();
        cpu.ebx = chance(80) ? below(2000) : random32();
        cpu.ecx = random32();
        g.word(0x6fd3b0) = below(3);
        g.word(0x791b2c) = 0x310000;
        g.word(0x791b34) = 0x310800;
        for (x86::reg32 at = 0x310000; at < 0x310400; at += 4)
            g.word(at) = aHudFloat();
        g.getMemory<x86::reg16>(0x791ae8) = x86::reg16(1 + below(2000));
        g.getMemory<x86::reg16>(0x791aea) = x86::reg16(1 + below(2000));
        for (x86::reg32 i = 0; i < 4; ++i)
        {
            g.word(0x724758 + 4 * i) = aHudFloat();
            g.word(0x724760 + 4 * i) = aHudFloat();
        }
        for (x86::reg32 v = 0; v < 6; ++v)
            for (x86::reg32 slot = 0; slot < 2; ++slot)
                g.word(0x6fbc6c + 0x348 * v + 0x1a4 * slot) = chance(60) ? 2 : below(5);
    }
    else if (name == "gaugeQuadClipped")
    {
        makeHudX87(cpu, 4);
        cpu.eax = below(4);
        cpu.edx = chance(50) ? 2 : below(4);
        for (x86::reg32 i = 0; i < 8; ++i)
            a[i] = aHudFloat();
        a[8] = random32();
        a[9] = random32();
        for (x86::reg32 grp = 0; grp < 4; ++grp)
        {
            g.word(0x791b60 + 4 * grp) = 0x310000 + 0x20 * grp;
            for (x86::reg32 k = 0; k < 4; ++k)
                g.word(0x310000 + 0x20 * grp + 4 * k) = aHudFloat();
            g.word(0x55d694 + 4 * grp) = below(4);
            for (x86::reg32 j = 0; j < 8; ++j)
                for (x86::reg32 k = 0; k < 4; ++k)
                    g.word(0x791b68 + 80 * grp + 16 * j + 4 * k) = aHudFloat();
        }
    }
    else if (name == "segmentsMet")
    {
        makeHudX87(cpu, 4);
        const auto half = [] {
            const float f = float(below(9)) / 2;
            x86::reg32 bits;
            std::memcpy(&bits, &f, 4);
            return bits;
        };
        for (x86::reg32 k = 0; k < 4; ++k)
        {
            g.word(0x30f620 + 4 * k) = half();
            g.word(0x30f640 + 4 * k) = half();
        }
        if (chance(10))
            for (x86::reg32 k = 0; k < 4; ++k)
                g.word(0x30f640 + 4 * k) = g.word(0x30f620 + 4 * k);
        cpu.eax = 0x30f620;
        cpu.ebx = 0x30f640;
        cpu.ecx = 0x30f6f0;
        cpu.edx = 0x30f660;
        for (x86::reg32 k = 0; k < 4; ++k)
            g.word(0x30f660 + 4 * k) = half();
        a[0] = 0x30f700;
        a[1] = 0x30f720;
    }
    else if (name == "boxQuadsSet")
    {
        makeHudX87(cpu, 4);
        const auto half = [] {
            const float f = float(below(9)) / 2 - 1;
            x86::reg32 bits;
            std::memcpy(&bits, &f, 4);
            return bits;
        };
        for (x86::reg32 k = 0; k < 12; ++k)
            g.word(0x30f800 + 4 * k) = half();
        for (x86::reg32 k = 0; k < 4; ++k)
            g.word(0x30f840 + 4 * k) = half();
        cpu.eax = 0x30f800;
        cpu.edx = 0x30f840;
        cpu.ebx = chance(60) ? 0xffffffff : random32();
        cpu.ecx = chance(60) ? 0x749a38 : random32();
    }
    else if (name == "markersSet")
    {
        makeFeDrawWorld(g, cpu);   // (THRASH's slots for the graphics natives it calls)
        makeHudX87(cpu, 4);
        cpu.eax = below(4);
        cpu.edx = 0x310000;
        cpu.ebx = 0x310400;
        for (x86::reg32 at = 0x310000; at < 0x310800; at += 4)
        {
            // (Sane cars: the steps along the track they work out are a few samples, not past its ends.)
            const float f = float(int(below(2000)) - 1000) / 8;
            std::memcpy(&g.getMemory<char>(at), &f, 4);
        }
        g.word(0x6fd4c4) = below(2);
        {
            // The track's samples (nine words each) from a base the window covers; the car's place among them.
            const x86::reg32 samples = 1000 + below(1000);
            g.word(0x5dd940) = samples;
            g.word(0x5dd948) = 0x320000;
            for (x86::reg32 i = 0; i < samples * 9; ++i)
                g.word(0x320000 + 4 * i) = below(1 << 21) - (1 << 20);
            cpu.ecx = below(samples);
        }
        g.word(0x791ac8) = below(60);
        for (x86::reg32 at = 0x749758; at < 0x749758 + 368 * 4; at += 4)
            g.word(at) = below(1000);
    }
    else if (name == "mapPointsDrawn")
    {
        makeFeDrawWorld(g, cpu);
        saneQuadLists(g);
        makeHudX87(cpu, 4);
        const auto sane = [] {
            const float f = float(int(below(2000)) - 1000) / 8;
            x86::reg32 bits;
            std::memcpy(&bits, &f, 4);
            return bits;
        };
        cpu.eax = below(4);
        cpu.edx = below(10);
        cpu.ebx = random32();
        cpu.ecx = random32();
        // A pool of car records: a word a small count or a sane float.
        for (x86::reg32 k = 0; k < 8; ++k)
            for (x86::reg32 at = 0x312000 + 0x1000 * k; at < 0x312000 + 0x1000 * k + 0xc00; at += 4)
                g.word(at) = chance(60) ? 0 : below(8);   // (a pointer among them reads low memory)
        g.word(0x5efd88) = below(8);   // (a count: the cars of the first table)
        for (const x86::reg32 table : {0x5efa88u, 0x5efac8u, 0x5efc48u})
            for (x86::reg32 i = 0; i < 16; ++i)
                g.word(table + 4 * i) = chance(90) ? 0x312000 + 0x1000 * below(8) : 0;
        g.word(0x5efda0) = below(8);
        {
            const x86::reg32 samples = 1000 + below(1000);
            g.word(0x5dd940) = samples;
            g.word(0x5dd948) = 0x320000;
            for (x86::reg32 i = 0; i < samples * 9; ++i)
                g.word(0x320000 + 4 * i) = below(1 << 21) - (1 << 20);
        }
        for (x86::reg32 at = 0x792870; at < 0x792910; at += 4)
            g.word(at) = sane();
        for (x86::reg32 at = 0x7925d0; at < 0x792610; at += 4)
            g.word(at) = sane();
        for (x86::reg32 at = 0x7930d8; at < 0x793110; at += 4)
            g.word(at) = sane();
        for (x86::reg32 at = 0x7497d8; at < 0x7497e8; at += 4)
            g.word(at) = below(700);
        for (x86::reg32 at = 0x6fbc50; at < 0x6fbc50 + 0x348 * 4; at += 4)
            g.word(at) = below(3);
        g.word(0x6fd3b0) = below(3);
        g.word(0x6fd3a0) = below(4);
        g.word(0x6fd4c4) = below(2);
        g.word(0x6fd4f4) = below(4);
        g.word(0x79c10c) = below(3);
        g.word(0x79c110) = below(3);
        g.word(0x55fe3c) = chance(70) ? 1 : 0;
        g.word(0x7a3d10) = chance(80) ? 0 : 1;
        g.getMemory<x86::reg8>(0x55e5ed) = x86::reg8(chance(70) ? 1 : below(4));
        g.word(0x7d367c) = 600 + below(300);
        g.word(0x7d3678) = 600;
        g.word(0x7d3684) = 520 + below(1500);
        g.word(0x55d000) = below(4);
    }
    else if (name == "boxDrawn")
    {
        cpu.eax = below(4);
        for (x86::reg32 i = 0; i < 4; ++i)
            a[i] = aHudFloat();
    }
    return true;
}
#endif
