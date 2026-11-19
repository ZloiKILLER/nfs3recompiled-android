#ifndef NFS3HP_ISND_H
#define NFS3HP_ISND_H

/* iSNDdirect: the interface between nfs3.exe and its sound driver, eacsnd.dll
 * -- 16 stdcall exports the game finds with GetProcAddress (iSNDloaddll,
 * pointers at 0x9ef61c..).  The game mixes everything itself; the driver only
 * feeds the mix to the card.  eacsnd has no code of its own in the port: its exports are
 * the port's functions of the same names, in
 * native_sound.cpp (the driver proper: caps, start, stop, serve) and
 * port_snd_isnd.cpp (the rest).
 *
 * eacsnd's start
 * (sub_a33150) tries 22050 first, then 16000, 32000, 44100 and 11025, and so
 * does the port.
 *
 * The exports, their address in eacsnd and the argument bytes:
 *
 *   iSNDdllversion          @0  0xa32bc4  kVersion, and the caps forgotten
 *   iSNDdirectsetfunctions  @32 0xa32bd8  the eight kFunction* below
 *   iSNDdirectcaps          @4  0xa32c28  window: kCaps* of the card
 *   iSNDdirectstart         @8  0xa3382c  kCaps* chosen, window
 *   iSNDdirectstop          @0  0xa3387c
 *   iSNDdirectrecordstart   @16 0xa338fc  recording: -1
 *   iSNDdirectrecordpacket  @4  0xa33904  0
 *   iSNDdirectrecordstop    @0  0xa3390c  0
 *   iSNDdirectserve         @0  0xa33b24  the mix topped up
 *   iSNDdirectvol           @8  0xa33e18  buffer, volume
 *   iSNDdirectpos3d         @12 0xa33e68  buffer, two position words
 *   iSNDdirectrate          @8  0xa33f0c  buffer, rate
 *   iSNDdirectcreate3dbuf   @28 0xa33f4c  volume, ?, frequency, ?, ?, ?, id: a slot
 *   iSNDdirectremovebuf     @4  0xa33fc0  slot
 *   iSNDdirectplay3d        @28 0xa34014  slot, buffer, looping, rate, volume, two position words
 *   iSNDdirectstopbuf       @4  0xa341b0  buffer
 *
 * The 3D buffers are never made: in eacsnd only its start (sub_a33150) sets
 * kDirectSoundMade and kIs3d, and the port's start is native_sound.cpp's,
 * which sets neither.  So the 3D functions always
 * take eacsnd's way without DirectSound: they keep what they are given, and
 * play3d, asked to play with no 3D, releases and clears the three pointers
 * at the caller's esi, as eacsnd does.
 *
 * The game keeps a pointer to each export but the first two at 0x9ef61c on
 * (kDirect*, kExports below); native code calls them through it,
 * guest::sound (isnd_calls.h). */

#include <game_types.h>

