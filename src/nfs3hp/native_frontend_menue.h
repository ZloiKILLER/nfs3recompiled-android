#ifndef NFS3HP_NATIVE_FRONTEND_MENUE_H
#define NFS3HP_NATIVE_FRONTEND_MENUE_H

#include "native_frontend.h"
#include "native_guest.h"
#include <cstdint>
#include <cstring>

/* native_frontend_menue.cpp's typed functions (the small things the menus' widgets lean on: the
 * sheets' text, the laps' option, the language's text, the menus' pointer and drawing), which
 * other files call as C++ (guest::direct), and how the game's code calls each by address.  Each calls the game's other functions by address on
 * `cpu` and throws fe::Terminated when the game stopped in one, as a call by address does; its result is
 * what the game's function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace menue
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x4cedd0 (stack: the speed, the buffer): a speed in metres per second as text, in the player's units.
reg32 metricText(App* app, CPU& cpu, float speed, reg32 buffer);
// The game's 0x4cee40: a time (64ths of a second) as minutes, seconds and hundredths into buffer.
reg32 timeText(App* app, CPU& cpu, reg32 time, reg32 buffer);
// The game's 0x4cf160: 2, 4 or 8 laps as their place in the laps' list.
reg32 lapsToOption(App* app, CPU& cpu, reg32 laps);
// The game's 0x4cf190: the laps of a place in the laps' list.
reg32 optionToLaps(App* app, CPU& cpu, reg32 option);
// The game's 0x4d1880: the language's text block given back.
reg32 freeLanguageTexts(App* app, CPU& cpu, reg32 eax);
// The game's 0x4d7560: an art record's tiles drawn at (x, y).
reg32 drawArtTiles(App* app, CPU& cpu, reg32 art, std::int32_t x, std::int32_t y);
// The game's 0x4d7cc0 (stdcall, all on the stack): the fade of a bouncing picture's corner at (x, y).
reg32 cornerFade(App* app, CPU& cpu, float x, float y, reg32 alpha);
// The game's 0x4d7d90 (stdcall, fromV on the stack): a stretch of a texture drawn scaled, its corners fading.
reg32 drawBouncer(App* app, CPU& cpu, reg32 texture, std::int32_t x, std::int32_t y, std::int32_t fromU,
                  std::int32_t fromV, std::int32_t width, std::int32_t height, float scale, reg32 colour);
// The game's 0x4d88c0 (stdcall, x2 on): the colour picker's shaded triangle, handed to the 2D layer's.
reg32 shadedTriangle(App* app, CPU& cpu, reg32 x0, reg32 y0, reg32 x1, reg32 y1, reg32 x2, reg32 y2, reg32 colour0,
                     reg32 colour1, reg32 colour2);
// The game's 0x4d9180: a colour's red, green and blue scaled by scale/256.
reg32 scaleColour(App* app, CPU& cpu, reg32 colour, std::int32_t scale);
// The game's 0x4daa10: the pointer's shapes out of their file, made textures, with their hot spots.
reg32 loadPointer(App* app, CPU& cpu, reg32 file);
// The game's 0x4dab10: the shown pointer's hot spot into [xAt] and [yAt].
reg32 pointerHotSpot(App* app, CPU& cpu, reg32 xAt, reg32 yAt);

/* A float as the stack word it is pushed as. */
inline reg32 stackWord(float value)
{
    reg32 bits;
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
