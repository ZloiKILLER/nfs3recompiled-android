#include "port_conv.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>

/* The functions tools/decomp.py wrote (port_conv.h says how). */

#if defined(_MSC_VER)
#pragma fp_contract(off)
#elif defined(__clang__)
#pragma STDC FP_CONTRACT OFF
#endif

namespace nfs3hp
{
namespace port
{
namespace conv
{

namespace
{

/* fistp as the code does it without __CHP: `value` rounded the way the x87's control word says. */
int32_t stored(CPU& cpu, double value)
{
    return fp::toInteger<int32_t>(x86::Float(value));
}

/* A tiny x87 stack of doubles: the functions move floats through the x87 (a store rounds to float). */
struct X87
{
    double v[8];
    int n = 0;
    void push(double d) { v[n++] = d; }
    double pop() { return v[--n]; }
    double& st(int i) { return v[n - 1 - i]; }
    void fxch(int i) { std::swap(st(0), st(i)); }
    void fst_st(int i) { st(i) = st(0); }
    void fstp_st(int i)
    {
        const double t = st(0);
        --n;
        st(i - 1) = t;
    }
};

/* The x87's sums, differences, products and quotients as it rounds them: to
 * the precision its control word asks for -- a race's single precision, the
 * menus' extended -- by the emulated FPU's own operations (include/fpu.h). */
double xadd(CPU& cpu, double a, double b) { return double(fp::add(x86::Float(a), x86::Float(b))); }
double xsub(CPU& cpu, double a, double b) { return double(fp::sub(x86::Float(a), x86::Float(b))); }
double xmul(CPU& cpu, double a, double b) { return double(fp::mul(x86::Float(a), x86::Float(b))); }
double xdiv(CPU& cpu, double a, double b) { return double(fp::div(x86::Float(a), x86::Float(b))); }

/* The flags the last instruction left, for the conditional jump after it. */
struct Flags
{
    enum Kind { Logic, Sub, Arith, Fpu } k = Logic;
    reg32 fa = 0, fb = 0, fr = 0;
    bool c = false, lt = false, eq = false, un = false;
    void sub(reg32 x, reg32 y) { k = Sub; fa = x; fb = y; fr = x - y; }
    void add(reg32 x, reg32 y) { k = Arith; fr = x + y; c = fr < x; }
    void arith(reg32 v) { k = Arith; fr = v; }
    void logic(reg32 v) { k = Logic; fr = v; }
    void fcmp(double x, double y) { k = Fpu; lt = x < y; eq = x == y; un = !(lt || eq || x > y); }
    bool e() const { return k == Fpu ? (eq || un) : k == Sub ? fa == fb : fr == 0; }
    bool ne() const { return !e(); }
    bool l() const { return k == Sub ? int32_t(fa) < int32_t(fb) : int32_t(fr) < 0; }
    bool le() const { return k == Sub ? int32_t(fa) <= int32_t(fb) : (fr == 0 || int32_t(fr) < 0); }
    bool g() const { return !le(); }
    bool ge() const { return !l(); }
    bool b() const { return k == Fpu ? (lt || un) : k == Sub ? fa < fb : k == Arith ? c : false; }
    bool be() const { return k == Fpu ? (lt || eq || un) : k == Sub ? fa <= fb : k == Arith ? (c || fr == 0) : fr == 0; }
    bool a() const { return !be(); }
    bool ae() const { return !b(); }
    bool s() const { return k == Sub ? int32_t(fa - fb) < 0 : int32_t(fr) < 0; }
    bool ns() const { return !s(); }
    /* fnstsw ax after a compare: C0, C2 and C3. */
    reg32 status() const { return un ? 0x4500u : lt ? 0x100u : eq ? 0x4000u : 0u; }
};

/* A call from such a function: the arguments already on the guest's stack at `esp`, the registers in and out. */
bool rawCall(App* app, CPU& cpu, reg32 target, reg32 esp, int words, reg32& eax, reg32& edx, reg32& ebx, reg32& ecx)
{
    (void)words;
    cpu.eax = eax;
    cpu.edx = edx;
    cpu.ebx = ebx;
    cpu.ecx = ecx;
    cpu.esp = esp - 4;
    app->getMemory<reg32>(esp - 4) = 0;
    app->dynamic_call(target, cpu);
    if (cpu.terminate)
        return false;
    cpu.esp = esp;
    eax = cpu.eax;
    edx = cpu.edx;
    ebx = cpu.ebx;
    ecx = cpu.ecx;
    return true;
}

/* What a callee left on the x87's stack (a float returned) taken into the function's own. */
void pullX87(CPU& cpu, int before, X87& x)
{
    const int k = cpu.fpu.count - before;
    if (k <= 0)
        return;
    double v[8];
    for (int i = 0; i < k && i < 8; ++i)
    {
        v[i] = double(cpu.fpu.st(0));
        cpu.fpu.pop();
    }
    for (int i = std::min(k, 8) - 1; i >= 0; --i)
        x.push(v[i]);
}

bool core49fe70(App* app, CPU& cpu, const Entry& e, reg32& result, reg32& popped_)
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
    const auto SD = [&](reg32 a, double v) { app->getMemory<double>(a) = v; };
    esp -= 4; wr32(esp, ebx);  // 49fe70 push ebx
    esp -= 4; wr32(esp, edx);  // 49fe71 push edx
    esp -= 4; wr32(esp, ebp);  // 49fe72 push ebp
    ebp = esp;  // 49fe73 mov ebp, esp
    esp -= 12u;  // 49fe75 sub esp, 0xc
    edx = eax;  // 49fe78 mov edx, eax
    x.push(F(eax + 0x98u));  // 49fe7a fld dword ptr [eax + 0x98]
    x.st(0) = xmul(cpu, x.st(0), D(0x53c7a4u));  // 49fe80 fmul qword ptr [0x53c7a4]
    esp -= 4u;  // 49fe86 sub esp, 4
    wr32((esp), stored(cpu, x.pop()));  // 49fe89 fistp dword ptr [esp]
    eax = rd32(esp); esp += 4;  // 49fe8c pop eax
    wr32((ebp - 0xcu), eax);  // 49fe8d mov dword ptr [ebp - 0xc], eax
    x.push(F(edx + 0x9cu));  // 49fe90 fld dword ptr [edx + 0x9c]
    x.st(0) = xmul(cpu, x.st(0), D(0x53c7a4u));  // 49fe96 fmul qword ptr [0x53c7a4]
    esp -= 4u;  // 49fe9c sub esp, 4
    wr32((esp), stored(cpu, x.pop()));  // 49fe9f fistp dword ptr [esp]
    eax = rd32(esp); esp += 4;  // 49fea2 pop eax
    wr32((ebp - 8u), eax);  // 49fea3 mov dword ptr [ebp - 8], eax
    x.push(F(edx + 0xa0u));  // 49fea6 fld dword ptr [edx + 0xa0]
    x.st(0) = xmul(cpu, x.st(0), D(0x53c7a4u));  // 49feac fmul qword ptr [0x53c7a4]
    esp -= 4u;  // 49feb2 sub esp, 4
    wr32((esp), stored(cpu, x.pop()));  // 49feb5 fistp dword ptr [esp]
    eax = rd32(esp); esp += 4;  // 49feb8 pop eax
    wr32((ebp - 4u), eax);  // 49feb9 mov dword ptr [ebp - 4], eax
    eax = 0u;  // 49febc xor eax, eax
    fl.logic(0u);  // 49febc xor eax, eax
    ebx = 1u;  // 49febe mov ebx, 1
    eax = (eax & 0xffffff00u) | ((rd8((edx + 0x88u))) & 0xffu);  // 49fec3 mov al, byte ptr [edx + 0x88]
    { const reg32 a_ = edx, b_ = 12u; edx = a_ + b_; fl.add(a_, b_); }  // 49fec9 add edx, 0xc
    { const reg32 a_ = ebx, b_ = eax; ebx = a_ - b_; fl.sub(a_, b_); }  // 49fecc sub ebx, eax
    eax = (ebp - 0xcu);  // 49fece lea eax, [ebp - 0xc]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x41f6e0, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 49fed1 call 0x41f6e0
    esp += 0u;  // 49fed1 call 0x41f6e0
    esp = ebp;  // 49fed6 mov esp, ebp
    ebp = rd32(esp); esp += 4;  // 49fed8 pop ebp
    edx = rd32(esp); esp += 4;  // 49fed9 pop edx
    ebx = rd32(esp); esp += 4;  // 49feda pop ebx
    for (int i_ = 0; i_ < x.n; ++i_) { cpu.fpu.count += 1; cpu.fpu.st(0) = x86::Float(x.v[i_]); }  // 49fedb ret
    result = eax; popped_ = 0u; return true;  // 49fedb ret
    return false;
}


bool core4a00d0(App* app, CPU& cpu, const Entry& e, reg32& result, reg32& popped_)
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
    const auto SD = [&](reg32 a, double v) { app->getMemory<double>(a) = v; };
    esp -= 4; wr32(esp, ebx);  // 4a00d0 push ebx
    esp -= 4; wr32(esp, ecx);  // 4a00d1 push ecx
    esp -= 4; wr32(esp, edx);  // 4a00d2 push edx
    esp -= 4; wr32(esp, esi);  // 4a00d3 push esi
    esp -= 4; wr32(esp, edi);  // 4a00d4 push edi
    esp -= 4; wr32(esp, ebp);  // 4a00d5 push ebp
    ebp = esp;  // 4a00d6 mov ebp, esp
    esp -= 16u;  // 4a00d8 sub esp, 0x10
    edi = (ebp - 0x10u);  // 4a00db lea edi, [ebp - 0x10]
    esi = (eax + 0x130u);  // 4a00de lea esi, [eax + 0x130]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 4a00e4 movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 4a00e5 movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 4a00e6 movsd dword ptr es:[edi], dword ptr [esi]
    fl.sub(rd32((ebp - 0xcu)), 1036831949u);  // 4a00e7 cmp dword ptr [ebp - 0xc], 0x3dcccccd
    if (fl.le()) goto L_4a0129;  // 4a00ee jle 0x4a0129
    ecx = rd32((eax + 0xa0u));  // 4a00f0 mov ecx, dword ptr [eax + 0xa0]
    esp -= 4; wr32(esp, ecx);  // 4a00f6 push ecx
    ebx = rd32((eax + 0x9cu));  // 4a00f7 mov ebx, dword ptr [eax + 0x9c]
    esp -= 4; wr32(esp, ebx);  // 4a00fd push ebx
    esi = rd32((eax + 0x98u));  // 4a00fe mov esi, dword ptr [eax + 0x98]
    esp -= 4; wr32(esp, esi);  // 4a0104 push esi
    edi = rd32((eax + 0x150u));  // 4a0105 mov edi, dword ptr [eax + 0x150]
    esp -= 4; wr32(esp, edi);  // 4a010b push edi
    edx = rd32((eax + 0x14cu));  // 4a010c mov edx, dword ptr [eax + 0x14c]
    esp -= 4; wr32(esp, edx);  // 4a0112 push edx
    ecx = rd32((eax + 0x148u));  // 4a0113 mov ecx, dword ptr [eax + 0x148]
    esp -= 4; wr32(esp, ecx);  // 4a0119 push ecx
    { const reg32 a_ = eax, b_ = 292u; eax = a_ + b_; fl.add(a_, b_); }  // 4a011a add eax, 0x124
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4968a0, esp, 6, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a011f call 0x4968a0
    esp += 24u;  // 4a011f call 0x4968a0
    SF((ebp - 4u), x.st(0));  // 4a0124 fstp dword ptr [ebp - 4]
    x.pop();  // 4a0124 fstp dword ptr [ebp - 4]
    goto L_4a0130;  // 4a0127 jmp 0x4a0130
L_4a0129:
    wr32((ebp - 4u), 3338272768u);  // 4a0129 mov dword ptr [ebp - 4], 0xc6fa0000
L_4a0130:
    x.push(F(ebp - 4u));  // 4a0130 fld dword ptr [ebp - 4]
    esp = ebp;  // 4a0133 mov esp, ebp
    ebp = rd32(esp); esp += 4;  // 4a0135 pop ebp
    edi = rd32(esp); esp += 4;  // 4a0136 pop edi
    esi = rd32(esp); esp += 4;  // 4a0137 pop esi
    edx = rd32(esp); esp += 4;  // 4a0138 pop edx
    ecx = rd32(esp); esp += 4;  // 4a0139 pop ecx
    ebx = rd32(esp); esp += 4;  // 4a013a pop ebx
    for (int i_ = 0; i_ < x.n; ++i_) { cpu.fpu.count += 1; cpu.fpu.st(0) = x86::Float(x.v[i_]); }  // 4a013b ret
    result = eax; popped_ = 0u; return true;  // 4a013b ret
    return false;
}


bool core4a0440(App* app, CPU& cpu, const Entry& e, reg32& result, reg32& popped_)
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
    const auto SD = [&](reg32 a, double v) { app->getMemory<double>(a) = v; };
    esp -= 4; wr32(esp, ebx);  // 4a0440 push ebx
    esp -= 4; wr32(esp, ecx);  // 4a0441 push ecx
    esp -= 4; wr32(esp, edx);  // 4a0442 push edx
    esp -= 4; wr32(esp, esi);  // 4a0443 push esi
    esp -= 4; wr32(esp, edi);  // 4a0444 push edi
    esp -= 4; wr32(esp, ebp);  // 4a0445 push ebp
    ebp = esp;  // 4a0446 mov ebp, esp
    esp -= 64u;  // 4a0448 sub esp, 0x40
    ecx = eax;  // 4a044b mov ecx, eax
    edi = (ebp - 0x40u);  // 4a044d lea edi, [ebp - 0x40]
    esi = 4849104u;  // 4a0450 mov esi, 0x49fdd0
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 4a0455 movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 4a0456 movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 4a0457 movsd dword ptr es:[edi], dword ptr [esi]
    fl.sub(rd32((eax + 0x184u)), 0u);  // 4a0458 cmp dword ptr [eax + 0x184], 0
    if (fl.e()) goto L_4a046d;  // 4a045f je 0x4a046d
    wr32((ebp - 8u), 1120403456u);  // 4a0461 mov dword ptr [ebp - 8], 0x42c80000
    goto L_4a0569;  // 4a0468 jmp 0x4a0569
L_4a046d:
    edi = (ebp - 0x28u);  // 4a046d lea edi, [ebp - 0x28]
    edx = (eax + 0xc0u);  // 4a0470 lea edx, [eax + 0xc0]
    esi = (ecx + 0x130u);  // 4a0476 lea esi, [ecx + 0x130]
    eax = (ebp - 0x28u);  // 4a047c lea eax, [ebp - 0x28]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 4a047f movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 4a0480 movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 4a0481 movsd dword ptr es:[edi], dword ptr [esi]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4e01f0, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a0482 call 0x4e01f0
    esp += 0u;  // 4a0482 call 0x4e01f0
    x.st(0) = xmul(cpu, x.st(0), F(ecx + 0x114u));  // 4a0487 fmul dword ptr [ecx + 0x114]
    eax = (ebp - 0x28u);  // 4a048d lea eax, [ebp - 0x28]
    edx = (ecx + 0xccu);  // 4a0490 lea edx, [ecx + 0xcc]
    SF((ebp - 0x34u), x.st(0));  // 4a0496 fstp dword ptr [ebp - 0x34]
    x.pop();  // 4a0496 fstp dword ptr [ebp - 0x34]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4e01f0, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a0499 call 0x4e01f0
    esp += 0u;  // 4a0499 call 0x4e01f0
    x.st(0) = xmul(cpu, x.st(0), F(ecx + 0x118u));  // 4a049e fmul dword ptr [ecx + 0x118]
    eax = (ebp - 0x28u);  // 4a04a4 lea eax, [ebp - 0x28]
    edx = (ecx + 0xd8u);  // 4a04a7 lea edx, [ecx + 0xd8]
    SF((ebp - 0x30u), x.st(0));  // 4a04ad fstp dword ptr [ebp - 0x30]
    x.pop();  // 4a04ad fstp dword ptr [ebp - 0x30]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4e01f0, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a04b0 call 0x4e01f0
    esp += 0u;  // 4a04b0 call 0x4e01f0
    x.st(0) = xmul(cpu, x.st(0), F(ecx + 0x11cu));  // 4a04b5 fmul dword ptr [ecx + 0x11c]
    x.push(0.0);  // 4a04bb fldz
    x.fxch(1);  // 4a04bd fxch st(1)
    SF((ebp - 0x2cu), x.st(0));  // 4a04bf fstp dword ptr [ebp - 0x2c]
    x.pop();  // 4a04bf fstp dword ptr [ebp - 0x2c]
    fl.fcmp(x.st(0), F(ebp - 0x34u));  // 4a04c2 fcomp dword ptr [ebp - 0x34]
    x.pop();  // 4a04c2 fcomp dword ptr [ebp - 0x34]
    eax = (eax & 0xffff0000u) | fl.status();  // 4a04c5 fnstsw ax
    // sahf  // 4a04c7 sahf
    if (fl.be()) goto L_4a04d1;  // 4a04c8 jbe 0x4a04d1
    edi = 1u;  // 4a04ca mov edi, 1
    goto L_4a04d6;  // 4a04cf jmp 0x4a04d6
L_4a04d1:
    edi = 4294967295u;  // 4a04d1 mov edi, 0xffffffff
L_4a04d6:
    x.push(0.0);  // 4a04d6 fldz
    fl.fcmp(x.st(0), F(ebp - 0x30u));  // 4a04d8 fcomp dword ptr [ebp - 0x30]
    x.pop();  // 4a04d8 fcomp dword ptr [ebp - 0x30]
    eax = (eax & 0xffff0000u) | fl.status();  // 4a04db fnstsw ax
    // sahf  // 4a04dd sahf
    if (fl.be()) goto L_4a04e9;  // 4a04de jbe 0x4a04e9
    wr32((ebp - 0xcu), 1u);  // 4a04e0 mov dword ptr [ebp - 0xc], 1
    goto L_4a04f0;  // 4a04e7 jmp 0x4a04f0
L_4a04e9:
    wr32((ebp - 0xcu), 4294967295u);  // 4a04e9 mov dword ptr [ebp - 0xc], 0xffffffff
L_4a04f0:
    x.push(0.0);  // 4a04f0 fldz
    fl.fcmp(x.st(0), F(ebp - 0x2cu));  // 4a04f2 fcomp dword ptr [ebp - 0x2c]
    x.pop();  // 4a04f2 fcomp dword ptr [ebp - 0x2c]
    eax = (eax & 0xffff0000u) | fl.status();  // 4a04f5 fnstsw ax
    // sahf  // 4a04f7 sahf
    if (fl.be()) goto L_4a0501;  // 4a04f8 jbe 0x4a0501
    esi = 1u;  // 4a04fa mov esi, 1
    goto L_4a0506;  // 4a04ff jmp 0x4a0506
L_4a0501:
    esi = 4294967295u;  // 4a0501 mov esi, 0xffffffff
L_4a0506:
    eax = rd32((ecx + 0x98u));  // 4a0506 mov eax, dword ptr [ecx + 0x98]
    wr32((ebp - 0x1cu), eax);  // 4a050c mov dword ptr [ebp - 0x1c], eax
    eax = rd32((ecx + 0x9cu));  // 4a050f mov eax, dword ptr [ecx + 0x9c]
    edx = (ebp - 0x1cu);  // 4a0515 lea edx, [ebp - 0x1c]
    wr32((ebp - 0x18u), eax);  // 4a0518 mov dword ptr [ebp - 0x18], eax
    ebx = (ecx + 0x148u);  // 4a051b lea ebx, [ecx + 0x148]
    eax = rd32((ecx + 0xa0u));  // 4a0521 mov eax, dword ptr [ecx + 0xa0]
    ecx = (ebp - 0x40u);  // 4a0527 lea ecx, [ebp - 0x40]
    wr32((ebp - 0x14u), eax);  // 4a052a mov dword ptr [ebp - 0x14], eax
    eax = 1u;  // 4a052d mov eax, 1
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4e0000, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a0532 call 0x4e0000
    esp += 0u;  // 4a0532 call 0x4e0000
    edx = (ebp - 0x40u);  // 4a0537 lea edx, [ebp - 0x40]
    eax = (ebp - 0x28u);  // 4a053a lea eax, [ebp - 0x28]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4e01f0, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a053d call 0x4e01f0
    esp += 0u;  // 4a053d call 0x4e01f0
    eax = rd32((ebp - 0xcu));  // 4a0542 mov eax, dword ptr [ebp - 0xc]
    wr32((ebp - 4u), eax);  // 4a0545 mov dword ptr [ebp - 4], eax
    x.push(double(int32_t(rd32((ebp - 4u)))));  // 4a0548 fild dword ptr [ebp - 4]
    x.st(0) = xmul(cpu, x.st(0), F(ebp - 0x30u));  // 4a054b fmul dword ptr [ebp - 0x30]
    wr32((ebp - 4u), edi);  // 4a054e mov dword ptr [ebp - 4], edi
    x.push(double(int32_t(rd32((ebp - 4u)))));  // 4a0551 fild dword ptr [ebp - 4]
    x.st(0) = xmul(cpu, x.st(0), F(ebp - 0x34u));  // 4a0554 fmul dword ptr [ebp - 0x34]
    x.st(1) = xadd(cpu, x.st(1), x.st(0));  // 4a0557 faddp st(1)
    x.pop();  // 4a0557 faddp st(1)
    wr32((ebp - 4u), esi);  // 4a0559 mov dword ptr [ebp - 4], esi
    x.push(double(int32_t(rd32((ebp - 4u)))));  // 4a055c fild dword ptr [ebp - 4]
    x.st(0) = xmul(cpu, x.st(0), F(ebp - 0x2cu));  // 4a055f fmul dword ptr [ebp - 0x2c]
    x.st(1) = xadd(cpu, x.st(1), x.st(0));  // 4a0562 faddp st(1)
    x.pop();  // 4a0562 faddp st(1)
    x.st(1) = xadd(cpu, x.st(1), x.st(0));  // 4a0564 faddp st(1)
    x.pop();  // 4a0564 faddp st(1)
    SF((ebp - 8u), x.st(0));  // 4a0566 fstp dword ptr [ebp - 8]
    x.pop();  // 4a0566 fstp dword ptr [ebp - 8]
L_4a0569:
    x.push(F(ebp - 8u));  // 4a0569 fld dword ptr [ebp - 8]
    esp = ebp;  // 4a056c mov esp, ebp
    ebp = rd32(esp); esp += 4;  // 4a056e pop ebp
    edi = rd32(esp); esp += 4;  // 4a056f pop edi
    esi = rd32(esp); esp += 4;  // 4a0570 pop esi
    edx = rd32(esp); esp += 4;  // 4a0571 pop edx
    ecx = rd32(esp); esp += 4;  // 4a0572 pop ecx
    ebx = rd32(esp); esp += 4;  // 4a0573 pop ebx
    for (int i_ = 0; i_ < x.n; ++i_) { cpu.fpu.count += 1; cpu.fpu.st(0) = x86::Float(x.v[i_]); }  // 4a0574 ret
    result = eax; popped_ = 0u; return true;  // 4a0574 ret
    return false;
}


bool core4a0980(App* app, CPU& cpu, const Entry& e, reg32& result, reg32& popped_)
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
    const auto SD = [&](reg32 a, double v) { app->getMemory<double>(a) = v; };
    esp -= 4; wr32(esp, ecx);  // 4a0980 push ecx
    esp -= 4; wr32(esp, esi);  // 4a0981 push esi
    esp -= 4; wr32(esp, edi);  // 4a0982 push edi
    esp -= 4; wr32(esp, ebp);  // 4a0983 push ebp
    ebp = esp;  // 4a0984 mov ebp, esp
    fl.logic(edx & edx);  // 4a0986 test edx, edx
    if (fl.ne()) goto L_4a09a2;  // 4a0988 jne 0x4a09a2
    ecx = 9u;  // 4a098a mov ecx, 9
    edi = (eax + 0xc0u);  // 4a098f lea edi, [eax + 0xc0]
    esi = (eax + 0x124u);  // 4a0995 lea esi, [eax + 0x124]
    for (; ecx != 0; --ecx) { wr32(edi, rd32(esi)); esi += 4; edi += 4; }  // 4a099b rep movsd dword ptr es:[edi], dword ptr [esi]
    ebp = rd32(esp); esp += 4;  // 4a099d pop ebp
    edi = rd32(esp); esp += 4;  // 4a099e pop edi
    esi = rd32(esp); esp += 4;  // 4a099f pop esi
    ecx = rd32(esp); esp += 4;  // 4a09a0 pop ecx
    for (int i_ = 0; i_ < x.n; ++i_) { cpu.fpu.count += 1; cpu.fpu.st(0) = x86::Float(x.v[i_]); }  // 4a09a1 ret
    result = eax; popped_ = 0u; return true;  // 4a09a1 ret
L_4a09a2:
    x.push(F(eax + 0x124u));  // 4a09a2 fld dword ptr [eax + 0x124]
    edx = (eax + 0xc0u);  // 4a09a8 lea edx, [eax + 0xc0]
    x.st(0) = -x.st(0);  // 4a09ae fchs
    SF((edx), x.st(0));  // 4a09b0 fstp dword ptr [edx]
    x.pop();  // 4a09b0 fstp dword ptr [edx]
    x.push(F(eax + 0x128u));  // 4a09b2 fld dword ptr [eax + 0x128]
    x.st(0) = -x.st(0);  // 4a09b8 fchs
    SF((edx + 4u), x.st(0));  // 4a09ba fstp dword ptr [edx + 4]
    x.pop();  // 4a09ba fstp dword ptr [edx + 4]
    x.push(F(eax + 0x12cu));  // 4a09bd fld dword ptr [eax + 0x12c]
    x.st(0) = -x.st(0);  // 4a09c3 fchs
    SF((edx + 8u), x.st(0));  // 4a09c5 fstp dword ptr [edx + 8]
    x.pop();  // 4a09c5 fstp dword ptr [edx + 8]
    ecx = rd32((eax + 0x130u));  // 4a09c8 mov ecx, dword ptr [eax + 0x130]
    wr32((edx + 0xcu), ecx);  // 4a09ce mov dword ptr [edx + 0xc], ecx
    ecx = rd32((eax + 0x134u));  // 4a09d1 mov ecx, dword ptr [eax + 0x134]
    wr32((edx + 0x10u), ecx);  // 4a09d7 mov dword ptr [edx + 0x10], ecx
    ecx = rd32((eax + 0x138u));  // 4a09da mov ecx, dword ptr [eax + 0x138]
    wr32((edx + 0x14u), ecx);  // 4a09e0 mov dword ptr [edx + 0x14], ecx
    x.push(F(eax + 0x13cu));  // 4a09e3 fld dword ptr [eax + 0x13c]
    x.st(0) = -x.st(0);  // 4a09e9 fchs
    SF((edx + 0x18u), x.st(0));  // 4a09eb fstp dword ptr [edx + 0x18]
    x.pop();  // 4a09eb fstp dword ptr [edx + 0x18]
    x.push(F(eax + 0x140u));  // 4a09ee fld dword ptr [eax + 0x140]
    x.st(0) = -x.st(0);  // 4a09f4 fchs
    SF((edx + 0x1cu), x.st(0));  // 4a09f6 fstp dword ptr [edx + 0x1c]
    x.pop();  // 4a09f6 fstp dword ptr [edx + 0x1c]
    x.push(F(eax + 0x144u));  // 4a09f9 fld dword ptr [eax + 0x144]
    x.st(0) = -x.st(0);  // 4a09ff fchs
    SF((edx + 0x20u), x.st(0));  // 4a0a01 fstp dword ptr [edx + 0x20]
    x.pop();  // 4a0a01 fstp dword ptr [edx + 0x20]
    ebp = rd32(esp); esp += 4;  // 4a0a04 pop ebp
    edi = rd32(esp); esp += 4;  // 4a0a05 pop edi
    esi = rd32(esp); esp += 4;  // 4a0a06 pop esi
    ecx = rd32(esp); esp += 4;  // 4a0a07 pop ecx
    for (int i_ = 0; i_ < x.n; ++i_) { cpu.fpu.count += 1; cpu.fpu.st(0) = x86::Float(x.v[i_]); }  // 4a0a08 ret
    result = eax; popped_ = 0u; return true;  // 4a0a08 ret
    return false;
}


