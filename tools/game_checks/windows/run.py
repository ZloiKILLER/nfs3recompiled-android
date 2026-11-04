"""tools/game_checks/run.sh on Windows: the game's code for the natives made
afresh (generate.py), built with winbuild.py, and every native held against
it -- or the one named.

    python tools/game_checks/windows/run.py [cases] [name ...]

    python tools/game_checks/windows/run.py 300 --changed
    python tools/game_checks/windows/run.py 1000 decompiled::resetSunLighting

A name ending in :: (graphics::) checks every function of that namespace.

--changed checks what changed since the last run: the functions of the sources
that differ from then (of a header, of the sources that include it), those that
differed last time, and nothing else.  A change to what everything stands on --
the generator, include/, src/lib, game_checks.cpp -- checks them all, and so does
a first run.  It says when native_vertices.cpp or native_render.cpp changed,
whose checks are tools/game_checks/vertices and render.

Each function's seconds go to build/timings.txt, the slowest first.

generate.py runs again only when something it reads changed (generate_key),
only the sources that changed since are compiled again, the generated code in
PARTS files side by side, and the functions are checked as many at a time as
the machine has threads (GAME_CHECKS_JOBS to set it), each in a process of
its own, reported in the order asked."""
import concurrent.futures, hashlib, json, os, re, shutil, subprocess, sys, time
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import winbuild  # noqa: E402

BUILD = HERE / "build"
ROOT = winbuild.ROOT
# game_functions.cpp, one file of tens of megabytes, compiled as this many
# parts side by side.
PARTS = 16
# Seconds a function's cases may take before it counts as hung.
TIMEOUT = 1200
# Cases whose generated side never finishes or crashes the check on the world it is given (a loop
# or a pointer no game has): left out, the cases after them as they were (game_checks.cpp, GAME_CHECKS_SKIP).
SKIP_CASES = "port::codedPacketDecode:482,port::streamReaderThread:138,port::streamReaderThread:321,port::streamReaderThread:978"


def generate_key():
    """What generate.py's output depends on: the files it reads whole, the
    rows game_checks' tables stand in for, and of each of the port's sources
    only what it looks for there -- the addresses a file names, the functions
    it defines, its namespaces and headers (named_addresses).  The same key,
    the same output."""
    h = hashlib.sha1()
    whole = [ROOT / "tools/game_checks/generate.py", ROOT / "tools/game_checks/entry_regs.py",
             ROOT / "disassemble_nfs3hp.py", ROOT / "nfs3hp/nfs3.exe", ROOT / "src/nfs3hp/decompiled.json"]
    whole += sorted((ROOT / "disasm").rglob("*.py")) + sorted((ROOT / "tools").glob("apply_*.py"))
    whole += sorted((ROOT / "src/nfs3hp").glob("*.inc"))
    for f in whole:
        h.update(f.name.encode())
        h.update(f.read_bytes())
    rows = re.compile(r'^\s*\{(0x[0-9a-f]+), "[^"]*", k(?:Watcom|Cdecl|Stdcall)', re.M)
    for f in [ROOT / "tools/game_checks/game_checks.cpp"] + sorted((ROOT / "tools/game_checks").glob("*.inc")):
        h.update(" ".join(sorted(rows.findall(f.read_text(encoding="utf-8")))).encode())
    for f in sorted((ROOT / "src/nfs3hp").glob("*.cpp")) + sorted((ROOT / "src/nfs3hp").glob("*.h")):
        code = re.sub(r"/\*.*?\*/|//[^\n]*", "", f.read_text(encoding="utf-8", errors="replace"), flags=re.S)
        h.update(f.name.encode())
        for found in (re.findall(r"\b0x[0-9a-f]{6}\b", code), re.findall(r"\bvoid (\w+)\(", code),
                      re.findall(r"\bnamespace (\w+)", code), re.findall(r'#include "([\w.]+\.h)"', code)):
            h.update((" ".join(sorted(set(found))) + "|").encode())
    return h.hexdigest()


def put(path, text):
    """`text` to `path` only when it differs: an unchanged file keeps its
    time, and what is built from it is not built again."""
    if not path.exists() or path.read_text(encoding="utf-8", errors="replace") != text:
        path.write_text(text, encoding="utf-8")


