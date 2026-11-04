# NFS III: Hot Pursuit — Android: technical details

An Android port of the static recompilation of **Need for Speed III: Hot Pursuit**.
The original 1998 Win32 x86 executable is translated into portable C++ that runs
on a virtual x86 CPU, with the Win32, DirectDraw and DirectSound calls it
makes reimplemented on top of SDL3 and OpenGL ES. Glide is not: the game's
Voodoo2 driver is the port's own C++ and draws with OpenGL ES itself. Nor is
DirectInput: the game's controls are native, and ask the port's own pads,
mouse and keyboard directly.

Based on [motor-dev/nfs-recompiled](https://github.com/motor-dev/nfs-recompiled),
which does the recompilation itself and targets desktop. This project adds the
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
effects are felt: off, the phone or the gamepad), **Display** with **Screen
adjustment** (orientation, frame rate cap, gamma, brightness, contrast) and
**Advanced** (below).

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

The report sorts the functions into generated code, natives (`native_*.cpp`) and
the typed port (`port_*.cpp`) by the source files in the library's debug info,
and says how many of each were seen, how the time divides, and what runs in
each 20 seconds. The log also notes the x87's precision: every control word the
game sets reaches `FPU::s_controlHook` (`include/fpu.h`, null otherwise), each
thread's last one is kept, and each sample carries the precision control its
thread was under, so the report divides the time between 24, 53 and 64 bits and
counts the words set.

The report needs the unstripped `libmain.so` of the build the APK was
made from (`android/app/build/intermediates/cxx/*/*/obj/arm64-v8a/`); the build
id in `functions.bin` is checked against it. A first measurement (0.78, 252 s,
by counting every call in an instrumented build): 859 of 2239 generated
functions ran, and in a race generated code was about 8% of the calls to the
original's functions.

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

### Full colour

The picture is drawn with eight bits a channel, races and menus alike, with no
dithering. Textures that are 32-bit on the disc stay 32-bit: most of the cars'
paint, the HUD, smoke, lights and sky, and the menus' track pictures, car
comparison and logos. The original let only its Direct3D drivers take them and
shrank them to 16 bits for a Voodoo2; here the Voodoo2 driver takes them too
(`tools/apply_full_colour.py`), so Screen Size reads "x 32". Art that is 16-bit
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
`NFS_MIRROR_FULL=0` brings the original's mirror back (`tools/apply_mirror_detail.py`).

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
  the Modern Patch has it; the launcher's Network screen shows the phone's
  address and switches to the original's 1030 (`NFS_NET_PORT`).

A race is lockstep: every machine simulates every car from the players'
inputs, so the port works the x87 as a PC does in a race (every player needs
the same precision, see below), and the game's
version is checked when a machine joins. The port reports nfs3.exe's own,
"27, 2.0" — the version resource the phone has no copy of, and the string the
Modern Patch keeps in its data. Winsock 1.1 is in `src/lib/winapi/wsock32.cpp`
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
as the x87's to the bit. Where the code itself loads a control word around its
work, the emulated FPU's operations go by it: 64 bits to turn numbers into text and
back in the runtime, and chopping for an integer.

The natives and the rewrites still work on the emulated FPU of the CPU they are
handed, whose arithmetic rounds by its control word: 24 bits since `FPU::init`, 64
in those same windows. The contact-physics functions `tools/decomp.py` converted
(`port_conv`) go through the same operations; the HUD and view rewrites (`port_view`,
`port_hud`) compute in doubles, which only what is drawn sees.

## What the old processors asked for, removed

The game measured the processor at start-up and chose by it: movies' quality and
whether to drop their in-between frames, the detail settings of a first start, MMX
routines or plain ones, the x87's copies. It also worked round the Pentium's FDIV
bug. None of this is on the port: `nfs3hp::removed` answers with constants, movies
are always at their best quality and the detail at the game's highest. A movie still
drops a frame that is late against its sound; that is keeping them together, not the
processor's speed.

## Native code

The game runs on a virtual x86 CPU, and the parts that cost the most no longer
go through it:

- voodoo2a, the THRASH driver behind the game's renderer, has no generated
  code: `disassemble_nfs3hp.py` binds its 34 exports to the port's own C++
  (`src/nfs3hp/port_thrash.cpp`, below) and writes only its data.
- softtria, the software renderer, is not built at all -- no code, no data,
  no library. The game loads one renderer, the one the registry's "Thrash
  Driver" names (`sub_4b5720`) unless a command-line option names another;
  the port sets voodoo2 and passes no options. The file itself stays among
  the bundled ones, which the game's integrity check looks for.
