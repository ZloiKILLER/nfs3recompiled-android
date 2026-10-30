#ifndef NFS3HP_PORT_CONV_H
#define NFS3HP_PORT_CONV_H

#include "port.h"

/* Functions of the game's that use the x87, rewritten from their code by
 * tools/decomp.py as C++ of the port's own: the registers, the flags and the
 * stack as locals, the x87 as a local stack of doubles (a store rounds to a
 * float where the game stores one), its sums, products and quotients through
 * the emulated FPU's operations, so they round as the control word asks: a
 * race's single precision as the original's.  The calls the code makes
 * go through the guest as they were.  tools/game_checks holds each against
 * the game's code (conv.inc).
 *
 * disassemble_nfs3hp.py binds each address to the adapter of that name here
 * (PORT_CONV_NATIVES). */
namespace nfs3hp
{
namespace port
{
namespace conv
{

void fn49fe70(App* app, CPU& cpu);  // 0x49fe70
void fn4a00d0(App* app, CPU& cpu);  // 0x4a00d0
void fn4a0440(App* app, CPU& cpu);  // 0x4a0440
void fn4a0980(App* app, CPU& cpu);  // 0x4a0980
void fn4a0a10(App* app, CPU& cpu);  // 0x4a0a10
void fn4a0d20(App* app, CPU& cpu);  // 0x4a0d20
void fn4a0f30(App* app, CPU& cpu);  // 0x4a0f30
void fn4a1090(App* app, CPU& cpu);  // 0x4a1090
void fn4a20c0(App* app, CPU& cpu);  // 0x4a20c0
void fn4a2630(App* app, CPU& cpu);  // 0x4a2630
void fn4a2830(App* app, CPU& cpu);  // 0x4a2830
void fn4a2c60(App* app, CPU& cpu);  // 0x4a2c60
void fn4a2e20(App* app, CPU& cpu);  // 0x4a2e20
// @@DECLS@@

}  // namespace conv
}  // namespace port
}  // namespace nfs3hp

#endif
