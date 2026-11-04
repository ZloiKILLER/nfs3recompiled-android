#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_4ffce0: native, nfs3hp::port::bufferPosition */

/* sub_4ffd80: native, nfs3hp::snd::dead_0x4ffd80 */

/* sub_4ffd84: native, nfs3hp::port::mixFunctions */

/* sub_4fff18: native, nfs3hp::port::iSNDmixinit */

/* sub_500008: native, nfs3hp::port::iSNDmixreinit */

/* sub_5000b8: native, nfs3hp::port::iSNDmixstart */

/* sub_5004c8: native, nfs3hp::port::mixerRamp */

/* sub_500604: native, nfs3hp::port::iSNDmixstop */

/* sub_500654: native, nfs3hp::port::setSendVolume */

/* sub_5006a4: native, nfs3hp::port::setMixVolume */

/* sub_5006f8: native, nfs3hp::snd::dead_0x5006f8 */

/* sub_500864: native, nfs3hp::port::mixerServe */

/* sub_5008a0: native, nfs3hp::port::convert16 */

/* sub_500910: native, nfs3hp::port::convertToWords */

/* sub_500990: native, nfs3hp::port::convertXaStereo */

/* sub_500b10: native, nfs3hp::port::convert8 */

/* sub_500b90: native, nfs3hp::port::convertXaMono */

/* sub_500c80: native, nfs3hp::port::soundTick */

/* sub_500e20: native, nfs3hp::port::iSNDenteraudio */

/* sub_500e7c: native, nfs3hp::port::iSNDleaveaudio */

/* sub_500e94: native, nfs3hp::port::iSNDserveradd100hzclient */

/* sub_500ef0: native, nfs3hp::port::iSNDserverremove100hzclient */

/* sub_500f80: native, nfs3hp::snd::dead_0x500f80 */

/* sub_500f8c: native, nfs3hp::watcom::set_errno_dos */

/* sub_500ff4: native, nfs3hp::watcom::set_errno_nt */

/* sub_501010: native, nfs3hp::ea::checkSurfaceUnlocked */

/* sub_501050: native, nfs3hp::ea::fileReport */

/* sub_501110: native, nfs3hp::ea::virtualAllocPages */

/* sub_501180: native, nfs3hp::ea::virtualFreePages */

/* sub_5011a0: native, nfs3hp::ea::touchMemoryPages */

/* sub_501270: native, nfs3hp::ea::openBlockHandle */

/* sub_501360: native, nfs3hp::ea::blockHandleFile */

/* sub_5013a0: native, nfs3hp::ea::closeBlockHandle */

/* sub_5013b0: native, nfs3hp::ea::readBlockHandle */

/* sub_501450: native, nfs3hp::ea::seekBlockHandle */

/* sub_5014a0: native, nfs3hp::port::updateVoiceVolume */

/* sub_501540: native, nfs3hp::port::iSNDvol */

/* sub_501670: native, nfs3hp::port::soundVolume */

/* sub_5016c0: native, nfs3hp::port::iSNDclearchunk */

/* sub_5017b4: native, nfs3hp::port::platformChunks */

/* sub_5017c0: native, nfs3hp::port::timbreDefaults */

/* sub_5017d8: native, nfs3hp::port::resetTimbre */

/* sub_5018dc: native, nfs3hp::port::addTimbreDeltas */

/* sub_501938: native, nfs3hp::port::newGroup */

/* sub_501978: native, nfs3hp::port::iSNDplaytaggedtimbre */

/* sub_501de8: native, nfs3hp::port::iSNDplaytaggedpatch */

/* sub_5021ac: native, nfs3hp::port::iSNDresolvetaggedpatch */

/* sub_5022d0: native, nfs3hp::port::unresolveTaggedPatch */

/* sub_502330: native, nfs3hp::port::iSNDextracthdrinfo */

/* sub_5024a0: native, nfs3hp::port::iSNDbankremovepat */

/* sub_502570: native, nfs3hp::port::bankUsable */

