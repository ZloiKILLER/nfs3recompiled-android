"""The game's code for the functions src/nfs3hp/native_game.cpp has natively,
made afresh from nfs3hp/nfs3.exe.

    python3 generate.py <out dir>

What disassemble_nfs3hp.py makes of nfs3.exe, without the port's hooks: each
function of its GAME_NATIVES as the game's code has it (game_functions.cpp),
every function those call by address named in a header of their own
(nfs3hp.h, for game_checks.cpp to stand in for), and the game's data
(game_data.cpp), which game_checks.cpp loads.  Needs capstone, as
disassemble_nfs3hp.py does; the whole of the game is read, which takes a
minute or so.
"""
import io
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))
sys.path.insert(0, str(ROOT / "disasm"))

from disasm import disassembler  # noqa: E402
from disasm.codegen import fpu as fpu_codegen  # noqa: E402
import disassemble_nfs3hp as nfs3hp  # noqa: E402


# The formatted output's entries -- vsprintf, sprintf, vfprintf -- as the
# other natives call them: by address, so through their stand-ins in
# game_checks.cpp on both sides, which print text of their own -- what the
# other natives hand them to format being made up.  The formatted output is
# tested itself with formats and arguments made to go together, and calls its
# own entries as it does (sprintf vsprintf, fprintf vfprintf).
# So are __tzset, exit and __MkTmpFile: the natives that call them (localtime,
# mktime, Error, tmpfile) are tested with their stand-ins -- the time zone
# made up, the game not left, a name made up -- and they are tested themselves.
# And _nrealloc, which __allocPOSIXHandle grows the handles' table with; and
# getche and __reporterror, the console's echo and the maths' error, whose
# stand-ins make up a key and _matherr's say.  Each with the natives that
# call it as they do, in the tests of their own.
STOOD_IN = {
    0x4df674: (0x4df690,),  # vsprintf: sprintf's
    0x4df690: (),
    0x4ff014: (0x4f4b80,),  # vfprintf: fprintf's
    0x4f4b80: (),           # fprintf: graphics::warnTextureMemory's
    0x515c2c: (),
    0x4dfd04: (),
    0x4ee168: (),
    0x5185c0: (0x4f8ce8, 0x518290, 0x52c178),  # _nrealloc: __setenv's, __wsetenv's, __wputenv's
    0x522e60: (),
    0x5243a7: (),
    # The port's rewrites one part calls of another's, by address: stood in on
    # both sides, each held against its code on its own (port_text's
    # flushQuads the 2D drawing's quads' outcodes; port_draw2d's
    # buildScreenFont and startScreens the shapes' texture and textures freed).
    0x4d5840: (),
    0x4d6070: (),
    0x4d5c40: (),
    # SND's bank add and header size and copy, which port_sound_banks/music/racebanks call by
    # address: the checks' random bank images would crash the real ones.
    0x4e8d20: (),
    0x4e8db0: (0x4e8e20,),  # SNDbankheadercopy calls the size as C++, like the real one
    0x4e8e20: (),
    # The CD stream delete the music's stream end calls, a random stream's: by address too.
    0x4e4b50: (),
    # The reverb's start, which the mixer's init calls by address (iSNDmixinit's check).
    0x5230b8: (),
    # SND's restore, which waits for threads and the message thread, none of which the checks have
    # (soundExit's check): by address on both sides.
    0x4e26b4: (),
    # The music started and stopped, which the menu loop, the attract mode and the race's sound
    # call by address: the real ones start streams the checks' random worlds have not.
    # musicAfterSetting and musicMenuSix (port_sound_music), and raceSoundEnd the stop, call them
    # as C++.
    # The path ended with a backslash (port::menus), which the car files, the cops' speech and
    # the race banks call by address: on both sides, as before it was a rewrite.  The .MNU
    # loader's own call it as C++.
    0x4529a0: (0x449510, 0x4498a0, 0x449960),
    # A shape into pixels (port::shapes), which the object's textures reach by address; the
    # shapes' own textures (by name, one, an art's) call it as C++.
    0x4d37a0: (0x4d5db0, 0x4d6070, 0x4d6390),
    0x4104f0: (0x4103a0, 0x410460),
    0x4109d0: (0x4103a0, 0x410460, 0x412b50),
    # port_obj3d's calls of port_fields' car colour and its bytes, by address;
    # port_fields' own call them as C++ (the colour list, the custom colours,
    # a colour's line, the wheel).
    0x439790: (0x456a20, 0x472360),
    0x439810: (0x43dbe0, 0x456a20, 0x456b50, 0x472360),
    # port_buttons' calls of port_shapes' art's textures (the track's map's
    # tile, the images' pictures): by address to its stand-in, as its
    # graphics are the shapes' cases' -- but shapeByName's, which calls it as
    # C++, and port_fields' sliderShape, which does the same.
    0x4d6390: (0x4d5db0, 0x4d5e40),
    # port_menus' codelink match (0x4429c0), which port_buttons calls by
    # address: to its stand-in (frontend_glue.inc's) on both sides, as before
    # it was rewritten -- port_menus holds it on its own.
    0x4429c0: (),
    # port_loop's calls of the port's own, held against their code on their
    # own: by address, to their stand-ins (loop.inc's), on both sides.  The
    # menu key, the text boxes, the menus' end, the pointer, the screens
    # started, the network's leaving, the .MNU files and the language's text
    # (the game's main calls it by address, so it is stood in there too): no
    # other part calls them as C++.
    0x451960: (), 0x451b20: (), 0x451fc0: (), 0x451ff0: (), 0x452110: (), 0x4dab90: (), 0x4daf80: (),
    0x440d20: (), 0x449510: (), 0x449960: (), 0x4d1750: (),
    # And those other parts call as C++, which they keep doing: the menu
    # click (port_text's key, the fields' colour list key, the cascade, the
    # check box, the drop list, the lists' key), the menu text (the fields'
    # and the widgets' drawing), the gradient and the frame (the 2D drawing's
    # own, the list box, the slider, the text box).
    0x4181d0: (0x451960, 0x457030, 0x461d00, 0x4624a0, 0x462ad0, 0x4634d0, 0x463ff0),
    0x452190: (0x456590, 0x456b50, 0x456ce0, 0x457c40, 0x4580b0, 0x458480, 0x458940, 0x458e30, 0x45fcd0, 0x4607c0,
               0x461d00, 0x4629c0, 0x462b90, 0x462d20, 0x4634d0, 0x465500, 0x469680, 0x46b3a0),
    0x4d8550: (0x465500, 0x4d8910, 0x4d8930, 0x4d89f0),
    0x4d8930: (0x455e30, 0x469680, 0x4d8aa0),
    # The menu loop's own (port::loop) that the widgets and the fields call by
    # address: stood in for them (loop.inc's rows and buttons.inc's), but for
    # the loop's entries that call each as C++ (the help box asks whether the
    # keys are taken and for the screen shown; leaving a screen and the loop
    # draw the items).
    0x449df0: (0x449aa0,),
    0x44a470: (),
    0x44a4f0: (0x44aa30, 0x44b230),
    0x44aeb0: (0x449aa0,),
    # port_dialogs' calls of the port's own, held on their own: by address,
    # to their stand-ins (dialogs.inc's), but for the entries that keep
    # calling each as C++ -- the text's font and width, the 2D drawing and the
    # shapes (the fields' and the widgets' drawing, the text's own, the
    # sliders, the shapes').
    0x452100: (0x451cd0, 0x452020, 0x452190, 0x452220, 0x4522c0, 0x455e30, 0x457c40, 0x4580b0, 0x458480, 0x458940,
               0x458e30, 0x45fcd0, 0x4607c0, 0x462b90, 0x4634d0, 0x469680, 0x46b3a0),
    0x452020: (0x452190, 0x452220, 0x4522c0, 0x4569a0, 0x457c40, 0x457f70, 0x4580b0, 0x458340, 0x458480, 0x4587e0,
               0x458940, 0x458ce0, 0x458e30, 0x459210, 0x460070, 0x4607c0, 0x4613e0, 0x4628d0, 0x4629c0, 0x462b30,
               0x462b90, 0x4634d0, 0x464560),
    0x4d8860: (0x455e30, 0x45fae0, 0x4607c0),
    0x4d8ac0: (0x456ce0, 0x45fae0, 0x460100, 0x461d00, 0x462b90, 0x462d20, 0x4634d0),
    0x4d8910: (0x456b50, 0x466cf0, 0x469680),
    0x4db000: (0x4607c0, 0x469680, 0x46b3a0),
    0x4d7160: (0x440c40, 0x456670, 0x460100, 0x4607c0, 0x462d20, 0x4634d0, 0x469680, 0x4d7990),
    0x4d66f0: (0x455e30, 0x456020, 0x456110, 0x460100, 0x469080),
    0x4d5db0: (0x456170, 0x456ea0, 0x460440, 0x461900, 0x4623a0, 0x462c30, 0x463e60),
    # The dialogs' own (port::dialogs) that the menu loop, the widgets and the
    # screens call by address: stood in for them (frontend_glue.inc's and
    # loop.inc's rows, dialogs.inc's), and the dialogs' cores each called as
    # C++ by the entries that call them.
    0x4438b0: (), 0x444a60: (), 0x445de0: (), 0x445e60: (), 0x445f50: (),
    0x445f10: (0x445e60, 0x445f50),
    0x4438d0: (0x4449d0, 0x444b60, 0x4450c0, 0x445680),
    0x443f40: (0x4449d0, 0x444b60, 0x4450c0, 0x445680),
    0x444000: (0x4449d0, 0x4450c0, 0x445680),
    0x444350: (0x4450c0,),
    0x445380: (0x445680,),
    0x4445d0: (0x4449d0, 0x4450c0, 0x445680, 0x445ac0),
    0x4451d0: (0x445680, 0x445ac0),
    0x445160: (0x445520,),
    0x4452d0: (0x445750,),
    0x444910: (0x445e60,), 0x444b20: (0x445e60,), 0x444f50: (0x445e60,), 0x445520: (0x445e60,),
    0x4449d0: (0x445f50,), 0x444b60: (0x445f50,), 0x4450c0: (0x445f50,), 0x445680: (0x445f50,),
    0x445ac0: (0x445f50,),
    # The view's grid (port::view), which the view's frame calls by address: stood in there, held on its own.
    0x464840: (),
    # The edge pair emitter (port::view), which the glyph geometry's builder calls by address.
    0x474db0: (),
    # The HUD's aligned text (port::hud), which the HUD's other texts call by address.
    0x4897f0: (),
    # The HUD's tables' and map's starters and view fitter (port::hud), which the HUD's start and its panes call by
    # address: stood in there, held on their own.
    0x48aa40: (),
    0x48cdb0: (),
    0x48d110: (),
    0x4844c0: (),
    0x484c10: (),
    0x4e8f28: (),
    0x485c20: (),
    0x4870c0: (),
    0x486f70: (),
    0x485f20: (0x4873e0,),   # (the stroke's own clip: port::hud::strokeClippedDrawn calls it as C++)
    0x4873e0: (),
    0x48cb50: (),
    0x48cc60: (),
    0x486df0: (),
}
# The car's colour count (port_fields), which the view's nearest colour calls by address: stood in there,
# held on its own; the fields' entries each call it as C++.
STOOD_IN[0x439760] = tuple(sorted(nfs3hp.PORT_FIELDS_NATIVES))