- About fifty of the game's own functions are native
  (`src/nfs3hp/native_vertices.cpp`): turning and projecting vertices,
  clipping, the track's and objects' polygons and their depth sorting,
  particles, the cars' lights and detail, and the small vector helpers called
  from everywhere. They are native whole: `disassemble_nfs3hp.py` writes no
  code for them (`VERTICES_NATIVES`) and the generated code calls them by
  name. (`tools/apply_native_vertices.py` keeps only hooks that watch: the
  `[VIEW]` arena peak, the opponents' lane votes, the `[COMM]` traces.)
- The rest of the drawing is native whole (`src/nfs3hp/native_render.cpp`,
  `nfs3hp::render`, bound by `disassemble_nfs3hp.py`'s `RENDER_NATIVES`, no
  generated code behind it; the port's changes inside these functions are the
  natives' own, and the `tools/apply_*.py` scripts leave them out;
  `tools/game_checks/render/run.py` holds each against its code made afresh
  on the desktop): the clipping of a triangle whose corners carry z (`sub_4bf790`), a
  quad of them (`sub_4c20e0`), the triangles and quads of a polygon list
  (`sub_434120`, `sub_4332b0`, `sub_433e30`), a triangle of one past the near
  plane (`sub_4c3ad0`), the subdivision of a large quad (`sub_433060`,
  `sub_432720`), the sky (`sub_47a880`, `sub_47a190`, `sub_47a1f0`,
  `sub_47b850`, the lightning `sub_4946e0`), the solid lettering and the
  countdown (`sub_475b50`, `sub_482620`), the cars' lights (`sub_4b9bc0`,
  `sub_4bae00`, `sub_4bb200`, the colour at night `sub_4b97b0`), a view's
  pass (`sub_41b9b0`), the choice of polygon drawers (`sub_434870`), colours
  under the sun's glare (`sub_49d800`), the detail settings (`sub_472d10`),
  the sparks (`sub_4ddb40`), a light's glow (`sub_491190`), vertices under the
  glare (`sub_47b7d0`), the 2D buffers cleared (`sub_4cb670`), a 2D screen's
  bouncing pictures (`sub_4a4410`), a headlight's patch on the road
  (`sub_491bc0`), the race's views and their records (`sub_41d960`,
  `sub_41df10`), a view's smoke and spray streaks (`sub_4de070`,
  `sub_4de700`), the views' distances for View Distance (`sub_41d620`), the
  transform buffer (`sub_4b56e0`), a screen mode (`sub_4bef50`), a driver
  started (`sub_4b59c0`), and in the HUD an element's frame (`sub_47f650`),
  its rectangle in pixels (`sub_480910`), the elements laid out inside a frame
  (`sub_480fc0`, `sub_4812e0`, `sub_481aa0`), the standings' and speeders'
  tables (`sub_481c50`), a frame's inset (`sub_487210`), the start lights
  (`sub_48c7c0`), the cockpit's needles, bar gauges and overlay (`sub_4880b0`,
  `sub_488470`, `sub_489240`), a clipped 3D line (`sub_4c0b20`), the segment
  meter (`sub_48c0e0`), the band behind the HUD in split screen
  (`sub_4800a0`) and the HUD's own sequence (`sub_482b00`), with the HUD's and
  the cabin's ports kept; and, while a race loads, a car's textures
  (`sub_4b7d30`), the 2D pictures made textures (`sub_4d0a10`, `sub_4d3f50`,
  `sub_4d41a0`), the cabin's pieces and bars (`sub_4d4580`, `sub_4d4ae0`), the
  loading screen (`sub_494cb0`), with the car detail and the loading screen
  at 4:3 kept. The movie player (`sub_495bc0`) stays the port's own MAD
  player (`src/nfs3hp/native_movie.cpp`), not this one.
- eacsnd, the sound driver, is native whole and has no generated code:
  `disassemble_nfs3hp.py` binds its sixteen exports to the port's functions of
  the same names and writes only its data. The driver proper
  (`src/nfs3hp/native_sound.cpp`) puts what the game mixes straight onto an SDL
  audio stream, with no DirectSound and no ring buffer in between; the other
  twelve exports -- the version, the game's functions, recording and the 3D
  buffers -- are in `src/nfs3hp/port_snd_isnd.cpp`, which
  `tools/sound_checks/run.sh` compares with eacsnd's generated code, made afresh
  from the DLL, on random states, to the byte.
