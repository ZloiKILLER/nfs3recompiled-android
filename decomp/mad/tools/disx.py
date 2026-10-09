"""Disassemble functions of nfs3.exe straight from the binary (read-only).

usage: dis.py 0x4f3600 [0x4f3560 ...]
Function ends come from the next function start the recompiler found.
"""
import os
import re
import sys
import glob

HERE = os.path.dirname(os.path.abspath(__file__))
# The repository root: this file lives in decomp/mad/tools; NFS3_REPO overrides.
REPO = os.environ.get('NFS3_REPO') or os.path.normpath(os.path.join(HERE, '..', '..', '..'))
sys.path.insert(0, os.path.join(REPO, 'disasm'))
import pefile  # noqa: E402
import capstone  # noqa: E402

pe = pefile.PE(os.path.join(REPO, 'nfs3hp/nfs3.exe'))
BASE = pe.OPTIONAL_HEADER.ImageBase
IMAGE = pe.get_memory_mapped_image()

IAT = {}
for e in pe.DIRECTORY_ENTRY_IMPORT:
    for i in e.imports:
        if i.name:
            IAT[i.address] = i.name.decode()

_starts = None


def starts():
    global _starts
    if _starts is None:
        s = set()
        for p in glob.glob(os.path.join(REPO, 'src/nfs3hp/disassembly/nfs3hp.*.cpp')):
            for line in open(p, errors='ignore'):
                m = re.match(r'void Application::sub_([0-9a-f]+)\(', line)
                if m:
                    s.add(int(m.group(1), 16))
        _starts = sorted(s)
    return _starts


def end_of(addr):
    for s in starts():
        if s > addr:
            return s
    return addr + 0x1000


def read(addr, n):
    return IMAGE[addr - BASE: addr - BASE + n]


def dword(addr):
    return int.from_bytes(read(addr, 4), 'little')


def dis(addr, end=None):
    end = end or end_of(addr)
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    out = []
    for ins in md.disasm(read(addr, end - addr), addr):
        note = ''
        for a in re.findall(r'0x([0-9a-f]+)', ins.op_str):
            v = int(a, 16)
            if v in IAT:
                note += ' ; ' + IAT[v]
        out.append('%08x  %-24s %s %s%s' % (ins.address, ins.bytes.hex(), ins.mnemonic, ins.op_str, note))
    return out


if __name__ == '__main__':
    for a in sys.argv[1:]:
        a = int(a, 16)
        print('==== sub_%x .. %x' % (a, end_of(a)))
        print('\n'.join(dis(a)))
        print()
