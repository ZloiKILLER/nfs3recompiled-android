"""Force feedback stays with player one when nothing else claims it; never patch input EXEs.

The game plays its force-feedback effects for one car only, and sub_4710f0
chooses it at the start of every race (from sub_4711e0, which then hands the
car to the per-tick update, sub_471e10).  The choice is made from the controls:
the car is the one whose table binds an axis (record type 1) on the
force-feedback device, the pad at [0x678b50] -- player one or two in split
screen, the local player [0x6fd4f8] otherwise.  When no axis is on that pad, no
car is chosen, [0x678b54] stays 0 and no effect is ever started.

That was harmless while the on-screen controls reached the game through the
first pad's axes.  They no longer do: a race driven on them has to be one the
game can replay, so the launcher binds the four driving functions to the keys
the buttons really send (ControlProfile.Kind.TOUCH), and from that race on the
phone had nothing to play -- no jolts, no engine, no road, and not the port's
own cornering either, which reads the same car (phoneTick).

So the scan now starts from the player who would have had the pad -- player
one in split screen, the local player in a single race -- instead of from
nobody.  An axis on the pad still decides as before, the second player's
included; a race with no force-feedback device still gets no car, because that
path leaves before either scan.  The bindings, and with them the replays, are
left exactly as they are.
"""
from pathlib import Path

# (generated file, address and bytes of the instruction, original code, patched
# code).  Each instruction is the first after the table lookup (sub_4904d0),
# which is where the original's -1 would otherwise still be standing.
SITES = [
    # The split-screen scan over both players' tables.
    ("nfs3hp.18.cpp", "00471128  89c3",
     "    cpu.ebx = cpu.eax;",
     "    cpu.ebx = cpu.eax;\n"
     "    cpu.edx = 0; /* port: player one, unless an axis on the pad says otherwise */"),
    # The single-player scan over the local player's table.
    ("nfs3hp.18.cpp", "00471177  89c6",
     "    cpu.esi = cpu.eax;",
     "    cpu.esi = cpu.eax;\n"
     "    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7329016) /* 0x6fd4f8 */);"
     " /* port: the local player, unless an axis on the pad says otherwise */"),
]


def apply(root):
    for name in dict.fromkeys(site[0] for site in SITES):
        path = root / "src/nfs3hp/disassembly" / name
        with path.open("r", newline="") as source:
            text = source.read()
        eol = "\r\n" if text.count("\r\n") * 2 > text.count("\n") else "\n"

        def native(s):
            return eol.join(s.split("\n"))

        for _, instruction, old, new in (site for site in SITES if site[0] == name):
            old, new = native(old), native(new)
            anchor = "    // " + instruction + "  "
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

        with path.open("w", newline="") as out:
            out.write(text)


if __name__ == "__main__":
    apply(Path(__file__).resolve().parents[1])
