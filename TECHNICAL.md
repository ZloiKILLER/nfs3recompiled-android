# NFS III: Hot Pursuit — Android: technical details

An Android port of the static recompilation of **Need for Speed III: Hot Pursuit**.
The original 1998 Win32 x86 executable is translated into portable C++ that runs
on a virtual x86 CPU, with the Win32, DirectDraw, DirectInput, DirectSound and
Glide 2x calls it makes reimplemented on top of SDL3 and OpenGL ES.

Based on [motor-dev/nfs-recompiled](https://github.com/motor-dev/nfs-recompiled),
which does the recompilation itself and targets desktop. This project adds the
Android target: a launcher, touch controls, gamepad handling, widescreen races,
and a number of fixes in the Glide layer that only showed up once the game ran
on a phone.

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

Android 8.0 or newer on a 64-bit ARM device (ARMv8, arm64-v8a) with OpenGL ES
3.0. Testing happens on Android 13, and
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
effects are felt: off, the phone or the gamepad) and **Display** with **Screen
adjustment** (orientation, frame rate cap, gamma, brightness, contrast).

The menus carry no buttons of the port's at all: a finger works them as a
pointer, and the system's own back is the game's Escape. When the game asks for
a name, a tap on the line being typed in brings up the phone's keyboard and a
tap beside it puts it away; a box opened from a pad brings the keyboard up at
once, and with a real keyboard connected it never comes up. A tap on the
player's name beside its tab opens the same box. A pad's buttons follow the same
two worlds — the racing set while a race is driven, confirm and back everywhere
else.

### Controls

The game always sees two DirectInput joysticks, `NFS Gamepad 1` and
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
(`src/lib/winapi/dinput/idirectinputeffect.cpp`) and translated to rumble. Only
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

Options → Graphics → Screen Size offers 1280x720, 1600x900 and 1920x1080
beside the original 4:3 modes, and the choice survives a restart. A race then
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
itself, and that pass hands the Glide layer texture coordinates up to thirty
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
join into the log in a debug build.

### The x87's precision

A PC of 1998 raced in single precision: before every race the game takes the
x87's precision control down to 24 bits (fldcw at 0x4a3e8d and 0x4a3ed3), and
every sum, product and quotient of the physics and of the opponents' driving
keeps a float's significand. Four functions that step out to the system mid-race
come back the same way (0x495179, 0x4955e9, 0x4b6239, 0x4bc811). The Modern
Patch drops that fldcw and races at the 64 bits fninit leaves. The port does as
the Modern Patch does at all six (`tools/apply_race_precision.py`);
`NFS_FPU_EXTENDED=0` brings back the original's single precision. Every machine
of a network race needs the same.

How wide "extended" is depends on the build. The default keeps the x87's
values in doubles: 53 bits. `WITH_PEDANTIC_FPU` (`./gradlew -Px87Exact
assembleDebug`) keeps them in 80 bits and does the x87's arithmetic in portable
C++ (`src/lib/x87soft.cpp`): addition, subtraction, multiplication, division,
square root, comparisons, frndint, fscale, fprem and every conversion come out
as an x87's to the bit, at every precision control and in every rounding mode
(`tools/x87soft_checks.cpp` compares a million random operands of each with a
real x87), and fsin, fcos, fptan, fpatan, fyl2x and f2xm1 within a last bit of
it — an arm64 long double has 113 bits, and the sine and cosine reduce their
argument by the x87's own 66-bit pi. It costs speed: every x87 operation of
the generated code is a call.

In an extended race the natives that only draw stay on; the ones the physics
and the opponents can reach — the vector helpers, the nearest block, the
ground's height, the particles — step aside for the generated code.

## Native code

The game runs on a virtual x86 CPU, and the parts that cost the most no longer
go through it:

- voodoo2a's THRASH driver, the game's renderer, is native C++ for every
  function the game uses (`src/nfs3hp/native_thrash.cpp`), drawing through its
  own renderer (`src/lib/thrashrenderer.cpp`): an OpenGL texture for each of
  the game's, the texture formats as they are, hardware filtering.
- About fifty of the game's own functions are native
  (`src/nfs3hp/native_vertices.cpp`): turning and projecting vertices,
  clipping, the track's and objects' polygons and their depth sorting,
  particles, the cars' lights and detail, and the small vector helpers called
  from everywhere. `tools/apply_native_vertices.py` hooks each at the top of its
  generated function, which stays behind it as the fallback.
- The rest of the drawing goes native function by function
  (`src/nfs3hp/native_render.cpp`, hooked by `tools/apply_native_render.py`):
  so far the clipping of a triangle whose corners carry z (`sub_4bf790`), a
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
- eacsnd's sound driver (`src/nfs3hp/native_sound.cpp`): what the game mixes
  goes straight onto an SDL audio stream, with no DirectSound and no ring
  buffer in between. `NFS_SND_NATIVE=0` goes back to eacsnd's generated code.
