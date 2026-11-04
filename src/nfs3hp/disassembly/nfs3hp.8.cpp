#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_432190: native, nfs3hp::dead::stub_0x432190 */

/* sub_432260: native, nfs3hp::dead::stub_0x432260 */

/* sub_432320: native, nfs3hp::dead::stub_0x432320 */

/* sub_432370: native, nfs3hp::graphics::clearSoftwareBlend */

/* sub_432390: native, nfs3hp::graphics::setSoftwareBlendColour */

/* sub_432440: native, nfs3hp::graphics::dead_0x432440 */

/* sub_432690: native, nfs3hp::collide::subdivisionDepth */

/* sub_432720: native, nfs3hp::render::subdivideQuad */

/* sub_433060: native, nfs3hp::render::quadSubdivided */

/* sub_4332b0: native, nfs3hp::render::quadRecords */

/* sub_433530: native, nfs3hp::collide::drawTexturedTriangles */

/* sub_433660: native, nfs3hp::collide::drawFlatTriangles */

/* sub_4336e0: native, nfs3hp::collide::drawTrianglesByZ */

/* sub_433840: native, nfs3hp::collide::drawQuadsByZ */

/* sub_4339d0: native, nfs3hp::collide::drawFlatTrianglesAgain */

/* sub_433a50: native, nfs3hp::vertices::triangleList */

/* sub_433bb0: native, nfs3hp::vertices::quadList */

/* sub_433d40: native, nfs3hp::graphics::drawColouredQuadRecord */

/* sub_433e30: native, nfs3hp::render::colouredTriangles */

/* sub_434120: native, nfs3hp::render::triangleRecords */

/* sub_434380: native, nfs3hp::vertices::submitPolygons */

/* sub_434799: native, nfs3hp::dead::stub_0x434799 */

/* sub_4347b0: native, nfs3hp::collide::drawColouredTriangles */

/* sub_4347d0: native, nfs3hp::graphics::drawLineRecord */

/* sub_434850: native, nfs3hp::collide::stateRecord */

/* sub_434870: native, nfs3hp::render::polygonDrawers */

/* sub_434950: native, nfs3hp::collide::drawRecords */

/* sub_434990: native, nfs3hp::collide::raceTextFrame */

/* sub_4349a0: native, nfs3hp::collide::setMirrored */

/* sub_4349d0: native, nfs3hp::collide::drawMirroredRecords */

/* sub_434a00: native, nfs3hp::graphics::endFrame */

/* sub_434a70: native, nfs3hp::collide::countFrame */

/* sub_434a80: native, nfs3hp::collide::raceViewDistance */

/* sub_434a90: native, nfs3hp::collide::raceFrameEnd */

/* sub_434ad0: native, nfs3hp::dead::stub_0x434ad0 */

/* sub_434ae0: native, nfs3hp::collide::raceFrameNothing */

/* sub_434af0: native, nfs3hp::collide::raceEndNothing */

/* sub_434b00: native, nfs3hp::collide::raceStartDrawing */

/* sub_434b10: native, nfs3hp::collide::raceStartNothing */

/* sub_434b20: native, nfs3hp::graphics::syncWindow */

/* sub_434b40: native, nfs3hp::graphics::flushAndFlip */

/* sub_434b60: native, nfs3hp::collide::drawPointer */

/* sub_434f20: native, nfs3hp::controls::startDirectInput */

/* sub_434fd0: native, nfs3hp::controls::stopDirectInput */

/* sub_4350a0: native, nfs3hp::controls::openMouse */

/* sub_435230: native, nfs3hp::frontend::placePointer */

/* sub_4352d0: native, nfs3hp::controls::setPointerBounds */

/* sub_435310: native, nfs3hp::controls::readMouse */

/* sub_435600: native, nfs3hp::controls::findJoysticks */

/* sub_435690: native, nfs3hp::controls::describeJoystick */

/* sub_4358f0: native, nfs3hp::controls::readJoystick */

/* sub_435ae0: native, nfs3hp::controls::readAxisZone */

/* sub_435b40: native, nfs3hp::controls::setAxisZone */

/* sub_435b60: native, nfs3hp::controls::resetForceFeedback */

/* sub_435bc0: native, nfs3hp::controls::setAutocenter */

/* sub_435c60: native, nfs3hp::controls::pauseEffects */

