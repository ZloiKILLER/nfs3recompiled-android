"""The game's code for native_vertices.cpp's natives, made afresh, for the
desktop differential check (vcheck.cpp) -- the port's own build has none of
it, the natives being whole (disassemble_nfs3hp.py's VERTICES_NATIVES):

    python gen_vertices.py <out dir>

game_functions.cpp: the natives' functions and the pieces they call that run
the same on both sides (PIECES) as the generator writes them, with the port's
changes the natives carry (tools/game_checks/generate.py's PORT_CHANGES, the
mirror's and the alpha intensity's elsewhere); nfs3hp.h: class Application
with every one of them and, for whatever else they call, a stand-in that goes
through dynamic_call; tested.inc: the functions with their natives;
game_data.cpp: the game's data as it loads."""
import io
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT))
sys.path.insert(0, str(ROOT / "disasm"))
from disasm import disassembler  # noqa: E402
from disasm.codegen import fpu as fpu_codegen  # noqa: E402
import disassemble_nfs3hp as nfs3hp  # noqa: E402
from tools.game_checks.generate import PORT_CHANGES, alpha_intensity, mirror_detail  # noqa: E402

# What the natives' functions call that both sides run as the game's code.
PIECES = [0x41a3e0, 0x41a120, 0x4bbde0, 0x4bbe40, 0x4bbe70, 0x4bbf80, 0x4227e0, 0x41e070, 0x422ba0, 0x423b00,
          0x4c7480, 0x4c20e0, 0x4bf790, 0x4e06d9, 0x4ea930, 0x4ea950, 0x4ea9c0, 0x4dfd56, 0x4e0110, 0x4cb220,
          0x4ca500, 0x4cab60]


def main():
    out = Path(sys.argv[1]).resolve()
    out.mkdir(parents=True, exist_ok=True)
    game = disassembler.disassemble("nfs3hp", str(ROOT / "nfs3hp/nfs3.exe"), nfs3hp.DATA_SEGMENTS,
                                    nfs3hp.KNOWN_SUBROUTINES, nfs3hp.SPLIT_INSTRUCTIONS,
                                    nfs3hp.THREAD_ROUTINES, nfs3hp.THREAD_SEGMENTS, nfs3hp.MERGE_ROUTINES)
    fpu_codegen.EXACT = True
    natives = {address: name.split("::")[-1] for address, name in nfs3hp.VERTICES_NATIVES.items()}
    pieces = {s.get_entry_point(): s for s in game.subroutines}
    wanted = sorted(set(natives) | set(PIECES))
    code = io.StringIO()
    for entry in wanted:
        if entry not in pieces:
            raise SystemExit("no function at %x" % entry)
        f = io.StringIO()
        game.write_function(f, pieces[entry], "sub_%x" % entry, {}, nfs3hp.THREAD_SEGMENTS, nfs3hp.SKIP_INSTRUCTIONS)
        text = f.getvalue()
        if entry in PORT_CHANGES:
            text = PORT_CHANGES[entry](text)
        else:
            text = mirror_detail(alpha_intensity(text))
        code.write(text)
    text = code.getvalue()
    called = sorted(set(int(a, 16) for a in re.findall(r"\bsub_([0-9a-f]+)\(app", text)) - set(wanted))
    (out / "game_functions.cpp").write_text(
        '#include "nfs3hp.h"\n#include <lib/thread.h>\n\nnamespace nfs3hp\n{\n\n%s}\n' % text)
    with open(out / "nfs3hp.h", "w") as h:
        h.write("#ifndef NFS3HP_H_\n#define NFS3HP_H_\n#include <x86.h>\n#include <lib/winapp.h>\n"
                "#include <lib/memmap.h>\n\nnamespace nfs3hp\n{\n\n"
                "x86::reg32 mirrorAsMain(x86::reg32 kind);\n"
                "double mirrorCarScale(double scale);\n")
        for address, name in sorted(natives.items()):
            h.write("namespace vertices { void %s(win32::WinApplication* app, x86::CPU& cpu); }\n" % name)
        h.write("\nclass Application : public win32::WinApplication\n{\npublic:\n")
        for address in wanted + called:
            h.write("    static void sub_%x(WinApplication* app, x86::CPU& cpu);\n" % address)
        h.write("};\n\n}\n\n#endif\n")
    with open(out / "tested.inc", "w") as f:
        for address, name in sorted(natives.items()):
            f.write('{0x%x, "%s", &nfs3hp::Application::sub_%x, &nfs3hp::vertices::%s},\n'
                    % (address, name, address, name))
    with open(out / "pieces.inc", "w") as f:
        for address in PIECES:
            f.write('{0x%x, &nfs3hp::Application::sub_%x},\n' % (address, address))
    with open(out / "called.inc", "w") as f:
        for address in called:
            f.write("STAND_IN(%x)\n" % address)
    with open(out / "game_data.cpp", "w") as f:
        f.write('#include <x86.h>\n\n')
        names = []
        for data, address, length, name, ro, is_code in game.sections:
            if data and not is_code:
                data = bytes(data[:length])
                names.append((address, len(data), name.decode()))
                f.write("extern const x86::reg8 kGame%s[] = {\n" % name.decode())
                for i in range(0, len(data), 32):
                    f.write("    %s,\n" % ", ".join("0x%02x" % b for b in data[i:i + 32]))
                f.write("};\n")
        f.write("extern const struct GameSegment { x86::reg32 address, length; const x86::reg8* bytes; } "
                "kGameSegments[] = {\n")
        for address, length, name in names:
            f.write("    {0x%x, %d, kGame%s},\n" % (address, length, name))
        f.write("};\nextern const unsigned kGameSegmentCount = %d;\n" % len(names))
    print("%d functions, %d native, stand-ins: %s" % (len(wanted), len(natives), " ".join("%x" % a for a in called)))


if __name__ == "__main__":
    main()
