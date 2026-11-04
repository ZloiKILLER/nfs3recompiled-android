#!/bin/sh
# Builds sound_checks and runs it: the natives of src/nfs3hp/port_snd_isnd.cpp
# against eacsnd's generated code, made afresh from nfs3hp/eacsnd.dll by the
# port's own generator (generate.py, which needs capstone; PYTHON=... for one
# that has it) -- once with the default FPU (doubles) and once with
# WITH_PEDANTIC_FPU (80 bits).  CASES=n for more or fewer cases, JOBS=n
# compilers at once.
#
# Everything but port_snd_isnd.cpp is built once into build/<fpu>/base/, which
# mutations.py links each of its mutants against.
set -e
here=$(cd "$(dirname "$0")" && pwd)
root=$(cd "$here/../.." && pwd)
build=${BUILD:-"$here/build"}
mkdir -p "$build"
${PYTHON:-python3} "$here/generate.py" "$build/generated" > "$build/generate.log"
for fpu in double; do
    out="$build/$fpu"
    rm -rf "$out"
    mkdir -p "$out/base"
    flags="-std=c++17 -O1 -fno-strict-aliasing -DWITH_MMX -w"
    sources="$(ls "$build"/generated/eacsnd.*.cpp) $root/src/lib/cpu.cpp $here/sound_checks.cpp"
    includes="-I$build/generated -I$root/sdl3/include -I$root/include -I$root/src/nfs3hp"
    printf '%s\n' $sources | xargs -P "${JOBS:-4}" -I{} \
        sh -c '${CXX:-c++} $1 $2 -c "$3" -o "$4/$(basename "$3" .cpp).o"' _ "$flags" "$includes" {} "$out/base"
    ${CXX:-c++} $flags $includes -c "$root/src/nfs3hp/port_snd_isnd.cpp" -o "$out/port_snd_isnd.o"
    ${CXX:-c++} "$out"/base/*.o "$out/port_snd_isnd.o" -o "$out/sound_checks"
    echo "== $fpu FPU"
    "$out/sound_checks" ${CASES:-20000}
done