- The game's side of the two drivers, and the logger they report through, is
  native and has no generated code: `disassemble_nfs3hp.py` binds the
  twenty functions of its `GAME_NATIVES` to the port's functions of the
  same names (`src/nfs3hp/native_game.cpp`), and the generated code calls
  them by name. They are `THRASH_opendll`, which loads the renderer and looks
  up its 34 functions; `iSNDloaddll`, `SNDcaps` and the focus and
  serve handlers, which load and drive eacsnd; `Error` and `Message` -- and the
  rest of what `iSNDloaddll` hands eacsnd, among it the sample converter --
  and the game's own fill at its three entries, which its code clears and
  fills memory with in hundreds of places.
  `tools/game_checks/run.sh` compares each with the game's code for it, made
  afresh from nfs3.exe, on random states, to the byte; on Windows,
  `tools/game_checks/windows/run.py [cases] [name]` does the same, built with
  the NDK's clang-cl and Visual Studio Build Tools (its double-precision build
  only; `GAME_CHECKS_DEBUG=1` adds debug information for the NDK's lldb).
  What a function calls is a stand-in where the native calls it by address --
  the address named in the code of the native's file -- and the callee's own
  code, run on both sides, where the native calls it by name or does what it
  does; a promise's `unread` lists the scratch registers no caller of the
  game's reads after the call (by liveness on the original code), which are
  left uncompared. `tools/frontend_checks/windows/run.py [rounds]` builds and
  runs `tools/frontend_checks` the same way.
  Both make the game's code again only when what their generators read has
  changed (a key of it in the build directory: for game_checks the
  generators, nfs3.exe, the stand-in rows, and of the port's sources only the
  addresses, functions and namespaces they name), compile only what changed
  since (game_checks' generated code in 16 parts), and check side by side on
  every thread (`GAME_CHECKS_JOBS`): game_checks a function a process,
  frontend_checks a share of the functions a process (`frontend_checks.exe
  nfs3.exe rounds k/n`). Both copy and compare only the pages a case wrote
  (`MEM_WRITE_WATCH`, `GetWriteWatch`): copying megabytes a case held them to
  the memory's bandwidth, so that all the threads got no further than one.
  `tools/frontend_checks/windows/run.py [rounds] [group|base]` takes a group of
  `tools/frontend_checks/groups/`: a row a function (`<address> <name> <kind>`),
  and that group's world, entries and watched memory, which run only in its own
  processes. A case past 200000 calls stops and says so.
- The C runtime the game was linked with, Watcom's, goes native family by
  family (`src/nfs3hp/native_watcom.cpp`, in `nfs3hp::watcom`, bound as the
  game's own are, `WATCOM_NATIVES`). `src/nfs3hp/watcom.md` maps all of it in
  nfs3.exe -- 275 functions among EA's libraries, by Open Watcom's names --
  and says which are native: all of it now -- the string and character functions,
  `atoi`, `rand`, `memset`, `__CHP`, the step before every `fistp` of the
  game, `qsort` and `bsearch`, `errno` and the functions on paths --
  `unlink`, `mkdir`, `chmod`, `access`, `getcwd` and the like -- and on
  handles -- `lseek`, `__qread`, `__qwrite`, `isatty` -- time -- `time`,
  `localtime`, `mktime` and daylight saving's rules -- the x87's `floor`,
  `ceil`, `modf`, `exp`, `acos` and `atan2`, the streams -- `fopen` and
  `fclose`, `fgetc`, `fgets`, `fread`, `fputc`, `fputs`, `fwrite`, `fseek`,
  `ftell`, `fflush`, the buffer under them and the handles under that --
  the formatted output -- `sprintf`, `vsprintf`, `fprintf`, `vfprintf` and
  `__prtf` under them, all of it but a float's conversion, which goes to the
  x87's formatter -- the environment, the time zone and the way out --
  `getenv` and the multibyte functions it compares with, `__tzset`, `exit`,
  `atexit`, the finalizers, `__GRO`, `sleep`, temporary files' names,
  `setenv` with the wide environment -- the formatted input -- `sscanf`,
  `__scnf`, `strtod` -- the console's `getch`, `getche` and `kbhit`, the
  structured exception handling, the start-up from `_cstart_` to the game's
  main, `__reporterror` -- and
  the heap, `_nrealloc` and `_nexpand` with it --
  `malloc`, `free` and what grows and shrinks it, in `native_watcom_heap.cpp`,
  which voodoo2a's heap is too.
  `tools/game_checks` holds them against their generated code as well.
- The game's mixing, written for MMX, is plain loops over 16-bit samples
  (`src/nfs3hp/native_mix.cpp`, `MIX_NATIVES`): a block added into the accumulator
  at its volume, the accumulator into the output, interpolating, decoding EA's XA
  ADPCM, and the forward copy its memmove and file reads use. They give the game's
  samples to the bit, and `tools/game_checks` holds them against its code, which is
  the only place `include/mmx.h` and `WITH_MMX` are still built.
- The game's controls are native whole, in `nfs3hp::controls`, bound as the
  game's own are (`CONTROLS_INPUT_NATIVES`, `CONTROLS_DEVICES_NATIVES`,
  `CONTROLS_RACE_NATIVES`, `CONTROLS_PIPELINE_NATIVES`, `CONTROLS_QUEUE_NATIVES`): its side of DirectInput -- DirectInput started and
  stopped, the mouse opened and read, the joysticks found, described, read and
  acquired, their dead zones and saturation, force feedback's gain, actuators
  and its 30 effects created, changed, started and stopped
  (`src/nfs3hp/native_controls_input.cpp`); the control sets the game makes
  for each player's device, by its kind or for the seven it knows by name
  (`native_controls_devices.cpp`); the controllers kept in `config.dat`
  matched with the ones found, and the race's force feedback -- the car whose
  player has the force controller, its jolts, the engine's and the road's
  rumble, the steering spring, every frame (`native_controls_race.cpp`, with
  the port's change of `tools/apply_force_feedback_car.py` in it); and a
  race's controls every frame -- the players' sets laid out by action as the
  race starts, the joysticks read, each control held or how far, and each
  player's four bytes made of them (steering, throttle, brake and a bit for
  each of 8 more actions, at 0x79c338 and 0x79c33c) and handed to the race's
  queue of inputs that feeds the cars (`native_controls_pipeline.cpp`); and
  that queue -- 128 frames of each player's four bytes, filled a few frames
  ahead and taken a frame at a time, the network race's sent and received
  through it, the frames a player missed repeated -- with each car's bytes
  taken from it (0x79c4a8, its one-shot buttons seen once a press), and the
  keys' and actions' edges the race's screens ask about
  (`native_controls_queue.cpp`). Each is
  written as its code is, and `tools/game_checks` holds them against their
  generated code, with DirectInput's objects, methods and results stood in.
  There is no DirectInput under them: each call the game made of it --
  DirectInputCreateA, each method of its objects -- is
  `controls::directInput`, which the port answers itself
  (`src/nfs3hp/native_controls_host.cpp`) from its pads, mouse and keyboard
  (`lib/gamepad.h`) and its force feedback (`lib/forcefeedback.h`), the
  objects only handles the game keeps where it kept them; `tools/game_checks`
  answers it as the game's code did, through the objects' tables
  (`controls_host.cpp`).
- Functions decompiled one at a time, wherever they are in the game
  (`src/nfs3hp/native_decompiled.cpp`): each address of
  `src/nfs3hp/decompiled.json` is bound to the function of its name there
  (`DECOMPILED_NATIVES`). They use nothing but registers, the stack and
  globals at fixed addresses, and `tools/game_checks` holds each against its
  generated code on random registers and stack and made-up values in every
  global the list names for it. So far: the menus' help line reset, the
  race's volumes handed between the settings and the race, the depth offsets
  for the driver, the sun's lighting of the cars reset, and the handling
  tunings reset.
- The game's graphics that drive the THRASH driver
  (`src/nfs3hp/native_graphics.cpp`, bound the same way, `graphics` in
  `decompiled.json`): every function the port's driver, voodoo2a, reaches that
  calls the driver -- the render states, the frame, the window, the screen
  modes and the Screen Size list, the textures and the shapes' conversion to
  them, the menus' art, the polygon lists' drawers, the HUD's message box and
  editor frame, the cabin's instruments, the loading screen and bar, the
  screen capture, the game's main. Each is written as its code is, port
  changes included (the widescreen list), and `tools/game_checks` holds it against the game's code
  with the patch made to it (`graphics_world.inc` makes up the records,
  modes and formats they read). What only the software renderers or Direct3D
  reach and nothing else calls is a stub that stops the game
  (`native_graphics_dead.inc`).
- EA's platform libraries under the game, native whole, in `nfs3hp::ea`
  (`EA_THREADS_NATIVES`, `EA_MEMORY_NATIVES`, `EA_FILES_NATIVES`,
  `EA_SUPPORT_NATIVES`): `win\threads.c` -- threads made and their
  priorities, events, waits and sleeps, the locks over critical sections and
  their pool (`src/nfs3hp/native_ea_threads.cpp`, with a few neighbours that
  are not threads: rotation matrices, a random fraction, whether a surface
  is locked, the memory there is); `cmn\memstd.c` -- the game's malloc and
  free, its memory classes and their blocks, and the memmove with its MMX,
  x87 and dword copies (`native_ea_memory.cpp`); `win\fileio.c`, `file.c`,
  `blockio.c` and `touchmem.c` -- the handles' table, files opened, read,
  written and seeked a block at a time or mapped, falling back to the
  archive, paths and drives, pages from `VirtualAlloc`, and the error and
  trace reporter (`native_ea_files.cpp`); and EA's trace onto the
  monochrome monitor, a file loaded whole (`loadfileadr`) or looked for, and
  `.viv` archives opened, searched and closed (`native_ea_support.cpp`).
  132 functions, each the x86 code instruction for instruction and held
  against the generated code by `tools/game_checks`.
- The race's own code, native whole a module at a time, the same way
  (each function the x86 code instruction for instruction, held against the
  generated code by `tools/game_checks`): `nfs3hp::trackdata`
  (`TRACKDATA_NATIVES`, `native_trackdata.cpp`) -- where a car is on the
  track: the nearest node, the road's quads found and followed, lanes,
  tunnels and the surface under it, the track's `.ccm` points and
  `simhts.dat`; `nfs3hp::racestart` (`RACESTART_NATIVES`,
  `native_racestart.cpp`) -- a car's race state each frame: put back on the
  road, the camera's shake, wheel slip, skid marks and their sounds, dust and
  sparks, the crash's effects, and the start grid from `start.dat` or
  computed; `nfs3hp::raceload` (`RACELOAD_NATIVES`, `native_raceload.cpp`) --
  the race's cars loaded from their `.viv` and handed their handlers, the
  laps, speeds and places counted each frame, the race's messages, the
  race-controls thread and its tick, and the cars against walls and each
  other. 122 functions.
- The main menu closed: everything it reaches is native whole. What it calls
  of the game's own (`FRONTEND_GLUE_NATIVES`, `native_frontend_glue.cpp`) --
  a button's keys, which start a race, leave the game or, over a network,
  say a machine is ready; the lists' and sliders' keys and layout; the
  attract mode's movies; the HUD's art in the language's version; the
  pointer and keys read -- and the menus' drawing (`FRONTEND_DRAW_NATIVES`,
  `native_frontend_draw.cpp`): the sliders, one and two axes, drawn and
  dragged, an item's layout, a .qfs loaded (the text, the 2D drawing and
  the shapes under them are the port's own, below); under them EA's network layer (`EA_NET_NATIVES`,
  `native_ea_net.cpp`: the game's packets, NET, CONN, the queues, LAYER) and
  EA's unpackers, keys and the low timer (`EA_FORMATS_NATIVES`,
  `native_ea_formats.cpp`, `native_ea_net.cpp`'s decompressors with them).
  112 functions, held against their generated code by `tools/game_checks`;
  and the menu engine's item lookup, an item shown and the dialog's button
  highlighted, hooked before, are native whole in `MENU_NATIVES`, held by
  `tools/frontend_checks` against the game's code made afresh.
- The game's data loaded and saved, native whole, in `nfs3hp::load`
  (`LOAD_TRACK_NATIVES`, `LOAD_CARDB_NATIVES`, `LOAD_CARMODELS_NATIVES`,
  `LOAD_SAVES_NATIVES`): a race's files -- the opponents' speed tables, the
  track's cameras, paths and textures, its horizon, sky and weather, the
  ghost -- and the files kept in memory as a BIGF archive
  (`src/nfs3hp/native_load_track.cpp`); the cars' database read from
  `fedata` -- the cars' list and records, their folders, `car.viv`, the
  `.FEC` texts, `carp.txt` (`native_load_cardb.cpp`); the cars' models, their
  textures and colours, the players' `carspecs` and a car's `carp.txt` sent
  and received in a network race (`native_load_carmodels.cpp`); replays,
  records and `config.dat` -- with the port's change of
  `tools/apply_track_detail.py` in its defaults: View Distance starts Full --
  and under them EA's BIG archives, `savefileadr` and a fixed-point sine
  (`native_load_saves.cpp`). 148 functions, each the x86 code instruction for
  instruction and held against the generated code by `tools/game_checks`.
- The cars' physics, native whole: the module at 0x4a78d6..0x4af3a1, compiled
  without optimisation, in three parts -- the step a frame and its forces,
  dampings and loads (`nfs3hp::physics`, `PHYSICS_NATIVES`,
  `src/nfs3hp/native_physics.cpp`), the engine, the gears and the driver's
  inputs (`nfs3hp::drivetrain`, `native_drivetrain.cpp`), the tyres' grip and
  slide, the handling made from the player's setup, the automatic gear and the
  entries the race's other code calls (`nfs3hp::handling`,
  `native_handling.cpp`). 35 functions, the x86 code instruction for
  instruction, the x87 through `cpu.fpu` under the race's control word, each
  held against the generated code by `tools/game_checks`.
