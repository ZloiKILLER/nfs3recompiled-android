"""Whether sound_checks catches what it is there to catch.

    python3 tools/sound_checks/mutations.py [cases]

Each mutant runs on the export it touches and stops at the first case it gets
wrong; one that gets none wrong in `cases` (5000) is missed.

Builds sound_checks again and again, each time with one mistake put into a
copy of src/nfs3hp/native_isnd.cpp -- an argument read at the wrong time, a
byte written as a word, a check left out -- and expects it to fail.  Only the
mutant is compiled; the rest is linked from what tools/sound_checks/run.sh
built, so run that first.
"""
import subprocess
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]

# The export a mutation is checked on, by what comes before the colon.
EXPORTS = {
    "pos3d": "iSNDdirectpos3d",
    "the calls out without a slot for their return address": "iSNDdirectpos3d",
    "vol": "iSNDdirectvol",
    "rate": "iSNDdirectrate",
    "dllversion": "iSNDdllversion",
    "setfunctions": "iSNDdirectsetfunctions",
    "recordstart": "iSNDdirectrecordstart",
    "create3dbuf": "iSNDdirectcreate3dbuf",
    "removebuf": "iSNDdirectremovebuf",
    "play3d": "iSNDdirectplay3d",
    "stopbuf": "iSNDdirectstopbuf",
}

# (what, [(the text in native_isnd.cpp, what it becomes), ...])
MUTATIONS = [
    ("pos3d: an argument read after esp has moved",
     [("    const x86::reg32 b = at(app, cpu.esp + 0x24);\n    cpu.esp -= 4;\n",
       "    cpu.esp -= 4;\n    const x86::reg32 b = at(app, cpu.esp + 0x24);\n")]),
    ("pos3d: the arguments pushed the other way round",
     [("    put(app, cpu.esp, b);\n", "    put(app, cpu.esp, at(app, cpu.esp + 0x20));\n")]),
    ("pos3d: the buffer read before the call out",
     [("    const x86::reg32 index = at(app, esp + 0x18);\n",
       "    const x86::reg32 index = at(app, position + 0x18);\n")]),
    ("pos3d: the scale taken as a float",
     [("    const x86::Float scale = x86::Float(app->getMemory<double>(isnd::kPositionScale));\n",
       "    const x86::Float scale = x86::Float(float(app->getMemory<double>(isnd::kPositionScale)));\n")]),
    ("pos3d: esp taken as the call should leave it, not as it does",
     [("    const x86::reg32 esp = cpu.esp;\n    const x86::Float x",
       "    const x86::reg32 esp = position;\n    const x86::Float x")]),
    ("the calls out without a slot for their return address",
     [("    cpu.esp -= 4;\n    app->dynamic_call(address, cpu);\n", "    app->dynamic_call(address, cpu);\n")]),
    ("vol: the volume written as a word",
     [("    app->getMemory<x86::reg8>(record + isnd::kBufferVolume) = x86::reg8(at(app, cpu.esp + 8));\n",
       "    app->getMemory<x86::reg16>(record + isnd::kBufferVolume) = x86::reg16(at(app, cpu.esp + 8));\n")]),
    ("rate: the rate two bytes off",
     [("    app->getMemory<x86::reg16>(record + isnd::kBufferRate) = x86::reg16(rate);\n",
       "    app->getMemory<x86::reg16>(record + isnd::kBufferRate + 2) = x86::reg16(rate);\n")]),
    ("dllversion: the caps not forgotten",
     [("    put(app, isnd::kCapsKept, isnd::kFailure);\n", "")]),
    ("setfunctions: one function short",
     [("    for (x86::reg32 i = 0; i < 8; ++i)\n", "    for (x86::reg32 i = 0; i < 7; ++i)\n")]),
    ("recordstart: 0 for -1",
     [("    return returned(cpu, isnd::kFailure, 0x10);\n", "    return returned(cpu, 0, 0x10);\n")]),
    ("create3dbuf: stepping aside with 3D alone",
     [("    if (flag(app, isnd::kIs3d) && flag(app, isnd::kDirectSoundMade))\n        return false;\n    return returned(cpu, 0, 0x1c);",
       "    if (flag(app, isnd::kIs3d))\n        return false;\n    return returned(cpu, 0, 0x1c);")]),
    ("removebuf: the slot not freed",
     [("    put(app, slot + isnd::kSlotId, 0);\n", "")]),
    ("play3d: the slot's own buffer released too",
     [("    if (active != 0 && active != at(app, cpu.esi + isnd::kBufferMain))\n", "    if (active != 0)\n")]),
    ("play3d: esi taken once, not as each call leaves it",
     [("    const x86::reg32 active = at(app, cpu.esi + isnd::kBufferActive);\n",
       "    const x86::reg32 record = cpu.esi;\n    const x86::reg32 active = at(app, cpu.esi + isnd::kBufferActive);\n"),
      ("    const x86::reg32 object3d = at(app, cpu.esi + isnd::kBuffer3d);\n",
       "    const x86::reg32 object3d = at(app, record + isnd::kBuffer3d);\n")]),
    ("play3d: the 3D object read before the first release",
     [("    const x86::reg32 active = at(app, cpu.esi + isnd::kBufferActive);\n",
       "    const x86::reg32 active = at(app, cpu.esi + isnd::kBufferActive);\n    const x86::reg32 early = at(app, cpu.esi + isnd::kBuffer3d);\n"),
      ("    const x86::reg32 object3d = at(app, cpu.esi + isnd::kBuffer3d);\n",
       "    const x86::reg32 object3d = early;\n")]),
    ("play3d: ebp kept, not popped",
     [("    cpu.ebp = at(app, frame);\n", "")]),
    ("play3d: going on after the game stopped",
     [("        if (!release(app, cpu, active))\n            return true;\n", "        release(app, cpu, active);\n")]),
    ("stopbuf: the 3D object not forgotten",
     [("    put(app, record + isnd::kBuffer3d, 0);\n    if (!callOut", "    if (!callOut")]),
    ("stopbuf: the result left as the game's function left it",
     [("    cpu.esp += 8;\n    return returned(cpu, 0, 4);\n", "    cpu.esp += 8 + 4 + 4;\n    return true;\n")]),
]


