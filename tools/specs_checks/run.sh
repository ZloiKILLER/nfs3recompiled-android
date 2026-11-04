#!/bin/sh
# Builds specs_checks and runs it: sub_4b22d0's decompiled native
# (src/nfs3hp/native_specs_file.cpp) against the generated transcription it
# stands in for, taken out of this tree's src/nfs3hp/disassembly (or $1's)
# without its hook -- the real Watcom sprintf and memset beside them, and the
# heap a bump allocator whose nth allocation can be made to fail, so the ways
# out when an allocation fails run too.
set -e
here=$(cd "$(dirname "$0")" && pwd)
root=$(cd "$here/../.." && pwd)
build=${BUILD:-"$here/build"}
mkdir -p "$build"
disassembly="${1:-$root}/src/nfs3hp/disassembly"
python3 "$here/gen_images.py" "$disassembly" "$build"
python3 "$here/extract.py" "$disassembly" "$build/generated.cpp" \
    sub_4b22d0 sub_4b0790 sub_4b07c0 sub_4b0880 sub_4b08d0 \
    sub_51f140 sub_51f254 sub_5167b3 \
    sub_5169ef sub_516a87 sub_516a9d sub_516ab4 sub_516acc sub_516ae2 \
    sub_516be2 sub_516bf5 sub_516dfa sub_516e13 sub_52448c
${CXX:-c++} -std=c++17 -O2 -fno-strict-aliasing -w \
    -I"$build" -I"$here/stub" -I"$here/../loading_checks/stub" -I"$root/include" \
    -I"$root/src/nfs3hp/disassembly" -I"$root/src/nfs3hp" \
    "$build/generated.cpp" "$root/src/nfs3hp/native_specs_file.cpp" \
    "$root/src/nfs3hp/native_watcom.cpp" "$root/src/nfs3hp/native_watcom_heap.cpp" \
    "$here/specs_checks.cpp" \
    -o "$build/specs_checks"
"$build/specs_checks"