- The game's sound, native whole but for the game's clock (`sub_48ed30`):
  EA's SND library -- its voices, banks and playing rewritten as the port's
  own C++ (below), the mixer (`src/nfs3hp/port_snd_mixer.cpp`), the
  channels' set-ups, volumes, resamplers and decoders (PCM, XA ADPCM mono and
  stereo, once, looped, with an intro, streamed from memory or a packet
  player, and EA's frame-coded speech with its x87 synthesis filter;
  `port_snd_mixsetup.cpp`, `native_snd_mixfuncs.cpp`, `native_mix.cpp`), the
  sample converter, streams and the reverb -- and the game's own use of it,
  every function the game reaches: music, effects, engines, sirens, cops,
  the announcer, the co-driver and the radio, the track's description, 3D
  positions and each race's sound set-up (`native_snd_*.cpp`). Each is the
  x86 code instruction for instruction, every register and the x87 as it
  leaves them, and `tools/game_checks` holds every one of them against the
  generated code. The port's CPU has MMX, so the mixer only ever takes the
  MMX set; the plain set and what only it reaches are stubs that stop the
  game with their address (`native_snd_dead.inc`), as is anything that
  nothing still generated, no game data and no native calls.
- The loading of the game's data (`src/nfs3hp/native_loading.cpp`, hooked by
  `tools/apply_native_loading.py`): EA's RefPack decoder behind every `.qfs`,
  an FCE model turned into the game's polygon records, and a track's FRD read into
  its blocks. The formats are the ones OpenNFS reads; the structures they go
  into are the game's own, filled as the generated code fills them.
  `tools/loading_checks/run.sh` builds them on the desktop with the generated
  functions they stand in for and compares the two on made-up data, without
  the game.
