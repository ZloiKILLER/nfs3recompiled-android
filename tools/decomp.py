"""decomp.py <asm file> <name> <entry address hex> [--table ADDR=N ...]: a whole function's listing as C++ in the port's style.

Registers are reg32 locals, the stack is the guest's (esp a local), flags are the runtime struct `fl`, the x87 is
the local `x` (doubles; fistp rounds with stored(cpu, ...), __CHP truncates), calls go through rawCall with the
arguments read back from the guest stack.  Out: `bool NAME(App* app, CPU& cpu, const Entry& e, reg32& result)`."""
import re
import struct
import sys

R32 = ('eax', 'ebx', 'ecx', 'edx', 'esi', 'edi', 'ebp', 'esp')
R16 = {'ax': 'eax', 'bx': 'ebx', 'cx': 'ecx', 'dx': 'edx', 'si': 'esi', 'di': 'edi', 'bp': 'ebp'}
R8L = {'al': 'eax', 'bl': 'ebx', 'cl': 'ecx', 'dl': 'edx'}
R8H = {'ah': 'eax', 'bh': 'ebx', 'ch': 'ecx', 'dh': 'edx'}
CC = ('e', 'ne', 'l', 'le', 'g', 'ge', 'b', 'be', 'a', 'ae', 's', 'ns', 'z', 'nz', 'nae', 'nb')
CCMAP = {'z': 'e', 'nz': 'ne', 'nae': 'b', 'nb': 'ae'}
SIZES = {'byte': 8, 'word': 16, 'dword': 32, 'qword': 64}


def num(s):
    s = s.strip()
    return int(s, 16) if re.match(r'^-?0x', s) else int(s)


def addr(expr):
    expr = expr.replace(' ', '')
    terms = re.findall(r'[+-]?[^+-]+', expr)
    out = []
    for t in terms:
        sign = '-' if t.startswith('-') else '+'
        t = t.lstrip('+-')
        m = re.match(r'(\w+)\*(\d+)$', t)
        if m:
            out.append((sign, '%s * %su' % (m.group(1), m.group(2))))
        elif t in R32:
            out.append((sign, t))
        else:
            out.append((sign, '%su' % t if t.startswith('0x') else t + 'u'))
    s = ''
    for i, (sign, t) in enumerate(out):
        s += (t if i == 0 and sign == '+' else (' %s %s' % (sign, t) if i else '-' + t))
    return '(' + s + ')'


class Op:
    def __init__(self, text):
        t = text.strip()
        self.text = t
        self.kind = None
        m = re.match(r'(?:(byte|word|dword|qword) ptr )?(?:\w\w:)?\[(.*)\]$', t)
        if m and ('ptr' in t or t.startswith('[') or ':' in t):
            self.kind = 'mem'
            self.size = SIZES.get(m.group(1), 32)
            self.addr = addr(m.group(2))
        elif t in R32:
            self.kind, self.size, self.reg = 'reg', 32, t
        elif t in R16:
            self.kind, self.size, self.reg = 'reg', 16, t
        elif t in R8L or t in R8H:
            self.kind, self.size, self.reg = 'reg', 8, t
        elif re.match(r'^-?(0x[0-9a-f]+|\d+)$', t):
            self.kind, self.value = 'imm', num(t)
        elif re.match(r'^st\((\d)\)$', t):
            self.kind, self.index = 'st', int(t[3])
        else:
            raise ValueError('operand ' + text)

    def get(self, size=None):
        size = size or getattr(self, 'size', 32)
        if self.kind == 'imm':
            return '%du' % (self.value & 0xffffffff) if self.value >= 0 else 'reg32(%d)' % self.value
        if self.kind == 'mem':
            return {8: 'rd8(%s)', 16: 'rd16(%s)', 32: 'rd32(%s)'}[self.size] % self.addr
        t = self.text
        if t in R32:
            return t
        if t in R16:
            return '(%s & 0xffffu)' % R16[t]
        if t in R8L:
            return '(%s & 0xffu)' % R8L[t]
        return '((%s >> 8) & 0xffu)' % R8H[t]

    def set(self, v):
        if self.kind == 'mem':
            return {8: 'wr8(%s, %s);', 16: 'wr16(%s, %s);', 32: 'wr32(%s, %s);'}[self.size] % (self.addr, v)
        t = self.text
        if t in R32:
            return '%s = %s;' % (t, v)
        if t in R16:
            r = R16[t]
            return '%s = (%s & 0xffff0000u) | ((%s) & 0xffffu);' % (r, r, v)
        if t in R8L:
            r = R8L[t]
            return '%s = (%s & 0xffffff00u) | ((%s) & 0xffu);' % (r, r, v)
        r = R8H[t]
        return '%s = (%s & 0xffff00ffu) | (((%s) & 0xffu) << 8);' % (r, r, v)