/* sub_5025b0: native, nfs3hp::port::iSNDbankalloc */

/* sub_502620: native, nfs3hp::port::iSNDdownloadbank */

/* sub_502760: native, nfs3hp::watcom::flush */

/* sub_502880: native, nfs3hp::watcom::set_errno */

/* sub_502890: native, nfs3hp::watcom::set_EDOM */

/* sub_5028a4: native, nfs3hp::watcom::set_ERANGE */

/* sub_5028b8: native, nfs3hp::dead::stub_0x5028b8 */

/* sub_5028c8: native, nfs3hp::watcom::set_doserrno */

/* sub_5028e0: native, nfs3hp::watcom::lseek */

/* sub_502960: native, nfs3hp::watcom::tell */

/* sub_5029c0: native, nfs3hp::watcom::ioalloc */

/* sub_502a50: native, nfs3hp::watcom::qread */

/* sub_502af0: native, nfs3hp::watcom::fgetc */

/* sub_502bb8: native, nfs3hp::watcom::filbuf */

/* sub_502be8: native, nfs3hp::watcom::fill_buffer */

/* sub_502cb0: native, nfs3hp::port::updateVoicePitch */

/* sub_502d30: native, nfs3hp::port::iSNDplatformdetune */

/* sub_502df0: native, nfs3hp::port::iSNDplatform3dpos */

/* sub_502ef0: native, nfs3hp::port::iSNDpan */

/* sub_502ff0: native, nfs3hp::port::iSNDwinsetfx */

/* sub_5030ac: native, nfs3hp::port::setBusFx */

/* sub_5030c4: native, nfs3hp::port::busLevelSet */

/* sub_5030d0: native, nfs3hp::port::SNDfxlevel */

/* sub_503280: native, nfs3hp::dead::stub_0x503280 */

/* sub_503281: native, nfs3hp::dead::stub_0x503281 */

/* sub_5033e4: native, nfs3hp::mix::copySwapped */

/* sub_5033e5: native, nfs3hp::mix::copy */

/* sub_50356c: native, nfs3hp::dead::stub_0x50356c */

/* sub_50356d: native, nfs3hp::dead::stub_0x50356d */

/* sub_503670: native, nfs3hp::port::mul64 */

/* sub_503700: native, nfs3hp::dead::stub_0x503700 */

/* sub_503760: native, nfs3hp::port::div64 */

/* sub_5037a0: native, nfs3hp::dead::stub_0x5037a0 */

/* sub_503800: native, nfs3hp::dead::stub_0x503800 */

/* sub_503860: native, nfs3hp::dead::stub_0x503860 */

/* sub_5038c0: native, nfs3hp::dead::stub_0x5038c0 */

/* sub_503910: native, nfs3hp::dead::stub_0x503910 */

/* sub_503990: native, nfs3hp::dead::stub_0x503990 */

/* sub_503a10: native, nfs3hp::dead::stub_0x503a10 */

/* sub_503aa0: native, nfs3hp::dead::stub_0x503aa0 */

