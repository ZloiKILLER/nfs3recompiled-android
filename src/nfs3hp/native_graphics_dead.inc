// The game's graphics code that never runs on the port, one DEAD(address) a
// function: native_graphics_dead.cpp makes each a stub that stops the game
// with its address, and it has no code of its own.
//
// The first part is what nothing calls: no call, jump or address of it in code
// that runs, and none in the game's data.  The last part is not
// proved so: see its comment.

// The software renderers' blend colour by mode (state 0x2bd).
DEAD(0x432440)
DEAD(0x4c0f60)
DEAD(0x524820)

// What only a branch the port's driver never takes reaches, stubbed on the
// user's decision of 2026-11-05 rather than proved: tools/audit_dead.py reports
// these as reachable, and a stub reached stops the game with its address.
//   0x5043f0..0x507a60  driver slot 0's functions (the table at 0x564af8), the
//                       software renderer's; useDriverSlot and keepDriverSlot
//                       copy their addresses, and those stay valid
//   0x507b10..0x508970  DirectDraw's start and modes: sub_5089d0 goes to them only
//                       without the screen's function sub_4f8830 sets, and
//                       sub_4eb5e0 only when a lock fails
//   0x518690..0x518ef0  the entries of the font table at 0x569038 other than the
//                       two allocators, and what only they call
//   0x523332, 0x523465  pieces only the dead sub_523488 enters
DEAD(0x5043f0)
DEAD(0x5049c0)
DEAD(0x5049f0)
DEAD(0x504a30)
DEAD(0x504af0)
DEAD(0x504b70)
DEAD(0x504bf0)
DEAD(0x504c00)
DEAD(0x504c20)
DEAD(0x504c40)
DEAD(0x504c80)
DEAD(0x504e00)
DEAD(0x504f90)
DEAD(0x5050b0)
DEAD(0x5050e0)
DEAD(0x5050f0)
DEAD(0x5051c0)
DEAD(0x505250)
DEAD(0x505280)
DEAD(0x5052a0)
DEAD(0x505320)
DEAD(0x505500)
DEAD(0x505570)
DEAD(0x505650)
DEAD(0x5057d0)
DEAD(0x505830)
DEAD(0x505870)
DEAD(0x505920)
DEAD(0x505950)
DEAD(0x505970)
DEAD(0x505980)
DEAD(0x5059d0)
DEAD(0x505a50)
DEAD(0x505a80)
DEAD(0x505b30)
DEAD(0x506230)
DEAD(0x506310)
DEAD(0x506360)
DEAD(0x5063d0)
DEAD(0x506440)
DEAD(0x506690)
DEAD(0x506910)
DEAD(0x506ac0)
DEAD(0x506ca0)
DEAD(0x507240)
DEAD(0x5074b0)
DEAD(0x5075d0)
DEAD(0x507720)
DEAD(0x507900)
DEAD(0x5079a0)
DEAD(0x507a60)
DEAD(0x507b10)
DEAD(0x507bc0)
DEAD(0x507d00)
DEAD(0x507ed0)
DEAD(0x507f30)
DEAD(0x508170)
DEAD(0x5082f0)
DEAD(0x508390)
DEAD(0x5084e0)
DEAD(0x508510)
DEAD(0x508970)
DEAD(0x518690)
DEAD(0x518870)
DEAD(0x518d50)
DEAD(0x518df0)
DEAD(0x518e40)
DEAD(0x518eb0)
DEAD(0x518ee0)
DEAD(0x518ef0)
DEAD(0x523332)
DEAD(0x523465)
