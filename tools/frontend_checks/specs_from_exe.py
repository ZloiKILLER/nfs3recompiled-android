"""Stand-in rows for frontend_checks.cpp's kSpecs, read off nfs3.exe.

    python tools/frontend_checks/specs_from_exe.py <group> [--write]
        the rows for what the group's functions call and no table has yet;
        --write puts them in groups/<group>_specs.inc, which windows/run.py adds
        to kSpecs in that group's processes (a row kSpecs has itself wins)
    python tools/frontend_checks/specs_from_exe.py 0x4d1000 0x48a220 ...
        the rows for these addresses
    python tools/frontend_checks/specs_from_exe.py --validate [--detail]
        every row in kSpecs and in the groups' worlds against what is derived
    python tools/frontend_checks/specs_from_exe.py --show 0x4ef1e0
        the function's code with what the reading saw at each instruction

A row is { reads, stack words, what its ret pops, what it changes besides eax, kind },
Spec's own fields.  How each is read:

  reads    Where a register's value goes is followed, not the instructions that
           name it: a push is not a read, nor a mov, nor a pop; `xor r,r` and
           `sub r,r` only write.  A value is read when it is used: in arithmetic,
           a comparison, an address, a store outside the stack, a callee that
           reads that register or the stack word it sits in.  So `push edx; ...
           pop edx` reads nothing and `push eax; call f` (f popping a word) reads
           eax.  al, ah and ax are their own parts of a register.  Two things stay
           out of the mask and are said in the row's comment: the upper bits of
           `mov bx,[..]; add eax,ebx`, and esi or edi only left in another register:
           leftovers, which a row reading them would hold a native to.
  clobbers Whatever does not hold the value it came in with at some ret: pairs
           and frame saves restore; a mov from another register changes (and
           reads that other one).  Calls count by their callee's own answer, to a
           fixed point, recursion included.  eax is not in the row: mock() always
           writes it.  A function that keeps eax comes out Kind::NoEax.
  pops     The ret's own number; through a tail jump, the target's.
  stack    The words pops says, or more where the code reads above its return
           address (the callers clean up: sprintf's kind).  Not 0 as a row by hand
           may have it: what is not listed here mock() does not record.

What the code does not say:

  imports and what GetProcAddress gave  stdcall: eax, ecx and edx change, ebx, esi, edi
           do not, and the ret pops the arguments (the imports' counts from
           include/winapi, the pointers' from the pushes before the call).  Not flagged.
  indirect call  `call eax`, `call [eax+0x64]`, a pointer in another global: assumed to
           change eax, ebx, ecx and edx and read nothing more.  Of the 1125 functions whose
           address the game takes, all but some 30 stay within that.  A row shows the
           assumption only where it reaches the row: a register changed by nothing else,
           or a register still as it came in at that call ("may read"), which is not in
           the mask.  Most such calls are behind saves and do not show.  Its pops are taken
           as the pushes right before it, unless the caller cleans up; where the rets then
           do not come out level the other answers are tried.
  stack lost     esp not 0 at a ret, even so, or not known: what pops restored there is
           not trusted, so clobbers can be too many, and no reads are drawn from it.
           Said in the row; not carried to callers, which it can only make over-cautious.
  x87      fpu_frame.py's own answer, the generator's: a row that leaves something
           there says so, as no Kind::Plain stand-in does it.  It wants a Kind of its own.
  kind     Only Plain and NoEax: what a Bool, a Shape or a string's recording are for is
           what the caller does with the result, which is not in the callee.
"""
import argparse
import hashlib
import json
import re
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import exe_index  # noqa: E402

ROOT = HERE.parents[1]
GROUPS = HERE / "groups"
CACHE = exe_index.BUILD / "exe_specs.json"

SPEC_REGS = ("eax", "ebx", "ecx", "edx", "esi", "edi")
TRACKED = SPEC_REGS + ("ebp",)
NAMES = {"eax": "EAX", "ebx": "EBX", "ecx": "ECX", "edx": "EDX", "esi": "ESI", "edi": "EDI"}
# The parts of a register: its low byte, its next byte, its upper half.
PARTS = {}
for _full, _word, _high, _low in (("eax", "ax", "ah", "al"), ("ebx", "bx", "bh", "bl"), ("ecx", "cx", "ch", "cl"),
                                  ("edx", "dx", "dh", "dl")):
    PARTS.update({_full: (_full, (0, 1, 2)), _word: (_full, (0, 1)), _high: (_full, (1,)), _low: (_full, (0,))})
for _full, _word in (("esi", "si"), ("edi", "di"), ("ebp", "bp")):
    PARTS.update({_full: (_full, (0, 1, 2)), _word: (_full, (0, 1))})

OTHER = "?"  # a value not any register's on entry
FRESH = frozenset((OTHER,))
FRESH3 = (FRESH, FRESH, FRESH)
# What an unknown callee is only assumed to have changed: told apart to the end, as most of
# these are saved and restored on the way up and then the assumption never shows.
ASSUMED = frozenset(("~",))
ASSUMED3 = (ASSUMED, ASSUMED, ASSUMED)
# The string instructions, which capstone's prefix handling leaves uneven: the registers they
# read, those they change.  The accumulator only as wide as the instruction.
STRING = {}
for _width, _acc in (("b", "al"), ("w", "ax"), ("d", "eax")):
    STRING["movs" + _width] = (("esi", "edi"), ("esi", "edi"))
    STRING["cmps" + _width] = (("esi", "edi"), ("esi", "edi"))
    STRING["stos" + _width] = ((_acc, "edi"), ("edi",))
    STRING["lods" + _width] = (("esi",), (_acc, "esi"))
    STRING["scas" + _width] = ((_acc, "edi"), ("edi",))
# What an unknown callee may change: the arguments' registers, as Watcom's callees
# here leave those they were given changed (kSpecs is full of it).  Not esi or edi.
INDIRECT_CLOBBERS = ("eax", "ebx", "ecx", "edx")
# stdcall, the Windows functions and the DLLs behind pointers.
IMPORT_CLOBBERS = ("eax", "ecx", "edx")


