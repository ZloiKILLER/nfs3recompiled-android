"""Native vertex loops at the top of the game's hottest functions; never patch input EXEs.

A profile of a split-screen race at night (2026-09-25) spends a quarter of the
game thread in sub_41a550, which takes the track's vertices into view space and
onto the screen and shades them (sub_41a3e0, called from it alone), and a tenth
in sub_4bf4c0, which does the same for the vertices of cars and objects.  Each x87
instruction of it is a call into the emulated FPU, rounded to single precision
afterwards.  src/nfs3hp/native_vertices.cpp does the same arithmetic as floats,
in the same order, with the same results; see there for when it steps aside.
The generated function stays whole behind the call: when the native one says
no, the original runs.

With those native, the next profile of the same race (2026-09-25) had the
track's polygon loop (sub_41ae90) and a car's chrome, lighting and polygon
culling (sub_49db70, sub_49da30, sub_49dbf0) at the top; they went native the
same way, and so did the renderer DLL's triangle (voodoo2a's THRASH_drawtri),
which hands each of the cars' triangles to Glide.
"""
from pathlib import Path

HEADER = "// Port (tools/apply_native_vertices.py): defined in native_vertices.cpp.\n"

# (module, function, native stand-in)
SITES = [
    ("nfs3hp", "sub_41a550", "trackVertices"),
    # Object vertices (cars, scenery): a tenth of the game thread.
    ("nfs3hp", "sub_4bf4c0", "objectVertices"),
    # Vectors turned by a matrix, from a dozen places: a twentieth.
    ("nfs3hp", "sub_4e03b0", "rotateVectors"),
    # A car's sphere-mapped chrome, its lighting and its polygons facing the
    # view: together a twelfth once the ones above were native (2026-09-25).
    ("nfs3hp", "sub_49db70", "envMapCoords"),
    ("nfs3hp", "sub_49da30", "shadeNormals"),
    ("nfs3hp", "sub_49dbf0", "cullPolygons"),
    # The track's polygons, culled and handed to the drawing: a thirteenth.
    ("nfs3hp", "sub_41ae90", "trackPolygons"),
    # THRASH_drawtri: a triangle of the game's vertices to Glide.
    ("voodoo2a", "sub_a85770", "thrashDrawTri"),
    # The frame's polygons to THRASH_drawtri, all of them: a thirteenth.
    ("nfs3hp", "sub_434380", "submitPolygons"),
    # The clipper: a triangle off the screen cut to it and drawn as a fan --
    # the hottest function left in a race (2026-10-03), a tenth of the game
    # thread.
    ("nfs3hp", "sub_4c2cd0", "clipPolygon"),
    # Its callers for a triangle and for a quad, which write the depths of the
    # ones not clipped (2026-10-03).
    ("nfs3hp", "sub_4c11b0", "clipTriangle"),
    ("nfs3hp", "sub_4c12e0", "clipQuad"),
    # The other way to the screen (2026-10-03): a triangle whose corners carry
    # z (sub_4c1aa0, a seventieth at night), a car's mesh of quads, and the
    # list of triangles that leads to both kinds.
    ("nfs3hp", "sub_4c1aa0", "clipTriangleByZ"),
    ("nfs3hp", "sub_4c1e20", "quadMesh"),
    ("nfs3hp", "sub_433a50", "triangleList"),
    # The next profile of a race (2026-10-03, day and night): dot products of
    # a vector list (sub_4e0210, a twenty-fifth), a 3x3 matrix product, points
    # turned and moved; the depth buckets gathered into one polygon list; an
    # object's vertices with their nearest depth; a piece of track's polygons
    # as list records.
    ("nfs3hp", "sub_4e0210", "dotProducts"),
    ("nfs3hp", "sub_4e0280", "multiplyMatrices"),
    ("nfs3hp", "sub_4e0430", "transformPoints"),
    ("nfs3hp", "sub_4c79a0", "gatherBuckets"),
    ("nfs3hp", "sub_49d9d0", "objectVerticesNearest"),
    ("nfs3hp", "sub_41b300", "trackRecords"),
    # The track's drawing, whole (2026-10-03): whether a box of four corners
    # is out of view (blocks of track, objects, cars), and a triangle of the
    # projected headlight pass (sub_41a360 with its sub_41a120).
    ("nfs3hp", "sub_41bf30", "boxOutside"),
    ("nfs3hp", "sub_41a360", "projectedTriangle"),
    # A list of quads to the drawing with their state (sub_433bb0), and the
    # frame's polygons into the depth buckets, the transparent ones in order.
    ("nfs3hp", "sub_433bb0", "quadList"),
    ("nfs3hp", "sub_4c7790", "depthBuckets"),
    ("nfs3hp", "sub_4c7610", "sortedBuckets"),
    # Small helpers called from hundreds of places: truncation to an
    # integer, a dot product and a normalisation on the x87 stack, the
    # nearest track block, the transparent buckets gathered, an object's
    # coloured vertices, and Watcom's memset at its three entries.
    ("nfs3hp", "sub_4dfd56", "truncateTop"),
    ("nfs3hp", "sub_4e01f0", "dotPush"),
    ("nfs3hp", "sub_4972f0", "normalise"),
    ("nfs3hp", "sub_41aab0", "groundDistanceOf"),
    ("nfs3hp", "sub_41e0f0", "nearestBlock"),
    ("nfs3hp", "sub_4c7320", "gatherSorted"),
    ("nfs3hp", "sub_41a970", "objectVerticesColoured"),
    ("nfs3hp", "sub_4e0721", "fillWord"),
    ("nfs3hp", "sub_4e0716", "fillByte"),
    ("nfs3hp", "sub_4e070c", "fillZero"),
    # A point onto a frame's ground plane, its height on the x87 stack.
    ("nfs3hp", "sub_4968a0", "groundHeight"),
    # Effects: the particles drawn (sub_4cbc90, the heaviest generated
    # function left at night), the drops on the view, the particles moved.
    ("nfs3hp", "sub_4cbc90", "particles"),
    ("nfs3hp", "sub_4cb750", "drops"),
    ("nfs3hp", "sub_4cb2c0", "moveParticles"),
    # A light's glow on a car (headlights, brake lights, flashers).
    ("nfs3hp", "sub_492370", "lightGlow"),
    # A headlight's beam: a cone of quads.
    ("nfs3hp", "sub_492980", "headlightBeam"),
    # Which model a car is drawn with in a pass; an object's vertices with
    # their colours, and its polygons as records.
    ("nfs3hp", "sub_4bb5d0", "carDetail"),
    ("nfs3hp", "sub_4dbef0", "objectVerticesList"),
    ("nfs3hp", "sub_41b640", "objectRecords"),
    # Small vector helpers from dozens of places: scale, add, subtract,
    # length, unit vector.
    ("nfs3hp", "sub_4e0050", "scaleVectors"),
    ("nfs3hp", "sub_4dffb0", "addVectors"),
    ("nfs3hp", "sub_4e0000", "subtractVectors"),
    ("nfs3hp", "sub_4e06a0", "vectorLengthOf"),
    ("nfs3hp", "sub_4e01c0", "unitVector"),
    # Not a stand-in: how much of Render_GetTm's buffer a view pass used, read
    # as the next one starts it over, for the [VIEW] trace.
    ("nfs3hp", "sub_4bbd70", "arenaReset"),
    # Not a stand-in: an opponent's lane votes as its driver acts on them
    # (NFS_CAR_TRACE), to find why the opponents pile up at Aquatica's tunnel.
    ("nfs3hp", "sub_40de30", "aiVotes"),
    # Not stand-ins: EA's comm library into the log (NFS_NET_TRACE) -- the
    # IPX transport opening, the library's event log, packet_sendpacket and
    # its message printer -- to see where an IPX race stops.
    ("nfs3hp", "sub_4f5c10", "commOpenTrace"),
    ("nfs3hp", "sub_512180", "commEventTrace"),
    ("nfs3hp", "sub_51d810", "commSendTrace"),
    ("nfs3hp", "sub_401010", "commMessageTrace"),
]