- The movies (`src/nfs3hp/native_movie.cpp`): EA's MAD player, decoder and
  display, decompiled to C in `decomp/mad`. The player calls the rest of the
  game -- streams, sound, memory, clock, keyboard -- as the original did; each
  frame goes to the screen in full colour, scaled on the GPU, instead of
  through the 16-bit surface.
- The code nothing can reach has no generated code: 904 functions, a stub
  each that stops the game with its address (`nfs3hp::dead`,
  `src/nfs3hp/native_dead.inc`, `native_dead.cpp`). Most of it the linker
  brought in with the libraries' objects -- the modem and serial links (TAPI,
  RAS, the COM ports), the C runtime's and EA's unused functions, import
  thunks the game calls past. `tools/audit_dead.py` proves each and writes the
  list (`--write`): no call, jump, immediate or displacement of it in any
  instruction that can run, the natives' original code included; no pointer
  to it or into it anywhere in the image outside instructions, every
  relocation and every 4-byte window of the data; no address of it in the
  port's own source; no running on into it. Every run proves the list again
  and fails if one of it can be reached.

Each was compared with the generated code bit for bit before it was switched
on — memory, registers, flags, the FPU and the triangles drawn. The vertex,
polygon and drawing natives (`native_vertices.cpp`, `native_render.cpp`) round
each step through the emulated FPU; in a race they are about three quarters of
the game thread's time. They are native whole, with no generated code behind
them in the game: their checks are on the desktop, against the game's code
made afresh with the port's changes they carry
(`python tools/game_checks/vertices/run.py`, and `render/run.py` beside it).
The loading natives keep their check
in the game (`NFS_NATIVE_CHECK`).

### The port's own C++

The natives above keep the x86's registers and the game's flat memory. The
port is being rewritten past that, module by module, as C++ of its own in
`nfs3hp::port` (`src/nfs3hp/port.h`): functions with arguments and results,
the game's records as structs, its variables by name. While code that is still
the game's shares the game's memory, a rewritten function keeps its data where
the game has it (`Var`, `Table`, read and written whole; `record`, a record
in place, field by field), calls what is not rewritten yet as Watcom's
convention has it (`callGame`), and is entered through an adapter that gives
the game's callers what that convention promises: the result in eax, every
other register as it was. The callers of the sound module and of EA's SND
library were checked for it -- none reads more. Rewrites call each other as
C++; to call the game's code they keep its stack where the game's code had
it, and where EA's code reads a register it never set (an uninitialised
variable the compiler kept in ebp), the rewrite reads it from the x86 as the
game's code would find it (`HeldEbp`). `tools/game_checks` holds each
`port::` function against the game's code to that: its result, the memory it
writes and every call it makes, with the registers each callee reads on
entry (`tools/game_checks/entry_regs.py`); a rewrite's calls to rewrites run
the game's code for them on the generated side.

So far: EA's SND library -- the voices, their volume, pan and pitch, the
100 Hz tick, the platform layer over the mixer, banks, patches, timbres and a
sample started on a channel (`port_snd.h`, `port_snd_voices.cpp`,
`port_snd_play.cpp`, 74 functions) -- and the game's sound effects and race
slots on it (`port_sound_effects.cpp`, 18).

