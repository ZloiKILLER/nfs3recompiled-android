"""Track textures out of and back into an FSH/QFS shape file.

  unpack <file.qfs|file.fsh> <dir>   every picture to <dir>/<name>.png (RGBA)
                                     plus <dir>/manifest.json
  pack <dir> <out.qfs|out.fsh>       the pictures of <dir> back, as listed
                                     in its manifest, each at the size its
                                     .png has now

The file is "SHPI": its size at +4, its count at +8, its directory id at
+0xc, from +0x10 an entry of 8 bytes a picture (4-byte name, offset).  A
picture's record is 16 bytes -- its type (byte), the offset of its next
block (3 bytes, 0 for none), width, height, centre x/y, position x/y (16 bit
each) -- then its pixels, then any blocks attached to it (a few track
textures and the suns carry a text block 0x6f: "<additive>", the sun's
angles), kept as they are.  The track files carry 0x7e (ARGB 1555) and 0x7d
(ARGB 8888, the bytes B, G, R, A), no mipmaps.

A .qfs is the shape file RefPack-packed; the game unpacks one only where it
looks packed (native_ea_formats.cpp), so pack writes it unpacked.  Every
picture packs as 0x7d: the port takes 8888 for any of them (port_thrash.cpp,
the full colour change).
"""

import json
import struct
import sys
from pathlib import Path

from PIL import Image


def unrefpack(data):
    flags = data[0] << 8 | data[1]
    at = 2
    if flags & 0x8000:
        at += 4
    else:
        at += 3
    if flags & 0x100:
        at += 4 if flags & 0x8000 else 3
    out = bytearray()
    while at < len(data):
        b0 = data[at]
        if b0 < 0x80:
            b1 = data[at + 1]
            at += 2
            literal = b0 & 3
            length, back = ((b0 & 0x1C) >> 2) + 3, ((b0 & 0x60) << 3) + b1 + 1
        elif b0 < 0xC0:
            b1, b2 = data[at + 1], data[at + 2]
            at += 3
            literal = b1 >> 6
            length, back = (b0 & 0x3F) + 4, ((b1 & 0x3F) << 8) + b2 + 1
        elif b0 < 0xE0:
            b1, b2, b3 = data[at + 1], data[at + 2], data[at + 3]
            at += 4
            literal = b0 & 3
            length, back = ((b0 & 0x0C) << 6) + b3 + 5, ((b0 & 0x10) << 12) + (b1 << 8) + b2 + 1
        elif b0 < 0xFC:
            at += 1
            literal = ((b0 & 0x1F) << 2) + 4
            out += data[at:at + literal]
            at += literal
            continue
        else:
            at += 1
            out += data[at:at + (b0 & 3)]
            break
        out += data[at:at + literal]
        at += literal
        for _ in range(length):
            out.append(out[-back])
    return bytes(out)


def read_shapes(path):
    data = Path(path).read_bytes()
    if data[:4] != b"SHPI":
        data = unrefpack(data)
    count, directory = struct.unpack_from("<I4s", data, 8)
    entries = [struct.unpack_from("<4sI", data, 0x10 + i * 8) for i in range(count)]
    shapes = []
    ends = [offset for _, offset in entries[1:]] + [len(data)]
    for (name, offset), end in zip(entries, ends):
        kind = data[offset]
        following = int.from_bytes(data[offset + 1:offset + 4], "little")
        width, height, cx, cy, px, py = struct.unpack_from("<6H", data, offset + 4)
        attached = data[offset + following:end].hex() if following else ""
        pixels = data[offset + 16:]
        if kind == 0x7E:
            words = struct.unpack_from(f"<{width * height}H", pixels)
            rgba = bytearray()
            for w in words:
                r, g, b = (w >> 10) & 31, (w >> 5) & 31, w & 31
                rgba += bytes((r << 3 | r >> 2, g << 3 | g >> 2, b << 3 | b >> 2, 255 if w & 0x8000 else 0))
            image = Image.frombytes("RGBA", (width, height), bytes(rgba))
        elif kind == 0x7D:
            image = Image.frombytes("RGBA", (width, height), pixels[:width * height * 4], "raw", "BGRA")
        else:
            raise ValueError(f"{name!r}: type {kind:#x} is not handled")
        shapes.append({"name": name.decode("latin-1"), "type": kind, "centre": [cx, cy],
                       "position": [px, py], "attached": attached, "image": image})
    return directory.decode("latin-1"), shapes


def unpack(source, folder):
    folder = Path(folder)
    folder.mkdir(parents=True, exist_ok=True)
    directory, shapes = read_shapes(source)
    for shape in shapes:
        shape.pop("image").save(folder / f"{shape['name']}.png")
    (folder / "manifest.json").write_text(json.dumps({"directory": directory, "shapes": shapes}, indent=1))


def pack(folder, target):
    folder = Path(folder)
    manifest = json.loads((folder / "manifest.json").read_text())
    shapes = manifest["shapes"]
    records = []
    for shape in shapes:
        image = Image.open(folder / f"{shape['name']}.png").convert("RGBA")
        cx, cy = shape["centre"]
        px, py = shape["position"]
        pixels = image.tobytes("raw", "BGRA")
        attached = bytes.fromhex(shape["attached"])
        following = 16 + len(pixels) if attached else 0
        records.append(struct.pack("<I6H", 0x7D | following << 8, *image.size, cx, cy, px, py) + pixels + attached)
    offset = 0x10 + 8 * len(shapes)
    directory = bytearray()
    for shape, record in zip(shapes, records):
        directory += struct.pack("<4sI", shape["name"].encode("latin-1"), offset)
        offset += len(record)
    body = bytes(directory) + b"".join(records)
    head = b"SHPI" + struct.pack("<II4s", 0x10 + len(body), len(shapes), manifest["directory"].encode("latin-1"))
    Path(target).write_bytes(head + body)


if __name__ == "__main__":
    if len(sys.argv) != 4 or sys.argv[1] not in ("unpack", "pack"):
        sys.exit(__doc__)
    (unpack if sys.argv[1] == "unpack" else pack)(sys.argv[2], sys.argv[3])
