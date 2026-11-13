"""How much of a group's native source its check would notice broken.

    python tools/frontend_checks/mutate.py <group> [--max N] [--rounds 300] [--seed 1] [--jobs N]

One change at a time is made to src/nfs3hp/native_frontend_<group>.cpp, and the
check is run on it:

  compare   == and !=, < and <=, > and >= turned round
  constant  a small number one more
  drop      a guest.setX, or a guest.call's call, gone (its result then what eax holds)
  args      two arguments of a call swapped
  order     the operands of fp::sub, fp::div, fp::compare swapped, and of fp::add and
            fp::mul where both can be NaN (nothing else tells their order apart):
            neither a literal nor made from an integer
  float     fp::add and the rest as plain float arithmetic

For each function: caught, survived with the changed line, and those that did not
compile, which do not count.  A helper of the file's own is reported as itself.

The source in the tree is never touched: each change is a file of its own in a
temporary directory, compiled and linked there beside the check's other objects
into an executable of its own, so several run side by side and an interrupted run
leaves nothing behind (the source's hash is checked at the end all the same).
A change runs first against the functions that can reach it, then, if it
survives, against the whole group.

Which changes are taken: every function in turn, in a shuffled order fixed by
--seed, one each until --max is reached, so a small function counts as a large one.
Survivors are what to look at: a world too narrow, or a change that truly changes
nothing (a NaN's order, a frame's size); the report does not tell those apart.
"""
import argparse
import concurrent.futures
import hashlib
import os
import random
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE / "windows"))
import run as check  # noqa: E402

winbuild = check.winbuild
ROOT = check.ROOT


class Mutant:
    __slots__ = ("function", "rule", "start", "end", "text", "line", "shown")

    def __init__(self, function, rule, start, end, text, line, shown):
        self.function, self.rule, self.start, self.end, self.text = function, rule, start, end, text
        self.line, self.shown = line, shown


def blank(source):
    """The source with comments, strings and characters blanked, same length: what to match on."""
    out = list(source)
    i, n = 0, len(source)
    while i < n:
        c = source[i]
        two = source[i:i + 2]
        if two == "//":
            while i < n and source[i] != "\n":
                out[i] = " "
                i += 1
        elif two == "/*":
            end = source.find("*/", i + 2)
            end = n if end < 0 else end + 2
            for k in range(i, end):
                if out[k] != "\n":
                    out[k] = " "
            i = end
        elif c == '"' or c == "'":
            k = i + 1
            while k < n and source[k] != c:
                k += 2 if source[k] == "\\" else 1
            for j in range(i + 1, min(k, n)):
                out[j] = " "
            i = k + 1
        else:
            i += 1
    return "".join(out)


def matching(code, at, opening="(", closing=")"):
    """Where the bracket opened at `at` closes."""
    depth = 0
    for i in range(at, len(code)):
        if code[i] == opening:
            depth += 1
        elif code[i] == closing:
            depth -= 1
            if depth == 0:
                return i
    return -1


def functions_of(code):
    """(name, body start, body end) of each function at the file's top level or in a namespace:
    a line opening `name(...)` at the left margin, a { on the next."""
    found = []
    pattern = re.compile(r"^(?:[\w:<>,&*]+[ \t]+)+[&*]?((?:\w+::)*\w+)\s*\([^;{}]*\)\s*(?:const\s*)?\{", re.M)
    at = 0
    while True:
        m = pattern.search(code, at)
        if not m:
            return found
        brace = m.end() - 1
        end = matching(code, brace, "{", "}")
        if end < 0:
            return found
        name = m.group(1).split("::")[-1]
        if name not in ("if", "for", "while", "switch"):
            found.append((name, brace + 1, end))
        at = end + 1


def split_arguments(code, start, end):
    """The (start, end) of each top-level argument between two offsets."""
    parts = []
    depth = 0
    begin = start
    for i in range(start, end):
        c = code[i]
        if c in "([{":
            depth += 1
        elif c in ")]}":
            depth -= 1
        elif c == "," and depth == 0:
            parts.append((begin, i))
            begin = i + 1
    parts.append((begin, end))
    return parts


def trimmed(source, part):
    start, end = part
    while start < end and source[start] in " \n\t\r":
        start += 1
    while end > start and source[end - 1] in " \n\t\r":
        end -= 1
    return start, end