/* sub_435cc0: native, nfs3hp::controls::continueEffects */

/* sub_435d20: native, nfs3hp::controls::actuatorsOn */

/* sub_435d90: native, nfs3hp::controls::actuatorsOff */

/* sub_435df0: native, nfs3hp::controls::setGain */

/* sub_435f10: native, nfs3hp::controls::createEffect */

/* sub_4361e0: native, nfs3hp::controls::startEffect */

/* sub_436230: native, nfs3hp::controls::stopEffect */

/* sub_436270: native, nfs3hp::controls::stopEffects */

/* sub_4362f0: native, nfs3hp::controls::downloadEffect */

/* sub_436310: native, nfs3hp::controls::unloadEffect */

/* sub_436330: native, nfs3hp::controls::updateEffect */

/* sub_436610: native, nfs3hp::controls::setCooperativeLevel */

/* sub_4366e0: native, nfs3hp::controls::acquireDevice */

/* sub_436800: native, nfs3hp::controls::unacquireDevice */

/* sub_4368c0: native, nfs3hp::controls::acquireResult */

/* sub_4368f0: native, nfs3hp::controls::effectPlaying */

/* sub_436930: native, nfs3hp::controls::hideCursor */

/* sub_436980: native, nfs3hp::controls::joystickFound */

/* sub_436b10: native, nfs3hp::controls::axisProperty */

/* sub_436c40: native, nfs3hp::collide::openMessages */

/* sub_436cf0: native, nfs3hp::collide::closeMessages */

/* sub_436d40: native, nfs3hp::collide::clearAllMessages */

/* sub_436d60: native, nfs3hp::collide::clearMessages */

/* sub_436dc0: native, nfs3hp::collide::clearMessageLine */

/* sub_436e10: native, nfs3hp::collide::addMessageText */

/* sub_436e90: native, nfs3hp::collide::addMessageCharacter */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_436fc0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00436fc0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00436fc1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00436fc2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00436fc3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00436fc4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00436fc5  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00436fc7  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00436fc9  833d0c4f550000         +cmp dword ptr [0x554f0c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5590796) /* 0x554f0c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00436fd0  7507                   -jne 0x436fd9
    if (!cpu.flags.zf)
    {
        goto L_0x00436fd9;
    }
    // 00436fd2  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 00436fd4  e99b000000             -jmp 0x437074
    goto L_0x00437074;
L_0x00436fd9:
    // 00436fd9  69cb94040000           -imul ecx, ebx, 0x494
    cpu.ecx = x86::reg32(x86::sreg64(x86::sreg32(cpu.ebx)) * x86::sreg64(x86::sreg32(1172 /*0x494*/)));
    // 00436fdf  a1044f5500             -mov eax, dword ptr [0x554f04]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5590788) /* 0x554f04 */);
    // 00436fe4  83c028                 -add eax, 0x28
    (cpu.eax) += x86::reg32(x86::sreg32(40 /*0x28*/));
    // 00436fe7  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00436fe9  8b5108                 -mov edx, dword ptr [ecx + 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(8) /* 0x8 */);
L_0x00436fec:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00436fec  3b5104                 +cmp edx, dword ptr [ecx + 4]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(4) /* 0x4 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00436fef  7428                   -je 0x437019
    if (cpu.flags.zf)
    {
        goto L_0x00437019;
    }
    // 00436ff1  8d04d500000000         -lea eax, [edx*8]
    cpu.eax = x86::reg32(cpu.edx * 8);
    // 00436ff8  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00436ffa  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00436ffd  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00436fff  8b748114               -mov esi, dword ptr [ecx + eax*4 + 0x14]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(20) /* 0x14 */ + cpu.eax * 4);
    // 00437003  3b748110               +cmp esi, dword ptr [ecx + eax*4 + 0x10]
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(16) /* 0x10 */ + cpu.eax * 4)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00437007  7510                   -jne 0x437019
    if (!cpu.flags.zf)
    {
        goto L_0x00437019;
    }
    // 00437009  4a                     -dec edx
    (cpu.edx)--;
    // 0043700a  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0043700c  7dde                   -jge 0x436fec
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00436fec;
    }
    // 0043700e  a1044f5500             -mov eax, dword ptr [0x554f04]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5590788) /* 0x554f04 */);
    // 00437013  8b5004                 -mov edx, dword ptr [eax + 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 00437016  4a                     +dec edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 00437017  ebd3                   -jmp 0x436fec
    goto L_0x00436fec;
