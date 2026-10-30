"""asm.py <outdir> <addr> [<addr> ...]: write each generated function's asm listing (the comment lines) to <outdir>/f_<addr>.asm.
asm.py <outdir> --group "<purpose>": all the functions of that purpose in diagnostics/x87/functions.csv."""
import csv
import glob
import re
import sys

out = sys.argv[1]
if sys.argv[2] == "--group":
    rows = list(csv.DictReader(open("diagnostics/x87/functions.csv", encoding="utf-8")))
    want = {int(r["address"], 16) for r in rows if r["purpose"] == sys.argv[3]}
else:
    want = {int(a, 16) for a in sys.argv[2:]}
DEF = re.compile(r"^void Application::(sub_[0-9a-f]+)\(", re.M)
found = set()
for p in glob.glob("src/nfs3hp/disassembly/nfs3hp.*.cpp"):
    tx = open(p, encoding="utf-8", errors="replace").read()
    d = list(DEF.finditer(tx))
    for i, m in enumerate(d):
        a = int(m.group(1)[4:], 16)
        if a in want:
            end = d[i + 1].start() if i + 1 < len(d) else len(tx)
            asm = [l.strip()[3:] for l in tx[m.start():end].splitlines() if re.match(r"\s*// [0-9a-f]{8}\s", l)]
            open("%s/f_%x.asm" % (out, a), "w").write("\n".join(asm))
            found.add(a)
print("written", len(found), "of", len(want), "missing", sorted(hex(a) for a in want - found))
