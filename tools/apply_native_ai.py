"""The opponents' driving, decompiled, at the top of its generated functions; never patch input EXEs.

src/nfs3hp/native_ai.h has sub_4064f0 (the pull of an opponent's engine at its
speed) and sub_4070c0 (how far the way it wants to go is from the way it
points) as C++ that comes out as the generated code to the bit, checked by
tools/native_ai_checks.py.  They are hooked as the vertex loops are
(tools/apply_native_vertices.py): a call at the top of the generated function,
which stays whole behind it and runs when NFS_NATIVE_AI=0.
"""
import sys
from pathlib import Path

HEADER = "// Port (tools/apply_native_ai.py): defined in native_ai.cpp.\n"

SITES = [
    ("nfs3hp", "sub_4064f0", "aiPull"),
    ("nfs3hp", "sub_4070c0", "aiHeading"),
]


def apply(root):
    sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
    from tools.apply_native_vertices import apply as apply_natives
    apply_natives(root, sites=SITES, header=HEADER, marker="tools/apply_native_ai.py")


if __name__ == "__main__":
    apply(Path(__file__).resolve().parents[1])
