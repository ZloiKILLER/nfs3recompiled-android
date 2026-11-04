#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{
// Port (tools/apply_native_movie.py): defined in native_movie.cpp.
bool madShowFrame(win32::WinApplication* app, x86::CPU& cpu);
// Port (tools/apply_view_angle.py): defined in nfs3hp_main.cpp.
x86::reg32 viewAngleFor(win32::WinApplication* app, x86::reg32 angle);
// Port (tools/apply_widescreen.py): defined in nfs3hp_main.cpp.
x86::reg32 widescreenHalfAngle(win32::WinApplication* app, x86::reg32 half, x86::reg32 vertical);

/* sub_4d66f0: native, nfs3hp::port::shapes::shapeSize */

/* sub_4d6730: native, nfs3hp::port::buttons::drawTurnedShape */

/* sub_4d6c70: native, nfs3hp::dead::stub_0x4d6c70 */

/* sub_4d70f0: native, nfs3hp::port::shapes::drawShapeQuad */

/* sub_4d7160: native, nfs3hp::port::shapes::drawArtShape */

/* sub_4d7560: native, nfs3hp::frontend::artTilesDraw */

/* sub_4d7990: native, nfs3hp::port::shapes::drawShape */

/* sub_4d79b0: native, nfs3hp::port::obj3d::drawLogo */

/* sub_4d7cc0: native, nfs3hp::frontend::bouncerCornerFade */

/* sub_4d7d90: native, nfs3hp::frontend::bouncerDraw */

/* sub_4d81b0: native, nfs3hp::port::draw2d::drawShadedRectangle */

/* sub_4d8550: native, nfs3hp::port::draw2d::gradientRectangle */

/* sub_4d8570: native, nfs3hp::port::fields::drawShadedTriangle */

/* sub_4d8860: native, nfs3hp::port::draw2d::drawShadedBox */

/* sub_4d88c0: native, nfs3hp::frontend::pickerShadedTriangle */

/* sub_4d88f0: native, nfs3hp::port::buttons::drawColouredRectangle */

/* sub_4d8910: native, nfs3hp::port::draw2d::filledRectangle */

/* sub_4d8930: native, nfs3hp::port::draw2d::frame */

/* sub_4d89f0: native, nfs3hp::port::draw2d::drawPanel */

/* sub_4d8aa0: native, nfs3hp::port::fields::drawFrameAt */

/* sub_4d8ac0: native, nfs3hp::port::draw2d::drawFieldBox */

/* sub_4d8b80: native, nfs3hp::port::draw2d::drawTileRectangle */

/* sub_4d8ed0: native, nfs3hp::port::draw2d::drawLine */

/* sub_4d9120: native, nfs3hp::graphics::takeWindowAndClear */

/* sub_4d9140: native, nfs3hp::graphics::flipAndWaitFrames */

/* sub_4d9180: native, nfs3hp::frontend::effectsBandColourScaled */

/* sub_4d9240: native, nfs3hp::port::draw2d::fontCharacter */

/* sub_4d9320: native, nfs3hp::port::draw2d::accentedCharacter */

/* sub_4d93a0: native, nfs3hp::port::draw2d::buildScreenFont */

/* sub_4d95c0: native, nfs3hp::port::draw2d::drawScreenCharacter */

/* sub_4d9b90: native, nfs3hp::port::draw2d::drawTurnedCharacter */

/* sub_4da120: native, nfs3hp::port::draw2d::drawTurnedText */

/* sub_4da1e0: native, nfs3hp::port::draw2d::drawTitleText */

/* sub_4da4a0: native, nfs3hp::port::draw2d::drawScreenText */

/* sub_4da560: native, nfs3hp::port::fields::drawCursor */

/* sub_4da8c0: native, nfs3hp::port::fields::drawCursorText */

/* sub_4daa10: native, nfs3hp::frontend::menuPointerLoad */

/* sub_4dab10: native, nfs3hp::frontend::menuPointerHotSpot */

/* sub_4dab90: native, nfs3hp::port::draw2d::drawPointer */

/* sub_4daf80: native, nfs3hp::port::draw2d::startScreens */

/* sub_4db000: native, nfs3hp::port::draw2d::drawTexturedQuad */

/* sub_4db380: native, nfs3hp::port::draw2d::drawShadedQuad */

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_4db750(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1;
    // 004db750  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004db751  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004db752  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004db753  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004db755  83ec28                 -sub esp, 0x28
    (cpu.esp) -= x86::reg32(x86::sreg32(40 /*0x28*/));
    // 004db758  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 004db75b  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 004db75d  89df                   -mov edi, ebx
    cpu.edi = cpu.ebx;
    // 004db75f  8a5d10                 -mov bl, byte ptr [ebp + 0x10]
    cpu.bl = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(16) /* 0x10 */);
    // 004db762  837d1402               +cmp dword ptr [ebp + 0x14], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004db766  7502                   -jne 0x4db76a
    if (!cpu.flags.zf)
    {
        goto L_0x004db76a;
    }
    // 004db768  30db                   -xor bl, bl
    cpu.bl ^= x86::reg8(x86::sreg8(cpu.bl));
L_0x004db76a:
    // 004db76a  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 004db76c  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004db76e  6800006041             -push 0x41600000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1096810496 /*0x41600000*/;
    cpu.esp -= 4;
    // 004db773  e8a8b7fbff             -call 0x496f20
    cpu.esp -= 4;
    sub_496f20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004db778  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004db77a  0f85c0000000           -jne 0x4db840
    if (!cpu.flags.zf)
    {
        goto L_0x004db840;
    }
    // 004db780  837df803               +cmp dword ptr [ebp - 8], 3
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
    // 004db784  0f8d97000000           -jge 0x4db821
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004db821;
    }
    // 004db78a  d906                   -fld dword ptr [esi]
    fp0 = x86::Float(app->getMemory<float>(cpu.esi));
    // 004db78c  d807                   -fadd dword ptr [edi]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.edi)));
    // 004db78e  dd050c975400           -fld qword ptr [0x54970c]
    fp1 = x86::Float(app->getMemory<double>(x86::reg32(5543692) /* 0x54970c */));
    // 004db794  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 004db796  d8c9                   -fmul st(1)
    fp1 = fp::mul(fp1, x86::Float(fp0));
    // 004db798  d95dd8                 -fstp dword ptr [ebp - 0x28]
    app->getMemory<float>(cpu.ebp + x86::reg32(-40) /* -0x28 */) = float(fp1);
    // 004db79b  d94604                 -fld dword ptr [esi + 4]
    fp1 = x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(4) /* 0x4 */));
    // 004db79e  d84704                 -fadd dword ptr [edi + 4]
    fp1 = fp::add(fp1, x86::Float(app->getMemory<float>(cpu.edi + x86::reg32(4) /* 0x4 */)));
    // 004db7a1  d8c9                   -fmul st(1)
    fp1 = fp::mul(fp1, x86::Float(fp0));
    // 004db7a3  d95ddc                 -fstp dword ptr [ebp - 0x24]
    app->getMemory<float>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = float(fp1);
    // 004db7a6  d94608                 -fld dword ptr [esi + 8]
    fp1 = x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(8) /* 0x8 */));
    // 004db7a9  d84708                 -fadd dword ptr [edi + 8]
    fp1 = fp::add(fp1, x86::Float(app->getMemory<float>(cpu.edi + x86::reg32(8) /* 0x8 */)));
    // 004db7ac  d8c9                   -fmul st(1)
    fp1 = fp::mul(fp1, x86::Float(fp0));
    // 004db7ae  d95de0                 -fstp dword ptr [ebp - 0x20]
    app->getMemory<float>(cpu.ebp + x86::reg32(-32) /* -0x20 */) = float(fp1);
    // 004db7b1  d9460c                 -fld dword ptr [esi + 0xc]
    fp1 = x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(12) /* 0xc */));
    // 004db7b4  d8470c                 -fadd dword ptr [edi + 0xc]
    fp1 = fp::add(fp1, x86::Float(app->getMemory<float>(cpu.edi + x86::reg32(12) /* 0xc */)));
    // 004db7b7  d8c9                   -fmul st(1)
    fp1 = fp::mul(fp1, x86::Float(fp0));
    // 004db7b9  d95de4                 -fstp dword ptr [ebp - 0x1c]
    app->getMemory<float>(cpu.ebp + x86::reg32(-28) /* -0x1c */) = float(fp1);
    // 004db7bc  d94610                 -fld dword ptr [esi + 0x10]
    fp1 = x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(16) /* 0x10 */));
    // 004db7bf  d84710                 -fadd dword ptr [edi + 0x10]
    fp1 = fp::add(fp1, x86::Float(app->getMemory<float>(cpu.edi + x86::reg32(16) /* 0x10 */)));
    // 004db7c2  d8c9                   -fmul st(1)
    fp1 = fp::mul(fp1, x86::Float(fp0));
    // 004db7c4  d95de8                 -fstp dword ptr [ebp - 0x18]
    app->getMemory<float>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = float(fp1);
    // 004db7c7  d94614                 -fld dword ptr [esi + 0x14]
    fp1 = x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(20) /* 0x14 */));
    // 004db7ca  d84714                 -fadd dword ptr [edi + 0x14]
    fp1 = fp::add(fp1, x86::Float(app->getMemory<float>(cpu.edi + x86::reg32(20) /* 0x14 */)));
    // 004db7cd  dec9                   -fmulp st(1)
    fp0 = fp::mul(fp0, fp1);
    // 004db7cf  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004db7d1  8b5514                 -mov edx, dword ptr [ebp + 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    // 004db7d4  88d8                   -mov al, bl
    cpu.al = cpu.bl;
    // 004db7d6  81e1ff000000           -and ecx, 0xff
    cpu.ecx &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 004db7dc  8945f0                 -mov dword ptr [ebp - 0x10], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.eax;
    // 004db7df  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004db7e1  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004db7e2  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 004db7e4  d95dec                 -fstp dword ptr [ebp - 0x14]
    app->getMemory<float>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = float(fp0);
    // 004db7e7  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 004db7ec  8d5dd8                 -lea ebx, [ebp - 0x28]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-40) /* -0x28 */);
    // 004db7ef  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004db7f2  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004db7f3  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004db7f6  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 004db7f8  40                     -inc eax
    (cpu.eax)++;
    // 004db7f9  8b7514                 -mov esi, dword ptr [ebp + 0x14]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    // 004db7fc  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
    // 004db7ff  e84cffffff             -call 0x4db750
    cpu.esp -= 4;
    sub_4db750(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004db804  8b45f0                 -mov eax, dword ptr [ebp - 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004db807  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004db808  8b4dfc                 -mov ecx, dword ptr [ebp - 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004db80b  8d55d8                 -lea edx, [ebp - 0x28]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-40) /* -0x28 */);
    // 004db80e  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004db80f  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 004db811  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004db814  e837ffffff             -call 0x4db750
    cpu.esp -= 4;
    sub_4db750(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004db819  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004db81b  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004db81c  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004db81d  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004db81e  c20800                 -ret 8
    cpu.esp += 4+8 /*0x8*/;
    return;
L_0x004db821:
    // 004db821  8b5514                 -mov edx, dword ptr [ebp + 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    // 004db824  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004db825  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004db827  88da                   -mov dl, bl
    cpu.dl = cpu.bl;
    // 004db829  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004db82b  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004db82d  88cb                   -mov bl, cl
    cpu.bl = cpu.cl;
    // 004db82f  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 004db831  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 004db833  e878b4feff             -call 0x4c6cb0
    cpu.esp -= 4;
    sub_4c6cb0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004db838  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004db83a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004db83b  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004db83c  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004db83d  c20800                 -ret 8
    cpu.esp += 4+8 /*0x8*/;
    return;
L_0x004db840:
    // 004db840  8b4514                 -mov eax, dword ptr [ebp + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    // 004db843  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004db844  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004db846  88d8                   -mov al, bl
    cpu.al = cpu.bl;
    // 004db848  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004db84a  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 004db84c  88cb                   -mov bl, cl
    cpu.bl = cpu.cl;
    // 004db84e  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004db850  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004db852  e859b4feff             -call 0x4c6cb0
    cpu.esp -= 4;
    sub_4c6cb0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004db857  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004db859  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004db85a  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004db85b  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004db85c  c20800                 -ret 8
    cpu.esp += 4+8 /*0x8*/;
    return;
}

/* align: skip 0x90 */
void Application::sub_4db860(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004db860  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004db861  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004db862  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004db863  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004db865  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 004db867  83e00f                 -and eax, 0xf
    cpu.eax &= x86::reg32(x86::sreg32(15 /*0xf*/));
    // 004db86a  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004db86c  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004db86f  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004db871  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004db874  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004db876  bba07b8b00             -mov ebx, 0x8b7ba0
    cpu.ebx = 9141152 /*0x8b7ba0*/;
    // 004db87b  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 004db87e  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 004db880  01c3                   -add ebx, eax
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004db882  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004db889  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004db88b  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004db88e  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004db890  c744832c00000000       -mov dword ptr [ebx + eax*4 + 0x2c], 0
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(44) /* 0x2c */ + cpu.eax * 4) = 0 /*0x0*/;
    // 004db898  c744833000000000       -mov dword ptr [ebx + eax*4 + 0x30], 0
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(48) /* 0x30 */ + cpu.eax * 4) = 0 /*0x0*/;
    // 004db8a0  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004db8a1  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004db8a2  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004db8a3  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_4db8b0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1;
    // 004db8b0  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004db8b1  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004db8b2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004db8b3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004db8b5  83ec18                 -sub esp, 0x18
    (cpu.esp) -= x86::reg32(x86::sreg32(24 /*0x18*/));
    // 004db8b8  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004db8ba  89cf                   -mov edi, ecx
    cpu.edi = cpu.ecx;
    // 004db8bc  8b4d10                 -mov ecx, dword ptr [ebp + 0x10]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(16) /* 0x10 */);
    // 004db8bf  d903                   -fld dword ptr [ebx]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebx));
    // 004db8c1  d822                   -fsub dword ptr [edx]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(cpu.edx)));
    // 004db8c3  d95de8                 -fstp dword ptr [ebp - 0x18]
    app->getMemory<float>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = float(fp0);
    // 004db8c6  d94304                 -fld dword ptr [ebx + 4]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebx + x86::reg32(4) /* 0x4 */));
    // 004db8c9  d86204                 -fsub dword ptr [edx + 4]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(4) /* 0x4 */)));
    // 004db8cc  8d45e8                 -lea eax, [ebp - 0x18]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004db8cf  d95dec                 -fstp dword ptr [ebp - 0x14]
    app->getMemory<float>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = float(fp0);
    // 004db8d2  d94308                 -fld dword ptr [ebx + 8]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebx + x86::reg32(8) /* 0x8 */));
    // 004db8d5  d86208                 -fsub dword ptr [edx + 8]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(8) /* 0x8 */)));
    // 004db8d8  8d5df4                 -lea ebx, [ebp - 0xc]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004db8db  d95df0                 -fstp dword ptr [ebp - 0x10]
    app->getMemory<float>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = float(fp0);
    // 004db8de  e80dbafbff             -call 0x4972f0
    cpu.esp -= 4;
    nfs3hp::vertices::normalise(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004db8e3  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 004db8e5  8d45e8                 -lea eax, [ebp - 0x18]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004db8e8  ddd8                   -fstp st(0)
    fp0 = x86::Float(fp0);
    // 004db8ea  e871480000             -call 0x4e0160
    cpu.esp -= 4;
    sub_4e0160(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004db8ef  dd0514975400           -fld qword ptr [0x549714]
    fp0 = x86::Float(app->getMemory<double>(x86::reg32(5543700) /* 0x549714 */));
    // 004db8f5  d945f4                 -fld dword ptr [ebp - 0xc]
    fp1 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */));
    // 004db8f8  d8c9                   -fmul st(1)
    fp1 = fp::mul(fp1, x86::Float(fp0));
    // 004db8fa  d82e                   -fsubr dword ptr [esi]
    fp1 = fp::sub(x86::Float(app->getMemory<float>(cpu.esi)), fp1);
    // 004db8fc  d919                   -fstp dword ptr [ecx]
    app->getMemory<float>(cpu.ecx) = float(fp1);
    // 004db8fe  d945f8                 -fld dword ptr [ebp - 8]
    fp1 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */));
    // 004db901  d8c9                   -fmul st(1)
    fp1 = fp::mul(fp1, x86::Float(fp0));
    // 004db903  d86e04                 -fsubr dword ptr [esi + 4]
    fp1 = fp::sub(x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(4) /* 0x4 */)), fp1);
    // 004db906  d95904                 -fstp dword ptr [ecx + 4]
    app->getMemory<float>(cpu.ecx + x86::reg32(4) /* 0x4 */) = float(fp1);
    // 004db909  d945fc                 -fld dword ptr [ebp - 4]
    fp1 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
    // 004db90c  d8c9                   -fmul st(1)
    fp1 = fp::mul(fp1, x86::Float(fp0));
    // 004db90e  d86e08                 -fsubr dword ptr [esi + 8]
    fp1 = fp::sub(x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(8) /* 0x8 */)), fp1);
    // 004db911  d95908                 -fstp dword ptr [ecx + 8]
    app->getMemory<float>(cpu.ecx + x86::reg32(8) /* 0x8 */) = float(fp1);
    // 004db914  d945f4                 -fld dword ptr [ebp - 0xc]
    fp1 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */));
    // 004db917  d8c9                   -fmul st(1)
    fp1 = fp::mul(fp1, x86::Float(fp0));
    // 004db919  d806                   -fadd dword ptr [esi]
    fp1 = fp::add(fp1, x86::Float(app->getMemory<float>(cpu.esi)));
    // 004db91b  d9590c                 -fstp dword ptr [ecx + 0xc]
    app->getMemory<float>(cpu.ecx + x86::reg32(12) /* 0xc */) = float(fp1);
    // 004db91e  d945f8                 -fld dword ptr [ebp - 8]
    fp1 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */));
    // 004db921  d8c9                   -fmul st(1)
    fp1 = fp::mul(fp1, x86::Float(fp0));
    // 004db923  d84604                 -fadd dword ptr [esi + 4]
    fp1 = fp::add(fp1, x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(4) /* 0x4 */)));
    // 004db926  d95910                 -fstp dword ptr [ecx + 0x10]
    app->getMemory<float>(cpu.ecx + x86::reg32(16) /* 0x10 */) = float(fp1);
    // 004db929  d84dfc                 -fmul dword ptr [ebp - 4]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 004db92c  d84608                 -fadd dword ptr [esi + 8]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(8) /* 0x8 */)));
    // 004db92f  d95914                 -fstp dword ptr [ecx + 0x14]
    app->getMemory<float>(cpu.ecx + x86::reg32(20) /* 0x14 */) = float(fp0);
    // 004db932  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004db934  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004db935  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004db936  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004db937  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_4db940(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1;
  x86::reg16 fsw = 0;
    // 004db940  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004db941  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004db942  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004db943  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004db945  83ec40                 -sub esp, 0x40
    (cpu.esp) -= x86::reg32(x86::sreg32(64 /*0x40*/));
    // 004db948  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 004db94a  db03                   -fild dword ptr [ebx]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebx)));
    // 004db94c  83e00f                 -and eax, 0xf
    cpu.eax &= x86::reg32(x86::sreg32(15 /*0xf*/));
    // 004db94f  dd051c975400           -fld qword ptr [0x54971c]
    fp1 = x86::Float(app->getMemory<double>(x86::reg32(5543708) /* 0x54971c */));
    // 004db955  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 004db957  d8c9                   -fmul st(1)
    fp1 = fp::mul(fp1, x86::Float(fp0));
    // 004db959  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004db95b  d95de4                 -fstp dword ptr [ebp - 0x1c]
    app->getMemory<float>(cpu.ebp + x86::reg32(-28) /* -0x1c */) = float(fp1);
    // 004db95e  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004db961  db4304                 -fild dword ptr [ebx + 4]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(4) /* 0x4 */)));
    // 004db964  d8c9                   -fmul st(1)
    fp1 = fp::mul(fp1, x86::Float(fp0));
    // 004db966  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004db968  d95de8                 -fstp dword ptr [ebp - 0x18]
    app->getMemory<float>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = float(fp1);
    // 004db96b  db4308                 -fild dword ptr [ebx + 8]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(8) /* 0x8 */)));
    // 004db96e  d8c9                   -fmul st(1)
    fp1 = fp::mul(fp1, x86::Float(fp0));
    // 004db970  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004db973  d95dec                 -fstp dword ptr [ebp - 0x14]
    app->getMemory<float>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = float(fp1);
    // 004db976  db01                   -fild dword ptr [ecx]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ecx)));
    // 004db978  d8c9                   -fmul st(1)
    fp1 = fp::mul(fp1, x86::Float(fp0));
    // 004db97a  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004db97c  bfa07b8b00             -mov edi, 0x8b7ba0
    cpu.edi = 9141152 /*0x8b7ba0*/;
    // 004db981  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 004db984  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 004db986  01c7                   -add edi, eax
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.eax));
    // 004db988  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004db98f  d95dd8                 -fstp dword ptr [ebp - 0x28]
    app->getMemory<float>(cpu.ebp + x86::reg32(-40) /* -0x28 */) = float(fp1);
    // 004db992  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004db994  db4104                 -fild dword ptr [ecx + 4]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(4) /* 0x4 */)));
    // 004db997  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004db99a  d8c9                   -fmul st(1)
    fp1 = fp::mul(fp1, x86::Float(fp0));
    // 004db99c  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004db99e  d95ddc                 -fstp dword ptr [ebp - 0x24]
    app->getMemory<float>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = float(fp1);
    // 004db9a1  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004db9a4  db4108                 -fild dword ptr [ecx + 8]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(8) /* 0x8 */)));
    // 004db9a7  dec9                   -fmulp st(1)
    fp0 = fp::mul(fp0, fp1);
    // 004db9a9  01c7                   +add edi, eax
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
    // 004db9ab  d9ee                   +fldz 
    fp1 = 0.0;
    // 004db9ad  897dfc                 -mov dword ptr [ebp - 4], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edi;
    // 004db9b0  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 004db9b2  d95de0                 +fstp dword ptr [ebp - 0x20]
    app->getMemory<float>(cpu.ebp + x86::reg32(-32) /* -0x20 */) = float(fp1);
    // 004db9b5  d85d14                 +fcomp dword ptr [ebp + 0x14]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(20) /* 0x14 */)));
    // 004db9b8  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004db9ba  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004db9bb  7607                   -jbe 0x4db9c4
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004db9c4;
    }
    // 004db9bd  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 004db9bf  8955f8                 -mov dword ptr [ebp - 8], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.edx;
    // 004db9c2  eb06                   -jmp 0x4db9ca
    goto L_0x004db9ca;
L_0x004db9c4:
    // 004db9c4  8b4514                 -mov eax, dword ptr [ebp + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    // 004db9c7  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
L_0x004db9ca:
    // 004db9ca  d9e8                   +fld1 
    fp0 = 1.0;
    // 004db9cc  d85df8                 +fcomp dword ptr [ebp - 8]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 004db9cf  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004db9d1  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004db9d2  7608                   -jbe 0x4db9dc
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004db9dc;
    }
    // 004db9d4  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004db9d7  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
    // 004db9da  eb07                   -jmp 0x4db9e3
    goto L_0x004db9e3;
L_0x004db9dc:
    // 004db9dc  c745f40000803f         -mov dword ptr [ebp - 0xc], 0x3f800000
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = 1065353216 /*0x3f800000*/;
L_0x004db9e3:
    // 004db9e3  d945f4                 -fld dword ptr [ebp - 0xc]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */));
    // 004db9e6  dc0d24975400           -fmul qword ptr [0x549724]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5543716) /* 0x549724 */)));
    // 004db9ec  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004db9ef  db1c24                 -fistp dword ptr [esp]
    app->getMemory<x86::reg32>(cpu.esp) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 004db9f2  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004db9f3  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004db9f5  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 004db9f8  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004db9fa  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 004db9fd  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004db9ff  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 004dba02  8b5d10                 -mov ebx, dword ptr [ebp + 0x10]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(16) /* 0x10 */);
    // 004dba05  8945f0                 -mov dword ptr [ebp - 0x10], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.eax;
    // 004dba08  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 004dba0a  7512                   -jne 0x4dba1e
    if (!cpu.flags.zf)
    {
        goto L_0x004dba1e;
    }
    // 004dba0c  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004dba0f  a184367d00             -mov eax, dword ptr [0x7d3684]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8205956) /* 0x7d3684 */);
    // 004dba14  2b02                   -sub eax, dword ptr [edx]
    (cpu.eax) -= x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.edx)));
    // 004dba16  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004dba18  0f8e70010000           -jle 0x4dbb8e
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004dbb8e;
    }
L_0x004dba1e:
    // 004dba1e  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004dba21  a184367d00             -mov eax, dword ptr [0x7d3684]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8205956) /* 0x7d3684 */);
    // 004dba26  8b7a2c                 -mov edi, dword ptr [edx + 0x2c]
    cpu.edi = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(44) /* 0x2c */);
    // 004dba29  8902                   -mov dword ptr [edx], eax
    app->getMemory<x86::reg32>(cpu.edx) = cpu.eax;
    // 004dba2b  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 004dba2d  753e                   -jne 0x4dba6d
    if (!cpu.flags.zf)
    {
        goto L_0x004dba6d;
    }
    // 004dba2f  837d1000               +cmp dword ptr [ebp + 0x10], 0
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
    // 004dba33  0f8555010000           -jne 0x4dbb8e
    if (!cpu.flags.zf)
    {
        goto L_0x004dbb8e;
    }
    // 004dba39  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004dba3b  c7422c01000000         -mov dword ptr [edx + 0x2c], 1
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(44) /* 0x2c */) = 1 /*0x1*/;
    // 004dba42  8d75e4                 -lea esi, [ebp - 0x1c]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 004dba45  8d7804                 -lea edi, [eax + 4]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 004dba48  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004dba49  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004dba4a  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004dba4b  8d75e4                 -lea esi, [ebp - 0x1c]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 004dba4e  8d7810                 -lea edi, [eax + 0x10]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 004dba51  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004dba52  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004dba53  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004dba54  8d75e4                 -lea esi, [ebp - 0x1c]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 004dba57  8d781c                 -lea edi, [eax + 0x1c]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(28) /* 0x1c */);
    // 004dba5a  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004dba5b  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004dba5c  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004dba5d  c7423001000000         -mov dword ptr [edx + 0x30], 1
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(48) /* 0x30 */) = 1 /*0x1*/;
    // 004dba64  c6422800               -mov byte ptr [edx + 0x28], 0
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(40) /* 0x28 */) = 0 /*0x0*/;
    // 004dba68  e921010000             -jmp 0x4dbb8e
    goto L_0x004dbb8e;