- The game's sound mixer (`src/nfs3hp/native_mixer.cpp`): all of a sample's
  way through EA's SND library -- PCM and XA ADPCM decoding, pitch, volume and
  pan, the race's reverb, the 16 channels added up -- done as the x86 does it,
  to the bit. Starting and stopping sounds and reading streams stay the
  game's. `NFS_SND_MIX=0` goes back to the generated mixer; `NFS_SND_CHECK=1`
  mixes every block both ways and compares them.
- The loading of the game's data (`src/nfs3hp/native_loading.cpp`, hooked by
  `tools/apply_native_loading.py`): EA's RefPack decoder behind every `.qfs`,
  the alpha scan each FSH texture gets in each of its pixel formats, an FCE
  model turned into the game's polygon records, and a track's FRD read into
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
- Two functions of the opponents' driving are decompiled into C++ to be read
  and changed (`src/nfs3hp/native_ai.h`): sub_4064f0, the pull of an
  opponent's engine at its speed, and sub_4070c0, how far the way it wants to
  go is from the way it points. They come out as the generated code to the
  bit in every precision and both builds (`tools/native_ai_checks.py`);
  `NFS_NATIVE_AI=0` runs the generated code instead.

Each was compared with the generated code bit for bit before it was switched
on — memory, registers, flags, the FPU and the triangles drawn
(`NFS_NATIVE_CHECK`) — and keeps the x87's single-precision rounding. In a race
this is about three quarters of the game thread's time. `NFS_NATIVES=0`,
`NFS_THRASH=0` and `NFS_THRASH_GL=0` go back to the generated code.

### Decompiled menus

The menus are being decompiled too, a screen at a time — not for speed, but to
be read and changed in C++. The menu engine finds a screen's handlers by its
name in a table at `0x555c48` (57 screens, one per `MENUS/*.MNU`): one as the
screen opens, one as it closes, one on every pass of the menu loop. A screen's
handlers and the functions only it calls go to `src/nfs3hp/native_frontend_*.cpp`,
hooked at the top of their generated functions by
`tools/apply_native_frontend.py`, the generated code kept behind them.

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
question before leaving.

The engine goes the same way, a layer at a time. First
(`native_frontend_menus.cpp`), what reads `FeData\Menus\*.mnu` into the table
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

Then the menu loop (`native_frontend_loop.cpp`), from the first screen until
the player leaves the menus. Each screen is opened (the art, the items' states
from their widgets', its onEnter) and passed over and over — its onFrame, the
items drawn layer by layer and given their own pass, the help box, the item
under the pointer selected, a key handed to the selected item or, when that
does not take it, Escape pressing the screen's Escape button and the arrows
moving the selection — until something leaves it: back through the 20
screens the history keeps, on to a button's `nextmenu` (asking for the
players' names first when they have none), into the race, or out (asked
first). The race settings are kept consistent on every pass.

Then the dialogs over the screens (`native_frontend_dialogs.cpp`): a question
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
function, `NFS_FE_NATIVE_OFF=sub_45d050,...` for the ones named; the log says
which ran which way (`[FE]`).

## Building

### Android