COMPARE = {"==": "!=", "!=": "==", "<=": "<", ">=": ">", "<": "<=", ">": ">="}
COMPARE_RE = re.compile(r"(?<=\s)(==|!=|<=|>=|<|>)(?=\s)")
NUMBER_RE = re.compile(r"(?<![\w.])(0x[0-9a-fA-F]+|\d+)(?![\w.'])")
FP = {"add": "+", "sub": "-", "mul": "*", "div": "/"}
FP_RE = re.compile(r"\bfp::(add|sub|mul|div|compare|atan|rem|scale)\s*\(")
GUEST_SET_RE = re.compile(r"\bguest\.set(?:Byte|Word|Dword|Float|Esi)\s*\(")
GUEST_CALL_RE = re.compile(r"\bguest\.call\s*\(")
CALL_RE = re.compile(r"(?<![\w.:>])(\w+(?:::\w+)*)\s*\(")
ARGS_RE = re.compile(r"\bargs\s*\(\s*\{")
NOT_CALLS = {"if", "for", "while", "switch", "return", "sizeof", "static_cast", "reg32", "sreg32", "Float", "float", "double",
             "int", "bool", "args", "catch"}
NEVER_NAN = re.compile(r"^(?:[\d.]+f?|Float\(\s*(?:sreg32|std::int\w+|reg32|int)\b.*)$", re.S)


def line_of(source, at):
    start = source.rfind("\n", 0, at) + 1
    end = source.find("\n", at)
    return source.count("\n", 0, at) + 1, source[start:end if end >= 0 else len(source)].strip()


def statement_start(code, at, body_start):
    """Where the statement holding `at` begins."""
    i = at - 1
    while i >= body_start and code[i] not in ";{}":
        i -= 1
    i += 1
    while i < at and code[i] in " \n\t\r":
        i += 1
    return i


def rules_in(source, code, name, start, end):
    """Every change that can be made between two offsets."""
    mutants = []

    def add(rule, at_start, at_end, text):
        line, shown = line_of(source, at_start)
        mutants.append(Mutant(name, rule, at_start, at_end, text, line, shown))

    for m in COMPARE_RE.finditer(code, start, end):
        add("compare", m.start(), m.end(), COMPARE[m.group(1)])

    for m in NUMBER_RE.finditer(code, start, end):
        line = code[code.rfind("\n", 0, m.start()) + 1:code.find("\n", m.start())]
        if "fe::Site" in line:
            continue
        value = int(m.group(1), 0)
        if value <= 0x100:
            new = hex(value + 1) if m.group(1).lower().startswith("0x") else str(value + 1)
            add("constant", m.start(), m.end(), new)

    for m in GUEST_SET_RE.finditer(code, start, end):
        close = matching(code, m.end() - 1)
        if close < 0 or code[close + 1:close + 2] != ";":
            continue
        add("drop", m.start(), close + 1, "(void)0;")

    for m in GUEST_CALL_RE.finditer(code, start, end):
        close = matching(code, m.end() - 1)
        if close < 0:
            continue
        before = statement_start(code, m.start(), start)
        if before == m.start() and code[close + 1:close + 2] == ";":
            add("drop", m.start(), close + 1, "(void)0;")
        else:
            # Its value is used: what eax holds instead.
            add("drop", m.start(), close + 1, "guest.eax()")

    for m in ARGS_RE.finditer(code, start, end):
        close = matching(code, m.end() - 1, "{", "}")
        if close < 0:
            continue
        parts = [trimmed(source, p) for p in split_arguments(code, m.end(), close)]
        add_swaps(source, parts, add)

    for m in CALL_RE.finditer(code, start, end):
        if m.group(1) in NOT_CALLS or m.group(1).startswith(("fp::", "fe::", "x86::", "std::")) or m.group(1).split("::")[-1] in NOT_CALLS:
            continue
        close = matching(code, m.end() - 1)
        if close < 0:
            continue
        parts = [trimmed(source, p) for p in split_arguments(code, m.end(), close)]
        add_swaps(source, parts, add)

    for m in FP_RE.finditer(code, start, end):
        close = matching(code, m.end() - 1)
        if close < 0:
            continue
        parts = [trimmed(source, p) for p in split_arguments(code, m.end(), close)]
        if len(parts) != 2:
            continue
        a, b = source[parts[0][0]:parts[0][1]], source[parts[1][0]:parts[1][1]]
        op = m.group(1)
        if a != b and (op not in ("add", "mul") or not (NEVER_NAN.match(a) or NEVER_NAN.match(b))):
            add("order", m.start(), close + 1, "fp::%s(%s, %s)" % (op, b, a))
        if op in FP:
            add("float", m.start(), close + 1, "double(float(%s) %s float(%s))" % (a, FP[op], b))

    return mutants


