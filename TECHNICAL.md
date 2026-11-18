# NFS III: Hot Pursuit — Android: technical details

An Android port of **Need for Speed III: Hot Pursuit**. The original 1998 Win32
x86 game is C++ that runs on the phone's own processor, with no x86 emulated:
it began as a static recompilation and was rewritten from there until every
function is the port's own (see How the port is made). What it asked of Windows
-- files, threads, a window, DirectDraw, DirectSound, DirectInput, the network
-- the port answers itself on top of SDL3 and OpenGL ES; the game's Voodoo2
driver is the port's own C++ and draws with OpenGL ES.

Based on [motor-dev/nfs-recompiled](https://github.com/motor-dev/nfs-recompiled),
the recompilation this port started from, which targets desktop. This project adds the
Android target: a launcher, touch controls, gamepad handling, widescreen races,
and a number of fixes in the graphics layer that only showed up once the game
ran on a phone.

## Game data you supply

The executables are in the repository, as upstream, so a clone builds and runs
as-is. The game content is not: bring `FEDATA` and `GAMEDATA` from a retail disc
of the 1998 release — the Europe "Sold Out Software" disc is known to match, and
a CD image works. Copy both folders somewhere writable and clear the read-only
attribute, or the game cannot write its settings and saves. That folder is what
you import in the launcher.

`install.win`, the table of data paths the game reads before anything else,
comes from the installer rather than the disc. The Android build packages the
copy at `android/app/src/main/assets/gamefiles/install.win`; for a desktop run
put it next to the game data. `tools/make_install_win.py` regenerates it and
documents the format, reverse-engineered from the executable. So a complete
desktop game folder is `fedata/`, `gamedata/`, `install.win` and the four
executables from `nfs3hp/`.

## Playing on a phone

The game's data is imported once, from a folder holding `FEDATA` and
`GAMEDATA`, a ZIP of them, or an image of the disc: an `.iso`, or the `.bin`
(`.img`) of a BIN/CUE pair, read in place (`DiscImage.java`: ISO 9660 in
2048-byte sectors or whole 2352-byte MODE1/MODE2 ones; the CUE sheet is not
needed).

Android 8.0 or newer on an ARM device with OpenGL ES 3.0: one APK holds
arm64-v8a and armeabi-v7a, and the installer takes the device's first, so 64-bit
Android always runs arm64-v8a. Testing happens on Android 13, and
Android 10 was confirmed by a tester; 8 and 9 install but have not been played
on yet.

1. Put the game data folder on the device, e.g. `/sdcard/nfs3-og`.
2. Launch the app, import that folder, make it the active data set.
3. Play.

An import leaves the game ready for a phone: the gamepad control set (see
Controls), View Distance at Full, races at 1280x720 and a HUD arranged for a
phone — the standings and the maps in the corners rather than under your thumbs
— are written into its `config.dat` once, as the last step. Data straight off
the disc has no `config.dat` yet — the game makes it the first time it runs —
and gets the same at that first start. From then on the file is the player's,
and the game's own Heads Up Display screen rearranges it.

A race also starts sooner than the original's: the game used to copy the whole
of the track it was about to play, 7 to 14 MB, into a file of its own and play
from the copy. That was for a 1998 CD drive; here the track is played where it
lies.

The launcher holds what the game cannot ask for itself: **Controls → Touch** (the
on-screen controls, their layout editor, the key each one sends),
**Controls → Gamepads** (which pad is which player, Analog or Digital for each,
the key each button sends), **Controls → Force Feedback** (where the game's
effects are felt: off, the phone or the gamepad), **Display** (orientation, frame
rate cap) and **Advanced** (below).

The orientation is the launcher's as well as the game's: **Auto-rotate** is
`USER_LANDSCAPE`, both landscape directions as long as the system's rotation
lock lets apps turn, and the two fixed choices pin one direction each
(`GamePreferences.screenOrientation`). A DeX window takes the shape the player
gives it.

There is one build, the release. **Advanced** on the main menu says whether it
behaves as a debug build: with debug mode off nothing is written to the log -- the Java
side through `AppLog`/`SDLLog`, the native side through `NFS_LOG=0` (log
priority critical, and SDL's direct Android logging held back by
`android/app/jni/sdl_quiet.h`) -- and the game starts as a player has it. With it
on, the log is open and the screen's switches (`DebugOptions`) set the
diagnostic variables below when the game next starts. Each switch keeps its
launch-extra name, and a launch extra wins over the screen for that start:
`adb shell am start -n dev.nfs3hp.port/.SplashActivity --ez debug true --ez trace_ticks true`.

**Live functions log** (`live_counter`, under Traces) records
which of the game's functions run. With it on (and debug mode) the game starts
a timer, `ITIMER_PROF`, that every 4 ms of CPU time interrupts the running
thread; the handler walks its frame-pointer chain and counts the addresses in
`libmain.so` (`src/nfs3hp/func_log.cpp`): the function the time is spent in, and
every function with a call in progress. Nothing is added to the game's code and
with the switch off none of it exists. A thread appends what changed to
`functions.bin` in the game's `logs` folder
(`/sdcard/Android/data/dev.nfs3hp.port/files/logs/`) every two seconds, and
the log's `[FUNCS]` lines say how it goes. It is a sample: a function that runs a few
times in a session may not be in it. Play a session -- menus, a race, its replay
-- leave the game, then:

```bash
adb pull /sdcard/Android/data/dev.nfs3hp.port/files/logs/functions.bin
python tools/profile_report.py functions.bin [libmain.so] [report.md]
```

The report sorts the functions into `native_*.cpp` and the typed port
(`port_*.cpp`) by the source files in the library's debug info,
and says how many of each were seen, how the time divides, and what runs in
each 20 seconds. The log also notes the x87's precision: every control word the
game sets reaches `FloatEnv::s_controlHook` (`include/fpu.h`, null otherwise), each
thread's last one is kept, and each sample carries the precision control its
thread was under, so the report divides the time between 24, 53 and 64 bits and
counts the words set.

The report needs the unstripped `libmain.so` of the build the APK was
made from (`android/app/build/intermediates/cxx/*/*/obj/arm64-v8a/`); the build
id in `functions.bin` is checked against it.

**Comparing builds by a replay.** A replay is the race driven again from its
recorded inputs, the opponents and the physics included, so the same replay on two
builds must drive the same cars. `replay_trace` (under Traces,
`src/nfs3hp/replay_trace.cpp`) writes every car at every physics step to
`logs/trace-race.bin` or `logs/trace-replay.bin`; a replay's first pass only, since
it goes round again at its end. `tools/replay_run.py` plays one on the phone with
nothing touched (launch extra `auto_replay`: no movies, from the first menu straight
into `replay.rp3`) and fetches the trace; `tools/replay_compare.py` says whether two
traces match, or the first step, car and field that differ:

```bash
python tools/replay_run.py new.bin --replay diagnostics/replay-golden/test2.rp3 --compare diagnostics/replay-golden/traces/test2-a.bin
```

It stops at the first step that differs, and `--seconds n` after that many seconds
of the race alike, for a quick look. The unlocked screen has to stay on. Two runs of
one build match to the bit. The physics
is chaotic, so a tolerance means little over a whole race: at 53 bits instead of 24
the first bit differs at the start, the cars are half a metre apart after 22 seconds
and up to 1.7 km apart after 219. A replay recorded under other arithmetic plays
differently for the same reason.

**Logs** (Data -> Logs, in English in every language like Advanced) gathers what
the diagnostics have written (`LogBundle.java`): the files in the game's `logs`
folder -- `functions.bin` and what other traces put there -- plus `cars.bin` and
`ai.bin` beside it, the app's own lines of the system log (`logcat -d`, which
Android lets an app read for itself; there are lines only in debug mode), and
`device.txt` (the build, the phone, the debug switches). *Export to a file* asks for a
place with the system's file picker; *Share* opens the share menu with the zip
attached; *Send to the developer* opens a letter to the address in
`developer_email` (`res/values/strings.xml`; the button is gone if it is empty)
with the zip attached. The zip for Share and Send is made in the cache and
reached through `FileProvider` (`res/xml/log_paths.xml`). *Clear logs* deletes
the files and runs `logcat -c`, which the system may refuse; it will not run
while the game is, since the game has the files open.

