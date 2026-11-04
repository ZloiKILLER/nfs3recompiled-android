#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{
// Port (tools/apply_widescreen.py): defined in nfs3hp_main.cpp.
void widescreenHudRect(win32::WinApplication* app, x86::reg32 slot);
// Port (tools/apply_hud_editor.py): defined in nfs3hp_main.cpp.

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x54 0x22 0x00 */
void Application::sub_47fcb0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0047fcb0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0047fcb1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0047fcb2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0047fcb3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0047fcb4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0047fcb5  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0047fcb7  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0047fcb9  83c003                 -add eax, 3
    (cpu.eax) += x86::reg32(x86::sreg32(3 /*0x3*/));
    // 0047fcbc  8d4e02                 -lea ecx, [esi + 2]
    cpu.ecx = x86::reg32(cpu.esi + x86::reg32(2) /* 0x2 */);
    // 0047fcbf  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0047fcc0  8d5e01                 -lea ebx, [esi + 1]
    cpu.ebx = x86::reg32(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 0047fcc3  b8389a7400             -mov eax, 0x749a38
    cpu.eax = 7641656 /*0x749a38*/;
    // 0047fcc8  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0047fcca  e8f1c00100             -call 0x49bdc0
    cpu.esp -= 4;
    nfs3hp::port::text::unqueueQuad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047fccf  8d4607                 -lea eax, [esi + 7]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(7) /* 0x7 */);
    // 0047fcd2  8d4e06                 -lea ecx, [esi + 6]
    cpu.ecx = x86::reg32(cpu.esi + x86::reg32(6) /* 0x6 */);
    // 0047fcd5  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0047fcd6  8d5e05                 -lea ebx, [esi + 5]
    cpu.ebx = x86::reg32(cpu.esi + x86::reg32(5) /* 0x5 */);
    // 0047fcd9  b8389a7400             -mov eax, 0x749a38
    cpu.eax = 7641656 /*0x749a38*/;
    // 0047fcde  8d5604                 -lea edx, [esi + 4]
    cpu.edx = x86::reg32(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 0047fce1  e8dac00100             -call 0x49bdc0
    cpu.esp -= 4;
    nfs3hp::port::text::unqueueQuad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047fce6  8d460b                 -lea eax, [esi + 0xb]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(11) /* 0xb */);
    // 0047fce9  8d4e0a                 -lea ecx, [esi + 0xa]
    cpu.ecx = x86::reg32(cpu.esi + x86::reg32(10) /* 0xa */);
    // 0047fcec  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0047fced  8d5e09                 -lea ebx, [esi + 9]
    cpu.ebx = x86::reg32(cpu.esi + x86::reg32(9) /* 0x9 */);
    // 0047fcf0  b8389a7400             -mov eax, 0x749a38
    cpu.eax = 7641656 /*0x749a38*/;
    // 0047fcf5  8d5608                 -lea edx, [esi + 8]
    cpu.edx = x86::reg32(cpu.esi + x86::reg32(8) /* 0x8 */);
    // 0047fcf8  e8c3c00100             -call 0x49bdc0
    cpu.esp -= 4;
    nfs3hp::port::text::unqueueQuad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047fcfd  8d460f                 -lea eax, [esi + 0xf]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(15) /* 0xf */);
    // 0047fd00  8d4e0e                 -lea ecx, [esi + 0xe]
    cpu.ecx = x86::reg32(cpu.esi + x86::reg32(14) /* 0xe */);
    // 0047fd03  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0047fd04  8d5e0d                 -lea ebx, [esi + 0xd]
    cpu.ebx = x86::reg32(cpu.esi + x86::reg32(13) /* 0xd */);
    // 0047fd07  b8389a7400             -mov eax, 0x749a38
    cpu.eax = 7641656 /*0x749a38*/;
    // 0047fd0c  8d560c                 -lea edx, [esi + 0xc]
    cpu.edx = x86::reg32(cpu.esi + x86::reg32(12) /* 0xc */);
    // 0047fd0f  e8acc00100             -call 0x49bdc0
    cpu.esp -= 4;
    nfs3hp::port::text::unqueueQuad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047fd14  8d4613                 -lea eax, [esi + 0x13]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(19) /* 0x13 */);
    // 0047fd17  8d4e12                 -lea ecx, [esi + 0x12]
    cpu.ecx = x86::reg32(cpu.esi + x86::reg32(18) /* 0x12 */);
    // 0047fd1a  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0047fd1b  8d5e11                 -lea ebx, [esi + 0x11]
    cpu.ebx = x86::reg32(cpu.esi + x86::reg32(17) /* 0x11 */);
    // 0047fd1e  b8389a7400             -mov eax, 0x749a38
    cpu.eax = 7641656 /*0x749a38*/;
    // 0047fd23  8d5610                 -lea edx, [esi + 0x10]
    cpu.edx = x86::reg32(cpu.esi + x86::reg32(16) /* 0x10 */);
    // 0047fd26  e895c00100             -call 0x49bdc0
    cpu.esp -= 4;
    nfs3hp::port::text::unqueueQuad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047fd2b  8d4617                 -lea eax, [esi + 0x17]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(23) /* 0x17 */);
    // 0047fd2e  8d4e16                 -lea ecx, [esi + 0x16]
    cpu.ecx = x86::reg32(cpu.esi + x86::reg32(22) /* 0x16 */);
    // 0047fd31  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0047fd32  8d5e15                 -lea ebx, [esi + 0x15]
    cpu.ebx = x86::reg32(cpu.esi + x86::reg32(21) /* 0x15 */);
    // 0047fd35  b8389a7400             -mov eax, 0x749a38
    cpu.eax = 7641656 /*0x749a38*/;
    // 0047fd3a  8d5614                 -lea edx, [esi + 0x14]
    cpu.edx = x86::reg32(cpu.esi + x86::reg32(20) /* 0x14 */);
    // 0047fd3d  e87ec00100             -call 0x49bdc0
    cpu.esp -= 4;
    nfs3hp::port::text::unqueueQuad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047fd42  8d461b                 -lea eax, [esi + 0x1b]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(27) /* 0x1b */);
    // 0047fd45  8d4e1a                 -lea ecx, [esi + 0x1a]
    cpu.ecx = x86::reg32(cpu.esi + x86::reg32(26) /* 0x1a */);
    // 0047fd48  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0047fd49  8d5e19                 -lea ebx, [esi + 0x19]
    cpu.ebx = x86::reg32(cpu.esi + x86::reg32(25) /* 0x19 */);
    // 0047fd4c  b8389a7400             -mov eax, 0x749a38
    cpu.eax = 7641656 /*0x749a38*/;
    // 0047fd51  8d5618                 -lea edx, [esi + 0x18]
    cpu.edx = x86::reg32(cpu.esi + x86::reg32(24) /* 0x18 */);
    // 0047fd54  e867c00100             -call 0x49bdc0
    cpu.esp -= 4;
    nfs3hp::port::text::unqueueQuad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047fd59  8d461f                 -lea eax, [esi + 0x1f]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(31) /* 0x1f */);
    // 0047fd5c  8d4e1e                 -lea ecx, [esi + 0x1e]
    cpu.ecx = x86::reg32(cpu.esi + x86::reg32(30) /* 0x1e */);
    // 0047fd5f  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0047fd60  8d5e1d                 -lea ebx, [esi + 0x1d]
    cpu.ebx = x86::reg32(cpu.esi + x86::reg32(29) /* 0x1d */);
    // 0047fd63  b8389a7400             -mov eax, 0x749a38
    cpu.eax = 7641656 /*0x749a38*/;
    // 0047fd68  8d561c                 -lea edx, [esi + 0x1c]
    cpu.edx = x86::reg32(cpu.esi + x86::reg32(28) /* 0x1c */);
    // 0047fd6b  e850c00100             -call 0x49bdc0
    cpu.esp -= 4;
    nfs3hp::port::text::unqueueQuad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047fd70  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047fd71  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047fd72  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047fd73  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047fd74  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0047fd75  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x54 0x22 0x00 */
void Application::sub_47fd80(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0;
  x86::reg16 fsw = 0;
    // 0047fd80  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0047fd81  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0047fd82  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0047fd83  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0047fd84  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0047fd85  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0047fd86  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0047fd88  83ec3c                 +sub esp, 0x3c
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(60 /*0x3c*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0047fd8b  a1c8da7c00             -mov eax, dword ptr [0x7cdac8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8182472) /* 0x7cdac8 */);
    // 0047fd90  8b3d901a7900           -mov edi, dword ptr [0x791a90]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7936656) /* 0x791a90 */);
    // 0047fd96  8b15f0977400           -mov edx, dword ptr [0x7497f0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7641072) /* 0x7497f0 */);
    // 0047fd9c  8b0df4977400           -mov ecx, dword ptr [0x7497f4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7641076) /* 0x7497f4 */);
    // 0047fda2  8b1de8977400           -mov ebx, dword ptr [0x7497e8]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7641064) /* 0x7497e8 */);
    // 0047fda8  8945c8                 -mov dword ptr [ebp - 0x38], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-56) /* -0x38 */) = cpu.eax;
    // 0047fdab  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0047fdac  a1ccda7c00             -mov eax, dword ptr [0x7cdacc]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8182476) /* 0x7cdacc */);
    // 0047fdb1  8b15ec977400           -mov edx, dword ptr [0x7497ec]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7641068) /* 0x7497ec */);
    // 0047fdb7  8945cc                 -mov dword ptr [ebp - 0x34], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-52) /* -0x34 */) = cpu.eax;
    // 0047fdba  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0047fdbc  8b35801a7900           -mov esi, dword ptr [0x791a80]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7936640) /* 0x791a80 */);
    // 0047fdc2  e889f8ffff             -call 0x47f650
    cpu.esp -= 4;
    nfs3hp::render::hudFrame(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047fdc7  8d4603                 -lea eax, [esi + 3]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(3) /* 0x3 */);
    // 0047fdca  8d4e02                 -lea ecx, [esi + 2]
    cpu.ecx = x86::reg32(cpu.esi + x86::reg32(2) /* 0x2 */);
    // 0047fdcd  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0047fdce  8d5e01                 -lea ebx, [esi + 1]
    cpu.ebx = x86::reg32(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 0047fdd1  b848027000             -mov eax, 0x700248
    cpu.eax = 7340616 /*0x700248*/;
    // 0047fdd6  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0047fdd8  e853bf0100             -call 0x49bd30
    cpu.esp -= 4;
    nfs3hp::port::text::queueQuad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047fddd  8d4607                 -lea eax, [esi + 7]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(7) /* 0x7 */);
    // 0047fde0  8d4e06                 -lea ecx, [esi + 6]
    cpu.ecx = x86::reg32(cpu.esi + x86::reg32(6) /* 0x6 */);
    // 0047fde3  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0047fde4  8d5e05                 -lea ebx, [esi + 5]
    cpu.ebx = x86::reg32(cpu.esi + x86::reg32(5) /* 0x5 */);
    // 0047fde7  b848027000             -mov eax, 0x700248
    cpu.eax = 7340616 /*0x700248*/;
    // 0047fdec  8d5604                 -lea edx, [esi + 4]
    cpu.edx = x86::reg32(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 0047fdef  e83cbf0100             -call 0x49bd30
    cpu.esp -= 4;
    nfs3hp::port::text::queueQuad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047fdf4  8d460b                 -lea eax, [esi + 0xb]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(11) /* 0xb */);
    // 0047fdf7  8945d4                 -mov dword ptr [ebp - 0x2c], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44) /* -0x2c */) = cpu.eax;
    // 0047fdfa  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0047fdfb  8d460a                 -lea eax, [esi + 0xa]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(10) /* 0xa */);
    // 0047fdfe  8945ec                 -mov dword ptr [ebp - 0x14], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.eax;
    // 0047fe01  8d4609                 -lea eax, [esi + 9]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(9) /* 0x9 */);
    // 0047fe04  8b4dec                 -mov ecx, dword ptr [ebp - 0x14]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0047fe07  8945f0                 -mov dword ptr [ebp - 0x10], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.eax;
    // 0047fe0a  8d4608                 -lea eax, [esi + 8]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(8) /* 0x8 */);
    // 0047fe0d  8b5df0                 -mov ebx, dword ptr [ebp - 0x10]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0047fe10  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
    // 0047fe13  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0047fe15  b848027000             -mov eax, 0x700248
    cpu.eax = 7340616 /*0x700248*/;
    // 0047fe1a  e811bf0100             -call 0x49bd30
    cpu.esp -= 4;
    nfs3hp::port::text::queueQuad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047fe1f  8d460f                 -lea eax, [esi + 0xf]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(15) /* 0xf */);
    // 0047fe22  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 0047fe25  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0047fe26  8d460e                 -lea eax, [esi + 0xe]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(14) /* 0xe */);
    // 0047fe29  8945e4                 -mov dword ptr [ebp - 0x1c], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */) = cpu.eax;
    // 0047fe2c  8d460d                 -lea eax, [esi + 0xd]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(13) /* 0xd */);
    // 0047fe2f  8b4de4                 -mov ecx, dword ptr [ebp - 0x1c]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 0047fe32  8945e8                 -mov dword ptr [ebp - 0x18], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = cpu.eax;
    // 0047fe35  8d460c                 -lea eax, [esi + 0xc]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(12) /* 0xc */);
    // 0047fe38  8b5de8                 -mov ebx, dword ptr [ebp - 0x18]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 0047fe3b  8945d8                 -mov dword ptr [ebp - 0x28], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-40) /* -0x28 */) = cpu.eax;
    // 0047fe3e  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0047fe40  b848027000             -mov eax, 0x700248
    cpu.eax = 7340616 /*0x700248*/;
    // 0047fe45  e8e6be0100             -call 0x49bd30
    cpu.esp -= 4;
    nfs3hp::port::text::queueQuad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047fe4a  8d4613                 -lea eax, [esi + 0x13]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(19) /* 0x13 */);
    // 0047fe4d  8945dc                 -mov dword ptr [ebp - 0x24], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = cpu.eax;
    // 0047fe50  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0047fe51  8d4612                 -lea eax, [esi + 0x12]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(18) /* 0x12 */);
    // 0047fe54  8945e0                 -mov dword ptr [ebp - 0x20], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */) = cpu.eax;
    // 0047fe57  8d4611                 -lea eax, [esi + 0x11]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(17) /* 0x11 */);
    // 0047fe5a  8b4de0                 -mov ecx, dword ptr [ebp - 0x20]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 0047fe5d  8945d0                 -mov dword ptr [ebp - 0x30], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-48) /* -0x30 */) = cpu.eax;
    // 0047fe60  8d4610                 -lea eax, [esi + 0x10]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(16) /* 0x10 */);
    // 0047fe63  8b5dd0                 -mov ebx, dword ptr [ebp - 0x30]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-48) /* -0x30 */);
    // 0047fe66  8945c4                 -mov dword ptr [ebp - 0x3c], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-60) /* -0x3c */) = cpu.eax;
    // 0047fe69  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0047fe6b  b848027000             -mov eax, 0x700248
    cpu.eax = 7340616 /*0x700248*/;
    // 0047fe70  e8bbbe0100             -call 0x49bd30
    cpu.esp -= 4;
    nfs3hp::port::text::queueQuad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047fe75  8d4617                 -lea eax, [esi + 0x17]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(23) /* 0x17 */);
    // 0047fe78  8d4e16                 -lea ecx, [esi + 0x16]
    cpu.ecx = x86::reg32(cpu.esi + x86::reg32(22) /* 0x16 */);
    // 0047fe7b  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0047fe7c  8d5e15                 -lea ebx, [esi + 0x15]
    cpu.ebx = x86::reg32(cpu.esi + x86::reg32(21) /* 0x15 */);
    // 0047fe7f  b848027000             -mov eax, 0x700248
    cpu.eax = 7340616 /*0x700248*/;
    // 0047fe84  8d5614                 -lea edx, [esi + 0x14]
    cpu.edx = x86::reg32(cpu.esi + x86::reg32(20) /* 0x14 */);
    // 0047fe87  e8a4be0100             -call 0x49bd30
    cpu.esp -= 4;
    nfs3hp::port::text::queueQuad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047fe8c  8d461b                 -lea eax, [esi + 0x1b]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(27) /* 0x1b */);
    // 0047fe8f  8d4e1a                 -lea ecx, [esi + 0x1a]
    cpu.ecx = x86::reg32(cpu.esi + x86::reg32(26) /* 0x1a */);
    // 0047fe92  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0047fe93  8d5e19                 -lea ebx, [esi + 0x19]
    cpu.ebx = x86::reg32(cpu.esi + x86::reg32(25) /* 0x19 */);
    // 0047fe96  b848027000             -mov eax, 0x700248
    cpu.eax = 7340616 /*0x700248*/;
    // 0047fe9b  8d5618                 -lea edx, [esi + 0x18]
    cpu.edx = x86::reg32(cpu.esi + x86::reg32(24) /* 0x18 */);
    // 0047fe9e  e88dbe0100             -call 0x49bd30
    cpu.esp -= 4;
    nfs3hp::port::text::queueQuad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047fea3  8d461f                 -lea eax, [esi + 0x1f]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(31) /* 0x1f */);
    // 0047fea6  8d4e1e                 -lea ecx, [esi + 0x1e]
    cpu.ecx = x86::reg32(cpu.esi + x86::reg32(30) /* 0x1e */);
    // 0047fea9  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0047feaa  8d5e1d                 -lea ebx, [esi + 0x1d]
    cpu.ebx = x86::reg32(cpu.esi + x86::reg32(29) /* 0x1d */);
    // 0047fead  b848027000             -mov eax, 0x700248
    cpu.eax = 7340616 /*0x700248*/;
    // 0047feb2  8d561c                 -lea edx, [esi + 0x1c]
    cpu.edx = x86::reg32(cpu.esi + x86::reg32(28) /* 0x1c */);
    // 0047feb5  e876be0100             -call 0x49bd30
    cpu.esp -= 4;
    nfs3hp::port::text::queueQuad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047feba  8b45c8                 -mov eax, dword ptr [ebp - 0x38]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-56) /* -0x38 */);
    // 0047febd  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0047fec0  db45fc                 +fild dword ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 0047fec3  d89f20010000           +fcomp dword ptr [edi + 0x120]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.edi + x86::reg32(288) /* 0x120 */)));
    // 0047fec9  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 0047fecb  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 0047fecc  7767                   -ja 0x47ff35
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x0047ff35;
    }
    // 0047fece  8b45c8                 -mov eax, dword ptr [ebp - 0x38]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-56) /* -0x38 */);
    // 0047fed1  8b55d4                 -mov edx, dword ptr [ebp - 0x2c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44) /* -0x2c */);
    // 0047fed4  8b4dec                 -mov ecx, dword ptr [ebp - 0x14]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0047fed7  8b5df0                 -mov ebx, dword ptr [ebp - 0x10]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0047feda  48                     +dec eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 0047fedb  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0047fedc  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0047fedf  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0047fee2  db45fc                 +fild dword ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 0047fee5  b848027000             -mov eax, 0x700248
    cpu.eax = 7340616 /*0x700248*/;
    // 0047feea  d997c0020000           +fst dword ptr [edi + 0x2c0]
    app->getMemory<float>(cpu.edi + x86::reg32(704) /* 0x2c0 */) = float(fp0);
    // 0047fef0  d997a0020000           +fst dword ptr [edi + 0x2a0]
    app->getMemory<float>(cpu.edi + x86::reg32(672) /* 0x2a0 */) = float(fp0);
    // 0047fef6  d997c0000000           +fst dword ptr [edi + 0xc0]
    app->getMemory<float>(cpu.edi + x86::reg32(192) /* 0xc0 */) = float(fp0);
    // 0047fefc  d99fa0000000           +fstp dword ptr [edi + 0xa0]
    app->getMemory<float>(cpu.edi + x86::reg32(160) /* 0xa0 */) = float(fp0);
    // 0047ff02  e8b9be0100             -call 0x49bdc0
    cpu.esp -= 4;
    nfs3hp::port::text::unqueueQuad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047ff07  8b4df8                 -mov ecx, dword ptr [ebp - 8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0047ff0a  8b5de8                 -mov ebx, dword ptr [ebp - 0x18]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 0047ff0d  8b55d8                 -mov edx, dword ptr [ebp - 0x28]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-40) /* -0x28 */);
    // 0047ff10  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0047ff11  b848027000             -mov eax, 0x700248
    cpu.eax = 7340616 /*0x700248*/;
    // 0047ff16  8b4de4                 -mov ecx, dword ptr [ebp - 0x1c]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 0047ff19  e8a2be0100             -call 0x49bdc0
    cpu.esp -= 4;
    nfs3hp::port::text::unqueueQuad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047ff1e  8b5ddc                 -mov ebx, dword ptr [ebp - 0x24]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 0047ff21  8b4de0                 -mov ecx, dword ptr [ebp - 0x20]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 0047ff24  8b55c4                 -mov edx, dword ptr [ebp - 0x3c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-60) /* -0x3c */);
    // 0047ff27  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0047ff28  b848027000             -mov eax, 0x700248
    cpu.eax = 7340616 /*0x700248*/;
    // 0047ff2d  8b5dd0                 -mov ebx, dword ptr [ebp - 0x30]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-48) /* -0x30 */);
    // 0047ff30  e88bbe0100             -call 0x49bdc0
    cpu.esp -= 4;
    nfs3hp::port::text::unqueueQuad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0047ff35:
    // 0047ff35  d9ee                   +fldz 
    fp0 = 0.0;
    // 0047ff37  d81f                   +fcomp dword ptr [edi]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.edi)));
    // 0047ff39  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 0047ff3b  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 0047ff3c  7666                   -jbe 0x47ffa4
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0047ffa4;
    }
    // 0047ff3e  c787e002000000000000   -mov dword ptr [edi + 0x2e0], 0
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(736) /* 0x2e0 */) = 0 /*0x0*/;
    // 0047ff48  8d4603                 -lea eax, [esi + 3]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(3) /* 0x3 */);
    // 0047ff4b  d987e0020000           +fld dword ptr [edi + 0x2e0]
    fp0 = x86::Float(app->getMemory<float>(cpu.edi + x86::reg32(736) /* 0x2e0 */));
    // 0047ff51  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0047ff52  8d4e02                 -lea ecx, [esi + 2]
    cpu.ecx = x86::reg32(cpu.esi + x86::reg32(2) /* 0x2 */);
    // 0047ff55  d99780020000           +fst dword ptr [edi + 0x280]
    app->getMemory<float>(cpu.edi + x86::reg32(640) /* 0x280 */) = float(fp0);
    // 0047ff5b  b848027000             -mov eax, 0x700248
    cpu.eax = 7340616 /*0x700248*/;
    // 0047ff60  8d5e01                 -lea ebx, [esi + 1]
    cpu.ebx = x86::reg32(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 0047ff63  d997e0000000           +fst dword ptr [edi + 0xe0]
    app->getMemory<float>(cpu.edi + x86::reg32(224) /* 0xe0 */) = float(fp0);
    // 0047ff69  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0047ff6b  d99f80000000           +fstp dword ptr [edi + 0x80]
    app->getMemory<float>(cpu.edi + x86::reg32(128) /* 0x80 */) = float(fp0);
    // 0047ff71  e84abe0100             -call 0x49bdc0
    cpu.esp -= 4;
    nfs3hp::port::text::unqueueQuad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047ff76  8d461b                 -lea eax, [esi + 0x1b]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(27) /* 0x1b */);
    // 0047ff79  8d4e1a                 -lea ecx, [esi + 0x1a]
    cpu.ecx = x86::reg32(cpu.esi + x86::reg32(26) /* 0x1a */);
    // 0047ff7c  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0047ff7d  8d5e19                 -lea ebx, [esi + 0x19]
    cpu.ebx = x86::reg32(cpu.esi + x86::reg32(25) /* 0x19 */);
    // 0047ff80  b848027000             -mov eax, 0x700248
    cpu.eax = 7340616 /*0x700248*/;
    // 0047ff85  8d5618                 -lea edx, [esi + 0x18]
    cpu.edx = x86::reg32(cpu.esi + x86::reg32(24) /* 0x18 */);
    // 0047ff88  e833be0100             -call 0x49bdc0
    cpu.esp -= 4;
    nfs3hp::port::text::unqueueQuad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047ff8d  8d461f                 -lea eax, [esi + 0x1f]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(31) /* 0x1f */);
    // 0047ff90  8d4e1e                 -lea ecx, [esi + 0x1e]
    cpu.ecx = x86::reg32(cpu.esi + x86::reg32(30) /* 0x1e */);
    // 0047ff93  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0047ff94  8d5e1d                 -lea ebx, [esi + 0x1d]
    cpu.ebx = x86::reg32(cpu.esi + x86::reg32(29) /* 0x1d */);
    // 0047ff97  b848027000             -mov eax, 0x700248
    cpu.eax = 7340616 /*0x700248*/;
    // 0047ff9c  8d561c                 -lea edx, [esi + 0x1c]
    cpu.edx = x86::reg32(cpu.esi + x86::reg32(28) /* 0x1c */);
    // 0047ff9f  e81cbe0100             -call 0x49bdc0
    cpu.esp -= 4;
    nfs3hp::port::text::unqueueQuad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0047ffa4:
    // 0047ffa4  d9ee                   +fldz 
    fp0 = 0.0;
    // 0047ffa6  d85f04                 +fcomp dword ptr [edi + 4]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.edi + x86::reg32(4) /* 0x4 */)));
    // 0047ffa9  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 0047ffab  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 0047ffac  7666                   -jbe 0x480014
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00480014;
    }
    // 0047ffae  c787a403000000000000   -mov dword ptr [edi + 0x3a4], 0
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(932) /* 0x3a4 */) = 0 /*0x0*/;
    // 0047ffb8  8d4603                 -lea eax, [esi + 3]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(3) /* 0x3 */);
    // 0047ffbb  d987a4030000           +fld dword ptr [edi + 0x3a4]
    fp0 = x86::Float(app->getMemory<float>(cpu.edi + x86::reg32(932) /* 0x3a4 */));
    // 0047ffc1  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0047ffc2  8d4e02                 -lea ecx, [esi + 2]
    cpu.ecx = x86::reg32(cpu.esi + x86::reg32(2) /* 0x2 */);
    // 0047ffc5  d99784030000           +fst dword ptr [edi + 0x384]
    app->getMemory<float>(cpu.edi + x86::reg32(900) /* 0x384 */) = float(fp0);
    // 0047ffcb  b848027000             -mov eax, 0x700248
    cpu.eax = 7340616 /*0x700248*/;
    // 0047ffd0  8d5e01                 -lea ebx, [esi + 1]
    cpu.ebx = x86::reg32(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 0047ffd3  d997a4010000           +fst dword ptr [edi + 0x1a4]
    app->getMemory<float>(cpu.edi + x86::reg32(420) /* 0x1a4 */) = float(fp0);
    // 0047ffd9  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0047ffdb  d99f84010000           +fstp dword ptr [edi + 0x184]
    app->getMemory<float>(cpu.edi + x86::reg32(388) /* 0x184 */) = float(fp0);
    // 0047ffe1  e8dabd0100             -call 0x49bdc0
    cpu.esp -= 4;
    nfs3hp::port::text::unqueueQuad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047ffe6  8d4607                 -lea eax, [esi + 7]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(7) /* 0x7 */);
    // 0047ffe9  8d4e06                 -lea ecx, [esi + 6]
    cpu.ecx = x86::reg32(cpu.esi + x86::reg32(6) /* 0x6 */);
    // 0047ffec  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0047ffed  8d5e05                 -lea ebx, [esi + 5]
    cpu.ebx = x86::reg32(cpu.esi + x86::reg32(5) /* 0x5 */);
    // 0047fff0  b848027000             -mov eax, 0x700248
    cpu.eax = 7340616 /*0x700248*/;
    // 0047fff5  8d5604                 -lea edx, [esi + 4]
    cpu.edx = x86::reg32(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 0047fff8  e8c3bd0100             -call 0x49bdc0
    cpu.esp -= 4;
    nfs3hp::port::text::unqueueQuad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0047fffd  8d460b                 -lea eax, [esi + 0xb]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(11) /* 0xb */);
    // 00480000  8d4e0a                 -lea ecx, [esi + 0xa]
    cpu.ecx = x86::reg32(cpu.esi + x86::reg32(10) /* 0xa */);
    // 00480003  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00480004  8d5e09                 -lea ebx, [esi + 9]
    cpu.ebx = x86::reg32(cpu.esi + x86::reg32(9) /* 0x9 */);
    // 00480007  b848027000             -mov eax, 0x700248
    cpu.eax = 7340616 /*0x700248*/;
    // 0048000c  8d5608                 -lea edx, [esi + 8]
    cpu.edx = x86::reg32(cpu.esi + x86::reg32(8) /* 0x8 */);
    // 0048000f  e8acbd0100             -call 0x49bdc0
    cpu.esp -= 4;
    nfs3hp::port::text::unqueueQuad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00480014:
    // 00480014  8b45cc                 -mov eax, dword ptr [ebp - 0x34]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-52) /* -0x34 */);
    // 00480017  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0048001a  db45fc                 +fild dword ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 0048001d  d89f44020000           +fcomp dword ptr [edi + 0x244]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.edi + x86::reg32(580) /* 0x244 */)));
    // 00480023  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00480025  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00480026  7767                   -ja 0x48008f
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x0048008f;
    }
    // 00480028  8b45cc                 -mov eax, dword ptr [ebp - 0x34]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-52) /* -0x34 */);
    // 0048002b  48                     -dec eax
    (cpu.eax)--;
    // 0048002c  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0048002f  db45fc                 -fild dword ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 00480032  8d4613                 -lea eax, [esi + 0x13]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(19) /* 0x13 */);
    // 00480035  d997e4030000           -fst dword ptr [edi + 0x3e4]
    app->getMemory<float>(cpu.edi + x86::reg32(996) /* 0x3e4 */) = float(fp0);
    // 0048003b  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0048003c  8d4e12                 -lea ecx, [esi + 0x12]
    cpu.ecx = x86::reg32(cpu.esi + x86::reg32(18) /* 0x12 */);
    // 0048003f  d997c4030000           -fst dword ptr [edi + 0x3c4]
    app->getMemory<float>(cpu.edi + x86::reg32(964) /* 0x3c4 */) = float(fp0);
    // 00480045  b848027000             -mov eax, 0x700248
    cpu.eax = 7340616 /*0x700248*/;
    // 0048004a  8d5e11                 -lea ebx, [esi + 0x11]
    cpu.ebx = x86::reg32(cpu.esi + x86::reg32(17) /* 0x11 */);
    // 0048004d  d997e4010000           -fst dword ptr [edi + 0x1e4]
    app->getMemory<float>(cpu.edi + x86::reg32(484) /* 0x1e4 */) = float(fp0);
    // 00480053  8d5610                 -lea edx, [esi + 0x10]
    cpu.edx = x86::reg32(cpu.esi + x86::reg32(16) /* 0x10 */);
    // 00480056  d99fc4010000           -fstp dword ptr [edi + 0x1c4]
    app->getMemory<float>(cpu.edi + x86::reg32(452) /* 0x1c4 */) = float(fp0);
    // 0048005c  e85fbd0100             -call 0x49bdc0
    cpu.esp -= 4;
    nfs3hp::port::text::unqueueQuad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00480061  8d4617                 -lea eax, [esi + 0x17]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(23) /* 0x17 */);
    // 00480064  8d4e16                 -lea ecx, [esi + 0x16]
    cpu.ecx = x86::reg32(cpu.esi + x86::reg32(22) /* 0x16 */);
    // 00480067  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00480068  8d5e15                 -lea ebx, [esi + 0x15]
    cpu.ebx = x86::reg32(cpu.esi + x86::reg32(21) /* 0x15 */);
    // 0048006b  b848027000             -mov eax, 0x700248
    cpu.eax = 7340616 /*0x700248*/;
    // 00480070  8d5614                 -lea edx, [esi + 0x14]
    cpu.edx = x86::reg32(cpu.esi + x86::reg32(20) /* 0x14 */);
    // 00480073  e848bd0100             -call 0x49bdc0
    cpu.esp -= 4;
    nfs3hp::port::text::unqueueQuad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00480078  8d461b                 -lea eax, [esi + 0x1b]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(27) /* 0x1b */);
    // 0048007b  8d4e1a                 -lea ecx, [esi + 0x1a]
    cpu.ecx = x86::reg32(cpu.esi + x86::reg32(26) /* 0x1a */);
    // 0048007e  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0048007f  8d5e19                 -lea ebx, [esi + 0x19]
    cpu.ebx = x86::reg32(cpu.esi + x86::reg32(25) /* 0x19 */);
    // 00480082  b848027000             -mov eax, 0x700248
    cpu.eax = 7340616 /*0x700248*/;
    // 00480087  8d5618                 -lea edx, [esi + 0x18]
    cpu.edx = x86::reg32(cpu.esi + x86::reg32(24) /* 0x18 */);
    // 0048008a  e831bd0100             -call 0x49bdc0
    cpu.esp -= 4;
    nfs3hp::port::text::unqueueQuad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0048008f:
    // 0048008f  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00480091  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00480092  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00480093  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00480094  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00480095  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00480096  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00480097  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4800a0: native, nfs3hp::render::hudBand */

