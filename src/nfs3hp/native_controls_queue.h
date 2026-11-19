#ifndef NFS3HP_NATIVE_CONTROLS_QUEUE_H
#define NFS3HP_NATIVE_CONTROLS_QUEUE_H

#include "native_guest.h"

/* native_controls_queue.cpp's typed functions that other files call as C++ (guest::direct), and how
 * the game's code calls each by address.  Each may call the
 * game's other functions by address on `ts`, and is false or nullopt when the game stopped in one. */
namespace nfs3hp
{
namespace controls
{

/* An answer, and what the code leaves in edx with it -- no caller reads it. */
struct Answered
{
    nfs::u32 answer;
    nfs::u32 edxLeft;
};

// The game's 0x48f6e0: choice `choice` of `count` stepped by the first player's buttons.
nfs::u8 stepChoice(nfs::App* app, nfs::u8 choice, nfs::u32 count);
// The game's 0x48f710: choice `choice` of three stepped by the first player's buttons.
nfs::u8 stepChoiceOfThree(nfs::App* app, nfs::u8 choice);
// The game's 0x4c60c0: this machine's four bytes at `actions` in a race over the network, queued and
// sent; eax as the code leaves it.
std::optional<nfs::u32> sendInput(nfs::App* app, nfs::ThreadState& ts, nfs::u32 actions);
// The game's 0x4c6530: player's four bytes at `actions` queued for its next frame: 1 if they were,
// else 0.
Answered queuePlayerInput(nfs::App* app, nfs::u32 player, nfs::u32 actions);

// The game's 0x48f6a0: player's frame of the race's controls taken; eax as the code leaves it.
nfs::u32 carControlsTaken(nfs::App* app, nfs::u32 player);
// The game's 0x48f7b0: whether function key `key` is held (`edge` 0) or was pressed this frame (`edge` nonzero).
nfs::u32 functionKeyHeld(nfs::App* app, nfs::u32 key, nfs::u32 edge);
// The game's 0x48f820: function key `key` held; eax as the code leaves it.
nfs::u32 functionKeyHeldDown(nfs::App* app, nfs::u32 key);
// The game's 0x48f8d0: whether key `key` is down (`edge` 0) or was pressed this frame (`edge` nonzero).
nfs::u32 keyDownNow(nfs::App* app, nfs::u32 key, nfs::u32 edge);
// The game's 0x4c67a0: frame `frame` taken from the queue; eax as the code leaves it.
nfs::u32 queuedFrameTaken(nfs::App* app, nfs::u32 frame);
// The game's 0x4c6840: player's taken controls copied to `to`; the bytes copied.
nfs::u32 controlsCopied(nfs::App* app, nfs::u32 player, nfs::u32 to);
// The game's 0x4c6970: the frame the players meet at.
nfs::u32 framesMet(nfs::App* app);
// The game's 0x4c69a0: how many frames were taken from the queue.
nfs::u32 framesTakenCount(nfs::App* app);
// The game's 0x4c6a40: the queue's delays set; eax as the code leaves it.
nfs::u32 queueDelaysSet(nfs::App* app);

}  // namespace controls
}  // namespace nfs3hp

#endif
