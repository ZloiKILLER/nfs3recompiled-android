#!/usr/bin/env python3
"""The map of the game's functions: each to a subsystem and a status, and the table
progress is counted by (plan.md, step 2 of the main plan).

  python tools/function_map.py [report.md.json ...]

Every function of nfs3.exe, from the generator's own view of it:

  status     port       a typed rewrite, nfs3hp::port (with an adapter to the registers)
             native     C++ on the x86 registers and guest memory (native_*.cpp)
             generated  the generator's code
             dead       a stub that only stops the game, or nothing can reach it
             import     a thunk to a Windows import
  subsystem  a native's by the table it is in; a generated function's by the code
             around it in the executable, the linker having kept a module's functions
             together, then by what it calls and what calls it.
  live       how the live functions log saw it, from the reports of
             tools/profile_report.py given (its .md.json): running, on stack, or unseen.

Writes diagnostics/function-map/map.csv and summary.md, and prints the summary.
"""
import collections
import contextlib
import csv
import io
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))

from disasm import disassembler  # noqa: E402
import disassemble_nfs3hp as game  # noqa: E402

MENU, INPUT, EA, NET, LOADING, SIM, AI, TRACK, RENDER, HUD, SOUND, CRT, LOOP = (
    "меню", "ввод", "библиотеки EA", "сеть", "загрузка", "физика", "ИИ", "трасса", "рендер", "HUD", "звук",
    "рантайм Watcom", "цикл игры")
UNKNOWN = "не определено"

TABLES = {
    "WATCOM_NATIVES": CRT, "FRONTEND_NATIVES": MENU, "FRONTEND_GLUE_NATIVES": MENU, "FRONTEND_DRAW_NATIVES": MENU,
    "MENU_NATIVES": MENU, "PORT_DRAW2D_NATIVES": MENU, "PORT_TEXT_NATIVES": MENU, "PORT_SHAPES_NATIVES": MENU,
    "PORT_FIELDS_NATIVES": MENU, "PORT_OBJ3D_NATIVES": MENU, "PORT_BUTTONS_NATIVES": MENU, "PORT_MENUS_NATIVES": MENU,
    "PORT_LOOP_NATIVES": MENU, "PORT_DIALOGS_NATIVES": MENU,
    "CONTROLS_INPUT_NATIVES": INPUT, "CONTROLS_DEVICES_NATIVES": INPUT, "CONTROLS_RACE_NATIVES": INPUT,
    "CONTROLS_PIPELINE_NATIVES": INPUT, "CONTROLS_QUEUE_NATIVES": INPUT,
    "EA_THREADS_NATIVES": EA, "EA_MEMORY_NATIVES": EA, "EA_FILES_NATIVES": EA, "EA_SUPPORT_NATIVES": EA,
    "EA_FORMATS_NATIVES": EA, "EA_NET_NATIVES": NET,
    "LOAD_TRACK_NATIVES": LOADING, "LOAD_CARDB_NATIVES": LOADING, "LOAD_CARMODELS_NATIVES": LOADING,
    "LOAD_SAVES_NATIVES": LOADING, "RACESTART_NATIVES": LOADING, "RACELOAD_NATIVES": LOADING,
    "TRACKDATA_NATIVES": LOADING,
    "PHYSICS_NATIVES": SIM, "HANDLING_NATIVES": SIM, "DRIVETRAIN_NATIVES": SIM, "CARSTATE_NATIVES": SIM,
    "COLLIDE_NATIVES": SIM, "PORT_CONV_NATIVES": SIM,
    "AIDRIVE_NATIVES": AI, "AI_NATIVES": AI, "PURSUIT_NATIVES": AI, "AIPLAN_NATIVES": AI,
    "TRACK_NATIVES": TRACK, "RENDER_NATIVES": RENDER, "VERTICES_NATIVES": RENDER, "MIX_NATIVES": SOUND,
    "PORT_VIEW_NATIVES": HUD, "PORT_HUD_NATIVES": HUD, "SND_NATIVES": SOUND,
}
# Executable areas no module of the game's own is in.
CRT_FROM = 0x4f7000
NET_NAME = re.compile(r"Tapi|Modem|Serial|Socket|Connection|Packet|Crc16|Network|Protocol|Peer|Ipx|Transmit")
DRAW_NAME = re.compile(r"Glide|Raster|Palette|Display|Rgb|Pixel|Clip|Font|Surface|Graphics")
OUT =ROOT / "diagnostics/function-map"