bool core4a0a10(App* app, CPU& cpu, const Entry& e, reg32& result, reg32& popped_)
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
    const auto SD = [&](reg32 a, double v) { app->getMemory<double>(a) = v; };
    esp -= 4; wr32(esp, ecx);  // 4a0a10 push ecx
    esp -= 4; wr32(esp, esi);  // 4a0a11 push esi
    esp -= 4; wr32(esp, edi);  // 4a0a12 push edi
    esp -= 4; wr32(esp, ebp);  // 4a0a13 push ebp
    ebp = esp;  // 4a0a14 mov ebp, esp
    fl.logic(edx & edx);  // 4a0a16 test edx, edx
    if (fl.ne()) goto L_4a0a32;  // 4a0a18 jne 0x4a0a32
    ecx = 9u;  // 4a0a1a mov ecx, 9
    edi = (eax + 0x190u);  // 4a0a1f lea edi, [eax + 0x190]
    esi = (eax + 0x124u);  // 4a0a25 lea esi, [eax + 0x124]
    for (; ecx != 0; --ecx) { wr32(edi, rd32(esi)); esi += 4; edi += 4; }  // 4a0a2b rep movsd dword ptr es:[edi], dword ptr [esi]
    ebp = rd32(esp); esp += 4;  // 4a0a2d pop ebp
    edi = rd32(esp); esp += 4;  // 4a0a2e pop edi
    esi = rd32(esp); esp += 4;  // 4a0a2f pop esi
    ecx = rd32(esp); esp += 4;  // 4a0a30 pop ecx
    for (int i_ = 0; i_ < x.n; ++i_) { cpu.fpu.count += 1; cpu.fpu.st(0) = x86::Float(x.v[i_]); }  // 4a0a31 ret
    result = eax; popped_ = 0u; return true;  // 4a0a31 ret
