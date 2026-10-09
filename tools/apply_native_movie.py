"""The movies native: the MAD player, decoder and display; never patch input EXEs.

decomp/mad has EA's MAD player, decoder and software display of NFS III in C,
checked against nfs3.exe; src/nfs3hp/native_movie.cpp wires them in, entered
from the top of the generated functions the way tools/apply_native_vertices.py
enters the vertex loops: when the native one says no, the generated code runs.

sub_495bc0 opens with tools/apply_movie_tap.py's MovieSession, so its hook goes
after that: the movie is marked playing, and a tap can end it, either way.
"""
from pathlib import Path

from tools.apply_native_vertices import apply as apply_sites

HEADER = "// Port (tools/apply_native_movie.py): defined in native_movie.cpp.\n"

# (module, function, native stand-in)
SITES = [
    # The decoder's two entries: a frame begun, a macroblock.
    ("nfs3hp", "sub_4f3560", "madBeginFrame"),
    ("nfs3hp", "sub_4f3600", "madMacroblock"),
    # A frame on the screen, in full colour through the renderer.
    ("nfs3hp", "sub_4df2b0", "madShowFrame"),
]

PLAYER_OPENING = ("void Application::sub_495bc0(WinApplication* __restrict app, x86::CPU& cpu_)\n"
                  "{\n"
                  "  MovieSession movieSession;\n"
                  "  x86::Local cpu(cpu_);\n"
                  "  NFS2_USE(cpu);\n"
                  "  NFS2_USE(app);\n")
PLAYER_CALL = ("    if (madPlay(app, cpu.sync())) /* port: native (tools/apply_native_movie.py) */\n"
               "    {\n"
               "        cpu.reload();\n"
               "        return;\n"
               "    }\n")
PLAYER_DECLARATION = HEADER + "bool madPlay(win32::WinApplication* app, x86::CPU& cpu);\n"


def apply_player(root):
    path = root / "src/nfs3hp/disassembly/nfs3hp.21.cpp"
    with path.open("r", newline="") as source:
        text = source.read()
    eol = "\r\n" if text.count("\r\n") * 2 > text.count("\n") else "\n"

    def local(s):
        return eol.join(s.split("\n"))

    opening = local(PLAYER_OPENING)
    if text.count(opening) != 1:
        raise RuntimeError("sub_495bc0: opening changed (tools/apply_movie_tap.py first)")
    call = local(PLAYER_CALL)
    if opening + call not in text:
        text = text.replace(opening, opening + call, 1)
    declaration = local(PLAYER_DECLARATION)
    if declaration not in text:
        namespace = local("namespace nfs3hp\n{\n")
        if text.count(namespace) != 1:
            raise RuntimeError("nfs3hp.21.cpp: namespace opening changed")
        text = text.replace(namespace, namespace + declaration, 1)
    with path.open("w", newline="") as out:
        out.write(text)


def apply(root):
    apply_sites(root, SITES, HEADER, "tools/apply_native_movie.py")
    apply_player(root)


if __name__ == "__main__":
    apply(Path(__file__).resolve().parents[1])
