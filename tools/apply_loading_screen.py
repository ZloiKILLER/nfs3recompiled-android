"""The loading screen at 4:3 on a wide screen; never patch input EXEs.

A race is loaded behind a picture of the car (sub_494cb0, called by sub_494fa0
from the race's setup, sub_4a3400): twenty by fifteen tiles of 32 texels, the
line "Press Escape Key To Abort" and, from sub_495230 as each part of the race
comes in, a progress bar.  All of it is laid out for 640x480 and scaled to the
screen by its width and its height, so on 16:9 the picture came out stretched
sideways.  The Modern Patch shows it at 4:3 in the middle of the screen, and so
does this: sub_494cb0 starts by blacking out the whole picture and has
everything drawn from then on squeezed sideways into the 4:3 rectangle in its
middle (nfs3hp::loadingScreenFit, glide2x::fitFourThree), tiles, text and bar
alike, until the setup is done (0x4a35c4) -- or the race is left, which is how
an aborted load ends too (nfs3hp::raceRunning).  The flag the game keeps for
its loading screen, [0x79f288], is no help there: only an abort clears it.
"""
from pathlib import Path

HEADER = "// Port (tools/apply_loading_screen.py): defined in nfs3hp_main.cpp.\n"

DECLARATION = "void loadingScreenFit(win32::WinApplication* app, x86::CPU& cpu, bool on);\n"

# (generated file, address and bytes of the instruction, original code, patched code)
SITES = [
    # sub_494cb0: the loading picture, from its first instruction.
    ("nfs3hp.21.cpp", "00494cb0  53",
     "    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;",
     "    loadingScreenFit(app, cpu, true); /* port: the loading screen at 4:3 */\n"
     "    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;"),
    # sub_4a3400: the race set up and loaded, the way out of it that is not an
    # abort.
    ("nfs3hp.24.cpp", "004a35c4  89ec",
     "    cpu.esp = cpu.ebp;",
     "    loadingScreenFit(app, cpu, false); /* port: loaded, the whole screen again */\n"
     "    cpu.esp = cpu.ebp;"),
]

NAMESPACE = "namespace nfs3hp\n{\n"


def apply(root):
    for name in dict.fromkeys(site[0] for site in SITES):
        path = root / "src/nfs3hp/disassembly" / name
        text = path.read_text(newline="")
        eol = "\r\n" if text.count("\r\n") * 2 > text.count("\n") else "\n"

        def native(s):
            return eol.join(s.split("\n"))

        declarations = native(HEADER + DECLARATION)
        if declarations not in text:
            namespace = native(NAMESPACE)
            if text.count(namespace) != 1:
                raise RuntimeError("%s: namespace opening changed" % name)
            text = text.replace(namespace, namespace + declarations, 1)

        for _, instruction, old, new in [site for site in SITES if site[0] == name]:
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
