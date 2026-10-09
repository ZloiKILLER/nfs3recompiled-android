"""The rest of the drawing native, one function at a time; never patch input EXEs.

What tools/apply_native_vertices.py left generated of the drawing goes to C++
(src/nfs3hp/native_render.cpp), each function entered from the top of its
generated one the same way: when the native one says no, the generated code
runs.  Each was compared with the generated code bit for bit in a differential
harness (memory, registers, flags, the FPU, the triangles drawn), and checks
itself against it in the game under NFS_NATIVE_CHECK=1.
"""
from pathlib import Path

from tools.apply_native_vertices import apply as apply_sites

HEADER = "// Port (tools/apply_native_render.py): defined in native_render.cpp.\n"

# (module, function, native stand-in)
SITES = [
    # The clipper of a triangle whose corners carry z (sub_4c1aa0's): cut to
    # the near plane and the screen, drawn as a fan of THRASH_drawtri.
    ("nfs3hp", "sub_4bf790", "clipByZ"),
    # A quad whose corners carry z (sub_433bb0's, under bit 0x10): drawn whole,
    # or as two triangles drawn or clipped by the one above.
    ("nfs3hp", "sub_4c20e0", "quadByZ"),
    # The triangles of a polygon list, with their textures, blending and second
    # pass; those past the near plane to sub_4c3ad0.
    ("nfs3hp", "sub_434120", "triangleRecords"),
    # One of those past the near plane, clipped and drawn as a fan, once or
    # twice with its two textures.
    ("nfs3hp", "sub_4c3ad0", "nearTriangle"),
    # The quads of a polygon list, with their textures and blending, each to
    # the drawing or to the clipping that suits it.
    ("nfs3hp", "sub_4332b0", "quadRecords"),
    # A quad drawn whole or subdivided while it is large on the screen, and the
    # subdivision itself.
    ("nfs3hp", "sub_433060", "quadSubdivided"),
    ("nfs3hp", "sub_432720", "subdivideQuad"),
    # The triangles of a polygon list that carry colours of their own.
    ("nfs3hp", "sub_433e30", "colouredTriangles"),
    # The sky of a view: which of its parts are drawn.
    ("nfs3hp", "sub_47a190", "sky"),
    # The rest of the sky: a view's sky as a whole, the flat sky as one quad,
    # the dome's grid of quads, and the lightning.
    ("nfs3hp", "sub_47a880", "viewSky"),
    ("nfs3hp", "sub_47a1f0", "flatSky"),
    ("nfs3hp", "sub_47b850", "skyDome"),
    ("nfs3hp", "sub_4946e0", "lightning"),
    # Solid lettering (the countdown's number, the race's messages) and the
    # countdown itself.
    ("nfs3hp", "sub_475b50", "solidText"),
    ("nfs3hp", "sub_482620", "countdown"),
    # A car's lights, and a detailed car's.
    ("nfs3hp", "sub_4b9bc0", "carLights"),
    ("nfs3hp", "sub_4bae00", "detailedCarLights"),
    # A medium car's lights, and the colour other cars' lights give a car at
    # night.
    ("nfs3hp", "sub_4bb200", "mediumCarLights"),
    ("nfs3hp", "sub_4b97b0", "nightColour"),
    # A view's pass, and the polygon lists' drawers for the driver.
    ("nfs3hp", "sub_41b9b0", "viewPass"),
    ("nfs3hp", "sub_434870", "polygonDrawers"),
    # Colours under the sun's glare, the detail settings, the sparks and a
    # light's glow.
    ("nfs3hp", "sub_49d800", "glareColours"),
    ("nfs3hp", "sub_472d10", "detailSettings"),
    ("nfs3hp", "sub_4ddb40", "sparks"),
    ("nfs3hp", "sub_491190", "glow"),
    # Vertices under the glare, the 2D buffers cleared, a 2D screen's
    # bouncing pictures, and a headlight's patch on the road.
    ("nfs3hp", "sub_47b7d0", "glareVertices"),
    ("nfs3hp", "sub_4cb670", "clearBuffers"),
    ("nfs3hp", "sub_4a4410", "bouncers"),
    ("nfs3hp", "sub_491bc0", "roadLight"),
    # The race's views and their records, a view's smoke and its spray
    # streaks.
    ("nfs3hp", "sub_41d960", "raceViews"),
    ("nfs3hp", "sub_41df10", "viewRecord"),
    ("nfs3hp", "sub_4de070", "smoke"),
    ("nfs3hp", "sub_4de700", "streaks"),
    # The views' distances for View Distance, the transform buffer, a screen
    # mode and a driver started.
    ("nfs3hp", "sub_41d620", "viewDistances"),
    ("nfs3hp", "sub_4b56e0", "transformBuffer"),
    ("nfs3hp", "sub_4bef50", "screenMode"),
    ("nfs3hp", "sub_4b59c0", "startDriver"),
    # The HUD: an element's frame, its rectangle in pixels, and the two
    # elements laid out inside a frame.
    ("nfs3hp", "sub_47f650", "hudFrame"),
    ("nfs3hp", "sub_480910", "hudRect"),
    ("nfs3hp", "sub_480fc0", "hudPicture"),
    ("nfs3hp", "sub_4812e0", "hudDial"),
    # The rest of the HUD: element 17's panel, the standings' and speeders'
    # tables, a frame's inset, the start lights, the cockpit's needles, bar
    # gauges and overlay, a clipped 3D line, the segment meter, the band behind
    # the HUD in split screen, and the HUD's own sequence.
    ("nfs3hp", "sub_481aa0", "hudPanel"),
    ("nfs3hp", "sub_481c50", "hudTable"),
    ("nfs3hp", "sub_487210", "hudInset"),
    ("nfs3hp", "sub_48c7c0", "startLights"),
    ("nfs3hp", "sub_4880b0", "needles"),
    ("nfs3hp", "sub_488470", "barGauges"),
    ("nfs3hp", "sub_489240", "cockpit"),
    ("nfs3hp", "sub_4c0b20", "line"),
    ("nfs3hp", "sub_48c0e0", "segmentMeter"),
    ("nfs3hp", "sub_4800a0", "hudBand"),
    ("nfs3hp", "sub_482b00", "hud"),
    # Loading: the car's and the cabin's textures, the pictures, the loading
    # screen and the movie player.
    ("nfs3hp", "sub_4b7d30", "carTextures"),
    ("nfs3hp", "sub_494cb0", "loadingScreen"),
    # sub_495bc0 (moviePlayer) is left out: the port plays movies with its own
    # MAD player, decoder and GL output (native_movie.cpp, apply_native_movie.py).
    ("nfs3hp", "sub_4d0a10", "pictureTexture"),
    ("nfs3hp", "sub_4d3f50", "squareTexture"),
    ("nfs3hp", "sub_4d41a0", "paletteTexture"),
    ("nfs3hp", "sub_4d4580", "pieceTexture"),
    ("nfs3hp", "sub_4d4ae0", "cabinTextures"),
]


def apply(root):
    apply_sites(root, SITES, HEADER, "tools/apply_native_render.py")


if __name__ == "__main__":
    apply(Path(__file__).resolve().parents[1])