def force_feedback_car(text):
    """tools/apply_force_feedback_car.py's change, which forcePlayerCar has."""
    from tools.apply_force_feedback_car import SITES
    for _, instruction, old, new in SITES:
        start = text.index("    // " + instruction + "  ")
        stop = text.index("\n    // 00", start + 1)
        block = text[start:stop]
        assert block.count(old) == 1, instruction
        text = text[:start] + block.replace(old, new, 1) + text[stop:]
    return text


def music_stream(text):
    """tools/apply_music_stream.py's change, which raceMusicPrepare and
    raceMusicStart have: Pathfinder's start called by address here."""
    from tools.apply_music_stream import SITES
    for _, instruction, old, new, _ in SITES:
        old = old.replace("nfs3hp::snd::SNDpathstartstream", "sub_4e6f0c")
        new = new.replace("nfs3hp::snd::SNDpathstartstream", "sub_4e6f0c")
        if "    // " + instruction not in text:
            continue
        start = text.index("    // " + instruction)
        stop = text.index("\n    // 00", start + 1)
        block = text[start:stop]
        assert block.count(old) == 1, instruction
        text = text[:start] + block.replace(old, new, 1) + text[stop:]
    return text


# The port's own changes to the game's code that natives carry: made to the
# game's code for them too, so that each is held against what the port runs.
def port_sites(sites):
    """A tools/apply_*.py's SITES -- (file, instruction, old, new, ...) --
    made to the game's code where the instruction is in it."""
    def change(text):
        for site in sites:
            _, instruction, old, new = site[:4]
            anchor = "    // " + instruction
            if anchor not in text:
                continue
            start = text.index(anchor)
            stop = text.index("\n    // 00", start + 1)
            block = text[start:stop]
            assert block.count(old) == 1, instruction
            text = text[:start] + block.replace(old, new, 1) + text[stop:]
        return text
    return change


