#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{
// Port (tools/apply_native_frontend.py): defined in native_frontend_tracks.cpp.
bool infoOnExit(win32::WinApplication* app, x86::CPU& cpu);
// Port (tools/apply_native_frontend.py): defined in native_frontend_tracks.cpp.
bool infoOnFrame(win32::WinApplication* app, x86::CPU& cpu);
// Port (tools/apply_native_frontend.py): defined in native_frontend_tracks.cpp.
bool infoOnEnter(win32::WinApplication* app, x86::CPU& cpu);
// Port (tools/apply_native_frontend.py): defined in native_frontend_tracks.cpp.
bool infoPlaceTrackPlaces(win32::WinApplication* app, x86::CPU& cpu);

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_4543e0(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004543e0  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004543e1  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004543e3  e8a8500700             -call 0x4c9490
    cpu.esp -= 4;
    nfs3hp::port::descriptionStop(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004543e8  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004543ea  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004543eb  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_4543f0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004543f0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004543f1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004543f2  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004543f3  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004543f4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004543f5  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004543f7  83ec6c                 -sub esp, 0x6c
    (cpu.esp) -= x86::reg32(x86::sreg32(108 /*0x6c*/));
    // 004543fa  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004543fc  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 004543fe  6850985300             -push 0x539850
    app->getMemory<x86::reg32>(cpu.esp-4) = 5478480 /*0x539850*/;
    cpu.esp -= 4;
    // 00454403  68b42d7a00             -push 0x7a2db4
    app->getMemory<x86::reg32>(cpu.esp-4) = 8007092 /*0x7a2db4*/;
    cpu.esp -= 4;
    // 00454408  685c985300             -push 0x53985c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5478492 /*0x53985c*/;
    cpu.esp -= 4;
    // 0045440d  8d4594                 -lea eax, [ebp - 0x6c]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-108) /* -0x6c */);
    // 00454410  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00454411  e87ab20800             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454416  83c410                 -add esp, 0x10
    (cpu.esp) += x86::reg32(x86::sreg32(16 /*0x10*/));
    // 00454419  8d4594                 -lea eax, [ebp - 0x6c]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-108) /* -0x6c */);
    // 0045441c  e86f68ffff             -call 0x44ac90
    cpu.esp -= 4;
    nfs3hp::port::buttons::loadShapes(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454421  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 00454424  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00454426  e8654efeff             -call 0x439290
    cpu.esp -= 4;
    sub_439290(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0045442b  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0045442d  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
L_0x0045442f:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0045442f  8a21                   -mov ah, byte ptr [ecx]
    cpu.ah = app->getMemory<x86::reg8>(cpu.ecx);
    // 00454431  84e4                   +test ah, ah
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.ah & cpu.ah));
    // 00454433  7420                   -je 0x454455
    if (cpu.flags.zf)
    {
        goto L_0x00454455;
    }
    // 00454435  88e0                   -mov al, ah
    cpu.al = cpu.ah;
    // 00454437  fec0                   -inc al
    (cpu.al)++;
    // 00454439  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 0045443e  f680f04e560040         +test byte ptr [eax + 0x564ef0], 0x40
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5656304) /* 0x564ef0 */) & 64 /*0x40*/));
    // 00454445  740b                   -je 0x454452
    if (cpu.flags.zf)
    {
        goto L_0x00454452;
    }
    // 00454447  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 00454449  8a01                   -mov al, byte ptr [ecx]
    cpu.al = app->getMemory<x86::reg8>(cpu.ecx);
    // 0045444b  e890c60900             -call 0x4f0ae0
    cpu.esp -= 4;
    nfs3hp::watcom::tolower(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454450  8801                   -mov byte ptr [ecx], al
    app->getMemory<x86::reg8>(cpu.ecx) = cpu.al;
L_0x00454452:
    // 00454452  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00454453  ebda                   -jmp 0x45442f
    goto L_0x0045442f;
L_0x00454455:
    // 00454455  b910000000             -mov ecx, 0x10
    cpu.ecx = 16 /*0x10*/;
    // 0045445a  bb20000000             -mov ebx, 0x20
    cpu.ebx = 32 /*0x20*/;
    // 0045445f  bae0000000             -mov edx, 0xe0
    cpu.edx = 224 /*0xe0*/;
    // 00454464  b820010000             -mov eax, 0x120
    cpu.eax = 288 /*0x120*/;
    // 00454469  e822ad0900             -call 0x4ef190
    cpu.esp -= 4;
    sub_4ef190(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0045446e  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 00454471  e82a690900             -call 0x4eada0
    cpu.esp -= 4;
    sub_4eada0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454476  803d1050560008         +cmp byte ptr [0x565010], 8
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(x86::reg32(5656592) /* 0x565010 */);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(8 /*0x8*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0045447d  7507                   -jne 0x454486
    if (!cpu.flags.zf)
    {
        goto L_0x00454486;
    }
    // 0045447f  b8ff000000             -mov eax, 0xff
    cpu.eax = 255 /*0xff*/;
    // 00454484  eb02                   -jmp 0x454488
    goto L_0x00454488;
L_0x00454486:
    // 00454486  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x00454488:
    // 00454488  e823ad0900             -call 0x4ef1b0
    cpu.esp -= 4;
    sub_4ef1b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0045448d  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 0045448f  7441                   -je 0x4544d2
    if (cpu.flags.zf)
    {
        goto L_0x004544d2;
    }
    // 00454491  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00454494  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 00454496  e8d5ad0900             -call 0x4ef270
    cpu.esp -= 4;
    nfs3hp::ea::locateShape(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0045449b  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0045449d  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0045449f  7431                   -je 0x4544d2
    if (cpu.flags.zf)
    {
        goto L_0x004544d2;
    }
    // 004544a1  bbe0000000             -mov ebx, 0xe0
    cpu.ebx = 224 /*0xe0*/;
    // 004544a6  8b4004                 -mov eax, dword ptr [eax + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 004544a9  8b5102                 -mov edx, dword ptr [ecx + 2]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(2) /* 0x2 */);
    // 004544ac  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 004544af  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 004544b2  29c3                   -sub ebx, eax
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 004544b4  b820010000             -mov eax, 0x120
    cpu.eax = 288 /*0x120*/;
    // 004544b9  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004544bb  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004544bd  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004544c0  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004544c2  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 004544c4  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004544c6  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004544c8  e8f3af0900             -call 0x4ef4c0
    cpu.esp -= 4;
    sub_4ef4c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004544cd  e8be680900             -call 0x4ead90
    cpu.esp -= 4;
    sub_4ead90(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004544d2:
    // 004544d2  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004544d5  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 004544d7  8b4020                 -mov eax, dword ptr [eax + 0x20]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(32) /* 0x20 */);
    // 004544da  e8b11e0800             -call 0x4d6390
    cpu.esp -= 4;
    nfs3hp::port::shapes::artShapeTextures(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004544df  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004544e1  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004544e4  e817ba0900             -call 0x4eff00
    cpu.esp -= 4;
    sub_4eff00(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004544e9  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004544eb  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004544ed  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004544ee  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004544ef  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004544f0  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004544f1  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004544f2  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_454500(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00454500  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00454501  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00454502  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00454503  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00454505  833d6c29660009         +cmp dword ptr [0x66296c], 9
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(6695276) /* 0x66296c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(9 /*0x9*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0045450c  742d                   -je 0x45453b
    if (cpu.flags.zf)
    {
        goto L_0x0045453b;
    }
    // 0045450e  e8edefffff             -call 0x453500
    cpu.esp -= 4;
    sub_453500(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454513  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00454515  740a                   -je 0x454521
    if (cpu.flags.zf)
    {
        goto L_0x00454521;
    }
    // 00454517  e8e4efffff             -call 0x453500
    cpu.esp -= 4;
    sub_453500(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0045451c  a388476600             -mov dword ptr [0x664788], eax
    app->getMemory<x86::reg32>(x86::reg32(6702984) /* 0x664788 */) = cpu.eax;
L_0x00454521:
    // 00454521  8b0dd4d46f00           -mov ecx, dword ptr [0x6fd4d4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7328980) /* 0x6fd4d4 */);
    // 00454527  83f903                 +cmp ecx, 3
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0045452a  7e05                   -jle 0x454531
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00454531;
    }
    // 0045452c  8d41fc                 -lea eax, [ecx - 4]
    cpu.eax = x86::reg32(cpu.ecx + x86::reg32(-4) /* -0x4 */);
    // 0045452f  eb05                   -jmp 0x454536
    goto L_0x00454536;
L_0x00454531:
    // 00454531  a188476600             -mov eax, dword ptr [0x664788]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6702984) /* 0x664788 */);
L_0x00454536:
    // 00454536  a384476600             -mov dword ptr [0x664784], eax
    app->getMemory<x86::reg32>(x86::reg32(6702980) /* 0x664784 */) = cpu.eax;
L_0x0045453b:
    // 0045453b  a188476600             -mov eax, dword ptr [0x664788]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6702984) /* 0x664788 */);
    // 00454540  a38c476600             -mov dword ptr [0x66478c], eax
    app->getMemory<x86::reg32>(x86::reg32(6702988) /* 0x66478c */) = cpu.eax;
    // 00454545  a184476600             -mov eax, dword ptr [0x664784]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6702980) /* 0x664784 */);
    // 0045454a  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0045454c  a390476600             -mov dword ptr [0x664790], eax
    app->getMemory<x86::reg32>(x86::reg32(6702992) /* 0x664790 */) = cpu.eax;
    // 00454551  a188476600             -mov eax, dword ptr [0x664788]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6702984) /* 0x664788 */);
    // 00454556  e895feffff             -call 0x4543f0
    cpu.esp -= 4;
    sub_4543f0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0045455b  a3ac955500             -mov dword ptr [0x5595ac], eax
    app->getMemory<x86::reg32>(x86::reg32(5608876) /* 0x5595ac */) = cpu.eax;
    // 00454560  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00454562  a184476600             -mov eax, dword ptr [0x664784]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6702980) /* 0x664784 */);
    // 00454567  e884feffff             -call 0x4543f0
    cpu.esp -= 4;
    sub_4543f0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0045456c  a3b0955500             -mov dword ptr [0x5595b0], eax
    app->getMemory<x86::reg32>(x86::reg32(5608880) /* 0x5595b0 */) = cpu.eax;
    // 00454571  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00454573  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454574  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454575  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454576  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_454580(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00454580  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00454581  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00454582  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00454583  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00454584  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00454586  83ec50                 -sub esp, 0x50
    (cpu.esp) -= x86::reg32(x86::sreg32(80 /*0x50*/));
    // 00454589  833d6c29660009         +cmp dword ptr [0x66296c], 9
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(6695276) /* 0x66296c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(9 /*0x9*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00454590  742d                   -je 0x4545bf
    if (cpu.flags.zf)
    {
        goto L_0x004545bf;
    }
    // 00454592  e869efffff             -call 0x453500
    cpu.esp -= 4;
    sub_453500(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454597  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00454599  740a                   -je 0x4545a5
    if (cpu.flags.zf)
    {
        goto L_0x004545a5;
    }
    // 0045459b  e860efffff             -call 0x453500
    cpu.esp -= 4;
    sub_453500(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004545a0  a388476600             -mov dword ptr [0x664788], eax
    app->getMemory<x86::reg32>(x86::reg32(6702984) /* 0x664788 */) = cpu.eax;
L_0x004545a5:
    // 004545a5  8b0dd4d46f00           -mov ecx, dword ptr [0x6fd4d4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7328980) /* 0x6fd4d4 */);
    // 004545ab  83f903                 +cmp ecx, 3
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004545ae  7e05                   -jle 0x4545b5
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004545b5;
    }
    // 004545b0  8d41fc                 -lea eax, [ecx - 4]
    cpu.eax = x86::reg32(cpu.ecx + x86::reg32(-4) /* -0x4 */);
    // 004545b3  eb05                   -jmp 0x4545ba
    goto L_0x004545ba;
L_0x004545b5:
    // 004545b5  a188476600             -mov eax, dword ptr [0x664788]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6702984) /* 0x664788 */);
L_0x004545ba:
    // 004545ba  a384476600             -mov dword ptr [0x664784], eax
    app->getMemory<x86::reg32>(x86::reg32(6702980) /* 0x664784 */) = cpu.eax;
L_0x004545bf:
    // 004545bf  68b42d7a00             -push 0x7a2db4
    app->getMemory<x86::reg32>(cpu.esp-4) = 8007092 /*0x7a2db4*/;
    cpu.esp -= 4;
    // 004545c4  6864985300             -push 0x539864
    app->getMemory<x86::reg32>(cpu.esp-4) = 5478500 /*0x539864*/;
    cpu.esp -= 4;
    // 004545c9  8d45b0                 -lea eax, [ebp - 0x50]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-80) /* -0x50 */);
    // 004545cc  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004545ce  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004545cf  891dd4955500           -mov dword ptr [0x5595d4], ebx
    app->getMemory<x86::reg32>(x86::reg32(5608916) /* 0x5595d4 */) = cpu.ebx;
    // 004545d5  891d74955500           -mov dword ptr [0x559574], ebx
    app->getMemory<x86::reg32>(x86::reg32(5608820) /* 0x559574 */) = cpu.ebx;
    // 004545db  e8b0b00800             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004545e0  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 004545e3  8d45b0                 -lea eax, [ebp - 0x50]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-80) /* -0x50 */);
    // 004545e6  e8a566ffff             -call 0x44ac90
    cpu.esp -= 4;
    nfs3hp::port::buttons::loadShapes(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004545eb  f605583a7a0002         +test byte ptr [0x7a3a58], 2
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(x86::reg32(8010328) /* 0x7a3a58 */) & 2 /*0x2*/));
    // 004545f2  7407                   -je 0x4545fb
    if (cpu.flags.zf)
    {
        goto L_0x004545fb;
    }
    // 004545f4  ba74985300             -mov edx, 0x539874
    cpu.edx = 5478516 /*0x539874*/;
    // 004545f9  eb05                   -jmp 0x454600
    goto L_0x00454600;
L_0x004545fb:
    // 004545fb  ba7c985300             -mov edx, 0x53987c
    cpu.edx = 5478524 /*0x53987c*/;
L_0x00454600:
    // 00454600  e8dbab0900             -call 0x4ef1e0
    cpu.esp -= 4;
    sub_4ef1e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454605  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00454607  e8841d0800             -call 0x4d6390
    cpu.esp -= 4;
    nfs3hp::port::shapes::artShapeTextures(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0045460c  a3b8955500             -mov dword ptr [0x5595b8], eax
    app->getMemory<x86::reg32>(x86::reg32(5608888) /* 0x5595b8 */) = cpu.eax;
    // 00454611  b884985300             -mov eax, 0x539884
    cpu.eax = 5478532 /*0x539884*/;
    // 00454616  e895170800             -call 0x4d5db0
    cpu.esp -= 4;
    nfs3hp::port::shapes::shapeByName(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0045461b  66a39c476600           -mov word ptr [0x66479c], ax
    app->getMemory<x86::reg16>(x86::reg32(6703004) /* 0x66479c */) = cpu.ax;
    // 00454621  b88c985300             -mov eax, 0x53988c
    cpu.eax = 5478540 /*0x53988c*/;
    // 00454626  e885170800             -call 0x4d5db0
    cpu.esp -= 4;
    nfs3hp::port::shapes::shapeByName(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0045462b  66a3a2476600           -mov word ptr [0x6647a2], ax
    app->getMemory<x86::reg16>(x86::reg32(6703010) /* 0x6647a2 */) = cpu.ax;
    // 00454631  b894985300             -mov eax, 0x539894
    cpu.eax = 5478548 /*0x539894*/;
    // 00454636  e875170800             -call 0x4d5db0
    cpu.esp -= 4;
    nfs3hp::port::shapes::shapeByName(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0045463b  66a3a0476600           -mov word ptr [0x6647a0], ax
    app->getMemory<x86::reg16>(x86::reg32(6703008) /* 0x6647a0 */) = cpu.ax;
    // 00454641  b89c985300             -mov eax, 0x53989c
    cpu.eax = 5478556 /*0x53989c*/;
    // 00454646  e865170800             -call 0x4d5db0
    cpu.esp -= 4;
    nfs3hp::port::shapes::shapeByName(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0045464b  66a39e476600           -mov word ptr [0x66479e], ax
    app->getMemory<x86::reg16>(x86::reg32(6703006) /* 0x66479e */) = cpu.ax;
    // 00454651  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00454653  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00454655  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454656  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454657  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454658  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454659  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_454660(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00454660  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00454661  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00454662  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00454663  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00454665  8b1570955500           -mov edx, dword ptr [0x559570]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5608816) /* 0x559570 */);
    // 0045466b  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0045466d  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0045466f  7510                   -jne 0x454681
    if (!cpu.flags.zf)
    {
        goto L_0x00454681;
    }
    // 00454671  b928975300             -mov ecx, 0x539728
    cpu.ecx = 5478184 /*0x539728*/;
    // 00454676  b809000000             -mov eax, 9
    cpu.eax = 9 /*0x9*/;
    // 0045467b  890d24925500           -mov dword ptr [0x559224], ecx
    app->getMemory<x86::reg32>(x86::reg32(5607972) /* 0x559224 */) = cpu.ecx;
L_0x00454681:
    // 00454681  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454682  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454683  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454684  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_454690(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00454690  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00454691  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00454692  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00454693  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00454694  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00454695  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00454697  8b1570955500           -mov edx, dword ptr [0x559570]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5608816) /* 0x559570 */);
    // 0045469d  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0045469f  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004546a1  740f                   -je 0x4546b2
    if (cpu.flags.zf)
    {
        goto L_0x004546b2;
    }
    // 004546a3  c7052492550030975300   -mov dword ptr [0x559224], 0x539730
    app->getMemory<x86::reg32>(x86::reg32(5607972) /* 0x559224 */) = 5478192 /*0x539730*/;
    // 004546ad  b909000000             -mov ecx, 9
    cpu.ecx = 9 /*0x9*/;
L_0x004546b2:
    // 004546b2  8b1d8c476600           -mov ebx, dword ptr [0x66478c]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(6702988) /* 0x66478c */);
    // 004546b8  a188476600             -mov eax, dword ptr [0x664788]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6702984) /* 0x664788 */);
    // 004546bd  39d8                   +cmp eax, ebx
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
    // 004546bf  7415                   -je 0x4546d6
    if (cpu.flags.zf)
    {
        goto L_0x004546d6;
    }
    // 004546c1  8b15ac955500           -mov edx, dword ptr [0x5595ac]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5608876) /* 0x5595ac */);
    // 004546c7  a38c476600             -mov dword ptr [0x66478c], eax
    app->getMemory<x86::reg32>(x86::reg32(6702988) /* 0x66478c */) = cpu.eax;
    // 004546cc  e81ffdffff             -call 0x4543f0
    cpu.esp -= 4;
    sub_4543f0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004546d1  a3ac955500             -mov dword ptr [0x5595ac], eax
    app->getMemory<x86::reg32>(x86::reg32(5608876) /* 0x5595ac */) = cpu.eax;
L_0x004546d6:
    // 004546d6  8b3590476600           -mov esi, dword ptr [0x664790]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(6702992) /* 0x664790 */);
    // 004546dc  a184476600             -mov eax, dword ptr [0x664784]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6702980) /* 0x664784 */);
    // 004546e1  39f0                   +cmp eax, esi
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
    // 004546e3  7415                   -je 0x4546fa
    if (cpu.flags.zf)
    {
        goto L_0x004546fa;
    }
    // 004546e5  8b15b0955500           -mov edx, dword ptr [0x5595b0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5608880) /* 0x5595b0 */);
    // 004546eb  a390476600             -mov dword ptr [0x664790], eax
    app->getMemory<x86::reg32>(x86::reg32(6702992) /* 0x664790 */) = cpu.eax;
    // 004546f0  e8fbfcffff             -call 0x4543f0
    cpu.esp -= 4;
    sub_4543f0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004546f5  a3b0955500             -mov dword ptr [0x5595b0], eax
    app->getMemory<x86::reg32>(x86::reg32(5608880) /* 0x5595b0 */) = cpu.eax;
L_0x004546fa:
    // 004546fa  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004546fc  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004546fd  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004546fe  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004546ff  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454700  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454701  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_454710(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00454710  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00454711  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00454713  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00454715  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454716  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_454730(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 00454730  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00454731  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00454732  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00454733  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00454735  8b0d70955500           -mov ecx, dword ptr [0x559570]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5608816) /* 0x559570 */);
    // 0045473b  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0045473d  83f904                 +cmp ecx, 4
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00454740  7734                   -ja 0x454776
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x00454776;
    }
    // 00454742  ff248d18474500         -jmp dword ptr [ecx*4 + 0x454718]
    cpu.ip = app->getMemory<x86::reg32>(4540184 + cpu.ecx * 4); goto dynamic_jump;
  case 0x00454749:
    // 00454749  e8e24ffeff             -call 0x439730
    cpu.esp -= 4;
    nfs3hp::load::carClass(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0045474e  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00454750  7524                   -jne 0x454776
    if (!cpu.flags.zf)
    {
        goto L_0x00454776;
    }
    // 00454752  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 00454757  eb1d                   -jmp 0x454776
    goto L_0x00454776;
  case 0x00454759:
    // 00454759  e8d24ffeff             -call 0x439730
    cpu.esp -= 4;
    nfs3hp::load::carClass(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0045475e  83f801                 +cmp eax, 1
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
    // 00454761  7513                   -jne 0x454776
    if (!cpu.flags.zf)
    {
        goto L_0x00454776;
    }
    // 00454763  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00454765  eb0f                   -jmp 0x454776
    goto L_0x00454776;
  case 0x00454767:
    // 00454767  e8c44ffeff             -call 0x439730
    cpu.esp -= 4;
    nfs3hp::load::carClass(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0045476c  83f802                 +cmp eax, 2
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
    // 0045476f  7505                   -jne 0x454776
    if (!cpu.flags.zf)
    {
        goto L_0x00454776;
    }
  [[fallthrough]];
  case 0x00454771:
    // 00454771  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
L_0x00454776:
    // 00454776  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00454778  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454779  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045477a  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045477b  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_454780(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00454780  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00454781  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00454782  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00454783  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00454784  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00454785  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00454786  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00454788  83ec0c                 -sub esp, 0xc
    (cpu.esp) -= x86::reg32(x86::sreg32(12 /*0xc*/));
    // 0045478b  e86056ffff             -call 0x449df0
    cpu.esp -= 4;
    nfs3hp::port::loop::keysTaken(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454790  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00454792  7407                   -je 0x45479b
    if (cpu.flags.zf)
    {
        goto L_0x0045479b;
    }
    // 00454794  ba964a0000             -mov edx, 0x4a96
    cpu.edx = 19094 /*0x4a96*/;
    // 00454799  eb05                   -jmp 0x4547a0
    goto L_0x004547a0;
L_0x0045479b:
    // 0045479b  ba40e4ff00             -mov edx, 0xffe440
    cpu.edx = 16770112 /*0xffe440*/;
L_0x004547a0:
    // 004547a0  e82b44feff             -call 0x438bd0
    cpu.esp -= 4;
    nfs3hp::load::carCount(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004547a5  8955f4                 -mov dword ptr [ebp - 0xc], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.edx;
    // 004547a8  8b15504f5500           -mov edx, dword ptr [0x554f50]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5590864) /* 0x554f50 */);
    // 004547ae  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 004547b0  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004547b2  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 004547b4  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
L_0x004547b7:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004547b7  3b75f8                 +cmp esi, dword ptr [ebp - 8]
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004547ba  7d0f                   -jge 0x4547cb
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004547cb;
    }
    // 004547bc  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004547be  e86dffffff             -call 0x454730
    cpu.esp -= 4;
    sub_454730(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004547c3  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004547c5  7401                   -je 0x4547c8
    if (cpu.flags.zf)
    {
        goto L_0x004547c8;
    }
    // 004547c7  47                     +inc edi
    {
        x86::reg32& tmp = cpu.edi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
L_0x004547c8:
    // 004547c8  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004547c9  ebec                   -jmp 0x4547b7
    goto L_0x004547b7;
L_0x004547cb:
    // 004547cb  a1d4955500             -mov eax, dword ptr [0x5595d4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5608916) /* 0x5595d4 */);
    // 004547d0  01f8                   -add eax, edi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edi));
    // 004547d2  83f812                 +cmp eax, 0x12
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(18 /*0x12*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004547d5  7d0a                   -jge 0x4547e1
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004547e1;
    }
    // 004547d7  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 004547d9  891dd4955500           -mov dword ptr [0x5595d4], ebx
    app->getMemory<x86::reg32>(x86::reg32(5608916) /* 0x5595d4 */) = cpu.ebx;
    // 004547df  eb60                   -jmp 0x454841
    goto L_0x00454841;
L_0x004547e1:
    // 004547e1  833dd495550000         +cmp dword ptr [0x5595d4], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5608916) /* 0x5595d4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004547e8  7e57                   -jle 0x454841
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00454841;
    }
    // 004547ea  bb9d000000             -mov ebx, 0x9d
    cpu.ebx = 157 /*0x9d*/;
    // 004547ef  a19a476600             -mov eax, dword ptr [0x66479a]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6703002) /* 0x66479a */);
    // 004547f4  bab9000000             -mov edx, 0xb9
    cpu.edx = 185 /*0xb9*/;
    // 004547f9  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 004547fc  b90f000000             -mov ecx, 0xf
    cpu.ecx = 15 /*0xf*/;
    // 00454801  e88a310800             -call 0x4d7990
    cpu.esp -= 4;
    nfs3hp::port::shapes::drawShape(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454806  ba9b000000             -mov edx, 0x9b
    cpu.edx = 155 /*0x9b*/;
    // 0045480b  b8b7000000             -mov eax, 0xb7
    cpu.eax = 183 /*0xb7*/;
    // 00454810  89cb                   -mov ebx, ecx
    cpu.ebx = cpu.ecx;
    // 00454812  e8297a0400             -call 0x49c240
    cpu.esp -= 4;
    sub_49c240(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454817  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00454819  7426                   -je 0x454841
    if (cpu.flags.zf)
    {
        goto L_0x00454841;
    }
    // 0045481b  bb9d000000             -mov ebx, 0x9d
    cpu.ebx = 157 /*0x9d*/;
    // 00454820  a19e476600             -mov eax, dword ptr [0x66479e]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6703006) /* 0x66479e */);
    // 00454825  bab9000000             -mov edx, 0xb9
    cpu.edx = 185 /*0xb9*/;
    // 0045482a  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0045482d  e85e310800             -call 0x4d7990
    cpu.esp -= 4;
    nfs3hp::port::shapes::drawShape(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454832  833d5846660000         +cmp dword ptr [0x664658], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(6702680) /* 0x664658 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00454839  7406                   -je 0x454841
    if (cpu.flags.zf)
    {
        goto L_0x00454841;
    }
    // 0045483b  ff0dd4955500           -dec dword ptr [0x5595d4]
    (app->getMemory<x86::reg32>(x86::reg32(5608916) /* 0x5595d4 */))--;
L_0x00454841:
    // 00454841  a1d4955500             -mov eax, dword ptr [0x5595d4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5608916) /* 0x5595d4 */);
    // 00454846  83c012                 -add eax, 0x12
    (cpu.eax) += x86::reg32(x86::sreg32(18 /*0x12*/));
    // 00454849  39f8                   +cmp eax, edi
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
    // 0045484b  7d57                   -jge 0x4548a4
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004548a4;
    }
    // 0045484d  bb9c010000             -mov ebx, 0x19c
    cpu.ebx = 412 /*0x19c*/;
    // 00454852  a1a0476600             -mov eax, dword ptr [0x6647a0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6703008) /* 0x6647a0 */);
    // 00454857  bab9000000             -mov edx, 0xb9
    cpu.edx = 185 /*0xb9*/;
    // 0045485c  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0045485f  b90f000000             -mov ecx, 0xf
    cpu.ecx = 15 /*0xf*/;
    // 00454864  e827310800             -call 0x4d7990
    cpu.esp -= 4;
    nfs3hp::port::shapes::drawShape(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454869  ba9a010000             -mov edx, 0x19a
    cpu.edx = 410 /*0x19a*/;
    // 0045486e  b8b7000000             -mov eax, 0xb7
    cpu.eax = 183 /*0xb7*/;
    // 00454873  89cb                   -mov ebx, ecx
    cpu.ebx = cpu.ecx;
    // 00454875  e8c6790400             -call 0x49c240
    cpu.esp -= 4;
    sub_49c240(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0045487a  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0045487c  7426                   -je 0x4548a4
    if (cpu.flags.zf)
    {
        goto L_0x004548a4;
    }
    // 0045487e  bb9c010000             -mov ebx, 0x19c
    cpu.ebx = 412 /*0x19c*/;
    // 00454883  a19c476600             -mov eax, dword ptr [0x66479c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6703004) /* 0x66479c */);
    // 00454888  bab9000000             -mov edx, 0xb9
    cpu.edx = 185 /*0xb9*/;
    // 0045488d  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00454890  e8fb300800             -call 0x4d7990
    cpu.esp -= 4;
    nfs3hp::port::shapes::drawShape(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454895  833d5846660000         +cmp dword ptr [0x664658], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(6702680) /* 0x664658 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0045489c  7406                   -je 0x4548a4
    if (cpu.flags.zf)
    {
        goto L_0x004548a4;
    }
    // 0045489e  ff05d4955500           -inc dword ptr [0x5595d4]
    (app->getMemory<x86::reg32>(x86::reg32(5608916) /* 0x5595d4 */))++;
L_0x004548a4:
    // 004548a4  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 004548a6  bf9b000000             -mov edi, 0x9b
    cpu.edi = 155 /*0x9b*/;
    // 004548ab  8975fc                 -mov dword ptr [ebp - 4], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.esi;
    // 004548ae  8b35d4955500           -mov esi, dword ptr [0x5595d4]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(5608916) /* 0x5595d4 */);
L_0x004548b4:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004548b4  3b75f8                 +cmp esi, dword ptr [ebp - 8]
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004548b7  0f8df6000000           -jge 0x4549b3
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004549b3;
    }
    // 004548bd  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004548bf  e86cfeffff             -call 0x454730
    cpu.esp -= 4;
    sub_454730(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004548c4  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004548c6  0f84e1000000           -je 0x4549ad
    if (cpu.flags.zf)
    {
        goto L_0x004549ad;
    }
    // 004548cc  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004548cf  83fa12                 +cmp edx, 0x12
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(18 /*0x12*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004548d2  0f8dd5000000           -jge 0x4549ad
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004549ad;
    }
    // 004548d8  8b4df4                 -mov ecx, dword ptr [ebp - 0xc]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004548db  8d5a01                 -lea ebx, [edx + 1]
    cpu.ebx = x86::reg32(cpu.edx + x86::reg32(1) /* 0x1 */);
    // 004548de  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004548df  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004548e1  ba02000000             -mov edx, 2
    cpu.edx = 2 /*0x2*/;
    // 004548e6  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 004548e8  895dfc                 -mov dword ptr [ebp - 4], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ebx;
    // 004548eb  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 004548f0  e8bb51feff             -call 0x439ab0
    cpu.esp -= 4;
    nfs3hp::port::fields::carText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004548f5  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 004548f7  bac8000000             -mov edx, 0xc8
    cpu.edx = 200 /*0xc8*/;
    // 004548fc  e88fd8ffff             -call 0x452190
    cpu.esp -= 4;
    nfs3hp::port::text::drawMenuText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454901  a174955500             -mov eax, dword ptr [0x559574]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5608820) /* 0x559574 */);
    // 00454906  8b148598955500         -mov edx, dword ptr [eax*4 + 0x559598]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5608856) /* 0x559598 */ + cpu.eax * 4);
    // 0045490d  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0045490f  e84c46feff             -call 0x438f60
    cpu.esp -= 4;
    sub_438f60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454914  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00454916  83f814                 +cmp eax, 0x14
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(20 /*0x14*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00454919  7e05                   -jle 0x454920
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00454920;
    }
    // 0045491b  ba14000000             -mov edx, 0x14
    cpu.edx = 20 /*0x14*/;
L_0x00454920:
    // 00454920  f605583a7a0002         +test byte ptr [0x7a3a58], 2
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(x86::reg32(8010328) /* 0x7a3a58 */) & 2 /*0x2*/));
    // 00454927  742d                   -je 0x454956
    if (cpu.flags.zf)
    {
        goto L_0x00454956;
    }
    // 00454929  69d29a000000           -imul edx, edx, 0x9a
    cpu.edx = x86::reg32(x86::sreg64(x86::sreg32(cpu.edx)) * x86::sreg64(x86::sreg32(154 /*0x9a*/)));
    // 0045492f  b914000000             -mov ecx, 0x14
    cpu.ecx = 20 /*0x14*/;
    // 00454934  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00454936  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00454939  f7f9                   -idiv ecx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ecx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 0045493b  b9e0010000             -mov ecx, 0x1e0
    cpu.ecx = 480 /*0x1e0*/;
    // 00454940  8d988c010000           -lea ebx, [eax + 0x18c]
    cpu.ebx = x86::reg32(cpu.eax + x86::reg32(396) /* 0x18c */);
    // 00454946  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00454948  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0045494a  e891a20600             -call 0x4bebe0
    cpu.esp -= 4;
    nfs3hp::graphics::setClip(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0045494f  ba59010000             -mov edx, 0x159
    cpu.edx = 345 /*0x159*/;
    // 00454954  eb35                   -jmp 0x45498b
    goto L_0x0045498b;
L_0x00454956:
    // 00454956  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 0045495d  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0045495f  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00454961  c1e006                 -shl eax, 6
    cpu.eax <<= 6 /*0x6*/ % 32;
    // 00454964  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00454966  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00454968  b914000000             -mov ecx, 0x14
    cpu.ecx = 20 /*0x14*/;
    // 0045496d  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00454970  f7f9                   -idiv ecx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ecx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 00454972  b9e0010000             -mov ecx, 0x1e0
    cpu.ecx = 480 /*0x1e0*/;
    // 00454977  8d9884010000           -lea ebx, [eax + 0x184]
    cpu.ebx = x86::reg32(cpu.eax + x86::reg32(388) /* 0x184 */);
    // 0045497d  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0045497f  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00454981  e85aa20600             -call 0x4bebe0
    cpu.esp -= 4;
    nfs3hp::graphics::setClip(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454986  ba45010000             -mov edx, 0x145
    cpu.edx = 325 /*0x145*/;
L_0x0045498b:
    // 0045498b  a1b8955500             -mov eax, dword ptr [0x5595b8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5608888) /* 0x5595b8 */);
    // 00454990  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 00454992  e8f92f0800             -call 0x4d7990
    cpu.esp -= 4;
    nfs3hp::port::shapes::drawShape(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454997  b9e0010000             -mov ecx, 0x1e0
    cpu.ecx = 480 /*0x1e0*/;
    // 0045499c  bb80020000             -mov ebx, 0x280
    cpu.ebx = 640 /*0x280*/;
    // 004549a1  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004549a3  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004549a5  e836a20600             -call 0x4bebe0
    cpu.esp -= 4;
    nfs3hp::graphics::setClip(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004549aa  83c70f                 +add edi, 0xf
    {
        x86::reg32& tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(15 /*0xf*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
L_0x004549ad:
    // 004549ad  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004549ae  e901ffffff             -jmp 0x4548b4
    goto L_0x004548b4;
L_0x004549b3:
    // 004549b3  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004549b5  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004549b6  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004549b7  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004549b8  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004549b9  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004549ba  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004549bb  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_4549c0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004549c0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004549c1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004549c2  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004549c3  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004549c4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004549c5  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004549c7  83ec2c                 -sub esp, 0x2c
    (cpu.esp) -= x86::reg32(x86::sreg32(44 /*0x2c*/));
    // 004549ca  ba1c000000             -mov edx, 0x1c
    cpu.edx = 28 /*0x1c*/;
    // 004549cf  e88c5cffff             -call 0x44a660
    cpu.esp -= 4;
    nfs3hp::frontend::menuShownNow(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004549d4  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004549d6  0f8409020000           -je 0x454be5
    if (cpu.flags.zf)
    {
        goto L_0x00454be5;
    }
    // 004549dc  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 004549de  eb05                   -jmp 0x4549e5
    goto L_0x004549e5;
L_0x004549e0:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004549e0  83fb04                 +cmp ebx, 4
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004549e3  7d53                   -jge 0x454a38
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00454a38;
    }
L_0x004549e5:
    // 004549e5  8d349d00000000         -lea esi, [ebx*4]
    cpu.esi = x86::reg32(cpu.ebx * 4);
    // 004549ec  a188476600             -mov eax, dword ptr [0x664788]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6702984) /* 0x664788 */);
    // 004549f1  8b9698955500           -mov edx, dword ptr [esi + 0x559598]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(5608856) /* 0x559598 */);
    // 004549f7  e86445feff             -call 0x438f60
    cpu.esp -= 4;
    sub_438f60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004549fc  89442ed4               -mov dword ptr [esi + ebp - 0x2c], eax
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(-44) /* -0x2c */ + cpu.ebp * 1) = cpu.eax;
    // 00454a00  83f814                 +cmp eax, 0x14
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(20 /*0x14*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00454a03  7e08                   -jle 0x454a0d
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00454a0d;
    }
    // 00454a05  c7442ed414000000       -mov dword ptr [esi + ebp - 0x2c], 0x14
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(-44) /* -0x2c */ + cpu.ebp * 1) = 20 /*0x14*/;
L_0x00454a0d:
    // 00454a0d  8d349d00000000         -lea esi, [ebx*4]
    cpu.esi = x86::reg32(cpu.ebx * 4);
    // 00454a14  a184476600             -mov eax, dword ptr [0x664784]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6702980) /* 0x664784 */);
    // 00454a19  8b9698955500           -mov edx, dword ptr [esi + 0x559598]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(5608856) /* 0x559598 */);
    // 00454a1f  e83c45feff             -call 0x438f60
    cpu.esp -= 4;
    sub_438f60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454a24  89442ee4               -mov dword ptr [esi + ebp - 0x1c], eax
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(-28) /* -0x1c */ + cpu.ebp * 1) = cpu.eax;
    // 00454a28  83f814                 +cmp eax, 0x14
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(20 /*0x14*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00454a2b  7e08                   -jle 0x454a35
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00454a35;
    }
    // 00454a2d  c7442ee414000000       -mov dword ptr [esi + ebp - 0x1c], 0x14
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(-28) /* -0x1c */ + cpu.ebp * 1) = 20 /*0x14*/;
L_0x00454a35:
    // 00454a35  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00454a36  eba8                   -jmp 0x4549e0
    goto L_0x004549e0;
L_0x00454a38:
    // 00454a38  31f6                   +xor esi, esi
    cpu.clear_co();
    cpu.set_szp((cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi))));
    // 00454a3a  e9ec000000             -jmp 0x454b2b
    goto L_0x00454b2b;
L_0x00454a3f:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00454a3f  3b4c2ad4               +cmp ecx, dword ptr [edx + ebp - 0x2c]
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.edx + x86::reg32(-44) /* -0x2c */ + cpu.ebp * 1)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00454a43  7d08                   -jge 0x454a4d
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00454a4d;
    }
    // 00454a45  a1bc955500             -mov eax, dword ptr [0x5595bc]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5608892) /* 0x5595bc */);
    // 00454a4a  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
L_0x00454a4d:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00454a4d  8b54b5d4               -mov edx, dword ptr [ebp + esi*4 - 0x2c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44) /* -0x2c */ + cpu.esi * 4);
    // 00454a51  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00454a58  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00454a5a  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 00454a5d  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00454a5f  c1e202                 -shl edx, 2
    cpu.edx <<= 2 /*0x2*/ % 32;
    // 00454a62  bb14000000             -mov ebx, 0x14
    cpu.ebx = 20 /*0x14*/;
    // 00454a67  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00454a69  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00454a6c  f7fb                   -idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 00454a6e  ba47010000             -mov edx, 0x147
    cpu.edx = 327 /*0x147*/;
    // 00454a73  b9e0010000             -mov ecx, 0x1e0
    cpu.ecx = 480 /*0x1e0*/;
    // 00454a78  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 00454a7a  bb80020000             -mov ebx, 0x280
    cpu.ebx = 640 /*0x280*/;
    // 00454a7f  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00454a81  29d3                   -sub ebx, edx
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00454a83  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00454a85  e856a10600             -call 0x4bebe0
    cpu.esp -= 4;
    nfs3hp::graphics::setClip(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454a8a  f605583a7a0002         +test byte ptr [0x7a3a58], 2
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(x86::reg32(8010328) /* 0x7a3a58 */) & 2 /*0x2*/));
    // 00454a91  740d                   -je 0x454aa0
    if (cpu.flags.zf)
    {
        goto L_0x00454aa0;
    }
    // 00454a93  bad6000000             -mov edx, 0xd6
    cpu.edx = 214 /*0xd6*/;
    // 00454a98  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00454a9b  8d5f0e                 -lea ebx, [edi + 0xe]
    cpu.ebx = x86::reg32(cpu.edi + x86::reg32(14) /* 0xe */);
    // 00454a9e  eb0a                   -jmp 0x454aaa
    goto L_0x00454aaa;
L_0x00454aa0:
    // 00454aa0  bad6000000             -mov edx, 0xd6
    cpu.edx = 214 /*0xd6*/;
    // 00454aa5  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00454aa8  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
L_0x00454aaa:
    // 00454aaa  e8e12e0800             -call 0x4d7990
    cpu.esp -= 4;
    nfs3hp::port::shapes::drawShape(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454aaf  8b54b5e4               -mov edx, dword ptr [ebp + esi*4 - 0x1c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */ + cpu.esi * 4);
    // 00454ab3  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00454aba  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00454abc  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 00454abf  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00454ac1  c1e202                 -shl edx, 2
    cpu.edx <<= 2 /*0x2*/ % 32;
    // 00454ac4  bb14000000             -mov ebx, 0x14
    cpu.ebx = 20 /*0x14*/;
    // 00454ac9  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00454acb  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00454ace  f7fb                   -idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 00454ad0  b9e0010000             -mov ecx, 0x1e0
    cpu.ecx = 480 /*0x1e0*/;
    // 00454ad5  8d98af010000           -lea ebx, [eax + 0x1af]
    cpu.ebx = x86::reg32(cpu.eax + x86::reg32(431) /* 0x1af */);
    // 00454adb  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00454add  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00454adf  e8fca00600             -call 0x4bebe0
    cpu.esp -= 4;
    nfs3hp::graphics::setClip(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454ae4  f605583a7a0002         +test byte ptr [0x7a3a58], 2
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(x86::reg32(8010328) /* 0x7a3a58 */) & 2 /*0x2*/));
    // 00454aeb  7412                   -je 0x454aff
    if (cpu.flags.zf)
    {
        goto L_0x00454aff;
    }
    // 00454aed  ba81010000             -mov edx, 0x181
    cpu.edx = 385 /*0x181*/;
    // 00454af2  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00454af5  8d5f0e                 -lea ebx, [edi + 0xe]
    cpu.ebx = x86::reg32(cpu.edi + x86::reg32(14) /* 0xe */);
    // 00454af8  e8932e0800             -call 0x4d7990
    cpu.esp -= 4;
    nfs3hp::port::shapes::drawShape(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454afd  eb0f                   -jmp 0x454b0e
    goto L_0x00454b0e;
L_0x00454aff:
    // 00454aff  ba54010000             -mov edx, 0x154
    cpu.edx = 340 /*0x154*/;
    // 00454b04  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00454b07  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 00454b09  e8522a0800             -call 0x4d7560
    cpu.esp -= 4;
    sub_4d7560(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00454b0e:
    // 00454b0e  b9e0010000             -mov ecx, 0x1e0
    cpu.ecx = 480 /*0x1e0*/;
    // 00454b13  bb80020000             -mov ebx, 0x280
    cpu.ebx = 640 /*0x280*/;
    // 00454b18  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00454b1a  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00454b1c  e8bfa00600             -call 0x4bebe0
    cpu.esp -= 4;
    nfs3hp::graphics::setClip(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454b21  46                     -inc esi
    (cpu.esi)++;
    // 00454b22  83fe04                 +cmp esi, 4
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00454b25  0f8d82000000           -jge 0x454bad
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00454bad;
    }
L_0x00454b2b:
    // 00454b2b  8a25583a7a00           -mov ah, byte ptr [0x7a3a58]
    cpu.ah = app->getMemory<x86::reg8>(x86::reg32(8010328) /* 0x7a3a58 */);
    // 00454b31  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00454b33  f6c402                 +test ah, 2
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.ah & 2 /*0x2*/));
    // 00454b36  7405                   -je 0x454b3d
    if (cpu.flags.zf)
    {
        goto L_0x00454b3d;
    }
    // 00454b38  ba03000000             -mov edx, 3
    cpu.edx = 3 /*0x3*/;
L_0x00454b3d:
    // 00454b3d  8d1c36                 -lea ebx, [esi + esi]
    cpu.ebx = x86::reg32(cpu.esi + cpu.esi * 1);
    // 00454b40  6840e4ff00             -push 0xffe440
    app->getMemory<x86::reg32>(cpu.esp-4) = 16770112 /*0xffe440*/;
    cpu.esp -= 4;
    // 00454b45  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 00454b47  c1e304                 -shl ebx, 4
    cpu.ebx <<= 4 /*0x4*/ % 32;
    // 00454b4a  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 00454b4f  29c3                   -sub ebx, eax
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 00454b51  6a02                   -push 2
    app->getMemory<x86::reg32>(cpu.esp-4) = 2 /*0x2*/;
    cpu.esp -= 4;
    // 00454b53  8dbb27010000           -lea edi, [ebx + 0x127]
    cpu.edi = x86::reg32(cpu.ebx + x86::reg32(295) /* 0x127 */);
    // 00454b59  8d04b500000000         -lea eax, [esi*4]
    cpu.eax = x86::reg32(cpu.esi * 4);
    // 00454b60  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 00454b62  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 00454b65  8b80c4955500           -mov eax, dword ptr [eax + 0x5595c4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(5608900) /* 0x5595c4 */);
    // 00454b6b  29d3                   -sub ebx, edx
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00454b6d  ba7a010000             -mov edx, 0x17a
    cpu.edx = 378 /*0x17a*/;
    // 00454b72  e8d9cc0700             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454b77  e814d6ffff             -call 0x452190
    cpu.esp -= 4;
    nfs3hp::port::text::drawMenuText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454b7c  a1b8955500             -mov eax, dword ptr [0x5595b8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5608888) /* 0x5595b8 */);
    // 00454b81  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 00454b84  a1c0955500             -mov eax, dword ptr [0x5595c0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5608896) /* 0x5595c0 */);
    // 00454b89  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00454b8c  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
    // 00454b8f  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00454b92  8b4c2ae4               -mov ecx, dword ptr [edx + ebp - 0x1c]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(-28) /* -0x1c */ + cpu.ebp * 1);
    // 00454b96  3b4c28d4               +cmp ecx, dword ptr [eax + ebp - 0x2c]
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-44) /* -0x2c */ + cpu.ebp * 1)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00454b9a  0f8e9ffeffff           -jle 0x454a3f
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00454a3f;
    }
    // 00454ba0  a1b4955500             -mov eax, dword ptr [0x5595b4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5608884) /* 0x5595b4 */);
    // 00454ba5  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 00454ba8  e9a0feffff             -jmp 0x454a4d
    goto L_0x00454a4d;
L_0x00454bad:
    // 00454bad  8b35ac955500           -mov esi, dword ptr [0x5595ac]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(5608876) /* 0x5595ac */);
    // 00454bb3  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 00454bb5  7411                   -je 0x454bc8
    if (cpu.flags.zf)
    {
        goto L_0x00454bc8;
    }
    // 00454bb7  bb4e000000             -mov ebx, 0x4e
    cpu.ebx = 78 /*0x4e*/;
    // 00454bbc  ba7a000000             -mov edx, 0x7a
    cpu.edx = 122 /*0x7a*/;
    // 00454bc1  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00454bc3  e8c82d0800             -call 0x4d7990
    cpu.esp -= 4;
    nfs3hp::port::shapes::drawShape(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00454bc8:
    // 00454bc8  8b3db0955500           -mov edi, dword ptr [0x5595b0]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(5608880) /* 0x5595b0 */);
    // 00454bce  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 00454bd0  7411                   -je 0x454be3
    if (cpu.flags.zf)
    {
        goto L_0x00454be3;
    }
    // 00454bd2  bb4e000000             -mov ebx, 0x4e
    cpu.ebx = 78 /*0x4e*/;
    // 00454bd7  ba63010000             -mov edx, 0x163
    cpu.edx = 355 /*0x163*/;
    // 00454bdc  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 00454bde  e8ad2d0800             -call 0x4d7990
    cpu.esp -= 4;
    nfs3hp::port::shapes::drawShape(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00454be3:
    // 00454be3  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x00454be5:
    // 00454be5  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00454be7  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454be8  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454be9  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454bea  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454beb  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454bec  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_454bf0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00454bf0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00454bf1  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00454bf2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00454bf3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00454bf5  81ec00010000           -sub esp, 0x100
    (cpu.esp) -= x86::reg32(x86::sreg32(256 /*0x100*/));
    // 00454bfb  ba1c000000             -mov edx, 0x1c
    cpu.edx = 28 /*0x1c*/;
    // 00454c00  e85b5affff             -call 0x44a660
    cpu.esp -= 4;
    nfs3hp::frontend::menuShownNow(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454c05  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00454c07  0f84d2000000           -je 0x454cdf
    if (cpu.flags.zf)
    {
        goto L_0x00454cdf;
    }
    // 00454c0d  68b42d7a00             -push 0x7a2db4
    app->getMemory<x86::reg32>(cpu.esp-4) = 8007092 /*0x7a2db4*/;
    cpu.esp -= 4;
    // 00454c12  6864985300             -push 0x539864
    app->getMemory<x86::reg32>(cpu.esp-4) = 5478500 /*0x539864*/;
    cpu.esp -= 4;
    // 00454c17  8d8500ffffff           -lea eax, [ebp - 0x100]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-256) /* -0x100 */);
    // 00454c1d  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00454c1e  e86daa0800             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454c23  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 00454c26  8d8500ffffff           -lea eax, [ebp - 0x100]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-256) /* -0x100 */);
    // 00454c2c  e85f60ffff             -call 0x44ac90
    cpu.esp -= 4;
    nfs3hp::port::buttons::loadShapes(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454c31  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00454c33  f605583a7a0002         +test byte ptr [0x7a3a58], 2
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(x86::reg32(8010328) /* 0x7a3a58 */) & 2 /*0x2*/));
    // 00454c3a  7462                   -je 0x454c9e
    if (cpu.flags.zf)
    {
        goto L_0x00454c9e;
    }
    // 00454c3c  baa4985300             -mov edx, 0x5398a4
    cpu.edx = 5478564 /*0x5398a4*/;
    // 00454c41  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00454c43  e898a50900             -call 0x4ef1e0
    cpu.esp -= 4;
    sub_4ef1e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454c48  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00454c4a  e841170800             -call 0x4d6390
    cpu.esp -= 4;
    nfs3hp::port::shapes::artShapeTextures(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454c4f  baac985300             -mov edx, 0x5398ac
    cpu.edx = 5478572 /*0x5398ac*/;
    // 00454c54  a3b4955500             -mov dword ptr [0x5595b4], eax
    app->getMemory<x86::reg32>(x86::reg32(5608884) /* 0x5595b4 */) = cpu.eax;
    // 00454c59  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00454c5b  e880a50900             -call 0x4ef1e0
    cpu.esp -= 4;
    sub_4ef1e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454c60  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00454c62  e829170800             -call 0x4d6390
    cpu.esp -= 4;
    nfs3hp::port::shapes::artShapeTextures(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454c67  bab4985300             -mov edx, 0x5398b4
    cpu.edx = 5478580 /*0x5398b4*/;
    // 00454c6c  a3b8955500             -mov dword ptr [0x5595b8], eax
    app->getMemory<x86::reg32>(x86::reg32(5608888) /* 0x5595b8 */) = cpu.eax;
    // 00454c71  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00454c73  e868a50900             -call 0x4ef1e0
    cpu.esp -= 4;
    sub_4ef1e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454c78  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00454c7a  e811170800             -call 0x4d6390
    cpu.esp -= 4;
    nfs3hp::port::shapes::artShapeTextures(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454c7f  babc985300             -mov edx, 0x5398bc
    cpu.edx = 5478588 /*0x5398bc*/;
    // 00454c84  a3bc955500             -mov dword ptr [0x5595bc], eax
    app->getMemory<x86::reg32>(x86::reg32(5608892) /* 0x5595bc */) = cpu.eax;
    // 00454c89  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00454c8b  e850a50900             -call 0x4ef1e0
    cpu.esp -= 4;
    sub_4ef1e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454c90  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 00454c92  e8f9160800             -call 0x4d6390
    cpu.esp -= 4;
    nfs3hp::port::shapes::artShapeTextures(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454c97  a3c0955500             -mov dword ptr [0x5595c0], eax
    app->getMemory<x86::reg32>(x86::reg32(5608896) /* 0x5595c0 */) = cpu.eax;
    // 00454c9c  eb41                   -jmp 0x454cdf
    goto L_0x00454cdf;
L_0x00454c9e:
    // 00454c9e  bac4985300             -mov edx, 0x5398c4
    cpu.edx = 5478596 /*0x5398c4*/;
    // 00454ca3  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00454ca5  e836a50900             -call 0x4ef1e0
    cpu.esp -= 4;
    sub_4ef1e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454caa  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00454cac  e8df160800             -call 0x4d6390
    cpu.esp -= 4;
    nfs3hp::port::shapes::artShapeTextures(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454cb1  bacc985300             -mov edx, 0x5398cc
    cpu.edx = 5478604 /*0x5398cc*/;
    // 00454cb6  a3b4955500             -mov dword ptr [0x5595b4], eax
    app->getMemory<x86::reg32>(x86::reg32(5608884) /* 0x5595b4 */) = cpu.eax;
    // 00454cbb  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00454cbd  e81ea50900             -call 0x4ef1e0
    cpu.esp -= 4;
    sub_4ef1e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454cc2  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00454cc4  e8c7160800             -call 0x4d6390
    cpu.esp -= 4;
    nfs3hp::port::shapes::artShapeTextures(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454cc9  8b0db4955500           -mov ecx, dword ptr [0x5595b4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5608884) /* 0x5595b4 */);
    // 00454ccf  a3b8955500             -mov dword ptr [0x5595b8], eax
    app->getMemory<x86::reg32>(x86::reg32(5608888) /* 0x5595b8 */) = cpu.eax;
    // 00454cd4  a3c0955500             -mov dword ptr [0x5595c0], eax
    app->getMemory<x86::reg32>(x86::reg32(5608896) /* 0x5595c0 */) = cpu.eax;
    // 00454cd9  890dbc955500           -mov dword ptr [0x5595bc], ecx
    app->getMemory<x86::reg32>(x86::reg32(5608892) /* 0x5595bc */) = cpu.ecx;
L_0x00454cdf:
    // 00454cdf  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00454ce1  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454ce2  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454ce3  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454ce4  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_454cf0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00454cf0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00454cf1  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00454cf2  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00454cf4  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00454cf6  e8b57c0100             -call 0x46c9b0
    cpu.esp -= 4;
    nfs3hp::frontend::sliderValue(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454cfb  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00454cfe  d91c24                 -fstp dword ptr [esp]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu1 = cpu.fpu.st(0);
    app->getMemory<float>(cpu.esp) = float(fpu1);
    cpu.fpu.count -= 1;
    }
    // 00454d01  e8daf90000             -call 0x4646e0
    cpu.esp -= 4;
    nfs3hp::port::view::storeIncremented(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454d06  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454d07  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454d08  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_454d10(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00454d10  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00454d11  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00454d12  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00454d13  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00454d15  bad4985300             -mov edx, 0x5398d4
    cpu.edx = 5478612 /*0x5398d4*/;
    // 00454d1a  e821ddfeff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454d1f  baf04c4500             -mov edx, 0x454cf0
    cpu.edx = 4541680 /*0x454cf0*/;
    // 00454d24  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00454d26  e8957d0100             -call 0x46cac0
    cpu.esp -= 4;
    nfs3hp::frontend::sliderSetOnMove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454d2b  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 00454d2d  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00454d2f  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 00454d31  e81a7c0100             -call 0x46c950
    cpu.esp -= 4;
    nfs3hp::port::fields::sliderSetValue(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454d36  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00454d38  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454d39  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454d3a  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454d3b  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_454d40(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00454d40  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00454d41  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00454d43  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00454d45  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454d46  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_454d50(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00454d50  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00454d51  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00454d53  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00454d55  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454d56  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_454d60(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00454d60  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00454d61  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00454d62  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00454d63  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00454d64  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00454d65  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00454d66  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00454d68  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00454d6a  bae0985300             -mov edx, 0x5398e0
    cpu.edx = 5478624 /*0x5398e0*/;
    // 00454d6f  e83c61ffff             -call 0x44aeb0
    cpu.esp -= 4;
    nfs3hp::port::loop::currentMenu(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454d74  e8c7dcfeff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454d79  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00454d7b  a194476600             -mov eax, dword ptr [0x664794]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6702996) /* 0x664794 */);
    // 00454d80  8b0445da635500         -mov eax, dword ptr [eax*2 + 0x5563da]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5596122) /* 0x5563da */ + cpu.eax * 2);
    // 00454d87  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00454d8a  e8c1ca0700             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454d8f  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00454d90  68e8985300             -push 0x5398e8
    app->getMemory<x86::reg32>(cpu.esp-4) = 5478632 /*0x5398e8*/;
    cpu.esp -= 4;
    // 00454d95  6800476600             -push 0x664700
    app->getMemory<x86::reg32>(cpu.esp-4) = 6702848 /*0x664700*/;
    cpu.esp -= 4;
    // 00454d9a  e8f1a80800             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454d9f  c7423c00476600         -mov dword ptr [edx + 0x3c], 0x664700
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(60) /* 0x3c */) = 6702848 /*0x664700*/;
    // 00454da6  8b154cbb6f00           -mov edx, dword ptr [0x6fbb4c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7322444) /* 0x6fbb4c */);
    // 00454dac  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 00454daf  83fa01                 +cmp edx, 1
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
    // 00454db2  7545                   -jne 0x454df9
    if (!cpu.flags.zf)
    {
        goto L_0x00454df9;
    }
    // 00454db4  a194476600             -mov eax, dword ptr [0x664794]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6702996) /* 0x664794 */);
    // 00454db9  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00454dbb  bb04000000             -mov ebx, 4
    cpu.ebx = 4 /*0x4*/;
    // 00454dc0  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00454dc2  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00454dc5  f7fb                   -idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 00454dc7  8b0455da635500         -mov eax, dword ptr [edx*2 + 0x5563da]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5596122) /* 0x5563da */ + cpu.edx * 2);
    // 00454dce  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00454dd1  bf80466600             -mov edi, 0x664680
    cpu.edi = 6702720 /*0x664680*/;
    // 00454dd6  e875ca0700             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454ddb  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00454ddd  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
L_0x00454dde:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00454dde  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 00454de0  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 00454de2  3c00                   +cmp al, 0
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 00454de4  7410                   -je 0x454df6
    if (cpu.flags.zf)
    {
        goto L_0x00454df6;
    }
    // 00454de6  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 00454de9  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 00454dec  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 00454def  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 00454df2  3c00                   +cmp al, 0
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 00454df4  75e8                   -jne 0x454dde
    if (!cpu.flags.zf)
    {
        goto L_0x00454dde;
    }
L_0x00454df6:
    // 00454df6  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454df7  eb36                   -jmp 0x454e2f
    goto L_0x00454e2f;
L_0x00454df9:
    // 00454df9  a194476600             -mov eax, dword ptr [0x664794]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6702996) /* 0x664794 */);
    // 00454dfe  8d5001                 -lea edx, [eax + 1]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(1) /* 0x1 */);
    // 00454e01  bb04000000             -mov ebx, 4
    cpu.ebx = 4 /*0x4*/;
    // 00454e06  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00454e08  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00454e0b  f7fb                   -idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 00454e0d  8b0455da635500         -mov eax, dword ptr [edx*2 + 0x5563da]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5596122) /* 0x5563da */ + cpu.edx * 2);
    // 00454e14  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00454e17  e834ca0700             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454e1c  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00454e1d  68ec985300             -push 0x5398ec
    app->getMemory<x86::reg32>(cpu.esp-4) = 5478636 /*0x5398ec*/;
    cpu.esp -= 4;
    // 00454e22  6880466600             -push 0x664680
    app->getMemory<x86::reg32>(cpu.esp-4) = 6702720 /*0x664680*/;
    cpu.esp -= 4;
    // 00454e27  e864a80800             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454e2c  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
L_0x00454e2f:
    // 00454e2f  c7413c80466600         -mov dword ptr [ecx + 0x3c], 0x664680
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(60) /* 0x3c */) = 6702720 /*0x664680*/;
    // 00454e36  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454e37  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454e38  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454e39  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454e3a  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454e3b  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454e3c  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_454e40(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00454e40  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00454e41  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00454e42  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00454e43  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00454e44  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00454e45  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00454e47  8b0d94476600           -mov ecx, dword ptr [0x664794]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(6702996) /* 0x664794 */);
    // 00454e4d  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00454e4f  41                     -inc ecx
    (cpu.ecx)++;
    // 00454e50  be04000000             -mov esi, 4
    cpu.esi = 4 /*0x4*/;
    // 00454e55  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 00454e57  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00454e59  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00454e5c  f7fe                   -idiv esi
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.esi);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 00454e5e  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 00454e60  891594476600           -mov dword ptr [0x664794], edx
    app->getMemory<x86::reg32>(x86::reg32(6702996) /* 0x664794 */) = cpu.edx;
    // 00454e66  e8f5feffff             -call 0x454d60
    cpu.esp -= 4;
    sub_454d60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454e6b  8b0d94476600           -mov ecx, dword ptr [0x664794]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(6702996) /* 0x664794 */);
    // 00454e71  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00454e73  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454e74  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454e75  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454e76  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454e77  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454e78  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_454e80(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00454e80  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00454e81  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00454e82  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00454e84  8b1580476600           -mov edx, dword ptr [0x664780]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(6702976) /* 0x664780 */);
    // 00454e8a  891594476600           -mov dword ptr [0x664794], edx
    app->getMemory<x86::reg32>(x86::reg32(6702996) /* 0x664794 */) = cpu.edx;
    // 00454e90  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00454e92  891598476600           -mov dword ptr [0x664798], edx
    app->getMemory<x86::reg32>(x86::reg32(6703000) /* 0x664798 */) = cpu.edx;
    // 00454e98  31d0                   -xor eax, edx
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00454e9a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454e9b  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454e9c  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_454ea0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00454ea0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00454ea1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00454ea2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00454ea3  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00454ea4  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00454ea6  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00454ea8  a194476600             -mov eax, dword ptr [0x664794]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6702996) /* 0x664794 */);
    // 00454ead  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00454eaf  a380476600             -mov dword ptr [0x664780], eax
    app->getMemory<x86::reg32>(x86::reg32(6702976) /* 0x664780 */) = cpu.eax;
    // 00454eb4  891594476600           -mov dword ptr [0x664794], edx
    app->getMemory<x86::reg32>(x86::reg32(6702996) /* 0x664794 */) = cpu.edx;
    // 00454eba  891598476600           -mov dword ptr [0x664798], edx
    app->getMemory<x86::reg32>(x86::reg32(6703000) /* 0x664798 */) = cpu.edx;
    // 00454ec0  bad4985300             -mov edx, 0x5398d4
    cpu.edx = 5478612 /*0x5398d4*/;
    // 00454ec5  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00454ec7  e874dbfeff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454ecc  baf04c4500             -mov edx, 0x454cf0
    cpu.edx = 4541680 /*0x454cf0*/;
    // 00454ed1  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00454ed3  e8e87b0100             -call 0x46cac0
    cpu.esp -= 4;
    nfs3hp::frontend::sliderSetOnMove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454ed8  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 00454eda  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 00454edc  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 00454ede  baf4985300             -mov edx, 0x5398f4
    cpu.edx = 5478644 /*0x5398f4*/;
    // 00454ee3  e8687a0100             -call 0x46c950
    cpu.esp -= 4;
    nfs3hp::port::fields::sliderSetValue(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454ee8  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00454eea  e851dbfeff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454eef  c74064404e4500         -mov dword ptr [eax + 0x64], 0x454e40
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(100) /* 0x64 */) = 4542016 /*0x454e40*/;
    // 00454ef6  e865feffff             -call 0x454d60
    cpu.esp -= 4;
    sub_454d60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00454efb  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00454efd  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454efe  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454eff  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454f00  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00454f01  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
/* data blob: 1e001900140010000c000800040000000c00000008000000060000000500000004000000030000000200000001000000140000000f0000000c0000000a0000000800000004000000030000000200000008000000070000000600000005000000040000000300000002000000010000000a00000009000000080000000700000006000000050000000300000002000000100000000800000006000000000000000000000000000000000000000000000014000000100000000c00000000000000000000000000000000000000000000000100000004000000080000000a00000014000000280000003c0000005000000002000000080000001000000014000000190000003c0000004b00000064000000 */
void Application::sub_455020(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00455020  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00455021  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00455023  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00455025  742a                   -je 0x455051
    if (cpu.flags.zf)
    {
        goto L_0x00455051;
    }
    // 00455027  f6800002000004         +test byte ptr [eax + 0x200], 4
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(512) /* 0x200 */) & 4 /*0x4*/));
    // 0045502e  7421                   -je 0x455051
    if (cpu.flags.zf)
    {
        goto L_0x00455051;
    }
    // 00455030  833db0d36f0001         +cmp dword ptr [0x6fd3b0], 1
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
    // 00455037  7e18                   -jle 0x455051
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00455051;
    }
    // 00455039  8b80f0010000           -mov eax, dword ptr [eax + 0x1f0]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(496) /* 0x1f0 */);
    // 0045503f  66833c45ca227a0000     +cmp word ptr [eax*2 + 0x7a22ca], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(x86::reg32(8004298) /* 0x7a22ca */ + cpu.eax * 2);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 00455048  7407                   -je 0x455051
    if (cpu.flags.zf)
    {
        goto L_0x00455051;
    }
    // 0045504a  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0045504f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455050  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00455051:
    // 00455051  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00455053  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455054  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_455060(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00455060  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00455061  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00455063  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455064  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_455070(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00455070  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00455071  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00455072  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00455073  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00455074  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00455075  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00455076  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00455078  e8039f0700             -call 0x4cef80
    cpu.esp -= 4;
    sub_4cef80(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0045507d  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0045507f  0f85ab000000           -jne 0x455130
    if (!cpu.flags.zf)
    {
        goto L_0x00455130;
    }
    // 00455085  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0045508a  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0045508c  e87f0b0000             -call 0x455c10
    cpu.esp -= 4;
    sub_455c10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00455091  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
L_0x00455093:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00455093  3b35307d6700           +cmp esi, dword ptr [0x677d30]
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(6782256) /* 0x677d30 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00455099  7d40                   -jge 0x4550db
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004550db;
    }
    // 0045509b  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0045509d  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0045509f  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004550a1  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004550a3  e8280c0000             -call 0x455cd0
    cpu.esp -= 4;
    sub_455cd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004550a8  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004550aa  742c                   -je 0x4550d8
    if (cpu.flags.zf)
    {
        goto L_0x004550d8;
    }
    // 004550ac  8b9020020000           -mov edx, dword ptr [eax + 0x220]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(544) /* 0x220 */);
    // 004550b2  8b4a10                 -mov ecx, dword ptr [edx + 0x10]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(16) /* 0x10 */);
    // 004550b5  668b1475104f4500       -mov dx, word ptr [esi*2 + 0x454f10]
    cpu.dx = app->getMemory<x86::reg16>(x86::reg32(4542224) /* 0x454f10 */ + cpu.esi * 2);
    // 004550bd  6689144d2ed46f00       -mov word ptr [ecx*2 + 0x6fd42e], dx
    app->getMemory<x86::reg16>(x86::reg32(7328814) /* 0x6fd42e */ + cpu.ecx * 2) = cpu.dx;
    // 004550c5  8b8020020000           -mov eax, dword ptr [eax + 0x220]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(544) /* 0x220 */);
    // 004550cb  8b4010                 -mov eax, dword ptr [eax + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 004550ce  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 004550d0  66013c454ed46f00       +add word ptr [eax*2 + 0x6fd44e], di
    {
        auto tmp1 = app->getMemory<x86::reg16>(x86::reg32(7328846) /* 0x6fd44e */ + cpu.eax * 2);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(cpu.di));
        x86::reg16 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) == (1 & (tmp2 >> 15));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
L_0x004550d8:
    // 004550d8  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004550d9  ebb8                   -jmp 0x455093
    goto L_0x00455093;
L_0x004550db:
    // 004550db  ba20000000             -mov edx, 0x20
    cpu.edx = 32 /*0x20*/;
    // 004550e0  b80ed46f00             -mov eax, 0x6fd40e
    cpu.eax = 7328782 /*0x6fd40e*/;
    // 004550e5  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 004550ea  e81db60800             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004550ef  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 004550f1  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 004550f3  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004550f5  31ff                   +xor edi, edi
    cpu.clear_co();
    cpu.set_szp((cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi))));
    // 004550f7  e8140b0000             -call 0x455c10
    cpu.esp -= 4;
    sub_455c10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004550fc  eb05                   -jmp 0x455103
    goto L_0x00455103;
L_0x004550fe:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004550fe  83fe08                 +cmp esi, 8
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(8 /*0x8*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00455101  7f26                   -jg 0x455129
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00455129;
    }
L_0x00455103:
    // 00455103  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 00455105  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00455107  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00455109  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0045510b  e8c00b0000             -call 0x455cd0
    cpu.esp -= 4;
    sub_455cd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00455110  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00455112  7412                   -je 0x455126
    if (cpu.flags.zf)
    {
        goto L_0x00455126;
    }
    // 00455114  8b8020020000           -mov eax, dword ptr [eax + 0x220]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(544) /* 0x220 */);
    // 0045511a  8b4010                 -mov eax, dword ptr [eax + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 0045511d  66893c450ed46f00       -mov word ptr [eax*2 + 0x6fd40e], di
    app->getMemory<x86::reg16>(x86::reg32(7328782) /* 0x6fd40e */ + cpu.eax * 2) = cpu.di;
    // 00455125  47                     +inc edi
    {
        x86::reg32& tmp = cpu.edi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
L_0x00455126:
    // 00455126  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00455127  ebd5                   -jmp 0x4550fe
    goto L_0x004550fe;
L_0x00455129:
    // 00455129  66ff050cd46f00         -inc word ptr [0x6fd40c]
    (app->getMemory<x86::reg16>(x86::reg32(7328780) /* 0x6fd40c */))++;
L_0x00455130:
    // 00455130  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455131  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455132  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455133  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455134  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455135  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455136  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_455140(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00455140  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00455141  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00455142  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00455143  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00455144  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00455145  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00455146  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00455148  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 0045514d  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0045514f  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 00455151  b902000000             -mov ecx, 2
    cpu.ecx = 2 /*0x2*/;
    // 00455156  e8b50a0000             -call 0x455c10
    cpu.esp -= 4;
    sub_455c10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0045515b  bb20000000             -mov ebx, 0x20
    cpu.ebx = 32 /*0x20*/;
    // 00455160  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00455162  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00455164  e8670b0000             -call 0x455cd0
    cpu.esp -= 4;
    sub_455cd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00455169  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0045516b  7426                   -je 0x455193
    if (cpu.flags.zf)
    {
        goto L_0x00455193;
    }
    // 0045516d  833d307d670001         +cmp dword ptr [0x677d30], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(6782256) /* 0x677d30 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00455174  7e1d                   -jle 0x455193
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00455193;
    }
    // 00455176  8bb020020000           -mov esi, dword ptr [eax + 0x220]
    cpu.esi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(544) /* 0x220 */);
    // 0045517c  8b7610                 -mov esi, dword ptr [esi + 0x10]
    cpu.esi = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(16) /* 0x10 */);
    // 0045517f  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00455181  66891475ded36f00       -mov word ptr [esi*2 + 0x6fd3de], dx
    app->getMemory<x86::reg16>(x86::reg32(7328734) /* 0x6fd3de */ + cpu.esi * 2) = cpu.dx;
    // 00455189  c780c402000002000000   -mov dword ptr [eax + 0x2c4], 2
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(708) /* 0x2c4 */) = 2 /*0x2*/;
L_0x00455193:
    // 00455193  ba20000000             -mov edx, 0x20
    cpu.edx = 32 /*0x20*/;
    // 00455198  b8bed36f00             -mov eax, 0x6fd3be
    cpu.eax = 7328702 /*0x6fd3be*/;
    // 0045519d  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 004551a2  e865b50800             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004551a7  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 004551a9  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 004551ab  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004551ad  31ff                   +xor edi, edi
    cpu.clear_co();
    cpu.set_szp((cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi))));
    // 004551af  e85c0a0000             -call 0x455c10
    cpu.esp -= 4;
    sub_455c10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004551b4  eb18                   -jmp 0x4551ce
    goto L_0x004551ce;
