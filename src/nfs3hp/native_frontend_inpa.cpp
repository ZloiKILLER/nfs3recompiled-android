#include "native_frontend_inpa.h"
#include "game_settings.h"
#include "native_controls_input.h"
#include "native_ea_net.h"
#include "native_ea_threads.h"
#include "native_frontend.h"
#include "native_frontend_connect.h"
#include "native_frontend_eaa.h"
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
#include "native_windows.h"
#include "host_threads.h"
#include "port_snd_stream_calls.h"
// What lib/thread.h holds pointers to and the checks' stand-in for SDL does not declare.
struct SDL_AtomicInt;
struct SDL_Semaphore;
#include <lib/thread.h>
#include <nfs3hp.h>
#include <lib/winapp.h>
#include <winapi/types.h>
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
 * must stay identical to the bit, so a few registers are kept as the generated code
 * keeps them: esi and edi where a call in between may change them (the sound's do, and
 * the loop goes on with what they leave), and eax where a callee or the return reads
 * what is left in it (sub_4c4490, sub_4c5720, the end of the loop).
 *
 * Plan step 5: each function a typed core in frontend::inpa (native_frontend_inpa.h), in a frame
 * of its own (fe::Frame: what it calls runs at the stack addresses it did), and an adapter under
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
using x86::reg32;
using x86::sreg32;
using App = win32::WinApplication;
using CPU = x86::CPU;

// ---------------------------------------------------------------------------
// The race's state
// ---------------------------------------------------------------------------

constexpr reg32 kTicks = 0x5efdf8;            // the system's clock
constexpr reg32 kMenuTicks = 0x5efdfc;        // the clock the pause menu is timed by
constexpr reg32 kInputFrame = 0x79c4ac;       // the frame the input has reached
constexpr reg32 kNextKeysFrame = 0x7d3688;    // the next frame whose keys are read
constexpr reg32 kLeaving = 0x7d369c;          // nonzero: the loop ends
constexpr reg32 kRestart = 0x7d3698;          // the pause menu asked for the race again
constexpr reg32 kPaused = 0x7d36a0;           // the pause menu is up
constexpr reg32 kPauseKeysFrom = 0x7d36a4;    // kMenuTicks from when it takes its keys
constexpr reg32 kPanelPausedSeen = 0x7d36a8;  // kPanelPaused as last acted on
constexpr reg32 kHalfFramesDue = 0x7d36ac;    // how far the simulation may run
constexpr reg32 kHalfFrame = 0x7d36b0;        // how far it has
constexpr reg32 kEnding = 0x7d36b4;           // how the race was left
constexpr reg32 kFramesQueued = 0x560104;     // the frames the race may run
constexpr reg32 kQueueStopped = 0x560108;
constexpr reg32 kQueueStarting = 0x56010c;
constexpr reg32 kSkipDraw = 0x7d3674;         // nonzero: this pass draws nothing
constexpr reg32 kNetworkLeaving = 0x7a22dc;   // int16: the game is over for this machine
constexpr reg32 kNetworkHost = 0x7a22da;      // int16
constexpr reg32 kNetworkPeers = 0x7a22e8;
constexpr reg32 kPanelState = 0x7cda58;       // 2 the replay's panel, 4 going into it
constexpr reg32 kPanelPaused = 0x7cd948;
constexpr reg32 kPanelSpeed = 0x7cd94c;       // 0 to 3
constexpr reg32 kPanelCommand = 0x7cd95c;     // 4: what its keys asked is to be acted on
constexpr reg32 kCamerasOnly = 0x5600ec;      // 1: this replay frame only moves the cameras
constexpr reg32 kMouseClick = 0x55e5ec;       // the buttons, dropped when the mouse moves
constexpr reg32 kMouseReleased = 0x55e5ed;    // byte, of them; 2: a click says whom the views follow
constexpr reg32 kCommandFlags = 0x55eb2e;     // byte; 0x20: the pause menu over a race that ends
constexpr reg32 kMouseX = 0x7d367c, kMouseY = 0x7d3678;
constexpr reg32 kLastMouseX = 0x7d36b8, kLastMouseY = 0x7d36bc;
constexpr reg32 kSplitLine = 0x7cdb2c;        // from this y down, the second player's half
constexpr reg32 kLookBack = 0x79c4a0;         // per player
constexpr reg32 kCars = 0x5efac8;             // per player, the car's record
constexpr reg32 kCarFlags = 0x200;            // byte; 1: its state counts
constexpr reg32 kCarState = 0x2c4;            // 3 and 4: the views do not go to it

// The developer's keys.
constexpr reg32 kFrozen = 0x5600fc;
constexpr reg32 kSingleStep = 0x55301c, kStepDirection = 0x553020, kStepSaved = 0x7d3670;

// A network race's hold.
constexpr reg32 kHoldAsked = 0x7d36c0, kHoldTimeout = 0x7d36c4, kHoldUntil = 0x7d36c8, kHoldLast = 0x7d36cc;
constexpr reg32 kHoldBytes = 0x10;            // the four of them
constexpr reg32 kHoldEvery = 0x20;            // frames between two
constexpr reg32 kHoldPacketBytes = 0xc;
constexpr std::uint8_t kHoldPacketKind = 8;
constexpr reg32 kNetworkLock = 0x7d36d8, kNetworkLockMade = 0x560100;
constexpr reg32 kEnterCriticalSection = 0x5344b8, kLeaveCriticalSection = 0x534588;

