# NFS III: Hot Pursuit — Android

An Android port of the static recompilation of **Need for Speed III: Hot Pursuit**.
The original 1998 Win32 x86 executable is translated into portable C++ that runs
on a virtual x86 CPU, with the Win32, DirectDraw, DirectInput, DirectSound and
Glide 2x calls it makes reimplemented on top of SDL3 and OpenGL ES.

Based on [motor-dev/nfs-recompiled](https://github.com/motor-dev/nfs-recompiled),
which does the recompilation itself and targets desktop. This fork adds the
Android target: a launcher, touch controls, gamepad handling, and a number of
fixes in the Glide layer that only showed up once the game ran on a phone.

<img src="screenshots/race-city.jpg" alt="racing through the city" width="49%">
<img src="screenshots/race-boardwalk.jpg" alt="racing on the boardwalk" width="49%">
<img src="screenshots/car-select-front.jpg" alt="car selection" width="49%">
<img src="screenshots/car-select-rear.jpg" alt="car selection, rear view" width="49%">

## What you need to supply

The original executables are in the repository, as in the upstream project, so a
clone builds and runs as-is. What is **not** here is the game content: you need
`FEDATA` and `GAMEDATA` from a retail disc of the 1998 release. The Europe
"Sold Out Software" disc is known to match; a CD image works.

Copy those two folders somewhere and clear the read-only attribute afterwards,
or the game cannot write its settings and saves. That folder is what you import
in the launcher.

Use data from the **same 1998 release**.

`install.win` is the table of data paths the game reads before anything else,
and the installer -- not the disc -- produces it. A working copy is in this
repository at `android/app/src/main/assets/gamefiles/install.win`; the Android
build packages it automatically, and for a desktop run copy it next to the game
data. `tools/make_install_win.py` regenerates it and documents the format, which
was reverse-engineered from the executable.

So a complete desktop game folder is: `fedata/`, `gamedata/`, `install.win`, and
the four executables from `nfs3hp/`.

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

`android/build.bat` does the same on Windows but hardcodes the JDK and SDK
paths of the machine it was written on — edit them or use `gradlew` directly.

The APK lands in `android/app/build/outputs/apk/debug/`. For a release build use
`assembleRelease`; it is unsigned, so align and sign it yourself with
`zipalign` and `apksigner`.

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

With two arguments the second is the CD path, for a setup where the data is
split between an install directory and the disc.

### CMake options

| Option | Default | Description |
|---|---|---|
| `WITH_MMX` | `ON` | MMX bit in the emulated CPUID. Leave it on: with MMX reported absent the game picks a different set of copy/decode routines and the movie streamer breaks. |
| `WITH_PEDANTIC_FPU` | `OFF` | Strict 80-bit x87 emulation through NASM helpers. Linux only. |
| `NFS_TRACE_MSG` | `OFF` | Message-pump and Glide state tracing. Very verbose, desktop diagnostics only. |
| `NFS2_ASSERT_TRAP` | `OFF` | Turn the unsupported-path assert into a debugger break instead of a log line. |
| `NFS_BUILD_FF_TESTS` | `OFF` | Build `force_feedback_checks`, a desktop regression test for the DirectInput force-feedback layer. Runs against an SDL virtual joystick; desktop only. |

## Running on a phone

1. Put the game data folder somewhere on the device, e.g. `/sdcard/nfs3-og`.
2. Launch the app, import that folder, make it the active data set.
3. Play.

The launcher also has a controls hub (Controls → Touch / Gamepad) with a
touch layout editor, the keys the touch controls send, gamepad assignment and
button mapping, and screen settings.

### Gamepads and the game's controls

The game always sees two DirectInput joysticks, `NFS Gamepad 1` and
`NFS Gamepad 2`, each with just two axes: X steers, and Y carries both pedals —
the right trigger pulls it up to accelerate, the left one down to brake.
Gamepad 1 is also where the touch controls steer and accelerate. Every other
button reaches the game as a keyboard key: player 1's keys from the first pad,
player 2's from the second, chosen per button under Controls → Gamepads →
Buttons.

*Gamepad ON* on the same screen writes a matching control set into
the game's `fedata/config/config.dat`, after backing the file up: the same set
for one player and for player 1 in split screen, and player 2 on the second
pad. It writes the description of both devices as well, because the game
regenerates all its bindings whenever the saved device list differs from what
it finds. *Default in game* writes the game's own keyboard defaults
instead.

### Vibration

The game's own DirectInput force-feedback effects are implemented
(`src/lib/winapi/dinput/idirectinputeffect.cpp`) and translated to rumble,
with envelopes and gain. The game plays a constant force for jolts
(collisions, landings, gear changes), which come out as hits that fade; a
square and a sine wave for the road and the engine, which run all race and
come out as light texture; and a centering spring, which pushes against the
player's hand and is not played at all. A gamepad's motor has no gentle
range — even a DualSense's weakest steady rumble is strong — so on a pad the
road and the engine come out as short, faint taps at the rates the game runs
them at (the road at twice its rate), longer where the effect is stronger,
and only jolts rumble steadily. The game drives a single
force-feedback device, the first one it finds, and computes effects only for
the car driven with it, so effects reach Gamepad 1 and the phone; Gamepad 2
never vibrates. The phone plays its own selection, apart from any pad: jolts
as hits, the engine as light ticks that follow the revs above idle, and
cornering, read from how hard the car leans into a turn. The road rumble is
left to a pad, where it does not turn the whole race into one buzz. Phone
vibration is switched on or off under Controls → Touch. Gamepad vibration
is off until switched on under Controls → Gamepads, and its strength is set in
the game's own Force Feedback menu. Nothing vibrates on a button
press; every effect comes from the game.

### Environment variables

Read once at startup, set from `NFS3Activity.onCreate()` on Android:

| Variable | Default | Description |
|---|---|---|
| `NFS_FPS_CAP` | `30` | Frame rate the renderer paces presents to. The game's logic is tuned for 30 Hz. |
| `NFS_CAR_DETAIL_FULL` | on | With Car Detail at High, every car (opponents, traffic, cops and the second player) keeps its detailed model out to the draw distance in every view, split screen and the rear-view mirror included, and the game's budget on how many detailed cars it draws at once is lifted. Every car also loads the player's 256×256 texture, and on devices with 3 GB of RAM or more the texture atlas grows to 4096×4096 to hold them. `0` restores the original. Wheels turning in the second split-screen view and in the mirror do not depend on this setting. |
| `NFS_CAR_DETAIL_TRACE` | unset; `1` in debug APKs | Once a second during a race, log a `[CARDETAIL]` line: the level each car was drawn at, their cost against the game's budget, how full the transform buffer got and how much of the texture atlas is free. |
| `NFS_ORIENTATION` | unset | `auto` allows portrait; anything else pins landscape. Must be set before `SDL_Init`. |
| `NFS_TRACE_API` | unset | Log every intercepted Win32/DirectX call. Pair with `SDL_LOGGING=app=verbose` and filter logcat to `SDL/APP`. |
| `NFS_SCREENSHOT` | unset | Path to write a `.bmp` of the framebuffer to, every `NFS_SCREENSHOT_MS` (default 2000). |
| `NFS_TOUCH_STEER_LEFT` etc. | unset | The keys the touch overlay sends for steering and the pedals, so that endpoint can report them as axes. Set from the touch mapping. |
| `NFS_GAMEPAD1_SOUTH` etc. | built-in defaults | What a button of Gamepad 1 or 2 sends: an SDL key name (`Space`, `Return`, `Up`), `axis:steer_left`, `axis:steer_right`, `axis:accelerate` or `axis:brake`, or empty for nothing. Buttons are `SOUTH`, `EAST`, `WEST`, `NORTH`, `LEFT_SHOULDER`, `RIGHT_SHOULDER`, `LEFT_STICK`, `RIGHT_STICK`, `BACK`, `START` and `DPAD_UP` / `DOWN` / `LEFT` / `RIGHT`. Set from Controls → Gamepad → Buttons. |

## Regenerating the recompilation

The repository ships the generated C++, so this is only needed if you change the
disassembler in `disasm/`:

```bash
pip3 install capstone
python3 disassemble_nfs3hp.py
```

It reads `nfs3hp/nfs3.exe` and the three DLLs and rewrites
`src/nfs3hp/disassembly`.

## Known issues

- **Mosaic artefacts in the headlight-lit area on Mali GPUs.** Blocky patches
  appear where the projected headlight texture falls on the road, only while
  moving. Not reproducible on Adreno with the same build, with or without
  mipmapping, so it looks like a driver difference rather than a bug in the
  Glide layer. Unresolved.

## How it works

1. **Python disassembler** (`disasm/`) — Capstone-based, turns the original
   `.exe` and `.dll` into C++ that reproduces the program as operations on a
   virtual CPU.
2. **Virtual x86 CPU** (`include/cpu.h`, `include/fpu.h`, `include/mmx.h`) —
   registers, flags, a full x87 FPU with optional 80-bit precision, MMX.
3. **Win32 layer** (`src/lib/winapi/`) — native reimplementations of the API
   modules the game uses, including Glide 2x.
4. **SDL3 + OpenGL backend** (`src/lib/sdl-backend/`, `src/lib/gliderenderer.cpp`)
   — windowing, audio, input, files, timers, and the translation of 3Dfx draw
   calls into OpenGL ES.

## Credits

- [motor-dev/nfs-recompiled](https://github.com/motor-dev/nfs-recompiled) — the
  recompilation this is built on.
- [libsdl-org/SDL](https://github.com/libsdl-org/SDL) — SDL3, zlib licence.
- [sse2neon](https://github.com/DLTcollab/sse2neon) — SSE2 intrinsics on ARM,
  MIT licence, vendored as `third_party/sse2neon.h`.

Need for Speed III: Hot Pursuit is the property of Electronic Arts. This
repository contains no game content -- no tracks, cars, audio or video. Those you bring from your own disc.
