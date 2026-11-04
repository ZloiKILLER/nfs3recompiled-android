"""The game's code for native_render.cpp's functions, for the desktop
differential check (rcheck.cpp):

    python gen_render.py <out dir>

Each function of disassemble_nfs3hp.py's RENDER_NATIVES as the generator
writes it without the port's natives and hooks, the port's changes in it made
as tools/game_checks/generate.py's PORT_CHANGES make them (what the natives
carry), renamed Gen::gen_X so that every call it makes by address -- to one of
the others too -- goes to rcheck.cpp's stand-in, as the natives' calls do.
Its idiv by zero gives what the generated code gives on arm64 (quotient 0, the
remainder the dividend), which the natives do too; the desktop would fault.

game_functions.cpp: the functions, the stand-ins (dynamic_call) and the tests'
table; nfs3hp.h: what they and native_render.cpp need of the port; tables.inc:
THRASH's slots and their stdcall sizes, and the ret N of each function called;
image.bin: nfs3.exe's sections, to start memory from."""
import io
import re
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT))
sys.path.insert(0, str(ROOT / "disasm"))
from disasm import disassembler  # noqa: E402
from disasm.codegen import fpu as fpu_codegen  # noqa: E402
import disassemble_nfs3hp as nfs3hp  # noqa: E402
from tools.game_checks.generate import PORT_CHANGES  # noqa: E402

# What the port's changes call (nfs3hp_main.cpp's), rcheck.cpp's stand-ins.
PORT_DECLARATIONS = """\
x86::reg32 mirrorAsMain(x86::reg32 kind);
void mirrorFollowsMain(win32::WinApplication* app);
x86::reg32 viewDistanceReduced(win32::WinApplication* app, x86::reg32 reduced);
x86::reg32 splitFarDistance(win32::WinApplication* app, x86::reg32 distance);
x86::reg32 fullFarDistance(win32::WinApplication* app, x86::reg32 distance);
x86::reg32 hudReferenceWidth(win32::WinApplication* app);
x86::reg32 hudReferenceHeight(win32::WinApplication* app);
void widescreenHudRect(win32::WinApplication* app, x86::reg32 slot);
void hudTablePanel(win32::WinApplication* app, x86::reg32 player, bool otherLayout);
x86::reg32 cabinRect(win32::WinApplication* app, x86::reg32 field);
bool fullCarDetail();
void loadingScreenFit(win32::WinApplication* app, x86::CPU& cpu, bool on);
bool dynamicFullscreenMode(x86::reg32 mode);
bool dynamicFullscreenRefresh(win32::WinApplication* app);
"""

IDIV = ("        cpu.edx_eax = x86::reg64(tmp / d);\n"
        "        cpu.edx = x86::reg32(tmp % d);\n")
IDIV_ARM64 = ("        cpu.edx_eax = x86::reg64(d ? tmp / d : 0);  /* arm64's sdiv by zero */\n"
              "        cpu.edx = x86::reg32(d ? tmp % d : tmp);  /* and msub after it */\n")