def widescreen(text):
    """tools/apply_widescreen.py's changes, which the Screen Size list's
    natives (graphics::listScreenModes, findScreenMode) have."""
    from tools.apply_widescreen import SITES
    return port_sites(SITES)(text)


def track_detail(text):
    """tools/apply_track_detail.py's changes, which load::defaultConfig has --
    View Distance Full where the settings start over -- and render::
    viewDistances, viewRecord and detailSettings: Full's whole distance."""
    from tools.apply_track_detail import NATIVE_SITES
    return port_sites(NATIVE_SITES)(text)


def alpha_intensity(text):
    """tools/apply_alpha_intensity.py's change, which track::projectedTriangle
    has: the scaling's driver probe taken as Direct3D's."""
    from tools.apply_alpha_intensity import SITES, MEMORY_TEST, REGISTER_TEST, FORCED
    for _, address, code, register in SITES:
        anchor = "    // %s  %s" % (address, code)
        if anchor not in text:
            continue
        test = MEMORY_TEST if register is None else REGISTER_TEST % register
        was = "the 0x7a3a58 probe" if register is None else "%s & 0x40, %s read from 0x7a3a58" % (register, register)
        start = text.index(anchor)
        stop = text.index("\n    // 00", start + 1)
        block = text[start:stop]
        assert block.count(test) == 1, address
        text = text[:start] + block.replace(test, FORCED % was, 1) + text[stop:]
    return text


def mirror_detail(text):
    """tools/apply_mirror_detail.py's changes, which track::modelByDistance,
    drawTrack and headlightsOnObjects have: the mirror's pass asked through
    nfs3hp::mirrorAsMain."""
    from tools.apply_mirror_detail import SITES
    return port_sites(SITES)(text)