L_0x004551b6:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004551b6  8b8120020000           -mov eax, dword ptr [ecx + 0x220]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(544) /* 0x220 */);
    // 004551bc  8b4010                 -mov eax, dword ptr [eax + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 004551bf  66893c45bed36f00       -mov word ptr [eax*2 + 0x6fd3be], di
    app->getMemory<x86::reg16>(x86::reg32(7328702) /* 0x6fd3be */ + cpu.eax * 2) = cpu.di;
    // 004551c7  47                     -inc edi
    (cpu.edi)++;
L_0x004551c8:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004551c8  46                     -inc esi
    (cpu.esi)++;
    // 004551c9  83fe08                 +cmp esi, 8
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(8 /*0x8*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004551cc  7f32                   -jg 0x455200
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00455200;
    }
L_0x004551ce:
    // 004551ce  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 004551d0  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 004551d2  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004551d4  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004551d6  e8f50a0000             -call 0x455cd0
    cpu.esp -= 4;
    sub_455cd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004551db  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004551dd  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004551df  74e7                   -je 0x4551c8
    if (cpu.flags.zf)
    {
        goto L_0x004551c8;
    }
    // 004551e1  f6800002000008         +test byte ptr [eax + 0x200], 8
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(512) /* 0x200 */) & 8 /*0x8*/));
    // 004551e8  74cc                   -je 0x4551b6
    if (cpu.flags.zf)
    {
        goto L_0x004551b6;
    }
    // 004551ea  8b8020020000           -mov eax, dword ptr [eax + 0x220]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(544) /* 0x220 */);
    // 004551f0  8b4010                 -mov eax, dword ptr [eax + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 004551f3  66833c45ded36f0000     +cmp word ptr [eax*2 + 0x6fd3de], 0
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
    // 004551fc  74ca                   -je 0x4551c8
    if (cpu.flags.zf)
    {
        goto L_0x004551c8;
    }
    // 004551fe  ebb6                   -jmp 0x4551b6
    goto L_0x004551b6;
