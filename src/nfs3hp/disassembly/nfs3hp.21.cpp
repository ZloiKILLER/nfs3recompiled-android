#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{
// Port (tools/apply_native_movie.py): defined in native_movie.cpp.
bool madPlay(win32::WinApplication* app, x86::CPU& cpu);
// Port (tools/apply_movie_tap.py): the UI thread requests a MAD exit.
void moviePlaying(bool playing);
bool movieSkipRequested();
struct MovieSession {
    MovieSession() { moviePlaying(true); }
    ~MovieSession() { moviePlaying(false); }
};

/* sub_48ec40: native, nfs3hp::port::soundThread */

/* sub_48ec80: native, nfs3hp::port::setSoundServer */

/* sub_48ed00: native, nfs3hp::port::clearTimerCallbacks */

/* sub_48ed30: native, nfs3hp::port::timerTick */

/* sub_48ee30: native, nfs3hp::port::addTimerCallback */

/* sub_48ee90: native, nfs3hp::port::removeTimerCallback */

/* sub_48eef0: native, nfs3hp::dead::stub_0x48eef0 */

/* sub_48ef20: native, nfs3hp::dead::stub_0x48ef20 */

/* sub_48ef30: native, nfs3hp::dead::stub_0x48ef30 */

/* sub_48ef50: native, nfs3hp::port::stopTimer */

/* sub_48eff0: native, nfs3hp::port::startTimer */

/* sub_48f1f0: native, nfs3hp::controls::bindPlayersControls */

/* sub_48f4f0: native, nfs3hp::controls::startRaceControls */

/* sub_48f550: native, nfs3hp::controls::readRaceControls */

/* sub_48f6a0: native, nfs3hp::controls::takeCarControls */

/* sub_48f6e0: native, nfs3hp::controls::stepMenuChoice */

/* sub_48f710: native, nfs3hp::controls::stepMenuChoiceOfThree */

/* sub_48f740: native, nfs3hp::controls::actionPressed */

/* sub_48f7b0: native, nfs3hp::controls::functionKeyPressed */

/* sub_48f820: native, nfs3hp::controls::pressFunctionKey */

/* sub_48f860: native, nfs3hp::controls::functionKeyRepeats */

/* sub_48f8d0: native, nfs3hp::controls::keyPressed */

/* sub_48f920: native, nfs3hp::controls::firstFunctionKeyHeld */

/* sub_48f940: native, nfs3hp::controls::bindControl */

/* sub_48f970: native, nfs3hp::controls::bindFunctionKeys */

/* sub_48fa10: native, nfs3hp::controls::queueRaceControls */

/* sub_48faa0: native, nfs3hp::dead::stub_0x48faa0 */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_48fb40(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1, fp2, fp3, fp4, fp5, fp6;
  x86::reg16 fsw = 0;
    // 0048fb40  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0048fb41  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0048fb42  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0048fb43  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0048fb45  83ec6c                 -sub esp, 0x6c
    (cpu.esp) -= x86::reg32(x86::sreg32(108 /*0x6c*/));
    // 0048fb48  8945d0                 -mov dword ptr [ebp - 0x30], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-48) /* -0x30 */) = cpu.eax;
    // 0048fb4b  8955d4                 -mov dword ptr [ebp - 0x2c], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44) /* -0x2c */) = cpu.edx;
    // 0048fb4e  89de                   -mov esi, ebx
    cpu.esi = cpu.ebx;
    // 0048fb50  89cf                   -mov edi, ecx
    cpu.edi = cpu.ecx;
    // 0048fb52  c745dc6f8f3543         -mov dword ptr [ebp - 0x24], 0x43358f6f
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = 1127583599 /*0x43358f6f*/;
    // 0048fb59  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 0048fb5b  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0048fb5d  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0048fb60  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0048fb62  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0048fb64  0345d0                 -add eax, dword ptr [ebp - 0x30]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-48) /* -0x30 */)));
    // 0048fb67  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 0048fb69  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0048fb6c  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0048fb6f  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0048fb71  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0048fb73  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0048fb75  c745cc23c40047         -mov dword ptr [ebp - 0x34], 0x4700c423
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-52) /* -0x34 */) = 1191232547 /*0x4700c423*/;
    // 0048fb7c  8d4da4                 -lea ecx, [ebp - 0x5c]
    cpu.ecx = x86::reg32(cpu.ebp + x86::reg32(-92) /* -0x5c */);
    // 0048fb7f  8d5da0                 -lea ebx, [ebp - 0x60]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-96) /* -0x60 */);
    // 0048fb82  8b55d4                 -mov edx, dword ptr [ebp - 0x2c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44) /* -0x2c */);
    // 0048fb85  db45fc                 -fild dword ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 0048fb88  01d0                   +add eax, edx
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
    // 0048fb8a  d95dd8                 +fstp dword ptr [ebp - 0x28]
    app->getMemory<float>(cpu.ebp + x86::reg32(-40) /* -0x28 */) = float(fp0);
    // 0048fb8d  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0048fb90  8d559c                 -lea edx, [ebp - 0x64]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-100) /* -0x64 */);
    // 0048fb93  db45fc                 +fild dword ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 0048fb96  8d4598                 -lea eax, [ebp - 0x68]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-104) /* -0x68 */);
    // 0048fb99  d95de4                 +fstp dword ptr [ebp - 0x1c]
    app->getMemory<float>(cpu.ebp + x86::reg32(-28) /* -0x1c */) = float(fp0);
    // 0048fb9c  e8dfef0200             -call 0x4beb80
    cpu.esp -= 4;
    nfs3hp::port::text::viewport(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0048fba1  8b55d4                 -mov edx, dword ptr [ebp - 0x2c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44) /* -0x2c */);
    // 0048fba4  8b45d0                 -mov eax, dword ptr [ebp - 0x30]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-48) /* -0x30 */);
    // 0048fba7  89f9                   -mov ecx, edi
    cpu.ecx = cpu.edi;
    // 0048fba9  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 0048fbab  e830f00200             -call 0x4bebe0
    cpu.esp -= 4;
    nfs3hp::graphics::setClip(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0048fbb0  8b4518                 -mov eax, dword ptr [ebp + 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(24) /* 0x18 */);
    // 0048fbb3  d900                   +fld dword ptr [eax]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax));
    // 0048fbb5  d9e8                   +fld1 
    fp1 = 1.0;
    // 0048fbb7  ded9                   +fcompp 
    fsw = fp::compare(fp1, fp0);
    // 0048fbb9  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 0048fbbb  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 0048fbbc  7609                   -jbe 0x48fbc7
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0048fbc7;
    }
    // 0048fbbe  8b4518                 -mov eax, dword ptr [ebp + 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(24) /* 0x18 */);
    // 0048fbc1  c7000000803f           -mov dword ptr [eax], 0x3f800000
    app->getMemory<x86::reg32>(cpu.eax) = 1065353216 /*0x3f800000*/;
L_0x0048fbc7:
    // 0048fbc7  8975fc                 -mov dword ptr [ebp - 4], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.esi;
    // 0048fbca  8b4518                 -mov eax, dword ptr [ebp + 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(24) /* 0x18 */);
    // 0048fbcd  db45fc                 -fild dword ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 0048fbd0  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 0048fbd2  d80d80bb5300           -fmul dword ptr [0x53bb80]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(5487488) /* 0x53bb80 */)));
    // 0048fbd8  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 0048fbdb  8b4514                 -mov eax, dword ptr [ebp + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    // 0048fbde  d84df8                 -fmul dword ptr [ebp - 8]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 0048fbe1  8b08                   -mov ecx, dword ptr [eax]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax);
    // 0048fbe3  d95df8                 -fstp dword ptr [ebp - 8]
    app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = float(fp0);
    // 0048fbe6  83f920                 +cmp ecx, 0x20
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(32 /*0x20*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0048fbe9  7e08                   -jle 0x48fbf3
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0048fbf3;
    }
    // 0048fbeb  c70020000000           -mov dword ptr [eax], 0x20
    app->getMemory<x86::reg32>(cpu.eax) = 32 /*0x20*/;
    // 0048fbf1  eb0b                   -jmp 0x48fbfe
    goto L_0x0048fbfe;
L_0x0048fbf3:
    // 0048fbf3  83f9e0                 +cmp ecx, -0x20
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(-32 /*-0x20*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0048fbf6  7d06                   -jge 0x48fbfe
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0048fbfe;
    }
    // 0048fbf8  c700e0ffffff           -mov dword ptr [eax], 0xffffffe0
    app->getMemory<x86::reg32>(cpu.eax) = 4294967264 /*0xffffffe0*/;
L_0x0048fbfe:
    // 0048fbfe  31f6                   +xor esi, esi
    cpu.clear_co();
    cpu.set_szp((cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi))));
    // 0048fc00  8975e8                 -mov dword ptr [ebp - 0x18], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = cpu.esi;
    // 0048fc03  e966040000             -jmp 0x49006e
    goto L_0x0049006e;
L_0x0048fc08:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0048fc08  83fa60                 +cmp edx, 0x60
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(96 /*0x60*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0048fc0b  0f8f0a040000           -jg 0x49001b
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x0049001b;
    }
    // 0048fc11  31f6                   +xor esi, esi
    cpu.clear_co();
    cpu.set_szp((cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi))));
    // 0048fc13  8975ec                 -mov dword ptr [ebp - 0x14], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.esi;
    // 0048fc16  e9e8030000             -jmp 0x490003
    goto L_0x00490003;
L_0x0048fc1b:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0048fc1b  3d80000000             +cmp eax, 0x80
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(128 /*0x80*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0048fc20  0f8fd4030000           -jg 0x48fffa
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x0048fffa;
    }
    // 0048fc26  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0048fc29  db45fc                 -fild dword ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 0048fc2c  d955a8                 -fst dword ptr [ebp - 0x58]
    app->getMemory<float>(cpu.ebp + x86::reg32(-88) /* -0x58 */) = float(fp0);
    // 0048fc2f  d84da8                 -fmul dword ptr [ebp - 0x58]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-88) /* -0x58 */)));
    // 0048fc32  83c010                 -add eax, 0x10
    (cpu.eax) += x86::reg32(x86::sreg32(16 /*0x10*/));
    // 0048fc35  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0048fc38  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0048fc3b  8b45e0                 -mov eax, dword ptr [ebp - 0x20]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 0048fc3e  db45fc                 -fild dword ptr [ebp - 4]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 0048fc41  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0048fc44  d95dac                 -fstp dword ptr [ebp - 0x54]
    app->getMemory<float>(cpu.ebp + x86::reg32(-84) /* -0x54 */) = float(fp1);
    // 0048fc47  db45fc                 -fild dword ptr [ebp - 4]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 0048fc4a  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 0048fc4c  d86dcc                 -fsubr dword ptr [ebp - 0x34]
    fp1 = fp::sub(x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-52) /* -0x34 */)), fp1);
    // 0048fc4f  83c010                 -add eax, 0x10
    (cpu.eax) += x86::reg32(x86::sreg32(16 /*0x10*/));
    // 0048fc52  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 0048fc54  d95db0                 -fstp dword ptr [ebp - 0x50]
    app->getMemory<float>(cpu.ebp + x86::reg32(-80) /* -0x50 */) = float(fp1);
    // 0048fc57  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0048fc5a  d91c24                 -fstp dword ptr [esp]
    app->getMemory<float>(cpu.esp) = float(fp0);
    // 0048fc5d  db45fc                 -fild dword ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 0048fc60  d95d94                 -fstp dword ptr [ebp - 0x6c]
    app->getMemory<float>(cpu.ebp + x86::reg32(-108) /* -0x6c */) = float(fp0);
    // 0048fc63  e8b8ac0500             -call 0x4ea920
    cpu.esp -= 4;
    nfs3hp::frontend::simSquareRoot(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0048fc68  d945ac                 -fld dword ptr [ebp - 0x54]
    fp1 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-84) /* -0x54 */));
    // 0048fc6b  d8c8                   -fmul st(0)
    fp1 = fp::mul(fp1, x86::Float(fp1));
    // 0048fc6d  d86dcc                 -fsubr dword ptr [ebp - 0x34]
    fp1 = fp::sub(x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-52) /* -0x34 */)), fp1);
    // 0048fc70  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0048fc73  d91c24                 -fstp dword ptr [esp]
    app->getMemory<float>(cpu.esp) = float(fp1);
    // 0048fc76  d95db4                 -fstp dword ptr [ebp - 0x4c]
    app->getMemory<float>(cpu.ebp + x86::reg32(-76) /* -0x4c */) = float(fp0);
    // 0048fc79  e8a2ac0500             -call 0x4ea920
    cpu.esp -= 4;
    nfs3hp::frontend::simSquareRoot(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0048fc7e  d9e8                   -fld1 
    fp1 = 1.0;
    // 0048fc80  d875b4                 -fdiv dword ptr [ebp - 0x4c]
    fp1 = fp::div(fp1, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-76) /* -0x4c */)));
    // 0048fc83  d945dc                 -fld dword ptr [ebp - 0x24]
    fp2 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-36) /* -0x24 */));
    // 0048fc86  d945a8                 -fld dword ptr [ebp - 0x58]
    fp3 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-88) /* -0x58 */));
    // 0048fc89  d8c9                   -fmul st(1)
    fp3 = fp::mul(fp3, x86::Float(fp2));
    // 0048fc8b  d945b0                 -fld dword ptr [ebp - 0x50]
    fp4 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-80) /* -0x50 */));
    // 0048fc8e  d8ca                   -fmul st(2)
    fp4 = fp::mul(fp4, x86::Float(fp2));
    // 0048fc90  d945ac                 -fld dword ptr [ebp - 0x54]
    fp5 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-84) /* -0x54 */));
    // 0048fc93  d8cb                   -fmul st(3)
    fp5 = fp::mul(fp5, x86::Float(fp2));
    // 0048fc95  d94594                 -fld dword ptr [ebp - 0x6c]
    fp6 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-108) /* -0x6c */));
    // 0048fc98  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0048fc9b  decc                   -fmulp st(4)
    fp2 = fp::mul(fp2, fp6);
    // 0048fc9d  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0048fca0  db45fc                 -fild dword ptr [ebp - 4]
    fp6 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 0048fca3  dc0d88bb5300           -fmul qword ptr [0x53bb88]
    fp6 = fp::mul(fp6, x86::Float(app->getMemory<double>(x86::reg32(5487496) /* 0x53bb88 */)));
    // 0048fca9  d9cb                   -fxch st(3)
    {
        x86::Float tmp = fp6;
        fp6 = fp3;
        fp3 = tmp;
    }
    // 0048fcab  d9cd                   -fxch st(5)
    {
        x86::Float tmp = fp6;
        fp6 = fp1;
        fp1 = tmp;
    }
    // 0048fcad  d95dfc                 -fstp dword ptr [ebp - 4]
    app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = float(fp6);
    // 0048fcb0  d9cc                   -fxch st(4)
    {
        x86::Float tmp = fp5;
        fp5 = fp1;
        fp1 = tmp;
    }
    // 0048fcb2  d84dfc                 -fmul dword ptr [ebp - 4]
    fp5 = fp::mul(fp5, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 0048fcb5  d9e8                   -fld1 
    fp6 = 1.0;
    // 0048fcb7  d9cd                   -fxch st(5)
    {
        x86::Float tmp = fp6;
        fp6 = fp1;
        fp1 = tmp;
    }
    // 0048fcb9  d95dc0                 -fstp dword ptr [ebp - 0x40]
    app->getMemory<float>(cpu.ebp + x86::reg32(-64) /* -0x40 */) = float(fp6);
    // 0048fcbc  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp5;
        fp5 = fp4;
        fp4 = tmp;
    }
    // 0048fcbe  d955bc                 -fst dword ptr [ebp - 0x44]
    app->getMemory<float>(cpu.ebp + x86::reg32(-68) /* -0x44 */) = float(fp5);
    // 0048fcc1  d945c0                 -fld dword ptr [ebp - 0x40]
    fp6 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-64) /* -0x40 */));
    // 0048fcc4  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp6;
        fp6 = fp5;
        fp5 = tmp;
    }
    // 0048fcc6  d84dfc                 -fmul dword ptr [ebp - 4]
    fp6 = fp::mul(fp6, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 0048fcc9  d9cd                   -fxch st(5)
    {
        x86::Float tmp = fp6;
        fp6 = fp1;
        fp1 = tmp;
    }
    // 0048fccb  def6                   -fdivrp st(6)
    fp0 = fp::div(fp6, x86::Float(fp0));
    // 0048fccd  d8cd                   -fmul st(5)
    fp5 = fp::mul(fp5, x86::Float(fp0));
    // 0048fccf  d945bc                 -fld dword ptr [ebp - 0x44]
    fp6 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-68) /* -0x44 */));
    // 0048fcd2  d8ce                   -fmul st(6)
    fp6 = fp::mul(fp6, x86::Float(fp0));
    // 0048fcd4  d9cc                   -fxch st(4)
    {
        x86::Float tmp = fp6;
        fp6 = fp2;
        fp2 = tmp;
    }
    // 0048fcd6  d955c0                 -fst dword ptr [ebp - 0x40]
    app->getMemory<float>(cpu.ebp + x86::reg32(-64) /* -0x40 */) = float(fp6);
    // 0048fcd9  dece                   -fmulp st(6)
    fp0 = fp::mul(fp0, fp6);
    // 0048fcdb  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp5;
        fp5 = fp4;
        fp4 = tmp;
    }
    // 0048fcdd  d84df8                 -fmul dword ptr [ebp - 8]
    fp5 = fp::mul(fp5, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 0048fce0  d9cc                   -fxch st(4)
    {
        x86::Float tmp = fp5;
        fp5 = fp1;
        fp1 = tmp;
    }
    // 0048fce2  d84df8                 -fmul dword ptr [ebp - 8]
    fp5 = fp::mul(fp5, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 0048fce5  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp5;
        fp5 = fp4;
        fp4 = tmp;
    }
    // 0048fce7  d84df8                 -fmul dword ptr [ebp - 8]
    fp5 = fp::mul(fp5, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 0048fcea  d9cb                   -fxch st(3)
    {
        x86::Float tmp = fp5;
        fp5 = fp2;
        fp2 = tmp;
    }
    // 0048fcec  d84df8                 -fmul dword ptr [ebp - 8]
    fp5 = fp::mul(fp5, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 0048fcef  d945c0                 -fld dword ptr [ebp - 0x40]
    fp6 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-64) /* -0x40 */));
    // 0048fcf2  d84dfc                 -fmul dword ptr [ebp - 4]
    fp6 = fp::mul(fp6, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 0048fcf5  d9ce                   -fxch st(6)
    {
        x86::Float tmp = fp6;
        fp6 = fp0;
        fp0 = tmp;
    }
    // 0048fcf7  d84df8                 -fmul dword ptr [ebp - 8]
    fp6 = fp::mul(fp6, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 0048fcfa  8b45f0                 -mov eax, dword ptr [ebp - 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0048fcfd  d9ce                   -fxch st(6)
    {
        x86::Float tmp = fp6;
        fp6 = fp0;
        fp0 = tmp;
    }
    // 0048fcff  d84df8                 -fmul dword ptr [ebp - 8]
    fp6 = fp::mul(fp6, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 0048fd02  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0048fd05  d9cb                   -fxch st(3)
    {
        x86::Float tmp = fp6;
        fp6 = fp3;
        fp3 = tmp;
    }
    // 0048fd07  d91df0e25500           -fstp dword ptr [0x55e2f0]
    app->getMemory<float>(x86::reg32(5628656) /* 0x55e2f0 */) = float(fp6);
    // 0048fd0d  db45fc                 -fild dword ptr [ebp - 4]
    fp6 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 0048fd10  dc0d88bb5300           -fmul qword ptr [0x53bb88]
    fp6 = fp::mul(fp6, x86::Float(app->getMemory<double>(x86::reg32(5487496) /* 0x53bb88 */)));
    // 0048fd16  8b55e8                 -mov edx, dword ptr [ebp - 0x18]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 0048fd19  8b4df4                 -mov ecx, dword ptr [ebp - 0xc]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0048fd1c  bfece25500             -mov edi, 0x55e2ec
    cpu.edi = 5628652 /*0x55e2ec*/;
    // 0048fd21  c1e202                 -shl edx, 2
    cpu.edx <<= 2 /*0x2*/ % 32;
    // 0048fd24  bed8e25500             -mov esi, 0x55e2d8
    cpu.esi = 5628632 /*0x55e2d8*/;
    // 0048fd29  01ca                   -add edx, ecx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.ecx));
    // 0048fd2b  d91df4e25500           -fstp dword ptr [0x55e2f4]
    app->getMemory<float>(x86::reg32(5628660) /* 0x55e2f4 */) = float(fp6);
    // 0048fd31  d905f0e25500           -fld dword ptr [0x55e2f0]
    fp6 = x86::Float(app->getMemory<float>(x86::reg32(5628656) /* 0x55e2f0 */));
    // 0048fd37  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 0048fd3e  dc0588bb5300           -fadd qword ptr [0x53bb88]
    fp6 = fp::add(fp6, x86::Float(app->getMemory<double>(x86::reg32(5487496) /* 0x53bb88 */)));
    // 0048fd44  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0048fd46  d91d10e35500           -fstp dword ptr [0x55e310]
    app->getMemory<float>(x86::reg32(5628688) /* 0x55e310 */) = float(fp6);
    // 0048fd4c  8b0df4e25500           -mov ecx, dword ptr [0x55e2f4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5628660) /* 0x55e2f4 */);
    // 0048fd52  890d14e35500           -mov dword ptr [0x55e314], ecx
    app->getMemory<x86::reg32>(x86::reg32(5628692) /* 0x55e314 */) = cpu.ecx;
    // 0048fd58  8b0d10e35500           -mov ecx, dword ptr [0x55e310]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5628688) /* 0x55e310 */);
    // 0048fd5e  890d30e35500           -mov dword ptr [0x55e330], ecx
    app->getMemory<x86::reg32>(x86::reg32(5628720) /* 0x55e330 */) = cpu.ecx;
    // 0048fd64  d90514e35500           -fld dword ptr [0x55e314]
    fp6 = x86::Float(app->getMemory<float>(x86::reg32(5628692) /* 0x55e314 */));
    // 0048fd6a  dc0588bb5300           -fadd qword ptr [0x53bb88]
    fp6 = fp::add(fp6, x86::Float(app->getMemory<double>(x86::reg32(5487496) /* 0x53bb88 */)));
    // 0048fd70  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 0048fd73  d91d34e35500           -fstp dword ptr [0x55e334]
    app->getMemory<float>(x86::reg32(5628724) /* 0x55e334 */) = float(fp6);
    // 0048fd79  d905f0e25500           -fld dword ptr [0x55e2f0]
    fp6 = x86::Float(app->getMemory<float>(x86::reg32(5628656) /* 0x55e2f0 */));
    // 0048fd7f  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0048fd81  d91d50e35500           -fstp dword ptr [0x55e350]
    app->getMemory<float>(x86::reg32(5628752) /* 0x55e350 */) = float(fp6);
    // 0048fd87  8b1485b8c47900         -mov edx, dword ptr [eax*4 + 0x79c4b8]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7980216) /* 0x79c4b8 */ + cpu.eax * 4);
    // 0048fd8e  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0048fd93  8b0d34e35500           -mov ecx, dword ptr [0x55e334]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5628724) /* 0x55e334 */);
    // 0048fd99  890d54e35500           -mov dword ptr [0x55e354], ecx
    app->getMemory<x86::reg32>(x86::reg32(5628756) /* 0x55e354 */) = cpu.ecx;
    // 0048fd9f  d9cc                   -fxch st(4)
    {
        x86::Float tmp = fp5;
        fp5 = fp1;
        fp1 = tmp;
    }
    // 0048fda1  d845d8                 -fadd dword ptr [ebp - 0x28]
    fp5 = fp::add(fp5, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-40) /* -0x28 */)));
    // 0048fda4  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp5;
        fp5 = fp4;
        fp4 = tmp;
    }
    // 0048fda6  d945e4                 -fld dword ptr [ebp - 0x1c]
    fp6 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-28) /* -0x1c */));
    // 0048fda9  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp6;
        fp6 = fp5;
        fp5 = tmp;
    }
    // 0048fdab  d8c1                   -fadd st(1)
    fp6 = fp::add(fp6, x86::Float(fp5));
    // 0048fdad  d9cc                   -fxch st(4)
    {
        x86::Float tmp = fp6;
        fp6 = fp2;
        fp2 = tmp;
    }
    // 0048fdaf  d845d8                 -fadd dword ptr [ebp - 0x28]
    fp6 = fp::add(fp6, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-40) /* -0x28 */)));
    // 0048fdb2  d9cd                   -fxch st(5)
    {
        x86::Float tmp = fp6;
        fp6 = fp1;
        fp1 = tmp;
    }
    // 0048fdb4  d8c1                   -fadd st(1)
    fp6 = fp::add(fp6, x86::Float(fp5));
    // 0048fdb6  d9ce                   -fxch st(6)
    {
        x86::Float tmp = fp6;
        fp6 = fp0;
        fp0 = tmp;
    }
    // 0048fdb8  d8c1                   -fadd st(1)
    fp6 = fp::add(fp6, x86::Float(fp5));
    // 0048fdba  d9cb                   -fxch st(3)
    {
        x86::Float tmp = fp6;
        fp6 = fp3;
        fp3 = tmp;
    }
    // 0048fdbc  dec1                   -faddp st(1)
    fp5 = fp::add(fp5, fp6);
    // 0048fdbe  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp5;
        fp5 = fp4;
        fp4 = tmp;
    }
    // 0048fdc0  d91dd8e25500           -fstp dword ptr [0x55e2d8]
    app->getMemory<float>(x86::reg32(5628632) /* 0x55e2d8 */) = float(fp5);
    // 0048fdc6  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp4;
        fp4 = fp2;
        fp2 = tmp;
    }
    // 0048fdc8  d91ddce25500           -fstp dword ptr [0x55e2dc]
    app->getMemory<float>(x86::reg32(5628636) /* 0x55e2dc */) = float(fp4);
    // 0048fdce  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp3;
        fp3 = fp1;
        fp1 = tmp;
    }
    // 0048fdd0  d91df8e25500           -fstp dword ptr [0x55e2f8]
    app->getMemory<float>(x86::reg32(5628664) /* 0x55e2f8 */) = float(fp3);
    // 0048fdd6  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 0048fdd8  d91dfce25500           -fstp dword ptr [0x55e2fc]
    app->getMemory<float>(x86::reg32(5628668) /* 0x55e2fc */) = float(fp2);
    // 0048fdde  d91d1ce35500           -fstp dword ptr [0x55e31c]
    app->getMemory<float>(x86::reg32(5628700) /* 0x55e31c */) = float(fp1);
    // 0048fde4  d91d3ce35500           -fstp dword ptr [0x55e33c]
    app->getMemory<float>(x86::reg32(5628732) /* 0x55e33c */) = float(fp0);
    // 0048fdea  d905d8e25500           -fld dword ptr [0x55e2d8]
    fp0 = x86::Float(app->getMemory<float>(x86::reg32(5628632) /* 0x55e2d8 */));
    // 0048fdf0  8b0df8e25500           -mov ecx, dword ptr [0x55e2f8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5628664) /* 0x55e2f8 */);
    // 0048fdf6  890d18e35500           -mov dword ptr [0x55e318], ecx
    app->getMemory<x86::reg32>(x86::reg32(5628696) /* 0x55e318 */) = cpu.ecx;
    // 0048fdfc  d91d38e35500           -fstp dword ptr [0x55e338]
    app->getMemory<float>(x86::reg32(5628728) /* 0x55e338 */) = float(fp0);
    // 0048fe02  e8f91afaff             -call 0x431900
    cpu.esp -= 4;
    nfs3hp::graphics::setRenderState(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0048fe07  8b460c                 -mov eax, dword ptr [esi + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(12) /* 0xc */);
    // 0048fe0a  8b0df8347d00           -mov ecx, dword ptr [0x7d34f8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205560) /* 0x7d34f8 */);
    // 0048fe10  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 0048fe15  7558                   -jne 0x48fe6f
    if (!cpu.flags.zf)
    {
        goto L_0x0048fe6f;
    }
    // 0048fe17  39c8                   +cmp eax, ecx
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
    // 0048fe19  7d54                   -jge 0x48fe6f
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0048fe6f;
    }
    // 0048fe1b  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 0048fe1e  bb00000000             -mov ebx, 0
    cpu.ebx = 0 /*0x0*/;
    // 0048fe23  8b0d04357d00           -mov ecx, dword ptr [0x7d3504]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205572) /* 0x7d3504 */);
    // 0048fe29  8b1500357d00           -mov edx, dword ptr [0x7d3500]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205568) /* 0x7d3500 */);
    // 0048fe2f  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 0048fe34  750d                   -jne 0x48fe43
    if (!cpu.flags.zf)
    {
        goto L_0x0048fe43;
    }
    // 0048fe36  39c8                   +cmp eax, ecx
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
    // 0048fe38  7c09                   -jl 0x48fe43
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0048fe43;
    }
    // 0048fe3a  39d0                   +cmp eax, edx
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
    // 0048fe3c  7e09                   -jle 0x48fe47
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0048fe47;
    }
    // 0048fe3e  83cb04                 +or ebx, 4
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(4 /*0x4*/))));
    // 0048fe41  eb04                   -jmp 0x48fe47
    goto L_0x0048fe47;
L_0x0048fe43:
    // 0048fe43  83cb08                 -or ebx, 8
    cpu.ebx |= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 0048fe46  90                     -nop 
    ;
L_0x0048fe47:
    // 0048fe47  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 0048fe49  90                     -nop 
    ;
    // 0048fe4a  8b0dfc347d00           -mov ecx, dword ptr [0x7d34fc]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205564) /* 0x7d34fc */);
    // 0048fe50  8b150c357d00           -mov edx, dword ptr [0x7d350c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205580) /* 0x7d350c */);
    // 0048fe56  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 0048fe5b  750d                   -jne 0x48fe6a
    if (!cpu.flags.zf)
    {
        goto L_0x0048fe6a;
    }
    // 0048fe5d  39c8                   +cmp eax, ecx
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
    // 0048fe5f  7c09                   -jl 0x48fe6a
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0048fe6a;
    }
    // 0048fe61  39d0                   +cmp eax, edx
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
    // 0048fe63  7e0f                   -jle 0x48fe74
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0048fe74;
    }
    // 0048fe65  83cb02                 +or ebx, 2
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(2 /*0x2*/))));
    // 0048fe68  eb0a                   -jmp 0x48fe74
    goto L_0x0048fe74;
L_0x0048fe6a:
    // 0048fe6a  83cb01                 +or ebx, 1
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(1 /*0x1*/))));
    // 0048fe6d  eb05                   -jmp 0x48fe74
    goto L_0x0048fe74;
L_0x0048fe6f:
    // 0048fe6f  bb10000000             -mov ebx, 0x10
    cpu.ebx = 16 /*0x10*/;
L_0x0048fe74:
    // 0048fe74  881f                   -mov byte ptr [edi], bl
    app->getMemory<x86::reg8>(cpu.edi) = cpu.bl;
    // 0048fe76  bf0ce35500             -mov edi, 0x55e30c
    cpu.edi = 5628684 /*0x55e30c*/;
    // 0048fe7b  bef8e25500             -mov esi, 0x55e2f8
    cpu.esi = 5628664 /*0x55e2f8*/;
    // 0048fe80  8b460c                 -mov eax, dword ptr [esi + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(12) /* 0xc */);
    // 0048fe83  8b0df8347d00           -mov ecx, dword ptr [0x7d34f8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205560) /* 0x7d34f8 */);
    // 0048fe89  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 0048fe8e  7558                   -jne 0x48fee8
    if (!cpu.flags.zf)
    {
        goto L_0x0048fee8;
    }
    // 0048fe90  39c8                   +cmp eax, ecx
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
    // 0048fe92  7d54                   -jge 0x48fee8
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0048fee8;
    }
    // 0048fe94  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 0048fe97  bb00000000             -mov ebx, 0
    cpu.ebx = 0 /*0x0*/;
    // 0048fe9c  8b0d04357d00           -mov ecx, dword ptr [0x7d3504]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205572) /* 0x7d3504 */);
    // 0048fea2  8b1500357d00           -mov edx, dword ptr [0x7d3500]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205568) /* 0x7d3500 */);
    // 0048fea8  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 0048fead  750d                   -jne 0x48febc
    if (!cpu.flags.zf)
    {
        goto L_0x0048febc;
    }
    // 0048feaf  39c8                   +cmp eax, ecx
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
    // 0048feb1  7c09                   -jl 0x48febc
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0048febc;
    }
    // 0048feb3  39d0                   +cmp eax, edx
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
    // 0048feb5  7e09                   -jle 0x48fec0
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0048fec0;
    }
    // 0048feb7  83cb04                 +or ebx, 4
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(4 /*0x4*/))));
    // 0048feba  eb04                   -jmp 0x48fec0
    goto L_0x0048fec0;
L_0x0048febc:
    // 0048febc  83cb08                 -or ebx, 8
    cpu.ebx |= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 0048febf  90                     -nop 
    ;
L_0x0048fec0:
    // 0048fec0  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 0048fec2  90                     -nop 
    ;
    // 0048fec3  8b0dfc347d00           -mov ecx, dword ptr [0x7d34fc]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205564) /* 0x7d34fc */);
    // 0048fec9  8b150c357d00           -mov edx, dword ptr [0x7d350c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205580) /* 0x7d350c */);
    // 0048fecf  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 0048fed4  750d                   -jne 0x48fee3
    if (!cpu.flags.zf)
    {
        goto L_0x0048fee3;
    }
    // 0048fed6  39c8                   +cmp eax, ecx
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
    // 0048fed8  7c09                   -jl 0x48fee3
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0048fee3;
    }
    // 0048feda  39d0                   +cmp eax, edx
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
    // 0048fedc  7e0f                   -jle 0x48feed
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0048feed;
    }
    // 0048fede  83cb02                 +or ebx, 2
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(2 /*0x2*/))));
    // 0048fee1  eb0a                   -jmp 0x48feed
    goto L_0x0048feed;
