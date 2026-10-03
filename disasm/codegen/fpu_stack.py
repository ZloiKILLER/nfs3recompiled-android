"""The x87 stack in locals, through each straight run of code.

The generated code keeps the x87 stack where the emulated FPU does: eight
doubles and a top-of-stack count in memory.  Every fld, fstp or fadd reads the
count, works out an index, loads, computes and stores back, and the next
instruction reloads what the last one stored; a profile of a split-screen race
(2026-09-25) spent half of the game thread in that traffic and the rounding
around it.

Between two places where control can arrive or leave -- a label, a jump, a
call, a return -- the depth of the stack relative to where the run began is
known while generating.  So through such a run the stack lives in the
generator's head instead: every st(N) the code names is an x86::Float local,
push and pop only move the model, and the stack is written back to the
emulated FPU once, where the run ends.  The arithmetic is untouched -- the same
FPU::add, mul, ... calls in the same order on the same doubles -- so the results
are the same to the bit; only where the values wait between instructions
changes.

The transform works on the statements the instruction generators already
produce (codegen/fpu.py), not on the instructions, so it inherits their exact
meaning: cpu.fpu.st(N) reads and writes, cpu.fpu.push(...), cpu.fpu.pop(), and
fxch's temporary.  A statement it does not understand ends the run first and is
written as it was.
"""
import os
import re

# NFS_FPU_LOCALS=0 in the generator's environment writes every instruction
# against the emulated FPU as before, to compare.
ENABLED = os.environ.get('NFS_FPU_LOCALS', '1') != '0'
# NFS_FPU_LOCALS_CHECK=1: every run is played through twice, symbolically --
# the statements as generated against a stack in memory, and what the run turned
# them into -- and the generator stops on the first run whose stack, count or
# side effects come out different (see _check below).
CHECK = os.environ.get('NFS_FPU_LOCALS_CHECK', '0') == '1'
checked_runs = 0


def text(lines):
    """Statements as the writers indent them."""
    return ''.join('    %s\n' % line for line in lines)


_ST = re.compile(r'cpu\.fpu\.st\((\d+)\)')
_ASSIGN_ST = re.compile(r'^cpu\.fpu\.st\((\d+)\) = (.*);$')
_PUSH = re.compile(r'^cpu\.fpu\.push\((.*)\);$')
_POP = 'cpu.fpu.pop();'
_TEMP = re.compile(r'^x86::Float (\w+) = (.*);$')
_ANY_FPU = re.compile(r'cpu\.fpu\.')
# Uses of the emulated FPU a run can carry: the arithmetic on values, the
# status and control words.  Anything else -- init, the count, the registers,
# push or pop inside an expression -- is not modelled.
_ALLOWED_CALL = re.compile(r'cpu\.fpu\.(st\(\d+\)|add|sub|mul|div|compare|sqrt|tan|sin|cos|abs|log2|atan|rem|scale'
                           r'|f2xm1|rndint\(|toInteger<|status\.word|control\.word|setControl)')
# Instructions that save, restore or reset the whole FPU: always a boundary.
_WHOLE_FPU = {'fninit', 'finit', 'fnsave', 'fsave', 'frstor', 'fldenv', 'fnstenv', 'fstenv'}