/* align: skip 0x90 */
void Application::sub_503b00(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00503b00  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00503b01  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00503b02  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00503b03  83ec08                 -sub esp, 8
    (cpu.esp) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 00503b06  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 00503b08  83fa20                 +cmp edx, 0x20
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(32 /*0x20*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00503b0b  7d29                   -jge 0x503b36
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00503b36;
    }
    // 00503b0d  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 00503b0f  7433                   -je 0x503b44
    if (cpu.flags.zf)
    {
        goto L_0x00503b44;
    }
    // 00503b11  88d1                   -mov cl, dl
    cpu.cl = cpu.dl;
    // 00503b13  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00503b15  d3e6                   -shl esi, cl
    cpu.esi <<= cpu.cl % 32;
    // 00503b17  b920000000             -mov ecx, 0x20
    cpu.ecx = 32 /*0x20*/;
    // 00503b1c  29d1                   -sub ecx, edx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00503b1e  d3f8                   -sar eax, cl
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (cpu.cl % 32));
    // 00503b20  893424                 -mov dword ptr [esp], esi
    app->getMemory<x86::reg32>(cpu.esp) = cpu.esi;
L_0x00503b23:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00503b23  89442404               -mov dword ptr [esp + 4], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */) = cpu.eax;
    // 00503b27  89e6                   -mov esi, esp
    cpu.esi = cpu.esp;
    // 00503b29  89df                   -mov edi, ebx
    cpu.edi = cpu.ebx;
    // 00503b2b  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00503b2c  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00503b2d  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 00503b2f  83c408                 -add esp, 8
    (cpu.esp) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 00503b32  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00503b33  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00503b34  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00503b35  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00503b36:
    // 00503b36  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00503b38  83ea20                 -sub edx, 0x20
    (cpu.edx) -= x86::reg32(x86::sreg32(32 /*0x20*/));
    // 00503b3b  890c24                 -mov dword ptr [esp], ecx
    app->getMemory<x86::reg32>(cpu.esp) = cpu.ecx;
    // 00503b3e  88d1                   -mov cl, dl
    cpu.cl = cpu.dl;
    // 00503b40  d3e0                   +shl eax, cl
    {
        x86::reg8 tmp = cpu.cl % 32;
        x86::reg32& op = cpu.eax;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 00503b42  ebdf                   -jmp 0x503b23
    goto L_0x00503b23;
L_0x00503b44:
    // 00503b44  890424                 -mov dword ptr [esp], eax
    app->getMemory<x86::reg32>(cpu.esp) = cpu.eax;
    // 00503b47  c1f81f                 -sar eax, 0x1f
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (31 /*0x1f*/ % 32));
    // 00503b4a  89442404               -mov dword ptr [esp + 4], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */) = cpu.eax;
    // 00503b4e  89e6                   -mov esi, esp
    cpu.esi = cpu.esp;
    // 00503b50  89df                   -mov edi, ebx
    cpu.edi = cpu.ebx;
    // 00503b52  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00503b53  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 00503b54  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 00503b56  83c408                 -add esp, 8
    (cpu.esp) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 00503b59  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00503b5a  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00503b5b  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00503b5c  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_503b60: native, nfs3hp::dead::stub_0x503b60 */

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_503bc0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00503bc0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00503bc1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00503bc2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00503bc3  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00503bc5  8b1d00505600           -mov ebx, dword ptr [0x565000]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5656576) /* 0x565000 */);
    // 00503bcb  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00503bcd  39d9                   +cmp ecx, ebx
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
    // 00503bcf  0f8ca2000000           -jl 0x503c77
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00503c77;
    }
    // 00503bd5  3b0d08505600           +cmp ecx, dword ptr [0x565008]
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5656584) /* 0x565008 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00503bdb  0f8d96000000           -jge 0x503c77
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00503c77;
    }
    // 00503be1  3b1504505600           +cmp edx, dword ptr [0x565004]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5656580) /* 0x565004 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00503be7  0f8c8a000000           -jl 0x503c77
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00503c77;
    }
    // 00503bed  3b150c505600           +cmp edx, dword ptr [0x56500c]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5656588) /* 0x56500c */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00503bf3  0f8d7e000000           -jge 0x503c77
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00503c77;
    }
    // 00503bf9  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00503c00  8b1520505600           -mov edx, dword ptr [0x565020]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5656608) /* 0x565020 */);
    // 00503c06  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 00503c08  8b1524505600           -mov edx, dword ptr [0x565024]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5656612) /* 0x565024 */);
    // 00503c0e  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 00503c10  8b1c8a                 -mov ebx, dword ptr [edx + ecx*4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.edx + cpu.ecx * 4);
    // 00503c13  8b1514505600           -mov edx, dword ptr [0x565014]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5656596) /* 0x565014 */);
    // 00503c19  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 00503c1b  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00503c1d  a010505600             -mov al, byte ptr [0x565010]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(5656592) /* 0x565010 */);
    // 00503c22  3c0f                   +cmp al, 0xf
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(15 /*0xf*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 00503c24  7355                   -jae 0x503c7b
    if (!cpu.flags.cf)
    {
        goto L_0x00503c7b;
    }
    // 00503c26  3c04                   +cmp al, 4
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(4 /*0x4*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 00503c28  0f836c000000           -jae 0x503c9a
    if (!cpu.flags.cf)
    {
        goto L_0x00503c9a;
    }
L_0x00503c2e:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00503c2e  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00503c2f  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00503c30  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00503c32  beb8eb5400             -mov esi, 0x54ebb8
    cpu.esi = 5565368 /*0x54ebb8*/;
    // 00503c37  a010505600             -mov al, byte ptr [0x565010]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(5656592) /* 0x565010 */);
    // 00503c3c  bfc8eb5400             -mov edi, 0x54ebc8
    cpu.edi = 5565384 /*0x54ebc8*/;
    // 00503c41  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00503c42  bd89000000             -mov ebp, 0x89
    cpu.ebp = 137 /*0x89*/;
    // 00503c47  893590215500           -mov dword ptr [0x552190], esi
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.esi;
    // 00503c4d  68d4eb5400             -push 0x54ebd4
    app->getMemory<x86::reg32>(cpu.esp-4) = 5565396 /*0x54ebd4*/;
    cpu.esp -= 4;
    // 00503c52  893d94215500           -mov dword ptr [0x552194], edi
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.edi;
    // 00503c58  892d98215500           -mov dword ptr [0x552198], ebp
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.ebp;
    // 00503c5e  e8add3efff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00503c63  83c408                 -add esp, 8
    (cpu.esp) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 00503c66  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00503c67  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
L_0x00503c68:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00503c68  f6c101                 +test cl, 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 1 /*0x1*/));
    // 00503c6b  743b                   -je 0x503ca8
    if (cpu.flags.zf)
    {
        goto L_0x00503ca8;
    }
    // 00503c6d  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00503c6f  8a02                   -mov al, byte ptr [edx]
    cpu.al = app->getMemory<x86::reg8>(cpu.edx);
    // 00503c71  c1f804                 -sar eax, 4
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (4 /*0x4*/ % 32));
    // 00503c74  83e00f                 +and eax, 0xf
    cpu.clear_co();
    cpu.set_szp((cpu.eax &= x86::reg32(x86::sreg32(15 /*0xf*/))));