L_4a0a32:
    x.push(F(eax + 0x124u));  // 4a0a32 fld dword ptr [eax + 0x124]
    edx = (eax + 0x190u);  // 4a0a38 lea edx, [eax + 0x190]
    x.st(0) = -x.st(0);  // 4a0a3e fchs
    SF((edx), x.st(0));  // 4a0a40 fstp dword ptr [edx]
    x.pop();  // 4a0a40 fstp dword ptr [edx]
    x.push(F(eax + 0x128u));  // 4a0a42 fld dword ptr [eax + 0x128]
    x.st(0) = -x.st(0);  // 4a0a48 fchs
    SF((edx + 4u), x.st(0));  // 4a0a4a fstp dword ptr [edx + 4]
    x.pop();  // 4a0a4a fstp dword ptr [edx + 4]
    x.push(F(eax + 0x12cu));  // 4a0a4d fld dword ptr [eax + 0x12c]
    x.st(0) = -x.st(0);  // 4a0a53 fchs
    SF((edx + 8u), x.st(0));  // 4a0a55 fstp dword ptr [edx + 8]
    x.pop();  // 4a0a55 fstp dword ptr [edx + 8]
    ecx = rd32((eax + 0x130u));  // 4a0a58 mov ecx, dword ptr [eax + 0x130]
    wr32((edx + 0xcu), ecx);  // 4a0a5e mov dword ptr [edx + 0xc], ecx
    ecx = rd32((eax + 0x134u));  // 4a0a61 mov ecx, dword ptr [eax + 0x134]
    wr32((edx + 0x10u), ecx);  // 4a0a67 mov dword ptr [edx + 0x10], ecx
    ecx = rd32((eax + 0x138u));  // 4a0a6a mov ecx, dword ptr [eax + 0x138]
    wr32((edx + 0x14u), ecx);  // 4a0a70 mov dword ptr [edx + 0x14], ecx
    x.push(F(eax + 0x13cu));  // 4a0a73 fld dword ptr [eax + 0x13c]
    x.st(0) = -x.st(0);  // 4a0a79 fchs
    SF((edx + 0x18u), x.st(0));  // 4a0a7b fstp dword ptr [edx + 0x18]
    x.pop();  // 4a0a7b fstp dword ptr [edx + 0x18]
    x.push(F(eax + 0x140u));  // 4a0a7e fld dword ptr [eax + 0x140]
    x.st(0) = -x.st(0);  // 4a0a84 fchs
    SF((edx + 0x1cu), x.st(0));  // 4a0a86 fstp dword ptr [edx + 0x1c]
    x.pop();  // 4a0a86 fstp dword ptr [edx + 0x1c]
    x.push(F(eax + 0x144u));  // 4a0a89 fld dword ptr [eax + 0x144]
    x.st(0) = -x.st(0);  // 4a0a8f fchs
    SF((edx + 0x20u), x.st(0));  // 4a0a91 fstp dword ptr [edx + 0x20]
    x.pop();  // 4a0a91 fstp dword ptr [edx + 0x20]
    ebp = rd32(esp); esp += 4;  // 4a0a94 pop ebp
    edi = rd32(esp); esp += 4;  // 4a0a95 pop edi
    esi = rd32(esp); esp += 4;  // 4a0a96 pop esi
    ecx = rd32(esp); esp += 4;  // 4a0a97 pop ecx
    for (int i_ = 0; i_ < x.n; ++i_) { cpu.fpu.count += 1; cpu.fpu.st(0) = x86::Float(x.v[i_]); }  // 4a0a98 ret
    result = eax; popped_ = 0u; return true;  // 4a0a98 ret
    return false;
}


bool core4a0d20(App* app, CPU& cpu, const Entry& e, reg32& result, reg32& popped_)
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
    const auto SD = [&](reg32 a, double v) { app->getMemory<double>(a) = v; };
    esp -= 4; wr32(esp, ebx);  // 4a0d20 push ebx
    esp -= 4; wr32(esp, ecx);  // 4a0d21 push ecx
    esp -= 4; wr32(esp, ebp);  // 4a0d22 push ebp
    ebp = esp;  // 4a0d23 mov ebp, esp
    esp -= 4u;  // 4a0d25 sub esp, 4
    ecx = eax;  // 4a0d28 mov ecx, eax
    ebx = edx;  // 4a0d2a mov ebx, edx
    edx = 484u;  // 4a0d2c mov edx, 0x1e4
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4e070c, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a0d31 call 0x4e070c
    esp += 0u;  // 4a0d31 call 0x4e070c
    x.push(F(ebp + 0x14u));  // 4a0d36 fld dword ptr [ebp + 0x14]
    eax = rd32((ebp + 0x14u));  // 4a0d39 mov eax, dword ptr [ebp + 0x14]
    wr32((ecx), ebx);  // 4a0d3c mov dword ptr [ecx], ebx
    x.st(0) = xmul(cpu, x.st(0), F(ebp + 0x14u));  // 4a0d3e fmul dword ptr [ebp + 0x14]
    wr32((ecx + 0x114u), eax);  // 4a0d41 mov dword ptr [ecx + 0x114], eax
    x.push(F(ebp + 0x18u));  // 4a0d47 fld dword ptr [ebp + 0x18]
    eax = rd32((ebp + 0x18u));  // 4a0d4a mov eax, dword ptr [ebp + 0x18]
    x.st(0) = xmul(cpu, x.st(0), F(ebp + 0x18u));  // 4a0d4d fmul dword ptr [ebp + 0x18]
    wr32((ecx + 0x118u), eax);  // 4a0d50 mov dword ptr [ecx + 0x118], eax
    x.st(1) = xadd(cpu, x.st(1), x.st(0));  // 4a0d56 faddp st(1)
    x.pop();  // 4a0d56 faddp st(1)
    eax = rd32((ebp + 0x1cu));  // 4a0d58 mov eax, dword ptr [ebp + 0x1c]
    x.push(F(ebp + 0x1cu));  // 4a0d5b fld dword ptr [ebp + 0x1c]
    x.st(0) = xmul(cpu, x.st(0), x.st(0));  // 4a0d5e fmul st(0)
    wr32((ecx + 0x11cu), eax);  // 4a0d60 mov dword ptr [ecx + 0x11c], eax
    x.st(1) = xadd(cpu, x.st(1), x.st(0));  // 4a0d66 faddp st(1)
    x.pop();  // 4a0d66 faddp st(1)
    esp -= 4u;  // 4a0d68 sub esp, 4
    SF((esp), x.st(0));  // 4a0d6b fstp dword ptr [esp]
    x.pop();  // 4a0d6b fstp dword ptr [esp]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4ea920, esp, 1, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a0d6e call 0x4ea920
    esp += 4u;  // 4a0d6e call 0x4ea920
    eax = rd32((ebp + 0x10u));  // 4a0d73 mov eax, dword ptr [ebp + 0x10]
    SF((ecx + 0x120u), x.st(0));  // 4a0d76 fstp dword ptr [ecx + 0x120]
    x.pop();  // 4a0d76 fstp dword ptr [ecx + 0x120]
    wr32((ecx + 0xb0u), eax);  // 4a0d7c mov dword ptr [ecx + 0xb0], eax
    fl.logic(eax & 2147483647u);  // 4a0d82 test eax, 0x7fffffff
    if (fl.ne()) goto L_4a0d93;  // 4a0d87 jne 0x4a0d93
    wr32((ecx + 0xb0u), 1148846080u);  // 4a0d89 mov dword ptr [ecx + 0xb0], 0x447a0000
L_4a0d93:
    x.push(F(ecx + 0xb0u));  // 4a0d93 fld dword ptr [ecx + 0xb0]
    x.push(1.0);  // 4a0d99 fld1
    x.st(1) = xdiv(cpu, x.st(0), x.st(1));  // 4a0d9b fdivrp st(1)
    x.pop();  // 4a0d9b fdivrp st(1)
    x.push(F(ebp + 0x1cu));  // 4a0d9d fld dword ptr [ebp + 0x1c]
    x.st(0) = xmul(cpu, x.st(0), x.st(0));  // 4a0da0 fmul st(0), st(0)
    x.push(F(ebp + 0x18u));  // 4a0da2 fld dword ptr [ebp + 0x18]
    x.st(0) = xmul(cpu, x.st(0), x.st(0));  // 4a0da5 fmul st(0), st(0)
    x.push(x.st(1));  // 4a0da7 fld st(1)
    x.st(0) = xadd(cpu, x.st(0), x.st(1));  // 4a0da9 fadd st(1)
    x.push(D(0x53c7ecu));  // 4a0dab fld qword ptr [0x53c7ec]
    x.push(F(ecx + 0xb0u));  // 4a0db1 fld dword ptr [ecx + 0xb0]
    x.st(0) = xmul(cpu, x.st(0), x.st(1));  // 4a0db7 fmul st(1)
    x.st(2) = xmul(cpu, x.st(2), x.st(0));  // 4a0db9 fmulp st(2)
    x.pop();  // 4a0db9 fmulp st(2)
    x.fxch(1);  // 4a0dbb fxch st(1)
    SF((ecx + 0xf0u), x.st(0));  // 4a0dbd fst dword ptr [ecx + 0xf0]
    x.push(1.0);  // 4a0dc3 fld1
    x.st(1) = xdiv(cpu, x.st(0), x.st(1));  // 4a0dc5 fdivrp st(1)
    x.pop();  // 4a0dc5 fdivrp st(1)
    x.push(F(ebp + 0x14u));  // 4a0dc7 fld dword ptr [ebp + 0x14]
    x.st(0) = xmul(cpu, x.st(0), x.st(0));  // 4a0dca fmul st(0), st(0)
    x.st(4) = xadd(cpu, x.st(4), x.st(0));  // 4a0dcc fadd st(4), st(0)
    x.push(F(ecx + 0xb0u));  // 4a0dce fld dword ptr [ecx + 0xb0]
    x.st(0) = xmul(cpu, x.st(0), x.st(3));  // 4a0dd4 fmul st(3)
    x.st(5) = xmul(cpu, x.st(5), x.st(0));  // 4a0dd6 fmulp st(5)
    x.pop();  // 4a0dd6 fmulp st(5)
    x.fxch(4);  // 4a0dd8 fxch st(4)
    SF((ecx + 0xf4u), x.st(0));  // 4a0dda fst dword ptr [ecx + 0xf4]
    x.push(1.0);  // 4a0de0 fld1
    x.st(1) = xdiv(cpu, x.st(0), x.st(1));  // 4a0de2 fdivrp st(1)
    x.pop();  // 4a0de2 fdivrp st(1)
    x.fxch(4);  // 4a0de4 fxch st(4)
    x.st(3) = xadd(cpu, x.st(3), x.st(0));  // 4a0de6 faddp st(3)
    x.pop();  // 4a0de6 faddp st(3)
    x.push(F(ecx + 0xb0u));  // 4a0de8 fld dword ptr [ecx + 0xb0]
    x.st(2) = xmul(cpu, x.st(2), x.st(0));  // 4a0dee fmulp st(2)
    x.pop();  // 4a0dee fmulp st(2)
    x.fxch(1);  // 4a0df0 fxch st(1)
    x.st(2) = xmul(cpu, x.st(2), x.st(0));  // 4a0df2 fmulp st(2)
    x.pop();  // 4a0df2 fmulp st(2)
    x.fxch(1);  // 4a0df4 fxch st(1)
    SF((ecx + 0xf8u), x.st(0));  // 4a0df6 fst dword ptr [ecx + 0xf8]
    x.push(1.0);  // 4a0dfc fld1
    x.st(1) = xdiv(cpu, x.st(0), x.st(1));  // 4a0dfe fdivrp st(1)
    x.pop();  // 4a0dfe fdivrp st(1)
    wr32((ecx + 0x20u), 0u);  // 4a0e00 mov dword ptr [ecx + 0x20], 0
    wr32((ecx + 0x2cu), 0u);  // 4a0e07 mov dword ptr [ecx + 0x2c], 0
    wr8((ecx + 0x88u), 0u);  // 4a0e0e mov byte ptr [ecx + 0x88], 0
    wr32((ecx + 0xa4u), 0u);  // 4a0e15 mov dword ptr [ecx + 0xa4], 0
    wr32((ecx + 0xa8u), 0u);  // 4a0e1f mov dword ptr [ecx + 0xa8], 0
    wr32((ecx + 0xacu), 0u);  // 4a0e29 mov dword ptr [ecx + 0xac], 0
    wr32((ecx + 0xe4u), 0u);  // 4a0e33 mov dword ptr [ecx + 0xe4], 0
    wr32((ecx + 0xe8u), 0u);  // 4a0e3d mov dword ptr [ecx + 0xe8], 0
    wr32((ecx + 0xecu), 0u);  // 4a0e47 mov dword ptr [ecx + 0xec], 0
    wr32((ecx + 0x108u), 0u);  // 4a0e51 mov dword ptr [ecx + 0x108], 0
    wr32((ecx + 0x10cu), 1065353216u);  // 4a0e5b mov dword ptr [ecx + 0x10c], 0x3f800000
    wr32((ecx + 0x110u), 0u);  // 4a0e65 mov dword ptr [ecx + 0x110], 0
    wr16((ecx + 0x164u), 0u);  // 4a0e6f mov word ptr [ecx + 0x164], 0
    wr8((ecx + 0x8bu), 0u);  // 4a0e78 mov byte ptr [ecx + 0x8b], 0
    wr32((ecx + 0x188u), 1u);  // 4a0e7f mov dword ptr [ecx + 0x188], 1
    wr32((ecx + 0x15cu), 0u);  // 4a0e89 mov dword ptr [ecx + 0x15c], 0
    wr32((ecx + 0x90u), 1065353216u);  // 4a0e93 mov dword ptr [ecx + 0x90], 0x3f800000
    wr32((ecx + 0xbcu), 0u);  // 4a0e9d mov dword ptr [ecx + 0xbc], 0
    wr32((ecx + 0x168u), 0u);  // 4a0ea7 mov dword ptr [ecx + 0x168], 0
    wr32((ecx + 0x16cu), 0u);  // 4a0eb1 mov dword ptr [ecx + 0x16c], 0
    wr32((ecx + 0x170u), 0u);  // 4a0ebb mov dword ptr [ecx + 0x170], 0
    wr32((ecx + 0x174u), 0u);  // 4a0ec5 mov dword ptr [ecx + 0x174], 0
    wr32((ecx + 0x184u), 0u);  // 4a0ecf mov dword ptr [ecx + 0x184], 0
    eax = rd32((ecx + 0x188u));  // 4a0ed9 mov eax, dword ptr [ecx + 0x188]
    wr32((ecx + 0x18cu), eax);  // 4a0edf mov dword ptr [ecx + 0x18c], eax
    x.fxch(3);  // 4a0ee5 fxch st(3)
    SF((ecx + 0xb4u), x.st(0));  // 4a0ee7 fstp dword ptr [ecx + 0xb4]
    x.pop();  // 4a0ee7 fstp dword ptr [ecx + 0xb4]
    SF((ecx + 0xfcu), x.st(0));  // 4a0eed fstp dword ptr [ecx + 0xfc]
    x.pop();  // 4a0eed fstp dword ptr [ecx + 0xfc]
    SF((ecx + 0x100u), x.st(0));  // 4a0ef3 fstp dword ptr [ecx + 0x100]
    x.pop();  // 4a0ef3 fstp dword ptr [ecx + 0x100]
    SF((ecx + 0x104u), x.st(0));  // 4a0ef9 fstp dword ptr [ecx + 0x104]
    x.pop();  // 4a0ef9 fstp dword ptr [ecx + 0x104]
    wr16((ecx + 0x166u), 0u);  // 4a0eff mov word ptr [ecx + 0x166], 0
    wr8((ecx + 0x89u), 1u);  // 4a0f08 mov byte ptr [ecx + 0x89], 1
    wr8((ecx + 0x8au), 0u);  // 4a0f0f mov byte ptr [ecx + 0x8a], 0
    wr32((ecx + 0x94u), 0u);  // 4a0f16 mov dword ptr [ecx + 0x94], 0
    wr8((ecx + 0x8cu), 0u);  // 4a0f20 mov byte ptr [ecx + 0x8c], 0
    esp = ebp;  // 4a0f27 mov esp, ebp
    ebp = rd32(esp); esp += 4;  // 4a0f29 pop ebp
    ecx = rd32(esp); esp += 4;  // 4a0f2a pop ecx
    ebx = rd32(esp); esp += 4;  // 4a0f2b pop ebx
    for (int i_ = 0; i_ < x.n; ++i_) { cpu.fpu.count += 1; cpu.fpu.st(0) = x86::Float(x.v[i_]); }  // 4a0f2c ret 0x10
    result = eax; popped_ = 16u; return true;  // 4a0f2c ret 0x10
    return false;
}