L_0x004dba6d:
    // 004dba6d  8d7a04                 -lea edi, [edx + 4]
    cpu.edi = x86::reg32(cpu.edx + x86::reg32(4) /* 0x4 */);
    // 004dba70  8d721c                 -lea esi, [edx + 0x1c]
    cpu.esi = x86::reg32(cpu.edx + x86::reg32(28) /* 0x1c */);
    // 004dba73  837d1000               +cmp dword ptr [ebp + 0x10], 0
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
    // 004dba77  0f8478000000           -je 0x4dbaf5
    if (cpu.flags.zf)
    {
        goto L_0x004dbaf5;
    }
    // 004dba7d  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004dba80  83783000               +cmp dword ptr [eax + 0x30], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(48) /* 0x30 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004dba84  755d                   -jne 0x4dbae3
    if (!cpu.flags.zf)
    {
        goto L_0x004dbae3;
    }
    // 004dba86  8d55e4                 -lea edx, [ebp - 0x1c]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 004dba89  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004dba8b  68cdcc4c3e             -push 0x3e4ccccd
    app->getMemory<x86::reg32>(cpu.esp-4) = 1045220557 /*0x3e4ccccd*/;
    cpu.esp -= 4;
    // 004dba90  e88bb4fbff             -call 0x496f20
    cpu.esp -= 4;
    sub_496f20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dba95  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004dba97  754a                   -jne 0x4dbae3
    if (!cpu.flags.zf)
    {
        goto L_0x004dbae3;
    }
    // 004dba99  8d45c0                 -lea eax, [ebp - 0x40]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-64) /* -0x40 */);
    // 004dba9c  8d4dd8                 -lea ecx, [ebp - 0x28]
    cpu.ecx = x86::reg32(cpu.ebp + x86::reg32(-40) /* -0x28 */);
    // 004dba9f  8d5de4                 -lea ebx, [ebp - 0x1c]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 004dbaa2  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004dbaa3  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 004dbaa5  8d45e4                 -lea eax, [ebp - 0x1c]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 004dbaa8  e803feffff             -call 0x4db8b0
    cpu.esp -= 4;
    sub_4db8b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dbaad  6a02                   -push 2
    app->getMemory<x86::reg32>(cpu.esp-4) = 2 /*0x2*/;
    cpu.esp -= 4;
    // 004dbaaf  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004dbab2  8d5dc0                 -lea ebx, [ebp - 0x40]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-64) /* -0x40 */);
    // 004dbab5  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 004dbab7  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 004dbab9  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 004dbabb  8a4828                 -mov cl, byte ptr [eax + 0x28]
    cpu.cl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(40) /* 0x28 */);
    // 004dbabe  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004dbac0  e88bfcffff             -call 0x4db750
    cpu.esp -= 4;
    sub_4db750(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dbac5  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004dbac8  8d75c0                 -lea esi, [ebp - 0x40]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-64) /* -0x40 */);
    // 004dbacb  8d7804                 -lea edi, [eax + 4]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 004dbace  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004dbacf  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004dbad0  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004dbad1  8d75cc                 -lea esi, [ebp - 0x34]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-52) /* -0x34 */);
    // 004dbad4  8d7810                 -lea edi, [eax + 0x10]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 004dbad7  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004dbad8  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004dbad9  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004dbada  8d75e4                 -lea esi, [ebp - 0x1c]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 004dbadd  8d781c                 -lea edi, [eax + 0x1c]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(28) /* 0x1c */);
    // 004dbae0  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004dbae1  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004dbae2  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
L_0x004dbae3:
    // 004dbae3  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004dbae6  c7402c00000000         -mov dword ptr [eax + 0x2c], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(44) /* 0x2c */) = 0 /*0x0*/;
    // 004dbaed  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004dbaef  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dbaf0  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dbaf1  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dbaf2  c20800                 -ret 8
    cpu.esp += 4+8 /*0x8*/;
    return;
L_0x004dbaf5:
    // 004dbaf5  8d55e4                 -lea edx, [ebp - 0x1c]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 004dbaf8  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004dbafa  680000003f             -push 0x3f000000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1056964608 /*0x3f000000*/;
    cpu.esp -= 4;
    // 004dbaff  e81cb4fbff             -call 0x496f20
    cpu.esp -= 4;
    sub_496f20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dbb04  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004dbb06  0f8582000000           -jne 0x4dbb8e
    if (!cpu.flags.zf)
    {
        goto L_0x004dbb8e;
    }
    // 004dbb0c  8d45c0                 -lea eax, [ebp - 0x40]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-64) /* -0x40 */);
    // 004dbb0f  8d4dd8                 -lea ecx, [ebp - 0x28]
    cpu.ecx = x86::reg32(cpu.ebp + x86::reg32(-40) /* -0x28 */);
    // 004dbb12  8d5de4                 -lea ebx, [ebp - 0x1c]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 004dbb15  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004dbb16  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 004dbb18  8d45e4                 -lea eax, [ebp - 0x1c]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 004dbb1b  e890fdffff             -call 0x4db8b0
    cpu.esp -= 4;
    sub_4db8b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dbb20  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004dbb23  83783000               +cmp dword ptr [eax + 0x30], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(48) /* 0x30 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004dbb27  741a                   -je 0x4dbb43
    if (cpu.flags.zf)
    {
        goto L_0x004dbb43;
    }
    // 004dbb29  8d4dd8                 -lea ecx, [ebp - 0x28]
    cpu.ecx = x86::reg32(cpu.ebp + x86::reg32(-40) /* -0x28 */);
    // 004dbb2c  8d5de4                 -lea ebx, [ebp - 0x1c]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 004dbb2f  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004dbb30  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 004dbb32  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004dbb34  e877fdffff             -call 0x4db8b0
    cpu.esp -= 4;
    sub_4db8b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dbb39  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004dbb3c  c7403000000000         -mov dword ptr [eax + 0x30], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(48) /* 0x30 */) = 0 /*0x0*/;
L_0x004dbb43:
    // 004dbb43  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004dbb45  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 004dbb47  8a45f0                 -mov al, byte ptr [ebp - 0x10]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004dbb4a  8d5dc0                 -lea ebx, [ebp - 0x40]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-64) /* -0x40 */);
    // 004dbb4d  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004dbb4e  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004dbb51  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 004dbb53  8d5004                 -lea edx, [eax + 4]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 004dbb56  8a4828                 -mov cl, byte ptr [eax + 0x28]
    cpu.cl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(40) /* 0x28 */);
    // 004dbb59  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004dbb5b  e8f0fbffff             -call 0x4db750
    cpu.esp -= 4;
    sub_4db750(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dbb60  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004dbb63  8d75c0                 -lea esi, [ebp - 0x40]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-64) /* -0x40 */);
    // 004dbb66  8d7804                 -lea edi, [eax + 4]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 004dbb69  c7402c01000000         -mov dword ptr [eax + 0x2c], 1
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(44) /* 0x2c */) = 1 /*0x1*/;
    // 004dbb70  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004dbb71  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004dbb72  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004dbb73  8d75cc                 -lea esi, [ebp - 0x34]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-52) /* -0x34 */);
    // 004dbb76  8d7810                 -lea edi, [eax + 0x10]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 004dbb79  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004dbb7a  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004dbb7b  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004dbb7c  8d75e4                 -lea esi, [ebp - 0x1c]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 004dbb7f  8d781c                 -lea edi, [eax + 0x1c]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(28) /* 0x1c */);
    // 004dbb82  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004dbb85  8a45f0                 -mov al, byte ptr [ebp - 0x10]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004dbb88  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004dbb89  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004dbb8a  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004dbb8b  884228                 -mov byte ptr [edx + 0x28], al
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(40) /* 0x28 */) = cpu.al;
L_0x004dbb8e:
    // 004dbb8e  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004dbb90  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dbb91  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dbb92  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dbb93  c20800                 -ret 8
    cpu.esp += 4+8 /*0x8*/;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x54 0x22 0x00 */
void Application::sub_4dbba0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004dbba0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004dbba1  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004dbba2  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004dbba4  ba000d0000             -mov edx, 0xd00
    cpu.edx = 3328 /*0xd00*/;
    // 004dbba9  b8a07b8b00             -mov eax, 0x8b7ba0
    cpu.eax = 9141152 /*0x8b7ba0*/;
    // 004dbbae  e8594b0000             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dbbb3  e81878fcff             -call 0x4a33d0
    cpu.esp -= 4;
    sub_4a33d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dbbb8  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dbbb9  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dbbba  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_4dbbc0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004dbbc0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004dbbc1  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004dbbc2  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004dbbc4  ba000d0000             -mov edx, 0xd00
    cpu.edx = 3328 /*0xd00*/;
    // 004dbbc9  b8a07b8b00             -mov eax, 0x8b7ba0
    cpu.eax = 9141152 /*0x8b7ba0*/;
    // 004dbbce  e8394b0000             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dbbd3  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dbbd4  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dbbd5  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x54 0x22 0x00 */
