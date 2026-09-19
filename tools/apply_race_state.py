"""Whether the player is driving; never patch input EXEs.

The on-screen controls need two layouts -- one to drive with, one to walk
through menus with -- and the game has to say which, rather than the player
pressing a button for it.

A race is one call: sub_4a3690 sets a race up with sub_4a3400, runs it, and
takes it down again with sub_4a35d0, the only two places either is called from.
Both are marked here, so nfs3hp::raceRunning() answers for the stretch between
them (src/nfs3hp/nfs3hp_main.cpp).  Whether the menu is open over that race is a
separate question and needs no mark: sub_4bc680, which opens and closes it, keeps
[0x7a3d10] set for as long as it is up.
"""
from pathlib import Path

HEADER = "// Port (tools/apply_race_state.py): defined in nfs3hp_main.cpp.\n"

SITES = [
    ("nfs3hp.24.cpp", "004a3409  ba01000000",
     "    cpu.edx = 1 /*0x1*/;",
     "    cpu.edx = 1 /*0x1*/;\n"
     "    raceRunning(true); /* port: sub_4a3400 sets a race up */",
     "void raceRunning(bool running);\n"),
    ("nfs3hp.24.cpp", "004a35d7  89c6",
     "    cpu.esi = cpu.eax;",
     "    cpu.esi = cpu.eax;\n"
     "    raceRunning(false); /* port: sub_4a35d0 takes it down again */",
     ""),
]

NAMESPACE = "namespace nfs3hp\n{\n"


def apply(root):
    for name in dict.fromkeys(site[0] for site in SITES):
        path = root / "src/nfs3hp/disassembly" / name
        text = path.read_text(newline="")
        eol = "\r\n" if text.count("\r\n") * 2 > text.count("\n") else "\n"

        def native(s):
            return eol.join(s.split("\n"))

        sites = [site for site in SITES if site[0] == name]
        declarations = native(HEADER + "".join(site[4] for site in sites))
        if declarations not in text:
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

        path.write_text(text, newline="")


if __name__ == "__main__":
    apply(Path(__file__).resolve().parents[1])
