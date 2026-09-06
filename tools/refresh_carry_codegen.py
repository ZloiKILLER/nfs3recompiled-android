"""Refresh only ADC/SBB flag-emission blocks, after exact old/new matching.

Default is a dry run. --write updates validated blocks without rerunning the
disassembler or changing any unrelated generated instruction.
"""
from pathlib import Path
import argparse
import importlib
import json
import re
import sys
import tempfile

from audit_upstream_arithmetic import BASE, ROOT, original


def main():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--write', action='store_true')
    args = parser.parse_args()
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    pattern = re.compile(r'^    // ([0-9a-f]{8})  ([0-9a-f]+)\s+\+(adc|sbb) ', re.M)
    updates = []
    sites = []
    with tempfile.TemporaryDirectory(prefix='nfs-carry-refresh-') as temp:
        sys.path.insert(0, temp)
        modules = []
        for name, upstream in [('carry_old', True), ('carry_new', False)]:
            package = Path(temp) / name
            package.mkdir()
            (package / '__init__.py').write_text('')
            for filename in ['arguments.py', 'arithmetic.py']:
                rel = 'disasm/codegen/' + filename
                (package / filename).write_text(original(rel) if upstream else (ROOT / rel).read_text())
            modules.append(importlib.import_module(name + '.arithmetic'))
        for path in sorted((ROOT / 'src').glob('*/disassembly/*.cpp')):
            raw = path.read_bytes()
            newline = '\r\n' if b'\r\n' in raw else '\n'
            source = raw.decode().replace('\r\n', '\n')
            replacements = []
            for match in pattern.finditer(source):
                ins = next(decoder.disasm(bytes.fromhex(match[2]), int(match[1], 16)))
                ins.precious_flags = True
                bodies = [''.join('    ' + line + '\n' for line in getattr(mod, 'cg_' + match[3])(ins, (0, 0), {}, *ins.operands)) for mod in modules]
                start = source.index('\n', match.start()) + 1
                old, new = bodies
                if source.startswith(new, start):
                    continue
                if not source.startswith(old, start):
                    raise RuntimeError(f'Unexpected generated body at {path}:{match[1]}; no files written')
                replacements.append((start, start + len(old), new))
                sites.append({'file': str(path.relative_to(ROOT)), 'guest_address': match[1], 'instruction': ins.mnemonic + ' ' + ins.op_str})
            if replacements:
                for start, end, new in reversed(replacements):
                    source = source[:start] + new + source[end:]
                updates.append((path, raw, source.replace('\n', newline).encode()))
    # Validate every file before the first write, including concurrent edits.
    for path, raw, _ in updates:
        if path.read_bytes() != raw:
            raise RuntimeError(f'Concurrent modification: {path}; no files written')
    if args.write:
        for path, _, new in updates:
            path.write_bytes(new)
        output = ROOT / 'diagnostics' / 'carry-refresh.json'
        output.parent.mkdir(exist_ok=True)
        if sites:
            output.write_text(json.dumps({'base': BASE, 'files': len(updates), 'sites': sites}, indent=2))
    print(json.dumps({'write': args.write, 'files': len(updates), 'instructions': len(sites)}))


if __name__ == '__main__':
    main()