class Run:
    def __init__(self):
        self.reset()

    def reset(self):
        """A new function: no run, and local names start again."""
        self.open = False
        self.counter = 0
        self._clear()

    def _clear(self):
        self.explicit = []   # st(0), st(1), ...: ('orig', k) or ('local', name)
        self.shift = 0       # originals popped below the ones in `explicit`
        self.aliases = {}
        self.recorded_in = []    # for CHECK: each instruction's statements as generated
        self.recorded_out = []   # and what the run wrote instead

    # -- the model --------------------------------------------------------
    def _get(self, n):
        if n < len(self.explicit):
            return self.explicit[n]
        return ('orig', n - len(self.explicit) + self.shift)

    def _set(self, n, entry):
        while len(self.explicit) <= n:
            self.explicit.append(('orig', self.shift))
            self.shift += 1
        self.explicit[n] = entry

    def _push(self, entry):
        self.explicit.insert(0, entry)

    def _pop(self):
        if self.explicit:
            self.explicit.pop(0)
        else:
            self.shift += 1

    def depth(self):
        return len(self.explicit) - self.shift

    def _fresh(self):
        self.counter += 1
        return 'fpu%d' % self.counter

    def _value(self, entry, out):
        """The C++ name of `entry`'s value, loading an original from the
        emulated FPU the first time it is needed.  The count in memory does not
        move during a run, so original k is still cpu.fpu.st(k).  A loaded
        original remembers where it came from: left in that place, it need not
        be stored back."""
        if entry[0] != 'orig':
            return entry[1]
        k = entry[1]
        name = self._fresh()
        out.append('x86::Float %s = cpu.fpu.st(%d);' % (name, k))
        loaded = ('loaded', name, k)
        self.explicit = [loaded if e == entry else e for e in self.explicit]
        for alias, e in list(self.aliases.items()):
            if e == entry:
                self.aliases[alias] = loaded
        return name

    def _substitute(self, text, out):
        def one(match):
            n = int(match.group(1))
            entry = self._get(n)
            name = self._value(entry, out)
            if entry[0] == 'orig' and n >= len(self.explicit):
                self._set(n, ('loaded', name, entry[1]))
            return name
        text = _ST.sub(one, text)
        for alias, entry in self.aliases.items():
            text = re.sub(r'\b%s\b' % alias, lambda m, e=entry: self._value(e, out), text)
        return text

    def _entry_of(self, expression):
        """A bare register or temporary names an entry; anything else does not."""
        m = re.match(r'^cpu\.fpu\.st\((\d+)\)$', expression)
        if m:
            return self._get(int(m.group(1)))
        if expression in self.aliases:
            return self.aliases[expression]
        return None

    # -- the writer's side ------------------------------------------------
    @staticmethod
    def boundary(instruction):
        """Control arrives or leaves here, or the whole FPU is touched."""
        mnemonic = instruction.mnemonic
        return (mnemonic.startswith('j') or mnemonic.startswith('loop') or mnemonic.startswith('ret')
                or mnemonic in ('call', 'int', 'int3', 'into', 'iret', 'iretd', 'hlt', 'ud2')
                or mnemonic in _WHOLE_FPU)

    @staticmethod
    def understood(lines):
        """Whether every use of the emulated FPU in `lines` is one a run can
        carry.  Lines that do not touch it at all always are."""
        for raw in lines:
            line = raw.strip()
            if not _ANY_FPU.search(line):
                continue
            if line == _POP or _PUSH.match(line):
                if _PUSH.match(line) and ('cpu.fpu.push' in _PUSH.match(line).group(1)
                                          or 'cpu.fpu.pop' in _PUSH.match(line).group(1)):
                    return False
                continue
            rest = line
            for m in _ALLOWED_CALL.finditer(line):
                rest = rest.replace(m.group(0), '', 1)
            if _ANY_FPU.search(rest):
                return False
            if 'cpu.fpu.pop' in line or 'cpu.fpu.push' in line:
                return False
            if re.search(r'cpu\.fpu\.rndint\(\)', line):
                return False
        return True

    def close(self):
        """The model written back to the emulated FPU, and the run's block
        closed: the originals a move needs are read first, then the count is
        moved, then every entry that is not already where it belongs is
        stored."""
        if not self.open:
            return []
        out = []
        d = self.depth()
        for j, entry in enumerate(list(self.explicit)):
            if entry[0] == 'orig' and entry[1] != j - d:
                self._value(entry, out)
        if d > 0:
            out.append('cpu.fpu.count += %d;' % d)
        elif d < 0:
            out.append('cpu.fpu.count -= %d;' % (-d))
        for j, entry in enumerate(self.explicit):
            if entry[0] == 'local' or (entry[0] == 'loaded' and entry[2] != j - d):
                out.append('cpu.fpu.st(%d) = %s;' % (j, entry[1]))
        out.append('}')
        if CHECK:
            _check(self.recorded_in, self.recorded_out + out)
        self.open = False
        self._clear()
        return out

    def transform(self, lines):
        """`lines` of one instruction, with the stack in locals.  Lines that do
        not touch the FPU come back as they are (and do not open a run)."""
        if not any(_ANY_FPU.search(line) for line in lines):
            if self.open and CHECK:
                self.recorded_in.append(list(lines))
                self.recorded_out.extend(lines)
            return lines
        out = []
        if not self.open:
            out.append('{  // x87 stack in locals (codegen/fpu_stack.py)')
            self.open = True
        if CHECK:
            self.recorded_in.append(list(lines))
        for raw in lines:
            line = raw.strip()
            if line in ('{', '}'):
                continue
            if not _ANY_FPU.search(line) and not any(re.search(r'\b%s\b' % a, line) for a in self.aliases):
                out.append(line)
                continue
            if line == _POP:
                self._pop()
                continue
            m = _TEMP.match(line)
            if m:
                entry = self._entry_of(m.group(2))
                if entry is None:
                    name = self._fresh()
                    out.append('x86::Float %s = %s;' % (name, self._substitute(m.group(2), out)))
                    entry = ('local', name)
                self.aliases[m.group(1)] = entry
                continue
            m = _ASSIGN_ST.match(line)
            if m:
                n = int(m.group(1))
                entry = self._entry_of(m.group(2))
                if entry is None:
                    value = self._substitute(m.group(2), out)
                    name = self._fresh()
                    out.append('x86::Float %s = %s;' % (name, value))
                    entry = ('local', name)
                self._set(n, entry)
                continue
            m = _PUSH.match(line)
            if m:
                entry = self._entry_of(m.group(1))
                if entry is None:
                    value = self._substitute(m.group(1), out)
                    name = self._fresh()
                    out.append('x86::Float %s = %s;' % (name, value))
                    entry = ('local', name)
                self._push(entry)
                continue
            out.append(self._substitute(line, out))
        self.aliases = {}
        if CHECK:
            self.recorded_out.extend(out)
        return out