The menus carry one button of the port's, the keyboard in the top-right corner:
a finger works them as a pointer, and the system's own back is the game's
Escape. The button is there for typing the game takes with no box on the
screen -- the main menu's cheat codes and a network game's chat from the menu
over a race -- and it follows the touch size and opacity settings. When the game asks for
a name, a tap on the line being typed in brings up the phone's keyboard and a
tap beside it puts it away; a box opened from a pad brings the keyboard up at
once, and with a real keyboard connected it never comes up. A tap on the
player's name beside its tab opens the same box. A pad's buttons follow the same
two worlds — the racing set while a race is driven, confirm and back everywhere
else.

The first screen's **NFS Web Site** button opens **Options** instead, the way
the race screen's Options button does, so the options are a tap away from the
start. The game reads the port's version of `MENUS/SETUP.MNU` in place of the
imported one (`src/nfs3hp/menu_data.cpp`, as for `MAINSPLT.MNU`): that button
takes the race screen's text and `nextmenu=options`, and loses its codelink, by
which the screen's code (`sub_452f80`) would otherwise make it open the web
site again. The imported data is left as it is.

### Controls

The game always sees two joysticks, `NFS Gamepad 1` and
`NFS Gamepad 2`, each with two axes: X steers, Y carries both pedals — the right
trigger pulls it up to accelerate, the left one down to brake. Gamepad 1 is
where the touch controls steer and accelerate too. Every other button arrives as
a keyboard key, chosen per button under Controls → Gamepads → Buttons: player
1's from the first pad, player 2's from the second.

