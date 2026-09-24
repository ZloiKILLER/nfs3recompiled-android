# NFS III: Hot Pursuit — Android: technical details

An Android port of the static recompilation of **Need for Speed III: Hot Pursuit**.
The original 1998 Win32 x86 executable is translated into portable C++ that runs
on a virtual x86 CPU, with the Win32, DirectDraw, DirectInput, DirectSound and
Glide 2x calls it makes reimplemented on top of SDL3 and OpenGL ES.

Based on [motor-dev/nfs-recompiled](https://github.com/motor-dev/nfs-recompiled),
which does the recompilation itself and targets desktop. This fork adds the
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

Android 8.0 or newer on a 64-bit ARM device. Testing happens on Android 13, and
Android 10 was confirmed by a tester; 8 and 9 install but have not been played
on yet.

1. Put the game data folder on the device, e.g. `/sdcard/nfs3-og`.
2. Launch the app, import that folder, make it the active data set.
3. Play.

An import leaves the game ready for a phone: the gamepad control set (see
Controls), View Distance at Full, races at 1280x720 and a HUD arranged for a
phone — the standings and the maps in the corners rather than under your thumbs
— are written into its `config.dat` once, as the last step. From then on the
file is the player's, and the game's own Heads Up Display screen rearranges it.

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

## Known issues

- **Mosaic artefacts in the headlight-lit area on Mali GPUs.** Blocky patches
  appear where the projected headlight texture falls on the road, only while
  moving. Not reproducible on Adreno with the same build, with or without
  mipmapping, so it looks like a driver difference rather than a bug in the
  Glide layer. Unresolved.

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
| `NFS_WIDESCREEN` | on | `0` takes the 1280x720, 1600x900 and 1920x1080 modes back out of the Screen Size list. |
| `NFS_TEXTURES32` | on | `0` has the Voodoo2 driver refuse 32-bit textures again, so the game shrinks them to 16 bits as it used to. |
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

## Credits

- [motor-dev/nfs-recompiled](https://github.com/motor-dev/nfs-recompiled) — the
  recompilation this is built on.
- [libsdl-org/SDL](https://github.com/libsdl-org/SDL) — SDL3, zlib licence.
- [sse2neon](https://github.com/DLTcollab/sse2neon) — SSE2 intrinsics on ARM,
  MIT licence, vendored as `third_party/sse2neon.h`.

Need for Speed III: Hot Pursuit is the property of Electronic Arts. This
repository contains no game content — no tracks, cars, audio or video. Those you
bring from your own disc.
