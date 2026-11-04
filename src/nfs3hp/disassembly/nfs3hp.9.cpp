#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_4379b0: native, nfs3hp::frontend::audioRaceMusicMoved */

/* sub_4379d0: native, nfs3hp::frontend::audioEffectsMoved */

/* sub_437a20: native, nfs3hp::frontend::audioEngineMoved */

/* sub_437a70: native, nfs3hp::frontend::audioSpeechMoved */

/* sub_437a90: native, nfs3hp::frontend::audioVolumeDraw */

/* sub_437e00: native, nfs3hp::frontend::audioOnEnter */

/* sub_438070: native, nfs3hp::frontend::audioOnExit */

/* sub_438260: native, nfs3hp::frontend::audioOnFrame */

/* sub_438340: native, nfs3hp::load::endWithSlash */

/* sub_438350: native, nfs3hp::load::folderWritable */

/* sub_4383f0: native, nfs3hp::load::firstFreeHexFolder */

/* sub_4384a0: native, nfs3hp::dead::stub_0x4384a0 */

/* sub_4384f0: native, nfs3hp::load::firstFreePlayerFolder */

/* sub_4385a0: native, nfs3hp::load::freePlayerCarFolder */

/* sub_4385f0: native, nfs3hp::dead::stub_0x4385f0 */

/* sub_4386a0: native, nfs3hp::dead::stub_0x4386a0 */

/* sub_4386b0: native, nfs3hp::load::firstSubfolder */

/* sub_4387e0: native, nfs3hp::load::nextSubfolder */

/* sub_438870: native, nfs3hp::load::firstFile */

/* sub_438970: native, nfs3hp::load::nextFile */

/* sub_4389d0: native, nfs3hp::load::removeFolder */

/* sub_438a80: native, nfs3hp::load::firstCar */

/* sub_438aa0: native, nfs3hp::load::nextCar */

/* sub_438ad0: native, nfs3hp::load::makeCarId */

/* sub_438af0: native, nfs3hp::load::firstCarGlobal */

/* sub_438b10: native, nfs3hp::load::nextCarGlobal */