bool core4a0f30(App* app, CPU& cpu, const Entry& e, reg32& result, reg32& popped_)
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
    const auto SD = [&](reg32 a, double v) { app->getMemory<double>(a) = v; };
    esp -= 4; wr32(esp, ebx);  // 4a0f30 push ebx
    esp -= 4; wr32(esp, ecx);  // 4a0f31 push ecx
    esp -= 4; wr32(esp, esi);  // 4a0f32 push esi
    esp -= 4; wr32(esp, edi);  // 4a0f33 push edi
    esp -= 4; wr32(esp, ebp);  // 4a0f34 push ebp
    ebp = esp;  // 4a0f35 mov ebp, esp
    esp -= 8u;  // 4a0f37 sub esp, 8
    wr32((ebp - 4u), eax);  // 4a0f3a mov dword ptr [ebp - 4], eax
    ebx = edx;  // 4a0f3d mov ebx, edx
    fl.logic(rd8((eax + 0x200u)) & 4u);  // 4a0f3f test byte ptr [eax + 0x200], 4
    if (fl.ne()) goto L_4a0f8b;  // 4a0f46 jne 0x4a0f8b
    x.push(F(ebp + 0x18u));  // 4a0f48 fld dword ptr [ebp + 0x18]
    fl.fcmp(x.st(0), F(0x53c7f4u));  // 4a0f4b fcomp dword ptr [0x53c7f4]
    x.pop();  // 4a0f4b fcomp dword ptr [0x53c7f4]
    eax = (eax & 0xffff0000u) | fl.status();  // 4a0f51 fnstsw ax
    // sahf  // 4a0f53 sahf
    if (fl.be()) goto L_4a0f8b;  // 4a0f54 jbe 0x4a0f8b
    eax = rd32((ebp - 4u));  // 4a0f56 mov eax, dword ptr [ebp - 4]
    ecx = 9u;  // 4a0f59 mov ecx, 9
    edi = (eax + 0x190u);  // 4a0f5e lea edi, [eax + 0x190]
    esi = (eax + 0xc0u);  // 4a0f64 lea esi, [eax + 0xc0]
    for (; ecx != 0; --ecx) { wr32(edi, rd32(esi)); esi += 4; edi += 4; }  // 4a0f6a rep movsd dword ptr es:[edi], dword ptr [esi]
    edx = rd32((ebx));  // 4a0f6c mov edx, dword ptr [ebx]
    wr32((eax + 0x19cu), edx);  // 4a0f6e mov dword ptr [eax + 0x19c], edx
    edx = rd32((ebx + 4u));  // 4a0f74 mov edx, dword ptr [ebx + 4]
    wr32((eax + 0x1a0u), edx);  // 4a0f77 mov dword ptr [eax + 0x1a0], edx
    edx = rd32((ebx + 8u));  // 4a0f7d mov edx, dword ptr [ebx + 8]
    wr32((eax + 0x1a4u), edx);  // 4a0f80 mov dword ptr [eax + 0x1a4], edx
    goto L_4a1081;  // 4a0f86 jmp 0x4a1081
L_4a0f8b:
    x.push(F(ebp + 0x18u));  // 4a0f8b fld dword ptr [ebp + 0x18]
    eax = rd32((ebp - 4u));  // 4a0f8e mov eax, dword ptr [ebp - 4]
    edx = rd32((ebx));  // 4a0f91 mov edx, dword ptr [ebx]
    wr32((eax + 0x19cu), edx);  // 4a0f93 mov dword ptr [eax + 0x19c], edx
    edx = rd32((ebx + 4u));  // 4a0f99 mov edx, dword ptr [ebx + 4]
    wr32((eax + 0x1a0u), edx);  // 4a0f9c mov dword ptr [eax + 0x1a0], edx
    edx = rd32((ebx + 8u));  // 4a0fa2 mov edx, dword ptr [ebx + 8]
    wr32((eax + 0x1a4u), edx);  // 4a0fa5 mov dword ptr [eax + 0x1a4], edx
    fl.fcmp(x.st(0), F(0x53c7f8u));  // 4a0fab fcomp dword ptr [0x53c7f8]
    x.pop();  // 4a0fab fcomp dword ptr [0x53c7f8]
    eax = (eax & 0xffff0000u) | fl.status();  // 4a0fb1 fnstsw ax
    // sahf  // 4a0fb3 sahf
    if (fl.ae()) goto L_4a1038;  // 4a0fb4 jae 0x4a1038
    ecx = rd32((ebp - 4u));  // 4a0fba mov ecx, dword ptr [ebp - 4]
    { const reg32 a_ = ecx, b_ = 192u; ecx = a_ + b_; fl.add(a_, b_); }  // 4a0fbd add ecx, 0xc0
    edx = ebx;  // 4a0fc3 mov edx, ebx
    eax = ecx;  // 4a0fc5 mov eax, ecx
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4e01f0, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a0fc7 call 0x4e01f0
    esp += 0u;  // 4a0fc7 call 0x4e01f0
    x.push(0.0);  // 4a0fcc fldz
    fl.fcmp(x.st(0), x.st(1));  // 4a0fce fcompp
    x.pop(); x.pop();  // 4a0fce fcompp
    eax = (eax & 0xffff0000u) | fl.status();  // 4a0fd0 fnstsw ax
    // sahf  // 4a0fd2 sahf
    if (fl.ae()) goto L_4a0fe0;  // 4a0fd3 jae 0x4a0fe0
    edx = ebx;  // 4a0fd5 mov edx, ebx
    eax = ecx;  // 4a0fd7 mov eax, ecx
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4e01f0, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a0fd9 call 0x4e01f0
    esp += 0u;  // 4a0fd9 call 0x4e01f0
    goto L_4a0feb;  // 4a0fde jmp 0x4a0feb
L_4a0fe0:
    edx = ebx;  // 4a0fe0 mov edx, ebx
    eax = ecx;  // 4a0fe2 mov eax, ecx
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4e01f0, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a0fe4 call 0x4e01f0
    esp += 0u;  // 4a0fe4 call 0x4e01f0
    x.st(0) = -x.st(0);  // 4a0fe9 fchs
L_4a0feb:
    SF((ebp - 8u), x.st(0));  // 4a0feb fstp dword ptr [ebp - 8]
    x.pop();  // 4a0feb fstp dword ptr [ebp - 8]
    x.push(F(ebp - 8u));  // 4a0fee fld dword ptr [ebp - 8]
    fl.fcmp(x.st(0), F(0x53c7f8u));  // 4a0ff1 fcomp dword ptr [0x53c7f8]
    x.pop();  // 4a0ff1 fcomp dword ptr [0x53c7f8]
    eax = (eax & 0xffff0000u) | fl.status();  // 4a0ff7 fnstsw ax
    // sahf  // 4a0ff9 sahf
    if (fl.be()) goto L_4a1038;  // 4a0ffa jbe 0x4a1038
    ecx = rd32((ebp - 4u));  // 4a0ffc mov ecx, dword ptr [ebp - 4]
    edx = rd32((ebp - 4u));  // 4a0fff mov edx, dword ptr [ebp - 4]
    edi = rd32((ebp - 4u));  // 4a1002 mov edi, dword ptr [ebp - 4]
    { const reg32 a_ = ecx, b_ = 400u; ecx = a_ + b_; fl.add(a_, b_); }  // 4a1005 add ecx, 0x190
    { const reg32 a_ = edx, b_ = 216u; edx = a_ + b_; fl.add(a_, b_); }  // 4a100b add edx, 0xd8
    { const reg32 a_ = edi, b_ = 412u; edi = a_ + b_; fl.add(a_, b_); }  // 4a1011 add edi, 0x19c
    ebx = ecx;  // 4a1017 mov ebx, ecx
    eax = edi;  // 4a1019 mov eax, edi
    esi = rd32((ebp - 4u));  // 4a101b mov esi, dword ptr [ebp - 4]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4e0160, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a101e call 0x4e0160
    esp += 0u;  // 4a101e call 0x4e0160
    eax = ecx;  // 4a1023 mov eax, ecx
    { const reg32 a_ = esi, b_ = 424u; esi = a_ + b_; fl.add(a_, b_); }  // 4a1025 add esi, 0x1a8
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4972f0, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a102b call 0x4972f0
    esp += 0u;  // 4a102b call 0x4972f0
    ebx = esi;  // 4a1030 mov ebx, esi
    edx = edi;  // 4a1032 mov edx, edi
    eax = ecx;  // 4a1034 mov eax, ecx
    goto L_4a1071;  // 4a1036 jmp 0x4a1071
L_4a1038:
    ecx = rd32((ebp - 4u));  // 4a1038 mov ecx, dword ptr [ebp - 4]
    edi = rd32((ebp - 4u));  // 4a103b mov edi, dword ptr [ebp - 4]
    eax = rd32((ebp - 4u));  // 4a103e mov eax, dword ptr [ebp - 4]
    { const reg32 a_ = ecx, b_ = 424u; ecx = a_ + b_; fl.add(a_, b_); }  // 4a1041 add ecx, 0x1a8
    { const reg32 a_ = edi, b_ = 412u; edi = a_ + b_; fl.add(a_, b_); }  // 4a1047 add edi, 0x19c
    { const reg32 a_ = eax, b_ = 192u; eax = a_ + b_; fl.add(a_, b_); }  // 4a104d add eax, 0xc0
    ebx = ecx;  // 4a1052 mov ebx, ecx
    edx = edi;  // 4a1054 mov edx, edi
    esi = rd32((ebp - 4u));  // 4a1056 mov esi, dword ptr [ebp - 4]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4e0160, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a1059 call 0x4e0160
    esp += 0u;  // 4a1059 call 0x4e0160
    eax = ecx;  // 4a105e mov eax, ecx
    { const reg32 a_ = esi, b_ = 400u; esi = a_ + b_; fl.add(a_, b_); }  // 4a1060 add esi, 0x190
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4972f0, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a1066 call 0x4972f0
    esp += 0u;  // 4a1066 call 0x4972f0
    ebx = esi;  // 4a106b mov ebx, esi
    edx = ecx;  // 4a106d mov edx, ecx
    eax = edi;  // 4a106f mov eax, edi
L_4a1071:
    x.fstp_st(0);  // 4a1071 fstp st(0)
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4e0160, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a1073 call 0x4e0160
    esp += 0u;  // 4a1073 call 0x4e0160
    eax = esi;  // 4a1078 mov eax, esi
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4972f0, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a107a call 0x4972f0
    esp += 0u;  // 4a107a call 0x4972f0
    x.fstp_st(0);  // 4a107f fstp st(0)
L_4a1081:
    esp = ebp;  // 4a1081 mov esp, ebp
    ebp = rd32(esp); esp += 4;  // 4a1083 pop ebp
    edi = rd32(esp); esp += 4;  // 4a1084 pop edi
    esi = rd32(esp); esp += 4;  // 4a1085 pop esi
    ecx = rd32(esp); esp += 4;  // 4a1086 pop ecx
    ebx = rd32(esp); esp += 4;  // 4a1087 pop ebx
    for (int i_ = 0; i_ < x.n; ++i_) { cpu.fpu.count += 1; cpu.fpu.st(0) = x86::Float(x.v[i_]); }  // 4a1088 ret 4
    result = eax; popped_ = 4u; return true;  // 4a1088 ret 4
    return false;
}


