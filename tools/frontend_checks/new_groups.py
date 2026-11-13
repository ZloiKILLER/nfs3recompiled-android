"""The sorting pass before a group is taken: what those addresses really are.

    python tools/frontend_checks/new_groups.py <name> 0x4d1000 4d1480 ... [--out dir]
    python tools/frontend_checks/new_groups.py <name> --from addresses.txt
    python tools/frontend_checks/new_groups.py <name> --range 0x480000 0x48d000

writes <name>.todo.json (here in groups/ unless --out says): per function its callers,
callees, the strings it names, its x87 and instruction counts, the stand-in row its callers
would need, and a flag where it looks like data decoded as code rather than a function:

  zero-bytes    no callers, and it starts with zero bytes (`add [eax], al` over and over)
  jump-padding  no callers, and it starts by jumping over zeros or padding
  fragment      no callers, and its rets do not come out level: the tail of a function the
                generator's tables cut at a label, not a function
  junk          instructions no compiler writes, more than now and then
  unreachable   no callers, no pointer to it in the image: nothing can reach it

Callers count direct calls and jumps; `pointers` the places in code and in data where its
address stands, which is how the screens' handlers are reached.  A summary is printed:
how many are real, what they call most from outside the list (what the group would need
stand-ins for), and which outside functions call into it.

The .todo.json is not a group: move what is real into groups/<g>.txt by hand.  Addresses
are all it needs, so it works in a copy of the tree without diagnostics/.
"""
import argparse
import json
import re
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import exe_index  # noqa: E402
import specs_from_exe  # noqa: E402

# No compiler of 1998 wrote these in game code: a disassembly running through data does.
JUNK = {"arpl", "bound", "aam", "aad", "aaa", "aas", "daa", "das", "into", "hlt", "lds", "les", "in", "out", "insb", "insd",
        "outsb", "outsd", "sldt", "salc", "xlatb", "icebp", "retf", "iretd", "wait", "pushal", "popal", "loopne", "loope",
        "lock", "cli", "sti", "sahf_", "enter"}
PADDING = (b"\x00\x00", b"\x90\x90", b"\x8d\x40\x00", b"\x8b\xc0", b"\x8d\x80\x00\x00\x00\x00")


def references(game):
    """(direct callers, pointers): {function: callers' entries}, {function: places its address stands}."""
    callers, pointers = {}, {}
    owner = {}
    for function in game.functions.values():
        for insn in function.insns:
            owner.setdefault(insn.address, function.entry)
    for function in game.functions.values():
        own = specs_from_exe.own_addresses(function)
        for insn in function.insns:
            is_flow = insn.mnemonic == "call" or insn.mnemonic.startswith(("j", "loop"))
            for op in insn.operands:
                value = op.imm if op.type == exe_index.IMM else (op.disp & 0xffffffff if op.type == exe_index.MEM else None)
                if value in game.functions and value != function.entry:
                    if is_flow and op.type == exe_index.IMM:
                        if value not in own:
                            callers.setdefault(value, set()).add(function.entry)
                    else:
                        pointers.setdefault(value, set()).add(insn.address)
            for d in insn.destinations:
                if d in game.functions and d not in own:
                    callers.setdefault(d, set()).add(function.entry)
    for start, data, _, code in game.sections:
        if code:
            continue
        for at in range(0, len(data) - 3, 4):
            value = int.from_bytes(data[at:at + 4], "little")
            if value in game.functions:
                pointers.setdefault(value, set()).add(start + at)
    return callers, pointers


def strings_named(game, function):
    found = []
    for insn in function.insns:
        for op in insn.operands:
            value = op.imm if op.type == exe_index.IMM else (op.disp & 0xffffffff if op.type == exe_index.MEM and not op.base else None)
            if value is None:
                continue
            text = game.string_at(value)
            if text is not None and text not in found:
                found.append(text)
    return found


