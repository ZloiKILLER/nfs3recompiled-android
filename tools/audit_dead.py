#!/usr/bin/env python3
"""Prove which functions of nfs3.exe can never run, and stub them.

Disassembles nfs3.exe as disassemble_nfs3hp.py does and computes what can be
reached, erring on the side of live everywhere:

roots
  - the entry point, every export, the generator's KNOWN_SUBROUTINES,
    THREAD_ROUTINES, MERGE_ROUTINES and SPLIT_INSTRUCTIONS
  - every function bound to a native (GAME_NATIVES) but the dead stubs
  - every address in the code of what is built into the game: src/ (not the
    generated disassembly, not the DEAD lists), include/, decomp/, android/app/jni,
    the patches in tools/apply_*.py and disassemble_nfs3hp.py, and not in comments
    or notes -- with a native's code gone what it calls is what its source names,
    so its x86 no longer counts unless asked (--natives-as-x86) and neither do
    the checkers, which name what they stand in for
  - every pointer the image holds outside instructions: the value at each base
    relocation whose site no instruction covers, and every 4-byte window, at
    any alignment, of every section that is not code and of the code bytes
    that are no instruction (jump tables, data in the code)
edges, from a function that is not a native to
  - every immediate and memory displacement of each of its instructions:
    direct calls, jumps, jcc, push offset, mov reg, offset, call [table]...
  - the vtables' and jump tables' destinations the disassembler resolved
  - the value at each relocation inside its instructions
  - the next function, when its last instruction can run on into it
An address counts for every function whose range [start, end) holds it, not
only the one it begins, and for the phony entries split out of that range.

A function nothing of that reaches can only be entered by an address computed
at run time from nothing in the image, which Watcom's code does not do.  Each
is a stub that stops the game with its address (src/nfs3hp/native_dead.inc,
native_dead.cpp), so if one is ever entered it says so instead of running.

The DEAD lists (native_dead.inc, native_snd_dead.inc,
native_graphics_dead.inc) are not roots.  Every run proves native_dead.inc
again and fails if one of it is reachable.  The other two stand on more than
references -- the plain mixer is in the game's tables, but the port's CPU has
MMX and they hand out the MMX set only -- so for them it only says how many it
proves from references alone; what they reach counts as live all the same.

Usage: tools/audit_dead.py [--write]   (from the repository root; needs capstone)
  --write  rewrite src/nfs3hp/native_dead.inc with every dead function that
           is still generated, and those already in it
"""
import argparse
import bisect
import collections
import glob
import os
import re
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
os.chdir(ROOT)
sys.path.insert(0, str(ROOT))
sys.path.insert(0, str(ROOT / "disasm"))

DEAD_FILES = ["src/nfs3hp/native_dead.inc", "src/nfs3hp/native_snd_dead.inc",
              "src/nfs3hp/native_graphics_dead.inc", "src/nfs3hp/native_pieces.inc"]
OWN = "src/nfs3hp/native_dead.inc"
DEAD_LINE = re.compile(r"^DEAD\((0x[0-9a-fA-F]+)\)", re.M)
ADDRESS = re.compile(r"(?:0x|sub_|loc_|L_0x)0*([0-9a-fA-F]{6,8})\b")
SOURCES = ["src/**/*.cpp", "src/**/*.h", "src/**/*.inc", "src/**/*.hpp", "src/**/*.c", "include/**/*.h", "include/**/*.inc",
           "include/**/*.hpp", "decomp/**/*.c", "decomp/**/*.h", "decomp/**/*.cpp", "android/app/jni/**/*.c",
           "android/app/jni/**/*.cpp", "android/app/jni/**/*.h", "tools/apply_*.py", "disassemble_nfs3hp.py"]
COMMENT = re.compile(r"/\*.*?\*/|//[^\n]*", re.S)
DECIMAL = re.compile(r"(?<![\w.])([1-9][0-9]{6})(?![\w.])")
TERMINAL = ("ret", "retf", "iret", "iretd", "jmp", "ljmp", "hlt")