/* align: skip 0x90 */
void Application::sub_4808b0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004808b0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004808b1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004808b2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004808b3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004808b4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004808b5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004808b6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004808b8  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 004808ba  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004808bd  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004808bf  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 004808c2  29f0                   -sub eax, esi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.esi));
    // 004808c4  8b35881a7900           -mov esi, dword ptr [0x791a88]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7936648) /* 0x791a88 */);
    // 004808ca  01c6                   -add esi, eax
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.eax));
L_0x004808cc:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004808cc  8d04bd00000000         -lea eax, [edi*4]
    cpu.eax = x86::reg32(cpu.edi * 4);
    // 004808d3  8b9040027000           -mov edx, dword ptr [eax + 0x700240]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(7340608) /* 0x700240 */);
    // 004808d9  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004808db  7424                   -je 0x480901
    if (cpu.flags.zf)
    {
        goto L_0x00480901;
    }
    // 004808dd  8d4aff                 -lea ecx, [edx - 1]
    cpu.ecx = x86::reg32(cpu.edx + x86::reg32(-1) /* -0x1 */);
    // 004808e0  898840027000           -mov dword ptr [eax + 0x700240], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(7340608) /* 0x700240 */) = cpu.ecx;
    // 004808e6  8d4603                 -lea eax, [esi + 3]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(3) /* 0x3 */);
    // 004808e9  8d4e02                 -lea ecx, [esi + 2]
    cpu.ecx = x86::reg32(cpu.esi + x86::reg32(2) /* 0x2 */);
    // 004808ec  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004808ed  8d5e01                 -lea ebx, [esi + 1]
    cpu.ebx = x86::reg32(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 004808f0  b8389a7400             -mov eax, 0x749a38
    cpu.eax = 7641656 /*0x749a38*/;
    // 004808f5  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 004808f7  e8c4b40100             -call 0x49bdc0
    cpu.esp -= 4;
    nfs3hp::port::text::unqueueQuad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004808fc  83c604                 +add esi, 4
    {
        x86::reg32& tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004808ff  ebcb                   -jmp 0x4808cc
    goto L_0x004808cc;
L_0x00480901:
    // 00480901  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00480902  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00480903  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00480904  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00480905  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00480906  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00480907  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_480910: native, nfs3hp::render::hudRect */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_480a00(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1, fp2;
  x86::reg16 fsw = 0;
    // 00480a00  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00480a01  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00480a02  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00480a03  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00480a04  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00480a05  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00480a06  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00480a08  81ec98000000           -sub esp, 0x98
    (cpu.esp) -= x86::reg32(x86::sreg32(152 /*0x98*/));
    // 00480a0e  81ed82000000           -sub ebp, 0x82
    (cpu.ebp) -= x86::reg32(x86::sreg32(130 /*0x82*/));
    // 00480a14  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 00480a16  db05c8da7c00           -fild dword ptr [0x7cdac8]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(8182472) /* 0x7cdac8 */)));
    // 00480a1c  d9e8                   -fld1 
    fp1 = 1.0;
    // 00480a1e  def1                   -fdivrp st(1)
    fp0 = fp::div(fp1, x86::Float(fp0));
    // 00480a20  8b15b0d36f00           -mov edx, dword ptr [0x6fd3b0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
    // 00480a26  d95d6a                 -fstp dword ptr [ebp + 0x6a]
    app->getMemory<float>(cpu.ebp + x86::reg32(106) /* 0x6a */) = float(fp0);
    // 00480a29  83fa01                 +cmp edx, 1
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
    // 00480a2c  7504                   -jne 0x480a32
    if (!cpu.flags.zf)
    {
        goto L_0x00480a32;
    }
    // 00480a2e  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00480a30  eb02                   -jmp 0x480a34
    goto L_0x00480a34;
L_0x00480a32:
    // 00480a32  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x00480a34:
    // 00480a34  8d1c07                 -lea ebx, [edi + eax]
    cpu.ebx = x86::reg32(cpu.edi + cpu.eax * 1);
    // 00480a37  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 00480a39  e89221faff             -call 0x422bd0
    cpu.esp -= 4;
    nfs3hp::carstate::cameraTarget(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00480a3e  f6800002000020         +test byte ptr [eax + 0x200], 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(512) /* 0x200 */) & 32 /*0x20*/));
    // 00480a45  7407                   -je 0x480a4e
    if (cpu.flags.zf)
    {
        goto L_0x00480a4e;
    }
    // 00480a47  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00480a4c  eb02                   -jmp 0x480a50
    goto L_0x00480a50;
L_0x00480a4e:
    // 00480a4e  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x00480a50:
    // 00480a50  894566                 -mov dword ptr [ebp + 0x66], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(102) /* 0x66 */) = cpu.eax;
    // 00480a53  31c9                   +xor ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx))));
    // 00480a55  e9d2030000             -jmp 0x480e2c
    goto L_0x00480e2c;
L_0x00480a5a:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00480a5a  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x00480a5c:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00480a5c  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
    // 00480a5f  db457e                 -fild dword ptr [ebp + 0x7e]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */)));
    // 00480a62  d95d2e                 -fstp dword ptr [ebp + 0x2e]
    app->getMemory<float>(cpu.ebp + x86::reg32(46) /* 0x2e */) = float(fp0);
    // 00480a65  f7452effffff7f         +test dword ptr [ebp + 0x2e], 0x7fffffff
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(46) /* 0x2e */) & 2147483647 /*0x7fffffff*/));
    // 00480a6c  7427                   -je 0x480a95
    if (cpu.flags.zf)
    {
        goto L_0x00480a95;
    }
    // 00480a6e  69d348030000           -imul edx, ebx, 0x348
    cpu.edx = x86::reg32(x86::sreg64(x86::sreg32(cpu.ebx)) * x86::sreg64(x86::sreg32(840 /*0x348*/)));
    // 00480a74  694566a4010000         -imul eax, dword ptr [ebp + 0x66], 0x1a4
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(102) /* 0x66 */))) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 00480a7b  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00480a7d  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00480a7f  c1e004                 +shl eax, 4
    {
        x86::reg8 tmp = 4 /*0x4*/ % 32;
        x86::reg32& op = cpu.eax;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 00480a82  d9e8                   +fld1 
    fp0 = 1.0;
    // 00480a84  d8a40280bc6f00         +fsub dword ptr [edx + eax + 0x6fbc80]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(7322752) /* 0x6fbc80 */ + cpu.eax * 1)));
    // 00480a8b  d95d6e                 +fstp dword ptr [ebp + 0x6e]
    app->getMemory<float>(cpu.ebp + x86::reg32(110) /* 0x6e */) = float(fp0);
    // 00480a8e  ba02000000             -mov edx, 2
    cpu.edx = 2 /*0x2*/;
    // 00480a93  eb35                   -jmp 0x480aca
    goto L_0x00480aca;
L_0x00480a95:
    // 00480a95  d9452e                 +fld dword ptr [ebp + 0x2e]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(46) /* 0x2e */));
    // 00480a98  d8656e                 +fsub dword ptr [ebp + 0x6e]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(110) /* 0x6e */)));
    // 00480a9b  d9ee                   +fldz 
    fp1 = 0.0;
    // 00480a9d  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 00480a9f  d95d3e                 +fstp dword ptr [ebp + 0x3e]
    app->getMemory<float>(cpu.ebp + x86::reg32(62) /* 0x3e */) = float(fp1);
    // 00480aa2  d85d3e                 +fcomp dword ptr [ebp + 0x3e]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(62) /* 0x3e */)));
    // 00480aa5  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00480aa7  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00480aa8  7308                   -jae 0x480ab2
    if (!cpu.flags.cf)
    {
        goto L_0x00480ab2;
    }
    // 00480aaa  8b453e                 -mov eax, dword ptr [ebp + 0x3e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(62) /* 0x3e */);
    // 00480aad  89452a                 -mov dword ptr [ebp + 0x2a], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(42) /* 0x2a */) = cpu.eax;
    // 00480ab0  eb08                   -jmp 0x480aba
    goto L_0x00480aba;
L_0x00480ab2:
    // 00480ab2  d9453e                 +fld dword ptr [ebp + 0x3e]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(62) /* 0x3e */));
    // 00480ab5  d9e0                   +fchs 
    fp0 = -fp0;
    // 00480ab7  d95d2a                 +fstp dword ptr [ebp + 0x2a]
    app->getMemory<float>(cpu.ebp + x86::reg32(42) /* 0x2a */) = float(fp0);
L_0x00480aba:
    // 00480aba  d9452a                 +fld dword ptr [ebp + 0x2a]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(42) /* 0x2a */));
    // 00480abd  d85d6a                 +fcomp dword ptr [ebp + 0x6a]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(106) /* 0x6a */)));
    // 00480ac0  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00480ac2  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00480ac3  7305                   -jae 0x480aca
    if (!cpu.flags.cf)
    {
        goto L_0x00480aca;
    }
    // 00480ac5  ba04000000             -mov edx, 4
    cpu.edx = 4 /*0x4*/;
L_0x00480aca:
    // 00480aca  31f6                   +xor esi, esi
    cpu.clear_co();
    cpu.set_szp((cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi))));
    // 00480acc  e97f010000             -jmp 0x480c50
    goto L_0x00480c50;
L_0x00480ad1:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00480ad1  69c348030000           -imul eax, ebx, 0x348
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.ebx)) * x86::sreg64(x86::sreg32(840 /*0x348*/)));
    // 00480ad7  8945ea                 -mov dword ptr [ebp - 0x16], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-22) /* -0x16 */) = cpu.eax;
    // 00480ada  694566a4010000         -imul eax, dword ptr [ebp + 0x66], 0x1a4
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(102) /* 0x66 */))) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 00480ae1  894572                 -mov dword ptr [ebp + 0x72], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(114) /* 0x72 */) = cpu.eax;
    // 00480ae4  8b45ea                 -mov eax, dword ptr [ebp - 0x16]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-22) /* -0x16 */);
    // 00480ae7  034572                 -add eax, dword ptr [ebp + 0x72]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(114) /* 0x72 */)));
    // 00480aea  894576                 -mov dword ptr [ebp + 0x76], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */) = cpu.eax;
    // 00480aed  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00480aef  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 00480af2  89450a                 -mov dword ptr [ebp + 0xa], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(10) /* 0xa */) = cpu.eax;
    // 00480af5  8b4576                 -mov eax, dword ptr [ebp + 0x76]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */);
    // 00480af8  03450a                 -add eax, dword ptr [ebp + 0xa]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(10) /* 0xa */)));
    // 00480afb  89454a                 -mov dword ptr [ebp + 0x4a], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(74) /* 0x4a */) = cpu.eax;
    // 00480afe  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00480b00  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 00480b03  89450e                 -mov dword ptr [ebp + 0xe], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(14) /* 0xe */) = cpu.eax;
    // 00480b06  8b4576                 -mov eax, dword ptr [ebp + 0x76]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */);
    // 00480b09  03450e                 +add eax, dword ptr [ebp + 0xe]
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(14) /* 0xe */)));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00480b0c  89455e                 -mov dword ptr [ebp + 0x5e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(94) /* 0x5e */) = cpu.eax;
    // 00480b0f  8b454a                 -mov eax, dword ptr [ebp + 0x4a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(74) /* 0x4a */);
    // 00480b12  d9807cbc6f00           +fld dword ptr [eax + 0x6fbc7c]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(7322748) /* 0x6fbc7c */));
    // 00480b18  8b455e                 -mov eax, dword ptr [ebp + 0x5e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(94) /* 0x5e */);
    // 00480b1b  d89874bc6f00           +fcomp dword ptr [eax + 0x6fbc74]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(7322740) /* 0x6fbc74 */)));
    // 00480b21  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00480b23  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00480b24  0f8220010000           -jb 0x480c4a
    if (cpu.flags.cf)
    {
        goto L_0x00480c4a;
    }
    // 00480b2a  8b454a                 -mov eax, dword ptr [ebp + 0x4a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(74) /* 0x4a */);
    // 00480b2d  d98074bc6f00           +fld dword ptr [eax + 0x6fbc74]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(7322740) /* 0x6fbc74 */));
    // 00480b33  8b455e                 -mov eax, dword ptr [ebp + 0x5e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(94) /* 0x5e */);
    // 00480b36  d8987cbc6f00           +fcomp dword ptr [eax + 0x6fbc7c]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(7322748) /* 0x6fbc7c */)));
    // 00480b3c  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00480b3e  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00480b3f  0f8705010000           -ja 0x480c4a
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x00480c4a;
    }
    // 00480b45  d9456e                 +fld dword ptr [ebp + 0x6e]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(110) /* 0x6e */));
    // 00480b48  8b454a                 -mov eax, dword ptr [ebp + 0x4a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(74) /* 0x4a */);
    // 00480b4b  d9ee                   +fldz 
    fp1 = 0.0;
    // 00480b4d  d98078bc6f00           +fld dword ptr [eax + 0x6fbc78]
    fp2 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(7322744) /* 0x6fbc78 */));
    // 00480b53  8b455e                 -mov eax, dword ptr [ebp + 0x5e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(94) /* 0x5e */);
    // 00480b56  d9ca                   +fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 00480b58  d8656a                 +fsub dword ptr [ebp + 0x6a]
    fp2 = fp::sub(fp2, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(106) /* 0x6a */)));
    // 00480b5b  d9ca                   +fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 00480b5d  d8a080bc6f00           +fsub dword ptr [eax + 0x6fbc80]
    fp2 = fp::sub(fp2, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(7322752) /* 0x6fbc80 */)));
    // 00480b63  d9ca                   +fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 00480b65  d95d16                 +fstp dword ptr [ebp + 0x16]
    app->getMemory<float>(cpu.ebp + x86::reg32(22) /* 0x16 */) = float(fp2);
    // 00480b68  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 00480b6a  d95d62                 +fstp dword ptr [ebp + 0x62]
    app->getMemory<float>(cpu.ebp + x86::reg32(98) /* 0x62 */) = float(fp1);
    // 00480b6d  d85d62                 +fcomp dword ptr [ebp + 0x62]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(98) /* 0x62 */)));
    // 00480b70  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00480b72  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00480b73  7762                   -ja 0x480bd7
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x00480bd7;
    }
    // 00480b75  d94562                 +fld dword ptr [ebp + 0x62]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(98) /* 0x62 */));
    // 00480b78  d85d16                 +fcomp dword ptr [ebp + 0x16]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(22) /* 0x16 */)));
    // 00480b7b  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00480b7d  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00480b7e  7710                   -ja 0x480b90
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x00480b90;
    }
    // 00480b80  8b4562                 -mov eax, dword ptr [ebp + 0x62]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(98) /* 0x62 */);
    // 00480b83  ba02000000             -mov edx, 2
    cpu.edx = 2 /*0x2*/;
    // 00480b88  89456e                 -mov dword ptr [ebp + 0x6e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(110) /* 0x6e */) = cpu.eax;
    // 00480b8b  e9ba000000             -jmp 0x480c4a
    goto L_0x00480c4a;
L_0x00480b90:
    // 00480b90  d94562                 +fld dword ptr [ebp + 0x62]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(98) /* 0x62 */));
    // 00480b93  d8656e                 +fsub dword ptr [ebp + 0x6e]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(110) /* 0x6e */)));
    // 00480b96  d9ee                   +fldz 
    fp1 = 0.0;
    // 00480b98  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 00480b9a  d95d3a                 +fstp dword ptr [ebp + 0x3a]
    app->getMemory<float>(cpu.ebp + x86::reg32(58) /* 0x3a */) = float(fp1);
    // 00480b9d  d85d3a                 +fcomp dword ptr [ebp + 0x3a]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(58) /* 0x3a */)));
    // 00480ba0  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00480ba2  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00480ba3  7308                   -jae 0x480bad
    if (!cpu.flags.cf)
    {
        goto L_0x00480bad;
    }
    // 00480ba5  8b453a                 -mov eax, dword ptr [ebp + 0x3a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(58) /* 0x3a */);
    // 00480ba8  89451e                 -mov dword ptr [ebp + 0x1e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(30) /* 0x1e */) = cpu.eax;
    // 00480bab  eb08                   -jmp 0x480bb5
    goto L_0x00480bb5;
L_0x00480bad:
    // 00480bad  d9453a                 +fld dword ptr [ebp + 0x3a]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(58) /* 0x3a */));
    // 00480bb0  d9e0                   +fchs 
    fp0 = -fp0;
    // 00480bb2  d95d1e                 +fstp dword ptr [ebp + 0x1e]
    app->getMemory<float>(cpu.ebp + x86::reg32(30) /* 0x1e */) = float(fp0);
L_0x00480bb5:
    // 00480bb5  d9451e                 +fld dword ptr [ebp + 0x1e]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(30) /* 0x1e */));
    // 00480bb8  d85d6a                 +fcomp dword ptr [ebp + 0x6a]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(106) /* 0x6a */)));
    // 00480bbb  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00480bbd  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00480bbe  0f8386000000           -jae 0x480c4a
    if (!cpu.flags.cf)
    {
        goto L_0x00480c4a;
    }
    // 00480bc4  83fa01                 +cmp edx, 1
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
    // 00480bc7  0f857d000000           -jne 0x480c4a
    if (!cpu.flags.zf)
    {
        goto L_0x00480c4a;
    }
    // 00480bcd  ba04000000             -mov edx, 4
    cpu.edx = 4 /*0x4*/;
    // 00480bd2  e973000000             -jmp 0x480c4a
    goto L_0x00480c4a;
L_0x00480bd7:
    // 00480bd7  8b455e                 -mov eax, dword ptr [ebp + 0x5e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(94) /* 0x5e */);
    // 00480bda  d98078bc6f00           +fld dword ptr [eax + 0x6fbc78]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(7322744) /* 0x6fbc78 */));
    // 00480be0  8b454a                 -mov eax, dword ptr [ebp + 0x4a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(74) /* 0x4a */);
    // 00480be3  d8a080bc6f00           +fsub dword ptr [eax + 0x6fbc80]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(7322752) /* 0x6fbc80 */)));
    // 00480be9  d9ee                   +fldz 
    fp1 = 0.0;
    // 00480beb  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 00480bed  d95d52                 +fstp dword ptr [ebp + 0x52]
    app->getMemory<float>(cpu.ebp + x86::reg32(82) /* 0x52 */) = float(fp1);
    // 00480bf0  d85d52                 +fcomp dword ptr [ebp + 0x52]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(82) /* 0x52 */)));
    // 00480bf3  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00480bf5  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00480bf6  7752                   -ja 0x480c4a
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x00480c4a;
    }
    // 00480bf8  d94552                 +fld dword ptr [ebp + 0x52]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(82) /* 0x52 */));
    // 00480bfb  d85d16                 +fcomp dword ptr [ebp + 0x16]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(22) /* 0x16 */)));
    // 00480bfe  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00480c00  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00480c01  770d                   -ja 0x480c10
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x00480c10;
    }
    // 00480c03  8b4552                 -mov eax, dword ptr [ebp + 0x52]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(82) /* 0x52 */);
    // 00480c06  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 00480c0b  89456e                 -mov dword ptr [ebp + 0x6e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(110) /* 0x6e */) = cpu.eax;
    // 00480c0e  eb3a                   -jmp 0x480c4a
    goto L_0x00480c4a;
