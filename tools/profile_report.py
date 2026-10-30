#!/usr/bin/env python3
"""Reads functions.bin, written by debug mode's "Live functions log"
(src/nfs3hp/func_log.cpp), and says which functions of the game ran: generated
code, natives (native_*.cpp) and the typed port (port_*.cpp), in all and by
stretch of the session.

  adb pull /sdcard/Android/data/dev.nfs3hp.port/files/functions.bin
  python tools/profile_report.py functions.bin [libmain.so] [report.md]

The log is a sample of what the game was doing (every 4 ms of CPU time, with
the whole stack), so a function that ran only a few times may be missing, and
a missing one is not proof it never runs; the counts are samples, not calls.
"Running" is a function the time was spent in, "on stack" any function that
had a call in progress, so the callers of the busy ones show up as well.

The library is the unstripped one the APK was built from (without it the
newest android/app/build/intermediates/cxx/*/*/obj/arm64-v8a/libmain.so); the
build id in the file is checked against it.
"""
import bisect
import collections
import glob
import json
import os
import re
import struct
import subprocess
import sys

NDK = os.environ.get("NFS_NDK_BIN",
                     r"D:\Android_SDK\ndk\28.2.13676358\toolchains\llvm\prebuilt\windows-x86_64\bin")
EXE = ".exe" if os.name == "nt" else ""
NM, SYMBOLIZER, CXXFILT, READELF = (os.path.join(NDK, t + EXE) for t in
                                    ("llvm-nm", "llvm-symbolizer", "llvm-cxxfilt", "llvm-readelf"))
SELF, KNOWN, REL = 0x80000000, 0x40000000, 0x0FFFFFFF
MARK, WORD, WORD_COUNT, OUTSIDE_OLD, OUTSIDE = 0xFFFFFFFF, 0xFFFFFFFD, 0xFFFFFFFC, 0xFFFFFFFE, 0xFFFFFFE0
PRECISION = {0: "24 бита", 1: "?", 2: "53 бита", 3: "64 бита", None: "без x87"}
WINDOW_MS = 20000


