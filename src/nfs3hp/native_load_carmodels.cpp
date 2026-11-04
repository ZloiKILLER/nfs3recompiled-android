#include "native_guest.h"
#include "port_thrash.h"

#include <utility>

/* The game's data: the cars' models and specs -- a car's carp.txt sent and
 * received over the network (the game's code from 0x49b540 to 0x49b9a9), the
 * cars' .fce models with their colours and their textures, from the .art,
 * the 00.tga or the car's .viv (0x49c700 to 0x49d596, 0x4b7a60, 0x4b7ba0),
 * and the players' own specs, carspecs\player1.spc and player2.spc (0x4dc3a0
 * to 0x4dcae1).  disassemble_nfs3hp.py binds each address to the function
 * of that name here (LOAD_CARMODELS_NATIVES), and the generated code calls it
 * by name.  tools/game_checks holds each against its generated code.
 *
 * A car's carp.txt goes over the network as a header of 0x1c bytes (0 its
 * kind, 1 the car's id of 16 bytes, 0x14 its record's +0x380, 0x18 the
 * text's size) and then its bytes; the receiver keeps a record of its own
 * (0x55e5bc: +0 done, +4 the size, +8 the buffer, +0xc what is in, +0x10 the
 * FILE it is written to, the data path's Carp.txt, when there is no buffer).
 * Whole, the text is put into the car's folder's car.viv with the others.
 *
 * A car's model (the .fce loaded, made ready by 0x49cfa0) holds its
 * pictures' count at +0xc; its colours, 16 bytes each (hue, saturation,
 * brightness in 8.8 parts of a turn, and a fourth passed along), [+0x7fc] of
 * them at +0x800, and as many second colours ([+0x900]) at +0x904; its
 * textures' handles at +0x1e04.  A texture is made of the pictures' 32-bit
 * pixels (red, green, blue, alpha) scaled to a power of two no larger than
 * the size asked for, as 1555 words in a block of the game's memory (+0 the
 * width, +4 the height, +8 the pixels); a pixel with the top bit of its
 * alpha clear takes the car's colour (0x49cc50), one not quite opaque the
 * second colour.  The .art holds the pictures one after the other (+0 the
 * width, +4 the height, +8 the pixels); a .tga one picture (its 18 bytes'
 * header, the width at +0xc, the height at +0xe, its id's length at +0).
 *
 * The players' specs: a table of 50 for each player (0x8b9200 the first's,
 * 0x8b88a0 the second's), 0x30 bytes each: +0 a byte, whether it is in use,
 * +4 the car's id of 16 bytes, +0x14 the car's spec of 0x1c bytes (0x55d024
 * the default).  The file is the table whole; one not of this machine
 * (0x472940, by the game's module's name) is deleted and made anew.
 *
 * Written as their code is: each push and pop to the guest's stack, each call
 * with the return address's slot, the frame's locals in the guest's memory,
 * the registers as the code leaves them; strings copied two bytes a turn as
 * Watcom's inlined strcpy and strcat do, blocks a dword then a byte at a
 * time; the x87's stack pushed and popped as the code has it, every operation
 * through cpu.fpu.  The hue's switch goes by the targets nfs3.exe's table
 * holds (0x49ca34).  Every other function of the game's, and Windows' through
 * its imports, is called by address. */
namespace nfs3hp
{
namespace load
{

namespace
{

using guest::at;
using guest::byte;
using guest::put;
using guest::putByte;
using x86::reg16;
using x86::reg32;
using x86::reg8;
using x86::sreg32;

// The network's carp.txt: the data path, the file it is written to, the
// receiver's record and what it takes next (0x12c bytes into the transfer's
// name), the .viv it is read from; the transfer's state's words.
constexpr reg32 kDataPath = 0x7a28b4, kCarpFile = 0x79faac, kReceiver = 0x55e5bc, kTransferName = 0x79f97c;
constexpr reg32 kNextBytes = 0x12c, kSendViv = 0x79f978, kTransferTitle = 0x79f5e0, kTransferWord = 0x79faa8;
// Its strings: "wb", "Carp.txt", "car.viv", "\carp.txt", "car.VIV".
constexpr reg32 kWriteMode = 0x53c590, kCarpName = 0x53c584, kCarVivName = 0x53c594, kCarpPath = 0x53c59c;
constexpr reg32 kCarVivPath = 0x53c5a8;
// The models': an empty name (the blocks' tag), "%s.viv", "car.fce", the
// trace's file and format, "%s.fce", "%s00.tga", "%s.art"; the .viv's ones.
constexpr reg32 kNoName = 0x53c60c, kVivFormat = 0x53c62c, kFceName = 0x53c634, kTraceFile = 0x53c63c;
constexpr reg32 kTraceFormat = 0x53c658, kFceFormat = 0x53c670, kTgaFormat = 0x53c678, kArtFormat = 0x53c684;
constexpr reg32 kVivFceFormat = 0x53ff70, kVivNoName = 0x53ff78, kVivTgaFormat = 0x53ff7c;
// Their numbers: 6 (a double, the hue's sixths), 255 (a double); 1000 (a
// float's bits, the nearest colour's distance to beat); the two records of
// zeros nfs3.exe keeps among its code (0x4b7a10, 0x4b7a20).
constexpr reg32 kSix = 0x53c660, kFull = 0x53c668, kThousand = 0x447a0000;
constexpr reg32 kZeros = 0x4b7a10, kMoreZeros = 0x4b7a20;
// Whether a pixel all black but its top bit is drawn as 0x8001.
constexpr reg32 kBlackVisible = 0x7a3a6c;
// The specs: the players' tables, their names, the default spec; the path
// made, the folder before it, "carspecs\", ".spc".
constexpr reg32 kFirstSpecs = 0x8b9200, kSecondSpecs = 0x8b88a0, kSpecsBytes = 0x960, kSpecBytes = 0x30;
constexpr reg32 kSpecCount = 0x32, kFirstPlayer = 0x549784, kSecondPlayer = 0x54978c, kDefaultSpec = 0x55d024;
constexpr reg32 kSpecPath = 0x564188, kSpecFolder = 0x7a2fb4, kSpecsFolder = 0x549770, kSpecExtension = 0x54977c;
// What is called by address: the paths' backslash, a file copied, the
// .viv's name the new car goes into, a file put into a .viv, the cars'
// data, the transfer's state; fopen, fclose, fwrite, mkdir, strrchr; a car's
// record, its id, its folder's name, its name, its index by id; sprintf; a
// .viv opened, a file found in it, closed; fseek, fread; a packet sent; the
// game's trace; the game's memory, memmove; a file loaded, there; a model
// made ready; a texture allocated, its pixels given; vectors less another, a
// vector's length; a file's place in a .viv in memory; __CHP; memset; a
// player's name valid; GetModuleFileNameA; a spec file of this machine's,
// unlink, a file's size, a file loaded quietly, access, chmod, a file saved.
constexpr reg32 kEndPath = 0x4529a0, kCopyFile = 0x452a00, kNewCarName = 0x43ba20, kAddToViv = 0x43baa0;
constexpr reg32 kMarkCar = 0x4385a0, kCarsData = 0x439d90, kTransferTitleSet = 0x43bc00;
constexpr reg32 kTransferNameSet = 0x43bc40, kTransferWordSet = 0x43bc70, kTransferDone = 0x43bc90;
constexpr reg32 kFopen = 0x4edfe8, kFclose = 0x4ee100, kFwrite = 0x4eebd0, kMkdir = 0x4eb1d0, kStrrchr = 0x4ee2f0;
constexpr reg32 kCarRecord = 0x439a50, kCarId = 0x439520, kCarFolder = 0x4392b0, kCarName = 0x439250;
constexpr reg32 kCarIndex = 0x439990, kSprintf = 0x4df690;
constexpr reg32 kOpenViv = 0x43b830, kFindInViv = 0x43b9d0, kCloseViv = 0x43b9b0, kFseek = 0x4e9088;
constexpr reg32 kFread = 0x4e9200, kSendPacket = 0x4a60e0, kTrace = 0x42d510;
constexpr reg32 kAllocate = 0x4e1620, kFree = 0x4e1890, kMoveMemory = 0x4ea4f0;
constexpr reg32 kLoadFile = 0x4e0ec0, kFileExists = 0x4e0e60, kModelReady = 0x49cfa0;
constexpr reg32 kAllocTexture = 0x49c600, kUpdateTexture = 0x49c620, kSubtract = 0x4e0000, kLength = 0x4e06a0;
constexpr reg32 kVivEntry = 0x4ea290, kChop = 0x4dfd56, kMemset = 0x4e0640, kNameValid = 0x4721f0;
constexpr reg32 kGetModuleFileName = 0x534544, kOwnSpecs = 0x472940, kUnlink = 0x4e28a0, kFileSize = 0x4e43a0;
constexpr reg32 kLoadQuietly = 0x4e0fe0, kAccess = 0x4f05f0, kChmod = 0x4ee3c0, kSaveFile = 0x4eebb0;

void push(win32::WinApplication* app, x86::CPU& cpu, reg32 value)
{
    cpu.esp -= 4;
    put(app, cpu.esp, value);
}

reg32 pop(win32::WinApplication* app, x86::CPU& cpu)
{
    const reg32 value = at(app, cpu.esp);
    cpu.esp += 4;
    return value;
}

/* A call as the code makes it: the return address's slot, and esp as the
 * callee leaves it.  False when the game stopped in it. */
bool direct(win32::WinApplication* app, x86::CPU& cpu, void (*function)(win32::WinApplication*, x86::CPU&))
{
    cpu.esp -= 4;
    function(app, cpu);
    return !cpu.terminate;
}

bool byAddress(win32::WinApplication* app, x86::CPU& cpu, reg32 target)
{
    cpu.esp -= 4;
    app->dynamic_call(target, cpu);
    return !cpu.terminate;
}

reg16 half(win32::WinApplication* app, reg32 address)
{
    return app->getMemory<reg16>(address);
}

void putHalf(win32::WinApplication* app, reg32 address, reg16 value)
{
    app->getMemory<reg16>(address) = value;
}

x86::Float f32(win32::WinApplication* app, reg32 address)
{
    return x86::Float(app->getMemory<float>(address));
}

x86::Float f64(win32::WinApplication* app, reg32 address)
{
    return x86::Float(app->getMemory<double>(address));
}

void putF32(win32::WinApplication* app, reg32 address, const x86::Float& value)
{
    app->getMemory<float>(address) = float(value);
}

/* fild: the dword at `address` onto the x87's stack. */
void loadInteger(win32::WinApplication* app, x86::CPU& cpu, reg32 address)
{
    cpu.fpu.push(x86::Float(sreg32(at(app, address))));
}

/* fistp: st(0) to the dword at `address` as the control word rounds it, popped. */
void storeInteger(win32::WinApplication* app, x86::CPU& cpu, reg32 address)
{
    put(app, address, reg32(fp::toInteger<sreg32>(cpu.fpu.st(0))));
    cpu.fpu.pop();
}

/* push edi; then two bytes a turn from esi to edi to the string's end; pop
 * edi: esi and al as the copy leaves them. */
void copyString(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.edi);
    for (;;)
    {
        cpu.al = byte(app, cpu.esi);
        putByte(app, cpu.edi, cpu.al);
        if (cpu.al == 0)
            break;
        cpu.al = byte(app, cpu.esi + 1);
        cpu.esi += 2;
        putByte(app, cpu.edi + 1, cpu.al);
        cpu.edi += 2;
        if (cpu.al == 0)
            break;
    }
    cpu.edi = pop(app, cpu);
}

/* push edi; sub ecx, ecx; dec ecx; mov al, 0; repne scasb; dec edi; then
 * the copy from esi to the string's end; pop edi: Watcom's strcat, ecx as the
 * scan leaves it. */
void appendString(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.edi);
    cpu.ecx = 0xffffffff;
    cpu.al = 0;
    while (cpu.ecx != 0)
    {
        --cpu.ecx;
        const reg8 c = byte(app, cpu.edi);
        ++cpu.edi;
        if (c == cpu.al)
            break;
    }
    --cpu.edi;
    for (;;)
    {
        cpu.al = byte(app, cpu.esi);
        putByte(app, cpu.edi, cpu.al);
        if (cpu.al == 0)
            break;
        cpu.al = byte(app, cpu.esi + 1);
        cpu.esi += 2;
        putByte(app, cpu.edi + 1, cpu.al);
        cpu.edi += 2;
        if (cpu.al == 0)
            break;
    }
    cpu.edi = pop(app, cpu);
}

/* push edi; mov eax, ecx; shr ecx, 2; rep movsd; mov cl, al; and cl, 3;
 * rep movsb; pop edi: ecx bytes from esi to edi, eax the count, ecx 0, esi
 * past them. */
void copyBytes(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.edi);
    cpu.eax = cpu.ecx;
    cpu.ecx >>= 2;
    for (; cpu.ecx != 0; --cpu.ecx)
    {
        put(app, cpu.edi, at(app, cpu.esi));
        cpu.edi += 4;
        cpu.esi += 4;
    }
    cpu.cl = cpu.al;
    cpu.cl &= 3;
    for (; cpu.ecx != 0; --cpu.ecx)
        putByte(app, cpu.edi++, byte(app, cpu.esi++));
    cpu.edi = pop(app, cpu);
}

