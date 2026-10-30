// game_checks.cpp's parts for the functions tools/decomp.py rewrote
// (src/nfs3hp/port_conv.cpp, port::conv), included in each table with the
// macro naming the part.  The functions compute in the host's doubles through
// the emulated FPU's operations, rounded to a float where the game stores one;
// the x87 of the checks is at either of the game's precisions -- the menus'
// extended, a race's single -- rounding to nearest, as the game's words have it.
#if defined(CONV_HOOKS)
#elif defined(CONV_EFFECTS)
#elif defined(CONV_EFFECT_CASES)
#elif defined(CONV_WINDOWS)
// {from, to}, -- what the functions write, compared (and the world made for the cases)
// The pool of records the generic world points the registers and arguments at.
{0x312000, 0x31a000},
{0x5dd000, 0x5de000},
{0x5ef000, 0x5f0000},
{0x6fd000, 0x6fe000},
{0x7d3000, 0x7d4000},
{0x56e000, 0x56f000},
{0x5e1000, 0x5e2000},
{0x79c000, 0x79d000},
{0x7cd000, 0x7ce000},
{0x6fb000, 0x6fc000},
{0x5f3000, 0x5f4000},
{0x662000, 0x663000},
{0x6fc000, 0x6fd000},
{0x7a3000, 0x7a4000},
{0x677000, 0x678000},
{0x791000, 0x792000},
{0x724000, 0x725000},
{0x725000, 0x726000},
{0x749000, 0x74a000},
{0x678000, 0x679000},
{0x679000, 0x67a000},
{0x67b000, 0x67c000},
{0x6ff000, 0x700000},
{0x700000, 0x701000},
{0x7dd000, 0x7de000},
{0x79f000, 0x7a0000},
{0x7a1000, 0x7a2000},
{0x8b7000, 0x8b8000},
{0x7dc000, 0x7dd000},
{0x8b4000, 0x8b5000},
{0x7d5000, 0x7d6000},
{0x7d9000, 0x7da000},
{0x7f7000, 0x7f8000},
{0x840000, 0x841000},
{0x8b5000, 0x8b6000},
{0x8c0000, 0x8c1000},
{0x8c3000, 0x8c4000},
{0x8c5000, 0x8c6000},
{0x8c7000, 0x8c8000},
// @@WINDOWS@@
#elif defined(CONV_GAME_FUNCTIONS)
// {address, "name", convention, registers, words, clobbers, through, deref, effect},
// @@ROWS@@
#elif defined(CONV_PROMISES)
// {"port::conv::name", watcom, result, arguments, fpu, heap, liveOnly},
{"port::conv::fn49fe70", true, true, 4},
{"port::conv::fn4a00d0", true, true, 4},
{"port::conv::fn4a0440", true, true, 4},
{"port::conv::fn4a0980", true, true, 4},
{"port::conv::fn4a0a10", true, true, 4},
{"port::conv::fn4a0d20", true, true, 4},
{"port::conv::fn4a0f30", true, true, 4},
{"port::conv::fn4a1090", true, true, 4},
{"port::conv::fn4a20c0", true, true, 4},
{"port::conv::fn4a2630", true, true, 4},
{"port::conv::fn4a2830", true, true, 4},
{"port::conv::fn4a2c60", true, true, 4},
{"port::conv::fn4a2e20", true, true, 4},
// @@PROMISES@@
#elif defined(CONV_ARGUMENTS)
constexpr x86::reg32 kConvPool = 0x312000, kConvRecord = 0x1000;

/* A float of the pool's: a sane place or size, half the time with a whole
 * significand (so that a single-precision race rounds). */
x86::reg32 aConvFloat()
{
    float f = float(int(below(2000)) - 1000) / 8;
    if (chance(90))
        f += float(random32() & 0xffffff) / float(0x1000000);
    x86::reg32 bits;
    std::memcpy(&bits, &f, 4);
    return bits;
}

/* The generic world: the x87 at a race's or the menus' precision, a pool of 16 records
 * of mostly zeros, small counts and sane floats (a pointer among them reads
 * low memory), the registers and the stack's arguments each a pointer into
 * it, a small number or a float. */