/* A function of Windows', its words handed: the port's own, called as C++ (native_windows.h); in
 * the check through its import's slot.  Terminated when the game stopped in it, as Guest::call. */
template <reg32 Slot, auto Function, typename... Words>
reg32 windows(Guest& guest, x86::CPU& cpu, Words... words)
{
    const std::optional<reg32> eax = guest::windows<Slot, Function>(guest.app(), cpu, words...);
    if (!eax)
        throw fe::Terminated();
    return *eax;
}

// The controls' queues (native_controls_queue.cpp reads them).
constexpr reg32 kQueueLock = 0x560114;
constexpr reg32 kQueuePlayers = 8, kPlayerBytes = 0x200, kQueueFrames = 0x80;
constexpr reg32 kQueue = 0x7d3714, kQueued = 0x7d4714, kQueueLeads = 0x7d5714, kQueueLag = 0x7d5740;
constexpr reg32 kFramesReadyFirst = 4;        // each player's first, ready and empty
constexpr reg32 kFramesToRunFirst = 3;

constexpr reg32 kCancelled = 0x55e88c;        // the race is given up

// ---------------------------------------------------------------------------
// What they call
// ---------------------------------------------------------------------------

constexpr reg32 kRunSystemTasks = 0x4e7630, kExitGame = 0x4f2a7d;
constexpr reg32 kFunctionKey = 0x48f7b0, kPressFunctionKey = 0x48f820, kKeyPressed = 0x48f8d0;
constexpr reg32 kSilenceForMenus = 0x4186a0, kSilenceForRace = 0x4186f0, kMasterSilent = 0x418610, kMasterBack = 0x418620;
constexpr reg32 kMusicMuteToggle = 0x4187e0, kSwapRaceVolumes = 0x418840, kSlotsStop = 0x4187a0;
constexpr reg32 kSettleCamera = 0x423b80, kAttachCameras = 0x4215c0;
constexpr reg32 kFrameHeld = 0x4c4de0, kPauseMenuStep = 0x4c4be0;
constexpr reg32 kNextViewDistance = 0x41d870;
constexpr reg32 kTakeQueuedFrame = 0x4c67a0;
constexpr reg32 kBeginDrawing = 0x4d5cc0;
constexpr reg32 kStopForces = 0x470f40, kStopForcesAgain = 0x470f80, kStopForcesOnceMore = 0x470fc0;
constexpr reg32 kCaptureScreen = 0x4c4490;
constexpr reg32 kFramesTaken = 0x4c69a0, kSyncFrame = 0x4c6970;
constexpr reg32 kClearMessages = 0x42d800, kKeyFromHook = 0x4f0100;
constexpr reg32 kFillZero = 0x4e070c, kRestoreConnection = 0x4f5280;
constexpr reg32 kMusicStreamEnd = 0x410aa0, kMusicStreamMake = 0x410370;
constexpr reg32 kStartControlsTick = 0x42dcb0, kRaceBanksLoad = 0x413bc0;
constexpr reg32 kShowWaitMessage = 0x4c5c00;
constexpr reg32 kSetQueueDelays = 0x4c6a40;

// The function keys (kFunctionKey's eax; its edx 1: only as it goes down), and scan codes.
constexpr reg32 kKeyFreeze = 1;
constexpr reg32 kKeyNextCamera = 7, kKeyLastCamera = 8, kKeyViewDistance = 9, kKeyRearView = 0xb;
constexpr reg32 kKeyHud0 = 0xc, kKeyHud1 = 0xd, kKeyMapPanes = 0xe, kKeyZoom0 = 0xf, kKeyZoom1 = 0x10;
constexpr reg32 kKeySwapVolumes = 0x12, kKeyMute = 0x14, kKeyLookBack = 0x15, kKeySettleCamera = 0x16;
constexpr reg32 kKeyLookBack2 = 0x17, kKeySettleCamera2 = 0x18;
constexpr reg32 kScanEnter = 0x1c, kScanF1 = 0x3b, kScanF2 = 0x3c, kScanF3 = 0x3d, kScanF4 = 0x3e;
constexpr reg32 kScanAlt = 0x38, kScanP = 0x19;
constexpr reg32 kFlushAll = 0x1b;             // what the keys' flush (sub_451940) is always handed here

/* Another thread's turn with the game, where the generated code offers it. */
void safepoint(win32::WinApplication* app, x86::CPU& cpu)
{
    if (app->contextWanted())
        app->yieldContext(cpu);
}

/* The calls these make as C++ leave eax as the code's calls by address do: their result, which
 * the code goes on with (the screen's capture takes what the pass left in it).  The game's code
 * hands sub_426680 a 0 in eax, which it does not read: there all the same, as the call by address
 * has it. */

/* The keys waiting thrown away (native_frontend_connect.cpp's). */
void flushKeys(Guest& guest, x86::CPU& cpu)
{
    cpu.eax = fe::direct<0x451940, frontend::connect::flushKeys>(guest.app(), cpu, kFlushAll);
}

/* The tasks due run (port_snd_stream.cpp's), with nothing. */
void runSystemTasks(Guest& guest, x86::CPU& cpu)
{
    cpu.eax = reg32(*fe::direct<0x4e7630, port::stream::runDueTasks>(guest.app(), cpu, cpu.esp - 4, reg32(0)));
}