L_0x00455200:
    // 00455200  66ff05bcd36f00         -inc word ptr [0x6fd3bc]
    (app->getMemory<x86::reg16>(x86::reg32(7328700) /* 0x6fd3bc */))++;
    // 00455207  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455208  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455209  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045520a  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045520b  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045520c  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045520d  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_455210(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00455210  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00455211  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00455212  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00455213  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00455214  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00455215  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00455217  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0045521a  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0045521d  c70200000000           -mov dword ptr [edx], 0
    app->getMemory<x86::reg32>(cpu.edx) = 0 /*0x0*/;
    // 00455223  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
L_0x00455225:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00455225  3b1d9cfd5e00           +cmp ebx, dword ptr [0x5efd9c]
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(6225308) /* 0x5efd9c */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0045522b  7d4a                   -jge 0x455277
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00455277;
    }
    // 0045522d  8d349d00000000         -lea esi, [ebx*4]
    cpu.esi = x86::reg32(cpu.ebx * 4);
    // 00455234  8b86c8fa5e00           -mov eax, dword ptr [esi + 0x5efac8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(6224584) /* 0x5efac8 */);
    // 0045523a  f6800002000001         +test byte ptr [eax + 0x200], 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(512) /* 0x200 */) & 1 /*0x1*/));
    // 00455241  7431                   -je 0x455274
    if (cpu.flags.zf)
    {
        goto L_0x00455274;
    }
    // 00455243  e8d8fdffff             -call 0x455020
    cpu.esp -= 4;
    sub_455020(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00455248  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0045524a  7528                   -jne 0x455274
    if (!cpu.flags.zf)
    {
        goto L_0x00455274;
    }
    // 0045524c  6902ac090000           -imul eax, dword ptr [edx], 0x9ac
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(app->getMemory<x86::reg32>(cpu.edx))) * x86::sreg64(x86::sreg32(2476 /*0x9ac*/)));
    // 00455252  b9ac090000             -mov ecx, 0x9ac
    cpu.ecx = 2476 /*0x9ac*/;
    // 00455257  8b7dfc                 -mov edi, dword ptr [ebp - 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0045525a  8bb6c8fa5e00           -mov esi, dword ptr [esi + 0x5efac8]
    cpu.esi = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(6224584) /* 0x5efac8 */);
    // 00455260  01c7                   -add edi, eax
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.eax));
    // 00455262  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00455263  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00455265  c1e902                 -shr ecx, 2
    cpu.ecx >>= 2 /*0x2*/ % 32;
    // 00455268  f2a5                   -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0045526a  8ac8                   -mov cl, al
    cpu.cl = cpu.al;
    // 0045526c  80e103                 +and cl, 3
    cpu.clear_co();
    cpu.set_szp((cpu.cl &= x86::reg8(x86::sreg8(3 /*0x3*/))));
    // 0045526f  f2a4                   -repne movsb byte ptr es:[edi], byte ptr [esi]
    while (cpu.ecx)
    {
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
        --cpu.ecx;
        if (cpu.flags.zf)
            break;
    }
    // 00455271  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455272  ff02                   +inc dword ptr [edx]
    {
        auto tmp = app->getMemory<x86::reg32>(cpu.edx);
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
L_0x00455274:
    // 00455274  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00455275  ebae                   -jmp 0x455225
    goto L_0x00455225;
L_0x00455277:
    // 00455277  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00455279  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045527a  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045527b  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045527c  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045527d  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045527e  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x90 */
void Application::sub_455280(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00455280  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00455281  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00455282  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00455283  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00455284  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00455285  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00455287  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0045528a  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0045528d  c70200000000           -mov dword ptr [edx], 0
    app->getMemory<x86::reg32>(cpu.edx) = 0 /*0x0*/;
    // 00455293  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
L_0x00455295:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00455295  3b1d9cfd5e00           +cmp ebx, dword ptr [0x5efd9c]
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(6225308) /* 0x5efd9c */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0045529b  7d4a                   -jge 0x4552e7
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004552e7;
    }
    // 0045529d  8d349d00000000         -lea esi, [ebx*4]
    cpu.esi = x86::reg32(cpu.ebx * 4);
    // 004552a4  8b86c8fa5e00           -mov eax, dword ptr [esi + 0x5efac8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(6224584) /* 0x5efac8 */);
    // 004552aa  f6800002000020         +test byte ptr [eax + 0x200], 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(512) /* 0x200 */) & 32 /*0x20*/));
    // 004552b1  7431                   -je 0x4552e4
    if (cpu.flags.zf)
    {
        goto L_0x004552e4;
    }
    // 004552b3  e868fdffff             -call 0x455020
    cpu.esp -= 4;
    sub_455020(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004552b8  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004552ba  7528                   -jne 0x4552e4
    if (!cpu.flags.zf)
    {
        goto L_0x004552e4;
    }
    // 004552bc  6902ac090000           -imul eax, dword ptr [edx], 0x9ac
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(app->getMemory<x86::reg32>(cpu.edx))) * x86::sreg64(x86::sreg32(2476 /*0x9ac*/)));
    // 004552c2  b9ac090000             -mov ecx, 0x9ac
    cpu.ecx = 2476 /*0x9ac*/;
    // 004552c7  8b7dfc                 -mov edi, dword ptr [ebp - 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004552ca  8bb6c8fa5e00           -mov esi, dword ptr [esi + 0x5efac8]
    cpu.esi = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(6224584) /* 0x5efac8 */);
    // 004552d0  01c7                   -add edi, eax
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.eax));
    // 004552d2  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004552d3  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004552d5  c1e902                 -shr ecx, 2
    cpu.ecx >>= 2 /*0x2*/ % 32;
    // 004552d8  f2a5                   -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004552da  8ac8                   -mov cl, al
    cpu.cl = cpu.al;
    // 004552dc  80e103                 +and cl, 3
    cpu.clear_co();
    cpu.set_szp((cpu.cl &= x86::reg8(x86::sreg8(3 /*0x3*/))));
    // 004552df  f2a4                   -repne movsb byte ptr es:[edi], byte ptr [esi]
    while (cpu.ecx)
    {
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
        --cpu.ecx;
        if (cpu.flags.zf)
            break;
    }
    // 004552e1  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004552e2  ff02                   +inc dword ptr [edx]
    {
        auto tmp = app->getMemory<x86::reg32>(cpu.edx);
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
L_0x004552e4:
    // 004552e4  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004552e5  ebae                   -jmp 0x455295
    goto L_0x00455295;
L_0x004552e7:
    // 004552e7  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004552e9  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004552ea  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004552eb  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004552ec  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004552ed  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004552ee  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x90 */
void Application::sub_4552f0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004552f0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004552f1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004552f2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004552f3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004552f4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004552f5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004552f6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004552f8  833db8d36f0002         +cmp dword ptr [0x6fd3b8], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328696) /* 0x6fd3b8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004552ff  753f                   -jne 0x455340
    if (!cpu.flags.zf)
    {
        goto L_0x00455340;
    }
    // 00455301  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 00455306  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00455308  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 0045530a  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 0045530c  e8ff080000             -call 0x455c10
    cpu.esp -= 4;
    sub_455c10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00455311:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00455311  3b35307d6700           +cmp esi, dword ptr [0x677d30]
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(6782256) /* 0x677d30 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00455317  7d27                   -jge 0x455340
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00455340;
    }
    // 00455319  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0045531b  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0045531d  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0045531f  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00455321  e8aa090000             -call 0x455cd0
    cpu.esp -= 4;
    sub_455cd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00455326  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00455328  7413                   -je 0x45533d
    if (cpu.flags.zf)
    {
        goto L_0x0045533d;
    }
    // 0045532a  f6800002000080         +test byte ptr [eax + 0x200], 0x80
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(512) /* 0x200 */) & 128 /*0x80*/));
    // 00455331  740a                   -je 0x45533d
    if (cpu.flags.zf)
    {
        goto L_0x0045533d;
    }
    // 00455333  c780c402000002000000   -mov dword ptr [eax + 0x2c4], 2
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(708) /* 0x2c4 */) = 2 /*0x2*/;
L_0x0045533d:
    // 0045533d  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0045533e  ebd1                   -jmp 0x455311
    goto L_0x00455311;