namespace nfs3hp
{
namespace isnd
{

/* The game's pointers to the exports, iSNDloaddll's lookups after
 * setfunctions (native_game.cpp), in their order. */
constexpr nfs::u32 kDirectCaps = 0x9ef61c;
constexpr nfs::u32 kDirectStart = 0x9ef620;
constexpr nfs::u32 kDirectStop = 0x9ef624;
constexpr nfs::u32 kDirectRecordStart = 0x9ef628;
constexpr nfs::u32 kDirectRecordPacket = 0x9ef62c;
constexpr nfs::u32 kDirectRecordStop = 0x9ef630;
constexpr nfs::u32 kDirectServe = 0x9ef634;
constexpr nfs::u32 kDirectCreate3dBuf = 0x9ef638;
constexpr nfs::u32 kDirectRemoveBuf = 0x9ef63c;
constexpr nfs::u32 kDirectPlay3d = 0x9ef640;
constexpr nfs::u32 kDirectPos3d = 0x9ef644;
constexpr nfs::u32 kDirectVol = 0x9ef648;
constexpr nfs::u32 kDirectRate = 0x9ef64c;
constexpr nfs::u32 kDirectStopBuf = 0x9ef650;

/* Each pointer with its export: eacsnd's address of it (the list above) and
 * the bytes of its words. */
struct Export
{
    nfs::u32 slot, eacsnd, bytes;
};

constexpr Export kExports[] = {
    {kDirectCaps, 0xa32c28, 4},         {kDirectStart, 0xa3382c, 8},        {kDirectStop, 0xa3387c, 0},
    {kDirectRecordStart, 0xa338fc, 16}, {kDirectRecordPacket, 0xa33904, 4}, {kDirectRecordStop, 0xa3390c, 0},
    {kDirectServe, 0xa33b24, 0},        {kDirectCreate3dBuf, 0xa33f4c, 28}, {kDirectRemoveBuf, 0xa33fc0, 4},
    {kDirectPlay3d, 0xa34014, 28},      {kDirectPos3d, 0xa33e68, 12},       {kDirectVol, 0xa33e18, 8},
    {kDirectRate, 0xa33f0c, 8},         {kDirectStopBuf, 0xa341b0, 4},
};

/* The export whose pointer is at `slot`; one of no address for another
 * slot. */
constexpr Export exportAt(nfs::u32 slot)
{
    for (const Export& e : kExports)
        if (e.slot == slot)
            return e;
    return {slot, 0, 0};
}

/* iSNDdllversion's answer: 16.3. */
constexpr nfs::u32 kVersion = 0x00100003;

/* The results: 0, or one of these. */
constexpr nfs::u32 kFailure = nfs::u32(-1);
constexpr nfs::u32 kMissingDriver = nfs::u32(-10);    // DSERR_NODRIVER
constexpr nfs::u32 kInvalidVersion = nfs::u32(-11);   // start does not give it
constexpr nfs::u32 kAlreadyAllocated = nfs::u32(-18); // DSERR_ALLOCATED

/* What iSNDdirectcaps answers, and what the game hands iSNDdirectstart: the
 * formats and rates the card plays.  start takes one format and one rate from
 * them, each in eacsnd's order. */
constexpr nfs::u32 kCaps8BitMono = 0x00001;
constexpr nfs::u32 kCaps8BitStereo = 0x00002;
constexpr nfs::u32 kCaps16BitMono = 0x00004;
constexpr nfs::u32 kCaps16BitStereo = 0x00008;
constexpr nfs::u32 kCaps11025 = 0x00200;
constexpr nfs::u32 kCaps16000 = 0x00400;
constexpr nfs::u32 kCaps22050 = 0x00800;
constexpr nfs::u32 kCaps32000 = 0x01000;
constexpr nfs::u32 kCaps44100 = 0x02000;
constexpr nfs::u32 kCapsUnknown = 0x10000;              // what it means is not known
constexpr nfs::u32 kCaps3dBuffers = 0x20000;
constexpr nfs::u32 kCapsReverb = 0x40000;
constexpr nfs::u32 kCapsAnyRate = kCaps11025 | kCaps16000 | kCaps22050 | kCaps32000 | kCaps44100;

/* eacsnd's data, at its load address. */
constexpr nfs::u32 kDirectSound = 0xa3c09c;             // IDirectSound*
constexpr nfs::u32 kDirectSoundMade = 0xa3c0b0;         // byte: 1 once DirectSoundCreate has answered
constexpr nfs::u32 kRate = 0xa3c100;                    // the rate start chose, Hz
constexpr nfs::u32 kCapsKept = 0xa3c0fc;                // iSNDdirectcaps's answer; -1 until asked, and again after iSNDdllversion
constexpr nfs::u32 kIs3d = 0xa3c0b7;                    // byte: start was asked for 3D buffers
constexpr nfs::u32 kPositionScale = 0xa3baec;           // a double: pos3d's factor from the game's position to DirectSound's

/* The buffers playing, kBufferCount of kBufferBytes. */
constexpr nfs::u32 kBuffers = 0xa3c124;
constexpr nfs::u32 kBufferBytes = 0x18;
constexpr nfs::u32 kBufferCount = 16;
constexpr nfs::u32 kBufferActive = 0x00;                // IDirectSoundBuffer*: what plays, the slot's or a duplicate of it
constexpr nfs::u32 kBufferMain = 0x04;                  // IDirectSoundBuffer*: the slot's
constexpr nfs::u32 kBuffer3d = 0x08;                    // IDirectSound3DBuffer*
constexpr nfs::u32 kBufferPositionA = 0x0c;             // word: pos3d's second argument
constexpr nfs::u32 kBufferPositionB = 0x0e;             // word: its third
constexpr nfs::u32 kBufferRate = 0x10;                  // word
constexpr nfs::u32 kBufferLooping = 0x12;               // byte
constexpr nfs::u32 kBufferVolume = 0x13;                // byte
constexpr nfs::u32 kBufferSlot = 0x14;                  // byte

/* The sounds made, kSlotCount of kSlotBytes; slot 0 is never
 * handed out. */
constexpr nfs::u32 kSlots = 0xa3c2a4;
constexpr nfs::u32 kSlotBytes = 0x1c;
constexpr nfs::u32 kSlotCount = 64;
constexpr nfs::u32 kSlotId = 0x00;                      // 0: free
constexpr nfs::u32 kSlotBuffer = 0x04;                  // IDirectSoundBuffer*

/* iSNDdirectsetfunctions' eight functions of the game, in the order they are
 * given and kept: each the native of that name in native_game.cpp, where the
 * game's code was -- stdcall, but for the three that format, cdecl. */
constexpr nfs::u32 kFunctionMix = 0xa3c104;             // AcquireSoundDataLambda, 0x4e1a24: mix into (buffer, frames)
constexpr nfs::u32 kFunctionStopBuffer = 0xa3c108;      // StopSoundBufferLambda, 0x4e1a00: (buffer)
constexpr nfs::u32 kFunctionLog = 0xa3c10c;             // abortmessagewrapper, 0x4e1a88: (text, ...), an Error
constexpr nfs::u32 kFunction4 = 0xa3c110;               // TraceSoundLambda, 0x4e1a64: (format, ...), to the log
constexpr nfs::u32 kFunctionTick = 0xa3c114;            // TickSoundLambda, 0x4e1adc: the game's 100 Hz tick
constexpr nfs::u32 kFunction6 = 0xa3c118;               // PrintSoundLambda, 0x4e1a34: (format, ...), to stdout
constexpr nfs::u32 kFunctionBufferPosition = 0xa3c11c;  // AcquireSoundBufferPositionLambda, 0x4e1a0c
constexpr nfs::u32 kFunctionConvert = 0xa3c120;         // iSNDconvertsample, 0x4e1ae4

}  // namespace isnd
}  // namespace nfs3hp

#endif
