"""The x87 stack in locals, through a whole function.

The depth of the x87 stack at every instruction of a function, relative to where
the function began, follows from the code: a compiler leaves the same depth on
every path into a label.  So a function can keep its stack in C++ locals, one for
each depth, with no count and no array behind them: st(N) at depth d is simply
the local for depth d-1-N, and a push or a pop only moves the generator's idea of
d.  A jump needs nothing, the depth being the same at both ends.

What is left of the x87 stack is what functions hand each other on it.  What each
function reads from below its entry depth and how far it moves the stack is worked
out here from the code too (the natives by the code they replace), to a fixed
point, since it depends on what the function calls.  The emulated FPU then carries
only those values: pushed before a call that takes them, popped after one that
leaves them, read at the entry of a function that takes them and written at its
exit.  A call that neither takes nor leaves any costs nothing, whatever is on the
caller's stack at the time.

The status word goes the same way.  A compare and the fnstsw that reads it are in
one function, with no call between them, in all of the game's code but a few
routines of the runtime; there the word is a local too.

The statements are those of codegen/fpu.py, so their meaning is inherited; only
where the values wait changes.
"""
import re

from capstone import x86

_ST = re.compile(r'cpu\.fpu\.st\((\d+)\)')
_PUSH = re.compile(r'^cpu\.fpu\.push\((.*)\);$')
_POP = 'cpu.fpu.pop();'
_TAIL = re.compile(r'^return .*handOver\(\)\);$')
_STOP = ('int', 'int3', 'into', 'iret', 'iretd', 'hlt', 'ud2')
_SETS_STATUS = ('fcom', 'fcomp', 'fcompp', 'ftst', 'fprem')
_READS_STATUS = ('fnstsw', 'fstsw')

# Functions a control word is loaded for without their loading it: the powers of
# ten of __LDScale10x, under the word of those that call them.
UNDER_CONTROL_WORD = {0x51667b}


def slot(depth):
    """The local for the stack entry at `depth`; below the function's own entry
    depth, one of the values it was handed."""
    return 'fp%d' % depth if depth >= 0 else 'fpa%d' % -depth


def statements_effect(lines):
    """(entries the statements need on the stack, how far they move it)."""
    needs = moved = 0
    for raw in lines:
        line = raw.strip()
        if line == _POP:
            needs = max(needs, 1 - moved)
            moved -= 1
            continue
        for match in _ST.finditer(line):
            needs = max(needs, int(match.group(1)) + 1 - moved)
        if _PUSH.match(line):
            moved += 1
    return needs, moved


class Node:
    __slots__ = ('needs', 'moved', 'next', 'targets', 'call', 'tail', 'exit', 'sets_status', 'reads_status')

    def __init__(self):
        self.needs = self.moved = 0
        self.next = True       # goes on to the following instruction
        self.targets = ()      # instruction indices it may jump to
        self.call = False      # calls and comes back
        self.tail = False      # leaves by a jump to another function
        self.exit = False      # ret
        self.sets_status = self.reads_status = False