And the THRASH driver (`port_thrash.h`, `port_thrash.cpp`): voodoo2a's 34
exports drawing straight through `ThrashRenderer` (`src/lib/thrashrenderer.cpp`
-- an OpenGL texture for each of the game's, the formats as they are, the
GPU's filtering), with no Glide emulated under it and none of the Win32 it set
the card up with: no glide2x.dll, no DirectDraw, no mode set on the window's
thread. What the game sees of the driver stays in voodoo2a's data, where the
game layer patches its mode table; its records come from voodoo2a's own heap
(`native_watcom_heap.cpp`). It draws as voodoo2a did over the port's Glide:
Glide's triangles, its lines as four triangles, nothing of the antialiased
ones or of points, trilinear mipmapping's two passes. The port's own features
moved in with it -- the 4:3 fit of the loading screen, the dialog emblem's
narrowing, the widescreen and Dynamic Fullscreen resolutions, full-colour
textures, the triangle trace. `tools/thrash_checks`
(`windows/run.py`) holds it against the driver and Glide as they were, frozen
in `tools/thrash_checks/reference/`, both under the same renderer: random
sessions of the game's calls, and after each call what the renderer was
handed and its state, the window, the frame buffer's locks, the game's
callbacks, the registers and the memory the game sees, all compared.

And the game's 2D drawing and text under the menus, the HUD and the pause
screens, the first layer of the menus taken out of emulation: the text and
its fonts -- the language's lines, a text measured, drawn and wrapped into a
box, the text scale, the characters queued as quads, the menus' key and
pointer read (`port_text.h`, `port_text.cpp`, 41 functions); the 2D drawing
-- rectangles, boxes, panels, frames and lines, the 640 by 480 quads scaled
to the screen, the 2D screens' font, texts and titles, the menus' pointer
(`port_draw2d.h`, `port_draw2d.cpp`, 22); the shapes -- an FSH shape's alpha
scanned and its pixels converted and resized into a texture's format, the
menus' art made textures a tile at a time, found by name and drawn
(`port_shapes.h`, `port_shapes.cpp`, 40; the race's textures load through
them too). Their x87 arithmetic is the x87's to the bit, through the
emulated FPU in the game's order. `PORT_TEXT_NATIVES`, `PORT_DRAW2D_NATIVES`
and `PORT_SHAPES_NATIVES` bind them; `tools/game_checks` holds each against
its code (`text.inc`, `draw2d.inc`, `shapes.inc`).

Then the menus' widgets, the second layer: what each widget of the registry
at 0x557a84 does as its item is shown, drawn, keyed, laid out and hidden.
The text and image items, buttons, cascades, drop and select lists, check
boxes, overlays, the speech and rectangle items and the text box
(`port_buttons.h`, `port_buttons.cpp`, 62 functions); the fields typed into
(a string, a number, a character, a float, a fixed point), the sliders and
two-axis sliders, the list of lines and the car's colour chooser
(`port_fields.h`, `port_fields.cpp`, 108); a 3D object on a menu screen, the
car shown with its lights (`port_obj3d.h`, `port_obj3d.cpp`, 33). They call
the menu loop and the dialogs, which are not rewritten yet, by address.
`PORT_BUTTONS_NATIVES`, `PORT_FIELDS_NATIVES` and `PORT_OBJ3D_NATIVES` bind
them; `tools/game_checks` holds each against its code (`buttons.inc`,
`fields.inc`, `obj3d.inc`). Where one rewrite calls another's address, the
call is stood in on both sides (`STOOD_IN` in `generate.py`): a rewrite's
call into a rewrite runs the real code on the generated side, which a
made-up world cannot always feed. The widgets that belong to one screen
(controls, colour wheel, records, tournament, pursuit, summary, compare,
performance, the HUD editor's) come later with their screens.

The third layer is the menu files: the loader that reads `FeData\Menus\*.mnu`
into the tables of screens and items, the parser's lookups it reads with, the
item lookup's codelink match, a screen's handlers by its name, the editor's
checks of the files and the path function the file's name is built with
(`port_menus.h`, `port_menus.cpp`, 18 functions, `PORT_MENUS_NATIVES`; held
by `menus.inc`). These were hooks into generated bodies
(`native_frontend_menus.cpp`); no generated code stays behind them now.

The fourth layer is the menu loop: the screen shown and the screens gone
through, the items shown, drawn and passed, the selection moved by the keys and
the mouse, a key handed to the item that has it, the race settings kept
consistent, the help box, the loop itself and the answers of the dialogs it
opens (`port_loop.h`, `port_loop.cpp`, 32 functions, `PORT_LOOP_NATIVES`; held
by `loop.inc`). The item and screen handlers it calls through the tables, the
drawing, the text and the dialogs are stand-ins on both sides of the check.

The dialogs over the screens are rewritten too (`port_dialogs.h`,
`port_dialogs.cpp`, 27 functions, `PORT_DIALOGS_NATIVES`; held by `dialogs.inc`):
which dialog is up and of what kind, what it draws (its box, lines, buttons, the
field typed in, a list's entries, the help box) in the x87's own arithmetic,
the pointer on its buttons and entries, each kind's keys and pass, a list
opened, and the dialog's own key and pass, which hand an answer to its
callback. What opens a question, a message and a line to type in and lays out
their boxes stays native whole (`native_frontend_dialogs.cpp`, `MENU_NATIVES`).

### Decompiled menus

The menus are being decompiled too, a screen at a time — not for speed, but to
be read and changed in C++. The menu engine finds a screen's handlers by its
name in a table at `0x555c48` (57 screens, one per `MENUS/*.MNU`): one as the
screen opens, one as it closes, one on every pass of the menu loop. A screen's
handlers and the functions only it calls go to `src/nfs3hp/native_frontend_*.cpp`,
bound to their addresses with no generated code behind them. All 57 screens and
the code under them are native now, and the race's HUD and the simulation's last
generated functions went the same way (`native_frontend_hudr`, `hudx`, `sima`,
`simb`, `simc`), listed by `tools/frontend_checks/groups/*.txt`. They are C++ but
not yet typed: they read and write guest memory and call by address.

The menus' state stays in guest memory, where the rest of the game reads it,
and what the engine does for a screen — find an item by its codelink, a line of
the language's text, a dialog box — is a call into the game's own function at
its address (`src/nfs3hp/native_frontend.h`, which also lays out a screen and
an item: the fields the `.MNU` keywords fill are in the parser's table at
`0x557c10`). A decompiled function leaves memory and registers as the generated
one does, and calls the game in a frame of the same size, so even the stack
addresses the game's functions see are the same; each was compared with the
generated one byte for byte over all of guest memory, in every branch, before
it went in.

Done so far: the main menu in its three forms (`native_frontend_main.cpp`) —
`MAIN.MNU` for one player, `MAINSPLT.MNU` for split screen, `MAINMULT.MNU` for
a network race: the items fitted to the kind of race, the players' name
dialogs, the attract-mode movie, the network screen's race list and its
question before leaving. It is the first screen native whole: its 17
handlers are bound to their addresses with no generated code behind them
(`MENU_NATIVES` in `disassemble_nfs3hp.py`), and `tools/frontend_checks`
compares them with the game's code made afresh from `nfs3.exe`, keeping the
port's change among them — mainsplt's rows stand higher while it is up. The
menus' text it asks for — a line of the language's by its number, the names
of display modes, cars and controllers past them, and the check of a
player's name — is native whole too (`native_frontend_text.cpp`,
`FRONTEND_NATIVES`; the language's lines themselves `port_text.cpp`'s),
compared by `tools/game_checks`. So is all the rest of
what the main menu calls of the menus' own: of the menu loop, a screen's
items shown and the shown screen's name (`native_frontend_loop.cpp`; the loop itself is `port_loop.cpp`'s); of the
dialogs, a question, a message and a line to type in as it opens them, their
boxes and buttons laid out and the pointer put on the button highlighted
(`native_frontend_dialogs.cpp`; the rest of them is `port_dialogs.cpp`'s) — these in `MENU_NATIVES`, compared by
`tools/frontend_checks` — and under them, in `FRONTEND_NATIVES`, compared by
`tools/game_checks`: the checks of a line typed in
(`native_frontend_text.cpp`; the font a text is measured and drawn in, a
character at a time as a quad of its texture, is `port_text.cpp`'s), the pointer moved and the screens' art loaded
(`native_frontend_screen.cpp`). What is left under the main menu is the
game's: its sound, its art and files, its drawing, the attract-mode movie
and the network.

Then the options screens (`native_frontend_options.cpp`): `OPTIONS.MNU` and
the screens it leads to, most of them a race's pause screens too — the sound
(its five volumes, which play as their slider is held), the audio feed's
keys for the time, the position and the speed in a race (`audfeed`), the
function keys,
the controllers (their reset to the defaults, the dead zones of each axis, force
feedback) and the graphics. A screen keeps its settings as it opens and its
Cancel puts them back. Their 54 functions — the screens' handlers, the
handlers they give their items (a key, drawing, a slider's moves) and the few
only they call — were compared with the generated ones by
`tools/frontend_checks/run.sh`, the same way as the loop below, and keep the
port's one change among them: the effects' intensity slider is never greyed
out (`tools/apply_menu_unlock.py`). The controls' screens among them — the
function keys, the fixed ones, the controllers, their dead zones and force
feedback, 28 functions — are native whole now (`MENU_NATIVES`).

Then the screens a race is chosen on. The cars' (`native_frontend_cars.cpp`):
each player's car (`CAR1.MNU`, `CAR2.MNU` -- its colour, its showcase, the
way to its tuning), the opponents (`OPPONENT.MNU` -- how many, which car,
traffic) and a car's tuning (`CEDBASIC.MNU` -- its seven sliders, their
defaults and the advanced tuning they may differ from, from a player's car or
a race's pause screens). The track's (`native_frontend_tracks.cpp`): the
track and its laps, direction, mirror image, night and weather
(`LOCATION.MNU`, greyed in a series) and its picture, its map with its places
named and its spoken description (`TRKINFO.MNU`). With them, the developer's
recorder the car and track screens keep, which saves the turning car or the
track frame by frame as `.tga` files. Their 55 functions were compared by the
same `tools/frontend_checks/run.sh`, and keep the port's changes among them:
Download Car stays hidden (`tools/apply_hide_download.py`) and the phone's
keyboard comes up while the game waits for a typed name (`nfs3hp::textEntry`).

The engine goes the same way, a layer at a time. First
(`port_menus.cpp` now; once `native_frontend_menus.cpp`), what reads `FeData\Menus\*.mnu` into the table
of screens (`0x6040d0`) and the table of items (`0x604ee0`): opening a screen
reads its file and, by its buttons' `nextmenu`, every screen reachable from
it; a section (`[button]`) starts an item from its widget's template, a key
fills the item's field the parser's table names, by its kind (a number, a
pooled string, a line of the language's text, a float); names are matched
case and all. The original's quirks are kept: a number with any character
other than a digit in it, a minus included, reads as 0, and a line that is
neither a section nor a key with a value stops the game. With the loader came
the item lookup the screens use (an item by a name in its codelink) and the
screen's handlers found by its name.

Then the menu loop (`port_loop.cpp` now; once `native_frontend_loop.cpp`), from the first screen until
the player leaves the menus. Each screen is opened (the art, the items' states
from their widgets', its onEnter) and passed over and over — its onFrame, the
items drawn layer by layer and given their own pass, the help box, the item
under the pointer selected, a key handed to the selected item or, when that
does not take it, Escape pressing the screen's Escape button and the arrows
moving the selection — until something leaves it: back through the 20
screens the history keeps, on to a button's `nextmenu` (asking for the
players' names first when they have none), into the race, or out (asked
first). The race settings are kept consistent on every pass.

Then the dialogs over the screens (`port_dialogs.cpp` now; once `native_frontend_dialogs.cpp`): a question
and its buttons, a message that goes away by itself after its time, a line to
type a name in, a list of entries to tick. One is up at a time; the menu loop
hands it each pass and each key, and an answer — Enter, Escape, the message's
time run out — goes to the dialog's callback, which can keep it up. Its box is
laid out as it opens, in floats, in the x87's own arithmetic
(`fe::X87`): in a race's pause screens that rounds to a float's precision and
the box lands where the game puts it, to the pixel. The NFS emblem on the box
keeps the wide-screen fix (`tools/apply_widescreen.py`). The widgets come next.

The loop was compared in a world of stand-ins: the generated functions that
draw, wait for the next frame, read the keys, play music or talk to the
network replaced by ones that record every call and answer from a script, the
screens' and the items' handlers too. Besides memory and registers, the
decompiled loop had to make the same calls, with the same arguments, in the
same order. The dialogs were compared the same way, besides the x87's status
and control words, in each of its precisions and rounding modes; and as a
whole, the decompiled dialogs calling one another against the generated ones.

`NFS_FE_NATIVE=0` goes back to the generated code for every decompiled
function that still has it behind it, `NFS_FE_NATIVE_OFF=sub_437e00,...` for
the ones named; the log says which ran which way (`[FE]`). The screens that
are native whole have none to go back to.

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
| `WITH_MMX` | `OFF` | The checks' only: the game's code they hold the port against still has MMX, `cpuid` and `rdtsc`. |
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
| `NFS_GAMMA`, `NFS_BRIGHTNESS`, `NFS_CONTRAST` | `1.0` | Applied to the finished frame in the final blit, so menus and movies are covered as well as a race. |
| `NFS_ORIENTATION` | unset | `auto` allows both landscape directions as the rotation lock allows; anything else pins one. Must be set before `SDL_Init`. |
| `NFS_CAR_DETAIL_FULL` | on | With Car Detail at High, every car keeps its detailed model and the player's texture size out to the draw distance, in every view, and the limit on how many are drawn at once is lifted. `0` restores the original. |
| `NFS_MIRROR_FULL` | on | The rear-view mirror is drawn as the main view is: its distances, the track's detailed models, cars at full detail, the sky with clouds and lightning, car lights and the headlights' light. `0` restores the original short, coarse mirror. |
| `NFS_WIDESCREEN` | on | `0` takes the 1280x720, 1600x900 and 1920x1080 modes back out of the Screen Size list. |
| `NFS_TEXTURES32` | on | `0` has the Voodoo2 driver refuse 32-bit textures again, so the game shrinks them to 16 bits as it used to. |
| `NFS_TEXCOORD_REBASE` | on | `0` leaves far-out texture coordinates as the game sends them (see Projected headlights), to compare. |
| `NFS_DEPTH16` | auto | On where the depth buffer has more than 16 bits and the GPU is not an Adreno 5xx. `0` keeps the GPU's own depth precision, `1` forces the Voodoo's 16-bit steps (see Projected headlights); the `[GPU]` lines count the frames drawn in them. |
| `NFS_NATIVES` | on | `0` runs the generated code instead of the car-specs native, to compare speeds. The loading, movie, vertex, polygon and drawing natives always run. |
| `NFS_MAD_CHECK` | off | `1` (debug) runs the generated MAD player and checks the native decoder (`src/nfs3hp/native_movie.cpp`) against the generated one ([MAD]). |
| `NFS_LOG` | on | `0` (debug mode off, the launcher's Advanced screen) writes nothing to the log: log priority critical, and SDL's direct Android logging held back (`android/app/jni/sdl_quiet.h`). With debug mode off the launcher sets none of the diagnostic variables. |
| `NFS_NET_PORT` | `9803` | TCP port of network races, as the Modern Patch has it; the launcher's Network screen offers the original's `1030`. |
| `NFS_NET_TRACE` | off | `1` (extra `net_trace`) writes a network race into the log: EA's comm library events and packet sends (`[COMM]`), up to 400 datagrams per socket instead of 16 (`[NET]`), and each new indirect call target once per thread after the IPX transport opens (`[CALL]`). For finding where two machines stop on the way into a race. |
| `NFS_FE_NATIVE` | on | `0` leaves every hooked decompiled menu function, the screens' and the engine's, to the generated code (see Decompiled menus); the main menu, what it calls of the menu loop and the dialogs, and the controls' screens are native whole and stay native. |
| `NFS_FE_NATIVE_OFF` | unset | A list of generated functions, `sub_437e00,sub_438260`, left to the generated code while the other decompiled ones run. |
| `NFS_REPLAY_TRACE` | off | `1` (debug mode, Traces: `replay_trace`) every car at every physics step to `logs/trace-*.bin` (see Comparing builds by a replay). |
| `NFS_AUTO_REPLAY` | off | `1` (debug mode, launch extra `auto_replay`) no movies, and from the first menu straight into the replay in `replay.rp3`. |
| `NFS_PERF_TRACE` | off | `1` (debug mode, Traces: `perf_trace`, "Missing performance data logging") follows player one's 0-100 and the other timings from the car (`sub_4cdf10`) through the race's take-down to the results' copy (`sub_455410`), for the race summary's Performance page showing `---`: `[PERF]` when the car's change, `[PERFTIME]` at each step of `takeDownRace` and when the results' records change. |
| `NFS_FUNC_LOG` | off | `1` (debug mode, switch `live_counter`) samples which functions run, to `functions.bin` (see Live functions log). |
| `NFS_NATIVE_CHECK` | off | `1` (or `load`) runs each call of the loading natives (`src/nfs3hp/native_loading.cpp`: RefPack, FCE models, FRD tracks) both ways -- the native, everything it wrote put back, then the generated code -- and logs any difference in memory, registers or flags (`[NATIVE]`), holding the guest context meanwhile (`WinApplication::KeepContext`). The vertex, polygon and drawing natives have no generated code left in the game to be held against: their checks are on the desktop (`tools/game_checks/vertices`, `tools/game_checks/render`). Launch extras: `--ez native_check true`, `--ez load_check true`. |
| `NFS_AUDIO_RATE` | unset | The output's own sample rate. The game mixes at 22050 Hz and SDL converts to this; a device opened at any other rate is resampled by Android, off its low-latency path. Unset, the device opens at SDL's default. |
| `NFS_TOUCH_STEER_LEFT` etc. | unset | Keys the touch overlay sends for steering and the pedals, so that endpoint can report them as axes. |
| `NFS_GAMEPAD1_SOUTH` etc. | built-in defaults | What a pad button sends, as `NFS_GAMEPAD<n>_<BUTTON>`: an SDL key name (`Space`, `Return`), `axis:steer_left`, `axis:steer_right`, `axis:accelerate`, `axis:brake`, or empty for nothing. `_UI` after it is the same button in the menus. `LEFT_TRIGGER` and `RIGHT_TRIGGER` are the triggers read as buttons, empty by default; the launcher gives them the pedals' keys for a pad set to Digital. |

## Regenerating the recompilation

The repository ships the generated C++, so this is only needed if you change the
disassembler in `disasm/`:

```bash
pip3 install capstone
python3 disassemble_nfs3hp.py
```

It reads `nfs3hp/nfs3.exe` and the three DLLs and rewrites
`src/nfs3hp/disassembly`.

The generated code has no x87 behind it (`disasm/codegen/fpu_frame.py`). The depth
of the x87 stack at every instruction follows from the code, so each function
keeps its stack in C++ locals, one for each depth, and its status word in another;
the arithmetic is `fp::add`, `fp::mul` and so on, and comparisons give back the
word `fnstsw` reads (`include/fpu.h`). What is left of the emulated FPU is what
functions hand each other: a function's table of what it takes from the stack and
how far it moves it is worked out from the code to a fixed point, the natives by
the code they replace, and of about 6,000 functions 117 pass anything that way
(sin, atan2, a helper returning a float). Only those values go through
`cpu.fpu.push` and `pop`, and the control word stays for the few functions that
load one. `NFS_FPU_LOCALS=0` in the generator's environment writes every instruction
against the emulated FPU, to compare. A function whose depth does not follow from
its code keeps the older form, the stack in locals only through straight runs
(`disasm/codegen/fpu_stack.py`); one dead routine of the runtime is left of those.

## Credits

- [motor-dev/nfs-recompiled](https://github.com/motor-dev/nfs-recompiled) — the
  recompilation this is built on.
- [libsdl-org/SDL](https://github.com/libsdl-org/SDL) — SDL3, zlib licence.
- [americusmaximus/AZHP](https://github.com/americusmaximus/AZHP) — AutoZone Hot
  Pursuit, MIT licence.

Need for Speed III: Hot Pursuit is the property of Electronic Arts. This
repository contains no game content — no tracks, cars, audio or video. Those you
bring from your own disc.