L_0x00480c10:
    // 00480c10  d94552                 +fld dword ptr [ebp + 0x52]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(82) /* 0x52 */));
    // 00480c13  d8656e                 +fsub dword ptr [ebp + 0x6e]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(110) /* 0x6e */)));
    // 00480c16  d9ee                   +fldz 
    fp1 = 0.0;
    // 00480c18  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 00480c1a  d95d32                 +fstp dword ptr [ebp + 0x32]
    app->getMemory<float>(cpu.ebp + x86::reg32(50) /* 0x32 */) = float(fp1);
    // 00480c1d  d85d32                 +fcomp dword ptr [ebp + 0x32]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(50) /* 0x32 */)));
    // 00480c20  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00480c22  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00480c23  7308                   -jae 0x480c2d
    if (!cpu.flags.cf)
    {
        goto L_0x00480c2d;
    }
    // 00480c25  8b4532                 -mov eax, dword ptr [ebp + 0x32]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(50) /* 0x32 */);
    // 00480c28  894512                 -mov dword ptr [ebp + 0x12], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(18) /* 0x12 */) = cpu.eax;
    // 00480c2b  eb08                   -jmp 0x480c35
    goto L_0x00480c35;
L_0x00480c2d:
    // 00480c2d  d94532                 +fld dword ptr [ebp + 0x32]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(50) /* 0x32 */));
    // 00480c30  d9e0                   +fchs 
    fp0 = -fp0;
    // 00480c32  d95d12                 +fstp dword ptr [ebp + 0x12]
    app->getMemory<float>(cpu.ebp + x86::reg32(18) /* 0x12 */) = float(fp0);
L_0x00480c35:
    // 00480c35  d94512                 +fld dword ptr [ebp + 0x12]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(18) /* 0x12 */));
    // 00480c38  d85d6a                 +fcomp dword ptr [ebp + 0x6a]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(106) /* 0x6a */)));
    // 00480c3b  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00480c3d  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00480c3e  730a                   -jae 0x480c4a
    if (!cpu.flags.cf)
    {
        goto L_0x00480c4a;
    }
    // 00480c40  83fa02                 +cmp edx, 2
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
    // 00480c43  7505                   -jne 0x480c4a
    if (!cpu.flags.zf)
    {
        goto L_0x00480c4a;
    }
    // 00480c45  ba04000000             -mov edx, 4
    cpu.edx = 4 /*0x4*/;
L_0x00480c4a:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00480c4a  46                     -inc esi
    (cpu.esi)++;
    // 00480c4b  83fe17                 +cmp esi, 0x17
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(23 /*0x17*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00480c4e  7d15                   -jge 0x480c65
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00480c65;
    }
L_0x00480c50:
    // 00480c50  39ce                   +cmp esi, ecx
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ecx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00480c52  74f6                   -je 0x480c4a
    if (cpu.flags.zf)
    {
        goto L_0x00480c4a;
    }
    // 00480c54  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00480c56  e8c5afffff             -call 0x47bc20
    cpu.esp -= 4;
    sub_47bc20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00480c5b  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00480c5d  0f846efeffff           -je 0x480ad1
    if (cpu.flags.zf)
    {
        goto L_0x00480ad1;
    }
    // 00480c63  ebe5                   -jmp 0x480c4a
    goto L_0x00480c4a;
L_0x00480c65:
    // 00480c65  31f6                   +xor esi, esi
    cpu.clear_co();
    cpu.set_szp((cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi))));
    // 00480c67  e97f010000             -jmp 0x480deb
    goto L_0x00480deb;
L_0x00480c6c:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00480c6c  69c348030000           -imul eax, ebx, 0x348
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.ebx)) * x86::sreg64(x86::sreg32(840 /*0x348*/)));
    // 00480c72  8945ee                 -mov dword ptr [ebp - 0x12], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-18) /* -0x12 */) = cpu.eax;
    // 00480c75  694566a4010000         -imul eax, dword ptr [ebp + 0x66], 0x1a4
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(102) /* 0x66 */))) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 00480c7c  89457a                 -mov dword ptr [ebp + 0x7a], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */) = cpu.eax;
    // 00480c7f  8b45ee                 -mov eax, dword ptr [ebp - 0x12]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-18) /* -0x12 */);
    // 00480c82  03457a                 -add eax, dword ptr [ebp + 0x7a]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */)));
    // 00480c85  8945f2                 -mov dword ptr [ebp - 0xe], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-14) /* -0xe */) = cpu.eax;
    // 00480c88  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00480c8a  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 00480c8d  8945fe                 -mov dword ptr [ebp - 2], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-2) /* -0x2 */) = cpu.eax;
    // 00480c90  8b45f2                 -mov eax, dword ptr [ebp - 0xe]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-14) /* -0xe */);
    // 00480c93  0345fe                 -add eax, dword ptr [ebp - 2]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-2) /* -0x2 */)));
    // 00480c96  89454e                 -mov dword ptr [ebp + 0x4e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(78) /* 0x4e */) = cpu.eax;
    // 00480c99  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00480c9b  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 00480c9e  8945f6                 -mov dword ptr [ebp - 0xa], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-10) /* -0xa */) = cpu.eax;
    // 00480ca1  8b45f2                 -mov eax, dword ptr [ebp - 0xe]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-14) /* -0xe */);
    // 00480ca4  0345f6                 +add eax, dword ptr [ebp - 0xa]
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-10) /* -0xa */)));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00480ca7  894546                 -mov dword ptr [ebp + 0x46], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(70) /* 0x46 */) = cpu.eax;
    // 00480caa  8b454e                 -mov eax, dword ptr [ebp + 0x4e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(78) /* 0x4e */);
    // 00480cad  d9807cbc6f00           +fld dword ptr [eax + 0x6fbc7c]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(7322748) /* 0x6fbc7c */));
    // 00480cb3  8b4546                 -mov eax, dword ptr [ebp + 0x46]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(70) /* 0x46 */);
    // 00480cb6  d89874bc6f00           +fcomp dword ptr [eax + 0x6fbc74]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(7322740) /* 0x6fbc74 */)));
    // 00480cbc  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00480cbe  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00480cbf  0f8220010000           -jb 0x480de5
    if (cpu.flags.cf)
    {
        goto L_0x00480de5;
    }
    // 00480cc5  8b454e                 -mov eax, dword ptr [ebp + 0x4e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(78) /* 0x4e */);
    // 00480cc8  d98074bc6f00           +fld dword ptr [eax + 0x6fbc74]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(7322740) /* 0x6fbc74 */));
    // 00480cce  8b4546                 -mov eax, dword ptr [ebp + 0x46]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(70) /* 0x46 */);
    // 00480cd1  d8987cbc6f00           +fcomp dword ptr [eax + 0x6fbc7c]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(7322748) /* 0x6fbc7c */)));
    // 00480cd7  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00480cd9  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00480cda  0f8705010000           -ja 0x480de5
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x00480de5;
    }
    // 00480ce0  d9456e                 +fld dword ptr [ebp + 0x6e]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(110) /* 0x6e */));
    // 00480ce3  8b454e                 -mov eax, dword ptr [ebp + 0x4e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(78) /* 0x4e */);
    // 00480ce6  d9ee                   +fldz 
    fp1 = 0.0;
    // 00480ce8  d98078bc6f00           +fld dword ptr [eax + 0x6fbc78]
    fp2 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(7322744) /* 0x6fbc78 */));
    // 00480cee  8b4546                 -mov eax, dword ptr [ebp + 0x46]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(70) /* 0x46 */);
    // 00480cf1  d9ca                   +fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 00480cf3  d8656a                 +fsub dword ptr [ebp + 0x6a]
    fp2 = fp::sub(fp2, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(106) /* 0x6a */)));
    // 00480cf6  d9ca                   +fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 00480cf8  d8a080bc6f00           +fsub dword ptr [eax + 0x6fbc80]
    fp2 = fp::sub(fp2, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(7322752) /* 0x6fbc80 */)));
    // 00480cfe  d9ca                   +fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 00480d00  d95d26                 +fstp dword ptr [ebp + 0x26]
    app->getMemory<float>(cpu.ebp + x86::reg32(38) /* 0x26 */) = float(fp2);
    // 00480d03  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 00480d05  d95d5a                 +fstp dword ptr [ebp + 0x5a]
    app->getMemory<float>(cpu.ebp + x86::reg32(90) /* 0x5a */) = float(fp1);
    // 00480d08  d85d5a                 +fcomp dword ptr [ebp + 0x5a]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(90) /* 0x5a */)));
    // 00480d0b  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00480d0d  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00480d0e  7762                   -ja 0x480d72
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x00480d72;
    }
    // 00480d10  d9455a                 +fld dword ptr [ebp + 0x5a]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(90) /* 0x5a */));
    // 00480d13  d85d26                 +fcomp dword ptr [ebp + 0x26]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(38) /* 0x26 */)));
    // 00480d16  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00480d18  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00480d19  7710                   -ja 0x480d2b
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x00480d2b;
    }
    // 00480d1b  8b455a                 -mov eax, dword ptr [ebp + 0x5a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(90) /* 0x5a */);
    // 00480d1e  ba02000000             -mov edx, 2
    cpu.edx = 2 /*0x2*/;
    // 00480d23  89456e                 -mov dword ptr [ebp + 0x6e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(110) /* 0x6e */) = cpu.eax;
    // 00480d26  e9ba000000             -jmp 0x480de5
    goto L_0x00480de5;
L_0x00480d2b:
    // 00480d2b  d9455a                 +fld dword ptr [ebp + 0x5a]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(90) /* 0x5a */));
    // 00480d2e  d8656e                 +fsub dword ptr [ebp + 0x6e]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(110) /* 0x6e */)));
    // 00480d31  d9ee                   +fldz 
    fp1 = 0.0;
    // 00480d33  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 00480d35  d95d42                 +fstp dword ptr [ebp + 0x42]
    app->getMemory<float>(cpu.ebp + x86::reg32(66) /* 0x42 */) = float(fp1);
    // 00480d38  d85d42                 +fcomp dword ptr [ebp + 0x42]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(66) /* 0x42 */)));
    // 00480d3b  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00480d3d  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00480d3e  7308                   -jae 0x480d48
    if (!cpu.flags.cf)
    {
        goto L_0x00480d48;
    }
    // 00480d40  8b4542                 -mov eax, dword ptr [ebp + 0x42]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(66) /* 0x42 */);
    // 00480d43  894522                 -mov dword ptr [ebp + 0x22], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(34) /* 0x22 */) = cpu.eax;
    // 00480d46  eb08                   -jmp 0x480d50
    goto L_0x00480d50;
L_0x00480d48:
    // 00480d48  d94542                 +fld dword ptr [ebp + 0x42]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(66) /* 0x42 */));
    // 00480d4b  d9e0                   +fchs 
    fp0 = -fp0;
    // 00480d4d  d95d22                 +fstp dword ptr [ebp + 0x22]
    app->getMemory<float>(cpu.ebp + x86::reg32(34) /* 0x22 */) = float(fp0);
L_0x00480d50:
    // 00480d50  d94522                 +fld dword ptr [ebp + 0x22]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(34) /* 0x22 */));
    // 00480d53  d85d6a                 +fcomp dword ptr [ebp + 0x6a]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(106) /* 0x6a */)));
    // 00480d56  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00480d58  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00480d59  0f8386000000           -jae 0x480de5
    if (!cpu.flags.cf)
    {
        goto L_0x00480de5;
    }
    // 00480d5f  83fa01                 +cmp edx, 1
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
    // 00480d62  0f857d000000           -jne 0x480de5
    if (!cpu.flags.zf)
    {
        goto L_0x00480de5;
    }
    // 00480d68  ba04000000             -mov edx, 4
    cpu.edx = 4 /*0x4*/;
    // 00480d6d  e973000000             -jmp 0x480de5
    goto L_0x00480de5;
L_0x00480d72:
    // 00480d72  8b4546                 -mov eax, dword ptr [ebp + 0x46]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(70) /* 0x46 */);
    // 00480d75  d98078bc6f00           +fld dword ptr [eax + 0x6fbc78]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(7322744) /* 0x6fbc78 */));
    // 00480d7b  8b454e                 -mov eax, dword ptr [ebp + 0x4e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(78) /* 0x4e */);
    // 00480d7e  d8a080bc6f00           +fsub dword ptr [eax + 0x6fbc80]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(7322752) /* 0x6fbc80 */)));
    // 00480d84  d9ee                   +fldz 
    fp1 = 0.0;
    // 00480d86  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 00480d88  d95d56                 +fstp dword ptr [ebp + 0x56]
    app->getMemory<float>(cpu.ebp + x86::reg32(86) /* 0x56 */) = float(fp1);
    // 00480d8b  d85d56                 +fcomp dword ptr [ebp + 0x56]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(86) /* 0x56 */)));
    // 00480d8e  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00480d90  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00480d91  7752                   -ja 0x480de5
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x00480de5;
    }
    // 00480d93  d94556                 +fld dword ptr [ebp + 0x56]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(86) /* 0x56 */));
    // 00480d96  d85d26                 +fcomp dword ptr [ebp + 0x26]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(38) /* 0x26 */)));
    // 00480d99  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00480d9b  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00480d9c  770d                   -ja 0x480dab
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x00480dab;
    }
    // 00480d9e  8b4556                 -mov eax, dword ptr [ebp + 0x56]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(86) /* 0x56 */);
    // 00480da1  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 00480da6  89456e                 -mov dword ptr [ebp + 0x6e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(110) /* 0x6e */) = cpu.eax;
    // 00480da9  eb3a                   -jmp 0x480de5
    goto L_0x00480de5;
L_0x00480dab:
    // 00480dab  d94556                 +fld dword ptr [ebp + 0x56]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(86) /* 0x56 */));
    // 00480dae  d8656e                 +fsub dword ptr [ebp + 0x6e]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(110) /* 0x6e */)));
    // 00480db1  d9ee                   +fldz 
    fp1 = 0.0;
    // 00480db3  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 00480db5  d95d36                 +fstp dword ptr [ebp + 0x36]
    app->getMemory<float>(cpu.ebp + x86::reg32(54) /* 0x36 */) = float(fp1);
    // 00480db8  d85d36                 +fcomp dword ptr [ebp + 0x36]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(54) /* 0x36 */)));
    // 00480dbb  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00480dbd  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00480dbe  7308                   -jae 0x480dc8
    if (!cpu.flags.cf)
    {
        goto L_0x00480dc8;
    }
    // 00480dc0  8b4536                 -mov eax, dword ptr [ebp + 0x36]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(54) /* 0x36 */);
    // 00480dc3  89451a                 -mov dword ptr [ebp + 0x1a], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(26) /* 0x1a */) = cpu.eax;
    // 00480dc6  eb08                   -jmp 0x480dd0
    goto L_0x00480dd0;
L_0x00480dc8:
    // 00480dc8  d94536                 +fld dword ptr [ebp + 0x36]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(54) /* 0x36 */));
    // 00480dcb  d9e0                   +fchs 
    fp0 = -fp0;
    // 00480dcd  d95d1a                 +fstp dword ptr [ebp + 0x1a]
    app->getMemory<float>(cpu.ebp + x86::reg32(26) /* 0x1a */) = float(fp0);
L_0x00480dd0:
    // 00480dd0  d9451a                 +fld dword ptr [ebp + 0x1a]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(26) /* 0x1a */));
    // 00480dd3  d85d6a                 +fcomp dword ptr [ebp + 0x6a]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(106) /* 0x6a */)));
    // 00480dd6  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00480dd8  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00480dd9  730a                   -jae 0x480de5
    if (!cpu.flags.cf)
    {
        goto L_0x00480de5;
    }
    // 00480ddb  83fa02                 +cmp edx, 2
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
    // 00480dde  7505                   -jne 0x480de5
    if (!cpu.flags.zf)
    {
        goto L_0x00480de5;
    }
    // 00480de0  ba04000000             -mov edx, 4
    cpu.edx = 4 /*0x4*/;
L_0x00480de5:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00480de5  46                     -inc esi
    (cpu.esi)++;
    // 00480de6  83fe17                 +cmp esi, 0x17
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(23 /*0x17*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00480de9  7d15                   -jge 0x480e00
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00480e00;
    }
L_0x00480deb:
    // 00480deb  39ce                   +cmp esi, ecx
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ecx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00480ded  74f6                   -je 0x480de5
    if (cpu.flags.zf)
    {
        goto L_0x00480de5;
    }
    // 00480def  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00480df1  e82aaeffff             -call 0x47bc20
    cpu.esp -= 4;
    sub_47bc20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00480df6  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00480df8  0f856efeffff           -jne 0x480c6c
    if (!cpu.flags.zf)
    {
        goto L_0x00480c6c;
    }
    // 00480dfe  ebe5                   -jmp 0x480de5
    goto L_0x00480de5;
L_0x00480e00:
    // 00480e00  8d04bd00000000         -lea eax, [edi*4]
    cpu.eax = x86::reg32(cpu.edi * 4);
    // 00480e07  29f8                   -sub eax, edi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edi));
    // 00480e09  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 00480e0c  29f8                   -sub eax, edi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edi));
    // 00480e0e  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00480e11  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
    // 00480e14  8b757e                 -mov esi, dword ptr [ebp + 0x7e]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 00480e17  8d048d00000000         -lea eax, [ecx*4]
    cpu.eax = x86::reg32(cpu.ecx * 4);
    // 00480e1e  01f0                   -add eax, esi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.esi));
    // 00480e20  899090567200           -mov dword ptr [eax + 0x725690], edx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(7493264) /* 0x725690 */) = 8; /* port: an element's text stays centred in it, was the edge it stood nearest */
L_0x00480e26:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00480e26  41                     -inc ecx
    (cpu.ecx)++;
    // 00480e27  83f917                 +cmp ecx, 0x17
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(23 /*0x17*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00480e2a  7d4f                   -jge 0x480e7b
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00480e7b;
    }
L_0x00480e2c:
    // 00480e2c  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00480e2e  e8edadffff             -call 0x47bc20
    cpu.esp -= 4;
    sub_47bc20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00480e33  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00480e35  74ef                   -je 0x480e26
    if (cpu.flags.zf)
    {
        goto L_0x00480e26;
    }
    // 00480e37  69d348030000           -imul edx, ebx, 0x348
    cpu.edx = x86::reg32(x86::sreg64(x86::sreg32(cpu.ebx)) * x86::sreg64(x86::sreg32(840 /*0x348*/)));
    // 00480e3d  694566a4010000         -imul eax, dword ptr [ebp + 0x66], 0x1a4
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(102) /* 0x66 */))) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 00480e44  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00480e46  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00480e48  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 00480e4b  01d0                   +add eax, edx
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
    // 00480e4d  d9e8                   +fld1 
    fp0 = 1.0;
    // 00480e4f  8b9078bc6f00           -mov edx, dword ptr [eax + 0x6fbc78]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(7322744) /* 0x6fbc78 */);
    // 00480e55  d8a080bc6f00           +fsub dword ptr [eax + 0x6fbc80]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(7322752) /* 0x6fbc80 */)));
    // 00480e5b  89556e                 -mov dword ptr [ebp + 0x6e], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(110) /* 0x6e */) = cpu.edx;
    // 00480e5e  d9456e                 +fld dword ptr [ebp + 0x6e]
    fp1 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(110) /* 0x6e */));
    // 00480e61  d8656a                 +fsub dword ptr [ebp + 0x6a]
    fp1 = fp::sub(fp1, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(106) /* 0x6a */)));
    // 00480e64  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 00480e69  ded9                   +fcompp 
    fsw = fp::compare(fp1, fp0);
    // 00480e6b  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00480e6d  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00480e6e  0f82e6fbffff           -jb 0x480a5a
    if (cpu.flags.cf)
    {
        goto L_0x00480a5a;
    }
    // 00480e74  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00480e76  e9e1fbffff             -jmp 0x480a5c
    goto L_0x00480a5c;
L_0x00480e7b:
    // 00480e7b  b90d000000             -mov ecx, 0xd
    cpu.ecx = 13 /*0xd*/;
L_0x00480e80:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00480e80  8d14bd00000000         -lea edx, [edi*4]
    cpu.edx = x86::reg32(cpu.edi * 4);
    // 00480e87  29fa                   -sub edx, edi
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.edi));
    // 00480e89  c1e203                 -shl edx, 3
    cpu.edx <<= 3 /*0x3*/ % 32;
    // 00480e8c  29fa                   -sub edx, edi
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.edi));
    // 00480e8e  c1e202                 -shl edx, 2
    cpu.edx <<= 2 /*0x2*/ % 32;
    // 00480e91  8955fa                 -mov dword ptr [ebp - 6], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-6) /* -0x6 */) = cpu.edx;
    // 00480e94  8b5566                 -mov edx, dword ptr [ebp + 0x66]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(102) /* 0x66 */);
    // 00480e97  69c2a4010000           -imul eax, edx, 0x1a4
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.edx)) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 00480e9d  894506                 -mov dword ptr [ebp + 6], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(6) /* 0x6 */) = cpu.eax;
    // 00480ea0  69c348030000           -imul eax, ebx, 0x348
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.ebx)) * x86::sreg64(x86::sreg32(840 /*0x348*/)));
    // 00480ea6  8b75fa                 -mov esi, dword ptr [ebp - 6]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-6) /* -0x6 */);
    // 00480ea9  8d148d00000000         -lea edx, [ecx*4]
    cpu.edx = x86::reg32(cpu.ecx * 4);
    // 00480eb0  894502                 -mov dword ptr [ebp + 2], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(2) /* 0x2 */) = cpu.eax;
    // 00480eb3  01f2                   -add edx, esi
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.esi));
    // 00480eb5  8b7502                 -mov esi, dword ptr [ebp + 2]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(2) /* 0x2 */);
    // 00480eb8  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00480eba  037506                 -add esi, dword ptr [ebp + 6]
    (cpu.esi) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(6) /* 0x6 */)));
    // 00480ebd  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 00480ec0  01f0                   -add eax, esi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.esi));
    // 00480ec2  83fb01                 +cmp ebx, 1
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
    // 00480ec5  7210                   -jb 0x480ed7
    if (cpu.flags.cf)
    {
        goto L_0x00480ed7;
    }
    // 00480ec7  7647                   -jbe 0x480f10
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00480f10;
    }
    // 00480ec9  83fb02                 +cmp ebx, 2
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
    // 00480ecc  0f848b000000           -je 0x480f5d
    if (cpu.flags.zf)
    {
        goto L_0x00480f5d;
    }
    // 00480ed2  e9c4000000             -jmp 0x480f9b
    goto L_0x00480f9b;
L_0x00480ed7:
    // 00480ed7  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 00480ed9  0f85bc000000           -jne 0x480f9b
    if (!cpu.flags.zf)
    {
        goto L_0x00480f9b;
    }
    // 00480edf  d9e8                   +fld1 
    fp0 = 1.0;
    // 00480ee1  d8a07cbc6f00           +fsub dword ptr [eax + 0x6fbc7c]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(7322748) /* 0x6fbc7c */)));
    // 00480ee7  d89874bc6f00           +fcomp dword ptr [eax + 0x6fbc74]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(7322740) /* 0x6fbc74 */)));
    // 00480eed  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00480eef  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00480ef0  730f                   -jae 0x480f01
    if (!cpu.flags.cf)
    {
        goto L_0x00480f01;
    }
    // 00480ef2  c7829056720010000000   -mov dword ptr [edx + 0x725690], 0x10
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7493264) /* 0x725690 */) = 16 /*0x10*/;
    // 00480efc  e99a000000             -jmp 0x480f9b
    goto L_0x00480f9b;
L_0x00480f01:
    // 00480f01  c7829056720008000000   -mov dword ptr [edx + 0x725690], 8
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7493264) /* 0x725690 */) = 8 /*0x8*/;
    // 00480f0b  e98b000000             -jmp 0x480f9b
    goto L_0x00480f9b;
