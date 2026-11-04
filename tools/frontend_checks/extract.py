"""Copies generated functions out of src/nfs3hp/disassembly, without hooks.

    python3 extract.py <disassembly dir> <out.cpp> <mocks.inc> [--native <file>] sub_... sub_...

Every port hook (`/* port: native (tools/apply_*.py) */`) is taken out of
them on the way: what is left is the generated code alone, for
frontend_checks.cpp to compare the decompiled functions with.  Each function
they call that is not among them goes to <mocks.inc> as a stand-in that
records the call (see frontend_checks.cpp).  A screen that is native whole
has no generated code in the tree: its function comes from the file
--native names, which generate.py writes, and the tree's nfs3hp.h, which
declares no Application::sub_ for it, is copied next to <out.cpp> with the
declarations it lacks.  The game's natives -- the menus' own, the controls',
the sound's, the graphics', EA's, any of nfs3hp's -- they call by address
instead of by name, so that each goes to its stand-in as the decompiled
code's calls do.
"""
import re
import sys
from pathlib import Path

HOOK = re.compile(r"    if \(\w+\(app, cpu(?:\.sync\(\))?\)\) "
                  r"/\* port: native \(tools/apply_\w+\.py\) \*/\r?\n"
                  r"    \{\r?\n(?:        cpu\.reload\(\);\r?\n)?        return;\r?\n    \}\r?\n")
CALL = re.compile(r"\bsub_([0-9a-f]{6})\(app")
# What the generated code calls by address that frontend_checks.cpp runs as
# it is: __CHP, the truncation before a fistp.
REAL = {"sub_4dfd56": "watcom::CHP"}
# The menus' functions native whole that are tested here and that the other
# functions tested call: those call them by address, to their stand-ins in
# frontend_checks.cpp, as before they were whole (kStandInElsewhere there).
STAND_IN_ELSEWHERE = ("sub_442a40", "sub_44a660", "sub_4446c0")
NATIVE = re.compile(r"^/\* sub_([0-9a-f]+): native, nfs3hp::((?:\w+::)*\w+) \*/$", re.M)


def main():
    source = Path(sys.argv[1])
    out = Path(sys.argv[2])
    mocks = Path(sys.argv[3])
    names = sys.argv[4:]
    sources = sorted(source.glob("nfs3hp.*.cpp"))
    if names[:1] == ["--native"]:
        sources.append(Path(names[1]))
        names = names[2:]
    wanted = set(names)
    found = {}
    natives = {}
    for path in sources:
        text = path.read_text(errors="replace")
        natives.update({name: int(address, 16) for address, name in NATIVE.findall(text)})
        lines = text.split("\n")
        i = 0
        while i < len(lines):
            m = re.match(r"^void Application::(sub_[0-9a-f]+)\(", lines[i])
            if m and m.group(1) in wanted and m.group(1) not in found:
                end = i + 1
                while lines[end].rstrip("\r") != "}":
                    end += 1
                found[m.group(1)] = HOOK.sub("", "\n".join(lines[i:end + 1]))
                i = end
            i += 1
    for name in found:
        for native, address in natives.items():
            found[name] = found[name].replace("nfs3hp::%s(app, cpu.sync());" % native,
                                              "app->dynamic_call(0x%x, cpu.sync());" % address)
    # The groups' functions, called by address too: the natives do, and in a process
    # that is not of their group both sides then go to the stand-in.
    in_groups = tuple("sub_" + line.split()[0]
                      for group in sorted((Path(__file__).resolve().parent / "groups").glob("*.txt"))
                      for line in group.read_text().splitlines() if line and not line.startswith("#"))
    for name in found:
        for other in STAND_IN_ELSEWHERE + in_groups:
            if other != name:
                found[name] = found[name].replace("%s(app, cpu.sync());" % other,
                                                  "app->dynamic_call(0x%s, cpu.sync());" % other[4:])
    missing = wanted - set(found)
    if missing:
        raise SystemExit("not found: " + " ".join(sorted(missing)))
    called = set()
    with out.open("w") as f:
        f.write('#include "nfs3hp.h"\n\nnamespace nfs3hp\n{\n\n')
        # What the port's own changes to them call (tools/apply_text_entry.py).
        f.write("void textEntry(bool taking);\n\n")
        f.write("x86::reg32 networkPort();\n\n")
        f.write('#include "group_decls.inc"\n\n')
        f.write("x86::reg32 shownDepth(x86::reg32 depth);\n\n")
        f.write("void dynamicFullscreenLabel(win32::WinApplication* app, x86::reg32 entry);\n\n")
        for name in sorted(found):
            if "/* port: native" in found[name]:
                raise SystemExit("a hook left in " + name)
            f.write(found[name] + "\n\n")
            called |= {"sub_" + c for c in CALL.findall(found[name])}
        f.write("}\n")
    with mocks.open("w") as f:
        f.write("// Written by extract.py: the generated functions' callees, recorded.\n")
        for name in sorted(called - set(found)):
            if name in REAL:
                # The game's own, native: run, not recorded.
                f.write("void Application::%s(WinApplication* app, x86::CPU& cpu) { %s(app, cpu); }\n"
                        % (name, REAL[name]))
                continue
            # Through the dispatch, for a stand-in a group's world has made its own to answer
            # calls by name as well as by address.
            f.write("void Application::%s(WinApplication* app, x86::CPU& cpu) { app->dynamic_call(0x%s, cpu); }\n"
                    % (name, name[4:]))
    with Path(str(mocks) + ".generated").open("w") as f:
        for name in sorted(found):
            f.write("{0x%s, &Application::%s},\n" % (name[4:], name))
    header = (source / "nfs3hp.h").read_text()
    declared = set(re.findall(r"static void (sub_[0-9a-f]+)\(", header))
    lacking = sorted((set(found) | called) - declared)
    at = header.index("    static void sub_")
    header = header[:at] + "".join("    static void %s(WinApplication* app, x86::CPU& cpu);\n" % name
                                   for name in lacking) + header[at:]
    # The groups' functions (groups/*.txt), which a tree not generated since has not declared.
    for group in sorted((Path(__file__).resolve().parent / "groups").glob("*.txt")):
        for line in group.read_text().splitlines():
            if line and not line.startswith("#"):
                function = line.split()[1]
                declaration = "void %s(win32::WinApplication* app, x86::CPU& cpu);" % function
                if declaration not in header:
                    header += "\nnamespace nfs3hp { namespace frontend { %s } }\n" % declaration
    (out.parent / "nfs3hp.h").write_text(header)


if __name__ == "__main__":
    main()