/* The replay panel's keys, and the pause it has of its own (0x4c4f21 to 0x4c4f9b). */
void panelKeys(Guest& guest, x86::CPU& cpu)
{
    if (guest.dword(kPanelState) != 2 || guest.dword(kPaused) != 0)
        return;
    cpu.eax = fe::direct<0x4bd640, frontend::simc::panelKeys>(guest.app(), cpu);
    if (guest.dword(kPanelCommand) != 4)
        return;
    const reg32 leaving = guest.dword(kLeaving);
    guest.setDword(kPanelCommand, 0);
    if (leaving != 0 && guest.dword(kPanelPausedSeen) != 0)
    {
        guest.setDword(kPanelPausedSeen, 0);
        guest.call(kSilenceForMenus);
        return;
    }
    const reg32 panelPaused = guest.dword(kPanelPaused);
    if (panelPaused == guest.dword(kPanelPausedSeen))
        return;
    guest.setDword(kPanelPausedSeen, panelPaused);
    if (panelPaused != 0)
    {
        cpu.eax = 0;
        cpu.eax = fe::direct<0x426680, frontend::sima::camerasStep>(guest.app(), cpu);
        guest.call(kMasterSilent);
    }
    else
        guest.call(kMasterBack);
}

/* The mouse (0x4c5158 to 0x4c5243): a move drops its click, and a click left standing
 * turns the views: over a split screen to the half it is in, otherwise to the player shown. */