*Gamepad ON* writes a matching control set into the game's
`fedata/config/config.dat`, backing the file up first, and the description of
both devices with it — the game regenerates every binding whenever the saved
device list differs from what it finds. *Default in game* writes the game's own
keyboard defaults instead. Those put split screen's second player on keys the
first pad's buttons send, so while a split-screen race runs on them the pads
send nothing but Escape.

Each pad is set to *Analog* or *Digital* under Controls → Gamepads. Analog is the
above: the stick steers, the triggers work the pedals' axis. Digital drives a
race as a keyboard does, and only a race — the menus are the same either way:
the D-pad steers left and right, D-pad up and the right trigger accelerate,
D-pad down and the left trigger brake (a trigger counts as held from a third of
its travel), and the stick does not steer. Each of those sends the player's own
key, and the control set binds it: player one's are the keys the touch
controls send, so a digital first pad is written as the set the on-screen
controls drive with when there is no pad; player two's are End, Page Down, Home
and Page Up — the keypad's 1, 3, 7 and 9 to the game, which hands those out in
its own split-screen defaults — bound for split screen's second player while
Gamepad 2 is digital. The game sees keys steering and steers them its own way,
easing to full lock and back, in a single race, split screen, a network race
and a replay alike. The launcher rewrites the set as the game starts whenever a
pad's setting changed. The two players never share a key: the second pad never
sends one of the first player's, so its D-pad's arrows, which steer player one
whenever player one drives on keys, stay quiet in a race.

On screen, the game decides which controls are up. In a race: two steering
buttons, the pedals and the rest, and the steering buttons turn the wheel the
way the game's keyboard steering does, easing to full lock and back, while a
pad's stick stays analog. In the menus a finger is the game's own pointer: a tap
clicks where it lands, resting still for a quarter of a second presses and
drags, and a swipe clicks nothing; no controls stay on screen. The layout
editor shows the controls exactly where the game puts them.

The phone's back button or gesture is Escape, in the menus and in a race; with
the on-screen keyboard open it closes the keyboard. A real mouse moves the
game's cursor to its own pointer and clicks with its buttons, races included.

### Vibration

The game's own force-feedback effects are implemented
(`src/lib/forcefeedback.cpp`) and translated to rumble. Only
the first force-feedback device gets them — Gamepad 1 or the phone — so Gamepad
2 never vibrates. Where they are felt is one choice under Controls → Force
Feedback: *Off*, *Phone* or *Gamepad*, never the phone and a pad at once. A pad
plays the game's jolts, road and engine; the phone plays jolts, the engine by
its revs, and how hard the car corners. How strong a pad plays is set in the
game's own Force Feedback menu. The choice takes over from the two switches
0.76 had, one under Controls → Touch and one under Controls → Gamepads: the pad
if its switch was on, otherwise the phone if its switch was. Nothing vibrates on
a button press.

### Widescreen races

Options → Graphics → Screen Size offers 1280x720, 1600x900, 1920x1080 and
Dynamic Fullscreen beside the original 4:3 modes, and the choice survives a
restart. Dynamic Fullscreen fits the race to the screen it runs on, whatever
its shape -- 16:9, 20:9, a tablet's 16:10 -- with no bars: its height is the
screen's up to 1080 and its width the screen's proportion of it, worked out
again as each race opens its mode, so a foldable opened or shut between races
gets its new shape at the next one (during a race the picture keeps its shape
between bars). Its size is in the Voodoo2 driver's mode table's last entry
under a resolution id of the port's own; the settings keep a marker for it
rather than a width (`src/nfs3hp/dynamic_fullscreen.h`), and a fresh import
starts with it. A race then
opens the view sideways instead of stretching it: the vertical angle stays what
the game draws and the horizontal one widens with the screen, in every view —
chase and in-car, the mirror, both halves of split screen. The menus stay 4:3;
only the race switches mode, and its loading screen is shown at 4:3 in the
middle, between black bars.

The HUD keeps the proportions it was drawn with: frames, text and the points on
the map are sized as on the 4:3 screen that fits, and the gauges drawn as
pictures keep their shape. The layout saved in `config.dat` is never touched, so
a 4:3 mode looks exactly as it did. The HUD editor frames each element by what
it draws, so a line of text can stand next to a gauge or at the edge of the
screen. The in-car cabin fills the width at its own proportions, centred, with a
little of the roof and the dashboard's lower edge cut off.

`NFS_WIDESCREEN=0` takes the wide modes back out wherever an environment
variable can be set — the launcher does not set this one.

View Distance at Full draws the whole track out to the far distance in every
mode, split screen and night included, as the Modern Patch does; the other
settings keep the original distances.

