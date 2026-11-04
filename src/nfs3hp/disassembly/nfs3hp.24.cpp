#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{
// Port (tools/apply_loading_screen.py): defined in nfs3hp_main.cpp.
void loadingScreenFit(win32::WinApplication* app, x86::CPU& cpu, bool on);
// Port (tools/apply_race_state.py): defined in nfs3hp_main.cpp.
void raceRunning(bool running);

/* sub_4a1090: native, nfs3hp::port::conv::fn4a1090 */

/* sub_4a1210: native, nfs3hp::frontend::simTrackProbeWalls */

/* sub_4a1610: native, nfs3hp::frontend::simTrackPredictContact */

/* sub_4a1ce0: native, nfs3hp::frontend::simTrackReactToSurface */

/* sub_4a20c0: native, nfs3hp::port::conv::fn4a20c0 */

/* sub_4a21a0: native, nfs3hp::frontend::simTrackTunnelRoof */

/* sub_4a2260: native, nfs3hp::frontend::simTrackDownforceAndDrag */

/* sub_4a2630: native, nfs3hp::port::conv::fn4a2630 */

/* sub_4a2690: native, nfs3hp::frontend::simTrackSettle */

/* sub_4a2830: native, nfs3hp::port::conv::fn4a2830 */

/* sub_4a2bc0: native, nfs3hp::frontend::simTrackOffsetAcross */

/* sub_4a2c60: native, nfs3hp::port::conv::fn4a2c60 */

/* sub_4a2cc0: native, nfs3hp::frontend::simTrackTurnMatrix */

/* sub_4a2e20: native, nfs3hp::port::conv::fn4a2e20 */

/* sub_4a2f10: native, nfs3hp::frontend::simTrackSpinThirtySecond */

/* sub_4a2f30: native, nfs3hp::frontend::simTrackSpinSixtyFourth */

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
/* data blob: 6e6673332e6578650000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000 */
void Application::sub_4a3050(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004a3050  fc                     -cld 
    cpu.flags.df = 0;
    // 004a3051  c8530020               -enter 0x53, 0x20
    {
        cpu.esp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) = cpu.ebp;
        x86::reg32 esp = cpu.esp;
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) = esp;
        cpu.ebp = esp;
        cpu.esp -= 83;
    }
    // 004a3055  c9                     -leave 
    cpu.esp = cpu.ebp;
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a3056  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a3057  0040c9                 -add byte ptr [eax - 0x37], al
    (app->getMemory<x86::reg8>(cpu.eax + x86::reg32(-55) /* -0x37 */)) += x86::reg8(x86::sreg8(cpu.al));
    // 004a305a  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a305b  0060c9                 -add byte ptr [eax - 0x37], ah
    (app->getMemory<x86::reg8>(cpu.eax + x86::reg32(-55) /* -0x37 */)) += x86::reg8(x86::sreg8(cpu.ah));
    // 004a305e  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a305f  00e8                   -add al, ch
    (cpu.al) += x86::reg8(x86::sreg8(cpu.ch));
    // 004a3061  c9                     -leave 
    cpu.esp = cpu.ebp;
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a3062  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a3063  0084ca53008d80         -add byte ptr [edx + ecx*8 - 0x7f72ffad], al
    (app->getMemory<x86::reg8>(cpu.edx + x86::reg32(-2138242989) /* -0x7f72ffad */ + cpu.ecx * 8)) += x86::reg8(x86::sreg8(cpu.al));
    // 004a306a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004a306c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004a306e  8bd2                   -mov edx, edx
    cpu.edx = cpu.edx;
    // 004a3070  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a3071  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a3073  3d8e0000c0             +cmp eax, 0xc000008e
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3221225614 /*0xc000008e*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a3078  7260                   -jb 0x4a30da
    if (cpu.flags.cf)
    {
        goto L_0x004a30da;
    }
    // 004a307a  0f86df000000           -jbe 0x4a315f
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004a315f;
    }
    // 004a3080  3d930000c0             +cmp eax, 0xc0000093
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3221225619 /*0xc0000093*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a3085  7232                   -jb 0x4a30b9
    if (cpu.flags.cf)
    {
        goto L_0x004a30b9;
    }
    // 004a3087  0f86f5000000           -jbe 0x4a3182
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004a3182;
    }
    // 004a308d  3d950000c0             +cmp eax, 0xc0000095
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3221225621 /*0xc0000095*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a3092  0f82ff000000           -jb 0x4a3197
    if (cpu.flags.cf)
    {
        goto L_0x004a3197;
    }
    // 004a3098  0f8600010000           -jbe 0x4a319e
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004a319e;
    }
    // 004a309e  3d960000c0             +cmp eax, 0xc0000096
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3221225622 /*0xc0000096*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a30a3  0f860a010000           -jbe 0x4a31b3
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004a31b3;
    }
    // 004a30a9  3dfd0000c0             +cmp eax, 0xc00000fd
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3221225725 /*0xc00000fd*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a30ae  0f840d010000           -je 0x4a31c1
    if (cpu.flags.zf)
    {
        goto L_0x004a31c1;
    }
    // 004a30b4  e90f010000             -jmp 0x4a31c8
    goto L_0x004a31c8;
L_0x004a30b9:
    // 004a30b9  3d900000c0             +cmp eax, 0xc0000090
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3221225616 /*0xc0000090*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a30be  0f82a2000000           -jb 0x4a3166
    if (cpu.flags.cf)
    {
        goto L_0x004a3166;
    }
    // 004a30c4  0f86a3000000           -jbe 0x4a316d
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004a316d;
    }
    // 004a30ca  3d910000c0             +cmp eax, 0xc0000091
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3221225617 /*0xc0000091*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a30cf  0f869f000000           -jbe 0x4a3174
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004a3174;
    }
    // 004a30d5  e9a1000000             -jmp 0x4a317b
    goto L_0x004a317b;
L_0x004a30da:
    // 004a30da  3d1d0000c0             +cmp eax, 0xc000001d
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3221225501 /*0xc000001d*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a30df  7232                   -jb 0x4a3113
    if (cpu.flags.cf)
    {
        goto L_0x004a3113;
    }
    // 004a30e1  0f86a2000000           -jbe 0x4a3189
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004a3189;
    }
    // 004a30e7  3d260000c0             +cmp eax, 0xc0000026
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3221225510 /*0xc0000026*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a30ec  7215                   -jb 0x4a3103
    if (cpu.flags.cf)
    {
        goto L_0x004a3103;
    }
    // 004a30ee  0f86b1000000           -jbe 0x4a31a5
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004a31a5;
    }
    // 004a30f4  3d8c0000c0             +cmp eax, 0xc000008c
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3221225612 /*0xc000008c*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a30f9  0f82c9000000           -jb 0x4a31c8
    if (cpu.flags.cf)
    {
        goto L_0x004a31c8;
    }
    // 004a30ff  7642                   -jbe 0x4a3143
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004a3143;
    }
    // 004a3101  eb55                   -jmp 0x4a3158
    goto L_0x004a3158;
L_0x004a3103:
    // 004a3103  3d250000c0             +cmp eax, 0xc0000025
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3221225509 /*0xc0000025*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a3108  0f849e000000           -je 0x4a31ac
    if (cpu.flags.zf)
    {
        goto L_0x004a31ac;
    }
    // 004a310e  e9b5000000             -jmp 0x4a31c8
    goto L_0x004a31c8;
L_0x004a3113:
    // 004a3113  3d03000080             +cmp eax, 0x80000003
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2147483651 /*0x80000003*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a3118  721d                   -jb 0x4a3137
    if (cpu.flags.cf)
    {
        goto L_0x004a3137;
    }
    // 004a311a  762e                   -jbe 0x4a314a
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004a314a;
    }
    // 004a311c  3d04000080             +cmp eax, 0x80000004
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2147483652 /*0x80000004*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a3121  0f8693000000           -jbe 0x4a31ba
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004a31ba;
    }
    // 004a3127  3d060000c0             +cmp eax, 0xc0000006
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3221225478 /*0xc0000006*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a312c  0f845e000000           -je 0x4a3190
    if (cpu.flags.zf)
    {
        goto L_0x004a3190;
    }
    // 004a3132  e991000000             -jmp 0x4a31c8
    goto L_0x004a31c8;
L_0x004a3137:
    // 004a3137  3d02000080             +cmp eax, 0x80000002
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2147483650 /*0x80000002*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a313c  7413                   -je 0x4a3151
    if (cpu.flags.zf)
    {
        goto L_0x004a3151;
    }
    // 004a313e  e985000000             -jmp 0x4a31c8
    goto L_0x004a31c8;
L_0x004a3143:
    // 004a3143  b808cb5300             -mov eax, 0x53cb08
    cpu.eax = 5491464 /*0x53cb08*/;
    // 004a3148  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a3149  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a314a:
    // 004a314a  b828cb5300             -mov eax, 0x53cb28
    cpu.eax = 5491496 /*0x53cb28*/;
    // 004a314f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a3150  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a3151:
    // 004a3151  b840cb5300             -mov eax, 0x53cb40
    cpu.eax = 5491520 /*0x53cb40*/;
    // 004a3156  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a3157  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a3158:
    // 004a3158  b860cb5300             -mov eax, 0x53cb60
    cpu.eax = 5491552 /*0x53cb60*/;
    // 004a315d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a315e  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a315f:
    // 004a315f  b880cb5300             -mov eax, 0x53cb80
    cpu.eax = 5491584 /*0x53cb80*/;
    // 004a3164  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a3165  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a3166:
    // 004a3166  b8a0cb5300             -mov eax, 0x53cba0
    cpu.eax = 5491616 /*0x53cba0*/;
    // 004a316b  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a316c  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a316d:
    // 004a316d  b8c0cb5300             -mov eax, 0x53cbc0
    cpu.eax = 5491648 /*0x53cbc0*/;
    // 004a3172  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a3173  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a3174:
    // 004a3174  b8e0cb5300             -mov eax, 0x53cbe0
    cpu.eax = 5491680 /*0x53cbe0*/;
    // 004a3179  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a317a  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a317b:
    // 004a317b  b8f8cb5300             -mov eax, 0x53cbf8
    cpu.eax = 5491704 /*0x53cbf8*/;
    // 004a3180  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a3181  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a3182:
    // 004a3182  b814cc5300             -mov eax, 0x53cc14
    cpu.eax = 5491732 /*0x53cc14*/;
    // 004a3187  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a3188  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a3189:
    // 004a3189  b82ccc5300             -mov eax, 0x53cc2c
    cpu.eax = 5491756 /*0x53cc2c*/;
    // 004a318e  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a318f  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a3190:
    // 004a3190  b84ccc5300             -mov eax, 0x53cc4c
    cpu.eax = 5491788 /*0x53cc4c*/;
    // 004a3195  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a3196  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a3197:
    // 004a3197  b864cc5300             -mov eax, 0x53cc64
    cpu.eax = 5491812 /*0x53cc64*/;
    // 004a319c  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a319d  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a319e:
    // 004a319e  b884cc5300             -mov eax, 0x53cc84
    cpu.eax = 5491844 /*0x53cc84*/;
    // 004a31a3  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a31a4  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a31a5:
    // 004a31a5  b89ccc5300             -mov eax, 0x53cc9c
    cpu.eax = 5491868 /*0x53cc9c*/;
    // 004a31aa  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a31ab  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a31ac:
    // 004a31ac  b8bccc5300             -mov eax, 0x53ccbc
    cpu.eax = 5491900 /*0x53ccbc*/;
    // 004a31b1  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a31b2  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a31b3:
    // 004a31b3  b8e0cc5300             -mov eax, 0x53cce0
    cpu.eax = 5491936 /*0x53cce0*/;
    // 004a31b8  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a31b9  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a31ba:
    // 004a31ba  b8fccc5300             -mov eax, 0x53ccfc
    cpu.eax = 5491964 /*0x53ccfc*/;
    // 004a31bf  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a31c0  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a31c1:
    // 004a31c1  b814cd5300             -mov eax, 0x53cd14
    cpu.eax = 5491988 /*0x53cd14*/;
    // 004a31c6  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a31c7  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a31c8:
    // 004a31c8  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a31c9  6830cd5300             -push 0x53cd30
    app->getMemory<x86::reg32>(cpu.esp-4) = 5492016 /*0x53cd30*/;
    cpu.esp -= 4;
    // 004a31ce  68841a7a00             -push 0x7a1a84
    app->getMemory<x86::reg32>(cpu.esp-4) = 8002180 /*0x7a1a84*/;
    cpu.esp -= 4;
    // 004a31d3  e8b8c40300             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a31d8  b8841a7a00             -mov eax, 0x7a1a84
    cpu.eax = 8002180 /*0x7a1a84*/;
    // 004a31dd  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 004a31e0  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a31e1  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_4a3070(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    goto L_entry_0x004a3070;
    // 004a3050  fc                     -cld 
    cpu.flags.df = 0;
    // 004a3051  c8530020               -enter 0x53, 0x20
    {
        cpu.esp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) = cpu.ebp;
        x86::reg32 esp = cpu.esp;
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        cpu.ebp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) =  app->getMemory<x86::reg32>(cpu.ebp);
        cpu.esp -= 4;
        app->getMemory<x86::reg32>(cpu.esp) = esp;
        cpu.ebp = esp;
        cpu.esp -= 83;
    }
    // 004a3055  c9                     -leave 
    cpu.esp = cpu.ebp;
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a3056  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a3057  0040c9                 -add byte ptr [eax - 0x37], al
    (app->getMemory<x86::reg8>(cpu.eax + x86::reg32(-55) /* -0x37 */)) += x86::reg8(x86::sreg8(cpu.al));
    // 004a305a  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a305b  0060c9                 -add byte ptr [eax - 0x37], ah
    (app->getMemory<x86::reg8>(cpu.eax + x86::reg32(-55) /* -0x37 */)) += x86::reg8(x86::sreg8(cpu.ah));
    // 004a305e  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a305f  00e8                   -add al, ch
    (cpu.al) += x86::reg8(x86::sreg8(cpu.ch));
    // 004a3061  c9                     -leave 
    cpu.esp = cpu.ebp;
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a3062  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a3063  0084ca53008d80         -add byte ptr [edx + ecx*8 - 0x7f72ffad], al
    (app->getMemory<x86::reg8>(cpu.edx + x86::reg32(-2138242989) /* -0x7f72ffad */ + cpu.ecx * 8)) += x86::reg8(x86::sreg8(cpu.al));
    // 004a306a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004a306c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 004a306e  8bd2                   -mov edx, edx
    cpu.edx = cpu.edx;
L_entry_0x004a3070:
    // 004a3070  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a3071  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a3073  3d8e0000c0             +cmp eax, 0xc000008e
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3221225614 /*0xc000008e*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a3078  7260                   -jb 0x4a30da
    if (cpu.flags.cf)
    {
        goto L_0x004a30da;
    }
    // 004a307a  0f86df000000           -jbe 0x4a315f
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004a315f;
    }
    // 004a3080  3d930000c0             +cmp eax, 0xc0000093
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3221225619 /*0xc0000093*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a3085  7232                   -jb 0x4a30b9
    if (cpu.flags.cf)
    {
        goto L_0x004a30b9;
    }
    // 004a3087  0f86f5000000           -jbe 0x4a3182
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004a3182;
    }
    // 004a308d  3d950000c0             +cmp eax, 0xc0000095
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3221225621 /*0xc0000095*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a3092  0f82ff000000           -jb 0x4a3197
    if (cpu.flags.cf)
    {
        goto L_0x004a3197;
    }
    // 004a3098  0f8600010000           -jbe 0x4a319e
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004a319e;
    }
    // 004a309e  3d960000c0             +cmp eax, 0xc0000096
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3221225622 /*0xc0000096*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a30a3  0f860a010000           -jbe 0x4a31b3
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004a31b3;
    }
    // 004a30a9  3dfd0000c0             +cmp eax, 0xc00000fd
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3221225725 /*0xc00000fd*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a30ae  0f840d010000           -je 0x4a31c1
    if (cpu.flags.zf)
    {
        goto L_0x004a31c1;
    }
    // 004a30b4  e90f010000             -jmp 0x4a31c8
    goto L_0x004a31c8;
L_0x004a30b9:
    // 004a30b9  3d900000c0             +cmp eax, 0xc0000090
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3221225616 /*0xc0000090*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a30be  0f82a2000000           -jb 0x4a3166
    if (cpu.flags.cf)
    {
        goto L_0x004a3166;
    }
    // 004a30c4  0f86a3000000           -jbe 0x4a316d
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004a316d;
    }
    // 004a30ca  3d910000c0             +cmp eax, 0xc0000091
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3221225617 /*0xc0000091*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a30cf  0f869f000000           -jbe 0x4a3174
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004a3174;
    }
    // 004a30d5  e9a1000000             -jmp 0x4a317b
    goto L_0x004a317b;
L_0x004a30da:
    // 004a30da  3d1d0000c0             +cmp eax, 0xc000001d
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3221225501 /*0xc000001d*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a30df  7232                   -jb 0x4a3113
    if (cpu.flags.cf)
    {
        goto L_0x004a3113;
    }
    // 004a30e1  0f86a2000000           -jbe 0x4a3189
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004a3189;
    }
    // 004a30e7  3d260000c0             +cmp eax, 0xc0000026
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3221225510 /*0xc0000026*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a30ec  7215                   -jb 0x4a3103
    if (cpu.flags.cf)
    {
        goto L_0x004a3103;
    }
    // 004a30ee  0f86b1000000           -jbe 0x4a31a5
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004a31a5;
    }
    // 004a30f4  3d8c0000c0             +cmp eax, 0xc000008c
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3221225612 /*0xc000008c*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a30f9  0f82c9000000           -jb 0x4a31c8
    if (cpu.flags.cf)
    {
        goto L_0x004a31c8;
    }
    // 004a30ff  7642                   -jbe 0x4a3143
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004a3143;
    }
    // 004a3101  eb55                   -jmp 0x4a3158
    goto L_0x004a3158;
L_0x004a3103:
    // 004a3103  3d250000c0             +cmp eax, 0xc0000025
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3221225509 /*0xc0000025*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a3108  0f849e000000           -je 0x4a31ac
    if (cpu.flags.zf)
    {
        goto L_0x004a31ac;
    }
    // 004a310e  e9b5000000             -jmp 0x4a31c8
    goto L_0x004a31c8;
L_0x004a3113:
    // 004a3113  3d03000080             +cmp eax, 0x80000003
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2147483651 /*0x80000003*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a3118  721d                   -jb 0x4a3137
    if (cpu.flags.cf)
    {
        goto L_0x004a3137;
    }
    // 004a311a  762e                   -jbe 0x4a314a
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004a314a;
    }
    // 004a311c  3d04000080             +cmp eax, 0x80000004
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2147483652 /*0x80000004*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a3121  0f8693000000           -jbe 0x4a31ba
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004a31ba;
    }
    // 004a3127  3d060000c0             +cmp eax, 0xc0000006
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3221225478 /*0xc0000006*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a312c  0f845e000000           -je 0x4a3190
    if (cpu.flags.zf)
    {
        goto L_0x004a3190;
    }
    // 004a3132  e991000000             -jmp 0x4a31c8
    goto L_0x004a31c8;
L_0x004a3137:
    // 004a3137  3d02000080             +cmp eax, 0x80000002
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2147483650 /*0x80000002*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a313c  7413                   -je 0x4a3151
    if (cpu.flags.zf)
    {
        goto L_0x004a3151;
    }
    // 004a313e  e985000000             -jmp 0x4a31c8
    goto L_0x004a31c8;
L_0x004a3143:
    // 004a3143  b808cb5300             -mov eax, 0x53cb08
    cpu.eax = 5491464 /*0x53cb08*/;
    // 004a3148  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a3149  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a314a:
    // 004a314a  b828cb5300             -mov eax, 0x53cb28
    cpu.eax = 5491496 /*0x53cb28*/;
    // 004a314f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a3150  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a3151:
    // 004a3151  b840cb5300             -mov eax, 0x53cb40
    cpu.eax = 5491520 /*0x53cb40*/;
    // 004a3156  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a3157  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a3158:
    // 004a3158  b860cb5300             -mov eax, 0x53cb60
    cpu.eax = 5491552 /*0x53cb60*/;
    // 004a315d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a315e  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a315f:
    // 004a315f  b880cb5300             -mov eax, 0x53cb80
    cpu.eax = 5491584 /*0x53cb80*/;
    // 004a3164  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a3165  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a3166:
    // 004a3166  b8a0cb5300             -mov eax, 0x53cba0
    cpu.eax = 5491616 /*0x53cba0*/;
    // 004a316b  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a316c  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a316d:
    // 004a316d  b8c0cb5300             -mov eax, 0x53cbc0
    cpu.eax = 5491648 /*0x53cbc0*/;
    // 004a3172  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a3173  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a3174:
    // 004a3174  b8e0cb5300             -mov eax, 0x53cbe0
    cpu.eax = 5491680 /*0x53cbe0*/;
    // 004a3179  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a317a  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a317b:
    // 004a317b  b8f8cb5300             -mov eax, 0x53cbf8
    cpu.eax = 5491704 /*0x53cbf8*/;
    // 004a3180  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a3181  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a3182:
    // 004a3182  b814cc5300             -mov eax, 0x53cc14
    cpu.eax = 5491732 /*0x53cc14*/;
    // 004a3187  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a3188  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a3189:
    // 004a3189  b82ccc5300             -mov eax, 0x53cc2c
    cpu.eax = 5491756 /*0x53cc2c*/;
    // 004a318e  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a318f  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a3190:
    // 004a3190  b84ccc5300             -mov eax, 0x53cc4c
    cpu.eax = 5491788 /*0x53cc4c*/;
    // 004a3195  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a3196  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a3197:
    // 004a3197  b864cc5300             -mov eax, 0x53cc64
    cpu.eax = 5491812 /*0x53cc64*/;
    // 004a319c  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a319d  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a319e:
    // 004a319e  b884cc5300             -mov eax, 0x53cc84
    cpu.eax = 5491844 /*0x53cc84*/;
    // 004a31a3  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a31a4  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a31a5:
    // 004a31a5  b89ccc5300             -mov eax, 0x53cc9c
    cpu.eax = 5491868 /*0x53cc9c*/;
    // 004a31aa  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a31ab  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a31ac:
    // 004a31ac  b8bccc5300             -mov eax, 0x53ccbc
    cpu.eax = 5491900 /*0x53ccbc*/;
    // 004a31b1  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a31b2  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a31b3:
    // 004a31b3  b8e0cc5300             -mov eax, 0x53cce0
    cpu.eax = 5491936 /*0x53cce0*/;
    // 004a31b8  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a31b9  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a31ba:
    // 004a31ba  b8fccc5300             -mov eax, 0x53ccfc
    cpu.eax = 5491964 /*0x53ccfc*/;
    // 004a31bf  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a31c0  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a31c1:
    // 004a31c1  b814cd5300             -mov eax, 0x53cd14
    cpu.eax = 5491988 /*0x53cd14*/;
    // 004a31c6  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a31c7  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a31c8:
    // 004a31c8  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a31c9  6830cd5300             -push 0x53cd30
    app->getMemory<x86::reg32>(cpu.esp-4) = 5492016 /*0x53cd30*/;
    cpu.esp -= 4;
    // 004a31ce  68841a7a00             -push 0x7a1a84
    app->getMemory<x86::reg32>(cpu.esp-4) = 8002180 /*0x7a1a84*/;
    cpu.esp -= 4;
    // 004a31d3  e8b8c40300             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a31d8  b8841a7a00             -mov eax, 0x7a1a84
    cpu.eax = 8002180 /*0x7a1a84*/;
    // 004a31dd  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 004a31e0  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a31e1  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_4a31f0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004a31f0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a31f1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a31f2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004a31f3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004a31f4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004a31f5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a31f6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a31f8  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004a31fa  f70004000100           +test dword ptr [eax], 0x10004
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(app->getMemory<x86::reg32>(cpu.eax) & 65540 /*0x10004*/));
    // 004a3200  7446                   -je 0x4a3248
    if (cpu.flags.zf)
    {
        goto L_0x004a3248;
    }
    // 004a3202  8b988c000000           -mov ebx, dword ptr [eax + 0x8c]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(140) /* 0x8c */);
    // 004a3208  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a3209  8bb090000000           -mov esi, dword ptr [eax + 0x90]
    cpu.esi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(144) /* 0x90 */);
    // 004a320f  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004a3210  8bb894000000           -mov edi, dword ptr [eax + 0x94]
    cpu.edi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(148) /* 0x94 */);
    // 004a3216  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004a3217  8b8098000000           -mov eax, dword ptr [eax + 0x98]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(152) /* 0x98 */);
    // 004a321d  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a321e  8b8abc000000           -mov ecx, dword ptr [edx + 0xbc]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(188) /* 0xbc */);
    // 004a3224  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a3225  684ccd5300             -push 0x53cd4c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5492044 /*0x53cd4c*/;
    cpu.esp -= 4;
    // 004a322a  bf78167a00             -mov edi, 0x7a1678
    cpu.edi = 8001144 /*0x7a1678*/;
    // 004a322f  29c9                   -sub ecx, ecx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004a3231  49                     -dec ecx
    (cpu.ecx)--;
    // 004a3232  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004a3234  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 004a3236  f7d1                   -not ecx
    cpu.ecx = ~cpu.ecx;
    // 004a3238  49                     -dec ecx
    (cpu.ecx)--;
    // 004a3239  81c178167a00           -add ecx, 0x7a1678
    (cpu.ecx) += x86::reg32(x86::sreg32(8001144 /*0x7a1678*/));
    // 004a323f  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a3240  e84bc40300             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a3245  83c41c                 -add esp, 0x1c
    (cpu.esp) += x86::reg32(x86::sreg32(28 /*0x1c*/));
L_0x004a3248:
    // 004a3248  f70202000100           +test dword ptr [edx], 0x10002
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(app->getMemory<x86::reg32>(cpu.edx) & 65538 /*0x10002*/));
    // 004a324e  744d                   -je 0x4a329d
    if (cpu.flags.zf)
    {
        goto L_0x004a329d;
    }
    // 004a3250  8bb2a8000000           -mov esi, dword ptr [edx + 0xa8]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(168) /* 0xa8 */);
    // 004a3256  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004a3257  8bbaac000000           -mov edi, dword ptr [edx + 0xac]
    cpu.edi = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(172) /* 0xac */);
    // 004a325d  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004a325e  8b82a4000000           -mov eax, dword ptr [edx + 0xa4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(164) /* 0xa4 */);
    // 004a3264  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a3265  8b8ab0000000           -mov ecx, dword ptr [edx + 0xb0]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(176) /* 0xb0 */);
    // 004a326b  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a326c  8b9aa0000000           -mov ebx, dword ptr [edx + 0xa0]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(160) /* 0xa0 */);
    // 004a3272  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a3273  8bb29c000000           -mov esi, dword ptr [edx + 0x9c]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(156) /* 0x9c */);
    // 004a3279  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004a327a  687ccd5300             -push 0x53cd7c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5492092 /*0x53cd7c*/;
    cpu.esp -= 4;
    // 004a327f  bf78167a00             -mov edi, 0x7a1678
    cpu.edi = 8001144 /*0x7a1678*/;
    // 004a3284  29c9                   -sub ecx, ecx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004a3286  49                     -dec ecx
    (cpu.ecx)--;
    // 004a3287  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004a3289  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 004a328b  f7d1                   -not ecx
    cpu.ecx = ~cpu.ecx;
    // 004a328d  49                     -dec ecx
    (cpu.ecx)--;
    // 004a328e  81c178167a00           -add ecx, 0x7a1678
    (cpu.ecx) += x86::reg32(x86::sreg32(8001144 /*0x7a1678*/));
    // 004a3294  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a3295  e8f6c30300             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a329a  83c420                 -add esp, 0x20
    (cpu.esp) += x86::reg32(x86::sreg32(32 /*0x20*/));
L_0x004a329d:
    // 004a329d  f70201000100           +test dword ptr [edx], 0x10001
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(app->getMemory<x86::reg32>(cpu.edx) & 65537 /*0x10001*/));
    // 004a32a3  743f                   -je 0x4a32e4
    if (cpu.flags.zf)
    {
        goto L_0x004a32e4;
    }
    // 004a32a5  8b82c8000000           -mov eax, dword ptr [edx + 0xc8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(200) /* 0xc8 */);
    // 004a32ab  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a32ac  8b8ac4000000           -mov ecx, dword ptr [edx + 0xc4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(196) /* 0xc4 */);
    // 004a32b2  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a32b3  8b9ab4000000           -mov ebx, dword ptr [edx + 0xb4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(180) /* 0xb4 */);
    // 004a32b9  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a32ba  8bb2b8000000           -mov esi, dword ptr [edx + 0xb8]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(184) /* 0xb8 */);
    // 004a32c0  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004a32c1  68b8cd5300             -push 0x53cdb8
    app->getMemory<x86::reg32>(cpu.esp-4) = 5492152 /*0x53cdb8*/;
    cpu.esp -= 4;
    // 004a32c6  bf78167a00             -mov edi, 0x7a1678
    cpu.edi = 8001144 /*0x7a1678*/;
    // 004a32cb  29c9                   -sub ecx, ecx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004a32cd  49                     -dec ecx
    (cpu.ecx)--;
    // 004a32ce  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004a32d0  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 004a32d2  f7d1                   -not ecx
    cpu.ecx = ~cpu.ecx;
    // 004a32d4  49                     -dec ecx
    (cpu.ecx)--;
    // 004a32d5  81c178167a00           -add ecx, 0x7a1678
    (cpu.ecx) += x86::reg32(x86::sreg32(8001144 /*0x7a1678*/));
    // 004a32db  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a32dc  e8afc30300             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a32e1  83c418                 -add esp, 0x18
    (cpu.esp) += x86::reg32(x86::sreg32(24 /*0x18*/));
L_0x004a32e4:
    // 004a32e4  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a32e5  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a32e6  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a32e7  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a32e8  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a32e9  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a32ea  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_4a32f0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004a32f0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a32f1  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004a32f2  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004a32f3  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a32f4  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a32f6  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004a32f8  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 004a32fa  7415                   -je 0x4a3311
    if (cpu.flags.zf)
    {
        goto L_0x004a3311;
    }
    // 004a32fc  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004a32fe  7507                   -jne 0x4a3307
    if (!cpu.flags.zf)
    {
        goto L_0x004a3307;
    }
    // 004a3300  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004a3302  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a3303  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a3304  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a3305  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a3306  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a3307:
    // 004a3307  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
    // 004a330c  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a330d  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a330e  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a330f  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a3310  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a3311:
    // 004a3311  30e4                   -xor ah, ah
    cpu.ah ^= x86::reg8(x86::sreg8(cpu.ah));
    // 004a3313  882578167a00           -mov byte ptr [0x7a1678], ah
    app->getMemory<x86::reg8>(x86::reg32(8001144) /* 0x7a1678 */) = cpu.ah;
    // 004a3319  8b01                   -mov eax, dword ptr [ecx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx);
    // 004a331b  8b18                   -mov ebx, dword ptr [eax]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax);
    // 004a331d  81fb050000c0           +cmp ebx, 0xc0000005
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3221225477 /*0xc0000005*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a3323  752c                   -jne 0x4a3351
    if (!cpu.flags.zf)
    {
        goto L_0x004a3351;
    }
    // 004a3325  8b7018                 -mov esi, dword ptr [eax + 0x18]
    cpu.esi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(24) /* 0x18 */);
    // 004a3328  8b7814                 -mov edi, dword ptr [eax + 0x14]
    cpu.edi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(20) /* 0x14 */);
    // 004a332b  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004a332c  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 004a332e  7407                   -je 0x4a3337
    if (cpu.flags.zf)
    {
        goto L_0x004a3337;
    }
    // 004a3330  b8e0cd5300             -mov eax, 0x53cde0
    cpu.eax = 5492192 /*0x53cde0*/;
    // 004a3335  eb05                   -jmp 0x4a333c
    goto L_0x004a333c;
L_0x004a3337:
    // 004a3337  b8e8cd5300             -mov eax, 0x53cde8
    cpu.eax = 5492200 /*0x53cde8*/;
L_0x004a333c:
    // 004a333c  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a333d  68f0cd5300             -push 0x53cdf0
    app->getMemory<x86::reg32>(cpu.esp-4) = 5492208 /*0x53cdf0*/;
    cpu.esp -= 4;
    // 004a3342  6878167a00             -push 0x7a1678
    app->getMemory<x86::reg32>(cpu.esp-4) = 8001144 /*0x7a1678*/;
    cpu.esp -= 4;
    // 004a3347  e844c30300             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a334c  83c410                 +add esp, 0x10
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(16 /*0x10*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004a334f  eb1a                   -jmp 0x4a336b
    goto L_0x004a336b;
L_0x004a3351:
    // 004a3351  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 004a3353  e818fdffff             -call 0x4a3070
    cpu.esp -= 4;
    sub_4a3070(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a3358  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a3359  6824ce5300             -push 0x53ce24
    app->getMemory<x86::reg32>(cpu.esp-4) = 5492260 /*0x53ce24*/;
    cpu.esp -= 4;
    // 004a335e  6878167a00             -push 0x7a1678
    app->getMemory<x86::reg32>(cpu.esp-4) = 8001144 /*0x7a1678*/;
    cpu.esp -= 4;
    // 004a3363  e828c30300             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a3368  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
L_0x004a336b:
    // 004a336b  8b4104                 -mov eax, dword ptr [ecx + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(4) /* 0x4 */);
    // 004a336e  e87dfeffff             -call 0x4a31f0
    cpu.esp -= 4;
    sub_4a31f0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a3373  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004a3375  7507                   -jne 0x4a337e
    if (!cpu.flags.zf)
    {
        goto L_0x004a337e;
    }
    // 004a3377  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004a3379  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a337a  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a337b  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a337c  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a337d  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a337e:
    // 004a337e  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004a3383  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a3384  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a3385  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a3386  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a3387  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_4a3390(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004a3390  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a3391  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a3393  e8882c0100             -call 0x4b6020
    cpu.esp -= 4;
    sub_4b6020(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a3398  e8e3950100             -call 0x4bc980
    cpu.esp -= 4;
    sub_4bc980(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a339d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a339e  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x90 */
void Application::sub_4a33a0(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004a33a0  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a33a1  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a33a3  e8e8950100             -call 0x4bc990
    cpu.esp -= 4;
    sub_4bc990(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a33a8  e8a32e0100             -call 0x4b6250
    cpu.esp -= 4;
    nfs3hp::graphics::shutDown(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a33ad  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a33ae  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x90 */
void Application::sub_4a33b0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004a33b0  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a33b1  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a33b3  e8d8970100             -call 0x4bcb90
    cpu.esp -= 4;
    sub_4bcb90(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a33b8  e883130200             -call 0x4c4740
    cpu.esp -= 4;
    sub_4c4740(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a33bd  e8beaaf7ff             -call 0x41de80
    cpu.esp -= 4;
    nfs3hp::track::resetTrackViews(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a33c2  e829f9f7ff             -call 0x422cf0
    cpu.esp -= 4;
    nfs3hp::carstate::initCameras(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a33c7  e8b4e2f6ff             -call 0x411680
    cpu.esp -= 4;
    nfs3hp::port::sourcesReset(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a33cc  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a33cd  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_4a33d0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004a33d0  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a33d1  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a33d3  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004a33d8  e8e37d0400             -call 0x4eb1c0
    cpu.esp -= 4;
    sub_4eb1c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a33dd  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a33df  7416                   -je 0x4a33f7
    if (cpu.flags.zf)
    {
        goto L_0x004a33f7;
    }
    // 004a33e1  c7058ce8550001000000   -mov dword ptr [0x55e88c], 1
    app->getMemory<x86::reg32>(x86::reg32(5630092) /* 0x55e88c */) = 1 /*0x1*/;
    // 004a33eb  e8e017ffff             -call 0x494bd0
    cpu.esp -= 4;
    nfs3hp::graphics::drawLoadingAborted(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a33f0  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004a33f5  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a33f6  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a33f7:
    // 004a33f7  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004a33f9  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a33fa  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_4a3400(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004a3400  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a3401  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a3402  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004a3403  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a3404  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a3406  83ec10                 -sub esp, 0x10
    (cpu.esp) -= x86::reg32(x86::sreg32(16 /*0x10*/));
    // 004a3409  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    raceRunning(true); /* port: sub_4a3400 sets a race up */
    // 004a340e  b80d000000             -mov eax, 0xd
    cpu.eax = 13 /*0xd*/;
    // 004a3413  e8e8e4f8ff             -call 0x431900
    cpu.esp -= 4;
    nfs3hp::graphics::setRenderState(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a3418  e8d34efaff             -call 0x4482f0
    cpu.esp -= 4;
    nfs3hp::frontend::modesChosenWidth(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a341d  a3c4fe5500             -mov dword ptr [0x55fec4], eax
    app->getMemory<x86::reg32>(x86::reg32(5635780) /* 0x55fec4 */) = cpu.eax;
    // 004a3422  e829bb0100             -call 0x4bef50
    cpu.esp -= 4;
    nfs3hp::render::screenMode(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a3427  f60544d56f0008         +test byte ptr [0x6fd544], 8
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(x86::reg32(7329092) /* 0x6fd544 */) & 8 /*0x8*/));
    // 004a342e  7407                   -je 0x4a3437
    if (cpu.flags.zf)
    {
        goto L_0x004a3437;
    }
    // 004a3430  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004a3435  eb02                   -jmp 0x4a3439
    goto L_0x004a3439;
L_0x004a3437:
    // 004a3437  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x004a3439:
    // 004a3439  e8a2b10100             -call 0x4be5e0
    cpu.esp -= 4;
    sub_4be5e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a343e  8d4df4                 -lea ecx, [ebp - 0xc]
    cpu.ecx = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004a3441  8d5df0                 -lea ebx, [ebp - 0x10]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004a3444  8d55fc                 -lea edx, [ebp - 4]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004a3447  8d45f8                 -lea eax, [ebp - 8]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004a344a  e831b70100             -call 0x4beb80
    cpu.esp -= 4;
    nfs3hp::port::text::viewport(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a344f  8b4df4                 -mov ecx, dword ptr [ebp - 0xc]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004a3452  8b5df0                 -mov ebx, dword ptr [ebp - 0x10]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004a3455  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004a3457  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004a3459  e8721ef9ff             -call 0x4352d0
    cpu.esp -= 4;
    nfs3hp::controls::setPointerBounds(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a345e  e8dd0bfeff             -call 0x484040
    cpu.esp -= 4;
    nfs3hp::port::hud::loadingTextPrepared(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a3463  e80817feff             -call 0x484b70
    cpu.esp -= 4;
    nfs3hp::port::hud::screenScalesSet(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a3468  e8331bffff             -call 0x494fa0
    cpu.esp -= 4;
    nfs3hp::graphics::showLoadingScreen(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a346d  e8ce0bfeff             -call 0x484040
    cpu.esp -= 4;
    nfs3hp::port::hud::loadingTextPrepared(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a3472  e8d945fdff             -call 0x477a50
    cpu.esp -= 4;
    sub_477a50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a3477  680000a040             -push 0x40a00000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1084227584 /*0x40a00000*/;
    cpu.esp -= 4;
    // 004a347c  e8af1dffff             -call 0x495230
    cpu.esp -= 4;
    nfs3hp::graphics::drawLoadingProgress(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a3481  e8bacd0400             -call 0x4f0240
    cpu.esp -= 4;
    nfs3hp::ea::flushKeys(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a3486  e8d54afdff             -call 0x477f60
    cpu.esp -= 4;
    sub_477f60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a348b  e840ffffff             -call 0x4a33d0
    cpu.esp -= 4;
    sub_4a33d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a3490  833d8ce8550000         +cmp dword ptr [0x55e88c], 0
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
    // 004a3497  740a                   -je 0x4a34a3
    if (cpu.flags.zf)
    {
        goto L_0x004a34a3;
    }
    // 004a3499  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004a349e  e921010000             -jmp 0x4a35c4
    goto L_0x004a35c4;
L_0x004a34a3:
    // 004a34a3  6800002041             -push 0x41200000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1092616192 /*0x41200000*/;
    cpu.esp -= 4;
    // 004a34a8  e8831dffff             -call 0x495230
    cpu.esp -= 4;
    nfs3hp::graphics::drawLoadingProgress(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a34ad  e81e4bfdff             -call 0x477fd0
    cpu.esp -= 4;
    sub_477fd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a34b2  e819ffffff             -call 0x4a33d0
    cpu.esp -= 4;
    sub_4a33d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a34b7  833d8ce8550000         +cmp dword ptr [0x55e88c], 0
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
    // 004a34be  740c                   -je 0x4a34cc
    if (cpu.flags.zf)
    {
        goto L_0x004a34cc;
    }
    // 004a34c0  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004a34c5  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004a34c7  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a34c8  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a34c9  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a34ca  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a34cb  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a34cc:
    // 004a34cc  6800000c42             -push 0x420c0000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1108082688 /*0x420c0000*/;
    cpu.esp -= 4;
    // 004a34d1  e85a1dffff             -call 0x495230
    cpu.esp -= 4;
    nfs3hp::graphics::drawLoadingProgress(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a34d6  e815220300             -call 0x4d56f0
    cpu.esp -= 4;
    sub_4d56f0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a34db  e8f0feffff             -call 0x4a33d0
    cpu.esp -= 4;
    sub_4a33d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a34e0  833d8ce8550000         +cmp dword ptr [0x55e88c], 0
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
    // 004a34e7  740c                   -je 0x4a34f5
    if (cpu.flags.zf)
    {
        goto L_0x004a34f5;
    }
    // 004a34e9  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004a34ee  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004a34f0  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a34f1  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a34f2  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a34f3  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a34f4  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a34f5:
    // 004a34f5  6800002042             -push 0x42200000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1109393408 /*0x42200000*/;
    cpu.esp -= 4;
    // 004a34fa  e8311dffff             -call 0x495230
    cpu.esp -= 4;
    nfs3hp::graphics::drawLoadingProgress(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a34ff  a1acd46f00             -mov eax, dword ptr [0x6fd4ac]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328940) /* 0x6fd4ac */);
    // 004a3504  e8e7860300             -call 0x4dbbf0
    cpu.esp -= 4;
    sub_4dbbf0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a3509  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a350b  740c                   -je 0x4a3519
    if (cpu.flags.zf)
    {
        goto L_0x004a3519;
    }
    // 004a350d  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004a3512  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004a3514  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a3515  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a3516  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a3517  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a3518  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a3519:
    // 004a3519  6800008c42             -push 0x428c0000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1116471296 /*0x428c0000*/;
    cpu.esp -= 4;
    // 004a351e  e80d1dffff             -call 0x495230
    cpu.esp -= 4;
    nfs3hp::graphics::drawLoadingProgress(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a3523  e8f8970300             -call 0x4dcd20
    cpu.esp -= 4;
    nfs3hp::load::loadWeatherShapes(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a3528  e8a3feffff             -call 0x4a33d0
    cpu.esp -= 4;
    sub_4a33d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a352d  833d8ce8550000         +cmp dword ptr [0x55e88c], 0
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
    // 004a3534  740c                   -je 0x4a3542
    if (cpu.flags.zf)
    {
        goto L_0x004a3542;
    }
    // 004a3536  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004a353b  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004a353d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a353e  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a353f  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a3540  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a3541  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a3542:
    // 004a3542  6800009642             -push 0x42960000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1117126656 /*0x42960000*/;
    cpu.esp -= 4;
    // 004a3547  e8e41cffff             -call 0x495230
    cpu.esp -= 4;
    nfs3hp::graphics::drawLoadingProgress(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a354c  e87f4ef8ff             -call 0x4283d0
    cpu.esp -= 4;
    sub_4283d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a3551  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a3553  740c                   -je 0x4a3561
    if (cpu.flags.zf)
    {
        goto L_0x004a3561;
    }
    // 004a3555  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004a355a  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004a355c  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a355d  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a355e  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a355f  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a3560  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a3561:
    // 004a3561  680000b442             -push 0x42b40000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1119092736 /*0x42b40000*/;
    cpu.esp -= 4;
    // 004a3566  e8c51cffff             -call 0x495230
    cpu.esp -= 4;
    nfs3hp::graphics::drawLoadingProgress(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a356b  e880f7f7ff             -call 0x422cf0
    cpu.esp -= 4;
    nfs3hp::carstate::initCameras(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a3570  680000ba42             -push 0x42ba0000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1119485952 /*0x42ba0000*/;
    cpu.esp -= 4;
    // 004a3575  e8b61cffff             -call 0x495230
    cpu.esp -= 4;
    nfs3hp::graphics::drawLoadingProgress(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a357a  e8b1920100             -call 0x4bc830
    cpu.esp -= 4;
    sub_4bc830(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a357f  e84cfeffff             -call 0x4a33d0
    cpu.esp -= 4;
    sub_4a33d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a3584  833d8ce8550000         +cmp dword ptr [0x55e88c], 0
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
    // 004a358b  740c                   -je 0x4a3599
    if (cpu.flags.zf)
    {
        goto L_0x004a3599;
    }
    // 004a358d  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004a3592  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004a3594  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a3595  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a3596  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a3597  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a3598  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a3599:
    // 004a3599  680000be42             -push 0x42be0000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1119748096 /*0x42be0000*/;
    cpu.esp -= 4;
    // 004a359e  e88d1cffff             -call 0x495230
    cpu.esp -= 4;
    nfs3hp::graphics::drawLoadingProgress(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a35a3  e8f82c0200             -call 0x4c62a0
    cpu.esp -= 4;
    sub_4c62a0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a35a8  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a35aa  740c                   -je 0x4a35b8
    if (cpu.flags.zf)
    {
        goto L_0x004a35b8;
    }
    // 004a35ac  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004a35b1  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004a35b3  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a35b4  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a35b5  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a35b6  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a35b7  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a35b8:
    // 004a35b8  680000c842             -push 0x42c80000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1120403456 /*0x42c80000*/;
    cpu.esp -= 4;
    // 004a35bd  e86e1cffff             -call 0x495230
    cpu.esp -= 4;
    nfs3hp::graphics::drawLoadingProgress(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a35c2  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x004a35c4:
    // 004a35c4  89ec                   -mov esp, ebp
    loadingScreenFit(app, cpu.sync(), false); /* port: loaded, the whole screen again */
    cpu.esp = cpu.ebp;
    // 004a35c6  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a35c7  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a35c8  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a35c9  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a35ca  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4a35d0: native, nfs3hp::graphics::takeDownRace */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_4a3690(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004a3690  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a3691  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a3692  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004a3693  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004a3694  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a3695  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a3697  81ecf4200000           -sub esp, 0x20f4
    (cpu.esp) -= x86::reg32(x86::sreg32(8436 /*0x20f4*/));
    // 004a369d  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 004a369f  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 004a36a2  b93c080000             -mov ecx, 0x83c
    cpu.ecx = 2108 /*0x83c*/;
    // 004a36a7  8dbd0cdfffff           -lea edi, [ebp - 0x20f4]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-8436) /* -0x20f4 */);
    // 004a36ad  be40bb6f00             -mov esi, 0x6fbb40
    cpu.esi = 7322432 /*0x6fbb40*/;
    // 004a36b2  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004a36b4  a1b4d36f00             -mov eax, dword ptr [0x6fd3b4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328692) /* 0x6fd3b4 */);
    // 004a36b9  89158ce85500           -mov dword ptr [0x55e88c], edx
    app->getMemory<x86::reg32>(x86::reg32(5630092) /* 0x55e88c */) = cpu.edx;
    // 004a36bf  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004a36c1  e828d70300             -call 0x4e0dee
    cpu.esp -= 4;
    nfs3hp::ea::seedRandom(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a36c6  833d5428550000         +cmp dword ptr [0x552854], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5580884) /* 0x552854 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a36cd  750c                   -jne 0x4a36db
    if (!cpu.flags.zf)
    {
        goto L_0x004a36db;
    }
    // 004a36cf  c7055428550001000000   -mov dword ptr [0x552854], 1
    app->getMemory<x86::reg32>(x86::reg32(5580884) /* 0x552854 */) = 1 /*0x1*/;
    // 004a36d9  eb06                   -jmp 0x4a36e1
    goto L_0x004a36e1;
L_0x004a36db:
    // 004a36db  891554285500           -mov dword ptr [0x552854], edx
    app->getMemory<x86::reg32>(x86::reg32(5580884) /* 0x552854 */) = cpu.edx;
L_0x004a36e1:
    // 004a36e1  833da0d36f0002         +cmp dword ptr [0x6fd3a0], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328672) /* 0x6fd3a0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a36e8  740e                   -je 0x4a36f8
    if (cpu.flags.zf)
    {
        goto L_0x004a36f8;
    }
    // 004a36ea  833d00d0550001         +cmp dword ptr [0x55d000], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5623808) /* 0x55d000 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a36f1  7405                   -je 0x4a36f8
    if (cpu.flags.zf)
    {
        goto L_0x004a36f8;
    }
    // 004a36f3  e8a815fdff             -call 0x474ca0
    cpu.esp -= 4;
    sub_474ca0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004a36f8:
    // 004a36f8  e803fdffff             -call 0x4a3400
    cpu.esp -= 4;
    sub_4a3400(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a36fd  98                     -cwde 
    cpu.eax = x86::reg32(static_cast<x86::sreg16>(cpu.ax));
    // 004a36fe  a38ce85500             -mov dword ptr [0x55e88c], eax
    app->getMemory<x86::reg32>(x86::reg32(5630092) /* 0x55e88c */) = cpu.eax;
    // 004a3703  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a3705  7466                   -je 0x4a376d
    if (cpu.flags.zf)
    {
        goto L_0x004a376d;
    }
    // 004a3707  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004a370c  b93c080000             -mov ecx, 0x83c
    cpu.ecx = 2108 /*0x83c*/;
    // 004a3711  8db50cdfffff           -lea esi, [ebp - 0x20f4]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-8436) /* -0x20f4 */);
    // 004a3717  bf40bb6f00             -mov edi, 0x6fbb40
    cpu.edi = 7322432 /*0x6fbb40*/;
    // 004a371c  e8affeffff             -call 0x4a35d0
    cpu.esp -= 4;
    nfs3hp::graphics::takeDownRace(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a3721  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004a3723  833db0d36f0001         +cmp dword ptr [0x6fd3b0], 1
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
    // 004a372a  7e24                   -jle 0x4a3750
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004a3750;
    }
    // 004a372c  833de0227a0000         +cmp dword ptr [0x7a22e0], 0
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
    // 004a3733  741b                   -je 0x4a3750
    if (cpu.flags.zf)
    {
        goto L_0x004a3750;
    }
    // 004a3735  e8a62e0000             -call 0x4a65e0
    cpu.esp -= 4;
    nfs3hp::ea::closeNetwork(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a373a  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 004a373f  b80b000000             -mov eax, 0xb
    cpu.eax = 11 /*0xb*/;
    // 004a3744  668915de227a00         -mov word ptr [0x7a22de], dx
    app->getMemory<x86::reg16>(x86::reg32(8004318) /* 0x7a22de */) = cpu.dx;
    // 004a374b  e964010000             -jmp 0x4a38b4
    goto L_0x004a38b4;
L_0x004a3750:
    // 004a3750  837dfc00               +cmp dword ptr [ebp - 4], 0
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
    // 004a3754  7405                   -je 0x4a375b
    if (cpu.flags.zf)
    {
        goto L_0x004a375b;
    }
    // 004a3756  e825f2fcff             -call 0x472980
    cpu.esp -= 4;
    nfs3hp::load::loadConfig(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004a375b:
    // 004a375b  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 004a375d  0f854c010000           -jne 0x4a38af
    if (!cpu.flags.zf)
    {
        goto L_0x004a38af;
    }
L_0x004a3763:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a3763  b80b000000             -mov eax, 0xb
    cpu.eax = 11 /*0xb*/;
    // 004a3768  e947010000             -jmp 0x4a38b4
    goto L_0x004a38b4;
L_0x004a376d:
    // 004a376d  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 004a376f  760c                   -jbe 0x4a377d
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004a377d;
    }
    // 004a3771  83fb01                 +cmp ebx, 1
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
    // 004a3774  7507                   -jne 0x4a377d
    if (!cpu.flags.zf)
    {
        goto L_0x004a377d;
    }
    // 004a3776  a3b4367d00             -mov dword ptr [0x7d36b4], eax
    app->getMemory<x86::reg32>(x86::reg32(8206004) /* 0x7d36b4 */) = cpu.eax;
    // 004a377b  eb0a                   -jmp 0x4a3787
    goto L_0x004a3787;
L_0x004a377d:
    // 004a377d  c705b4367d0002000000   -mov dword ptr [0x7d36b4], 2
    app->getMemory<x86::reg32>(x86::reg32(8206004) /* 0x7d36b4 */) = 2 /*0x2*/;
L_0x004a3787:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a3787  833d98367d0000         +cmp dword ptr [0x7d3698], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8205976) /* 0x7d3698 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a378e  745a                   -je 0x4a37ea
    if (cpu.flags.zf)
    {
        goto L_0x004a37ea;
    }
    // 004a3790  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 004a3792  893d98367d00           -mov dword ptr [0x7d3698], edi
    app->getMemory<x86::reg32>(x86::reg32(8205976) /* 0x7d3698 */) = cpu.edi;
    // 004a3798  e813fcffff             -call 0x4a33b0
    cpu.esp -= 4;
    sub_4a33b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a379d  a1f0bb6f00             -mov eax, dword ptr [0x6fbbf0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7322608) /* 0x6fbbf0 */);
    // 004a37a2  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a37a4  7429                   -je 0x4a37cf
    if (cpu.flags.zf)
    {
        goto L_0x004a37cf;
    }
    // 004a37a6  833de4bb6f0002         +cmp dword ptr [0x6fbbe4], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7322596) /* 0x6fbbe4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a37ad  7e09                   -jle 0x4a37b8
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004a37b8;
    }
    // 004a37af  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 004a37b4  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004a37b6  eb04                   -jmp 0x4a37bc
    goto L_0x004a37bc;
L_0x004a37b8:
    // 004a37b8  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004a37ba  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
L_0x004a37bc:
    // 004a37bc  a1acd46f00             -mov eax, dword ptr [0x6fd4ac]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328940) /* 0x6fd4ac */);
    // 004a37c1  e8facff6ff             -call 0x4107c0
    cpu.esp -= 4;
    nfs3hp::port::raceMusicStart(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a37c6  c60590e8550001         -mov byte ptr [0x55e890], 1
    app->getMemory<x86::reg8>(x86::reg32(5630096) /* 0x55e890 */) = 1 /*0x1*/;
    // 004a37cd  eb08                   -jmp 0x4a37d7
    goto L_0x004a37d7;
L_0x004a37cf:
    // 004a37cf  30d2                   -xor dl, dl
    cpu.dl ^= x86::reg8(x86::sreg8(cpu.dl));
    // 004a37d1  881590e85500           -mov byte ptr [0x55e890], dl
    app->getMemory<x86::reg8>(x86::reg32(5630096) /* 0x55e890 */) = cpu.dl;
L_0x004a37d7:
    // 004a37d7  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004a37d9  e8523e0400             -call 0x4e7630
    cpu.esp -= 4;
    nfs3hp::port::runSystemTasks(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a37de  e80d2c0200             -call 0x4c63f0
    cpu.esp -= 4;
    sub_4c63f0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a37e3  e858160200             -call 0x4c4e40
    cpu.esp -= 4;
    sub_4c4e40(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a37e8  eb9d                   -jmp 0x4a3787
    goto L_0x004a3787;
L_0x004a37ea:
    // 004a37ea  833db4367d0001         +cmp dword ptr [0x7d36b4], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8206004) /* 0x7d36b4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a37f1  742c                   -je 0x4a381f
    if (cpu.flags.zf)
    {
        goto L_0x004a381f;
    }
    // 004a37f3  837dfc00               +cmp dword ptr [ebp - 4], 0
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
    // 004a37f7  7526                   -jne 0x4a381f
    if (!cpu.flags.zf)
    {
        goto L_0x004a381f;
    }
    // 004a37f9  833db0d36f0001         +cmp dword ptr [0x6fd3b0], 1
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
    // 004a3800  7e1d                   -jle 0x4a381f
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004a381f;
    }
    // 004a3802  66833ddc227a0000       +cmp word ptr [0x7a22dc], 0
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
    // 004a380a  7509                   -jne 0x4a3815
    if (!cpu.flags.zf)
    {
        goto L_0x004a3815;
    }
    // 004a380c  833de0227a0000         +cmp dword ptr [0x7a22e0], 0
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
    // 004a3813  750a                   -jne 0x4a381f
    if (!cpu.flags.zf)
    {
        goto L_0x004a381f;
    }
L_0x004a3815:
    // 004a3815  c705b4367d0002000000   -mov dword ptr [0x7d36b4], 2
    app->getMemory<x86::reg32>(x86::reg32(8206004) /* 0x7d36b4 */) = 2 /*0x2*/;
L_0x004a381f:
    // 004a381f  e82cf3f6ff             -call 0x412b50
    cpu.esp -= 4;
    nfs3hp::port::raceSoundEnd(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a3824  e8a7d1f6ff             -call 0x4109d0
    cpu.esp -= 4;
    nfs3hp::port::musicStop(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a3829  833db4367d0001         +cmp dword ptr [0x7d36b4], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8206004) /* 0x7d36b4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a3830  7407                   -je 0x4a3839
    if (cpu.flags.zf)
    {
        goto L_0x004a3839;
    }
    // 004a3832  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004a3837  eb02                   -jmp 0x4a383b
    goto L_0x004a383b;
L_0x004a3839:
    // 004a3839  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x004a383b:
    // 004a383b  e890fdffff             -call 0x4a35d0
    cpu.esp -= 4;
    nfs3hp::graphics::takeDownRace(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a3840  e85bd2f6ff             -call 0x410aa0
    cpu.esp -= 4;
    nfs3hp::port::musicStreamEnd(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a3845  b82c010000             -mov eax, 0x12c
    cpu.eax = 300 /*0x12c*/;
    // 004a384a  8b4dfc                 -mov ecx, dword ptr [ebp - 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004a384d  e81ecbf6ff             -call 0x410370
    cpu.esp -= 4;
    nfs3hp::port::musicStreamMakeOf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a3852  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004a3854  750e                   -jne 0x4a3864
    if (!cpu.flags.zf)
    {
        goto L_0x004a3864;
    }
    // 004a3856  833db4367d0000         +cmp dword ptr [0x7d36b4], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8206004) /* 0x7d36b4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a385d  7505                   -jne 0x4a3864
    if (!cpu.flags.zf)
    {
        goto L_0x004a3864;
    }
    // 004a385f  e8ac1bfbff             -call 0x455410
    cpu.esp -= 4;
    nfs3hp::frontend::resultsBuild(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004a3864:
    // 004a3864  837dfc00               +cmp dword ptr [ebp - 4], 0
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
    // 004a3868  7405                   -je 0x4a386f
    if (cpu.flags.zf)
    {
        goto L_0x004a386f;
    }
    // 004a386a  e811f1fcff             -call 0x472980
    cpu.esp -= 4;
    nfs3hp::load::loadConfig(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004a386f:
    // 004a386f  a1b4367d00             -mov eax, dword ptr [0x7d36b4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8206004) /* 0x7d36b4 */);
    // 004a3874  83f801                 +cmp eax, 1
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
    // 004a3877  7218                   -jb 0x4a3891
    if (cpu.flags.cf)
    {
        goto L_0x004a3891;
    }
    // 004a3879  7627                   -jbe 0x4a38a2
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004a38a2;
    }
    // 004a387b  83f802                 +cmp eax, 2
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
    // 004a387e  0f84dffeffff           -je 0x4a3763
    if (cpu.flags.zf)
    {
        goto L_0x004a3763;
    }
    // 004a3884  b80b000000             -mov eax, 0xb
    cpu.eax = 11 /*0xb*/;
    // 004a3889  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004a388b  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a388c  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a388d  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a388e  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a388f  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a3890  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a3891:
    // 004a3891  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a3893  741a                   -je 0x4a38af
    if (cpu.flags.zf)
    {
        goto L_0x004a38af;
    }
    // 004a3895  b80b000000             -mov eax, 0xb
    cpu.eax = 11 /*0xb*/;
    // 004a389a  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004a389c  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a389d  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a389e  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a389f  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a38a0  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a38a1  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a38a2:
    // 004a38a2  b80a000000             -mov eax, 0xa
    cpu.eax = 10 /*0xa*/;
    // 004a38a7  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004a38a9  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a38aa  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a38ab  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a38ac  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a38ad  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a38ae  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a38af:
    // 004a38af  b80c000000             -mov eax, 0xc
    cpu.eax = 12 /*0xc*/;
L_0x004a38b4:
    // 004a38b4  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004a38b6  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a38b7  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a38b8  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a38b9  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a38ba  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a38bb  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_4a38c0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004a38c0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a38c1  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004a38c2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a38c3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a38c5  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 004a38c7  8b15a0125600           -mov edx, dword ptr [0x5612a0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5640864) /* 0x5612a0 */);
    // 004a38cd  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004a38ce  2eff1534475300         -call dword ptr cs:[0x534734]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457716) /* 0x534734 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a38d5  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a38d7  7405                   -je 0x4a38de
    if (cpu.flags.zf)
    {
        goto L_0x004a38de;
    }
    // 004a38d9  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
L_0x004a38de:
    // 004a38de  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a38df  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a38e0  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a38e1  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_4a38f0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004a38f0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a38f1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a38f2  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004a38f3  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004a38f4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a38f5  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a38f7  81ec18010000           -sub esp, 0x118
    (cpu.esp) -= x86::reg32(x86::sreg32(280 /*0x118*/));
    // 004a38fd  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
    // 004a3900  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 004a3903  b940000000             -mov ecx, 0x40
    cpu.ecx = 64 /*0x40*/;
    // 004a3908  8dbde8feffff           -lea edi, [ebp - 0x118]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-280) /* -0x118 */);
    // 004a390e  8d45e8                 -lea eax, [ebp - 0x18]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004a3911  be502f4a00             -mov esi, 0x4a2f50
    cpu.esi = 4861776 /*0x4a2f50*/;
    // 004a3916  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004a3918  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a3919  8d85e8feffff           -lea eax, [ebp - 0x118]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-280) /* -0x118 */);
    // 004a391f  8955f8                 -mov dword ptr [ebp - 8], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.edx;
    // 004a3922  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a3923  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004a3925  e86e050900             -call 0x533e98
    cpu.esp -= 4;
    sub_533e98(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a392a  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004a392c  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a392e  7526                   -jne 0x4a3956
    if (!cpu.flags.zf)
    {
        goto L_0x004a3956;
    }
    // 004a3930  68ff000000             -push 0xff
    app->getMemory<x86::reg32>(cpu.esp-4) = 255 /*0xff*/;
    cpu.esp -= 4;
    // 004a3935  8d85e8feffff           -lea eax, [ebp - 0x118]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-280) /* -0x118 */);
    // 004a393b  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a393c  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004a393d  2eff1544455300         -call dword ptr cs:[0x534544]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457220) /* 0x534544 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a3944  8d45e8                 -lea eax, [ebp - 0x18]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004a3947  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a3948  8d85e8feffff           -lea eax, [ebp - 0x118]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-280) /* -0x118 */);
    // 004a394e  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a394f  e844050900             -call 0x533e98
    cpu.esp -= 4;
    sub_533e98(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a3954  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
L_0x004a3956:
    // 004a3956  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004a3958  7507                   -jne 0x4a3961
    if (!cpu.flags.zf)
    {
        goto L_0x004a3961;
    }
    // 004a395a  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004a395c  e969000000             -jmp 0x4a39ca
    goto L_0x004a39ca;
L_0x004a3961:
    // 004a3961  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004a3963  e8983f0500             -call 0x4f7900
    cpu.esp -= 4;
    nfs3hp::watcom::nmalloc(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a3968  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 004a396a  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004a396c  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a396e  7457                   -je 0x4a39c7
    if (cpu.flags.zf)
    {
        goto L_0x004a39c7;
    }
    // 004a3970  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a3971  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004a3972  8b4de8                 -mov ecx, dword ptr [ebp - 0x18]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004a3975  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a3976  8d85e8feffff           -lea eax, [ebp - 0x118]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-280) /* -0x118 */);
    // 004a397c  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a397d  e810050900             -call 0x533e92
    cpu.esp -= 4;
    sub_533e92(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a3982  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a3984  743a                   -je 0x4a39c0
    if (cpu.flags.zf)
    {
        goto L_0x004a39c0;
    }
    // 004a3986  8d45f0                 -lea eax, [ebp - 0x10]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004a3989  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a398a  8d45ec                 -lea eax, [ebp - 0x14]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004a398d  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a398e  682cce5300             -push 0x53ce2c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5492268 /*0x53ce2c*/;
    cpu.esp -= 4;
    // 004a3993  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a3994  e8f3040900             -call 0x533e8c
    cpu.esp -= 4;
    sub_533e8c(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a3999  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a399b  7423                   -je 0x4a39c0
    if (cpu.flags.zf)
    {
        goto L_0x004a39c0;
    }
    // 004a399d  8b5df0                 -mov ebx, dword ptr [ebp - 0x10]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004a39a0  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 004a39a2  741c                   -je 0x4a39c0
    if (cpu.flags.zf)
    {
        goto L_0x004a39c0;
    }
    // 004a39a4  bf01000000             -mov edi, 1
    cpu.edi = 1 /*0x1*/;
    // 004a39a9  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004a39ac  897df8                 -mov dword ptr [ebp - 8], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.edi;
    // 004a39af  39d3                   +cmp ebx, edx
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a39b1  7202                   -jb 0x4a39b5
    if (cpu.flags.cf)
    {
        goto L_0x004a39b5;
    }
    // 004a39b3  89d3                   -mov ebx, edx
    cpu.ebx = cpu.edx;
L_0x004a39b5:
    // 004a39b5  8b55ec                 -mov edx, dword ptr [ebp - 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004a39b8  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004a39bb  e870d40300             -call 0x4e0e30
    cpu.esp -= 4;
    nfs3hp::watcom::strncpy(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004a39c0:
    // 004a39c0  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004a39c2  e829400500             -call 0x4f79f0
    cpu.esp -= 4;
    nfs3hp::watcom::nfree(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004a39c7:
    // 004a39c7  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
L_0x004a39ca:
    // 004a39ca  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004a39cc  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a39cd  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a39ce  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a39cf  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a39d0  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a39d1  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4a39f0: native, nfs3hp::graphics::gameMain */

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_4a4020(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004a4020  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a4021  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a4023  a194e85500             -mov eax, dword ptr [0x55e894]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5630100) /* 0x55e894 */);
    // 004a4028  83f801                 +cmp eax, 1
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
    // 004a402b  7209                   -jb 0x4a4036
    if (cpu.flags.cf)
    {
        goto L_0x004a4036;
    }
    // 004a402d  760d                   -jbe 0x4a403c
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004a403c;
    }
    // 004a402f  83f802                 +cmp eax, 2
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
    // 004a4032  740f                   -je 0x4a4043
    if (cpu.flags.zf)
    {
        goto L_0x004a4043;
    }
    // 004a4034  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a4035  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a4036:
    // 004a4036  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a4038  7413                   -je 0x4a404d
    if (cpu.flags.zf)
    {
        goto L_0x004a404d;
    }
    // 004a403a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a403b  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a403c:
    // 004a403c  b89cce5300             -mov eax, 0x53ce9c
    cpu.eax = 5492380 /*0x53ce9c*/;
    // 004a4041  eb05                   -jmp 0x4a4048
    goto L_0x004a4048;
L_0x004a4043:
    // 004a4043  b8b8ce5300             -mov eax, 0x53ceb8
    cpu.eax = 5492408 /*0x53ceb8*/;
L_0x004a4048:
    // 004a4048  e8734cf7ff             -call 0x418cc0
    cpu.esp -= 4;
    nfs3hp::track::shellOpen(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004a404d:
    // 004a404d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a404e  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 */
/* data blob: e4ce5300ecce5300f4ce5300fcce5300 */
void Application::sub_4a4060(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004a4060  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a4061  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a4062  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004a4063  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a4064  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a4066  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004a4068  39c2                   +cmp edx, eax
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
    // 004a406a  7d06                   -jge 0x4a4072
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004a4072;
    }
    // 004a406c  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004a406e  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 004a4070  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
L_0x004a4072:
    // 004a4072  a06b125700             -mov al, byte ptr [0x57126b]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(5706347) /* 0x57126b */);
    // 004a4077  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004a4079  88c3                   -mov bl, al
    cpu.bl = cpu.al;
    // 004a407b  8a9b38295500           -mov bl, byte ptr [ebx + 0x552938]
    cpu.bl = app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(5581112) /* 0x552938 */);
    // 004a4081  81e3ff000000           -and ebx, 0xff
    cpu.ebx &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 004a4087  29ca                   -sub edx, ecx
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004a4089  fec0                   -inc al
    (cpu.al)++;
    // 004a408b  8d7201                 -lea esi, [edx + 1]
    cpu.esi = x86::reg32(cpu.edx + x86::reg32(1) /* 0x1 */);
    // 004a408e  a26b125700             -mov byte ptr [0x57126b], al
    app->getMemory<x86::reg8>(x86::reg32(5706347) /* 0x57126b */) = cpu.al;
    // 004a4093  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 004a4095  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 004a4097  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004a409a  f7fe                   -idiv esi
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.esi);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004a409c  8d0411                 -lea eax, [ecx + edx]
    cpu.eax = x86::reg32(cpu.ecx + cpu.edx * 1);
    // 004a409f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a40a0  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a40a1  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a40a2  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a40a3  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_4a40c0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1;
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 004a40c0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a40c1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a40c2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004a40c3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004a40c4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004a40c5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a40c6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a40c8  83ec08                 -sub esp, 8
    (cpu.esp) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 004a40cb  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004a40cd  c705901b7a0006000000   -mov dword ptr [0x7a1b90], 6
    app->getMemory<x86::reg32>(x86::reg32(8002448) /* 0x7a1b90 */) = 6 /*0x6*/;
    // 004a40d7  83f809                 +cmp eax, 9
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
    // 004a40da  755f                   -jne 0x4a413b
    if (!cpu.flags.zf)
    {
        goto L_0x004a413b;
    }
    // 004a40dc  a184e95500             -mov eax, dword ptr [0x55e984]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5630340) /* 0x55e984 */);
    // 004a40e1  89048d381b7a00         -mov dword ptr [ecx*4 + 0x7a1b38], eax
    app->getMemory<x86::reg32>(x86::reg32(8002360) /* 0x7a1b38 */ + cpu.ecx * 4) = cpu.eax;
    // 004a40e8  8d5001                 -lea edx, [eax + 1]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(1) /* 0x1 */);
    // 004a40eb  be06000000             -mov esi, 6
    cpu.esi = 6 /*0x6*/;
    // 004a40f0  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004a40f2  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004a40f5  f7fe                   -idiv esi
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.esi);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004a40f7  bbcdcccc3f             -mov ebx, 0x3fcccccd
    cpu.ebx = 1070386381 /*0x3fcccccd*/;
    // 004a40fc  bf3c000000             -mov edi, 0x3c
    cpu.edi = 60 /*0x3c*/;
    // 004a4101  891c8d601b7a00         -mov dword ptr [ecx*4 + 0x7a1b60], ebx
    app->getMemory<x86::reg32>(x86::reg32(8002400) /* 0x7a1b60 */ + cpu.ecx * 4) = cpu.ebx;
    // 004a4108  893c8dc01a7a00         -mov dword ptr [ecx*4 + 0x7a1ac0], edi
    app->getMemory<x86::reg32>(x86::reg32(8002240) /* 0x7a1ac0 */ + cpu.ecx * 4) = cpu.edi;
    // 004a410f  be9a010000             -mov esi, 0x19a
    cpu.esi = 410 /*0x19a*/;
    // 004a4114  891584e95500           -mov dword ptr [0x55e984], edx
    app->getMemory<x86::reg32>(x86::reg32(5630340) /* 0x55e984 */) = cpu.edx;
    // 004a411a  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004a411c  ba03000000             -mov edx, 3
    cpu.edx = 3 /*0x3*/;
    // 004a4121  89348d5ce95500         -mov dword ptr [ecx*4 + 0x55e95c], esi
    app->getMemory<x86::reg32>(x86::reg32(5630300) /* 0x55e95c */ + cpu.ecx * 4) = cpu.esi;
    // 004a4128  89048de81a7a00         -mov dword ptr [ecx*4 + 0x7a1ae8], eax
    app->getMemory<x86::reg32>(x86::reg32(8002280) /* 0x7a1ae8 */ + cpu.ecx * 4) = cpu.eax;
    // 004a412f  89148d101b7a00         -mov dword ptr [ecx*4 + 0x7a1b10], edx
    app->getMemory<x86::reg32>(x86::reg32(8002320) /* 0x7a1b10 */ + cpu.ecx * 4) = cpu.edx;
    // 004a4136  e92a020000             -jmp 0x4a4365
    goto L_0x004a4365;
L_0x004a413b:
    // 004a413b  8a1d6b125700           -mov bl, byte ptr [0x57126b]
    cpu.bl = app->getMemory<x86::reg8>(x86::reg32(5706347) /* 0x57126b */);
    // 004a4141  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004a4143  88d8                   -mov al, bl
    cpu.al = cpu.bl;
    // 004a4145  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004a4147  8a9038295500           -mov dl, byte ptr [eax + 0x552938]
    cpu.dl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5581112) /* 0x552938 */);
    // 004a414d  be04000000             -mov esi, 4
    cpu.esi = 4 /*0x4*/;
    // 004a4152  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004a4154  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004a4157  f7fe                   -idiv esi
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.esi);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004a4159  fec3                   -inc bl
    (cpu.bl)++;
    // 004a415b  881d6b125700           -mov byte ptr [0x57126b], bl
    app->getMemory<x86::reg8>(x86::reg32(5706347) /* 0x57126b */) = cpu.bl;
    // 004a4161  83fa03                 +cmp edx, 3
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
    // 004a4164  0f87cd010000           -ja 0x4a4337
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x004a4337;
    }
    // 004a416a  ff2495a4404a00         -jmp dword ptr [edx*4 + 0x4a40a4]
    cpu.ip = app->getMemory<x86::reg32>(4866212 + cpu.edx * 4); goto dynamic_jump;
  case 0x004a4171:
    // 004a4171  bb96000000             -mov ebx, 0x96
    cpu.ebx = 150 /*0x96*/;
    // 004a4176  bac2010000             -mov edx, 0x1c2
    cpu.edx = 450 /*0x1c2*/;
    // 004a417b  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004a417d  891c8d5ce95500         -mov dword ptr [ecx*4 + 0x55e95c], ebx
    app->getMemory<x86::reg32>(x86::reg32(5630300) /* 0x55e95c */ + cpu.ecx * 4) = cpu.ebx;
    // 004a4184  e8d7feffff             -call 0x4a4060
    cpu.esp -= 4;
    sub_4a4060(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a4189  ba04000000             -mov edx, 4
    cpu.edx = 4 /*0x4*/;
    // 004a418e  89048dc01a7a00         -mov dword ptr [ecx*4 + 0x7a1ac0], eax
    app->getMemory<x86::reg32>(x86::reg32(8002240) /* 0x7a1ac0 */ + cpu.ecx * 4) = cpu.eax;
    // 004a4195  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004a419a  e8c1feffff             -call 0x4a4060
    cpu.esp -= 4;
    sub_4a4060(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a419f  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004a41a1  89048de81a7a00         -mov dword ptr [ecx*4 + 0x7a1ae8], eax
    app->getMemory<x86::reg32>(x86::reg32(8002280) /* 0x7a1ae8 */ + cpu.ecx * 4) = cpu.eax;
    // 004a41a8  a06b125700             -mov al, byte ptr [0x57126b]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(5706347) /* 0x57126b */);
    // 004a41ad  88c2                   -mov dl, al
    cpu.dl = cpu.al;
    // 004a41af  fec0                   -inc al
    (cpu.al)++;
    // 004a41b1  a26b125700             -mov byte ptr [0x57126b], al
    app->getMemory<x86::reg8>(x86::reg32(5706347) /* 0x57126b */) = cpu.al;
    // 004a41b6  f6823829550001         +test byte ptr [edx + 0x552938], 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.edx + x86::reg32(5581112) /* 0x552938 */) & 1 /*0x1*/));
    // 004a41bd  7407                   -je 0x4a41c6
    if (cpu.flags.zf)
    {
        goto L_0x004a41c6;
    }
    // 004a41bf  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 004a41c4  eb05                   -jmp 0x4a41cb
    goto L_0x004a41cb;
L_0x004a41c6:
    // 004a41c6  baffffffff             -mov edx, 0xffffffff
    cpu.edx = 4294967295 /*0xffffffff*/;
L_0x004a41cb:
    // 004a41cb  becdcc4c3f             -mov esi, 0x3f4ccccd
    cpu.esi = 1061997773 /*0x3f4ccccd*/;
    // 004a41d0  89148d101b7a00         -mov dword ptr [ecx*4 + 0x7a1b10], edx
    app->getMemory<x86::reg32>(x86::reg32(8002320) /* 0x7a1b10 */ + cpu.ecx * 4) = cpu.edx;
    // 004a41d7  89348d601b7a00         -mov dword ptr [ecx*4 + 0x7a1b60], esi
    app->getMemory<x86::reg32>(x86::reg32(8002400) /* 0x7a1b60 */ + cpu.ecx * 4) = cpu.esi;
    // 004a41de  e954010000             -jmp 0x4a4337
    goto L_0x004a4337;
  case 0x004a41e3:
    // 004a41e3  bf80020000             -mov edi, 0x280
    cpu.edi = 640 /*0x280*/;
    // 004a41e8  bac2010000             -mov edx, 0x1c2
    cpu.edx = 450 /*0x1c2*/;
    // 004a41ed  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004a41ef  893c8d5ce95500         -mov dword ptr [ecx*4 + 0x55e95c], edi
    app->getMemory<x86::reg32>(x86::reg32(5630300) /* 0x55e95c */ + cpu.ecx * 4) = cpu.edi;
    // 004a41f6  e865feffff             -call 0x4a4060
    cpu.esp -= 4;
    sub_4a4060(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a41fb  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 004a41fd  89048dc01a7a00         -mov dword ptr [ecx*4 + 0x7a1ac0], eax
    app->getMemory<x86::reg32>(x86::reg32(8002240) /* 0x7a1ac0 */ + cpu.ecx * 4) = cpu.eax;
    // 004a4204  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004a4209  e852feffff             -call 0x4a4060
    cpu.esp -= 4;
    sub_4a4060(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a420e  f7d8                   -neg eax
    cpu.eax = ~cpu.eax + 1;
    // 004a4210  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004a4212  89048de81a7a00         -mov dword ptr [ecx*4 + 0x7a1ae8], eax
    app->getMemory<x86::reg32>(x86::reg32(8002280) /* 0x7a1ae8 */ + cpu.ecx * 4) = cpu.eax;
    // 004a4219  a06b125700             -mov al, byte ptr [0x57126b]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(5706347) /* 0x57126b */);
    // 004a421e  88c2                   -mov dl, al
    cpu.dl = cpu.al;
    // 004a4220  fec0                   -inc al
    (cpu.al)++;
    // 004a4222  8a9a38295500           -mov bl, byte ptr [edx + 0x552938]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(5581112) /* 0x552938 */);
    // 004a4228  a26b125700             -mov byte ptr [0x57126b], al
    app->getMemory<x86::reg8>(x86::reg32(5706347) /* 0x57126b */) = cpu.al;
    // 004a422d  f6c301                 +test bl, 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.bl & 1 /*0x1*/));
    // 004a4230  7407                   -je 0x4a4239
    if (cpu.flags.zf)
    {
        goto L_0x004a4239;
    }
    // 004a4232  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 004a4237  eb05                   -jmp 0x4a423e
    goto L_0x004a423e;
L_0x004a4239:
    // 004a4239  baffffffff             -mov edx, 0xffffffff
    cpu.edx = 4294967295 /*0xffffffff*/;
L_0x004a423e:
    // 004a423e  b80000803f             -mov eax, 0x3f800000
    cpu.eax = 1065353216 /*0x3f800000*/;
    // 004a4243  89148d101b7a00         -mov dword ptr [ecx*4 + 0x7a1b10], edx
    app->getMemory<x86::reg32>(x86::reg32(8002320) /* 0x7a1b10 */ + cpu.ecx * 4) = cpu.edx;
    // 004a424a  89048d601b7a00         -mov dword ptr [ecx*4 + 0x7a1b60], eax
    app->getMemory<x86::reg32>(x86::reg32(8002400) /* 0x7a1b60 */ + cpu.ecx * 4) = cpu.eax;
    // 004a4251  e9e1000000             -jmp 0x4a4337
    goto L_0x004a4337;
  case 0x004a4256:
    // 004a4256  ba80020000             -mov edx, 0x280
    cpu.edx = 640 /*0x280*/;
    // 004a425b  b896000000             -mov eax, 0x96
    cpu.eax = 150 /*0x96*/;
    // 004a4260  e8fbfdffff             -call 0x4a4060
    cpu.esp -= 4;
    sub_4a4060(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a4265  bac2010000             -mov edx, 0x1c2
    cpu.edx = 450 /*0x1c2*/;
    // 004a426a  89048d5ce95500         -mov dword ptr [ecx*4 + 0x55e95c], eax
    app->getMemory<x86::reg32>(x86::reg32(5630300) /* 0x55e95c */ + cpu.ecx * 4) = cpu.eax;
    // 004a4271  a06b125700             -mov al, byte ptr [0x57126b]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(5706347) /* 0x57126b */);
    // 004a4276  89148dc01a7a00         -mov dword ptr [ecx*4 + 0x7a1ac0], edx
    app->getMemory<x86::reg32>(x86::reg32(8002240) /* 0x7a1ac0 */ + cpu.ecx * 4) = cpu.edx;
    // 004a427d  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004a427f  88c2                   -mov dl, al
    cpu.dl = cpu.al;
    // 004a4281  fec0                   -inc al
    (cpu.al)++;
    // 004a4283  8aba38295500           -mov bh, byte ptr [edx + 0x552938]
    cpu.bh = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(5581112) /* 0x552938 */);
    // 004a4289  a26b125700             -mov byte ptr [0x57126b], al
    app->getMemory<x86::reg8>(x86::reg32(5706347) /* 0x57126b */) = cpu.al;
    // 004a428e  f6c701                 +test bh, 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.bh & 1 /*0x1*/));
    // 004a4291  7407                   -je 0x4a429a
    if (cpu.flags.zf)
    {
        goto L_0x004a429a;
    }
    // 004a4293  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004a4298  eb05                   -jmp 0x4a429f
    goto L_0x004a429f;
L_0x004a429a:
    // 004a429a  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
L_0x004a429f:
    // 004a429f  ba04000000             -mov edx, 4
    cpu.edx = 4 /*0x4*/;
    // 004a42a4  89048de81a7a00         -mov dword ptr [ecx*4 + 0x7a1ae8], eax
    app->getMemory<x86::reg32>(x86::reg32(8002280) /* 0x7a1ae8 */ + cpu.ecx * 4) = cpu.eax;
    // 004a42ab  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004a42b0  bbcdcc4c3f             -mov ebx, 0x3f4ccccd
    cpu.ebx = 1061997773 /*0x3f4ccccd*/;
    // 004a42b5  e8a6fdffff             -call 0x4a4060
    cpu.esp -= 4;
    sub_4a4060(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a42ba  f7d8                   +neg eax
    {
        x86::reg32 tmp1 = 0;
        x86::reg32& tmp2 = cpu.eax;
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp2 = result;
        cpu.set_szp(tmp2);
    }
    // 004a42bc  891c8d601b7a00         -mov dword ptr [ecx*4 + 0x7a1b60], ebx
    app->getMemory<x86::reg32>(x86::reg32(8002400) /* 0x7a1b60 */ + cpu.ecx * 4) = cpu.ebx;
    // 004a42c3  89048d101b7a00         -mov dword ptr [ecx*4 + 0x7a1b10], eax
    app->getMemory<x86::reg32>(x86::reg32(8002320) /* 0x7a1b10 */ + cpu.ecx * 4) = cpu.eax;
    // 004a42ca  eb6b                   -jmp 0x4a4337
    goto L_0x004a4337;
  case 0x004a42cc:
    // 004a42cc  b896000000             -mov eax, 0x96
    cpu.eax = 150 /*0x96*/;
    // 004a42d1  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004a42d3  e888fdffff             -call 0x4a4060
    cpu.esp -= 4;
    sub_4a4060(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a42d8  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004a42da  89048d5ce95500         -mov dword ptr [ecx*4 + 0x55e95c], eax
    app->getMemory<x86::reg32>(x86::reg32(5630300) /* 0x55e95c */ + cpu.ecx * 4) = cpu.eax;
    // 004a42e1  a06b125700             -mov al, byte ptr [0x57126b]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(5706347) /* 0x57126b */);
    // 004a42e6  88c2                   -mov dl, al
    cpu.dl = cpu.al;
    // 004a42e8  fec0                   -inc al
    (cpu.al)++;
    // 004a42ea  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 004a42ec  a26b125700             -mov byte ptr [0x57126b], al
    app->getMemory<x86::reg8>(x86::reg32(5706347) /* 0x57126b */) = cpu.al;
    // 004a42f1  8a8238295500           -mov al, byte ptr [edx + 0x552938]
    cpu.al = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(5581112) /* 0x552938 */);
    // 004a42f7  89348dc01a7a00         -mov dword ptr [ecx*4 + 0x7a1ac0], esi
    app->getMemory<x86::reg32>(x86::reg32(8002240) /* 0x7a1ac0 */ + cpu.ecx * 4) = cpu.esi;
    // 004a42fe  a801                   +test al, 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.al & 1 /*0x1*/));
    // 004a4300  7407                   -je 0x4a4309
    if (cpu.flags.zf)
    {
        goto L_0x004a4309;
    }
    // 004a4302  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004a4307  eb05                   -jmp 0x4a430e
    goto L_0x004a430e;
L_0x004a4309:
    // 004a4309  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
L_0x004a430e:
    // 004a430e  ba04000000             -mov edx, 4
    cpu.edx = 4 /*0x4*/;
    // 004a4313  89048de81a7a00         -mov dword ptr [ecx*4 + 0x7a1ae8], eax
    app->getMemory<x86::reg32>(x86::reg32(8002280) /* 0x7a1ae8 */ + cpu.ecx * 4) = cpu.eax;
    // 004a431a  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004a431f  bf0000803f             -mov edi, 0x3f800000
    cpu.edi = 1065353216 /*0x3f800000*/;
    // 004a4324  e837fdffff             -call 0x4a4060
    cpu.esp -= 4;
    sub_4a4060(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a4329  89048d101b7a00         -mov dword ptr [ecx*4 + 0x7a1b10], eax
    app->getMemory<x86::reg32>(x86::reg32(8002320) /* 0x7a1b10 */ + cpu.ecx * 4) = cpu.eax;
    // 004a4330  893c8d601b7a00         -mov dword ptr [ecx*4 + 0x7a1b60], edi
    app->getMemory<x86::reg32>(x86::reg32(8002400) /* 0x7a1b60 */ + cpu.ecx * 4) = cpu.edi;
L_0x004a4337:
    // 004a4337  8a1d6b125700           -mov bl, byte ptr [0x57126b]
    cpu.bl = app->getMemory<x86::reg8>(x86::reg32(5706347) /* 0x57126b */);
    // 004a433d  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004a433f  88d8                   -mov al, bl
    cpu.al = cpu.bl;
    // 004a4341  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004a4343  8a9038295500           -mov dl, byte ptr [eax + 0x552938]
    cpu.dl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5581112) /* 0x552938 */);
    // 004a4349  8b35901b7a00           -mov esi, dword ptr [0x7a1b90]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(8002448) /* 0x7a1b90 */);
    // 004a434f  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004a4351  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004a4354  f7fe                   -idiv esi
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.esi);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004a4356  fec3                   -inc bl
    (cpu.bl)++;
    // 004a4358  881d6b125700           -mov byte ptr [0x57126b], bl
    app->getMemory<x86::reg8>(x86::reg32(5706347) /* 0x57126b */) = cpu.bl;
    // 004a435e  89148d381b7a00         -mov dword ptr [ecx*4 + 0x7a1b38], edx
    app->getMemory<x86::reg32>(x86::reg32(8002360) /* 0x7a1b38 */ + cpu.ecx * 4) = cpu.edx;
L_0x004a4365:
    // 004a4365  d9048d601b7a00         -fld dword ptr [ecx*4 + 0x7a1b60]
    fp0 = x86::Float(app->getMemory<float>(x86::reg32(8002400) /* 0x7a1b60 */ + cpu.ecx * 4));
    // 004a436c  d9c0                   -fld st(0)
    fp1 = x86::Float(fp0);
    // 004a436e  dc0d04cf5300           -fmul qword ptr [0x53cf04]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<double>(x86::reg32(5492484) /* 0x53cf04 */)));
    // 004a4374  ddd9                   -fstp st(1)
    fp0 = x86::Float(fp1);
    // 004a4376  d91c8d601b7a00         -fstp dword ptr [ecx*4 + 0x7a1b60]
    app->getMemory<float>(x86::reg32(8002400) /* 0x7a1b60 */ + cpu.ecx * 4) = float(fp0);
    // 004a437d  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004a437f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a4380  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a4381  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a4382  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a4383  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a4384  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a4385  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* sub_4a4390: native, nfs3hp::frontend::effectsLogoTextureStart */

/* sub_4a4410: native, nfs3hp::render::bouncers */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_4a46f0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004a46f0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a46f1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a46f2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004a46f3  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a46f4  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a46f6  8b0dbcd26f00           -mov ecx, dword ptr [0x6fd2bc]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7328444) /* 0x6fd2bc */);
    // 004a46fc  bba06040ff             -mov ebx, 0xff4060a0
    cpu.ebx = 4282409120 /*0xff4060a0*/;
    // 004a4701  01c9                   -add ecx, ecx
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004a4703  83f801                 -cmp eax, 1
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
    // 004a4706  8b81a64f5500           -mov eax, dword ptr [ecx + 0x554fa6]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(5590950) /* 0x554fa6 */);
    // 004a470c  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 004a470f  e83cd10200             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a4714  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004a4716  ba40cf5300             -mov edx, 0x53cf40
    cpu.edx = 5492544 /*0x53cf40*/;
    // 004a471b  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004a471d  e8fec00300             -call 0x4e0820
    cpu.esp -= 4;
    nfs3hp::watcom::strstr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a4722  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a4724  740a                   -je 0x4a4730
    if (cpu.flags.zf)
    {
        goto L_0x004a4730;
    }
    // 004a4726  bb29b8fbff             -mov ebx, 0xfffbb829
    cpu.ebx = 4294686761 /*0xfffbb829*/;
    // 004a472b  e985000000             -jmp 0x4a47b5
    goto L_0x004a47b5;
L_0x004a4730:
    // 004a4730  ba48cf5300             -mov edx, 0x53cf48
    cpu.edx = 5492552 /*0x53cf48*/;
    // 004a4735  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004a4737  e8e4c00300             -call 0x4e0820
    cpu.esp -= 4;
    nfs3hp::watcom::strstr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a473c  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a473e  740c                   -je 0x4a474c
    if (cpu.flags.zf)
    {
        goto L_0x004a474c;
    }
    // 004a4740  bbffff64ff             -mov ebx, 0xff64ffff
    cpu.ebx = 4284809215 /*0xff64ffff*/;
    // 004a4745  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 004a4747  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a4748  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a4749  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a474a  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a474b  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a474c:
    // 004a474c  ba50cf5300             -mov edx, 0x53cf50
    cpu.edx = 5492560 /*0x53cf50*/;
    // 004a4751  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004a4753  e8c8c00300             -call 0x4e0820
    cpu.esp -= 4;
    nfs3hp::watcom::strstr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a4758  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a475a  740c                   -je 0x4a4768
    if (cpu.flags.zf)
    {
        goto L_0x004a4768;
    }
    // 004a475c  bbff0000ff             -mov ebx, 0xff0000ff
    cpu.ebx = 4278190335 /*0xff0000ff*/;
    // 004a4761  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 004a4763  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a4764  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a4765  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a4766  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a4767  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a4768:
    // 004a4768  ba5ccf5300             -mov edx, 0x53cf5c
    cpu.edx = 5492572 /*0x53cf5c*/;
    // 004a476d  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004a476f  e8acc00300             -call 0x4e0820
    cpu.esp -= 4;
    nfs3hp::watcom::strstr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a4774  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a4776  740c                   -je 0x4a4784
    if (cpu.flags.zf)
    {
        goto L_0x004a4784;
    }
    // 004a4778  bbff0000ff             -mov ebx, 0xff0000ff
    cpu.ebx = 4278190335 /*0xff0000ff*/;
    // 004a477d  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 004a477f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a4780  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a4781  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a4782  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a4783  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a4784:
    // 004a4784  ba68cf5300             -mov edx, 0x53cf68
    cpu.edx = 5492584 /*0x53cf68*/;
    // 004a4789  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004a478b  e890c00300             -call 0x4e0820
    cpu.esp -= 4;
    nfs3hp::watcom::strstr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a4790  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a4792  740c                   -je 0x4a47a0
    if (cpu.flags.zf)
    {
        goto L_0x004a47a0;
    }
    // 004a4794  bb2820c0ff             -mov ebx, 0xffc02028
    cpu.ebx = 4290781224 /*0xffc02028*/;
    // 004a4799  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 004a479b  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a479c  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a479d  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a479e  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a479f  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a47a0:
    // 004a47a0  ba70cf5300             -mov edx, 0x53cf70
    cpu.edx = 5492592 /*0x53cf70*/;
    // 004a47a5  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004a47a7  e874c00300             -call 0x4e0820
    cpu.esp -= 4;
    nfs3hp::watcom::strstr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a47ac  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a47ae  7405                   -je 0x4a47b5
    if (cpu.flags.zf)
    {
        goto L_0x004a47b5;
    }
    // 004a47b0  bbffeebbff             -mov ebx, 0xffbbeeff
    cpu.ebx = 4290506495 /*0xffbbeeff*/;
L_0x004a47b5:
    // 004a47b5  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 004a47b7  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a47b8  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a47b9  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a47ba  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a47bb  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4a47c0: native, nfs3hp::frontend::effectsBandDraw */

/* sub_4a49c0: native, nfs3hp::frontend::effectsScreenDraw */

/* sub_4a4a10: native, nfs3hp::frontend::effectsScreenStart */

/* sub_4a4ba0: native, nfs3hp::frontend::effectsLoadScreenArt */

/* sub_4a4c20: native, nfs3hp::frontend::effectsLogoDraw */

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_4a5080(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004a5080  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a5081  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a5082  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004a5083  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004a5084  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a5085  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a5087  81ec68010000           -sub esp, 0x168
    (cpu.esp) -= x86::reg32(x86::sreg32(360 /*0x168*/));
    // 004a508d  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
    // 004a5090  8955f0                 -mov dword ptr [ebp - 0x10], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.edx;
    // 004a5093  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 004a5095  b8a01f7a00             -mov eax, 0x7a1fa0
    cpu.eax = 8003488 /*0x7a1fa0*/;
    // 004a509a  8955ec                 -mov dword ptr [ebp - 0x14], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.edx;
    // 004a509d  ba7c000000             -mov edx, 0x7c
    cpu.edx = 124 /*0x7c*/;
    // 004a50a2  e865b60300             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004a50a7:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a50a7  8b4df4                 -mov ecx, dword ptr [ebp - 0xc]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004a50aa  49                     +dec ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 004a50ab  894df4                 -mov dword ptr [ebp - 0xc], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.ecx;
    // 004a50ae  0f84cc030000           -je 0x4a5480
    if (cpu.flags.zf)
    {
        goto L_0x004a5480;
    }
    // 004a50b4  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 004a50b6  31c9                   +xor ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx))));
    // 004a50b8  8975fc                 -mov dword ptr [ebp - 4], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.esi;
    // 004a50bb  eb05                   -jmp 0x4a50c2
    goto L_0x004a50c2;
L_0x004a50bd:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a50bd  83f905                 +cmp ecx, 5
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(5 /*0x5*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a50c0  7d27                   -jge 0x4a50e9
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004a50e9;
    }
L_0x004a50c2:
    // 004a50c2  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004a50c5  8b7df0                 -mov edi, dword ptr [ebp - 0x10]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004a50c8  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004a50cb  01f8                   -add eax, edi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edi));
    // 004a50cd  8b148d3cec5500         -mov edx, dword ptr [ecx*4 + 0x55ec3c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5631036) /* 0x55ec3c */ + cpu.ecx * 4);
    // 004a50d4  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 004a50d6  e8e54d0400             -call 0x4e9ec0
    cpu.esp -= 4;
    nfs3hp::watcom::stricmp(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a50db  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a50dd  7507                   -jne 0x4a50e6
    if (!cpu.flags.zf)
    {
        goto L_0x004a50e6;
    }
    // 004a50df  c745ec01000000         -mov dword ptr [ebp - 0x14], 1
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = 1 /*0x1*/;
L_0x004a50e6:
    // 004a50e6  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004a50e7  ebd4                   -jmp 0x4a50bd
    goto L_0x004a50bd;
L_0x004a50e9:
    // 004a50e9  837dec00               +cmp dword ptr [ebp - 0x14], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a50ed  0f856c000000           -jne 0x4a515f
    if (!cpu.flags.zf)
    {
        goto L_0x004a515f;
    }
    // 004a50f3  31c9                   +xor ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx))));
    // 004a50f5  eb06                   -jmp 0x4a50fd
    goto L_0x004a50fd;
L_0x004a50f7:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a50f7  41                     -inc ecx
    (cpu.ecx)++;
    // 004a50f8  83f921                 +cmp ecx, 0x21
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(33 /*0x21*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a50fb  7d62                   -jge 0x4a515f
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004a515f;
    }
L_0x004a50fd:
    // 004a50fd  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004a5100  8b5df0                 -mov ebx, dword ptr [ebp - 0x10]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004a5103  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004a5106  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 004a5108  8b148d34eb5500         -mov edx, dword ptr [ecx*4 + 0x55eb34]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5630772) /* 0x55eb34 */ + cpu.ecx * 4);
    // 004a510f  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 004a5111  e8aa4d0400             -call 0x4e9ec0
    cpu.esp -= 4;
    nfs3hp::watcom::stricmp(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a5116  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a5118  75dd                   -jne 0x4a50f7
    if (!cpu.flags.zf)
    {
        goto L_0x004a50f7;
    }
    // 004a511a  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 004a511c  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004a511e  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004a5121  c1e205                 +shl edx, 5
    {
        x86::reg8 tmp = 5 /*0x5*/ % 32;
        x86::reg32& op = cpu.edx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 004a5124  1bc2                   -sbb eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 004a5126  c1f805                 -sar eax, 5
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (5 /*0x5*/ % 32));
    // 004a5129  8d5101                 -lea edx, [ecx + 1]
    cpu.edx = x86::reg32(cpu.ecx + x86::reg32(1) /* 0x1 */);
    // 004a512c  8d1c8500000000         -lea ebx, [eax*4]
    cpu.ebx = x86::reg32(cpu.eax * 4);
    // 004a5133  b920000000             -mov ecx, 0x20
    cpu.ecx = 32 /*0x20*/;
    // 004a5138  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004a513a  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004a513d  f7f9                   -idiv ecx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ecx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004a513f  4a                     -dec edx
    (cpu.edx)--;
    // 004a5140  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004a5145  88d1                   -mov cl, dl
    cpu.cl = cpu.dl;
    // 004a5147  8bb32ceb5500           -mov esi, dword ptr [ebx + 0x55eb2c]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(5630764) /* 0x55eb2c */);
    // 004a514d  d3e0                   -shl eax, cl
    cpu.eax <<= cpu.cl % 32;
    // 004a514f  bf01000000             -mov edi, 1
    cpu.edi = 1 /*0x1*/;
    // 004a5154  09c6                   -or esi, eax
    cpu.esi |= x86::reg32(x86::sreg32(cpu.eax));
    // 004a5156  897dfc                 -mov dword ptr [ebp - 4], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edi;
    // 004a5159  89b32ceb5500           -mov dword ptr [ebx + 0x55eb2c], esi
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(5630764) /* 0x55eb2c */) = cpu.esi;
L_0x004a515f:
    // 004a515f  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004a5162  8b55f0                 -mov edx, dword ptr [ebp - 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004a5165  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004a5168  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004a516a  8d7d98                 -lea edi, [ebp - 0x68]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-104) /* -0x68 */);
    // 004a516d  8b30                   -mov esi, dword ptr [eax]
    cpu.esi = app->getMemory<x86::reg32>(cpu.eax);
    // 004a516f  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
L_0x004a5170:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a5170  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 004a5172  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 004a5174  3c00                   +cmp al, 0
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
    // 004a5176  7410                   -je 0x4a5188
    if (cpu.flags.zf)
    {
        goto L_0x004a5188;
    }
    // 004a5178  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 004a517b  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004a517e  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 004a5181  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004a5184  3c00                   +cmp al, 0
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
    // 004a5186  75e8                   -jne 0x4a5170
    if (!cpu.flags.zf)
    {
        goto L_0x004a5170;
    }
L_0x004a5188:
    // 004a5188  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5189  8d4598                 -lea eax, [ebp - 0x68]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-104) /* -0x68 */);
    // 004a518c  8b4dfc                 -mov ecx, dword ptr [ebp - 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004a518f  e88c2e0500             -call 0x4f8020
    cpu.esp -= 4;
    nfs3hp::watcom::strupr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a5194  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004a5196  7560                   -jne 0x4a51f8
    if (!cpu.flags.zf)
    {
        goto L_0x004a51f8;
    }
    // 004a5198  bb04000000             -mov ebx, 4
    cpu.ebx = 4 /*0x4*/;
    // 004a519d  ba84d65300             -mov edx, 0x53d684
    cpu.edx = 5494404 /*0x53d684*/;
    // 004a51a2  8d4598                 -lea eax, [ebp - 0x68]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-104) /* -0x68 */);
    // 004a51a5  e8e6610400             -call 0x4eb390
    cpu.esp -= 4;
    nfs3hp::watcom::strncmp(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a51aa  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a51ac  754a                   -jne 0x4a51f8
    if (!cpu.flags.zf)
    {
        goto L_0x004a51f8;
    }
    // 004a51ae  8d45e8                 -lea eax, [ebp - 0x18]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004a51b1  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a51b2  688cd65300             -push 0x53d68c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5494412 /*0x53d68c*/;
    cpu.esp -= 4;
    // 004a51b7  8d459c                 -lea eax, [ebp - 0x64]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-100) /* -0x64 */);
    // 004a51ba  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a51bb  e888b30400             -call 0x4f0548
    cpu.esp -= 4;
    nfs3hp::watcom::sscanf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a51c0  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 004a51c3  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a51c5  7431                   -je 0x4a51f8
    if (cpu.flags.zf)
    {
        goto L_0x004a51f8;
    }
    // 004a51c7  8b45e8                 -mov eax, dword ptr [ebp - 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004a51ca  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004a51cc  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004a51cf  c1e205                 +shl edx, 5
    {
        x86::reg8 tmp = 5 /*0x5*/ % 32;
        x86::reg32& op = cpu.edx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 004a51d2  1bc2                   -sbb eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 004a51d4  c1f805                 -sar eax, 5
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (5 /*0x5*/ % 32));
    // 004a51d7  8a14852ceb5500         -mov dl, byte ptr [eax*4 + 0x55eb2c]
    cpu.dl = app->getMemory<x86::reg8>(x86::reg32(5630764) /* 0x55eb2c */ + cpu.eax * 4);
    // 004a51de  80ca20                 -or dl, 0x20
    cpu.dl |= x86::reg8(x86::sreg8(32 /*0x20*/));
    // 004a51e1  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 004a51e6  8814852ceb5500         -mov byte ptr [eax*4 + 0x55eb2c], dl
    app->getMemory<x86::reg8>(x86::reg32(5630764) /* 0x55eb2c */ + cpu.eax * 4) = cpu.dl;
    // 004a51ed  8b45e8                 -mov eax, dword ptr [ebp - 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004a51f0  895dfc                 -mov dword ptr [ebp - 4], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ebx;
    // 004a51f3  a3bcfd5500             -mov dword ptr [0x55fdbc], eax
    app->getMemory<x86::reg32>(x86::reg32(5635516) /* 0x55fdbc */) = cpu.eax;
L_0x004a51f8:
    // 004a51f8  8b75fc                 -mov esi, dword ptr [ebp - 4]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004a51fb  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 004a51fd  0f8531010000           -jne 0x4a5334
    if (!cpu.flags.zf)
    {
        goto L_0x004a5334;
    }
    // 004a5203  8975f8                 -mov dword ptr [ebp - 8], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.esi;
    // 004a5206  e9d8000000             -jmp 0x4a52e3
    goto L_0x004a52e3;
L_0x004a520b:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a520b  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004a520d  0f859d000000           -jne 0x4a52b0
    if (!cpu.flags.zf)
    {
        goto L_0x004a52b0;
    }
    // 004a5213  89f7                   -mov edi, esi
    cpu.edi = cpu.esi;
    // 004a5215  29c9                   -sub ecx, ecx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004a5217  49                     -dec ecx
    (cpu.ecx)--;
    // 004a5218  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004a521a  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 004a521c  f7d1                   -not ecx
    cpu.ecx = ~cpu.ecx;
    // 004a521e  49                     -dec ecx
    (cpu.ecx)--;
    // 004a521f  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004a5221  0f84b3000000           -je 0x4a52da
    if (cpu.flags.zf)
    {
        goto L_0x004a52da;
    }
    // 004a5227  89f7                   -mov edi, esi
    cpu.edi = cpu.esi;
    // 004a5229  29c9                   -sub ecx, ecx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004a522b  49                     -dec ecx
    (cpu.ecx)--;
    // 004a522c  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004a522e  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 004a5230  f7d1                   -not ecx
    cpu.ecx = ~cpu.ecx;
    // 004a5232  49                     -dec ecx
    (cpu.ecx)--;
    // 004a5233  83f93c                 +cmp ecx, 0x3c
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(60 /*0x3c*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a5236  0f839e000000           -jae 0x4a52da
    if (!cpu.flags.cf)
    {
        goto L_0x004a52da;
    }
    // 004a523c  bfa01f7a00             -mov edi, 0x7a1fa0
    cpu.edi = 8003488 /*0x7a1fa0*/;
    // 004a5241  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 004a5246  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
L_0x004a5247:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a5247  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 004a5249  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 004a524b  3c00                   +cmp al, 0
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
    // 004a524d  7410                   -je 0x4a525f
    if (cpu.flags.zf)
    {
        goto L_0x004a525f;
    }
    // 004a524f  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 004a5252  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004a5255  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 004a5258  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004a525b  3c00                   +cmp al, 0
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
    // 004a525d  75e8                   -jne 0x4a5247
    if (!cpu.flags.zf)
    {
        goto L_0x004a5247;
    }
L_0x004a525f:
    // 004a525f  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5260  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 004a5263  e972000000             -jmp 0x4a52da
    goto L_0x004a52da;
L_0x004a5268:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a5268  68dc1f7a00             -push 0x7a1fdc
    app->getMemory<x86::reg32>(cpu.esp-4) = 8003548 /*0x7a1fdc*/;
    cpu.esp -= 4;
    // 004a526d  688cd65300             -push 0x53d68c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5494412 /*0x53d68c*/;
    cpu.esp -= 4;
    // 004a5272  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004a5273  e8d0b20400             -call 0x4f0548
    cpu.esp -= 4;
    nfs3hp::watcom::sscanf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a5278  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 004a527b  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a527d  745b                   -je 0x4a52da
    if (cpu.flags.zf)
    {
        goto L_0x004a52da;
    }
    // 004a527f  c745fc01000000         -mov dword ptr [ebp - 4], 1
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = 1 /*0x1*/;
    // 004a5286  eb52                   -jmp 0x4a52da
    goto L_0x004a52da;
L_0x004a5288:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a5288  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004a528b  8b5df0                 -mov ebx, dword ptr [ebp - 0x10]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004a528e  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004a5291  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 004a5293  be01000000             -mov esi, 1
    cpu.esi = 1 /*0x1*/;
    // 004a5298  bb0b000000             -mov ebx, 0xb
    cpu.ebx = 11 /*0xb*/;
    // 004a529d  8b10                   -mov edx, dword ptr [eax]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax);
    // 004a529f  b8e01f7a00             -mov eax, 0x7a1fe0
    cpu.eax = 8003552 /*0x7a1fe0*/;
    // 004a52a4  01ca                   +add edx, ecx
    {
        x86::reg32& tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ecx));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004a52a6  8975fc                 -mov dword ptr [ebp - 4], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.esi;
    // 004a52a9  e882bb0300             -call 0x4e0e30
    cpu.esp -= 4;
    nfs3hp::watcom::strncpy(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a52ae  eb2a                   -jmp 0x4a52da
    goto L_0x004a52da;
L_0x004a52b0:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a52b0  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004a52b3  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004a52b6  05ec1f7a00             -add eax, 0x7a1fec
    (cpu.eax) += x86::reg32(x86::sreg32(8003564 /*0x7a1fec*/));
    // 004a52bb  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a52bc  8d4598                 -lea eax, [ebp - 0x68]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-104) /* -0x68 */);
    // 004a52bf  688cd65300             -push 0x53d68c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5494412 /*0x53d68c*/;
    cpu.esp -= 4;
    // 004a52c4  01f8                   -add eax, edi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edi));
    // 004a52c6  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a52c7  e87cb20400             -call 0x4f0548
    cpu.esp -= 4;
    nfs3hp::watcom::sscanf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a52cc  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 004a52cf  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a52d1  7407                   -je 0x4a52da
    if (cpu.flags.zf)
    {
        goto L_0x004a52da;
    }
    // 004a52d3  c745fc01000000         -mov dword ptr [ebp - 4], 1
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = 1 /*0x1*/;
L_0x004a52da:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a52da  ff45f8                 -inc dword ptr [ebp - 8]
    (app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */))++;
    // 004a52dd  837df80c               +cmp dword ptr [ebp - 8], 0xc
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(12 /*0xc*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a52e1  7d51                   -jge 0x4a5334
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004a5334;
    }
L_0x004a52e3:
    // 004a52e3  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004a52e6  8b3c9550ec5500         -mov edi, dword ptr [edx*4 + 0x55ec50]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(5631056) /* 0x55ec50 */ + cpu.edx * 4);
    // 004a52ed  29c9                   -sub ecx, ecx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004a52ef  49                     -dec ecx
    (cpu.ecx)--;
    // 004a52f0  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004a52f2  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 004a52f4  f7d1                   -not ecx
    cpu.ecx = ~cpu.ecx;
    // 004a52f6  49                     -dec ecx
    (cpu.ecx)--;
    // 004a52f7  8d4598                 -lea eax, [ebp - 0x68]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-104) /* -0x68 */);
    // 004a52fa  89cb                   -mov ebx, ecx
    cpu.ebx = cpu.ecx;
    // 004a52fc  8b149550ec5500         -mov edx, dword ptr [edx*4 + 0x55ec50]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5631056) /* 0x55ec50 */ + cpu.edx * 4);
    // 004a5303  89cf                   -mov edi, ecx
    cpu.edi = cpu.ecx;
    // 004a5305  e886600400             -call 0x4eb390
    cpu.esp -= 4;
    nfs3hp::watcom::strncmp(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a530a  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a530c  75cc                   -jne 0x4a52da
    if (!cpu.flags.zf)
    {
        goto L_0x004a52da;
    }
    // 004a530e  8d4598                 -lea eax, [ebp - 0x68]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-104) /* -0x68 */);
    // 004a5311  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004a5314  8d3408                 -lea esi, [eax + ecx]
    cpu.esi = x86::reg32(cpu.eax + cpu.ecx * 1);
    // 004a5317  83fa01                 +cmp edx, 1
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
    // 004a531a  0f82ebfeffff           -jb 0x4a520b
    if (cpu.flags.cf)
    {
        goto L_0x004a520b;
    }
    // 004a5320  0f8642ffffff           -jbe 0x4a5268
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004a5268;
    }
    // 004a5326  83fa02                 +cmp edx, 2
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
    // 004a5329  0f8459ffffff           -je 0x4a5288
    if (cpu.flags.zf)
    {
        goto L_0x004a5288;
    }
    // 004a532f  e97cffffff             -jmp 0x4a52b0
    goto L_0x004a52b0;
L_0x004a5334:
    // 004a5334  837dfc00               +cmp dword ptr [ebp - 4], 0
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
    // 004a5338  740a                   -je 0x4a5344
    if (cpu.flags.zf)
    {
        goto L_0x004a5344;
    }
    // 004a533a  837dec00               +cmp dword ptr [ebp - 0x14], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a533e  0f8463fdffff           -je 0x4a50a7
    if (cpu.flags.zf)
    {
        goto L_0x004a50a7;
    }
L_0x004a5344:
    // 004a5344  6890d65300             -push 0x53d690
    app->getMemory<x86::reg32>(cpu.esp-4) = 5494416 /*0x53d690*/;
    cpu.esp -= 4;
    // 004a5349  689cd65300             -push 0x53d69c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5494428 /*0x53d69c*/;
    cpu.esp -= 4;
    // 004a534e  68a8d65300             -push 0x53d6a8
    app->getMemory<x86::reg32>(cpu.esp-4) = 5494440 /*0x53d6a8*/;
    cpu.esp -= 4;
    // 004a5353  68acd65300             -push 0x53d6ac
    app->getMemory<x86::reg32>(cpu.esp-4) = 5494444 /*0x53d6ac*/;
    cpu.esp -= 4;
    // 004a5358  68a01b7a00             -push 0x7a1ba0
    app->getMemory<x86::reg32>(cpu.esp-4) = 8002464 /*0x7a1ba0*/;
    cpu.esp -= 4;
    // 004a535d  8b5dec                 -mov ebx, dword ptr [ebp - 0x14]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004a5360  e82ba30300             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a5365  83c414                 -add esp, 0x14
    (cpu.esp) += x86::reg32(x86::sreg32(20 /*0x14*/));
    // 004a5368  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 004a536a  7532                   -jne 0x4a539e
    if (!cpu.flags.zf)
    {
        goto L_0x004a539e;
    }
    // 004a536c  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004a536f  8b75f0                 -mov esi, dword ptr [ebp - 0x10]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004a5372  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004a5375  01f0                   -add eax, esi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.esi));
    // 004a5377  8b38                   -mov edi, dword ptr [eax]
    cpu.edi = app->getMemory<x86::reg32>(cpu.eax);
    // 004a5379  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004a537a  680cd75300             -push 0x53d70c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5494540 /*0x53d70c*/;
    cpu.esp -= 4;
    // 004a537f  bfa01b7a00             -mov edi, 0x7a1ba0
    cpu.edi = 8002464 /*0x7a1ba0*/;
    // 004a5384  29c9                   -sub ecx, ecx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004a5386  49                     -dec ecx
    (cpu.ecx)--;
    // 004a5387  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004a5389  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 004a538b  f7d1                   -not ecx
    cpu.ecx = ~cpu.ecx;
    // 004a538d  49                     -dec ecx
    (cpu.ecx)--;
    // 004a538e  b8a01b7a00             -mov eax, 0x7a1ba0
    cpu.eax = 8002464 /*0x7a1ba0*/;
    // 004a5393  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004a5395  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a5396  e8f5a20300             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a539b  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
L_0x004a539e:
    // 004a539e  6824d75300             -push 0x53d724
    app->getMemory<x86::reg32>(cpu.esp-4) = 5494564 /*0x53d724*/;
    cpu.esp -= 4;
    // 004a53a3  bfa01b7a00             -mov edi, 0x7a1ba0
    cpu.edi = 8002464 /*0x7a1ba0*/;
    // 004a53a8  29c9                   -sub ecx, ecx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004a53aa  49                     -dec ecx
    (cpu.ecx)--;
    // 004a53ab  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004a53ad  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 004a53af  f7d1                   -not ecx
    cpu.ecx = ~cpu.ecx;
    // 004a53b1  49                     -dec ecx
    (cpu.ecx)--;
    // 004a53b2  b8a01b7a00             -mov eax, 0x7a1ba0
    cpu.eax = 8002464 /*0x7a1ba0*/;
    // 004a53b7  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004a53b9  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a53ba  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 004a53bc  e8cfa20300             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a53c1  83c408                 +add esp, 8
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(8 /*0x8*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004a53c4  e977000000             -jmp 0x4a5440
    goto L_0x004a5440;
L_0x004a53c9:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a53c9  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x004a53cb:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a53cb  8d1cb500000000         -lea ebx, [esi*4]
    cpu.ebx = x86::reg32(cpu.esi * 4);
    // 004a53d2  8bbb34eb5500           -mov edi, dword ptr [ebx + 0x55eb34]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(5630772) /* 0x55eb34 */);
    // 004a53d8  29c9                   -sub ecx, ecx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004a53da  49                     -dec ecx
    (cpu.ecx)--;
    // 004a53db  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004a53dd  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 004a53df  f7d1                   -not ecx
    cpu.ecx = ~cpu.ecx;
    // 004a53e1  49                     -dec ecx
    (cpu.ecx)--;
    // 004a53e2  b80e000000             -mov eax, 0xe
    cpu.eax = 14 /*0xe*/;
    // 004a53e7  29c8                   -sub eax, ecx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004a53e9  39c2                   +cmp edx, eax
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
    // 004a53eb  730b                   -jae 0x4a53f8
    if (!cpu.flags.cf)
    {
        goto L_0x004a53f8;
    }
    // 004a53ed  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004a53ee  c6842a97feffff20       -mov byte ptr [edx + ebp - 0x169], 0x20
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(-361) /* -0x169 */ + cpu.ebp * 1) = 32 /*0x20*/;
    // 004a53f6  ebd3                   -jmp 0x4a53cb
    goto L_0x004a53cb;
L_0x004a53f8:
    // 004a53f8  8b83b8eb5500           -mov eax, dword ptr [ebx + 0x55ebb8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(5630904) /* 0x55ebb8 */);
    // 004a53fe  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a53ff  8d8598feffff           -lea eax, [ebp - 0x168]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-360) /* -0x168 */);
    // 004a5405  30c9                   -xor cl, cl
    cpu.cl ^= x86::reg8(x86::sreg8(cpu.cl));
    // 004a5407  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a5408  888c2a98feffff         -mov byte ptr [edx + ebp - 0x168], cl
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(-360) /* -0x168 */ + cpu.ebp * 1) = cpu.cl;
    // 004a540f  8b9334eb5500           -mov edx, dword ptr [ebx + 0x55eb34]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(5630772) /* 0x55eb34 */);
    // 004a5415  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004a5416  6828d75300             -push 0x53d728
    app->getMemory<x86::reg32>(cpu.esp-4) = 5494568 /*0x53d728*/;
    cpu.esp -= 4;
    // 004a541b  bfa01b7a00             -mov edi, 0x7a1ba0
    cpu.edi = 8002464 /*0x7a1ba0*/;
    // 004a5420  29c9                   -sub ecx, ecx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004a5422  49                     -dec ecx
    (cpu.ecx)--;
    // 004a5423  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004a5425  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 004a5427  f7d1                   -not ecx
    cpu.ecx = ~cpu.ecx;
    // 004a5429  49                     -dec ecx
    (cpu.ecx)--;
    // 004a542a  b8a01b7a00             -mov eax, 0x7a1ba0
    cpu.eax = 8002464 /*0x7a1ba0*/;
    // 004a542f  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004a5431  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a5432  e859a20300             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a5437  83c414                 -add esp, 0x14
    (cpu.esp) += x86::reg32(x86::sreg32(20 /*0x14*/));
L_0x004a543a:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a543a  46                     -inc esi
    (cpu.esi)++;
    // 004a543b  83fe21                 +cmp esi, 0x21
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(33 /*0x21*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a543e  7d12                   -jge 0x4a5452
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004a5452;
    }
L_0x004a5440:
    // 004a5440  8b04b534eb5500         -mov eax, dword ptr [esi*4 + 0x55eb34]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5630772) /* 0x55eb34 */ + cpu.esi * 4);
    // 004a5447  803800                 +cmp byte ptr [eax], 0
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.eax);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 004a544a  0f8579ffffff           -jne 0x4a53c9
    if (!cpu.flags.zf)
    {
        goto L_0x004a53c9;
    }
    // 004a5450  ebe8                   -jmp 0x4a543a
    goto L_0x004a543a;
L_0x004a5452:
    // 004a5452  8b4dfc                 -mov ecx, dword ptr [ebp - 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004a5455  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 004a5457  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004a5459  7507                   -jne 0x4a5462
    if (!cpu.flags.zf)
    {
        goto L_0x004a5462;
    }
    // 004a545b  b834d75300             -mov eax, 0x53d734
    cpu.eax = 5494580 /*0x53d734*/;
    // 004a5460  eb05                   -jmp 0x4a5467
    goto L_0x004a5467;
L_0x004a5462:
    // 004a5462  b848d75300             -mov eax, 0x53d748
    cpu.eax = 5494600 /*0x53d748*/;
L_0x004a5467:
    // 004a5467  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a5468  68a01b7a00             -push 0x7a1ba0
    app->getMemory<x86::reg32>(cpu.esp-4) = 8002464 /*0x7a1ba0*/;
    cpu.esp -= 4;
    // 004a546d  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 004a546f  2eff1564475300         -call dword ptr cs:[0x534764]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457764) /* 0x534764 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a5476  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004a5478  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004a547a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a547b  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a547c  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a547d  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a547e  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a547f  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a5480:
    // 004a5480  833ddc1f7a0000         +cmp dword ptr [0x7a1fdc], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8003548) /* 0x7a1fdc */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a5487  744e                   -je 0x4a54d7
    if (cpu.flags.zf)
    {
        goto L_0x004a54d7;
    }
    // 004a5489  bfa01f7a00             -mov edi, 0x7a1fa0
    cpu.edi = 8003488 /*0x7a1fa0*/;
    // 004a548e  29c9                   -sub ecx, ecx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004a5490  49                     -dec ecx
    (cpu.ecx)--;
    // 004a5491  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004a5493  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 004a5495  f7d1                   -not ecx
    cpu.ecx = ~cpu.ecx;
    // 004a5497  49                     -dec ecx
    (cpu.ecx)--;
    // 004a5498  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004a549a  741c                   -je 0x4a54b8
    if (cpu.flags.zf)
    {
        goto L_0x004a54b8;
    }
    // 004a549c  bfe01f7a00             -mov edi, 0x7a1fe0
    cpu.edi = 8003552 /*0x7a1fe0*/;
    // 004a54a1  29c9                   -sub ecx, ecx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004a54a3  49                     -dec ecx
    (cpu.ecx)--;
    // 004a54a4  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004a54a6  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 004a54a8  f7d1                   -not ecx
    cpu.ecx = ~cpu.ecx;
    // 004a54aa  49                     -dec ecx
    (cpu.ecx)--;
    // 004a54ab  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004a54ad  7409                   -je 0x4a54b8
    if (cpu.flags.zf)
    {
        goto L_0x004a54b8;
    }
    // 004a54af  833df81f7a0000         +cmp dword ptr [0x7a1ff8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8003576) /* 0x7a1ff8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a54b6  751f                   -jne 0x4a54d7
    if (!cpu.flags.zf)
    {
        goto L_0x004a54d7;
    }
L_0x004a54b8:
    // 004a54b8  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 004a54ba  6850d75300             -push 0x53d750
    app->getMemory<x86::reg32>(cpu.esp-4) = 5494608 /*0x53d750*/;
    cpu.esp -= 4;
    // 004a54bf  6864d75300             -push 0x53d764
    app->getMemory<x86::reg32>(cpu.esp-4) = 5494628 /*0x53d764*/;
    cpu.esp -= 4;
    // 004a54c4  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 004a54c6  2eff1564475300         -call dword ptr cs:[0x534764]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457764) /* 0x534764 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a54cd  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004a54cf  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004a54d1  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a54d2  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a54d3  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a54d4  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a54d5  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a54d6  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a54d7:
    // 004a54d7  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004a54dc  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004a54de  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a54df  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a54e0  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a54e1  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a54e2  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a54e3  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_4a54f0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004a54f0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a54f1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a54f2  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004a54f3  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004a54f4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a54f5  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a54f7  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 004a54f9  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a54fa  2eff158c455300         -call dword ptr cs:[0x53458c]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457292) /* 0x53458c */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a5501  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 004a5503  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 004a5505  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a5507  0f845a020000           -je 0x4a5767
    if (cpu.flags.zf)
    {
        goto L_0x004a5767;
    }
    // 004a550d  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 004a550f  0f855c020000           -jne 0x4a5771
    if (!cpu.flags.zf)
    {
        goto L_0x004a5771;
    }
    // 004a5515  6808d85300             -push 0x53d808
    app->getMemory<x86::reg32>(cpu.esp-4) = 5494792 /*0x53d808*/;
    cpu.esp -= 4;
    // 004a551a  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a551b  2eff1558455300         -call dword ptr cs:[0x534558]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457240) /* 0x534558 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a5522  681cd85300             -push 0x53d81c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5494812 /*0x53d81c*/;
    cpu.esp -= 4;
    // 004a5527  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a5528  a3e4217a00             -mov dword ptr [0x7a21e4], eax
    app->getMemory<x86::reg32>(x86::reg32(8004068) /* 0x7a21e4 */) = cpu.eax;
    // 004a552d  2eff1558455300         -call dword ptr cs:[0x534558]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457240) /* 0x534558 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a5534  6830d85300             -push 0x53d830
    app->getMemory<x86::reg32>(cpu.esp-4) = 5494832 /*0x53d830*/;
    cpu.esp -= 4;
    // 004a5539  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a553a  a3dc217a00             -mov dword ptr [0x7a21dc], eax
    app->getMemory<x86::reg32>(x86::reg32(8004060) /* 0x7a21dc */) = cpu.eax;
    // 004a553f  2eff1558455300         -call dword ptr cs:[0x534558]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457240) /* 0x534558 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a5546  6844d85300             -push 0x53d844
    app->getMemory<x86::reg32>(cpu.esp-4) = 5494852 /*0x53d844*/;
    cpu.esp -= 4;
    // 004a554b  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a554c  a3d0217a00             -mov dword ptr [0x7a21d0], eax
    app->getMemory<x86::reg32>(x86::reg32(8004048) /* 0x7a21d0 */) = cpu.eax;
    // 004a5551  2eff1558455300         -call dword ptr cs:[0x534558]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457240) /* 0x534558 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a5558  6858d85300             -push 0x53d858
    app->getMemory<x86::reg32>(cpu.esp-4) = 5494872 /*0x53d858*/;
    cpu.esp -= 4;
    // 004a555d  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a555e  a3e0217a00             -mov dword ptr [0x7a21e0], eax
    app->getMemory<x86::reg32>(x86::reg32(8004064) /* 0x7a21e0 */) = cpu.eax;
    // 004a5563  2eff1558455300         -call dword ptr cs:[0x534558]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457240) /* 0x534558 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a556a  6868d85300             -push 0x53d868
    app->getMemory<x86::reg32>(cpu.esp-4) = 5494888 /*0x53d868*/;
    cpu.esp -= 4;
    // 004a556f  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a5570  a3cc217a00             -mov dword ptr [0x7a21cc], eax
    app->getMemory<x86::reg32>(x86::reg32(8004044) /* 0x7a21cc */) = cpu.eax;
    // 004a5575  2eff1558455300         -call dword ptr cs:[0x534558]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457240) /* 0x534558 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a557c  6878d85300             -push 0x53d878
    app->getMemory<x86::reg32>(cpu.esp-4) = 5494904 /*0x53d878*/;
    cpu.esp -= 4;
    // 004a5581  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a5582  a3d8217a00             -mov dword ptr [0x7a21d8], eax
    app->getMemory<x86::reg32>(x86::reg32(8004056) /* 0x7a21d8 */) = cpu.eax;
    // 004a5587  2eff1558455300         -call dword ptr cs:[0x534558]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457240) /* 0x534558 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a558e  6888d85300             -push 0x53d888
    app->getMemory<x86::reg32>(cpu.esp-4) = 5494920 /*0x53d888*/;
    cpu.esp -= 4;
    // 004a5593  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a5594  a3c8217a00             -mov dword ptr [0x7a21c8], eax
    app->getMemory<x86::reg32>(x86::reg32(8004040) /* 0x7a21c8 */) = cpu.eax;
    // 004a5599  2eff1558455300         -call dword ptr cs:[0x534558]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457240) /* 0x534558 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a55a0  689cd85300             -push 0x53d89c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5494940 /*0x53d89c*/;
    cpu.esp -= 4;
    // 004a55a5  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a55a6  a3c0217a00             -mov dword ptr [0x7a21c0], eax
    app->getMemory<x86::reg32>(x86::reg32(8004032) /* 0x7a21c0 */) = cpu.eax;
    // 004a55ab  2eff1558455300         -call dword ptr cs:[0x534558]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457240) /* 0x534558 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a55b2  68b0d85300             -push 0x53d8b0
    app->getMemory<x86::reg32>(cpu.esp-4) = 5494960 /*0x53d8b0*/;
    cpu.esp -= 4;
    // 004a55b7  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a55b8  a37c217a00             -mov dword ptr [0x7a217c], eax
    app->getMemory<x86::reg32>(x86::reg32(8003964) /* 0x7a217c */) = cpu.eax;
    // 004a55bd  2eff1558455300         -call dword ptr cs:[0x534558]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457240) /* 0x534558 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a55c4  68c0d85300             -push 0x53d8c0
    app->getMemory<x86::reg32>(cpu.esp-4) = 5494976 /*0x53d8c0*/;
    cpu.esp -= 4;
    // 004a55c9  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a55ca  a384217a00             -mov dword ptr [0x7a2184], eax
    app->getMemory<x86::reg32>(x86::reg32(8003972) /* 0x7a2184 */) = cpu.eax;
    // 004a55cf  2eff1558455300         -call dword ptr cs:[0x534558]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457240) /* 0x534558 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a55d6  68d0d85300             -push 0x53d8d0
    app->getMemory<x86::reg32>(cpu.esp-4) = 5494992 /*0x53d8d0*/;
    cpu.esp -= 4;
    // 004a55db  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a55dc  a3b8217a00             -mov dword ptr [0x7a21b8], eax
    app->getMemory<x86::reg32>(x86::reg32(8004024) /* 0x7a21b8 */) = cpu.eax;
    // 004a55e1  2eff1558455300         -call dword ptr cs:[0x534558]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457240) /* 0x534558 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a55e8  68e0d85300             -push 0x53d8e0
    app->getMemory<x86::reg32>(cpu.esp-4) = 5495008 /*0x53d8e0*/;
    cpu.esp -= 4;
    // 004a55ed  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a55ee  a390217a00             -mov dword ptr [0x7a2190], eax
    app->getMemory<x86::reg32>(x86::reg32(8003984) /* 0x7a2190 */) = cpu.eax;
    // 004a55f3  2eff1558455300         -call dword ptr cs:[0x534558]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457240) /* 0x534558 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a55fa  a368217a00             -mov dword ptr [0x7a2168], eax
    app->getMemory<x86::reg32>(x86::reg32(8003944) /* 0x7a2168 */) = cpu.eax;
    // 004a55ff  68f0d85300             -push 0x53d8f0
    app->getMemory<x86::reg32>(cpu.esp-4) = 5495024 /*0x53d8f0*/;
    cpu.esp -= 4;
    // 004a5604  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a5605  2eff1558455300         -call dword ptr cs:[0x534558]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457240) /* 0x534558 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a560c  6808d95300             -push 0x53d908
    app->getMemory<x86::reg32>(cpu.esp-4) = 5495048 /*0x53d908*/;
    cpu.esp -= 4;
    // 004a5611  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a5612  a3a4217a00             -mov dword ptr [0x7a21a4], eax
    app->getMemory<x86::reg32>(x86::reg32(8004004) /* 0x7a21a4 */) = cpu.eax;
    // 004a5617  2eff1558455300         -call dword ptr cs:[0x534558]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457240) /* 0x534558 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a561e  681cd95300             -push 0x53d91c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5495068 /*0x53d91c*/;
    cpu.esp -= 4;
    // 004a5623  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a5624  a398217a00             -mov dword ptr [0x7a2198], eax
    app->getMemory<x86::reg32>(x86::reg32(8003992) /* 0x7a2198 */) = cpu.eax;
    // 004a5629  2eff1558455300         -call dword ptr cs:[0x534558]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457240) /* 0x534558 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a5630  6838d95300             -push 0x53d938
    app->getMemory<x86::reg32>(cpu.esp-4) = 5495096 /*0x53d938*/;
    cpu.esp -= 4;
    // 004a5635  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a5636  a364217a00             -mov dword ptr [0x7a2164], eax
    app->getMemory<x86::reg32>(x86::reg32(8003940) /* 0x7a2164 */) = cpu.eax;
    // 004a563b  2eff1558455300         -call dword ptr cs:[0x534558]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457240) /* 0x534558 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a5642  684cd95300             -push 0x53d94c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5495116 /*0x53d94c*/;
    cpu.esp -= 4;
    // 004a5647  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a5648  a374217a00             -mov dword ptr [0x7a2174], eax
    app->getMemory<x86::reg32>(x86::reg32(8003956) /* 0x7a2174 */) = cpu.eax;
    // 004a564d  2eff1558455300         -call dword ptr cs:[0x534558]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457240) /* 0x534558 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a5654  6860d95300             -push 0x53d960
    app->getMemory<x86::reg32>(cpu.esp-4) = 5495136 /*0x53d960*/;
    cpu.esp -= 4;
    // 004a5659  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a565a  a3d4217a00             -mov dword ptr [0x7a21d4], eax
    app->getMemory<x86::reg32>(x86::reg32(8004052) /* 0x7a21d4 */) = cpu.eax;
    // 004a565f  2eff1558455300         -call dword ptr cs:[0x534558]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457240) /* 0x534558 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a5666  6874d95300             -push 0x53d974
    app->getMemory<x86::reg32>(cpu.esp-4) = 5495156 /*0x53d974*/;
    cpu.esp -= 4;
    // 004a566b  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a566c  a388217a00             -mov dword ptr [0x7a2188], eax
    app->getMemory<x86::reg32>(x86::reg32(8003976) /* 0x7a2188 */) = cpu.eax;
    // 004a5671  2eff1558455300         -call dword ptr cs:[0x534558]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457240) /* 0x534558 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a5678  6884d95300             -push 0x53d984
    app->getMemory<x86::reg32>(cpu.esp-4) = 5495172 /*0x53d984*/;
    cpu.esp -= 4;
    // 004a567d  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a567e  a3b4217a00             -mov dword ptr [0x7a21b4], eax
    app->getMemory<x86::reg32>(x86::reg32(8004020) /* 0x7a21b4 */) = cpu.eax;
    // 004a5683  2eff1558455300         -call dword ptr cs:[0x534558]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457240) /* 0x534558 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a568a  6898d95300             -push 0x53d998
    app->getMemory<x86::reg32>(cpu.esp-4) = 5495192 /*0x53d998*/;
    cpu.esp -= 4;
    // 004a568f  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a5690  a39c217a00             -mov dword ptr [0x7a219c], eax
    app->getMemory<x86::reg32>(x86::reg32(8003996) /* 0x7a219c */) = cpu.eax;
    // 004a5695  2eff1558455300         -call dword ptr cs:[0x534558]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457240) /* 0x534558 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a569c  68a8d95300             -push 0x53d9a8
    app->getMemory<x86::reg32>(cpu.esp-4) = 5495208 /*0x53d9a8*/;
    cpu.esp -= 4;
    // 004a56a1  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a56a2  a394217a00             -mov dword ptr [0x7a2194], eax
    app->getMemory<x86::reg32>(x86::reg32(8003988) /* 0x7a2194 */) = cpu.eax;
    // 004a56a7  2eff1558455300         -call dword ptr cs:[0x534558]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457240) /* 0x534558 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a56ae  68c0d95300             -push 0x53d9c0
    app->getMemory<x86::reg32>(cpu.esp-4) = 5495232 /*0x53d9c0*/;
    cpu.esp -= 4;
    // 004a56b3  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a56b4  a3ac217a00             -mov dword ptr [0x7a21ac], eax
    app->getMemory<x86::reg32>(x86::reg32(8004012) /* 0x7a21ac */) = cpu.eax;
    // 004a56b9  2eff1558455300         -call dword ptr cs:[0x534558]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457240) /* 0x534558 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a56c0  68d4d95300             -push 0x53d9d4
    app->getMemory<x86::reg32>(cpu.esp-4) = 5495252 /*0x53d9d4*/;
    cpu.esp -= 4;
    // 004a56c5  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a56c6  a3a8217a00             -mov dword ptr [0x7a21a8], eax
    app->getMemory<x86::reg32>(x86::reg32(8004008) /* 0x7a21a8 */) = cpu.eax;
    // 004a56cb  2eff1558455300         -call dword ptr cs:[0x534558]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457240) /* 0x534558 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a56d2  68e8d95300             -push 0x53d9e8
    app->getMemory<x86::reg32>(cpu.esp-4) = 5495272 /*0x53d9e8*/;
    cpu.esp -= 4;
    // 004a56d7  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a56d8  a38c217a00             -mov dword ptr [0x7a218c], eax
    app->getMemory<x86::reg32>(x86::reg32(8003980) /* 0x7a218c */) = cpu.eax;
    // 004a56dd  2eff1558455300         -call dword ptr cs:[0x534558]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457240) /* 0x534558 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a56e4  a3c4217a00             -mov dword ptr [0x7a21c4], eax
    app->getMemory<x86::reg32>(x86::reg32(8004036) /* 0x7a21c4 */) = cpu.eax;
    // 004a56e9  68fcd95300             -push 0x53d9fc
    app->getMemory<x86::reg32>(cpu.esp-4) = 5495292 /*0x53d9fc*/;
    cpu.esp -= 4;
    // 004a56ee  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a56ef  2eff1558455300         -call dword ptr cs:[0x534558]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457240) /* 0x534558 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a56f6  6814da5300             -push 0x53da14
    app->getMemory<x86::reg32>(cpu.esp-4) = 5495316 /*0x53da14*/;
    cpu.esp -= 4;
    // 004a56fb  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a56fc  a3b0217a00             -mov dword ptr [0x7a21b0], eax
    app->getMemory<x86::reg32>(x86::reg32(8004016) /* 0x7a21b0 */) = cpu.eax;
    // 004a5701  2eff1558455300         -call dword ptr cs:[0x534558]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457240) /* 0x534558 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a5708  682cda5300             -push 0x53da2c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5495340 /*0x53da2c*/;
    cpu.esp -= 4;
    // 004a570d  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a570e  a370217a00             -mov dword ptr [0x7a2170], eax
    app->getMemory<x86::reg32>(x86::reg32(8003952) /* 0x7a2170 */) = cpu.eax;
    // 004a5713  2eff1558455300         -call dword ptr cs:[0x534558]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457240) /* 0x534558 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a571a  6844da5300             -push 0x53da44
    app->getMemory<x86::reg32>(cpu.esp-4) = 5495364 /*0x53da44*/;
    cpu.esp -= 4;
    // 004a571f  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a5720  a3bc217a00             -mov dword ptr [0x7a21bc], eax
    app->getMemory<x86::reg32>(x86::reg32(8004028) /* 0x7a21bc */) = cpu.eax;
    // 004a5725  2eff1558455300         -call dword ptr cs:[0x534558]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457240) /* 0x534558 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a572c  6858da5300             -push 0x53da58
    app->getMemory<x86::reg32>(cpu.esp-4) = 5495384 /*0x53da58*/;
    cpu.esp -= 4;
    // 004a5731  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a5732  a3a0217a00             -mov dword ptr [0x7a21a0], eax
    app->getMemory<x86::reg32>(x86::reg32(8004000) /* 0x7a21a0 */) = cpu.eax;
    // 004a5737  2eff1558455300         -call dword ptr cs:[0x534558]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457240) /* 0x534558 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a573e  6870da5300             -push 0x53da70
    app->getMemory<x86::reg32>(cpu.esp-4) = 5495408 /*0x53da70*/;
    cpu.esp -= 4;
    // 004a5743  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a5744  a36c217a00             -mov dword ptr [0x7a216c], eax
    app->getMemory<x86::reg32>(x86::reg32(8003948) /* 0x7a216c */) = cpu.eax;
    // 004a5749  2eff1558455300         -call dword ptr cs:[0x534558]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457240) /* 0x534558 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a5750  6880da5300             -push 0x53da80
    app->getMemory<x86::reg32>(cpu.esp-4) = 5495424 /*0x53da80*/;
    cpu.esp -= 4;
    // 004a5755  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a5756  a378217a00             -mov dword ptr [0x7a2178], eax
    app->getMemory<x86::reg32>(x86::reg32(8003960) /* 0x7a2178 */) = cpu.eax;
    // 004a575b  2eff1558455300         -call dword ptr cs:[0x534558]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457240) /* 0x534558 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a5762  a380217a00             -mov dword ptr [0x7a2180], eax
    app->getMemory<x86::reg32>(x86::reg32(8003968) /* 0x7a2180 */) = cpu.eax;
L_0x004a5767:
    // 004a5767  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 004a5769  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 004a576b  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a576c  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a576d  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a576e  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a576f  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5770  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a5771:
    // 004a5771  bbffffffff             -mov ebx, 0xffffffff
    cpu.ebx = 4294967295 /*0xffffffff*/;
    // 004a5776  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 004a5778  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5779  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a577a  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a577b  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a577c  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a577d  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_4a5780(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004a5780  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a5781  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a5782  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004a5783  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a5784  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a5786  83ec48                 -sub esp, 0x48
    (cpu.esp) -= x86::reg32(x86::sreg32(72 /*0x48*/));
    // 004a5789  8d45f8                 -lea eax, [ebp - 8]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004a578c  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a578d  683f000f00             -push 0xf003f
    app->getMemory<x86::reg32>(cpu.esp-4) = 983103 /*0xf003f*/;
    cpu.esp -= 4;
    // 004a5792  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 004a5794  6894da5300             -push 0x53da94
    app->getMemory<x86::reg32>(cpu.esp-4) = 5495444 /*0x53da94*/;
    cpu.esp -= 4;
    // 004a5799  6802000080             -push 0x80000002
    app->getMemory<x86::reg32>(cpu.esp-4) = 2147483650 /*0x80000002*/;
    cpu.esp -= 4;
    // 004a579e  2eff156c445300         -call dword ptr cs:[0x53446c]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457004) /* 0x53446c */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a57a5  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a57a7  0f85c6000000           -jne 0x4a5873
    if (!cpu.flags.zf)
    {
        goto L_0x004a5873;
    }
    // 004a57ad  8d45fc                 -lea eax, [ebp - 4]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004a57b0  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a57b1  8d45b8                 -lea eax, [ebp - 0x48]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-72) /* -0x48 */);
    // 004a57b4  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a57b5  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 004a57b7  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 004a57b9  68a8da5300             -push 0x53daa8
    app->getMemory<x86::reg32>(cpu.esp-4) = 5495464 /*0x53daa8*/;
    cpu.esp -= 4;
    // 004a57be  8b4df8                 -mov ecx, dword ptr [ebp - 8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004a57c1  ba40000000             -mov edx, 0x40
    cpu.edx = 64 /*0x40*/;
    // 004a57c6  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a57c7  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 004a57ca  2eff1570445300         -call dword ptr cs:[0x534470]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457008) /* 0x534470 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a57d1  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a57d3  0f859a000000           -jne 0x4a5873
    if (!cpu.flags.zf)
    {
        goto L_0x004a5873;
    }
    // 004a57d9  8b5df8                 -mov ebx, dword ptr [ebp - 8]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004a57dc  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a57dd  2eff1564445300         -call dword ptr cs:[0x534464]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5456996) /* 0x534464 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a57e4  bab8da5300             -mov edx, 0x53dab8
    cpu.edx = 5495480 /*0x53dab8*/;
    // 004a57e9  8d45b8                 -lea eax, [ebp - 0x48]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-72) /* -0x48 */);
    // 004a57ec  e81f8b0400             -call 0x4ee310
    cpu.esp -= 4;
    nfs3hp::watcom::strcmp(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a57f1  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a57f3  750a                   -jne 0x4a57ff
    if (!cpu.flags.zf)
    {
        goto L_0x004a57ff;
    }
    // 004a57f5  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004a57fa  e976000000             -jmp 0x4a5875
    goto L_0x004a5875;
L_0x004a57ff:
    // 004a57ff  babcda5300             -mov edx, 0x53dabc
    cpu.edx = 5495484 /*0x53dabc*/;
    // 004a5804  8d45b8                 -lea eax, [ebp - 0x48]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-72) /* -0x48 */);
    // 004a5807  e8048b0400             -call 0x4ee310
    cpu.esp -= 4;
    nfs3hp::watcom::strcmp(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a580c  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a580e  750c                   -jne 0x4a581c
    if (!cpu.flags.zf)
    {
        goto L_0x004a581c;
    }
    // 004a5810  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004a5815  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004a5817  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5818  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5819  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a581a  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a581b  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a581c:
    // 004a581c  bac0da5300             -mov edx, 0x53dac0
    cpu.edx = 5495488 /*0x53dac0*/;
    // 004a5821  8d45b8                 -lea eax, [ebp - 0x48]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-72) /* -0x48 */);
    // 004a5824  e8e78a0400             -call 0x4ee310
    cpu.esp -= 4;
    nfs3hp::watcom::strcmp(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a5829  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a582b  750c                   -jne 0x4a5839
    if (!cpu.flags.zf)
    {
        goto L_0x004a5839;
    }
    // 004a582d  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004a5832  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004a5834  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5835  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5836  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5837  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5838  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a5839:
    // 004a5839  bac4da5300             -mov edx, 0x53dac4
    cpu.edx = 5495492 /*0x53dac4*/;
    // 004a583e  8d45b8                 -lea eax, [ebp - 0x48]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-72) /* -0x48 */);
    // 004a5841  e8ca8a0400             -call 0x4ee310
    cpu.esp -= 4;
    nfs3hp::watcom::strcmp(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a5846  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a5848  750c                   -jne 0x4a5856
    if (!cpu.flags.zf)
    {
        goto L_0x004a5856;
    }
    // 004a584a  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004a584f  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004a5851  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5852  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5853  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5854  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5855  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a5856:
    // 004a5856  bac8da5300             -mov edx, 0x53dac8
    cpu.edx = 5495496 /*0x53dac8*/;
    // 004a585b  8d45b8                 -lea eax, [ebp - 0x48]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-72) /* -0x48 */);
    // 004a585e  e8ad8a0400             -call 0x4ee310
    cpu.esp -= 4;
    nfs3hp::watcom::strcmp(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a5863  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a5865  750c                   -jne 0x4a5873
    if (!cpu.flags.zf)
    {
        goto L_0x004a5873;
    }
    // 004a5867  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004a586c  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004a586e  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a586f  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5870  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5871  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5872  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a5873:
    // 004a5873  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x004a5875:
    // 004a5875  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004a5877  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5878  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5879  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a587a  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a587b  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_4a5880(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004a5880  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a5881  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a5882  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004a5883  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004a5884  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004a5885  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a5886  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a5888  81ec08010000           -sub esp, 0x108
    (cpu.esp) -= x86::reg32(x86::sreg32(264 /*0x108*/));
    // 004a588e  81ed82000000           -sub ebp, 0x82
    (cpu.ebp) -= x86::reg32(x86::sreg32(130 /*0x82*/));
    // 004a5894  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004a5896  8d457a                 -lea eax, [ebp + 0x7a]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 004a5899  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a589a  683f000f00             -push 0xf003f
    app->getMemory<x86::reg32>(cpu.esp-4) = 983103 /*0xf003f*/;
    cpu.esp -= 4;
    // 004a589f  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 004a58a1  6894da5300             -push 0x53da94
    app->getMemory<x86::reg32>(cpu.esp-4) = 5495444 /*0x53da94*/;
    cpu.esp -= 4;
    // 004a58a6  6802000080             -push 0x80000002
    app->getMemory<x86::reg32>(cpu.esp-4) = 2147483650 /*0x80000002*/;
    cpu.esp -= 4;
    // 004a58ab  2eff156c445300         -call dword ptr cs:[0x53446c]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457004) /* 0x53446c */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a58b2  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a58b4  0f85f1000000           -jne 0x4a59ab
    if (!cpu.flags.zf)
    {
        goto L_0x004a59ab;
    }
    // 004a58ba  ba80000000             -mov edx, 0x80
    cpu.edx = 128 /*0x80*/;
    // 004a58bf  8dbd7affffff           -lea edi, [ebp - 0x86]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-134) /* -0x86 */);
    // 004a58c5  8b34b5a0ec5500         -mov esi, dword ptr [esi*4 + 0x55eca0]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(5631136) /* 0x55eca0 */ + cpu.esi * 4);
    // 004a58cc  89557e                 -mov dword ptr [ebp + 0x7e], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.edx;
    // 004a58cf  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
L_0x004a58d0:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a58d0  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 004a58d2  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 004a58d4  3c00                   +cmp al, 0
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
    // 004a58d6  7410                   -je 0x4a58e8
    if (cpu.flags.zf)
    {
        goto L_0x004a58e8;
    }
    // 004a58d8  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 004a58db  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004a58de  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 004a58e1  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004a58e4  3c00                   +cmp al, 0
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
    // 004a58e6  75e8                   -jne 0x4a58d0
    if (!cpu.flags.zf)
    {
        goto L_0x004a58d0;
    }
L_0x004a58e8:
    // 004a58e8  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a58e9  e892feffff             -call 0x4a5780
    cpu.esp -= 4;
    sub_4a5780(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a58ee  83f801                 +cmp eax, 1
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
    // 004a58f1  7550                   -jne 0x4a5943
    if (!cpu.flags.zf)
    {
        goto L_0x004a5943;
    }
    // 004a58f3  beccda5300             -mov esi, 0x53dacc
    cpu.esi = 5495500 /*0x53dacc*/;
    // 004a58f8  8dbd7affffff           -lea edi, [ebp - 0x86]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-134) /* -0x86 */);
    // 004a58fe  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004a58ff  2bc9                   +sub ecx, ecx
    {
        x86::reg32& tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ecx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004a5901  49                     +dec ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 004a5902  b000                   -mov al, 0
    cpu.al = 0 /*0x0*/;
    // 004a5904  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 004a5906  4f                     -dec edi
    (cpu.edi)--;
L_0x004a5907:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a5907  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 004a5909  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 004a590b  3c00                   +cmp al, 0
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
    // 004a590d  7410                   -je 0x4a591f
    if (cpu.flags.zf)
    {
        goto L_0x004a591f;
    }
    // 004a590f  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 004a5912  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004a5915  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 004a5918  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004a591b  3c00                   +cmp al, 0
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
    // 004a591d  75e8                   -jne 0x4a5907
    if (!cpu.flags.zf)
    {
        goto L_0x004a5907;
    }
L_0x004a591f:
    // 004a591f  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5920  8d857affffff           -lea eax, [ebp - 0x86]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-134) /* -0x86 */);
    // 004a5926  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a5927  68d8da5300             -push 0x53dad8
    app->getMemory<x86::reg32>(cpu.esp-4) = 5495512 /*0x53dad8*/;
    cpu.esp -= 4;
    // 004a592c  8d45fa                 -lea eax, [ebp - 6]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-6) /* -0x6 */);
    // 004a592f  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a5930  e85b9d0300             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a5935  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 004a5938  8d45fa                 -lea eax, [ebp - 6]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-6) /* -0x6 */);
    // 004a593b  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a593c  2eff159c455300         -call dword ptr cs:[0x53459c]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457308) /* 0x53459c */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004a5943:
    // 004a5943  8d457e                 -lea eax, [ebp + 0x7e]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 004a5946  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a5947  6820207a00             -push 0x7a2020
    app->getMemory<x86::reg32>(cpu.esp-4) = 8003616 /*0x7a2020*/;
    cpu.esp -= 4;
    // 004a594c  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 004a594e  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 004a5950  8d857affffff           -lea eax, [ebp - 0x86]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-134) /* -0x86 */);
    // 004a5956  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a5957  8b4d7a                 -mov ecx, dword ptr [ebp + 0x7a]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 004a595a  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a595b  2eff1570445300         -call dword ptr cs:[0x534470]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457008) /* 0x534470 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a5962  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a5964  753a                   -jne 0x4a59a0
    if (!cpu.flags.zf)
    {
        goto L_0x004a59a0;
    }
L_0x004a5966:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a5966  b22f                   -mov dl, 0x2f
    cpu.dl = 47 /*0x2f*/;
    // 004a5968  be20207a00             -mov esi, 0x7a2020
    cpu.esi = 8003616 /*0x7a2020*/;
L_0x004a596d:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a596d  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 004a596f  3ac2                   +cmp al, dl
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(cpu.dl));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 004a5971  7412                   -je 0x4a5985
    if (cpu.flags.zf)
    {
        goto L_0x004a5985;
    }
    // 004a5973  3c00                   +cmp al, 0
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
    // 004a5975  740c                   -je 0x4a5983
    if (cpu.flags.zf)
    {
        goto L_0x004a5983;
    }
    // 004a5977  46                     -inc esi
    (cpu.esi)++;
    // 004a5978  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 004a597a  3ac2                   +cmp al, dl
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(cpu.dl));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 004a597c  7407                   -je 0x4a5985
    if (cpu.flags.zf)
    {
        goto L_0x004a5985;
    }
    // 004a597e  46                     -inc esi
    (cpu.esi)++;
    // 004a597f  3c00                   +cmp al, 0
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
    // 004a5981  75ea                   -jne 0x4a596d
    if (!cpu.flags.zf)
    {
        goto L_0x004a596d;
    }
L_0x004a5983:
    // 004a5983  2bf6                   -sub esi, esi
    (cpu.esi) -= x86::reg32(x86::sreg32(cpu.esi));
L_0x004a5985:
    // 004a5985  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 004a5987  7405                   -je 0x4a598e
    if (cpu.flags.zf)
    {
        goto L_0x004a598e;
    }
    // 004a5989  c6065c                 -mov byte ptr [esi], 0x5c
    app->getMemory<x86::reg8>(cpu.esi) = 92 /*0x5c*/;
    // 004a598c  ebd8                   -jmp 0x4a5966
    goto L_0x004a5966;
L_0x004a598e:
    // 004a598e  8b5d7a                 -mov ebx, dword ptr [ebp + 0x7a]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 004a5991  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a5992  2eff1564445300         -call dword ptr cs:[0x534464]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5456996) /* 0x534464 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a5999  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004a599e  eb0d                   -jmp 0x4a59ad
    goto L_0x004a59ad;
L_0x004a59a0:
    // 004a59a0  8b757a                 -mov esi, dword ptr [ebp + 0x7a]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 004a59a3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004a59a4  2eff1564445300         -call dword ptr cs:[0x534464]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5456996) /* 0x534464 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004a59ab:
    // 004a59ab  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x004a59ad:
    // 004a59ad  8da582000000           -lea esp, [ebp + 0x82]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(130) /* 0x82 */);
    // 004a59b3  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a59b4  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a59b5  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a59b6  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a59b7  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a59b8  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a59b9  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_4a59c0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004a59c0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a59c1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a59c2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004a59c3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004a59c4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004a59c5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a59c6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a59c8  81ec00010000           -sub esp, 0x100
    (cpu.esp) -= x86::reg32(x86::sreg32(256 /*0x100*/));
    // 004a59ce  81edfe000000           -sub ebp, 0xfe
    (cpu.ebp) -= x86::reg32(x86::sreg32(254 /*0xfe*/));
    // 004a59d4  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 004a59d6  8d348500000000         -lea esi, [eax*4]
    cpu.esi = x86::reg32(cpu.eax * 4);
    // 004a59dd  8b9680ec5500           -mov edx, dword ptr [esi + 0x55ec80]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(5631104) /* 0x55ec80 */);
    // 004a59e3  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004a59e5  0f859f000000           -jne 0x4a5a8a
    if (!cpu.flags.zf)
    {
        goto L_0x004a5a8a;
    }
    // 004a59eb  e890feffff             -call 0x4a5880
    cpu.esp -= 4;
    sub_4a5880(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a59f0  83f801                 +cmp eax, 1
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
    // 004a59f3  756a                   -jne 0x4a5a5f
    if (!cpu.flags.zf)
    {
        goto L_0x004a5a5f;
    }
    // 004a59f5  b820207a00             -mov eax, 0x7a2020
    cpu.eax = 8003616 /*0x7a2020*/;
    // 004a59fa  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 004a59fc  e8effaffff             -call 0x4a54f0
    cpu.esp -= 4;
    sub_4a54f0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a5a01  898690ec5500           -mov dword ptr [esi + 0x55ec90], eax
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(5631120) /* 0x55ec90 */) = cpu.eax;
    // 004a5a07  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a5a09  740d                   -je 0x4a5a18
    if (cpu.flags.zf)
    {
        goto L_0x004a5a18;
    }
    // 004a5a0b  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004a5a10  018680ec5500           +add dword ptr [esi + 0x55ec80], eax
    {
        auto tmp1 = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(5631104) /* 0x55ec80 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004a5a16  eb74                   -jmp 0x4a5a8c
    goto L_0x004a5a8c;
L_0x004a5a18:
    // 004a5a18  2eff1534455300         -call dword ptr cs:[0x534534]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457204) /* 0x534534 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a5a1f  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a5a20  6820207a00             -push 0x7a2020
    app->getMemory<x86::reg32>(cpu.esp-4) = 8003616 /*0x7a2020*/;
    cpu.esp -= 4;
    // 004a5a25  8bbea0ec5500           -mov edi, dword ptr [esi + 0x55eca0]
    cpu.edi = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(5631136) /* 0x55eca0 */);
    // 004a5a2b  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004a5a2c  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a5a2d  68f0da5300             -push 0x53daf0
    app->getMemory<x86::reg32>(cpu.esp-4) = 5495536 /*0x53daf0*/;
    cpu.esp -= 4;
    // 004a5a32  8d457e                 -lea eax, [ebp + 0x7e]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 004a5a35  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a5a36  e8559c0300             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a5a3b  83c418                 +add esp, 0x18
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(24 /*0x18*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004a5a3e  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 004a5a40  6830db5300             -push 0x53db30
    app->getMemory<x86::reg32>(cpu.esp-4) = 5495600 /*0x53db30*/;
    cpu.esp -= 4;
    // 004a5a45  8d457e                 -lea eax, [ebp + 0x7e]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 004a5a48  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a5a49  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 004a5a4b  2eff1564475300         -call dword ptr cs:[0x534764]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457764) /* 0x534764 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a5a52  8d457e                 -lea eax, [ebp + 0x7e]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 004a5a55  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a5a56  2eff159c455300         -call dword ptr cs:[0x53459c]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457308) /* 0x53459c */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a5a5d  eb2b                   -jmp 0x4a5a8a
    goto L_0x004a5a8a;
L_0x004a5a5f:
    // 004a5a5f  8b86a0ec5500           -mov eax, dword ptr [esi + 0x55eca0]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(5631136) /* 0x55eca0 */);
    // 004a5a65  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a5a66  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a5a67  6844db5300             -push 0x53db44
    app->getMemory<x86::reg32>(cpu.esp-4) = 5495620 /*0x53db44*/;
    cpu.esp -= 4;
    // 004a5a6c  8d45fe                 -lea eax, [ebp - 2]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-2) /* -0x2 */);
    // 004a5a6f  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a5a70  e81b9c0300             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a5a75  83c410                 -add esp, 0x10
    (cpu.esp) += x86::reg32(x86::sreg32(16 /*0x10*/));
    // 004a5a78  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004a5a79  6890db5300             -push 0x53db90
    app->getMemory<x86::reg32>(cpu.esp-4) = 5495696 /*0x53db90*/;
    cpu.esp -= 4;
    // 004a5a7e  8d45fe                 -lea eax, [ebp - 2]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-2) /* -0x2 */);
    // 004a5a81  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a5a82  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004a5a83  2eff1564475300         -call dword ptr cs:[0x534764]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457764) /* 0x534764 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004a5a8a:
    // 004a5a8a  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x004a5a8c:
    // 004a5a8c  8da5fe000000           -lea esp, [ebp + 0xfe]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(254) /* 0xfe */);
    // 004a5a92  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5a93  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5a94  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5a95  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5a96  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5a97  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5a98  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4a5aa0: native, nfs3hp::dead::stub_0x4a5aa0 */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_4a5af0(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004a5af0  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a5af1  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a5af3  a0a0207a00             -mov al, byte ptr [0x7a20a0]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(8003744) /* 0x7a20a0 */);
    // 004a5af8  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5af9  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_4a5b00(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004a5b00  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004a5b01  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004a5b02  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a5b03  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a5b05  81ecc0070000           -sub esp, 0x7c0
    (cpu.esp) -= x86::reg32(x86::sreg32(1984 /*0x7c0*/));
    // 004a5b0b  81ed02080000           -sub ebp, 0x802
    (cpu.ebp) -= x86::reg32(x86::sreg32(2050 /*0x802*/));
    // 004a5b11  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004a5b13  8995fe070000           -mov dword ptr [ebp + 0x7fe], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(2046) /* 0x7fe */) = cpu.edx;
    // 004a5b19  898dfa070000           -mov dword ptr [ebp + 0x7fa], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(2042) /* 0x7fa */) = cpu.ecx;
    // 004a5b1f  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004a5b21  e89afeffff             -call 0x4a59c0
    cpu.esp -= 4;
    sub_4a59c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a5b26  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a5b28  0f84d8010000           -je 0x4a5d06
    if (cpu.flags.zf)
    {
        goto L_0x004a5d06;
    }
    // 004a5b2e  babc000000             -mov edx, 0xbc
    cpu.edx = 188 /*0xbc*/;
    // 004a5b33  b8a0207a00             -mov eax, 0x7a20a0
    cpu.eax = 8003744 /*0x7a20a0*/;
    // 004a5b38  e8cfab0300             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a5b3d  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 004a5b3f  7505                   -jne 0x4a5b46
    if (!cpu.flags.zf)
    {
        goto L_0x004a5b46;
    }
    // 004a5b41  bea4db5300             -mov esi, 0x53dba4
    cpu.esi = 5495716 /*0x53dba4*/;
L_0x004a5b46:
    // 004a5b46  bfa4207a00             -mov edi, 0x7a20a4
    cpu.edi = 8003748 /*0x7a20a4*/;
    // 004a5b4b  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
L_0x004a5b4c:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a5b4c  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 004a5b4e  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 004a5b50  3c00                   +cmp al, 0
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
    // 004a5b52  7410                   -je 0x4a5b64
    if (cpu.flags.zf)
    {
        goto L_0x004a5b64;
    }
    // 004a5b54  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 004a5b57  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004a5b5a  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 004a5b5d  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004a5b60  3c00                   +cmp al, 0
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
    // 004a5b62  75e8                   -jne 0x4a5b4c
    if (!cpu.flags.zf)
    {
        goto L_0x004a5b4c;
    }
L_0x004a5b64:
    // 004a5b64  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5b65  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004a5b67  668b85fe070000         -mov ax, word ptr [ebp + 0x7fe]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(2046) /* 0x7fe */);
    // 004a5b6e  a3a0207a00             -mov dword ptr [0x7a20a0], eax
    app->getMemory<x86::reg32>(x86::reg32(8003744) /* 0x7a20a0 */) = cpu.eax;
    // 004a5b73  663d0100               +cmp ax, 1
    {
        x86::reg16 tmp1 = cpu.ax;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(1 /*0x1*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004a5b77  760a                   -jbe 0x4a5b83
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004a5b83;
    }
    // 004a5b79  30e4                   +xor ah, ah
    cpu.clear_co();
    cpu.set_szp((cpu.ah ^= x86::reg8(x86::sreg8(cpu.ah))));
    // 004a5b7b  882504217a00           -mov byte ptr [0x7a2104], ah
    app->getMemory<x86::reg8>(x86::reg32(8003844) /* 0x7a2104 */) = cpu.ah;
    // 004a5b81  eb07                   -jmp 0x4a5b8a
    goto L_0x004a5b8a;
L_0x004a5b83:
    // 004a5b83  c60504217a0001         -mov byte ptr [0x7a2104], 1
    app->getMemory<x86::reg8>(x86::reg32(8003844) /* 0x7a2104 */) = 1 /*0x1*/;
L_0x004a5b8a:
    // 004a5b8a  bea4207a00             -mov esi, 0x7a20a4
    cpu.esi = 8003748 /*0x7a20a4*/;
    // 004a5b8f  8dbd82030000           -lea edi, [ebp + 0x382]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(898) /* 0x382 */);
    // 004a5b95  b940000000             -mov ecx, 0x40
    cpu.ecx = 64 /*0x40*/;
    // 004a5b9a  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
L_0x004a5b9b:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a5b9b  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 004a5b9d  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 004a5b9f  3c00                   +cmp al, 0
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
    // 004a5ba1  7410                   -je 0x4a5bb3
    if (cpu.flags.zf)
    {
        goto L_0x004a5bb3;
    }
    // 004a5ba3  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 004a5ba6  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004a5ba9  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 004a5bac  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004a5baf  3c00                   +cmp al, 0
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
    // 004a5bb1  75e8                   -jne 0x4a5b9b
    if (!cpu.flags.zf)
    {
        goto L_0x004a5b9b;
    }
L_0x004a5bb3:
    // 004a5bb3  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5bb4  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004a5bb6  be04217a00             -mov esi, 0x7a2104
    cpu.esi = 8003844 /*0x7a2104*/;
    // 004a5bbb  a1a0207a00             -mov eax, dword ptr [0x7a20a0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8003744) /* 0x7a20a0 */);
    // 004a5bc0  8dbda6030000           -lea edi, [ebp + 0x3a6]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(934) /* 0x3a6 */);
    // 004a5bc6  8985a2030000           -mov dword ptr [ebp + 0x3a2], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(930) /* 0x3a2 */) = cpu.eax;
    // 004a5bcc  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004a5bcd  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004a5bcf  c1e902                 -shr ecx, 2
    cpu.ecx >>= 2 /*0x2*/ % 32;
    // 004a5bd2  f2a5                   -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004a5bd4  8ac8                   -mov cl, al
    cpu.cl = cpu.al;
    // 004a5bd6  80e103                 +and cl, 3
    cpu.clear_co();
    cpu.set_szp((cpu.cl &= x86::reg8(x86::sreg8(3 /*0x3*/))));
    // 004a5bd9  f2a4                   -repne movsb byte ptr es:[edi], byte ptr [esi]
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
    // 004a5bdb  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5bdc  6a0a                   -push 0xa
    app->getMemory<x86::reg32>(cpu.esp-4) = 10 /*0xa*/;
    cpu.esp -= 4;
    // 004a5bde  8d85e6050000           -lea eax, [ebp + 0x5e6]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(1510) /* 0x5e6 */);
    // 004a5be4  8995e6050000           -mov dword ptr [ebp + 0x5e6], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(1510) /* 0x5e6 */) = cpu.edx;
    // 004a5bea  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a5beb  8d8582030000           -lea eax, [ebp + 0x382]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(898) /* 0x382 */);
    // 004a5bf1  8995ee050000           -mov dword ptr [ebp + 0x5ee], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(1518) /* 0x5ee */) = cpu.edx;
    // 004a5bf7  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a5bf8  8995f6050000           -mov dword ptr [ebp + 0x5f6], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(1526) /* 0x5f6 */) = cpu.edx;
    // 004a5bfe  8b95fa070000           -mov edx, dword ptr [ebp + 0x7fa]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(2042) /* 0x7fa */);
    // 004a5c04  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004a5c05  b9f05e4a00             -mov ecx, 0x4a5ef0
    cpu.ecx = 4873968 /*0x4a5ef0*/;
    // 004a5c0a  bf505e4a00             -mov edi, 0x4a5e50
    cpu.edi = 4873808 /*0x4a5e50*/;
    // 004a5c0f  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a5c10  898dea050000           -mov dword ptr [ebp + 0x5ea], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(1514) /* 0x5ea */) = cpu.ecx;
    // 004a5c16  89bdf2050000           -mov dword ptr [ebp + 0x5f2], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(1522) /* 0x5f2 */) = cpu.edi;
    // 004a5c1c  ff15e4217a00           -call dword ptr [0x7a21e4]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(8004068) /* 0x7a21e4 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a5c22  83c414                 -add esp, 0x14
    (cpu.esp) += x86::reg32(x86::sreg32(20 /*0x14*/));
    // 004a5c25  a3b0ec5500             -mov dword ptr [0x55ecb0], eax
    app->getMemory<x86::reg32>(x86::reg32(5631152) /* 0x55ecb0 */) = cpu.eax;
    // 004a5c2a  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a5c2c  7d07                   -jge 0x4a5c35
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004a5c35;
    }
    // 004a5c2e  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004a5c30  e9d1000000             -jmp 0x4a5d06
    goto L_0x004a5d06;
L_0x004a5c35:
    // 004a5c35  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 004a5c3a  beacdb5300             -mov esi, 0x53dbac
    cpu.esi = 5495724 /*0x53dbac*/;
    // 004a5c3f  8d7d46                 -lea edi, [ebp + 0x46]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(70) /* 0x46 */);
    // 004a5c42  bb00010000             -mov ebx, 0x100
    cpu.ebx = 256 /*0x100*/;
    // 004a5c47  890d5c217a00           -mov dword ptr [0x7a215c], ecx
    app->getMemory<x86::reg32>(x86::reg32(8003932) /* 0x7a215c */) = cpu.ecx;
    // 004a5c4d  894d42                 -mov dword ptr [ebp + 0x42], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(66) /* 0x42 */) = cpu.ecx;
    // 004a5c50  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
L_0x004a5c51:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a5c51  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 004a5c53  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 004a5c55  3c00                   +cmp al, 0
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
    // 004a5c57  7410                   -je 0x4a5c69
    if (cpu.flags.zf)
    {
        goto L_0x004a5c69;
    }
    // 004a5c59  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 004a5c5c  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004a5c5f  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 004a5c62  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004a5c65  3c00                   +cmp al, 0
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
    // 004a5c67  75e8                   -jne 0x4a5c51
    if (!cpu.flags.zf)
    {
        goto L_0x004a5c51;
    }
L_0x004a5c69:
    // 004a5c69  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5c6a  be21000000             -mov esi, 0x21
    cpu.esi = 33 /*0x21*/;
    // 004a5c6f  8d7d6c                 -lea edi, [ebp + 0x6c]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(108) /* 0x6c */);
    // 004a5c72  897566                 -mov dword ptr [ebp + 0x66], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(102) /* 0x66 */) = cpu.esi;
    // 004a5c75  beb4db5300             -mov esi, 0x53dbb4
    cpu.esi = 5495732 /*0x53dbb4*/;
    // 004a5c7a  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004a5c7c  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
L_0x004a5c7d:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a5c7d  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 004a5c7f  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 004a5c81  3c00                   +cmp al, 0
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
    // 004a5c83  7410                   -je 0x4a5c95
    if (cpu.flags.zf)
    {
        goto L_0x004a5c95;
    }
    // 004a5c85  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 004a5c88  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004a5c8b  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 004a5c8e  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004a5c91  3c00                   +cmp al, 0
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
    // 004a5c93  75e8                   -jne 0x4a5c7d
    if (!cpu.flags.zf)
    {
        goto L_0x004a5c7d;
    }
L_0x004a5c95:
    // 004a5c95  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5c96  bf9a020000             -mov edi, 0x29a
    cpu.edi = 666 /*0x29a*/;
    // 004a5c9b  bec0db5300             -mov esi, 0x53dbc0
    cpu.esi = 5495744 /*0x53dbc0*/;
    // 004a5ca0  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004a5ca2  897d76                 -mov dword ptr [ebp + 0x76], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */) = cpu.edi;
    // 004a5ca5  89457a                 -mov dword ptr [ebp + 0x7a], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */) = cpu.eax;
    // 004a5ca8  8d457e                 -lea eax, [ebp + 0x7e]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 004a5cab  8d7d7e                 -lea edi, [ebp + 0x7e]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 004a5cae  e88da90300             -call 0x4e0640
    cpu.esp -= 4;
    nfs3hp::watcom::memset(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a5cb3  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
L_0x004a5cb4:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a5cb4  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 004a5cb6  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 004a5cb8  3c00                   +cmp al, 0
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
    // 004a5cba  7410                   -je 0x4a5ccc
    if (cpu.flags.zf)
    {
        goto L_0x004a5ccc;
    }
    // 004a5cbc  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 004a5cbf  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004a5cc2  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 004a5cc5  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004a5cc8  3c00                   +cmp al, 0
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
    // 004a5cca  75e8                   -jne 0x4a5cb4
    if (!cpu.flags.zf)
    {
        goto L_0x004a5cb4;
    }
L_0x004a5ccc:
    // 004a5ccc  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5ccd  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 004a5ccf  8d4542                 -lea eax, [ebp + 0x42]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(66) /* 0x42 */);
    // 004a5cd2  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a5cd3  8b155c217a00           -mov edx, dword ptr [0x7a215c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8003932) /* 0x7a215c */);
    // 004a5cd9  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004a5cda  6a21                   -push 0x21
    app->getMemory<x86::reg32>(cpu.esp-4) = 33 /*0x21*/;
    cpu.esp -= 4;
    // 004a5cdc  8b0db0ec5500           -mov ecx, dword ptr [0x55ecb0]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5631152) /* 0x55ecb0 */);
    // 004a5ce2  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a5ce3  ff15dc217a00           -call dword ptr [0x7a21dc]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(8004060) /* 0x7a21dc */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a5ce9  83c414                 -add esp, 0x14
    (cpu.esp) += x86::reg32(x86::sreg32(20 /*0x14*/));
    // 004a5cec  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a5cee  7d11                   -jge 0x4a5d01
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004a5d01;
    }
    // 004a5cf0  e8db000000             -call 0x4a5dd0
    cpu.esp -= 4;
    sub_4a5dd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a5cf5  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004a5cf7  8da502080000           -lea esp, [ebp + 0x802]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(2050) /* 0x802 */);
    // 004a5cfd  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5cfe  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5cff  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5d00  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a5d01:
    // 004a5d01  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
L_0x004a5d06:
    // 004a5d06  8da502080000           -lea esp, [ebp + 0x802]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(2050) /* 0x802 */);
    // 004a5d0c  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5d0d  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5d0e  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5d0f  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4a5d10: native, nfs3hp::ea::sendOverLink */

/* sub_4a5d70: native, nfs3hp::dead::stub_0x4a5d70 */

/* align: skip 0x8b 0xc0 */
void Application::sub_4a5dd0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004a5dd0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a5dd1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a5dd2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004a5dd3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004a5dd4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004a5dd5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a5dd6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a5dd8  8b15b0ec5500           -mov edx, dword ptr [0x55ecb0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5631152) /* 0x55ecb0 */);
    // 004a5dde  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004a5de0  7c5a                   -jl 0x4a5e3c
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004a5e3c;
    }
    // 004a5de2  8b0d5c217a00           -mov ecx, dword ptr [0x7a215c]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8003932) /* 0x7a215c */);
    // 004a5de8  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004a5dea  7413                   -je 0x4a5dff
    if (cpu.flags.zf)
    {
        goto L_0x004a5dff;
    }
    // 004a5dec  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a5ded  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004a5dee  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 004a5df0  ff15e0217a00           -call dword ptr [0x7a21e0]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(8004064) /* 0x7a21e0 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a5df6  83c408                 -add esp, 8
    (cpu.esp) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 004a5df9  893d5c217a00           -mov dword ptr [0x7a215c], edi
    app->getMemory<x86::reg32>(x86::reg32(8003932) /* 0x7a215c */) = cpu.edi;
L_0x004a5dff:
    // 004a5dff  a160217a00             -mov eax, dword ptr [0x7a2160]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8003936) /* 0x7a2160 */);
    // 004a5e04  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a5e06  7419                   -je 0x4a5e21
    if (cpu.flags.zf)
    {
        goto L_0x004a5e21;
    }
    // 004a5e08  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a5e09  8b0db0ec5500           -mov ecx, dword ptr [0x55ecb0]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5631152) /* 0x55ecb0 */);
    // 004a5e0f  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a5e10  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004a5e12  ff15e0217a00           -call dword ptr [0x7a21e0]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(8004064) /* 0x7a21e0 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a5e18  83c408                 -add esp, 8
    (cpu.esp) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 004a5e1b  891d60217a00           -mov dword ptr [0x7a2160], ebx
    app->getMemory<x86::reg32>(x86::reg32(8003936) /* 0x7a2160 */) = cpu.ebx;
L_0x004a5e21:
    // 004a5e21  8b35b0ec5500           -mov esi, dword ptr [0x55ecb0]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(5631152) /* 0x55ecb0 */);
    // 004a5e27  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004a5e28  bfecffffff             -mov edi, 0xffffffec
    cpu.edi = 4294967276 /*0xffffffec*/;
    // 004a5e2d  ff15cc217a00           -call dword ptr [0x7a21cc]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(8004044) /* 0x7a21cc */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a5e33  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004a5e36  893db0ec5500           -mov dword ptr [0x55ecb0], edi
    app->getMemory<x86::reg32>(x86::reg32(5631152) /* 0x55ecb0 */) = cpu.edi;
L_0x004a5e3c:
    // 004a5e3c  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5e3d  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5e3e  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5e3f  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5e40  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5e41  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5e42  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_4a5e50(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004a5e50  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a5e51  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a5e53  8b450c                 -mov eax, dword ptr [ebp + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(12) /* 0xc */);
    // 004a5e56  8b5510                 -mov edx, dword ptr [ebp + 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(16) /* 0x10 */);
    // 004a5e59  663d0502               +cmp ax, 0x205
    {
        x86::reg16 tmp1 = cpu.ax;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(517 /*0x205*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004a5e5d  7230                   -jb 0x4a5e8f
    if (cpu.flags.cf)
    {
        goto L_0x004a5e8f;
    }
    // 004a5e5f  0f8685000000           -jbe 0x4a5eea
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004a5eea;
    }
    // 004a5e65  663d0802               +cmp ax, 0x208
    {
        x86::reg16 tmp1 = cpu.ax;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(520 /*0x208*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004a5e69  721c                   -jb 0x4a5e87
    if (cpu.flags.cf)
    {
        goto L_0x004a5e87;
    }
    // 004a5e6b  0f8679000000           -jbe 0x4a5eea
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004a5eea;
    }
    // 004a5e71  663d0902               +cmp ax, 0x209
    {
        x86::reg16 tmp1 = cpu.ax;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(521 /*0x209*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004a5e75  0f866f000000           -jbe 0x4a5eea
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004a5eea;
    }
    // 004a5e7b  663da002               +cmp ax, 0x2a0
    {
        x86::reg16 tmp1 = cpu.ax;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(672 /*0x2a0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004a5e7f  0f8465000000           -je 0x4a5eea
    if (cpu.flags.zf)
    {
        goto L_0x004a5eea;
    }
    // 004a5e85  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5e86  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a5e87:
    // 004a5e87  663d0602               +cmp ax, 0x206
    {
        x86::reg16 tmp1 = cpu.ax;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(518 /*0x206*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004a5e8b  745d                   -je 0x4a5eea
    if (cpu.flags.zf)
    {
        goto L_0x004a5eea;
    }
    // 004a5e8d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5e8e  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a5e8f:
    // 004a5e8f  663d0202               +cmp ax, 0x202
    {
        x86::reg16 tmp1 = cpu.ax;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(514 /*0x202*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004a5e93  720a                   -jb 0x4a5e9f
    if (cpu.flags.cf)
    {
        goto L_0x004a5e9f;
    }
    // 004a5e95  7610                   -jbe 0x4a5ea7
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004a5ea7;
    }
    // 004a5e97  663d0302               +cmp ax, 0x203
    {
        x86::reg16 tmp1 = cpu.ax;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(515 /*0x203*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004a5e9b  7613                   -jbe 0x4a5eb0
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004a5eb0;
    }
    // 004a5e9d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5e9e  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a5e9f:
    // 004a5e9f  663d0102               +cmp ax, 0x201
    {
        x86::reg16 tmp1 = cpu.ax;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(513 /*0x201*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004a5ea3  7445                   -je 0x4a5eea
    if (cpu.flags.zf)
    {
        goto L_0x004a5eea;
    }
    // 004a5ea5  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5ea6  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a5ea7:
    // 004a5ea7  8b02                   -mov eax, dword ptr [edx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx);
    // 004a5ea9  e882000000             -call 0x4a5f30
    cpu.esp -= 4;
    sub_4a5f30(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a5eae  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5eaf  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a5eb0:
    // 004a5eb0  66833dda227a0000       +cmp word ptr [0x7a22da], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(x86::reg32(8004314) /* 0x7a22da */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004a5eb8  7519                   -jne 0x4a5ed3
    if (!cpu.flags.zf)
    {
        goto L_0x004a5ed3;
    }
    // 004a5eba  807a6401               +cmp byte ptr [edx + 0x64], 1
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(100) /* 0x64 */);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(1 /*0x1*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 004a5ebe  752a                   -jne 0x4a5eea
    if (!cpu.flags.zf)
    {
        goto L_0x004a5eea;
    }
    // 004a5ec0  c7050c237a0001000000   -mov dword ptr [0x7a230c], 1
    app->getMemory<x86::reg32>(x86::reg32(8004364) /* 0x7a230c */) = 1 /*0x1*/;
    // 004a5eca  8a02                   -mov al, byte ptr [edx]
    cpu.al = app->getMemory<x86::reg8>(cpu.edx);
    // 004a5ecc  a21c237a00             -mov byte ptr [0x7a231c], al
    app->getMemory<x86::reg8>(x86::reg32(8004380) /* 0x7a231c */) = cpu.al;
    // 004a5ed1  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5ed2  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a5ed3:
    // 004a5ed3  a1a0207a00             -mov eax, dword ptr [0x7a20a0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8003744) /* 0x7a20a0 */);
    // 004a5ed8  3b02                   +cmp eax, dword ptr [edx]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.edx)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a5eda  740e                   -je 0x4a5eea
    if (cpu.flags.zf)
    {
        goto L_0x004a5eea;
    }
    // 004a5edc  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 004a5ede  8d4204                 -lea eax, [edx + 4]
    cpu.eax = x86::reg32(cpu.edx + x86::reg32(4) /* 0x4 */);
    // 004a5ee1  8a0a                   -mov cl, byte ptr [edx]
    cpu.cl = app->getMemory<x86::reg8>(cpu.edx);
    // 004a5ee3  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 004a5ee5  e83684ffff             -call 0x49e320
    cpu.esp -= 4;
    sub_49e320(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004a5eea:
    // 004a5eea  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5eeb  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_4a5ef0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004a5ef0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a5ef1  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a5ef2  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a5ef4  8b4514                 -mov eax, dword ptr [ebp + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    // 004a5ef7  8b551c                 -mov edx, dword ptr [ebp + 0x1c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(28) /* 0x1c */);
    // 004a5efa  e831000000             -call 0x4a5f30
    cpu.esp -= 4;
    sub_4a5f30(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a5eff  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004a5f01  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a5f03  7409                   -je 0x4a5f0e
    if (cpu.flags.zf)
    {
        goto L_0x004a5f0e;
    }
    // 004a5f05  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004a5f07  668b5d20               -mov bx, word ptr [ebp + 0x20]
    cpu.bx = app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(32) /* 0x20 */);
    // 004a5f0b  ff5120                 -call dword ptr [ecx + 0x20]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(32) /* 0x20 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004a5f0e:
    // 004a5f0e  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5f0f  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5f10  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4a5f20: native, nfs3hp::dead::stub_0x4a5f20 */

/* align: skip 0x8b 0xc0 */
void Application::sub_4a5f30(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004a5f30  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a5f31  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a5f32  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004a5f33  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a5f34  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a5f36  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 004a5f38  31c9                   +xor ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx))));
    // 004a5f3a  eb06                   -jmp 0x4a5f42
    goto L_0x004a5f42;
L_0x004a5f3c:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a5f3c  41                     -inc ecx
    (cpu.ecx)++;
    // 004a5f3d  83f908                 +cmp ecx, 8
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(8 /*0x8*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a5f40  7d29                   -jge 0x4a5f6b
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004a5f6b;
    }
L_0x004a5f42:
    // 004a5f42  8d048d00000000         -lea eax, [ecx*4]
    cpu.eax = x86::reg32(cpu.ecx * 4);
    // 004a5f49  29c8                   -sub eax, ecx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004a5f4b  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004a5f4e  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004a5f50  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 004a5f53  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004a5f55  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004a5f57  8a901c237a00           -mov dl, byte ptr [eax + 0x7a231c]
    cpu.dl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(8004380) /* 0x7a231c */);
    // 004a5f5d  39da                   +cmp edx, ebx
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a5f5f  75db                   -jne 0x4a5f3c
    if (!cpu.flags.zf)
    {
        goto L_0x004a5f3c;
    }
    // 004a5f61  05e8227a00             -add eax, 0x7a22e8
    (cpu.eax) += x86::reg32(x86::sreg32(8004328 /*0x7a22e8*/));
    // 004a5f66  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5f67  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5f68  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5f69  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5f6a  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a5f6b:
    // 004a5f6b  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004a5f6d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5f6e  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5f6f  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5f70  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a5f71  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4a5f80: native, nfs3hp::dead::stub_0x4a5f80 */

/* sub_4a5f90: native, nfs3hp::dead::stub_0x4a5f90 */

/* sub_4a5fb0: native, nfs3hp::dead::stub_0x4a5fb0 */

/* sub_4a5ff0: native, nfs3hp::dead::stub_0x4a5ff0 */

/* sub_4a6020: native, nfs3hp::dead::stub_0x4a6020 */

/* sub_4a60a0: native, nfs3hp::dead::stub_0x4a60a0 */

/* sub_4a60ab: native, nfs3hp::dead::stub_0x4a60ab */

/* sub_4a60d0: native, nfs3hp::dead::stub_0x4a60d0 */

/* sub_4a60e0: native, nfs3hp::ea::sendPacket */

/* sub_4a6390: native, nfs3hp::ea::resetNetwork */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_4a6410(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004a6410  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a6411  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a6413  c7401870644a00         -mov dword ptr [eax + 0x18], 0x4a6470
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(24) /* 0x18 */) = 4875376 /*0x4a6470*/;
    // 004a641a  c7402040644a00         -mov dword ptr [eax + 0x20], 0x4a6440
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(32) /* 0x20 */) = 4875328 /*0x4a6440*/;
    // 004a6421  c7401c00000000         -mov dword ptr [eax + 0x1c], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(28) /* 0x1c */) = 0 /*0x0*/;
    // 004a6428  c74014c0644a00         -mov dword ptr [eax + 0x14], 0x4a64c0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(20) /* 0x14 */) = 4875456 /*0x4a64c0*/;
    // 004a642f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6430  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_4a6440(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004a6440  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a6441  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004a6442  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a6443  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a6445  668b35dc227a00         -mov si, word ptr [0x7a22dc]
    cpu.si = app->getMemory<x86::reg16>(x86::reg32(8004316) /* 0x7a22dc */);
    // 004a644c  8a0a                   -mov cl, byte ptr [edx]
    cpu.cl = app->getMemory<x86::reg8>(cpu.edx);
    // 004a644e  6685f6                 +test si, si
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg16>(cpu.si & cpu.si));
    // 004a6451  7512                   -jne 0x4a6465
    if (!cpu.flags.zf)
    {
        goto L_0x004a6465;
    }
    // 004a6453  80f913                 +cmp cl, 0x13
    {
        x86::reg8 tmp1 = cpu.cl;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(19 /*0x13*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 004a6456  730d                   -jae 0x4a6465
    if (!cpu.flags.cf)
    {
        goto L_0x004a6465;
    }
    // 004a6458  81e1ff000000           -and ecx, 0xff
    cpu.ecx &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 004a645e  ff148dbcec5500         -call dword ptr [ecx*4 + 0x55ecbc]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5631164) /* 0x55ecbc */ + cpu.ecx * 4);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004a6465:
    // 004a6465  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004a646a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a646b  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a646c  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a646d  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_4a6470(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004a6470  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a6471  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a6472  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a6474  66c70596227a000100     -mov word ptr [0x7a2296], 1
    app->getMemory<x86::reg16>(x86::reg32(8004246) /* 0x7a2296 */) = 1 /*0x1*/;
    // 004a647d  833de0227a0003         +cmp dword ptr [0x7a22e0], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8004320) /* 0x7a22e0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a6484  750a                   -jne 0x4a6490
    if (!cpu.flags.zf)
    {
        goto L_0x004a6490;
    }
    // 004a6486  66833dda227a0000       +cmp word ptr [0x7a22da], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(x86::reg32(8004314) /* 0x7a22da */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004a648e  7518                   -jne 0x4a64a8
    if (!cpu.flags.zf)
    {
        goto L_0x004a64a8;
    }
L_0x004a6490:
    // 004a6490  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 004a6495  66890dde227a00         -mov word ptr [0x7a22de], cx
    app->getMemory<x86::reg16>(x86::reg32(8004318) /* 0x7a22de */) = cpu.cx;
    // 004a649c  66890ddc227a00         -mov word ptr [0x7a22dc], cx
    app->getMemory<x86::reg16>(x86::reg32(8004316) /* 0x7a22dc */) = cpu.cx;
    // 004a64a3  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004a64a5  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a64a6  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a64a7  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a64a8:
    // 004a64a8  e8a3010000             -call 0x4a6650
    cpu.esp -= 4;
    nfs3hp::ea::machineNumber(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a64ad  98                     -cwde 
    cpu.eax = x86::reg32(static_cast<x86::sreg16>(cpu.ax));
    // 004a64ae  83f8ff                 +cmp eax, -1
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
    // 004a64b1  7e05                   -jle 0x4a64b8
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004a64b8;
    }
    // 004a64b3  e83888ffff             -call 0x49ecf0
    cpu.esp -= 4;
    nfs3hp::ea::announceMachineLeft(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004a64b8:
    // 004a64b8  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004a64ba  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a64bb  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a64bc  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_4a64c0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004a64c0  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a64c1  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a64c3  66833dda227a0000       +cmp word ptr [0x7a22da], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(x86::reg32(8004314) /* 0x7a22da */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004a64cb  741d                   -je 0x4a64ea
    if (cpu.flags.zf)
    {
        goto L_0x004a64ea;
    }
    // 004a64cd  833de0227a0003         +cmp dword ptr [0x7a22e0], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8004320) /* 0x7a22e0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a64d4  7514                   -jne 0x4a64ea
    if (!cpu.flags.zf)
    {
        goto L_0x004a64ea;
    }
    // 004a64d6  a1a4c17900             -mov eax, dword ptr [0x79c1a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7979428) /* 0x79c1a4 */);
    // 004a64db  3b0514e75500           +cmp eax, dword ptr [0x55e714]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5629716) /* 0x55e714 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a64e1  7e07                   -jle 0x4a64ea
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004a64ea;
    }
    // 004a64e3  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004a64e8  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a64e9  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a64ea:
    // 004a64ea  803d1948560000         +cmp byte ptr [0x564819], 0
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(x86::reg32(5654553) /* 0x564819 */);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 004a64f1  7407                   -je 0x4a64fa
    if (cpu.flags.zf)
    {
        goto L_0x004a64fa;
    }
    // 004a64f3  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004a64f8  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a64f9  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a64fa:
    // 004a64fa  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004a64fc  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a64fd  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_4a6500(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004a6500  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a6501  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a6502  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004a6503  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a6504  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a6506  ba80070000             -mov edx, 0x780
    cpu.edx = 1920 /*0x780*/;
    // 004a650b  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004a650d  8915f06d5600           -mov dword ptr [0x566df0], edx
    app->getMemory<x86::reg32>(x86::reg32(5664240) /* 0x566df0 */) = cpu.edx;
    // 004a6513  40                     -inc eax
    (cpu.eax)++;
    // 004a6514  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004a6516  6689144592227a00       -mov word ptr [eax*2 + 0x7a2292], dx
    app->getMemory<x86::reg16>(x86::reg32(8004242) /* 0x7a2292 */ + cpu.eax * 2) = cpu.dx;
L_0x004a651e:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a651e  83f813                 +cmp eax, 0x13
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(19 /*0x13*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a6521  7d0d                   -jge 0x4a6530
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004a6530;
    }
    // 004a6523  40                     -inc eax
    (cpu.eax)++;
    // 004a6524  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 004a6526  6689144592227a00       -mov word ptr [eax*2 + 0x7a2292], dx
    app->getMemory<x86::reg16>(x86::reg32(8004242) /* 0x7a2292 */ + cpu.eax * 2) = cpu.dx;
    // 004a652e  ebee                   -jmp 0x4a651e
    goto L_0x004a651e;
L_0x004a6530:
    // 004a6530  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004a6532  eb05                   -jmp 0x4a6539
    goto L_0x004a6539;
L_0x004a6534:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a6534  83f808                 +cmp eax, 8
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
    // 004a6537  7d15                   -jge 0x4a654e
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004a654e;
    }
L_0x004a6539:
    // 004a6539  40                     -inc eax
    (cpu.eax)++;
    // 004a653a  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 004a653c  66891c45b8227a00       -mov word ptr [eax*2 + 0x7a22b8], bx
    app->getMemory<x86::reg16>(x86::reg32(8004280) /* 0x7a22b8 */ + cpu.eax * 2) = cpu.bx;
    // 004a6544  66891c45c8227a00       -mov word ptr [eax*2 + 0x7a22c8], bx
    app->getMemory<x86::reg16>(x86::reg32(8004296) /* 0x7a22c8 */ + cpu.eax * 2) = cpu.bx;
    // 004a654c  ebe6                   -jmp 0x4a6534
    goto L_0x004a6534;
L_0x004a654e:
    // 004a654e  bb10000000             -mov ebx, 0x10
    cpu.ebx = 16 /*0x10*/;
    // 004a6553  31c9                   +xor ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx))));
    // 004a6555  ba4c030000             -mov edx, 0x34c
    cpu.edx = 844 /*0x34c*/;
    // 004a655a  890d10227a00           -mov dword ptr [0x7a2210], ecx
    app->getMemory<x86::reg32>(x86::reg32(8004112) /* 0x7a2210 */) = cpu.ecx;
    // 004a6560  e82b3fffff             -call 0x49a490
    cpu.esp -= 4;
    sub_49a490(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a6565  891df4435600           -mov dword ptr [0x5643f4], ebx
    app->getMemory<x86::reg32>(x86::reg32(5653492) /* 0x5643f4 */) = cpu.ebx;
    // 004a656b  e8c0ec0400             -call 0x4f5230
    cpu.esp -= 4;
    sub_4f5230(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a6570  b8e8227a00             -mov eax, 0x7a22e8
    cpu.eax = 8004328 /*0x7a22e8*/;
    // 004a6575  890df4435600           -mov dword ptr [0x5643f4], ecx
    app->getMemory<x86::reg32>(x86::reg32(5653492) /* 0x5643f4 */) = cpu.ecx;
    // 004a657b  e88ca10300             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a6580  eb05                   -jmp 0x4a6587
    goto L_0x004a6587;
L_0x004a6582:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a6582  83f907                 +cmp ecx, 7
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(7 /*0x7*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a6585  7d20                   -jge 0x4a65a7
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004a65a7;
    }
L_0x004a6587:
    // 004a6587  8d048d00000000         -lea eax, [ecx*4]
    cpu.eax = x86::reg32(cpu.ecx * 4);
    // 004a658e  29c8                   -sub eax, ecx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004a6590  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004a6593  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004a6595  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 004a6598  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004a659a  05e8227a00             +add eax, 0x7a22e8
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(8004328 /*0x7a22e8*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004a659f  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004a65a0  e86bfeffff             -call 0x4a6410
    cpu.esp -= 4;
    sub_4a6410(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a65a5  ebdb                   -jmp 0x4a6582
    goto L_0x004a6582;
L_0x004a65a7:
    // 004a65a7  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a65a8  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a65a9  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a65aa  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a65ab  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_4a65b0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004a65b0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004a65b1  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a65b2  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a65b4  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004a65b6  e875100400             -call 0x4e7630
    cpu.esp -= 4;
    nfs3hp::port::runSystemTasks(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a65bb  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004a65bd  e8fefeffff             -call 0x4a64c0
    cpu.esp -= 4;
    sub_4a64c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a65c2  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004a65c4  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a65c6  7409                   -je 0x4a65d1
    if (cpu.flags.zf)
    {
        goto L_0x004a65d1;
    }
L_0x004a65c8:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a65c8  803d1948560000         +cmp byte ptr [0x564819], 0
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(x86::reg32(5654553) /* 0x564819 */);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 004a65cf  75f7                   -jne 0x4a65c8
    if (!cpu.flags.zf)
    {
        goto L_0x004a65c8;
    }
L_0x004a65d1:
    // 004a65d1  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004a65d3  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a65d4  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a65d5  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4a65e0: native, nfs3hp::ea::closeNetwork */

/* sub_4a6650: native, nfs3hp::ea::machineNumber */

/* sub_4a6690: native, nfs3hp::ea::playerMachine */

/* align: skip  */
void Application::sub_4a66c0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004a66c0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a66c1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a66c2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004a66c3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004a66c4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004a66c5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a66c6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a66c8  81ec80000000           -sub esp, 0x80
    (cpu.esp) -= x86::reg32(x86::sreg32(128 /*0x80*/));
    // 004a66ce  81ed82000000           -sub ebp, 0x82
    (cpu.ebp) -= x86::reg32(x86::sreg32(130 /*0x82*/));
    // 004a66d4  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 004a66d9  befcd26f00             -mov esi, 0x6fd2fc
    cpu.esi = 7328508 /*0x6fd2fc*/;
    // 004a66de  bf15227a00             -mov edi, 0x7a2215
    cpu.edi = 8004117 /*0x7a2215*/;
    // 004a66e3  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004a66e5  a1e4227a00             -mov eax, dword ptr [0x7a22e4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8004324) /* 0x7a22e4 */);
    // 004a66ea  66891578227a00         -mov word ptr [0x7a2278], dx
    app->getMemory<x86::reg16>(x86::reg32(8004216) /* 0x7a2278 */) = cpu.dx;
    // 004a66f1  66891d76227a00         -mov word ptr [0x7a2276], bx
    app->getMemory<x86::reg16>(x86::reg32(8004214) /* 0x7a2276 */) = cpu.bx;
    // 004a66f8  881d14227a00           -mov byte ptr [0x7a2214], bl
    app->getMemory<x86::reg8>(x86::reg32(8004116) /* 0x7a2214 */) = cpu.bl;
    // 004a66fe  a38c227a00             -mov dword ptr [0x7a228c], eax
    app->getMemory<x86::reg32>(x86::reg32(8004236) /* 0x7a228c */) = cpu.eax;
    // 004a6703  66891596227a00         -mov word ptr [0x7a2296], dx
    app->getMemory<x86::reg16>(x86::reg32(8004246) /* 0x7a2296 */) = cpu.dx;
    // 004a670a  e8e1f3ffff             -call 0x4a5af0
    cpu.esp -= 4;
    sub_4a5af0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a670f  a26d227a00             -mov byte ptr [0x7a226d], al
    app->getMemory<x86::reg8>(x86::reg32(8004205) /* 0x7a226d */) = cpu.al;
    // 004a6714  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004a6716  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
L_0x004a6717:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a6717  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 004a6719  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 004a671b  3c00                   +cmp al, 0
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
    // 004a671d  7410                   -je 0x4a672f
    if (cpu.flags.zf)
    {
        goto L_0x004a672f;
    }
    // 004a671f  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 004a6722  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004a6725  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 004a6728  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004a672b  3c00                   +cmp al, 0
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
    // 004a672d  75e8                   -jne 0x4a6717
    if (!cpu.flags.zf)
    {
        goto L_0x004a6717;
    }
L_0x004a672f:
    // 004a672f  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6730  89557e                 -mov dword ptr [ebp + 0x7e], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.edx;
    // 004a6733  eb0a                   -jmp 0x4a673f
    goto L_0x004a673f;
L_0x004a6735:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a6735  837d7e07               +cmp dword ptr [ebp + 0x7e], 7
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(7 /*0x7*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a6739  0f8db0000000           -jge 0x4a67ef
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004a67ef;
    }
L_0x004a673f:
    // 004a673f  8b557e                 -mov edx, dword ptr [ebp + 0x7e]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 004a6742  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004a6749  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004a674b  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004a674e  8d0cc500000000         -lea ecx, [eax*8]
    cpu.ecx = x86::reg32(cpu.eax * 8);
    // 004a6755  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004a6757  83b90c237a0000         +cmp dword ptr [ecx + 0x7a230c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(8004364) /* 0x7a230c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a675e  0f8483000000           -je 0x4a67e7
    if (cpu.flags.zf)
    {
        goto L_0x004a67e7;
    }
    // 004a6764  8b3d74227a00           -mov edi, dword ptr [0x7a2274]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(8004212) /* 0x7a2274 */);
    // 004a676a  bae8227a00             -mov edx, 0x7a22e8
    cpu.edx = 8004328 /*0x7a22e8*/;
    // 004a676f  c1ff10                 -sar edi, 0x10
    cpu.edi = x86::reg32(x86::sreg32(cpu.edi) >> (16 /*0x10*/ % 32));
    // 004a6772  01ca                   -add edx, ecx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004a6774  8914bdf0217a00         -mov dword ptr [edi*4 + 0x7a21f0], edx
    app->getMemory<x86::reg32>(x86::reg32(8004080) /* 0x7a21f0 */ + cpu.edi * 4) = cpu.edx;
    // 004a677b  8b557e                 -mov edx, dword ptr [ebp + 0x7e]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 004a677e  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004a6785  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004a6787  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004a678a  bedc257a00             -mov esi, 0x7a25dc
    cpu.esi = 8005084 /*0x7a25dc*/;
    // 004a678f  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004a6791  01c6                   -add esi, eax
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.eax));
    // 004a6793  8d04bd00000000         -lea eax, [edi*4]
    cpu.eax = x86::reg32(cpu.edi * 4);
    // 004a679a  29f8                   -sub eax, edi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edi));
    // 004a679c  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004a679f  29f8                   -sub eax, edi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edi));
    // 004a67a1  bf15227a00             -mov edi, 0x7a2215
    cpu.edi = 8004117 /*0x7a2215*/;
    // 004a67a6  01c7                   -add edi, eax
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.eax));
    // 004a67a8  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
L_0x004a67a9:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a67a9  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 004a67ab  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 004a67ad  3c00                   +cmp al, 0
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
    // 004a67af  7410                   -je 0x4a67c1
    if (cpu.flags.zf)
    {
        goto L_0x004a67c1;
    }
    // 004a67b1  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 004a67b4  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004a67b7  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 004a67ba  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004a67bd  3c00                   +cmp al, 0
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
    // 004a67bf  75e8                   -jne 0x4a67a9
    if (!cpu.flags.zf)
    {
        goto L_0x004a67a9;
    }
L_0x004a67c1:
    // 004a67c1  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a67c2  833de4227a0002         +cmp dword ptr [0x7a22e4], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8004324) /* 0x7a22e4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a67c9  7515                   -jne 0x4a67e0
    if (!cpu.flags.zf)
    {
        goto L_0x004a67e0;
    }
    // 004a67cb  8b1574227a00           -mov edx, dword ptr [0x7a2274]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8004212) /* 0x7a2274 */);
    // 004a67d1  c1fa10                 +sar edx, 0x10
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
    // 004a67d4  8a811c237a00           -mov al, byte ptr [ecx + 0x7a231c]
    cpu.al = app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(8004380) /* 0x7a231c */);
    // 004a67da  88826d227a00           -mov byte ptr [edx + 0x7a226d], al
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(8004205) /* 0x7a226d */) = cpu.al;
L_0x004a67e0:
    // 004a67e0  66ff0576227a00         +inc word ptr [0x7a2276]
    {
        auto tmp = app->getMemory<x86::reg16>(x86::reg32(8004214) /* 0x7a2276 */);
        cpu.flags.of = ~(1 & (tmp >> 15));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 15);
        cpu.set_szp(tmp);
    }
L_0x004a67e7:
    // 004a67e7  ff457e                 +inc dword ptr [ebp + 0x7e]
    {
        auto tmp = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004a67ea  e946ffffff             -jmp 0x4a6735
    goto L_0x004a6735;
L_0x004a67ef:
    // 004a67ef  31ff                   +xor edi, edi
    cpu.clear_co();
    cpu.set_szp((cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi))));
    // 004a67f1  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004a67f6  66893d7a227a00         -mov word ptr [0x7a227a], di
    app->getMemory<x86::reg16>(x86::reg32(8004218) /* 0x7a227a */) = cpu.di;
    // 004a67fd  66893dca227a00         -mov word ptr [0x7a22ca], di
    app->getMemory<x86::reg16>(x86::reg32(8004298) /* 0x7a22ca */) = cpu.di;
    // 004a6804  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
    // 004a6807  eb06                   -jmp 0x4a680f
    goto L_0x004a680f;
L_0x004a6809:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a6809  837d7e08               +cmp dword ptr [ebp + 0x7e], 8
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(8 /*0x8*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a680d  7d3b                   -jge 0x4a684a
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004a684a;
    }
L_0x004a680f:
    // 004a680f  8b4d7e                 -mov ecx, dword ptr [ebp + 0x7e]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 004a6812  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004a6814  6689144dca227a00       -mov word ptr [ecx*2 + 0x7a22ca], dx
    app->getMemory<x86::reg16>(x86::reg32(8004298) /* 0x7a22ca */ + cpu.ecx * 2) = cpu.dx;
    // 004a681c  8d51ff                 -lea edx, [ecx - 1]
    cpu.edx = x86::reg32(cpu.ecx + x86::reg32(-1) /* -0x1 */);
    // 004a681f  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004a6826  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004a6828  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004a682b  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004a682d  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 004a6830  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004a6832  41                     -inc ecx
    (cpu.ecx)++;
    // 004a6833  05e8227a00             +add eax, 0x7a22e8
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(8004328 /*0x7a22e8*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004a6838  894d7e                 -mov dword ptr [ebp + 0x7e], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.ecx;
    // 004a683b  e810feffff             -call 0x4a6650
    cpu.esp -= 4;
    nfs3hp::ea::machineNumber(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a6840  6689044d78227a00       -mov word ptr [ecx*2 + 0x7a2278], ax
    app->getMemory<x86::reg16>(x86::reg32(8004216) /* 0x7a2278 */ + cpu.ecx * 2) = cpu.ax;
    // 004a6848  ebbf                   -jmp 0x4a6809
    goto L_0x004a6809;
L_0x004a684a:
    // 004a684a  b97c000000             -mov ecx, 0x7c
    cpu.ecx = 124 /*0x7c*/;
    // 004a684f  be14227a00             -mov esi, 0x7a2214
    cpu.esi = 8004116 /*0x7a2214*/;
    // 004a6854  8d7d02                 -lea edi, [ebp + 2]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(2) /* 0x2 */);
    // 004a6857  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004a6858  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004a685a  c1e902                 -shr ecx, 2
    cpu.ecx >>= 2 /*0x2*/ % 32;
    // 004a685d  f2a5                   -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004a685f  8ac8                   -mov cl, al
    cpu.cl = cpu.al;
    // 004a6861  80e103                 +and cl, 3
    cpu.clear_co();
    cpu.set_szp((cpu.cl &= x86::reg8(x86::sreg8(3 /*0x3*/))));
    // 004a6864  f2a4                   -repne movsb byte ptr es:[edi], byte ptr [esi]
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
    // 004a6866  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6867  8b1db4ec5500           -mov ebx, dword ptr [0x55ecb4]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5631156) /* 0x55ecb4 */);
    // 004a686d  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a686e  be01000000             -mov esi, 1
    cpu.esi = 1 /*0x1*/;
    // 004a6873  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a6879  89757e                 -mov dword ptr [ebp + 0x7e], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.esi;
L_0x004a687c:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a687c  a174227a00             -mov eax, dword ptr [0x7a2274]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8004212) /* 0x7a2274 */);
    // 004a6881  8b7d7e                 -mov edi, dword ptr [ebp + 0x7e]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 004a6884  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 004a6887  39f8                   +cmp eax, edi
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
    // 004a6889  7e23                   -jle 0x4a68ae
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004a68ae;
    }
    // 004a688b  8b457e                 -mov eax, dword ptr [ebp + 0x7e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 004a688e  bb7c000000             -mov ebx, 0x7c
    cpu.ebx = 124 /*0x7c*/;
    // 004a6893  8d5502                 -lea edx, [ebp + 2]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(2) /* 0x2 */);
    // 004a6896  66894566               -mov word ptr [ebp + 0x66], ax
    app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(102) /* 0x66 */) = cpu.ax;
    // 004a689a  8b04bdf0217a00         -mov eax, dword ptr [edi*4 + 0x7a21f0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8004080) /* 0x7a21f0 */ + cpu.edi * 4);
    // 004a68a1  e83af8ffff             -call 0x4a60e0
    cpu.esp -= 4;
    nfs3hp::ea::sendPacket(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a68a6  8d4701                 -lea eax, [edi + 1]
    cpu.eax = x86::reg32(cpu.edi + x86::reg32(1) /* 0x1 */);
    // 004a68a9  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
    // 004a68ac  ebce                   -jmp 0x4a687c
    goto L_0x004a687c;
L_0x004a68ae:
    // 004a68ae  8b15b4ec5500           -mov edx, dword ptr [0x55ecb4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5631156) /* 0x55ecb4 */);
    // 004a68b4  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004a68b5  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a68bb  e8c041ffff             -call 0x49aa80
    cpu.esp -= 4;
    sub_49aa80(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a68c0  8da582000000           -lea esp, [ebp + 0x82]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(130) /* 0x82 */);
    // 004a68c6  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a68c7  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a68c8  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a68c9  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a68ca  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a68cb  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a68cc  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_4a68d0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004a68d0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a68d1  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004a68d2  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004a68d3  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a68d4  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a68d6  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 004a68d8  833d10227a0000         +cmp dword ptr [0x7a2210], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8004112) /* 0x7a2210 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a68df  740d                   -je 0x4a68ee
    if (cpu.flags.zf)
    {
        goto L_0x004a68ee;
    }
    // 004a68e1  8b0db4ec5500           -mov ecx, dword ptr [0x55ecb4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5631156) /* 0x55ecb4 */);
    // 004a68e7  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a68e8  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004a68ee:
    // 004a68ee  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004a68f0  40                     -inc eax
    (cpu.eax)++;
    // 004a68f1  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004a68f3  66891445c8227a00       -mov word ptr [eax*2 + 0x7a22c8], dx
    app->getMemory<x86::reg16>(x86::reg32(8004296) /* 0x7a22c8 */ + cpu.eax * 2) = cpu.dx;
L_0x004a68fb:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a68fb  83f808                 +cmp eax, 8
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
    // 004a68fe  7d0d                   -jge 0x4a690d
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004a690d;
    }
    // 004a6900  40                     -inc eax
    (cpu.eax)++;
    // 004a6901  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 004a6903  66891445c8227a00       -mov word ptr [eax*2 + 0x7a22c8], dx
    app->getMemory<x86::reg16>(x86::reg32(8004296) /* 0x7a22c8 */ + cpu.eax * 2) = cpu.dx;
    // 004a690b  ebee                   -jmp 0x4a68fb
    goto L_0x004a68fb;
L_0x004a690d:
    // 004a690d  b97c000000             -mov ecx, 0x7c
    cpu.ecx = 124 /*0x7c*/;
    // 004a6912  bf14227a00             -mov edi, 0x7a2214
    cpu.edi = 8004116 /*0x7a2214*/;
    // 004a6917  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 004a691c  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004a691d  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004a691f  c1e902                 -shr ecx, 2
    cpu.ecx >>= 2 /*0x2*/ % 32;
    // 004a6922  f2a5                   -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004a6924  8ac8                   -mov cl, al
    cpu.cl = cpu.al;
    // 004a6926  80e103                 +and cl, 3
    cpu.clear_co();
    cpu.set_szp((cpu.cl &= x86::reg8(x86::sreg8(3 /*0x3*/))));
    // 004a6929  f2a4                   -repne movsb byte ptr es:[edi], byte ptr [esi]
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
    // 004a692b  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a692c  66891d96227a00         -mov word ptr [0x7a2296], bx
    app->getMemory<x86::reg16>(x86::reg32(8004246) /* 0x7a2296 */) = cpu.bx;
    // 004a6933  833de4227a0002         +cmp dword ptr [0x7a22e4], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8004324) /* 0x7a22e4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a693a  755e                   -jne 0x4a699a
    if (!cpu.flags.zf)
    {
        goto L_0x004a699a;
    }
    // 004a693c  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 004a693e  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 004a6940  eb49                   -jmp 0x4a698b
    goto L_0x004a698b;
L_0x004a6942:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a6942  8d048d00000000         -lea eax, [ecx*4]
    cpu.eax = x86::reg32(cpu.ecx * 4);
    // 004a6949  29c8                   -sub eax, ecx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004a694b  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004a694e  8b3574227a00           -mov esi, dword ptr [0x7a2274]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(8004212) /* 0x7a2274 */);
    // 004a6954  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 004a6956  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 004a6959  c1fe10                 -sar esi, 0x10
    cpu.esi = x86::reg32(x86::sreg32(cpu.esi) >> (16 /*0x10*/ % 32));
    // 004a695c  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 004a695e  39f2                   +cmp edx, esi
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.esi));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a6960  7d12                   -jge 0x4a6974
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004a6974;
    }
    // 004a6962  c7800c237a0001000000   -mov dword ptr [eax + 0x7a230c], 1
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8004364) /* 0x7a230c */) = 1 /*0x1*/;
    // 004a696c  8a9a6d227a00           -mov bl, byte ptr [edx + 0x7a226d]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(8004205) /* 0x7a226d */);
    // 004a6972  eb0a                   -jmp 0x4a697e
    goto L_0x004a697e;
L_0x004a6974:
    // 004a6974  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 004a6976  89b80c237a00           -mov dword ptr [eax + 0x7a230c], edi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8004364) /* 0x7a230c */) = cpu.edi;
    // 004a697c  30db                   -xor bl, bl
    cpu.bl ^= x86::reg8(x86::sreg8(cpu.bl));
L_0x004a697e:
    // 004a697e  88981c237a00           -mov byte ptr [eax + 0x7a231c], bl
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(8004380) /* 0x7a231c */) = cpu.bl;
    // 004a6984  41                     -inc ecx
    (cpu.ecx)++;
L_0x004a6985:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a6985  42                     -inc edx
    (cpu.edx)++;
    // 004a6986  83fa08                 +cmp edx, 8
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
    // 004a6989  7d0f                   -jge 0x4a699a
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004a699a;
    }
L_0x004a698b:
    // 004a698b  e860f1ffff             -call 0x4a5af0
    cpu.esp -= 4;
    sub_4a5af0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a6990  3a826d227a00           +cmp al, byte ptr [edx + 0x7a226d]
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(app->getMemory<x86::reg8>(cpu.edx + x86::reg32(8004205) /* 0x7a226d */)));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 004a6996  75aa                   -jne 0x4a6942
    if (!cpu.flags.zf)
    {
        goto L_0x004a6942;
    }
    // 004a6998  ebeb                   -jmp 0x4a6985
    goto L_0x004a6985;
L_0x004a699a:
    // 004a699a  833d10227a0000         +cmp dword ptr [0x7a2210], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8004112) /* 0x7a2210 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a69a1  740d                   -je 0x4a69b0
    if (cpu.flags.zf)
    {
        goto L_0x004a69b0;
    }
    // 004a69a3  8b15b4ec5500           -mov edx, dword ptr [0x55ecb4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5631156) /* 0x55ecb4 */);
    // 004a69a9  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004a69aa  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004a69b0:
    // 004a69b0  c70510227a0001000000   -mov dword ptr [0x7a2210], 1
    app->getMemory<x86::reg32>(x86::reg32(8004112) /* 0x7a2210 */) = 1 /*0x1*/;
    // 004a69ba  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a69bb  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a69bc  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a69bd  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a69be  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x90 */
void Application::sub_4a69c0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004a69c0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a69c1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a69c2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004a69c3  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a69c4  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a69c6  66833d96227a0000       +cmp word ptr [0x7a2296], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(x86::reg32(8004246) /* 0x7a2296 */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004a69ce  7507                   -jne 0x4a69d7
    if (!cpu.flags.zf)
    {
        goto L_0x004a69d7;
    }
    // 004a69d0  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004a69d2  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a69d3  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a69d4  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a69d5  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a69d6  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a69d7:
    // 004a69d7  8b15b4ec5500           -mov edx, dword ptr [0x55ecb4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5631156) /* 0x55ecb4 */);
    // 004a69dd  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004a69de  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a69e4  8b0db4ec5500           -mov ecx, dword ptr [0x55ecb4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5631156) /* 0x55ecb4 */);
    // 004a69ea  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004a69ec  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a69ed  66891d96227a00         -mov word ptr [0x7a2296], bx
    app->getMemory<x86::reg16>(x86::reg32(8004246) /* 0x7a2296 */) = cpu.bx;
    // 004a69f4  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a69fa  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004a69ff  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6a00  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6a01  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6a02  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6a03  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_4a6a10(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004a6a10  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a6a11  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a6a12  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004a6a13  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004a6a14  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a6a15  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a6a17  83ec08                 -sub esp, 8
    (cpu.esp) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 004a6a1a  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 004a6a1d  833de0227a0000         +cmp dword ptr [0x7a22e0], 0
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
    // 004a6a24  0f840e020000           -je 0x4a6c38
    if (cpu.flags.zf)
    {
        goto L_0x004a6c38;
    }
    // 004a6a2a  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 004a6a2c  668b15dc227a00         -mov dx, word ptr [0x7a22dc]
    cpu.dx = app->getMemory<x86::reg16>(x86::reg32(8004316) /* 0x7a22dc */);
    // 004a6a33  c1e707                 -shl edi, 7
    cpu.edi <<= 7 /*0x7*/ % 32;
    // 004a6a36  6685d2                 +test dx, dx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg16>(cpu.dx & cpu.dx));
    // 004a6a39  0f85f9010000           -jne 0x4a6c38
    if (!cpu.flags.zf)
    {
        goto L_0x004a6c38;
    }
    // 004a6a3f  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 004a6a41  7407                   -je 0x4a6a4a
    if (cpu.flags.zf)
    {
        goto L_0x004a6a4a;
    }
    // 004a6a43  a1a4c17900             -mov eax, dword ptr [0x79c1a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7979428) /* 0x79c1a4 */);
    // 004a6a48  01c7                   -add edi, eax
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.eax));
L_0x004a6a4a:
    // 004a6a4a  c645f804               -mov byte ptr [ebp - 8], 4
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = 4 /*0x4*/;
    // 004a6a4e  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004a6a51  668b1dda227a00         -mov bx, word ptr [0x7a22da]
    cpu.bx = app->getMemory<x86::reg16>(x86::reg32(8004314) /* 0x7a22da */);
    // 004a6a58  668945fa               -mov word ptr [ebp - 6], ax
    app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-6) /* -0x6 */) = cpu.ax;
    // 004a6a5c  6685db                 +test bx, bx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg16>(cpu.bx & cpu.bx));
    // 004a6a5f  0f8422010000           -je 0x4a6b87
    if (cpu.flags.zf)
    {
        goto L_0x004a6b87;
    }
    // 004a6a65  bb04000000             -mov ebx, 4
    cpu.ebx = 4 /*0x4*/;
    // 004a6a6a  8d55f8                 -lea edx, [ebp - 8]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004a6a6d  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004a6a6f  be01000000             -mov esi, 1
    cpu.esi = 1 /*0x1*/;
    // 004a6a74  e867f6ffff             -call 0x4a60e0
    cpu.esp -= 4;
    nfs3hp::ea::sendPacket(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004a6a79:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a6a79  a174227a00             -mov eax, dword ptr [0x7a2274]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8004212) /* 0x7a2274 */);
    // 004a6a7e  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 004a6a81  39c6                   +cmp esi, eax
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
    // 004a6a83  0f8daf010000           -jge 0x4a6c38
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004a6c38;
    }
    // 004a6a89  8b04b5f0217a00         -mov eax, dword ptr [esi*4 + 0x7a21f0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8004080) /* 0x7a21f0 */ + cpu.esi * 4);
    // 004a6a90  83782400               +cmp dword ptr [eax + 0x24], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(36) /* 0x24 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a6a94  740b                   -je 0x4a6aa1
    if (cpu.flags.zf)
    {
        goto L_0x004a6aa1;
    }
    // 004a6a96  66833c75ca227a0000     +cmp word ptr [esi*2 + 0x7a22ca], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(x86::reg32(8004298) /* 0x7a22ca */ + cpu.esi * 2);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004a6a9f  740f                   -je 0x4a6ab0
    if (cpu.flags.zf)
    {
        goto L_0x004a6ab0;
    }
L_0x004a6aa1:
    // 004a6aa1  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004a6aa3  66890475ba227a00       -mov word ptr [esi*2 + 0x7a22ba], ax
    app->getMemory<x86::reg16>(x86::reg32(8004282) /* 0x7a22ba */ + cpu.esi * 2) = cpu.ax;
    // 004a6aab  e9c4000000             -jmp 0x4a6b74
    goto L_0x004a6b74;
L_0x004a6ab0:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a6ab0  8d1436                 -lea edx, [esi + esi]
    cpu.edx = x86::reg32(cpu.esi + cpu.esi * 1);
    // 004a6ab3  8b82b8227a00           -mov eax, dword ptr [edx + 0x7a22b8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(8004280) /* 0x7a22b8 */);
    // 004a6ab9  8b5dfc                 -mov ebx, dword ptr [ebp - 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004a6abc  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 004a6abf  39d8                   +cmp eax, ebx
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
    // 004a6ac1  0f8478000000           -je 0x4a6b3f
    if (cpu.flags.zf)
    {
        goto L_0x004a6b3f;
    }
    // 004a6ac7  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004a6ac9  e8620b0400             -call 0x4e7630
    cpu.esp -= 4;
    nfs3hp::port::runSystemTasks(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a6ace  66833ddc227a0000       +cmp word ptr [0x7a22dc], 0
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
    // 004a6ad6  740e                   -je 0x4a6ae6
    if (cpu.flags.zf)
    {
        goto L_0x004a6ae6;
    }
    // 004a6ad8  31c9                   +xor ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx))));
    // 004a6ada  66898aba227a00         -mov word ptr [edx + 0x7a22ba], cx
    app->getMemory<x86::reg16>(cpu.edx + x86::reg32(8004282) /* 0x7a22ba */) = cpu.cx;
    // 004a6ae1  e952010000             -jmp 0x4a6c38
    goto L_0x004a6c38;
L_0x004a6ae6:
    // 004a6ae6  8d0cb500000000         -lea ecx, [esi*4]
    cpu.ecx = x86::reg32(cpu.esi * 4);
    // 004a6aed  8b81f0217a00           -mov eax, dword ptr [ecx + 0x7a21f0]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(8004080) /* 0x7a21f0 */);
    // 004a6af3  83782400               +cmp dword ptr [eax + 0x24], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(36) /* 0x24 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a6af7  750b                   -jne 0x4a6b04
    if (!cpu.flags.zf)
    {
        goto L_0x004a6b04;
    }
    // 004a6af9  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004a6afb  668982ba227a00         -mov word ptr [edx + 0x7a22ba], ax
    app->getMemory<x86::reg16>(cpu.edx + x86::reg32(8004282) /* 0x7a22ba */) = cpu.ax;
    // 004a6b02  eb34                   -jmp 0x4a6b38
    goto L_0x004a6b38;
L_0x004a6b04:
    // 004a6b04  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 004a6b06  74a8                   -je 0x4a6ab0
    if (cpu.flags.zf)
    {
        goto L_0x004a6ab0;
    }
    // 004a6b08  a1a4c17900             -mov eax, dword ptr [0x79c1a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7979428) /* 0x79c1a4 */);
    // 004a6b0d  39f8                   +cmp eax, edi
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
    // 004a6b0f  7f02                   -jg 0x4a6b13
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x004a6b13;
    }
    // 004a6b11  eb9d                   -jmp 0x4a6ab0
    goto L_0x004a6ab0;
L_0x004a6b13:
    // 004a6b13  b80c000000             -mov eax, 0xc
    cpu.eax = 12 /*0xc*/;
    // 004a6b18  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004a6b1a  668945fa               -mov word ptr [ebp - 6], ax
    app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-6) /* -0x6 */) = cpu.ax;
    // 004a6b1e  66899aba227a00         -mov word ptr [edx + 0x7a22ba], bx
    app->getMemory<x86::reg16>(cpu.edx + x86::reg32(8004282) /* 0x7a22ba */) = cpu.bx;
    // 004a6b25  bb04000000             -mov ebx, 4
    cpu.ebx = 4 /*0x4*/;
    // 004a6b2a  8d55f8                 -lea edx, [ebp - 8]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004a6b2d  8b81f0217a00           -mov eax, dword ptr [ecx + 0x7a21f0]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(8004080) /* 0x7a21f0 */);
    // 004a6b33  e8a8f5ffff             -call 0x4a60e0
    cpu.esp -= 4;
    nfs3hp::ea::sendPacket(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004a6b38:
    // 004a6b38  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004a6b3a  e8b181ffff             -call 0x49ecf0
    cpu.esp -= 4;
    nfs3hp::ea::announceMachineLeft(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004a6b3f:
    // 004a6b3f  a1b8ec5500             -mov eax, dword ptr [0x55ecb8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5631160) /* 0x55ecb8 */);
    // 004a6b44  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a6b45  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a6b4b  8d1436                 -lea edx, [esi + esi]
    cpu.edx = x86::reg32(cpu.esi + cpu.esi * 1);
    // 004a6b4e  8b82b8227a00           -mov eax, dword ptr [edx + 0x7a22b8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(8004280) /* 0x7a22b8 */);
    // 004a6b54  8b4dfc                 -mov ecx, dword ptr [ebp - 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004a6b57  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 004a6b5a  39c8                   +cmp eax, ecx
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
    // 004a6b5c  7509                   -jne 0x4a6b67
    if (!cpu.flags.zf)
    {
        goto L_0x004a6b67;
    }
    // 004a6b5e  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 004a6b60  66899aba227a00         -mov word ptr [edx + 0x7a22ba], bx
    app->getMemory<x86::reg16>(cpu.edx + x86::reg32(8004282) /* 0x7a22ba */) = cpu.bx;
L_0x004a6b67:
    // 004a6b67  8b1db8ec5500           -mov ebx, dword ptr [0x55ecb8]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5631160) /* 0x55ecb8 */);
    // 004a6b6d  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a6b6e  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004a6b74:
    // 004a6b74  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004a6b75  e9fffeffff             -jmp 0x4a6a79
    goto L_0x004a6a79;
L_0x004a6b7a:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a6b7a  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 004a6b7c  7409                   -je 0x4a6b87
    if (cpu.flags.zf)
    {
        goto L_0x004a6b87;
    }
    // 004a6b7e  a1a4c17900             -mov eax, dword ptr [0x79c1a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7979428) /* 0x79c1a4 */);
    // 004a6b83  39f8                   +cmp eax, edi
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
    // 004a6b85  7f4e                   -jg 0x4a6bd5
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x004a6bd5;
    }
L_0x004a6b87:
    // 004a6b87  a19a227a00             -mov eax, dword ptr [0x7a229a]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8004250) /* 0x7a229a */);
    // 004a6b8c  8b75fc                 -mov esi, dword ptr [ebp - 4]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004a6b8f  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 004a6b92  39f0                   +cmp eax, esi
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
    // 004a6b94  7461                   -je 0x4a6bf7
    if (cpu.flags.zf)
    {
        goto L_0x004a6bf7;
    }
    // 004a6b96  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004a6b98  e8930a0400             -call 0x4e7630
    cpu.esp -= 4;
    nfs3hp::port::runSystemTasks(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a6b9d  668b0ddc227a00         -mov cx, word ptr [0x7a22dc]
    cpu.cx = app->getMemory<x86::reg16>(x86::reg32(8004316) /* 0x7a22dc */);
    // 004a6ba4  6685c9                 +test cx, cx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg16>(cpu.cx & cpu.cx));
    // 004a6ba7  7509                   -jne 0x4a6bb2
    if (!cpu.flags.zf)
    {
        goto L_0x004a6bb2;
    }
    // 004a6ba9  833d0c237a0000         +cmp dword ptr [0x7a230c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8004364) /* 0x7a230c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a6bb0  75c8                   -jne 0x4a6b7a
    if (!cpu.flags.zf)
    {
        goto L_0x004a6b7a;
    }
L_0x004a6bb2:
    // 004a6bb2  be01000000             -mov esi, 1
    cpu.esi = 1 /*0x1*/;
    // 004a6bb7  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004a6bb9  668935de227a00         -mov word ptr [0x7a22de], si
    app->getMemory<x86::reg16>(x86::reg32(8004318) /* 0x7a22de */) = cpu.si;
    // 004a6bc0  66a39c227a00           -mov word ptr [0x7a229c], ax
    app->getMemory<x86::reg16>(x86::reg32(8004252) /* 0x7a229c */) = cpu.ax;
    // 004a6bc6  668935dc227a00         -mov word ptr [0x7a22dc], si
    app->getMemory<x86::reg16>(x86::reg32(8004316) /* 0x7a22dc */) = cpu.si;
    // 004a6bcd  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004a6bcf  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6bd0  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6bd1  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6bd2  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6bd3  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6bd4  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a6bd5:
    // 004a6bd5  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 004a6bda  66890d9c227a00         -mov word ptr [0x7a229c], cx
    app->getMemory<x86::reg16>(x86::reg32(8004252) /* 0x7a229c */) = cpu.cx;
    // 004a6be1  668915de227a00         -mov word ptr [0x7a22de], dx
    app->getMemory<x86::reg16>(x86::reg32(8004318) /* 0x7a22de */) = cpu.dx;
    // 004a6be8  668915dc227a00         -mov word ptr [0x7a22dc], dx
    app->getMemory<x86::reg16>(x86::reg32(8004316) /* 0x7a22dc */) = cpu.dx;
    // 004a6bef  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004a6bf1  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6bf2  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6bf3  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6bf4  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6bf5  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6bf6  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a6bf7:
    // 004a6bf7  8b15b8ec5500           -mov edx, dword ptr [0x55ecb8]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5631160) /* 0x55ecb8 */);
    // 004a6bfd  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004a6bfe  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a6c04  a19a227a00             -mov eax, dword ptr [0x7a229a]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8004250) /* 0x7a229a */);
    // 004a6c09  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 004a6c0c  39f0                   +cmp eax, esi
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
    // 004a6c0e  7509                   -jne 0x4a6c19
    if (!cpu.flags.zf)
    {
        goto L_0x004a6c19;
    }
    // 004a6c10  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 004a6c12  6689359c227a00         -mov word ptr [0x7a229c], si
    app->getMemory<x86::reg16>(x86::reg32(8004252) /* 0x7a229c */) = cpu.si;
L_0x004a6c19:
    // 004a6c19  8b1db8ec5500           -mov ebx, dword ptr [0x55ecb8]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5631160) /* 0x55ecb8 */);
    // 004a6c1f  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a6c20  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a6c26  bb04000000             -mov ebx, 4
    cpu.ebx = 4 /*0x4*/;
    // 004a6c2b  8d55f8                 -lea edx, [ebp - 8]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004a6c2e  b8e8227a00             -mov eax, 0x7a22e8
    cpu.eax = 8004328 /*0x7a22e8*/;
    // 004a6c33  e8a8f4ffff             -call 0x4a60e0
    cpu.esp -= 4;
    nfs3hp::ea::sendPacket(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004a6c38:
    // 004a6c38  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004a6c3a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6c3b  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6c3c  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6c3d  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6c3e  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6c3f  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_4a6c40(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004a6c40  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a6c41  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004a6c42  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004a6c43  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a6c44  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a6c46  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 004a6c48  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 004a6c4a  66833dda227a0000       +cmp word ptr [0x7a22da], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(x86::reg32(8004314) /* 0x7a22da */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004a6c52  7439                   -je 0x4a6c8d
    if (cpu.flags.zf)
    {
        goto L_0x004a6c8d;
    }
    // 004a6c54  8b15b8ec5500           -mov edx, dword ptr [0x55ecb8]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5631160) /* 0x55ecb8 */);
    // 004a6c5a  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004a6c5b  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a6c61  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 004a6c63  e8e8f9ffff             -call 0x4a6650
    cpu.esp -= 4;
    nfs3hp::ea::machineNumber(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a6c68  98                     -cwde 
    cpu.eax = x86::reg32(static_cast<x86::sreg16>(cpu.ax));
    // 004a6c69  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a6c6b  7d09                   -jge 0x4a6c76
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004a6c76;
    }
    // 004a6c6d  8b0db8ec5500           -mov ecx, dword ptr [0x55ecb8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5631160) /* 0x55ecb8 */);
    // 004a6c73  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a6c74  eb5d                   -jmp 0x4a6cd3
    goto L_0x004a6cd3;
L_0x004a6c76:
    // 004a6c76  8d1400                 -lea edx, [eax + eax]
    cpu.edx = x86::reg32(cpu.eax + cpu.eax * 1);
    // 004a6c79  8b1db8ec5500           -mov ebx, dword ptr [0x55ecb8]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5631160) /* 0x55ecb8 */);
    // 004a6c7f  668b4602               -mov ax, word ptr [esi + 2]
    cpu.ax = app->getMemory<x86::reg16>(cpu.esi + x86::reg32(2) /* 0x2 */);
    // 004a6c83  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a6c84  668982ba227a00         -mov word ptr [edx + 0x7a22ba], ax
    app->getMemory<x86::reg16>(cpu.edx + x86::reg32(8004282) /* 0x7a22ba */) = cpu.ax;
    // 004a6c8b  eb46                   -jmp 0x4a6cd3
    goto L_0x004a6cd3;
L_0x004a6c8d:
    // 004a6c8d  66837e020c             +cmp word ptr [esi + 2], 0xc
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.esi + x86::reg32(2) /* 0x2 */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(12 /*0xc*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004a6c92  7522                   -jne 0x4a6cb6
    if (!cpu.flags.zf)
    {
        goto L_0x004a6cb6;
    }
    // 004a6c94  66833ddc227a0000       +cmp word ptr [0x7a22dc], 0
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
    // 004a6c9c  753b                   -jne 0x4a6cd9
    if (!cpu.flags.zf)
    {
        goto L_0x004a6cd9;
    }
    // 004a6c9e  be01000000             -mov esi, 1
    cpu.esi = 1 /*0x1*/;
    // 004a6ca3  668935de227a00         -mov word ptr [0x7a22de], si
    app->getMemory<x86::reg16>(x86::reg32(8004318) /* 0x7a22de */) = cpu.si;
    // 004a6caa  668935dc227a00         -mov word ptr [0x7a22dc], si
    app->getMemory<x86::reg16>(x86::reg32(8004316) /* 0x7a22dc */) = cpu.si;
    // 004a6cb1  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6cb2  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6cb3  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6cb4  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6cb5  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a6cb6:
    // 004a6cb6  8b3db8ec5500           -mov edi, dword ptr [0x55ecb8]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(5631160) /* 0x55ecb8 */);
    // 004a6cbc  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004a6cbd  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a6cc3  668b4602               -mov ax, word ptr [esi + 2]
    cpu.ax = app->getMemory<x86::reg16>(cpu.esi + x86::reg32(2) /* 0x2 */);
    // 004a6cc7  66a39c227a00           -mov word ptr [0x7a229c], ax
    app->getMemory<x86::reg16>(x86::reg32(8004252) /* 0x7a229c */) = cpu.ax;
    // 004a6ccd  a1b8ec5500             -mov eax, dword ptr [0x55ecb8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5631160) /* 0x55ecb8 */);
    // 004a6cd2  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
L_0x004a6cd3:
    // 004a6cd3  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004a6cd9:
    // 004a6cd9  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6cda  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6cdb  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6cdc  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6cdd  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_4a6ce0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004a6ce0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a6ce1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a6ce2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004a6ce3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004a6ce4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004a6ce5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a6ce6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a6ce8  83ec2c                 -sub esp, 0x2c
    (cpu.esp) -= x86::reg32(x86::sreg32(44 /*0x2c*/));
    // 004a6ceb  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004a6ced  be98e85500             -mov esi, 0x55e898
    cpu.esi = 5630104 /*0x55e898*/;
    // 004a6cf2  30e4                   -xor ah, ah
    cpu.ah ^= x86::reg8(x86::sreg8(cpu.ah));
    // 004a6cf4  8d7dd5                 -lea edi, [ebp - 0x2b]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-43) /* -0x2b */);
    // 004a6cf7  8865d4                 -mov byte ptr [ebp - 0x2c], ah
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-44) /* -0x2c */) = cpu.ah;
    // 004a6cfa  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
L_0x004a6cfb:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a6cfb  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 004a6cfd  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 004a6cff  3c00                   +cmp al, 0
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
    // 004a6d01  7410                   -je 0x4a6d13
    if (cpu.flags.zf)
    {
        goto L_0x004a6d13;
    }
    // 004a6d03  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 004a6d06  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004a6d09  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 004a6d0c  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004a6d0f  3c00                   +cmp al, 0
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
    // 004a6d11  75e8                   -jne 0x4a6cfb
    if (!cpu.flags.zf)
    {
        goto L_0x004a6cfb;
    }
L_0x004a6d13:
    // 004a6d13  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6d14  66833dda227a0000       +cmp word ptr [0x7a22da], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(x86::reg32(8004314) /* 0x7a22da */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004a6d1c  7419                   -je 0x4a6d37
    if (cpu.flags.zf)
    {
        goto L_0x004a6d37;
    }
    // 004a6d1e  bb29000000             -mov ebx, 0x29
    cpu.ebx = 41 /*0x29*/;
    // 004a6d23  8d55d4                 -lea edx, [ebp - 0x2c]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-44) /* -0x2c */);
    // 004a6d26  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004a6d28  e8b3f3ffff             -call 0x4a60e0
    cpu.esp -= 4;
    nfs3hp::ea::sendPacket(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a6d2d  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004a6d32  e9af000000             -jmp 0x4a6de6
    goto L_0x004a6de6;
L_0x004a6d37:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a6d37  803d08ed550000         +cmp byte ptr [0x55ed08], 0
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(x86::reg32(5631240) /* 0x55ed08 */);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 004a6d3e  7530                   -jne 0x4a6d70
    if (!cpu.flags.zf)
    {
        goto L_0x004a6d70;
    }
    // 004a6d40  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004a6d42  e8e9080400             -call 0x4e7630
    cpu.esp -= 4;
    nfs3hp::port::runSystemTasks(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a6d47  803d1948560000         +cmp byte ptr [0x564819], 0
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(x86::reg32(5654553) /* 0x564819 */);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 004a6d4e  740b                   -je 0x4a6d5b
    if (cpu.flags.zf)
    {
        goto L_0x004a6d5b;
    }
    // 004a6d50  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004a6d52  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004a6d54  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6d55  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6d56  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6d57  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6d58  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6d59  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6d5a  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a6d5b:
    // 004a6d5b  66833ddc227a0000       +cmp word ptr [0x7a22dc], 0
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
    // 004a6d63  74d2                   -je 0x4a6d37
    if (cpu.flags.zf)
    {
        goto L_0x004a6d37;
    }
    // 004a6d65  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004a6d67  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004a6d69  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6d6a  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6d6b  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6d6c  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6d6d  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6d6e  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6d6f  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a6d70:
    // 004a6d70  8d55d5                 -lea edx, [ebp - 0x2b]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-43) /* -0x2b */);
    // 004a6d73  b808ed5500             -mov eax, 0x55ed08
    cpu.eax = 5631240 /*0x55ed08*/;
    // 004a6d78  e893750400             -call 0x4ee310
    cpu.esp -= 4;
    nfs3hp::watcom::strcmp(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a6d7d  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a6d7f  7458                   -je 0x4a6dd9
    if (cpu.flags.zf)
    {
        goto L_0x004a6dd9;
    }
    // 004a6d81  e85af8ffff             -call 0x4a65e0
    cpu.esp -= 4;
    nfs3hp::ea::closeNetwork(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a6d86  8d45d5                 -lea eax, [ebp - 0x2b]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-43) /* -0x2b */);
    // 004a6d89  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a6d8a  6808ed5500             -push 0x55ed08
    app->getMemory<x86::reg32>(cpu.esp-4) = 5631240 /*0x55ed08*/;
    cpu.esp -= 4;
    // 004a6d8f  68d1040000             -push 0x4d1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1233 /*0x4d1*/;
    cpu.esp -= 4;
    // 004a6d94  ba5c000000             -mov edx, 0x5c
    cpu.edx = 92 /*0x5c*/;
    // 004a6d99  b8dcdb5300             -mov eax, 0x53dbdc
    cpu.eax = 5495772 /*0x53dbdc*/;
    // 004a6d9e  e84d750400             -call 0x4ee2f0
    cpu.esp -= 4;
    nfs3hp::watcom::strrchr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a6da3  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a6da5  7412                   -je 0x4a6db9
    if (cpu.flags.zf)
    {
        goto L_0x004a6db9;
    }
    // 004a6da7  ba5c000000             -mov edx, 0x5c
    cpu.edx = 92 /*0x5c*/;
    // 004a6dac  b8dcdb5300             -mov eax, 0x53dbdc
    cpu.eax = 5495772 /*0x53dbdc*/;
    // 004a6db1  e83a750400             -call 0x4ee2f0
    cpu.esp -= 4;
    nfs3hp::watcom::strrchr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a6db6  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004a6db7  eb05                   -jmp 0x4a6dbe
    goto L_0x004a6dbe;
L_0x004a6db9:
    // 004a6db9  b8dcdb5300             -mov eax, 0x53dbdc
    cpu.eax = 5495772 /*0x53dbdc*/;
L_0x004a6dbe:
    // 004a6dbe  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a6dbf  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 004a6dc1  68fcdb5300             -push 0x53dbfc
    app->getMemory<x86::reg32>(cpu.esp-4) = 5495804 /*0x53dbfc*/;
    cpu.esp -= 4;
    // 004a6dc6  e845a2f5ff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a6dcb  83c418                 -add esp, 0x18
    (cpu.esp) += x86::reg32(x86::sreg32(24 /*0x18*/));
    // 004a6dce  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004a6dd0  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004a6dd2  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6dd3  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6dd4  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6dd5  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6dd6  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6dd7  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6dd8  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004a6dd9:
    // 004a6dd9  30db                   -xor bl, bl
    cpu.bl ^= x86::reg8(x86::sreg8(cpu.bl));
    // 004a6ddb  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004a6de0  881d08ed5500           -mov byte ptr [0x55ed08], bl
    app->getMemory<x86::reg8>(x86::reg32(5631240) /* 0x55ed08 */) = cpu.bl;
L_0x004a6de6:
    // 004a6de6  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004a6de8  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6de9  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6dea  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6deb  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6dec  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6ded  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6dee  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x90 */
void Application::sub_4a6df0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004a6df0  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a6df1  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a6df3  8d4201                 -lea eax, [edx + 1]
    cpu.eax = x86::reg32(cpu.edx + x86::reg32(1) /* 0x1 */);
    // 004a6df6  bb28000000             -mov ebx, 0x28
    cpu.ebx = 40 /*0x28*/;
    // 004a6dfb  ba08ed5500             -mov edx, 0x55ed08
    cpu.edx = 5631240 /*0x55ed08*/;
    // 004a6e00  e8eb360400             -call 0x4ea4f0
    cpu.esp -= 4;
    nfs3hp::ea::memmove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a6e05  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6e06  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_4a6e10(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004a6e10  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a6e11  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a6e12  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004a6e13  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004a6e14  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004a6e15  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a6e16  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a6e18  81ec88000000           -sub esp, 0x88
    (cpu.esp) -= x86::reg32(x86::sreg32(136 /*0x88*/));
    // 004a6e1e  81ed82000000           -sub ebp, 0x82
    (cpu.ebp) -= x86::reg32(x86::sreg32(130 /*0x82*/));
    // 004a6e24  bb00140000             -mov ebx, 0x1400
    cpu.ebx = 5120 /*0x1400*/;
    // 004a6e29  b834267a00             -mov eax, 0x7a2634
    cpu.eax = 8005172 /*0x7a2634*/;
    // 004a6e2e  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004a6e30  e80b980300             -call 0x4e0640
    cpu.esp -= 4;
    nfs3hp::watcom::memset(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a6e35  ba3cdc5300             -mov edx, 0x53dc3c
    cpu.edx = 5495868 /*0x53dc3c*/;
    // 004a6e3a  b840dc5300             -mov eax, 0x53dc40
    cpu.eax = 5495872 /*0x53dc40*/;
    // 004a6e3f  e8a4710400             -call 0x4edfe8
    cpu.esp -= 4;
    nfs3hp::watcom::fopen(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a6e44  89457a                 -mov dword ptr [ebp + 0x7a], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */) = cpu.eax;
    // 004a6e47  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a6e49  7507                   -jne 0x4a6e52
    if (!cpu.flags.zf)
    {
        goto L_0x004a6e52;
    }
    // 004a6e4b  31f6                   +xor esi, esi
    cpu.clear_co();
    cpu.set_szp((cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi))));
    // 004a6e4d  e900010000             -jmp 0x4a6f52
    goto L_0x004a6f52;
L_0x004a6e52:
    // 004a6e52  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004a6e54  89557e                 -mov dword ptr [ebp + 0x7e], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.edx;
L_0x004a6e57:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a6e57  8b457a                 -mov eax, dword ptr [ebp + 0x7a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 004a6e5a  f6400c10               +test byte ptr [eax + 0xc], 0x10
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(12) /* 0xc */) & 16 /*0x10*/));
    // 004a6e5e  0f85a0000000           -jne 0x4a6f04
    if (!cpu.flags.zf)
    {
        goto L_0x004a6f04;
    }
    // 004a6e64  ba7f000000             -mov edx, 0x7f
    cpu.edx = 127 /*0x7f*/;
    // 004a6e69  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 004a6e6b  8d45fa                 -lea eax, [ebp - 6]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-6) /* -0x6 */);
    // 004a6e6e  8d75fa                 -lea esi, [ebp - 6]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-6) /* -0x6 */);
    // 004a6e71  e8fa960400             -call 0x4f0570
    cpu.esp -= 4;
    nfs3hp::watcom::fgets(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a6e76  b20a                   -mov dl, 0xa
    cpu.dl = 10 /*0xa*/;
L_0x004a6e78:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a6e78  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 004a6e7a  3ac2                   +cmp al, dl
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(cpu.dl));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 004a6e7c  7412                   -je 0x4a6e90
    if (cpu.flags.zf)
    {
        goto L_0x004a6e90;
    }
    // 004a6e7e  3c00                   +cmp al, 0
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
    // 004a6e80  740c                   -je 0x4a6e8e
    if (cpu.flags.zf)
    {
        goto L_0x004a6e8e;
    }
    // 004a6e82  46                     -inc esi
    (cpu.esi)++;
    // 004a6e83  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 004a6e85  3ac2                   +cmp al, dl
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(cpu.dl));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 004a6e87  7407                   -je 0x4a6e90
    if (cpu.flags.zf)
    {
        goto L_0x004a6e90;
    }
    // 004a6e89  46                     -inc esi
    (cpu.esi)++;
    // 004a6e8a  3c00                   +cmp al, 0
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
    // 004a6e8c  75ea                   -jne 0x4a6e78
    if (!cpu.flags.zf)
    {
        goto L_0x004a6e78;
    }
L_0x004a6e8e:
    // 004a6e8e  2bf6                   -sub esi, esi
    (cpu.esi) -= x86::reg32(x86::sreg32(cpu.esi));
L_0x004a6e90:
    // 004a6e90  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 004a6e92  7403                   -je 0x4a6e97
    if (cpu.flags.zf)
    {
        goto L_0x004a6e97;
    }
    // 004a6e94  c60600                 -mov byte ptr [esi], 0
    app->getMemory<x86::reg8>(cpu.esi) = 0 /*0x0*/;
L_0x004a6e97:
    // 004a6e97  b20d                   -mov dl, 0xd
    cpu.dl = 13 /*0xd*/;
    // 004a6e99  8d75fa                 -lea esi, [ebp - 6]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-6) /* -0x6 */);
L_0x004a6e9c:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a6e9c  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 004a6e9e  3ac2                   +cmp al, dl
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(cpu.dl));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 004a6ea0  7412                   -je 0x4a6eb4
    if (cpu.flags.zf)
    {
        goto L_0x004a6eb4;
    }
    // 004a6ea2  3c00                   +cmp al, 0
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
    // 004a6ea4  740c                   -je 0x4a6eb2
    if (cpu.flags.zf)
    {
        goto L_0x004a6eb2;
    }
    // 004a6ea6  46                     -inc esi
    (cpu.esi)++;
    // 004a6ea7  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 004a6ea9  3ac2                   +cmp al, dl
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(cpu.dl));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 004a6eab  7407                   -je 0x4a6eb4
    if (cpu.flags.zf)
    {
        goto L_0x004a6eb4;
    }
    // 004a6ead  46                     -inc esi
    (cpu.esi)++;
    // 004a6eae  3c00                   +cmp al, 0
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
    // 004a6eb0  75ea                   -jne 0x4a6e9c
    if (!cpu.flags.zf)
    {
        goto L_0x004a6e9c;
    }
L_0x004a6eb2:
    // 004a6eb2  2bf6                   -sub esi, esi
    (cpu.esi) -= x86::reg32(x86::sreg32(cpu.esi));
L_0x004a6eb4:
    // 004a6eb4  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 004a6eb6  7403                   -je 0x4a6ebb
    if (cpu.flags.zf)
    {
        goto L_0x004a6ebb;
    }
    // 004a6eb8  c60600                 -mov byte ptr [esi], 0
    app->getMemory<x86::reg8>(cpu.esi) = 0 /*0x0*/;
L_0x004a6ebb:
    // 004a6ebb  8d7dfa                 -lea edi, [ebp - 6]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-6) /* -0x6 */);
    // 004a6ebe  29c9                   -sub ecx, ecx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004a6ec0  49                     -dec ecx
    (cpu.ecx)--;
    // 004a6ec1  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004a6ec3  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 004a6ec5  f7d1                   -not ecx
    cpu.ecx = ~cpu.ecx;
    // 004a6ec7  49                     -dec ecx
    (cpu.ecx)--;
    // 004a6ec8  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004a6eca  7502                   -jne 0x4a6ece
    if (!cpu.flags.zf)
    {
        goto L_0x004a6ece;
    }
    // 004a6ecc  eb89                   -jmp 0x4a6e57
    goto L_0x004a6e57;
L_0x004a6ece:
    // 004a6ece  8b457e                 -mov eax, dword ptr [ebp + 0x7e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 004a6ed1  bf34267a00             -mov edi, 0x7a2634
    cpu.edi = 8005172 /*0x7a2634*/;
    // 004a6ed6  8d75fa                 -lea esi, [ebp - 6]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-6) /* -0x6 */);
    // 004a6ed9  c1e007                 -shl eax, 7
    cpu.eax <<= 7 /*0x7*/ % 32;
    // 004a6edc  8b4d7e                 -mov ecx, dword ptr [ebp + 0x7e]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 004a6edf  01c7                   -add edi, eax
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.eax));
    // 004a6ee1  41                     -inc ecx
    (cpu.ecx)++;
    // 004a6ee2  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
L_0x004a6ee3:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a6ee3  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 004a6ee5  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 004a6ee7  3c00                   +cmp al, 0
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
    // 004a6ee9  7410                   -je 0x4a6efb
    if (cpu.flags.zf)
    {
        goto L_0x004a6efb;
    }
    // 004a6eeb  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 004a6eee  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004a6ef1  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 004a6ef4  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004a6ef7  3c00                   +cmp al, 0
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
    // 004a6ef9  75e8                   -jne 0x4a6ee3
    if (!cpu.flags.zf)
    {
        goto L_0x004a6ee3;
    }
L_0x004a6efb:
    // 004a6efb  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6efc  894d7e                 -mov dword ptr [ebp + 0x7e], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.ecx;
    // 004a6eff  e953ffffff             -jmp 0x4a6e57
    goto L_0x004a6e57;
L_0x004a6f04:
    // 004a6f04  e8f7710400             -call 0x4ee100
    cpu.esp -= 4;
    nfs3hp::watcom::fclose(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a6f09  31f6                   +xor esi, esi
    cpu.clear_co();
    cpu.set_szp((cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi))));
    // 004a6f0b  eb06                   -jmp 0x4a6f13
    goto L_0x004a6f13;
L_0x004a6f0d:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004a6f0d  46                     -inc esi
    (cpu.esi)++;
    // 004a6f0e  83fe06                 +cmp esi, 6
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(6 /*0x6*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a6f11  7d28                   -jge 0x4a6f3b
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004a6f3b;
    }
L_0x004a6f13:
    // 004a6f13  8b3cb524fd5500         -mov edi, dword ptr [esi*4 + 0x55fd24]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(5635364) /* 0x55fd24 */ + cpu.esi * 4);
    // 004a6f1a  ba34267a00             -mov edx, 0x7a2634
    cpu.edx = 8005172 /*0x7a2634*/;
    // 004a6f1f  29c9                   -sub ecx, ecx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004a6f21  49                     -dec ecx
    (cpu.ecx)--;
    // 004a6f22  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004a6f24  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 004a6f26  f7d1                   -not ecx
    cpu.ecx = ~cpu.ecx;
    // 004a6f28  49                     -dec ecx
    (cpu.ecx)--;
    // 004a6f29  89cb                   -mov ebx, ecx
    cpu.ebx = cpu.ecx;
    // 004a6f2b  8b04b524fd5500         -mov eax, dword ptr [esi*4 + 0x55fd24]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5635364) /* 0x55fd24 */ + cpu.esi * 4);
    // 004a6f32  e859440400             -call 0x4eb390
    cpu.esp -= 4;
    nfs3hp::watcom::strncmp(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a6f37  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004a6f39  75d2                   -jne 0x4a6f0d
    if (!cpu.flags.zf)
    {
        goto L_0x004a6f0d;
    }
L_0x004a6f3b:
    // 004a6f3b  83fe06                 +cmp esi, 6
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(6 /*0x6*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004a6f3e  7507                   -jne 0x4a6f47
    if (!cpu.flags.zf)
    {
        goto L_0x004a6f47;
    }
    // 004a6f40  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004a6f42  e809ef0000             -call 0x4b5e50
    cpu.esp -= 4;
    sub_4b5e50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004a6f47:
    // 004a6f47  8935503a7a00           -mov dword ptr [0x7a3a50], esi
    app->getMemory<x86::reg32>(x86::reg32(8010320) /* 0x7a3a50 */) = cpu.esi;
    // 004a6f4d  be01000000             -mov esi, 1
    cpu.esi = 1 /*0x1*/;
L_0x004a6f52:
    // 004a6f52  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004a6f54  8da582000000           -lea esp, [ebp + 0x82]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(130) /* 0x82 */);
    // 004a6f5a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6f5b  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6f5c  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6f5d  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6f5e  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6f5f  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6f60  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_4a6f70(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004a6f70  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a6f71  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a6f73  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a6f74  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
/* data blob: 00000000000000000000000000ff7f3f0000000000000000000000000000000000000000000000000000000000ff7f3f0000000000000000000000000000000000000000000000000000000000ff7f3f0000000000000000000000000000000000000000000000000000000000ff7f3f0000000000000000000000000000000000000000000000000000000000ff7f3f0000000000000000000000000000000000000000000000000000000000ff7f3f0000000000000000000000000000000000000000000000000000000000ff7f3f0000000000000000000000000000000000000000000000000000000000ff7f3f00000000000000000000000000000000 */
void Application::sub_4a7080(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1, fp2;
    // 004a7080  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004a7081  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004a7082  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a7083  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a7085  81ec90000000           -sub esp, 0x90
    (cpu.esp) -= x86::reg32(x86::sreg32(144 /*0x90*/));
    // 004a708b  83ed6e                 -sub ebp, 0x6e
    (cpu.ebp) -= x86::reg32(x86::sreg32(110 /*0x6e*/));
    // 004a708e  89455e                 -mov dword ptr [ebp + 0x5e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(94) /* 0x5e */) = cpu.eax;
    // 004a7091  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004a7093  b920000000             -mov ecx, 0x20
    cpu.ecx = 32 /*0x20*/;
    // 004a7098  8d7dde                 -lea edi, [ebp - 0x22]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-34) /* -0x22 */);
    // 004a709b  be806f4a00             -mov esi, 0x4a6f80
    cpu.esi = 4878208 /*0x4a6f80*/;
    // 004a70a0  895562                 -mov dword ptr [ebp + 0x62], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(98) /* 0x62 */) = cpu.edx;
    // 004a70a3  895d66                 -mov dword ptr [ebp + 0x66], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(102) /* 0x66 */) = cpu.ebx;
    // 004a70a6  89456a                 -mov dword ptr [ebp + 0x6a], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(106) /* 0x6a */) = cpu.eax;
    // 004a70a9  db4562                 -fild dword ptr [ebp + 0x62]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(98) /* 0x62 */)));
    // 004a70ac  db4566                 -fild dword ptr [ebp + 0x66]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(102) /* 0x66 */)));
    // 004a70af  db456a                 -fild dword ptr [ebp + 0x6a]
    fp2 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(106) /* 0x6a */)));
    // 004a70b2  8b958e000000           -mov edx, dword ptr [ebp + 0x8e]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(142) /* 0x8e */);
    // 004a70b8  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004a70ba  8b457e                 -mov eax, dword ptr [ebp + 0x7e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 004a70bd  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 004a70bf  d95de2                 -fstp dword ptr [ebp - 0x1e]
    app->getMemory<float>(cpu.ebp + x86::reg32(-30) /* -0x1e */) = float(fp2);
    // 004a70c2  d95dfe                 -fstp dword ptr [ebp - 2]
    app->getMemory<float>(cpu.ebp + x86::reg32(-2) /* -0x2 */) = float(fp1);
    // 004a70c5  d95d02                 -fstp dword ptr [ebp + 2]
    app->getMemory<float>(cpu.ebp + x86::reg32(2) /* 0x2 */) = float(fp0);
    // 004a70c8  8b4d5e                 -mov ecx, dword ptr [ebp + 0x5e]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(94) /* 0x5e */);
    // 004a70cb  8d7df2                 -lea edi, [ebp - 0xe]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-14) /* -0xe */);
    // 004a70ce  8d75de                 -lea esi, [ebp - 0x22]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-34) /* -0x22 */);
    // 004a70d1  8955ee                 -mov dword ptr [ebp - 0x12], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-18) /* -0x12 */) = cpu.edx;
    // 004a70d4  89550e                 -mov dword ptr [ebp + 0xe], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(14) /* 0xe */) = cpu.edx;
    // 004a70d7  894562                 -mov dword ptr [ebp + 0x62], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(98) /* 0x62 */) = cpu.eax;
    // 004a70da  89552e                 -mov dword ptr [ebp + 0x2e], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(46) /* 0x2e */) = cpu.edx;
    // 004a70dd  89554e                 -mov dword ptr [ebp + 0x4e], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(78) /* 0x4e */) = cpu.edx;
    // 004a70e0  db4562                 -fild dword ptr [ebp + 0x62]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(98) /* 0x62 */)));
    // 004a70e3  8b8582000000           -mov eax, dword ptr [ebp + 0x82]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(130) /* 0x82 */);
    // 004a70e9  894d62                 -mov dword ptr [ebp + 0x62], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(98) /* 0x62 */) = cpu.ecx;
    // 004a70ec  d95d1e                 -fstp dword ptr [ebp + 0x1e]
    app->getMemory<float>(cpu.ebp + x86::reg32(30) /* 0x1e */) = float(fp0);
    // 004a70ef  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004a70f1  db4562                 -fild dword ptr [ebp + 0x62]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(98) /* 0x62 */)));
    // 004a70f4  89456a                 -mov dword ptr [ebp + 0x6a], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(106) /* 0x6a */) = cpu.eax;
    // 004a70f7  d95dde                 -fstp dword ptr [ebp - 0x22]
    app->getMemory<float>(cpu.ebp + x86::reg32(-34) /* -0x22 */) = float(fp0);
    // 004a70fa  db456a                 -fild dword ptr [ebp + 0x6a]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(106) /* 0x6a */)));
    // 004a70fd  8b8586000000           -mov eax, dword ptr [ebp + 0x86]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(134) /* 0x86 */);
    // 004a7103  d95d22                 -fstp dword ptr [ebp + 0x22]
    app->getMemory<float>(cpu.ebp + x86::reg32(34) /* 0x22 */) = float(fp0);
    // 004a7106  89456a                 -mov dword ptr [ebp + 0x6a], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(106) /* 0x6a */) = cpu.eax;
    // 004a7109  8b858a000000           -mov eax, dword ptr [ebp + 0x8a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(138) /* 0x8a */);
    // 004a710f  db456a                 -fild dword ptr [ebp + 0x6a]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(106) /* 0x6a */)));
    // 004a7112  894562                 -mov dword ptr [ebp + 0x62], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(98) /* 0x62 */) = cpu.eax;
    // 004a7115  d95d3e                 -fstp dword ptr [ebp + 0x3e]
    app->getMemory<float>(cpu.ebp + x86::reg32(62) /* 0x3e */) = float(fp0);
    // 004a7118  db4562                 -fild dword ptr [ebp + 0x62]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(98) /* 0x62 */)));
    // 004a711b  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004a7120  d95d42                 -fstp dword ptr [ebp + 0x42]
    app->getMemory<float>(cpu.ebp + x86::reg32(66) /* 0x42 */) = float(fp0);
    // 004a7123  e8d8a7f8ff             -call 0x431900
    cpu.esp -= 4;
    nfs3hp::graphics::setRenderState(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a7128  8b460c                 -mov eax, dword ptr [esi + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(12) /* 0xc */);
    // 004a712b  8b0df8347d00           -mov ecx, dword ptr [0x7d34f8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205560) /* 0x7d34f8 */);
    // 004a7131  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 004a7136  7558                   -jne 0x4a7190
    if (!cpu.flags.zf)
    {
        goto L_0x004a7190;
    }
    // 004a7138  39c8                   +cmp eax, ecx
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
    // 004a713a  7d54                   -jge 0x4a7190
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004a7190;
    }
    // 004a713c  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 004a713f  bb00000000             -mov ebx, 0
    cpu.ebx = 0 /*0x0*/;
    // 004a7144  8b0d04357d00           -mov ecx, dword ptr [0x7d3504]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205572) /* 0x7d3504 */);
    // 004a714a  8b1500357d00           -mov edx, dword ptr [0x7d3500]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205568) /* 0x7d3500 */);
    // 004a7150  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 004a7155  750d                   -jne 0x4a7164
    if (!cpu.flags.zf)
    {
        goto L_0x004a7164;
    }
    // 004a7157  39c8                   +cmp eax, ecx
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
    // 004a7159  7c09                   -jl 0x4a7164
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004a7164;
    }
    // 004a715b  39d0                   +cmp eax, edx
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
    // 004a715d  7e09                   -jle 0x4a7168
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004a7168;
    }
    // 004a715f  83cb04                 +or ebx, 4
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(4 /*0x4*/))));
    // 004a7162  eb04                   -jmp 0x4a7168
    goto L_0x004a7168;
L_0x004a7164:
    // 004a7164  83cb08                 -or ebx, 8
    cpu.ebx |= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 004a7167  90                     -nop 
    ;
L_0x004a7168:
    // 004a7168  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 004a716a  90                     -nop 
    ;
    // 004a716b  8b0dfc347d00           -mov ecx, dword ptr [0x7d34fc]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205564) /* 0x7d34fc */);
    // 004a7171  8b150c357d00           -mov edx, dword ptr [0x7d350c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205580) /* 0x7d350c */);
    // 004a7177  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 004a717c  750d                   -jne 0x4a718b
    if (!cpu.flags.zf)
    {
        goto L_0x004a718b;
    }
    // 004a717e  39c8                   +cmp eax, ecx
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
    // 004a7180  7c09                   -jl 0x4a718b
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004a718b;
    }
    // 004a7182  39d0                   +cmp eax, edx
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
    // 004a7184  7e0f                   -jle 0x4a7195
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004a7195;
    }
    // 004a7186  83cb02                 +or ebx, 2
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(2 /*0x2*/))));
    // 004a7189  eb0a                   -jmp 0x4a7195
    goto L_0x004a7195;
L_0x004a718b:
    // 004a718b  83cb01                 +or ebx, 1
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(1 /*0x1*/))));
    // 004a718e  eb05                   -jmp 0x4a7195
    goto L_0x004a7195;
L_0x004a7190:
    // 004a7190  bb10000000             -mov ebx, 0x10
    cpu.ebx = 16 /*0x10*/;
L_0x004a7195:
    // 004a7195  881f                   -mov byte ptr [edi], bl
    app->getMemory<x86::reg8>(cpu.edi) = cpu.bl;
    // 004a7197  8d7d12                 -lea edi, [ebp + 0x12]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(18) /* 0x12 */);
    // 004a719a  8d75fe                 -lea esi, [ebp - 2]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-2) /* -0x2 */);
    // 004a719d  8b460c                 -mov eax, dword ptr [esi + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(12) /* 0xc */);
    // 004a71a0  8b0df8347d00           -mov ecx, dword ptr [0x7d34f8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205560) /* 0x7d34f8 */);
    // 004a71a6  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 004a71ab  7558                   -jne 0x4a7205
    if (!cpu.flags.zf)
    {
        goto L_0x004a7205;
    }
    // 004a71ad  39c8                   +cmp eax, ecx
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
    // 004a71af  7d54                   -jge 0x4a7205
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004a7205;
    }
    // 004a71b1  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 004a71b4  bb00000000             -mov ebx, 0
    cpu.ebx = 0 /*0x0*/;
    // 004a71b9  8b0d04357d00           -mov ecx, dword ptr [0x7d3504]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205572) /* 0x7d3504 */);
    // 004a71bf  8b1500357d00           -mov edx, dword ptr [0x7d3500]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205568) /* 0x7d3500 */);
    // 004a71c5  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 004a71ca  750d                   -jne 0x4a71d9
    if (!cpu.flags.zf)
    {
        goto L_0x004a71d9;
    }
    // 004a71cc  39c8                   +cmp eax, ecx
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
    // 004a71ce  7c09                   -jl 0x4a71d9
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004a71d9;
    }
    // 004a71d0  39d0                   +cmp eax, edx
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
    // 004a71d2  7e09                   -jle 0x4a71dd
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004a71dd;
    }
    // 004a71d4  83cb04                 +or ebx, 4
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(4 /*0x4*/))));
    // 004a71d7  eb04                   -jmp 0x4a71dd
    goto L_0x004a71dd;
L_0x004a71d9:
    // 004a71d9  83cb08                 -or ebx, 8
    cpu.ebx |= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 004a71dc  90                     -nop 
    ;
L_0x004a71dd:
    // 004a71dd  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 004a71df  90                     -nop 
    ;
    // 004a71e0  8b0dfc347d00           -mov ecx, dword ptr [0x7d34fc]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205564) /* 0x7d34fc */);
    // 004a71e6  8b150c357d00           -mov edx, dword ptr [0x7d350c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205580) /* 0x7d350c */);
    // 004a71ec  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 004a71f1  750d                   -jne 0x4a7200
    if (!cpu.flags.zf)
    {
        goto L_0x004a7200;
    }
    // 004a71f3  39c8                   +cmp eax, ecx
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
    // 004a71f5  7c09                   -jl 0x4a7200
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004a7200;
    }
    // 004a71f7  39d0                   +cmp eax, edx
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
    // 004a71f9  7e0f                   -jle 0x4a720a
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004a720a;
    }
    // 004a71fb  83cb02                 +or ebx, 2
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(2 /*0x2*/))));
    // 004a71fe  eb0a                   -jmp 0x4a720a
    goto L_0x004a720a;
L_0x004a7200:
    // 004a7200  83cb01                 +or ebx, 1
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(1 /*0x1*/))));
    // 004a7203  eb05                   -jmp 0x4a720a
    goto L_0x004a720a;
L_0x004a7205:
    // 004a7205  bb10000000             -mov ebx, 0x10
    cpu.ebx = 16 /*0x10*/;
L_0x004a720a:
    // 004a720a  881f                   -mov byte ptr [edi], bl
    app->getMemory<x86::reg8>(cpu.edi) = cpu.bl;
    // 004a720c  8d7d32                 -lea edi, [ebp + 0x32]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(50) /* 0x32 */);
    // 004a720f  8d751e                 -lea esi, [ebp + 0x1e]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(30) /* 0x1e */);
    // 004a7212  8b460c                 -mov eax, dword ptr [esi + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(12) /* 0xc */);
    // 004a7215  8b0df8347d00           -mov ecx, dword ptr [0x7d34f8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205560) /* 0x7d34f8 */);
    // 004a721b  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 004a7220  7558                   -jne 0x4a727a
    if (!cpu.flags.zf)
    {
        goto L_0x004a727a;
    }
    // 004a7222  39c8                   +cmp eax, ecx
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
    // 004a7224  7d54                   -jge 0x4a727a
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004a727a;
    }
    // 004a7226  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 004a7229  bb00000000             -mov ebx, 0
    cpu.ebx = 0 /*0x0*/;
    // 004a722e  8b0d04357d00           -mov ecx, dword ptr [0x7d3504]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205572) /* 0x7d3504 */);
    // 004a7234  8b1500357d00           -mov edx, dword ptr [0x7d3500]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205568) /* 0x7d3500 */);
    // 004a723a  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 004a723f  750d                   -jne 0x4a724e
    if (!cpu.flags.zf)
    {
        goto L_0x004a724e;
    }
    // 004a7241  39c8                   +cmp eax, ecx
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
    // 004a7243  7c09                   -jl 0x4a724e
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004a724e;
    }
    // 004a7245  39d0                   +cmp eax, edx
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
    // 004a7247  7e09                   -jle 0x4a7252
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004a7252;
    }
    // 004a7249  83cb04                 +or ebx, 4
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(4 /*0x4*/))));
    // 004a724c  eb04                   -jmp 0x4a7252
    goto L_0x004a7252;
L_0x004a724e:
    // 004a724e  83cb08                 -or ebx, 8
    cpu.ebx |= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 004a7251  90                     -nop 
    ;
L_0x004a7252:
    // 004a7252  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 004a7254  90                     -nop 
    ;
    // 004a7255  8b0dfc347d00           -mov ecx, dword ptr [0x7d34fc]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205564) /* 0x7d34fc */);
    // 004a725b  8b150c357d00           -mov edx, dword ptr [0x7d350c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205580) /* 0x7d350c */);
    // 004a7261  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 004a7266  750d                   -jne 0x4a7275
    if (!cpu.flags.zf)
    {
        goto L_0x004a7275;
    }
    // 004a7268  39c8                   +cmp eax, ecx
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
    // 004a726a  7c09                   -jl 0x4a7275
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004a7275;
    }
    // 004a726c  39d0                   +cmp eax, edx
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
    // 004a726e  7e0f                   -jle 0x4a727f
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004a727f;
    }
    // 004a7270  83cb02                 +or ebx, 2
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(2 /*0x2*/))));
    // 004a7273  eb0a                   -jmp 0x4a727f
    goto L_0x004a727f;
L_0x004a7275:
    // 004a7275  83cb01                 +or ebx, 1
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(1 /*0x1*/))));
    // 004a7278  eb05                   -jmp 0x4a727f
    goto L_0x004a727f;
L_0x004a727a:
    // 004a727a  bb10000000             -mov ebx, 0x10
    cpu.ebx = 16 /*0x10*/;
L_0x004a727f:
    // 004a727f  881f                   -mov byte ptr [edi], bl
    app->getMemory<x86::reg8>(cpu.edi) = cpu.bl;
    // 004a7281  8d7d52                 -lea edi, [ebp + 0x52]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(82) /* 0x52 */);
    // 004a7284  8d753e                 -lea esi, [ebp + 0x3e]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(62) /* 0x3e */);
    // 004a7287  8b460c                 -mov eax, dword ptr [esi + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(12) /* 0xc */);
    // 004a728a  8b0df8347d00           -mov ecx, dword ptr [0x7d34f8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205560) /* 0x7d34f8 */);
    // 004a7290  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 004a7295  7558                   -jne 0x4a72ef
    if (!cpu.flags.zf)
    {
        goto L_0x004a72ef;
    }
    // 004a7297  39c8                   +cmp eax, ecx
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
    // 004a7299  7d54                   -jge 0x4a72ef
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004a72ef;
    }
    // 004a729b  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 004a729e  bb00000000             -mov ebx, 0
    cpu.ebx = 0 /*0x0*/;
    // 004a72a3  8b0d04357d00           -mov ecx, dword ptr [0x7d3504]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205572) /* 0x7d3504 */);
    // 004a72a9  8b1500357d00           -mov edx, dword ptr [0x7d3500]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205568) /* 0x7d3500 */);
    // 004a72af  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 004a72b4  750d                   -jne 0x4a72c3
    if (!cpu.flags.zf)
    {
        goto L_0x004a72c3;
    }
    // 004a72b6  39c8                   +cmp eax, ecx
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
    // 004a72b8  7c09                   -jl 0x4a72c3
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004a72c3;
    }
    // 004a72ba  39d0                   +cmp eax, edx
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
    // 004a72bc  7e09                   -jle 0x4a72c7
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004a72c7;
    }
    // 004a72be  83cb04                 +or ebx, 4
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(4 /*0x4*/))));
    // 004a72c1  eb04                   -jmp 0x4a72c7
    goto L_0x004a72c7;
L_0x004a72c3:
    // 004a72c3  83cb08                 -or ebx, 8
    cpu.ebx |= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 004a72c6  90                     -nop 
    ;
L_0x004a72c7:
    // 004a72c7  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 004a72c9  90                     -nop 
    ;
    // 004a72ca  8b0dfc347d00           -mov ecx, dword ptr [0x7d34fc]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205564) /* 0x7d34fc */);
    // 004a72d0  8b150c357d00           -mov edx, dword ptr [0x7d350c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205580) /* 0x7d350c */);
    // 004a72d6  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 004a72db  750d                   -jne 0x4a72ea
    if (!cpu.flags.zf)
    {
        goto L_0x004a72ea;
    }
    // 004a72dd  39c8                   +cmp eax, ecx
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
    // 004a72df  7c09                   -jl 0x4a72ea
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004a72ea;
    }
    // 004a72e1  39d0                   +cmp eax, edx
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
    // 004a72e3  7e0f                   -jle 0x4a72f4
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004a72f4;
    }
    // 004a72e5  83cb02                 +or ebx, 2
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(2 /*0x2*/))));
    // 004a72e8  eb0a                   -jmp 0x4a72f4
    goto L_0x004a72f4;
L_0x004a72ea:
    // 004a72ea  83cb01                 +or ebx, 1
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(1 /*0x1*/))));
    // 004a72ed  eb05                   -jmp 0x4a72f4
    goto L_0x004a72f4;
L_0x004a72ef:
    // 004a72ef  bb10000000             -mov ebx, 0x10
    cpu.ebx = 16 /*0x10*/;
L_0x004a72f4:
    // 004a72f4  881f                   -mov byte ptr [edi], bl
    app->getMemory<x86::reg8>(cpu.edi) = cpu.bl;
    // 004a72f6  8d4d3e                 -lea ecx, [ebp + 0x3e]
    cpu.ecx = x86::reg32(cpu.ebp + x86::reg32(62) /* 0x3e */);
    // 004a72f9  8d5d1e                 -lea ebx, [ebp + 0x1e]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(30) /* 0x1e */);
    // 004a72fc  8d55fe                 -lea edx, [ebp - 2]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-2) /* -0x2 */);
    // 004a72ff  8d45de                 -lea eax, [ebp - 0x22]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-34) /* -0x22 */);
    // 004a7302  e8d99f0100             -call 0x4c12e0
    cpu.esp -= 4;
    nfs3hp::vertices::clipQuad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a7307  8d656e                 -lea esp, [ebp + 0x6e]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(110) /* 0x6e */);
    // 004a730a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a730b  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a730c  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a730d  c21400                 -ret 0x14
    cpu.esp += 4+20 /*0x14*/;
    return;
}

/* align: skip  */
void Application::sub_4a7310(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1, fp2;
    // 004a7310  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004a7311  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004a7312  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a7313  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a7315  81ec90000000           -sub esp, 0x90
    (cpu.esp) -= x86::reg32(x86::sreg32(144 /*0x90*/));
    // 004a731b  83ed4a                 -sub ebp, 0x4a
    (cpu.ebp) -= x86::reg32(x86::sreg32(74 /*0x4a*/));
    // 004a731e  894d3a                 -mov dword ptr [ebp + 0x3a], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(58) /* 0x3a */) = cpu.ecx;
    // 004a7321  b920000000             -mov ecx, 0x20
    cpu.ecx = 32 /*0x20*/;
    // 004a7326  8d7dba                 -lea edi, [ebp - 0x46]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-70) /* -0x46 */);
    // 004a7329  be00704a00             -mov esi, 0x4a7000
    cpu.esi = 4878336 /*0x4a7000*/;
    // 004a732e  89453e                 -mov dword ptr [ebp + 0x3e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(62) /* 0x3e */) = cpu.eax;
    // 004a7331  895542                 -mov dword ptr [ebp + 0x42], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(66) /* 0x42 */) = cpu.edx;
    // 004a7334  895d46                 -mov dword ptr [ebp + 0x46], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(70) /* 0x46 */) = cpu.ebx;
    // 004a7337  db453e                 -fild dword ptr [ebp + 0x3e]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(62) /* 0x3e */)));
    // 004a733a  8b858e000000           -mov eax, dword ptr [ebp + 0x8e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(142) /* 0x8e */);
    // 004a7340  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004a7342  db4542                 -fild dword ptr [ebp + 0x42]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(66) /* 0x42 */)));
    // 004a7345  8945ca                 -mov dword ptr [ebp - 0x36], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-54) /* -0x36 */) = cpu.eax;
    // 004a7348  8b456a                 -mov eax, dword ptr [ebp + 0x6a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(106) /* 0x6a */);
    // 004a734b  db4546                 -fild dword ptr [ebp + 0x46]
    fp2 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(70) /* 0x46 */)));
    // 004a734e  8945d2                 -mov dword ptr [ebp - 0x2e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-46) /* -0x2e */) = cpu.eax;
    // 004a7351  8b456e                 -mov eax, dword ptr [ebp + 0x6e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(110) /* 0x6e */);
    // 004a7354  8b958a000000           -mov edx, dword ptr [ebp + 0x8a]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(138) /* 0x8a */);
    // 004a735a  8945d6                 -mov dword ptr [ebp - 0x2a], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-42) /* -0x2a */) = cpu.eax;
    // 004a735d  8b453a                 -mov eax, dword ptr [ebp + 0x3a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(58) /* 0x3a */);
    // 004a7360  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 004a7362  d95dba                 -fstp dword ptr [ebp - 0x46]
    app->getMemory<float>(cpu.ebp + x86::reg32(-70) /* -0x46 */) = float(fp2);
    // 004a7365  894546                 -mov dword ptr [ebp + 0x46], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(70) /* 0x46 */) = cpu.eax;
    // 004a7368  8b45ca                 -mov eax, dword ptr [ebp - 0x36]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-54) /* -0x36 */);
    // 004a736b  d95dbe                 -fstp dword ptr [ebp - 0x42]
    app->getMemory<float>(cpu.ebp + x86::reg32(-66) /* -0x42 */) = float(fp1);
    // 004a736e  8945ea                 -mov dword ptr [ebp - 0x16], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-22) /* -0x16 */) = cpu.eax;
    // 004a7371  8b4572                 -mov eax, dword ptr [ebp + 0x72]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(114) /* 0x72 */);
    // 004a7374  d95dda                 -fstp dword ptr [ebp - 0x26]
    app->getMemory<float>(cpu.ebp + x86::reg32(-38) /* -0x26 */) = float(fp0);
    // 004a7377  8945f2                 -mov dword ptr [ebp - 0xe], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-14) /* -0xe */) = cpu.eax;
    // 004a737a  8b4576                 -mov eax, dword ptr [ebp + 0x76]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */);
    // 004a737d  8d7dce                 -lea edi, [ebp - 0x32]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-50) /* -0x32 */);
    // 004a7380  8945f6                 -mov dword ptr [ebp - 0xa], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-10) /* -0xa */) = cpu.eax;
    // 004a7383  8b455a                 -mov eax, dword ptr [ebp + 0x5a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(90) /* 0x5a */);
    // 004a7386  8d75ba                 -lea esi, [ebp - 0x46]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-70) /* -0x46 */);
    // 004a7389  89453e                 -mov dword ptr [ebp + 0x3e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(62) /* 0x3e */) = cpu.eax;
    // 004a738c  8b455e                 -mov eax, dword ptr [ebp + 0x5e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(94) /* 0x5e */);
    // 004a738f  db4546                 -fild dword ptr [ebp + 0x46]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(70) /* 0x46 */)));
    // 004a7392  894546                 -mov dword ptr [ebp + 0x46], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(70) /* 0x46 */) = cpu.eax;
    // 004a7395  8b457a                 -mov eax, dword ptr [ebp + 0x7a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 004a7398  d95dde                 -fstp dword ptr [ebp - 0x22]
    app->getMemory<float>(cpu.ebp + x86::reg32(-34) /* -0x22 */) = float(fp0);
    // 004a739b  894512                 -mov dword ptr [ebp + 0x12], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(18) /* 0x12 */) = cpu.eax;
    // 004a739e  8b457e                 -mov eax, dword ptr [ebp + 0x7e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 004a73a1  db453e                 -fild dword ptr [ebp + 0x3e]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(62) /* 0x3e */)));
    // 004a73a4  894516                 -mov dword ptr [ebp + 0x16], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(22) /* 0x16 */) = cpu.eax;
    // 004a73a7  8b45ca                 -mov eax, dword ptr [ebp - 0x36]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-54) /* -0x36 */);
    // 004a73aa  d95dfa                 -fstp dword ptr [ebp - 6]
    app->getMemory<float>(cpu.ebp + x86::reg32(-6) /* -0x6 */) = float(fp0);
    // 004a73ad  89450a                 -mov dword ptr [ebp + 0xa], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(10) /* 0xa */) = cpu.eax;
    // 004a73b0  8b4562                 -mov eax, dword ptr [ebp + 0x62]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(98) /* 0x62 */);
    // 004a73b3  db4546                 -fild dword ptr [ebp + 0x46]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(70) /* 0x46 */)));
    // 004a73b6  894542                 -mov dword ptr [ebp + 0x42], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(66) /* 0x42 */) = cpu.eax;
    // 004a73b9  8b4566                 -mov eax, dword ptr [ebp + 0x66]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(102) /* 0x66 */);
    // 004a73bc  d95dfe                 -fstp dword ptr [ebp - 2]
    app->getMemory<float>(cpu.ebp + x86::reg32(-2) /* -0x2 */) = float(fp0);
    // 004a73bf  89453e                 -mov dword ptr [ebp + 0x3e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(62) /* 0x3e */) = cpu.eax;
    // 004a73c2  8b8582000000           -mov eax, dword ptr [ebp + 0x82]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(130) /* 0x82 */);
    // 004a73c8  db4542                 -fild dword ptr [ebp + 0x42]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(66) /* 0x42 */)));
    // 004a73cb  894532                 -mov dword ptr [ebp + 0x32], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(50) /* 0x32 */) = cpu.eax;
    // 004a73ce  8b8586000000           -mov eax, dword ptr [ebp + 0x86]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(134) /* 0x86 */);
    // 004a73d4  d95d1a                 -fstp dword ptr [ebp + 0x1a]
    app->getMemory<float>(cpu.ebp + x86::reg32(26) /* 0x1a */) = float(fp0);
    // 004a73d7  894536                 -mov dword ptr [ebp + 0x36], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(54) /* 0x36 */) = cpu.eax;
    // 004a73da  8b45ca                 -mov eax, dword ptr [ebp - 0x36]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-54) /* -0x36 */);
    // 004a73dd  db453e                 -fild dword ptr [ebp + 0x3e]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(62) /* 0x3e */)));
    // 004a73e0  89452a                 -mov dword ptr [ebp + 0x2a], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(42) /* 0x2a */) = cpu.eax;
    // 004a73e3  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004a73e8  d95d1e                 -fstp dword ptr [ebp + 0x1e]
    app->getMemory<float>(cpu.ebp + x86::reg32(30) /* 0x1e */) = float(fp0);
    // 004a73eb  e810a5f8ff             -call 0x431900
    cpu.esp -= 4;
    nfs3hp::graphics::setRenderState(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a73f0  8b460c                 -mov eax, dword ptr [esi + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(12) /* 0xc */);
    // 004a73f3  8b0df8347d00           -mov ecx, dword ptr [0x7d34f8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205560) /* 0x7d34f8 */);
    // 004a73f9  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 004a73fe  7558                   -jne 0x4a7458
    if (!cpu.flags.zf)
    {
        goto L_0x004a7458;
    }
    // 004a7400  39c8                   +cmp eax, ecx
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
    // 004a7402  7d54                   -jge 0x4a7458
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004a7458;
    }
    // 004a7404  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 004a7407  bb00000000             -mov ebx, 0
    cpu.ebx = 0 /*0x0*/;
    // 004a740c  8b0d04357d00           -mov ecx, dword ptr [0x7d3504]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205572) /* 0x7d3504 */);
    // 004a7412  8b1500357d00           -mov edx, dword ptr [0x7d3500]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205568) /* 0x7d3500 */);
    // 004a7418  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 004a741d  750d                   -jne 0x4a742c
    if (!cpu.flags.zf)
    {
        goto L_0x004a742c;
    }
    // 004a741f  39c8                   +cmp eax, ecx
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
    // 004a7421  7c09                   -jl 0x4a742c
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004a742c;
    }
    // 004a7423  39d0                   +cmp eax, edx
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
    // 004a7425  7e09                   -jle 0x4a7430
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004a7430;
    }
    // 004a7427  83cb04                 +or ebx, 4
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(4 /*0x4*/))));
    // 004a742a  eb04                   -jmp 0x4a7430
    goto L_0x004a7430;
L_0x004a742c:
    // 004a742c  83cb08                 -or ebx, 8
    cpu.ebx |= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 004a742f  90                     -nop 
    ;
L_0x004a7430:
    // 004a7430  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 004a7432  90                     -nop 
    ;
    // 004a7433  8b0dfc347d00           -mov ecx, dword ptr [0x7d34fc]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205564) /* 0x7d34fc */);
    // 004a7439  8b150c357d00           -mov edx, dword ptr [0x7d350c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205580) /* 0x7d350c */);
    // 004a743f  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 004a7444  750d                   -jne 0x4a7453
    if (!cpu.flags.zf)
    {
        goto L_0x004a7453;
    }
    // 004a7446  39c8                   +cmp eax, ecx
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
    // 004a7448  7c09                   -jl 0x4a7453
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004a7453;
    }
    // 004a744a  39d0                   +cmp eax, edx
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
    // 004a744c  7e0f                   -jle 0x4a745d
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004a745d;
    }
    // 004a744e  83cb02                 +or ebx, 2
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(2 /*0x2*/))));
    // 004a7451  eb0a                   -jmp 0x4a745d
    goto L_0x004a745d;
L_0x004a7453:
    // 004a7453  83cb01                 +or ebx, 1
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(1 /*0x1*/))));
    // 004a7456  eb05                   -jmp 0x4a745d
    goto L_0x004a745d;
L_0x004a7458:
    // 004a7458  bb10000000             -mov ebx, 0x10
    cpu.ebx = 16 /*0x10*/;
L_0x004a745d:
    // 004a745d  881f                   -mov byte ptr [edi], bl
    app->getMemory<x86::reg8>(cpu.edi) = cpu.bl;
    // 004a745f  8d7dee                 -lea edi, [ebp - 0x12]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-18) /* -0x12 */);
    // 004a7462  8d75da                 -lea esi, [ebp - 0x26]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-38) /* -0x26 */);
    // 004a7465  8b460c                 -mov eax, dword ptr [esi + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(12) /* 0xc */);
    // 004a7468  8b0df8347d00           -mov ecx, dword ptr [0x7d34f8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205560) /* 0x7d34f8 */);
    // 004a746e  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 004a7473  7558                   -jne 0x4a74cd
    if (!cpu.flags.zf)
    {
        goto L_0x004a74cd;
    }
    // 004a7475  39c8                   +cmp eax, ecx
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
    // 004a7477  7d54                   -jge 0x4a74cd
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004a74cd;
    }
    // 004a7479  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 004a747c  bb00000000             -mov ebx, 0
    cpu.ebx = 0 /*0x0*/;
    // 004a7481  8b0d04357d00           -mov ecx, dword ptr [0x7d3504]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205572) /* 0x7d3504 */);
    // 004a7487  8b1500357d00           -mov edx, dword ptr [0x7d3500]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205568) /* 0x7d3500 */);
    // 004a748d  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 004a7492  750d                   -jne 0x4a74a1
    if (!cpu.flags.zf)
    {
        goto L_0x004a74a1;
    }
    // 004a7494  39c8                   +cmp eax, ecx
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
    // 004a7496  7c09                   -jl 0x4a74a1
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004a74a1;
    }
    // 004a7498  39d0                   +cmp eax, edx
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
    // 004a749a  7e09                   -jle 0x4a74a5
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004a74a5;
    }
    // 004a749c  83cb04                 +or ebx, 4
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(4 /*0x4*/))));
    // 004a749f  eb04                   -jmp 0x4a74a5
    goto L_0x004a74a5;
L_0x004a74a1:
    // 004a74a1  83cb08                 -or ebx, 8
    cpu.ebx |= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 004a74a4  90                     -nop 
    ;
L_0x004a74a5:
    // 004a74a5  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 004a74a7  90                     -nop 
    ;
    // 004a74a8  8b0dfc347d00           -mov ecx, dword ptr [0x7d34fc]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205564) /* 0x7d34fc */);
    // 004a74ae  8b150c357d00           -mov edx, dword ptr [0x7d350c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205580) /* 0x7d350c */);
    // 004a74b4  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 004a74b9  750d                   -jne 0x4a74c8
    if (!cpu.flags.zf)
    {
        goto L_0x004a74c8;
    }
    // 004a74bb  39c8                   +cmp eax, ecx
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
    // 004a74bd  7c09                   -jl 0x4a74c8
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004a74c8;
    }
    // 004a74bf  39d0                   +cmp eax, edx
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
    // 004a74c1  7e0f                   -jle 0x4a74d2
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004a74d2;
    }
    // 004a74c3  83cb02                 +or ebx, 2
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(2 /*0x2*/))));
    // 004a74c6  eb0a                   -jmp 0x4a74d2
    goto L_0x004a74d2;
L_0x004a74c8:
    // 004a74c8  83cb01                 +or ebx, 1
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(1 /*0x1*/))));
    // 004a74cb  eb05                   -jmp 0x4a74d2
    goto L_0x004a74d2;
L_0x004a74cd:
    // 004a74cd  bb10000000             -mov ebx, 0x10
    cpu.ebx = 16 /*0x10*/;
L_0x004a74d2:
    // 004a74d2  881f                   -mov byte ptr [edi], bl
    app->getMemory<x86::reg8>(cpu.edi) = cpu.bl;
    // 004a74d4  8d7d0e                 -lea edi, [ebp + 0xe]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(14) /* 0xe */);
    // 004a74d7  8d75fa                 -lea esi, [ebp - 6]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-6) /* -0x6 */);
    // 004a74da  8b460c                 -mov eax, dword ptr [esi + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(12) /* 0xc */);
    // 004a74dd  8b0df8347d00           -mov ecx, dword ptr [0x7d34f8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205560) /* 0x7d34f8 */);
    // 004a74e3  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 004a74e8  7558                   -jne 0x4a7542
    if (!cpu.flags.zf)
    {
        goto L_0x004a7542;
    }
    // 004a74ea  39c8                   +cmp eax, ecx
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
    // 004a74ec  7d54                   -jge 0x4a7542
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004a7542;
    }
    // 004a74ee  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 004a74f1  bb00000000             -mov ebx, 0
    cpu.ebx = 0 /*0x0*/;
    // 004a74f6  8b0d04357d00           -mov ecx, dword ptr [0x7d3504]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205572) /* 0x7d3504 */);
    // 004a74fc  8b1500357d00           -mov edx, dword ptr [0x7d3500]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205568) /* 0x7d3500 */);
    // 004a7502  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 004a7507  750d                   -jne 0x4a7516
    if (!cpu.flags.zf)
    {
        goto L_0x004a7516;
    }
    // 004a7509  39c8                   +cmp eax, ecx
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
    // 004a750b  7c09                   -jl 0x4a7516
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004a7516;
    }
    // 004a750d  39d0                   +cmp eax, edx
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
    // 004a750f  7e09                   -jle 0x4a751a
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004a751a;
    }
    // 004a7511  83cb04                 +or ebx, 4
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(4 /*0x4*/))));
    // 004a7514  eb04                   -jmp 0x4a751a
    goto L_0x004a751a;
L_0x004a7516:
    // 004a7516  83cb08                 -or ebx, 8
    cpu.ebx |= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 004a7519  90                     -nop 
    ;
L_0x004a751a:
    // 004a751a  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 004a751c  90                     -nop 
    ;
    // 004a751d  8b0dfc347d00           -mov ecx, dword ptr [0x7d34fc]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205564) /* 0x7d34fc */);
    // 004a7523  8b150c357d00           -mov edx, dword ptr [0x7d350c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205580) /* 0x7d350c */);
    // 004a7529  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 004a752e  750d                   -jne 0x4a753d
    if (!cpu.flags.zf)
    {
        goto L_0x004a753d;
    }
    // 004a7530  39c8                   +cmp eax, ecx
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
    // 004a7532  7c09                   -jl 0x4a753d
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004a753d;
    }
    // 004a7534  39d0                   +cmp eax, edx
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
    // 004a7536  7e0f                   -jle 0x4a7547
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004a7547;
    }
    // 004a7538  83cb02                 +or ebx, 2
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(2 /*0x2*/))));
    // 004a753b  eb0a                   -jmp 0x4a7547
    goto L_0x004a7547;
L_0x004a753d:
    // 004a753d  83cb01                 +or ebx, 1
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(1 /*0x1*/))));
    // 004a7540  eb05                   -jmp 0x4a7547
    goto L_0x004a7547;
L_0x004a7542:
    // 004a7542  bb10000000             -mov ebx, 0x10
    cpu.ebx = 16 /*0x10*/;
L_0x004a7547:
    // 004a7547  881f                   -mov byte ptr [edi], bl
    app->getMemory<x86::reg8>(cpu.edi) = cpu.bl;
    // 004a7549  8d7d2e                 -lea edi, [ebp + 0x2e]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(46) /* 0x2e */);
    // 004a754c  8d751a                 -lea esi, [ebp + 0x1a]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(26) /* 0x1a */);
    // 004a754f  8b460c                 -mov eax, dword ptr [esi + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(12) /* 0xc */);
    // 004a7552  8b0df8347d00           -mov ecx, dword ptr [0x7d34f8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205560) /* 0x7d34f8 */);
    // 004a7558  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 004a755d  7558                   -jne 0x4a75b7
    if (!cpu.flags.zf)
    {
        goto L_0x004a75b7;
    }
    // 004a755f  39c8                   +cmp eax, ecx
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
    // 004a7561  7d54                   -jge 0x4a75b7
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004a75b7;
    }
    // 004a7563  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 004a7566  bb00000000             -mov ebx, 0
    cpu.ebx = 0 /*0x0*/;
    // 004a756b  8b0d04357d00           -mov ecx, dword ptr [0x7d3504]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205572) /* 0x7d3504 */);
    // 004a7571  8b1500357d00           -mov edx, dword ptr [0x7d3500]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205568) /* 0x7d3500 */);
    // 004a7577  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 004a757c  750d                   -jne 0x4a758b
    if (!cpu.flags.zf)
    {
        goto L_0x004a758b;
    }
    // 004a757e  39c8                   +cmp eax, ecx
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
    // 004a7580  7c09                   -jl 0x4a758b
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004a758b;
    }
    // 004a7582  39d0                   +cmp eax, edx
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
    // 004a7584  7e09                   -jle 0x4a758f
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004a758f;
    }
    // 004a7586  83cb04                 +or ebx, 4
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(4 /*0x4*/))));
    // 004a7589  eb04                   -jmp 0x4a758f
    goto L_0x004a758f;
L_0x004a758b:
    // 004a758b  83cb08                 -or ebx, 8
    cpu.ebx |= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 004a758e  90                     -nop 
    ;
L_0x004a758f:
    // 004a758f  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 004a7591  90                     -nop 
    ;
    // 004a7592  8b0dfc347d00           -mov ecx, dword ptr [0x7d34fc]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205564) /* 0x7d34fc */);
    // 004a7598  8b150c357d00           -mov edx, dword ptr [0x7d350c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205580) /* 0x7d350c */);
    // 004a759e  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 004a75a3  750d                   -jne 0x4a75b2
    if (!cpu.flags.zf)
    {
        goto L_0x004a75b2;
    }
    // 004a75a5  39c8                   +cmp eax, ecx
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
    // 004a75a7  7c09                   -jl 0x4a75b2
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004a75b2;
    }
    // 004a75a9  39d0                   +cmp eax, edx
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
    // 004a75ab  7e0f                   -jle 0x4a75bc
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004a75bc;
    }
    // 004a75ad  83cb02                 +or ebx, 2
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(2 /*0x2*/))));
    // 004a75b0  eb0a                   -jmp 0x4a75bc
    goto L_0x004a75bc;
L_0x004a75b2:
    // 004a75b2  83cb01                 +or ebx, 1
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(1 /*0x1*/))));
    // 004a75b5  eb05                   -jmp 0x4a75bc
    goto L_0x004a75bc;
L_0x004a75b7:
    // 004a75b7  bb10000000             -mov ebx, 0x10
    cpu.ebx = 16 /*0x10*/;
L_0x004a75bc:
    // 004a75bc  881f                   -mov byte ptr [edi], bl
    app->getMemory<x86::reg8>(cpu.edi) = cpu.bl;
    // 004a75be  8d4d1a                 -lea ecx, [ebp + 0x1a]
    cpu.ecx = x86::reg32(cpu.ebp + x86::reg32(26) /* 0x1a */);
    // 004a75c1  8d5dfa                 -lea ebx, [ebp - 6]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-6) /* -0x6 */);
    // 004a75c4  8d55da                 -lea edx, [ebp - 0x26]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-38) /* -0x26 */);
    // 004a75c7  8d45ba                 -lea eax, [ebp - 0x46]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-70) /* -0x46 */);
    // 004a75ca  e8119d0100             -call 0x4c12e0
    cpu.esp -= 4;
    nfs3hp::vertices::clipQuad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a75cf  8d654a                 -lea esp, [ebp + 0x4a]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(74) /* 0x4a */);
    // 004a75d2  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a75d3  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a75d4  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a75d5  c23800                 -ret 0x38
    cpu.esp += 4+56 /*0x38*/;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_4a75e0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004a75e0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a75e1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a75e2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004a75e3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004a75e4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004a75e5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004a75e6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004a75e8  83ec18                 -sub esp, 0x18
    (cpu.esp) -= x86::reg32(x86::sreg32(24 /*0x18*/));
    // 004a75eb  8d4df4                 -lea ecx, [ebp - 0xc]
    cpu.ecx = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004a75ee  8d5df0                 -lea ebx, [ebp - 0x10]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004a75f1  8d55ec                 -lea edx, [ebp - 0x14]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004a75f4  8d45e8                 -lea eax, [ebp - 0x18]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004a75f7  e884750100             -call 0x4beb80
    cpu.esp -= 4;
    nfs3hp::port::text::viewport(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a75fc  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004a75ff  c1e205                 -shl edx, 5
    cpu.edx <<= 5 /*0x5*/ % 32;
    // 004a7602  b9c0030000             -mov ecx, 0x3c0
    cpu.ecx = 960 /*0x3c0*/;
    // 004a7607  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004a7609  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004a760c  f7f9                   -idiv ecx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ecx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004a760e  8b55f0                 -mov edx, dword ptr [ebp - 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004a7611  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 004a7613  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004a761a  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004a761c  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004a761f  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004a7621  b980020000             -mov ecx, 0x280
    cpu.ecx = 640 /*0x280*/;
    // 004a7626  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004a7628  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004a762b  f7f9                   -idiv ecx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ecx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004a762d  8b55f0                 -mov edx, dword ptr [ebp - 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004a7630  c1e204                 -shl edx, 4
    cpu.edx <<= 4 /*0x4*/ % 32;
    // 004a7633  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004a7636  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004a7638  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004a763b  f7f9                   -idiv ecx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ecx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004a763d  8b55f0                 -mov edx, dword ptr [ebp - 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004a7640  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 004a7643  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004a764a  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004a764c  8d9040010000           -lea edx, [eax + 0x140]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(320) /* 0x140 */);
    // 004a7652  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004a7654  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004a7657  f7f9                   -idiv ecx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ecx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004a7659  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004a765b  8b45f0                 -mov eax, dword ptr [ebp - 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004a765e  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004a7660  b914000000             -mov ecx, 0x14
    cpu.ecx = 20 /*0x14*/;
    // 004a7665  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004a7668  f7f9                   -idiv ecx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ecx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004a766a  8b55e8                 -mov edx, dword ptr [ebp - 0x18]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004a766d  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004a766f  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004a7672  8955e8                 -mov dword ptr [ebp - 0x18], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = cpu.edx;
    // 004a7675  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004a7677  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004a767a  f7f9                   -idiv ecx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ecx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004a767c  0345ec                 -add eax, dword ptr [ebp - 0x14]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */)));
    // 004a767f  01f8                   -add eax, edi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edi));
    // 004a7681  8b55f0                 -mov edx, dword ptr [ebp - 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004a7684  8945ec                 -mov dword ptr [ebp - 0x14], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.eax;
    // 004a7687  8d04d500000000         -lea eax, [edx*8]
    cpu.eax = x86::reg32(cpu.edx * 8);
    // 004a768e  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004a7690  01d2                   -add edx, edx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.edx));
    // 004a7692  b914000000             -mov ecx, 0x14
    cpu.ecx = 20 /*0x14*/;
    // 004a7697  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004a7699  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004a769c  f7f9                   -idiv ecx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ecx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004a769e  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004a76a1  8945f0                 -mov dword ptr [ebp - 0x10], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.eax;
    // 004a76a4  8d04d500000000         -lea eax, [edx*8]
    cpu.eax = x86::reg32(cpu.edx * 8);
    // 004a76ab  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004a76ad  01d2                   -add edx, edx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.edx));
    // 004a76af  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004a76b1  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004a76b4  f7f9                   -idiv ecx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ecx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004a76b6  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
    // 004a76b9  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004a76bc  8b5df4                 -mov ebx, dword ptr [ebp - 0xc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004a76bf  683f06103f             -push 0x3f10063f
    app->getMemory<x86::reg32>(cpu.esp-4) = 1058014783 /*0x3f10063f*/;
    cpu.esp -= 4;
    // 004a76c4  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 004a76c6  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a76c7  8b55e8                 -mov edx, dword ptr [ebp - 0x18]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004a76ca  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004a76cb  8b4df0                 -mov ecx, dword ptr [ebp - 0x10]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004a76ce  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a76cf  8d1c0a                 -lea ebx, [edx + ecx]
    cpu.ebx = x86::reg32(cpu.edx + cpu.ecx * 1);
    // 004a76d2  8b4dec                 -mov ecx, dword ptr [ebp - 0x14]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004a76d5  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a76d6  8b45e8                 -mov eax, dword ptr [ebp - 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004a76d9  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 004a76db  e8a0f9ffff             -call 0x4a7080
    cpu.esp -= 4;
    sub_4a7080(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a76e0  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004a76e3  8b5df4                 -mov ebx, dword ptr [ebp - 0xc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004a76e6  6800000020             -push 0x20000000
    app->getMemory<x86::reg32>(cpu.esp-4) = 536870912 /*0x20000000*/;
    cpu.esp -= 4;
    // 004a76eb  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 004a76ed  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a76ee  8b55e8                 -mov edx, dword ptr [ebp - 0x18]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004a76f1  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004a76f2  8b4df0                 -mov ecx, dword ptr [ebp - 0x10]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004a76f5  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a76f6  8d1c0a                 -lea ebx, [edx + ecx]
    cpu.ebx = x86::reg32(cpu.edx + cpu.ecx * 1);
    // 004a76f9  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a76fa  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004a76fc  8b45e8                 -mov eax, dword ptr [ebp - 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004a76ff  29f2                   -sub edx, esi
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.esi));
    // 004a7701  29f3                   -sub ebx, esi
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.esi));
    // 004a7703  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 004a7705  01f0                   -add eax, esi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.esi));
    // 004a7707  e874f9ffff             -call 0x4a7080
    cpu.esp -= 4;
    sub_4a7080(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a770c  8b55ec                 -mov edx, dword ptr [ebp - 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004a770f  8b5df4                 -mov ebx, dword ptr [ebp - 0xc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004a7712  01da                   -add edx, ebx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.ebx));
    // 004a7714  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004a7716  6800000020             -push 0x20000000
    app->getMemory<x86::reg32>(cpu.esp-4) = 536870912 /*0x20000000*/;
    cpu.esp -= 4;
    // 004a771b  29f0                   -sub eax, esi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.esi));
    // 004a771d  8b5de8                 -mov ebx, dword ptr [ebp - 0x18]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004a7720  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a7721  8b45f0                 -mov eax, dword ptr [ebp - 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004a7724  01c3                   -add ebx, eax
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004a7726  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 004a7728  29f0                   -sub eax, esi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.esi));
    // 004a772a  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a772b  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004a772c  8b55ec                 -mov edx, dword ptr [ebp - 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004a772f  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a7730  8b4dec                 -mov ecx, dword ptr [ebp - 0x14]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004a7733  01f2                   -add edx, esi
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.esi));
    // 004a7735  e846f9ffff             -call 0x4a7080
    cpu.esp -= 4;
    sub_4a7080(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a773a  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004a773d  68ffffff20             -push 0x20ffffff
    app->getMemory<x86::reg32>(cpu.esp-4) = 553648127 /*0x20ffffff*/;
    cpu.esp -= 4;
    // 004a7742  01f0                   -add eax, esi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.esi));
    // 004a7744  8b55e8                 -mov edx, dword ptr [ebp - 0x18]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004a7747  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a7748  01f2                   -add edx, esi
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.esi));
    // 004a774a  8b5de8                 -mov ebx, dword ptr [ebp - 0x18]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004a774d  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004a774e  035df0                 -add ebx, dword ptr [ebp - 0x10]
    (cpu.ebx) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */)));
    // 004a7751  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a7752  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 004a7754  29f0                   -sub eax, esi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.esi));
    // 004a7756  8b4dec                 -mov ecx, dword ptr [ebp - 0x14]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004a7759  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a775a  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 004a775c  8b45e8                 -mov eax, dword ptr [ebp - 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004a775f  e81cf9ffff             -call 0x4a7080
    cpu.esp -= 4;
    sub_4a7080(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a7764  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004a7767  8b4df4                 -mov ecx, dword ptr [ebp - 0xc]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004a776a  68ffffff20             -push 0x20ffffff
    app->getMemory<x86::reg32>(cpu.esp-4) = 553648127 /*0x20ffffff*/;
    cpu.esp -= 4;
    // 004a776f  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004a7771  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a7772  8b5de8                 -mov ebx, dword ptr [ebp - 0x18]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004a7775  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a7776  29f0                   -sub eax, esi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.esi));
    // 004a7778  8b55ec                 -mov edx, dword ptr [ebp - 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004a777b  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a777c  01f3                   -add ebx, esi
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.esi));
    // 004a777e  8b4dec                 -mov ecx, dword ptr [ebp - 0x14]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004a7781  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a7782  01f1                   -add ecx, esi
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.esi));
    // 004a7784  8b45e8                 -mov eax, dword ptr [ebp - 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004a7787  e8f4f8ffff             -call 0x4a7080
    cpu.esp -= 4;
    sub_4a7080(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a778c  68ffffffaf             -push 0xafffffff
    app->getMemory<x86::reg32>(cpu.esp-4) = 2952790015 /*0xafffffff*/;
    cpu.esp -= 4;
    // 004a7791  8b1d389a7400           -mov ebx, dword ptr [0x749a38]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7641656) /* 0x749a38 */);
    // 004a7797  8b75ec                 -mov esi, dword ptr [ebp - 0x14]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004a779a  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a779b  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004a779e  8b55e8                 -mov edx, dword ptr [ebp - 0x18]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004a77a1  680000633f             -push 0x3f630000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1063452672 /*0x3f630000*/;
    cpu.esp -= 4;
    // 004a77a6  8b4df0                 -mov ecx, dword ptr [ebp - 0x10]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004a77a9  6800004d3f             -push 0x3f4d0000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1062010880 /*0x3f4d0000*/;
    cpu.esp -= 4;
    // 004a77ae  29fe                   -sub esi, edi
    (cpu.esi) -= x86::reg32(x86::sreg32(cpu.edi));
    // 004a77b0  680000633f             -push 0x3f630000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1063452672 /*0x3f630000*/;
    cpu.esp -= 4;
    // 004a77b5  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 004a77b7  680000623f             -push 0x3f620000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1063387136 /*0x3f620000*/;
    cpu.esp -= 4;
    // 004a77bc  8975ec                 -mov dword ptr [ebp - 0x14], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.esi;
    // 004a77bf  680000443f             -push 0x3f440000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1061421056 /*0x3f440000*/;
    cpu.esp -= 4;
    // 004a77c4  01c0                   -add eax, eax
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.eax));
    // 004a77c6  680000623f             -push 0x3f620000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1063387136 /*0x3f620000*/;
    cpu.esp -= 4;
    // 004a77cb  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004a77cd  680000443f             -push 0x3f440000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1061421056 /*0x3f440000*/;
    cpu.esp -= 4;
    // 004a77d2  8d043e                 -lea eax, [esi + edi]
    cpu.eax = x86::reg32(cpu.esi + cpu.edi * 1);
    // 004a77d5  6800004d3f             -push 0x3f4d0000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1062010880 /*0x3f4d0000*/;
    cpu.esp -= 4;
    // 004a77da  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a77db  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004a77dc  8955e8                 -mov dword ptr [ebp - 0x18], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = cpu.edx;
    // 004a77df  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a77e0  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004a77e3  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 004a77e5  8d1c02                 -lea ebx, [edx + eax]
    cpu.ebx = x86::reg32(cpu.edx + cpu.eax * 1);
    // 004a77e8  894df0                 -mov dword ptr [ebp - 0x10], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.ecx;
    // 004a77eb  8b55ec                 -mov edx, dword ptr [ebp - 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004a77ee  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a77ef  29fa                   -sub edx, edi
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.edi));
    // 004a77f1  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004a77f3  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 004a77f5  e816fbffff             -call 0x4a7310
    cpu.esp -= 4;
    sub_4a7310(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a77fa  68ffffffaf             -push 0xafffffff
    app->getMemory<x86::reg32>(cpu.esp-4) = 2952790015 /*0xafffffff*/;
    cpu.esp -= 4;
    // 004a77ff  8b15389a7400           -mov edx, dword ptr [0x749a38]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7641656) /* 0x749a38 */);
    // 004a7805  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004a7806  680000633f             -push 0x3f630000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1063452672 /*0x3f630000*/;
    cpu.esp -= 4;
    // 004a780b  6800007b3f             -push 0x3f7b0000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1065025536 /*0x3f7b0000*/;
    cpu.esp -= 4;
    // 004a7810  680000633f             -push 0x3f630000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1063452672 /*0x3f630000*/;
    cpu.esp -= 4;
    // 004a7815  6800007b3f             -push 0x3f7b0000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1065025536 /*0x3f7b0000*/;
    cpu.esp -= 4;
    // 004a781a  8b45e8                 -mov eax, dword ptr [ebp - 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004a781d  680000443f             -push 0x3f440000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1061421056 /*0x3f440000*/;
    cpu.esp -= 4;
    // 004a7822  8b4dfc                 -mov ecx, dword ptr [ebp - 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004a7825  6800007b3f             -push 0x3f7b0000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1065025536 /*0x3f7b0000*/;
    cpu.esp -= 4;
    // 004a782a  8b55ec                 -mov edx, dword ptr [ebp - 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004a782d  680000443f             -push 0x3f440000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1061421056 /*0x3f440000*/;
    cpu.esp -= 4;
    // 004a7832  01fa                   -add edx, edi
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.edi));
    // 004a7834  6800007b3f             -push 0x3f7b0000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1065025536 /*0x3f7b0000*/;
    cpu.esp -= 4;
    // 004a7839  8b5de8                 -mov ebx, dword ptr [ebp - 0x18]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004a783c  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004a783d  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004a783f  8b75f0                 -mov esi, dword ptr [ebp - 0x10]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004a7842  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a7843  01f3                   -add ebx, esi
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.esi));
    // 004a7845  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004a7846  8b55ec                 -mov edx, dword ptr [ebp - 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004a7849  29cb                   -sub ebx, ecx
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004a784b  29fa                   -sub edx, edi
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.edi));
    // 004a784d  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a784e  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 004a7850  e8bbfaffff             -call 0x4a7310
    cpu.esp -= 4;
    sub_4a7310(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a7855  68ffffffaf             -push 0xafffffff
    app->getMemory<x86::reg32>(cpu.esp-4) = 2952790015 /*0xafffffff*/;
    cpu.esp -= 4;
    // 004a785a  8b0d389a7400           -mov ecx, dword ptr [0x749a38]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7641656) /* 0x749a38 */);
    // 004a7860  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004a7861  680000633f             -push 0x3f630000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1063452672 /*0x3f630000*/;
    cpu.esp -= 4;
    // 004a7866  680080643f             -push 0x3f648000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1063550976 /*0x3f648000*/;
    cpu.esp -= 4;
    // 004a786b  8b55ec                 -mov edx, dword ptr [ebp - 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004a786e  680000633f             -push 0x3f630000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1063452672 /*0x3f630000*/;
    cpu.esp -= 4;
    // 004a7873  8b5de8                 -mov ebx, dword ptr [ebp - 0x18]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004a7876  680000793f             -push 0x3f790000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1064894464 /*0x3f790000*/;
    cpu.esp -= 4;
    // 004a787b  8b75f0                 -mov esi, dword ptr [ebp - 0x10]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004a787e  680000443f             -push 0x3f440000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1061421056 /*0x3f440000*/;
    cpu.esp -= 4;
    // 004a7883  01fa                   -add edx, edi
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.edi));
    // 004a7885  680000793f             -push 0x3f790000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1064894464 /*0x3f790000*/;
    cpu.esp -= 4;
    // 004a788a  01f3                   -add ebx, esi
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.esi));
    // 004a788c  680000443f             -push 0x3f440000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1061421056 /*0x3f440000*/;
    cpu.esp -= 4;
    // 004a7891  8b4dfc                 -mov ecx, dword ptr [ebp - 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004a7894  680080643f             -push 0x3f648000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1063550976 /*0x3f648000*/;
    cpu.esp -= 4;
    // 004a7899  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 004a789b  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004a789c  29c8                   -sub eax, ecx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004a789e  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004a789f  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004a78a0  8b55ec                 -mov edx, dword ptr [ebp - 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004a78a3  29fa                   -sub edx, edi
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.edi));
    // 004a78a5  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004a78a6  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 004a78a8  e863faffff             -call 0x4a7310
    cpu.esp -= 4;
    sub_4a7310(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004a78ad  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004a78af  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a78b0  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a78b1  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a78b2  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a78b3  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a78b4  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004a78b5  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4a78b6: native, nfs3hp::dead::stub_0x4a78b6 */

/* sub_4a78d6: native, nfs3hp::handling::applyTuning */

/* sub_4a83af: native, nfs3hp::dead::stub_0x4a83af */

/* sub_4a83d3: native, nfs3hp::drivetrain::computerDriver */

/* sub_4a8a2b: native, nfs3hp::dead::stub_0x4a8a2b */

/* sub_4a8a99: native, nfs3hp::drivetrain::engineTorque */

/* sub_4a8bb9: native, nfs3hp::handling::automaticGear */

/* sub_4a90a0: native, nfs3hp::drivetrain::automaticGears */

/* sub_4a9769: native, nfs3hp::drivetrain::gearLimit */

/* sub_4a97cb: native, nfs3hp::drivetrain::autoReverse */

/* sub_4a9bc4: native, nfs3hp::drivetrain::driverInputs */

/* sub_4aa4f4: native, nfs3hp::handling::matchRevsToSpeed */

/* sub_4aa682: native, nfs3hp::dead::stub_0x4aa682 */

}
