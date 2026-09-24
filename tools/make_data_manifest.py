"""Write the list of game files an import must bring, for the launcher to check.

    python tools/make_data_manifest.py [REFERENCE_DATA_FOLDER]

The reference is a complete copy of the retail disc's FEDATA and GAMEDATA -- by
default APK/gamedata-original-cd, the Europe "Sold Out Software" disc.  The list
goes to android/app/src/main/assets/game-data-manifest.txt, one path per line,
lower case, as DataImporter.missingFiles compares them.

Left out, so that another edition of the game is not taken for an incomplete
copy of this one: whatever depends on the language (the text files, the speech
banks), and what belongs to the player rather than the disc (saves, records,
settings -- config.dat is not on the disc at all: the game makes it the first
time it runs).
"""
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "android/app/src/main/assets/game-data-manifest.txt"

EXCLUDED = [
    re.compile(r"^fedata/text/text"),        # TEXT.ENG, TEXT.GER ... and TEXT0ENG.TXT
    re.compile(r"^gamedata/audio/speech/"),  # one folder of speech per language
    re.compile(r"^fedata/(save|stats|config)/"),
]


def main():
    reference = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / "APK/gamedata-original-cd"
    paths = []
    for top in ("fedata", "gamedata"):
        folder = next((p for p in reference.iterdir() if p.name.lower() == top), None)
        if folder is None:
            raise SystemExit("%s: no %s folder" % (reference, top))
        for file in folder.rglob("*"):
            if file.is_file():
                path = top + "/" + file.relative_to(folder).as_posix().lower()
                if not any(rule.search(path) for rule in EXCLUDED):
                    paths.append(path)
    paths.sort()
    header = ("# Game files an import must bring (tools/make_data_manifest.py), from\n"
              "# the retail disc's FEDATA and GAMEDATA.  Lower case, one per line.\n")
    OUTPUT.write_text(header + "\n".join(paths) + "\n", encoding="utf-8", newline="\n")
    print("%d files -> %s" % (len(paths), OUTPUT))


if __name__ == "__main__":
    main()
