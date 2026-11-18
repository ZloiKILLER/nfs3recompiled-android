#include "native_ea_memory.h"
#include "native_ea_support.h"
#include "native_snd.h"
#include "port_snd_calls.h"

/* The speech's one function that stays register-level native (native_snd.h
 * says how): the description bank loaded, which has SND's SNDbankadd add the
 * bank of the file it loads.  The rest of the speech
 * is port_sound_speech.cpp's. */
namespace nfs3hp
{
namespace snd
{

/* 0x4c9240: the description bank eax (".bnk" added) loaded, if it is there
 * and none is: its handle kept (0x7dd844), the header copied out of the file
 * where SND asks (0x7dd84c); 1, or 0.  Every register but eax as it was, esp
 * past the return address. */
x86::reg32 descriptionLoad(App* app, CPU& cpu, x86::Args& args)
{
    const port::Entry e(cpu, args);
    args.ebp = e.frame(6);
    cpu.esp = args.ebp - 0x104;
    const reg32 name = args.ebp - 0x104;
    auto none = [&] { return e.back(cpu, args, 0); };
    // The game's inlined strcpy of `from` to `to`, two bytes a turn: al left 0, esi where the copy stopped
    // reading; and its strcat, its repne scasb leaving ecx past the end.
    auto copyString = [&](reg32 to, reg32 from) {
        args.esi = from;
        for (reg32 at = to;; at += 2)
        {
            reg8 c = byte(app, args.esi);
            putByte(app, at, c);
            if (c == 0)
                break;
            c = byte(app, args.esi + 1);
            args.esi += 2;
            putByte(app, at + 1, c);
            if (c == 0)
                break;
        }
        args.eax &= 0xffffff00;
    };
    auto appendString = [&](reg32 to, reg32 from) {
        reg32 end = to;
        while (byte(app, end) != 0)
            ++end;
        args.ecx = reg32(-2 - i32(end - to));
        copyString(end, from);
    };
    if (at(app, 0x55290c) == 0)
        return none();
    copyString(name, args.eax);
    appendString(name, 0x54033c);
    args.edi = name;
    const std::optional<bool> there = guest::direct<0x4e0e60, ea::fileThere>(app, cpu, name);
    if (!there)
        return args.eax;
    args.eax = *there ? 1 : 0;
    if (args.eax == 0 || at(app, 0x7dd838) != 0)
        return none();
    args.edx = 0;
    const std::optional<reg32> file = guest::direct<0x4e0ec0, ea::loadWhole>(app, cpu, name, args.edx);
    if (!file)
        return args.eax;
    args.eax = *file;
    args.ecx = args.eax;
    args.ebx = args.eax;
    args.edx = args.eax;
    // SNDbankadd: the handle into [ebp - 4], the bank the file.
    const port::Result added = guest::direct<0x4e8d20, port::bankAdded>(app, cpu, cpu.esp - 4, args.ebp - 4, *file);
    if (!added)
        return args.eax;
    args.eax = reg32(*added);
    args.edi = args.eax;
    if (args.eax == 7)
    {
        // SNDbankheadersize of the handle.
        const port::Result size =
            guest::direct<0x4e8db0, port::bankHeaderSize>(app, cpu, cpu.esp - 4, i32(at(app, args.ebp - 4)));
        if (!size)
            return args.eax;
        args.eax = reg32(*size);
        args.ebx ^= args.ecx;
        args.edx = args.eax;
        // ebx and edx after it are the code's own again at once, ecx the bank's (kept).
        const std::optional<reg32> copy = guest::direct<0x4e1620, ea::reserved>(app, cpu, 0x540344u, args.edx, args.ebx);
        if (!copy)
            return args.eax;
        args.eax = *copy;
        args.edx = at(app, args.ebp - 4);
        args.ebx = args.eax;
        // SNDbankheadercopy to the copy, of the handle.
        const port::Result copied =
            guest::direct<0x4e8e20, port::copyBankHeader>(app, cpu, cpu.esp - 4, args.eax, i32(args.edx));
        if (!copied)
            return args.eax;
        args.eax = reg32(*copied);
        // ebx (the copy) kept across it, as the code counts on.
        const std::optional<reg32> purged = guest::direct<0x4e1890, ea::purgeBlock>(app, cpu, args.ecx);
        if (!purged)
            return args.eax;
        args.eax = *purged;
    }
    if (i32(args.edi) < 0)
        return none();
    args.eax = at(app, args.ebp - 4);
    put(app, 0x7dd84c, args.ebx);
    args.ebx = 1;
    put(app, 0x7dd844, args.eax);
    put(app, 0x7dd838, args.ebx);
    return e.back(cpu, args, args.ebx);
}

}  // namespace snd
}  // namespace nfs3hp