def main():
    cases = sys.argv[1] if len(sys.argv) > 1 else "5000"
    build = HERE / "build"
    base = sorted((build / "double/base").glob("*.o"))
    if not base:
        raise SystemExit("run tools/sound_checks/run.sh first")
    original = (ROOT / "src/nfs3hp/native_isnd.cpp").read_text()
    missed = 0
    with tempfile.TemporaryDirectory() as tmp:
        for what, edits in MUTATIONS:
            text = original
            for old, new in edits:
                if text.count(old) != 1:
                    raise SystemExit("mutation does not apply: " + what)
                text = text.replace(old, new)
            source = Path(tmp) / "native_isnd.cpp"
            source.write_text(text)
            obj = Path(tmp) / "native_isnd.o"
            exe = Path(tmp) / "mutant"
            # Only the mutant is compiled; the rest is run.sh's, linked as it is.
            subprocess.run(["c++", "-std=c++17", "-O1", "-fno-strict-aliasing", "-DWITH_MMX", "-DNFS_SOUND_CHECKS", "-w",
                            "-I" + str(build / "generated"), "-I" + str(ROOT / "sdl3/include"),
                            "-I" + str(ROOT / "include"),
                            "-I" + str(ROOT / "src/nfs3hp"), "-c", str(source), "-o", str(obj)], check=True)
            subprocess.run(["c++"] + [str(p) for p in base] + [str(obj), "-o", str(exe)], check=True)
            export = EXPORTS[what.split(":")[0]]
            result = subprocess.run([str(exe), cases, "--first", export], capture_output=True, text=True)
            caught = result.returncode != 0
            missed += 0 if caught else 1
            print("%-8s %s" % ("caught" if caught else "MISSED", what), flush=True)
    print("%d of %d caught" % (len(MUTATIONS) - missed, len(MUTATIONS)))
    sys.exit(1 if missed else 0)


if __name__ == "__main__":
    main()
