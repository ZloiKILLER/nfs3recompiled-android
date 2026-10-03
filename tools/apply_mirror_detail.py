"""The rear-view mirror drawn as the main view is; never patch input EXEs.

A single-screen race has two views (sub_41d960): the main one from the first of
sub_41df10's templates and the mirror from its fourth, which culls polygons
beyond 150, ends the world at 100, keeps the track's detailed model to 40 and
its middle one to 80, and walks six blocks of track against the main view's
forty-two.  The mirror showed little behind the car, and the holes of the
track's low model.  sub_41d620 works out the main view's distances from View
Distance; just before it turns every view's distances into floats (0x41d7e5),
nfs3hp::mirrorFollowsMain gives the mirror the main view's set.

Cars: sub_4bb5d0 scales the level-of-detail switch distances and the cars' cull
distance by 0.6 ([0x540138]) in the mirror's pass; nfs3hp::mirrorCarScale makes
it 1.

The pass (sub_4bc470 sets [0x7a3d14] = 1 for the mirror) leaves things out on
its own as well; at each place it asks, nfs3hp::mirrorAsMain answers as the main
view would:
- sub_41aa60 picks the track's model by distance (2 near, 0 far) but gave the
  mirror 0, the coarsest, whatever the distance, and sub_41cdc0 forced 0 again
  -- the holes in the mirror's scenery;
- sub_47a880 drew it a flat sky (sub_47a1f0) rather than the dome, and
  sub_47a190 no clouds or lightning (sub_47b850, sub_4946e0);
- sub_4b9bc0 left out a car's lights, brake lights and flashers, and it,
  sub_4bae00 and sub_4bb200 their light at night (sub_4b97b0);
- sub_41c310 and sub_41ae90 the headlights' light on the track's objects and
  on the road (the latter in trackPolygonsNative as well).
Left as they are: the mirror does not draw its own car under an in-car camera
(sub_4bb5d0), clear the frame to the fog colour (THRASH_clearwindow) or the HUD.

The Modern Patch has the same as its "Rear View Mirror" High setting.  No menu
item: NFS_MIRROR_FULL=0 (the launch's mirror_full false) restores the
original mirror.
"""
from pathlib import Path

from tools.apply_track_detail import apply as apply_sites

HEADER = "// Port (tools/apply_mirror_detail.py): defined in nfs3hp_main.cpp.\n"

SITES = [
    ("nfs3hp.4.cpp", "0041d7e5  31c9",
     "    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));",
     "    mirrorFollowsMain(app); /* port: the mirror at the main view's distances */\n"
     "    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));",
     "void mirrorFollowsMain(win32::WinApplication* app);\n"),
    ("nfs3hp.26.cpp", "004bb6c8  dd0538015400",
     "    x86::Float fpu9 = x86::Float(app->getMemory<double>(x86::reg32(5505336) /* 0x540138 */));",
     "    x86::Float fpu9 = x86::Float(mirrorCarScale(app->getMemory<double>(x86::reg32(5505336) /* 0x540138 */)));"
     " /* port: cars in the mirror as in the main view */",
     "double mirrorCarScale(double scale);\n"),
    # Each place a view pass asks whether it is the mirror's ([view] == 1, the
    # pass sub_4bc470 sets up) to leave something out: asked through
    # nfs3hp::mirrorAsMain, which answers as the main view would.
    ("nfs3hp.4.cpp", "0041aa64  833801",
     "        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax);",
     "        x86::reg32 tmp1 = mirrorAsMain(app->getMemory<x86::reg32>(cpu.eax)); /* port: the track's model by distance in the mirror too, not always the coarsest */",
     "x86::reg32 mirrorAsMain(x86::reg32 kind);\n"),
    ("nfs3hp.4.cpp", "0041cea2  833e01",
     "        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esi);",
     "        x86::reg32 tmp1 = mirrorAsMain(app->getMemory<x86::reg32>(cpu.esi)); /* port: nor forced to the coarsest in the mirror */",
     "x86::reg32 mirrorAsMain(x86::reg32 kind);\n"),
    ("nfs3hp.19.cpp", "0047a8e2  833e01",
     "        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esi);",
     "        x86::reg32 tmp1 = mirrorAsMain(app->getMemory<x86::reg32>(cpu.esi)); /* port: the sky's dome rather than a flat sky in the mirror too */",
     "x86::reg32 mirrorAsMain(x86::reg32 kind);\n"),
    ("nfs3hp.19.cpp", "0047a1ad  833a01",
     "        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.edx);",
     "        x86::reg32 tmp1 = mirrorAsMain(app->getMemory<x86::reg32>(cpu.edx)); /* port: clouds and lightning in the mirror too */",
     "x86::reg32 mirrorAsMain(x86::reg32 kind);\n"),
    ("nfs3hp.26.cpp", "004b9bf0  833801",
     "        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax);",
     "        x86::reg32 tmp1 = mirrorAsMain(app->getMemory<x86::reg32>(cpu.eax)); /* port: a car's lights, brake lights and flashers in the mirror too */",
     "x86::reg32 mirrorAsMain(x86::reg32 kind);\n"),
    ("nfs3hp.26.cpp", "004b9c4f  833801",
     "        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax);",
     "        x86::reg32 tmp1 = mirrorAsMain(app->getMemory<x86::reg32>(cpu.eax)); /* port: a car's lights at night in the mirror too */",
     "x86::reg32 mirrorAsMain(x86::reg32 kind);\n"),
    ("nfs3hp.26.cpp", "004bae51  833801",
     "        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax);",
     "        x86::reg32 tmp1 = mirrorAsMain(app->getMemory<x86::reg32>(cpu.eax)); /* port: a detailed car's lights at night in the mirror too */",
     "x86::reg32 mirrorAsMain(x86::reg32 kind);\n"),
    ("nfs3hp.26.cpp", "004bb259  833801",
     "        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax);",
     "        x86::reg32 tmp1 = mirrorAsMain(app->getMemory<x86::reg32>(cpu.eax)); /* port: a medium car's lights at night in the mirror too */",
     "x86::reg32 mirrorAsMain(x86::reg32 kind);\n"),
    ("nfs3hp.4.cpp", "0041c34a  833801",
     "        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax);",
     "        x86::reg32 tmp1 = mirrorAsMain(app->getMemory<x86::reg32>(cpu.eax)); /* port: headlight light on the track's objects in the mirror too */",
     "x86::reg32 mirrorAsMain(x86::reg32 kind);\n"),
    ("nfs3hp.4.cpp", "0041b08e  833801",
     "        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax);",
     "        x86::reg32 tmp1 = mirrorAsMain(app->getMemory<x86::reg32>(cpu.eax)); /* port: headlight light on the road in the mirror too */",
     "x86::reg32 mirrorAsMain(x86::reg32 kind);\n"),
]


def apply(root):
    apply_sites(root, SITES, HEADER)


if __name__ == "__main__":
    apply(Path(__file__).resolve().parents[1])
