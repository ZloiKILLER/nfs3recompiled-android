// A case's registers and stack for these functions, after caseEntry: statements on `entry`, by c.address.
// Where the selected look's own words are, for the arguments that may point into it.
const x86::reg32 look = 0x55b08c + 0x238 * word32(0x55b088);
switch (c.address)
{
case 0x477170:
    // The direction: the game's vector, or the look itself, which the copy then overlaps.
    entry.edx = random(4) == 0 ? look + 0x1c + 4 * random(3) : 0x55d448;
    break;
case 0x4771c0:
    // Likewise the light's position, somewhere among the lights.
    entry.edx = random(4) == 0 ? look + 0x30 + 0x40 * random(8) + 4 * random(16) : 0x55d448;
    break;
case 0x484260:
case 0x48ccf0:
case 0x485640:
    entry.eax = random(3);
    break;
case 0x48eb60:
    entry.eax = random(2) ? random(2) : x86::reg32(-1);
    break;
case 0x487340:
case 0x4873c0:
    entry.eax = random(8);
    break;
case 0x497590:
    // Floats, some of them anything at all: NaN and infinity come out as the x87's.
    for (x86::reg32 at : { entry.esp + 4, entry.esp + 8 })
    {
        if (random(8) == 0)
            setWord32(at, random(0));
        else
            setFloat(at, someValue(g_random) * float(x86::sreg32(random(2000)) - 1000));
    }
    break;
case 0x49bef0:
    entry.eax = 0x793108;
    break;
case 0x49bf40:
    // Where the first's address goes: the surface's own count, now and then.
    entry.eax = 0x793108;
    entry.edx = random(32);
    entry.ebx = random(6) == 0 ? 0x793108 : 0x5f3e00 + 4 * random(16);
    break;
case 0x4d10e0:
    entry.edx = random(4) == 0 ? random(0) : x86::reg32(x86::sreg32(random(1000)) - 200);
    break;
default:
    break;
}
// What each caller goes on with is what a callee keeps; the rest it loses, to show a register
// a native should have restored or left as it was.  sub_485640 counts on ebx and ecx through
// sub_4844c0; sub_484260, sub_48ccf0 on ecx and edx through sub_422bd0, sub_484260 on edx through
// sub_4844c0.
auto& pane = const_cast<Spec&>(kSpecs.at(0x4844c0));
pane.clobbers = c.address == 0x485640 ? ESI | EDI : EBX | ECX | ESI | EDI;
auto& camera = const_cast<Spec&>(kSpecs.at(0x422bd0));
camera.clobbers = c.address == 0x484450 ? EBX | ECX | EDX | ESI | EDI : EBX | ESI | EDI;
