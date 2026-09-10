"""Unlock the Advanced Graphics items the original hides on this driver.

sub_448610 is the menu's codelink resolver: for each name it looks the item up
and sets bit 0 of the item's flags at +4, which is what greys an entry out.

SFX_INTENSITY -- alpha intensity -- is disabled there unless bit 0x40 of the
driver capability byte at 0x7a3a58 is set.  The enabled path binds the slider to
the float at 0x6fbc38, which is why alpha intensity has no listname entry in the
settings table: code binds it directly.  Making the test report the bit present
takes the game's own branch, with no control flow rewritten and no unreachable
code left behind.

GRAPH_PERSP -- perspective correction -- was unlocked here too and has been
dropped again: the setting picks a rasterisation mode, and this port has exactly
one rasteriser.  OpenGL interpolates perspective-correct by construction, so
there was nothing behind the menu entry and it did nothing when pressed.
"""
from pathlib import Path

FILE = "nfs3hp.12.cpp"

CAP_TEST = ("    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>"
            "(x86::reg32(8010328) /* 0x7a3a58 */) & 64 /*0x40*/));")
CAP_FORCED = ("    // Port: alpha intensity capability reported present.\n"
              "    cpu.set_szp(static_cast<x86::reg8>(64 /*0x40, port: was the"
              " 0x7a3a58 probe*/));")


def apply(root):
    path = root / "src/nfs3hp/disassembly" / FILE
    text = path.read_text(newline="")
    eol = "\r\n" if text.count("\r\n") * 2 > text.count("\n") else "\n"

    def line(s):
        return eol.join(s.split("\n"))

    if line(CAP_FORCED) not in text:
        if text.count(line(CAP_TEST)) != 1:
            raise RuntimeError("Expected exactly one SFX_INTENSITY capability test")
        text = text.replace(line(CAP_TEST), line(CAP_FORCED), 1)

    path.write_text(text, newline="")


if __name__ == "__main__":
    apply(Path(__file__).resolve().parents[1])
