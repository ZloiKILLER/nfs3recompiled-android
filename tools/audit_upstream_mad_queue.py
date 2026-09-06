"""Run unchanged upstream MAD queue/time helpers without SDL or Android.

Uses upstream CPU flags and MemoryAccessor; clocks and frame presentation
are controlled substitutes. This tests local helper semantics, not the game.
"""
import subprocess
import os
import re
from audit_upstream_arithmetic import ROOT, original
from pathlib import Path


def main():
    out = ROOT / 'diagnostics' / 'upstream-mad-queue'
    out.mkdir(parents=True, exist_ok=True)
    names = ['sub_495890', 'sub_4958c0', 'sub_495920', 'sub_495a10', 'sub_495a50']
    generated = original('src/nfs3hp/disassembly/nfs3hp.21.cpp')
    functions = []
    for name in names:
        start = generated.index('void Application::' + name + '(')
        end = generated.index('\n}\n', start) + 3
        functions.append(generated[start:end])
    cpu = original('include/cpu.h')
    cpu = re.sub(r'^#include.*\n', '', cpu, flags=re.M)
    header = original('include/lib/winapp.h')
    start = header.index('template< typename T >\nstruct MemoryAccessor')
    accessor = header[start:header.index('\ntemplate<>', start)]
    source = r'''
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <set>
#define NFS2_USE(x) (void)(x)
#define NFS2_ASSERT(x) do { if(!(x)) std::abort(); } while(0)
namespace x86 {
using reg8=uint8_t; using reg16=uint16_t; using reg32=uint32_t; using reg64=uint64_t;
using sreg8=int8_t; using sreg16=int16_t; using sreg32=int32_t; using sreg64=int64_t;
struct FPU { void init() {} }; struct MMX {};
}
''' + cpu + '\nnamespace win32 { class WinApplication;\n' + accessor + r'''
class WinApplication {
    std::vector<uint8_t> bytes=std::vector<uint8_t>(0xb00000);
public:
    uint32_t now=1000, audioMs=0;
    std::vector<uint32_t> presented;
    template<class T> MemoryAccessor<T> getMemory(uint32_t address) {
        NFS2_ASSERT(uint64_t(address)+sizeof(T)<=bytes.size());
        return MemoryAccessor<T>{bytes.data()+address};
    }
};
}
using win32::WinApplication;
class Application { public:
''' + ''.join('static void '+n+'(WinApplication*, x86::CPU&);\n' for n in names) + r'''
static void sub_4df2b0(WinApplication* app,x86::CPU& cpu) { app->presented.push_back(cpu.eax); cpu.esp+=4; }
static void sub_4f2790(WinApplication* app,x86::CPU& cpu) { cpu.eax=app->now; cpu.esp+=4; }
static void sub_4f2f50(WinApplication* app,x86::CPU& cpu) {
    app->getMemory<uint32_t>(cpu.edx)=2;
    app->getMemory<uint32_t>(cpu.edx+4)=app->audioMs;
    app->getMemory<uint32_t>(cpu.edx+8)=0;
    app->getMemory<uint32_t>(cpu.edx+12)=0;
    cpu.eax=0; cpu.esp+=4;
}
};
''' + '\n'.join(functions) + r'''
unsigned checks=0;
void expect(bool ok,const char* name) { ++checks; if(!ok) { std::printf("FAIL %s\n",name); std::exit(1); } }
void call(void(*fn)(WinApplication*,x86::CPU&),WinApplication& app,x86::CPU& cpu) {
    auto sp=cpu.esp; cpu.esp-=4; fn(&app,cpu); expect(cpu.esp==sp,"stack balance");
}
int main() {
    WinApplication app; x86::CPU cpu{}; cpu.esp=0x900000;
    app.getMemory<uint32_t>(0x79f358)=6;
    for(unsigned i=0;i<6;++i) app.getMemory<uint32_t>(0x79f294+8*i)=0x100000+i*0x1000;
    const uint32_t timestamps[]={100,33,66,0,133,99};
    for(auto t:timestamps) {
        call(Application::sub_495890,app,cpu); expect(cpu.eax!=0,"free slot before full");
        cpu.eax=t; call(Application::sub_4958c0,app,cpu);
    }
    call(Application::sub_495890,app,cpu); expect(cpu.eax==0,"full queue has no free slot");
    const int times[]={-1,0,32,33,100,133}; const unsigned counts[]={6,5,5,4,1,0};
    for(unsigned i=0;i<6;++i) {
        cpu.eax=uint32_t(times[i]); call(Application::sub_495920,app,cpu);
        expect(uint32_t(app.getMemory<uint32_t>(0x79f354))==counts[i],"due frames recycled");
        std::set<uint32_t> pointers;
        for(unsigned j=0;j<6;++j) pointers.insert(app.getMemory<uint32_t>(0x79f294+8*j));
        expect(pointers.size()==6,"buffer ownership preserved");
    }
    cpu.eax=uint32_t(-1); call(Application::sub_495a10,app,cpu);
    for(unsigned delta=0;delta<=1000;delta+=10) {
        app.now=1000+delta; call(Application::sub_495a50,app,cpu);
        expect(cpu.eax==delta,"wall-clock progress without audio");
    }
    // Model a normal full queue, with the next frame scheduled 33ms ahead.
    // 500 clock polls at the same millisecond are enough to fire HANGDIAG,
    // although the queue makes progress as soon as its frame becomes due.
    app.getMemory<uint32_t>(0x79f354)=6;
    for(unsigned j=0;j<6;++j) app.getMemory<uint32_t>(0x79f290+8*j)=33+33*j;
    app.now=1000; cpu.eax=uint32_t(-1); call(Application::sub_495a10,app,cpu);
    for(unsigned j=0;j<500;++j) {
        call(Application::sub_495a50,app,cpu); call(Application::sub_495920,app,cpu);
    }
    expect(uint32_t(app.getMemory<uint32_t>(0x79f354))==6,"normal wait before due time");
    app.now=1033; call(Application::sub_495a50,app,cpu); call(Application::sub_495920,app,cpu);
    expect(uint32_t(app.getMemory<uint32_t>(0x79f354))==5,"normal wait completes at due time");
    std::printf("PASS %u checks; queue ordering/recycling and wall clock; 500 polls can be a normal wait\n",checks);
}
'''
    (out / 'probe.cpp').write_text(source)
    vcvars = Path(os.environ.get('ProgramFiles(x86)', r'C:\Program Files (x86)')) / 'Microsoft Visual Studio/2022/BuildTools/VC/Auxiliary/Build/vcvars64.bat'
    batch = out / 'compile.bat'
    batch.write_text('@echo off\ncall "'+str(vcvars)+'" >nul\nif errorlevel 1 exit /b 2\ncl /nologo /EHsc /std:c++17 /O2 probe.cpp /Fe:probe.exe\n')
    build = subprocess.run(['cmd.exe','/d','/c',str(batch)],cwd=out,text=True,capture_output=True)
    (out/'build.log').write_text(build.stdout+build.stderr)
    if build.returncode: raise RuntimeError(build.stdout+build.stderr)
    run = subprocess.run([str(out/'probe.exe')],cwd=out,text=True,capture_output=True,timeout=15)
    (out/'results.txt').write_text(run.stdout+run.stderr)
    print(run.stdout,end='')
    return run.returncode


if __name__=='__main__':
    raise SystemExit(main())