class Summary:
    """One subroutine's instructions, as the walk sees them."""

    def __init__(self, subroutine, module, skip):
        self.subroutine = subroutine
        self.entry = subroutine.get_entry_point()
        self.start = subroutine.get_start_address()
        self.end = subroutine.get_end_address()
        self.index = {i.address: k for k, i in enumerate(subroutine.instructions)}
        self.nodes = [self._node(i, module, skip) for i in subroutine.instructions]
        self.callees = set()
        for node in self.nodes:
            for callee in (node.call, node.tail):
                if callee is not False and callee is not None:
                    self.callees.add(callee)
        self.has_x87 = any(i.mnemonic.startswith('f') for i in subroutine.instructions)

    def _inside(self, address):
        return self.start <= address < self.end

    def _node(self, instruction, module, skip):
        node = Node()
        mnemonic = instruction.mnemonic
        if instruction.address in skip:
            return node
        if mnemonic.startswith('f'):
            lines = module.generate(instruction, (self.start, self.end), {})
            node.needs, node.moved = statements_effect(lines)
            node.sets_status = mnemonic in _SETS_STATUS
            node.reads_status = mnemonic in _READS_STATUS
        elif mnemonic.startswith('ret'):
            node.next = False
            node.exit = True
        elif mnemonic in _STOP:
            node.next = False
        elif mnemonic == 'call':
            operand = instruction.operands[0]
            node.call = operand.imm if operand.type == x86.X86_OP_IMM else None
        elif mnemonic[0] == 'j' or mnemonic.startswith('loop'):
            self._jump(node, instruction)
        return node

    def _jump(self, node, instruction):
        operand = instruction.operands[0]
        always = instruction.mnemonic == 'jmp'
        node.next = not always
        if operand.type == x86.X86_OP_IMM:
            if self._inside(operand.imm):
                node.targets = (self.index[operand.imm],)
            else:
                node.tail = operand.imm
        else:
            destinations = getattr(instruction, 'potential_destinations', [])
            if destinations and all(d in self.index for d in destinations):
                node.targets = tuple(self.index[d] for d in destinations)
            else:
                node.tail = None  # a call through a pointer, for good

    def walk(self, effects):
        """The depths this function's code reaches, given what is known of its
        callees (unknown ones stop a path for now).  Returns (depths by index,
        what it needs from below, the depths it exits at, the indices where two
        paths disagree, whether a path stopped at an unknown callee)."""
        depths = {}
        needs = 0
        nets = set()
        conflicts = []
        stopped = False
        work = [(self.index[self.entry], 0)]
        count = len(self.nodes)
        while work:
            k, depth = work.pop()
            if k >= count:
                continue
            if k in depths:
                if depths[k] != depth:
                    conflicts.append(k)
                continue
            depths[k] = depth
            node = self.nodes[k]
            needs = max(needs, node.needs - depth)
            after = depth + node.moved
            if node.call is not False:
                effect = effects.get(node.call, (0, 0)) if node.call is not None else (0, 0)
                if effect is UNKNOWN:
                    stopped = True
                    continue
                needs = max(needs, effect[0] - after)
                after += effect[1]
            if node.exit:
                nets.add(after)
                continue
            if node.tail is not False:
                effect = effects.get(node.tail, (0, 0)) if node.tail is not None else (0, 0)
                if effect is UNKNOWN:
                    stopped = True
                else:
                    needs = max(needs, effect[0] - after)
                    nets.add(after + effect[1])
            for target in node.targets:
                work.append((target, after))
            if node.next:
                work.append((k + 1, after))
        return depths, needs, nets, conflicts, stopped

    def status_is_local(self, depths):
        """Whether every status word this function reads comes from a compare
        of its own, with no call since on any path there."""
        known = {}
        work = [(self.index[self.entry], False)]
        while work:
            k, state = work.pop()
            if k not in depths:
                continue
            if k in known and (not known[k] or state):
                continue
            known[k] = state
            node = self.nodes[k]
            if node.reads_status and not state:
                return False
            after = state or node.sets_status
            if node.call is not False:
                after = False
            for target in node.targets:
                work.append((target, after))
            if node.next:
                work.append((k + 1, after))
        return True


UNKNOWN = object()