Alpha intensity in Advanced Graphics works here as well. The original applies it
only on its Direct3D driver, so on this one the slider used to move and change
nothing.

Gamma, brightness and contrast are sliders on the game's own Advanced Graphics
screen. The port hands the game its own versions of two menu files
(`src/nfs3hp/menu_data.cpp`): Advanced Graphics with the three sliders, of Alpha
Intensity's kind, and both it and Graphics in the pause menu without Perspective,
which picked a rasteriser this port does not have; the pause menu's Alpha
Intensity stands in its row. `src/nfs3hp/port_display.cpp` runs after Advanced
Graphics' handlers, which stay as the checks hold them: the picture follows a
slider at once, as all three act on the finished frame, Cancel puts it back, and
anything else writes the values to the launcher's preferences, which hand them to
the game as it starts and carry them in its settings file. The three names are in
English in every language.

### Full colour

The picture is drawn with eight bits a channel, races and menus alike, with no
dithering. Textures that are 32-bit on the disc stay 32-bit: most of the cars'
paint, the HUD, smoke, lights and sky, and the menus' track pictures, car
comparison and logos. The original let only its Direct3D drivers take them and
shrank them to 16 bits for a Voodoo2; here the Voodoo2 driver takes them too
(`port_thrash.cpp`), so Screen Size reads "x 32". Art that is 16-bit
on the disc — the track surfaces and most of the menu backgrounds — stays as it
is. `NFS_TEXTURES32=0` goes back to 16-bit textures.

### Projected headlights

With projected headlights the game draws the lit road a second time over
itself, and that pass hands the driver texture coordinates up to thirty
repeats of the road texture away from the origin. On Mali GPUs that was enough
to lose the fraction of a texel per pixel, and the lit area broke into moving
blocks — the same artefact the Modern Patch shows under dgVoodoo. A triangle
whose wrapped coordinates lie that far out is moved back by a whole number of
repeats before it is drawn (worked out in double, so nothing it samples
changes), and the lit road draws clean. `NFS_TEXCOORD_REBASE=0` turns this off.

The headlights' light pool, a car's shadow and the glow of its lights in the
rain are laid on the road with a less-or-equal depth test. The Voodoo kept 16
bits of depth, and a decal came out equal to the road under it; the finer depth
a phone's GPU keeps differs in the last bits from pixel to pixel, and the decals
flickered against the road and each other. The fragment shader rounds depth to
the Voodoo's 16-bit steps, but only where the driver gave a depth buffer of more
than the 16 bits asked for (Mali gives 24); a real 16-bit buffer already holds
those steps. Never on an Adreno 5xx: there the shader's depth write broke the
depth test, and the wheels showed through the body. `NFS_DEPTH16=0` or `1`
overrides the choice.

### Rear-view mirror

In a single race the mirror is drawn as the main view is: the same draw
distance, the near track model, cars at full size, the sky with its clouds and
lightning, the cars' lights and the headlights' light. The original cut all of
that down for the mirror (a reach of 100 against 500, the far track model, cars
at 0.6 of their size, a flat sky). There is no menu option;
`NFS_MIRROR_FULL=0` brings the original's mirror back (`nfs3hp_main.cpp`, `fullMirror`).

### Network races

The game's Connection screen offers IPX and TCP/IP; RaceNet, Modem and Serial
are hidden, as nothing carries them any more.

- **IPX** finds races on the local network, phone with phone. It is carried on
  UDP: an IPX socket is the UDP port of the same number (the game's 0x452 is
  UDP 1106), a node is the phone's IPv4 address, and a broadcast goes to every
  network's broadcast address. Only the port speaks it; a PC would need an IPX
  wrapper.
- **TCP/IP** connects to an address: phone with phone, or phone with a PC
  running the Modern Patch (tested both ways with 1.6.1). The port is 9803, as
  the Modern Patch has it; the connection screen shows the phone's address and
  switches to the original's 1030 (`NFS_NET_PORT`).

The game's own connection screen calls IPX "Local WiFi", and has the TCP/IP port
and the phone's address under its buttons: the port's version of
`MENUS/CONNECT.MNU` (`src/nfs3hp/menu_data.cpp`) and `src/nfs3hp/port_connect.cpp`.

A race is lockstep: every machine simulates every car from the players'
inputs, so the port works the x87 as a PC does in a race (every player needs
the same precision, see below), and the game's
version is checked when a machine joins. The port reports nfs3.exe's own,
"27, 2.0" — the version resource the phone has no copy of, and the string the
Modern Patch keeps in its data. Winsock 1.1 is in `src/nfs3hp/host_net.cpp`
over the phone's sockets (`src/lib/socket.cpp`); `NFS_NET_TRACE=1` writes a
join into the log in debug mode (switch `net_trace`).

### The x87's precision

