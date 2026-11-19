#include "game_settings.h"
#include "native_ea_memory.h"
#include "native_ea_support.h"
#include "native_watcom.h"
#include "port_snd_calls.h"
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

constexpr u32 kAllocName = 0x536a58;  // what the bank's memory is allocated as
constexpr u32 kBankAdd = 0x4e8d20, kBankHeaderSize = 0x4e8db0, kBankHeaderCopy = 0x4e8e20;
constexpr u32 kSeek = 0x4e9088, kRead = 0x4e9200;
constexpr u32 kBankNotKept = 7;  // what SND answers when it wants the header copied out
constexpr Var<int32_t> kSurround{0x5525a4};
constexpr Table<u32> kSpeakerNames{0x412920}, kUnitNames{0x412950};
constexpr u32 kAudioFolder = 0x7a2ab4;

#pragma pack(push, 1)
struct GameBank
{
    int32_t handle;
    int32_t other;
    u32 copy;
};
#pragma pack(pop)
constexpr Records<GameBank> kGameBanks{0x571124};

}  // namespace

/* 0x413750: the voice bank of archive eax -- or, no such archive, the
 * units' one (0x412950's names) of the audio folder -- for speaker edx (a
 * name of 0x412920's, now and then the plain one; all for 0x63) into bank
 * slot ebx.  eax as the code leaves it: the memory the bank is kept in. */