L_0x00503c77:
    // 00503c77  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00503c78  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00503c79  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00503c7a  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00503c7b:
    // 00503c7b  7608                   -jbe 0x503c85
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00503c85;
    }
    // 00503c7d  3c18                   +cmp al, 0x18
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(24 /*0x18*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 00503c7f  730d                   -jae 0x503c8e
    if (!cpu.flags.cf)
    {
        goto L_0x00503c8e;
    }
    // 00503c81  3c10                   +cmp al, 0x10
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(16 /*0x10*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 00503c83  75a9                   -jne 0x503c2e
    if (!cpu.flags.zf)
    {
        goto L_0x00503c2e;
    }
L_0x00503c85:
    // 00503c85  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 00503c87  668b02                 -mov ax, word ptr [edx]
    cpu.ax = app->getMemory<x86::reg16>(cpu.edx);
    // 00503c8a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00503c8b  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00503c8c  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00503c8d  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00503c8e:
    // 00503c8e  7623                   -jbe 0x503cb3
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00503cb3;
    }
    // 00503c90  3c20                   +cmp al, 0x20
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(32 /*0x20*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 00503c92  759a                   -jne 0x503c2e
    if (!cpu.flags.zf)
    {
        goto L_0x00503c2e;
    }
    // 00503c94  8b02                   -mov eax, dword ptr [edx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx);
    // 00503c96  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00503c97  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00503c98  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00503c99  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00503c9a:
    // 00503c9a  76cc                   -jbe 0x503c68
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x00503c68;
    }
    // 00503c9c  3c08                   +cmp al, 8
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(8 /*0x8*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 00503c9e  758e                   -jne 0x503c2e
    if (!cpu.flags.zf)
    {
        goto L_0x00503c2e;
    }
    // 00503ca0  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00503ca2  8a02                   -mov al, byte ptr [edx]
    cpu.al = app->getMemory<x86::reg8>(cpu.edx);
    // 00503ca4  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00503ca5  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00503ca6  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00503ca7  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00503ca8:
    // 00503ca8  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00503caa  8a02                   -mov al, byte ptr [edx]
    cpu.al = app->getMemory<x86::reg8>(cpu.edx);
    // 00503cac  83e00f                 -and eax, 0xf
    cpu.eax &= x86::reg32(x86::sreg32(15 /*0xf*/));
    // 00503caf  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00503cb0  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00503cb1  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00503cb2  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00503cb3:
    // 00503cb3  b903000000             -mov ecx, 3
    cpu.ecx = 3 /*0x3*/;
    // 00503cb8  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 00503cba  8b4401fc               -mov eax, dword ptr [ecx + eax - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(-4) /* -0x4 */ + cpu.eax * 1);
    // 00503cbe  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
    // 00503cc0  8d0ccd20000000         -lea ecx, [ecx*8 + 0x20]
    cpu.ecx = x86::reg32(x86::reg32(32) /* 0x20 */ + cpu.ecx * 8);
    // 00503cc7  d3e8                   -shr eax, cl
    cpu.eax >>= cpu.cl % 32;
    // 00503cc9  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00503cca  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00503ccb  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00503ccc  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 0x00 */