bool core4a1090(App* app, CPU& cpu, const Entry& e, reg32& result, reg32& popped_)
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
    const auto SD = [&](reg32 a, double v) { app->getMemory<double>(a) = v; };
    esp -= 4; wr32(esp, ebx);  // 4a1090 push ebx
    esp -= 4; wr32(esp, ecx);  // 4a1091 push ecx
    esp -= 4; wr32(esp, edx);  // 4a1092 push edx
    esp -= 4; wr32(esp, esi);  // 4a1093 push esi
    esp -= 4; wr32(esp, ebp);  // 4a1094 push ebp
    ebp = esp;  // 4a1095 mov ebp, esp
    esp -= 72u;  // 4a1097 sub esp, 0x48
    esi = eax;  // 4a109a mov esi, eax
    eax = (ebp - 0x48u);  // 4a109c lea eax, [ebp - 0x48]
    esp -= 4; wr32(esp, eax);  // 4a109f push eax
    edx = (esi + 0x1a8u);  // 4a10a0 lea edx, [esi + 0x1a8]
    eax = 1u;  // 4a10a6 mov eax, 1
    esp -= 4; wr32(esp, rd32((esi + 0x11cu)));  // 4a10ab push dword ptr [esi + 0x11c]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4e0050, esp, 2, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a10b1 call 0x4e0050
    esp += 8u;  // 4a10b1 call 0x4e0050
    eax = (ebp - 0x30u);  // 4a10b6 lea eax, [ebp - 0x30]
    esp -= 4; wr32(esp, eax);  // 4a10b9 push eax
    edx = (esi + 0x190u);  // 4a10ba lea edx, [esi + 0x190]
    eax = 1u;  // 4a10c0 mov eax, 1
    esp -= 4; wr32(esp, rd32((esi + 0x114u)));  // 4a10c5 push dword ptr [esi + 0x114]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4e0050, esp, 2, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a10cb call 0x4e0050
    esp += 8u;  // 4a10cb call 0x4e0050
    fl.sub(rd32((esi + 0x10cu)), 1061997773u);  // 4a10d0 cmp dword ptr [esi + 0x10c], 0x3f4ccccd
    if (fl.le()) goto L_4a1134;  // 4a10da jle 0x4a1134
    eax = (ebp - 0xcu);  // 4a10dc lea eax, [ebp - 0xc]
    x.push(F(esi + 0x118u));  // 4a10df fld dword ptr [esi + 0x118]
    esp -= 4; wr32(esp, eax);  // 4a10e5 push eax
    x.st(0) = -x.st(0);  // 4a10e6 fchs
    edx = (esi + 0x130u);  // 4a10e8 lea edx, [esi + 0x130]
    esp -= 4u;  // 4a10ee sub esp, 4
    x.st(0) = xsub(cpu, x.st(0), F(esi + 0x15cu));  // 4a10f1 fsub dword ptr [esi + 0x15c]
    eax = 1u;  // 4a10f7 mov eax, 1
    SF((esp), x.st(0));  // 4a10fc fstp dword ptr [esp]
    x.pop();  // 4a10fc fstp dword ptr [esp]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4e0050, esp, 2, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a10ff call 0x4e0050
    esp += 8u;  // 4a10ff call 0x4e0050
    eax = rd32((esi + 0x98u));  // 4a1104 mov eax, dword ptr [esi + 0x98]
    wr32((ebp - 0x3cu), eax);  // 4a110a mov dword ptr [ebp - 0x3c], eax
    eax = rd32((esi + 0x9cu));  // 4a110d mov eax, dword ptr [esi + 0x9c]
    ecx = (ebp - 0x24u);  // 4a1113 lea ecx, [ebp - 0x24]
    wr32((ebp - 0x38u), eax);  // 4a1116 mov dword ptr [ebp - 0x38], eax
    ebx = (ebp - 0xcu);  // 4a1119 lea ebx, [ebp - 0xc]
    eax = rd32((esi + 0xa0u));  // 4a111c mov eax, dword ptr [esi + 0xa0]
    edx = (ebp - 0x3cu);  // 4a1122 lea edx, [ebp - 0x3c]
    wr32((ebp - 0x34u), eax);  // 4a1125 mov dword ptr [ebp - 0x34], eax
    eax = 1u;  // 4a1128 mov eax, 1
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4dffb0, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a112d call 0x4dffb0
    esp += 0u;  // 4a112d call 0x4dffb0
    goto L_4a1158;  // 4a1132 jmp 0x4a1158
L_4a1134:
    eax = rd32((esi + 0x98u));  // 4a1134 mov eax, dword ptr [esi + 0x98]
    wr32((ebp - 0x24u), eax);  // 4a113a mov dword ptr [ebp - 0x24], eax
    eax = rd32((esi + 0x9cu));  // 4a113d mov eax, dword ptr [esi + 0x9c]
    wr32((ebp - 0x20u), eax);  // 4a1143 mov dword ptr [ebp - 0x20], eax
    eax = rd32((esi + 0xa0u));  // 4a1146 mov eax, dword ptr [esi + 0xa0]
    wr32((ebp - 0x1cu), eax);  // 4a114c mov dword ptr [ebp - 0x1c], eax
    eax = rd32((esi + 0x158u));  // 4a114f mov eax, dword ptr [esi + 0x158]
    wr32((ebp - 0x20u), eax);  // 4a1155 mov dword ptr [ebp - 0x20], eax
L_4a1158:
    ecx = (ebp - 0x18u);  // 4a1158 lea ecx, [ebp - 0x18]
    ebx = (ebp - 0x48u);  // 4a115b lea ebx, [ebp - 0x48]
    edx = (ebp - 0x24u);  // 4a115e lea edx, [ebp - 0x24]
    eax = 1u;  // 4a1161 mov eax, 1
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4dffb0, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a1166 call 0x4dffb0
    esp += 0u;  // 4a1166 call 0x4dffb0
    ebx = (ebp - 0x30u);  // 4a116b lea ebx, [ebp - 0x30]
    edx = (ebp - 0x18u);  // 4a116e lea edx, [ebp - 0x18]
    eax = 1u;  // 4a1171 mov eax, 1
    ecx = (esi + 0x1b4u);  // 4a1176 lea ecx, [esi + 0x1b4]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4e0000, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a117c call 0x4e0000
    esp += 0u;  // 4a117c call 0x4e0000
    ebx = (ebp - 0x30u);  // 4a1181 lea ebx, [ebp - 0x30]
    edx = (ebp - 0x18u);  // 4a1184 lea edx, [ebp - 0x18]
    eax = 1u;  // 4a1187 mov eax, 1
    ecx = (esi + 0x1c0u);  // 4a118c lea ecx, [esi + 0x1c0]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4dffb0, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a1192 call 0x4dffb0
    esp += 0u;  // 4a1192 call 0x4dffb0
    ecx = (ebp - 0x18u);  // 4a1197 lea ecx, [ebp - 0x18]
    ebx = (ebp - 0x48u);  // 4a119a lea ebx, [ebp - 0x48]
    edx = (ebp - 0x24u);  // 4a119d lea edx, [ebp - 0x24]
    eax = 1u;  // 4a11a0 mov eax, 1
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4e0000, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a11a5 call 0x4e0000
    esp += 0u;  // 4a11a5 call 0x4e0000
    ebx = (ebp - 0x30u);  // 4a11aa lea ebx, [ebp - 0x30]
    edx = (ebp - 0x18u);  // 4a11ad lea edx, [ebp - 0x18]
    eax = 1u;  // 4a11b0 mov eax, 1
    ecx = (esi + 0x1ccu);  // 4a11b5 lea ecx, [esi + 0x1cc]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4e0000, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a11bb call 0x4e0000
    esp += 0u;  // 4a11bb call 0x4e0000
    x.push(F(ebp - 0x18u));  // 4a11c0 fld dword ptr [ebp - 0x18]
    ebx = (ebp - 0x30u);  // 4a11c3 lea ebx, [ebp - 0x30]
    x.st(0) = xadd(cpu, x.st(0), F(ebp - 0x30u));  // 4a11c6 fadd dword ptr [ebp - 0x30]
    edx = (ebp - 0x18u);  // 4a11c9 lea edx, [ebp - 0x18]
    SF((esi + 0x1d8u), x.st(0));  // 4a11cc fstp dword ptr [esi + 0x1d8]
    x.pop();  // 4a11cc fstp dword ptr [esi + 0x1d8]
    x.push(F(ebp - 0x14u));  // 4a11d2 fld dword ptr [ebp - 0x14]
    x.st(0) = xadd(cpu, x.st(0), F(ebp - 0x2cu));  // 4a11d5 fadd dword ptr [ebp - 0x2c]
    eax = 1u;  // 4a11d8 mov eax, 1
    SF((esi + 0x1dcu), x.st(0));  // 4a11dd fstp dword ptr [esi + 0x1dc]
    x.pop();  // 4a11dd fstp dword ptr [esi + 0x1dc]
    x.push(F(ebp - 0x10u));  // 4a11e3 fld dword ptr [ebp - 0x10]
    x.st(0) = xadd(cpu, x.st(0), F(ebp - 0x28u));  // 4a11e6 fadd dword ptr [ebp - 0x28]
    ecx = (esi + 0x1d8u);  // 4a11e9 lea ecx, [esi + 0x1d8]
    SF((esi + 0x1e0u), x.st(0));  // 4a11ef fstp dword ptr [esi + 0x1e0]
    x.pop();  // 4a11ef fstp dword ptr [esi + 0x1e0]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4dffb0, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a11f5 call 0x4dffb0
    esp += 0u;  // 4a11f5 call 0x4dffb0
    esp = ebp;  // 4a11fa mov esp, ebp
    ebp = rd32(esp); esp += 4;  // 4a11fc pop ebp
    esi = rd32(esp); esp += 4;  // 4a11fd pop esi
    edx = rd32(esp); esp += 4;  // 4a11fe pop edx
    ecx = rd32(esp); esp += 4;  // 4a11ff pop ecx
    ebx = rd32(esp); esp += 4;  // 4a1200 pop ebx
    for (int i_ = 0; i_ < x.n; ++i_) { cpu.fpu.count += 1; cpu.fpu.st(0) = x86::Float(x.v[i_]); }  // 4a1201 ret
    result = eax; popped_ = 0u; return true;  // 4a1201 ret
    return false;
}


bool core4a20c0(App* app, CPU& cpu, const Entry& e, reg32& result, reg32& popped_)
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
    const auto SD = [&](reg32 a, double v) { app->getMemory<double>(a) = v; };
    esp -= 4; wr32(esp, ebx);  // 4a20c0 push ebx
    esp -= 4; wr32(esp, edx);  // 4a20c1 push edx
    esp -= 4; wr32(esp, esi);  // 4a20c2 push esi
    esp -= 4; wr32(esp, ebp);  // 4a20c3 push ebp
    ebp = esp;  // 4a20c4 mov ebp, esp
    esp -= 24u;  // 4a20c6 sub esp, 0x18
    edx = eax;  // 4a20c9 mov edx, eax
    x.push(F(eax + 0xe4u));  // 4a20cb fld dword ptr [eax + 0xe4]
    fl.fcmp(x.st(0), D(0x53c89cu));  // 4a20d1 fcomp qword ptr [0x53c89c]
    x.pop();  // 4a20d1 fcomp qword ptr [0x53c89c]
    eax = (eax & 0xffff0000u) | fl.status();  // 4a20d7 fnstsw ax
    // sahf  // 4a20d9 sahf
    if (fl.ae()) goto L_4a20e5;  // 4a20da jae 0x4a20e5
    wr32((ebp - 8u), 3222903194u);  // 4a20dc mov dword ptr [ebp - 8], 0xc019999a
    goto L_4a2105;  // 4a20e3 jmp 0x4a2105
L_4a20e5:
    ebx = rd32((edx + 0xe4u));  // 4a20e5 mov ebx, dword ptr [edx + 0xe4]
    fl.sub(ebx, 1075419546u);  // 4a20eb cmp ebx, 0x4019999a
    if (fl.le()) goto L_4a20fc;  // 4a20f1 jle 0x4a20fc
    wr32((ebp - 0x14u), 1075419546u);  // 4a20f3 mov dword ptr [ebp - 0x14], 0x4019999a
    goto L_4a20ff;  // 4a20fa jmp 0x4a20ff
L_4a20fc:
    wr32((ebp - 0x14u), ebx);  // 4a20fc mov dword ptr [ebp - 0x14], ebx
L_4a20ff:
    eax = rd32((ebp - 0x14u));  // 4a20ff mov eax, dword ptr [ebp - 0x14]
    wr32((ebp - 8u), eax);  // 4a2102 mov dword ptr [ebp - 8], eax
L_4a2105:
    eax = rd32((ebp - 8u));  // 4a2105 mov eax, dword ptr [ebp - 8]
    x.push(F(edx + 0xe8u));  // 4a2108 fld dword ptr [edx + 0xe8]
    wr32((edx + 0xe4u), eax);  // 4a210e mov dword ptr [edx + 0xe4], eax
    fl.fcmp(x.st(0), D(0x53c89cu));  // 4a2114 fcomp qword ptr [0x53c89c]
    x.pop();  // 4a2114 fcomp qword ptr [0x53c89c]
    eax = (eax & 0xffff0000u) | fl.status();  // 4a211a fnstsw ax
    // sahf  // 4a211c sahf
    if (fl.ae()) goto L_4a2128;  // 4a211d jae 0x4a2128
    wr32((ebp - 0x18u), 3222903194u);  // 4a211f mov dword ptr [ebp - 0x18], 0xc019999a
    goto L_4a2147;  // 4a2126 jmp 0x4a2147
L_4a2128:
    eax = rd32((edx + 0xe8u));  // 4a2128 mov eax, dword ptr [edx + 0xe8]
    fl.sub(eax, 1075419546u);  // 4a212e cmp eax, 0x4019999a
    if (fl.le()) goto L_4a213e;  // 4a2133 jle 0x4a213e
    wr32((ebp - 0xcu), 1075419546u);  // 4a2135 mov dword ptr [ebp - 0xc], 0x4019999a
    goto L_4a2141;  // 4a213c jmp 0x4a2141
L_4a213e:
    wr32((ebp - 0xcu), eax);  // 4a213e mov dword ptr [ebp - 0xc], eax
L_4a2141:
    eax = rd32((ebp - 0xcu));  // 4a2141 mov eax, dword ptr [ebp - 0xc]
    wr32((ebp - 0x18u), eax);  // 4a2144 mov dword ptr [ebp - 0x18], eax
L_4a2147:
    eax = rd32((ebp - 0x18u));  // 4a2147 mov eax, dword ptr [ebp - 0x18]
    x.push(F(edx + 0xecu));  // 4a214a fld dword ptr [edx + 0xec]
    wr32((edx + 0xe8u), eax);  // 4a2150 mov dword ptr [edx + 0xe8], eax
    fl.fcmp(x.st(0), D(0x53c89cu));  // 4a2156 fcomp qword ptr [0x53c89c]
    x.pop();  // 4a2156 fcomp qword ptr [0x53c89c]
    eax = (eax & 0xffff0000u) | fl.status();  // 4a215c fnstsw ax
    // sahf  // 4a215e sahf
    if (fl.ae()) goto L_4a216a;  // 4a215f jae 0x4a216a
    wr32((ebp - 0x10u), 3222903194u);  // 4a2161 mov dword ptr [ebp - 0x10], 0xc019999a
    goto L_4a218a;  // 4a2168 jmp 0x4a218a
L_4a216a:
    esi = rd32((edx + 0xecu));  // 4a216a mov esi, dword ptr [edx + 0xec]
    fl.sub(esi, 1075419546u);  // 4a2170 cmp esi, 0x4019999a
    if (fl.le()) goto L_4a2181;  // 4a2176 jle 0x4a2181
    wr32((ebp - 4u), 1075419546u);  // 4a2178 mov dword ptr [ebp - 4], 0x4019999a
    goto L_4a2184;  // 4a217f jmp 0x4a2184
L_4a2181:
    wr32((ebp - 4u), esi);  // 4a2181 mov dword ptr [ebp - 4], esi
L_4a2184:
    eax = rd32((ebp - 4u));  // 4a2184 mov eax, dword ptr [ebp - 4]
    wr32((ebp - 0x10u), eax);  // 4a2187 mov dword ptr [ebp - 0x10], eax
L_4a218a:
    eax = rd32((ebp - 0x10u));  // 4a218a mov eax, dword ptr [ebp - 0x10]
    wr32((edx + 0xecu), eax);  // 4a218d mov dword ptr [edx + 0xec], eax
    esp = ebp;  // 4a2193 mov esp, ebp
    ebp = rd32(esp); esp += 4;  // 4a2195 pop ebp
    esi = rd32(esp); esp += 4;  // 4a2196 pop esi
    edx = rd32(esp); esp += 4;  // 4a2197 pop edx
    ebx = rd32(esp); esp += 4;  // 4a2198 pop ebx
    for (int i_ = 0; i_ < x.n; ++i_) { cpu.fpu.count += 1; cpu.fpu.st(0) = x86::Float(x.v[i_]); }  // 4a2199 ret
    result = eax; popped_ = 0u; return true;  // 4a2199 ret
    return false;
}


