"""A new player's settings are the phone's; never patch input EXEs.

The retail disc has no settings file: FEDATA/CONFIG holds PLATCFG.DAT and
nothing else, and the game makes config.dat itself the first time it runs.  An
import of an installed copy brings one along, and the launcher writes what a
new game on a phone starts with into it (ControlProfile.writeImportDefaults):
the control set, View Distance at Full, a 1280-wide screen for races, the HUD
arranged for a phone and the cop's map as a minimap.  Data copied straight off
the disc brings nothing to write into, so it would start with the settings of
a 1998 PC instead.

The game's own start is where that is put right:

- sub_472980 loads the settings.  Finding no file, or one it will not take
  (the wrong size, the wrong first word, older than the exe), it starts them
  over with sub_4723f0 -- which reads the HUD from DASHHUD/DEF.POS as well --
  and sets [0x67900c].
- sub_4730d0, the front end, sees [0x67900c] the first time it comes up
  ([0x55b03c]) and calls sub_472d10, which chooses the graphics that suit the
  machine: View Distance and the rest of 0x6fbc10..0x6fbc4c.

Right after that the phone's settings go over the game's
(nfs3hp::firstSettings, NFS3Activity.onFirstSettings), and the game takes them
up as it would a loaded file: sub_43c040 reads from the new bindings whether
each player steers and works the pedals digitally -- sub_472b60 did so from the
game's own bindings when the settings were made -- and sub_472820 saves the
file, as the game does each time it leaves the front end.  So config.dat is
there from the first minute, and every later start loads it like any other.
"""
from pathlib import Path

HEADER = "// Port (tools/apply_first_settings.py): defined in nfs3hp_main.cpp.\n"

DECLARATION = "bool firstSettings(win32::WinApplication* app);\n"

# (generated file, address and bytes of the instruction, original code, patched code)
SITES = [
    # sub_4730d0: the game has just chosen what suits the machine (sub_472d10),
    # on the first front end after it started its settings over.
    ("nfs3hp.18.cpp", "00473195  e876fbffff",
     "    sub_472d10(app, cpu);\n"
     "    if (cpu.terminate) return;",
     "    sub_472d10(app, cpu);\n"
     "    if (cpu.terminate) return;\n"
     "    if (firstSettings(app)) /* port: the phone's settings over the new player's */\n"
     "    {\n"
     "        const x86::reg32 kept = cpu.eax;\n"
     "        cpu.esp -= 4;\n"
     "        sub_43c040(app, cpu); /* the game takes up the new bindings */\n"
     "        if (cpu.terminate) return;\n"
     "        cpu.esp -= 4;\n"
     "        sub_472820(app, cpu); /* and saves them, as it does leaving the front end */\n"
     "        if (cpu.terminate) return;\n"
     "        cpu.eax = kept;\n"
     "    }"),
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

        declaration = native(HEADER + DECLARATION)
        if declaration not in text:
            namespace = native(NAMESPACE)
            if text.count(namespace) != 1:
                raise RuntimeError("%s: namespace opening changed" % name)
            text = text.replace(namespace, namespace + declaration, 1)

        for _, instruction, old, new in (site for site in SITES if site[0] == name):
            old, new = native(old), native(new)
            anchor = "    // " + instruction + "  "
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