def split_functions(generated):
    """game_functions.cpp as PARTS files, cut between functions."""
    text = (generated / "game_functions.cpp").read_text(encoding="utf-8", errors="replace")
    head = '#include "nfs3hp.h"\n#include <lib/thread.h>\n\nnamespace nfs3hp\n{\n\n'
    assert text.startswith(head) and text.endswith("}\n"), "game_functions.cpp is not as split_functions knows it"
    body = text[len(head):-2]
    size = len(body) // PARTS + 1
    bounds, last = [0], 0
    for m in re.finditer(r"\nvoid Application::sub_", body):
        if m.start() + 1 - last >= size and len(bounds) < PARTS:
            last = m.start() + 1
            bounds.append(last)
    bounds.append(len(body))
    parts = []
    for i in range(PARTS):
        piece = body[bounds[i]:bounds[i + 1]] if i + 1 < len(bounds) else ""
        path = generated / ("game_functions_%02d.cpp" % i)
        put(path, head + piece + "}\n")
        parts.append(path)
    return parts


def build(generated):
    """generate.py's output, made again only when its key changed (into a
    fresh directory, copied over where a file differs), and the checker built
    from it; the checker's path."""
    key = generate_key()
    stamp = generated / "generate.key"
    if not stamp.exists() or stamp.read_text() != key:
        fresh = BUILD / "generated.new"
        shutil.rmtree(fresh, ignore_errors=True)
        r = subprocess.run([sys.executable, str(ROOT / "tools/game_checks/generate.py"), str(fresh)],
                           cwd=ROOT, stdin=subprocess.DEVNULL)
        if r.returncode:
            sys.exit("generate.py failed")
        generated.mkdir(parents=True, exist_ok=True)
        for f in fresh.iterdir():
            put(generated / f.name, f.read_text(encoding="utf-8", errors="replace"))
        shutil.rmtree(fresh, ignore_errors=True)
        stamp.write_text(key)
    else:
        print("generate.py: nothing it reads changed, its output kept", flush=True)
    sources = split_functions(generated) + [s for s in winbuild.base_sources(generated)
                                            if s.name != "game_functions.cpp"]
    objects = BUILD / ("obj_debug" if winbuild.DEBUG else "obj")
    errors = winbuild.compile_all(sources, objects, generated)
    if errors:
        sys.exit("\n".join(errors))
    exe = BUILD / ("game_checks_debug.exe" if winbuild.DEBUG else "game_checks.exe")
    err = winbuild.link(sorted(objects.glob("*.obj")), exe)
    if err:
        sys.exit(err)
    return exe


STATE = BUILD / "last_run.json"


def watched():
    """What the checks stand on, relative to the root: the port's sources, what all
    of them stand on, and the checks' own."""
    files = []
    for folder, pattern in (("src/nfs3hp", "*.cpp"), ("src/nfs3hp", "*.h"), ("src/nfs3hp", "*.inc"),
                            ("include", "**/*.h"), ("src/lib", "**/*.cpp"), ("src/lib", "**/*.h"),
                            ("disasm", "**/*.py"), ("tools/game_checks", "*.inc"), ("tools/game_checks", "*.cpp"),
                            ("tools/game_checks", "*.py"), ("tools", "apply_*.py")):
        files += (ROOT / folder).glob(pattern)
    files += [ROOT / "disassemble_nfs3hp.py", ROOT / "nfs3hp/nfs3.exe"]
    return {f.relative_to(ROOT).as_posix(): hashlib.sha1(f.read_bytes()).hexdigest() for f in files}


def stands_under_everything(path):
    return (path.startswith(("include/", "src/lib/", "disasm/", "tools/apply_", "nfs3hp/"))
            or path in ("disassemble_nfs3hp.py", "tools/game_checks/game_checks.cpp")
            or (path.startswith("tools/game_checks/") and path.endswith(".py"))
            or path.endswith(".inc") and path.startswith("src/nfs3hp/"))


def code_of(path):
    return re.sub(r"/\*.*?\*/|//[^\n]*", "", (ROOT / path).read_text(encoding="utf-8", errors="replace"), flags=re.S)


def includers(header, sources):
    """The sources that include `header`, however deep."""
    found, work = set(), [header]
    while work:
        name = Path(work.pop()).name
        for path, code in sources.items():
            if path not in found and '#include "%s"' % name in code:
                found.add(path)
                if path.endswith(".h"):
                    work.append(path)
    return found