def add_swaps(source, parts, add):
    """Each two neighbours among a call's arguments that are not the same, the guest and the like left."""
    for first, second in zip(parts, parts[1:]):
        a, b = source[first[0]:first[1]], source[second[0]:second[1]]
        if a == b or a in ("guest", "app", "cpu") or b in ("guest", "app", "cpu") or not a or not b:
            continue
        add("args", first[0], second[1], "%s%s%s" % (b, source[first[1]:second[0]], a))


def collect(source):
    """All changes to the source, by the function they are in; numbers at file scope are
    "(constants)", as every function reads them."""
    code = blank(source)
    mutants = []
    bodies = functions_of(code)
    for name, start, end in bodies:
        mutants += rules_in(source, code, name, start, end)
    inside = [(s, e) for _, s, e in bodies]
    # The named offsets: wrong field, wrong size.
    for m in re.finditer(r"constexpr\b[^;\n]*?=\s*(0x[0-9a-fA-F]+|\d+)\s*;", code):
        if any(s <= m.start() < e for s, e in inside):
            continue
        value = int(m.group(1), 0)
        if value <= 0x1000:
            line, shown = line_of(source, m.start(1))
            new = hex(value + 1) if m.group(1).lower().startswith("0x") else str(value + 1)
            mutants.append(Mutant("(constants)", "constant", m.start(1), m.end(1), new, line, shown))
    return mutants, bodies


def fair_sample(mutants, wanted, seed):
    """One from each function in turn, each function's in a shuffled order."""
    by_function = {}
    for mutant in mutants:
        by_function.setdefault(mutant.function, []).append(mutant)
    generator = random.Random(seed)
    names = sorted(by_function)
    for name in names:
        generator.shuffle(by_function[name])
    chosen = []
    while len(chosen) < wanted and any(by_function.values()):
        for name in names:
            if by_function[name] and len(chosen) < wanted:
                chosen.append(by_function[name].pop())
    return chosen


def reaching(group_rows, source, code, bodies):
    """{function in the file: the group's addresses that can reach it}, by who names whom in the file."""
    names = {name: (start, end) for name, start, end in bodies}
    calls = {name: {other for other in names if other != name
                    and re.search(r"\b%s\b" % re.escape(other), code[start:end])}
             for name, (start, end) in names.items()}
    entry_of = {}
    for row in group_rows:
        if row[1] in names:
            entry_of[row[1]] = row[0]
    reach = {}
    for name in names:
        seen, work = {name}, [name]
        while work:
            for caller, called in calls.items():
                if work[0] in called and caller not in seen:
                    seen.add(caller)
                    work.append(caller)
            work.pop(0)
        reach[name] = {entry_of[n] for n in seen if n in entry_of}
    return reach


def run_exe(exe, rounds, group, only, timeout):
    """(status, what it said): "match", "differ", "crash" or "timeout"."""
    env = dict(os.environ, FRONTEND_CHECKS_GROUP=group, FRONTEND_CHECKS_ONLY=",".join(only))
    try:
        result = subprocess.run([str(exe), str(ROOT / "nfs3hp/nfs3.exe"), str(rounds)], env=env, stdout=subprocess.PIPE,
                                stderr=subprocess.STDOUT, text=True, errors="replace", timeout=timeout)
    except subprocess.TimeoutExpired:
        return "timeout", ""
    if result.returncode == 0:
        return "match", ""
    if result.returncode == 1:
        return "differ", ""
    return "crash", result.stdout[-200:]


