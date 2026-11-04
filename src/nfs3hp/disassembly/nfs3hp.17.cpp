#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_4652f0: native, nfs3hp::frontend::viewWidgetState */

/* sub_465320: native, nfs3hp::frontend::viewLaidOut */

/* sub_465360: native, nfs3hp::frontend::viewNoEntry */

/* sub_465370: native, nfs3hp::frontend::viewLoadCar */

/* sub_4654c0: native, nfs3hp::frontend::setSliderComponents */

/* sub_4654e0: native, nfs3hp::dead::stub_0x4654e0 */

/* sub_4654f0: native, nfs3hp::dead::stub_0x4654f0 */

/* sub_465500: native, nfs3hp::port::fields::drawListBox */

/* sub_465880: native, nfs3hp::port::fields::pointAtListBoxLine */

/* sub_4658f0: native, nfs3hp::port::fields::listBoxDown */

/* sub_465950: native, nfs3hp::port::fields::listBoxUp */

/* sub_4659a0: native, nfs3hp::port::fields::listBoxPartAt */

/* sub_465a10: native, nfs3hp::port::fields::listBoxArrowAt */

/* sub_465a30: native, nfs3hp::port::fields::listBoxKey */

/* sub_465ba0: native, nfs3hp::port::fields::layoutListBox */

/* sub_465c10: native, nfs3hp::port::fields::listBoxShown */

/* sub_465c60: native, nfs3hp::port::fields::listBoxHidden */

/* sub_465ca0: native, nfs3hp::port::fields::listBoxChooseAt */

/* sub_465d60: native, nfs3hp::frontend::clearMenuArtSlots */

/* sub_465d80: native, nfs3hp::frontend::loadMenuArtSlots */

/* sub_465eb0: native, nfs3hp::port::obj3d::flareQuad */

/* sub_466330: native, nfs3hp::port::obj3d::lightPlaced */

/* sub_4663a0: native, nfs3hp::port::obj3d::headlightGlare */

/* sub_466580: native, nfs3hp::port::obj3d::taillightGlare */

/* sub_466740: native, nfs3hp::port::obj3d::objectLights */

/* sub_4667d0: native, nfs3hp::port::obj3d::objectDragged */

/* sub_4668b0: native, nfs3hp::port::obj3d::objectBounds */

/* sub_466cf0: native, nfs3hp::port::obj3d::objectDraw */

/* sub_4675b0: native, nfs3hp::port::obj3d::objectKeyed */

/* sub_4675d0: native, nfs3hp::port::obj3d::objectPassed */

/* sub_467650: native, nfs3hp::port::obj3d::objectReleased */

/* sub_467700: native, nfs3hp::port::obj3d::objectOpened */

/* sub_467930: native, nfs3hp::port::obj3d::objectClosed */

/* sub_4679f0: native, nfs3hp::ea::findVivEntry */

/* sub_467ac0: native, nfs3hp::port::obj3d::resetLoader */

/* sub_467bc0: native, nfs3hp::port::obj3d::genericVivPath */

/* sub_467be0: native, nfs3hp::port::obj3d::genericModelName */

/* sub_467c00: native, nfs3hp::port::obj3d::trackVivPath */

/* sub_467c20: native, nfs3hp::port::obj3d::trackModelName */

/* sub_467c40: native, nfs3hp::port::obj3d::chooseFiles */

/* sub_467ea0: native, nfs3hp::port::obj3d::readPrehead */

/* sub_467ef0: native, nfs3hp::port::obj3d::readHead */

/* sub_468010: native, nfs3hp::port::obj3d::readModel */

/* sub_468170: native, nfs3hp::port::obj3d::readTexture */

/* sub_4682c0: native, nfs3hp::port::obj3d::readShapes */

/* sub_468410: native, nfs3hp::port::obj3d::takeModel */

/* sub_468580: native, nfs3hp::port::obj3d::fadeIn */

/* sub_4685e0: native, nfs3hp::port::obj3d::fadeOut */

/* sub_468670: native, nfs3hp::port::obj3d::wantedChanged */

/* sub_468860: native, nfs3hp::port::obj3d::runLoader */

/* sub_468da0: native, nfs3hp::port::obj3d::carColour */

/* sub_468f00: native, nfs3hp::port::obj3d::makeTexture */

/* sub_469020: native, nfs3hp::dead::stub_0x469020 */

/* sub_469080: native, nfs3hp::port::fields::sliderPartSizes */

/* sub_4692c0: native, nfs3hp::port::fields::sliderPartUnderPointer */

/* sub_469340: native, nfs3hp::port::fields::sliderThumbPlace */

/* sub_4693c0: native, nfs3hp::port::fields::snapToStep */

/* sub_469480: native, nfs3hp::port::fields::setSliderValue */

/* sub_469520: native, nfs3hp::frontend::sliderStoredValue */

/* sub_469530: native, nfs3hp::port::fields::setSliderStep */

/* sub_469580: native, nfs3hp::port::view::recordFloat */

/* sub_469590: native, nfs3hp::port::fields::dragSliderTo */

/* sub_469680: native, nfs3hp::port::fields::drawSlider */