void Application::sub_4dbbe0(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004dbbe0  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004dbbe1  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004dbbe3  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dbbe4  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_4dbbf0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004dbbf0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004dbbf1  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004dbbf2  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004dbbf4  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004dbbf6  e8c51ff4ff             -call 0x41dbc0
    cpu.esp -= 4;
    nfs3hp::load::loadTrackTextures(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dbbfb  e8d077fcff             -call 0x4a33d0
    cpu.esp -= 4;
    sub_4a33d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dbc00  833d8ce8550000         +cmp dword ptr [0x55e88c], 0
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
    // 004dbc07  7408                   -je 0x4dbc11
    if (cpu.flags.zf)
    {
        goto L_0x004dbc11;
    }
    // 004dbc09  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004dbc0e  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dbc0f  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dbc10  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004dbc11:
    // 004dbc11  6800002042             -push 0x42200000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1109393408 /*0x42200000*/;
    cpu.esp -= 4;
    // 004dbc16  e81596fbff             -call 0x495230
    cpu.esp -= 4;
    nfs3hp::graphics::drawLoadingProgress(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dbc1b  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004dbc1d  e81ec7f9ff             -call 0x478340
    cpu.esp -= 4;
    nfs3hp::load::loadHorizon(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dbc22  e8a977fcff             -call 0x4a33d0
    cpu.esp -= 4;
    sub_4a33d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dbc27  833d8ce8550000         +cmp dword ptr [0x55e88c], 0
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
    // 004dbc2e  7408                   -je 0x4dbc38
    if (cpu.flags.zf)
    {
        goto L_0x004dbc38;
    }
    // 004dbc30  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004dbc35  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dbc36  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dbc37  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004dbc38:
    // 004dbc38  6800002c42             -push 0x422c0000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1110179840 /*0x422c0000*/;
    cpu.esp -= 4;
    // 004dbc3d  e8ee95fbff             -call 0x495230
    cpu.esp -= 4;
    nfs3hp::graphics::drawLoadingProgress(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dbc42  e8191df4ff             -call 0x41d960
    cpu.esp -= 4;
    nfs3hp::render::raceViews(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dbc47  e88477fcff             -call 0x4a33d0
    cpu.esp -= 4;
    sub_4a33d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dbc4c  833d8ce8550000         +cmp dword ptr [0x55e88c], 0
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
    // 004dbc53  7408                   -je 0x4dbc5d
    if (cpu.flags.zf)
    {
        goto L_0x004dbc5d;
    }
    // 004dbc55  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004dbc5a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dbc5b  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dbc5c  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004dbc5d:
    // 004dbc5d  6800006842             -push 0x42680000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1114112000 /*0x42680000*/;
    cpu.esp -= 4;
    // 004dbc62  e8c995fbff             -call 0x495230
    cpu.esp -= 4;
    nfs3hp::graphics::drawLoadingProgress(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dbc67  e8040cfeff             -call 0x4bc870
    cpu.esp -= 4;
    sub_4bc870(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dbc6c  e85f77fcff             -call 0x4a33d0
    cpu.esp -= 4;
    sub_4a33d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dbc71  833d8ce8550000         +cmp dword ptr [0x55e88c], 0
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
    // 004dbc78  7408                   -je 0x4dbc82
    if (cpu.flags.zf)
    {
        goto L_0x004dbc82;
    }
    // 004dbc7a  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004dbc7f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dbc80  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dbc81  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004dbc82:
    // 004dbc82  6800007442             -push 0x42740000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1114898432 /*0x42740000*/;
    cpu.esp -= 4;
    // 004dbc87  e8a495fbff             -call 0x495230
    cpu.esp -= 4;
    nfs3hp::graphics::drawLoadingProgress(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dbc8c  e82fc6f9ff             -call 0x4782c0
    cpu.esp -= 4;
    sub_4782c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dbc91  e83a77fcff             -call 0x4a33d0
    cpu.esp -= 4;
    sub_4a33d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dbc96  833d8ce8550000         +cmp dword ptr [0x55e88c], 0
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
    // 004dbc9d  7408                   -je 0x4dbca7
    if (cpu.flags.zf)
    {
        goto L_0x004dbca7;
    }
    // 004dbc9f  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004dbca4  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dbca5  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dbca6  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004dbca7:
    // 004dbca7  6800008042             -push 0x42800000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1115684864 /*0x42800000*/;
    cpu.esp -= 4;
    // 004dbcac  e87f95fbff             -call 0x495230
    cpu.esp -= 4;
    nfs3hp::graphics::drawLoadingProgress(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dbcb1  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004dbcb3  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dbcb4  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dbcb5  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_4dbcc0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004dbcc0  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004dbcc1  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004dbcc3  a140bc6f00             -mov eax, dword ptr [0x6fbc40]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7322688) /* 0x6fbc40 */);
    // 004dbcc8  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004dbcca  760c                   -jbe 0x4dbcd8
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004dbcd8;
    }
    // 004dbccc  83f801                 +cmp eax, 1
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
    // 004dbccf  7507                   -jne 0x4dbcd8
    if (!cpu.flags.zf)
    {
        goto L_0x004dbcd8;
    }
    // 004dbcd1  b84b000000             -mov eax, 0x4b
    cpu.eax = 75 /*0x4b*/;
    // 004dbcd6  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dbcd7  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004dbcd8:
    // 004dbcd8  b85a000000             -mov eax, 0x5a
    cpu.eax = viewAngleFor(app, 90 /*0x5a*/); /* port: View Angle's third choice */
    // 004dbcdd  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dbcde  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x90 */
void Application::sub_4dbce0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1;
    // 004dbce0  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004dbce1  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004dbce2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004dbce3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004dbce5  83ec18                 -sub esp, 0x18
    (cpu.esp) -= x86::reg32(x86::sreg32(24 /*0x18*/));
    // 004dbce8  8b7510                 -mov esi, dword ptr [ebp + 0x10]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(16) /* 0x10 */);
    // 004dbceb  8945ec                 -mov dword ptr [ebp - 0x14], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.eax;
    // 004dbcee  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 004dbcf0  895df4                 -mov dword ptr [ebp - 0xc], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.ebx;
    // 004dbcf3  894df0                 -mov dword ptr [ebp - 0x10], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.ecx;
    // 004dbcf6  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 004dbcf8  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004dbcfa  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004dbcfd  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004dbcff  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 004dbd01  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004dbd03  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004dbd06  29f0                   -sub eax, esi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.esi));
    // 004dbd08  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004dbd0b  8d1406                 -lea edx, [esi + eax]
    cpu.edx = x86::reg32(cpu.esi + cpu.eax * 1);
    // 004dbd0e  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004dbd10  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004dbd13  c1e204                 +shl edx, 4
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
    // 004dbd16  1bc2                   -sbb eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 004dbd18  c1f804                 -sar eax, 4
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (4 /*0x4*/ % 32));
    // 004dbd1b  8945e8                 -mov dword ptr [ebp - 0x18], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = cpu.eax;
    cpu.esi = widescreenHalfAngle(app, cpu.esi, cpu.eax); /* port: the horizontal angle the view's own shape asks for */
    // 004dbd1e  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 004dbd20  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004dbd23  897dfc                 -mov dword ptr [ebp - 4], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edi;
    // 004dbd26  e855150200             -call 0x4fd280
    cpu.esp -= 4;
    sub_4fd280(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dbd2b  8b55e8                 -mov edx, dword ptr [ebp - 0x18]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004dbd2e  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004dbd30  8975f8                 -mov dword ptr [ebp - 8], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.esi;
    // 004dbd33  e8b8160200             -call 0x4fd3f0
    cpu.esp -= 4;
    sub_4fd3f0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dbd38  8b5514                 -mov edx, dword ptr [ebp + 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    // 004dbd3b  8b45e8                 -mov eax, dword ptr [ebp - 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004dbd3e  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004dbd3f  db45f8                 -fild dword ptr [ebp - 8]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 004dbd42  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 004dbd45  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004dbd48  db45f8                 -fild dword ptr [ebp - 8]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 004dbd4b  d91c24                 -fstp dword ptr [esp]
    app->getMemory<float>(cpu.esp) = float(fp1);
    // 004dbd4e  8b45f0                 -mov eax, dword ptr [ebp - 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004dbd51  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004dbd54  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 004dbd57  d91c24                 -fstp dword ptr [esp]
    app->getMemory<float>(cpu.esp) = float(fp0);
    // 004dbd5a  db45f8                 -fild dword ptr [ebp - 8]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 004dbd5d  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004dbd60  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004dbd63  d91c24                 -fstp dword ptr [esp]
    app->getMemory<float>(cpu.esp) = float(fp0);
    // 004dbd66  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 004dbd69  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004dbd6c  db45f8                 -fild dword ptr [ebp - 8]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 004dbd6f  d91c24                 -fstp dword ptr [esp]
    app->getMemory<float>(cpu.esp) = float(fp0);
    // 004dbd72  db45fc                 -fild dword ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 004dbd75  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004dbd78  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004dbd7b  d91c24                 -fstp dword ptr [esp]
    app->getMemory<float>(cpu.esp) = float(fp0);
    // 004dbd7e  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004dbd81  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004dbd84  db45fc                 -fild dword ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 004dbd87  d91c24                 -fstp dword ptr [esp]
    app->getMemory<float>(cpu.esp) = float(fp0);
    // 004dbd8a  e8d134feff             -call 0x4bf260
    cpu.esp -= 4;
    sub_4bf260(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dbd8f  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004dbd91  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dbd92  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dbd93  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dbd94  c20800                 -ret 8
    cpu.esp += 4+8 /*0x8*/;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_4dbda0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004dbda0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004dbda1  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004dbda2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004dbda3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004dbda5  833db0d36f0001         +cmp dword ptr [0x6fd3b0], 1
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
    // 004dbdac  7518                   -jne 0x4dbdc6
    if (!cpu.flags.zf)
    {
        goto L_0x004dbdc6;
    }
    // 004dbdae  8b4004                 -mov eax, dword ptr [eax + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 004dbdb1  ba78000000             -mov edx, 0x78
    cpu.edx = 120 /*0x78*/;
    // 004dbdb6  c1e005                 -shl eax, 5
    cpu.eax <<= 5 /*0x5*/ % 32;
    // 004dbdb9  8b1dc4d46f00           -mov ebx, dword ptr [0x6fd4c4]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7328964) /* 0x6fd4c4 */);
    // 004dbdbf  0580da7c00             +add eax, 0x7cda80
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(8182400 /*0x7cda80*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004dbdc4  eb10                   -jmp 0x4dbdd6
    goto L_0x004dbdd6;
L_0x004dbdc6:
    // 004dbdc6  ba78000000             -mov edx, 0x78
    cpu.edx = 120 /*0x78*/;
    // 004dbdcb  b888db7c00             -mov eax, 0x7cdb88
    cpu.eax = 8182664 /*0x7cdb88*/;
    // 004dbdd0  8b1dc4d46f00           -mov ebx, dword ptr [0x6fd4c4]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7328964) /* 0x6fd4c4 */);
L_0x004dbdd6:
    // 004dbdd6  e8452ffeff             -call 0x4bed20
    cpu.esp -= 4;
    sub_4bed20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dbddb  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dbddc  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dbddd  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dbdde  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4dbde0: native, nfs3hp::frontend::simSetView */

/* sub_4dbef0: native, nfs3hp::vertices::objectVerticesList */

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_4dc0e0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004dc0e0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004dc0e1  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004dc0e2  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004dc0e3  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004dc0e4  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004dc0e6  83ec0c                 -sub esp, 0xc
    (cpu.esp) -= x86::reg32(x86::sreg32(12 /*0xc*/));
    // 004dc0e9  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 004dc0eb  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 004dc0ed  895dfc                 -mov dword ptr [ebp - 4], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ebx;
    // 004dc0f0  bb10000000             -mov ebx, 0x10
    cpu.ebx = 16 /*0x10*/;
    // 004dc0f5  ba0c000000             -mov edx, 0xc
    cpu.edx = 12 /*0xc*/;
    // 004dc0fa  b834975400             -mov eax, 0x549734
    cpu.eax = 5543732 /*0x549734*/;
    // 004dc0ff  e81c550000             -call 0x4e1620
    cpu.esp -= 4;
    nfs3hp::ea::reservememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dc104  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004dc106  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004dc108  751a                   -jne 0x4dc124
    if (!cpu.flags.zf)
    {
        goto L_0x004dc124;
    }
    // 004dc10a  837dfc00               +cmp dword ptr [ebp - 4], 0
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
    // 004dc10e  740d                   -je 0x4dc11d
    if (cpu.flags.zf)
    {
        goto L_0x004dc11d;
    }
    // 004dc110  6838975400             -push 0x549738
    app->getMemory<x86::reg32>(cpu.esp-4) = 5543736 /*0x549738*/;
    cpu.esp -= 4;
    // 004dc115  e8f64ef2ff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dc11a  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
L_0x004dc11d:
    // 004dc11d  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004dc11f  e981000000             -jmp 0x4dc1a5
    goto L_0x004dc1a5;
L_0x004dc124:
    // 004dc124  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 004dc126  7504                   -jne 0x4dc12c
    if (!cpu.flags.zf)
    {
        goto L_0x004dc12c;
    }
    // 004dc128  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004dc12a  7408                   -je 0x4dc134
    if (cpu.flags.zf)
    {
        goto L_0x004dc134;
    }
L_0x004dc12c:
    // 004dc12c  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 004dc12e  742b                   -je 0x4dc15b
    if (cpu.flags.zf)
    {
        goto L_0x004dc15b;
    }
    // 004dc130  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004dc132  7427                   -je 0x4dc15b
    if (cpu.flags.zf)
    {
        goto L_0x004dc15b;
    }
L_0x004dc134:
    // 004dc134  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 004dc136  7407                   -je 0x4dc13f
    if (cpu.flags.zf)
    {
        goto L_0x004dc13f;
    }
    // 004dc138  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004dc13a  e851570000             -call 0x4e1890
    cpu.esp -= 4;
    nfs3hp::ea::purgememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004dc13f:
    // 004dc13f  837dfc00               +cmp dword ptr [ebp - 4], 0
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
    // 004dc143  740d                   -je 0x4dc152
    if (cpu.flags.zf)
    {
        goto L_0x004dc152;
    }
    // 004dc145  6858975400             -push 0x549758
    app->getMemory<x86::reg32>(cpu.esp-4) = 5543768 /*0x549758*/;
    cpu.esp -= 4;
    // 004dc14a  e8c14ef2ff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dc14f  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
L_0x004dc152:
    // 004dc152  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004dc154  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004dc156  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dc157  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dc158  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dc159  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dc15a  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004dc15b:
    // 004dc15b  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004dc15d  7412                   -je 0x4dc171
    if (cpu.flags.zf)
    {
        goto L_0x004dc171;
    }
    // 004dc15f  c7460400000000         -mov dword ptr [esi + 4], 0
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */) = 0 /*0x0*/;
    // 004dc166  c70601000000           -mov dword ptr [esi], 1
    app->getMemory<x86::reg32>(cpu.esi) = 1 /*0x1*/;
    // 004dc16c  894e08                 -mov dword ptr [esi + 8], ecx
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(8) /* 0x8 */) = cpu.ecx;
    // 004dc16f  eb32                   -jmp 0x4dc1a3
    goto L_0x004dc1a3;
L_0x004dc171:
    // 004dc171  8d5df4                 -lea ebx, [ebp - 0xc]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004dc174  894df4                 -mov dword ptr [ebp - 0xc], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.ecx;
    // 004dc177  894df8                 -mov dword ptr [ebp - 8], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.ecx;
    // 004dc17a  8d5604                 -lea edx, [esi + 4]
    cpu.edx = x86::reg32(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 004dc17d  890e                   -mov dword ptr [esi], ecx
    app->getMemory<x86::reg32>(cpu.esi) = cpu.ecx;
    // 004dc17f  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 004dc181  894e08                 -mov dword ptr [esi + 8], ecx
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(8) /* 0x8 */) = cpu.ecx;
    // 004dc184  8d4df8                 -lea ecx, [ebp - 8]
    cpu.ecx = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004dc187  e844670000             -call 0x4e28d0
    cpu.esp -= 4;
    nfs3hp::ea::openHandle(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dc18c  8b5604                 -mov edx, dword ptr [esi + 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 004dc18f  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004dc191  7510                   -jne 0x4dc1a3
    if (!cpu.flags.zf)
    {
        goto L_0x004dc1a3;
    }
    // 004dc193  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004dc195  e8f6560000             -call 0x4e1890
    cpu.esp -= 4;
    nfs3hp::ea::purgememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dc19a  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004dc19c  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004dc19e  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dc19f  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dc1a0  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dc1a1  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dc1a2  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004dc1a3:
    // 004dc1a3  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
L_0x004dc1a5:
    // 004dc1a5  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004dc1a7  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dc1a8  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dc1a9  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dc1aa  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dc1ab  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_4dc1b0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004dc1b0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004dc1b1  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004dc1b2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004dc1b3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004dc1b5  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004dc1b7  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004dc1b9  741a                   -je 0x4dc1d5
    if (cpu.flags.zf)
    {
        goto L_0x004dc1d5;
    }
    // 004dc1bb  833800                 +cmp dword ptr [eax], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004dc1be  750e                   -jne 0x4dc1ce
    if (!cpu.flags.zf)
    {
        goto L_0x004dc1ce;
    }
    // 004dc1c0  8b5804                 -mov ebx, dword ptr [eax + 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 004dc1c3  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 004dc1c5  7407                   -je 0x4dc1ce
    if (cpu.flags.zf)
    {
        goto L_0x004dc1ce;
    }
    // 004dc1c7  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 004dc1c9  e8d26b0000             -call 0x4e2da0
    cpu.esp -= 4;
    nfs3hp::ea::closeHandle(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004dc1ce:
    // 004dc1ce  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004dc1d0  e8bb560000             -call 0x4e1890
    cpu.esp -= 4;
    nfs3hp::ea::purgememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004dc1d5:
    // 004dc1d5  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dc1d6  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dc1d7  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dc1d8  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_4dc1e0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004dc1e0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004dc1e1  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004dc1e2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004dc1e3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004dc1e5  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004dc1e8  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004dc1ea  833800                 +cmp dword ptr [eax], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004dc1ed  751b                   -jne 0x4dc20a
    if (!cpu.flags.zf)
    {
        goto L_0x004dc20a;
    }
    // 004dc1ef  bb04000000             -mov ebx, 4
    cpu.ebx = 4 /*0x4*/;
    // 004dc1f4  8d55fc                 -lea edx, [ebp - 4]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004dc1f7  8b4004                 -mov eax, dword ptr [eax + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 004dc1fa  e8e16b0000             -call 0x4e2de0
    cpu.esp -= 4;
    nfs3hp::ea::readHandle(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dc1ff  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004dc201  7513                   -jne 0x4dc216
    if (!cpu.flags.zf)
    {
        goto L_0x004dc216;
    }
    // 004dc203  e875680100             -call 0x4f2a7d
    cpu.esp -= 4;
    sub_4f2a7d(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dc208  eb0c                   -jmp 0x4dc216
    goto L_0x004dc216;
L_0x004dc20a:
    // 004dc20a  8b4008                 -mov eax, dword ptr [eax + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 004dc20d  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 004dc20f  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004dc212  83420804               -add dword ptr [edx + 8], 4
    (app->getMemory<x86::reg32>(cpu.edx + x86::reg32(8) /* 0x8 */)) += x86::reg32(x86::sreg32(4 /*0x4*/));
L_0x004dc216:
    // 004dc216  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004dc219  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004dc21b  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dc21c  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dc21d  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dc21e  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4dc220: native, nfs3hp::dead::stub_0x4dc220 */

/* sub_4dc260: native, nfs3hp::dead::stub_0x4dc260 */

/* sub_4dc2a0: native, nfs3hp::dead::stub_0x4dc2a0 */

/* sub_4dc2e0: native, nfs3hp::dead::stub_0x4dc2e0 */

/* sub_4dc300: native, nfs3hp::dead::stub_0x4dc300 */

/* sub_4dc350: native, nfs3hp::dead::stub_0x4dc350 */

/* sub_4dc3a0: native, nfs3hp::load::setSpecUsed */

/* sub_4dc3f0: native, nfs3hp::load::specUsed */

/* sub_4dc440: native, nfs3hp::load::specCarName */

/* sub_4dc490: native, nfs3hp::load::findSpec */

/* sub_4dc500: native, nfs3hp::load::freeSpecSlot */

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_4dc550(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004dc550  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004dc551  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004dc552  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004dc553  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004dc555  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004dc557  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 004dc559  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004dc55b  7507                   -jne 0x4dc564
    if (!cpu.flags.zf)
    {
        goto L_0x004dc564;
    }
    // 004dc55d  b900928b00             -mov ecx, 0x8b9200
    cpu.ecx = 9146880 /*0x8b9200*/;
    // 004dc562  eb0a                   -jmp 0x4dc56e
    goto L_0x004dc56e;
L_0x004dc564:
    // 004dc564  83f801                 +cmp eax, 1
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
    // 004dc567  7505                   -jne 0x4dc56e
    if (!cpu.flags.zf)
    {
        goto L_0x004dc56e;
    }
    // 004dc569  b9a0888b00             -mov ecx, 0x8b88a0
    cpu.ecx = 9144480 /*0x8b88a0*/;
L_0x004dc56e:
    // 004dc56e  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004dc570  741d                   -je 0x4dc58f
    if (cpu.flags.zf)
    {
        goto L_0x004dc58f;
    }
    // 004dc572  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004dc574  7419                   -je 0x4dc58f
    if (cpu.flags.zf)
    {
        goto L_0x004dc58f;
    }
    // 004dc576  e815ffffff             -call 0x4dc490
    cpu.esp -= 4;
    nfs3hp::load::findSpec(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dc57b  83f8ff                 +cmp eax, -1
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
    // 004dc57e  740f                   -je 0x4dc58f
    if (cpu.flags.zf)
    {
        goto L_0x004dc58f;
    }
    // 004dc580  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004dc582  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004dc585  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004dc587  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 004dc58a  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004dc58c  8d5814                 -lea ebx, [eax + 0x14]
    cpu.ebx = x86::reg32(cpu.eax + x86::reg32(20) /* 0x14 */);
L_0x004dc58f:
    // 004dc58f  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 004dc591  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dc592  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dc593  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dc594  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4dc5a0: native, nfs3hp::load::setSpec */

/* sub_4dc670: native, nfs3hp::load::defaultSpecs */

/* sub_4dc710: native, nfs3hp::load::refreshSpecs */

/* sub_4dc800: native, nfs3hp::load::specPath */

/* sub_4dc8b0: native, nfs3hp::load::readSpecs */

/* sub_4dc940: native, nfs3hp::load::loadPlayerSpecs */

/* sub_4dca50: native, nfs3hp::load::savePlayerSpecs */

/* sub_4dcaf0: native, nfs3hp::frontend::tuningAdvancedDiffers */

/* sub_4dcb60: native, nfs3hp::frontend::tuningCopyAdvanced */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_4dcbc0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004dcbc0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004dcbc1  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004dcbc2  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004dcbc3  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004dcbc4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004dcbc5  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004dcbc7  a1bcd26f00             -mov eax, dword ptr [0x6fd2bc]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328444) /* 0x6fd2bc */);
    // 004dcbcc  e87fcef5ff             -call 0x439a50
    cpu.esp -= 4;
    nfs3hp::load::carRecord(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dcbd1  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004dcbd3  746b                   -je 0x4dcc40
    if (cpu.flags.zf)
    {
        goto L_0x004dcc40;
    }
    // 004dcbd5  a1bcd26f00             -mov eax, dword ptr [0x6fd2bc]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328444) /* 0x6fd2bc */);
    // 004dcbda  e841c9f5ff             -call 0x439520
    cpu.esp -= 4;
    nfs3hp::load::carId(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dcbdf  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004dcbe1  743c                   -je 0x4dcc1f
    if (cpu.flags.zf)
    {
        goto L_0x004dcc1f;
    }
    // 004dcbe3  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004dcbe5  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004dcbe7  e864f9ffff             -call 0x4dc550
    cpu.esp -= 4;
    sub_4dc550(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dcbec  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004dcbee  740e                   -je 0x4dcbfe
    if (cpu.flags.zf)
    {
        goto L_0x004dcbfe;
    }
    // 004dcbf0  b91c000000             -mov ecx, 0x1c
    cpu.ecx = 28 /*0x1c*/;
    // 004dcbf5  bf0cd36f00             -mov edi, 0x6fd30c
    cpu.edi = 7328524 /*0x6fd30c*/;
    // 004dcbfa  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004dcbfc  eb0f                   -jmp 0x4dcc0d
    goto L_0x004dcc0d;
L_0x004dcbfe:
    // 004dcbfe  b91c000000             -mov ecx, 0x1c
    cpu.ecx = 28 /*0x1c*/;
    // 004dcc03  be24d05500             -mov esi, 0x55d024
    cpu.esi = 5623844 /*0x55d024*/;
    // 004dcc08  bf0cd36f00             -mov edi, 0x6fd30c
    cpu.edi = 7328524 /*0x6fd30c*/;
L_0x004dcc0d:
    // 004dcc0d  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004dcc0e  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004dcc10  c1e902                 -shr ecx, 2
    cpu.ecx >>= 2 /*0x2*/ % 32;
    // 004dcc13  f2a5                   -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004dcc15  8ac8                   -mov cl, al
    cpu.cl = cpu.al;
    // 004dcc17  80e103                 +and cl, 3
    cpu.clear_co();
    cpu.set_szp((cpu.cl &= x86::reg8(x86::sreg8(3 /*0x3*/))));
    // 004dcc1a  f2a4                   -repne movsb byte ptr es:[edi], byte ptr [esi]
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
    // 004dcc1c  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dcc1d  eb40                   -jmp 0x4dcc5f
    goto L_0x004dcc5f;
L_0x004dcc1f:
    // 004dcc1f  b91c000000             -mov ecx, 0x1c
    cpu.ecx = 28 /*0x1c*/;
    // 004dcc24  be24d05500             -mov esi, 0x55d024
    cpu.esi = 5623844 /*0x55d024*/;
    // 004dcc29  bf0cd36f00             -mov edi, 0x6fd30c
    cpu.edi = 7328524 /*0x6fd30c*/;
    // 004dcc2e  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004dcc2f  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004dcc31  c1e902                 -shr ecx, 2
    cpu.ecx >>= 2 /*0x2*/ % 32;
    // 004dcc34  f2a5                   -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004dcc36  8ac8                   -mov cl, al
    cpu.cl = cpu.al;
    // 004dcc38  80e103                 +and cl, 3
    cpu.clear_co();
    cpu.set_szp((cpu.cl &= x86::reg8(x86::sreg8(3 /*0x3*/))));
    // 004dcc3b  f2a4                   -repne movsb byte ptr es:[edi], byte ptr [esi]
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
    // 004dcc3d  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dcc3e  eb1f                   -jmp 0x4dcc5f
    goto L_0x004dcc5f;
L_0x004dcc40:
    // 004dcc40  b91c000000             -mov ecx, 0x1c
    cpu.ecx = 28 /*0x1c*/;
    // 004dcc45  be24d05500             -mov esi, 0x55d024
    cpu.esi = 5623844 /*0x55d024*/;
    // 004dcc4a  bf0cd36f00             -mov edi, 0x6fd30c
    cpu.edi = 7328524 /*0x6fd30c*/;
    // 004dcc4f  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004dcc50  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004dcc52  c1e902                 -shr ecx, 2
    cpu.ecx >>= 2 /*0x2*/ % 32;
    // 004dcc55  f2a5                   -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004dcc57  8ac8                   -mov cl, al
    cpu.cl = cpu.al;
    // 004dcc59  80e103                 +and cl, 3
    cpu.clear_co();
    cpu.set_szp((cpu.cl &= x86::reg8(x86::sreg8(3 /*0x3*/))));
    // 004dcc5c  f2a4                   -repne movsb byte ptr es:[edi], byte ptr [esi]
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
    // 004dcc5e  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
L_0x004dcc5f:
    // 004dcc5f  a128d36f00             -mov eax, dword ptr [0x6fd328]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328552) /* 0x6fd328 */);
    // 004dcc64  e8e7cdf5ff             -call 0x439a50
    cpu.esp -= 4;
    nfs3hp::load::carRecord(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dcc69  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004dcc6b  0f8472000000           -je 0x4dcce3
    if (cpu.flags.zf)
    {
        goto L_0x004dcce3;
    }
    // 004dcc71  a128d36f00             -mov eax, dword ptr [0x6fd328]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328552) /* 0x6fd328 */);
    // 004dcc76  e8a5c8f5ff             -call 0x439520
    cpu.esp -= 4;
    nfs3hp::load::carId(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dcc7b  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004dcc7d  743f                   -je 0x4dccbe
    if (cpu.flags.zf)
    {
        goto L_0x004dccbe;
    }
    // 004dcc7f  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004dcc81  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004dcc86  e8c5f8ffff             -call 0x4dc550
    cpu.esp -= 4;
    sub_4dc550(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dcc8b  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004dcc8d  740e                   -je 0x4dcc9d
    if (cpu.flags.zf)
    {
        goto L_0x004dcc9d;
    }
    // 004dcc8f  b91c000000             -mov ecx, 0x1c
    cpu.ecx = 28 /*0x1c*/;
    // 004dcc94  bf78d36f00             -mov edi, 0x6fd378
    cpu.edi = 7328632 /*0x6fd378*/;
    // 004dcc99  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004dcc9b  eb0f                   -jmp 0x4dccac
    goto L_0x004dccac;
L_0x004dcc9d:
    // 004dcc9d  b91c000000             -mov ecx, 0x1c
    cpu.ecx = 28 /*0x1c*/;
    // 004dcca2  be24d05500             -mov esi, 0x55d024
    cpu.esi = 5623844 /*0x55d024*/;
    // 004dcca7  bf78d36f00             -mov edi, 0x6fd378
    cpu.edi = 7328632 /*0x6fd378*/;
L_0x004dccac:
    // 004dccac  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004dccad  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004dccaf  c1e902                 -shr ecx, 2
    cpu.ecx >>= 2 /*0x2*/ % 32;
    // 004dccb2  f2a5                   -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004dccb4  8ac8                   -mov cl, al
    cpu.cl = cpu.al;
    // 004dccb6  80e103                 +and cl, 3
    cpu.clear_co();
    cpu.set_szp((cpu.cl &= x86::reg8(x86::sreg8(3 /*0x3*/))));
    // 004dccb9  f2a4                   -repne movsb byte ptr es:[edi], byte ptr [esi]
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
    // 004dccbb  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dccbc  eb44                   -jmp 0x4dcd02
    goto L_0x004dcd02;
L_0x004dccbe:
    // 004dccbe  b91c000000             -mov ecx, 0x1c
    cpu.ecx = 28 /*0x1c*/;
    // 004dccc3  be24d05500             -mov esi, 0x55d024
    cpu.esi = 5623844 /*0x55d024*/;
    // 004dccc8  bf78d36f00             -mov edi, 0x6fd378
    cpu.edi = 7328632 /*0x6fd378*/;
    // 004dcccd  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004dccce  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004dccd0  c1e902                 -shr ecx, 2
    cpu.ecx >>= 2 /*0x2*/ % 32;
    // 004dccd3  f2a5                   -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004dccd5  8ac8                   -mov cl, al
    cpu.cl = cpu.al;
    // 004dccd7  80e103                 +and cl, 3
    cpu.clear_co();
    cpu.set_szp((cpu.cl &= x86::reg8(x86::sreg8(3 /*0x3*/))));
    // 004dccda  f2a4                   -repne movsb byte ptr es:[edi], byte ptr [esi]
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
    // 004dccdc  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dccdd  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dccde  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dccdf  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dcce0  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dcce1  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dcce2  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004dcce3:
    // 004dcce3  b91c000000             -mov ecx, 0x1c
    cpu.ecx = 28 /*0x1c*/;
    // 004dcce8  be24d05500             -mov esi, 0x55d024
    cpu.esi = 5623844 /*0x55d024*/;
    // 004dcced  bf78d36f00             -mov edi, 0x6fd378
    cpu.edi = 7328632 /*0x6fd378*/;
    // 004dccf2  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004dccf3  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004dccf5  c1e902                 -shr ecx, 2
    cpu.ecx >>= 2 /*0x2*/ % 32;
    // 004dccf8  f2a5                   -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004dccfa  8ac8                   -mov cl, al
    cpu.cl = cpu.al;
    // 004dccfc  80e103                 +and cl, 3
    cpu.clear_co();
    cpu.set_szp((cpu.cl &= x86::reg8(x86::sreg8(3 /*0x3*/))));
    // 004dccff  f2a4                   -repne movsb byte ptr es:[edi], byte ptr [esi]
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
    // 004dcd01  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
L_0x004dcd02:
    // 004dcd02  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dcd03  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dcd04  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dcd05  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dcd06  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dcd07  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_4dcd10(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004dcd10  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004dcd11  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004dcd13  e8d8000000             -call 0x4dcdf0
    cpu.esp -= 4;
    sub_4dcdf0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dcd18  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dcd19  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4dcd20: native, nfs3hp::load::loadWeatherShapes */

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_4dcdd0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004dcdd0  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004dcdd1  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004dcdd3  833dccd46f0000         +cmp dword ptr [0x6fd4cc], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328972) /* 0x6fd4cc */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004dcdda  7407                   -je 0x4dcde3
    if (cpu.flags.zf)
    {
        goto L_0x004dcde3;
    }
    // 004dcddc  a12ca28c00             -mov eax, dword ptr [0x8ca22c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(9216556) /* 0x8ca22c */);
    // 004dcde1  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dcde2  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004dcde3:
    // 004dcde3  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004dcde5  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dcde6  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_4dcdf0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1, fp2;
    // 004dcdf0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004dcdf1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004dcdf2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004dcdf3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004dcdf4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004dcdf5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004dcdf6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004dcdf8  83ec0c                 -sub esp, 0xc
    (cpu.esp) -= x86::reg32(x86::sreg32(12 /*0xc*/));
    // 004dcdfb  ba5c000000             -mov edx, 0x5c
    cpu.edx = 92 /*0x5c*/;
    // 004dce00  b82ca28c00             -mov eax, 0x8ca22c
    cpu.eax = 9216556 /*0x8ca22c*/;
    // 004dce05  e802390000             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dce0a  ba004b0000             -mov edx, 0x4b00
    cpu.edx = 19200 /*0x4b00*/;
    // 004dce0f  b8e80b8c00             -mov eax, 0x8c0be8
    cpu.eax = 9178088 /*0x8c0be8*/;
    // 004dce14  e8f3380000             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dce19  ba20030000             -mov edx, 0x320
    cpu.edx = 800 /*0x320*/;
    // 004dce1e  b8c0088c00             -mov eax, 0x8c08c0
    cpu.eax = 9177280 /*0x8c08c0*/;
    // 004dce23  e8e4380000             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dce28  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004dce2a  891588425600           -mov dword ptr [0x564288], edx
    app->getMemory<x86::reg32>(x86::reg32(5653128) /* 0x564288 */) = cpu.edx;
    // 004dce30  89158c425600           -mov dword ptr [0x56428c], edx
    app->getMemory<x86::reg32>(x86::reg32(5653132) /* 0x56428c */) = cpu.edx;
    // 004dce36  8a15e8a18c00           -mov dl, byte ptr [0x8ca1e8]
    cpu.dl = app->getMemory<x86::reg8>(x86::reg32(9216488) /* 0x8ca1e8 */);
    // 004dce3c  b903000000             -mov ecx, 3
    cpu.ecx = 3 /*0x3*/;
    // 004dce41  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004dce43  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004dce46  f7f9                   -idiv ecx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ecx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004dce48  89152ca28c00           -mov dword ptr [0x8ca22c], edx
    app->getMemory<x86::reg32>(x86::reg32(9216556) /* 0x8ca22c */) = cpu.edx;
    // 004dce4e  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004dce50  0f848e030000           -je 0x4dd1e4
    if (cpu.flags.zf)
    {
        goto L_0x004dd1e4;
    }
    // 004dce56  bb64000000             -mov ebx, 0x64
    cpu.ebx = 100 /*0x64*/;
    // 004dce5b  66a1eaa18c00           -mov ax, word ptr [0x8ca1ea]
    cpu.ax = app->getMemory<x86::reg16>(x86::reg32(9216490) /* 0x8ca1ea */);
    // 004dce61  8d0c9500000000         -lea ecx, [edx*4]
    cpu.ecx = x86::reg32(cpu.edx * 4);
    // 004dce68  66a33ca28c00           -mov word ptr [0x8ca23c], ax
    app->getMemory<x86::reg16>(x86::reg32(9216572) /* 0x8ca23c */) = cpu.ax;
    // 004dce6e  29d1                   -sub ecx, edx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004dce70  66a1eca18c00           -mov ax, word ptr [0x8ca1ec]
    cpu.ax = app->getMemory<x86::reg16>(x86::reg32(9216492) /* 0x8ca1ec */);
    // 004dce76  c1e102                 -shl ecx, 2
    cpu.ecx <<= 2 /*0x2*/ % 32;
    // 004dce79  66a33ea28c00           -mov word ptr [0x8ca23e], ax
    app->getMemory<x86::reg16>(x86::reg32(9216574) /* 0x8ca23e */) = cpu.ax;
    // 004dce7f  01d1                   -add ecx, edx
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.edx));
    // 004dce81  66a1eea18c00           -mov ax, word ptr [0x8ca1ee]
    cpu.ax = app->getMemory<x86::reg16>(x86::reg32(9216494) /* 0x8ca1ee */);
    // 004dce87  c1e102                 -shl ecx, 2
    cpu.ecx <<= 2 /*0x2*/ % 32;
    // 004dce8a  66a340a28c00           -mov word ptr [0x8ca240], ax
    app->getMemory<x86::reg16>(x86::reg32(9216576) /* 0x8ca240 */) = cpu.ax;
    // 004dce90  8b9198425600           -mov edx, dword ptr [ecx + 0x564298]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(5653144) /* 0x564298 */);
    // 004dce96  a184367d00             -mov eax, dword ptr [0x7d3684]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8205956) /* 0x7d3684 */);
    // 004dce9b  c1e210                 -shl edx, 0x10
    cpu.edx <<= 16 /*0x10*/ % 32;
    // 004dce9e  a348a28c00             -mov dword ptr [0x8ca248], eax
    app->getMemory<x86::reg32>(x86::reg32(9216584) /* 0x8ca248 */) = cpu.eax;
    // 004dcea3  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004dcea5  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004dcea8  f7fb                   -idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004dceaa  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004dceac  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004dceaf  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004dceb1  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 004dceb4  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004dceb6  c1e205                 -shl edx, 5
    cpu.edx <<= 5 /*0x5*/ % 32;
    // 004dceb9  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004dcebb  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004dcebe  c1e210                 +shl edx, 0x10
    {
        x86::reg8 tmp = 16 /*0x10*/ % 32;
        x86::reg32& op = cpu.edx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 004dcec1  1bc2                   -sbb eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 004dcec3  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 004dcec6  8b9194425600           -mov edx, dword ptr [ecx + 0x564294]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(5653140) /* 0x564294 */);
    // 004dcecc  891530a28c00           -mov dword ptr [0x8ca230], edx
    app->getMemory<x86::reg32>(x86::reg32(9216560) /* 0x8ca230 */) = cpu.edx;
    // 004dced2  8bb19c425600           -mov esi, dword ptr [ecx + 0x56429c]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(5653148) /* 0x56429c */);
    // 004dced8  a334a28c00             -mov dword ptr [0x8ca234], eax
    app->getMemory<x86::reg32>(x86::reg32(9216564) /* 0x8ca234 */) = cpu.eax;
    // 004dcedd  83fe01                 +cmp esi, 1
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
    // 004dcee0  7e04                   -jle 0x4dcee6
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004dcee6;
    }
    // 004dcee2  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004dcee4  eb05                   -jmp 0x4dceeb
    goto L_0x004dceeb;
L_0x004dcee6:
    // 004dcee6  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
L_0x004dceeb:
    // 004dceeb  8b152ca28c00           -mov edx, dword ptr [0x8ca22c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(9216556) /* 0x8ca22c */);
    // 004dcef1  8d0c9500000000         -lea ecx, [edx*4]
    cpu.ecx = x86::reg32(cpu.edx * 4);
    // 004dcef8  29d1                   -sub ecx, edx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004dcefa  c1e102                 -shl ecx, 2
    cpu.ecx <<= 2 /*0x2*/ % 32;
    // 004dcefd  bb68010000             -mov ebx, 0x168
    cpu.ebx = 360 /*0x168*/;
    // 004dcf02  01d1                   -add ecx, edx
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.edx));
    // 004dcf04  a34ca28c00             -mov dword ptr [0x8ca24c], eax
    app->getMemory<x86::reg32>(x86::reg32(9216588) /* 0x8ca24c */) = cpu.eax;
    // 004dcf09  c1e102                 -shl ecx, 2
    cpu.ecx <<= 2 /*0x2*/ % 32;
    // 004dcf0c  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004dcf0f  8b81a0425600           -mov eax, dword ptr [ecx + 0x5642a0]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(5653152) /* 0x5642a0 */);
    // 004dcf15  8b91a8425600           -mov edx, dword ptr [ecx + 0x5642a8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(5653160) /* 0x5642a8 */);
    // 004dcf1b  a354a28c00             -mov dword ptr [0x8ca254], eax
    app->getMemory<x86::reg32>(x86::reg32(9216596) /* 0x8ca254 */) = cpu.eax;
    // 004dcf20  8b81a4425600           -mov eax, dword ptr [ecx + 0x5642a4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(5653156) /* 0x5642a4 */);
    // 004dcf26  c1e210                 -shl edx, 0x10
    cpu.edx <<= 16 /*0x10*/ % 32;
    // 004dcf29  a358a28c00             -mov dword ptr [0x8ca258], eax
    app->getMemory<x86::reg32>(x86::reg32(9216600) /* 0x8ca258 */) = cpu.eax;
    // 004dcf2e  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004dcf30  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004dcf33  f7fb                   -idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004dcf35  db45fc                 -fild dword ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 004dcf38  d9e8                   -fld1 
    fp1 = 1.0;
    // 004dcf3a  def1                   -fdivrp st(1)
    fp0 = fp::div(fp1, x86::Float(fp0));
    // 004dcf3c  8b91ac425600           -mov edx, dword ptr [ecx + 0x5642ac]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(5653164) /* 0x5642ac */);
    // 004dcf42  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004dcf45  c1e210                 -shl edx, 0x10
    cpu.edx <<= 16 /*0x10*/ % 32;
    // 004dcf48  b90a000000             -mov ecx, 0xa
    cpu.ecx = 10 /*0xa*/;
    // 004dcf4d  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004dcf4f  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004dcf52  f7f9                   -idiv ecx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ecx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004dcf54  db45fc                 -fild dword ptr [ebp - 4]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 004dcf57  dc0db4975400           -fmul qword ptr [0x5497b4]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<double>(x86::reg32(5543860) /* 0x5497b4 */)));
    // 004dcf5d  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004dcf60  db45fc                 -fild dword ptr [ebp - 4]
    fp2 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 004dcf63  dc0db4975400           -fmul qword ptr [0x5497b4]
    fp2 = fp::mul(fp2, x86::Float(app->getMemory<double>(x86::reg32(5543860) /* 0x5497b4 */)));
    // 004dcf69  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 004dcf6b  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 004dcf6d  d91d50a28c00           -fstp dword ptr [0x8ca250]
    app->getMemory<float>(x86::reg32(9216592) /* 0x8ca250 */) = float(fp2);
    // 004dcf73  bb74a28c00             -mov ebx, 0x8ca274
    cpu.ebx = 9216628 /*0x8ca274*/;
    // 004dcf78  d95df4                 -fstp dword ptr [ebp - 0xc]
    app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */) = float(fp1);
    // 004dcf7b  ba6ca28c00             -mov edx, 0x8ca26c
    cpu.edx = 9216620 /*0x8ca26c*/;
    // 004dcf80  8d45f4                 -lea eax, [ebp - 0xc]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004dcf83  d95df8                 -fstp dword ptr [ebp - 8]
    app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = float(fp0);
    // 004dcf86  e8e5d90000             -call 0x4ea970
    cpu.esp -= 4;
    sub_4ea970(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dcf8b  686ca28c00             -push 0x8ca26c
    app->getMemory<x86::reg32>(cpu.esp-4) = 9216620 /*0x8ca26c*/;
    cpu.esp -= 4;
    // 004dcf90  ba6ca28c00             -mov edx, 0x8ca26c
    cpu.edx = 9216620 /*0x8ca26c*/;
    // 004dcf95  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004dcf9a  ff75f8                 -push dword ptr [ebp - 8]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    cpu.esp -= 4;
    // 004dcf9d  893d70a28c00           -mov dword ptr [0x8ca270], edi
    app->getMemory<x86::reg32>(x86::reg32(9216624) /* 0x8ca270 */) = cpu.edi;
    // 004dcfa3  e8a8300000             -call 0x4e0050
    cpu.esp -= 4;
    nfs3hp::vertices::scaleVectors(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dcfa8  8b152ca28c00           -mov edx, dword ptr [0x8ca22c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(9216556) /* 0x8ca22c */);
    // 004dcfae  8d0c9500000000         -lea ecx, [edx*4]
    cpu.ecx = x86::reg32(cpu.edx * 4);
    // 004dcfb5  29d1                   -sub ecx, edx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004dcfb7  c1e102                 -shl ecx, 2
    cpu.ecx <<= 2 /*0x2*/ % 32;
    // 004dcfba  01d1                   -add ecx, edx
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.edx));
    // 004dcfbc  c1e102                 -shl ecx, 2
    cpu.ecx <<= 2 /*0x2*/ % 32;
    // 004dcfbf  8b8190425600           -mov eax, dword ptr [ecx + 0x564290]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(5653136) /* 0x564290 */);
    // 004dcfc5  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004dcfc7  bb0a000000             -mov ebx, 0xa
    cpu.ebx = 10 /*0xa*/;
    // 004dcfcc  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004dcfcf  f7fb                   -idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004dcfd1  bb64000000             -mov ebx, 0x64
    cpu.ebx = 100 /*0x64*/;
    // 004dcfd6  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004dcfd9  8b81b0425600           -mov eax, dword ptr [ecx + 0x5642b0]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(5653168) /* 0x5642b0 */);
    // 004dcfdf  8b91c0425600           -mov edx, dword ptr [ecx + 0x5642c0]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(5653184) /* 0x5642c0 */);
    // 004dcfe5  a338a28c00             -mov dword ptr [0x8ca238], eax
    app->getMemory<x86::reg32>(x86::reg32(9216568) /* 0x8ca238 */) = cpu.eax;
    // 004dcfea  a184367d00             -mov eax, dword ptr [0x7d3684]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8205956) /* 0x7d3684 */);
    // 004dcfef  c1e210                 -shl edx, 0x10
    cpu.edx <<= 16 /*0x10*/ % 32;
    // 004dcff2  a3e00b8c00             -mov dword ptr [0x8c0be0], eax
    app->getMemory<x86::reg32>(x86::reg32(9178080) /* 0x8c0be0 */) = cpu.eax;
    // 004dcff7  a3e40b8c00             -mov dword ptr [0x8c0be4], eax
    app->getMemory<x86::reg32>(x86::reg32(9178084) /* 0x8c0be4 */) = cpu.eax;
    // 004dcffc  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004dcffe  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004dd001  f7fb                   -idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004dd003  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004dd005  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004dd008  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004dd00a  c1e202                 -shl edx, 2
    cpu.edx <<= 2 /*0x2*/ % 32;
    // 004dd00d  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004dd00f  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004dd012  c1e210                 +shl edx, 0x10
    {
        x86::reg8 tmp = 16 /*0x10*/ % 32;
        x86::reg32& op = cpu.edx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 004dd015  1bc2                   -sbb eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 004dd017  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 004dd01a  db45fc                 -fild dword ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 004dd01d  dc0db4975400           -fmul qword ptr [0x5497b4]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5543860) /* 0x5497b4 */)));
    // 004dd023  db054ca28c00           -fild dword ptr [0x8ca24c]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(9216588) /* 0x8ca24c */)));
    // 004dd029  db0554a28c00           -fild dword ptr [0x8ca254]
    fp2 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(9216596) /* 0x8ca254 */)));
    // 004dd02f  893d44a28c00           -mov dword ptr [0x8ca244], edi
    app->getMemory<x86::reg32>(x86::reg32(9216580) /* 0x8ca244 */) = cpu.edi;
    // 004dd035  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp2;
        fp2 = fp1;
        fp1 = tmp;
    }
    // 004dd037  d91d5ca28c00           -fstp dword ptr [0x8ca25c]
    app->getMemory<float>(x86::reg32(9216604) /* 0x8ca25c */) = float(fp2);
    // 004dd03d  d91d64a28c00           -fstp dword ptr [0x8ca264]
    app->getMemory<float>(x86::reg32(9216612) /* 0x8ca264 */) = float(fp1);
    // 004dd043  a378a28c00             -mov dword ptr [0x8ca278], eax
    app->getMemory<x86::reg32>(x86::reg32(9216632) /* 0x8ca278 */) = cpu.eax;
    // 004dd048  8b81b4425600           -mov eax, dword ptr [ecx + 0x5642b4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(5653172) /* 0x5642b4 */);
    // 004dd04e  d90564a28c00           -fld dword ptr [0x8ca264]
    fp1 = x86::Float(app->getMemory<float>(x86::reg32(9216612) /* 0x8ca264 */));
    // 004dd054  a37ca28c00             -mov dword ptr [0x8ca27c], eax
    app->getMemory<x86::reg32>(x86::reg32(9216636) /* 0x8ca27c */) = cpu.eax;
    // 004dd059  8b81bc425600           -mov eax, dword ptr [ecx + 0x5642bc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(5653180) /* 0x5642bc */);
    // 004dd05f  d981b8425600           -fld dword ptr [ecx + 0x5642b8]
    fp2 = x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(5653176) /* 0x5642b8 */));
    // 004dd065  a384a28c00             -mov dword ptr [0x8ca284], eax
    app->getMemory<x86::reg32>(x86::reg32(9216644) /* 0x8ca284 */) = cpu.eax;
    // 004dd06a  a158a28c00             -mov eax, dword ptr [0x8ca258]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(9216600) /* 0x8ca258 */);
    // 004dd06f  d91d80a28c00           -fstp dword ptr [0x8ca280]
    app->getMemory<float>(x86::reg32(9216640) /* 0x8ca280 */) = float(fp2);
    // 004dd075  f7d8                   -neg eax
    cpu.eax = ~cpu.eax + 1;
    // 004dd077  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 004dd079  d9e0                   -fchs 
    fp1 = -fp1;
    // 004dd07b  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004dd07e  d91d70a28c00           -fstp dword ptr [0x8ca270]
    app->getMemory<float>(x86::reg32(9216624) /* 0x8ca270 */) = float(fp1);
    // 004dd084  db45fc                 -fild dword ptr [ebp - 4]
    fp1 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 004dd087  d91d60a28c00           -fstp dword ptr [0x8ca260]
    app->getMemory<float>(x86::reg32(9216608) /* 0x8ca260 */) = float(fp1);
    // 004dd08d  d82560a28c00           -fsub dword ptr [0x8ca260]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(x86::reg32(9216608) /* 0x8ca260 */)));
    // 004dd093  8b152ca28c00           -mov edx, dword ptr [0x8ca22c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(9216556) /* 0x8ca22c */);
    // 004dd099  d91d68a28c00           -fstp dword ptr [0x8ca268]
    app->getMemory<float>(x86::reg32(9216616) /* 0x8ca268 */) = float(fp0);
    // 004dd09f  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004dd0a1  0f843d010000           -je 0x4dd1e4
    if (cpu.flags.zf)
    {
        goto L_0x004dd1e4;
    }
    // 004dd0a7  e82463fcff             -call 0x4a33d0
    cpu.esp -= 4;
    sub_4a33d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dd0ac  31c9                   +xor ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx))));
    // 004dd0ae  eb0c                   -jmp 0x4dd0bc
    goto L_0x004dd0bc;