void mouseStep(Guest& guest, x86::CPU& cpu)
{
    // native_controls_input.cpp's: the buttons and the wheel nowhere, the pointer to its two words.
    cpu.eax = *fe::direct<0x435310, controls::mouseRead>(guest.app(), cpu, reg32(0), kMouseX, kMouseY, reg32(0));
    cpu.eax = guest.dword(kMouseX);
    if (cpu.eax != guest.dword(kLastMouseX) || guest.dword(kLastMouseY) != guest.dword(kMouseY))
    {
        guest.setDword(kLastMouseX, cpu.eax);
        cpu.eax = guest.dword(kMouseY);
        guest.setDword(kMouseClick, 0);
        guest.setDword(kLastMouseY, cpu.eax);
    }
    reg32 half = 0;
    reg32 player;
    if (gameSettings(guest).playMode == 1)
    {
        if (sreg32(guest.dword(kSplitLine)) > sreg32(guest.dword(kMouseY)))
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
    const auto carIn = [&](reg32 state) {
        cpu.eax = guest.dword(kCars + player * 4);
        return (guest.byte(cpu.eax + kCarFlags) & 1) != 0 && guest.dword(cpu.eax + kCarState) == state;
    };
    if (carIn(3) || carIn(4))
        return;
    gameSettings(guest).viewedPlayer = player;
    guest.setDword(kMouseClick, 0);
    guest.call(kAttachCameras, args({ half, player }));
    cpu.eax = fe::direct<0x485640, frontend::hudx::rebuildView>(guest.app(), cpu, half);
}

/* One input frame's keys (0x4c4f13 to 0x4c5276).  edi takes the capture key. */
void frameKeys(Guest& guest, x86::CPU& cpu)
{
    if (sreg32(gameSettings(guest).playMode) < 2)
        cpu.eax = fe::direct<0x4bc820, frontend::loopb::nothing>(guest.app(), cpu);
    panelKeys(guest, cpu);

    if (guest.dword(kLeaving) == 0)
    {
        // By address: it leaves edi as the sound's calls in it leave it.
        if (guest.dword(kPaused) != 0)
            guest.call(kPauseMenuStep);
        else
            guest.call(kFrameHeld, args({ 1 }));
    }

    if (guest.call(kFunctionKey, args({ kKeyNextCamera, 0 })) != 0)
        cpu.eax = fe::direct<0x4be970, frontend::sime::viewsResize>(guest.app(), cpu, 1);
    if (guest.call(kFunctionKey, args({ kKeyLastCamera, 0 })) != 0)
        cpu.eax = fe::direct<0x4be970, frontend::sime::viewsResize>(guest.app(), cpu, reg32(-1));

    if (guest.dword(kPaused) != 0)
    {
        if (sreg32(gameSettings(guest).playMode) <= 1)
            cpu.eax = guest::direct<0x4c4ac0, frontend::inpa::raceDebugKeys>(guest.app(), cpu);
    }
    else
    {
        if (guest.call(kFunctionKey, args({ kKeyViewDistance, 1 })) != 0)
            guest.call(kNextViewDistance);
        if (guest.call(kFunctionKey, args({ kKeyHud0, 1 })) != 0)
            cpu.eax = fe::direct<0x484260, frontend::hudx::toggleShown>(guest.app(), cpu, 0);
        if (guest.call(kFunctionKey, args({ kKeyHud1, 1 })) != 0)
            cpu.eax = fe::direct<0x484260, frontend::hudx::toggleShown>(guest.app(), cpu, 1);
        if (guest.call(kFunctionKey, args({ kKeyMapPanes, 1 })) != 0)
            cpu.eax = fe::direct<0x48e920, frontend::hudr::stepMapPanes>(guest.app(), cpu);
        if (guest.call(kFunctionKey, args({ kKeyZoom0, 0 })) != 0)
            cpu.eax = fe::direct<0x48eb60, frontend::hudx::stepZoom>(guest.app(), cpu, 0);
        if (guest.call(kFunctionKey, args({ kKeyZoom1, 0 })) != 0)
            cpu.eax = fe::direct<0x48eb60, frontend::hudx::stepZoom>(guest.app(), cpu, 1);
        if (guest.call(kFunctionKey, args({ kKeyRearView, 1 })) != 0)
            cpu.eax = fe::direct<0x484450, frontend::hudx::toggleRearView>(guest.app(), cpu);
        if (guest.call(kFunctionKey, args({ kKeyMute, 1 })) != 0)
            guest.call(kMusicMuteToggle);
        if (guest.call(kFunctionKey, args({ kKeySwapVolumes, 1 })) != 0)
            guest.call(kSwapRaceVolumes);
        if (guest.call(kFunctionKey, args({ kKeySettleCamera, 1 })) != 0 && sreg32(guest.dword(kPanelState)) < 2)
            guest.call(kSettleCamera, args({ 0 }));
        if (gameSettings(guest).playMode == 1 && guest.call(kFunctionKey, args({ kKeySettleCamera2, 1 })) != 0
            && sreg32(guest.dword(kPanelState)) < 2)
            guest.call(kSettleCamera, args({ 1 }));
        guest.setDword(kLookBack, guest.call(kFunctionKey, args({ kKeyLookBack, 0 })));
        if (gameSettings(guest).playMode == 1)
            guest.setDword(kLookBack + 4, guest.call(kFunctionKey, args({ kKeyLookBack2, 0 })));
        mouseStep(guest, cpu);
    }

    // Alt+P: a picture of the screen at the end of the pass, for a game that is not
    // a network's or that is a replay.
    if (sreg32(gameSettings(guest).playMode) < 2 || sreg32(gameSettings(guest).replayMode) > 1)
    {
        if ((cpu.eax = fe::direct<0x4eb1c0, frontend::eaa::keyStateByte>(guest.app(), cpu, kScanAlt)) != 0
            && (cpu.eax = fe::direct<0x4eb1c0, frontend::eaa::keyStateByte>(guest.app(), cpu, kScanP)) != 0)
            cpu.edi = 1;
    }
}

/* A tick of the race (native_frontend_tail.cpp's).  The game's code hands it the half frame in
 * eax, which it does not read: there all the same, as the call by address has it. */
void runFrame(Guest& guest, x86::CPU& cpu, reg32 halfFrame)
{
    cpu.eax = halfFrame;
    cpu.eax = guest::direct<0x4c48d0, frontend::tail::raceFrame>(guest.app(), cpu);
}

/* A frame as the replay's speed has it (0x4c530c to 0x4c5381): at the slowest every
 * fourth half frame runs, at half every second, at full each, at double two at a time;
 * between them only the cameras move. */
void replayFrame(Guest& guest, x86::CPU& cpu)
{
    const reg32 speed = guest.dword(kPanelSpeed);
    const sreg32 halfFrame = sreg32(guest.dword(kHalfFrame));
    if ((speed == 0 && halfFrame % 4 == 0) || (speed == 1 && halfFrame % 2 == 0) || speed == 2)
        runFrame(guest, cpu, reg32(halfFrame));
    else if (speed == 3)
    {
        runFrame(guest, cpu, reg32(halfFrame));
        runFrame(guest, cpu, guest.dword(kHalfFrame));
    }
    else
    {
        guest.setDword(kCamerasOnly, 1);
        cpu.eax = 0;
        cpu.eax = fe::direct<0x426680, frontend::sima::camerasStep>(guest.app(), cpu);
    }
}

/* The half frames that are due, run (0x4c52a8 on, to the jump to 0x4c546e). */
void runFrames(win32::WinApplication* app, Guest& guest, x86::CPU& cpu)
{
    if (sreg32(guest.dword(kHalfFramesDue)) < sreg32(guest.dword(kHalfFrame)))
        guest.setDword(kSkipDraw, 1);

    for (;;)
    {
        safepoint(app, cpu);
        if (sreg32(guest.dword(kHalfFramesDue)) < sreg32(guest.dword(kHalfFrame)))
            return;
        if (guest.call(kFrameHeld, args({ 0 })) != 0)
            return;
        if ((guest.byte(kHalfFrame) & 1) == 0)
            guest.call(kTakeQueuedFrame, args({ reg32(sreg32(guest.dword(kHalfFrame)) >> 1) }));

        cpu.esi = 0;
        const reg32 panel = guest.dword(kPanelState);
        guest.setDword(kCamerasOnly, 0);
        if (panel == 2)
            replayFrame(guest, cpu);
        else if (panel == 4)
        {
            // Into the replay: its frames count from nothing, and this pass draws none.
            guest.call(kSlotsStop);
            cpu.eax = fe::direct<0x4a33b0, frontend::loopa::raceRestart>(app, cpu);
            guest.setDword(kFramesQueued, cpu.esi);
            guest.setDword(kPanelState, 2);
            guest.setDword(kSkipDraw, 1);
            return;
        }
        else
        {
            cpu.eax = fe::direct<0x4c81c0, frontend::neta::takeInputsOnTick>(app, cpu, guest.dword(kHalfFrame));
            runFrame(guest, cpu, guest.dword(kHalfFrame));
            if ((guest.byte(kCommandFlags) & 0x20) != 0)
            {
                cpu.eax = guest.dword(kPaused);
                if (cpu.eax == 0 && guest.dword(kLeaving) != 0 && guest.dword(kEnding) == 0)
                {
                    // The race over and the flag on the command line: the pause menu over it.
                    guest.call(kBeginDrawing, args({ 0, 0 }));
                    guest.setDword(kPauseKeysFrom, guest.dword(kMenuTicks) + 0x10);
                    guest.call(kMasterSilent);
                    guest.setDword(kQueueStopped, 1);
                    guest.setDword(kPaused, 1);
                    guest.setDword(kLeaving, cpu.esi);  // as the sound's call left it
                    guest.call(kStopForces);
                    flushKeys(guest, cpu);
                }
            }
            if ((guest.byte(kHalfFrame) & 0x1f) == 0)
                cpu.eax = fe::direct<0x4c7bd0, frontend::simd::netSendPlace>(app, cpu, guest.dword(kHalfFrame));
        }
        guest.setDword(kHalfFrame, guest.dword(kHalfFrame) + 1);
    }
}

}

