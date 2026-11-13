"""A group from a copy of the tree into this one.

    python tools/frontend_checks/merge_group.py <copy-dir> <group> [--check] [--anyway]

Copies src/nfs3hp/native_frontend_<group>.cpp and groups/<group>{.txt,_world.inc,_entry.inc,
_watched.inc,_hooks.inc,_decls.inc,_specs.inc} in.  Before that it says what would go wrong:

  an address or a name two groups have, or a native of another table already has
    (disassemble_nfs3hp.py would quietly take the last): nothing is copied, unless --anyway;
  an address that is no function of the exe.

And what the merge now means without anyone moving it (port_sites.py does that by address):

  each tool's site inside the group's functions: gone from the tool in the generator's run,
    made to the code the native is held against, so the native must carry the change;
  what those sites call that nothing here stands in for: the group's _hooks.inc and
    _decls.inc must have it, or the check will not link;
  each hook on a function now native, which no longer runs unless the native calls it.

--check says all this and copies nothing.  Then run disassemble_nfs3hp.py and
tools/frontend_checks/windows/run.py.
"""
import argparse
import re
import shutil
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
sys.path.insert(0, str(ROOT))
sys.path.insert(0, str(ROOT / "disasm"))
sys.path.insert(0, str(HERE))

SUFFIXES = (".txt", "_world.inc", "_entry.inc", "_watched.inc", "_hooks.inc", "_decls.inc", "_specs.inc")
CALL = re.compile(r"\b((?:\w+::)*\w+)\s*\(")
# Not what a patch calls of the port's: the generated code's own vocabulary.
KNOWN = {"app", "cpu", "x86", "if", "return", "sync", "reload", "getMemory", "static_cast", "reg32", "reg8", "reg16", "Float",
         "set_szp", "terminate", "sub_", "const", "float"}


def rows(path):
    return [line.split() for line in path.read_text().splitlines() if line and not line.startswith("#")]


def duplicates(groups):
    """Lines saying who has an address or a name twice: groups is {group: rows}."""
    addresses, names = {}, {}
    for group, table in groups.items():
        for row in table:
            addresses.setdefault(int(row[0], 16), []).append(group)
            names.setdefault(row[1], []).append(group)
    out = []
    for address, owners in sorted(addresses.items()):
        if len(owners) > 1:
            out.append("address %x in %s" % (address, " and ".join(owners)))
    for name, owners in sorted(names.items()):
        if len(owners) > 1:
            out.append("name %s in %s" % (name, " and ".join(owners)))
    return out


def other_tables(nfs3hp, addresses):
    """A group's address the generator already has as another kind of native: a hand-kept row
    of MENU_NATIVES or any other table GAME_NATIVES was made from."""
    from_groups = {int(row[0], 16) for path in sorted((HERE / "groups").glob("*.txt")) for row in rows(path)}
    out = []
    for address in sorted(addresses):
        if address in nfs3hp.GAME_NATIVES and address not in from_groups:
            out.append("address %x is already native as %s" % (address, nfs3hp.GAME_NATIVES[address]))
    return out


def called_by(patch):
    """What a patch's new code calls that its old code does not."""
    def calls(text):
        return {m.group(1) for m in CALL.finditer(re.sub(r"/\*.*?\*/", "", text, flags=re.S))}
    return {name for name in calls(patch.new) - calls(patch.old)
            if name.split("::")[-1] not in KNOWN and not name.startswith(("sub_", "x86::", "nfs3hp::", "cpu."))}


def stands_in(name, group_hooks):
    """Whether the check has a definition for what a patch calls."""
    plain = name.split("::")[-1]
    pattern = re.compile(r"\b%s\s*\(" % re.escape(plain))
    if any(pattern.search(text) for text in group_hooks):
        return True
    return bool(pattern.search((HERE / "frontend_checks.cpp").read_text(errors="replace")))


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("copy", type=Path)
    parser.add_argument("group")
    parser.add_argument("--check", action="store_true", help="say what the merge would do, copy nothing")
    parser.add_argument("--anyway", action="store_true", help="copy despite duplicates")
    options = parser.parse_args()
    group = options.group
    incoming = options.copy / "tools/frontend_checks/groups"
    table_path = incoming / (group + ".txt")
    source = options.copy / "src/nfs3hp" / ("native_frontend_%s.cpp" % group)
    if not table_path.exists() or not source.exists():
        sys.exit("%s has no group %s" % (options.copy, group))

    table = rows(table_path)
    groups = {path.stem: rows(path) for path in sorted((HERE / "groups").glob("*.txt")) if path.stem != group}
    groups[group] = table
    problems = duplicates(groups)

    import exe_index
    import port_sites
    import disassemble_nfs3hp as nfs3hp
    addresses = {int(row[0], 16) for row in table}
    problems += other_tables(nfs3hp, addresses)
    game = exe_index.load()
    problems += ["address %x is no function of the exe" % a for a in sorted(addresses) if a not in game.functions]
    problems += ["%x is changed by text of its own in %s: not by a list, merge it by hand" % (a, port_sites.WHOLE_FUNCTION[a])
                 for a in sorted(addresses) if a in port_sites.WHOLE_FUNCTION]
    for problem in problems:
        print("PROBLEM: " + problem)

    functions = {entry: [i.address for i in f.insns] for entry, f in game.functions.items()}
    natives = {a: nfs3hp.MENU_NATIVES.get(a, "frontend::" + next(r[1] for r in table if int(r[0], 16) == a)) for a in addresses}
    moved = port_sites.sites_of_natives(functions, natives)
    hook_texts = [path.read_text(errors="replace") for path in sorted(incoming.glob("*_hooks.inc"))]
    hook_texts += [path.read_text(errors="replace") for path in sorted((HERE / "groups").glob("*_hooks.inc"))]
    for entry, patches in sorted(moved.items()):
        for patch in patches:
            print("site %s in sub_%x (%s): the native must carry it; made to the code it is held against"
                  % (patch.instruction, entry, patch.tool))
            for name in sorted(called_by(patch)):
                if not stands_in(name, hook_texts):
                    print("    it calls %s, which nothing stands in for: %s_hooks.inc and _decls.inc" % (name, group))
    for tool, name, address, hook in port_sites.hooks_of_natives(natives):
        print("hook %s on sub_%x (%s): runs no more unless the native calls it" % (hook, address, tool))

    if options.check or (problems and not options.anyway):
        print("nothing copied" + (": %d problems" % len(problems) if problems else ""))
        sys.exit(1 if problems else 0)
    copied = 0
    for suffix in SUFFIXES:
        path = incoming / (group + suffix)
        if path.exists():
            shutil.copyfile(path, HERE / "groups" / path.name)
            copied += 1
    shutil.copyfile(source, ROOT / "src/nfs3hp" / source.name)
    print("%s: %d functions, %d files copied. Now disassemble_nfs3hp.py, then tools/frontend_checks/windows/run.py."
          % (group, len(table), copied + 1))
    sys.exit(1 if problems else 0)


if __name__ == "__main__":
    main()
