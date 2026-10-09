"""The game's sound mixer native; never patch input EXEs.

sub_500864 mixes EA SND's 16 channels into the block eacsnd's serve puts on
SDL's stream: decoding, resampling, volume and pan, all of it.  It goes to C++
(src/nfs3hp/native_mixer.cpp), entered from the top of its generated function
the way tools/apply_native_vertices.py enters the vertex loops: when the native
one says no (NFS_SND_MIX=0), the generated code mixes.
"""
from pathlib import Path

from tools.apply_native_vertices import apply as apply_sites

HEADER = "// Port (tools/apply_native_mixer.py): defined in native_mixer.cpp.\n"

# (module, function, native stand-in)
SITES = [
    ("nfs3hp", "sub_500864", "soundMix"),
]


def apply(root):
    apply_sites(root, SITES, HEADER, "tools/apply_native_mixer.py")


if __name__ == "__main__":
    apply(Path(__file__).resolve().parents[1])