bool core4a2630(App* app, CPU& cpu, const Entry& e, reg32& result, reg32& popped_)
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
    const auto SD = [&](reg32 a, double v) { app->getMemory<double>(a) = v; };
    esp -= 4; wr32(esp, edx);  // 4a2630 push edx
    esp -= 4; wr32(esp, ebp);  // 4a2631 push ebp
    ebp = esp;  // 4a2632 mov ebp, esp
    edx = eax;  // 4a2634 mov edx, eax
    fl.logic(rd8((eax + 0x200u)) & 64u);  // 4a2636 test byte ptr [eax + 0x200], 0x40
    if (fl.e()) goto L_4a264d;  // 4a263d je 0x4a264d
    eax = rd32((edx + 0x14cu));  // 4a263f mov eax, dword ptr [edx + 0x14c]
    wr32((edx + 0x9cu), eax);  // 4a2645 mov dword ptr [edx + 0x9c], eax
    goto L_4a266e;  // 4a264b jmp 0x4a266e
L_4a264d:
    eax = edx;  // 4a264d mov eax, edx
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4a00d0, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a264f call 0x4a00d0
    esp += 0u;  // 4a264f call 0x4a00d0
    x.push(F(edx + 0x118u));  // 4a2654 fld dword ptr [edx + 0x118]
    x.fxch(1);  // 4a265a fxch st(1)
    SF((edx + 0x9cu), x.st(0));  // 4a265c fstp dword ptr [edx + 0x9c]
    x.pop();  // 4a265c fstp dword ptr [edx + 0x9c]
    x.st(0) = xadd(cpu, x.st(0), F(edx + 0x9cu));  // 4a2662 fadd dword ptr [edx + 0x9c]
    SF((edx + 0x9cu), x.st(0));  // 4a2668 fstp dword ptr [edx + 0x9c]
    x.pop();  // 4a2668 fstp dword ptr [edx + 0x9c]
L_4a266e:
    wr16((edx + 0x164u), 0u);  // 4a266e mov word ptr [edx + 0x164], 0
    wr8((edx + 0x8bu), 0u);  // 4a2677 mov byte ptr [edx + 0x8b], 0
    wr32((edx + 0xa8u), 0u);  // 4a267e mov dword ptr [edx + 0xa8], 0
    ebp = rd32(esp); esp += 4;  // 4a2688 pop ebp
    edx = rd32(esp); esp += 4;  // 4a2689 pop edx
    for (int i_ = 0; i_ < x.n; ++i_) { cpu.fpu.count += 1; cpu.fpu.st(0) = x86::Float(x.v[i_]); }  // 4a268a ret
    result = eax; popped_ = 0u; return true;  // 4a268a ret
    return false;
}


bool core4a2830(App* app, CPU& cpu, const Entry& e, reg32& result, reg32& popped_)
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
    const auto SD = [&](reg32 a, double v) { app->getMemory<double>(a) = v; };
    esp -= 4; wr32(esp, ebx);  // 4a2830 push ebx
    esp -= 4; wr32(esp, ecx);  // 4a2831 push ecx
    esp -= 4; wr32(esp, edx);  // 4a2832 push edx
    esp -= 4; wr32(esp, esi);  // 4a2833 push esi
    esp -= 4; wr32(esp, edi);  // 4a2834 push edi
    esp -= 4; wr32(esp, ebp);  // 4a2835 push ebp
    ebp = esp;  // 4a2836 mov ebp, esp
    esp -= 104u;  // 4a2838 sub esp, 0x68
    ecx = eax;  // 4a283b mov ecx, eax
    fl.sub(rd32((0x7d3684u)), 512u);  // 4a283d cmp dword ptr [0x7d3684], 0x200
    if (fl.l()) goto L_4a2bad;  // 4a2847 jl 0x4a2bad
    fl.sub(rd8((eax + 0x89u)), 0u);  // 4a284d cmp byte ptr [eax + 0x89], 0
    if (fl.e()) goto L_4a2bad;  // 4a2854 je 0x4a2bad
    fl.sub(rd8((ecx + 0x88u)), 0u);  // 4a285a cmp byte ptr [ecx + 0x88], 0
    if (fl.e()) goto L_4a286f;  // 4a2861 je 0x4a286f
    eax = ecx;  // 4a2863 mov eax, ecx
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4a2630, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a2865 call 0x4a2630
    esp += 0u;  // 4a2865 call 0x4a2630
    goto L_4a2bad;  // 4a286a jmp 0x4a2bad
L_4a286f:
    eax = ecx;  // 4a286f mov eax, ecx
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4a2260, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a2871 call 0x4a2260
    esp += 0u;  // 4a2871 call 0x4a2260
    eax = ecx;  // 4a2876 mov eax, ecx
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4a00d0, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a2878 call 0x4a00d0
    esp += 0u;  // 4a2878 call 0x4a00d0
    eax = ecx;  // 4a287d mov eax, ecx
    SF((ecx + 0x158u), x.st(0));  // 4a287f fstp dword ptr [ecx + 0x158]
    x.pop();  // 4a287f fstp dword ptr [ecx + 0x158]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4a21a0, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a2885 call 0x4a21a0
    esp += 0u;  // 4a2885 call 0x4a21a0
    eax = ecx;  // 4a288a mov eax, ecx
    edi = (ebp - 0x5cu);  // 4a288c lea edi, [ebp - 0x5c]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4a0440, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a288f call 0x4a0440
    esp += 0u;  // 4a288f call 0x4a0440
    esi = (ecx + 0x130u);  // 4a2894 lea esi, [ecx + 0x130]
    SF((ecx + 0x15cu), x.st(0));  // 4a289a fstp dword ptr [ecx + 0x15c]
    x.pop();  // 4a289a fstp dword ptr [ecx + 0x15c]
    eax = ecx;  // 4a28a0 mov eax, ecx
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 4a28a2 movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 4a28a3 movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 4a28a4 movsd dword ptr es:[edi], dword ptr [esi]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x49fde0, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a28a5 call 0x49fde0
    esp += 0u;  // 4a28a5 call 0x49fde0
    fl.logic(eax & eax);  // 4a28aa test eax, eax
    if (fl.e()) goto L_4a28bc;  // 4a28ac je 0x4a28bc
    x.push(F(ebp - 0x58u));  // 4a28ae fld dword ptr [ebp - 0x58]
    fl.fcmp(x.st(0), D(0x53c8c4u));  // 4a28b1 fcomp qword ptr [0x53c8c4]
    x.pop();  // 4a28b1 fcomp qword ptr [0x53c8c4]
    eax = (eax & 0xffff0000u) | fl.status();  // 4a28b7 fnstsw ax
    // sahf  // 4a28b9 sahf
    if (fl.ae()) goto L_4a28d2;  // 4a28ba jae 0x4a28d2
L_4a28bc:
    fl.sub(rd16((ecx + 0x164u)), 0u);  // 4a28bc cmp word ptr [ecx + 0x164], 0
    if (fl.ne()) goto L_4a28d2;  // 4a28c4 jne 0x4a28d2
    ebx = 1u;  // 4a28c6 mov ebx, 1
    wr16((ecx + 0x164u), (ebx & 0xffffu));  // 4a28cb mov word ptr [ecx + 0x164], bx
L_4a28d2:
    x.push(0.0);  // 4a28d2 fldz
    fl.fcmp(x.st(0), F(ecx + 0xa4u));  // 4a28d4 fcomp dword ptr [ecx + 0xa4]
    x.pop();  // 4a28d4 fcomp dword ptr [ecx + 0xa4]
    eax = (eax & 0xffff0000u) | fl.status();  // 4a28da fnstsw ax
    // sahf  // 4a28dc sahf
    if (fl.ae()) goto L_4a28ea;  // 4a28dd jae 0x4a28ea
    eax = rd32((ecx + 0xa4u));  // 4a28df mov eax, dword ptr [ecx + 0xa4]
    wr32((ebp - 8u), eax);  // 4a28e5 mov dword ptr [ebp - 8], eax
    goto L_4a28f5;  // 4a28e8 jmp 0x4a28f5
L_4a28ea:
    x.push(F(ecx + 0xa4u));  // 4a28ea fld dword ptr [ecx + 0xa4]
    x.st(0) = -x.st(0);  // 4a28f0 fchs
    SF((ebp - 8u), x.st(0));  // 4a28f2 fstp dword ptr [ebp - 8]
    x.pop();  // 4a28f2 fstp dword ptr [ebp - 8]
L_4a28f5:
    x.push(0.0);  // 4a28f5 fldz
    fl.fcmp(x.st(0), F(ecx + 0xacu));  // 4a28f7 fcomp dword ptr [ecx + 0xac]
    x.pop();  // 4a28f7 fcomp dword ptr [ecx + 0xac]
    eax = (eax & 0xffff0000u) | fl.status();  // 4a28fd fnstsw ax
    // sahf  // 4a28ff sahf
    if (fl.ae()) goto L_4a290d;  // 4a2900 jae 0x4a290d
    eax = rd32((ecx + 0xacu));  // 4a2902 mov eax, dword ptr [ecx + 0xac]
    wr32((ebp - 0x10u), eax);  // 4a2908 mov dword ptr [ebp - 0x10], eax
    goto L_4a2918;  // 4a290b jmp 0x4a2918
L_4a290d:
    x.push(F(ecx + 0xacu));  // 4a290d fld dword ptr [ecx + 0xac]
    x.st(0) = -x.st(0);  // 4a2913 fchs
    SF((ebp - 0x10u), x.st(0));  // 4a2915 fstp dword ptr [ebp - 0x10]
    x.pop();  // 4a2915 fstp dword ptr [ebp - 0x10]
L_4a2918:
    x.push(F(ebp - 8u));  // 4a2918 fld dword ptr [ebp - 8]
    fl.fcmp(x.st(0), F(ebp - 0x10u));  // 4a291b fcomp dword ptr [ebp - 0x10]
    x.pop();  // 4a291b fcomp dword ptr [ebp - 0x10]
    eax = (eax & 0xffff0000u) | fl.status();  // 4a291e fnstsw ax
    // sahf  // 4a2920 sahf
    if (fl.be()) goto L_4a297f;  // 4a2921 jbe 0x4a297f
    x.push(0.0);  // 4a2923 fldz
    fl.fcmp(x.st(0), F(ecx + 0xacu));  // 4a2925 fcomp dword ptr [ecx + 0xac]
    x.pop();  // 4a2925 fcomp dword ptr [ecx + 0xac]
    eax = (eax & 0xffff0000u) | fl.status();  // 4a292b fnstsw ax
    // sahf  // 4a292d sahf
    if (fl.ae()) goto L_4a293b;  // 4a292e jae 0x4a293b
    eax = rd32((ecx + 0xacu));  // 4a2930 mov eax, dword ptr [ecx + 0xac]
    wr32((ebp - 0x1cu), eax);  // 4a2936 mov dword ptr [ebp - 0x1c], eax
    goto L_4a2946;  // 4a2939 jmp 0x4a2946
L_4a293b:
    x.push(F(ecx + 0xacu));  // 4a293b fld dword ptr [ecx + 0xac]
    x.st(0) = -x.st(0);  // 4a2941 fchs
    SF((ebp - 0x1cu), x.st(0));  // 4a2943 fstp dword ptr [ebp - 0x1c]
    x.pop();  // 4a2943 fstp dword ptr [ebp - 0x1c]
L_4a2946:
    x.push(F(ebp - 0x1cu));  // 4a2946 fld dword ptr [ebp - 0x1c]
    x.st(0) = xmul(cpu, x.st(0), F(0x53c8ccu));  // 4a2949 fmul dword ptr [0x53c8cc]
    x.push(0.0);  // 4a294f fldz
    x.fxch(1);  // 4a2951 fxch st(1)
    SF((ebp - 0x24u), x.st(0));  // 4a2953 fstp dword ptr [ebp - 0x24]
    x.pop();  // 4a2953 fstp dword ptr [ebp - 0x24]
    fl.fcmp(x.st(0), F(ecx + 0xa4u));  // 4a2956 fcomp dword ptr [ecx + 0xa4]
    x.pop();  // 4a2956 fcomp dword ptr [ecx + 0xa4]
    eax = (eax & 0xffff0000u) | fl.status();  // 4a295c fnstsw ax
    // sahf  // 4a295e sahf
    if (fl.ae()) goto L_4a296c;  // 4a295f jae 0x4a296c
    eax = rd32((ecx + 0xa4u));  // 4a2961 mov eax, dword ptr [ecx + 0xa4]
    wr32((ebp - 0xcu), eax);  // 4a2967 mov dword ptr [ebp - 0xc], eax
    goto L_4a2977;  // 4a296a jmp 0x4a2977
L_4a296c:
    x.push(F(ecx + 0xa4u));  // 4a296c fld dword ptr [ecx + 0xa4]
    x.st(0) = -x.st(0);  // 4a2972 fchs
    SF((ebp - 0xcu), x.st(0));  // 4a2974 fstp dword ptr [ebp - 0xc]
    x.pop();  // 4a2974 fstp dword ptr [ebp - 0xc]
L_4a2977:
    x.push(F(ebp - 0xcu));  // 4a2977 fld dword ptr [ebp - 0xc]
    x.st(0) = xadd(cpu, x.st(0), F(ebp - 0x24u));  // 4a297a fadd dword ptr [ebp - 0x24]
    goto L_4a29d9;  // 4a297d jmp 0x4a29d9
L_4a297f:
    x.push(0.0);  // 4a297f fldz
    fl.fcmp(x.st(0), F(ecx + 0xa4u));  // 4a2981 fcomp dword ptr [ecx + 0xa4]
    x.pop();  // 4a2981 fcomp dword ptr [ecx + 0xa4]
    eax = (eax & 0xffff0000u) | fl.status();  // 4a2987 fnstsw ax
    // sahf  // 4a2989 sahf
    if (fl.ae()) goto L_4a2997;  // 4a298a jae 0x4a2997
    eax = rd32((ecx + 0xa4u));  // 4a298c mov eax, dword ptr [ecx + 0xa4]
    wr32((ebp - 0x18u), eax);  // 4a2992 mov dword ptr [ebp - 0x18], eax
    goto L_4a29a2;  // 4a2995 jmp 0x4a29a2
L_4a2997:
    x.push(F(ecx + 0xa4u));  // 4a2997 fld dword ptr [ecx + 0xa4]
    x.st(0) = -x.st(0);  // 4a299d fchs
    SF((ebp - 0x18u), x.st(0));  // 4a299f fstp dword ptr [ebp - 0x18]
    x.pop();  // 4a299f fstp dword ptr [ebp - 0x18]
