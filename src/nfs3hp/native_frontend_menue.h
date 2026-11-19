#ifndef NFS3HP_NATIVE_FRONTEND_MENUE_H
#define NFS3HP_NATIVE_FRONTEND_MENUE_H

#include "native_frontend.h"
#include "native_guest.h"
#include <cstdint>
#include <cstring>

/* native_frontend_menue.cpp's typed functions (the small things the menus' widgets lean on: the
 * sheets' text, the laps' option, the language's text, the menus' pointer and drawing), which
 * other files call as C++ (guest::direct), and how the game's code calls each by address.  Each calls the game's other functions by address on
 * `ts` and throws fe::Terminated when the game stopped in one, as a call by address does; its result is
 * what the game's function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace menue
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x4cedd0 (stack: the speed, the buffer): a speed in metres per second as text, in the player's units.
u32 metricText(App* app, ThreadState& ts, float speed, u32 buffer);
// The game's 0x4cee40: a time (64ths of a second) as minutes, seconds and hundredths into buffer.
u32 timeText(App* app, ThreadState& ts, u32 time, u32 buffer);
// The game's 0x4cf160: 2, 4 or 8 laps as their place in the laps' list.
u32 lapsToOption(App* app, ThreadState& ts, u32 laps);
// The game's 0x4cf190: the laps of a place in the laps' list.
u32 optionToLaps(App* app, ThreadState& ts, u32 option);
// The game's 0x4d1880: the language's text block given back.
u32 freeLanguageTexts(App* app, ThreadState& ts, u32 eax);
// The game's 0x4d7560: an art record's tiles drawn at (x, y).
u32 drawArtTiles(App* app, ThreadState& ts, u32 art, std::int32_t x, std::int32_t y);
// The game's 0x4d7cc0 (stdcall, all on the stack): the fade of a bouncing picture's corner at (x, y).
u32 cornerFade(App* app, ThreadState& ts, float x, float y, u32 alpha);
// The game's 0x4d7d90 (stdcall, fromV on the stack): a stretch of a texture drawn scaled, its corners fading.
u32 drawBouncer(App* app, ThreadState& ts, u32 texture, std::int32_t x, std::int32_t y, std::int32_t fromU,
                  std::int32_t fromV, std::int32_t width, std::int32_t height, float scale, u32 colour);
// The game's 0x4d88c0 (stdcall, x2 on): the colour picker's shaded triangle, handed to the 2D layer's.
u32 shadedTriangle(App* app, ThreadState& ts, u32 x0, u32 y0, u32 x1, u32 y1, u32 x2, u32 y2, u32 colour0,
                     u32 colour1, u32 colour2);
// The game's 0x4d9180: a colour's red, green and blue scaled by scale/256.
u32 scaleColour(App* app, ThreadState& ts, u32 colour, std::int32_t scale);
// The game's 0x4daa10: the pointer's shapes out of their file, made textures, with their hot spots.
u32 loadPointer(App* app, ThreadState& ts, u32 file);
// The game's 0x4dab10: the shown pointer's hot spot into [xAt] and [yAt].
u32 pointerHotSpot(App* app, ThreadState& ts, u32 xAt, u32 yAt);

/* A float as the stack word it is pushed as. */
inline u32 stackWord(float value)
{
    u32 bits;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}

}  // namespace menue
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