L_0x00437019:
    // 00437019  8d04d500000000         -lea eax, [edx*8]
    cpu.eax = x86::reg32(cpu.edx * 8);
    // 00437020  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00437022  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00437025  8d3402                 -lea esi, [edx + eax]
    cpu.esi = x86::reg32(cpu.edx + cpu.eax * 1);
    // 00437028  c1e602                 -shl esi, 2
    cpu.esi <<= 2 /*0x2*/ % 32;
    // 0043702b  8d410c                 -lea eax, [ecx + 0xc]
    cpu.eax = x86::reg32(cpu.ecx + x86::reg32(12) /* 0xc */);
    // 0043702e  01f0                   -add eax, esi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.esi));
    // 00437030  8b7008                 -mov esi, dword ptr [eax + 8]
    cpu.esi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 00437033  3b7004                 +cmp esi, dword ptr [eax + 4]
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00437036  7508                   -jne 0x437040
    if (!cpu.flags.zf)
    {
        goto L_0x00437040;
    }
    // 00437038  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0043703a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043703b  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043703c  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043703d  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043703e  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043703f  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00437040:
    // 00437040  4e                     -dec esi
    (cpu.esi)--;
    // 00437041  897008                 -mov dword ptr [eax + 8], esi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */) = cpu.esi;
    // 00437044  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 00437046  7d0b                   -jge 0x437053
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00437053;
    }
    // 00437048  8b35044f5500           -mov esi, dword ptr [0x554f04]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(5590788) /* 0x554f04 */);
    // 0043704e  8b36                   -mov esi, dword ptr [esi]
    cpu.esi = app->getMemory<x86::reg32>(cpu.esi);
    // 00437050  897008                 -mov dword ptr [eax + 8], esi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */) = cpu.esi;
L_0x00437053:
    // 00437053  895108                 -mov dword ptr [ecx + 8], edx
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(8) /* 0x8 */) = cpu.edx;
    // 00437056  034008                 -add eax, dword ptr [eax + 8]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */)));
    // 00437059  c6400c00               -mov byte ptr [eax + 0xc], 0
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(12) /* 0xc */) = 0 /*0x0*/;
    // 0043705d  a1044f5500             -mov eax, dword ptr [0x554f04]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5590788) /* 0x554f04 */);
    // 00437062  83782000               +cmp dword ptr [eax + 0x20], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(32) /* 0x20 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00437066  7407                   -je 0x43706f
    if (cpu.flags.zf)
    {
        goto L_0x0043706f;
    }
    // 00437068  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0043706a  e831010000             -call 0x4371a0
    cpu.esp -= 4;
    nfs3hp::collide::messagesScrolled(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0043706f:
    // 0043706f  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
L_0x00437074:
    // 00437074  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00437075  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00437076  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00437077  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00437078  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00437079  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_437080: native, nfs3hp::collide::copyMessageLines */

/* sub_4371a0: native, nfs3hp::collide::messagesScrolled */

/* sub_4371c0: native, nfs3hp::collide::messagesNothing */

/* sub_4371d0: native, nfs3hp::collide::seedHash */

/* sub_437250: native, nfs3hp::frontend::feedSeparate */

/* sub_437300: native, nfs3hp::frontend::feedTimeButtonKey */

/* sub_437350: native, nfs3hp::frontend::feedTimeListKey */

/* sub_437390: native, nfs3hp::frontend::feedPositionButtonKey */

/* sub_4373e0: native, nfs3hp::frontend::feedPositionListKey */

/* sub_437420: native, nfs3hp::frontend::feedSpeedButtonKey */

/* sub_437470: native, nfs3hp::frontend::feedSpeedListKey */

/* sub_4374b0: native, nfs3hp::frontend::feedOnEnter */

/* sub_437590: native, nfs3hp::frontend::feedOnExit */

/* sub_437600: native, nfs3hp::frontend::funkeysOnEnter */

/* sub_4377c0: native, nfs3hp::frontend::funkeysOnExit */

/* sub_4377d0: native, nfs3hp::frontend::hardkeysOnEnter */

/* sub_437990: native, nfs3hp::frontend::audioMenuMusicMoved */

}
