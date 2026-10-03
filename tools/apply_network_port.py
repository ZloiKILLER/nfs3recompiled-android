"""The TCP/IP port is the player's, 9803 unless they choose 1030; never patch input EXEs.

The original game hosts and joins a TCP/IP race on port 1030 (0x406), written
into three places: sub_49e9d0 opens the host's transport with it (0x49ea09),
sub_49ea70 connects a joining player to it (0x49ea8b), and sub_49e5f0 gives it
to the transport a race is set up on (0x49e70c).  The Modern Patch moved to
9803, because Windows often holds 1030 for itself, and kept 1030 as a choice
for playing the original; every player of one race has to use the same.  So
the port does as the Modern Patch does: 9803 by default, 1030 when the launcher
says so (nfs3hp::networkPort, NFS_NET_PORT).
"""
from pathlib import Path

HEADER = "// Port (tools/apply_network_port.py): defined in nfs3hp_main.cpp.\n"

DECLARATION = "x86::reg32 networkPort();\n"

NOTE = "/* port: 9803 or 1030, the launcher's (tools/apply_network_port.py) */"

# (generated file, address and bytes of the instruction, original code, patched code)
SITES = [
    # sub_49e5f0: the port of the transport a race is set up on.
    ("nfs3hp.23.cpp", "0049e70c  b806040000",
     "    cpu.eax = 1030 /*0x406*/;",
     "    cpu.eax = networkPort(); " + NOTE),
    # sub_49e9d0: the host opens its transport on it.
    ("nfs3hp.23.cpp", "0049ea09  6806040000",
     "    app->getMemory<x86::reg32>(cpu.esp-4) = 1030 /*0x406*/;",
     "    app->getMemory<x86::reg32>(cpu.esp-4) = networkPort(); " + NOTE),
    # sub_49ea70: a joining player connects to it.
    ("nfs3hp.23.cpp", "0049ea8b  bb06040000",
     "    cpu.ebx = 1030 /*0x406*/;",
     "    cpu.ebx = networkPort(); " + NOTE),
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