void Application::sub_503cd0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00503cd0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00503cd1  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00503cd2  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00503cd3  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00503cd4  81ec80000000           -sub esp, 0x80
    (cpu.esp) -= x86::reg32(x86::sreg32(128 /*0x80*/));
    // 00503cda  89c5                   -mov ebp, eax
    cpu.ebp = cpu.eax;
    // 00503cdc  8954247c               -mov dword ptr [esp + 0x7c], edx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(124) /* 0x7c */) = cpu.edx;
    // 00503ce0  89de                   -mov esi, ebx
    cpu.esi = cpu.ebx;
    // 00503ce2  ff05e4435600           -inc dword ptr [0x5643e4]
    (app->getMemory<x86::reg32>(x86::reg32(5653476) /* 0x5643e4 */))++;
    // 00503ce8  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00503cea  7505                   -jne 0x503cf1
    if (!cpu.flags.zf)
    {
        goto L_0x00503cf1;
    }
    // 00503cec  bdd0389f00             -mov ebp, 0x9f38d0
    cpu.ebp = 10434768 /*0x9f38d0*/;
L_0x00503cf1:
    // 00503cf1  833d6443560007         +cmp dword ptr [0x564364], 7
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5653348) /* 0x564364 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(7 /*0x7*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00503cf8  0f8529010000           -jne 0x503e27
    if (!cpu.flags.zf)
    {
        goto L_0x00503e27;
    }
    // 00503cfe  837d1c08               +cmp dword ptr [ebp + 0x1c], 8
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(28) /* 0x1c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(8 /*0x8*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00503d02  0f8f1f010000           -jg 0x503e27
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x00503e27;
    }
    // 00503d08  8b44247c               -mov eax, dword ptr [esp + 0x7c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(124) /* 0x7c */);
    // 00503d0c  01f0                   -add eax, esi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.esi));
    // 00503d0e  48                     -dec eax
    (cpu.eax)--;
    // 00503d0f  8bbd3c040000           -mov edi, dword ptr [ebp + 0x43c]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(1084) /* 0x43c */);
    // 00503d15  89442478               -mov dword ptr [esp + 0x78], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(120) /* 0x78 */) = cpu.eax;
    // 00503d19  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 00503d1b  0f8411010000           -je 0x503e32
    if (cpu.flags.zf)
    {
        goto L_0x00503e32;
    }
    // 00503d21  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