def car_detail(text):
    """tools/apply_car_detail.py's changes, which render::carTextures and
    transformBuffer have: every car's texture at the player's size, four times
    the transform buffer."""
    from tools.apply_car_detail import SITES
    return port_sites(SITES)(text)


def hud_scale(text):
    """tools/apply_hud_scale.py's changes, which render's HUD frame and the
    layouts inside it have: the HUD's sizes from a 4:3 screen."""
    from tools.apply_hud_scale import READS
    changed = port_sites([(None, address + "  ", old, new) for address, (old, new) in READS])(text)
    if "hudReference" in changed and "x86::reg32 hudReferenceWidth(" not in changed:
        # (Inside the function: the generated file is cut into parts at each function's signature.)
        declarations = ("    x86::reg32 hudReferenceWidth(win32::WinApplication* app);\n"
                        "    x86::reg32 hudReferenceHeight(win32::WinApplication* app);\n")
        brace = changed.index("\n{\n") + 3
        changed = changed[:brace] + declarations + changed[brace:]
    return changed


def cabin_fit(text):
    """tools/apply_cabin_fit.py's changes, which render::needles, barGauges
    and cockpit have: the cabin's rectangle fitted to 4:3."""
    from tools.apply_cabin_fit import READS, FIELDS
    sites = []
    for instruction, field in READS:
        decimal, index = FIELDS[field]
        sites.append((None, instruction, "app->getMemory<x86::reg32>(x86::reg32(%d) /* %s */)" % (decimal, field),
                      "cabinRect(app, %d) /* port: the cabin fitted to 4:3, was [%s] */" % (index, field)))
    return port_sites(sites)(text)


def hud_editor(text):
    """tools/apply_hud_editor.py's changes, which render::hudTable has -- the
    table's rectangle is its panel -- and port::text::queueQuad: each quad
    queued measured for the editor."""
    from tools.apply_hud_editor import SITES
    return port_sites(SITES)(text)


def textures_512(text):
    """The port's textures of 512 a side (port_thrash.h), which the natives
    have as edits of their own, no tools/apply_*.py: track::loadTextures asks
    for 512, not 128; load::loadTgaTexture makes a .tga of 512 or more 512
    (texture512Side) and a texture it is given the size fitted
    (resizeTexture)."""
    side_512 = ("if (app->getMemory<x86::reg32>(cpu.edi + 0xc) == 1)\n"
                "    {\n"
                "        const x86::sreg32 side = x86::sreg32(app->getMemory<x86::reg8>(cpu.edx + 0xc)"
                " | app->getMemory<x86::reg8>(cpu.edx + 0xd) << 8);\n"
                "        if (cpu.ebx != 0 && x86::sreg32(cpu.ebx) < 0x200 && side >= 0x200)\n"
                "            cpu.ebx = 0x200;\n"
                "    }\n    ")
    resize = ("port::driver::resizeTexture(app, app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */),"
              " app->getMemory<x86::reg32>(cpu.esi));\n        ")
    text = port_sites([
        (None, "0041db7b", "cpu.ebx = 128 /*0x80*/;", "cpu.ebx = 512 /*0x200*/;"),
        (None, "0049d32d", "app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.ebx;",
         side_512 + "app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.ebx;"),
        (None, "0049d3e7", "goto L_0x0049d3f6;", resize + "goto L_0x0049d3f6;"),
    ])(text)
    if "resizeTexture" in text:
        text = ("namespace port { namespace driver {\n"
                "void resizeTexture(win32::WinApplication* app, x86::reg32 record, x86::reg32 side);\n"
                "} }\n\n" + text)
    return text


def loading_screen(text):
    """tools/apply_loading_screen.py's change, which render::loadingScreen
    has: the loading screen at 4:3."""
    from tools.apply_loading_screen import SITES
    return port_sites(SITES)(text)


def dynamic_fullscreen_mode(text):
    """Dynamic Fullscreen's change to sub_4bef50, which only its native
    (render::screenMode) has, no tools/apply_*.py: as a race opens the mode
    the screen is measured again, and a size that changed opens the mode
    afresh -- the mode kept at 0x55fec0 forgotten."""
    return port_sites([(None, "004bef5b  89c3", "    cpu.ebx = cpu.eax;",
                        "    if (dynamicFullscreenMode(cpu.eax) && dynamicFullscreenRefresh(app)"
                        " && app->getMemory<x86::reg32>(x86::reg32(5635776) /* 0x55fec0 */) == cpu.eax)"
                        " /* port: Dynamic Fullscreen measured again */\n"
                        "        app->getMemory<x86::reg32>(x86::reg32(5635776) /* 0x55fec0 */) = 0;\n"
                        "    cpu.ebx = cpu.eax;")])(text)