def dead_static():
    """What nothing can reach, by diagnostics/dead-code."""
    path = ROOT / "diagnostics/dead-code/dead-functions.txt"
    if not path.exists():
        return set()
    return {int(m.group(1), 16) for m in re.finditer(r"^  ([0-9a-f]{6,8}) ", path.read_text(errors="replace"), re.M)}


def native_tables():
    names, subsystem = {}, {}
    for table, area in TABLES.items():
        for address, name in getattr(game, table).items():
            subsystem[address] = area
    for address, name in game.GAME_NATIVES.items():
        names[address] = name
        if address not in subsystem:
            subsystem[address] = LOOP if name.startswith("graphics::") else None
    return names, subsystem


def status_of(entry, subroutine, names, dead):
    name = names.get(entry)
    if name and (name.startswith("dead::") or "::dead_" in name):
        return "dead"
    if name and name.startswith(("port::", "decompiled::")):
        return "port"
    if name:
        return "native"
    if len(subroutine.instructions) <= 2 and subroutine.instructions[0].mnemonic == "jmp" \
            and "ptr" in subroutine.instructions[0].op_str:
        return "import"
    if entry in dead:
        return "dead"
    return "generated"


def spread(subs, known):
    """Subsystems for what the tables do not say: by the neighbours in the executable,
    the linker keeping a module together, and then by the calls."""
    order = sorted(subs)
    position = {e: i for i, e in enumerate(order)}
    callers = collections.defaultdict(set)
    for e, s in subs.items():
        for _, d in s.calls:
            callers[d].add(e)
    # Between two functions of one subsystem, with nothing else between them.
    last = None
    gap = []
    for e in order:
        if known.get(e) is None:
            gap.append(e)
            continue
        if gap and last == known[e]:
            for g in gap:
                known[g] = last
        gap = []
        last = known[e]
    # By calls, the strongest evidence first, until nothing changes.
    changed = True
    while changed:
        changed = False
        for e in order:
            if known.get(e) is not None:
                continue
            votes = collections.Counter()
            for _, d in subs[e].calls:
                if known.get(d):
                    votes[known[d]] += 1
            for c in callers[e]:
                if known.get(c):
                    votes[known[c]] += 2
            i = position[e]
            for n in order[max(0, i - 1):i] + order[i + 1:i + 2]:
                if known.get(n):
                    votes[known[n]] += 1
            if votes:
                top = votes.most_common(2)
                if len(top) == 1 or top[0][1] > top[1][1]:
                    known[e] = top[0][0]
                    changed = True
    # What nothing said anything about, in the libraries' end of the executable.
    for e in order:
        if known.get(e) is None and e >= CRT_FROM:
            known[e] = CRT
    return known


def by_ghidra_name(subs, known, ghidra):
    """The libraries' end of the executable holds more than the runtime: the comm
    library and the display code, whose functions the Ghidra names tell apart."""
    for e in subs:
        name = ghidra.get(e, "")
        if known.get(e) is not None or e < CRT_FROM or not name:
            continue
        if NET_NAME.search(name):
            known[e] = NET
        elif DRAW_NAME.search(name):
            known[e] = RENDER


def live_from(reports, names):
    by_name = {re.sub(r".*::", "", n): a for a, n in names.items()}
    full = {"nfs3hp::" + n: a for a, n in names.items()}
    seen = {}
    for report in reports:
        for f in json.load(open(report, encoding="utf-8")).values():
            name = f["name"].split("(")[0]
            m = re.search(r"sub_([0-9a-f]{6})", name)
            entry = int(m.group(1), 16) if m else full.get(name)
            if entry is None:
                continue
            kind = "running" if f.get("running") else "on stack"
            if seen.get(entry) != "running":
                seen[entry] = kind
    return seen