L_4a29a2:
    x.push(F(ebp - 0x18u));  // 4a29a2 fld dword ptr [ebp - 0x18]
    x.st(0) = xmul(cpu, x.st(0), F(0x53c8ccu));  // 4a29a5 fmul dword ptr [0x53c8cc]
    x.push(0.0);  // 4a29ab fldz
    x.fxch(1);  // 4a29ad fxch st(1)
    SF((ebp - 0x20u), x.st(0));  // 4a29af fstp dword ptr [ebp - 0x20]
    x.pop();  // 4a29af fstp dword ptr [ebp - 0x20]
    fl.fcmp(x.st(0), F(ecx + 0xacu));  // 4a29b2 fcomp dword ptr [ecx + 0xac]
    x.pop();  // 4a29b2 fcomp dword ptr [ecx + 0xac]
    eax = (eax & 0xffff0000u) | fl.status();  // 4a29b8 fnstsw ax
    // sahf  // 4a29ba sahf
    if (fl.ae()) goto L_4a29c8;  // 4a29bb jae 0x4a29c8
    eax = rd32((ecx + 0xacu));  // 4a29bd mov eax, dword ptr [ecx + 0xac]
    wr32((ebp - 0x14u), eax);  // 4a29c3 mov dword ptr [ebp - 0x14], eax
    goto L_4a29d3;  // 4a29c6 jmp 0x4a29d3
L_4a29c8:
    x.push(F(ecx + 0xacu));  // 4a29c8 fld dword ptr [ecx + 0xac]
    x.st(0) = -x.st(0);  // 4a29ce fchs
    SF((ebp - 0x14u), x.st(0));  // 4a29d0 fstp dword ptr [ebp - 0x14]
    x.pop();  // 4a29d0 fstp dword ptr [ebp - 0x14]
L_4a29d3:
    x.push(F(ebp - 0x14u));  // 4a29d3 fld dword ptr [ebp - 0x14]
    x.st(0) = xadd(cpu, x.st(0), F(ebp - 0x20u));  // 4a29d6 fadd dword ptr [ebp - 0x20]
L_4a29d9:
    SF((ebp - 4u), x.st(0));  // 4a29d9 fstp dword ptr [ebp - 4]
    x.pop();  // 4a29d9 fstp dword ptr [ebp - 4]
    eax = rd32((ebp - 4u));  // 4a29dc mov eax, dword ptr [ebp - 4]
    esi = (esi & 0xffff0000u) | ((rd16((ecx + 0x164u))) & 0xffffu);  // 4a29df mov si, word ptr [ecx + 0x164]
    wr32((ecx + 0xb8u), eax);  // 4a29e6 mov dword ptr [ecx + 0xb8], eax
    fl.logic((esi & 0xffffu) & (esi & 0xffffu));  // 4a29ec test si, si
    if (fl.e()) goto L_4a2a0c;  // 4a29ef je 0x4a2a0c
    x.push(F(ecx + 0xa8u));  // 4a29f1 fld dword ptr [ecx + 0xa8]
    x.push(x.st(0));  // 4a29f7 fld st(0)
    x.st(0) = xadd(cpu, x.st(0), D(0x53c8d4u));  // 4a29f9 fadd qword ptr [0x53c8d4]
    x.fstp_st(1);  // 4a29ff fstp st(1)
    SF((ecx + 0xa8u), x.st(0));  // 4a2a01 fstp dword ptr [ecx + 0xa8]
    x.pop();  // 4a2a01 fstp dword ptr [ecx + 0xa8]
    goto L_4a2b28;  // 4a2a07 jmp 0x4a2b28
L_4a2a0c:
    x.push(F(0x53c8dcu));  // 4a2a0c fld dword ptr [0x53c8dc]
    x.push(F(ecx + 0x194u));  // 4a2a12 fld dword ptr [ecx + 0x194]
    x.st(0) = xmul(cpu, x.st(0), x.st(1));  // 4a2a18 fmul st(1)
    x.push(F(ecx + 0x1a0u));  // 4a2a1a fld dword ptr [ecx + 0x1a0]
    x.st(0) = xmul(cpu, x.st(0), x.st(2));  // 4a2a20 fmul st(2)
    x.push(F(ecx + 0x1acu));  // 4a2a22 fld dword ptr [ecx + 0x1ac]
    x.st(3) = xmul(cpu, x.st(3), x.st(0));  // 4a2a28 fmulp st(3)
    x.pop();  // 4a2a28 fmulp st(3)
    eax = (ebp - 0x68u);  // 4a2a2a lea eax, [ebp - 0x68]
    edx = (ecx + 0x190u);  // 4a2a2d lea edx, [ecx + 0x190]
    esp -= 4; wr32(esp, eax);  // 4a2a33 push eax
    x.fxch(1);  // 4a2a34 fxch st(1)
    SF((ebp - 0x30u), x.st(0));  // 4a2a36 fstp dword ptr [ebp - 0x30]
    x.pop();  // 4a2a36 fstp dword ptr [ebp - 0x30]
    SF((ebp - 0x2cu), x.st(0));  // 4a2a39 fstp dword ptr [ebp - 0x2c]
    x.pop();  // 4a2a39 fstp dword ptr [ebp - 0x2c]
    esp -= 4; wr32(esp, rd32((ebp - 0x30u)));  // 4a2a3c push dword ptr [ebp - 0x30]
    eax = 1u;  // 4a2a3f mov eax, 1
    SF((ebp - 0x28u), x.st(0));  // 4a2a44 fstp dword ptr [ebp - 0x28]
    x.pop();  // 4a2a44 fstp dword ptr [ebp - 0x28]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4e0050, esp, 2, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a2a47 call 0x4e0050
    esp += 8u;  // 4a2a47 call 0x4e0050
    eax = (ebp - 0x50u);  // 4a2a4c lea eax, [ebp - 0x50]
    esp -= 4; wr32(esp, eax);  // 4a2a4f push eax
    edx = (ecx + 0x19cu);  // 4a2a50 lea edx, [ecx + 0x19c]
    eax = 1u;  // 4a2a56 mov eax, 1
    esp -= 4; wr32(esp, rd32((ebp - 0x2cu)));  // 4a2a5b push dword ptr [ebp - 0x2c]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4e0050, esp, 2, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a2a5e call 0x4e0050
    esp += 8u;  // 4a2a5e call 0x4e0050
    eax = (ebp - 0x44u);  // 4a2a63 lea eax, [ebp - 0x44]
    esp -= 4; wr32(esp, eax);  // 4a2a66 push eax
    edx = (ecx + 0x1a8u);  // 4a2a67 lea edx, [ecx + 0x1a8]
    eax = 1u;  // 4a2a6d mov eax, 1
    esp -= 4; wr32(esp, rd32((ebp - 0x28u)));  // 4a2a72 push dword ptr [ebp - 0x28]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4e0050, esp, 2, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a2a75 call 0x4e0050
    esp += 8u;  // 4a2a75 call 0x4e0050
    x.push(F(ecx + 0xa4u));  // 4a2a7a fld dword ptr [ecx + 0xa4]
    x.st(0) = xadd(cpu, x.st(0), F(ebp - 0x50u));  // 4a2a80 fadd dword ptr [ebp - 0x50]
    x.push(F(ecx + 0xa8u));  // 4a2a83 fld dword ptr [ecx + 0xa8]
    x.fxch(1);  // 4a2a89 fxch st(1)
    SF((ecx + 0xa4u), x.st(0));  // 4a2a8b fstp dword ptr [ecx + 0xa4]
    x.pop();  // 4a2a8b fstp dword ptr [ecx + 0xa4]
    x.st(0) = xadd(cpu, x.st(0), F(ebp - 0x4cu));  // 4a2a91 fadd dword ptr [ebp - 0x4c]
    x.push(F(ecx + 0xacu));  // 4a2a94 fld dword ptr [ecx + 0xac]
    x.fxch(1);  // 4a2a9a fxch st(1)
    SF((ecx + 0xa8u), x.st(0));  // 4a2a9c fstp dword ptr [ecx + 0xa8]
    x.pop();  // 4a2a9c fstp dword ptr [ecx + 0xa8]
    x.st(0) = xadd(cpu, x.st(0), F(ebp - 0x48u));  // 4a2aa2 fadd dword ptr [ebp - 0x48]
    SF((ecx + 0xacu), x.st(0));  // 4a2aa5 fstp dword ptr [ecx + 0xac]
    x.pop();  // 4a2aa5 fstp dword ptr [ecx + 0xac]
    x.push(F(ebp - 0x58u));  // 4a2aab fld dword ptr [ebp - 0x58]
    fl.fcmp(x.st(0), F(0x5efdd8u));  // 4a2aae fcomp dword ptr [0x5efdd8]
    x.pop();  // 4a2aae fcomp dword ptr [0x5efdd8]
    eax = (eax & 0xffff0000u) | fl.status();  // 4a2ab4 fnstsw ax
    // sahf  // 4a2ab6 sahf
    if (fl.ae()) goto L_4a2aeb;  // 4a2ab7 jae 0x4a2aeb
    x.push(F(ebp - 0x68u));  // 4a2ab9 fld dword ptr [ebp - 0x68]
    x.st(0) = xadd(cpu, x.st(0), F(ebp - 0x44u));  // 4a2abc fadd dword ptr [ebp - 0x44]
    x.st(0) = xadd(cpu, x.st(0), F(ecx + 0xa4u));  // 4a2abf fadd dword ptr [ecx + 0xa4]
    SF((ecx + 0xa4u), x.st(0));  // 4a2ac5 fstp dword ptr [ecx + 0xa4]
    x.pop();  // 4a2ac5 fstp dword ptr [ecx + 0xa4]
    x.push(F(ebp - 0x64u));  // 4a2acb fld dword ptr [ebp - 0x64]
    x.st(0) = xadd(cpu, x.st(0), F(ebp - 0x40u));  // 4a2ace fadd dword ptr [ebp - 0x40]
    x.st(0) = xadd(cpu, x.st(0), F(ecx + 0xa8u));  // 4a2ad1 fadd dword ptr [ecx + 0xa8]
    SF((ecx + 0xa8u), x.st(0));  // 4a2ad7 fstp dword ptr [ecx + 0xa8]
    x.pop();  // 4a2ad7 fstp dword ptr [ecx + 0xa8]
    x.push(F(ebp - 0x60u));  // 4a2add fld dword ptr [ebp - 0x60]
    x.st(0) = xadd(cpu, x.st(0), F(ebp - 0x3cu));  // 4a2ae0 fadd dword ptr [ebp - 0x3c]
    x.st(0) = xadd(cpu, x.st(0), F(ecx + 0xacu));  // 4a2ae3 fadd dword ptr [ecx + 0xac]
    goto L_4a2b22;  // 4a2ae9 jmp 0x4a2b22
L_4a2aeb:
    fl.sub(rd32((ecx + 0x10cu)), 1063675494u);  // 4a2aeb cmp dword ptr [ecx + 0x10c], 0x3f666666
    if (fl.le()) goto L_4a2b28;  // 4a2af5 jle 0x4a2b28
    x.push(F(ecx + 0xa4u));  // 4a2af7 fld dword ptr [ecx + 0xa4]
    x.st(0) = xadd(cpu, x.st(0), F(ebp - 0x44u));  // 4a2afd fadd dword ptr [ebp - 0x44]
    x.push(F(ecx + 0xa8u));  // 4a2b00 fld dword ptr [ecx + 0xa8]
    x.fxch(1);  // 4a2b06 fxch st(1)
    SF((ecx + 0xa4u), x.st(0));  // 4a2b08 fstp dword ptr [ecx + 0xa4]
    x.pop();  // 4a2b08 fstp dword ptr [ecx + 0xa4]
    x.st(0) = xadd(cpu, x.st(0), F(ebp - 0x40u));  // 4a2b0e fadd dword ptr [ebp - 0x40]
    x.push(F(ecx + 0xacu));  // 4a2b11 fld dword ptr [ecx + 0xac]
    x.fxch(1);  // 4a2b17 fxch st(1)
    SF((ecx + 0xa8u), x.st(0));  // 4a2b19 fstp dword ptr [ecx + 0xa8]
    x.pop();  // 4a2b19 fstp dword ptr [ecx + 0xa8]
    x.st(0) = xadd(cpu, x.st(0), F(ebp - 0x3cu));  // 4a2b1f fadd dword ptr [ebp - 0x3c]
L_4a2b22:
    SF((ecx + 0xacu), x.st(0));  // 4a2b22 fstp dword ptr [ecx + 0xac]
    x.pop();  // 4a2b22 fstp dword ptr [ecx + 0xac]
L_4a2b28:
    esi = rd32((ecx + 0x15cu));  // 4a2b28 mov esi, dword ptr [ecx + 0x15c]
    wr8((ecx + 0x8bu), 0u);  // 4a2b2e mov byte ptr [ecx + 0x8b], 0
    fl.sub(esi, 1058642330u);  // 4a2b35 cmp esi, 0x3f19999a
    if (fl.ge()) goto L_4a2b86;  // 4a2b3b jge 0x4a2b86
    fl.sub(rd16((ecx + 0x164u)), 0u);  // 4a2b3d cmp word ptr [ecx + 0x164], 0
    if (fl.e()) goto L_4a2b6a;  // 4a2b45 je 0x4a2b6a
    fl.logic(rd8((ecx + 0x200u)) & 4u);  // 4a2b47 test byte ptr [ecx + 0x200], 4
    if (fl.e()) goto L_4a2b57;  // 4a2b4e je 0x4a2b57
    eax = ecx;  // 4a2b50 mov eax, ecx
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4aa4f4, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a2b52 call 0x4aa4f4
    esp += 0u;  // 4a2b52 call 0x4aa4f4
L_4a2b57:
    edx = (ebp - 0x5cu);  // 4a2b57 lea edx, [ebp - 0x5c]
    eax = ecx;  // 4a2b5a mov eax, ecx
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4a1ce0, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a2b5c call 0x4a1ce0
    esp += 0u;  // 4a2b5c call 0x4a1ce0
    esp = ebp;  // 4a2b61 mov esp, ebp
    ebp = rd32(esp); esp += 4;  // 4a2b63 pop ebp
    edi = rd32(esp); esp += 4;  // 4a2b64 pop edi
    esi = rd32(esp); esp += 4;  // 4a2b65 pop esi
    edx = rd32(esp); esp += 4;  // 4a2b66 pop edx
    ecx = rd32(esp); esp += 4;  // 4a2b67 pop ecx
    ebx = rd32(esp); esp += 4;  // 4a2b68 pop ebx
    for (int i_ = 0; i_ < x.n; ++i_) { cpu.fpu.count += 1; cpu.fpu.st(0) = x86::Float(x.v[i_]); }  // 4a2b69 ret
    result = eax; popped_ = 0u; return true;  // 4a2b69 ret
L_4a2b6a:
    edi = rd32((ebp - 0x54u));  // 4a2b6a mov edi, dword ptr [ebp - 0x54]
    esp -= 4; wr32(esp, edi);  // 4a2b6d push edi
    eax = rd32((ebp - 0x58u));  // 4a2b6e mov eax, dword ptr [ebp - 0x58]
    esp -= 4; wr32(esp, eax);  // 4a2b71 push eax
    edx = rd32((ebp - 0x5cu));  // 4a2b72 mov edx, dword ptr [ebp - 0x5c]
    esp -= 4; wr32(esp, edx);  // 4a2b75 push edx
    eax = ecx;  // 4a2b76 mov eax, ecx
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4a1610, esp, -3, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a2b78 call 0x4a1610
    esp += -12u;  // 4a2b78 call 0x4a1610
    esp = ebp;  // 4a2b7d mov esp, ebp
    ebp = rd32(esp); esp += 4;  // 4a2b7f pop ebp
    edi = rd32(esp); esp += 4;  // 4a2b80 pop edi
    esi = rd32(esp); esp += 4;  // 4a2b81 pop esi
    edx = rd32(esp); esp += 4;  // 4a2b82 pop edx
    ecx = rd32(esp); esp += 4;  // 4a2b83 pop ecx
    ebx = rd32(esp); esp += 4;  // 4a2b84 pop ebx
    for (int i_ = 0; i_ < x.n; ++i_) { cpu.fpu.count += 1; cpu.fpu.st(0) = x86::Float(x.v[i_]); }  // 4a2b85 ret
    result = eax; popped_ = 0u; return true;  // 4a2b85 ret