L_0x0048fee3:
    // 0048fee3  83cb01                 +or ebx, 1
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(1 /*0x1*/))));
    // 0048fee6  eb05                   -jmp 0x48feed
    goto L_0x0048feed;
L_0x0048fee8:
    // 0048fee8  bb10000000             -mov ebx, 0x10
    cpu.ebx = 16 /*0x10*/;
L_0x0048feed:
    // 0048feed  881f                   -mov byte ptr [edi], bl
    app->getMemory<x86::reg8>(cpu.edi) = cpu.bl;
    // 0048feef  bf2ce35500             -mov edi, 0x55e32c
    cpu.edi = 5628716 /*0x55e32c*/;
    // 0048fef4  be18e35500             -mov esi, 0x55e318
    cpu.esi = 5628696 /*0x55e318*/;
    // 0048fef9  8b460c                 -mov eax, dword ptr [esi + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(12) /* 0xc */);
    // 0048fefc  8b0df8347d00           -mov ecx, dword ptr [0x7d34f8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205560) /* 0x7d34f8 */);
    // 0048ff02  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 0048ff07  7558                   -jne 0x48ff61
    if (!cpu.flags.zf)
    {
        goto L_0x0048ff61;
    }
    // 0048ff09  39c8                   +cmp eax, ecx
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
    // 0048ff0b  7d54                   -jge 0x48ff61
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0048ff61;
    }
    // 0048ff0d  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 0048ff10  bb00000000             -mov ebx, 0
    cpu.ebx = 0 /*0x0*/;
    // 0048ff15  8b0d04357d00           -mov ecx, dword ptr [0x7d3504]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205572) /* 0x7d3504 */);
    // 0048ff1b  8b1500357d00           -mov edx, dword ptr [0x7d3500]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205568) /* 0x7d3500 */);
    // 0048ff21  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 0048ff26  750d                   -jne 0x48ff35
    if (!cpu.flags.zf)
    {
        goto L_0x0048ff35;
    }
    // 0048ff28  39c8                   +cmp eax, ecx
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
    // 0048ff2a  7c09                   -jl 0x48ff35
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0048ff35;
    }
    // 0048ff2c  39d0                   +cmp eax, edx
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
    // 0048ff2e  7e09                   -jle 0x48ff39
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0048ff39;
    }
    // 0048ff30  83cb04                 +or ebx, 4
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(4 /*0x4*/))));
    // 0048ff33  eb04                   -jmp 0x48ff39
    goto L_0x0048ff39;
L_0x0048ff35:
    // 0048ff35  83cb08                 -or ebx, 8
    cpu.ebx |= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 0048ff38  90                     -nop 
    ;
L_0x0048ff39:
    // 0048ff39  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 0048ff3b  90                     -nop 
    ;
    // 0048ff3c  8b0dfc347d00           -mov ecx, dword ptr [0x7d34fc]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205564) /* 0x7d34fc */);
    // 0048ff42  8b150c357d00           -mov edx, dword ptr [0x7d350c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205580) /* 0x7d350c */);
    // 0048ff48  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 0048ff4d  750d                   -jne 0x48ff5c
    if (!cpu.flags.zf)
    {
        goto L_0x0048ff5c;
    }
    // 0048ff4f  39c8                   +cmp eax, ecx
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
    // 0048ff51  7c09                   -jl 0x48ff5c
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0048ff5c;
    }
    // 0048ff53  39d0                   +cmp eax, edx
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
    // 0048ff55  7e0f                   -jle 0x48ff66
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0048ff66;
    }
    // 0048ff57  83cb02                 +or ebx, 2
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(2 /*0x2*/))));
    // 0048ff5a  eb0a                   -jmp 0x48ff66
    goto L_0x0048ff66;
L_0x0048ff5c:
    // 0048ff5c  83cb01                 +or ebx, 1
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(1 /*0x1*/))));
    // 0048ff5f  eb05                   -jmp 0x48ff66
    goto L_0x0048ff66;
L_0x0048ff61:
    // 0048ff61  bb10000000             -mov ebx, 0x10
    cpu.ebx = 16 /*0x10*/;
L_0x0048ff66:
    // 0048ff66  881f                   -mov byte ptr [edi], bl
    app->getMemory<x86::reg8>(cpu.edi) = cpu.bl;
    // 0048ff68  bf4ce35500             -mov edi, 0x55e34c
    cpu.edi = 5628748 /*0x55e34c*/;
    // 0048ff6d  be38e35500             -mov esi, 0x55e338
    cpu.esi = 5628728 /*0x55e338*/;
    // 0048ff72  8b460c                 -mov eax, dword ptr [esi + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(12) /* 0xc */);
    // 0048ff75  8b0df8347d00           -mov ecx, dword ptr [0x7d34f8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205560) /* 0x7d34f8 */);
    // 0048ff7b  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 0048ff80  7558                   -jne 0x48ffda
    if (!cpu.flags.zf)
    {
        goto L_0x0048ffda;
    }
    // 0048ff82  39c8                   +cmp eax, ecx
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
    // 0048ff84  7d54                   -jge 0x48ffda
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0048ffda;
    }
    // 0048ff86  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 0048ff89  bb00000000             -mov ebx, 0
    cpu.ebx = 0 /*0x0*/;
    // 0048ff8e  8b0d04357d00           -mov ecx, dword ptr [0x7d3504]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205572) /* 0x7d3504 */);
    // 0048ff94  8b1500357d00           -mov edx, dword ptr [0x7d3500]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205568) /* 0x7d3500 */);
    // 0048ff9a  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 0048ff9f  750d                   -jne 0x48ffae
    if (!cpu.flags.zf)
    {
        goto L_0x0048ffae;
    }
    // 0048ffa1  39c8                   +cmp eax, ecx
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
    // 0048ffa3  7c09                   -jl 0x48ffae
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0048ffae;
    }
    // 0048ffa5  39d0                   +cmp eax, edx
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
    // 0048ffa7  7e09                   -jle 0x48ffb2
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0048ffb2;
    }
    // 0048ffa9  83cb04                 +or ebx, 4
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(4 /*0x4*/))));
    // 0048ffac  eb04                   -jmp 0x48ffb2
    goto L_0x0048ffb2;
L_0x0048ffae:
    // 0048ffae  83cb08                 -or ebx, 8
    cpu.ebx |= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 0048ffb1  90                     -nop 
    ;
L_0x0048ffb2:
    // 0048ffb2  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 0048ffb4  90                     -nop 
    ;
    // 0048ffb5  8b0dfc347d00           -mov ecx, dword ptr [0x7d34fc]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205564) /* 0x7d34fc */);
    // 0048ffbb  8b150c357d00           -mov edx, dword ptr [0x7d350c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205580) /* 0x7d350c */);
    // 0048ffc1  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 0048ffc6  750d                   -jne 0x48ffd5
    if (!cpu.flags.zf)
    {
        goto L_0x0048ffd5;
    }
    // 0048ffc8  39c8                   +cmp eax, ecx
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
    // 0048ffca  7c09                   -jl 0x48ffd5
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0048ffd5;
    }
    // 0048ffcc  39d0                   +cmp eax, edx
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
    // 0048ffce  7e0f                   -jle 0x48ffdf
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0048ffdf;
    }
    // 0048ffd0  83cb02                 +or ebx, 2
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(2 /*0x2*/))));
    // 0048ffd3  eb0a                   -jmp 0x48ffdf
    goto L_0x0048ffdf;
L_0x0048ffd5:
    // 0048ffd5  83cb01                 +or ebx, 1
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(1 /*0x1*/))));
    // 0048ffd8  eb05                   -jmp 0x48ffdf
    goto L_0x0048ffdf;
L_0x0048ffda:
    // 0048ffda  bb10000000             -mov ebx, 0x10
    cpu.ebx = 16 /*0x10*/;
L_0x0048ffdf:
    // 0048ffdf  881f                   -mov byte ptr [edi], bl
    app->getMemory<x86::reg8>(cpu.edi) = cpu.bl;
    // 0048ffe1  b938e35500             -mov ecx, 0x55e338
    cpu.ecx = 5628728 /*0x55e338*/;
    // 0048ffe6  bb18e35500             -mov ebx, 0x55e318
    cpu.ebx = 5628696 /*0x55e318*/;
    // 0048ffeb  baf8e25500             -mov edx, 0x55e2f8
    cpu.edx = 5628664 /*0x55e2f8*/;
    // 0048fff0  b8d8e25500             -mov eax, 0x55e2d8
    cpu.eax = 5628632 /*0x55e2d8*/;
    // 0048fff5  e8e6120300             -call 0x4c12e0
    cpu.esp -= 4;
    nfs3hp::vertices::clipQuad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0048fffa:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0048fffa  ff45ec                 -inc dword ptr [ebp - 0x14]
    (app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */))++;
    // 0048fffd  837dec10               +cmp dword ptr [ebp - 0x14], 0x10
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(16 /*0x10*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00490001  7d18                   -jge 0x49001b
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0049001b;
    }
L_0x00490003:
    // 00490003  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00490006  8b55c8                 -mov edx, dword ptr [ebp - 0x38]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-56) /* -0x38 */);
    // 00490009  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 0049000c  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 0049000e  3d70ffffff             +cmp eax, 0xffffff70
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4294967152 /*0xffffff70*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00490013  0f8d02fcffff           -jge 0x48fc1b
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0048fc1b;
    }
    // 00490019  ebdf                   -jmp 0x48fffa
    goto L_0x0048fffa;
L_0x0049001b:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049001b  ff45f0                 -inc dword ptr [ebp - 0x10]
    (app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */))++;
    // 0049001e  837df010               +cmp dword ptr [ebp - 0x10], 0x10
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(16 /*0x10*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00490022  7d19                   -jge 0x49003d
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0049003d;
    }
L_0x00490024:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00490024  8b45f0                 -mov eax, dword ptr [ebp - 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00490027  8b55c4                 -mov edx, dword ptr [ebp - 0x3c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-60) /* -0x3c */);
    // 0049002a  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 0049002d  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0049002f  8955e0                 -mov dword ptr [ebp - 0x20], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */) = cpu.edx;
    // 00490032  83fa90                 +cmp edx, -0x70
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(-112 /*-0x70*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00490035  0f8dcdfbffff           -jge 0x48fc08
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0048fc08;
    }
    // 0049003b  ebde                   -jmp 0x49001b
    goto L_0x0049001b;
L_0x0049003d:
    // 0049003d  ff45f4                 -inc dword ptr [ebp - 0xc]
    (app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */))++;
    // 00490040  837df404               +cmp dword ptr [ebp - 0xc], 4
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00490044  7d1f                   -jge 0x490065
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00490065;
    }
L_0x00490046:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00490046  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00490049  8b5510                 -mov edx, dword ptr [ebp + 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(16) /* 0x10 */);
    // 0049004c  c1e008                 -shl eax, 8
    cpu.eax <<= 8 /*0x8*/ % 32;
    // 0049004f  0302                   -add eax, dword ptr [edx]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.edx)));
    // 00490051  25ff030000             -and eax, 0x3ff
    cpu.eax &= x86::reg32(x86::sreg32(1023 /*0x3ff*/));
    // 00490056  2d80010000             -sub eax, 0x180
    (cpu.eax) -= x86::reg32(x86::sreg32(384 /*0x180*/));
    // 0049005b  8945c8                 -mov dword ptr [ebp - 0x38], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-56) /* -0x38 */) = cpu.eax;
    // 0049005e  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 00490060  8945f0                 -mov dword ptr [ebp - 0x10], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.eax;
    // 00490063  ebbf                   -jmp 0x490024
    goto L_0x00490024;
L_0x00490065:
    // 00490065  ff45e8                 -inc dword ptr [ebp - 0x18]
    (app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */))++;
    // 00490068  837de801               +cmp dword ptr [ebp - 0x18], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049006c  7d1a                   -jge 0x490088
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00490088;
    }
L_0x0049006e:
    // 0049006e  8b45e8                 -mov eax, dword ptr [ebp - 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 00490071  8b5514                 -mov edx, dword ptr [ebp + 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    // 00490074  c1e008                 -shl eax, 8
    cpu.eax <<= 8 /*0x8*/ % 32;
    // 00490077  0302                   -add eax, dword ptr [edx]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.edx)));
    // 00490079  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0049007b  2d80000000             +sub eax, 0x80
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(128 /*0x80*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00490080  895df4                 -mov dword ptr [ebp - 0xc], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.ebx;
    // 00490083  8945c4                 -mov dword ptr [ebp - 0x3c], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-60) /* -0x3c */) = cpu.eax;
    // 00490086  ebbe                   -jmp 0x490046
    goto L_0x00490046;
L_0x00490088:
    // 00490088  8b4da4                 -mov ecx, dword ptr [ebp - 0x5c]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-92) /* -0x5c */);
    // 0049008b  8b5da0                 -mov ebx, dword ptr [ebp - 0x60]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-96) /* -0x60 */);
    // 0049008e  8b559c                 -mov edx, dword ptr [ebp - 0x64]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-100) /* -0x64 */);
    // 00490091  8b4598                 -mov eax, dword ptr [ebp - 0x68]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-104) /* -0x68 */);
    // 00490094  e847eb0200             -call 0x4bebe0
    cpu.esp -= 4;
    nfs3hp::graphics::setClip(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00490099  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0049009b  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049009c  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049009d  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049009e  c20c00                 -ret 0xc
    cpu.esp += 4+12 /*0xc*/;
    return;
}

/* sub_4900b0: native, nfs3hp::dead::stub_0x4900b0 */

/* sub_490110: native, nfs3hp::dead::stub_0x490110 */

/* sub_490140: native, nfs3hp::dead::stub_0x490140 */

/* sub_4902f0: native, nfs3hp::dead::stub_0x4902f0 */

/* align: skip 0x00 0x00 0x00 0x00 */
void Application::sub_490430(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00490430  044d                   -add al, 0x4d
    (cpu.al) += x86::reg8(x86::sreg8(77 /*0x4d*/));
    // 00490432  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00490434  044b                   -add al, 0x4b
    (cpu.al) += x86::reg8(x86::sreg8(75 /*0x4b*/));
    // 00490436  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00490438  0448                   -add al, 0x48
    (cpu.al) += x86::reg8(x86::sreg8(72 /*0x48*/));
    // 0049043a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0049043c  0450                   -add al, 0x50
    (cpu.al) += x86::reg8(x86::sreg8(80 /*0x50*/));
    // 0049043e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00490440  0439                   -add al, 0x39
    (cpu.al) += x86::reg8(x86::sreg8(57 /*0x39*/));
    // 00490442  0020                   -add byte ptr [eax], ah
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.ah));
    // 00490444  041e                   -add al, 0x1e
    (cpu.al) += x86::reg8(x86::sreg8(30 /*0x1e*/));
    // 00490446  004104                 -add byte ptr [ecx + 4], al
    (app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(4) /* 0x4 */)) += x86::reg8(x86::sreg8(cpu.al));
    // 00490449  2c00                   -sub al, 0
    (cpu.al) -= x86::reg8(x86::sreg8(0 /*0x0*/));
    // 0049044b  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049044c  042e                   -add al, 0x2e
    (cpu.al) += x86::reg8(x86::sreg8(46 /*0x2e*/));
    // 0049044e  004304                 -add byte ptr [ebx + 4], al
    (app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(4) /* 0x4 */)) += x86::reg8(x86::sreg8(cpu.al));
    // 00490451  2300                   -and eax, dword ptr [eax]
    cpu.eax &= x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax)));
    // 00490453  48                     -dec eax
    (cpu.eax)--;
    // 00490454  0430                   -add al, 0x30
    (cpu.al) += x86::reg8(x86::sreg8(48 /*0x30*/));
    // 00490456  004204                 +add byte ptr [edx + 4], al
    {
        auto tmp1 = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(4) /* 0x4 */);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(cpu.al));
        x86::reg8 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) == (1 & (tmp2 >> 7));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00490459  1300                   -adc eax, dword ptr [eax]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax)) + cpu.flags.cf);
    // 0049045b  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049045c  041f                   -add al, 0x1f
    (cpu.al) += x86::reg8(x86::sreg8(31 /*0x1f*/));
    // 0049045e  005304                 -add byte ptr [ebx + 4], dl
    (app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(4) /* 0x4 */)) += x86::reg8(x86::sreg8(cpu.dl));
    // 00490461  26004c044d             -add byte ptr es:[esp + eax + 0x4d], cl
    (app->getMemory<x86::reg8>(cpu.ees + cpu.esp + x86::reg32(77) /* 0x4d */ + cpu.eax * 1)) += x86::reg8(x86::sreg8(cpu.cl));
    // 00490466  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00490468  044b                   -add al, 0x4b
    (cpu.al) += x86::reg8(x86::sreg8(75 /*0x4b*/));
    // 0049046a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0049046c  0448                   -add al, 0x48
    (cpu.al) += x86::reg8(x86::sreg8(72 /*0x48*/));
    // 0049046e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00490470  0450                   -add al, 0x50
    (cpu.al) += x86::reg8(x86::sreg8(80 /*0x50*/));
    // 00490472  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00490474  0452                   -add al, 0x52
    (cpu.al) += x86::reg8(x86::sreg8(82 /*0x52*/));
    // 00490476  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00490478  0449                   -add al, 0x49
    (cpu.al) += x86::reg8(x86::sreg8(73 /*0x49*/));
    // 0049047a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0049047c  0451                   -add al, 0x51
    (cpu.al) += x86::reg8(x86::sreg8(81 /*0x51*/));
    // 0049047e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00490480  0425                   -add al, 0x25
    (cpu.al) += x86::reg8(x86::sreg8(37 /*0x25*/));
    // 00490482  004b04                 -add byte ptr [ebx + 4], cl
    (app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(4) /* 0x4 */)) += x86::reg8(x86::sreg8(cpu.cl));
    // 00490485  3200                   -xor al, byte ptr [eax]
    cpu.al ^= x86::reg8(x86::sreg8(app->getMemory<x86::reg8>(cpu.eax)));
    // 00490487  4d                     -dec ebp
    (cpu.ebp)--;
    // 00490488  0447                   -add al, 0x47
    (cpu.al) += x86::reg8(x86::sreg8(71 /*0x47*/));
    // 0049048a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0049048c  044f                   -add al, 0x4f
    (cpu.al) += x86::reg8(x86::sreg8(79 /*0x4f*/));
    // 0049048e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00490490  0419                   -add al, 0x19
    (cpu.al) += x86::reg8(x86::sreg8(25 /*0x19*/));
    // 00490492  005004                 -add byte ptr [eax + 4], dl
    (app->getMemory<x86::reg8>(cpu.eax + x86::reg32(4) /* 0x4 */)) += x86::reg8(x86::sreg8(cpu.dl));
    // 00490495  26004c0422             -add byte ptr es:[esp + eax + 0x22], cl
    (app->getMemory<x86::reg8>(cpu.ees + cpu.esp + x86::reg32(34) /* 0x22 */ + cpu.eax * 1)) += x86::reg8(x86::sreg8(cpu.cl));
    // 0049049a  004704                 -add byte ptr [edi + 4], al
    (app->getMemory<x86::reg8>(cpu.edi + x86::reg32(4) /* 0x4 */)) += x86::reg8(x86::sreg8(cpu.al));
    // 0049049d  2000                   -and byte ptr [eax], al
    app->getMemory<x86::reg8>(cpu.eax) &= x86::reg8(x86::sreg8(cpu.al));
    // 0049049f  44                     -inc esp
    (cpu.esp)++;
    // 004904a0  0413                   -add al, 0x13
    (cpu.al) += x86::reg8(x86::sreg8(19 /*0x13*/));
    // 004904a2  005204                 -add byte ptr [edx + 4], dl
    (app->getMemory<x86::reg8>(cpu.edx + x86::reg32(4) /* 0x4 */)) += x86::reg8(x86::sreg8(cpu.dl));
    // 004904a5  2100                   -and dword ptr [eax], eax
    app->getMemory<x86::reg32>(cpu.eax) &= x86::reg32(x86::sreg32(cpu.eax));
    // 004904a7  46                     -inc esi
    (cpu.esi)++;
    // 004904a8  0439                   -add al, 0x39
    (cpu.al) += x86::reg8(x86::sreg8(57 /*0x39*/));
    // 004904aa  0020                   -add byte ptr [eax], ah
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.ah));
    // 004904ac  041e                   -add al, 0x1e
    (cpu.al) += x86::reg8(x86::sreg8(30 /*0x1e*/));
    // 004904ae  004104                 -add byte ptr [ecx + 4], al
    (app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(4) /* 0x4 */)) += x86::reg8(x86::sreg8(cpu.al));
    // 004904b1  2c00                   -sub al, 0
    (cpu.al) -= x86::reg8(x86::sreg8(0 /*0x0*/));
    // 004904b3  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004904b4  0410                   -add al, 0x10
    (cpu.al) += x86::reg8(x86::sreg8(16 /*0x10*/));
    // 004904b6  005104                 +add byte ptr [ecx + 4], dl
    {
        auto tmp1 = app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(4) /* 0x4 */);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(cpu.dl));
        x86::reg8 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) == (1 & (tmp2 >> 7));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004904b9  1100                   -adc dword ptr [eax], eax
    (app->getMemory<x86::reg32>(cpu.eax)) += x86::reg32(x86::sreg32(cpu.eax) + cpu.flags.cf);
    // 004904bb  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004904bc  0412                   -add al, 0x12
    (cpu.al) += x86::reg8(x86::sreg8(18 /*0x12*/));
    // 004904be  004504                 -add byte ptr [ebp + 4], al
    (app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(4) /* 0x4 */)) += x86::reg8(x86::sreg8(cpu.al));
    // 004904c1  2d0058041f             -sub eax, 0x1f045800
    (cpu.eax) -= x86::reg32(x86::sreg32(520378368 /*0x1f045800*/));
    // 004904c6  005304                 +add byte ptr [ebx + 4], dl
    {
        auto tmp1 = app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(4) /* 0x4 */);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(cpu.dl));
        x86::reg8 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) == (1 & (tmp2 >> 7));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004904c9  1500598d44             -adc eax, 0x448d5900
    (cpu.eax) += x86::reg32(x86::sreg32(1150114048 /*0x448d5900*/) + cpu.flags.cf);
    // 004904ce  2000                   -and byte ptr [eax], al
    app->getMemory<x86::reg8>(cpu.eax) &= x86::reg8(x86::sreg8(cpu.al));
    // 004904d0  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004904d1  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004904d3  83f801                 +cmp eax, 1
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
    // 004904d6  7507                   -jne 0x4904df
    if (!cpu.flags.zf)
    {
        goto L_0x004904df;
    }
    // 004904d8  b840d26f00             -mov eax, 0x6fd240
    cpu.eax = 7328320 /*0x6fd240*/;
    // 004904dd  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004904de  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004904df:
    // 004904df  b80cd26f00             -mov eax, 0x6fd20c
    cpu.eax = 7328268 /*0x6fd20c*/;
    // 004904e4  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004904e5  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4904d0: native, nfs3hp::controls::controlsFor */

/* sub_4904f0: native, nfs3hp::controls::defaultControls */

/* sub_4905b0: native, nfs3hp::controls::setControls */

/* sub_490610: native, nfs3hp::controls::controllerNamed */

/* sub_490640: native, nfs3hp::controls::knownControllerControls */

/* sub_4906b0: native, nfs3hp::controls::sideWinderGamePadControls */

/* sub_490770: native, nfs3hp::controls::gravisGripControls */

/* sub_490830: native, nfs3hp::controls::wingManForceControls */

/* sub_4908f0: native, nfs3hp::controls::combatStickControls */

/* sub_4909b0: native, nfs3hp::controls::sideWinderProControls */

/* sub_490a70: native, nfs3hp::controls::sideWinderWheelControls */

/* sub_490b30: native, nfs3hp::controls::steeringWheelControls */

/* sub_490c40: native, nfs3hp::controls::keyboardControls */

/* sub_490cb0: native, nfs3hp::controls::gamePadControls */

/* sub_490e30: native, nfs3hp::controls::joystickControls */

/* sub_490fa0: native, nfs3hp::controls::otherControllerControls */

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
/* data blob: 000000000000803f000000008d442000 */
void Application::sub_491120(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00491120  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00491121  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00491122  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00491123  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00491124  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00491126  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00491128  833d30bc6f0002         +cmp dword ptr [0x6fbc30], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7322672) /* 0x6fbc30 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049112f  7507                   -jne 0x491138
    if (!cpu.flags.zf)
    {
        goto L_0x00491138;
    }
L_0x00491131:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00491131  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00491133  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00491134  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00491135  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00491136  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00491137  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00491138:
    // 00491138  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 0049113b  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0049113d  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00491140  6683b8e0e3550000       +cmp word ptr [eax + 0x55e3e0], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(5628896) /* 0x55e3e0 */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 00491148  7425                   -je 0x49116f
    if (cpu.flags.zf)
    {
        goto L_0x0049116f;
    }
    // 0049114a  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0049114c  8b1d84367d00           -mov ebx, dword ptr [0x7d3684]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(8205956) /* 0x7d3684 */);
    // 00491152  8a88e3e35500           -mov cl, byte ptr [eax + 0x55e3e3]
    cpu.cl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5628899) /* 0x55e3e3 */);
    // 00491158  8a80e2e35500           -mov al, byte ptr [eax + 0x55e3e2]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5628898) /* 0x55e3e2 */);
    // 0049115e  01cb                   -add ebx, ecx
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.ecx));
    // 00491160  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 00491165  88c1                   -mov cl, al
    cpu.cl = cpu.al;
    // 00491167  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 00491169  d3f8                   -sar eax, cl
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (cpu.cl % 32));
    // 0049116b  a801                   +test al, 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.al & 1 /*0x1*/));
    // 0049116d  75c2                   -jne 0x491131
    if (!cpu.flags.zf)
    {
        goto L_0x00491131;
    }
L_0x0049116f:
    // 0049116f  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00491176  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00491178  8b0485dce35500         -mov eax, dword ptr [eax*4 + 0x55e3dc]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5628892) /* 0x55e3dc */ + cpu.eax * 4);
    // 0049117f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00491180  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00491181  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00491182  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00491183  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_491190: native, nfs3hp::render::glow */

/* sub_491bc0: native, nfs3hp::render::roadLight */

/* sub_492370: native, nfs3hp::vertices::lightGlow */

/* sub_492980: native, nfs3hp::vertices::headlightBeam */

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
/* data blob: 000000000000a0be000000008d442000 */
void Application::sub_493380(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00493380  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00493381  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00493382  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00493383  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00493384  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00493385  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00493387  8b3548d95d00           -mov esi, dword ptr [0x5dd948]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(6150472) /* 0x5dd948 */);
    // 0049338d  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0049338f  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00493391  89cb                   -mov ebx, ecx
    cpu.ebx = cpu.ecx;
    // 00493393  8d14dd00000000         -lea edx, [ebx*8]
    cpu.edx = x86::reg32(cpu.ebx * 8);
    // 0049339a  01da                   -add edx, ebx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.ebx));
    // 0049339c  c1e202                 -shl edx, 2
    cpu.edx <<= 2 /*0x2*/ % 32;
    // 0049339f  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 004933a1  01f2                   -add edx, esi
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.esi));
    // 004933a3  31f3                   -xor ebx, esi
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.esi));
    // 004933a5  8a5a13                 -mov bl, byte ptr [edx + 0x13]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(19) /* 0x13 */);
    // 004933a8  c1fb04                 -sar ebx, 4
    cpu.ebx = x86::reg32(x86::sreg32(cpu.ebx) >> (4 /*0x4*/ % 32));
    // 004933ab  bf06000000             -mov edi, 6
    cpu.edi = 6 /*0x6*/;
    // 004933b0  83e30f                 -and ebx, 0xf
    cpu.ebx &= x86::reg32(x86::sreg32(15 /*0xf*/));
    // 004933b3  29df                   -sub edi, ebx
    (cpu.edi) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 004933b5  39f8                   +cmp eax, edi
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
    // 004933b7  7413                   -je 0x4933cc
    if (cpu.flags.zf)
    {
        goto L_0x004933cc;
    }
    // 004933b9  8a5a13                 -mov bl, byte ptr [edx + 0x13]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(19) /* 0x13 */);
    // 004933bc  80e30f                 -and bl, 0xf
    cpu.bl &= x86::reg8(x86::sreg8(15 /*0xf*/));
    // 004933bf  81e3ff000000           -and ebx, 0xff
    cpu.ebx &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 004933c5  83c307                 -add ebx, 7
    (cpu.ebx) += x86::reg32(x86::sreg32(7 /*0x7*/));
    // 004933c8  39d8                   +cmp eax, ebx
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
    // 004933ca  7552                   -jne 0x49341e
    if (!cpu.flags.zf)
    {
        goto L_0x0049341e;
    }
L_0x004933cc:
    // 004933cc  8d14cd00000000         -lea edx, [ecx*8]
    cpu.edx = x86::reg32(cpu.ecx * 8);
    // 004933d3  01ca                   -add edx, ecx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004933d5  c1e202                 -shl edx, 2
    cpu.edx <<= 2 /*0x2*/ % 32;
    // 004933d8  83f807                 +cmp eax, 7
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(7 /*0x7*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004933db  7d25                   -jge 0x493402
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00493402;
    }
    // 004933dd  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 004933df  8a541a17               -mov dl, byte ptr [edx + ebx + 0x17]
    cpu.dl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(23) /* 0x17 */ + cpu.ebx * 1);
    // 004933e3  bb07000000             -mov ebx, 7
    cpu.ebx = 7 /*0x7*/;
    // 004933e8  29c3                   -sub ebx, eax
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 004933ea  81e2ff000000           -and edx, 0xff
    cpu.edx &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 004933f0  8d43ff                 -lea eax, [ebx - 1]
    cpu.eax = x86::reg32(cpu.ebx + x86::reg32(-1) /* -0x1 */);
    // 004933f3  c1e20f                 -shl edx, 0xf
    cpu.edx <<= 15 /*0xf*/ % 32;
    // 004933f6  f7d8                   -neg eax
    cpu.eax = ~cpu.eax + 1;
    // 004933f8  0fafc2                 -imul eax, edx
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 004933fb  2d00000200             +sub eax, 0x20000
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(131072 /*0x20000*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00493400  eb68                   -jmp 0x49346a
    goto L_0x0049346a;
L_0x00493402:
    // 00493402  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 00493404  8a541a1b               -mov dl, byte ptr [edx + ebx + 0x1b]
    cpu.dl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(27) /* 0x1b */ + cpu.ebx * 1);
    // 00493408  81e2ff000000           -and edx, 0xff
    cpu.edx &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 0049340e  83e807                 -sub eax, 7
    (cpu.eax) -= x86::reg32(x86::sreg32(7 /*0x7*/));
    // 00493411  c1e20f                 -shl edx, 0xf
    cpu.edx <<= 15 /*0xf*/ % 32;
    // 00493414  0fafc2                 -imul eax, edx
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 00493417  0500000200             +add eax, 0x20000
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(131072 /*0x20000*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0049341c  eb4c                   -jmp 0x49346a
    goto L_0x0049346a;
L_0x0049341e:
    // 0049341e  83f807                 +cmp eax, 7
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(7 /*0x7*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00493421  7d28                   -jge 0x49344b
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0049344b;
    }
    // 00493423  8a5217                 -mov dl, byte ptr [edx + 0x17]
    cpu.dl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(23) /* 0x17 */);
    // 00493426  bb07000000             -mov ebx, 7
    cpu.ebx = 7 /*0x7*/;
    // 0049342b  29c3                   -sub ebx, eax
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 0049342d  81e2ff000000           -and edx, 0xff
    cpu.edx &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 00493433  4b                     -dec ebx
    (cpu.ebx)--;
    // 00493434  c1e20f                 -shl edx, 0xf
    cpu.edx <<= 15 /*0xf*/ % 32;
    // 00493437  f7db                   -neg ebx
    cpu.ebx = ~cpu.ebx + 1;
    // 00493439  0fafda                 -imul ebx, edx
    cpu.ebx = x86::reg32(x86::sreg64(x86::sreg32(cpu.ebx)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 0049343c  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0049343e  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00493441  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00493443  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 00493445  29c3                   +sub ebx, eax
    {
        x86::reg32& tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00493447  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 00493449  eb1f                   -jmp 0x49346a
    goto L_0x0049346a;
L_0x0049344b:
    // 0049344b  8a521b                 -mov dl, byte ptr [edx + 0x1b]
    cpu.dl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(27) /* 0x1b */);
    // 0049344e  81e2ff000000           -and edx, 0xff
    cpu.edx &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 00493454  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00493456  c1e20f                 -shl edx, 0xf
    cpu.edx <<= 15 /*0xf*/ % 32;
    // 00493459  83eb07                 -sub ebx, 7
    (cpu.ebx) -= x86::reg32(x86::sreg32(7 /*0x7*/));
    // 0049345c  0fafda                 -imul ebx, edx
    cpu.ebx = x86::reg32(x86::sreg64(x86::sreg32(cpu.ebx)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 0049345f  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00493461  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00493464  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00493466  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 00493468  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
L_0x0049346a:
    // 0049346a  8d14cd00000000         -lea edx, [ecx*8]
    cpu.edx = x86::reg32(cpu.ecx * 8);
    // 00493471  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 00493473  01ca                   -add edx, ecx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.ecx));
    // 00493475  8b549320               -mov edx, dword ptr [ebx + edx*4 + 0x20]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(32) /* 0x20 */ + cpu.edx * 4);
    // 00493479  81ea00000200           -sub edx, 0x20000
    (cpu.edx) -= x86::reg32(x86::sreg32(131072 /*0x20000*/));
    // 0049347f  39d0                   +cmp eax, edx
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
    // 00493481  7e02                   -jle 0x493485
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00493485;
    }
    // 00493483  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
L_0x00493485:
    // 00493485  8d14cd00000000         -lea edx, [ecx*8]
    cpu.edx = x86::reg32(cpu.ecx * 8);
    // 0049348c  01d1                   -add ecx, edx
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.edx));
    // 0049348e  c1e102                 -shl ecx, 2
    cpu.ecx <<= 2 /*0x2*/ % 32;
    // 00493491  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 00493493  8b54111c               -mov edx, dword ptr [ecx + edx + 0x1c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(28) /* 0x1c */ + cpu.edx * 1);
    // 00493497  f7da                   -neg edx
    cpu.edx = ~cpu.edx + 1;
    // 00493499  81c200000200           -add edx, 0x20000
    (cpu.edx) += x86::reg32(x86::sreg32(131072 /*0x20000*/));
    // 0049349f  39d0                   +cmp eax, edx
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
    // 004934a1  7d02                   -jge 0x4934a5
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004934a5;
    }
    // 004934a3  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
