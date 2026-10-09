"""Build and run tools/native_ai_checks.cpp: src/nfs3hp/native_ai.h against the
generated code it stands in for.

    python3 tools/native_ai_checks.py [cases]

Takes the generated sub_4064f0, sub_4070c0 and sub_4e06d9 out of
src/nfs3hp/disassembly as they are (without the native hook in front of them),
compiles them next to the natives on a plain block of memory holding the
executable's data, and compares registers, flags, the x87 stack, status and
control words and memory after both, on random states: once with the default
FPU (doubles) and once with WITH_PEDANTIC_FPU (80 bits, src/lib/x87soft.cpp).
Needs a C++17 compiler with unsigned __int128 (GCC or Clang)."""
import re
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
FUNCTIONS = ["sub_4064f0", "sub_4070c0", "sub_4e06d9"]
HOOK = re.compile(r"    if \(\w+\(app, cpu(?:\.sync\(\))?\)\) /\* port: native \([^)]*\) \*/\n    \{\n(?:        cpu\.reload\(\);\n)?        return;\n    \}\n")


def body(name):
    for path in sorted((ROOT / "src/nfs3hp/disassembly").glob("nfs3hp.*.cpp")):
        text = path.read_text(errors="replace").replace("\r\n", "\n")
        m = re.search(r"^void Application::%s\((WinApplication\* (?:__restrict )?app, x86::CPU& cpu_?)\)\n\{\n" % name, text, re.M)
        if not m:
            continue
        start = m.end()
        end = text.index("\n}\n", start)
        code = HOOK.sub("", text[start:end + 1])
        params = m.group(1).replace("WinApplication*", "MockApp*")
        return "static void %s(%s)\n{\n%s}\n" % (name, params, code)
    raise SystemExit("no generated " + name)


def main():
    cases = sys.argv[1] if len(sys.argv) > 1 else "200000"
    with tempfile.TemporaryDirectory() as tmp:
        generated = Path(tmp) / "native_ai_generated.inc"
        generated.write_text("".join(body(f) for f in FUNCTIONS))
        failed = False
        for label, extra in (("double", []), ("80-bit", ["-DWITH_PEDANTIC_FPU", str(ROOT / "src/lib/x87soft.cpp")])):
            exe = Path(tmp) / ("checks-" + label)
            cmd = ["g++", "-std=c++17", "-O2", "-fno-strict-aliasing", "-w", "-DWITH_MMX",
                   "-I" + str(ROOT / "include"), "-I" + str(ROOT / "src/nfs3hp"), "-I" + tmp,
                   str(ROOT / "tools/native_ai_checks.cpp")] + extra + ["-o", str(exe)]
            subprocess.run(cmd, check=True)
            print("== %s FPU" % label, flush=True)
            result = subprocess.run([str(exe), str(ROOT / "nfs3hp/nfs3.exe"), cases])
            failed = failed or result.returncode != 0
    sys.exit(1 if failed else 0)


if __name__ == "__main__":
    main()
