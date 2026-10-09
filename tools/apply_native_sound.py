"""eacsnd's sound driver native, straight into SDL; never patch input EXEs.

eacsnd.dll is the iSNDdirect* half of EA's SND library: the game mixes, the
DLL feeds what it mixes to DirectSound.  Four of its exports do the work --
caps, start, stop and serve -- and go to C++ (src/nfs3hp/native_sound.cpp),
each entered from the top of its generated function the way
tools/apply_native_vertices.py enters the vertex loops: when the native one
says no (NFS_SND_NATIVE=0), the generated code runs, DirectSound and all.
"""
from pathlib import Path

from tools.apply_native_vertices import apply as apply_sites

HEADER = "// Port (tools/apply_native_sound.py): defined in native_sound.cpp.\n"

# (module, function, native stand-in)
SITES = [
    ("eacsnd", "sub_a32c28", "soundCaps"),     # iSNDdirectcaps
    ("eacsnd", "sub_a3382c", "soundStart"),    # iSNDdirectstart
    ("eacsnd", "sub_a3387c", "soundStop"),     # iSNDdirectstop
    ("eacsnd", "sub_a33b24", "soundServe"),    # iSNDdirectserve
]


def apply(root):
    apply_sites(root, SITES, HEADER, "tools/apply_native_sound.py")


if __name__ == "__main__":
    apply(Path(__file__).resolve().parents[1])
