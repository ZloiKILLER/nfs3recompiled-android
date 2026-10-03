"""voodoo2a's THRASH functions native, one at a time; never patch input EXEs.

THRASH is the game's interface to its renderer DLLs.  Its functions in voodoo2a
-- the Voodoo2 one, the port's -- go to C++ one by one
(src/nfs3hp/native_thrash.cpp), each entered from the top of its generated
function the way tools/apply_native_vertices.py enters the vertex loops: when
the native one says no, the generated code runs.  voodoo2a's own state stays in
guest memory, the one copy both read, until every function is native.

First, the drawing beside THRASH_drawtri (native already): quads, meshes, fans
and lines, all of it the same conversion of the game's vertices to Glide's.
Then THRASH_setstate, the renderer's state, the textures, the frame and the setting up.
"""
from pathlib import Path

from tools.apply_native_vertices import apply as apply_sites

HEADER = "// Port (tools/apply_native_thrash.py): defined in native_thrash.cpp.\n"

# (module, function, native stand-in)
SITES = [
    ("voodoo2a", "sub_a85300", "thrashDrawQuad"),
    ("voodoo2a", "sub_a85520", "thrashDrawQuadMesh"),
    ("voodoo2a", "sub_a85900", "thrashDrawTriMesh"),
    ("voodoo2a", "sub_a86000", "thrashDrawTriFan"),
    ("voodoo2a", "sub_a86240", "thrashDrawLine"),
    ("voodoo2a", "sub_a86350", "thrashDrawLineMesh"),
    # THRASH_setstate: the renderer's state, a piece at a time, 124 call sites.
    ("voodoo2a", "sub_a84990", "thrashSetState"),
    # The textures: a record allocated, its texels downloaded, all of them
    # freed, one selected.
    ("voodoo2a", "sub_a844b0", "thrashTextureAllocate"),
    ("voodoo2a", "sub_a84630", "thrashTextureUpdate"),
    ("voodoo2a", "sub_a84690", "thrashTextureReset"),
    ("voodoo2a", "sub_a846d0", "thrashSetTexture"),
    # The frame: the buffer drawn into, cleared, swapped, locked for the game's
    # own pixels; waiting on the card, the clip rectangle.
    ("voodoo2a", "sub_a84780", "thrashWindow"),
    ("voodoo2a", "sub_a847b0", "thrashClearWindow"),
    ("voodoo2a", "sub_a847d0", "thrashFlushWindow"),
    ("voodoo2a", "sub_a847e0", "thrashPageFlip"),
    ("voodoo2a", "sub_a84810", "thrashIdle"),
    ("voodoo2a", "sub_a84830", "thrashSync"),
    ("voodoo2a", "sub_a848b0", "thrashClip"),
    ("voodoo2a", "sub_a850f0", "thrashLockWindow"),
    ("voodoo2a", "sub_a851d0", "thrashUnlockWindow"),
    ("voodoo2a", "sub_a85220", "thrashReadRect"),
    # Setting up: the driver described, the card found, the window opened in a
    # mode (on the window's thread when the game asks), everything let go.
    ("voodoo2a", "sub_a83c60", "thrashAbout"),
    ("voodoo2a", "sub_a83f50", "thrashInit"),
    ("voodoo2a", "sub_a83f10", "thrashSelectDisplay"),
    ("voodoo2a", "sub_a843d0", "thrashSetVideoMode"),
    ("voodoo2a", "sub_a84180", "thrashOpenMode"),
    ("voodoo2a", "sub_a84360", "thrashWindowThread"),
    ("voodoo2a", "sub_a84100", "thrashRestore"),
]


def apply(root):
    apply_sites(root, SITES, HEADER, "tools/apply_native_thrash.py")


if __name__ == "__main__":
    apply(Path(__file__).resolve().parents[1])