/* sub_4c4ac0: the developer's keys while paused, on one screen or a split one: the
 * freeze key or F1 freezes and frees the race, F3 single steps, F2 and F4 choose the way. */
x86::reg32 frontend::inpa::raceDebugKeys(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    bool toggle = false;
    if (guest.dword(kFrozen) != 0
        && (guest.call(kFunctionKey, args({ kKeyFreeze, 1 })) != 0 || guest.call(kKeyPressed, args({ kScanF1, 1 })) != 0))
    {
        flushKeys(guest, cpu);
        toggle = true;
    }
    else if (guest.dword(kFrozen) == 0 && guest.call(kKeyPressed, args({ kScanF1, 1 })) != 0)
        toggle = true;
    if (toggle)
        guest.setDword(kFrozen, guest.dword(kFrozen) == 0 ? 1 : 0);

    if (guest.call(kKeyPressed, args({ kScanF3, 1 })) != 0)
    {
        const reg32 stepping = guest.dword(kSingleStep) == 0 ? 1 : 0;
        guest.setDword(kSingleStep, stepping);
        if (stepping != 0)
            guest.setDword(kStepSaved, guest.dword(kStepDirection));
        else
            guest.setDword(kStepDirection, guest.dword(kStepSaved));
    }

    if (guest.call(kKeyPressed, args({ kScanF2, 0 })) != 0)
    {
        guest.setDword(kStepSaved, reg32(-1));
        guest.setDword(kStepDirection, reg32(-1));
        return reg32(-1);
    }
    if (guest.call(kKeyPressed, args({ kScanF4, 0 })) != 0)
    {
        guest.setDword(kStepSaved, 1);
        guest.setDword(kStepDirection, 1);
    }
    else if (guest.dword(kSingleStep) != 0)
        guest.setDword(kStepDirection, 0);
    return guest.eax();
}

/* sub_4c4be0: the pause menu's turn in a frame, once its time has come and the race is
 * not frozen: its answer 1 goes back to the race, 2 restarts it, 3, 4 and 5 leave it. */
x86::reg32 frontend::inpa::pauseMenuStep(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20);
    if (sreg32(guest.dword(kMenuTicks)) < sreg32(guest.dword(kPauseKeysFrom)) || guest.dword(kFrozen) != 0)
        return guest.eax();
    // A pass of the pause menu (native_frontend_menua.cpp's); what it leaves in eax goes in to its answer.
    const reg32 shown = fe::direct<0x44bac0, frontend::menua::racePauseMenuFrame>(app, cpu);
    const reg32 answer = cpu.eax = fe::direct<0x4c5720, frontend::neta::raceQuitVote>(app, cpu, shown);
    const auto leave = [&]() {
        cpu.eax = fe::direct<0x4d5d10, frontend::loopb::clearDrawnFlag>(app, cpu);
        guest.call(kSilenceForMenus);
        guest.setDword(kPaused, 0);
        guest.setDword(kRestart, 0);
        guest.setDword(kLeaving, 1);
        guest.call(kStopForcesOnceMore);
        cpu.eax = fe::direct<0x4ce750, frontend::loopb::closeRace>(app, cpu, 1);
    };
    switch (answer)
    {
    case 1:
        cpu.eax = fe::direct<0x4d5d10, frontend::loopb::clearDrawnFlag>(app, cpu);
        if (guest.dword(kPanelPausedSeen) == 0)
            guest.call(kMasterBack);
        guest.setDword(kQueueStopped, 0);
        guest.setDword(kPaused, 0);
        guest.call(kStopForcesAgain);
        guest.call(kPressFunctionKey, args({ kKeyFreeze, 1 }));
        guest.call(kKeyPressed, args({ kScanEnter }));  // edx still 1
        flushKeys(guest, cpu);
        break;
    case 2:
        cpu.eax = fe::direct<0x4d5d10, frontend::loopb::clearDrawnFlag>(app, cpu);
        guest.call(kSilenceForRace);
        guest.leaveEdx(1);
        guest.setDword(kPaused, 0);
        guest.setDword(kLeaving, 1);
        guest.setDword(kRestart, 1);
        guest.call(kStopForcesOnceMore);
        guest.call(kKeyPressed, args({ kScanEnter }));  // edx still 1
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
    return guest.eax();
}