class Frame:
    """What write_function needs of one function."""

    def __init__(self, summary, effects):
        self.summary = summary
        self.effects = effects
        self.depths, self.needs, nets, self.conflicts, _ = summary.walk(effects)
        self.net = next(iter(nets)) if nets else 0
        self.nets = nets
        self.valid = not self.conflicts and len(nets) <= 1 and self.net + self.needs >= 0
        self.used = set()
        self.local_status = self.valid and summary.status_is_local(self.depths)
        self.uses_status = False

    def depth_at(self, instruction):
        return self.depths.get(self.summary.index[instruction.address])

    # -- writing ----------------------------------------------------------
    def _name(self, depth):
        self.used.add(depth)
        return slot(depth)

    def _at(self, depth, n):
        return self._name(depth - 1 - n)

    def _hand_over(self, depth):
        """The emulated FPU as a caller or the next function must find it: moved
        by `depth` from this function's entry, with everything this function
        holds of what is left on it."""
        out = []
        if depth > 0:
            out.append('cpu.fpu.count += %d;' % depth)
        elif depth < 0:
            out.append('cpu.fpu.count -= %d;' % -depth)
        for d in range(depth - 1, -self.needs - 1, -1):
            out.append('cpu.fpu.st(%d) = %s;' % (depth - 1 - d, self._name(d)))
        return out

    def _effect(self, callee):
        effect = self.effects.get(callee, (0, 0)) if callee is not None else (0, 0)
        return (0, 0) if effect is UNKNOWN else effect

    def transform(self, instruction, lines):
        """`lines` of one instruction with the stack in locals."""
        k = self.summary.index[instruction.address]
        depth = self.depths[k]
        node = self.summary.nodes[k]
        out = []
        if node.call is not False:
            takes, moves = self._effect(node.call)
            for d in range(depth - takes, depth):
                out.append('cpu.fpu.push(%s);' % self._name(d))
            out.extend(lines)
            for d in range(depth + moves - 1, depth - takes - 1, -1):
                out.append('%s = cpu.fpu.take();' % self._name(d))
            return out
        if node.exit:
            return self._hand_over(depth) + list(lines)
        for raw in lines:
            line = raw.strip()
            if line == _POP:
                depth -= 1
                continue
            if _TAIL.match(line):
                out.extend(self._hand_over(depth))
                out.append(line)
                continue
            match = _PUSH.match(line)
            if match:
                value = _ST.sub(lambda m, d=depth: self._at(d, int(m.group(1))), match.group(1))
                out.append('%s = %s;' % (self._name(depth), value))
                depth += 1
                continue
            out.append(self._status(_ST.sub(lambda m, d=depth: self._at(d, int(m.group(1))), raw)))
        return out

    def _status(self, line):
        if not self.local_status or 'cpu.fpu.' not in line:
            return line
        changed = (line.replace('cpu.fpu.compare(', 'fsw = fp::compare(')
                   .replace('cpu.fpu.status.c2 = 0;', 'fsw &= ~0x400;')
                   .replace('cpu.fpu.status.word', 'fsw'))
        self.uses_status = self.uses_status or changed != line
        return changed

    def opening(self):
        """The locals, at the very top: every jump in the function lands after
        them.  Call when the function's code has been written."""
        out = []
        for k in range(1, self.needs + 1):
            out.append('  x86::Float %s = cpu.fpu.st(%d);\n' % (slot(-k), k - 1))
        own = sorted(d for d in self.used if d >= 0)
        if own:
            out.append('  x86::Float %s;\n' % ', '.join(slot(d) for d in own))
        if self.uses_status:
            out.append('  x86::reg16 fsw = 0;\n')
        return ''.join(out)


# What the code alone does not show: (entries taken, how far the stack moves).
# The FDIV bug's atan2 saves the whole FPU around its work (fnsave, frstor) and
# divides by a table that picks a register at run time, so to the walk it seems
# to read all eight.  A square root's error path pops its caller's value too, and
# says so in al, which acos checks; the error never comes up for 1-x*x.
FORCED = {
    0x4ff624: (2, -1),  # __fpatan_wrap
    0x51f320: (2, -1),  # its atan2
    0x526aeb: (2, -1),  # __fdiv_fpr, as that atan2 calls it
    0x4f9482: (1, 0),   # IF@DACOS
    0x51a1ba: (1, 0),   # __DSQRT
}


def analyse(module, skip_instructions):
    """Every subroutine's Frame by its entry point."""
    skip = set(skip_instructions)
    summaries = {s.get_entry_point(): Summary(s, module, skip) for s in module.subroutines}
    effects = {entry: UNKNOWN for entry in summaries}
    effects.update((entry, effect) for entry, effect in FORCED.items() if entry in summaries)
    callers = {}
    for entry, summary in summaries.items():
        for callee in summary.callees:
            callers.setdefault(callee, set()).add(entry)

    def settle(work):
        work -= set(FORCED)
        while work:
            entry = work.pop()
            if entry in FORCED:
                continue
            _, needs, nets, _, _ = summaries[entry].walk(effects)
            if not nets:
                continue
            # A function whose paths disagree is reported by Frame; any answer will do here.
            effect = (needs, max(nets, key=abs))
            if effects[entry] is UNKNOWN or effects[entry] != effect:
                effects[entry] = effect
                work.update(callers.get(entry, ()))
                work.add(entry)

    settle(set(summaries))
    # What is still unknown never comes back on any known path: it moves nothing.
    while True:
        unknown = [entry for entry, effect in effects.items() if effect is UNKNOWN]
        if not unknown:
            break
        for entry in unknown:
            _, needs, _, _, _ = summaries[entry].walk(effects)
            effects[entry] = (needs, 0)
        work = set(unknown)
        for entry in unknown:
            work.update(callers.get(entry, ()))
        settle(work)
    return {entry: Frame(summary, effects) for entry, summary in summaries.items()}, effects