L_0x004934a5:
    // 004934a5  893548d95d00           -mov dword ptr [0x5dd948], esi
    app->getMemory<x86::reg32>(x86::reg32(6150472) /* 0x5dd948 */) = cpu.esi;
    // 004934ab  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004934ac  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004934ad  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004934ae  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004934af  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004934b0  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_4934c0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1, fp2, fp3;
    // 004934c0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004934c1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004934c2  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004934c3  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004934c4  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004934c6  83ec38                 -sub esp, 0x38
    (cpu.esp) -= x86::reg32(x86::sreg32(56 /*0x38*/));
    // 004934c9  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 004934cb  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004934cd  8955f8                 -mov dword ptr [ebp - 8], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.edx;
    // 004934d0  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 004934d3  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004934d5  8b3548d95d00           -mov esi, dword ptr [0x5dd948]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(6150472) /* 0x5dd948 */);
    // 004934db  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004934de  01f0                   -add eax, esi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.esi));
    // 004934e0  8d7dd4                 -lea edi, [ebp - 0x2c]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-44) /* -0x2c */);
    // 004934e3  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004934e5  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004934e6  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004934e7  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004934e8  8b7015                 -mov esi, dword ptr [eax + 0x15]
    cpu.esi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(21) /* 0x15 */);
    // 004934eb  c1fe18                 -sar esi, 0x18
    cpu.esi = x86::reg32(x86::sreg32(cpu.esi) >> (24 /*0x18*/ % 32));
    // 004934ee  c1e609                 -shl esi, 9
    cpu.esi <<= 9 /*0x9*/ % 32;
    // 004934f1  8975ec                 -mov dword ptr [ebp - 0x14], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.esi;
    // 004934f4  8b7016                 -mov esi, dword ptr [eax + 0x16]
    cpu.esi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(22) /* 0x16 */);
    // 004934f7  8b4017                 -mov eax, dword ptr [eax + 0x17]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(23) /* 0x17 */);
    // 004934fa  c1f818                 -sar eax, 0x18
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (24 /*0x18*/ % 32));
    // 004934fd  c1e009                 -shl eax, 9
    cpu.eax <<= 9 /*0x9*/ % 32;
    // 00493500  c1fe18                 -sar esi, 0x18
    cpu.esi = x86::reg32(x86::sreg32(cpu.esi) >> (24 /*0x18*/ % 32));
    // 00493503  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
    // 00493506  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00493508  c1e609                 -shl esi, 9
    cpu.esi <<= 9 /*0x9*/ % 32;
    // 0049350b  e8b0abf7ff             -call 0x40e0c0
    cpu.esp -= 4;
    nfs3hp::aiplan::roadKindAt(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00493510  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00493512  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00493514  8975f0                 -mov dword ptr [ebp - 0x10], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.esi;
    // 00493517  e864feffff             -call 0x493380
    cpu.esp -= 4;
    sub_493380(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049351c  8b55ec                 -mov edx, dword ptr [ebp - 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0049351f  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00493521  f7ea                   -imul edx
    cpu.edx_eax = x86::reg64(x86::sreg64(static_cast<x86::sreg32>(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 00493523  c1e210                 -shl edx, 0x10
    cpu.edx <<= 16 /*0x10*/ % 32;
    // 00493526  c1e810                 +shr eax, 0x10
    {
        x86::reg8 tmp = 16 /*0x10*/ % 32;
        x86::reg32& op = cpu.eax;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = (1 & (op >> (32 - 1)));
            cpu.set_szp((op >>= tmp));
        }
    }
    // 00493529  11d0                   -adc eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 0049352b  8b55f0                 -mov edx, dword ptr [ebp - 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0049352e  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00493530  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00493532  f7ea                   -imul edx
    cpu.edx_eax = x86::reg64(x86::sreg64(static_cast<x86::sreg32>(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 00493534  c1e210                 -shl edx, 0x10
    cpu.edx <<= 16 /*0x10*/ % 32;
    // 00493537  c1e810                 +shr eax, 0x10
    {
        x86::reg8 tmp = 16 /*0x10*/ % 32;
        x86::reg32& op = cpu.eax;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = (1 & (op >> (32 - 1)));
            cpu.set_szp((op >>= tmp));
        }
    }
    // 0049353a  11d0                   -adc eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 0049353c  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0049353e  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00493540  8b75d4                 -mov esi, dword ptr [ebp - 0x2c]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44) /* -0x2c */);
    // 00493543  01ce                   -add esi, ecx
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.ecx));
    // 00493545  8975c8                 -mov dword ptr [ebp - 0x38], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-56) /* -0x38 */) = cpu.esi;
    // 00493548  8b75d8                 -mov esi, dword ptr [ebp - 0x28]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-40) /* -0x28 */);
    // 0049354b  01fe                   -add esi, edi
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.edi));
    // 0049354d  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00493550  8975cc                 -mov dword ptr [ebp - 0x34], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-52) /* -0x34 */) = cpu.esi;
    // 00493553  8b75dc                 -mov esi, dword ptr [ebp - 0x24]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 00493556  f7ea                   -imul edx
    cpu.edx_eax = x86::reg64(x86::sreg64(static_cast<x86::sreg32>(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 00493558  c1e210                 -shl edx, 0x10
    cpu.edx <<= 16 /*0x10*/ % 32;
    // 0049355b  c1e810                 +shr eax, 0x10
    {
        x86::reg8 tmp = 16 /*0x10*/ % 32;
        x86::reg32& op = cpu.eax;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = (1 & (op >> (32 - 1)));
            cpu.set_szp((op >>= tmp));
        }
    }
    // 0049355e  11d0                   -adc eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 00493560  01c6                   -add esi, eax
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.eax));
    // 00493562  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00493565  8975d0                 -mov dword ptr [ebp - 0x30], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-48) /* -0x30 */) = cpu.esi;
    // 00493568  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0049356a  743a                   -je 0x4935a6
    if (cpu.flags.zf)
    {
        goto L_0x004935a6;
    }
    // 0049356c  8b55ec                 -mov edx, dword ptr [ebp - 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0049356f  b800000400             -mov eax, 0x40000
    cpu.eax = 262144 /*0x40000*/;
    // 00493574  f7ea                   -imul edx
    cpu.edx_eax = x86::reg64(x86::sreg64(static_cast<x86::sreg32>(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 00493576  c1e210                 -shl edx, 0x10
    cpu.edx <<= 16 /*0x10*/ % 32;
    // 00493579  c1e810                 +shr eax, 0x10
    {
        x86::reg8 tmp = 16 /*0x10*/ % 32;
        x86::reg32& op = cpu.eax;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = (1 & (op >> (32 - 1)));
            cpu.set_szp((op >>= tmp));
        }
    }
    // 0049357c  11d0                   -adc eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 0049357e  8b55f0                 -mov edx, dword ptr [ebp - 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00493581  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00493583  b800000400             -mov eax, 0x40000
    cpu.eax = 262144 /*0x40000*/;
    // 00493588  8b7dc8                 -mov edi, dword ptr [ebp - 0x38]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-56) /* -0x38 */);
    // 0049358b  f7ea                   -imul edx
    cpu.edx_eax = x86::reg64(x86::sreg64(static_cast<x86::sreg32>(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 0049358d  c1e210                 -shl edx, 0x10
    cpu.edx <<= 16 /*0x10*/ % 32;
    // 00493590  c1e810                 +shr eax, 0x10
    {
        x86::reg8 tmp = 16 /*0x10*/ % 32;
        x86::reg32& op = cpu.eax;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = (1 & (op >> (32 - 1)));
            cpu.set_szp((op >>= tmp));
        }
    }
    // 00493593  11d0                   -adc eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 00493595  01cf                   +add edi, ecx
    {
        x86::reg32& tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ecx));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00493597  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0049359a  8b4dd0                 -mov ecx, dword ptr [ebp - 0x30]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-48) /* -0x30 */);
    // 0049359d  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0049359f  b800000400             -mov eax, 0x40000
    cpu.eax = 262144 /*0x40000*/;
    // 004935a4  eb38                   -jmp 0x4935de
    goto L_0x004935de;
L_0x004935a6:
    // 004935a6  8b55ec                 -mov edx, dword ptr [ebp - 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004935a9  b80000fcff             -mov eax, 0xfffc0000
    cpu.eax = 4294705152 /*0xfffc0000*/;
    // 004935ae  f7ea                   -imul edx
    cpu.edx_eax = x86::reg64(x86::sreg64(static_cast<x86::sreg32>(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 004935b0  c1e210                 -shl edx, 0x10
    cpu.edx <<= 16 /*0x10*/ % 32;
    // 004935b3  c1e810                 +shr eax, 0x10
    {
        x86::reg8 tmp = 16 /*0x10*/ % 32;
        x86::reg32& op = cpu.eax;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = (1 & (op >> (32 - 1)));
            cpu.set_szp((op >>= tmp));
        }
    }
    // 004935b6  11d0                   -adc eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 004935b8  8b55f0                 -mov edx, dword ptr [ebp - 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004935bb  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004935bd  b80000fcff             -mov eax, 0xfffc0000
    cpu.eax = 4294705152 /*0xfffc0000*/;
    // 004935c2  8b7dc8                 -mov edi, dword ptr [ebp - 0x38]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-56) /* -0x38 */);
    // 004935c5  f7ea                   -imul edx
    cpu.edx_eax = x86::reg64(x86::sreg64(static_cast<x86::sreg32>(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 004935c7  c1e210                 -shl edx, 0x10
    cpu.edx <<= 16 /*0x10*/ % 32;
    // 004935ca  c1e810                 +shr eax, 0x10
    {
        x86::reg8 tmp = 16 /*0x10*/ % 32;
        x86::reg32& op = cpu.eax;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = (1 & (op >> (32 - 1)));
            cpu.set_szp((op >>= tmp));
        }
    }
    // 004935cd  11d0                   -adc eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 004935cf  01cf                   -add edi, ecx
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004935d1  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004935d4  8b4dd0                 -mov ecx, dword ptr [ebp - 0x30]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-48) /* -0x30 */);
    // 004935d7  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004935d9  b80000fcff             -mov eax, 0xfffc0000
    cpu.eax = 4294705152 /*0xfffc0000*/;
L_0x004935de:
    // 004935de  897dc8                 -mov dword ptr [ebp - 0x38], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-56) /* -0x38 */) = cpu.edi;
    // 004935e1  f7ea                   -imul edx
    cpu.edx_eax = x86::reg64(x86::sreg64(static_cast<x86::sreg32>(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 004935e3  c1e210                 -shl edx, 0x10
    cpu.edx <<= 16 /*0x10*/ % 32;
    // 004935e6  c1e810                 +shr eax, 0x10
    {
        x86::reg8 tmp = 16 /*0x10*/ % 32;
        x86::reg32& op = cpu.eax;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = (1 & (op >> (32 - 1)));
            cpu.set_szp((op >>= tmp));
        }
    }
    // 004935e9  11d0                   -adc eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 004935eb  8b55cc                 -mov edx, dword ptr [ebp - 0x34]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-52) /* -0x34 */);
    // 004935ee  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004935f0  01f2                   -add edx, esi
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.esi));
    // 004935f2  894dd0                 -mov dword ptr [ebp - 0x30], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-48) /* -0x30 */) = cpu.ecx;
    // 004935f5  8955cc                 -mov dword ptr [ebp - 0x34], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-52) /* -0x34 */) = cpu.edx;
    // 004935f8  8b45c8                 -mov eax, dword ptr [ebp - 0x38]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-56) /* -0x38 */);
    // 004935fb  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004935fe  db45fc                 -fild dword ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 00493601  8b45cc                 -mov eax, dword ptr [ebp - 0x34]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-52) /* -0x34 */);
    // 00493604  dd0528bd5300           -fld qword ptr [0x53bd28]
    fp1 = x86::Float(app->getMemory<double>(x86::reg32(5487912) /* 0x53bd28 */));
    // 0049360a  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 0049360c  d8c9                   -fmul st(1)
    fp1 = fp::mul(fp1, x86::Float(fp0));
    // 0049360e  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 00493611  8b45d0                 -mov eax, dword ptr [ebp - 0x30]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-48) /* -0x30 */);
    // 00493614  db45fc                 -fild dword ptr [ebp - 4]
    fp2 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 00493617  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0049361a  d8ca                   -fmul st(2)
    fp2 = fp::mul(fp2, x86::Float(fp0));
    // 0049361c  db45fc                 -fild dword ptr [ebp - 4]
    fp3 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 0049361f  decb                   -fmulp st(3)
    fp0 = fp::mul(fp0, fp3);
    // 00493621  8d75e0                 -lea esi, [ebp - 0x20]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 00493624  89df                   -mov edi, ebx
    cpu.edi = cpu.ebx;
    // 00493626  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp2;
        fp2 = fp1;
        fp1 = tmp;
    }
    // 00493628  d95de0                 -fstp dword ptr [ebp - 0x20]
    app->getMemory<float>(cpu.ebp + x86::reg32(-32) /* -0x20 */) = float(fp2);
    // 0049362b  d95de4                 -fstp dword ptr [ebp - 0x1c]
    app->getMemory<float>(cpu.ebp + x86::reg32(-28) /* -0x1c */) = float(fp1);
    // 0049362e  d95de8                 -fstp dword ptr [ebp - 0x18]
    app->getMemory<float>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = float(fp0);
    // 00493631  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00493632  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00493633  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00493634  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 00493636  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00493638  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00493639  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049363a  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049363b  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049363c  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_493640(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1, fp2, fp3;
    // 00493640  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00493641  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00493642  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00493643  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00493644  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00493645  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00493646  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00493648  81ecc0000000           -sub esp, 0xc0
    (cpu.esp) -= x86::reg32(x86::sreg32(192 /*0xc0*/));
    // 0049364e  81ed82000000           -sub ebp, 0x82
    (cpu.ebp) -= x86::reg32(x86::sreg32(130 /*0x82*/));
    // 00493654  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00493656  3b0d40d95d00           +cmp ecx, dword ptr [0x5dd940]
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(6150464) /* 0x5dd940 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049365c  7d42                   -jge 0x4936a0
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004936a0;
    }
    // 0049365e  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 00493660  eb05                   -jmp 0x493667
    goto L_0x00493667;
L_0x00493662:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00493662  83fb02                 +cmp ebx, 2
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
    // 00493665  7d2c                   -jge 0x493693
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00493693;
    }
L_0x00493667:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00493667  8d753a                 -lea esi, [ebp + 0x3a]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(58) /* 0x3a */);
    // 0049366a  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 0049366c  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0049366e  e84dfeffff             -call 0x4934c0
    cpu.esp -= 4;
    sub_4934c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00493673  8d048d00000000         -lea eax, [ecx*4]
    cpu.eax = x86::reg32(cpu.ecx * 4);
    // 0049367a  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 0049367c  8b149da0e45500         -mov edx, dword ptr [ebx*4 + 0x55e4a0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5629088) /* 0x55e4a0 */ + cpu.ebx * 4);
    // 00493683  c1e004                 +shl eax, 4
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
    // 00493686  8d753a                 -lea esi, [ebp + 0x3a]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(58) /* 0x3a */);
    // 00493689  8d7c0238               -lea edi, [edx + eax + 0x38]
    cpu.edi = x86::reg32(cpu.edx + x86::reg32(56) /* 0x38 */ + cpu.eax * 1);
    // 0049368d  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0049368e  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0049368f  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00493690  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00493691  ebcf                   -jmp 0x493662
    goto L_0x00493662;
L_0x00493693:
    // 00493693  41                     -inc ecx
    (cpu.ecx)++;
    // 00493694  3b0d40d95d00           +cmp ecx, dword ptr [0x5dd940]
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(6150464) /* 0x5dd940 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049369a  7d04                   -jge 0x4936a0
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004936a0;
    }
    // 0049369c  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 0049369e  ebc7                   -jmp 0x493667
    goto L_0x00493667;
L_0x004936a0:
    // 004936a0  31c9                   +xor ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx))));
    // 004936a2  894d66                 -mov dword ptr [ebp + 0x66], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(102) /* 0x66 */) = cpu.ecx;
    // 004936a5  e9d7000000             -jmp 0x493781
    goto L_0x00493781;
L_0x004936aa:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004936aa  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
L_0x004936ac:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004936ac  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004936b3  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004936b5  8b756a                 -mov esi, dword ptr [ebp + 0x6a]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(106) /* 0x6a */);
    // 004936b8  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 004936bb  8b14b5a0e45500         -mov edx, dword ptr [esi*4 + 0x55e4a0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5629088) /* 0x55e4a0 */ + cpu.esi * 4);
    // 004936c2  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004936c4  8b4d72                 -mov ecx, dword ptr [ebp + 0x72]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(114) /* 0x72 */);
    // 004936c7  8d5838                 -lea ebx, [eax + 0x38]
    cpu.ebx = x86::reg32(cpu.eax + x86::reg32(56) /* 0x38 */);
    // 004936ca  8d048d00000000         -lea eax, [ecx*4]
    cpu.eax = x86::reg32(cpu.ecx * 4);
    // 004936d1  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004936d3  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 004936d6  8d4d52                 -lea ecx, [ebp + 0x52]
    cpu.ecx = x86::reg32(cpu.ebp + x86::reg32(82) /* 0x52 */);
    // 004936d9  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
    // 004936dc  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004936de  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004936e3  83c238                 -add edx, 0x38
    (cpu.edx) += x86::reg32(x86::sreg32(56 /*0x38*/));
    // 004936e6  e8c5c80400             -call 0x4dffb0
    cpu.esp -= 4;
    nfs3hp::vertices::addVectors(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004936eb  8d04bd00000000         -lea eax, [edi*4]
    cpu.eax = x86::reg32(cpu.edi * 4);
    // 004936f2  01f8                   -add eax, edi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edi));
    // 004936f4  8b14b5a0e45500         -mov edx, dword ptr [esi*4 + 0x55e4a0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5629088) /* 0x55e4a0 */ + cpu.esi * 4);
    // 004936fb  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 004936fe  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00493700  8d4d52                 -lea ecx, [ebp + 0x52]
    cpu.ecx = x86::reg32(cpu.ebp + x86::reg32(82) /* 0x52 */);
    // 00493703  8d5838                 -lea ebx, [eax + 0x38]
    cpu.ebx = x86::reg32(cpu.eax + x86::reg32(56) /* 0x38 */);
    // 00493706  8d5552                 -lea edx, [ebp + 0x52]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(82) /* 0x52 */);
    // 00493709  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0049370e  e89dc80400             -call 0x4dffb0
    cpu.esp -= 4;
    nfs3hp::vertices::addVectors(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00493713  d90530bd5300           -fld dword ptr [0x53bd30]
    fp0 = x86::Float(app->getMemory<float>(x86::reg32(5487920) /* 0x53bd30 */));
    // 00493719  d94552                 -fld dword ptr [ebp + 0x52]
    fp1 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(82) /* 0x52 */));
    // 0049371c  d8c9                   -fmul st(1)
    fp1 = fp::mul(fp1, x86::Float(fp0));
    // 0049371e  d94556                 -fld dword ptr [ebp + 0x56]
    fp2 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(86) /* 0x56 */));
    // 00493721  d8ca                   -fmul st(2)
    fp2 = fp::mul(fp2, x86::Float(fp0));
    // 00493723  d9455a                 -fld dword ptr [ebp + 0x5a]
    fp3 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(90) /* 0x5a */));
    // 00493726  decb                   -fmulp st(3)
    fp0 = fp::mul(fp0, fp3);
    // 00493728  8b557e                 -mov edx, dword ptr [ebp + 0x7e]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 0049372b  8b4d6a                 -mov ecx, dword ptr [ebp + 0x6a]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(106) /* 0x6a */);
    // 0049372e  8b04b5a0e45500         -mov eax, dword ptr [esi*4 + 0x55e4a0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629088) /* 0x55e4a0 */ + cpu.esi * 4);
    // 00493735  8d7552                 -lea esi, [ebp + 0x52]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(82) /* 0x52 */);
    // 00493738  41                     -inc ecx
    (cpu.ecx)++;
    // 00493739  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 0049373b  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp2;
        fp2 = fp1;
        fp1 = tmp;
    }
    // 0049373d  d95d52                 -fstp dword ptr [ebp + 0x52]
    app->getMemory<float>(cpu.ebp + x86::reg32(82) /* 0x52 */) = float(fp2);
    // 00493740  d95d56                 -fstp dword ptr [ebp + 0x56]
    app->getMemory<float>(cpu.ebp + x86::reg32(86) /* 0x56 */) = float(fp1);
    // 00493743  d95d5a                 -fstp dword ptr [ebp + 0x5a]
    app->getMemory<float>(cpu.ebp + x86::reg32(90) /* 0x5a */) = float(fp0);
    // 00493746  8d7838                 -lea edi, [eax + 0x38]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(56) /* 0x38 */);
    // 00493749  8b5572                 -mov edx, dword ptr [ebp + 0x72]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(114) /* 0x72 */);
    // 0049374c  894d6a                 -mov dword ptr [ebp + 0x6a], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(106) /* 0x6a */) = cpu.ecx;
    // 0049374f  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00493750  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00493751  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00493752  837d6a02               +cmp dword ptr [ebp + 0x6a], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(106) /* 0x6a */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00493756  7d1b                   -jge 0x493773
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00493773;
    }
L_0x00493758:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00493758  8b4572                 -mov eax, dword ptr [ebp + 0x72]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(114) /* 0x72 */);
    // 0049375b  8b1d40d95d00           -mov ebx, dword ptr [0x5dd940]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(6150464) /* 0x5dd940 */);
    // 00493761  40                     -inc eax
    (cpu.eax)++;
    // 00493762  39d8                   +cmp eax, ebx
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
    // 00493764  0f8c40ffffff           -jl 0x4936aa
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004936aa;
    }
    // 0049376a  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0049376c  29df                   +sub edi, ebx
    {
        x86::reg32& tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0049376e  e939ffffff             -jmp 0x4936ac
    goto L_0x004936ac;
L_0x00493773:
    // 00493773  ff4572                 +inc dword ptr [ebp + 0x72]
    {
        auto tmp = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(114) /* 0x72 */);
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00493776  eb15                   -jmp 0x49378d
    goto L_0x0049378d;
L_0x00493778:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00493778  ff4566                 -inc dword ptr [ebp + 0x66]
    (app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(102) /* 0x66 */))++;
    // 0049377b  837d6604               +cmp dword ptr [ebp + 0x66], 4
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(102) /* 0x66 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049377f  7d1e                   -jge 0x49379f
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0049379f;
    }
L_0x00493781:
    // 00493781  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 00493783  8b1540d95d00           -mov edx, dword ptr [0x5dd940]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(6150464) /* 0x5dd940 */);
    // 00493789  897572                 -mov dword ptr [ebp + 0x72], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(114) /* 0x72 */) = cpu.esi;
    // 0049378c  4a                     -dec edx
    (cpu.edx)--;
L_0x0049378d:
    // 0049378d  8b4572                 -mov eax, dword ptr [ebp + 0x72]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(114) /* 0x72 */);
    // 00493790  3b0540d95d00           +cmp eax, dword ptr [0x5dd940]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(6150464) /* 0x5dd940 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00493796  7de0                   -jge 0x493778
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00493778;
    }
    // 00493798  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0049379a  89456a                 -mov dword ptr [ebp + 0x6a], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(106) /* 0x6a */) = cpu.eax;
    // 0049379d  ebb9                   -jmp 0x493758
    goto L_0x00493758;
L_0x0049379f:
    // 0049379f  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 004937a1  897d76                 -mov dword ptr [ebp + 0x76], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */) = cpu.edi;
L_0x004937a4:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004937a4  8b4576                 -mov eax, dword ptr [ebp + 0x76]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */);
    // 004937a7  3b0540d95d00           +cmp eax, dword ptr [0x5dd940]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(6150464) /* 0x5dd940 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004937ad  0f8d6c020000           -jge 0x493a1f
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00493a1f;
    }
    // 004937b3  31c9                   +xor ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx))));
    // 004937b5  894d6e                 -mov dword ptr [ebp + 0x6e], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(110) /* 0x6e */) = cpu.ecx;
    // 004937b8  e93a010000             -jmp 0x4938f7
    goto L_0x004938f7;
L_0x004937bd:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004937bd  8b457a                 -mov eax, dword ptr [ebp + 0x7a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 004937c0  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 004937c2  89de                   -mov esi, ebx
    cpu.esi = cpu.ebx;
    // 004937c4  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 004937c7  8b80a0e45500           -mov eax, dword ptr [eax + 0x55e4a0]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(5629088) /* 0x55e4a0 */);
    // 004937cd  c1fe08                 -sar esi, 8
    cpu.esi = x86::reg32(x86::sreg32(cpu.esi) >> (8 /*0x8*/ % 32));
    // 004937d0  81e3ff000000           -and ebx, 0xff
    cpu.ebx &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 004937d6  d944013c               -fld dword ptr [ecx + eax + 0x3c]
    fp0 = x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(60) /* 0x3c */ + cpu.eax * 1));
    // 004937da  81e2ff000000           -and edx, 0xff
    cpu.edx &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 004937e0  81e6ff000000           -and esi, 0xff
    cpu.esi &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 004937e6  81c3ff000000           -add ebx, 0xff
    (cpu.ebx) += x86::reg32(x86::sreg32(255 /*0xff*/));
    // 004937ec  d9c0                   -fld st(0)
    fp1 = x86::Float(fp0);
    // 004937ee  81c2ff000000           -add edx, 0xff
    (cpu.edx) += x86::reg32(x86::sreg32(255 /*0xff*/));
    // 004937f4  81c6ff000000           -add esi, 0xff
    (cpu.esi) += x86::reg32(x86::sreg32(255 /*0xff*/));
    // 004937fa  d1fa                   -sar edx, 1
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (1 /*0x1*/ % 32));
    // 004937fc  dc0548bd5300           -fadd qword ptr [0x53bd48]
    fp1 = fp::add(fp1, x86::Float(app->getMemory<double>(x86::reg32(5487944) /* 0x53bd48 */)));
    // 00493802  d1fe                   -sar esi, 1
    cpu.esi = x86::reg32(x86::sreg32(cpu.esi) >> (1 /*0x1*/ % 32));
    // 00493804  ddd9                   -fstp st(1)
    fp0 = x86::Float(fp1);
    // 00493806  81e2ff000000           -and edx, 0xff
    cpu.edx &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 0049380c  d95c013c               -fstp dword ptr [ecx + eax + 0x3c]
    app->getMemory<float>(cpu.ecx + x86::reg32(60) /* 0x3c */ + cpu.eax * 1) = float(fp0);
    // 00493810  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00493812  c1e210                 -shl edx, 0x10
    cpu.edx <<= 16 /*0x10*/ % 32;
    // 00493815  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 0049381a  81ca000000ff           -or edx, 0xff000000
    cpu.edx |= x86::reg32(x86::sreg32(4278190080 /*0xff000000*/));
    // 00493820  c1e008                 -shl eax, 8
    cpu.eax <<= 8 /*0x8*/ % 32;
    // 00493823  d1fb                   -sar ebx, 1
    cpu.ebx = x86::reg32(x86::sreg32(cpu.ebx) >> (1 /*0x1*/ % 32));
    // 00493825  09c2                   -or edx, eax
    cpu.edx |= x86::reg32(x86::sreg32(cpu.eax));
    // 00493827  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 00493829  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 0049382e  09d0                   -or eax, edx
    cpu.eax |= x86::reg32(x86::sreg32(cpu.edx));
    // 00493830  8b557a                 -mov edx, dword ptr [ebp + 0x7a]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 00493833  8b92a0e45500           -mov edx, dword ptr [edx + 0x55e4a0]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(5629088) /* 0x55e4a0 */);
    // 00493839  89441104               -mov dword ptr [ecx + edx + 4], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(4) /* 0x4 */ + cpu.edx * 1) = cpu.eax;
    // 0049383d  8b457a                 -mov eax, dword ptr [ebp + 0x7a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 00493840  8b80a0e45500           -mov eax, dword ptr [eax + 0x55e4a0]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(5629088) /* 0x55e4a0 */);
    // 00493846  8d7c012c               -lea edi, [ecx + eax + 0x2c]
    cpu.edi = x86::reg32(cpu.ecx + x86::reg32(44) /* 0x2c */ + cpu.eax * 1);
    // 0049384a  8d740138               -lea esi, [ecx + eax + 0x38]
    cpu.esi = x86::reg32(cpu.ecx + x86::reg32(56) /* 0x38 */ + cpu.eax * 1);
    // 0049384e  8b457a                 -mov eax, dword ptr [ebp + 0x7a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 00493851  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00493852  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00493853  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00493854  8b80a0e45500           -mov eax, dword ptr [eax + 0x55e4a0]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(5629088) /* 0x55e4a0 */);
    // 0049385a  c74401080000803f       -mov dword ptr [ecx + eax + 8], 0x3f800000
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(8) /* 0x8 */ + cpu.eax * 1) = 1065353216 /*0x3f800000*/;
    // 00493862  8b457a                 -mov eax, dword ptr [ebp + 0x7a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 00493865  8b80a0e45500           -mov eax, dword ptr [eax + 0x55e4a0]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(5629088) /* 0x55e4a0 */);
    // 0049386b  c744010c00000000       -mov dword ptr [ecx + eax + 0xc], 0
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(12) /* 0xc */ + cpu.eax * 1) = 0 /*0x0*/;
    // 00493873  8b457a                 -mov eax, dword ptr [ebp + 0x7a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 00493876  8b80a0e45500           -mov eax, dword ptr [eax + 0x55e4a0]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(5629088) /* 0x55e4a0 */);
    // 0049387c  c744011000000000       -mov dword ptr [ecx + eax + 0x10], 0
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(16) /* 0x10 */ + cpu.eax * 1) = 0 /*0x0*/;
    // 00493884  8b457a                 -mov eax, dword ptr [ebp + 0x7a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 00493887  8b80a0e45500           -mov eax, dword ptr [eax + 0x55e4a0]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(5629088) /* 0x55e4a0 */);
    // 0049388d  c744011400000000       -mov dword ptr [ecx + eax + 0x14], 0
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(20) /* 0x14 */ + cpu.eax * 1) = 0 /*0x0*/;
    // 00493895  8b457a                 -mov eax, dword ptr [ebp + 0x7a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 00493898  8b80a0e45500           -mov eax, dword ptr [eax + 0x55e4a0]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(5629088) /* 0x55e4a0 */);
    // 0049389e  c74401180000803f       -mov dword ptr [ecx + eax + 0x18], 0x3f800000
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(24) /* 0x18 */ + cpu.eax * 1) = 1065353216 /*0x3f800000*/;
    // 004938a6  8b457a                 -mov eax, dword ptr [ebp + 0x7a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 004938a9  8b80a0e45500           -mov eax, dword ptr [eax + 0x55e4a0]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(5629088) /* 0x55e4a0 */);
    // 004938af  c744011c00000000       -mov dword ptr [ecx + eax + 0x1c], 0
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(28) /* 0x1c */ + cpu.eax * 1) = 0 /*0x0*/;
    // 004938b7  8b457a                 -mov eax, dword ptr [ebp + 0x7a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 004938ba  8b80a0e45500           -mov eax, dword ptr [eax + 0x55e4a0]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(5629088) /* 0x55e4a0 */);
    // 004938c0  c744012000000000       -mov dword ptr [ecx + eax + 0x20], 0
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(32) /* 0x20 */ + cpu.eax * 1) = 0 /*0x0*/;
    // 004938c8  8b457a                 -mov eax, dword ptr [ebp + 0x7a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 004938cb  8b80a0e45500           -mov eax, dword ptr [eax + 0x55e4a0]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(5629088) /* 0x55e4a0 */);
    // 004938d1  c744012400000000       -mov dword ptr [ecx + eax + 0x24], 0
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(36) /* 0x24 */ + cpu.eax * 1) = 0 /*0x0*/;
    // 004938d9  8b457a                 -mov eax, dword ptr [ebp + 0x7a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 004938dc  8b80a0e45500           -mov eax, dword ptr [eax + 0x55e4a0]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(5629088) /* 0x55e4a0 */);
    // 004938e2  c74401280000803f       -mov dword ptr [ecx + eax + 0x28], 0x3f800000
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(40) /* 0x28 */ + cpu.eax * 1) = 1065353216 /*0x3f800000*/;
L_0x004938ea:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004938ea  ff456e                 -inc dword ptr [ebp + 0x6e]
    (app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(110) /* 0x6e */))++;
    // 004938ed  837d6e02               +cmp dword ptr [ebp + 0x6e], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(110) /* 0x6e */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004938f1  0f8d20010000           -jge 0x493a17
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00493a17;
    }
