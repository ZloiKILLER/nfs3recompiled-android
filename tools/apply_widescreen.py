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
- The saved screen size.  config.dat keeps the width of the chosen size, not
  its place in the list ([0x6fbc18]), and at startup sub_4730d0 hands it to
  sub_4bee80 to find the mode again: the one of that width, or the nearest.
  sub_4bee80 runs the list's own tests on the table, the 4:3 one among them
  (0x4beee6), so a 16:9 size was never found and 1920 came back as 1024x768,
  the nearest 4:3 width -- the menu had to be set again every time the game
  started.  That jump asks nfs3hp::widescreenMode too.  Dynamic Fullscreen
  is kept as a marker rather than a width (its width is the screen's), and
  as sub_4bee80 ends (0x4bef35) nfs3hp::dynamicSavedMode answers the marker
  with its mode.  The Graphics menu names it: as sub_448210 has written a
  mode's line (0x448291), nfs3hp::dynamicFullscreenLabel puts the name in
  Dynamic Fullscreen's.
- Field of view.  Every camera -- chase and in-car, the mirror, each half of
  split screen -- sets up its projection in sub_4dbce0 from one horizontal half
  angle, and makes the vertical one 13/16 of it, in whole degrees.  What is
  drawn is undistorted only where the two angles' tangents stand in the ratio
  of the view's own sides, and 13/16 of an angle does that at one angle and no
  other: about 1.27 at 30 degrees, 1.35 at 45, 1.52 at 60, against the 1.33 a
  4:3 screen wants.  So the game itself squeezed the picture at close angles
  and stretched it at wide ones -- a few percent, unremarked on a 4:3 monitor,
  but the cinema cameras of a replay and of a finish open wide enough to show
  it.  Once the vertical angle is stored (0x4dbd1b),
  nfs3hp::widescreenHalfAngle gives back the horizontal angle that the
  vertical one and the view's own shape ask for, so the two tangents stand in
  that ratio at every camera angle and on every screen -- 16:9, 16:10 or a
  phone's 20:9 -- which is Hor+ where the screen is wide and the game's own
  proportions put right where it is not.  The float projection (sub_4bf260) and
  the game's older integer one (sub_4fd3f0) both take the angles from there,
  and nothing reads them before that point.
- Zoomed cameras.  The television cameras of a replay and of the finish take
  no angle at all: sub_4dbde0 sets their projection from the camera's zoom
  (+0x54), the x scale half the view's width times it and the y scale half the
  height times it and times 1.34834 ([0x54972c]) -- 4:3 written into the code.
  On a 16:9 view that leaves the y scale a quarter short of the x one, and every
  car these cameras show is squashed flat.  Where the view is wider than 4:3,
  nfs3hp::zoomHalfWidth hands the x scale the y scale's half height instead
  (0x4dbe8f), but only while a race is active: square pixels, the vertical view
  as the game made it and more of the world at the sides, as the other cameras
  have it.  The Player Car menu takes the angle projection, not this one; there
  nfs3hp::widescreenHalfAngle measures the screen rather than the view, which
  a race leaves at its own size.
- A dialog's emblem.  Over a race, sub_4438d0 draws a dialog box -- the pause
  menu's "Quit to:" -- stretched with the rest of the 640x480 layout, and its
  round NFS emblem ([0x749a38], drawn by sub_4db000 at 0x443a44) with it.
  nfs3hp::dialogEmblem has that one draw narrowed towards its left edge by the
  4:3 factor: a round emblem on the box as the game stretches it, which is what
  the Modern Patch shows.
- Wide Screen in the Graphics menu ([0x6fbc4c]): bars above and below the view.
  sub_4be5e0 makes the view half as high as it is wide and centres it, which
  on a 4:3 screen leaves bars (drawn by sub_4be8e0); on a screen wider than
  2:1 the view came out taller than the screen, so no bars and a squeezed
  picture.  nfs3hp::letterboxHeight keeps 2:1 below that shape and makes the
  view 2.76:1 above it (0x4be626).  The bars crop the picture: the game
  projects through a second rectangle, the whole screen's height (sub_4beb00,
  0x7cdb88), and draws into the bars' (sub_4beb40, 0x7cdb98), so
  nfs3hp::widescreenHalfAngle takes the horizontal angle from the screen's
  height there, as the projection does.
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
# The Graphics menu's line for Dynamic Fullscreen: its name rather than the size and
# depth sub_448210 has just written for it.  That function is native whole
# (native_frontend_menua.cpp); tools/frontend_checks/generate.py makes the change to the
# game's code it is held against.
MODE_LABEL = [
    ("nfs3hp.12.cpp", "00448291  8b1518925500",
     "    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5607960) /* 0x559218 */);",
     "    dynamicFullscreenLabel(app, cpu.edx); /* port: Dynamic Fullscreen named in the list */\n"
     "    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5607960) /* 0x559218 */);",
     "void dynamicFullscreenLabel(win32::WinApplication* app, x86::reg32 entry);\n"),
]

# sub_4dbde0 is native whole (native_frontend_simc.cpp) and calls zoomHalfWidth itself;
# tools/frontend_checks/generate.py makes the change to the game's code it is held against.
ZOOM = [
    ("nfs3hp.31.cpp", "004dbe8f  d905ac005600",
     "x86::Float(app->getMemory<float>(x86::reg32(5636268) /* 0x5600ac */))",
     "x86::Float(zoomHalfWidth(app)) /* port: a racing zoom camera's x scale is its y scale on a wide view */",
     "float zoomHalfWidth(win32::WinApplication* app);\n"),
]

