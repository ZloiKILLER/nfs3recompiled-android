#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_44f670: native, nfs3hp::frontend::recordsSelect */

/* sub_44f6e0: native, nfs3hp::frontend::recordsCompare */

/* sub_44f830: native, nfs3hp::frontend::recordsSortBy */

/* sub_44f8d0: native, nfs3hp::frontend::recordsCount */

/* sub_44f910: native, nfs3hp::frontend::recordByRank */

/* sub_44f960: native, nfs3hp::frontend::recordsTimeQualifies */

/* sub_44f9b0: native, nfs3hp::frontend::recordsClearMarks */

/* sub_44fa10: native, nfs3hp::frontend::recordsInsert */

/* sub_44fa90: native, nfs3hp::frontend::recordsRead */

/* sub_44fb20: native, nfs3hp::frontend::recordsHolderMark */

/* sub_44fb30: native, nfs3hp::frontend::recordsSetHolder */

/* sub_44fbc0: native, nfs3hp::frontend::recordsClear */

/* sub_44fc00: native, nfs3hp::frontend::recordsSheetSortMark */

/* sub_44fc70: native, nfs3hp::frontend::recordsSheetDraw */

/* sub_450770: native, nfs3hp::frontend::recordsSheetLoad */

/* sub_450950: native, nfs3hp::frontend::recordsSheetSave */

/* sub_450970: native, nfs3hp::frontend::recordsSheetLayout */

/* sub_450a90: native, nfs3hp::frontend::recordsSheetKey */

/* sub_450e30: native, nfs3hp::frontend::recordsSheetIdle */

/* sub_450e50: native, nfs3hp::frontend::recordsDeleteAnswered */

/* sub_450eb0: native, nfs3hp::frontend::recordsDeleteClicked */

/* sub_450f20: native, nfs3hp::frontend::recordsOnFrame */

/* sub_450f70: native, nfs3hp::frontend::recordsOnEnter */

/* sub_450fd0: native, nfs3hp::frontend::recordsOnExit */

/* sub_450fe0: native, nfs3hp::frontend::recordsEnterResult */

/* sub_4512f0: native, nfs3hp::frontend::recordsEnterPlayers */

/* sub_4513c0: native, nfs3hp::port::text::setTextColour */

/* sub_4513e0: native, nfs3hp::port::fields::setCursorColour */

/* sub_451400: native, nfs3hp::port::text::toScreenX */

/* sub_451440: native, nfs3hp::port::text::toScreenY */

/* sub_451480: native, nfs3hp::port::text::screenToMenuX */

/* sub_4514c0: native, nfs3hp::port::text::screenToMenuY */

/* sub_451500: native, nfs3hp::port::text::readMenuMouse */

/* sub_451530: native, nfs3hp::port::text::dimBackground */

/* sub_451590: native, nfs3hp::port::text::drawTextAt */

/* sub_4515e0: native, nfs3hp::port::fields::drawFieldText */

/* sub_451640: native, nfs3hp::dead::stub_0x451640 */

/* sub_451670: native, nfs3hp::dead::stub_0x451670 */

/* align: skip 0x8b 0xc0 */
void Application::sub_4516a0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004516a0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004516a1  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004516a2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004516a3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004516a5  81ec0c010000           -sub esp, 0x10c
    (cpu.esp) -= x86::reg32(x86::sreg32(268 /*0x10c*/));
    // 004516ab  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004516ad  bafa000000             -mov edx, 0xfa
    cpu.edx = 250 /*0xfa*/;
    // 004516b2  8d85f4feffff           -lea eax, [ebp - 0x10c]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-268) /* -0x10c */);
    // 004516b8  e873f00900             -call 0x4f0730
    cpu.esp -= 4;
    nfs3hp::watcom::getcwd(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004516bd  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004516bf  8a85f4feffff           -mov al, byte ptr [ebp - 0x10c]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-268) /* -0x10c */);
    // 004516c5  8d55f0                 -lea edx, [ebp - 0x10]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004516c8  83e840                 -sub eax, 0x40
    (cpu.eax) -= x86::reg32(x86::sreg32(64 /*0x40*/));
    // 004516cb  e800f10900             -call 0x4f07d0
    cpu.esp -= 4;
    nfs3hp::watcom::getdiskfree(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004516d0  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004516d2  7404                   -je 0x4516d8
    if (cpu.flags.zf)
    {
        goto L_0x004516d8;
    }
    // 004516d4  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004516d6  eb1c                   -jmp 0x4516f4
    goto L_0x004516f4;
L_0x004516d8:
    // 004516d8  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004516db  0faf45fc               -imul eax, dword ptr [ebp - 4]
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */))));
    // 004516df  0faf45f8               -imul eax, dword ptr [ebp - 8]
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */))));
    // 004516e3  39c8                   +cmp eax, ecx
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
    // 004516e5  7d08                   -jge 0x4516ef
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004516ef;
    }
    // 004516e7  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004516e9  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004516eb  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004516ec  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004516ed  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004516ee  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004516ef:
    // 004516ef  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