A PC of 1998 raced in single precision: before every race the game takes the
x87's precision control down to 24 bits (fldcw at 0x4a3e8d and 0x4a3ed3), and
every sum, product and quotient of the physics and of the opponents' driving
keeps a float's significand. The single precision starts as the game leaves its
menus, so a race's loading and its replay run in it too. The Modern Patch drops
that fldcw and races at 64 bits. The port's arithmetic is the original race's, all
the time: 24 bits in the race as on a PC, and in the menus too, where the original
had 64; a replay comes out the same to the bit whichever the menus and the
tables built at the start are in. Every machine of a network race needs the same.

Single precision rounds sums, products, quotients and roots only: sin, cos and
atan2 (fsin, fcos, fpatan) keep their full width, as on the x87, until the next
operation or a float store rounds them. So a value between operations is a double,
holding a float's 24 bits with the x87's wider exponent; the operations are `fp::`
in `include/fpu.h`, which work out the exact result's side of a tie, and come out
as the x87's to the bit. There is no x87 under them, only its control word: the
thread's `FloatEnv` (`include/fpu.h`), which starts at single precision and which
the game's runtime sets and reads as it did the x87's (`_control87`). Where the
code itself loads a control word around its work, its arithmetic goes by it: 64
bits to turn numbers into text and back in the runtime, and chopping for an
integer. The HUD and view rewrites (`port_view`, `port_hud`) compute in doubles,
which only what is drawn sees.

## What the old processors asked for, removed

The game measured the processor at start-up and chose by it: movies' quality and
whether to drop their in-between frames, the detail settings of a first start, MMX
routines or plain ones, the x87's copies. It also worked round the Pentium's FDIV
bug. None of this is on the port: `nfs3hp::removed` answers with constants, movies
are always at their best quality and the detail at the game's highest. A movie still
drops a frame that is late against its sound; that is keeping them together, not the
processor's speed.

## Windows, on the host

The game asks Windows for files, threads, a window, DirectDraw, sound, input and
the network through its imports. None of them is a Windows of the port's any more:
each call site in the native code calls the port's host code in the import's
place (`guest::windows<slot, &host::fn>`, `src/nfs3hp/native_windows.h`), with the
words the game's code pushes for it, and gets back what Windows gave in eax. The
host code is in `src/nfs3hp/host_*.cpp` on `src/lib`: `host_files` (files,
directories, drives, mappings), `host_system` (the process, memory, modules,
environment, console, code pages, errors, the registry, the version resource and
what was left of user32, ole32 and shell32), `host_threads`, `host_window`,
`host_video` (DirectDraw) and `host_net`. `src/lib/winapi` keeps only the types,
COM's objects and the wrapper that makes a stdcall thunk of a host function
(`win32::Wrapper`, for what the game reaches through a COM object's table).

## How the port is made

Every function of the game is C++ that runs on the phone's own processor: no
x86 is emulated, and nothing is generated any more. The port began as
motor-dev's static recompilation -- nfs3.exe and its DLLs translated into C++
on a virtual x86 -- and was rewritten from there, a subsystem at a time, until
no code of that kind was left; then the virtual CPU, its x87, the dispatcher
and the generator were deleted. What stays of the original is its data and the
way its functions call each other.

**The game's memory.** The game's data lies in one flat block at the
original's addresses: its variables, tables and records where nfs3.exe kept
them, so the files it saves and the network packets are the original's to the
byte.
`src/nfs3hp/disassembly` holds that image -- the executable's and the DLLs'
sections (`*.DGROUP.cpp`, `*.AUTO.cpp`, `*._idata.cpp`, `*._rsrc.cpp`) -- and
the tables that put each of the port's functions at the address the game's own
had (`nfs3hp.cpp`, `eacsnd.cpp`, `voodoo2a.cpp`, declared in `nfs3hp.h`). The
generator made them once from the original files; they are ordinary sources
now.

**Calls.** A function calls another as C++, by name (`guest::direct`,
`fe::direct`, `src/nfs3hp/native_guest.h`). Where the game hands a record's
address on -- a buffer, a string, a frame `strtok` keeps a pointer into -- the
function keeps that frame on the game's stack in the game's memory
(`fe::Frame`, `GuestFrame`, below `cpu.esp`), at the depth the original had it,
so every address the game sees is the one it saw on a PC. A call through a
pointer the game's data holds -- a table of handlers, a callback, a COM
object's method, a thread's start -- goes to the function registered at that
address (`app->callAddress`, `guest::pointerCall`): a thunk
`x86::reg32 f(App*, CPU&, x86::Args&)`. `x86::Args` (`include/cpu.h`) is the
calling convention the game's Watcom code was compiled with, as a record: the
arguments in eax, edx, ebx and ecx, more on the stack, a float argument and
result where the math library passes one, and what a callee hands back besides
eax. Some of EA's code reads ebp it never set (an uninitialised variable the
compiler kept there); `port::HeldEbp` and `guest::heldEbp` keep the value it
found on a PC and hand it on.