L_0x00455340:
    // 00455340  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 00455345  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00455347  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 00455349  b902000000             -mov ecx, 2
    cpu.ecx = 2 /*0x2*/;
    // 0045534e  e8bd080000             -call 0x455c10
    cpu.esp -= 4;
    sub_455c10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00455353  bb20000000             -mov ebx, 0x20
    cpu.ebx = 32 /*0x20*/;
    // 00455358  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0045535a  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0045535c  e86f090000             -call 0x455cd0
    cpu.esp -= 4;
    sub_455cd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00455361  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00455363  745f                   -je 0x4553c4
    if (cpu.flags.zf)
    {
        goto L_0x004553c4;
    }
    // 00455365  8bb038020000           -mov esi, dword ptr [eax + 0x238]
    cpu.esi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(568) /* 0x238 */);
    // 0045536b  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 0045536d  83c63c                 -add esi, 0x3c
    (cpu.esi) += x86::reg32(x86::sreg32(60 /*0x3c*/));
L_0x00455370:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00455370  3b3d307d6700           +cmp edi, dword ptr [0x677d30]
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(6782256) /* 0x677d30 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00455376  7d21                   -jge 0x455399
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00455399;
    }
    // 00455378  b902000000             -mov ecx, 2
    cpu.ecx = 2 /*0x2*/;
    // 0045537d  bb10000000             -mov ebx, 0x10
    cpu.ebx = 16 /*0x10*/;
    // 00455382  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 00455384  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00455386  e845090000             -call 0x455cd0
    cpu.esp -= 4;
    sub_455cd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0045538b  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0045538d  7407                   -je 0x455396
    if (cpu.flags.zf)
    {
        goto L_0x00455396;
    }
    // 0045538f  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00455390  89b038020000           -mov dword ptr [eax + 0x238], esi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(568) /* 0x238 */) = cpu.esi;
L_0x00455396:
    // 00455396  47                     +inc edi
    {
        x86::reg32& tmp = cpu.edi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00455397  ebd7                   -jmp 0x455370
    goto L_0x00455370;
L_0x00455399:
    // 00455399  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
L_0x0045539b:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0045539b  3b3d307d6700           +cmp edi, dword ptr [0x677d30]
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(6782256) /* 0x677d30 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004553a1  7d21                   -jge 0x4553c4
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004553c4;
    }
    // 004553a3  b904000000             -mov ecx, 4
    cpu.ecx = 4 /*0x4*/;
    // 004553a8  bb10000000             -mov ebx, 0x10
    cpu.ebx = 16 /*0x10*/;
    // 004553ad  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 004553af  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004553b1  e81a090000             -call 0x455cd0
    cpu.esp -= 4;
    sub_455cd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004553b6  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004553b8  7407                   -je 0x4553c1
    if (cpu.flags.zf)
    {
        goto L_0x004553c1;
    }
    // 004553ba  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004553bb  89b038020000           -mov dword ptr [eax + 0x238], esi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(568) /* 0x238 */) = cpu.esi;
L_0x004553c1:
    // 004553c1  47                     +inc edi
    {
        x86::reg32& tmp = cpu.edi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004553c2  ebd7                   -jmp 0x45539b
    goto L_0x0045539b;
L_0x004553c4:
    // 004553c4  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 004553c9  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004553cb  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004553cd  bf01000000             -mov edi, 1
    cpu.edi = 1 /*0x1*/;
    // 004553d2  e839080000             -call 0x455c10
    cpu.esp -= 4;
    sub_455c10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004553d7  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
L_0x004553d9:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004553d9  3b35307d6700           +cmp esi, dword ptr [0x677d30]
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(6782256) /* 0x677d30 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004553df  7d1b                   -jge 0x4553fc
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004553fc;
    }
    // 004553e1  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 004553e3  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 004553e5  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004553e7  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004553e9  e8e2080000             -call 0x455cd0
    cpu.esp -= 4;
    sub_455cd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004553ee  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004553f0  7406                   -je 0x4553f8
    if (cpu.flags.zf)
    {
        goto L_0x004553f8;
    }
    // 004553f2  89b8bc020000           -mov dword ptr [eax + 0x2bc], edi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(700) /* 0x2bc */) = cpu.edi;
L_0x004553f8:
    // 004553f8  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004553f9  47                     +inc edi
    {
        x86::reg32& tmp = cpu.edi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004553fa  ebdd                   -jmp 0x4553d9
    goto L_0x004553d9;
L_0x004553fc:
    // 004553fc  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004553fd  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004553fe  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004553ff  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455400  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455401  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455402  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_455410(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00455410  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00455411  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00455412  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00455413  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00455414  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00455416  833da0d36f0000         +cmp dword ptr [0x6fd3a0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328672) /* 0x6fd3a0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0045541d  0f85af000000           -jne 0x4554d2
    if (!cpu.flags.zf)
    {
        goto L_0x004554d2;
    }
    // 00455423  e8782a0700             -call 0x4c7ea0
    cpu.esp -= 4;
    sub_4c7ea0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00455428  e8a393ffff             -call 0x44e7d0
    cpu.esp -= 4;
    sub_44e7d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0045542d  e8dea0ffff             -call 0x44f510
    cpu.esp -= 4;
    nfs3hp::load::loadRecords(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00455432  ba307d6700             -mov edx, 0x677d30
    cpu.edx = 6782256 /*0x677d30*/;
    // 00455437  b8b0476600             -mov eax, 0x6647b0
    cpu.eax = 6703024 /*0x6647b0*/;
    // 0045543c  e8cffdffff             -call 0x455210
    cpu.esp -= 4;
    sub_455210(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00455441  ba347d6700             -mov edx, 0x677d34
    cpu.edx = 6782260 /*0x677d34*/;
    // 00455446  b870e26600             -mov eax, 0x66e270
    cpu.eax = 6742640 /*0x66e270*/;
    // 0045544b  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0045544d  e82efeffff             -call 0x455280
    cpu.esp -= 4;
    sub_455280(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00455452  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00455454  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00455456  e8b5070000             -call 0x455c10
    cpu.esp -= 4;
    sub_455c10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0045545b  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00455460  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00455462  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00455464  e8a7070000             -call 0x455c10
    cpu.esp -= 4;
    sub_455c10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00455469  e882feffff             -call 0x4552f0
    cpu.esp -= 4;
    sub_4552f0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0045546e  833db8d36f0001         +cmp dword ptr [0x6fd3b8], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328696) /* 0x6fd3b8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00455475  7505                   -jne 0x45547c
    if (!cpu.flags.zf)
    {
        goto L_0x0045547c;
    }
    // 00455477  e8f4fbffff             -call 0x455070
    cpu.esp -= 4;
    sub_455070(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0045547c:
    // 0045547c  833db8d36f0002         +cmp dword ptr [0x6fd3b8], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328696) /* 0x6fd3b8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00455483  7505                   -jne 0x45548a
    if (!cpu.flags.zf)
    {
        goto L_0x0045548a;
    }
    // 00455485  e8b6fcffff             -call 0x455140
    cpu.esp -= 4;
    sub_455140(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0045548a:
    // 0045548a  e821a5ffff             -call 0x44f9b0
    cpu.esp -= 4;
    sub_44f9b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0045548f  e8ec9a0700             -call 0x4cef80
    cpu.esp -= 4;
    sub_4cef80(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00455494  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00455496  751e                   -jne 0x4554b6
    if (!cpu.flags.zf)
    {
        goto L_0x004554b6;
    }
    // 00455498  e853beffff             -call 0x4512f0
    cpu.esp -= 4;
    sub_4512f0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0045549d  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0045549f  e87c93ffff             -call 0x44e820
    cpu.esp -= 4;
    sub_44e820(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004554a4  8b35b0d36f00           -mov esi, dword ptr [0x6fd3b0]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
    // 004554aa  83fe01                 +cmp esi, 1
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
    // 004554ad  7507                   -jne 0x4554b6
    if (!cpu.flags.zf)
    {
        goto L_0x004554b6;
    }
    // 004554af  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004554b1  e86a93ffff             -call 0x44e820
    cpu.esp -= 4;
    sub_44e820(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004554b6:
    // 004554b6  e835a1ffff             -call 0x44f5f0
    cpu.esp -= 4;
    nfs3hp::load::saveRecords(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004554bb  833db0d36f0001         +cmp dword ptr [0x6fd3b0], 1
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
    // 004554c2  7e0e                   -jle 0x4554d2
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004554d2;
    }
    // 004554c4  e8b79a0700             -call 0x4cef80
    cpu.esp -= 4;
    sub_4cef80(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004554c9  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004554cb  7505                   -jne 0x4554d2
    if (!cpu.flags.zf)
    {
        goto L_0x004554d2;
    }
    // 004554cd  e88efbffff             -call 0x455060
    cpu.esp -= 4;
    sub_455060(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004554d2:
    // 004554d2  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004554d3  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004554d4  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004554d5  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004554d6  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_4554e0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004554e0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004554e1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004554e2  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004554e3  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004554e4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004554e5  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004554e7  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004554ea  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004554ec  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 004554ef  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x004554f1:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004554f1  3b048d307d6700         +cmp eax, dword ptr [ecx*4 + 0x677d30]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(6782256) /* 0x677d30 */ + cpu.ecx * 4)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004554f8  7d28                   -jge 0x455522
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00455522;
    }
    // 004554fa  69d0ac090000           -imul edx, eax, 0x9ac
    cpu.edx = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(2476 /*0x9ac*/)));
    // 00455500  69d9c09a0000           -imul ebx, ecx, 0x9ac0
    cpu.ebx = x86::reg32(x86::sreg64(x86::sreg32(cpu.ecx)) * x86::sreg64(x86::sreg32(39616 /*0x9ac0*/)));
    // 00455506  8d3413                 -lea esi, [ebx + edx]
    cpu.esi = x86::reg32(cpu.ebx + cpu.edx * 1);
    // 00455509  8b7dfc                 -mov edi, dword ptr [ebp - 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0045550c  3bbea0496600           +cmp edi, dword ptr [esi + 0x6649a0]
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.esi + x86::reg32(6703520) /* 0x6649a0 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00455512  750b                   -jne 0x45551f
    if (!cpu.flags.zf)
    {
        goto L_0x0045551f;
    }
    // 00455514  b8b0476600             -mov eax, 0x6647b0
    cpu.eax = 6703024 /*0x6647b0*/;
    // 00455519  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 0045551b  01d0                   +add eax, edx
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
    // 0045551d  eb05                   -jmp 0x455524
    goto L_0x00455524;
L_0x0045551f:
    // 0045551f  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00455520  ebcf                   -jmp 0x4554f1
    goto L_0x004554f1;
L_0x00455522:
    // 00455522  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x00455524:
    // 00455524  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00455526  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455527  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455528  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455529  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045552a  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045552b  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_455530(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00455530  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00455531  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00455532  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00455534  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00455536  742a                   -je 0x455562
    if (cpu.flags.zf)
    {
        goto L_0x00455562;
    }
    // 00455538  8b8820020000           -mov ecx, dword ptr [eax + 0x220]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(544) /* 0x220 */);
    // 0045553e  8b4910                 -mov ecx, dword ptr [ecx + 0x10]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(16) /* 0x10 */);
    // 00455541  8b0c4d2cd46f00         -mov ecx, dword ptr [ecx*2 + 0x6fd42c]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7328812) /* 0x6fd42c */ + cpu.ecx * 2);
    // 00455548  c1f910                 -sar ecx, 0x10
    cpu.ecx = x86::reg32(x86::sreg32(cpu.ecx) >> (16 /*0x10*/ % 32));
    // 0045554b  890a                   -mov dword ptr [edx], ecx
    app->getMemory<x86::reg32>(cpu.edx) = cpu.ecx;
    // 0045554d  8b8020020000           -mov eax, dword ptr [eax + 0x220]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(544) /* 0x220 */);
    // 00455553  8b4010                 -mov eax, dword ptr [eax + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 00455556  8b04454cd46f00         -mov eax, dword ptr [eax*2 + 0x6fd44c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328844) /* 0x6fd44c */ + cpu.eax * 2);
    // 0045555d  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00455560  8903                   -mov dword ptr [ebx], eax
    app->getMemory<x86::reg32>(cpu.ebx) = cpu.eax;
L_0x00455562:
    // 00455562  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455563  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455564  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_455570(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00455570  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00455571  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00455572  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00455573  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00455575  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 00455577  c60200                 -mov byte ptr [edx], 0
    app->getMemory<x86::reg8>(cpu.edx) = 0 /*0x0*/;
    // 0045557a  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0045557c  743b                   -je 0x4555b9
    if (cpu.flags.zf)
    {
        goto L_0x004555b9;
    }
    // 0045557e  8b90f0010000           -mov edx, dword ptr [eax + 0x1f0]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(496) /* 0x1f0 */);
    // 00455584  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 0045558b  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0045558d  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00455590  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00455592  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 00455595  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00455597  052cd56f00             -add eax, 0x6fd52c
    (cpu.eax) += x86::reg32(x86::sreg32(7329068 /*0x6fd52c*/));
    // 0045559c  8d7034                 -lea esi, [eax + 0x34]
    cpu.esi = x86::reg32(cpu.eax + x86::reg32(52) /* 0x34 */);
    // 0045559f  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
L_0x004555a0:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004555a0  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 004555a2  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 004555a4  3c00                   +cmp al, 0
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 004555a6  7410                   -je 0x4555b8
    if (cpu.flags.zf)
    {
        goto L_0x004555b8;
    }
    // 004555a8  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 004555ab  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004555ae  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 004555b1  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004555b4  3c00                   +cmp al, 0
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 004555b6  75e8                   -jne 0x4555a0
    if (!cpu.flags.zf)
    {
        goto L_0x004555a0;
    }
L_0x004555b8:
    // 004555b8  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
L_0x004555b9:
    // 004555b9  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004555ba  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004555bb  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004555bc  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_4555c0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004555c0  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004555c1  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004555c2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004555c3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004555c5  8b80bc020000           -mov eax, dword ptr [eax + 0x2bc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(700) /* 0x2bc */);
    // 004555cb  83c02f                 -add eax, 0x2f
    (cpu.eax) += x86::reg32(x86::sreg32(47 /*0x2f*/));
    // 004555ce  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 004555d0  e87bc20700             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004555d5  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004555d7  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
L_0x004555d8:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004555d8  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 004555da  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 004555dc  3c00                   +cmp al, 0
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 004555de  7410                   -je 0x4555f0
    if (cpu.flags.zf)
    {
        goto L_0x004555f0;
    }
    // 004555e0  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 004555e3  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004555e6  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 004555e9  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004555ec  3c00                   +cmp al, 0
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 004555ee  75e8                   -jne 0x4555d8
    if (!cpu.flags.zf)
    {
        goto L_0x004555d8;
    }
L_0x004555f0:
    // 004555f0  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004555f1  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004555f2  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004555f3  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004555f4  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_455600(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00455600  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00455601  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00455602  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00455603  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00455605  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 00455607  c60200                 -mov byte ptr [edx], 0
    app->getMemory<x86::reg8>(cpu.edx) = 0 /*0x0*/;
    // 0045560a  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0045560c  7442                   -je 0x455650
    if (cpu.flags.zf)
    {
        goto L_0x00455650;
    }
    // 0045560e  8b90f0010000           -mov edx, dword ptr [eax + 0x1f0]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(496) /* 0x1f0 */);
    // 00455614  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 0045561b  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0045561d  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00455620  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00455622  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 00455625  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00455627  052cd56f00             -add eax, 0x6fd52c
    (cpu.eax) += x86::reg32(x86::sreg32(7329068 /*0x6fd52c*/));
    // 0045562c  8b4014                 -mov eax, dword ptr [eax + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(20) /* 0x14 */);
    // 0045562f  e81c3cfeff             -call 0x439250
    cpu.esp -= 4;
    nfs3hp::load::carName(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00455634  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00455636  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
L_0x00455637:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00455637  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 00455639  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 0045563b  3c00                   +cmp al, 0
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0045563d  7410                   -je 0x45564f
    if (cpu.flags.zf)
    {
        goto L_0x0045564f;
    }
    // 0045563f  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 00455642  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 00455645  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 00455648  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0045564b  3c00                   +cmp al, 0
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0045564d  75e8                   -jne 0x455637
    if (!cpu.flags.zf)
    {
        goto L_0x00455637;
    }
L_0x0045564f:
    // 0045564f  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
L_0x00455650:
    // 00455650  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455651  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455652  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455653  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_455660(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00455660  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00455661  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00455662  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00455663  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00455665  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 00455667  c60200                 -mov byte ptr [edx], 0
    app->getMemory<x86::reg8>(cpu.edx) = 0 /*0x0*/;
    // 0045566a  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0045566c  7444                   -je 0x4556b2
    if (cpu.flags.zf)
    {
        goto L_0x004556b2;
    }
    // 0045566e  8b90f0010000           -mov edx, dword ptr [eax + 0x1f0]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(496) /* 0x1f0 */);
    // 00455674  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 0045567b  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0045567d  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00455680  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00455682  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 00455685  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00455687  052cd56f00             -add eax, 0x6fd52c
    (cpu.eax) += x86::reg32(x86::sreg32(7329068 /*0x6fd52c*/));
    // 0045568c  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0045568e  8b4014                 -mov eax, dword ptr [eax + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(20) /* 0x14 */);
    // 00455691  e81a44feff             -call 0x439ab0
    cpu.esp -= 4;
    nfs3hp::port::fields::carText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00455696  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00455698  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
L_0x00455699:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00455699  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 0045569b  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 0045569d  3c00                   +cmp al, 0
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0045569f  7410                   -je 0x4556b1
    if (cpu.flags.zf)
    {
        goto L_0x004556b1;
    }
    // 004556a1  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 004556a4  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004556a7  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 004556aa  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004556ad  3c00                   +cmp al, 0
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 004556af  75e8                   -jne 0x455699
    if (!cpu.flags.zf)
    {
        goto L_0x00455699;
    }
L_0x004556b1:
    // 004556b1  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
L_0x004556b2:
    // 004556b2  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004556b3  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004556b4  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004556b5  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x54 0x22 0x00 */
void Application::sub_4556c0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004556c0  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004556c1  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004556c2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004556c3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004556c5  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 004556c7  c60200                 -mov byte ptr [edx], 0
    app->getMemory<x86::reg8>(cpu.edx) = 0 /*0x0*/;
    // 004556ca  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004556cc  7447                   -je 0x455715
    if (cpu.flags.zf)
    {
        goto L_0x00455715;
    }
    // 004556ce  8b90f0010000           -mov edx, dword ptr [eax + 0x1f0]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(496) /* 0x1f0 */);
    // 004556d4  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004556db  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004556dd  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004556e0  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004556e2  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 004556e5  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004556e7  052cd56f00             -add eax, 0x6fd52c
    (cpu.eax) += x86::reg32(x86::sreg32(7329068 /*0x6fd52c*/));
    // 004556ec  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 004556f1  8b4014                 -mov eax, dword ptr [eax + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(20) /* 0x14 */);
    // 004556f4  e8b743feff             -call 0x439ab0
    cpu.esp -= 4;
    nfs3hp::port::fields::carText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004556f9  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004556fb  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
L_0x004556fc:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004556fc  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 004556fe  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 00455700  3c00                   +cmp al, 0
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 00455702  7410                   -je 0x455714
    if (cpu.flags.zf)
    {
        goto L_0x00455714;
    }
    // 00455704  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 00455707  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0045570a  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 0045570d  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 00455710  3c00                   +cmp al, 0
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 00455712  75e8                   -jne 0x4556fc
    if (!cpu.flags.zf)
    {
        goto L_0x004556fc;
    }
L_0x00455714:
    // 00455714  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
L_0x00455715:
    // 00455715  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455716  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455717  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455718  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_455720(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00455720  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00455721  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00455722  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00455724  83ec04                 +sub esp, 4
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00455727  8b80b8020000           -mov eax, dword ptr [eax + 0x2b8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(696) /* 0x2b8 */);
    // 0045572d  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 00455730  d945fc                 +fld dword ptr [ebp - 4]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu1 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
    // 00455733  d9e8                   +fld1 
    x86::Float fpu2 = 1.0;
    // 00455735  ded9                   +fcompp 
    cpu.fpu.compare(fpu2, fpu1);
    // 00455737  dfe0                   -fnstsw ax
    cpu.ax = cpu.fpu.status.word;
    // 00455739  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    }
    // 0045573a  7353                   -jae 0x45578f
    if (!cpu.flags.cf)
    {
        goto L_0x0045578f;
    }
    // 0045573c  8b0d68bc6f00           -mov ecx, dword ptr [0x6fbc68]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7322728) /* 0x6fbc68 */);
    // 00455742  83f903                 +cmp ecx, 3
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00455745  7405                   -je 0x45574c
    if (cpu.flags.zf)
    {
        goto L_0x0045574c;
    }
    // 00455747  83f904                 +cmp ecx, 4
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0045574a  7516                   -jne 0x455762
    if (!cpu.flags.zf)
    {
        goto L_0x00455762;
    }
L_0x0045574c:
    // 0045574c  b857010000             -mov eax, 0x157
    cpu.eax = 343 /*0x157*/;
    // 00455751  e8fac00700             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00455756  d945fc                 +fld dword ptr [ebp - 4]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu3 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
    // 00455759  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0045575a  d80d10995300           +fmul dword ptr [0x539910]
    x86::Float fpu4 = cpu.fpu.mul(fpu3, x86::Float(app->getMemory<float>(x86::reg32(5478672) /* 0x539910 */)));
    cpu.fpu.count += 1;
    cpu.fpu.st(0) = fpu4;
    }
    // 00455760  eb14                   -jmp 0x455776
    goto L_0x00455776;
L_0x00455762:
    // 00455762  b856010000             -mov eax, 0x156
    cpu.eax = 342 /*0x156*/;
    // 00455767  e8e4c00700             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0045576c  d945fc                 -fld dword ptr [ebp - 4]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu5 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
    // 0045576f  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00455770  d80d14995300           -fmul dword ptr [0x539914]
    x86::Float fpu6 = cpu.fpu.mul(fpu5, x86::Float(app->getMemory<float>(x86::reg32(5478676) /* 0x539914 */)));
    cpu.fpu.count += 1;
    cpu.fpu.st(0) = fpu6;
    }