L_0x00480f10:
    // 00480f10  d90510b55300           +fld dword ptr [0x53b510]
    fp0 = x86::Float(app->getMemory<float>(x86::reg32(5485840) /* 0x53b510 */));
    // 00480f16  d8a07cbc6f00           +fsub dword ptr [eax + 0x6fbc7c]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(7322748) /* 0x6fbc7c */)));
    // 00480f1c  d89874bc6f00           +fcomp dword ptr [eax + 0x6fbc74]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(7322740) /* 0x6fbc74 */)));
    // 00480f22  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00480f24  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00480f25  731d                   -jae 0x480f44
    if (!cpu.flags.cf)
    {
        goto L_0x00480f44;
    }
    // 00480f27  c7829056720010000000   -mov dword ptr [edx + 0x725690], 0x10
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7493264) /* 0x725690 */) = 16 /*0x10*/;
    // 00480f31  83f90d                 +cmp ecx, 0xd
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(13 /*0xd*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00480f34  0f8570000000           -jne 0x480faa
    if (!cpu.flags.zf)
    {
        goto L_0x00480faa;
    }
    // 00480f3a  b914000000             -mov ecx, 0x14
    cpu.ecx = 20 /*0x14*/;
    // 00480f3f  e93cffffff             -jmp 0x480e80
    goto L_0x00480e80;
L_0x00480f44:
    // 00480f44  c7829056720008000000   -mov dword ptr [edx + 0x725690], 8
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7493264) /* 0x725690 */) = 8 /*0x8*/;
    // 00480f4e  83f90d                 +cmp ecx, 0xd
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(13 /*0xd*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00480f51  7557                   -jne 0x480faa
    if (!cpu.flags.zf)
    {
        goto L_0x00480faa;
    }
    // 00480f53  b914000000             -mov ecx, 0x14
    cpu.ecx = 20 /*0x14*/;
    // 00480f58  e923ffffff             -jmp 0x480e80
    goto L_0x00480e80;
L_0x00480f5d:
    // 00480f5d  d9e8                   +fld1 
    fp0 = 1.0;
    // 00480f5f  d8a07cbc6f00           +fsub dword ptr [eax + 0x6fbc7c]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(7322748) /* 0x6fbc7c */)));
    // 00480f65  d98074bc6f00           +fld dword ptr [eax + 0x6fbc74]
    fp1 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(7322740) /* 0x6fbc74 */));
    // 00480f6b  d80514b55300           +fadd dword ptr [0x53b514]
    fp1 = fp::add(fp1, x86::Float(app->getMemory<float>(x86::reg32(5485844) /* 0x53b514 */)));
    // 00480f71  ded9                   +fcompp 
    fsw = fp::compare(fp1, fp0);
    // 00480f73  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00480f75  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00480f76  7619                   -jbe 0x480f91
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00480f91;
    }
    // 00480f78  c7829056720010000000   -mov dword ptr [edx + 0x725690], 0x10
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7493264) /* 0x725690 */) = 16 /*0x10*/;
    // 00480f82  83f90d                 +cmp ecx, 0xd
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(13 /*0xd*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00480f85  7523                   -jne 0x480faa
    if (!cpu.flags.zf)
    {
        goto L_0x00480faa;
    }
    // 00480f87  b914000000             -mov ecx, 0x14
    cpu.ecx = 20 /*0x14*/;
    // 00480f8c  e9effeffff             -jmp 0x480e80
    goto L_0x00480e80;
L_0x00480f91:
    // 00480f91  c7829056720008000000   -mov dword ptr [edx + 0x725690], 8
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7493264) /* 0x725690 */) = 8 /*0x8*/;
L_0x00480f9b:
    // 00480f9b  83f90d                 +cmp ecx, 0xd
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(13 /*0xd*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00480f9e  750a                   -jne 0x480faa
    if (!cpu.flags.zf)
    {
        goto L_0x00480faa;
    }
    // 00480fa0  b914000000             -mov ecx, 0x14
    cpu.ecx = 20 /*0x14*/;
    // 00480fa5  e9d6feffff             -jmp 0x480e80
    goto L_0x00480e80;
L_0x00480faa:
    // 00480faa  8da582000000           -lea esp, [ebp + 0x82]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(130) /* 0x82 */);
    // 00480fb0  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00480fb1  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00480fb2  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00480fb3  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00480fb4  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00480fb5  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00480fb6  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_480fc0: native, nfs3hp::render::hudPicture */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_481170(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00481170  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00481171  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00481173  69d2a4010000           -imul edx, edx, 0x1a4
    cpu.edx = x86::reg32(x86::sreg64(x86::sreg32(cpu.edx)) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 00481179  69c048030000           -imul eax, eax, 0x348
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(840 /*0x348*/)));
    // 0048117f  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00481181  8b9068bc6f00           -mov edx, dword ptr [eax + 0x6fbc68]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(7322728) /* 0x6fbc68 */);
    // 00481187  83fa01                 +cmp edx, 1
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
    // 0048118a  7409                   -je 0x481195
    if (cpu.flags.zf)
    {
        goto L_0x00481195;
    }
    // 0048118c  83fa02                 +cmp edx, 2
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
    // 0048118f  0f857d000000           -jne 0x481212
    if (!cpu.flags.zf)
    {
        goto L_0x00481212;
    }
L_0x00481195:
    // 00481195  833dc8d46f0000         +cmp dword ptr [0x6fd4c8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328968) /* 0x6fd4c8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0048119c  743a                   -je 0x4811d8
    if (cpu.flags.zf)
    {
        goto L_0x004811d8;
    }
    // 0048119e  c7431c0000003b         -mov dword ptr [ebx + 0x1c], 0x3b000000
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(28) /* 0x1c */) = 989855744 /*0x3b000000*/;
    // 004811a5  c743380000fd3e         -mov dword ptr [ebx + 0x38], 0x3efd0000
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(56) /* 0x38 */) = 1056768000 /*0x3efd0000*/;
    // 004811ac  c7433c0000003b         -mov dword ptr [ebx + 0x3c], 0x3b000000
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(60) /* 0x3c */) = 989855744 /*0x3b000000*/;
    // 004811b3  c743580000fd3e         -mov dword ptr [ebx + 0x58], 0x3efd0000
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(88) /* 0x58 */) = 1056768000 /*0x3efd0000*/;
    // 004811ba  c7435c00007a3e         -mov dword ptr [ebx + 0x5c], 0x3e7a0000
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(92) /* 0x5c */) = 1048182784 /*0x3e7a0000*/;
    // 004811c1  c743780000813e         -mov dword ptr [ebx + 0x78], 0x3e810000
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(120) /* 0x78 */) = 1048641536 /*0x3e810000*/;
    // 004811c8  c7437c00007a3e         -mov dword ptr [ebx + 0x7c], 0x3e7a0000
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(124) /* 0x7c */) = 1048182784 /*0x3e7a0000*/;
    // 004811cf  c743180000813e         -mov dword ptr [ebx + 0x18], 0x3e810000
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(24) /* 0x18 */) = 1048641536 /*0x3e810000*/;
    // 004811d6  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004811d7  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004811d8:
    // 004811d8  c7431c0000003b         -mov dword ptr [ebx + 0x1c], 0x3b000000
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(28) /* 0x1c */) = 989855744 /*0x3b000000*/;
    // 004811df  c7433800007a3e         -mov dword ptr [ebx + 0x38], 0x3e7a0000
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(56) /* 0x38 */) = 1048182784 /*0x3e7a0000*/;
    // 004811e6  c7433c0000003b         -mov dword ptr [ebx + 0x3c], 0x3b000000
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(60) /* 0x3c */) = 989855744 /*0x3b000000*/;
    // 004811ed  c7435800007a3e         -mov dword ptr [ebx + 0x58], 0x3e7a0000
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(88) /* 0x58 */) = 1048182784 /*0x3e7a0000*/;
    // 004811f4  c7435c00007a3e         -mov dword ptr [ebx + 0x5c], 0x3e7a0000
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(92) /* 0x5c */) = 1048182784 /*0x3e7a0000*/;
    // 004811fb  c743780000003b         -mov dword ptr [ebx + 0x78], 0x3b000000
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(120) /* 0x78 */) = 989855744 /*0x3b000000*/;
    // 00481202  c7437c00007a3e         -mov dword ptr [ebx + 0x7c], 0x3e7a0000
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(124) /* 0x7c */) = 1048182784 /*0x3e7a0000*/;
    // 00481209  c743180000003b         -mov dword ptr [ebx + 0x18], 0x3b000000
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(24) /* 0x18 */) = 989855744 /*0x3b000000*/;
    // 00481210  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00481211  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00481212:
    // 00481212  833dc8d46f0000         +cmp dword ptr [0x6fd4c8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328968) /* 0x6fd4c8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00481219  743a                   -je 0x481255
    if (cpu.flags.zf)
    {
        goto L_0x00481255;
    }
    // 0048121b  c7431c0000813e         -mov dword ptr [ebx + 0x1c], 0x3e810000
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(28) /* 0x1c */) = 1048641536 /*0x3e810000*/;
    // 00481222  c743380000fd3e         -mov dword ptr [ebx + 0x38], 0x3efd0000
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(56) /* 0x38 */) = 1056768000 /*0x3efd0000*/;
    // 00481229  c7433c0000813e         -mov dword ptr [ebx + 0x3c], 0x3e810000
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(60) /* 0x3c */) = 1048641536 /*0x3e810000*/;
    // 00481230  c743580000fd3e         -mov dword ptr [ebx + 0x58], 0x3efd0000
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(88) /* 0x58 */) = 1056768000 /*0x3efd0000*/;
    // 00481237  c7435c0000fd3e         -mov dword ptr [ebx + 0x5c], 0x3efd0000
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(92) /* 0x5c */) = 1056768000 /*0x3efd0000*/;
    // 0048123e  c743780000813e         -mov dword ptr [ebx + 0x78], 0x3e810000
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(120) /* 0x78 */) = 1048641536 /*0x3e810000*/;
    // 00481245  c7437c0000fd3e         -mov dword ptr [ebx + 0x7c], 0x3efd0000
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(124) /* 0x7c */) = 1056768000 /*0x3efd0000*/;
    // 0048124c  c743180000813e         -mov dword ptr [ebx + 0x18], 0x3e810000
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(24) /* 0x18 */) = 1048641536 /*0x3e810000*/;
    // 00481253  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00481254  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00481255:
    // 00481255  c7431c0000813e         -mov dword ptr [ebx + 0x1c], 0x3e810000
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(28) /* 0x1c */) = 1048641536 /*0x3e810000*/;
    // 0048125c  c7433800007a3e         -mov dword ptr [ebx + 0x38], 0x3e7a0000
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(56) /* 0x38 */) = 1048182784 /*0x3e7a0000*/;
    // 00481263  c7433c0000813e         -mov dword ptr [ebx + 0x3c], 0x3e810000
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(60) /* 0x3c */) = 1048641536 /*0x3e810000*/;
    // 0048126a  c7435800007a3e         -mov dword ptr [ebx + 0x58], 0x3e7a0000
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(88) /* 0x58 */) = 1048182784 /*0x3e7a0000*/;
    // 00481271  c7435c0000fd3e         -mov dword ptr [ebx + 0x5c], 0x3efd0000
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(92) /* 0x5c */) = 1056768000 /*0x3efd0000*/;
    // 00481278  c743780000003b         -mov dword ptr [ebx + 0x78], 0x3b000000
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(120) /* 0x78 */) = 989855744 /*0x3b000000*/;
    // 0048127f  c7437c0000fd3e         -mov dword ptr [ebx + 0x7c], 0x3efd0000
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(124) /* 0x7c */) = 1056768000 /*0x3efd0000*/;
    // 00481286  c743180000003b         -mov dword ptr [ebx + 0x18], 0x3b000000
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(24) /* 0x18 */) = 989855744 /*0x3b000000*/;
    // 0048128d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0048128e  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x90 */
void Application::sub_481290(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00481290  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00481291  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00481292  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00481293  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00481294  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00481296  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00481298  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 0048129b  29c8                   -sub eax, ecx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 0048129d  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 004812a0  29c8                   -sub eax, ecx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004812a2  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 004812a5  8b9080987400           -mov edx, dword ptr [eax + 0x749880]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(7641216) /* 0x749880 */);
    // 004812ab  8bb084987400           -mov esi, dword ptr [eax + 0x749884]
    cpu.esi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(7641220) /* 0x749884 */);
    // 004812b1  8b9878987400           -mov ebx, dword ptr [eax + 0x749878]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(7641208) /* 0x749878 */);
    // 004812b7  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004812b8  8b907c987400           -mov edx, dword ptr [eax + 0x74987c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(7641212) /* 0x74987c */);
    // 004812be  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004812c0  8b0de41a7900           -mov ecx, dword ptr [0x791ae4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7936740) /* 0x791ae4 */);
    // 004812c6  c1e00a                 -shl eax, 0xa
    cpu.eax <<= 10 /*0xa*/ % 32;
    // 004812c9  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004812cb  89f1                   -mov ecx, esi
    cpu.ecx = cpu.esi;
    // 004812cd  e87ee3ffff             -call 0x47f650
    cpu.esp -= 4;
    nfs3hp::render::hudFrame(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004812d2  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004812d3  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004812d4  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004812d5  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004812d6  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4812e0: native, nfs3hp::render::hudDial */

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_481510(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0;
    // 00481510  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00481511  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00481512  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00481513  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00481515  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00481517  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00481519  a1e01a7900             -mov eax, dword ptr [0x791ae0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7936736) /* 0x791ae0 */);
    // 0048151e  c1e207                 -shl edx, 7
    cpu.edx <<= 7 /*0x7*/ % 32;
    // 00481521  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00481523  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 00481525  c1e202                 -shl edx, 2
    cpu.edx <<= 2 /*0x2*/ % 32;
    // 00481528  29ca                   -sub edx, ecx
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 0048152a  c1e203                 -shl edx, 3
    cpu.edx <<= 3 /*0x3*/ % 32;
    // 0048152d  29ca                   -sub edx, ecx
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 0048152f  c1e204                 -shl edx, 4
    cpu.edx <<= 4 /*0x4*/ % 32;
    // 00481532  db82a8987400           -fild dword ptr [edx + 0x7498a8]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7641256) /* 0x7498a8 */)));
    // 00481538  d95060                 -fst dword ptr [eax + 0x60]
    app->getMemory<float>(cpu.eax + x86::reg32(96) /* 0x60 */) = float(fp0);
    // 0048153b  d918                   -fstp dword ptr [eax]
    app->getMemory<float>(cpu.eax) = float(fp0);
    // 0048153d  db82b0987400           -fild dword ptr [edx + 0x7498b0]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7641264) /* 0x7498b0 */)));
    // 00481543  d95040                 -fst dword ptr [eax + 0x40]
    app->getMemory<float>(cpu.eax + x86::reg32(64) /* 0x40 */) = float(fp0);
    // 00481546  d95820                 -fstp dword ptr [eax + 0x20]
    app->getMemory<float>(cpu.eax + x86::reg32(32) /* 0x20 */) = float(fp0);
    // 00481549  db82ac987400           -fild dword ptr [edx + 0x7498ac]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7641260) /* 0x7498ac */)));
    // 0048154f  d95024                 -fst dword ptr [eax + 0x24]
    app->getMemory<float>(cpu.eax + x86::reg32(36) /* 0x24 */) = float(fp0);
    // 00481552  d95804                 -fstp dword ptr [eax + 4]
    app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */) = float(fp0);
    // 00481555  db82b4987400           -fild dword ptr [edx + 0x7498b4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7641268) /* 0x7498b4 */)));
    // 0048155b  d95064                 -fst dword ptr [eax + 0x64]
    app->getMemory<float>(cpu.eax + x86::reg32(100) /* 0x64 */) = float(fp0);
    // 0048155e  d95844                 -fstp dword ptr [eax + 0x44]
    app->getMemory<float>(cpu.eax + x86::reg32(68) /* 0x44 */) = float(fp0);
    // 00481561  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00481562  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00481563  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00481564  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_481570(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00481570  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00481571  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00481573  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00481574  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_4815d0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1, fp2, fp3, fp4, fp5;
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 004815d0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004815d1  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004815d2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004815d3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004815d5  83ec18                 -sub esp, 0x18
    (cpu.esp) -= x86::reg32(x86::sreg32(24 /*0x18*/));
    // 004815d8  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004815da  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004815dd  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004815df  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 004815e2  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004815e4  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 004815e7  db80d8977400           -fild dword ptr [eax + 0x7497d8]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(7641048) /* 0x7497d8 */)));
    // 004815ed  db80e0977400           -fild dword ptr [eax + 0x7497e0]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(7641056) /* 0x7497e0 */)));
    // 004815f3  db80dc977400           -fild dword ptr [eax + 0x7497dc]
    fp2 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(7641052) /* 0x7497dc */)));
    // 004815f9  db80e4977400           -fild dword ptr [eax + 0x7497e4]
    fp3 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(7641060) /* 0x7497e4 */)));
    // 004815ff  d9cb                   -fxch st(3)
    {
        x86::Float tmp = fp3;
        fp3 = fp0;
        fp0 = tmp;
    }
    // 00481601  d95df4                 -fstp dword ptr [ebp - 0xc]
    app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */) = float(fp3);
    // 00481604  d95df0                 -fstp dword ptr [ebp - 0x10]
    app->getMemory<float>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = float(fp2);
    // 00481607  d865f4                 -fsub dword ptr [ebp - 0xc]
    fp1 = fp::sub(fp1, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */)));
    // 0048160a  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 0048160c  d865f0                 -fsub dword ptr [ebp - 0x10]
    fp1 = fp::sub(fp1, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-16) /* -0x10 */)));
    // 0048160f  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 00481611  d95dfc                 -fstp dword ptr [ebp - 4]
    app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = float(fp1);
    // 00481614  d95df8                 -fstp dword ptr [ebp - 8]
    app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = float(fp0);
    // 00481617  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 00481619  740e                   -je 0x481629
    if (cpu.flags.zf)
    {
        goto L_0x00481629;
    }
    // 0048161b  e987020000             -jmp 0x4818a7
    goto L_0x004818a7;
