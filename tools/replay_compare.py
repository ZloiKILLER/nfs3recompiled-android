#!/usr/bin/env python3
"""Holds two car traces against each other (src/nfs3hp/replay_trace.cpp).

  python tools/replay_compare.py a.bin b.bin [--metres 0.5]

The same replay on two builds should drive the same cars.  Says the first step
where anything differs at all, the first where a car is further than --metres
from itself in the other run, and how far each car ended up from itself.
Exit status 0 when no car ever strays further than --metres, 1 otherwise.
"""
import argparse
import struct
import sys

FIELDS = ["node", "x", "y", "z", "vx", "vy", "vz", "nosex", "nosey", "nosez", "spinx", "spiny", "spinz"]
TICKS_PER_SECOND = 64


def read(path):
    data = open(path, "rb").read()
    magic, version, what, cars = struct.unpack_from("<4sIII", data, 0)
    if magic != b"NFTR" or version != 1:
        sys.exit("%s: not a car trace" % path)
    step = 4 + cars * 13 * 4
    count = (len(data) - 16) // step
    return {"what": what, "cars": cars, "data": data, "step": step, "count": count}


def record(t, i):
    """The clock and, for each car, its node and twelve floats at step i."""
    at = 16 + i * t["step"]
    clock = struct.unpack_from("<I", t["data"], at)[0]
    cars = []
    for c in range(t["cars"]):
        cars.append(struct.unpack_from("<I12f", t["data"], at + 4 + c * 52))
    return clock, cars


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("a")
    ap.add_argument("b")
    ap.add_argument("--metres", type=float, default=0.5)
    args = ap.parse_args()
    a, b = read(args.a), read(args.b)
    what = ("race", "replay")
    print("A: %s, %d cars, %d steps; B: %s, %d cars, %d steps" % (
        what[a["what"]], a["cars"], a["count"], what[b["what"]], b["cars"], b["count"]))
    if a["cars"] != b["cars"]:
        print("different cars: not the same race")
        return 1
    # Equal bytes settle it at once.
    n = min(a["count"], b["count"])
    if a["data"][16:16 + n * a["step"]] == b["data"][16:16 + n * b["step"]]:
        print("MATCH: all %d common steps are identical to the bit (%.1f s)" % (
            n, record(a, n - 1)[0] / TICKS_PER_SECOND if n else 0))
        return 0 if a["count"] == b["count"] else 1

    first_any = None
    first_far = None
    worst = [0.0] * a["cars"]
    last = [0.0] * a["cars"]
    for i in range(n):
        ca, ra = record(a, i)
        cb, rb = record(b, i)
        if ca != cb and first_any is None:
            first_any = (i, ca, None, "clock", ca, cb)
        for c in range(a["cars"]):
            if ra[c] != rb[c] and first_any is None:
                f = next(k for k in range(13) if ra[c][k] != rb[c][k])
                first_any = (i, ca, c, FIELDS[f], ra[c][f], rb[c][f])
            d = sum((ra[c][k] - rb[c][k]) ** 2 for k in (1, 2, 3)) ** 0.5
            worst[c] = max(worst[c], d)
            last[c] = d
            if d > args.metres and first_far is None:
                first_far = (i, ca, c, d)
    if first_any:
        i, clock, c, field, va, vb = first_any
        who = "clock" if c is None else "car %d, %s" % (c, field)
        print("first difference: step %d (%.2f s), %s: %r / %r" % (i, clock / TICKS_PER_SECOND, who, va, vb))
    if first_far:
        i, clock, c, d = first_far
        print("DIVERGED: step %d (%.2f s), car %d is %.2f m from itself" % (i, clock / TICKS_PER_SECOND, c, d))
    else:
        print("MATCH within %.2f m over %d steps" % (args.metres, n))
    print("metres apart, worst / at the end, by car: " + ", ".join(
        "%d: %.2f/%.2f" % (c, worst[c], last[c]) for c in range(a["cars"])))
    if a["count"] != b["count"]:
        print("step counts differ: %d / %d" % (a["count"], b["count"]))
    return 1 if first_far else 0


if __name__ == "__main__":
    sys.exit(main())