def dynamic_fullscreen_memory(text):
    """Dynamic Fullscreen's change to sub_4b59c0, which only its native
    (render::startDriver) has: the driver started, the memory sub_4bed90
    lists a mode by at least 32 MB -- at each way out but the one for a
    driver past 10."""
    clamp = ("    if (app->getMemory<x86::reg32>(x86::reg32(5635792) /* 0x55fed0 */) < 0x2000000u)"
             " /* port: Dynamic Fullscreen's listing memory */\n"
             "        app->getMemory<x86::reg32>(x86::reg32(5635792) /* 0x55fed0 */) = 0x2000000u;\n"
             "    cpu.esp = cpu.ebp;")
    return port_sites([(None, address + "  89ec", "    cpu.esp = cpu.ebp;", clamp)
                       for address in ("004b5a20", "004b5ab0", "004b5be7", "004b5c6e", "004b5d5d", "004b5e02",
                                       "004b5e2e")])(text)


def dialog_emblem(text):
    """tools/apply_widescreen.py's change at 0x443a44, which port::dialogs::
    drawBox has: over a race the box's art is drawn with the emblem narrowed
    on a wide screen (dialogEmblem).  The art's quad is drawn by address here,
    as the generator writes the game's calls."""
    anchor = "    // 00443a44  e8b7750900"
    start = text.index(anchor)
    stop = text.index("\n    // 00", start + 1)
    block = text[start:stop]
    call = "    sub_4db000(app, cpu.sync());"
    assert block.count(call) == 1
    block = block.replace(call, "    dialogEmblem(true);\n" + call + "\n    dialogEmblem(false);", 1)
    return "void dialogEmblem(bool drawing);\n\n" + text[:start] + block + text[stop:]


def coll_file_strncmp(text):
    """collide::openCollFile's strncmp of the COLL header as the port runs it,
    native (watcom::strncmp): the game's own leaves edx otherwise (an argument
    register, the caller's no longer), and the file's close after it is
    handed edx as the strncmp left it."""
    return text.replace("sub_4eb390(app, cpu.sync());", "nfs3hp::watcom::strncmp(app, cpu.sync());")


PORT_CHANGES = {0x4710f0: force_feedback_car, 0x410030: music_stream, 0x4107c0: music_stream,
                0x49bd30: hud_editor,
                0x4bed90: widescreen, 0x4bee80: widescreen,
                0x4723f0: track_detail,
                0x41a120: alpha_intensity, 0x41aa60: mirror_detail, 0x41c310: mirror_detail,
                0x41cdc0: mirror_detail, 0x431320: coll_file_strncmp,
                0x4438d0: dialog_emblem}
# native_render.cpp's, for tools/game_checks/render (gen_render.py).
PORT_CHANGES.update({
    0x41b9b0: alpha_intensity, 0x41d620: lambda text: mirror_detail(track_detail(text)),
    0x41df10: track_detail, 0x472d10: track_detail,
    0x47a190: mirror_detail, 0x47a880: mirror_detail, 0x47b850: alpha_intensity,
    0x47f650: hud_scale, 0x4800a0: hud_scale, 0x480910: widescreen, 0x480fc0: hud_scale,
    0x4812e0: hud_scale, 0x481aa0: hud_scale, 0x481c50: lambda text: hud_editor(hud_scale(text)),
    0x487210: hud_scale, 0x4880b0: cabin_fit, 0x488470: cabin_fit, 0x489240: cabin_fit,
    0x491190: alpha_intensity, 0x491bc0: alpha_intensity, 0x494cb0: loading_screen,
    0x49d800: alpha_intensity, 0x4b56e0: car_detail, 0x4b59c0: dynamic_fullscreen_memory,
    0x4b7d30: car_detail, 0x4bef50: dynamic_fullscreen_mode,
    0x4b9bc0: mirror_detail, 0x4bae00: mirror_detail, 0x4bb200: mirror_detail,
    0x4ddb40: alpha_intensity, 0x4de070: alpha_intensity, 0x4de700: alpha_intensity})
# port_hud.cpp's: the HUD's, with the changes of the HUD's scale in them.
PORT_CHANGES.update({0x484b70: hud_scale, 0x485180: hud_scale, 0x48a660: hud_scale, 0x48ae70: hud_scale, 0x48aa40: hud_scale, 0x48d110: hud_scale})
# native_vertices.cpp's, for tools/game_checks/vertices (gen_vertices.py).
PORT_CHANGES.update({0x492370: alpha_intensity, 0x492980: alpha_intensity, 0x4cb750: alpha_intensity,
                     0x4cbc90: alpha_intensity, 0x41ae90: mirror_detail, 0x4bb5d0: mirror_detail})
PORT_CHANGES.update({0x41db10: textures_512, 0x49d320: textures_512})


def stand_ins():
    """The addresses game_checks.cpp's tables of the game's functions stand in
    for: their rows, {0x..., "name", ...}, in it and its parts."""
    here = Path(__file__).parent
    found = set()
    for path in [here / "game_checks.cpp"] + sorted(here.glob("*.inc")):
        found |= {int(a, 16) for a in re.findall(r'^\s*\{(0x[0-9a-f]+), "[^"]*", k(?:Watcom|Cdecl|Stdcall)',
                                                   path.read_text(encoding="utf-8"), re.M)}
    return found


_SOURCES = {}


