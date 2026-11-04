"""tools/game_checks built on Windows: the same sources and
flags as run.sh, compiled by the NDK's clang-cl against Visual Studio Build
Tools' MSVC and the Windows SDK, linked by MSVC's link.exe.  POSIX's mmap,
the one thing of Linux game_checks.cpp uses, comes from shim/sys/mman.h."""
import concurrent.futures, hashlib, os, subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
HERE = Path(__file__).resolve().parent
NDK = Path(os.environ.get("NDK_LLVM", r"D:\Android_SDK\ndk\28.2.13676358\toolchains\llvm\prebuilt\windows-x86_64\bin"))
MSVC = Path(os.environ.get("MSVC_ROOT",
            r"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.44.35207"))
SDK = Path(r"C:\Program Files (x86)\Windows Kits\10")
SDKV = os.environ.get("WINSDK_VERSION", "10.0.26100.0")

# run.sh's natives.
NATIVES = ["native_game", "native_watcom", "native_watcom_heap", "native_mix", "native_frontend_text",
           "native_frontend_screen", "native_controls_input", "native_controls_devices", "native_controls_race",
           "native_controls_pipeline", "native_controls_queue", "native_ea_threads", "native_ea_memory", "native_ea_files", "native_ea_support", "native_load_track", "native_load_cardb", "native_load_carmodels", "native_load_saves", "native_ea_net", "native_ea_formats", "native_frontend_glue", "native_frontend_draw", "port_draw2d", "native_racestart", "native_raceload", "native_trackdata", "native_physics", "native_handling", "native_drivetrain", "native_carstate", "native_track", "native_collide", "native_ai_race", "native_aidrive", "native_pursuit", "native_aiplan", "native_decompiled", "native_graphics", "port_snd_voices", "port_snd_play", "port_snd_driver", "port_timer", "port_sound_effects", "native_snd_speech", "port_sound_race", "native_snd_dead", "port_text", "port_shapes", "port_fields", "port_obj3d", "port_buttons", "port_snd_mixfuncs", "port_snd_fx", "port_snd_mixsetup", "port_snd_mixer", "port_snd_stream", "port_snd_cdstream", "port_snd_path", "port_snd_strm", "port_sound_racebanks", "port_sound_announcer", "port_sound_copradio", "port_sound_speech", "port_sound_scenery", "port_sound_ambient", "port_sound_tick", "port_sound_3d", "port_sound_music", "port_sound_player", "port_sound_banks", "port_sound_racesetup", "port_sound_pursuit", "port_sound_cops", "port_sound_crash", "port_sound_siren", "port_menus", "port_loop", "port_dialogs", "port_view", "port_hud", "port_conv", "port_thrash_records"]

FLAGS = ["/std:c++17", "/O1", "/EHsc", "-fno-strict-aliasing", "/DWITH_MMX", "/w", "/MT", "/bigobj",
         "/D_CRT_SECURE_NO_WARNINGS", "/DNOMINMAX"]
# GAME_CHECKS_DEBUG=1: debug information in the objects and a .pdb by the exe,
# for lldb (the NDK's) to say where a case crashed.
DEBUG = os.environ.get("GAME_CHECKS_DEBUG") == "1"
if DEBUG:
    FLAGS = FLAGS + ["/Z7"]

# The machine's threads, for compiling and for running checks side by side
# (GAME_CHECKS_JOBS to set it).
JOBS = int(os.environ.get("GAME_CHECKS_JOBS", "0")) or os.cpu_count() or 8


def env():
    e = dict(os.environ)
    e["INCLUDE"] = ";".join(str(p) for p in [MSVC / "include", SDK / "Include" / SDKV / "ucrt",
                                              SDK / "Include" / SDKV / "um", SDK / "Include" / SDKV / "shared"])
    e["LIB"] = ";".join(str(p) for p in [MSVC / "lib/x64", SDK / "Lib" / SDKV / "ucrt/x64",
                                          SDK / "Lib" / SDKV / "um/x64"])
    return e


def includes(generated):
    return ["/I" + str(p) for p in [HERE / "shim", generated, ROOT / "tools/game_checks", ROOT / "sdl3/include",
                                    ROOT / "include", ROOT / "src/nfs3hp"]]


def compile_one(source, obj, generated):
    """None, or the compiler's complaint."""
    cmd = [str(NDK / "clang-cl.exe"), "/nologo", "/c", str(source), "/Fo" + str(obj)] + FLAGS + includes(generated)
    r = subprocess.run(cmd, capture_output=True, text=True, env=env())
    return "%s\n%s%s" % (source.name, r.stdout, r.stderr) if r.returncode else None


def newest_header(dirs):
    """The latest change to a header under `dirs` (an "include" directory with
    its subdirectories)."""
    newest = 0.0
    for d in dirs:
        if not d.is_dir():
            continue
        walk = d.rglob("*") if d.name == "include" else d.iterdir()
        for f in walk:
            if f.suffix in (".h", ".hpp", ".inc") and f.is_file():
                newest = max(newest, f.stat().st_mtime)
    return newest


def compile_all(sources, out, generated):
    """Each source to out/<its name>.obj, only where the source, a header or
    the flags changed since; objects of no source removed.  The complaints, if
    any."""
    out.mkdir(parents=True, exist_ok=True)
    flags = hashlib.sha1(" ".join(FLAGS + includes(generated)).encode()).hexdigest()
    stamp = out / "flags.txt"
    if not stamp.exists() or stamp.read_text() != flags:
        for obj in out.glob("*.obj"):
            obj.unlink()
        stamp.write_text(flags)
    wanted = {s.stem + ".obj" for s in sources}
    for obj in out.glob("*.obj"):
        if obj.name not in wanted:
            obj.unlink()
    # The generated code includes only its own header and the port's headers;
    # the rest may include anything on the include path or beside it.
    everything = newest_header([Path(flag[2:]) for flag in includes(generated)]
                               + [ROOT / "tools/game_checks", ROOT / "src/nfs3hp"])
    own = newest_header([generated, ROOT / "include"])

    def stale(source):
        obj = out / (source.stem + ".obj")
        headers = own if source.parent == generated else everything
        return not obj.exists() or obj.stat().st_mtime < max(source.stat().st_mtime, headers)

    todo = [s for s in sources if stale(s)]
    with concurrent.futures.ThreadPoolExecutor(JOBS) as pool:
        errors = [e for e in pool.map(lambda s: compile_one(s, out / (s.stem + ".obj"), generated), todo) if e]
    return errors


def link(objects, exe):
    r = subprocess.run([str(MSVC / "bin/Hostx64/x64/link.exe"), "/nologo", "/OUT:" + str(exe), "/STACK:16777216"] + (["/DEBUG"] if DEBUG else [])
                       + [str(o) for o in objects], capture_output=True, text=True, env=env())
    return None if r.returncode == 0 else (r.stdout + r.stderr)[-3000:]


def base_sources(generated, natives=NATIVES):
    return [generated / "game_functions.cpp", generated / "game_data.cpp", ROOT / "src/lib/cpu.cpp",
            ROOT / "tools/game_checks/game_checks.cpp", ROOT / "tools/game_checks/controls_host.cpp"] + \
           [ROOT / "src/nfs3hp" / (n + ".cpp") for n in natives]
