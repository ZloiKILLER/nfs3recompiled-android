// The game's code that nothing can reach, one DEAD(address) a function:
// native_dead.cpp makes each a stub that stops the game with its address, and
// disassemble_nfs3hp.py generates no code for it.
//
// tools/audit_dead.py proves each, and writes this list (--write): no call,
// jump, immediate or displacement of it in any instruction that can run, the
// natives' original code included; no pointer to it, or into it, anywhere in
// the image outside instructions (every relocation, every 4-byte window of the
// data); no address of it in the port's hand-written source; and no running on
// into it from the function before.  Most are what the linker brought in with
// the libraries' objects -- the modem and serial links (TAPI, RAS, COM ports),
// the C runtime's unused functions, EA's unused helpers.
//
// After each the instructions in it, the Windows functions it calls and the
// first text it names, for reading.

DEAD(0x404310)  // 17 instructions
DEAD(0x404590)  // 4 instructions
DEAD(0x4045a0)  // 4 instructions
DEAD(0x4045b0)  // 4 instructions
DEAD(0x4045c0)  // 4 instructions
DEAD(0x4045d0)  // 4 instructions
DEAD(0x4045e0)  // 4 instructions
DEAD(0x4045f0)  // 4 instructions
DEAD(0x404600)  // 4 instructions
DEAD(0x405b50)  // 77 instructions
DEAD(0x405c20)  // 33 instructions
DEAD(0x405e50)  // 4 instructions
DEAD(0x408af0)  // 28 instructions
DEAD(0x4094ba)  // 3 instructions
DEAD(0x40b7f9)  // 26 instructions
DEAD(0x40bd60)  // 4 instructions
DEAD(0x40bd70)  // 4 instructions
DEAD(0x40ece0)  // 20 instructions
DEAD(0x40ef30)  // 23 instructions
DEAD(0x40f0c0)  // 18 instructions
DEAD(0x40fa10)  // 62 instructions
DEAD(0x410a40)  // 26 instructions
DEAD(0x410c30)  // 4 instructions
DEAD(0x417d50)  // 33 instructions
DEAD(0x418670)  // 13 instructions
DEAD(0x418cf0)  // 4 instructions
DEAD(0x418db0)  // 10 instructions
DEAD(0x418e00)  // 16 instructions
DEAD(0x418e20)  // 28 instructions
DEAD(0x418e60)  // 20 instructions
DEAD(0x418fa0)  // 25 instructions
DEAD(0x41ffb0)  // 8 instructions
DEAD(0x421370)  // 4 instructions
DEAD(0x422b70)  // 14 instructions
DEAD(0x422c40)  // 21 instructions
DEAD(0x422c80)  // 4 instructions
DEAD(0x428480)  // 22 instructions
DEAD(0x4284b0)  // 12 instructions
DEAD(0x429490)  // 8 instructions
DEAD(0x42d4f0)  // 4 instructions
DEAD(0x42d530)  // 4 instructions
DEAD(0x42d540)  // 4 instructions
DEAD(0x42daa0)  // 37 instructions; EnterCriticalSection, LeaveCriticalSection
DEAD(0x42dc3e)  // 2 instructions
DEAD(0x431e80)  // 91 instructions
DEAD(0x431fa0)  // 64 instructions
DEAD(0x432070)  // 90 instructions
DEAD(0x432190)  // 63 instructions
DEAD(0x432260)  // 59 instructions
DEAD(0x432320)  // 33 instructions
DEAD(0x434799)  // 9 instructions
DEAD(0x434ad0)  // 4 instructions
DEAD(0x4384a0)  // 31 instructions
DEAD(0x4385f0)  // 70 instructions; "fedata"
DEAD(0x4386a0)  // 5 instructions
DEAD(0x438b40)  // 18 instructions
DEAD(0x438fc0)  // 104 instructions
DEAD(0x4390f0)  // 81 instructions
DEAD(0x4391d0)  // 47 instructions
DEAD(0x439450)  // 88 instructions
DEAD(0x439590)  // 6 instructions
DEAD(0x4396a0)  // 6 instructions
DEAD(0x4396d0)  // 6 instructions
DEAD(0x439700)  // 6 instructions
DEAD(0x439740)  // 7 instructions
DEAD(0x439930)  // 5 instructions
DEAD(0x439940)  // 5 instructions
DEAD(0x439950)  // 5 instructions
DEAD(0x439960)  // 5 instructions
DEAD(0x439970)  // 5 instructions
DEAD(0x439980)  // 5 instructions
DEAD(0x43b330)  // 36 instructions
DEAD(0x43bde0)  // 37 instructions
DEAD(0x4417f0)  // 74 instructions
DEAD(0x442d30)  // 12 instructions
DEAD(0x442d50)  // 5 instructions
DEAD(0x442d60)  // 42 instructions
DEAD(0x44a680)  // 5 instructions
DEAD(0x44a6a0)  // 6 instructions
DEAD(0x44ad60)  // 32 instructions
DEAD(0x44adb0)  // 32 instructions
DEAD(0x44ae20)  // 26 instructions
DEAD(0x451640)  // 22 instructions
DEAD(0x451670)  // 22 instructions
DEAD(0x451715)  // 52 instructions
DEAD(0x4517d0)  // 17 instructions
DEAD(0x452910)  // 38 instructions
DEAD(0x453440)  // 15 instructions
DEAD(0x453470)  // 12 instructions
DEAD(0x4534a0)  // 15 instructions
DEAD(0x457310)  // 5 instructions
DEAD(0x4654e0)  // 4 instructions
DEAD(0x4654f0)  // 5 instructions
DEAD(0x470a70)  // 14 instructions
DEAD(0x476f30)  // 17 instructions
DEAD(0x476f60)  // 17 instructions
DEAD(0x476f90)  // 7 instructions
DEAD(0x477030)  // 22 instructions
DEAD(0x4770c0)  // 30 instructions
DEAD(0x477120)  // 27 instructions
DEAD(0x477380)  // 76 instructions
DEAD(0x477690)  // 30 instructions
DEAD(0x4776f0)  // 25 instructions
DEAD(0x477740)  // 37 instructions
DEAD(0x4777a0)  // 5 instructions
DEAD(0x4777b0)  // 4 instructions
DEAD(0x4777c0)  // 4 instructions
DEAD(0x477df0)  // 120 instructions
DEAD(0x47b380)  // 4 instructions
DEAD(0x482610)  // 4 instructions
DEAD(0x484230)  // 15 instructions
DEAD(0x484390)  // 32 instructions
DEAD(0x4843f0)  // 31 instructions
DEAD(0x489ab0)  // 49 instructions; "%d %s"
DEAD(0x48cae0)  // 4 instructions
DEAD(0x48caf0)  // 42 instructions
DEAD(0x48eef0)  // 18 instructions
DEAD(0x48ef20)  // 5 instructions
DEAD(0x48ef30)  // 11 instructions
DEAD(0x48faa0)  // 53 instructions; "%s%s"
DEAD(0x4900b0)  // 35 instructions
DEAD(0x490110)  // 17 instructions
DEAD(0x490140)  // 153 instructions; "NETSCAPE"
DEAD(0x4902f0)  // 92 instructions; "NETREPORT"
DEAD(0x495a00)  // 5 instructions
DEAD(0x496370)  // 11 instructions
DEAD(0x496390)  // 75 instructions
DEAD(0x496440)  // 13 instructions
DEAD(0x496490)  // 28 instructions
DEAD(0x496690)  // 146 instructions
DEAD(0x496b10)  // 13 instructions
DEAD(0x496b60)  // 58 instructions
DEAD(0x496c40)  // 48 instructions
DEAD(0x496cb0)  // 133 instructions
DEAD(0x496df0)  // 128 instructions
DEAD(0x496fc0)  // 36 instructions
DEAD(0x497010)  // 64 instructions
DEAD(0x497240)  // 74 instructions
DEAD(0x497360)  // 4 instructions
DEAD(0x497370)  // 4 instructions
DEAD(0x497380)  // 25 instructions
DEAD(0x4973e0)  // 37 instructions
DEAD(0x497490)  // 35 instructions
DEAD(0x4992f0)  // 6 instructions
DEAD(0x499300)  // 17 instructions
DEAD(0x49bcf0)  // 16 instructions
DEAD(0x49c1b0)  // 4 instructions
DEAD(0x49c280)  // 8 instructions
DEAD(0x49c290)  // 5 instructions
DEAD(0x49c2d0)  // 24 instructions
DEAD(0x49c310)  // 52 instructions
DEAD(0x49c3b0)  // 14 instructions
DEAD(0x49f940)  // 122 instructions
DEAD(0x49fa90)  // 44 instructions; GetTickCount
DEAD(0x49fc80)  // 91 instructions
DEAD(0x49fda0)  // 12 instructions
DEAD(0x4a5aa0)  // 30 instructions; FreeLibrary
DEAD(0x4a5d70)  // 38 instructions
DEAD(0x4a5f20)  // 7 instructions
DEAD(0x4a5f80)  // 5 instructions
DEAD(0x4a5f90)  // 12 instructions
DEAD(0x4a5fb0)  // 21 instructions; "Gordy"
DEAD(0x4a5ff0)  // 20 instructions
DEAD(0x4a6020)  // 53 instructions
DEAD(0x4a60a0)  // 10 instructions
DEAD(0x4a60ab)  // 15 instructions
DEAD(0x4a60d0)  // 5 instructions
DEAD(0x4a83af)  // 18 instructions
DEAD(0x4a8a2b)  // 40 instructions
DEAD(0x4aa682)  // 76 instructions
DEAD(0x4b54f0)  // 63 instructions; "(%d)"
DEAD(0x4b5710)  // 9 instructions; GetWindowsDirectoryA
DEAD(0x4b6bb0)  // 91 instructions
DEAD(0x4b70b0)  // 24 instructions
DEAD(0x4b7180)  // 38 instructions
DEAD(0x4b76d0)  // 26 instructions
DEAD(0x4b7920)  // 57 instructions
DEAD(0x4b7f30)  // 31 instructions
DEAD(0x4bbcc0)  // 24 instructions; RegCloseKey
DEAD(0x4bbd00)  // 21 instructions; RegOpenKeyExA
DEAD(0x4bbd30)  // 32 instructions; RegQueryValueExA
DEAD(0x4be090)  // 122 instructions
DEAD(0x4beb60)  // 15 instructions
DEAD(0x4becd0)  // 31 instructions
DEAD(0x4bf160)  // 24 instructions
DEAD(0x4c4480)  // 4 instructions
DEAD(0x4c8200)  // 27 instructions; EnterCriticalSection, LeaveCriticalSection
DEAD(0x4cc820)  // 11 instructions; LeaveCriticalSection
DEAD(0x4cc840)  // 11 instructions; EnterCriticalSection
DEAD(0x4cca00)  // 55 instructions; EnterCriticalSection, LeaveCriticalSection
DEAD(0x4ccbb0)  // 3 instructions
DEAD(0x4cdc20)  // 20 instructions; GetSystemMetrics
DEAD(0x4cdcd0)  // 17 instructions
DEAD(0x4cebf0)  // 78 instructions
DEAD(0x4cecc0)  // 92 instructions
DEAD(0x4cefc0)  // 27 instructions; ";50}g"
DEAD(0x4cf120)  // 29 instructions
DEAD(0x4cfda0)  // 21 instructions
DEAD(0x4cfec0)  // 19 instructions
DEAD(0x4d0540)  // 19 instructions
DEAD(0x4d0570)  // 67 instructions; EnterCriticalSection, LeaveCriticalSection
DEAD(0x4d0610)  // 29 instructions
DEAD(0x4d15e0)  // 116 instructions
DEAD(0x4d3410)  // 59 instructions
DEAD(0x4d34b0)  // 193 instructions
DEAD(0x4d43e0)  // 35 instructions
DEAD(0x4d5030)  // 14 instructions
DEAD(0x4d5360)  // 4 instructions
DEAD(0x4d56c0)  // 5 instructions
DEAD(0x4d56d0)  // 5 instructions
DEAD(0x4d56e0)  // 4 instructions
DEAD(0x4d5a80)  // 153 instructions
DEAD(0x4d6c70)  // 345 instructions; "ERROR: shape %d not loaded in texture pool"
DEAD(0x4dc220)  // 26 instructions
DEAD(0x4dc260)  // 27 instructions
DEAD(0x4dc2a0)  // 27 instructions
DEAD(0x4dc2e0)  // 8 instructions
DEAD(0x4dc300)  // 43 instructions
DEAD(0x4dc350)  // 34 instructions
DEAD(0x4dd1f0)  // 23 instructions
DEAD(0x4df350)  // 19 instructions
DEAD(0x4df380)  // 5 instructions
DEAD(0x4df390)  // 4 instructions
DEAD(0x4df3a0)  // 43 instructions
DEAD(0x4df420)  // 90 instructions
DEAD(0x4df530)  // 21 instructions; "%04X.prf"
DEAD(0x4df570)  // 30 instructions
DEAD(0x4e00c0)  // 28 instructions
DEAD(0x4e0250)  // 13 instructions
DEAD(0x4e04e0)  // 76 instructions
DEAD(0x4e05b0)  // 31 instructions
DEAD(0x4e0620)  // 10 instructions
DEAD(0x4e0660)  // 16 instructions
DEAD(0x4e06c4)  // 7 instructions
DEAD(0x4e06ee)  // 3 instructions
DEAD(0x4e06fa)  // 4 instructions
DEAD(0x4e2224)  // 2 instructions
DEAD(0x4e42e0)  // 26 instructions
DEAD(0x4e43b0)  // 5 instructions
DEAD(0x4e43c0)  // 2 instructions
DEAD(0x4e43d0)  // 26 instructions; "cmn/cdstream.c"
DEAD(0x4e4a10)  // 9 instructions
DEAD(0x4e50e0)  // 32 instructions; EnterCriticalSection, LeaveCriticalSection
DEAD(0x4e5140)  // 65 instructions; EnterCriticalSection, LeaveCriticalSection
DEAD(0x4e5250)  // 69 instructions; "cmn/cdstream.c"
DEAD(0x4e68d0)  // 4 instructions
DEAD(0x4e6910)  // 20 instructions; EnterCriticalSection, LeaveCriticalSection
DEAD(0x4e6960)  // 26 instructions
DEAD(0x4e69a0)  // 9 instructions
DEAD(0x4e69c0)  // 9 instructions
DEAD(0x4e6ad0)  // 33 instructions; EnterCriticalSection, LeaveCriticalSection
DEAD(0x4ea330)  // 20 instructions
DEAD(0x4ea360)  // 19 instructions
DEAD(0x4ea390)  // 12 instructions
DEAD(0x4ea3b0)  // 57 instructions; "cmn/locatbig.c"
DEAD(0x4ea750)  // 71 instructions
DEAD(0x4ea8f0)  // 3 instructions
DEAD(0x4ea900)  // 3 instructions
DEAD(0x4ea910)  // 3 instructions
DEAD(0x4ea990)  // 16 instructions
DEAD(0x4ead10)  // 1 instructions
DEAD(0x4ead80)  // 2 instructions
DEAD(0x4eaea0)  // 2 instructions
DEAD(0x4eb810)  // 8 instructions
DEAD(0x4eb830)  // 29 instructions
DEAD(0x4eba30)  // 24 instructions; GetClientRect
DEAD(0x4ebaa0)  // 20 instructions; ShowWindow
DEAD(0x4ebae0)  // 8 instructions
DEAD(0x4ebd50)  // 78 instructions
DEAD(0x4ece00)  // 11 instructions; SetForegroundWindow
DEAD(0x4ece20)  // 24 instructions
DEAD(0x4ece60)  // 30 instructions
DEAD(0x4eceb0)  // 21 instructions
DEAD(0x4ecf20)  // 24 instructions
DEAD(0x4ed060)  // 48 instructions
DEAD(0x4ed440)  // 27 instructions
DEAD(0x4ed760)  // 8 instructions
DEAD(0x4ed7a0)  // 19 instructions
DEAD(0x4ed7d0)  // 19 instructions
DEAD(0x4ed800)  // 15 instructions
DEAD(0x4ed910)  // 31 instructions
DEAD(0x4ed950)  // 31 instructions
DEAD(0x4ed990)  // 40 instructions
DEAD(0x4eda10)  // 97 instructions
DEAD(0x4edb80)  // 24 instructions
DEAD(0x4edff4)  // 52 instructions
DEAD(0x4ee07c)  // 46 instructions
DEAD(0x4ef1a0)  // 3 instructions
DEAD(0x4ef4a0)  // 12 instructions
DEAD(0x4ef4e0)  // 12 instructions
DEAD(0x4ef500)  // 10 instructions
DEAD(0x4f0160)  // 34 instructions; "win/key.c"
DEAD(0x4f01c0)  // 22 instructions
DEAD(0x4f0250)  // 3 instructions
DEAD(0x4f0640)  // 6 instructions
DEAD(0x4f0670)  // 8 instructions
DEAD(0x4f0690)  // 7 instructions
DEAD(0x4f06a1)  // 7 instructions
DEAD(0x4f06c0)  // 8 instructions
DEAD(0x4f0870)  // 2 instructions
DEAD(0x4f0a30)  // 10 instructions
DEAD(0x4f0a50)  // 12 instructions
DEAD(0x4f0a70)  // 10 instructions
DEAD(0x4f1110)  // 82 instructions; "win/async.c"
DEAD(0x4f1230)  // 5 instructions
DEAD(0x4f1240)  // 83 instructions; "win/async.c"
DEAD(0x4f1360)  // 5 instructions
DEAD(0x4f14b0)  // 11 instructions; "win/async.c"
DEAD(0x4f14f0)  // 5 instructions
DEAD(0x4f15e0)  // 8 instructions
DEAD(0x4f15f0)  // 10 instructions
DEAD(0x4f1610)  // 83 instructions; "win/async.c"
DEAD(0x4f1710)  // 42 instructions; "win/async.c"
DEAD(0x4f2360)  // 10 instructions
DEAD(0x4f2d00)  // 12 instructions
DEAD(0x4f2d20)  // 10 instructions
DEAD(0x4f2d40)  // 12 instructions
DEAD(0x4f2d60)  // 10 instructions
DEAD(0x4f2da0)  // 45 instructions
DEAD(0x4f2f30)  // 10 instructions
DEAD(0x4f3a30)  // 15 instructions
DEAD(0x4f43ac)  // 8 instructions
DEAD(0x4f4438)  // 3 instructions
DEAD(0x4f45d1)  // 4 instructions
DEAD(0x4f4630)  // 8 instructions
DEAD(0x4f4810)  // 84 instructions; LeaveCriticalSection, EnterCriticalSection; "validatememclass"
DEAD(0x4f48f0)  // 19 instructions
DEAD(0x4f4ef0)  // 11 instructions; "NET_readgame"
DEAD(0x4f4fd0)  // 11 instructions; "NET_getplayers"
DEAD(0x4f5010)  // 17 instructions; "NET_setfilter"
DEAD(0x4f5070)  // 11 instructions
DEAD(0x4f5090)  // 11 instructions
DEAD(0x4f7b1a)  // 4 instructions
DEAD(0x4f7c09)  // 6 instructions
DEAD(0x4f80ac)  // 4 instructions
DEAD(0x4f88c0)  // 27 instructions; "3rash/inithal.c"
DEAD(0x4f9090)  // 3 instructions
DEAD(0x4f90a0)  // 8 instructions
DEAD(0x4f90d0)  // 3 instructions
DEAD(0x4f9140)  // 15 instructions; GetSystemMetrics
DEAD(0x4f9170)  // 5 instructions
DEAD(0x4f9190)  // 5 instructions
DEAD(0x4f91b0)  // 5 instructions
DEAD(0x4f92e0)  // 5 instructions
DEAD(0x4f92f0)  // 14 instructions
DEAD(0x4f9310)  // 2 instructions
DEAD(0x4f9470)  // 5 instructions
DEAD(0x4f94bf)  // 3 instructions
DEAD(0x4fa830)  // 145 instructions
DEAD(0x4faa40)  // 107 instructions
DEAD(0x4fabe0)  // 167 instructions
DEAD(0x4faea0)  // 121 instructions
DEAD(0x4fb030)  // 16 instructions
DEAD(0x4fb080)  // 67 instructions; "win/inversep.c"
DEAD(0x4fb140)  // 13 instructions
DEAD(0x4fb170)  // 13 instructions
DEAD(0x4fb1a0)  // 22 instructions
DEAD(0x4fb560)  // 17 instructions
DEAD(0x4fb590)  // 2 instructions
DEAD(0x4fb5a0)  // 80 instructions; EnterCriticalSection, LeaveCriticalSection
DEAD(0x4fb6f0)  // 6 instructions
DEAD(0x4fb850)  // 4 instructions
DEAD(0x4fc400)  // 52 instructions
DEAD(0x4fc480)  // 62 instructions
DEAD(0x4fc510)  // 70 instructions; "win/nfile.c"
DEAD(0x4fc860)  // 52 instructions
DEAD(0x4fca00)  // 15 instructions
DEAD(0x4fca30)  // 15 instructions
DEAD(0x4fcb00)  // 25 instructions
DEAD(0x4fcb50)  // 12 instructions
DEAD(0x4fcb70)  // 16 instructions
DEAD(0x4fcba0)  // 13 instructions
DEAD(0x4fcbc0)  // 65 instructions; EnterCriticalSection, LeaveCriticalSection; "win/nfile.c"
DEAD(0x4fcc90)  // 31 instructions
DEAD(0x4fcce0)  // 96 instructions; EnterCriticalSection, LeaveCriticalSection; "win/nfile.c"
DEAD(0x4fce20)  // 5 instructions
DEAD(0x4fce30)  // 1 instructions
DEAD(0x4fce40)  // 1 instructions
DEAD(0x4fce90)  // 1 instructions
DEAD(0x4fcea0)  // 1 instructions
DEAD(0x4fceb0)  // 47 instructions
DEAD(0x4fcf40)  // 49 instructions
DEAD(0x4fcfe0)  // 28 instructions
DEAD(0x4fd020)  // 7 instructions
DEAD(0x4fd040)  // 7 instructions
DEAD(0x4fd090)  // 20 instructions
DEAD(0x4fd0c0)  // 20 instructions
DEAD(0x4fd0f0)  // 28 instructions
DEAD(0x4fd130)  // 20 instructions
DEAD(0x4fd160)  // 20 instructions
DEAD(0x4fd190)  // 20 instructions
DEAD(0x4fd420)  // 35 instructions
DEAD(0x4ff0e0)  // 3 instructions
DEAD(0x4ff5d0)  // 3 instructions
DEAD(0x5028b8)  // 4 instructions
DEAD(0x503700)  // 45 instructions
DEAD(0x5037a0)  // 39 instructions
DEAD(0x503800)  // 39 instructions
DEAD(0x503860)  // 39 instructions
DEAD(0x5038c0)  // 36 instructions
DEAD(0x503910)  // 49 instructions
DEAD(0x503990)  // 51 instructions
DEAD(0x503a10)  // 63 instructions
DEAD(0x503aa0)  // 47 instructions
DEAD(0x503b60)  // 33 instructions
DEAD(0x503e80)  // 16 instructions
DEAD(0x503f10)  // 22 instructions
DEAD(0x503f50)  // 12 instructions; "win/palette.c"
DEAD(0x503f90)  // 87 instructions; "win/palette.c"
DEAD(0x508500)  // 2 instructions
DEAD(0x5085a0)  // 8 instructions
DEAD(0x5085c0)  // 272 instructions
DEAD(0x508b20)  // 40 instructions
DEAD(0x508ec0)  // 10 instructions
DEAD(0x508ee0)  // 10 instructions
DEAD(0x508f00)  // 68 instructions; "win/vidwin.c"
DEAD(0x508fd0)  // 16 instructions
DEAD(0x5093d0)  // 90 instructions
DEAD(0x509bd0)  // 13 instructions
DEAD(0x509ec0)  // 17 instructions
DEAD(0x50a700)  // 106 instructions; "win/convshp.c"
DEAD(0x50af80)  // 8 instructions
DEAD(0x50afa0)  // 15 instructions
DEAD(0x50dbb0)  // 12 instructions
DEAD(0x50dbd0)  // 10 instructions
DEAD(0x50e360)  // 53 instructions; EnterCriticalSection, LeaveCriticalSection; "qhighpriority"
DEAD(0x50e670)  // 72 instructions; EnterCriticalSection, LeaveCriticalSection; "qfetchindex"
DEAD(0x50e740)  // 61 instructions; EnterCriticalSection, LeaveCriticalSection; "qfetchnext"
DEAD(0x510740)  // 10 instructions
DEAD(0x511200)  // 8 instructions
DEAD(0x5116ac)  // 33 instructions; "cmn/spktplay.c"
DEAD(0x511d30)  // 43 instructions; "cmn/capture.c"
DEAD(0x511dc0)  // 288 instructions; "ATTN"
DEAD(0x512710)  // 28 instructions; "cmn/layer.c"
DEAD(0x512790)  // 10 instructions
DEAD(0x5127b0)  // 10 instructions
DEAD(0x515af0)  // 8 instructions
DEAD(0x515b0c)  // 11 instructions
DEAD(0x5160b8)  // 12 instructions
DEAD(0x5160d8)  // 10 instructions
DEAD(0x517560)  // 2 instructions
DEAD(0x517570)  // 2 instructions
DEAD(0x518100)  // 148 instructions
DEAD(0x519080)  // 63 instructions
DEAD(0x5191b0)  // 306 instructions
DEAD(0x519560)  // 24 instructions
DEAD(0x5195b0)  // 309 instructions
DEAD(0x519a10)  // 30 instructions
DEAD(0x519ae0)  // 246 instructions; "font/fontdraw.c"
DEAD(0x519e30)  // 22 instructions
DEAD(0x51a188)  // 14 instructions
DEAD(0x51a1f0)  // 5 instructions; "ATZS7=60S11=55"
DEAD(0x51a200)  // 37 instructions; "ATE1Q0X4"
DEAD(0x51a260)  // 22 instructions; "ATH0"
DEAD(0x51a2a0)  // 52 instructions
DEAD(0x51a310)  // 71 instructions
DEAD(0x51a3b0)  // 17 instructions
DEAD(0x51a3e0)  // 13 instructions; "ATS0=1"
DEAD(0x51a400)  // 8 instructions
DEAD(0x51a420)  // 37 instructions
DEAD(0x51ab10)  // 15 instructions; "ATM0"
DEAD(0x51b060)  // 36 instructions; SetLastError, EnterCriticalSection, LeaveCriticalSection
DEAD(0x51b0d0)  // 36 instructions; SetLastError, EnterCriticalSection, LeaveCriticalSection
DEAD(0x51b140)  // 44 instructions; SetLastError, EnterCriticalSection, LeaveCriticalSection
DEAD(0x51b5a0)  // 29 instructions
DEAD(0x51b830)  // 34 instructions; EnterCriticalSection, LeaveCriticalSection
DEAD(0x51bae0)  // 41 instructions; EnterCriticalSection, LeaveCriticalSection
DEAD(0x51bb60)  // 40 instructions; EnterCriticalSection, LeaveCriticalSection
DEAD(0x51db20)  // 232 instructions
DEAD(0x51dd50)  // 63 instructions
DEAD(0x51ddf0)  // 46 instructions
DEAD(0x51def0)  // 46 instructions
DEAD(0x51dfb0)  // 47 instructions
DEAD(0x51e020)  // 23 instructions
DEAD(0x51e1c8)  // 39 instructions
DEAD(0x51e26c)  // 13 instructions
DEAD(0x51e290)  // 4 instructions
DEAD(0x523488)  // 4 instructions
DEAD(0x5234a0)  // 14 instructions
DEAD(0x5235d0)  // 3 instructions
DEAD(0x5235dc)  // 3 instructions
DEAD(0x5235f0)  // 12 instructions; MoveFileA
DEAD(0x524f80)  // 1 instructions
DEAD(0x525130)  // 47 instructions
DEAD(0x525590)  // 49 instructions
DEAD(0x5256b0)  // 58 instructions
DEAD(0x525ac0)  // 60 instructions
DEAD(0x525b90)  // 7 instructions
DEAD(0x525d04)  // 35 instructions
DEAD(0x525fd0)  // 13 instructions
DEAD(0x5261cc)  // 15 instructions; SetConsoleCtrlHandler
DEAD(0x526238)  // 2 instructions
DEAD(0x526288)  // 46 instructions
DEAD(0x526f9a)  // 6 instructions
DEAD(0x526fad)  // 6 instructions
DEAD(0x526fc0)  // 25 instructions
DEAD(0x527058)  // 25 instructions
DEAD(0x5270a4)  // 25 instructions
DEAD(0x52b641)  // 2 instructions
DEAD(0x52b7d0)  // 36 instructions; "font/textcen.c"
DEAD(0x52bf70)  // 46 instructions
DEAD(0x52bfe0)  // 163 instructions
DEAD(0x52c410)  // 4 instructions
DEAD(0x52c420)  // 4 instructions
DEAD(0x52c430)  // 109 instructions
DEAD(0x52c580)  // 32 instructions
DEAD(0x52c5d0)  // 92 instructions
DEAD(0x52c720)  // 52 instructions
DEAD(0x52c7b0)  // 42 instructions
DEAD(0x52c814)  // 2 instructions
DEAD(0x52c82c)  // 23 instructions
DEAD(0x52c8c7)  // 11 instructions
DEAD(0x52e654)  // 5 instructions
DEAD(0x52eb40)  // 20 instructions
DEAD(0x52f370)  // 17 instructions; "lineAccept"
DEAD(0x52f3bb)  // 17 instructions; "lineAddProvider"
DEAD(0x52f406)  // 17 instructions; "lineAddProviderA"
DEAD(0x52f451)  // 17 instructions; "lineAddProviderW"
DEAD(0x52f49c)  // 16 instructions; "lineAddToConference"
DEAD(0x52f4e3)  // 22 instructions; "lineAgentSpecific"
DEAD(0x52f580)  // 17 instructions; "lineBlindTransfer"
DEAD(0x52f5cb)  // 17 instructions; "lineBlindTransferA"
DEAD(0x52f616)  // 17 instructions; "lineBlindTransferW"
DEAD(0x52f6a4)  // 21 instructions; "lineCompleteCall"
DEAD(0x52f6f3)  // 21 instructions; "lineCompleteTransfer"
DEAD(0x52f742)  // 17 instructions; "lineConfigDialog"
DEAD(0x52f78d)  // 17 instructions; "lineConfigDialogA"
DEAD(0x52f7d8)  // 17 instructions; "lineConfigDialogW"
DEAD(0x52f823)  // 23 instructions; "lineConfigDialogEdit"
DEAD(0x52f878)  // 23 instructions; "lineConfigDialogEditA"
DEAD(0x52f8cd)  // 23 instructions; "lineConfigDialogEditW"
DEAD(0x52f922)  // 16 instructions; "lineConfigProvider"
DEAD(0x52f9ac)  // 22 instructions; "lineDevSpecific"
DEAD(0x52f9fe)  // 21 instructions; "lineDevSpecificFeature"
DEAD(0x52fa4d)  // 17 instructions; "lineDial"
DEAD(0x52fa98)  // 17 instructions; "lineDialA"
DEAD(0x52fae3)  // 17 instructions; "lineDialW"
DEAD(0x52fb79)  // 24 instructions; "lineForward"
DEAD(0x52fbd1)  // 24 instructions; "lineForwardA"
DEAD(0x52fc29)  // 24 instructions; "lineForwardW"
DEAD(0x52fc81)  // 24 instructions; "lineGatherDigits"
DEAD(0x52fcd9)  // 24 instructions; "lineGatherDigitsA"
DEAD(0x52fd31)  // 24 instructions; "lineGatherDigitsW"
DEAD(0x52fd89)  // 21 instructions; "lineGenerateDigits"
DEAD(0x52fdd8)  // 21 instructions; "lineGenerateDigitsA"
DEAD(0x52fe27)  // 21 instructions; "lineGenerateDigitsW"
DEAD(0x52fe76)  // 22 instructions; "lineGenerateTone"
DEAD(0x52fec8)  // 23 instructions; "lineGetAddressCaps"
DEAD(0x52ff1d)  // 23 instructions; "lineGetAddressCapsA"
DEAD(0x52ff72)  // 23 instructions; "lineGetAddressCapsW"
DEAD(0x5300bd)  // 17 instructions; "lineGetAddressStatus"
DEAD(0x530108)  // 17 instructions; "lineGetAddressStatusA"
DEAD(0x530153)  // 17 instructions; "lineGetAddressStatusW"
DEAD(0x53019e)  // 17 instructions; "lineGetAgentActivityListA"
DEAD(0x5301e9)  // 17 instructions; "lineGetAgentActivityListW"
DEAD(0x530234)  // 22 instructions; "lineGetAgentCapsA"
DEAD(0x530286)  // 22 instructions; "lineGetAgentCapsW"
DEAD(0x5302d8)  // 17 instructions; "lineGetAgentGroupListA"
DEAD(0x530323)  // 17 instructions; "lineGetAgentGroupListW"
DEAD(0x53036e)  // 17 instructions; "lineGetAgentStatusA"
DEAD(0x5303b9)  // 17 instructions; "lineGetAgentStatusW"
DEAD(0x530404)  // 23 instructions; "lineGetAppPriority"
DEAD(0x530459)  // 23 instructions; "lineGetAppPriorityA"
DEAD(0x5304ae)  // 23 instructions; "lineGetAppPriorityW"
DEAD(0x530503)  // 16 instructions; "lineGetCallInfo"
DEAD(0x53054a)  // 16 instructions; "lineGetCallInfoA"
DEAD(0x530591)  // 16 instructions; "lineGetCallInfoW"
DEAD(0x5305d8)  // 16 instructions; "lineGetCallStatus"
DEAD(0x53061f)  // 16 instructions; "lineGetConfRelatedCalls"
DEAD(0x530666)  // 17 instructions; "lineGetCountry"
DEAD(0x5306b1)  // 17 instructions; "lineGetCountryA"
DEAD(0x5306fc)  // 17 instructions; "lineGetCountryW"
DEAD(0x530799)  // 22 instructions; "lineGetDevCapsA"
DEAD(0x5307eb)  // 22 instructions; "lineGetDevCapsW"
DEAD(0x530888)  // 17 instructions; "lineGetDevConfigA"
DEAD(0x5308d3)  // 17 instructions; "lineGetDevConfigW"
DEAD(0x53091e)  // 21 instructions; "lineGetNewCalls"
DEAD(0x53096d)  // 17 instructions; "lineGetIcon"
DEAD(0x5309b8)  // 17 instructions; "lineGetIconA"
DEAD(0x530a03)  // 17 instructions; "lineGetIconW"
DEAD(0x530aa3)  // 23 instructions; "lineGetIDA"
DEAD(0x530af8)  // 23 instructions; "lineGetIDW"
DEAD(0x530b94)  // 16 instructions; "lineGetLineDevStatusA"
DEAD(0x530bdb)  // 16 instructions; "lineGetLineDevStatusW"
DEAD(0x530c22)  // 17 instructions; "lineGetMessage"
DEAD(0x530c6d)  // 17 instructions; "lineGetNumRings"
DEAD(0x530cb8)  // 16 instructions; "lineGetProviderList"
DEAD(0x530cff)  // 16 instructions; "lineGetProviderListA"
DEAD(0x530d46)  // 16 instructions; "lineGetProviderListW"
DEAD(0x530d8d)  // 17 instructions; "lineGetRequest"
DEAD(0x530dd8)  // 17 instructions; "lineGetRequestA"
DEAD(0x530e23)  // 17 instructions; "lineGetRequestW"
DEAD(0x530e6e)  // 17 instructions; "lineGetStatusMessages"
DEAD(0x530eb9)  // 17 instructions; "lineGetTranslateCaps"
DEAD(0x530f04)  // 17 instructions; "lineGetTranslateCapsA"
DEAD(0x530f4f)  // 17 instructions; "lineGetTranslateCapsW"
DEAD(0x530f9a)  // 17 instructions; "lineHandoff"
DEAD(0x530fe5)  // 17 instructions; "lineHandoffA"
DEAD(0x531030)  // 17 instructions; "lineHandoffW"
DEAD(0x53107b)  // 15 instructions; "lineHold"
DEAD(0x531110)  // 24 instructions; "lineInitializeExA"
DEAD(0x531168)  // 24 instructions; "lineInitializeExW"
DEAD(0x531212)  // 22 instructions; "lineMakeCallA"
DEAD(0x531264)  // 22 instructions; "lineMakeCallW"
DEAD(0x5312b6)  // 16 instructions; "lineMonitorDigits"
DEAD(0x5312fd)  // 16 instructions; "lineMonitorMedia"
DEAD(0x531344)  // 17 instructions; "lineMonitorTones"
DEAD(0x5313e4)  // 23 instructions; "lineNegotiateExtVersion"
DEAD(0x531497)  // 26 instructions; "lineOpenA"
DEAD(0x5314f5)  // 26 instructions; "lineOpenW"
DEAD(0x531553)  // 21 instructions; "linePark"
DEAD(0x5315a2)  // 21 instructions; "lineParkA"
DEAD(0x5315f1)  // 21 instructions; "lineParkW"
DEAD(0x531640)  // 22 instructions; "linePickup"
DEAD(0x531692)  // 22 instructions; "linePickupA"
DEAD(0x5316e4)  // 22 instructions; "linePickupW"
DEAD(0x531736)  // 17 instructions; "linePrepareAddToConference"
DEAD(0x531781)  // 17 instructions; "linePrepareAddToConferenceA"
DEAD(0x5317cc)  // 17 instructions; "linePrepareAddToConferenceW"
DEAD(0x531817)  // 23 instructions; "lineProxyMessage"
DEAD(0x53186c)  // 17 instructions; "lineProxyResponse"
DEAD(0x5318b7)  // 17 instructions; "lineRedirect"
DEAD(0x531902)  // 17 instructions; "lineRedirectA"
DEAD(0x53194d)  // 17 instructions; "lineRedirectW"
DEAD(0x531998)  // 21 instructions; "lineRegisterRequestRecipient"
DEAD(0x5319e7)  // 15 instructions; "lineReleaseUserUserInfo"
DEAD(0x531a2a)  // 15 instructions; "lineRemoveFromConference"
DEAD(0x531a6d)  // 16 instructions; "lineRemoveProvider"
DEAD(0x531ab4)  // 15 instructions; "lineSecureCall"
DEAD(0x531af7)  // 17 instructions; "lineSendUserUserInfo"
DEAD(0x531b42)  // 17 instructions; "lineSetAgentActivity"
DEAD(0x531b8d)  // 17 instructions; "lineSetAgentGroup"
DEAD(0x531bd8)  // 21 instructions; "lineSetAgentState"
DEAD(0x531c27)  // 23 instructions; "lineSetAppPriority"
DEAD(0x531c7c)  // 23 instructions; "lineSetAppPriorityA"
DEAD(0x531cd1)  // 23 instructions; "lineSetAppPriorityW"
DEAD(0x531d26)  // 16 instructions; "lineSetAppSpecific"
DEAD(0x531d6d)  // 17 instructions; "lineSetCallData"
DEAD(0x531db8)  // 22 instructions; "lineSetCallParams"
DEAD(0x531e0a)  // 16 instructions; "lineSetCallPrivilege"
DEAD(0x531e51)  // 22 instructions; "lineSetCallQualityOfService"
DEAD(0x531ea3)  // 16 instructions; "lineSetCallTreatment"
DEAD(0x531eea)  // 16 instructions; "lineSetCurrentLocation"
DEAD(0x531f80)  // 21 instructions; "lineSetDevConfigA"
DEAD(0x531fcf)  // 21 instructions; "lineSetDevConfigW"
DEAD(0x53201e)  // 17 instructions; "lineSetLineDevStatus"
DEAD(0x532069)  // 29 instructions; "lineSetMediaControl"
DEAD(0x5320d0)  // 16 instructions; "lineSetMediaMode"
DEAD(0x532117)  // 17 instructions; "lineSetNumRings"
DEAD(0x5321ad)  // 24 instructions; "lineSetTerminal"
DEAD(0x532205)  // 21 instructions; "lineSetTollList"
DEAD(0x532254)  // 21 instructions; "lineSetTollListA"
DEAD(0x5322a3)  // 21 instructions; "lineSetTollListW"
DEAD(0x5322f2)  // 23 instructions; "lineSetupConference"
DEAD(0x532347)  // 23 instructions; "lineSetupConferenceA"
DEAD(0x53239c)  // 23 instructions; "lineSetupConferenceW"
DEAD(0x5323f1)  // 17 instructions; "lineSetupTransfer"
DEAD(0x53243c)  // 17 instructions; "lineSetupTransferA"
DEAD(0x532487)  // 17 instructions; "lineSetupTransferW"
DEAD(0x532515)  // 16 instructions; "lineSwapHold"
DEAD(0x53255c)  // 24 instructions; "lineTranslateAddress"
DEAD(0x5325b4)  // 24 instructions; "lineTranslateAddressA"
DEAD(0x53260c)  // 24 instructions; "lineTranslateAddressW"
DEAD(0x532664)  // 22 instructions; "lineTranslateDialog"
DEAD(0x5326b6)  // 22 instructions; "lineTranslateDialogA"
DEAD(0x532708)  // 22 instructions; "lineTranslateDialogW"
DEAD(0x53275a)  // 16 instructions; "lineUncompleteCall"
DEAD(0x5327a1)  // 15 instructions; "lineUnhold"
DEAD(0x5327e4)  // 21 instructions; "lineUnpark"
DEAD(0x532833)  // 21 instructions; "lineUnparkA"
DEAD(0x532882)  // 21 instructions; "lineUnparkW"
DEAD(0x5328d1)  // 15 instructions; "phoneClose"
DEAD(0x532914)  // 17 instructions; "phoneConfigDialog"
DEAD(0x53295f)  // 17 instructions; "phoneConfigDialogA"
DEAD(0x5329aa)  // 17 instructions; "phoneConfigDialogW"
DEAD(0x5329f5)  // 17 instructions; "phoneDevSpecific"
DEAD(0x532a40)  // 17 instructions; "phoneGetButtonInfo"
DEAD(0x532a8b)  // 17 instructions; "phoneGetButtonInfoA"
DEAD(0x532ad6)  // 17 instructions; "phoneGetButtonInfoW"
DEAD(0x532b21)  // 21 instructions; "phoneGetData"
DEAD(0x532b70)  // 22 instructions; "phoneGetDevCaps"
DEAD(0x532bc2)  // 22 instructions; "phoneGetDevCapsA"
DEAD(0x532c14)  // 22 instructions; "phoneGetDevCapsW"
DEAD(0x532c66)  // 16 instructions; "phoneGetDisplay"
DEAD(0x532cad)  // 17 instructions; "phoneGetGain"
DEAD(0x532cf8)  // 16 instructions; "phoneGetHookSwitch"
DEAD(0x532d3f)  // 17 instructions; "phoneGetIcon"
DEAD(0x532d8a)  // 17 instructions; "phoneGetIconA"
DEAD(0x532dd5)  // 17 instructions; "phoneGetIconW"
DEAD(0x532e20)  // 17 instructions; "phoneGetID"
DEAD(0x532e6b)  // 17 instructions; "phoneGetIDA"
DEAD(0x532eb6)  // 17 instructions; "phoneGetIDW"
DEAD(0x532f01)  // 17 instructions; "phoneGetLamp"
DEAD(0x532f4c)  // 17 instructions; "phoneGetMessage"
DEAD(0x532f97)  // 17 instructions; "phoneGetRing"
DEAD(0x532fe2)  // 16 instructions; "phoneGetStatus"
DEAD(0x533029)  // 16 instructions; "phoneGetStatusA"
DEAD(0x533070)  // 16 instructions; "phoneGetStatusW"
DEAD(0x5330b7)  // 21 instructions; "phoneGetStatusMessages"
DEAD(0x533106)  // 17 instructions; "phoneGetVolume"
DEAD(0x533151)  // 22 instructions; "phoneInitialize"
DEAD(0x5331a3)  // 24 instructions; "phoneInitializeExA"
DEAD(0x5331fb)  // 24 instructions; "phoneInitializeExW"
DEAD(0x533253)  // 23 instructions; "phoneNegotiateAPIVersion"
DEAD(0x5332a8)  // 23 instructions; "phoneNegotiateExtVersion"
DEAD(0x5332fd)  // 24 instructions; "phoneOpen"
DEAD(0x533355)  // 17 instructions; "phoneSetButtonInfo"
DEAD(0x5333a0)  // 17 instructions; "phoneSetButtonInfoA"
DEAD(0x5333eb)  // 17 instructions; "phoneSetButtonInfoW"
DEAD(0x533436)  // 21 instructions; "phoneSetData"
DEAD(0x533485)  // 22 instructions; "phoneSetDisplay"
DEAD(0x5334d7)  // 17 instructions; "phoneSetGain"
DEAD(0x533522)  // 17 instructions; "phoneSetHookSwitch"
DEAD(0x53356d)  // 17 instructions; "phoneSetLamp"
DEAD(0x5335b8)  // 17 instructions; "phoneSetRing"
DEAD(0x533603)  // 21 instructions; "phoneSetStatusMessages"
DEAD(0x533652)  // 17 instructions; "phoneSetVolume"
DEAD(0x53369d)  // 15 instructions; "phoneShutdown"
DEAD(0x5336e0)  // 16 instructions; "tapiGetLocationInfo"
DEAD(0x533727)  // 16 instructions; "tapiGetLocationInfoA"
DEAD(0x53376e)  // 16 instructions; "tapiGetLocationInfoW"
DEAD(0x5337b5)  // 16 instructions; "tapiRequestDrop"
DEAD(0x5337fc)  // 21 instructions; "tapiRequestMakeCall"
DEAD(0x53384b)  // 21 instructions; "tapiRequestMakeCallA"
DEAD(0x53389a)  // 21 instructions; "tapiRequestMakeCallW"
DEAD(0x5338e9)  // 27 instructions; "tapiRequestMediaCall"
DEAD(0x53394a)  // 27 instructions; "tapiRequestMediaCallA"
DEAD(0x5339ab)  // 27 instructions; "tapiRequestMediaCallW"
DEAD(0x533a0c)  // 1 instructions; MoveFileA
DEAD(0x533a12)  // 1 instructions; GetFileType
DEAD(0x533a18)  // 1 instructions; FlushFileBuffers
DEAD(0x533a1e)  // 1 instructions; SetUnhandledExceptionFilter
DEAD(0x533a24)  // 1 instructions; VirtualQuery
DEAD(0x533a2a)  // 1 instructions; MultiByteToWideChar
DEAD(0x533a30)  // 1 instructions; GetModuleFileNameW
DEAD(0x533a36)  // 1 instructions; TlsFree
DEAD(0x533a3c)  // 1 instructions; TlsSetValue
DEAD(0x533a42)  // 1 instructions; TlsAlloc
DEAD(0x533a48)  // 1 instructions; TlsGetValue
DEAD(0x533a4e)  // 1 instructions; GetStdHandle
DEAD(0x533a54)  // 1 instructions; SetStdHandle
DEAD(0x533a5a)  // 1 instructions; WideCharToMultiByte
DEAD(0x533a60)  // 1 instructions; SetCommState
DEAD(0x533a66)  // 1 instructions; GetCommState
DEAD(0x533a6c)  // 1 instructions; SetCommTimeouts
DEAD(0x533a72)  // 1 instructions; GetCommModemStatus
DEAD(0x533a78)  // 1 instructions; EscapeCommFunction
DEAD(0x533a7e)  // 1 instructions; PurgeComm
DEAD(0x533a84)  // 1 instructions; GetOverlappedResult
DEAD(0x533a8a)  // 1 instructions; GetKeyboardType
DEAD(0x533a90)  // 1 instructions; GetTimeZoneInformation
DEAD(0x533a96)  // 1 instructions; GetLocalTime
DEAD(0x533a9c)  // 1 instructions; GetCurrentProcessId
DEAD(0x533aa2)  // 1 instructions; WaitForSingleObject
DEAD(0x533aa8)  // 1 instructions; GetSystemInfo
DEAD(0x533aae)  // 1 instructions; GetCommandLineW
DEAD(0x533ab4)  // 1 instructions; GetCommandLineA
DEAD(0x533aba)  // 1 instructions; GetVersion
DEAD(0x533ac0)  // 1 instructions; GetEnvironmentStrings
DEAD(0x533ac6)  // 1 instructions; keybd_event
DEAD(0x533acc)  // 1 instructions; GetKeyState
DEAD(0x533ad2)  // 1 instructions; PostThreadMessageA
DEAD(0x533ad8)  // 1 instructions; GetCommProperties
DEAD(0x533ade)  // 1 instructions; wsprintfA
DEAD(0x533ae4)  // 1 instructions; GetDriveTypeA
DEAD(0x533aea)  // 1 instructions; CreateMutexA
DEAD(0x533af0)  // 1 instructions; SetEnvironmentVariableA
DEAD(0x533af6)  // 1 instructions; SetConsoleMode
DEAD(0x533afc)  // 1 instructions; GetConsoleMode
DEAD(0x533b02)  // 1 instructions; ReadConsoleInputA
DEAD(0x533b08)  // 1 instructions; PeekConsoleInputA
DEAD(0x533b0e)  // 1 instructions; GetFileAttributesA
DEAD(0x533b14)  // 1 instructions; RemoveDirectoryA
DEAD(0x533b1a)  // 1 instructions; RegFlushKey
DEAD(0x533b20)  // 1 instructions; ShowWindow
DEAD(0x533b26)  // 1 instructions; DeleteCriticalSection
DEAD(0x533b2c)  // 1 instructions; InitializeCriticalSection
DEAD(0x533b32)  // 1 instructions; CreateDirectoryA
DEAD(0x533b38)  // 1 instructions; SetEndOfFile
DEAD(0x533b3e)  // 1 instructions; UnmapViewOfFile
DEAD(0x533b44)  // 1 instructions; SetFilePointer
DEAD(0x533b4a)  // 1 instructions; WriteFile
DEAD(0x533b50)  // 1 instructions; ReadFile
DEAD(0x533b56)  // 1 instructions; MapViewOfFile
DEAD(0x533b5c)  // 1 instructions; CreateFileMappingA
DEAD(0x533b62)  // 1 instructions; GetFileSize
DEAD(0x533b68)  // 1 instructions; CreateFileA
DEAD(0x533b6e)  // 1 instructions; GetCurrentDirectoryA
DEAD(0x533b74)  // 1 instructions; SetCurrentDirectoryA
DEAD(0x533b7a)  // 1 instructions; GetDiskFreeSpaceA
DEAD(0x533b80)  // 1 instructions; GetLogicalDriveStringsA
DEAD(0x533b86)  // 1 instructions; GetLogicalDrives
DEAD(0x533b8c)  // 1 instructions; CharUpperA
DEAD(0x533b92)  // 1 instructions; GetVolumeInformationA
DEAD(0x533b98)  // 1 instructions; GetFullPathNameA
DEAD(0x533b9e)  // 1 instructions; DeleteFileA
DEAD(0x533ba4)  // 1 instructions; VirtualFree
DEAD(0x533baa)  // 1 instructions; VirtualAlloc
DEAD(0x533bb0)  // 1 instructions; SetLastError
DEAD(0x533bb6)  // 1 instructions; PulseEvent
DEAD(0x533bbc)  // 1 instructions; ResetEvent
DEAD(0x533bc2)  // 1 instructions; SetEvent
DEAD(0x533bc8)  // 1 instructions; ExitThread
DEAD(0x533bce)  // 1 instructions; SetThreadPriority
DEAD(0x533bd4)  // 1 instructions; GetThreadPriority
DEAD(0x533bda)  // 1 instructions; TerminateThread
DEAD(0x533be0)  // 1 instructions; SleepEx
DEAD(0x533be6)  // 1 instructions; ResumeThread
DEAD(0x533bec)  // 1 instructions; CreateThread
DEAD(0x533bf2)  // 1 instructions; DuplicateHandle
DEAD(0x533bf8)  // 1 instructions; GetCurrentThread
DEAD(0x533bfe)  // 1 instructions; GetCurrentProcess
DEAD(0x533c04)  // 1 instructions; GetCurrentThreadId
DEAD(0x533c0a)  // 1 instructions; WaitForMultipleObjectsEx
DEAD(0x533c10)  // 1 instructions; SetWindowPos
DEAD(0x533c16)  // 1 instructions; GetWindowLongA
DEAD(0x533c1c)  // 1 instructions; RegisterClassA
DEAD(0x533c22)  // 1 instructions; GetStockObject
DEAD(0x533c28)  // 1 instructions; LoadIconA
DEAD(0x533c2e)  // 1 instructions; IsIconic
DEAD(0x533c34)  // 1 instructions; DestroyWindow
DEAD(0x533c3a)  // 1 instructions; DispatchMessageA
DEAD(0x533c40)  // 1 instructions; TranslateMessage
DEAD(0x533c46)  // 1 instructions; GetMessageA
DEAD(0x533c4c)  // 1 instructions; SetActiveWindow
DEAD(0x533c52)  // 1 instructions; CreateWindowExA
DEAD(0x533c58)  // 1 instructions; AdjustWindowRect
DEAD(0x533c5e)  // 1 instructions; DefWindowProcA
DEAD(0x533c64)  // 1 instructions; EndPaint
DEAD(0x533c6a)  // 1 instructions; BeginPaint
DEAD(0x533c70)  // 1 instructions; SetForegroundWindow
DEAD(0x533c76)  // 1 instructions; GetClientRect
DEAD(0x533c7c)  // 1 instructions; SystemParametersInfoA
DEAD(0x533c82)  // 1 instructions; PostQuitMessage
DEAD(0x533c88)  // 1 instructions; UnhookWindowsHookEx
DEAD(0x533c8e)  // 1 instructions; SetWindowsHookExA
DEAD(0x533c94)  // 1 instructions; CallNextHookEx
DEAD(0x533c9a)  // 1 instructions; SetCursorPos
DEAD(0x533ca0)  // 1 instructions; ClientToScreen
DEAD(0x533ca6)  // 1 instructions; PostMessageA
DEAD(0x533cac)  // 1 instructions; SendNotifyMessageA
DEAD(0x533cb2)  // 1 instructions; GetVersionExA
DEAD(0x533cb8)  // 1 instructions; GlobalMemoryStatus
DEAD(0x533cbe)  // 1 instructions; GetDeviceCaps
DEAD(0x533cc4)  // 1 instructions; GetWindowDC
DEAD(0x533cca)  // 1 instructions; GetDesktopWindow
DEAD(0x533cd0)  // 1 instructions; GetWindowsDirectoryA
DEAD(0x533cd6)  // 1 instructions; GetLastError
DEAD(0x533cdc)  // 1 instructions; RegCloseKey
DEAD(0x533ce2)  // 1 instructions; RegQueryValueExA
DEAD(0x533ce8)  // 1 instructions; RegOpenKeyExA
DEAD(0x533cee)  // 1 instructions; UnhandledExceptionFilter
DEAD(0x533cf4)  // 1 instructions; SetErrorMode
DEAD(0x533cfa)  // 1 instructions; FindWindowA
DEAD(0x533d00)  // 1 instructions; FreeLibrary
DEAD(0x533d06)  // 1 instructions; GetProcAddress
DEAD(0x533d0c)  // 1 instructions; LoadLibraryA
DEAD(0x533d12)  // 1 instructions; GetSystemMetrics
DEAD(0x533d18)  // 1 instructions; GetTickCount
DEAD(0x533d1e)  // 1 instructions; GetModuleFileNameA
DEAD(0x533d24)  // 1 instructions; CompareFileTime
DEAD(0x533d2a)  // 1 instructions; _lclose
DEAD(0x533d30)  // 1 instructions; GetFileTime
DEAD(0x533d36)  // 1 instructions; _lopen
DEAD(0x533d3c)  // 1 instructions; GetAsyncKeyState
DEAD(0x533d42)  // 1 instructions; CoCreateGuid
DEAD(0x533d48)  // 1 instructions; FindClose
DEAD(0x533d4e)  // 1 instructions; FindNextFileA
DEAD(0x533d54)  // 1 instructions; FindFirstFileA
DEAD(0x533d5a)  // 1 instructions; SetCursor
DEAD(0x533d60)  // 1 instructions; CreateEventA
DEAD(0x533d66)  // 1 instructions; SwapMouseButton
DEAD(0x533d6c)  // 1 instructions; CloseHandle
DEAD(0x533d72)  // 1 instructions; EnterCriticalSection
DEAD(0x533d78)  // 1 instructions; ShellExecuteA
DEAD(0x533d7e)  // 1 instructions; GetModuleHandleA
DEAD(0x533d84)  // 1 instructions; SetFileAttributesA
DEAD(0x533d8a)  // 1 instructions; ExitProcess
DEAD(0x533d90)  // 1 instructions; LeaveCriticalSection
DEAD(0x533d96)  // 1 instructions; MessageBoxA
DEAD(0x533d9c)  // 1 instructions; ShowCursor
DEAD(0x533da2)  // 1 instructions; DebugBreak
DEAD(0x533da8)  // 1 instructions; OutputDebugStringA
DEAD(0x533dae)  // 1 instructions; CharUpperBuffA
DEAD(0x533db4)  // 1 instructions; WriteConsoleA
DEAD(0x533dba)  // 1 instructions; SetConsoleCtrlHandler
DEAD(0x533dc0)  // 1 instructions; GetCPInfo
DEAD(0x533dc6)  // 1 instructions; GetOEMCP
DEAD(0x533dcc)  // 1 instructions; GetACP
DEAD(0x533dd2)  // 1 instructions; SetEnvironmentVariableW
DEAD(0x533dd8)  // 1 instructions; FreeEnvironmentStringsA
DEAD(0x533e62)  // 1 instructions; timeBeginPeriod
DEAD(0x533e68)  // 1 instructions; timeGetDevCaps
DEAD(0x533e6e)  // 1 instructions; timeKillEvent
DEAD(0x533e74)  // 1 instructions; timeEndPeriod
DEAD(0x533e7a)  // 1 instructions; timeSetEvent
DEAD(0x533e80)  // 1 instructions; DirectDrawEnumerateA
DEAD(0x533eb6)  // 1 instructions; DdeQueryStringA
DEAD(0x533ebc)  // 1 instructions; DdeClientTransaction
DEAD(0x533ec2)  // 1 instructions; DdeGetLastError
DEAD(0x533ec8)  // 1 instructions; DdeConnect
DEAD(0x533ece)  // 1 instructions; DdeCreateStringHandleA
DEAD(0x533ed4)  // 1 instructions; DdeUninitialize
DEAD(0x533eda)  // 1 instructions; DdeInitializeA
DEAD(0x4ff624)
DEAD(0x51f320)
DEAD(0x5269d4)
DEAD(0x526aeb)
DEAD(0x50ea68)
DEAD(0x50ea85)
DEAD(0x50eaa7)
DEAD(0x4f1ea0)
DEAD(0x4f21c0)
DEAD(0x4f20b0)
DEAD(0x4df5c0)
DEAD(0x51f272)
DEAD(0x503280)
DEAD(0x503281)
DEAD(0x50356c)
DEAD(0x50356d)
// Data decoded as code, found reading them 2026-11-03: the fields' blank buffers, shape names, a table.
DEAD(0x457260)
DEAD(0x4576e0)
DEAD(0x469020)
DEAD(0x49fdd0)
DEAD(0x4b6a10)
DEAD(0x4a78b6)
