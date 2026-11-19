#include "game_settings.h"
#include "native_controls_queue.h"
#include "native_ea_net.h"
#include "native_guest.h"
#include "os_threads.h"
#include <lib/app.h>

#include <optional>

/* The race's queue of inputs: each player's four bytes by frame, handed to
 * the cars and to the other machines in a network race, and the edges of the
 * keys and actions the race's screens ask about.
 *
 * The queue is a ring of 128 frames for each of 8 players: a player's four
 * bytes (native_controls_pipeline.cpp's: the steering, the throttle, the
 * brake, 8 actions' bits) at 0x7d3714 + player * 0x200 + (frame & 0x7f) * 4,
 * a dword at 0x7d4714 on the same way saying the slot holds them, and each
 * player's frames queued so far at 0x7d5718 + player * 4 -- the next frame
 * queued goes to that count's slot.  Frames taken by the race so far are at
 * 0x7d5714: each frame of the race takes one frame's slots of every player
 * into 0x7d36f0 (player * 4) and frees them, and each car copies its own
 * player's four bytes from there into 0x79c4a8, which its code reads.  How
 * far the queue may run ahead of the frames taken (its lead, 0x7d573c, and
 * a remote player's, 0x7d5738, 0x7d3710) the race's mode sets: one frame
 * ahead of what is taken in one player's race, more over the network.  The
 * frames the race may run, 0x560104, counted up as frames are queued.
 *
 * Each is a typed function under a name of its own, with a thunk under the
 * game's name for the callers still on registers and the calls through a pointer: it leaves
 * eax -- and for some edx or ebx, which the code leaves otherwise and no
 * caller reads -- as the game's code does, every other register as the caller
 * had it.  A dword is copied as the code copies it (movsd, then movsb for
 * none).  The switches on the race's mode go by the targets nfs3.exe's tables
 * hold (0x4c6514, 0x4c6a2c, 0x4c6acc).  The network's packets sent and a
 * machine's player found are native_ea_net.h's, called as C++
 * (guest::direct); the critical section's Windows functions are called by
 * address, their registers kept (EspKept), under the network's lock as
 * the code takes it. */