L_0x00455776:
    // 00455776  83ec08                 -sub esp, 8
    (cpu.esp) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 00455779  dd1c24                 -fstp qword ptr [esp]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu7 = cpu.fpu.st(0);
    app->getMemory<double>(cpu.esp) = double(fpu7);
    // 0045577c  6800995300             -push 0x539900
    app->getMemory<x86::reg32>(cpu.esp-4) = 5478656 /*0x539900*/;
    cpu.esp -= 4;
    // 00455781  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    cpu.fpu.count -= 1;
    }
    // 00455782  e8099f0800             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00455787  83c414                 -add esp, 0x14
    (cpu.esp) += x86::reg32(x86::sreg32(20 /*0x14*/));
    // 0045578a  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0045578c  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045578d  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045578e  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0045578f:
    // 0045578f  680c995300             -push 0x53990c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5478668 /*0x53990c*/;
    cpu.esp -= 4;
    // 00455794  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00455795  e8f69e0800             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0045579a  83c408                 -add esp, 8
    (cpu.esp) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 0045579d  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0045579f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004557a0  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004557a1  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_4557b0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004557b0  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004557b1  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004557b3  f6c204                 +test dl, 4
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.dl & 4 /*0x4*/));
    // 004557b6  740d                   -je 0x4557c5
    if (cpu.flags.zf)
    {
        goto L_0x004557c5;
    }
    // 004557b8  f6800002000002         +test byte ptr [eax + 0x200], 2
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(512) /* 0x200 */) & 2 /*0x2*/));
    // 004557bf  7504                   -jne 0x4557c5
    if (!cpu.flags.zf)
    {
        goto L_0x004557c5;
    }
L_0x004557c1:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004557c1  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004557c3  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004557c4  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004557c5:
    // 004557c5  f6c208                 +test dl, 8
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.dl & 8 /*0x8*/));
    // 004557c8  7409                   -je 0x4557d3
    if (cpu.flags.zf)
    {
        goto L_0x004557d3;
    }
    // 004557ca  f6800002000004         +test byte ptr [eax + 0x200], 4
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(512) /* 0x200 */) & 4 /*0x4*/));
    // 004557d1  74ee                   -je 0x4557c1
    if (cpu.flags.zf)
    {
        goto L_0x004557c1;
    }
L_0x004557d3:
    // 004557d3  f6c201                 +test dl, 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.dl & 1 /*0x1*/));
    // 004557d6  7409                   -je 0x4557e1
    if (cpu.flags.zf)
    {
        goto L_0x004557e1;
    }
    // 004557d8  f6800002000002         +test byte ptr [eax + 0x200], 2
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(512) /* 0x200 */) & 2 /*0x2*/));
    // 004557df  75e0                   -jne 0x4557c1
    if (!cpu.flags.zf)
    {
        goto L_0x004557c1;
    }
L_0x004557e1:
    // 004557e1  f6c202                 +test dl, 2
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.dl & 2 /*0x2*/));
    // 004557e4  7409                   -je 0x4557ef
    if (cpu.flags.zf)
    {
        goto L_0x004557ef;
    }
    // 004557e6  f6800002000004         +test byte ptr [eax + 0x200], 4
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(512) /* 0x200 */) & 4 /*0x4*/));
    // 004557ed  75d2                   -jne 0x4557c1
    if (!cpu.flags.zf)
    {
        goto L_0x004557c1;
    }
L_0x004557ef:
    // 004557ef  f6c210                 +test dl, 0x10
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.dl & 16 /*0x10*/));
    // 004557f2  7408                   -je 0x4557fc
    if (cpu.flags.zf)
    {
        goto L_0x004557fc;
    }
    // 004557f4  3b98c4020000           +cmp ebx, dword ptr [eax + 0x2c4]
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(708) /* 0x2c4 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004557fa  75c5                   -jne 0x4557c1
    if (!cpu.flags.zf)
    {
        goto L_0x004557c1;
    }
L_0x004557fc:
    // 004557fc  f6c220                 +test dl, 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.dl & 32 /*0x20*/));
    // 004557ff  7408                   -je 0x455809
    if (cpu.flags.zf)
    {
        goto L_0x00455809;
    }
    // 00455801  3b98c4020000           +cmp ebx, dword ptr [eax + 0x2c4]
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(708) /* 0x2c4 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00455807  74b8                   -je 0x4557c1
    if (cpu.flags.zf)
    {
        goto L_0x004557c1;
    }
L_0x00455809:
    // 00455809  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0045580e  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045580f  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_455840(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 00455840  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00455841  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00455842  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00455843  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00455844  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00455845  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00455847  89d3                   -mov ebx, edx
    cpu.ebx = cpu.edx;
    // 00455849  8b08                   -mov ecx, dword ptr [eax]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax);
    // 0045584b  8b1b                   -mov ebx, dword ptr [ebx]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebx);
    // 0045584d  a118965500             -mov eax, dword ptr [0x559618]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5608984) /* 0x559618 */);
    // 00455852  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00455854  83f808                 +cmp eax, 8
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
    // 00455857  0f8758030000           -ja 0x455bb5
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x00455bb5;
    }
    // 0045585d  ff248510584500         -jmp dword ptr [eax*4 + 0x455810]
    cpu.ip = app->getMemory<x86::reg32>(4544528 + cpu.eax * 4); goto dynamic_jump;
  case 0x00455864:
    // 00455864  8b9320020000           -mov edx, dword ptr [ebx + 0x220]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(544) /* 0x220 */);
    // 0045586a  8b8120020000           -mov eax, dword ptr [ecx + 0x220]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(544) /* 0x220 */);
    // 00455870  83c234                 -add edx, 0x34
    (cpu.edx) += x86::reg32(x86::sreg32(52 /*0x34*/));
    // 00455873  83c034                 +add eax, 0x34
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(52 /*0x34*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00455876  e845460900             -call 0x4e9ec0
    cpu.esp -= 4;
    nfs3hp::watcom::stricmp(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0045587b  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0045587d  e933030000             -jmp 0x455bb5
    goto L_0x00455bb5;
  case 0x00455882:
    // 00455882  8bb3d4020000           -mov esi, dword ptr [ebx + 0x2d4]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(724) /* 0x2d4 */);
    // 00455888  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 0045588a  743f                   -je 0x4558cb
    if (cpu.flags.zf)
    {
        goto L_0x004558cb;
    }
    // 0045588c  8bb9d4020000           -mov edi, dword ptr [ecx + 0x2d4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(724) /* 0x2d4 */);
    // 00455892  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 00455894  7435                   -je 0x4558cb
    if (cpu.flags.zf)
    {
        goto L_0x004558cb;
    }
    // 00455896  39f7                   +cmp edi, esi
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.esi));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00455898  7d05                   -jge 0x45589f
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0045589f;
    }
    // 0045589a  baffffffff             -mov edx, 0xffffffff
    cpu.edx = 4294967295 /*0xffffffff*/;
L_0x0045589f:
    // 0045589f  8bbbd4020000           -mov edi, dword ptr [ebx + 0x2d4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(724) /* 0x2d4 */);
    // 004558a5  3bb9d4020000           +cmp edi, dword ptr [ecx + 0x2d4]
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(724) /* 0x2d4 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004558ab  7d05                   -jge 0x4558b2
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004558b2;
    }
    // 004558ad  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
L_0x004558b2:
    // 004558b2  8bb3d4020000           -mov esi, dword ptr [ebx + 0x2d4]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(724) /* 0x2d4 */);
    // 004558b8  3bb1d4020000           +cmp esi, dword ptr [ecx + 0x2d4]
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(724) /* 0x2d4 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004558be  0f85f1020000           -jne 0x455bb5
    if (!cpu.flags.zf)
    {
        goto L_0x00455bb5;
    }
    // 004558c4  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 004558c6  e9ea020000             -jmp 0x455bb5
    goto L_0x00455bb5;
L_0x004558cb:
    // 004558cb  83b9d402000000         +cmp dword ptr [ecx + 0x2d4], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(724) /* 0x2d4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004558d2  740a                   -je 0x4558de
    if (cpu.flags.zf)
    {
        goto L_0x004558de;
    }
    // 004558d4  baffffffff             -mov edx, 0xffffffff
    cpu.edx = 4294967295 /*0xffffffff*/;
    // 004558d9  e9d7020000             -jmp 0x455bb5
    goto L_0x00455bb5;
L_0x004558de:
    // 004558de  83bbd402000000         +cmp dword ptr [ebx + 0x2d4], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(724) /* 0x2d4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004558e5  740a                   -je 0x4558f1
    if (cpu.flags.zf)
    {
        goto L_0x004558f1;
    }
    // 004558e7  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 004558ec  e9c4020000             -jmp 0x455bb5
    goto L_0x00455bb5;
L_0x004558f1:
    // 004558f1  8bb3cc020000           -mov esi, dword ptr [ebx + 0x2cc]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(716) /* 0x2cc */);
    // 004558f7  3bb1cc020000           +cmp esi, dword ptr [ecx + 0x2cc]
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(716) /* 0x2cc */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004558fd  7e05                   -jle 0x455904
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00455904;
    }
    // 004558ff  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
L_0x00455904:
    // 00455904  8bbbcc020000           -mov edi, dword ptr [ebx + 0x2cc]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(716) /* 0x2cc */);
    // 0045590a  3bb9cc020000           +cmp edi, dword ptr [ecx + 0x2cc]
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(716) /* 0x2cc */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00455910  7d05                   -jge 0x455917
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00455917;
    }
    // 00455912  baffffffff             -mov edx, 0xffffffff
    cpu.edx = 4294967295 /*0xffffffff*/;
L_0x00455917:
    // 00455917  8bb3cc020000           -mov esi, dword ptr [ebx + 0x2cc]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(716) /* 0x2cc */);
    // 0045591d  3bb1cc020000           +cmp esi, dword ptr [ecx + 0x2cc]
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(716) /* 0x2cc */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00455923  0f858c020000           -jne 0x455bb5
    if (!cpu.flags.zf)
    {
        goto L_0x00455bb5;
    }
    // 00455929  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 0045592b  e985020000             -jmp 0x455bb5
    goto L_0x00455bb5;
  case 0x00455930:
    // 00455930  8bbbc8020000           -mov edi, dword ptr [ebx + 0x2c8]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(712) /* 0x2c8 */);
    // 00455936  3bb9c8020000           +cmp edi, dword ptr [ecx + 0x2c8]
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(712) /* 0x2c8 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0045593c  7e05                   -jle 0x455943
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00455943;
    }
    // 0045593e  baffffffff             -mov edx, 0xffffffff
    cpu.edx = 4294967295 /*0xffffffff*/;
L_0x00455943:
    // 00455943  8bb3c8020000           -mov esi, dword ptr [ebx + 0x2c8]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(712) /* 0x2c8 */);
    // 00455949  3bb1c8020000           +cmp esi, dword ptr [ecx + 0x2c8]
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(712) /* 0x2c8 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0045594f  7d05                   -jge 0x455956
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00455956;
    }
    // 00455951  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
L_0x00455956:
    // 00455956  8bbbc8020000           -mov edi, dword ptr [ebx + 0x2c8]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(712) /* 0x2c8 */);
    // 0045595c  3bb9c8020000           +cmp edi, dword ptr [ecx + 0x2c8]
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(712) /* 0x2c8 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00455962  0f854d020000           -jne 0x455bb5
    if (!cpu.flags.zf)
    {
        goto L_0x00455bb5;
    }
    // 00455968  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 0045596a  e946020000             -jmp 0x455bb5
    goto L_0x00455bb5;
  case 0x0045596f:
    // 0045596f  8bb3cc020000           -mov esi, dword ptr [ebx + 0x2cc]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(716) /* 0x2cc */);
    // 00455975  3bb1cc020000           +cmp esi, dword ptr [ecx + 0x2cc]
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(716) /* 0x2cc */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0045597b  7e05                   -jle 0x455982
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00455982;
    }
    // 0045597d  baffffffff             -mov edx, 0xffffffff
    cpu.edx = 4294967295 /*0xffffffff*/;
L_0x00455982:
    // 00455982  8bbbcc020000           -mov edi, dword ptr [ebx + 0x2cc]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(716) /* 0x2cc */);
    // 00455988  3bb9cc020000           +cmp edi, dword ptr [ecx + 0x2cc]
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(716) /* 0x2cc */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0045598e  7d05                   -jge 0x455995
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00455995;
    }
    // 00455990  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
L_0x00455995:
    // 00455995  8bb3cc020000           -mov esi, dword ptr [ebx + 0x2cc]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(716) /* 0x2cc */);
    // 0045599b  3bb1cc020000           +cmp esi, dword ptr [ecx + 0x2cc]
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(716) /* 0x2cc */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004559a1  0f850e020000           -jne 0x455bb5
    if (!cpu.flags.zf)
    {
        goto L_0x00455bb5;
    }
    // 004559a7  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 004559a9  e907020000             -jmp 0x455bb5
    goto L_0x00455bb5;
  case 0x004559ae:
    // 004559ae  8b8120020000           -mov eax, dword ptr [ecx + 0x220]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(544) /* 0x220 */);
    // 004559b4  8b4010                 -mov eax, dword ptr [eax + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 004559b7  8b34454cd46f00         -mov esi, dword ptr [eax*2 + 0x6fd44c]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7328844) /* 0x6fd44c */ + cpu.eax * 2);
    // 004559be  8b8320020000           -mov eax, dword ptr [ebx + 0x220]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(544) /* 0x220 */);
    // 004559c4  8b4010                 -mov eax, dword ptr [eax + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 004559c7  8b04454cd46f00         -mov eax, dword ptr [eax*2 + 0x6fd44c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328844) /* 0x6fd44c */ + cpu.eax * 2);
    // 004559ce  c1fe10                 -sar esi, 0x10
    cpu.esi = x86::reg32(x86::sreg32(cpu.esi) >> (16 /*0x10*/ % 32));
    // 004559d1  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 004559d4  39c6                   +cmp esi, eax
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
    // 004559d6  7d05                   -jge 0x4559dd
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004559dd;
    }
    // 004559d8  baffffffff             -mov edx, 0xffffffff
    cpu.edx = 4294967295 /*0xffffffff*/;
L_0x004559dd:
    // 004559dd  8b8120020000           -mov eax, dword ptr [ecx + 0x220]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(544) /* 0x220 */);
    // 004559e3  8b4010                 -mov eax, dword ptr [eax + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 004559e6  8b34454cd46f00         -mov esi, dword ptr [eax*2 + 0x6fd44c]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7328844) /* 0x6fd44c */ + cpu.eax * 2);
    // 004559ed  8b8320020000           -mov eax, dword ptr [ebx + 0x220]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(544) /* 0x220 */);
    // 004559f3  8b4010                 -mov eax, dword ptr [eax + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 004559f6  8b04454cd46f00         -mov eax, dword ptr [eax*2 + 0x6fd44c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328844) /* 0x6fd44c */ + cpu.eax * 2);
    // 004559fd  c1fe10                 -sar esi, 0x10
    cpu.esi = x86::reg32(x86::sreg32(cpu.esi) >> (16 /*0x10*/ % 32));
    // 00455a00  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00455a03  39c6                   +cmp esi, eax
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
    // 00455a05  7e05                   -jle 0x455a0c
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00455a0c;
    }
    // 00455a07  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
L_0x00455a0c:
    // 00455a0c  8b8120020000           -mov eax, dword ptr [ecx + 0x220]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(544) /* 0x220 */);
    // 00455a12  8b7810                 -mov edi, dword ptr [eax + 0x10]
    cpu.edi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 00455a15  8b8320020000           -mov eax, dword ptr [ebx + 0x220]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(544) /* 0x220 */);
    // 00455a1b  8b7010                 -mov esi, dword ptr [eax + 0x10]
    cpu.esi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 00455a1e  668b047d4ed46f00       -mov ax, word ptr [edi*2 + 0x6fd44e]
    cpu.ax = app->getMemory<x86::reg16>(x86::reg32(7328846) /* 0x6fd44e */ + cpu.edi * 2);
    // 00455a26  663b04754ed46f00       +cmp ax, word ptr [esi*2 + 0x6fd44e]
    {
        x86::reg16 tmp1 = cpu.ax;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(app->getMemory<x86::reg16>(x86::reg32(7328846) /* 0x6fd44e */ + cpu.esi * 2)));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 00455a2e  7504                   -jne 0x455a34
    if (!cpu.flags.zf)
    {
        goto L_0x00455a34;
    }
    // 00455a30  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 00455a32  eb08                   -jmp 0x455a3c
    goto L_0x00455a3c;
L_0x00455a34:
    // 00455a34  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 00455a36  0f8579010000           -jne 0x455bb5
    if (!cpu.flags.zf)
    {
        goto L_0x00455bb5;
    }
L_0x00455a3c:
    // 00455a3c  f6810002000004         +test byte ptr [ecx + 0x200], 4
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(512) /* 0x200 */) & 4 /*0x4*/));
    // 00455a43  740e                   -je 0x455a53
    if (cpu.flags.zf)
    {
        goto L_0x00455a53;
    }
    // 00455a45  f6830002000004         +test byte ptr [ebx + 0x200], 4
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(512) /* 0x200 */) & 4 /*0x4*/));
    // 00455a4c  7505                   -jne 0x455a53
    if (!cpu.flags.zf)
    {
        goto L_0x00455a53;
    }
    // 00455a4e  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
L_0x00455a53:
    // 00455a53  f6810002000004         +test byte ptr [ecx + 0x200], 4
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(512) /* 0x200 */) & 4 /*0x4*/));
    // 00455a5a  0f8555010000           -jne 0x455bb5
    if (!cpu.flags.zf)
    {
        goto L_0x00455bb5;
    }
    // 00455a60  f6830002000004         +test byte ptr [ebx + 0x200], 4
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(512) /* 0x200 */) & 4 /*0x4*/));
    // 00455a67  0f8448010000           -je 0x455bb5
    if (cpu.flags.zf)
    {
        goto L_0x00455bb5;
    }
    // 00455a6d  baffffffff             -mov edx, 0xffffffff
    cpu.edx = 4294967295 /*0xffffffff*/;
    // 00455a72  e93e010000             -jmp 0x455bb5
    goto L_0x00455bb5;
  case 0x00455a77:
    // 00455a77  8b8120020000           -mov eax, dword ptr [ecx + 0x220]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(544) /* 0x220 */);
    // 00455a7d  8b4010                 -mov eax, dword ptr [eax + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 00455a80  8b34452cd46f00         -mov esi, dword ptr [eax*2 + 0x6fd42c]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7328812) /* 0x6fd42c */ + cpu.eax * 2);
    // 00455a87  8b8320020000           -mov eax, dword ptr [ebx + 0x220]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(544) /* 0x220 */);
    // 00455a8d  8b4010                 -mov eax, dword ptr [eax + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 00455a90  8b04452cd46f00         -mov eax, dword ptr [eax*2 + 0x6fd42c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328812) /* 0x6fd42c */ + cpu.eax * 2);
    // 00455a97  c1fe10                 -sar esi, 0x10
    cpu.esi = x86::reg32(x86::sreg32(cpu.esi) >> (16 /*0x10*/ % 32));
    // 00455a9a  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00455a9d  39c6                   +cmp esi, eax
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
    // 00455a9f  7d05                   -jge 0x455aa6
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00455aa6;
    }
    // 00455aa1  baffffffff             -mov edx, 0xffffffff
    cpu.edx = 4294967295 /*0xffffffff*/;
L_0x00455aa6:
    // 00455aa6  8b8120020000           -mov eax, dword ptr [ecx + 0x220]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(544) /* 0x220 */);
    // 00455aac  8b4010                 -mov eax, dword ptr [eax + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 00455aaf  8b34452cd46f00         -mov esi, dword ptr [eax*2 + 0x6fd42c]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7328812) /* 0x6fd42c */ + cpu.eax * 2);
    // 00455ab6  8b8320020000           -mov eax, dword ptr [ebx + 0x220]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(544) /* 0x220 */);
    // 00455abc  8b4010                 -mov eax, dword ptr [eax + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 00455abf  8b04452cd46f00         -mov eax, dword ptr [eax*2 + 0x6fd42c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328812) /* 0x6fd42c */ + cpu.eax * 2);
    // 00455ac6  c1fe10                 -sar esi, 0x10
    cpu.esi = x86::reg32(x86::sreg32(cpu.esi) >> (16 /*0x10*/ % 32));
    // 00455ac9  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00455acc  39c6                   +cmp esi, eax
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
    // 00455ace  7e05                   -jle 0x455ad5
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00455ad5;
    }
    // 00455ad0  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
L_0x00455ad5:
    // 00455ad5  8b8120020000           -mov eax, dword ptr [ecx + 0x220]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(544) /* 0x220 */);
    // 00455adb  8b7810                 -mov edi, dword ptr [eax + 0x10]
    cpu.edi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 00455ade  8b8320020000           -mov eax, dword ptr [ebx + 0x220]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(544) /* 0x220 */);
    // 00455ae4  8b7010                 -mov esi, dword ptr [eax + 0x10]
    cpu.esi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 00455ae7  668b047d2ed46f00       -mov ax, word ptr [edi*2 + 0x6fd42e]
    cpu.ax = app->getMemory<x86::reg16>(x86::reg32(7328814) /* 0x6fd42e */ + cpu.edi * 2);
    // 00455aef  663b04752ed46f00       +cmp ax, word ptr [esi*2 + 0x6fd42e]
    {
        x86::reg16 tmp1 = cpu.ax;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(app->getMemory<x86::reg16>(x86::reg32(7328814) /* 0x6fd42e */ + cpu.esi * 2)));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 00455af7  7504                   -jne 0x455afd
    if (!cpu.flags.zf)
    {
        goto L_0x00455afd;
    }
    // 00455af9  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 00455afb  eb08                   -jmp 0x455b05
    goto L_0x00455b05;
L_0x00455afd:
    // 00455afd  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 00455aff  0f85b0000000           -jne 0x455bb5
    if (!cpu.flags.zf)
    {
        goto L_0x00455bb5;
    }
L_0x00455b05:
    // 00455b05  f6810002000004         +test byte ptr [ecx + 0x200], 4
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(512) /* 0x200 */) & 4 /*0x4*/));
    // 00455b0c  740e                   -je 0x455b1c
    if (cpu.flags.zf)
    {
        goto L_0x00455b1c;
    }
    // 00455b0e  f6830002000004         +test byte ptr [ebx + 0x200], 4
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(512) /* 0x200 */) & 4 /*0x4*/));
    // 00455b15  7505                   -jne 0x455b1c
    if (!cpu.flags.zf)
    {
        goto L_0x00455b1c;
    }
    // 00455b17  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
L_0x00455b1c:
    // 00455b1c  f6810002000004         +test byte ptr [ecx + 0x200], 4
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(512) /* 0x200 */) & 4 /*0x4*/));
    // 00455b23  0f858c000000           -jne 0x455bb5
    if (!cpu.flags.zf)
    {
        goto L_0x00455bb5;
    }
    // 00455b29  f6830002000004         +test byte ptr [ebx + 0x200], 4
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(512) /* 0x200 */) & 4 /*0x4*/));
    // 00455b30  0f847f000000           -je 0x455bb5
    if (cpu.flags.zf)
    {
        goto L_0x00455bb5;
    }
    // 00455b36  baffffffff             -mov edx, 0xffffffff
    cpu.edx = 4294967295 /*0xffffffff*/;
    // 00455b3b  e975000000             -jmp 0x455bb5
    goto L_0x00455bb5;
  case 0x00455b40:
    // 00455b40  8bbb38020000           -mov edi, dword ptr [ebx + 0x238]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(568) /* 0x238 */);
    // 00455b46  3bb938020000           +cmp edi, dword ptr [ecx + 0x238]
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(568) /* 0x238 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00455b4c  7e05                   -jle 0x455b53
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00455b53;
    }
    // 00455b4e  baffffffff             -mov edx, 0xffffffff
    cpu.edx = 4294967295 /*0xffffffff*/;
L_0x00455b53:
    // 00455b53  8bb338020000           -mov esi, dword ptr [ebx + 0x238]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(568) /* 0x238 */);
    // 00455b59  3bb138020000           +cmp esi, dword ptr [ecx + 0x238]
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(568) /* 0x238 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00455b5f  7d05                   -jge 0x455b66
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00455b66;
    }
    // 00455b61  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
L_0x00455b66:
    // 00455b66  8bbb38020000           -mov edi, dword ptr [ebx + 0x238]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(568) /* 0x238 */);
    // 00455b6c  3bb938020000           +cmp edi, dword ptr [ecx + 0x238]
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(568) /* 0x238 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00455b72  7541                   -jne 0x455bb5
    if (!cpu.flags.zf)
    {
        goto L_0x00455bb5;
    }
    // 00455b74  eb3d                   -jmp 0x455bb3
    goto L_0x00455bb3;
  case 0x00455b76:
    // 00455b76  d981b8020000           +fld dword ptr [ecx + 0x2b8]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu1 = x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(696) /* 0x2b8 */));
    // 00455b7c  d89bb8020000           +fcomp dword ptr [ebx + 0x2b8]
    cpu.fpu.compare(fpu1, x86::Float(app->getMemory<float>(cpu.ebx + x86::reg32(696) /* 0x2b8 */)));
    // 00455b82  dfe0                   -fnstsw ax
    cpu.ax = cpu.fpu.status.word;
    // 00455b84  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    }
    // 00455b85  7305                   -jae 0x455b8c
    if (!cpu.flags.cf)
    {
        goto L_0x00455b8c;
    }
    // 00455b87  baffffffff             -mov edx, 0xffffffff
    cpu.edx = 4294967295 /*0xffffffff*/;
L_0x00455b8c:
    // 00455b8c  d981b8020000           +fld dword ptr [ecx + 0x2b8]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu2 = x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(696) /* 0x2b8 */));
    // 00455b92  d89bb8020000           +fcomp dword ptr [ebx + 0x2b8]
    cpu.fpu.compare(fpu2, x86::Float(app->getMemory<float>(cpu.ebx + x86::reg32(696) /* 0x2b8 */)));
    // 00455b98  dfe0                   -fnstsw ax
    cpu.ax = cpu.fpu.status.word;
    // 00455b9a  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    }
    // 00455b9b  7605                   -jbe 0x455ba2
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00455ba2;
    }
    // 00455b9d  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
L_0x00455ba2:
    // 00455ba2  d981b8020000           +fld dword ptr [ecx + 0x2b8]
    {  // x87 stack in locals (codegen/fpu_stack.py)
    x86::Float fpu3 = x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(696) /* 0x2b8 */));
    // 00455ba8  d89bb8020000           +fcomp dword ptr [ebx + 0x2b8]
    cpu.fpu.compare(fpu3, x86::Float(app->getMemory<float>(cpu.ebx + x86::reg32(696) /* 0x2b8 */)));
    // 00455bae  dfe0                   -fnstsw ax
    cpu.ax = cpu.fpu.status.word;
    // 00455bb0  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    }
    // 00455bb1  7502                   -jne 0x455bb5
    if (!cpu.flags.zf)
    {
        goto L_0x00455bb5;
    }
L_0x00455bb3:
    // 00455bb3  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
  [[fallthrough]];
  case 0x00455bb5:
L_0x00455bb5:
    // 00455bb5  833d1c96550001         +cmp dword ptr [0x55961c], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5608988) /* 0x55961c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00455bbc  7502                   -jne 0x455bc0
    if (!cpu.flags.zf)
    {
        goto L_0x00455bc0;
    }
    // 00455bbe  f7da                   -neg edx
    cpu.edx = ~cpu.edx + 1;