def main():
    out = Path(sys.argv[1]).resolve()
    out.mkdir(parents=True, exist_ok=True)
    game = disassembler.disassemble("nfs3hp", str(ROOT / "nfs3hp/nfs3.exe"), nfs3hp.DATA_SEGMENTS,
                                    nfs3hp.KNOWN_SUBROUTINES, nfs3hp.SPLIT_INSTRUCTIONS,
                                    nfs3hp.THREAD_ROUTINES, nfs3hp.THREAD_SEGMENTS, nfs3hp.MERGE_ROUTINES)
    fpu_codegen.EXACT = True  # as Application.write has the game's arithmetic
    # detailSettings no longer chooses by the machine's speed, as the game's code does.
    tested = {address: name.split("::")[1] for address, name in nfs3hp.RENDER_NATIVES.items()
              if address != 0x472d10}
    pieces = {s.get_entry_point(): s for s in game.subroutines}

    def code_of(address):
        f = io.StringIO()
        game.write_function(f, pieces[address], "sub_%x" % address, {}, nfs3hp.THREAD_SEGMENTS,
                            nfs3hp.SKIP_INSTRUCTIONS)
        return f.getvalue()

    code = []
    idivs = 0
    for address in sorted(tested):
        if address not in pieces:
            raise SystemExit("no function at %x" % address)
        text = code_of(address)
        if address in PORT_CHANGES:
            text = PORT_CHANGES[address](text)
        idivs += text.count(IDIV)
        text = text.replace(IDIV, IDIV_ARM64)
        opening = "void Application::sub_%x(" % address
        if text.count(opening) != 1:
            raise SystemExit("%x: opening changed" % address)
        code.append(text.replace(opening, "void Gen::gen_%x(" % address, 1))
    text = "".join(code)
    called = sorted(set(int(a, 16) for a in re.findall(r"\bsub_([0-9a-f]+)\(app", text)))

    with open(out / "game_functions.cpp", "w") as f:
        f.write('#include "nfs3hp.h"\n#include <lib/thread.h>\n\nnamespace nfs3hp\n{\n\n')
        f.write("struct Gen : Application\n{\n")
        for address in sorted(tested):
            f.write("    static void gen_%x(WinApplication* app, x86::CPU& cpu);\n" % address)
        f.write("};\n\n%s\n" % text)
        f.write("/* What the game's code calls by address: rcheck.cpp's stand-ins. */\n")
        for address in called:
            f.write("void Application::sub_%x(WinApplication* app, x86::CPU& cpu) { app->dynamic_call(0x%x, cpu); }\n"
                    % (address, address))
        f.write("\n}\n\n/* The tests: address, name, the game's code, the native. */\n")
        f.write("struct RenderTest { unsigned address; const char* name; void (*generated)(win32::WinApplication*, x86::CPU&);"
                " void (*native)(win32::WinApplication*, x86::CPU&); };\n")
        f.write("extern const RenderTest kRenderTests[] = {\n")
        for address in sorted(tested):
            f.write('    {0x%x, "%s", &nfs3hp::Gen::gen_%x, &nfs3hp::render::%s},\n'
                    % (address, tested[address], address, tested[address]))
        f.write("};\nextern const unsigned kRenderTestCount = %d;\n" % len(tested))

    with open(out / "nfs3hp.h", "w") as h:
        h.write("#ifndef NFS3HP_H_\n#define NFS3HP_H_\n#include <x86.h>\n#include <lib/winapp.h>\n"
                "#include <lib/memmap.h>\n\nnamespace nfs3hp\n{\n\n")
        h.write(PORT_DECLARATIONS)
        h.write("\nnamespace render\n{\n")
        for address in sorted(tested):
            h.write("void %s(win32::WinApplication* app, x86::CPU& cpu);  // 0x%x\n" % (tested[address], address))
        h.write("}\n\nclass Application : public win32::WinApplication\n{\npublic:\n")
        for address in called:
            h.write("    static void sub_%x(WinApplication* app, x86::CPU& cpu);\n" % address)
        h.write("};\n\n}\n\n#endif\n")

    # THRASH: the slots and their stdcall sizes, from nfs3.exe's names in order.
    exe = (ROOT / "nfs3hp/nfs3.exe").read_bytes()
    names = [m.group().decode() for m in re.finditer(rb"_THRASH_\w+@\d+", exe)]
    slots_src = (ROOT / "src/nfs3hp/native_game.cpp").read_text(encoding="utf-8")
    table = re.search(r"kThrashFunctions\[\] = \{(.*?)\};", slots_src, re.S).group(1)
    pairs = [(int(a, 16), int(b, 16)) for a, b in re.findall(r"\{0x([0-9a-f]+), 0x([0-9a-f]+)\}", table)]
    assert len(pairs) == len(names), (len(pairs), len(names))
    with open(out / "tables.inc", "w") as f:
        f.write("/* THRASH's slots: the slot, its stdcall bytes, its name. */\n")
        f.write("const struct { unsigned slot, bytes; const char* name; } kThrashSlots[] = {\n")
        for (name_at, slot), name in zip(sorted(pairs), names):
            f.write('    {0x%x, %s, "%s"},\n' % (slot, name.rsplit("@", 1)[1], name))
        f.write("};\n")
        # The ret N of what is called, as the game's code has it.
        f.write("/* Each called function's ret N. */\nconst struct { unsigned address, pops; } kPops[] = {\n")
        for address in called:
            if address in pieces:
                rets = [int(n) for n in re.findall(r"cpu\.esp \+= 4\+(\d+) /\*", code_of(address))]
                if max(rets, default=0):
                    f.write("    {0x%x, %d},\n" % (address, max(rets)))
        f.write("    {0, 0},\n};\n")

    # The image: nfs3.exe's sections where they load.
    pe = struct.unpack_from("<I", exe, 0x3c)[0]
    count = struct.unpack_from("<H", exe, pe + 6)[0]
    optional = struct.unpack_from("<H", exe, pe + 20)[0]
    base = struct.unpack_from("<I", exe, pe + 24 + 28)[0]
    image = bytearray(0xa40000 - 0x400000)
    o = pe + 24 + optional
    for i in range(count):
        name, vsize, va, rsize, raw = struct.unpack_from("<8sIIII", exe, o + 40 * i)
        data = exe[raw:raw + rsize]
        at = base + va - 0x400000
        image[at:at + len(data)] = data
    (out / "image.bin").write_bytes(bytes(image))
    print("%d functions, %d idiv, stand-ins: %s" % (len(tested), idivs, " ".join("%x" % a for a in called)))


if __name__ == "__main__":
    main()
