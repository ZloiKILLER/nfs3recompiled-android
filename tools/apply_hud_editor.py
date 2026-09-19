"""The HUD editor measures what is drawn; never patch input EXEs.

The game keeps every HUD element's place as fractions of the screen, four floats
per element in config.dat -- top, left, bottom, right -- from 0x6fbc74, 0x348
bytes per player (one player, then split screen's two) and 0x1a4 per layout
within it.  sub_480910 turns them into the pixel rectangles at 0x749758.  The
HUD editor (pause menu, Heads Up Display) frames each element by that
rectangle, moves it, asks sub_459670 whether it may stand there, and writes the
fractions back.  In four ways the rectangle was not what the screen showed:

- The round gauges, the tutorial icon and the replay bar.  On a screen wider
  than 4:3 nfs3hp::widescreenHudRect narrows them about their centre (the spike
  belt indicator it makes taller), so they keep their shape; the layout stays as
  laid out.  The editor took the drawn rectangle and wrote it back -- sub_459db0
  after every step of a drag (from 0x459ec4), sub_459fe0 when the element is let
  go (from 0x45a02e) -- so every edit made a gauge thinner.
  nfs3hp::hudLayoutDesigned widens what is written, for those elements.
- The standings table.  Its layout is a box half the screen wide and a third
  high, and sub_481c50 draws the table as a panel only as big as its rows, at
  the box's left edge and at its top or bottom, whichever the table keeps to.
  The rest of the box is empty, yet it was the frame the editor showed and what
  every other element was measured against.  Once sub_481c50 has placed the
  panel (0x481f86), nfs3hp::hudTablePanel makes the pixel rectangle the panel's:
  the edges the panel is placed by stay where they are, so the table is drawn as
  before, and the editor frames, moves and measures what is on the screen.
- Every other element drawn smaller than its box: a line of text at one side of
  it, an icon in its middle.  sub_45bcb0 draws each element in the editor
  (sub_45ab30, 0x45bcc3) and then its frame (sub_459440, 0x45bd0c).
  nfs3hp::hudEditorDraw measures what the first puts on the screen -- every
  quad it queues meanwhile, text glyph by glyph, which sub_49bd30 is handed
  (nfs3hp::hudQueueQuad) -- and nfs3hp::hudEditorFrame has the frame drawn
  about that instead of the box.  What reaches Glide during an element's turn
  is no measure of it: the queues are drawn later, all together.  A drag
  (sub_45a120) kept the whole box on the screen, so text in the middle of a
  wide box could never come near an edge; its four clamps now let the part of
  the box that draws nothing go past it (nfs3hp::hudDragSlack).
- The test itself.  sub_459670 compares the element being moved, as the editor
  has it, with every other element's fractions, as laid out: wider than the
  gauges on the screen, whole boxes where a line of text is drawn, and to the
  pixel -- an overlap of one pixel was enough, which a finger putting two frames
  edge to edge could hardly avoid, and the element blinked red over empty space.
  For the length of that call (0x459f7a) nfs3hp::hudLayoutsDrawn puts the
  player's layouts, and the element being moved, as they were measured drawn
  and a little inside that, and puts them back exactly afterwards.

A 4:3 screen draws the gauges as laid out, so there the first changes nothing.
"""
from pathlib import Path

HEADER = "// Port (tools/apply_hud_editor.py): defined in nfs3hp_main.cpp.\n"

MOVSD = (
    "    app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);\n"
    "    if (cpu.flags.df)\n"
    "    {\n"
    "        cpu.edi -= 4;\n"
    "        cpu.esi -= 4;\n"
    "    }\n"
    "    else\n"
    "    {\n"
    "        cpu.edi += 4;\n"
    "        cpu.esi += 4;\n"
    "    }"
)

CALL_FITS = (
    "    cpu.esp -= 4;\n"
    "    sub_459670(app, cpu);"
)

