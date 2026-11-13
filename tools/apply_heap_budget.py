"""Apply the reviewed heap immediates after code generation; never patch input EXEs."""
from pathlib import Path

# (generated file, address and bytes of the instruction, original code, patched code)
SITES = [
    (name, address + "  b800000001", "    cpu.eax = 16777216 /*0x1000000*/;",
     "    // Port heap budget: 64 MiB (original instruction requests 16 MiB).\n    cpu.eax = 64u * 1024u * 1024u;")
    for name, address in (("nfs3hp.25.cpp", "004b6097"), ("nfs3hp.39.cpp", "004f92d1"))
]


def apply(root):
    for filename, instruction, code, patched in SITES:
        path = root / "src/nfs3hp/disassembly" / filename
        text = path.read_text()
        comment = "// " + instruction[:8] + "  b800000001             -mov eax, 0x1000000\n"
        old = comment + code
        new = comment + patched
        if new in text:
            continue
        if text.count(old) != 1:
            raise RuntimeError("Heap patch site changed: " + str(path))
        path.write_text(text.replace(old, new))

if __name__ == "__main__":
    apply(Path(__file__).resolve().parents[1])
