"""View Distance Full reaches the whole distance in every view; never patch input EXEs.

sub_41d620 works out the distances the world is drawn at.  Each view keeps its
own set -- the four records at 0x5dd0e0, 84 bytes each -- of a near clip, a far
one and two in between, and the world drops to less detail at the two in
between.  That drop, running up the road ahead of the car, is the detail change
a player sees.

View Distance ([0x6fbc28], Full, Far, Medium, Close = 0 to 3) is the player's
own choice in the Graphics menu and is left to them.  At Full the game already
makes both middle distances equal to the far one (0x41d7b5), so nothing
switches between the car and the horizon, but two things still held the far
distance short of its 500:

- [0x6fd4c8] picks the table of multipliers: 0x41a8bc, whose widest row leaves
  the far distance whole, or 0x41a8b0, whose widest row keeps three quarters of
  it.  At Full the whole one is taken (nfs3hp::viewDistanceReduced).
- Split screen starts from 440 instead of 500 (0x41d719), and its views cull
  every polygon beyond 400 (+0x24, set once for the view by sub_41df10).  At
  Full both halves reach 500 (nfs3hp::splitFarDistance), and the cull distance
  is 500 at every setting, as it is for a single view, where the far distance
  sets the reach.

Below Full the game's own distances stand, and the setting still picks them.
sub_4dbbf0 reapplies the distances as the race runs and the pause menu's View
Distance item calls sub_41d620 again, so a change takes effect at once.

Full is also what the game starts from.  It never picked it itself: the
settings it falls back to write Far (0x47266c), and the profile sub_472d10
chooses by the speed it measures the CPU at picks Close to Far from the table at
0x472168 (0x472e01) -- a measure of a 1999 PC that says nothing about a phone.
The launcher writes Full into config.dat once, as it imports the game's data;
these two keep it Full should the game ever start its settings over.
"""
from pathlib import Path

HEADER = "// Port (tools/apply_track_detail.py): defined in nfs3hp_main.cpp.\n"

# (generated file, address and bytes of the instruction, original code, patched
# code, the declaration the patched code needs)
SITES = [
    ("nfs3hp.4.cpp", "0041d637  8b15c8d46f00",
     "    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7328968) /* 0x6fd4c8 */);",
     "    cpu.edx = viewDistanceReduced(app, app->getMemory<x86::reg32>(x86::reg32(7328968) /* 0x6fd4c8 */));"
     " /* port: Full leaves the far distance whole */",
     "x86::reg32 viewDistanceReduced(win32::WinApplication* app, x86::reg32 reduced);\n"),
    ("nfs3hp.4.cpp", "0041d719  b80000b801",
     "    cpu.eax = 28835840 /*0x1b80000*/;",
     "    cpu.eax = splitFarDistance(app, 28835840 /*0x1b80000*/); /* port: Full reaches 500 in split screen too */",
     "x86::reg32 splitFarDistance(win32::WinApplication* app, x86::reg32 distance);\n"),
    ("nfs3hp.4.cpp", "0041df99  c7402400009001",
     "    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(36) /* 0x24 */) = 26214400 /*0x1900000*/;",
     "    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(36) /* 0x24 */) = 32768000 /*0x1f40000*/;"
     " /* port: split screen culls at 500, as a single view does */",
     ""),
    ("nfs3hp.18.cpp", "0047266c  893d28bc6f00",
     "    app->getMemory<x86::reg32>(x86::reg32(7322664) /* 0x6fbc28 */) = cpu.edi;",
     "    app->getMemory<x86::reg32>(x86::reg32(7322664) /* 0x6fbc28 */) = 0; /* port: View Distance Full, was Far */",
     ""),
    ("nfs3hp.18.cpp", "00472e01  891528bc6f00",
     "    app->getMemory<x86::reg32>(x86::reg32(7322664) /* 0x6fbc28 */) = cpu.edx;",
     "    app->getMemory<x86::reg32>(x86::reg32(7322664) /* 0x6fbc28 */) = 0; /* port: View Distance Full at any CPU speed */",
     ""),
]

NAMESPACE = "namespace nfs3hp\n{\n"


def apply(root):
    for name in dict.fromkeys(site[0] for site in SITES):
        path = root / "src/nfs3hp/disassembly" / name
        text = path.read_text(newline="")
        eol = "\r\n" if text.count("\r\n") * 2 > text.count("\n") else "\n"

        def native(s):
            return eol.join(s.split("\n"))

        sites = [site for site in SITES if site[0] == name]
        needed = "".join(site[4] for site in sites)
        declarations = native(HEADER + needed)
        if needed and declarations not in text:
            namespace = native(NAMESPACE)
            if text.count(namespace) != 1:
                raise RuntimeError("%s: namespace opening changed" % name)
            text = text.replace(namespace, namespace + declarations, 1)

        for _, instruction, old, new, _ in sites:
            old, new = native(old), native(new)
            anchor = "    // " + instruction
            start = text.find(anchor)
            if start < 0:
                raise RuntimeError("%s: no instruction %s" % (name, instruction))
            # Everything this one instruction generated: from its own address
            # comment up to the next one.
            stop = text.find(eol + "    // 00", start + len(anchor))
            if stop < 0:
                raise RuntimeError("%s: runaway block at %s" % (name, instruction))
            block = text[start:stop]
            if new in block:
                continue
            if block.count(old) != 1:
                raise RuntimeError("%s: unexpected shape at %s" % (name, instruction))
            text = text[:start] + block.replace(old, new, 1) + text[stop:]

        path.write_text(text, newline="")


if __name__ == "__main__":
    apply(Path(__file__).resolve().parents[1])
