#ifndef NFS3HP_NATIVE_SND_H
#define NFS3HP_NATIVE_SND_H

#include "native_guest.h"

#include <cstdint>

/* What native_snd_*.cpp share: EA's SND library's state where nfs3.exe keeps
 * it, and the helpers their functions are written with.
 *
 * What is left of the game's sound code at the register level here
 * (native_snd_speech.cpp's description bank, native_snd_dead.cpp's stubs)
 * does what the game's code does -- the same memory written in the same order,
 * the game's functions called with the same registers and stack, the
 * registers its callers count on as the code leaves them, eax its result, and
 * the return address popped.  Each address is bound to the
 * function of that name (SND_NATIVES).  The SND library itself -- voices, banks, playing -- is
 * rewritten as the port's own C++ (port_snd.h). */
namespace nfs3hp
{
namespace snd
{

using guest::at;
using guest::byte;
using guest::call;
using guest::put;
using guest::putByte;
using x86::reg16;
using x86::reg32;
using x86::reg8;
using App = win32::WinApplication;
using CPU = x86::CPU;
using i32 = std::int32_t;
using i16 = std::int16_t;
using i8 = std::int8_t;

inline i32 s32(App* app, reg32 a) { return i32(at(app, a)); }
inline i32 sbyte(App* app, reg32 a) { return i8(byte(app, a)); }
inline reg16 half(App* app, reg32 a) { return app->getMemory<reg16>(a); }
inline i32 shalf(App* app, reg32 a) { return i16(half(app, a)); }
inline void putHalf(App* app, reg32 a, reg16 v) { app->getMemory<reg16>(a) = v; }

/* imul and idiv as the code has them: 32 bits, wrapping; a quotient towards 0. */
inline i32 mul(i32 a, i32 b) { return i32(reg32(a) * reg32(b)); }
inline i32 quotient(i32 a, i32 b) { return b == -1 ? i32(0u - reg32(a)) : a / b; }

inline void setByte0(reg32& r, reg8 v) { r = (r & 0xffffff00) | v; }
inline void setByte1(reg32& r, reg8 v) { r = (r & 0xffff00ff) | (reg32(v) << 8); }

/* A pan (0..0xffff around) as stereo's (0..127), as the game's code makes it. */
inline reg32 stereoPan(reg32 pan)
{
    const reg32 a = i32(pan) > 0x3fff && i32(pan) < 0xc000 ? 0xbfff - pan : (pan + 0x4000) & 0xffff;
    return reg32(i32(a) >> 8);
}

// The library's state.
constexpr reg32 kVoices = 0xa09b3c, kVoiceSize = 0x60, kVoiceCount = 0x10;
constexpr reg32 kPlatform = 0x9ef658;
constexpr reg32 kChannels = 0x9fbcb4, kChannelSize = 0xde4;
constexpr reg32 kInited = 0xa09af4;         // byte: SNDinit done
constexpr reg32 kMasterVolume = 0xa09af5;   // byte, 0..127
constexpr reg32 kVoicesUsed = 0xa09af6;     // byte: the voices the game asked for
constexpr reg32 kInTick = 0xa09af7;         // byte: in the tick
constexpr reg32 kEntered = 0xa09af8;        // byte: in the library's lock
constexpr reg32 kClientCount = 0xa09af9;    // byte: the tick's clients
constexpr reg32 kTicks = 0xa09afc;
constexpr reg32 kLock = 0xa09b04;           // the critical section's address
constexpr reg32 kClients = 0xa09b0c;        // 6 functions the tick calls first
constexpr reg32 kGroupSerial = 0x5678b0;    // byte: the last group handed out
constexpr reg32 kHandleSerial = 0x56786c;   // a handle's high bits, 0x20 a time
constexpr reg32 kPicked = 0x9f6a54;         // bytes: the voices allocVoices picked
constexpr reg32 kMixRate = 0x9ef614;        // word: the mixer's rate; its high word a pitch's offset
constexpr reg32 kBanks = 0xa0a13c;          // 24 of 12 bytes: the bank, ..., +8 being loaded
constexpr reg32 kBusLevels = 0xa0a260;      // the effect bus's level, 0x10 a bus
constexpr reg32 kChannelEnded = 0x9f6b1c;   // what the mixer calls when a channel ends
constexpr reg32 kEmms = 0x9f6b24;
// The game's functions called here.
constexpr reg32 kSurfaceLocked = 0x4f9070;
constexpr reg32 kEnterCriticalSection = 0x5344b8, kLeaveCriticalSection = 0x534588;
constexpr reg32 kChop = 0x4dfd56;           // Watcom's __CHP: st(0) truncated
constexpr reg32 kRamp = 0x5004c8;           // the mixer's volume ramp (channel, frames)
// The eacsnd 3D functions the 3D voices go to.
constexpr reg32 kDirectPos3d = 0x9ef644, kDirectVol = 0x9ef648, kDirectRate = 0x9ef64c, kDirectStopBuf = 0x9ef650;

inline reg32 voice(reg32 index) { return kVoices + index * kVoiceSize; }
inline reg32 platform(reg32 index) { return kPlatform + (index << 4); }
inline reg32 channel(reg32 index) { return kChannels + index * kChannelSize; }

}  // namespace snd
}  // namespace nfs3hp

#endif
