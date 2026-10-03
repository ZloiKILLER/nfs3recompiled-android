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
on-screen controls, their layout editor, the key each one sends, the phone's
vibration switch), **Controls → Gamepads** (which pad is which player, the key
each button sends, the pad vibration switch) and **Display** with **Screen
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
the first force-feedback device gets them — Gamepad 1 and the phone — so Gamepad
2 never vibrates. A pad plays the game's jolts, road and engine; the phone plays
jolts, the engine by its revs, and how hard the car corners. Phone vibration is
switched on under Controls → Touch, a pad's under Controls → Gamepads, and how
strong a pad plays is set in the game's own Force Feedback menu. Nothing
vibrates on a button press.

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
inputs, so the port rounds the x87 as a PC does in a race, and the game's
version is checked when a machine joins. The port reports nfs3.exe's own,
"27, 2.0" — the version resource the phone has no copy of, and the string the
Modern Patch keeps in its data. Winsock 1.1 is in `src/lib/winapi/wsock32.cpp`
over the phone's sockets (`src/lib/socket.cpp`); `NFS_NET_TRACE=1` writes a
join into the log in a debug build.

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

Each was compared with the generated code bit for bit before it was switched
on — memory, registers, flags, the FPU and the triangles drawn
(`NFS_NATIVE_CHECK`) — and keeps the x87's single-precision rounding. In a race
this is about three quarters of the game thread's time. `NFS_NATIVES=0`,
`NFS_THRASH=0` and `NFS_THRASH_GL=0` go back to the generated code.

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
| `WITH_PEDANTIC_FPU` | `OFF` | Strict 80-bit x87 emulation through NASM helpers. Linux only. |
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
| `NFS_NATIVES` | on | `0` runs the generated code instead of every native stand-in for the hot vertex and polygon loops; `vertices` keeps only the three vertex loops; `no-drawtri` all but THRASH_drawtri; `no-clip` all but the clipping of off-screen polygons (`sub_4c2cd0` and its callers `sub_4c11b0`, `sub_4c12e0`). For comparing speeds. |
| `NFS_THRASH` | on | `0` leaves voodoo2a's THRASH functions (the game's renderer driver) to their generated code instead of the native ones in `src/nfs3hp/native_thrash.cpp`. All of them the game uses: drawing, state, textures, the frame, setting up and video modes. `NFS_THRASH_OFF=THRASH_setstate,...` leaves the named ones generated. `NFS_NATIVE_CHECK=thrash` checks only these. |
| `NFS_THRASH_GL` | on | `0` draws through the old renderer, the Voodoo2 emulated over a texture atlas with its filtering done in the shader, instead of `ThrashRenderer`, which gives each of the game's textures an OpenGL texture of its own and lets the GPU filter, wrap and clamp it. |
| *(release)* | | A release build writes nothing to the log and takes none of these flags: the native code is built with `NFS_RELEASE` (log priority critical, SDL's direct Android logging compiled out through `android/app/jni/sdl_quiet.h`), the Java side logs through `AppLog`/`SDLLog` only when `BuildConfig.DEBUG`. Use the debug build (`-Pandroid10`) for diagnostics. |
| `NFS_NET_PORT` | `9803` | TCP port of network races, as the Modern Patch has it; the launcher's Network screen offers the original's `1030`. |
| `NFS_NET_TRACE` | off | `1` (extra `net_trace`) writes a network race into the log: EA's comm library events and packet sends (`[COMM]`), up to 400 datagrams per socket instead of 16 (`[NET]`), and each new indirect call target once per thread after the IPX transport opens (`[CALL]`). For finding where two machines stop on the way into a race. |
| `NFS_NATIVE_CHECK` | off | `1` runs each native stand-in against the generated code and logs any difference (`[NATIVE]`). The polygon and car loops are compared over all of guest memory once a second. Slow, and draws some triangles twice. `clip` checks only the stand-ins of the clipping (both ways to the screen, the car's quad mesh included), the track, the effects, the car lights and detail, the objects and the vector math, every call; the small helpers called from hundreds of places are checked on every 64th call. |
| `NFS_AUDIO_RATE` | unset | The output's own sample rate. The game mixes at 22050 Hz and SDL converts to this; a device opened at any other rate is resampled by Android, off its low-latency path. Unset, the device opens at SDL's default. |
| `NFS_TOUCH_STEER_LEFT` etc. | unset | Keys the touch overlay sends for steering and the pedals, so that endpoint can report them as axes. |
| `NFS_GAMEPAD1_SOUTH` etc. | built-in defaults | What a pad button sends, as `NFS_GAMEPAD<n>_<BUTTON>`: an SDL key name (`Space`, `Return`), `axis:steer_left`, `axis:steer_right`, `axis:accelerate`, `axis:brake`, or empty for nothing. |

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
