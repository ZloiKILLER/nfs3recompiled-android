"""A track is played from its own file; never patch input EXEs.

Starting a race, the game copies the whole of the track it is about to play --
gamedata/audio/pc/<track><rock|tech>.mus, 7 to 14 MB -- into a file of its own,
<temp>temp.mus, and streams from that copy: sub_410030 does the copying, half a
megabyte at a time, turning the loading bar as it goes; sub_4107c0 queues the
copy on the music stream; sub_40fed0 deletes it afterwards.  In 1998 that
bought a game running from a CD an uninterrupted read from the hard disk.  On a
phone the CD is long gone and what is left is those megabytes written to flash
at every race start, and the seconds of loading spent writing them.

So the copy is not made and the stream is given the track itself, which is what
the Modern Patch does as well:

- sub_410030 keeps everything it works out -- which track, rock or tech, and
  the state the rest of the game reads from it -- and stops where the copying
  begins, before it opens either file.  The half-megabyte buffer it copies
  through is not allocated either, so nothing is left to free.
- sub_4107c0 builds the track's own name to open its index file
  (<audio><track><rock|tech> and ".map", or ".lin"), and then builds the copy's
  name over it for the stream.  nfs3hp::musicIndexOpened keeps the first as the
  game opens it and nfs3hp::musicStreamFile puts it back in place of the second,
  with ".mus" for the index's extension: the track's own name, spelled by the
  game itself rather than by us.

sub_40ff20, which reserved 15 MB for the copy, is never called in this
executable and is left as it is.  The delete stays: it still clears a temp.mus
left behind by an older build.
"""
from pathlib import Path

HEADER = "// Port (tools/apply_music_stream.py): defined in nfs3hp_main.cpp.\n"

# (generated file, address and bytes of the instruction, original code, patched
# code, the declaration the patched code needs)
SITES = [
    # sub_410030: the buffer the copy would have gone through.
    ("nfs3hp.2.cpp", "00410089  e892150d00",
     "    cpu.esp -= 4;\n"
     "    sub_4e1620(app, cpu);",
     "    cpu.eax = 0; /* port: no copy buffer, nothing is copied */",
     ""),
    # sub_410030: everything from here on opens the two files and copies.
    ("nfs3hp.2.cpp", "00410214  6834277a00",
     "    app->getMemory<x86::reg32>(cpu.esp-4) = 8005428 /*0x7a2734*/;",
     "    goto L_0x0041031c; /* port: the track is played where it lies, not copied */\n"
     "    app->getMemory<x86::reg32>(cpu.esp-4) = 8005428 /*0x7a2734*/;",
     ""),
    # sub_4107c0: the track's index file, opened by the name the game built.
    ("nfs3hp.3.cpp", "00410908  e8b3050d00",
     "    cpu.esp -= 4;\n"
     "    sub_4e0ec0(app, cpu);",
     "    musicIndexOpened(app, cpu.eax); /* port: the track's name, as the game spells it */\n"
     "    cpu.esp -= 4;\n"
     "    sub_4e0ec0(app, cpu);",
     "void musicIndexOpened(win32::WinApplication* app, x86::reg32 path);\n"),
    # sub_4107c0: the name queued on the music stream, the copy's until now.
    ("nfs3hp.3.cpp", "00410965  e8a2650d00",
     "    cpu.esp -= 4;\n"
     "    sub_4e6f0c(app, cpu);",
     "    musicStreamFile(app, cpu.edx); /* port: the track itself, not a copy of it */\n"
     "    cpu.esp -= 4;\n"
     "    sub_4e6f0c(app, cpu);",
     "void musicStreamFile(win32::WinApplication* app, x86::reg32 path);\n"),
]

NAMESPACE = "namespace nfs3hp\n{\n"


def apply(root):
    for name in dict.fromkeys(site[0] for site in SITES):
        path = root / "src/nfs3hp/disassembly" / name
        with path.open("r", newline="") as source:
            text = source.read()
        eol = "\r\n" if text.count("\r\n") * 2 > text.count("\n") else "\n"

        def native(s):
            return eol.join(s.split("\n"))

        sites = [site for site in SITES if site[0] == name]
        declarations = native(HEADER + "".join(site[4] for site in sites))
        if declarations.strip() and declarations not in text:
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

        with path.open("w", newline="") as out:
            out.write(text)


if __name__ == "__main__":
    apply(Path(__file__).resolve().parents[1])
