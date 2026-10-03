"""Let an Android tap leave the original MAD playback through its cleanup path.

The only playback call (sub_495bc0) is wrapped by a scope guard so early
termination also clears the playing flag. The skip check is at the player's
frame loop and branches to its existing resource cleanup at 0x4960b3.
Run after regenerating nfs3hp.21.cpp.
"""
from pathlib import Path


def apply(root: Path) -> None:
    path = root / "src/nfs3hp/disassembly/nfs3hp.21.cpp"
    data = path.read_bytes()
    eol = b"\r\n" if data.count(b"\r\n") > data.count(b"\n") // 2 else b"\n"

    def put(after: bytes, addition: bytes) -> None:
        nonlocal data
        after = after.replace(b"\n", eol)
        addition = addition.replace(b"\n", eol)
        if addition in data:
            return
        if data.count(after) != 1:
            raise RuntimeError(f"expected one anchor in {path}: {after!r}")
        data = data.replace(after, after + addition, 1)

    put(b"namespace nfs3hp\n{\n",
        b"// Port (tools/apply_movie_tap.py): the UI thread requests a MAD exit.\n"
        b"void moviePlaying(bool playing);\n"
        b"bool movieSkipRequested();\n"
        b"struct MovieSession {\n"
        b"    MovieSession() { moviePlaying(true); }\n"
        b"    ~MovieSession() { moviePlaying(false); }\n"
        b"};\n")
    put(b"void Application::sub_495bc0(WinApplication* __restrict app, x86::CPU& cpu_)\n{\n",
        b"  MovieSession movieSession;\n")
    old_skip = b"    if (movieSkipRequested()) goto L_0x004960b3; /* port: tap skips MAD */\n".replace(b"\n", eol)
    if old_skip in data:
        data = data.replace(old_skip, b"", 1)
    put(b"L_0x00495e72:\n",
        b"    if (movieSkipRequested()) {\n"
        b"        /* Same abort marker as the player's event type 2 (Escape). */\n"
        b"        auto abort = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44));\n"
        b"        app->getMemory<x86::reg32>(abort) = 1;\n"
        b"        goto L_0x004960b3;\n"
        b"    }\n")
    path.write_bytes(data)


if __name__ == "__main__":
    apply(Path(__file__).resolve().parents[1])
