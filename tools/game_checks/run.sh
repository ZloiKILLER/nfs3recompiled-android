#!/bin/sh
# Builds game_checks and runs it: the natives of src/nfs3hp/native_game.cpp,
# native_watcom.cpp, native_watcom_heap.cpp, native_mix.cpp,
# native_frontend_text.cpp, native_frontend_screen.cpp, port_text.cpp and the controls'
# native_controls_*.cpp against the
# game's code for them, made afresh from nfs3hp/nfs3.exe by the port's own
# generator (generate.py, which needs capstone; PYTHON=... for one that has
# it, and reads the whole game, a minute or so) -- once with the default FPU
# (doubles).  CASES=n for more or
# fewer cases a function, JOBS=n compilers at once.  What the natives ask of
# DirectInput controls_host.cpp answers as the game's code asked it, through
# the objects' tables.
#
# Everything but the natives is built once into build/<fpu>/base/, and the
# natives beside it, which mutations.py links its mutants with.
set -e
here=$(cd "$(dirname "$0")" && pwd)
root=$(cd "$here/../.." && pwd)
build=${BUILD:-"$here/build"}
mkdir -p "$build"
${PYTHON:-python3} "$here/generate.py" "$build/generated" > "$build/generate.log" 2>&1
for fpu in double; do
    out="$build/$fpu"
    mkdir -p "$out/base"
    find "$out" -name '*.o' -delete
    flags="-std=c++17 -O1 -fno-strict-aliasing -DWITH_MMX -w"
    sources="$build/generated/game_functions.cpp $build/generated/game_data.cpp $root/src/lib/cpu.cpp $here/game_checks.cpp $here/controls_host.cpp"
    includes="-I$build/generated -I$root/sdl3/include -I$root/include -I$root/src/nfs3hp"
    printf '%s\n' $sources | xargs -P "${JOBS:-4}" -I{} \
        sh -c '${CXX:-c++} $1 $2 -c "$3" -o "$4/$(basename "$3" .cpp).o"' _ "$flags" "$includes" {} "$out/base"
    for native in native_game native_watcom native_watcom_heap native_mix native_frontend_text \
                  native_frontend_screen native_controls_input native_controls_devices native_controls_race \
                  native_controls_pipeline native_controls_queue native_ea_threads native_ea_memory native_ea_files native_ea_support native_load_track native_load_cardb native_load_carmodels native_load_saves native_ea_net native_ea_formats native_frontend_glue native_frontend_draw port_draw2d native_racestart native_raceload native_trackdata native_physics native_handling native_drivetrain native_carstate native_track native_collide native_ai_race native_aidrive native_pursuit native_aiplan native_decompiled native_graphics port_snd_voices port_snd_play port_snd_driver port_timer port_sound_effects native_snd_speech port_sound_race native_snd_dead port_text port_shapes port_fields port_obj3d port_buttons port_snd_mixfuncs port_snd_fx port_snd_mixsetup port_snd_mixer port_snd_stream port_snd_cdstream port_snd_path port_snd_strm port_sound_racebanks port_sound_announcer port_sound_copradio port_sound_speech port_sound_scenery port_sound_ambient port_sound_tick port_sound_3d port_sound_music port_sound_player port_sound_banks port_sound_racesetup port_sound_pursuit port_sound_cops port_sound_crash port_sound_siren port_menus port_loop port_dialogs port_view port_hud port_conv port_thrash_records; do
        ${CXX:-c++} $flags $includes -c "$root/src/nfs3hp/$native.cpp" -o "$out/$native.o"
    done
    ${CXX:-c++} "$out"/base/*.o "$out/native_game.o" "$out/native_watcom.o" "$out/native_watcom_heap.o" \
        "$out/native_mix.o" "$out/native_frontend_text.o" "$out/native_frontend_screen.o" \
        "$out/native_controls_input.o" "$out/native_controls_devices.o" "$out/native_controls_race.o" \
        "$out/native_controls_pipeline.o" "$out/native_controls_queue.o" "$out/native_ea_threads.o" "$out/native_ea_memory.o" "$out/native_ea_files.o" "$out/native_ea_support.o" "$out/native_load_track.o" "$out/native_load_cardb.o" "$out/native_load_carmodels.o" "$out/native_load_saves.o" "$out/native_ea_net.o" "$out/native_ea_formats.o" "$out/native_frontend_glue.o" "$out/native_frontend_draw.o" "$out/port_draw2d.o" "$out/native_racestart.o" "$out/native_raceload.o" "$out/native_trackdata.o" "$out/native_physics.o" "$out/native_handling.o" "$out/native_drivetrain.o" "$out/native_carstate.o" "$out/native_track.o" "$out/native_collide.o" "$out/native_ai_race.o" "$out/native_aidrive.o" "$out/native_pursuit.o" "$out/native_aiplan.o" "$out/native_decompiled.o" \
        "$out/native_graphics.o" "$out/port_snd_voices.o" "$out/port_snd_play.o" "$out/port_snd_driver.o" "$out/port_timer.o" "$out/port_sound_effects.o" "$out/native_snd_speech.o" "$out/port_sound_race.o" "$out/native_snd_dead.o" "$out/port_text.o" "$out/port_shapes.o" "$out/port_fields.o" "$out/port_obj3d.o" "$out/port_buttons.o" "$out/port_snd_mixfuncs.o" "$out/port_snd_fx.o" "$out/port_snd_mixsetup.o" "$out/port_snd_mixer.o" "$out/port_snd_stream.o" "$out/port_snd_cdstream.o" "$out/port_snd_path.o" "$out/port_snd_strm.o" "$out/port_sound_racebanks.o" "$out/port_sound_announcer.o" "$out/port_sound_copradio.o" "$out/port_sound_speech.o" "$out/port_sound_scenery.o" "$out/port_sound_ambient.o" "$out/port_sound_tick.o" "$out/port_sound_3d.o" "$out/port_sound_music.o" "$out/port_sound_player.o" "$out/port_sound_banks.o" "$out/port_sound_racesetup.o" "$out/port_sound_pursuit.o" "$out/port_sound_cops.o" "$out/port_sound_crash.o" "$out/port_sound_siren.o" "$out/port_menus.o" "$out/port_loop.o" "$out/port_dialogs.o" "$out/port_view.o" "$out/port_hud.o" "$out/port_conv.o" "$out/port_thrash_records.o" \
        -o "$out/game_checks"
    echo "== $fpu FPU"
    "$out/game_checks" ${CASES:-20000}
done