namespace nfs3hp
{
namespace controls
{

namespace
{

using guest::at;
using guest::byte;
using guest::put;
using guest::putByte;
using nfs::u16;
using nfs::u32;
using nfs::u8;
using nfs::s32;
using App = nfs::App;
using ThreadState = nfs::ThreadState;

// The race's frame as its screens see it (0x79c4ac as the frame began); the
// actions' and the function keys' rings of 32 frames of bits; a function
// key's frames held unchanged.
constexpr u32 kScreenFrame = 0x7d3688;
constexpr u32 kActionRing = 0x79c238, kFunctionKeyRing = 0x79c2b8, kRingMask = 0x1f;
constexpr u32 kRepeatFrames = 0x79c4b0;
// The keys' state, a byte each by scan code, and whether each was pressed
// already, a byte each (0xff) up to 0x80.
constexpr u32 kKeys = 0x564818, kKeysPressed = 0x79c1b8, kKeyCount = 0x80;
constexpr u8 kPressed = 0xff;
// The car's four bytes, taken from the queue; its buttons' byte; the
// players' buttons held the frame before, a byte each.
constexpr u32 kCarControls = 0x79c4a8, kCarButtons = 0x79c4ab;
constexpr u32 kButtonsBefore = 0x79c1b0;
// The buttons that count once as pressed, not while held: 0x02, 0x10, 0x20,
// 0x40, 0x80; and those a frame repeated for a player missing keeps.
constexpr u8 kPressedButtons = 0xf2, kRepeatedButtons = 0x8d;
constexpr u8 kMenuUp = 0x10, kMenuDown = 0x20;
// The queue.
constexpr u32 kQueue = 0x7d3714, kQueueButtons = 0x7d3717, kQueued = 0x7d4714;
constexpr u32 kFramesTaken = 0x7d5714, kFramesQueued = 0x7d5718, kSecondQueued = 0x7d571c;
constexpr u32 kTaken = 0x7d36f0;
constexpr u32 kRemoteLead = 0x7d3710, kRemoteDelay = 0x7d5738, kQueueLead = 0x7d573c, kLag = 0x7d5740;
constexpr u32 kFrameMask = 0x7f, kPlayerShift = 9;
constexpr u32 kControlsBytes = 4;          // a player's four bytes, which rep movs leaves in eax
// The race: its modes (one player, two on one screen, past those over the network).
constexpr u32 kOneMode = 0, kSplitMode = 1, kLastMode = 4;
constexpr u32 kRaceFrames = 0x560104;      // the frames the race may run
constexpr u32 kQueueStopped = 0x560108;
constexpr u32 kStarting = 0x56010c;
constexpr u32 kTakingPackets = 0x560100;
// The network: its kind (1 the host's side or a client's of one, 2 two
// machines), this machine its host, the connection lost; its lock; the
// packet of this machine's input -- 6, its lead, its four bytes; the packet
// being built, its length and its start.
constexpr u32 kTwoMachines = 0, kHostKind = 1;
constexpr u32 kNetworkKind = 0x7a22e4, kNetworkHost = 0x7a22da;
constexpr u32 kConnectionLost = 0x7a22dc, kConnectionLostToo = 0x7a22de;
constexpr u32 kNetworkLock = 0x7d36d8;
constexpr u32 kInputPacket = 0x7d36e4, kInputPacketLead = 0x7d36e5, kInputPacketControls = 0x7d36e6;
constexpr u8 kInputPacketType = 6;
constexpr u32 kInputPacketBytes = 6, kAllInputsBytes = 0x21;
constexpr u32 kPacketLength = 0x7d36d0, kPacketStart = 0x7d36d4;

u16 half(App* app, u32 address)
{
    return app->getMemory<u16>(address);
}

/* A frame whose address a callee is handed, on the guest's stack at the
 * code's depth: `pushed` bytes of registers pushed, then `bytes` of locals. */
class GuestFrame
{
public:
    GuestFrame(ThreadState& ts, u32 pushed, u32 bytes) : ts_(ts), esp_(ts.sp), base_(ts.sp - pushed)
    {
        ts_.sp = base_ - bytes;
    }
    ~GuestFrame() { ts_.sp = esp_; }
    u32 operator-(u32 offset) const { return base_ - offset; }

private:
    ThreadState& ts_;
    const u32 esp_;
    const u32 base_;
};

/* The lock `lock` entered or left (os::locks), with eax and edx as the code has them as it calls:
 * the lock's functions leave both as they are.  nullopt when the game stopped. */
std::optional<Answered> lockEnter(App* app, ThreadState& ts, u32 lock, u32 eaxBefore, u32 edxBefore)
{
    if (!os::locks::enter(app, ts, lock))
        return std::nullopt;
    return Answered{eaxBefore, edxBefore};
}

std::optional<Answered> lockLeave(App* app, ThreadState& ts, u32 lock, u32 eaxBefore, u32 edxBefore)
{
    if (!os::locks::leave(app, ts, lock))
        return std::nullopt;
    return Answered{eaxBefore, edxBefore};
}

/* `count` bytes from `from` to `to`, as rep movsd and rep movsb copy them: a
 * dword at a time, then the bytes left. */
void copyBytes(App* app, u32 to, u32 from, u32 count)
{
    for (u32 n = count >> 2; n != 0; --n, to += 4, from += 4)
        put(app, to, at(app, from));
    for (u32 n = count & 3; n != 0; --n, ++to, ++from)
        putByte(app, to, byte(app, from));
}

/* Player's slot for frame `index` (0 to 0x7f) in the queue's tables. */
u32 slotOf(u32 player, u32 index)
{
    return (player << kPlayerShift) + index * 4;
}

/* 0x4c6840: player's four bytes taken for the frame (0x7d36f0) copied to
 * `to`. */
void copyTaken(App* app, u32 player, u32 to)
{
    copyBytes(app, to, kTaken + (player << 2), kControlsBytes);
}

/* 0x48f6a0: the car of `player` given its four bytes for the frame (0x79c4a8,
 * from those taken); of its buttons, those of 0xf2 count only the frame they
 * are pressed -- held the frame before (0x79c1b0 by player), they are
 * cleared.  Returns the buttons of 0xf2 held. */
u32 takeCarFrame(App* app, u32 player)
{
    copyTaken(app, player, kCarControls);
    const u8 heldBefore = byte(app, player + kButtonsBefore);
    const u8 held = byte(app, kCarButtons) & kPressedButtons;
    putByte(app, player + kButtonsBefore, held);
    putByte(app, kCarButtons, byte(app, kCarButtons) & u8(~heldBefore));
    return held;
}

}  // namespace

/* 0x48f6e0: choice `choice` of `count` stepped by the first player's buttons:
 * one on with 0x10 if not the last, one back with 0x20 if not the first. */
u8 stepChoice(App* app, u8 choice, u32 count)
{
    const u8 buttons = byte(app, kCarButtons);
    if (buttons & kMenuUp)
    {
        if (s32(choice) < s32(count - 1))
            return u8(choice + 1);
    }
    else if ((buttons & kMenuDown) && choice != 0)
        return u8(choice - 1);
    return choice;
}

/* 0x48f710: choice `choice` of three stepped by the first player's buttons:
 * 0x10 0 to 1, 1 to 2; 0x20 1 to 0, any past 1 to 1. */
u8 stepChoiceOfThree(App* app, u8 choice)
{
    const u8 buttons = byte(app, kCarButtons);
    if (buttons & kMenuUp)
    {
        if (choice == 0)
            return 1;
        if (choice == 1)
            return 2;
    }
    else if (buttons & kMenuDown)
    {
        if (choice == 1)
            return 0;
        if (choice > 1)
            return 1;
    }
    return choice;
}

namespace
{

/* A bit's mask in a ring's slots. */
u32 maskOf(u32 bit)
{
    return 1u << (bit & 0x1f);
}

/* The ring's slot for the screens' frame, and for the frame before. */
u32 ringNow(App* app, u32 ring)
{
    return at(app, (at(app, kScreenFrame) & kRingMask) * 4 + ring);
}

u32 ringBefore(App* app, u32 ring)
{
    return at(app, ((at(app, kScreenFrame) - 1) & kRingMask) * 4 + ring);
}

/* 0x48f740, 0x48f7b0: whether `bit` of the ring is held at the screens'
 * frame or, `edge`, was pressed in it -- not held the frame before. */
bool ringBitPressed(App* app, u32 ring, u32 bit, bool edge)
{
    if (edge)
        return (maskOf(bit) & ringNow(app, ring) & ~ringBefore(app, ring)) != 0;
    return (ringNow(app, ring) & maskOf(bit)) != 0;
}

/* 0x48f820: function key `bit` held at the screens' frame and the one before:
 * held, not pressed, so that it counts as pressed no more.  Returns its
 * mask. */
u32 holdFunctionKey(App* app, u32 bit)
{
    const u32 frame = at(app, kScreenFrame);
    const u32 mask = maskOf(bit);
    const u32 before = ((frame - 1) & kRingMask) * 4 + kFunctionKeyRing;
    put(app, before, at(app, before) | mask);
    const u32 now = (frame & kRingMask) * 4 + kFunctionKeyRing;
    put(app, now, at(app, now) | mask);
    return mask;
}

/* 0x48f860: whether function key `bit` is held at the screens' frame and the
 * function keys changed since the frame before, or have not for more than
 * `frames` frames (counted at 0x79c4b0, from 0 again as it answers so). */
bool functionKeyRepeat(App* app, u32 bit, u32 frames)
{
    u32 unchanged = at(app, kRepeatFrames);
    bool repeats = false;
    if ((ringNow(app, kFunctionKeyRing) & maskOf(bit)) != 0)
    {
        if (ringNow(app, kFunctionKeyRing) != ringBefore(app, kFunctionKeyRing))
            repeats = true;
        else
            repeats = s32(unchanged++) > s32(frames);
    }
    if (repeats)
        unchanged = 0;
    put(app, kRepeatFrames, unchanged);
    return repeats;
}

/* 0x48f8d0: whether key `key` (a scan code under 0x80) is held or, `edge`,
 * was pressed since it was last asked: its byte at 0x79c1b8 0xff as it says
 * so, cleared once the key is let go. */
bool keyDownOrPressed(App* app, u32 key, bool edge)
{
    if (s32(key) >= s32(kKeyCount))
        return false;
    const u8 held = byte(app, key + kKeys);
    const u8 asked = byte(app, key + kKeysPressed) & held;
    putByte(app, key + kKeysPressed, asked);
    if (!edge)
        return held != 0;
    if (held == 0 || asked != 0)
        return false;
    putByte(app, key + kKeysPressed, kPressed);
    return true;
}

/* 0x48f920: whether the first function key (bit 0, no key's -- only what
 * holdFunctionKey sets) is held at the screens' frame. */
u32 firstFunctionKey(App* app)
{
    return ringNow(app, kFunctionKeyRing) & 1;
}

/* 0x4c6080: a packet's `length` bytes but its first (its type) at `packet`
 * appended to the packet being built ([0x7d36d4] on, [0x7d36d0] long so
 * far).  Returns the bytes appended. */
u32 appendPacket(App* app, u32 packet, u32 length)
{
    const u32 to = at(app, kPacketLength) + at(app, kPacketStart);
    const u32 bytes = length - 1;
    copyBytes(app, to, packet + 1, bytes);
    put(app, kPacketLength, at(app, kPacketLength) + bytes);
    return bytes;
}

/* 0x4c6ae0: 1 if player's queue is as far ahead of the frames taken as it
 * may be (the lead, 0x7d573c), so that no more is queued, else 0.  One
 * player's race asks of player 0's frames only, two on one screen of the
 * first's (the second's always 0).  Over the network: with only this
 * machine's queued here, 0 -- the code compares the frames and goes on
 * whatever they say; the host's side (kind 1) with player 0 on it, also as
 * the race starts, and its lag (0x7d5740) halved instead of the answer while
 * more than the remote delay; otherwise, this machine's player only, its lag
 * halved over 4 instead, and the fewer of the first two players' frames
 * compared.  The lead and the lag written back as they end. */
u32 queueAhead(App* app, u32 player)
{
    u8 lag = byte(app, kLag);
    const u32 lead = at(app, kQueueLead);
    const u32 mode = gameSettings(app).playMode;
    const auto answer = [&](u32 full) {
        put(app, kQueueLead, lead);
        putByte(app, kLag, lag);
        return full;
    };
    // Player 0's frames as far ahead of those taken as the lead allows.
    const auto aheadOfTaken = [&] {
        return answer(s32(at(app, kFramesTaken) + lead) > s32(at(app, kFramesQueued)) ? 0 : 1);
    };
    // The table at 0x4c6acc: modes 0 and 1 their own, 2 to 4 the network's.
    if (mode > kLastMode)
        return answer(0);
    if (mode == kSplitMode)
        return player != 0 ? answer(0) : aheadOfTaken();
    if (mode == kOneMode)
        return aheadOfTaken();
    const u32 delayed = at(app, kFramesTaken) + at(app, kRemoteDelay);
    if (gameSettings(app).networkGame != 0)
        return answer(0);  // cmp edx, [ecx + 0x7d5718], its answer not looked at
    if (at(app, kNetworkKind) == kHostKind)
    {
        if (gameSettings(app).localPlayer != 0)
            return answer(0);
        if (at(app, kStarting) != 0 && player != 0)
            return answer(1);
        if (player == 0 && s32(lag) > s32(at(app, kRemoteDelay)))
        {
            lag = u8(lag >> 1);
            return answer(1);
        }
        return aheadOfTaken();
    }
    if (player != gameSettings(app).localPlayer)
        return answer(0);
    if (s32(lag) > 4)
    {
        lag = u8(lag >> 1);
        return answer(1);
    }
    if (s32(delayed) <= s32(at(app, player * 4 + kFramesQueued)))
        return answer(1);
    u32 fewer = at(app, kFramesQueued);
    const u32 second = at(app, kSecondQueued);
    if (s32(fewer) > s32(second))
        fewer = second;
    return answer(s32(fewer) < s32(at(app, kFramesTaken) + lead) ? 0 : 1);
}

}  // namespace

/* 0x4c6530: player's four bytes at `actions` queued for its next frame, if
 * the queue is not too far ahead (queueAhead) and that frame's slot is free;
 * 1 if they were, else 0.  The connection lost stops the queue; a slot not
 * free in a race over the network loses it.  The frames the race may run
 * counted by the mode: one each in one player's race; one for each of the
 * second player's on one screen; over the network, this machine's alone
 * queued here, one for each of its own; the host's side, one for each of
 * player 0's on a client; otherwise the fewer of the first two players'
 * frames, less one.  edx as the code leaves it. */
Answered queuePlayerInput(App* app, u32 player, u32 actions)
{
    const u32 count = player << 2;
    const u16 lost = half(app, kConnectionLost);
    const u32 index = at(app, count + kFramesQueued) & kFrameMask;
    if (lost != 0)
    {
        put(app, kQueueStopped, 1);
        return {0, 1};
    }
    if (queueAhead(app, player) != 0)
        return {0, actions};
    const u32 slot = slotOf(player, index);
    if (at(app, slot + kQueued) != 0)
    {
        if (s32(gameSettings(app).playMode) > 1)
        {
            app->getMemory<u16>(kConnectionLostToo) = 1;
            app->getMemory<u16>(kConnectionLost) = 1;
        }
        return {0, actions};
    }
    copyBytes(app, (player << kPlayerShift) + kQueue + index * 4, actions, kControlsBytes);
    put(app, slot + kQueued, 1);
    put(app, count + kFramesQueued, at(app, count + kFramesQueued) + 1);
    // The table at 0x4c6514: modes 0 and 1 their own, 2 to 4 the network's.
    const u32 mode = gameSettings(app).playMode;
    if (mode > kLastMode)
        return {1, actions};
    if (mode == kOneMode)
    {
        put(app, kRaceFrames, at(app, kRaceFrames) + 1);
        return {1, actions};
    }
    if (mode == kSplitMode)
    {
        if (player != 0)
            put(app, kRaceFrames, at(app, kRaceFrames) + 1);
        return {1, actions};
    }
    if (gameSettings(app).networkGame != 0 && player == gameSettings(app).localPlayer)
    {
        put(app, kRaceFrames, at(app, kRaceFrames) + 1);
        return {1, actions};
    }
    if (at(app, kNetworkKind) == kHostKind)
    {
        if (player == 0 && gameSettings(app).localPlayer != 0)
            put(app, kRaceFrames, at(app, kRaceFrames) + kHostKind);
        return {1, actions};
    }
    const u32 first = at(app, kFramesQueued);
    u32 fewer = at(app, kSecondQueued);
    if (s32(first) < s32(fewer))
        fewer = first;
    put(app, kRaceFrames, fewer - 1);
    return {1, first};
}

namespace
{

/* 0x4c66a0: a remote player's four bytes at `actions` queued on the host,
 * unless the queue is too far ahead or the slot is taken.  A player more
 * than its lead (0x7d3710) ahead of player 0 has a frame dropped first: its
 * count one back, its frames from player 0's on moved back by one.  Returns
 * eax as the code leaves it -- the answer that stopped it, else 4 -- and edx. */
Answered queueRemote(App* app, u32 player, u32 actions)
{
    const u32 count = player * 4;
    u32 queued = at(app, count + kFramesQueued);
    const u32 full = queueAhead(app, player);
    if (full != 0)
        return {full, count};
    const u32 taken = at(app, slotOf(player, queued & kFrameMask) + kQueued);
    if (taken != 0)
        return {taken, count};
    const u32 mine = at(app, count + kFramesQueued);
    if (s32(at(app, kFramesQueued) + at(app, kRemoteLead)) < s32(mine))
    {
        put(app, count + kFramesQueued, mine - 1);
        for (u32 frame = at(app, kFramesQueued);; ++frame)
        {
            queued = at(app, player * 4 + kFramesQueued);
            if (s32(frame) >= s32(queued))
                break;
            copyBytes(app, kQueue + slotOf(player, frame & kFrameMask),
                      kQueue + slotOf(player, (frame + 1) & kFrameMask), kControlsBytes);
        }
    }
    const u32 slot = slotOf(player, queued & kFrameMask);
    copyBytes(app, kQueue + slot, actions, kControlsBytes);
    put(app, slot + kQueued, 1);
    put(app, player * 4 + kFramesQueued, at(app, player * 4 + kFramesQueued) + 1);
    return {kControlsBytes, slot};
}

/* 0x4c67a0: frame `frame` taken from the queue, but in a replay: each
 * player's four bytes into 0x7d36f0 (only this machine's player's if only its
 * own are queued here), every player's slot freed, the frames taken counted.
 * Returns eax as the code leaves it: the players' count (0 for none), or
 * `unchanged` in a replay. */
u32 takeFrame(App* app, u32 frame, u32 unchanged)
{
    const u32 index = frame & kFrameMask;
    if (s32(gameSettings(app).replayMode) > 1)
        return unchanged;
    for (u32 player = 0; s32(player) < s32(gameSettings(app).racerCount); ++player)
    {
        if (gameSettings(app).networkGame != 0 && player != gameSettings(app).localPlayer)
            continue;
        copyBytes(app, kTaken + player * 4, kQueue + slotOf(player, index), kControlsBytes);
    }
    u32 player = 0;
    for (; s32(player) < s32(gameSettings(app).racerCount); ++player)
        put(app, slotOf(player, index) + kQueued, 0);
    put(app, kFramesTaken, at(app, kFramesTaken) + 1);
    return player;
}

/* 0x4c6870: player 0's last frame queued made every player's: a player
 * whose slot for it is free has it queued with the four bytes of its frame
 * before, the buttons that count once as pressed cleared (0x8d kept); each
 * player's four bytes for it copied to `records`, a dword each.  The frames
 * the race may run counted.  Returns eax as the code leaves it: 4, or
 * `unchanged` for no players. */
u32 repeatMissing(App* app, u32 records, u32 unchanged)
{
    u32 left = unchanged;
    const u32 last = (at(app, kFramesQueued) - 1) & kFrameMask;
    for (u32 player = 0; s32(player) < s32(gameSettings(app).racerCount); ++player)
    {
        const u32 slot = slotOf(player, last);
        if (at(app, slot + kQueued) == 0)
        {
            put(app, player * 4 + kFramesQueued, at(app, player * 4 + kFramesQueued) + 1);
            put(app, slot + kQueued, 1);
            const u32 before = (last != 0 ? last - 1 : kFrameMask) & kFrameMask;
            copyBytes(app, kQueue + slot, kQueue + slotOf(player, before), kControlsBytes);
            putByte(app, slot + kQueueButtons, byte(app, slot + kQueueButtons) & kRepeatedButtons);
        }
        copyBytes(app, records + player * 4, kQueue + slot, kControlsBytes);
        left = kControlsBytes;
    }
    put(app, kRaceFrames, at(app, kRaceFrames) + 1);
    return left;
}

/* 0x4c6970: the frame the machines are to meet at, sent over the network:
 * player 0's frames queued and two more; 0 on a client of a host's. */
u32 meetingFrame(App* app)
{
    if (gameSettings(app).localPlayer != 0 && at(app, kNetworkKind) != kTwoMachines)
        return 0;
    return at(app, kFramesQueued) + 2;
}

/* 0x4c69c0: the queue's lead sent with this machine's input, a byte: on two
 * machines (0x7a22e4 0) the other player's frames queued less this
 * machine's player's, else player 0's frames queued less those taken. */
u8 queueLeadByte(App* app)
{
    if (at(app, kNetworkKind) == kTwoMachines)
    {
        const u32 player = gameSettings(app).localPlayer;
        const u32 other = player == 0 ? 1 : 0;
        return u8(byte(app, other * 4 + kFramesQueued) - byte(app, player * 4 + kFramesQueued));
    }
    return u8(byte(app, kFramesQueued) - byte(app, kFramesTaken));
}

/* 0x4c6a10: a machine's lag noted: the byte at 0x7d5740 the greatest yet.
 * Returns the greater, as eax holds it. */
u32 noteMachineLag(App* app, u32 lag)
{
    const u32 most = byte(app, kLag);
    const u32 greater = s32(lag) <= s32(most) ? most : lag;
    putByte(app, kLag, u8(greater));
    return greater;
}

/* 0x4c6a40: the queue's delays by the race's mode -- the lead it may run
 * ahead of the frames taken (0x7d573c), the delay a remote player's may
 * (0x7d5738), the lead a remote player may have on player 0 (0x7d3710):
 * 4 ahead on one machine, two players 4 and 4; over the network 8 and 0x10
 * (mode 2), and in mode 4 by the network's kind: 8, 0x10 and 2; 0x20, 0x20
 * and 4; 0x10, 0x20 and 0x10.  Any other mode leaves them.  Returns the
 * mode, as the code leaves eax. */
u32 setDelays(App* app)
{
    u32 lead = at(app, kQueueLead);
    u32 delay = at(app, kRemoteDelay);
    u32 remoteLead = at(app, kRemoteLead);
    const u32 mode = gameSettings(app).playMode;
    // The table at 0x4c6a2c, by the mode.
    switch (mode)
    {
    case 0:
    case 1:
        lead = 4;
        break;
    case 2:
        delay = 0x10;
        lead = 8;
        break;
    case 3:
        lead = 4;
        delay = 4;
        break;
    case 4:
    {
        const u32 kind = at(app, kNetworkKind);
        if (kind == 1)
        {
            remoteLead = 2;
            delay = 0x10;
            lead = 8;
        }
        else if (kind == 2)
        {
            lead = 0x20;
            remoteLead = 4;
            delay = 0x20;
        }
        else
        {
            lead = 0x10;
            delay = 0x20;
            remoteLead = 0x10;
        }
        break;
    }
    default:
        break;
    }
    put(app, kRemoteLead, remoteLead);
    put(app, kRemoteDelay, delay);
    put(app, kQueueLead, lead);
    return mode;
}

}  // namespace

/* 0x4c60c0: this machine's four bytes at `actions` in a race over the
 * network.  The host of several (0x7a22e4 1, 0x7a22da set) queues them under
 * the network's lock and, unless only its own are queued here, sends every
 * player's for the frame -- those missing repeated -- as a packet of 6 and 32
 * bytes, built in its frame.  Otherwise they are queued (only this machine's
 * queued here: nothing more), and sent as a packet of 6, the queue's lead and
 * the four bytes -- a client of a host's sent even if they were not queued,
 * which it leaves to the host.  Returns eax as the code leaves it. */
std::optional<u32> sendInput(App* app, ThreadState& ts, u32 actions)
{
    // ebx, ecx, edx, esi, edi, ebp pushed; the packet at ebp - 0x24.
    const GuestFrame frame(ts, 0x18, 0x24);
    const u32 packet = frame - 0x24;
    if (at(app, kNetworkKind) == kHostKind && half(app, kNetworkHost) != 0)
    {
        const u32 lock = at(app, kNetworkLock);
        putByte(app, packet, kInputPacketType);
        if (!lockEnter(app, ts, lock, 0, 0))
            return std::nullopt;
        const u32 queued = queuePlayerInput(app, gameSettings(app).localPlayer, actions).answer;
        if (queued == 0 || gameSettings(app).networkGame != 0)
        {
            // Unlocked with the queue's answer in eax, which the lock's leave keeps.
            const auto unlocked = lockLeave(
                app, ts, at(app, kNetworkLock), queued, 0);
            if (!unlocked)
                return std::nullopt;
            return unlocked->answer;
        }
        repeatMissing(app, packet + 1, 0);
        if (!lockLeave(app, ts, at(app, kNetworkLock), 0,
                                                                                     0))
            return std::nullopt;
        const auto sent = guest::direct<0x4a60e0, ea::packetSent>(app, ts, 0u, packet, kAllInputsBytes);
        if (!sent)
            return std::nullopt;
        return sent->eax;
    }
    if (gameSettings(app).networkGame != 0)
        return queuePlayerInput(app, gameSettings(app).localPlayer, actions).answer;
    putByte(app, kInputPacketLead, queueLeadByte(app));
    if (at(app, kNetworkKind) == kTwoMachines
        && queuePlayerInput(app, gameSettings(app).localPlayer, actions).answer == 0)
        return 0;
    putByte(app, kInputPacket, kInputPacketType);
    copyBytes(app, kInputPacketControls, actions, kControlsBytes);
    const auto sent = guest::direct<0x4a60e0, ea::packetSent>(app, ts, 0u, kInputPacket, kInputPacketBytes);
    if (!sent)
        return std::nullopt;
    return sent->eax;
}

namespace
{

/* What receivePacket leaves in eax, ebx and edx, as the code leaves them;
 * no caller reads the last two. */
struct Received
{
    u32 answer;
    u32 ebxLeft;
    u32 edxLeft;
};

/* 0x4c61e0: a packet of 6 at `packet` from machine `machine`, while packets
 * are taken (0x560100).  A client of a host's queues every player's four
 * bytes from it, the last player's first; otherwise the sender's lag (its
 * second byte) is noted, and its four bytes queued for its player -- the
 * host's, under the network's lock, by the player the machine is (0x4a6650),
 * filling a gap; two machines', for the other player.  `left` is what the
 * registers held on entry; returns them as the code leaves them. */
std::optional<Received> receivePacket(App* app, ThreadState& ts, u32 machine, u32 packet, Received left)
{
    if (at(app, kTakingPackets) == 0)
        return left;
    if (half(app, kNetworkHost) == 0 && at(app, kNetworkKind) == kHostKind)
    {
        u32 player = gameSettings(app).racerCount;
        for (;;)
        {
            --player;
            if (s32(player) < 0)
                break;
            const Answered queued = queuePlayerInput(app, player, player * 4 + packet + 1);
            left.answer = queued.answer;
            left.edxLeft = queued.edxLeft;
        }
        left.ebxLeft = player;
        return left;
    }
    noteMachineLag(app, u32(s32(nfs::s8(byte(app, packet + 1)))));
    const u32 controls = packet + 2;
    if (at(app, kNetworkKind) == kHostKind)
    {
        if (!lockEnter(app, ts, at(app, kNetworkLock), 0,
                                                                                     0))
            return std::nullopt;
        // The player the machine is, a word.
        const u32 player = guest::direct<0x4a6650, ea::machineOf>(app, ts, machine);
        if (ts.terminate)
            return std::nullopt;
        const Answered queued = queueRemote(app, u32(s32(nfs::s16(u16(player)))), controls);
        // Unlocked with the queue's answer in eax and the lock in edx, which
        // the lock's leave keeps.
        const u32 lock = at(app, kNetworkLock);
        const auto unlocked = lockLeave(
            app, ts, lock, queued.answer, lock);
        if (!unlocked)
            return std::nullopt;
        return Received{unlocked->answer, controls, unlocked->edxLeft};
    }
    const Answered queued = queuePlayerInput(app, gameSettings(app).localPlayer == 0 ? 1 : 0, controls);
    return Received{queued.answer, controls, queued.edxLeft};
}

}  // namespace

nfs::u32 takeCarControls(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 stepMenuChoice(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 stepMenuChoiceOfThree(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 actionPressed(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 functionKeyPressed(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 pressFunctionKey(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 functionKeyRepeats(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 keyPressed(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 firstFunctionKeyHeld(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 appendToPacket(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 sendOwnInput(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 receiveInput(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 queueInput(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 queueRemoteInput(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 takeQueuedFrame(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 takenControls(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 repeatMissingInputs(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 syncFrame(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 framesTaken(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 framesQueued(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 queueLead(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 noteLag(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 setQueueDelays(App* app, ThreadState& ts, nfs::Args& args);
nfs::u32 queueFull(App* app, ThreadState& ts, nfs::Args& args);

/* 0x48f6a0: takeCarFrame for player eax. */
nfs::u32 takeCarControls(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = takeCarFrame(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

/* 0x48f6e0: stepChoice of choice al of edx; the code leaves the buttons in
 * ah, and edx less one when it steps up. */
nfs::u32 stepMenuChoice(App* app, ThreadState& ts, nfs::Args& args)
{
    const u8 buttons = byte(app, kCarButtons);
    args.eax = (args.eax & 0xffff0000) | (u32(buttons) << 8) | stepChoice(app, u8(args.eax), args.edx);
    if (buttons & kMenuUp)
        --args.edx;
    ts.sp += 4;
    return args.eax;
}

/* 0x48f710: stepChoiceOfThree of choice al; the buttons left in ah. */
nfs::u32 stepMenuChoiceOfThree(App* app, ThreadState& ts, nfs::Args& args)
{
    const u8 buttons = byte(app, kCarButtons);
    args.eax = (args.eax & 0xffff0000) | (u32(buttons) << 8) | stepChoiceOfThree(app, u8(args.eax));
    ts.sp += 4;
    return args.eax;
}

/* 0x48f740: whether action al (a bit of the actions' ring, 0x79c238) is held
 * at the screens' frame (edx 0), or was pressed in it (edx nonzero). */
nfs::u32 actionPressed(App* app, ThreadState& ts, nfs::Args& args)
{
    const u32 bit = args.eax;
    const bool edge = args.edx != 0;
    args.edx = edge ? maskOf(bit) & ringNow(app, kActionRing) : maskOf(bit);
    args.eax = ringBitPressed(app, kActionRing, bit, edge) ? 1 : 0;
    ts.sp += 4;
    return args.eax;
}

/* 0x48f7b0: whether function key al (a bit of their ring, 0x79c2b8) is held
 * at the screens' frame (edx 0), or was pressed in it (edx nonzero). */
nfs::u32 functionKeyPressed(App* app, ThreadState& ts, nfs::Args& args)
{
    const u32 bit = args.eax;
    const bool edge = args.edx != 0;
    args.edx = edge ? maskOf(bit) & ringNow(app, kFunctionKeyRing) : maskOf(bit);
    args.eax = ringBitPressed(app, kFunctionKeyRing, bit, edge) ? 1 : 0;
    ts.sp += 4;
    return args.eax;
}

/* 0x48f820: holdFunctionKey of bit al. */
nfs::u32 pressFunctionKey(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = holdFunctionKey(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

/* 0x48f860: functionKeyRepeat of bit al past edx frames. */
nfs::u32 functionKeyRepeats(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = functionKeyRepeat(app, args.eax, args.edx) ? 1 : 0;
    ts.sp += 4;
    return args.eax;
}

/* 0x48f8d0: keyDownOrPressed of key eax, an edge for edx nonzero. */
nfs::u32 keyPressed(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = keyDownOrPressed(app, args.eax, args.edx != 0) ? 1 : 0;
    ts.sp += 4;
    return args.eax;
}

/* 0x48f920: firstFunctionKey. */
nfs::u32 firstFunctionKeyHeld(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = firstFunctionKey(app);
    ts.sp += 4;
    return args.eax;
}

/* 0x4c6080: appendPacket of edx, ebx long; eax and edx left the bytes
 * appended, ebx the packet's start, as the code leaves them. */
nfs::u32 appendToPacket(App* app, ThreadState& ts, nfs::Args& args)
{
    const u32 start = at(app, kPacketStart);
    const u32 bytes = appendPacket(app, args.edx, args.ebx);
    args.edx = bytes;
    args.ebx = start;
    args.eax = bytes;
    ts.sp += 4;
    return args.eax;
}

/* 0x4c60c0: sendInput of the four bytes at eax. */
nfs::u32 sendOwnInput(App* app, ThreadState& ts, nfs::Args& args)
{
    if (const auto sent = sendInput(app, ts, args.eax))
    {
        args.eax = *sent;
        ts.sp += 4;
    }
    return args.eax;
}

/* 0x4c61e0: receivePacket of the packet at edx from machine eax; ebx and edx
 * as it leaves them. */
nfs::u32 receiveInput(App* app, ThreadState& ts, nfs::Args& args)
{
    const auto received = receivePacket(app, ts, args.eax, args.edx, {args.eax, args.ebx, args.edx});
    if (!received)
        return args.eax;
    args.ebx = received->ebxLeft;
    args.edx = received->edxLeft;
    args.eax = received->answer;
    ts.sp += 4;
    return args.eax;
}

/* 0x4c6530: queuePlayerInput of player eax's four bytes at edx; edx as it
 * leaves it. */
nfs::u32 queueInput(App* app, ThreadState& ts, nfs::Args& args)
{
    const Answered queued = queuePlayerInput(app, args.eax, args.edx);
    args.edx = queued.edxLeft;
    args.eax = queued.answer;
    ts.sp += 4;
    return args.eax;
}

/* 0x4c66a0: queueRemote of player eax's four bytes at edx; edx as it leaves
 * it. */
nfs::u32 queueRemoteInput(App* app, ThreadState& ts, nfs::Args& args)
{
    const Answered queued = queueRemote(app, args.eax, args.edx);
    args.edx = queued.edxLeft;
    args.eax = queued.answer;
    ts.sp += 4;
    return args.eax;
}

/* 0x4c67a0: takeFrame of frame eax. */
nfs::u32 takeQueuedFrame(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = takeFrame(app, args.eax, args.eax);
    ts.sp += 4;
    return args.eax;
}

/* 0x4c6840: copyTaken of player eax to edx; eax left the bytes copied. */
nfs::u32 takenControls(App* app, ThreadState& ts, nfs::Args& args)
{
    copyTaken(app, args.eax, args.edx);
    args.eax = kControlsBytes;
    ts.sp += 4;
    return args.eax;
}

/* 0x4c6870: repeatMissing to the records at eax. */
nfs::u32 repeatMissingInputs(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = repeatMissing(app, args.eax, args.eax);
    ts.sp += 4;
    return args.eax;
}

/* 0x4c6970: meetingFrame. */
nfs::u32 syncFrame(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = meetingFrame(app);
    ts.sp += 4;
    return args.eax;
}

/* 0x4c69a0: the frames taken from the queue. */
nfs::u32 framesTaken(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = at(app, kFramesTaken);
    ts.sp += 4;
    return args.eax;
}

/* 0x4c69b0: player 0's frames queued. */
nfs::u32 framesQueued(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = at(app, kFramesQueued);
    ts.sp += 4;
    return args.eax;
}

/* 0x4c69c0: queueLeadByte in al; the rest of eax 0 on two machines, else as
 * it was, as the code leaves it. */
nfs::u32 queueLead(App* app, ThreadState& ts, nfs::Args& args)
{
    const u32 lead = queueLeadByte(app);
    args.eax = at(app, kNetworkKind) == kTwoMachines ? lead : (args.eax & 0xffffff00) | lead;
    ts.sp += 4;
    return args.eax;
}

/* 0x4c6a10: noteMachineLag of eax. */
nfs::u32 noteLag(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = noteMachineLag(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

/* 0x4c6a40: setDelays. */
nfs::u32 setQueueDelays(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = setDelays(app);
    ts.sp += 4;
    return args.eax;
}

/* 0x4c6ae0: queueAhead for player eax. */
nfs::u32 queueFull(App* app, ThreadState& ts, nfs::Args& args)
{
    args.eax = queueAhead(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

/* The typed cores of the thunks above, for callers that call them as C++ (native_controls_queue.h). */
u32 carControlsTaken(App* app, u32 player)
{
    return takeCarFrame(app, player);
}

u32 functionKeyHeld(App* app, u32 key, u32 edge)
{
    return ringBitPressed(app, kFunctionKeyRing, key, edge != 0) ? 1 : 0;
}

u32 functionKeyHeldDown(App* app, u32 key)
{
    return holdFunctionKey(app, key);
}

u32 keyDownNow(App* app, u32 key, u32 edge)
{
    return keyDownOrPressed(app, key, edge != 0) ? 1 : 0;
}

u32 queuedFrameTaken(App* app, u32 frame)
{
    return takeFrame(app, frame, frame);
}

u32 controlsCopied(App* app, u32 player, u32 to)
{
    copyTaken(app, player, to);
    return kControlsBytes;
}

u32 framesMet(App* app)
{
    return meetingFrame(app);
}

u32 framesTakenCount(App* app)
{
    return at(app, kFramesTaken);
}

u32 queueDelaysSet(App* app)
{
    return setDelays(app);
}

}  // namespace controls
}  // namespace nfs3hp