L_0x00481620:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00481620  83fa15                 +cmp edx, 0x15
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(21 /*0x15*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00481623  0f8d7e020000           -jge 0x4818a7
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004818a7;
    }
L_0x00481629:
    // 00481629  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0048162b  c1e007                 -shl eax, 7
    cpu.eax <<= 7 /*0x7*/ % 32;
    // 0048162e  0590477200             -add eax, 0x724790
    (cpu.eax) += x86::reg32(x86::sreg32(7489424 /*0x724790*/));
    // 00481633  83fa14                 +cmp edx, 0x14
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(20 /*0x14*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00481636  0f8765020000           -ja 0x4818a1
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x004818a1;
    }
    // 0048163c  ff249578154800         -jmp dword ptr [edx*4 + 0x481578]
    cpu.ip = app->getMemory<x86::reg32>(4724088 + cpu.edx * 4); goto dynamic_jump;
  case 0x00481643:
    // 00481643  d945fc                 +fld dword ptr [ebp - 4]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
    // 00481646  d9c0                   +fld st(0)
    fp1 = x86::Float(fp0);
    // 00481648  d80d30b55300           +fmul dword ptr [0x53b530]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<float>(x86::reg32(5485872) /* 0x53b530 */)));
    // 0048164e  8b4df0                 -mov ecx, dword ptr [ebp - 0x10]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00481651  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 00481653  d80d34b55300           +fmul dword ptr [0x53b534]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<float>(x86::reg32(5485876) /* 0x53b534 */)));
    // 00481659  894804                 -mov dword ptr [eax + 4], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */) = cpu.ecx;
    // 0048165c  d94004                 +fld dword ptr [eax + 4]
    fp2 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 0048165f  d95024                 +fst dword ptr [eax + 0x24]
    app->getMemory<float>(cpu.eax + x86::reg32(36) /* 0x24 */) = float(fp2);
    // 00481662  d9ca                   +fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 00481664  d845f4                 +fadd dword ptr [ebp - 0xc]
    fp2 = fp::add(fp2, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */)));
    // 00481667  d9ca                   +fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 00481669  d845f8                 +fadd dword ptr [ebp - 8]
    fp2 = fp::add(fp2, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 0048166c  d9ca                   +fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 0048166e  d910                   +fst dword ptr [eax]
    app->getMemory<float>(cpu.eax) = float(fp2);
    // 00481670  d9c0                   +fld st(0)
    fp3 = x86::Float(fp2);
    // 00481672  d9cb                   +fxch st(3)
    {
        x86::Float tmp = fp3;
        fp3 = fp0;
        fp0 = tmp;
    }
    // 00481674  d95044                 +fst dword ptr [eax + 0x44]
    app->getMemory<float>(cpu.eax + x86::reg32(68) /* 0x44 */) = float(fp3);
    // 00481677  d9cb                   +fxch st(3)
    {
        x86::Float tmp = fp3;
        fp3 = fp0;
        fp0 = tmp;
    }
    // 00481679  d95860                 +fstp dword ptr [eax + 0x60]
    app->getMemory<float>(cpu.eax + x86::reg32(96) /* 0x60 */) = float(fp3);
    // 0048167c  dec1                   +faddp st(1)
    fp1 = fp::add(fp1, fp2);
    // 0048167e  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 00481680  d95864                 +fstp dword ptr [eax + 0x64]
    app->getMemory<float>(cpu.eax + x86::reg32(100) /* 0x64 */) = float(fp1);
    // 00481683  d95020                 +fst dword ptr [eax + 0x20]
    app->getMemory<float>(cpu.eax + x86::reg32(32) /* 0x20 */) = float(fp0);
    // 00481686  d95840                 +fstp dword ptr [eax + 0x40]
    app->getMemory<float>(cpu.eax + x86::reg32(64) /* 0x40 */) = float(fp0);
    // 00481689  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0048168a  eb94                   -jmp 0x481620
    goto L_0x00481620;
  case 0x0048168c:
    // 0048168c  d945fc                 +fld dword ptr [ebp - 4]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
    // 0048168f  8b4df4                 -mov ecx, dword ptr [ebp - 0xc]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00481692  d80d30b55300           +fmul dword ptr [0x53b530]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(5485872) /* 0x53b530 */)));
    // 00481698  8908                   -mov dword ptr [eax], ecx
    app->getMemory<x86::reg32>(cpu.eax) = cpu.ecx;
    // 0048169a  d900                   +fld dword ptr [eax]
    fp1 = x86::Float(app->getMemory<float>(cpu.eax));
    // 0048169c  d9c0                   +fld st(0)
    fp2 = x86::Float(fp1);
    // 0048169e  8b4df0                 -mov ecx, dword ptr [ebp - 0x10]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004816a1  d95860                 +fstp dword ptr [eax + 0x60]
    app->getMemory<float>(cpu.eax + x86::reg32(96) /* 0x60 */) = float(fp2);
    // 004816a4  894804                 -mov dword ptr [eax + 4], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */) = cpu.ecx;
    // 004816a7  dec1                   +faddp st(1)
    fp0 = fp::add(fp0, fp1);
    // 004816a9  d94004                 +fld dword ptr [eax + 4]
    fp1 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 004816ac  d9c0                   +fld st(0)
    fp2 = x86::Float(fp1);
    // 004816ae  d9ca                   +fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 004816b0  d95020                 +fst dword ptr [eax + 0x20]
    app->getMemory<float>(cpu.eax + x86::reg32(32) /* 0x20 */) = float(fp2);
    // 004816b3  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp2;
        fp2 = fp1;
        fp1 = tmp;
    }
    // 004816b5  d95824                 +fstp dword ptr [eax + 0x24]
    app->getMemory<float>(cpu.eax + x86::reg32(36) /* 0x24 */) = float(fp2);
    // 004816b8  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 004816ba  d845f8                 +fadd dword ptr [ebp - 8]
    fp1 = fp::add(fp1, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 004816bd  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 004816bf  d95840                 +fstp dword ptr [eax + 0x40]
    app->getMemory<float>(cpu.eax + x86::reg32(64) /* 0x40 */) = float(fp1);
    // 004816c2  d95044                 +fst dword ptr [eax + 0x44]
    app->getMemory<float>(cpu.eax + x86::reg32(68) /* 0x44 */) = float(fp0);
    // 004816c5  d95864                 +fstp dword ptr [eax + 0x64]
    app->getMemory<float>(cpu.eax + x86::reg32(100) /* 0x64 */) = float(fp0);
    // 004816c8  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004816c9  e952ffffff             -jmp 0x481620
    goto L_0x00481620;
  case 0x004816ce:
    // 004816ce  d945fc                 +fld dword ptr [ebp - 4]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
    // 004816d1  d9c0                   +fld st(0)
    fp1 = x86::Float(fp0);
    // 004816d3  d80d34b55300           +fmul dword ptr [0x53b534]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<float>(x86::reg32(5485876) /* 0x53b534 */)));
    // 004816d9  8b4df0                 -mov ecx, dword ptr [ebp - 0x10]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004816dc  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 004816de  d80d38b55300           +fmul dword ptr [0x53b538]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<float>(x86::reg32(5485880) /* 0x53b538 */)));
    // 004816e4  894804                 -mov dword ptr [eax + 4], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */) = cpu.ecx;
    // 004816e7  d94004                 +fld dword ptr [eax + 4]
    fp2 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 004816ea  d95024                 +fst dword ptr [eax + 0x24]
    app->getMemory<float>(cpu.eax + x86::reg32(36) /* 0x24 */) = float(fp2);
    // 004816ed  d9ca                   +fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 004816ef  d845f4                 +fadd dword ptr [ebp - 0xc]
    fp2 = fp::add(fp2, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */)));
    // 004816f2  d9ca                   +fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 004816f4  d845f8                 +fadd dword ptr [ebp - 8]
    fp2 = fp::add(fp2, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 004816f7  d9ca                   +fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 004816f9  d910                   +fst dword ptr [eax]
    app->getMemory<float>(cpu.eax) = float(fp2);
    // 004816fb  d9c0                   +fld st(0)
    fp3 = x86::Float(fp2);
    // 004816fd  d9cb                   +fxch st(3)
    {
        x86::Float tmp = fp3;
        fp3 = fp0;
        fp0 = tmp;
    }
    // 004816ff  d95044                 +fst dword ptr [eax + 0x44]
    app->getMemory<float>(cpu.eax + x86::reg32(68) /* 0x44 */) = float(fp3);
    // 00481702  d9cb                   +fxch st(3)
    {
        x86::Float tmp = fp3;
        fp3 = fp0;
        fp0 = tmp;
    }
    // 00481704  d95860                 +fstp dword ptr [eax + 0x60]
    app->getMemory<float>(cpu.eax + x86::reg32(96) /* 0x60 */) = float(fp3);
    // 00481707  dec1                   +faddp st(1)
    fp1 = fp::add(fp1, fp2);
    // 00481709  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 0048170b  d95864                 +fstp dword ptr [eax + 0x64]
    app->getMemory<float>(cpu.eax + x86::reg32(100) /* 0x64 */) = float(fp1);
    // 0048170e  d95020                 +fst dword ptr [eax + 0x20]
    app->getMemory<float>(cpu.eax + x86::reg32(32) /* 0x20 */) = float(fp0);
    // 00481711  d95840                 +fstp dword ptr [eax + 0x40]
    app->getMemory<float>(cpu.eax + x86::reg32(64) /* 0x40 */) = float(fp0);
    // 00481714  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00481715  e906ffffff             -jmp 0x481620
    goto L_0x00481620;
  case 0x0048171a:
    // 0048171a  d945fc                 +fld dword ptr [ebp - 4]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
    // 0048171d  d9c0                   +fld st(0)
    fp1 = x86::Float(fp0);
    // 0048171f  d80d3cb55300           +fmul dword ptr [0x53b53c]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<float>(x86::reg32(5485884) /* 0x53b53c */)));
    // 00481725  d945f8                 +fld dword ptr [ebp - 8]
    fp2 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */));
    // 00481728  d9c0                   +fld st(0)
    fp3 = x86::Float(fp2);
    // 0048172a  d80d40b55300           +fmul dword ptr [0x53b540]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(x86::reg32(5485888) /* 0x53b540 */)));
    // 00481730  d9cb                   +fxch st(3)
    {
        x86::Float tmp = fp3;
        fp3 = fp0;
        fp0 = tmp;
    }
    // 00481732  d80d44b55300           +fmul dword ptr [0x53b544]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(x86::reg32(5485892) /* 0x53b544 */)));
    // 00481738  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp3;
        fp3 = fp2;
        fp2 = tmp;
    }
    // 0048173a  d80d48b55300           +fmul dword ptr [0x53b548]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(x86::reg32(5485896) /* 0x53b548 */)));
    // 00481740  e91f010000             -jmp 0x481864
    goto L_0x00481864;
  case 0x00481745:
    // 00481745  d945fc                 +fld dword ptr [ebp - 4]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
    // 00481748  d9c0                   +fld st(0)
    fp1 = x86::Float(fp0);
    // 0048174a  d80d4cb55300           +fmul dword ptr [0x53b54c]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<float>(x86::reg32(5485900) /* 0x53b54c */)));
    // 00481750  d945f8                 +fld dword ptr [ebp - 8]
    fp2 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */));
    // 00481753  d9c0                   +fld st(0)
    fp3 = x86::Float(fp2);
    // 00481755  d80d40b55300           +fmul dword ptr [0x53b540]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(x86::reg32(5485888) /* 0x53b540 */)));
    // 0048175b  d9cb                   +fxch st(3)
    {
        x86::Float tmp = fp3;
        fp3 = fp0;
        fp0 = tmp;
    }
    // 0048175d  d80d50b55300           +fmul dword ptr [0x53b550]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(x86::reg32(5485904) /* 0x53b550 */)));
    // 00481763  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp3;
        fp3 = fp2;
        fp2 = tmp;
    }
    // 00481765  d80d48b55300           +fmul dword ptr [0x53b548]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(x86::reg32(5485896) /* 0x53b548 */)));
    // 0048176b  e9f4000000             -jmp 0x481864
    goto L_0x00481864;
  case 0x00481770:
    // 00481770  d945fc                 +fld dword ptr [ebp - 4]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
    // 00481773  d9c0                   +fld st(0)
    fp1 = x86::Float(fp0);
    // 00481775  d80d54b55300           +fmul dword ptr [0x53b554]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<float>(x86::reg32(5485908) /* 0x53b554 */)));
    // 0048177b  d945f8                 +fld dword ptr [ebp - 8]
    fp2 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */));
    // 0048177e  d9c0                   +fld st(0)
    fp3 = x86::Float(fp2);
    // 00481780  d80d58b55300           +fmul dword ptr [0x53b558]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(x86::reg32(5485912) /* 0x53b558 */)));
    // 00481786  d9cb                   +fxch st(3)
    {
        x86::Float tmp = fp3;
        fp3 = fp0;
        fp0 = tmp;
    }
    // 00481788  d80d5cb55300           +fmul dword ptr [0x53b55c]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(x86::reg32(5485916) /* 0x53b55c */)));
    // 0048178e  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp3;
        fp3 = fp2;
        fp2 = tmp;
    }
    // 00481790  d80d60b55300           +fmul dword ptr [0x53b560]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(x86::reg32(5485920) /* 0x53b560 */)));
    // 00481796  e9c9000000             -jmp 0x481864
    goto L_0x00481864;
  case 0x0048179b:
    // 0048179b  d945fc                 +fld dword ptr [ebp - 4]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
    // 0048179e  d9c0                   +fld st(0)
    fp1 = x86::Float(fp0);
    // 004817a0  d80d64b55300           +fmul dword ptr [0x53b564]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<float>(x86::reg32(5485924) /* 0x53b564 */)));
    // 004817a6  d945f8                 +fld dword ptr [ebp - 8]
    fp2 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */));
    // 004817a9  d9c0                   +fld st(0)
    fp3 = x86::Float(fp2);
    // 004817ab  d80d58b55300           +fmul dword ptr [0x53b558]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(x86::reg32(5485912) /* 0x53b558 */)));
    // 004817b1  d9cb                   +fxch st(3)
    {
        x86::Float tmp = fp3;
        fp3 = fp0;
        fp0 = tmp;
    }
    // 004817b3  d80d68b55300           +fmul dword ptr [0x53b568]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(x86::reg32(5485928) /* 0x53b568 */)));
    // 004817b9  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp3;
        fp3 = fp2;
        fp2 = tmp;
    }
    // 004817bb  d80d60b55300           +fmul dword ptr [0x53b560]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(x86::reg32(5485920) /* 0x53b560 */)));
    // 004817c1  e99e000000             -jmp 0x481864
    goto L_0x00481864;
  case 0x004817c6:
    // 004817c6  d945fc                 +fld dword ptr [ebp - 4]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
    // 004817c9  d9c0                   +fld st(0)
    fp1 = x86::Float(fp0);
    // 004817cb  d80d6cb55300           +fmul dword ptr [0x53b56c]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<float>(x86::reg32(5485932) /* 0x53b56c */)));
    // 004817d1  d945f8                 +fld dword ptr [ebp - 8]
    fp2 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */));
    // 004817d4  d9c0                   +fld st(0)
    fp3 = x86::Float(fp2);
    // 004817d6  d80d58b55300           +fmul dword ptr [0x53b558]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(x86::reg32(5485912) /* 0x53b558 */)));
    // 004817dc  d9cb                   +fxch st(3)
    {
        x86::Float tmp = fp3;
        fp3 = fp0;
        fp0 = tmp;
    }
    // 004817de  d80d70b55300           +fmul dword ptr [0x53b570]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(x86::reg32(5485936) /* 0x53b570 */)));
    // 004817e4  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp3;
        fp3 = fp2;
        fp2 = tmp;
    }
    // 004817e6  d80d60b55300           +fmul dword ptr [0x53b560]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(x86::reg32(5485920) /* 0x53b560 */)));
    // 004817ec  eb76                   -jmp 0x481864
    goto L_0x00481864;
  case 0x004817ee:
    // 004817ee  d945fc                 +fld dword ptr [ebp - 4]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
    // 004817f1  d9c0                   +fld st(0)
    fp1 = x86::Float(fp0);
    // 004817f3  d80d74b55300           +fmul dword ptr [0x53b574]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<float>(x86::reg32(5485940) /* 0x53b574 */)));
    // 004817f9  d945f8                 +fld dword ptr [ebp - 8]
    fp2 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */));
    // 004817fc  d9c0                   +fld st(0)
    fp3 = x86::Float(fp2);
    // 004817fe  d80d40b55300           +fmul dword ptr [0x53b540]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(x86::reg32(5485888) /* 0x53b540 */)));
    // 00481804  d9cb                   +fxch st(3)
    {
        x86::Float tmp = fp3;
        fp3 = fp0;
        fp0 = tmp;
    }
    // 00481806  d80d44b55300           +fmul dword ptr [0x53b544]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(x86::reg32(5485892) /* 0x53b544 */)));
    // 0048180c  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp3;
        fp3 = fp2;
        fp2 = tmp;
    }
    // 0048180e  d80d48b55300           +fmul dword ptr [0x53b548]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(x86::reg32(5485896) /* 0x53b548 */)));
    // 00481814  eb4e                   -jmp 0x481864
    goto L_0x00481864;
  case 0x00481816:
    // 00481816  d945fc                 +fld dword ptr [ebp - 4]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
    // 00481819  d9c0                   +fld st(0)
    fp1 = x86::Float(fp0);
    // 0048181b  d80d30b55300           +fmul dword ptr [0x53b530]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<float>(x86::reg32(5485872) /* 0x53b530 */)));
    // 00481821  d945f8                 +fld dword ptr [ebp - 8]
    fp2 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */));
    // 00481824  d9c0                   +fld st(0)
    fp3 = x86::Float(fp2);
    // 00481826  d80d78b55300           +fmul dword ptr [0x53b578]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(x86::reg32(5485944) /* 0x53b578 */)));
    // 0048182c  d9cb                   +fxch st(3)
    {
        x86::Float tmp = fp3;
        fp3 = fp0;
        fp0 = tmp;
    }
    // 0048182e  d80d7cb55300           +fmul dword ptr [0x53b57c]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(x86::reg32(5485948) /* 0x53b57c */)));
    // 00481834  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp3;
        fp3 = fp2;
        fp2 = tmp;
    }
    // 00481836  d80d80b55300           +fmul dword ptr [0x53b580]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(x86::reg32(5485952) /* 0x53b580 */)));
    // 0048183c  eb26                   -jmp 0x481864
    goto L_0x00481864;
  case 0x0048183e:
    // 0048183e  d945fc                 +fld dword ptr [ebp - 4]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
    // 00481841  d9c0                   +fld st(0)
    fp1 = x86::Float(fp0);
    // 00481843  d80d5cb55300           +fmul dword ptr [0x53b55c]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<float>(x86::reg32(5485916) /* 0x53b55c */)));
    // 00481849  d945f8                 +fld dword ptr [ebp - 8]
    fp2 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */));
    // 0048184c  d9c0                   +fld st(0)
    fp3 = x86::Float(fp2);
    // 0048184e  d80d40b55300           +fmul dword ptr [0x53b540]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(x86::reg32(5485888) /* 0x53b540 */)));
    // 00481854  d9cb                   +fxch st(3)
    {
        x86::Float tmp = fp3;
        fp3 = fp0;
        fp0 = tmp;
    }
    // 00481856  d80d84b55300           +fmul dword ptr [0x53b584]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(x86::reg32(5485956) /* 0x53b584 */)));
    // 0048185c  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp3;
        fp3 = fp2;
        fp2 = tmp;
    }
    // 0048185e  d80d88b55300           +fmul dword ptr [0x53b588]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(x86::reg32(5485960) /* 0x53b588 */)));
L_0x00481864:
    // 00481864  d9ca                   +fxch st(2)
    {
        x86::Float tmp = fp3;
        fp3 = fp1;
        fp1 = tmp;
    }
    // 00481866  d845f4                 +fadd dword ptr [ebp - 0xc]
    fp3 = fp::add(fp3, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */)));
    // 00481869  d9cb                   +fxch st(3)
    {
        x86::Float tmp = fp3;
        fp3 = fp0;
        fp0 = tmp;
    }
    // 0048186b  d845f0                 +fadd dword ptr [ebp - 0x10]
    fp3 = fp::add(fp3, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-16) /* -0x10 */)));
    // 0048186e  d9cb                   +fxch st(3)
    {
        x86::Float tmp = fp3;
        fp3 = fp0;
        fp0 = tmp;
    }
    // 00481870  d918                   +fstp dword ptr [eax]
    app->getMemory<float>(cpu.eax) = float(fp3);
    // 00481872  d9ca                   +fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 00481874  d95804                 +fstp dword ptr [eax + 4]
    app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */) = float(fp2);
    // 00481877  d900                   +fld dword ptr [eax]
    fp2 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00481879  d94004                 +fld dword ptr [eax + 4]
    fp3 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 0048187c  d9c0                   +fld st(0)
    fp4 = x86::Float(fp3);
    // 0048187e  d900                   +fld dword ptr [eax]
    fp5 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00481880  d9ca                   +fxch st(2)
    {
        x86::Float tmp = fp5;
        fp5 = fp3;
        fp3 = tmp;
    }
    // 00481882  d95824                 +fstp dword ptr [eax + 0x24]
    app->getMemory<float>(cpu.eax + x86::reg32(36) /* 0x24 */) = float(fp5);
    // 00481885  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp4;
        fp4 = fp3;
        fp3 = tmp;
    }
    // 00481887  d95860                 +fstp dword ptr [eax + 0x60]
    app->getMemory<float>(cpu.eax + x86::reg32(96) /* 0x60 */) = float(fp4);
    // 0048188a  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp3;
        fp3 = fp2;
        fp2 = tmp;
    }
    // 0048188c  dec3                   +faddp st(3)
    fp0 = fp::add(fp0, fp3);
    // 0048188e  dec1                   +faddp st(1)
    fp1 = fp::add(fp1, fp2);
    // 00481890  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 00481892  d95820                 +fstp dword ptr [eax + 0x20]
    app->getMemory<float>(cpu.eax + x86::reg32(32) /* 0x20 */) = float(fp1);
    // 00481895  d95044                 +fst dword ptr [eax + 0x44]
    app->getMemory<float>(cpu.eax + x86::reg32(68) /* 0x44 */) = float(fp0);
    // 00481898  8b4820                 -mov ecx, dword ptr [eax + 0x20]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(32) /* 0x20 */);
    // 0048189b  894840                 -mov dword ptr [eax + 0x40], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(64) /* 0x40 */) = cpu.ecx;
    // 0048189e  d95864                 +fstp dword ptr [eax + 0x64]
    app->getMemory<float>(cpu.eax + x86::reg32(100) /* 0x64 */) = float(fp0);
L_0x004818a1:
    // 004818a1  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004818a2  e979fdffff             -jmp 0x481620
    goto L_0x00481620;
L_0x004818a7:
    // 004818a7  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004818a9  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004818aa  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004818ab  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004818ac  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_4818b0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1, fp2;
    // 004818b0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004818b1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004818b2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004818b3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004818b4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004818b5  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004818b7  83ec0c                 -sub esp, 0xc
    (cpu.esp) -= x86::reg32(x86::sreg32(12 /*0xc*/));
    // 004818ba  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004818bc  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004818bf  29c8                   -sub eax, ecx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004818c1  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004818c4  8d1401                 -lea edx, [ecx + eax]
    cpu.edx = x86::reg32(cpu.ecx + cpu.eax * 1);
    // 004818c7  a1581a7900             -mov eax, dword ptr [0x791a58]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7936600) /* 0x791a58 */);
    // 004818cc  c1e207                 -shl edx, 7
    cpu.edx <<= 7 /*0x7*/ % 32;
    // 004818cf  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004818d1  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 004818d3  c1e202                 -shl edx, 2
    cpu.edx <<= 2 /*0x2*/ % 32;
    // 004818d6  29ca                   -sub edx, ecx
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004818d8  c1e203                 -shl edx, 3
    cpu.edx <<= 3 /*0x3*/ % 32;
    // 004818db  29ca                   -sub edx, ecx
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004818dd  c1e204                 -shl edx, 4
    cpu.edx <<= 4 /*0x4*/ % 32;
    // 004818e0  8b8ac0977400           -mov ecx, dword ptr [edx + 0x7497c0]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7641024) /* 0x7497c0 */);
    // 004818e6  8b9ab8977400           -mov ebx, dword ptr [edx + 0x7497b8]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7641016) /* 0x7497b8 */);
    // 004818ec  db82b8977400           -fild dword ptr [edx + 0x7497b8]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7641016) /* 0x7497b8 */)));
    // 004818f2  29d9                   -sub ecx, ebx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 004818f4  8bb2bc977400           -mov esi, dword ptr [edx + 0x7497bc]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7641020) /* 0x7497bc */);
    // 004818fa  894dfc                 -mov dword ptr [ebp - 4], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
    // 004818fd  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004818ff  db45fc                 -fild dword ptr [ebp - 4]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 00481902  d95df4                 -fstp dword ptr [ebp - 0xc]
    app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */) = float(fp1);
    // 00481905  8b8ac4977400           -mov ecx, dword ptr [edx + 0x7497c4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7641028) /* 0x7497c4 */);
    // 0048190b  d95060                 -fst dword ptr [eax + 0x60]
    app->getMemory<float>(cpu.eax + x86::reg32(96) /* 0x60 */) = float(fp0);
    // 0048190e  29f1                   +sub ecx, esi
    {
        x86::reg32& tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.esi));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00481910  d918                   +fstp dword ptr [eax]
    app->getMemory<float>(cpu.eax) = float(fp0);
    // 00481912  894dfc                 -mov dword ptr [ebp - 4], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
    // 00481915  db45fc                 +fild dword ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 00481918  db82c0977400           +fild dword ptr [edx + 0x7497c0]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7641024) /* 0x7497c0 */)));
    // 0048191e  d95840                 +fstp dword ptr [eax + 0x40]
    app->getMemory<float>(cpu.eax + x86::reg32(64) /* 0x40 */) = float(fp1);
    // 00481921  d95df8                 +fstp dword ptr [ebp - 8]
    app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = float(fp0);
    // 00481924  8b4840                 -mov ecx, dword ptr [eax + 0x40]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(64) /* 0x40 */);
    // 00481927  894820                 -mov dword ptr [eax + 0x20], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(32) /* 0x20 */) = cpu.ecx;
    // 0048192a  db82bc977400           +fild dword ptr [edx + 0x7497bc]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7641020) /* 0x7497bc */)));
    // 00481930  d95024                 +fst dword ptr [eax + 0x24]
    app->getMemory<float>(cpu.eax + x86::reg32(36) /* 0x24 */) = float(fp0);
    // 00481933  d95804                 +fstp dword ptr [eax + 4]
    app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */) = float(fp0);
    // 00481936  db82c4977400           +fild dword ptr [edx + 0x7497c4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.edx + x86::reg32(7641028) /* 0x7497c4 */)));
    // 0048193c  d95064                 +fst dword ptr [eax + 0x64]
    app->getMemory<float>(cpu.eax + x86::reg32(100) /* 0x64 */) = float(fp0);
    // 0048193f  d95844                 +fstp dword ptr [eax + 0x44]
    app->getMemory<float>(cpu.eax + x86::reg32(68) /* 0x44 */) = float(fp0);
    // 00481942  eb09                   -jmp 0x48194d
    goto L_0x0048194d;
L_0x00481944:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00481944  83fb08                 +cmp ebx, 8
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
    // 00481947  0f8d9f000000           -jge 0x4819ec
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004819ec;
    }
L_0x0048194d:
    // 0048194d  d945f4                 -fld dword ptr [ebp - 0xc]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */));
    // 00481950  d9049d00d45500         -fld dword ptr [ebx*4 + 0x55d400]
    fp1 = x86::Float(app->getMemory<float>(x86::reg32(5624832) /* 0x55d400 */ + cpu.ebx * 4));
    // 00481957  d8c9                   -fmul st(1)
    fp1 = fp::mul(fp1, x86::Float(fp0));
    // 00481959  d835b8d35500           -fdiv dword ptr [0x55d3b8]
    fp1 = fp::div(fp1, x86::Float(app->getMemory<float>(x86::reg32(5624760) /* 0x55d3b8 */)));
    // 0048195f  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 00481961  d800                   -fadd dword ptr [eax]
    fp1 = fp::add(fp1, x86::Float(app->getMemory<float>(cpu.eax)));
    // 00481963  c1e207                 +shl edx, 7
    {
        x86::reg8 tmp = 7 /*0x7*/ % 32;
        x86::reg32& op = cpu.edx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 00481966  d99402e0000000         +fst dword ptr [edx + eax + 0xe0]
    app->getMemory<float>(cpu.edx + x86::reg32(224) /* 0xe0 */ + cpu.eax * 1) = float(fp1);
    // 0048196d  d99c0280000000         +fstp dword ptr [edx + eax + 0x80]
    app->getMemory<float>(cpu.edx + x86::reg32(128) /* 0x80 */ + cpu.eax * 1) = float(fp1);
    // 00481974  d945f8                 +fld dword ptr [ebp - 8]
    fp1 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */));
    // 00481977  d9049d20d45500         +fld dword ptr [ebx*4 + 0x55d420]
    fp2 = x86::Float(app->getMemory<float>(x86::reg32(5624864) /* 0x55d420 */ + cpu.ebx * 4));
    // 0048197e  d8c9                   +fmul st(1)
    fp2 = fp::mul(fp2, x86::Float(fp1));
    // 00481980  d835dcd35500           +fdiv dword ptr [0x55d3dc]
    fp2 = fp::div(fp2, x86::Float(app->getMemory<float>(x86::reg32(5624796) /* 0x55d3dc */)));
    // 00481986  d84004                 +fadd dword ptr [eax + 4]
    fp2 = fp::add(fp2, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */)));
    // 00481989  d99402a4000000         +fst dword ptr [edx + eax + 0xa4]
    app->getMemory<float>(cpu.edx + x86::reg32(164) /* 0xa4 */ + cpu.eax * 1) = float(fp2);
    // 00481990  d99c0284000000         +fstp dword ptr [edx + eax + 0x84]
    app->getMemory<float>(cpu.edx + x86::reg32(132) /* 0x84 */ + cpu.eax * 1) = float(fp2);
    // 00481997  d9049d00d45500         +fld dword ptr [ebx*4 + 0x55d400]
    fp2 = x86::Float(app->getMemory<float>(x86::reg32(5624832) /* 0x55d400 */ + cpu.ebx * 4));
    // 0048199e  d8049dbcd35500         +fadd dword ptr [ebx*4 + 0x55d3bc]
    fp2 = fp::add(fp2, x86::Float(app->getMemory<float>(x86::reg32(5624764) /* 0x55d3bc */ + cpu.ebx * 4)));
    // 004819a5  deca                   +fmulp st(2)
    fp0 = fp::mul(fp0, fp2);
    // 004819a7  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 004819a9  d835b8d35500           +fdiv dword ptr [0x55d3b8]
    fp1 = fp::div(fp1, x86::Float(app->getMemory<float>(x86::reg32(5624760) /* 0x55d3b8 */)));
    // 004819af  d800                   +fadd dword ptr [eax]
    fp1 = fp::add(fp1, x86::Float(app->getMemory<float>(cpu.eax)));
    // 004819b1  d99402c0000000         +fst dword ptr [edx + eax + 0xc0]
    app->getMemory<float>(cpu.edx + x86::reg32(192) /* 0xc0 */ + cpu.eax * 1) = float(fp1);
    // 004819b8  d99c02a0000000         +fstp dword ptr [edx + eax + 0xa0]
    app->getMemory<float>(cpu.edx + x86::reg32(160) /* 0xa0 */ + cpu.eax * 1) = float(fp1);
    // 004819bf  d9049d20d45500         +fld dword ptr [ebx*4 + 0x55d420]
    fp1 = x86::Float(app->getMemory<float>(x86::reg32(5624864) /* 0x55d420 */ + cpu.ebx * 4));
    // 004819c6  d8049de0d35500         +fadd dword ptr [ebx*4 + 0x55d3e0]
    fp1 = fp::add(fp1, x86::Float(app->getMemory<float>(x86::reg32(5624800) /* 0x55d3e0 */ + cpu.ebx * 4)));
    // 004819cd  dec9                   +fmulp st(1)
    fp0 = fp::mul(fp0, fp1);
    // 004819cf  d835dcd35500           +fdiv dword ptr [0x55d3dc]
    fp0 = fp::div(fp0, x86::Float(app->getMemory<float>(x86::reg32(5624796) /* 0x55d3dc */)));
    // 004819d5  d84004                 +fadd dword ptr [eax + 4]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */)));
    // 004819d8  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004819d9  d99402e4000000         +fst dword ptr [edx + eax + 0xe4]
    app->getMemory<float>(cpu.edx + x86::reg32(228) /* 0xe4 */ + cpu.eax * 1) = float(fp0);
    // 004819e0  d99c02c4000000         +fstp dword ptr [edx + eax + 0xc4]
    app->getMemory<float>(cpu.edx + x86::reg32(196) /* 0xc4 */ + cpu.eax * 1) = float(fp0);
    // 004819e7  e958ffffff             -jmp 0x481944
    goto L_0x00481944;
L_0x004819ec:
    // 004819ec  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 004819ee  eb09                   -jmp 0x4819f9
    goto L_0x004819f9;
L_0x004819f0:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004819f0  83fb04                 +cmp ebx, 4
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
    // 004819f3  0f8d9f000000           -jge 0x481a98
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00481a98;
    }