L_0x00455bc0:
    // 00455bc0  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 00455bc2  7536                   -jne 0x455bfa
    if (!cpu.flags.zf)
    {
        goto L_0x00455bfa;
    }
    // 00455bc4  8bb338020000           -mov esi, dword ptr [ebx + 0x238]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(568) /* 0x238 */);
    // 00455bca  3bb138020000           +cmp esi, dword ptr [ecx + 0x238]
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(568) /* 0x238 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00455bd0  7e05                   -jle 0x455bd7
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00455bd7;
    }
    // 00455bd2  baffffffff             -mov edx, 0xffffffff
    cpu.edx = 4294967295 /*0xffffffff*/;
L_0x00455bd7:
    // 00455bd7  8bbb38020000           -mov edi, dword ptr [ebx + 0x238]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(568) /* 0x238 */);
    // 00455bdd  3bb938020000           +cmp edi, dword ptr [ecx + 0x238]
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(568) /* 0x238 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00455be3  7d05                   -jge 0x455bea
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00455bea;
    }
    // 00455be5  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
L_0x00455bea:
    // 00455bea  8b8138020000           -mov eax, dword ptr [ecx + 0x238]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(568) /* 0x238 */);
    // 00455bf0  3b8338020000           +cmp eax, dword ptr [ebx + 0x238]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(568) /* 0x238 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00455bf6  7502                   -jne 0x455bfa
    if (!cpu.flags.zf)
    {
        goto L_0x00455bfa;
    }
    // 00455bf8  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x00455bfa:
    // 00455bfa  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00455bfc  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455bfd  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455bfe  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455bff  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455c00  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455c01  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_455c10(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00455c10  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00455c11  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00455c12  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00455c13  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00455c14  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00455c16  83ec40                 -sub esp, 0x40
    (cpu.esp) -= x86::reg32(x86::sreg32(64 /*0x40*/));
    // 00455c19  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00455c1b  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 00455c1d  eb05                   -jmp 0x455c24
    goto L_0x00455c24;
L_0x00455c1f:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00455c1f  83f810                 +cmp eax, 0x10
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(16 /*0x10*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00455c22  7d36                   -jge 0x455c5a
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00455c5a;
    }
L_0x00455c24:
    // 00455c24  89f1                   -mov ecx, esi
    cpu.ecx = cpu.esi;
    // 00455c26  8d3c00                 -lea edi, [eax + eax]
    cpu.edi = x86::reg32(cpu.eax + cpu.eax * 1);
    // 00455c29  c1e105                 -shl ecx, 5
    cpu.ecx <<= 5 /*0x5*/ % 32;
    // 00455c2c  01f9                   -add ecx, edi
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.edi));
    // 00455c2e  66c781d8955500ffff     -mov word ptr [ecx + 0x5595d8], 0xffff
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(5608920) /* 0x5595d8 */) = 65535 /*0xffff*/;
    // 00455c37  69cec09a0000           -imul ecx, esi, 0x9ac0
    cpu.ecx = x86::reg32(x86::sreg64(x86::sreg32(cpu.esi)) * x86::sreg64(x86::sreg32(39616 /*0x9ac0*/)));
    // 00455c3d  bfb0476600             -mov edi, 0x6647b0
    cpu.edi = 6703024 /*0x6647b0*/;
    // 00455c42  01cf                   -add edi, ecx
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.ecx));
    // 00455c44  69c8ac090000           -imul ecx, eax, 0x9ac
    cpu.ecx = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(2476 /*0x9ac*/)));
    // 00455c4a  01f9                   +add ecx, edi
    {
        x86::reg32& tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edi));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00455c4c  8d3c8500000000         -lea edi, [eax*4]
    cpu.edi = x86::reg32(cpu.eax * 4);
    // 00455c53  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00455c54  894c2fc0               -mov dword ptr [edi + ebp - 0x40], ecx
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(-64) /* -0x40 */ + cpu.ebp * 1) = cpu.ecx;
    // 00455c58  ebc5                   -jmp 0x455c1f
    goto L_0x00455c1f;
L_0x00455c5a:
    // 00455c5a  b940584500             -mov ecx, 0x455840
    cpu.ecx = 4544576 /*0x455840*/;
    // 00455c5f  8d45c0                 -lea eax, [ebp - 0x40]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-64) /* -0x40 */);
    // 00455c62  891518965500           -mov dword ptr [0x559618], edx
    app->getMemory<x86::reg32>(x86::reg32(5608984) /* 0x559618 */) = cpu.edx;
    // 00455c68  891d1c965500           -mov dword ptr [0x55961c], ebx
    app->getMemory<x86::reg32>(x86::reg32(5608988) /* 0x55961c */) = cpu.ebx;
    // 00455c6e  bb04000000             -mov ebx, 4
    cpu.ebx = 4 /*0x4*/;
    // 00455c73  8b14b5307d6700         -mov edx, dword ptr [esi*4 + 0x677d30]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(6782256) /* 0x677d30 */ + cpu.esi * 4);
    // 00455c7a  e879880900             -call 0x4ee4f8
    cpu.esp -= 4;
    nfs3hp::watcom::qsort(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00455c7f  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x00455c81:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00455c81  3b14b5307d6700         +cmp edx, dword ptr [esi*4 + 0x677d30]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(6782256) /* 0x677d30 */ + cpu.esi * 4)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00455c88  7d38                   -jge 0x455cc2
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00455cc2;
    }
    // 00455c8a  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x00455c8c:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00455c8c  3b04b5307d6700         +cmp eax, dword ptr [esi*4 + 0x677d30]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(6782256) /* 0x677d30 */ + cpu.esi * 4)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00455c93  7d2a                   -jge 0x455cbf
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00455cbf;
    }
    // 00455c95  69dec09a0000           -imul ebx, esi, 0x9ac0
    cpu.ebx = x86::reg32(x86::sreg64(x86::sreg32(cpu.esi)) * x86::sreg64(x86::sreg32(39616 /*0x9ac0*/)));
    // 00455c9b  69c8ac090000           -imul ecx, eax, 0x9ac
    cpu.ecx = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(2476 /*0x9ac*/)));
    // 00455ca1  81c3b0476600           -add ebx, 0x6647b0
    (cpu.ebx) += x86::reg32(x86::sreg32(6703024 /*0x6647b0*/));
    // 00455ca7  01d9                   -add ecx, ebx
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.ebx));
    // 00455ca9  3b4c95c0               +cmp ecx, dword ptr [ebp + edx*4 - 0x40]
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-64) /* -0x40 */ + cpu.edx * 4)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00455cad  750d                   -jne 0x455cbc
    if (!cpu.flags.zf)
    {
        goto L_0x00455cbc;
    }
    // 00455caf  89f1                   -mov ecx, esi
    cpu.ecx = cpu.esi;
    // 00455cb1  c1e105                 +shl ecx, 5
    {
        x86::reg8 tmp = 5 /*0x5*/ % 32;
        x86::reg32& op = cpu.ecx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 00455cb4  66898451d8955500       -mov word ptr [ecx + edx*2 + 0x5595d8], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(5608920) /* 0x5595d8 */ + cpu.edx * 2) = cpu.ax;
L_0x00455cbc:
    // 00455cbc  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00455cbd  ebcd                   -jmp 0x455c8c
    goto L_0x00455c8c;
L_0x00455cbf:
    // 00455cbf  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00455cc0  ebbf                   -jmp 0x455c81
    goto L_0x00455c81;
L_0x00455cc2:
    // 00455cc2  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00455cc4  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455cc5  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455cc6  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455cc7  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455cc8  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_455cd0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00455cd0  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00455cd1  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00455cd2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00455cd3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00455cd5  83ec14                 -sub esp, 0x14
    (cpu.esp) -= x86::reg32(x86::sreg32(20 /*0x14*/));
    // 00455cd8  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00455cda  8955f8                 -mov dword ptr [ebp - 8], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.edx;
    // 00455cdd  89df                   -mov edi, ebx
    cpu.edi = cpu.ebx;
    // 00455cdf  894dec                 -mov dword ptr [ebp - 0x14], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.ecx;
    // 00455ce2  83fa10                 +cmp edx, 0x10
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(16 /*0x10*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00455ce5  0f8fd4000000           -jg 0x455dbf
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00455dbf;
    }
    // 00455ceb  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 00455ced  7545                   -jne 0x455d34
    if (!cpu.flags.zf)
    {
        goto L_0x00455d34;
    }
    // 00455cef  c1e005                 -shl eax, 5
    cpu.eax <<= 5 /*0x5*/ % 32;
    // 00455cf2  01d2                   -add edx, edx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.edx));
    // 00455cf4  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00455cf6  6683b8d895550000       +cmp word ptr [eax + 0x5595d8], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(5608920) /* 0x5595d8 */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 00455cfe  0f8cbb000000           -jl 0x455dbf
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00455dbf;
    }
    // 00455d04  8b80d6955500           -mov eax, dword ptr [eax + 0x5595d6]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(5608918) /* 0x5595d6 */);
    // 00455d0a  8b0cb5307d6700         -mov ecx, dword ptr [esi*4 + 0x677d30]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(6782256) /* 0x677d30 */ + cpu.esi * 4);
    // 00455d11  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00455d14  39c8                   +cmp eax, ecx
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
    // 00455d16  0f8fa3000000           -jg 0x455dbf
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00455dbf;
    }
    // 00455d1c  69d0ac090000           -imul edx, eax, 0x9ac
    cpu.edx = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(2476 /*0x9ac*/)));
    // 00455d22  69c6c09a0000           -imul eax, esi, 0x9ac0
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.esi)) * x86::sreg64(x86::sreg32(39616 /*0x9ac0*/)));
    // 00455d28  05b0476600             -add eax, 0x6647b0
    (cpu.eax) += x86::reg32(x86::sreg32(6703024 /*0x6647b0*/));
    // 00455d2d  01d0                   +add eax, edx
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
    // 00455d2f  e98d000000             -jmp 0x455dc1
    goto L_0x00455dc1;
L_0x00455d34:
    // 00455d34  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00455d36  31c9                   +xor ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx))));
    // 00455d38  895dfc                 -mov dword ptr [ebp - 4], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ebx;
    // 00455d3b  eb55                   -jmp 0x455d92
    goto L_0x00455d92;
L_0x00455d3d:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00455d3d  69c0ac090000           -imul eax, eax, 0x9ac
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(2476 /*0x9ac*/)));
    // 00455d43  69d6c09a0000           -imul edx, esi, 0x9ac0
    cpu.edx = x86::reg32(x86::sreg64(x86::sreg32(cpu.esi)) * x86::sreg64(x86::sreg32(39616 /*0x9ac0*/)));
    // 00455d49  bbb0476600             -mov ebx, 0x6647b0
    cpu.ebx = 6703024 /*0x6647b0*/;
    // 00455d4e  01d3                   -add ebx, edx
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.edx));
    // 00455d50  895df0                 -mov dword ptr [ebp - 0x10], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.ebx;
    // 00455d53  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 00455d55  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 00455d57  8b5dec                 -mov ebx, dword ptr [ebp - 0x14]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00455d5a  e851faffff             -call 0x4557b0
    cpu.esp -= 4;
    sub_4557b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00455d5f  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00455d61  7429                   -je 0x455d8c
    if (cpu.flags.zf)
    {
        goto L_0x00455d8c;
    }
    // 00455d63  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00455d66  3b45f8                 +cmp eax, dword ptr [ebp - 8]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00455d69  751b                   -jne 0x455d86
    if (!cpu.flags.zf)
    {
        goto L_0x00455d86;
    }
    // 00455d6b  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00455d6e  8b80d6955500           -mov eax, dword ptr [eax + 0x5595d6]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(5608918) /* 0x5595d6 */);
    // 00455d74  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00455d77  69c0ac090000           -imul eax, eax, 0x9ac
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(2476 /*0x9ac*/)));
    // 00455d7d  0345f0                 -add eax, dword ptr [ebp - 0x10]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */)));
    // 00455d80  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00455d82  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455d83  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455d84  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455d85  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00455d86:
    // 00455d86  8d5801                 -lea ebx, [eax + 1]
    cpu.ebx = x86::reg32(cpu.eax + x86::reg32(1) /* 0x1 */);
    // 00455d89  895dfc                 -mov dword ptr [ebp - 4], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ebx;
L_0x00455d8c:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00455d8c  41                     -inc ecx
    (cpu.ecx)++;
    // 00455d8d  83f910                 +cmp ecx, 0x10
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(16 /*0x10*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00455d90  7d2d                   -jge 0x455dbf
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00455dbf;
    }
L_0x00455d92:
    // 00455d92  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 00455d94  8d0409                 -lea eax, [ecx + ecx]
    cpu.eax = x86::reg32(cpu.ecx + cpu.ecx * 1);
    // 00455d97  c1e205                 -shl edx, 5
    cpu.edx <<= 5 /*0x5*/ % 32;
    // 00455d9a  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00455d9c  668b9ad8955500         -mov bx, word ptr [edx + 0x5595d8]
    cpu.bx = app->getMemory<x86::reg16>(cpu.edx + x86::reg32(5608920) /* 0x5595d8 */);
    // 00455da3  8955f4                 -mov dword ptr [ebp - 0xc], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.edx;
    // 00455da6  6685db                 +test bx, bx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg16>(cpu.bx & cpu.bx));
    // 00455da9  7ce1                   -jl 0x455d8c
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00455d8c;
    }
    // 00455dab  8b82d6955500           -mov eax, dword ptr [edx + 0x5595d6]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(5608918) /* 0x5595d6 */);
    // 00455db1  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00455db4  3b04b5307d6700         +cmp eax, dword ptr [esi*4 + 0x677d30]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(6782256) /* 0x677d30 */ + cpu.esi * 4)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00455dbb  7e80                   -jle 0x455d3d
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00455d3d;
    }
    // 00455dbd  ebcd                   -jmp 0x455d8c
    goto L_0x00455d8c;
L_0x00455dbf:
    // 00455dbf  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x00455dc1:
    // 00455dc1  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00455dc3  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455dc4  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455dc5  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455dc6  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_455dd0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00455dd0  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00455dd1  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00455dd2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00455dd3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00455dd5  83ec28                 -sub esp, 0x28
    (cpu.esp) -= x86::reg32(x86::sreg32(40 /*0x28*/));
    // 00455dd8  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00455dda  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 00455ddd  895df8                 -mov dword ptr [ebp - 8], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.ebx;
    // 00455de0  bfd8955500             -mov edi, 0x5595d8
    cpu.edi = 5608920 /*0x5595d8*/;
    // 00455de5  c1e005                 -shl eax, 5
    cpu.eax <<= 5 /*0x5*/ % 32;
    // 00455de8  bb20000000             -mov ebx, 0x20
    cpu.ebx = 32 /*0x20*/;
    // 00455ded  01c7                   -add edi, eax
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.eax));
    // 00455def  8d55d8                 -lea edx, [ebp - 0x28]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-40) /* -0x28 */);
    // 00455df2  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 00455df4  e8f7460900             -call 0x4ea4f0
    cpu.esp -= 4;
    nfs3hp::ea::memmove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00455df9  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00455dfc  89cb                   -mov ebx, ecx
    cpu.ebx = cpu.ecx;
    // 00455dfe  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00455e00  e80bfeffff             -call 0x455c10
    cpu.esp -= 4;
    sub_455c10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00455e05  8b4d14                 -mov ecx, dword ptr [ebp + 0x14]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    // 00455e08  8b5d10                 -mov ebx, dword ptr [ebp + 0x10]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(16) /* 0x10 */);
    // 00455e0b  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00455e0e  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00455e10  e8bbfeffff             -call 0x455cd0
    cpu.esp -= 4;
    sub_455cd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00455e15  bb20000000             -mov ebx, 0x20
    cpu.ebx = 32 /*0x20*/;
    // 00455e1a  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00455e1c  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 00455e1e  8d45d8                 -lea eax, [ebp - 0x28]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-40) /* -0x28 */);
    // 00455e21  e8ca460900             -call 0x4ea4f0
    cpu.esp -= 4;
    nfs3hp::ea::memmove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00455e26  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00455e28  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00455e2a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455e2b  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455e2c  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00455e2d  c20800                 -ret 8
    cpu.esp += 4+8 /*0x8*/;
    return;
}

/* sub_455e30: native, nfs3hp::port::buttons::textBoxDraw */

/* sub_456020: native, nfs3hp::port::buttons::textBoxKey */

/* sub_456110: native, nfs3hp::port::buttons::textBoxUp */

/* sub_456170: native, nfs3hp::port::buttons::textBoxShown */

/* sub_4561d0: native, nfs3hp::port::buttons::textBoxHidden */

/* align: skip 0x00 0x00 */
void Application::sub_456200(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    if (infoPlaceTrackPlaces(app, cpu.sync())) /* port: native (tools/apply_native_frontend.py) */
    {
        cpu.reload();
        return;
    }
    // 00456200  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00456201  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00456202  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00456203  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00456204  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00456205  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00456206  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00456208  83ec50                 -sub esp, 0x50
    (cpu.esp) -= x86::reg32(x86::sreg32(80 /*0x50*/));
    // 0045620b  8b15acd46f00           -mov edx, dword ptr [0x6fd4ac]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7328940) /* 0x6fd4ac */);
    // 00456211  83fa09                 +cmp edx, 9
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(9 /*0x9*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00456214  7d0d                   -jge 0x456223
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00456223;
    }
    // 00456216  833de0227a0000         +cmp dword ptr [0x7a22e0], 0
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
    // 0045621d  7427                   -je 0x456246
    if (cpu.flags.zf)
    {
        goto L_0x00456246;
    }
    // 0045621f  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 00456221  7523                   -jne 0x456246
    if (!cpu.flags.zf)
    {
        goto L_0x00456246;
    }
L_0x00456223:
    // 00456223  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 00456225  668915747d6700         -mov word ptr [0x677d74], dx
    app->getMemory<x86::reg16>(x86::reg32(6782324) /* 0x677d74 */) = cpu.dx;
    // 0045622c  668915727d6700         -mov word ptr [0x677d72], dx
    app->getMemory<x86::reg16>(x86::reg32(6782322) /* 0x677d72 */) = cpu.dx;
    // 00456233  668915707d6700         -mov word ptr [0x677d70], dx
    app->getMemory<x86::reg16>(x86::reg32(6782320) /* 0x677d70 */) = cpu.dx;
    // 0045623a  668915767d6700         -mov word ptr [0x677d76], dx
    app->getMemory<x86::reg16>(x86::reg32(6782326) /* 0x677d76 */) = cpu.dx;
    // 00456241  e9c9000000             -jmp 0x45630f
    goto L_0x0045630f;
L_0x00456246:
    // 00456246  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
L_0x00456248:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00456248  a1acd46f00             -mov eax, dword ptr [0x6fd4ac]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328940) /* 0x6fd4ac */);
    // 0045624d  3b0c85f4975500         +cmp ecx, dword ptr [eax*4 + 0x5597f4]
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5609460) /* 0x5597f4 */ + cpu.eax * 4)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00456254  0f8db5000000           -jge 0x45630f
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0045630f;
    }
    // 0045625a  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 0045625d  8bbc88d4965500         -mov edi, dword ptr [eax + ecx*4 + 0x5596d4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(5609172) /* 0x5596d4 */ + cpu.ecx * 4);
    // 00456264  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00456265  8d45b0                 -lea eax, [ebp - 0x50]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-80) /* -0x50 */);
    // 00456268  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00456269  8d55b0                 -lea edx, [ebp - 0x50]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-80) /* -0x50 */);
    // 0045626c  e81f940800             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00456271  83c408                 -add esp, 8
    (cpu.esp) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 00456274  a1807d6700             -mov eax, dword ptr [0x677d80]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6782336) /* 0x677d80 */);
    // 00456279  8d3409                 -lea esi, [ecx + ecx]
    cpu.esi = x86::reg32(cpu.ecx + cpu.ecx * 1);
    // 0045627c  e8ef8f0900             -call 0x4ef270
    cpu.esp -= 4;
    nfs3hp::ea::locateShape(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00456281  8a15583a7a00           -mov dl, byte ptr [0x7a3a58]
    cpu.dl = app->getMemory<x86::reg8>(x86::reg32(8010328) /* 0x7a3a58 */);
    // 00456287  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00456289  f6c202                 +test dl, 2
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.dl & 2 /*0x2*/));
    // 0045628c  7407                   -je 0x456295
    if (cpu.flags.zf)
    {
        goto L_0x00456295;
    }
    // 0045628e  baffffffff             -mov edx, 0xffffffff
    cpu.edx = 4294967295 /*0xffffffff*/;
    // 00456293  eb02                   -jmp 0x456297
    goto L_0x00456297;
L_0x00456295:
    // 00456295  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x00456297:
    // 00456297  e8f4000800             -call 0x4d6390
    cpu.esp -= 4;
    nfs3hp::port::shapes::artShapeTextures(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0045629c  668986707d6700         -mov word ptr [esi + 0x677d70], ax
    app->getMemory<x86::reg16>(cpu.esi + x86::reg32(6782320) /* 0x677d70 */) = cpu.ax;
    // 004562a3  8b430c                 -mov eax, dword ptr [ebx + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(12) /* 0xc */);
    // 004562a6  c1e014                 -shl eax, 0x14
    cpu.eax <<= 20 /*0x14*/ % 32;
    // 004562a9  c1f814                 -sar eax, 0x14
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (20 /*0x14*/ % 32));
    // 004562ac  6689044d507d6700       -mov word ptr [ecx*2 + 0x677d50], ax
    app->getMemory<x86::reg16>(x86::reg32(6782288) /* 0x677d50 */ + cpu.ecx * 2) = cpu.ax;
    // 004562b4  8b430c                 -mov eax, dword ptr [ebx + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(12) /* 0xc */);
    // 004562b7  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 004562ba  c1f814                 -sar eax, 0x14
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (20 /*0x14*/ % 32));
    // 004562bd  6689044d587d6700       -mov word ptr [ecx*2 + 0x677d58], ax
    app->getMemory<x86::reg16>(x86::reg32(6782296) /* 0x677d58 */ + cpu.ecx * 2) = cpu.ax;
    // 004562c5  8b5302                 -mov edx, dword ptr [ebx + 2]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(2) /* 0x2 */);
    // 004562c8  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 004562cb  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004562cd  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004562d0  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004562d2  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 004562d4  8b530c                 -mov edx, dword ptr [ebx + 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(12) /* 0xc */);
    // 004562d7  c1e214                 -shl edx, 0x14
    cpu.edx <<= 20 /*0x14*/ % 32;
    // 004562da  c1fa14                 -sar edx, 0x14
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (20 /*0x14*/ % 32));
    // 004562dd  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004562df  6689044d687d6700       -mov word ptr [ecx*2 + 0x677d68], ax
    app->getMemory<x86::reg16>(x86::reg32(6782312) /* 0x677d68 */ + cpu.ecx * 2) = cpu.ax;
    // 004562e7  8b5304                 -mov edx, dword ptr [ebx + 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(4) /* 0x4 */);
    // 004562ea  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 004562ed  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004562ef  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004562f2  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004562f4  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 004562f6  8b530c                 -mov edx, dword ptr [ebx + 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(12) /* 0xc */);
    // 004562f9  c1e204                 -shl edx, 4
    cpu.edx <<= 4 /*0x4*/ % 32;
    // 004562fc  c1fa14                 -sar edx, 0x14
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (20 /*0x14*/ % 32));
    // 004562ff  41                     -inc ecx
    (cpu.ecx)++;
    // 00456300  01d0                   +add eax, edx
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
    // 00456302  6689044d5e7d6700       -mov word ptr [ecx*2 + 0x677d5e], ax
    app->getMemory<x86::reg16>(x86::reg32(6782302) /* 0x677d5e */ + cpu.ecx * 2) = cpu.ax;
    // 0045630a  e939ffffff             -jmp 0x456248
    goto L_0x00456248;
L_0x0045630f:
    // 0045630f  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00456311  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00456312  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00456313  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00456314  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00456315  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00456316  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00456317  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_456320: native, nfs3hp::port::buttons::slideTrackMap */

/* sub_456590: native, nfs3hp::port::buttons::drawTrackFacts */

/* sub_456670: native, nfs3hp::port::buttons::drawGlow */

