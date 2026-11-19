#ifndef NFS3HP_NATIVE_FRONTEND_TEXT_H
#define NFS3HP_NATIVE_FRONTEND_TEXT_H

#include "native_guest.h"

/* native_frontend_text.cpp's typed functions (a display mode's or a controller's name, the checks
 * of a player's name and of a line typed in), which other files call as C++ (guest::direct), and
 * how the game's code calls each by address.
 *
 * They are written as their code is, register by register: each takes its arguments into the
 * registers the game's function takes them in, runs on `ts` as the code does, and returns what
 * it leaves in eax.  When the game stopped in a call they return at once with ts.terminate set,
 * as the code does -- they do not throw -- and so do their calls by address here. */
namespace nfs3hp
{
namespace frontend
{
namespace text
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x448400: display mode `mode` (counted from 0x7a8)'s name.
u32 displayModeName(App* app, ThreadState& ts, u32 mode);
// The game's 0x43d9f0: controller `controller`'s name, its row of the menu shown in its colour.
u32 controllerName(App* app, ThreadState& ts, u32 controller);
// The game's 0x4721f0: whether `name` will do for a player's: 1 to 10 characters.
u32 nameValid(App* app, ThreadState& ts, u32 name);
// The game's 0x443630: whether the character in al may be in a short name; ah left al + 1.
u32 shortNameCharacter(App* app, ThreadState& ts, u32 character);
// The game's 0x443660: whether `line` is a short name: 0 to 8 of those characters.
u32 shortNameValid(App* app, ThreadState& ts, u32 line);
// The game's 0x443700: whether the language takes `line` as a player's name.
u32 languageNameValid(App* app, ThreadState& ts, u32 line);
// The game's 0x443780: whether the language takes `line` as a file's name.
u32 languageFileNameValid(App* app, ThreadState& ts, u32 line);
// The game's 0x443860: whether `line` will do in mode `mode` (0 a short name, 1 and 2 the language's).
u32 lineAllowed(App* app, ThreadState& ts, u32 line, u32 mode);

}  // namespace text
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
