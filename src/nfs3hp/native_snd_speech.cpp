#include "native_ea_memory.h"
#include "native_ea_support.h"
#include "native_snd.h"

/* The speech's one function that stays register-level native (native_snd.h
 * says how): the description bank loaded, which has SND's SNDbankadd add the
 * bank of the file it loads.  The checks put a stand-in with a made-up answer
 * in the place of SNDbankadd for a native like this, but not for a rewrite
 * (port::) that calls a rewrite, which would run the real one -- on a file
 * the checks' file stand-in cannot give a real bank.  The rest of the speech
 * is port_sound_speech.cpp's. */
namespace nfs3hp
{
namespace snd
{

/* 0x4c9240: the description bank eax (".bnk" added) loaded, if it is there
 * and none is: its handle kept (0x7dd844), the header copied out of the file
 * where SND asks (0x7dd84c); 1, or 0. */
void descriptionLoad(App* app, CPU& cpu)
{
    const Kept kept(cpu);
    const unsigned pushed = kEbx | kEcx | kEdx | kEsi | kEdi;
    cpu.ebp = kept.frame(6);
    cpu.esp = cpu.ebp - 0x104;
    const reg32 name = cpu.ebp - 0x104;
    auto none = [&] {
        cpu.eax = 0;
        leave(cpu, kept, pushed);
    };
    if (at(app, 0x55290c) == 0)
        return none();
    copyString(app, cpu, name, cpu.eax);
    appendString(app, cpu, name, 0x54033c);
    cpu.edi = name;
    const std::optional<bool> there = guest::direct<0x4e0e60, ea::fileThere>(app, cpu, name);
    if (!there)
        return;
    cpu.eax = *there ? 1 : 0;
    if (cpu.eax == 0 || at(app, 0x7dd838) != 0)
        return none();
    cpu.edx = 0;
    const std::optional<reg32> file = guest::direct<0x4e0ec0, ea::loadWhole>(app, cpu, name, cpu.edx);
    if (!file)
        return;
    cpu.eax = *file;
    cpu.ecx = cpu.eax;
    cpu.ebx = cpu.eax;
    cpu.edx = cpu.eax;
    cpu.eax = cpu.ebp - 4;
    if (!call(app, cpu, 0x4e8d20))
        return;
    cpu.edi = cpu.eax;
    if (cpu.eax == 7)
    {
        cpu.eax = at(app, cpu.ebp - 4);
        if (!call(app, cpu, 0x4e8db0))
            return;
        cpu.ebx ^= cpu.ecx;
        cpu.edx = cpu.eax;
        // ebx and edx after it are the code's own again at once, ecx the bank's (kept).
        const std::optional<reg32> copy = guest::direct<0x4e1620, ea::reserved>(app, cpu, 0x540344u, cpu.edx, cpu.ebx);
        if (!copy)
            return;
        cpu.eax = *copy;
        cpu.edx = at(app, cpu.ebp - 4);
        cpu.ebx = cpu.eax;
        if (!call(app, cpu, 0x4e8e20))
            return;
        // ebx (the copy) kept across it, as the code counts on.
        const std::optional<reg32> purged = guest::direct<0x4e1890, ea::purgeBlock>(app, cpu, cpu.ecx);
        if (!purged)
            return;
        cpu.eax = *purged;
    }
    if (i32(cpu.edi) < 0)
        return none();
    cpu.eax = at(app, cpu.ebp - 4);
    put(app, 0x7dd84c, cpu.ebx);
    cpu.ebx = 1;
    put(app, 0x7dd844, cpu.eax);
    put(app, 0x7dd838, cpu.ebx);
    cpu.eax = cpu.ebx;
    leave(cpu, kept, pushed);
}

}  // namespace snd
}  // namespace nfs3hp
