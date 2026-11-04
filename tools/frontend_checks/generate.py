"""The game's code for the menus' screens that are native whole, made afresh
from nfs3hp/nfs3.exe by the port's own generator.

    python3 generate.py <out.cpp>

The screens of disassemble_nfs3hp.py's MENU_NATIVES have no generated code in
src/nfs3hp/disassembly: their addresses are bound to the port's functions.
For frontend_checks.cpp to compare those with the game's code, it is written
here as the generator writes any function -- every call in it by address
(sub_...), as extract.py leaves the hooked ones' -- and extract.py takes it
from here.  The whole game is read, a minute or so.
"""
import io
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))
sys.path.insert(0, str(ROOT / "disasm"))

from disasm import disassembler  # noqa: E402
from disasm.codegen import fpu as fpu_codegen  # noqa: E402
import disassemble_nfs3hp as nfs3hp  # noqa: E402
from tools.apply_menu_unlock import CAP_TEST, CAP_FORCED  # noqa: E402
from tools.apply_hide_download import SITES as DOWNLOAD  # noqa: E402
from tools.apply_hide_connections import SITES as HIDDEN_CONNECTIONS  # noqa: E402
from tools.apply_network_port import SITES as NETWORK_PORT  # noqa: E402
from tools.apply_full_colour import NATIVE_SITES as FULL_COLOUR  # noqa: E402
from tools.apply_first_settings import SITES as FIRST_SETTINGS  # noqa: E402
from tools.apply_hud_editor import SITES as HUD_EDITOR  # noqa: E402
from tools.apply_widescreen import MODE_LABEL as WIDESCREEN_LABEL, ZOOM as WIDESCREEN_ZOOM  # noqa: E402


# The port's changes the natives carry, to the game's code they are held against.
HUD_ELEMENT = [site for site in HUD_EDITOR if site[1][:8] in ("0045bcc3", "0045bd0c")]
PATCHES = {0x4dbde0: WIDESCREEN_ZOOM, 0x45bcb0: HUD_ELEMENT, 0x4730d0: FIRST_SETTINGS,
           0x448210: FULL_COLOUR + WIDESCREEN_LABEL, 0x4405c0: HIDDEN_CONNECTIONS, 0x49e5f0: NETWORK_PORT[:1], 0x49e9d0: NETWORK_PORT[1:2],
           0x49ea70: NETWORK_PORT[2:]}
TEXT_ENTRY = {0x428020: ("    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;", "true"),
              0x427d00: ("    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;", "false"),
              0x427ea0: ("    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;", "false")}


def main():
    out = Path(sys.argv[1])
    game = disassembler.disassemble("nfs3hp", str(ROOT / "nfs3hp/nfs3.exe"), nfs3hp.DATA_SEGMENTS,
                                    nfs3hp.KNOWN_SUBROUTINES, nfs3hp.SPLIT_INSTRUCTIONS,
                                    nfs3hp.THREAD_ROUTINES, nfs3hp.THREAD_SEGMENTS, nfs3hp.MERGE_ROUTINES)
    fpu_codegen.EXACT = True  # as Application.write has the game's arithmetic
    code = io.StringIO()
    written = set()
    for subroutine in game.subroutines:
        entry = subroutine.get_entry_point()
        if entry in nfs3hp.MENU_NATIVES:
            function = io.StringIO()
            game.write_function(function, subroutine, "sub_%x" % entry, {}, nfs3hp.THREAD_SEGMENTS,
                                nfs3hp.SKIP_INSTRUCTIONS)
            text = function.getvalue()
            if entry in (0x448610, 0x44d2a0):
                text = text.replace(CAP_TEST, CAP_FORCED)
            for site in PATCHES.get(entry, ()):
                # After the instruction it is on: the same code may come earlier.
                at = text.index("// " + site[1])
                # The tree names a native callee; this code, by its address.
                old, new = site[2], site[3]
                for name, address in (("nfs3hp::graphics::hudEditorFrame", "sub_459440"),
                                      ("nfs3hp::render::detailSettings", "sub_472d10")):
                    old, new = old.replace(name, address), new.replace(name, address)
                if old not in text[at:]:
                    raise SystemExit("sub_%x: no %r" % (entry, old))
                text = text[:at] + text[at:].replace(old, new, 1)
            if entry == 0x426d60:
                text = text.replace(DOWNLOAD[0][2], DOWNLOAD[0][3])
            if entry in TEXT_ENTRY:
                # The phone's keyboard told, as the natives tell it (nfs3hp::textEntry).
                line, taking = TEXT_ENTRY[entry]
                text = text.replace(line, "    textEntry(%s);\n%s" % (taking, line), 1)
            code.write(text)
            written.add(entry)
    missing = set(nfs3hp.MENU_NATIVES) - written
    if missing:
        raise SystemExit("no function at %s" % " ".join("%x" % a for a in sorted(missing)))
    out.write_text(code.getvalue())


if __name__ == "__main__":
    main()
