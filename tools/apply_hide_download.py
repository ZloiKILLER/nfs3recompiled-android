"""Download Car taken off the car screens; never patch input EXEs.

Both car screens -- MENUS/CAR1.MNU and CAR2.MNU, player one's and split screen's
player two's -- list a Download Car tab (codelink CAR_DOWNLOAD) that fetched new
cars from the game's website.  There is nothing left to download, so the tab
only ever led to an error.  The screens are set up by sub_426d60, which looks up
each item by its codelink (sub_442a40) and gives CAR_DOWNLOAD its click handler
(sub_452e60) at +0x64; instead the item is hidden the way the game hides one of
its own -- `hidethis`, a few instructions further on (0x426e12), sets bit 0x10
of the flags byte at +5 -- so it is neither drawn nor reached by the keys.  It
is the last tab in its column, so nothing below it moves up into a gap.  The
menu files themselves come with the player's game data and stay as they are.
"""
from pathlib import Path

# (generated file, address and bytes of the instruction, original code, patched code)
SITES = [
    # sub_426d60: CAR_DOWNLOAD found; hidden rather than given its handler.
    ("nfs3hp.6.cpp", "00426dfb  c74064602e4500",
     "    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(100) /* 0x64 */) = 4533856 /*0x452e60*/;",
     "    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(16 /*0x10*/); "
     "/* port: Download Car hidden, as hidethis is (tools/apply_hide_download.py) */"),
]


def apply(root):
    for name in dict.fromkeys(site[0] for site in SITES):
        path = root / "src/nfs3hp/disassembly" / name
        text = path.read_text(newline="")
        eol = "\r\n" if text.count("\r\n") * 2 > text.count("\n") else "\n"

        def native(s):
            return eol.join(s.split("\n"))

        for _, instruction, old, new in [site for site in SITES if site[0] == name]:
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