L_0x00503d26:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00503d26  89442470               -mov dword ptr [esp + 0x70], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(112) /* 0x70 */) = cpu.eax;
    // 00503d2a  8b44247c               -mov eax, dword ptr [esp + 0x7c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(124) /* 0x7c */);
    // 00503d2e  8d5530                 -lea edx, [ebp + 0x30]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(48) /* 0x30 */);
    // 00503d31  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00503d34  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00503d36  8d1c02                 -lea ebx, [edx + eax]
    cpu.ebx = x86::reg32(cpu.edx + cpu.eax * 1);
    // 00503d39  0514a8a000             -add eax, 0xa0a814
    (cpu.eax) += x86::reg32(x86::sreg32(10528788 /*0xa0a814*/));
    // 00503d3e  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 00503d40  7e2e                   -jle 0x503d70
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00503d70;
    }
    // 00503d42  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
L_0x00503d44:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00503d44  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00503d46  8a5801                 -mov bl, byte ptr [eax + 1]
    cpu.bl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(1) /* 0x1 */);
    // 00503d49  0fb67802               -movzx edi, byte ptr [eax + 2]
    cpu.edi = x86::reg32(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(2) /* 0x2 */));
    // 00503d4d  c1e308                 -shl ebx, 8
    cpu.ebx <<= 8 /*0x8*/ % 32;
    // 00503d50  09df                   -or edi, ebx
    cpu.edi |= x86::reg32(x86::sreg32(cpu.ebx));
    // 00503d52  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00503d54  8a18                   -mov bl, byte ptr [eax]
    cpu.bl = app->getMemory<x86::reg8>(cpu.eax);
    // 00503d56  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00503d59  c1e310                 -shl ebx, 0x10
    cpu.ebx <<= 16 /*0x10*/ % 32;
    // 00503d5c  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00503d5f  09fb                   -or ebx, edi
    cpu.ebx |= x86::reg32(x86::sreg32(cpu.edi));
    // 00503d61  41                     -inc ecx
    (cpu.ecx)++;
    // 00503d62  895afc                 -mov dword ptr [edx - 4], ebx
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(-4) /* -0x4 */) = cpu.ebx;
    // 00503d65  39f1                   +cmp ecx, esi
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.esi));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00503d67  7cdb                   -jl 0x503d44
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00503d44;
    }
    // 00503d69  8d8000000000           -lea eax, [eax]
    cpu.eax = x86::reg32(cpu.eax);
    // 00503d6f  90                     -nop 
    ;
L_0x00503d70:
    // 00503d70  8b852e040000           -mov eax, dword ptr [ebp + 0x42e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(1070) /* 0x42e */);
    // 00503d76  8b54247c               -mov edx, dword ptr [esp + 0x7c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(124) /* 0x7c */);
    // 00503d7a  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00503d7d  39d0                   +cmp eax, edx
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
    // 00503d7f  7e0b                   -jle 0x503d8c
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00503d8c;
    }
    // 00503d81  8b44247c               -mov eax, dword ptr [esp + 0x7c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(124) /* 0x7c */);
    // 00503d85  66898530040000         -mov word ptr [ebp + 0x430], ax
    app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(1072) /* 0x430 */) = cpu.ax;
L_0x00503d8c:
    // 00503d8c  8b8530040000           -mov eax, dword ptr [ebp + 0x430]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(1072) /* 0x430 */);
    // 00503d92  8b4c2478               -mov ecx, dword ptr [esp + 0x78]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(120) /* 0x78 */);
    // 00503d96  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00503d99  39c8                   +cmp eax, ecx
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
    // 00503d9b  7d0b                   -jge 0x503da8
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x00503da8;
    }
    // 00503d9d  8b442478               -mov eax, dword ptr [esp + 0x78]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(120) /* 0x78 */);
    // 00503da1  66898532040000         -mov word ptr [ebp + 0x432], ax
    app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(1074) /* 0x432 */) = cpu.ax;
