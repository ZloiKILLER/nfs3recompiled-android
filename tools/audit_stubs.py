#!/usr/bin/env python3
r"""Inventory every stubbed/unimplemented entry point in the win32+Glide layer,
cross-referenced against what nfs3.exe actually asks for.

Two questions per function:

  * is it a stub, and of what kind.  A function that merely *contains* an
    NFS2_ASSERT(false) on some unsupported branch is implemented and is not
    listed; a stub is one whose entire body is that assert plus a return, or
    one that does nothing at all.
  * does the game reach it -- through the PE import table, or through a
    GetProcAddress name string in the binary, which is how the whole Glide
    driver is loaded (nfs3.exe imports no glide2x statically).

Writes diagnostics/stub-inventory.md.
"""
import io
import os
import re
import struct
import glob
import collections

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
EXE = os.path.join(ROOT, 'nfs3hp', 'nfs3.exe')


def pe_info(path):
    d = open(path, 'rb').read()
    pe = struct.unpack_from('<I', d, 0x3c)[0]
    nsec = struct.unpack_from('<H', d, pe + 6)[0]
    optsz = struct.unpack_from('<H', d, pe + 20)[0]
    opt = pe + 24
    secs = []
    st = opt + optsz
    for i in range(nsec):
        o = st + i * 40
        nm = d[o:o + 8].rstrip(b'\0').decode('latin1')
        vsize, va, rawsize, raw = struct.unpack_from('<IIII', d, o + 8)
        secs.append((nm, va, vsize, raw, rawsize))

    def rva2off(rva):
        for nm, va, vsize, raw, rawsize in secs:
            if va <= rva < va + max(vsize, rawsize):
                return raw + (rva - va)
        return None

    imports = collections.defaultdict(set)
    imp_rva = struct.unpack_from('<I', d, opt + 96 + 8)[0]
    o = rva2off(imp_rva)
    while True:
        oft, ts, fc, namerva, firstthunk = struct.unpack_from('<IIIII', d, o)
        if namerva == 0:
            break
        dll = d[rva2off(namerva):]
        dll = dll[:dll.index(b'\0')].decode('latin1').lower()
        to = rva2off(oft or firstthunk)
        idx = 0
        while True:
            v = struct.unpack_from('<I', d, to + idx * 4)[0]
            if v == 0:
                break
            if not (v & 0x80000000):
                s = d[rva2off(v) + 2:]
                imports[dll].add(s[:s.index(b'\0')].decode('latin1'))
            idx += 1
        o += 20

    dynamic = set()
    for m in re.finditer(rb'_([A-Za-z]\w{2,40})@\d{1,3}\x00', d):
        dynamic.add(m.group(1).decode('latin1'))
    return imports, dynamic


imports, dynamic = pe_info(EXE)
static_names = set()
for names in imports.values():
    static_names |= names

FUNC = re.compile(
    r'^(?:static\s+)?[A-Za-z_][\w:<>,\s\*&]*?[\s\*&]'
    r'((?:[A-Za-z_]\w*::)?[A-Za-z_]\w*)\s*\([^;]*?\)\s*$', re.M)


def split_functions(text):
    out = []
    for m in FUNC.finditer(text):
        i = text.find('{', m.end())
        if i < 0:
            continue
        if text[m.end():i].strip():
            continue
        depth, j = 0, i
        while j < len(text):
            if text[j] == '{':
                depth += 1
            elif text[j] == '}':
                depth -= 1
                if depth == 0:
                    break
            j += 1
        out.append((m.group(1), text[i + 1:j]))
    return out


ASSERT_FALSE = 'NFS2_ASSERT(false)'


