// The game's graphics code that never runs on the port, one DEAD(address) a
// function: native_graphics_dead.cpp makes each a stub that stops the game
// with its address, and it has no code of its own.
//
// What only a branch the port's driver never takes reaches, stubbed on the
// user's decision of 2026-11-05 rather than proved: tools/audit_dead.py reports
// these as reachable, and a stub reached stops the game with its address.
//   0x507b10..0x508970  DirectDraw's start and modes: sub_5089d0 goes to them only
//                       without the screen's function sub_4f8830 sets, and
//                       sub_4eb5e0 only when a lock fails
//   0x518690..0x518ef0  the entries of the font table at 0x569038 other than the
//                       two allocators, and what only they call
//   0x523332, 0x523465  pieces only the dead sub_523488 enters
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