**A thread.** `x86::CPU` is no processor any more: the top of the thread's
stack in the game's memory (`esp`), its thread block (the exception chain and
the context lock), its floating-point environment (`FloatEnv`, the control and
status words the game's runtime sets and reads) and whether the game is
stopping. The game's threads -- its own, the timer, the sound's -- take one
lock between them, as on Windows 98, and hand it over at the safepoints of
their waiting loops (`lib/winapp.h`).

**Unreachable code.** What nothing can call -- the modem and serial links,
the software renderers and Direct3D, the library functions the linker brought
in unused -- is a stub that stops the game with its address
(`native_dead.inc`, `native_graphics_dead.inc`, `native_snd_dead.inc`); one
that ever fires names the function still to write.

**Checking a change.** A replay of a race (see Comparing builds by a replay)
is the check: the same build gives the same race to the bit, so a rewrite that
keeps the numbers must keep the replay's trace bit for bit
(`diagnostics/replay-golden/test2.rp3` and its trace), and one that changes
them is judged by how the race plays. The HUD, the look-back camera and other
pictures are checked by eye on a phone.

### Where each part is

- The THRASH driver behind the renderer, voodoo2a, is the port's own
  (`port_thrash.h`, `port_thrash.cpp`): its 34 exports draw straight through
  `ThrashRenderer` (`src/lib/thrashrenderer.cpp` -- an OpenGL ES texture for
  each of the game's, the formats as they are, the GPU's filtering), with no
  Glide, no glide2x.dll and no DirectDraw under it. What the game sees of the
  driver stays in voodoo2a's data, where the game patches its mode table; its
  records come from voodoo2a's own heap. It draws as voodoo2a did: Glide's
  triangles, its lines as four triangles, trilinear mipmapping's two passes.
  The port's own features are in it: the 4:3 fit of the loading screen, the
  dialog emblem's narrowing, the widescreen and Dynamic Fullscreen
  resolutions, full-colour textures. softtria, the software renderer, is not
  built; the file stays among the bundled ones, which the game's integrity
  check looks for.
- The drawing: vertices turned and projected, clipping, the track's and
  objects' polygons and their depth sorting, particles, the cars' lights and
  detail (`native_vertices.cpp`); the triangles and quads clipped and drawn,
  the sky and lightning, the solid lettering and the countdown, a view's pass,
  smoke and spray, the HUD's frames, gauges and tables, the cabin, a car's and
  the 2D pictures' textures while a race loads (`native_render.cpp`); and what
  drives the driver -- render states, the frame, the window, screen modes and
  the Screen Size list, the textures, the menus' art, the polygon lists'
  drawers, the loading screen and bar, the screen capture
  (`native_graphics.cpp`). In a race these are about three quarters of the
  game thread's time.
- The sound: eacsnd, the driver, puts what the game mixes straight onto an SDL
  audio stream (`native_sound.cpp`, the rest of its exports
  `port_snd_isnd.cpp`); EA's SND library -- voices, banks, patches, the 100 Hz
  tick, streams (`port_snd*.cpp`) -- its mixer (`port_snd_mixer.cpp`) and the
  channels' set-ups, resamplers and decoders (PCM, XA ADPCM, EA's frame-coded
  speech; `port_snd_mixsetup.cpp`, `native_snd_mixfuncs.cpp`), the mixing,
  written for MMX, as plain loops that give the same samples
  (`native_mix.cpp`), and the game's own use of it: music, effects, engines,
  sirens, cops, the announcer, the co-driver and the radio, 3D positions
  (`port_sound_*.cpp`, `native_snd_*.cpp`).
- The game's side of the two drivers and the logger they report through
  (`native_game.cpp`): the renderer loaded and its functions looked up, eacsnd
  loaded and driven, `Error` and `Message`, the game's fill.
- The C runtime the game was linked with, Watcom's (`native_watcom.cpp`,
  `native_watcom_heap.cpp`; `src/nfs3hp/watcom.md` maps its 275 functions by
  Open Watcom's names): strings, `qsort`, files and streams, `sprintf` and
  `sscanf`, time, the environment, the math library's `floor`, `exp`, `acos`
  and `atan2`, structured exceptions, the start-up, the heap.
- EA's platform libraries (`native_ea_threads.cpp`, `native_ea_memory.cpp`,
  `native_ea_files.cpp`, `native_ea_support.cpp`, `native_ea_formats.cpp`,
  `native_ea_net.cpp`): threads, events and locks, the memory classes, file
  handles, `.viv` and BIG archives, the unpackers, the network layer (NET,
  CONN, the queues), the low timer.
- The controls (`native_controls_*.cpp`): DirectInput as the game used it,
  the control sets for each player's device, `config.dat`'s controllers, the
  race's force feedback and each frame's inputs, and the queue of inputs that
  feeds the cars and the network race. There is no DirectInput under them: the
  port answers each of its calls itself (`native_controls_host.cpp`) from its
  pads, mouse and keyboard (`lib/gamepad.h`) and force feedback
  (`lib/forcefeedback.h`).