/* sub_456800: native, nfs3hp::port::buttons::trackMapShape */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_456890(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    if (infoOnFrame(app, cpu.sync())) /* port: native (tools/apply_native_frontend.py) */
    {
        cpu.reload();
        return;
    }
    // 00456890  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00456891  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00456892  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00456893  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00456894  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00456896  833d2898550000         +cmp dword ptr [0x559828], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5609512) /* 0x559828 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0045689d  742a                   -je 0x4568c9
    if (cpu.flags.zf)
    {
        goto L_0x004568c9;
    }
    // 0045689f  8b15acd46f00           -mov edx, dword ptr [0x6fd4ac]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7328940) /* 0x6fd4ac */);
    // 004568a5  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 004568a7  a14cbb6f00             -mov eax, dword ptr [0x6fbb4c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7322444) /* 0x6fbb4c */);
    // 004568ac  890d28985500           -mov dword ptr [0x559828], ecx
    app->getMemory<x86::reg32>(x86::reg32(5609512) /* 0x559828 */) = cpu.ecx;
    // 004568b2  e8292c0700             -call 0x4c94e0
    cpu.esp -= 4;
    sub_4c94e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004568b7  ba33000000             -mov edx, 0x33
    cpu.edx = 51 /*0x33*/;
    // 004568bc  8b1d4cbb6f00           -mov ebx, dword ptr [0x6fbb4c]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7322444) /* 0x6fbb4c */);
    // 004568c2  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004568c4  e8272d0700             -call 0x4c95f0
    cpu.esp -= 4;
    nfs3hp::port::descriptionStart(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004568c9:
    // 004568c9  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004568cb  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004568cc  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004568cd  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004568ce  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004568cf  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_4568d0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    if (infoOnEnter(app, cpu.sync())) /* port: native (tools/apply_native_frontend.py) */
    {
        cpu.reload();
        return;
    }
    // 004568d0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004568d1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004568d2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004568d3  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004568d4  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004568d6  83ec50                 -sub esp, 0x50
    (cpu.esp) -= x86::reg32(x86::sreg32(80 /*0x50*/));
    // 004568d9  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004568db  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 004568e0  a1acd46f00             -mov eax, dword ptr [0x6fd4ac]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328940) /* 0x6fd4ac */);
    // 004568e5  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004568e7  891528985500           -mov dword ptr [0x559828], edx
    app->getMemory<x86::reg32>(x86::reg32(5609512) /* 0x559828 */) = cpu.edx;
    // 004568ed  a38c7d6700             -mov dword ptr [0x677d8c], eax
    app->getMemory<x86::reg32>(x86::reg32(6782348) /* 0x677d8c */) = cpu.eax;
    // 004568f2  891d18985500           -mov dword ptr [0x559818], ebx
    app->getMemory<x86::reg32>(x86::reg32(5609496) /* 0x559818 */) = cpu.ebx;
    // 004568f8  891d1c985500           -mov dword ptr [0x55981c], ebx
    app->getMemory<x86::reg32>(x86::reg32(5609500) /* 0x55981c */) = cpu.ebx;
    // 004568fe  b8889a5300             -mov eax, 0x539a88
    cpu.eax = 5479048 /*0x539a88*/;
    // 00456903  891d787d6700           -mov dword ptr [0x677d78], ebx
    app->getMemory<x86::reg32>(x86::reg32(6782328) /* 0x677d78 */) = cpu.ebx;
    // 00456909  e8a2f40700             -call 0x4d5db0
    cpu.esp -= 4;
    nfs3hp::port::shapes::shapeByName(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0045690e  66a3927d6700           -mov word ptr [0x677d92], ax
    app->getMemory<x86::reg16>(x86::reg32(6782354) /* 0x677d92 */) = cpu.ax;
    // 00456914  a1acd46f00             -mov eax, dword ptr [0x6fd4ac]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328940) /* 0x6fd4ac */);
    // 00456919  8b148520965500         -mov edx, dword ptr [eax*4 + 0x559620]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5608992) /* 0x559620 */ + cpu.eax * 4);
    // 00456920  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00456921  68b4317a00             -push 0x7a31b4
    app->getMemory<x86::reg32>(cpu.esp-4) = 8008116 /*0x7a31b4*/;
    cpu.esp -= 4;
    // 00456926  68909a5300             -push 0x539a90
    app->getMemory<x86::reg32>(cpu.esp-4) = 5479056 /*0x539a90*/;
    cpu.esp -= 4;
    // 0045692b  8d45b0                 -lea eax, [ebp - 0x50]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-80) /* -0x50 */);
    // 0045692e  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0045692f  e85c8d0800             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00456934  83c410                 -add esp, 0x10
    (cpu.esp) += x86::reg32(x86::sreg32(16 /*0x10*/));
    // 00456937  8d45b0                 -lea eax, [ebp - 0x50]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-80) /* -0x50 */);
    // 0045693a  ba9c9a5300             -mov edx, 0x539a9c
    cpu.edx = 5479068 /*0x539a9c*/;
    // 0045693f  e84c43ffff             -call 0x44ac90
    cpu.esp -= 4;
    nfs3hp::port::buttons::loadShapes(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00456944  a3807d6700             -mov dword ptr [0x677d80], eax
    app->getMemory<x86::reg32>(x86::reg32(6782336) /* 0x677d80 */) = cpu.eax;
    // 00456949  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0045694b  e8f0c0feff             -call 0x442a40
    cpu.esp -= 4;
    nfs3hp::frontend::menuFindItem(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00456950  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00456952  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00456954  741a                   -je 0x456970
    if (cpu.flags.zf)
    {
        goto L_0x00456970;
    }
    // 00456956  baa89a5300             -mov edx, 0x539aa8
    cpu.edx = 5479080 /*0x539aa8*/;
    // 0045695b  a1807d6700             -mov eax, dword ptr [0x677d80]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6782336) /* 0x677d80 */);
    // 00456960  e87b880900             -call 0x4ef1e0
    cpu.esp -= 4;
    sub_4ef1e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00456965  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00456967  e824fa0700             -call 0x4d6390
    cpu.esp -= 4;
    nfs3hp::port::shapes::artShapeTextures(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0045696c  66894144               -mov word ptr [ecx + 0x44], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(68) /* 0x44 */) = cpu.ax;
L_0x00456970:
    // 00456970  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00456972  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00456974  e887feffff             -call 0x456800
    cpu.esp -= 4;
    nfs3hp::port::buttons::trackMapShape(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00456979  66a3907d6700           -mov word ptr [0x677d90], ax
    app->getMemory<x86::reg16>(x86::reg32(6782352) /* 0x677d90 */) = cpu.ax;
    // 0045697f  e87cf8ffff             -call 0x456200
    cpu.esp -= 4;
    sub_456200(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00456984  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00456986  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00456988  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00456989  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045698a  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045698b  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045698c  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_456990(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    if (infoOnExit(app, cpu)) /* port: native (tools/apply_native_frontend.py) */
    {
        return;
    }
    // 00456990  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00456991  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00456993  e8f82b0700             -call 0x4c9590
    cpu.esp -= 4;
    sub_4c9590(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00456998  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0045699a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045699b  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4569a0: native, nfs3hp::port::fields::colourListWidth */

/* sub_456a20: native, nfs3hp::port::fields::colourListRefresh */

/* sub_456b50: native, nfs3hp::port::fields::drawColourLine */

/* sub_456ce0: native, nfs3hp::port::fields::drawColourList */

/* sub_456ea0: native, nfs3hp::port::fields::colourListShown */

/* sub_456f10: native, nfs3hp::port::fields::pointAtColour */

/* sub_456f50: native, nfs3hp::port::fields::openColourList */

/* sub_456f70: native, nfs3hp::port::fields::closeColourList */

/* sub_456f90: native, nfs3hp::port::fields::chooseColour */

/* sub_457030: native, nfs3hp::port::fields::colourListKey */

/* sub_4571e0: native, nfs3hp::port::fields::layoutColourList */

/* sub_457250: native, nfs3hp::port::fields::colourListHidden */

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_457260(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00457260  696e7430008d80         -imul ebp, dword ptr [esi + 0x74], 0x808d0030
    cpu.ebp = x86::reg32(x86::sreg64(x86::sreg32(app->getMemory<x86::reg32>(cpu.esi + x86::reg32(116) /* 0x74 */))) * x86::sreg64(x86::sreg32(2156724272 /*0x808d0030*/)));
    // 00457267  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457269  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045726b  8d5200                 -lea edx, [edx]
    cpu.edx = x86::reg32(cpu.edx);
    // 0045726e  8bdb                   -mov ebx, ebx
    cpu.ebx = cpu.ebx;
    // 00457270  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00457271  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00457272  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00457273  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00457274  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00457276  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 00457278  eb2b                   -jmp 0x4572a5
    goto L_0x004572a5;
L_0x0045727a:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0045727a  f7da                   -neg edx
    cpu.edx = ~cpu.edx + 1;
L_0x0045727c:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0045727c  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 0045727e  8b144582985500         -mov edx, dword ptr [eax*2 + 0x559882]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5609602) /* 0x559882 */ + cpu.eax * 2);
    // 00457285  8b1de8e55500           -mov ebx, dword ptr [0x55e5e8]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5629416) /* 0x55e5e8 */);
    // 0045728b  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0045728e  29d3                   -sub ebx, edx
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00457290  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 00457292  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 00457294  7f02                   -jg 0x457298
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00457298;
    }
    // 00457296  f7da                   -neg edx
    cpu.edx = ~cpu.edx + 1;
L_0x00457298:
    // 00457298  01d1                   -add ecx, edx
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.edx));
    // 0045729a  83f910                 +cmp ecx, 0x10
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(16 /*0x10*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0045729d  7c25                   -jl 0x4572c4
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004572c4;
    }
    // 0045729f  40                     -inc eax
    (cpu.eax)++;
    // 004572a0  83f808                 +cmp eax, 8
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
    // 004572a3  7d1a                   -jge 0x4572bf
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004572bf;
    }
L_0x004572a5:
    // 004572a5  8b144572985500         -mov edx, dword ptr [eax*2 + 0x559872]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5609586) /* 0x559872 */ + cpu.eax * 2);
    // 004572ac  8b0de4e55500           -mov ecx, dword ptr [0x55e5e4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5629412) /* 0x55e5e4 */);
    // 004572b2  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 004572b5  29d1                   -sub ecx, edx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004572b7  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 004572b9  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004572bb  7ebd                   -jle 0x45727a
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0045727a;
    }
    // 004572bd  ebbd                   -jmp 0x45727c
    goto L_0x0045727c;
L_0x004572bf:
    // 004572bf  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
L_0x004572c4:
    // 004572c4  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004572c5  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004572c6  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004572c7  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004572c8  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_457270(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    goto L_entry_0x00457270;
    // 00457260  696e7430008d80         -imul ebp, dword ptr [esi + 0x74], 0x808d0030
    cpu.ebp = x86::reg32(x86::sreg64(x86::sreg32(app->getMemory<x86::reg32>(cpu.esi + x86::reg32(116) /* 0x74 */))) * x86::sreg64(x86::sreg32(2156724272 /*0x808d0030*/)));
    // 00457267  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457269  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045726b  8d5200                 -lea edx, [edx]
    cpu.edx = x86::reg32(cpu.edx);
    // 0045726e  8bdb                   -mov ebx, ebx
    cpu.ebx = cpu.ebx;
L_entry_0x00457270:
    // 00457270  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00457271  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00457272  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00457273  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00457274  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00457276  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 00457278  eb2b                   -jmp 0x4572a5
    goto L_0x004572a5;
L_0x0045727a:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0045727a  f7da                   -neg edx
    cpu.edx = ~cpu.edx + 1;
L_0x0045727c:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0045727c  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 0045727e  8b144582985500         -mov edx, dword ptr [eax*2 + 0x559882]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5609602) /* 0x559882 */ + cpu.eax * 2);
    // 00457285  8b1de8e55500           -mov ebx, dword ptr [0x55e5e8]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5629416) /* 0x55e5e8 */);
    // 0045728b  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0045728e  29d3                   -sub ebx, edx
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00457290  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 00457292  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 00457294  7f02                   -jg 0x457298
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00457298;
    }
    // 00457296  f7da                   -neg edx
    cpu.edx = ~cpu.edx + 1;
L_0x00457298:
    // 00457298  01d1                   -add ecx, edx
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.edx));
    // 0045729a  83f910                 +cmp ecx, 0x10
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(16 /*0x10*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0045729d  7c25                   -jl 0x4572c4
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004572c4;
    }
    // 0045729f  40                     -inc eax
    (cpu.eax)++;
    // 004572a0  83f808                 +cmp eax, 8
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
    // 004572a3  7d1a                   -jge 0x4572bf
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004572bf;
    }
L_0x004572a5:
    // 004572a5  8b144572985500         -mov edx, dword ptr [eax*2 + 0x559872]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5609586) /* 0x559872 */ + cpu.eax * 2);
    // 004572ac  8b0de4e55500           -mov ecx, dword ptr [0x55e5e4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5629412) /* 0x55e5e4 */);
    // 004572b2  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 004572b5  29d1                   -sub ecx, edx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004572b7  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 004572b9  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004572bb  7ebd                   -jle 0x45727a
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0045727a;
    }
    // 004572bd  ebbd                   -jmp 0x45727c
    goto L_0x0045727c;
L_0x004572bf:
    // 004572bf  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
L_0x004572c4:
    // 004572c4  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004572c5  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004572c6  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004572c7  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004572c8  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_4572d0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004572d0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004572d1  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004572d2  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004572d4  a1e4e55500             -mov eax, dword ptr [0x55e5e4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629412) /* 0x55e5e4 */);
    // 004572d9  2d2f020000             -sub eax, 0x22f
    (cpu.eax) -= x86::reg32(x86::sreg32(559 /*0x22f*/));
    // 004572de  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004572e0  7f02                   -jg 0x4572e4
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x004572e4;
    }
    // 004572e2  f7d8                   -neg eax
    cpu.eax = ~cpu.eax + 1;
L_0x004572e4:
    // 004572e4  8b15e8e55500           -mov edx, dword ptr [0x55e5e8]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5629416) /* 0x55e5e8 */);
    // 004572ea  81ea1b010000           -sub edx, 0x11b
    (cpu.edx) -= x86::reg32(x86::sreg32(283 /*0x11b*/));
    // 004572f0  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004572f2  7f02                   -jg 0x4572f6
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x004572f6;
    }
    // 004572f4  f7da                   -neg edx
    cpu.edx = ~cpu.edx + 1;
L_0x004572f6:
    // 004572f6  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004572f8  83f810                 +cmp eax, 0x10
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(16 /*0x10*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004572fb  7d08                   -jge 0x457305
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00457305;
    }
    // 004572fd  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00457302  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457303  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457304  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00457305:
    // 00457305  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00457307  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457308  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457309  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_457310: native, nfs3hp::dead::stub_0x457310 */

/* align: skip 0x8b 0xc0 */
void Application::sub_457340(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 00457340  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00457341  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00457342  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00457343  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00457344  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00457345  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00457346  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00457348  83ec08                 -sub esp, 8
    (cpu.esp) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 0045734b  40                     -inc eax
    (cpu.eax)++;
    // 0045734c  83f808                 +cmp eax, 8
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
    // 0045734f  0f87f2000000           -ja 0x457447
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x00457447;
    }
    // 00457355  ff24851c734500         -jmp dword ptr [eax*4 + 0x45731c]
    cpu.ip = app->getMemory<x86::reg32>(4551452 + cpu.eax * 4); goto dynamic_jump;
  case 0x0045735c:
    // 0045735c  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0045735e  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 00457361  8955f8                 -mov dword ptr [ebp - 8], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.edx;
    // 00457364  ff75fc                 -push dword ptr [ebp - 4]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    cpu.esp -= 4;
    // 00457367  ff75f8                 -push dword ptr [ebp - 8]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    cpu.esp -= 4;
    // 0045736a  e851e10000             -call 0x4654c0
    cpu.esp -= 4;
    sub_4654c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0045736f  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00457371  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457372  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457373  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457374  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457375  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457376  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457377  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x00457378:
    // 00457378  be0000803f             -mov esi, 0x3f800000
    cpu.esi = 1065353216 /*0x3f800000*/;
    // 0045737d  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 0045737f  8975fc                 -mov dword ptr [ebp - 4], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.esi;
    // 00457382  895df8                 -mov dword ptr [ebp - 8], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.ebx;
    // 00457385  ff75fc                 -push dword ptr [ebp - 4]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    cpu.esp -= 4;
    // 00457388  ff75f8                 -push dword ptr [ebp - 8]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    cpu.esp -= 4;
    // 0045738b  e830e10000             -call 0x4654c0
    cpu.esp -= 4;
    sub_4654c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00457390  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00457392  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457393  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457394  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457395  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457396  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457397  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457398  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x00457399:
    // 00457399  b80000803f             -mov eax, 0x3f800000
    cpu.eax = 1065353216 /*0x3f800000*/;
    // 0045739e  bf000080bf             -mov edi, 0xbf800000
    cpu.edi = 3212836864 /*0xbf800000*/;
    // 004573a3  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004573a6  e999000000             -jmp 0x457444
    goto L_0x00457444;
  case 0x004573ab:
    // 004573ab  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 004573ad  ba000080bf             -mov edx, 0xbf800000
    cpu.edx = 3212836864 /*0xbf800000*/;
    // 004573b2  894dfc                 -mov dword ptr [ebp - 4], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
    // 004573b5  8955f8                 -mov dword ptr [ebp - 8], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.edx;
    // 004573b8  ff75fc                 -push dword ptr [ebp - 4]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    cpu.esp -= 4;
    // 004573bb  ff75f8                 -push dword ptr [ebp - 8]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    cpu.esp -= 4;
    // 004573be  e8fde00000             -call 0x4654c0
    cpu.esp -= 4;
    sub_4654c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004573c3  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004573c5  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004573c6  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004573c7  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004573c8  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004573c9  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004573ca  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004573cb  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x004573cc:
    // 004573cc  bb000080bf             -mov ebx, 0xbf800000
    cpu.ebx = 3212836864 /*0xbf800000*/;
    // 004573d1  895dfc                 -mov dword ptr [ebp - 4], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ebx;
    // 004573d4  895df8                 -mov dword ptr [ebp - 8], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.ebx;
    // 004573d7  ff75fc                 -push dword ptr [ebp - 4]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    cpu.esp -= 4;
    // 004573da  ff75f8                 -push dword ptr [ebp - 8]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    cpu.esp -= 4;
    // 004573dd  e8dee00000             -call 0x4654c0
    cpu.esp -= 4;
    sub_4654c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004573e2  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004573e4  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004573e5  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004573e6  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004573e7  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004573e8  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004573e9  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004573ea  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x004573eb:
    // 004573eb  b8000080bf             -mov eax, 0xbf800000
    cpu.eax = 3212836864 /*0xbf800000*/;
    // 004573f0  31ff                   +xor edi, edi
    cpu.clear_co();
    cpu.set_szp((cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi))));
    // 004573f2  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004573f5  eb4d                   -jmp 0x457444
    goto L_0x00457444;
  case 0x004573f7:
    // 004573f7  b9000080bf             -mov ecx, 0xbf800000
    cpu.ecx = 3212836864 /*0xbf800000*/;
    // 004573fc  ba0000803f             -mov edx, 0x3f800000
    cpu.edx = 1065353216 /*0x3f800000*/;
    // 00457401  894dfc                 -mov dword ptr [ebp - 4], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
    // 00457404  8955f8                 -mov dword ptr [ebp - 8], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.edx;
    // 00457407  ff75fc                 -push dword ptr [ebp - 4]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    cpu.esp -= 4;
    // 0045740a  ff75f8                 -push dword ptr [ebp - 8]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    cpu.esp -= 4;
    // 0045740d  e8aee00000             -call 0x4654c0
    cpu.esp -= 4;
    sub_4654c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00457412  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00457414  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457415  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457416  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457417  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457418  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457419  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045741a  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x0045741b:
    // 0045741b  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 0045741d  bb0000803f             -mov ebx, 0x3f800000
    cpu.ebx = 1065353216 /*0x3f800000*/;
    // 00457422  8975fc                 -mov dword ptr [ebp - 4], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.esi;
    // 00457425  895df8                 -mov dword ptr [ebp - 8], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.ebx;
    // 00457428  ff75fc                 -push dword ptr [ebp - 4]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    cpu.esp -= 4;
    // 0045742b  ff75f8                 -push dword ptr [ebp - 8]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    cpu.esp -= 4;
    // 0045742e  e88de00000             -call 0x4654c0
    cpu.esp -= 4;
    sub_4654c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00457433  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00457435  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457436  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457437  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457438  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457439  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045743a  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045743b  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x0045743c:
    // 0045743c  bf0000803f             -mov edi, 0x3f800000
    cpu.edi = 1065353216 /*0x3f800000*/;
    // 00457441  897dfc                 -mov dword ptr [ebp - 4], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edi;
L_0x00457444:
    // 00457444  897df8                 -mov dword ptr [ebp - 8], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.edi;