/* sub_4c4e40: the race, every pass of it, until it is over. */
x86::reg32 frontend::inpa::runRaceLoop(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 32);
    reg32 lastTicks = guest.dword(kTicks) + 10000;
    cpu.edi = 0;
    // By address: the code goes on with the edx it leaves.
    guest.call(kRunSystemTasks, args({ 0, 0 }));
    reg32 lastDue = guest.edx();
    // By address: it does not come back, but the check's stand-in does, with the edi the loop reads.
    if (sreg32(guest.dword(kTicks)) <= 0)
        guest.call(kExitGame);
    const reg32 firstFrame = guest.dword(kInputFrame);
    guest.setDword(kNextKeysFrame, firstFrame);
    cpu.eax = firstFrame;  // the game's code hands it the frame, which it does not read
    cpu.eax = fe::direct<0x4ead90, frontend::eaa::bitmapDrawToScreen>(app, cpu);

    for (;;)
    {
        app->unlockContext(cpu);
        win32::Thread::sleep(0);
        app->lockContext(cpu);
        safepoint(app, cpu);
        if (guest.dword(kLeaving) != 0)
            break;

        // How far the simulation may run: the frames queued, and half a frame more
        // each pass the clock moved further than they did.
        runSystemTasks(guest, cpu);
        guest.setDword(kHalfFramesDue, guest.dword(kFramesQueued) * 2);
        const reg32 ticks = guest.dword(kTicks);
        if (sreg32(ticks - lastTicks) > sreg32(guest.dword(kHalfFramesDue) - lastDue))
            guest.setDword(kHalfFramesDue, guest.dword(kHalfFramesDue) + 1);
        lastTicks = ticks;
        lastDue = guest.dword(kHalfFramesDue);

        // Each input frame's keys, those of at most 16 behind.
        guest.setDword(kSkipDraw, 0);
        cpu.eax = guest.dword(kInputFrame) - guest.dword(kNextKeysFrame);
        if (sreg32(cpu.eax) > 0x10)
        {
            cpu.eax = guest.dword(kInputFrame);
            guest.setDword(kNextKeysFrame, cpu.eax);
        }
        for (;;)
        {
            safepoint(app, cpu);
            if (sreg32(guest.dword(kInputFrame)) < sreg32(guest.dword(kNextKeysFrame)) || guest.dword(kLeaving) != 0)
                break;
            frameKeys(guest, cpu);
            guest.setDword(kNextKeysFrame, guest.dword(kNextKeysFrame) + 1);
        }

        if (guest.dword(kPaused) != 0 || guest.dword(kPanelPausedSeen) != 0 || guest.dword(kLeaving) != 0)
        {
            cpu.eax = 0;
            cpu.eax = fe::direct<0x426680, frontend::sima::camerasStep>(app, cpu);
        }
        else
            runFrames(app, guest, cpu);

        if (guest.dword(kSkipDraw) == 0)
        {
            if (cpu.edi != 0)
            {
                guest.call(kCaptureScreen);  // eax as the pass left it goes in
                cpu.edi = 0;
            }
            else
                cpu.eax = fe::direct<0x4bc680, frontend::loopb::pauseDraw>(app, cpu, guest.dword(kPaused));
        }
        if (gameSettings(guest).playMode == 4 && sreg32(gameSettings(guest).replayMode) <= 1 && gameSettings(guest).localPlayer == 0)
            cpu.eax = fe::direct<0x49ec70, frontend::neta::dropGone>(app, cpu);

        if (guest.word(kNetworkLeaving) != 0)
        {
            if (sreg32(gameSettings(guest).replayMode) <= 1)
            {
                guest.setDword(kQueueStarting, 1);
                guest.setDword(kQueueStopped, 1);
            }
            cpu.esi = 1;
            cpu.eax = fe::direct<0x4ce750, frontend::loopb::closeRace>(app, cpu, 1);
            guest.setDword(kLeaving, cpu.esi);
        }
    }

    if (sreg32(gameSettings(guest).replayMode) > 1)
        return guest.eax();
    guest.setDword(kQueueStarting, 1);
    guest.setDword(kQueueStopped, 1);
    return reg32(1);
}

/* sub_4c5520: the network's lock made. */
x86::reg32 frontend::inpa::networkLockMake(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    const reg32 lock = *fe::direct<0x4eb2e0, ea::newLock>(app, cpu);
    guest.setDword(kNetworkLock, lock);
    guest.setDword(kNetworkLockMade, 1);
    return lock;
}

/* sub_4c5540: a network race's hold forgotten. */
x86::reg32 frontend::inpa::holdClear(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    guest.call(kFillZero, args({ kHoldAsked, kHoldBytes }));
    guest.setDword(kHoldLast, kHoldEvery);
    return guest.eax();
}

/* sub_4c5560: the network's lock freed at the race's end, and the connection put back
 * when the game is over for this machine or the race was left that way. */
x86::reg32 frontend::inpa::networkLockFree(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    fe::direct<0x4a6a10, frontend::neta::barrier>(app, cpu, 0x78, 7);
    const reg32 lock = guest.dword(kNetworkLock);
    guest.setDword(kNetworkLockMade, 0);
    // The game's code hands it 0 in edx too, which it does not read: there all the same.
    guest.leaveEdx(0);
    reg32 left = *fe::direct<0x4eb360, ea::lockFreed>(app, cpu, lock);
    if (guest.word(kNetworkLeaving) != 0 || guest.dword(kEnding) == 1)
        left = guest.call(kRestoreConnection);
    return left;
}