/* idiv `divisor`: edx:eax by it, eax the quotient, edx the remainder --
 * worked in 64 bits as the generated code does, the quotient's low half. */
void idivide(x86::CPU& cpu, reg32 divisor)
{
    const x86::sreg64 dividend = x86::sreg64(cpu.edx_eax);
    const x86::sreg64 by = sreg32(divisor);
    cpu.eax = reg32(x86::reg64(dividend / by));
    cpu.edx = reg32(x86::reg64(dividend % by));
}

/* cdq; idiv ebx; mov ecx, eax; xor eax, eax; shrd eax, edx, 0x10; sar edx,
 * 0x10; idiv ebx; shl ecx, 0x10; add eax, ecx: eax over ebx in 16.16. */
void fixedDivide(x86::CPU& cpu)
{
    cpu.edx = reg32(sreg32(cpu.eax) >> 31);
    idivide(cpu, cpu.ebx);
    cpu.ecx = cpu.eax;
    cpu.eax = cpu.edx << 16;
    cpu.edx = reg32(sreg32(cpu.edx) >> 16);
    idivide(cpu, cpu.ebx);
    cpu.ecx <<= 16;
    cpu.eax += cpu.ecx;
}

/* imul edx; shl edx, 0x10; shr eax, 0x10; adc eax, edx: eax times edx in
 * 16.16, rounded by the bit shifted out. */
void fixedMultiply(x86::CPU& cpu)
{
    const x86::sreg64 product = x86::sreg64(sreg32(cpu.eax)) * sreg32(cpu.edx);
    const reg32 low = reg32(x86::reg64(product));
    cpu.edx = reg32(x86::reg64(product) >> 32) << 16;
    cpu.eax = (low >> 16) + cpu.edx + ((low >> 15) & 1);
}

/* Which table of specs player eax's is: the first's, the second's, or none. */
reg32 specsOf(reg32 player)
{
    return player == 0 ? kFirstSpecs : player == 1 ? kSecondSpecs : 0;
}

}  // namespace

void beginCarpReceive(win32::WinApplication* app, x86::CPU& cpu);
void receiveCarpChunk(win32::WinApplication* app, x86::CPU& cpu);
void beginCarpSend(win32::WinApplication* app, x86::CPU& cpu);
void readCarColors(win32::WinApplication* app, x86::CPU& cpu);
void rgbToHsv(win32::WinApplication* app, x86::CPU& cpu);
void hsvToRgb(win32::WinApplication* app, x86::CPU& cpu);
void tintPixel(win32::WinApplication* app, x86::CPU& cpu);
void scaleTexture(win32::WinApplication* app, x86::CPU& cpu);
void fitTexture(win32::WinApplication* app, x86::CPU& cpu);
void loadArtTextures(win32::WinApplication* app, x86::CPU& cpu);
void nearestModelColor(win32::WinApplication* app, x86::CPU& cpu);
void loadTgaTexture(win32::WinApplication* app, x86::CPU& cpu);
void loadCarModel(win32::WinApplication* app, x86::CPU& cpu);
void loadVivCarModel(win32::WinApplication* app, x86::CPU& cpu);
void copyVivCarModel(win32::WinApplication* app, x86::CPU& cpu);
void setSpecUsed(win32::WinApplication* app, x86::CPU& cpu);
void specUsed(win32::WinApplication* app, x86::CPU& cpu);
void specCarName(win32::WinApplication* app, x86::CPU& cpu);
void findSpec(win32::WinApplication* app, x86::CPU& cpu);
void freeSpecSlot(win32::WinApplication* app, x86::CPU& cpu);
void setSpec(win32::WinApplication* app, x86::CPU& cpu);
void defaultSpecs(win32::WinApplication* app, x86::CPU& cpu);
void refreshSpecs(win32::WinApplication* app, x86::CPU& cpu);
void specPath(win32::WinApplication* app, x86::CPU& cpu);
void readSpecs(win32::WinApplication* app, x86::CPU& cpu);
void loadPlayerSpecs(win32::WinApplication* app, x86::CPU& cpu);
void savePlayerSpecs(win32::WinApplication* app, x86::CPU& cpu);

/* 0x49b540: the receiver's record eax made ready for ebx bytes: into the
 * buffer edx, or with none into the data path's Carp.txt, opened to write
 * (0x4edfe8); 0, or -1 when it would not open. */