L_4a2b86:
    fl.sub(rd16((ecx + 0x164u)), 0u);  // 4a2b86 cmp word ptr [ecx + 0x164], 0
    if (fl.ne()) goto L_4a2b97;  // 4a2b8e jne 0x4a2b97
    wr8((ecx + 0x8bu), 1u);  // 4a2b90 mov byte ptr [ecx + 0x8b], 1
L_4a2b97:
    edx = (edx & 0xffff0000u) | ((rd16((ecx + 0x164u))) & 0xffffu);  // 4a2b97 mov dx, word ptr [ecx + 0x164]
    { const reg32 r_ = edx + 1u; edx = r_; fl.arith(r_); }  // 4a2b9e inc edx
    eax = ecx;  // 4a2b9f mov eax, ecx
    wr16((ecx + 0x164u), (edx & 0xffffu));  // 4a2ba1 mov word ptr [ecx + 0x164], dx
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4a20c0, esp, -6, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a2ba8 call 0x4a20c0
    esp += -24u;  // 4a2ba8 call 0x4a20c0
L_4a2bad:
    esp = ebp;  // 4a2bad mov esp, ebp
    ebp = rd32(esp); esp += 4;  // 4a2baf pop ebp
    edi = rd32(esp); esp += 4;  // 4a2bb0 pop edi
    esi = rd32(esp); esp += 4;  // 4a2bb1 pop esi
    edx = rd32(esp); esp += 4;  // 4a2bb2 pop edx
    ecx = rd32(esp); esp += 4;  // 4a2bb3 pop ecx
    ebx = rd32(esp); esp += 4;  // 4a2bb4 pop ebx
    for (int i_ = 0; i_ < x.n; ++i_) { cpu.fpu.count += 1; cpu.fpu.st(0) = x86::Float(x.v[i_]); }  // 4a2bb5 ret
    result = eax; popped_ = 0u; return true;  // 4a2bb5 ret
    return false;
}


bool core4a2c60(App* app, CPU& cpu, const Entry& e, reg32& result, reg32& popped_)
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
    const auto SD = [&](reg32 a, double v) { app->getMemory<double>(a) = v; };
    esp -= 4; wr32(esp, ebp);  // 4a2c60 push ebp
    ebp = esp;  // 4a2c61 mov ebp, esp
    fl.sub(rd8((eax + 0x89u)), 0u);  // 4a2c63 cmp byte ptr [eax + 0x89], 0
    if (fl.e()) goto L_4a2cb8;  // 4a2c6a je 0x4a2cb8
    x.push(F(0x53c8ecu));  // 4a2c6c fld dword ptr [0x53c8ec]
    x.push(F(eax + 0xa4u));  // 4a2c72 fld dword ptr [eax + 0xa4]
    x.st(0) = xmul(cpu, x.st(0), x.st(1));  // 4a2c78 fmul st(1)
    x.push(F(eax + 0xa8u));  // 4a2c7a fld dword ptr [eax + 0xa8]
    x.st(0) = xmul(cpu, x.st(0), x.st(2));  // 4a2c80 fmul st(2)
    x.push(F(eax + 0xacu));  // 4a2c82 fld dword ptr [eax + 0xac]
    x.st(3) = xmul(cpu, x.st(3), x.st(0));  // 4a2c88 fmulp st(3)
    x.pop();  // 4a2c88 fmulp st(3)
    x.fxch(1);  // 4a2c8a fxch st(1)
    x.st(0) = xadd(cpu, x.st(0), F(eax + 0x98u));  // 4a2c8c fadd dword ptr [eax + 0x98]
    x.fxch(1);  // 4a2c92 fxch st(1)
    x.st(0) = xadd(cpu, x.st(0), F(eax + 0x9cu));  // 4a2c94 fadd dword ptr [eax + 0x9c]
    x.fxch(2);  // 4a2c9a fxch st(2)
    x.st(0) = xadd(cpu, x.st(0), F(eax + 0xa0u));  // 4a2c9c fadd dword ptr [eax + 0xa0]
    x.fxch(2);  // 4a2ca2 fxch st(2)
    SF((eax + 0x9cu), x.st(0));  // 4a2ca4 fstp dword ptr [eax + 0x9c]
    x.pop();  // 4a2ca4 fstp dword ptr [eax + 0x9c]
    x.fxch(1);  // 4a2caa fxch st(1)
    SF((eax + 0xa0u), x.st(0));  // 4a2cac fstp dword ptr [eax + 0xa0]
    x.pop();  // 4a2cac fstp dword ptr [eax + 0xa0]
    SF((eax + 0x98u), x.st(0));  // 4a2cb2 fstp dword ptr [eax + 0x98]
    x.pop();  // 4a2cb2 fstp dword ptr [eax + 0x98]
L_4a2cb8:
    ebp = rd32(esp); esp += 4;  // 4a2cb8 pop ebp
    for (int i_ = 0; i_ < x.n; ++i_) { cpu.fpu.count += 1; cpu.fpu.st(0) = x86::Float(x.v[i_]); }  // 4a2cb9 ret
    result = eax; popped_ = 0u; return true;  // 4a2cb9 ret
    return false;
}


bool core4a2e20(App* app, CPU& cpu, const Entry& e, reg32& result, reg32& popped_)
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
    const auto SD = [&](reg32 a, double v) { app->getMemory<double>(a) = v; };
    esp -= 4; wr32(esp, ebx);  // 4a2e20 push ebx
    esp -= 4; wr32(esp, ecx);  // 4a2e21 push ecx
    esp -= 4; wr32(esp, ebp);  // 4a2e22 push ebp
    ebp = esp;  // 4a2e23 mov ebp, esp
    esp -= 44u;  // 4a2e25 sub esp, 0x2c
    ecx = eax;  // 4a2e28 mov ecx, eax
    wr32((ebp - 8u), edx);  // 4a2e2a mov dword ptr [ebp - 8], edx
    fl.sub(rd8((eax + 0x89u)), 0u);  // 4a2e2d cmp byte ptr [eax + 0x89], 0
    if (fl.e()) goto L_4a2efe;  // 4a2e34 je 0x4a2efe
    fl.sub(rd8((ecx + 0x8au)), 0u);  // 4a2e3a cmp byte ptr [ecx + 0x8a], 0
    if (fl.ne()) goto L_4a2efe;  // 4a2e41 jne 0x4a2efe
    fl.sub(rd32((0x7d3684u)), 512u);  // 4a2e47 cmp dword ptr [0x7d3684], 0x200
    if (fl.l()) goto L_4a2efe;  // 4a2e51 jl 0x4a2efe
    eax = (ecx + 0x94u);  // 4a2e57 lea eax, [ecx + 0x94]
    esp -= 4; wr32(esp, eax);  // 4a2e5d push eax
    eax = rd32((ebp - 8u));  // 4a2e5e mov eax, dword ptr [ebp - 8]
    wr32((ebp - 4u), eax);  // 4a2e61 mov dword ptr [ebp - 4], eax
    x.push(F(ecx + 0xecu));  // 4a2e64 fld dword ptr [ecx + 0xec]
    x.push(double(int32_t(rd32((ebp - 4u)))));  // 4a2e6a fild dword ptr [ebp - 4]
    x.push(1.0);  // 4a2e6d fld1
    x.st(1) = xdiv(cpu, x.st(0), x.st(1));  // 4a2e6f fdivrp st(1)
    x.pop();  // 4a2e6f fdivrp st(1)
    x.fxch(1);  // 4a2e71 fxch st(1)
    x.st(0) = xmul(cpu, x.st(0), x.st(1));  // 4a2e73 fmul st(1)
    esp -= 4u;  // 4a2e75 sub esp, 4
    SF((esp), x.st(0));  // 4a2e78 fstp dword ptr [esp]
    x.pop();  // 4a2e78 fstp dword ptr [esp]
    x.push(F(ecx + 0xe8u));  // 4a2e7b fld dword ptr [ecx + 0xe8]
    x.st(0) = xmul(cpu, x.st(0), x.st(1));  // 4a2e81 fmul st(1)
    esp -= 4u;  // 4a2e83 sub esp, 4
    SF((esp), x.st(0));  // 4a2e86 fstp dword ptr [esp]
    x.pop();  // 4a2e86 fstp dword ptr [esp]
    x.st(0) = xmul(cpu, x.st(0), F(ecx + 0xe4u));  // 4a2e89 fmul dword ptr [ecx + 0xe4]
    esp -= 4u;  // 4a2e8f sub esp, 4
    eax = (ebp - 0x2cu);  // 4a2e92 lea eax, [ebp - 0x2c]
    SF((esp), x.st(0));  // 4a2e95 fstp dword ptr [esp]
    x.pop();  // 4a2e95 fstp dword ptr [esp]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4a2cc0, esp, 4, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a2e98 call 0x4a2cc0
    esp += 16u;  // 4a2e98 call 0x4a2cc0
    fl.logic(eax & eax);  // 4a2e9d test eax, eax
    if (fl.e()) goto L_4a2ef7;  // 4a2e9f je 0x4a2ef7
    eax = (ecx + 0xc0u);  // 4a2ea1 lea eax, [ecx + 0xc0]
    edx = (ebp - 0x2cu);  // 4a2ea7 lea edx, [ebp - 0x2c]
    ebx = eax;  // 4a2eaa mov ebx, eax
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4e0280, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a2eac call 0x4e0280
    esp += 0u;  // 4a2eac call 0x4e0280
    x.push(F(ecx + 0x94u));  // 4a2eb1 fld dword ptr [ecx + 0x94]
    fl.fcmp(x.st(0), D(0x53c8f4u));  // 4a2eb7 fcomp qword ptr [0x53c8f4]
    x.pop();  // 4a2eb7 fcomp qword ptr [0x53c8f4]
    eax = (eax & 0xffff0000u) | fl.status();  // 4a2ebd fnstsw ax
    // sahf  // 4a2ebf sahf
    if (fl.a()) goto L_4a2ed4;  // 4a2ec0 ja 0x4a2ed4
    edx = (edx & 0xffff00ffu) | (((rd8((ecx + 0x8cu))) & 0xffu) << 8);  // 4a2ec2 mov dh, byte ptr [ecx + 0x8c]
    { const reg32 r_ = ((edx >> 8) & 0xffu) - 1u; edx = (edx & 0xffff00ffu) | (((r_) & 0xffu) << 8); fl.arith(r_); }  // 4a2ec8 dec dh
    wr8((ecx + 0x8cu), ((edx >> 8) & 0xffu));  // 4a2eca mov byte ptr [ecx + 0x8c], dh
    fl.logic(((edx >> 8) & 0xffu) & ((edx >> 8) & 0xffu));  // 4a2ed0 test dh, dh
    if (fl.a()) goto L_4a2ef7;  // 4a2ed2 ja 0x4a2ef7
L_4a2ed4:
    edx = 10u;  // 4a2ed4 mov edx, 0xa
    eax = (ecx + 0xc0u);  // 4a2ed9 lea eax, [ecx + 0xc0]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4b7840, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a2edf call 0x4b7840
    esp += 0u;  // 4a2edf call 0x4b7840
    eax = (eax & 0xffffff00u) | ((rd8((ebp - 8u))) & 0xffu);  // 4a2ee4 mov al, byte ptr [ebp - 8]
    wr32((ecx + 0x94u), 0u);  // 4a2ee7 mov dword ptr [ecx + 0x94], 0
    wr8((ecx + 0x8cu), (eax & 0xffu));  // 4a2ef1 mov byte ptr [ecx + 0x8c], al
L_4a2ef7:
    eax = ecx;  // 4a2ef7 mov eax, ecx
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4a2690, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 4a2ef9 call 0x4a2690
    esp += 0u;  // 4a2ef9 call 0x4a2690
L_4a2efe:
    esp = ebp;  // 4a2efe mov esp, ebp
    ebp = rd32(esp); esp += 4;  // 4a2f00 pop ebp
    ecx = rd32(esp); esp += 4;  // 4a2f01 pop ecx
    ebx = rd32(esp); esp += 4;  // 4a2f02 pop ebx
    for (int i_ = 0; i_ < x.n; ++i_) { cpu.fpu.count += 1; cpu.fpu.st(0) = x86::Float(x.v[i_]); }  // 4a2f03 ret
    result = eax; popped_ = 0u; return true;  // 4a2f03 ret
    return false;
}


// @@FUNCTIONS@@

}  // namespace

/* ---- The adapters ---- */

void fn49fe70(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 result, popped = 0;
    if (core49fe70(app, cpu, e, result, popped))
        e.back(cpu, result, popped);
}

void fn4a00d0(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 result, popped = 0;
    if (core4a00d0(app, cpu, e, result, popped))
        e.back(cpu, result, popped);
}

void fn4a0440(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 result, popped = 0;
    if (core4a0440(app, cpu, e, result, popped))
        e.back(cpu, result, popped);
}

void fn4a0980(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 result, popped = 0;
    if (core4a0980(app, cpu, e, result, popped))
        e.back(cpu, result, popped);
}

void fn4a0a10(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 result, popped = 0;
    if (core4a0a10(app, cpu, e, result, popped))
        e.back(cpu, result, popped);
}

void fn4a0d20(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 result, popped = 0;
    if (core4a0d20(app, cpu, e, result, popped))
        e.back(cpu, result, popped);
}

void fn4a0f30(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 result, popped = 0;
    if (core4a0f30(app, cpu, e, result, popped))
        e.back(cpu, result, popped);
}

void fn4a1090(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 result, popped = 0;
    if (core4a1090(app, cpu, e, result, popped))
        e.back(cpu, result, popped);
}

void fn4a20c0(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 result, popped = 0;
    if (core4a20c0(app, cpu, e, result, popped))
        e.back(cpu, result, popped);
}

void fn4a2630(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 result, popped = 0;
    if (core4a2630(app, cpu, e, result, popped))
        e.back(cpu, result, popped);
}

void fn4a2830(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 result, popped = 0;
    if (core4a2830(app, cpu, e, result, popped))
        e.back(cpu, result, popped);
}

void fn4a2c60(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 result, popped = 0;
    if (core4a2c60(app, cpu, e, result, popped))
        e.back(cpu, result, popped);
}

void fn4a2e20(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 result, popped = 0;
    if (core4a2e20(app, cpu, e, result, popped))
        e.back(cpu, result, popped);
}

// @@ADAPTERS@@

}  // namespace conv
}  // namespace port
}  // namespace nfs3hp
