# NFS III: Hot Pursuit — Android

An Android port of the static recompilation of **Need for Speed III: Hot Pursuit**.
The original 1998 Win32 x86 executable is translated into portable C++ that runs
on a virtual x86 CPU, with the Win32, DirectDraw, DirectInput, DirectSound and
Glide 2x calls it makes reimplemented on top of SDL3 and OpenGL ES.

Based on [motor-dev/nfs-recompiled](https://github.com/motor-dev/nfs-recompiled),
which does the recompilation itself and targets desktop. This fork adds the
Android target: a launcher, touch controls, gamepad handling, widescreen races,
and a number of fixes in the Glide layer that only showed up once the game ran
on a phone.

<img src="screenshots/race-city.jpg" alt="racing through the city" width="49%">
<img src="screenshots/race-boardwalk.jpg" alt="racing on the boardwalk" width="49%">
<img src="screenshots/car-select-front.jpg" alt="car selection" width="49%">
<img src="screenshots/car-select-rear.jpg" alt="car selection, rear view" width="49%">

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

1. Put the game data folder on the device, e.g. `/sdcard/nfs3-og`.
2. Launch the app, import that folder, make it the active data set.
3. Play.

The launcher holds what the game cannot ask for itself: **Controls → Touch** (the
on-screen controls, their layout editor, the key each one sends, the phone's
vibration switch), **Controls → Gamepads** (which pad is which player, the key
each button sends, the pad vibration switch) and **Display** with **Screen
adjustment** (orientation, frame rate cap, gamma, brightness, contrast).

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
keyboard defaults instead.

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

Options → Graphics → Screen Size offers `1280 x 720 x 16 (z)` beside the
original 4:3 modes. A race then opens the view sideways instead of stretching
it: the vertical angle stays what the game draws and the horizontal one widens
with the screen, in every view — chase and in-car, the mirror, both halves of
split screen. HUD elements drawn as pictures keep their shape, and the layout
saved in `config.dat` is never touched, so a 4:3 mode looks exactly as it did.
The menus stay 4:3; only the race switches mode. This part is unfinished, see
Known issues, and `NFS_WIDESCREEN=0` takes the mode back out wherever an
environment variable can be set — the launcher does not set this one.

Alpha intensity in Advanced Graphics works here as well. The original applies it
only on its Direct3D driver, so on this one the slider used to move and change
nothing.

## Known issues

- **Mosaic artefacts in the headlight-lit area on Mali GPUs.** Blocky patches
  appear where the projected headlight texture falls on the road, only while
  moving. Not reproducible on Adreno with the same build, with or without
  mipmapping, so it looks like a driver difference rather than a bug in the
  Glide layer. Unresolved.
- **Widescreen is unfinished.** Font size, HUD border thickness and the points on
  the map still follow the screen width, the cabin image in the in-car view is
  stretched to it, and the HUD editor still previews a 4:3 screen.

## Building

### Android

Requires Android SDK with **NDK 28.2.13676358**, build-tools 36.0.0, compileSdk
36, and a JDK (Android Studio's bundled JBR works). The app is arm64-v8a only,
minSdk 29, so it installs on Android 10 and later.

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
keys and one variable per pad button from its own screens; the rest matter for a
desktop run.

| Variable | Default | Description |
|---|---|---|
| `NFS_FPS_CAP` | `30` | Frame rate the renderer paces presents to. The game's logic is tuned for 30 Hz. |
| `NFS_GAMMA`, `NFS_BRIGHTNESS`, `NFS_CONTRAST` | `1.0` | Applied to the finished frame in the final blit, so menus and movies are covered as well as a race. |
| `NFS_ORIENTATION` | unset | `auto` allows both landscape directions; anything else pins one. Must be set before `SDL_Init`. |
| `NFS_CAR_DETAIL_FULL` | on | With Car Detail at High, every car keeps its detailed model and the player's texture size out to the draw distance, in every view, and the limit on how many are drawn at once is lifted. `0` restores the original. |
| `NFS_WIDESCREEN` | on | `0` takes the 1280x720 mode back out of the Screen Size list. |
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
