#include "native_controls_queue.h"
#include "native_controls_race.h"
#include "native_frontend_inpa.h"
#include "native_track.h"
#include "native_carstate.h"
#include "game_settings.h"
#include "native_controls_input.h"
#include "native_ea_net.h"
#include "native_ea_threads.h"
#include "native_ea_formats.h"
#include "port_sound_music.h"
#include "native_frontend.h"
#include "native_frontend_fx.h"
#include "native_frontend_connect.h"
#include "native_frontend_eaa.h"
#include "native_frontend_eab.h"
#include "native_game.h"
#include "native_frontend_hudr.h"
#include "native_frontend_hudx.h"
#include "native_frontend_loada.h"
#include "native_frontend_loopa.h"
#include "native_frontend_loopb.h"
#include "native_frontend_menua.h"
#include "native_frontend_neta.h"
#include "native_frontend_sima.h"
#include "native_frontend_simc.h"
#include "native_frontend_simd.h"
#include "native_frontend_sime.h"
#include "native_frontend_tail.h"
#include "native_frontend_wina.h"
#include "native_raceload.h"
#include "port_snd_stream_calls.h"
#include "native_graphics.h"
// What lib/thread.h holds pointers to, declared here.
struct SDL_AtomicInt;
struct SDL_Semaphore;
#include <lib/thread.h>
#include <nfs3hp.h>
#include <lib/app.h>
#include <optional>

/* The race's main loop and what hangs off it, decompiled:
 *
 *   the loop         sub_4c4e40: every pass of the race: the frames' keys against the
 *                    clock, the simulation's frames that are due, what is drawn;
 *   a frame's keys   sub_4c4be0 while the pause menu is up, sub_4c4ac0 the developer's
 *                    freeze and step keys under it;
 *   a network race   sub_4c55a0: the race held while the machines agree on the frame
 *                    to pick up from;
 *   the set-up       sub_4c62a0: what a race loads, a piece at a time, each a place to
 *                    be told to give up; the controls' queues emptied (sub_4c63f0) and
 *                    their locks made and freed (sub_4c63c0, sub_4c64e0, sub_4c5520,
 *                    sub_4c5560, sub_4c7b30, sub_4c7bb0).
 *
 * The state stays in guest memory and what these call is called by address.  Replays
 * must stay identical to the bit, so a few registers are kept as the game's code
 * keeps them: esi and edi where a call in between may change them (the sound's do, and
 * the loop goes on with what they leave), and eax where a callee or the return reads
 * what is left in it (sub_4c4490, sub_4c5720, the end of the loop).
 *
 * Plan step 5: each function a typed core in frontend::inpa (native_frontend_inpa.h), in a frame
 * of its own (fe::Frame: what it calls runs at the stack addresses it did), and a thunk under
 * the game's name for the calls by address.  A core of this file calls another, and
 * native_frontend_tail.cpp's, as C++ (guest::direct), and the typed cores of other files too
 * (fe::direct); the rest of the game still by address.  The pause menu's turn (sub_4c4be0) stays
 * by address from the loop: it leaves edi as the sound's calls in it leave it, and the loop goes on
 * with edi. */
