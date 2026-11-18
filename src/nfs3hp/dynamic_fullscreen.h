#ifndef NFS3HP_DYNAMIC_FULLSCREEN_H_
#define NFS3HP_DYNAMIC_FULLSCREEN_H_

#include <nfs3hp.h>

/* Dynamic Fullscreen (nfs3hp_main.cpp): a race on the whole screen, its size
 * worked out from the screen the game runs on. */
namespace nfs3hp
{

/* What the settings keep as the screen width ([0x6fbc18]) for Dynamic
 * Fullscreen, in place of a width that differs from one screen to the next. */
constexpr x86::reg32 kDynamicFullscreenWidth = 0xffff;

/* The Graphics menu's name for the mode. */
constexpr const char* kDynamicFullscreenName = "Dynamic Fullscreen";

/* Dynamic Fullscreen put in the Voodoo2 driver's mode table (main). */
void dynamicFullscreenStart(win32::WinApplication* app);

/* sub_4bee80's answer for the saved size (the port's widescreen change). */
x86::reg32 dynamicSavedMode(x86::reg32 found, x86::reg32 width);

/* Whether the driver's mode `mode` (an index into its table) is Dynamic
 * Fullscreen. */
bool dynamicFullscreenMode(x86::reg32 mode);

/* Dynamic Fullscreen measured against the screen again; true when its size
 * changed. */
bool dynamicFullscreenRefresh(win32::WinApplication* app);

}

#endif
