"""native_render.cpp's natives held against the game's code on Windows, in
the x87 modes the game runs in (rcheck.cpp):

    python tools/game_checks/render/run.py [cases] [control words] [name ...]

    python tools/game_checks/render/run.py 300 7f,37f,27f,c7f,f7f hud sky

The game's code made afresh (gen_render.py, a minute or so), built with the
NDK's clang-cl as tools/game_checks/windows/winbuild.py builds game_checks,
into build/ here, and run function by function and mode by mode, in parallel
(RCHECK_JOBS, the processors by default).  A case that runs away (over 4 s,
too many pages touched, a host fault) is counted as one, and the run goes on
after it.  The control word defaults to 7f, the one mode the game's code is in
now (fp::, include/fpu.h).  Per mode: the
cases run, those the game stopped in a call, those that differ (the first two
shown), the runaways, the words told apart only by a NaN's payload, and the
cases whose outcome differs from the first mode's (mode-sensitive: the modes
reach them).  RENDER_VERBOSE=1 lists a differing case's calls,
RENDER_WATCH=<hex,hex> some words before and after (build/rcheck.exe <name>
1 <case> <control word>, run in build/generated, takes them too); RCHECK_DF=1
sets the direction flag in a fifth of the cases; RCHECK_NOGEN=1 builds on the
game's code made last time."""
import concurrent.futures
import os
import re
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
sys.path.insert(0, str(ROOT / "tools/game_checks/windows"))
import winbuild  # noqa: E402

BUILD = HERE / "build"
GEN = BUILD / "generated"
OBJ = BUILD / "obj"
EXE = BUILD / "rcheck.exe"
CONTROLS = "7f"


def compile_one(source):
    inc = ["/I" + str(p) for p in [GEN, HERE, ROOT / "sdl3/include", ROOT / "include", ROOT / "src/nfs3hp"]]
    cmd = [str(winbuild.NDK / "clang-cl.exe"), "/nologo", "/c", str(source), "/Fo" + str(OBJ / (source.stem + ".obj"))] \
        + winbuild.FLAGS + inc
    r = subprocess.run(cmd, capture_output=True, text=True, env=winbuild.env())
    return None if r.returncode == 0 else source.name + "\n" + r.stdout[-6000:] + r.stderr[-6000:]


def build():
    if not (os.environ.get("RCHECK_NOGEN") and (GEN / "game_functions.cpp").exists()):
        r = subprocess.run([sys.executable, str(HERE / "gen_render.py"), str(GEN)], stdin=subprocess.DEVNULL)
        if r.returncode:
            sys.exit("gen_render.py failed")
    OBJ.mkdir(parents=True, exist_ok=True)
    sources = [GEN / "game_functions.cpp", ROOT / "src/lib/cpu.cpp", ROOT / "src/nfs3hp/native_render.cpp",
               HERE / "rcheck.cpp", HERE / "mem.cpp"]
    with concurrent.futures.ThreadPoolExecutor(8) as pool:
        errors = [e for e in pool.map(compile_one, sources) if e]
    if errors:
        sys.exit("\n".join(errors))
    err = winbuild.link([OBJ / (s.stem + ".obj") for s in sources], EXE)
    if err:
        sys.exit(err)


def run_mode(name, control, cases):
    """One function in one mode: (stopped, failed cases, first failures, runaways, NaN words, hashes)."""
    start, stopped, runaways, shown, nan, failed, hashes = 0, 0, [], [], 0, set(), {}
    while start < cases:
        cmd = [str(EXE), name, str(cases - start), str(start), control] + (["df"] if os.environ.get("RCHECK_DF") else [])
        try:
            out = subprocess.run(cmd, capture_output=True, text=True, timeout=3600, cwd=GEN).stdout
        except subprocess.TimeoutExpired:
            runaways.append("driver timeout at %d" % start)
            break
        for line in out.splitlines():
            m = re.match(r"\w+\s+cw \w+:\s+(\d+) cases,\s+(\d+) stopped,\s+(\d+) failed", line)
            if m:
                stopped += int(m.group(2))
            elif line.startswith("HASH"):
                _, i, h = line.split()
                hashes[int(i)] = h
            elif line.startswith("FAILCASE"):
                failed.add(int(line.split()[1]))
            elif line.startswith("  ") and len(shown) < 2:
                shown.append(line.strip())
            elif line.startswith("NaN words"):
                nan += int(line.split(":")[1])
        m = re.search(r"RUNAWAY \w+ \w+ (\d+) (.*)", out)
        if m:
            runaways.append(int(m.group(1)))
            start = int(m.group(1)) + 1
            continue
        break
    return stopped, failed, shown, runaways, nan, hashes


def run_function(name, controls, cases):
    lines = []
    first = None
    total = 0
    for control in controls:
        stopped, failed, shown, runaways, nan, hashes = run_mode(name, control, cases)
        first = hashes if first is None else first
        sensitive = sum(1 for i, h in hashes.items() if i in first and first[i] != h)
        total += len(failed)
        lines.append("%-20s cw %4s: %5d cases, %4d stopped, %4d failed, %4d runaway, %d NaN-payload words, "
                     "%d mode-sensitive" % (name, control, cases - len(runaways), stopped, len(failed), len(runaways),
                                            nan, sensitive))
        if failed:
            lines.append("    failing cases: %s" % " ".join(str(i) for i in sorted(failed)[:20]))
        lines += ["    " + s[:500] for s in shown]
    return total, "\n".join(lines)


def main():
    args = sys.argv[1:]
    cases = int(args.pop(0)) if args and args[0].isdigit() else 300
    controls = (args.pop(0) if args and re.fullmatch(r"[0-9a-fA-F,]+", args[0]) else CONTROLS).split(",")
    build()
    names = args or re.findall(r'"(\w+)", &nfs3hp::Gen', (GEN / "game_functions.cpp").read_text())
    jobs = int(os.environ.get("RCHECK_JOBS", os.cpu_count() or 4))
    failed = 0
    with concurrent.futures.ThreadPoolExecutor(jobs) as pool:
        for total, text in pool.map(lambda n: run_function(n, controls, cases), names):
            failed += total
            print(text, flush=True)
    print("%d functions, %d differing cases" % (len(names), failed))
    sys.exit(1 if failed else 0)


if __name__ == "__main__":
    main()