L_0x00457447:
    // 00457447  ff75fc                 -push dword ptr [ebp - 4]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    cpu.esp -= 4;
    // 0045744a  ff75f8                 -push dword ptr [ebp - 8]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    cpu.esp -= 4;
    // 0045744d  e86ee00000             -call 0x4654c0
    cpu.esp -= 4;
    sub_4654c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00457452  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00457454  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457455  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457456  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457457  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457458  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457459  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045745a  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_457460(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00457460  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00457461  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00457462  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00457463  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00457464  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00457465  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00457467  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00457469  8915a07d6700           -mov dword ptr [0x677da0], edx
    app->getMemory<x86::reg32>(x86::reg32(6782368) /* 0x677da0 */) = cpu.edx;
    // 0045746f  ba22000000             -mov edx, 0x22
    cpu.edx = 34 /*0x22*/;
    // 00457474  e8e731ffff             -call 0x44a660
    cpu.esp -= 4;
    nfs3hp::frontend::menuShownNow(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00457479  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0045747b  0f8419010000           -je 0x45759a
    if (cpu.flags.zf)
    {
        goto L_0x0045759a;
    }
    // 00457481  e8eafdffff             -call 0x457270
    cpu.esp -= 4;
    sub_457270(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00457486  83f8ff                 +cmp eax, -1
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
    // 00457489  0f848c000000           -je 0x45751b
    if (cpu.flags.zf)
    {
        goto L_0x0045751b;
    }
    // 0045748f  8d3400                 -lea esi, [eax + eax]
    cpu.esi = x86::reg32(cpu.eax + cpu.eax * 1);
    // 00457492  8b963c985500           -mov edx, dword ptr [esi + 0x55983c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(5609532) /* 0x55983c */);
    // 00457498  bb12010000             -mov ebx, 0x112
    cpu.ebx = 274 /*0x112*/;
    // 0045749d  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 004574a0  29d3                   -sub ebx, edx
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004574a2  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 004574a4  8b5906                 -mov ebx, dword ptr [ecx + 6]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6) /* 0x6 */);
    // 004574a7  c1fb10                 -sar ebx, 0x10
    cpu.ebx = x86::reg32(x86::sreg32(cpu.ebx) >> (16 /*0x10*/ % 32));
    // 004574aa  29d3                   -sub ebx, edx
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004574ac  8b962a985500           -mov edx, dword ptr [esi + 0x55982a]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(5609514) /* 0x55982a */);
    // 004574b2  bf26020000             -mov edi, 0x226
    cpu.edi = 550 /*0x226*/;
    // 004574b7  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 004574ba  29d7                   -sub edi, edx
    (cpu.edi) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004574bc  8b5104                 -mov edx, dword ptr [ecx + 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(4) /* 0x4 */);
    // 004574bf  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 004574c2  29fa                   -sub edx, edi
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.edi));
    // 004574c4  8d3c8500000000         -lea edi, [eax*4]
    cpu.edi = x86::reg32(cpu.eax * 4);
    // 004574cb  8b87a47d6700           -mov eax, dword ptr [edi + 0x677da4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(6782372) /* 0x677da4 */);
    // 004574d1  e8ba040800             -call 0x4d7990
    cpu.esp -= 4;
    nfs3hp::port::shapes::drawShape(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004574d6  ba12010000             -mov edx, 0x112
    cpu.edx = 274 /*0x112*/;
    // 004574db  8b8660985500           -mov eax, dword ptr [esi + 0x559860]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(5609568) /* 0x559860 */);
    // 004574e1  8b5906                 -mov ebx, dword ptr [ecx + 6]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6) /* 0x6 */);
    // 004574e4  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 004574e7  c1fb10                 -sar ebx, 0x10
    cpu.ebx = x86::reg32(x86::sreg32(cpu.ebx) >> (16 /*0x10*/ % 32));
    // 004574ea  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 004574ec  8b864e985500           -mov eax, dword ptr [esi + 0x55984e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(5609550) /* 0x55984e */);
    // 004574f2  29d3                   -sub ebx, edx
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004574f4  ba26020000             -mov edx, 0x226
    cpu.edx = 550 /*0x226*/;
    // 004574f9  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 004574fc  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 004574fe  8b4104                 -mov eax, dword ptr [ecx + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(4) /* 0x4 */);
    // 00457501  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00457504  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00457506  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00457508  8b87c87d6700           -mov eax, dword ptr [edi + 0x677dc8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(6782408) /* 0x677dc8 */);
    // 0045750e  e87d040800             -call 0x4d7990
    cpu.esp -= 4;
    nfs3hp::port::shapes::drawShape(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00457513  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00457515  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457516  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457517  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457518  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457519  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045751a  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0045751b:
    // 0045751b  e8b0fdffff             -call 0x4572d0
    cpu.esp -= 4;
    sub_4572d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00457520  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00457522  7474                   -je 0x457598
    if (cpu.flags.zf)
    {
        goto L_0x00457598;
    }
    // 00457524  ba12010000             -mov edx, 0x112
    cpu.edx = 274 /*0x112*/;
    // 00457529  a14c985500             -mov eax, dword ptr [0x55984c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5609548) /* 0x55984c */);
    // 0045752e  8b5906                 -mov ebx, dword ptr [ecx + 6]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6) /* 0x6 */);
    // 00457531  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00457534  c1fb10                 -sar ebx, 0x10
    cpu.ebx = x86::reg32(x86::sreg32(cpu.ebx) >> (16 /*0x10*/ % 32));
    // 00457537  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 00457539  a13a985500             -mov eax, dword ptr [0x55983a]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5609530) /* 0x55983a */);
    // 0045753e  29d3                   -sub ebx, edx
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00457540  ba26020000             -mov edx, 0x226
    cpu.edx = 550 /*0x226*/;
    // 00457545  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00457548  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 0045754a  8b4104                 -mov eax, dword ptr [ecx + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(4) /* 0x4 */);
    // 0045754d  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00457550  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00457552  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00457554  a1c47d6700             -mov eax, dword ptr [0x677dc4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6782404) /* 0x677dc4 */);
    // 00457559  e832040800             -call 0x4d7990
    cpu.esp -= 4;
    nfs3hp::port::shapes::drawShape(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0045755e  ba12010000             -mov edx, 0x112
    cpu.edx = 274 /*0x112*/;
    // 00457563  a170985500             -mov eax, dword ptr [0x559870]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5609584) /* 0x559870 */);
    // 00457568  8b5906                 -mov ebx, dword ptr [ecx + 6]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(6) /* 0x6 */);
    // 0045756b  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0045756e  c1fb10                 -sar ebx, 0x10
    cpu.ebx = x86::reg32(x86::sreg32(cpu.ebx) >> (16 /*0x10*/ % 32));
    // 00457571  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 00457573  a15e985500             -mov eax, dword ptr [0x55985e]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5609566) /* 0x55985e */);
    // 00457578  29d3                   -sub ebx, edx
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0045757a  ba26020000             -mov edx, 0x226
    cpu.edx = 550 /*0x226*/;
    // 0045757f  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00457582  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 00457584  8b4104                 -mov eax, dword ptr [ecx + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(4) /* 0x4 */);
    // 00457587  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0045758a  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0045758c  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0045758e  a1e87d6700             -mov eax, dword ptr [0x677de8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6782440) /* 0x677de8 */);
    // 00457593  e8f8030800             -call 0x4d7990
    cpu.esp -= 4;
    nfs3hp::port::shapes::drawShape(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00457598:
    // 00457598  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x0045759a:
    // 0045759a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045759b  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045759c  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045759d  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045759e  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045759f  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_4575a0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004575a0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004575a1  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004575a2  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004575a4  ba22000000             -mov edx, 0x22
    cpu.edx = 34 /*0x22*/;
    // 004575a9  e8b230ffff             -call 0x44a660
    cpu.esp -= 4;
    nfs3hp::frontend::menuShownNow(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004575ae  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 004575b0  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004575b2  745f                   -je 0x457613
    if (cpu.flags.zf)
    {
        goto L_0x00457613;
    }
    // 004575b4  833da07d670000         +cmp dword ptr [0x677da0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(6782368) /* 0x677da0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004575bb  7505                   -jne 0x4575c2
    if (!cpu.flags.zf)
    {
        goto L_0x004575c2;
    }
    // 004575bd  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004575bf  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004575c0  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004575c1  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004575c2:
    // 004575c2  6681fb004b             +cmp bx, 0x4b00
    {
        x86::reg16 tmp1 = cpu.bx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(19200 /*0x4b00*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004575c7  7505                   -jne 0x4575ce
    if (!cpu.flags.zf)
    {
        goto L_0x004575ce;
    }
    // 004575c9  b902000000             -mov ecx, 2
    cpu.ecx = 2 /*0x2*/;
L_0x004575ce:
    // 004575ce  6681fb004d             +cmp bx, 0x4d00
    {
        x86::reg16 tmp1 = cpu.bx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(19712 /*0x4d00*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004575d3  7505                   -jne 0x4575da
    if (!cpu.flags.zf)
    {
        goto L_0x004575da;
    }
    // 004575d5  b902000000             -mov ecx, 2
    cpu.ecx = 2 /*0x2*/;
L_0x004575da:
    // 004575da  6683fb20               +cmp bx, 0x20
    {
        x86::reg16 tmp1 = cpu.bx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(32 /*0x20*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004575de  7505                   -jne 0x4575e5
    if (!cpu.flags.zf)
    {
        goto L_0x004575e5;
    }
    // 004575e0  b902000000             -mov ecx, 2
    cpu.ecx = 2 /*0x2*/;
L_0x004575e5:
    // 004575e5  6683fb0d               +cmp bx, 0xd
    {
        x86::reg16 tmp1 = cpu.bx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(13 /*0xd*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004575e9  7526                   -jne 0x457611
    if (!cpu.flags.zf)
    {
        goto L_0x00457611;
    }
    // 004575eb  e880fcffff             -call 0x457270
    cpu.esp -= 4;
    sub_457270(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004575f0  83f8ff                 +cmp eax, -1
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
    // 004575f3  740a                   -je 0x4575ff
    if (cpu.flags.zf)
    {
        goto L_0x004575ff;
    }
    // 004575f5  e846fdffff             -call 0x457340
    cpu.esp -= 4;
    sub_457340(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004575fa  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004575fc  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004575fd  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004575fe  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004575ff:
    // 004575ff  e8ccfcffff             -call 0x4572d0
    cpu.esp -= 4;
    sub_4572d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00457604  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00457606  7409                   -je 0x457611
    if (cpu.flags.zf)
    {
        goto L_0x00457611;
    }
    // 00457608  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0045760a  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0045760c  e8afde0000             -call 0x4654c0
    cpu.esp -= 4;
    sub_4654c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00457611:
    // 00457611  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
L_0x00457613:
    // 00457613  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457614  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457615  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x54 0x22 0x00 */
void Application::sub_457620(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00457620  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00457621  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00457623  b8fdffffff             -mov eax, 0xfffffffd
    cpu.eax = 4294967293 /*0xfffffffd*/;
    // 00457628  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457629  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_457630(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00457630  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00457631  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00457632  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00457633  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00457635  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00457637  ba22000000             -mov edx, 0x22
    cpu.edx = 34 /*0x22*/;
    // 0045763c  e81f30ffff             -call 0x44a660
    cpu.esp -= 4;
    nfs3hp::frontend::menuShownNow(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00457641  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00457643  7424                   -je 0x457669
    if (cpu.flags.zf)
    {
        goto L_0x00457669;
    }
    // 00457645  668b4106               -mov ax, word ptr [ecx + 6]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(6) /* 0x6 */);
    // 00457649  66c7411e3d00           -mov word ptr [ecx + 0x1e], 0x3d
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(30) /* 0x1e */) = 61 /*0x3d*/;
    // 0045764f  83e815                 -sub eax, 0x15
    (cpu.eax) -= x86::reg32(x86::sreg32(21 /*0x15*/));
    // 00457652  6689411c               -mov word ptr [ecx + 0x1c], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(28) /* 0x1c */) = cpu.ax;
    // 00457656  668b4108               -mov ax, word ptr [ecx + 8]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(8) /* 0x8 */);
    // 0045765a  66c741203d00           -mov word ptr [ecx + 0x20], 0x3d
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(32) /* 0x20 */) = 61 /*0x3d*/;
    // 00457660  83e816                 -sub eax, 0x16
    (cpu.eax) -= x86::reg32(x86::sreg32(22 /*0x16*/));
    // 00457663  6689411a               -mov word ptr [ecx + 0x1a], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(26) /* 0x1a */) = cpu.ax;
    // 00457667  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x00457669:
    // 00457669  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045766a  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045766b  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0045766c  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_457670(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00457670  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00457671  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00457673  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457674  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_457680(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00457680  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00457681  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00457682  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00457683  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00457684  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00457686  83ec08                 -sub esp, 8
    (cpu.esp) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 00457689  8d7df8                 -lea edi, [ebp - 8]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0045768c  be60724500             -mov esi, 0x457260
    cpu.esi = 4551264 /*0x457260*/;
    // 00457691  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 00457693  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00457694  a4                     -movsb byte ptr es:[edi], byte ptr [esi]
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
    // 00457695  eb05                   -jmp 0x45769c
    goto L_0x0045769c;
L_0x00457697:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00457697  83fa09                 +cmp edx, 9
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(9 /*0x9*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0045769a  7d2f                   -jge 0x4576cb
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004576cb;
    }
L_0x0045769c:
    // 0045769c  88d0                   -mov al, dl
    cpu.al = cpu.dl;
    // 0045769e  0431                   -add al, 0x31
    (cpu.al) += x86::reg8(x86::sreg8(49 /*0x31*/));
    // 004576a0  8845fb                 -mov byte ptr [ebp - 5], al
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-5) /* -0x5 */) = cpu.al;
    // 004576a3  8d45f8                 -lea eax, [ebp - 8]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004576a6  e805e70700             -call 0x4d5db0
    cpu.esp -= 4;
    nfs3hp::port::shapes::shapeByName(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004576ab  890495a47d6700         -mov dword ptr [edx*4 + 0x677da4], eax
    app->getMemory<x86::reg32>(x86::reg32(6782372) /* 0x677da4 */ + cpu.edx * 4) = cpu.eax;
    // 004576b2  88d0                   -mov al, dl
    cpu.al = cpu.dl;
    // 004576b4  0461                   +add al, 0x61
    {
        x86::reg8& tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(97 /*0x61*/));
        x86::reg8 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) == (1 & (tmp2 >> 7));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004576b6  8845fb                 -mov byte ptr [ebp - 5], al
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-5) /* -0x5 */) = cpu.al;
    // 004576b9  8d45f8                 -lea eax, [ebp - 8]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004576bc  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004576bd  e8eee60700             -call 0x4d5db0
    cpu.esp -= 4;
    nfs3hp::port::shapes::shapeByName(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004576c2  890495c47d6700         -mov dword ptr [edx*4 + 0x677dc4], eax
    app->getMemory<x86::reg32>(x86::reg32(6782404) /* 0x677dc4 */ + cpu.edx * 4) = cpu.eax;
    // 004576c9  ebcc                   -jmp 0x457697
    goto L_0x00457697;
L_0x004576cb:
    // 004576cb  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004576cd  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004576ce  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004576cf  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004576d0  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004576d1  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_4576e0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004576e0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004576e2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004576e4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004576e6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004576e8  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004576ea  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004576ec  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004576ee  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004576f0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004576f2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004576f4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004576f6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004576f8  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004576fa  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004576fc  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004576fe  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457700  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457702  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457704  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457706  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457708  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045770a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045770c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045770e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457710  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457712  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457714  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457716  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457718  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045771a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045771c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045771e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457720  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457722  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457724  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457726  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457728  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045772a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045772c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045772e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457730  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457732  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457734  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457736  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457738  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045773a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045773c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045773e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457740  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457742  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457744  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457746  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457748  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045774a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045774c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045774e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457750  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457752  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457754  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457756  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457758  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045775a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045775c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045775e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457760  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457762  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457764  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457766  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457768  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045776a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045776c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045776e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457770  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457772  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457774  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457776  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457778  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045777a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045777c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045777e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457780  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457782  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457784  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457786  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457788  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045778a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045778c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045778e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457790  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457792  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457794  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457796  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457798  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045779a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045779c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045779e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577a0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577a2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577a4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577a6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577a8  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577aa  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577ac  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577ae  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577b0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577b2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577b4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577b6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577b8  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577ba  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577bc  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577be  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577c0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577c2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577c4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577c6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577c8  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577ca  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577cc  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577ce  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577d0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577d2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577d4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577d6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577d8  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577da  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577dc  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577de  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577e0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577e2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577e4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577e6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577e8  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577ea  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577ec  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577ee  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577f0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577f2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577f4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577f6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577f8  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577fa  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577fc  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004577fe  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457800  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457802  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457804  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457806  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457808  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045780a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045780c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045780e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457810  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457812  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457814  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457816  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457818  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045781a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045781c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045781e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457820  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457822  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457824  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457826  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457828  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045782a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045782c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045782e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457830  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457832  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457834  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457836  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457838  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045783a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045783c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045783e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457840  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457842  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457844  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457846  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457848  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045784a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045784c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045784e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457850  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457852  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457854  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457856  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457858  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045785a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045785c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045785e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457860  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457862  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457864  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457866  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457868  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045786a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045786c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045786e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457870  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457872  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457874  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457876  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457878  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045787a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045787c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045787e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457880  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457882  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457884  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457886  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457888  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045788a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045788c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045788e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457890  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457892  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457894  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457896  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457898  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045789a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045789c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045789e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578a0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578a2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578a4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578a6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578a8  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578aa  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578ac  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578ae  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578b0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578b2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578b4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578b6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578b8  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578ba  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578bc  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578be  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578c0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578c2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578c4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578c6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578c8  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578ca  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578cc  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578ce  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578d0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578d2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578d4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578d6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578d8  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578da  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578dc  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578de  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578e0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578e2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578e4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578e6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578e8  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578ea  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578ec  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578ee  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578f0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578f2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578f4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578f6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578f8  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578fa  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578fc  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004578fe  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457900  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457902  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457904  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457906  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457908  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045790a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045790c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045790e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457910  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457912  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457914  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457916  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457918  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045791a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045791c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045791e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457920  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457922  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457924  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457926  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457928  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045792a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045792c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045792e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457930  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457932  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457934  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457936  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457938  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045793a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045793c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045793e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457940  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457942  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457944  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457946  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457948  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045794a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045794c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045794e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457950  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457952  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457954  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457956  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457958  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045795a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045795c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045795e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457960  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457962  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457964  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457966  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457968  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045796a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045796c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045796e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457970  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457972  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457974  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457976  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457978  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045797a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045797c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045797e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457980  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457982  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457984  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457986  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457988  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045798a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045798c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045798e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457990  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457992  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457994  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457996  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457998  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045799a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045799c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0045799e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579a0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579a2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579a4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579a6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579a8  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579aa  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579ac  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579ae  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579b0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579b2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579b4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579b6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579b8  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579ba  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579bc  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579be  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579c0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579c2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579c4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579c6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579c8  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579ca  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579cc  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579ce  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579d0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579d2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579d4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579d6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579d8  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579da  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579dc  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579de  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579e0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579e2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579e4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579e6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579e8  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579ea  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579ec  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579ee  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579f0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579f2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579f4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579f6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579f8  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579fa  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579fc  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004579fe  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a00  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a02  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a04  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a06  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a08  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a0a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a0c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a0e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a10  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a12  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a14  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a16  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a18  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a1a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a1c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a1e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a20  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a22  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a24  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a26  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a28  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a2a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a2c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a2e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a30  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a32  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a34  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a36  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a38  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a3a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a3c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a3e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a40  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a42  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a44  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a46  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a48  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a4a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a4c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a4e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a50  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a52  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a54  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a56  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a58  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a5a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a5c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a5e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a60  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a62  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a64  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a66  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a68  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a6a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a6c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a6e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a70  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a72  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a74  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a76  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a78  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a7a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a7c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a7e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a80  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a82  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a84  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a86  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a88  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a8a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a8c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a8e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a90  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a92  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a94  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a96  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a98  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a9a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a9c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457a9e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457aa0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457aa2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457aa4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457aa6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457aa8  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457aaa  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457aac  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457aae  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457ab0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457ab2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457ab4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457ab6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457ab8  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457aba  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457abc  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457abe  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457ac0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457ac2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457ac4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457ac6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457ac8  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457aca  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457acc  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457ace  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457ad0  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457ad2  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457ad4  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457ad6  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457ad8  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457ada  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457adc  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457ade  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00457ae0  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00457ae1  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00457ae2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00457ae3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00457ae5  83ec08                 -sub esp, 8
    (cpu.esp) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 00457ae8  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00457aea  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00457aec  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00457aee  8b3b                   -mov edi, dword ptr [ebx]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebx);
    // 00457af0  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 00457af3  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 00457af5  7c04                   -jl 0x457afb
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00457afb;
    }
    // 00457af7  39f8                   +cmp eax, edi
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
    // 00457af9  7f06                   -jg 0x457b01
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00457b01;
    }
L_0x00457afb:
    // 00457afb  c70300000000           -mov dword ptr [ebx], 0
    app->getMemory<x86::reg32>(cpu.ebx) = 0 /*0x0*/;
L_0x00457b01:
    // 00457b01  6683f933               +cmp cx, 0x33
    {
        x86::reg16 tmp1 = cpu.cx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(51 /*0x33*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 00457b05  7238                   -jb 0x457b3f
    if (cpu.flags.cf)
    {
        goto L_0x00457b3f;
    }
    // 00457b07  0f8680000000           -jbe 0x457b8d
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00457b8d;
    }
    // 00457b0d  6683f937               +cmp cx, 0x37
    {
        x86::reg16 tmp1 = cpu.cx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(55 /*0x37*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 00457b11  7222                   -jb 0x457b35
    if (cpu.flags.cf)
    {
        goto L_0x00457b35;
    }
    // 00457b13  0f8674000000           -jbe 0x457b8d
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00457b8d;
    }
    // 00457b19  6683f939               +cmp cx, 0x39
    {
        x86::reg16 tmp1 = cpu.cx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(57 /*0x39*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 00457b1d  0f826a000000           -jb 0x457b8d
    if (cpu.flags.cf)
    {
        goto L_0x00457b8d;
    }
    // 00457b23  0f8664000000           -jbe 0x457b8d
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00457b8d;
    }
    // 00457b29  6681f90053             +cmp cx, 0x5300
    {
        x86::reg16 tmp1 = cpu.cx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(21248 /*0x5300*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 00457b2e  7447                   -je 0x457b77
    if (cpu.flags.zf)
    {
        goto L_0x00457b77;
    }
    // 00457b30  e9fa000000             -jmp 0x457c2f
    goto L_0x00457c2f;
L_0x00457b35:
    // 00457b35  6683f935               +cmp cx, 0x35
    {
        x86::reg16 tmp1 = cpu.cx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(53 /*0x35*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 00457b39  7252                   -jb 0x457b8d
    if (cpu.flags.cf)
    {
        goto L_0x00457b8d;
    }
    // 00457b3b  7650                   -jbe 0x457b8d
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00457b8d;
    }
    // 00457b3d  eb4e                   -jmp 0x457b8d
    goto L_0x00457b8d;
L_0x00457b3f:
    // 00457b3f  6683f92e               +cmp cx, 0x2e
    {
        x86::reg16 tmp1 = cpu.cx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(46 /*0x2e*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 00457b43  7217                   -jb 0x457b5c
    if (cpu.flags.cf)
    {
        goto L_0x00457b5c;
    }
    // 00457b45  7646                   -jbe 0x457b8d
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00457b8d;
    }
    // 00457b47  6683f931               +cmp cx, 0x31
    {
        x86::reg16 tmp1 = cpu.cx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(49 /*0x31*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 00457b4b  7204                   -jb 0x457b51
    if (cpu.flags.cf)
    {
        goto L_0x00457b51;
    }
    // 00457b4d  763e                   -jbe 0x457b8d
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00457b8d;
    }
    // 00457b4f  eb3c                   -jmp 0x457b8d
    goto L_0x00457b8d;
L_0x00457b51:
    // 00457b51  6683f930               +cmp cx, 0x30
    {
        x86::reg16 tmp1 = cpu.cx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(48 /*0x30*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 00457b55  7436                   -je 0x457b8d
    if (cpu.flags.zf)
    {
        goto L_0x00457b8d;
    }
    // 00457b57  e9d3000000             -jmp 0x457c2f
    goto L_0x00457c2f;
L_0x00457b5c:
    // 00457b5c  6683f908               +cmp cx, 8
    {
        x86::reg16 tmp1 = cpu.cx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(8 /*0x8*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 00457b60  0f82c9000000           -jb 0x457c2f
    if (cpu.flags.cf)
    {
        goto L_0x00457c2f;
    }
    // 00457b66  760f                   -jbe 0x457b77
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00457b77;
    }
    // 00457b68  6683f92d               +cmp cx, 0x2d
    {
        x86::reg16 tmp1 = cpu.cx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(45 /*0x2d*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 00457b6c  0f849f000000           -je 0x457c11
    if (cpu.flags.zf)
    {
        goto L_0x00457c11;
    }
    // 00457b72  e9b8000000             -jmp 0x457c2f
    goto L_0x00457c2f;
L_0x00457b77:
    // 00457b77  c70300000000           -mov dword ptr [ebx], 0
    app->getMemory<x86::reg32>(cpu.ebx) = 0 /*0x0*/;
    // 00457b7d  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 00457b7f  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00457b81  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00457b83  e8b88a0800             -call 0x4e0640
    cpu.esp -= 4;
    nfs3hp::watcom::memset(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00457b88  e99d000000             -jmp 0x457c2a
    goto L_0x00457c2a;
L_0x00457b8d:
    // 00457b8d  6683f92e               +cmp cx, 0x2e
    {
        x86::reg16 tmp1 = cpu.cx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(46 /*0x2e*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 00457b91  750a                   -jne 0x457b9d
    if (!cpu.flags.zf)
    {
        goto L_0x00457b9d;
    }
    // 00457b93  837d1000               +cmp dword ptr [ebp + 0x10], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(16) /* 0x10 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00457b97  0f858d000000           -jne 0x457c2a
    if (!cpu.flags.zf)
    {
        goto L_0x00457c2a;
    }
L_0x00457b9d:
    // 00457b9d  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x00457b9f:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00457b9f  39c2                   +cmp edx, eax
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
    // 00457ba1  7d12                   -jge 0x457bb5
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00457bb5;
    }
    // 00457ba3  8d3c16                 -lea edi, [esi + edx]
    cpu.edi = x86::reg32(cpu.esi + cpu.edx * 1);
    // 00457ba6  803f2e                 +cmp byte ptr [edi], 0x2e
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.edi);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(46 /*0x2e*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 00457ba9  7507                   -jne 0x457bb2
    if (!cpu.flags.zf)
    {
        goto L_0x00457bb2;
    }
    // 00457bab  c745fc01000000         -mov dword ptr [ebp - 4], 1
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = 1 /*0x1*/;
L_0x00457bb2:
    // 00457bb2  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00457bb3  ebea                   -jmp 0x457b9f
    goto L_0x00457b9f;
L_0x00457bb5:
    // 00457bb5  6683f92e               +cmp cx, 0x2e
    {
        x86::reg16 tmp1 = cpu.cx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(46 /*0x2e*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 00457bb9  750a                   -jne 0x457bc5
    if (!cpu.flags.zf)
    {
        goto L_0x00457bc5;
    }
    // 00457bbb  837dfc00               +cmp dword ptr [ebp - 4], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00457bbf  0f8565000000           -jne 0x457c2a
    if (!cpu.flags.zf)
    {
        goto L_0x00457c2a;
    }
L_0x00457bc5:
    // 00457bc5  803e2d                 +cmp byte ptr [esi], 0x2d
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.esi);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(45 /*0x2d*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 00457bc8  7507                   -jne 0x457bd1
    if (!cpu.flags.zf)
    {
        goto L_0x00457bd1;
    }
    // 00457bca  ba02000000             -mov edx, 2
    cpu.edx = 2 /*0x2*/;
    // 00457bcf  eb05                   -jmp 0x457bd6
    goto L_0x00457bd6;
L_0x00457bd1:
    // 00457bd1  ba03000000             -mov edx, 3
    cpu.edx = 3 /*0x3*/;
L_0x00457bd6:
    // 00457bd6  6683f92e               +cmp cx, 0x2e
    {
        x86::reg16 tmp1 = cpu.cx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(46 /*0x2e*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 00457bda  7406                   -je 0x457be2
    if (cpu.flags.zf)
    {
        goto L_0x00457be2;
    }
    // 00457bdc  837dfc00               +cmp dword ptr [ebp - 4], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00457be0  7401                   -je 0x457be3
    if (cpu.flags.zf)
    {
        goto L_0x00457be3;
    }
L_0x00457be2:
    // 00457be2  42                     -inc edx
    (cpu.edx)++;
L_0x00457be3:
    // 00457be3  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 00457be5  29d7                   -sub edi, edx
    (cpu.edi) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00457be7  897df8                 -mov dword ptr [ebp - 8], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.edi;
    // 00457bea  8b3b                   -mov edi, dword ptr [ebx]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebx);
    // 00457bec  3b7df8                 +cmp edi, dword ptr [ebp - 8]
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00457bef  7f04                   -jg 0x457bf5
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00457bf5;
    }
    // 00457bf1  01fe                   -add esi, edi
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.edi));
    // 00457bf3  880e                   -mov byte ptr [esi], cl
    app->getMemory<x86::reg8>(cpu.esi) = cpu.cl;
L_0x00457bf5:
    // 00457bf5  8b0b                   -mov ecx, dword ptr [ebx]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebx);
    // 00457bf7  41                     -inc ecx
    (cpu.ecx)++;
    // 00457bf8  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00457bfa  890b                   -mov dword ptr [ebx], ecx
    app->getMemory<x86::reg32>(cpu.ebx) = cpu.ecx;
    // 00457bfc  39c8                   +cmp eax, ecx
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
    // 00457bfe  7d2a                   -jge 0x457c2a
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00457c2a;
    }
    // 00457c00  8903                   -mov dword ptr [ebx], eax
    app->getMemory<x86::reg32>(cpu.ebx) = cpu.eax;
    // 00457c02  ba02000000             -mov edx, 2
    cpu.edx = 2 /*0x2*/;
    // 00457c07  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00457c09  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00457c0b  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457c0c  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457c0d  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457c0e  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
L_0x00457c11:
    // 00457c11  8b3b                   -mov edi, dword ptr [ebx]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebx);
    // 00457c13  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 00457c15  7513                   -jne 0x457c2a
    if (!cpu.flags.zf)
    {
        goto L_0x00457c2a;
    }
    // 00457c17  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 00457c19  880c32                 -mov byte ptr [edx + esi], cl
    app->getMemory<x86::reg8>(cpu.edx + cpu.esi * 1) = cpu.cl;
    // 00457c1c  ff03                   -inc dword ptr [ebx]
    (app->getMemory<x86::reg32>(cpu.ebx))++;
    // 00457c1e  8d50ff                 -lea edx, [eax - 1]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(-1) /* -0x1 */);
    // 00457c21  3b13                   +cmp edx, dword ptr [ebx]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebx)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00457c23  7f05                   -jg 0x457c2a
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00457c2a;
    }
    // 00457c25  83e802                 -sub eax, 2
    (cpu.eax) -= x86::reg32(x86::sreg32(2 /*0x2*/));
    // 00457c28  8903                   -mov dword ptr [ebx], eax
    app->getMemory<x86::reg32>(cpu.ebx) = cpu.eax;
L_0x00457c2a:
    // 00457c2a  ba02000000             -mov edx, 2
    cpu.edx = 2 /*0x2*/;
L_0x00457c2f:
    // 00457c2f  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00457c31  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00457c33  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457c34  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457c35  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00457c36  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
}

/* sub_457ae0: native, nfs3hp::port::fields::editKey */

/* sub_457c40: native, nfs3hp::port::fields::drawIntField */

/* sub_457df0: native, nfs3hp::port::fields::intRangeMessage */

/* sub_457e30: native, nfs3hp::port::fields::intFieldKey */

/* sub_457f70: native, nfs3hp::port::fields::layoutIntField */

/* sub_458010: native, nfs3hp::port::fields::intFieldShown */

/* sub_458050: native, nfs3hp::port::fields::intFieldHidden */

/* sub_458080: native, nfs3hp::port::fields::bindIntField */

/* sub_4580b0: native, nfs3hp::port::fields::drawStringField */

/* sub_4581f0: native, nfs3hp::port::fields::stringFieldKey */

/* sub_458340: native, nfs3hp::port::fields::layoutStringField */

/* sub_4583b0: native, nfs3hp::port::fields::stringFieldShown */

/* sub_458410: native, nfs3hp::port::fields::stringFieldHidden */

/* align: skip 0x90 */
void Application::sub_458450(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00458450  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00458451  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00458452  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00458453  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00458455  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00458457  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 00458459  ba09000000             -mov edx, 9
    cpu.edx = 9 /*0x9*/;
    // 0045845e  e8fd21ffff             -call 0x44a660
    cpu.esp -= 4;
    nfs3hp::frontend::menuShownNow(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00458463  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00458465  740f                   -je 0x458476
    if (cpu.flags.zf)
    {
        goto L_0x00458476;
    }
    // 00458467  895e50                 -mov dword ptr [esi + 0x50], ebx
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(80) /* 0x50 */) = cpu.ebx;
    // 0045846a  66894e4c               -mov word ptr [esi + 0x4c], cx
    app->getMemory<x86::reg16>(cpu.esi + x86::reg32(76) /* 0x4c */) = cpu.cx;
    // 0045846e  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00458473  897e44                 -mov dword ptr [esi + 0x44], edi
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(68) /* 0x44 */) = cpu.edi;
L_0x00458476:
    // 00458476  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00458477  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00458478  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00458479  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_458480: native, nfs3hp::port::fields::drawCharField */

/* sub_458640: native, nfs3hp::port::fields::charRangeMessage */

/* sub_458690: native, nfs3hp::port::fields::charFieldKey */

/* sub_4587e0: native, nfs3hp::port::fields::layoutCharField */

/* sub_4588a0: native, nfs3hp::port::fields::charFieldShown */

/* sub_4588e0: native, nfs3hp::port::fields::charFieldHidden */

/* sub_458910: native, nfs3hp::port::fields::bindCharField */

/* sub_458940: native, nfs3hp::port::fields::drawFloatField */

/* sub_458b20: native, nfs3hp::port::fields::floatRangeMessage */

}
