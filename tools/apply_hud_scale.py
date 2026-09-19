"""The HUD keeps its proportions on a wide screen; never patch input EXEs.

The game sizes parts of its HUD from the size of the screen it draws on, held in
the display record sub_4bef50 fills: [0x7cdac8] the width, [0x7cdacc] the height.
On a 4:3 screen sizing a thing by the width is the same as sizing it by the
height, and the game takes that for granted:

- every HUD element is drawn inside a frame, and the frame's border, its corners
  and the insets of what sits inside it are all the width times 4/640 -- in
  sub_47f650, which draws the frame, and in the functions that lay each kind of
  element out inside one: sub_480fc0, sub_4812e0, sub_481aa0, sub_481c50 and
  sub_487210, plus the gap sub_4800a0 leaves;
- the four text sizes the HUD writes in -- [0x791b3c], [0x791b4c], [0x791b50] and
  [0x791b54], which every HUD caller of sub_4d1390 passes on -- are the width
  times four constants, worked out once a race starts (sub_484b70);
- so are the sizes of the points on the map, the cars and the markers sub_48d4c0
  draws ([0x7930d8] to [0x793104]), in sub_48d110.

Make the screen wider and every one of them grows with it, so a 16:9 race came
out with a third more border, and text a third larger, than it was drawn with.

Each of those reads goes through nfs3hp::hudReferenceWidth instead, and the one
height read among them through nfs3hp::hudReferenceHeight
(src/nfs3hp/nfs3hp_main.cpp): the size a 4:3 screen fitted to this one would
have.  They are the reads the Modern Patch sends to its own 4:3 reference size
(0x7cdae8 and 0x7cdaec), and the map points, which it leaves alone.  The reads
that clip to the real screen -- in sub_4800a0 at 0x48044a and 0x480494, and in
sub_487210 from 0x4872f5 on -- bound the drawing to the picture rather than
sizing anything, and stay as they are.
Where each element sits is untouched: that is sub_480910's business, and
tools/apply_widescreen.py deals with it.
"""
from pathlib import Path

HEADER = "// Port (tools/apply_hud_scale.py): defined in nfs3hp_main.cpp.\n"

WIDTH = "app->getMemory<x86::reg32>(x86::reg32(8182472) /* 0x7cdac8 */)"
HEIGHT = "app->getMemory<x86::reg32>(x86::reg32(8182476) /* 0x7cdacc */)"

FILD_WIDTH = ("    cpu.fpu.push(x86::Float(x86::sreg32(" + WIDTH + ")));",
              "    cpu.fpu.push(x86::Float(x86::sreg32(hudReferenceWidth(app))));"
              " /* port: 4:3 proportions, was [0x7cdac8] */")
FILD_HEIGHT = ("    cpu.fpu.push(x86::Float(x86::sreg32(" + HEIGHT + ")));",
               "    cpu.fpu.push(x86::Float(x86::sreg32(hudReferenceHeight(app))));"
               " /* port: 4:3 proportions, was [0x7cdacc] */")
# sub_484b70 loads the width into eax, compares it against 640 and scales the
# four text sizes by it.
MOV_WIDTH = ("    cpu.eax = " + WIDTH + ";",
             "    cpu.eax = hudReferenceWidth(app); /* port: HUD text sized for 4:3, was [0x7cdac8] */")