namespace nfs3hp
{

namespace
{

using fe::args;
using fe::Guest;
using nfs::u32;
using nfs::s32;
using App = nfs::App;
using ThreadState = nfs::ThreadState;

// ---------------------------------------------------------------------------
// The race's state
// ---------------------------------------------------------------------------

constexpr u32 kTicks = 0x5efdf8;            // the system's clock
constexpr u32 kMenuTicks = 0x5efdfc;        // the clock the pause menu is timed by
constexpr u32 kInputFrame = 0x79c4ac;       // the frame the input has reached
constexpr u32 kNextKeysFrame = 0x7d3688;    // the next frame whose keys are read
constexpr u32 kLeaving = 0x7d369c;          // nonzero: the loop ends
constexpr u32 kRestart = 0x7d3698;          // the pause menu asked for the race again
constexpr u32 kPaused = 0x7d36a0;           // the pause menu is up
constexpr u32 kPauseKeysFrom = 0x7d36a4;    // kMenuTicks from when it takes its keys
constexpr u32 kPanelPausedSeen = 0x7d36a8;  // kPanelPaused as last acted on
constexpr u32 kHalfFramesDue = 0x7d36ac;    // how far the simulation may run
constexpr u32 kHalfFrame = 0x7d36b0;        // how far it has
constexpr u32 kEnding = 0x7d36b4;           // how the race was left
constexpr u32 kFramesQueued = 0x560104;     // the frames the race may run
constexpr u32 kQueueStopped = 0x560108;
constexpr u32 kQueueStarting = 0x56010c;
constexpr u32 kSkipDraw = 0x7d3674;         // nonzero: this pass draws nothing
constexpr u32 kNetworkLeaving = 0x7a22dc;   // int16: the game is over for this machine
constexpr u32 kNetworkHost = 0x7a22da;      // int16
constexpr u32 kNetworkPeers = 0x7a22e8;
constexpr u32 kPanelState = 0x7cda58;       // 2 the replay's panel, 4 going into it
constexpr u32 kPanelPaused = 0x7cd948;
constexpr u32 kPanelSpeed = 0x7cd94c;       // 0 to 3
constexpr u32 kPanelCommand = 0x7cd95c;     // 4: what its keys asked is to be acted on
constexpr u32 kCamerasOnly = 0x5600ec;      // 1: this replay frame only moves the cameras
constexpr u32 kMouseClick = 0x55e5ec;       // the buttons, dropped when the mouse moves
constexpr u32 kMouseReleased = 0x55e5ed;    // byte, of them; 2: a click says whom the views follow
constexpr u32 kCommandFlags = 0x55eb2e;     // byte; 0x20: the pause menu over a race that ends
constexpr u32 kMouseX = 0x7d367c, kMouseY = 0x7d3678;
constexpr u32 kLastMouseX = 0x7d36b8, kLastMouseY = 0x7d36bc;
constexpr u32 kSplitLine = 0x7cdb2c;        // from this y down, the second player's half
constexpr u32 kLookBack = 0x79c4a0;         // per player
constexpr u32 kCars = 0x5efac8;             // per player, the car's record
constexpr u32 kCarFlags = 0x200;            // byte; 1: its state counts
constexpr u32 kCarState = 0x2c4;            // 3 and 4: the views do not go to it

// The developer's keys.
constexpr u32 kFrozen = 0x5600fc;
constexpr u32 kSingleStep = 0x55301c, kStepDirection = 0x553020, kStepSaved = 0x7d3670;

// A network race's hold.
constexpr u32 kHoldAsked = 0x7d36c0, kHoldTimeout = 0x7d36c4, kHoldUntil = 0x7d36c8, kHoldLast = 0x7d36cc;
constexpr u32 kHoldBytes = 0x10;            // the four of them
constexpr u32 kHoldEvery = 0x20;            // frames between two
constexpr u32 kHoldPacketBytes = 0xc;
constexpr std::uint8_t kHoldPacketKind = 8;
constexpr u32 kNetworkLock = 0x7d36d8, kNetworkLockMade = 0x560100;

// The controls' queues (native_controls_queue.cpp reads them).
constexpr u32 kQueueLock = 0x560114;
constexpr u32 kQueuePlayers = 8, kPlayerBytes = 0x200, kQueueFrames = 0x80;
constexpr u32 kQueue = 0x7d3714, kQueued = 0x7d4714, kQueueLeads = 0x7d5714, kQueueLag = 0x7d5740;
constexpr u32 kFramesReadyFirst = 4;        // each player's first, ready and empty
constexpr u32 kFramesToRunFirst = 3;

constexpr u32 kCancelled = 0x55e88c;        // the race is given up

// ---------------------------------------------------------------------------
// What they call
// ---------------------------------------------------------------------------

constexpr u32 kRunSystemTasks = 0x4e7630, kExitGame = 0x4f2a7d;
constexpr u32 kSilenceForMenus = 0x4186a0, kSilenceForRace = 0x4186f0, kMasterSilent = 0x418610, kMasterBack = 0x418620;
constexpr u32 kMusicMuteToggle = 0x4187e0, kSwapRaceVolumes = 0x418840, kSlotsStop = 0x4187a0;
constexpr u32 kSettleCamera = 0x423b80, kAttachCameras = 0x4215c0;
constexpr u32 kFrameHeld = 0x4c4de0, kPauseMenuStep = 0x4c4be0;
constexpr u32 kNextViewDistance = 0x41d870;
constexpr u32 kBeginDrawing = 0x4d5cc0;
constexpr u32 kCaptureScreen = 0x4c4490;
constexpr u32 kKeyFromHook = 0x4f0100;
constexpr u32 kFillZero = 0x4e070c, kRestoreConnection = 0x4f5280;
constexpr u32 kMusicStreamEnd = 0x410aa0, kMusicStreamMake = 0x410370;
constexpr u32 kStartControlsTick = 0x42dcb0, kRaceBanksLoad = 0x413bc0;
constexpr u32 kSetQueueDelays = 0x4c6a40;
constexpr u32 kShowWaitMessage = 0x4c5c00;

// The function keys (kFunctionKey's eax; its edx 1: only as it goes down), and scan codes.
constexpr u32 kKeyFreeze = 1;
constexpr u32 kKeyNextCamera = 7, kKeyLastCamera = 8, kKeyViewDistance = 9, kKeyRearView = 0xb;
constexpr u32 kKeyHud0 = 0xc, kKeyHud1 = 0xd, kKeyMapPanes = 0xe, kKeyZoom0 = 0xf, kKeyZoom1 = 0x10;
constexpr u32 kKeySwapVolumes = 0x12, kKeyMute = 0x14, kKeyLookBack = 0x15, kKeySettleCamera = 0x16;
constexpr u32 kKeyLookBack2 = 0x17, kKeySettleCamera2 = 0x18;
constexpr u32 kScanEnter = 0x1c, kScanF1 = 0x3b, kScanF2 = 0x3c, kScanF3 = 0x3d, kScanF4 = 0x3e;
constexpr u32 kScanAlt = 0x38, kScanP = 0x19;
constexpr u32 kFlushAll = 0x1b;             // what the keys' flush (sub_451940) is always handed here

/* Another thread's turn with the game, where the game's code offers it. */
void safepoint(nfs::App* app, nfs::ThreadState& ts)
{
    if (app->contextWanted())
        app->yieldContext(ts);
}

/* The calls these make as C++ leave eax as the code's calls by address do: their result, which
 * the code goes on with (the screen's capture takes what the pass left in it).  The game's code
 * hands sub_426680 a 0 in eax, which it does not read: there all the same, as the call by address
 * has it. */

/* The keys waiting thrown away (native_frontend_connect.cpp's). */
/* A call by address that leaves eax in the caller's value, as the code's registers do. */
u32 call(Guest& guest, u32& eax, u32 address, fe::Arguments arguments = fe::Arguments())
{
    // eax goes in where the call hands none: the callee finds what the code before left there.
    if (arguments.registerCount == 0)
    {
        arguments.registers[0] = eax;
        arguments.registerCount = 1;
    }
    return eax = fx::callAt(guest, address, arguments);
}

/* The function keys' and keys' cores as C++: eax the result, as call() leaves it. */
u32 functionKey(Guest& guest, u32& eax, u32 key, u32 edge)
{
    return eax = fe::direct<0x48f7b0, controls::functionKeyHeld>(guest.app(), guest.ts(), key, edge);
}

u32 keyPressed(Guest& guest, u32& eax, u32 key, u32 edge)
{
    return eax = fe::direct<0x48f8d0, controls::keyDownNow>(guest.app(), guest.ts(), key, edge);
}

u32 framesTaken(Guest& guest, u32& eax)
{
    return eax = fe::direct<0x4c69a0, controls::framesTakenCount>(guest.app(), guest.ts());
}

u32 frameSynced(Guest& guest, u32& eax)
{
    return eax = fe::direct<0x4c6970, controls::framesMet>(guest.app(), guest.ts());
}

void flushKeys(Guest& guest, nfs::ThreadState& ts, u32& eax)
{
    eax = fe::direct<0x451940, frontend::connect::flushKeys>(guest.app(), ts, kFlushAll);
}

/* The tasks due run (port_snd_stream.cpp's), with nothing. */
void runSystemTasks(Guest& guest, nfs::ThreadState& ts, u32& eax)
{
    eax = u32(*fe::direct<0x4e7630, port::stream::runDueTasks>(guest.app(), ts, ts.sp - 4, u32(0)));
}

/* The replay panel's keys, and the pause it has of its own (0x4c4f21 to 0x4c4f9b). */
void panelKeys(Guest& guest, nfs::ThreadState& ts, u32& eax)
{
    if (guest.dword(kPanelState) != 2 || guest.dword(kPaused) != 0)
        return;
    eax = fe::direct<0x4bd640, frontend::simc::panelKeys>(guest.app(), ts);
    if (guest.dword(kPanelCommand) != 4)
        return;
    const u32 leaving = guest.dword(kLeaving);
    guest.setDword(kPanelCommand, 0);
    if (leaving != 0 && guest.dword(kPanelPausedSeen) != 0)
    {
        guest.setDword(kPanelPausedSeen, 0);
        call(guest, eax, kSilenceForMenus);
        return;
    }
    const u32 panelPaused = guest.dword(kPanelPaused);
    if (panelPaused == guest.dword(kPanelPausedSeen))
        return;
    guest.setDword(kPanelPausedSeen, panelPaused);
    if (panelPaused != 0)
    {
        eax = fe::direct<0x426680, frontend::sima::camerasStep>(guest.app(), ts);
        call(guest, eax, kMasterSilent);
    }
    else
        call(guest, eax, kMasterBack);
}

/* The mouse (0x4c5158 to 0x4c5243): a move drops its click, and a click left standing
 * turns the views: over a split screen to the half it is in, otherwise to the player shown. */
void mouseStep(Guest& guest, nfs::ThreadState& ts, u32& eax)
{
    // native_controls_input.cpp's: the buttons and the wheel nowhere, the pointer to its two words.
    eax = *fe::direct<0x435310, controls::mouseRead>(guest.app(), ts, u32(0), kMouseX, kMouseY, u32(0));
    eax = guest.dword(kMouseX);
    if (eax != guest.dword(kLastMouseX) || guest.dword(kLastMouseY) != guest.dword(kMouseY))
    {
        guest.setDword(kLastMouseX, eax);
        eax = guest.dword(kMouseY);
        guest.setDword(kMouseClick, 0);
        guest.setDword(kLastMouseY, eax);
    }
    u32 half = 0;
    u32 player;
    if (gameSettings(guest).playMode == 1)
    {
        if (s32(guest.dword(kSplitLine)) > s32(guest.dword(kMouseY)))
            player = 0;
        else
        {
            half = 1;
            player = 1;
        }
    }
    else
        player = gameSettings(guest).localPlayer;
    if (player == gameSettings(guest).viewedPlayer || (guest.byte(kMouseReleased) & 2) == 0)
        return;
    const auto carIn = [&](u32 state) {
        eax = guest.dword(kCars + player * 4);
        return (guest.byte(eax + kCarFlags) & 1) != 0 && guest.dword(eax + kCarState) == state;
    };
    if (carIn(3) || carIn(4))
        return;
    gameSettings(guest).viewedPlayer = player;
    guest.setDword(kMouseClick, 0);
    fe::direct<kAttachCameras, carstate::camerasAttached>(guest.app(), ts, half, player);
    eax = fe::direct<0x485640, frontend::hudx::rebuildView>(guest.app(), ts, half);
}

/* One input frame's keys (0x4c4f13 to 0x4c5276).  edi takes the capture key. */
void frameKeys(Guest& guest, nfs::ThreadState& ts, u32& eax, u32& edi)
{
    if (s32(gameSettings(guest).playMode) < 2)
        fe::direct<0x4bc820, frontend::loopb::nothing>(guest.app(), ts);  // eax stays as it is
    panelKeys(guest, ts, eax);

    if (guest.dword(kLeaving) == 0)
    {
        // It leaves edi as the sound's calls in it leave it (the loop's edi is its own).
        if (guest.dword(kPaused) != 0)
            eax = fe::direct<kPauseMenuStep, frontend::inpa::pauseMenuStep>(guest.app(), ts);
        else
            eax = fe::direct<kFrameHeld, frontend::loopb::asked>(guest.app(), ts, 1u);
    }

    if (functionKey(guest, eax, kKeyNextCamera, 0) != 0)
        eax = fe::direct<0x4be970, frontend::sime::viewsResize>(guest.app(), ts, 1);
    if (functionKey(guest, eax, kKeyLastCamera, 0) != 0)
        eax = fe::direct<0x4be970, frontend::sime::viewsResize>(guest.app(), ts, u32(-1));

    if (guest.dword(kPaused) != 0)
    {
        if (s32(gameSettings(guest).playMode) <= 1)
            eax = guest::direct<0x4c4ac0, frontend::inpa::raceDebugKeys>(guest.app(), ts);
    }
    else
    {
        if (functionKey(guest, eax, kKeyViewDistance, 1) != 0)
            fe::direct<kNextViewDistance, track::viewDistanceOnward>(guest.app(), ts);
        if (functionKey(guest, eax, kKeyHud0, 1) != 0)
            eax = fe::direct<0x484260, frontend::hudx::toggleShown>(guest.app(), ts, 0);
        if (functionKey(guest, eax, kKeyHud1, 1) != 0)
            eax = fe::direct<0x484260, frontend::hudx::toggleShown>(guest.app(), ts, 1);
        if (functionKey(guest, eax, kKeyMapPanes, 1) != 0)
            eax = fe::direct<0x48e920, frontend::hudr::stepMapPanes>(guest.app(), ts);
        if (functionKey(guest, eax, kKeyZoom0, 0) != 0)
            eax = fe::direct<0x48eb60, frontend::hudx::stepZoom>(guest.app(), ts, 0);
        if (functionKey(guest, eax, kKeyZoom1, 0) != 0)
            eax = fe::direct<0x48eb60, frontend::hudx::stepZoom>(guest.app(), ts, 1);
        if (functionKey(guest, eax, kKeyRearView, 1) != 0)
            eax = fe::direct<0x484450, frontend::hudx::toggleRearView>(guest.app(), ts);
        if (functionKey(guest, eax, kKeyMute, 1) != 0)
            call(guest, eax, kMusicMuteToggle);
        if (functionKey(guest, eax, kKeySwapVolumes, 1) != 0)
            call(guest, eax, kSwapRaceVolumes);
        if (functionKey(guest, eax, kKeySettleCamera, 1) != 0 && s32(guest.dword(kPanelState)) < 2)
            fe::direct<kSettleCamera, carstate::cameraSettled>(guest.app(), ts, 0);
        if (gameSettings(guest).playMode == 1 && functionKey(guest, eax, kKeySettleCamera2, 1) != 0
            && s32(guest.dword(kPanelState)) < 2)
            fe::direct<kSettleCamera, carstate::cameraSettled>(guest.app(), ts, 1);
        guest.setDword(kLookBack, functionKey(guest, eax, kKeyLookBack, 0));
        if (gameSettings(guest).playMode == 1)
            guest.setDword(kLookBack + 4, functionKey(guest, eax, kKeyLookBack2, 0));
        mouseStep(guest, ts, eax);
    }

    // Alt+P: a picture of the screen at the end of the pass, for a game that is not
    // a network's or that is a replay.
    if (s32(gameSettings(guest).playMode) < 2 || s32(gameSettings(guest).replayMode) > 1)
    {
        if ((eax = fe::direct<0x4eb1c0, frontend::eaa::keyStateByte>(guest.app(), ts, kScanAlt)) != 0
            && (eax = fe::direct<0x4eb1c0, frontend::eaa::keyStateByte>(guest.app(), ts, kScanP)) != 0)
            edi = 1;
    }
}

/* A tick of the race (native_frontend_tail.cpp's).  The game's code hands it the half frame in
 * eax, which it does not read: there all the same, as the call by address has it. */
void runFrame(Guest& guest, nfs::ThreadState& ts, u32& eax, u32 halfFrame)
{
    eax = guest::direct<0x4c48d0, frontend::tail::raceFrame>(guest.app(), ts);
}

/* A frame as the replay's speed has it (0x4c530c to 0x4c5381): at the slowest every
 * fourth half frame runs, at half every second, at full each, at double two at a time;
 * between them only the cameras move. */
void replayFrame(Guest& guest, nfs::ThreadState& ts, u32& eax)
{
    const u32 speed = guest.dword(kPanelSpeed);
    const s32 halfFrame = s32(guest.dword(kHalfFrame));
    if ((speed == 0 && halfFrame % 4 == 0) || (speed == 1 && halfFrame % 2 == 0) || speed == 2)
        runFrame(guest, ts, eax, u32(halfFrame));
    else if (speed == 3)
    {
        runFrame(guest, ts, eax, u32(halfFrame));
        runFrame(guest, ts, eax, guest.dword(kHalfFrame));
    }
    else
    {
        guest.setDword(kCamerasOnly, 1);
        eax = fe::direct<0x426680, frontend::sima::camerasStep>(guest.app(), ts);
    }
}

/* The half frames that are due, run (0x4c52a8 on, to the jump to 0x4c546e). */
void runFrames(nfs::App* app, Guest& guest, nfs::ThreadState& ts, u32& eax)
{
    if (s32(guest.dword(kHalfFramesDue)) < s32(guest.dword(kHalfFrame)))
        guest.setDword(kSkipDraw, 1);

    for (;;)
    {
        safepoint(app, ts);
        if (s32(guest.dword(kHalfFramesDue)) < s32(guest.dword(kHalfFrame)))
            return;
        eax = fe::direct<kFrameHeld, frontend::loopb::asked>(guest.app(), ts, 0u);
        if (eax != 0)
            return;
        if ((guest.byte(kHalfFrame) & 1) == 0)
            eax = fe::direct<0x4c67a0, controls::queuedFrameTaken>(guest.app(), guest.ts(), u32(s32(guest.dword(kHalfFrame)) >> 1));

        const u32 esi = 0;
        const u32 panel = guest.dword(kPanelState);
        guest.setDword(kCamerasOnly, 0);
        if (panel == 2)
            replayFrame(guest, ts, eax);
        else if (panel == 4)
        {
            // Into the replay: its frames count from nothing, and this pass draws none.
            call(guest, eax, kSlotsStop);
            eax = fe::direct<0x4a33b0, frontend::loopa::raceRestart>(app, ts);
            guest.setDword(kFramesQueued, esi);
            guest.setDword(kPanelState, 2);
            guest.setDword(kSkipDraw, 1);
            return;
        }
        else
        {
            eax = fe::direct<0x4c81c0, frontend::neta::takeInputsOnTick>(app, ts, guest.dword(kHalfFrame));
            runFrame(guest, ts, eax, guest.dword(kHalfFrame));
            if ((guest.byte(kCommandFlags) & 0x20) != 0)
            {
                eax = guest.dword(kPaused);
                if (eax == 0 && guest.dword(kLeaving) != 0 && guest.dword(kEnding) == 0)
                {
                    // The race over and the flag on the command line: the pause menu over it.
                    fe::direct<kBeginDrawing, graphics::drawingBegun>(guest.app(), ts, 0u, 0u);
                    eax = 0;
                    guest.setDword(kPauseKeysFrom, guest.dword(kMenuTicks) + 0x10);
                    call(guest, eax, kMasterSilent);
                    guest.setDword(kQueueStopped, 1);
                    guest.setDword(kPaused, 1);
                    guest.setDword(kLeaving, esi);  // as the sound's call left it
                    eax = *fe::direct<0x470f40, controls::forcesStopped>(guest.app(), guest.ts(), eax);
                    flushKeys(guest, ts, eax);
                }
            }
            if ((guest.byte(kHalfFrame) & 0x1f) == 0)
                eax = fe::direct<0x4c7bd0, frontend::simd::netSendPlace>(app, ts, guest.dword(kHalfFrame));
        }
        guest.setDword(kHalfFrame, guest.dword(kHalfFrame) + 1);
    }
}

}

/* sub_4c4ac0: the developer's keys while paused, on one screen or a split one: the
 * freeze key or F1 freezes and frees the race, F3 single steps, F2 and F4 choose the way. */
nfs::u32 frontend::inpa::raceDebugKeys(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    u32 eax = 0;
    const fe::Frame frame(guest, ts, 12);
    bool toggle = false;
    if (guest.dword(kFrozen) != 0
        && (functionKey(guest, eax, kKeyFreeze, 1) != 0 || keyPressed(guest, eax, kScanF1, 1) != 0))
    {
        flushKeys(guest, ts, eax);
        toggle = true;
    }
    else if (guest.dword(kFrozen) == 0 && keyPressed(guest, eax, kScanF1, 1) != 0)
        toggle = true;
    if (toggle)
        guest.setDword(kFrozen, guest.dword(kFrozen) == 0 ? 1 : 0);

    if (keyPressed(guest, eax, kScanF3, 1) != 0)
    {
        const u32 stepping = guest.dword(kSingleStep) == 0 ? 1 : 0;
        guest.setDword(kSingleStep, stepping);
        if (stepping != 0)
            guest.setDword(kStepSaved, guest.dword(kStepDirection));
        else
            guest.setDword(kStepDirection, guest.dword(kStepSaved));
    }

    if (keyPressed(guest, eax, kScanF2, 0) != 0)
    {
        guest.setDword(kStepSaved, u32(-1));
        guest.setDword(kStepDirection, u32(-1));
        return u32(-1);
    }
    if (keyPressed(guest, eax, kScanF4, 0) != 0)
    {
        guest.setDword(kStepSaved, 1);
        guest.setDword(kStepDirection, 1);
    }
    else if (guest.dword(kSingleStep) != 0)
        guest.setDword(kStepDirection, 0);
    return eax;
}

/* sub_4c4be0: the pause menu's turn in a frame, once its time has come and the race is
 * not frozen: its answer 1 goes back to the race, 2 restarts it, 3, 4 and 5 leave it. */
nfs::u32 frontend::inpa::pauseMenuStep(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    u32 eax = 0;
    const fe::Frame frame(guest, ts, 20);
    if (s32(guest.dword(kMenuTicks)) < s32(guest.dword(kPauseKeysFrom)) || guest.dword(kFrozen) != 0)
        return eax;
    // A pass of the pause menu (native_frontend_menua.cpp's); what it leaves in eax goes in to its answer.
    const u32 shown = fe::direct<0x44bac0, frontend::menua::racePauseMenuFrame>(app, ts);
    const u32 answer = eax = fe::direct<0x4c5720, frontend::neta::raceQuitVote>(app, ts, shown);
    const auto leave = [&]() {
        fe::direct<0x4d5d10, frontend::loopb::clearDrawnFlag>(app, ts);  // eax stays as it is
        call(guest, eax, kSilenceForMenus);
        guest.setDword(kPaused, 0);
        guest.setDword(kRestart, 0);
        guest.setDword(kLeaving, 1);
        eax = *fe::direct<0x470fc0, controls::forcesStopped>(guest.app(), guest.ts(), eax);
        eax = fe::direct<0x4ce750, frontend::loopb::closeRace>(app, ts, 1);
    };
    switch (answer)
    {
    case 1:
        fe::direct<0x4d5d10, frontend::loopb::clearDrawnFlag>(app, ts);  // eax stays as it is
        if (guest.dword(kPanelPausedSeen) == 0)
            call(guest, eax, kMasterBack);
        guest.setDword(kQueueStopped, 0);
        guest.setDword(kPaused, 0);
        eax = *fe::direct<0x470f80, controls::forcesStopped>(guest.app(), guest.ts(), eax);
        eax = fe::direct<0x48f820, controls::functionKeyHeldDown>(guest.app(), guest.ts(), kKeyFreeze);
        keyPressed(guest, eax, kScanEnter, 1);
        flushKeys(guest, ts, eax);
        break;
    case 2:
        fe::direct<0x4d5d10, frontend::loopb::clearDrawnFlag>(app, ts);  // eax stays as it is
        call(guest, eax, kSilenceForRace);
        guest.setDword(kPaused, 0);
        guest.setDword(kLeaving, 1);
        guest.setDword(kRestart, 1);
        eax = *fe::direct<0x470fc0, controls::forcesStopped>(guest.app(), guest.ts(), eax);
        keyPressed(guest, eax, kScanEnter, 1);
        break;
    case 3:
        leave();
        if (gameSettings(guest).replayMode == 0)
            guest.setDword(kEnding, 0);
        break;
    case 4:
        leave();
        if (gameSettings(guest).replayMode == 0)
            guest.setDword(kEnding, 2);
        break;
    case 5:
        leave();
        guest.setDword(kEnding, 1);
        break;
    default:
        break;
    }
    return eax;
}

/* sub_4c4e40: the race, every pass of it, until it is over. */
nfs::u32 frontend::inpa::runRaceLoop(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    u32 eax = 0;
    const fe::Frame frame(guest, ts, 32);
    u32 lastTicks = guest.dword(kTicks) + 10000;
    u32 edi = 0;
    // The tasks due, handed 0 in eax and edx; the code goes on with the edx it hands, which they leave.
    runSystemTasks(guest, ts, eax);
    u32 lastDue = 0;
    // It does not come back.
    if (s32(guest.dword(kTicks)) <= 0)
        eax = fe::direct<kExitGame, frontend::eab::exitGameNow>(guest.app(), ts);
    const u32 firstFrame = guest.dword(kInputFrame);
    guest.setDword(kNextKeysFrame, firstFrame);
    eax = fe::direct<0x4ead90, frontend::eaa::bitmapDrawToScreen>(app, ts);

    for (;;)
    {
        app->unlockContext(ts);
        nfs::Thread::sleep(0);
        app->lockContext(ts);
        safepoint(app, ts);
        if (guest.dword(kLeaving) != 0)
            break;

        // How far the simulation may run: the frames queued, and half a frame more
        // each pass the clock moved further than they did.
        runSystemTasks(guest, ts, eax);
        guest.setDword(kHalfFramesDue, guest.dword(kFramesQueued) * 2);
        const u32 ticks = guest.dword(kTicks);
        if (s32(ticks - lastTicks) > s32(guest.dword(kHalfFramesDue) - lastDue))
            guest.setDword(kHalfFramesDue, guest.dword(kHalfFramesDue) + 1);
        lastTicks = ticks;
        lastDue = guest.dword(kHalfFramesDue);

        // Each input frame's keys, those of at most 16 behind.
        guest.setDword(kSkipDraw, 0);
        eax = guest.dword(kInputFrame) - guest.dword(kNextKeysFrame);
        if (s32(eax) > 0x10)
        {
            eax = guest.dword(kInputFrame);
            guest.setDword(kNextKeysFrame, eax);
        }
        for (;;)
        {
            safepoint(app, ts);
            if (s32(guest.dword(kInputFrame)) < s32(guest.dword(kNextKeysFrame)) || guest.dword(kLeaving) != 0)
                break;
            frameKeys(guest, ts, eax, edi);
            guest.setDword(kNextKeysFrame, guest.dword(kNextKeysFrame) + 1);
        }

        if (guest.dword(kPaused) != 0 || guest.dword(kPanelPausedSeen) != 0 || guest.dword(kLeaving) != 0)
        {
            eax = fe::direct<0x426680, frontend::sima::camerasStep>(app, ts);
        }
        else
            runFrames(app, guest, ts, eax);

        if (guest.dword(kSkipDraw) == 0)
        {
            if (edi != 0)
            {
                fe::direct<kCaptureScreen, graphics::screenCaptured>(guest.app(), ts);  // eax as the pass left it stays
                edi = 0;
            }
            else
                eax = fe::direct<0x4bc680, frontend::loopb::pauseDraw>(app, ts, guest.dword(kPaused));
        }
        if (gameSettings(guest).playMode == 4 && s32(gameSettings(guest).replayMode) <= 1 && gameSettings(guest).localPlayer == 0)
            eax = fe::direct<0x49ec70, frontend::neta::dropGone>(app, ts);

        if (guest.word(kNetworkLeaving) != 0)
        {
            if (s32(gameSettings(guest).replayMode) <= 1)
            {
                guest.setDword(kQueueStarting, 1);
                guest.setDword(kQueueStopped, 1);
            }
            eax = fe::direct<0x4ce750, frontend::loopb::closeRace>(app, ts, 1);
            guest.setDword(kLeaving, 1);
        }
    }

    if (s32(gameSettings(guest).replayMode) > 1)
        return eax;
    guest.setDword(kQueueStarting, 1);
    guest.setDword(kQueueStopped, 1);
    return u32(1);
}

/* sub_4c5520: the network's lock made. */
nfs::u32 frontend::inpa::networkLockMake(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    const u32 lock = *fe::direct<0x4eb2e0, ea::newLock>(app, ts);
    guest.setDword(kNetworkLock, lock);
    guest.setDword(kNetworkLockMade, 1);
    return lock;
}

/* sub_4c5540: a network race's hold forgotten. */
nfs::u32 frontend::inpa::holdClear(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    u32 eax = 0;
    const fe::Frame frame(guest, ts, 8);
    eax = fe::direct<kFillZero, ea::clearBytes>(app, ts, kHoldAsked, kHoldBytes);
    guest.setDword(kHoldLast, kHoldEvery);
    return eax;
}

/* sub_4c5560: the network's lock freed at the race's end, and the connection put back
 * when the game is over for this machine or the race was left that way. */
nfs::u32 frontend::inpa::networkLockFree(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    fe::direct<0x4a6a10, frontend::neta::barrier>(app, ts, 0x78, 7);
    const u32 lock = guest.dword(kNetworkLock);
    guest.setDword(kNetworkLockMade, 0);
    u32 left = *fe::direct<0x4eb360, ea::lockFreed>(app, ts, lock);
    if (guest.word(kNetworkLeaving) != 0 || guest.dword(kEnding) == 1)
        left = *fe::direct<kRestoreConnection, ea::connectionsRestored>(app, ts, left);
    return left;
}

/* sub_4c55a0: a network race's hold, for the machines to agree on a frame to pick up
 * from.  With eax nonzero, kHoldEvery frames after the last, it is asked for: the host
 * says the frame, the others ask it.  1 once that frame's input has been taken, and
 * the race goes on; 0 meanwhile; eax back when not a network race. */
nfs::u32 frontend::inpa::networkHold(App* app, ThreadState& ts, u32 mayAsk)
{
    Guest guest(app, ts);
    u32 eax = 0;
    const fe::Frame frame(guest, ts, 36);
    const u32 mode = gameSettings(guest).playMode;
    if (mode == 0 || mode == 1 || s32(gameSettings(guest).replayMode) > 1)
        return mayAsk;

    if (mayAsk != 0 && guest.dword(kHoldUntil) == 0 && guest.dword(kHoldAsked) == 0)
    {
        const u32 next = guest.dword(kHoldLast) + kHoldEvery;
        if (s32(next) < s32(framesTaken(guest, eax)))
        {
            // The packet, in the frame: its kind, a word of nothing, the frame.
            constexpr u32 kPacketNothing = 4, kPacketFrame = 8;
            const u32 packet = guest.frame();
            guest.setDword(packet + kPacketNothing, 0);
            guest.setByte(packet, kHoldPacketKind);
            if (guest.word(kNetworkHost) == 0)
            {
                guest.setDword(packet + kPacketFrame, 0);
                guest.setDword(kHoldAsked, 1);
                fe::direct<0x4a60e0, ea::packetSent>(app, ts, kNetworkPeers, packet, kHoldPacketBytes);
            }
            else
            {
                fe::enterLock(guest, ts, guest.dword(kNetworkLock));
                const u32 frameNow = frameSynced(guest, eax);
                guest.setDword(kHoldUntil, frameNow);
                guest.setDword(packet + kPacketFrame, frameNow);
                guest.setDword(kHoldAsked, 1);
                fe::direct<0x4a60e0, ea::packetSent>(app, ts, u32(0), packet, kHoldPacketBytes);
                fe::leaveLock(guest, ts, guest.dword(kNetworkLock));
            }
        }
    }

    if (guest.dword(kHoldAsked) == 0 || guest.dword(kHoldUntil) == 0)
        return u32(0);
    const u32 taken = framesTaken(guest, eax);
    const u32 until = guest.dword(kHoldUntil);
    if (s32(taken) < s32(until))
        return u32(0);

    guest.setDword(kHoldLast, until);
    guest.setDword(kQueueStopped, 1);
    guest.setDword(kHoldTimeout, 0);
    fe::direct<0x4a6a10, frontend::neta::barrier>(app, ts, 0x1e, 8);
    eax = *fe::direct<0x42d800, raceload::messagesCleared>(guest.app(), guest.ts());
    guest.setDword(kHoldUntil, 0);
    guest.setDword(kHoldAsked, 0);
    for (;;)
    {
        safepoint(app, ts);
        eax = *fe::direct<kKeyFromHook, ea::hookedKey>(guest.app(), ts);
        if (eax == 0)
            break;
        runSystemTasks(guest, ts, eax);  // eax its result: the key hook's call takes what is left in it
    }
    return u32(1);
}

/* sub_4c62a0: what a race needs before its first frame, a piece at a time with the
 * system's messages looked at between them.  1 when the race is given up, as it is at
 * once when a network game is over for this machine. */
nfs::u32 frontend::inpa::raceSetUp(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    const u32 mode = gameSettings(guest).playMode;
    if (mode != 0 && mode != 1 && guest.word(kNetworkLeaving) != 0)
    {
        guest.setDword(kCancelled, 1);
        return u32(1);
    }
    const auto cancelled = [&]() {
        fe::direct<0x4a33d0, frontend::loada::stopCheck>(app, ts);
        return guest.dword(kCancelled) != 0;
    };
    // The stream's end hands ebx and ecx on to a call that reads neither.
    fe::direct<kMusicStreamEnd, port::musicStreamEndOf>(app, ts, ts.sp - 4, 0u, 0u, 0u);
    fe::direct<0x4a33d0, frontend::loada::stopCheck>(app, ts);
    const u32 stream = u32(*fe::direct<kMusicStreamMake, port::musicStreamOf>(app, ts, ts.sp - 4, 100u, 0u));
    if (cancelled())
        return u32(1);
    fe::direct<0x42dcb0, raceload::controlsTickStarted>(app, ts, stream);
    if (cancelled())
        return u32(1);
    fx::callAt(guest, kRaceBanksLoad);
    if (cancelled())
        return u32(1);
    fe::direct<0x4c45f0, frontend::simd::raceOpen>(app, ts);
    if (cancelled())
        return u32(1);
    fe::direct<0x4bc8e0, frontend::simd::raceLeaveTasks>(app, ts);
    if (cancelled())
        return u32(1);
    fe::direct<0x4fb060, frontend::wina::freeKeptBlock>(app, ts);
    if (cancelled())
        return u32(1);
    if (s32(gameSettings(guest).playMode) > 1)
        fe::direct<0x4c5c00, graphics::waitMessage>(app, ts);
    guest::direct<0x4c63c0, inputLocksMake>(app, ts);
    if (cancelled())
        return u32(1);
    return u32(0);
}

/* sub_4c63c0: the input's locks made, the queue held stopped; not for a replay. */
nfs::u32 frontend::inpa::inputLocksMake(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    if (s32(gameSettings(guest).replayMode) > 1)
        return u32(0);
    guest.setDword(kQueueStopped, 1);
    guest.setDword(kQueueStarting, 1);
    guest::direct<0x4c7b30, queueLockMake>(app, ts);
    guest::direct<0x4c5520, networkLockMake>(app, ts);
    return fe::direct<0x4c6a40, controls::queueDelaysSet>(guest.app(), guest.ts());
}

/* sub_4c63f0: the controls' queues back to their start: each player's first frames
 * ready and empty, the rest not, and the first frames allowed to run. */
nfs::u32 frontend::inpa::inputReset(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    if (s32(gameSettings(guest).replayMode) > 1)
    {
        guest.setDword(kFramesQueued, 0);
        return u32(0);
    }
    guest.setDword(kQueueStopped, 1);
    guest.setDword(kQueueStarting, 1);
    fe::direct<0x4a6a10, frontend::neta::barrier>(app, ts, 0x5a, 5);
    guest::direct<0x4c7b50, frontend::tail::queuesClear>(app, ts);
    for (u32 player = 0; player < kQueuePlayers; ++player)
    {
        for (u32 queued = 0; queued < kQueueFrames; ++queued)
        {
            safepoint(app, ts);
            guest.setDword(kQueued + player * kPlayerBytes + queued * 4, queued < kFramesReadyFirst ? 1 : 0);
            guest.setDword(kQueue + player * kPlayerBytes + queued * 4, 0);
        }
        guest.setDword(kQueueLeads + (player + 1) * 4, kFramesReadyFirst);
    }
    guest.setDword(kQueueLeads, 0);
    guest.setDword(kFramesQueued, kFramesToRunFirst);
    guest::direct<0x4c5540, holdClear>(app, ts);
    guest.setByte(kQueueLag, 0);
    guest.setDword(kQueueStarting, 0);
    const u32 left = fe::direct<0x4a6a10, frontend::neta::barrier>(app, ts, 0x3c, 6);
    guest.setDword(kQueueStopped, 0);
    return left;
}

/* sub_4c64e0: the input's locks freed, unless the race was given up or is a replay. */
nfs::u32 frontend::inpa::inputLocksFree(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    if (guest.dword(kCancelled) != 0 || s32(gameSettings(guest).replayMode) > 1)
        return u32(0);
    guest.setDword(kQueueStopped, 1);
    guest.setDword(kQueueStarting, 1);
    guest::direct<0x4c7bb0, queueLockFree>(app, ts);
    return guest::direct<0x4c5560, networkLockFree>(app, ts);
}

/* sub_4c7b30: the queue's lock made, once. */
nfs::u32 frontend::inpa::queueLockMake(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    if (guest.dword(kQueueLock) != 0)
        return u32(0);
    const u32 lock = *fe::direct<0x4eb2e0, ea::newLock>(app, ts);
    guest.setDword(kQueueLock, lock);
    return lock;
}

/* sub_4c7bb0: the queue's lock freed. */
nfs::u32 frontend::inpa::queueLockFree(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    const u32 left = *fe::direct<0x4eb360, ea::lockFreed>(app, ts, guest.dword(kQueueLock));
    guest.setDword(kQueueLock, 0);
    return left;
}

/* The thunks under the game's names: the arguments from the registers, the core in the frame it
 * makes. */
nfs::u32 frontend::inpaRaceDebugKeys(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c4ac0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return inpa::raceDebugKeys(app, ts); });
}