# -- CHECK -------------------------------------------------------------------
# Values are the text of the expressions that make them, with every value used
# inside another in brackets; a register or temporary copied bare is the same
# value, not a new one.  Two runs agree when they leave the same values in the
# same places and do the same other things in the same order.

_BASE = 100


def _evaluate(expression, read_st, names):
    bare = re.match(r'^cpu\.fpu\.st\((\d+)\)$', expression)
    if bare:
        return read_st(int(bare.group(1)))
    if expression in names:
        return names[expression]
    text = _ST.sub(lambda m: '(' + read_st(int(m.group(1))) + ')', expression)
    for name in sorted(names, key=len, reverse=True):
        text = re.sub(r'\b%s\b' % re.escape(name), lambda m, v=names[name]: '(' + v + ')', text)
    return text


def _initial():
    return {_BASE - k: 'orig%d' % k for k in range(-16, 17)}


def _reference(instructions):
    count = _BASE
    regs = _initial()
    effects = []
    for lines in instructions:
        temps = {}
        for raw in lines:
            line = raw.strip()
            if line in ('{', '}'):
                continue
            read = lambda n: regs[count - n]
            if line == _POP:
                count -= 1
                continue
            m = _TEMP.match(line)
            if m:
                temps[m.group(1)] = _evaluate(m.group(2), read, temps)
                continue
            m = _ASSIGN_ST.match(line)
            if m:
                value = _evaluate(m.group(2), read, temps)
                regs[count - int(m.group(1))] = value
                continue
            m = _PUSH.match(line)
            if m:
                value = _evaluate(m.group(1), read, temps)
                count += 1
                regs[count] = value
                continue
            effects.append(_evaluate(line, read, temps))
    return count, regs, effects


def _transformed(lines):
    count = _BASE
    regs = _initial()
    effects = []
    names = {}
    for raw in lines:
        line = raw.strip()
        if line == '}' or line.startswith('{'):
            continue
        read = lambda n: regs[count - n]
        m = _TEMP.match(line)
        if m and m.group(1).startswith('fpu'):
            names[m.group(1)] = _evaluate(m.group(2), read, names)
            continue
        m = re.match(r'^cpu\.fpu\.count ([+-])= (\d+);$', line)
        if m:
            count += int(m.group(2)) * (1 if m.group(1) == '+' else -1)
            continue
        m = re.match(r'^cpu\.fpu\.st\((\d+)\) = (fpu\d+);$', line)
        if m:
            regs[count - int(m.group(1))] = names[m.group(2)]
            continue
        effects.append(_evaluate(line, read, names))
    return count, regs, effects


def _live(state):
    """What a stack leaves that anything can read: its count, st(0)..st(7) and
    the side effects.  A slot above the top is empty to the x87 -- the next
    push writes it before anything reads it -- so what the generated code left
    there after a pop is no difference."""
    count, regs, effects = state
    return count, {position: regs.get(position) for position in range(count - 7, count + 1)}, effects


def _check(instructions, emitted):
    global checked_runs
    reference = _live(_reference(instructions))
    result = _live(_transformed(emitted))
    if reference != result:
        detail = ['run does not match:', '--- as generated:']
        detail += ['  ' + l for lines in instructions for l in lines]
        detail += ['--- as a run:'] + ['  ' + l for l in emitted]
        if reference[0] != result[0]:
            detail.append('count %d != %d' % (reference[0], result[0]))
        for position in sorted(set(reference[1]) | set(result[1])):
            a, b = reference[1].get(position), result[1].get(position)
            if a != b:
                detail.append('st at %d: %s != %s' % (position - reference[0], a, b))
        if reference[2] != result[2]:
            detail.append('effects differ:')
            detail += ['  < ' + e for e in reference[2]] + ['  > ' + e for e in result[2]]
        raise RuntimeError('\n'.join(detail))
    checked_runs += 1