L_0x004dd0b0:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004dd0b0  81f920030000           +cmp ecx, 0x320
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(800 /*0x320*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004dd0b6  0f8dfa000000           -jge 0x4dd1b6
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004dd1b6;
    }
L_0x004dd0bc:
    // 004dd0bc  e86fe10000             -call 0x4eb230
    cpu.esp -= 4;
    nfs3hp::ea::randomFraction(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004dd0c1  d80d5ca28c00           -fmul dword ptr [0x8ca25c]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(9216604) /* 0x8ca25c */)));
    // 004dd0c7  d80dbc975400           -fmul dword ptr [0x5497bc]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(5543868) /* 0x5497bc */)));
    // 004dd0cd  8d148d00000000         -lea edx, [ecx*4]
    cpu.edx = x86::reg32(cpu.ecx * 4);
    // 004dd0d4  d8255ca28c00           -fsub dword ptr [0x8ca25c]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(x86::reg32(9216604) /* 0x8ca25c */)));
    // 004dd0da  29ca                   -sub edx, ecx
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004dd0dc  d91c95e8568c00         -fstp dword ptr [edx*4 + 0x8c56e8]
    app->getMemory<float>(x86::reg32(9197288) /* 0x8c56e8 */ + cpu.edx * 4) = float(fp0);
    // 004dd0e3  e848e10000             -call 0x4eb230
    cpu.esp -= 4;
    nfs3hp::ea::randomFraction(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004dd0e8  d80d68a28c00           -fmul dword ptr [0x8ca268]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(9216616) /* 0x8ca268 */)));
    // 004dd0ee  d80560a28c00           -fadd dword ptr [0x8ca260]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(x86::reg32(9216608) /* 0x8ca260 */)));
    // 004dd0f4  d91c95ec568c00         -fstp dword ptr [edx*4 + 0x8c56ec]
    app->getMemory<float>(x86::reg32(9197292) /* 0x8c56ec */ + cpu.edx * 4) = float(fp0);
    // 004dd0fb  e830e10000             -call 0x4eb230
    cpu.esp -= 4;
    nfs3hp::ea::randomFraction(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004dd100  d80d5ca28c00           -fmul dword ptr [0x8ca25c]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(9216604) /* 0x8ca25c */)));
    // 004dd106  d80dbc975400           -fmul dword ptr [0x5497bc]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(5543868) /* 0x5497bc */)));
    // 004dd10c  d8255ca28c00           -fsub dword ptr [0x8ca25c]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(x86::reg32(9216604) /* 0x8ca25c */)));
    // 004dd112  d91c95f0568c00         -fstp dword ptr [edx*4 + 0x8c56f0]
    app->getMemory<float>(x86::reg32(9197296) /* 0x8c56f0 */ + cpu.edx * 4) = float(fp0);
    // 004dd119  e812e10000             -call 0x4eb230
    cpu.esp -= 4;
    nfs3hp::ea::randomFraction(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004dd11e  d80d5ca28c00           -fmul dword ptr [0x8ca25c]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(9216604) /* 0x8ca25c */)));
    // 004dd124  d80dbc975400           -fmul dword ptr [0x5497bc]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(5543868) /* 0x5497bc */)));
    // 004dd12a  d8255ca28c00           -fsub dword ptr [0x8ca25c]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(x86::reg32(9216604) /* 0x8ca25c */)));
    // 004dd130  d91c95687c8c00         -fstp dword ptr [edx*4 + 0x8c7c68]
    app->getMemory<float>(x86::reg32(9206888) /* 0x8c7c68 */ + cpu.edx * 4) = float(fp0);
    // 004dd137  e8f4e00000             -call 0x4eb230
    cpu.esp -= 4;
    nfs3hp::ea::randomFraction(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004dd13c  d80d68a28c00           -fmul dword ptr [0x8ca268]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(9216616) /* 0x8ca268 */)));
    // 004dd142  d80560a28c00           -fadd dword ptr [0x8ca260]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(x86::reg32(9216608) /* 0x8ca260 */)));
    // 004dd148  bb000080bf             -mov ebx, 0xbf800000
    cpu.ebx = 3212836864 /*0xbf800000*/;
    // 004dd14d  d91c956c7c8c00         -fstp dword ptr [edx*4 + 0x8c7c6c]
    app->getMemory<float>(x86::reg32(9206892) /* 0x8c7c6c */ + cpu.edx * 4) = float(fp0);
    // 004dd154  e8d7e00000             -call 0x4eb230
    cpu.esp -= 4;
    nfs3hp::ea::randomFraction(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004dd159  891c95e80b8c00         -mov dword ptr [edx*4 + 0x8c0be8], ebx
    app->getMemory<x86::reg32>(x86::reg32(9178088) /* 0x8c0be8 */ + cpu.edx * 4) = cpu.ebx;
    // 004dd160  d80d5ca28c00           -fmul dword ptr [0x8ca25c]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(9216604) /* 0x8ca25c */)));
    // 004dd166  891c95ec0b8c00         -mov dword ptr [edx*4 + 0x8c0bec], ebx
    app->getMemory<x86::reg32>(x86::reg32(9178092) /* 0x8c0bec */ + cpu.edx * 4) = cpu.ebx;
    // 004dd16d  d80dbc975400           -fmul dword ptr [0x5497bc]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(5543868) /* 0x5497bc */)));
    // 004dd173  891c9568318c00         -mov dword ptr [edx*4 + 0x8c3168], ebx
    app->getMemory<x86::reg32>(x86::reg32(9187688) /* 0x8c3168 */ + cpu.edx * 4) = cpu.ebx;
    // 004dd17a  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 004dd17c  891c956c318c00         -mov dword ptr [edx*4 + 0x8c316c], ebx
    app->getMemory<x86::reg32>(x86::reg32(9187692) /* 0x8c316c */ + cpu.edx * 4) = cpu.ebx;
    // 004dd183  30fc                   -xor ah, bh
    cpu.ah ^= x86::reg8(x86::sreg8(cpu.bh));
    // 004dd185  30db                   +xor bl, bl
    cpu.clear_co();
    cpu.set_szp((cpu.bl ^= x86::reg8(x86::sreg8(cpu.bl))));
    // 004dd187  882495f10b8c00         -mov byte ptr [edx*4 + 0x8c0bf1], ah
    app->getMemory<x86::reg8>(x86::reg32(9178097) /* 0x8c0bf1 */ + cpu.edx * 4) = cpu.ah;
    // 004dd18e  881c9571318c00         -mov byte ptr [edx*4 + 0x8c3171], bl
    app->getMemory<x86::reg8>(x86::reg32(9187697) /* 0x8c3171 */ + cpu.edx * 4) = cpu.bl;
    // 004dd195  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004dd196  881c95f00b8c00         -mov byte ptr [edx*4 + 0x8c0bf0], bl
    app->getMemory<x86::reg8>(x86::reg32(9178096) /* 0x8c0bf0 */ + cpu.edx * 4) = cpu.bl;
    // 004dd19d  d8255ca28c00           +fsub dword ptr [0x8ca25c]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(x86::reg32(9216604) /* 0x8ca25c */)));
    // 004dd1a3  881c9570318c00         -mov byte ptr [edx*4 + 0x8c3170], bl
    app->getMemory<x86::reg8>(x86::reg32(9187696) /* 0x8c3170 */ + cpu.edx * 4) = cpu.bl;
    // 004dd1aa  d91c95707c8c00         +fstp dword ptr [edx*4 + 0x8c7c70]
    app->getMemory<float>(x86::reg32(9206896) /* 0x8c7c70 */ + cpu.edx * 4) = float(fp0);
    // 004dd1b1  e9fafeffff             -jmp 0x4dd0b0
    goto L_0x004dd0b0;
L_0x004dd1b6:
    // 004dd1b6  e81562fcff             -call 0x4a33d0
    cpu.esp -= 4;
    sub_4a33d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dd1bb  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 004dd1bd  eb05                   -jmp 0x4dd1c4
    goto L_0x004dd1c4;
L_0x004dd1bf:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004dd1bf  83fa14                 +cmp edx, 0x14
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
    // 004dd1c2  7d20                   -jge 0x4dd1e4
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004dd1e4;
    }
L_0x004dd1c4:
    // 004dd1c4  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 004dd1c6  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004dd1cd  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004dd1cf  31d1                   -xor ecx, edx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004dd1d1  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 004dd1d3  890c85c8088c00         -mov dword ptr [eax*4 + 0x8c08c8], ecx
    app->getMemory<x86::reg32>(x86::reg32(9177288) /* 0x8c08c8 */ + cpu.eax * 4) = cpu.ecx;
    // 004dd1da  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004dd1db  891c85580a8c00         -mov dword ptr [eax*4 + 0x8c0a58], ebx
    app->getMemory<x86::reg32>(x86::reg32(9177688) /* 0x8c0a58 */ + cpu.eax * 4) = cpu.ebx;
    // 004dd1e2  ebdb                   -jmp 0x4dd1bf
    goto L_0x004dd1bf;
L_0x004dd1e4:
    // 004dd1e4  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004dd1e6  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd1e7  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd1e8  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd1e9  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd1ea  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd1eb  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd1ec  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4dd1f0: native, nfs3hp::dead::stub_0x4dd1f0 */

/* align: skip 0x8b 0xc0 */
void Application::sub_4dd230(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0;
    // 004dd230  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004dd231  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004dd232  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004dd233  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004dd234  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004dd235  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004dd236  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004dd238  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004dd23a  8b0d2ca28c00           -mov ecx, dword ptr [0x8ca22c]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(9216556) /* 0x8ca22c */);
    // 004dd240  8b7004                 -mov esi, dword ptr [eax + 4]
    cpu.esi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 004dd243  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004dd245  0f84d7000000           -je 0x4dd322
    if (cpu.flags.zf)
    {
        goto L_0x004dd322;
    }
    // 004dd24b  e8103bf4ff             -call 0x420d60
    cpu.esp -= 4;
    nfs3hp::trackdata::carFacingWay(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dd250  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004dd252  0f8598000000           -jne 0x4dd2f0
    if (!cpu.flags.zf)
    {
        goto L_0x004dd2f0;
    }
    // 004dd258  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
L_0x004dd25a:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004dd25a  3b1d34a28c00           +cmp ebx, dword ptr [0x8ca234]
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(9216564) /* 0x8ca234 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004dd260  0f8d8a000000           -jge 0x4dd2f0
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004dd2f0;
    }
    // 004dd266  8d04b500000000         -lea eax, [esi*4]
    cpu.eax = x86::reg32(cpu.esi * 4);
    // 004dd26d  01f0                   -add eax, esi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.esi));
    // 004dd26f  c1e007                 -shl eax, 7
    cpu.eax <<= 7 /*0x7*/ % 32;
    // 004dd272  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 004dd274  c1e704                 -shl edi, 4
    cpu.edi <<= 4 /*0x4*/ % 32;
    // 004dd277  29c7                   -sub edi, eax
    (cpu.edi) -= x86::reg32(x86::sreg32(cpu.eax));
    // 004dd279  8d049d00000000         -lea eax, [ebx*4]
    cpu.eax = x86::reg32(cpu.ebx * 4);
    // 004dd280  29d8                   -sub eax, ebx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 004dd282  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004dd285  8d0c07                 -lea ecx, [edi + eax]
    cpu.ecx = x86::reg32(cpu.edi + cpu.eax * 1);
    // 004dd288  30e4                   +xor ah, ah
    cpu.clear_co();
    cpu.set_szp((cpu.ah ^= x86::reg8(x86::sreg8(cpu.ah))));
    // 004dd28a  88a1f10b8c00           -mov byte ptr [ecx + 0x8c0bf1], ah
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(9178097) /* 0x8c0bf1 */) = cpu.ah;
    // 004dd290  e89bdf0000             -call 0x4eb230
    cpu.esp -= 4;
    nfs3hp::ea::randomFraction(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004dd295  d80d5ca28c00           +fmul dword ptr [0x8ca25c]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(9216604) /* 0x8ca25c */)));
    // 004dd29b  d80dc0975400           +fmul dword ptr [0x5497c0]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(5543872) /* 0x5497c0 */)));
    // 004dd2a1  d8255ca28c00           +fsub dword ptr [0x8ca25c]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(x86::reg32(9216604) /* 0x8ca25c */)));
    // 004dd2a7  d84208                 +fadd dword ptr [edx + 8]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(8) /* 0x8 */)));
    // 004dd2aa  d999e8568c00           +fstp dword ptr [ecx + 0x8c56e8]
    app->getMemory<float>(cpu.ecx + x86::reg32(9197288) /* 0x8c56e8 */) = float(fp0);
    // 004dd2b0  e87bdf0000             -call 0x4eb230
    cpu.esp -= 4;
    nfs3hp::ea::randomFraction(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004dd2b5  d80d68a28c00           +fmul dword ptr [0x8ca268]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(9216616) /* 0x8ca268 */)));
    // 004dd2bb  d80560a28c00           +fadd dword ptr [0x8ca260]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(x86::reg32(9216608) /* 0x8ca260 */)));
    // 004dd2c1  d8420c                 +fadd dword ptr [edx + 0xc]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(12) /* 0xc */)));
    // 004dd2c4  d999ec568c00           +fstp dword ptr [ecx + 0x8c56ec]
    app->getMemory<float>(cpu.ecx + x86::reg32(9197292) /* 0x8c56ec */) = float(fp0);
    // 004dd2ca  e861df0000             -call 0x4eb230
    cpu.esp -= 4;
    nfs3hp::ea::randomFraction(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004dd2cf  d80d5ca28c00           +fmul dword ptr [0x8ca25c]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(9216604) /* 0x8ca25c */)));
    // 004dd2d5  d80dc0975400           +fmul dword ptr [0x5497c0]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(5543872) /* 0x5497c0 */)));
    // 004dd2db  d8255ca28c00           +fsub dword ptr [0x8ca25c]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(x86::reg32(9216604) /* 0x8ca25c */)));
    // 004dd2e1  d84210                 +fadd dword ptr [edx + 0x10]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(16) /* 0x10 */)));
    // 004dd2e4  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004dd2e5  d999f0568c00           +fstp dword ptr [ecx + 0x8c56f0]
    app->getMemory<float>(cpu.ecx + x86::reg32(9197296) /* 0x8c56f0 */) = float(fp0);
    // 004dd2eb  e96affffff             -jmp 0x4dd25a
    goto L_0x004dd25a;
L_0x004dd2f0:
    // 004dd2f0  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x004dd2f2:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004dd2f2  3b1578a28c00           +cmp edx, dword ptr [0x8ca278]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(9216632) /* 0x8ca278 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004dd2f8  7d28                   -jge 0x4dd322
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004dd322;
    }
    // 004dd2fa  8d1cb500000000         -lea ebx, [esi*4]
    cpu.ebx = x86::reg32(cpu.esi * 4);
    // 004dd301  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 004dd303  29f3                   -sub ebx, esi
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.esi));
    // 004dd305  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004dd30c  c1e303                 -shl ebx, 3
    cpu.ebx <<= 3 /*0x3*/ % 32;
    // 004dd30f  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004dd311  01f3                   -add ebx, esi
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.esi));
    // 004dd313  31d1                   -xor ecx, edx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004dd315  c1e304                 +shl ebx, 4
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
    // 004dd318  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004dd319  898c83c8088c00         -mov dword ptr [ebx + eax*4 + 0x8c08c8], ecx
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(9177288) /* 0x8c08c8 */ + cpu.eax * 4) = cpu.ecx;
    // 004dd320  ebd0                   -jmp 0x4dd2f2
    goto L_0x004dd2f2;