L_0x004819f9:
    // 004819f9  d945f4                 -fld dword ptr [ebp - 0xc]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */));
    // 004819fc  d9049d00d45500         -fld dword ptr [ebx*4 + 0x55d400]
    fp1 = x86::Float(app->getMemory<float>(x86::reg32(5624832) /* 0x55d400 */ + cpu.ebx * 4));
    // 00481a03  d8c9                   -fmul st(1)
    fp1 = fp::mul(fp1, x86::Float(fp0));
    // 00481a05  d835b8d35500           -fdiv dword ptr [0x55d3b8]
    fp1 = fp::div(fp1, x86::Float(app->getMemory<float>(x86::reg32(5624760) /* 0x55d3b8 */)));
    // 00481a0b  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 00481a0d  d800                   -fadd dword ptr [eax]
    fp1 = fp::add(fp1, x86::Float(app->getMemory<float>(cpu.eax)));
    // 00481a0f  c1e207                 +shl edx, 7
    {
        x86::reg8 tmp = 7 /*0x7*/ % 32;
        x86::reg32& op = cpu.edx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 00481a12  d99402e0040000         +fst dword ptr [edx + eax + 0x4e0]
    app->getMemory<float>(cpu.edx + x86::reg32(1248) /* 0x4e0 */ + cpu.eax * 1) = float(fp1);
    // 00481a19  d99c0280040000         +fstp dword ptr [edx + eax + 0x480]
    app->getMemory<float>(cpu.edx + x86::reg32(1152) /* 0x480 */ + cpu.eax * 1) = float(fp1);
    // 00481a20  d945f8                 +fld dword ptr [ebp - 8]
    fp1 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */));
    // 00481a23  d9049d20d45500         +fld dword ptr [ebx*4 + 0x55d420]
    fp2 = x86::Float(app->getMemory<float>(x86::reg32(5624864) /* 0x55d420 */ + cpu.ebx * 4));
    // 00481a2a  d8c9                   +fmul st(1)
    fp2 = fp::mul(fp2, x86::Float(fp1));
    // 00481a2c  d835dcd35500           +fdiv dword ptr [0x55d3dc]
    fp2 = fp::div(fp2, x86::Float(app->getMemory<float>(x86::reg32(5624796) /* 0x55d3dc */)));
    // 00481a32  d84004                 +fadd dword ptr [eax + 4]
    fp2 = fp::add(fp2, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */)));
    // 00481a35  d99402a4040000         +fst dword ptr [edx + eax + 0x4a4]
    app->getMemory<float>(cpu.edx + x86::reg32(1188) /* 0x4a4 */ + cpu.eax * 1) = float(fp2);
    // 00481a3c  d99c0284040000         +fstp dword ptr [edx + eax + 0x484]
    app->getMemory<float>(cpu.edx + x86::reg32(1156) /* 0x484 */ + cpu.eax * 1) = float(fp2);
    // 00481a43  d9049d00d45500         +fld dword ptr [ebx*4 + 0x55d400]
    fp2 = x86::Float(app->getMemory<float>(x86::reg32(5624832) /* 0x55d400 */ + cpu.ebx * 4));
    // 00481a4a  d8049dbcd35500         +fadd dword ptr [ebx*4 + 0x55d3bc]
    fp2 = fp::add(fp2, x86::Float(app->getMemory<float>(x86::reg32(5624764) /* 0x55d3bc */ + cpu.ebx * 4)));
    // 00481a51  deca                   +fmulp st(2)
    fp0 = fp::mul(fp0, fp2);
    // 00481a53  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 00481a55  d835b8d35500           +fdiv dword ptr [0x55d3b8]
    fp1 = fp::div(fp1, x86::Float(app->getMemory<float>(x86::reg32(5624760) /* 0x55d3b8 */)));
    // 00481a5b  d800                   +fadd dword ptr [eax]
    fp1 = fp::add(fp1, x86::Float(app->getMemory<float>(cpu.eax)));
    // 00481a5d  d99402c0040000         +fst dword ptr [edx + eax + 0x4c0]
    app->getMemory<float>(cpu.edx + x86::reg32(1216) /* 0x4c0 */ + cpu.eax * 1) = float(fp1);
    // 00481a64  d99c02a0040000         +fstp dword ptr [edx + eax + 0x4a0]
    app->getMemory<float>(cpu.edx + x86::reg32(1184) /* 0x4a0 */ + cpu.eax * 1) = float(fp1);
    // 00481a6b  d9049d20d45500         +fld dword ptr [ebx*4 + 0x55d420]
    fp1 = x86::Float(app->getMemory<float>(x86::reg32(5624864) /* 0x55d420 */ + cpu.ebx * 4));
    // 00481a72  d8049de0d35500         +fadd dword ptr [ebx*4 + 0x55d3e0]
    fp1 = fp::add(fp1, x86::Float(app->getMemory<float>(x86::reg32(5624800) /* 0x55d3e0 */ + cpu.ebx * 4)));
    // 00481a79  dec9                   +fmulp st(1)
    fp0 = fp::mul(fp0, fp1);
    // 00481a7b  d835dcd35500           +fdiv dword ptr [0x55d3dc]
    fp0 = fp::div(fp0, x86::Float(app->getMemory<float>(x86::reg32(5624796) /* 0x55d3dc */)));
    // 00481a81  d84004                 +fadd dword ptr [eax + 4]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */)));
    // 00481a84  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00481a85  d99402e4040000         +fst dword ptr [edx + eax + 0x4e4]
    app->getMemory<float>(cpu.edx + x86::reg32(1252) /* 0x4e4 */ + cpu.eax * 1) = float(fp0);
    // 00481a8c  d99c02c4040000         +fstp dword ptr [edx + eax + 0x4c4]
    app->getMemory<float>(cpu.edx + x86::reg32(1220) /* 0x4c4 */ + cpu.eax * 1) = float(fp0);
    // 00481a93  e958ffffff             -jmp 0x4819f0
    goto L_0x004819f0;
L_0x00481a98:
    // 00481a98  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00481a9a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00481a9b  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00481a9c  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00481a9d  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00481a9e  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00481a9f  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_481aa0: native, nfs3hp::render::hudPanel */

/* sub_481c50: native, nfs3hp::render::hudTable */

/* align: skip 0x90 */
void Application::sub_481f90(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0;
  x86::reg16 fsw = 0;
    // 00481f90  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00481f91  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00481f92  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00481f93  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00481f94  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00481f96  8b15b0d36f00           -mov edx, dword ptr [0x6fd3b0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
    // 00481f9c  83fa01                 +cmp edx, 1
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
    // 00481f9f  7402                   -je 0x481fa3
    if (cpu.flags.zf)
    {
        goto L_0x00481fa3;
    }
    // 00481fa1  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x00481fa3:
    // 00481fa3  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00481fa5  e8260cfaff             -call 0x422bd0
    cpu.esp -= 4;
    nfs3hp::carstate::cameraTarget(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00481faa  f6800002000020         +test byte ptr [eax + 0x200], 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(512) /* 0x200 */) & 32 /*0x20*/));
    // 00481fb1  7407                   -je 0x481fba
    if (cpu.flags.zf)
    {
        goto L_0x00481fba;
    }
    // 00481fb3  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00481fb8  eb02                   -jmp 0x481fbc
    goto L_0x00481fbc;
L_0x00481fba:
    // 00481fba  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x00481fbc:
    // 00481fbc  69c8a4010000           -imul ecx, eax, 0x1a4
    cpu.ecx = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 00481fc2  69d248030000           -imul edx, edx, 0x348
    cpu.edx = x86::reg32(x86::sreg64(x86::sreg32(cpu.edx)) * x86::sreg64(x86::sreg32(840 /*0x348*/)));
    // 00481fc8  8d1c0a                 -lea ebx, [edx + ecx]
    cpu.ebx = x86::reg32(cpu.edx + cpu.ecx * 1);
    // 00481fcb  8b9b70bc6f00           -mov ebx, dword ptr [ebx + 0x6fbc70]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(7322736) /* 0x6fbc70 */);
    // 00481fd1  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 00481fd3  7507                   -jne 0x481fdc
    if (!cpu.flags.zf)
    {
        goto L_0x00481fdc;
    }
    // 00481fd5  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00481fd7  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00481fd8  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00481fd9  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00481fda  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00481fdb  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00481fdc:
    // 00481fdc  81c250bc6f00           -add edx, 0x6fbc50
    (cpu.edx) += x86::reg32(x86::sreg32(7322704 /*0x6fbc50*/));
    // 00481fe2  01ca                   -add edx, ecx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.ecx));
    // 00481fe4  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00481fe6  7508                   -jne 0x481ff0
    if (!cpu.flags.zf)
    {
        goto L_0x00481ff0;
    }
    // 00481fe8  81c234010000           +add edx, 0x134
    {
        x86::reg32& tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(308 /*0x134*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00481fee  eb06                   -jmp 0x481ff6
    goto L_0x00481ff6;
L_0x00481ff0:
    // 00481ff0  81c284010000           +add edx, 0x184
    {
        x86::reg32& tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(388 /*0x184*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
L_0x00481ff6:
    // 00481ff6  d9420c                 +fld dword ptr [edx + 0xc]
    fp0 = x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(12) /* 0xc */));
    // 00481ff9  d86204                 +fsub dword ptr [edx + 4]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(4) /* 0x4 */)));
    // 00481ffc  d81dc0b55300           +fcomp dword ptr [0x53b5c0]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(x86::reg32(5486016) /* 0x53b5c0 */)));
    // 00482002  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00482004  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00482005  7710                   -ja 0x482017
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x00482017;
    }
    // 00482007  d94208                 +fld dword ptr [edx + 8]
    fp0 = x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(8) /* 0x8 */));
    // 0048200a  d822                   +fsub dword ptr [edx]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(cpu.edx)));
    // 0048200c  d81dc0b55300           +fcomp dword ptr [0x53b5c0]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(x86::reg32(5486016) /* 0x53b5c0 */)));
    // 00482012  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00482014  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00482015  7607                   -jbe 0x48201e
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0048201e;
    }
L_0x00482017:
    // 00482017  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00482019  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0048201a  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0048201b  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0048201c  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0048201d  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0048201e:
    // 0048201e  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00482023  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00482024  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00482025  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00482026  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00482027  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_482030(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00482030  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00482031  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00482032  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00482033  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00482034  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00482036  83ec08                 -sub esp, 8
    (cpu.esp) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 00482039  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0048203c  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 0048203e  895df8                 -mov dword ptr [ebp - 8], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.ebx;
    // 00482041  8d1c9500000000         -lea ebx, [edx*4]
    cpu.ebx = x86::reg32(cpu.edx * 4);
    // 00482048  83bb38d3550000         +cmp dword ptr [ebx + 0x55d338], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(5624632) /* 0x55d338 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0048204f  7c5b                   -jl 0x4820ac
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004820ac;
    }
    // 00482051  f605583a7a0002         +test byte ptr [0x7a3a58], 2
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(x86::reg32(8010328) /* 0x7a3a58 */) & 2 /*0x2*/));
    // 00482058  7552                   -jne 0x4820ac
    if (!cpu.flags.zf)
    {
        goto L_0x004820ac;
    }
    // 0048205a  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0048205c  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00482063  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00482065  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00482068  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 0048206a  8b15841a7900           -mov edx, dword ptr [0x791a84]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7936644) /* 0x791a84 */);
    // 00482070  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00482073  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00482075  8b8338d35500           -mov eax, dword ptr [ebx + 0x55d338]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(5624632) /* 0x55d338 */);
    // 0048207b  40                     -inc eax
    (cpu.eax)++;
    // 0048207c  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 0048207f  8d3c02                 -lea edi, [edx + eax]
    cpu.edi = x86::reg32(cpu.edx + cpu.eax * 1);
    // 00482082  8d4203                 -lea eax, [edx + 3]
    cpu.eax = x86::reg32(cpu.edx + x86::reg32(3) /* 0x3 */);
    // 00482085  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00482086  8d4a02                 -lea ecx, [edx + 2]
    cpu.ecx = x86::reg32(cpu.edx + x86::reg32(2) /* 0x2 */);
    // 00482089  b8389a7400             -mov eax, 0x749a38
    cpu.eax = 7641656 /*0x749a38*/;
    // 0048208e  8d5a01                 -lea ebx, [edx + 1]
    cpu.ebx = x86::reg32(cpu.edx + x86::reg32(1) /* 0x1 */);
    // 00482091  e82a9d0100             -call 0x49bdc0
    cpu.esp -= 4;
    nfs3hp::port::text::unqueueQuad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00482096  8d4703                 -lea eax, [edi + 3]
    cpu.eax = x86::reg32(cpu.edi + x86::reg32(3) /* 0x3 */);
    // 00482099  8d4f02                 -lea ecx, [edi + 2]
    cpu.ecx = x86::reg32(cpu.edi + x86::reg32(2) /* 0x2 */);
    // 0048209c  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0048209d  8d5f01                 -lea ebx, [edi + 1]
    cpu.ebx = x86::reg32(cpu.edi + x86::reg32(1) /* 0x1 */);
    // 004820a0  b8389a7400             -mov eax, 0x749a38
    cpu.eax = 7641656 /*0x749a38*/;
    // 004820a5  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 004820a7  e8149d0100             -call 0x49bdc0
    cpu.esp -= 4;
    nfs3hp::port::text::unqueueQuad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004820ac:
    // 004820ac  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004820af  8914b538d35500         -mov dword ptr [esi*4 + 0x55d338], edx
    app->getMemory<x86::reg32>(x86::reg32(5624632) /* 0x55d338 */ + cpu.esi * 4) = cpu.edx;
    // 004820b6  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004820b8  0f8cb5000000           -jl 0x482173
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00482173;
    }
    // 004820be  8d04b500000000         -lea eax, [esi*4]
    cpu.eax = x86::reg32(cpu.esi * 4);
    // 004820c5  29f0                   -sub eax, esi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.esi));
    // 004820c7  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004820ca  01f0                   -add eax, esi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.esi));
    // 004820cc  8b15581a7900           -mov edx, dword ptr [0x791a58]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7936600) /* 0x791a58 */);
    // 004820d2  c1e007                 -shl eax, 7
    cpu.eax <<= 7 /*0x7*/ % 32;
    // 004820d5  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004820d7  8d14b500000000         -lea edx, [esi*4]
    cpu.edx = x86::reg32(cpu.esi * 4);
    // 004820de  29f2                   -sub edx, esi
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.esi));
    // 004820e0  c1e202                 -shl edx, 2
    cpu.edx <<= 2 /*0x2*/ % 32;
    // 004820e3  8b5dfc                 -mov ebx, dword ptr [ebp - 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004820e6  01f2                   -add edx, esi
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.esi));
    // 004820e8  8b3d841a7900           -mov edi, dword ptr [0x791a84]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7936644) /* 0x791a84 */);
    // 004820ee  c1e202                 -shl edx, 2
    cpu.edx <<= 2 /*0x2*/ % 32;
    // 004820f1  c1e302                 -shl ebx, 2
    cpu.ebx <<= 2 /*0x2*/ % 32;
    // 004820f4  01d7                   -add edi, edx
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.edx));
    // 004820f6  8d5304                 -lea edx, [ebx + 4]
    cpu.edx = x86::reg32(cpu.ebx + x86::reg32(4) /* 0x4 */);
    // 004820f9  8d3417                 -lea esi, [edi + edx]
    cpu.esi = x86::reg32(cpu.edi + cpu.edx * 1);
    // 004820fc  8b0d841a7900           -mov ecx, dword ptr [0x791a84]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7936644) /* 0x791a84 */);
    // 00482102  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 00482104  29ca                   -sub edx, ecx
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 00482106  c1e205                 -shl edx, 5
    cpu.edx <<= 5 /*0x5*/ % 32;
    // 00482109  8b4df8                 -mov ecx, dword ptr [ebp - 8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0048210c  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0048210e  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 00482110  740e                   -je 0x482120
    if (cpu.flags.zf)
    {
        goto L_0x00482120;
    }
    // 00482112  894850                 -mov dword ptr [eax + 0x50], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(80) /* 0x50 */) = cpu.ecx;
    // 00482115  894830                 -mov dword ptr [eax + 0x30], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(48) /* 0x30 */) = cpu.ecx;
    // 00482118  894810                 -mov dword ptr [eax + 0x10], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */) = cpu.ecx;
    // 0048211b  894870                 -mov dword ptr [eax + 0x70], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(112) /* 0x70 */) = cpu.ecx;
    // 0048211e  eb12                   -jmp 0x482132
    goto L_0x00482132;
L_0x00482120:
    // 00482120  8b8b40d35500           -mov ecx, dword ptr [ebx + 0x55d340]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(5624640) /* 0x55d340 */);
    // 00482126  894870                 -mov dword ptr [eax + 0x70], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(112) /* 0x70 */) = cpu.ecx;
    // 00482129  894850                 -mov dword ptr [eax + 0x50], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(80) /* 0x50 */) = cpu.ecx;
    // 0048212c  894830                 -mov dword ptr [eax + 0x30], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(48) /* 0x30 */) = cpu.ecx;
    // 0048212f  894810                 -mov dword ptr [eax + 0x10], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */) = cpu.ecx;
L_0x00482132:
    // 00482132  894a70                 -mov dword ptr [edx + 0x70], ecx
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(112) /* 0x70 */) = cpu.ecx;
    // 00482135  894a50                 -mov dword ptr [edx + 0x50], ecx
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(80) /* 0x50 */) = cpu.ecx;
    // 00482138  894a30                 -mov dword ptr [edx + 0x30], ecx
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(48) /* 0x30 */) = cpu.ecx;
    // 0048213b  894a10                 -mov dword ptr [edx + 0x10], ecx
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(16) /* 0x10 */) = cpu.ecx;
    // 0048213e  f605583a7a0002         +test byte ptr [0x7a3a58], 2
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(x86::reg32(8010328) /* 0x7a3a58 */) & 2 /*0x2*/));
    // 00482145  752c                   -jne 0x482173
    if (!cpu.flags.zf)
    {
        goto L_0x00482173;
    }
    // 00482147  8d4703                 -lea eax, [edi + 3]
    cpu.eax = x86::reg32(cpu.edi + x86::reg32(3) /* 0x3 */);
    // 0048214a  8d4f02                 -lea ecx, [edi + 2]
    cpu.ecx = x86::reg32(cpu.edi + x86::reg32(2) /* 0x2 */);
    // 0048214d  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0048214e  8d5f01                 -lea ebx, [edi + 1]
    cpu.ebx = x86::reg32(cpu.edi + x86::reg32(1) /* 0x1 */);
    // 00482151  b8389a7400             -mov eax, 0x749a38
    cpu.eax = 7641656 /*0x749a38*/;
    // 00482156  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 00482158  e8d39b0100             -call 0x49bd30
    cpu.esp -= 4;
    nfs3hp::port::text::queueQuad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0048215d  8d4603                 -lea eax, [esi + 3]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(3) /* 0x3 */);
    // 00482160  8d4e02                 -lea ecx, [esi + 2]
    cpu.ecx = x86::reg32(cpu.esi + x86::reg32(2) /* 0x2 */);
    // 00482163  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00482164  8d5e01                 -lea ebx, [esi + 1]
    cpu.ebx = x86::reg32(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 00482167  b8389a7400             -mov eax, 0x749a38
    cpu.eax = 7641656 /*0x749a38*/;
    // 0048216c  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0048216e  e8bd9b0100             -call 0x49bd30
    cpu.esp -= 4;
    nfs3hp::port::text::queueQuad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00482173:
    // 00482173  8b5dfc                 -mov ebx, dword ptr [ebp - 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00482176  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00482177  68c4b55300             -push 0x53b5c4
    app->getMemory<x86::reg32>(cpu.esp-4) = 5486020 /*0x53b5c4*/;
    cpu.esp -= 4;
    // 0048217c  e87fb3faff             -call 0x42d500
    cpu.esp -= 4;
    nfs3hp::raceload::emptyHook(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00482181  83c408                 -add esp, 8
    (cpu.esp) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 00482184  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00482186  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00482187  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00482188  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00482189  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0048218a  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_482190(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00482190  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00482191  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00482192  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00482193  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00482194  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00482195  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00482196  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00482198  83ec20                 -sub esp, 0x20
    (cpu.esp) -= x86::reg32(x86::sreg32(32 /*0x20*/));
    // 0048219b  8d3c8500000000         -lea edi, [eax*4]
    cpu.edi = x86::reg32(cpu.eax * 4);
    // 004821a2  83bf38d3550000         +cmp dword ptr [edi + 0x55d338], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(5624632) /* 0x55d338 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004821a9  0f8c4b040000           -jl 0x4825fa
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004825fa;
    }
    // 004821af  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004821b1  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004821b4  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004821b6  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004821b9  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004821bb  8b15581a7900           -mov edx, dword ptr [0x791a58]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7936600) /* 0x791a58 */);
    // 004821c1  c1e007                 -shl eax, 7
    cpu.eax <<= 7 /*0x7*/ % 32;
    // 004821c4  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004821c6  8b8738d35500           -mov eax, dword ptr [edi + 0x55d338]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(5624632) /* 0x55d338 */);
    // 004821cc  8955e0                 -mov dword ptr [ebp - 0x20], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */) = cpu.edx;
    // 004821cf  40                     -inc eax
    (cpu.eax)++;
    // 004821d0  8b7de0                 -mov edi, dword ptr [ebp - 0x20]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 004821d3  c1e007                 -shl eax, 7
    cpu.eax <<= 7 /*0x7*/ % 32;
    // 004821d6  8b75e0                 -mov esi, dword ptr [ebp - 0x20]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 004821d9  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004821db  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004821e0  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 004821e3  8b15389a7400           -mov edx, dword ptr [0x749a38]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7641656) /* 0x749a38 */);
    // 004821e9  83c714                 -add edi, 0x14
    (cpu.edi) += x86::reg32(x86::sreg32(20 /*0x14*/));
    // 004821ec  e80ff7faff             -call 0x431900
    cpu.esp -= 4;
    nfs3hp::graphics::setRenderState(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004821f1  8b460c                 -mov eax, dword ptr [esi + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(12) /* 0xc */);
    // 004821f4  8b0df8347d00           -mov ecx, dword ptr [0x7d34f8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205560) /* 0x7d34f8 */);
    // 004821fa  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 004821ff  7558                   -jne 0x482259
    if (!cpu.flags.zf)
    {
        goto L_0x00482259;
    }
    // 00482201  39c8                   +cmp eax, ecx
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
    // 00482203  7d54                   -jge 0x482259
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00482259;
    }
    // 00482205  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 00482208  bb00000000             -mov ebx, 0
    cpu.ebx = 0 /*0x0*/;
    // 0048220d  8b0d04357d00           -mov ecx, dword ptr [0x7d3504]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205572) /* 0x7d3504 */);
    // 00482213  8b1500357d00           -mov edx, dword ptr [0x7d3500]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205568) /* 0x7d3500 */);
    // 00482219  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 0048221e  750d                   -jne 0x48222d
    if (!cpu.flags.zf)
    {
        goto L_0x0048222d;
    }
    // 00482220  39c8                   +cmp eax, ecx
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
    // 00482222  7c09                   -jl 0x48222d
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0048222d;
    }
    // 00482224  39d0                   +cmp eax, edx
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
    // 00482226  7e09                   -jle 0x482231
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00482231;
    }
    // 00482228  83cb04                 +or ebx, 4
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(4 /*0x4*/))));
    // 0048222b  eb04                   -jmp 0x482231
    goto L_0x00482231;
L_0x0048222d:
    // 0048222d  83cb08                 -or ebx, 8
    cpu.ebx |= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 00482230  90                     -nop 
    ;
L_0x00482231:
    // 00482231  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 00482233  90                     -nop 
    ;
    // 00482234  8b0dfc347d00           -mov ecx, dword ptr [0x7d34fc]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205564) /* 0x7d34fc */);
    // 0048223a  8b150c357d00           -mov edx, dword ptr [0x7d350c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205580) /* 0x7d350c */);
    // 00482240  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 00482245  750d                   -jne 0x482254
    if (!cpu.flags.zf)
    {
        goto L_0x00482254;
    }
    // 00482247  39c8                   +cmp eax, ecx
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
    // 00482249  7c09                   -jl 0x482254
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00482254;
    }
    // 0048224b  39d0                   +cmp eax, edx
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
    // 0048224d  7e0f                   -jle 0x48225e
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0048225e;
    }
    // 0048224f  83cb02                 +or ebx, 2
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(2 /*0x2*/))));
    // 00482252  eb0a                   -jmp 0x48225e
    goto L_0x0048225e;
L_0x00482254:
    // 00482254  83cb01                 +or ebx, 1
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(1 /*0x1*/))));
    // 00482257  eb05                   -jmp 0x48225e
    goto L_0x0048225e;
L_0x00482259:
    // 00482259  bb10000000             -mov ebx, 0x10
    cpu.ebx = 16 /*0x10*/;
L_0x0048225e:
    // 0048225e  881f                   -mov byte ptr [edi], bl
    app->getMemory<x86::reg8>(cpu.edi) = cpu.bl;
    // 00482260  8b45e0                 -mov eax, dword ptr [ebp - 0x20]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 00482263  8b7de0                 -mov edi, dword ptr [ebp - 0x20]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 00482266  83c020                 -add eax, 0x20
    (cpu.eax) += x86::reg32(x86::sreg32(32 /*0x20*/));
    // 00482269  83c734                 -add edi, 0x34
    (cpu.edi) += x86::reg32(x86::sreg32(52 /*0x34*/));
    // 0048226c  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0048226e  8945e4                 -mov dword ptr [ebp - 0x1c], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */) = cpu.eax;
    // 00482271  8b460c                 -mov eax, dword ptr [esi + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(12) /* 0xc */);
    // 00482274  8b0df8347d00           -mov ecx, dword ptr [0x7d34f8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205560) /* 0x7d34f8 */);
    // 0048227a  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 0048227f  7558                   -jne 0x4822d9
    if (!cpu.flags.zf)
    {
        goto L_0x004822d9;
    }
    // 00482281  39c8                   +cmp eax, ecx
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
    // 00482283  7d54                   -jge 0x4822d9
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004822d9;
    }
    // 00482285  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 00482288  bb00000000             -mov ebx, 0
    cpu.ebx = 0 /*0x0*/;
    // 0048228d  8b0d04357d00           -mov ecx, dword ptr [0x7d3504]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205572) /* 0x7d3504 */);
    // 00482293  8b1500357d00           -mov edx, dword ptr [0x7d3500]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205568) /* 0x7d3500 */);
    // 00482299  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 0048229e  750d                   -jne 0x4822ad
    if (!cpu.flags.zf)
    {
        goto L_0x004822ad;
    }
    // 004822a0  39c8                   +cmp eax, ecx
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
    // 004822a2  7c09                   -jl 0x4822ad
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004822ad;
    }
    // 004822a4  39d0                   +cmp eax, edx
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
    // 004822a6  7e09                   -jle 0x4822b1
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004822b1;
    }
    // 004822a8  83cb04                 +or ebx, 4
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(4 /*0x4*/))));
    // 004822ab  eb04                   -jmp 0x4822b1
    goto L_0x004822b1;
L_0x004822ad:
    // 004822ad  83cb08                 -or ebx, 8
    cpu.ebx |= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 004822b0  90                     -nop 
    ;