static std::optional<u32> voiceBankLoadWith(App* app, ThreadState& ts, const Entry& e)
{
    const u32 frame = e.frame(4);  // ebp as the code has it
    const u32 name = frame - 0x16c, speakers = frame - 0x6c, units = frame - 0x3c;
    const u32 fileAt = frame - 0x14, sizeAt = frame - 0x18, offsetAt = frame - 0x1c, handleAt = frame - 0x20;
    Game g(app, ts, name);
    u32 eax;
    // SND's bank added from the memory at `memory` (0x4e8d20), its handle to the word at handleAt.
    const auto bankAdd = [&](u32 memory) {
        g.atCall();
        return tookEax(guest::direct<kBankAdd, bankAdded>(app, ts, g.esp - 4, handleAt, memory), eax);
    };
    const u32 archive = e.eax;
    const int32_t speaker = int32_t(e.edx), slot = int32_t(e.ebx);
    // The code's two arrays of names on its stack (an index past their ends
    // reads what lies beyond).
    for (u32 k = 0; k < 0xc; ++k)
        store<u32>(app, speakers + 4 * k, kSpeakerNames(app, int32_t(k)));
    for (u32 k = 0; k < 7; ++k)
        store<u32>(app, units + 4 * k, kUnitNames(app, int32_t(k)));
    g.atCall();
    if (!tookEax(guest::direct<kFileExists, ea::fileThere>(app, ts, archive), eax))
        return std::nullopt;
    u32 path = archive;
    if (eax == 0)
    {
        // No such archive: the units' bank file of the audio folder.
        eax = u32(gameSettings(app).language);
        const u32 unit = load<u32>(app, units + 4 * eax);
        g.atCall();
        if (!tookEax(ts, guest::direct<kSprintf, watcom::clib::sprintf>(app, ts, name, 0x536a28, guest::Words{kAudioFolder, unit}),
                     eax))
            return std::nullopt;
        path = name;
    }
    g.atCall();
    if (!tookEax(guest::direct<kOpenArchive, ea::vivOpened>(app, ts, path, fileAt), eax))
        return std::nullopt;
    const u32 file = eax;
    bool plain = speaker == 0x63;
    if (!plain)
    {
        // Now and then (in 16 of 100) the plain one.
        const u32 random = nextRandom(app);
        eax = random / 100;
        plain = random % 100 <= 0xf;
    }
    if (!plain)
    {
        const u32 speakerName = load<u32>(app, speakers + 4 * u32(speaker));
        g.atCall();
        if (!tookEax(ts, guest::direct<kSprintf, watcom::clib::sprintf>(app, ts, name, 0x536a48, guest::Words{speakerName}),
                     eax))
            return std::nullopt;
    }
    else
    {
        g.atCall();
        if (!tookEax(ts, guest::direct<kSprintf, watcom::clib::sprintf>(app, ts, name, 0x536a38, guest::Words{}), eax))
            return std::nullopt;
    }
    // Found or not, its eax is not read: the size word says.
    g.atCall();
    if (!guest::direct<kFindInArchive, ea::vivFileFound>(app, ts, file, name, offsetAt, sizeAt))
        return std::nullopt;
    const u32 size = load<u32>(app, sizeAt);
    if (size == 0)
    {
        // The archive has no such bank: it is closed again and nothing loaded.
        g.atCall();
        if (!tookEax(guest::direct<kCloseArchive, ea::vivClosed>(app, ts, file, fileAt), eax))
            return std::nullopt;
        ts.sp = e.esp + 4;
        return eax;
    }
    g.atCall();
    if (!tookEax(guest::direct<kReserveMemory, ea::reserved>(app, ts, kAllocName, size, 0), eax))
        return std::nullopt;
    u32 memory = eax;
    // The bank read from its place in the archive.
    g.atCall();
    if (!tookEax(ts, guest::direct<kSeek, watcom::clib::fseek>(app, ts, load<u32>(app, fileAt),
                                                                  load<u32>(app, offsetAt), 0),
                 eax))
        return std::nullopt;
    g.atCall();
    if (!tookEax(ts, guest::direct<kRead, watcom::clib::fread>(app, ts, memory, load<u32>(app, sizeAt), 1,
                                                                  load<u32>(app, fileAt)),
                 eax))
        return std::nullopt;
    g.atCall();
    if (!tookEax(guest::direct<kCloseArchive, ea::vivClosed>(app, ts, file, fileAt), eax)
        || !bankAdd(memory))
        return std::nullopt;
    if (eax == kBankNotKept)
    {
        // SND keeps no bank but its header: that copied out, the bank's memory freed.
        const u32 handle = load<u32>(app, handleAt);
        g.atCall();
        if (!tookEax(guest::direct<kBankHeaderSize, bankHeaderSize>(app, ts, g.esp - 4, int32_t(handle)), eax))
            return std::nullopt;
        g.atCall();
        if (!tookEax(guest::direct<kReserveMemory, ea::reserved>(app, ts, kAllocName, eax, 0), eax))
            return std::nullopt;
        const u32 copy = eax;
        g.atCall();
        if (!tookEax(guest::direct<kBankHeaderCopy, copyBankHeader>(app, ts, g.esp - 4, copy,
                                                                    int32_t(load<u32>(app, handleAt))),
                     eax))
            return std::nullopt;
        g.atCall();
        if (!tookEax(guest::direct<kPurgeMemory, ea::purgeBlock>(app, ts, memory), eax))
            return std::nullopt;
        memory = copy;
    }
    GameBank& bank = kGameBanks(app, slot);
    bank.handle = int32_t(load<u32>(app, handleAt));
    bank.copy = memory;
    ts.sp = e.esp + 4;
    return memory;
}

nfs::u32 voiceBankLoad(App* app, ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> memory = voiceBankLoadWith(app, ts, Entry(ts, args));
    if (memory)
        args.eax = *memory;
    return args.eax;
}

/* 0x413750's core: voiceBankLoad of archive `archive` for speaker `speaker` into slot `slot`; the memory the
 * bank is kept in (what eax holds), nullopt when the game stopped; esp the frame base. */
std::optional<u32> voiceBankLoaded(App* app, ThreadState& ts, u32 esp, u32 archive, int32_t speaker, int32_t slot)
{
    nfs::Args args;
    args.eax = archive;
    args.edx = u32(speaker);
    args.ebx = u32(slot);
    Entry e(ts, args);
    e.esp = esp;
    return voiceBankLoadWith(app, ts, e);
}

/* 0x4138e0's: player `player`'s car bank of archive `archiveName` -- the
 * second player's in the split screen, the first's, or another's (surround or
 * not) -- or, missing from the archive, the audio folder's bank file of that
 * name.  eax as the code leaves it. */