# How each module's generated functions open, and where a declaration goes.
OPENING = {
    "nfs3hp": ("void Application::%s(WinApplication* __restrict app, x86::CPU& cpu_)\n"
               "{\n"
               "  x86::Local cpu(cpu_);\n"
               "  NFS2_USE(cpu);\n"
               "  NFS2_USE(app);\n"),
    "voodoo2a": ("void %s(win32::WinApplication* __restrict app, x86::CPU& cpu_)\n"
                 "{\n"
                 "  x86::Local cpu(cpu_);\n"
                 "  NFS2_USE(cpu);\n"
                 "  NFS2_USE(app);\n"),
    "eacsnd": ("void %s(win32::WinApplication* __restrict app, x86::CPU& cpu_)\n"
               "{\n"
               "  x86::Local cpu(cpu_);\n"
               "  NFS2_USE(cpu);\n"
               "  NFS2_USE(app);\n"),
}

GUARD = "  MovieSession movieSession;\n"

CALL = ("    if (%s(app, cpu.sync())) /* port: native (tools/apply_native_vertices.py) */\n"
        "    {\n"
        "        cpu.reload();\n"
        "        return;\n"
        "    }\n")

# A function too small for x86::Local works on the CPU itself.
PLAIN_OPENING = {
    "nfs3hp": ("void Application::%s(WinApplication* app, x86::CPU& cpu)\n"
               "{\n"
               "  NFS2_USE(cpu);\n"
               "  NFS2_USE(app);\n"),
    "voodoo2a": ("void %s(win32::WinApplication* app, x86::CPU& cpu)\n"
                 "{\n"
                 "  NFS2_USE(cpu);\n"
                 "  NFS2_USE(app);\n"),
}

