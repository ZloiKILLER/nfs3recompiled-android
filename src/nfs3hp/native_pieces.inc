// Addresses that are no function of their own, though the disassembly takes them
// for one: a stub each that stops the game with its address, no code of its own.
// tools/audit_dead.py reports these as reachable, by what the sources name: their
// addresses, or those of the natives they are pieces of.
//
// Data in the code.  The port reads it where it is in the image, which a stub leaves.
DEAD(0x428460)
DEAD(0x474d20)
DEAD(0x47bbb8)
DEAD(0x490430)
DEAD(0x4a3050)
DEAD(0x4ca480)
DEAD(0x512dd0)

// Pieces of functions native whole, which do not jump into other code:
// 0x4f80b4's x87 exp
DEAD(0x4f80b6)
DEAD(0x4f80e5)
// 0x4fb470, the pointer moved, and 0x4fb440
DEAD(0x4fb4c0)
DEAD(0x4fb4d0)
DEAD(0x4fb4ea)
DEAD(0x4fb50b)
DEAD(0x4fb51b)
DEAD(0x4fb527)
DEAD(0x4fb536)
DEAD(0x4fb6a0)
DEAD(0x4fb6a7)
DEAD(0x4fb6ae)
DEAD(0x4fb6b5)
DEAD(0x4fb6c2)
DEAD(0x4fb6d7)
// 0x5102a4, RefPack
DEAD(0x510337)
DEAD(0x51039e)
// 0x524640, the driver closed
DEAD(0x52468f)
DEAD(0x5247ab)

// Data in the code, read by natives where it is in the image; a piece, under a false pointer.
DEAD(0x457260)
DEAD(0x4576e0)
DEAD(0x469020)
DEAD(0x49fdd0)
DEAD(0x4a78b6)
DEAD(0x4b6a10)
DEAD(0x4ccbfa)

// Nothing calls these, and no aligned word of the image holds their address: the audit
// keeps them only for words that happen to read so at odd addresses, or for lying in
// the range of a native.
DEAD(0x401000)
DEAD(0x405fc0)
DEAD(0x40ff20)
DEAD(0x422c00)
DEAD(0x436fc0)
DEAD(0x439600)
DEAD(0x43ace0)
DEAD(0x46ca10)
DEAD(0x477290)
DEAD(0x484330)
DEAD(0x48fb40)
DEAD(0x4b48d0)
DEAD(0x4d5060)
DEAD(0x4d5220)
DEAD(0x4e47a0)
DEAD(0x4ecef0)
DEAD(0x4f4f30)
DEAD(0x4fb440)
DEAD(0x4ff4f2)
DEAD(0x5157ec)
DEAD(0x51a1b8)
DEAD(0x52c870)
DEAD(0x52c88f)
DEAD(0x52ffc7)
DEAD(0x530019)
DEAD(0x53006b)

// Called only by what is a stub already.
DEAD(0x496300)
DEAD(0x4eb420)
DEAD(0x4f45e1)
DEAD(0x5251a0)

// The writer of a car's specs file and its helpers, which nothing in the game calls.
DEAD(0x4b22d0)
DEAD(0x4b0790)
DEAD(0x4b07c0)
DEAD(0x4b0880)
DEAD(0x4b08d0)