L_0x004938f7:
    // 004938f7  8b456e                 -mov eax, dword ptr [ebp + 0x6e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(110) /* 0x6e */);
    // 004938fa  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004938fd  89457a                 -mov dword ptr [ebp + 0x7a], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */) = cpu.eax;
    // 00493900  8b4576                 -mov eax, dword ptr [ebp + 0x76]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */);
    // 00493903  8d0c8500000000         -lea ecx, [eax*4]
    cpu.ecx = x86::reg32(cpu.eax * 4);
    // 0049390a  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0049390c  8b457a                 -mov eax, dword ptr [ebp + 0x7a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 0049390f  c1e104                 -shl ecx, 4
    cpu.ecx <<= 4 /*0x4*/ % 32;
    // 00493912  8b80a0e45500           -mov eax, dword ptr [eax + 0x55e4a0]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(5629088) /* 0x55e4a0 */);
    // 00493918  d9440138               -fld dword ptr [ecx + eax + 0x38]
    fp0 = x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(56) /* 0x38 */ + cpu.eax * 1));
    // 0049391c  dc0d38bd5300           -fmul qword ptr [0x53bd38]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5487928) /* 0x53bd38 */)));
    // 00493922  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00493925  db1c24                 -fistp dword ptr [esp]
    app->getMemory<x86::reg32>(cpu.esp) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 00493928  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00493929  894546                 -mov dword ptr [ebp + 0x46], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(70) /* 0x46 */) = cpu.eax;
    // 0049392c  8b457a                 -mov eax, dword ptr [ebp + 0x7a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 0049392f  8b80a0e45500           -mov eax, dword ptr [eax + 0x55e4a0]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(5629088) /* 0x55e4a0 */);
    // 00493935  d944013c               -fld dword ptr [ecx + eax + 0x3c]
    fp0 = x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(60) /* 0x3c */ + cpu.eax * 1));
    // 00493939  dc0d38bd5300           -fmul qword ptr [0x53bd38]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5487928) /* 0x53bd38 */)));
    // 0049393f  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00493942  db1c24                 -fistp dword ptr [esp]
    app->getMemory<x86::reg32>(cpu.esp) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 00493945  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00493946  89454a                 -mov dword ptr [ebp + 0x4a], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(74) /* 0x4a */) = cpu.eax;
    // 00493949  8b457a                 -mov eax, dword ptr [ebp + 0x7a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 0049394c  8b80a0e45500           -mov eax, dword ptr [eax + 0x55e4a0]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(5629088) /* 0x55e4a0 */);
    // 00493952  d9440140               -fld dword ptr [ecx + eax + 0x40]
    fp0 = x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(64) /* 0x40 */ + cpu.eax * 1));
    // 00493956  dc0d38bd5300           -fmul qword ptr [0x53bd38]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5487928) /* 0x53bd38 */)));
    // 0049395c  8b5d76                 -mov ebx, dword ptr [ebp + 0x76]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */);
    // 0049395f  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 00493964  8d75c2                 -lea esi, [ebp - 0x3e]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-62) /* -0x3e */);
    // 00493967  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0049396a  db1c24                 -fistp dword ptr [esp]
    app->getMemory<x86::reg32>(cpu.esp) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 0049396d  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049396e  89454e                 -mov dword ptr [ebp + 0x4e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(78) /* 0x4e */) = cpu.eax;
    // 00493971  8d4546                 -lea eax, [ebp + 0x46]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(70) /* 0x46 */);
    // 00493974  e8d7ccf8ff             -call 0x420650
    cpu.esp -= 4;
    nfs3hp::trackdata::placeOnTrack(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00493979  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0049397e  8d45c2                 -lea eax, [ebp - 0x3e]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-62) /* -0x3e */);
    // 00493981  e86acdf8ff             -call 0x4206f0
    cpu.esp -= 4;
    nfs3hp::trackdata::positionColour(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00493986  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00493988  8b4546                 -mov eax, dword ptr [ebp + 0x46]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(70) /* 0x46 */);
    // 0049398b  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
    // 0049398e  db457e                 -fild dword ptr [ebp + 0x7e]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */)));
    // 00493991  8b457a                 -mov eax, dword ptr [ebp + 0x7a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 00493994  dd0540bd5300           -fld qword ptr [0x53bd40]
    fp1 = x86::Float(app->getMemory<double>(x86::reg32(5487936) /* 0x53bd40 */));
    // 0049399a  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 0049399c  d8c9                   -fmul st(1)
    fp1 = fp::mul(fp1, x86::Float(fp0));
    // 0049399e  8b80a0e45500           -mov eax, dword ptr [eax + 0x55e4a0]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(5629088) /* 0x55e4a0 */);
    // 004939a4  d95c0138               -fstp dword ptr [ecx + eax + 0x38]
    app->getMemory<float>(cpu.ecx + x86::reg32(56) /* 0x38 */ + cpu.eax * 1) = float(fp1);
    // 004939a8  8b454a                 -mov eax, dword ptr [ebp + 0x4a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(74) /* 0x4a */);
    // 004939ab  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
    // 004939ae  db457e                 -fild dword ptr [ebp + 0x7e]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */)));
    // 004939b1  8b457a                 -mov eax, dword ptr [ebp + 0x7a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 004939b4  d8c9                   -fmul st(1)
    fp1 = fp::mul(fp1, x86::Float(fp0));
    // 004939b6  8b80a0e45500           -mov eax, dword ptr [eax + 0x55e4a0]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(5629088) /* 0x55e4a0 */);
    // 004939bc  d95c013c               -fstp dword ptr [ecx + eax + 0x3c]
    app->getMemory<float>(cpu.ecx + x86::reg32(60) /* 0x3c */ + cpu.eax * 1) = float(fp1);
    // 004939c0  8b454e                 -mov eax, dword ptr [ebp + 0x4e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(78) /* 0x4e */);
    // 004939c3  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
    // 004939c6  db457e                 -fild dword ptr [ebp + 0x7e]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */)));
    // 004939c9  8b457a                 -mov eax, dword ptr [ebp + 0x7a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 004939cc  dec9                   -fmulp st(1)
    fp0 = fp::mul(fp0, fp1);
    // 004939ce  8b80a0e45500           -mov eax, dword ptr [eax + 0x55e4a0]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(5629088) /* 0x55e4a0 */);
    // 004939d4  d95c0140               -fstp dword ptr [ecx + eax + 0x40]
    app->getMemory<float>(cpu.ecx + x86::reg32(64) /* 0x40 */ + cpu.eax * 1) = float(fp0);
    // 004939d8  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 004939dd  8d45c2                 -lea eax, [ebp - 0x3e]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-62) /* -0x3e */);
    // 004939e0  e8fbcdf8ff             -call 0x4207e0
    cpu.esp -= 4;
    nfs3hp::trackdata::positionSurface(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004939e5  a83f                   +test al, 0x3f
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.al & 63 /*0x3f*/));
    // 004939e7  740a                   -je 0x4939f3
    if (cpu.flags.zf)
    {
        goto L_0x004939f3;
    }
    // 004939e9  837d2e00               +cmp dword ptr [ebp + 0x2e], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(46) /* 0x2e */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004939ed  0f84cafdffff           -je 0x4937bd
    if (cpu.flags.zf)
    {
        goto L_0x004937bd;
    }
L_0x004939f3:
    // 004939f3  8b5576                 -mov edx, dword ptr [ebp + 0x76]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */);
    // 004939f6  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004939fd  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004939ff  8b4d6e                 -mov ecx, dword ptr [ebp + 0x6e]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(110) /* 0x6e */);
    // 00493a02  c1e004                 +shl eax, 4
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
    // 00493a05  8b148da0e45500         -mov edx, dword ptr [ecx*4 + 0x55e4a0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5629088) /* 0x55e4a0 */ + cpu.ecx * 4);
    // 00493a0c  66c70402ffff           -mov word ptr [edx + eax], 0xffff
    app->getMemory<x86::reg16>(cpu.edx + cpu.eax * 1) = 65535 /*0xffff*/;
    // 00493a12  e9d3feffff             -jmp 0x4938ea
    goto L_0x004938ea;
L_0x00493a17:
    // 00493a17  ff4576                 +inc dword ptr [ebp + 0x76]
    {
        auto tmp = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */);
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00493a1a  e985fdffff             -jmp 0x4937a4
    goto L_0x004937a4;
L_0x00493a1f:
    // 00493a1f  8da582000000           -lea esp, [ebp + 0x82]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(130) /* 0x82 */);
    // 00493a25  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00493a26  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00493a27  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00493a28  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00493a29  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00493a2a  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00493a2b  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_493a30(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0;
    // 00493a30  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00493a31  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00493a32  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00493a33  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00493a34  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00493a35  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00493a37  83ec08                 -sub esp, 8
    (cpu.esp) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 00493a3a  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00493a3c  8b481c                 -mov ecx, dword ptr [eax + 0x1c]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(28) /* 0x1c */);
    // 00493a3f  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00493a41  e87aa6f7ff             -call 0x40e0c0
    cpu.esp -= 4;
    nfs3hp::aiplan::roadKindAt(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00493a46  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00493a48  8b82b8070000           -mov eax, dword ptr [edx + 0x7b8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(1976) /* 0x7b8 */);
    // 00493a4e  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 00493a51  8b8214010000           -mov eax, dword ptr [edx + 0x114]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(276) /* 0x114 */);
    // 00493a57  8b3584367d00           -mov esi, dword ptr [0x7d3684]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(8205956) /* 0x7d3684 */);
    // 00493a5d  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 00493a60  81fe80040000           +cmp esi, 0x480
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1152 /*0x480*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00493a66  7d08                   -jge 0x493a70
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00493a70;
    }
    // 00493a68  ff82dc020000           +inc dword ptr [edx + 0x2dc]
    {
        auto tmp = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(732) /* 0x2dc */);
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00493a6e  eb50                   -jmp 0x493ac0
    goto L_0x00493ac0;
L_0x00493a70:
    // 00493a70  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00493a72  ff75fc                 -push dword ptr [ebp - 4]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    cpu.esp -= 4;
    // 00493a75  e836a8f7ff             -call 0x40e2b0
    cpu.esp -= 4;
    nfs3hp::aiplan::roadSliceIndex(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00493a7a  39c3                   +cmp ebx, eax
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
    // 00493a7c  742e                   -je 0x493aac
    if (cpu.flags.zf)
    {
        goto L_0x00493aac;
    }
    // 00493a7e  d945fc                 -fld dword ptr [ebp - 4]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
    // 00493a81  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00493a84  d865f8                 -fsub dword ptr [ebp - 8]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 00493a87  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00493a89  d91c24                 -fstp dword ptr [esp]
    app->getMemory<float>(cpu.esp) = float(fp0);
    // 00493a8c  e81fa8f7ff             -call 0x40e2b0
    cpu.esp -= 4;
    nfs3hp::aiplan::roadSliceIndex(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00493a91  39c3                   +cmp ebx, eax
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
    // 00493a93  7417                   -je 0x493aac
    if (cpu.flags.zf)
    {
        goto L_0x00493aac;
    }
    // 00493a95  d945fc                 -fld dword ptr [ebp - 4]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
    // 00493a98  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00493a9b  d845f8                 -fadd dword ptr [ebp - 8]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 00493a9e  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00493aa0  d91c24                 -fstp dword ptr [esp]
    app->getMemory<float>(cpu.esp) = float(fp0);
    // 00493aa3  e808a8f7ff             -call 0x40e2b0
    cpu.esp -= 4;
    nfs3hp::aiplan::roadSliceIndex(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00493aa8  39c3                   +cmp ebx, eax
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
    // 00493aaa  750e                   -jne 0x493aba
    if (!cpu.flags.zf)
    {
        goto L_0x00493aba;
    }
L_0x00493aac:
    // 00493aac  ff82dc020000           -inc dword ptr [edx + 0x2dc]
    (app->getMemory<x86::reg32>(cpu.edx + x86::reg32(732) /* 0x2dc */))++;
    // 00493ab2  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00493ab4  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00493ab5  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00493ab6  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00493ab7  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00493ab8  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00493ab9  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00493aba:
    // 00493aba  ff82e0020000           -inc dword ptr [edx + 0x2e0]
    (app->getMemory<x86::reg32>(cpu.edx + x86::reg32(736) /* 0x2e0 */))++;
L_0x00493ac0:
    // 00493ac0  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00493ac2  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00493ac3  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00493ac4  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00493ac5  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00493ac6  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00493ac7  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_493ad0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1, fp2, fp3, fp4, fp5;
  x86::reg16 fsw = 0;
    // 00493ad0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00493ad1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00493ad2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00493ad3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00493ad4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00493ad5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00493ad6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00493ad8  83ec74                 -sub esp, 0x74
    (cpu.esp) -= x86::reg32(x86::sreg32(116 /*0x74*/));
    // 00493adb  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 00493ade  833d98d46f0000         +cmp dword ptr [0x6fd498], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328920) /* 0x6fd498 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00493ae5  0f8453030000           -je 0x493e3e
    if (cpu.flags.zf)
    {
        goto L_0x00493e3e;
    }
    // 00493aeb  d980b8000000           +fld dword ptr [eax + 0xb8]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(184) /* 0xb8 */));
    // 00493af1  dc1d50bd5300           +fcomp qword ptr [0x53bd50]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<double>(x86::reg32(5487952) /* 0x53bd50 */)));
    // 00493af7  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00493af9  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00493afa  0f823e030000           -jb 0x493e3e
    if (cpu.flags.cf)
    {
        goto L_0x00493e3e;
    }
    // 00493b00  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00493b03  b9fdffffff             -mov ecx, 0xfffffffd
    cpu.ecx = 4294967293 /*0xfffffffd*/;
    // 00493b08  8d7db0                 -lea edi, [ebp - 0x50]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-80) /* -0x50 */);
    // 00493b0b  8db014010000           -lea esi, [eax + 0x114]
    cpu.esi = x86::reg32(cpu.eax + x86::reg32(276) /* 0x114 */);
    // 00493b11  894df4                 -mov dword ptr [ebp - 0xc], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.ecx;
    // 00493b14  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00493b15  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00493b16  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00493b17  31f6                   +xor esi, esi
    cpu.clear_co();
    cpu.set_szp((cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi))));
    // 00493b19  8975f0                 -mov dword ptr [ebp - 0x10], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.esi;
    // 00493b1c  e9c5020000             -jmp 0x493de6
    goto L_0x00493de6;
L_0x00493b21:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00493b21  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00493b24  8b75f4                 -mov esi, dword ptr [ebp - 0xc]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00493b27  8b401c                 -mov eax, dword ptr [eax + 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(28) /* 0x1c */);
    // 00493b2a  01f0                   -add eax, esi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.esi));
    // 00493b2c  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00493b2e  7d06                   -jge 0x493b36
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00493b36;
    }
    // 00493b30  030540d95d00           -add eax, dword ptr [0x5dd940]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(6150464) /* 0x5dd940 */)));
L_0x00493b36:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00493b36  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00493b38  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00493b3b  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00493b3d  8b4df0                 -mov ecx, dword ptr [ebp - 0x10]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00493b40  c1e204                 -shl edx, 4
    cpu.edx <<= 4 /*0x4*/ % 32;
    // 00493b43  8b048da0e45500         -mov eax, dword ptr [ecx*4 + 0x55e4a0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629088) /* 0x55e4a0 */ + cpu.ecx * 4);
    // 00493b4a  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00493b4c  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 00493b4f  66833800               +cmp word ptr [eax], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.eax);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 00493b53  0f8584020000           -jne 0x493ddd
    if (!cpu.flags.zf)
    {
        goto L_0x00493ddd;
    }
    // 00493b59  6683780200             +cmp word ptr [eax + 2], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(2) /* 0x2 */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 00493b5e  0f8579020000           -jne 0x493ddd
    if (!cpu.flags.zf)
    {
        goto L_0x00493ddd;
    }
    // 00493b64  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00493b67  8d5838                 -lea ebx, [eax + 0x38]
    cpu.ebx = x86::reg32(cpu.eax + x86::reg32(56) /* 0x38 */);
    // 00493b6a  8d4da4                 -lea ecx, [ebp - 0x5c]
    cpu.ecx = x86::reg32(cpu.ebp + x86::reg32(-92) /* -0x5c */);
    // 00493b6d  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00493b72  81c298000000           -add edx, 0x98
    (cpu.edx) += x86::reg32(x86::sreg32(152 /*0x98*/));
    // 00493b78  e883c40400             -call 0x4e0000
    cpu.esp -= 4;
    nfs3hp::vertices::subtractVectors(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00493b7d  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00493b80  8d5d98                 -lea ebx, [ebp - 0x68]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-104) /* -0x68 */);
    // 00493b83  8d45a4                 -lea eax, [ebp - 0x5c]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-92) /* -0x5c */);
    // 00493b86  81c2c0000000           +add edx, 0xc0
    {
        x86::reg32& tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(192 /*0xc0*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00493b8c  d900                   +fld dword ptr [eax]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00493b8e  d80a                   +fmul dword ptr [edx]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.edx)));
    // 00493b90  d900                   +fld dword ptr [eax]
    fp1 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00493b92  d84a04                 +fmul dword ptr [edx + 4]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(4) /* 0x4 */)));
    // 00493b95  d900                   +fld dword ptr [eax]
    fp2 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00493b97  d84a08                 +fmul dword ptr [edx + 8]
    fp2 = fp::mul(fp2, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(8) /* 0x8 */)));
    // 00493b9a  d9ca                   +fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 00493b9c  d94004                 +fld dword ptr [eax + 4]
    fp3 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00493b9f  d84a0c                 +fmul dword ptr [edx + 0xc]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(12) /* 0xc */)));
    // 00493ba2  d94004                 +fld dword ptr [eax + 4]
    fp4 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00493ba5  d84a10                 +fmul dword ptr [edx + 0x10]
    fp4 = fp::mul(fp4, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(16) /* 0x10 */)));
    // 00493ba8  d94004                 +fld dword ptr [eax + 4]
    fp5 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00493bab  d84a14                 +fmul dword ptr [edx + 0x14]
    fp5 = fp::mul(fp5, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(20) /* 0x14 */)));
    // 00493bae  d9ca                   +fxch st(2)
    {
        x86::Float tmp = fp5;
        fp5 = fp3;
        fp3 = tmp;
    }
    // 00493bb0  dec3                   +faddp st(3)
    fp2 = fp::add(fp2, fp5);
    // 00493bb2  dec3                   +faddp st(3)
    fp1 = fp::add(fp1, fp4);
    // 00493bb4  dec3                   +faddp st(3)
    fp0 = fp::add(fp0, fp3);
    // 00493bb6  d94008                 +fld dword ptr [eax + 8]
    fp3 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00493bb9  d84a18                 +fmul dword ptr [edx + 0x18]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(24) /* 0x18 */)));
    // 00493bbc  d94008                 +fld dword ptr [eax + 8]
    fp4 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00493bbf  d84a1c                 +fmul dword ptr [edx + 0x1c]
    fp4 = fp::mul(fp4, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(28) /* 0x1c */)));
    // 00493bc2  d94008                 +fld dword ptr [eax + 8]
    fp5 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 00493bc5  d84a20                 +fmul dword ptr [edx + 0x20]
    fp5 = fp::mul(fp5, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(32) /* 0x20 */)));
    // 00493bc8  d9ca                   +fxch st(2)
    {
        x86::Float tmp = fp5;
        fp5 = fp3;
        fp3 = tmp;
    }
    // 00493bca  dec3                   +faddp st(3)
    fp2 = fp::add(fp2, fp5);
    // 00493bcc  dec3                   +faddp st(3)
    fp1 = fp::add(fp1, fp4);
    // 00493bce  dec3                   +faddp st(3)
    fp0 = fp::add(fp0, fp3);
    // 00493bd0  8d400c                 -lea eax, [eax + 0xc]
    cpu.eax = x86::reg32(cpu.eax + x86::reg32(12) /* 0xc */);
    // 00493bd3  d91b                   +fstp dword ptr [ebx]
    app->getMemory<float>(cpu.ebx) = float(fp2);
    // 00493bd5  d95b04                 +fstp dword ptr [ebx + 4]
    app->getMemory<float>(cpu.ebx + x86::reg32(4) /* 0x4 */) = float(fp1);
    // 00493bd8  d95b08                 +fstp dword ptr [ebx + 8]
    app->getMemory<float>(cpu.ebx + x86::reg32(8) /* 0x8 */) = float(fp0);
    // 00493bdb  d9ee                   +fldz 
    fp0 = 0.0;
    // 00493bdd  d85d98                 +fcomp dword ptr [ebp - 0x68]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-104) /* -0x68 */)));
    // 00493be0  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00493be2  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00493be3  7308                   -jae 0x493bed
    if (!cpu.flags.cf)
    {
        goto L_0x00493bed;
    }
    // 00493be5  8b4598                 -mov eax, dword ptr [ebp - 0x68]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-104) /* -0x68 */);
    // 00493be8  8945e8                 -mov dword ptr [ebp - 0x18], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = cpu.eax;
    // 00493beb  eb08                   -jmp 0x493bf5
    goto L_0x00493bf5;
L_0x00493bed:
    // 00493bed  d94598                 +fld dword ptr [ebp - 0x68]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-104) /* -0x68 */));
    // 00493bf0  d9e0                   +fchs 
    fp0 = -fp0;
    // 00493bf2  d95de8                 +fstp dword ptr [ebp - 0x18]
    app->getMemory<float>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = float(fp0);
L_0x00493bf5:
    // 00493bf5  d945e8                 +fld dword ptr [ebp - 0x18]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-24) /* -0x18 */));
    // 00493bf8  d85db0                 +fcomp dword ptr [ebp - 0x50]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-80) /* -0x50 */)));
    // 00493bfb  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00493bfd  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00493bfe  0f83d9010000           -jae 0x493ddd
    if (!cpu.flags.cf)
    {
        goto L_0x00493ddd;
    }
    // 00493c04  d9ee                   +fldz 
    fp0 = 0.0;
    // 00493c06  d85d9c                 +fcomp dword ptr [ebp - 0x64]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-100) /* -0x64 */)));
    // 00493c09  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00493c0b  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00493c0c  7308                   -jae 0x493c16
    if (!cpu.flags.cf)
    {
        goto L_0x00493c16;
    }
    // 00493c0e  8b459c                 -mov eax, dword ptr [ebp - 0x64]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-100) /* -0x64 */);
    // 00493c11  8945ec                 -mov dword ptr [ebp - 0x14], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.eax;
    // 00493c14  eb08                   -jmp 0x493c1e
    goto L_0x00493c1e;
L_0x00493c16:
    // 00493c16  d9459c                 +fld dword ptr [ebp - 0x64]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-100) /* -0x64 */));
    // 00493c19  d9e0                   +fchs 
    fp0 = -fp0;
    // 00493c1b  d95dec                 +fstp dword ptr [ebp - 0x14]
    app->getMemory<float>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = float(fp0);
L_0x00493c1e:
    // 00493c1e  d945ec                 +fld dword ptr [ebp - 0x14]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-20) /* -0x14 */));
    // 00493c21  d85db4                 +fcomp dword ptr [ebp - 0x4c]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-76) /* -0x4c */)));
    // 00493c24  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00493c26  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00493c27  0f83b0010000           -jae 0x493ddd
    if (!cpu.flags.cf)
    {
        goto L_0x00493ddd;
    }
    // 00493c2d  d9ee                   +fldz 
    fp0 = 0.0;
    // 00493c2f  d85da0                 +fcomp dword ptr [ebp - 0x60]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-96) /* -0x60 */)));
    // 00493c32  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00493c34  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00493c35  7308                   -jae 0x493c3f
    if (!cpu.flags.cf)
    {
        goto L_0x00493c3f;
    }
    // 00493c37  8b45a0                 -mov eax, dword ptr [ebp - 0x60]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-96) /* -0x60 */);
    // 00493c3a  8945e4                 -mov dword ptr [ebp - 0x1c], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */) = cpu.eax;
    // 00493c3d  eb08                   -jmp 0x493c47
    goto L_0x00493c47;
L_0x00493c3f:
    // 00493c3f  d945a0                 +fld dword ptr [ebp - 0x60]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-96) /* -0x60 */));
    // 00493c42  d9e0                   +fchs 
    fp0 = -fp0;
    // 00493c44  d95de4                 +fstp dword ptr [ebp - 0x1c]
    app->getMemory<float>(cpu.ebp + x86::reg32(-28) /* -0x1c */) = float(fp0);
L_0x00493c47:
    // 00493c47  d945e4                 +fld dword ptr [ebp - 0x1c]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-28) /* -0x1c */));
    // 00493c4a  d85db8                 +fcomp dword ptr [ebp - 0x48]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-72) /* -0x48 */)));
    // 00493c4d  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00493c4f  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00493c50  0f8387010000           -jae 0x493ddd
    if (!cpu.flags.cf)
    {
        goto L_0x00493ddd;
    }
    // 00493c56  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00493c59  8d7d8c                 -lea edi, [ebp - 0x74]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-116) /* -0x74 */);
    // 00493c5c  8db0c0000000           -lea esi, [eax + 0xc0]
    cpu.esi = x86::reg32(cpu.eax + x86::reg32(192) /* 0xc0 */);
    // 00493c62  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00493c63  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00493c64  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00493c65  d9458c                 -fld dword ptr [ebp - 0x74]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-116) /* -0x74 */));
    // 00493c68  d9c0                   -fld st(0)
    fp1 = x86::Float(fp0);
    // 00493c6a  dd0558bd5300           -fld qword ptr [0x53bd58]
    fp2 = x86::Float(app->getMemory<double>(x86::reg32(5487960) /* 0x53bd58 */));
    // 00493c70  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp2;
        fp2 = fp1;
        fp1 = tmp;
    }
    // 00493c72  d8c9                   -fmul st(1)
    fp2 = fp::mul(fp2, x86::Float(fp1));
    // 00493c74  d94590                 -fld dword ptr [ebp - 0x70]
    fp3 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-112) /* -0x70 */));
    // 00493c77  d9c0                   -fld st(0)
    fp4 = x86::Float(fp3);
    // 00493c79  d8cb                   -fmul st(3)
    fp4 = fp::mul(fp4, x86::Float(fp1));
    // 00493c7b  d94594                 -fld dword ptr [ebp - 0x6c]
    fp5 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-108) /* -0x6c */));
    // 00493c7e  dccc                   -fmul st(4), st(0)
    fp1 = fp::mul(fp1, fp5);
    // 00493c80  d9cb                   -fxch st(3)
    {
        x86::Float tmp = fp5;
        fp5 = fp2;
        fp2 = tmp;
    }
    // 00493c82  dddd                   -fstp st(5)
    fp0 = x86::Float(fp5);
    // 00493c84  ddd9                   -fstp st(1)
    fp3 = x86::Float(fp4);
    // 00493c86  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp3;
        fp3 = fp1;
        fp1 = tmp;
    }
    // 00493c88  ddd9                   -fstp st(1)
    fp2 = x86::Float(fp3);
    // 00493c8a  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 00493c8c  d95d8c                 -fstp dword ptr [ebp - 0x74]
    app->getMemory<float>(cpu.ebp + x86::reg32(-116) /* -0x74 */) = float(fp2);
    // 00493c8f  d95d90                 -fstp dword ptr [ebp - 0x70]
    app->getMemory<float>(cpu.ebp + x86::reg32(-112) /* -0x70 */) = float(fp1);
    // 00493c92  d95d94                 -fstp dword ptr [ebp - 0x6c]
    app->getMemory<float>(cpu.ebp + x86::reg32(-108) /* -0x6c */) = float(fp0);
    // 00493c95  8b9068010000           -mov edx, dword ptr [eax + 0x168]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(360) /* 0x168 */);
    // 00493c9b  f7c2ffffff7f           +test edx, 0x7fffffff
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & 2147483647 /*0x7fffffff*/));
    // 00493ca1  7544                   -jne 0x493ce7
    if (!cpu.flags.zf)
    {
        goto L_0x00493ce7;
    }
    // 00493ca3  c7806801000000007041   -mov dword ptr [eax + 0x168], 0x41700000
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(360) /* 0x168 */) = 1097859072 /*0x41700000*/;
    // 00493cad  c7807001000009000600   -mov dword ptr [eax + 0x170], 0x60009
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(368) /* 0x170 */) = 393225 /*0x60009*/;
    // 00493cb7  c7806c01000000000000   -mov dword ptr [eax + 0x16c], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(364) /* 0x16c */) = 0 /*0x0*/;
    // 00493cc1  d98098000000           +fld dword ptr [eax + 0x98]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(152) /* 0x98 */));
    // 00493cc7  d9809c000000           +fld dword ptr [eax + 0x9c]
    fp1 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(156) /* 0x9c */));
    // 00493ccd  d980a0000000           +fld dword ptr [eax + 0xa0]
    fp2 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(160) /* 0xa0 */));
    // 00493cd3  d9ca                   +fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 00493cd5  d99878010000           +fstp dword ptr [eax + 0x178]
    app->getMemory<float>(cpu.eax + x86::reg32(376) /* 0x178 */) = float(fp2);
    // 00493cdb  d9987c010000           +fstp dword ptr [eax + 0x17c]
    app->getMemory<float>(cpu.eax + x86::reg32(380) /* 0x17c */) = float(fp1);
    // 00493ce1  d99880010000           +fstp dword ptr [eax + 0x180]
    app->getMemory<float>(cpu.eax + x86::reg32(384) /* 0x180 */) = float(fp0);
L_0x00493ce7:
    // 00493ce7  d9ee                   +fldz 
    fp0 = 0.0;
    // 00493ce9  d85d98                 +fcomp dword ptr [ebp - 0x68]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-104) /* -0x68 */)));
    // 00493cec  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00493cee  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00493cef  761b                   -jbe 0x493d0c
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00493d0c;
    }
    // 00493cf1  8a5593                 -mov dl, byte ptr [ebp - 0x6d]
    cpu.dl = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-109) /* -0x6d */);
    // 00493cf4  80f280                 -xor dl, 0x80
    cpu.dl ^= x86::reg8(x86::sreg8(128 /*0x80*/));
    // 00493cf7  8a658f                 -mov ah, byte ptr [ebp - 0x71]
    cpu.ah = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-113) /* -0x71 */);
    // 00493cfa  885593                 -mov byte ptr [ebp - 0x6d], dl
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-109) /* -0x6d */) = cpu.dl;
    // 00493cfd  8a7597                 -mov dh, byte ptr [ebp - 0x69]
    cpu.dh = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-105) /* -0x69 */);
    // 00493d00  80f480                 -xor ah, 0x80
    cpu.ah ^= x86::reg8(x86::sreg8(128 /*0x80*/));
    // 00493d03  80f680                 -xor dh, 0x80
    cpu.dh ^= x86::reg8(x86::sreg8(128 /*0x80*/));
    // 00493d06  88658f                 -mov byte ptr [ebp - 0x71], ah
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-113) /* -0x71 */) = cpu.ah;
    // 00493d09  887597                 -mov byte ptr [ebp - 0x69], dh
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-105) /* -0x69 */) = cpu.dh;
L_0x00493d0c:
    // 00493d0c  8b4df8                 -mov ecx, dword ptr [ebp - 8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00493d0f  8b5dfc                 -mov ebx, dword ptr [ebp - 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00493d12  8d558c                 -lea edx, [ebp - 0x74]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-116) /* -0x74 */);
    // 00493d15  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00493d1a  83c144                 -add ecx, 0x44
    (cpu.ecx) += x86::reg32(x86::sreg32(68 /*0x44*/));
    // 00493d1d  81c3a4000000           -add ebx, 0xa4
    (cpu.ebx) += x86::reg32(x86::sreg32(164 /*0xa4*/));
    // 00493d23  e888c20400             -call 0x4dffb0
    cpu.esp -= 4;
    nfs3hp::vertices::addVectors(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00493d28  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00493d2b  d980b8000000           -fld dword ptr [eax + 0xb8]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(184) /* 0xb8 */));
    // 00493d31  dc0d60bd5300           -fmul qword ptr [0x53bd60]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5487968) /* 0x53bd60 */)));
    // 00493d37  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00493d3a  d9e8                   -fld1 
    fp1 = 1.0;
    // 00493d3c  83c248                 +add edx, 0x48
    {
        x86::reg32& tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(72 /*0x48*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00493d3f  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 00493d41  dd5dc4                 +fstp qword ptr [ebp - 0x3c]
    app->getMemory<double>(cpu.ebp + x86::reg32(-60) /* -0x3c */) = double(fp1);
    // 00493d44  dc5dc4                 +fcomp qword ptr [ebp - 0x3c]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<double>(cpu.ebp + x86::reg32(-60) /* -0x3c */)));
    // 00493d47  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00493d49  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00493d4a  760e                   -jbe 0x493d5a
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00493d5a;
    }
    // 00493d4c  8b45c4                 -mov eax, dword ptr [ebp - 0x3c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-60) /* -0x3c */);
    // 00493d4f  8945dc                 -mov dword ptr [ebp - 0x24], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = cpu.eax;
    // 00493d52  8b45c8                 -mov eax, dword ptr [ebp - 0x38]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-56) /* -0x38 */);
    // 00493d55  8945e0                 -mov dword ptr [ebp - 0x20], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */) = cpu.eax;
    // 00493d58  eb0d                   -jmp 0x493d67
    goto L_0x00493d67;
L_0x00493d5a:
    // 00493d5a  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00493d5c  bb0000f03f             -mov ebx, 0x3ff00000
    cpu.ebx = 1072693248 /*0x3ff00000*/;
    // 00493d61  894ddc                 -mov dword ptr [ebp - 0x24], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = cpu.ecx;
    // 00493d64  895de0                 -mov dword ptr [ebp - 0x20], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */) = cpu.ebx;