/* sub_4c55a0: a network race's hold, for the machines to agree on a frame to pick up
 * from.  With eax nonzero, kHoldEvery frames after the last, it is asked for: the host
 * says the frame, the others ask it.  1 once that frame's input has been taken, and
 * the race goes on; 0 meanwhile; eax back when not a network race. */
x86::reg32 frontend::inpa::networkHold(App* app, CPU& cpu, reg32 mayAsk)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 36);
    const reg32 mode = gameSettings(guest).playMode;
    if (mode == 0 || mode == 1 || sreg32(gameSettings(guest).replayMode) > 1)
        return mayAsk;

    if (mayAsk != 0 && guest.dword(kHoldUntil) == 0 && guest.dword(kHoldAsked) == 0)
    {
        const reg32 next = guest.dword(kHoldLast) + kHoldEvery;
        if (sreg32(next) < sreg32(guest.call(kFramesTaken)))
        {
            // The packet, in the frame: its kind, a word of nothing, the frame.
            constexpr reg32 kPacketNothing = 4, kPacketFrame = 8;
            const reg32 packet = guest.frame();
            guest.setDword(packet + kPacketNothing, 0);
            guest.setByte(packet, kHoldPacketKind);
            if (guest.word(kNetworkHost) == 0)
            {
                guest.setDword(packet + kPacketFrame, 0);
                guest.setDword(kHoldAsked, 1);
                fe::direct<0x4a60e0, ea::packetSent>(app, cpu, kNetworkPeers, packet, kHoldPacketBytes);
            }
            else
            {
                windows<kEnterCriticalSection, &host::enterLock>(guest, cpu, guest.dword(kNetworkLock));
                const reg32 frameNow = guest.call(kSyncFrame);
                guest.setDword(kHoldUntil, frameNow);
                guest.setDword(packet + kPacketFrame, frameNow);
                guest.setDword(kHoldAsked, 1);
                fe::direct<0x4a60e0, ea::packetSent>(app, cpu, reg32(0), packet, kHoldPacketBytes);
                windows<kLeaveCriticalSection, &host::leaveLock>(guest, cpu, guest.dword(kNetworkLock));
            }
        }
    }

    if (guest.dword(kHoldAsked) == 0 || guest.dword(kHoldUntil) == 0)
        return reg32(0);
    const reg32 taken = guest.call(kFramesTaken);
    const reg32 until = guest.dword(kHoldUntil);
    if (sreg32(taken) < sreg32(until))
        return reg32(0);

    guest.setDword(kHoldLast, until);
    guest.setDword(kQueueStopped, 1);
    guest.setDword(kHoldTimeout, 0);
    fe::direct<0x4a6a10, frontend::neta::barrier>(app, cpu, 0x1e, 8);
    guest.call(kClearMessages);
    guest.setDword(kHoldUntil, 0);
    guest.setDword(kHoldAsked, 0);
    for (;;)
    {
        safepoint(app, cpu);
        if (guest.call(kKeyFromHook) == 0)
            break;
        runSystemTasks(guest, cpu);  // eax its result: the key hook's call takes what is left in it
    }
    return reg32(1);
}

/* sub_4c62a0: what a race needs before its first frame, a piece at a time with the
 * system's messages looked at between them.  1 when the race is given up, as it is at
 * once when a network game is over for this machine. */
x86::reg32 frontend::inpa::raceSetUp(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    const reg32 mode = gameSettings(guest).playMode;
    if (mode != 0 && mode != 1 && guest.word(kNetworkLeaving) != 0)
    {
        guest.setDword(kCancelled, 1);
        return reg32(1);
    }
    const auto cancelled = [&]() {
        cpu.eax = fe::direct<0x4a33d0, frontend::loada::stopCheck>(app, cpu);
        return guest.dword(kCancelled) != 0;
    };
    // By address: it leaves esi and edi as the sound's code does, and this function hands them on.
    guest.call(kMusicStreamEnd);
    cpu.eax = fe::direct<0x4a33d0, frontend::loada::stopCheck>(app, cpu);
    guest.call(kMusicStreamMake, args({ 100 }));
    if (cancelled())
        return reg32(1);
    guest.call(kStartControlsTick);
    if (cancelled())
        return reg32(1);
    guest.call(kRaceBanksLoad);
    if (cancelled())
        return reg32(1);
    cpu.eax = fe::direct<0x4c45f0, frontend::simd::raceOpen>(app, cpu);
    if (cancelled())
        return reg32(1);
    cpu.eax = fe::direct<0x4bc8e0, frontend::simd::raceLeaveTasks>(app, cpu);
    if (cancelled())
        return reg32(1);
    cpu.eax = fe::direct<0x4fb060, frontend::wina::freeKeptBlock>(app, cpu);
    if (cancelled())
        return reg32(1);
    if (sreg32(gameSettings(guest).playMode) > 1)
        guest.call(kShowWaitMessage);
    guest::direct<0x4c63c0, inputLocksMake>(app, cpu);
    if (cancelled())
        return reg32(1);
    return reg32(0);
}

/* sub_4c63c0: the input's locks made, the queue held stopped; not for a replay. */
x86::reg32 frontend::inpa::inputLocksMake(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    if (sreg32(gameSettings(guest).replayMode) > 1)
        return guest.eax();
    guest.setDword(kQueueStopped, 1);
    guest.setDword(kQueueStarting, 1);
    guest::direct<0x4c7b30, queueLockMake>(app, cpu);
    guest::direct<0x4c5520, networkLockMake>(app, cpu);
    return guest.call(kSetQueueDelays);
}

