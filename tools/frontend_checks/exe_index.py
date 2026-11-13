"""nfs3.exe read once: its instructions in plain Python objects, kept in the build
directory, for what reads it statically (specs_from_exe.py, new_groups.py).

    import exe_index
    game = exe_index.load()

The disassembly of the whole game takes a minute and capstone's objects cannot
be kept, so what the tools ask of an instruction is copied out and pickled; the
pickle stands until nfs3.exe, the disasm package or the generator's tables
change.  The functions are the generator's own (disassembler.disassemble, with
disassemble_nfs3hp.py's tables), so an address here is a sub_xxx there.
"""
import contextlib
import hashlib
import io
import pickle
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
BUILD = Path(__file__).resolve().parent / "build"
CACHE = BUILD / "exe_index.pickle"

IMM, REG, MEM = "imm", "reg", "mem"


class Operand:
    __slots__ = ("type", "reg", "imm", "base", "index", "scale", "disp", "segment", "size")

    def __init__(self, type, reg=None, imm=0, base=None, index=None, scale=1, disp=0, segment=None, size=4):
        self.type, self.reg, self.imm = type, reg, imm
        self.base, self.index, self.scale, self.disp, self.segment, self.size = base, index, scale, disp, segment, size


class Insn:
    __slots__ = ("address", "size", "raw", "mnemonic", "text", "operands", "reads", "writes", "destinations")

    def __init__(self, address, size, raw, mnemonic, text, operands, reads, writes, destinations):
        self.address, self.size, self.raw, self.mnemonic, self.text = address, size, raw, mnemonic, text
        self.operands, self.reads, self.writes, self.destinations = operands, reads, writes, destinations

    def __repr__(self):
        return "%08x  %-22s %s %s" % (self.address, self.raw.hex(), self.mnemonic, self.text)


class Function:
    __slots__ = ("entry", "start", "end", "insns", "skipped", "data_before")

    def __init__(self, entry, start, end, insns, skipped, data_before):
        self.entry, self.start, self.end, self.insns = entry, start, end, insns
        self.skipped, self.data_before = skipped, data_before


class Game:
    """functions: {entry: Function}; at: {address: Insn}; imports: {IAT slot: name};
    params: {imported name: its stack words}; x87: {entry: (needs, net)}."""

    def __init__(self):
        self.functions = {}
        self.at = {}
        self.imports = {}
        self.params = {}
        self.x87 = {}
        self.sections = []  # (start, bytes, writable, code)

    def dword(self, address):
        for start, data, _, _ in self.sections:
            if start <= address and address + 4 <= start + len(data):
                return int.from_bytes(data[address - start:address - start + 4], "little")
        return None

    def string_at(self, address, shortest=3, longest=200):
        """The printable zero-ended string at an address in the image's data, or None."""
        for start, data, _, code in self.sections:
            if code or not (start <= address < start + len(data)):
                continue
            at = address - start
            end = data.find(b"\0", at, at + longest + 1)
            if end < 0 or end - at < shortest:
                return None
            text = data[at:end]
            if all(32 <= c < 127 or c in (9, 10, 13) for c in text):
                # Most of it letters, or any number looks like a string.
                if sum(chr(c).isalnum() or c in b" ._-%\\/:" for c in text) * 4 >= len(text) * 3:
                    return text.decode("ascii")
            return None
        return None

    def is_code(self, address):
        return any(code and start <= address < start + len(data) for start, data, _, code in self.sections)


def _light(instruction):
    """A capstone instruction as an Insn; the generator's split jumps as a plain jmp."""
    from capstone import x86
    raw = bytes(instruction.bytes) if hasattr(instruction, "bytes") else b""
    if not hasattr(instruction, "op_str"):  # disasm's FakeJmpInstruction, FakeRetInstruction
        operands = [Operand(IMM, imm=op.imm) for op in instruction.operands]
        return Insn(instruction.address, instruction.size, raw, instruction.mnemonic, "", operands, (), (), ())
    operands = []
    for op in instruction.operands:
        if op.type == x86.X86_OP_IMM:
            operands.append(Operand(IMM, imm=op.imm & 0xffffffff, size=op.size))
        elif op.type == x86.X86_OP_REG:
            operands.append(Operand(REG, reg=instruction.reg_name(op.reg), size=op.size))
        elif op.type == x86.X86_OP_MEM:
            operands.append(Operand(MEM, base=instruction.reg_name(op.mem.base) if op.mem.base else None,
                                    index=instruction.reg_name(op.mem.index) if op.mem.index else None,
                                    scale=op.mem.scale, disp=op.mem.disp,
                                    segment=instruction.reg_name(op.mem.segment) if op.mem.segment else None,
                                    size=op.size))
    read, written = instruction.regs_access()
    return Insn(instruction.address, instruction.size, raw, instruction.mnemonic, instruction.op_str, operands,
                tuple(instruction.reg_name(r) for r in read), tuple(instruction.reg_name(r) for r in written),
                tuple(getattr(instruction, "potential_destinations", ())))