L_0x004822b1:
    // 004822b1  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 004822b3  90                     -nop 
    ;
    // 004822b4  8b0dfc347d00           -mov ecx, dword ptr [0x7d34fc]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205564) /* 0x7d34fc */);
    // 004822ba  8b150c357d00           -mov edx, dword ptr [0x7d350c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205580) /* 0x7d350c */);
    // 004822c0  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 004822c5  750d                   -jne 0x4822d4
    if (!cpu.flags.zf)
    {
        goto L_0x004822d4;
    }
    // 004822c7  39c8                   +cmp eax, ecx
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
    // 004822c9  7c09                   -jl 0x4822d4
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004822d4;
    }
    // 004822cb  39d0                   +cmp eax, edx
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
    // 004822cd  7e0f                   -jle 0x4822de
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004822de;
    }
    // 004822cf  83cb02                 +or ebx, 2
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(2 /*0x2*/))));
    // 004822d2  eb0a                   -jmp 0x4822de
    goto L_0x004822de;
L_0x004822d4:
    // 004822d4  83cb01                 +or ebx, 1
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(1 /*0x1*/))));
    // 004822d7  eb05                   -jmp 0x4822de
    goto L_0x004822de;
L_0x004822d9:
    // 004822d9  bb10000000             -mov ebx, 0x10
    cpu.ebx = 16 /*0x10*/;
L_0x004822de:
    // 004822de  881f                   -mov byte ptr [edi], bl
    app->getMemory<x86::reg8>(cpu.edi) = cpu.bl;
    // 004822e0  8b45e0                 -mov eax, dword ptr [ebp - 0x20]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 004822e3  8b7de0                 -mov edi, dword ptr [ebp - 0x20]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 004822e6  83c040                 -add eax, 0x40
    (cpu.eax) += x86::reg32(x86::sreg32(64 /*0x40*/));
    // 004822e9  83c754                 -add edi, 0x54
    (cpu.edi) += x86::reg32(x86::sreg32(84 /*0x54*/));
    // 004822ec  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004822ee  8945e8                 -mov dword ptr [ebp - 0x18], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = cpu.eax;
    // 004822f1  8b460c                 -mov eax, dword ptr [esi + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(12) /* 0xc */);
    // 004822f4  8b0df8347d00           -mov ecx, dword ptr [0x7d34f8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205560) /* 0x7d34f8 */);
    // 004822fa  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 004822ff  7558                   -jne 0x482359
    if (!cpu.flags.zf)
    {
        goto L_0x00482359;
    }
    // 00482301  39c8                   +cmp eax, ecx
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
    // 00482303  7d54                   -jge 0x482359
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00482359;
    }
    // 00482305  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 00482308  bb00000000             -mov ebx, 0
    cpu.ebx = 0 /*0x0*/;
    // 0048230d  8b0d04357d00           -mov ecx, dword ptr [0x7d3504]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205572) /* 0x7d3504 */);
    // 00482313  8b1500357d00           -mov edx, dword ptr [0x7d3500]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205568) /* 0x7d3500 */);
    // 00482319  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 0048231e  750d                   -jne 0x48232d
    if (!cpu.flags.zf)
    {
        goto L_0x0048232d;
    }
    // 00482320  39c8                   +cmp eax, ecx
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
    // 00482322  7c09                   -jl 0x48232d
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0048232d;
    }
    // 00482324  39d0                   +cmp eax, edx
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
    // 00482326  7e09                   -jle 0x482331
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00482331;
    }
    // 00482328  83cb04                 +or ebx, 4
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(4 /*0x4*/))));
    // 0048232b  eb04                   -jmp 0x482331
    goto L_0x00482331;
L_0x0048232d:
    // 0048232d  83cb08                 -or ebx, 8
    cpu.ebx |= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 00482330  90                     -nop 
    ;
L_0x00482331:
    // 00482331  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 00482333  90                     -nop 
    ;
    // 00482334  8b0dfc347d00           -mov ecx, dword ptr [0x7d34fc]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205564) /* 0x7d34fc */);
    // 0048233a  8b150c357d00           -mov edx, dword ptr [0x7d350c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205580) /* 0x7d350c */);
    // 00482340  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 00482345  750d                   -jne 0x482354
    if (!cpu.flags.zf)
    {
        goto L_0x00482354;
    }
    // 00482347  39c8                   +cmp eax, ecx
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
    // 00482349  7c09                   -jl 0x482354
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00482354;
    }
    // 0048234b  39d0                   +cmp eax, edx
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
    // 0048234d  7e0f                   -jle 0x48235e
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0048235e;
    }
    // 0048234f  83cb02                 +or ebx, 2
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(2 /*0x2*/))));
    // 00482352  eb0a                   -jmp 0x48235e
    goto L_0x0048235e;
L_0x00482354:
    // 00482354  83cb01                 +or ebx, 1
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(1 /*0x1*/))));
    // 00482357  eb05                   -jmp 0x48235e
    goto L_0x0048235e;
L_0x00482359:
    // 00482359  bb10000000             -mov ebx, 0x10
    cpu.ebx = 16 /*0x10*/;
L_0x0048235e:
    // 0048235e  881f                   -mov byte ptr [edi], bl
    app->getMemory<x86::reg8>(cpu.edi) = cpu.bl;
    // 00482360  8b45e0                 -mov eax, dword ptr [ebp - 0x20]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 00482363  8b7de0                 -mov edi, dword ptr [ebp - 0x20]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 00482366  83c060                 -add eax, 0x60
    (cpu.eax) += x86::reg32(x86::sreg32(96 /*0x60*/));
    // 00482369  83c774                 -add edi, 0x74
    (cpu.edi) += x86::reg32(x86::sreg32(116 /*0x74*/));
    // 0048236c  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0048236e  8945ec                 -mov dword ptr [ebp - 0x14], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.eax;
    // 00482371  8b460c                 -mov eax, dword ptr [esi + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(12) /* 0xc */);
    // 00482374  8b0df8347d00           -mov ecx, dword ptr [0x7d34f8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205560) /* 0x7d34f8 */);
    // 0048237a  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 0048237f  7558                   -jne 0x4823d9
    if (!cpu.flags.zf)
    {
        goto L_0x004823d9;
    }
    // 00482381  39c8                   +cmp eax, ecx
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
    // 00482383  7d54                   -jge 0x4823d9
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004823d9;
    }
    // 00482385  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 00482388  bb00000000             -mov ebx, 0
    cpu.ebx = 0 /*0x0*/;
    // 0048238d  8b0d04357d00           -mov ecx, dword ptr [0x7d3504]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205572) /* 0x7d3504 */);
    // 00482393  8b1500357d00           -mov edx, dword ptr [0x7d3500]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205568) /* 0x7d3500 */);
    // 00482399  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 0048239e  750d                   -jne 0x4823ad
    if (!cpu.flags.zf)
    {
        goto L_0x004823ad;
    }
    // 004823a0  39c8                   +cmp eax, ecx
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
    // 004823a2  7c09                   -jl 0x4823ad
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004823ad;
    }
    // 004823a4  39d0                   +cmp eax, edx
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
    // 004823a6  7e09                   -jle 0x4823b1
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004823b1;
    }
    // 004823a8  83cb04                 +or ebx, 4
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(4 /*0x4*/))));
    // 004823ab  eb04                   -jmp 0x4823b1
    goto L_0x004823b1;
L_0x004823ad:
    // 004823ad  83cb08                 -or ebx, 8
    cpu.ebx |= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 004823b0  90                     -nop 
    ;
L_0x004823b1:
    // 004823b1  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 004823b3  90                     -nop 
    ;
    // 004823b4  8b0dfc347d00           -mov ecx, dword ptr [0x7d34fc]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205564) /* 0x7d34fc */);
    // 004823ba  8b150c357d00           -mov edx, dword ptr [0x7d350c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205580) /* 0x7d350c */);
    // 004823c0  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 004823c5  750d                   -jne 0x4823d4
    if (!cpu.flags.zf)
    {
        goto L_0x004823d4;
    }
    // 004823c7  39c8                   +cmp eax, ecx
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
    // 004823c9  7c09                   -jl 0x4823d4
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004823d4;
    }
    // 004823cb  39d0                   +cmp eax, edx
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
    // 004823cd  7e0f                   -jle 0x4823de
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004823de;
    }
    // 004823cf  83cb02                 +or ebx, 2
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(2 /*0x2*/))));
    // 004823d2  eb0a                   -jmp 0x4823de
    goto L_0x004823de;
L_0x004823d4:
    // 004823d4  83cb01                 +or ebx, 1
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(1 /*0x1*/))));
    // 004823d7  eb05                   -jmp 0x4823de
    goto L_0x004823de;
L_0x004823d9:
    // 004823d9  bb10000000             -mov ebx, 0x10
    cpu.ebx = 16 /*0x10*/;
L_0x004823de:
    // 004823de  881f                   -mov byte ptr [edi], bl
    app->getMemory<x86::reg8>(cpu.edi) = cpu.bl;
    // 004823e0  8b7dfc                 -mov edi, dword ptr [ebp - 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004823e3  8b75fc                 -mov esi, dword ptr [ebp - 4]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004823e6  83c714                 -add edi, 0x14
    (cpu.edi) += x86::reg32(x86::sreg32(20 /*0x14*/));
    // 004823e9  8b460c                 -mov eax, dword ptr [esi + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(12) /* 0xc */);
    // 004823ec  8b0df8347d00           -mov ecx, dword ptr [0x7d34f8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205560) /* 0x7d34f8 */);
    // 004823f2  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 004823f7  7558                   -jne 0x482451
    if (!cpu.flags.zf)
    {
        goto L_0x00482451;
    }
    // 004823f9  39c8                   +cmp eax, ecx
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
    // 004823fb  7d54                   -jge 0x482451
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00482451;
    }
    // 004823fd  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 00482400  bb00000000             -mov ebx, 0
    cpu.ebx = 0 /*0x0*/;
    // 00482405  8b0d04357d00           -mov ecx, dword ptr [0x7d3504]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205572) /* 0x7d3504 */);
    // 0048240b  8b1500357d00           -mov edx, dword ptr [0x7d3500]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205568) /* 0x7d3500 */);
    // 00482411  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 00482416  750d                   -jne 0x482425
    if (!cpu.flags.zf)
    {
        goto L_0x00482425;
    }
    // 00482418  39c8                   +cmp eax, ecx
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
    // 0048241a  7c09                   -jl 0x482425
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00482425;
    }
    // 0048241c  39d0                   +cmp eax, edx
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
    // 0048241e  7e09                   -jle 0x482429
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00482429;
    }
    // 00482420  83cb04                 +or ebx, 4
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(4 /*0x4*/))));
    // 00482423  eb04                   -jmp 0x482429
    goto L_0x00482429;
L_0x00482425:
    // 00482425  83cb08                 -or ebx, 8
    cpu.ebx |= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 00482428  90                     -nop 
    ;
L_0x00482429:
    // 00482429  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 0048242b  90                     -nop 
    ;
    // 0048242c  8b0dfc347d00           -mov ecx, dword ptr [0x7d34fc]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205564) /* 0x7d34fc */);
    // 00482432  8b150c357d00           -mov edx, dword ptr [0x7d350c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205580) /* 0x7d350c */);
    // 00482438  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 0048243d  750d                   -jne 0x48244c
    if (!cpu.flags.zf)
    {
        goto L_0x0048244c;
    }
    // 0048243f  39c8                   +cmp eax, ecx
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
    // 00482441  7c09                   -jl 0x48244c
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0048244c;
    }
    // 00482443  39d0                   +cmp eax, edx
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
    // 00482445  7e0f                   -jle 0x482456
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00482456;
    }
    // 00482447  83cb02                 +or ebx, 2
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(2 /*0x2*/))));
    // 0048244a  eb0a                   -jmp 0x482456
    goto L_0x00482456;
L_0x0048244c:
    // 0048244c  83cb01                 +or ebx, 1
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(1 /*0x1*/))));
    // 0048244f  eb05                   -jmp 0x482456
    goto L_0x00482456;
L_0x00482451:
    // 00482451  bb10000000             -mov ebx, 0x10
    cpu.ebx = 16 /*0x10*/;
L_0x00482456:
    // 00482456  881f                   -mov byte ptr [edi], bl
    app->getMemory<x86::reg8>(cpu.edi) = cpu.bl;
    // 00482458  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0048245b  8b7dfc                 -mov edi, dword ptr [ebp - 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0048245e  83c020                 -add eax, 0x20
    (cpu.eax) += x86::reg32(x86::sreg32(32 /*0x20*/));
    // 00482461  83c734                 -add edi, 0x34
    (cpu.edi) += x86::reg32(x86::sreg32(52 /*0x34*/));
    // 00482464  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00482466  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
    // 00482469  8b460c                 -mov eax, dword ptr [esi + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(12) /* 0xc */);
    // 0048246c  8b0df8347d00           -mov ecx, dword ptr [0x7d34f8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205560) /* 0x7d34f8 */);
    // 00482472  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 00482477  7558                   -jne 0x4824d1
    if (!cpu.flags.zf)
    {
        goto L_0x004824d1;
    }
    // 00482479  39c8                   +cmp eax, ecx
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
    // 0048247b  7d54                   -jge 0x4824d1
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004824d1;
    }
    // 0048247d  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 00482480  bb00000000             -mov ebx, 0
    cpu.ebx = 0 /*0x0*/;
    // 00482485  8b0d04357d00           -mov ecx, dword ptr [0x7d3504]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205572) /* 0x7d3504 */);
    // 0048248b  8b1500357d00           -mov edx, dword ptr [0x7d3500]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205568) /* 0x7d3500 */);
    // 00482491  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 00482496  750d                   -jne 0x4824a5
    if (!cpu.flags.zf)
    {
        goto L_0x004824a5;
    }
    // 00482498  39c8                   +cmp eax, ecx
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
    // 0048249a  7c09                   -jl 0x4824a5
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004824a5;
    }
    // 0048249c  39d0                   +cmp eax, edx
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
    // 0048249e  7e09                   -jle 0x4824a9
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004824a9;
    }
    // 004824a0  83cb04                 +or ebx, 4
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(4 /*0x4*/))));
    // 004824a3  eb04                   -jmp 0x4824a9
    goto L_0x004824a9;
L_0x004824a5:
    // 004824a5  83cb08                 -or ebx, 8
    cpu.ebx |= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 004824a8  90                     -nop 
    ;
L_0x004824a9:
    // 004824a9  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 004824ab  90                     -nop 
    ;
    // 004824ac  8b0dfc347d00           -mov ecx, dword ptr [0x7d34fc]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205564) /* 0x7d34fc */);
    // 004824b2  8b150c357d00           -mov edx, dword ptr [0x7d350c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205580) /* 0x7d350c */);
    // 004824b8  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 004824bd  750d                   -jne 0x4824cc
    if (!cpu.flags.zf)
    {
        goto L_0x004824cc;
    }
    // 004824bf  39c8                   +cmp eax, ecx
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
    // 004824c1  7c09                   -jl 0x4824cc
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004824cc;
    }
    // 004824c3  39d0                   +cmp eax, edx
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
    // 004824c5  7e0f                   -jle 0x4824d6
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004824d6;
    }
    // 004824c7  83cb02                 +or ebx, 2
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(2 /*0x2*/))));
    // 004824ca  eb0a                   -jmp 0x4824d6
    goto L_0x004824d6;
L_0x004824cc:
    // 004824cc  83cb01                 +or ebx, 1
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(1 /*0x1*/))));
    // 004824cf  eb05                   -jmp 0x4824d6
    goto L_0x004824d6;
L_0x004824d1:
    // 004824d1  bb10000000             -mov ebx, 0x10
    cpu.ebx = 16 /*0x10*/;
L_0x004824d6:
    // 004824d6  881f                   -mov byte ptr [edi], bl
    app->getMemory<x86::reg8>(cpu.edi) = cpu.bl;
    // 004824d8  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004824db  8b7dfc                 -mov edi, dword ptr [ebp - 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004824de  83c040                 -add eax, 0x40
    (cpu.eax) += x86::reg32(x86::sreg32(64 /*0x40*/));
    // 004824e1  83c754                 -add edi, 0x54
    (cpu.edi) += x86::reg32(x86::sreg32(84 /*0x54*/));
    // 004824e4  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004824e6  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 004824e9  8b460c                 -mov eax, dword ptr [esi + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(12) /* 0xc */);
    // 004824ec  8b0df8347d00           -mov ecx, dword ptr [0x7d34f8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205560) /* 0x7d34f8 */);
    // 004824f2  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 004824f7  7558                   -jne 0x482551
    if (!cpu.flags.zf)
    {
        goto L_0x00482551;
    }
    // 004824f9  39c8                   +cmp eax, ecx
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
    // 004824fb  7d54                   -jge 0x482551
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00482551;
    }
    // 004824fd  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 00482500  bb00000000             -mov ebx, 0
    cpu.ebx = 0 /*0x0*/;
    // 00482505  8b0d04357d00           -mov ecx, dword ptr [0x7d3504]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205572) /* 0x7d3504 */);
    // 0048250b  8b1500357d00           -mov edx, dword ptr [0x7d3500]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205568) /* 0x7d3500 */);
    // 00482511  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 00482516  750d                   -jne 0x482525
    if (!cpu.flags.zf)
    {
        goto L_0x00482525;
    }
    // 00482518  39c8                   +cmp eax, ecx
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
    // 0048251a  7c09                   -jl 0x482525
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00482525;
    }
    // 0048251c  39d0                   +cmp eax, edx
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
    // 0048251e  7e09                   -jle 0x482529
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00482529;
    }
    // 00482520  83cb04                 +or ebx, 4
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(4 /*0x4*/))));
    // 00482523  eb04                   -jmp 0x482529
    goto L_0x00482529;
L_0x00482525:
    // 00482525  83cb08                 -or ebx, 8
    cpu.ebx |= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 00482528  90                     -nop 
    ;
L_0x00482529:
    // 00482529  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 0048252b  90                     -nop 
    ;
    // 0048252c  8b0dfc347d00           -mov ecx, dword ptr [0x7d34fc]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205564) /* 0x7d34fc */);
    // 00482532  8b150c357d00           -mov edx, dword ptr [0x7d350c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205580) /* 0x7d350c */);
    // 00482538  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 0048253d  750d                   -jne 0x48254c
    if (!cpu.flags.zf)
    {
        goto L_0x0048254c;
    }
    // 0048253f  39c8                   +cmp eax, ecx
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
    // 00482541  7c09                   -jl 0x48254c
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0048254c;
    }
    // 00482543  39d0                   +cmp eax, edx
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
    // 00482545  7e0f                   -jle 0x482556
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00482556;
    }
    // 00482547  83cb02                 +or ebx, 2
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(2 /*0x2*/))));
    // 0048254a  eb0a                   -jmp 0x482556
    goto L_0x00482556;
L_0x0048254c:
    // 0048254c  83cb01                 +or ebx, 1
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(1 /*0x1*/))));
    // 0048254f  eb05                   -jmp 0x482556
    goto L_0x00482556;
L_0x00482551:
    // 00482551  bb10000000             -mov ebx, 0x10
    cpu.ebx = 16 /*0x10*/;
L_0x00482556:
    // 00482556  881f                   -mov byte ptr [edi], bl
    app->getMemory<x86::reg8>(cpu.edi) = cpu.bl;
    // 00482558  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0048255b  8b7dfc                 -mov edi, dword ptr [ebp - 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0048255e  83c060                 -add eax, 0x60
    (cpu.eax) += x86::reg32(x86::sreg32(96 /*0x60*/));
    // 00482561  83c774                 -add edi, 0x74
    (cpu.edi) += x86::reg32(x86::sreg32(116 /*0x74*/));
    // 00482564  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00482566  8945f0                 -mov dword ptr [ebp - 0x10], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.eax;
    // 00482569  8b460c                 -mov eax, dword ptr [esi + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(12) /* 0xc */);
    // 0048256c  8b0df8347d00           -mov ecx, dword ptr [0x7d34f8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205560) /* 0x7d34f8 */);
    // 00482572  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 00482577  7558                   -jne 0x4825d1
    if (!cpu.flags.zf)
    {
        goto L_0x004825d1;
    }
    // 00482579  39c8                   +cmp eax, ecx
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
    // 0048257b  7d54                   -jge 0x4825d1
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004825d1;
    }
    // 0048257d  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 00482580  bb00000000             -mov ebx, 0
    cpu.ebx = 0 /*0x0*/;
    // 00482585  8b0d04357d00           -mov ecx, dword ptr [0x7d3504]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205572) /* 0x7d3504 */);
    // 0048258b  8b1500357d00           -mov edx, dword ptr [0x7d3500]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205568) /* 0x7d3500 */);
    // 00482591  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 00482596  750d                   -jne 0x4825a5
    if (!cpu.flags.zf)
    {
        goto L_0x004825a5;
    }
    // 00482598  39c8                   +cmp eax, ecx
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
    // 0048259a  7c09                   -jl 0x4825a5
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004825a5;
    }
    // 0048259c  39d0                   +cmp eax, edx
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
    // 0048259e  7e09                   -jle 0x4825a9
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004825a9;
    }
    // 004825a0  83cb04                 +or ebx, 4
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(4 /*0x4*/))));
    // 004825a3  eb04                   -jmp 0x4825a9
    goto L_0x004825a9;
L_0x004825a5:
    // 004825a5  83cb08                 -or ebx, 8
    cpu.ebx |= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 004825a8  90                     -nop 
    ;
L_0x004825a9:
    // 004825a9  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 004825ab  90                     -nop 
    ;
    // 004825ac  8b0dfc347d00           -mov ecx, dword ptr [0x7d34fc]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205564) /* 0x7d34fc */);
    // 004825b2  8b150c357d00           -mov edx, dword ptr [0x7d350c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205580) /* 0x7d350c */);
    // 004825b8  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 004825bd  750d                   -jne 0x4825cc
    if (!cpu.flags.zf)
    {
        goto L_0x004825cc;
    }
    // 004825bf  39c8                   +cmp eax, ecx
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
    // 004825c1  7c09                   -jl 0x4825cc
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004825cc;
    }
    // 004825c3  39d0                   +cmp eax, edx
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
    // 004825c5  7e0f                   -jle 0x4825d6
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004825d6;
    }
    // 004825c7  83cb02                 +or ebx, 2
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(2 /*0x2*/))));
    // 004825ca  eb0a                   -jmp 0x4825d6
    goto L_0x004825d6;
L_0x004825cc:
    // 004825cc  83cb01                 +or ebx, 1
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(1 /*0x1*/))));
    // 004825cf  eb05                   -jmp 0x4825d6
    goto L_0x004825d6;
L_0x004825d1:
    // 004825d1  bb10000000             -mov ebx, 0x10
    cpu.ebx = 16 /*0x10*/;
L_0x004825d6:
    // 004825d6  881f                   -mov byte ptr [edi], bl
    app->getMemory<x86::reg8>(cpu.edi) = cpu.bl;
    // 004825d8  8b4dec                 -mov ecx, dword ptr [ebp - 0x14]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004825db  8b5de8                 -mov ebx, dword ptr [ebp - 0x18]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004825de  8b55e4                 -mov edx, dword ptr [ebp - 0x1c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 004825e1  8b45e0                 -mov eax, dword ptr [ebp - 0x20]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 004825e4  e8f7ec0300             -call 0x4c12e0
    cpu.esp -= 4;
    nfs3hp::vertices::clipQuad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004825e9  8b4df0                 -mov ecx, dword ptr [ebp - 0x10]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004825ec  8b5df8                 -mov ebx, dword ptr [ebp - 8]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004825ef  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004825f2  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004825f5  e8e6ec0300             -call 0x4c12e0
    cpu.esp -= 4;
    nfs3hp::vertices::clipQuad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004825fa:
    // 004825fa  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004825fc  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004825fd  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004825fe  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004825ff  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00482600  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00482601  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00482602  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_482610: native, nfs3hp::dead::stub_0x482610 */

/* sub_482620: native, nfs3hp::render::countdown */

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_482880(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 00482880  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00482881  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00482882  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00482883  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00482884  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00482885  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00482887  8b1554d97c00           -mov edx, dword ptr [0x7cd954]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8182100) /* 0x7cd954 */);
    // 0048288d  83fa08                 +cmp edx, 8
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(8 /*0x8*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00482890  7744                   -ja 0x4828d6
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x004828d6;
    }
    // 00482892  ff249548284800         -jmp dword ptr [edx*4 + 0x482848]
    cpu.ip = app->getMemory<x86::reg32>(4728904 + cpu.edx * 4); goto dynamic_jump;
  case 0x00482899:
    // 00482899  b80c000000             -mov eax, 0xc
    cpu.eax = 12 /*0xc*/;
    // 0048289e  eb36                   -jmp 0x4828d6
    goto L_0x004828d6;
  case 0x004828a0:
    // 004828a0  b80d000000             -mov eax, 0xd
    cpu.eax = 13 /*0xd*/;
    // 004828a5  eb2f                   -jmp 0x4828d6
    goto L_0x004828d6;
  case 0x004828a7:
    // 004828a7  b80e000000             -mov eax, 0xe
    cpu.eax = 14 /*0xe*/;
    // 004828ac  eb28                   -jmp 0x4828d6
    goto L_0x004828d6;
  case 0x004828ae:
    // 004828ae  b80f000000             -mov eax, 0xf
    cpu.eax = 15 /*0xf*/;
    // 004828b3  eb21                   -jmp 0x4828d6
    goto L_0x004828d6;
  case 0x004828b5:
    // 004828b5  b810000000             -mov eax, 0x10
    cpu.eax = 16 /*0x10*/;
    // 004828ba  eb1a                   -jmp 0x4828d6
    goto L_0x004828d6;
  case 0x004828bc:
    // 004828bc  b811000000             -mov eax, 0x11
    cpu.eax = 17 /*0x11*/;
    // 004828c1  eb13                   -jmp 0x4828d6
    goto L_0x004828d6;
  case 0x004828c3:
    // 004828c3  b812000000             -mov eax, 0x12
    cpu.eax = 18 /*0x12*/;
    // 004828c8  eb0c                   -jmp 0x4828d6
    goto L_0x004828d6;
  case 0x004828ca:
    // 004828ca  b813000000             -mov eax, 0x13
    cpu.eax = 19 /*0x13*/;
    // 004828cf  eb05                   -jmp 0x4828d6
    goto L_0x004828d6;
  case 0x004828d1:
    // 004828d1  b814000000             -mov eax, 0x14
    cpu.eax = 20 /*0x14*/;
L_0x004828d6:
    // 004828d6  8b1548d97c00           -mov edx, dword ptr [0x7cd948]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8182088) /* 0x7cd948 */);
    // 004828dc  be03000000             -mov esi, 3
    cpu.esi = 3 /*0x3*/;
    // 004828e1  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004828e3  7407                   -je 0x4828ec
    if (cpu.flags.zf)
    {
        goto L_0x004828ec;
    }
    // 004828e5  ba05000000             -mov edx, 5
    cpu.edx = 5 /*0x5*/;
    // 004828ea  eb05                   -jmp 0x4828f1
    goto L_0x004828f1;
L_0x004828ec:
    // 004828ec  ba04000000             -mov edx, 4
    cpu.edx = 4 /*0x4*/;
L_0x004828f1:
    // 004828f1  8b0d4cd97c00           -mov ecx, dword ptr [0x7cd94c]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8182092) /* 0x7cd94c */);
    // 004828f7  83f903                 +cmp ecx, 3
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
    // 004828fa  7721                   -ja 0x48291d
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x0048291d;
    }
    // 004828fc  ff248d6c284800         -jmp dword ptr [ecx*4 + 0x48286c]
    cpu.ip = app->getMemory<x86::reg32>(4728940 + cpu.ecx * 4); goto dynamic_jump;
  case 0x00482903:
    // 00482903  bb06000000             -mov ebx, 6
    cpu.ebx = 6 /*0x6*/;
    // 00482908  eb13                   -jmp 0x48291d
    goto L_0x0048291d;
  case 0x0048290a:
    // 0048290a  bb07000000             -mov ebx, 7
    cpu.ebx = 7 /*0x7*/;
    // 0048290f  eb0c                   -jmp 0x48291d
    goto L_0x0048291d;
  case 0x00482911:
    // 00482911  bb08000000             -mov ebx, 8
    cpu.ebx = 8 /*0x8*/;
    // 00482916  eb05                   -jmp 0x48291d
    goto L_0x0048291d;
  case 0x00482918:
    // 00482918  bb09000000             -mov ebx, 9
    cpu.ebx = 9 /*0x9*/;
