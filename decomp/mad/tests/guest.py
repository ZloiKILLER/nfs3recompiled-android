"""Run functions of the original nfs3.exe in Unicorn (x86-32, QEMU's x87).

The executable is mapped at its own base with its sections as they are on
disk; the BSS is zero, as on Windows.  A call goes through a small stub that
resets the FPU and loads a control word first, so each test chooses the x87
precision (0x27f: 53-bit, Windows' default for a Win32 thread and what the
port's emulated FPU gives; 0x37f: 64-bit extended).
"""
import os
import struct
import sys

from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_PROT_ALL
from unicorn.x86_const import (UC_X86_REG_EAX, UC_X86_REG_EBX, UC_X86_REG_ECX, UC_X86_REG_EDX,
                               UC_X86_REG_ESI, UC_X86_REG_EDI, UC_X86_REG_EBP, UC_X86_REG_ESP)

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', 'tools'))
from disx import pe  # noqa: E402

STUB = 0x0e000000
STACK = 0x0f000000
STACK_SIZE = 0x100000
DATA = 0x10000000
DATA_SIZE = 0x01000000

REGS = {'eax': UC_X86_REG_EAX, 'ebx': UC_X86_REG_EBX, 'ecx': UC_X86_REG_ECX, 'edx': UC_X86_REG_EDX,
        'esi': UC_X86_REG_ESI, 'edi': UC_X86_REG_EDI, 'ebp': UC_X86_REG_EBP, 'esp': UC_X86_REG_ESP}


def _round_up(v, a=0x1000):
    return (v + a - 1) & ~(a - 1)


class Guest:
    def __init__(self, control_word=0x27f):
        self.cw = control_word
        self.uc = Uc(UC_ARCH_X86, UC_MODE_32)
        base = pe.OPTIONAL_HEADER.ImageBase
        size = _round_up(pe.OPTIONAL_HEADER.SizeOfImage)
        self.uc.mem_map(base, size, UC_PROT_ALL)
        for s in pe.sections:
            raw = s.get_data()[:s.Misc_VirtualSize] if s.Misc_VirtualSize else s.get_data()
            self.uc.mem_write(base + s.VirtualAddress, raw)
        self.uc.mem_map(STUB, 0x1000, UC_PROT_ALL)
        self.uc.mem_map(STACK, STACK_SIZE, UC_PROT_ALL)
        self.uc.mem_map(DATA, DATA_SIZE, UC_PROT_ALL)

    # memory helpers
    def read(self, addr, n):
        return bytes(self.uc.mem_read(addr, n))

    def write(self, addr, data):
        self.uc.mem_write(addr, bytes(data))

    def u32(self, addr):
        return struct.unpack('<I', self.read(addr, 4))[0]

    def put_u32(self, addr, v):
        self.write(addr, struct.pack('<I', v & 0xffffffff))

    def call(self, target, stack_args=(), **regs):
        """Call `target` with registers (Watcom register convention) and
        dword stack arguments; returns the registers afterwards."""
        code = bytearray()
        code += b'\xdb\xe3'                                   # fninit
        code += b'\xd9\x2d' + struct.pack('<I', STUB + 0x800)  # fldcw [cw]
        for a in reversed(stack_args):
            code += b'\x68' + struct.pack('<I', a & 0xffffffff)  # push imm32
        call_at = STUB + len(code)
        code += b'\xe8' + struct.pack('<i', target - (call_at + 5))
        end = STUB + len(code)
        self.write(STUB, code)
        self.write(STUB + 0x800, struct.pack('<H', self.cw))
        defaults = {'eax': 0, 'ebx': 0, 'ecx': 0, 'edx': 0,
                    'esi': 0x5e5e5e5e, 'edi': 0xd1d1d1d1, 'ebp': 0xb0b0b0b0}
        defaults.update(regs)
        for k, v in defaults.items():
            self.uc.reg_write(REGS[k], v & 0xffffffff)
        self.uc.reg_write(UC_X86_REG_ESP, STACK + STACK_SIZE - 0x100)
        self.uc.emu_start(STUB, end, count=50_000_000)
        return {k: self.uc.reg_read(r) for k, r in REGS.items()}
