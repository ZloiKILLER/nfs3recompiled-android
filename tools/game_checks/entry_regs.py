"""Which of Watcom's argument registers (eax, edx, ebx, ecx) a function of
nfs3.exe reads on entry before writing them -- its register arguments, by its
own code: every path from its entry followed (a call ends eax's life, a push
of a register saves it rather than reading it, a write of a part kills that
part only), up to its returns.

generate.py gives the stand-ins of the game's functions that many registers,
so that a check compares what a callee takes and not what a caller happens
to leave in the others (the port's rewrites pass only arguments)."""
from pathlib import Path

import capstone
import pefile
from capstone import x86 as X

ROOT = Path(__file__).resolve().parents[2]

PARTS = {}
for fam, (full, word, lo, hi) in {"eax": ("eax", "ax", "al", "ah"), "ebx": ("ebx", "bx", "bl", "bh"),
                                  "ecx": ("ecx", "cx", "cl", "ch"), "edx": ("edx", "dx", "dl", "dh")}.items():
    PARTS[full] = {(fam, "L"), (fam, "H"), (fam, "U")}
    PARTS[word] = {(fam, "L"), (fam, "H")}
    PARTS[lo] = {(fam, "L")}
    PARTS[hi] = {(fam, "H")}
ORDER = ("eax", "edx", "ebx", "ecx")


class Reader:
    def __init__(self, exe=ROOT / "nfs3hp/nfs3.exe"):
        pe = pefile.PE(str(exe))
        self.base = pe.OPTIONAL_HEADER.ImageBase
        self.image = pe.get_memory_mapped_image()
        self.md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        self.md.detail = True
        self.cache = {}

    def at(self, address):
        if address not in self.cache:
            offset = address - self.base
            self.cache[address] = next(self.md.disasm(self.image[offset:offset + 16], address), None)
        return self.cache[address]

    @staticmethod
    def access(i):
        read, written = i.regs_access()
        rd, wr = set(), set()
        for r in read:
            rd |= PARTS.get(i.reg_name(r), set())
        for r in written:
            wr |= PARTS.get(i.reg_name(r), set())
        two = len(i.operands) == 2 and all(op.type == X.X86_OP_REG for op in i.operands)
        if i.mnemonic in ("xor", "sub") and two and i.operands[0].reg == i.operands[1].reg:
            rd -= PARTS.get(i.reg_name(i.operands[0].reg), set())
        if i.mnemonic == "push" and i.operands and i.operands[0].type == X.X86_OP_REG:
            rd -= PARTS.get(i.reg_name(i.operands[0].reg), set())
        return rd, wr

    def entry_reads(self, entry, limit=400):
        need = set()
        work, seen = [(entry, frozenset(), 0)], set()
        while work:
            a, dead, n = work.pop()
            while (a, dead) not in seen and n <= limit:
                seen.add((a, dead))
                i = self.at(a)
                if i is None:
                    break
                n += 1
                if i.mnemonic == "call":
                    dead = dead | PARTS["eax"]
                    a += i.size
                    continue
                rd, wr = self.access(i)
                need |= {p[0] for p in rd - dead}
                dead = dead | wr
                if i.mnemonic.startswith("ret"):
                    break
                if i.mnemonic == "jmp":
                    if i.operands[0].type == X.X86_OP_IMM:
                        a = i.operands[0].imm
                        continue
                    break
                if i.mnemonic.startswith("j") and i.operands[0].type == X.X86_OP_IMM:
                    work.append((i.operands[0].imm, dead, n))
                a += i.size
        return need

    def register_arguments(self, entry):
        """How many of eax, edx, ebx, ecx (in that order) the function takes."""
        reads = self.entry_reads(entry)
        return max([ORDER.index(r) + 1 for r in reads if r in ORDER] or [0])

    def calls_to(self):
        """Every direct call in the code (a linear sweep of it, a byte on where it decodes none):
        its target and the address after it."""
        if not hasattr(self, "_calls"):
            self._calls = {}
            start, end = 0x401000, 0x534000
            offset, a = start - self.base, start
            while a < end:
                decoded = False
                for i in self.md.disasm(self.image[offset + (a - start):end - self.base], a):
                    decoded = True
                    if i.mnemonic == "call" and i.operands and i.operands[0].type == X.X86_OP_IMM:
                        self._calls.setdefault(i.operands[0].imm, []).append(i.address + i.size)
                    a = i.address + i.size
                if not decoded:
                    a += 1
        return self._calls

    def eax_read_after(self, at, limit=200):
        """Whether the code from `at` on -- where a call came back -- reads what eax was before it
        writes it all: an instruction reading it, a call to a function taking it, a return (handed
        on) or a jump or call through a register (unknown)."""
        work, seen = [(at, frozenset(), 0)], set()
        eax = PARTS["eax"]
        while work:
            a, dead, n = work.pop()
            while (a, dead) not in seen and n <= limit:
                seen.add((a, dead))
                if eax <= dead:
                    break
                i = self.at(a)
                if i is None:
                    return True
                n += 1
                if i.mnemonic == "call":
                    if not (i.operands and i.operands[0].type == X.X86_OP_IMM):
                        return True
                    if self.register_arguments(i.operands[0].imm) >= 1:
                        return True
                    dead = dead | eax
                    a += i.size
                    continue
                rd, wr = self.access(i)
                if rd & eax - dead:
                    return True
                dead = dead | (wr & eax)
                if i.mnemonic.startswith("ret"):
                    return True
                if i.mnemonic == "jmp":
                    if i.operands[0].type == X.X86_OP_IMM:
                        a = i.operands[0].imm
                        continue
                    return True
                if i.mnemonic.startswith("j") and i.operands[0].type == X.X86_OP_IMM:
                    work.append((i.operands[0].imm, dead, n))
                a += i.size
        return False

    def result_read(self, entry):
        """Whether a caller of the function reads what it leaves in eax: true unless every call of it
        is a direct one whose code after it writes eax before reading it -- and for one whose
        address is anywhere in the image as a word (a pointer to it, called who knows how)."""
        if not hasattr(self, "_words"):
            self._words = set()
            image = bytes(self.image)
            for k in range(0, len(image) - 3):
                w = image[k] | image[k + 1] << 8 | image[k + 2] << 16 | image[k + 3] << 24
                if 0x401000 <= w < 0x534000:
                    self._words.add(w)
        if entry in self._words:
            return True
        sites = self.calls_to().get(entry)
        if not sites:
            return True
        return any(self.eax_read_after(s) for s in sites)
