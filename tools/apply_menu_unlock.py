"""Unlock the Advanced Graphics items the original hides on this driver.

Each Advanced Graphics screen has its own menu handler, registered in the
name -> handler table at 0x555c48 (57 entries, "enter/frame/exit" apiece):
sub_448610 serves `advgrph`, the screen reached from the main menu, and
sub_44d2a0 serves `psgraph`, the one reached from the pause menu.  Both walk
the same list of codelinks and set bit 0 of an item's flags at +4, which is
what greys an entry out.

SFX_INTENSITY -- alpha intensity -- is disabled in both unless bit 0x40 of the
byte at 0x7a3a58 is set: 1 << the driver type, and 0x40 is the Direct3D
driver's.  The enabled path binds the slider
to the float at 0x6fbc38, which is why alpha intensity has no listname entry in
the settings table: code binds it directly.  Making the test report the bit
present takes the game's own branch, with no control flow rewritten and no
unreachable code left behind.

Only these two tests are touched here.  The same probe decides, at draw time,
whether the value is applied at all -- those thirteen sites are
tools/apply_alpha_intensity.py's -- and the rest gate Direct3D-only things that
must stay off, so each site is matched by its own address rather than by the
shape of the generated line.

GRAPH_PERSP -- perspective correction -- was unlocked here too and has been
dropped again: the setting picks a rasterisation mode, and this port has
exactly one rasteriser.  OpenGL interpolates perspective-correct by
construction, so there was nothing behind the menu entry and it did nothing
when pressed.  Both handlers grey it unconditionally, with no capability test
of their own, which is why it needed a separate patch and not this one.
"""
from pathlib import Path

# (generated file, address of the `test byte ptr [0x7a3a58], 0x40` it guards)
SITES = [
    ("nfs3hp.12.cpp", "00448738"),   # sub_448610 -- advgrph, from Options
    ("nfs3hp.13.cpp", "0044d36d"),   # sub_44d2a0 -- psgraph, from the pause menu
]

CAP_TEST = ("    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>"
            "(x86::reg32(8010328) /* 0x7a3a58 */) & 64 /*0x40*/));")
CAP_FORCED = ("    // Port: alpha intensity capability reported present.\n"
              "    cpu.set_szp(static_cast<x86::reg8>(64 /*0x40, port: was the"
              " 0x7a3a58 probe*/));")


def apply(root):
    for name, address in SITES:
        path = root / "src/nfs3hp/disassembly" / name
        text = path.read_text(newline="")
        eol = "\r\n" if text.count("\r\n") * 2 > text.count("\n") else "\n"
        forced = eol.join(CAP_FORCED.split("\n"))
        test = eol.join(CAP_TEST.split("\n"))

        anchor = "    // %s  f605583a7a0040" % address
        start = text.find(anchor)
        if start < 0:
            raise RuntimeError("%s: no capability test at %s" % (name, address))
        # Everything this one instruction generated: from its own address
        # comment up to the next one.  Counting lines would not do -- the
        # patched form is a line longer than the original.
        stop = text.find(eol + "    // 00", start + len(anchor))
        if stop < 0:
            raise RuntimeError("%s: runaway block at %s" % (name, address))
        block = text[start:stop]
        if forced in block:
            continue
        if block.count(test) != 1:
            raise RuntimeError("%s: unexpected shape at %s" % (name, address))
        text = text[:start] + block.replace(test, forced, 1) + text[stop:]
        path.write_text(text, newline="")


if __name__ == "__main__":
    apply(Path(__file__).resolve().parents[1])