def dead_list(path):
    p = ROOT / path
    return [int(a, 16) for a in DEAD_LINE.findall(p.read_text())] if p.exists() else []


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--write", action="store_true")
    ap.add_argument("--why", nargs="*", default=[], metavar="ADDRESS",
                    help="how each of these functions is reached, from which root")
    ap.add_argument("--natives-as-x86", action="store_true",
                    help="the old proof: a native's original code counts, and every source and checker")
    args = ap.parse_args()

    import disassemble_nfs3hp as D
    from disasm import disassembler
    from capstone import x86 as X
    import pefile

    stubbed = {f: set(dead_list(f)) for f in DEAD_FILES}
    all_stubbed = set().union(*stubbed.values())

    import contextlib
    import io
    with contextlib.redirect_stdout(io.StringIO()):  # the disassembler's notes
        app = disassembler.disassemble("nfs3hp", "nfs3hp/nfs3.exe", D.DATA_SEGMENTS,
                                       D.KNOWN_SUBROUTINES, D.SPLIT_INSTRUCTIONS,
                                       D.THREAD_ROUTINES, D.THREAD_SEGMENTS, D.MERGE_ROUTINES)
    funcs = {s.get_entry_point(): s for s in app.subroutines}
    ranges = sorted((s.get_start_address(), s.get_end_address(), e) for e, s in funcs.items())
    starts = [r[0] for r in ranges]
    longest = max(r[1] - r[0] for r in ranges)

    def owners(address):
        found = [address] if address in funcs else []
        i = bisect.bisect_right(starts, address) - 1
        while i >= 0 and ranges[i][0] >= address - longest:
            if ranges[i][0] <= address < ranges[i][1]:
                found.append(ranges[i][2])
            i -= 1
        return found

    covered = set()
    for s in app.subroutines:
        if not s.phony:
            for ins in s.instructions:
                covered.update(range(ins.address, ins.address + ins.size))

    edges = collections.defaultdict(set)
    for e, s in funcs.items():
        if e in D.GAME_NATIVES and not args.natives_as_x86:
            continue
        for ins in s.instructions:
            for op in getattr(ins, "operands", []):
                if op.type == X.X86_OP_IMM:
                    edges[e].add(op.imm & 0xffffffff)
                elif op.type == X.X86_OP_MEM:
                    edges[e].add(op.mem.disp & 0xffffffff)
            edges[e].update(getattr(ins, "potential_destinations", []))
        edges[e].update(t for _, t in s.calls + s.static_labels + s.dynamic_labels)
    real = sorted((s for s in app.subroutines if not s.phony), key=lambda s: s.get_start_address())
    for a, b in zip(real, real[1:]):
        if a.instructions[-1].mnemonic.split()[-1] not in TERMINAL and (
                args.natives_as_x86 or a.get_entry_point() not in D.GAME_NATIVES):
            edges[a.get_entry_point()].add(b.get_start_address())

    roots = {app.entry_point}
    roots.update(a for a, _ in app.exported_symbols)
    roots.update(D.KNOWN_SUBROUTINES)
    roots.update(D.THREAD_ROUTINES)
    roots.update(a for pair in D.MERGE_ROUTINES for a in pair)
    roots.update(D.SPLIT_INSTRUCTIONS)
    roots.update(a for a in D.GAME_NATIVES if a in funcs and a not in all_stubbed)

    skip = {os.path.normpath(str(ROOT / f)) for f in DEAD_FILES}
    named = set()
    sources = SOURCES
    if args.natives_as_x86:
        sources = ["src/**/*", "include/**/*", "tools/**/*", "decomp/**/*", "android/app/jni/**/*", "cmake/**/*",
                   "disasm/**/*.py", "disassemble_nfs3hp.py"]
    for pattern in sources:
        for f in map(os.path.normpath, glob.glob(str(ROOT / pattern), recursive=True)):
            # Paths with "/" whatever the system's separator; what the generators and
            # the checkers' builds wrote is no one's source.
            g = f.replace(os.sep, "/")
            if (f in skip or "/disassembly/" in g or "__pycache__" in g or "/build/" in g
                    or not os.path.isfile(f)):
                continue
            text = open(f, encoding="utf-8", errors="replace").read()
            # A patch's code names addresses in decimal with the hex beside it in a comment.
            if not args.natives_as_x86 and not f.endswith(".py"):
                text = COMMENT.sub(" ", text)
            named.update(int(h, 16) for h in ADDRESS.findall(text))
            named.update(int(d) for d in DECIMAL.findall(text))

    pe = pefile.PE("nfs3hp/nfs3.exe")
    base = pe.OPTIONAL_HEADER.ImageBase
    image = pe.get_memory_mapped_image()
    low, high = ranges[0][0], max(r[1] for r in ranges)
    pointers = set()
    for block in getattr(pe, "DIRECTORY_ENTRY_BASERELOC", []):
        for entry in block.entries:
            if entry.type == 0:
                continue
            site = entry.rva + base
            value = struct.unpack_from("<I", image, entry.rva)[0]
            if site in covered:
                for o in owners(site):
                    edges[o].add(value)
            else:
                pointers.add(value)
    for section in pe.sections:
        va = section.VirtualAddress
        blob = image[va:va + max(section.Misc_VirtualSize, section.SizeOfRawData)]
        code = bool(section.Characteristics & 0x20000000)
        for o in range(len(blob) - 3):
            if code and va + o + base in covered:
                continue
            value = struct.unpack_from("<I", blob, o)[0]
            if low <= value < high:
                pointers.add(value)
    # Why a root is one, for --why.
    root_reason = {}
    for r in roots:
        root_reason[r] = "a native or a listed routine"
    for address in sorted(pointers):
        for o in owners(address):
            root_reason.setdefault(o, "a pointer in the image, %x" % address)
    for address in sorted(named):
        for o in owners(address):
            root_reason.setdefault(o, "named in the source, %x" % address)
    for address in named | pointers:
        roots.update(owners(address))

    live = set()
    came_from = {}
    stack = [r for r in sorted(roots) if r in funcs]
    while stack:
        f = stack.pop()
        if f in live:
            continue
        live.add(f)
        for t in edges[f]:
            for o in owners(t):
                if o not in live:
                    came_from.setdefault(o, f)
                    stack.append(o)
    for w in args.why:
        a = int(w, 16)
        chain = []
        while a is not None and len(chain) < 40:
            chain.append(a)
            a = None if a in roots else came_from.get(a)
        print("%s: %s <- %s" % (w, " <- ".join("%x" % c for c in chain), root_reason.get(chain[-1], "not live")))

    generated = [e for e in funcs if e not in D.GAME_NATIVES]
    new_dead = sorted(e for e in generated if e not in live)
    print("functions %d, generated %d, live %d" % (len(funcs), len(generated), len(live)))
    failed = False
    for f, entries in stubbed.items():
        wrong = sorted(a for a in entries if a in live)
        print("%-38s %4d stubs, %4d proved dead from references" % (f, len(entries), len(entries) - len(wrong)))
        if f == OWN:
            for a in wrong:
                failed = True
                print("  REACHABLE: sub_%x" % a)
    print("generated and dead: %d, %d instructions" % (len(new_dead), sum(len(funcs[e].instructions) for e in new_dead)))
    if os.environ.get("AUDIT_LIST"):
        with open(os.environ["AUDIT_LIST"], "w") as f:
            f.write("\n".join("%x %d" % (e, len(funcs[e].instructions)) for e in new_dead) + "\n")

    if args.write:
        own = sorted((stubbed[OWN] - live) | set(new_dead))
        iat = {}
        for imp in getattr(pe, "DIRECTORY_ENTRY_IMPORT", []):
            for i in imp.imports:
                if i.name:
                    iat[i.address] = i.name.decode()

        def text_at(address):
            o = address - base
            if not 0 <= o < len(image):
                return None
            end = image.find(b"\0", o, o + 80)
            s = image[o:end] if end > o else b""
            return s.decode() if len(s) >= 4 and all(32 <= c < 127 for c in s) else None

        def note(e):
            s = funcs[e]
            apis, texts = [], []
            for ins in s.instructions:
                for op in getattr(ins, "operands", []):
                    v = (op.imm if op.type == X.X86_OP_IMM else
                         op.mem.disp if op.type == X.X86_OP_MEM else None)
                    if v is None:
                        continue
                    v &= 0xffffffff
                    if v in iat and iat[v] not in apis:
                        apis.append(iat[v])
                    t = text_at(v)
                    if t and t not in texts:
                        texts.append(t)
            parts = ["%d instructions" % len(s.instructions)]
            if apis:
                parts.append(", ".join(apis[:3]))
            if texts:
                parts.append('"%s"' % texts[0].replace("\\", "/")[:48])
            return "; ".join(parts)

        head = (ROOT / OWN).read_text().split("\nDEAD(")[0].rstrip("\n") if (ROOT / OWN).exists() else HEADER
        lines = [head, ""]
        for e in own:
            lines.append("DEAD(0x%x)  // %s" % (e, note(e)))
        (ROOT / OWN).write_text("\n".join(lines) + "\n")
        print("wrote %s: %d" % (OWN, len(own)))
    sys.exit(1 if failed else 0)


HEADER = """\
// The game's code that nothing can reach, one DEAD(address) a function:
// native_dead.cpp makes each a stub that stops the game with its address, and
// disassemble_nfs3hp.py generates no code for it.
//
// tools/audit_dead.py proves each, and writes this list (--write): no call,
// jump, immediate or displacement of it in any instruction that can run, the
// natives' original code included; no pointer to it, or into it, anywhere in
// the image outside instructions (every relocation, every 4-byte window of the
// data); no address of it in the port's hand-written source; and no running on
// into it from the function before.  Most are what the linker brought in with
// the libraries' objects -- the modem and serial links (TAPI, RAS, COM ports),
// the C runtime's unused functions, EA's unused helpers.
//
// After each the instructions in it, the Windows functions it calls and the
// first text it names, for reading."""

if __name__ == "__main__":
    main()
