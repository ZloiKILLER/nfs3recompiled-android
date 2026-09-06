"""Compile upstream ADC/SBB emission and compare result/CF/OF with wide arithmetic.

Diagnostic only: reads the base commit, never regenerates or patches game code.
Requires Python capstone and the installed MSVC Build Tools on this Windows host.
Exit 1 means a semantic mismatch was reproduced, not a compiler failure.
"""
from pathlib import Path
import json
import os
import subprocess
import sys
import tempfile
import argparse

ROOT = Path(__file__).resolve().parents[1]
BASE = "77ebdb3b6a21925ccfc4fc4c1d712abec3e1945a"


def original(path):
    return subprocess.check_output(["git", "show", f"{BASE}:{path}"], cwd=ROOT, text=True)


def main():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--working-tree', action='store_true', help='Test the current generator instead of the baseline')
    args = parser.parse_args()
    output = ROOT / "diagnostics" / ("working-arithmetic" if args.working_tree else "upstream-arithmetic-expanded")
    output.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="nfs3-codegen-") as temp:
        package = Path(temp) / "audit_codegen"
        package.mkdir()
        (package / "__init__.py").write_text("")
        for name in ("arguments.py", "arithmetic.py"):
            path = "disasm/codegen/" + name
            (package / name).write_text((ROOT / path).read_text() if args.working_tree else original(path))
        sys.path.insert(0, temp)
        from audit_codegen import arithmetic

        decoder = Cs(CS_ARCH_X86, CS_MODE_32)
        decoder.detail = True
        functions = []
        for name, opcode in (("adc8", "10d0"), ("sbb8", "18d0"),
                             ("adc16", "6611d0"), ("sbb16", "6619d0"),
                             ("adc32", "11d0"), ("sbb32", "19d0")):
            instruction = next(decoder.disasm(bytes.fromhex(opcode), 0x1000))
            instruction.precious_flags = True
            body = getattr(arithmetic, "cg_" + name[:3])(instruction, (0, 0), {}, *instruction.operands)
            functions.append("void " + name + "(CPU& cpu) {\n" + "\n".join(body) + "\n}")

    source = r'''
#include <cstdint>
#include <cstdio>
#include <initializer_list>
namespace x86 { using reg8=uint8_t; using sreg8=int8_t; using reg16=uint16_t; using sreg16=int16_t; using reg32=uint32_t; using sreg32=int32_t; }
struct CPU {
    union { uint32_t eax; uint16_t ax; uint8_t al; };
    union { uint32_t edx; uint16_t dx; uint8_t dl; };
    struct { unsigned cf, of, zf, sf; } flags;
    void set_szp(uint8_t v) { flags.zf=!v; flags.sf=v>>7; }
    void set_szp(uint16_t v) { flags.zf=!v; flags.sf=v>>15; }
    void set_szp(uint32_t v) { flags.zf=!v; flags.sf=v>>31; }
};
''' + "\n".join(functions) + r'''
static unsigned tests=0, failures=0;
void check(unsigned bits, int sub, uint32_t a, uint32_t b, unsigned carry) {
        const uint64_t modulus=uint64_t(1)<<bits, mask=modulus-1, sign=modulus>>1;
        a=uint32_t(a & mask); b=uint32_t(b & mask);
        CPU cpu{}; cpu.eax=a; cpu.edx=b; cpu.flags.cf=carry;
        if(bits==8) { if(sub) sbb8(cpu); else adc8(cpu); }
        else if(bits==16) { if(sub) sbb16(cpu); else adc16(cpu); }
        else { if(sub) sbb32(cpu); else adc32(cpu); }
        const uint64_t rhs=uint64_t(b)+carry;
        const uint64_t wide=sub ? uint64_t(a)-rhs : uint64_t(a)+rhs;
        const unsigned cf=sub ? uint64_t(a)<rhs : wide>=modulus;
        const int64_t sa=(a & sign) ? int64_t(a)-int64_t(modulus) : a;
        const int64_t sb=(b & sign) ? int64_t(b)-int64_t(modulus) : b;
        const int64_t signedResult=sub ? sa-sb-carry : sa+sb+carry;
        const unsigned of=signedResult < -int64_t(sign) || signedResult >= int64_t(sign);
        const uint32_t result=uint32_t(wide & mask);
        ++tests;
        if(cpu.eax!=result || cpu.flags.cf!=cf || cpu.flags.of!=of || cpu.flags.zf!=(result==0) || cpu.flags.sf!=((result & sign)!=0)) {
            ++failures;
            if(failures<=32) std::printf("%s%u a=%08x b=%08x carry=%u got=%08x CF=%u OF=%u expected=%08x CF=%u OF=%u\n",
                sub?"SBB":"ADC",bits,a,b,carry,cpu.eax,cpu.flags.cf,cpu.flags.of,result,cf,of);
        }
}
int main() {
    for(int sub=0;sub<2;++sub) for(unsigned a=0;a<256;++a) for(unsigned b=0;b<256;++b) for(unsigned c=0;c<2;++c) check(8,sub,a,b,c);
    for(unsigned bits : {16u,32u}) {
        const uint64_t modulus=uint64_t(1)<<bits;
        const uint32_t values[]={0,1,uint32_t(modulus/2-2),uint32_t(modulus/2-1),uint32_t(modulus/2),uint32_t(modulus-2),uint32_t(modulus-1)};
        for(int sub=0;sub<2;++sub) for(auto a:values) for(auto b:values) for(unsigned c=0;c<2;++c) check(bits,sub,a,b,c);
        uint32_t seed=0x495f6c;
        for(unsigned i=0;i<100000;++i) {
            seed=seed*1664525u+1013904223u; const auto a=seed;
            seed=seed*1664525u+1013904223u; const auto b=seed;
            for(int sub=0;sub<2;++sub) for(unsigned c=0;c<2;++c) check(bits,sub,a,b,c);
        }
    }
    std::printf("SUMMARY tests=%u failures=%u\n",tests,failures);
    return failures ? 1 : 0;
}
'''
    (output / "probe.cpp").write_text(source)
    vcvars = Path(os.environ.get("ProgramFiles(x86)", r"C:\Program Files (x86)")) / "Microsoft Visual Studio/2022/BuildTools/VC/Auxiliary/Build/vcvars64.bat"
    if not vcvars.is_file():
        raise RuntimeError(f"MSVC environment not found: {vcvars}")
    batch = output / "compile.bat"
    batch.write_text('@echo off\ncall "' + str(vcvars) + '" >nul\n'
                     'if errorlevel 1 exit /b 2\n'
                     'cl /nologo /EHsc /std:c++17 /O2 probe.cpp /Fe:probe.exe\n')
    build = subprocess.run(["cmd.exe", "/d", "/c", str(batch)], cwd=output, text=True, capture_output=True)
    (output / "build.log").write_text(build.stdout + build.stderr)
    if build.returncode:
        raise RuntimeError(build.stdout + build.stderr)
    result = subprocess.run([str(output / "probe.exe")], cwd=output, text=True, capture_output=True)
    (output / "results.txt").write_text(result.stdout + result.stderr)
    (output / "metadata.json").write_text(json.dumps({
        "upstream_commit": BASE, "working_tree": args.working_tree,
        "scope": "8/16/32-bit ADC/SBB register destination, result/CF/OF/ZF/SF; exhaustive 8-bit and deterministic wider cases",
        "reference": "64-bit unsigned carry/borrow and signed range arithmetic",
        "game_run": False, "hang_causality": "not established", "exit_code": result.returncode,
    }, indent=2))
    print(result.stdout, end="")
    print("Evidence:", output)
    return result.returncode


if __name__ == "__main__":
    sys.exit(main())
