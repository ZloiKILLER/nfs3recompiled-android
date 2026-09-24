"""Full-colour textures in a race, and the Screen Size list saying so; never patch input EXEs.

The picture has been full colour for a while: the Glide renderer draws into an
eight-bit-a-channel target and dithers nothing (renderer.cpp, setDither).
What stayed sixteen bits were the textures, and not because the art is: on
the disc the cars are 86% ARGB 8888 by pixel, the HUD all of it, the smoke,
lights and sky of gamedata/render 82% (the tracks are 1555 on the disc).
The game shrank them on their way to the card:

- The Voodoo2 driver tells the game which of THRASH's texture formats it takes
  through the table THRASH_about points at (+0x30, 0xa91fd4, one dword a
  format): 2 (P_8), 3 (1555), 4 (565), 7 (4444), 8 and 9.  Format 6 -- ARGB
  8888, the one the game's Direct3D drivers take -- is not among them, and
  THRASH_talloc turns a format into Glide's through the byte table at 0xa92348,
  where 6 has nothing.
- The game picks the format of every texture it loads in sub_4d37a0: a 32-bit
  picture (FSH record 0x7d) goes to format 6 whenever the driver takes it and
  has the texture memory for it (THRASH_about +0x70, which the probe sets to
  what TMU 0 holds, well above the 2 MB it asks for); otherwise, by its alpha,
  to 4444, 1555 or 565.  The conversion to format 6 is already in the game,
  for Direct3D.

So the driver is made to take format 6 and hand it to Glide as ARGB 8888,
numbered as Glide 3 numbers it for the Voodoo 4 and 5, which the port's Glide
layer keeps as it comes (glide2x.cpp, kTexFmtArgb8888).  THRASH_tupdate and
THRASH_settexture go through Glide's own texture record and care nothing for
the format.  NFS_TEXTURES32=0 leaves the driver as it was.

The Modern Patch's own Voodoo2 driver maps the formats with the same table, so
its 32-bit mode under nGlide or dgVoodoo is a full-colour picture of 16-bit
textures -- the picture this port already had.

The Screen Size list then says what a race is drawn in: its entries read
"640 x 480 x 16" from the mode table (sub_448210, "%d x %d x %d"), and the
table keeps 16 -- the list takes only 15- and 16-bit modes (sub_4bed90), and a
32-bit mode in the table blacked out every race before (apply_glide_output.py).
Only the number written into the entry's text changes.
"""
from pathlib import Path

# (generated file, address and bytes of the instruction, original code, patched code)
SITES = [
    # THRASH_about, filling its struct the first time: right after it points the
    # game at the table of formats it takes.
    ("voodoo2a.0.cpp", "00a83d3c  8915841fa900",
     "    app->getMemory<x86::reg32>(x86::reg32(11083652) /* 0xa91f84 */) = cpu.edx;",
     "    app->getMemory<x86::reg32>(x86::reg32(11083652) /* 0xa91f84 */) = cpu.edx;\n"
     "    if (win32::glide2x::fullColourTextures())\n"
     "    {\n"
     "        /* port: THRASH format 6, ARGB 8888, taken, and handed to Glide as such */\n"
     "        app->getMemory<x86::reg32>(x86::reg32(0xa91fd4 + 6 * 4)) = 1;\n"
     "        app->getMemory<x86::reg8>(x86::reg32(0xa92348 + 6)) = x86::reg8(win32::glide2x::kTexFmtArgb8888);\n"
     "    }"),
    # sub_448210, a Screen Size entry: the depth pushed for "%d x %d x %d".
    ("nfs3hp.12.cpp", "0044823e  51",
     "    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;",
     "    app->getMemory<x86::reg32>(cpu.esp-4) = shownDepth(cpu.ecx); /* port: the depth a race is drawn in */"),
]

# What each file needs before its patched code can compile.
PREAMBLE = {
    "voodoo2a.0.cpp": ("#include \"voodoo2a.h\"\n",
                       "#include <winapi/glide2x.h> // Port (tools/apply_full_colour.py)\n"),
    "nfs3hp.12.cpp": ("namespace nfs3hp\n{\n",
                      "// Port (tools/apply_full_colour.py): defined in nfs3hp_main.cpp.\n"
                      "x86::reg32 shownDepth(x86::reg32 depth);\n"),
}


def apply(root):
    for name in dict.fromkeys(site[0] for site in SITES):
        path = root / "src/nfs3hp/disassembly" / name
        with path.open("r", newline="") as source:
            text = source.read()
        eol = "\r\n" if text.count("\r\n") * 2 > text.count("\n") else "\n"

        def native(s):
            return eol.join(s.split("\n"))

        after, insert = (native(part) for part in PREAMBLE[name])
        if insert not in text:
            if text.count(after) != 1:
                raise RuntimeError("%s: preamble anchor changed" % name)
            text = text.replace(after, after + insert, 1)

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
