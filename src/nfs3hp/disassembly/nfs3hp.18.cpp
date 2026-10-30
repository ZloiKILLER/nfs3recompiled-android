#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{
// Port (tools/apply_first_settings.py): defined in nfs3hp_main.cpp.
bool firstSettings(win32::WinApplication* app);

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_46e270(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 0046e270  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0046e271  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0046e272  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0046e273  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0046e274  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0046e276  83ec0c                 -sub esp, 0xc
    (cpu.esp) -= x86::reg32(x86::sreg32(12 /*0xc*/));
    // 0046e279  89df                   -mov edi, ebx
    cpu.edi = cpu.ebx;
    // 0046e27b  bbffffffff             -mov ebx, 0xffffffff
    cpu.ebx = 4294967295 /*0xffffffff*/;
    // 0046e280  ba17000000             -mov edx, 0x17
    cpu.edx = 23 /*0x17*/;
    // 0046e285  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0046e287  e8d4c3fdff             -call 0x44a660
    cpu.esp -= 4;
    nfs3hp::frontend::menuShownNow(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046e28c  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 0046e28e  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046e290  7507                   -jne 0x46e299
    if (!cpu.flags.zf)
    {
        goto L_0x0046e299;
    }
    // 0046e292  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 0046e294  e990030000             -jmp 0x46e629
    goto L_0x0046e629;
L_0x0046e299:
    // 0046e299  e8e20c0600             -call 0x4cef80
    cpu.esp -= 4;
    sub_4cef80(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046e29e  8b15307d6700           -mov edx, dword ptr [0x677d30]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(6782256) /* 0x677d30 */);
    // 0046e2a4  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 0046e2a7  83fa01                 +cmp edx, 1
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046e2aa  7e0b                   -jle 0x46e2b7
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0046e2b7;
    }
    // 0046e2ac  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046e2ae  7507                   -jne 0x46e2b7
    if (!cpu.flags.zf)
    {
        goto L_0x0046e2b7;
    }
    // 0046e2b0  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0046e2b5  eb02                   -jmp 0x46e2b9
    goto L_0x0046e2b9;
L_0x0046e2b7:
    // 0046e2b7  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x0046e2b9:
    // 0046e2b9  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 0046e2bc  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 0046e2be  eb06                   -jmp 0x46e2c6
    goto L_0x0046e2c6;
L_0x0046e2c0:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0046e2c0  42                     -inc edx
    (cpu.edx)++;
    // 0046e2c1  83fa04                 +cmp edx, 4
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046e2c4  7d1b                   -jge 0x46e2e1
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0046e2e1;
    }
L_0x0046e2c6:
    // 0046e2c6  8d04d500000000         -lea eax, [edx*8]
    cpu.eax = x86::reg32(cpu.edx * 8);
    // 0046e2cd  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
    // 0046e2d0  8d4148                 -lea eax, [ecx + 0x48]
    cpu.eax = x86::reg32(cpu.ecx + x86::reg32(72) /* 0x48 */);
    // 0046e2d3  0345f4                 -add eax, dword ptr [ebp - 0xc]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */)));
    // 0046e2d6  e815df0200             -call 0x49c1f0
    cpu.esp -= 4;
    nfs3hp::frontend::pointerInBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046e2db  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046e2dd  74e1                   -je 0x46e2c0
    if (cpu.flags.zf)
    {
        goto L_0x0046e2c0;
    }
    // 0046e2df  89d3                   -mov ebx, edx
    cpu.ebx = cpu.edx;
L_0x0046e2e1:
    // 0046e2e1  6683ff0d               +cmp di, 0xd
    {
        x86::reg16 tmp1 = cpu.di;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(13 /*0xd*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046e2e5  0f8581000000           -jne 0x46e36c
    if (!cpu.flags.zf)
    {
        goto L_0x0046e36c;
    }
    // 0046e2eb  837df800               +cmp dword ptr [ebp - 8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046e2ef  0f8477000000           -je 0x46e36c
    if (cpu.flags.zf)
    {
        goto L_0x0046e36c;
    }
    // 0046e2f5  83fb03                 +cmp ebx, 3
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046e2f8  7772                   -ja 0x46e36c
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x0046e36c;
    }
    // 0046e2fa  ff249d54e24600         -jmp dword ptr [ebx*4 + 0x46e254]
    cpu.ip = app->getMemory<x86::reg32>(4645460 + cpu.ebx * 4); goto dynamic_jump;
  case 0x0046e301:
    // 0046e301  ba7f000000             -mov edx, 0x7f
    cpu.edx = 127 /*0x7f*/;
    // 0046e306  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0046e30b  be02000000             -mov esi, 2
    cpu.esi = 2 /*0x2*/;
    // 0046e310  66c781860000000000     -mov word ptr [ecx + 0x86], 0
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(134) /* 0x86 */) = 0 /*0x0*/;
    // 0046e319  eb4c                   -jmp 0x46e367
    goto L_0x0046e367;
  case 0x0046e31b:
    // 0046e31b  ba7f000000             -mov edx, 0x7f
    cpu.edx = 127 /*0x7f*/;
    // 0046e320  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0046e325  be02000000             -mov esi, 2
    cpu.esi = 2 /*0x2*/;
    // 0046e32a  66c781860000000100     -mov word ptr [ecx + 0x86], 1
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(134) /* 0x86 */) = 1 /*0x1*/;
    // 0046e333  eb32                   -jmp 0x46e367
    goto L_0x0046e367;
  case 0x0046e335:
    // 0046e335  ba7f000000             -mov edx, 0x7f
    cpu.edx = 127 /*0x7f*/;
    // 0046e33a  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0046e33f  be02000000             -mov esi, 2
    cpu.esi = 2 /*0x2*/;
    // 0046e344  66c781860000000200     -mov word ptr [ecx + 0x86], 2
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(134) /* 0x86 */) = 2 /*0x2*/;
    // 0046e34d  eb18                   -jmp 0x46e367
    goto L_0x0046e367;
  case 0x0046e34f:
    // 0046e34f  ba7f000000             -mov edx, 0x7f
    cpu.edx = 127 /*0x7f*/;
    // 0046e354  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0046e359  be02000000             -mov esi, 2
    cpu.esi = 2 /*0x2*/;
    // 0046e35e  66c781860000000300     -mov word ptr [ecx + 0x86], 3
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(134) /* 0x86 */) = 3 /*0x3*/;
L_0x0046e367:
    // 0046e367  e8649efaff             -call 0x4181d0
    cpu.esp -= 4;
    nfs3hp::port::playMenuEffect(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0046e36c:
    // 0046e36c  837df800               +cmp dword ptr [ebp - 8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046e370  0f84f6000000           -je 0x46e46c
    if (cpu.flags.zf)
    {
        goto L_0x0046e46c;
    }
    // 0046e376  6681ff004d             +cmp di, 0x4d00
    {
        x86::reg16 tmp1 = cpu.di;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(19712 /*0x4d00*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046e37b  0f8577000000           -jne 0x46e3f8
    if (!cpu.flags.zf)
    {
        goto L_0x0046e3f8;
    }
    // 0046e381  43                     -inc ebx
    (cpu.ebx)++;
    // 0046e382  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0046e384  7d04                   -jge 0x46e38a
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0046e38a;
    }
    // 0046e386  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0046e388  eb0e                   -jmp 0x46e398
    goto L_0x0046e398;
L_0x0046e38a:
    // 0046e38a  83fb03                 +cmp ebx, 3
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046e38d  7e07                   -jle 0x46e396
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0046e396;
    }
    // 0046e38f  b803000000             -mov eax, 3
    cpu.eax = 3 /*0x3*/;
    // 0046e394  eb02                   -jmp 0x46e398
    goto L_0x0046e398;
L_0x0046e396:
    // 0046e396  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
L_0x0046e398:
    // 0046e398  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0046e39a  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 0046e39d  8d3401                 -lea esi, [ecx + eax]
    cpu.esi = x86::reg32(cpu.ecx + cpu.eax * 1);
    // 0046e3a0  8b464a                 -mov eax, dword ptr [esi + 0x4a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(74) /* 0x4a */);
    // 0046e3a3  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046e3a6  8d148500000000         -lea edx, [eax*4]
    cpu.edx = x86::reg32(cpu.eax * 4);
    // 0046e3ad  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 0046e3af  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046e3b1  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046e3b4  c1e202                 +shl edx, 2
    {
        x86::reg8 tmp = 2 /*0x2*/ % 32;
        x86::reg32& op = cpu.edx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 0046e3b7  1bc2                   -sbb eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 0046e3b9  c1f802                 -sar eax, 2
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (2 /*0x2*/ % 32));
    // 0046e3bc  8b5648                 -mov edx, dword ptr [esi + 0x48]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(72) /* 0x48 */);
    // 0046e3bf  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046e3c2  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046e3c4  8915e4e55500           -mov dword ptr [0x55e5e4], edx
    app->getMemory<x86::reg32>(x86::reg32(5629412) /* 0x55e5e4 */) = cpu.edx;
    // 0046e3ca  8b564c                 -mov edx, dword ptr [esi + 0x4c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(76) /* 0x4c */);
    // 0046e3cd  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046e3d0  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046e3d2  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046e3d5  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0046e3d7  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0046e3d9  8b7646                 -mov esi, dword ptr [esi + 0x46]
    cpu.esi = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(70) /* 0x46 */);
    // 0046e3dc  c1fe10                 -sar esi, 0x10
    cpu.esi = x86::reg32(x86::sreg32(cpu.esi) >> (16 /*0x10*/ % 32));
    // 0046e3df  01c6                   -add esi, eax
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046e3e1  a1e4e55500             -mov eax, dword ptr [0x55e5e4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629412) /* 0x55e5e4 */);
    // 0046e3e6  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0046e3e8  8935e8e55500           -mov dword ptr [0x55e5e8], esi
    app->getMemory<x86::reg32>(x86::reg32(5629416) /* 0x55e5e8 */) = cpu.esi;
    // 0046e3ee  e8cddb0200             -call 0x49bfc0
    cpu.esp -= 4;
    nfs3hp::frontend::setPointer(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046e3f3  be02000000             -mov esi, 2
    cpu.esi = 2 /*0x2*/;
L_0x0046e3f8:
    // 0046e3f8  6681ff004b             +cmp di, 0x4b00
    {
        x86::reg16 tmp1 = cpu.di;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(19200 /*0x4b00*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046e3fd  756d                   -jne 0x46e46c
    if (!cpu.flags.zf)
    {
        goto L_0x0046e46c;
    }
    // 0046e3ff  4b                     -dec ebx
    (cpu.ebx)--;
    // 0046e400  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0046e402  7d04                   -jge 0x46e408
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0046e408;
    }
    // 0046e404  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 0046e406  eb0a                   -jmp 0x46e412
    goto L_0x0046e412;
L_0x0046e408:
    // 0046e408  83fb03                 +cmp ebx, 3
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046e40b  7e05                   -jle 0x46e412
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0046e412;
    }
    // 0046e40d  bb03000000             -mov ebx, 3
    cpu.ebx = 3 /*0x3*/;
L_0x0046e412:
    // 0046e412  8b44d94a               -mov eax, dword ptr [ecx + ebx*8 + 0x4a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(74) /* 0x4a */ + cpu.ebx * 8);
    // 0046e416  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046e419  8d148500000000         -lea edx, [eax*4]
    cpu.edx = x86::reg32(cpu.eax * 4);
    // 0046e420  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 0046e422  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046e424  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046e427  c1e202                 +shl edx, 2
    {
        x86::reg8 tmp = 2 /*0x2*/ % 32;
        x86::reg32& op = cpu.edx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 0046e42a  1bc2                   -sbb eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 0046e42c  c1f802                 -sar eax, 2
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (2 /*0x2*/ % 32));
    // 0046e42f  8b54d948               -mov edx, dword ptr [ecx + ebx*8 + 0x48]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(72) /* 0x48 */ + cpu.ebx * 8);
    // 0046e433  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046e436  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046e438  8915e4e55500           -mov dword ptr [0x55e5e4], edx
    app->getMemory<x86::reg32>(x86::reg32(5629412) /* 0x55e5e4 */) = cpu.edx;
    // 0046e43e  8b54d94c               -mov edx, dword ptr [ecx + ebx*8 + 0x4c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(76) /* 0x4c */ + cpu.ebx * 8);
    // 0046e442  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046e445  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046e447  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046e44a  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0046e44c  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0046e44e  8b54d946               -mov edx, dword ptr [ecx + ebx*8 + 0x46]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(70) /* 0x46 */ + cpu.ebx * 8);
    // 0046e452  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046e455  be02000000             -mov esi, 2
    cpu.esi = 2 /*0x2*/;
    // 0046e45a  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046e45c  a1e4e55500             -mov eax, dword ptr [0x55e5e4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629412) /* 0x55e5e4 */);
    // 0046e461  8915e8e55500           -mov dword ptr [0x55e5e8], edx
    app->getMemory<x86::reg32>(x86::reg32(5629416) /* 0x55e5e8 */) = cpu.edx;
    // 0046e467  e854db0200             -call 0x49bfc0
    cpu.esp -= 4;
    nfs3hp::frontend::setPointer(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0046e46c:
    // 0046e46c  8b4144                 -mov eax, dword ptr [ecx + 0x44]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(68) /* 0x44 */);
    // 0046e46f  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046e472  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046e475  39d0                   +cmp eax, edx
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046e477  0f8daa010000           -jge 0x46e627
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0046e627;
    }
    // 0046e47d  6681ff0049             +cmp di, 0x4900
    {
        x86::reg16 tmp1 = cpu.di;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(18688 /*0x4900*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046e482  0f857e000000           -jne 0x46e506
    if (!cpu.flags.zf)
    {
        goto L_0x0046e506;
    }
    // 0046e488  6683b98400000000       +cmp word ptr [ecx + 0x84], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(132) /* 0x84 */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046e490  7474                   -je 0x46e506
    if (cpu.flags.zf)
    {
        goto L_0x0046e506;
    }
    // 0046e492  ba7f000000             -mov edx, 0x7f
    cpu.edx = 127 /*0x7f*/;
    // 0046e497  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0046e49c  e82f9dfaff             -call 0x4181d0
    cpu.esp -= 4;
    nfs3hp::port::playMenuEffect(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046e4a1  8b416a                 -mov eax, dword ptr [ecx + 0x6a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(106) /* 0x6a */);
    // 0046e4a4  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046e4a7  8d148500000000         -lea edx, [eax*4]
    cpu.edx = x86::reg32(cpu.eax * 4);
    // 0046e4ae  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 0046e4b0  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046e4b2  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046e4b5  c1e202                 +shl edx, 2
    {
        x86::reg8 tmp = 2 /*0x2*/ % 32;
        x86::reg32& op = cpu.edx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 0046e4b8  1bc2                   -sbb eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 0046e4ba  c1f802                 -sar eax, 2
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (2 /*0x2*/ % 32));
    // 0046e4bd  8b5168                 -mov edx, dword ptr [ecx + 0x68]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(104) /* 0x68 */);
    // 0046e4c0  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046e4c3  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046e4c5  8915e4e55500           -mov dword ptr [0x55e5e4], edx
    app->getMemory<x86::reg32>(x86::reg32(5629412) /* 0x55e5e4 */) = cpu.edx;
    // 0046e4cb  8b516c                 -mov edx, dword ptr [ecx + 0x6c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(108) /* 0x6c */);
    // 0046e4ce  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046e4d1  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046e4d3  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046e4d6  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0046e4d8  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0046e4da  be02000000             -mov esi, 2
    cpu.esi = 2 /*0x2*/;
    // 0046e4df  668b9984000000         -mov bx, word ptr [ecx + 0x84]
    cpu.bx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(132) /* 0x84 */);
    // 0046e4e6  8b5166                 -mov edx, dword ptr [ecx + 0x66]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(102) /* 0x66 */);
    // 0046e4e9  4b                     -dec ebx
    (cpu.ebx)--;
    // 0046e4ea  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046e4ed  66899984000000         -mov word ptr [ecx + 0x84], bx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(132) /* 0x84 */) = cpu.bx;
    // 0046e4f4  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046e4f6  a1e4e55500             -mov eax, dword ptr [0x55e5e4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629412) /* 0x55e5e4 */);
    // 0046e4fb  8915e8e55500           -mov dword ptr [0x55e5e8], edx
    app->getMemory<x86::reg32>(x86::reg32(5629416) /* 0x55e5e8 */) = cpu.edx;
    // 0046e501  e8bada0200             -call 0x49bfc0
    cpu.esp -= 4;
    nfs3hp::frontend::setPointer(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0046e506:
    // 0046e506  6681ff0051             +cmp di, 0x5100
    {
        x86::reg16 tmp1 = cpu.di;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(20736 /*0x5100*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046e50b  0f858c000000           -jne 0x46e59d
    if (!cpu.flags.zf)
    {
        goto L_0x0046e59d;
    }
    // 0046e511  8b5144                 -mov edx, dword ptr [ecx + 0x44]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(68) /* 0x44 */);
    // 0046e514  8b8182000000           -mov eax, dword ptr [ecx + 0x82]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(130) /* 0x82 */);
    // 0046e51a  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046e51d  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046e520  8b5dfc                 -mov ebx, dword ptr [ebp - 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046e523  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 0046e525  39d8                   +cmp eax, ebx
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046e527  7d74                   -jge 0x46e59d
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0046e59d;
    }
    // 0046e529  ba7f000000             -mov edx, 0x7f
    cpu.edx = 127 /*0x7f*/;
    // 0046e52e  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0046e533  e8989cfaff             -call 0x4181d0
    cpu.esp -= 4;
    nfs3hp::port::playMenuEffect(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046e538  8b4172                 -mov eax, dword ptr [ecx + 0x72]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(114) /* 0x72 */);
    // 0046e53b  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046e53e  8d148500000000         -lea edx, [eax*4]
    cpu.edx = x86::reg32(cpu.eax * 4);
    // 0046e545  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 0046e547  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046e549  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046e54c  c1e202                 +shl edx, 2
    {
        x86::reg8 tmp = 2 /*0x2*/ % 32;
        x86::reg32& op = cpu.edx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 0046e54f  1bc2                   -sbb eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 0046e551  c1f802                 -sar eax, 2
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (2 /*0x2*/ % 32));
    // 0046e554  8b5170                 -mov edx, dword ptr [ecx + 0x70]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(112) /* 0x70 */);
    // 0046e557  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046e55a  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046e55c  8915e4e55500           -mov dword ptr [0x55e5e4], edx
    app->getMemory<x86::reg32>(x86::reg32(5629412) /* 0x55e5e4 */) = cpu.edx;
    // 0046e562  8b5174                 -mov edx, dword ptr [ecx + 0x74]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(116) /* 0x74 */);
    // 0046e565  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046e568  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046e56a  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046e56d  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0046e56f  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0046e571  668bb184000000         -mov si, word ptr [ecx + 0x84]
    cpu.si = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(132) /* 0x84 */);
    // 0046e578  46                     -inc esi
    (cpu.esi)++;
    // 0046e579  8b516e                 -mov edx, dword ptr [ecx + 0x6e]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(110) /* 0x6e */);
    // 0046e57c  6689b184000000         -mov word ptr [ecx + 0x84], si
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(132) /* 0x84 */) = cpu.si;
    // 0046e583  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046e586  be02000000             -mov esi, 2
    cpu.esi = 2 /*0x2*/;
    // 0046e58b  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046e58d  a1e4e55500             -mov eax, dword ptr [0x55e5e4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629412) /* 0x55e5e4 */);
    // 0046e592  8915e8e55500           -mov dword ptr [0x55e5e8], edx
    app->getMemory<x86::reg32>(x86::reg32(5629416) /* 0x55e5e8 */) = cpu.edx;
    // 0046e598  e823da0200             -call 0x49bfc0
    cpu.esp -= 4;
    nfs3hp::frontend::setPointer(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0046e59d:
    // 0046e59d  6683ff0d               +cmp di, 0xd
    {
        x86::reg16 tmp1 = cpu.di;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(13 /*0xd*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046e5a1  0f8580000000           -jne 0x46e627
    if (!cpu.flags.zf)
    {
        goto L_0x0046e627;
    }
    // 0046e5a7  8d4168                 -lea eax, [ecx + 0x68]
    cpu.eax = x86::reg32(cpu.ecx + x86::reg32(104) /* 0x68 */);
    // 0046e5aa  e841dc0200             -call 0x49c1f0
    cpu.esp -= 4;
    nfs3hp::frontend::pointerInBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046e5af  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046e5b1  742d                   -je 0x46e5e0
    if (cpu.flags.zf)
    {
        goto L_0x0046e5e0;
    }
    // 0046e5b3  6683b98400000000       +cmp word ptr [ecx + 0x84], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(132) /* 0x84 */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046e5bb  7423                   -je 0x46e5e0
    if (cpu.flags.zf)
    {
        goto L_0x0046e5e0;
    }
    // 0046e5bd  ba7f000000             -mov edx, 0x7f
    cpu.edx = 127 /*0x7f*/;
    // 0046e5c2  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0046e5c7  e8049cfaff             -call 0x4181d0
    cpu.esp -= 4;
    nfs3hp::port::playMenuEffect(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046e5cc  668b8184000000         -mov ax, word ptr [ecx + 0x84]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(132) /* 0x84 */);
    // 0046e5d3  48                     -dec eax
    (cpu.eax)--;
    // 0046e5d4  be02000000             -mov esi, 2
    cpu.esi = 2 /*0x2*/;
    // 0046e5d9  66898184000000         -mov word ptr [ecx + 0x84], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(132) /* 0x84 */) = cpu.ax;
L_0x0046e5e0:
    // 0046e5e0  8d4170                 -lea eax, [ecx + 0x70]
    cpu.eax = x86::reg32(cpu.ecx + x86::reg32(112) /* 0x70 */);
    // 0046e5e3  e808dc0200             -call 0x49c1f0
    cpu.esp -= 4;
    nfs3hp::frontend::pointerInBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046e5e8  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046e5ea  743b                   -je 0x46e627
    if (cpu.flags.zf)
    {
        goto L_0x0046e627;
    }
    // 0046e5ec  8b4144                 -mov eax, dword ptr [ecx + 0x44]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(68) /* 0x44 */);
    // 0046e5ef  8b9182000000           -mov edx, dword ptr [ecx + 0x82]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(130) /* 0x82 */);
    // 0046e5f5  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046e5f8  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046e5fb  8b7dfc                 -mov edi, dword ptr [ebp - 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046e5fe  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 0046e600  39f8                   +cmp eax, edi
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edi));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046e602  7d23                   -jge 0x46e627
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0046e627;
    }
    // 0046e604  ba7f000000             -mov edx, 0x7f
    cpu.edx = 127 /*0x7f*/;
    // 0046e609  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0046e60e  e8bd9bfaff             -call 0x4181d0
    cpu.esp -= 4;
    nfs3hp::port::playMenuEffect(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046e613  668b9184000000         -mov dx, word ptr [ecx + 0x84]
    cpu.dx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(132) /* 0x84 */);
    // 0046e61a  42                     -inc edx
    (cpu.edx)++;
    // 0046e61b  be02000000             -mov esi, 2
    cpu.esi = 2 /*0x2*/;
    // 0046e620  66899184000000         -mov word ptr [ecx + 0x84], dx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(132) /* 0x84 */) = cpu.dx;
L_0x0046e627:
    // 0046e627  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
L_0x0046e629:
    // 0046e629  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046e62b  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0046e62d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046e62e  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046e62f  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046e630  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046e631  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_46e640(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0046e640  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0046e641  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0046e642  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0046e644  ba17000000             -mov edx, 0x17
    cpu.edx = 23 /*0x17*/;
    // 0046e649  e812c0fdff             -call 0x44a660
    cpu.esp -= 4;
    nfs3hp::frontend::menuShownNow(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046e64e  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046e650  7402                   -je 0x46e654
    if (cpu.flags.zf)
    {
        goto L_0x0046e654;
    }
    // 0046e652  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x0046e654:
    // 0046e654  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046e655  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046e656  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_46e660(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0046e660  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0046e661  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0046e662  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0046e663  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0046e665  8b5074                 -mov edx, dword ptr [eax + 0x74]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(116) /* 0x74 */);
    // 0046e668  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046e66b  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0046e66c  8b507a                 -mov edx, dword ptr [eax + 0x7a]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(122) /* 0x7a */);
    // 0046e66f  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046e672  8b4cd04a               -mov ecx, dword ptr [eax + edx*8 + 0x4a]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(74) /* 0x4a */ + cpu.edx * 8);
    // 0046e676  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0046e677  8b5cd046               -mov ebx, dword ptr [eax + edx*8 + 0x46]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(70) /* 0x46 */ + cpu.edx * 8);
    // 0046e67b  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0046e67c  e85fe6ffff             -call 0x46cce0
    cpu.esp -= 4;
    sub_46cce0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046e681  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046e682  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046e683  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046e684  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_46e690(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0046e690  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0046e691  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0046e692  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0046e693  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0046e694  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0046e695  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0046e697  81ec70020000           -sub esp, 0x270
    (cpu.esp) -= x86::reg32(x86::sreg32(624 /*0x270*/));
    // 0046e69d  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0046e69f  8955ac                 -mov dword ptr [ebp - 0x54], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-84) /* -0x54 */) = cpu.edx;
    // 0046e6a2  ba19000000             -mov edx, 0x19
    cpu.edx = 25 /*0x19*/;
    // 0046e6a7  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0046e6aa  e8b1bffdff             -call 0x44a660
    cpu.esp -= 4;
    nfs3hp::frontend::menuShownNow(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046e6af  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046e6b1  0f84be080000           -je 0x46ef75
    if (cpu.flags.zf)
    {
        goto L_0x0046ef75;
    }
    // 0046e6b7  66837e7e02             +cmp word ptr [esi + 0x7e], 2
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.esi + x86::reg32(126) /* 0x7e */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(2 /*0x2*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046e6bc  7527                   -jne 0x46e6e5
    if (!cpu.flags.zf)
    {
        goto L_0x0046e6e5;
    }
    // 0046e6be  668b9e80000000         -mov bx, word ptr [esi + 0x80]
    cpu.bx = app->getMemory<x86::reg16>(cpu.esi + x86::reg32(128) /* 0x80 */);
    // 0046e6c5  83c314                 -add ebx, 0x14
    (cpu.ebx) += x86::reg32(x86::sreg32(20 /*0x14*/));
    // 0046e6c8  66899e80000000         -mov word ptr [esi + 0x80], bx
    app->getMemory<x86::reg16>(cpu.esi + x86::reg32(128) /* 0x80 */) = cpu.bx;
    // 0046e6cf  6681fba802             +cmp bx, 0x2a8
    {
        x86::reg16 tmp1 = cpu.bx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(680 /*0x2a8*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046e6d4  7c0f                   -jl 0x46e6e5
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0046e6e5;
    }
    // 0046e6d6  66c78680000000a802     -mov word ptr [esi + 0x80], 0x2a8
    app->getMemory<x86::reg16>(cpu.esi + x86::reg32(128) /* 0x80 */) = 680 /*0x2a8*/;
    // 0046e6df  66c7467e0300           -mov word ptr [esi + 0x7e], 3
    app->getMemory<x86::reg16>(cpu.esi + x86::reg32(126) /* 0x7e */) = 3 /*0x3*/;
L_0x0046e6e5:
    // 0046e6e5  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046e6e8  6683787e01             +cmp word ptr [eax + 0x7e], 1
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(126) /* 0x7e */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(1 /*0x1*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046e6ed  752f                   -jne 0x46e71e
    if (!cpu.flags.zf)
    {
        goto L_0x0046e71e;
    }
    // 0046e6ef  6683a88000000014       -sub word ptr [eax + 0x80], 0x14
    (app->getMemory<x86::reg16>(cpu.eax + x86::reg32(128) /* 0x80 */)) -= x86::reg16(x86::sreg16(20 /*0x14*/));
    // 0046e6f7  8b507e                 -mov edx, dword ptr [eax + 0x7e]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(126) /* 0x7e */);
    // 0046e6fa  8b4004                 -mov eax, dword ptr [eax + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 0046e6fd  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046e700  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046e703  39c2                   +cmp edx, eax
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046e705  7f17                   -jg 0x46e71e
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x0046e71e;
    }
    // 0046e707  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046e70a  66c7407e0000           -mov word ptr [eax + 0x7e], 0
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(126) /* 0x7e */) = 0 /*0x0*/;
    // 0046e710  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046e713  668b4006               -mov ax, word ptr [eax + 6]
    cpu.ax = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(6) /* 0x6 */);
    // 0046e717  66898280000000         -mov word ptr [edx + 0x80], ax
    app->getMemory<x86::reg16>(cpu.edx + x86::reg32(128) /* 0x80 */) = cpu.ax;
L_0x0046e71e:
    // 0046e71e  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046e721  6683787e03             +cmp word ptr [eax + 0x7e], 3
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(126) /* 0x7e */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(3 /*0x3*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046e726  0f8449080000           -je 0x46ef75
    if (cpu.flags.zf)
    {
        goto L_0x0046ef75;
    }
    // 0046e72c  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0046e72e  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046e731  e88a090000             -call 0x46f0c0
    cpu.esp -= 4;
    sub_46f0c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046e736  e8b5b6fdff             -call 0x449df0
    cpu.esp -= 4;
    nfs3hp::port::loop::keysTaken(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046e73b  8945d8                 -mov dword ptr [ebp - 0x28], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-40) /* -0x28 */) = cpu.eax;
    // 0046e73e  e83d080600             -call 0x4cef80
    cpu.esp -= 4;
    sub_4cef80(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046e743  668b527c               -mov dx, word ptr [edx + 0x7c]
    cpu.dx = app->getMemory<x86::reg16>(cpu.edx + x86::reg32(124) /* 0x7c */);
    // 0046e747  8945c8                 -mov dword ptr [ebp - 0x38], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-56) /* -0x38 */) = cpu.eax;
    // 0046e74a  6683fa01               +cmp dx, 1
    {
        x86::reg16 tmp1 = cpu.dx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(1 /*0x1*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046e74e  720a                   -jb 0x46e75a
    if (cpu.flags.cf)
    {
        goto L_0x0046e75a;
    }
    // 0046e750  7622                   -jbe 0x46e774
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0046e774;
    }
    // 0046e752  6683fa02               +cmp dx, 2
    {
        x86::reg16 tmp1 = cpu.dx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(2 /*0x2*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046e756  7410                   -je 0x46e768
    if (cpu.flags.zf)
    {
        goto L_0x0046e768;
    }
    // 0046e758  eb2b                   -jmp 0x46e785
    goto L_0x0046e785;
L_0x0046e75a:
    // 0046e75a  6685d2                 +test dx, dx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg16>(cpu.dx & cpu.dx));
    // 0046e75d  7526                   -jne 0x46e785
    if (!cpu.flags.zf)
    {
        goto L_0x0046e785;
    }
    // 0046e75f  ba08000000             -mov edx, 8
    cpu.edx = 8 /*0x8*/;
    // 0046e764  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 0046e766  eb16                   -jmp 0x46e77e
    goto L_0x0046e77e;
L_0x0046e768:
    // 0046e768  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 0046e76d  ba04000000             -mov edx, 4
    cpu.edx = 4 /*0x4*/;
    // 0046e772  eb0a                   -jmp 0x46e77e
    goto L_0x0046e77e;
L_0x0046e774:
    // 0046e774  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 0046e779  ba03000000             -mov edx, 3
    cpu.edx = 3 /*0x3*/;
L_0x0046e77e:
    // 0046e77e  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0046e780  e88b74feff             -call 0x455c10
    cpu.esp -= 4;
    sub_455c10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0046e785:
    // 0046e785  a1307d6700             -mov eax, dword ptr [0x677d30]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6782256) /* 0x677d30 */);
    // 0046e78a  8945ec                 -mov dword ptr [ebp - 0x14], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.eax;
    // 0046e78d  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046e790  8b4042                 -mov eax, dword ptr [eax + 0x42]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(66) /* 0x42 */);
    // 0046e793  8b55ec                 -mov edx, dword ptr [ebp - 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0046e796  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046e799  39d0                   +cmp eax, edx
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046e79b  7c02                   -jl 0x46e79f
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0046e79f;
    }
    // 0046e79d  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
L_0x0046e79f:
    // 0046e79f  8b75fc                 -mov esi, dword ptr [ebp - 4]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046e7a2  8945c0                 -mov dword ptr [ebp - 0x40], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-64) /* -0x40 */) = cpu.eax;
    // 0046e7a5  8b7640                 -mov esi, dword ptr [esi + 0x40]
    cpu.esi = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(64) /* 0x40 */);
    // 0046e7a8  40                     -inc eax
    (cpu.eax)++;
    // 0046e7a9  c1fe10                 -sar esi, 0x10
    cpu.esi = x86::reg32(x86::sreg32(cpu.esi) >> (16 /*0x10*/ % 32));
    // 0046e7ac  0faff0                 -imul esi, eax
    cpu.esi = x86::reg32(x86::sreg64(x86::sreg32(cpu.esi)) * x86::sreg64(x86::sreg32(cpu.eax)));
    // 0046e7af  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046e7b2  8b7dfc                 -mov edi, dword ptr [ebp - 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046e7b5  bb2894d5ff             -mov ebx, 0xffd59428
    cpu.ebx = 4292187176 /*0xffd59428*/;
    // 0046e7ba  68000000ff             -push 0xff000000
    app->getMemory<x86::reg32>(cpu.esp-4) = 4278190080 /*0xff000000*/;
    cpu.esp -= 4;
    // 0046e7bf  895de8                 -mov dword ptr [ebp - 0x18], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = cpu.ebx;
    // 0046e7c2  8b4a3a                 -mov ecx, dword ptr [edx + 0x3a]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(58) /* 0x3a */);
    // 0046e7c5  8b7f3e                 -mov edi, dword ptr [edi + 0x3e]
    cpu.edi = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(62) /* 0x3e */);
    // 0046e7c8  8b523c                 -mov edx, dword ptr [edx + 0x3c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(60) /* 0x3c */);
    // 0046e7cb  c1f910                 -sar ecx, 0x10
    cpu.ecx = x86::reg32(x86::sreg32(cpu.ecx) >> (16 /*0x10*/ % 32));
    // 0046e7ce  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046e7d1  c1ff10                 -sar edi, 0x10
    cpu.edi = x86::reg32(x86::sreg32(cpu.edi) >> (16 /*0x10*/ % 32));
    // 0046e7d4  01ca                   -add edx, ecx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.ecx));
    // 0046e7d6  b9aa0000ff             -mov ecx, 0xff0000aa
    cpu.ecx = 4278190250 /*0xff0000aa*/;
    // 0046e7db  01d7                   -add edi, edx
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.edx));
    // 0046e7dd  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046e7e0  894de4                 -mov dword ptr [ebp - 0x1c], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */) = cpu.ecx;
    // 0046e7e3  8d5f02                 -lea ebx, [edi + 2]
    cpu.ebx = x86::reg32(cpu.edi + x86::reg32(2) /* 0x2 */);
    // 0046e7e6  8d4602                 -lea eax, [esi + 2]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(2) /* 0x2 */);
    // 0046e7e9  8b5206                 -mov edx, dword ptr [edx + 6]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(6) /* 0x6 */);
    // 0046e7ec  8945a8                 -mov dword ptr [ebp - 0x58], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-88) /* -0x58 */) = cpu.eax;
    // 0046e7ef  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046e7f2  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046e7f5  8b407e                 -mov eax, dword ptr [eax + 0x7e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(126) /* 0x7e */);
    // 0046e7f8  8b4da8                 -mov ecx, dword ptr [ebp - 0x58]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-88) /* -0x58 */);
    // 0046e7fb  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046e7fe  e82da10600             -call 0x4d8930
    cpu.esp -= 4;
    nfs3hp::port::draw2d::frame(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046e803  68aa0000ff             -push 0xff0000aa
    app->getMemory<x86::reg32>(cpu.esp-4) = 4278190250 /*0xff0000aa*/;
    cpu.esp -= 4;
    // 0046e808  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046e80b  b902000000             -mov ecx, 2
    cpu.ecx = 2 /*0x2*/;
    // 0046e810  8d4704                 -lea eax, [edi + 4]
    cpu.eax = x86::reg32(cpu.edi + x86::reg32(4) /* 0x4 */);
    // 0046e813  682894d5ff             -push 0xffd59428
    app->getMemory<x86::reg32>(cpu.esp-4) = 4292187176 /*0xffd59428*/;
    cpu.esp -= 4;
    // 0046e818  8945a8                 -mov dword ptr [ebp - 0x58], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-88) /* -0x58 */) = cpu.eax;
    // 0046e81b  8b5206                 -mov edx, dword ptr [edx + 6]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(6) /* 0x6 */);
    // 0046e81e  682894d5ff             -push 0xffd59428
    app->getMemory<x86::reg32>(cpu.esp-4) = 4292187176 /*0xffd59428*/;
    cpu.esp -= 4;
    // 0046e823  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046e826  8b5da8                 -mov ebx, dword ptr [ebp - 0x58]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-88) /* -0x58 */);
    // 0046e829  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046e82c  8b407e                 -mov eax, dword ptr [eax + 0x7e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(126) /* 0x7e */);
    // 0046e82f  68aa0000ff             -push 0xff0000aa
    app->getMemory<x86::reg32>(cpu.esp-4) = 4278190250 /*0xff0000aa*/;
    cpu.esp -= 4;
    // 0046e834  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046e837  83ea02                 -sub edx, 2
    (cpu.edx) -= x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0046e83a  83e802                 -sub eax, 2
    (cpu.eax) -= x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0046e83d  e81ea00600             -call 0x4d8860
    cpu.esp -= 4;
    nfs3hp::port::draw2d::drawShadedBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046e842  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046e845  8b4006                 -mov eax, dword ptr [eax + 6]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(6) /* 0x6 */);
    // 0046e848  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046e84b  01f0                   -add eax, esi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.esi));
    // 0046e84d  682894d5ff             -push 0xffd59428
    app->getMemory<x86::reg32>(cpu.esp-4) = 4292187176 /*0xffd59428*/;
    cpu.esp -= 4;
    // 0046e852  8d5002                 -lea edx, [eax + 2]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(2) /* 0x2 */);
    // 0046e855  682894d5ff             -push 0xffd59428
    app->getMemory<x86::reg32>(cpu.esp-4) = 4292187176 /*0xffd59428*/;
    cpu.esp -= 4;
    // 0046e85a  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046e85d  682894d5ff             -push 0xffd59428
    app->getMemory<x86::reg32>(cpu.esp-4) = 4292187176 /*0xffd59428*/;
    cpu.esp -= 4;
    // 0046e862  b902000000             -mov ecx, 2
    cpu.ecx = 2 /*0x2*/;
    // 0046e867  8b407e                 -mov eax, dword ptr [eax + 0x7e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(126) /* 0x7e */);
    // 0046e86a  682894d5ff             -push 0xffd59428
    app->getMemory<x86::reg32>(cpu.esp-4) = 4292187176 /*0xffd59428*/;
    cpu.esp -= 4;
    // 0046e86f  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046e872  8b5da8                 -mov ebx, dword ptr [ebp - 0x58]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-88) /* -0x58 */);
    // 0046e875  83e802                 -sub eax, 2
    (cpu.eax) -= x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0046e878  e8e39f0600             -call 0x4d8860
    cpu.esp -= 4;
    nfs3hp::port::draw2d::drawShadedBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046e87d  682894d5ff             -push 0xffd59428
    app->getMemory<x86::reg32>(cpu.esp-4) = 4292187176 /*0xffd59428*/;
    cpu.esp -= 4;
    // 0046e882  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046e885  bb02000000             -mov ebx, 2
    cpu.ebx = 2 /*0x2*/;
    // 0046e88a  8d4604                 -lea eax, [esi + 4]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 0046e88d  682894d5ff             -push 0xffd59428
    app->getMemory<x86::reg32>(cpu.esp-4) = 4292187176 /*0xffd59428*/;
    cpu.esp -= 4;
    // 0046e892  8945b8                 -mov dword ptr [ebp - 0x48], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-72) /* -0x48 */) = cpu.eax;
    // 0046e895  8b5206                 -mov edx, dword ptr [edx + 6]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(6) /* 0x6 */);
    // 0046e898  68aa0000ff             -push 0xff0000aa
    app->getMemory<x86::reg32>(cpu.esp-4) = 4278190250 /*0xff0000aa*/;
    cpu.esp -= 4;
    // 0046e89d  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046e8a0  8b4db8                 -mov ecx, dword ptr [ebp - 0x48]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-72) /* -0x48 */);
    // 0046e8a3  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046e8a6  8b407e                 -mov eax, dword ptr [eax + 0x7e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(126) /* 0x7e */);
    // 0046e8a9  68aa0000ff             -push 0xff0000aa
    app->getMemory<x86::reg32>(cpu.esp-4) = 4278190250 /*0xff0000aa*/;
    cpu.esp -= 4;
    // 0046e8ae  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046e8b1  83ea02                 -sub edx, 2
    (cpu.edx) -= x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0046e8b4  83e802                 -sub eax, 2
    (cpu.eax) -= x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0046e8b7  e8a49f0600             -call 0x4d8860
    cpu.esp -= 4;
    nfs3hp::port::draw2d::drawShadedBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046e8bc  682894d5ff             -push 0xffd59428
    app->getMemory<x86::reg32>(cpu.esp-4) = 4292187176 /*0xffd59428*/;
    cpu.esp -= 4;
    // 0046e8c1  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046e8c4  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046e8c7  8b4db8                 -mov ecx, dword ptr [ebp - 0x48]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-72) /* -0x48 */);
    // 0046e8ca  bb02000000             -mov ebx, 2
    cpu.ebx = 2 /*0x2*/;
    // 0046e8cf  682894d5ff             -push 0xffd59428
    app->getMemory<x86::reg32>(cpu.esp-4) = 4292187176 /*0xffd59428*/;
    cpu.esp -= 4;
    // 0046e8d4  8b5206                 -mov edx, dword ptr [edx + 6]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(6) /* 0x6 */);
    // 0046e8d7  8b407e                 -mov eax, dword ptr [eax + 0x7e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(126) /* 0x7e */);
    // 0046e8da  682894d5ff             -push 0xffd59428
    app->getMemory<x86::reg32>(cpu.esp-4) = 4292187176 /*0xffd59428*/;
    cpu.esp -= 4;
    // 0046e8df  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046e8e2  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046e8e5  682894d5ff             -push 0xffd59428
    app->getMemory<x86::reg32>(cpu.esp-4) = 4292187176 /*0xffd59428*/;
    cpu.esp -= 4;
    // 0046e8ea  83ea02                 -sub edx, 2
    (cpu.edx) -= x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0046e8ed  01f8                   -add eax, edi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edi));
    // 0046e8ef  e86c9f0600             -call 0x4d8860
    cpu.esp -= 4;
    nfs3hp::port::draw2d::drawShadedBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046e8f4  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046e8f7  8b4042                 -mov eax, dword ptr [eax + 0x42]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(66) /* 0x42 */);
    // 0046e8fa  8b55ec                 -mov edx, dword ptr [ebp - 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0046e8fd  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046e900  39d0                   +cmp eax, edx
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046e902  7d2a                   -jge 0x46e92e
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0046e92e;
    }
    // 0046e904  682894d5ff             -push 0xffd59428
    app->getMemory<x86::reg32>(cpu.esp-4) = 4292187176 /*0xffd59428*/;
    cpu.esp -= 4;
    // 0046e909  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046e90c  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046e90f  bb12000000             -mov ebx, 0x12
    cpu.ebx = 18 /*0x12*/;
    // 0046e914  8d4e06                 -lea ecx, [esi + 6]
    cpu.ecx = x86::reg32(cpu.esi + x86::reg32(6) /* 0x6 */);
    // 0046e917  8b525c                 -mov edx, dword ptr [edx + 0x5c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(92) /* 0x5c */);
    // 0046e91a  8b405e                 -mov eax, dword ptr [eax + 0x5e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(94) /* 0x5e */);
    // 0046e91d  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046e920  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046e923  83ea03                 -sub edx, 3
    (cpu.edx) -= x86::reg32(x86::sreg32(3 /*0x3*/));
    // 0046e926  83e803                 -sub eax, 3
    (cpu.eax) -= x86::reg32(x86::sreg32(3 /*0x3*/));
    // 0046e929  e802a00600             -call 0x4d8930
    cpu.esp -= 4;
    nfs3hp::port::draw2d::frame(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0046e92e:
    // 0046e92e  8b4de4                 -mov ecx, dword ptr [ebp - 0x1c]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 0046e931  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046e934  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046e937  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0046e938  8b4006                 -mov eax, dword ptr [eax + 6]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(6) /* 0x6 */);
    // 0046e93b  8b5240                 -mov edx, dword ptr [edx + 0x40]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(64) /* 0x40 */);
    // 0046e93e  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046e941  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046e944  8b5de8                 -mov ebx, dword ptr [ebp - 0x18]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 0046e947  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 0046e949  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0046e94a  8d50ff                 -lea edx, [eax - 1]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(-1) /* -0x1 */);
    // 0046e94d  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0046e94e  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046e951  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0046e952  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 0046e954  8b407e                 -mov eax, dword ptr [eax + 0x7e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(126) /* 0x7e */);
    // 0046e957  b902000000             -mov ecx, 2
    cpu.ecx = 2 /*0x2*/;
    // 0046e95c  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046e95f  e8fc9e0600             -call 0x4d8860
    cpu.esp -= 4;
    nfs3hp::port::draw2d::drawShadedBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046e964  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046e967  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046e96a  8b5206                 -mov edx, dword ptr [edx + 6]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(6) /* 0x6 */);
    // 0046e96d  8b4040                 -mov eax, dword ptr [eax + 0x40]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(64) /* 0x40 */);
    // 0046e970  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046e973  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046e976  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
L_0x0046e978:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0046e978  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046e97a  894dd0                 -mov dword ptr [ebp - 0x30], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-48) /* -0x30 */) = cpu.ecx;
    // 0046e97d  8955cc                 -mov dword ptr [ebp - 0x34], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-52) /* -0x34 */) = cpu.edx;
    // 0046e980  8b45d0                 -mov eax, dword ptr [ebp - 0x30]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-48) /* -0x30 */);
    // 0046e983  3b45c0                 +cmp eax, dword ptr [ebp - 0x40]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-64) /* -0x40 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046e986  7d34                   -jge 0x46e9bc
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0046e9bc;
    }
    // 0046e988  8b45e4                 -mov eax, dword ptr [ebp - 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 0046e98b  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046e98c  8b55e8                 -mov edx, dword ptr [ebp - 0x18]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 0046e98f  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0046e990  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0046e991  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046e992  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046e995  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 0046e99a  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 0046e99c  8b407e                 -mov eax, dword ptr [eax + 0x7e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(126) /* 0x7e */);
    // 0046e99f  8b55cc                 -mov edx, dword ptr [ebp - 0x34]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-52) /* -0x34 */);
    // 0046e9a2  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046e9a5  e8b69e0600             -call 0x4d8860
    cpu.esp -= 4;
    nfs3hp::port::draw2d::drawShadedBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046e9aa  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046e9ad  8b55cc                 -mov edx, dword ptr [ebp - 0x34]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-52) /* -0x34 */);
    // 0046e9b0  8b4040                 -mov eax, dword ptr [eax + 0x40]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(64) /* 0x40 */);
    // 0046e9b3  8b4dd0                 -mov ecx, dword ptr [ebp - 0x30]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-48) /* -0x30 */);
    // 0046e9b6  c1f810                 +sar eax, 0x10
    {
        x86::reg8 tmp = 16 /*0x10*/ % 32;
        x86::reg32& op = cpu.eax;
        if (tmp)
        {
            cpu.flags.cf = 1 & (x86::sreg32(op) >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = 0;
            cpu.set_szp((op = x86::reg32(x86::sreg32(op) >> tmp)));
        }
    }
    // 0046e9b9  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0046e9ba  ebbc                   -jmp 0x46e978
    goto L_0x0046e978;
L_0x0046e9bc:
    // 0046e9bc  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046e9bf  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046e9c2  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 0046e9c7  8b527e                 -mov edx, dword ptr [edx + 0x7e]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(126) /* 0x7e */);
    // 0046e9ca  8b403a                 -mov eax, dword ptr [eax + 0x3a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(58) /* 0x3a */);
    // 0046e9cd  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046e9d0  c1f810                 +sar eax, 0x10
    {
        x86::reg8 tmp = 16 /*0x10*/ % 32;
        x86::reg32& op = cpu.eax;
        if (tmp)
        {
            cpu.flags.cf = 1 & (x86::sreg32(op) >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = 0;
            cpu.set_szp((op = x86::reg32(x86::sreg32(op) >> tmp)));
        }
    }
    // 0046e9d3  895de0                 -mov dword ptr [ebp - 0x20], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */) = cpu.ebx;
    // 0046e9d6  8d3c02                 -lea edi, [edx + eax]
    cpu.edi = x86::reg32(cpu.edx + cpu.eax * 1);
    // 0046e9d9  eb06                   -jmp 0x46e9e1
    goto L_0x0046e9e1;
L_0x0046e9db:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0046e9db  837de004               +cmp dword ptr [ebp - 0x20], 4
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046e9df  7d3c                   -jge 0x46ea1d
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0046ea1d;
    }
L_0x0046e9e1:
    // 0046e9e1  8b55e8                 -mov edx, dword ptr [ebp - 0x18]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 0046e9e4  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0046e9e5  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0046e9e6  8b5de4                 -mov ebx, dword ptr [ebp - 0x1c]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 0046e9e9  89f1                   -mov ecx, esi
    cpu.ecx = cpu.esi;
    // 0046e9eb  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0046e9ec  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046e9ef  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0046e9f1  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0046e9f2  8b5206                 -mov edx, dword ptr [edx + 6]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(6) /* 0x6 */);
    // 0046e9f5  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 0046e9fa  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046e9fd  e85e9e0600             -call 0x4d8860
    cpu.esp -= 4;
    nfs3hp::port::draw2d::drawShadedBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046ea02  8b45e0                 -mov eax, dword ptr [ebp - 0x20]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 0046ea05  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046ea08  01c0                   -add eax, eax
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046ea0a  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 0046ea0c  8b4de0                 -mov ecx, dword ptr [ebp - 0x20]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 0046ea0f  8b403a                 -mov eax, dword ptr [eax + 0x3a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(58) /* 0x3a */);
    // 0046ea12  41                     -inc ecx
    (cpu.ecx)++;
    // 0046ea13  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046ea16  894de0                 -mov dword ptr [ebp - 0x20], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */) = cpu.ecx;
    // 0046ea19  01c7                   +add edi, eax
    {
        x86::reg32& tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0046ea1b  ebbe                   -jmp 0x46e9db
    goto L_0x0046e9db;
L_0x0046ea1d:
    // 0046ea1d  837dc800               +cmp dword ptr [ebp - 0x38], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-56) /* -0x38 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046ea21  7511                   -jne 0x46ea34
    if (!cpu.flags.zf)
    {
        goto L_0x0046ea34;
    }
    // 0046ea23  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046ea26  6683787e00             +cmp word ptr [eax + 0x7e], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(126) /* 0x7e */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046ea2b  7507                   -jne 0x46ea34
    if (!cpu.flags.zf)
    {
        goto L_0x0046ea34;
    }
    // 0046ea2d  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0046ea2f  e82cfcffff             -call 0x46e660
    cpu.esp -= 4;
    sub_46e660(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0046ea34:
    // 0046ea34  31f6                   +xor esi, esi
    cpu.clear_co();
    cpu.set_szp((cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi))));
    // 0046ea36  8975f8                 -mov dword ptr [ebp - 8], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.esi;
    // 0046ea39  e9e0000000             -jmp 0x46eb1e
    goto L_0x0046eb1e;
L_0x0046ea3e:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0046ea3e  8b4646                 -mov eax, dword ptr [esi + 0x46]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(70) /* 0x46 */);
    // 0046ea41  8b564a                 -mov edx, dword ptr [esi + 0x4a]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(74) /* 0x4a */);
    // 0046ea44  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046ea47  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046ea4a  8d780a                 -lea edi, [eax + 0xa]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(10) /* 0xa */);
L_0x0046ea4d:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0046ea4d  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046ea4f  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046ea52  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0046ea54  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0046ea56  8b5644                 -mov edx, dword ptr [esi + 0x44]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(68) /* 0x44 */);
    // 0046ea59  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046ea5c  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 0046ea5e  8d70fb                 -lea esi, [eax - 5]
    cpu.esi = x86::reg32(cpu.eax + x86::reg32(-5) /* -0x5 */);
    // 0046ea61  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0046ea64  8b44458e               -mov eax, dword ptr [ebp + eax*2 - 0x72]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-114) /* -0x72 */ + cpu.eax * 2);
    // 0046ea68  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046ea6b  e8e02d0600             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046ea70  8945b0                 -mov dword ptr [ebp - 0x50], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-80) /* -0x50 */) = cpu.eax;
    // 0046ea73  837dc800               +cmp dword ptr [ebp - 0x38], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-56) /* -0x38 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046ea77  7524                   -jne 0x46ea9d
    if (!cpu.flags.zf)
    {
        goto L_0x0046ea9d;
    }
    // 0046ea79  837dac00               +cmp dword ptr [ebp - 0x54], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-84) /* -0x54 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046ea7d  741e                   -je 0x46ea9d
    if (cpu.flags.zf)
    {
        goto L_0x0046ea9d;
    }
    // 0046ea7f  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0046ea82  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046ea85  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 0046ea88  83c246                 -add edx, 0x46
    (cpu.edx) += x86::reg32(x86::sreg32(70 /*0x46*/));
    // 0046ea8b  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 0046ea8d  e85ed70200             -call 0x49c1f0
    cpu.esp -= 4;
    nfs3hp::frontend::pointerInBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046ea92  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046ea94  7407                   -je 0x46ea9d
    if (cpu.flags.zf)
    {
        goto L_0x0046ea9d;
    }
    // 0046ea96  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0046ea9b  eb02                   -jmp 0x46ea9f
    goto L_0x0046ea9f;
L_0x0046ea9d:
    // 0046ea9d  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x0046ea9f:
    // 0046ea9f  8945b4                 -mov dword ptr [ebp - 0x4c], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-76) /* -0x4c */) = cpu.eax;
    // 0046eaa2  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046eaa4  7433                   -je 0x46ead9
    if (cpu.flags.zf)
    {
        goto L_0x0046ead9;
    }
    // 0046eaa6  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0046eaa9  8b4dfc                 -mov ecx, dword ptr [ebp - 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046eaac  c1e203                 -shl edx, 3
    cpu.edx <<= 3 /*0x3*/ % 32;
    // 0046eaaf  01ca                   -add edx, ecx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.ecx));
    // 0046eab1  6840e4ffff             -push 0xffffe440
    app->getMemory<x86::reg32>(cpu.esp-4) = 4294960192 /*0xffffe440*/;
    cpu.esp -= 4;
    // 0046eab6  8b4a4a                 -mov ecx, dword ptr [edx + 0x4a]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(74) /* 0x4a */);
    // 0046eab9  8b5a48                 -mov ebx, dword ptr [edx + 0x48]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(72) /* 0x48 */);
    // 0046eabc  8b4244                 -mov eax, dword ptr [edx + 0x44]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(68) /* 0x44 */);
    // 0046eabf  c1f910                 -sar ecx, 0x10
    cpu.ecx = x86::reg32(x86::sreg32(cpu.ecx) >> (16 /*0x10*/ % 32));
    // 0046eac2  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046eac5  c1fb10                 -sar ebx, 0x10
    cpu.ebx = x86::reg32(x86::sreg32(cpu.ebx) >> (16 /*0x10*/ % 32));
    // 0046eac8  8945a8                 -mov dword ptr [ebp - 0x58], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-88) /* -0x58 */) = cpu.eax;
    // 0046eacb  8b4246                 -mov eax, dword ptr [edx + 0x46]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(70) /* 0x46 */);
    // 0046eace  8b55a8                 -mov edx, dword ptr [ebp - 0x58]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-88) /* -0x58 */);
    // 0046ead1  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046ead4  e8579e0600             -call 0x4d8930
    cpu.esp -= 4;
    nfs3hp::port::draw2d::frame(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0046ead9:
    // 0046ead9  837dd800               +cmp dword ptr [ebp - 0x28], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-40) /* -0x28 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046eadd  7407                   -je 0x46eae6
    if (cpu.flags.zf)
    {
        goto L_0x0046eae6;
    }
    // 0046eadf  b8964a0000             -mov eax, 0x4a96
    cpu.eax = 19094 /*0x4a96*/;
    // 0046eae4  eb12                   -jmp 0x46eaf8
    goto L_0x0046eaf8;
L_0x0046eae6:
    // 0046eae6  837db400               +cmp dword ptr [ebp - 0x4c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-76) /* -0x4c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046eaea  7507                   -jne 0x46eaf3
    if (!cpu.flags.zf)
    {
        goto L_0x0046eaf3;
    }
    // 0046eaec  b82894d5ff             -mov eax, 0xffd59428
    cpu.eax = 4292187176 /*0xffd59428*/;
    // 0046eaf1  eb05                   -jmp 0x46eaf8
    goto L_0x0046eaf8;
L_0x0046eaf3:
    // 0046eaf3  b840e4ff00             -mov eax, 0xffe440
    cpu.eax = 16770112 /*0xffe440*/;
L_0x0046eaf8:
    // 0046eaf8  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046eaf9  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0046eafc  8b444596               -mov eax, dword ptr [ebp + eax*2 - 0x6a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-106) /* -0x6a */ + cpu.eax * 2);
    // 0046eb00  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 0046eb05  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046eb08  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 0046eb0a  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046eb0b  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 0046eb0d  8b45b0                 -mov eax, dword ptr [ebp - 0x50]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-80) /* -0x50 */);
    // 0046eb10  e87b36feff             -call 0x452190
    cpu.esp -= 4;
    nfs3hp::port::text::drawMenuText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046eb15  ff45f8                 -inc dword ptr [ebp - 8]
    (app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */))++;
    // 0046eb18  837df803               +cmp dword ptr [ebp - 8], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046eb1c  7d57                   -jge 0x46eb75
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0046eb75;
    }
L_0x0046eb1e:
    // 0046eb1e  8d7d90                 -lea edi, [ebp - 0x70]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-112) /* -0x70 */);
    // 0046eb21  bec0cc4600             -mov esi, 0x46ccc0
    cpu.esi = 4639936 /*0x46ccc0*/;
    // 0046eb26  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
    app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
    if (cpu.flags.df)
    {
        cpu.edi -= 4;
        cpu.esi -= 4;
    }
    else
    {
        cpu.edi += 4;
        cpu.esi += 4;
    }
    // 0046eb27  66a5                   -movsw word ptr es:[edi], word ptr [esi]
    app->getMemory<x86::reg16>(cpu.ees + cpu.edi) = app->getMemory<x86::reg16>(cpu.esi);
    if (cpu.flags.df)
    {
        cpu.edi -= 2;
        cpu.esi -= 2;
    }
    else
    {
        cpu.edi += 2;
        cpu.esi += 2;
    }
    // 0046eb29  8d7d98                 -lea edi, [ebp - 0x68]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-104) /* -0x68 */);
    // 0046eb2c  bec6cc4600             -mov esi, 0x46ccc6
    cpu.esi = 4639942 /*0x46ccc6*/;
    // 0046eb31  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0046eb34  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
    app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
    if (cpu.flags.df)
    {
        cpu.edi -= 4;
        cpu.esi -= 4;
    }
    else
    {
        cpu.edi += 4;
        cpu.esi += 4;
    }
    // 0046eb35  66a5                   -movsw word ptr es:[edi], word ptr [esi]
    app->getMemory<x86::reg16>(cpu.ees + cpu.edi) = app->getMemory<x86::reg16>(cpu.esi);
    if (cpu.flags.df)
    {
        cpu.edi -= 2;
        cpu.esi -= 2;
    }
    else
    {
        cpu.edi += 2;
        cpu.esi += 2;
    }
    // 0046eb37  8d04d500000000         -lea eax, [edx*8]
    cpu.eax = x86::reg32(cpu.edx * 8);
    // 0046eb3e  8b75fc                 -mov esi, dword ptr [ebp - 4]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046eb41  668b7c5598             -mov di, word ptr [ebp + edx*2 - 0x68]
    cpu.di = app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-104) /* -0x68 */ + cpu.edx * 2);
    // 0046eb46  01c6                   -add esi, eax
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046eb48  6683ff05               +cmp di, 5
    {
        x86::reg16 tmp1 = cpu.di;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(5 /*0x5*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046eb4c  0f85ecfeffff           -jne 0x46ea3e
    if (!cpu.flags.zf)
    {
        goto L_0x0046ea3e;
    }
    // 0046eb52  8b5648                 -mov edx, dword ptr [esi + 0x48]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(72) /* 0x48 */);
    // 0046eb55  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046eb58  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046eb5a  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046eb5d  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0046eb5f  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0046eb61  8b5646                 -mov edx, dword ptr [esi + 0x46]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(70) /* 0x46 */);
    // 0046eb64  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046eb67  8d3c02                 -lea edi, [edx + eax]
    cpu.edi = x86::reg32(cpu.edx + cpu.eax * 1);
    // 0046eb6a  8b564a                 -mov edx, dword ptr [esi + 0x4a]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(74) /* 0x4a */);
    // 0046eb6d  c1fa10                 +sar edx, 0x10
    {
        x86::reg8 tmp = 16 /*0x10*/ % 32;
        x86::reg32& op = cpu.edx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (x86::sreg32(op) >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = 0;
            cpu.set_szp((op = x86::reg32(x86::sreg32(op) >> tmp)));
        }
    }
    // 0046eb70  e9d8feffff             -jmp 0x46ea4d
    goto L_0x0046ea4d;
L_0x0046eb75:
    // 0046eb75  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046eb78  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046eb7b  8b5240                 -mov edx, dword ptr [edx + 0x40]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(64) /* 0x40 */);
    // 0046eb7e  8b4006                 -mov eax, dword ptr [eax + 6]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(6) /* 0x6 */);
    // 0046eb81  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046eb84  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046eb87  8d0c10                 -lea ecx, [eax + edx]
    cpu.ecx = x86::reg32(cpu.eax + cpu.edx * 1);
    // 0046eb8a  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046eb8c  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046eb8f  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0046eb91  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0046eb93  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 0046eb95  8d78fb                 -lea edi, [eax - 5]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(-5) /* -0x5 */);
    // 0046eb98  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046eb9b  8b4078                 -mov eax, dword ptr [eax + 0x78]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(120) /* 0x78 */);
    // 0046eb9e  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0046eba0  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046eba3  894dd4                 -mov dword ptr [ebp - 0x2c], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44) /* -0x2c */) = cpu.ecx;
    // 0046eba6  8945dc                 -mov dword ptr [ebp - 0x24], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = cpu.eax;
L_0x0046eba9:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0046eba9  8b45d4                 -mov eax, dword ptr [ebp - 0x2c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44) /* -0x2c */);
    // 0046ebac  3b45c0                 +cmp eax, dword ptr [ebp - 0x40]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-64) /* -0x40 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046ebaf  0f8d2a030000           -jge 0x46eedf
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0046eedf;
    }
    // 0046ebb5  8b45dc                 -mov eax, dword ptr [ebp - 0x24]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 0046ebb8  3b45ec                 +cmp eax, dword ptr [ebp - 0x14]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046ebbb  0f8d1e030000           -jge 0x46eedf
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0046eedf;
    }
    // 0046ebc1  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0046ebc3  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0046ebc5  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0046ebc7  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0046ebc9  e80271feff             -call 0x455cd0
    cpu.esp -= 4;
    sub_455cd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046ebce  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0046ebd0  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046ebd2  0f8407030000           -je 0x46eedf
    if (cpu.flags.zf)
    {
        goto L_0x0046eedf;
    }
    // 0046ebd8  83b8c402000002         +cmp dword ptr [eax + 0x2c4], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(708) /* 0x2c4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046ebdf  7507                   -jne 0x46ebe8
    if (!cpu.flags.zf)
    {
        goto L_0x0046ebe8;
    }
    // 0046ebe1  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 0046ebe6  eb02                   -jmp 0x46ebea
    goto L_0x0046ebea;
L_0x0046ebe8:
    // 0046ebe8  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
L_0x0046ebea:
    // 0046ebea  8d5da4                 -lea ebx, [ebp - 0x5c]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-92) /* -0x5c */);
    // 0046ebed  8d55a0                 -lea edx, [ebp - 0x60]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-96) /* -0x60 */);
    // 0046ebf0  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0046ebf2  e83969feff             -call 0x455530
    cpu.esp -= 4;
    sub_455530(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046ebf7  837dc800               +cmp dword ptr [ebp - 0x38], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-56) /* -0x38 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046ebfb  0f844e010000           -je 0x46ed4f
    if (cpu.flags.zf)
    {
        goto L_0x0046ed4f;
    }
    // 0046ec01  837dd800               +cmp dword ptr [ebp - 0x28], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-40) /* -0x28 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046ec05  7407                   -je 0x46ec0e
    if (cpu.flags.zf)
    {
        goto L_0x0046ec0e;
    }
    // 0046ec07  b9964a0000             -mov ecx, 0x4a96
    cpu.ecx = 19094 /*0x4a96*/;
    // 0046ec0c  eb05                   -jmp 0x46ec13
    goto L_0x0046ec13;
L_0x0046ec0e:
    // 0046ec0e  b9fd9d64ff             -mov ecx, 0xff649dfd
    cpu.ecx = 4284784125 /*0xff649dfd*/;
L_0x0046ec13:
    // 0046ec13  8b15f4d46f00           -mov edx, dword ptr [0x6fd4f4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */);
    // 0046ec19  8b86f0010000           -mov eax, dword ptr [esi + 0x1f0]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(496) /* 0x1f0 */);
    // 0046ec1f  894dbc                 -mov dword ptr [ebp - 0x44], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-68) /* -0x44 */) = cpu.ecx;
    // 0046ec22  39d0                   +cmp eax, edx
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046ec24  740f                   -je 0x46ec35
    if (cpu.flags.zf)
    {
        goto L_0x0046ec35;
    }
    // 0046ec26  8b1db0d36f00           -mov ebx, dword ptr [0x6fd3b0]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
    // 0046ec2c  83fb01                 +cmp ebx, 1
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046ec2f  7509                   -jne 0x46ec3a
    if (!cpu.flags.zf)
    {
        goto L_0x0046ec3a;
    }
    // 0046ec31  39d8                   +cmp eax, ebx
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046ec33  7505                   -jne 0x46ec3a
    if (!cpu.flags.zf)
    {
        goto L_0x0046ec3a;
    }
L_0x0046ec35:
    // 0046ec35  b940e4ff00             -mov ecx, 0xffe440
    cpu.ecx = 16770112 /*0xffe440*/;
L_0x0046ec3a:
    // 0046ec3a  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046ec3d  8b407e                 -mov eax, dword ptr [eax + 0x7e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(126) /* 0x7e */);
    // 0046ec40  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046ec43  8d9590fdffff           -lea edx, [ebp - 0x270]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-624) /* -0x270 */);
    // 0046ec49  8945f0                 -mov dword ptr [ebp - 0x10], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.eax;
    // 0046ec4c  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0046ec4e  e81d69feff             -call 0x455570
    cpu.esp -= 4;
    sub_455570(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046ec53  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0046ec54  8d5ff6                 -lea ebx, [edi - 0xa]
    cpu.ebx = x86::reg32(cpu.edi + x86::reg32(-10) /* -0xa */);
    // 0046ec57  8b45f0                 -mov eax, dword ptr [ebp - 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0046ec5a  6a03                   -push 3
    app->getMemory<x86::reg32>(cpu.esp-4) = 3 /*0x3*/;
    cpu.esp -= 4;
    // 0046ec5c  83c00a                 -add eax, 0xa
    (cpu.eax) += x86::reg32(x86::sreg32(10 /*0xa*/));
    // 0046ec5f  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 0046ec64  8945a8                 -mov dword ptr [ebp - 0x58], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-88) /* -0x58 */) = cpu.eax;
    // 0046ec67  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0046ec69  8d8590fdffff           -lea eax, [ebp - 0x270]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-624) /* -0x270 */);
    // 0046ec6f  e81c35feff             -call 0x452190
    cpu.esp -= 4;
    nfs3hp::port::text::drawMenuText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046ec74  8d9590fdffff           -lea edx, [ebp - 0x270]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-624) /* -0x270 */);
    // 0046ec7a  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0046ec7c  e87f69feff             -call 0x455600
    cpu.esp -= 4;
    sub_455600(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046ec81  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046ec84  8b403a                 -mov eax, dword ptr [eax + 0x3a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(58) /* 0x3a */);
    // 0046ec87  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046ec8a  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046ec8b  8b55bc                 -mov edx, dword ptr [ebp - 0x44]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-68) /* -0x44 */);
    // 0046ec8e  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0046ec8f  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 0046ec94  8d5f05                 -lea ebx, [edi + 5]
    cpu.ebx = x86::reg32(cpu.edi + x86::reg32(5) /* 0x5 */);
    // 0046ec97  6a03                   -push 3
    app->getMemory<x86::reg32>(cpu.esp-4) = 3 /*0x3*/;
    cpu.esp -= 4;
    // 0046ec99  8d8590fdffff           -lea eax, [ebp - 0x270]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-624) /* -0x270 */);
    // 0046ec9f  8b55a8                 -mov edx, dword ptr [ebp - 0x58]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-88) /* -0x58 */);
    // 0046eca2  e87934feff             -call 0x452120
    cpu.esp -= 4;
    sub_452120(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046eca7  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046ecaa  8b403a                 -mov eax, dword ptr [eax + 0x3a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(58) /* 0x3a */);
    // 0046ecad  8b4df0                 -mov ecx, dword ptr [ebp - 0x10]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0046ecb0  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046ecb3  8b5dd8                 -mov ebx, dword ptr [ebp - 0x28]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-40) /* -0x28 */);
    // 0046ecb6  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046ecb8  b8eb000000             -mov eax, 0xeb
    cpu.eax = 235 /*0xeb*/;
    // 0046ecbd  894df0                 -mov dword ptr [ebp - 0x10], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.ecx;
    // 0046ecc0  e88b2b0600             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046ecc5  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0046ecc7  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0046ecc9  7407                   -je 0x46ecd2
    if (cpu.flags.zf)
    {
        goto L_0x0046ecd2;
    }
    // 0046eccb  b8964a0000             -mov eax, 0x4a96
    cpu.eax = 19094 /*0x4a96*/;
    // 0046ecd0  eb05                   -jmp 0x46ecd7
    goto L_0x0046ecd7;
L_0x0046ecd2:
    // 0046ecd2  b800008000             -mov eax, 0x800000
    cpu.eax = 8388608 /*0x800000*/;
L_0x0046ecd7:
    // 0046ecd7  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046ecda  8b523c                 -mov edx, dword ptr [edx + 0x3c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(60) /* 0x3c */);
    // 0046ecdd  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046ece0  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046ece1  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046ece3  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046ece6  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0046ece8  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0046ecea  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 0046ecef  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 0046ecf1  8b55f0                 -mov edx, dword ptr [ebp - 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0046ecf4  6a05                   -push 5
    app->getMemory<x86::reg32>(cpu.esp-4) = 5 /*0x5*/;
    cpu.esp -= 4;
    // 0046ecf6  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046ecf8  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0046ecfa  e89134feff             -call 0x452190
    cpu.esp -= 4;
    nfs3hp::port::text::drawMenuText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046ecff  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046ed02  8b403c                 -mov eax, dword ptr [eax + 0x3c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(60) /* 0x3c */);
    // 0046ed05  8b75f0                 -mov esi, dword ptr [ebp - 0x10]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0046ed08  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046ed0b  01c6                   -add esi, eax
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046ed0d  8b45a4                 -mov eax, dword ptr [ebp - 0x5c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-92) /* -0x5c */);
    // 0046ed10  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046ed11  68b8ae5300             -push 0x53aeb8
    app->getMemory<x86::reg32>(cpu.esp-4) = 5484216 /*0x53aeb8*/;
    cpu.esp -= 4;
    // 0046ed16  8d8590fdffff           -lea eax, [ebp - 0x270]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-624) /* -0x270 */);
    // 0046ed1c  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046ed1d  e86e090700             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046ed22  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 0046ed25  8b55bc                 -mov edx, dword ptr [ebp - 0x44]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-68) /* -0x44 */);
    // 0046ed28  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0046ed29  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046ed2c  8b523e                 -mov edx, dword ptr [edx + 0x3e]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(62) /* 0x3e */);
    // 0046ed2f  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046ed32  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046ed34  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046ed37  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0046ed39  d1f8                   +sar eax, 1
    {
        x86::reg32 tmp = 1 /*0x1*/ % 32;
        x86::reg32& op = cpu.eax;
        if (tmp)
        {
            cpu.flags.cf = 1 & (x86::sreg32(op) >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = 0;
            cpu.set_szp((op = x86::reg32(x86::sreg32(op) >> tmp)));
        }
    }
    // 0046ed3b  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 0046ed40  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 0046ed42  8975f0                 -mov dword ptr [ebp - 0x10], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.esi;
    // 0046ed45  6a05                   -push 5
    app->getMemory<x86::reg32>(cpu.esp-4) = 5 /*0x5*/;
    cpu.esp -= 4;
    // 0046ed47  8d1406                 -lea edx, [esi + eax]
    cpu.edx = x86::reg32(cpu.esi + cpu.eax * 1);
    // 0046ed4a  e967010000             -jmp 0x46eeb6
    goto L_0x0046eeb6;
L_0x0046ed4f:
    // 0046ed4f  837dd800               +cmp dword ptr [ebp - 0x28], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-40) /* -0x28 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046ed53  7407                   -je 0x46ed5c
    if (cpu.flags.zf)
    {
        goto L_0x0046ed5c;
    }
    // 0046ed55  b8964a0000             -mov eax, 0x4a96
    cpu.eax = 19094 /*0x4a96*/;
    // 0046ed5a  eb10                   -jmp 0x46ed6c
    goto L_0x0046ed6c;
L_0x0046ed5c:
    // 0046ed5c  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0046ed5e  7407                   -je 0x46ed67
    if (cpu.flags.zf)
    {
        goto L_0x0046ed67;
    }
    // 0046ed60  b800008000             -mov eax, 0x800000
    cpu.eax = 8388608 /*0x800000*/;
    // 0046ed65  eb05                   -jmp 0x46ed6c
    goto L_0x0046ed6c;
L_0x0046ed67:
    // 0046ed67  b8fd9d64ff             -mov eax, 0xff649dfd
    cpu.eax = 4284784125 /*0xff649dfd*/;
L_0x0046ed6c:
    // 0046ed6c  8b15f4d46f00           -mov edx, dword ptr [0x6fd4f4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */);
    // 0046ed72  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0046ed74  8945c4                 -mov dword ptr [ebp - 0x3c], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-60) /* -0x3c */) = cpu.eax;
    // 0046ed77  8b86f0010000           -mov eax, dword ptr [esi + 0x1f0]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(496) /* 0x1f0 */);
    // 0046ed7d  39d0                   +cmp eax, edx
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046ed7f  740f                   -je 0x46ed90
    if (cpu.flags.zf)
    {
        goto L_0x0046ed90;
    }
    // 0046ed81  8b1db0d36f00           -mov ebx, dword ptr [0x6fd3b0]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
    // 0046ed87  83fb01                 +cmp ebx, 1
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046ed8a  7509                   -jne 0x46ed95
    if (!cpu.flags.zf)
    {
        goto L_0x0046ed95;
    }
    // 0046ed8c  39d8                   +cmp eax, ebx
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046ed8e  7505                   -jne 0x46ed95
    if (!cpu.flags.zf)
    {
        goto L_0x0046ed95;
    }
L_0x0046ed90:
    // 0046ed90  b940e4ff00             -mov ecx, 0xffe440
    cpu.ecx = 16770112 /*0xffe440*/;
L_0x0046ed95:
    // 0046ed95  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046ed98  8b407e                 -mov eax, dword ptr [eax + 0x7e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(126) /* 0x7e */);
    // 0046ed9b  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046ed9e  8d9590fdffff           -lea edx, [ebp - 0x270]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-624) /* -0x270 */);
    // 0046eda4  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
    // 0046eda7  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0046eda9  e8c267feff             -call 0x455570
    cpu.esp -= 4;
    sub_455570(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046edae  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0046edaf  8d5ff6                 -lea ebx, [edi - 0xa]
    cpu.ebx = x86::reg32(cpu.edi + x86::reg32(-10) /* -0xa */);
    // 0046edb2  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0046edb5  6a03                   -push 3
    app->getMemory<x86::reg32>(cpu.esp-4) = 3 /*0x3*/;
    cpu.esp -= 4;
    // 0046edb7  83c00a                 -add eax, 0xa
    (cpu.eax) += x86::reg32(x86::sreg32(10 /*0xa*/));
    // 0046edba  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 0046edbf  8945a8                 -mov dword ptr [ebp - 0x58], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-88) /* -0x58 */) = cpu.eax;
    // 0046edc2  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0046edc4  8d8590fdffff           -lea eax, [ebp - 0x270]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-624) /* -0x270 */);
    // 0046edca  e8c133feff             -call 0x452190
    cpu.esp -= 4;
    nfs3hp::port::text::drawMenuText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046edcf  8d9590fdffff           -lea edx, [ebp - 0x270]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-624) /* -0x270 */);
    // 0046edd5  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0046edd7  e82468feff             -call 0x455600
    cpu.esp -= 4;
    sub_455600(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046eddc  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046eddf  8b403a                 -mov eax, dword ptr [eax + 0x3a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(58) /* 0x3a */);
    // 0046ede2  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046ede5  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046ede6  8b55c4                 -mov edx, dword ptr [ebp - 0x3c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-60) /* -0x3c */);
    // 0046ede9  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0046edea  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 0046edef  8d5f05                 -lea ebx, [edi + 5]
    cpu.ebx = x86::reg32(cpu.edi + x86::reg32(5) /* 0x5 */);
    // 0046edf2  6a03                   -push 3
    app->getMemory<x86::reg32>(cpu.esp-4) = 3 /*0x3*/;
    cpu.esp -= 4;
    // 0046edf4  8d8590fdffff           -lea eax, [ebp - 0x270]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-624) /* -0x270 */);
    // 0046edfa  8b55a8                 -mov edx, dword ptr [ebp - 0x58]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-88) /* -0x58 */);
    // 0046edfd  e81e33feff             -call 0x452120
    cpu.esp -= 4;
    sub_452120(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046ee02  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046ee05  8b403a                 -mov eax, dword ptr [eax + 0x3a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(58) /* 0x3a */);
    // 0046ee08  8b4df4                 -mov ecx, dword ptr [ebp - 0xc]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0046ee0b  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046ee0e  8d9590feffff           -lea edx, [ebp - 0x170]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-368) /* -0x170 */);
    // 0046ee14  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046ee16  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0046ee18  e8a367feff             -call 0x4555c0
    cpu.esp -= 4;
    sub_4555c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046ee1d  8d8590feffff           -lea eax, [ebp - 0x170]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-368) /* -0x170 */);
    // 0046ee23  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046ee24  8b5da0                 -mov ebx, dword ptr [ebp - 0x60]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-96) /* -0x60 */);
    // 0046ee27  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0046ee28  68e4ae5300             -push 0x53aee4
    app->getMemory<x86::reg32>(cpu.esp-4) = 5484260 /*0x53aee4*/;
    cpu.esp -= 4;
    // 0046ee2d  8d8590fdffff           -lea eax, [ebp - 0x270]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-624) /* -0x270 */);
    // 0046ee33  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046ee34  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046ee37  e854080700             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046ee3c  8b523c                 -mov edx, dword ptr [edx + 0x3c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(60) /* 0x3c */);
    // 0046ee3f  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046ee42  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046ee44  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046ee47  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0046ee49  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0046ee4b  83c410                 -add esp, 0x10
    (cpu.esp) += x86::reg32(x86::sreg32(16 /*0x10*/));
    // 0046ee4e  8b75c4                 -mov esi, dword ptr [ebp - 0x3c]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-60) /* -0x3c */);
    // 0046ee51  894df4                 -mov dword ptr [ebp - 0xc], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.ecx;
    // 0046ee54  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0046ee55  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 0046ee57  8d1401                 -lea edx, [ecx + eax]
    cpu.edx = x86::reg32(cpu.ecx + cpu.eax * 1);
    // 0046ee5a  6a05                   -push 5
    app->getMemory<x86::reg32>(cpu.esp-4) = 5 /*0x5*/;
    cpu.esp -= 4;
    // 0046ee5c  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 0046ee61  8d8590fdffff           -lea eax, [ebp - 0x270]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-624) /* -0x270 */);
    // 0046ee67  e82433feff             -call 0x452190
    cpu.esp -= 4;
    nfs3hp::port::text::drawMenuText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046ee6c  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046ee6f  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0046ee72  8b4da4                 -mov ecx, dword ptr [ebp - 0x5c]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-92) /* -0x5c */);
    // 0046ee75  8b403c                 -mov eax, dword ptr [eax + 0x3c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(60) /* 0x3c */);
    // 0046ee78  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0046ee79  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046ee7c  68b8ae5300             -push 0x53aeb8
    app->getMemory<x86::reg32>(cpu.esp-4) = 5484216 /*0x53aeb8*/;
    cpu.esp -= 4;
    // 0046ee81  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046ee83  8d8590fdffff           -lea eax, [ebp - 0x270]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-624) /* -0x270 */);
    // 0046ee89  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046ee8a  8955f4                 -mov dword ptr [ebp - 0xc], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.edx;
    // 0046ee8d  e8fe070700             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046ee92  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 0046ee95  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046ee98  8b523e                 -mov edx, dword ptr [edx + 0x3e]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(62) /* 0x3e */);
    // 0046ee9b  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046ee9e  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046eea0  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046eea3  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0046eea5  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0046eea7  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 0046eeac  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0046eead  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 0046eeaf  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0046eeb2  6a05                   -push 5
    app->getMemory<x86::reg32>(cpu.esp-4) = 5 /*0x5*/;
    cpu.esp -= 4;
    // 0046eeb4  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
L_0x0046eeb6:
    // 0046eeb6  8d8590fdffff           -lea eax, [ebp - 0x270]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-624) /* -0x270 */);
    // 0046eebc  e8cf32feff             -call 0x452190
    cpu.esp -= 4;
    nfs3hp::port::text::drawMenuText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046eec1  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046eec4  8b4040                 -mov eax, dword ptr [eax + 0x40]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(64) /* 0x40 */);
    // 0046eec7  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046eeca  8b75dc                 -mov esi, dword ptr [ebp - 0x24]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 0046eecd  01c7                   +add edi, eax
    {
        x86::reg32& tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0046eecf  8b45d4                 -mov eax, dword ptr [ebp - 0x2c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44) /* -0x2c */);
    // 0046eed2  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0046eed3  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0046eed4  8975dc                 -mov dword ptr [ebp - 0x24], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = cpu.esi;
    // 0046eed7  8945d4                 -mov dword ptr [ebp - 0x2c], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44) /* -0x2c */) = cpu.eax;
    // 0046eeda  e9cafcffff             -jmp 0x46eba9
    goto L_0x0046eba9;
L_0x0046eedf:
    // 0046eedf  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046eee2  8b4042                 -mov eax, dword ptr [eax + 0x42]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(66) /* 0x42 */);
    // 0046eee5  8b55ec                 -mov edx, dword ptr [ebp - 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0046eee8  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046eeeb  39d0                   +cmp eax, edx
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046eeed  0f8d82000000           -jge 0x46ef75
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0046ef75;
    }
    // 0046eef3  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046eef6  6683787a00             +cmp word ptr [eax + 0x7a], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(122) /* 0x7a */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046eefb  742d                   -je 0x46ef2a
    if (cpu.flags.zf)
    {
        goto L_0x0046ef2a;
    }
    // 0046eefd  83c05e                 -add eax, 0x5e
    (cpu.eax) += x86::reg32(x86::sreg32(94 /*0x5e*/));
    // 0046ef00  8b58fe                 -mov ebx, dword ptr [eax - 2]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-2) /* -0x2 */);
    // 0046ef03  8b10                   -mov edx, dword ptr [eax]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax);
    // 0046ef05  e8e6d20200             -call 0x49c1f0
    cpu.esp -= 4;
    nfs3hp::frontend::pointerInBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046ef0a  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046ef0d  c1fb10                 -sar ebx, 0x10
    cpu.ebx = x86::reg32(x86::sreg32(cpu.ebx) >> (16 /*0x10*/ % 32));
    // 0046ef10  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046ef12  7409                   -je 0x46ef1d
    if (cpu.flags.zf)
    {
        goto L_0x0046ef1d;
    }
    // 0046ef14  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046ef17  668b4072               -mov ax, word ptr [eax + 0x72]
    cpu.ax = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(114) /* 0x72 */);
    // 0046ef1b  eb07                   -jmp 0x46ef24
    goto L_0x0046ef24;
L_0x0046ef1d:
    // 0046ef1d  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046ef20  668b406e               -mov ax, word ptr [eax + 0x6e]
    cpu.ax = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(110) /* 0x6e */);
L_0x0046ef24:
    // 0046ef24  98                     -cwde 
    cpu.eax = x86::reg32(static_cast<x86::sreg16>(cpu.ax));
    // 0046ef25  e8668a0600             -call 0x4d7990
    cpu.esp -= 4;
    nfs3hp::port::shapes::drawShape(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0046ef2a:
    // 0046ef2a  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046ef2d  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046ef30  8b4042                 -mov eax, dword ptr [eax + 0x42]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(66) /* 0x42 */);
    // 0046ef33  8b5278                 -mov edx, dword ptr [edx + 0x78]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(120) /* 0x78 */);
    // 0046ef36  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046ef39  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046ef3c  8b4dec                 -mov ecx, dword ptr [ebp - 0x14]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0046ef3f  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 0046ef41  39c8                   +cmp eax, ecx
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ecx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046ef43  7d30                   -jge 0x46ef75
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0046ef75;
    }
    // 0046ef45  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046ef48  83c066                 -add eax, 0x66
    (cpu.eax) += x86::reg32(x86::sreg32(102 /*0x66*/));
    // 0046ef4b  8b58fe                 -mov ebx, dword ptr [eax - 2]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-2) /* -0x2 */);
    // 0046ef4e  8b10                   -mov edx, dword ptr [eax]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax);
    // 0046ef50  e89bd20200             -call 0x49c1f0
    cpu.esp -= 4;
    nfs3hp::frontend::pointerInBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046ef55  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046ef58  c1fb10                 -sar ebx, 0x10
    cpu.ebx = x86::reg32(x86::sreg32(cpu.ebx) >> (16 /*0x10*/ % 32));
    // 0046ef5b  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046ef5d  7409                   -je 0x46ef68
    if (cpu.flags.zf)
    {
        goto L_0x0046ef68;
    }
    // 0046ef5f  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046ef62  668b4074               -mov ax, word ptr [eax + 0x74]
    cpu.ax = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(116) /* 0x74 */);
    // 0046ef66  eb07                   -jmp 0x46ef6f
    goto L_0x0046ef6f;
L_0x0046ef68:
    // 0046ef68  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046ef6b  668b4070               -mov ax, word ptr [eax + 0x70]
    cpu.ax = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(112) /* 0x70 */);
L_0x0046ef6f:
    // 0046ef6f  98                     -cwde 
    cpu.eax = x86::reg32(static_cast<x86::sreg16>(cpu.ax));
    // 0046ef70  e81b8a0600             -call 0x4d7990
    cpu.esp -= 4;
    nfs3hp::port::shapes::drawShape(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0046ef75:
    // 0046ef75  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0046ef77  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046ef79  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0046ef7b  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046ef7c  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046ef7d  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046ef7e  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046ef7f  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046ef80  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_46ef90(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0046ef90  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0046ef91  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0046ef92  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0046ef93  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0046ef94  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0046ef95  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0046ef96  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0046ef98  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0046ef9b  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0046ef9d  ba19000000             -mov edx, 0x19
    cpu.edx = 25 /*0x19*/;
    // 0046efa2  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0046efa4  e8b7b6fdff             -call 0x44a660
    cpu.esp -= 4;
    nfs3hp::frontend::menuShownNow(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046efa9  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046efab  0f84e5000000           -je 0x46f096
    if (cpu.flags.zf)
    {
        goto L_0x0046f096;
    }
    // 0046efb1  a1307d6700             -mov eax, dword ptr [0x677d30]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6782256) /* 0x677d30 */);
    // 0046efb6  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0046efb9  b8c4ae5300             -mov eax, 0x53aec4
    cpu.eax = 5484228 /*0x53aec4*/;
    // 0046efbe  e8ed6d0600             -call 0x4d5db0
    cpu.esp -= 4;
    nfs3hp::port::shapes::shapeByName(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046efc3  6689416e               -mov word ptr [ecx + 0x6e], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(110) /* 0x6e */) = cpu.ax;
    // 0046efc7  b8ccae5300             -mov eax, 0x53aecc
    cpu.eax = 5484236 /*0x53aecc*/;
    // 0046efcc  e8df6d0600             -call 0x4d5db0
    cpu.esp -= 4;
    nfs3hp::port::shapes::shapeByName(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046efd1  66894170               -mov word ptr [ecx + 0x70], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(112) /* 0x70 */) = cpu.ax;
    // 0046efd5  b8d4ae5300             -mov eax, 0x53aed4
    cpu.eax = 5484244 /*0x53aed4*/;
    // 0046efda  e8d16d0600             -call 0x4d5db0
    cpu.esp -= 4;
    nfs3hp::port::shapes::shapeByName(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046efdf  66894172               -mov word ptr [ecx + 0x72], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(114) /* 0x72 */) = cpu.ax;
    // 0046efe3  b8dcae5300             -mov eax, 0x53aedc
    cpu.eax = 5484252 /*0x53aedc*/;
    // 0046efe8  e8c36d0600             -call 0x4d5db0
    cpu.esp -= 4;
    nfs3hp::port::shapes::shapeByName(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046efed  66894174               -mov word ptr [ecx + 0x74], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(116) /* 0x74 */) = cpu.ax;
    // 0046eff1  b8ccae5300             -mov eax, 0x53aecc
    cpu.eax = 5484236 /*0x53aecc*/;
    // 0046eff6  e8b56d0600             -call 0x4d5db0
    cpu.esp -= 4;
    nfs3hp::port::shapes::shapeByName(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046effb  66894176               -mov word ptr [ecx + 0x76], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(118) /* 0x76 */) = cpu.ax;
    // 0046efff  b8c4ae5300             -mov eax, 0x53aec4
    cpu.eax = 5484228 /*0x53aec4*/;
    // 0046f004  e8a76d0600             -call 0x4d5db0
    cpu.esp -= 4;
    nfs3hp::port::shapes::shapeByName(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046f009  66c7417c0200           -mov word ptr [ecx + 0x7c], 2
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(124) /* 0x7c */) = 2 /*0x2*/;
    // 0046f00f  66c7417a0000           -mov word ptr [ecx + 0x7a], 0
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(122) /* 0x7a */) = 0 /*0x0*/;
    // 0046f015  668b9180000000         -mov dx, word ptr [ecx + 0x80]
    cpu.dx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(128) /* 0x80 */);
    // 0046f01c  66894178               -mov word ptr [ecx + 0x78], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(120) /* 0x78 */) = cpu.ax;
    // 0046f020  6685d2                 +test dx, dx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg16>(cpu.dx & cpu.dx));
    // 0046f023  750f                   -jne 0x46f034
    if (!cpu.flags.zf)
    {
        goto L_0x0046f034;
    }
    // 0046f025  668b4106               -mov ax, word ptr [ecx + 6]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(6) /* 0x6 */);
    // 0046f029  6689517e               -mov word ptr [ecx + 0x7e], dx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(126) /* 0x7e */) = cpu.dx;
    // 0046f02d  66898180000000         -mov word ptr [ecx + 0x80], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(128) /* 0x80 */) = cpu.ax;
L_0x0046f034:
    // 0046f034  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 0046f039  ba04000000             -mov edx, 4
    cpu.edx = 4 /*0x4*/;
    // 0046f03e  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0046f040  e8cb6bfeff             -call 0x455c10
    cpu.esp -= 4;
    sub_455c10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046f045  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
L_0x0046f047:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0046f047  3b75fc                 +cmp esi, dword ptr [ebp - 4]
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046f04a  7d4a                   -jge 0x46f096
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0046f096;
    }
    // 0046f04c  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0046f04e  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0046f050  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0046f052  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0046f054  e8776cfeff             -call 0x455cd0
    cpu.esp -= 4;
    sub_455cd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046f059  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046f05b  7439                   -je 0x46f096
    if (cpu.flags.zf)
    {
        goto L_0x0046f096;
    }
    // 0046f05d  8b1df4d46f00           -mov ebx, dword ptr [0x6fd4f4]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */);
    // 0046f063  8b88f0010000           -mov ecx, dword ptr [eax + 0x1f0]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(496) /* 0x1f0 */);
    // 0046f069  39d9                   +cmp ecx, ebx
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046f06b  740f                   -je 0x46f07c
    if (cpu.flags.zf)
    {
        goto L_0x0046f07c;
    }
    // 0046f06d  8b15b0d36f00           -mov edx, dword ptr [0x6fd3b0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
    // 0046f073  83fa01                 +cmp edx, 1
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046f076  751b                   -jne 0x46f093
    if (!cpu.flags.zf)
    {
        goto L_0x0046f093;
    }
    // 0046f078  39d1                   +cmp ecx, edx
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046f07a  7517                   -jne 0x46f093
    if (!cpu.flags.zf)
    {
        goto L_0x0046f093;
    }
L_0x0046f07c:
    // 0046f07c  8b4742                 -mov eax, dword ptr [edi + 0x42]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(66) /* 0x42 */);
    // 0046f07f  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046f082  39c6                   +cmp esi, eax
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046f084  7c0d                   -jl 0x46f093
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0046f093;
    }
    // 0046f086  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0046f088  662b4744               +sub ax, word ptr [edi + 0x44]
    {
        x86::reg16& tmp1 = cpu.ax;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(app->getMemory<x86::reg16>(cpu.edi + x86::reg32(68) /* 0x44 */)));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0046f08c  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0046f08d  6689477a               -mov word ptr [edi + 0x7a], ax
    app->getMemory<x86::reg16>(cpu.edi + x86::reg32(122) /* 0x7a */) = cpu.ax;
    // 0046f091  eb03                   -jmp 0x46f096
    goto L_0x0046f096;
L_0x0046f093:
    // 0046f093  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0046f094  ebb1                   -jmp 0x46f047
    goto L_0x0046f047;
L_0x0046f096:
    // 0046f096  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0046f098  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046f099  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046f09a  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046f09b  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046f09c  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046f09d  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046f09e  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x90 */
void Application::sub_46f0a0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0046f0a0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0046f0a1  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0046f0a2  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0046f0a4  ba19000000             -mov edx, 0x19
    cpu.edx = 25 /*0x19*/;
    // 0046f0a9  e8b2b5fdff             -call 0x44a660
    cpu.esp -= 4;
    nfs3hp::frontend::menuShownNow(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046f0ae  85c0                   -test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046f0b0  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046f0b1  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046f0b2  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_46f0c0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0046f0c0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0046f0c1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0046f0c2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0046f0c3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0046f0c4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0046f0c5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0046f0c6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0046f0c8  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0046f0ca  ba19000000             -mov edx, 0x19
    cpu.edx = 25 /*0x19*/;
    // 0046f0cf  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0046f0d1  e88ab5fdff             -call 0x44a660
    cpu.esp -= 4;
    nfs3hp::frontend::menuShownNow(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046f0d6  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046f0d8  0f841b010000           -je 0x46f1f9
    if (cpu.flags.zf)
    {
        goto L_0x0046f1f9;
    }
    // 0046f0de  8b5342                 -mov edx, dword ptr [ebx + 0x42]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(66) /* 0x42 */);
    // 0046f0e1  a1307d6700             -mov eax, dword ptr [0x677d30]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6782256) /* 0x677d30 */);
    // 0046f0e6  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046f0e9  39c2                   +cmp edx, eax
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046f0eb  7c02                   -jl 0x46f0ef
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0046f0ef;
    }
    // 0046f0ed  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
L_0x0046f0ef:
    // 0046f0ef  8b593c                 -mov ebx, dword ptr [ecx + 0x3c]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(60) /* 0x3c */);
    // 0046f0f2  8b413a                 -mov eax, dword ptr [ecx + 0x3a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(58) /* 0x3a */);
    // 0046f0f5  c1fb10                 -sar ebx, 0x10
    cpu.ebx = x86::reg32(x86::sreg32(cpu.ebx) >> (16 /*0x10*/ % 32));
    // 0046f0f8  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046f0fb  01c3                   -add ebx, eax
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046f0fd  8b413e                 -mov eax, dword ptr [ecx + 0x3e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(62) /* 0x3e */);
    // 0046f100  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046f103  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 0046f105  8b5940                 -mov ebx, dword ptr [ecx + 0x40]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(64) /* 0x40 */);
    // 0046f108  42                     -inc edx
    (cpu.edx)++;
    // 0046f109  c1fb10                 -sar ebx, 0x10
    cpu.ebx = x86::reg32(x86::sreg32(cpu.ebx) >> (16 /*0x10*/ % 32));
    // 0046f10c  0fafd3                 -imul edx, ebx
    cpu.edx = x86::reg32(x86::sreg64(x86::sreg32(cpu.edx)) * x86::sreg64(x86::sreg32(cpu.ebx)));
    // 0046f10f  668b9980000000         -mov bx, word ptr [ecx + 0x80]
    cpu.bx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(128) /* 0x80 */);
    // 0046f116  66895948               -mov word ptr [ecx + 0x48], bx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(72) /* 0x48 */) = cpu.bx;
    // 0046f11a  668b5908               -mov bx, word ptr [ecx + 8]
    cpu.bx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(8) /* 0x8 */);
    // 0046f11e  66895946               -mov word ptr [ecx + 0x46], bx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(70) /* 0x46 */) = cpu.bx;
    // 0046f122  668b593c               -mov bx, word ptr [ecx + 0x3c]
    cpu.bx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(60) /* 0x3c */);
    // 0046f126  6689594a               -mov word ptr [ecx + 0x4a], bx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(74) /* 0x4a */) = cpu.bx;
    // 0046f12a  668b5942               -mov bx, word ptr [ecx + 0x42]
    cpu.bx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(66) /* 0x42 */);
    // 0046f12e  6689594c               -mov word ptr [ecx + 0x4c], bx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(76) /* 0x4c */) = cpu.bx;
    // 0046f132  668b713c               -mov si, word ptr [ecx + 0x3c]
    cpu.si = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(60) /* 0x3c */);
    // 0046f136  668b9980000000         -mov bx, word ptr [ecx + 0x80]
    cpu.bx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(128) /* 0x80 */);
    // 0046f13d  01f3                   -add ebx, esi
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.esi));
    // 0046f13f  66895950               -mov word ptr [ecx + 0x50], bx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(80) /* 0x50 */) = cpu.bx;
    // 0046f143  668b5908               -mov bx, word ptr [ecx + 8]
    cpu.bx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(8) /* 0x8 */);
    // 0046f147  6689594e               -mov word ptr [ecx + 0x4e], bx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(78) /* 0x4e */) = cpu.bx;
    // 0046f14b  668b593e               -mov bx, word ptr [ecx + 0x3e]
    cpu.bx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(62) /* 0x3e */);
    // 0046f14f  66895952               -mov word ptr [ecx + 0x52], bx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(82) /* 0x52 */) = cpu.bx;
    // 0046f153  668b5942               -mov bx, word ptr [ecx + 0x42]
    cpu.bx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(66) /* 0x42 */);
    // 0046f157  66895954               -mov word ptr [ecx + 0x54], bx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(84) /* 0x54 */) = cpu.bx;
    // 0046f15b  668b793c               -mov di, word ptr [ecx + 0x3c]
    cpu.di = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(60) /* 0x3c */);
    // 0046f15f  668b9980000000         -mov bx, word ptr [ecx + 0x80]
    cpu.bx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(128) /* 0x80 */);
    // 0046f166  668b713e               -mov si, word ptr [ecx + 0x3e]
    cpu.si = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(62) /* 0x3e */);
    // 0046f16a  01fb                   -add ebx, edi
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.edi));
    // 0046f16c  01f3                   -add ebx, esi
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.esi));
    // 0046f16e  66895958               -mov word ptr [ecx + 0x58], bx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(88) /* 0x58 */) = cpu.bx;
    // 0046f172  668b5908               -mov bx, word ptr [ecx + 8]
    cpu.bx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(8) /* 0x8 */);
    // 0046f176  66895956               -mov word ptr [ecx + 0x56], bx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(86) /* 0x56 */) = cpu.bx;
    // 0046f17a  668b5940               -mov bx, word ptr [ecx + 0x40]
    cpu.bx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(64) /* 0x40 */);
    // 0046f17e  6689595a               -mov word ptr [ecx + 0x5a], bx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(90) /* 0x5a */) = cpu.bx;
    // 0046f182  668b5942               -mov bx, word ptr [ecx + 0x42]
    cpu.bx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(66) /* 0x42 */);
    // 0046f186  6689595c               -mov word ptr [ecx + 0x5c], bx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(92) /* 0x5c */) = cpu.bx;
    // 0046f18a  668b9980000000         -mov bx, word ptr [ecx + 0x80]
    cpu.bx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(128) /* 0x80 */);
    // 0046f191  6689591c               -mov word ptr [ecx + 0x1c], bx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(28) /* 0x1c */) = cpu.bx;
    // 0046f195  668b5908               -mov bx, word ptr [ecx + 8]
    cpu.bx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(8) /* 0x8 */);
    // 0046f199  6689591a               -mov word ptr [ecx + 0x1a], bx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(26) /* 0x1a */) = cpu.bx;
    // 0046f19d  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0046f19f  83c320                 -add ebx, 0x20
    (cpu.ebx) += x86::reg32(x86::sreg32(32 /*0x20*/));
    // 0046f1a2  6689591e               -mov word ptr [ecx + 0x1e], bx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(30) /* 0x1e */) = cpu.bx;
    // 0046f1a6  668b9980000000         -mov bx, word ptr [ecx + 0x80]
    cpu.bx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(128) /* 0x80 */);
    // 0046f1ad  01c3                   -add ebx, eax
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046f1af  66c741620c00           -mov word ptr [ecx + 0x62], 0xc
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(98) /* 0x62 */) = 12 /*0xc*/;
    // 0046f1b5  83c314                 -add ebx, 0x14
    (cpu.ebx) += x86::reg32(x86::sreg32(20 /*0x14*/));
    // 0046f1b8  66895960               -mov word ptr [ecx + 0x60], bx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(96) /* 0x60 */) = cpu.bx;
    // 0046f1bc  668b5908               -mov bx, word ptr [ecx + 8]
    cpu.bx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(8) /* 0x8 */);
    // 0046f1c0  6689595e               -mov word ptr [ecx + 0x5e], bx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(94) /* 0x5e */) = cpu.bx;
    // 0046f1c4  668b9980000000         -mov bx, word ptr [ecx + 0x80]
    cpu.bx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(128) /* 0x80 */);
    // 0046f1cb  66c741640c00           -mov word ptr [ecx + 0x64], 0xc
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(100) /* 0x64 */) = 12 /*0xc*/;
    // 0046f1d1  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 0046f1d3  66c7416a0c00           -mov word ptr [ecx + 0x6a], 0xc
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(106) /* 0x6a */) = 12 /*0xc*/;
    // 0046f1d9  0514000000             -add eax, 0x14
    (cpu.eax) += x86::reg32(x86::sreg32(20 /*0x14*/));
    // 0046f1de  66894168               -mov word ptr [ecx + 0x68], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(104) /* 0x68 */) = cpu.ax;
    // 0046f1e2  668b4108               -mov ax, word ptr [ecx + 8]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(8) /* 0x8 */);
    // 0046f1e6  66c7416c0c00           -mov word ptr [ecx + 0x6c], 0xc
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(108) /* 0x6c */) = 12 /*0xc*/;
    // 0046f1ec  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 0046f1ee  66895120               -mov word ptr [ecx + 0x20], dx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(32) /* 0x20 */) = cpu.dx;
    // 0046f1f2  83e80c                 -sub eax, 0xc
    (cpu.eax) -= x86::reg32(x86::sreg32(12 /*0xc*/));
    // 0046f1f5  66894166               -mov word ptr [ecx + 0x66], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(102) /* 0x66 */) = cpu.ax;
L_0x0046f1f9:
    // 0046f1f9  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046f1fa  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046f1fb  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046f1fc  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046f1fd  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046f1fe  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046f1ff  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_46f200(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0046f200  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0046f201  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0046f202  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0046f203  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0046f204  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0046f206  83ec0c                 -sub esp, 0xc
    (cpu.esp) -= x86::reg32(x86::sreg32(12 /*0xc*/));
    // 0046f209  89de                   -mov esi, ebx
    cpu.esi = cpu.ebx;
    // 0046f20b  bbffffffff             -mov ebx, 0xffffffff
    cpu.ebx = 4294967295 /*0xffffffff*/;
    // 0046f210  ba19000000             -mov edx, 0x19
    cpu.edx = 25 /*0x19*/;
    // 0046f215  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0046f217  e844b4fdff             -call 0x44a660
    cpu.esp -= 4;
    nfs3hp::frontend::menuShownNow(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046f21c  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 0046f21e  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046f220  7507                   -jne 0x46f229
    if (!cpu.flags.zf)
    {
        goto L_0x0046f229;
    }
    // 0046f222  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 0046f224  e98b020000             -jmp 0x46f4b4
    goto L_0x0046f4b4;
L_0x0046f229:
    // 0046f229  e852fd0500             -call 0x4cef80
    cpu.esp -= 4;
    sub_4cef80(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046f22e  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 0046f231  a1307d6700             -mov eax, dword ptr [0x677d30]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6782256) /* 0x677d30 */);
    // 0046f236  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 0046f238  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0046f23b  eb06                   -jmp 0x46f243
    goto L_0x0046f243;
L_0x0046f23d:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0046f23d  42                     -inc edx
    (cpu.edx)++;
    // 0046f23e  83fa03                 +cmp edx, 3
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046f241  7d1b                   -jge 0x46f25e
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0046f25e;
    }
L_0x0046f243:
    // 0046f243  8d04d500000000         -lea eax, [edx*8]
    cpu.eax = x86::reg32(cpu.edx * 8);
    // 0046f24a  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
    // 0046f24d  8d4146                 -lea eax, [ecx + 0x46]
    cpu.eax = x86::reg32(cpu.ecx + x86::reg32(70) /* 0x46 */);
    // 0046f250  0345f4                 -add eax, dword ptr [ebp - 0xc]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */)));
    // 0046f253  e898cf0200             -call 0x49c1f0
    cpu.esp -= 4;
    nfs3hp::frontend::pointerInBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046f258  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046f25a  74e1                   -je 0x46f23d
    if (cpu.flags.zf)
    {
        goto L_0x0046f23d;
    }
    // 0046f25c  89d3                   -mov ebx, edx
    cpu.ebx = cpu.edx;
L_0x0046f25e:
    // 0046f25e  6683fe0d               +cmp si, 0xd
    {
        x86::reg16 tmp1 = cpu.si;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(13 /*0xd*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046f262  7522                   -jne 0x46f286
    if (!cpu.flags.zf)
    {
        goto L_0x0046f286;
    }
    // 0046f264  837df800               +cmp dword ptr [ebp - 8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046f268  751c                   -jne 0x46f286
    if (!cpu.flags.zf)
    {
        goto L_0x0046f286;
    }
    // 0046f26a  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0046f26c  7c18                   -jl 0x46f286
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0046f286;
    }
    // 0046f26e  ba7f000000             -mov edx, 0x7f
    cpu.edx = 127 /*0x7f*/;
    // 0046f273  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0046f278  bf02000000             -mov edi, 2
    cpu.edi = 2 /*0x2*/;
    // 0046f27d  6689597c               -mov word ptr [ecx + 0x7c], bx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(124) /* 0x7c */) = cpu.bx;
    // 0046f281  e84a8ffaff             -call 0x4181d0
    cpu.esp -= 4;
    nfs3hp::port::playMenuEffect(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0046f286:
    // 0046f286  837df800               +cmp dword ptr [ebp - 8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046f28a  0f858b000000           -jne 0x46f31b
    if (!cpu.flags.zf)
    {
        goto L_0x0046f31b;
    }
    // 0046f290  6681fe004d             +cmp si, 0x4d00
    {
        x86::reg16 tmp1 = cpu.si;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(19712 /*0x4d00*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046f295  740b                   -je 0x46f2a2
    if (cpu.flags.zf)
    {
        goto L_0x0046f2a2;
    }
    // 0046f297  6681fe004b             +cmp si, 0x4b00
    {
        x86::reg16 tmp1 = cpu.si;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(19200 /*0x4b00*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046f29c  0f8579000000           -jne 0x46f31b
    if (!cpu.flags.zf)
    {
        goto L_0x0046f31b;
    }
L_0x0046f2a2:
    // 0046f2a2  6681fe004d             +cmp si, 0x4d00
    {
        x86::reg16 tmp1 = cpu.si;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(19712 /*0x4d00*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046f2a7  7503                   -jne 0x46f2ac
    if (!cpu.flags.zf)
    {
        goto L_0x0046f2ac;
    }
    // 0046f2a9  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0046f2aa  eb01                   -jmp 0x46f2ad
    goto L_0x0046f2ad;
L_0x0046f2ac:
    // 0046f2ac  4b                     -dec ebx
    (cpu.ebx)--;
L_0x0046f2ad:
    // 0046f2ad  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0046f2af  7d04                   -jge 0x46f2b5
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0046f2b5;
    }
    // 0046f2b1  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 0046f2b3  eb0a                   -jmp 0x46f2bf
    goto L_0x0046f2bf;
L_0x0046f2b5:
    // 0046f2b5  83fb02                 +cmp ebx, 2
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046f2b8  7e05                   -jle 0x46f2bf
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0046f2bf;
    }
    // 0046f2ba  bb02000000             -mov ebx, 2
    cpu.ebx = 2 /*0x2*/;
L_0x0046f2bf:
    // 0046f2bf  c1e303                 -shl ebx, 3
    cpu.ebx <<= 3 /*0x3*/ % 32;
    // 0046f2c2  8d3c19                 -lea edi, [ecx + ebx]
    cpu.edi = x86::reg32(cpu.ecx + cpu.ebx * 1);
    // 0046f2c5  8b4748                 -mov eax, dword ptr [edi + 0x48]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(72) /* 0x48 */);
    // 0046f2c8  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046f2cb  8d148500000000         -lea edx, [eax*4]
    cpu.edx = x86::reg32(cpu.eax * 4);
    // 0046f2d2  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 0046f2d4  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046f2d6  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046f2d9  c1e202                 +shl edx, 2
    {
        x86::reg8 tmp = 2 /*0x2*/ % 32;
        x86::reg32& op = cpu.edx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 0046f2dc  1bc2                   -sbb eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 0046f2de  c1f802                 -sar eax, 2
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (2 /*0x2*/ % 32));
    // 0046f2e1  8b5746                 -mov edx, dword ptr [edi + 0x46]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(70) /* 0x46 */);
    // 0046f2e4  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046f2e7  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046f2e9  8915e4e55500           -mov dword ptr [0x55e5e4], edx
    app->getMemory<x86::reg32>(x86::reg32(5629412) /* 0x55e5e4 */) = cpu.edx;
    // 0046f2ef  8b574a                 -mov edx, dword ptr [edi + 0x4a]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(74) /* 0x4a */);
    // 0046f2f2  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046f2f5  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046f2f7  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046f2fa  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0046f2fc  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0046f2fe  8b5744                 -mov edx, dword ptr [edi + 0x44]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(68) /* 0x44 */);
    // 0046f301  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046f304  bf02000000             -mov edi, 2
    cpu.edi = 2 /*0x2*/;
    // 0046f309  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046f30b  a1e4e55500             -mov eax, dword ptr [0x55e5e4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629412) /* 0x55e5e4 */);
    // 0046f310  8915e8e55500           -mov dword ptr [0x55e5e8], edx
    app->getMemory<x86::reg32>(x86::reg32(5629416) /* 0x55e5e8 */) = cpu.edx;
    // 0046f316  e8a5cc0200             -call 0x49bfc0
    cpu.esp -= 4;
    nfs3hp::frontend::setPointer(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0046f31b:
    // 0046f31b  8b4142                 -mov eax, dword ptr [ecx + 0x42]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(66) /* 0x42 */);
    // 0046f31e  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046f321  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046f324  39d0                   +cmp eax, edx
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046f326  0f8d86010000           -jge 0x46f4b2
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0046f4b2;
    }
    // 0046f32c  6681fe0049             +cmp si, 0x4900
    {
        x86::reg16 tmp1 = cpu.si;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(18688 /*0x4900*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046f331  0f8575000000           -jne 0x46f3ac
    if (!cpu.flags.zf)
    {
        goto L_0x0046f3ac;
    }
    // 0046f337  6683797a00             +cmp word ptr [ecx + 0x7a], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(122) /* 0x7a */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046f33c  746e                   -je 0x46f3ac
    if (cpu.flags.zf)
    {
        goto L_0x0046f3ac;
    }
    // 0046f33e  ba7f000000             -mov edx, 0x7f
    cpu.edx = 127 /*0x7f*/;
    // 0046f343  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0046f348  e8838efaff             -call 0x4181d0
    cpu.esp -= 4;
    nfs3hp::port::playMenuEffect(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046f34d  8b4160                 -mov eax, dword ptr [ecx + 0x60]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(96) /* 0x60 */);
    // 0046f350  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046f353  8d148500000000         -lea edx, [eax*4]
    cpu.edx = x86::reg32(cpu.eax * 4);
    // 0046f35a  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 0046f35c  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046f35e  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046f361  c1e202                 +shl edx, 2
    {
        x86::reg8 tmp = 2 /*0x2*/ % 32;
        x86::reg32& op = cpu.edx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 0046f364  1bc2                   -sbb eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 0046f366  c1f802                 -sar eax, 2
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (2 /*0x2*/ % 32));
    // 0046f369  8b515e                 -mov edx, dword ptr [ecx + 0x5e]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(94) /* 0x5e */);
    // 0046f36c  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046f36f  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046f371  8915e4e55500           -mov dword ptr [0x55e5e4], edx
    app->getMemory<x86::reg32>(x86::reg32(5629412) /* 0x55e5e4 */) = cpu.edx;
    // 0046f377  8b5162                 -mov edx, dword ptr [ecx + 0x62]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(98) /* 0x62 */);
    // 0046f37a  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046f37d  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046f37f  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046f382  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0046f384  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0046f386  bf02000000             -mov edi, 2
    cpu.edi = 2 /*0x2*/;
    // 0046f38b  668b597a               -mov bx, word ptr [ecx + 0x7a]
    cpu.bx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(122) /* 0x7a */);
    // 0046f38f  8b515c                 -mov edx, dword ptr [ecx + 0x5c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(92) /* 0x5c */);
    // 0046f392  4b                     -dec ebx
    (cpu.ebx)--;
    // 0046f393  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046f396  6689597a               -mov word ptr [ecx + 0x7a], bx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(122) /* 0x7a */) = cpu.bx;
    // 0046f39a  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046f39c  a1e4e55500             -mov eax, dword ptr [0x55e5e4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629412) /* 0x55e5e4 */);
    // 0046f3a1  8915e8e55500           -mov dword ptr [0x55e5e8], edx
    app->getMemory<x86::reg32>(x86::reg32(5629416) /* 0x55e5e8 */) = cpu.edx;
    // 0046f3a7  e814cc0200             -call 0x49bfc0
    cpu.esp -= 4;
    nfs3hp::frontend::setPointer(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0046f3ac:
    // 0046f3ac  6681fe0051             +cmp si, 0x5100
    {
        x86::reg16 tmp1 = cpu.si;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(20736 /*0x5100*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046f3b1  0f8583000000           -jne 0x46f43a
    if (!cpu.flags.zf)
    {
        goto L_0x0046f43a;
    }
    // 0046f3b7  8b4142                 -mov eax, dword ptr [ecx + 0x42]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(66) /* 0x42 */);
    // 0046f3ba  8b5178                 -mov edx, dword ptr [ecx + 0x78]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(120) /* 0x78 */);
    // 0046f3bd  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046f3c0  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046f3c3  8b5dfc                 -mov ebx, dword ptr [ebp - 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046f3c6  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 0046f3c8  39d8                   +cmp eax, ebx
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046f3ca  7d6e                   -jge 0x46f43a
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0046f43a;
    }
    // 0046f3cc  ba7f000000             -mov edx, 0x7f
    cpu.edx = 127 /*0x7f*/;
    // 0046f3d1  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0046f3d6  e8f58dfaff             -call 0x4181d0
    cpu.esp -= 4;
    nfs3hp::port::playMenuEffect(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046f3db  8b4168                 -mov eax, dword ptr [ecx + 0x68]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(104) /* 0x68 */);
    // 0046f3de  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046f3e1  8d148500000000         -lea edx, [eax*4]
    cpu.edx = x86::reg32(cpu.eax * 4);
    // 0046f3e8  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 0046f3ea  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046f3ec  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046f3ef  c1e202                 +shl edx, 2
    {
        x86::reg8 tmp = 2 /*0x2*/ % 32;
        x86::reg32& op = cpu.edx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 0046f3f2  1bc2                   -sbb eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 0046f3f4  c1f802                 -sar eax, 2
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (2 /*0x2*/ % 32));
    // 0046f3f7  8b5166                 -mov edx, dword ptr [ecx + 0x66]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(102) /* 0x66 */);
    // 0046f3fa  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046f3fd  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046f3ff  8915e4e55500           -mov dword ptr [0x55e5e4], edx
    app->getMemory<x86::reg32>(x86::reg32(5629412) /* 0x55e5e4 */) = cpu.edx;
    // 0046f405  8b516a                 -mov edx, dword ptr [ecx + 0x6a]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(106) /* 0x6a */);
    // 0046f408  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046f40b  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046f40d  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046f410  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0046f412  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0046f414  668b797a               -mov di, word ptr [ecx + 0x7a]
    cpu.di = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(122) /* 0x7a */);
    // 0046f418  47                     -inc edi
    (cpu.edi)++;
    // 0046f419  8b5164                 -mov edx, dword ptr [ecx + 0x64]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(100) /* 0x64 */);
    // 0046f41c  6689797a               -mov word ptr [ecx + 0x7a], di
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(122) /* 0x7a */) = cpu.di;
    // 0046f420  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046f423  bf02000000             -mov edi, 2
    cpu.edi = 2 /*0x2*/;
    // 0046f428  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046f42a  a1e4e55500             -mov eax, dword ptr [0x55e5e4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629412) /* 0x55e5e4 */);
    // 0046f42f  8915e8e55500           -mov dword ptr [0x55e5e8], edx
    app->getMemory<x86::reg32>(x86::reg32(5629416) /* 0x55e5e8 */) = cpu.edx;
    // 0046f435  e886cb0200             -call 0x49bfc0
    cpu.esp -= 4;
    nfs3hp::frontend::setPointer(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0046f43a:
    // 0046f43a  6683fe0d               +cmp si, 0xd
    {
        x86::reg16 tmp1 = cpu.si;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(13 /*0xd*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046f43e  0f856e000000           -jne 0x46f4b2
    if (!cpu.flags.zf)
    {
        goto L_0x0046f4b2;
    }
    // 0046f444  8d415e                 -lea eax, [ecx + 0x5e]
    cpu.eax = x86::reg32(cpu.ecx + x86::reg32(94) /* 0x5e */);
    // 0046f447  e8a4cd0200             -call 0x49c1f0
    cpu.esp -= 4;
    nfs3hp::frontend::pointerInBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046f44c  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046f44e  7424                   -je 0x46f474
    if (cpu.flags.zf)
    {
        goto L_0x0046f474;
    }
    // 0046f450  6683797a00             +cmp word ptr [ecx + 0x7a], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(122) /* 0x7a */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046f455  741d                   -je 0x46f474
    if (cpu.flags.zf)
    {
        goto L_0x0046f474;
    }
    // 0046f457  ba7f000000             -mov edx, 0x7f
    cpu.edx = 127 /*0x7f*/;
    // 0046f45c  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0046f461  e86a8dfaff             -call 0x4181d0
    cpu.esp -= 4;
    nfs3hp::port::playMenuEffect(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046f466  668b517a               -mov dx, word ptr [ecx + 0x7a]
    cpu.dx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(122) /* 0x7a */);
    // 0046f46a  4a                     -dec edx
    (cpu.edx)--;
    // 0046f46b  bf02000000             -mov edi, 2
    cpu.edi = 2 /*0x2*/;
    // 0046f470  6689517a               -mov word ptr [ecx + 0x7a], dx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(122) /* 0x7a */) = cpu.dx;
L_0x0046f474:
    // 0046f474  8d4166                 -lea eax, [ecx + 0x66]
    cpu.eax = x86::reg32(cpu.ecx + x86::reg32(102) /* 0x66 */);
    // 0046f477  e874cd0200             -call 0x49c1f0
    cpu.esp -= 4;
    nfs3hp::frontend::pointerInBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046f47c  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046f47e  7432                   -je 0x46f4b2
    if (cpu.flags.zf)
    {
        goto L_0x0046f4b2;
    }
    // 0046f480  8b5142                 -mov edx, dword ptr [ecx + 0x42]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(66) /* 0x42 */);
    // 0046f483  8b4178                 -mov eax, dword ptr [ecx + 0x78]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(120) /* 0x78 */);
    // 0046f486  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046f489  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046f48c  8b75fc                 -mov esi, dword ptr [ebp - 4]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046f48f  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 0046f491  39f0                   +cmp eax, esi
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.esi));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046f493  7d1d                   -jge 0x46f4b2
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0046f4b2;
    }
    // 0046f495  ba7f000000             -mov edx, 0x7f
    cpu.edx = 127 /*0x7f*/;
    // 0046f49a  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0046f49f  e82c8dfaff             -call 0x4181d0
    cpu.esp -= 4;
    nfs3hp::port::playMenuEffect(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046f4a4  668b597a               -mov bx, word ptr [ecx + 0x7a]
    cpu.bx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(122) /* 0x7a */);
    // 0046f4a8  43                     -inc ebx
    (cpu.ebx)++;
    // 0046f4a9  bf02000000             -mov edi, 2
    cpu.edi = 2 /*0x2*/;
    // 0046f4ae  6689597a               -mov word ptr [ecx + 0x7a], bx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(122) /* 0x7a */) = cpu.bx;
L_0x0046f4b2:
    // 0046f4b2  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
L_0x0046f4b4:
    // 0046f4b4  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046f4b6  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0046f4b8  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046f4b9  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046f4ba  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046f4bb  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046f4bc  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_46f4c0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0046f4c0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0046f4c1  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0046f4c2  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0046f4c4  ba19000000             -mov edx, 0x19
    cpu.edx = 25 /*0x19*/;
    // 0046f4c9  e892b1fdff             -call 0x44a660
    cpu.esp -= 4;
    nfs3hp::frontend::menuShownNow(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046f4ce  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046f4d0  7402                   -je 0x46f4d4
    if (cpu.flags.zf)
    {
        goto L_0x0046f4d4;
    }
    // 0046f4d2  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x0046f4d4:
    // 0046f4d4  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046f4d5  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046f4d6  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_46f4e0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0046f4e0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0046f4e1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0046f4e2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0046f4e3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0046f4e5  8b5074                 -mov edx, dword ptr [eax + 0x74]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(116) /* 0x74 */);
    // 0046f4e8  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046f4eb  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0046f4ec  8b507a                 -mov edx, dword ptr [eax + 0x7a]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(122) /* 0x7a */);
    // 0046f4ef  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046f4f2  8b4cd04a               -mov ecx, dword ptr [eax + edx*8 + 0x4a]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(74) /* 0x4a */ + cpu.edx * 8);
    // 0046f4f6  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0046f4f7  8b5cd046               -mov ebx, dword ptr [eax + edx*8 + 0x46]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(70) /* 0x46 */ + cpu.edx * 8);
    // 0046f4fb  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0046f4fc  e8dfd7ffff             -call 0x46cce0
    cpu.esp -= 4;
    sub_46cce0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046f501  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046f502  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046f503  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046f504  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_46f510(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0046f510  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0046f511  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0046f512  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0046f513  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0046f514  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0046f515  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0046f517  81ec6c010000           -sub esp, 0x16c
    (cpu.esp) -= x86::reg32(x86::sreg32(364 /*0x16c*/));
    // 0046f51d  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0046f51f  8955a8                 -mov dword ptr [ebp - 0x58], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-88) /* -0x58 */) = cpu.edx;
    // 0046f522  ba1a000000             -mov edx, 0x1a
    cpu.edx = 26 /*0x1a*/;
    // 0046f527  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0046f52a  e831b1fdff             -call 0x44a660
    cpu.esp -= 4;
    nfs3hp::frontend::menuShownNow(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046f52f  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046f531  0f84e5080000           -je 0x46fe1c
    if (cpu.flags.zf)
    {
        goto L_0x0046fe1c;
    }
    // 0046f537  66837e7e02             +cmp word ptr [esi + 0x7e], 2
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.esi + x86::reg32(126) /* 0x7e */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(2 /*0x2*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046f53c  7527                   -jne 0x46f565
    if (!cpu.flags.zf)
    {
        goto L_0x0046f565;
    }
    // 0046f53e  668b9e80000000         -mov bx, word ptr [esi + 0x80]
    cpu.bx = app->getMemory<x86::reg16>(cpu.esi + x86::reg32(128) /* 0x80 */);
    // 0046f545  83c314                 -add ebx, 0x14
    (cpu.ebx) += x86::reg32(x86::sreg32(20 /*0x14*/));
    // 0046f548  66899e80000000         -mov word ptr [esi + 0x80], bx
    app->getMemory<x86::reg16>(cpu.esi + x86::reg32(128) /* 0x80 */) = cpu.bx;
    // 0046f54f  6681fba802             +cmp bx, 0x2a8
    {
        x86::reg16 tmp1 = cpu.bx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(680 /*0x2a8*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046f554  7c0f                   -jl 0x46f565
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0046f565;
    }
    // 0046f556  66c78680000000a802     -mov word ptr [esi + 0x80], 0x2a8
    app->getMemory<x86::reg16>(cpu.esi + x86::reg32(128) /* 0x80 */) = 680 /*0x2a8*/;
    // 0046f55f  66c7467e0300           -mov word ptr [esi + 0x7e], 3
    app->getMemory<x86::reg16>(cpu.esi + x86::reg32(126) /* 0x7e */) = 3 /*0x3*/;
L_0x0046f565:
    // 0046f565  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046f568  6683787e01             +cmp word ptr [eax + 0x7e], 1
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(126) /* 0x7e */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(1 /*0x1*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046f56d  752f                   -jne 0x46f59e
    if (!cpu.flags.zf)
    {
        goto L_0x0046f59e;
    }
    // 0046f56f  6683a88000000014       -sub word ptr [eax + 0x80], 0x14
    (app->getMemory<x86::reg16>(cpu.eax + x86::reg32(128) /* 0x80 */)) -= x86::reg16(x86::sreg16(20 /*0x14*/));
    // 0046f577  8b507e                 -mov edx, dword ptr [eax + 0x7e]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(126) /* 0x7e */);
    // 0046f57a  8b4004                 -mov eax, dword ptr [eax + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 0046f57d  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046f580  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046f583  39c2                   +cmp edx, eax
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046f585  7f17                   -jg 0x46f59e
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x0046f59e;
    }
    // 0046f587  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046f58a  66c7407e0000           -mov word ptr [eax + 0x7e], 0
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(126) /* 0x7e */) = 0 /*0x0*/;
    // 0046f590  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046f593  668b4006               -mov ax, word ptr [eax + 6]
    cpu.ax = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(6) /* 0x6 */);
    // 0046f597  66898280000000         -mov word ptr [edx + 0x80], ax
    app->getMemory<x86::reg16>(cpu.edx + x86::reg32(128) /* 0x80 */) = cpu.ax;
L_0x0046f59e:
    // 0046f59e  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046f5a1  6683787e03             +cmp word ptr [eax + 0x7e], 3
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(126) /* 0x7e */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(3 /*0x3*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046f5a6  0f8470080000           -je 0x46fe1c
    if (cpu.flags.zf)
    {
        goto L_0x0046fe1c;
    }
    // 0046f5ac  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0046f5ae  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046f5b1  e84a090000             -call 0x46ff00
    cpu.esp -= 4;
    sub_46ff00(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046f5b6  e835a8fdff             -call 0x449df0
    cpu.esp -= 4;
    nfs3hp::port::loop::keysTaken(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046f5bb  8945e0                 -mov dword ptr [ebp - 0x20], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */) = cpu.eax;
    // 0046f5be  e8bdf90500             -call 0x4cef80
    cpu.esp -= 4;
    sub_4cef80(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046f5c3  668b527c               -mov dx, word ptr [edx + 0x7c]
    cpu.dx = app->getMemory<x86::reg16>(cpu.edx + x86::reg32(124) /* 0x7c */);
    // 0046f5c7  8945bc                 -mov dword ptr [ebp - 0x44], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-68) /* -0x44 */) = cpu.eax;
    // 0046f5ca  6683fa01               +cmp dx, 1
    {
        x86::reg16 tmp1 = cpu.dx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(1 /*0x1*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046f5ce  720a                   -jb 0x46f5da
    if (cpu.flags.cf)
    {
        goto L_0x0046f5da;
    }
    // 0046f5d0  7629                   -jbe 0x46f5fb
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0046f5fb;
    }
    // 0046f5d2  6683fa02               +cmp dx, 2
    {
        x86::reg16 tmp1 = cpu.dx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(2 /*0x2*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046f5d6  7415                   -je 0x46f5ed
    if (cpu.flags.zf)
    {
        goto L_0x0046f5ed;
    }
    // 0046f5d8  eb32                   -jmp 0x46f60c
    goto L_0x0046f60c;
L_0x0046f5da:
    // 0046f5da  6685d2                 +test dx, dx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg16>(cpu.dx & cpu.dx));
    // 0046f5dd  752d                   -jne 0x46f60c
    if (!cpu.flags.zf)
    {
        goto L_0x0046f60c;
    }
    // 0046f5df  ba08000000             -mov edx, 8
    cpu.edx = 8 /*0x8*/;
    // 0046f5e4  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0046f5e9  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 0046f5eb  eb1a                   -jmp 0x46f607
    goto L_0x0046f607;
L_0x0046f5ed:
    // 0046f5ed  ba07000000             -mov edx, 7
    cpu.edx = 7 /*0x7*/;
    // 0046f5f2  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0046f5f7  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 0046f5f9  eb0c                   -jmp 0x46f607
    goto L_0x0046f607;
L_0x0046f5fb:
    // 0046f5fb  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 0046f600  ba05000000             -mov edx, 5
    cpu.edx = 5 /*0x5*/;
    // 0046f605  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
L_0x0046f607:
    // 0046f607  e80466feff             -call 0x455c10
    cpu.esp -= 4;
    sub_455c10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0046f60c:
    // 0046f60c  a1347d6700             -mov eax, dword ptr [0x677d34]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6782260) /* 0x677d34 */);
    // 0046f611  8945e8                 -mov dword ptr [ebp - 0x18], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = cpu.eax;
    // 0046f614  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046f617  8b4042                 -mov eax, dword ptr [eax + 0x42]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(66) /* 0x42 */);
    // 0046f61a  8b55e8                 -mov edx, dword ptr [ebp - 0x18]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 0046f61d  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046f620  39d0                   +cmp eax, edx
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046f622  7c02                   -jl 0x46f626
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0046f626;
    }
    // 0046f624  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
L_0x0046f626:
    // 0046f626  8b75fc                 -mov esi, dword ptr [ebp - 4]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046f629  8945b8                 -mov dword ptr [ebp - 0x48], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-72) /* -0x48 */) = cpu.eax;
    // 0046f62c  8b7640                 -mov esi, dword ptr [esi + 0x40]
    cpu.esi = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(64) /* 0x40 */);
    // 0046f62f  40                     -inc eax
    (cpu.eax)++;
    // 0046f630  c1fe10                 -sar esi, 0x10
    cpu.esi = x86::reg32(x86::sreg32(cpu.esi) >> (16 /*0x10*/ % 32));
    // 0046f633  0faff0                 -imul esi, eax
    cpu.esi = x86::reg32(x86::sreg64(x86::sreg32(cpu.esi)) * x86::sreg64(x86::sreg32(cpu.eax)));
    // 0046f636  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046f639  8b4dfc                 -mov ecx, dword ptr [ebp - 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046f63c  8b523a                 -mov edx, dword ptr [edx + 0x3a]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(58) /* 0x3a */);
    // 0046f63f  8b493c                 -mov ecx, dword ptr [ecx + 0x3c]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(60) /* 0x3c */);
    // 0046f642  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046f645  c1f910                 -sar ecx, 0x10
    cpu.ecx = x86::reg32(x86::sreg32(cpu.ecx) >> (16 /*0x10*/ % 32));
    // 0046f648  01d1                   -add ecx, edx
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.edx));
    // 0046f64a  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046f64d  8b523e                 -mov edx, dword ptr [edx + 0x3e]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(62) /* 0x3e */);
    // 0046f650  bb2894d5ff             -mov ebx, 0xffd59428
    cpu.ebx = 4292187176 /*0xffd59428*/;
    // 0046f655  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046f658  68000000ff             -push 0xff000000
    app->getMemory<x86::reg32>(cpu.esp-4) = 4278190080 /*0xff000000*/;
    cpu.esp -= 4;
    // 0046f65d  8d3c11                 -lea edi, [ecx + edx]
    cpu.edi = x86::reg32(cpu.ecx + cpu.edx * 1);
    // 0046f660  895de4                 -mov dword ptr [ebp - 0x1c], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */) = cpu.ebx;
    // 0046f663  b9aa0000ff             -mov ecx, 0xff0000aa
    cpu.ecx = 4278190250 /*0xff0000aa*/;
    // 0046f668  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046f66b  8d4602                 -lea eax, [esi + 2]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(2) /* 0x2 */);
    // 0046f66e  8d5f02                 -lea ebx, [edi + 2]
    cpu.ebx = x86::reg32(cpu.edi + x86::reg32(2) /* 0x2 */);
    // 0046f671  8945a4                 -mov dword ptr [ebp - 0x5c], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-92) /* -0x5c */) = cpu.eax;
    // 0046f674  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046f677  894dec                 -mov dword ptr [ebp - 0x14], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.ecx;
    // 0046f67a  8b5206                 -mov edx, dword ptr [edx + 6]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(6) /* 0x6 */);
    // 0046f67d  8b407e                 -mov eax, dword ptr [eax + 0x7e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(126) /* 0x7e */);
    // 0046f680  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046f683  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046f686  8b4da4                 -mov ecx, dword ptr [ebp - 0x5c]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-92) /* -0x5c */);
    // 0046f689  e8a2920600             -call 0x4d8930
    cpu.esp -= 4;
    nfs3hp::port::draw2d::frame(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046f68e  68aa0000ff             -push 0xff0000aa
    app->getMemory<x86::reg32>(cpu.esp-4) = 4278190250 /*0xff0000aa*/;
    cpu.esp -= 4;
    // 0046f693  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046f696  b902000000             -mov ecx, 2
    cpu.ecx = 2 /*0x2*/;
    // 0046f69b  8d4704                 -lea eax, [edi + 4]
    cpu.eax = x86::reg32(cpu.edi + x86::reg32(4) /* 0x4 */);
    // 0046f69e  682894d5ff             -push 0xffd59428
    app->getMemory<x86::reg32>(cpu.esp-4) = 4292187176 /*0xffd59428*/;
    cpu.esp -= 4;
    // 0046f6a3  8945a4                 -mov dword ptr [ebp - 0x5c], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-92) /* -0x5c */) = cpu.eax;
    // 0046f6a6  8b5206                 -mov edx, dword ptr [edx + 6]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(6) /* 0x6 */);
    // 0046f6a9  682894d5ff             -push 0xffd59428
    app->getMemory<x86::reg32>(cpu.esp-4) = 4292187176 /*0xffd59428*/;
    cpu.esp -= 4;
    // 0046f6ae  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046f6b1  8b5da4                 -mov ebx, dword ptr [ebp - 0x5c]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-92) /* -0x5c */);
    // 0046f6b4  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046f6b7  8b407e                 -mov eax, dword ptr [eax + 0x7e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(126) /* 0x7e */);
    // 0046f6ba  68aa0000ff             -push 0xff0000aa
    app->getMemory<x86::reg32>(cpu.esp-4) = 4278190250 /*0xff0000aa*/;
    cpu.esp -= 4;
    // 0046f6bf  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046f6c2  83ea02                 -sub edx, 2
    (cpu.edx) -= x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0046f6c5  83e802                 -sub eax, 2
    (cpu.eax) -= x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0046f6c8  e893910600             -call 0x4d8860
    cpu.esp -= 4;
    nfs3hp::port::draw2d::drawShadedBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046f6cd  682894d5ff             -push 0xffd59428
    app->getMemory<x86::reg32>(cpu.esp-4) = 4292187176 /*0xffd59428*/;
    cpu.esp -= 4;
    // 0046f6d2  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046f6d5  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046f6d8  b902000000             -mov ecx, 2
    cpu.ecx = 2 /*0x2*/;
    // 0046f6dd  8b5da4                 -mov ebx, dword ptr [ebp - 0x5c]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-92) /* -0x5c */);
    // 0046f6e0  682894d5ff             -push 0xffd59428
    app->getMemory<x86::reg32>(cpu.esp-4) = 4292187176 /*0xffd59428*/;
    cpu.esp -= 4;
    // 0046f6e5  8b5206                 -mov edx, dword ptr [edx + 6]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(6) /* 0x6 */);
    // 0046f6e8  8b407e                 -mov eax, dword ptr [eax + 0x7e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(126) /* 0x7e */);
    // 0046f6eb  682894d5ff             -push 0xffd59428
    app->getMemory<x86::reg32>(cpu.esp-4) = 4292187176 /*0xffd59428*/;
    cpu.esp -= 4;
    // 0046f6f0  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046f6f3  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046f6f6  682894d5ff             -push 0xffd59428
    app->getMemory<x86::reg32>(cpu.esp-4) = 4292187176 /*0xffd59428*/;
    cpu.esp -= 4;
    // 0046f6fb  01f2                   -add edx, esi
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.esi));
    // 0046f6fd  83e802                 -sub eax, 2
    (cpu.eax) -= x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0046f700  83c202                 -add edx, 2
    (cpu.edx) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0046f703  e858910600             -call 0x4d8860
    cpu.esp -= 4;
    nfs3hp::port::draw2d::drawShadedBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046f708  682894d5ff             -push 0xffd59428
    app->getMemory<x86::reg32>(cpu.esp-4) = 4292187176 /*0xffd59428*/;
    cpu.esp -= 4;
    // 0046f70d  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046f710  bb02000000             -mov ebx, 2
    cpu.ebx = 2 /*0x2*/;
    // 0046f715  8d4604                 -lea eax, [esi + 4]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 0046f718  682894d5ff             -push 0xffd59428
    app->getMemory<x86::reg32>(cpu.esp-4) = 4292187176 /*0xffd59428*/;
    cpu.esp -= 4;
    // 0046f71d  8945b4                 -mov dword ptr [ebp - 0x4c], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-76) /* -0x4c */) = cpu.eax;
    // 0046f720  8b5206                 -mov edx, dword ptr [edx + 6]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(6) /* 0x6 */);
    // 0046f723  68aa0000ff             -push 0xff0000aa
    app->getMemory<x86::reg32>(cpu.esp-4) = 4278190250 /*0xff0000aa*/;
    cpu.esp -= 4;
    // 0046f728  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046f72b  8b4db4                 -mov ecx, dword ptr [ebp - 0x4c]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-76) /* -0x4c */);
    // 0046f72e  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046f731  8b407e                 -mov eax, dword ptr [eax + 0x7e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(126) /* 0x7e */);
    // 0046f734  68aa0000ff             -push 0xff0000aa
    app->getMemory<x86::reg32>(cpu.esp-4) = 4278190250 /*0xff0000aa*/;
    cpu.esp -= 4;
    // 0046f739  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046f73c  83ea02                 -sub edx, 2
    (cpu.edx) -= x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0046f73f  83e802                 -sub eax, 2
    (cpu.eax) -= x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0046f742  e819910600             -call 0x4d8860
    cpu.esp -= 4;
    nfs3hp::port::draw2d::drawShadedBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046f747  682894d5ff             -push 0xffd59428
    app->getMemory<x86::reg32>(cpu.esp-4) = 4292187176 /*0xffd59428*/;
    cpu.esp -= 4;
    // 0046f74c  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046f74f  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046f752  8b4db4                 -mov ecx, dword ptr [ebp - 0x4c]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-76) /* -0x4c */);
    // 0046f755  bb02000000             -mov ebx, 2
    cpu.ebx = 2 /*0x2*/;
    // 0046f75a  682894d5ff             -push 0xffd59428
    app->getMemory<x86::reg32>(cpu.esp-4) = 4292187176 /*0xffd59428*/;
    cpu.esp -= 4;
    // 0046f75f  8b5206                 -mov edx, dword ptr [edx + 6]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(6) /* 0x6 */);
    // 0046f762  8b407e                 -mov eax, dword ptr [eax + 0x7e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(126) /* 0x7e */);
    // 0046f765  682894d5ff             -push 0xffd59428
    app->getMemory<x86::reg32>(cpu.esp-4) = 4292187176 /*0xffd59428*/;
    cpu.esp -= 4;
    // 0046f76a  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046f76d  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046f770  682894d5ff             -push 0xffd59428
    app->getMemory<x86::reg32>(cpu.esp-4) = 4292187176 /*0xffd59428*/;
    cpu.esp -= 4;
    // 0046f775  83ea02                 -sub edx, 2
    (cpu.edx) -= x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0046f778  01f8                   -add eax, edi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edi));
    // 0046f77a  e8e1900600             -call 0x4d8860
    cpu.esp -= 4;
    nfs3hp::port::draw2d::drawShadedBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046f77f  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046f782  8b4042                 -mov eax, dword ptr [eax + 0x42]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(66) /* 0x42 */);
    // 0046f785  8b55e8                 -mov edx, dword ptr [ebp - 0x18]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 0046f788  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046f78b  39d0                   +cmp eax, edx
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046f78d  7d2a                   -jge 0x46f7b9
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0046f7b9;
    }
    // 0046f78f  682894d5ff             -push 0xffd59428
    app->getMemory<x86::reg32>(cpu.esp-4) = 4292187176 /*0xffd59428*/;
    cpu.esp -= 4;
    // 0046f794  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046f797  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046f79a  bb12000000             -mov ebx, 0x12
    cpu.ebx = 18 /*0x12*/;
    // 0046f79f  8d4e06                 -lea ecx, [esi + 6]
    cpu.ecx = x86::reg32(cpu.esi + x86::reg32(6) /* 0x6 */);
    // 0046f7a2  8b525c                 -mov edx, dword ptr [edx + 0x5c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(92) /* 0x5c */);
    // 0046f7a5  8b405e                 -mov eax, dword ptr [eax + 0x5e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(94) /* 0x5e */);
    // 0046f7a8  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046f7ab  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046f7ae  83ea03                 -sub edx, 3
    (cpu.edx) -= x86::reg32(x86::sreg32(3 /*0x3*/));
    // 0046f7b1  83e803                 -sub eax, 3
    (cpu.eax) -= x86::reg32(x86::sreg32(3 /*0x3*/));
    // 0046f7b4  e877910600             -call 0x4d8930
    cpu.esp -= 4;
    nfs3hp::port::draw2d::frame(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0046f7b9:
    // 0046f7b9  8b4dec                 -mov ecx, dword ptr [ebp - 0x14]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0046f7bc  8b5de4                 -mov ebx, dword ptr [ebp - 0x1c]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 0046f7bf  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046f7c2  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046f7c5  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0046f7c6  8b5206                 -mov edx, dword ptr [edx + 6]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(6) /* 0x6 */);
    // 0046f7c9  8b4040                 -mov eax, dword ptr [eax + 0x40]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(64) /* 0x40 */);
    // 0046f7cc  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0046f7cd  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046f7d0  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046f7d3  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0046f7d4  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046f7d6  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046f7d9  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0046f7da  4a                     -dec edx
    (cpu.edx)--;
    // 0046f7db  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 0046f7dd  8b407e                 -mov eax, dword ptr [eax + 0x7e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(126) /* 0x7e */);
    // 0046f7e0  b902000000             -mov ecx, 2
    cpu.ecx = 2 /*0x2*/;
    // 0046f7e5  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046f7e8  e873900600             -call 0x4d8860
    cpu.esp -= 4;
    nfs3hp::port::draw2d::drawShadedBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046f7ed  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046f7f0  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046f7f3  8b5206                 -mov edx, dword ptr [edx + 6]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(6) /* 0x6 */);
    // 0046f7f6  8b4040                 -mov eax, dword ptr [eax + 0x40]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(64) /* 0x40 */);
    // 0046f7f9  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046f7fc  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046f7ff  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
L_0x0046f801:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0046f801  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046f803  894dc8                 -mov dword ptr [ebp - 0x38], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-56) /* -0x38 */) = cpu.ecx;
    // 0046f806  8955cc                 -mov dword ptr [ebp - 0x34], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-52) /* -0x34 */) = cpu.edx;
    // 0046f809  8b45c8                 -mov eax, dword ptr [ebp - 0x38]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-56) /* -0x38 */);
    // 0046f80c  3b45b8                 +cmp eax, dword ptr [ebp - 0x48]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-72) /* -0x48 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046f80f  7d34                   -jge 0x46f845
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0046f845;
    }
    // 0046f811  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0046f814  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046f815  8b55e4                 -mov edx, dword ptr [ebp - 0x1c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 0046f818  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0046f819  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0046f81a  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046f81b  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046f81e  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 0046f823  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 0046f825  8b407e                 -mov eax, dword ptr [eax + 0x7e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(126) /* 0x7e */);
    // 0046f828  8b55cc                 -mov edx, dword ptr [ebp - 0x34]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-52) /* -0x34 */);
    // 0046f82b  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046f82e  e82d900600             -call 0x4d8860
    cpu.esp -= 4;
    nfs3hp::port::draw2d::drawShadedBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046f833  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046f836  8b55cc                 -mov edx, dword ptr [ebp - 0x34]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-52) /* -0x34 */);
    // 0046f839  8b4040                 -mov eax, dword ptr [eax + 0x40]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(64) /* 0x40 */);
    // 0046f83c  8b4dc8                 -mov ecx, dword ptr [ebp - 0x38]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-56) /* -0x38 */);
    // 0046f83f  c1f810                 +sar eax, 0x10
    {
        x86::reg8 tmp = 16 /*0x10*/ % 32;
        x86::reg32& op = cpu.eax;
        if (tmp)
        {
            cpu.flags.cf = 1 & (x86::sreg32(op) >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = 0;
            cpu.set_szp((op = x86::reg32(x86::sreg32(op) >> tmp)));
        }
    }
    // 0046f842  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0046f843  ebbc                   -jmp 0x46f801
    goto L_0x0046f801;
L_0x0046f845:
    // 0046f845  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046f848  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046f84b  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 0046f850  8b407e                 -mov eax, dword ptr [eax + 0x7e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(126) /* 0x7e */);
    // 0046f853  8b523a                 -mov edx, dword ptr [edx + 0x3a]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(58) /* 0x3a */);
    // 0046f856  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046f859  c1fa10                 +sar edx, 0x10
    {
        x86::reg8 tmp = 16 /*0x10*/ % 32;
        x86::reg32& op = cpu.edx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (x86::sreg32(op) >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = 0;
            cpu.set_szp((op = x86::reg32(x86::sreg32(op) >> tmp)));
        }
    }
    // 0046f85c  895ddc                 -mov dword ptr [ebp - 0x24], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = cpu.ebx;
    // 0046f85f  8d3c10                 -lea edi, [eax + edx]
    cpu.edi = x86::reg32(cpu.eax + cpu.edx * 1);
    // 0046f862  eb06                   -jmp 0x46f86a
    goto L_0x0046f86a;
L_0x0046f864:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0046f864  837ddc03               +cmp dword ptr [ebp - 0x24], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046f868  7d3c                   -jge 0x46f8a6
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0046f8a6;
    }
L_0x0046f86a:
    // 0046f86a  8b55e4                 -mov edx, dword ptr [ebp - 0x1c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 0046f86d  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0046f86e  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0046f86f  8b5dec                 -mov ebx, dword ptr [ebp - 0x14]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0046f872  89f1                   -mov ecx, esi
    cpu.ecx = cpu.esi;
    // 0046f874  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0046f875  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046f878  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0046f87a  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0046f87b  8b5206                 -mov edx, dword ptr [edx + 6]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(6) /* 0x6 */);
    // 0046f87e  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 0046f883  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046f886  e8d58f0600             -call 0x4d8860
    cpu.esp -= 4;
    nfs3hp::port::draw2d::drawShadedBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046f88b  8b45dc                 -mov eax, dword ptr [ebp - 0x24]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 0046f88e  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046f891  01c0                   -add eax, eax
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046f893  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 0046f895  8b4ddc                 -mov ecx, dword ptr [ebp - 0x24]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 0046f898  8b403a                 -mov eax, dword ptr [eax + 0x3a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(58) /* 0x3a */);
    // 0046f89b  41                     -inc ecx
    (cpu.ecx)++;
    // 0046f89c  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046f89f  894ddc                 -mov dword ptr [ebp - 0x24], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = cpu.ecx;
    // 0046f8a2  01c7                   +add edi, eax
    {
        x86::reg32& tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0046f8a4  ebbe                   -jmp 0x46f864
    goto L_0x0046f864;
L_0x0046f8a6:
    // 0046f8a6  837dbc00               +cmp dword ptr [ebp - 0x44], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-68) /* -0x44 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046f8aa  7511                   -jne 0x46f8bd
    if (!cpu.flags.zf)
    {
        goto L_0x0046f8bd;
    }
    // 0046f8ac  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046f8af  6683787e00             +cmp word ptr [eax + 0x7e], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(126) /* 0x7e */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046f8b4  7507                   -jne 0x46f8bd
    if (!cpu.flags.zf)
    {
        goto L_0x0046f8bd;
    }
    // 0046f8b6  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0046f8b8  e823fcffff             -call 0x46f4e0
    cpu.esp -= 4;
    sub_46f4e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0046f8bd:
    // 0046f8bd  31f6                   +xor esi, esi
    cpu.clear_co();
    cpu.set_szp((cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi))));
    // 0046f8bf  8975f8                 -mov dword ptr [ebp - 8], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.esi;
    // 0046f8c2  e910010000             -jmp 0x46f9d7
    goto L_0x0046f9d7;
L_0x0046f8c7:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0046f8c7  83c60a                 -add esi, 0xa
    (cpu.esi) += x86::reg32(x86::sreg32(10 /*0xa*/));
L_0x0046f8ca:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0046f8ca  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0046f8cd  8b4dfc                 -mov ecx, dword ptr [ebp - 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046f8d0  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 0046f8d3  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046f8d5  8b514a                 -mov edx, dword ptr [ecx + 0x4a]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(74) /* 0x4a */);
    // 0046f8d8  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046f8db  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046f8dd  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046f8e0  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0046f8e2  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0046f8e4  8b5144                 -mov edx, dword ptr [ecx + 0x44]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(68) /* 0x44 */);
    // 0046f8e7  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046f8ea  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 0046f8ec  83e805                 -sub eax, 5
    (cpu.eax) -= x86::reg32(x86::sreg32(5 /*0x5*/));
    // 0046f8ef  8945b0                 -mov dword ptr [ebp - 0x50], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-80) /* -0x50 */) = cpu.eax;
    // 0046f8f2  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0046f8f5  8b444592               -mov eax, dword ptr [ebp + eax*2 - 0x6e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-110) /* -0x6e */ + cpu.eax * 2);
    // 0046f8f9  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046f8fc  e84f1f0600             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046f901  8b15b8d36f00           -mov edx, dword ptr [0x6fd3b8]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7328696) /* 0x6fd3b8 */);
    // 0046f907  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0046f909  83fa03                 +cmp edx, 3
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046f90c  7511                   -jne 0x46f91f
    if (!cpu.flags.zf)
    {
        goto L_0x0046f91f;
    }
    // 0046f90e  3b55f8                 +cmp edx, dword ptr [ebp - 8]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046f911  750c                   -jne 0x46f91f
    if (!cpu.flags.zf)
    {
        goto L_0x0046f91f;
    }
    // 0046f913  b893010000             -mov eax, 0x193
    cpu.eax = 403 /*0x193*/;
    // 0046f918  e8331f0600             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046f91d  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
L_0x0046f91f:
    // 0046f91f  837da800               +cmp dword ptr [ebp - 0x58], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-88) /* -0x58 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046f923  7424                   -je 0x46f949
    if (cpu.flags.zf)
    {
        goto L_0x0046f949;
    }
    // 0046f925  837dbc00               +cmp dword ptr [ebp - 0x44], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-68) /* -0x44 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046f929  751e                   -jne 0x46f949
    if (!cpu.flags.zf)
    {
        goto L_0x0046f949;
    }
    // 0046f92b  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0046f92e  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046f931  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 0046f934  83c246                 -add edx, 0x46
    (cpu.edx) += x86::reg32(x86::sreg32(70 /*0x46*/));
    // 0046f937  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 0046f939  e8b2c80200             -call 0x49c1f0
    cpu.esp -= 4;
    nfs3hp::frontend::pointerInBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046f93e  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046f940  7407                   -je 0x46f949
    if (cpu.flags.zf)
    {
        goto L_0x0046f949;
    }
    // 0046f942  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0046f947  eb02                   -jmp 0x46f94b
    goto L_0x0046f94b;
L_0x0046f949:
    // 0046f949  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x0046f94b:
    // 0046f94b  8945ac                 -mov dword ptr [ebp - 0x54], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-84) /* -0x54 */) = cpu.eax;
    // 0046f94e  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046f950  7440                   -je 0x46f992
    if (cpu.flags.zf)
    {
        goto L_0x0046f992;
    }
    // 0046f952  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0046f955  8b4dfc                 -mov ecx, dword ptr [ebp - 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046f958  c1e203                 -shl edx, 3
    cpu.edx <<= 3 /*0x3*/ % 32;
    // 0046f95b  01ca                   -add edx, ecx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.ecx));
    // 0046f95d  8b424a                 -mov eax, dword ptr [edx + 0x4a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(74) /* 0x4a */);
    // 0046f960  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046f963  48                     -dec eax
    (cpu.eax)--;
    // 0046f964  8945a4                 -mov dword ptr [ebp - 0x5c], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-92) /* -0x5c */) = cpu.eax;
    // 0046f967  8b4248                 -mov eax, dword ptr [edx + 0x48]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(72) /* 0x48 */);
    // 0046f96a  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046f96d  8d58ff                 -lea ebx, [eax - 1]
    cpu.ebx = x86::reg32(cpu.eax + x86::reg32(-1) /* -0x1 */);
    // 0046f970  8b4244                 -mov eax, dword ptr [edx + 0x44]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(68) /* 0x44 */);
    // 0046f973  8b5246                 -mov edx, dword ptr [edx + 0x46]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(70) /* 0x46 */);
    // 0046f976  6840e4ffff             -push 0xffffe440
    app->getMemory<x86::reg32>(cpu.esp-4) = 4294960192 /*0xffffe440*/;
    cpu.esp -= 4;
    // 0046f97b  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046f97e  8b4da4                 -mov ecx, dword ptr [ebp - 0x5c]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-92) /* -0x5c */);
    // 0046f981  42                     -inc edx
    (cpu.edx)++;
    // 0046f982  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046f985  8955a4                 -mov dword ptr [ebp - 0x5c], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-92) /* -0x5c */) = cpu.edx;
    // 0046f988  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0046f98a  8b45a4                 -mov eax, dword ptr [ebp - 0x5c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-92) /* -0x5c */);
    // 0046f98d  e89e8f0600             -call 0x4d8930
    cpu.esp -= 4;
    nfs3hp::port::draw2d::frame(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0046f992:
    // 0046f992  837de000               +cmp dword ptr [ebp - 0x20], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046f996  7407                   -je 0x46f99f
    if (cpu.flags.zf)
    {
        goto L_0x0046f99f;
    }
    // 0046f998  b8964a0000             -mov eax, 0x4a96
    cpu.eax = 19094 /*0x4a96*/;
    // 0046f99d  eb12                   -jmp 0x46f9b1
    goto L_0x0046f9b1;
L_0x0046f99f:
    // 0046f99f  837dac00               +cmp dword ptr [ebp - 0x54], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-84) /* -0x54 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046f9a3  7507                   -jne 0x46f9ac
    if (!cpu.flags.zf)
    {
        goto L_0x0046f9ac;
    }
    // 0046f9a5  b82894d5ff             -mov eax, 0xffd59428
    cpu.eax = 4292187176 /*0xffd59428*/;
    // 0046f9aa  eb05                   -jmp 0x46f9b1
    goto L_0x0046f9b1;
L_0x0046f9ac:
    // 0046f9ac  b840e4ff00             -mov eax, 0xffe440
    cpu.eax = 16770112 /*0xffe440*/;
L_0x0046f9b1:
    // 0046f9b1  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046f9b2  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0046f9b5  8b44459a               -mov eax, dword ptr [ebp + eax*2 - 0x66]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-102) /* -0x66 */ + cpu.eax * 2);
    // 0046f9b9  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 0046f9be  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046f9c1  8b5db0                 -mov ebx, dword ptr [ebp - 0x50]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-80) /* -0x50 */);
    // 0046f9c4  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046f9c5  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0046f9c7  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0046f9c9  e8c227feff             -call 0x452190
    cpu.esp -= 4;
    nfs3hp::port::text::drawMenuText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046f9ce  ff45f8                 -inc dword ptr [ebp - 8]
    (app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */))++;
    // 0046f9d1  837df803               +cmp dword ptr [ebp - 8], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046f9d5  7d4f                   -jge 0x46fa26
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0046fa26;
    }
L_0x0046f9d7:
    // 0046f9d7  8d7d94                 -lea edi, [ebp - 0x6c]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-108) /* -0x6c */);
    // 0046f9da  becccc4600             -mov esi, 0x46cccc
    cpu.esi = 4639948 /*0x46cccc*/;
    // 0046f9df  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0046f9e2  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046f9e5  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
    app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
    if (cpu.flags.df)
    {
        cpu.edi -= 4;
        cpu.esi -= 4;
    }
    else
    {
        cpu.edi += 4;
        cpu.esi += 4;
    }
    // 0046f9e6  66a5                   -movsw word ptr es:[edi], word ptr [esi]
    app->getMemory<x86::reg16>(cpu.ees + cpu.edi) = app->getMemory<x86::reg16>(cpu.esi);
    if (cpu.flags.df)
    {
        cpu.edi -= 2;
        cpu.esi -= 2;
    }
    else
    {
        cpu.edi += 2;
        cpu.esi += 2;
    }
    // 0046f9e8  c1e203                 -shl edx, 3
    cpu.edx <<= 3 /*0x3*/ % 32;
    // 0046f9eb  8d7d9c                 -lea edi, [ebp - 0x64]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-100) /* -0x64 */);
    // 0046f9ee  bed2cc4600             -mov esi, 0x46ccd2
    cpu.esi = 4639954 /*0x46ccd2*/;
    // 0046f9f3  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046f9f5  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
    app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
    if (cpu.flags.df)
    {
        cpu.edi -= 4;
        cpu.esi -= 4;
    }
    else
    {
        cpu.edi += 4;
        cpu.esi += 4;
    }
    // 0046f9f6  66a5                   -movsw word ptr es:[edi], word ptr [esi]
    app->getMemory<x86::reg16>(cpu.ees + cpu.edi) = app->getMemory<x86::reg16>(cpu.esi);
    if (cpu.flags.df)
    {
        cpu.edi -= 2;
        cpu.esi -= 2;
    }
    else
    {
        cpu.edi += 2;
        cpu.esi += 2;
    }
    // 0046f9f8  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0046f9fb  8b7246                 -mov esi, dword ptr [edx + 0x46]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(70) /* 0x46 */);
    // 0046f9fe  668b7c459c             -mov di, word ptr [ebp + eax*2 - 0x64]
    cpu.di = app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-100) /* -0x64 */ + cpu.eax * 2);
    // 0046fa03  c1fe10                 -sar esi, 0x10
    cpu.esi = x86::reg32(x86::sreg32(cpu.esi) >> (16 /*0x10*/ % 32));
    // 0046fa06  6683ff05               +cmp di, 5
    {
        x86::reg16 tmp1 = cpu.di;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(5 /*0x5*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046fa0a  0f85b7feffff           -jne 0x46f8c7
    if (!cpu.flags.zf)
    {
        goto L_0x0046f8c7;
    }
    // 0046fa10  8b5248                 -mov edx, dword ptr [edx + 0x48]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(72) /* 0x48 */);
    // 0046fa13  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046fa16  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046fa18  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046fa1b  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0046fa1d  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0046fa1f  01c6                   +add esi, eax
    {
        x86::reg32& tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0046fa21  e9a4feffff             -jmp 0x46f8ca
    goto L_0x0046f8ca;
L_0x0046fa26:
    // 0046fa26  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046fa29  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046fa2c  8b5240                 -mov edx, dword ptr [edx + 0x40]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(64) /* 0x40 */);
    // 0046fa2f  8b4006                 -mov eax, dword ptr [eax + 6]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(6) /* 0x6 */);
    // 0046fa32  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046fa35  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046fa38  8d0c10                 -lea ecx, [eax + edx]
    cpu.ecx = x86::reg32(cpu.eax + cpu.edx * 1);
    // 0046fa3b  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046fa3d  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046fa40  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0046fa42  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0046fa44  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 0046fa46  8d70fb                 -lea esi, [eax - 5]
    cpu.esi = x86::reg32(cpu.eax + x86::reg32(-5) /* -0x5 */);
    // 0046fa49  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046fa4c  8b4078                 -mov eax, dword ptr [eax + 0x78]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(120) /* 0x78 */);
    // 0046fa4f  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0046fa51  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046fa54  894dc4                 -mov dword ptr [ebp - 0x3c], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-60) /* -0x3c */) = cpu.ecx;
    // 0046fa57  8945d8                 -mov dword ptr [ebp - 0x28], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-40) /* -0x28 */) = cpu.eax;
L_0x0046fa5a:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0046fa5a  8b45c4                 -mov eax, dword ptr [ebp - 0x3c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-60) /* -0x3c */);
    // 0046fa5d  3b45b8                 +cmp eax, dword ptr [ebp - 0x48]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-72) /* -0x48 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046fa60  0f8d20030000           -jge 0x46fd86
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0046fd86;
    }
    // 0046fa66  8b45d8                 -mov eax, dword ptr [ebp - 0x28]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-40) /* -0x28 */);
    // 0046fa69  3b45e8                 +cmp eax, dword ptr [ebp - 0x18]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046fa6c  0f8d14030000           -jge 0x46fd86
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0046fd86;
    }
    // 0046fa72  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0046fa74  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0046fa76  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0046fa78  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0046fa7d  e84e62feff             -call 0x455cd0
    cpu.esp -= 4;
    sub_455cd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046fa82  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0046fa84  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046fa86  0f84fa020000           -je 0x46fd86
    if (cpu.flags.zf)
    {
        goto L_0x0046fd86;
    }
    // 0046fa8c  837dbc00               +cmp dword ptr [ebp - 0x44], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-68) /* -0x44 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046fa90  0f841d010000           -je 0x46fbb3
    if (cpu.flags.zf)
    {
        goto L_0x0046fbb3;
    }
    // 0046fa96  837de000               +cmp dword ptr [ebp - 0x20], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046fa9a  7409                   -je 0x46faa5
    if (cpu.flags.zf)
    {
        goto L_0x0046faa5;
    }
    // 0046fa9c  c745d4964a0000         -mov dword ptr [ebp - 0x2c], 0x4a96
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44) /* -0x2c */) = 19094 /*0x4a96*/;
    // 0046faa3  eb07                   -jmp 0x46faac
    goto L_0x0046faac;
L_0x0046faa5:
    // 0046faa5  c745d4fd9d64ff         -mov dword ptr [ebp - 0x2c], 0xff649dfd
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44) /* -0x2c */) = 4284784125 /*0xff649dfd*/;
L_0x0046faac:
    // 0046faac  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046faaf  8b407e                 -mov eax, dword ptr [eax + 0x7e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(126) /* 0x7e */);
    // 0046fab2  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046fab5  8d9594feffff           -lea edx, [ebp - 0x16c]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-364) /* -0x16c */);
    // 0046fabb  8945f0                 -mov dword ptr [ebp - 0x10], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.eax;
    // 0046fabe  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0046fac0  e8ab5afeff             -call 0x455570
    cpu.esp -= 4;
    sub_455570(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046fac5  8b45d4                 -mov eax, dword ptr [ebp - 0x2c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44) /* -0x2c */);
    // 0046fac8  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 0046facd  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046face  8b45f0                 -mov eax, dword ptr [ebp - 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0046fad1  8d5ef6                 -lea ebx, [esi - 0xa]
    cpu.ebx = x86::reg32(cpu.esi + x86::reg32(-10) /* -0xa */);
    // 0046fad4  83c00a                 -add eax, 0xa
    (cpu.eax) += x86::reg32(x86::sreg32(10 /*0xa*/));
    // 0046fad7  6a03                   -push 3
    app->getMemory<x86::reg32>(cpu.esp-4) = 3 /*0x3*/;
    cpu.esp -= 4;
    // 0046fad9  8945a4                 -mov dword ptr [ebp - 0x5c], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-92) /* -0x5c */) = cpu.eax;
    // 0046fadc  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0046fade  8d8594feffff           -lea eax, [ebp - 0x16c]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-364) /* -0x16c */);
    // 0046fae4  e8a726feff             -call 0x452190
    cpu.esp -= 4;
    nfs3hp::port::text::drawMenuText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046fae9  8d9594feffff           -lea edx, [ebp - 0x16c]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-364) /* -0x16c */);
    // 0046faef  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0046faf1  e80a5bfeff             -call 0x455600
    cpu.esp -= 4;
    sub_455600(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046faf6  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046faf9  8b403a                 -mov eax, dword ptr [eax + 0x3a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(58) /* 0x3a */);
    // 0046fafc  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046faff  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046fb00  8b55d4                 -mov edx, dword ptr [ebp - 0x2c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44) /* -0x2c */);
    // 0046fb03  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0046fb04  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 0046fb09  8d5e05                 -lea ebx, [esi + 5]
    cpu.ebx = x86::reg32(cpu.esi + x86::reg32(5) /* 0x5 */);
    // 0046fb0c  6a03                   -push 3
    app->getMemory<x86::reg32>(cpu.esp-4) = 3 /*0x3*/;
    cpu.esp -= 4;
    // 0046fb0e  8d8594feffff           -lea eax, [ebp - 0x16c]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-364) /* -0x16c */);
    // 0046fb14  8b55a4                 -mov edx, dword ptr [ebp - 0x5c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-92) /* -0x5c */);
    // 0046fb17  e80426feff             -call 0x452120
    cpu.esp -= 4;
    sub_452120(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046fb1c  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046fb1f  8b403a                 -mov eax, dword ptr [eax + 0x3a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(58) /* 0x3a */);
    // 0046fb22  8b4df0                 -mov ecx, dword ptr [ebp - 0x10]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0046fb25  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046fb28  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046fb2a  b8ed000000             -mov eax, 0xed
    cpu.eax = 237 /*0xed*/;
    // 0046fb2f  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046fb32  e8191d0600             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046fb37  8b523c                 -mov edx, dword ptr [edx + 0x3c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(60) /* 0x3c */);
    // 0046fb3a  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046fb3d  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0046fb3f  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046fb41  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046fb44  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0046fb46  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0046fb48  8b5dd4                 -mov ebx, dword ptr [ebp - 0x2c]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44) /* -0x2c */);
    // 0046fb4b  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0046fb4c  894df0                 -mov dword ptr [ebp - 0x10], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.ecx;
    // 0046fb4f  6a05                   -push 5
    app->getMemory<x86::reg32>(cpu.esp-4) = 5 /*0x5*/;
    cpu.esp -= 4;
    // 0046fb51  8d1401                 -lea edx, [ecx + eax]
    cpu.edx = x86::reg32(cpu.ecx + cpu.eax * 1);
    // 0046fb54  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 0046fb56  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 0046fb5b  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0046fb5d  e82e26feff             -call 0x452190
    cpu.esp -= 4;
    nfs3hp::port::text::drawMenuText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046fb62  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046fb65  8b403c                 -mov eax, dword ptr [eax + 0x3c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(60) /* 0x3c */);
    // 0046fb68  8b7df0                 -mov edi, dword ptr [ebp - 0x10]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0046fb6b  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046fb6e  01c7                   -add edi, eax
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046fb70  b8eb000000             -mov eax, 0xeb
    cpu.eax = 235 /*0xeb*/;
    // 0046fb75  897df0                 -mov dword ptr [ebp - 0x10], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.edi;
    // 0046fb78  e8d31c0600             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046fb7d  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0046fb7f  837de000               +cmp dword ptr [ebp - 0x20], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046fb83  7407                   -je 0x46fb8c
    if (cpu.flags.zf)
    {
        goto L_0x0046fb8c;
    }
    // 0046fb85  b8964a0000             -mov eax, 0x4a96
    cpu.eax = 19094 /*0x4a96*/;
    // 0046fb8a  eb05                   -jmp 0x46fb91
    goto L_0x0046fb91;
L_0x0046fb8c:
    // 0046fb8c  b800008000             -mov eax, 0x800000
    cpu.eax = 8388608 /*0x800000*/;
L_0x0046fb91:
    // 0046fb91  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046fb94  8b523e                 -mov edx, dword ptr [edx + 0x3e]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(62) /* 0x3e */);
    // 0046fb97  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046fb9a  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046fb9b  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046fb9d  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046fba0  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0046fba2  d1f8                   +sar eax, 1
    {
        x86::reg32 tmp = 1 /*0x1*/ % 32;
        x86::reg32& op = cpu.eax;
        if (tmp)
        {
            cpu.flags.cf = 1 & (x86::sreg32(op) >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = 0;
            cpu.set_szp((op = x86::reg32(x86::sreg32(op) >> tmp)));
        }
    }
    // 0046fba4  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 0046fba9  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 0046fbab  8b55f0                 -mov edx, dword ptr [ebp - 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0046fbae  e9aa010000             -jmp 0x46fd5d
    goto L_0x0046fd5d;
L_0x0046fbb3:
    // 0046fbb3  837de000               +cmp dword ptr [ebp - 0x20], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046fbb7  7409                   -je 0x46fbc2
    if (cpu.flags.zf)
    {
        goto L_0x0046fbc2;
    }
    // 0046fbb9  c745d0964a0000         -mov dword ptr [ebp - 0x30], 0x4a96
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-48) /* -0x30 */) = 19094 /*0x4a96*/;
    // 0046fbc0  eb07                   -jmp 0x46fbc9
    goto L_0x0046fbc9;
L_0x0046fbc2:
    // 0046fbc2  c745d0fd9d64ff         -mov dword ptr [ebp - 0x30], 0xff649dfd
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-48) /* -0x30 */) = 4284784125 /*0xff649dfd*/;
L_0x0046fbc9:
    // 0046fbc9  8b45d0                 -mov eax, dword ptr [ebp - 0x30]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-48) /* -0x30 */);
    // 0046fbcc  8945c0                 -mov dword ptr [ebp - 0x40], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-64) /* -0x40 */) = cpu.eax;
    // 0046fbcf  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046fbd2  8b407e                 -mov eax, dword ptr [eax + 0x7e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(126) /* 0x7e */);
    // 0046fbd5  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046fbd8  8d9594feffff           -lea edx, [ebp - 0x16c]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-364) /* -0x16c */);
    // 0046fbde  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
    // 0046fbe1  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0046fbe3  e88859feff             -call 0x455570
    cpu.esp -= 4;
    sub_455570(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046fbe8  8b45d0                 -mov eax, dword ptr [ebp - 0x30]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-48) /* -0x30 */);
    // 0046fbeb  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 0046fbf0  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046fbf1  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0046fbf4  8d5ef6                 -lea ebx, [esi - 0xa]
    cpu.ebx = x86::reg32(cpu.esi + x86::reg32(-10) /* -0xa */);
    // 0046fbf7  83c00a                 -add eax, 0xa
    (cpu.eax) += x86::reg32(x86::sreg32(10 /*0xa*/));
    // 0046fbfa  6a03                   -push 3
    app->getMemory<x86::reg32>(cpu.esp-4) = 3 /*0x3*/;
    cpu.esp -= 4;
    // 0046fbfc  8945a4                 -mov dword ptr [ebp - 0x5c], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-92) /* -0x5c */) = cpu.eax;
    // 0046fbff  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0046fc01  8d8594feffff           -lea eax, [ebp - 0x16c]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-364) /* -0x16c */);
    // 0046fc07  e88425feff             -call 0x452190
    cpu.esp -= 4;
    nfs3hp::port::text::drawMenuText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046fc0c  8d9594feffff           -lea edx, [ebp - 0x16c]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-364) /* -0x16c */);
    // 0046fc12  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0046fc14  e8e759feff             -call 0x455600
    cpu.esp -= 4;
    sub_455600(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046fc19  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046fc1c  8b403a                 -mov eax, dword ptr [eax + 0x3a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(58) /* 0x3a */);
    // 0046fc1f  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046fc22  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046fc23  8b55d0                 -mov edx, dword ptr [ebp - 0x30]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-48) /* -0x30 */);
    // 0046fc26  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0046fc27  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 0046fc2c  8d5e05                 -lea ebx, [esi + 5]
    cpu.ebx = x86::reg32(cpu.esi + x86::reg32(5) /* 0x5 */);
    // 0046fc2f  6a03                   -push 3
    app->getMemory<x86::reg32>(cpu.esp-4) = 3 /*0x3*/;
    cpu.esp -= 4;
    // 0046fc31  8d8594feffff           -lea eax, [ebp - 0x16c]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-364) /* -0x16c */);
    // 0046fc37  8b55a4                 -mov edx, dword ptr [ebp - 0x5c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-92) /* -0x5c */);
    // 0046fc3a  e8e124feff             -call 0x452120
    cpu.esp -= 4;
    sub_452120(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046fc3f  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046fc42  8b403a                 -mov eax, dword ptr [eax + 0x3a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(58) /* 0x3a */);
    // 0046fc45  8b4df4                 -mov ecx, dword ptr [ebp - 0xc]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0046fc48  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046fc4b  8b1db8d46f00           -mov ebx, dword ptr [0x6fd4b8]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7328952) /* 0x6fd4b8 */);
    // 0046fc51  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046fc53  8b87cc020000           -mov eax, dword ptr [edi + 0x2cc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(716) /* 0x2cc */);
    // 0046fc59  894df4                 -mov dword ptr [ebp - 0xc], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.ecx;
    // 0046fc5c  39d8                   +cmp eax, ebx
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046fc5e  7c2e                   -jl 0x46fc8e
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0046fc8e;
    }
    // 0046fc60  b894010000             -mov eax, 0x194
    cpu.eax = 404 /*0x194*/;
    // 0046fc65  e8e61b0600             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046fc6a  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046fc6b  a1b8d46f00             -mov eax, dword ptr [0x6fd4b8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328952) /* 0x6fd4b8 */);
    // 0046fc70  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046fc71  8b97cc020000           -mov edx, dword ptr [edi + 0x2cc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(716) /* 0x2cc */);
    // 0046fc77  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0046fc78  68ecae5300             -push 0x53aeec
    app->getMemory<x86::reg32>(cpu.esp-4) = 5484268 /*0x53aeec*/;
    cpu.esp -= 4;
    // 0046fc7d  8d8594feffff           -lea eax, [ebp - 0x16c]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-364) /* -0x16c */);
    // 0046fc83  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046fc84  e807fa0600             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046fc89  83c414                 +add esp, 0x14
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(20 /*0x14*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0046fc8c  eb16                   -jmp 0x46fca4
    goto L_0x0046fca4;
L_0x0046fc8e:
    // 0046fc8e  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0046fc8f  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046fc90  68bcae5300             -push 0x53aebc
    app->getMemory<x86::reg32>(cpu.esp-4) = 5484220 /*0x53aebc*/;
    cpu.esp -= 4;
    // 0046fc95  8d8594feffff           -lea eax, [ebp - 0x16c]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-364) /* -0x16c */);
    // 0046fc9b  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046fc9c  e8eff90600             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046fca1  83c410                 -add esp, 0x10
    (cpu.esp) += x86::reg32(x86::sreg32(16 /*0x10*/));
L_0x0046fca4:
    // 0046fca4  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046fca7  8b523c                 -mov edx, dword ptr [edx + 0x3c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(60) /* 0x3c */);
    // 0046fcaa  8b45c0                 -mov eax, dword ptr [ebp - 0x40]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-64) /* -0x40 */);
    // 0046fcad  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046fcb0  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046fcb1  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046fcb3  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046fcb6  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0046fcb8  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0046fcba  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 0046fcbf  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 0046fcc1  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0046fcc4  6a05                   -push 5
    app->getMemory<x86::reg32>(cpu.esp-4) = 5 /*0x5*/;
    cpu.esp -= 4;
    // 0046fcc6  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046fcc8  8d8594feffff           -lea eax, [ebp - 0x16c]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-364) /* -0x16c */);
    // 0046fcce  e8bd24feff             -call 0x452190
    cpu.esp -= 4;
    nfs3hp::port::text::drawMenuText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046fcd3  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046fcd6  8b403c                 -mov eax, dword ptr [eax + 0x3c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(60) /* 0x3c */);
    // 0046fcd9  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0046fcdc  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046fcdf  8b0db8d46f00           -mov ecx, dword ptr [0x6fd4b8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7328952) /* 0x6fd4b8 */);
    // 0046fce5  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046fce7  8b87cc020000           -mov eax, dword ptr [edi + 0x2cc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(716) /* 0x2cc */);
    // 0046fced  8955f4                 -mov dword ptr [ebp - 0xc], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.edx;
    // 0046fcf0  39c8                   +cmp eax, ecx
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ecx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046fcf2  7c3d                   -jl 0x46fd31
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0046fd31;
    }
    // 0046fcf4  8d9594feffff           -lea edx, [ebp - 0x16c]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-364) /* -0x16c */);
    // 0046fcfa  8b87d4020000           -mov eax, dword ptr [edi + 0x2d4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(724) /* 0x2d4 */);
    // 0046fd00  e83bf10500             -call 0x4cee40
    cpu.esp -= 4;
    sub_4cee40(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046fd05  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046fd08  8b523e                 -mov edx, dword ptr [edx + 0x3e]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(62) /* 0x3e */);
    // 0046fd0b  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046fd0e  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046fd10  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046fd13  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0046fd15  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0046fd17  8b5dc0                 -mov ebx, dword ptr [ebp - 0x40]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-64) /* -0x40 */);
    // 0046fd1a  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0046fd1b  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 0046fd20  6a05                   -push 5
    app->getMemory<x86::reg32>(cpu.esp-4) = 5 /*0x5*/;
    cpu.esp -= 4;
    // 0046fd22  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0046fd25  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 0046fd27  01c2                   +add edx, eax
    {
        x86::reg32& tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0046fd29  8d8594feffff           -lea eax, [ebp - 0x16c]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-364) /* -0x16c */);
    // 0046fd2f  eb32                   -jmp 0x46fd63
    goto L_0x0046fd63;
L_0x0046fd31:
    // 0046fd31  b8ed000000             -mov eax, 0xed
    cpu.eax = 237 /*0xed*/;
    // 0046fd36  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046fd39  e8121b0600             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046fd3e  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0046fd40  8b523e                 -mov edx, dword ptr [edx + 0x3e]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(62) /* 0x3e */);
    // 0046fd43  8b45c0                 -mov eax, dword ptr [ebp - 0x40]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-64) /* -0x40 */);
    // 0046fd46  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046fd49  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046fd4a  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046fd4c  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046fd4f  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0046fd51  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0046fd53  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 0046fd58  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 0046fd5a  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
L_0x0046fd5d:
    // 0046fd5d  6a05                   -push 5
    app->getMemory<x86::reg32>(cpu.esp-4) = 5 /*0x5*/;
    cpu.esp -= 4;
    // 0046fd5f  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046fd61  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
L_0x0046fd63:
    // 0046fd63  e82824feff             -call 0x452190
    cpu.esp -= 4;
    nfs3hp::port::text::drawMenuText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046fd68  8b55d8                 -mov edx, dword ptr [ebp - 0x28]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-40) /* -0x28 */);
    // 0046fd6b  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046fd6e  8b4dc4                 -mov ecx, dword ptr [ebp - 0x3c]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-60) /* -0x3c */);
    // 0046fd71  42                     -inc edx
    (cpu.edx)++;
    // 0046fd72  8b4040                 -mov eax, dword ptr [eax + 0x40]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(64) /* 0x40 */);
    // 0046fd75  41                     -inc ecx
    (cpu.ecx)++;
    // 0046fd76  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046fd79  8955d8                 -mov dword ptr [ebp - 0x28], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-40) /* -0x28 */) = cpu.edx;
    // 0046fd7c  894dc4                 -mov dword ptr [ebp - 0x3c], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-60) /* -0x3c */) = cpu.ecx;
    // 0046fd7f  01c6                   +add esi, eax
    {
        x86::reg32& tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0046fd81  e9d4fcffff             -jmp 0x46fa5a
    goto L_0x0046fa5a;
L_0x0046fd86:
    // 0046fd86  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046fd89  8b4042                 -mov eax, dword ptr [eax + 0x42]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(66) /* 0x42 */);
    // 0046fd8c  8b5de8                 -mov ebx, dword ptr [ebp - 0x18]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 0046fd8f  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046fd92  39d8                   +cmp eax, ebx
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046fd94  0f8d82000000           -jge 0x46fe1c
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0046fe1c;
    }
    // 0046fd9a  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046fd9d  6683787a00             +cmp word ptr [eax + 0x7a], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(122) /* 0x7a */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046fda2  742d                   -je 0x46fdd1
    if (cpu.flags.zf)
    {
        goto L_0x0046fdd1;
    }
    // 0046fda4  83c05e                 -add eax, 0x5e
    (cpu.eax) += x86::reg32(x86::sreg32(94 /*0x5e*/));
    // 0046fda7  8b58fe                 -mov ebx, dword ptr [eax - 2]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-2) /* -0x2 */);
    // 0046fdaa  8b10                   -mov edx, dword ptr [eax]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax);
    // 0046fdac  e83fc40200             -call 0x49c1f0
    cpu.esp -= 4;
    nfs3hp::frontend::pointerInBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046fdb1  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046fdb4  c1fb10                 -sar ebx, 0x10
    cpu.ebx = x86::reg32(x86::sreg32(cpu.ebx) >> (16 /*0x10*/ % 32));
    // 0046fdb7  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046fdb9  7409                   -je 0x46fdc4
    if (cpu.flags.zf)
    {
        goto L_0x0046fdc4;
    }
    // 0046fdbb  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046fdbe  668b4072               -mov ax, word ptr [eax + 0x72]
    cpu.ax = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(114) /* 0x72 */);
    // 0046fdc2  eb07                   -jmp 0x46fdcb
    goto L_0x0046fdcb;
L_0x0046fdc4:
    // 0046fdc4  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046fdc7  668b406e               -mov ax, word ptr [eax + 0x6e]
    cpu.ax = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(110) /* 0x6e */);
L_0x0046fdcb:
    // 0046fdcb  98                     -cwde 
    cpu.eax = x86::reg32(static_cast<x86::sreg16>(cpu.ax));
    // 0046fdcc  e8bf7b0600             -call 0x4d7990
    cpu.esp -= 4;
    nfs3hp::port::shapes::drawShape(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0046fdd1:
    // 0046fdd1  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046fdd4  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046fdd7  8b5242                 -mov edx, dword ptr [edx + 0x42]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(66) /* 0x42 */);
    // 0046fdda  8b4078                 -mov eax, dword ptr [eax + 0x78]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(120) /* 0x78 */);
    // 0046fddd  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046fde0  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046fde3  8b75e8                 -mov esi, dword ptr [ebp - 0x18]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 0046fde6  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 0046fde8  39f0                   +cmp eax, esi
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.esi));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046fdea  7d30                   -jge 0x46fe1c
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0046fe1c;
    }
    // 0046fdec  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046fdef  83c066                 -add eax, 0x66
    (cpu.eax) += x86::reg32(x86::sreg32(102 /*0x66*/));
    // 0046fdf2  8b58fe                 -mov ebx, dword ptr [eax - 2]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-2) /* -0x2 */);
    // 0046fdf5  8b10                   -mov edx, dword ptr [eax]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax);
    // 0046fdf7  e8f4c30200             -call 0x49c1f0
    cpu.esp -= 4;
    nfs3hp::frontend::pointerInBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046fdfc  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046fdff  c1fb10                 -sar ebx, 0x10
    cpu.ebx = x86::reg32(x86::sreg32(cpu.ebx) >> (16 /*0x10*/ % 32));
    // 0046fe02  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046fe04  7409                   -je 0x46fe0f
    if (cpu.flags.zf)
    {
        goto L_0x0046fe0f;
    }
    // 0046fe06  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046fe09  668b4074               -mov ax, word ptr [eax + 0x74]
    cpu.ax = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(116) /* 0x74 */);
    // 0046fe0d  eb07                   -jmp 0x46fe16
    goto L_0x0046fe16;
L_0x0046fe0f:
    // 0046fe0f  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046fe12  668b4070               -mov ax, word ptr [eax + 0x70]
    cpu.ax = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(112) /* 0x70 */);
L_0x0046fe16:
    // 0046fe16  98                     -cwde 
    cpu.eax = x86::reg32(static_cast<x86::sreg16>(cpu.ax));
    // 0046fe17  e8747b0600             -call 0x4d7990
    cpu.esp -= 4;
    nfs3hp::port::shapes::drawShape(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0046fe1c:
    // 0046fe1c  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0046fe1e  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046fe20  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0046fe22  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046fe23  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046fe24  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046fe25  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046fe26  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046fe27  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_46fe30(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0046fe30  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0046fe31  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0046fe32  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0046fe33  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0046fe34  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0046fe36  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0046fe38  ba1a000000             -mov edx, 0x1a
    cpu.edx = 26 /*0x1a*/;
    // 0046fe3d  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0046fe3f  e81ca8fdff             -call 0x44a660
    cpu.esp -= 4;
    nfs3hp::frontend::menuShownNow(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046fe44  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046fe46  0f8481000000           -je 0x46fecd
    if (cpu.flags.zf)
    {
        goto L_0x0046fecd;
    }
    // 0046fe4c  b8c4ae5300             -mov eax, 0x53aec4
    cpu.eax = 5484228 /*0x53aec4*/;
    // 0046fe51  e85a5f0600             -call 0x4d5db0
    cpu.esp -= 4;
    nfs3hp::port::shapes::shapeByName(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046fe56  6689416e               -mov word ptr [ecx + 0x6e], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(110) /* 0x6e */) = cpu.ax;
    // 0046fe5a  b8ccae5300             -mov eax, 0x53aecc
    cpu.eax = 5484236 /*0x53aecc*/;
    // 0046fe5f  e84c5f0600             -call 0x4d5db0
    cpu.esp -= 4;
    nfs3hp::port::shapes::shapeByName(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046fe64  66894170               -mov word ptr [ecx + 0x70], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(112) /* 0x70 */) = cpu.ax;
    // 0046fe68  b8d4ae5300             -mov eax, 0x53aed4
    cpu.eax = 5484244 /*0x53aed4*/;
    // 0046fe6d  e83e5f0600             -call 0x4d5db0
    cpu.esp -= 4;
    nfs3hp::port::shapes::shapeByName(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046fe72  66894172               -mov word ptr [ecx + 0x72], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(114) /* 0x72 */) = cpu.ax;
    // 0046fe76  b8dcae5300             -mov eax, 0x53aedc
    cpu.eax = 5484252 /*0x53aedc*/;
    // 0046fe7b  e8305f0600             -call 0x4d5db0
    cpu.esp -= 4;
    nfs3hp::port::shapes::shapeByName(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046fe80  66894174               -mov word ptr [ecx + 0x74], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(116) /* 0x74 */) = cpu.ax;
    // 0046fe84  b8ccae5300             -mov eax, 0x53aecc
    cpu.eax = 5484236 /*0x53aecc*/;
    // 0046fe89  e8225f0600             -call 0x4d5db0
    cpu.esp -= 4;
    nfs3hp::port::shapes::shapeByName(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046fe8e  66894176               -mov word ptr [ecx + 0x76], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(118) /* 0x76 */) = cpu.ax;
    // 0046fe92  b8c4ae5300             -mov eax, 0x53aec4
    cpu.eax = 5484228 /*0x53aec4*/;
    // 0046fe97  e8145f0600             -call 0x4d5db0
    cpu.esp -= 4;
    nfs3hp::port::shapes::shapeByName(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046fe9c  66c7417c0200           -mov word ptr [ecx + 0x7c], 2
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(124) /* 0x7c */) = 2 /*0x2*/;
    // 0046fea2  668b9180000000         -mov dx, word ptr [ecx + 0x80]
    cpu.dx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(128) /* 0x80 */);
    // 0046fea9  66894178               -mov word ptr [ecx + 0x78], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(120) /* 0x78 */) = cpu.ax;
    // 0046fead  6685d2                 +test dx, dx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg16>(cpu.dx & cpu.dx));
    // 0046feb0  750f                   -jne 0x46fec1
    if (!cpu.flags.zf)
    {
        goto L_0x0046fec1;
    }
    // 0046feb2  668b4106               -mov ax, word ptr [ecx + 6]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(6) /* 0x6 */);
    // 0046feb6  6689517e               -mov word ptr [ecx + 0x7e], dx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(126) /* 0x7e */) = cpu.dx;
    // 0046feba  66898180000000         -mov word ptr [ecx + 0x80], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(128) /* 0x80 */) = cpu.ax;
L_0x0046fec1:
    // 0046fec1  66c7437a0000           -mov word ptr [ebx + 0x7a], 0
    app->getMemory<x86::reg16>(cpu.ebx + x86::reg32(122) /* 0x7a */) = 0 /*0x0*/;
    // 0046fec7  66c7437c0200           -mov word ptr [ebx + 0x7c], 2
    app->getMemory<x86::reg16>(cpu.ebx + x86::reg32(124) /* 0x7c */) = 2 /*0x2*/;
L_0x0046fecd:
    // 0046fecd  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046fece  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046fecf  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046fed0  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046fed1  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_46fee0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0046fee0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0046fee1  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0046fee2  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0046fee4  ba1a000000             -mov edx, 0x1a
    cpu.edx = 26 /*0x1a*/;
    // 0046fee9  e872a7fdff             -call 0x44a660
    cpu.esp -= 4;
    nfs3hp::frontend::menuShownNow(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046feee  85c0                   -test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046fef0  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046fef1  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046fef2  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_46ff00(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0046ff00  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0046ff01  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0046ff02  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0046ff03  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0046ff04  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0046ff05  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0046ff06  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0046ff08  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0046ff0a  ba1a000000             -mov edx, 0x1a
    cpu.edx = 26 /*0x1a*/;
    // 0046ff0f  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0046ff11  e84aa7fdff             -call 0x44a660
    cpu.esp -= 4;
    nfs3hp::frontend::menuShownNow(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046ff16  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046ff18  0f841b010000           -je 0x470039
    if (cpu.flags.zf)
    {
        goto L_0x00470039;
    }
    // 0046ff1e  8b5342                 -mov edx, dword ptr [ebx + 0x42]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(66) /* 0x42 */);
    // 0046ff21  a1347d6700             -mov eax, dword ptr [0x677d34]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6782260) /* 0x677d34 */);
    // 0046ff26  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046ff29  39c2                   +cmp edx, eax
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046ff2b  7c02                   -jl 0x46ff2f
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0046ff2f;
    }
    // 0046ff2d  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
L_0x0046ff2f:
    // 0046ff2f  8b593c                 -mov ebx, dword ptr [ecx + 0x3c]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(60) /* 0x3c */);
    // 0046ff32  8b413a                 -mov eax, dword ptr [ecx + 0x3a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(58) /* 0x3a */);
    // 0046ff35  c1fb10                 -sar ebx, 0x10
    cpu.ebx = x86::reg32(x86::sreg32(cpu.ebx) >> (16 /*0x10*/ % 32));
    // 0046ff38  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046ff3b  01c3                   -add ebx, eax
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046ff3d  8b413e                 -mov eax, dword ptr [ecx + 0x3e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(62) /* 0x3e */);
    // 0046ff40  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046ff43  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 0046ff45  8b5940                 -mov ebx, dword ptr [ecx + 0x40]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(64) /* 0x40 */);
    // 0046ff48  42                     -inc edx
    (cpu.edx)++;
    // 0046ff49  c1fb10                 -sar ebx, 0x10
    cpu.ebx = x86::reg32(x86::sreg32(cpu.ebx) >> (16 /*0x10*/ % 32));
    // 0046ff4c  0fafd3                 -imul edx, ebx
    cpu.edx = x86::reg32(x86::sreg64(x86::sreg32(cpu.edx)) * x86::sreg64(x86::sreg32(cpu.ebx)));
    // 0046ff4f  668b9980000000         -mov bx, word ptr [ecx + 0x80]
    cpu.bx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(128) /* 0x80 */);
    // 0046ff56  66895948               -mov word ptr [ecx + 0x48], bx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(72) /* 0x48 */) = cpu.bx;
    // 0046ff5a  668b5908               -mov bx, word ptr [ecx + 8]
    cpu.bx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(8) /* 0x8 */);
    // 0046ff5e  66895946               -mov word ptr [ecx + 0x46], bx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(70) /* 0x46 */) = cpu.bx;
    // 0046ff62  668b593c               -mov bx, word ptr [ecx + 0x3c]
    cpu.bx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(60) /* 0x3c */);
    // 0046ff66  6689594a               -mov word ptr [ecx + 0x4a], bx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(74) /* 0x4a */) = cpu.bx;
    // 0046ff6a  668b5942               -mov bx, word ptr [ecx + 0x42]
    cpu.bx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(66) /* 0x42 */);
    // 0046ff6e  6689594c               -mov word ptr [ecx + 0x4c], bx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(76) /* 0x4c */) = cpu.bx;
    // 0046ff72  668b713c               -mov si, word ptr [ecx + 0x3c]
    cpu.si = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(60) /* 0x3c */);
    // 0046ff76  668b9980000000         -mov bx, word ptr [ecx + 0x80]
    cpu.bx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(128) /* 0x80 */);
    // 0046ff7d  01f3                   -add ebx, esi
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.esi));
    // 0046ff7f  66895950               -mov word ptr [ecx + 0x50], bx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(80) /* 0x50 */) = cpu.bx;
    // 0046ff83  668b5908               -mov bx, word ptr [ecx + 8]
    cpu.bx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(8) /* 0x8 */);
    // 0046ff87  6689594e               -mov word ptr [ecx + 0x4e], bx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(78) /* 0x4e */) = cpu.bx;
    // 0046ff8b  668b593e               -mov bx, word ptr [ecx + 0x3e]
    cpu.bx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(62) /* 0x3e */);
    // 0046ff8f  66895952               -mov word ptr [ecx + 0x52], bx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(82) /* 0x52 */) = cpu.bx;
    // 0046ff93  668b5942               -mov bx, word ptr [ecx + 0x42]
    cpu.bx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(66) /* 0x42 */);
    // 0046ff97  66895954               -mov word ptr [ecx + 0x54], bx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(84) /* 0x54 */) = cpu.bx;
    // 0046ff9b  668b793c               -mov di, word ptr [ecx + 0x3c]
    cpu.di = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(60) /* 0x3c */);
    // 0046ff9f  668b9980000000         -mov bx, word ptr [ecx + 0x80]
    cpu.bx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(128) /* 0x80 */);
    // 0046ffa6  668b713e               -mov si, word ptr [ecx + 0x3e]
    cpu.si = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(62) /* 0x3e */);
    // 0046ffaa  01fb                   -add ebx, edi
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.edi));
    // 0046ffac  01f3                   -add ebx, esi
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.esi));
    // 0046ffae  66895958               -mov word ptr [ecx + 0x58], bx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(88) /* 0x58 */) = cpu.bx;
    // 0046ffb2  668b5908               -mov bx, word ptr [ecx + 8]
    cpu.bx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(8) /* 0x8 */);
    // 0046ffb6  66895956               -mov word ptr [ecx + 0x56], bx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(86) /* 0x56 */) = cpu.bx;
    // 0046ffba  668b5940               -mov bx, word ptr [ecx + 0x40]
    cpu.bx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(64) /* 0x40 */);
    // 0046ffbe  6689595a               -mov word ptr [ecx + 0x5a], bx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(90) /* 0x5a */) = cpu.bx;
    // 0046ffc2  668b5942               -mov bx, word ptr [ecx + 0x42]
    cpu.bx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(66) /* 0x42 */);
    // 0046ffc6  6689595c               -mov word ptr [ecx + 0x5c], bx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(92) /* 0x5c */) = cpu.bx;
    // 0046ffca  668b9980000000         -mov bx, word ptr [ecx + 0x80]
    cpu.bx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(128) /* 0x80 */);
    // 0046ffd1  6689591c               -mov word ptr [ecx + 0x1c], bx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(28) /* 0x1c */) = cpu.bx;
    // 0046ffd5  668b5908               -mov bx, word ptr [ecx + 8]
    cpu.bx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(8) /* 0x8 */);
    // 0046ffd9  6689591a               -mov word ptr [ecx + 0x1a], bx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(26) /* 0x1a */) = cpu.bx;
    // 0046ffdd  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0046ffdf  83c320                 -add ebx, 0x20
    (cpu.ebx) += x86::reg32(x86::sreg32(32 /*0x20*/));
    // 0046ffe2  6689591e               -mov word ptr [ecx + 0x1e], bx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(30) /* 0x1e */) = cpu.bx;
    // 0046ffe6  668b9980000000         -mov bx, word ptr [ecx + 0x80]
    cpu.bx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(128) /* 0x80 */);
    // 0046ffed  01c3                   -add ebx, eax
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046ffef  66c741620c00           -mov word ptr [ecx + 0x62], 0xc
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(98) /* 0x62 */) = 12 /*0xc*/;
    // 0046fff5  83c314                 -add ebx, 0x14
    (cpu.ebx) += x86::reg32(x86::sreg32(20 /*0x14*/));
    // 0046fff8  66895960               -mov word ptr [ecx + 0x60], bx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(96) /* 0x60 */) = cpu.bx;
    // 0046fffc  668b5908               -mov bx, word ptr [ecx + 8]
    cpu.bx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(8) /* 0x8 */);
    // 00470000  6689595e               -mov word ptr [ecx + 0x5e], bx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(94) /* 0x5e */) = cpu.bx;
    // 00470004  668b9980000000         -mov bx, word ptr [ecx + 0x80]
    cpu.bx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(128) /* 0x80 */);
    // 0047000b  66c741640c00           -mov word ptr [ecx + 0x64], 0xc
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(100) /* 0x64 */) = 12 /*0xc*/;
    // 00470011  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 00470013  66c7416a0c00           -mov word ptr [ecx + 0x6a], 0xc
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(106) /* 0x6a */) = 12 /*0xc*/;
    // 00470019  0514000000             -add eax, 0x14
    (cpu.eax) += x86::reg32(x86::sreg32(20 /*0x14*/));
    // 0047001e  66894168               -mov word ptr [ecx + 0x68], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(104) /* 0x68 */) = cpu.ax;
    // 00470022  668b4108               -mov ax, word ptr [ecx + 8]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(8) /* 0x8 */);
    // 00470026  66c7416c0c00           -mov word ptr [ecx + 0x6c], 0xc
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(108) /* 0x6c */) = 12 /*0xc*/;
    // 0047002c  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 0047002e  66895120               -mov word ptr [ecx + 0x20], dx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(32) /* 0x20 */) = cpu.dx;
    // 00470032  83e80c                 -sub eax, 0xc
    (cpu.eax) -= x86::reg32(x86::sreg32(12 /*0xc*/));
    // 00470035  66894166               -mov word ptr [ecx + 0x66], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(102) /* 0x66 */) = cpu.ax;
L_0x00470039:
    // 00470039  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047003a  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047003b  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047003c  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047003d  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047003e  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047003f  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_470040(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00470040  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00470041  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00470042  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00470043  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00470044  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00470046  83ec0c                 -sub esp, 0xc
    (cpu.esp) -= x86::reg32(x86::sreg32(12 /*0xc*/));
    // 00470049  89df                   -mov edi, ebx
    cpu.edi = cpu.ebx;
    // 0047004b  bbffffffff             -mov ebx, 0xffffffff
    cpu.ebx = 4294967295 /*0xffffffff*/;
    // 00470050  ba1a000000             -mov edx, 0x1a
    cpu.edx = 26 /*0x1a*/;
    // 00470055  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00470057  e804a6fdff             -call 0x44a660
    cpu.esp -= 4;
    nfs3hp::frontend::menuShownNow(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047005c  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 0047005e  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00470060  7507                   -jne 0x470069
    if (!cpu.flags.zf)
    {
        goto L_0x00470069;
    }
    // 00470062  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 00470064  e9c6020000             -jmp 0x47032f
    goto L_0x0047032f;
L_0x00470069:
    // 00470069  e812ef0500             -call 0x4cef80
    cpu.esp -= 4;
    sub_4cef80(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047006e  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 00470071  a1347d6700             -mov eax, dword ptr [0x677d34]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6782260) /* 0x677d34 */);
    // 00470076  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 00470078  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0047007b  eb06                   -jmp 0x470083
    goto L_0x00470083;
L_0x0047007d:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0047007d  42                     -inc edx
    (cpu.edx)++;
    // 0047007e  83fa03                 +cmp edx, 3
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00470081  7d1b                   -jge 0x47009e
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0047009e;
    }
L_0x00470083:
    // 00470083  8d04d500000000         -lea eax, [edx*8]
    cpu.eax = x86::reg32(cpu.edx * 8);
    // 0047008a  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
    // 0047008d  8d4146                 -lea eax, [ecx + 0x46]
    cpu.eax = x86::reg32(cpu.ecx + x86::reg32(70) /* 0x46 */);
    // 00470090  0345f4                 -add eax, dword ptr [ebp - 0xc]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */)));
    // 00470093  e858c10200             -call 0x49c1f0
    cpu.esp -= 4;
    nfs3hp::frontend::pointerInBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00470098  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0047009a  74e1                   -je 0x47007d
    if (cpu.flags.zf)
    {
        goto L_0x0047007d;
    }
    // 0047009c  89d3                   -mov ebx, edx
    cpu.ebx = cpu.edx;
L_0x0047009e:
    // 0047009e  6683ff0d               +cmp di, 0xd
    {
        x86::reg16 tmp1 = cpu.di;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(13 /*0xd*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004700a2  0f856b000000           -jne 0x470113
    if (!cpu.flags.zf)
    {
        goto L_0x00470113;
    }
    // 004700a8  837df800               +cmp dword ptr [ebp - 8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004700ac  0f8561000000           -jne 0x470113
    if (!cpu.flags.zf)
    {
        goto L_0x00470113;
    }
    // 004700b2  83fb01                 +cmp ebx, 1
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004700b5  7209                   -jb 0x4700c0
    if (cpu.flags.cf)
    {
        goto L_0x004700c0;
    }
    // 004700b7  7627                   -jbe 0x4700e0
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004700e0;
    }
    // 004700b9  83fb02                 +cmp ebx, 2
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004700bc  743e                   -je 0x4700fc
    if (cpu.flags.zf)
    {
        goto L_0x004700fc;
    }
    // 004700be  eb53                   -jmp 0x470113
    goto L_0x00470113;
L_0x004700c0:
    // 004700c0  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 004700c2  754f                   -jne 0x470113
    if (!cpu.flags.zf)
    {
        goto L_0x00470113;
    }
    // 004700c4  ba7f000000             -mov edx, 0x7f
    cpu.edx = 127 /*0x7f*/;
    // 004700c9  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004700ce  be02000000             -mov esi, 2
    cpu.esi = 2 /*0x2*/;
    // 004700d3  66c7417c0000           -mov word ptr [ecx + 0x7c], 0
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(124) /* 0x7c */) = 0 /*0x0*/;
    // 004700d9  e8f280faff             -call 0x4181d0
    cpu.esp -= 4;
    nfs3hp::port::playMenuEffect(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004700de  eb33                   -jmp 0x470113
    goto L_0x00470113;
L_0x004700e0:
    // 004700e0  ba7f000000             -mov edx, 0x7f
    cpu.edx = 127 /*0x7f*/;
    // 004700e5  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004700ea  be02000000             -mov esi, 2
    cpu.esi = 2 /*0x2*/;
    // 004700ef  66c7417c0100           -mov word ptr [ecx + 0x7c], 1
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(124) /* 0x7c */) = 1 /*0x1*/;
    // 004700f5  e8d680faff             -call 0x4181d0
    cpu.esp -= 4;
    nfs3hp::port::playMenuEffect(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004700fa  eb17                   -jmp 0x470113
    goto L_0x00470113;
L_0x004700fc:
    // 004700fc  ba7f000000             -mov edx, 0x7f
    cpu.edx = 127 /*0x7f*/;
    // 00470101  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00470106  66c7417c0200           -mov word ptr [ecx + 0x7c], 2
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(124) /* 0x7c */) = 2 /*0x2*/;
    // 0047010c  e8bf80faff             -call 0x4181d0
    cpu.esp -= 4;
    nfs3hp::port::playMenuEffect(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00470111  89de                   -mov esi, ebx
    cpu.esi = cpu.ebx;
L_0x00470113:
    // 00470113  837df800               +cmp dword ptr [ebp - 8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00470117  0f8579000000           -jne 0x470196
    if (!cpu.flags.zf)
    {
        goto L_0x00470196;
    }
    // 0047011d  6681ff004d             +cmp di, 0x4d00
    {
        x86::reg16 tmp1 = cpu.di;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(19712 /*0x4d00*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 00470122  7407                   -je 0x47012b
    if (cpu.flags.zf)
    {
        goto L_0x0047012b;
    }
    // 00470124  6681ff004b             +cmp di, 0x4b00
    {
        x86::reg16 tmp1 = cpu.di;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(19200 /*0x4b00*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 00470129  756b                   -jne 0x470196
    if (!cpu.flags.zf)
    {
        goto L_0x00470196;
    }
L_0x0047012b:
    // 0047012b  43                     -inc ebx
    (cpu.ebx)++;
    // 0047012c  be03000000             -mov esi, 3
    cpu.esi = 3 /*0x3*/;
    // 00470131  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 00470133  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 00470135  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00470138  f7fe                   -idiv esi
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.esi);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 0047013a  89d3                   -mov ebx, edx
    cpu.ebx = cpu.edx;
    // 0047013c  8b44d948               -mov eax, dword ptr [ecx + ebx*8 + 0x48]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(72) /* 0x48 */ + cpu.ebx * 8);
    // 00470140  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00470143  8d148500000000         -lea edx, [eax*4]
    cpu.edx = x86::reg32(cpu.eax * 4);
    // 0047014a  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 0047014c  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0047014e  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00470151  c1e202                 +shl edx, 2
    {
        x86::reg8 tmp = 2 /*0x2*/ % 32;
        x86::reg32& op = cpu.edx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 00470154  1bc2                   -sbb eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 00470156  c1f802                 -sar eax, 2
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (2 /*0x2*/ % 32));
    // 00470159  8b54d94e               -mov edx, dword ptr [ecx + ebx*8 + 0x4e]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(78) /* 0x4e */ + cpu.ebx * 8);
    // 0047015d  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 00470160  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00470162  8915e4e55500           -mov dword ptr [0x55e5e4], edx
    app->getMemory<x86::reg32>(x86::reg32(5629412) /* 0x55e5e4 */) = cpu.edx;
    // 00470168  8b54d94a               -mov edx, dword ptr [ecx + ebx*8 + 0x4a]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(74) /* 0x4a */ + cpu.ebx * 8);
    // 0047016c  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0047016f  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00470171  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00470174  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00470176  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 00470178  8b54d94c               -mov edx, dword ptr [ecx + ebx*8 + 0x4c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(76) /* 0x4c */ + cpu.ebx * 8);
    // 0047017c  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0047017f  be02000000             -mov esi, 2
    cpu.esi = 2 /*0x2*/;
    // 00470184  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00470186  a1e4e55500             -mov eax, dword ptr [0x55e5e4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629412) /* 0x55e5e4 */);
    // 0047018b  8915e8e55500           -mov dword ptr [0x55e5e8], edx
    app->getMemory<x86::reg32>(x86::reg32(5629416) /* 0x55e5e8 */) = cpu.edx;
    // 00470191  e82abe0200             -call 0x49bfc0
    cpu.esp -= 4;
    nfs3hp::frontend::setPointer(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00470196:
    // 00470196  8b4142                 -mov eax, dword ptr [ecx + 0x42]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(66) /* 0x42 */);
    // 00470199  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0047019c  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0047019f  39d0                   +cmp eax, edx
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004701a1  0f8d86010000           -jge 0x47032d
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0047032d;
    }
    // 004701a7  6681ff0049             +cmp di, 0x4900
    {
        x86::reg16 tmp1 = cpu.di;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(18688 /*0x4900*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004701ac  0f8575000000           -jne 0x470227
    if (!cpu.flags.zf)
    {
        goto L_0x00470227;
    }
    // 004701b2  6683797a00             +cmp word ptr [ecx + 0x7a], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(122) /* 0x7a */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004701b7  746e                   -je 0x470227
    if (cpu.flags.zf)
    {
        goto L_0x00470227;
    }
    // 004701b9  ba7f000000             -mov edx, 0x7f
    cpu.edx = 127 /*0x7f*/;
    // 004701be  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004701c3  e80880faff             -call 0x4181d0
    cpu.esp -= 4;
    nfs3hp::port::playMenuEffect(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004701c8  8b4160                 -mov eax, dword ptr [ecx + 0x60]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(96) /* 0x60 */);
    // 004701cb  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 004701ce  8d148500000000         -lea edx, [eax*4]
    cpu.edx = x86::reg32(cpu.eax * 4);
    // 004701d5  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 004701d7  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004701d9  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004701dc  c1e202                 +shl edx, 2
    {
        x86::reg8 tmp = 2 /*0x2*/ % 32;
        x86::reg32& op = cpu.edx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 004701df  1bc2                   -sbb eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 004701e1  c1f802                 -sar eax, 2
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (2 /*0x2*/ % 32));
    // 004701e4  8b515e                 -mov edx, dword ptr [ecx + 0x5e]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(94) /* 0x5e */);
    // 004701e7  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 004701ea  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004701ec  8915e4e55500           -mov dword ptr [0x55e5e4], edx
    app->getMemory<x86::reg32>(x86::reg32(5629412) /* 0x55e5e4 */) = cpu.edx;
    // 004701f2  8b5162                 -mov edx, dword ptr [ecx + 0x62]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(98) /* 0x62 */);
    // 004701f5  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 004701f8  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004701fa  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004701fd  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004701ff  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 00470201  be02000000             -mov esi, 2
    cpu.esi = 2 /*0x2*/;
    // 00470206  668b597a               -mov bx, word ptr [ecx + 0x7a]
    cpu.bx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(122) /* 0x7a */);
    // 0047020a  8b515c                 -mov edx, dword ptr [ecx + 0x5c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(92) /* 0x5c */);
    // 0047020d  4b                     -dec ebx
    (cpu.ebx)--;
    // 0047020e  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 00470211  6689597a               -mov word ptr [ecx + 0x7a], bx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(122) /* 0x7a */) = cpu.bx;
    // 00470215  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00470217  a1e4e55500             -mov eax, dword ptr [0x55e5e4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629412) /* 0x55e5e4 */);
    // 0047021c  8915e8e55500           -mov dword ptr [0x55e5e8], edx
    app->getMemory<x86::reg32>(x86::reg32(5629416) /* 0x55e5e8 */) = cpu.edx;
    // 00470222  e899bd0200             -call 0x49bfc0
    cpu.esp -= 4;
    nfs3hp::frontend::setPointer(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00470227:
    // 00470227  6681ff0051             +cmp di, 0x5100
    {
        x86::reg16 tmp1 = cpu.di;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(20736 /*0x5100*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0047022c  0f8583000000           -jne 0x4702b5
    if (!cpu.flags.zf)
    {
        goto L_0x004702b5;
    }
    // 00470232  8b4142                 -mov eax, dword ptr [ecx + 0x42]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(66) /* 0x42 */);
    // 00470235  8b5178                 -mov edx, dword ptr [ecx + 0x78]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(120) /* 0x78 */);
    // 00470238  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0047023b  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0047023e  8b5dfc                 -mov ebx, dword ptr [ebp - 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00470241  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00470243  39d8                   +cmp eax, ebx
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00470245  7d6e                   -jge 0x4702b5
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004702b5;
    }
    // 00470247  ba7f000000             -mov edx, 0x7f
    cpu.edx = 127 /*0x7f*/;
    // 0047024c  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00470251  e87a7ffaff             -call 0x4181d0
    cpu.esp -= 4;
    nfs3hp::port::playMenuEffect(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00470256  8b4168                 -mov eax, dword ptr [ecx + 0x68]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(104) /* 0x68 */);
    // 00470259  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0047025c  8d148500000000         -lea edx, [eax*4]
    cpu.edx = x86::reg32(cpu.eax * 4);
    // 00470263  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 00470265  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00470267  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0047026a  c1e202                 +shl edx, 2
    {
        x86::reg8 tmp = 2 /*0x2*/ % 32;
        x86::reg32& op = cpu.edx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 0047026d  1bc2                   -sbb eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 0047026f  c1f802                 -sar eax, 2
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (2 /*0x2*/ % 32));
    // 00470272  8b5166                 -mov edx, dword ptr [ecx + 0x66]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(102) /* 0x66 */);
    // 00470275  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 00470278  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0047027a  8915e4e55500           -mov dword ptr [0x55e5e4], edx
    app->getMemory<x86::reg32>(x86::reg32(5629412) /* 0x55e5e4 */) = cpu.edx;
    // 00470280  8b516a                 -mov edx, dword ptr [ecx + 0x6a]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(106) /* 0x6a */);
    // 00470283  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 00470286  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00470288  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0047028b  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0047028d  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0047028f  668b717a               -mov si, word ptr [ecx + 0x7a]
    cpu.si = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(122) /* 0x7a */);
    // 00470293  46                     -inc esi
    (cpu.esi)++;
    // 00470294  8b5164                 -mov edx, dword ptr [ecx + 0x64]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(100) /* 0x64 */);
    // 00470297  6689717a               -mov word ptr [ecx + 0x7a], si
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(122) /* 0x7a */) = cpu.si;
    // 0047029b  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0047029e  be02000000             -mov esi, 2
    cpu.esi = 2 /*0x2*/;
    // 004702a3  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004702a5  a1e4e55500             -mov eax, dword ptr [0x55e5e4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629412) /* 0x55e5e4 */);
    // 004702aa  8915e8e55500           -mov dword ptr [0x55e5e8], edx
    app->getMemory<x86::reg32>(x86::reg32(5629416) /* 0x55e5e8 */) = cpu.edx;
    // 004702b0  e80bbd0200             -call 0x49bfc0
    cpu.esp -= 4;
    nfs3hp::frontend::setPointer(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004702b5:
    // 004702b5  6683ff0d               +cmp di, 0xd
    {
        x86::reg16 tmp1 = cpu.di;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(13 /*0xd*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004702b9  0f856e000000           -jne 0x47032d
    if (!cpu.flags.zf)
    {
        goto L_0x0047032d;
    }
    // 004702bf  8d415e                 -lea eax, [ecx + 0x5e]
    cpu.eax = x86::reg32(cpu.ecx + x86::reg32(94) /* 0x5e */);
    // 004702c2  e829bf0200             -call 0x49c1f0
    cpu.esp -= 4;
    nfs3hp::frontend::pointerInBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004702c7  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004702c9  7424                   -je 0x4702ef
    if (cpu.flags.zf)
    {
        goto L_0x004702ef;
    }
    // 004702cb  6683797a00             +cmp word ptr [ecx + 0x7a], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(122) /* 0x7a */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004702d0  741d                   -je 0x4702ef
    if (cpu.flags.zf)
    {
        goto L_0x004702ef;
    }
    // 004702d2  ba7f000000             -mov edx, 0x7f
    cpu.edx = 127 /*0x7f*/;
    // 004702d7  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004702dc  e8ef7efaff             -call 0x4181d0
    cpu.esp -= 4;
    nfs3hp::port::playMenuEffect(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004702e1  668b417a               -mov ax, word ptr [ecx + 0x7a]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(122) /* 0x7a */);
    // 004702e5  48                     -dec eax
    (cpu.eax)--;
    // 004702e6  be02000000             -mov esi, 2
    cpu.esi = 2 /*0x2*/;
    // 004702eb  6689417a               -mov word ptr [ecx + 0x7a], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(122) /* 0x7a */) = cpu.ax;
L_0x004702ef:
    // 004702ef  8d4166                 -lea eax, [ecx + 0x66]
    cpu.eax = x86::reg32(cpu.ecx + x86::reg32(102) /* 0x66 */);
    // 004702f2  e8f9be0200             -call 0x49c1f0
    cpu.esp -= 4;
    nfs3hp::frontend::pointerInBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004702f7  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004702f9  7432                   -je 0x47032d
    if (cpu.flags.zf)
    {
        goto L_0x0047032d;
    }
    // 004702fb  8b4142                 -mov eax, dword ptr [ecx + 0x42]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(66) /* 0x42 */);
    // 004702fe  8b5178                 -mov edx, dword ptr [ecx + 0x78]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(120) /* 0x78 */);
    // 00470301  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00470304  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 00470307  8b7dfc                 -mov edi, dword ptr [ebp - 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0047030a  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 0047030c  39f8                   +cmp eax, edi
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edi));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0047030e  7d1d                   -jge 0x47032d
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0047032d;
    }
    // 00470310  ba7f000000             -mov edx, 0x7f
    cpu.edx = 127 /*0x7f*/;
    // 00470315  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0047031a  e8b17efaff             -call 0x4181d0
    cpu.esp -= 4;
    nfs3hp::port::playMenuEffect(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047031f  668b517a               -mov dx, word ptr [ecx + 0x7a]
    cpu.dx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(122) /* 0x7a */);
    // 00470323  42                     -inc edx
    (cpu.edx)++;
    // 00470324  be02000000             -mov esi, 2
    cpu.esi = 2 /*0x2*/;
    // 00470329  6689517a               -mov word ptr [ecx + 0x7a], dx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(122) /* 0x7a */) = cpu.dx;
L_0x0047032d:
    // 0047032d  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
L_0x0047032f:
    // 0047032f  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00470331  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00470333  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00470334  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00470335  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00470336  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00470337  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_470340(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00470340  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00470341  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00470342  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00470344  ba1a000000             -mov edx, 0x1a
    cpu.edx = 26 /*0x1a*/;
    // 00470349  e812a3fdff             -call 0x44a660
    cpu.esp -= 4;
    nfs3hp::frontend::menuShownNow(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047034e  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00470350  7402                   -je 0x470354
    if (cpu.flags.zf)
    {
        goto L_0x00470354;
    }
    // 00470352  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x00470354:
    // 00470354  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00470355  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00470356  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_470360(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00470360  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00470361  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00470362  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00470363  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00470365  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00470367  ba1b000000             -mov edx, 0x1b
    cpu.edx = 27 /*0x1b*/;
    // 0047036c  e8efa2fdff             -call 0x44a660
    cpu.esp -= 4;
    nfs3hp::frontend::menuShownNow(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00470371  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00470373  7415                   -je 0x47038a
    if (cpu.flags.zf)
    {
        goto L_0x0047038a;
    }
    // 00470375  668b516e               -mov dx, word ptr [ecx + 0x6e]
    cpu.dx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(110) /* 0x6e */);
    // 00470379  6685d2                 +test dx, dx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg16>(cpu.dx & cpu.dx));
    // 0047037c  750c                   -jne 0x47038a
    if (!cpu.flags.zf)
    {
        goto L_0x0047038a;
    }
    // 0047037e  668b4106               -mov ax, word ptr [ecx + 6]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(6) /* 0x6 */);
    // 00470382  6689516c               -mov word ptr [ecx + 0x6c], dx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(108) /* 0x6c */) = cpu.dx;
    // 00470386  6689416e               -mov word ptr [ecx + 0x6e], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(110) /* 0x6e */) = cpu.ax;
L_0x0047038a:
    // 0047038a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047038b  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047038c  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047038d  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_470390(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00470390  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00470391  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00470393  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00470394  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_4703a0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004703a0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004703a1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004703a2  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004703a3  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004703a4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004703a5  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004703a7  81ec54010000           -sub esp, 0x154
    (cpu.esp) -= x86::reg32(x86::sreg32(340 /*0x154*/));
    // 004703ad  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004703af  ba1b000000             -mov edx, 0x1b
    cpu.edx = 27 /*0x1b*/;
    // 004703b4  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004703b6  e8a5a2fdff             -call 0x44a660
    cpu.esp -= 4;
    nfs3hp::frontend::menuShownNow(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004703bb  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004703bd  0f8484060000           -je 0x470a47
    if (cpu.flags.zf)
    {
        goto L_0x00470a47;
    }
    // 004703c3  c745f8aa0000ff         -mov dword ptr [ebp - 8], 0xff0000aa
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = 4278190250 /*0xff0000aa*/;
    // 004703ca  668b516c               -mov dx, word ptr [ecx + 0x6c]
    cpu.dx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(108) /* 0x6c */);
    // 004703ce  bf2894d5ff             -mov edi, 0xffd59428
    cpu.edi = 4292187176 /*0xffd59428*/;
    // 004703d3  6683fa02               +cmp dx, 2
    {
        x86::reg16 tmp1 = cpu.dx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(2 /*0x2*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004703d7  751e                   -jne 0x4703f7
    if (!cpu.flags.zf)
    {
        goto L_0x004703f7;
    }
    // 004703d9  668b596e               -mov bx, word ptr [ecx + 0x6e]
    cpu.bx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(110) /* 0x6e */);
    // 004703dd  83c314                 -add ebx, 0x14
    (cpu.ebx) += x86::reg32(x86::sreg32(20 /*0x14*/));
    // 004703e0  6689596e               -mov word ptr [ecx + 0x6e], bx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(110) /* 0x6e */) = cpu.bx;
    // 004703e4  6681fba802             +cmp bx, 0x2a8
    {
        x86::reg16 tmp1 = cpu.bx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(680 /*0x2a8*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004703e9  7c0c                   -jl 0x4703f7
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004703f7;
    }
    // 004703eb  66c7416ea802           -mov word ptr [ecx + 0x6e], 0x2a8
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(110) /* 0x6e */) = 680 /*0x2a8*/;
    // 004703f1  66c7416c0300           -mov word ptr [ecx + 0x6c], 3
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(108) /* 0x6c */) = 3 /*0x3*/;
L_0x004703f7:
    // 004703f7  66837e6c01             +cmp word ptr [esi + 0x6c], 1
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.esi + x86::reg32(108) /* 0x6c */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(1 /*0x1*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004703fc  7523                   -jne 0x470421
    if (!cpu.flags.zf)
    {
        goto L_0x00470421;
    }
    // 004703fe  66836e6e14             -sub word ptr [esi + 0x6e], 0x14
    (app->getMemory<x86::reg16>(cpu.esi + x86::reg32(110) /* 0x6e */)) -= x86::reg16(x86::sreg16(20 /*0x14*/));
    // 00470403  8b5604                 -mov edx, dword ptr [esi + 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 00470406  8b466c                 -mov eax, dword ptr [esi + 0x6c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(108) /* 0x6c */);
    // 00470409  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0047040c  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0047040f  39d0                   +cmp eax, edx
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00470411  7f0e                   -jg 0x470421
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00470421;
    }
    // 00470413  668b4606               -mov ax, word ptr [esi + 6]
    cpu.ax = app->getMemory<x86::reg16>(cpu.esi + x86::reg32(6) /* 0x6 */);
    // 00470417  66c7466c0000           -mov word ptr [esi + 0x6c], 0
    app->getMemory<x86::reg16>(cpu.esi + x86::reg32(108) /* 0x6c */) = 0 /*0x0*/;
    // 0047041d  6689466e               -mov word ptr [esi + 0x6e], ax
    app->getMemory<x86::reg16>(cpu.esi + x86::reg32(110) /* 0x6e */) = cpu.ax;
L_0x00470421:
    // 00470421  e8ca99fdff             -call 0x449df0
    cpu.esp -= 4;
    nfs3hp::port::loop::keysTaken(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00470426  8b0db0d36f00           -mov ecx, dword ptr [0x6fd3b0]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
    // 0047042c  8945b8                 -mov dword ptr [ebp - 0x48], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-72) /* -0x48 */) = cpu.eax;
    // 0047042f  83f901                 +cmp ecx, 1
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00470432  7504                   -jne 0x470438
    if (!cpu.flags.zf)
    {
        goto L_0x00470438;
    }
    // 00470434  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00470436  eb02                   -jmp 0x47043a
    goto L_0x0047043a;
L_0x00470438:
    // 00470438  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x0047043a:
    // 0047043a  8945d0                 -mov dword ptr [ebp - 0x30], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-48) /* -0x30 */) = cpu.eax;
    // 0047043d  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0047043f  a168bc6f00             -mov eax, dword ptr [0x6fbc68]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7322728) /* 0x6fbc68 */);
    // 00470444  895de8                 -mov dword ptr [ebp - 0x18], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = cpu.ebx;
    // 00470447  83f803                 +cmp eax, 3
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0047044a  7405                   -je 0x470451
    if (cpu.flags.zf)
    {
        goto L_0x00470451;
    }
    // 0047044c  83f804                 +cmp eax, 4
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0047044f  7507                   -jne 0x470458
    if (!cpu.flags.zf)
    {
        goto L_0x00470458;
    }
L_0x00470451:
    // 00470451  c745e801000000         -mov dword ptr [ebp - 0x18], 1
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = 1 /*0x1*/;
L_0x00470458:
    // 00470458  8b463c                 -mov eax, dword ptr [esi + 0x3c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(60) /* 0x3c */);
    // 0047045b  8b563a                 -mov edx, dword ptr [esi + 0x3a]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(58) /* 0x3a */);
    // 0047045e  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00470461  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 00470464  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00470466  8b5dd0                 -mov ebx, dword ptr [ebp - 0x30]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-48) /* -0x30 */);
    // 00470469  8955ec                 -mov dword ptr [ebp - 0x14], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.edx;
    // 0047046c  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0047046e  740b                   -je 0x47047b
    if (cpu.flags.zf)
    {
        goto L_0x0047047b;
    }
    // 00470470  8b463e                 -mov eax, dword ptr [esi + 0x3e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(62) /* 0x3e */);
    // 00470473  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00470476  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00470478  8955ec                 -mov dword ptr [ebp - 0x14], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.edx;
L_0x0047047b:
    // 0047047b  8b5640                 -mov edx, dword ptr [esi + 0x40]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(64) /* 0x40 */);
    // 0047047e  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 00470481  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00470488  68000000ff             -push 0xff000000
    app->getMemory<x86::reg32>(cpu.esp-4) = 4278190080 /*0xff000000*/;
    cpu.esp -= 4;
    // 0047048d  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0047048f  8b5dec                 -mov ebx, dword ptr [ebp - 0x14]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00470492  01c0                   -add eax, eax
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.eax));
    // 00470494  83c302                 -add ebx, 2
    (cpu.ebx) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 00470497  8945dc                 -mov dword ptr [ebp - 0x24], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = cpu.eax;
    // 0047049a  8d4802                 -lea ecx, [eax + 2]
    cpu.ecx = x86::reg32(cpu.eax + x86::reg32(2) /* 0x2 */);
    // 0047049d  8b5606                 -mov edx, dword ptr [esi + 6]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(6) /* 0x6 */);
    // 004704a0  8b466c                 -mov eax, dword ptr [esi + 0x6c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(108) /* 0x6c */);
    // 004704a3  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 004704a6  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 004704a9  e882840600             -call 0x4d8930
    cpu.esp -= 4;
    nfs3hp::port::draw2d::frame(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004704ae  8b4df8                 -mov ecx, dword ptr [ebp - 8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004704b1  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004704b4  8b5606                 -mov edx, dword ptr [esi + 6]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(6) /* 0x6 */);
    // 004704b7  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004704b8  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004704bb  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 004704be  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004704bf  8945ac                 -mov dword ptr [ebp - 0x54], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-84) /* -0x54 */) = cpu.eax;
    // 004704c2  83ea02                 -sub edx, 2
    (cpu.edx) -= x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004704c5  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004704c6  8b5dac                 -mov ebx, dword ptr [ebp - 0x54]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-84) /* -0x54 */);
    // 004704c9  8b466c                 -mov eax, dword ptr [esi + 0x6c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(108) /* 0x6c */);
    // 004704cc  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004704cd  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 004704d0  b902000000             -mov ecx, 2
    cpu.ecx = 2 /*0x2*/;
    // 004704d5  83e802                 -sub eax, 2
    (cpu.eax) -= x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004704d8  e883830600             -call 0x4d8860
    cpu.esp -= 4;
    nfs3hp::port::draw2d::drawShadedBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004704dd  8b45dc                 -mov eax, dword ptr [ebp - 0x24]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 004704e0  b902000000             -mov ecx, 2
    cpu.ecx = 2 /*0x2*/;
    // 004704e5  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004704e6  8b5dac                 -mov ebx, dword ptr [ebp - 0x54]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-84) /* -0x54 */);
    // 004704e9  8b5606                 -mov edx, dword ptr [esi + 6]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(6) /* 0x6 */);
    // 004704ec  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004704ed  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 004704f0  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004704f1  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004704f3  8b466c                 -mov eax, dword ptr [esi + 0x6c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(108) /* 0x6c */);
    // 004704f6  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004704f7  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 004704fa  83c202                 -add edx, 2
    (cpu.edx) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004704fd  83e802                 -sub eax, 2
    (cpu.eax) -= x86::reg32(x86::sreg32(2 /*0x2*/));
    // 00470500  e85b830600             -call 0x4d8860
    cpu.esp -= 4;
    nfs3hp::port::draw2d::drawShadedBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00470505  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00470506  8b45dc                 -mov eax, dword ptr [ebp - 0x24]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 00470509  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0047050a  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0047050d  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00470510  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00470511  8945ac                 -mov dword ptr [ebp - 0x54], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-84) /* -0x54 */) = cpu.eax;
    // 00470514  bb02000000             -mov ebx, 2
    cpu.ebx = 2 /*0x2*/;
    // 00470519  8b4dac                 -mov ecx, dword ptr [ebp - 0x54]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-84) /* -0x54 */);
    // 0047051c  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0047051d  8b466c                 -mov eax, dword ptr [esi + 0x6c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(108) /* 0x6c */);
    // 00470520  8b5606                 -mov edx, dword ptr [esi + 6]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(6) /* 0x6 */);
    // 00470523  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00470526  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 00470529  83e802                 -sub eax, 2
    (cpu.eax) -= x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0047052c  83ea02                 -sub edx, 2
    (cpu.edx) -= x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0047052f  e82c830600             -call 0x4d8860
    cpu.esp -= 4;
    nfs3hp::port::draw2d::drawShadedBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00470534  8b5dec                 -mov ebx, dword ptr [ebp - 0x14]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00470537  8b4dac                 -mov ecx, dword ptr [ebp - 0x54]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-84) /* -0x54 */);
    // 0047053a  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0047053b  8b5606                 -mov edx, dword ptr [esi + 6]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(6) /* 0x6 */);
    // 0047053e  8b466c                 -mov eax, dword ptr [esi + 0x6c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(108) /* 0x6c */);
    // 00470541  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00470542  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 00470545  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00470546  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00470549  83ea02                 -sub edx, 2
    (cpu.edx) -= x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0047054c  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0047054d  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 0047054f  bb02000000             -mov ebx, 2
    cpu.ebx = 2 /*0x2*/;
    // 00470554  e807830600             -call 0x4d8860
    cpu.esp -= 4;
    nfs3hp::port::draw2d::drawShadedBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00470559  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0047055c  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0047055d  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0047055e  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0047055f  b902000000             -mov ecx, 2
    cpu.ecx = 2 /*0x2*/;
    // 00470564  8b5606                 -mov edx, dword ptr [esi + 6]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(6) /* 0x6 */);
    // 00470567  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00470568  8b4640                 -mov eax, dword ptr [esi + 0x40]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(64) /* 0x40 */);
    // 0047056b  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0047056e  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00470571  8b5dec                 -mov ebx, dword ptr [ebp - 0x14]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00470574  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00470576  8b466c                 -mov eax, dword ptr [esi + 0x6c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(108) /* 0x6c */);
    // 00470579  4a                     -dec edx
    (cpu.edx)--;
    // 0047057a  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0047057d  e8de820600             -call 0x4d8860
    cpu.esp -= 4;
    nfs3hp::port::draw2d::drawShadedBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00470582  8b466c                 -mov eax, dword ptr [esi + 0x6c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(108) /* 0x6c */);
    // 00470585  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00470587  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0047058a  894dd8                 -mov dword ptr [ebp - 0x28], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-40) /* -0x28 */) = cpu.ecx;
    // 0047058d  8945c8                 -mov dword ptr [ebp - 0x38], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-56) /* -0x38 */) = cpu.eax;
L_0x00470590:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00470590  8b45d8                 -mov eax, dword ptr [ebp - 0x28]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-40) /* -0x28 */);
    // 00470593  3b45d0                 +cmp eax, dword ptr [ebp - 0x30]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-48) /* -0x30 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00470596  7f34                   -jg 0x4705cc
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x004705cc;
    }
    // 00470598  8b55c8                 -mov edx, dword ptr [ebp - 0x38]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-56) /* -0x38 */);
    // 0047059b  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0047059c  8b4df8                 -mov ecx, dword ptr [ebp - 8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0047059f  8b44463a               -mov eax, dword ptr [esi + eax*2 + 0x3a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(58) /* 0x3a */ + cpu.eax * 2);
    // 004705a3  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004705a4  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 004705a7  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004705a8  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004705aa  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 004705af  8955c8                 -mov dword ptr [ebp - 0x38], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-56) /* -0x38 */) = cpu.edx;
    // 004705b2  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004705b3  8b4ddc                 -mov ecx, dword ptr [ebp - 0x24]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 004705b6  8b45c8                 -mov eax, dword ptr [ebp - 0x38]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-56) /* -0x38 */);
    // 004705b9  8b5606                 -mov edx, dword ptr [esi + 6]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(6) /* 0x6 */);
    // 004705bc  83c102                 -add ecx, 2
    (cpu.ecx) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004705bf  c1fa10                 +sar edx, 0x10
    {
        x86::reg8 tmp = 16 /*0x10*/ % 32;
        x86::reg32& op = cpu.edx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (x86::sreg32(op) >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = 0;
            cpu.set_szp((op = x86::reg32(x86::sreg32(op) >> tmp)));
        }
    }
    // 004705c2  e899820600             -call 0x4d8860
    cpu.esp -= 4;
    nfs3hp::port::draw2d::drawShadedBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004705c7  ff45d8                 +inc dword ptr [ebp - 0x28]
    {
        auto tmp = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-40) /* -0x28 */);
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004705ca  ebc4                   -jmp 0x470590
    goto L_0x00470590;
L_0x004705cc:
    // 004705cc  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004705ce  8b4606                 -mov eax, dword ptr [esi + 6]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(6) /* 0x6 */);
    // 004705d1  8955cc                 -mov dword ptr [ebp - 0x34], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-52) /* -0x34 */) = cpu.edx;
    // 004705d4  8b5640                 -mov edx, dword ptr [esi + 0x40]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(64) /* 0x40 */);
    // 004705d7  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 004705da  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 004705dd  01d0                   +add eax, edx
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004705df  8945c0                 -mov dword ptr [ebp - 0x40], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-64) /* -0x40 */) = cpu.eax;
    // 004705e2  eb06                   -jmp 0x4705ea
    goto L_0x004705ea;
L_0x004705e4:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004705e4  837dcc05               +cmp dword ptr [ebp - 0x34], 5
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-52) /* -0x34 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(5 /*0x5*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004705e8  7d34                   -jge 0x47061e
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0047061e;
    }
L_0x004705ea:
    // 004705ea  8b5df8                 -mov ebx, dword ptr [ebp - 8]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004705ed  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004705ee  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004705ef  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 004705f4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004705f5  8b55c0                 -mov edx, dword ptr [ebp - 0x40]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-64) /* -0x40 */);
    // 004705f8  8b466c                 -mov eax, dword ptr [esi + 0x6c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(108) /* 0x6c */);
    // 004705fb  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004705fc  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 004705ff  8b5dec                 -mov ebx, dword ptr [ebp - 0x14]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00470602  e859820600             -call 0x4d8860
    cpu.esp -= 4;
    nfs3hp::port::draw2d::drawShadedBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00470607  8b4dc0                 -mov ecx, dword ptr [ebp - 0x40]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-64) /* -0x40 */);
    // 0047060a  8b5640                 -mov edx, dword ptr [esi + 0x40]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(64) /* 0x40 */);
    // 0047060d  8b5dcc                 -mov ebx, dword ptr [ebp - 0x34]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-52) /* -0x34 */);
    // 00470610  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 00470613  43                     -inc ebx
    (cpu.ebx)++;
    // 00470614  01d1                   +add ecx, edx
    {
        x86::reg32& tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00470616  895dcc                 -mov dword ptr [ebp - 0x34], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-52) /* -0x34 */) = cpu.ebx;
    // 00470619  894dc0                 -mov dword ptr [ebp - 0x40], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-64) /* -0x40 */) = cpu.ecx;
    // 0047061c  ebc6                   -jmp 0x4705e4
    goto L_0x004705e4;
L_0x0047061e:
    // 0047061e  8b466c                 -mov eax, dword ptr [esi + 0x6c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(108) /* 0x6c */);
    // 00470621  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00470624  8945b0                 -mov dword ptr [ebp - 0x50], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-80) /* -0x50 */) = cpu.eax;
    // 00470627  8b4606                 -mov eax, dword ptr [esi + 6]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(6) /* 0x6 */);
    // 0047062a  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0047062d  8d0c10                 -lea ecx, [eax + edx]
    cpu.ecx = x86::reg32(cpu.eax + cpu.edx * 1);
    // 00470630  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00470632  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00470635  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00470637  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 00470639  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0047063b  8b7db8                 -mov edi, dword ptr [ebp - 0x48]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-72) /* -0x48 */);
    // 0047063e  894dc4                 -mov dword ptr [ebp - 0x3c], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-60) /* -0x3c */) = cpu.ecx;
    // 00470641  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 00470643  7407                   -je 0x47064c
    if (cpu.flags.zf)
    {
        goto L_0x0047064c;
    }
    // 00470645  b8964a0000             -mov eax, 0x4a96
    cpu.eax = 19094 /*0x4a96*/;
    // 0047064a  eb05                   -jmp 0x470651
    goto L_0x00470651;
L_0x0047064c:
    // 0047064c  b82894d5ff             -mov eax, 0xffd59428
    cpu.eax = 4292187176 /*0xffd59428*/;
L_0x00470651:
    // 00470651  8945b4                 -mov dword ptr [ebp - 0x4c], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-76) /* -0x4c */) = cpu.eax;
    // 00470654  31ff                   +xor edi, edi
    cpu.clear_co();
    cpu.set_szp((cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi))));
    // 00470656  eb05                   -jmp 0x47065d
    goto L_0x0047065d;
L_0x00470658:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00470658  83ff05                 +cmp edi, 5
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(5 /*0x5*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0047065b  7d42                   -jge 0x47069f
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0047069f;
    }
L_0x0047065d:
    // 0047065d  8b45b4                 -mov eax, dword ptr [ebp - 0x4c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-76) /* -0x4c */);
    // 00470660  8b5dc4                 -mov ebx, dword ptr [ebp - 0x3c]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-60) /* -0x3c */);
    // 00470663  8b55b0                 -mov edx, dword ptr [ebp - 0x50]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-80) /* -0x50 */);
    // 00470666  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00470667  8d04fd00000000         -lea eax, [edi*8]
    cpu.eax = x86::reg32(cpu.edi * 8);
    // 0047066e  83eb0a                 -sub ebx, 0xa
    (cpu.ebx) -= x86::reg32(x86::sreg32(10 /*0xa*/));
    // 00470671  8d0c06                 -lea ecx, [esi + eax]
    cpu.ecx = x86::reg32(cpu.esi + cpu.eax * 1);
    // 00470674  8b45e8                 -mov eax, dword ptr [ebp - 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 00470677  83c20a                 -add edx, 0xa
    (cpu.edx) += x86::reg32(x86::sreg32(10 /*0xa*/));
    // 0047067a  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 0047067d  6a03                   -push 3
    app->getMemory<x86::reg32>(cpu.esp-4) = 3 /*0x3*/;
    cpu.esp -= 4;
    // 0047067f  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 00470681  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 00470686  8b4044                 -mov eax, dword ptr [eax + 0x44]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(68) /* 0x44 */);
    // 00470689  e8021bfeff             -call 0x452190
    cpu.esp -= 4;
    nfs3hp::port::text::drawMenuText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047068e  8b4640                 -mov eax, dword ptr [esi + 0x40]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(64) /* 0x40 */);
    // 00470691  8b5dc4                 -mov ebx, dword ptr [ebp - 0x3c]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-60) /* -0x3c */);
    // 00470694  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00470697  01c3                   +add ebx, eax
    {
        x86::reg32& tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00470699  47                     +inc edi
    {
        x86::reg32& tmp = cpu.edi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0047069a  895dc4                 -mov dword ptr [ebp - 0x3c], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-60) /* -0x3c */) = cpu.ebx;
    // 0047069d  ebb9                   -jmp 0x470658
    goto L_0x00470658;
L_0x0047069f:
    // 0047069f  8b463a                 -mov eax, dword ptr [esi + 0x3a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(58) /* 0x3a */);
    // 004706a2  8b566c                 -mov edx, dword ptr [esi + 0x6c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(108) /* 0x6c */);
    // 004706a5  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 004706a7  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 004706aa  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 004706ad  897de0                 -mov dword ptr [ebp - 0x20], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */) = cpu.edi;
    // 004706b0  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004706b2  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004706b7  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 004706ba  8945f0                 -mov dword ptr [ebp - 0x10], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.eax;
L_0x004706bd:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004706bd  8b45e0                 -mov eax, dword ptr [ebp - 0x20]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 004706c0  3b45d0                 +cmp eax, dword ptr [ebp - 0x30]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-48) /* -0x30 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004706c3  0f8f7e030000           -jg 0x470a47
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00470a47;
    }
    // 004706c9  8b5640                 -mov edx, dword ptr [esi + 0x40]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(64) /* 0x40 */);
    // 004706cc  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 004706cf  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004706d1  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004706d4  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004706d6  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 004706d8  8b5606                 -mov edx, dword ptr [esi + 6]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(6) /* 0x6 */);
    // 004706db  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 004706de  8b4de0                 -mov ecx, dword ptr [ebp - 0x20]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 004706e1  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004706e3  8b15f4d46f00           -mov edx, dword ptr [0x6fd4f4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */);
    // 004706e9  8d78fb                 -lea edi, [eax - 5]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(-5) /* -0x5 */);
    // 004706ec  01ca                   -add edx, ecx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004706ee  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004706f0  e8eb4dfeff             -call 0x4554e0
    cpu.esp -= 4;
    sub_4554e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004706f5  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
    // 004706f8  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004706fa  7515                   -jne 0x470711
    if (!cpu.flags.zf)
    {
        goto L_0x00470711;
    }
    // 004706fc  8b15f4d46f00           -mov edx, dword ptr [0x6fd4f4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */);
    // 00470702  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00470707  01ca                   -add edx, ecx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.ecx));
    // 00470709  e8d24dfeff             -call 0x4554e0
    cpu.esp -= 4;
    sub_4554e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047070e  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
L_0x00470711:
    // 00470711  837db800               +cmp dword ptr [ebp - 0x48], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-72) /* -0x48 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00470715  7407                   -je 0x47071e
    if (cpu.flags.zf)
    {
        goto L_0x0047071e;
    }
    // 00470717  b9964a0000             -mov ecx, 0x4a96
    cpu.ecx = 19094 /*0x4a96*/;
    // 0047071c  eb05                   -jmp 0x470723
    goto L_0x00470723;
L_0x0047071e:
    // 0047071e  b940e4ff00             -mov ecx, 0xffe440
    cpu.ecx = 16770112 /*0xffe440*/;
L_0x00470723:
    // 00470723  8d95acfeffff           -lea edx, [ebp - 0x154]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-340) /* -0x154 */);
    // 00470729  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0047072c  e83f4efeff             -call 0x455570
    cpu.esp -= 4;
    sub_455570(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00470731  8b55f0                 -mov edx, dword ptr [ebp - 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00470734  8b54563a               -mov edx, dword ptr [esi + edx*2 + 0x3a]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(58) /* 0x3a */ + cpu.edx * 2);
    // 00470738  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0047073b  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0047073d  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00470740  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00470742  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 00470744  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00470745  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 00470747  6a05                   -push 5
    app->getMemory<x86::reg32>(cpu.esp-4) = 5 /*0x5*/;
    cpu.esp -= 4;
    // 00470749  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0047074c  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 00470751  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00470753  8d85acfeffff           -lea eax, [ebp - 0x154]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-340) /* -0x154 */);
    // 00470759  e8321afeff             -call 0x452190
    cpu.esp -= 4;
    nfs3hp::port::text::drawMenuText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047075e  837db800               +cmp dword ptr [ebp - 0x48], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-72) /* -0x48 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00470762  7409                   -je 0x47076d
    if (cpu.flags.zf)
    {
        goto L_0x0047076d;
    }
    // 00470764  c745e4964a0000         -mov dword ptr [ebp - 0x1c], 0x4a96
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */) = 19094 /*0x4a96*/;
    // 0047076b  eb07                   -jmp 0x470774
    goto L_0x00470774;
L_0x0047076d:
    // 0047076d  c745e4fd9d64ff         -mov dword ptr [ebp - 0x1c], 0xff649dfd
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */) = 4284784125 /*0xff649dfd*/;
L_0x00470774:
    // 00470774  8b45e4                 -mov eax, dword ptr [ebp - 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 00470777  8945bc                 -mov dword ptr [ebp - 0x44], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-68) /* -0x44 */) = cpu.eax;
    // 0047077a  8b4640                 -mov eax, dword ptr [esi + 0x40]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(64) /* 0x40 */);
    // 0047077d  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00470780  8d95acfeffff           -lea edx, [ebp - 0x154]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-340) /* -0x154 */);
    // 00470786  01c7                   -add edi, eax
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.eax));
    // 00470788  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0047078b  e8d04efeff             -call 0x455660
    cpu.esp -= 4;
    sub_455660(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00470790  8b45f0                 -mov eax, dword ptr [ebp - 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00470793  01c0                   -add eax, eax
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.eax));
    // 00470795  8d1406                 -lea edx, [esi + eax]
    cpu.edx = x86::reg32(cpu.esi + cpu.eax * 1);
    // 00470798  8955d4                 -mov dword ptr [ebp - 0x2c], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44) /* -0x2c */) = cpu.edx;
    // 0047079b  8b523a                 -mov edx, dword ptr [edx + 0x3a]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(58) /* 0x3a */);
    // 0047079e  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 004707a1  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004707a2  8b45e4                 -mov eax, dword ptr [ebp - 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 004707a5  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004707a6  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004707a8  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004707ab  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004707ad  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 004707af  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 004707b4  8d5ff6                 -lea ebx, [edi - 0xa]
    cpu.ebx = x86::reg32(cpu.edi + x86::reg32(-10) /* -0xa */);
    // 004707b7  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004707ba  6a05                   -push 5
    app->getMemory<x86::reg32>(cpu.esp-4) = 5 /*0x5*/;
    cpu.esp -= 4;
    // 004707bc  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004707be  8d85acfeffff           -lea eax, [ebp - 0x154]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-340) /* -0x154 */);
    // 004707c4  e85719feff             -call 0x452120
    cpu.esp -= 4;
    sub_452120(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004707c9  8d95acfeffff           -lea edx, [ebp - 0x154]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-340) /* -0x154 */);
    // 004707cf  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004707d2  e8e94efeff             -call 0x4556c0
    cpu.esp -= 4;
    sub_4556c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004707d7  8b55d4                 -mov edx, dword ptr [ebp - 0x2c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44) /* -0x2c */);
    // 004707da  8b523a                 -mov edx, dword ptr [edx + 0x3a]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(58) /* 0x3a */);
    // 004707dd  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 004707e0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004707e1  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004707e3  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004707e6  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004707e8  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 004707ea  8b4de4                 -mov ecx, dword ptr [ebp - 0x1c]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 004707ed  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004707ee  8d5f05                 -lea ebx, [edi + 5]
    cpu.ebx = x86::reg32(cpu.edi + x86::reg32(5) /* 0x5 */);
    // 004707f1  6a05                   -push 5
    app->getMemory<x86::reg32>(cpu.esp-4) = 5 /*0x5*/;
    cpu.esp -= 4;
    // 004707f3  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004707f6  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 004707fb  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004707fd  8d85acfeffff           -lea eax, [ebp - 0x154]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-340) /* -0x154 */);
    // 00470803  e81819feff             -call 0x452120
    cpu.esp -= 4;
    sub_452120(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00470808  8b4640                 -mov eax, dword ptr [esi + 0x40]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(64) /* 0x40 */);
    // 0047080b  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0047080e  01c7                   -add edi, eax
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.eax));
    // 00470810  8b45e8                 -mov eax, dword ptr [ebp - 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 00470813  8b5df4                 -mov ebx, dword ptr [ebp - 0xc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00470816  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00470819  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 0047081b  8b9068020000           -mov edx, dword ptr [eax + 0x268]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(616) /* 0x268 */);
    // 00470821  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 00470823  7439                   -je 0x47085e
    if (cpu.flags.zf)
    {
        goto L_0x0047085e;
    }
    // 00470825  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00470827  8d95acfeffff           -lea edx, [ebp - 0x154]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-340) /* -0x154 */);
    // 0047082d  e80ee60500             -call 0x4cee40
    cpu.esp -= 4;
    sub_4cee40(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00470832  8b55d4                 -mov edx, dword ptr [ebp - 0x2c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44) /* -0x2c */);
    // 00470835  8b523a                 -mov edx, dword ptr [edx + 0x3a]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(58) /* 0x3a */);
    // 00470838  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0047083b  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0047083d  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00470840  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00470842  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 00470844  8b4de4                 -mov ecx, dword ptr [ebp - 0x1c]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 00470847  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00470848  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 0047084a  6a05                   -push 5
    app->getMemory<x86::reg32>(cpu.esp-4) = 5 /*0x5*/;
    cpu.esp -= 4;
    // 0047084c  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0047084f  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 00470854  01c2                   +add edx, eax
    {
        x86::reg32& tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00470856  8d85acfeffff           -lea eax, [ebp - 0x154]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-340) /* -0x154 */);
    // 0047085c  eb2a                   -jmp 0x470888
    goto L_0x00470888;
L_0x0047085e:
    // 0047085e  8b55d4                 -mov edx, dword ptr [ebp - 0x2c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44) /* -0x2c */);
    // 00470861  8b523a                 -mov edx, dword ptr [edx + 0x3a]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(58) /* 0x3a */);
    // 00470864  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 00470867  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00470869  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0047086c  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0047086e  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 00470870  68964a0000             -push 0x4a96
    app->getMemory<x86::reg32>(cpu.esp-4) = 19094 /*0x4a96*/;
    cpu.esp -= 4;
    // 00470875  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 0047087a  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 0047087c  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0047087f  6a05                   -push 5
    app->getMemory<x86::reg32>(cpu.esp-4) = 5 /*0x5*/;
    cpu.esp -= 4;
    // 00470881  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00470883  b8f8ae5300             -mov eax, 0x53aef8
    cpu.eax = 5484280 /*0x53aef8*/;
L_0x00470888:
    // 00470888  e80319feff             -call 0x452190
    cpu.esp -= 4;
    nfs3hp::port::text::drawMenuText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047088d  8b4640                 -mov eax, dword ptr [esi + 0x40]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(64) /* 0x40 */);
    // 00470890  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00470893  8b5df4                 -mov ebx, dword ptr [ebp - 0xc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00470896  01c7                   -add edi, eax
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.eax));
    // 00470898  8b45e8                 -mov eax, dword ptr [ebp - 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 0047089b  8b55f0                 -mov edx, dword ptr [ebp - 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0047089e  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004708a1  01d2                   -add edx, edx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.edx));
    // 004708a3  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 004708a5  8d0c16                 -lea ecx, [esi + edx]
    cpu.ecx = x86::reg32(cpu.esi + cpu.edx * 1);
    // 004708a8  8b9070020000           -mov edx, dword ptr [eax + 0x270]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(624) /* 0x270 */);
    // 004708ae  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004708b0  7436                   -je 0x4708e8
    if (cpu.flags.zf)
    {
        goto L_0x004708e8;
    }
    // 004708b2  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004708b4  8d95acfeffff           -lea edx, [ebp - 0x154]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-340) /* -0x154 */);
    // 004708ba  e881e50500             -call 0x4cee40
    cpu.esp -= 4;
    sub_4cee40(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004708bf  8b513a                 -mov edx, dword ptr [ecx + 0x3a]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(58) /* 0x3a */);
    // 004708c2  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 004708c5  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004708c7  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004708ca  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004708cc  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 004708ce  8b5dbc                 -mov ebx, dword ptr [ebp - 0x44]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-68) /* -0x44 */);
    // 004708d1  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004708d2  6a05                   -push 5
    app->getMemory<x86::reg32>(cpu.esp-4) = 5 /*0x5*/;
    cpu.esp -= 4;
    // 004708d4  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 004708d9  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004708dc  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 004708de  01c2                   +add edx, eax
    {
        x86::reg32& tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004708e0  8d85acfeffff           -lea eax, [ebp - 0x154]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-340) /* -0x154 */);
    // 004708e6  eb27                   -jmp 0x47090f
    goto L_0x0047090f;
L_0x004708e8:
    // 004708e8  8b513a                 -mov edx, dword ptr [ecx + 0x3a]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(58) /* 0x3a */);
    // 004708eb  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 004708ee  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004708f0  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004708f3  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004708f5  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 004708f7  68964a0000             -push 0x4a96
    app->getMemory<x86::reg32>(cpu.esp-4) = 19094 /*0x4a96*/;
    cpu.esp -= 4;
    // 004708fc  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 004708fe  6a05                   -push 5
    app->getMemory<x86::reg32>(cpu.esp-4) = 5 /*0x5*/;
    cpu.esp -= 4;
    // 00470900  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00470903  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 00470908  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0047090a  b8f8ae5300             -mov eax, 0x53aef8
    cpu.eax = 5484280 /*0x53aef8*/;
L_0x0047090f:
    // 0047090f  e87c18feff             -call 0x452190
    cpu.esp -= 4;
    nfs3hp::port::text::drawMenuText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00470914  8b4640                 -mov eax, dword ptr [esi + 0x40]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(64) /* 0x40 */);
    // 00470917  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0047091a  01c7                   -add edi, eax
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.eax));
    // 0047091c  8b45e8                 -mov eax, dword ptr [ebp - 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 0047091f  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00470922  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00470925  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00470927  8b55f0                 -mov edx, dword ptr [ebp - 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0047092a  01d2                   -add edx, edx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.edx));
    // 0047092c  8b9878020000           -mov ebx, dword ptr [eax + 0x278]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(632) /* 0x278 */);
    // 00470932  8d0c16                 -lea ecx, [esi + edx]
    cpu.ecx = x86::reg32(cpu.esi + cpu.edx * 1);
    // 00470935  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 00470937  7436                   -je 0x47096f
    if (cpu.flags.zf)
    {
        goto L_0x0047096f;
    }
    // 00470939  8d95acfeffff           -lea edx, [ebp - 0x154]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-340) /* -0x154 */);
    // 0047093f  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 00470941  e8fae40500             -call 0x4cee40
    cpu.esp -= 4;
    sub_4cee40(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00470946  8b513a                 -mov edx, dword ptr [ecx + 0x3a]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(58) /* 0x3a */);
    // 00470949  8b45bc                 -mov eax, dword ptr [ebp - 0x44]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-68) /* -0x44 */);
    // 0047094c  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0047094f  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00470950  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00470952  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00470955  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00470957  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 00470959  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 0047095b  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 00470960  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00470963  6a05                   -push 5
    app->getMemory<x86::reg32>(cpu.esp-4) = 5 /*0x5*/;
    cpu.esp -= 4;
    // 00470965  01c2                   +add edx, eax
    {
        x86::reg32& tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00470967  8d85acfeffff           -lea eax, [ebp - 0x154]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-340) /* -0x154 */);
    // 0047096d  eb27                   -jmp 0x470996
    goto L_0x00470996;
L_0x0047096f:
    // 0047096f  8b513a                 -mov edx, dword ptr [ecx + 0x3a]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(58) /* 0x3a */);
    // 00470972  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 00470975  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00470977  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0047097a  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0047097c  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0047097e  68964a0000             -push 0x4a96
    app->getMemory<x86::reg32>(cpu.esp-4) = 19094 /*0x4a96*/;
    cpu.esp -= 4;
    // 00470983  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 00470985  6a05                   -push 5
    app->getMemory<x86::reg32>(cpu.esp-4) = 5 /*0x5*/;
    cpu.esp -= 4;
    // 00470987  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0047098a  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 0047098f  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00470991  b8f8ae5300             -mov eax, 0x53aef8
    cpu.eax = 5484280 /*0x53aef8*/;
L_0x00470996:
    // 00470996  e8f517feff             -call 0x452190
    cpu.esp -= 4;
    nfs3hp::port::text::drawMenuText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047099b  8b4640                 -mov eax, dword ptr [esi + 0x40]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(64) /* 0x40 */);
    // 0047099e  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 004709a1  01c7                   -add edi, eax
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.eax));
    // 004709a3  8b45e8                 -mov eax, dword ptr [ebp - 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004709a6  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004709a9  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004709ac  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004709ae  8b55f0                 -mov edx, dword ptr [ebp - 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004709b1  01d2                   -add edx, edx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.edx));
    // 004709b3  8b9880020000           -mov ebx, dword ptr [eax + 0x280]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(640) /* 0x280 */);
    // 004709b9  8d0c16                 -lea ecx, [esi + edx]
    cpu.ecx = x86::reg32(cpu.esi + cpu.edx * 1);
    // 004709bc  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 004709be  7436                   -je 0x4709f6
    if (cpu.flags.zf)
    {
        goto L_0x004709f6;
    }
    // 004709c0  8d95acfeffff           -lea edx, [ebp - 0x154]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-340) /* -0x154 */);
    // 004709c6  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 004709c8  e873e40500             -call 0x4cee40
    cpu.esp -= 4;
    sub_4cee40(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004709cd  8b513a                 -mov edx, dword ptr [ecx + 0x3a]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(58) /* 0x3a */);
    // 004709d0  8b45bc                 -mov eax, dword ptr [ebp - 0x44]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-68) /* -0x44 */);
    // 004709d3  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 004709d6  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004709d7  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004709d9  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004709dc  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004709de  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 004709e0  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 004709e2  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 004709e7  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004709ea  6a05                   -push 5
    app->getMemory<x86::reg32>(cpu.esp-4) = 5 /*0x5*/;
    cpu.esp -= 4;
    // 004709ec  01c2                   +add edx, eax
    {
        x86::reg32& tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004709ee  8d85acfeffff           -lea eax, [ebp - 0x154]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-340) /* -0x154 */);
    // 004709f4  eb27                   -jmp 0x470a1d
    goto L_0x00470a1d;
L_0x004709f6:
    // 004709f6  8b513a                 -mov edx, dword ptr [ecx + 0x3a]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(58) /* 0x3a */);
    // 004709f9  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 004709fc  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004709fe  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00470a01  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00470a03  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 00470a05  68964a0000             -push 0x4a96
    app->getMemory<x86::reg32>(cpu.esp-4) = 19094 /*0x4a96*/;
    cpu.esp -= 4;
    // 00470a0a  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 00470a0c  6a05                   -push 5
    app->getMemory<x86::reg32>(cpu.esp-4) = 5 /*0x5*/;
    cpu.esp -= 4;
    // 00470a0e  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00470a11  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 00470a16  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00470a18  b8f8ae5300             -mov eax, 0x53aef8
    cpu.eax = 5484280 /*0x53aef8*/;
L_0x00470a1d:
    // 00470a1d  e86e17feff             -call 0x452190
    cpu.esp -= 4;
    nfs3hp::port::text::drawMenuText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00470a22  8b4de0                 -mov ecx, dword ptr [ebp - 0x20]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 00470a25  8b45f0                 -mov eax, dword ptr [ebp - 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00470a28  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00470a2b  8b5df0                 -mov ebx, dword ptr [ebp - 0x10]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00470a2e  41                     -inc ecx
    (cpu.ecx)++;
    // 00470a2f  8b44463a               -mov eax, dword ptr [esi + eax*2 + 0x3a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(58) /* 0x3a */ + cpu.eax * 2);
    // 00470a33  43                     -inc ebx
    (cpu.ebx)++;
    // 00470a34  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00470a37  894de0                 -mov dword ptr [ebp - 0x20], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */) = cpu.ecx;
    // 00470a3a  01c2                   +add edx, eax
    {
        x86::reg32& tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00470a3c  895df0                 -mov dword ptr [ebp - 0x10], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.ebx;
    // 00470a3f  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 00470a42  e976fcffff             -jmp 0x4706bd
    goto L_0x004706bd;
L_0x00470a47:
    // 00470a47  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00470a49  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00470a4b  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00470a4d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00470a4e  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00470a4f  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00470a50  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00470a51  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00470a52  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_470a60(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00470a60  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00470a61  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00470a63  a1408b6700             -mov eax, dword ptr [0x678b40]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6785856) /* 0x678b40 */);
    // 00470a68  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00470a69  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_470a70: native, nfs3hp::dead::stub_0x470a70 */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_470aa0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00470aa0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00470aa1  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00470aa2  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00470aa4  833d643a7a0000         +cmp dword ptr [0x7a3a64], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8010340) /* 0x7a3a64 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00470aab  7411                   -je 0x470abe
    if (cpu.flags.zf)
    {
        goto L_0x00470abe;
    }
    // 00470aad  a3448b6700             -mov dword ptr [0x678b44], eax
    app->getMemory<x86::reg32>(x86::reg32(6785860) /* 0x678b44 */) = cpu.eax;
    // 00470ab2  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00470ab4  b80f000000             -mov eax, 0xf
    cpu.eax = 15 /*0xf*/;
    // 00470ab9  e8420efcff             -call 0x431900
    cpu.esp -= 4;
    nfs3hp::graphics::setRenderState(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00470abe:
    // 00470abe  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00470abf  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00470ac0  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_470ad0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00470ad0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00470ad1  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00470ad2  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00470ad4  833d643a7a0000         +cmp dword ptr [0x7a3a64], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8010340) /* 0x7a3a64 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00470adb  743e                   -je 0x470b1b
    if (cpu.flags.zf)
    {
        goto L_0x00470b1b;
    }
    // 00470add  a3408b6700             -mov dword ptr [0x678b40], eax
    app->getMemory<x86::reg32>(x86::reg32(6785856) /* 0x678b40 */) = cpu.eax;
    // 00470ae2  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00470ae4  7424                   -je 0x470b0a
    if (cpu.flags.zf)
    {
        goto L_0x00470b0a;
    }
    // 00470ae6  ba64000000             -mov edx, 0x64
    cpu.edx = 100 /*0x64*/;
    // 00470aeb  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 00470aed  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00470aef  83fa64                 +cmp edx, 0x64
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(100 /*0x64*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00470af2  7f0c                   -jg 0x470b00
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00470b00;
    }
    // 00470af4  83fa01                 +cmp edx, 1
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00470af7  7d07                   -jge 0x470b00
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00470b00;
    }
    // 00470af9  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00470afe  eb0a                   -jmp 0x470b0a
    goto L_0x00470b0a;
L_0x00470b00:
    // 00470b00  83f864                 +cmp eax, 0x64
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(100 /*0x64*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00470b03  7e05                   -jle 0x470b0a
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00470b0a;
    }
    // 00470b05  b864000000             -mov eax, 0x64
    cpu.eax = 100 /*0x64*/;
L_0x00470b0a:
    // 00470b0a  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00470b0c  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00470b0f  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00470b11  b80e000000             -mov eax, 0xe
    cpu.eax = 14 /*0xe*/;
    // 00470b16  e8e50dfcff             -call 0x431900
    cpu.esp -= 4;
    nfs3hp::graphics::setRenderState(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00470b1b:
    // 00470b1b  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00470b1c  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00470b1d  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_470b20(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00470b20  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00470b21  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00470b22  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00470b24  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 00470b26  833d643a7a0000         +cmp dword ptr [0x7a3a64], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8010340) /* 0x7a3a64 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00470b2d  7438                   -je 0x470b67
    if (cpu.flags.zf)
    {
        goto L_0x00470b67;
    }
    // 00470b2f  ba64000000             -mov edx, 0x64
    cpu.edx = 100 /*0x64*/;
    // 00470b34  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 00470b36  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00470b38  83fa64                 +cmp edx, 0x64
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(100 /*0x64*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00470b3b  7f0c                   -jg 0x470b49
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00470b49;
    }
    // 00470b3d  83fa01                 +cmp edx, 1
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00470b40  7d07                   -jge 0x470b49
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00470b49;
    }
    // 00470b42  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00470b47  eb0a                   -jmp 0x470b53
    goto L_0x00470b53;
L_0x00470b49:
    // 00470b49  83f864                 +cmp eax, 0x64
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(100 /*0x64*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00470b4c  7e05                   -jle 0x470b53
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00470b53;
    }
    // 00470b4e  b864000000             -mov eax, 0x64
    cpu.eax = 100 /*0x64*/;
L_0x00470b53:
    // 00470b53  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00470b55  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00470b58  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00470b5a  0fafd1                 -imul edx, ecx
    cpu.edx = x86::reg32(x86::sreg64(x86::sreg32(cpu.edx)) * x86::sreg64(x86::sreg32(cpu.ecx)));
    // 00470b5d  b80e000000             -mov eax, 0xe
    cpu.eax = 14 /*0xe*/;
    // 00470b62  e8990dfcff             -call 0x431900
    cpu.esp -= 4;
    nfs3hp::graphics::setRenderState(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00470b67:
    // 00470b67  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00470b68  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00470b69  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_470b70(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00470b70  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00470b71  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00470b72  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00470b74  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00470b76  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00470b78  e853ffffff             -call 0x470ad0
    cpu.esp -= 4;
    sub_470ad0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00470b7d  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00470b7f  e81cffffff             -call 0x470aa0
    cpu.esp -= 4;
    sub_470aa0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00470b84  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00470b85  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00470b86  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_470b90(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00470b90  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00470b91  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00470b92  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00470b93  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00470b95  baffffffff             -mov edx, 0xffffffff
    cpu.edx = 4294967295 /*0xffffffff*/;
    // 00470b9a  b80e000000             -mov eax, 0xe
    cpu.eax = 14 /*0xe*/;
    // 00470b9f  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00470ba1  8915408b6700           -mov dword ptr [0x678b40], edx
    app->getMemory<x86::reg32>(x86::reg32(6785856) /* 0x678b40 */) = cpu.edx;
    // 00470ba7  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00470ba9  890d448b6700           -mov dword ptr [0x678b44], ecx
    app->getMemory<x86::reg32>(x86::reg32(6785860) /* 0x678b44 */) = cpu.ecx;
    // 00470baf  e84c0dfcff             -call 0x431900
    cpu.esp -= 4;
    nfs3hp::graphics::setRenderState(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00470bb4  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00470bb5  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00470bb6  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00470bb7  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_470bc0: native, nfs3hp::controls::steadyEngineRumble */

/* sub_470c60: native, nfs3hp::controls::startEngineRumble */

/* sub_470cb0: native, nfs3hp::controls::silenceEngineRumble */

/* sub_470d10: native, nfs3hp::controls::setForceGain */

/* sub_470d30: native, nfs3hp::controls::findForceController */

/* sub_470dc0: native, nfs3hp::controls::createForceEffects */

/* sub_470f40: native, nfs3hp::controls::stopForces */

/* sub_470f80: native, nfs3hp::controls::stopForcesAgain */

/* sub_470fc0: native, nfs3hp::controls::stopForcesOnceMore */

/* sub_471000: native, nfs3hp::controls::stopForcesOrActuatorsOn */

/* sub_471070: native, nfs3hp::controls::stopForcesOrActuatorsOff */

/* sub_4710f0: native, nfs3hp::controls::forcePlayerCar */

/* sub_4711e0: native, nfs3hp::controls::startRaceForces */

/* sub_471240: native, nfs3hp::controls::stopRaceEffects */

/* sub_471270: native, nfs3hp::controls::crashJolt */

/* sub_471440: native, nfs3hp::controls::bumpJolt */

/* sub_4714f0: native, nfs3hp::controls::gearChangeKick */

/* sub_471650: native, nfs3hp::controls::steeringAmount */

/* sub_4716e0: native, nfs3hp::controls::playSteeringSpring */

/* sub_471860: native, nfs3hp::controls::playSlideSpring */

/* sub_4719f0: native, nfs3hp::controls::playRoadRumble */

/* sub_471af0: native, nfs3hp::controls::playEngineRumble */

/* sub_471dc0: native, nfs3hp::controls::isForceCar */

/* sub_471e10: native, nfs3hp::controls::raceForcesFrame */

/* sub_471fb0: native, nfs3hp::controls::allActuatorsOff */

/* sub_471fd0: native, nfs3hp::controls::playJolt */

/* sub_4720a0: native, nfs3hp::controls::stopEngineRumble */

/* sub_4720e0: native, nfs3hp::controls::stopSteeringSpring */

/* sub_472110: native, nfs3hp::controls::carSpeedMph */

/* sub_4721f0: native, nfs3hp::frontend::nameValid */

/* sub_472230: native, nfs3hp::load::languageStub */

/* sub_472250: native, nfs3hp::load::setLanguage */

/* sub_472300: native, nfs3hp::load::setCameraViews */

/* sub_472360: native, nfs3hp::port::fields::refreshCustomColours */

/* sub_4723f0: native, nfs3hp::load::defaultConfig */

/* sub_472820: native, nfs3hp::load::saveConfig */

/* sub_4728e0: native, nfs3hp::load::fileWriteTime */

/* sub_472940: native, nfs3hp::load::fileOlder */

/* sub_472980: native, nfs3hp::load::loadConfig */

/* sub_472b60: native, nfs3hp::controls::loadControllers */

/* sub_472d10: native, nfs3hp::render::detailSettings */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_472f20(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00472f20  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00472f21  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00472f22  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00472f23  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00472f24  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00472f25  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00472f26  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00472f28  81ec34010000           -sub esp, 0x134
    (cpu.esp) -= x86::reg32(x86::sreg32(308 /*0x134*/));
    // 00472f2e  8d7df8                 -lea edi, [ebp - 8]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00472f31  beb8214700             -mov esi, 0x4721b8
    cpu.esi = 4661688 /*0x4721b8*/;
    // 00472f36  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
    app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
    if (cpu.flags.df)
    {
        cpu.edi -= 4;
        cpu.esi -= 4;
    }
    else
    {
        cpu.edi += 4;
        cpu.esi += 4;
    }
    // 00472f37  66a5                   -movsw word ptr es:[edi], word ptr [esi]
    app->getMemory<x86::reg16>(cpu.ees + cpu.edi) = app->getMemory<x86::reg16>(cpu.esi);
    if (cpu.flags.df)
    {
        cpu.edi -= 2;
        cpu.esi -= 2;
    }
    else
    {
        cpu.edi += 2;
        cpu.esi += 2;
    }
    // 00472f39  a4                     -movsb byte ptr es:[edi], byte ptr [esi]
    app->getMemory<x86::reg8>(cpu.ees + cpu.edi) = app->getMemory<x86::reg8>(cpu.esi);
    if (cpu.flags.df)
    {
        cpu.edi -= 1;
        cpu.esi -= 1;
    }
    else
    {
        cpu.edi += 1;
        cpu.esi += 1;
    }
    // 00472f3a  e841800600             -call 0x4daf80
    cpu.esp -= 4;
    nfs3hp::port::draw2d::startScreens(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00472f3f  a1503a7a00             -mov eax, dword ptr [0x7a3a50]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8010320) /* 0x7a3a50 */);
    // 00472f44  8a4428f8               -mov al, byte ptr [eax + ebp - 8]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(-8) /* -0x8 */ + cpu.ebp * 1);
    // 00472f48  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 00472f4d  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00472f4e  68b42d7a00             -push 0x7a2db4
    app->getMemory<x86::reg32>(cpu.esp-4) = 8007092 /*0x7a2db4*/;
    cpu.esp -= 4;
    // 00472f53  6830b05300             -push 0x53b030
    app->getMemory<x86::reg32>(cpu.esp-4) = 5484592 /*0x53b030*/;
    cpu.esp -= 4;
    // 00472f58  8d85ccfeffff           -lea eax, [ebp - 0x134]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-308) /* -0x134 */);
    // 00472f5e  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00472f5f  e82cc70600             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00472f64  83c410                 -add esp, 0x10
    (cpu.esp) += x86::reg32(x86::sreg32(16 /*0x10*/));
    // 00472f67  ba10000000             -mov edx, 0x10
    cpu.edx = 16 /*0x10*/;
    // 00472f6c  8d85ccfeffff           -lea eax, [ebp - 0x134]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-308) /* -0x134 */);
    // 00472f72  e879270200             -call 0x4956f0
    cpu.esp -= 4;
    nfs3hp::frontend::loadQfs(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00472f77  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00472f79  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00472f7b  7435                   -je 0x472fb2
    if (cpu.flags.zf)
    {
        goto L_0x00472fb2;
    }
    // 00472f7d  ba40b05300             -mov edx, 0x53b040
    cpu.edx = 5484608 /*0x53b040*/;
    // 00472f82  e859c20700             -call 0x4ef1e0
    cpu.esp -= 4;
    sub_4ef1e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00472f87  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00472f89  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00472f8b  e800340600             -call 0x4d6390
    cpu.esp -= 4;
    nfs3hp::port::shapes::artShapeTextures(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00472f90  a338768b00             -mov dword ptr [0x8b7638], eax
    app->getMemory<x86::reg32>(x86::reg32(9139768) /* 0x8b7638 */) = cpu.eax;
    // 00472f95  e886610600             -call 0x4d9120
    cpu.esp -= 4;
    nfs3hp::graphics::takeWindowAndClear(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00472f9a  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00472f9c  a138768b00             -mov eax, dword ptr [0x8b7638]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(9139768) /* 0x8b7638 */);
    // 00472fa1  e8ea490600             -call 0x4d7990
    cpu.esp -= 4;
    nfs3hp::port::shapes::drawShape(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00472fa6  e895610600             -call 0x4d9140
    cpu.esp -= 4;
    nfs3hp::graphics::flipAndWaitFrames(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00472fab  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00472fad  e8dee80600             -call 0x4e1890
    cpu.esp -= 4;
    nfs3hp::ea::purgememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00472fb2:
    // 00472fb2  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00472fb4  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00472fb5  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00472fb6  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00472fb7  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00472fb8  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00472fb9  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00472fba  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_472fc0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00472fc0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00472fc1  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00472fc2  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00472fc4  833d54bb6f0000         +cmp dword ptr [0x6fbb54], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7322452) /* 0x6fbb54 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00472fcb  7505                   -jne 0x472fd2
    if (!cpu.flags.zf)
    {
        goto L_0x00472fd2;
    }
    // 00472fcd  e84effffff             -call 0x472f20
    cpu.esp -= 4;
    sub_472f20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00472fd2:
    // 00472fd2  e879d60700             -call 0x4f0650
    cpu.esp -= 4;
    sub_4f0650(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00472fd7  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00472fd9  e862e9fdff             -call 0x451940
    cpu.esp -= 4;
    sub_451940(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00472fde  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x00472fe0:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00472fe0  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00472fe5  e876e9fdff             -call 0x451960
    cpu.esp -= 4;
    nfs3hp::port::text::menuKey(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00472fea  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00472fec  751c                   -jne 0x47300a
    if (!cpu.flags.zf)
    {
        goto L_0x0047300a;
    }
    // 00472fee  81fa80010000           +cmp edx, 0x180
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(384 /*0x180*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00472ff4  7d14                   -jge 0x47300a
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0047300a;
    }
    // 00472ff6  e865680500             -call 0x4c9860
    cpu.esp -= 4;
    nfs3hp::port::descriptionService(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00472ffb  e850d60700             -call 0x4f0650
    cpu.esp -= 4;
    sub_4f0650(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473000  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00473002  81fa80010000           +cmp edx, 0x180
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(384 /*0x180*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00473008  7ed6                   -jle 0x472fe0
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00472fe0;
    }
L_0x0047300a:
    // 0047300a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047300b  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047300c  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_473010(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00473010  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00473011  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00473013  833d54bb6f0000         -cmp dword ptr [0x6fbb54], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7322452) /* 0x6fbb54 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0047301a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047301b  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_473020(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00473020  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00473021  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00473022  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00473024  e897ebfbff             -call 0x431bc0
    cpu.esp -= 4;
    nfs3hp::collide::statesForScreens(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473029  b803000000             -mov eax, 3
    cpu.eax = 3 /*0x3*/;
    // 0047302e  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00473030  e8cbe8fbff             -call 0x431900
    cpu.esp -= 4;
    nfs3hp::graphics::setRenderState(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473035  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0047303a  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0047303c  e87f2c0600             -call 0x4d5cc0
    cpu.esp -= 4;
    nfs3hp::graphics::beginDrawing(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473041  a18c435600             -mov eax, dword ptr [0x56438c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5653388) /* 0x56438c */);
    // 00473046  ba10000000             -mov edx, 0x10
    cpu.edx = 16 /*0x10*/;
    // 0047304b  a308906700             -mov dword ptr [0x679008], eax
    app->getMemory<x86::reg32>(x86::reg32(6787080) /* 0x679008 */) = cpu.eax;
    // 00473050  89158c435600           -mov dword ptr [0x56438c], edx
    app->getMemory<x86::reg32>(x86::reg32(5653388) /* 0x56438c */) = cpu.edx;
    // 00473056  e8d5d10700             -call 0x4f0230
    cpu.esp -= 4;
    sub_4f0230(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047305b  a190435600             -mov eax, dword ptr [0x564390]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5653392) /* 0x564390 */);
    // 00473060  a304906700             -mov dword ptr [0x679004], eax
    app->getMemory<x86::reg32>(x86::reg32(6787076) /* 0x679004 */) = cpu.eax;
    // 00473065  800d9043560004         -or byte ptr [0x564390], 4
    app->getMemory<x86::reg8>(x86::reg32(5653392) /* 0x564390 */) |= x86::reg8(x86::sreg8(4 /*0x4*/));
    // 0047306c  e86f06faff             -call 0x4136e0
    cpu.esp -= 4;
    nfs3hp::port::raceBankLoad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473071  e80a7f0600             -call 0x4daf80
    cpu.esp -= 4;
    nfs3hp::port::draw2d::startScreens(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473076  e8c5900200             -call 0x49c140
    cpu.esp -= 4;
    sub_49c140(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047307b  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047307c  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047307d  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_473080(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00473080  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00473081  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00473082  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00473084  e847d9f9ff             -call 0x4109d0
    cpu.esp -= 4;
    nfs3hp::port::musicStop(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473089  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0047308b  e890daf9ff             -call 0x410b20
    cpu.esp -= 4;
    nfs3hp::port::banksRemove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473090  891534b05500           -mov dword ptr [0x55b034], edx
    app->getMemory<x86::reg32>(x86::reg32(5615668) /* 0x55b034 */) = cpu.edx;
    // 00473096  e805920200             -call 0x49c2a0
    cpu.esp -= 4;
    sub_49c2a0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047309b  a108906700             -mov eax, dword ptr [0x679008]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6787080) /* 0x679008 */);
    // 004730a0  a38c435600             -mov dword ptr [0x56438c], eax
    app->getMemory<x86::reg32>(x86::reg32(5653388) /* 0x56438c */) = cpu.eax;
    // 004730a5  a104906700             -mov eax, dword ptr [0x679004]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6787076) /* 0x679004 */);
    // 004730aa  a390435600             -mov dword ptr [0x564390], eax
    app->getMemory<x86::reg32>(x86::reg32(5653392) /* 0x564390 */) = cpu.eax;
    // 004730af  e8acf2ffff             -call 0x472360
    cpu.esp -= 4;
    nfs3hp::port::fields::refreshCustomColours(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004730b4  e867f7ffff             -call 0x472820
    cpu.esp -= 4;
    nfs3hp::load::saveConfig(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004730b9  e8822b0600             -call 0x4d5c40
    cpu.esp -= 4;
    nfs3hp::port::shapes::resetTextures(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004730be  e84d2c0600             -call 0x4d5d10
    cpu.esp -= 4;
    sub_4d5d10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004730c3  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004730c4  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004730c5  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x54 0x22 0x00 */
void Application::sub_4730d0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004730d0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004730d1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004730d2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004730d3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004730d4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004730d5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004730d6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004730d8  83ec24                 -sub esp, 0x24
    (cpu.esp) -= x86::reg32(x86::sreg32(36 /*0x24*/));
    // 004730db  8d7ddc                 -lea edi, [ebp - 0x24]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 004730de  bec0214700             -mov esi, 0x4721c0
    cpu.esi = 4661696 /*0x4721c0*/;
    // 004730e3  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
    app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
    if (cpu.flags.df)
    {
        cpu.edi -= 4;
        cpu.esi -= 4;
    }
    else
    {
        cpu.edi += 4;
        cpu.esi += 4;
    }
    // 004730e4  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
    app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
    if (cpu.flags.df)
    {
        cpu.edi -= 4;
        cpu.esi -= 4;
    }
    else
    {
        cpu.edi += 4;
        cpu.esi += 4;
    }
    // 004730e5  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
    app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
    if (cpu.flags.df)
    {
        cpu.edi -= 4;
        cpu.esi -= 4;
    }
    else
    {
        cpu.edi += 4;
        cpu.esi += 4;
    }
    // 004730e6  8d7de8                 -lea edi, [ebp - 0x18]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004730e9  becc214700             -mov esi, 0x4721cc
    cpu.esi = 4661708 /*0x4721cc*/;
    // 004730ee  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
    app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
    if (cpu.flags.df)
    {
        cpu.edi -= 4;
        cpu.esi -= 4;
    }
    else
    {
        cpu.edi += 4;
        cpu.esi += 4;
    }
    // 004730ef  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
    app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
    if (cpu.flags.df)
    {
        cpu.edi -= 4;
        cpu.esi -= 4;
    }
    else
    {
        cpu.edi += 4;
        cpu.esi += 4;
    }
    // 004730f0  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
    app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
    if (cpu.flags.df)
    {
        cpu.edi -= 4;
        cpu.esi -= 4;
    }
    else
    {
        cpu.edi += 4;
        cpu.esi += 4;
    }
    // 004730f1  8d7df4                 -lea edi, [ebp - 0xc]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004730f4  bed8214700             -mov esi, 0x4721d8
    cpu.esi = 4661720 /*0x4721d8*/;
    // 004730f9  bab0bf4900             -mov edx, 0x49bfb0
    cpu.edx = 4833200 /*0x49bfb0*/;
    // 004730fe  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00473100  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
    app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
    if (cpu.flags.df)
    {
        cpu.edi -= 4;
        cpu.esi -= 4;
    }
    else
    {
        cpu.edi += 4;
        cpu.esi += 4;
    }
    // 00473101  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
    app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
    if (cpu.flags.df)
    {
        cpu.edi -= 4;
        cpu.esi -= 4;
    }
    else
    {
        cpu.edi += 4;
        cpu.esi += 4;
    }
    // 00473102  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
    app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
    if (cpu.flags.df)
    {
        cpu.edi -= 4;
        cpu.esi -= 4;
    }
    else
    {
        cpu.edi += 4;
        cpu.esi += 4;
    }
    // 00473103  e88882fcff             -call 0x43b390
    cpu.esp -= 4;
    sub_43b390(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473108  89156c445600           -mov dword ptr [0x56446c], edx
    app->getMemory<x86::reg32>(x86::reg32(5653612) /* 0x56446c */) = cpu.edx;
    // 0047310e  891dfcd46f00           -mov dword ptr [0x6fd4fc], ebx
    app->getMemory<x86::reg32>(x86::reg32(7329020) /* 0x6fd4fc */) = cpu.ebx;
    // 00473114  891d00d56f00           -mov dword ptr [0x6fd500], ebx
    app->getMemory<x86::reg32>(x86::reg32(7329024) /* 0x6fd500 */) = cpu.ebx;
    // 0047311a  891d28945500           -mov dword ptr [0x559428], ebx
    app->getMemory<x86::reg32>(x86::reg32(5608488) /* 0x559428 */) = cpu.ebx;
    // 00473120  891d2c945500           -mov dword ptr [0x55942c], ebx
    app->getMemory<x86::reg32>(x86::reg32(5608492) /* 0x55942c */) = cpu.ebx;
    // 00473126  e8e5feffff             -call 0x473010
    cpu.esp -= 4;
    sub_473010(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047312b  833d54bb6f0000         +cmp dword ptr [0x6fbb54], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7322452) /* 0x6fbb54 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00473132  7535                   -jne 0x473169
    if (!cpu.flags.zf)
    {
        goto L_0x00473169;
    }
    // 00473134  833d8ce8550000         +cmp dword ptr [0x55e88c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5630092) /* 0x55e88c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0047313b  752c                   -jne 0x473169
    if (!cpu.flags.zf)
    {
        goto L_0x00473169;
    }
    // 0047313d  f6052deb550008         +test byte ptr [0x55eb2d], 8
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(x86::reg32(5630765) /* 0x55eb2d */) & 8 /*0x8*/));
    // 00473144  7523                   -jne 0x473169
    if (!cpu.flags.zf)
    {
        goto L_0x00473169;
    }
    // 00473146  833d3cb0550000         +cmp dword ptr [0x55b03c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5615676) /* 0x55b03c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0047314d  741a                   -je 0x473169
    if (cpu.flags.zf)
    {
        goto L_0x00473169;
    }
    // 0047314f  b848b05300             -mov eax, 0x53b048
    cpu.eax = 5484616 /*0x53b048*/;
    // 00473154  bf01000000             -mov edi, 1
    cpu.edi = 1 /*0x1*/;
    // 00473159  e8822f0200             -call 0x4960e0
    cpu.esp -= 4;
    nfs3hp::frontend::playMovie(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047315e  e87d300200             -call 0x4961e0
    cpu.esp -= 4;
    nfs3hp::frontend::resetAttractTimer(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473163  893d4ce55500           -mov dword ptr [0x55e54c], edi
    app->getMemory<x86::reg32>(x86::reg32(5629260) /* 0x55e54c */) = cpu.edi;
L_0x00473169:
    // 00473169  e8b2feffff             -call 0x473020
    cpu.esp -= 4;
    sub_473020(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047316e  833d3cb0550000         +cmp dword ptr [0x55b03c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5615676) /* 0x55b03c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00473175  7423                   -je 0x47319a
    if (cpu.flags.zf)
    {
        goto L_0x0047319a;
    }
    // 00473177  8b158ce85500           -mov edx, dword ptr [0x55e88c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5630092) /* 0x55e88c */);
    // 0047317d  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0047317f  7519                   -jne 0x47319a
    if (!cpu.flags.zf)
    {
        goto L_0x0047319a;
    }
    // 00473181  e83afeffff             -call 0x472fc0
    cpu.esp -= 4;
    sub_472fc0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473186  833d0c90670000         +cmp dword ptr [0x67900c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(6787084) /* 0x67900c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0047318d  740b                   -je 0x47319a
    if (cpu.flags.zf)
    {
        goto L_0x0047319a;
    }
    // 0047318f  89150c906700           -mov dword ptr [0x67900c], edx
    app->getMemory<x86::reg32>(x86::reg32(6787084) /* 0x67900c */) = cpu.edx;
    // 00473195  e876fbffff             -call 0x472d10
    cpu.esp -= 4;
    nfs3hp::render::detailSettings(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    if (firstSettings(app)) /* port: the phone's settings over the new player's */
    {
        const x86::reg32 kept = cpu.eax;
        cpu.esp -= 4;
        nfs3hp::frontend::noteDigitalControls(app, cpu.sync()); /* the game takes up the new bindings */
        cpu.reload();
        if (cpu.terminate) return;
        cpu.esp -= 4;
        nfs3hp::load::saveConfig(app, cpu.sync()); /* and saves them, as it does leaving the front end */
        cpu.reload();
        if (cpu.terminate) return;
        cpu.eax = kept;
    }
L_0x0047319a:
    // 0047319a  e8a1d00700             -call 0x4f0240
    cpu.esp -= 4;
    nfs3hp::ea::flushKeys(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047319f  ba50000000             -mov edx, 0x50
    cpu.edx = 80 /*0x50*/;
    // 004731a4  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 004731a6  b8bcd36f00             -mov eax, 0x6fd3bc
    cpu.eax = 7328700 /*0x6fd3bc*/;
    // 004731ab  893da0d36f00           -mov dword ptr [0x6fd3a0], edi
    app->getMemory<x86::reg32>(x86::reg32(7328672) /* 0x6fd3a0 */) = cpu.edi;
    // 004731b1  893dacd36f00           -mov dword ptr [0x6fd3ac], edi
    app->getMemory<x86::reg32>(x86::reg32(7328684) /* 0x6fd3ac */) = cpu.edi;
    // 004731b7  e850d50600             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004731bc  ba62000000             -mov edx, 0x62
    cpu.edx = 98 /*0x62*/;
    // 004731c1  b80cd46f00             -mov eax, 0x6fd40c
    cpu.eax = 7328780 /*0x6fd40c*/;
    // 004731c6  e841d50600             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004731cb  e8304a0000             -call 0x477c00
    cpu.esp -= 4;
    sub_477c00(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004731d0  833d3cb0550000         +cmp dword ptr [0x55b03c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5615676) /* 0x55b03c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004731d7  753f                   -jne 0x473218
    if (!cpu.flags.zf)
    {
        goto L_0x00473218;
    }
    // 004731d9  a1b0d36f00             -mov eax, dword ptr [0x6fd3b0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
    // 004731de  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004731e0  760d                   -jbe 0x4731ef
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004731ef;
    }
    // 004731e2  83f801                 +cmp eax, 1
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004731e5  7510                   -jne 0x4731f7
    if (!cpu.flags.zf)
    {
        goto L_0x004731f7;
    }
    // 004731e7  8d55e8                 -lea edx, [ebp - 0x18]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004731ea  e978000000             -jmp 0x473267
    goto L_0x00473267;
L_0x004731ef:
    // 004731ef  8d55dc                 -lea edx, [ebp - 0x24]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 004731f2  e970000000             -jmp 0x473267
    goto L_0x00473267;
L_0x004731f7:
    // 004731f7  833de0227a0000         +cmp dword ptr [0x7a22e0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8004320) /* 0x7a22e0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004731fe  740a                   -je 0x47320a
    if (cpu.flags.zf)
    {
        goto L_0x0047320a;
    }
    // 00473200  66833ddc227a0000       +cmp word ptr [0x7a22dc], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(x86::reg32(8004316) /* 0x7a22dc */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 00473208  7404                   -je 0x47320e
    if (cpu.flags.zf)
    {
        goto L_0x0047320e;
    }
L_0x0047320a:
    // 0047320a  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 0047320c  eb03                   -jmp 0x473211
    goto L_0x00473211;
L_0x0047320e:
    // 0047320e  8d55f4                 -lea edx, [ebp - 0xc]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
L_0x00473211:
    // 00473211  e8cabb0200             -call 0x49ede0
    cpu.esp -= 4;
    sub_49ede0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473216  eb4f                   -jmp 0x473267
    goto L_0x00473267;
L_0x00473218:
    // 00473218  bfa01f7a00             -mov edi, 0x7a1fa0
    cpu.edi = 8003488 /*0x7a1fa0*/;
    // 0047321d  29c9                   -sub ecx, ecx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 0047321f  49                     -dec ecx
    (cpu.ecx)--;
    // 00473220  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 00473222  f2ae                   -repne scasb al, byte ptr es:[edi]
    while (cpu.ecx)
    {
        {
            x86::reg8 tmp1 = cpu.al;
            x86::reg8 tmp2 = x86::reg8(x86::sreg8(app->getMemory<x86::reg8>(cpu.ees + cpu.edi)));
            x86::reg8 result = tmp1 - tmp2;
            cpu.flags.cf = tmp1 < tmp2;
            cpu.flags.of = 1 & (tmp1 >> 7);
            cpu.flags.of ^= 1 & (result >> 7);
            cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
            cpu.set_szp(result);
        }
        if (cpu.flags.df)
        {
            cpu.edi -= 1;
        }
        else
        {
            cpu.edi += 1;
        }
        --cpu.ecx;
        if (cpu.flags.zf)
            break;
    }
    // 00473224  f7d1                   -not ecx
    cpu.ecx = ~cpu.ecx;
    // 00473226  49                     -dec ecx
    (cpu.ecx)--;
    // 00473227  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 00473229  743a                   -je 0x473265
    if (cpu.flags.zf)
    {
        goto L_0x00473265;
    }
    // 0047322b  8b35fc1f7a00           -mov esi, dword ptr [0x7a1ffc]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(8003580) /* 0x7a1ffc */);
    // 00473231  8d55f4                 -lea edx, [ebp - 0xc]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00473234  83fe01                 +cmp esi, 1
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00473237  7e10                   -jle 0x473249
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00473249;
    }
    // 00473239  e8e2af0200             -call 0x49e220
    cpu.esp -= 4;
    sub_49e220(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047323e  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00473240  7515                   -jne 0x473257
    if (!cpu.flags.zf)
    {
        goto L_0x00473257;
    }
    // 00473242  bb0a000000             -mov ebx, 0xa
    cpu.ebx = 10 /*0xa*/;
    // 00473247  eb0e                   -jmp 0x473257
    goto L_0x00473257;
L_0x00473249:
    // 00473249  e892b00200             -call 0x49e2e0
    cpu.esp -= 4;
    sub_49e2e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047324e  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00473250  7505                   -jne 0x473257
    if (!cpu.flags.zf)
    {
        goto L_0x00473257;
    }
    // 00473252  bb0a000000             -mov ebx, 0xa
    cpu.ebx = 10 /*0xa*/;
L_0x00473257:
    // 00473257  83fb0a                 +cmp ebx, 0xa
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(10 /*0xa*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0047325a  750b                   -jne 0x473267
    if (!cpu.flags.zf)
    {
        goto L_0x00473267;
    }
    // 0047325c  e81ffeffff             -call 0x473080
    cpu.esp -= 4;
    sub_473080(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473261  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 00473263  eb36                   -jmp 0x47329b
    goto L_0x0047329b;
L_0x00473265:
    // 00473265  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x00473267:
    // 00473267  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 00473269  893d3cb05500           -mov dword ptr [0x55b03c], edi
    app->getMemory<x86::reg32>(x86::reg32(5615676) /* 0x55b03c */) = cpu.edi;
    // 0047326f  e85c240400             -call 0x4b56d0
    cpu.esp -= 4;
    sub_4b56d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473274  a3b4d36f00             -mov dword ptr [0x6fd3b4], eax
    app->getMemory<x86::reg32>(x86::reg32(7328692) /* 0x6fd3b4 */) = cpu.eax;
    // 00473279  a118bc6f00             -mov eax, dword ptr [0x6fbc18]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7322648) /* 0x6fbc18 */);
    // 0047327e  e8fdbb0400             -call 0x4bee80
    cpu.esp -= 4;
    nfs3hp::graphics::findScreenMode(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473283  e81851fdff             -call 0x4483a0
    cpu.esp -= 4;
    sub_4483a0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473288  b8b8af5300             -mov eax, 0x53afb8
    cpu.eax = 5484472 /*0x53afb8*/;
    // 0047328d  e89e7ffdff             -call 0x44b230
    cpu.esp -= 4;
    nfs3hp::port::loop::menuLoop(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473292  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00473294  e8e7fdffff             -call 0x473080
    cpu.esp -= 4;
    sub_473080(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473299  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
L_0x0047329b:
    // 0047329b  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0047329d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047329e  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047329f  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004732a0  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004732a1  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004732a2  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004732a3  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_4732b0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004732b0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004732b1  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004732b2  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004732b3  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004732b4  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004732b6  e865fdffff             -call 0x473020
    cpu.esp -= 4;
    sub_473020(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004732bb  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 004732c0  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004732c2  a1b8d36f00             -mov eax, dword ptr [0x6fd3b8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328696) /* 0x6fd3b8 */);
    // 004732c7  8915a0d36f00           -mov dword ptr [0x6fd3a0], edx
    app->getMemory<x86::reg32>(x86::reg32(7328672) /* 0x6fd3a0 */) = cpu.edx;
    // 004732cd  890d30768b00           -mov dword ptr [0x8b7630], ecx
    app->getMemory<x86::reg32>(x86::reg32(9139760) /* 0x8b7630 */) = cpu.ecx;
    // 004732d3  39c8                   +cmp eax, ecx
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ecx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004732d5  7229                   -jb 0x473300
    if (cpu.flags.cf)
    {
        goto L_0x00473300;
    }
    // 004732d7  7607                   -jbe 0x4732e0
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004732e0;
    }
    // 004732d9  83f802                 +cmp eax, 2
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004732dc  741e                   -je 0x4732fc
    if (cpu.flags.zf)
    {
        goto L_0x004732fc;
    }
    // 004732de  eb20                   -jmp 0x473300
    goto L_0x00473300;
L_0x004732e0:
    // 004732e0  833d70d46f0000         +cmp dword ptr [0x6fd470], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328880) /* 0x6fd470 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004732e7  7507                   -jne 0x4732f0
    if (!cpu.flags.zf)
    {
        goto L_0x004732f0;
    }
    // 004732e9  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 004732ee  eb05                   -jmp 0x4732f5
    goto L_0x004732f5;
L_0x004732f0:
    // 004732f0  b803000000             -mov eax, 3
    cpu.eax = 3 /*0x3*/;
L_0x004732f5:
    // 004732f5  e896650500             -call 0x4c9890
    cpu.esp -= 4;
    nfs3hp::port::raceVoiceLoad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004732fa  eb0e                   -jmp 0x47330a
    goto L_0x0047330a;
L_0x004732fc:
    // 004732fc  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004732fe  eb05                   -jmp 0x473305
    goto L_0x00473305;
L_0x00473300:
    // 00473300  b804000000             -mov eax, 4
    cpu.eax = 4 /*0x4*/;
L_0x00473305:
    // 00473305  e886650500             -call 0x4c9890
    cpu.esp -= 4;
    nfs3hp::port::raceVoiceLoad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0047330a:
    // 0047330a  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0047330c  e87f650500             -call 0x4c9890
    cpu.esp -= 4;
    nfs3hp::port::raceVoiceLoad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473311  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00473313  b854b05300             -mov eax, 0x53b054
    cpu.eax = 5484628 /*0x53b054*/;
    // 00473318  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 0047331a  e8117ffdff             -call 0x44b230
    cpu.esp -= 4;
    nfs3hp::port::loop::menuLoop(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047331f  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00473321  e8da660500             -call 0x4c9a00
    cpu.esp -= 4;
    nfs3hp::port::raceVoiceFree(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473326  e8a5b4fdff             -call 0x44e7d0
    cpu.esp -= 4;
    sub_44e7d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047332b  893530768b00           -mov dword ptr [0x8b7630], esi
    app->getMemory<x86::reg32>(x86::reg32(9139760) /* 0x8b7630 */) = cpu.esi;
    // 00473331  e84afdffff             -call 0x473080
    cpu.esp -= 4;
    sub_473080(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473336  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00473338  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473339  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047333a  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047333b  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047333c  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_473340(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00473340  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00473341  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00473342  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00473344  ba00040000             -mov edx, 0x400
    cpu.edx = 1024 /*0x400*/;
    // 00473349  b8008c6700             -mov eax, 0x678c00
    cpu.eax = 6786048 /*0x678c00*/;
    // 0047334e  e8b9d30600             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473353  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473354  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473355  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x54 0x22 0x00 */
void Application::sub_473360(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00473360  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00473361  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00473362  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00473363  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00473364  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00473366  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00473369  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0047336b  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0047336d  e8ee180000             -call 0x474c60
    cpu.esp -= 4;
    sub_474c60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473372  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00473374  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00473377  f7fb                   -idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 00473379  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0047337b  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
L_0x0047337e:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0047337e  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00473380  c1e008                 -shl eax, 8
    cpu.eax <<= 8 /*0x8*/ % 32;
    // 00473383  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00473385  0fb6b8008c6700         -movzx edi, byte ptr [eax + 0x678c00]
    cpu.edi = x86::reg32(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(6786048) /* 0x678c00 */));
    // 0047338c  39f9                   +cmp ecx, edi
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edi));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0047338e  7d10                   -jge 0x4733a0
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004733a0;
    }
    // 00473390  42                     -inc edx
    (cpu.edx)++;
    // 00473391  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00473393  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00473396  f7fb                   -idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 00473398  3b55fc                 +cmp edx, dword ptr [ebp - 4]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0047339b  75e1                   -jne 0x47337e
    if (!cpu.flags.zf)
    {
        goto L_0x0047337e;
    }
    // 0047339d  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0047339e  ebde                   -jmp 0x47337e
    goto L_0x0047337e;
L_0x004733a0:
    // 004733a0  fe80008c6700           -inc byte ptr [eax + 0x678c00]
    (app->getMemory<x86::reg8>(cpu.eax + x86::reg32(6786048) /* 0x678c00 */))++;
    // 004733a6  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004733a8  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004733aa  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004733ab  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004733ac  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004733ad  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004733ae  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x90 */
void Application::sub_4733b0(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004733b0  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004733b1  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004733b3  c1e008                 -shl eax, 8
    cpu.eax <<= 8 /*0x8*/ % 32;
    // 004733b6  fe8402008c6700         -inc byte ptr [edx + eax + 0x678c00]
    (app->getMemory<x86::reg8>(cpu.edx + x86::reg32(6786048) /* 0x678c00 */ + cpu.eax * 1))++;
    // 004733bd  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004733be  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4733c0: native, nfs3hp::port::view::colourNearest */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_4734d0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004734d0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004734d1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004734d2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004734d3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004734d4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004734d5  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004734d7  8b15d4d46f00           -mov edx, dword ptr [0x6fd4d4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7328980) /* 0x6fd4d4 */);
    // 004734dd  83fa04                 +cmp edx, 4
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004734e0  7d54                   -jge 0x473536
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00473536;
    }
    // 004734e2  8b1c95f8555500         -mov ebx, dword ptr [edx*4 + 0x5555f8]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5592568) /* 0x5555f8 */ + cpu.edx * 4);
    // 004734e9  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004734eb  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004734f0  e86bfeffff             -call 0x473360
    cpu.esp -= 4;
    sub_473360(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004734f5  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004734f7  8b1dd4d46f00           -mov ebx, dword ptr [0x6fd4d4]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7328980) /* 0x6fd4d4 */);
    // 004734fd  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00473500  8b349df8555500         -mov esi, dword ptr [ebx*4 + 0x5555f8]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(5592568) /* 0x5555f8 */ + cpu.ebx * 4);
    // 00473507  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00473509  f7fe                   -idiv esi
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.esi);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 0047350b  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0047350d  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 0047350f  e89c71fcff             -call 0x43a6b0
    cpu.esp -= 4;
    sub_43a6b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473514  8b35d4d46f00           -mov esi, dword ptr [0x6fd4d4]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7328980) /* 0x6fd4d4 */);
    // 0047351a  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0047351c  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 0047351e  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00473520  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00473523  f73cb5f8555500         +idiv dword ptr [esi*4 + 0x5555f8]
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5592568) /* 0x5555f8 */ + cpu.esi * 4));
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 0047352a  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0047352f  e87cfeffff             -call 0x4733b0
    cpu.esp -= 4;
    sub_4733b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473534  eb03                   -jmp 0x473539
    goto L_0x00473539;
L_0x00473536:
    // 00473536  8d5afc                 -lea ebx, [edx - 4]
    cpu.ebx = x86::reg32(cpu.edx + x86::reg32(-4) /* -0x4 */);
L_0x00473539:
    // 00473539  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0047353b  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047353c  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047353d  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047353e  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047353f  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473540  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_473550(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00473550  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00473551  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00473552  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00473553  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00473555  bb07000000             -mov ebx, 7
    cpu.ebx = 7 /*0x7*/;
    // 0047355a  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0047355c  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 00473561  e8fafdffff             -call 0x473360
    cpu.esp -= 4;
    sub_473360(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473566  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473567  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473568  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473569  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_473570(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00473570  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00473571  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00473572  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00473573  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00473575  bb08000000             -mov ebx, 8
    cpu.ebx = 8 /*0x8*/;
    // 0047357a  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0047357c  b803000000             -mov eax, 3
    cpu.eax = 3 /*0x3*/;
    // 00473581  e8dafdffff             -call 0x473360
    cpu.esp -= 4;
    sub_473360(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473586  83c007                 -add eax, 7
    (cpu.eax) += x86::reg32(x86::sreg32(7 /*0x7*/));
    // 00473589  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047358a  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047358b  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047358c  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_4735c0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 004735c0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004735c1  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004735c2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004735c3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004735c5  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004735c7  8b1dacd46f00           -mov ebx, dword ptr [0x6fd4ac]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7328940) /* 0x6fd4ac */);
    // 004735cd  b832000000             -mov eax, 0x32
    cpu.eax = 50 /*0x32*/;
    // 004735d2  83fb0a                 +cmp ebx, 0xa
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(10 /*0xa*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004735d5  0f87d5000000           -ja 0x4736b0
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x004736b0;
    }
    // 004735db  ff249d90354700         -jmp dword ptr [ebx*4 + 0x473590]
    cpu.ip = app->getMemory<x86::reg32>(4666768 + cpu.ebx * 4); goto dynamic_jump;
  case 0x004735e2:
    // 004735e2  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004735e4  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004735e6  8a1d40b05500           -mov bl, byte ptr [0x55b040]
    cpu.bl = app->getMemory<x86::reg8>(x86::reg32(5615680) /* 0x55b040 */);
    // 004735ec  e86ffdffff             -call 0x473360
    cpu.esp -= 4;
    sub_473360(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004735f1  8a8041b05500           -mov al, byte ptr [eax + 0x55b041]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5615681) /* 0x55b041 */);
    // 004735f7  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 004735fc  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004735fd  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004735fe  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004735ff  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x00473600:
    // 00473600  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00473602  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00473604  8a1d4cb05500           -mov bl, byte ptr [0x55b04c]
    cpu.bl = app->getMemory<x86::reg8>(x86::reg32(5615692) /* 0x55b04c */);
    // 0047360a  e851fdffff             -call 0x473360
    cpu.esp -= 4;
    sub_473360(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047360f  8a804db05500           -mov al, byte ptr [eax + 0x55b04d]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5615693) /* 0x55b04d */);
    // 00473615  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 0047361a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047361b  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047361c  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047361d  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x0047361e:
    // 0047361e  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00473620  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00473622  8a1d58b05500           -mov bl, byte ptr [0x55b058]
    cpu.bl = app->getMemory<x86::reg8>(x86::reg32(5615704) /* 0x55b058 */);
    // 00473628  e833fdffff             -call 0x473360
    cpu.esp -= 4;
    sub_473360(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047362d  8a8059b05500           -mov al, byte ptr [eax + 0x55b059]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5615705) /* 0x55b059 */);
    // 00473633  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 00473638  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473639  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047363a  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047363b  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x0047363c:
    // 0047363c  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0047363e  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00473640  8a1d64b05500           -mov bl, byte ptr [0x55b064]
    cpu.bl = app->getMemory<x86::reg8>(x86::reg32(5615716) /* 0x55b064 */);
    // 00473646  e815fdffff             -call 0x473360
    cpu.esp -= 4;
    sub_473360(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047364b  8a8065b05500           -mov al, byte ptr [eax + 0x55b065]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5615717) /* 0x55b065 */);
    // 00473651  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 00473656  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473657  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473658  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473659  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x0047365a:
    // 0047365a  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0047365c  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0047365e  8a1d6fb05500           -mov bl, byte ptr [0x55b06f]
    cpu.bl = app->getMemory<x86::reg8>(x86::reg32(5615727) /* 0x55b06f */);
    // 00473664  e8f7fcffff             -call 0x473360
    cpu.esp -= 4;
    sub_473360(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473669  8a8070b05500           -mov al, byte ptr [eax + 0x55b070]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5615728) /* 0x55b070 */);
    // 0047366f  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 00473674  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473675  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473676  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473677  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x00473678:
    // 00473678  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0047367a  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0047367c  8a1d78b05500           -mov bl, byte ptr [0x55b078]
    cpu.bl = app->getMemory<x86::reg8>(x86::reg32(5615736) /* 0x55b078 */);
    // 00473682  e8d9fcffff             -call 0x473360
    cpu.esp -= 4;
    sub_473360(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473687  8a8079b05500           -mov al, byte ptr [eax + 0x55b079]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5615737) /* 0x55b079 */);
    // 0047368d  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 00473692  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473693  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473694  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473695  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x00473696:
    // 00473696  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00473698  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0047369a  8a1d80b05500           -mov bl, byte ptr [0x55b080]
    cpu.bl = app->getMemory<x86::reg8>(x86::reg32(5615744) /* 0x55b080 */);
    // 004736a0  e8bbfcffff             -call 0x473360
    cpu.esp -= 4;
    sub_473360(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004736a5  8a8081b05500           -mov al, byte ptr [eax + 0x55b081]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5615745) /* 0x55b081 */);
    // 004736ab  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
L_0x004736b0:
    // 004736b0  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004736b1  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004736b2  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004736b3  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_4736c0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004736c0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004736c1  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004736c2  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004736c3  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004736c4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004736c5  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004736c7  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004736c9  a130dc6f00             -mov eax, dword ptr [0x6fdc30]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7330864) /* 0x6fdc30 */);
    // 004736ce  894214                 -mov dword ptr [edx + 0x14], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(20) /* 0x14 */) = cpu.eax;
    // 004736d1  8b4214                 -mov eax, dword ptr [edx + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(20) /* 0x14 */);
    // 004736d4  c7421c01000000         -mov dword ptr [edx + 0x1c], 1
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(28) /* 0x1c */) = 1 /*0x1*/;
    // 004736db  e8305efcff             -call 0x439510
    cpu.esp -= 4;
    sub_439510(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004736e0  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004736e2  8d7a5c                 -lea edi, [edx + 0x5c]
    cpu.edi = x86::reg32(cpu.edx + x86::reg32(92) /* 0x5c */);
    // 004736e5  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
    app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
    if (cpu.flags.df)
    {
        cpu.edi -= 4;
        cpu.esi -= 4;
    }
    else
    {
        cpu.edi += 4;
        cpu.esi += 4;
    }
    // 004736e6  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
    app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
    if (cpu.flags.df)
    {
        cpu.edi -= 4;
        cpu.esi -= 4;
    }
    else
    {
        cpu.edi += 4;
        cpu.esi += 4;
    }
    // 004736e7  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
    app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
    if (cpu.flags.df)
    {
        cpu.edi -= 4;
        cpu.esi -= 4;
    }
    else
    {
        cpu.edi += 4;
        cpu.esi += 4;
    }
    // 004736e8  b907000000             -mov ecx, 7
    cpu.ecx = 7 /*0x7*/;
    // 004736ed  be24d05500             -mov esi, 0x55d024
    cpu.esi = 5623844 /*0x55d024*/;
    // 004736f2  8d7a40                 -lea edi, [edx + 0x40]
    cpu.edi = x86::reg32(cpu.edx + x86::reg32(64) /* 0x40 */);
    // 004736f5  c7426800000000         -mov dword ptr [edx + 0x68], 0
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(104) /* 0x68 */) = 0 /*0x0*/;
    // 004736fc  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
    while (cpu.ecx)
    {
        app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
        if (cpu.flags.df)
        {
            cpu.edi -= 4;
            cpu.esi -= 4;
        }
        else
        {
            cpu.edi += 4;
            cpu.esi += 4;
        }
        --cpu.ecx;
    }
    // 004736fe  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004736ff  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473700  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473701  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473702  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473703  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_473710(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00473710  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00473711  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00473712  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00473713  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00473714  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00473716  83ec24                 -sub esp, 0x24
    (cpu.esp) -= x86::reg32(x86::sreg32(36 /*0x24*/));
    // 00473719  8945ec                 -mov dword ptr [ebp - 0x14], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.eax;
    // 0047371c  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 0047371e  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 00473720  7505                   -jne 0x473727
    if (!cpu.flags.zf)
    {
        goto L_0x00473727;
    }
    // 00473722  bebcd26f00             -mov esi, 0x6fd2bc
    cpu.esi = 7328444 /*0x6fd2bc*/;
L_0x00473727:
    // 00473727  83fb01                 +cmp ebx, 1
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0047372a  7505                   -jne 0x473731
    if (!cpu.flags.zf)
    {
        goto L_0x00473731;
    }
    // 0047372c  be28d36f00             -mov esi, 0x6fd328
    cpu.esi = 7328552 /*0x6fd328*/;
L_0x00473731:
    // 00473731  8b55ec                 -mov edx, dword ptr [ebp - 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00473734  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 00473736  c7426800000000         -mov dword ptr [edx + 0x68], 0
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(104) /* 0x68 */) = 0 /*0x0*/;
    // 0047373d  894214                 -mov dword ptr [edx + 0x14], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(20) /* 0x14 */) = cpu.eax;
    // 00473740  8b4618                 -mov eax, dword ptr [esi + 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(24) /* 0x18 */);
    // 00473743  89421c                 -mov dword ptr [edx + 0x1c], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(28) /* 0x1c */) = cpu.eax;
    // 00473746  f6052eeb550020         +test byte ptr [0x55eb2e], 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(x86::reg32(5630766) /* 0x55eb2e */) & 32 /*0x20*/));
    // 0047374d  0f846f000000           -je 0x4737c2
    if (cpu.flags.zf)
    {
        goto L_0x004737c2;
    }
    // 00473753  ba0f000000             -mov edx, 0xf
    cpu.edx = 15 /*0xf*/;
    // 00473758  8d45dc                 -lea eax, [ebp - 0x24]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 0047375b  e87053fcff             -call 0x438ad0
    cpu.esp -= 4;
    nfs3hp::load::makeCarId(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473760  8d45dc                 -lea eax, [ebp - 0x24]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 00473763  8b55ec                 -mov edx, dword ptr [ebp - 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00473766  e82562fcff             -call 0x439990
    cpu.esp -= 4;
    nfs3hp::load::findCarById(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047376b  894214                 -mov dword ptr [edx + 0x14], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(20) /* 0x14 */) = cpu.eax;
L_0x0047376e:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0047376e  e8897a0700             -call 0x4eb1fc
    cpu.esp -= 4;
    nfs3hp::watcom::rand(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473773  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00473775  83e207                 +and edx, 7
    cpu.clear_co();
    cpu.set_szp((cpu.edx &= x86::reg32(x86::sreg32(7 /*0x7*/))));
    // 00473778  74f4                   -je 0x47376e
    if (cpu.flags.zf)
    {
        goto L_0x0047376e;
    }
    // 0047377a  83fa02                 +cmp edx, 2
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0047377d  7502                   -jne 0x473781
    if (!cpu.flags.zf)
    {
        goto L_0x00473781;
    }
    // 0047377f  ebed                   -jmp 0x47376e
    goto L_0x0047376e;
L_0x00473781:
    // 00473781  83fa03                 +cmp edx, 3
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00473784  74e8                   -je 0x47376e
    if (cpu.flags.zf)
    {
        goto L_0x0047376e;
    }
    // 00473786  8d45f0                 -lea eax, [ebp - 0x10]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00473789  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0047378a  8d45f8                 -lea eax, [ebp - 8]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0047378d  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0047378e  8d45f4                 -lea eax, [ebp - 0xc]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00473791  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00473792  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00473795  8d4dfc                 -lea ecx, [ebp - 4]
    cpu.ecx = x86::reg32(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00473798  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 0047379a  8b4014                 -mov eax, dword ptr [eax + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(20) /* 0x14 */);
    // 0047379d  891500906700           -mov dword ptr [0x679000], edx
    app->getMemory<x86::reg32>(x86::reg32(6787072) /* 0x679000 */) = cpu.edx;
    // 004737a3  e86860fcff             -call 0x439810
    cpu.esp -= 4;
    nfs3hp::port::fields::carColourRgb(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004737a8  8b55ec                 -mov edx, dword ptr [ebp - 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004737ab  8a45fc                 -mov al, byte ptr [ebp - 4]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004737ae  884220                 -mov byte ptr [edx + 0x20], al
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(32) /* 0x20 */) = cpu.al;
    // 004737b1  8a45f4                 -mov al, byte ptr [ebp - 0xc]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004737b4  884221                 -mov byte ptr [edx + 0x21], al
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(33) /* 0x21 */) = cpu.al;
    // 004737b7  8a45f8                 -mov al, byte ptr [ebp - 8]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004737ba  884222                 -mov byte ptr [edx + 0x22], al
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(34) /* 0x22 */) = cpu.al;
    // 004737bd  8a45f0                 -mov al, byte ptr [ebp - 0x10]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004737c0  eb15                   -jmp 0x4737d7
    goto L_0x004737d7;
L_0x004737c2:
    // 004737c2  8a464b                 -mov al, byte ptr [esi + 0x4b]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(75) /* 0x4b */);
    // 004737c5  884220                 -mov byte ptr [edx + 0x20], al
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(32) /* 0x20 */) = cpu.al;
    // 004737c8  8a464c                 -mov al, byte ptr [esi + 0x4c]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(76) /* 0x4c */);
    // 004737cb  884221                 -mov byte ptr [edx + 0x21], al
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(33) /* 0x21 */) = cpu.al;
    // 004737ce  8a464d                 -mov al, byte ptr [esi + 0x4d]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(77) /* 0x4d */);
    // 004737d1  884222                 -mov byte ptr [edx + 0x22], al
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(34) /* 0x22 */) = cpu.al;
    // 004737d4  8a464e                 -mov al, byte ptr [esi + 0x4e]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(78) /* 0x4e */);
L_0x004737d7:
    // 004737d7  884223                 -mov byte ptr [edx + 0x23], al
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(35) /* 0x23 */) = cpu.al;
    // 004737da  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004737dd  c7401801000000         -mov dword ptr [eax + 0x18], 1
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(24) /* 0x18 */) = 1 /*0x1*/;
    // 004737e4  8b55ec                 -mov edx, dword ptr [ebp - 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004737e7  8b4620                 -mov eax, dword ptr [esi + 0x20]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(32) /* 0x20 */);
    // 004737ea  894224                 -mov dword ptr [edx + 0x24], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(36) /* 0x24 */) = cpu.eax;
    // 004737ed  8b4624                 -mov eax, dword ptr [esi + 0x24]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(36) /* 0x24 */);
    // 004737f0  894228                 -mov dword ptr [edx + 0x28], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(40) /* 0x28 */) = cpu.eax;
    // 004737f3  8b4628                 -mov eax, dword ptr [esi + 0x28]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(40) /* 0x28 */);
    // 004737f6  bb0b000000             -mov ebx, 0xb
    cpu.ebx = 11 /*0xb*/;
    // 004737fb  89422c                 -mov dword ptr [edx + 0x2c], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(44) /* 0x2c */) = cpu.eax;
    // 004737fe  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00473801  8d5640                 -lea edx, [esi + 0x40]
    cpu.edx = x86::reg32(cpu.esi + x86::reg32(64) /* 0x40 */);
    // 00473804  83c034                 -add eax, 0x34
    (cpu.eax) += x86::reg32(x86::sreg32(52 /*0x34*/));
    // 00473807  e824d60600             -call 0x4e0e30
    cpu.esp -= 4;
    nfs3hp::watcom::strncpy(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047380c  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0047380f  897810                 -mov dword ptr [eax + 0x10], edi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */) = cpu.edi;
    // 00473812  833db0d36f0001         +cmp dword ptr [0x6fd3b0], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00473819  7f1d                   -jg 0x473838
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00473838;
    }
    // 0047381b  8b0db8d36f00           -mov ecx, dword ptr [0x6fd3b8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7328696) /* 0x6fd3b8 */);
    // 00473821  83f901                 +cmp ecx, 1
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00473824  7412                   -je 0x473838
    if (cpu.flags.zf)
    {
        goto L_0x00473838;
    }
    // 00473826  83f902                 +cmp ecx, 2
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00473829  740d                   -je 0x473838
    if (cpu.flags.zf)
    {
        goto L_0x00473838;
    }
    // 0047382b  b907000000             -mov ecx, 7
    cpu.ecx = 7 /*0x7*/;
    // 00473830  8d7840                 -lea edi, [eax + 0x40]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(64) /* 0x40 */);
    // 00473833  8d7650                 -lea esi, [esi + 0x50]
    cpu.esi = x86::reg32(cpu.esi + x86::reg32(80) /* 0x50 */);
    // 00473836  eb10                   -jmp 0x473848
    goto L_0x00473848;
L_0x00473838:
    // 00473838  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0047383b  b907000000             -mov ecx, 7
    cpu.ecx = 7 /*0x7*/;
    // 00473840  be24d05500             -mov esi, 0x55d024
    cpu.esi = 5623844 /*0x55d024*/;
    // 00473845  8d7840                 -lea edi, [eax + 0x40]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(64) /* 0x40 */);
L_0x00473848:
    // 00473848  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
    while (cpu.ecx)
    {
        app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
        if (cpu.flags.df)
        {
            cpu.edi -= 4;
            cpu.esi -= 4;
        }
        else
        {
            cpu.edi += 4;
            cpu.esi += 4;
        }
        --cpu.ecx;
    }
    // 0047384a  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0047384d  8b4014                 -mov eax, dword ptr [eax + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(20) /* 0x14 */);
    // 00473850  8b55ec                 -mov edx, dword ptr [ebp - 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00473853  e8b85cfcff             -call 0x439510
    cpu.esp -= 4;
    sub_439510(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473858  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0047385a  8d7a5c                 -lea edi, [edx + 0x5c]
    cpu.edi = x86::reg32(cpu.edx + x86::reg32(92) /* 0x5c */);
    // 0047385d  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
    app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
    if (cpu.flags.df)
    {
        cpu.edi -= 4;
        cpu.esi -= 4;
    }
    else
    {
        cpu.edi += 4;
        cpu.esi += 4;
    }
    // 0047385e  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
    app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
    if (cpu.flags.df)
    {
        cpu.edi -= 4;
        cpu.esi -= 4;
    }
    else
    {
        cpu.edi += 4;
        cpu.esi += 4;
    }
    // 0047385f  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
    app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
    if (cpu.flags.df)
    {
        cpu.edi -= 4;
        cpu.esi -= 4;
    }
    else
    {
        cpu.edi += 4;
        cpu.esi += 4;
    }
    // 00473860  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00473862  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473863  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473864  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473865  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473866  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_473890(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 00473890  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00473891  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00473892  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00473893  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00473894  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00473896  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00473899  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0047389c  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0047389e  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 004738a0  741e                   -je 0x4738c0
    if (cpu.flags.zf)
    {
        goto L_0x004738c0;
    }
    // 004738a2  8b5dfc                 -mov ebx, dword ptr [ebp - 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004738a5  8a152eeb5500           -mov dl, byte ptr [0x55eb2e]
    cpu.dl = app->getMemory<x86::reg8>(x86::reg32(5630766) /* 0x55eb2e */);
    // 004738ab  c74314f8010000         -mov dword ptr [ebx + 0x14], 0x1f8
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(20) /* 0x14 */) = 504 /*0x1f8*/;
    // 004738b2  f6c220                 +test dl, 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.dl & 32 /*0x20*/));
    // 004738b5  7449                   -je 0x473900
    if (cpu.flags.zf)
    {
        goto L_0x00473900;
    }
    // 004738b7  c74314f9010000         -mov dword ptr [ebx + 0x14], 0x1f9
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(20) /* 0x14 */) = 505 /*0x1f9*/;
    // 004738be  eb40                   -jmp 0x473900
    goto L_0x00473900;
L_0x004738c0:
    // 004738c0  8b1dacd46f00           -mov ebx, dword ptr [0x6fd4ac]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7328940) /* 0x6fd4ac */);
    // 004738c6  83fb08                 +cmp ebx, 8
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(8 /*0x8*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004738c9  772b                   -ja 0x4738f6
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x004738f6;
    }
    // 004738cb  ff249d68384700         -jmp dword ptr [ebx*4 + 0x473868]
    cpu.ip = app->getMemory<x86::reg32>(4667496 + cpu.ebx * 4); goto dynamic_jump;
  case 0x004738d2:
    // 004738d2  8b5dfc                 -mov ebx, dword ptr [ebp - 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004738d5  c74314f5010000         -mov dword ptr [ebx + 0x14], 0x1f5
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(20) /* 0x14 */) = 501 /*0x1f5*/;
    // 004738dc  eb22                   -jmp 0x473900
    goto L_0x00473900;
  case 0x004738de:
    // 004738de  8b5dfc                 -mov ebx, dword ptr [ebp - 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004738e1  c74314f6010000         -mov dword ptr [ebx + 0x14], 0x1f6
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(20) /* 0x14 */) = 502 /*0x1f6*/;
    // 004738e8  eb16                   -jmp 0x473900
    goto L_0x00473900;
  case 0x004738ea:
    // 004738ea  8b5dfc                 -mov ebx, dword ptr [ebp - 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004738ed  c74314f7010000         -mov dword ptr [ebx + 0x14], 0x1f7
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(20) /* 0x14 */) = 503 /*0x1f7*/;
    // 004738f4  eb0a                   -jmp 0x473900
    goto L_0x00473900;
  case 0x004738f6:
L_0x004738f6:
    // 004738f6  8b5dfc                 -mov ebx, dword ptr [ebp - 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004738f9  c74314f4010000         -mov dword ptr [ebx + 0x14], 0x1f4
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(20) /* 0x14 */) = 500 /*0x1f4*/;
L_0x00473900:
    // 00473900  8b5dfc                 -mov ebx, dword ptr [ebp - 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00473903  c7431c01000000         -mov dword ptr [ebx + 0x1c], 1
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(28) /* 0x1c */) = 1 /*0x1*/;
    // 0047390a  c7431808000000         -mov dword ptr [ebx + 0x18], 8
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(24) /* 0x18 */) = 8 /*0x8*/;
    // 00473911  e85afcffff             -call 0x473570
    cpu.esp -= 4;
    sub_473570(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473916  894330                 -mov dword ptr [ebx + 0x30], eax
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(48) /* 0x30 */) = cpu.eax;
    // 00473919  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0047391b  e8d00ef9ff             -call 0x4047f0
    cpu.esp -= 4;
    nfs3hp::airace::classValue(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473920  8d4b34                 -lea ecx, [ebx + 0x34]
    cpu.ecx = x86::reg32(cpu.ebx + x86::reg32(52) /* 0x34 */);
    // 00473923  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00473925  bb0b000000             -mov ebx, 0xb
    cpu.ebx = 11 /*0xb*/;
    // 0047392a  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0047392c  e8ffd40600             -call 0x4e0e30
    cpu.esp -= 4;
    nfs3hp::watcom::strncpy(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473931  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00473934  be24d05500             -mov esi, 0x55d024
    cpu.esi = 5623844 /*0x55d024*/;
    // 00473939  b907000000             -mov ecx, 7
    cpu.ecx = 7 /*0x7*/;
    // 0047393e  8d7840                 -lea edi, [eax + 0x40]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(64) /* 0x40 */);
    // 00473941  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
    while (cpu.ecx)
    {
        app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
        if (cpu.flags.df)
        {
            cpu.edi -= 4;
            cpu.esi -= 4;
        }
        else
        {
            cpu.edi += 4;
            cpu.esi += 4;
        }
        --cpu.ecx;
    }
    // 00473943  be8c445600             -mov esi, 0x56448c
    cpu.esi = 5653644 /*0x56448c*/;
    // 00473948  8d785c                 -lea edi, [eax + 0x5c]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(92) /* 0x5c */);
    // 0047394b  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
    app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
    if (cpu.flags.df)
    {
        cpu.edi -= 4;
        cpu.esi -= 4;
    }
    else
    {
        cpu.edi += 4;
        cpu.esi += 4;
    }
    // 0047394c  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
    app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
    if (cpu.flags.df)
    {
        cpu.edi -= 4;
        cpu.esi -= 4;
    }
    else
    {
        cpu.edi += 4;
        cpu.esi += 4;
    }
    // 0047394d  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
    app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
    if (cpu.flags.df)
    {
        cpu.edi -= 4;
        cpu.esi -= 4;
    }
    else
    {
        cpu.edi += 4;
        cpu.esi += 4;
    }
    // 0047394e  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00473950  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473951  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473952  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473953  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473954  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_473960(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00473960  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00473961  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00473962  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00473963  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00473964  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00473965  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00473967  83ec24                 -sub esp, 0x24
    (cpu.esp) -= x86::reg32(x86::sreg32(36 /*0x24*/));
    // 0047396a  8945ec                 -mov dword ptr [ebp - 0x14], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.eax;
    // 0047396d  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0047396f  8b4dec                 -mov ecx, dword ptr [ebp - 0x14]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00473972  e859fbffff             -call 0x4734d0
    cpu.esp -= 4;
    sub_4734d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473977  c7411c01000000         -mov dword ptr [ecx + 0x1c], 1
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(28) /* 0x1c */) = 1 /*0x1*/;
    // 0047397e  c7416800000000         -mov dword ptr [ecx + 0x68], 0
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(104) /* 0x68 */) = 0 /*0x0*/;
    // 00473985  bb0b000000             -mov ebx, 0xb
    cpu.ebx = 11 /*0xb*/;
    // 0047398a  894114                 -mov dword ptr [ecx + 0x14], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(20) /* 0x14 */) = cpu.eax;
    // 0047398d  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0047398f  c7411802000000         -mov dword ptr [ecx + 0x18], 2
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(24) /* 0x18 */) = 2 /*0x2*/;
    // 00473996  e8b5fbffff             -call 0x473550
    cpu.esp -= 4;
    sub_473550(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047399b  894130                 -mov dword ptr [ecx + 0x30], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(48) /* 0x30 */) = cpu.eax;
    // 0047399e  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004739a0  895110                 -mov dword ptr [ecx + 0x10], edx
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(16) /* 0x10 */) = cpu.edx;
    // 004739a3  e8480ef9ff             -call 0x4047f0
    cpu.esp -= 4;
    nfs3hp::airace::classValue(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004739a8  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004739aa  8d4134                 -lea eax, [ecx + 0x34]
    cpu.eax = x86::reg32(cpu.ecx + x86::reg32(52) /* 0x34 */);
    // 004739ad  e87ed40600             -call 0x4e0e30
    cpu.esp -= 4;
    nfs3hp::watcom::strncpy(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004739b2  f6052eeb550020         +test byte ptr [0x55eb2e], 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(x86::reg32(5630766) /* 0x55eb2e */) & 32 /*0x20*/));
    // 004739b9  0f8473000000           -je 0x473a32
    if (cpu.flags.zf)
    {
        goto L_0x00473a32;
    }
    // 004739bf  ba0f000000             -mov edx, 0xf
    cpu.edx = 15 /*0xf*/;
    // 004739c4  8d45dc                 -lea eax, [ebp - 0x24]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 004739c7  e80451fcff             -call 0x438ad0
    cpu.esp -= 4;
    nfs3hp::load::makeCarId(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004739cc  8d45dc                 -lea eax, [ebp - 0x24]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 004739cf  e8bc5ffcff             -call 0x439990
    cpu.esp -= 4;
    nfs3hp::load::findCarById(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004739d4  894114                 -mov dword ptr [ecx + 0x14], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(20) /* 0x14 */) = cpu.eax;
L_0x004739d7:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004739d7  e820780700             -call 0x4eb1fc
    cpu.esp -= 4;
    nfs3hp::watcom::rand(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004739dc  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004739de  83e207                 +and edx, 7
    cpu.clear_co();
    cpu.set_szp((cpu.edx &= x86::reg32(x86::sreg32(7 /*0x7*/))));
    // 004739e1  74f4                   -je 0x4739d7
    if (cpu.flags.zf)
    {
        goto L_0x004739d7;
    }
    // 004739e3  83fa02                 +cmp edx, 2
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004739e6  7502                   -jne 0x4739ea
    if (!cpu.flags.zf)
    {
        goto L_0x004739ea;
    }
    // 004739e8  ebed                   -jmp 0x4739d7
    goto L_0x004739d7;
L_0x004739ea:
    // 004739ea  83fa03                 +cmp edx, 3
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004739ed  7502                   -jne 0x4739f1
    if (!cpu.flags.zf)
    {
        goto L_0x004739f1;
    }
    // 004739ef  ebe6                   -jmp 0x4739d7
    goto L_0x004739d7;
L_0x004739f1:
    // 004739f1  3b1500906700           +cmp edx, dword ptr [0x679000]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(6787072) /* 0x679000 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004739f7  74de                   -je 0x4739d7
    if (cpu.flags.zf)
    {
        goto L_0x004739d7;
    }
    // 004739f9  8d45fc                 -lea eax, [ebp - 4]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004739fc  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004739fd  8d45f8                 -lea eax, [ebp - 8]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00473a00  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00473a01  8d45f4                 -lea eax, [ebp - 0xc]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00473a04  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00473a05  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00473a08  8d4df0                 -lea ecx, [ebp - 0x10]
    cpu.ecx = x86::reg32(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00473a0b  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 00473a0d  8b4014                 -mov eax, dword ptr [eax + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(20) /* 0x14 */);
    // 00473a10  e8fb5dfcff             -call 0x439810
    cpu.esp -= 4;
    nfs3hp::port::fields::carColourRgb(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473a15  8b55ec                 -mov edx, dword ptr [ebp - 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00473a18  8a45f0                 -mov al, byte ptr [ebp - 0x10]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00473a1b  884220                 -mov byte ptr [edx + 0x20], al
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(32) /* 0x20 */) = cpu.al;
    // 00473a1e  8a45f4                 -mov al, byte ptr [ebp - 0xc]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00473a21  884221                 -mov byte ptr [edx + 0x21], al
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(33) /* 0x21 */) = cpu.al;
    // 00473a24  8a45f8                 -mov al, byte ptr [ebp - 8]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00473a27  884222                 -mov byte ptr [edx + 0x22], al
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(34) /* 0x22 */) = cpu.al;
    // 00473a2a  8a45fc                 -mov al, byte ptr [ebp - 4]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00473a2d  884223                 -mov byte ptr [edx + 0x23], al
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(35) /* 0x23 */) = cpu.al;
    // 00473a30  eb10                   -jmp 0x473a42
    goto L_0x00473a42;
L_0x00473a32:
    // 00473a32  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00473a34  e8d70df9ff             -call 0x404810
    cpu.esp -= 4;
    nfs3hp::airace::classValueToo(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473a39  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00473a3b  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00473a3d  e87ef9ffff             -call 0x4733c0
    cpu.esp -= 4;
    nfs3hp::port::view::colourNearest(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00473a42:
    // 00473a42  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00473a45  b907000000             -mov ecx, 7
    cpu.ecx = 7 /*0x7*/;
    // 00473a4a  be24d05500             -mov esi, 0x55d024
    cpu.esi = 5623844 /*0x55d024*/;
    // 00473a4f  8d7840                 -lea edi, [eax + 0x40]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(64) /* 0x40 */);
    // 00473a52  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
    while (cpu.ecx)
    {
        app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
        if (cpu.flags.df)
        {
            cpu.edi -= 4;
            cpu.esi -= 4;
        }
        else
        {
            cpu.edi += 4;
            cpu.esi += 4;
        }
        --cpu.ecx;
    }
    // 00473a54  be8c445600             -mov esi, 0x56448c
    cpu.esi = 5653644 /*0x56448c*/;
    // 00473a59  8d785c                 -lea edi, [eax + 0x5c]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(92) /* 0x5c */);
    // 00473a5c  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
    app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
    if (cpu.flags.df)
    {
        cpu.edi -= 4;
        cpu.esi -= 4;
    }
    else
    {
        cpu.edi += 4;
        cpu.esi += 4;
    }
    // 00473a5d  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
    app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
    if (cpu.flags.df)
    {
        cpu.edi -= 4;
        cpu.esi -= 4;
    }
    else
    {
        cpu.edi += 4;
        cpu.esi += 4;
    }
    // 00473a5e  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
    app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
    if (cpu.flags.df)
    {
        cpu.edi -= 4;
        cpu.esi -= 4;
    }
    else
    {
        cpu.edi += 4;
        cpu.esi += 4;
    }
    // 00473a5f  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00473a61  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473a62  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473a63  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473a64  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473a65  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473a66  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_473a70(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00473a70  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00473a71  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00473a72  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00473a73  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00473a74  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00473a75  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00473a77  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00473a79  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00473a7b  e840fbffff             -call 0x4735c0
    cpu.esp -= 4;
    sub_4735c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473a80  c7431804000000         -mov dword ptr [ebx + 0x18], 4
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(24) /* 0x18 */) = 4 /*0x4*/;
    // 00473a87  c7431c01000000         -mov dword ptr [ebx + 0x1c], 1
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(28) /* 0x1c */) = 1 /*0x1*/;
    // 00473a8e  c6433400               -mov byte ptr [ebx + 0x34], 0
    app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(52) /* 0x34 */) = 0 /*0x0*/;
    // 00473a92  b907000000             -mov ecx, 7
    cpu.ecx = 7 /*0x7*/;
    // 00473a97  c7436800000000         -mov dword ptr [ebx + 0x68], 0
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(104) /* 0x68 */) = 0 /*0x0*/;
    // 00473a9e  be24d05500             -mov esi, 0x55d024
    cpu.esi = 5623844 /*0x55d024*/;
    // 00473aa3  8d7b40                 -lea edi, [ebx + 0x40]
    cpu.edi = x86::reg32(cpu.ebx + x86::reg32(64) /* 0x40 */);
    // 00473aa6  894314                 -mov dword ptr [ebx + 0x14], eax
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(20) /* 0x14 */) = cpu.eax;
    // 00473aa9  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
    while (cpu.ecx)
    {
        app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
        if (cpu.flags.df)
        {
            cpu.edi -= 4;
            cpu.esi -= 4;
        }
        else
        {
            cpu.edi += 4;
            cpu.esi += 4;
        }
        --cpu.ecx;
    }
    // 00473aab  be8c445600             -mov esi, 0x56448c
    cpu.esi = 5653644 /*0x56448c*/;
    // 00473ab0  8d7b5c                 -lea edi, [ebx + 0x5c]
    cpu.edi = x86::reg32(cpu.ebx + x86::reg32(92) /* 0x5c */);
    // 00473ab3  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
    app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
    if (cpu.flags.df)
    {
        cpu.edi -= 4;
        cpu.esi -= 4;
    }
    else
    {
        cpu.edi += 4;
        cpu.esi += 4;
    }
    // 00473ab4  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
    app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
    if (cpu.flags.df)
    {
        cpu.edi -= 4;
        cpu.esi -= 4;
    }
    else
    {
        cpu.edi += 4;
        cpu.esi += 4;
    }
    // 00473ab5  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
    app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
    if (cpu.flags.df)
    {
        cpu.edi -= 4;
        cpu.esi -= 4;
    }
    else
    {
        cpu.edi += 4;
        cpu.esi += 4;
    }
    // 00473ab6  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473ab7  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473ab8  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473ab9  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473aba  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473abb  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_473ac0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00473ac0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00473ac1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00473ac2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00473ac3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00473ac4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00473ac5  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00473ac7  83ec24                 -sub esp, 0x24
    (cpu.esp) -= x86::reg32(x86::sreg32(36 /*0x24*/));
    // 00473aca  b938010000             -mov ecx, 0x138
    cpu.ecx = 312 /*0x138*/;
    // 00473acf  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00473ad1  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 00473ad6  895485dc               -mov dword ptr [ebp + eax*4 - 0x24], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */ + cpu.eax * 4) = cpu.edx;
    // 00473ada  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
L_0x00473adc:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00473adc  83f809                 +cmp eax, 9
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(9 /*0x9*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00473adf  7d0d                   -jge 0x473aee
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00473aee;
    }
    // 00473ae1  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 00473ae6  895485dc               -mov dword ptr [ebp + eax*4 - 0x24], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */ + cpu.eax * 4) = cpu.edx;
    // 00473aea  01d0                   +add eax, edx
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00473aec  ebee                   -jmp 0x473adc
    goto L_0x00473adc;
L_0x00473aee:
    // 00473aee  bb02000000             -mov ebx, 2
    cpu.ebx = 2 /*0x2*/;
    // 00473af3  eb2d                   -jmp 0x473b22
    goto L_0x00473b22;
L_0x00473af5:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00473af5  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00473af7  e864110000             -call 0x474c60
    cpu.esp -= 4;
    sub_474c60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473afc  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00473afe  be04000000             -mov esi, 4
    cpu.esi = 4 /*0x4*/;
    // 00473b03  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00473b06  41                     -inc ecx
    (cpu.ecx)++;
    // 00473b07  f7fe                   -idiv esi
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.esi);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
L_0x00473b09:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00473b09  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00473b10  837c28dc00             +cmp dword ptr [eax + ebp - 0x24], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-36) /* -0x24 */ + cpu.ebp * 1);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00473b15  7407                   -je 0x473b1e
    if (cpu.flags.zf)
    {
        goto L_0x00473b1e;
    }
    // 00473b17  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00473b19  4b                     -dec ebx
    (cpu.ebx)--;
    // 00473b1a  895428dc               -mov dword ptr [eax + ebp - 0x24], edx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-36) /* -0x24 */ + cpu.ebp * 1) = cpu.edx;
L_0x00473b1e:
    // 00473b1e  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 00473b20  7421                   -je 0x473b43
    if (cpu.flags.zf)
    {
        goto L_0x00473b43;
    }
L_0x00473b22:
    // 00473b22  833d70d46f0000         +cmp dword ptr [0x6fd470], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328880) /* 0x6fd470 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00473b29  75ca                   -jne 0x473af5
    if (!cpu.flags.zf)
    {
        goto L_0x00473af5;
    }
    // 00473b2b  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00473b2d  e82e110000             -call 0x474c60
    cpu.esp -= 4;
    sub_474c60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473b32  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00473b34  be04000000             -mov esi, 4
    cpu.esi = 4 /*0x4*/;
    // 00473b39  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00473b3c  f7fe                   -idiv esi
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.esi);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 00473b3e  41                     -inc ecx
    (cpu.ecx)++;
    // 00473b3f  01f2                   +add edx, esi
    {
        x86::reg32& tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.esi));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00473b41  ebc6                   -jmp 0x473b09
    goto L_0x00473b09;
L_0x00473b43:
    // 00473b43  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00473b45  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 00473b47  eb0f                   -jmp 0x473b58
    goto L_0x00473b58;
L_0x00473b49:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00473b49  66890455fed36f00       -mov word ptr [edx*2 + 0x6fd3fe], ax
    app->getMemory<x86::reg16>(x86::reg32(7328766) /* 0x6fd3fe */ + cpu.edx * 2) = cpu.ax;
    // 00473b51  42                     -inc edx
    (cpu.edx)++;
    // 00473b52  40                     -inc eax
    (cpu.eax)++;
    // 00473b53  83fa07                 +cmp edx, 7
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(7 /*0x7*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00473b56  7d0a                   -jge 0x473b62
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00473b62;
    }
L_0x00473b58:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00473b58  837c85dc00             +cmp dword ptr [ebp + eax*4 - 0x24], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */ + cpu.eax * 4);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00473b5d  75ea                   -jne 0x473b49
    if (!cpu.flags.zf)
    {
        goto L_0x00473b49;
    }
    // 00473b5f  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00473b60  ebf6                   -jmp 0x473b58
    goto L_0x00473b58;
L_0x00473b62:
    // 00473b62  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00473b64  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473b65  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473b66  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473b67  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473b68  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473b69  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_473b70(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00473b70  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00473b71  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00473b72  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00473b73  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00473b74  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00473b75  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00473b77  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00473b7a  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 00473b7d  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 00473b7f  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 00473b81  7511                   -jne 0x473b94
    if (!cpu.flags.zf)
    {
        goto L_0x00473b94;
    }
    // 00473b83  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x00473b85:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00473b85  3b45fc                 +cmp eax, dword ptr [ebp - 4]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00473b88  7d45                   -jge 0x473bcf
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00473bcf;
    }
    // 00473b8a  890485ecdb6f00         -mov dword ptr [eax*4 + 0x6fdbec], eax
    app->getMemory<x86::reg32>(x86::reg32(7330796) /* 0x6fdbec */ + cpu.eax * 4) = cpu.eax;
    // 00473b91  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00473b92  ebf1                   -jmp 0x473b85
    goto L_0x00473b85;
L_0x00473b94:
    // 00473b94  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00473b96  31f6                   +xor esi, esi
    cpu.clear_co();
    cpu.set_szp((cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi))));
    // 00473b98  eb06                   -jmp 0x473ba0
    goto L_0x00473ba0;
L_0x00473b9a:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00473b9a  43                     -inc ebx
    (cpu.ebx)++;
    // 00473b9b  83fb10                 +cmp ebx, 0x10
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(16 /*0x10*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00473b9e  7d2f                   -jge 0x473bcf
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00473bcf;
    }
L_0x00473ba0:
    // 00473ba0  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x00473ba2:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00473ba2  3b55fc                 +cmp edx, dword ptr [ebp - 4]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00473ba5  7df3                   -jge 0x473b9a
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00473b9a;
    }
    // 00473ba7  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00473bae  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00473bb0  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00473bb3  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00473bb5  8b84c13cd56f00         -mov eax, dword ptr [ecx + eax*8 + 0x6fd53c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(7329084) /* 0x6fd53c */ + cpu.eax * 8);
    // 00473bbc  0fbf0447               -movsx eax, word ptr [edi + eax*2]
    cpu.eax = x86::reg32(static_cast<x86::sreg16>(app->getMemory<x86::reg16>(cpu.edi + cpu.eax * 2)));
    // 00473bc0  39d8                   +cmp eax, ebx
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00473bc2  7508                   -jne 0x473bcc
    if (!cpu.flags.zf)
    {
        goto L_0x00473bcc;
    }
    // 00473bc4  893495ecdb6f00         -mov dword ptr [edx*4 + 0x6fdbec], esi
    app->getMemory<x86::reg32>(x86::reg32(7330796) /* 0x6fdbec */ + cpu.edx * 4) = cpu.esi;
    // 00473bcb  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
L_0x00473bcc:
    // 00473bcc  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00473bcd  ebd3                   -jmp 0x473ba2
    goto L_0x00473ba2;
L_0x00473bcf:
    // 00473bcf  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00473bd1  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473bd2  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473bd3  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473bd4  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473bd5  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473bd6  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_473bf0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 00473bf0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00473bf1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00473bf2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00473bf3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00473bf4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00473bf5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00473bf6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00473bf8  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00473bfb  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00473bfd  b894d36f00             -mov eax, 0x6fd394
    cpu.eax = 7328660 /*0x6fd394*/;
    // 00473c02  891510d56f00           -mov dword ptr [0x6fd510], edx
    app->getMemory<x86::reg32>(x86::reg32(7329040) /* 0x6fd510 */) = cpu.edx;
    // 00473c08  891514d56f00           -mov dword ptr [0x6fd514], edx
    app->getMemory<x86::reg32>(x86::reg32(7329044) /* 0x6fd514 */) = cpu.edx;
    // 00473c0e  891518d56f00           -mov dword ptr [0x6fd518], edx
    app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */) = cpu.edx;
    // 00473c14  89151cd56f00           -mov dword ptr [0x6fd51c], edx
    app->getMemory<x86::reg32>(x86::reg32(7329052) /* 0x6fd51c */) = cpu.edx;
    // 00473c1a  891520d56f00           -mov dword ptr [0x6fd520], edx
    app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */) = cpu.edx;
    // 00473c20  891524d56f00           -mov dword ptr [0x6fd524], edx
    app->getMemory<x86::reg32>(x86::reg32(7329060) /* 0x6fd524 */) = cpu.edx;
    // 00473c26  891528d56f00           -mov dword ptr [0x6fd528], edx
    app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */) = cpu.edx;
    // 00473c2c  ba08000000             -mov edx, 8
    cpu.edx = 8 /*0x8*/;
    // 00473c31  e8d6ca0600             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473c36  a174d46f00             -mov eax, dword ptr [0x6fd474]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328884) /* 0x6fd474 */);
    // 00473c3b  a390d46f00             -mov dword ptr [0x6fd490], eax
    app->getMemory<x86::reg32>(x86::reg32(7328912) /* 0x6fd490 */) = cpu.eax;
    // 00473c40  a178d46f00             -mov eax, dword ptr [0x6fd478]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328888) /* 0x6fd478 */);
    // 00473c45  a394d46f00             -mov dword ptr [0x6fd494], eax
    app->getMemory<x86::reg32>(x86::reg32(7328916) /* 0x6fd494 */) = cpu.eax;
    // 00473c4a  a17cd46f00             -mov eax, dword ptr [0x6fd47c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328892) /* 0x6fd47c */);
    // 00473c4f  a398d46f00             -mov dword ptr [0x6fd498], eax
    app->getMemory<x86::reg32>(x86::reg32(7328920) /* 0x6fd498 */) = cpu.eax;
    // 00473c54  a180d46f00             -mov eax, dword ptr [0x6fd480]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328896) /* 0x6fd480 */);
    // 00473c59  a39cd46f00             -mov dword ptr [0x6fd49c], eax
    app->getMemory<x86::reg32>(x86::reg32(7328924) /* 0x6fd49c */) = cpu.eax;
    // 00473c5e  a184d46f00             -mov eax, dword ptr [0x6fd484]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328900) /* 0x6fd484 */);
    // 00473c63  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00473c65  a3a0d46f00             -mov dword ptr [0x6fd4a0], eax
    app->getMemory<x86::reg32>(x86::reg32(7328928) /* 0x6fd4a0 */) = cpu.eax;
    // 00473c6a  a188d46f00             -mov eax, dword ptr [0x6fd488]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328904) /* 0x6fd488 */);
    // 00473c6f  894dfc                 -mov dword ptr [ebp - 4], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
    // 00473c72  a3a4d46f00             -mov dword ptr [0x6fd4a4], eax
    app->getMemory<x86::reg32>(x86::reg32(7328932) /* 0x6fd4a4 */) = cpu.eax;
    // 00473c77  a1b0d36f00             -mov eax, dword ptr [0x6fd3b0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
    // 00473c7c  890da8d46f00           -mov dword ptr [0x6fd4a8], ecx
    app->getMemory<x86::reg32>(x86::reg32(7328936) /* 0x6fd4a8 */) = cpu.ecx;
    // 00473c82  83f804                 +cmp eax, 4
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00473c85  0f874e000000           -ja 0x473cd9
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x00473cd9;
    }
    // 00473c8b  ff2485d83b4700         -jmp dword ptr [eax*4 + 0x473bd8]
    cpu.ip = app->getMemory<x86::reg32>(4668376 + cpu.eax * 4); goto dynamic_jump;
  case 0x00473c92:
    // 00473c92  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00473c95  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00473c9c  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00473c9e  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 00473ca0  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00473ca3  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00473ca5  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00473ca7  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 00473caa  8935f4d46f00           -mov dword ptr [0x6fd4f4], esi
    app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */) = cpu.esi;
    // 00473cb0  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00473cb2  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00473cb5  052cd56f00             -add eax, 0x6fd52c
    (cpu.eax) += x86::reg32(x86::sreg32(7329068 /*0x6fd52c*/));
    // 00473cba  8935f8d46f00           -mov dword ptr [0x6fd4f8], esi
    app->getMemory<x86::reg32>(x86::reg32(7329016) /* 0x6fd4f8 */) = cpu.esi;
    // 00473cc0  e84bfaffff             -call 0x473710
    cpu.esp -= 4;
    sub_473710(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473cc5  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00473cc8  8b1518d56f00           -mov edx, dword ptr [0x6fd518]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 00473cce  40                     -inc eax
    (cpu.eax)++;
    // 00473ccf  42                     -inc edx
    (cpu.edx)++;
    // 00473cd0  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 00473cd3  891518d56f00           -mov dword ptr [0x6fd518], edx
    app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */) = cpu.edx;
L_0x00473cd9:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00473cd9  a1d0d46f00             -mov eax, dword ptr [0x6fd4d0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328976) /* 0x6fd4d0 */);
    // 00473cde  83f802                 +cmp eax, 2
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00473ce1  0f8ce6000000           -jl 0x473dcd
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00473dcd;
    }
    // 00473ce7  ba08000000             -mov edx, 8
    cpu.edx = 8 /*0x8*/;
    // 00473cec  a118d56f00             -mov eax, dword ptr [0x6fd518]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 00473cf1  29c2                   +sub edx, eax
    {
        x86::reg32& tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00473cf3  891520d56f00           -mov dword ptr [0x6fd520], edx
    app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */) = cpu.edx;
    // 00473cf9  e9dd000000             -jmp 0x473ddb
    goto L_0x00473ddb;
  case 0x00473cfe:
    // 00473cfe  b82cd56f00             -mov eax, 0x6fd52c
    cpu.eax = 7329068 /*0x6fd52c*/;
    // 00473d03  890df4d46f00           -mov dword ptr [0x6fd4f4], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */) = cpu.ecx;
    // 00473d09  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00473d0b  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00473d0d  891df8d46f00           -mov dword ptr [0x6fd4f8], ebx
    app->getMemory<x86::reg32>(x86::reg32(7329016) /* 0x6fd4f8 */) = cpu.ebx;
    // 00473d13  e8f8f9ffff             -call 0x473710
    cpu.esp -= 4;
    sub_473710(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473d18  b82cd56f00             -mov eax, 0x6fd52c
    cpu.eax = 7329068 /*0x6fd52c*/;
    // 00473d1d  8b3518d56f00           -mov esi, dword ptr [0x6fd518]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 00473d23  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 00473d28  46                     -inc esi
    (cpu.esi)++;
    // 00473d29  83c06c                 +add eax, 0x6c
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(108 /*0x6c*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00473d2c  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 00473d2e  893518d56f00           -mov dword ptr [0x6fd518], esi
    app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */) = cpu.esi;
    // 00473d34  e8d7f9ffff             -call 0x473710
    cpu.esp -= 4;
    sub_473710(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473d39  a118d56f00             -mov eax, dword ptr [0x6fd518]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 00473d3e  bf02000000             -mov edi, 2
    cpu.edi = 2 /*0x2*/;
    // 00473d43  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00473d44  897dfc                 -mov dword ptr [ebp - 4], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edi;
    // 00473d47  a318d56f00             -mov dword ptr [0x6fd518], eax
    app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */) = cpu.eax;
    // 00473d4c  eb8b                   -jmp 0x473cd9
    goto L_0x00473cd9;
  case 0x00473d4e:
    // 00473d4e  a176227a00             -mov eax, dword ptr [0x7a2276]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8004214) /* 0x7a2276 */);
    // 00473d53  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00473d56  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00473d58  a3f4d46f00             -mov dword ptr [0x6fd4f4], eax
    app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */) = cpu.eax;
    // 00473d5d  a3f8d46f00             -mov dword ptr [0x6fd4f8], eax
    app->getMemory<x86::reg32>(x86::reg32(7329016) /* 0x6fd4f8 */) = cpu.eax;
L_0x00473d62:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00473d62  a174227a00             -mov eax, dword ptr [0x7a2274]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8004212) /* 0x7a2274 */);
    // 00473d67  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00473d6a  39c3                   +cmp ebx, eax
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00473d6c  0f8d67ffffff           -jge 0x473cd9
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00473cd9;
    }
    // 00473d72  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00473d75  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00473d7c  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00473d7e  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00473d81  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00473d83  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 00473d86  01c2                   +add edx, eax
    {
        x86::reg32& tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00473d88  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 00473d8d  be24d05500             -mov esi, 0x55d024
    cpu.esi = 5623844 /*0x55d024*/;
    // 00473d92  8dba6cd56f00           -lea edi, [edx + 0x6fd56c]
    cpu.edi = x86::reg32(cpu.edx + x86::reg32(7329132) /* 0x6fd56c */);
    // 00473d98  898a44d56f00           -mov dword ptr [edx + 0x6fd544], ecx
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7329092) /* 0x6fd544 */) = cpu.ecx;
    // 00473d9e  b907000000             -mov ecx, 7
    cpu.ecx = 7 /*0x7*/;
    // 00473da3  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
    while (cpu.ecx)
    {
        app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
        if (cpu.flags.df)
        {
            cpu.edi -= 4;
            cpu.esi -= 4;
        }
        else
        {
            cpu.edi += 4;
            cpu.esi += 4;
        }
        --cpu.ecx;
    }
    // 00473da5  8b8240d56f00           -mov eax, dword ptr [edx + 0x6fd540]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7329088) /* 0x6fd540 */);
    // 00473dab  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00473dac  e8ef57fcff             -call 0x4395a0
    cpu.esp -= 4;
    sub_4395a0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473db1  898294d56f00           -mov dword ptr [edx + 0x6fd594], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7329172) /* 0x6fd594 */) = cpu.eax;
    // 00473db7  8b7dfc                 -mov edi, dword ptr [ebp - 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00473dba  8b3518d56f00           -mov esi, dword ptr [0x6fd518]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 00473dc0  47                     +inc edi
    {
        x86::reg32& tmp = cpu.edi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00473dc1  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00473dc2  897dfc                 -mov dword ptr [ebp - 4], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edi;
    // 00473dc5  893518d56f00           -mov dword ptr [0x6fd518], esi
    app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */) = cpu.esi;
    // 00473dcb  eb95                   -jmp 0x473d62
    goto L_0x00473d62;
L_0x00473dcd:
    // 00473dcd  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00473dcf  7405                   -je 0x473dd6
    if (cpu.flags.zf)
    {
        goto L_0x00473dd6;
    }
    // 00473dd1  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
L_0x00473dd6:
    // 00473dd6  a320d56f00             -mov dword ptr [0x6fd520], eax
    app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */) = cpu.eax;
L_0x00473ddb:
    // 00473ddb  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
L_0x00473ddd:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00473ddd  8b1d20d56f00           -mov ebx, dword ptr [0x6fd520]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */);
    // 00473de3  39d9                   +cmp ecx, ebx
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00473de5  7d2d                   -jge 0x473e14
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00473e14;
    }
    // 00473de7  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00473dea  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00473df1  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00473df3  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00473df6  8b75fc                 -mov esi, dword ptr [ebp - 4]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00473df9  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00473dfb  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 00473dfe  41                     -inc ecx
    (cpu.ecx)++;
    // 00473dff  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00473e01  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00473e04  052cd56f00             +add eax, 0x6fd52c
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(7329068 /*0x6fd52c*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00473e09  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00473e0a  e851fbffff             -call 0x473960
    cpu.esp -= 4;
    sub_473960(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473e0f  8975fc                 -mov dword ptr [ebp - 4], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.esi;
    // 00473e12  ebc9                   -jmp 0x473ddd
    goto L_0x00473ddd;
L_0x00473e14:
    // 00473e14  8b3de8d46f00           -mov edi, dword ptr [0x6fd4e8]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7329000) /* 0x6fd4e8 */);
    // 00473e1a  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 00473e1c  7557                   -jne 0x473e75
    if (!cpu.flags.zf)
    {
        goto L_0x00473e75;
    }
    // 00473e1e  8b1518d56f00           -mov edx, dword ptr [0x6fd518]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 00473e24  8d0413                 -lea eax, [ebx + edx]
    cpu.eax = x86::reg32(cpu.ebx + cpu.edx * 1);
    // 00473e27  ba08000000             -mov edx, 8
    cpu.edx = 8 /*0x8*/;
    // 00473e2c  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 00473e2e  891528d56f00           -mov dword ptr [0x6fd528], edx
    app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */) = cpu.edx;
    // 00473e34  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 00473e36  7d06                   -jge 0x473e3e
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00473e3e;
    }
    // 00473e38  893d28d56f00           -mov dword ptr [0x6fd528], edi
    app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */) = cpu.edi;
L_0x00473e3e:
    // 00473e3e  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
L_0x00473e40:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00473e40  3b0d28d56f00           +cmp ecx, dword ptr [0x6fd528]
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00473e46  7d2d                   -jge 0x473e75
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00473e75;
    }
    // 00473e48  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00473e4b  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00473e52  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00473e54  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00473e57  8b7dfc                 -mov edi, dword ptr [ebp - 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00473e5a  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00473e5c  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 00473e5f  41                     -inc ecx
    (cpu.ecx)++;
    // 00473e60  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00473e62  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00473e65  052cd56f00             +add eax, 0x6fd52c
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(7329068 /*0x6fd52c*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00473e6a  47                     +inc edi
    {
        x86::reg32& tmp = cpu.edi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00473e6b  e800fcffff             -call 0x473a70
    cpu.esp -= 4;
    sub_473a70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473e70  897dfc                 -mov dword ptr [ebp - 4], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edi;
    // 00473e73  ebcb                   -jmp 0x473e40
    goto L_0x00473e40;
L_0x00473e75:
    // 00473e75  a118d56f00             -mov eax, dword ptr [0x6fd518]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 00473e7a  8b1520d56f00           -mov edx, dword ptr [0x6fd520]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */);
    // 00473e80  a314d56f00             -mov dword ptr [0x6fd514], eax
    app->getMemory<x86::reg32>(x86::reg32(7329044) /* 0x6fd514 */) = cpu.eax;
    // 00473e85  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00473e87  a128d56f00             -mov eax, dword ptr [0x6fd528]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */);
    // 00473e8c  8b0d20d56f00           -mov ecx, dword ptr [0x6fd520]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */);
    // 00473e92  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00473e94  a118d56f00             -mov eax, dword ptr [0x6fd518]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 00473e99  891510d56f00           -mov dword ptr [0x6fd510], edx
    app->getMemory<x86::reg32>(x86::reg32(7329040) /* 0x6fd510 */) = cpu.edx;
    // 00473e9f  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 00473ea1  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00473ea3  e8c8fcffff             -call 0x473b70
    cpu.esp -= 4;
    sub_473b70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473ea8  833db0d36f0000         +cmp dword ptr [0x6fd3b0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00473eaf  740e                   -je 0x473ebf
    if (cpu.flags.zf)
    {
        goto L_0x00473ebf;
    }
    // 00473eb1  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 00473eb3  893500d56f00           -mov dword ptr [0x6fd500], esi
    app->getMemory<x86::reg32>(x86::reg32(7329024) /* 0x6fd500 */) = cpu.esi;
    // 00473eb9  8935fcd46f00           -mov dword ptr [0x6fd4fc], esi
    app->getMemory<x86::reg32>(x86::reg32(7329020) /* 0x6fd4fc */) = cpu.esi;
L_0x00473ebf:
    // 00473ebf  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00473ec1  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473ec2  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473ec3  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473ec4  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473ec5  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473ec6  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00473ec7  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_473ee0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 00473ee0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00473ee1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00473ee2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00473ee3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00473ee4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00473ee5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00473ee6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00473ee8  83ec0c                 -sub esp, 0xc
    (cpu.esp) -= x86::reg32(x86::sreg32(12 /*0xc*/));
    // 00473eeb  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 00473ef0  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00473ef2  b890d46f00             -mov eax, 0x6fd490
    cpu.eax = 7328912 /*0x6fd490*/;
    // 00473ef7  8955f8                 -mov dword ptr [ebp - 8], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.edx;
    // 00473efa  ba1c000000             -mov edx, 0x1c
    cpu.edx = 28 /*0x1c*/;
    // 00473eff  890df0d46f00           -mov dword ptr [0x6fd4f0], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329008) /* 0x6fd4f0 */) = cpu.ecx;
    // 00473f05  e802c80600             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473f0a  66833dbcd36f0000       +cmp word ptr [0x6fd3bc], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(x86::reg32(7328700) /* 0x6fd3bc */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 00473f12  753e                   -jne 0x473f52
    if (!cpu.flags.zf)
    {
        goto L_0x00473f52;
    }
    // 00473f14  e8a7fbffff             -call 0x473ac0
    cpu.esp -= 4;
    sub_473ac0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473f19  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 00473f1b  eb05                   -jmp 0x473f22
    goto L_0x00473f22;
L_0x00473f1d:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00473f1d  83fb10                 +cmp ebx, 0x10
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(16 /*0x10*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00473f20  7f0b                   -jg 0x473f2d
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00473f2d;
    }
L_0x00473f22:
    // 00473f22  66891c5dbed36f00       -mov word ptr [ebx*2 + 0x6fd3be], bx
    app->getMemory<x86::reg16>(x86::reg32(7328702) /* 0x6fd3be */ + cpu.ebx * 2) = cpu.bx;
    // 00473f2a  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00473f2b  ebf0                   -jmp 0x473f1d
    goto L_0x00473f1d;
L_0x00473f2d:
    // 00473f2d  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 00473f2f  eb05                   -jmp 0x473f36
    goto L_0x00473f36;
L_0x00473f31:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00473f31  83fb10                 +cmp ebx, 0x10
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(16 /*0x10*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00473f34  7d0d                   -jge 0x473f43
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00473f43;
    }
L_0x00473f36:
    // 00473f36  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00473f37  66c7045ddcd36f000100   -mov word ptr [ebx*2 + 0x6fd3dc], 1
    app->getMemory<x86::reg16>(x86::reg32(7328732) /* 0x6fd3dc */ + cpu.ebx * 2) = 1 /*0x1*/;
    // 00473f41  ebee                   -jmp 0x473f31
    goto L_0x00473f31;
L_0x00473f43:
    // 00473f43  ba08000000             -mov edx, 8
    cpu.edx = 8 /*0x8*/;
    // 00473f48  b894d36f00             -mov eax, 0x6fd394
    cpu.eax = 7328660 /*0x6fd394*/;
    // 00473f4d  e8bac70600             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00473f52:
    // 00473f52  8b1d70d46f00           -mov ebx, dword ptr [0x6fd470]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7328880) /* 0x6fd470 */);
    // 00473f58  83fb01                 +cmp ebx, 1
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00473f5b  756a                   -jne 0x473fc7
    if (!cpu.flags.zf)
    {
        goto L_0x00473fc7;
    }
    // 00473f5d  a1bad36f00             -mov eax, dword ptr [0x6fd3ba]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328698) /* 0x6fd3ba */);
    // 00473f62  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00473f65  8d0c8500000000         -lea ecx, [eax*4]
    cpu.ecx = x86::reg32(cpu.eax * 4);
    // 00473f6c  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00473f6e  e8ed0c0000             -call 0x474c60
    cpu.esp -= 4;
    sub_474c60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473f73  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00473f75  01d9                   -add ecx, ebx
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.ebx));
    // 00473f77  bb02000000             -mov ebx, 2
    cpu.ebx = 2 /*0x2*/;
    // 00473f7c  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00473f7f  f7fb                   -idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 00473f81  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00473f83  8915c0d46f00           -mov dword ptr [0x6fd4c0], edx
    app->getMemory<x86::reg32>(x86::reg32(7328960) /* 0x6fd4c0 */) = cpu.edx;
    // 00473f89  e8d20c0000             -call 0x474c60
    cpu.esp -= 4;
    sub_474c60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473f8e  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00473f90  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00473f93  f7fb                   -idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 00473f95  41                     -inc ecx
    (cpu.ecx)++;
    // 00473f96  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00473f98  8915c4d46f00           -mov dword ptr [0x6fd4c4], edx
    app->getMemory<x86::reg32>(x86::reg32(7328964) /* 0x6fd4c4 */) = cpu.edx;
    // 00473f9e  e8bd0c0000             -call 0x474c60
    cpu.esp -= 4;
    sub_474c60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473fa3  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00473fa5  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00473fa8  f7fb                   -idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 00473faa  41                     -inc ecx
    (cpu.ecx)++;
    // 00473fab  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00473fad  8915c8d46f00           -mov dword ptr [0x6fd4c8], edx
    app->getMemory<x86::reg32>(x86::reg32(7328968) /* 0x6fd4c8 */) = cpu.edx;
    // 00473fb3  e8a80c0000             -call 0x474c60
    cpu.esp -= 4;
    sub_474c60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00473fb8  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00473fba  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00473fbd  f7fb                   +idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 00473fbf  8915ccd46f00           -mov dword ptr [0x6fd4cc], edx
    app->getMemory<x86::reg32>(x86::reg32(7328972) /* 0x6fd4cc */) = cpu.edx;
    // 00473fc5  eb1a                   -jmp 0x473fe1
    goto L_0x00473fe1;
L_0x00473fc7:
    // 00473fc7  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 00473fc9  8935c4d46f00           -mov dword ptr [0x6fd4c4], esi
    app->getMemory<x86::reg32>(x86::reg32(7328964) /* 0x6fd4c4 */) = cpu.esi;
    // 00473fcf  8935c8d46f00           -mov dword ptr [0x6fd4c8], esi
    app->getMemory<x86::reg32>(x86::reg32(7328968) /* 0x6fd4c8 */) = cpu.esi;
    // 00473fd5  8935ccd46f00           -mov dword ptr [0x6fd4cc], esi
    app->getMemory<x86::reg32>(x86::reg32(7328972) /* 0x6fd4cc */) = cpu.esi;
    // 00473fdb  8935c0d46f00           -mov dword ptr [0x6fd4c0], esi
    app->getMemory<x86::reg32>(x86::reg32(7328960) /* 0x6fd4c0 */) = cpu.esi;
L_0x00473fe1:
    // 00473fe1  bb02000000             -mov ebx, 2
    cpu.ebx = 2 /*0x2*/;
    // 00473fe6  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00473fe8  a1b0d36f00             -mov eax, dword ptr [0x6fd3b0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
    // 00473fed  890d14d56f00           -mov dword ptr [0x6fd514], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329044) /* 0x6fd514 */) = cpu.ecx;
    // 00473ff3  890d18d56f00           -mov dword ptr [0x6fd518], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */) = cpu.ecx;
    // 00473ff9  890d1cd56f00           -mov dword ptr [0x6fd51c], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329052) /* 0x6fd51c */) = cpu.ecx;
    // 00473fff  890d20d56f00           -mov dword ptr [0x6fd520], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */) = cpu.ecx;
    // 00474005  890d24d56f00           -mov dword ptr [0x6fd524], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329060) /* 0x6fd524 */) = cpu.ecx;
    // 0047400b  890d28d56f00           -mov dword ptr [0x6fd528], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */) = cpu.ecx;
    // 00474011  891db4d46f00           -mov dword ptr [0x6fd4b4], ebx
    app->getMemory<x86::reg32>(x86::reg32(7328948) /* 0x6fd4b4 */) = cpu.ebx;
    // 00474017  894dfc                 -mov dword ptr [ebp - 4], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
    // 0047401a  890d10d56f00           -mov dword ptr [0x6fd510], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329040) /* 0x6fd510 */) = cpu.ecx;
    // 00474020  83f804                 +cmp eax, 4
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00474023  0f87f0010000           -ja 0x474219
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x00474219;
    }
    // 00474029  ff2485c83e4700         -jmp dword ptr [eax*4 + 0x473ec8]
    cpu.ip = app->getMemory<x86::reg32>(4669128 + cpu.eax * 4); goto dynamic_jump;
  case 0x00474030:
    // 00474030  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00474033  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 0047403a  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 0047403c  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0047403e  893df4d46f00           -mov dword ptr [0x6fd4f4], edi
    app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */) = cpu.edi;
    // 00474044  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00474047  893df8d46f00           -mov dword ptr [0x6fd4f8], edi
    app->getMemory<x86::reg32>(x86::reg32(7329016) /* 0x6fd4f8 */) = cpu.edi;
    // 0047404d  8d3cc500000000         -lea edi, [eax*8]
    cpu.edi = x86::reg32(cpu.eax * 8);
    // 00474054  be2cd56f00             -mov esi, 0x6fd52c
    cpu.esi = 7329068 /*0x6fd52c*/;
    // 00474059  01c7                   -add edi, eax
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.eax));
    // 0047405b  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0047405d  01fe                   -add esi, edi
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.edi));
    // 0047405f  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 00474061  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00474063  e8a8f6ffff             -call 0x473710
    cpu.esp -= 4;
    sub_473710(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474068  898f3cd56f00           -mov dword ptr [edi + 0x6fd53c], ecx
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(7329084) /* 0x6fd53c */) = cpu.ecx;
    // 0047406e  66833c4dded36f0000     +cmp word ptr [ecx*2 + 0x6fd3de], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(x86::reg32(7328734) /* 0x6fd3de */ + cpu.ecx * 2);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 00474077  750e                   -jne 0x474087
    if (!cpu.flags.zf)
    {
        goto L_0x00474087;
    }
    // 00474079  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0047407c  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0047407e  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0047407f  e83cf6ffff             -call 0x4736c0
    cpu.esp -= 4;
    sub_4736c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474084  8955f8                 -mov dword ptr [ebp - 8], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.edx;
L_0x00474087:
    // 00474087  8b5dfc                 -mov ebx, dword ptr [ebp - 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0047408a  8b3518d56f00           -mov esi, dword ptr [0x6fd518]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 00474090  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474091  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474092  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474093  893518d56f00           -mov dword ptr [0x6fd518], esi
    app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */) = cpu.esi;
    // 00474099  895dfc                 -mov dword ptr [ebp - 4], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ebx;
    // 0047409c  e978010000             -jmp 0x474219
    goto L_0x00474219;
  case 0x004740a1:
    // 004740a1  b82cd56f00             -mov eax, 0x6fd52c
    cpu.eax = 7329068 /*0x6fd52c*/;
    // 004740a6  890df4d46f00           -mov dword ptr [0x6fd4f4], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */) = cpu.ecx;
    // 004740ac  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004740ae  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004740b0  890df8d46f00           -mov dword ptr [0x6fd4f8], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329016) /* 0x6fd4f8 */) = cpu.ecx;
    // 004740b6  e855f6ffff             -call 0x473710
    cpu.esp -= 4;
    sub_473710(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004740bb  66a1ded36f00           -mov ax, word ptr [0x6fd3de]
    cpu.ax = app->getMemory<x86::reg16>(x86::reg32(7328734) /* 0x6fd3de */);
    // 004740c1  890d3cd56f00           -mov dword ptr [0x6fd53c], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329084) /* 0x6fd53c */) = cpu.ecx;
    // 004740c7  6685c0                 +test ax, ax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg16>(cpu.ax & cpu.ax));
    // 004740ca  7511                   -jne 0x4740dd
    if (!cpu.flags.zf)
    {
        goto L_0x004740dd;
    }
    // 004740cc  8b5df8                 -mov ebx, dword ptr [ebp - 8]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004740cf  b82cd56f00             -mov eax, 0x6fd52c
    cpu.eax = 7329068 /*0x6fd52c*/;
    // 004740d4  43                     -inc ebx
    (cpu.ebx)++;
    // 004740d5  e8e6f5ffff             -call 0x4736c0
    cpu.esp -= 4;
    sub_4736c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004740da  895df8                 -mov dword ptr [ebp - 8], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.ebx;
L_0x004740dd:
    // 004740dd  8b75fc                 -mov esi, dword ptr [ebp - 4]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004740e0  46                     -inc esi
    (cpu.esi)++;
    // 004740e1  8975fc                 -mov dword ptr [ebp - 4], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.esi;
    // 004740e4  8d04b500000000         -lea eax, [esi*4]
    cpu.eax = x86::reg32(cpu.esi * 4);
    // 004740eb  29f0                   -sub eax, esi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.esi));
    // 004740ed  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 004740f2  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004740f5  41                     -inc ecx
    (cpu.ecx)++;
    // 004740f6  8d3cc500000000         -lea edi, [eax*8]
    cpu.edi = x86::reg32(cpu.eax * 8);
    // 004740fd  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 004740ff  01c7                   -add edi, eax
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.eax));
    // 00474101  a118d56f00             -mov eax, dword ptr [0x6fd518]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 00474106  be2cd56f00             -mov esi, 0x6fd52c
    cpu.esi = 7329068 /*0x6fd52c*/;
    // 0047410b  40                     -inc eax
    (cpu.eax)++;
    // 0047410c  01fe                   -add esi, edi
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.edi));
    // 0047410e  a318d56f00             -mov dword ptr [0x6fd518], eax
    app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */) = cpu.eax;
    // 00474113  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00474115  e8f6f5ffff             -call 0x473710
    cpu.esp -= 4;
    sub_473710(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047411a  898f3cd56f00           -mov dword ptr [edi + 0x6fd53c], ecx
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(7329084) /* 0x6fd53c */) = cpu.ecx;
    // 00474120  66833c4dded36f0000     +cmp word ptr [ecx*2 + 0x6fd3de], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(x86::reg32(7328734) /* 0x6fd3de */ + cpu.ecx * 2);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 00474129  750e                   -jne 0x474139
    if (!cpu.flags.zf)
    {
        goto L_0x00474139;
    }
    // 0047412b  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0047412e  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00474130  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474131  e88af5ffff             -call 0x4736c0
    cpu.esp -= 4;
    sub_4736c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474136  8955f8                 -mov dword ptr [ebp - 8], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.edx;
L_0x00474139:
    // 00474139  8b5dfc                 -mov ebx, dword ptr [ebp - 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0047413c  8b3518d56f00           -mov esi, dword ptr [0x6fd518]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 00474142  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474143  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474144  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474145  893518d56f00           -mov dword ptr [0x6fd518], esi
    app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */) = cpu.esi;
    // 0047414b  895dfc                 -mov dword ptr [ebp - 4], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ebx;
    // 0047414e  e9c6000000             -jmp 0x474219
    goto L_0x00474219;
  case 0x00474153:
    // 00474153  a176227a00             -mov eax, dword ptr [0x7a2276]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8004214) /* 0x7a2276 */);
    // 00474158  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0047415b  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0047415d  a3f4d46f00             -mov dword ptr [0x6fd4f4], eax
    app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */) = cpu.eax;
    // 00474162  a3f8d46f00             -mov dword ptr [0x6fd4f8], eax
    app->getMemory<x86::reg32>(x86::reg32(7329016) /* 0x6fd4f8 */) = cpu.eax;
L_0x00474167:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00474167  a174227a00             -mov eax, dword ptr [0x7a2274]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8004212) /* 0x7a2274 */);
    // 0047416c  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0047416f  39c3                   +cmp ebx, eax
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00474171  0f8d9d000000           -jge 0x474214
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00474214;
    }
    // 00474177  0fbfc3                 -movsx eax, bx
    cpu.eax = x86::reg32(static_cast<x86::sreg16>(cpu.bx));
    // 0047417a  e811250300             -call 0x4a6690
    cpu.esp -= 4;
    nfs3hp::ea::playerMachine(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047417f  98                     -cwde 
    cpu.eax = x86::reg32(static_cast<x86::sreg16>(cpu.ax));
    // 00474180  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
    // 00474183  83f8ff                 +cmp eax, -1
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(-1 /*-0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00474186  0f8482000000           -je 0x47420e
    if (cpu.flags.zf)
    {
        goto L_0x0047420e;
    }
    // 0047418c  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0047418f  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00474196  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00474198  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 0047419b  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0047419d  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 004741a0  b907000000             -mov ecx, 7
    cpu.ecx = 7 /*0x7*/;
    // 004741a5  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004741a7  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004741ac  be24d05500             -mov esi, 0x55d024
    cpu.esi = 5623844 /*0x55d024*/;
    // 004741b1  898244d56f00           -mov dword ptr [edx + 0x6fd544], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7329092) /* 0x6fd544 */) = cpu.eax;
    // 004741b7  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004741ba  8dba6cd56f00           -lea edi, [edx + 0x6fd56c]
    cpu.edi = x86::reg32(cpu.edx + x86::reg32(7329132) /* 0x6fd56c */);
    // 004741c0  89823cd56f00           -mov dword ptr [edx + 0x6fd53c], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7329084) /* 0x6fd53c */) = cpu.eax;
    // 004741c6  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
    while (cpu.ecx)
    {
        app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
        if (cpu.flags.df)
        {
            cpu.edi -= 4;
            cpu.esi -= 4;
        }
        else
        {
            cpu.edi += 4;
            cpu.esi += 4;
        }
        --cpu.ecx;
    }
    // 004741c8  8b8240d56f00           -mov eax, dword ptr [edx + 0x6fd540]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7329088) /* 0x6fd540 */);
    // 004741ce  e8cd53fcff             -call 0x4395a0
    cpu.esp -= 4;
    sub_4395a0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004741d3  898294d56f00           -mov dword ptr [edx + 0x6fd594], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7329172) /* 0x6fd594 */) = cpu.eax;
    // 004741d9  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004741dc  66833c45ded36f0000     +cmp word ptr [eax*2 + 0x6fd3de], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(x86::reg32(7328734) /* 0x6fd3de */ + cpu.eax * 2);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004741e5  7513                   -jne 0x4741fa
    if (!cpu.flags.zf)
    {
        goto L_0x004741fa;
    }
    // 004741e7  b82cd56f00             -mov eax, 0x6fd52c
    cpu.eax = 7329068 /*0x6fd52c*/;
    // 004741ec  01d0                   +add eax, edx
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004741ee  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004741f1  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004741f2  e8c9f4ffff             -call 0x4736c0
    cpu.esp -= 4;
    sub_4736c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004741f7  8955f8                 -mov dword ptr [ebp - 8], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.edx;
L_0x004741fa:
    // 004741fa  8b3518d56f00           -mov esi, dword ptr [0x6fd518]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 00474200  8b4dfc                 -mov ecx, dword ptr [ebp - 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00474203  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474204  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474205  893518d56f00           -mov dword ptr [0x6fd518], esi
    app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */) = cpu.esi;
    // 0047420b  894dfc                 -mov dword ptr [ebp - 4], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
L_0x0047420e:
    // 0047420e  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0047420f  e953ffffff             -jmp 0x474167
    goto L_0x00474167;
L_0x00474214:
    // 00474214  b908000000             -mov ecx, 8
    cpu.ecx = 8 /*0x8*/;
L_0x00474219:
    // 00474219  bf08000000             -mov edi, 8
    cpu.edi = 8 /*0x8*/;
    // 0047421e  a118d56f00             -mov eax, dword ptr [0x6fd518]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 00474223  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00474225  29c7                   -sub edi, eax
    (cpu.edi) -= x86::reg32(x86::sreg32(cpu.eax));
L_0x00474227:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00474227  39fb                   +cmp ebx, edi
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edi));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00474229  7d56                   -jge 0x474281
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00474281;
    }
    // 0047422b  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0047422e  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00474235  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00474237  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 0047423a  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0047423c  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 0047423f  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00474241  668b344dded36f00       -mov si, word ptr [ecx*2 + 0x6fd3de]
    cpu.si = app->getMemory<x86::reg16>(x86::reg32(7328734) /* 0x6fd3de */ + cpu.ecx * 2);
    // 00474249  052cd56f00             -add eax, 0x6fd52c
    (cpu.eax) += x86::reg32(x86::sreg32(7329068 /*0x6fd52c*/));
    // 0047424e  6685f6                 +test si, si
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg16>(cpu.si & cpu.si));
    // 00474251  7511                   -jne 0x474264
    if (!cpu.flags.zf)
    {
        goto L_0x00474264;
    }
    // 00474253  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 00474255  e806f7ffff             -call 0x473960
    cpu.esp -= 4;
    sub_473960(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047425a  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0047425d  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0047425e  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0047425f  8955f8                 -mov dword ptr [ebp - 8], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.edx;
    // 00474262  eb1a                   -jmp 0x47427e
    goto L_0x0047427e;
L_0x00474264:
    // 00474264  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 00474266  8b75fc                 -mov esi, dword ptr [ebp - 4]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00474269  e8f2f6ffff             -call 0x473960
    cpu.esp -= 4;
    sub_473960(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047426e  a120d56f00             -mov eax, dword ptr [0x6fd520]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */);
    // 00474273  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474274  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474275  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474276  8975fc                 -mov dword ptr [ebp - 4], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.esi;
    // 00474279  a320d56f00             -mov dword ptr [0x6fd520], eax
    app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */) = cpu.eax;
L_0x0047427e:
    // 0047427e  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0047427f  eba6                   -jmp 0x474227
    goto L_0x00474227;
L_0x00474281:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00474281  a1bad36f00             -mov eax, dword ptr [0x6fd3ba]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328698) /* 0x6fd3ba */);
    // 00474286  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00474289  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0047428c  39d0                   +cmp eax, edx
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0047428e  7e1d                   -jle 0x4742ad
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004742ad;
    }
    // 00474290  8b75fc                 -mov esi, dword ptr [ebp - 4]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00474293  8b3d20d56f00           -mov edi, dword ptr [0x6fd520]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */);
    // 00474299  8d5a01                 -lea ebx, [edx + 1]
    cpu.ebx = x86::reg32(cpu.edx + x86::reg32(1) /* 0x1 */);
    // 0047429c  49                     +dec ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 0047429d  4e                     +dec esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 0047429e  4f                     +dec edi
    {
        x86::reg32& tmp = cpu.edi;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 0047429f  895df8                 -mov dword ptr [ebp - 8], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.ebx;
    // 004742a2  8975fc                 -mov dword ptr [ebp - 4], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.esi;
    // 004742a5  893d20d56f00           -mov dword ptr [0x6fd520], edi
    app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */) = cpu.edi;
    // 004742ab  ebd4                   -jmp 0x474281
    goto L_0x00474281;
L_0x004742ad:
    // 004742ad  a118d56f00             -mov eax, dword ptr [0x6fd518]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 004742b2  8b1520d56f00           -mov edx, dword ptr [0x6fd520]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */);
    // 004742b8  a314d56f00             -mov dword ptr [0x6fd514], eax
    app->getMemory<x86::reg32>(x86::reg32(7329044) /* 0x6fd514 */) = cpu.eax;
    // 004742bd  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004742bf  babed36f00             -mov edx, 0x6fd3be
    cpu.edx = 7328702 /*0x6fd3be*/;
    // 004742c4  a310d56f00             -mov dword ptr [0x6fd510], eax
    app->getMemory<x86::reg32>(x86::reg32(7329040) /* 0x6fd510 */) = cpu.eax;
    // 004742c9  e8a2f8ffff             -call 0x473b70
    cpu.esp -= 4;
    sub_473b70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004742ce  a1bad36f00             -mov eax, dword ptr [0x6fd3ba]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328698) /* 0x6fd3ba */);
    // 004742d3  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 004742d6  8b0445fcd36f00         -mov eax, dword ptr [eax*2 + 0x6fd3fc]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328764) /* 0x6fd3fc */ + cpu.eax * 2);
    // 004742dd  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 004742df  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 004742e2  890dfcd46f00           -mov dword ptr [0x6fd4fc], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329020) /* 0x6fd4fc */) = cpu.ecx;
    // 004742e8  890d00d56f00           -mov dword ptr [0x6fd500], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329024) /* 0x6fd500 */) = cpu.ecx;
    // 004742ee  a3acd46f00             -mov dword ptr [0x6fd4ac], eax
    app->getMemory<x86::reg32>(x86::reg32(7328940) /* 0x6fd4ac */) = cpu.eax;
    // 004742f3  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004742f5  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004742f6  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004742f7  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004742f8  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004742f9  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004742fa  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004742fb  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* align: skip  */
void Application::sub_474310(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 00474310  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00474311  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00474312  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00474313  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00474314  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00474315  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00474316  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00474318  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0047431b  a10ad46f00             -mov eax, dword ptr [0x6fd40a]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328778) /* 0x6fd40a */);
    // 00474320  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 00474325  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00474328  8915f0d46f00           -mov dword ptr [0x6fd4f0], edx
    app->getMemory<x86::reg32>(x86::reg32(7329008) /* 0x6fd4f0 */) = cpu.edx;
    // 0047432e  8b048504d05500         -mov eax, dword ptr [eax*4 + 0x55d004]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5623812) /* 0x55d004 */ + cpu.eax * 4);
    // 00474335  ba1c000000             -mov edx, 0x1c
    cpu.edx = 28 /*0x1c*/;
    // 0047433a  a3acd46f00             -mov dword ptr [0x6fd4ac], eax
    app->getMemory<x86::reg32>(x86::reg32(7328940) /* 0x6fd4ac */) = cpu.eax;
    // 0047433f  b890d46f00             -mov eax, 0x6fd490
    cpu.eax = 7328912 /*0x6fd490*/;
    // 00474344  e8c3c30600             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474349  66833d0cd46f0000       +cmp word ptr [0x6fd40c], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(x86::reg32(7328780) /* 0x6fd40c */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 00474351  7523                   -jne 0x474376
    if (!cpu.flags.zf)
    {
        goto L_0x00474376;
    }
    // 00474353  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 00474355  eb05                   -jmp 0x47435c
    goto L_0x0047435c;
L_0x00474357:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00474357  83fb10                 +cmp ebx, 0x10
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(16 /*0x10*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0047435a  7f0b                   -jg 0x474367
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00474367;
    }
L_0x0047435c:
    // 0047435c  66891c5d0ed46f00       -mov word ptr [ebx*2 + 0x6fd40e], bx
    app->getMemory<x86::reg16>(x86::reg32(7328782) /* 0x6fd40e */ + cpu.ebx * 2) = cpu.bx;
    // 00474364  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474365  ebf0                   -jmp 0x474357
    goto L_0x00474357;
L_0x00474367:
    // 00474367  ba08000000             -mov edx, 8
    cpu.edx = 8 /*0x8*/;
    // 0047436c  b894d36f00             -mov eax, 0x6fd394
    cpu.eax = 7328660 /*0x6fd394*/;
    // 00474371  e896c30600             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00474376:
    // 00474376  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00474378  8b1db0d36f00           -mov ebx, dword ptr [0x6fd3b0]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
    // 0047437e  890d14d56f00           -mov dword ptr [0x6fd514], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329044) /* 0x6fd514 */) = cpu.ecx;
    // 00474384  890d18d56f00           -mov dword ptr [0x6fd518], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */) = cpu.ecx;
    // 0047438a  890d1cd56f00           -mov dword ptr [0x6fd51c], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329052) /* 0x6fd51c */) = cpu.ecx;
    // 00474390  890d20d56f00           -mov dword ptr [0x6fd520], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */) = cpu.ecx;
    // 00474396  890d24d56f00           -mov dword ptr [0x6fd524], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329060) /* 0x6fd524 */) = cpu.ecx;
    // 0047439c  890d28d56f00           -mov dword ptr [0x6fd528], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */) = cpu.ecx;
    // 004743a2  890d10d56f00           -mov dword ptr [0x6fd510], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329040) /* 0x6fd510 */) = cpu.ecx;
    // 004743a8  83fb01                 +cmp ebx, 1
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004743ab  7e0c                   -jle 0x4743b9
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004743b9;
    }
    // 004743ad  c705b4d46f0002000000   -mov dword ptr [0x6fd4b4], 2
    app->getMemory<x86::reg32>(x86::reg32(7328948) /* 0x6fd4b4 */) = 2 /*0x2*/;
    // 004743b7  eb0a                   -jmp 0x4743c3
    goto L_0x004743c3;
L_0x004743b9:
    // 004743b9  c705b4d46f0004000000   -mov dword ptr [0x6fd4b4], 4
    app->getMemory<x86::reg32>(x86::reg32(7328948) /* 0x6fd4b4 */) = 4 /*0x4*/;
L_0x004743c3:
    // 004743c3  833d70d46f0001         +cmp dword ptr [0x6fd470], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328880) /* 0x6fd470 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004743ca  7569                   -jne 0x474435
    if (!cpu.flags.zf)
    {
        goto L_0x00474435;
    }
    // 004743cc  a10ad46f00             -mov eax, dword ptr [0x6fd40a]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328778) /* 0x6fd40a */);
    // 004743d1  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 004743d4  8d0c8500000000         -lea ecx, [eax*4]
    cpu.ecx = x86::reg32(cpu.eax * 4);
    // 004743db  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004743dd  e87e080000             -call 0x474c60
    cpu.esp -= 4;
    sub_474c60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004743e2  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004743e4  bb02000000             -mov ebx, 2
    cpu.ebx = 2 /*0x2*/;
    // 004743e9  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004743ec  f7fb                   -idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004743ee  41                     -inc ecx
    (cpu.ecx)++;
    // 004743ef  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004743f1  8915c0d46f00           -mov dword ptr [0x6fd4c0], edx
    app->getMemory<x86::reg32>(x86::reg32(7328960) /* 0x6fd4c0 */) = cpu.edx;
    // 004743f7  e864080000             -call 0x474c60
    cpu.esp -= 4;
    sub_474c60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004743fc  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004743fe  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00474401  f7fb                   -idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 00474403  41                     -inc ecx
    (cpu.ecx)++;
    // 00474404  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00474406  8915c4d46f00           -mov dword ptr [0x6fd4c4], edx
    app->getMemory<x86::reg32>(x86::reg32(7328964) /* 0x6fd4c4 */) = cpu.edx;
    // 0047440c  e84f080000             -call 0x474c60
    cpu.esp -= 4;
    sub_474c60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474411  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00474413  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00474416  f7fb                   -idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 00474418  41                     -inc ecx
    (cpu.ecx)++;
    // 00474419  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0047441b  8915c8d46f00           -mov dword ptr [0x6fd4c8], edx
    app->getMemory<x86::reg32>(x86::reg32(7328968) /* 0x6fd4c8 */) = cpu.edx;
    // 00474421  e83a080000             -call 0x474c60
    cpu.esp -= 4;
    sub_474c60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474426  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00474428  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0047442b  f7fb                   +idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 0047442d  8915ccd46f00           -mov dword ptr [0x6fd4cc], edx
    app->getMemory<x86::reg32>(x86::reg32(7328972) /* 0x6fd4cc */) = cpu.edx;
    // 00474433  eb1a                   -jmp 0x47444f
    goto L_0x0047444f;
L_0x00474435:
    // 00474435  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00474437  8915c4d46f00           -mov dword ptr [0x6fd4c4], edx
    app->getMemory<x86::reg32>(x86::reg32(7328964) /* 0x6fd4c4 */) = cpu.edx;
    // 0047443d  8915c8d46f00           -mov dword ptr [0x6fd4c8], edx
    app->getMemory<x86::reg32>(x86::reg32(7328968) /* 0x6fd4c8 */) = cpu.edx;
    // 00474443  8915ccd46f00           -mov dword ptr [0x6fd4cc], edx
    app->getMemory<x86::reg32>(x86::reg32(7328972) /* 0x6fd4cc */) = cpu.edx;
    // 00474449  8915c0d46f00           -mov dword ptr [0x6fd4c0], edx
    app->getMemory<x86::reg32>(x86::reg32(7328960) /* 0x6fd4c0 */) = cpu.edx;
L_0x0047444f:
    // 0047444f  a1b0d36f00             -mov eax, dword ptr [0x6fd3b0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
    // 00474454  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 00474456  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00474458  897dfc                 -mov dword ptr [ebp - 4], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edi;
    // 0047445b  83f804                 +cmp eax, 4
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0047445e  0f873a010000           -ja 0x47459e
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x0047459e;
    }
    // 00474464  ff2485fc424700         -jmp dword ptr [eax*4 + 0x4742fc]
    cpu.ip = app->getMemory<x86::reg32>(4670204 + cpu.eax * 4); goto dynamic_jump;
  case 0x0047446b:
    // 0047446b  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0047446d  a3f4d46f00             -mov dword ptr [0x6fd4f4], eax
    app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */) = cpu.eax;
    // 00474472  a3f8d46f00             -mov dword ptr [0x6fd4f8], eax
    app->getMemory<x86::reg32>(x86::reg32(7329016) /* 0x6fd4f8 */) = cpu.eax;
    // 00474477  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0047447a  8d1c8500000000         -lea ebx, [eax*4]
    cpu.ebx = x86::reg32(cpu.eax * 4);
    // 00474481  29c3                   -sub ebx, eax
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 00474483  c1e302                 -shl ebx, 2
    cpu.ebx <<= 2 /*0x2*/ % 32;
    // 00474486  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 00474488  c1e303                 -shl ebx, 3
    cpu.ebx <<= 3 /*0x3*/ % 32;
    // 0047448b  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 0047448d  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 0047448f  052cd56f00             -add eax, 0x6fd52c
    (cpu.eax) += x86::reg32(x86::sreg32(7329068 /*0x6fd52c*/));
    // 00474494  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 00474496  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474497  e874f2ffff             -call 0x473710
    cpu.esp -= 4;
    sub_473710(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047449c  8b5dfc                 -mov ebx, dword ptr [ebp - 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0047449f  8b3518d56f00           -mov esi, dword ptr [0x6fd518]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 004744a5  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004744a6  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004744a7  895dfc                 -mov dword ptr [ebp - 4], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ebx;
    // 004744aa  893518d56f00           -mov dword ptr [0x6fd518], esi
    app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */) = cpu.esi;
    // 004744b0  e9e9000000             -jmp 0x47459e
    goto L_0x0047459e;
  case 0x004744b5:
    // 004744b5  b82cd56f00             -mov eax, 0x6fd52c
    cpu.eax = 7329068 /*0x6fd52c*/;
    // 004744ba  893df4d46f00           -mov dword ptr [0x6fd4f4], edi
    app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */) = cpu.edi;
    // 004744c0  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004744c2  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004744c4  893df8d46f00           -mov dword ptr [0x6fd4f8], edi
    app->getMemory<x86::reg32>(x86::reg32(7329016) /* 0x6fd4f8 */) = cpu.edi;
    // 004744ca  e841f2ffff             -call 0x473710
    cpu.esp -= 4;
    sub_473710(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004744cf  b82cd56f00             -mov eax, 0x6fd52c
    cpu.eax = 7329068 /*0x6fd52c*/;
    // 004744d4  8b1518d56f00           -mov edx, dword ptr [0x6fd518]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 004744da  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 004744df  42                     -inc edx
    (cpu.edx)++;
    // 004744e0  83c06c                 +add eax, 0x6c
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(108 /*0x6c*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004744e3  891518d56f00           -mov dword ptr [0x6fd518], edx
    app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */) = cpu.edx;
    // 004744e9  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 004744eb  e820f2ffff             -call 0x473710
    cpu.esp -= 4;
    sub_473710(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004744f0  8b1d18d56f00           -mov ebx, dword ptr [0x6fd518]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 004744f6  b902000000             -mov ecx, 2
    cpu.ecx = 2 /*0x2*/;
    // 004744fb  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004744fc  894dfc                 -mov dword ptr [ebp - 4], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
    // 004744ff  891d18d56f00           -mov dword ptr [0x6fd518], ebx
    app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */) = cpu.ebx;
    // 00474505  e994000000             -jmp 0x47459e
    goto L_0x0047459e;
  case 0x0047450a:
    // 0047450a  a176227a00             -mov eax, dword ptr [0x7a2276]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8004214) /* 0x7a2276 */);
    // 0047450f  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00474512  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00474514  a3f4d46f00             -mov dword ptr [0x6fd4f4], eax
    app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */) = cpu.eax;
    // 00474519  a3f8d46f00             -mov dword ptr [0x6fd4f8], eax
    app->getMemory<x86::reg32>(x86::reg32(7329016) /* 0x6fd4f8 */) = cpu.eax;
L_0x0047451e:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0047451e  a174227a00             -mov eax, dword ptr [0x7a2274]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8004212) /* 0x7a2274 */);
    // 00474523  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00474526  39c3                   +cmp ebx, eax
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00474528  7d6f                   -jge 0x474599
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00474599;
    }
    // 0047452a  0fbfc3                 -movsx eax, bx
    cpu.eax = x86::reg32(static_cast<x86::sreg16>(cpu.bx));
    // 0047452d  e85e210300             -call 0x4a6690
    cpu.esp -= 4;
    nfs3hp::ea::playerMachine(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474532  0fbfc8                 -movsx ecx, ax
    cpu.ecx = x86::reg32(static_cast<x86::sreg16>(cpu.ax));
    // 00474535  83f9ff                 +cmp ecx, -1
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(-1 /*-0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00474538  745c                   -je 0x474596
    if (cpu.flags.zf)
    {
        goto L_0x00474596;
    }
    // 0047453a  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0047453d  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00474544  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00474546  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00474549  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0047454b  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 0047454e  01c2                   +add edx, eax
    {
        x86::reg32& tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00474550  be01000000             -mov esi, 1
    cpu.esi = 1 /*0x1*/;
    // 00474555  8dba6cd56f00           -lea edi, [edx + 0x6fd56c]
    cpu.edi = x86::reg32(cpu.edx + x86::reg32(7329132) /* 0x6fd56c */);
    // 0047455b  898a3cd56f00           -mov dword ptr [edx + 0x6fd53c], ecx
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7329084) /* 0x6fd53c */) = cpu.ecx;
    // 00474561  b907000000             -mov ecx, 7
    cpu.ecx = 7 /*0x7*/;
    // 00474566  89b244d56f00           -mov dword ptr [edx + 0x6fd544], esi
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7329092) /* 0x6fd544 */) = cpu.esi;
    // 0047456c  be24d05500             -mov esi, 0x55d024
    cpu.esi = 5623844 /*0x55d024*/;
    // 00474571  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
    while (cpu.ecx)
    {
        app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
        if (cpu.flags.df)
        {
            cpu.edi -= 4;
            cpu.esi -= 4;
        }
        else
        {
            cpu.edi += 4;
            cpu.esi += 4;
        }
        --cpu.ecx;
    }
    // 00474573  8b8240d56f00           -mov eax, dword ptr [edx + 0x6fd540]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7329088) /* 0x6fd540 */);
    // 00474579  e82250fcff             -call 0x4395a0
    cpu.esp -= 4;
    sub_4395a0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047457e  898294d56f00           -mov dword ptr [edx + 0x6fd594], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7329172) /* 0x6fd594 */) = cpu.eax;
    // 00474584  8b7dfc                 -mov edi, dword ptr [ebp - 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00474587  a118d56f00             -mov eax, dword ptr [0x6fd518]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 0047458c  47                     +inc edi
    {
        x86::reg32& tmp = cpu.edi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0047458d  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0047458e  897dfc                 -mov dword ptr [ebp - 4], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edi;
    // 00474591  a318d56f00             -mov dword ptr [0x6fd518], eax
    app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */) = cpu.eax;
L_0x00474596:
    // 00474596  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474597  eb85                   -jmp 0x47451e
    goto L_0x0047451e;
L_0x00474599:
    // 00474599  b908000000             -mov ecx, 8
    cpu.ecx = 8 /*0x8*/;
L_0x0047459e:
    // 0047459e  be08000000             -mov esi, 8
    cpu.esi = 8 /*0x8*/;
    // 004745a3  8b1518d56f00           -mov edx, dword ptr [0x6fd518]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 004745a9  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004745ab  29d6                   -sub esi, edx
    (cpu.esi) -= x86::reg32(x86::sreg32(cpu.edx));
L_0x004745ad:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004745ad  39f3                   +cmp ebx, esi
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.esi));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004745af  7d38                   -jge 0x4745e9
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004745e9;
    }
    // 004745b1  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004745b4  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004745bb  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004745bd  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004745c0  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004745c2  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 004745c5  8b7dfc                 -mov edi, dword ptr [ebp - 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004745c8  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004745ca  43                     -inc ebx
    (cpu.ebx)++;
    // 004745cb  052cd56f00             +add eax, 0x6fd52c
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(7329068 /*0x6fd52c*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004745d0  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 004745d2  47                     +inc edi
    {
        x86::reg32& tmp = cpu.edi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004745d3  e888f3ffff             -call 0x473960
    cpu.esp -= 4;
    sub_473960(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004745d8  a120d56f00             -mov eax, dword ptr [0x6fd520]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */);
    // 004745dd  897dfc                 -mov dword ptr [ebp - 4], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edi;
    // 004745e0  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004745e1  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004745e2  a320d56f00             -mov dword ptr [0x6fd520], eax
    app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */) = cpu.eax;
    // 004745e7  ebc4                   -jmp 0x4745ad
    goto L_0x004745ad;
L_0x004745e9:
    // 004745e9  a118d56f00             -mov eax, dword ptr [0x6fd518]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 004745ee  8b1520d56f00           -mov edx, dword ptr [0x6fd520]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */);
    // 004745f4  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 004745f6  a314d56f00             -mov dword ptr [0x6fd514], eax
    app->getMemory<x86::reg32>(x86::reg32(7329044) /* 0x6fd514 */) = cpu.eax;
    // 004745fb  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004745fd  ba0ed46f00             -mov edx, 0x6fd40e
    cpu.edx = 7328782 /*0x6fd40e*/;
    // 00474602  a310d56f00             -mov dword ptr [0x6fd510], eax
    app->getMemory<x86::reg32>(x86::reg32(7329040) /* 0x6fd510 */) = cpu.eax;
    // 00474607  e864f5ffff             -call 0x473b70
    cpu.esp -= 4;
    sub_473b70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047460c  890dfcd46f00           -mov dword ptr [0x6fd4fc], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329020) /* 0x6fd4fc */) = cpu.ecx;
    // 00474612  890d00d56f00           -mov dword ptr [0x6fd500], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329024) /* 0x6fd500 */) = cpu.ecx;
    // 00474618  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0047461a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047461b  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047461c  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047461d  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047461e  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047461f  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00474620  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_474640(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 00474640  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00474641  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00474642  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00474643  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00474644  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00474645  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00474646  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00474648  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0047464b  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0047464d  8a252eeb5500           -mov ah, byte ptr [0x55eb2e]
    cpu.ah = app->getMemory<x86::reg8>(x86::reg32(5630766) /* 0x55eb2e */);
    // 00474653  891514d56f00           -mov dword ptr [0x6fd514], edx
    app->getMemory<x86::reg32>(x86::reg32(7329044) /* 0x6fd514 */) = cpu.edx;
    // 00474659  891518d56f00           -mov dword ptr [0x6fd518], edx
    app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */) = cpu.edx;
    // 0047465f  89151cd56f00           -mov dword ptr [0x6fd51c], edx
    app->getMemory<x86::reg32>(x86::reg32(7329052) /* 0x6fd51c */) = cpu.edx;
    // 00474665  891520d56f00           -mov dword ptr [0x6fd520], edx
    app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */) = cpu.edx;
    // 0047466b  891524d56f00           -mov dword ptr [0x6fd524], edx
    app->getMemory<x86::reg32>(x86::reg32(7329060) /* 0x6fd524 */) = cpu.edx;
    // 00474671  891528d56f00           -mov dword ptr [0x6fd528], edx
    app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */) = cpu.edx;
    // 00474677  891510d56f00           -mov dword ptr [0x6fd510], edx
    app->getMemory<x86::reg32>(x86::reg32(7329040) /* 0x6fd510 */) = cpu.edx;
    // 0047467d  f6c420                 +test ah, 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.ah & 32 /*0x20*/));
    // 00474680  7417                   -je 0x474699
    if (cpu.flags.zf)
    {
        goto L_0x00474699;
    }
    // 00474682  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00474684  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 00474689  e822dc0700             -call 0x4f22b0
    cpu.esp -= 4;
    nfs3hp::watcom::time(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047468e  e88d6b0700             -call 0x4eb220
    cpu.esp -= 4;
    nfs3hp::watcom::srand(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474693  890df0d46f00           -mov dword ptr [0x6fd4f0], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329008) /* 0x6fd4f0 */) = cpu.ecx;
L_0x00474699:
    // 00474699  ba08000000             -mov edx, 8
    cpu.edx = 8 /*0x8*/;
    // 0047469e  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004746a0  a18cd46f00             -mov eax, dword ptr [0x6fd48c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328908) /* 0x6fd48c */);
    // 004746a5  891d90d46f00           -mov dword ptr [0x6fd490], ebx
    app->getMemory<x86::reg32>(x86::reg32(7328912) /* 0x6fd490 */) = cpu.ebx;
    // 004746ab  891d94d46f00           -mov dword ptr [0x6fd494], ebx
    app->getMemory<x86::reg32>(x86::reg32(7328916) /* 0x6fd494 */) = cpu.ebx;
    // 004746b1  891d98d46f00           -mov dword ptr [0x6fd498], ebx
    app->getMemory<x86::reg32>(x86::reg32(7328920) /* 0x6fd498 */) = cpu.ebx;
    // 004746b7  891d9cd46f00           -mov dword ptr [0x6fd49c], ebx
    app->getMemory<x86::reg32>(x86::reg32(7328924) /* 0x6fd49c */) = cpu.ebx;
    // 004746bd  891da0d46f00           -mov dword ptr [0x6fd4a0], ebx
    app->getMemory<x86::reg32>(x86::reg32(7328928) /* 0x6fd4a0 */) = cpu.ebx;
    // 004746c3  a3a8d46f00             -mov dword ptr [0x6fd4a8], eax
    app->getMemory<x86::reg32>(x86::reg32(7328936) /* 0x6fd4a8 */) = cpu.eax;
    // 004746c8  b894d36f00             -mov eax, 0x6fd394
    cpu.eax = 7328660 /*0x6fd394*/;
    // 004746cd  891da4d46f00           -mov dword ptr [0x6fd4a4], ebx
    app->getMemory<x86::reg32>(x86::reg32(7328932) /* 0x6fd4a4 */) = cpu.ebx;
    // 004746d3  e834c00600             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004746d8  a1b0d36f00             -mov eax, dword ptr [0x6fd3b0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
    // 004746dd  895dfc                 -mov dword ptr [ebp - 4], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ebx;
    // 004746e0  83f804                 +cmp eax, 4
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004746e3  0f872c040000           -ja 0x474b15
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x00474b15;
    }
    // 004746e9  ff248524464700         -jmp dword ptr [eax*4 + 0x474624]
    cpu.ip = app->getMemory<x86::reg32>(4671012 + cpu.eax * 4); goto dynamic_jump;
  case 0x004746f0:
    // 004746f0  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004746f3  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004746fa  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004746fc  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004746ff  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 00474701  8d0cc500000000         -lea ecx, [eax*8]
    cpu.ecx = x86::reg32(cpu.eax * 8);
    // 00474708  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0047470a  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0047470c  b82cd56f00             -mov eax, 0x6fd52c
    cpu.eax = 7329068 /*0x6fd52c*/;
    // 00474711  8935f4d46f00           -mov dword ptr [0x6fd4f4], esi
    app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */) = cpu.esi;
    // 00474717  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 00474719  8935f8d46f00           -mov dword ptr [0x6fd4f8], esi
    app->getMemory<x86::reg32>(x86::reg32(7329016) /* 0x6fd4f8 */) = cpu.esi;
    // 0047471f  e8ecefffff             -call 0x473710
    cpu.esp -= 4;
    sub_473710(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474724  a1bcd26f00             -mov eax, dword ptr [0x6fd2bc]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328444) /* 0x6fd2bc */);
    // 00474729  e8824ffcff             -call 0x4396b0
    cpu.esp -= 4;
    nfs3hp::load::carIsPolice(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047472e  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00474730  7470                   -je 0x4747a2
    if (cpu.flags.zf)
    {
        goto L_0x004747a2;
    }
    // 00474732  808944d56f0008         -or byte ptr [ecx + 0x6fd544], 8
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(7329092) /* 0x6fd544 */) |= x86::reg8(x86::sreg8(8 /*0x8*/));
    // 00474739  ba06000000             -mov edx, 6
    cpu.edx = 6 /*0x6*/;
    // 0047473e  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 00474743  8b1d1cd56f00           -mov ebx, dword ptr [0x6fd51c]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7329052) /* 0x6fd51c */);
    // 00474749  893524d56f00           -mov dword ptr [0x6fd524], esi
    app->getMemory<x86::reg32>(x86::reg32(7329060) /* 0x6fd524 */) = cpu.esi;
    // 0047474f  891520d56f00           -mov dword ptr [0x6fd520], edx
    app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */) = cpu.edx;
    // 00474755  890d28d56f00           -mov dword ptr [0x6fd528], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */) = cpu.ecx;
    // 0047475b  01cb                   -add ebx, ecx
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.ecx));
    // 0047475d  8a352eeb5500           -mov dh, byte ptr [0x55eb2e]
    cpu.dh = app->getMemory<x86::reg8>(x86::reg32(5630766) /* 0x55eb2e */);
    // 00474763  891d1cd56f00           -mov dword ptr [0x6fd51c], ebx
    app->getMemory<x86::reg32>(x86::reg32(7329052) /* 0x6fd51c */) = cpu.ebx;
    // 00474769  f6c620                 +test dh, 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.dh & 32 /*0x20*/));
    // 0047476c  741d                   -je 0x47478b
    if (cpu.flags.zf)
    {
        goto L_0x0047478b;
    }
    // 0047476e  bf03000000             -mov edi, 3
    cpu.edi = 3 /*0x3*/;
    // 00474773  be02000000             -mov esi, 2
    cpu.esi = 2 /*0x2*/;
    // 00474778  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0047477a  893d28d56f00           -mov dword ptr [0x6fd528], edi
    app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */) = cpu.edi;
    // 00474780  a3e8d46f00             -mov dword ptr [0x6fd4e8], eax
    app->getMemory<x86::reg32>(x86::reg32(7329000) /* 0x6fd4e8 */) = cpu.eax;
    // 00474785  893524d56f00           -mov dword ptr [0x6fd524], esi
    app->getMemory<x86::reg32>(x86::reg32(7329060) /* 0x6fd524 */) = cpu.esi;
L_0x0047478b:
    // 0047478b  a120d56f00             -mov eax, dword ptr [0x6fd520]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */);
    // 00474790  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 00474795  a3b8d46f00             -mov dword ptr [0x6fd4b8], eax
    app->getMemory<x86::reg32>(x86::reg32(7328952) /* 0x6fd4b8 */) = cpu.eax;
    // 0047479a  8915bcd46f00           -mov dword ptr [0x6fd4bc], edx
    app->getMemory<x86::reg32>(x86::reg32(7328956) /* 0x6fd4bc */) = cpu.edx;
    // 004747a0  eb6d                   -jmp 0x47480f
    goto L_0x0047480f;
L_0x004747a2:
    // 004747a2  bb06000000             -mov ebx, 6
    cpu.ebx = 6 /*0x6*/;
    // 004747a7  be04000000             -mov esi, 4
    cpu.esi = 4 /*0x4*/;
    // 004747ac  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 004747b1  8b3d18d56f00           -mov edi, dword ptr [0x6fd518]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 004747b7  891d24d56f00           -mov dword ptr [0x6fd524], ebx
    app->getMemory<x86::reg32>(x86::reg32(7329060) /* 0x6fd524 */) = cpu.ebx;
    // 004747bd  893528d56f00           -mov dword ptr [0x6fd528], esi
    app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */) = cpu.esi;
    // 004747c3  890d20d56f00           -mov dword ptr [0x6fd520], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */) = cpu.ecx;
    // 004747c9  01cf                   -add edi, ecx
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004747cb  8a1d2eeb5500           -mov bl, byte ptr [0x55eb2e]
    cpu.bl = app->getMemory<x86::reg8>(x86::reg32(5630766) /* 0x55eb2e */);
    // 004747d1  893d18d56f00           -mov dword ptr [0x6fd518], edi
    app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */) = cpu.edi;
    // 004747d7  f6c320                 +test bl, 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.bl & 32 /*0x20*/));
    // 004747da  741d                   -je 0x4747f9
    if (cpu.flags.zf)
    {
        goto L_0x004747f9;
    }
    // 004747dc  ba03000000             -mov edx, 3
    cpu.edx = 3 /*0x3*/;
    // 004747e1  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 004747e6  31c9                   +xor ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx))));
    // 004747e8  891528d56f00           -mov dword ptr [0x6fd528], edx
    app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */) = cpu.edx;
    // 004747ee  890de8d46f00           -mov dword ptr [0x6fd4e8], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329000) /* 0x6fd4e8 */) = cpu.ecx;
    // 004747f4  a324d56f00             -mov dword ptr [0x6fd524], eax
    app->getMemory<x86::reg32>(x86::reg32(7329060) /* 0x6fd524 */) = cpu.eax;
L_0x004747f9:
    // 004747f9  a1b4d46f00             -mov eax, dword ptr [0x6fd4b4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328948) /* 0x6fd4b4 */);
    // 004747fe  bb64000000             -mov ebx, 0x64
    cpu.ebx = 100 /*0x64*/;
    // 00474803  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474804  891db8d46f00           -mov dword ptr [0x6fd4b8], ebx
    app->getMemory<x86::reg32>(x86::reg32(7328952) /* 0x6fd4b8 */) = cpu.ebx;
    // 0047480a  a3bcd46f00             -mov dword ptr [0x6fd4bc], eax
    app->getMemory<x86::reg32>(x86::reg32(7328956) /* 0x6fd4bc */) = cpu.eax;
L_0x0047480f:
    // 0047480f  ff45fc                 +inc dword ptr [ebp - 4]
    {
        auto tmp = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474812  e9fe020000             -jmp 0x474b15
    goto L_0x00474b15;
  case 0x00474817:
    // 00474817  b82cd56f00             -mov eax, 0x6fd52c
    cpu.eax = 7329068 /*0x6fd52c*/;
    // 0047481c  891df4d46f00           -mov dword ptr [0x6fd4f4], ebx
    app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */) = cpu.ebx;
    // 00474822  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00474824  891df8d46f00           -mov dword ptr [0x6fd4f8], ebx
    app->getMemory<x86::reg32>(x86::reg32(7329016) /* 0x6fd4f8 */) = cpu.ebx;
    // 0047482a  e8e1eeffff             -call 0x473710
    cpu.esp -= 4;
    sub_473710(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047482f  a1bcd26f00             -mov eax, dword ptr [0x6fd2bc]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328444) /* 0x6fd2bc */);
    // 00474834  e8774efcff             -call 0x4396b0
    cpu.esp -= 4;
    nfs3hp::load::carIsPolice(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474839  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0047483b  741e                   -je 0x47485b
    if (cpu.flags.zf)
    {
        goto L_0x0047485b;
    }
    // 0047483d  8b151cd56f00           -mov edx, dword ptr [0x6fd51c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7329052) /* 0x6fd51c */);
    // 00474843  8a3d44d56f00           -mov bh, byte ptr [0x6fd544]
    cpu.bh = app->getMemory<x86::reg8>(x86::reg32(7329092) /* 0x6fd544 */);
    // 00474849  42                     -inc edx
    (cpu.edx)++;
    // 0047484a  80cf08                 +or bh, 8
    cpu.clear_co();
    cpu.set_szp((cpu.bh |= x86::reg8(x86::sreg8(8 /*0x8*/))));
    // 0047484d  89151cd56f00           -mov dword ptr [0x6fd51c], edx
    app->getMemory<x86::reg32>(x86::reg32(7329052) /* 0x6fd51c */) = cpu.edx;
    // 00474853  883d44d56f00           -mov byte ptr [0x6fd544], bh
    app->getMemory<x86::reg8>(x86::reg32(7329092) /* 0x6fd544 */) = cpu.bh;
    // 00474859  eb06                   -jmp 0x474861
    goto L_0x00474861;
L_0x0047485b:
    // 0047485b  ff0518d56f00           -inc dword ptr [0x6fd518]
    (app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */))++;
L_0x00474861:
    // 00474861  8b5dfc                 -mov ebx, dword ptr [ebp - 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00474864  43                     -inc ebx
    (cpu.ebx)++;
    // 00474865  895dfc                 -mov dword ptr [ebp - 4], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ebx;
    // 00474868  8d049d00000000         -lea eax, [ebx*4]
    cpu.eax = x86::reg32(cpu.ebx * 4);
    // 0047486f  29d8                   -sub eax, ebx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 00474871  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00474874  8d0cc500000000         -lea ecx, [eax*8]
    cpu.ecx = x86::reg32(cpu.eax * 8);
    // 0047487b  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0047487e  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00474880  b82cd56f00             -mov eax, 0x6fd52c
    cpu.eax = 7329068 /*0x6fd52c*/;
    // 00474885  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 0047488a  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 0047488c  e87feeffff             -call 0x473710
    cpu.esp -= 4;
    sub_473710(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474891  a128d36f00             -mov eax, dword ptr [0x6fd328]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328552) /* 0x6fd328 */);
    // 00474896  e8154efcff             -call 0x4396b0
    cpu.esp -= 4;
    nfs3hp::load::carIsPolice(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047489b  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0047489d  740f                   -je 0x4748ae
    if (cpu.flags.zf)
    {
        goto L_0x004748ae;
    }
    // 0047489f  808944d56f0008         +or byte ptr [ecx + 0x6fd544], 8
    cpu.clear_co();
    cpu.set_szp((app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(7329092) /* 0x6fd544 */) |= x86::reg8(x86::sreg8(8 /*0x8*/))));
    // 004748a6  ff051cd56f00           +inc dword ptr [0x6fd51c]
    {
        auto tmp = app->getMemory<x86::reg32>(x86::reg32(7329052) /* 0x6fd51c */);
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004748ac  eb06                   -jmp 0x4748b4
    goto L_0x004748b4;
L_0x004748ae:
    // 004748ae  ff0518d56f00           -inc dword ptr [0x6fd518]
    (app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */))++;
L_0x004748b4:
    // 004748b4  ff45fc                 -inc dword ptr [ebp - 4]
    (app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */))++;
    // 004748b7  a1b4d46f00             -mov eax, dword ptr [0x6fd4b4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328948) /* 0x6fd4b4 */);
    // 004748bc  8b1518d56f00           -mov edx, dword ptr [0x6fd518]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 004748c2  40                     -inc eax
    (cpu.eax)++;
    // 004748c3  83fa02                 +cmp edx, 2
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004748c6  7533                   -jne 0x4748fb
    if (!cpu.flags.zf)
    {
        goto L_0x004748fb;
    }
    // 004748c8  be04000000             -mov esi, 4
    cpu.esi = 4 /*0x4*/;
    // 004748cd  bf64000000             -mov edi, 0x64
    cpu.edi = 100 /*0x64*/;
    // 004748d2  b906000000             -mov ecx, 6
    cpu.ecx = 6 /*0x6*/;
    // 004748d7  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 004748d9  a3bcd46f00             -mov dword ptr [0x6fd4bc], eax
    app->getMemory<x86::reg32>(x86::reg32(7328956) /* 0x6fd4bc */) = cpu.eax;
    // 004748de  891d20d56f00           -mov dword ptr [0x6fd520], ebx
    app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */) = cpu.ebx;
    // 004748e4  893528d56f00           -mov dword ptr [0x6fd528], esi
    app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */) = cpu.esi;
    // 004748ea  893db8d46f00           -mov dword ptr [0x6fd4b8], edi
    app->getMemory<x86::reg32>(x86::reg32(7328952) /* 0x6fd4b8 */) = cpu.edi;
    // 004748f0  890d24d56f00           -mov dword ptr [0x6fd524], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329060) /* 0x6fd524 */) = cpu.ecx;
    // 004748f6  e91a020000             -jmp 0x474b15
    goto L_0x00474b15;
L_0x004748fb:
    // 004748fb  833d1cd56f0002         +cmp dword ptr [0x6fd51c], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7329052) /* 0x6fd51c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00474902  7533                   -jne 0x474937
    if (!cpu.flags.zf)
    {
        goto L_0x00474937;
    }
    // 00474904  bb06000000             -mov ebx, 6
    cpu.ebx = 6 /*0x6*/;
    // 00474909  bf03000000             -mov edi, 3
    cpu.edi = 3 /*0x3*/;
    // 0047490e  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00474913  31c9                   +xor ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx))));
    // 00474915  891d20d56f00           -mov dword ptr [0x6fd520], ebx
    app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */) = cpu.ebx;
    // 0047491b  890d28d56f00           -mov dword ptr [0x6fd528], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */) = cpu.ecx;
    // 00474921  893db8d46f00           -mov dword ptr [0x6fd4b8], edi
    app->getMemory<x86::reg32>(x86::reg32(7328952) /* 0x6fd4b8 */) = cpu.edi;
    // 00474927  a3bcd46f00             -mov dword ptr [0x6fd4bc], eax
    app->getMemory<x86::reg32>(x86::reg32(7328956) /* 0x6fd4bc */) = cpu.eax;
    // 0047492c  890d24d56f00           -mov dword ptr [0x6fd524], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329060) /* 0x6fd524 */) = cpu.ecx;
    // 00474932  e9de010000             -jmp 0x474b15
    goto L_0x00474b15;
L_0x00474937:
    // 00474937  bb06000000             -mov ebx, 6
    cpu.ebx = 6 /*0x6*/;
    // 0047493c  a3b8d46f00             -mov dword ptr [0x6fd4b8], eax
    app->getMemory<x86::reg32>(x86::reg32(7328952) /* 0x6fd4b8 */) = cpu.eax;
    // 00474941  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 00474943  a3bcd46f00             -mov dword ptr [0x6fd4bc], eax
    app->getMemory<x86::reg32>(x86::reg32(7328956) /* 0x6fd4bc */) = cpu.eax;
    // 00474948  891520d56f00           -mov dword ptr [0x6fd520], edx
    app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */) = cpu.edx;
    // 0047494e  891d28d56f00           -mov dword ptr [0x6fd528], ebx
    app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */) = cpu.ebx;
    // 00474954  891524d56f00           -mov dword ptr [0x6fd524], edx
    app->getMemory<x86::reg32>(x86::reg32(7329060) /* 0x6fd524 */) = cpu.edx;
    // 0047495a  e9b6010000             -jmp 0x474b15
    goto L_0x00474b15;
  case 0x0047495f:
    // 0047495f  a176227a00             -mov eax, dword ptr [0x7a2276]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8004214) /* 0x7a2276 */);
    // 00474964  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00474967  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00474969  a3f4d46f00             -mov dword ptr [0x6fd4f4], eax
    app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */) = cpu.eax;
    // 0047496e  a3f8d46f00             -mov dword ptr [0x6fd4f8], eax
    app->getMemory<x86::reg32>(x86::reg32(7329016) /* 0x6fd4f8 */) = cpu.eax;
L_0x00474973:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00474973  a174227a00             -mov eax, dword ptr [0x7a2274]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8004212) /* 0x7a2274 */);
    // 00474978  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0047497b  39c3                   +cmp ebx, eax
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0047497d  7d72                   -jge 0x4749f1
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004749f1;
    }
    // 0047497f  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00474982  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00474989  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0047498b  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 0047498e  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00474990  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 00474993  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00474995  be01000000             -mov esi, 1
    cpu.esi = 1 /*0x1*/;
    // 0047499a  b907000000             -mov ecx, 7
    cpu.ecx = 7 /*0x7*/;
    // 0047499f  8dba6cd56f00           -lea edi, [edx + 0x6fd56c]
    cpu.edi = x86::reg32(cpu.edx + x86::reg32(7329132) /* 0x6fd56c */);
    // 004749a5  89b244d56f00           -mov dword ptr [edx + 0x6fd544], esi
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7329092) /* 0x6fd544 */) = cpu.esi;
    // 004749ab  be24d05500             -mov esi, 0x55d024
    cpu.esi = 5623844 /*0x55d024*/;
    // 004749b0  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
    while (cpu.ecx)
    {
        app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
        if (cpu.flags.df)
        {
            cpu.edi -= 4;
            cpu.esi -= 4;
        }
        else
        {
            cpu.edi += 4;
            cpu.esi += 4;
        }
        --cpu.ecx;
    }
    // 004749b2  8b8240d56f00           -mov eax, dword ptr [edx + 0x6fd540]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7329088) /* 0x6fd540 */);
    // 004749b8  e8e34bfcff             -call 0x4395a0
    cpu.esp -= 4;
    sub_4395a0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004749bd  898294d56f00           -mov dword ptr [edx + 0x6fd594], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7329172) /* 0x6fd594 */) = cpu.eax;
    // 004749c3  8b8240d56f00           -mov eax, dword ptr [edx + 0x6fd540]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7329088) /* 0x6fd540 */);
    // 004749c9  e8e24cfcff             -call 0x4396b0
    cpu.esp -= 4;
    nfs3hp::load::carIsPolice(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004749ce  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004749d0  740f                   -je 0x4749e1
    if (cpu.flags.zf)
    {
        goto L_0x004749e1;
    }
    // 004749d2  808a44d56f0008         +or byte ptr [edx + 0x6fd544], 8
    cpu.clear_co();
    cpu.set_szp((app->getMemory<x86::reg8>(cpu.edx + x86::reg32(7329092) /* 0x6fd544 */) |= x86::reg8(x86::sreg8(8 /*0x8*/))));
    // 004749d9  ff051cd56f00           +inc dword ptr [0x6fd51c]
    {
        auto tmp = app->getMemory<x86::reg32>(x86::reg32(7329052) /* 0x6fd51c */);
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004749df  eb06                   -jmp 0x4749e7
    goto L_0x004749e7;
L_0x004749e1:
    // 004749e1  ff0518d56f00           +inc dword ptr [0x6fd518]
    {
        auto tmp = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
L_0x004749e7:
    // 004749e7  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004749ea  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004749eb  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004749ec  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 004749ef  eb82                   -jmp 0x474973
    goto L_0x00474973;
L_0x004749f1:
    // 004749f1  8b0d1cd56f00           -mov ecx, dword ptr [0x6fd51c]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7329052) /* 0x6fd51c */);
    // 004749f7  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004749f9  0f8562000000           -jne 0x474a61
    if (!cpu.flags.zf)
    {
        goto L_0x00474a61;
    }
    // 004749ff  833de8d46f0000         +cmp dword ptr [0x6fd4e8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7329000) /* 0x6fd4e8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00474a06  7515                   -jne 0x474a1d
    if (!cpu.flags.zf)
    {
        goto L_0x00474a1d;
    }
    // 00474a08  833d18d56f0004         +cmp dword ptr [0x6fd518], 4
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00474a0f  7d0c                   -jge 0x474a1d
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00474a1d;
    }
    // 00474a11  c70528d56f0001000000   -mov dword ptr [0x6fd528], 1
    app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */) = 1 /*0x1*/;
    // 00474a1b  eb07                   -jmp 0x474a24
    goto L_0x00474a24;
L_0x00474a1d:
    // 00474a1d  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00474a1f  a328d56f00             -mov dword ptr [0x6fd528], eax
    app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */) = cpu.eax;
L_0x00474a24:
    // 00474a24  ba08000000             -mov edx, 8
    cpu.edx = 8 /*0x8*/;
    // 00474a29  8b0d28d56f00           -mov ecx, dword ptr [0x6fd528]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */);
    // 00474a2f  a118d56f00             -mov eax, dword ptr [0x6fd518]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 00474a34  29ca                   -sub edx, ecx
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 00474a36  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 00474a38  891524d56f00           -mov dword ptr [0x6fd524], edx
    app->getMemory<x86::reg32>(x86::reg32(7329060) /* 0x6fd524 */) = cpu.edx;
    // 00474a3e  83fa06                 +cmp edx, 6
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(6 /*0x6*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00474a41  7e07                   -jle 0x474a4a
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00474a4a;
    }
    // 00474a43  b806000000             -mov eax, 6
    cpu.eax = 6 /*0x6*/;
    // 00474a48  eb02                   -jmp 0x474a4c
    goto L_0x00474a4c;
L_0x00474a4a:
    // 00474a4a  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
L_0x00474a4c:
    // 00474a4c  a324d56f00             -mov dword ptr [0x6fd524], eax
    app->getMemory<x86::reg32>(x86::reg32(7329060) /* 0x6fd524 */) = cpu.eax;
    // 00474a51  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00474a53  7f02                   -jg 0x474a57
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00474a57;
    }
    // 00474a55  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
L_0x00474a57:
    // 00474a57  a324d56f00             -mov dword ptr [0x6fd524], eax
    app->getMemory<x86::reg32>(x86::reg32(7329060) /* 0x6fd524 */) = cpu.eax;
    // 00474a5c  e9a4000000             -jmp 0x474b05
    goto L_0x00474b05;
L_0x00474a61:
    // 00474a61  8b3518d56f00           -mov esi, dword ptr [0x6fd518]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 00474a67  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 00474a69  0f8560000000           -jne 0x474acf
    if (!cpu.flags.zf)
    {
        goto L_0x00474acf;
    }
    // 00474a6f  833de8d46f0000         +cmp dword ptr [0x6fd4e8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7329000) /* 0x6fd4e8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00474a76  7511                   -jne 0x474a89
    if (!cpu.flags.zf)
    {
        goto L_0x00474a89;
    }
    // 00474a78  83f906                 +cmp ecx, 6
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(6 /*0x6*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00474a7b  7d0c                   -jge 0x474a89
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00474a89;
    }
    // 00474a7d  c70528d56f0001000000   -mov dword ptr [0x6fd528], 1
    app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */) = 1 /*0x1*/;
    // 00474a87  eb08                   -jmp 0x474a91
    goto L_0x00474a91;
L_0x00474a89:
    // 00474a89  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00474a8b  890d28d56f00           -mov dword ptr [0x6fd528], ecx
    app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */) = cpu.ecx;
L_0x00474a91:
    // 00474a91  ba08000000             -mov edx, 8
    cpu.edx = 8 /*0x8*/;
    // 00474a96  8b1d28d56f00           -mov ebx, dword ptr [0x6fd528]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */);
    // 00474a9c  a11cd56f00             -mov eax, dword ptr [0x6fd51c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7329052) /* 0x6fd51c */);
    // 00474aa1  29da                   -sub edx, ebx
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 00474aa3  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 00474aa5  891520d56f00           -mov dword ptr [0x6fd520], edx
    app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */) = cpu.edx;
    // 00474aab  83fa06                 +cmp edx, 6
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(6 /*0x6*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00474aae  7e07                   -jle 0x474ab7
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00474ab7;
    }
    // 00474ab0  b806000000             -mov eax, 6
    cpu.eax = 6 /*0x6*/;
    // 00474ab5  eb02                   -jmp 0x474ab9
    goto L_0x00474ab9;
L_0x00474ab7:
    // 00474ab7  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
L_0x00474ab9:
    // 00474ab9  a320d56f00             -mov dword ptr [0x6fd520], eax
    app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */) = cpu.eax;
    // 00474abe  83f801                 +cmp eax, 1
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00474ac1  7f05                   -jg 0x474ac8
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00474ac8;
    }
    // 00474ac3  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
L_0x00474ac8:
    // 00474ac8  a320d56f00             -mov dword ptr [0x6fd520], eax
    app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */) = cpu.eax;
    // 00474acd  eb36                   -jmp 0x474b05
    goto L_0x00474b05;
L_0x00474acf:
    // 00474acf  833de8d46f0000         +cmp dword ptr [0x6fd4e8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7329000) /* 0x6fd4e8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00474ad6  752d                   -jne 0x474b05
    if (!cpu.flags.zf)
    {
        goto L_0x00474b05;
    }
    // 00474ad8  ba08000000             -mov edx, 8
    cpu.edx = 8 /*0x8*/;
    // 00474add  29f2                   -sub edx, esi
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.esi));
    // 00474adf  29ca                   -sub edx, ecx
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 00474ae1  891528d56f00           -mov dword ptr [0x6fd528], edx
    app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */) = cpu.edx;
    // 00474ae7  83fa04                 +cmp edx, 4
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00474aea  7e07                   -jle 0x474af3
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00474af3;
    }
    // 00474aec  b804000000             -mov eax, 4
    cpu.eax = 4 /*0x4*/;
    // 00474af1  eb02                   -jmp 0x474af5
    goto L_0x00474af5;
L_0x00474af3:
    // 00474af3  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
L_0x00474af5:
    // 00474af5  a328d56f00             -mov dword ptr [0x6fd528], eax
    app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */) = cpu.eax;
    // 00474afa  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00474afc  7f02                   -jg 0x474b00
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00474b00;
    }
    // 00474afe  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x00474b00:
    // 00474b00  a328d56f00             -mov dword ptr [0x6fd528], eax
    app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */) = cpu.eax;
L_0x00474b05:
    // 00474b05  a1b4d46f00             -mov eax, dword ptr [0x6fd4b4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328948) /* 0x6fd4b4 */);
    // 00474b0a  40                     -inc eax
    (cpu.eax)++;
    // 00474b0b  a3b8d46f00             -mov dword ptr [0x6fd4b8], eax
    app->getMemory<x86::reg32>(x86::reg32(7328952) /* 0x6fd4b8 */) = cpu.eax;
    // 00474b10  a3bcd46f00             -mov dword ptr [0x6fd4bc], eax
    app->getMemory<x86::reg32>(x86::reg32(7328956) /* 0x6fd4bc */) = cpu.eax;
L_0x00474b15:
    // 00474b15  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
L_0x00474b17:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00474b17  3b0d20d56f00           +cmp ecx, dword ptr [0x6fd520]
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00474b1d  7d2d                   -jge 0x474b4c
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00474b4c;
    }
    // 00474b1f  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00474b22  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00474b29  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00474b2b  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00474b2e  8b75fc                 -mov esi, dword ptr [ebp - 4]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00474b31  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00474b33  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 00474b36  41                     -inc ecx
    (cpu.ecx)++;
    // 00474b37  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00474b39  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00474b3c  052cd56f00             +add eax, 0x6fd52c
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(7329068 /*0x6fd52c*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00474b41  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474b42  e819eeffff             -call 0x473960
    cpu.esp -= 4;
    sub_473960(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474b47  8975fc                 -mov dword ptr [ebp - 4], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.esi;
    // 00474b4a  ebcb                   -jmp 0x474b17
    goto L_0x00474b17;
L_0x00474b4c:
    // 00474b4c  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
L_0x00474b4e:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00474b4e  3b0d24d56f00           +cmp ecx, dword ptr [0x6fd524]
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7329060) /* 0x6fd524 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00474b54  7d5f                   -jge 0x474bb5
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00474bb5;
    }
    // 00474b56  f6052eeb550020         +test byte ptr [0x55eb2e], 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(x86::reg32(5630766) /* 0x55eb2e */) & 32 /*0x20*/));
    // 00474b5d  741b                   -je 0x474b7a
    if (cpu.flags.zf)
    {
        goto L_0x00474b7a;
    }
    // 00474b5f  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00474b62  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00474b69  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00474b6b  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00474b6e  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00474b70  c1e003                 +shl eax, 3
    {
        x86::reg8 tmp = 3 /*0x3*/ % 32;
        x86::reg32& op = cpu.eax;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 00474b73  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 00474b78  eb22                   -jmp 0x474b9c
    goto L_0x00474b9c;
L_0x00474b7a:
    // 00474b7a  f6c103                 +test cl, 3
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 3 /*0x3*/));
    // 00474b7d  7507                   -jne 0x474b86
    if (!cpu.flags.zf)
    {
        goto L_0x00474b86;
    }
    // 00474b7f  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 00474b84  eb02                   -jmp 0x474b88
    goto L_0x00474b88;
L_0x00474b86:
    // 00474b86  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
L_0x00474b88:
    // 00474b88  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00474b8b  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00474b92  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00474b94  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00474b97  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00474b99  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
L_0x00474b9c:
    // 00474b9c  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00474b9e  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00474ba1  052cd56f00             +add eax, 0x6fd52c
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(7329068 /*0x6fd52c*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00474ba6  e8e5ecffff             -call 0x473890
    cpu.esp -= 4;
    sub_473890(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474bab  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00474bae  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474baf  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474bb0  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 00474bb3  eb99                   -jmp 0x474b4e
    goto L_0x00474b4e;
L_0x00474bb5:
    // 00474bb5  833de8d46f0000         +cmp dword ptr [0x6fd4e8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7329000) /* 0x6fd4e8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00474bbc  7537                   -jne 0x474bf5
    if (!cpu.flags.zf)
    {
        goto L_0x00474bf5;
    }
    // 00474bbe  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
L_0x00474bc0:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00474bc0  3b0d28d56f00           +cmp ecx, dword ptr [0x6fd528]
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00474bc6  7d35                   -jge 0x474bfd
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00474bfd;
    }
    // 00474bc8  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00474bcb  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00474bd2  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00474bd4  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00474bd7  8b75fc                 -mov esi, dword ptr [ebp - 4]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00474bda  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00474bdc  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 00474bdf  41                     -inc ecx
    (cpu.ecx)++;
    // 00474be0  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00474be2  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00474be5  052cd56f00             +add eax, 0x6fd52c
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(7329068 /*0x6fd52c*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00474bea  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474beb  e880eeffff             -call 0x473a70
    cpu.esp -= 4;
    sub_473a70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474bf0  8975fc                 -mov dword ptr [ebp - 4], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.esi;
    // 00474bf3  ebcb                   -jmp 0x474bc0
    goto L_0x00474bc0;
L_0x00474bf5:
    // 00474bf5  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 00474bf7  893d28d56f00           -mov dword ptr [0x6fd528], edi
    app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */) = cpu.edi;
L_0x00474bfd:
    // 00474bfd  a118d56f00             -mov eax, dword ptr [0x6fd518]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 00474c02  8b151cd56f00           -mov edx, dword ptr [0x6fd51c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7329052) /* 0x6fd51c */);
    // 00474c08  8b0d24d56f00           -mov ecx, dword ptr [0x6fd524]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7329060) /* 0x6fd524 */);
    // 00474c0e  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00474c10  8b1d20d56f00           -mov ebx, dword ptr [0x6fd520]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7329056) /* 0x6fd520 */);
    // 00474c16  a314d56f00             -mov dword ptr [0x6fd514], eax
    app->getMemory<x86::reg32>(x86::reg32(7329044) /* 0x6fd514 */) = cpu.eax;
    // 00474c1b  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 00474c1d  8b1528d56f00           -mov edx, dword ptr [0x6fd528]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7329064) /* 0x6fd528 */);
    // 00474c23  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 00474c25  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00474c27  a310d56f00             -mov dword ptr [0x6fd510], eax
    app->getMemory<x86::reg32>(x86::reg32(7329040) /* 0x6fd510 */) = cpu.eax;
    // 00474c2c  a118d56f00             -mov eax, dword ptr [0x6fd518]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7329048) /* 0x6fd518 */);
    // 00474c31  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00474c33  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 00474c35  e836efffff             -call 0x473b70
    cpu.esp -= 4;
    sub_473b70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474c3a  8b3dfcd46f00           -mov edi, dword ptr [0x6fd4fc]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7329020) /* 0x6fd4fc */);
    // 00474c40  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00474c42  83e710                 -and edi, 0x10
    cpu.edi &= x86::reg32(x86::sreg32(16 /*0x10*/));
    // 00474c45  a300d56f00             -mov dword ptr [0x6fd500], eax
    app->getMemory<x86::reg32>(x86::reg32(7329024) /* 0x6fd500 */) = cpu.eax;
    // 00474c4a  893dfcd46f00           -mov dword ptr [0x6fd4fc], edi
    app->getMemory<x86::reg32>(x86::reg32(7329020) /* 0x6fd4fc */) = cpu.edi;
    // 00474c50  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00474c52  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00474c53  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00474c54  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00474c55  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00474c56  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00474c57  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00474c58  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_474c60(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00474c60  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00474c61  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00474c62  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00474c63  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00474c65  8b15b4d36f00           -mov edx, dword ptr [0x6fd3b4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7328692) /* 0x6fd3b4 */);
    // 00474c6b  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00474c6d  b900010000             -mov ecx, 0x100
    cpu.ecx = 256 /*0x100*/;
    // 00474c72  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00474c74  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00474c77  f7f9                   -idiv ecx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ecx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 00474c79  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00474c7b  8a8238295500           -mov al, byte ptr [edx + 0x552938]
    cpu.al = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(5581112) /* 0x552938 */);
    // 00474c81  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00474c82  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00474c83  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00474c84  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_474ca0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 00474ca0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00474ca1  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00474ca2  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00474ca4  ba0d000000             -mov edx, 0xd
    cpu.edx = 13 /*0xd*/;
    // 00474ca9  b83c000000             -mov eax, 0x3c
    cpu.eax = 60 /*0x3c*/;
    // 00474cae  e85d1d0300             -call 0x4a6a10
    cpu.esp -= 4;
    sub_4a6a10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474cb3  e8c858fcff             -call 0x43a580
    cpu.esp -= 4;
    nfs3hp::load::buildCarClassLists(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474cb8  e883e6ffff             -call 0x473340
    cpu.esp -= 4;
    sub_473340(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474cbd  a1b4d36f00             -mov eax, dword ptr [0x6fd3b4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328692) /* 0x6fd3b4 */);
    // 00474cc2  e827c10600             -call 0x4e0dee
    cpu.esp -= 4;
    nfs3hp::ea::seedRandom(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474cc7  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00474cc9  a1b8d36f00             -mov eax, dword ptr [0x6fd3b8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328696) /* 0x6fd3b8 */);
    // 00474cce  8915acd36f00           -mov dword ptr [0x6fd3ac], edx
    app->getMemory<x86::reg32>(x86::reg32(7328684) /* 0x6fd3ac */) = cpu.edx;
    // 00474cd4  83f803                 +cmp eax, 3
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00474cd7  7721                   -ja 0x474cfa
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x00474cfa;
    }
    // 00474cd9  ff2485884c4700         -jmp dword ptr [eax*4 + 0x474c88]
    cpu.ip = app->getMemory<x86::reg32>(4672648 + cpu.eax * 4); goto dynamic_jump;
  case 0x00474ce0:
    // 00474ce0  e80befffff             -call 0x473bf0
    cpu.esp -= 4;
    sub_473bf0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474ce5  eb13                   -jmp 0x474cfa
    goto L_0x00474cfa;
  case 0x00474ce7:
    // 00474ce7  e824f6ffff             -call 0x474310
    cpu.esp -= 4;
    sub_474310(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474cec  eb0c                   -jmp 0x474cfa
    goto L_0x00474cfa;
  case 0x00474cee:
    // 00474cee  e8edf1ffff             -call 0x473ee0
    cpu.esp -= 4;
    sub_473ee0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474cf3  eb05                   -jmp 0x474cfa
    goto L_0x00474cfa;
  case 0x00474cf5:
    // 00474cf5  e846f9ffff             -call 0x474640
    cpu.esp -= 4;
    sub_474640(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00474cfa:
    // 00474cfa  a1aad46f00             -mov eax, dword ptr [0x6fd4aa]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328938) /* 0x6fd4aa */);
    // 00474cff  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00474d02  e8f9a10500             -call 0x4cef00
    cpu.esp -= 4;
    sub_4cef00(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474d07  98                     -cwde 
    cpu.eax = x86::reg32(static_cast<x86::sreg16>(cpu.ax));
    // 00474d08  a3b4d26f00             -mov dword ptr [0x6fd2b4], eax
    app->getMemory<x86::reg32>(x86::reg32(7328436) /* 0x6fd2b4 */) = cpu.eax;
    // 00474d0d  a350bb6f00             -mov dword ptr [0x6fbb50], eax
    app->getMemory<x86::reg32>(x86::reg32(7322448) /* 0x6fbb50 */) = cpu.eax;
    // 00474d12  b840bb6f00             -mov eax, 0x6fbb40
    cpu.eax = 7322432 /*0x6fbb40*/;
    // 00474d17  e894340000             -call 0x4781b0
    cpu.esp -= 4;
    sub_4781b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00474d1c  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00474d1d  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00474d1e  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* align: skip 0x00 */
void Application::sub_474d20(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00474d20  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d22  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d24  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d26  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d28  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d2a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d2c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d2e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d30  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d32  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d34  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d36  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d38  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d3a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d3c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d3e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d40  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d42  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d44  8d8000000000           -lea eax, [eax]
    cpu.eax = x86::reg32(cpu.eax);
    // 00474d4a  8d9200000000           -lea edx, [edx]
    cpu.edx = x86::reg32(cpu.edx);
    // 00474d50  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00474d51  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00474d52  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00474d53  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00474d55  8b1588b05500           -mov edx, dword ptr [0x55b088]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5615752) /* 0x55b088 */);
    // 00474d5b  8d04d500000000         -lea eax, [edx*8]
    cpu.eax = x86::reg32(cpu.edx * 8);
    // 00474d62  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00474d64  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 00474d67  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00474d69  ba8cb05500             -mov edx, 0x55b08c
    cpu.edx = 5615756 /*0x55b08c*/;
    // 00474d6e  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 00474d71  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00474d73  83ba3002000000         +cmp dword ptr [edx + 0x230], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(560) /* 0x230 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00474d7a  742d                   -je 0x474da9
    if (cpu.flags.zf)
    {
        goto L_0x00474da9;
    }
    // 00474d7c  c7823402000000000000   -mov dword ptr [edx + 0x234], 0
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(564) /* 0x234 */) = 0 /*0x0*/;
    // 00474d86  c7823002000000000000   -mov dword ptr [edx + 0x230], 0
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(560) /* 0x230 */) = 0 /*0x0*/;
    // 00474d90  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 00474d92  eb05                   -jmp 0x474d99
    goto L_0x00474d99;
L_0x00474d94:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00474d94  83f808                 +cmp eax, 8
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(8 /*0x8*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00474d97  7d10                   -jge 0x474da9
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00474da9;
    }
L_0x00474d99:
    // 00474d99  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00474d9b  c1e106                 +shl ecx, 6
    {
        x86::reg8 tmp = 6 /*0x6*/ % 32;
        x86::reg32& op = cpu.ecx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 00474d9e  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474d9f  c744113c00000000       -mov dword ptr [ecx + edx + 0x3c], 0
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(60) /* 0x3c */ + cpu.edx * 1) = 0 /*0x0*/;
    // 00474da7  ebeb                   -jmp 0x474d94
    goto L_0x00474d94;
L_0x00474da9:
    // 00474da9  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00474daa  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00474dab  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00474dac  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_474d50(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    goto L_entry_0x00474d50;
    // 00474d20  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d22  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d24  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d26  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d28  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d2a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d2c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d2e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d30  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d32  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d34  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d36  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d38  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d3a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d3c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d3e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d40  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d42  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00474d44  8d8000000000           -lea eax, [eax]
    cpu.eax = x86::reg32(cpu.eax);
    // 00474d4a  8d9200000000           -lea edx, [edx]
    cpu.edx = x86::reg32(cpu.edx);
L_entry_0x00474d50:
    // 00474d50  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00474d51  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00474d52  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00474d53  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00474d55  8b1588b05500           -mov edx, dword ptr [0x55b088]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5615752) /* 0x55b088 */);
    // 00474d5b  8d04d500000000         -lea eax, [edx*8]
    cpu.eax = x86::reg32(cpu.edx * 8);
    // 00474d62  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00474d64  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 00474d67  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00474d69  ba8cb05500             -mov edx, 0x55b08c
    cpu.edx = 5615756 /*0x55b08c*/;
    // 00474d6e  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 00474d71  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00474d73  83ba3002000000         +cmp dword ptr [edx + 0x230], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(560) /* 0x230 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00474d7a  742d                   -je 0x474da9
    if (cpu.flags.zf)
    {
        goto L_0x00474da9;
    }
    // 00474d7c  c7823402000000000000   -mov dword ptr [edx + 0x234], 0
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(564) /* 0x234 */) = 0 /*0x0*/;
    // 00474d86  c7823002000000000000   -mov dword ptr [edx + 0x230], 0
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(560) /* 0x230 */) = 0 /*0x0*/;
    // 00474d90  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 00474d92  eb05                   -jmp 0x474d99
    goto L_0x00474d99;
L_0x00474d94:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00474d94  83f808                 +cmp eax, 8
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(8 /*0x8*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00474d97  7d10                   -jge 0x474da9
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00474da9;
    }
L_0x00474d99:
    // 00474d99  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00474d9b  c1e106                 +shl ecx, 6
    {
        x86::reg8 tmp = 6 /*0x6*/ % 32;
        x86::reg32& op = cpu.ecx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 00474d9e  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00474d9f  c744113c00000000       -mov dword ptr [ecx + edx + 0x3c], 0
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(60) /* 0x3c */ + cpu.edx * 1) = 0 /*0x0*/;
    // 00474da7  ebeb                   -jmp 0x474d94
    goto L_0x00474d94;
L_0x00474da9:
    // 00474da9  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00474daa  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00474dab  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00474dac  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_474db0: native, nfs3hp::port::view::edgeTriangles */

/* sub_475240: native, nfs3hp::port::view::glyphResource */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_475b10(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00475b10  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00475b11  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00475b12  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00475b13  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00475b15  8b1510906700           -mov edx, dword ptr [0x679010]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(6787088) /* 0x679010 */);
    // 00475b1b  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 00475b1d  740f                   -je 0x475b2e
    if (cpu.flags.zf)
    {
        goto L_0x00475b2e;
    }
    // 00475b1f  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00475b21  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00475b23  e868bd0600             -call 0x4e1890
    cpu.esp -= 4;
    nfs3hp::ea::purgememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00475b28  890d10906700           -mov dword ptr [0x679010], ecx
    app->getMemory<x86::reg32>(x86::reg32(6787088) /* 0x679010 */) = cpu.ecx;
L_0x00475b2e:
    // 00475b2e  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00475b2f  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00475b30  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00475b31  c3                     -ret 
    cpu.esp += 4;
    return;
}

}
