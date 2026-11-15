#include "game_settings.h"
#include "native_ea_memory.h"
#include "native_ea_support.h"
#include "native_watcom.h"
#include "port_sound_slice.h"

#include <cstdint>

/* The race's banks from the game's archives, rewritten (port.h says how): a
 * bank read out of a .viv archive (0x43b830 opens one, 0x43b9d0 finds a file
 * in it, 0x43b9b0 closes it) into memory and added to SND -- its header
 * copied out where SND asks -- or, missing from the archive, the bank file of
 * the audio folder loaded instead (port_sound_music.cpp's bankLoaded).
 *
 * The game's state: 0x571124 the banks (12 bytes each: the handle, then at
 * +8 the memory it is in).  The archive functions and the file's read leave
 * their results in words on the game's stack (the file, the bank's place and
 * size in it, SND's handle); they are read from there, where the game's code
 * has them. */
namespace nfs3hp
{
namespace port
{

namespace
{

constexpr reg32 kAllocName = 0x536a58;  // what the bank's memory is allocated as
constexpr reg32 kBankAdd = 0x4e8d20, kBankHeaderSize = 0x4e8db0, kBankHeaderCopy = 0x4e8e20;
constexpr reg32 kSeek = 0x4e9088, kRead = 0x4e9200;
constexpr reg32 kBankNotKept = 7;  // what SND answers when it wants the header copied out
constexpr Var<int32_t> kSurround{0x5525a4};
constexpr Table<reg32> kSpeakerNames{0x412920}, kUnitNames{0x412950};
constexpr reg32 kAudioFolder = 0x7a2ab4;

#pragma pack(push, 1)
struct GameBank
{
    int32_t handle;
    int32_t other;
    reg32 copy;
};
#pragma pack(pop)
constexpr Records<GameBank> kGameBanks{0x571124};

}  // namespace

/* 0x413750: the voice bank of archive eax -- or, no such archive, the
 * units' one (0x412950's names) of the audio folder -- for speaker edx (a
 * name of 0x412920's, now and then the plain one; all for 0x63) into bank
 * slot ebx.  eax as the code leaves it: the memory the bank is kept in. */
void voiceBankLoad(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 frame = e.frame(4);  // ebp as the code has it
    const reg32 name = frame - 0x16c, speakers = frame - 0x6c, units = frame - 0x3c;
    const reg32 fileAt = frame - 0x14, sizeAt = frame - 0x18, offsetAt = frame - 0x1c, handleAt = frame - 0x20;
    const HeldEbp ebp(cpu, frame);
    Game g(app, cpu, name);
    const reg32 archive = e.eax;
    const int32_t speaker = int32_t(e.edx), slot = int32_t(e.ebx);
    // The code's two arrays of names on its stack (an index past their ends
    // reads what lies beyond).
    for (reg32 k = 0; k < 0xc; ++k)
        store<reg32>(app, speakers + 4 * k, kSpeakerNames(app, int32_t(k)));
    for (reg32 k = 0; k < 7; ++k)
        store<reg32>(app, units + 4 * k, kUnitNames(app, int32_t(k)));
    g.esi = kUnitNames.at + 0x1c;
    g.edi = units + 0x1c;
    reg32 eax;
    g.atCall();
    if (!tookEax(guest::direct<kFileExists, ea::fileThere>(app, cpu, archive), eax))
        return;
    reg32 path = archive;
    if (eax == 0)
    {
        // No such archive: the units' bank file of the audio folder.
        eax = reg32(gameSettings(app).language);
        const reg32 unit = load<reg32>(app, units + 4 * eax);
        g.atCall();
        if (!tookEax(cpu, guest::direct<kSprintf, watcom::clib::sprintf>(app, cpu, name, 0x536a28, guest::Words{kAudioFolder, unit}),
                     eax))
            return;
        path = name;
    }
    g.atCall();
    if (!tookEax(guest::direct<kOpenArchive, ea::vivOpened>(app, cpu, path, fileAt), eax))
        return;
    const reg32 file = eax;
    g.edi = file;
    bool plain = speaker == 0x63;
    if (!plain)
    {
        // Now and then (in 16 of 100) the plain one.
        const reg32 random = nextRandom(app);
        eax = random / 100;
        plain = random % 100 <= 0xf;
    }
    if (!plain)
    {
        const reg32 speakerName = load<reg32>(app, speakers + 4 * reg32(speaker));
        g.atCall();
        if (!tookEax(cpu, guest::direct<kSprintf, watcom::clib::sprintf>(app, cpu, name, 0x536a48, guest::Words{speakerName}),
                     eax))
            return;
    }
    else
    {
        g.atCall();
        if (!tookEax(cpu, guest::direct<kSprintf, watcom::clib::sprintf>(app, cpu, name, 0x536a38, guest::Words{}), eax))
            return;
    }
    // Found or not, its eax is not read: the size word says.
    g.atCall();
    if (!guest::direct<kFindInArchive, ea::vivFileFound>(app, cpu, file, name, offsetAt, sizeAt))
        return;
    const reg32 size = load<reg32>(app, sizeAt);
    g.esi = size;
    if (size == 0)
    {
        // The archive has no such bank: it is closed again and nothing loaded.
        g.atCall();
        if (!tookEax(guest::direct<kCloseArchive, ea::vivClosed>(app, cpu, file, fileAt), eax))
            return;
        return e.back(cpu, eax);
    }
    g.atCall();
    if (!tookEax(guest::direct<kReserveMemory, ea::reserved>(app, cpu, kAllocName, size, 0), eax))
        return;
    reg32 memory = eax;
    g.esi = memory;
    // The bank read from its place in the archive.
    g.atCall();
    if (!tookEax(cpu, guest::direct<kSeek, watcom::clib::fseek>(app, cpu, load<reg32>(app, fileAt),
                                                                  load<reg32>(app, offsetAt), 0),
                 eax))
        return;
    g.atCall();
    if (!tookEax(cpu, guest::direct<kRead, watcom::clib::fread>(app, cpu, memory, load<reg32>(app, sizeAt), 1,
                                                                  load<reg32>(app, fileAt)),
                 eax))
        return;
    g.atCall();
    if (!tookEax(guest::direct<kCloseArchive, ea::vivClosed>(app, cpu, file, fileAt), eax)
        || !g.call(kBankAdd, {handleAt, memory}, eax))
        return;
    if (eax == kBankNotKept)
    {
        // SND keeps no bank but its header: that copied out, the bank's memory freed.
        const reg32 handle = load<reg32>(app, handleAt);
        if (!g.call(kBankHeaderSize, {handle}, eax))
            return;
        g.atCall();
        if (!tookEax(guest::direct<kReserveMemory, ea::reserved>(app, cpu, kAllocName, eax, 0), eax))
            return;
        const reg32 copy = eax;
        if (!g.call(kBankHeaderCopy, {copy, load<reg32>(app, handleAt), copy}, eax))
            return;
        g.atCall();
        if (!tookEax(guest::direct<kPurgeMemory, ea::purgeBlock>(app, cpu, memory), eax))
            return;
        memory = copy;
    }
    GameBank& bank = kGameBanks(app, slot);
    bank.handle = int32_t(load<reg32>(app, handleAt));
    bank.copy = memory;
    e.back(cpu, memory);
}

/* 0x4138e0: player edx's car bank of archive eax -- the second player's in
 * the split screen, the first's, or another's (surround or not) -- or,
 * missing from the archive, the audio folder's bank file of that name. */
void carBankLoad(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 frame = e.frame(5);  // ebp as the code has it
    const reg32 name = frame - 0x11c;
    const reg32 fileAt = frame - 0x10, handleAt = frame - 0x14, sizeAt = frame - 0x1c, offsetAt = frame - 0x18;
    const HeldEbp ebp(cpu, frame);
    Game g(app, cpu, name);
    const int32_t player = int32_t(e.edx);
    reg32 eax;
    g.atCall();
    if (!tookEax(guest::direct<kOpenArchive, ea::vivOpened>(app, cpu, e.eax, fileAt), eax))
        return;
    const reg32 archive = eax;
    // The bank's name in the archive, and the file's ending without it.
    bool first = false;
    reg32 inArchive, ending;
    const int32_t split = gameSettings(app).playMode;
    if (split == 1 && split != player)
    {
        inArchive = 0x536a5c;
        ending = 0x536a68;
    }
    else if (player == 0)
    {
        first = true;
        inArchive = 0x536a70;
        ending = 0x536a78;
    }
    else
    {
        inArchive = kSurround(app) != 0 ? 0x536a80 : 0x536a8c;
        ending = 0x536a98;
    }
    // Found or not, its eax is not read: the size word says.
    g.atCall();
    if (!guest::direct<kFindInArchive, ea::vivFileFound>(app, cpu, archive, inArchive, offsetAt, sizeAt))
        return;
    const reg32 size = load<reg32>(app, sizeAt);
    if (first)
        g.edi = size;
    if (size != 0)
    {
        g.atCall();
        if (!tookEax(guest::direct<kReserveMemory, ea::reserved>(app, cpu, kAllocName, size, 0), eax))
            return;
        reg32 memory = eax;
        g.esi = memory;
        // The bank read from its place in the archive.
        g.atCall();
        if (!tookEax(cpu, guest::direct<kSeek, watcom::clib::fseek>(app, cpu, load<reg32>(app, fileAt),
                                                                      load<reg32>(app, offsetAt), 0),
                     eax))
            return;
        g.atCall();
        if (!tookEax(cpu, guest::direct<kRead, watcom::clib::fread>(app, cpu, memory, load<reg32>(app, sizeAt), 1,
                                                                      load<reg32>(app, fileAt)),
                     eax))
            return;
        g.atCall();
        if (!tookEax(guest::direct<kCloseArchive, ea::vivClosed>(app, cpu, archive, fileAt), eax)
            || !g.call(kBankAdd, {handleAt, memory}, eax))
            return;
        if (eax == kBankNotKept)
        {
            // SND keeps no bank but its header: that copied out, the bank's memory freed.
            const reg32 handle = load<reg32>(app, handleAt);
            if (!g.call(kBankHeaderSize, {handle}, eax))
                return;
            g.atCall();
            if (!tookEax(guest::direct<kReserveMemory, ea::reserved>(app, cpu, kAllocName, eax, 0), eax))
                return;
            const reg32 copy = eax;
            if (!g.call(kBankHeaderCopy, {copy, load<reg32>(app, handleAt), 0, copy}, eax))
                return;
            g.atCall();
            if (!tookEax(guest::direct<kPurgeMemory, ea::purgeBlock>(app, cpu, memory), eax))
                return;
            memory = copy;
            g.esi = copy;
        }
        GameBank& bank = kGameBanks(app, player);
        bank.copy = memory;
        const reg32 handle = load<reg32>(app, handleAt);
        bank.handle = int32_t(handle);
        return e.back(cpu, handle);
    }
    // The archive has no such bank: the audio folder's file of its own.
    g.esi = copyText(app, name, kAudioFolder);
    appendText(app, name, ending, &g.esi);
    g.edi = name;
    g.sync();
    if (!bankLoaded(app, cpu, g.esp - 4, name, player, 0, name))
        return;
    g.atCall();
    if (!tookEax(guest::direct<kCloseArchive, ea::vivClosed>(app, cpu, archive, fileAt), eax))
        return;
    e.back(cpu, eax);
}

}  // namespace port
}  // namespace nfs3hp