class Summary:
    __slots__ = ("reads", "clobbers", "assumed", "passes", "upper", "flows", "pops", "stack", "indirect", "lost", "ends",
                 "pop_values", "real", "unknown")

    def __init__(self):
        self.reads = set()
        self.clobbers = set()
        self.assumed = set()    # of clobbers, those only by what an unknown callee may do
        self.passes = set()     # registers still as they came in at an unknown call: it may read them
        self.upper = set()      # registers of which only the upper bits are read, as they came in
        self.flows = set()      # esi, edi as they came in, only left in another register
        self.pops = 0
        self.stack = 0
        self.indirect = set()   # the unknown calls under it, however far down
        self.lost = False       # its own stack, not its callees': theirs is only ever too many clobbers
        self.ends = False       # some path reaches a ret
        self.pop_values = set()
        self.real = set()       # during the reading: changed on a path by something known
        self.unknown = set()    # its own unknown call sites


class State:
    __slots__ = ("regs", "slots", "esp", "ebp")

    def __init__(self, regs, slots, esp, ebp):
        self.regs, self.slots, self.esp, self.ebp = regs, slots, esp, ebp

    def copy(self):
        return State(dict(self.regs), dict(self.slots), self.esp, self.ebp)


def entry_state():
    return State({r: (frozenset((r,)),) * 3 for r in TRACKED}, {}, 0, None)


def frame_label(offset):
    """ebp as a frame pointer, in its value: a save and a restore of it hand it on."""
    return (frozenset(("@%d" % offset,)),) * 3


def frame_of(value):
    labels = value[0]
    if len(labels) == 1:
        label = next(iter(labels))
        if label.startswith("@"):
            return int(label[1:])
    return None


def merge(a, b):
    """b into a; whether a changed."""
    changed = False
    for r, parts in b.regs.items():
        joined = tuple(x | y for x, y in zip(a.regs[r], parts))
        if joined != a.regs[r]:
            a.regs[r] = joined
            changed = True
    for at in set(a.slots) | set(b.slots):
        here, there = a.slots.get(at, FRESH3), b.slots.get(at, FRESH3)
        joined = tuple(x | y for x, y in zip(here, there))
        if a.slots.get(at) != joined:
            a.slots[at] = joined
            changed = True
    if a.esp != b.esp and a.esp is not None:
        a.esp = None
        changed = True
    if a.ebp != b.ebp and a.ebp is not None:
        a.ebp = None
        changed = True
    return changed


