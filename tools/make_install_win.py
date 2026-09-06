#!/usr/bin/env python3
r"""Generate the install.win file that NFS III: Hot Pursuit requires at startup.

Why this exists
---------------
The original 1998 nfs3.exe -- the binary this project statically recompiles --
reads `install.win` from the game directory before doing anything else.  Without
it the game shows "Need For Speed 3 files are corrupted; please re-install." and
exits.  The file is normally written by the original CD installer, so it is
missing from repacks built for the "NFS3 Modern Patch" (v1.6.1), which dropped
the requirement and moved the settings into nfs3.ini.

Format (reverse-engineered from nfs3.exe, reader at VA 0x4a6e10)
----------------------------------------------------------------
    fopen("install.win", "rt")
    read up to 40 lines with fgets(buf, 0x7f, f)
    strip the first '\n' and the first '\r'
    SKIP EMPTY LINES  -- an empty line does not consume a slot, it shifts
                         everything after it, so every slot must be non-empty
    store each line into a 40 x 128 byte table at VA 0x7a2634

Each slot is then used as a printf path prefix, e.g. slot 3 in
"%sTRK%03x\\3Tr%02x.hrz".  Slot 0 is the language name rather than a path.

Slots confirmed by running the game and watching every file open:

      0  language name                      "english"
      2  writable root (%stemp.mus)
      3  gamedata\tracks\                   %sTRK%03x\3Tr%02x.hrz, sky.fsh
      5  gamedata\carmodel\                 %sknoc\car.viv, traffic\pursuit\
      6  gamedata\render\pc\                %scop0, %scone
      7  gamedata\dashhud\                  %shud98.fsh, %sCP16BI.ffn
      8  gamedata\audio\pc\                 %sshow6, %sshow%d
      9  gamedata\audio\sfx\                %str3d%02x, %sIPspch%s.viv
     10  gamedata\audio\speech\<language>\  cnteng.bnk, lapeng.bnk
     15  fedata\art\                        %scompare.fsh, joys.qfs, cred.qfs
     16  fedata\text\                       %ssci20.ffn, %sarial10b.ffn
     17  fedata\text\                       text.<lang>
     18  fedata\save\                       %s%s.trn
     19  fedata\stats\                      %srecords.dat
     20  fedata\config\                     %sconfig.dat
     21  fedata\stats\carspecs\             player1.spc, player2.spc
     22  fedata\art\slides\                 15_00.qfs, t0_00.qfs
     23  fedata\art\track\                  %strkgo.viv
     24  fedata\art\showcase\               shcout.qfs, h<car>.qfs
     25  fedata\movies\                     titleav.mad, demoav%d.mad

Slot 24 was found by the same marker-driven method, but on a later run: it went
unused by title video/main menu/a single race, and only showed up as a tight,
uncapped open-retry loop -- 10000+ failed opens/sec, 100% CPU, looked like a
frozen screen -- once the Options graphics screen was actually reached.

Slots 1, 4, 11-14, 26-39 are still never touched by any of the above.  They are
filled with self-identifying markers so that any future use shows up in the log
as "unable to open .../SLOTnn/<file>" instead of silently resolving to the wrong
directory.  Pass --no-markers to fill them with ".\\" instead.

Also required next to install.win: nfs3.exe itself (the game stats it during the
integrity check) plus eacsnd.dll, voodoo2a.dll and softtria.dll.
"""

import argparse
import os

BS = chr(92)

SLOTS = {
    0:  "{lang}",
    2:  "." + BS,
    3:  BS.join(["gamedata", "tracks", ""]),
    5:  BS.join(["gamedata", "carmodel", ""]),
    6:  BS.join(["gamedata", "render", "pc", ""]),
    7:  BS.join(["gamedata", "dashhud", ""]),
    8:  BS.join(["gamedata", "audio", "pc", ""]),
    9:  BS.join(["gamedata", "audio", "sfx", ""]),
    10: BS.join(["gamedata", "audio", "speech", "{lang}", ""]),
    15: BS.join(["fedata", "art", ""]),
    16: BS.join(["fedata", "text", ""]),
    17: BS.join(["fedata", "text", ""]),
    18: BS.join(["fedata", "save", ""]),
    19: BS.join(["fedata", "stats", ""]),
    20: BS.join(["fedata", "config", ""]),
    21: BS.join(["fedata", "stats", "carspecs", ""]),
    22: BS.join(["fedata", "art", "slides", ""]),
    23: BS.join(["fedata", "art", "track", ""]),
    24: BS.join(["fedata", "art", "showcase", ""]),
    25: BS.join(["fedata", "movies", ""]),
}

SLOT_COUNT = 40


def build(language="english", markers=True):
    lines = []
    for i in range(SLOT_COUNT):
        if i in SLOTS:
            lines.append(SLOTS[i].format(lang=language))
        else:
            # never empty: an empty line would shift every following slot
            lines.append("SLOT%02d" % i + BS if markers else "." + BS)
    return lines


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("game_dir", help="directory holding fedata/ and gamedata/")
    ap.add_argument("--language", default="english")
    ap.add_argument("--no-markers", action="store_true",
                    help="fill unknown slots with '.\\' instead of SLOTnn markers")
    args = ap.parse_args()

    lines = build(args.language, markers=not args.no_markers)
    path = os.path.join(args.game_dir, "install.win")
    # the game opens it in text mode; CRLF matches what the original installer wrote
    with open(path, "w", newline="\r\n") as f:
        f.write("\n".join(lines) + "\n")
    print("wrote %s (%d slots)" % (path, len(lines)))


if __name__ == "__main__":
    main()
