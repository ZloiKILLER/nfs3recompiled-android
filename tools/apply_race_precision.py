"""Races at the x87's extended precision, as the Modern Patch runs them; never patch input EXEs.

The race loop (sub_4a3690, the state machine behind sub_4a3400) starts each
race over with fninit and then takes the precision control down to single:
fnstcw, `and eax, 0xfffff0ff`, fldcw -- at 0x4a3e8d for one way into a race and
at 0x4a3ed3 for the other.  Every sum, product and quotient of the physics and
the opponents' driving then keeps a float's 24 bits, as it did on a PC of 1998.
The Modern Patch keeps the fninit and drops that fldcw ("FPU always uses
extended precision", since 1.6.0), so on its PCs a race runs at the 64 bits
fninit leaves; include/fpu.h says the same of WITH_WIDE_FPU.

Four more functions step out of the race's word and back into it: they load the
word from before the race (fldcw [0x79f3f4]), call out -- sub_494fa0 and
sub_495230 through the import table, sub_4b6020 to sub_4bf110, sub_4bc680 when
[0x7a3a70] is clear -- and come back with the same fninit, fnstcw, `and`, fldcw
at 0x495179, 0x4955e9, 0x4b6239 and 0x4bc811.  Left alone, the first of them
to run in a race would put it back to single precision.

All six fldcw go through nfs3hp::raceControl (nfs3hp_main.cpp), which hands the
word on as it is or, with extended races on, with the precision control put
back to 64 bits -- the word the Modern Patch's race runs under.  NFS_FPU_EXTENDED
picks: on unless it is 0.  The loads of the word from before the race
(fldcw [0x79f3f4]) are left as they are.

What extended means is the build's: with WITH_PEDANTIC_FPU every value is 80
bits and the arithmetic the x87's to the bit (src/lib/x87soft.cpp); the default
build holds them in doubles, 53 bits.
"""
from pathlib import Path

HEADER = "// Port (tools/apply_race_precision.py): defined in nfs3hp_main.cpp.\n"

DECLARATION = "x86::reg16 raceControl(x86::reg16 word);\n"

LOAD = "cpu.fpu.setControl(app->getMemory<x86::reg16>(x86::reg32(7992300) /* 0x79f3ec */));"
EXTENDED = ("cpu.fpu.setControl(raceControl(app->getMemory<x86::reg16>(x86::reg32(7992300) /* 0x79f3ec */)));"
            " /* port: extended races as the Modern Patch has them */")

# (generated file, address and bytes of the instruction)
SITES = [
    ("nfs3hp.21.cpp", "00495179  d92decf37900"),
    ("nfs3hp.21.cpp", "004955e9  d92decf37900"),
    ("nfs3hp.24.cpp", "004a3e8d  d92decf37900"),
    ("nfs3hp.24.cpp", "004a3ed3  d92decf37900"),
    ("nfs3hp.25.cpp", "004b6239  d92decf37900"),
    ("nfs3hp.26.cpp", "004bc811  d92decf37900"),
]

NAMESPACE = "namespace nfs3hp\n{\n"


def apply(root):
    for name in dict.fromkeys(site[0] for site in SITES):
        path = root / "src/nfs3hp/disassembly" / name
        with open(path, newline="") as f:
            text = f.read()
        eol = "\r\n" if text.count("\r\n") * 2 > text.count("\n") else "\n"

        def native(s):
            return eol.join(s.split("\n"))

        declarations = native(HEADER + DECLARATION)
        if declarations not in text:
            namespace = native(NAMESPACE)
            if text.count(namespace) != 1:
                raise RuntimeError("%s: namespace opening changed" % name)
            text = text.replace(namespace, namespace + declarations, 1)

        for _, instruction in (site for site in SITES if site[0] == name):
            anchor = "    // " + instruction
            start = text.find(anchor)
            if start < 0:
                raise RuntimeError("%s: no instruction %s" % (name, instruction))
            stop = text.find(eol + "    // 00", start + len(anchor))
            if stop < 0:
                raise RuntimeError("%s: runaway block at %s" % (name, instruction))
            block = text[start:stop]
            if EXTENDED in block:
                continue
            if block.count(LOAD) != 1:
                raise RuntimeError("%s: unexpected shape at %s" % (name, instruction))
            text = text[:start] + block.replace(LOAD, EXTENDED, 1) + text[stop:]

        path.write_text(text, newline="")


if __name__ == "__main__":
    apply(Path(__file__).resolve().parents[1])