def corrections(subs, known):
    """diagnostics/function-map/corrections.txt: what people who read the functions found."""
    path = OUT / "corrections.txt"
    if not path.exists():
        return
    for line in path.read_text(encoding="utf-8").splitlines():
        if not line.strip() or line.startswith("#"):
            continue
        where, subsystem = line.split(None, 1)
        first, _, last = where.partition("-")
        first, last = int(first, 16), int(last or first, 16)
        for e in subs:
            if first <= e <= last:
                known[e] = subsystem.strip()


def main():
    with contextlib.redirect_stdout(io.StringIO()):
        image = disassembler.disassemble("nfs3hp", str(ROOT / "nfs3hp/nfs3.exe"), game.DATA_SEGMENTS,
                                         game.KNOWN_SUBROUTINES, game.SPLIT_INSTRUCTIONS, game.THREAD_ROUTINES,
                                         game.THREAD_SEGMENTS, game.MERGE_ROUTINES)
    subs = {s.get_entry_point(): s for s in image.subroutines}
    names, known = native_tables()
    dead = dead_static()
    ghidra = {}
    names_json = ROOT / "diagnostics/nfs3re/names.json"
    if names_json.exists():
        ghidra = {int(a, 16): n for a, n in json.load(open(names_json, encoding="utf-8")).items()}
    known = {e: known.get(e) for e in subs}
    by_ghidra_name(subs, known, ghidra)
    # Before spreading as well, for what was read to spread and the guess not to.
    corrections(subs, known)
    known = spread(subs, known)
    corrections(subs, known)
    live = live_from(sys.argv[1:], names)

    OUT.mkdir(parents=True, exist_ok=True)
    rows = []
    for e in sorted(subs):
        s = subs[e]
        status = status_of(e, s, names, dead)
        rows.append({"address": "%x" % e, "status": status, "subsystem": known.get(e) or UNKNOWN,
                     "live": live.get(e, ""), "x87": sum(i.mnemonic.startswith("f") for i in s.instructions),
                     "instructions": len(s.instructions), "name": names.get(e, ""), "ghidra": ghidra.get(e, "")})
    with open(OUT / "map.csv", "w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(f, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)

    statuses = ("port", "native", "generated", "dead", "import")
    table = collections.defaultdict(collections.Counter)
    for r in rows:
        table[r["subsystem"]][r["status"]] += 1
    lines = ["| подсистема | port | native | generated | мёртвые | импорт | живых | порт среди живых |",
             "|---|---:|---:|---:|---:|---:|---:|---:|"]
    area_order = sorted(table, key=lambda a: -(table[a]["native"] + table[a]["generated"]))
    total = collections.Counter()
    for area in area_order:
        c = table[area]
        total.update(c)
        alive = c["port"] + c["native"] + c["generated"]
        lines.append("| %s | %d | %d | %d | %d | %d | %d | %.0f%% |" % (
            area, c["port"], c["native"], c["generated"], c["dead"], c["import"], alive,
            100 * c["port"] / max(1, alive)))
    alive = total["port"] + total["native"] + total["generated"]
    lines.append("| **всего** | %d | %d | %d | %d | %d | %d | %.0f%% |" % (
        total["port"], total["native"], total["generated"], total["dead"], total["import"], alive,
        100 * total["port"] / max(1, alive)))
    if live:
        lines += ["", "По журналу функций (замеченные в сценарии):", "",
                  "| подсистема | port | native | generated |", "|---|---:|---:|---:|"]
        seen = collections.defaultdict(collections.Counter)
        for r in rows:
            if r["live"]:
                seen[r["subsystem"]][r["status"]] += 1
        for area in sorted(seen, key=lambda a: -sum(seen[a].values())):
            c = seen[area]
            lines.append("| %s | %d | %d | %d |" % (area, c["port"], c["native"], c["generated"]))
    text = "\n".join(lines) + "\n"
    (OUT / "summary.md").write_text(text, encoding="utf-8")
    sys.stdout.reconfigure(encoding="utf-8")
    print(text)


if __name__ == "__main__":
    main()