def try_mutant(index, mutant, source, work, objects, group, rows, reach, rounds, timeout):
    """Compiled and linked beside the other objects, run: (mutant, outcome)."""
    folder = work / str(index)
    folder.mkdir()
    try:
        path = folder / ("native_frontend_%s.cpp" % group)
        path.write_text(source[:mutant.start] + mutant.text + source[mutant.end:], encoding="utf-8")
        obj = folder / "mutant.obj"
        if winbuild.compile_one(path, obj, check.BUILD):
            return mutant, "build"
        exe = folder / "check.exe"
        if winbuild.link(objects + [obj], exe):
            return mutant, "build"
        whole = [r[0] for r in rows]
        first = sorted(reach.get(mutant.function, ())) or whole
        status, _ = run_exe(exe, rounds, group, first, timeout)
        if status == "match" and len(first) < len(whole):
            status, _ = run_exe(exe, rounds, group, whole, timeout)
        return mutant, status
    finally:
        shutil.rmtree(folder, ignore_errors=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("group")
    parser.add_argument("--max", type=int, default=100)
    parser.add_argument("--rounds", type=int, default=300)
    parser.add_argument("--seed", type=int, default=1)
    parser.add_argument("--jobs", type=int, default=winbuild.JOBS)
    options = parser.parse_args()
    group = options.group
    native = ROOT / "src/nfs3hp" / ("native_frontend_%s.cpp" % group)
    source = native.read_bytes().decode("utf-8")
    before = hashlib.sha1(native.read_bytes()).hexdigest()

    exe = check.build()
    rows = check.rows(check.CHECKS / "groups" / (group + ".txt"))
    whole = [r[0] for r in rows]
    # As it stands it must match, or nothing below says anything; and how long that takes
    # says when a change has hung.
    import time
    began = time.time()
    status, said = run_exe(exe, options.rounds, group, whole, 900)
    if status != "match":
        sys.exit("%s does not match as it stands (%s) %s" % (group, status, said))
    timeout = max(30, 10 * (time.time() - began))

    # CRLF kept: the offsets are the file's own.
    mutants, bodies = collect(source)
    code = blank(source)
    reach = reaching(rows, source, code, bodies)
    chosen = fair_sample(mutants, options.max, options.seed)
    print("%s: %d functions, %d changes possible, %d taken, %d rounds, %d at a time"
          % (group, len({m.function for m in mutants}), len(mutants), len(chosen), options.rounds, options.jobs), flush=True)

    mine = native.stem + ".obj"
    objects = [o for o in sorted((check.BUILD / "obj").glob("*.obj")) if o.name != mine]
    # What a run that was killed left.
    for old in check.BUILD.glob("mutate_*"):
        shutil.rmtree(old, ignore_errors=True)
    work = Path(tempfile.mkdtemp(prefix="mutate_", dir=str(check.BUILD)))
    outcomes = []
    try:
        with concurrent.futures.ThreadPoolExecutor(options.jobs) as pool:
            futures = [pool.submit(try_mutant, i, m, source, work, objects, group, rows, reach, options.rounds, timeout)
                       for i, m in enumerate(chosen)]
            for future in concurrent.futures.as_completed(futures):
                outcomes.append(future.result())
    except KeyboardInterrupt:
        for future in futures:
            future.cancel()
        print("interrupted: what ran is below")
    finally:
        shutil.rmtree(work, ignore_errors=True)
    if hashlib.sha1(native.read_bytes()).hexdigest() != before:
        sys.exit("%s changed while this ran, not by this: look at it" % native)

    report(outcomes)


def report(outcomes):
    tally = {}
    for mutant, status in outcomes:
        entry = tally.setdefault(mutant.function, {"caught": 0, "survived": [], "build": 0})
        if status == "build":
            entry["build"] += 1
        elif status == "match":
            entry["survived"].append(mutant)
        else:
            entry["caught"] += 1
    total_caught = total_survived = total_build = 0
    for name in sorted(tally):
        entry = tally[name]
        total_caught += entry["caught"]
        total_survived += len(entry["survived"])
        total_build += entry["build"]
        print("%-34s caught %3d  survived %3d  build-failed %d" % (name, entry["caught"], len(entry["survived"]), entry["build"]))
        for mutant in sorted(entry["survived"], key=lambda m: m.line):
            print("    %-8s line %d: %s  -> %r" % (mutant.rule, mutant.line, mutant.shown, mutant.text))
    counted = total_caught + total_survived
    print("%d of %d caught (%d%%), %d did not compile and do not count"
          % (total_caught, counted, 100 * total_caught // max(counted, 1), total_build))
    by_rule = {}
    for mutant, status in outcomes:
        if status != "build":
            caught, seen = by_rule.get(mutant.rule, (0, 0))
            by_rule[mutant.rule] = (caught + (status != "match"), seen + 1)
    print("by rule: " + ", ".join("%s %d/%d" % (rule, c, s) for rule, (c, s) in sorted(by_rule.items())))


if __name__ == "__main__":
    main()