# (generated file, address and bytes of the instruction, original code, patched
# code, the declaration the patched code needs)
SITES = [
    # sub_459db0: the last of the four floats copied into the layout; edi is past
    # them, ebx the element.
    ("nfs3hp.16.cpp", "00459ecd  a5", MOVSD,
     MOVSD + "\n    hudLayoutDesigned(app, cpu.edi - 16, cpu.ebx); /* port: the layout as laid out, not as drawn */",
     "void hudLayoutDesigned(win32::WinApplication* app, x86::reg32 rect, x86::reg32 element);\n"),
    # sub_459fe0: likewise, the last good place put back; edx is the element.
    ("nfs3hp.16.cpp", "0045a031  a5", MOVSD,
     MOVSD + "\n    hudLayoutDesigned(app, cpu.edi - 16, cpu.edx); /* port: the layout as laid out, not as drawn */",
     ""),
    # sub_459db0: may the element stand here?  eax is the rectangle tried, edx
    # the player, ebx the element.
    ("nfs3hp.16.cpp", "00459f7a  e8f1f6ffff", CALL_FITS,
     "    cpu.esp -= 4;\n"
     "    hudLayoutsDrawn(app, cpu, true); /* port: measured against what is drawn */\n"
     "    sub_459670(app, cpu);\n"
     "    hudLayoutsDrawn(app, cpu, false);",
     "void hudLayoutsDrawn(win32::WinApplication* app, x86::CPU& cpu, bool drawn);\n"),
    # sub_45a120, a drag keeping the element's box on the screen: the right and
    # bottom limits (the screen less the element's size) and the left and top
    # ones (0), each moved by as much of the box as is not drawn.  [ebp-0xc] is
    # the element's item.
    ("nfs3hp.16.cpp", "0045a581  29d0",
     "    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));",
     "    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));\n"
     "    cpu.eax += x86::reg32(hudDragSlack(app, app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12)), 1));"
     " /* port: what is drawn stays on the screen */",
     "x86::sreg32 hudDragSlack(win32::WinApplication* app, x86::reg32 object, x86::reg32 side);\n"),
    ("nfs3hp.16.cpp", "0045a59c  29d8",
     "    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ebx));",
     "    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ebx));\n"
     "    cpu.eax += x86::reg32(hudDragSlack(app, cpu.edx, 1)); /* port: what is drawn stays on the screen */",
     ""),
    ("nfs3hp.16.cpp", "0045a5a5  6683780600",
     "        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));",
     "        x86::reg16 tmp2 = x86::reg16(x86::sreg16(-hudDragSlack(app, cpu.eax, 0))); /* port: was 0 */",
     ""),
    ("nfs3hp.16.cpp", "0045a5ac  66c740060000",
     "    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(6) /* 0x6 */) = 0 /*0x0*/;",
     "    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(6) /* 0x6 */) = x86::reg16(x86::sreg16(-hudDragSlack(app, cpu.eax, 0)));"
     " /* port: was 0 */",
     ""),
    ("nfs3hp.16.cpp", "0045a5c1  29ca",
     "    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.ecx));",
     "    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.ecx));\n"
     "    cpu.edx += x86::reg32(hudDragSlack(app, app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12)), 3));"
     " /* port: what is drawn stays on the screen */",
     ""),
    ("nfs3hp.16.cpp", "0045a5d6  29f0",
     "    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.esi));",
     "    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.esi));\n"
     "    cpu.eax += x86::reg32(hudDragSlack(app, cpu.edx, 3)); /* port: what is drawn stays on the screen */",
     ""),
    ("nfs3hp.16.cpp", "0045a5df  6683780800",
     "        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));",
     "        x86::reg16 tmp2 = x86::reg16(x86::sreg16(-hudDragSlack(app, cpu.eax, 2))); /* port: was 0 */",
     ""),
    ("nfs3hp.16.cpp", "0045a5e6  66c740080000",
     "    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(8) /* 0x8 */) = 0 /*0x0*/;",
     "    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(8) /* 0x8 */) = x86::reg16(x86::sreg16(-hudDragSlack(app, cpu.eax, 2)));"
     " /* port: was 0 */",
     ""),
    # sub_45bcb0: the element drawn in the editor, ecx the element's item, which
    # sub_45ab30 keeps.
    ("nfs3hp.16.cpp", "0045bcc3  e868eeffff",
     "    cpu.esp -= 4;\n"
     "    sub_45ab30(app, cpu);",
     "    cpu.esp -= 4;\n"
     "    hudEditorDraw(app, cpu.ecx, true); /* port: measure what the element draws */\n"
     "    sub_45ab30(app, cpu);\n"
     "    hudEditorDraw(app, cpu.ecx, false);",
     "void hudEditorDraw(win32::WinApplication* app, x86::reg32 object, bool begin);\n"),
    # sub_45bcb0: the frame about it, eax and ebx the item, which sub_459440
    # keeps in ebx.
    ("nfs3hp.16.cpp", "0045bd0c  e82fd7ffff",
     "    cpu.esp -= 4;\n"
     "    sub_459440(app, cpu);",
     "    cpu.esp -= 4;\n"
     "    hudEditorFrame(app, cpu.eax, true); /* port: the frame about what is drawn */\n"
     "    sub_459440(app, cpu);\n"
     "    hudEditorFrame(app, cpu.ebx, false);",
     "void hudEditorFrame(win32::WinApplication* app, x86::reg32 object, bool drawn);\n"),
    # sub_49bd30, a quad into one of the 2D queues: eax the queue, edx, ebx and
    # ecx three of its vertices and the fourth on the stack above the return.
    ("nfs3hp.22.cpp", "0049bd30  56",
     "    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;",
     "    hudQueueQuad(app, cpu.eax, cpu.edx, cpu.ebx, cpu.ecx, app->getMemory<x86::reg32>(cpu.esp + 4));"
     " /* port: measured for the HUD editor */\n"
     "    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;",
     "void hudQueueQuad(win32::WinApplication* app, x86::reg32 list, x86::reg32 a, x86::reg32 b, x86::reg32 c,"
     " x86::reg32 d);\n"),
    # sub_481c50, the table's panel placed: esi is the player, [ebp-8] the car's
    # flag 0x20 that picks the table's other layout.
    ("nfs3hp.20.cpp", "00481f86  89ec",
     "    cpu.esp = cpu.ebp;",
     "    hudTablePanel(app, cpu.esi, app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8)) != 0);"
     " /* port: the table's rectangle is its panel */\n"
     "    cpu.esp = cpu.ebp;",
     "void hudTablePanel(win32::WinApplication* app, x86::reg32 player, bool otherLayout);\n"),
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