def changed_functions(all_names):
    """The functions --changed checks, or all of them."""
    state = json.loads(STATE.read_text()) if STATE.exists() else None
    if state is None:
        print("--changed: no earlier run, so everything", flush=True)
        return list(all_names)
    now = watched()
    changed = sorted(p for p in set(now) | set(state["files"]) if now.get(p) != state["files"].get(p))
    print("--changed: %s" % (", ".join(changed) or "nothing changed"), flush=True)
    under = [p for p in changed if stands_under_everything(p)]
    if under:
        print("  under everything: %s, so everything" % ", ".join(under), flush=True)
        return list(all_names)
    wanted = set(n for n in state["failed"] if n in all_names)
    if any(p.endswith(("native_vertices.cpp", "native_render.cpp")) for p in changed):
        print("  also run tools/game_checks/vertices/run.py and render/run.py", flush=True)
    sources = {p: code_of(p) for p in now if p.startswith("src/nfs3hp/") and p.endswith((".cpp", ".h"))}
    touched = set()
    for path in changed:
        if path not in now:
            continue
        if path.startswith("tools/game_checks/"):
            rows = re.findall(r'^\s*\{"([\w:]+)",', (ROOT / path).read_text(encoding="utf-8", errors="replace"), re.M)
            wanted.update(n for n in rows if n in all_names)
        elif path.endswith(".h"):
            touched |= includers(path, sources) | {path}
        else:
            touched.add(path)
    for path in touched:
        code = sources.get(path, "")
        defined = set(re.findall(r"\bvoid (\w+)\(", code))
        wanted.update(n for n in all_names if n.split("::")[-1] in defined)
    return sorted(wanted)


def main():
    os.environ.setdefault("GAME_CHECKS_SKIP", SKIP_CASES)
    args = sys.argv[1:]
    only_changed = "--changed" in args
    if only_changed:
        args.remove("--changed")
    cases = args[0] if args else "300"
    names = args[1:]
    generated = BUILD / "generated"
    # What is checked is what is here now: a file edited while they run counts as changed next time.
    files = watched()
    exe = build(generated)
    tested = sorted(set(re.findall(r'"([\w:]+)"', (generated / "game_tested.inc").read_text())))
    if only_changed:
        wanted = changed_functions(tested)
    elif not names:
        wanted = tested
    else:
        wanted = []
        for name in names:
            wanted += sorted(n for n in tested if n.startswith(name)) if name.endswith("::") else [name]

    def check(name):
        # A function whose case hangs (the generated side looping) is cut off, not waited for.
        began = time.time()
        try:
            r = subprocess.run([str(exe), cases, "all", name], capture_output=True, text=True, timeout=TIMEOUT)
        except subprocess.TimeoutExpired as e:
            r = subprocess.CompletedProcess(e.cmd, -1, "", "")
        return r, time.time() - began

    failed, timings = [], {}
    began = time.time()
    with concurrent.futures.ThreadPoolExecutor(winbuild.JOBS) as pool:
        for name, (r, seconds) in zip(wanted, pool.map(check, wanted)):
            timings[name] = seconds
            alike = r.returncode == 0 and "all alike" in r.stdout
            if not alike:
                failed.append(name)
                print("%-50s DIFFERS (exit %d)" % (name, r.returncode), flush=True)
                print("\n".join(l for l in r.stdout.splitlines() if l.strip())[-1500:])
    spent = sum(timings.values())
    slowest = sorted(timings, key=timings.get, reverse=True)
    (BUILD / "timings.txt").write_text("".join("%8.1f  %s\n" % (timings[n], n) for n in slowest))
    print("%d of %d alike in %.0f s; the slowest: %s" % (
        len(wanted) - len(failed), len(wanted), time.time() - began,
        ", ".join("%s %.0f s" % (n, timings[n]) for n in slowest[:5])), flush=True)
    print("%.0f s of work, the slowest ten %.0f%% of it (build/timings.txt)" % (
        spent, 100 * sum(timings[n] for n in slowest[:10]) / max(spent, 1)), flush=True)
    if only_changed or not names:
        STATE.write_text(json.dumps({"files": files, "failed": failed}))
    sys.exit(1 if failed else 0)


if __name__ == "__main__":
    main()