L_0x004516f4:
    // 004516f4  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004516f6  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004516f7  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004516f8  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004516f9  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_451700: native, nfs3hp::port::text::joystickKey */

/* sub_451715: native, nfs3hp::dead::stub_0x451715 */

/* sub_4517d0: native, nfs3hp::dead::stub_0x4517d0 */

/* sub_451810: native, nfs3hp::port::text::captureKeys */

/* sub_451840: native, nfs3hp::port::text::scanCodeOfKey */

/* sub_4518c0: native, nfs3hp::port::text::keyOfScanCode */

/* sub_451940: native, nfs3hp::frontend::connectFlushKeys */

/* sub_451960: native, nfs3hp::port::text::menuKey */

/* sub_451b20: native, nfs3hp::port::text::resetMenuKey */

/* sub_451b50: native, nfs3hp::port::text::nextWord */

/* sub_451cd0: native, nfs3hp::port::text::drawWrappedText */

/* sub_451fc0: native, nfs3hp::port::text::drawTextBox */

/* sub_451ff0: native, nfs3hp::port::text::measureTextBox */

/* sub_452020: native, nfs3hp::port::text::textWidth */

/* sub_452090: native, nfs3hp::frontend::textFit */

/* sub_452100: native, nfs3hp::port::text::selectFont */

/* sub_452110: native, nfs3hp::port::text::menusEnd */

/* sub_452120: native, nfs3hp::frontend::textDrawFitted */

/* sub_452190: native, nfs3hp::port::text::drawMenuText */

/* sub_452220: native, nfs3hp::port::fields::drawFieldLine */

/* sub_4522c0: native, nfs3hp::port::fields::drawSpacedText */

/* sub_452410: native, nfs3hp::frontend::menuCaptureScreen */

/* sub_452420: native, nfs3hp::frontend::cheatAdvance */

/* sub_4524e0: native, nfs3hp::frontend::cheatKey */

/* sub_452910: native, nfs3hp::dead::stub_0x452910 */