Requires Android SDK with **NDK 28.2.13676358**, build-tools 36.0.0, compileSdk
36, and a JDK (Android Studio's bundled JBR works). The app is arm64-v8a only,
minSdk 26, so it installs on Android 8.0 and later. The floor is `java.nio.file`,
which the launcher reads and writes the game's files with; every platform call
above 26 is behind a version check with a fallback.

SDL3 is built from source on Android and is not vendored here — clone it first:

```bash
git clone --depth 1 --branch release-3.4.2 https://github.com/libsdl-org/SDL third_party/SDL
```

Then build:

```bash
cd android
./gradlew assembleDebug
```

The APK lands in `android/app/build/outputs/apk/debug/`. `assembleRelease`
builds unsigned, so align and sign it yourself with `zipalign` and `apksigner`.

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
| `WITH_MMX` | `ON` | MMX bit in the emulated CPUID. Leave it on: reported absent, the game picks different copy routines and the movie streamer breaks. |
| `WITH_PEDANTIC_FPU` | `OFF` | Every x87 value in 80 bits and the x87's own arithmetic to the bit (`src/lib/x87soft.cpp`). Any GCC or Clang build, Android included (`-Px87Exact`); not MSVC. |
| `NFS_TRACE_MSG` | `OFF` | Message-pump and Glide state tracing. Very verbose. |
| `NFS2_ASSERT_TRAP` | `OFF` | Unsupported-path asserts break into the debugger instead of logging. |
| `NFS_BUILD_FF_TESTS` | `OFF` | Build `force_feedback_checks`, the DirectInput force-feedback regression test, against an SDL virtual joystick. |
| `NFS_BUILD_MEMORY_TESTS` | `OFF` | Build the guest memory allocator checks and `glidetmu_checks`, the texture atlas regression test. |

## Environment variables

Read once at startup. On Android the launcher sets `NFS_ORIENTATION`,
`NFS_FPS_CAP`, `NFS_GAMMA`, `NFS_BRIGHTNESS`, `NFS_CONTRAST`, the `NFS_TOUCH_*`
keys and one variable per pad button from its own screens, and `NFS_AUDIO_RATE`
from the phone; the rest matter for a desktop run.

| Variable | Default | Description |
|---|---|---|
| `NFS_FPS_CAP` | `30` | Frame rate the renderer paces presents to. The game's logic is tuned for 30 Hz. |
| `NFS_GAMMA`, `NFS_BRIGHTNESS`, `NFS_CONTRAST` | `1.0` | Applied to the finished frame in the final blit, so menus and movies are covered as well as a race. |
| `NFS_ORIENTATION` | unset | `auto` allows both landscape directions; anything else pins one. Must be set before `SDL_Init`. |
| `NFS_CAR_DETAIL_FULL` | on | With Car Detail at High, every car keeps its detailed model and the player's texture size out to the draw distance, in every view, and the limit on how many are drawn at once is lifted. `0` restores the original. |
| `NFS_MIRROR_FULL` | on | The rear-view mirror is drawn as the main view is: its distances, the track's detailed models, cars at full detail, the sky with clouds and lightning, car lights and the headlights' light. `0` restores the original short, coarse mirror. |
| `NFS_WIDESCREEN` | on | `0` takes the 1280x720, 1600x900 and 1920x1080 modes back out of the Screen Size list. |
| `NFS_TEXTURES32` | on | `0` has the Voodoo2 driver refuse 32-bit textures again, so the game shrinks them to 16 bits as it used to. |
| `NFS_TEXCOORD_REBASE` | on | `0` leaves far-out texture coordinates as the game sends them (see Projected headlights), to compare. |
| `NFS_DEPTH16` | auto | On where the depth buffer has more than 16 bits and the GPU is not an Adreno 5xx. `0` keeps the GPU's own depth precision, `1` forces the Voodoo's 16-bit steps (see Projected headlights). `ab` switches between the two every 5 s, so both can be compared in one race; the `[GPU]` lines count the frames of each. |
| `NFS_GPU_FILTER` | on | `0` filters every texel in the shader. By default the GPU's bilinear filter takes a pixel whose 2x2 texels lie inside their atlas tile and have no chroma key. |
| `NFS_TEXCOORD_CUT` | off | `1` cuts triangles with far-out clamped texture coordinates along the texture's edges. An experiment, not needed for the headlights. |
| `NFS_NATIVES` | on | `0` runs the generated code instead of every native stand-in for the hot vertex and polygon loops; `vertices` keeps only the three vertex loops; `no-drawtri` all but THRASH_drawtri; `no-clip` all but the clipping of off-screen polygons (`sub_4c2cd0` and its callers `sub_4c11b0`, `sub_4c12e0`); `no-render` all but the ones in `src/nfs3hp/native_render.cpp`. For comparing speeds. |
| `NFS_RENDER_OFF` | unset | Addresses of functions in `src/nfs3hp/native_render.cpp` to leave to their generated code, e.g. `4bf790,47a880`. |
| `NFS_NATIVE_LOAD` | on | `0` leaves the loading of the game's data to the generated code: RefPack, the FSH textures' alpha scans, FCE models and FRD tracks (`src/nfs3hp/native_loading.cpp`). `NFS_NATIVE_CHECK=load` checks only these, every call. |
| `NFS_MAD` | on | `0` plays the movies with the generated MAD player, decoder and display instead of `src/nfs3hp/native_movie.cpp`; `NFS_MAD_PLAYER=0` and `NFS_MAD_GL=0` leave just the player or the full-colour output generated. `NFS_MAD_CHECK=1` (debug) runs the generated player and checks the native decoder against the generated one ([MAD]). |
| `NFS_THRASH` | on | `0` leaves voodoo2a's THRASH functions (the game's renderer driver) to their generated code instead of the native ones in `src/nfs3hp/native_thrash.cpp`. All of them the game uses: drawing, state, textures, the frame, setting up and video modes. `NFS_THRASH_OFF=THRASH_setstate,...` leaves the named ones generated. `NFS_NATIVE_CHECK=thrash` checks only these. |
| `NFS_THRASH_GL` | on | `0` draws through the old renderer, the Voodoo2 emulated over a texture atlas with its filtering done in the shader, instead of `ThrashRenderer`, which gives each of the game's textures an OpenGL texture of its own and lets the GPU filter, wrap and clamp it. |
| *(release)* | | A release build writes nothing to the log and takes none of these flags: the native code is built with `NFS_RELEASE` (log priority critical, SDL's direct Android logging compiled out through `android/app/jni/sdl_quiet.h`), the Java side logs through `AppLog`/`SDLLog` only when `BuildConfig.DEBUG`. Use the debug build (`-Pandroid10`) for diagnostics. |
| `NFS_FPU_EXTENDED` | on | Races at the x87's extended precision, as the Modern Patch runs them; `0` races in single precision, as the original (see The x87's precision). Every player of a network race needs the same. A debug build takes it from a launch: `adb shell am start -n dev.nfs3hp.port/.SplashActivity --ez fpu_single_races true`. |
| `NFS_NATIVE_AI` | on | `0` runs the generated sub_4064f0 and sub_4070c0 instead of their decompiled C++ (`src/nfs3hp/native_ai.h`). A debug build takes it from a launch: `--ez native_ai_off true`. |
| `NFS_NET_PORT` | `9803` | TCP port of network races, as the Modern Patch has it; the launcher's Network screen offers the original's `1030`. |
| `NFS_NET_TRACE` | off | `1` (extra `net_trace`) writes a network race into the log: EA's comm library events and packet sends (`[COMM]`), up to 400 datagrams per socket instead of 16 (`[NET]`), and each new indirect call target once per thread after the IPX transport opens (`[CALL]`). For finding where two machines stop on the way into a race. |
| `NFS_FE_NATIVE` | on | `0` leaves every decompiled menu function, the screens' and the engine's, to the generated code (see Decompiled menus). |
| `NFS_FE_NATIVE_OFF` | unset | A list of generated functions, `sub_45d050,sub_45cff0`, left to the generated code while the other decompiled ones run. |
| `NFS_NATIVE_CHECK` | off | `1` runs each native stand-in against the generated code and logs any difference (`[NATIVE]`). The polygon and car loops are compared over all of guest memory once a second. Slow, and draws some triangles twice. `clip` checks only the stand-ins of the clipping (both ways to the screen, the car's quad mesh included), the track, the effects, the car lights and detail, the objects and the vector math, every call; the small helpers called from hundreds of places are checked on every 64th call. |
| `NFS_AUDIO_RATE` | unset | The output's own sample rate. The game mixes at 22050 Hz and SDL converts to this; a device opened at any other rate is resampled by Android, off its low-latency path. Unset, the device opens at SDL's default. |
| `NFS_SND_NATIVE` | on | `0` leaves eacsnd's `iSNDdirect` functions (caps, start, stop, serve) to their generated code, which feeds the game's mix to the emulated DirectSound, instead of the native ones in `src/nfs3hp/native_sound.cpp`, which put it on an SDL audio stream themselves. |
| `NFS_SND_MIX` | on | `0` leaves the game's sound mixer (`sub_500864` and everything under it) to its generated code instead of `src/nfs3hp/native_mixer.cpp`. `NFS_SND_CHECK=1` (debug) mixes each block natively and with the generated code and logs any byte that differs (`[SNDCHK]`); a block that calls the game, to refill a stream or end a sound, is left to the generated code. |
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

Between two places where control can arrive or leave — a label, a jump, a
call, a return — the generator keeps the x87 stack in C++ locals rather than in
the emulated FPU's memory, and writes it back where the run ends
(`disasm/codegen/fpu_stack.py`). The arithmetic is the same calls in the same
order, so the results are the same to the bit; on a phone it is about a quarter
more frames in the heaviest split-screen races. `NFS_FPU_LOCALS=0` in the
generator's environment writes every instruction against the emulated FPU as
before, and `NFS_FPU_LOCALS_CHECK=1` plays every run through both ways,
symbolically, and stops on the first one that differs.

## Credits

- [motor-dev/nfs-recompiled](https://github.com/motor-dev/nfs-recompiled) — the
  recompilation this is built on.
- [libsdl-org/SDL](https://github.com/libsdl-org/SDL) — SDL3, zlib licence.
- [sse2neon](https://github.com/DLTcollab/sse2neon) — SSE2 intrinsics on ARM,
  MIT licence, vendored as `third_party/sse2neon.h`.

Need for Speed III: Hot Pursuit is the property of Electronic Arts. This
repository contains no game content — no tracks, cars, audio or video. Those you
bring from your own disc.