def fop(op):
    if op.kind == 'st':
        return 'x.st(%d)' % op.index
    if op.size == 32:
        return 'F%s' % op.addr
    return 'D%s' % op.addr


def parse(path):
    lines = []
    for l in open(path):
        m = re.match(r'([0-9a-f]{8})\s+[0-9a-f]+\s+[-+]?\s*(.*)', l.strip())
        if m:
            lines.append((int(m.group(1), 16), m.group(2).strip()))
    return lines


def table(exe, ad, count):
    d = open(exe, 'rb').read()
    pe = struct.unpack_from('<I', d, 0x3c)[0]
    nsec = struct.unpack_from('<H', d, pe + 6)[0]
    opt = pe + 24
    sz = struct.unpack_from('<H', d, pe + 20)[0]
    base = struct.unpack_from('<I', d, opt + 28)[0]
    for i in range(nsec):
        o = opt + sz + 40 * i
        vs, va, rs, ra = struct.unpack_from('<IIII', d, o + 8)
        rva = ad - base
        if va <= rva < va + max(vs, rs):
            off = ra + rva - va
            return list(struct.unpack_from('<%dI' % count, d, off))
    raise ValueError('table')


def main():
    path, name, entry = sys.argv[1], sys.argv[2], int(sys.argv[3], 16)
    tables = {}
    for a in sys.argv[4:]:
        k, v = a.split('=')
        tables[int(k, 16)] = int(v)
    lines = parse(path)
    starts = {a for a, _ in lines}
    targets = set()
    for a, ins in lines:
        m = re.match(r'(j\w+|jmp) (0x[0-9a-f]+)$', ins)
        if m:
            targets.add(int(m.group(2), 16))
    for ad, n in tables.items():
        targets |= set(table('nfs3hp/nfs3.exe', ad, n))
    out = []
    emit = out.append
    pending = 0
    after_call = False
    prologue_done = False
    seen_frame = False
    popped = 0
    idx = {a: i for i, (a, _) in enumerate(lines)}
    i = 0
    while i < len(lines):
        a, ins = lines[i]
        if a in targets:
            emit('L_%x:' % a)
        mn, _, rest = ins.partition(' ')
        rep = ''
        if mn in ('rep', 'repne', 'repe'):
            rep = mn
            mn, _, rest = rest.partition(' ')
        c = []
        was_call = False
        ops = []
        try:
            ops = [Op(o) for o in re.split(r',(?![^\[]*\])', rest)] if rest and not mn.startswith(('j', 'call', 'movsd', 'movsb', 'stosb', 'scasb', 'stosd', 'ret', 'cwde', 'wait', 'fsqrt', 'fsin', 'fcos', 'fabs')) else []
            if mn == 'push':
                c.append('esp -= 4; wr32(esp, %s);' % ops[0].get())
                pending += 1
            elif mn == 'pop':
                c.append(ops[0].set('rd32(esp)') + ' esp += 4;')
                pending -= 1
            elif mn == 'mov':
                c.append(ops[0].set(ops[1].get()))
            elif mn in ('movzx', 'movsx'):
                v = ops[1].get()
                if mn == 'movsx':
                    s = ops[1].size
                    v = 'reg32(int32_t(int%d_t(%s)))' % (s, v)
                c.append(ops[0].set(v))
            elif mn == 'lea':
                m = re.match(r'\[(.*)\]$', ops[1].text) if False else None
                c.append(ops[0].set(ops[1].addr))
            elif mn in ('add', 'sub', 'and', 'or', 'xor', 'adc', 'sbb'):
                if ops[0].text == 'esp' and mn in ('sub', 'add'):
                    n = ops[1].value
                    if mn == 'sub':
                        c.append('esp -= %du;' % n)
                        if prologue_done:
                            pending += n // 4
                    elif not after_call:
                        c.append('esp += %du;' % n)
                        if prologue_done:
                            pending -= n // 4
                    else:
                        c.append('// (cleanup after the call: esp is back already)')
                else:
                    sym = {'add': '+', 'sub': '-', 'and': '&', 'or': '|', 'xor': '^'}.get(mn)
                    d, s = ops[0], ops[1]
                    if mn == 'adc':
                        raise ValueError('adc ' + ins)
                    if mn == 'sbb':
                        c.append('{ const reg32 r_ = %s - %s - carry_; %s fl.logic(r_); }' % (d.get(), s.get(), d.set('r_')))
                        raise StopIteration
                    if mn == 'xor' and d.text == s.text:
                        c.append(d.set('0u'))
                        c.append('fl.logic(0u);')
                    elif mn == 'sub' or mn == 'cmp':
                        c.append('{ const reg32 a_ = %s, b_ = %s; %s fl.sub(a_, b_); }' % (d.get(), s.get(), d.set('a_ - b_')))
                    elif mn == 'add':
                        c.append('{ const reg32 a_ = %s, b_ = %s; %s fl.add(a_, b_); }' % (d.get(), s.get(), d.set('a_ + b_')))
                    else:
                        c.append('{ const reg32 r_ = %s %s %s; %s fl.logic(r_); }' % (d.get(), sym, s.get(), d.set('r_')))
            elif mn == 'cmp':
                c.append('fl.sub(%s, %s);' % (ops[0].get(), ops[1].get()))
            elif mn == 'test':
                c.append('fl.logic(%s & %s);' % (ops[0].get(), ops[1].get()))
            elif mn in ('inc', 'dec'):
                sym = '+' if mn == 'inc' else '-'
                c.append('{ const reg32 r_ = %s %s 1u; %s fl.arith(r_); }' % (ops[0].get(), sym, ops[0].set('r_')))
            elif mn == 'neg':
                c.append('{ const reg32 v_ = %s; %s fl.sub(0u, v_); }' % (ops[0].get(), ops[0].set('0u - v_')))
            elif mn == 'not':
                c.append(ops[0].set('~' + ops[0].get()))
            elif mn in ('shl', 'shr', 'sar', 'sal') and ops[1].kind != 'imm':
                g = ops[0].get()
                cnt = '(ecx & 31u)'
                r = {'shl': '%s << %s', 'sal': '%s << %s', 'shr': '%s >> %s', 'sar': 'reg32(int32_t(%s) >> %s)'}[mn] % (g, cnt)
                c.append('if (%s != 0) { const reg32 r_ = %s; %s fl.logic(r_); }' % (cnt, r, ops[0].set('r_')))
            elif mn in ('shl', 'shr', 'sar', 'sal'):
                n = ops[1].value if ops[1].kind == 'imm' else None
                g = ops[0].get()
                if mn in ('shl', 'sal'):
                    r = '%s << %d' % (g, n)
                elif mn == 'shr':
                    r = '%s >> %d' % (g, n)
                else:
                    r = 'reg32(int32_t(%s) >> %d)' % (g, n)
                if mn in ('shl', 'sal'):
                    c.append('carry_ = (%s >> %d) & 1u;' % (g, 32 - n))
                elif mn == 'shr':
                    c.append('carry_ = (%s >> %d) & 1u;' % (g, n - 1))
                else:
                    c.append('carry_ = (%s >> %d) & 1u;' % (g, n - 1))
                c.append('{ const reg32 r_ = %s; %s fl.logic(r_); }' % (r, ops[0].set('r_')))
            elif mn == 'imul':
                if len(ops) == 3:
                    c.append(ops[0].set('reg32(int32_t(%s) * int32_t(%s))' % (ops[1].get(), ops[2].get())))
                else:
                    c.append(ops[0].set('reg32(int32_t(%s) * int32_t(%s))' % (ops[0].get(), ops[1].get())))
            elif mn == 'div':
                c.append('{ const uint64_t n_ = (uint64_t(edx) << 32) | eax; const reg32 d_ = %s; eax = reg32(n_ / d_); edx = reg32(n_ %% d_); }' % ops[0].get())
            elif mn == 'mul':
                c.append('{ const uint64_t p_ = uint64_t(eax) * uint64_t(%s); eax = reg32(p_); edx = reg32(p_ >> 32); }' % ops[0].get())
            elif mn == 'imul' and len(ops) == 1:
                c.append('{ const int64_t p_ = int64_t(int32_t(eax)) * int64_t(int32_t(%s)); eax = reg32(p_); edx = reg32(uint64_t(p_) >> 32); }' % ops[0].get())
            elif mn == 'cwde':
                c.append('eax = reg32(int32_t(int16_t(eax & 0xffffu)));')
            elif mn == 'xchg':
                c.append('{ const reg32 a_ = %s, b_ = %s; %s %s }' % (ops[0].get(), ops[1].get(), ops[0].set('b_'), ops[1].set('a_')))
            elif mn == 'wait':
                pass
            elif mn == 'fsqrt':
                c.append('x.st(0) = std::sqrt(x.st(0));')
            elif mn == 'fsin':
                c.append('x.st(0) = std::sin(x.st(0));')
            elif mn == 'fcos':
                c.append('x.st(0) = std::cos(x.st(0));')
            elif mn == 'fabs':
                c.append('x.st(0) = std::fabs(x.st(0));')
            elif mn == 'idiv':
                c.append('{ const int32_t n_ = int32_t(eax), d_ = int32_t(%s); eax = reg32(n_ / d_); edx = reg32(n_ %% d_); }' % ops[0].get())
            elif mn == 'cdq':
                c.append('edx = reg32(int32_t(eax) >> 31);')
            elif mn.startswith('set'):
                cc = mn[3:]
                cc = CCMAP.get(cc, cc)
                c.append(ops[0].set('(fl.%s() ? 1u : 0u)' % cc))
            elif mn == 'nop':
                pass
            elif mn == 'jmp':
                m = re.match(r'(0x[0-9a-f]+)$', rest.strip())
                if m:
                    c.append('goto L_%x;' % int(m.group(1), 16))
                else:
                    t = re.match(r'dword ptr \[(\w+)\*4 \+ (0x[0-9a-f]+)\]$', rest.strip())
                    ad = int(t.group(2), 16)
                    ent = table('nfs3hp/nfs3.exe', ad, tables[ad])
                    c.append('switch (%s) {' % t.group(1))
                    for k, tg in enumerate(ent):
                        c.append('case %d: goto L_%x;' % (k, tg))
                    c.append('default: break; }')
                    c.append('return false;  // (no entry)')
            elif mn.startswith('j'):
                cc = mn[1:]
                cc = CCMAP.get(cc, cc)
                tg = int(rest.strip(), 16)
                c.append('if (fl.%s()) goto L_%x;' % (cc, tg))
            elif mn == 'call':
                tg = rest.strip()
                if tg.startswith('dword ptr') or tg in R32:
                    tg = Op(tg).get() if tg not in R32 else tg
                if tg == '0x4dfd56':
                    c.append('x.st(0) = std::trunc(x.st(0));  // __CHP')
                else:
                    n = pending
                    c.append('{ const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, %s, esp, %d, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }' % (tg, n))
                    c.append('esp += %du;' % (4 * n))
                    pending = 0
                    was_call = True
            elif mn == 'ret':
                popped = num(rest) if rest.strip() else 0
                c.append('for (int i_ = 0; i_ < x.n; ++i_) { cpu.fpu.count += 1; cpu.fpu.st(0) = x86::Float(x.v[i_]); }')
                c.append('result = eax; popped_ = %du; return true;' % popped)
            elif mn == 'movsd':
                c.append('wr32(edi, rd32(esi)); esi += 4; edi += 4;' if not rep else 'for (; ecx != 0; --ecx) { wr32(edi, rd32(esi)); esi += 4; edi += 4; }')
            elif mn == 'scasb':
                c.append('while (ecx != 0) { --ecx; const reg32 a_ = eax & 0xffu, b_ = rd8(edi); edi += 1; fl.sub(a_, b_); if (a_ == b_) break; }')
            elif mn == 'movsb':
                c.append('wr8(edi, rd8(esi)); esi += 1; edi += 1;' if not rep else 'for (; ecx != 0; --ecx) { wr8(edi, rd8(esi)); esi += 1; edi += 1; }')
            elif mn == 'stosb':
                c.append('wr8(edi, eax); edi += 1;' if not rep else 'for (; ecx != 0; --ecx) { wr8(edi, eax); edi += 1; }')
            elif mn == 'stosd':
                c.append('wr32(edi, eax); edi += 4;' if not rep else 'for (; ecx != 0; --ecx) { wr32(edi, eax); edi += 4; }')
            # x87
            elif mn == 'fld':
                c.append('x.push(%s);' % fop(ops[0]))
            elif mn == 'fld1':
                c.append('x.push(1.0);')
            elif mn == 'fldz':
                c.append('x.push(0.0);')
            elif mn == 'fild':
                c.append('x.push(double(int32_t(%s)));' % ops[0].get())
            elif mn in ('fst', 'fstp'):
                if ops[0].kind == 'mem':
                    c.append('%s(%s, x.st(0));' % ('SF' if ops[0].size == 32 else 'SD', ops[0].addr))
                    if mn == 'fstp':
                        c.append('x.pop();')
                else:
                    c.append('x.%s(%d);' % ('fstp_st' if mn == 'fstp' else 'fst_st', ops[0].index))
            elif mn == 'fistp':
                c.append('wr32(%s, stored(cpu, x.pop()));' % ops[0].addr)
            elif mn == 'fxch':
                c.append('x.fxch(%d);' % (ops[0].index if ops else 1))
            elif mn == 'fchs':
                c.append('x.st(0) = -x.st(0);')
            elif mn in ('fadd', 'fsub', 'fsubr', 'fmul', 'fdiv', 'fdivr', 'faddp', 'fsubp', 'fsubrp', 'fmulp', 'fdivp', 'fdivrp'):
                pop = mn.endswith('p')
                base_ = mn[:-1] if pop else mn
                sym = {'fadd': '+', 'fsub': '-', 'fsubr': '-', 'fmul': '*', 'fdiv': '/', 'fdivr': '/'}[base_]
                rev = base_ in ('fsubr', 'fdivr')
                if len(ops) == 2:
                    dest, src = ops
                elif pop:
                    dest, src = ops[0], Op('st(0)')
                else:
                    dest, src = Op('st(0)'), ops[0]
                d, s = fop(dest), fop(src)
                # Through the FPU's own operation: it rounds as the control word asks (a race's single precision).
                fn = {'+': 'xadd', '-': 'xsub', '*': 'xmul', '/': 'xdiv'}[sym]
                c.append('%s = %s(cpu, %s);' % (d, fn, ('%s, %s' % (s, d)) if rev else ('%s, %s' % (d, s))))
                if pop:
                    c.append('x.pop();')
            elif mn in ('fcom', 'fcomp', 'fcompp'):
                other = fop(ops[0]) if ops else 'x.st(1)'
                c.append('fl.fcmp(x.st(0), %s);' % other)
                if mn == 'fcompp':
                    c.append('x.pop(); x.pop();')
                elif mn == 'fcomp':
                    c.append('x.pop();')
            elif mn == 'fnstsw':
                c.append('eax = (eax & 0xffff0000u) | fl.status();')
            elif mn == 'sahf':
                c.append('// ' + ins)
            else:
                raise ValueError('unsupported ' + ins)
        except StopIteration:
            pass
        except Exception as ex:
            c = ['#error "%s: %s"' % (ins, ex)]
        after_call = was_call or (after_call and mn == 'add' and ops and ops[0].text == 'esp')
        if not prologue_done:
            if mn == 'mov' and rest.strip() == 'ebp, esp':
                seen_frame = True
            elif seen_frame and not (mn == 'sub' and rest.startswith('esp')):
                prologue_done = True
                pending = 0
        for line in c:
            emit('    %s  // %x %s' % (line, a, ins) if not line.startswith('#') else line)
        i += 1
    body = '\n'.join(out)
    print(HEAD % (name,))
    print(body)
    print('    return false;\n}')
    print('// popped_ = %d' % popped)