READS = [
    # The frame every element is drawn in.
    ("0047f65a", FILD_WIDTH), ("0047f673", FILD_WIDTH), ("0047f6a8", FILD_WIDTH),
    ("0047f7dd", FILD_WIDTH), ("0047f80e", FILD_WIDTH), ("0047f848", FILD_WIDTH),
    ("0047f934", FILD_WIDTH), ("0047f97e", FILD_WIDTH), ("0047f9b8", FILD_WIDTH),
    ("0047fa6b", FILD_WIDTH), ("0047fa84", FILD_WIDTH), ("0047faef", FILD_WIDTH),
    # The gap beside an element.
    ("00480187", FILD_WIDTH),
    # The layouts inside a frame.
    ("00480fcd", FILD_WIDTH), ("00480ffb", FILD_WIDTH), ("0048101a", FILD_WIDTH),
    ("00481038", FILD_WIDTH), ("00481072", FILD_WIDTH), ("00481098", FILD_WIDTH),
    ("004810c0", FILD_WIDTH), ("004810ea", FILD_WIDTH),
    ("00481322", FILD_WIDTH), ("00481349", FILD_WIDTH), ("0048136e", FILD_WIDTH),
    ("0048138f", FILD_WIDTH), ("004813d0", FILD_WIDTH), ("00481406", FILD_WIDTH),
    ("00481434", FILD_WIDTH), ("00481476", FILD_WIDTH),
    ("00481ae5", FILD_WIDTH), ("00481afe", FILD_WIDTH), ("00481b1d", FILD_WIDTH),
    ("00481b3b", FILD_WIDTH), ("00481b72", FILD_WIDTH), ("00481b97", FILD_WIDTH),
    ("00481bbf", FILD_WIDTH), ("00481be9", FILD_WIDTH),
    ("00481d0c", FILD_WIDTH), ("00481d44", FILD_WIDTH), ("00481d7d", FILD_HEIGHT),
    ("00481de6", FILD_WIDTH), ("00481e1e", FILD_WIDTH), ("00481e90", FILD_WIDTH),
    ("00481eb5", FILD_WIDTH), ("00481ef3", FILD_WIDTH), ("00481f3b", FILD_WIDTH),
    ("00487238", FILD_WIDTH), ("00487251", FILD_WIDTH), ("00487284", FILD_WIDTH),
    ("004872b3", FILD_WIDTH),
    # The text sizes.
    ("00484b7a", MOV_WIDTH),
    # The sizes of the points on the map, worked out as a race starts.
    ("0048d40a", FILD_WIDTH),
]

DECLARATIONS = ("x86::reg32 hudReferenceWidth(win32::WinApplication* app);\n"
                "x86::reg32 hudReferenceHeight(win32::WinApplication* app);\n")

NAMESPACE = "namespace nfs3hp\n{\n"


def file_of(address):
    # The frame drawer, sub_47f650, sits in one generated file; the rest in the next.
    return "nfs3hp.19.cpp" if int(address, 16) < 0x480000 else "nfs3hp.20.cpp"


def apply(root):
    for name in dict.fromkeys(file_of(address) for address, _ in READS):
        apply_file(root, name, [read for read in READS if file_of(read[0]) == name])


def apply_file(root, name, reads):
    path = root / "src/nfs3hp/disassembly" / name
    text = path.read_text(newline="")
    eol = "\r\n" if text.count("\r\n") * 2 > text.count("\n") else "\n"

    def native(s):
        return eol.join(s.split("\n"))

    declarations = native(HEADER + DECLARATIONS)
    if declarations not in text:
        # An earlier version of this tool declared the width alone.
        older = native(HEADER + DECLARATIONS.split("\n")[0] + "\n")
        if older in text:
            text = text.replace(older, declarations, 1)
        else:
            namespace = native(NAMESPACE)
            if text.count(namespace) != 1:
                raise RuntimeError("%s: namespace opening changed" % name)
            text = text.replace(namespace, namespace + declarations, 1)

    for address, (old, new) in reads:
        old, new = native(old), native(new)
        anchor = eol + "    // " + address + "  "
        start = text.find(anchor)
        if start < 0:
            raise RuntimeError("%s: no instruction %s" % (name, address))
        start += len(eol)
        # Everything this one instruction generated: from its own address
        # comment up to the next one.
        stop = text.find(eol + "    // 00", start + len(anchor))
        if stop < 0:
            raise RuntimeError("%s: runaway block at %s" % (name, address))
        block = text[start:stop]
        if new in block:
            continue
        if block.count(old) != 1:
            raise RuntimeError("%s: unexpected shape at %s" % (name, address))
        text = text[:start] + block.replace(old, new, 1) + text[stop:]

    path.write_text(text, newline="")


if __name__ == "__main__":
    apply(Path(__file__).resolve().parents[1])
