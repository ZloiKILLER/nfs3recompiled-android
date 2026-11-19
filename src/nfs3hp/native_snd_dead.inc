// EA SND's mixer code that never runs on the port, one DEAD(address) a
// function: native_snd_dead.cpp makes each a stub that stops the game with
// its address.
//
// The mixer's start hands the channels the MMX set of mixing routines, the
// only one the port has (port_snd_fx.cpp, port_snd_mixsetup.cpp), which
// port_snd_mixfuncs.cpp and native_mix.cpp translate.  Left: the block mix
// (0x5006f8), which port_snd_mixer.cpp does itself; the reverb's hooks
// (0x5230dc, 0x523118), whose addresses iSNDwinsetfx still puts in the mixer's
// table but which port_snd_mixer.cpp runs itself, and what of them it runs
// inline (0x522f90, 0x522fd4, 0x52b300); and four that nothing calls
// (0x500f80; 0x522130, 0x522220 and 0x522250, the memory and registry
// queries).  No game data and no native code calls any of these.

DEAD(0x5006f8)
DEAD(0x500f80)
DEAD(0x522130)
DEAD(0x522220)
DEAD(0x522250)
DEAD(0x522f90)
DEAD(0x522fd4)
DEAD(0x5230dc)
DEAD(0x523118)
DEAD(0x52b300)