_frame = {'set': False}


def out_has_frame(out):
    return _frame['set']


def in_prologue_locals(out):
    return not _frame['set']


def out_set_frame(out):
    _frame['set'] = True


HEAD = '''bool %s(App* app, CPU& cpu, const Entry& e, reg32& result, reg32& popped_)
{
    reg32 eax = e.eax, ebx = e.ebx, ecx = e.ecx, edx = e.edx, esi = e.esi, edi = e.edi, ebp = e.ebp, esp = e.esp;
    Flags fl;
    X87 x;
    reg32 carry_ = 0;
    (void)carry_;
    const auto rd32 = [&](reg32 a) { return app->getMemory<reg32>(a); };
    const auto rd16 = [&](reg32 a) { return reg32(app->getMemory<uint16_t>(a)); };
    const auto rd8 = [&](reg32 a) { return reg32(app->getMemory<uint8_t>(a)); };
    const auto wr32 = [&](reg32 a, reg32 v) { app->getMemory<reg32>(a) = v; };
    const auto wr16 = [&](reg32 a, reg32 v) { app->getMemory<uint16_t>(a) = uint16_t(v); };
    const auto wr8 = [&](reg32 a, reg32 v) { app->getMemory<uint8_t>(a) = uint8_t(v); };
    const auto F = [&](reg32 a) { return double(app->getMemory<float>(a)); };
    const auto D = [&](reg32 a) { return app->getMemory<double>(a); };
    const auto SF = [&](reg32 a, double v) { app->getMemory<float>(a) = float(v); };
    const auto SD = [&](reg32 a, double v) { app->getMemory<double>(a) = v; };'''

if __name__ == '__main__':
    main()
