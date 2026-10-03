"""RaceNet, Modem and Serial taken off the connection screen; never patch input EXEs.

MENUS/CONNECT.MNU lists the ways to race another machine: RaceNet, IPX, TCP/IP,
Modem and Serial, in that order, one tab under another.  A phone has neither a
modem nor a serial port, and RaceNet was EA's own server, long gone.  The
screen is set up by sub_4405c0, which looks up each tab by its codelink
(sub_442a40) and gives it its click handler at +0x64 -- racenet sub_43fd10,
modem sub_43ff50, serial sub_43fee0.  Instead those three are hidden the way
the game hides one of its own items (`hidethis`, bit 0x10 of the flags byte at
+5; see tools/apply_hide_download.py), so they are neither drawn nor reached by
the keys.  Modem and Serial are the last two tabs of the column; RaceNet is
the first, so IPX and TCP/IP move up one place each (y, the word at +8, in
640x480) and the column starts where it did.  The move is made once: when the
screen is set up again with the items still in place, IPX already stands where
RaceNet does.  The menu files themselves come with the player's game data and
stay as they are.
"""
from pathlib import Path

HIDE = ("    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5) /* 0x5 */) |= x86::reg8(16 /*0x10*/); "
        "/* port: %s hidden, as hidethis is (tools/apply_hide_connections.py) */")

HIDE_RACENET = """\
    {
        /* port: RaceNet hidden, as hidethis is, and IPX and TCP/IP moved up
           into its place (tools/apply_hide_connections.py) */
        const x86::reg32 raceNet = cpu.eax;
        app->getMemory<x86::reg8>(raceNet + x86::reg32(5)) |= x86::reg8(16 /*0x10*/);
        cpu.edx = 5470464 /*0x537900 "ipx"*/;
        cpu.eax = cpu.ecx;
        cpu.esp -= 4;
        sub_442a40(app, cpu.sync());
        cpu.reload();
        if (cpu.terminate) return;
        const x86::reg32 ipx = cpu.eax;
        cpu.edx = 5470468 /*0x537904 "tcp"*/;
        cpu.eax = cpu.ecx;
        cpu.esp -= 4;
        sub_442a40(app, cpu.sync());
        cpu.reload();
        if (cpu.terminate) return;
        const x86::reg32 tcp = cpu.eax;
        if (ipx != 0 && tcp != 0
            && app->getMemory<x86::reg16>(ipx + x86::reg32(8)) != app->getMemory<x86::reg16>(raceNet + x86::reg32(8)))
        {
            app->getMemory<x86::reg16>(tcp + x86::reg32(8)) = app->getMemory<x86::reg16>(ipx + x86::reg32(8));
            app->getMemory<x86::reg16>(ipx + x86::reg32(8)) = app->getMemory<x86::reg16>(raceNet + x86::reg32(8));
        }
    }"""

# (generated file, address and bytes of the instruction, original code, patched code)
SITES = [
    # sub_4405c0: the RaceNet tab found; hidden rather than given its handler,
    # and the two tabs under it moved up.  eax and edx are set afresh by the
    # next lookup, and ecx -- the menu -- is kept by sub_442a40.
    ("nfs3hp.11.cpp", "004406d7  c7406410fd4300",
     "    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(100) /* 0x64 */) = 4455696 /*0x43fd10*/;",
     HIDE_RACENET),
    # sub_4405c0: the Modem tab found; hidden rather than given its handler.
    ("nfs3hp.11.cpp", "00440705  c7406450ff4300",
     "    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(100) /* 0x64 */) = 4456272 /*0x43ff50*/;",
     HIDE % "Modem"),
    # sub_4405c0: the Serial tab, likewise.
    ("nfs3hp.11.cpp", "0044071c  c74064e0fe4300",
     "    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(100) /* 0x64 */) = 4456160 /*0x43fee0*/;",
     HIDE % "Serial"),
]


def apply(root):
    for name in dict.fromkeys(site[0] for site in SITES):
        path = root / "src/nfs3hp/disassembly" / name
        with path.open("r", newline="") as source:
            text = source.read()
        eol = "\r\n" if text.count("\r\n") * 2 > text.count("\n") else "\n"

        def native(s):
            return eol.join(s.split("\n"))

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

        with path.open("w", newline="") as out:
            out.write(text)


if __name__ == "__main__":
    apply(Path(__file__).resolve().parents[1])