L_0x004dd322:
    // 004dd322  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd323  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd324  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd325  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd326  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd327  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd328  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_4dd330(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004dd330  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004dd331  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004dd332  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004dd333  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004dd334  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004dd335  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004dd336  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004dd338  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004dd33b  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004dd33d  a1f0a18c00             -mov eax, dword ptr [0x8ca1f0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(9216496) /* 0x8ca1f0 */);
    // 004dd342  8b1df4a18c00           -mov ebx, dword ptr [0x8ca1f4]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(9216500) /* 0x8ca1f4 */);
    // 004dd348  8b1548a28c00           -mov edx, dword ptr [0x8ca248]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(9216584) /* 0x8ca248 */);
    // 004dd34e  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004dd351  a184367d00             -mov eax, dword ptr [0x7d3684]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8205956) /* 0x7d3684 */);
    // 004dd356  8b3df8a18c00           -mov edi, dword ptr [0x8ca1f8]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(9216504) /* 0x8ca1f8 */);
    // 004dd35c  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004dd35e  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004dd361  8b35fca18c00           -mov esi, dword ptr [0x8ca1fc]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(9216508) /* 0x8ca1fc */);
    // 004dd367  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004dd369  0f8453000000           -je 0x4dd3c2
    if (cpu.flags.zf)
    {
        goto L_0x004dd3c2;
    }
    // 004dd36f  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 004dd371  744f                   -je 0x4dd3c2
    if (cpu.flags.zf)
    {
        goto L_0x004dd3c2;
    }
    // 004dd373  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 004dd375  744b                   -je 0x4dd3c2
    if (cpu.flags.zf)
    {
        goto L_0x004dd3c2;
    }
    // 004dd377  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 004dd379  7447                   -je 0x4dd3c2
    if (cpu.flags.zf)
    {
        goto L_0x004dd3c2;
    }
    // 004dd37b  39d0                   +cmp eax, edx
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
    // 004dd37d  7c43                   -jl 0x4dd3c2
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004dd3c2;
    }
    // 004dd37f  39d8                   +cmp eax, ebx
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
    // 004dd381  7d15                   -jge 0x4dd398
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004dd398;
    }
    // 004dd383  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 004dd385  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 004dd387  0fafd1                 -imul edx, ecx
    cpu.edx = x86::reg32(x86::sreg64(x86::sreg32(cpu.edx)) * x86::sreg64(x86::sreg32(cpu.ecx)));
    // 004dd38a  8b4dfc                 -mov ecx, dword ptr [ebp - 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004dd38d  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004dd38f  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004dd392  29cb                   -sub ebx, ecx
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004dd394  f7fb                   +idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004dd396  eb2c                   -jmp 0x4dd3c4
    goto L_0x004dd3c4;
L_0x004dd398:
    // 004dd398  39f8                   +cmp eax, edi
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
    // 004dd39a  7d04                   -jge 0x4dd3a0
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004dd3a0;
    }
    // 004dd39c  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004dd39e  eb24                   -jmp 0x4dd3c4
    goto L_0x004dd3c4;
L_0x004dd3a0:
    // 004dd3a0  39f0                   +cmp eax, esi
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
    // 004dd3a2  7d14                   -jge 0x4dd3b8
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004dd3b8;
    }
    // 004dd3a4  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 004dd3a6  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 004dd3a8  0fafd1                 -imul edx, ecx
    cpu.edx = x86::reg32(x86::sreg64(x86::sreg32(cpu.edx)) * x86::sreg64(x86::sreg32(cpu.ecx)));
    // 004dd3ab  29fe                   -sub esi, edi
    (cpu.esi) -= x86::reg32(x86::sreg32(cpu.edi));
    // 004dd3ad  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004dd3af  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004dd3b2  f7fe                   -idiv esi
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.esi);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004dd3b4  29c1                   +sub ecx, eax
    {
        x86::reg32& tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004dd3b6  eb0a                   -jmp 0x4dd3c2
    goto L_0x004dd3c2;
L_0x004dd3b8:
    // 004dd3b8  a184367d00             -mov eax, dword ptr [0x7d3684]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8205956) /* 0x7d3684 */);
    // 004dd3bd  a348a28c00             -mov dword ptr [0x8ca248], eax
    app->getMemory<x86::reg32>(x86::reg32(9216584) /* 0x8ca248 */) = cpu.eax;
L_0x004dd3c2:
    // 004dd3c2  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
L_0x004dd3c4:
    // 004dd3c4  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004dd3c6  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd3c7  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd3c8  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd3c9  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd3ca  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd3cb  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd3cc  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_4dd3d0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004dd3d0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004dd3d1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004dd3d2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004dd3d3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004dd3d4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004dd3d5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004dd3d6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004dd3d8  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004dd3db  8b353aa28c00           -mov esi, dword ptr [0x8ca23a]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(9216570) /* 0x8ca23a */);
    // 004dd3e1  8b1544a28c00           -mov edx, dword ptr [0x8ca244]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(9216580) /* 0x8ca244 */);
    // 004dd3e7  c1fe10                 -sar esi, 0x10
    cpu.esi = x86::reg32(x86::sreg32(cpu.esi) >> (16 /*0x10*/ % 32));
    // 004dd3ea  01d6                   -add esi, edx
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.edx));
    // 004dd3ec  8b153ca28c00           -mov edx, dword ptr [0x8ca23c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(9216572) /* 0x8ca23c */);
    // 004dd3f2  8b0d44a28c00           -mov ecx, dword ptr [0x8ca244]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(9216580) /* 0x8ca244 */);
    // 004dd3f8  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 004dd3fb  01ca                   -add edx, ecx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004dd3fd  8b0d3ea28c00           -mov ecx, dword ptr [0x8ca23e]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(9216574) /* 0x8ca23e */);
    // 004dd403  c1f910                 -sar ecx, 0x10
    cpu.ecx = x86::reg32(x86::sreg32(cpu.ecx) >> (16 /*0x10*/ % 32));
    // 004dd406  8d1c0e                 -lea ebx, [esi + ecx]
    cpu.ebx = x86::reg32(cpu.esi + cpu.ecx * 1);
    // 004dd409  895dfc                 -mov dword ptr [ebp - 4], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ebx;
    // 004dd40c  89d3                   -mov ebx, edx
    cpu.ebx = cpu.edx;
    // 004dd40e  29cb                   -sub ebx, ecx
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004dd410  39d6                   +cmp esi, edx
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004dd412  7508                   -jne 0x4dd41c
    if (!cpu.flags.zf)
    {
        goto L_0x004dd41c;
    }
    // 004dd414  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004dd416  0f848d000000           -je 0x4dd4a9
    if (cpu.flags.zf)
    {
        goto L_0x004dd4a9;
    }
L_0x004dd41c:
    // 004dd41c  39d6                   +cmp esi, edx
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004dd41e  7e14                   -jle 0x4dd434
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004dd434;
    }
    // 004dd420  39d0                   +cmp eax, edx
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
    // 004dd422  7d06                   -jge 0x4dd42a
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004dd42a;
    }
    // 004dd424  030540d95d00           -add eax, dword ptr [0x5dd940]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(6150464) /* 0x5dd940 */)));
L_0x004dd42a:
    // 004dd42a  8b3d40d95d00           -mov edi, dword ptr [0x5dd940]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(6150464) /* 0x5dd940 */);
    // 004dd430  01fb                   -add ebx, edi
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.edi));
    // 004dd432  01fa                   -add edx, edi
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.edi));
L_0x004dd434:
    // 004dd434  39f0                   +cmp eax, esi
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
    // 004dd436  7c04                   -jl 0x4dd43c
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004dd43c;
    }
    // 004dd438  39d0                   +cmp eax, edx
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
    // 004dd43a  7e07                   -jle 0x4dd443
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004dd443;
    }
L_0x004dd43c:
    // 004dd43c  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004dd43e  e96b000000             -jmp 0x4dd4ae
    goto L_0x004dd4ae;
L_0x004dd443:
    // 004dd443  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004dd445  7462                   -je 0x4dd4a9
    if (cpu.flags.zf)
    {
        goto L_0x004dd4a9;
    }
    // 004dd447  3b45fc                 +cmp eax, dword ptr [ebp - 4]
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
    // 004dd44a  7e04                   -jle 0x4dd450
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004dd450;
    }
    // 004dd44c  39d8                   +cmp eax, ebx
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
    // 004dd44e  7c59                   -jl 0x4dd4a9
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004dd4a9;
    }
L_0x004dd450:
    // 004dd450  8b75fc                 -mov esi, dword ptr [ebp - 4]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004dd453  39f0                   +cmp eax, esi
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
    // 004dd455  7d29                   -jge 0x4dd480
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004dd480;
    }
    // 004dd457  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 004dd459  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 004dd45b  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004dd45d  8b1534a28c00           -mov edx, dword ptr [0x8ca234]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(9216564) /* 0x8ca234 */);
    // 004dd463  0fafd0                 -imul edx, eax
    cpu.edx = x86::reg32(x86::sreg64(x86::sreg32(cpu.edx)) * x86::sreg64(x86::sreg32(cpu.eax)));
    // 004dd466  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004dd468  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004dd46b  f7f9                   -idiv ecx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ecx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004dd46d  8b1534a28c00           -mov edx, dword ptr [0x8ca234]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(9216564) /* 0x8ca234 */);
    // 004dd473  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 004dd475  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004dd477  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004dd479  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd47a  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd47b  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd47c  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd47d  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd47e  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd47f  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004dd480:
    // 004dd480  39d8                   +cmp eax, ebx
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
    // 004dd482  7e25                   -jle 0x4dd4a9
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004dd4a9;
    }
    // 004dd484  8b1534a28c00           -mov edx, dword ptr [0x8ca234]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(9216564) /* 0x8ca234 */);
    // 004dd48a  29d8                   -sub eax, ebx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 004dd48c  0fafd0                 -imul edx, eax
    cpu.edx = x86::reg32(x86::sreg64(x86::sreg32(cpu.edx)) * x86::sreg64(x86::sreg32(cpu.eax)));
    // 004dd48f  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004dd491  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004dd494  f7f9                   -idiv ecx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ecx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004dd496  8b1534a28c00           -mov edx, dword ptr [0x8ca234]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(9216564) /* 0x8ca234 */);
    // 004dd49c  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 004dd49e  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004dd4a0  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004dd4a2  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd4a3  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd4a4  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd4a5  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd4a6  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd4a7  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd4a8  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004dd4a9:
    // 004dd4a9  a134a28c00             -mov eax, dword ptr [0x8ca234]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(9216564) /* 0x8ca234 */);
L_0x004dd4ae:
    // 004dd4ae  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004dd4b0  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd4b1  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd4b2  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd4b3  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd4b4  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd4b5  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd4b6  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_4dd4c0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004dd4c0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004dd4c1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004dd4c2  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004dd4c3  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004dd4c4  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004dd4c6  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 004dd4c8  833d34a28c0000         +cmp dword ptr [0x8ca234], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(9216564) /* 0x8ca234 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004dd4cf  742c                   -je 0x4dd4fd
    if (cpu.flags.zf)
    {
        goto L_0x004dd4fd;
    }
    // 004dd4d1  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004dd4d3  7428                   -je 0x4dd4fd
    if (cpu.flags.zf)
    {
        goto L_0x004dd4fd;
    }
    // 004dd4d5  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004dd4d7  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004dd4da  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004dd4dc  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 004dd4df  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004dd4e1  c1e202                 -shl edx, 2
    cpu.edx <<= 2 /*0x2*/ % 32;
    // 004dd4e4  8b1d34a28c00           -mov ebx, dword ptr [0x8ca234]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(9216564) /* 0x8ca234 */);
    // 004dd4ea  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004dd4ec  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004dd4ef  f7fb                   -idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004dd4f1  89048d88425600         -mov dword ptr [ecx*4 + 0x564288], eax
    app->getMemory<x86::reg32>(x86::reg32(5653128) /* 0x564288 */ + cpu.ecx * 4) = cpu.eax;
    // 004dd4f8  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd4f9  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd4fa  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd4fb  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd4fc  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004dd4fd:
    // 004dd4fd  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 004dd4ff  89348d88425600         -mov dword ptr [ecx*4 + 0x564288], esi
    app->getMemory<x86::reg32>(x86::reg32(5653128) /* 0x564288 */ + cpu.ecx * 4) = cpu.esi;
    // 004dd506  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd507  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd508  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd509  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd50a  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_4dd510(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004dd510  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004dd511  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004dd512  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004dd513  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004dd514  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004dd515  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004dd516  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004dd518  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004dd51a  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004dd51d  8b88b8105e00           -mov ecx, dword ptr [eax + 0x5e10b8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(6164664) /* 0x5e10b8 */);
    // 004dd523  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 004dd525  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004dd527  740a                   -je 0x4dd533
    if (cpu.flags.zf)
    {
        goto L_0x004dd533;
    }
    // 004dd529  7404                   -je 0x4dd52f
    if (cpu.flags.zf)
    {
        goto L_0x004dd52f;
    }
    // 004dd52b  8b01                   -mov eax, dword ptr [ecx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx);
    // 004dd52d  eb14                   -jmp 0x4dd543
    goto L_0x004dd543;
L_0x004dd52f:
    // 004dd52f  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004dd531  eb10                   -jmp 0x4dd543
    goto L_0x004dd543;
L_0x004dd533:
    // 004dd533  8bb8b0105e00           -mov edi, dword ptr [eax + 0x5e10b0]
    cpu.edi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(6164656) /* 0x5e10b0 */);
    // 004dd539  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 004dd53b  7404                   -je 0x4dd541
    if (cpu.flags.zf)
    {
        goto L_0x004dd541;
    }
    // 004dd53d  8b07                   -mov eax, dword ptr [edi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi);
    // 004dd53f  eb02                   -jmp 0x4dd543
    goto L_0x004dd543;
L_0x004dd541:
    // 004dd541  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x004dd543:
    // 004dd543  83f804                 +cmp eax, 4
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
    // 004dd546  7f3f                   -jg 0x4dd587
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x004dd587;
    }
    // 004dd548  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004dd54f  8b88b8105e00           -mov ecx, dword ptr [eax + 0x5e10b8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(6164664) /* 0x5e10b8 */);
    // 004dd555  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004dd557  740a                   -je 0x4dd563
    if (cpu.flags.zf)
    {
        goto L_0x004dd563;
    }
    // 004dd559  7404                   -je 0x4dd55f
    if (cpu.flags.zf)
    {
        goto L_0x004dd55f;
    }
    // 004dd55b  8b01                   -mov eax, dword ptr [ecx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx);
    // 004dd55d  eb14                   -jmp 0x4dd573
    goto L_0x004dd573;
L_0x004dd55f:
    // 004dd55f  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004dd561  eb10                   -jmp 0x4dd573
    goto L_0x004dd573;
L_0x004dd563:
    // 004dd563  8bb8b0105e00           -mov edi, dword ptr [eax + 0x5e10b0]
    cpu.edi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(6164656) /* 0x5e10b0 */);
    // 004dd569  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 004dd56b  7404                   -je 0x4dd571
    if (cpu.flags.zf)
    {
        goto L_0x004dd571;
    }
    // 004dd56d  8b07                   -mov eax, dword ptr [edi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi);
    // 004dd56f  eb02                   -jmp 0x4dd573
    goto L_0x004dd573;
L_0x004dd571:
    // 004dd571  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x004dd573:
    // 004dd573  83f802                 +cmp eax, 2
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
    // 004dd576  7c0f                   -jl 0x4dd587
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004dd587;
    }
    // 004dd578  833c95a0c4790000       +cmp dword ptr [edx*4 + 0x79c4a0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7980192) /* 0x79c4a0 */ + cpu.edx * 4);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004dd580  7405                   -je 0x4dd587
    if (cpu.flags.zf)
    {
        goto L_0x004dd587;
    }
    // 004dd582  be01000000             -mov esi, 1
    cpu.esi = 1 /*0x1*/;
L_0x004dd587:
    // 004dd587  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004dd58e  3bb0b4a28c00           +cmp esi, dword ptr [eax + 0x8ca2b4]
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(9216692) /* 0x8ca2b4 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004dd594  7509                   -jne 0x4dd59f
    if (!cpu.flags.zf)
    {
        goto L_0x004dd59f;
    }
    // 004dd596  83b8f42f550000         +cmp dword ptr [eax + 0x552ff4], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(5582836) /* 0x552ff4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004dd59d  7432                   -je 0x4dd5d1
    if (cpu.flags.zf)
    {
        goto L_0x004dd5d1;
    }
L_0x004dd59f:
    // 004dd59f  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
L_0x004dd5a1:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004dd5a1  3b1d34a28c00           +cmp ebx, dword ptr [0x8ca234]
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(9216564) /* 0x8ca234 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004dd5a7  7d28                   -jge 0x4dd5d1
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004dd5d1;
    }
    // 004dd5a9  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004dd5b0  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004dd5b2  c1e007                 -shl eax, 7
    cpu.eax <<= 7 /*0x7*/ % 32;
    // 004dd5b5  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 004dd5b7  c1e704                 -shl edi, 4
    cpu.edi <<= 4 /*0x4*/ % 32;
    // 004dd5ba  29c7                   -sub edi, eax
    (cpu.edi) -= x86::reg32(x86::sreg32(cpu.eax));
    // 004dd5bc  8d049d00000000         -lea eax, [ebx*4]
    cpu.eax = x86::reg32(cpu.ebx * 4);
    // 004dd5c3  29d8                   -sub eax, ebx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 004dd5c5  30c9                   +xor cl, cl
    cpu.clear_co();
    cpu.set_szp((cpu.cl ^= x86::reg8(x86::sreg8(cpu.cl))));
    // 004dd5c7  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004dd5c8  888c87f10b8c00         -mov byte ptr [edi + eax*4 + 0x8c0bf1], cl
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(9178097) /* 0x8c0bf1 */ + cpu.eax * 4) = cpu.cl;
    // 004dd5cf  ebd0                   -jmp 0x4dd5a1
    goto L_0x004dd5a1;
L_0x004dd5d1:
    // 004dd5d1  893495b4a28c00         -mov dword ptr [edx*4 + 0x8ca2b4], esi
    app->getMemory<x86::reg32>(x86::reg32(9216692) /* 0x8ca2b4 */ + cpu.edx * 4) = cpu.esi;
    // 004dd5d8  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd5d9  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd5da  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd5db  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd5dc  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd5dd  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd5de  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x90 */
void Application::sub_4dd5e0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1, fp2;
  x86::reg16 fsw = 0;
    // 004dd5e0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004dd5e1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004dd5e2  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004dd5e3  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004dd5e4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004dd5e5  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004dd5e7  83ec2c                 -sub esp, 0x2c
    (cpu.esp) -= x86::reg32(x86::sreg32(44 /*0x2c*/));
    // 004dd5ea  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004dd5ec  8955e8                 -mov dword ptr [ebp - 0x18], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = cpu.edx;
    // 004dd5ef  8b4004                 -mov eax, dword ptr [eax + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 004dd5f2  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 004dd5f5  8d148500000000         -lea edx, [eax*4]
    cpu.edx = x86::reg32(cpu.eax * 4);
    // 004dd5fc  a184367d00             -mov eax, dword ptr [0x7d3684]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8205956) /* 0x7d3684 */);
    // 004dd601  2b82e00b8c00           -sub eax, dword ptr [edx + 0x8c0be0]
    (cpu.eax) -= x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.edx + x86::reg32(9178080) /* 0x8c0be0 */)));
    // 004dd607  83f801                 +cmp eax, 1
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
    // 004dd60a  0f8e16030000           -jle 0x4dd926
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004dd926;
    }
    // 004dd610  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004dd612  e84937f4ff             -call 0x420d60
    cpu.esp -= 4;
    nfs3hp::trackdata::carFacingWay(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dd617  8945ec                 -mov dword ptr [ebp - 0x14], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.eax;
    // 004dd61a  a184367d00             -mov eax, dword ptr [0x7d3684]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8205956) /* 0x7d3684 */);
    // 004dd61f  8982e00b8c00           -mov dword ptr [edx + 0x8c0be0], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(9178080) /* 0x8c0be0 */) = cpu.eax;
    // 004dd625  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x004dd627:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004dd627  3b1578a28c00           +cmp edx, dword ptr [0x8ca278]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(9216632) /* 0x8ca278 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004dd62d  7d38                   -jge 0x4dd667
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004dd667;
    }
    // 004dd62f  8b4df8                 -mov ecx, dword ptr [ebp - 8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004dd632  8d1c8d00000000         -lea ebx, [ecx*4]
    cpu.ebx = x86::reg32(cpu.ecx * 4);
    // 004dd639  29cb                   -sub ebx, ecx
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004dd63b  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004dd642  c1e303                 -shl ebx, 3
    cpu.ebx <<= 3 /*0x3*/ % 32;
    // 004dd645  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004dd647  01cb                   -add ebx, ecx
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004dd649  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004dd64c  c1e304                 -shl ebx, 4
    cpu.ebx <<= 4 /*0x4*/ % 32;
    // 004dd64f  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 004dd651  8bb8c8088c00           -mov edi, dword ptr [eax + 0x8c08c8]
    cpu.edi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(9177288) /* 0x8c08c8 */);
    // 004dd657  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 004dd659  7409                   -je 0x4dd664
    if (cpu.flags.zf)
    {
        goto L_0x004dd664;
    }
    // 004dd65b  8d4fff                 -lea ecx, [edi - 1]
    cpu.ecx = x86::reg32(cpu.edi + x86::reg32(-1) /* -0x1 */);
    // 004dd65e  8988c8088c00           -mov dword ptr [eax + 0x8c08c8], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(9177288) /* 0x8c08c8 */) = cpu.ecx;
L_0x004dd664:
    // 004dd664  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004dd665  ebc0                   -jmp 0x4dd627
    goto L_0x004dd627;
