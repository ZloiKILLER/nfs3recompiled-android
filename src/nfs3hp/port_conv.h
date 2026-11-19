#ifndef NFS3HP_PORT_CONV_H
#define NFS3HP_PORT_CONV_H

#include "native_guest.h"
#include "port.h"

/* Functions of the game's that use the x87, rewritten from their code by
 * tools/decomp.py as C++ of the port's own: the registers, the flags and the
 * stack as locals, the x87 as a local stack of doubles (a store rounds to a
 * float where the game stores one), its sums, products and quotients through
 * the emulated FPU's operations, so they round as the control word asks: a
 * race's single precision as the original's.  The calls the code makes
 * go through the guest as they were.
 *
 * The game's code calls each by the name it has here. */
namespace nfs3hp
{
namespace port
{
namespace conv
{

nfs::u32 fn49fe70(App* app, ThreadState& ts, nfs::Args& args);  // 0x49fe70
nfs::u32 fn4a00d0(App* app, ThreadState& ts, nfs::Args& args);  // 0x4a00d0
nfs::u32 fn4a0440(App* app, ThreadState& ts, nfs::Args& args);  // 0x4a0440
nfs::u32 fn4a0980(App* app, ThreadState& ts, nfs::Args& args);  // 0x4a0980
nfs::u32 fn4a0a10(App* app, ThreadState& ts, nfs::Args& args);  // 0x4a0a10
nfs::u32 fn4a0d20(App* app, ThreadState& ts, nfs::Args& args);  // 0x4a0d20
nfs::u32 fn4a0f30(App* app, ThreadState& ts, nfs::Args& args);  // 0x4a0f30
nfs::u32 fn4a1090(App* app, ThreadState& ts, nfs::Args& args);  // 0x4a1090
nfs::u32 fn4a20c0(App* app, ThreadState& ts, nfs::Args& args);  // 0x4a20c0
nfs::u32 fn4a2630(App* app, ThreadState& ts, nfs::Args& args);  // 0x4a2630
nfs::u32 fn4a2830(App* app, ThreadState& ts, nfs::Args& args);  // 0x4a2830
nfs::u32 fn4a2c60(App* app, ThreadState& ts, nfs::Args& args);  // 0x4a2c60
nfs::u32 fn4a2e20(App* app, ThreadState& ts, nfs::Args& args);  // 0x4a2e20
// @@DECLS@@

/* The typed functions other files call as C++ (guest::direct): the registers the game's code hands
 * each as parameters, the words it pushes at `words` (where the pushes put them), what it leaves in
 * eax back -- and, for one that returns a float, the float.  Nullopt when the game stopped. */
struct Left
{
    u32 eax;
    double st0;  // the float it leaves on the x87's stack, if it returns one
};

// The game's 0x49fe70, 0x4a00d0 (a float), 0x4a0440 (a float): a car's node taken, its height over the road, its
// lowest point.
std::optional<Left> carNodeNow(App* app, ThreadState& ts, u32 car);
std::optional<Left> carHeightOverRoad(App* app, ThreadState& ts, u32 car);
std::optional<Left> carLowestPoint(App* app, ThreadState& ts, u32 car);
// The game's 0x4a0980, 0x4a0a10: a car's frame from the road, `facesBack` whether it faces back.
std::optional<Left> frameFromRoad(App* app, ThreadState& ts, u32 car, u32 facesBack);
std::optional<Left> lastFrameFromRoad(App* app, ThreadState& ts, u32 car, u32 facesBack);
// The game's 0x4a0f30: a car's frame followed on `up`, `upright` how upright it is (the stack word).
std::optional<Left> followFrame(App* app, ThreadState& ts, u32 car, u32 up, u32 upright);
// The game's 0x4a1090, 0x4a20c0: a car's frame eased, its spin clamped.
std::optional<Left> easeFrame(App* app, ThreadState& ts, u32 car);
std::optional<Left> clampSpin(App* app, ThreadState& ts, u32 car);
// The game's 0x4a2e20: a car's spin put on its frame in `steps` a second.
std::optional<Left> spinStep(App* app, ThreadState& ts, u32 car, u32 steps);

// The game's 0x4a0d20: car's handling scaled for the players (`number` its first word), the scale and
// three words at `words` -- where the code pushes them, the four words its stdcall takes.
bool scaleHandling(App* app, ThreadState& ts, u32 car, u32 number, u32 words);

}  // namespace conv
}  // namespace port

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