L_0x00503da8:
    // 00503da8  837c247000             +cmp dword ptr [esp + 0x70], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(112) /* 0x70 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00503dad  740c                   -je 0x503dbb
    if (cpu.flags.zf)
    {
        goto L_0x00503dbb;
    }
    // 00503daf  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 00503db1  e81a09ffff             -call 0x4f46d0
    cpu.esp -= 4;
    sub_4f46d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00503db6  e8d56ffeff             -call 0x4ead90
    cpu.esp -= 4;
    sub_4ead90(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00503dbb:
    // 00503dbb  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 00503dbd  e89e91feff             -call 0x4ecf60
    cpu.esp -= 4;
    sub_4ecf60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00503dc2  80bd5d04000000         +cmp byte ptr [ebp + 0x45d], 0
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(1117) /* 0x45d */);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 00503dc9  7555                   -jne 0x503e20
    if (!cpu.flags.zf)
    {
        goto L_0x00503e20;
    }
    // 00503dcb  8b44247c               -mov eax, dword ptr [esp + 0x7c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(124) /* 0x7c */);
    // 00503dcf  8d5530                 -lea edx, [ebp + 0x30]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(48) /* 0x30 */);
    // 00503dd2  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00503dd5  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00503dd7  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00503dd9  0514a8a000             -add eax, 0xa0a814
    (cpu.eax) += x86::reg32(x86::sreg32(10528788 /*0xa0a814*/));
    // 00503dde  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 00503de0  7e3e                   -jle 0x503e20
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00503e20;
    }
L_0x00503de2:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00503de2  8a1a                   -mov bl, byte ptr [edx]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edx);
    // 00503de4  885802                 -mov byte ptr [eax + 2], bl
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(2) /* 0x2 */) = cpu.bl;
    // 00503de7  8b1a                   -mov ebx, dword ptr [edx]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.edx);
    // 00503de9  c1eb08                 -shr ebx, 8
    cpu.ebx >>= 8 /*0x8*/ % 32;
    // 00503dec  81e3ff000000           -and ebx, 0xff
    cpu.ebx &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 00503df2  895c2474               -mov dword ptr [esp + 0x74], ebx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(116) /* 0x74 */) = cpu.ebx;
    // 00503df6  8a5c2474               -mov bl, byte ptr [esp + 0x74]
    cpu.bl = app->getMemory<x86::reg8>(cpu.esp + x86::reg32(116) /* 0x74 */);
    // 00503dfa  885801                 -mov byte ptr [eax + 1], bl
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(1) /* 0x1 */) = cpu.bl;
    // 00503dfd  8b1a                   -mov ebx, dword ptr [edx]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.edx);
    // 00503dff  c1eb10                 -shr ebx, 0x10
    cpu.ebx >>= 16 /*0x10*/ % 32;
    // 00503e02  81e3ff000000           -and ebx, 0xff
    cpu.ebx &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 00503e08  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00503e0b  895c2474               -mov dword ptr [esp + 0x74], ebx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(116) /* 0x74 */) = cpu.ebx;
    // 00503e0f  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00503e12  8a5c2474               -mov bl, byte ptr [esp + 0x74]
    cpu.bl = app->getMemory<x86::reg8>(cpu.esp + x86::reg32(116) /* 0x74 */);
    // 00503e16  41                     -inc ecx
    (cpu.ecx)++;
    // 00503e17  8858fc                 -mov byte ptr [eax - 4], bl
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(-4) /* -0x4 */) = cpu.bl;
    // 00503e1a  39f1                   +cmp ecx, esi
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.esi));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00503e1c  7cc4                   -jl 0x503de2
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00503de2;
    }
    // 00503e1e  8bc0                   -mov eax, eax
    cpu.eax = cpu.eax;
L_0x00503e20:
    // 00503e20  837c247000             +cmp dword ptr [esp + 0x70], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(112) /* 0x70 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00503e25  7512                   -jne 0x503e39
    if (!cpu.flags.zf)
    {
        goto L_0x00503e39;
    }
L_0x00503e27:
    // 00503e27  81c480000000           -add esp, 0x80
    (cpu.esp) += x86::reg32(x86::sreg32(128 /*0x80*/));
    // 00503e2d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00503e2e  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00503e2f  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00503e30  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00503e31  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00503e32:
    // 00503e32  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 00503e34  e9edfeffff             -jmp 0x503d26
    goto L_0x00503d26;
