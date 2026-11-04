"""Hooks that watch the game's code at the top of a few of its functions; never patch input EXEs.

What this put in front of the game's hottest drawing functions -- the vertex
and polygon loops of src/nfs3hp/native_vertices.cpp, entered from the top of
their generated code since 2026-09-25 -- is native whole since 2026-10-27
(disassemble_nfs3hp.py's VERTICES_NATIVES): no generated code is left behind
them, and tools/game_checks/vertices holds them against the game's code, made
afresh, on the desktop.

What is left here does no work of its own: each hook looks at the call and
hands it on to the generated code (it returns false).
"""
from pathlib import Path

HEADER = "// Port (tools/apply_native_vertices.py): defined in native_vertices.cpp.\n"

# (module, function, hook)
SITES = [
    # How much of Render_GetTm's buffer a view pass used, read as the next one
    # starts it over, for the [VIEW] trace.
    ("nfs3hp", "sub_4bbd70", "arenaReset"),
    # An opponent's lane votes as its driver acts on them (NFS_CAR_TRACE), to
    # find why the opponents pile up at Aquatica's tunnel.
    # EA's comm library into the log (NFS_NET_TRACE) -- the IPX transport
    # opening, the library's event log and packet_sendpacket -- to see where an
    # IPX race stops.  Its message printer, Error, is native
    # (src/nfs3hp/native_game.cpp) and calls commMessageTrace itself.
    ("nfs3hp", "sub_4f5c10", "commOpenTrace"),
    ("nfs3hp", "sub_512180", "commEventTrace"),
    ("nfs3hp", "sub_51d810", "commSendTrace"),
]

# How each module's generated functions open, and where a declaration goes.
OPENING = {
    "nfs3hp": ("void Application::%s(WinApplication* __restrict app, x86::CPU& cpu_)\n"
               "{\n"
               "  x86::Local cpu(cpu_);\n"
               "  NFS2_USE(cpu);\n"
               "  NFS2_USE(app);\n"),
    "voodoo2a": ("void %s(win32::WinApplication* __restrict app, x86::CPU& cpu_)\n"
                 "{\n"
                 "  x86::Local cpu(cpu_);\n"
                 "  NFS2_USE(cpu);\n"
                 "  NFS2_USE(app);\n"),
    "eacsnd": ("void %s(win32::WinApplication* __restrict app, x86::CPU& cpu_)\n"
               "{\n"
               "  x86::Local cpu(cpu_);\n"
               "  NFS2_USE(cpu);\n"
               "  NFS2_USE(app);\n"),
}

GUARD = "  MovieSession movieSession;\n"

CALL = ("    if (%s(app, cpu.sync())) /* port: native (tools/apply_native_vertices.py) */\n"
        "    {\n"
        "        cpu.reload();\n"
        "        return;\n"
        "    }\n")

# A function too small for x86::Local works on the CPU itself.
PLAIN_OPENING = {
    "nfs3hp": ("void Application::%s(WinApplication* app, x86::CPU& cpu)\n"
               "{\n"
               "  NFS2_USE(cpu);\n"
               "  NFS2_USE(app);\n"),
    "voodoo2a": ("void %s(win32::WinApplication* app, x86::CPU& cpu)\n"
                 "{\n"
                 "  NFS2_USE(cpu);\n"
                 "  NFS2_USE(app);\n"),
    "eacsnd": ("void %s(win32::WinApplication* app, x86::CPU& cpu)\n"
               "{\n"
               "  NFS2_USE(cpu);\n"
               "  NFS2_USE(app);\n"),
}

PLAIN_CALL = ("    if (%s(app, cpu)) /* port: native (tools/apply_native_vertices.py) */\n"
              "    {\n"
              "        return;\n"
              "    }\n")

# The natives live in namespace nfs3hp: declared inside it in nfs3hp's files,
# in a namespace block of their own ahead of voodoo2a's and eacsnd's.
NAMESPACE = {"nfs3hp": "namespace nfs3hp\n{\n", "voodoo2a": "namespace voodoo2a\n{\n",
             "eacsnd": "namespace eacsnd\n{\n"}
DECLARATION = {
    "nfs3hp": "bool %s(win32::WinApplication* app, x86::CPU& cpu);\n",
    "voodoo2a": "namespace nfs3hp\n{\nbool %s(win32::WinApplication* app, x86::CPU& cpu);\n}\n\n",
    "eacsnd": "namespace nfs3hp\n{\nbool %s(win32::WinApplication* app, x86::CPU& cpu);\n}\n\n",
}
CALLEE = {"nfs3hp": "%s", "voodoo2a": "nfs3hp::%s", "eacsnd": "nfs3hp::%s"}


def apply(root, sites=SITES, header=HEADER, marker="tools/apply_native_vertices.py"):
    for module, function, native in sites:
        sources = sorted((root / "src/nfs3hp/disassembly").glob(module + ".*.cpp"))
        found = False
        for path in sources:
            with path.open("r", newline="") as source:
                text = source.read()
            eol = "\r\n" if text.count("\r\n") * 2 > text.count("\n") else "\n"

            def local(s):
                return eol.join(s.split("\n"))

            def marked(s):
                return s.replace("tools/apply_native_vertices.py", marker)

            opening = local(OPENING[module] % function)
            call = local(marked(CALL) % (CALLEE[module] % native))
            if opening not in text:
                # sub_495bc0 opens with tools/apply_movie_tap.py's guard, which
                # then holds for the native as well.
                opening = local((OPENING[module] % function).replace("{\n", "{\n" + GUARD, 1))
            if opening not in text:
                opening = local(PLAIN_OPENING[module] % function)
                call = local(marked(PLAIN_CALL) % (CALLEE[module] % native))
                if opening not in text:
                    continue
            found = True
            if opening + call not in text:
                text = text.replace(opening, opening + call, 1)
            declaration = local(header + DECLARATION[module] % native)
            if declaration not in text:
                namespace = local(NAMESPACE[module])
                if text.count(namespace) != 1:
                    raise RuntimeError("%s: namespace opening changed" % path.name)
                if module == "nfs3hp":
                    text = text.replace(namespace, namespace + declaration, 1)
                else:
                    text = text.replace(namespace, declaration + namespace, 1)
            with path.open("w", newline="") as out:
                out.write(text)
            break
        if not found:
            raise RuntimeError("no generated %s with the expected opening" % function)


if __name__ == "__main__":
    apply(Path(__file__).resolve().parents[1])