/* sub_46a5f0: native, nfs3hp::port::fields::trackSlider */

/* sub_46a860: native, nfs3hp::port::fields::sliderKey */

/* sub_46a980: native, nfs3hp::port::fields::placeSliderParts */

/* sub_46ab10: native, nfs3hp::port::fields::setUpSlider */

/* sub_46abb0: native, nfs3hp::port::fields::sliderBox */

/* sub_46ac40: native, nfs3hp::port::fields::twoAxisThumbPlace */

/* sub_46add0: native, nfs3hp::port::fields::setTwoAxisValue */

/* sub_46afd0: native, nfs3hp::frontend::twoAxisValue */

/* sub_46afe0: native, nfs3hp::port::fields::setTwoAxisStep */

/* sub_46b030: native, nfs3hp::port::view::recordFloatAt */

/* sub_46b040: native, nfs3hp::port::fields::dragTwoAxisTo */

/* sub_46b3a0: native, nfs3hp::port::fields::drawTwoAxis */

/* sub_46c320: native, nfs3hp::port::fields::trackTwoAxis */

/* sub_46c4a0: native, nfs3hp::port::fields::twoAxisKey */

/* sub_46c640: native, nfs3hp::port::fields::placeTwoAxisParts */

/* sub_46c820: native, nfs3hp::port::fields::setUpTwoAxis */

/* sub_46c8b0: native, nfs3hp::port::fields::twoAxisBox */

/* sub_46c950: native, nfs3hp::port::fields::sliderSetValue */

/* sub_46c9b0: native, nfs3hp::frontend::sliderValue */

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_46ca10(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 0046ca10  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0046ca11  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0046ca12  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0046ca14  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0046ca16  668b5042               -mov dx, word ptr [eax + 0x42]
    cpu.dx = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(66) /* 0x42 */);
    // 0046ca1a  6683fa05               +cmp dx, 5
    {
        x86::reg16 tmp1 = cpu.dx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(5 /*0x5*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046ca1e  771e                   -ja 0x46ca3e
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x0046ca3e;
    }
    // 0046ca20  81e2ffff0000           +and edx, 0xffff
    cpu.clear_co();
    cpu.set_szp((cpu.edx &= x86::reg32(x86::sreg32(65535 /*0xffff*/))));
    // 0046ca26  ff2495f4c94600         -jmp dword ptr [edx*4 + 0x46c9f4]
    cpu.ip = app->getMemory<x86::reg32>(4639220 + cpu.edx * 4); goto dynamic_jump;
  case 0x0046ca2d:
    // 0046ca2d  8b5510                 -mov edx, dword ptr [ebp + 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(16) /* 0x10 */);
    // 0046ca30  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0046ca31  ff750c                 -push dword ptr [ebp + 0xc]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(12) /* 0xc */);
    cpu.esp -= 4;
    // 0046ca34  e8a7e5ffff             -call 0x46afe0
    cpu.esp -= 4;
    nfs3hp::port::fields::setTwoAxisStep(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046ca39  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046ca3a  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046ca3b  c20800                 -ret 8
    cpu.esp += 4+8 /*0x8*/;
    return;
  case 0x0046ca3e:
L_0x0046ca3e:
    // 0046ca3e  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0046ca40  ff750c                 -push dword ptr [ebp + 0xc]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(12) /* 0xc */);
    cpu.esp -= 4;
    // 0046ca43  e8e8caffff             -call 0x469530
    cpu.esp -= 4;
    nfs3hp::port::fields::setSliderStep(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
  [[fallthrough]];
  case 0x0046ca48:
    // 0046ca48  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046ca49  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046ca4a  c20800                 -ret 8
    cpu.esp += 4+8 /*0x8*/;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* sub_46ca70: native, nfs3hp::port::view::recordFloatByKind */

/* sub_46cac0: native, nfs3hp::frontend::sliderSetOnMove */

/* sub_46cae0: native, nfs3hp::port::fields::sliderDraw */

/* sub_46cb80: native, nfs3hp::port::fields::sliderWidgetKey */

/* sub_46cbe0: native, nfs3hp::port::fields::sliderWidgetLayout */

/* sub_46cc10: native, nfs3hp::port::fields::sliderWidgetHidden */

/* sub_46cc50: native, nfs3hp::port::fields::sliderWidgetShown */

/* sub_46cca0: native, nfs3hp::frontend::sliderSetColour */

/* sub_46cce0: native, nfs3hp::frontend::drawColumnSortMark */

/* sub_46cdc0: native, nfs3hp::frontend::resultsSortMark */

/* sub_46cdf0: native, nfs3hp::frontend::resultsPopupColumn */

/* sub_46cf30: native, nfs3hp::port::view::metricPopup */

/* sub_46d180: native, nfs3hp::frontend::resultsLapTimesPopup */

/* sub_46d3c0: native, nfs3hp::frontend::resultsTableDraw */

/* sub_46df90: native, nfs3hp::frontend::resultsTableShown */

/* sub_46e0b0: native, nfs3hp::frontend::resultsTablePass */

/* sub_46e0d0: native, nfs3hp::frontend::resultsTableLayout */

}