L_0x00493d67:
    // 00493d67  dd45dc                 -fld qword ptr [ebp - 0x24]
    fp0 = x86::Float(app->getMemory<double>(cpu.ebp + x86::reg32(-36) /* -0x24 */));
    // 00493d6a  dc0d68bd5300           -fmul qword ptr [0x53bd68]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5487976) /* 0x53bd68 */)));
    // 00493d70  d902                   -fld dword ptr [edx]
    fp1 = x86::Float(app->getMemory<float>(cpu.edx));
    // 00493d72  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 00493d74  d8c1                   -fadd st(1)
    fp1 = fp::add(fp1, x86::Float(fp0));
    // 00493d76  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00493d79  ddd9                   -fstp st(1)
    fp0 = x86::Float(fp1);
    // 00493d7b  d91a                   -fstp dword ptr [edx]
    app->getMemory<float>(cpu.edx) = float(fp0);
    // 00493d7d  8d5008                 -lea edx, [eax + 8]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 00493d80  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00493d81  66c740028000           -mov word ptr [eax + 2], 0x80
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(2) /* 0x2 */) = 128 /*0x80*/;
    // 00493d87  e8a4740500             -call 0x4eb230
    cpu.esp -= 4;
    nfs3hp::ea::randomFraction(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 00493d8c  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00493d8f  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00493d91  d91c24                 -fstp dword ptr [esp]
    app->getMemory<float>(cpu.esp) = float(fp0);
    // 00493d94  e8476c0500             -call 0x4ea9e0
    cpu.esp -= 4;
    sub_4ea9e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00493d99  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00493d9a  e891740500             -call 0x4eb230
    cpu.esp -= 4;
    nfs3hp::ea::randomFraction(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 00493d9f  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00493da2  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00493da4  d91c24                 -fstp dword ptr [esp]
    app->getMemory<float>(cpu.esp) = float(fp0);
    // 00493da7  e8046d0500             -call 0x4eaab0
    cpu.esp -= 4;
    sub_4eaab0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00493dac  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00493dad  e87e740500             -call 0x4eb230
    cpu.esp -= 4;
    nfs3hp::ea::randomFraction(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 00493db2  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00493db5  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00493db7  d91c24                 -fstp dword ptr [esp]
    app->getMemory<float>(cpu.esp) = float(fp0);
    // 00493dba  e8c16d0500             -call 0x4eab80
    cpu.esp -= 4;
    sub_4eab80(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00493dbf  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00493dc2  f6800002000001         +test byte ptr [eax + 0x200], 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(512) /* 0x200 */) & 1 /*0x1*/));
    // 00493dc9  7409                   -je 0x493dd4
    if (cpu.flags.zf)
    {
        goto L_0x00493dd4;
    }
    // 00493dcb  83b8c402000003         +cmp dword ptr [eax + 0x2c4], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(708) /* 0x2c4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00493dd2  7409                   -je 0x493ddd
    if (cpu.flags.zf)
    {
        goto L_0x00493ddd;
    }
L_0x00493dd4:
    // 00493dd4  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00493dd7  ff80d8020000           -inc dword ptr [eax + 0x2d8]
    (app->getMemory<x86::reg32>(cpu.eax + x86::reg32(728) /* 0x2d8 */))++;
L_0x00493ddd:
    // 00493ddd  ff45f0                 -inc dword ptr [ebp - 0x10]
    (app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */))++;
    // 00493de0  837df002               +cmp dword ptr [ebp - 0x10], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00493de4  7d2b                   -jge 0x493e11
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00493e11;
    }
L_0x00493de6:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00493de6  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00493de9  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00493deb  0f8c30fdffff           -jl 0x493b21
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00493b21;
    }
    // 00493df1  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00493df4  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00493df7  8b401c                 -mov eax, dword ptr [eax + 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(28) /* 0x1c */);
    // 00493dfa  8b0d40d95d00           -mov ecx, dword ptr [0x5dd940]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(6150464) /* 0x5dd940 */);
    // 00493e00  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00493e02  39c8                   +cmp eax, ecx
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
    // 00493e04  0f8c2cfdffff           -jl 0x493b36
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00493b36;
    }
    // 00493e0a  29c8                   +sub eax, ecx
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ecx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00493e0c  e925fdffff             -jmp 0x493b36
    goto L_0x00493b36;
L_0x00493e11:
    // 00493e11  ff45f4                 -inc dword ptr [ebp - 0xc]
    (app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */))++;
    // 00493e14  837df403               +cmp dword ptr [ebp - 0xc], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00493e18  7d07                   -jge 0x493e21
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00493e21;
    }
    // 00493e1a  31f6                   +xor esi, esi
    cpu.clear_co();
    cpu.set_szp((cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi))));
    // 00493e1c  8975f0                 -mov dword ptr [ebp - 0x10], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.esi;
    // 00493e1f  ebc5                   -jmp 0x493de6
    goto L_0x00493de6;
L_0x00493e21:
    // 00493e21  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00493e24  f6800002000001         +test byte ptr [eax + 0x200], 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(512) /* 0x200 */) & 1 /*0x1*/));
    // 00493e2b  7409                   -je 0x493e36
    if (cpu.flags.zf)
    {
        goto L_0x00493e36;
    }
    // 00493e2d  83b8c402000003         +cmp dword ptr [eax + 0x2c4], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(708) /* 0x2c4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00493e34  7408                   -je 0x493e3e
    if (cpu.flags.zf)
    {
        goto L_0x00493e3e;
    }
L_0x00493e36:
    // 00493e36  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00493e39  e8f2fbffff             -call 0x493a30
    cpu.esp -= 4;
    sub_493a30(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00493e3e:
    // 00493e3e  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00493e40  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00493e41  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00493e42  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00493e43  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00493e44  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00493e45  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00493e46  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_493e50(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00493e50  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00493e51  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00493e52  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00493e53  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00493e54  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00493e55  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00493e56  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00493e58  833d98d46f0000         +cmp dword ptr [0x6fd498], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328920) /* 0x6fd498 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00493e5f  0f847a000000           -je 0x493edf
    if (cpu.flags.zf)
    {
        goto L_0x00493edf;
    }
    // 00493e65  a184367d00             -mov eax, dword ptr [0x7d3684]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8205956) /* 0x7d3684 */);
    // 00493e6a  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00493e6c  a39ce45500             -mov dword ptr [0x55e49c], eax
    app->getMemory<x86::reg32>(x86::reg32(5629084) /* 0x55e49c */) = cpu.eax;
    // 00493e71  3b1540d95d00           +cmp edx, dword ptr [0x5dd940]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(6150464) /* 0x5dd940 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00493e77  7d66                   -jge 0x493edf
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00493edf;
    }
    // 00493e79  31c9                   +xor ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx))));
    // 00493e7b  eb05                   -jmp 0x493e82
    goto L_0x00493e82;
L_0x00493e7d:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00493e7d  83f902                 +cmp ecx, 2
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
    // 00493e80  7d50                   -jge 0x493ed2
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00493ed2;
    }
L_0x00493e82:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00493e82  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00493e89  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00493e8b  8b348da0e45500         -mov esi, dword ptr [ecx*4 + 0x55e4a0]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(5629088) /* 0x55e4a0 */ + cpu.ecx * 4);
    // 00493e92  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 00493e95  66c74406020000         -mov word ptr [esi + eax + 2], 0
    app->getMemory<x86::reg16>(cpu.esi + x86::reg32(2) /* 0x2 */ + cpu.eax * 1) = 0 /*0x0*/;
    // 00493e9c  8b1c8da0e45500         -mov ebx, dword ptr [ecx*4 + 0x55e4a0]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5629088) /* 0x55e4a0 */ + cpu.ecx * 4);
    // 00493ea3  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 00493ea5  66833800               +cmp word ptr [eax], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.eax);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 00493ea9  7e05                   -jle 0x493eb0
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00493eb0;
    }
    // 00493eab  66c7000000             -mov word ptr [eax], 0
    app->getMemory<x86::reg16>(cpu.eax) = 0 /*0x0*/;
L_0x00493eb0:
    // 00493eb0  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00493eb7  8d1c02                 -lea ebx, [edx + eax]
    cpu.ebx = x86::reg32(cpu.edx + cpu.eax * 1);
    // 00493eba  c1e304                 +shl ebx, 4
    {
        x86::reg8 tmp = 4 /*0x4*/ % 32;
        x86::reg32& op = cpu.ebx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 00493ebd  8b048da0e45500         -mov eax, dword ptr [ecx*4 + 0x55e4a0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629088) /* 0x55e4a0 */ + cpu.ecx * 4);
    // 00493ec4  8d7c0338               -lea edi, [ebx + eax + 0x38]
    cpu.edi = x86::reg32(cpu.ebx + x86::reg32(56) /* 0x38 */ + cpu.eax * 1);
    // 00493ec8  8d74032c               -lea esi, [ebx + eax + 0x2c]
    cpu.esi = x86::reg32(cpu.ebx + x86::reg32(44) /* 0x2c */ + cpu.eax * 1);
    // 00493ecc  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00493ecd  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00493ece  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00493ecf  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00493ed0  ebab                   -jmp 0x493e7d
    goto L_0x00493e7d;
L_0x00493ed2:
    // 00493ed2  42                     -inc edx
    (cpu.edx)++;
    // 00493ed3  3b1540d95d00           +cmp edx, dword ptr [0x5dd940]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(6150464) /* 0x5dd940 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00493ed9  7d04                   -jge 0x493edf
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00493edf;
    }
    // 00493edb  31c9                   +xor ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx))));
    // 00493edd  eba3                   -jmp 0x493e82
    goto L_0x00493e82;
L_0x00493edf:
    // 00493edf  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00493ee0  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00493ee1  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00493ee2  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00493ee3  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00493ee4  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00493ee5  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x54 0x22 0x00 */
void Application::sub_493ef0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00493ef0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00493ef1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00493ef2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00493ef3  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00493ef4  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00493ef6  81ec00020000           -sub esp, 0x200
    (cpu.esp) -= x86::reg32(x86::sreg32(512 /*0x200*/));
    // 00493efc  833d98d46f0000         +cmp dword ptr [0x6fd498], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328920) /* 0x6fd498 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00493f03  0f849e000000           -je 0x493fa7
    if (cpu.flags.zf)
    {
        goto L_0x00493fa7;
    }
    // 00493f09  8b0da0e45500           -mov ecx, dword ptr [0x55e4a0]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5629088) /* 0x55e4a0 */);
    // 00493f0f  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 00493f11  7552                   -jne 0x493f65
    if (!cpu.flags.zf)
    {
        goto L_0x00493f65;
    }
    // 00493f13  eb05                   -jmp 0x493f1a
    goto L_0x00493f1a;
L_0x00493f15:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00493f15  83f902                 +cmp ecx, 2
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
    // 00493f18  7d46                   -jge 0x493f60
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00493f60;
    }
L_0x00493f1a:
    // 00493f1a  8b1540d95d00           -mov edx, dword ptr [0x5dd940]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(6150464) /* 0x5dd940 */);
    // 00493f20  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00493f27  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00493f29  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00493f2b  b870bd5300             -mov eax, 0x53bd70
    cpu.eax = 5487984 /*0x53bd70*/;
    // 00493f30  c1e204                 -shl edx, 4
    cpu.edx <<= 4 /*0x4*/ % 32;
    // 00493f33  e8e8d60400             -call 0x4e1620
    cpu.esp -= 4;
    nfs3hp::ea::reservememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00493f38  8b1540d95d00           -mov edx, dword ptr [0x5dd940]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(6150464) /* 0x5dd940 */);
    // 00493f3e  89048da0e45500         -mov dword ptr [ecx*4 + 0x55e4a0], eax
    app->getMemory<x86::reg32>(x86::reg32(5629088) /* 0x55e4a0 */ + cpu.ecx * 4) = cpu.eax;
    // 00493f45  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00493f4c  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00493f4e  c1e204                 +shl edx, 4
    {
        x86::reg8 tmp = 4 /*0x4*/ % 32;
        x86::reg32& op = cpu.edx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 00493f51  8b048da0e45500         -mov eax, dword ptr [ecx*4 + 0x55e4a0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629088) /* 0x55e4a0 */ + cpu.ecx * 4);
    // 00493f58  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00493f59  e8aec70400             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00493f5e  ebb5                   -jmp 0x493f15
    goto L_0x00493f15;
L_0x00493f60:
    // 00493f60  e8dbf6ffff             -call 0x493640
    cpu.esp -= 4;
    sub_493640(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00493f65:
    // 00493f65  8b1da8e45500           -mov ebx, dword ptr [0x55e4a8]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5629096) /* 0x55e4a8 */);
    // 00493f6b  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 00493f6d  752e                   -jne 0x493f9d
    if (!cpu.flags.zf)
    {
        goto L_0x00493f9d;
    }
    // 00493f6f  6834297a00             -push 0x7a2934
    app->getMemory<x86::reg32>(cpu.esp-4) = 8005940 /*0x7a2934*/;
    cpu.esp -= 4;
    // 00493f74  6874bd5300             -push 0x53bd74
    app->getMemory<x86::reg32>(cpu.esp-4) = 5487988 /*0x53bd74*/;
    cpu.esp -= 4;
    // 00493f79  8d8500feffff           -lea eax, [ebp - 0x200]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-512) /* -0x200 */);
    // 00493f7f  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00493f80  e80bb70400             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00493f85  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 00493f88  ba08000000             -mov edx, 8
    cpu.edx = 8 /*0x8*/;
    // 00493f8d  8d8500feffff           -lea eax, [ebp - 0x200]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-512) /* -0x200 */);
    // 00493f93  e8b8940000             -call 0x49d450
    cpu.esp -= 4;
    nfs3hp::load::loadCarModel(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00493f98  a3a8e45500             -mov dword ptr [0x55e4a8], eax
    app->getMemory<x86::reg32>(x86::reg32(5629096) /* 0x55e4a8 */) = cpu.eax;
L_0x00493f9d:
    // 00493f9d  a184367d00             -mov eax, dword ptr [0x7d3684]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8205956) /* 0x7d3684 */);
    // 00493fa2  a39ce45500             -mov dword ptr [0x55e49c], eax
    app->getMemory<x86::reg32>(x86::reg32(5629084) /* 0x55e49c */) = cpu.eax;
L_0x00493fa7:
    // 00493fa7  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00493fa9  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00493faa  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00493fab  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00493fac  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00493fad  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_493fb0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00493fb0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00493fb1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00493fb2  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00493fb3  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00493fb4  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00493fb6  833d98d46f0000         +cmp dword ptr [0x6fd498], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328920) /* 0x6fd498 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00493fbd  7441                   -je 0x494000
    if (cpu.flags.zf)
    {
        goto L_0x00494000;
    }
    // 00493fbf  8b0da0e45500           -mov ecx, dword ptr [0x55e4a0]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5629088) /* 0x55e4a0 */);
    // 00493fc5  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 00493fc7  741f                   -je 0x493fe8
    if (cpu.flags.zf)
    {
        goto L_0x00493fe8;
    }
    // 00493fc9  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00493fcb  e8c0d80400             -call 0x4e1890
    cpu.esp -= 4;
    nfs3hp::ea::purgememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00493fd0  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00493fd2  a1a4e45500             -mov eax, dword ptr [0x55e4a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629092) /* 0x55e4a4 */);
    // 00493fd7  891da0e45500           -mov dword ptr [0x55e4a0], ebx
    app->getMemory<x86::reg32>(x86::reg32(5629088) /* 0x55e4a0 */) = cpu.ebx;
    // 00493fdd  e8aed80400             -call 0x4e1890
    cpu.esp -= 4;
    nfs3hp::ea::purgememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00493fe2  891da4e45500           -mov dword ptr [0x55e4a4], ebx
    app->getMemory<x86::reg32>(x86::reg32(5629092) /* 0x55e4a4 */) = cpu.ebx;
L_0x00493fe8:
    // 00493fe8  8b3da8e45500           -mov edi, dword ptr [0x55e4a8]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(5629096) /* 0x55e4a8 */);
    // 00493fee  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 00493ff0  740e                   -je 0x494000
    if (cpu.flags.zf)
    {
        goto L_0x00494000;
    }
    // 00493ff2  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 00493ff4  e8a7950000             -call 0x49d5a0
    cpu.esp -= 4;
    sub_49d5a0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00493ff9  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00493ffb  a3a8e45500             -mov dword ptr [0x55e4a8], eax
    app->getMemory<x86::reg32>(x86::reg32(5629096) /* 0x55e4a8 */) = cpu.eax;
L_0x00494000:
    // 00494000  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00494001  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00494002  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00494003  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00494004  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_494010(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1, fp2, fp3;
  x86::reg16 fsw = 0;
    // 00494010  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00494011  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00494012  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00494013  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00494014  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00494015  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00494016  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00494018  83ec14                 -sub esp, 0x14
    (cpu.esp) -= x86::reg32(x86::sreg32(20 /*0x14*/));
    // 0049401b  8d7dec                 -lea edi, [ebp - 0x14]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0049401e  be70334900             -mov esi, 0x493370
    cpu.esi = 4797296 /*0x493370*/;
    // 00494023  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00494024  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00494025  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
L_0x00494026:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00494026  8b1584367d00           -mov edx, dword ptr [0x7d3684]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205956) /* 0x7d3684 */);
    // 0049402c  a19ce45500             -mov eax, dword ptr [0x55e49c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629084) /* 0x55e49c */);
    // 00494031  39d0                   +cmp eax, edx
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
    // 00494033  0f8d25010000           -jge 0x49415e
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0049415e;
    }
    // 00494039  8d5801                 -lea ebx, [eax + 1]
    cpu.ebx = x86::reg32(cpu.eax + x86::reg32(1) /* 0x1 */);
    // 0049403c  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0049403e  891d9ce45500           -mov dword ptr [0x55e49c], ebx
    app->getMemory<x86::reg32>(x86::reg32(5629084) /* 0x55e49c */) = cpu.ebx;
    // 00494044  894dfc                 -mov dword ptr [ebp - 4], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
L_0x00494047:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00494047  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0049404a  3b0540d95d00           +cmp eax, dword ptr [0x5dd940]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(6150464) /* 0x5dd940 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00494050  7dd4                   -jge 0x494026
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00494026;
    }
    // 00494052  31ff                   +xor edi, edi
    cpu.clear_co();
    cpu.set_szp((cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi))));
    // 00494054  897df8                 -mov dword ptr [ebp - 8], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.edi;
    // 00494057  e9c1000000             -jmp 0x49411d
    goto L_0x0049411d;
L_0x0049405c:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049405c  d9057cbd5300           +fld dword ptr [0x53bd7c]
    fp0 = x86::Float(app->getMemory<float>(x86::reg32(5487996) /* 0x53bd7c */));
    // 00494062  d94644                 +fld dword ptr [esi + 0x44]
    fp1 = x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(68) /* 0x44 */));
    // 00494065  d8c9                   +fmul st(1)
    fp1 = fp::mul(fp1, x86::Float(fp0));
    // 00494067  d94648                 +fld dword ptr [esi + 0x48]
    fp2 = x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(72) /* 0x48 */));
    // 0049406a  d8ca                   +fmul st(2)
    fp2 = fp::mul(fp2, x86::Float(fp0));
    // 0049406c  d9464c                 +fld dword ptr [esi + 0x4c]
    fp3 = x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(76) /* 0x4c */));
    // 0049406f  decb                   +fmulp st(3)
    fp0 = fp::mul(fp0, fp3);
    // 00494071  8d5dec                 -lea ebx, [ebp - 0x14]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00494074  8d5644                 -lea edx, [esi + 0x44]
    cpu.edx = x86::reg32(cpu.esi + x86::reg32(68) /* 0x44 */);
    // 00494077  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0049407c  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 0049407e  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp2;
        fp2 = fp1;
        fp1 = tmp;
    }
    // 00494080  d84638                 +fadd dword ptr [esi + 0x38]
    fp2 = fp::add(fp2, x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(56) /* 0x38 */)));
    // 00494083  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp2;
        fp2 = fp1;
        fp1 = tmp;
    }
    // 00494085  d8463c                 +fadd dword ptr [esi + 0x3c]
    fp2 = fp::add(fp2, x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(60) /* 0x3c */)));
    // 00494088  d9ca                   +fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 0049408a  d84640                 +fadd dword ptr [esi + 0x40]
    fp2 = fp::add(fp2, x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(64) /* 0x40 */)));
    // 0049408d  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp2;
        fp2 = fp1;
        fp1 = tmp;
    }
    // 0049408f  d95e38                 +fstp dword ptr [esi + 0x38]
    app->getMemory<float>(cpu.esi + x86::reg32(56) /* 0x38 */) = float(fp2);
    // 00494092  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 00494094  d95e3c                 +fstp dword ptr [esi + 0x3c]
    app->getMemory<float>(cpu.esi + x86::reg32(60) /* 0x3c */) = float(fp1);
    // 00494097  d95e40                 +fstp dword ptr [esi + 0x40]
    app->getMemory<float>(cpu.esi + x86::reg32(64) /* 0x40 */) = float(fp0);
    // 0049409a  e811bf0400             -call 0x4dffb0
    cpu.esp -= 4;
    nfs3hp::vertices::addVectors(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049409f  d9463c                 +fld dword ptr [esi + 0x3c]
    fp0 = x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(60) /* 0x3c */));
    // 004940a2  d85e30                 +fcomp dword ptr [esi + 0x30]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(48) /* 0x30 */)));
    // 004940a5  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004940a7  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004940a8  730e                   -jae 0x4940b8
    if (!cpu.flags.cf)
    {
        goto L_0x004940b8;
    }
    // 004940aa  d94648                 -fld dword ptr [esi + 0x48]
    fp0 = x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(72) /* 0x48 */));
    // 004940ad  dc0d80bd5300           -fmul qword ptr [0x53bd80]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5488000) /* 0x53bd80 */)));
    // 004940b3  d9e0                   -fchs 
    fp0 = -fp0;
    // 004940b5  d95e48                 -fstp dword ptr [esi + 0x48]
    app->getMemory<float>(cpu.esi + x86::reg32(72) /* 0x48 */) = float(fp0);
L_0x004940b8:
    // 004940b8  d90588bd5300           -fld dword ptr [0x53bd88]
    fp0 = x86::Float(app->getMemory<float>(x86::reg32(5488008) /* 0x53bd88 */));
    // 004940be  d94644                 -fld dword ptr [esi + 0x44]
    fp1 = x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(68) /* 0x44 */));
    // 004940c1  d8c9                   -fmul st(1)
    fp1 = fp::mul(fp1, x86::Float(fp0));
    // 004940c3  d9464c                 -fld dword ptr [esi + 0x4c]
    fp2 = x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(76) /* 0x4c */));
    // 004940c6  83c608                 +add esi, 8
    {
        x86::reg32& tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(8 /*0x8*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004940c9  deca                   +fmulp st(2)
    fp0 = fp::mul(fp0, fp2);
    // 004940cb  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004940cc  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004940ce  68cdcccc3c             -push 0x3ccccccd
    app->getMemory<x86::reg32>(cpu.esp-4) = 1020054733 /*0x3ccccccd*/;
    cpu.esp -= 4;
    // 004940d3  d95e3c                 +fstp dword ptr [esi + 0x3c]
    app->getMemory<float>(cpu.esi + x86::reg32(60) /* 0x3c */) = float(fp1);
    // 004940d6  d95e44                 +fstp dword ptr [esi + 0x44]
    app->getMemory<float>(cpu.esi + x86::reg32(68) /* 0x44 */) = float(fp0);
    // 004940d9  e802690500             -call 0x4ea9e0
    cpu.esp -= 4;
    sub_4ea9e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004940de  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004940df  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004940e1  68cdccccbc             -push 0xbccccccd
    app->getMemory<x86::reg32>(cpu.esp-4) = 3167538381 /*0xbccccccd*/;
    cpu.esp -= 4;
    // 004940e6  e8c5690500             -call 0x4eaab0
    cpu.esp -= 4;
    sub_4eaab0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004940eb  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004940ec  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004940ee  688fc2753c             -push 0x3c75c28f
    app->getMemory<x86::reg32>(cpu.esp-4) = 1014350479 /*0x3c75c28f*/;
    cpu.esp -= 4;
    // 004940f3  e8886a0500             -call 0x4eab80
    cpu.esp -= 4;
    sub_4eab80(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004940f8  eb1a                   -jmp 0x494114
    goto L_0x00494114;
L_0x004940fa:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004940fa  668b3e                 -mov di, word ptr [esi]
    cpu.di = app->getMemory<x86::reg16>(cpu.esi);
    // 004940fd  6685ff                 +test di, di
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg16>(cpu.di & cpu.di));
    // 00494100  7e12                   -jle 0x494114
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00494114;
    }
    // 00494102  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 00494104  6648                   +dec ax
    {
        x86::reg16& tmp = cpu.ax;
        cpu.flags.of = 1 & (tmp >> 15);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 15));
        cpu.set_szp(tmp);
    }
    // 00494106  668906                 -mov word ptr [esi], ax
    app->getMemory<x86::reg16>(cpu.esi) = cpu.ax;
    // 00494109  7509                   -jne 0x494114
    if (!cpu.flags.zf)
    {
        goto L_0x00494114;
    }
    // 0049410b  8d7e38                 -lea edi, [esi + 0x38]
    cpu.edi = x86::reg32(cpu.esi + x86::reg32(56) /* 0x38 */);
    // 0049410e  8d762c                 -lea esi, [esi + 0x2c]
    cpu.esi = x86::reg32(cpu.esi + x86::reg32(44) /* 0x2c */);
    // 00494111  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00494112  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00494113  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
L_0x00494114:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00494114  ff45f8                 -inc dword ptr [ebp - 8]
    (app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */))++;
    // 00494117  837df802               +cmp dword ptr [ebp - 8], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049411b  7d39                   -jge 0x494156
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00494156;
    }
L_0x0049411d:
    // 0049411d  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00494120  8d348500000000         -lea esi, [eax*4]
    cpu.esi = x86::reg32(cpu.eax * 4);
    // 00494127  01c6                   -add esi, eax
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.eax));
    // 00494129  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0049412c  c1e604                 -shl esi, 4
    cpu.esi <<= 4 /*0x4*/ % 32;
    // 0049412f  8b0495a0e45500         -mov eax, dword ptr [edx*4 + 0x55e4a0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629088) /* 0x55e4a0 */ + cpu.edx * 4);
    // 00494136  01c6                   -add esi, eax
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.eax));
    // 00494138  668b5602               -mov dx, word ptr [esi + 2]
    cpu.dx = app->getMemory<x86::reg16>(cpu.esi + x86::reg32(2) /* 0x2 */);
    // 0049413c  6685d2                 +test dx, dx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg16>(cpu.dx & cpu.dx));
    // 0049413f  74b9                   -je 0x4940fa
    if (cpu.flags.zf)
    {
        goto L_0x004940fa;
    }
    // 00494141  89d3                   -mov ebx, edx
    cpu.ebx = cpu.edx;
    // 00494143  664b                   +dec bx
    {
        x86::reg16& tmp = cpu.bx;
        cpu.flags.of = 1 & (tmp >> 15);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 15));
        cpu.set_szp(tmp);
    }
    // 00494145  66895e02               -mov word ptr [esi + 2], bx
    app->getMemory<x86::reg16>(cpu.esi + x86::reg32(2) /* 0x2 */) = cpu.bx;
    // 00494149  0f850dffffff           -jne 0x49405c
    if (!cpu.flags.zf)
    {
        goto L_0x0049405c;
    }
    // 0049414f  66c7060002             -mov word ptr [esi], 0x200
    app->getMemory<x86::reg16>(cpu.esi) = 512 /*0x200*/;
    // 00494154  ebbe                   -jmp 0x494114
    goto L_0x00494114;
L_0x00494156:
    // 00494156  ff45fc                 +inc dword ptr [ebp - 4]
    {
        auto tmp = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00494159  e9e9feffff             -jmp 0x494047
    goto L_0x00494047;
L_0x0049415e:
    // 0049415e  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00494160  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00494161  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00494162  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00494163  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00494164  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00494165  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00494166  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_494170(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0;
  x86::reg16 fsw = 0;
    // 00494170  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00494171  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00494172  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00494173  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00494174  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00494175  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00494177  83ec34                 -sub esp, 0x34
    (cpu.esp) -= x86::reg32(x86::sreg32(52 /*0x34*/));
    // 0049417a  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0049417d  8955ec                 -mov dword ptr [ebp - 0x14], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.edx;
    // 00494180  833d98d46f0000         +cmp dword ptr [0x6fd498], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328920) /* 0x6fd498 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00494187  0f84c8010000           -je 0x494355
    if (cpu.flags.zf)
    {
        goto L_0x00494355;
    }
    // 0049418d  e87e820000             -call 0x49c410
    cpu.esp -= 4;
    sub_49c410(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00494192  b9ecffffff             -mov ecx, 0xffffffec
    cpu.ecx = 4294967276 /*0xffffffec*/;
    // 00494197  e874feffff             -call 0x494010
    cpu.esp -= 4;
    sub_494010(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049419c  894df8                 -mov dword ptr [ebp - 8], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.ecx;
    // 0049419f  31f6                   +xor esi, esi
    cpu.clear_co();
    cpu.set_szp((cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi))));
    // 004941a1  8975f0                 -mov dword ptr [ebp - 0x10], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.esi;
    // 004941a4  e96d010000             -jmp 0x494316
    goto L_0x00494316;
L_0x004941a9:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004941a9  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004941ab  7d06                   -jge 0x4941b3
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004941b3;
    }
    // 004941ad  030540d95d00           -add eax, dword ptr [0x5dd940]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(6150464) /* 0x5dd940 */)));
L_0x004941b3:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004941b3  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004941b5  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004941b8  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004941ba  8b4df0                 -mov ecx, dword ptr [ebp - 0x10]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004941bd  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 004941c0  8b148da0e45500         -mov edx, dword ptr [ecx*4 + 0x55e4a0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5629088) /* 0x55e4a0 */ + cpu.ecx * 4);
    // 004941c7  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004941c9  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004941cb  8d7dcc                 -lea edi, [ebp - 0x34]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-52) /* -0x34 */);
    // 004941ce  8d7038                 -lea esi, [eax + 0x38]
    cpu.esi = x86::reg32(cpu.eax + x86::reg32(56) /* 0x38 */);
    // 004941d1  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004941d2  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004941d3  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004941d4  8955f4                 -mov dword ptr [ebp - 0xc], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.edx;
    // 004941d7  66833a00               +cmp word ptr [edx], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.edx);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004941db  0f852c010000           -jne 0x49430d
    if (!cpu.flags.zf)
    {
        goto L_0x0049430d;
    }
    // 004941e1  8d45d8                 -lea eax, [ebp - 0x28]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-40) /* -0x28 */);
    // 004941e4  8b4dfc                 -mov ecx, dword ptr [ebp - 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004941e7  8b5dfc                 -mov ebx, dword ptr [ebp - 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004941ea  8d55cc                 -lea edx, [ebp - 0x34]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-52) /* -0x34 */);
    // 004941ed  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004941ee  83c138                 -add ecx, 0x38
    (cpu.ecx) += x86::reg32(x86::sreg32(56 /*0x38*/));
    // 004941f1  83c344                 +add ebx, 0x44
    {
        x86::reg32& tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(68 /*0x44*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004941f4  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004941f9  e832c20400             -call 0x4e0430
    cpu.esp -= 4;
    nfs3hp::vertices::transformPoints(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004941fe  d945e0                 +fld dword ptr [ebp - 0x20]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-32) /* -0x20 */));
    // 00494201  dc1d90bd5300           +fcomp qword ptr [0x53bd90]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<double>(x86::reg32(5488016) /* 0x53bd90 */)));
    // 00494207  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00494209  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 0049420a  0f82fd000000           -jb 0x49430d
    if (cpu.flags.cf)
    {
        goto L_0x0049430d;
    }
    // 00494210  817de00000c842         +cmp dword ptr [ebp - 0x20], 0x42c80000
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1120403456 /*0x42c80000*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00494217  0f8ff0000000           -jg 0x49430d
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x0049430d;
    }
    // 0049421d  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00494222  e829950000             -call 0x49d750
    cpu.esp -= 4;
    sub_49d750(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00494227  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0049422a  8b4004                 -mov eax, dword ptr [eax + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 0049422d  e8be960000             -call 0x49d8f0
    cpu.esp -= 4;
    sub_49d8f0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00494232  f605583a7a0002         +test byte ptr [0x7a3a58], 2
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(x86::reg32(8010328) /* 0x7a3a58 */) & 2 /*0x2*/));
    // 00494239  7411                   -je 0x49424c
    if (cpu.flags.zf)
    {
        goto L_0x0049424c;
    }
    // 0049423b  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0049423d  e80e950000             -call 0x49d750
    cpu.esp -= 4;
    sub_49d750(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00494242  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
    // 00494247  e8a4960000             -call 0x49d8f0
    cpu.esp -= 4;
    sub_49d8f0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0049424c:
    // 0049424c  d945e0                 +fld dword ptr [ebp - 0x20]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-32) /* -0x20 */));
    // 0049424f  dc1d98bd5300           +fcomp qword ptr [0x53bd98]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<double>(x86::reg32(5488024) /* 0x53bd98 */)));
    // 00494255  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00494257  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00494258  7304                   -jae 0x49425e
    if (!cpu.flags.cf)
    {
        goto L_0x0049425e;
    }
    // 0049425a  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 0049425c  eb15                   -jmp 0x494273
    goto L_0x00494273;
L_0x0049425e:
    // 0049425e  817de000004842         +cmp dword ptr [ebp - 0x20], 0x42480000
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1112014848 /*0x42480000*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00494265  7d07                   -jge 0x49426e
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0049426e;
    }
    // 00494267  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0049426c  eb05                   -jmp 0x494273
    goto L_0x00494273;
L_0x0049426e:
    // 0049426e  ba02000000             -mov edx, 2
    cpu.edx = 2 /*0x2*/;
L_0x00494273:
    // 00494273  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 0049427a  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0049427c  8d0c8500000000         -lea ecx, [eax*4]
    cpu.ecx = x86::reg32(cpu.eax * 4);
    // 00494283  8b5dfc                 -mov ebx, dword ptr [ebp - 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00494286  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00494289  8b75fc                 -mov esi, dword ptr [ebp - 4]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0049428c  83c338                 -add ebx, 0x38
    (cpu.ebx) += x86::reg32(x86::sreg32(56 /*0x38*/));
    // 0049428f  668b7802               -mov di, word ptr [eax + 2]
    cpu.di = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(2) /* 0x2 */);
    // 00494293  83c644                 -add esi, 0x44
    (cpu.esi) += x86::reg32(x86::sreg32(68 /*0x44*/));
    // 00494296  6685ff                 +test di, di
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg16>(cpu.di & cpu.di));
    // 00494299  7422                   -je 0x4942bd
    if (cpu.flags.zf)
    {
        goto L_0x004942bd;
    }
    // 0049429b  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0049429d  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0049429e  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0049429f  8d5dcc                 -lea ebx, [ebp - 0x34]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-52) /* -0x34 */);
    // 004942a2  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004942a3  8b5df4                 -mov ebx, dword ptr [ebp - 0xc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004942a6  d945d0                 -fld dword ptr [ebp - 0x30]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-48) /* -0x30 */));
    // 004942a9  83c308                 +add ebx, 8
    {
        x86::reg32& tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(8 /*0x8*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004942ac  a1a8e45500             -mov eax, dword ptr [0x55e4a8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629096) /* 0x55e4a8 */);
    // 004942b1  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004942b2  8d98fc000000           -lea ebx, [eax + 0xfc]
    cpu.ebx = x86::reg32(cpu.eax + x86::reg32(252) /* 0xfc */);
    // 004942b8  d8402c                 +fadd dword ptr [eax + 0x2c]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(44) /* 0x2c */)));
    // 004942bb  eb1b                   -jmp 0x4942d8
    goto L_0x004942d8;
L_0x004942bd:
    // 004942bd  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 004942bf  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004942c0  a1a8e45500             -mov eax, dword ptr [0x55e4a8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629096) /* 0x55e4a8 */);
    // 004942c5  8d5dcc                 -lea ebx, [ebp - 0x34]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-52) /* -0x34 */);
    // 004942c8  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004942c9  d945d0                 -fld dword ptr [ebp - 0x30]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-48) /* -0x30 */));
    // 004942cc  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004942cd  d8402c                 -fadd dword ptr [eax + 0x2c]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(44) /* 0x2c */)));
    // 004942d0  8d98fc000000           -lea ebx, [eax + 0xfc]
    cpu.ebx = x86::reg32(cpu.eax + x86::reg32(252) /* 0xfc */);
    // 004942d6  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
L_0x004942d8:
    // 004942d8  01d9                   -add ecx, ebx
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.ebx));
    // 004942da  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004942dc  d95dd0                 -fstp dword ptr [ebp - 0x30]
    app->getMemory<float>(cpu.ebp + x86::reg32(-48) /* -0x30 */) = float(fp0);
    // 004942df  e83c9b0000             -call 0x49de20
    cpu.esp -= 4;
    sub_49de20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004942e4  ddd8                   -fstp st(0)
    fp0 = x86::Float(fp0);
    // 004942e6  8d75e4                 -lea esi, [ebp - 0x1c]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 004942e9  8d55e4                 -lea edx, [ebp - 0x1c]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 004942ec  e8af810000             -call 0x49c4a0
    cpu.esp -= 4;
    sub_49c4a0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004942f1  d945e0                 -fld dword ptr [ebp - 0x20]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-32) /* -0x20 */));
    // 004942f4  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004942f7  d82500d07d00           -fsub dword ptr [0x7dd000]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(x86::reg32(8245248) /* 0x7dd000 */)));
    // 004942fd  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00494300  d91c24                 -fstp dword ptr [esp]
    app->getMemory<float>(cpu.esp) = float(fp0);
    // 00494303  e818810000             -call 0x49c420
    cpu.esp -= 4;
    sub_49c420(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00494308  e803810000             -call 0x49c410
    cpu.esp -= 4;
    sub_49c410(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0049430d:
    // 0049430d  ff45f0                 -inc dword ptr [ebp - 0x10]
    (app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */))++;
    // 00494310  837df002               +cmp dword ptr [ebp - 0x10], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00494314  7d25                   -jge 0x49433b
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0049433b;
    }