class Reader:
    """One function's reading, with what its callees are thought to do."""

    def __init__(self, game, summaries, entries):
        self.game, self.summaries, self.entries = game, summaries, entries
        # What comes right after a call, for a caller's own `add esp`.
        self.after_calls = {i.address + i.size for i in game.at.values() if i.mnemonic == "call"}
        self.dll_pointers = dll_pointers(game)
        self.pop_choice, self.pop_default = {}, True
        self.why, self.at = None, 0  # --show: where each read was seen

    def import_at(self, op):
        """The imported function a memory operand holds the pointer to."""
        if op.type == exe_index.MEM and not op.base and not op.index:
            return self.game.imports.get(op.disp & 0xffffffff)
        return None

    # ----- what a value's use means

    def use(self, out, value):
        """A value used, whole or the parts given.  Watcom's 16-bit arithmetic -- `mov bx,[..];
        add eax,ebx`, of which only the low half is ever kept -- reads a register's upper half
        as it came in: said in the row, not in its mask, as no caller means it as an argument."""
        whole = len(value) == 3
        for i, part in enumerate(value):
            for label in part:
                if label in NAMES:
                    if whole and label not in value[0]:
                        out.upper.add(label)
                        continue
                    out.reads.add(label)
                    if self.why is not None:
                        self.why.setdefault(label, self.at)

    def use_reg(self, out, state, name):
        if name in PARTS:
            full, parts = PARTS[name]
            self.use(out, [state.regs[full][p] for p in parts])

    def write_reg(self, state, name, value=None):
        """value: one set of labels per part written, or fresh."""
        if name not in PARTS:
            return
        full, parts = PARTS[name]
        current = list(state.regs[full])
        for i, p in enumerate(parts):
            current[p] = value[i] if value is not None else FRESH
        state.regs[full] = tuple(current)

    def reg_value(self, state, name):
        full, parts = PARTS[name]
        return [state.regs[full][p] for p in parts]

    # ----- the stack

    def stack_offset(self, state, op):
        """Where a memory operand is relative to esp on entry; None if not known to be the stack."""
        if op.type != exe_index.MEM or op.index:
            return None
        if op.base == "esp" and state.esp is not None:
            return state.esp + op.disp
        if op.base == "ebp" and state.ebp is not None:
            return state.ebp + op.disp
        return None

    def on_stack(self, op):
        return op.type == exe_index.MEM and op.base in ("esp", "ebp")

    def load(self, out, state, op):
        """A memory operand read: its address's registers used; its value if it is a word of the stack."""
        at = self.stack_offset(state, op)
        if at is None:
            self.address(out, state, op)
            return FRESH3
        if at >= 4:
            out.stack = max(out.stack, (at - 4) // 4 + 1)
            return FRESH3
        word = at & ~3
        value = state.slots.get(word, FRESH3)
        return value

    def address(self, out, state, op):
        """The registers in a memory operand's address used, the stack's own excepted."""
        if op.base and op.base not in ("esp",) and not (op.base == "ebp" and state.ebp is not None):
            self.use_reg(out, state, op.base)
        if op.index:
            self.use_reg(out, state, op.index)

    def store(self, out, state, op, value, whole=True):
        at = self.stack_offset(state, op)
        if at is None:
            self.address(out, state, op)
            # Out of the stack: whoever reads it gets it.
            self.use(out, value)
            return
        if at >= 4:
            # Its own argument overwritten: the caller's to know, not a register's.
            self.use(out, value)
            return
        word = at & ~3
        if whole and at == word:
            state.slots[word] = tuple(value) if len(value) == 3 else FRESH3
        else:
            self.use(out, value)
            state.slots[word] = FRESH3

    # ----- calls

    def apply_callee(self, out, state, summary, site):
        for r in summary.reads:
            self.use(out, state.regs[r])
        for r in summary.upper:
            out.upper |= {label for part in state.regs[r][1:] for label in part if label in NAMES}
        for r in summary.flows:
            out.flows |= {label for part in state.regs[r] for label in part if label in NAMES}
        if state.esp is not None:
            # At the call, before the return address: the callee's first argument at esp.
            for i in range(max(summary.stack, summary.pops // 4)):
                self.use(out, state.slots.get(state.esp + 4 * i, FRESH3))
            state.esp += summary.pops
        for r in summary.passes:
            for part in state.regs[r]:
                out.passes |= {label for label in part if label in NAMES}
        for r in summary.clobbers:
            state.regs[r] = ASSUMED3 if r in summary.assumed else FRESH3
        out.indirect |= summary.indirect

    def pushes_before(self, function, insn):
        """The arguments an unknown callee is taken to pop: the pushes right before the
        call, back to the last call or jump, the function's opening saves left out."""
        if function is None:
            return 0
        insns = function.insns
        try:
            i = next(k for k, x in enumerate(insns) if x is insn)
        except StopIteration:
            return 0
        opening = 0
        while opening < len(insns) and (
                (insns[opening].mnemonic == "push" and insns[opening].operands[0].type == exe_index.REG)
                or (insns[opening].mnemonic == "mov" and insns[opening].text == "ebp, esp")
                or (insns[opening].mnemonic == "sub" and insns[opening].text.startswith("esp,"))):
            opening += 1
        count = 0
        k = i - 1
        while k >= opening:
            m = insns[k].mnemonic
            if m == "push":
                count += 1
            elif m in ("call", "ret") or m.startswith("j") or m == "add" and insns[k].text.startswith("esp"):
                break
            k -= 1
        return count

    def unknown_callee(self, out, state, function, insn, next_insn, clobbers, value=FRESH3):
        """A call the code does not name.  Its arguments as above; a caller's own `add esp`
        after it means they stay for the caller to drop, and read_function tries the other
        answer where the function's rets do not then come out level."""
        cleaned = (next_insn is not None and next_insn.mnemonic == "add" and next_insn.text.startswith("esp,"))
        words = self.pushes_before(function, insn)
        pops = self.pop_choice.get(insn.address, self.pop_default and not cleaned)
        if words:
            out.unknown.add(insn.address)
        if state.esp is not None:
            for i in range(words):
                # Only "may": a guess at what is an argument must not put a saved register in the row.
                for part in state.slots.get(state.esp + 4 * i, FRESH3):
                    out.passes |= {label for label in part if label in NAMES}
            if pops:
                state.esp += 4 * words
        for r in clobbers:
            state.regs[r] = value

    def assumed_callee(self, out, state, function, insn, next_insn):
        """An unknown callee of the game's own: what it is taken to change is marked so."""
        out.indirect.add(insn.address)
        for r in ("eax", "ebx", "ecx", "edx"):
            for part in state.regs[r]:
                out.passes |= {label for label in part if label in NAMES}
        self.unknown_callee(out, state, function, insn, next_insn, INDIRECT_CLOBBERS, ASSUMED3)

    def call(self, out, state, function, insn, next_insn):
        op = insn.operands[0]
        if op.type == exe_index.IMM:
            summary = self.summaries.get(op.imm)
            if summary is None:
                self.assumed_callee(out, state, function, insn, next_insn)
            else:
                self.apply_callee(out, state, summary, insn.address)
            return
        if op.type == exe_index.REG:
            self.use_reg(out, state, op.reg)
        else:
            self.address(out, state, op)
        name = self.import_at(op)
        if name is not None:
            self.import_call(out, state, name, next_insn)
            return
        if op.type == exe_index.MEM and not op.base and not op.index and (op.disp & 0xffffffff) in self.dll_pointers:
            # What GetProcAddress gave: stdcall, in another DLL.
            self.unknown_callee(out, state, function, insn, next_insn, IMPORT_CLOBBERS)
            return
        known = [self.summaries[d] for d in insn.destinations if d in self.summaries]
        if known:
            # A table of the code's own: any of them.
            base = state.copy()
            first = True
            for summary in known:
                branch = base.copy()
                self.apply_callee(out, branch, summary, insn.address)
                if first:
                    state.regs, state.slots, state.esp, state.ebp = branch.regs, branch.slots, branch.esp, branch.ebp
                    first = False
                else:
                    merge(state, branch)
            return
        self.assumed_callee(out, state, function, insn, next_insn)

    def import_call(self, out, state, name, next_insn):
        words = self.game.params.get(name)
        if words is None:
            words = 0
            out.lost = True
        if state.esp is not None:
            for i in range(words):
                self.use(out, state.slots.get(state.esp + 4 * i, FRESH3))
            # stdcall, but for the one that cannot be: an `add esp` after the others is locals.
            if "printf" not in name:
                state.esp += 4 * words
        for r in IMPORT_CLOBBERS:
            state.regs[r] = FRESH3

    # ----- leaving

    def leave(self, out, state, pops):
        out.ends = True
        out.pop_values.add(pops)
        if state.esp != 0:
            out.lost = True
        for r in TRACKED:
            own = frozenset((r,))
            for part in state.regs[r]:
                if part != own:
                    if r in NAMES:
                        out.clobbers.add(r)
                        if part - own - ASSUMED:
                            out.real.add(r)
                        else:
                            out.assumed.add(r)
                    # A value that came in in another register and left in this one: read.
                    # Not where the stack is lost: what a pop gave there may be anyone's.  Nor
                    # esi and edi, where Watcom passes nothing: that is a variable the compiler
                    # never set (sub_50e150's `mov eax,edi`), and a row reading it would hold a
                    # native to the caller's leftovers.  Said in the row.
                    for label in part:
                        if label in NAMES and label != r and state.esp == 0:
                            (out.flows if label in ("esi", "edi") else out.reads).add(label)

    # ----- one instruction: the states to go on with

    def step(self, out, state, function, insn, next_insn, after_call_cleanup=False):
        """Changes `state` to the state after the instruction.  Its successors: a list of
        addresses, or None for a ret."""
        m = insn.mnemonic.split()[-1] if insn.mnemonic.startswith(("rep ", "repe ", "repne ")) else insn.mnemonic
        prefixed = m != insn.mnemonic
        ops = insn.operands
        following = insn.address + insn.size
        two_regs = len(ops) == 2 and ops[0].type == exe_index.REG and ops[1].type == exe_index.REG

        if m in ("nop", "fwait", "wait", "cld", "std", "clc", "stc", "cmc", "cli", "sti", "fnop", "emms", "fninit"):
            return [following]

        if m == "ret" or m == "retn":
            pops = ops[0].imm if ops else 0
            self.leave(out, state, pops)
            return None

        if m == "call":
            self.call(out, state, function, insn, next_insn)
            return [following]

        if m == "jmp" or m.startswith("j") or m.startswith("loop"):
            return self.jump(out, state, function, insn, m, ops, following)

        if m == "push" or m == "pushal" or m == "pushad":
            if m != "push":
                for r in ("eax", "ecx", "edx", "ebx", None, "ebp", "esi", "edi"):
                    if state.esp is not None:
                        state.esp -= 4
                        state.slots[state.esp] = tuple(state.regs[r]) if r else FRESH3
                return [following]
            op = ops[0]
            if op.type == exe_index.REG and op.reg in PARTS and op.size == 4:
                value = tuple(state.regs[PARTS[op.reg][0]])
            elif op.type == exe_index.MEM:
                value = self.load(out, state, op)
            else:
                value = FRESH3
            if state.esp is not None:
                state.esp -= 4
                state.slots[state.esp] = value
            return [following]

        if m == "pop" or m == "popal" or m == "popad":
            if m != "pop":
                for r in ("edi", "esi", "ebp", None, "ebx", "edx", "ecx", "eax"):
                    if state.esp is not None:
                        if r:
                            state.regs[r] = state.slots.get(state.esp, FRESH3)
                        state.esp += 4
                state.ebp = frame_of(state.regs["ebp"])
                return [following]
            op = ops[0]
            value = state.slots.get(state.esp, FRESH3) if state.esp is not None and state.esp < 4 else FRESH3
            if state.esp is not None:
                state.esp += 4
            if op.type == exe_index.REG:
                if op.reg == "ebp":
                    state.ebp = frame_of(value)
                if op.reg in PARTS and op.size == 4:
                    state.regs[PARTS[op.reg][0]] = tuple(value)
                elif op.reg == "esp":
                    state.esp = None
            else:
                self.store(out, state, op, value)
            return [following]

        if m == "pushfd" or m == "pushf":
            if state.esp is not None:
                state.esp -= 4
                state.slots[state.esp] = FRESH3
            return [following]
        if m == "popfd" or m == "popf":
            if state.esp is not None:
                state.esp += 4
            return [following]
        if m == "leave":
            state.esp = state.ebp + 4 if state.ebp is not None else None
            value = state.slots.get(state.ebp, FRESH3) if state.ebp is not None else FRESH3
            state.regs["ebp"] = tuple(value)
            state.ebp = frame_of(value)
            return [following]
        if m == "enter" and state.esp is not None:
            state.esp -= 4
            state.slots[state.esp] = tuple(state.regs["ebp"])
            state.ebp = state.esp
            state.regs["ebp"] = frame_label(state.esp)
            state.esp -= ops[0].imm
            return [following]

        # The idioms that name a register they do not read.
        if m in ("xor", "sub", "sbb", "pxor") and two_regs and ops[0].reg == ops[1].reg and ops[0].reg in PARTS:
            self.write_reg(state, ops[0].reg)
            return [following]
        if m == "mov" and two_regs and ops[0].reg == ops[1].reg:
            return [following]
        if m == "xchg" and two_regs and ops[0].reg == ops[1].reg:
            return [following]
        if m == "lea" and ops[0].type == exe_index.REG:
            return self.lea(out, state, ops, following)

        if m == "mov":
            return self.mov(out, state, ops, following)

        if m == "xchg" and two_regs and ops[0].reg in PARTS and ops[1].reg in PARTS:
            a, b = self.reg_value(state, ops[0].reg), self.reg_value(state, ops[1].reg)
            if len(a) == len(b):
                self.write_reg(state, ops[0].reg, b)
                self.write_reg(state, ops[1].reg, a)
                return [following]

        # esp's own arithmetic.
        if len(ops) == 2 and ops[0].type == exe_index.REG and ops[0].reg == "esp":
            if m in ("add", "sub") and ops[1].type == exe_index.IMM and state.esp is not None:
                delta = ops[1].imm if ops[1].imm < 0x80000000 else ops[1].imm - (1 << 32)
                state.esp += delta if m == "add" else -delta
                if m == "add" and after_call_cleanup:
                    # What a caller drops right after a call was the callee's to read.
                    for at in range(state.esp - delta, state.esp, 4):
                        self.use(out, state.slots.get(at, FRESH3))
            else:
                if ops[1].type == exe_index.REG:
                    self.use_reg(out, state, ops[1].reg)
                state.esp = None
            return [following]
        if len(ops) == 2 and ops[0].type == exe_index.REG and ops[0].reg == "ebp" and state.ebp is not None:
            if m in ("add", "sub") and ops[1].type == exe_index.IMM:
                delta = ops[1].imm if ops[1].imm < 0x80000000 else ops[1].imm - (1 << 32)
                state.ebp += delta if m == "add" else -delta
                return [following]
            state.ebp = None

        if m in STRING:
            read, written = STRING[m]
            for r in read:
                self.use_reg(out, state, r)
            for r in written:
                self.write_reg(state, r)
            if prefixed:
                self.use(out, state.regs["ecx"])
                state.regs["ecx"] = FRESH3
            return [following]

        return self.generic(out, state, insn, m, ops, following)

    def lea(self, out, state, ops, following):
        to, source = ops
        at = self.stack_offset(state, source)
        if to.reg == "esp":
            state.esp = at
            return [following]
        if to.reg == "ebp":
            state.ebp = at
            if at is not None:
                state.regs["ebp"] = frame_label(at)
            else:
                self.address(out, state, source)
                self.write_reg(state, "ebp")
            return [following]
        if source.base == to.reg and not source.index and source.disp == 0:
            return [following]  # padding
        self.address(out, state, source)
        self.write_reg(state, to.reg)
        return [following]

    def mov(self, out, state, ops, following):
        to, source = ops
        if to.type == exe_index.REG:
            if to.reg == "esp":
                if source.type == exe_index.REG and source.reg == "ebp":
                    state.esp = state.ebp
                else:
                    state.esp = None
                return [following]
            if source.type == exe_index.REG and source.reg == "esp":
                if to.reg == "ebp" and state.esp is not None:
                    state.ebp = state.esp
                    state.regs["ebp"] = frame_label(state.esp)
                else:
                    self.write_reg(state, to.reg)
                return [following]
            if to.reg == "ebp":
                state.ebp = None
            if source.type == exe_index.REG:
                if source.reg in PARTS and to.reg in PARTS:
                    value = self.reg_value(state, source.reg)
                    if len(value) == len(PARTS[to.reg][1]):
                        self.write_reg(state, to.reg, value)
                    else:
                        self.use(out, value)
                        self.write_reg(state, to.reg)
                else:
                    self.write_reg(state, to.reg)
            elif source.type == exe_index.MEM:
                value = self.load(out, state, source)
                if len(PARTS[to.reg][1]) == 3 if to.reg in PARTS else False:
                    self.write_reg(state, to.reg, value)
                else:
                    # A part of a word off the stack: its use is as good as seen here.
                    self.use(out, value)
                    self.write_reg(state, to.reg)
            else:
                self.write_reg(state, to.reg)
            return [following]
        # To memory.
        if source.type == exe_index.REG:
            value = self.reg_value(state, source.reg) if source.reg in PARTS else [FRESH, FRESH, FRESH]
            self.store(out, state, to, value, whole=source.size == 4)
        else:
            self.store(out, state, to, FRESH3, whole=to.size == 4)
        return [following]

    def jump(self, out, state, function, insn, m, ops, following):
        if m in ("jecxz", "jcxz") or m.startswith("loop"):
            self.use(out, state.regs["ecx"])
            if m.startswith("loop"):
                state.regs["ecx"] = FRESH3
        op = ops[0]
        conditional = m != "jmp"
        if op.type == exe_index.IMM:
            target = op.imm
            if (target in self.entries and target != function.entry and target not in own_addresses(function)
                    and state.esp == 0):
                # A tail call: what it leaves is what this function leaves.  Only at the depth
                # the function came in at: elsewhere the generator's table has cut the
                # function's own code at a label, and the jump just goes on in it.
                leaving = state.copy()
                self.apply_callee(out, leaving, self.summaries[target], insn.address)
                self.leave(out, leaving, self.summaries[target].pops)
                return [following] if conditional else None
            return [target, following] if conditional else [target]
        if op.type == exe_index.REG:
            self.use_reg(out, state, op.reg)
        else:
            self.address(out, state, op)
        name = self.import_at(op)
        if name is not None:
            words = self.game.params.get(name)
            if words is None:
                words = 0
                out.lost = True
            out.stack = max(out.stack, words)
            leaving = state.copy()
            for r in IMPORT_CLOBBERS:
                leaving.regs[r] = FRESH3
            # A thunk: the import pops as it would have popped its caller's words.
            self.leave(out, leaving, 4 * words)
            return None
        if insn.destinations:
            return list(insn.destinations)
        # Somewhere the code does not say.
        out.indirect.add(insn.address)
        leaving = state.copy()
        for r in INDIRECT_CLOBBERS:
            leaving.regs[r] = ASSUMED3
        self.leave(out, leaving, 0)
        return None

    def generic(self, out, state, insn, m, ops, following):
        """All else by what capstone says it reads and writes, memory operands by their place."""
        for i, op in enumerate(ops):
            if op.type != exe_index.MEM:
                continue
            at = self.stack_offset(state, op)
            # The first operand of most is both; fst, fstp and the like only write; cmp, test, fld only read.
            writes = i == 0 and m not in ("cmp", "test", "push") and not m.startswith(("fld", "fild", "fcom", "fadd", "fsub", "fmul", "fdiv", "fldcw", "ficom", "fiadd", "fisub", "fimul", "fidiv", "movq", "bt"))
            only_writes = i == 0 and (m.startswith(("fst", "fist", "fnst", "set")) or (m == "movq"))
            if not only_writes:
                self.use(out, self.load(out, state, op))
            elif at is None:
                self.address(out, state, op)
            if writes and at is not None and at < 4:
                state.slots[at & ~3] = FRESH3
        for name in insn.reads:
            if name in PARTS:
                full, parts = PARTS[name]
                # A memory operand's base and index were dealt with above.
                if any(op.type == exe_index.MEM and name in (op.base, op.index) for op in ops):
                    continue
                self.use(out, [state.regs[full][p] for p in parts])
        for name in insn.writes:
            if name in PARTS:
                if name == "ebp":
                    state.ebp = None
                self.write_reg(state, name)
            elif name == "esp":
                state.esp = None
        return [following]


def dll_pointers(game):
    """The globals that hold what GetProcAddress gave: `call GetProcAddress` and, a few
    instructions on, its eax stored.  The DLLs' exports are stdcall (their names say @n)."""
    slots = {slot for slot, name in game.imports.items() if name == "GetProcAddress"}
    thunks = {f.entry for f in game.functions.values()
              if f.insns and f.insns[0].mnemonic == "jmp" and f.insns[0].operands[0].type == exe_index.MEM
              and (f.insns[0].operands[0].disp & 0xffffffff) in slots}
    pointers = set()
    for function in game.functions.values():
        insns = function.insns
        for i, insn in enumerate(insns):
            if insn.mnemonic != "call":
                continue
            op = insn.operands[0]
            if not ((op.type == exe_index.IMM and op.imm in thunks)
                    or (op.type == exe_index.MEM and (op.disp & 0xffffffff) in slots)):
                continue
            for later in insns[i + 1:i + 7]:
                if later.mnemonic == "mov" and later.operands[0].type == exe_index.MEM and later.operands[1].type == exe_index.REG \
                        and later.operands[1].reg == "eax" and not later.operands[0].base and not later.operands[0].index:
                    pointers.add(later.operands[0].disp & 0xffffffff)
                    break
                if later.mnemonic == "call":
                    break
    return pointers


def own_addresses(function):
    cached = _OWN.get(function.entry)
    if cached is None:
        cached = frozenset(i.address for i in function.insns)
        _OWN[function.entry] = cached
    return cached


_OWN = {}


def read_function(reader, function, trace=None):
    """The function's reading.  Where its rets do not come out level the unknown callees'
    pops were guessed wrong: the other answer tried, for them all and then for each alone."""
    reader.pop_choice, reader.pop_default = {}, True
    out = read_once(reader, function, trace)
    if not out.lost or not out.unknown or not out.pop_values:
        return out
    tries = [({}, False)] + [({site: False}, True) for site in sorted(out.unknown)[:16]]
    for choice, default in tries:
        reader.pop_choice, reader.pop_default = choice, default
        other = read_once(reader, function, trace if trace is not None else None)
        if not other.lost:
            reader.pop_choice, reader.pop_default = {}, True
            return other
    reader.pop_choice, reader.pop_default = {}, True
    return read_once(reader, function, trace)


def read_once(reader, function, trace=None):
    """One reading, as a new Summary, by a worklist over its code (and any it jumps or
    falls into)."""
    out = Summary()
    # A function no ret was found in keeps what scan_pops said, if anything.
    out.pops = reader.summaries[function.entry].pops
    states = {function.entry: entry_state()}
    work = [function.entry]
    at = reader.game.at
    seen = 0
    while work and seen < 400000:
        seen += 1
        address = work.pop()
        insn = at.get(address)
        if insn is None:
            out.lost = True
            continue
        state = states[address].copy()
        next_insn = at.get(address + insn.size)
        reader.at = address
        cleanup = address in reader.after_calls
        following = reader.step(out, state, function, insn, next_insn, cleanup)
        if trace is not None:
            trace[address] = (states[address], state)
        if following is None:
            continue
        for target in following:
            if target not in at:
                continue
            if target not in states:
                states[target] = state.copy()
                work.append(target)
            elif merge(states[target], state):
                work.append(target)
    out.assumed -= out.real
    if out.pop_values:
        out.pops = max(out.pop_values)
    if len(out.pop_values) > 1:
        out.lost = True
    if out.pops:
        out.stack = max(out.stack, out.pops // 4)
    return out


def scan_pops(game, entries):
    """What each function's ret pops, ahead of the reading: esp's tracking needs it of callees."""
    pops = {}
    tails = {}
    for entry, function in game.functions.items():
        seen, work, found, tail = set(), [entry], None, None
        while work and found is None:
            address = work.pop()
            if address in seen or address not in game.at:
                continue
            seen.add(address)
            insn = game.at[address]
            m = insn.mnemonic
            if m in ("ret", "retn"):
                found = insn.operands[0].imm if insn.operands else 0
                break
            if m == "jmp" or m.startswith("j"):
                op = insn.operands[0]
                if op.type == exe_index.IMM:
                    if op.imm in entries and op.imm not in own_addresses(function):
                        tail = tail or op.imm
                    else:
                        work.append(op.imm)
                else:
                    work.extend(insn.destinations)
                    if op.type == exe_index.MEM and not op.base and not op.index:
                        name = game.imports.get(op.disp & 0xffffffff)
                        if name is not None and name in game.params:
                            found = 4 * game.params[name]
                            break
                if m == "jmp":
                    continue
            work.append(address + insn.size)
        if found is not None:
            pops[entry] = found
        elif tail is not None:
            tails[entry] = tail
    for _ in range(8):
        for entry, tail in list(tails.items()):
            if tail in pops:
                pops[entry] = pops[tail]
                del tails[entry]
    return pops


def derive(game):
    """Every function's Summary, to a fixed point."""
    entries = set(game.functions)
    summaries = {entry: Summary() for entry in entries}
    for entry, value in scan_pops(game, entries).items():
        summaries[entry].pops = value
        summaries[entry].stack = value // 4
    reader = Reader(game, summaries, entries)
    callers = {}
    for entry, function in game.functions.items():
        for insn in function.insns:
            if insn.mnemonic == "call" or insn.mnemonic.startswith("j"):
                for op in insn.operands[:1]:
                    if op.type == exe_index.IMM and op.imm in entries:
                        callers.setdefault(op.imm, set()).add(entry)
                for d in insn.destinations:
                    if d in entries:
                        callers.setdefault(d, set()).add(entry)
    # Each function's callers read again when its answer grew.  Reads and clobbers only
    # ever grow, so this ends, and recursion comes out the same from any start.
    work = set(entries)
    rounds = 0
    while work and rounds < 2000000:
        entry = work.pop()
        rounds += 1
        new = read_function(reader, game.functions[entry])
        old = summaries[entry]
        new.reads |= old.reads
        new.clobbers |= old.clobbers
        new.indirect |= old.indirect
        new.passes |= old.passes
        new.upper |= old.upper
        new.flows |= old.flows
        new.real |= old.real
        new.assumed = (new.assumed | old.assumed) - new.real
        new.stack = max(new.stack, old.stack)
        if (new.reads, new.clobbers, new.assumed, new.passes, new.upper, new.flows, new.pops, new.stack, new.indirect) != \
                (old.reads, old.clobbers, old.assumed, old.passes, old.upper, old.flows, old.pops, old.stack, old.indirect):
            summaries[entry] = new
            work.add(entry)
            work.update(callers.get(entry, ()))
        else:
            old.ends = old.ends or new.ends
            old.lost = new.lost
            old.pop_values = new.pop_values
    return summaries


def table():
    """{address: row} of the whole game, kept in the build directory."""
    key = hashlib.sha1(Path(__file__).read_bytes() + (HERE / "exe_index.py").read_bytes()
                       + (ROOT / "nfs3hp/nfs3.exe").read_bytes()).hexdigest()
    if CACHE.exists():
        stored = json.loads(CACHE.read_text())
        if stored.get("key") == key:
            return {int(k): v for k, v in stored["rows"].items()}
    game = exe_index.load()
    summaries = derive(game)
    rows = {}
    for entry, s in summaries.items():
        x87 = game.x87.get(entry)
        rows[entry] = {"reads": sorted(s.reads), "clobbers": sorted(s.clobbers), "assumed": sorted(s.assumed),
                       "passes": sorted(s.passes - s.reads), "upper": sorted(s.upper - s.reads),
                       "flows": sorted(s.flows - s.reads),
                       "pops": s.pops, "stack": s.stack,
                       "indirect": sorted(s.indirect), "lost": s.lost, "ends": s.ends,
                       "x87": list(x87) if x87 else [0, 0]}
    CACHE.parent.mkdir(parents=True, exist_ok=True)
    CACHE.write_text(json.dumps({"key": key, "rows": rows}))
    return rows


def mask(names):
    ordered = [r for r in ("eax", "ebx", "ecx", "edx", "esi", "edi") if r in names]
    return " | ".join(NAMES[r] for r in ordered) or "0"


def row_text(address, row):
    """A line for kSpecs, what the reading could not settle in its comment."""
    clobbers = [r for r in row["clobbers"] if r != "eax"]
    kind = "Plain" if "eax" in row["clobbers"] else "NoEax"
    notes = []
    assumed = [r for r in row["assumed"] if r != "eax"]
    if assumed or row["passes"]:
        where = " ".join("%x" % a for a in row["indirect"][:3]) + (" ..." if len(row["indirect"]) > 3 else "")
        if assumed:
            notes.append("%s changed only if the indirect calls at %s change them" % (mask(assumed), where))
        if row["passes"]:
            notes.append("those calls may read %s" % mask(row["passes"]))
    if row["upper"]:
        notes.append("the upper bits of %s go through as they came" % mask(row["upper"]))
    if row["flows"]:
        notes.append("%s left in another register as it came" % mask(row["flows"]))
    if row["lost"]:
        notes.append("stack lost: what its pops restore not trusted")
    if not row["ends"]:
        notes.append("no ret reached")
    if row["x87"][1]:
        notes.append("leaves %+d on the x87: no Kind::Plain does that" % row["x87"][1])
    if row["x87"][0]:
        notes.append("takes %d from the x87" % row["x87"][0])
    if row["pops"] == 0 and row["stack"]:
        notes.append("reads %d words above its return, the caller cleans" % row["stack"])
    return "{ 0x%x, { %s, %d, %d, %s, Kind::%s } },%s" % (
        address, mask(row["reads"]), row["stack"], row["pops"], mask(clobbers), kind,
        "  // " + "; ".join(notes) if notes else "")


# ---------------------------------------------------------------------------
# The tables by hand
# ---------------------------------------------------------------------------

def hand_rows():
    """{address: [(reads, stack, pops, clobbers, kind, where), ...]} of kSpecs and every group's
    world: `{ 0x..., { in, stack, pops, clobbers, Kind::X[, string] } }`, `specs[0x...] = { ... }`,
    `add(0x..., { ... })` and the `plain(0x..., in, stack, pops, clobbers)` of a world.  An
    address can have several: a world's own row goes over kSpecs' in that group's process."""
    rows = {}
    regs = {"EAX": "eax", "EBX": "ebx", "ECX": "ecx", "EDX": "edx", "ESI": "esi", "EDI": "edi"}

    def registers(text):
        return {regs[t] for t in re.findall(r"\b[A-Z]{3}\b", text) if t in regs}

    source = (HERE / "frontend_checks.cpp").read_text()
    begin = source.index("const std::map<x86::reg32, Spec> kSpecs")
    end = source.index("x86::reg32* reg(x86::CPU& cpu", begin)
    row = re.compile(r"\{ *0x([0-9a-f]+), *\{ *([^,]*), *(\d+), *(\d+), *([^,]*), *Kind::(\w+)")
    for m in row.finditer(source[begin:end]):
        rows.setdefault(int(m.group(1), 16), []).append((registers(m.group(2)), int(m.group(3)), int(m.group(4)),
                                                         registers(m.group(5)), m.group(6), "kSpecs"))
    number = r"(0x[0-9a-fA-F]+|\d+)"
    set_row = re.compile(r"(?:specs\[0x([0-9a-f]+)\] *= *|\b\w+\(0x([0-9a-f]+), *)\{ *([^,]*), *" + number + ", *" + number
                         + r", *([^,]*), *Kind::(\w+)")
    plain = re.compile(r"\bplain\(0x([0-9a-f]+), *([^,)]*)(?:, *(\d+))?(?:, *(\d+))?(?:, *([^,)]*))?\)")
    # A world's names for a set of registers: constexpr ... kArgs3 = EAX | EDX | EBX;
    named = re.compile(r"\b(k\w+) *= *((?:[A-Z]{3}|k\w+)(?: *\| *(?:[A-Z]{3}|k\w+))*) *;")
    for world in sorted(GROUPS.glob("*_world.inc")):
        group = world.stem[:-len("_world")]
        text = world.read_text(errors="replace")
        names = {}

        def registers_of(field):
            return registers(re.sub(r"\bk\w+", lambda n: " ".join(sorted(names.get(n.group(0), ()))).upper(), field))

        for m in named.finditer(text):
            names[m.group(1)] = registers_of(m.group(2))
        for m in set_row.finditer(text):
            rows.setdefault(int(m.group(1) or m.group(2), 16), []).append(
                (registers_of(m.group(3)), int(m.group(4), 0), int(m.group(5), 0), registers_of(m.group(6)), m.group(7), group))
        for m in plain.finditer(text):
            rows.setdefault(int(m.group(1), 16), []).append(
                (registers_of(m.group(2)), int(m.group(3) or 0), int(m.group(4) or 0), registers_of(m.group(5) or ""), "Plain", group))
    return rows


def validate(detail):
    derived = table()
    hand = hand_rows()
    tally = {"same": 0, "absent": 0}
    classes = {}
    lines = []
    every = [(address, entry) for address, entries in sorted(hand.items()) for entry in entries]
    for address, (reads, stack, pops, clobbers, kind, where) in every:
        row = derived.get(address)
        if row is None:
            tally["absent"] += 1
            lines.append("%x (%s): no function at this address" % (address, where))
            continue
        got_reads = set(row["reads"])
        got_clobbers = set(row["clobbers"]) - {"eax"}
        differences = []
        if got_reads - reads:
            differences.append(("reads more", got_reads - reads))
        if reads - got_reads:
            differences.append(("reads less", reads - got_reads))
        if got_clobbers - clobbers:
            differences.append(("changes more", got_clobbers - clobbers))
        if clobbers - got_clobbers:
            differences.append(("changes less", clobbers - got_clobbers))
        if row["pops"] != pops:
            differences.append(("pops", "%d not %d" % (row["pops"], pops)))
        if row["stack"] != stack:
            differences.append(("stack", "%d not %d" % (row["stack"], stack)))
        if ("eax" in row["clobbers"]) == (kind == "NoEax") and kind in ("Plain", "NoEax"):
            differences.append(("eax", "kept" if kind == "Plain" else "changed"))
        touched = (set(row["assumed"]) & (clobbers ^ got_clobbers)) | (set(row["passes"]) & (reads - got_reads))
        unsure = ("indirect " if touched else "") + ("lost " if row["lost"] else "") + ("noret " if not row["ends"] else "")
        if not differences:
            tally["same"] += 1
            continue
        for name, what in differences:
            key = name + (" (unsure)" if unsure else "")
            classes[key] = classes.get(key, 0) + 1
        text = "; ".join("%s %s" % (name, " ".join(sorted(what)) if isinstance(what, set) else what)
                         for name, what in differences)
        lines.append("%x (%s, %s): %s%s" % (address, where, kind, text, "  [" + unsure.strip() + "]" if unsure else ""))
    print("%d rows by hand, of %d functions: %d agree, %d disagree on something"
          % (len(every), len(hand), tally["same"], len(lines)))
    for name in sorted(classes):
        print("  %-28s %d" % (name, classes[name]))
    if detail:
        print("\n".join(lines))
    total = len(derived)
    print("the whole game: %d functions; %d with an indirect call somewhere under them, of which %d show it (a register "
          "changed only by assumption, or one that call may read); %d with the stack lost; %d with no ret"
          % (total, sum(1 for r in derived.values() if r["indirect"]),
             sum(1 for r in derived.values() if r["assumed"] or r["passes"]),
             sum(1 for r in derived.values() if r["lost"]), sum(1 for r in derived.values() if not r["ends"])))


def show(address):
    game = exe_index.load()
    derived = table()
    summaries = {int(k): summary_of(v) for k, v in derived.items()}
    reader = Reader(game, summaries, set(game.functions))
    trace = {}
    function = game.functions[address]
    reader.why = {}
    read_function(reader, function, trace)
    print(row_text(address, derived[address]))
    print("read first at: " + ", ".join("%s %x" % (r, a) for r, a in sorted(reader.why.items())))
    for at in sorted(trace):
        before, after = trace[at]
        changed = [r[1] + ("=" + ",".join(sorted("".join(sorted(p)) for p in set(after.regs[r])))) for r in TRACKED
                   if after.regs[r] != before.regs[r]]
        insn = game.at[at]
        extra = ""
        if insn.mnemonic == "call" and insn.operands[0].type == exe_index.IMM and insn.operands[0].imm in derived:
            extra = "  ; " + row_text(insn.operands[0].imm, derived[insn.operands[0].imm])[:70]
        print("%08x esp%+-5s %-34s %s%s" % (at, before.esp if before.esp is not None else "?",
                                            insn.mnemonic + " " + insn.text, " ".join(changed), extra))


def summary_of(row):
    s = Summary()
    s.reads, s.clobbers = set(row["reads"]), set(row["clobbers"])
    s.assumed, s.passes, s.upper, s.flows = set(row["assumed"]), set(row["passes"]), set(row["upper"]), set(row["flows"])
    s.pops, s.stack, s.indirect, s.lost, s.ends = row["pops"], row["stack"], set(row["indirect"]), row["lost"], row["ends"]
    return s


# ---------------------------------------------------------------------------
# A group's rows
# ---------------------------------------------------------------------------

def group_functions(group):
    path = GROUPS / (group + ".txt")
    return [int(line.split()[0], 16) for line in path.read_text().splitlines() if line and not line.startswith("#")]


def callees(game, address):
    """What the function calls by address, its tail jumps too."""
    function = game.functions[address]
    out = set()
    own = own_addresses(function)
    for insn in function.insns:
        if insn.mnemonic == "call" or insn.mnemonic.startswith("j"):
            op = insn.operands[0]
            if op.type == exe_index.IMM and op.imm in game.functions and op.imm not in own:
                out.add(op.imm)
            for d in insn.destinations:
                if d in game.functions and d not in own:
                    out.add(d)
    return out


def group_rows(group):
    """The rows the group's functions need and no table has: their callees' that are not
    in the group, kSpecs or this group's own world (another group's world is not in its process)."""
    game = exe_index.load()
    derived = table()
    own = set(group_functions(group))
    hand = hand_rows()
    # The runtime's __CHP runs as it is (extract.py's REAL).
    wanted = sorted(set().union(*(callees(game, a) for a in own)) - own - {0x4dfd56}) if own else []
    missing = [a for a in wanted if not any(entry[5] in ("kSpecs", group) for entry in hand.get(a, ()))]
    return [row_text(a, derived[a]) for a in missing], len(wanted), len(wanted) - len(missing)


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("what", nargs="*", help="a group, or addresses")
    parser.add_argument("--write", action="store_true", help="into groups/<group>_specs.inc")
    parser.add_argument("--validate", action="store_true")
    parser.add_argument("--detail", action="store_true")
    parser.add_argument("--show", type=lambda s: int(s, 16))
    options = parser.parse_args()
    if options.validate:
        validate(options.detail)
        return
    if options.show:
        show(options.show)
        return
    if len(options.what) == 1 and (GROUPS / (options.what[0] + ".txt")).exists():
        group = options.what[0]
        rows, wanted, had = group_rows(group)
        header = "// Read off nfs3.exe by specs_from_exe.py: what %s calls and no table had.\n" % group
        if options.write:
            (GROUPS / (group + "_specs.inc")).write_text(header + "".join(row + "\n" for row in rows))
        print("%s calls %d functions outside itself, %d have a row by hand, %d derived:" % (group, wanted, had, len(rows)))
        print("\n".join(rows))
        return
    derived = table()
    for word in options.what:
        address = int(word, 16)
        if address not in derived:
            sys.exit("no function at %x" % address)
        print(row_text(address, derived[address]))


if __name__ == "__main__":
    main()