/* sub_4c63f0: the controls' queues back to their start: each player's first frames
 * ready and empty, the rest not, and the first frames allowed to run. */
x86::reg32 frontend::inpa::inputReset(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24);
    if (sreg32(gameSettings(guest).replayMode) > 1)
    {
        guest.setDword(kFramesQueued, 0);
        return guest.eax();
    }
    guest.setDword(kQueueStopped, 1);
    guest.setDword(kQueueStarting, 1);
    cpu.eax = fe::direct<0x4a6a10, frontend::neta::barrier>(app, cpu, 0x5a, 5);
    guest::direct<0x4c7b50, frontend::tail::queuesClear>(app, cpu);
    for (reg32 player = 0; player < kQueuePlayers; ++player)
    {
        for (reg32 queued = 0; queued < kQueueFrames; ++queued)
        {
            safepoint(app, cpu);
            guest.setDword(kQueued + player * kPlayerBytes + queued * 4, queued < kFramesReadyFirst ? 1 : 0);
            guest.setDword(kQueue + player * kPlayerBytes + queued * 4, 0);
        }
        guest.setDword(kQueueLeads + (player + 1) * 4, kFramesReadyFirst);
    }
    guest.setDword(kQueueLeads, 0);
    guest.setDword(kFramesQueued, kFramesToRunFirst);
    guest::direct<0x4c5540, holdClear>(app, cpu);
    guest.setByte(kQueueLag, 0);
    guest.setDword(kQueueStarting, 0);
    const reg32 left = fe::direct<0x4a6a10, frontend::neta::barrier>(app, cpu, 0x3c, 6);
    guest.setDword(kQueueStopped, 0);
    return left;
}

/* sub_4c64e0: the input's locks freed, unless the race was given up or is a replay. */
x86::reg32 frontend::inpa::inputLocksFree(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    if (guest.dword(kCancelled) != 0 || sreg32(gameSettings(guest).replayMode) > 1)
        return guest.eax();
    guest.setDword(kQueueStopped, 1);
    guest.setDword(kQueueStarting, 1);
    guest::direct<0x4c7bb0, queueLockFree>(app, cpu);
    return guest::direct<0x4c5560, networkLockFree>(app, cpu);
}

/* sub_4c7b30: the queue's lock made, once. */
x86::reg32 frontend::inpa::queueLockMake(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    if (guest.dword(kQueueLock) != 0)
        return guest.eax();
    const reg32 lock = *fe::direct<0x4eb2e0, ea::newLock>(app, cpu);
    guest.setDword(kQueueLock, lock);
    return lock;
}

/* sub_4c7bb0: the queue's lock freed. */
x86::reg32 frontend::inpa::queueLockFree(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    // The game's code hands it 0 in edx too, which it does not read: there all the same.
    guest.leaveEdx(0);
    const reg32 left = *fe::direct<0x4eb360, ea::lockFreed>(app, cpu, guest.dword(kQueueLock));
    guest.setDword(kQueueLock, 0);
    return left;
}

/* The adapters under the game's names: the arguments from the registers, the core in the frame it
 * makes. */
void frontend::inpaRaceDebugKeys(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4c4ac0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return inpa::raceDebugKeys(app, cpu); });
}

void frontend::inpaPauseMenuStep(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4c4be0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    fe::runWhole(app, cpu, site, [&](Guest&) { return inpa::pauseMenuStep(app, cpu); });
}

void frontend::inpaRunRaceLoop(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4c4e40", 0, 0, nullptr, 0, fe::kRestoreAll };
    fe::runWhole(app, cpu, site, [&](Guest&) { return inpa::runRaceLoop(app, cpu); });
}

void frontend::inpaNetworkLockMake(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4c5520", 0, 0, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return inpa::networkLockMake(app, cpu); });
}

void frontend::inpaHoldClear(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4c5540", 0, 0, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return inpa::holdClear(app, cpu); });
}

void frontend::inpaNetworkLockFree(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4c5560", 0, 0, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return inpa::networkLockFree(app, cpu); });
}

void frontend::inpaNetworkHold(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4c55a0", 0, 0, nullptr, 0, fe::kRestoreAll };
    const reg32 mayAsk = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return inpa::networkHold(app, cpu, mayAsk); });
}

void frontend::inpaRaceSetUp(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4c62a0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return inpa::raceSetUp(app, cpu); });
}

void frontend::inpaInputLocksMake(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4c63c0", 0, 0, nullptr, 0, fe::kRestoreEcx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return inpa::inputLocksMake(app, cpu); });
}

void frontend::inpaInputReset(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4c63f0", 0, 0, nullptr, 0, fe::kRestoreAll };
    fe::runWhole(app, cpu, site, [&](Guest&) { return inpa::inputReset(app, cpu); });
}

void frontend::inpaInputLocksFree(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4c64e0", 0, 0, nullptr, 0, fe::kRestoreEbx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return inpa::inputLocksFree(app, cpu); });
}

void frontend::inpaQueueLockMake(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4c7b30", 0, 0, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return inpa::queueLockMake(app, cpu); });
}

void frontend::inpaQueueLockFree(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4c7bb0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return inpa::queueLockFree(app, cpu); });
}

}