L_0x00494316:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00494316  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00494319  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0049431c  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 0049431e  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 00494320  0f8c83feffff           -jl 0x4941a9
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004941a9;
    }
    // 00494326  8b1d40d95d00           -mov ebx, dword ptr [0x5dd940]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(6150464) /* 0x5dd940 */);
    // 0049432c  39d8                   +cmp eax, ebx
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
    // 0049432e  0f8c7ffeffff           -jl 0x4941b3
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004941b3;
    }
    // 00494334  29d8                   +sub eax, ebx
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00494336  e978feffff             -jmp 0x4941b3
    goto L_0x004941b3;
L_0x0049433b:
    // 0049433b  ff45f8                 -inc dword ptr [ebp - 8]
    (app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */))++;
    // 0049433e  837df814               +cmp dword ptr [ebp - 8], 0x14
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(20 /*0x14*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00494342  7d07                   -jge 0x49434b
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0049434b;
    }
    // 00494344  31f6                   +xor esi, esi
    cpu.clear_co();
    cpu.set_szp((cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi))));
    // 00494346  8975f0                 -mov dword ptr [ebp - 0x10], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.esi;
    // 00494349  ebcb                   -jmp 0x494316
    goto L_0x00494316;
L_0x0049434b:
    // 0049434b  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00494350  e8fb930000             -call 0x49d750
    cpu.esp -= 4;
    sub_49d750(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00494355:
    // 00494355  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00494357  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00494358  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00494359  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049435a  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049435b  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049435c  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 0x00 */
void Application::sub_494360(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00494360  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00494361  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00494362  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00494364  ba002d0000             -mov edx, 0x2d00
    cpu.edx = 11520 /*0x2d00*/;
    // 00494369  b870c57900             -mov eax, 0x79c570
    cpu.eax = 7980400 /*0x79c570*/;
    // 0049436e  e899c30400             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00494373  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00494375  8915c0e45500           -mov dword ptr [0x55e4c0], edx
    app->getMemory<x86::reg32>(x86::reg32(5629120) /* 0x55e4c0 */) = cpu.edx;
    // 0049437b  891570f27900           -mov dword ptr [0x79f270], edx
    app->getMemory<x86::reg32>(x86::reg32(7991920) /* 0x79f270 */) = cpu.edx;
    // 00494381  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00494382  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00494383  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_494390(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00494390  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00494391  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00494392  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00494393  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00494394  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00494395  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00494397  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 00494399  8b1570f27900           -mov edx, dword ptr [0x79f270]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7991920) /* 0x79f270 */);
    // 0049439f  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004943a1  7435                   -je 0x4943d8
    if (cpu.flags.zf)
    {
        goto L_0x004943d8;
    }
    // 004943a3  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 004943a5  8d7210                 -lea esi, [edx + 0x10]
    cpu.esi = x86::reg32(cpu.edx + x86::reg32(16) /* 0x10 */);
    // 004943a8  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004943a9  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004943aa  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004943ab  8b02                   -mov eax, dword ptr [edx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx);
    // 004943ad  01c0                   -add eax, eax
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.eax));
    // 004943af  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004943b1  c1e008                 -shl eax, 8
    cpu.eax <<= 8 /*0x8*/ % 32;
    // 004943b4  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004943b6  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004943b8  8b1dace45500           -mov ebx, dword ptr [0x55e4ac]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5629100) /* 0x55e4ac */);
    // 004943be  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004943c1  f7fb                   -idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004943c3  3dff000000             +cmp eax, 0xff
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(255 /*0xff*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004943c8  7c05                   -jl 0x4943cf
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004943cf;
    }
    // 004943ca  b8ff000000             -mov eax, 0xff
    cpu.eax = 255 /*0xff*/;
L_0x004943cf:
    // 004943cf  8901                   -mov dword ptr [ecx], eax
    app->getMemory<x86::reg32>(cpu.ecx) = cpu.eax;
    // 004943d1  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004943d6  eb02                   -jmp 0x4943da
    goto L_0x004943da;
L_0x004943d8:
    // 004943d8  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x004943da:
    // 004943da  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004943db  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004943dc  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004943dd  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004943de  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004943df  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_4943e0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004943e0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004943e1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004943e2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004943e3  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004943e4  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004943e6  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004943e8  31c9                   +xor ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx))));
    // 004943ea  eb08                   -jmp 0x4943f4
    goto L_0x004943f4;
L_0x004943ec:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004943ec  81faf0000000           +cmp edx, 0xf0
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(240 /*0xf0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004943f2  7d1c                   -jge 0x494410
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00494410;
    }
L_0x004943f4:
    // 004943f4  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004943fb  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004943fd  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 00494400  0570c57900             -add eax, 0x79c570
    (cpu.eax) += x86::reg32(x86::sreg32(7980400 /*0x79c570*/));
    // 00494405  8b18                   -mov ebx, dword ptr [eax]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax);
    // 00494407  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 00494409  7502                   -jne 0x49440d
    if (!cpu.flags.zf)
    {
        goto L_0x0049440d;
    }
    // 0049440b  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
L_0x0049440d:
    // 0049440d  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0049440e  ebdc                   -jmp 0x4943ec
    goto L_0x004943ec;
L_0x00494410:
    // 00494410  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00494412  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00494413  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00494414  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00494415  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00494416  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_494420(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1;
    // 00494420  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00494421  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00494422  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00494423  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00494425  83ec14                 -sub esp, 0x14
    (cpu.esp) -= x86::reg32(x86::sreg32(20 /*0x14*/));
    // 00494428  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 0049442b  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 0049442d  89df                   -mov edi, ebx
    cpu.edi = cpu.ebx;
    // 0049442f  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 00494431  0f845a010000           -je 0x494591
    if (cpu.flags.zf)
    {
        goto L_0x00494591;
    }
    // 00494437  e8a4ffffff             -call 0x4943e0
    cpu.esp -= 4;
    sub_4943e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049443c  8902                   -mov dword ptr [edx], eax
    app->getMemory<x86::reg32>(cpu.edx) = cpu.eax;
    // 0049443e  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00494440  0f844d010000           -je 0x494593
    if (cpu.flags.zf)
    {
        goto L_0x00494593;
    }
    // 00494446  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00494449  8b12                   -mov edx, dword ptr [edx]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx);
    // 0049444b  8910                   -mov dword ptr [eax], edx
    app->getMemory<x86::reg32>(cpu.eax) = cpu.edx;
    // 0049444d  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 0049444f  bb05000000             -mov ebx, 5
    cpu.ebx = 5 /*0x5*/;
    // 00494454  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00494456  c7400400000000         -mov dword ptr [eax + 4], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */) = 0 /*0x0*/;
    // 0049445d  e826c90400             -call 0x4e0d88
    cpu.esp -= 4;
    nfs3hp::port::eaRandom(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00494462  f7f3                   -div ebx
    {
        x86::reg64 tmp = cpu.edx_eax;
        x86::reg32 d = cpu.ebx;
        cpu.edx_eax /= d;
        cpu.edx = tmp % d;
    }
    // 00494464  d94510                 -fld dword ptr [ebp + 0x10]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(16) /* 0x10 */));
    // 00494467  dc0da0bd5300           -fmul qword ptr [0x53bda0]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5488032) /* 0x53bda0 */)));
    // 0049446d  83ea02                 -sub edx, 2
    (cpu.edx) -= x86::reg32(x86::sreg32(2 /*0x2*/));
    // 00494470  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 00494473  db45fc                 -fild dword ptr [ebp - 4]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 00494476  dec9                   -fmulp st(1)
    fp0 = fp::mul(fp0, fp1);
    // 00494478  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0049447b  d8400c                 -fadd dword ptr [eax + 0xc]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(12) /* 0xc */)));
    // 0049447e  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 00494480  d9580c                 -fstp dword ptr [eax + 0xc]
    app->getMemory<float>(cpu.eax + x86::reg32(12) /* 0xc */) = float(fp0);
    // 00494483  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 00494485  ff700c                 -push dword ptr [eax + 0xc]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    cpu.esp -= 4;
    // 00494488  e8c3640500             -call 0x4ea950
    cpu.esp -= 4;
    sub_4ea950(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0049448d  d80db0e45500           -fmul dword ptr [0x55e4b0]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(5629104) /* 0x55e4b0 */)));
    // 00494493  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00494496  d84010                 -fadd dword ptr [eax + 0x10]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(16) /* 0x10 */)));
    // 00494499  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 0049449b  d95810                 -fstp dword ptr [eax + 0x10]
    app->getMemory<float>(cpu.eax + x86::reg32(16) /* 0x10 */) = float(fp0);
    // 0049449e  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 004944a0  ff700c                 -push dword ptr [eax + 0xc]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    cpu.esp -= 4;
    // 004944a3  e888640500             -call 0x4ea930
    cpu.esp -= 4;
    sub_4ea930(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004944a8  d80db0e45500           -fmul dword ptr [0x55e4b0]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(5629104) /* 0x55e4b0 */)));
    // 004944ae  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004944b1  d84018                 -fadd dword ptr [eax + 0x18]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(24) /* 0x18 */)));
    // 004944b4  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 004944b6  d95818                 -fstp dword ptr [eax + 0x18]
    app->getMemory<float>(cpu.eax + x86::reg32(24) /* 0x18 */) = float(fp0);
    // 004944b9  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 004944bb  83c010                 -add eax, 0x10
    (cpu.eax) += x86::reg32(x86::sreg32(16 /*0x10*/));
    // 004944be  897dfc                 -mov dword ptr [ebp - 4], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edi;
    // 004944c1  e8fa68feff             -call 0x47adc0
    cpu.esp -= 4;
    sub_47adc0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004944c6  db05b4e45500           -fild dword ptr [0x55e4b4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5629108) /* 0x55e4b4 */)));
    // 004944cc  db45fc                 -fild dword ptr [ebp - 4]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 004944cf  def1                   -fdivrp st(1)
    fp0 = fp::div(fp1, x86::Float(fp0));
    // 004944d1  d9e8                   -fld1 
    fp1 = 1.0;
    // 004944d3  dee1                   -fsubrp st(1)
    fp0 = fp::sub(fp1, x86::Float(fp0));
    // 004944d5  d80db8e45500           -fmul dword ptr [0x55e4b8]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(5629112) /* 0x55e4b8 */)));
    // 004944db  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 004944dd  d95808                 -fstp dword ptr [eax + 8]
    app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */) = float(fp0);
    // 004944e0  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 004944e2  c7401c00000000         -mov dword ptr [eax + 0x1c], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(28) /* 0x1c */) = 0 /*0x0*/;
    // 004944e9  39cf                   +cmp edi, ecx
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ecx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004944eb  0f857b000000           -jne 0x49456c
    if (!cpu.flags.zf)
    {
        goto L_0x0049456c;
    }
    // 004944f1  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004944f3  e890c80400             -call 0x4e0d88
    cpu.esp -= 4;
    nfs3hp::port::eaRandom(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004944f8  f7f1                   -div ecx
    {
        x86::reg64 tmp = cpu.edx_eax;
        x86::reg32 d = cpu.ecx;
        cpu.edx_eax /= d;
        cpu.edx = tmp % d;
    }
    // 004944fa  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004944fc  746e                   -je 0x49456c
    if (cpu.flags.zf)
    {
        goto L_0x0049456c;
    }
    // 004944fe  894dfc                 -mov dword ptr [ebp - 4], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
    // 00494501  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 00494503  db45fc                 -fild dword ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 00494506  dd55ec                 -fst qword ptr [ebp - 0x14]
    app->getMemory<double>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = double(fp0);
    // 00494509  dc0da8bd5300           -fmul qword ptr [0x53bda8]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5488040) /* 0x53bda8 */)));
    // 0049450f  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
    // 00494512  8d59ff                 -lea ebx, [ecx - 1]
    cpu.ebx = x86::reg32(cpu.ecx + x86::reg32(-1) /* -0x1 */);
    // 00494515  8d5020                 -lea edx, [eax + 0x20]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(32) /* 0x20 */);
    // 00494518  e839b80400             -call 0x4dfd56
    cpu.fpu.push(fp0);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0049451d  db5dfc                 -fistp dword ptr [ebp - 4]
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 00494520  8b4dfc                 -mov ecx, dword ptr [ebp - 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00494523  ff35bce45500           -push dword ptr [0x55e4bc]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(x86::reg32(5629116) /* 0x55e4bc */);
    cpu.esp -= 4;
    // 00494529  e8f2feffff             -call 0x494420
    cpu.esp -= 4;
    sub_494420(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049452e  d905bce45500           -fld dword ptr [0x55e4bc]
    fp0 = x86::Float(app->getMemory<float>(x86::reg32(5629116) /* 0x55e4bc */));
    // 00494534  d80db0bd5300           -fmul dword ptr [0x53bdb0]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(5488048) /* 0x53bdb0 */)));
    // 0049453a  dd45ec                 -fld qword ptr [ebp - 0x14]
    fp1 = x86::Float(app->getMemory<double>(cpu.ebp + x86::reg32(-20) /* -0x14 */));
    // 0049453d  dc0db8bd5300           -fmul qword ptr [0x53bdb8]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<double>(x86::reg32(5488056) /* 0x53bdb8 */)));
    // 00494543  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00494546  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00494549  8b4a1c                 -mov ecx, dword ptr [edx + 0x1c]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(28) /* 0x1c */);
    // 0049454c  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 0049454e  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00494550  d1fb                   +sar ebx, 1
    {
        x86::reg32 tmp = 1 /*0x1*/ % 32;
        x86::reg32& op = cpu.ebx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (x86::sreg32(op) >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = 0;
            cpu.set_szp((op = x86::reg32(x86::sreg32(op) >> tmp)));
        }
    }
    // 00494552  894a1c                 -mov dword ptr [edx + 0x1c], ecx
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(28) /* 0x1c */) = cpu.ecx;
    // 00494555  e8fcb70400             -call 0x4dfd56
    cpu.fpu.push(fp1);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp1 = cpu.fpu.take();
    // 0049455a  8b36                   -mov esi, dword ptr [esi]
    cpu.esi = app->getMemory<x86::reg32>(cpu.esi);
    // 0049455c  db5dfc                 +fistp dword ptr [ebp - 4]
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp1));
    // 0049455f  8d5624                 -lea edx, [esi + 0x24]
    cpu.edx = x86::reg32(cpu.esi + x86::reg32(36) /* 0x24 */);
    // 00494562  8b4dfc                 -mov ecx, dword ptr [ebp - 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00494565  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00494567  d91c24                 +fstp dword ptr [esp]
    app->getMemory<float>(cpu.esp) = float(fp0);
    // 0049456a  eb10                   -jmp 0x49457c
    goto L_0x0049457c;
L_0x0049456c:
    // 0049456c  8b36                   -mov esi, dword ptr [esi]
    cpu.esi = app->getMemory<x86::reg32>(cpu.esi);
    // 0049456e  8d5620                 -lea edx, [esi + 0x20]
    cpu.edx = x86::reg32(cpu.esi + x86::reg32(32) /* 0x20 */);
    // 00494571  8d5fff                 -lea ebx, [edi - 1]
    cpu.ebx = x86::reg32(cpu.edi + x86::reg32(-1) /* -0x1 */);
    // 00494574  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00494576  ff35bce45500           -push dword ptr [0x55e4bc]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(x86::reg32(5629116) /* 0x55e4bc */);
    cpu.esp -= 4;
L_0x0049457c:
    // 0049457c  e89ffeffff             -call 0x494420
    cpu.esp -= 4;
    sub_494420(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00494581  01461c                 -add dword ptr [esi + 0x1c], eax
    (app->getMemory<x86::reg32>(cpu.esi + x86::reg32(28) /* 0x1c */)) += x86::reg32(x86::sreg32(cpu.eax));
    // 00494584  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00494589  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0049458b  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049458c  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049458d  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049458e  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
L_0x00494591:
    // 00494591  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x00494593:
    // 00494593  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00494595  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00494596  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00494597  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00494598  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
}

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_4945a0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0;
    // 004945a0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004945a1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004945a2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004945a3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004945a4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004945a5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004945a6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004945a8  83ec08                 -sub esp, 8
    (cpu.esp) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 004945ab  e830feffff             -call 0x4943e0
    cpu.esp -= 4;
    sub_4943e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004945b0  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004945b3  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004945b5  746e                   -je 0x494625
    if (cpu.flags.zf)
    {
        goto L_0x00494625;
    }
    // 004945b7  db05b4e45500           -fild dword ptr [0x55e4b4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5629108) /* 0x55e4b4 */)));
    // 004945bd  dc0dc0bd5300           -fmul qword ptr [0x53bdc0]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5488064) /* 0x53bdc0 */)));
    // 004945c3  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004945c6  a1ace45500             -mov eax, dword ptr [0x55e4ac]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629100) /* 0x55e4ac */);
    // 004945cb  ff35bce45500           -push dword ptr [0x55e4bc]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(x86::reg32(5629116) /* 0x55e4bc */);
    cpu.esp -= 4;
    // 004945d1  8902                   -mov dword ptr [edx], eax
    app->getMemory<x86::reg32>(cpu.edx) = cpu.eax;
    // 004945d3  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004945d5  c7420401000000         -mov dword ptr [edx + 4], 1
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(4) /* 0x4 */) = 1 /*0x1*/;
    // 004945dc  8d751c                 -lea esi, [ebp + 0x1c]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(28) /* 0x1c */);
    // 004945df  8d7810                 -lea edi, [eax + 0x10]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 004945e2  8b1db4e45500           -mov ebx, dword ptr [0x55e4b4]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5629108) /* 0x55e4b4 */);
    // 004945e8  8b4528                 -mov eax, dword ptr [ebp + 0x28]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(40) /* 0x28 */);
    // 004945eb  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004945ec  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004945ed  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004945ee  e863b70400             -call 0x4dfd56
    cpu.fpu.push(fp0);
    cpu.esp -= 4;
    nfs3hp::watcom::CHP(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004945f3  89420c                 -mov dword ptr [edx + 0xc], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(12) /* 0xc */) = cpu.eax;
    // 004945f6  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004945f8  83c220                 -add edx, 0x20
    (cpu.edx) += x86::reg32(x86::sreg32(32 /*0x20*/));
    // 004945fb  db5df8                 -fistp dword ptr [ebp - 8]
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 004945fe  c742e800000000         -mov dword ptr [edx - 0x18], 0
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(-24) /* -0x18 */) = 0 /*0x0*/;
    // 00494605  8b4df8                 -mov ecx, dword ptr [ebp - 8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00494608  c742fc00000000         -mov dword ptr [edx - 4], 0
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(-4) /* -0x4 */) = 0 /*0x0*/;
    // 0049460f  e80cfeffff             -call 0x494420
    cpu.esp -= 4;
    sub_494420(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00494614  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00494617  8b4a1c                 -mov ecx, dword ptr [edx + 0x1c]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(28) /* 0x1c */);
    // 0049461a  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0049461c  891570f27900           -mov dword ptr [0x79f270], edx
    app->getMemory<x86::reg32>(x86::reg32(7991920) /* 0x79f270 */) = cpu.edx;
    // 00494622  894a1c                 -mov dword ptr [edx + 0x1c], ecx
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(28) /* 0x1c */) = cpu.ecx;
L_0x00494625:
    // 00494625  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00494627  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00494628  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00494629  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049462a  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049462b  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049462c  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049462d  c21000                 -ret 0x10
    cpu.esp += 4+16 /*0x10*/;
    return;
}

/* align: skip  */
void Application::sub_494630(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1;
    // 00494630  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00494631  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00494632  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00494633  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00494634  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00494635  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00494636  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00494638  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
L_0x0049463b:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049463b  8b1584367d00           -mov edx, dword ptr [0x7d3684]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205956) /* 0x7d3684 */);
    // 00494641  a1c0e45500             -mov eax, dword ptr [0x55e4c0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5629120) /* 0x55e4c0 */);
    // 00494646  39d0                   +cmp eax, edx
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
    // 00494648  0f8d88000000           -jge 0x4946d6
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004946d6;
    }
    // 0049464e  8d4801                 -lea ecx, [eax + 1]
    cpu.ecx = x86::reg32(cpu.eax + x86::reg32(1) /* 0x1 */);
    // 00494651  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 00494653  890dc0e45500           -mov dword ptr [0x55e4c0], ecx
    app->getMemory<x86::reg32>(x86::reg32(5629120) /* 0x55e4c0 */) = cpu.ecx;
    // 00494659  eb11                   -jmp 0x49466c
    goto L_0x0049466c;
L_0x0049465b:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049465b  31c1                   -xor ecx, eax
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049465d  890d70f27900           -mov dword ptr [0x79f270], ecx
    app->getMemory<x86::reg32>(x86::reg32(7991920) /* 0x79f270 */) = cpu.ecx;