def named_addresses(native):
    """The addresses the source file of `native` ("ns::name") names in its code
    (comments left out): those it can call by address.  None when no file
    under src/nfs3hp defines it."""
    if not _SOURCES:
        def code_of(path):
            text = path.read_text(encoding="utf-8", errors="replace")
            return re.sub(r"/\*.*?\*/|//[^\n]*", "", text, flags=re.S)

        def addresses_of(code):
            return {int(a, 16) for a in re.findall(r"\b0x([0-9a-f]{6})\b", code)}

        # The headers' addresses count for the files that include them
        # (native_snd.h's, port_snd.h's).
        headers = {path.name: addresses_of(code_of(path)) for path in (ROOT / "src/nfs3hp").glob("*.h")}
        for path in sorted((ROOT / "src/nfs3hp").glob("*.cpp")):
            code = code_of(path)
            addresses = addresses_of(code)
            for header in re.findall(r'#include "([\w.]+\.h)"', code):
                addresses |= headers.get(header, set())
            _SOURCES[path] = (code, addresses)
    space, _, name = native.rpartition("::")
    for code, addresses in _SOURCES.values():
        if re.search(r"\bvoid %s\(" % re.escape(name), code) and (
                not space or re.search(r"\bnamespace %s\b" % re.escape(space.split("::")[-1]), code)):
            return addresses
    return None