nfs::u32 frontend::inpaPauseMenuStep(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c4be0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return inpa::pauseMenuStep(app, ts); });
}

nfs::u32 frontend::inpaRunRaceLoop(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c4e40", 0, 0, nullptr, 0, fe::kRestoreAll };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return inpa::runRaceLoop(app, ts); });
}

nfs::u32 frontend::inpaNetworkLockMake(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c5520", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return inpa::networkLockMake(app, ts); });
}

nfs::u32 frontend::inpaHoldClear(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c5540", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return inpa::holdClear(app, ts); });
}

nfs::u32 frontend::inpaNetworkLockFree(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c5560", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return inpa::networkLockFree(app, ts); });
}

nfs::u32 frontend::inpaNetworkHold(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c55a0", 0, 0, nullptr, 0, fe::kRestoreAll };
    const u32 mayAsk = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return inpa::networkHold(app, ts, mayAsk); });
}

nfs::u32 frontend::inpaRaceSetUp(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c62a0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return inpa::raceSetUp(app, ts); });
}

nfs::u32 frontend::inpaInputLocksMake(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c63c0", 0, 0, nullptr, 0, fe::kRestoreEcx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return inpa::inputLocksMake(app, ts); });
}

nfs::u32 frontend::inpaInputReset(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c63f0", 0, 0, nullptr, 0, fe::kRestoreAll };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return inpa::inputReset(app, ts); });
}

nfs::u32 frontend::inpaInputLocksFree(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c64e0", 0, 0, nullptr, 0, fe::kRestoreEbx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return inpa::inputLocksFree(app, ts); });
}

nfs::u32 frontend::inpaQueueLockMake(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c7b30", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return inpa::queueLockMake(app, ts); });
}

nfs::u32 frontend::inpaQueueLockFree(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c7bb0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return inpa::queueLockFree(app, ts); });
}

}