L_0x00494663:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00494663  43                     -inc ebx
    (cpu.ebx)++;
    // 00494664  81fbf0000000           +cmp ebx, 0xf0
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(240 /*0xf0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049466a  7dcf                   -jge 0x49463b
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0049463b;
    }
L_0x0049466c:
    // 0049466c  8d049d00000000         -lea eax, [ebx*4]
    cpu.eax = x86::reg32(cpu.ebx * 4);
    // 00494673  29d8                   -sub eax, ebx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 00494675  b970c57900             -mov ecx, 0x79c570
    cpu.ecx = 7980400 /*0x79c570*/;
    // 0049467a  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 0049467d  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0049467f  8b31                   -mov esi, dword ptr [ecx]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ecx);
    // 00494681  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 00494683  74de                   -je 0x494663
    if (cpu.flags.zf)
    {
        goto L_0x00494663;
    }
    // 00494685  be03000000             -mov esi, 3
    cpu.esi = 3 /*0x3*/;
    // 0049468a  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0049468c  e8f7c60400             -call 0x4e0d88
    cpu.esp -= 4;
    nfs3hp::port::eaRandom(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00494691  f7f6                   -div esi
    {
        x86::reg64 tmp = cpu.edx_eax;
        x86::reg32 d = cpu.esi;
        cpu.edx_eax /= d;
        cpu.edx = tmp % d;
    }
    // 00494693  8d7aff                 -lea edi, [edx - 1]
    cpu.edi = x86::reg32(cpu.edx + x86::reg32(-1) /* -0x1 */);
    // 00494696  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00494698  e8ebc60400             -call 0x4e0d88
    cpu.esp -= 4;
    nfs3hp::port::eaRandom(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049469d  f7f6                   -div esi
    {
        x86::reg64 tmp = cpu.edx_eax;
        x86::reg32 d = cpu.esi;
        cpu.edx_eax /= d;
        cpu.edx = tmp % d;
    }
    // 0049469f  897dfc                 -mov dword ptr [ebp - 4], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edi;
    // 004946a2  db45fc                 -fild dword ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 004946a5  8b39                   -mov edi, dword ptr [ecx]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ecx);
    // 004946a7  d84908                 -fmul dword ptr [ecx + 8]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(8) /* 0x8 */)));
    // 004946aa  4f                     -dec edi
    (cpu.edi)--;
    // 004946ab  8d42ff                 -lea eax, [edx - 1]
    cpu.eax = x86::reg32(cpu.edx + x86::reg32(-1) /* -0x1 */);
    // 004946ae  8939                   -mov dword ptr [ecx], edi
    app->getMemory<x86::reg32>(cpu.ecx) = cpu.edi;
    // 004946b0  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004946b3  d84110                 -fadd dword ptr [ecx + 0x10]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(16) /* 0x10 */)));
    // 004946b6  db45fc                 -fild dword ptr [ebp - 4]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 004946b9  d84908                 -fmul dword ptr [ecx + 8]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(8) /* 0x8 */)));
    // 004946bc  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 004946be  d95910                 -fstp dword ptr [ecx + 0x10]
    app->getMemory<float>(cpu.ecx + x86::reg32(16) /* 0x10 */) = float(fp1);
    // 004946c1  d84118                 -fadd dword ptr [ecx + 0x18]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(24) /* 0x18 */)));
    // 004946c4  a170f27900             -mov eax, dword ptr [0x79f270]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7991920) /* 0x79f270 */);
    // 004946c9  d95918                 -fstp dword ptr [ecx + 0x18]
    app->getMemory<float>(cpu.ecx + x86::reg32(24) /* 0x18 */) = float(fp0);
    // 004946cc  39c1                   +cmp ecx, eax
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004946ce  7593                   -jne 0x494663
    if (!cpu.flags.zf)
    {
        goto L_0x00494663;
    }
    // 004946d0  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 004946d2  7487                   -je 0x49465b
    if (cpu.flags.zf)
    {
        goto L_0x0049465b;
    }
    // 004946d4  eb8d                   -jmp 0x494663
    goto L_0x00494663;
L_0x004946d6:
    // 004946d6  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004946d8  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004946d9  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004946da  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004946db  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004946dc  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004946dd  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004946de  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4946e0: native, nfs3hp::render::lightning */

/* sub_494bd0: native, nfs3hp::graphics::drawLoadingAborted */

/* sub_494cb0: native, nfs3hp::render::loadingScreen */

/* sub_494fa0: native, nfs3hp::graphics::showLoadingScreen */

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_495190(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1, fp2, fp3;
    // 00495190  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00495191  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00495192  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00495193  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00495195  83ec24                 -sub esp, 0x24
    (cpu.esp) -= x86::reg32(x86::sreg32(36 /*0x24*/));
    // 00495198  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0049519a  8955f4                 -mov dword ptr [ebp - 0xc], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.edx;
    // 0049519d  895df8                 -mov dword ptr [ebp - 8], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.ebx;
    // 004951a0  89ce                   -mov esi, ecx
    cpu.esi = cpu.ecx;
    // 004951a2  8d45f0                 -lea eax, [ebp - 0x10]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004951a5  8d4dec                 -lea ecx, [ebp - 0x14]
    cpu.ecx = x86::reg32(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004951a8  8d5de8                 -lea ebx, [ebp - 0x18]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004951ab  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004951ac  8d55e4                 -lea edx, [ebp - 0x1c]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 004951af  b8c0da7c00             -mov eax, 0x7cdac0
    cpu.eax = 8182464 /*0x7cdac0*/;
    // 004951b4  e867990200             -call 0x4beb20
    cpu.esp -= 4;
    nfs3hp::frontend::pauseReadRect(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004951b9  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004951bc  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004951bf  db45fc                 -fild dword ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 004951c2  d80d28be5300           -fmul dword ptr [0x53be28]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(5488168) /* 0x53be28 */)));
    // 004951c8  8b45f0                 -mov eax, dword ptr [ebp - 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004951cb  d907                   -fld dword ptr [edi]
    fp1 = x86::Float(app->getMemory<float>(cpu.edi));
    // 004951cd  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004951d0  d8c9                   -fmul st(1)
    fp1 = fp::mul(fp1, x86::Float(fp0));
    // 004951d2  db45fc                 -fild dword ptr [ebp - 4]
    fp2 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 004951d5  d80d2cbe5300           -fmul dword ptr [0x53be2c]
    fp2 = fp::mul(fp2, x86::Float(app->getMemory<float>(x86::reg32(5488172) /* 0x53be2c */)));
    // 004951db  d94704                 -fld dword ptr [edi + 4]
    fp3 = x86::Float(app->getMemory<float>(cpu.edi + x86::reg32(4) /* 0x4 */));
    // 004951de  d8c9                   -fmul st(1)
    fp3 = fp::mul(fp3, x86::Float(fp2));
    // 004951e0  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004951e3  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp3;
        fp3 = fp1;
        fp1 = tmp;
    }
    // 004951e5  d91f                   -fstp dword ptr [edi]
    app->getMemory<float>(cpu.edi) = float(fp3);
    // 004951e7  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp2;
        fp2 = fp1;
        fp1 = tmp;
    }
    // 004951e9  d95f04                 -fstp dword ptr [edi + 4]
    app->getMemory<float>(cpu.edi + x86::reg32(4) /* 0x4 */) = float(fp2);
    // 004951ec  d900                   -fld dword ptr [eax]
    fp2 = x86::Float(app->getMemory<float>(cpu.eax));
    // 004951ee  d8ca                   -fmul st(2)
    fp2 = fp::mul(fp2, x86::Float(fp0));
    // 004951f0  d94004                 -fld dword ptr [eax + 4]
    fp3 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 004951f3  d8ca                   -fmul st(2)
    fp3 = fp::mul(fp3, x86::Float(fp1));
    // 004951f5  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp3;
        fp3 = fp2;
        fp2 = tmp;
    }
    // 004951f7  d918                   -fstp dword ptr [eax]
    app->getMemory<float>(cpu.eax) = float(fp3);
    // 004951f9  d95804                 -fstp dword ptr [eax + 4]
    app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */) = float(fp2);
    // 004951fc  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004951ff  d900                   -fld dword ptr [eax]
    fp2 = x86::Float(app->getMemory<float>(cpu.eax));
    // 00495201  d8ca                   -fmul st(2)
    fp2 = fp::mul(fp2, x86::Float(fp0));
    // 00495203  d94004                 -fld dword ptr [eax + 4]
    fp3 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 00495206  d8ca                   -fmul st(2)
    fp3 = fp::mul(fp3, x86::Float(fp1));
    // 00495208  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp3;
        fp3 = fp2;
        fp2 = tmp;
    }
    // 0049520a  d918                   -fstp dword ptr [eax]
    app->getMemory<float>(cpu.eax) = float(fp3);
    // 0049520c  d95804                 -fstp dword ptr [eax + 4]
    app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */) = float(fp2);
    // 0049520f  d906                   -fld dword ptr [esi]
    fp2 = x86::Float(app->getMemory<float>(cpu.esi));
    // 00495211  deca                   -fmulp st(2)
    fp0 = fp::mul(fp0, fp2);
    // 00495213  d84e04                 -fmul dword ptr [esi + 4]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(4) /* 0x4 */)));
    // 00495216  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 00495218  d91e                   -fstp dword ptr [esi]
    app->getMemory<float>(cpu.esi) = float(fp1);
    // 0049521a  d95e04                 -fstp dword ptr [esi + 4]
    app->getMemory<float>(cpu.esi + x86::reg32(4) /* 0x4 */) = float(fp0);
    // 0049521d  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0049521f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00495220  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00495221  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00495222  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_495230: native, nfs3hp::graphics::drawLoadingProgress */

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_495600(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00495600  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00495601  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00495602  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00495603  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00495604  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00495605  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00495606  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00495608  81ec00010000           -sub esp, 0x100
    (cpu.esp) -= x86::reg32(x86::sreg32(256 /*0x100*/));
    // 0049560e  81ed7e010000           -sub ebp, 0x17e
    (cpu.ebp) -= x86::reg32(x86::sreg32(382 /*0x17e*/));
    // 00495614  9b                     -wait 
    /*nothing*/;
    // 00495615  dbe3                   -fninit 
    cpu.fpu.init();
    // 00495617  9b                     -wait 
    /*nothing*/;
    // 00495618  d92df4f37900           -fldcw word ptr [0x79f3f4]
    cpu.fpu.setControl(app->getMemory<x86::reg16>(x86::reg32(7992308) /* 0x79f3f4 */));
    // 0049561e  9b                     -wait 
    /*nothing*/;
    // 0049561f  be34317a00             -mov esi, 0x7a3134
    cpu.esi = 8007988 /*0x7a3134*/;
    // 00495624  8d7d7e                 -lea edi, [ebp + 0x7e]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 00495627  e854590400             -call 0x4daf80
    cpu.esp -= 4;
    nfs3hp::port::draw2d::startScreens(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049562c  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
L_0x0049562d:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049562d  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 0049562f  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 00495631  3c00                   +cmp al, 0
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
    // 00495633  7410                   -je 0x495645
    if (cpu.flags.zf)
    {
        goto L_0x00495645;
    }
    // 00495635  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 00495638  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0049563b  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 0049563e  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 00495641  3c00                   +cmp al, 0
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
    // 00495643  75e8                   -jne 0x49562d
    if (!cpu.flags.zf)
    {
        goto L_0x0049562d;
    }
L_0x00495645:
    // 00495645  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00495646  8d457e                 -lea eax, [ebp + 0x7e]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 00495649  e852d3fbff             -call 0x4529a0
    cpu.esp -= 4;
    nfs3hp::port::menus::appendBackslash(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049564e  e83d2cfeff             -call 0x478290
    cpu.esp -= 4;
    sub_478290(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495653  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00495654  6848be5300             -push 0x53be48
    app->getMemory<x86::reg32>(cpu.esp-4) = 5488200 /*0x53be48*/;
    cpu.esp -= 4;
    // 00495659  8d7d7e                 -lea edi, [ebp + 0x7e]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 0049565c  29c9                   -sub ecx, ecx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 0049565e  49                     -dec ecx
    (cpu.ecx)--;
    // 0049565f  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 00495661  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 00495663  f7d1                   -not ecx
    cpu.ecx = ~cpu.ecx;
    // 00495665  49                     -dec ecx
    (cpu.ecx)--;
    // 00495666  8d457e                 -lea eax, [ebp + 0x7e]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 00495669  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 0049566b  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0049566c  e81fa00400             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495671  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 00495674  ba10000000             -mov edx, 0x10
    cpu.edx = 16 /*0x10*/;
    // 00495679  8d457e                 -lea eax, [ebp + 0x7e]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 0049567c  e86f000000             -call 0x4956f0
    cpu.esp -= 4;
    nfs3hp::frontend::loadQfs(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495681  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00495683  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00495685  744d                   -je 0x4956d4
    if (cpu.flags.zf)
    {
        goto L_0x004956d4;
    }
    // 00495687  ba20be5300             -mov edx, 0x53be20
    cpu.edx = 5488160 /*0x53be20*/;
    // 0049568c  e84f9b0500             -call 0x4ef1e0
    cpu.esp -= 4;
    sub_4ef1e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495691  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00495693  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00495695  e8f60c0400             -call 0x4d6390
    cpu.esp -= 4;
    nfs3hp::port::shapes::artShapeTextures(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049569a  a338768b00             -mov dword ptr [0x8b7638], eax
    app->getMemory<x86::reg32>(x86::reg32(9139768) /* 0x8b7638 */) = cpu.eax;
    // 0049569f  e87c3a0400             -call 0x4d9120
    cpu.esp -= 4;
    nfs3hp::graphics::takeWindowAndClear(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004956a4  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004956a6  a138768b00             -mov eax, dword ptr [0x8b7638]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(9139768) /* 0x8b7638 */);
    // 004956ab  e8e0220400             -call 0x4d7990
    cpu.esp -= 4;
    nfs3hp::port::shapes::drawShape(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004956b0  e88b3a0400             -call 0x4d9140
    cpu.esp -= 4;
    nfs3hp::graphics::flipAndWaitFrames(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004956b5  e8663a0400             -call 0x4d9120
    cpu.esp -= 4;
    nfs3hp::graphics::takeWindowAndClear(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004956ba  a138768b00             -mov eax, dword ptr [0x8b7638]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(9139768) /* 0x8b7638 */);
    // 004956bf  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004956c1  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004956c3  e8c8220400             -call 0x4d7990
    cpu.esp -= 4;
    nfs3hp::port::shapes::drawShape(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004956c8  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004956ca  e8c1c10400             -call 0x4e1890
    cpu.esp -= 4;
    nfs3hp::ea::purgememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004956cf  e8ac580400             -call 0x4daf80
    cpu.esp -= 4;
    nfs3hp::port::draw2d::startScreens(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004956d4:
    // 004956d4  8da57e010000           -lea esp, [ebp + 0x17e]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(382) /* 0x17e */);
    // 004956da  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004956db  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004956dc  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004956dd  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004956de  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004956df  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004956e0  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4956f0: native, nfs3hp::frontend::loadQfs */

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_4957a0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004957a0  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004957a1  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004957a2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004957a3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004957a5  8b7d10                 -mov edi, dword ptr [ebp + 0x10]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(16) /* 0x10 */);
    // 004957a8  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004957aa  83ff10                 +cmp edi, 0x10
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(16 /*0x10*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004957ad  7e0d                   -jle 0x4957bc
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004957bc;
    }
    // 004957af  6858be5300             -push 0x53be58
    app->getMemory<x86::reg32>(cpu.esp-4) = 5488216 /*0x53be58*/;
    cpu.esp -= 4;
    // 004957b4  e857b8f6ff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004957b9  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
L_0x004957bc:
    // 004957bc  891548f37900           -mov dword ptr [0x79f348], edx
    app->getMemory<x86::reg32>(x86::reg32(7992136) /* 0x79f348 */) = cpu.edx;
    // 004957c2  891d50f37900           -mov dword ptr [0x79f350], ebx
    app->getMemory<x86::reg32>(x86::reg32(7992144) /* 0x79f350 */) = cpu.ebx;
    // 004957c8  890d44f37900           -mov dword ptr [0x79f344], ecx
    app->getMemory<x86::reg32>(x86::reg32(7992132) /* 0x79f344 */) = cpu.ecx;
    // 004957ce  893d58f37900           -mov dword ptr [0x79f358], edi
    app->getMemory<x86::reg32>(x86::reg32(7992152) /* 0x79f358 */) = cpu.edi;
    // 004957d4  89354cf37900           -mov dword ptr [0x79f34c], esi
    app->getMemory<x86::reg32>(x86::reg32(7992140) /* 0x79f34c */) = cpu.esi;
    // 004957da  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004957dc  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 004957de  891554f37900           -mov dword ptr [0x79f354], edx
    app->getMemory<x86::reg32>(x86::reg32(7992148) /* 0x79f354 */) = cpu.edx;
    // 004957e4  891540f37900           -mov dword ptr [0x79f340], edx
    app->getMemory<x86::reg32>(x86::reg32(7992128) /* 0x79f340 */) = cpu.edx;
    // 004957ea  891560f37900           -mov dword ptr [0x79f360], edx
    app->getMemory<x86::reg32>(x86::reg32(7992160) /* 0x79f360 */) = cpu.edx;
    // 004957f0  89155cf37900           -mov dword ptr [0x79f35c], edx
    app->getMemory<x86::reg32>(x86::reg32(7992156) /* 0x79f35c */) = cpu.edx;
L_0x004957f6:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004957f6  3b0d58f37900           +cmp ecx, dword ptr [0x79f358]
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7992152) /* 0x79f358 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004957fc  7d2a                   -jge 0x495828
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00495828;
    }
    // 004957fe  8b1550f37900           -mov edx, dword ptr [0x79f350]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992144) /* 0x79f350 */);
    // 00495804  a144f37900             -mov eax, dword ptr [0x79f344]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992132) /* 0x79f344 */);
    // 00495809  01d2                   -add edx, edx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.edx));
    // 0049580b  0fafd0                 +imul edx, eax
    {
        x86::sreg64 tmp = x86::sreg64(x86::sreg32(cpu.edx)) * x86::sreg64(x86::sreg32(cpu.eax));
        cpu.edx = static_cast<x86::reg32>(static_cast<x86::sreg32>(tmp));
        cpu.flags.of = cpu.flags.cf = (tmp != x86::sreg64(x86::sreg32(cpu.edx)));
    }
    // 0049580e  8b1df4435600           -mov ebx, dword ptr [0x5643f4]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5653492) /* 0x5643f4 */);
    // 00495814  b888be5300             -mov eax, 0x53be88
    cpu.eax = 5488264 /*0x53be88*/;
    // 00495819  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0049581a  e801be0400             -call 0x4e1620
    cpu.esp -= 4;
    nfs3hp::ea::reservememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049581f  8904cd8cf27900         -mov dword ptr [ecx*8 + 0x79f28c], eax
    app->getMemory<x86::reg32>(x86::reg32(7991948) /* 0x79f28c */ + cpu.ecx * 8) = cpu.eax;
    // 00495826  ebce                   -jmp 0x4957f6
    goto L_0x004957f6;
L_0x00495828:
    // 00495828  8b551c                 -mov edx, dword ptr [ebp + 0x1c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(28) /* 0x1c */);
    // 0049582b  8b4d18                 -mov ecx, dword ptr [ebp + 0x18]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(24) /* 0x18 */);
    // 0049582e  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0049582f  8b1d50f37900           -mov ebx, dword ptr [0x79f350]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7992144) /* 0x79f350 */);
    // 00495835  a14cf37900             -mov eax, dword ptr [0x79f34c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992140) /* 0x79f34c */);
    // 0049583a  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0049583b  8b1548f37900           -mov edx, dword ptr [0x79f348]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992136) /* 0x79f348 */);
    // 00495841  8b0d44f37900           -mov ecx, dword ptr [0x79f344]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992132) /* 0x79f344 */);
    // 00495847  e8a4990400             -call 0x4df1f0
    cpu.esp -= 4;
    sub_4df1f0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049584c  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049584d  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049584e  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049584f  c21000                 -ret 0x10
    cpu.esp += 4+16 /*0x10*/;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_495860(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00495860  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00495861  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00495862  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00495864  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x00495866:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00495866  3b1558f37900           +cmp edx, dword ptr [0x79f358]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7992152) /* 0x79f358 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049586c  7d0f                   -jge 0x49587d
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0049587d;
    }
    // 0049586e  8b04d594f27900         -mov eax, dword ptr [edx*8 + 0x79f294]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7991956) /* 0x79f294 */ + cpu.edx * 8);
    // 00495875  e816c00400             -call 0x4e1890
    cpu.esp -= 4;
    nfs3hp::ea::purgememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049587a  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0049587b  ebe9                   -jmp 0x495866
    goto L_0x00495866;
L_0x0049587d:
    // 0049587d  e88e9a0400             -call 0x4df310
    cpu.esp -= 4;
    sub_4df310(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495882  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00495883  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00495884  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_495890(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00495890  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00495891  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00495892  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00495894  8b1558f37900           -mov edx, dword ptr [0x79f358]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992152) /* 0x79f358 */);
    // 0049589a  a154f37900             -mov eax, dword ptr [0x79f354]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992148) /* 0x79f354 */);
    // 0049589f  39d0                   +cmp eax, edx
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
    // 004958a1  7505                   -jne 0x4958a8
    if (!cpu.flags.zf)
    {
        goto L_0x004958a8;
    }
    // 004958a3  31d0                   -xor eax, edx
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004958a5  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004958a6  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004958a7  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004958a8:
    // 004958a8  8b04c594f27900         -mov eax, dword ptr [eax*8 + 0x79f294]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7991956) /* 0x79f294 */ + cpu.eax * 8);
    // 004958af  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004958b0  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004958b1  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_4958c0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004958c0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004958c1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004958c2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004958c3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004958c4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004958c5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004958c6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004958c8  8b0d54f37900           -mov ecx, dword ptr [0x79f354]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992148) /* 0x79f354 */);
    // 004958ce  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004958d0  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004958d2  8b1ccd94f27900         -mov ebx, dword ptr [ecx*8 + 0x79f294]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7991956) /* 0x79f294 */ + cpu.ecx * 8);
L_0x004958d9:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004958d9  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004958db  7e20                   -jle 0x4958fd
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004958fd;
    }
    // 004958dd  8d34c500000000         -lea esi, [eax*8]
    cpu.esi = x86::reg32(cpu.eax * 8);
    // 004958e4  3b9688f27900           +cmp edx, dword ptr [esi + 0x79f288]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.esi + x86::reg32(7991944) /* 0x79f288 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004958ea  7f11                   -jg 0x4958fd
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x004958fd;
    }
    // 004958ec  8dbe90f27900           -lea edi, [esi + 0x79f290]
    cpu.edi = x86::reg32(cpu.esi + x86::reg32(7991952) /* 0x79f290 */);
    // 004958f2  8db688f27900           -lea esi, [esi + 0x79f288]
    cpu.esi = x86::reg32(cpu.esi + x86::reg32(7991944) /* 0x79f288 */);
    // 004958f8  48                     +dec eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 004958f9  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004958fa  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004958fb  ebdc                   -jmp 0x4958d9
    goto L_0x004958d9;
L_0x004958fd:
    // 004958fd  8914c590f27900         -mov dword ptr [eax*8 + 0x79f290], edx
    app->getMemory<x86::reg32>(x86::reg32(7991952) /* 0x79f290 */ + cpu.eax * 8) = cpu.edx;
    // 00495904  41                     -inc ecx
    (cpu.ecx)++;
    // 00495905  891cc594f27900         -mov dword ptr [eax*8 + 0x79f294], ebx
    app->getMemory<x86::reg32>(x86::reg32(7991956) /* 0x79f294 */ + cpu.eax * 8) = cpu.ebx;
    // 0049590c  890d54f37900           -mov dword ptr [0x79f354], ecx
    app->getMemory<x86::reg32>(x86::reg32(7992148) /* 0x79f354 */) = cpu.ecx;
    // 00495912  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00495913  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00495914  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00495915  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00495916  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00495917  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00495918  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_495920(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00495920  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00495921  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00495922  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00495923  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00495924  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00495925  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00495926  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00495928  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0049592a  833d54f3790000         +cmp dword ptr [0x79f354], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7992148) /* 0x79f354 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00495931  7507                   -jne 0x49593a
    if (!cpu.flags.zf)
    {
        goto L_0x0049593a;
    }
    // 00495933  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 00495935  e9b2000000             -jmp 0x4959ec
    goto L_0x004959ec;
L_0x0049593a:
    // 0049593a  3b0590f27900           +cmp eax, dword ptr [0x79f290]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7991952) /* 0x79f290 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00495940  7d09                   -jge 0x49594b
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0049594b;
    }
    // 00495942  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00495944  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00495945  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00495946  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00495947  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00495948  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00495949  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0049594a  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0049594b:
    // 0049594b  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
L_0x00495950:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00495950  3b1554f37900           +cmp edx, dword ptr [0x79f354]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7992148) /* 0x79f354 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00495956  7d0c                   -jge 0x495964
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00495964;
    }
    // 00495958  3b0cd590f27900         +cmp ecx, dword ptr [edx*8 + 0x79f290]
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7991952) /* 0x79f290 */ + cpu.edx * 8)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049595f  7c03                   -jl 0x495964
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00495964;
    }
    // 00495961  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00495962  ebec                   -jmp 0x495950
    goto L_0x00495950;
L_0x00495964:
    // 00495964  8b04d58cf27900         -mov eax, dword ptr [edx*8 + 0x79f28c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7991948) /* 0x79f28c */ + cpu.edx * 8);
    // 0049596b  e840990400             -call 0x4df2b0
    cpu.esp -= 4;
    sub_4df2b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495970  8b04d588f27900         -mov eax, dword ptr [edx*8 + 0x79f288]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7991944) /* 0x79f288 */ + cpu.edx * 8);
    // 00495977  8b1d60f37900           -mov ebx, dword ptr [0x79f360]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7992160) /* 0x79f360 */);
    // 0049597d  29c1                   -sub ecx, eax
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 0049597f  8b3540f37900           -mov esi, dword ptr [0x79f340]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7992128) /* 0x79f340 */);
    // 00495985  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00495987  8b0d5cf37900           -mov ecx, dword ptr [0x79f35c]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992156) /* 0x79f35c */);
    // 0049598d  46                     -inc esi
    (cpu.esi)++;
    // 0049598e  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00495990  8d42ff                 -lea eax, [edx - 1]
    cpu.eax = x86::reg32(cpu.edx + x86::reg32(-1) /* -0x1 */);
    // 00495993  893540f37900           -mov dword ptr [0x79f340], esi
    app->getMemory<x86::reg32>(x86::reg32(7992128) /* 0x79f340 */) = cpu.esi;
    // 00495999  01c3                   -add ebx, eax
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0049599b  890d5cf37900           -mov dword ptr [0x79f35c], ecx
    app->getMemory<x86::reg32>(x86::reg32(7992156) /* 0x79f35c */) = cpu.ecx;
    // 004959a1  891d60f37900           -mov dword ptr [0x79f360], ebx
    app->getMemory<x86::reg32>(x86::reg32(7992160) /* 0x79f360 */) = cpu.ebx;
    // 004959a7  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x004959a9:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004959a9  8b0d58f37900           -mov ecx, dword ptr [0x79f358]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992152) /* 0x79f358 */);
    // 004959af  29d1                   -sub ecx, edx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004959b1  39c8                   +cmp eax, ecx
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
    // 004959b3  7d24                   -jge 0x4959d9
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004959d9;
    }
    // 004959b5  8d0c10                 -lea ecx, [eax + edx]
    cpu.ecx = x86::reg32(cpu.eax + cpu.edx * 1);
    // 004959b8  8d3cc590f27900         -lea edi, [eax*8 + 0x79f290]
    cpu.edi = x86::reg32(x86::reg32(7991952) /* 0x79f290 */ + cpu.eax * 8);
    // 004959bf  8d34cd90f27900         -lea esi, [ecx*8 + 0x79f290]
    cpu.esi = x86::reg32(x86::reg32(7991952) /* 0x79f290 */ + cpu.ecx * 8);
    // 004959c6  8b1cc594f27900         -mov ebx, dword ptr [eax*8 + 0x79f294]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7991956) /* 0x79f294 */ + cpu.eax * 8);
    // 004959cd  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004959ce  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004959cf  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004959d0  891ccd94f27900         -mov dword ptr [ecx*8 + 0x79f294], ebx
    app->getMemory<x86::reg32>(x86::reg32(7991956) /* 0x79f294 */ + cpu.ecx * 8) = cpu.ebx;
    // 004959d7  ebd0                   -jmp 0x4959a9
    goto L_0x004959a9;
L_0x004959d9:
    // 004959d9  8b3d54f37900           -mov edi, dword ptr [0x79f354]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7992148) /* 0x79f354 */);
    // 004959df  29d7                   -sub edi, edx
    (cpu.edi) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004959e1  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004959e6  893d54f37900           -mov dword ptr [0x79f354], edi
    app->getMemory<x86::reg32>(x86::reg32(7992148) /* 0x79f354 */) = cpu.edi;
L_0x004959ec:
    // 004959ec  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004959ed  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004959ee  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004959ef  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004959f0  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004959f1  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004959f2  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_495a00: native, nfs3hp::dead::stub_0x495a00 */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_495a10(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00495a10  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00495a11  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00495a12  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00495a14  a33cf37900             -mov dword ptr [0x79f33c], eax
    app->getMemory<x86::reg32>(x86::reg32(7992124) /* 0x79f33c */) = cpu.eax;
    // 00495a19  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00495a1b  7c07                   -jl 0x495a24
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00495a24;
    }
    // 00495a1d  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00495a22  eb02                   -jmp 0x495a26
    goto L_0x00495a26;
L_0x00495a24:
    // 00495a24  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x00495a26:
    // 00495a26  a338f37900             -mov dword ptr [0x79f338], eax
    app->getMemory<x86::reg32>(x86::reg32(7992120) /* 0x79f338 */) = cpu.eax;
    // 00495a2b  e860cd0500             -call 0x4f2790
    cpu.esp -= 4;
    sub_4f2790(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495a30  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00495a32  a328f37900             -mov dword ptr [0x79f328], eax
    app->getMemory<x86::reg32>(x86::reg32(7992104) /* 0x79f328 */) = cpu.eax;
    // 00495a37  891520f37900           -mov dword ptr [0x79f320], edx
    app->getMemory<x86::reg32>(x86::reg32(7992096) /* 0x79f320 */) = cpu.edx;
    // 00495a3d  891518f37900           -mov dword ptr [0x79f318], edx
    app->getMemory<x86::reg32>(x86::reg32(7992088) /* 0x79f318 */) = cpu.edx;
    // 00495a43  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00495a44  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00495a45  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x54 0x22 0x00 */
void Application::sub_495a50(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00495a50  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00495a51  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00495a52  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00495a53  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00495a54  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00495a55  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00495a56  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00495a58  83ec10                 -sub esp, 0x10
    (cpu.esp) -= x86::reg32(x86::sreg32(16 /*0x10*/));
    // 00495a5b  8b0d18f37900           -mov ecx, dword ptr [0x79f318]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992088) /* 0x79f318 */);
    // 00495a61  e82acd0500             -call 0x4f2790
    cpu.esp -= 4;
    sub_4f2790(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495a66  8b1528f37900           -mov edx, dword ptr [0x79f328]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992104) /* 0x79f328 */);
    // 00495a6c  8b3538f37900           -mov esi, dword ptr [0x79f338]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7992120) /* 0x79f338 */);
    // 00495a72  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00495a74  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 00495a76  29d3                   -sub ebx, edx
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00495a78  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 00495a7a  0f8472000000           -je 0x495af2
    if (cpu.flags.zf)
    {
        goto L_0x00495af2;
    }
    // 00495a80  8d55f0                 -lea edx, [ebp - 0x10]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00495a83  a13cf37900             -mov eax, dword ptr [0x79f33c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992124) /* 0x79f33c */);
    // 00495a88  e8c3d40500             -call 0x4f2f50
    cpu.esp -= 4;
    nfs3hp::port::SNDSTRM_requeststatus(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495a8d  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00495a90  3b0520f37900           +cmp eax, dword ptr [0x79f320]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7992096) /* 0x79f320 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00495a96  7e5a                   -jle 0x495af2
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00495af2;
    }
    // 00495a98  8b0d18f37900           -mov ecx, dword ptr [0x79f318]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992088) /* 0x79f318 */);
    // 00495a9e  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00495aa0  a320f37900             -mov dword ptr [0x79f320], eax
    app->getMemory<x86::reg32>(x86::reg32(7992096) /* 0x79f320 */) = cpu.eax;
    // 00495aa5  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 00495aa7  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00495aa9  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00495aac  c1e203                 +shl edx, 3
    {
        x86::reg8 tmp = 3 /*0x3*/ % 32;
        x86::reg32& op = cpu.edx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 00495aaf  1bc2                   -sbb eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 00495ab1  c1f803                 -sar eax, 3
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (3 /*0x3*/ % 32));
    // 00495ab4  29de                   -sub esi, ebx
    (cpu.esi) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 00495ab6  29c1                   -sub ecx, eax
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 00495ab8  01f1                   -add ecx, esi
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.esi));
    // 00495aba  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 00495abc  7e04                   -jle 0x495ac2
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00495ac2;
    }
    // 00495abe  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00495ac0  eb04                   -jmp 0x495ac6
    goto L_0x00495ac6;
L_0x00495ac2:
    // 00495ac2  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00495ac4  f7d8                   -neg eax
    cpu.eax = ~cpu.eax + 1;
L_0x00495ac6:
    // 00495ac6  3d08010000             +cmp eax, 0x108
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(264 /*0x108*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00495acb  7e1f                   -jle 0x495aec
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00495aec;
    }
    // 00495acd  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 00495acf  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00495ad1  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00495ad4  c1e203                 +shl edx, 3
    {
        x86::reg8 tmp = 3 /*0x3*/ % 32;
        x86::reg32& op = cpu.edx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 00495ad7  1bc2                   -sbb eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 00495ad9  c1f803                 -sar eax, 3
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (3 /*0x3*/ % 32));
    // 00495adc  290528f37900           -sub dword ptr [0x79f328], eax
    (app->getMemory<x86::reg32>(x86::reg32(7992104) /* 0x79f328 */)) -= x86::reg32(x86::sreg32(cpu.eax));
    // 00495ae2  2b3d28f37900           -sub edi, dword ptr [0x79f328]
    (cpu.edi) -= x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7992104) /* 0x79f328 */)));
    // 00495ae8  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00495aea  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
L_0x00495aec:
    // 00495aec  890d18f37900           -mov dword ptr [0x79f318], ecx
    app->getMemory<x86::reg32>(x86::reg32(7992088) /* 0x79f318 */) = cpu.ecx;
L_0x00495af2:
    // 00495af2  8b0d18f37900           -mov ecx, dword ptr [0x79f318]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992088) /* 0x79f318 */);
    // 00495af8  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 00495afa  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00495afc  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00495afd  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00495afe  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00495aff  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00495b00  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00495b01  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00495b02  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_495b10(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00495b10  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00495b11  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00495b13  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00495b18  e843befbff             -call 0x451960
    cpu.esp -= 4;
    nfs3hp::port::text::menuKey(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495b1d  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00495b1f  740a                   -je 0x495b2b
    if (cpu.flags.zf)
    {
        goto L_0x00495b2b;
    }
    // 00495b21  c70530f3790001000000   -mov dword ptr [0x79f330], 1
    app->getMemory<x86::reg32>(x86::reg32(7992112) /* 0x79f330 */) = 1 /*0x1*/;
L_0x00495b2b:
    // 00495b2b  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00495b2c  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_495b30(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00495b30  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00495b31  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00495b32  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00495b33  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00495b35  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
L_0x00495b37:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00495b37  e8c4a70300             -call 0x4d0300
    cpu.esp -= 4;
    sub_4d0300(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495b3c  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00495b3e  740d                   -je 0x495b4d
    if (cpu.flags.zf)
    {
        goto L_0x00495b4d;
    }
    // 00495b40  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00495b42  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00495b44  e877a80300             -call 0x4d03c0
    cpu.esp -= 4;
    sub_4d03c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495b49  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00495b4b  ebea                   -jmp 0x495b37
    goto L_0x00495b37;
L_0x00495b4d:
    // 00495b4d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00495b4e  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00495b4f  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00495b50  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_495b60(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00495b60  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00495b61  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00495b62  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00495b63  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00495b64  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00495b66  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
L_0x00495b68:
    app->unlockContext(cpu);
    win32::Thread::sleep(0);
    app->lockContext(cpu);
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00495b68  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00495b6a  e891a70300             -call 0x4d0300
    cpu.esp -= 4;
    sub_4d0300(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495b6f  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00495b71  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00495b73  742e                   -je 0x495ba3
    if (cpu.flags.zf)
    {
        goto L_0x00495ba3;
    }
    // 00495b75  8b18                   -mov ebx, dword ptr [eax]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax);
    // 00495b77  81fb4d41446d           +cmp ebx, 0x6d44414d
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1833189709 /*0x6d44414d*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00495b7d  7410                   -je 0x495b8f
    if (cpu.flags.zf)
    {
        goto L_0x00495b8f;
    }
    // 00495b7f  81fb4d414465           +cmp ebx, 0x6544414d
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1698971981 /*0x6544414d*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00495b85  7408                   -je 0x495b8f
    if (cpu.flags.zf)
    {
        goto L_0x00495b8f;
    }
    // 00495b87  81fb4d41446b           +cmp ebx, 0x6b44414d
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1799635277 /*0x6b44414d*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00495b8d  750d                   -jne 0x495b9c
    if (!cpu.flags.zf)
    {
        goto L_0x00495b9c;
    }
L_0x00495b8f:
    // 00495b8f  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00495b91  ff052cf37900           -inc dword ptr [0x79f32c]
    (app->getMemory<x86::reg32>(x86::reg32(7992108) /* 0x79f32c */))++;
    // 00495b97  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00495b98  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00495b99  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00495b9a  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00495b9b  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00495b9c:
    // 00495b9c  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00495b9e  e81da80300             -call 0x4d03c0
    cpu.esp -= 4;
    sub_4d03c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00495ba3:
    // 00495ba3  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00495ba5  e816a90300             -call 0x4d04c0
    cpu.esp -= 4;
    sub_4d04c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495baa  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00495bac  74ba                   -je 0x495b68
    if (cpu.flags.zf)
    {
        goto L_0x00495b68;
    }
    // 00495bae  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00495bb0  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00495bb1  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00495bb2  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00495bb3  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00495bb4  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_495bc0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  MovieSession movieSession;
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    if (madPlay(app, cpu.sync())) /* port: native (tools/apply_native_movie.py) */
    {
        cpu.reload();
        return;
    }
    // 00495bc0  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00495bc1  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00495bc2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00495bc3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00495bc5  83ec68                 -sub esp, 0x68
    (cpu.esp) -= x86::reg32(x86::sreg32(104 /*0x68*/));
    // 00495bc8  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 00495bca  8955d4                 -mov dword ptr [ebp - 0x2c], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44) /* -0x2c */) = cpu.edx;
    // 00495bcd  895dc4                 -mov dword ptr [ebp - 0x3c], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-60) /* -0x3c */) = cpu.ebx;
    // 00495bd0  894dd0                 -mov dword ptr [ebp - 0x30], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-48) /* -0x30 */) = cpu.ecx;
    // 00495bd3  ba00001000             -mov edx, 0x100000
    cpu.edx = 1048576 /*0x100000*/;
    // 00495bd8  b888be5300             -mov eax, 0x53be88
    cpu.eax = 5488264 /*0x53be88*/;
    // 00495bdd  8b1df4435600           -mov ebx, dword ptr [0x5643f4]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5653492) /* 0x5643f4 */);
    // 00495be3  e838ba0400             -call 0x4e1620
    cpu.esp -= 4;
    nfs3hp::ea::reservememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495be8  6800001000             -push 0x100000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1048576 /*0x100000*/;
    cpu.esp -= 4;
    // 00495bed  bb02000000             -mov ebx, 2
    cpu.ebx = 2 /*0x2*/;
    // 00495bf2  8945c0                 -mov dword ptr [ebp - 0x40], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-64) /* -0x40 */) = cpu.eax;
    // 00495bf5  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00495bf7  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 00495bf9  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 00495bfb  e8809e0300             -call 0x4cfa80
    cpu.esp -= 4;
    sub_4cfa80(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495c00  ba02000000             -mov edx, 2
    cpu.edx = 2 /*0x2*/;
    // 00495c05  b953430000             -mov ecx, 0x4353
    cpu.ecx = 17235 /*0x4353*/;
    // 00495c0a  bbffff0000             -mov ebx, 0xffff
    cpu.ebx = 65535 /*0xffff*/;
    // 00495c0f  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00495c11  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 00495c14  e857a20300             -call 0x4cfe70
    cpu.esp -= 4;
    sub_4cfe70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495c19  6a02                   -push 2
    app->getMemory<x86::reg32>(cpu.esp-4) = 2 /*0x2*/;
    cpu.esp -= 4;
    // 00495c1b  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 00495c20  8945dc                 -mov dword ptr [ebp - 0x24], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = cpu.eax;
    // 00495c23  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00495c25  e876a00300             -call 0x4cfca0
    cpu.esp -= 4;
    sub_4cfca0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495c2a  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 00495c2c  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00495c2e  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00495c30  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00495c32  e8b9a20300             -call 0x4cfef0
    cpu.esp -= 4;
    sub_4cfef0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495c37  8945cc                 -mov dword ptr [ebp - 0x34], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-52) /* -0x34 */) = cpu.eax;
    // 00495c3a  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00495c3c  e81fffffff             -call 0x495b60
    cpu.esp -= 4;
    sub_495b60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495c41  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00495c43  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00495c45  750d                   -jne 0x495c54
    if (!cpu.flags.zf)
    {
        goto L_0x00495c54;
    }
    // 00495c47  688cbe5300             -push 0x53be8c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5488268 /*0x53be8c*/;
    cpu.esp -= 4;
    // 00495c4c  e8bfb3f6ff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495c51  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
L_0x00495c54:
    // 00495c54  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 00495c59  8b460c                 -mov eax, dword ptr [esi + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(12) /* 0xc */);
    // 00495c5c  8b7dd0                 -mov edi, dword ptr [ebp - 0x30]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-48) /* -0x30 */);
    // 00495c5f  a310f37900             -mov dword ptr [0x79f310], eax
    app->getMemory<x86::reg32>(x86::reg32(7992080) /* 0x79f310 */) = cpu.eax;
    // 00495c64  8b460e                 -mov eax, dword ptr [esi + 0xe]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(14) /* 0xe */);
    // 00495c67  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00495c69  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00495c6c  89152cf37900           -mov dword ptr [0x79f32c], edx
    app->getMemory<x86::reg32>(x86::reg32(7992108) /* 0x79f32c */) = cpu.edx;
    // 00495c72  a324f37900             -mov dword ptr [0x79f324], eax
    app->getMemory<x86::reg32>(x86::reg32(7992100) /* 0x79f324 */) = cpu.eax;
    // 00495c77  8b4610                 -mov eax, dword ptr [esi + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(16) /* 0x10 */);
    // 00495c7a  890d14f37900           -mov dword ptr [0x79f314], ecx
    app->getMemory<x86::reg32>(x86::reg32(7992084) /* 0x79f314 */) = cpu.ecx;
    // 00495c80  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00495c83  890d34f37900           -mov dword ptr [0x79f334], ecx
    app->getMemory<x86::reg32>(x86::reg32(7992116) /* 0x79f334 */) = cpu.ecx;
    // 00495c89  a31cf37900             -mov dword ptr [0x79f31c], eax
    app->getMemory<x86::reg32>(x86::reg32(7992092) /* 0x79f31c */) = cpu.eax;
    // 00495c8e  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 00495c90  7531                   -jne 0x495cc3
    if (!cpu.flags.zf)
    {
        goto L_0x00495cc3;
    }
    // 00495c92  a124f37900             -mov eax, dword ptr [0x79f324]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992100) /* 0x79f324 */);
    // 00495c97  8b1584435600           -mov edx, dword ptr [0x564384]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5653380) /* 0x564384 */);
    // 00495c9d  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 00495c9f  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00495ca1  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00495ca4  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00495ca6  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 00495ca8  8b0d1cf37900           -mov ecx, dword ptr [0x79f31c]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992092) /* 0x79f31c */);
    // 00495cae  8b1588435600           -mov edx, dword ptr [0x564388]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5653384) /* 0x564388 */);
    // 00495cb4  29ca                   -sub edx, ecx
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 00495cb6  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 00495cb8  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00495cba  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00495cbd  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00495cbf  d1f8                   +sar eax, 1
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
    // 00495cc1  eb2e                   -jmp 0x495cf1
    goto L_0x00495cf1;
