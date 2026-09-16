"""Car detail patches applied after code generation; never patch input EXEs.

Three things the original decides in code rather than in data, each matched by
the address of its own instruction:

- Texture size.  sub_4b7d30 loads a car's texture at 256x256 for the local
  player's car and at 128x128 for every other one: opponents, traffic, cops and
  the second player in split screen (`mov ebx, 0x80` at 0x4b7d5b).  With
  NFS_CAR_DETAIL_FULL on, the default, every car gets the player's size.
- The transform buffer behind Render_GetTm.  sub_4b56e0 sizes it at 0xc3400
  bytes; it starts over for every 3D pass and simply leaves out whatever does
  not fit.  With every car on its detailed model, in both split-screen views,
  it gets four times that.
- Wheel spin.  sub_4b9140 and sub_4b9280 turn the wheels by the frames elapsed
  since [0x55fe28], and sub_4b90e0 moves the body only while that stamp is
  behind the frame counter.  The stamp is a single global written at the end
  of every view pass, so a car first drawn in a later pass of the same frame --
  the second split-screen view, the rear-view mirror -- saw no time pass and
  its wheels stood still.  Each of the three reads goes through
  nfs3hp::carWheelStamp instead, which keeps the stamp per car
  (src/nfs3hp/nfs3hp_main.cpp).
"""
from pathlib import Path

DECLARATIONS = (
    "// Port (tools/apply_car_detail.py): defined in nfs3hp_main.cpp.\n"
    "bool fullCarDetail();\n"
    "x86::reg32 carWheelStamp(win32::WinApplication* app, x86::reg32 car, x86::reg32 which);\n"
)

STAMP = "app->getMemory<x86::reg32>(x86::reg32(5635624) /* 0x55fe28 */);"

# (generated file, address and bytes of the instruction, original line, patched line)
SITES = [
    ("nfs3hp.25.cpp", "004b7d5b  bb80000000",
     "    cpu.ebx = 128 /*0x80*/;",
     "    cpu.ebx = fullCarDetail() ? 256u : 128u; /* port: every car at the player's texture size */"),
    ("nfs3hp.25.cpp", "004b56e5  ba00340c00",
     "    cpu.edx = 799744 /*0xc3400*/;",
     "    cpu.edx = 4u * 799744u; /* port: four times the transform buffer */"),
    ("nfs3hp.25.cpp", "004b56ea  c70000340c00",
     "    app->getMemory<x86::reg32>(cpu.eax) = 799744 /*0xc3400*/;",
     "    app->getMemory<x86::reg32>(cpu.eax) = 4u * 799744u; /* port: four times the transform buffer */"),
    ("nfs3hp.26.cpp", "004b90e7  8b1d28fe5500",
     "    cpu.ebx = " + STAMP,
     "    cpu.ebx = carWheelStamp(app, cpu.ecx, cpu.edx ? 3u : 2u); /* port: per-car stamp, was [0x55fe28] */"),
    ("nfs3hp.26.cpp", "004b9157  8b3528fe5500",
     "    cpu.esi = " + STAMP,
     "    cpu.esi = carWheelStamp(app, cpu.ecx, 0u); /* port: per-car stamp, was [0x55fe28] */"),
    ("nfs3hp.26.cpp", "004b9296  8b1d28fe5500",
     "    cpu.ebx = " + STAMP,
     "    cpu.ebx = carWheelStamp(app, cpu.ecx, 1u); /* port: per-car stamp, was [0x55fe28] */"),
]

NAMESPACE = "namespace nfs3hp\n{\n"


def apply(root):
    for name in dict.fromkeys(site[0] for site in SITES):
        path = root / "src/nfs3hp/disassembly" / name
        text = path.read_text(newline="")
        eol = "\r\n" if text.count("\r\n") * 2 > text.count("\n") else "\n"

        declarations = eol.join(DECLARATIONS.split("\n"))
        if declarations not in text:
            namespace = eol.join(NAMESPACE.split("\n"))
            if text.count(namespace) != 1:
                raise RuntimeError("%s: namespace opening changed" % name)
            text = text.replace(namespace, namespace + declarations, 1)

        for site_name, instruction, old, new in SITES:
            if site_name != name:
                continue
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