Result carBankLoaded(App* app, ThreadState& ts, u32 esp, u32 archiveName, int32_t player)
{
    const u32 frame = esp - 5 * 4;  // ebp as the code has it
    const u32 name = frame - 0x11c;
    const u32 fileAt = frame - 0x10, handleAt = frame - 0x14, sizeAt = frame - 0x1c, offsetAt = frame - 0x18;
    Game g(app, ts, name);
    u32 eax;
    // SND's bank added from the memory at `memory` (0x4e8d20), its handle to the word at handleAt.
    const auto bankAdd = [&](u32 memory) {
        g.atCall();
        return tookEax(guest::direct<kBankAdd, bankAdded>(app, ts, g.esp - 4, handleAt, memory), eax);
    };
    g.atCall();
    if (!tookEax(guest::direct<kOpenArchive, ea::vivOpened>(app, ts, archiveName, fileAt), eax))
        return {};
    const u32 archive = eax;
    // The bank's name in the archive, and the file's ending without it.
    u32 inArchive, ending;
    const int32_t split = gameSettings(app).playMode;
    if (split == 1 && split != player)
    {
        inArchive = 0x536a5c;
        ending = 0x536a68;
    }
    else if (player == 0)
    {
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
    if (!guest::direct<kFindInArchive, ea::vivFileFound>(app, ts, archive, inArchive, offsetAt, sizeAt))
        return {};
    const u32 size = load<u32>(app, sizeAt);
    if (size != 0)
    {
        g.atCall();
        if (!tookEax(guest::direct<kReserveMemory, ea::reserved>(app, ts, kAllocName, size, 0), eax))
            return {};
        u32 memory = eax;
        // The bank read from its place in the archive.
        g.atCall();
        if (!tookEax(ts, guest::direct<kSeek, watcom::clib::fseek>(app, ts, load<u32>(app, fileAt),
                                                                      load<u32>(app, offsetAt), 0),
                     eax))
            return {};
        g.atCall();
        if (!tookEax(ts, guest::direct<kRead, watcom::clib::fread>(app, ts, memory, load<u32>(app, sizeAt), 1,
                                                                      load<u32>(app, fileAt)),
                     eax))
            return {};
        g.atCall();
        if (!tookEax(guest::direct<kCloseArchive, ea::vivClosed>(app, ts, archive, fileAt), eax)
            || !bankAdd(memory))
            return {};
        if (eax == kBankNotKept)
        {
            // SND keeps no bank but its header: that copied out, the bank's memory freed.
            const u32 handle = load<u32>(app, handleAt);
            g.atCall();
            if (!tookEax(guest::direct<kBankHeaderSize, bankHeaderSize>(app, ts, g.esp - 4, int32_t(handle)), eax))
                return {};
            g.atCall();
            if (!tookEax(guest::direct<kReserveMemory, ea::reserved>(app, ts, kAllocName, eax, 0), eax))
                return {};
            const u32 copy = eax;
            g.atCall();
            if (!tookEax(guest::direct<kBankHeaderCopy, copyBankHeader>(app, ts, g.esp - 4, copy,
                                                                        int32_t(load<u32>(app, handleAt))),
                         eax))
                return {};
            g.atCall();
            if (!tookEax(guest::direct<kPurgeMemory, ea::purgeBlock>(app, ts, memory), eax))
                return {};
            memory = copy;
        }
        GameBank& bank = kGameBanks(app, player);
        bank.copy = memory;
        const u32 handle = load<u32>(app, handleAt);
        bank.handle = int32_t(handle);
        return int32_t(handle);
    }
    // The archive has no such bank: the audio folder's file of its own.
    copyText(app, name, kAudioFolder);
    appendText(app, name, ending);
    if (!bankLoaded(app, ts, g.esp - 4, name, player, 0, name))
        return {};
    g.atCall();
    if (!tookEax(guest::direct<kCloseArchive, ea::vivClosed>(app, ts, archive, fileAt), eax))
        return {};
    return int32_t(eax);
}

/* 0x4138e0: player edx's car bank of archive eax. */
nfs::u32 carBankLoad(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, carBankLoaded(app, ts, e.esp, e.eax, int32_t(e.edx)));
}

}  // namespace port
}  // namespace nfs3hp
