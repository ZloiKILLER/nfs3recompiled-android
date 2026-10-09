"""Copies generated functions out of src/nfs3hp/disassembly.

    python3 extract.py <disassembly dir> <out.cpp> sub_... sub_...

tools/apply_native_loading.py's hooks are taken out of them on the way: what
is left is the generated code alone, for loading_checks.cpp to compare the
natives with.
"""
import re
import sys
from pathlib import Path

HOOK = re.compile(r"    if \(\w+\(app, cpu(?:\.sync\(\))?\)\) "
                  r"/\* port: native \(tools/apply_native_loading\.py\) \*/\r?\n"
                  r"    \{\r?\n(?:        cpu\.reload\(\);\r?\n)?        return;\r?\n    \}\r?\n")


def main():
    source = Path(sys.argv[1])
    out = Path(sys.argv[2])
    wanted = set(sys.argv[3:])
    found = {}
    for path in sorted(source.glob("nfs3hp.*.cpp")):
        lines = path.read_text(errors="replace").split("\n")
        i = 0
        while i < len(lines):
            m = re.match(r"^void Application::(sub_[0-9a-f]+)\(", lines[i])
            if m and m.group(1) in wanted:
                end = i + 1
                while lines[end].rstrip("\r") != "}":
                    end += 1
                found[m.group(1)] = HOOK.sub("", "\n".join(lines[i:end + 1]))
                i = end
            i += 1
    missing = wanted - set(found)
    if missing:
        raise SystemExit("not found: " + " ".join(sorted(missing)))
    with out.open("w") as f:
        f.write('#include "nfs3hp.h"\n\nnamespace nfs3hp\n{\n\n')
        for name in sorted(found):
            if "apply_native_loading" in found[name]:
                raise SystemExit("a hook left in " + name)
            f.write(found[name] + "\n\n")
        f.write("}\n")


if __name__ == "__main__":
    main()