SITES = [
    ("nfs3hp.27.cpp", "004bedf8  0f8565000000",
     "    if (!cpu.flags.zf)",
     "    if (!cpu.flags.zf && !widescreenMode(cpu.edi, cpu.ebx)) /* port: 16:9 listed as well as 4:3 */",
     "bool widescreenMode(x86::reg32 width, x86::reg32 height);\n"),
    ("nfs3hp.27.cpp", "004beee6  7547",
     "    if (!cpu.flags.zf)",
     "    if (!cpu.flags.zf && !widescreenMode(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4)),"
     " app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16)))) /* port: the saved 16:9 size is found again */",
     ""),
    # The saved size is Dynamic Fullscreen's marker rather than a width: its
    # mode, whatever the screen makes its width.
    ("nfs3hp.27.cpp", "004bef35  85db",
     "    cpu.clear_co();",
     "    cpu.ebx = dynamicSavedMode(cpu.ebx, app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8)));"
     " /* port: Dynamic Fullscreen found again */\n"
     "    cpu.clear_co();",
     "x86::reg32 dynamicSavedMode(x86::reg32 found, x86::reg32 width);\n"),
    ("nfs3hp.31.cpp", "004dbd1b  8945e8",
     "    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = cpu.eax;",
     "    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = cpu.eax;\n"
     "    cpu.esi = widescreenHalfAngle(app, cpu.esi, cpu.eax);"
     " /* port: the horizontal angle the view's own shape asks for */",
     "x86::reg32 widescreenHalfAngle(win32::WinApplication* app, x86::reg32 half, x86::reg32 vertical);\n"),
    # Wide Screen's view: 2:1 on a screen narrower than that, as the game has
    # it, and 2.76:1 on a wider one, where 2:1 was taller than the screen.
    ("nfs3hp.26.cpp", "004be626  c1f804",
     "    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (4 /*0x4*/ % 32));",
     "    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (4 /*0x4*/ % 32));\n"
     "    cpu.eax = letterboxHeight(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32)),"
     " app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28)), cpu.eax); /* port: bars on any screen */",
     "x86::reg32 letterboxHeight(x86::reg32 width, x86::reg32 height, x86::reg32 half);\n"),
    ("nfs3hp.11.cpp", "00443a44  e8b7750900",
     "    nfs3hp::port::draw2d::drawTexturedQuad(app, cpu.sync());",
     "    dialogEmblem(true); /* port: the dialog's NFS emblem round on a wide screen */\n"
     "    nfs3hp::port::draw2d::drawTexturedQuad(app, cpu.sync());\n"
     "    dialogEmblem(false);",
     "void dialogEmblem(bool drawing);\n"),
    ("nfs3hp.20.cpp", "004809eb  898264977400",
     "    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7640932) /* 0x749764 */) = cpu.eax;",
     "    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7640932) /* 0x749764 */) = cpu.eax;\n"
     "    widescreenHudRect(app, cpu.edx); /* port: pictures in the HUD keep their shape on a wide screen */",
     "void widescreenHudRect(win32::WinApplication* app, x86::reg32 slot);\n"),
]

NAMESPACE = "namespace nfs3hp\n{\n"

# Sites in functions that are native now, the change made in the native
# (src/nfs3hp/native_graphics.cpp): the Screen Size list (sub_4bed90,
# graphics::listScreenModes) and the saved size found again (sub_4bee80,
# graphics::findScreenMode), a HUD element's rectangle (sub_480910,
# native_render.cpp's render::hudRect), and a dialog box's emblem (sub_4438d0,
# port_dialogs.cpp's port::dialogs::drawBox).  tools/game_checks/generate.py still
# makes them to the game's code the natives are held against.
NATIVE_SITES = {0x4bedf8, 0x4beee6, 0x4bef35, 0x4809eb, 0x443a44}

# What an earlier version of this tool put after the stored vertical angle.  A
# tree patched by it goes back to the generated code first, so that one site
# serves a fresh disassembly and an old patch alike.
STALE_HALF_ANGLE = ("    cpu.esi = widescreenHalfAngle(app, cpu.esi);"
                    " /* port: Hor+ on a wide screen, the vertical angle as it was */\n")


def apply(root):
    for name in dict.fromkeys(site[0] for site in SITES):
        path = root / "src/nfs3hp/disassembly" / name
        text = path.read_text(newline="")
        eol = "\r\n" if text.count("\r\n") * 2 > text.count("\n") else "\n"

        def native(s):
            return eol.join(s.split("\n"))

        sites = [site for site in SITES if site[0] == name]
        declarations = native(HEADER + "".join(site[4] for site in sites))
        # An earlier version of this tool asked widescreenHalfAngle for the
        # horizontal angle alone; leave no second declaration of it behind.
        stale = native(HEADER + "x86::reg32 widescreenHalfAngle(win32::WinApplication* app, x86::reg32 half);\n")
        if stale != declarations:
            text = text.replace(stale, "")
        text = text.replace(eol + native(STALE_HALF_ANGLE), eol)
        if declarations not in text:
            namespace = native(NAMESPACE)
            if text.count(namespace) != 1:
                raise RuntimeError("%s: namespace opening changed" % name)
            text = text.replace(namespace, namespace + declarations, 1)

        for _, instruction, old, new, _ in sites:
            old, new = native(old), native(new)
            anchor = "    // " + instruction
            start = text.find(anchor)
            if start < 0 and int(instruction[:8], 16) in NATIVE_SITES:
                continue  # its function is native and carries the change itself
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
