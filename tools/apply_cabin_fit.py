"""The in-car cabin keeps its shape on a wide screen; never patch input EXEs.

In the in-car view sub_489240 lays the car's cabin -- dashboard, pillars and
wheel -- over the 3D view as one picture, and has the wheel drawn over it in two
layers that turn with the steering, sub_4880b0 and sub_488470.  All three size
and place themselves from the view's rectangle, [0x7cdb98] x, [0x7cdb9c] y,
[0x7cdba0] width and [0x7cdba4] height -- the picture spans it, the wheel is a
fraction of it -- and the picture is drawn for a 4:3 view.  On a wider one the
dashboard came out stretched and the wheel an oval, a third larger across.

Those seventeen reads, the same ones the Modern Patch sends to a copy of that
rectangle of its own, go through nfs3hp::cabinRect instead
(src/nfs3hp/nfs3hp_main.cpp): the view's own width, and the height that width
has on a 4:3 screen, centred on the view from top to bottom.  The dashboard and
the wheel keep their shape and span the view, and the picture runs off the top
and the bottom alike.  A screen no wider than 4:3 gets the view's rectangle back
unchanged.
Nothing else reads it here: sub_4be3d0 sets the view up from the same record and
keeps doing so.
"""
from pathlib import Path

HEADER = "// Port (tools/apply_cabin_fit.py): defined in nfs3hp_main.cpp.\n"

FIELDS = {
    "0x7cdb98": (8182680, 0),  # x
    "0x7cdb9c": (8182684, 1),  # y
    "0x7cdba0": (8182688, 2),  # width
    "0x7cdba4": (8182692, 3),  # height
}

# (address and bytes of the instruction, the field it reads)
READS = [
    ("004880c7  db05a0db7c00", "0x7cdba0"),
    ("0048818d  db0598db7c00", "0x7cdb98"),
    ("004881a4  db059cdb7c00", "0x7cdb9c"),
    ("0048832f  db0598db7c00", "0x7cdb98"),
    ("00488346  db059cdb7c00", "0x7cdb9c"),
    ("00488486  db05a0db7c00", "0x7cdba0"),
    ("00488492  db05a4db7c00", "0x7cdba4"),
    ("0048849e  a198db7c00", "0x7cdb98"),
    ("004884ad  a19cdb7c00", "0x7cdb9c"),
    ("0048929f  db05a0db7c00", "0x7cdba0"),
    ("004892a7  db05a4db7c00", "0x7cdba4"),
    ("004892bd  8b3d98db7c00", "0x7cdb98"),
    ("004892c3  a1a0db7c00", "0x7cdba0"),
    ("004892cf  8b359cdb7c00", "0x7cdb9c"),
    ("004892d8  a1a4db7c00", "0x7cdba4"),
    ("004894fe  db05a0db7c00", "0x7cdba0"),
    ("0048950a  db05a4db7c00", "0x7cdba4"),
]

DECLARATION = "x86::reg32 cabinRect(win32::WinApplication* app, x86::reg32 field);\n"

NAMESPACE = "namespace nfs3hp\n{\n"
NAME = "nfs3hp.20.cpp"


def apply(root):
    path = root / "src/nfs3hp/disassembly" / NAME
    text = path.read_text(newline="")
    eol = "\r\n" if text.count("\r\n") * 2 > text.count("\n") else "\n"

    def native(s):
        return eol.join(s.split("\n"))

    declarations = native(HEADER + DECLARATION)
    if declarations not in text:
        namespace = native(NAMESPACE)
        if text.count(namespace) != 1:
            raise RuntimeError("%s: namespace opening changed" % NAME)
        text = text.replace(namespace, namespace + declarations, 1)

    for instruction, field in READS:
        decimal, index = FIELDS[field]
        old = "app->getMemory<x86::reg32>(x86::reg32(%d) /* %s */)" % (decimal, field)
        new = "cabinRect(app, %d) /* port: the cabin fitted to 4:3, was [%s] */" % (index, field)
        anchor = "    // " + instruction
        start = text.find(anchor)
        if start < 0:
            raise RuntimeError("%s: no instruction %s" % (NAME, instruction))
        # Everything this one instruction generated: from its own address
        # comment up to the next one.
        stop = text.find(eol + "    // 00", start + len(anchor))
        if stop < 0:
            raise RuntimeError("%s: runaway block at %s" % (NAME, instruction))
        block = text[start:stop]
        if new in block:
            continue
        if block.count(old) != 1:
            raise RuntimeError("%s: unexpected shape at %s" % (NAME, instruction))
        text = text[:start] + block.replace(old, new, 1) + text[stop:]

    path.write_text(text, newline="")


if __name__ == "__main__":
    apply(Path(__file__).resolve().parents[1])