/* sub_4529a0: native, nfs3hp::port::menus::appendBackslash */

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_452a00(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00452a00  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00452a01  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00452a02  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00452a03  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00452a04  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00452a05  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00452a07  81ec08020000           -sub esp, 0x208
    (cpu.esp) -= x86::reg32(x86::sreg32(520 /*0x208*/));
    // 00452a0d  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00452a0f  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00452a11  ba04965300             -mov edx, 0x539604
    cpu.edx = 5477892 /*0x539604*/;
    // 00452a16  e8cdb50900             -call 0x4edfe8
    cpu.esp -= 4;
    nfs3hp::watcom::fopen(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00452a1b  ba08965300             -mov edx, 0x539608
    cpu.edx = 5477896 /*0x539608*/;
    // 00452a20  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00452a22  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 00452a24  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00452a26  e8bdb50900             -call 0x4edfe8
    cpu.esp -= 4;
    nfs3hp::watcom::fopen(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00452a2b  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 00452a2e  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 00452a30  7404                   -je 0x452a36
    if (cpu.flags.zf)
    {
        goto L_0x00452a36;
    }
    // 00452a32  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00452a34  7507                   -jne 0x452a3d
    if (!cpu.flags.zf)
    {
        goto L_0x00452a3d;
    }
L_0x00452a36:
    // 00452a36  beffffffff             -mov esi, 0xffffffff
    cpu.esi = 4294967295 /*0xffffffff*/;
    // 00452a3b  eb54                   -jmp 0x452a91
    goto L_0x00452a91;
L_0x00452a3d:
    // 00452a3d  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00452a3f  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
L_0x00452a42:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00452a42  bb00020000             -mov ebx, 0x200
    cpu.ebx = 512 /*0x200*/;
    // 00452a47  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 00452a4c  8d85f8fdffff           -lea eax, [ebp - 0x208]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-520) /* -0x208 */);
    // 00452a52  89f9                   -mov ecx, edi
    cpu.ecx = cpu.edi;
    // 00452a54  e8a7670900             -call 0x4e9200
    cpu.esp -= 4;
    nfs3hp::watcom::fread(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00452a59  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00452a5b  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00452a5d  7420                   -je 0x452a7f
    if (cpu.flags.zf)
    {
        goto L_0x00452a7f;
    }
    // 00452a5f  8b4df8                 -mov ecx, dword ptr [ebp - 8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00452a62  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 00452a67  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 00452a69  8d85f8fdffff           -lea eax, [ebp - 0x208]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-520) /* -0x208 */);
    // 00452a6f  e85cc10900             -call 0x4eebd0
    cpu.esp -= 4;
    nfs3hp::watcom::fwrite(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00452a74  39c6                   +cmp esi, eax
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
    // 00452a76  74ca                   -je 0x452a42
    if (cpu.flags.zf)
    {
        goto L_0x00452a42;
    }
    // 00452a78  c745fc01000000         -mov dword ptr [ebp - 4], 1
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = 1 /*0x1*/;
L_0x00452a7f:
    // 00452a7f  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 00452a81  e87ab60900             -call 0x4ee100
    cpu.esp -= 4;
    nfs3hp::watcom::fclose(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00452a86  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 00452a89  8b75fc                 -mov esi, dword ptr [ebp - 4]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00452a8c  e86fb60900             -call 0x4ee100
    cpu.esp -= 4;
    nfs3hp::watcom::fclose(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00452a91:
    // 00452a91  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00452a93  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00452a95  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00452a96  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00452a97  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00452a98  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00452a99  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00452a9a  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_452ac0: native, nfs3hp::frontend::setupRaceAnswered */

/* sub_452bd0: native, nfs3hp::frontend::setupAskRace */

/* sub_452d40: native, nfs3hp::frontend::setupTwoPlayersClicked */

/* sub_452d80: native, nfs3hp::frontend::setupWebSiteRefused */

/* sub_452db0: native, nfs3hp::frontend::setupIsCurrent */

/* sub_452de0: native, nfs3hp::frontend::setupWebSiteAnswered */

/* sub_452e60: native, nfs3hp::frontend::setupWebSiteClicked */

/* sub_452f20: native, nfs3hp::frontend::setupOnePlayerClicked */

/* sub_452f60: native, nfs3hp::frontend::setupOnFrame */

/* sub_452f80: native, nfs3hp::frontend::setupOnEnter */

/* sub_4530c0: native, nfs3hp::frontend::carArtExists */

/* sub_453160: native, nfs3hp::frontend::showcaseHasInterior */

/* sub_453200: native, nfs3hp::frontend::showcaseHasSlides */

/* sub_453290: native, nfs3hp::frontend::showcaseHasHistory */

/* sub_453320: native, nfs3hp::frontend::showcaseLoadArt */

/* sub_453440: native, nfs3hp::dead::stub_0x453440 */

/* sub_453470: native, nfs3hp::dead::stub_0x453470 */

/* sub_4534a0: native, nfs3hp::dead::stub_0x4534a0 */

/* sub_4534d0: native, nfs3hp::frontend::carViewRemember */

/* sub_453500: native, nfs3hp::frontend::showcaseResolveCar */

/* sub_4535e0: native, nfs3hp::frontend::showcaseOnEnter */

/* sub_4537d0: native, nfs3hp::frontend::showcaseOnFrame */

/* sub_453830: native, nfs3hp::frontend::showcaseOnExit */

/* sub_453840: native, nfs3hp::frontend::showcaseNextSlide */

/* sub_453860: native, nfs3hp::frontend::showcasePreviousSlide */

/* sub_453940: native, nfs3hp::frontend::carStatsDraw */

/* sub_453c00: native, nfs3hp::frontend::showcaseSlidesOnEnter */

/* sub_453ef0: native, nfs3hp::frontend::showcaseSlidesOnFrame */

/* sub_453f50: native, nfs3hp::frontend::showcaseSlidesOnExit */

/* sub_453f60: native, nfs3hp::frontend::carSpecsDraw */

/* sub_454000: native, nfs3hp::frontend::showcaseHistoryPage */

/* sub_454210: native, nfs3hp::frontend::showcaseHistoryPageFrom */

/* sub_454260: native, nfs3hp::frontend::showcaseHistoryOnEnter */

/* sub_454310: native, nfs3hp::frontend::showcaseHistoryOnFrame */

}
