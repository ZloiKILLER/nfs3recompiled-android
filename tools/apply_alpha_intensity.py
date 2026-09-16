"""Make the alpha intensity slider change the picture on this driver.

apply_menu_unlock.py lets the Advanced Graphics slider be moved: it binds to
the float at 0x6fbc38, 1.0 until the player lowers it.  What that value does is
decided at draw time, in thirteen places that build the colours of the game's
see-through and glowing effects.  Each packs a colour as ARGB and, while the
value is below 1, multiplies all four channels by it (value x 65536, clamped
to 0xffff, >> 8, one channel pair at a time).  Each also does so only when the
byte at 0x7a3a58 has bit 0x40 set -- and that byte is not a capability mask but
1 << the driver type (sub_4b6020), so 0x40 names type 6, the Direct3D driver.
On this Glide driver every one of them skipped the scaling, which is why the
slider moved and nothing on screen changed.

The scaled colours reach the Glide layer as vertex colour and alpha, which the
port's shader always applies, textured or not (gliderenderer.cpp), so the
game's own scaling shows the way it did under Direct3D.  At the default of 1
every site still skips the scaling, exactly as before.

Each site is matched by its own address.  The same probe also picks the
renderer's entry points (sub_434870), compares the driver's name (sub_4a4410),
fills a colour table only Direct3D uses (sub_4cb670) and chooses this value's
default for one card (sub_472d10); none of those is touched.  The two tests
that grey the slider out in the menus are apply_menu_unlock.py's.
"""
from pathlib import Path

# (generated file, address, instruction bytes, register or None for the memory
# probe) of each `test ..., 0x40` that decides whether the scaling runs.
SITES = [
    ("nfs3hp.4.cpp", "0041a1c6", "f6c440", "ah"),           # sub_41a120
    ("nfs3hp.4.cpp", "0041b9cd", "f6c440", "ah"),           # sub_41b9b0
    ("nfs3hp.19.cpp", "0047b8f4", "f605583a7a0040", None),  # sub_47b850, via sub_47b7d0
    ("nfs3hp.21.cpp", "00491223", "f6c440", "ah"),          # sub_491190
    ("nfs3hp.21.cpp", "00492202", "f605583a7a0040", None),  # sub_491bc0
    ("nfs3hp.21.cpp", "0049259c", "f605583a7a0040", None),  # sub_492370
    ("nfs3hp.21.cpp", "00492c0f", "f605583a7a0040", None),  # sub_492980
    ("nfs3hp.23.cpp", "0049d80e", "f605583a7a0040", None),  # sub_49d800
    ("nfs3hp.28.cpp", "004cb86e", "f6c440", "ah"),          # sub_4cb750
    ("nfs3hp.28.cpp", "004cbf2f", "f6c340", "bl"),          # sub_4cbc90
    ("nfs3hp.31.cpp", "004ddd57", "f6c440", "ah"),          # sub_4ddb40
    ("nfs3hp.31.cpp", "004de0bc", "f6c440", "ah"),          # sub_4de070
    ("nfs3hp.31.cpp", "004de76a", "f605583a7a0040", None),  # sub_4de700
]

MEMORY_TEST = ("    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>"
               "(x86::reg32(8010328) /* 0x7a3a58 */) & 64 /*0x40*/));")
REGISTER_TEST = "    cpu.set_szp(static_cast<x86::reg8>(cpu.%s & 64 /*0x40*/));"
FORCED = ("    // Port: alpha intensity applies on this driver too (tools/apply_alpha_intensity.py).\n"
          "    cpu.set_szp(static_cast<x86::reg8>(64 /*0x40, port: was %s*/));")


def apply(root):
    for name, address, code, register in SITES:
        path = root / "src/nfs3hp/disassembly" / name
        text = path.read_text(newline="")
        eol = "\r\n" if text.count("\r\n") * 2 > text.count("\n") else "\n"
        test = MEMORY_TEST if register is None else REGISTER_TEST % register
        was = "the 0x7a3a58 probe" if register is None else "%s & 0x40, %s read from 0x7a3a58" % (register, register)
        forced = eol.join((FORCED % was).split("\n"))

        anchor = "    // %s  %s" % (address, code)
        start = text.find(anchor)
        if start < 0:
            raise RuntimeError("%s: no driver probe at %s" % (name, address))
        # Everything this one instruction generated: from its own address
        # comment up to the next one.
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