- The race: where a car is on the track (`native_trackdata.cpp`), its race
  state each frame and the start grid (`native_racestart.cpp`), the race's cars
  loaded, laps and places, the race-controls thread (`native_raceload.cpp`),
  the physics (`native_physics.cpp`, `native_drivetrain.cpp`,
  `native_handling.cpp`, `port_conv.cpp`), collisions (`native_collide.cpp`),
  the cars' state (`native_carstate.cpp`), the opponents' and the traffic's
  driving (`native_ai*.cpp`, `native_aidrive.cpp`, `native_aiplan.cpp`), the
  police (`native_pursuit.cpp`), the HUD's layout (`port_hud.cpp`) and the
  views (`port_view.cpp`).
- Loading and saving: EA's RefPack decoder, FCE models and a track's FRD
  (`native_loading.cpp`; the formats are the ones OpenNFS reads, the
  structures the game's own), a race's files, the track's cameras, paths,
  textures, sky and weather (`native_load_track.cpp`), the cars' database
  (`native_load_cardb.cpp`), the cars' models and specs
  (`native_load_carmodels.cpp`), replays, records and `config.dat`
  (`native_load_saves.cpp`).
- The movies (`native_movie.cpp`): EA's MAD player, decoder and display,
  decompiled to C in `decomp/mad`; each frame goes to the screen in full
  colour, scaled on the GPU.
- The menus: the 57 screens of the handler table at `0x555c48` and what they
  call (`native_frontend_*.cpp`, each with its header), and under them the
  menu engine in layers -- the text and fonts (`port_text.cpp`), the 2D
  drawing (`port_draw2d.cpp`), the shapes and art (`port_shapes.cpp`), the
  widgets (`port_buttons.cpp`, `port_fields.cpp`, `port_obj3d.cpp`), the
  `.mnu` loader (`port_menus.cpp`), the menu loop (`port_loop.cpp`) and the
  dialogs (`port_dialogs.cpp`). The menus' state stays in the game's memory,
  where the race reads it (`native_frontend.h` lays out a screen and an
  item). Their arithmetic is the game's single precision
  (`fe::FloatMath`), so a box in a race's pause screens lands where the
  game puts it, to the pixel.
- Windows (see Windows, on the host): `host_*.cpp`.

## Building

### Android

Requires Android SDK with **NDK 28.2.13676358**, build-tools 36.0.0, compileSdk
36, and a JDK (Android Studio's bundled JBR works). The app is arm64-v8a and armeabi-v7a (the second with `-fmax-type-align=1`: 32-bit
ARM faults on a float at an unaligned address, where the game keeps some), minSdk 26, so it installs on Android 8.0 and later. The floor is `java.nio.file`,
which the launcher reads and writes the game's files with; every platform call
above 26 is behind a version check with a fallback.

SDL3 is built from source on Android and is not vendored here — clone it first:

```bash
git clone --depth 1 --branch release-3.4.2 https://github.com/libsdl-org/SDL third_party/SDL
```

Then build:

```bash
cd android
./gradlew assembleRelease
```

The APK lands in `android/app/build/outputs/apk/release/` unsigned: align it
with `zipalign -p 4` and sign it with `apksigner` and the key in `APK/`. Testing needs no other build: the launcher's Advanced screen
opens the log and the diagnostic switches.

### Desktop (Windows)

Uses the prebuilt SDL3 in `sdl3/`. CMake ≥ 3.15 and a C++17 compiler:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Run it with the path to your game folder:

```bash
./build/nfs3hp /path/to/game
```

With two arguments the second is the CD path, for data split between an install
directory and the disc.

### CMake options

| Option | Default | Description |
|---|---|---|
| `NFS_TRACE_MSG` | `OFF` | Message-pump tracing. Very verbose. |
| `NFS2_ASSERT_TRAP` | `OFF` | Unsupported-path asserts break into the debugger instead of logging. |
| `NFS_BUILD_FF_TESTS` | `OFF` | Build `force_feedback_checks`, the force-feedback (`lib/forcefeedback.h`) regression test, against an SDL virtual joystick. |
| `NFS_BUILD_MEMORY_TESTS` | `OFF` | Build the guest memory allocator checks. |

## Environment variables

Read once at startup. On Android the launcher sets `NFS_ORIENTATION`,
`NFS_FPS_CAP`, `NFS_GAMMA`, `NFS_BRIGHTNESS`, `NFS_CONTRAST`, the `NFS_TOUCH_*`
keys and one variable per pad button from its own screens, and `NFS_AUDIO_RATE`
from the phone; the rest matter for a desktop run.

| Variable | Default | Description |
|---|---|---|
| `NFS_FPS_CAP` | `30` | Frame rate the renderer paces presents to. The game's logic is tuned for 30 Hz. |
| `NFS_GAMMA`, `NFS_BRIGHTNESS`, `NFS_CONTRAST` | `1.0`, `0`, `1.0` | Applied to the finished frame in the final blit, so menus and movies are covered as well as a race. Where they start; Advanced Graphics moves them from there. |
| `NFS_ORIENTATION` | unset | `auto` allows both landscape directions as the rotation lock allows; anything else pins one. Must be set before `SDL_Init`. |
| `NFS_CAR_DETAIL_FULL` | on | With Car Detail at High, every car keeps its detailed model and the player's texture size out to the draw distance, in every view, and the limit on how many are drawn at once is lifted. `0` restores the original. |
| `NFS_MIRROR_FULL` | on | The rear-view mirror is drawn as the main view is: its distances, the track's detailed models, cars at full detail, the sky with clouds and lightning, car lights and the headlights' light. `0` restores the original short, coarse mirror. |
| `NFS_WIDESCREEN` | on | `0` takes the 1280x720, 1600x900 and 1920x1080 modes back out of the Screen Size list. |
| `NFS_TEXTURES32` | on | `0` has the Voodoo2 driver refuse 32-bit textures again, so the game shrinks them to 16 bits as it used to. |
| `NFS_TEXCOORD_REBASE` | on | `0` leaves far-out texture coordinates as the game sends them (see Projected headlights), to compare. |
| `NFS_DEPTH16` | auto | On where the depth buffer has more than 16 bits and the GPU is not an Adreno 5xx. `0` keeps the GPU's own depth precision, `1` forces the Voodoo's 16-bit steps (see Projected headlights); the `[GPU]` lines count the frames drawn in them. |
| `NFS_LOG` | on | `0` (debug mode off, the launcher's Advanced screen) writes nothing to the log: log priority critical, and SDL's direct Android logging held back (`android/app/jni/sdl_quiet.h`). With debug mode off the launcher sets none of the diagnostic variables. |
| `NFS_NET_PORT` | `9803` | TCP port of network races, as the Modern Patch has it; the connection screen offers the original's `1030`. |
| `NFS_NET_TRACE` | off | `1` (extra `net_trace`) writes a network race into the log: EA's comm library events and packet sends (`[COMM]`), up to 400 datagrams per socket instead of 16 (`[NET]`), and each new indirect call target once per thread after the IPX transport opens (`[CALL]`). For finding where two machines stop on the way into a race. |
| `NFS_REPLAY_TRACE` | off | `1` (debug mode, Traces: `replay_trace`) every car at every physics step to `logs/trace-*.bin` (see Comparing builds by a replay). |
| `NFS_AUTO_REPLAY` | off | `1` (debug mode, launch extra `auto_replay`) no movies, and from the first menu straight into the replay in `replay.rp3`. |
| `NFS_PERF_TRACE` | off | `1` (debug mode, Traces: `perf_trace`, "Missing performance data logging") follows player one's 0-100 and the other timings from the car (`sub_4cdf10`) through the race's take-down to the results' copy (`sub_455410`), for the race summary's Performance page showing `---`: `[PERF]` when the car's change, `[PERFTIME]` at each step of `takeDownRace` and when the results' records change. |
| `NFS_FUNC_LOG` | off | `1` (debug mode, switch `live_counter`) samples which functions run, to `functions.bin` (see Live functions log). |
| `NFS_AUDIO_RATE` | unset | The output's own sample rate. The game mixes at 22050 Hz and SDL converts to this; a device opened at any other rate is resampled by Android, off its low-latency path. Unset, the device opens at SDL's default. |
| `NFS_TOUCH_STEER_LEFT` etc. | unset | Keys the touch overlay sends for steering and the pedals, so that endpoint can report them as axes. |
| `NFS_GAMEPAD1_SOUTH` etc. | built-in defaults | What a pad button sends, as `NFS_GAMEPAD<n>_<BUTTON>`: an SDL key name (`Space`, `Return`), `axis:steer_left`, `axis:steer_right`, `axis:accelerate`, `axis:brake`, or empty for nothing. `_UI` after it is the same button in the menus. `LEFT_TRIGGER` and `RIGHT_TRIGGER` are the triggers read as buttons, empty by default; the launcher gives them the pedals' keys for a pad set to Digital. |

## Credits

- [motor-dev/nfs-recompiled](https://github.com/motor-dev/nfs-recompiled) — the
  recompilation this port started from.
- [libsdl-org/SDL](https://github.com/libsdl-org/SDL) — SDL3, zlib licence.
- [americusmaximus/AZHP](https://github.com/americusmaximus/AZHP) — AutoZone Hot
  Pursuit, MIT licence.

Need for Speed III: Hot Pursuit is the property of Electronic Arts. This
repository contains no game content — no tracks, cars, audio or video. Those you
bring from your own disc.