L_0x004dd667:
    // 004dd667  8b5df8                 -mov ebx, dword ptr [ebp - 8]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004dd66a  8d049d00000000         -lea eax, [ebx*4]
    cpu.eax = x86::reg32(cpu.ebx * 4);
    // 004dd671  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 004dd673  c1e007                 -shl eax, 7
    cpu.eax <<= 7 /*0x7*/ % 32;
    // 004dd676  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 004dd678  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 004dd67b  bae8568c00             -mov edx, 0x8c56e8
    cpu.edx = 9197288 /*0x8c56e8*/;
    // 004dd680  29d8                   -sub eax, ebx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 004dd682  bb6ca28c00             -mov ebx, 0x8ca26c
    cpu.ebx = 9216620 /*0x8ca26c*/;
    // 004dd687  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004dd689  a134a28c00             -mov eax, dword ptr [0x8ca234]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(9216564) /* 0x8ca234 */);
    // 004dd68e  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 004dd690  e81b290000             -call 0x4dffb0
    cpu.esp -= 4;
    nfs3hp::vertices::addVectors(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dd695  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
L_0x004dd697:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004dd697  3b5de8                 +cmp ebx, dword ptr [ebp - 0x18]
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004dd69a  0f8d86020000           -jge 0x4dd926
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004dd926;
    }
    // 004dd6a0  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004dd6a3  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004dd6aa  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004dd6ac  c1e007                 -shl eax, 7
    cpu.eax <<= 7 /*0x7*/ % 32;
    // 004dd6af  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 004dd6b1  c1e704                 -shl edi, 4
    cpu.edi <<= 4 /*0x4*/ % 32;
    // 004dd6b4  29c7                   -sub edi, eax
    (cpu.edi) -= x86::reg32(x86::sreg32(cpu.eax));
    // 004dd6b6  8d049d00000000         -lea eax, [ebx*4]
    cpu.eax = x86::reg32(cpu.ebx * 4);
    // 004dd6bd  b9e80b8c00             -mov ecx, 0x8c0be8
    cpu.ecx = 9178088 /*0x8c0be8*/;
    // 004dd6c2  29d8                   -sub eax, ebx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 004dd6c4  01f9                   -add ecx, edi
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.edi));
    // 004dd6c6  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004dd6c9  81c7e8568c00           -add edi, 0x8c56e8
    (cpu.edi) += x86::reg32(x86::sreg32(9197288 /*0x8c56e8*/));
    // 004dd6cf  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004dd6d1  8d1407                 -lea edx, [edi + eax]
    cpu.edx = x86::reg32(cpu.edi + cpu.eax * 1);
    // 004dd6d4  a12ca28c00             -mov eax, dword ptr [0x8ca22c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(9216556) /* 0x8ca22c */);
    // 004dd6d9  83f802                 +cmp eax, 2
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
    // 004dd6dc  7551                   -jne 0x4dd72f
    if (!cpu.flags.zf)
    {
        goto L_0x004dd72f;
    }
    // 004dd6de  80790800               +cmp byte ptr [ecx + 8], 0
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(8) /* 0x8 */);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 004dd6e2  754b                   -jne 0x4dd72f
    if (!cpu.flags.zf)
    {
        goto L_0x004dd72f;
    }
    // 004dd6e4  89df                   -mov edi, ebx
    cpu.edi = cpu.ebx;
    // 004dd6e6  a184367d00             -mov eax, dword ptr [0x7d3684]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8205956) /* 0x7d3684 */);
    // 004dd6eb  c1e708                 -shl edi, 8
    cpu.edi <<= 8 /*0x8*/ % 32;
    // 004dd6ee  c1e008                 -shl eax, 8
    cpu.eax <<= 8 /*0x8*/ % 32;
    // 004dd6f1  01c7                   -add edi, eax
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.eax));
    // 004dd6f3  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 004dd6f5  e8864c0100             -call 0x4f2380
    cpu.esp -= 4;
    nfs3hp::load::fixedSine(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dd6fa  c1f804                 -sar eax, 4
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (4 /*0x4*/ % 32));
    // 004dd6fd  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004dd700  db45fc                 -fild dword ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 004dd703  dc0dc4975400           -fmul qword ptr [0x5497c4]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5543876) /* 0x5497c4 */)));
    // 004dd709  d902                   -fld dword ptr [edx]
    fp1 = x86::Float(app->getMemory<float>(cpu.edx));
    // 004dd70b  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 004dd70d  d8c1                   -fadd st(1)
    fp1 = fp::add(fp1, x86::Float(fp0));
    // 004dd70f  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 004dd711  ddd9                   -fstp st(1)
    fp0 = x86::Float(fp1);
    // 004dd713  d91a                   -fstp dword ptr [edx]
    app->getMemory<float>(cpu.edx) = float(fp0);
    // 004dd715  e8d64c0100             -call 0x4f23f0
    cpu.esp -= 4;
    nfs3hp::load::fixedCosine(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dd71a  c1f804                 +sar eax, 4
    {
        x86::reg8 tmp = 4 /*0x4*/ % 32;
        x86::reg32& op = cpu.eax;
        if (tmp)
        {
            cpu.flags.cf = 1 & (x86::sreg32(op) >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = 0;
            cpu.set_szp((op = x86::reg32(x86::sreg32(op) >> tmp)));
        }
    }
    // 004dd71d  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004dd720  db45fc                 +fild dword ptr [ebp - 4]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
    // 004dd723  dc0dc4975400           +fmul qword ptr [0x5497c4]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5543876) /* 0x5497c4 */)));
    // 004dd729  d84208                 +fadd dword ptr [edx + 8]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(8) /* 0x8 */)));
    // 004dd72c  d95a08                 +fstp dword ptr [edx + 8]
    app->getMemory<float>(cpu.edx + x86::reg32(8) /* 0x8 */) = float(fp0);
L_0x004dd72f:
    // 004dd72f  d902                   +fld dword ptr [edx]
    fp0 = x86::Float(app->getMemory<float>(cpu.edx));
    // 004dd731  d94204                 +fld dword ptr [edx + 4]
    fp1 = x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(4) /* 0x4 */));
    // 004dd734  d94208                 +fld dword ptr [edx + 8]
    fp2 = x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(8) /* 0x8 */));
    // 004dd737  d9ca                   +fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 004dd739  d86608                 +fsub dword ptr [esi + 8]
    fp2 = fp::sub(fp2, x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(8) /* 0x8 */)));
    // 004dd73c  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp2;
        fp2 = fp1;
        fp1 = tmp;
    }
    // 004dd73e  d8660c                 +fsub dword ptr [esi + 0xc]
    fp2 = fp::sub(fp2, x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(12) /* 0xc */)));
    // 004dd741  d9ca                   +fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 004dd743  d86610                 +fsub dword ptr [esi + 0x10]
    fp2 = fp::sub(fp2, x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(16) /* 0x10 */)));
    // 004dd746  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp2;
        fp2 = fp1;
        fp1 = tmp;
    }
    // 004dd748  d95dd4                 +fstp dword ptr [ebp - 0x2c]
    app->getMemory<float>(cpu.ebp + x86::reg32(-44) /* -0x2c */) = float(fp2);
    // 004dd74b  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 004dd74d  d95dd8                 +fstp dword ptr [ebp - 0x28]
    app->getMemory<float>(cpu.ebp + x86::reg32(-40) /* -0x28 */) = float(fp1);
    // 004dd750  d95ddc                 +fstp dword ptr [ebp - 0x24]
    app->getMemory<float>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = float(fp0);
    // 004dd753  d945d8                 +fld dword ptr [ebp - 0x28]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-40) /* -0x28 */));
    // 004dd756  d81d60a28c00           +fcomp dword ptr [0x8ca260]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(x86::reg32(9216608) /* 0x8ca260 */)));
    // 004dd75c  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004dd75e  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004dd75f  0f8262000000           -jb 0x4dd7c7
    if (cpu.flags.cf)
    {
        goto L_0x004dd7c7;
    }
    // 004dd765  d945d8                 +fld dword ptr [ebp - 0x28]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-40) /* -0x28 */));
    // 004dd768  d81d64a28c00           +fcomp dword ptr [0x8ca264]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(x86::reg32(9216612) /* 0x8ca264 */)));
    // 004dd76e  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004dd770  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004dd771  7754                   -ja 0x4dd7c7
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x004dd7c7;
    }
    // 004dd773  d9ee                   +fldz 
    fp0 = 0.0;
    // 004dd775  d85dd4                 +fcomp dword ptr [ebp - 0x2c]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-44) /* -0x2c */)));
    // 004dd778  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004dd77a  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004dd77b  760a                   -jbe 0x4dd787
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004dd787;
    }
    // 004dd77d  d945d4                 +fld dword ptr [ebp - 0x2c]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-44) /* -0x2c */));
    // 004dd780  d9e0                   +fchs 
    fp0 = -fp0;
    // 004dd782  d95df4                 +fstp dword ptr [ebp - 0xc]
    app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */) = float(fp0);
    // 004dd785  eb06                   -jmp 0x4dd78d
    goto L_0x004dd78d;
L_0x004dd787:
    // 004dd787  8b45d4                 -mov eax, dword ptr [ebp - 0x2c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44) /* -0x2c */);
    // 004dd78a  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
L_0x004dd78d:
    // 004dd78d  d945f4                 +fld dword ptr [ebp - 0xc]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */));
    // 004dd790  d81d5ca28c00           +fcomp dword ptr [0x8ca25c]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(x86::reg32(9216604) /* 0x8ca25c */)));
    // 004dd796  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004dd798  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004dd799  772c                   -ja 0x4dd7c7
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x004dd7c7;
    }
    // 004dd79b  d9ee                   +fldz 
    fp0 = 0.0;
    // 004dd79d  d85ddc                 +fcomp dword ptr [ebp - 0x24]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-36) /* -0x24 */)));
    // 004dd7a0  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004dd7a2  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004dd7a3  760a                   -jbe 0x4dd7af
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004dd7af;
    }
    // 004dd7a5  d945dc                 +fld dword ptr [ebp - 0x24]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-36) /* -0x24 */));
    // 004dd7a8  d9e0                   +fchs 
    fp0 = -fp0;
    // 004dd7aa  d95df0                 +fstp dword ptr [ebp - 0x10]
    app->getMemory<float>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = float(fp0);
    // 004dd7ad  eb06                   -jmp 0x4dd7b5
    goto L_0x004dd7b5;
L_0x004dd7af:
    // 004dd7af  8b45dc                 -mov eax, dword ptr [ebp - 0x24]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 004dd7b2  8945f0                 -mov dword ptr [ebp - 0x10], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.eax;
L_0x004dd7b5:
    // 004dd7b5  d945f0                 +fld dword ptr [ebp - 0x10]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-16) /* -0x10 */));
    // 004dd7b8  d81d5ca28c00           +fcomp dword ptr [0x8ca25c]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(x86::reg32(9216604) /* 0x8ca25c */)));
    // 004dd7be  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004dd7c0  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004dd7c1  0f8659010000           -jbe 0x4dd920
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004dd920;
    }
L_0x004dd7c7:
    // 004dd7c7  8a45ec                 -mov al, byte ptr [ebp - 0x14]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004dd7ca  884108                 -mov byte ptr [ecx + 8], al
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(8) /* 0x8 */) = cpu.al;
    // 004dd7cd  d945d4                 +fld dword ptr [ebp - 0x2c]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-44) /* -0x2c */));
    // 004dd7d0  d81d5ca28c00           +fcomp dword ptr [0x8ca25c]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(x86::reg32(9216604) /* 0x8ca25c */)));
    // 004dd7d6  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004dd7d8  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004dd7d9  7611                   -jbe 0x4dd7ec
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004dd7ec;
    }
    // 004dd7db  c6410900               -mov byte ptr [ecx + 9], 0
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(9) /* 0x9 */) = 0 /*0x0*/;
    // 004dd7df  d9055ca28c00           +fld dword ptr [0x8ca25c]
    fp0 = x86::Float(app->getMemory<float>(x86::reg32(9216604) /* 0x8ca25c */));
    // 004dd7e5  d9e0                   +fchs 
    fp0 = -fp0;
    // 004dd7e7  d95dd4                 +fstp dword ptr [ebp - 0x2c]
    app->getMemory<float>(cpu.ebp + x86::reg32(-44) /* -0x2c */) = float(fp0);
    // 004dd7ea  eb1c                   -jmp 0x4dd808
    goto L_0x004dd808;
L_0x004dd7ec:
    // 004dd7ec  d9055ca28c00           +fld dword ptr [0x8ca25c]
    fp0 = x86::Float(app->getMemory<float>(x86::reg32(9216604) /* 0x8ca25c */));
    // 004dd7f2  d9e0                   +fchs 
    fp0 = -fp0;
    // 004dd7f4  d85dd4                 +fcomp dword ptr [ebp - 0x2c]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-44) /* -0x2c */)));
    // 004dd7f7  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004dd7f9  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004dd7fa  763a                   -jbe 0x4dd836
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004dd836;
    }
    // 004dd7fc  c6410900               -mov byte ptr [ecx + 9], 0
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(9) /* 0x9 */) = 0 /*0x0*/;
    // 004dd800  a15ca28c00             -mov eax, dword ptr [0x8ca25c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(9216604) /* 0x8ca25c */);
    // 004dd805  8945d4                 -mov dword ptr [ebp - 0x2c], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44) /* -0x2c */) = cpu.eax;
L_0x004dd808:
    // 004dd808  e823da0000             -call 0x4eb230
    cpu.esp -= 4;
    nfs3hp::ea::randomFraction(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004dd80d  d80d68a28c00           +fmul dword ptr [0x8ca268]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(9216616) /* 0x8ca268 */)));
    // 004dd813  d80560a28c00           +fadd dword ptr [0x8ca260]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(x86::reg32(9216608) /* 0x8ca260 */)));
    // 004dd819  d95dd8                 +fstp dword ptr [ebp - 0x28]
    app->getMemory<float>(cpu.ebp + x86::reg32(-40) /* -0x28 */) = float(fp0);
    // 004dd81c  e80fda0000             -call 0x4eb230
    cpu.esp -= 4;
    nfs3hp::ea::randomFraction(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004dd821  d80d5ca28c00           +fmul dword ptr [0x8ca25c]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(9216604) /* 0x8ca25c */)));
    // 004dd827  d80dcc975400           +fmul dword ptr [0x5497cc]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(5543884) /* 0x5497cc */)));
    // 004dd82d  d8255ca28c00           +fsub dword ptr [0x8ca25c]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(x86::reg32(9216604) /* 0x8ca25c */)));
    // 004dd833  d95ddc                 +fstp dword ptr [ebp - 0x24]
    app->getMemory<float>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = float(fp0);
L_0x004dd836:
    // 004dd836  d945dc                 +fld dword ptr [ebp - 0x24]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-36) /* -0x24 */));
    // 004dd839  d81d5ca28c00           +fcomp dword ptr [0x8ca25c]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(x86::reg32(9216604) /* 0x8ca25c */)));
    // 004dd83f  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004dd841  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004dd842  7611                   -jbe 0x4dd855
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004dd855;
    }
    // 004dd844  c6410900               -mov byte ptr [ecx + 9], 0
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(9) /* 0x9 */) = 0 /*0x0*/;
    // 004dd848  d9055ca28c00           +fld dword ptr [0x8ca25c]
    fp0 = x86::Float(app->getMemory<float>(x86::reg32(9216604) /* 0x8ca25c */));
    // 004dd84e  d9e0                   +fchs 
    fp0 = -fp0;
    // 004dd850  d95ddc                 +fstp dword ptr [ebp - 0x24]
    app->getMemory<float>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = float(fp0);
    // 004dd853  eb1c                   -jmp 0x4dd871
    goto L_0x004dd871;
L_0x004dd855:
    // 004dd855  d9055ca28c00           +fld dword ptr [0x8ca25c]
    fp0 = x86::Float(app->getMemory<float>(x86::reg32(9216604) /* 0x8ca25c */));
    // 004dd85b  d9e0                   +fchs 
    fp0 = -fp0;
    // 004dd85d  d85ddc                 +fcomp dword ptr [ebp - 0x24]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-36) /* -0x24 */)));
    // 004dd860  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004dd862  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004dd863  763a                   -jbe 0x4dd89f
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004dd89f;
    }
    // 004dd865  c6410900               -mov byte ptr [ecx + 9], 0
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(9) /* 0x9 */) = 0 /*0x0*/;
    // 004dd869  a15ca28c00             -mov eax, dword ptr [0x8ca25c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(9216604) /* 0x8ca25c */);
    // 004dd86e  8945dc                 -mov dword ptr [ebp - 0x24], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = cpu.eax;
L_0x004dd871:
    // 004dd871  e8bad90000             -call 0x4eb230
    cpu.esp -= 4;
    nfs3hp::ea::randomFraction(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004dd876  d80d68a28c00           +fmul dword ptr [0x8ca268]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(9216616) /* 0x8ca268 */)));
    // 004dd87c  d80560a28c00           +fadd dword ptr [0x8ca260]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(x86::reg32(9216608) /* 0x8ca260 */)));
    // 004dd882  d95dd8                 +fstp dword ptr [ebp - 0x28]
    app->getMemory<float>(cpu.ebp + x86::reg32(-40) /* -0x28 */) = float(fp0);
    // 004dd885  e8a6d90000             -call 0x4eb230
    cpu.esp -= 4;
    nfs3hp::ea::randomFraction(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004dd88a  d80d5ca28c00           +fmul dword ptr [0x8ca25c]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(9216604) /* 0x8ca25c */)));
    // 004dd890  d80dcc975400           +fmul dword ptr [0x5497cc]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(5543884) /* 0x5497cc */)));
    // 004dd896  d8255ca28c00           +fsub dword ptr [0x8ca25c]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(x86::reg32(9216604) /* 0x8ca25c */)));
    // 004dd89c  d95ddc                 +fstp dword ptr [ebp - 0x24]
    app->getMemory<float>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = float(fp0);
L_0x004dd89f:
    // 004dd89f  d945d8                 +fld dword ptr [ebp - 0x28]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-40) /* -0x28 */));
    // 004dd8a2  d81d64a28c00           +fcomp dword ptr [0x8ca264]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(x86::reg32(9216612) /* 0x8ca264 */)));
    // 004dd8a8  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004dd8aa  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004dd8ab  760b                   -jbe 0x4dd8b8
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004dd8b8;
    }
    // 004dd8ad  c6410900               -mov byte ptr [ecx + 9], 0
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(9) /* 0x9 */) = 0 /*0x0*/;
    // 004dd8b1  a160a28c00             -mov eax, dword ptr [0x8ca260]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(9216608) /* 0x8ca260 */);
    // 004dd8b6  eb17                   -jmp 0x4dd8cf
    goto L_0x004dd8cf;
L_0x004dd8b8:
    // 004dd8b8  d945d8                 +fld dword ptr [ebp - 0x28]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-40) /* -0x28 */));
    // 004dd8bb  d81d60a28c00           +fcomp dword ptr [0x8ca260]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(x86::reg32(9216608) /* 0x8ca260 */)));
    // 004dd8c1  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004dd8c3  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004dd8c4  7340                   -jae 0x4dd906
    if (!cpu.flags.cf)
    {
        goto L_0x004dd906;
    }
    // 004dd8c6  c6410900               -mov byte ptr [ecx + 9], 0
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(9) /* 0x9 */) = 0 /*0x0*/;
    // 004dd8ca  a164a28c00             -mov eax, dword ptr [0x8ca264]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(9216612) /* 0x8ca264 */);
L_0x004dd8cf:
    // 004dd8cf  8945d8                 -mov dword ptr [ebp - 0x28], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-40) /* -0x28 */) = cpu.eax;
    // 004dd8d2  e859d90000             -call 0x4eb230
    cpu.esp -= 4;
    nfs3hp::ea::randomFraction(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004dd8d7  d80d5ca28c00           +fmul dword ptr [0x8ca25c]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(9216604) /* 0x8ca25c */)));
    // 004dd8dd  d80dcc975400           +fmul dword ptr [0x5497cc]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(5543884) /* 0x5497cc */)));
    // 004dd8e3  d8255ca28c00           +fsub dword ptr [0x8ca25c]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(x86::reg32(9216604) /* 0x8ca25c */)));
    // 004dd8e9  d95dd4                 +fstp dword ptr [ebp - 0x2c]
    app->getMemory<float>(cpu.ebp + x86::reg32(-44) /* -0x2c */) = float(fp0);
    // 004dd8ec  e83fd90000             -call 0x4eb230
    cpu.esp -= 4;
    nfs3hp::ea::randomFraction(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004dd8f1  d80d5ca28c00           +fmul dword ptr [0x8ca25c]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(9216604) /* 0x8ca25c */)));
    // 004dd8f7  d80dcc975400           +fmul dword ptr [0x5497cc]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(5543884) /* 0x5497cc */)));
    // 004dd8fd  d8255ca28c00           +fsub dword ptr [0x8ca25c]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(x86::reg32(9216604) /* 0x8ca25c */)));
    // 004dd903  d95ddc                 +fstp dword ptr [ebp - 0x24]
    app->getMemory<float>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = float(fp0);
L_0x004dd906:
    // 004dd906  d94608                 +fld dword ptr [esi + 8]
    fp0 = x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(8) /* 0x8 */));
    // 004dd909  d845d4                 +fadd dword ptr [ebp - 0x2c]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-44) /* -0x2c */)));
    // 004dd90c  d91a                   +fstp dword ptr [edx]
    app->getMemory<float>(cpu.edx) = float(fp0);
    // 004dd90e  d9460c                 +fld dword ptr [esi + 0xc]
    fp0 = x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(12) /* 0xc */));
    // 004dd911  d845d8                 +fadd dword ptr [ebp - 0x28]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-40) /* -0x28 */)));
    // 004dd914  d95a04                 +fstp dword ptr [edx + 4]
    app->getMemory<float>(cpu.edx + x86::reg32(4) /* 0x4 */) = float(fp0);
    // 004dd917  d94610                 +fld dword ptr [esi + 0x10]
    fp0 = x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(16) /* 0x10 */));
    // 004dd91a  d845dc                 +fadd dword ptr [ebp - 0x24]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-36) /* -0x24 */)));
    // 004dd91d  d95a08                 +fstp dword ptr [edx + 8]
    app->getMemory<float>(cpu.edx + x86::reg32(8) /* 0x8 */) = float(fp0);
L_0x004dd920:
    // 004dd920  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004dd921  e971fdffff             -jmp 0x4dd697
    goto L_0x004dd697;
L_0x004dd926:
    // 004dd926  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004dd928  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd929  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd92a  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd92b  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd92c  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dd92d  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_4dd930(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1, fp2, fp3, fp4, fp5;
  x86::reg16 fsw = 0;
    // 004dd930  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004dd931  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004dd932  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004dd933  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004dd934  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004dd935  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004dd936  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004dd938  83ec34                 -sub esp, 0x34
    (cpu.esp) -= x86::reg32(x86::sreg32(52 /*0x34*/));
    // 004dd93b  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004dd93d  8b15a0367d00           -mov edx, dword ptr [0x7d36a0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205984) /* 0x7d36a0 */);
    // 004dd943  8b7804                 -mov edi, dword ptr [eax + 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 004dd946  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004dd948  0f85e2010000           -jne 0x4ddb30
    if (!cpu.flags.zf)
    {
        goto L_0x004ddb30;
    }
    // 004dd94e  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
L_0x004dd950:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004dd950  3b0d78a28c00           +cmp ecx, dword ptr [0x8ca278]
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(9216632) /* 0x8ca278 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004dd956  0f8dd4010000           -jge 0x4ddb30
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004ddb30;
    }
    // 004dd95c  8d1cbd00000000         -lea ebx, [edi*4]
    cpu.ebx = x86::reg32(cpu.edi * 4);
    // 004dd963  29fb                   -sub ebx, edi
    (cpu.ebx) -= x86::reg32(x86::sreg32(cpu.edi));
    // 004dd965  c1e303                 -shl ebx, 3
    cpu.ebx <<= 3 /*0x3*/ % 32;
    // 004dd968  8d048d00000000         -lea eax, [ecx*4]
    cpu.eax = x86::reg32(cpu.ecx * 4);
    // 004dd96f  01fb                   -add ebx, edi
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.edi));
    // 004dd971  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004dd973  c1e304                 -shl ebx, 4
    cpu.ebx <<= 4 /*0x4*/ % 32;
    // 004dd976  83bc83c8088c0000       +cmp dword ptr [ebx + eax*4 + 0x8c08c8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(9177288) /* 0x8c08c8 */ + cpu.eax * 4);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004dd97e  0f85a6010000           -jne 0x4ddb2a
    if (!cpu.flags.zf)
    {
        goto L_0x004ddb2a;
    }
    // 004dd984  8d5dcc                 -lea ebx, [ebp - 0x34]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-52) /* -0x34 */);
    // 004dd987  8d451c                 -lea eax, [ebp + 0x1c]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(28) /* 0x1c */);
    // 004dd98a  8d5644                 -lea edx, [esi + 0x44]
    cpu.edx = x86::reg32(cpu.esi + x86::reg32(68) /* 0x44 */);
    // 004dd98d  d900                   -fld dword ptr [eax]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax));
    // 004dd98f  d80a                   -fmul dword ptr [edx]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.edx)));
    // 004dd991  d900                   -fld dword ptr [eax]
    fp1 = x86::Float(app->getMemory<float>(cpu.eax));
    // 004dd993  d84a04                 -fmul dword ptr [edx + 4]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(4) /* 0x4 */)));
    // 004dd996  d900                   -fld dword ptr [eax]
    fp2 = x86::Float(app->getMemory<float>(cpu.eax));
    // 004dd998  d84a08                 -fmul dword ptr [edx + 8]
    fp2 = fp::mul(fp2, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(8) /* 0x8 */)));
    // 004dd99b  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp2;
        fp2 = fp0;
        fp0 = tmp;
    }
    // 004dd99d  d94004                 -fld dword ptr [eax + 4]
    fp3 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 004dd9a0  d84a0c                 -fmul dword ptr [edx + 0xc]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(12) /* 0xc */)));
    // 004dd9a3  d94004                 -fld dword ptr [eax + 4]
    fp4 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 004dd9a6  d84a10                 -fmul dword ptr [edx + 0x10]
    fp4 = fp::mul(fp4, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(16) /* 0x10 */)));
    // 004dd9a9  d94004                 -fld dword ptr [eax + 4]
    fp5 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 004dd9ac  d84a14                 -fmul dword ptr [edx + 0x14]
    fp5 = fp::mul(fp5, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(20) /* 0x14 */)));
    // 004dd9af  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp5;
        fp5 = fp3;
        fp3 = tmp;
    }
    // 004dd9b1  dec3                   -faddp st(3)
    fp2 = fp::add(fp2, fp5);
    // 004dd9b3  dec3                   -faddp st(3)
    fp1 = fp::add(fp1, fp4);
    // 004dd9b5  dec3                   -faddp st(3)
    fp0 = fp::add(fp0, fp3);
    // 004dd9b7  d94008                 -fld dword ptr [eax + 8]
    fp3 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 004dd9ba  d84a18                 -fmul dword ptr [edx + 0x18]
    fp3 = fp::mul(fp3, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(24) /* 0x18 */)));
    // 004dd9bd  d94008                 -fld dword ptr [eax + 8]
    fp4 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 004dd9c0  d84a1c                 -fmul dword ptr [edx + 0x1c]
    fp4 = fp::mul(fp4, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(28) /* 0x1c */)));
    // 004dd9c3  d94008                 -fld dword ptr [eax + 8]
    fp5 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 004dd9c6  d84a20                 -fmul dword ptr [edx + 0x20]
    fp5 = fp::mul(fp5, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(32) /* 0x20 */)));
    // 004dd9c9  d9ca                   -fxch st(2)
    {
        x86::Float tmp = fp5;
        fp5 = fp3;
        fp3 = tmp;
    }
    // 004dd9cb  dec3                   -faddp st(3)
    fp2 = fp::add(fp2, fp5);
    // 004dd9cd  dec3                   -faddp st(3)
    fp1 = fp::add(fp1, fp4);
    // 004dd9cf  dec3                   -faddp st(3)
    fp0 = fp::add(fp0, fp3);
    // 004dd9d1  8d400c                 -lea eax, [eax + 0xc]
    cpu.eax = x86::reg32(cpu.eax + x86::reg32(12) /* 0xc */);
    // 004dd9d4  d91b                   -fstp dword ptr [ebx]
    app->getMemory<float>(cpu.ebx) = float(fp2);
    // 004dd9d6  d95b04                 -fstp dword ptr [ebx + 4]
    app->getMemory<float>(cpu.ebx + x86::reg32(4) /* 0x4 */) = float(fp1);
    // 004dd9d9  d95b08                 -fstp dword ptr [ebx + 8]
    app->getMemory<float>(cpu.ebx + x86::reg32(8) /* 0x8 */) = float(fp0);
    // 004dd9dc  d945cc                 -fld dword ptr [ebp - 0x34]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-52) /* -0x34 */));
    // 004dd9df  d84638                 -fadd dword ptr [esi + 0x38]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(56) /* 0x38 */)));
    // 004dd9e2  d945d0                 -fld dword ptr [ebp - 0x30]
    fp1 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-48) /* -0x30 */));
    // 004dd9e5  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 004dd9e7  d95dcc                 -fstp dword ptr [ebp - 0x34]
    app->getMemory<float>(cpu.ebp + x86::reg32(-52) /* -0x34 */) = float(fp1);
    // 004dd9ea  d8463c                 -fadd dword ptr [esi + 0x3c]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(60) /* 0x3c */)));
    // 004dd9ed  d945d4                 -fld dword ptr [ebp - 0x2c]
    fp1 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-44) /* -0x2c */));
    // 004dd9f0  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 004dd9f2  d95dd0                 -fstp dword ptr [ebp - 0x30]
    app->getMemory<float>(cpu.ebp + x86::reg32(-48) /* -0x30 */) = float(fp1);
    // 004dd9f5  d84640                 -fadd dword ptr [esi + 0x40]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(64) /* 0x40 */)));
    // 004dd9f8  d95dd4                 -fstp dword ptr [ebp - 0x2c]
    app->getMemory<float>(cpu.ebp + x86::reg32(-44) /* -0x2c */) = float(fp0);
    // 004dd9fb  8b5dd4                 -mov ebx, dword ptr [ebp - 0x2c]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44) /* -0x2c */);
    // 004dd9fe  f7c3ffffff7f           +test ebx, 0x7fffffff
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & 2147483647 /*0x7fffffff*/));
    // 004dda04  740c                   -je 0x4dda12
    if (cpu.flags.zf)
    {
        goto L_0x004dda12;
    }
    // 004dda06  d945d4                 +fld dword ptr [ebp - 0x2c]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-44) /* -0x2c */));
    // 004dda09  d9e8                   +fld1 
    fp1 = 1.0;
    // 004dda0b  def1                   +fdivrp st(1)
    fp0 = fp::div(fp1, x86::Float(fp0));
    // 004dda0d  dd5de0                 +fstp qword ptr [ebp - 0x20]
    app->getMemory<double>(cpu.ebp + x86::reg32(-32) /* -0x20 */) = double(fp0);
    // 004dda10  eb0d                   -jmp 0x4dda1f
    goto L_0x004dda1f;