L_0x00503e39:
    // 00503e39  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 00503e3b  e8c008ffff             -call 0x4f4700
    cpu.esp -= 4;
    sub_4f4700(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00503e40  81c480000000           -add esp, 0x80
    (cpu.esp) += x86::reg32(x86::sreg32(128 /*0x80*/));
    // 00503e46  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00503e47  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00503e48  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00503e49  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00503e4a  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_503e50(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00503e50  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00503e51  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00503e52  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00503e54  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 00503e56  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 00503e58  8d14b500000000         -lea edx, [esi*4]
    cpu.edx = x86::reg32(cpu.esi * 4);
    // 00503e5f  89cb                   -mov ebx, ecx
    cpu.ebx = cpu.ecx;
    // 00503e61  81c214a8a000           -add edx, 0xa0a814
    (cpu.edx) += x86::reg32(x86::sreg32(10528788 /*0xa0a814*/));
    // 00503e67  e8f1030000             -call 0x50425d
    cpu.esp -= 4;
    sub_50425d(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00503e6c  b8d0389f00             -mov eax, 0x9f38d0
    cpu.eax = 10434768 /*0x9f38d0*/;
    // 00503e71  89cb                   -mov ebx, ecx
    cpu.ebx = cpu.ecx;
    // 00503e73  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 00503e75  e856feffff             -call 0x503cd0
    cpu.esp -= 4;
    sub_503cd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00503e7a  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00503e7b  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00503e7c  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_503e80: native, nfs3hp::dead::stub_0x503e80 */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_503ec0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00503ec0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00503ec1  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00503ec2  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 00503ec4  b914a8a000             -mov ecx, 0xa0a814
    cpu.ecx = 10528788 /*0xa0a814*/;
    // 00503ec9  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00503ecc  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00503ece  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 00503ed0  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 00503ed2  7e2c                   -jle 0x503f00
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00503f00;
    }
    // 00503ed4  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
L_0x00503ed6:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00503ed6  43                     -inc ebx
    (cpu.ebx)++;
    // 00503ed7  8a4802                 -mov cl, byte ptr [eax + 2]
    cpu.cl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(2) /* 0x2 */);
    // 00503eda  884bff                 -mov byte ptr [ebx - 1], cl
    app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(-1) /* -0x1 */) = cpu.cl;
    // 00503edd  43                     -inc ebx
    (cpu.ebx)++;
    // 00503ede  8a4801                 -mov cl, byte ptr [eax + 1]
    cpu.cl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(1) /* 0x1 */);
    // 00503ee1  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00503ee4  884bff                 -mov byte ptr [ebx - 1], cl
    app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(-1) /* -0x1 */) = cpu.cl;
    // 00503ee7  43                     -inc ebx
    (cpu.ebx)++;
    // 00503ee8  8a48fc                 -mov cl, byte ptr [eax - 4]
    cpu.cl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(-4) /* -0x4 */);
    // 00503eeb  42                     -inc edx
    (cpu.edx)++;
    // 00503eec  884bff                 -mov byte ptr [ebx - 1], cl
    app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(-1) /* -0x1 */) = cpu.cl;
    // 00503eef  39f2                   +cmp edx, esi
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
    // 00503ef1  7ce3                   -jl 0x503ed6
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x00503ed6;
    }
    // 00503ef3  8d8000000000           -lea eax, [eax]
    cpu.eax = x86::reg32(cpu.eax);
    // 00503ef9  8d9200000000           -lea edx, [edx]
    cpu.edx = x86::reg32(cpu.edx);
    // 00503eff  90                     -nop 
    ;
L_0x00503f00:
    // 00503f00  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00503f01  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00503f02  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_503f10: native, nfs3hp::dead::stub_0x503f10 */

/* sub_503f50: native, nfs3hp::dead::stub_0x503f50 */

}