L_0x0048291d:
    // 0048291d  8b0d58d97c00           -mov ecx, dword ptr [0x7cd958]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8182104) /* 0x7cd958 */);
    // 00482923  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00482924  89f1                   -mov ecx, esi
    cpu.ecx = cpu.esi;
    // 00482926  e8959e0000             -call 0x48c7c0
    cpu.esp -= 4;
    nfs3hp::render::startLights(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0048292b  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0048292c  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0048292d  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0048292e  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0048292f  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00482930  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* sub_482940: native, nfs3hp::graphics::hudMessageBox */

/* sub_482b00: native, nfs3hp::render::hud */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_484030(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00484030  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00484031  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00484033  b848027000             -mov eax, 0x700248
    cpu.eax = 7340616 /*0x700248*/;
    // 00484038  e8037e0100             -call 0x49be40
    cpu.esp -= 4;
    nfs3hp::port::text::flushQuads(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0048403d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0048403e  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_484040: native, nfs3hp::port::hud::loadingTextPrepared */

/* sub_484070: native, nfs3hp::port::hud::hudStarted */

/* sub_484220: native, nfs3hp::frontend::hudClearRebuildFlag */

/* sub_484230: native, nfs3hp::dead::stub_0x484230 */

/* sub_484260: native, nfs3hp::frontend::hudToggleShown */

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_484330(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00484330  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00484331  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00484332  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00484333  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00484335  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00484337  8b0db0d36f00           -mov ecx, dword ptr [0x6fd3b0]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
    // 0048433d  83f901                 +cmp ecx, 1
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
    // 00484340  7504                   -jne 0x484346
    if (!cpu.flags.zf)
    {
        goto L_0x00484346;
    }
    // 00484342  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00484344  eb02                   -jmp 0x484348
    goto L_0x00484348;
L_0x00484346:
    // 00484346  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x00484348:
    // 00484348  8d0c02                 -lea ecx, [edx + eax]
    cpu.ecx = x86::reg32(cpu.edx + cpu.eax * 1);
    // 0048434b  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0048434d  e87ee8f9ff             -call 0x422bd0
    cpu.esp -= 4;
    nfs3hp::carstate::cameraTarget(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00484352  f6800002000020         +test byte ptr [eax + 0x200], 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(512) /* 0x200 */) & 32 /*0x20*/));
    // 00484359  7407                   -je 0x484362
    if (cpu.flags.zf)
    {
        goto L_0x00484362;
    }
    // 0048435b  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00484360  eb02                   -jmp 0x484364
    goto L_0x00484364;
L_0x00484362:
    // 00484362  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x00484364:
    // 00484364  69c0a4010000           -imul eax, eax, 0x1a4
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(420 /*0x1a4*/)));
    // 0048436a  69c948030000           -imul ecx, ecx, 0x348
    cpu.ecx = x86::reg32(x86::sreg64(x86::sreg32(cpu.ecx)) * x86::sreg64(x86::sreg32(840 /*0x348*/)));
    // 00484370  83bc0150bc6f0000       +cmp dword ptr [ecx + eax + 0x6fbc50], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(7322704) /* 0x6fbc50 */ + cpu.eax * 1);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00484378  7507                   -jne 0x484381
    if (!cpu.flags.zf)
    {
        goto L_0x00484381;
    }
    // 0048437a  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0048437c  e8dffeffff             -call 0x484260
    cpu.esp -= 4;
    nfs3hp::frontend::hudToggleShown(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00484381:
    // 00484381  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00484382  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00484383  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00484384  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_484390: native, nfs3hp::dead::stub_0x484390 */

/* sub_4843f0: native, nfs3hp::dead::stub_0x4843f0 */

/* sub_484450: native, nfs3hp::frontend::hudToggleRearView */

/* sub_4844c0: native, nfs3hp::port::hud::hudPaneSet */

/* sub_484b70: native, nfs3hp::port::hud::screenScalesSet */

/* sub_484c10: native, nfs3hp::port::hud::hudTablesStarted */

/* sub_485180: native, nfs3hp::port::hud::labelTextSet */

/* sub_4853c0: native, nfs3hp::frontend::hudRebuildElement */

/* sub_485640: native, nfs3hp::frontend::hudRebuildView */

/* sub_485680: native, nfs3hp::frontend::hudEditLoadDefaults */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x54 0x22 0x00 */
void Application::sub_485880(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00485880  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00485881  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00485882  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00485883  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00485884  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00485885  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00485887  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0048588c  e8effdffff             -call 0x485680
    cpu.esp -= 4;
    nfs3hp::frontend::hudEditLoadDefaults(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00485891  8915b8bb6f00           -mov dword ptr [0x6fbbb8], edx
    app->getMemory<x86::reg32>(x86::reg32(7322552) /* 0x6fbbb8 */) = cpu.edx;
    // 00485897  8915bcbb6f00           -mov dword ptr [0x6fbbbc], edx
    app->getMemory<x86::reg32>(x86::reg32(7322556) /* 0x6fbbbc */) = cpu.edx;
    // 0048589d  891584c46f00           -mov dword ptr [0x6fc484], edx
    app->getMemory<x86::reg32>(x86::reg32(7324804) /* 0x6fc484 */) = cpu.edx;
    // 004858a3  8915e0c26f00           -mov dword ptr [0x6fc2e0], edx
    app->getMemory<x86::reg32>(x86::reg32(7324384) /* 0x6fc2e0 */) = cpu.edx;
    // 004858a9  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 004858ab  89153cc16f00           -mov dword ptr [0x6fc13c], edx
    app->getMemory<x86::reg32>(x86::reg32(7323964) /* 0x6fc13c */) = cpu.edx;
    // 004858b1  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004858b3  891598bf6f00           -mov dword ptr [0x6fbf98], edx
    app->getMemory<x86::reg32>(x86::reg32(7323544) /* 0x6fbf98 */) = cpu.edx;
    // 004858b9  8915f4bd6f00           -mov dword ptr [0x6fbdf4], edx
    app->getMemory<x86::reg32>(x86::reg32(7323124) /* 0x6fbdf4 */) = cpu.edx;
    // 004858bf  891550bc6f00           -mov dword ptr [0x6fbc50], edx
    app->getMemory<x86::reg32>(x86::reg32(7322704) /* 0x6fbc50 */) = cpu.edx;
    // 004858c5  8915e4c26f00           -mov dword ptr [0x6fc2e4], edx
    app->getMemory<x86::reg32>(x86::reg32(7324388) /* 0x6fc2e4 */) = cpu.edx;
    // 004858cb  89159cbf6f00           -mov dword ptr [0x6fbf9c], edx
    app->getMemory<x86::reg32>(x86::reg32(7323548) /* 0x6fbf9c */) = cpu.edx;
    // 004858d1  31d7                   -xor edi, edx
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004858d3  31d0                   -xor eax, edx
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004858d5  893d88c46f00           -mov dword ptr [0x6fc488], edi
    app->getMemory<x86::reg32>(x86::reg32(7324808) /* 0x6fc488 */) = cpu.edi;
    // 004858db  a340c16f00             -mov dword ptr [0x6fc140], eax
    app->getMemory<x86::reg32>(x86::reg32(7323968) /* 0x6fc140 */) = cpu.eax;
    // 004858e0  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004858e2  8a25583a7a00           -mov ah, byte ptr [0x7a3a58]
    cpu.ah = app->getMemory<x86::reg8>(x86::reg32(8010328) /* 0x7a3a58 */);
    // 004858e8  8915f8bd6f00           -mov dword ptr [0x6fbdf8], edx
    app->getMemory<x86::reg32>(x86::reg32(7323128) /* 0x6fbdf8 */) = cpu.edx;
    // 004858ee  89158cc46f00           -mov dword ptr [0x6fc48c], edx
    app->getMemory<x86::reg32>(x86::reg32(7324812) /* 0x6fc48c */) = cpu.edx;
    // 004858f4  891544c16f00           -mov dword ptr [0x6fc144], edx
    app->getMemory<x86::reg32>(x86::reg32(7323972) /* 0x6fc144 */) = cpu.edx;
    // 004858fa  8915e8c26f00           -mov dword ptr [0x6fc2e8], edx
    app->getMemory<x86::reg32>(x86::reg32(7324392) /* 0x6fc2e8 */) = cpu.edx;
    // 00485900  8915a0bf6f00           -mov dword ptr [0x6fbfa0], edx
    app->getMemory<x86::reg32>(x86::reg32(7323552) /* 0x6fbfa0 */) = cpu.edx;
    // 00485906  891590c46f00           -mov dword ptr [0x6fc490], edx
    app->getMemory<x86::reg32>(x86::reg32(7324816) /* 0x6fc490 */) = cpu.edx;
    // 0048590c  8915ecc26f00           -mov dword ptr [0x6fc2ec], edx
    app->getMemory<x86::reg32>(x86::reg32(7324396) /* 0x6fc2ec */) = cpu.edx;
    // 00485912  891548c16f00           -mov dword ptr [0x6fc148], edx
    app->getMemory<x86::reg32>(x86::reg32(7323976) /* 0x6fc148 */) = cpu.edx;
    // 00485918  8915a4bf6f00           -mov dword ptr [0x6fbfa4], edx
    app->getMemory<x86::reg32>(x86::reg32(7323556) /* 0x6fbfa4 */) = cpu.edx;
    // 0048591e  f6c402                 +test ah, 2
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.ah & 2 /*0x2*/));
    // 00485921  7425                   -je 0x485948
    if (cpu.flags.zf)
    {
        goto L_0x00485948;
    }
    // 00485923  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 00485928  89155cbc6f00           -mov dword ptr [0x6fbc5c], edx
    app->getMemory<x86::reg32>(x86::reg32(7322716) /* 0x6fbc5c */) = cpu.edx;
    // 0048592e  8915fcbd6f00           -mov dword ptr [0x6fbdfc], edx
    app->getMemory<x86::reg32>(x86::reg32(7323132) /* 0x6fbdfc */) = cpu.edx;
    // 00485934  891558bc6f00           -mov dword ptr [0x6fbc58], edx
    app->getMemory<x86::reg32>(x86::reg32(7322712) /* 0x6fbc58 */) = cpu.edx;
    // 0048593a  891500be6f00           -mov dword ptr [0x6fbe00], edx
    app->getMemory<x86::reg32>(x86::reg32(7323136) /* 0x6fbe00 */) = cpu.edx;
    // 00485940  890d54bc6f00           -mov dword ptr [0x6fbc54], ecx
    app->getMemory<x86::reg32>(x86::reg32(7322708) /* 0x6fbc54 */) = cpu.ecx;
    // 00485946  eb23                   -jmp 0x48596b
    goto L_0x0048596b;
L_0x00485948:
    // 00485948  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 0048594d  891554bc6f00           -mov dword ptr [0x6fbc54], edx
    app->getMemory<x86::reg32>(x86::reg32(7322708) /* 0x6fbc54 */) = cpu.edx;
    // 00485953  891d5cbc6f00           -mov dword ptr [0x6fbc5c], ebx
    app->getMemory<x86::reg32>(x86::reg32(7322716) /* 0x6fbc5c */) = cpu.ebx;
    // 00485959  891dfcbd6f00           -mov dword ptr [0x6fbdfc], ebx
    app->getMemory<x86::reg32>(x86::reg32(7323132) /* 0x6fbdfc */) = cpu.ebx;
    // 0048595f  891d58bc6f00           -mov dword ptr [0x6fbc58], ebx
    app->getMemory<x86::reg32>(x86::reg32(7322712) /* 0x6fbc58 */) = cpu.ebx;
    // 00485965  891d00be6f00           -mov dword ptr [0x6fbe00], ebx
    app->getMemory<x86::reg32>(x86::reg32(7323136) /* 0x6fbe00 */) = cpu.ebx;
L_0x0048596b:
    // 0048596b  b900008040             -mov ecx, 0x40800000
    cpu.ecx = 1082130432 /*0x40800000*/;
    // 00485970  bf0000803f             -mov edi, 0x3f800000
    cpu.edi = 1065353216 /*0x3f800000*/;
    // 00485975  890da8bf6f00           -mov dword ptr [0x6fbfa8], ecx
    app->getMemory<x86::reg32>(x86::reg32(7323560) /* 0x6fbfa8 */) = cpu.ecx;
    // 0048597b  890d60bc6f00           -mov dword ptr [0x6fbc60], ecx
    app->getMemory<x86::reg32>(x86::reg32(7322720) /* 0x6fbc60 */) = cpu.ecx;
    // 00485981  890df0c26f00           -mov dword ptr [0x6fc2f0], ecx
    app->getMemory<x86::reg32>(x86::reg32(7324400) /* 0x6fc2f0 */) = cpu.ecx;
    // 00485987  893d94c46f00           -mov dword ptr [0x6fc494], edi
    app->getMemory<x86::reg32>(x86::reg32(7324820) /* 0x6fc494 */) = cpu.edi;
    // 0048598d  893d4cc16f00           -mov dword ptr [0x6fc14c], edi
    app->getMemory<x86::reg32>(x86::reg32(7323980) /* 0x6fc14c */) = cpu.edi;
    // 00485993  8b0d4cbb6f00           -mov ecx, dword ptr [0x6fbb4c]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7322444) /* 0x6fbb4c */);
    // 00485999  893d04be6f00           -mov dword ptr [0x6fbe04], edi
    app->getMemory<x86::reg32>(x86::reg32(7323140) /* 0x6fbe04 */) = cpu.edi;
    // 0048599f  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004859a1  7407                   -je 0x4859aa
    if (cpu.flags.zf)
    {
        goto L_0x004859aa;
    }
    // 004859a3  bb03000000             -mov ebx, 3
    cpu.ebx = 3 /*0x3*/;
    // 004859a8  eb05                   -jmp 0x4859af
    goto L_0x004859af;
L_0x004859aa:
    // 004859aa  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
L_0x004859af:
    // 004859af  891d0cbe6f00           -mov dword ptr [0x6fbe0c], ebx
    app->getMemory<x86::reg32>(x86::reg32(7323148) /* 0x6fbe0c */) = cpu.ebx;
    // 004859b5  891db0bf6f00           -mov dword ptr [0x6fbfb0], ebx
    app->getMemory<x86::reg32>(x86::reg32(7323568) /* 0x6fbfb0 */) = cpu.ebx;
    // 004859bb  891d54c16f00           -mov dword ptr [0x6fc154], ebx
    app->getMemory<x86::reg32>(x86::reg32(7323988) /* 0x6fc154 */) = cpu.ebx;
    // 004859c1  891df8c26f00           -mov dword ptr [0x6fc2f8], ebx
    app->getMemory<x86::reg32>(x86::reg32(7324408) /* 0x6fc2f8 */) = cpu.ebx;
    // 004859c7  891d9cc46f00           -mov dword ptr [0x6fc49c], ebx
    app->getMemory<x86::reg32>(x86::reg32(7324828) /* 0x6fc49c */) = cpu.ebx;
    // 004859cd  891d68bc6f00           -mov dword ptr [0x6fbc68], ebx
    app->getMemory<x86::reg32>(x86::reg32(7322728) /* 0x6fbc68 */) = cpu.ebx;
    // 004859d3  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 004859d8  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 004859dd  891dfcc26f00           -mov dword ptr [0x6fc2fc], ebx
    app->getMemory<x86::reg32>(x86::reg32(7324412) /* 0x6fc2fc */) = cpu.ebx;
    // 004859e3  891d58c16f00           -mov dword ptr [0x6fc158], ebx
    app->getMemory<x86::reg32>(x86::reg32(7323992) /* 0x6fc158 */) = cpu.ebx;
    // 004859e9  891db4bf6f00           -mov dword ptr [0x6fbfb4], ebx
    app->getMemory<x86::reg32>(x86::reg32(7323572) /* 0x6fbfb4 */) = cpu.ebx;
    // 004859ef  891d10be6f00           -mov dword ptr [0x6fbe10], ebx
    app->getMemory<x86::reg32>(x86::reg32(7323152) /* 0x6fbe10 */) = cpu.ebx;
    // 004859f5  891d6cbc6f00           -mov dword ptr [0x6fbc6c], ebx
    app->getMemory<x86::reg32>(x86::reg32(7322732) /* 0x6fbc6c */) = cpu.ebx;
    // 004859fb  891d00c36f00           -mov dword ptr [0x6fc300], ebx
    app->getMemory<x86::reg32>(x86::reg32(7324416) /* 0x6fc300 */) = cpu.ebx;
    // 00485a01  891db8bf6f00           -mov dword ptr [0x6fbfb8], ebx
    app->getMemory<x86::reg32>(x86::reg32(7323576) /* 0x6fbfb8 */) = cpu.ebx;
    // 00485a07  891d70bc6f00           -mov dword ptr [0x6fbc70], ebx
    app->getMemory<x86::reg32>(x86::reg32(7322736) /* 0x6fbc70 */) = cpu.ebx;
    // 00485a0d  891da0c46f00           -mov dword ptr [0x6fc4a0], ebx
    app->getMemory<x86::reg32>(x86::reg32(7324832) /* 0x6fc4a0 */) = cpu.ebx;
    // 00485a13  a3a4c46f00             -mov dword ptr [0x6fc4a4], eax
    app->getMemory<x86::reg32>(x86::reg32(7324836) /* 0x6fc4a4 */) = cpu.eax;
    // 00485a18  a35cc16f00             -mov dword ptr [0x6fc15c], eax
    app->getMemory<x86::reg32>(x86::reg32(7323996) /* 0x6fc15c */) = cpu.eax;
    // 00485a1d  bb03000000             -mov ebx, 3
    cpu.ebx = 3 /*0x3*/;
    // 00485a22  a314be6f00             -mov dword ptr [0x6fbe14], eax
    app->getMemory<x86::reg32>(x86::reg32(7323156) /* 0x6fbe14 */) = cpu.eax;
    // 00485a27  891d98c46f00           -mov dword ptr [0x6fc498], ebx
    app->getMemory<x86::reg32>(x86::reg32(7324824) /* 0x6fc498 */) = cpu.ebx;
    // 00485a2d  891df4c26f00           -mov dword ptr [0x6fc2f4], ebx
    app->getMemory<x86::reg32>(x86::reg32(7324404) /* 0x6fc2f4 */) = cpu.ebx;
    // 00485a33  891d50c16f00           -mov dword ptr [0x6fc150], ebx
    app->getMemory<x86::reg32>(x86::reg32(7323984) /* 0x6fc150 */) = cpu.ebx;
    // 00485a39  891dacbf6f00           -mov dword ptr [0x6fbfac], ebx
    app->getMemory<x86::reg32>(x86::reg32(7323564) /* 0x6fbfac */) = cpu.ebx;
    // 00485a3f  891d08be6f00           -mov dword ptr [0x6fbe08], ebx
    app->getMemory<x86::reg32>(x86::reg32(7323144) /* 0x6fbe08 */) = cpu.ebx;
    // 00485a45  891d64bc6f00           -mov dword ptr [0x6fbc64], ebx
    app->getMemory<x86::reg32>(x86::reg32(7322724) /* 0x6fbc64 */) = cpu.ebx;
    // 00485a4b  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00485a4c  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00485a4d  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00485a4e  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00485a4f  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00485a50  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_485a60: native, nfs3hp::port::hud::standingsGapMeasured */

/* sub_485c20: native, nfs3hp::port::hud::floatsSwapped */

/* sub_485c30: native, nfs3hp::port::hud::pointClipped */

/* sub_485f20: native, nfs3hp::port::hud::segmentClippedToRect */

/* sub_4860b0: native, nfs3hp::port::hud::segmentCutIntoPieces */

/* sub_486340: native, nfs3hp::port::hud::segmentsMet */

/* sub_486df0: native, nfs3hp::port::hud::floatsQuadSet */

/* sub_486f70: native, nfs3hp::port::hud::stackQuadSet */

/* sub_4870c0: native, nfs3hp::port::hud::rectangleQuadSet */

/* sub_487210: native, nfs3hp::render::hudInset */

/* sub_487340: native, nfs3hp::frontend::hudGroupClearCount */

/* sub_487350: native, nfs3hp::port::hud::fourIntegersAppended */

/* sub_4873c0: native, nfs3hp::frontend::hudGroupDropLast */

/* sub_4873e0: native, nfs3hp::port::hud::strokeClippedDrawn */

/* sub_487620: native, nfs3hp::port::hud::gaugeQuadClipped */

/* sub_487a30: native, nfs3hp::port::hud::boxQuadsSet */

/* sub_4880b0: native, nfs3hp::render::needles */

/* sub_488470: native, nfs3hp::render::barGauges */

/* sub_488a80: native, nfs3hp::graphics::loadCabinInstruments */

/* sub_489240: native, nfs3hp::render::cockpit */

/* sub_4896f0: native, nfs3hp::frontend::hudOrdinalLetters */

/* sub_489750: native, nfs3hp::frontend::hudFormatTime */

/* sub_4897f0: native, nfs3hp::port::hud::textAlignedDrawn */

/* sub_4898b0: native, nfs3hp::frontend::hudDrawTime */

/* sub_489900: native, nfs3hp::frontend::hudDrawPercentAndCount */

/* sub_4899d0: native, nfs3hp::frontend::hudDrawLaps */

/* sub_489a50: native, nfs3hp::frontend::hudDrawRatio */

/* sub_489ab0: native, nfs3hp::dead::stub_0x489ab0 */

/* sub_489b30: native, nfs3hp::port::hud::listRowsDrawn */

/* sub_489c40: native, nfs3hp::port::hud::timeTextDrawn */

/* sub_489d70: native, nfs3hp::port::hud::percentTextDrawn */

/* sub_489e00: native, nfs3hp::port::hud::indicatorQuadSet */

/* sub_48a220: native, nfs3hp::port::hud::indicatorGeometryUpdated */

/* sub_48a320: native, nfs3hp::frontend::hudDrawPlaceAmong */

/* sub_48a3a0: native, nfs3hp::port::hud::rankTextDrawn */

/* sub_48a660: native, nfs3hp::port::hud::labelRowsDrawn */

/* sub_48aa40: native, nfs3hp::port::hud::columnsStarted */

/* sub_48ae70: native, nfs3hp::port::hud::standingsDrawn */

/* sub_48b2a0: native, nfs3hp::frontend::hudDrawTriangle */

/* sub_48b4a0: native, nfs3hp::frontend::hudDrawLine */

/* sub_48b5f0: native, nfs3hp::port::hud::markersSet */

/* sub_48c0e0: native, nfs3hp::render::segmentMeter */

/* sub_48c7c0: native, nfs3hp::render::startLights */

/* sub_48cae0: native, nfs3hp::dead::stub_0x48cae0 */

/* sub_48caf0: native, nfs3hp::dead::stub_0x48caf0 */

/* sub_48cb50: native, nfs3hp::port::hud::pointTransformed */

/* sub_48cc60: native, nfs3hp::port::hud::boxDrawn */

/* sub_48ccf0: native, nfs3hp::frontend::hudRefreshPane */

/* sub_48cdb0: native, nfs3hp::port::hud::mapOutlineStarted */

/* sub_48d110: native, nfs3hp::port::hud::hudViewsSet */

/* sub_48d4c0: native, nfs3hp::port::hud::mapPointsDrawn */

/* sub_48e770: native, nfs3hp::frontend::hudFlushLines */

/* sub_48e780: native, nfs3hp::frontend::hudPrepareLines */

/* sub_48e790: native, nfs3hp::port::hud::viewPaneStepped */

/* sub_48e920: native, nfs3hp::frontend::hudStepMapPanes */

/* sub_48ea20: native, nfs3hp::port::hud::viewZoomStepped */

/* sub_48eb60: native, nfs3hp::frontend::hudStepZoom */

/* sub_48eb90: native, nfs3hp::port::timerThread */

}