void makeConvWorld(CheckApp& g, x86::CPU& cpu, x86::reg32* a)
{
    cpu.fpu.setControl(x86::reg16(chance(50) ? 0x07f : 0x37f));
    cpu.fpu.count = below(4);
    for (x86::Float& f : cpu.fpu.regs)
        f = x86::Float(anyDouble());
    for (x86::reg32 k = 0; k < 8; ++k)
        for (x86::reg32 at = kConvPool + kConvRecord * k; at < kConvPool + kConvRecord * (k + 1); at += 4)
            g.word(at) = chance(30) ? 0 : chance(25) ? below(8) : aConvFloat();
    const auto anArgument = [&] {
        return chance(40) ? kConvPool + kConvRecord * below(8) : chance(50) ? below(8) : aConvFloat();
    };
    cpu.eax = anArgument();
    cpu.edx = anArgument();
    cpu.ebx = anArgument();
    cpu.ecx = anArgument();
    for (x86::reg32 i = 0; i < 16; ++i)
        a[i] = anArgument();
}

bool makeConvArguments(CheckApp& g, const std::string& name, x86::CPU& cpu, x86::reg32* a)
{
    makeConvWorld(g, cpu, a);
    if (name == "fn49fe70")
    {
        cpu.eax = kConvPool + kConvRecord * below(8);
        cpu.edx = below(8);
        cpu.ebx = below(8);
        cpu.ecx = below(8);
    }
    if (name == "fn4a00d0")
    {
        cpu.eax = kConvPool + kConvRecord * below(8);
        cpu.edx = below(8);
        cpu.ebx = below(8);
        cpu.ecx = below(8);
    }
    if (name == "fn4a0440")
    {
        cpu.eax = kConvPool + kConvRecord * below(8);
        cpu.edx = below(8);
        cpu.ebx = below(8);
        cpu.ecx = below(8);
    }
    if (name == "fn4a0980")
    {
        cpu.eax = kConvPool + kConvRecord * below(8);
        cpu.edx = below(8);
        cpu.ebx = below(8);
        cpu.ecx = below(8);
    }
    if (name == "fn4a0a10")
    {
        cpu.eax = kConvPool + kConvRecord * below(8);
        cpu.edx = below(8);
        cpu.ebx = below(8);
        cpu.ecx = below(8);
    }
    if (name == "fn4a0d20")
    {
        cpu.eax = kConvPool + kConvRecord * below(8);
        cpu.edx = below(8);
        cpu.ebx = below(8);
        cpu.ecx = below(8);
        a[0] = below(8);
        a[1] = aConvFloat();
        a[2] = aConvFloat();
        a[3] = below(8);
    }
    if (name == "fn4a0f30")
    {
        cpu.eax = kConvPool + kConvRecord * below(8);
        cpu.edx = kConvPool + kConvRecord * below(8);
        cpu.ebx = below(8);
        cpu.ecx = below(8);
        a[0] = aConvFloat();
    }
    if (name == "fn4a1090")
    {
        cpu.eax = kConvPool + kConvRecord * below(8);
        cpu.edx = below(8);
        cpu.ebx = below(8);
        cpu.ecx = below(8);
    }
    if (name == "fn4a20c0")
    {
        cpu.eax = kConvPool + kConvRecord * below(8);
        cpu.edx = below(8);
        cpu.ebx = below(8);
        cpu.ecx = below(8);
    }
    if (name == "fn4a2630")
    {
        cpu.eax = kConvPool + kConvRecord * below(8);
        cpu.edx = below(8);
        cpu.ebx = below(8);
        cpu.ecx = below(8);
    }
    if (name == "fn4a2830")
    {
        cpu.eax = kConvPool + kConvRecord * below(8);
        cpu.edx = below(8);
        cpu.ebx = below(8);
        cpu.ecx = below(8);
        g.word(0x5efdd8) = aConvFloat();
        g.word(0x7d3684) = below(8);
    }
    if (name == "fn4a2c60")
    {
        cpu.eax = kConvPool + kConvRecord * below(8);
        cpu.edx = below(8);
        cpu.ebx = below(8);
        cpu.ecx = below(8);
    }
    if (name == "fn4a2e20")
    {
        cpu.eax = kConvPool + kConvRecord * below(8);
        cpu.edx = below(8);
        cpu.ebx = below(8);
        cpu.ecx = below(8);
        g.word(0x7d3684) = below(8);
    }
    // @@WORLDS@@
    return true;
}
#endif