def main():
    out = Path(sys.argv[1]).resolve()
    out.mkdir(parents=True, exist_ok=True)
    game = disassembler.disassemble("nfs3hp", str(ROOT / "nfs3hp/nfs3.exe"), nfs3hp.DATA_SEGMENTS,
                                    nfs3hp.KNOWN_SUBROUTINES, nfs3hp.SPLIT_INSTRUCTIONS,
                                    nfs3hp.THREAD_ROUTINES, nfs3hp.THREAD_SEGMENTS, nfs3hp.MERGE_ROUTINES)
    # The rewrites written in doubles (the HUD, the view, the speech decoder) are
    # held against the game's code in doubles: these differ from a float in the
    # last bit, and only what they draw or play sees it.
    game.double_functions = {address for address, name in nfs3hp.GAME_NATIVES.items()
                             if name.startswith(("port::hud::", "port::view::", "port::coded", "port::convert"))
                             or name in ("port::reverbLength", "port::gapLine")}
    # The menus' screens are frontend_checks' (tools/frontend_checks).
    # Not held against their code: the sound and graphics code's dead stubs (they only stop
    # the game), and the mix and a channel's ramp, whose native mixer was held
    # against the game's mix by mix on the phone (0.77's snd_check).
    natives = {address: name for address, name in nfs3hp.GAME_NATIVES.items()
               if address not in nfs3hp.MENU_NATIVES and not name.startswith(("snd::dead_", "graphics::dead_", "dead::", "removed::"))
               and name not in ("port::mixerServe", "port::mixerRamp")}
    # native_render.cpp's are tools/game_checks/render's: here, as before they
    # were native whole, stand-ins for the natives that call them.
    natives = {address: name for address, name in natives.items() if address not in nfs3hp.RENDER_NATIVES}
    # native_vertices.cpp's are tools/game_checks/vertices': here, as before
    # they were native whole, stand-ins for the natives that call them.
    natives = {address: name for address, name in natives.items() if address not in nfs3hp.VERTICES_NATIVES}
    stood_in_natives = stand_ins() & set(natives)
    code = io.StringIO()
    tested = []
    for subroutine in game.subroutines:
        entry = subroutine.get_entry_point()
        if entry in natives:
            function = io.StringIO()
            game.write_function(function, subroutine, "sub_%x" % entry, {}, nfs3hp.THREAD_SEGMENTS,
                                nfs3hp.SKIP_INSTRUCTIONS)
            text = function.getvalue()
            if entry in PORT_CHANGES:
                text = PORT_CHANGES[entry](text)
            # The natives game_checks.cpp stands in for, from the others: by
            # address too, as the natives call them -- but not a rewrite's
            # (port::) from another rewrite, which calls it as C++: the game's
            # code for it comes along there, on both sides.  Nor one STOOD_IN
            # lets this entry call as it does (sprintf its vsprintf), nor one
            # the entry's file never names in its code: that one the native
            # calls by name or does as the callee's code does (watcom's fopen
            # its fseek, frontend's text its languageText), and the stand-ins
            # other parts put on it (sound_lib's fseek) are for their calls by
            # address.  One controls file's from another's: below.
            rewrite = natives[entry].startswith("port::")
            named = named_addresses(natives[entry])
            for address in stood_in_natives:
                if address != entry and not (rewrite and natives[address].startswith("port::")) \
                        and entry not in STOOD_IN.get(address, ()) \
                        and (named is None or address in named):
                    text = text.replace("sub_%x(app, cpu.sync());" % address,
                                        "app->dynamic_call(0x%x, cpu.sync());" % address)
                    text = text.replace("return sub_%x(app, cpu.handOver());" % address,
                                        "return app->dynamic_call(0x%x, cpu.handOver());" % address)
            for address, callers in STOOD_IN.items():
                if entry not in callers:
                    text = text.replace("sub_%x(app, cpu.sync());" % address,
                                        "app->dynamic_call(0x%x, cpu.sync());" % address)
            # The menus' natives that are frontend_checks' are called by
            # address too, to their stand-ins (loadScreenArt's menuCurrentName).
            for address, name in nfs3hp.MENU_NATIVES.items():
                text = text.replace("nfs3hp::%s(app, cpu.sync());" % name,
                                    "app->dynamic_call(0x%x, cpu.sync());" % address)
            # And one controls file's natives, from another's: by address, to
            # stand-ins, as the natives call them.
            own = [group for group in nfs3hp.CONTROLS_GROUPS if entry in group]
            for group in nfs3hp.CONTROLS_GROUPS:
                if own and group is not own[0]:
                    for address in group:
                        text = text.replace("sub_%x(app, cpu.sync());" % address,
                                            "app->dynamic_call(0x%x, cpu.sync());" % address)
            code.write(text)
            tested.append(entry)
    if sorted(tested) != sorted(natives):
        raise SystemExit("no function at %s" % sorted(set(natives) - set(tested)))
    text = code.getvalue()
    # Where a graphics native's code jumps out into another piece of the
    # game's code (return sub_X(app, cpu.handOver())), that piece is part of
    # it -- a tail, not a call -- and comes along as the game has it, however
    # the port binds it (closeDriver's way out, graphics::dead_ on the port).
    graphics = [address for address, name in natives.items() if name.startswith("graphics::")]
    tails = set()
    pieces = {subroutine.get_entry_point(): subroutine for subroutine in game.subroutines}
    pending = [text[text.index("void Application::sub_%x(" % a):] for a in graphics]
    while pending:
        body = pending.pop()
        end = body.find("\nvoid Application::", body.find("void Application::") + 1)
        body = body[:end] if end >= 0 else body
        for tail in set(int(a, 16) for a in re.findall(r"return sub_([0-9a-f]+)\(app, cpu\.handOver\(\)\)", body)):
            if tail in natives or tail in tails or tail not in pieces:
                continue
            tails.add(tail)
            function = io.StringIO()
            game.write_function(function, pieces[tail], "sub_%x" % tail, {}, nfs3hp.THREAD_SEGMENTS,
                                nfs3hp.SKIP_INSTRUCTIONS)
            text += function.getvalue()
            pending.append(function.getvalue())
    called = sorted(set(int(a, 16) for a in re.findall(r"\bsub_([0-9a-f]+)\(app", text)) - set(natives) - tails)

    with open(out / "game_functions.cpp", "w") as f:
        f.write('#include "nfs3hp.h"\n#include <lib/thread.h>\n\nnamespace nfs3hp\n{\n\n%s}\n' % text)
    with open(out / "nfs3hp.h", "w") as h:
        h.write("#ifndef NFS3HP_H_\n#define NFS3HP_H_\n#include <x86.h>\n#include <lib/winapp.h>\n\n"
                "namespace nfs3hp\n{\n\n")
        for address, name in sorted(natives.items()):
            h.write(game.native_declaration(address, name))
        h.write("/* The race music's hooks (music_stream). */\n"
                "void musicIndexOpened(win32::WinApplication* app, x86::reg32 path);\n"
                "void musicStreamFile(win32::WinApplication* app, x86::reg32 path);\n"
                "/* The Screen Size list's (widescreen). */\n"
                "bool widescreenMode(x86::reg32 width, x86::reg32 height);\n"
                "x86::reg32 dynamicSavedMode(x86::reg32 found, x86::reg32 width);\n"
                "/* The races' x87 word (race precision). */\n"
                "/* The mirror's pass as the main view's (mirror detail). */\n"
                "x86::reg32 mirrorAsMain(x86::reg32 kind);\n"
                "/* The HUD editor's measure of each quad queued (hud editor). */\n"
                "void hudQueueQuad(win32::WinApplication* app, x86::reg32 list, x86::reg32 a, x86::reg32 b,"
                " x86::reg32 c, x86::reg32 d);\n")
        h.write("\nclass Application : public win32::WinApplication\n{\npublic:\n"
                "    /* The game's code for the natives (game_functions.cpp). */\n")
        for address in sorted(natives):
            h.write("    static void sub_%x(WinApplication* app, x86::CPU& cpu);\n" % address)
        h.write("    /* The graphics natives' tails, as the game has them. */\n")
        for address in sorted(tails):
            h.write("    static void sub_%x(WinApplication* app, x86::CPU& cpu);\n" % address)
        h.write("    /* What it calls by address: game_checks.cpp's stand-ins. */\n")
        for address in called:
            h.write("    static void sub_%x(WinApplication* app, x86::CPU& cpu);\n" % address)
        h.write("};\n\n}\n\n#endif\n")
    with open(out / "game_tested.inc", "w") as f:
        for address, name in sorted(natives.items()):
            f.write('{0x%x, "%s", &nfs3hp::Application::sub_%x, &nfs3hp::%s},\n' % (address, name, address, name))
    with open(out / "game_stood_in.inc", "w") as f:
        # Called by address on both sides, to stand-ins (STOOD_IN): the native side keeps them
        # even when the callee is another rewrite (game_checks.cpp, dynamicCallElsewhere).
        for address in sorted(STOOD_IN):
            f.write("0x%x,\n" % address)
    with open(out / "game_called.inc", "w") as f:
        for address in called:
            f.write("STAND_IN(%x)\n" % address)
    # The decompiled functions' (nfs3hp.decompiled_list()): every register
    # compared, what their callers count on not being known -- the x87's stack
    # too for one that uses it ("fpu"); the pages their globals are on
    # compared; those globals made up for every case.  What they call of the
    # game's that has no stand-in of its own gets one that keeps to Watcom's
    # convention, its four argument registers logged and eax its result --
    # and the words its ret takes off the stack, logged too.
    decompiled = nfs3hp.decompiled_list()
    with open(out / "decompiled_promises.inc", "w") as f:
        for entry in decompiled:
            fpu = ", true, kNoHeap, true" if entry.get("fpu") else ""
            f.write('{"%s", true, true, 0%s},\n' % (nfs3hp.decompiled_name(entry), fpu))
    pages = sorted({int(g, 16) & ~0xfff for entry in decompiled for g in entry["globals"]})
    with open(out / "decompiled_windows.inc", "w") as f:
        for page in pages:
            f.write("{0x%x, 0x%x},\n" % (page, page + 0x2000))
    with open(out / "decompiled_world.inc", "w") as f:
        for entry in decompiled:
            f.write('{"%s", {%s}, {%s}},\n' % (nfs3hp.decompiled_name(entry), ", ".join(entry["globals"]),
                                                ", ".join(entry.get("values", []))))
    words = {}
    for subroutine in game.subroutines:
        entry = subroutine.get_entry_point()
        if entry in called:
            function = io.StringIO()
            game.write_function(function, subroutine, "sub_%x" % entry, {}, nfs3hp.THREAD_SEGMENTS,
                                nfs3hp.SKIP_INSTRUCTIONS)
            rets = [int(n) for n in re.findall(r"cpu\.esp \+= 4\+(\d+) /\*", function.getvalue())]
            words[entry] = max(rets, default=0) // 4
    from entry_regs import Reader
    reader = Reader()
    with open(out / "decompiled_standins.inc", "w") as f:
        for address in called:
            # A ret that takes words off the stack: stdcall's, which the
            # stand-in takes off too.  Its register arguments: those its code
            # reads on entry (entry_regs.py).
            n = words.get(address, 0)
            f.write('{0x%x, "sub_%x", %s, %d, %d, kEax, -1, 0, kNothing},\n'
                    % (address, address, "kStdcall" if n else "kWatcom", reader.register_arguments(address), n))
    with open(out / "game_data.cpp", "w") as f:
        f.write('#include <x86.h>\n\n/* The game\'s data as it loads: its address, length and bytes. */\n')
        names = []
        for data, address, length, name, ro, is_code in game.sections:
            if data and not is_code:
                data = bytes(data[:length])
                names.append((address, len(data), name.decode()))
                f.write("extern const x86::reg8 kGame%s[] = {\n" % name.decode())
                for i in range(0, len(data), 32):
                    f.write("    %s,\n" % ", ".join("0x%02x" % b for b in data[i:i + 32]))
                f.write("};\n")
        # The jump tables the tested code reads from among the game's code
        # (jmp [reg*4 + table]): their words, while they lead into the code.
        code = [(address, bytes(data[:length])) for data, address, length, name, ro, is_code in game.sections
                if data and is_code]
        for table in sorted(set(int(a) for a in re.findall(
                r"getMemory<x86::reg32>\((\d+) \+ cpu\.\w+ \* 4\); goto dynamic_jump", text))):
            for start, data in code:
                if start <= table < start + len(data):
                    words = []
                    while table + 4 * len(words) + 4 <= start + len(data) and len(words) < 256:
                        offset = table - start + 4 * len(words)
                        word = int.from_bytes(data[offset:offset + 4], "little")
                        if not start <= word < start + len(data):
                            break
                        words.append(word)
                    if words:
                        names.append((table, 4 * len(words), "Jumps%x" % table))
                        f.write("extern const x86::reg8 kGameJumps%x[] = {\n    %s,\n};\n" % (
                            table, ", ".join("0x%02x" % b for w in words for b in w.to_bytes(4, "little"))))
        f.write("extern const struct GameSegment { x86::reg32 address, length; const x86::reg8* bytes; } "
                "kGameSegments[] = {\n")
        for address, length, name in names:
            f.write("    {0x%x, %d, kGame%s},\n" % (address, length, name))
        f.write("};\nextern const unsigned kGameSegmentCount = %d;\n" % len(names))
    print("nfs3hp: %d functions tested, %d stood in for: %s" % (
        len(tested), len(called), " ".join("%x" % a for a in called)), file=sys.stderr)


if __name__ == "__main__":
    main()