L_0x004dda12:
    // 004dda12  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 004dda14  b8e0ffef40             -mov eax, 0x40efffe0
    cpu.eax = 1089470432 /*0x40efffe0*/;
    // 004dda19  8975e0                 -mov dword ptr [ebp - 0x20], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */) = cpu.esi;
    // 004dda1c  8945e4                 -mov dword ptr [ebp - 0x1c], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */) = cpu.eax;
L_0x004dda1f:
    // 004dda1f  a17ca28c00             -mov eax, dword ptr [0x8ca27c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(9216636) /* 0x8ca27c */);
    // 004dda24  dd45e0                 -fld qword ptr [ebp - 0x20]
    fp0 = x86::Float(app->getMemory<double>(cpu.ebp + x86::reg32(-32) /* -0x20 */));
    // 004dda27  c1f802                 -sar eax, 2
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (2 /*0x2*/ % 32));
    // 004dda2a  d95df4                 -fstp dword ptr [ebp - 0xc]
    app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */) = float(fp0);
    // 004dda2d  83f801                 +cmp eax, 1
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
    // 004dda30  7e04                   -jle 0x4dda36
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004dda36;
    }
    // 004dda32  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 004dda34  eb05                   -jmp 0x4dda3b
    goto L_0x004dda3b;
L_0x004dda36:
    // 004dda36  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
L_0x004dda3b:
    // 004dda3b  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004dda3d  e846330000             -call 0x4e0d88
    cpu.esp -= 4;
    nfs3hp::port::eaRandom(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dda42  f7f3                   -div ebx
    {
        x86::reg64 tmp = cpu.edx_eax;
        x86::reg32 d = cpu.ebx;
        cpu.edx_eax /= d;
        cpu.edx = tmp % d;
    }
    // 004dda44  d945f4                 -fld dword ptr [ebp - 0xc]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */));
    // 004dda47  d945cc                 -fld dword ptr [ebp - 0x34]
    fp1 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-52) /* -0x34 */));
    // 004dda4a  d8c9                   -fmul st(1)
    fp1 = fp::mul(fp1, x86::Float(fp0));
    // 004dda4c  d945d0                 -fld dword ptr [ebp - 0x30]
    fp2 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-48) /* -0x30 */));
    // 004dda4f  8d34bd00000000         -lea esi, [edi*4]
    cpu.esi = x86::reg32(cpu.edi * 4);
    // 004dda56  deca                   -fmulp st(2)
    fp0 = fp::mul(fp0, fp2);
    // 004dda58  29fe                   -sub esi, edi
    (cpu.esi) -= x86::reg32(x86::sreg32(cpu.edi));
    // 004dda5a  c1e603                 -shl esi, 3
    cpu.esi <<= 3 /*0x3*/ % 32;
    // 004dda5d  01fe                   -add esi, edi
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.edi));
    // 004dda5f  8d048d00000000         -lea eax, [ecx*4]
    cpu.eax = x86::reg32(cpu.ecx * 4);
    // 004dda66  8b1d7ca28c00           -mov ebx, dword ptr [0x8ca27c]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(9216636) /* 0x8ca27c */);
    // 004dda6c  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004dda6e  c1e604                 -shl esi, 4
    cpu.esi <<= 4 /*0x4*/ % 32;
    // 004dda71  29d3                   +sub ebx, edx
    {
        x86::reg32& tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004dda73  d99c86c0088c00         +fstp dword ptr [esi + eax*4 + 0x8c08c0]
    app->getMemory<float>(cpu.esi + x86::reg32(9177280) /* 0x8c08c0 */ + cpu.eax * 4) = float(fp1);
    // 004dda7a  d99c86c4088c00         +fstp dword ptr [esi + eax*4 + 0x8c08c4]
    app->getMemory<float>(cpu.esi + x86::reg32(9177284) /* 0x8c08c4 */ + cpu.eax * 4) = float(fp0);
    // 004dda81  899c86c8088c00         -mov dword ptr [esi + eax*4 + 0x8c08c8], ebx
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(9177288) /* 0x8c08c8 */ + cpu.eax * 4) = cpu.ebx;
    // 004dda88  e8a3d70000             -call 0x4eb230
    cpu.esp -= 4;
    nfs3hp::ea::randomFraction(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004dda8d  d80d80a28c00           +fmul dword ptr [0x8ca280]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(9216640) /* 0x8ca280 */)));
    // 004dda93  d90580a28c00           +fld dword ptr [0x8ca280]
    fp1 = x86::Float(app->getMemory<float>(x86::reg32(9216640) /* 0x8ca280 */));
    // 004dda99  dc0dd4975400           +fmul qword ptr [0x5497d4]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<double>(x86::reg32(5543892) /* 0x5497d4 */)));
    // 004dda9f  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 004ddaa1  d955fc                 +fst dword ptr [ebp - 4]
    app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = float(fp1);
    // 004ddaa4  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 004ddaa6  dd5dd8                 +fstp qword ptr [ebp - 0x28]
    app->getMemory<double>(cpu.ebp + x86::reg32(-40) /* -0x28 */) = double(fp1);
    // 004ddaa9  dc5dd8                 +fcomp qword ptr [ebp - 0x28]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<double>(cpu.ebp + x86::reg32(-40) /* -0x28 */)));
    // 004ddaac  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004ddaae  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004ddaaf  730e                   -jae 0x4ddabf
    if (!cpu.flags.cf)
    {
        goto L_0x004ddabf;
    }
    // 004ddab1  8b45d8                 -mov eax, dword ptr [ebp - 0x28]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-40) /* -0x28 */);
    // 004ddab4  8945e8                 -mov dword ptr [ebp - 0x18], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = cpu.eax;
    // 004ddab7  8b45dc                 -mov eax, dword ptr [ebp - 0x24]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 004ddaba  8945ec                 -mov dword ptr [ebp - 0x14], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.eax;
    // 004ddabd  eb21                   -jmp 0x4ddae0
    goto L_0x004ddae0;
L_0x004ddabf:
    // 004ddabf  d945fc                 +fld dword ptr [ebp - 4]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
    // 004ddac2  d81d80a28c00           +fcomp dword ptr [0x8ca280]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(x86::reg32(9216640) /* 0x8ca280 */)));
    // 004ddac8  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004ddaca  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004ddacb  7607                   -jbe 0x4ddad4
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004ddad4;
    }
    // 004ddacd  a180a28c00             -mov eax, dword ptr [0x8ca280]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(9216640) /* 0x8ca280 */);
    // 004ddad2  eb03                   -jmp 0x4ddad7
    goto L_0x004ddad7;
L_0x004ddad4:
    // 004ddad4  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
L_0x004ddad7:
    // 004ddad7  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 004ddada  d945f8                 -fld dword ptr [ebp - 8]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */));
    // 004ddadd  dd5de8                 -fstp qword ptr [ebp - 0x18]
    app->getMemory<double>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = double(fp0);
L_0x004ddae0:
    // 004ddae0  8d04bd00000000         -lea eax, [edi*4]
    cpu.eax = x86::reg32(cpu.edi * 4);
    // 004ddae7  29f8                   -sub eax, edi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edi));
    // 004ddae9  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 004ddaec  8d1407                 -lea edx, [edi + eax]
    cpu.edx = x86::reg32(cpu.edi + cpu.eax * 1);
    // 004ddaef  8d048d00000000         -lea eax, [ecx*4]
    cpu.eax = x86::reg32(cpu.ecx * 4);
    // 004ddaf6  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004ddaf8  c1e204                 -shl edx, 4
    cpu.edx <<= 4 /*0x4*/ % 32;
    // 004ddafb  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004ddafe  dd45e8                 -fld qword ptr [ebp - 0x18]
    fp0 = x86::Float(app->getMemory<double>(cpu.ebp + x86::reg32(-24) /* -0x18 */));
    // 004ddb01  01c2                   +add edx, eax
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
    // 004ddb03  d95df0                 +fstp dword ptr [ebp - 0x10]
    app->getMemory<float>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = float(fp0);
    // 004ddb06  8b45f0                 -mov eax, dword ptr [ebp - 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004ddb09  8982cc088c00           -mov dword ptr [edx + 0x8c08cc], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(9177292) /* 0x8c08cc */) = cpu.eax;
    // 004ddb0f  e81cd70000             -call 0x4eb230
    cpu.esp -= 4;
    nfs3hp::ea::randomFraction(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 004ddb14  d99ad0088c00           +fstp dword ptr [edx + 0x8c08d0]
    app->getMemory<float>(cpu.edx + x86::reg32(9177296) /* 0x8c08d0 */) = float(fp0);
    // 004ddb1a  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004ddb1f  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004ddb21  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ddb22  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ddb23  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ddb24  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ddb25  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ddb26  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ddb27  c20c00                 -ret 0xc
    cpu.esp += 4+12 /*0xc*/;
    return;
L_0x004ddb2a:
    // 004ddb2a  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004ddb2b  e920feffff             -jmp 0x4dd950
    goto L_0x004dd950;
L_0x004ddb30:
    // 004ddb30  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004ddb32  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004ddb34  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ddb35  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ddb36  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ddb37  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ddb38  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ddb39  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ddb3a  c20c00                 -ret 0xc
    cpu.esp += 4+12 /*0xc*/;
    return;
}

/* sub_4ddb40: native, nfs3hp::render::sparks */

/* sub_4de070: native, nfs3hp::render::smoke */

/* sub_4de700: native, nfs3hp::render::streaks */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_4dece0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004dece0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004dece1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004dece2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004dece3  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004dece4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004dece5  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004dece7  83ec14                 -sub esp, 0x14
    (cpu.esp) -= x86::reg32(x86::sreg32(20 /*0x14*/));
    // 004decea  89f1                   -mov ecx, esi
    cpu.ecx = cpu.esi;
    // 004decec  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 004decee  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004decf0  8d75f4                 -lea esi, [ebp - 0xc]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004decf3  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 004decf6  e875d1fdff             -call 0x4bbe70
    cpu.esp -= 4;
    sub_4bbe70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004decfb  833f00                 +cmp dword ptr [edi], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.edi);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004decfe  7509                   -jne 0x4ded09
    if (!cpu.flags.zf)
    {
        goto L_0x004ded09;
    }
    // 004ded00  833d2ca28c0000         +cmp dword ptr [0x8ca22c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(9216556) /* 0x8ca22c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ded07  750b                   -jne 0x4ded14
    if (!cpu.flags.zf)
    {
        goto L_0x004ded14;
    }
L_0x004ded09:
    // 004ded09  8d7dec                 -lea edi, [ebp - 0x14]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004ded0c  8d75f4                 -lea esi, [ebp - 0xc]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004ded0f  e9c3000000             -jmp 0x4dedd7
    goto L_0x004dedd7;
L_0x004ded14:
    // 004ded14  8b4704                 -mov eax, dword ptr [edi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(4) /* 0x4 */);
    // 004ded17  833c85f42f550000       +cmp dword ptr [eax*4 + 0x552ff4], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5582836) /* 0x552ff4 */ + cpu.eax * 4);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004ded1f  7407                   -je 0x4ded28
    if (cpu.flags.zf)
    {
        goto L_0x004ded28;
    }
    // 004ded21  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 004ded23  e808e5ffff             -call 0x4dd230
    cpu.esp -= 4;
    sub_4dd230(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004ded28:
    // 004ded28  8b4704                 -mov eax, dword ptr [edi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(4) /* 0x4 */);
    // 004ded2b  e8703ef4ff             -call 0x422ba0
    cpu.esp -= 4;
    nfs3hp::carstate::cameraCar(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004ded30  8b401c                 -mov eax, dword ptr [eax + 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(28) /* 0x1c */);
    // 004ded33  e898e6ffff             -call 0x4dd3d0
    cpu.esp -= 4;
    sub_4dd3d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004ded38  e8f3e5ffff             -call 0x4dd330
    cpu.esp -= 4;
    sub_4dd330(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004ded3d  8b1d30bc6f00           -mov ebx, dword ptr [0x6fbc30]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7322672) /* 0x6fbc30 */);
    // 004ded43  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004ded45  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004ded47  83fb01                 +cmp ebx, 1
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
    // 004ded4a  7509                   -jne 0x4ded55
    if (!cpu.flags.zf)
    {
        goto L_0x004ded55;
    }
    // 004ded4c  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004ded4f  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004ded51  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 004ded53  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
L_0x004ded55:
    // 004ded55  833d30bc6f0002         +cmp dword ptr [0x6fbc30], 2
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
    // 004ded5c  7511                   -jne 0x4ded6f
    if (!cpu.flags.zf)
    {
        goto L_0x004ded6f;
    }
    // 004ded5e  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 004ded60  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004ded62  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004ded65  c1e202                 +shl edx, 2
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
    // 004ded68  1bc2                   -sbb eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 004ded6a  c1f802                 -sar eax, 2
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (2 /*0x2*/ % 32));
    // 004ded6d  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
L_0x004ded6f:
    // 004ded6f  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 004ded71  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 004ded73  e868e8ffff             -call 0x4dd5e0
    cpu.esp -= 4;
    sub_4dd5e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004ded78  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 004ded7a  e821d0ffff             -call 0x4dbda0
    cpu.esp -= 4;
    sub_4dbda0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004ded7f  a12ca28c00             -mov eax, dword ptr [0x8ca22c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(9216556) /* 0x8ca22c */);
    // 004ded84  83f801                 +cmp eax, 1
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
    // 004ded87  7241                   -jb 0x4dedca
    if (cpu.flags.cf)
    {
        goto L_0x004dedca;
    }
    // 004ded89  7607                   -jbe 0x4ded92
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004ded92;
    }
    // 004ded8b  83f802                 +cmp eax, 2
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
    // 004ded8e  742a                   -je 0x4dedba
    if (cpu.flags.zf)
    {
        goto L_0x004dedba;
    }
    // 004ded90  eb38                   -jmp 0x4dedca
    goto L_0x004dedca;
L_0x004ded92:
    // 004ded92  8b4704                 -mov eax, dword ptr [edi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(4) /* 0x4 */);
    // 004ded95  e876e7ffff             -call 0x4dd510
    cpu.esp -= 4;
    sub_4dd510(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004ded9a  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 004ded9c  740f                   -je 0x4dedad
    if (cpu.flags.zf)
    {
        goto L_0x004dedad;
    }
    // 004ded9e  8d5dfc                 -lea ebx, [ebp - 4]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004deda1  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 004deda3  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 004deda5  8d75f4                 -lea esi, [ebp - 0xc]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004deda8  e853f9ffff             -call 0x4de700
    cpu.esp -= 4;
    nfs3hp::render::streaks(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004dedad:
    // 004dedad  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004dedb0  8b5704                 -mov edx, dword ptr [edi + 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(4) /* 0x4 */);
    // 004dedb3  e808e7ffff             -call 0x4dd4c0
    cpu.esp -= 4;
    sub_4dd4c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dedb8  eb10                   -jmp 0x4dedca
    goto L_0x004dedca;
L_0x004dedba:
    // 004dedba  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 004dedbc  740c                   -je 0x4dedca
    if (cpu.flags.zf)
    {
        goto L_0x004dedca;
    }
    // 004dedbe  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 004dedc0  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 004dedc2  8d75f4                 -lea esi, [ebp - 0xc]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004dedc5  e8a6f2ffff             -call 0x4de070
    cpu.esp -= 4;
    nfs3hp::render::smoke(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004dedca:
    // 004dedca  8d75f4                 -lea esi, [ebp - 0xc]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004dedcd  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 004dedcf  8d7dec                 -lea edi, [ebp - 0x14]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004dedd2  e809d0ffff             -call 0x4dbde0
    cpu.esp -= 4;
    nfs3hp::frontend::simSetView(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004dedd7:
    // 004dedd7  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004dedd8  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004dedd9  8d75ec                 -lea esi, [ebp - 0x14]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004deddc  89cf                   -mov edi, ecx
    cpu.edi = cpu.ecx;
    // 004dedde  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004deddf  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004dede0  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004dede2  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004dede4  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dede5  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dede6  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dede7  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dede8  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dede9  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_4dedf0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004dedf0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004dedf1  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004dedf2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004dedf3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004dedf5  83ec08                 -sub esp, 8
    (cpu.esp) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 004dedf8  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 004dedfa  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004dedfc  e81fd1fdff             -call 0x4bbf20
    cpu.esp -= 4;
    sub_4bbf20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dee01  833e00                 +cmp dword ptr [esi], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esi);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004dee04  7509                   -jne 0x4dee0f
    if (!cpu.flags.zf)
    {
        goto L_0x004dee0f;
    }
    // 004dee06  833d2ca28c0000         +cmp dword ptr [0x8ca22c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(9216556) /* 0x8ca22c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004dee0d  7505                   -jne 0x4dee14
    if (!cpu.flags.zf)
    {
        goto L_0x004dee14;
    }
L_0x004dee0f:
    // 004dee0f  8d75f8                 -lea esi, [ebp - 8]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004dee12  eb49                   -jmp 0x4dee5d
    goto L_0x004dee5d;
L_0x004dee14:
    // 004dee14  833d30bc6f0002         +cmp dword ptr [0x6fbc30], 2
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
    // 004dee1b  7517                   -jne 0x4dee34
    if (!cpu.flags.zf)
    {
        goto L_0x004dee34;
    }
    // 004dee1d  8d75f8                 -lea esi, [ebp - 8]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004dee20  e89bd1fdff             -call 0x4bbfc0
    cpu.esp -= 4;
    sub_4bbfc0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dee25  8d75f8                 -lea esi, [ebp - 8]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004dee28  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 004dee2a  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004dee2b  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004dee2c  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004dee2e  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004dee30  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dee31  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dee32  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dee33  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004dee34:
    // 004dee34  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004dee36  e865cfffff             -call 0x4dbda0
    cpu.esp -= 4;
    sub_4dbda0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dee3b  a12ca28c00             -mov eax, dword ptr [0x8ca22c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(9216556) /* 0x8ca22c */);
    // 004dee40  83f801                 +cmp eax, 1
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
    // 004dee43  720e                   -jb 0x4dee53
    if (cpu.flags.cf)
    {
        goto L_0x004dee53;
    }
    // 004dee45  7605                   -jbe 0x4dee4c
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004dee4c;
    }
    // 004dee47  83f802                 +cmp eax, 2
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
    // 004dee4a  7507                   -jne 0x4dee53
    if (!cpu.flags.zf)
    {
        goto L_0x004dee53;
    }
L_0x004dee4c:
    // 004dee4c  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004dee4e  e8edecffff             -call 0x4ddb40
    cpu.esp -= 4;
    nfs3hp::render::sparks(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004dee53:
    // 004dee53  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004dee55  8d75f8                 -lea esi, [ebp - 8]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004dee58  e883cfffff             -call 0x4dbde0
    cpu.esp -= 4;
    nfs3hp::frontend::simSetView(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004dee5d:
    // 004dee5d  e85ed1fdff             -call 0x4bbfc0
    cpu.esp -= 4;
    sub_4bbfc0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dee62  8d75f8                 -lea esi, [ebp - 8]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004dee65  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 004dee67  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004dee68  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004dee69  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004dee6b  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004dee6d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dee6e  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dee6f  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004dee70  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_4dee80(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004dee80  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004dee81  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004dee82  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004dee83  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004dee84  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004dee85  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004dee86  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004dee88  81ecd0000000           -sub esp, 0xd0
    (cpu.esp) -= x86::reg32(x86::sreg32(208 /*0xd0*/));
    // 004dee8e  81ed82000000           -sub ebp, 0x82
    (cpu.ebp) -= x86::reg32(x86::sreg32(130 /*0x82*/));
    // 004dee94  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
    // 004dee97  e8e4cb0000             -call 0x4eba80
    cpu.esp -= 4;
    sub_4eba80(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004dee9c  8b15c0a28c00           -mov edx, dword ptr [0x8ca2c0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(9216704) /* 0x8ca2c0 */);
    // 004deea2  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 004deea4  39d0                   +cmp eax, edx
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
    // 004deea6  0f848c000000           -je 0x4def38
    if (cpu.flags.zf)
    {
        goto L_0x004def38;
    }
    // 004deeac  e8cfcb0000             -call 0x4eba80
    cpu.esp -= 4;
    sub_4eba80(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004deeb1  b96c000000             -mov ecx, 0x6c
    cpu.ecx = 108 /*0x6c*/;
    // 004deeb6  bb07100000             -mov ebx, 0x1007
    cpu.ebx = 4103 /*0x1007*/;
    // 004deebb  a3c0a28c00             -mov dword ptr [0x8ca2c0], eax
    app->getMemory<x86::reg32>(x86::reg32(9216704) /* 0x8ca2c0 */) = cpu.eax;
    // 004deec0  e8cbcb0000             -call 0x4eba90
    cpu.esp -= 4;
    sub_4eba90(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004deec5  8b35d4a28c00           -mov esi, dword ptr [0x8ca2d4]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(9216724) /* 0x8ca2d4 */);
    // 004deecb  894db2                 -mov dword ptr [ebp - 0x4e], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-78) /* -0x4e */) = cpu.ecx;
    // 004deece  895db6                 -mov dword ptr [ebp - 0x4a], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-74) /* -0x4a */) = cpu.ebx;
    // 004deed1  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 004deed3  7509                   -jne 0x4deede
    if (!cpu.flags.zf)
    {
        goto L_0x004deede;
    }
    // 004deed5  c7451a40400000         -mov dword ptr [ebp + 0x1a], 0x4040
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(26) /* 0x1a */) = 16448 /*0x4040*/;
    // 004deedc  eb07                   -jmp 0x4deee5
    goto L_0x004deee5;
L_0x004deede:
    // 004deede  c7451a80400000         -mov dword ptr [ebp + 0x1a], 0x4080
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(26) /* 0x1a */) = 16512 /*0x4080*/;
L_0x004deee5:
    // 004deee5  b920000000             -mov ecx, 0x20
    cpu.ecx = 32 /*0x20*/;
    // 004deeea  bb04000000             -mov ebx, 4
    cpu.ebx = 4 /*0x4*/;
    // 004deeef  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 004deef1  be59555932             -mov esi, 0x32595559
    cpu.esi = 844715353 /*0x32595559*/;
    // 004deef6  8b15d0a28c00           -mov edx, dword ptr [0x8ca2d0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(9216720) /* 0x8ca2d0 */);
    // 004deefc  68c8a28c00             -push 0x8ca2c8
    app->getMemory<x86::reg32>(cpu.esp-4) = 9216712 /*0x8ca2c8*/;
    cpu.esp -= 4;
    // 004def01  8955be                 -mov dword ptr [ebp - 0x42], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-66) /* -0x42 */) = cpu.edx;
    // 004def04  894dfa                 -mov dword ptr [ebp - 6], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-6) /* -0x6 */) = cpu.ecx;
    // 004def07  8b15c4a28c00           -mov edx, dword ptr [0x8ca2c4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(9216708) /* 0x8ca2c4 */);
    // 004def0d  895dfe                 -mov dword ptr [ebp - 2], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-2) /* -0x2 */) = cpu.ebx;
    // 004def10  8955ba                 -mov dword ptr [ebp - 0x46], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-70) /* -0x46 */) = cpu.edx;
    // 004def13  8d55b2                 -lea edx, [ebp - 0x4e]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-78) /* -0x4e */);
    // 004def16  897502                 -mov dword ptr [ebp + 2], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(2) /* 0x2 */) = cpu.esi;
    // 004def19  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004def1a  8b08                   -mov ecx, dword ptr [eax]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax);
    // 004def1c  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004def1d  ff5118                 -call dword ptr [ecx + 0x18]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(24) /* 0x18 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004def20  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004def22  740f                   -je 0x4def33
    if (cpu.flags.zf)
    {
        goto L_0x004def33;
    }
L_0x004def24:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004def24  c705d4a28c0002000000   -mov dword ptr [0x8ca2d4], 2
    app->getMemory<x86::reg32>(x86::reg32(9216724) /* 0x8ca2d4 */) = 2 /*0x2*/;
    // 004def2e  e9a5020000             -jmp 0x4df1d8
    goto L_0x004df1d8;
L_0x004def33:
    // 004def33  be01000000             -mov esi, 1
    cpu.esi = 1 /*0x1*/;
L_0x004def38:
    // 004def38  e8e3bd0000             -call 0x4ead20
    cpu.esp -= 4;
    sub_4ead20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004def3d  833dd4a28c0001         +cmp dword ptr [0x8ca2d4], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(9216724) /* 0x8ca2d4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004def44  7561                   -jne 0x4defa7
    if (!cpu.flags.zf)
    {
        goto L_0x004defa7;
    }
    // 004def46  8b15d8a28c00           -mov edx, dword ptr [0x8ca2d8]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(9216728) /* 0x8ca2d8 */);
    // 004def4c  a17c715600             -mov eax, dword ptr [0x56717c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5665148) /* 0x56717c */);
    // 004def51  39d0                   +cmp eax, edx
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
    // 004def53  7452                   -je 0x4defa7
    if (cpu.flags.zf)
    {
        goto L_0x004defa7;
    }
    // 004def55  8b0dcca28c00           -mov ecx, dword ptr [0x8ca2cc]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(9216716) /* 0x8ca2cc */);
    // 004def5b  a3d8a28c00             -mov dword ptr [0x8ca2d8], eax
    app->getMemory<x86::reg32>(x86::reg32(9216728) /* 0x8ca2d8 */) = cpu.eax;
    // 004def60  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004def62  7507                   -jne 0x4def6b
    if (!cpu.flags.zf)
    {
        goto L_0x004def6b;
    }
    // 004def64  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004def69  eb05                   -jmp 0x4def70
    goto L_0x004def70;
L_0x004def6b:
    // 004def6b  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
L_0x004def70:
    // 004def70  8b0dc4a28c00           -mov ecx, dword ptr [0x8ca2c4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(9216708) /* 0x8ca2c4 */);
    // 004def76  0fafc8                 -imul ecx, eax
    cpu.ecx = x86::reg32(x86::sreg64(x86::sreg32(cpu.ecx)) * x86::sreg64(x86::sreg32(cpu.eax)));
    // 004def79  0faf05d0a28c00         -imul eax, dword ptr [0x8ca2d0]
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(9216720) /* 0x8ca2d0 */))));
    // 004def80  8b1ddca28c00           -mov ebx, dword ptr [0x8ca2dc]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(9216732) /* 0x8ca2dc */);
    // 004def86  6a01                   -push 1
    app->getMemory<x86::reg32>(cpu.esp-4) = 1 /*0x1*/;
    cpu.esp -= 4;
    // 004def88  01d9                   -add ecx, ebx
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.ebx));
    // 004def8a  8b1de0a28c00           -mov ebx, dword ptr [0x8ca2e0]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(9216736) /* 0x8ca2e0 */);
    // 004def90  8b15dca28c00           -mov edx, dword ptr [0x8ca2dc]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(9216732) /* 0x8ca2dc */);
    // 004def96  01c3                   -add ebx, eax
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004def98  a1e0a28c00             -mov eax, dword ptr [0x8ca2e0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(9216736) /* 0x8ca2e0 */);
    // 004def9d  be01000000             -mov esi, 1
    cpu.esi = 1 /*0x1*/;
    // 004defa2  e899e50100             -call 0x4fd540
    cpu.esp -= 4;
    sub_4fd540(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004defa7:
    // 004defa7  e8e4bd0000             -call 0x4ead90
    cpu.esp -= 4;
    sub_4ead90(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004defac  a1c8a28c00             -mov eax, dword ptr [0x8ca2c8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(9216712) /* 0x8ca2c8 */);
    // 004defb1  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004defb2  8b10                   -mov edx, dword ptr [eax]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax);
    // 004defb4  ff5260                 -call dword ptr [edx + 0x60]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(96) /* 0x60 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004defb7  3dc2017688             +cmp eax, 0x887601c2
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2289435074 /*0x887601c2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004defbc  752c                   -jne 0x4defea
    if (!cpu.flags.zf)
    {
        goto L_0x004defea;
    }
    // 004defbe  a1c8a28c00             -mov eax, dword ptr [0x8ca2c8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(9216712) /* 0x8ca2c8 */);
    // 004defc3  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004defc4  8b10                   -mov edx, dword ptr [eax]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax);
    // 004defc6  ff526c                 -call dword ptr [edx + 0x6c]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(108) /* 0x6c */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004defc9  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004defcb  7418                   -je 0x4defe5
    if (cpu.flags.zf)
    {
        goto L_0x004defe5;
    }
    // 004defcd  3d4b027688             +cmp eax, 0x8876024b
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2289435211 /*0x8876024b*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004defd2  0f854cffffff           -jne 0x4def24
    if (!cpu.flags.zf)
    {
        goto L_0x004def24;
    }
    // 004defd8  8da582000000           -lea esp, [ebp + 0x82]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(130) /* 0x82 */);
    // 004defde  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004defdf  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004defe0  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004defe1  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004defe2  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004defe3  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004defe4  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004defe5:
    // 004defe5  be01000000             -mov esi, 1
    cpu.esi = 1 /*0x1*/;