void beginCarpReceive(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    put(app, cpu.ebp - 4, cpu.eax);
    put(app, cpu.eax + 0xc, 0);
    put(app, cpu.eax, 0);
    put(app, cpu.eax + 4, cpu.ebx);
    bool failed = false;
    if (cpu.edx != 0)
    {
        put(app, cpu.eax + 0x10, 0);
        put(app, cpu.eax + 8, cpu.edx);
    }
    else
    {
        cpu.esi = kDataPath;
        cpu.edi = kCarpFile;
        cpu.edx = kWriteMode;
        copyString(app, cpu);
        cpu.eax = cpu.edi;
        cpu.esi = kCarpName;
        if (!byAddress(app, cpu, kEndPath))
            return;
        appendString(app, cpu);
        cpu.eax = cpu.edi;
        if (!byAddress(app, cpu, kFopen))
            return;
        cpu.edx = at(app, cpu.ebp - 4);
        put(app, cpu.edx + 0x10, cpu.eax);
        failed = cpu.eax == 0;
    }
    cpu.eax = failed ? 0xffffffff : 0;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x49b5f0: ebx bytes at edx taken by the receiver's record eax, unless it is
 * done: written to its file (fwrite; failing, the file closed, the record
 * done, -1) or copied into its buffer.  All in, the file closed and the
 * receiver made ready for the next 0x12c bytes, into the transfer's name; or,
 * from the buffer, the record done and the car put together in its folder
 * (the data path's, made): its carp.txt copied in, car.viv made of it and
 * the others, the cars' data and the transfer's state brought up to date.
 * 0. */
void receiveCarpChunk(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x260;
    const reg32 record = cpu.ebp - 4, count = cpu.ebp - 8;
    const reg32 folder = cpu.ebp - 0x134, carp = cpu.ebp - 0x260;
    put(app, record, cpu.eax);
    put(app, count, cpu.ebx);
    cpu.eax = cpu.edx;
    cpu.edx = at(app, record);
    const auto done = [&](reg32 result) {
        cpu.eax = result;
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.esp += 4;
    };
    if (at(app, cpu.edx) != 0)
        return done(0);
    cpu.ebx = at(app, cpu.edx + 0x10);
    if (cpu.ebx != 0)
    {
        cpu.edx = at(app, count);
        cpu.ecx = cpu.ebx;
        cpu.ebx = 1;
        if (!byAddress(app, cpu, kFwrite))
            return;
        if (cpu.eax == 0)
        {
            cpu.eax = at(app, record);
            cpu.eax = at(app, cpu.eax + 0x10);
            if (!byAddress(app, cpu, kFclose))
                return;
            cpu.eax = at(app, record);
            put(app, cpu.eax + 0x10, 0);
            put(app, cpu.eax, 1);
            return done(0xffffffff);
        }
    }
    else
    {
        cpu.edi = at(app, cpu.edx + 8);
        cpu.esi = at(app, cpu.edx + 0xc);
        cpu.ecx = at(app, count);
        cpu.edi += cpu.esi;
        cpu.esi = cpu.eax;
        copyBytes(app, cpu);
    }
    cpu.edx = at(app, record);
    cpu.eax = at(app, count);
    cpu.edi = at(app, cpu.edx + 0xc);
    cpu.edi += cpu.eax;
    cpu.ecx = at(app, cpu.edx + 4);
    put(app, cpu.edx + 0xc, cpu.edi);
    if (cpu.edi != cpu.ecx)
        return done(0);
    cpu.ebx = at(app, cpu.edx + 0x10);
    if (cpu.ebx != 0)
    {
        cpu.eax = cpu.ebx;
        if (!byAddress(app, cpu, kFclose))
            return;
        cpu.ebx = kNextBytes;
        cpu.eax = kReceiver;
        put(app, cpu.edx + 0x10, 0);
        cpu.edx = kTransferName;
        if (!direct(app, cpu, beginCarpReceive))
            return;
        return done(0);
    }
    put(app, cpu.edx, 1);
    cpu.esi = kDataPath;
    cpu.edi = folder;
    copyString(app, cpu);
    cpu.eax = folder;
    cpu.edx = 0x5c;
    if (!byAddress(app, cpu, kEndPath))
        return;
    cpu.eax = folder;
    if (!byAddress(app, cpu, kStrrchr))
        return;
    ++cpu.eax;
    cpu.esi = kCarpName;
    if (!byAddress(app, cpu, kMarkCar))
        return;
    cpu.eax = folder;
    cpu.edi = folder;
    if (!byAddress(app, cpu, kMkdir))
        return;
    cpu.eax = folder;
    cpu.edx = kCarpFile;
    if (!byAddress(app, cpu, kEndPath))
        return;
    appendString(app, cpu);
    cpu.eax = folder;
    cpu.esi = folder;
    cpu.edi = carp;
    if (!byAddress(app, cpu, kCopyFile))
        return;
    copyString(app, cpu);
    cpu.edx = 0x5c;
    cpu.eax = carp;
    cpu.esi = kCarVivName;
    if (!byAddress(app, cpu, kStrrchr))
        return;
    cpu.edi = cpu.eax + 1;
    copyString(app, cpu);
    if (!byAddress(app, cpu, kNewCarName))
        return;
    cpu.edx = cpu.eax;
    cpu.eax = carp;
    if (!byAddress(app, cpu, kCopyFile))
        return;
    cpu.eax = carp;
    if (!byAddress(app, cpu, kAddToViv) || !byAddress(app, cpu, kCarsData))
        return;
    cpu.eax = kTransferTitle;
    if (!byAddress(app, cpu, kTransferTitleSet))
        return;
    cpu.eax = kTransferName;
    if (!byAddress(app, cpu, kTransferNameSet))
        return;
    cpu.eax = at(app, kTransferWord);
    if (!byAddress(app, cpu, kTransferWordSet) || !byAddress(app, cpu, kTransferDone)
        || !byAddress(app, cpu, kCarsData))
        return;
    done(0);
}

/* 0x49b800: car edx's carp.txt made ready to send: the header at eax (+1 the
 * car's id, +0x14 its record's +0x380, +0x18 the text's size) filled in from
 * the car's folder's car.VIV, opened (its handle to 0x79f978) and its FILE
 * put at the text; the car's name to the transfer's name, and the header's
 * 0x1c bytes sent to every machine.  The FILE. */
void beginCarpSend(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x270;
    const reg32 header = cpu.ebp - 4, car = cpu.ebp - 8, carRecord = cpu.ebp - 0xc, size = cpu.ebp - 0x10;
    const reg32 file = cpu.ebp - 0x14, place = cpu.ebp - 0x18, viv = cpu.ebp - 0x144, path = cpu.ebp - 0x270;
    put(app, header, cpu.eax);
    put(app, car, cpu.edx);
    cpu.edi = at(app, header);
    cpu.eax = cpu.edx;
    cpu.ecx = 0x10;
    if (!byAddress(app, cpu, kCarRecord))
        return;
    put(app, carRecord, cpu.eax);
    cpu.eax = cpu.edx;
    ++cpu.edi;
    if (!byAddress(app, cpu, kCarId))
        return;
    cpu.esi = cpu.eax;
    copyBytes(app, cpu);
    cpu.esi = kDataPath;
    cpu.edi = path;
    copyString(app, cpu);
    cpu.eax = path;
    if (!byAddress(app, cpu, kEndPath))
        return;
    cpu.eax = cpu.edx;
    cpu.edi = path;
    if (!byAddress(app, cpu, kCarFolder))
        return;
    cpu.esi = cpu.eax;
    appendString(app, cpu);
    cpu.esi = kCarpPath;
    cpu.edi = path;
    appendString(app, cpu);
    cpu.esi = path;
    cpu.edi = viv;
    cpu.edx = 0x5c;
    copyString(app, cpu);
    cpu.eax = viv;
    if (!byAddress(app, cpu, kStrrchr))
        return;
    push(app, cpu, kCarVivPath);
    ++cpu.eax;
    push(app, cpu, cpu.eax);
    if (!byAddress(app, cpu, kSprintf))
        return;
    cpu.esp += 8;
    cpu.edx = file;
    cpu.eax = viv;
    if (!byAddress(app, cpu, kOpenViv))
        return;
    put(app, kSendViv, cpu.eax);
    cpu.edx = 0x5c;
    cpu.eax = path;
    cpu.ecx = size;
    if (!byAddress(app, cpu, kStrrchr))
        return;
    cpu.edx = cpu.eax + 1;
    cpu.ebx = place;
    cpu.eax = at(app, kSendViv);
    if (!byAddress(app, cpu, kFindInViv))
        return;
    cpu.edx = at(app, place);
    cpu.eax = at(app, file);
    cpu.ebx = 0;
    if (!byAddress(app, cpu, kFseek))
        return;
    cpu.edx = at(app, header);
    cpu.eax = at(app, size);
    put(app, cpu.edx + 0x18, cpu.eax);
    cpu.eax = at(app, carRecord);
    cpu.eax = at(app, cpu.eax + 0x380);
    put(app, cpu.edx + 0x14, cpu.eax);
    cpu.eax = at(app, car);
    cpu.edi = kTransferName;
    if (!byAddress(app, cpu, kCarName))
        return;
    cpu.esi = cpu.eax;
    cpu.ebx = 0x1c;
    copyString(app, cpu);
    cpu.eax = 0;
    if (!byAddress(app, cpu, kSendPacket))
        return;
    cpu.eax = at(app, file);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x49c700: the colours of the model car.fce in the .viv named eax ("%s.viv"):
 * their count ([+0x7fc]) to [ecx], the first ebx of them (16 at most) as
 * red, green, blue and the fourth, a byte each, to edx on; its 12 bytes at
 * +0x28 to [esp+4].  Not found, a trace of mrc.c's line 0x5a and [ecx] 0. */
void readCarColors(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x254;
    const reg32 found = cpu.ebp - 4, place = cpu.ebp - 8, wanted = cpu.ebp - 0xc, colors = cpu.ebp - 0x10;
    const reg32 file = cpu.ebp - 0x14, count = cpu.ebp - 0x18, rgb = cpu.ebp - 0x28, hsv = cpu.ebp - 0x128;
    const reg32 name = cpu.ebp - 0x254;
    const auto seekFrom = [&](reg32 offset) {
        cpu.edx = at(app, place);
        cpu.eax = at(app, file);
        cpu.ebx = 0;
        if (!byAddress(app, cpu, kFseek))
            return false;
        cpu.ebx = 1;
        cpu.edx = offset;
        cpu.eax = at(app, file);
        return byAddress(app, cpu, kFseek);
    };
    const auto read = [&](reg32 to, reg32 bytes) {
        cpu.ecx = at(app, file);
        cpu.ebx = 1;
        cpu.edx = bytes;
        cpu.eax = to;
        return byAddress(app, cpu, kFread);
    };
    put(app, wanted, cpu.ebx);
    cpu.edi = cpu.ecx;
    push(app, cpu, cpu.eax);
    cpu.ebx = place;
    push(app, cpu, kVivFormat);
    cpu.eax = name;
    put(app, colors, cpu.edx);
    push(app, cpu, cpu.eax);
    put(app, cpu.ecx, 0);
    cpu.edx = file;
    if (!byAddress(app, cpu, kSprintf))
        return;
    cpu.esp += 0xc;
    cpu.eax = name;
    cpu.ecx = found;
    if (!byAddress(app, cpu, kOpenViv))
        return;
    cpu.edx = kFceName;
    cpu.esi = cpu.eax;
    if (!byAddress(app, cpu, kFindInViv))
        return;
    if (at(app, found) == 0)
    {
        cpu.edx = file;
        cpu.eax = cpu.esi;
        if (!byAddress(app, cpu, kCloseViv))
            return;
        push(app, cpu, 0x104);
        push(app, cpu, kTraceFile);
        push(app, cpu, kTraceFormat);
        push(app, cpu, 1);
        push(app, cpu, 0x5a);
        if (!byAddress(app, cpu, kTrace))
            return;
        cpu.esp += 0x14;
    }
    else
    {
        if (!seekFrom(0x7fc) || !read(count, 4) || !seekFrom(0x800) || !read(hsv, 0x100) || !seekFrom(0x28)
            || !read(at(app, cpu.ebp + 0x10), 0xc))
            return;
        cpu.edx = file;
        cpu.eax = cpu.esi;
        if (!byAddress(app, cpu, kCloseViv))
            return;
        cpu.ecx = 0;
        while (sreg32(cpu.ecx) < sreg32(at(app, wanted)) && cpu.ecx < 0x10)
        {
            cpu.edx = cpu.ecx;
            cpu.eax = hsv;
            cpu.edx <<= 4;
            cpu.eax += cpu.edx;
            cpu.edx = rgb;
            cpu.esi = at(app, colors);
            if (!direct(app, cpu, hsvToRgb))
                return;
            cpu.edx = cpu.ecx * 4;
            cpu.edx += cpu.esi;
            cpu.al = byte(app, rgb);
            putByte(app, cpu.edx, cpu.al);
            cpu.al = byte(app, rgb + 4);
            putByte(app, cpu.edx + 1, cpu.al);
            cpu.al = byte(app, rgb + 8);
            putByte(app, cpu.edx + 2, cpu.al);
            cpu.al = byte(app, rgb + 0xc);
            ++cpu.ecx;
            putByte(app, cpu.edx + 3, cpu.al);
        }
        cpu.eax = at(app, count);
        put(app, cpu.edi, cpu.eax);
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.esp += 4 + 4;
}

/* 0x49c880: the colour at eax -- red, green, blue, 0 to 255, and a fourth,
 * a dword each -- as hue, saturation and brightness to edx: the hue in 8.8
 * parts of a turn (its sixth from which of the three is largest), the
 * others 0 to 255, each held to 0 to 255; the fourth as it is.  A hue
 * below 0 has its high word counted on (inc word [edx + 2]), as the code
 * does. */
void rgbToHsv(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ebx = cpu.eax;
    cpu.esi = cpu.edx;
    // The largest of the three to edi, the smallest to edx.
    cpu.edx = at(app, cpu.ebx + 8);
    cpu.eax = sreg32(cpu.edx) < sreg32(at(app, cpu.eax + 4)) ? at(app, cpu.ebx + 4) : cpu.edx;
    cpu.ecx = at(app, cpu.ebx);
    if (sreg32(cpu.eax) < sreg32(cpu.ecx))
        cpu.edi = cpu.ecx;
    else
    {
        cpu.edi = at(app, cpu.ebx + 8);
        cpu.eax = at(app, cpu.ebx + 4);
        if (sreg32(cpu.eax) > sreg32(cpu.edi))
            cpu.edi = cpu.eax;
    }
    cpu.eax = at(app, cpu.ebx + 4);
    cpu.edx = at(app, cpu.ebx + 8);
    if (!(sreg32(cpu.eax) < sreg32(cpu.edx)))
        cpu.eax = cpu.edx;
    cpu.ecx = at(app, cpu.ebx);
    if (sreg32(cpu.eax) > sreg32(cpu.ecx))
        cpu.edx = cpu.ecx;
    else
    {
        cpu.edx = at(app, cpu.ebx + 8);
        cpu.eax = at(app, cpu.ebx + 4);
        if (sreg32(cpu.eax) < sreg32(cpu.edx))
            cpu.edx = cpu.eax;
    }
    cpu.eax = at(app, cpu.ebx + 0xc);
    put(app, cpu.esi, 0);
    put(app, cpu.esi + 4, 0);
    cpu.ecx = cpu.edi;
    put(app, cpu.esi + 0xc, cpu.eax);
    cpu.ecx -= cpu.edx;
    put(app, cpu.esi + 8, cpu.edi);
    if (cpu.edi != 0 && cpu.ecx != 0)
    {
        cpu.edx = cpu.ecx << 8;
        cpu.eax = cpu.edx;
        cpu.edx = reg32(sreg32(cpu.edx) >> 31);
        idivide(cpu, cpu.edi);
        put(app, cpu.esi + 4, cpu.eax);
        if (cpu.edi == at(app, cpu.ebx))
        {
            cpu.eax = at(app, cpu.ebx + 4);
            cpu.edi = at(app, cpu.ebx + 8);
            cpu.ebx = cpu.ecx;
            cpu.eax -= cpu.edi;
            fixedDivide(cpu);
        }
        else
        {
            cpu.eax = at(app, cpu.ebx + 4);
            if (cpu.edi == cpu.eax)
            {
                cpu.eax = at(app, cpu.ebx + 8);
                cpu.edi = at(app, cpu.ebx);
                cpu.ebx = cpu.ecx;
                cpu.eax -= cpu.edi;
                fixedDivide(cpu);
                cpu.eax += 0x20000;
            }
            else
            {
                cpu.edx = at(app, cpu.ebx);
                cpu.edx -= cpu.eax;
                cpu.ebx = cpu.ecx;
                cpu.eax = cpu.edx;
                fixedDivide(cpu);
                cpu.eax += 0x40000;
            }
        }
        put(app, cpu.esi, cpu.eax);
        cpu.eax = at(app, cpu.esi);
        cpu.edx = reg32(sreg32(cpu.eax) >> 31);
        cpu.ecx = 6;
        idivide(cpu, cpu.ecx);
        put(app, cpu.esi, cpu.eax);
        if (sreg32(cpu.eax) < 0)
            putHalf(app, cpu.esi + 2, reg16(half(app, cpu.esi + 2) + 1));
    }
    cpu.eax = 0x100;
    cpu.edx = at(app, cpu.esi);
    fixedMultiply(cpu);
    put(app, cpu.esi, cpu.eax);
    // Each held to 0 to 255.
    if (sreg32(cpu.eax) <= 0xff && sreg32(cpu.eax) < 0)
        cpu.eax = 0;
    else
    {
        cpu.ebx = at(app, cpu.esi);
        cpu.eax = sreg32(cpu.ebx) > 0xff ? 0xff : cpu.ebx;
    }
    cpu.edi = at(app, cpu.esi + 4);
    put(app, cpu.esi, cpu.eax);
    if (sreg32(cpu.edi) <= 0xff && (cpu.eax = cpu.edi, sreg32(cpu.edi) < 0))
        cpu.edx = 0;
    else
    {
        cpu.eax = at(app, cpu.esi + 4);
        cpu.edx = sreg32(cpu.eax) > 0xff ? 0xff : cpu.eax;
    }
    put(app, cpu.esi + 4, cpu.edx);
    cpu.edx = at(app, cpu.esi + 8);
    if (sreg32(cpu.edx) <= 0xff && (cpu.eax = cpu.edx, sreg32(cpu.edx) < 0))
        cpu.eax ^= cpu.edx;
    else
    {
        cpu.ecx = at(app, cpu.esi + 8);
        cpu.eax = sreg32(cpu.ecx) > 0xff ? 0xff : cpu.ecx;
    }
    put(app, cpu.esi + 8, cpu.eax);
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x49ca50: the colour at eax as hue, saturation and brightness to edx as
 * red, green and blue, each held to 0 to 255, and the fourth as it is: none
 * bright, black; none saturated, grey; else by the hue's sixth (the x87's
 * (hue * 6) less its whole sixths, chopped), the switch's six cases -- a
 * sixth out of them leaves the three as they were before the holding. */
void hsvToRgb(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0xc;
    const reg32 fraction = cpu.ebp - 4, sixth = cpu.ebp - 8, falling = cpu.ebp - 0xc;
    x86::FPU& fpu = cpu.fpu;
    cpu.edi = cpu.eax;
    cpu.esi = cpu.edx;
    cpu.eax = at(app, cpu.eax + 0xc);
    put(app, cpu.edx + 0xc, cpu.eax);
    cpu.edx = at(app, cpu.edi + 8);
    // Where the holding starts: 0 at [esi] first, or what is there.
    enum { kHold, kHoldBelow, kHoldAbove } hold = kHold;
    if (sreg32(cpu.edx) <= 0)
    {
        put(app, cpu.esi + 4, 0);
        put(app, cpu.esi + 8, 0);
        put(app, cpu.esi, 0);
    }
    else if (sreg32(at(app, cpu.edi + 4)) <= 0)
    {
        put(app, cpu.esi, cpu.edx);
        put(app, cpu.esi + 4, cpu.edx);
        put(app, cpu.esi + 8, cpu.edx);
        cpu.ecx = at(app, cpu.esi);
        hold = sreg32(cpu.ecx) <= 0xff ? kHoldBelow : kHoldAbove;
    }
    else
    {
        cpu.edx = at(app, cpu.edi);
        loadInteger(app, cpu, cpu.edi);
        cpu.eax = cpu.edx * 4;
        fpu.st(0) = fp::mul(fpu.st(0), f64(app, kSix));
        cpu.eax -= cpu.edx;
        cpu.eax += cpu.eax;
        cpu.eax = reg32(sreg32(cpu.eax) >> 8);
        put(app, sixth, cpu.eax);
        cpu.eax <<= 8;
        put(app, fraction, cpu.eax);
        loadInteger(app, cpu, fraction);
        fpu.st(1) = fp::sub(fpu.st(1), fpu.st(0));
        fpu.pop();
        cpu.ebx = 0x100;
        if (!byAddress(app, cpu, kChop))
            return;
        storeInteger(app, cpu, fraction);
        cpu.eax = at(app, fraction);
        fixedDivide(cpu);
        cpu.ebx = cpu.eax;
        cpu.edx = at(app, cpu.edi + 4);
        cpu.eax = 0x100;
        cpu.ecx = at(app, cpu.edi + 8);
        cpu.eax -= cpu.edx;
        cpu.ecx *= cpu.eax;
        cpu.eax = cpu.edx;
        cpu.edx = cpu.ebx;
        fixedMultiply(cpu);
        cpu.edx = 0x100;
        cpu.edx -= cpu.eax;
        cpu.eax = cpu.edx;
        cpu.eax *= at(app, cpu.edi + 8);
        cpu.eax = reg32(sreg32(cpu.eax) >> 8);
        cpu.edx = 0x10000;
        put(app, falling, cpu.eax);
        cpu.edx -= cpu.ebx;
        cpu.eax = at(app, cpu.edi + 4);
        fixedMultiply(cpu);
        cpu.edx = 0x100;
        cpu.edx -= cpu.eax;
        cpu.ebx = at(app, cpu.edi + 8);
        cpu.eax = cpu.edx;
        cpu.eax *= cpu.ebx;
        cpu.ecx = reg32(sreg32(cpu.ecx) >> 8);
        cpu.edx = at(app, sixth);
        cpu.eax = reg32(sreg32(cpu.eax) >> 8);
        if (cpu.edx <= 5)
        {
            // jmp [edx * 4 + 0x49ca34]: 0x49cb5b, 0x49cb72, 0x49cb8c,
            // 0x49cb96, 0x49cba3, 0x49cbad.
            switch (cpu.edx)
            {
            case 0:
                cpu.edx = at(app, cpu.edi + 8);
                put(app, cpu.esi, cpu.edx);
                put(app, cpu.esi + 4, cpu.eax);
                put(app, cpu.esi + 8, cpu.ecx);
                cpu.ecx = at(app, cpu.esi);
                hold = sreg32(cpu.ecx) <= 0xff ? kHoldBelow : kHoldAbove;
                break;
            case 1:
                cpu.eax = at(app, falling);
                put(app, cpu.esi, cpu.eax);
                cpu.eax = at(app, cpu.edi + 8);
                put(app, cpu.esi + 4, cpu.eax);
                put(app, cpu.esi + 8, cpu.ecx);
                cpu.ecx = at(app, cpu.esi);
                hold = sreg32(cpu.ecx) <= 0xff ? kHoldBelow : kHoldAbove;
                break;
            case 2:
                put(app, cpu.esi, cpu.ecx);
                cpu.edx = at(app, cpu.edi + 8);
                put(app, cpu.esi + 4, cpu.edx);
                put(app, cpu.esi + 8, cpu.eax);
                break;
            case 3:
                cpu.eax = at(app, falling);
                put(app, cpu.esi, cpu.ecx);
                put(app, cpu.esi + 4, cpu.eax);
                cpu.eax = at(app, cpu.edi + 8);
                put(app, cpu.esi + 8, cpu.eax);
                break;
            case 4:
                put(app, cpu.esi + 4, cpu.ecx);
                put(app, cpu.esi, cpu.eax);
                cpu.eax = at(app, cpu.edi + 8);
                put(app, cpu.esi + 8, cpu.eax);
                break;
            default:
                put(app, cpu.esi, cpu.ebx);
                cpu.eax = at(app, falling);
                put(app, cpu.esi + 4, cpu.ecx);
                put(app, cpu.esi + 8, cpu.eax);
                break;
            }
        }
    }
    // Each held to 0 to 255.
    if (hold == kHold)
    {
        cpu.ecx = at(app, cpu.esi);
        hold = sreg32(cpu.ecx) > 0xff ? kHoldAbove : kHoldBelow;
    }
    if (hold == kHoldBelow && (cpu.eax = cpu.ecx, sreg32(cpu.ecx) < 0))
        cpu.eax ^= cpu.ecx;
    else
    {
        cpu.ebx = at(app, cpu.esi);
        cpu.eax = sreg32(cpu.ebx) > 0xff ? 0xff : cpu.ebx;
    }
    cpu.edi = at(app, cpu.esi + 4);
    put(app, cpu.esi, cpu.eax);
    if (sreg32(cpu.edi) <= 0xff && (cpu.eax = cpu.edi, sreg32(cpu.edi) < 0))
        cpu.eax ^= cpu.edi;
    else
    {
        cpu.eax = at(app, cpu.esi + 4);
        if (sreg32(cpu.eax) > 0xff)
            cpu.eax = 0xff;
    }
    cpu.edx = at(app, cpu.esi + 8);
    put(app, cpu.esi + 4, cpu.eax);
    if (sreg32(cpu.edx) <= 0xff && (cpu.eax = cpu.edx, sreg32(cpu.edx) < 0))
        cpu.eax ^= cpu.edx;
    else
    {
        cpu.ecx = at(app, cpu.esi + 8);
        cpu.eax = sreg32(cpu.ecx) > 0xff ? 0xff : cpu.ecx;
    }
    put(app, cpu.esi + 8, cpu.eax);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x49cc50: the 1555 pixel eax in the colour at edx (hue, saturation,
 * brightness, the fourth): 0 unless its top bit is set; else its blue (as a
 * brightness, over 255 less the colour's fourth unless that is 255) times the
 * colour's brightness, held to 255, makes the colour's red, green and blue,
 * 1555 again with the top bit. */
void tintPixel(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x30;
    const auto done = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.esp += 4;
    };
    if (!(cpu.ah & 0x80))
    {
        cpu.eax = 0;
        return done();
    }
    const reg32 blue = cpu.ebp - 4, scale = cpu.ebp - 8, hsv = cpu.ebp - 0x20, rgb = cpu.ebp - 0x30;
    x86::FPU& fpu = cpu.fpu;
    cpu.eax &= 0xffff;
    cpu.eax <<= 3;
    cpu.eax &= 0xff;
    put(app, blue, cpu.eax);
    cpu.ecx = at(app, cpu.edx + 0xc);
    loadInteger(app, cpu, blue);
    putF32(app, scale, fpu.pop());
    if (cpu.ecx != 0xff)
    {
        loadInteger(app, cpu, cpu.edx + 0xc);
        fpu.st(0) = fp::sub(f64(app, kFull), fpu.st(0));
        fpu.st(0) = fp::div(f32(app, scale), fpu.st(0));
        putF32(app, scale, fpu.pop());
    }
    cpu.eax = at(app, cpu.edx);
    put(app, hsv, cpu.eax);
    cpu.eax = at(app, cpu.edx + 4);
    put(app, hsv + 4, cpu.eax);
    loadInteger(app, cpu, cpu.edx + 8);
    fpu.st(0) = fp::mul(fpu.st(0), f32(app, scale));
    if (!byAddress(app, cpu, kChop))
        return;
    storeInteger(app, cpu, hsv + 8);
    if (sreg32(at(app, hsv + 8)) > 0xff)
        put(app, hsv + 8, 0xff);
    cpu.eax = at(app, cpu.edx + 0xc);
    cpu.edx = rgb;
    put(app, hsv + 0xc, cpu.eax);
    cpu.eax = hsv;
    if (!direct(app, cpu, hsvToRgb))
        return;
    cpu.eax = at(app, rgb);
    cpu.eax = reg32(sreg32(cpu.eax) >> 3);
    cpu.eax <<= 10;
    cpu.ah |= 0x80;
    cpu.edx = cpu.eax;
    cpu.eax = at(app, rgb + 4);
    cpu.eax = reg32(sreg32(cpu.eax) >> 3);
    cpu.eax <<= 5;
    cpu.edx |= cpu.eax;
    cpu.eax = at(app, rgb + 8);
    cpu.eax = reg32(sreg32(cpu.eax) >> 3);
    cpu.eax |= cpu.edx;
    done();
}

/* 0x49cd00: the ebx by ecx pixels at edx (32 bits each) scaled to [esp+4] by
 * [esp+8] 1555 words at eax, the nearest pixel of each: the top bit from the
 * alpha's being any; tinted in the colour [esp+0xc] (0x49cc50) where the
 * alpha's top bit is clear, else in the colour [esp+0x10] where it is not
 * 0xff -- when there is the colour; 0x8000 drawn as 0x8001 while [0x7a3a6c]'s
 * low bit is set. */
void scaleTexture(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x30;
    const reg32 row = cpu.ebp - 4, red = cpu.ebp - 8, column = cpu.ebp - 0xc, blue = cpu.ebp - 0x10;
    const reg32 width = cpu.ebp - 0x14, rowStep = cpu.ebp - 0x18, columnStep = cpu.ebp - 0x1c;
    const reg32 pixels = cpu.ebp - 0x20, x = cpu.ebp - 0x24, scratch1 = cpu.ebp - 0x28, scratch2 = cpu.ebp - 0x2c;
    const reg32 scratch3 = cpu.ebp - 0x30;
    const reg32 toWidth = cpu.ebp + 0x10, toHeight = cpu.ebp + 0x14, color = cpu.ebp + 0x18;
    const reg32 secondColor = cpu.ebp + 0x1c;
    x86::FPU& fpu = cpu.fpu;
    cpu.esi = cpu.eax;
    put(app, pixels, cpu.edx);
    put(app, width, cpu.ebx);
    cpu.eax = at(app, toWidth);
    put(app, scratch1, cpu.ebx);
    put(app, scratch2, cpu.eax);
    loadInteger(app, cpu, scratch1);
    loadInteger(app, cpu, scratch2);
    fpu.st(1) = fp::div(fpu.st(1), fpu.st(0));
    fpu.pop();
    cpu.eax = at(app, toHeight);
    put(app, scratch2, cpu.ecx);
    put(app, scratch1, cpu.eax);
    loadInteger(app, cpu, scratch2);
    loadInteger(app, cpu, scratch1);
    fpu.st(1) = fp::div(fpu.st(1), fpu.st(0));
    fpu.pop();
    cpu.ecx = 0;
    std::swap(fpu.st(0), fpu.st(1));
    putF32(app, columnStep, fpu.pop());
    putF32(app, rowStep, fpu.pop());
    // The nearest pixel's address in the row, eax.
    const auto nearest = [&](reg32 into) {
        fpu.push(f32(app, x));
        if (!byAddress(app, cpu, kChop))
            return false;
        storeInteger(app, cpu, into);
        cpu.eax = at(app, into);
        return true;
    };
    for (; sreg32(cpu.ecx) < sreg32(at(app, toHeight)); ++cpu.ecx)
    {
        put(app, scratch1, cpu.ecx);
        loadInteger(app, cpu, scratch1);
        fpu.st(0) = fp::mul(fpu.st(0), f32(app, rowStep));
        cpu.edi = at(app, width);
        if (!byAddress(app, cpu, kChop))
            return;
        storeInteger(app, cpu, scratch2);
        cpu.eax = at(app, scratch2);
        cpu.eax *= cpu.edi;
        cpu.edx = at(app, pixels);
        cpu.ebx = 0;
        cpu.eax <<= 2;
        put(app, column, cpu.ebx);
        cpu.edx += cpu.eax;
        put(app, x, cpu.ebx);
        put(app, row, cpu.edx);
        while (true)
        {
            cpu.eax = at(app, column);
            if (sreg32(cpu.eax) >= sreg32(at(app, toWidth)))
                break;
            if (!nearest(scratch2))
                return;
            cpu.ebx = at(app, row);
            cpu.eax <<= 2;
            cpu.eax += cpu.ebx;
            cpu.edx = byte(app, cpu.eax + 3) != 0 ? 0x8000 : 0;
            if (!nearest(scratch3))
                return;
            cpu.edi = at(app, row);
            cpu.eax <<= 2;
            cpu.eax += cpu.edi;
            cpu.ebx = 0;
            cpu.bl = byte(app, cpu.eax + 2);
            cpu.edi = byte(app, cpu.eax);
            cpu.ebx = reg32(sreg32(cpu.ebx) >> 3);
            put(app, blue, cpu.ebx);
            cpu.edi = reg32(sreg32(cpu.edi) >> 3);
            cpu.ebx = 0;
            put(app, red, cpu.edi);
            cpu.bl = byte(app, cpu.eax + 1);
            cpu.edi = at(app, color);
            cpu.ebx = reg32(sreg32(cpu.ebx) >> 3);
            bool tinted = false;
            if (cpu.edi != 0 && byte(app, cpu.eax + 3) < 0x80)
            {
                cpu.eax = at(app, blue) << 10;
                cpu.ebx <<= 5;
                cpu.eax |= cpu.edx;
                cpu.edx = at(app, red);
                cpu.eax |= cpu.ebx;
                cpu.eax |= cpu.edx;
                cpu.eax &= 0xffff;
                cpu.edx = cpu.edi;
                if (!direct(app, cpu, tintPixel))
                    return;
                tinted = true;
            }
            else if (at(app, secondColor) != 0)
            {
                if (!nearest(scratch1))
                    return;
                cpu.edi = at(app, row);
                cpu.eax <<= 2;
                cpu.eax += cpu.edi;
                if (byte(app, cpu.eax + 3) < 0xff)
                {
                    cpu.eax = at(app, blue) << 10;
                    cpu.edx |= cpu.eax;
                    cpu.eax = cpu.ebx << 5;
                    cpu.ebx = at(app, red);
                    cpu.eax |= cpu.edx;
                    cpu.eax |= cpu.ebx;
                    cpu.edx = at(app, secondColor);
                    cpu.eax &= 0xffff;
                    if (!direct(app, cpu, tintPixel))
                        return;
                    tinted = true;
                }
            }
            if (!tinted)
            {
                cpu.eax = at(app, blue) << 10;
                cpu.ebx <<= 5;
                cpu.edx |= cpu.eax;
                cpu.eax = at(app, red);
                cpu.edx |= cpu.ebx;
                cpu.eax |= cpu.edx;
            }
            putHalf(app, cpu.esi, cpu.ax);
            cpu.eax = half(app, cpu.esi) == 0x8000 ? 1 : 0;
            if (at(app, kBlackVisible) & cpu.eax)
                putHalf(app, cpu.esi, 0x8001);
            fpu.push(f32(app, x));
            cpu.ebx = at(app, column);
            cpu.esi += 2;
            fpu.st(0) = fp::add(fpu.st(0), f32(app, columnStep));
            ++cpu.ebx;
            putF32(app, x, fpu.pop());
            put(app, column, cpu.ebx);
        }
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.esp += 4 + 0x10;
}

/* 0x49cea0: the ebx by ecx pixels at edx made a texture at eax: its size
 * ([eax], [eax + 4]) the largest power of two no larger than ebx, 256 at
 * most -- or the size asked for ([esp+4]) when ebx is larger than it -- and
 * the pixels scaled to it at eax + 8 (0x49cd00), in the colours [esp+8] and
 * [esp+0xc]. */
void fitTexture(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edi = at(app, cpu.ebp + 0x10);
    cpu.esi = cpu.eax + 8;
    reg32 size;
    if (sreg32(cpu.ebx) > sreg32(cpu.edi))
        size = cpu.edi;
    else
    {
        size = 1;
        // port: 0x200 first, reached only where the size asked for is 512 (port_thrash.h)
        for (const reg32 power : {0x200u, 0x100u, 0x80u, 0x40u, 0x20u, 0x10u, 8u, 4u, 2u})
            if (sreg32(cpu.ebx) >= sreg32(power))
            {
                size = power;
                break;
            }
    }
    put(app, cpu.eax + 4, size);
    put(app, cpu.eax, size);
    cpu.edi = at(app, cpu.ebp + 0x18);
    push(app, cpu, cpu.edi);
    cpu.edi = at(app, cpu.ebp + 0x14);
    push(app, cpu, cpu.edi);
    cpu.edi = at(app, cpu.eax + 4);
    push(app, cpu, cpu.edi);
    cpu.edi = at(app, cpu.eax);
    push(app, cpu, cpu.edi);
    cpu.eax = cpu.esi;
    if (!direct(app, cpu, scaleTexture))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.esp += 4 + 0xc;
}

/* 0x49d130: the textures of the model eax from the .art's pictures at edx,
 * ebx at most each way, in the colour ecx -- and the model's second colours'
 * first, if it has them: each picture fitted into a block of the game's
 * memory (0x49cea0), allocated as a texture (its handle to the model's
 * +0x1e04 on) and given the pixels; a picture of no size has no texture (0).
 * Only for a model of more than one picture -- the block freed only then. */
void loadArtTextures(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x1c;
    const reg32 model = cpu.ebp - 4, index = cpu.ebp - 8, most = cpu.ebp - 0xc, count = cpu.ebp - 0x10;
    const reg32 second = cpu.ebp - 0x14, color = cpu.ebp - 0x18, picture = cpu.ebp - 0x1c;
    put(app, model, cpu.eax);
    cpu.edi = cpu.edx;
    put(app, most, cpu.ebx);
    cpu.edx = cpu.ebx;
    cpu.edx *= cpu.ebx;
    cpu.eax = kNoName;
    cpu.edx += cpu.edx;
    cpu.ebx = 0x10;
    cpu.edx += 8;
    if (!byAddress(app, cpu, kAllocate))
        return;
    cpu.esi = cpu.eax;
    cpu.eax = at(app, model);
    cpu.edx = at(app, cpu.eax + 0x900);
    put(app, color, cpu.ecx);
    if (cpu.edx != 0)
    {
        cpu.eax += 0x904;
        put(app, second, cpu.eax);
    }
    else
        put(app, second, cpu.edx);
    cpu.eax = at(app, model);
    cpu.eax = at(app, cpu.eax + 0xc);
    put(app, count, cpu.eax);
    if (sreg32(cpu.eax) > 1)
    {
        cpu.eax = 0;
        put(app, index, cpu.eax);
        while (cpu.eax = at(app, index), sreg32(cpu.eax) < sreg32(at(app, count)))
        {
            cpu.ecx = at(app, cpu.edi);
            put(app, picture, cpu.edi);
            if (cpu.ecx != 0 && (cpu.ebx = at(app, cpu.edi + 4)) != 0)
            {
                cpu.eax = at(app, second);
                push(app, cpu, cpu.eax);
                cpu.edx = at(app, color);
                push(app, cpu, cpu.edx);
                cpu.ecx = at(app, most);
                cpu.eax = cpu.esi;
                push(app, cpu, cpu.ecx);
                cpu.edx = cpu.edi + 8;
                cpu.ecx = cpu.ebx;
                cpu.ebx = at(app, cpu.edi);
                if (!direct(app, cpu, fitTexture))
                    return;
                cpu.edx = at(app, cpu.esi + 4);
                cpu.eax = at(app, cpu.esi);
                if (!byAddress(app, cpu, kAllocTexture))
                    return;
                cpu.edx = at(app, index);
                cpu.ebx = at(app, model);
                cpu.edx <<= 2;
                cpu.edx += cpu.ebx;
                put(app, cpu.edx + 0x1e04, cpu.eax);
                cpu.edx = cpu.esi + 8;
                cpu.ecx = at(app, cpu.esi + 4);
                cpu.ebx = at(app, cpu.esi);
                if (!byAddress(app, cpu, kUpdateTexture))
                    return;
            }
            else
            {
                cpu.eax = at(app, index);
                cpu.edx = at(app, model);
                cpu.eax <<= 2;
                cpu.eax += cpu.edx;
                put(app, cpu.eax + 0x1e04, 0);
            }
            cpu.ecx = at(app, cpu.edi + 4);
            cpu.eax = at(app, cpu.edi);
            cpu.eax *= cpu.ecx;
            cpu.ebx = at(app, index);
            cpu.edi = at(app, picture);
            cpu.eax <<= 2;
            ++cpu.ebx;
            cpu.eax += 8;
            put(app, index, cpu.ebx);
            cpu.edi += cpu.eax;
        }
        if (cpu.esi != 0)
        {
            cpu.eax = cpu.esi;
            if (!byAddress(app, cpu, kFree))
                return;
        }
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x49d240: which of the model eax's colours (as many as it has of both
 * kinds) is nearest the colour at edx, as red, green and blue (0x49ca50
 * each): the first nearer than 1000 and every other one nearer; 0 if none. */
void nearestModelColor(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x44;
    const reg32 scratch = cpu.ebp - 4, best = cpu.ebp - 8, nearest = cpu.ebp - 0xc, distance = cpu.ebp - 0x10;
    const reg32 color = cpu.ebp - 0x1c, wanted = cpu.ebp - 0x28, difference = cpu.ebp - 0x34;
    const reg32 rgb = cpu.ebp - 0x44;
    x86::FPU& fpu = cpu.fpu;
    cpu.edi = cpu.eax;
    cpu.eax = cpu.edx;
    put(app, best, kThousand);
    cpu.edx = rgb;
    cpu.ecx = 0;
    if (!direct(app, cpu, hsvToRgb))
        return;
    cpu.eax = at(app, rgb);
    cpu.esi = 0;
    put(app, scratch, cpu.eax);
    put(app, nearest, cpu.ecx);
    loadInteger(app, cpu, scratch);
    cpu.eax = at(app, rgb + 4);
    putF32(app, wanted, fpu.pop());
    put(app, scratch, cpu.eax);
    cpu.eax = at(app, rgb + 8);
    loadInteger(app, cpu, scratch);
    put(app, scratch, cpu.eax);
    putF32(app, wanted + 4, fpu.pop());
    loadInteger(app, cpu, scratch);
    putF32(app, wanted + 8, fpu.pop());
    for (; sreg32(cpu.esi) < sreg32(at(app, cpu.edi + 0x7fc)) && sreg32(cpu.esi) < sreg32(at(app, cpu.edi + 0x900));
         ++cpu.esi)
    {
        cpu.edx = cpu.esi << 4;
        cpu.eax = cpu.edi + 0x800;
        cpu.eax += cpu.edx;
        cpu.edx = rgb;
        cpu.ecx = difference;
        if (!direct(app, cpu, hsvToRgb))
            return;
        cpu.eax = at(app, rgb);
        cpu.ebx = color;
        put(app, scratch, cpu.eax);
        cpu.edx = wanted;
        loadInteger(app, cpu, scratch);
        cpu.eax = at(app, rgb + 4);
        putF32(app, color, fpu.pop());
        put(app, scratch, cpu.eax);
        cpu.eax = at(app, rgb + 8);
        loadInteger(app, cpu, scratch);
        put(app, scratch, cpu.eax);
        putF32(app, color + 4, fpu.pop());
        loadInteger(app, cpu, scratch);
        cpu.eax = 1;
        putF32(app, color + 8, fpu.pop());
        if (!byAddress(app, cpu, kSubtract))
            return;
        cpu.eax = difference;
        if (!byAddress(app, cpu, kLength))
            return;
        putF32(app, distance, fpu.st(0));
        fpu.compare(fpu.st(0), f32(app, best));
        cpu.ax = fpu.status.word;
        const bool below = (cpu.ax & 0x100) != 0;
        fpu.pop();
        if (below)
        {
            cpu.eax = at(app, distance);
            put(app, nearest, cpu.esi);
            put(app, best, cpu.eax);
        }
    }
    cpu.eax = at(app, nearest);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x49d320: the model eax's texture from the .tga at edx, ebx at most each
 * way: in the second colour of the model's nearest the colour [esp+4] (the
 * first without that colour; none without second colours) -- a model of one
 * picture only, and a picture of some size, else no texture (0); the texture
 * ecx, or one allocated, its handle to [eax + 0x1e04], given the pixels when
 * [esp+8] says so; the fitted pixels copied to [esp+0xc] when there is one.
 * The block of the game's memory they were fitted in freed. */
void loadTgaTexture(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x18;
    const reg32 tga = cpu.ebp - 4, texture = cpu.ebp - 8, most = cpu.ebp - 0xc, second = cpu.ebp - 0x10;
    const reg32 height = cpu.ebp - 0x14, color = cpu.ebp - 0x18;
    const reg32 upload = cpu.ebp + 0x14, copyTo = cpu.ebp + 0x18;
    cpu.edi = cpu.eax;
    put(app, tga, cpu.edx);
    // port: a .tga of 512 a side or more at 512 (port_thrash.h), for a model of the one picture it is read for
    if (at(app, cpu.edi + 0xc) == 1)
        cpu.ebx = reg32(port::driver::texture512Side(sreg32(cpu.ebx),
                                                     int32_t(byte(app, cpu.edx + 0xc) | byte(app, cpu.edx + 0xd) << 8)));
    put(app, most, cpu.ebx);
    put(app, texture, cpu.ecx);
    cpu.ecx = at(app, cpu.ebp + 0x10);
    cpu.eax = cpu.ebx;
    cpu.eax *= cpu.ebx;
    cpu.eax += cpu.eax;
    cpu.edx = cpu.eax + 8;
    cpu.ebx = 0x10;
    cpu.eax = kNoName;
    put(app, color, cpu.ecx);
    if (!byAddress(app, cpu, kAllocate))
        return;
    cpu.ebx = at(app, cpu.edi + 0x900);
    cpu.esi = cpu.eax;
    if (cpu.ebx != 0)
    {
        if (cpu.ecx != 0)
        {
            cpu.edx = cpu.ecx;
            cpu.eax = cpu.edi;
            if (!direct(app, cpu, nearestModelColor))
                return;
        }
        else
            cpu.eax = 0;
        cpu.edx = cpu.edi + 0x904;
        cpu.eax <<= 4;
        cpu.edx += cpu.eax;
        put(app, second, cpu.edx);
    }
    else
        put(app, second, cpu.ebx);
    cpu.eax = at(app, cpu.edi + 0xc);
    if (cpu.eax == 1)
    {
        cpu.eax = at(app, tga);
        cpu.ebx = 0;
        cpu.edx = at(app, tga);
        cpu.bl = byte(app, cpu.eax + 0xd);
        cpu.eax = 0;
        cpu.ebx <<= 8;
        cpu.al = byte(app, cpu.edx + 0xc);
        cpu.ebx += cpu.eax;
        cpu.eax = 0;
        cpu.al = byte(app, cpu.edx + 0xf);
        cpu.ecx = at(app, tga);
        cpu.edx = cpu.eax;
        cpu.eax = 0;
        cpu.edx <<= 8;
        cpu.al = byte(app, cpu.ecx + 0xe);
        cpu.edx += cpu.eax;
        cpu.eax = 0;
        put(app, height, cpu.edx);
        cpu.al = byte(app, cpu.ecx);
        cpu.edx = cpu.ecx + 0x12;
        cpu.edx += cpu.eax;
        if (cpu.ebx != 0 && at(app, height) != 0)
        {
            cpu.ecx = at(app, second);
            push(app, cpu, cpu.ecx);
            cpu.eax = at(app, color);
            push(app, cpu, cpu.eax);
            cpu.ecx = at(app, most);
            push(app, cpu, cpu.ecx);
            cpu.eax = cpu.esi;
            cpu.ecx = at(app, height);
            if (!direct(app, cpu, fitTexture))
                return;
            if (at(app, texture) == 0)
            {
                cpu.edx = at(app, cpu.esi + 4);
                cpu.eax = at(app, cpu.esi);
                if (!byAddress(app, cpu, kAllocTexture))
                    return;
                put(app, texture, cpu.eax);
            }
            else
            {
                // port: the texture given made the fitted size, 256 or 512 (port_thrash.h)
                port::driver::resizeTexture(app, at(app, texture), at(app, cpu.esi));
            }
            cpu.eax = at(app, texture);
            put(app, cpu.edi + 0x1e04, cpu.eax);
            if (at(app, upload) != 0)
            {
                cpu.edx = cpu.esi + 8;
                cpu.ecx = at(app, cpu.esi + 4);
                cpu.ebx = at(app, cpu.esi);
                if (!byAddress(app, cpu, kUpdateTexture))
                    return;
            }
        }
        else
            put(app, cpu.edi + 0x1e04, 0);
    }
    if (at(app, copyTo) != 0)
    {
        cpu.ebx = at(app, cpu.esi);
        cpu.edx = at(app, cpu.esi + 4);
        cpu.ebx += cpu.ebx;
        cpu.ebx *= cpu.edx;
        cpu.eax = cpu.esi + 8;
        cpu.edx = at(app, copyTo);
        if (!byAddress(app, cpu, kMoveMemory))
            return;
    }
    if (cpu.esi != 0)
    {
        cpu.eax = cpu.esi;
        if (!byAddress(app, cpu, kFree))
            return;
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.esp += 4 + 0xc;
}

/* 0x49d450: the car's model named eax: "%s.fce" loaded and made ready, its
 * texture from "%s00.tga" when it is of one picture and the file named eax
 * is there, else from "%s.art"; edx at most each way, in its colour ebx (of
 * as many as it has).  The files loaded freed; the model. */
void loadCarModel(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0xd8;
    cpu.ebp -= 0x82;
    const reg32 name = cpu.ebp - 0x56, model = cpu.ebp + 0x7e, tga = cpu.ebp + 0x7a, tgaLoaded = cpu.ebp + 0x76;
    const reg32 art = cpu.ebp + 0x72;
    cpu.ecx = cpu.eax;
    cpu.edi = cpu.edx;
    push(app, cpu, cpu.eax);
    push(app, cpu, kFceFormat);
    cpu.eax = name;
    push(app, cpu, cpu.eax);
    if (!byAddress(app, cpu, kSprintf))
        return;
    cpu.esp += 0xc;
    cpu.edx = 0;
    cpu.eax = name;
    if (!byAddress(app, cpu, kLoadFile) || !byAddress(app, cpu, kModelReady))
        return;
    push(app, cpu, cpu.ecx);
    cpu.esi = cpu.eax;
    push(app, cpu, kTgaFormat);
    put(app, model, cpu.eax);
    cpu.eax = name;
    push(app, cpu, cpu.eax);
    if (!byAddress(app, cpu, kSprintf))
        return;
    cpu.edx = at(app, cpu.esi + 0xc);
    cpu.esp += 0xc;
    bool fromTga = false;
    if (cpu.edx == 1)
    {
        cpu.eax = cpu.ecx;
        if (!byAddress(app, cpu, kFileExists))
            return;
        fromTga = cpu.eax != 0;
    }
    if (fromTga)
    {
        cpu.edx = 0x10;
        cpu.eax = name;
        if (!byAddress(app, cpu, kLoadFile))
            return;
        put(app, tga, cpu.eax);
        cpu.ecx = at(app, cpu.esi + 0x7fc);
        put(app, tgaLoaded, cpu.eax);
        if (sreg32(cpu.ecx) > 0)
        {
            cpu.edx = cpu.ebx;
            cpu.eax = cpu.ebx;
            cpu.edx = reg32(sreg32(cpu.edx) >> 31);
            idivide(cpu, cpu.ecx);
            push(app, cpu, 0);
            push(app, cpu, 1);
            cpu.edx <<= 4;
            cpu.eax = cpu.esi + 0x800;
            cpu.ebx = cpu.edi;
            cpu.eax += cpu.edx;
            cpu.ecx = 0;
            push(app, cpu, cpu.eax);
            cpu.edx = at(app, tga);
        }
        else
        {
            push(app, cpu, 0);
            push(app, cpu, 1);
            cpu.ebx = cpu.edi;
            cpu.edx = cpu.eax;
            push(app, cpu, 0);
            cpu.ecx = 0;
        }
        cpu.eax = cpu.esi;
        if (!direct(app, cpu, loadTgaTexture))
            return;
        cpu.esi = at(app, tgaLoaded);
        if (cpu.esi != 0)
        {
            cpu.eax = cpu.esi;
            if (!byAddress(app, cpu, kFree))
                return;
        }
    }
    else
    {
        push(app, cpu, cpu.ecx);
        push(app, cpu, kArtFormat);
        cpu.eax = name;
        push(app, cpu, cpu.eax);
        if (!byAddress(app, cpu, kSprintf))
            return;
        cpu.esp += 0xc;
        cpu.edx = 0x10;
        cpu.eax = name;
        if (!byAddress(app, cpu, kLoadFile))
            return;
        cpu.esi = cpu.eax;
        put(app, art, cpu.eax);
        cpu.eax = at(app, model);
        if (sreg32(at(app, cpu.eax + 0x7fc)) > 0)
        {
            cpu.eax = cpu.ebx;
            cpu.edx = cpu.ebx;
            cpu.ebx = at(app, model);
            cpu.edx = reg32(sreg32(cpu.edx) >> 31);
            idivide(cpu, at(app, cpu.ebx + 0x7fc));
            cpu.edx <<= 4;
            cpu.eax = cpu.ebx + 0x800;
            cpu.ecx = cpu.eax + cpu.edx;
            cpu.ebx = cpu.edi;
            cpu.eax = at(app, model);
            cpu.edx = cpu.esi;
        }
        else
        {
            cpu.ebx = cpu.edi;
            cpu.edx = cpu.esi;
            cpu.ecx = 0;
        }
        if (!direct(app, cpu, loadArtTextures))
            return;
        cpu.ebx = at(app, art);
        if (cpu.ebx != 0)
        {
            cpu.eax = cpu.ebx;
            if (!byAddress(app, cpu, kFree))
                return;
        }
    }
    cpu.eax = at(app, model);
    cpu.esp = cpu.ebp + 0x82;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4b7a60: the model named edx in the .viv at [esp+4] (0x4ea290 finds
 * "%s.fce" and "%s00.tga" in it): the .fce copied into a block of the game's
 * memory and made ready, its texture from the .tga, ebx at most each way --
 * in the colour ecx (red, green, blue and the fourth, a byte each: as hue,
 * saturation and brightness to the car eax's +0x8a8 on) when there is one.
 * The block, the model. */
void loadVivCarModel(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x23c;
    const reg32 color = cpu.ebp - 4, car = cpu.ebp - 8, most = cpu.ebp - 0xc, name = cpu.ebp - 0x10;
    const reg32 block = cpu.ebp - 0x14, place = cpu.ebp - 0x18, size = cpu.ebp - 0x1c, hsv = cpu.ebp - 0x2c;
    const reg32 rgb = cpu.ebp - 0x3c, path = cpu.ebp - 0x23c, viv = cpu.ebp + 0x10;
    const auto moveDwords = [&] {
        for (int i = 0; i < 4; ++i)
        {
            put(app, cpu.edi, at(app, cpu.esi));
            cpu.edi += 4;
            cpu.esi += 4;
        }
    };
    put(app, car, cpu.eax);
    put(app, name, cpu.edx);
    put(app, most, cpu.ebx);
    put(app, color, cpu.ecx);
    cpu.edi = hsv;
    cpu.esi = kZeros;
    cpu.eax = path;
    push(app, cpu, cpu.edx);
    moveDwords();
    push(app, cpu, kVivFceFormat);
    cpu.edi = rgb;
    cpu.esi = kMoreZeros;
    cpu.ecx = place;
    push(app, cpu, cpu.eax);
    moveDwords();
    if (!byAddress(app, cpu, kSprintf))
        return;
    cpu.esp += 0xc;
    cpu.eax = size;
    cpu.ebx = 0;
    push(app, cpu, cpu.eax);
    cpu.edx = path;
    cpu.eax = at(app, viv);
    if (!byAddress(app, cpu, kVivEntry))
        return;
    cpu.ecx = at(app, viv);
    cpu.ebx = at(app, place);
    cpu.edx = at(app, size);
    cpu.eax = kVivNoName;
    cpu.ecx += cpu.ebx;
    cpu.ebx = 0;
    if (!byAddress(app, cpu, kAllocate))
        return;
    cpu.ebx = at(app, size);
    cpu.esi = cpu.eax;
    put(app, block, cpu.eax);
    cpu.edx = cpu.eax;
    cpu.eax = cpu.ecx;
    if (!byAddress(app, cpu, kMoveMemory))
        return;
    cpu.eax = cpu.esi;
    cpu.edi = at(app, name);
    if (!byAddress(app, cpu, kModelReady))
        return;
    push(app, cpu, cpu.edi);
    push(app, cpu, kVivTgaFormat);
    cpu.eax = path;
    push(app, cpu, cpu.eax);
    cpu.ecx = place;
    cpu.edx = path;
    if (!byAddress(app, cpu, kSprintf))
        return;
    cpu.esp += 0xc;
    cpu.eax = size;
    cpu.ebx = 0;
    push(app, cpu, cpu.eax);
    cpu.eax = at(app, viv);
    cpu.edi = at(app, viv);
    if (!byAddress(app, cpu, kVivEntry))
        return;
    cpu.eax = at(app, place);
    cpu.edx = at(app, color);
    cpu.edi += cpu.eax;
    if (cpu.edx != 0)
    {
        cpu.eax = byte(app, cpu.edx);
        put(app, rgb, cpu.eax);
        cpu.eax = byte(app, cpu.edx + 1);
        put(app, rgb + 4, cpu.eax);
        cpu.eax = byte(app, cpu.edx + 2);
        put(app, rgb + 8, cpu.eax);
        cpu.eax = byte(app, cpu.edx + 3);
        cpu.edx = hsv;
        put(app, rgb + 0xc, cpu.eax);
        cpu.eax = rgb;
        if (!direct(app, cpu, rgbToHsv))
            return;
        cpu.edx = at(app, car);
        cpu.eax = at(app, hsv);
        put(app, cpu.edx + 0x8a8, cpu.eax);
        cpu.eax = at(app, hsv + 4);
        push(app, cpu, 0);
        put(app, cpu.edx + 0x8ac, cpu.eax);
        cpu.ebx = at(app, most);
        cpu.eax = at(app, hsv + 8);
        push(app, cpu, 1);
        put(app, cpu.edx + 0x8b0, cpu.eax);
        cpu.eax = hsv;
        cpu.ecx = 0;
        push(app, cpu, cpu.eax);
        cpu.edx = cpu.edi;
        cpu.eax = cpu.esi;
    }
    else
    {
        push(app, cpu, cpu.edx);
        push(app, cpu, 1);
        cpu.ebx = at(app, most);
        cpu.eax = cpu.esi;
        push(app, cpu, cpu.edx);
        cpu.ecx = 0;
        cpu.edx = cpu.edi;
    }
    if (!direct(app, cpu, loadTgaTexture))
        return;
    cpu.eax = at(app, block);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.esp += 4 + 4;
}

/* 0x4b7ba0: another of the model named edx in the .viv at ebx: "%s.fce"
 * copied into a block of the game's memory, given the texture of the car
 * eax's model ([eax + 0x8a4]) and made ready.  The block. */
void copyVivCarModel(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x208;
    const reg32 place = cpu.ebp - 4, size = cpu.ebp - 8, path = cpu.ebp - 0x208;
    cpu.edi = cpu.eax;
    cpu.esi = cpu.ebx;
    push(app, cpu, cpu.edx);
    push(app, cpu, kVivFceFormat);
    cpu.edx = path;
    push(app, cpu, cpu.edx);
    cpu.ecx = place;
    if (!byAddress(app, cpu, kSprintf))
        return;
    cpu.esp += 0xc;
    cpu.edx = size;
    cpu.ebx = 0;
    push(app, cpu, cpu.edx);
    cpu.eax = cpu.esi;
    cpu.edx = path;
    if (!byAddress(app, cpu, kVivEntry))
        return;
    cpu.edx = at(app, place);
    cpu.eax = kVivNoName;
    cpu.ebx = 0;
    cpu.esi += cpu.edx;
    cpu.edx = at(app, size);
    if (!byAddress(app, cpu, kAllocate))
        return;
    cpu.ebx = at(app, size);
    cpu.ecx = cpu.eax;
    cpu.edx = cpu.eax;
    cpu.eax = cpu.esi;
    if (!byAddress(app, cpu, kMoveMemory))
        return;
    cpu.edx = at(app, cpu.edi + 0x8a4);
    cpu.edx = at(app, cpu.edx + 0x1e04);
    cpu.eax = cpu.ecx;
    put(app, cpu.ecx + 0x1e04, cpu.edx);
    if (!byAddress(app, cpu, kModelReady))
        return;
    cpu.eax = cpu.ecx;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4dc3a0: player eax's spec edx (0 to 49) marked in use or not, bl. */
void setSpecUsed(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    cpu.ecx = cpu.eax;
    putByte(app, cpu.ebp - 4, cpu.bl);
    cpu.eax = cpu.edx;
    cpu.edx = specsOf(cpu.ecx);
    if (cpu.edx != 0 && sreg32(cpu.eax) >= 0 && sreg32(cpu.eax) < sreg32(kSpecCount))
    {
        cpu.ebx = cpu.eax;
        cpu.eax <<= 2;
        cpu.eax -= cpu.ebx;
        cpu.eax <<= 4;
        cpu.eax += cpu.edx;
        cpu.dl = byte(app, cpu.ebp - 4);
        putByte(app, cpu.eax, cpu.dl);
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4dc3f0: whether player eax's spec edx is in use, al; al 0 for none. */
void specUsed(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    cpu.eax = cpu.edx;
    cpu.edx = specsOf(cpu.ecx);
    if (cpu.edx != 0 && sreg32(cpu.eax) >= 0 && sreg32(cpu.eax) < sreg32(kSpecCount))
    {
        cpu.ecx = cpu.eax;
        cpu.eax <<= 2;
        cpu.eax -= cpu.ecx;
        cpu.eax <<= 4;
        cpu.al = byte(app, cpu.edx + cpu.eax);
    }
    else
        cpu.al = 0;
    cpu.ebp = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4dc440: player eax's spec edx's car id (+4), or 0. */
void specCarName(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    cpu.eax = cpu.edx;
    cpu.edx = specsOf(cpu.ecx);
    cpu.ebx = 0;
    if (cpu.edx != 0 && sreg32(cpu.eax) >= 0 && sreg32(cpu.eax) < sreg32(kSpecCount))
    {
        cpu.ecx = cpu.eax;
        cpu.eax <<= 2;
        cpu.eax -= cpu.ecx;
        cpu.eax <<= 4;
        cpu.eax += cpu.edx;
        cpu.ebx = cpu.eax + 4;
    }
    cpu.eax = cpu.ebx;
    cpu.ebp = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4dc490: the first of player eax's specs whose car id is the 16 bytes at
 * edx (repe cmpsb), in use or not; -1 for none. */
void findSpec(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 8;
    const reg32 found = cpu.ebp - 4, id = cpu.ebp - 8;
    put(app, id, cpu.edx);
    cpu.edx = 0xffffffff;
    cpu.ebx = 0;
    put(app, found, cpu.edx);
    cpu.ebx = specsOf(cpu.eax);
    if (cpu.ebx != 0)
    {
        for (cpu.edx = 0; sreg32(cpu.edx) < sreg32(kSpecCount); ++cpu.edx)
        {
            cpu.eax = cpu.edx * 4;
            cpu.eax -= cpu.edx;
            cpu.eax <<= 4;
            cpu.ecx = 0x10;
            cpu.eax += cpu.ebx;
            cpu.edi = at(app, id);
            cpu.esi = cpu.eax + 4;
            cpu.eax = 0;
            bool equal = true, below = false;
            while (cpu.ecx != 0)
            {
                const reg8 a = byte(app, cpu.esi++), b = byte(app, cpu.edi++);
                --cpu.ecx;
                equal = a == b;
                below = a < b;
                if (!equal)
                    break;
            }
            if (!equal)
                cpu.eax = below ? 0xffffffff : 1;
            if (cpu.eax == 0)
            {
                put(app, found, cpu.edx);
                break;
            }
        }
    }
    cpu.eax = at(app, found);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4dc500: player eax's first spec not in use (0x4dc3f0), or -1. */
void freeSpecSlot(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ebx = cpu.eax;
    cpu.eax = specsOf(cpu.ebx);
    bool found = false;
    if (cpu.eax != 0)
    {
        for (cpu.ecx = 0; sreg32(cpu.ecx) < sreg32(kSpecCount); ++cpu.ecx)
        {
            cpu.edx = cpu.ecx;
            cpu.eax = cpu.ebx;
            if (!direct(app, cpu, specUsed))
                return;
            if (cpu.al == 0)
            {
                found = true;
                break;
            }
        }
    }
    cpu.eax = found ? cpu.ecx : 0xffffffff;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4dc5a0: player eax's spec edx (0 to 49) given the car id at ebx (16
 * bytes) and the spec at ecx (0x1c), each zeros where there is none. */
void setSpec(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0xc;
    const reg32 table = cpu.ebp - 4, index = cpu.ebp - 8, spec = cpu.ebp - 0xc;
    put(app, index, cpu.edx);
    put(app, spec, cpu.ecx);
    cpu.edx = 0;
    put(app, table, cpu.edx);
    if (specsOf(cpu.eax) != 0)
        put(app, table, specsOf(cpu.eax));
    cpu.edi = at(app, table);
    const auto done = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.esp += 4;
    };
    if (cpu.edi == 0)
        return done();
    cpu.eax = at(app, index);
    if (sreg32(cpu.eax) < 0 || sreg32(cpu.eax) >= sreg32(kSpecCount))
        return done();
    cpu.edx = cpu.eax;
    cpu.eax <<= 2;
    cpu.eax -= cpu.edx;
    cpu.eax <<= 4;
    cpu.eax += cpu.edi;
    cpu.eax += 4;
    if (cpu.ebx != 0)
    {
        cpu.ecx = 0x10;
        cpu.esi = cpu.ebx;
        cpu.edi = cpu.eax;
        copyBytes(app, cpu);
    }
    else
    {
        cpu.ebx = 0x10;
        cpu.edx = 0;
        if (!byAddress(app, cpu, kMemset))
            return;
    }
    cpu.ebx = at(app, index);
    cpu.eax = cpu.ebx * 4;
    cpu.eax -= cpu.ebx;
    cpu.ebx = at(app, table);
    cpu.eax <<= 4;
    cpu.eax += cpu.ebx;
    cpu.esi = at(app, spec);
    cpu.eax += 0x14;
    if (cpu.esi != 0)
    {
        cpu.ecx = 0x1c;
        cpu.edi = cpu.eax;
        copyBytes(app, cpu);
    }
    else
    {
        cpu.ebx = 0x1c;
        cpu.edx = 0;
        if (!byAddress(app, cpu, kMemset))
            return;
    }
    done();
}

/* 0x4dc670: player eax's specs made anew: the table cleared, then each car
 * there is (0x439a50, to the first there is not) given a spec in use, its
 * id and the default spec, while there are specs free.  How many. */
void defaultSpecs(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 8;
    const reg32 car = cpu.ebp - 4, carRecord = cpu.ebp - 8;
    cpu.edi = cpu.eax;
    cpu.eax = 0;
    cpu.esi = 0;
    cpu.eax = specsOf(cpu.edi);
    if (cpu.eax != 0)
    {
        cpu.ebx = kSpecsBytes;
        cpu.edx = 0;
        if (!byAddress(app, cpu, kMemset))
            return;
        cpu.edx = 0;
        put(app, car, cpu.edx);
        do
        {
            cpu.eax = at(app, car);
            if (!byAddress(app, cpu, kCarRecord))
                return;
            put(app, carRecord, cpu.eax);
            if (cpu.eax != 0 && sreg32(cpu.esi) < sreg32(kSpecCount))
            {
                cpu.eax = at(app, car);
                if (!byAddress(app, cpu, kCarId))
                    return;
                if (cpu.eax != 0)
                {
                    cpu.ecx = kDefaultSpec;
                    cpu.ebx = cpu.eax;
                    cpu.edx = cpu.esi;
                    cpu.eax = cpu.edi;
                    if (!direct(app, cpu, setSpec))
                        return;
                    cpu.ebx = 1;
                    cpu.edx = cpu.esi;
                    cpu.eax = cpu.edi;
                    if (!direct(app, cpu, setSpecUsed))
                        return;
                    ++cpu.esi;
                }
            }
            cpu.ecx = at(app, car);
            ++cpu.ecx;
            cpu.ebx = at(app, carRecord);
            put(app, car, cpu.ecx);
        } while (cpu.ebx != 0);
    }
    cpu.eax = cpu.esi;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4dc710: player eax's specs brought up to the cars there are: each in use
 * whose car is no longer known (0x439990) cleared and marked free; each car
 * there is (to the first there is not) without a spec given one, free, its
 * id and the default spec.  al: whether anything was added or taken. */
void refreshSpecs(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 8;
    const reg32 car = cpu.ebp - 4, carRecord = cpu.ebp - 8;
    cpu.esi = cpu.eax;
    cpu.eax = 0;
    cpu.cl = 0;
    cpu.eax = specsOf(cpu.esi);
    if (cpu.eax != 0)
    {
        for (cpu.edi = 0; sreg32(cpu.edi) < sreg32(kSpecCount); ++cpu.edi)
        {
            cpu.edx = cpu.edi;
            cpu.eax = cpu.esi;
            if (!direct(app, cpu, specUsed))
                return;
            if (cpu.al == 0)
                continue;
            cpu.edx = cpu.edi;
            cpu.eax = cpu.esi;
            if (!direct(app, cpu, specCarName))
                return;
            if (cpu.eax == 0)
                continue;
            if (!byAddress(app, cpu, kCarIndex))
                return;
            if (cpu.eax != 0xffffffff)
                continue;
            cpu.edx = cpu.edi;
            cpu.eax = cpu.esi;
            cpu.ecx = 0;
            cpu.ebx = 0;
            if (!direct(app, cpu, setSpec))
                return;
            cpu.edx = cpu.edi;
            cpu.eax = cpu.esi;
            cpu.ebx = 0;
            cpu.cl = 1;
            if (!direct(app, cpu, setSpecUsed))
                return;
        }
        cpu.edx = 0;
        put(app, car, cpu.edx);
        do
        {
            cpu.eax = at(app, car);
            if (!byAddress(app, cpu, kCarRecord))
                return;
            put(app, carRecord, cpu.eax);
            if (cpu.eax != 0)
            {
                cpu.eax = at(app, car);
                if (!byAddress(app, cpu, kCarId))
                    return;
                cpu.ebx = cpu.eax;
                if (cpu.eax != 0)
                {
                    cpu.edx = cpu.eax;
                    cpu.eax = cpu.esi;
                    if (!direct(app, cpu, findSpec))
                        return;
                    if (cpu.eax == 0xffffffff)
                    {
                        cpu.eax = cpu.esi;
                        if (!direct(app, cpu, freeSpecSlot))
                            return;
                        cpu.edi = cpu.eax;
                        if (cpu.eax != 0xffffffff)
                        {
                            cpu.ecx = kDefaultSpec;
                            cpu.edx = cpu.eax;
                            cpu.eax = cpu.esi;
                            if (!direct(app, cpu, setSpec))
                                return;
                            cpu.ebx = 1;
                            cpu.edx = cpu.edi;
                            cpu.eax = cpu.esi;
                            cpu.cl = 1;
                            if (!direct(app, cpu, setSpecUsed))
                                return;
                        }
                    }
                }
            }
            cpu.ebx = at(app, car);
            ++cpu.ebx;
            cpu.edi = at(app, carRecord);
            put(app, car, cpu.ebx);
        } while (cpu.edi != 0);
    }
    cpu.al = cpu.cl;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4dc800: the specs' file of the player named eax, made at 0x564188: the
 * folder ([0x7a2fb4] on), "carspecs\", the name, ".spc".  Its address. */
void specPath(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = cpu.eax;
    cpu.esi = kSpecFolder;
    cpu.ah = 0;
    cpu.edi = kSpecPath;
    putByte(app, kSpecPath, cpu.ah);
    copyString(app, cpu);
    cpu.esi = kSpecsFolder;
    appendString(app, cpu);
    cpu.esi = cpu.edx;
    appendString(app, cpu);
    cpu.esi = kSpecExtension;
    appendString(app, cpu);
    cpu.eax = cpu.edi;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4dc8b0: player eax's specs from the file's ebx bytes at edx (0x30 at
 * least), a spec's 0x30 bytes at a time, 50 at most. */
void readSpecs(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0xc;
    const reg32 index = cpu.ebp - 4, bytes = cpu.ebp - 8, file = cpu.ebp - 0xc;
    put(app, file, cpu.edx);
    put(app, bytes, cpu.ebx);
    cpu.edx = 0;
    cpu.ebx = 0;
    put(app, index, cpu.edx);
    cpu.ebx = specsOf(cpu.eax);
    if (at(app, file) != 0 && cpu.ebx != 0 && at(app, bytes) >= kSpecBytes)
    {
        while (sreg32(cpu.edx) < sreg32(at(app, bytes)))
        {
            cpu.eax = at(app, index);
            if (sreg32(cpu.eax) >= sreg32(kSpecCount))
                break;
            cpu.ecx = cpu.eax;
            cpu.eax <<= 2;
            cpu.esi = at(app, file);
            cpu.eax -= cpu.ecx;
            cpu.esi += cpu.edx;
            cpu.eax <<= 4;
            cpu.ecx = kSpecBytes;
            cpu.edi = cpu.ebx + cpu.eax;
            copyBytes(app, cpu);
            cpu.ecx = at(app, index);
            ++cpu.ecx;
            cpu.edx += kSpecBytes;
            put(app, index, cpu.ecx);
        }
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4dc940: player eax's specs loaded, when the player's name will do: the
 * table cleared; a file of another machine's (0x472940, by the module's name)
 * deleted, and with none the specs made anew (0x4dc670) and saved; else the
 * file read in (0x4dc8b0) and freed, and saved again if bringing it up to
 * the cars there are changed it -- made anew instead when it would not
 * load. */
void loadPlayerSpecs(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0xfc;
    const reg32 module = cpu.ebp - 0xfc;
    cpu.esi = cpu.eax;
    cpu.edi = 0;
    cpu.ecx = 0;
    if (cpu.eax == 0)
    {
        cpu.edi = kFirstSpecs;
        cpu.ecx = kFirstPlayer;
    }
    else if (cpu.eax == 1)
    {
        cpu.edi = kSecondSpecs;
        cpu.ecx = kSecondPlayer;
    }
    const auto done = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    if (cpu.ecx == 0)
        return done();
    cpu.eax = cpu.ecx;
    if (!byAddress(app, cpu, kNameValid))
        return;
    if (cpu.eax == 0 || cpu.edi == 0)
        return done();
    cpu.ebx = kSpecsBytes;
    cpu.eax = cpu.edi;
    cpu.edx = 0;
    if (!byAddress(app, cpu, kMemset))
        return;
    cpu.eax = cpu.ecx;
    if (!direct(app, cpu, specPath))
        return;
    push(app, cpu, 0xf9);
    cpu.ebx = cpu.eax;
    cpu.edi = cpu.eax;
    cpu.eax = module;
    push(app, cpu, cpu.eax);
    push(app, cpu, 0);
    if (!byAddress(app, cpu, at(app, kGetModuleFileName)))
        return;
    cpu.eax = cpu.ebx;
    if (!byAddress(app, cpu, kFileExists))
        return;
    cpu.ecx = cpu.eax;
    bool anew = cpu.ecx == 0;
    if (!anew)
    {
        cpu.edx = module;
        cpu.eax = cpu.ebx;
        if (!byAddress(app, cpu, kOwnSpecs))
            return;
        if (cpu.eax != 0)
        {
            cpu.eax = cpu.ebx;
            if (!byAddress(app, cpu, kUnlink))
                return;
            anew = true;
        }
    }
    if (anew)
    {
        cpu.eax = cpu.esi;
        if (!direct(app, cpu, defaultSpecs))
            return;
        cpu.eax = cpu.esi;
        if (!direct(app, cpu, savePlayerSpecs))
            return;
        return done();
    }
    cpu.eax = cpu.edi;
    cpu.edx = 0x10;
    if (!byAddress(app, cpu, kFileSize))
        return;
    cpu.ebx = cpu.eax;
    cpu.eax = cpu.edi;
    if (!byAddress(app, cpu, kLoadQuietly))
        return;
    cpu.ecx = cpu.eax;
    if (cpu.eax == 0)
    {
        cpu.eax = cpu.esi;
        if (!direct(app, cpu, defaultSpecs))
            return;
        return done();
    }
    if (sreg32(cpu.ebx) > 0)
    {
        cpu.edx = cpu.eax;
        cpu.eax = cpu.esi;
        if (!direct(app, cpu, readSpecs))
            return;
    }
    cpu.eax = cpu.ecx;
    if (!byAddress(app, cpu, kFree))
        return;
    cpu.eax = cpu.esi;
    if (!direct(app, cpu, refreshSpecs))
        return;
    if (cpu.al != 0)
    {
        cpu.eax = cpu.esi;
        if (!direct(app, cpu, savePlayerSpecs))
            return;
    }
    done();
}

/* 0x4dca50: player eax's specs saved, when the player's name will do: the
 * file made writable if it is there and is not (chmod 0x180), the specs
 * brought up to the cars there are (0x4dc710), the table written whole
 * (0x4eebb0).  What that gives, or 0. */
void savePlayerSpecs(win32::WinApplication* app, x86::CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esi = cpu.eax;
    cpu.ebx = 0;
    cpu.ecx = 0;
    cpu.edx = 0;
    if (cpu.eax == 0)
    {
        cpu.ecx = kFirstSpecs;
        cpu.ebx = kFirstPlayer;
    }
    else if (cpu.eax == 1)
    {
        cpu.ecx = kSecondSpecs;
        cpu.ebx = kSecondPlayer;
    }
    const auto done = [&] {
        cpu.eax = cpu.edx;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    if (cpu.ebx == 0)
        return done();
    cpu.eax = cpu.ebx;
    if (!byAddress(app, cpu, kNameValid))
        return;
    if (cpu.eax == 0 || cpu.ecx == 0)
        return done();
    cpu.eax = cpu.ebx;
    if (!direct(app, cpu, specPath))
        return;
    cpu.ebx = cpu.eax;
    cpu.edi = cpu.eax;
    if (!byAddress(app, cpu, kFileExists))
        return;
    if (cpu.eax != 0)
    {
        cpu.edx = 2;
        cpu.eax = cpu.ebx;
        if (!byAddress(app, cpu, kAccess))
            return;
        if (cpu.eax != 0)
        {
            cpu.edx = 0x180;
            cpu.eax = cpu.ebx;
            if (!byAddress(app, cpu, kChmod))
                return;
        }
    }
    cpu.eax = cpu.esi;
    cpu.ebx = kSpecsBytes;
    if (!direct(app, cpu, refreshSpecs))
        return;
    cpu.edx = cpu.ecx;
    cpu.eax = cpu.edi;
    if (!byAddress(app, cpu, kSaveFile))
        return;
    cpu.edx = cpu.eax;
    done();
}

}  // namespace load
}  // namespace nfs3hp
