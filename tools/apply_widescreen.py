"""Widescreen races: list a 16:9 screen size, and fit the picture and HUD to it.

- Screen Size.  sub_4bed90 walks the thrash driver's mode table and hands the
  Screen Size list only the modes the game can use: at least two colour
  buffers, 15 or 16 bits per pixel, marked available and not flagged, within
  the driver's memory -- and 4:3 exactly, three times the width equal to four
  times the height (the `cmp edx, eax` at 0x4bedf6).  The jump that test
  decides (0x4bedf8) now also stays put when nfs3hp::widescreenMode accepts the
  size, so every other test still applies to a widescreen mode exactly as it
  does to a 4:3 one.  Which widescreen mode there is to list is the driver
  table's business, set up in main().
- Field of view.  Every camera -- chase and in-car, the mirror, each half of
  split screen -- sets up its projection in sub_4dbce0 from one horizontal half
  angle, and makes the vertical one 13/16 of it: proportions that hold on a 4:3
  screen only, so a race on a wider one came out stretched sideways.  Once the
  vertical angle is stored (0x4dbd1b), the horizontal one goes through
  nfs3hp::widescreenHalfAngle, which opens it by as much as the screen is wider
  than 4:3 (Hor+) and leaves 4:3 exactly as it was.  The float projection
  (sub_4bf260) and the game's older integer one (sub_4fd3f0) both take it from
  there, and nothing reads the angle before that point.
- HUD.  sub_480910 turns an element's layout, a fraction of the screen, into
  the pixel rectangle the HUD is drawn in.  After it stores the last edge
  (0x4809eb), nfs3hp::widescreenHudRect gives the elements drawn as pictures
  back their shape on a wide screen.  The layout itself, which config.dat
  keeps, is never touched.
"""
from pathlib import Path

HEADER = "// Port (tools/apply_widescreen.py): defined in nfs3hp_main.cpp.\n"

# (generated file, address and bytes of the instruction, original code, patched
# code, the declaration the patched code needs)
SITES = [
    ("nfs3hp.27.cpp", "004bedf8  0f8565000000",
     "    if (!cpu.flags.zf)",
     "    if (!cpu.flags.zf && !widescreenMode(cpu.edi, cpu.ebx)) /* port: 16:9 listed as well as 4:3 */",
     "bool widescreenMode(x86::reg32 width, x86::reg32 height);\n"),
    ("nfs3hp.31.cpp", "004dbd1b  8945e8",
     "    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = cpu.eax;",
     "    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = cpu.eax;\n"
     "    cpu.esi = widescreenHalfAngle(app, cpu.esi); /* port: Hor+ on a wide screen, the vertical angle as it was */",
     "x86::reg32 widescreenHalfAngle(win32::WinApplication* app, x86::reg32 half);\n"),
    ("nfs3hp.20.cpp", "004809eb  898264977400",
     "    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7640932) /* 0x749764 */) = cpu.eax;",
     "    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7640932) /* 0x749764 */) = cpu.eax;\n"
     "    widescreenHudRect(app, cpu.edx); /* port: pictures in the HUD keep their shape on a wide screen */",
     "void widescreenHudRect(win32::WinApplication* app, x86::reg32 slot);\n"),
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
        declarations = native(HEADER + "".join(site[4] for site in sites))
        if declarations not in text:
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