L_0x004defea:
    // 004defea  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 004defec  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 004defee  8d4db2                 -lea ecx, [ebp - 0x4e]
    cpu.ecx = x86::reg32(cpu.ebp + x86::reg32(-78) /* -0x4e */);
    // 004deff1  b86c000000             -mov eax, 0x6c
    cpu.eax = 108 /*0x6c*/;
    // 004deff6  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004deff7  8945b2                 -mov dword ptr [ebp - 0x4e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-78) /* -0x4e */) = cpu.eax;
    // 004deffa  a1c8a28c00             -mov eax, dword ptr [0x8ca2c8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(9216712) /* 0x8ca2c8 */);
    // 004defff  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 004df001  8b10                   -mov edx, dword ptr [eax]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax);
    // 004df003  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004df004  ff5264                 -call dword ptr [edx + 0x64]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(100) /* 0x64 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004df007  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004df009  742c                   -je 0x4df037
    if (cpu.flags.zf)
    {
        goto L_0x004df037;
    }
    // 004df00b  3dc2017688             +cmp eax, 0x887601c2
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2289435074 /*0x887601c2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004df010  750d                   -jne 0x4df01f
    if (!cpu.flags.zf)
    {
        goto L_0x004df01f;
    }
    // 004df012  8da582000000           -lea esp, [ebp + 0x82]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(130) /* 0x82 */);
    // 004df018  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df019  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df01a  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df01b  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df01c  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df01d  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df01e  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004df01f:
    // 004df01f  3d1c027688             +cmp eax, 0x8876021c
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2289435164 /*0x8876021c*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004df024  0f85fafeffff           -jne 0x4def24
    if (!cpu.flags.zf)
    {
        goto L_0x004def24;
    }
    // 004df02a  8da582000000           -lea esp, [ebp + 0x82]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(130) /* 0x82 */);
    // 004df030  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df031  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df032  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df033  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df034  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df035  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df036  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004df037:
    // 004df037  a1d0a28c00             -mov eax, dword ptr [0x8ca2d0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(9216720) /* 0x8ca2d0 */);
    // 004df03c  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004df03e  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004df041  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004df043  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 004df045  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 004df047  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
L_0x004df049:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004df049  3b0dc4a28c00           +cmp ecx, dword ptr [0x8ca2c4]
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(9216708) /* 0x8ca2c4 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004df04f  7d22                   -jge 0x4df073
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004df073;
    }
    // 004df051  8b45c2                 -mov eax, dword ptr [ebp - 0x3e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-62) /* -0x3e */);
    // 004df054  0fafc1                 -imul eax, ecx
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.ecx)));
    // 004df057  8b55d6                 -mov edx, dword ptr [ebp - 0x2a]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-42) /* -0x2a */);
    // 004df05a  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004df05c  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004df05e  0fafc7                 -imul eax, edi
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edi)));
    // 004df061  8b5d7e                 -mov ebx, dword ptr [ebp + 0x7e]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 004df064  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004df067  01d8                   +add eax, ebx
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebx));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004df069  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 004df06b  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004df06c  e81be90100             -call 0x4fd98c
    cpu.esp -= 4;
    sub_4fd98c(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004df071  ebd6                   -jmp 0x4df049
    goto L_0x004df049;
L_0x004df073:
    // 004df073  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 004df075  a1c8a28c00             -mov eax, dword ptr [0x8ca2c8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(9216712) /* 0x8ca2c8 */);
    // 004df07a  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004df07b  8b10                   -mov edx, dword ptr [eax]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax);
    // 004df07d  ff9280000000           -call dword ptr [edx + 0x80]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(128) /* 0x80 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004df083  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004df085  7418                   -je 0x4df09f
    if (cpu.flags.zf)
    {
        goto L_0x004df09f;
    }
    // 004df087  3dc2017688             +cmp eax, 0x887601c2
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2289435074 /*0x887601c2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004df08c  0f8592feffff           -jne 0x4def24
    if (!cpu.flags.zf)
    {
        goto L_0x004def24;
    }
    // 004df092  8da582000000           -lea esp, [ebp + 0x82]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(130) /* 0x82 */);
    // 004df098  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df099  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df09a  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df09b  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df09c  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df09d  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df09e  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004df09f:
    // 004df09f  833dd4a28c0000         +cmp dword ptr [0x8ca2d4], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(9216724) /* 0x8ca2d4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004df0a6  7408                   -je 0x4df0b0
    if (cpu.flags.zf)
    {
        goto L_0x004df0b0;
    }
    // 004df0a8  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 004df0aa  0f8428010000           -je 0x4df1d8
    if (cpu.flags.zf)
    {
        goto L_0x004df1d8;
    }
L_0x004df0b0:
    // 004df0b0  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004df0b2  89456a                 -mov dword ptr [ebp + 0x6a], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(106) /* 0x6a */) = cpu.eax;
    // 004df0b5  894566                 -mov dword ptr [ebp + 0x66], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(102) /* 0x66 */) = cpu.eax;
    // 004df0b8  a1d0a28c00             -mov eax, dword ptr [0x8ca2d0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(9216720) /* 0x8ca2d0 */);
    // 004df0bd  48                     -dec eax
    (cpu.eax)--;
    // 004df0be  89456e                 -mov dword ptr [ebp + 0x6e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(110) /* 0x6e */) = cpu.eax;
    // 004df0c1  a1c4a28c00             -mov eax, dword ptr [0x8ca2c4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(9216708) /* 0x8ca2c4 */);
    // 004df0c6  48                     -dec eax
    (cpu.eax)--;
    // 004df0c7  8b0dcca28c00           -mov ecx, dword ptr [0x8ca2cc]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(9216716) /* 0x8ca2cc */);
    // 004df0cd  894572                 -mov dword ptr [ebp + 0x72], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(114) /* 0x72 */) = cpu.eax;
    // 004df0d0  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004df0d2  7507                   -jne 0x4df0db
    if (!cpu.flags.zf)
    {
        goto L_0x004df0db;
    }
    // 004df0d4  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004df0d9  eb05                   -jmp 0x4df0e0
    goto L_0x004df0e0;
L_0x004df0db:
    // 004df0db  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
L_0x004df0e0:
    // 004df0e0  8b15e0a28c00           -mov edx, dword ptr [0x8ca2e0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(9216736) /* 0x8ca2e0 */);
    // 004df0e6  895556                 -mov dword ptr [ebp + 0x56], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(86) /* 0x56 */) = cpu.edx;
    // 004df0e9  8b15dca28c00           -mov edx, dword ptr [0x8ca2dc]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(9216732) /* 0x8ca2dc */);
    // 004df0ef  89555a                 -mov dword ptr [ebp + 0x5a], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(90) /* 0x5a */) = cpu.edx;
    // 004df0f2  8b15d0a28c00           -mov edx, dword ptr [0x8ca2d0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(9216720) /* 0x8ca2d0 */);
    // 004df0f8  0fafd0                 -imul edx, eax
    cpu.edx = x86::reg32(x86::sreg64(x86::sreg32(cpu.edx)) * x86::sreg64(x86::sreg32(cpu.eax)));
    // 004df0fb  0faf05c4a28c00         -imul eax, dword ptr [0x8ca2c4]
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(9216708) /* 0x8ca2c4 */))));
    // 004df102  8b4d56                 -mov ecx, dword ptr [ebp + 0x56]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(86) /* 0x56 */);
    // 004df105  01d1                   -add ecx, edx
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.edx));
    // 004df107  8b555a                 -mov edx, dword ptr [ebp + 0x5a]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(90) /* 0x5a */);
    // 004df10a  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004df10c  894d5e                 -mov dword ptr [ebp + 0x5e], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(94) /* 0x5e */) = cpu.ecx;
    // 004df10f  895562                 -mov dword ptr [ebp + 0x62], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(98) /* 0x62 */) = cpu.edx;
    // 004df112  e859c90000             -call 0x4eba70
    cpu.esp -= 4;
    nfs3hp::AcquireWindow(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004df117  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 004df119  8d4556                 -lea eax, [ebp + 0x56]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(86) /* 0x56 */);
    // 004df11c  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004df11d  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004df11e  2eff1500475300         -call dword ptr cs:[0x534700]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457664) /* 0x534700 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004df125  8d455e                 -lea eax, [ebp + 0x5e]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(94) /* 0x5e */);
    // 004df128  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004df129  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004df12a  2eff1500475300         -call dword ptr cs:[0x534700]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457664) /* 0x534700 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004df131  e85abd0000             -call 0x4eae90
    cpu.esp -= 4;
    sub_4eae90(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004df136  8b3dd4a28c00           -mov edi, dword ptr [0x8ca2d4]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(9216724) /* 0x8ca2d4 */);
    // 004df13c  8b5834                 -mov ebx, dword ptr [eax + 0x34]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(52) /* 0x34 */);
    // 004df13f  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 004df141  7537                   -jne 0x4df17a
    if (!cpu.flags.zf)
    {
        goto L_0x004df17a;
    }
    // 004df143  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004df144  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004df145  8d5566                 -lea edx, [ebp + 0x66]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(102) /* 0x66 */);
    // 004df148  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004df149  8b15c8a28c00           -mov edx, dword ptr [0x8ca2c8]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(9216712) /* 0x8ca2c8 */);
    // 004df14f  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004df150  8d5556                 -lea edx, [ebp + 0x56]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(86) /* 0x56 */);
    // 004df153  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004df154  8b03                   -mov eax, dword ptr [ebx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx);
    // 004df156  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004df157  ff5014                 -call dword ptr [eax + 0x14]
    cpu.ip = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(20) /* 0x14 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004df15a  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004df15c  0f8476000000           -je 0x4df1d8
    if (cpu.flags.zf)
    {
        goto L_0x004df1d8;
    }
    // 004df162  3dc2017688             +cmp eax, 0x887601c2
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2289435074 /*0x887601c2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004df167  0f85b7fdffff           -jne 0x4def24
    if (!cpu.flags.zf)
    {
        goto L_0x004def24;
    }
    // 004df16d  8da582000000           -lea esp, [ebp + 0x82]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(130) /* 0x82 */);
    // 004df173  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df174  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df175  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df176  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df177  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df178  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df179  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004df17a:
    // 004df17a  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 004df17c  745a                   -je 0x4df1d8
    if (cpu.flags.zf)
    {
        goto L_0x004df1d8;
    }
    // 004df17e  8d4576                 -lea eax, [ebp + 0x76]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(118) /* 0x76 */);
    // 004df181  be01000000             -mov esi, 1
    cpu.esi = 1 /*0x1*/;
    // 004df186  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004df187  897576                 -mov dword ptr [ebp + 0x76], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */) = cpu.esi;
    // 004df18a  89757a                 -mov dword ptr [ebp + 0x7a], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */) = cpu.esi;
    // 004df18d  6a04                   -push 4
    app->getMemory<x86::reg32>(cpu.esp-4) = 4 /*0x4*/;
    cpu.esp -= 4;
    // 004df18f  8b13                   -mov edx, dword ptr [ebx]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx);
    // 004df191  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004df192  ff5274                 -call dword ptr [edx + 0x74]
    cpu.ip = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(116) /* 0x74 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004df195  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004df197  7534                   -jne 0x4df1cd
    if (!cpu.flags.zf)
    {
        goto L_0x004df1cd;
    }
    // 004df199  c7451e38000000         -mov dword ptr [ebp + 0x1e], 0x38
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(30) /* 0x1e */) = 56 /*0x38*/;
    // 004df1a0  8d551e                 -lea edx, [ebp + 0x1e]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(30) /* 0x1e */);
    // 004df1a3  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004df1a4  6800480800             -push 0x84800
    app->getMemory<x86::reg32>(cpu.esp-4) = 542720 /*0x84800*/;
    cpu.esp -= 4;
    // 004df1a9  8d5556                 -lea edx, [ebp + 0x56]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(86) /* 0x56 */);
    // 004df1ac  a1c8a28c00             -mov eax, dword ptr [0x8ca2c8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(9216712) /* 0x8ca2c8 */);
    // 004df1b1  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004df1b2  89754e                 -mov dword ptr [ebp + 0x4e], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(78) /* 0x4e */) = cpu.esi;
    // 004df1b5  89753e                 -mov dword ptr [ebp + 0x3e], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(62) /* 0x3e */) = cpu.esi;
    // 004df1b8  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004df1b9  8d5566                 -lea edx, [ebp + 0x66]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(102) /* 0x66 */);
    // 004df1bc  897542                 -mov dword ptr [ebp + 0x42], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(66) /* 0x42 */) = cpu.esi;
    // 004df1bf  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004df1c0  8b08                   -mov ecx, dword ptr [eax]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax);
    // 004df1c2  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004df1c3  ff9184000000           -call dword ptr [ecx + 0x84]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(132) /* 0x84 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004df1c9  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004df1cb  740b                   -je 0x4df1d8
    if (cpu.flags.zf)
    {
        goto L_0x004df1d8;
    }
L_0x004df1cd:
    // 004df1cd  3dc2017688             +cmp eax, 0x887601c2
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2289435074 /*0x887601c2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004df1d2  0f854cfdffff           -jne 0x4def24
    if (!cpu.flags.zf)
    {
        goto L_0x004def24;
    }
L_0x004df1d8:
    // 004df1d8  8da582000000           -lea esp, [ebp + 0x82]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(130) /* 0x82 */);
    // 004df1de  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df1df  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df1e0  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df1e1  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df1e2  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df1e3  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df1e4  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_4df1f0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004df1f0  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004df1f1  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004df1f2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004df1f3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004df1f5  81ecd8020000           -sub esp, 0x2d8
    (cpu.esp) -= x86::reg32(x86::sreg32(728 /*0x2d8*/));
    // 004df1fb  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004df1fd  e80e010000             -call 0x4df310
    cpu.esp -= 4;
    sub_4df310(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004df202  8b4510                 -mov eax, dword ptr [ebp + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(16) /* 0x10 */);
    // 004df205  8935e0a28c00           -mov dword ptr [0x8ca2e0], esi
    app->getMemory<x86::reg32>(x86::reg32(9216736) /* 0x8ca2e0 */) = cpu.esi;
    // 004df20b  8915dca28c00           -mov dword ptr [0x8ca2dc], edx
    app->getMemory<x86::reg32>(x86::reg32(9216732) /* 0x8ca2dc */) = cpu.edx;
    // 004df211  891dd0a28c00           -mov dword ptr [0x8ca2d0], ebx
    app->getMemory<x86::reg32>(x86::reg32(9216720) /* 0x8ca2d0 */) = cpu.ebx;
    // 004df217  890dc4a28c00           -mov dword ptr [0x8ca2c4], ecx
    app->getMemory<x86::reg32>(x86::reg32(9216708) /* 0x8ca2c4 */) = cpu.ecx;
    // 004df21d  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 004df21f  ba02000000             -mov edx, 2
    cpu.edx = 2 /*0x2*/;
    // 004df224  8b4d14                 -mov ecx, dword ptr [ebp + 0x14]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    // 004df227  893dd8a28c00           -mov dword ptr [0x8ca2d8], edi
    app->getMemory<x86::reg32>(x86::reg32(9216728) /* 0x8ca2d8 */) = cpu.edi;
    // 004df22d  893dc0a28c00           -mov dword ptr [0x8ca2c0], edi
    app->getMemory<x86::reg32>(x86::reg32(9216704) /* 0x8ca2c0 */) = cpu.edi;
    // 004df233  a3cca28c00             -mov dword ptr [0x8ca2cc], eax
    app->getMemory<x86::reg32>(x86::reg32(9216716) /* 0x8ca2cc */) = cpu.eax;
    // 004df238  8915d4a28c00           -mov dword ptr [0x8ca2d4], edx
    app->getMemory<x86::reg32>(x86::reg32(9216724) /* 0x8ca2d4 */) = cpu.edx;
    // 004df23e  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004df240  0f8561000000           -jne 0x4df2a7
    if (!cpu.flags.zf)
    {
        goto L_0x004df2a7;
    }
    // 004df246  e845c80000             -call 0x4eba90
    cpu.esp -= 4;
    sub_4eba90(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004df24b  bb6c010000             -mov ebx, 0x16c
    cpu.ebx = 364 /*0x16c*/;
    // 004df250  8d9528fdffff           -lea edx, [ebp - 0x2d8]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-728) /* -0x2d8 */);
    // 004df256  899d94feffff           -mov dword ptr [ebp - 0x16c], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-364) /* -0x16c */) = cpu.ebx;
    // 004df25c  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004df25d  8d9594feffff           -lea edx, [ebp - 0x16c]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-364) /* -0x16c */);
    // 004df263  899d28fdffff           -mov dword ptr [ebp - 0x2d8], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-728) /* -0x2d8 */) = cpu.ebx;
    // 004df269  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004df26a  8b08                   -mov ecx, dword ptr [eax]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax);
    // 004df26c  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004df26d  ff512c                 -call dword ptr [ecx + 0x2c]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(44) /* 0x2c */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004df270  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004df272  7533                   -jne 0x4df2a7
    if (!cpu.flags.zf)
    {
        goto L_0x004df2a7;
    }
    // 004df274  8aa599feffff           -mov ah, byte ptr [ebp - 0x167]
    cpu.ah = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-359) /* -0x167 */);
    // 004df27a  f6c420                 +test ah, 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.ah & 32 /*0x20*/));
    // 004df27d  7411                   -je 0x4df290
    if (cpu.flags.zf)
    {
        goto L_0x004df290;
    }
    // 004df27f  f6c440                 +test ah, 0x40
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.ah & 64 /*0x40*/));
    // 004df282  740c                   -je 0x4df290
    if (cpu.flags.zf)
    {
        goto L_0x004df290;
    }
    // 004df284  c705d4a28c0001000000   -mov dword ptr [0x8ca2d4], 1
    app->getMemory<x86::reg32>(x86::reg32(9216724) /* 0x8ca2d4 */) = 1 /*0x1*/;
    // 004df28e  eb17                   -jmp 0x4df2a7
    goto L_0x004df2a7;
L_0x004df290:
    // 004df290  8ab599feffff           -mov dh, byte ptr [ebp - 0x167]
    cpu.dh = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-359) /* -0x167 */);
    // 004df296  f6c601                 +test dh, 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.dh & 1 /*0x1*/));
    // 004df299  740c                   -je 0x4df2a7
    if (cpu.flags.zf)
    {
        goto L_0x004df2a7;
    }
    // 004df29b  f6c602                 +test dh, 2
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.dh & 2 /*0x2*/));
    // 004df29e  7407                   -je 0x4df2a7
    if (cpu.flags.zf)
    {
        goto L_0x004df2a7;
    }
    // 004df2a0  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004df2a2  a3d4a28c00             -mov dword ptr [0x8ca2d4], eax
    app->getMemory<x86::reg32>(x86::reg32(9216724) /* 0x8ca2d4 */) = cpu.eax;
L_0x004df2a7:
    // 004df2a7  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004df2a9  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df2aa  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df2ab  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df2ac  c20800                 -ret 8
    cpu.esp += 4+8 /*0x8*/;
    return;
}

/* align: skip 0x90 */
void Application::sub_4df2b0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    if (madShowFrame(app, cpu.sync())) /* port: native (tools/apply_native_movie.py) */
    {
        cpu.reload();
        return;
    }
    // 004df2b0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004df2b1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004df2b2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004df2b3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004df2b4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004df2b5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004df2b6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004df2b8  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 004df2ba  833dd4a28c0002         +cmp dword ptr [0x8ca2d4], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(9216724) /* 0x8ca2d4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004df2c1  7405                   -je 0x4df2c8
    if (cpu.flags.zf)
    {
        goto L_0x004df2c8;
    }
    // 004df2c3  e8b8fbffff             -call 0x4dee80
    cpu.esp -= 4;
    sub_4dee80(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004df2c8:
    // 004df2c8  833dd4a28c0002         +cmp dword ptr [0x8ca2d4], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(9216724) /* 0x8ca2d4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004df2cf  752e                   -jne 0x4df2ff
    if (!cpu.flags.zf)
    {
        goto L_0x004df2ff;
    }
    // 004df2d1  e84aba0000             -call 0x4ead20
    cpu.esp -= 4;
    sub_4ead20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004df2d6  8b35cca28c00           -mov esi, dword ptr [0x8ca2cc]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(9216716) /* 0x8ca2cc */);
    // 004df2dc  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004df2dd  8b3dc4a28c00           -mov edi, dword ptr [0x8ca2c4]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(9216708) /* 0x8ca2c4 */);
    // 004df2e3  8b0dd0a28c00           -mov ecx, dword ptr [0x8ca2d0]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(9216720) /* 0x8ca2d0 */);
    // 004df2e9  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004df2ea  8b15dca28c00           -mov edx, dword ptr [0x8ca2dc]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(9216732) /* 0x8ca2dc */);
    // 004df2f0  a1e0a28c00             -mov eax, dword ptr [0x8ca2e0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(9216736) /* 0x8ca2e0 */);
    // 004df2f5  e896e90100             -call 0x4fdc90
    cpu.esp -= 4;
    sub_4fdc90(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004df2fa  e891ba0000             -call 0x4ead90
    cpu.esp -= 4;
    sub_4ead90(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004df2ff:
    // 004df2ff  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df300  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df301  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df302  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df303  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df304  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004df305  c3                     -ret 
    cpu.esp += 4;
    return;
}

}