def classify(body):
    stripped = re.sub(r'/\*.*?\*/', '', body, flags=re.S)
    stripped = re.sub(r'//[^\n]*', '', stripped)
    stmts = [s.strip() for s in stripped.split(';')]
    stmts = [s for s in stmts if s.strip('{}\n\r\t ')]
    stmts = [s for s in stmts if not s.startswith('NFS2_USE(')]

    asserts = [x for x in stmts if x.replace(' ', '').startswith('NFS2_ASSERT(')]
    has_false = any(x.replace(' ', '') == ASSERT_FALSE for x in asserts)
    rest = [x for x in stmts if x not in asserts]
    returns_only = all(re.match(r'^return\b', x) for x in rest) if rest else True

    if has_false and returns_only:
        return 'ЗАГЛУШКА: assert false + return'
    if not rest:
        return ('ПУСТАЯ: только проверяет аргументы' if asserts
                else 'ПУСТАЯ: не делает ничего')
    if returns_only and len(rest) == 1:
        v = rest[0].strip()
        if re.match(r'^return\s*(0|1|-1|x86::reg32\(-?\d+\)|nullptr|TRUE|FALSE|S_OK)?$', v):
            return ('ПУСТАЯ: проверяет аргументы, возвращает константу' if asserts
                    else 'ПУСТАЯ: возвращает константу')
    if has_false:
        return None
    if 'TODO' in body:
        return 'ЧАСТИЧНО: помечена TODO'
    return None


rows = []
for path in sorted(glob.glob(os.path.join(ROOT, 'src', 'lib', 'winapi', '**', '*.cpp'),
                             recursive=True)):
    text = io.open(path, 'r', encoding='utf-8', errors='replace').read()
    mod = os.path.basename(path)[:-4]
    for name, body in split_functions(text):
        kind = classify(body)
        if not kind:
            continue
        short = name.split('::')[-1]
        used = []
        if short in static_names:
            used.append('импорт')
        if short in dynamic:
            used.append('GetProcAddress')
        rows.append((mod, name, kind, ', '.join(used) or '—'))

rows.sort(key=lambda r: (r[3] == '—', r[0], r[1]))
used_rows = [r for r in rows if r[3] != '—']
unused_rows = [r for r in rows if r[3] == '—']

out = []
out.append('# Инвентаризация заглушек слоя win32/Glide')
out.append('')
out.append('Генерируется `tools/audit_stubs.py`. Заглушкой считается функция, всё тело')
out.append('которой — `NFS2_ASSERT(false)` с возвратом, либо которая не делает ничего,')
out.append('либо возвращает константу. Функции, где `NFS2_ASSERT(false)` стоит лишь на')
out.append('одной неподдержанной ветке, а в остальном реализованы, в список НЕ попадают.')
out.append('')
out.append('Колонка «зовёт игра» — сверка с таблицей импорта `nfs3.exe` и со строками')
out.append('имён для `GetProcAddress` в нём: glide2x целиком грузится вторым способом,')
out.append('статических импортов у него нет.')
out.append('')
out.append('Прочерк не означает «мертва»: методы COM-интерфейсов DirectDraw, DirectInput')
out.append('и DirectSound вызываются через vtable и по имени в бинарнике не ищутся.')
out.append('')
out.append('## Достижимые игрой — %d' % len(used_rows))
out.append('')
out.append('| Модуль | Функция | Состояние | Зовёт игра |')
out.append('|---|---|---|---|')
for mod, name, kind, used in used_rows:
    out.append('| %s | `%s` | %s | %s |' % (mod, name, kind, used))
out.append('')
out.append('## Без следов вызова по имени — %d' % len(unused_rows))
out.append('')
out.append('| Модуль | Функция | Состояние |')
out.append('|---|---|---|')
for mod, name, kind, used in unused_rows:
    out.append('| %s | `%s` | %s |' % (mod, name, kind))
out.append('')

dst = os.path.join(ROOT, 'diagnostics', 'stub-inventory.md')
io.open(dst, 'w', encoding='utf-8', newline='\n').write('\n'.join(out) + '\n')
print('всего заглушек: %d, достижимых игрой: %d' % (len(rows), len(used_rows)))
print('отчёт:', dst)