PLAIN_CALL = ("    if (%s(app, cpu)) /* port: native (tools/apply_native_vertices.py) */\n"
              "    {\n"
              "        return;\n"
              "    }\n")

# The natives live in namespace nfs3hp: declared inside it in nfs3hp's files,
# in a namespace block of their own ahead of voodoo2a's and eacsnd's.
NAMESPACE = {"nfs3hp": "namespace nfs3hp\n{\n", "voodoo2a": "namespace voodoo2a\n{\n",
             "eacsnd": "namespace eacsnd\n{\n"}
DECLARATION = {
    "nfs3hp": "bool %s(win32::WinApplication* app, x86::CPU& cpu);\n",
    "voodoo2a": "namespace nfs3hp\n{\nbool %s(win32::WinApplication* app, x86::CPU& cpu);\n}\n\n",
    "eacsnd": "namespace nfs3hp\n{\nbool %s(win32::WinApplication* app, x86::CPU& cpu);\n}\n\n",
}
CALLEE = {"nfs3hp": "%s", "voodoo2a": "nfs3hp::%s", "eacsnd": "nfs3hp::%s"}


def apply(root, sites=SITES, header=HEADER, marker="tools/apply_native_vertices.py"):
    for module, function, native in sites:
        sources = sorted((root / "src/nfs3hp/disassembly").glob(module + ".*.cpp"))
        found = False
        for path in sources:
            with path.open("r", newline="") as source:
                text = source.read()
            eol = "\r\n" if text.count("\r\n") * 2 > text.count("\n") else "\n"

            def local(s):
                return eol.join(s.split("\n"))

            def marked(s):
                return s.replace("tools/apply_native_vertices.py", marker)

            opening = local(OPENING[module] % function)
            call = local(marked(CALL) % (CALLEE[module] % native))
            if opening not in text:
                # sub_495bc0 opens with tools/apply_movie_tap.py's guard, which
                # then holds for the native as well.
                opening = local((OPENING[module] % function).replace("{\n", "{\n" + GUARD, 1))
            if opening not in text:
                opening = local(PLAIN_OPENING[module] % function)
                call = local(marked(PLAIN_CALL) % (CALLEE[module] % native))
                if opening not in text:
                    continue
            found = True
            if opening + call not in text:
                text = text.replace(opening, opening + call, 1)
            declaration = local(header + DECLARATION[module] % native)
            if declaration not in text:
                namespace = local(NAMESPACE[module])
                if text.count(namespace) != 1:
                    raise RuntimeError("%s: namespace opening changed" % path.name)
                if module == "nfs3hp":
                    text = text.replace(namespace, namespace + declaration, 1)
                else:
                    text = text.replace(namespace, declaration + namespace, 1)
            with path.open("w", newline="") as out:
                out.write(text)
            break
        if not found:
            raise RuntimeError("no generated %s with the expected opening" % function)


if __name__ == "__main__":
    apply(Path(__file__).resolve().parents[1])