L_0x00495cc3:
    // 00495cc3  a184435600             -mov eax, dword ptr [0x564384]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5653380) /* 0x564384 */);
    // 00495cc8  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00495cca  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00495ccd  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00495ccf  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 00495cd1  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 00495cd3  a188435600             -mov eax, dword ptr [0x564388]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5653384) /* 0x564388 */);
    // 00495cd8  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00495cda  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00495cdd  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00495cdf  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 00495ce1  8b1d24f37900           -mov ebx, dword ptr [0x79f324]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7992100) /* 0x79f324 */);
    // 00495ce7  8b151cf37900           -mov edx, dword ptr [0x79f31c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992092) /* 0x79f31c */);
    // 00495ced  29df                   -sub edi, ebx
    (cpu.edi) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 00495cef  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
L_0x00495cf1:
    // 00495cf1  8b4d10                 -mov ecx, dword ptr [ebp + 0x10]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(16) /* 0x10 */);
    // 00495cf4  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00495cf5  8b5dd0                 -mov ebx, dword ptr [ebp - 0x30]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-48) /* -0x30 */);
    // 00495cf8  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00495cf9  8b55c4                 -mov edx, dword ptr [ebp - 0x3c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-60) /* -0x3c */);
    // 00495cfc  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00495cfd  8b0d1cf37900           -mov ecx, dword ptr [0x79f31c]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992092) /* 0x79f31c */);
    // 00495d03  8b1d24f37900           -mov ebx, dword ptr [0x79f324]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7992100) /* 0x79f324 */);
    // 00495d09  6a06                   -push 6
    app->getMemory<x86::reg32>(cpu.esp-4) = 6 /*0x6*/;
    cpu.esp -= 4;
    // 00495d0b  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00495d0d  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 00495d0f  e88cfaffff             -call 0x4957a0
    cpu.esp -= 4;
    sub_4957a0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495d14  e877fbffff             -call 0x495890
    cpu.esp -= 4;
    sub_495890(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495d19  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00495d1b  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 00495d1d  894dfc                 -mov dword ptr [ebp - 4], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
    // 00495d20  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
L_0x00495d22:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00495d22  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 00495d24  0f84cf000000           -je 0x495df9
    if (cpu.flags.zf)
    {
        goto L_0x00495df9;
    }
    // 00495d2a  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 00495d2c  0f84c7000000           -je 0x495df9
    if (cpu.flags.zf)
    {
        goto L_0x00495df9;
    }
    // 00495d32  8b1e                   -mov ebx, dword ptr [esi]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esi);
    // 00495d34  8d4618                 -lea eax, [esi + 0x18]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(24) /* 0x18 */);
    // 00495d37  81fb4d41446b           +cmp ebx, 0x6b44414d
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1799635277 /*0x6b44414d*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00495d3d  7506                   -jne 0x495d45
    if (!cpu.flags.zf)
    {
        goto L_0x00495d45;
    }
    // 00495d3f  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00495d41  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 00495d43  eb07                   -jmp 0x495d4c
    goto L_0x00495d4c;
L_0x00495d45:
    // 00495d45  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00495d47  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
L_0x00495d4c:
    // 00495d4c  8a5e15                 -mov bl, byte ptr [esi + 0x15]
    cpu.bl = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(21) /* 0x15 */);
    // 00495d4f  e80cd80500             -call 0x4f3560
    cpu.esp -= 4;
    sub_4f3560(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495d54  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00495d56  8945ec                 -mov dword ptr [ebp - 0x14], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.eax;
L_0x00495d59:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00495d59  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00495d5c  3b051cf37900           +cmp eax, dword ptr [0x79f31c]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7992092) /* 0x79f31c */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00495d62  7d47                   -jge 0x495dab
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00495dab;
    }
    // 00495d64  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00495d66  895df0                 -mov dword ptr [ebp - 0x10], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.ebx;
L_0x00495d69:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00495d69  8b45f0                 -mov eax, dword ptr [ebp - 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 00495d6c  3b0524f37900           +cmp eax, dword ptr [0x79f324]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7992100) /* 0x79f324 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00495d72  7d31                   -jge 0x495da5
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00495da5;
    }
    // 00495d74  8b55ec                 -mov edx, dword ptr [ebp - 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 00495d77  8b1d24f37900           -mov ebx, dword ptr [0x79f324]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7992100) /* 0x79f324 */);
    // 00495d7d  0fafd3                 -imul edx, ebx
    cpu.edx = x86::reg32(x86::sreg64(x86::sreg32(cpu.edx)) * x86::sreg64(x86::sreg32(cpu.ebx)));
    // 00495d80  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00495d82  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00495d84  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00495d87  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00495d89  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 00495d8b  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00495d8e  8d1401                 -lea edx, [ecx + eax]
    cpu.edx = x86::reg32(cpu.ecx + cpu.eax * 1);
    // 00495d91  8955b8                 -mov dword ptr [ebp - 0x48], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-72) /* -0x48 */) = cpu.edx;
    // 00495d94  0345e0                 -add eax, dword ptr [ebp - 0x20]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */)));
    // 00495d97  8b55b8                 -mov edx, dword ptr [ebp - 0x48]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-72) /* -0x48 */);
    // 00495d9a  e861d80500             -call 0x4f3600
    cpu.esp -= 4;
    sub_4f3600(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495d9f  8345f010               +add dword ptr [ebp - 0x10], 0x10
    {
        auto tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(16 /*0x10*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00495da3  ebc4                   -jmp 0x495d69
    goto L_0x00495d69;
L_0x00495da5:
    // 00495da5  8345ec10               +add dword ptr [ebp - 0x14], 0x10
    {
        auto tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(16 /*0x10*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00495da9  ebae                   -jmp 0x495d59
    goto L_0x00495d59;
L_0x00495dab:
    // 00495dab  813e4d414465           +cmp dword ptr [esi], 0x6544414d
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esi);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1698971981 /*0x6544414d*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00495db1  7403                   -je 0x495db6
    if (cpu.flags.zf)
    {
        goto L_0x00495db6;
    }
    // 00495db3  894de0                 -mov dword ptr [ebp - 0x20], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */) = cpu.ecx;
L_0x00495db6:
    // 00495db6  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00495db9  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 00495dbb  e800a60300             -call 0x4d03c0
    cpu.esp -= 4;
    sub_4d03c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495dc0  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00495dc3  e8f8faffff             -call 0x4958c0
    cpu.esp -= 4;
    sub_4958c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495dc8  8b0d10f37900           -mov ecx, dword ptr [0x79f310]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7992080) /* 0x79f310 */);
    // 00495dce  01cf                   -add edi, ecx
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.ecx));
    // 00495dd0  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 00495dd2  8b5dfc                 -mov ebx, dword ptr [ebp - 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00495dd5  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00495dd8  81e7ffff0000           -and edi, 0xffff
    cpu.edi &= x86::reg32(x86::sreg32(65535 /*0xffff*/));
    // 00495dde  01c3                   +add ebx, eax
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
    // 00495de0  e8abfaffff             -call 0x495890
    cpu.esp -= 4;
    sub_495890(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495de5  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00495de7  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00495dea  895dfc                 -mov dword ptr [ebp - 4], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ebx;
    // 00495ded  e86efdffff             -call 0x495b60
    cpu.esp -= 4;
    sub_495b60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495df2  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00495df4  e929ffffff             -jmp 0x495d22
    goto L_0x00495d22;
L_0x00495df9:
    // 00495df9  8d4598                 -lea eax, [ebp - 0x68]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-104) /* -0x68 */);
    // 00495dfc  ba1e000000             -mov edx, 0x1e
    cpu.edx = 30 /*0x1e*/;
    // 00495e01  e81a3d0500             -call 0x4e9b20
    cpu.esp -= 4;
    nfs3hp::port::SNDplaysetdef(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495e06  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00495e0b  e800dc0500             -call 0x4f3a10
    cpu.esp -= 4;
    nfs3hp::port::streamMemory(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495e10  8b1df4435600           -mov ebx, dword ptr [0x5643f4]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5653492) /* 0x5643f4 */);
    // 00495e16  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00495e18  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00495e1a  b888be5300             -mov eax, 0x53be88
    cpu.eax = 5488264 /*0x53be88*/;
    // 00495e1f  e8fcb70400             -call 0x4e1620
    cpu.esp -= 4;
    nfs3hp::ea::reservememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495e24  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 00495e29  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00495e2a  8d5598                 -lea edx, [ebp - 0x68]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-104) /* -0x68 */);
    // 00495e2d  8945c8                 -mov dword ptr [ebp - 0x38], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-56) /* -0x38 */) = cpu.eax;
    // 00495e30  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00495e31  b91e000000             -mov ecx, 0x1e
    cpu.ecx = 30 /*0x1e*/;
    // 00495e36  8b45dc                 -mov eax, dword ptr [ebp - 0x24]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 00495e39  e822dc0500             -call 0x4f3a60
    cpu.esp -= 4;
    nfs3hp::port::SNDSTRM_createqueued(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495e3e  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00495e40  8945bc                 -mov dword ptr [ebp - 0x44], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-68) /* -0x44 */) = cpu.eax;
    // 00495e43  8b45dc                 -mov eax, dword ptr [ebp - 0x24]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 00495e46  e815a60300             -call 0x4d0460
    cpu.esp -= 4;
    sub_4d0460(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495e4b  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00495e4d  7507                   -jne 0x495e56
    if (!cpu.flags.zf)
    {
        goto L_0x00495e56;
    }
    // 00495e4f  b9ffffffff             -mov ecx, 0xffffffff
    cpu.ecx = 4294967295 /*0xffffffff*/;
    // 00495e54  eb0e                   -jmp 0x495e64
    goto L_0x00495e64;
L_0x00495e56:
    // 00495e56  8b5dcc                 -mov ebx, dword ptr [ebp - 0x34]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-52) /* -0x34 */);
    // 00495e59  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00495e5b  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00495e5d  e81edc0500             -call 0x4f3a80
    cpu.esp -= 4;
    nfs3hp::port::SNDSTRM_requestdata(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495e62  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
L_0x00495e64:
    // 00495e64  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00495e66  e8a5fbffff             -call 0x495a10
    cpu.esp -= 4;
    sub_495a10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495e6b  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00495e6d  a330f37900             -mov dword ptr [0x79f330], eax
    app->getMemory<x86::reg32>(x86::reg32(7992112) /* 0x79f330 */) = cpu.eax;
L_0x00495e72:
    if (movieSkipRequested()) {
        /* Same abort marker as the player's event type 2 (Escape). */
        auto abort = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44));
        app->getMemory<x86::reg32>(abort) = 1;
        goto L_0x004960b3;
    }
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00495e72  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 00495e74  0f84f8010000           -je 0x496072
    if (cpu.flags.zf)
    {
        goto L_0x00496072;
    }
    // 00495e7a  833d30f3790000         +cmp dword ptr [0x79f330], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7992112) /* 0x79f330 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00495e81  0f85eb010000           -jne 0x496072
    if (!cpu.flags.zf)
    {
        goto L_0x00496072;
    }
    // 00495e87  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00495e89  e8a2170500             -call 0x4e7630
    cpu.esp -= 4;
    nfs3hp::port::runSystemTasks(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495e8e  e8ad940400             -call 0x4df340
    cpu.esp -= 4;
    sub_4df340(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495e93  83f802                 +cmp eax, 2
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
    // 00495e96  7509                   -jne 0x495ea1
    if (!cpu.flags.zf)
    {
        goto L_0x00495ea1;
    }
    // 00495e98  8b45d4                 -mov eax, dword ptr [ebp - 0x2c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44) /* -0x2c */);
    // 00495e9b  c70001000000           -mov dword ptr [eax], 1
    app->getMemory<x86::reg32>(cpu.eax) = 1 /*0x1*/;
L_0x00495ea1:
    // 00495ea1  e8aafbffff             -call 0x495a50
    cpu.esp -= 4;
    sub_495a50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495ea6  8b16                   -mov edx, dword ptr [esi]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi);
    // 00495ea8  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00495eaa  81fa4d414465           +cmp edx, 0x6544414d
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1698971981 /*0x6544414d*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00495eb0  7557                   -jne 0x495f09
    if (!cpu.flags.zf)
    {
        goto L_0x00495f09;
    }
    // 00495eb2  8b55d4                 -mov edx, dword ptr [ebp - 0x2c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44) /* -0x2c */);
    // 00495eb5  833a00                 +cmp dword ptr [edx], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.edx);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00495eb8  750d                   -jne 0x495ec7
    if (!cpu.flags.zf)
    {
        goto L_0x00495ec7;
    }
    // 00495eba  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00495ebd  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 00495ebf  81fa85000000           +cmp edx, 0x85
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(133 /*0x85*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00495ec5  7d42                   -jge 0x495f09
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00495f09;
    }
L_0x00495ec7:
    // 00495ec7  a134f37900             -mov eax, dword ptr [0x79f334]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7992116) /* 0x79f334 */);
    // 00495ecc  8b1510f37900           -mov edx, dword ptr [0x79f310]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7992080) /* 0x79f310 */);
    // 00495ed2  40                     -inc eax
    (cpu.eax)++;
    // 00495ed3  01d7                   -add edi, edx
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.edx));
    // 00495ed5  a334f37900             -mov dword ptr [0x79f334], eax
    app->getMemory<x86::reg32>(x86::reg32(7992116) /* 0x79f334 */) = cpu.eax;
    // 00495eda  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 00495edc  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00495edf  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00495ee2  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00495ee4  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00495ee7  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 00495eea  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 00495eec  e8cfa40300             -call 0x4d03c0
    cpu.esp -= 4;
    sub_4d03c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495ef1  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00495ef4  81e7ffff0000           -and edi, 0xffff
    cpu.edi &= x86::reg32(x86::sreg32(65535 /*0xffff*/));
    // 00495efa  e861fcffff             -call 0x495b60
    cpu.esp -= 4;
    sub_495b60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495eff  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00495f01  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00495f03  0f8469010000           -je 0x496072
    if (cpu.flags.zf)
    {
        goto L_0x00496072;
    }
L_0x00495f09:
    // 00495f09  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00495f0c  29d8                   -sub eax, ebx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 00495f0e  83f843                 +cmp eax, 0x43
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(67 /*0x43*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00495f11  7d59                   -jge 0x495f6c
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00495f6c;
    }
L_0x00495f13:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00495f13  813e4d414465           +cmp dword ptr [esi], 0x6544414d
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esi);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1698971981 /*0x6544414d*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00495f19  7408                   -je 0x495f23
    if (cpu.flags.zf)
    {
        goto L_0x00495f23;
    }
    // 00495f1b  ff0514f37900           +inc dword ptr [0x79f314]
    {
        auto tmp = app->getMemory<x86::reg32>(x86::reg32(7992084) /* 0x79f314 */);
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00495f21  eb06                   -jmp 0x495f29
    goto L_0x00495f29;
L_0x00495f23:
    // 00495f23  ff0534f37900           -inc dword ptr [0x79f334]
    (app->getMemory<x86::reg32>(x86::reg32(7992116) /* 0x79f334 */))++;
L_0x00495f29:
    // 00495f29  8b1d10f37900           -mov ebx, dword ptr [0x79f310]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7992080) /* 0x79f310 */);
    // 00495f2f  01df                   -add edi, ebx
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.ebx));
    // 00495f31  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 00495f33  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00495f36  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00495f39  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00495f3b  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00495f3e  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 00495f41  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 00495f43  e878a40300             -call 0x4d03c0
    cpu.esp -= 4;
    sub_4d03c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495f48  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00495f4b  81e7ffff0000           -and edi, 0xffff
    cpu.edi &= x86::reg32(x86::sreg32(65535 /*0xffff*/));
    // 00495f51  e80afcffff             -call 0x495b60
    cpu.esp -= 4;
    sub_495b60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495f56  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00495f58  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00495f5a  7408                   -je 0x495f64
    if (cpu.flags.zf)
    {
        goto L_0x00495f64;
    }
    // 00495f5c  81384d41446b           +cmp dword ptr [eax], 0x6b44414d
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1799635277 /*0x6b44414d*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00495f62  75af                   -jne 0x495f13
    if (!cpu.flags.zf)
    {
        goto L_0x00495f13;
    }
L_0x00495f64:
    // 00495f64  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 00495f66  0f8406010000           -je 0x496072
    if (cpu.flags.zf)
    {
        goto L_0x00496072;
    }
L_0x00495f6c:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00495f6c  e81ff9ffff             -call 0x495890
    cpu.esp -= 4;
    sub_495890(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495f71  8945e4                 -mov dword ptr [ebp - 0x1c], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */) = cpu.eax;
    // 00495f74  837de400               +cmp dword ptr [ebp - 0x1c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00495f78  750c                   -jne 0x495f86
    if (!cpu.flags.zf)
    {
        goto L_0x00495f86;
    }
    // 00495f7a  e8d1faffff             -call 0x495a50
    cpu.esp -= 4;
    sub_495a50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495f7f  e89cf9ffff             -call 0x495920
    cpu.esp -= 4;
    sub_495920(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495f84  ebe6                   -jmp 0x495f6c
    goto L_0x00495f6c;
L_0x00495f86:
    // 00495f86  8b16                   -mov edx, dword ptr [esi]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi);
    // 00495f88  8d4618                 -lea eax, [esi + 0x18]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(24) /* 0x18 */);
    // 00495f8b  81fa4d41446b           +cmp edx, 0x6b44414d
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1799635277 /*0x6b44414d*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00495f91  7506                   -jne 0x495f99
    if (!cpu.flags.zf)
    {
        goto L_0x00495f99;
    }
    // 00495f93  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00495f95  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 00495f97  eb07                   -jmp 0x495fa0
    goto L_0x00495fa0;
L_0x00495f99:
    // 00495f99  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00495f9b  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
L_0x00495fa0:
    // 00495fa0  8a5e15                 -mov bl, byte ptr [esi + 0x15]
    cpu.bl = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(21) /* 0x15 */);
    // 00495fa3  e8b8d50500             -call 0x4f3560
    cpu.esp -= 4;
    sub_4f3560(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495fa8  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00495faa  895df4                 -mov dword ptr [ebp - 0xc], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.ebx;
    // 00495fad  895dd8                 -mov dword ptr [ebp - 0x28], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-40) /* -0x28 */) = cpu.ebx;
L_0x00495fb0:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00495fb0  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00495fb3  3b051cf37900           +cmp eax, dword ptr [0x79f31c]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7992092) /* 0x79f31c */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00495fb9  7d5c                   -jge 0x496017
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00496017;
    }
    // 00495fbb  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00495fbd  895de8                 -mov dword ptr [ebp - 0x18], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = cpu.ebx;
L_0x00495fc0:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00495fc0  8b45e8                 -mov eax, dword ptr [ebp - 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 00495fc3  3b0524f37900           +cmp eax, dword ptr [0x79f324]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7992100) /* 0x79f324 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00495fc9  7d33                   -jge 0x495ffe
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00495ffe;
    }
    // 00495fcb  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 00495fce  8b1d24f37900           -mov ebx, dword ptr [0x79f324]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7992100) /* 0x79f324 */);
    // 00495fd4  0fafd3                 -imul edx, ebx
    cpu.edx = x86::reg32(x86::sreg64(x86::sreg32(cpu.edx)) * x86::sreg64(x86::sreg32(cpu.ebx)));
    // 00495fd7  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00495fd9  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00495fdb  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 00495fde  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00495fe0  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 00495fe2  8b55e4                 -mov edx, dword ptr [ebp - 0x1c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 00495fe5  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00495fe8  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00495fea  8955b8                 -mov dword ptr [ebp - 0x48], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-72) /* -0x48 */) = cpu.edx;
    // 00495fed  0345e0                 -add eax, dword ptr [ebp - 0x20]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */)));
    // 00495ff0  8b55b8                 -mov edx, dword ptr [ebp - 0x48]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-72) /* -0x48 */);
    // 00495ff3  e808d60500             -call 0x4f3600
    cpu.esp -= 4;
    sub_4f3600(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00495ff8  8345e810               +add dword ptr [ebp - 0x18], 0x10
    {
        auto tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(16 /*0x10*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00495ffc  ebc2                   -jmp 0x495fc0
    goto L_0x00495fc0;
L_0x00495ffe:
    // 00495ffe  837dd800               +cmp dword ptr [ebp - 0x28], 0
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
    // 00496002  750d                   -jne 0x496011
    if (!cpu.flags.zf)
    {
        goto L_0x00496011;
    }
    // 00496004  e847faffff             -call 0x495a50
    cpu.esp -= 4;
    sub_495a50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00496009  e812f9ffff             -call 0x495920
    cpu.esp -= 4;
    sub_495920(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049600e  8945d8                 -mov dword ptr [ebp - 0x28], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-40) /* -0x28 */) = cpu.eax;
L_0x00496011:
    // 00496011  8345f410               +add dword ptr [ebp - 0xc], 0x10
    {
        auto tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(16 /*0x10*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00496015  eb99                   -jmp 0x495fb0
    goto L_0x00495fb0;
L_0x00496017:
    // 00496017  813e4d414465           +cmp dword ptr [esi], 0x6544414d
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esi);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1698971981 /*0x6544414d*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0049601d  7406                   -je 0x496025
    if (cpu.flags.zf)
    {
        goto L_0x00496025;
    }
    // 0049601f  8b45e4                 -mov eax, dword ptr [ebp - 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 00496022  8945e0                 -mov dword ptr [ebp - 0x20], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */) = cpu.eax;
L_0x00496025:
    // 00496025  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00496028  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0049602a  e891a30300             -call 0x4d03c0
    cpu.esp -= 4;
    sub_4d03c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049602f  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00496032  e889f8ffff             -call 0x4958c0
    cpu.esp -= 4;
    sub_4958c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00496037  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 00496039  7d08                   -jge 0x496043
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00496043;
    }
    // 0049603b  8b45dc                 -mov eax, dword ptr [ebp - 0x24]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 0049603e  e8edfaffff             -call 0x495b30
    cpu.esp -= 4;
    sub_495b30(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00496043:
    // 00496043  8b3510f37900           -mov esi, dword ptr [0x79f310]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7992080) /* 0x79f310 */);
    // 00496049  01f7                   -add edi, esi
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.esi));
    // 0049604b  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0049604d  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00496050  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00496053  81e7ffff0000           -and edi, 0xffff
    cpu.edi &= x86::reg32(x86::sreg32(65535 /*0xffff*/));
    // 00496059  01c2                   +add edx, eax
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
    // 0049605b  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0049605e  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 00496061  e8fafaffff             -call 0x495b60
    cpu.esp -= 4;
    sub_495b60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00496066  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00496068  e8a3faffff             -call 0x495b10
    cpu.esp -= 4;
    sub_495b10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049606d  e900feffff             -jmp 0x495e72
    goto L_0x00495e72;
L_0x00496072:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00496072  e8d9f9ffff             -call 0x495a50
    cpu.esp -= 4;
    sub_495a50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00496077  3b45fc                 +cmp eax, dword ptr [ebp - 4]
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
    // 0049607a  7d0e                   -jge 0x49608a
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0049608a;
    }
    // 0049607c  e89ff8ffff             -call 0x495920
    cpu.esp -= 4;
    sub_495920(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00496081  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 00496083  e8a8150500             -call 0x4e7630
    cpu.esp -= 4;
    nfs3hp::port::runSystemTasks(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00496088  ebe8                   -jmp 0x496072
    goto L_0x00496072;
L_0x0049608a:
    // 0049608a  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0049608c  7c25                   -jl 0x4960b3
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004960b3;
    }
L_0x0049608e:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0049608e  8d55a8                 -lea edx, [ebp - 0x58]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-88) /* -0x58 */);
    // 00496091  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 00496093  e8b8ce0500             -call 0x4f2f50
    cpu.esp -= 4;
    nfs3hp::port::SNDSTRM_requeststatus(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00496098  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0049609a  e891150500             -call 0x4e7630
    cpu.esp -= 4;
    nfs3hp::port::runSystemTasks(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0049609f  e86cfaffff             -call 0x495b10
    cpu.esp -= 4;
    sub_495b10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004960a4  837da803               +cmp dword ptr [ebp - 0x58], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-88) /* -0x58 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004960a8  7409                   -je 0x4960b3
    if (cpu.flags.zf)
    {
        goto L_0x004960b3;
    }
    // 004960aa  833d30f3790000         +cmp dword ptr [0x79f330], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7992112) /* 0x79f330 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004960b1  74db                   -je 0x49608e
    if (cpu.flags.zf)
    {
        goto L_0x0049608e;
    }
L_0x004960b3:
    // 004960b3  8b45bc                 -mov eax, dword ptr [ebp - 0x44]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-68) /* -0x44 */);
    // 004960b6  e805e30500             -call 0x4f43c0
    cpu.esp -= 4;
    nfs3hp::port::SNDSTRM_destroy(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004960bb  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004960be  e86d9c0300             -call 0x4cfd30
    cpu.esp -= 4;
    sub_4cfd30(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004960c3  8b45c0                 -mov eax, dword ptr [ebp - 0x40]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-64) /* -0x40 */);
    // 004960c6  e8c5b70400             -call 0x4e1890
    cpu.esp -= 4;
    nfs3hp::ea::purgememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004960cb  8b45c8                 -mov eax, dword ptr [ebp - 0x38]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-56) /* -0x38 */);
    // 004960ce  e8bdb70400             -call 0x4e1890
    cpu.esp -= 4;
    nfs3hp::ea::purgememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004960d3  e888f7ffff             -call 0x495860
    cpu.esp -= 4;
    sub_495860(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004960d8  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004960da  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004960db  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004960dc  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004960dd  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
}

/* sub_4960e0: native, nfs3hp::frontend::playMovie */

/* sub_4961e0: native, nfs3hp::frontend::resetAttractTimer */

/* sub_496200: native, nfs3hp::frontend::attractMode */

/* align: skip 0x00 0x00 0x00 */
void Application::sub_496300(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1;
    // 00496300  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00496302  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00496304  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00496306  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00496308  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0049630a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0049630c  8d442000               -lea eax, [eax]
    cpu.eax = x86::reg32(cpu.eax);
    // 00496310  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00496311  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00496312  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00496313  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00496315  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00496318  b964f37900             -mov ecx, 0x79f364
    cpu.ecx = 7992164 /*0x79f364*/;
    // 0049631d  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 0049631f  eb08                   -jmp 0x496329
    goto L_0x00496329;
L_0x00496321:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00496321  81fa80000000           +cmp edx, 0x80
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(128 /*0x80*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00496327  7d2e                   -jge 0x496357
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00496357;
    }
L_0x00496329:
    // 00496329  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0049632b  c1e011                 -shl eax, 0x11
    cpu.eax <<= 17 /*0x11*/ % 32;
    // 0049632e  0d0000003f             -or eax, 0x3f000000
    cpu.eax |= x86::reg32(x86::sreg32(1056964608 /*0x3f000000*/));
    // 00496333  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00496334  e8e7450500             -call 0x4ea920
    cpu.esp -= 4;
    nfs3hp::frontend::simSquareRoot(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 00496339  d9e8                   -fld1 
    fp1 = 1.0;
    // 0049633b  def1                   -fdivrp st(1)
    fp0 = fp::div(fp1, x86::Float(fp0));
    // 0049633d  d95dfc                 -fstp dword ptr [ebp - 4]
    app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = float(fp0);
    // 00496340  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00496343  0500200000             -add eax, 0x2000
    (cpu.eax) += x86::reg32(x86::sreg32(8192 /*0x2000*/));
    // 00496348  c1e80f                 -shr eax, 0xf
    cpu.eax >>= 15 /*0xf*/ % 32;
    // 0049634b  41                     -inc ecx
    (cpu.ecx)++;
    // 0049634c  25ff000000             +and eax, 0xff
    cpu.clear_co();
    cpu.set_szp((cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/))));
    // 00496351  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00496352  8841ff                 -mov byte ptr [ecx - 1], al
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(-1) /* -0x1 */) = cpu.al;
    // 00496355  ebca                   -jmp 0x496321
    goto L_0x00496321;
L_0x00496357:
    // 00496357  c605a4f37900ff         -mov byte ptr [0x79f3a4], 0xff
    app->getMemory<x86::reg8>(x86::reg32(7992228) /* 0x79f3a4 */) = 255 /*0xff*/;
    // 0049635e  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00496360  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00496361  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00496362  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00496363  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_496310(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1;
    goto L_entry_0x00496310;
    // 00496300  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00496302  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00496304  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00496306  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00496308  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0049630a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0049630c  8d442000               -lea eax, [eax]
    cpu.eax = x86::reg32(cpu.eax);
L_entry_0x00496310:
    // 00496310  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00496311  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00496312  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00496313  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00496315  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00496318  b964f37900             -mov ecx, 0x79f364
    cpu.ecx = 7992164 /*0x79f364*/;
    // 0049631d  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 0049631f  eb08                   -jmp 0x496329
    goto L_0x00496329;
L_0x00496321:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00496321  81fa80000000           +cmp edx, 0x80
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(128 /*0x80*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00496327  7d2e                   -jge 0x496357
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00496357;
    }
L_0x00496329:
    // 00496329  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0049632b  c1e011                 -shl eax, 0x11
    cpu.eax <<= 17 /*0x11*/ % 32;
    // 0049632e  0d0000003f             -or eax, 0x3f000000
    cpu.eax |= x86::reg32(x86::sreg32(1056964608 /*0x3f000000*/));
    // 00496333  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00496334  e8e7450500             -call 0x4ea920
    cpu.esp -= 4;
    nfs3hp::frontend::simSquareRoot(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 00496339  d9e8                   -fld1 
    fp1 = 1.0;
    // 0049633b  def1                   -fdivrp st(1)
    fp0 = fp::div(fp1, x86::Float(fp0));
    // 0049633d  d95dfc                 -fstp dword ptr [ebp - 4]
    app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = float(fp0);
    // 00496340  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00496343  0500200000             -add eax, 0x2000
    (cpu.eax) += x86::reg32(x86::sreg32(8192 /*0x2000*/));
    // 00496348  c1e80f                 -shr eax, 0xf
    cpu.eax >>= 15 /*0xf*/ % 32;
    // 0049634b  41                     -inc ecx
    (cpu.ecx)++;
    // 0049634c  25ff000000             +and eax, 0xff
    cpu.clear_co();
    cpu.set_szp((cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/))));
    // 00496351  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 00496352  8841ff                 -mov byte ptr [ecx - 1], al
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(-1) /* -0x1 */) = cpu.al;
    // 00496355  ebca                   -jmp 0x496321
    goto L_0x00496321;
L_0x00496357:
    // 00496357  c605a4f37900ff         -mov byte ptr [0x79f3a4], 0xff
    app->getMemory<x86::reg8>(x86::reg32(7992228) /* 0x79f3a4 */) = 255 /*0xff*/;
    // 0049635e  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00496360  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00496361  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00496362  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00496363  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_496370: native, nfs3hp::dead::stub_0x496370 */

/* sub_496390: native, nfs3hp::dead::stub_0x496390 */

/* sub_496440: native, nfs3hp::dead::stub_0x496440 */

/* sub_496490: native, nfs3hp::dead::stub_0x496490 */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_4964d0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0;
  x86::reg16 fsw = 0;
    // 004964d0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004964d1  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004964d2  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004964d4  83ec10                 -sub esp, 0x10
    (cpu.esp) -= x86::reg32(x86::sreg32(16 /*0x10*/));
    // 004964d7  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004964d9  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 004964db  0d00000080             -or eax, 0x80000000
    cpu.eax |= x86::reg32(x86::sreg32(2147483648 /*0x80000000*/));
    // 004964e0  3d0000c0ff             +cmp eax, 0xffc00000
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4290772992 /*0xffc00000*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004964e5  750b                   -jne 0x4964f2
    if (!cpu.flags.zf)
    {
        goto L_0x004964f2;
    }
    // 004964e7  c70200000000           -mov dword ptr [edx], 0
    app->getMemory<x86::reg32>(cpu.edx) = 0 /*0x0*/;
    // 004964ed  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004964ef  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004964f0  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004964f1  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004964f2:
    // 004964f2  8b02                   -mov eax, dword ptr [edx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx);
    // 004964f4  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 004964f7  3d0000803f             +cmp eax, 0x3f800000
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1065353216 /*0x3f800000*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004964fc  7e12                   -jle 0x496510
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00496510;
    }
    // 004964fe  83ec08                 +sub esp, 8
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(8 /*0x8*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00496501  d945f8                 +fld dword ptr [ebp - 8]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */));
    // 00496504  dd1c24                 +fstp qword ptr [esp]
    app->getMemory<double>(cpu.esp) = double(fp0);
    // 00496507  e824b70500             -call 0x4f1c30
    cpu.esp -= 4;
    nfs3hp::watcom::floor(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0049650c  d82a                   +fsubr dword ptr [edx]
    fp0 = fp::sub(x86::Float(app->getMemory<float>(cpu.edx)), fp0);
    // 0049650e  eb2c                   -jmp 0x49653c
    goto L_0x0049653c;
L_0x00496510:
    // 00496510  d9ee                   +fldz 
    fp0 = 0.0;
    // 00496512  d85df8                 +fcomp dword ptr [ebp - 8]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 00496515  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 00496517  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 00496518  7624                   -jbe 0x49653e
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0049653e;
    }
    // 0049651a  760a                   -jbe 0x496526
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00496526;
    }
    // 0049651c  d945f8                 +fld dword ptr [ebp - 8]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */));
    // 0049651f  d9e0                   +fchs 
    fp0 = -fp0;
    // 00496521  d95dfc                 +fstp dword ptr [ebp - 4]
    app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = float(fp0);
    // 00496524  eb06                   -jmp 0x49652c
    goto L_0x0049652c;
L_0x00496526:
    // 00496526  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00496529  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
L_0x0049652c:
    // 0049652c  83ec08                 -sub esp, 8
    (cpu.esp) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 0049652f  d945fc                 -fld dword ptr [ebp - 4]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
    // 00496532  dd1c24                 -fstp qword ptr [esp]
    app->getMemory<double>(cpu.esp) = double(fp0);
    // 00496535  e83ec80500             -call 0x4f2d78
    cpu.esp -= 4;
    nfs3hp::watcom::ceil(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0049653a  d802                   -fadd dword ptr [edx]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.edx)));
L_0x0049653c:
    // 0049653c  d91a                   -fstp dword ptr [edx]
    app->getMemory<float>(cpu.edx) = float(fp0);
L_0x0049653e:
    // 0049653e  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00496540  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00496541  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00496542  c3                     -ret 
    cpu.esp += 4;
    return;
}

}