def _params():
    """The Windows functions' stack words, from the port's declarations of them: each
    takes (app, cpu, ...) and is stdcall, so its ret pops them."""
    import re
    params = {}
    for header in (ROOT / "include/winapi").rglob("*.h"):
        text = header.read_text(errors="replace")
        for m in re.finditer(r"\b(\w+)\s*\(\s*WinApplication\s*\*\s*\w*\s*,\s*x86::CPU\s*&\s*\w*\s*([^)]*)\)", text):
            rest = m.group(2).strip().strip(",").strip()
            params[m.group(1)] = len([p for p in rest.split(",") if p.strip()]) if rest else 0
    return params


def _key():
    h = hashlib.sha1()
    for f in [ROOT / "nfs3hp/nfs3.exe", ROOT / "disassemble_nfs3hp.py", Path(__file__)] + sorted((ROOT / "disasm").rglob("*.py")):
        h.update(f.read_bytes())
    return h.hexdigest()


def build():
    sys.path.insert(0, str(ROOT))
    sys.path.insert(0, str(ROOT / "disasm"))
    from disasm import disassembler
    from disasm.codegen import fpu_frame
    import disassemble_nfs3hp as nfs3hp
    game = Game()
    with contextlib.redirect_stdout(io.StringIO()):
        module = disassembler.disassemble("nfs3hp", str(ROOT / "nfs3hp/nfs3.exe"), nfs3hp.DATA_SEGMENTS,
                                          nfs3hp.KNOWN_SUBROUTINES, nfs3hp.SPLIT_INSTRUCTIONS,
                                          nfs3hp.THREAD_ROUTINES, nfs3hp.THREAD_SEGMENTS, nfs3hp.MERGE_ROUTINES)
        _, effects = fpu_frame.analyse(module, nfs3hp.SKIP_INSTRUCTIONS)
    game.x87 = {entry: effect for entry, effect in effects.items() if isinstance(effect, tuple)}
    light = {}
    for subroutine in module.subroutines:
        insns = []
        for instruction in subroutine.instructions:
            key = id(instruction)
            if key not in light:
                light[key] = _light(instruction)
            insns.append(light[key])
            game.at.setdefault(insns[-1].address, insns[-1])
        entry = subroutine.get_entry_point()
        game.functions[entry] = Function(entry, subroutine.get_start_address(), subroutine.get_end_address(), insns,
                                         bytes(subroutine.skipped_blob or b""), bytes(subroutine.data_blob or b""))
    for data, start, length, name, read_only, code in module.sections:
        if data:
            game.sections.append((start, bytes(data), not read_only, code))
    # The generator's IAT holds made-up addresses, one a function (disasm/module.py).
    fake = {address: name.decode() if isinstance(name, bytes) else str(name)
            for _, methods in module.dll_imports for name, address in methods}
    for start, data, _, _ in game.sections:
        for at in range(0, len(data) - 3, 4):
            value = int.from_bytes(data[at:at + 4], "little")
            if value in fake:
                game.imports[start + at] = fake[value]
    game.params = _params()
    return game


def load():
    key = _key()
    if CACHE.exists():
        try:
            with CACHE.open("rb") as f:
                stored, game = pickle.load(f)
            if stored == key:
                return game
        except (pickle.PickleError, EOFError, AttributeError, ImportError):
            pass
    game = build()
    BUILD.mkdir(parents=True, exist_ok=True)
    temporary = CACHE.with_suffix(".tmp")
    with temporary.open("wb") as f:
        pickle.dump((key, game), f, protocol=4)
    temporary.replace(CACHE)
    return game


if __name__ == "__main__":
    game = load()
    print(len(game.functions), "functions", len(game.at), "instructions", len(game.imports), "imports")
