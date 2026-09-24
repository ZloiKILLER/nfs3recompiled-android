"""The phone's keyboard follows the game's own; never patch input EXEs.

A player renames themselves on the car screen, and the game puts up a little
dialog for it: a line of text with Done and Cancel under it.  Nothing in that
dialog takes focus -- on a keyboard you simply type, and the two buttons are
all there is to move between -- so there is no field to notice being tapped and
no state to read off the screen.  What there is, is the dialog itself:

- sub_428020 puts the dialog up: it resolves CEB_DONE and CEB_CANCEL and hands
  each of them the key handler that will read the typing (sub_427e10 and
  sub_427ea0 land at item + 0x30).  That is the moment the game starts waiting
  for characters, and the moment the phone's keyboard belongs on the screen.
- sub_427d00 takes it down again when the name is accepted, and sub_427ea0 when
  it is cancelled.  Both end the waiting, and the keyboard goes away with it.

nfs3hp::textEntry is told each way round; what it does with it, and the one
safety net -- a keyboard left up when a race starts goes down -- is in
nfs3hp_main.cpp.  The alternative was to watch the car screen's own state
([0x553048] and the two beside it), and a session on the phone showed why that
is no good: the numbers never moved while the dialog was up.
"""
from pathlib import Path

HEADER = "// Port (tools/apply_text_entry.py): defined in nfs3hp_main.cpp.\n"

DECLARATION = "void textEntry(bool taking);\n"

# (generated file, address and bytes of the instruction, original code, patched
# code, the declaration the patched code needs)
SITES = [
    # sub_428020: the dialog's buttons are given their key handlers.
    ("nfs3hp.6.cpp", "00428020  53",
     "    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;",
     "    textEntry(true); /* port: the game is waiting for a name */\n"
     "    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;",
     DECLARATION),
    # sub_427d00: the name is taken and the dialog goes.
    ("nfs3hp.6.cpp", "00427d00  53",
     "    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;",
     "    textEntry(false); /* port: the name is taken */\n"
     "    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;",
     ""),
    # sub_427ea0: Cancel, which puts the old name back and closes it too.
    ("nfs3hp.6.cpp", "00427ea0  51",
     "    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;",
     "    textEntry(false); /* port: the name is let go */\n"
     "    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;",
     ""),
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
        if declarations not in text:
            namespace = native(NAMESPACE)
            if text.count(namespace) != 1:
                raise RuntimeError("%s: namespace opening changed" % name)
            text = text.replace(namespace, namespace + declarations, 1)

        for _, instruction, old, new, _ in sites:
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