def data_flags(game, function, callers, pointers, row):
    flags = []
    alone = not callers and not pointers
    first = function.insns[0]
    if not callers and first.raw[:2] == b"\x00\x00":
        flags.append("zero-bytes")
    if not callers and first.mnemonic == "jmp" and first.operands and first.operands[0].type == exe_index.IMM:
        target = first.operands[0].imm
        between = b""
        for start, data, _, _ in game.sections:
            at = first.address + first.size
            if start <= at < start + len(data) and target > at:
                between = data[at - start:target - start]
                break
        if between and (set(between) <= {0, 0x90} or between.startswith(PADDING)):
            flags.append("jump-padding")
    if not callers and row["lost"]:
        flags.append("fragment")
    junk = sum(1 for i in function.insns if i.mnemonic.split()[-1] in JUNK or (i.raw[:2] == b"\x00\x00"))
    if junk and junk * 20 >= len(function.insns):
        flags.append("junk")
    if alone:
        flags.append("unreachable")
    return flags


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("name")
    parser.add_argument("addresses", nargs="*")
    parser.add_argument("--from", dest="source", type=Path, help="a file of addresses, the first word of each line")
    parser.add_argument("--range", nargs=2, help="every function between two addresses")
    parser.add_argument("--out", type=Path, default=HERE / "groups")
    options = parser.parse_args()

    game = exe_index.load()
    words = list(options.addresses)
    if options.source:
        words += [line.split()[0] for line in options.source.read_text().splitlines() if line.strip() and not line.startswith("#")]
    wanted = {int(word, 16) for word in words}
    if options.range:
        low, high = int(options.range[0], 16), int(options.range[1], 16)
        wanted |= {entry for entry in game.functions if low <= entry < high}
    absent = sorted(a for a in wanted if a not in game.functions)
    wanted -= set(absent)
    if not wanted:
        sys.exit("no function among those")

    table = specs_from_exe.table()
    callers, pointers = references(game)
    groups = {}
    for path in sorted((HERE / "groups").glob("*.txt")):
        for line in path.read_text().splitlines():
            if line and not line.startswith("#"):
                groups[int(line.split()[0], 16)] = path.stem

    functions = {}
    outside_called, outside_callers = {}, {}
    for entry in sorted(wanted):
        function = game.functions[entry]
        calls = specs_from_exe.callees(game, entry)
        here_callers = callers.get(entry, set())
        row = table[entry]
        x87 = sum(1 for i in function.insns if i.mnemonic.startswith("f") and i.mnemonic not in ("fs",))
        flags = data_flags(game, function, here_callers, pointers.get(entry, ()), row)
        indirect = [i.address for i in function.insns
                    if i.mnemonic == "call" and i.operands[0].type != exe_index.IMM
                    and not (i.operands[0].type == exe_index.MEM and (i.operands[0].disp & 0xffffffff) in game.imports)]
        functions["%x" % entry] = {
            "callers": ["%x" % c for c in sorted(here_callers)],
            "pointers": ["%x" % p for p in sorted(pointers.get(entry, ()))][:12],
            "callees": ["%x" % c for c in sorted(calls)],
            "strings": strings_named(game, function),
            "x87": x87,
            "instructions": len(function.insns),
            "indirect_calls": ["%x" % a for a in indirect],
            "row": specs_from_exe.row_text(entry, row),
            "flags": flags,
            "in_group": groups.get(entry, ""),
        }
        if not flags:
            for c in calls - wanted:
                outside_called[c] = outside_called.get(c, 0) + 1
            for c in here_callers - wanted:
                outside_callers[c] = outside_callers.get(c, 0) + 1

    options.out.mkdir(parents=True, exist_ok=True)
    path = options.out / (options.name + ".todo.json")
    path.write_text(json.dumps({"name": options.name, "absent": ["%x" % a for a in absent], "functions": functions}, indent=1))

    flagged = {a: f["flags"] for a, f in functions.items() if f["flags"]}
    real = [a for a in functions if a not in flagged]
    print("%s: %d functions, %d look real, %d flagged, %d no function at that address -> %s"
          % (options.name, len(functions), len(real), len(flagged), len(absent), path))
    for address, flags in sorted(flagged.items()):
        print("  %s  %s" % (address, " ".join(flags)))
    taken = [a for a, f in functions.items() if f["in_group"]]
    if taken:
        print("already in a group: " + " ".join(sorted(taken)))
    print("instructions in the real ones: %d, x87 in them: %d"
          % (sum(functions[a]["instructions"] for a in real), sum(functions[a]["x87"] for a in real)))
    top = sorted(outside_called.items(), key=lambda item: -item[1])[:15]
    print("calls most from outside: " + " ".join("%x(%d)" % item for item in top))
    top = sorted(outside_callers.items(), key=lambda item: -item[1])[:15]
    print("called from outside by: " + " ".join("%x(%d)" % item for item in top))
    named = sorted({s for a in real for s in functions[a]["strings"]})[:30]
    if named:
        print("strings named: " + " | ".join(re.sub(r"\s+", " ", s)[:40] for s in named))


if __name__ == "__main__":
    main()