/* sub_438b40: native, nfs3hp::dead::stub_0x438b40 */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_438b70(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00438b70  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00438b71  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00438b72  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00438b74  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00438b76  e875ffffff             -call 0x438af0
    cpu.esp -= 4;
    nfs3hp::load::firstCarGlobal(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00438b7b:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00438b7b  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00438b7d  740b                   -je 0x438b8a
    if (cpu.flags.zf)
    {
        goto L_0x00438b8a;
    }
    // 00438b7f  3b10                   +cmp edx, dword ptr [eax]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00438b81  7407                   -je 0x438b8a
    if (cpu.flags.zf)
    {
        goto L_0x00438b8a;
    }
    // 00438b83  e888ffffff             -call 0x438b10
    cpu.esp -= 4;
    nfs3hp::load::nextCarGlobal(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00438b88  ebf1                   -jmp 0x438b7b
    goto L_0x00438b7b;
L_0x00438b8a:
    // 00438b8a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00438b8b  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00438b8c  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_438b90: native, nfs3hp::load::appendCar */

/* sub_438bd0: native, nfs3hp::load::carCount */

/* sub_438be0: native, nfs3hp::load::countCars */

/* sub_438c10: native, nfs3hp::load::unlinkCar */

/* sub_438c50: native, nfs3hp::load::insertCarByName */

/* sub_438cc0: native, nfs3hp::load::joinCarLists */

/* sub_438d20: native, nfs3hp::load::carListOf */

/* sub_438d60: native, nfs3hp::load::takePoliceCars */

/* sub_438dc0: native, nfs3hp::load::takeClasslessCars */

/* sub_438e30: native, nfs3hp::load::takeOrdinaryCars */

/* sub_438eb0: native, nfs3hp::load::sortCars */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_438f60(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 00438f60  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00438f61  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00438f62  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00438f64  e8e70a0000             -call 0x439a50
    cpu.esp -= 4;
    nfs3hp::load::carRecord(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00438f69  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00438f6b  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00438f6d  7443                   -je 0x438fb2
    if (cpu.flags.zf)
    {
        goto L_0x00438fb2;
    }
    // 00438f6f  83fa03                 +cmp edx, 3
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
    // 00438f72  773c                   -ja 0x438fb0
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x00438fb0;
    }
    // 00438f74  ff2495488f4300         -jmp dword ptr [edx*4 + 0x438f48]
    cpu.ip = app->getMemory<x86::reg32>(4427592 + cpu.edx * 4); goto dynamic_jump;
  case 0x00438f7b:
    // 00438f7b  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00438f7d  8a818f030000           -mov al, byte ptr [ecx + 0x38f]
    cpu.al = app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(911) /* 0x38f */);
    // 00438f83  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00438f84  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00438f85  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x00438f86:
    // 00438f86  8a8090030000           -mov al, byte ptr [eax + 0x390]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(912) /* 0x390 */);
    // 00438f8c  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 00438f91  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00438f92  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00438f93  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x00438f94:
    // 00438f94  8a8091030000           -mov al, byte ptr [eax + 0x391]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(913) /* 0x391 */);
    // 00438f9a  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 00438f9f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00438fa0  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00438fa1  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x00438fa2:
    // 00438fa2  8a8092030000           -mov al, byte ptr [eax + 0x392]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(914) /* 0x392 */);
    // 00438fa8  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 00438fad  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00438fae  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00438faf  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00438fb0:
    // 00438fb0  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x00438fb2:
    // 00438fb2  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00438fb3  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00438fb4  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* sub_438fc0: native, nfs3hp::dead::stub_0x438fc0 */

/* sub_4390f0: native, nfs3hp::dead::stub_0x4390f0 */

/* sub_4391d0: native, nfs3hp::dead::stub_0x4391d0 */

/* sub_439250: native, nfs3hp::load::carName */

/* sub_439290: native, nfs3hp::frontend::showcaseCarFileStem */

/* sub_4392b0: native, nfs3hp::load::carFolder */

/* sub_439300: native, nfs3hp::load::carFilePrefix */

/* sub_439360: native, nfs3hp::port::obj3d::carVivPath */

/* sub_439450: native, nfs3hp::dead::stub_0x439450 */

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_439510(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00439510  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00439511  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00439513  e838050000             -call 0x439a50
    cpu.esp -= 4;
    nfs3hp::load::carRecord(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00439518  0594030000             -add eax, 0x394
    (cpu.eax) += x86::reg32(x86::sreg32(916 /*0x394*/));
    // 0043951d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043951e  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_439520: native, nfs3hp::load::carId */

/* align: skip 0x90 */
void Application::sub_439530(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00439530  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00439531  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00439532  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00439534  e857040000             -call 0x439990
    cpu.esp -= 4;
    nfs3hp::load::findCarById(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00439539  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0043953b  e830000000             -call 0x439570
    cpu.esp -= 4;
    nfs3hp::load::carFlag40(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00439540  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00439542  7405                   -je 0x439549
    if (cpu.flags.zf)
    {
        goto L_0x00439549;
    }
    // 00439544  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00439546  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00439547  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00439548  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00439549:
    // 00439549  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0043954b  e830010000             -call 0x439680
    cpu.esp -= 4;
    nfs3hp::load::carIsBonus(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00439550  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00439552  7405                   -je 0x439559
    if (cpu.flags.zf)
    {
        goto L_0x00439559;
    }
    // 00439554  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00439556  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00439557  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00439558  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00439559:
    // 00439559  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0043955e  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043955f  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00439560  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_439570: native, nfs3hp::load::carFlag40 */

/* sub_439590: native, nfs3hp::dead::stub_0x439590 */

/* align: skip 0x90 */
void Application::sub_4395a0(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004395a0  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004395a1  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004395a3  e8a8040000             -call 0x439a50
    cpu.esp -= 4;
    nfs3hp::load::carRecord(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004395a8  8b8080030000           -mov eax, dword ptr [eax + 0x380]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(896) /* 0x380 */);
    // 004395ae  83e008                 -and eax, 8
    cpu.eax &= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 004395b1  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004395b2  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4395c0: native, nfs3hp::load::carFlag4 */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_4395e0(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004395e0  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004395e1  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004395e3  e868040000             -call 0x439a50
    cpu.esp -= 4;
    nfs3hp::load::carRecord(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004395e8  8b8080030000           -mov eax, dword ptr [eax + 0x380]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(896) /* 0x380 */);
    // 004395ee  83e002                 -and eax, 2
    cpu.eax &= x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004395f1  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004395f2  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_439600(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00439600  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00439601  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00439603  e888030000             -call 0x439990
    cpu.esp -= 4;
    nfs3hp::load::findCarById(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00439608  e8d3ffffff             -call 0x4395e0
    cpu.esp -= 4;
    sub_4395e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043960d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043960e  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_439610: native, nfs3hp::load::carHasNumericId */

/* sub_439620: native, nfs3hp::load::idIsNumeric */

/* sub_439660: native, nfs3hp::load::numericId */

/* sub_439680: native, nfs3hp::load::carIsBonus */

/* sub_4396a0: native, nfs3hp::dead::stub_0x4396a0 */

/* sub_4396b0: native, nfs3hp::load::carIsPolice */

/* sub_4396d0: native, nfs3hp::dead::stub_0x4396d0 */

/* sub_4396e0: native, nfs3hp::load::carIsFerrariOrMercedes */

/* sub_439700: native, nfs3hp::dead::stub_0x439700 */

/* align: skip 0x90 */
void Application::sub_439710(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00439710  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00439711  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00439713  e838030000             -call 0x439a50
    cpu.esp -= 4;
    nfs3hp::load::carRecord(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00439718  8a80a0030000           -mov al, byte ptr [eax + 0x3a0]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(928) /* 0x3a0 */);
    // 0043971e  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 00439723  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00439724  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_439730: native, nfs3hp::load::carClass */

/* sub_439740: native, nfs3hp::dead::stub_0x439740 */

/* sub_439760: native, nfs3hp::port::fields::carColourCount */

/* sub_439780: native, nfs3hp::port::fields::carCustomColour */

/* sub_439790: native, nfs3hp::port::fields::carColour */

/* sub_4397c0: native, nfs3hp::port::fields::carColourName */

/* sub_4397d0: native, nfs3hp::port::fields::setCarColour */

/* sub_439810: native, nfs3hp::port::fields::carColourRgb */

/* sub_4398c0: native, nfs3hp::port::fields::setCarCustomColour */

/* sub_439930: native, nfs3hp::dead::stub_0x439930 */

/* sub_439940: native, nfs3hp::dead::stub_0x439940 */

/* sub_439950: native, nfs3hp::dead::stub_0x439950 */

/* sub_439960: native, nfs3hp::dead::stub_0x439960 */

/* sub_439970: native, nfs3hp::dead::stub_0x439970 */

/* sub_439980: native, nfs3hp::dead::stub_0x439980 */

/* sub_439990: native, nfs3hp::load::findCarById */

/* sub_4399f0: native, nfs3hp::load::clearCarCache */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_439a10(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00439a10  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00439a11  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00439a12  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00439a14  8b1485844f5500         -mov edx, dword ptr [eax*4 + 0x554f84]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5590916) /* 0x554f84 */ + cpu.eax * 4);
    // 00439a1b  b8c0d26f00             -mov eax, 0x6fd2c0
    cpu.eax = 7328448 /*0x6fd2c0*/;
    // 00439a20  e8abf0ffff             -call 0x438ad0
    cpu.esp -= 4;
    nfs3hp::load::makeCarId(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00439a25  b8c0d26f00             -mov eax, 0x6fd2c0
    cpu.eax = 7328448 /*0x6fd2c0*/;
    // 00439a2a  e861ffffff             -call 0x439990
    cpu.esp -= 4;
    nfs3hp::load::findCarById(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00439a2f  a3bcd26f00             -mov dword ptr [0x6fd2bc], eax
    app->getMemory<x86::reg32>(x86::reg32(7328444) /* 0x6fd2bc */) = cpu.eax;
    // 00439a34  e8f7fcffff             -call 0x439730
    cpu.esp -= 4;
    nfs3hp::load::carClass(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00439a39  a3d0d26f00             -mov dword ptr [0x6fd2d0], eax
    app->getMemory<x86::reg32>(x86::reg32(7328464) /* 0x6fd2d0 */) = cpu.eax;
    // 00439a3e  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00439a3f  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00439a40  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_439a50: native, nfs3hp::load::carRecord */

/* sub_439ab0: native, nfs3hp::port::fields::carText */

/* sub_439af0: native, nfs3hp::load::fecText */

/* sub_439b20: native, nfs3hp::load::findCarByName */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_439b70(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00439b70  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00439b71  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00439b72  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
L_0x00439b74:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00439b74  8b15704f5500           -mov edx, dword ptr [0x554f70]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5590896) /* 0x554f70 */);
    // 00439b7a  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 00439b7c  7427                   -je 0x439ba5
    if (cpu.flags.zf)
    {
        goto L_0x00439ba5;
    }
    // 00439b7e  8b02                   -mov eax, dword ptr [edx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx);
    // 00439b80  a3704f5500             -mov dword ptr [0x554f70], eax
    app->getMemory<x86::reg32>(x86::reg32(5590896) /* 0x554f70 */) = cpu.eax;
    // 00439b85  f6828003000008         +test byte ptr [edx + 0x380], 8
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.edx + x86::reg32(896) /* 0x380 */) & 8 /*0x8*/));
    // 00439b8c  7407                   -je 0x439b95
    if (cpu.flags.zf)
    {
        goto L_0x00439b95;
    }
    // 00439b8e  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00439b90  e80b170000             -call 0x43b2a0
    cpu.esp -= 4;
    sub_43b2a0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00439b95:
    // 00439b95  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00439b97  e814000000             -call 0x439bb0
    cpu.esp -= 4;
    sub_439bb0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00439b9c  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00439b9e  e8ed7c0a00             -call 0x4e1890
    cpu.esp -= 4;
    nfs3hp::ea::purgememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00439ba3  ebcf                   -jmp 0x439b74
    goto L_0x00439b74;
L_0x00439ba5:
    // 00439ba5  8915704f5500           -mov dword ptr [0x554f70], edx
    app->getMemory<x86::reg32>(x86::reg32(5590896) /* 0x554f70 */) = cpu.edx;
    // 00439bab  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00439bad  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00439bae  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00439baf  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_439bb0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00439bb0  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00439bb1  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00439bb3  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00439bb5  7507                   -jne 0x439bbe
    if (!cpu.flags.zf)
    {
        goto L_0x00439bbe;
    }
    // 00439bb7  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
    // 00439bbc  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00439bbd  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00439bbe:
    // 00439bbe  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00439bc1  e82a000000             -call 0x439bf0
    cpu.esp -= 4;
    sub_439bf0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00439bc6  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00439bc8  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00439bc9  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_439bd0: native, nfs3hp::load::attachFecTexts */

/* align: skip 0x8b 0xc0 */
void Application::sub_439bf0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00439bf0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00439bf1  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00439bf2  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00439bf4  8b5004                 -mov edx, dword ptr [eax + 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 00439bf7  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 00439bf9  7407                   -je 0x439c02
    if (cpu.flags.zf)
    {
        goto L_0x00439c02;
    }
    // 00439bfb  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00439bfd  e88e7c0a00             -call 0x4e1890
    cpu.esp -= 4;
    nfs3hp::ea::purgememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00439c02:
    // 00439c02  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00439c04  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00439c05  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00439c06  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_439c10: native, nfs3hp::load::bonusCarLocked */

/* align: skip 0x90 */
void Application::sub_439cc0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00439cc0  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00439cc1  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00439cc3  e888fdffff             -call 0x439a50
    cpu.esp -= 4;
    nfs3hp::load::carRecord(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00439cc8  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00439cca  7405                   -je 0x439cd1
    if (cpu.flags.zf)
    {
        goto L_0x00439cd1;
    }
    // 00439ccc  e83fffffff             -call 0x439c10
    cpu.esp -= 4;
    nfs3hp::load::bonusCarLocked(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00439cd1:
    // 00439cd1  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00439cd2  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_439ce0: native, nfs3hp::frontend::creditsHaveBonusCar */

/* sub_439d20: native, nfs3hp::load::checkDuplicateCars */

/* sub_439d90: native, nfs3hp::load::loadCarList */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_43a4d0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0043a4d0  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0043a4d1  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0043a4d3  8b12                   -mov edx, dword ptr [edx]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx);
    // 0043a4d5  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 0043a4d7  83c210                 -add edx, 0x10
    (cpu.edx) += x86::reg32(x86::sreg32(16 /*0x10*/));
    // 0043a4da  83c010                 -add eax, 0x10
    (cpu.eax) += x86::reg32(x86::sreg32(16 /*0x10*/));
    // 0043a4dd  e82e3e0b00             -call 0x4ee310
    cpu.esp -= 4;
    nfs3hp::watcom::strcmp(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043a4e2  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043a4e3  c3                     -ret 
    cpu.esp += 4;
    return;
}

}