def read_log(path):
    """Snapshots of (ms, {key: samples}, {control word: times set})."""
    data = open(path, "rb").read()
    assert data[:4] in (b"NFF1", b"NFF2"), "not a functions.bin"
    n = data[12]
    build_id = data[13:13 + n].hex()
    snaps, cur, words, word = [], None, None, None
    body = data[13 + n:]
    for key, val in struct.iter_unpack("<II", body[:len(body) // 8 * 8]):
        if key == MARK:
            cur, words = {}, {}
            snaps.append((val, cur, words))
        elif cur is None:
            continue
        elif key == WORD:
            word = val
        elif key == WORD_COUNT:
            words[word] = words.get(word, 0) + val
        else:
            if key == OUTSIDE_OLD:
                key = OUTSIDE
            cur[key] = cur.get(key, 0) + val
    return build_id, snaps


def precision_of(key):
    """The precision control a running sample's key carries, None if its thread set no word."""
    if key >= OUTSIDE:
        return (key & 3) if key & 4 else None
    return (key >> 28) & 3 if key & KNOWN else None


def library_build_id(so):
    out = subprocess.run([READELF, "-n", so], capture_output=True, text=True).stdout
    m = re.search(r"Build ID:\s*([0-9a-f]+)", out)
    return m.group(1) if m else ""


def load_functions(so):
    """Sorted starts, and start -> (size, demangled name)."""
    out = subprocess.run([NM, "-S", "--defined-only", so], capture_output=True, text=True).stdout
    syms = {}
    for line in out.splitlines():
        p = line.split()
        if len(p) == 4 and p[2] in "tTwW" and int(p[1], 16):
            syms[int(p[0], 16)] = (int(p[1], 16), p[3])
    starts = sorted(syms)
    dem = subprocess.run([CXXFILT], input="\n".join(syms[a][1] for a in starts),
                         capture_output=True, text=True).stdout.splitlines()
    return starts, {a: (syms[a][0], d) for a, d in zip(starts, dem)}


def source_files(so, addrs):
    out = subprocess.run([SYMBOLIZER, "--obj=" + so, "--functions=none", "--no-inlines", "--basenames"],
                         input="\n".join("0x%x" % a for a in addrs), capture_output=True, text=True).stdout
    return {a: b.split(":")[0].strip() for a, b in zip(addrs, out.strip().split("\n\n"))}


def classify(name, src):
    if re.fullmatch(r"nfs3hp\.\d*\.?cpp", src) or src.startswith("eacsnd"):
        return "generated"
    if src.startswith("port_") or "nfs3hp::port" in name:
        return "ported"
    if src.startswith(("native_", "mad_")) or src in ("nfs3hp_main.cpp", "menu_data.cpp"):
        return "native"
    return "other"        # header helpers (MemoryAccessor, getMemory), the runtime, libc++


def short(name):
    m = re.search(r"sub_[0-9a-f]{6}", name)
    return m.group(0) if m else name.split("(")[0].split("::", 1)[-1][:60]


def main():
    args = sys.argv[1:]
    if not args:
        sys.exit(__doc__)
    build_id, snaps = read_log(args[0])
    so = next((a for a in args[1:] if a.endswith(".so")), None)
    if not so:
        found = sorted(glob.glob("android/app/build/intermediates/cxx/*/*/obj/arm64-v8a/libmain.so"),
                       key=os.path.getmtime)
        if not found:
            sys.exit("no libmain.so: pass the unstripped one")
        so = found[-1]
    out_md = next((a for a in args[1:] if a.endswith(".md")), None)
    lib_id = library_build_id(so)
    warn = ""
    if build_id and lib_id and build_id != lib_id:
        warn = "\n**Внимание:** build id журнала (%s) не совпадает с библиотекой (%s): имена могут быть чужими.\n" % (
            build_id[:12], lib_id[:12])

    starts, funcs = load_functions(so)
    src = source_files(so, starts)
    cat = {a: classify(funcs[a][1], src.get(a, "")) for a in starts}
    in_build = collections.Counter(cat.values())

    def owner(addr):
        i = bisect.bisect_right(starts, addr) - 1
        if i >= 0 and addr < starts[i] + funcs[starts[i]][0]:
            return starts[i]
        return None

    cache = {}

    def fn(addr):
        if addr not in cache:
            cache[addr] = owner(addr)
        return cache[addr]

    race = next((a for a in starts if re.search(r"Application::sub_4a3690\(", funcs[a][1])), None)

    # per snapshot: function -> (on stack, running); outside samples
    per = []
    prec = []          # per snapshot: Counter of (precision, category or "outside")
    words = collections.Counter()
    for ms, d, wd in snaps:
        words.update(wd)
        incl, run, outside = collections.Counter(), collections.Counter(), 0
        pc = collections.Counter()
        for key, c in d.items():
            if key >= OUTSIDE:
                outside += c
                pc[(precision_of(key), "outside")] += c
                continue
            f = fn(key & REL)
            if key & SELF:
                pc[(precision_of(key), cat.get(f, "other") if f is not None else "other")] += c
            if f is None:
                continue
            (run if key & SELF else incl)[f] += c
        per.append((ms, incl, run, outside))
        prec.append((ms, pc))

    tot_incl, tot_run, tot_out = collections.Counter(), collections.Counter(), 0
    for ms, incl, run, outside in per:
        tot_incl.update(incl)
        tot_run.update(run)
        tot_out += outside
    samples = sum(tot_run.values()) + tot_out
    end = snaps[-1][0] + 2000 if snaps else 0

    L = []
    w = L.append
    w("# Журнал работающих функций\n")
    w("Библиотека `%s`, снимков %d, длительность %d с, выборок %d (вне игры: %.1f%%).%s" % (
        os.path.basename(so), len(snaps), end // 1000, samples, 100 * tot_out / max(1, samples), warn))
    w("\nВыборка: функция, которая успела хоть раз попасть в стек, видна; редкие вызовы могут не попасть.")
    w("\n## Сколько функций видно\n")
    w("| категория | в сборке | в стеке | выполнялись | доля времени |")
    w("|---|---:|---:|---:|---:|")
    run_all = sum(tot_run.values()) or 1
    for k in ("generated", "native", "ported", "other"):
        seen = [f for f in tot_incl if cat.get(f) == k]
        running = [f for f in tot_run if cat.get(f) == k]
        w("| %s | %d | %d (%.0f%%) | %d | %.1f%% |" % (
            k, in_build[k], len(seen), 100 * len(seen) / max(1, in_build[k]), len(running),
            100 * sum(tot_run[f] for f in running) / run_all))
    w("\n«Доля времени» считается от выборок внутри библиотеки. `other` — шаблоны из заголовков, рантайм.")

    w("\n## По отрезкам по %d с\n" % (WINDOW_MS // 1000))
    w("| с | выборок | гонка на стеке | generated: в стеке / доля времени | native | ported |")
    w("|---|---:|---:|---|---|---|")
    for a in range(0, end, WINDOW_MS):
        inc, run, n_out = collections.Counter(), collections.Counter(), 0
        for ms, i, r, o in per:
            if a <= ms < a + WINDOW_MS:
                inc.update(i)
                run.update(r)
                n_out += o
        n = sum(run.values()) + n_out
        if not n:
            continue
        rr = sum(run.values()) or 1
        row = []
        for k in ("generated", "native", "ported"):
            row.append("%d / %.0f%%" % (sum(1 for f in inc if cat.get(f) == k),
                                         100 * sum(c for f, c in run.items() if cat.get(f) == k) / rr))
        w("| %d–%d | %d | %.0f%% | %s |" % (a // 1000, min(a + WINDOW_MS, end) // 1000, n,
                                           100 * inc.get(race, 0) / n if race else 0, " | ".join(row)))

    w("\n## Точность x87\n")
    w("Слово управления потока в момент выборки: до скольки бит x87 округляет каждый результат. "
      "«без x87» — поток, который слово не задавал (GL, система, Java).\n")
    w("Установки слова (fldcw и fninit) за сессию: " + (", ".join(
        "0x%04x (%s) ×%d" % (wd, PRECISION[(wd >> 8) & 3], c) for wd, c in sorted(words.items())) or "нет") + ".\n")
    allp = collections.Counter()
    for ms, pc in prec:
        allp.update(pc)
    order = [0, 2, 3, None]
    w("| где выполнялась выборка | " + " | ".join(PRECISION[p] for p in order) + " |")
    w("|---|" + "---:|" * len(order))
    for where in ("generated", "native", "ported", "other", "outside"):
        row = [allp.get((p, where), 0) for p in order]
        tot = sum(row)
        if tot:
            w("| %s | %s |" % (where, " | ".join("%d (%.0f%%)" % (v, 100 * v / tot) for v in row)))
    w("\n| с | " + " | ".join(PRECISION[p] for p in order) + " | гонка на стеке |")
    w("|---|" + "---:|" * (len(order) + 1))
    for a in range(0, end, WINDOW_MS):
        c = collections.Counter()
        for ms, pc in prec:
            if a <= ms < a + WINDOW_MS:
                for (p, where), v in pc.items():
                    c[p] += v
        n = sum(c.values())
        if not n:
            continue
        inc = collections.Counter()
        for ms, i, r, o in per:
            if a <= ms < a + WINDOW_MS:
                inc.update(i)
        w("| %d–%d | %s | %.0f%% |" % (a // 1000, min(a + WINDOW_MS, end) // 1000,
                                       " | ".join("%.0f%%" % (100 * c.get(p, 0) / n) for p in order),
                                       100 * inc.get(race, 0) / n if race else 0))

    for k in ("generated", "native", "ported"):
        w("\n## %s: на чём уходит время (топ-25)\n" % k)
        w("| выполнялась, выборок | на стеке | функция | файл |")
        w("|---:|---:|---|---|")
        for f in sorted((f for f in tot_run if cat.get(f) == k), key=lambda f: -tot_run[f])[:25]:
            w("| %d | %d | `%s` | %s |" % (tot_run[f], tot_incl[f], funcs[f][1][:95], src.get(f, "")))
    gen = [f for f in tot_incl if cat.get(f) == "generated"]
    w("\n## Сгенерированные функции в стеке: %d из %d\n" % (len(gen), in_build["generated"]))
    w(", ".join("%s ×%d" % (short(funcs[f][1]), tot_incl[f])
                for f in sorted(gen, key=lambda f: -tot_incl[f])[:40]))
    text = "\n".join(L)
    if out_md:
        open(out_md, "w", encoding="utf-8").write(text)
        json.dump({"%x" % f: {"name": funcs[f][1], "category": cat[f], "on_stack": tot_incl[f],
                              "running": tot_run.get(f, 0)} for f in tot_incl},
                  open(out_md + ".json", "w"))
    sys.stdout.reconfigure(encoding="utf-8")
    print(text)


if __name__ == "__main__":
    main()
