#!/bin/sh
# Builds loading_checks and runs it: the natives of src/nfs3hp/native_loading.cpp
# against the generated functions they stand in for, taken out of this tree's
# src/nfs3hp/disassembly (or $1's) without their hooks.
set -e
here=$(cd "$(dirname "$0")" && pwd)
root=$(cd "$here/../.." && pwd)
build=${BUILD:-"$here/build"}
mkdir -p "$build"
disassembly="${1:-$root}/src/nfs3hp/disassembly"
python3 "$here/extract.py" "$disassembly" "$build/generated.cpp" \
    sub_5102a4 sub_5102f0 sub_510337 sub_51039e \
    sub_4d18e0 sub_4d1950 sub_4d19b0 sub_4d1a00 sub_4d1a90 sub_4d1ad0 \
    sub_49cfa0 \
    sub_419c20 sub_419bf0 sub_419bc0 sub_4198e0 sub_419ab0 \
    sub_4ea4f0 sub_503281 sub_5033e5 sub_50356d
${CXX:-c++} -std=c++17 -O2 -fno-strict-aliasing -DWITH_MMX -w \
    -I"$here/stub" -I"$root/third_party" -I"$root/include" -I"$root/src/nfs3hp/disassembly" -I"$root/src/nfs3hp" \
    "$build/generated.cpp" "$root/src/nfs3hp/native_loading.cpp" "$here/loading_checks.cpp" \
    -o "$build/loading_checks"
"$build/loading_checks"
