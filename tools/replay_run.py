#!/usr/bin/env python3
"""Plays a replay on the phone by itself and fetches its car trace.

  python tools/replay_run.py out.bin [--replay some.rp3] [--extra name ...] [--compare ref.bin] [--seconds n]

Pushes the replay (without --replay, the last race's stays), force-stops the game,
starts it in debug mode with replay_trace and auto_replay -- no movies, from the
first menu straight into the replay -- waits for its [TRACE] line and pulls
logs/trace-replay.bin to out.bin.  --extra adds a boolean launch extra
(live_counter, say).  --compare holds the trace against ref.bin while it is written
and stops at the first step that differs, then runs tools/replay_compare.py on what
there is.  --seconds stops after that many seconds of the race if all was alike, for
a quick check.  The screen must stay on: the game's time stands still in the background.
"""
import argparse
import os
import struct
import subprocess
import sys
import time

PACKAGE = "dev.nfs3hp.port"
FILES = "/sdcard/Android/data/%s/files" % PACKAGE
ENV = dict(os.environ, MSYS_NO_PATHCONV="1")
TICKS_PER_SECOND = 64


def adb(*args, check=True):
    return subprocess.run(["adb", *args], capture_output=True, text=True, errors="replace", env=ENV, check=check).stdout


def held_against(reference, remote, local, seconds):
    """What there is of the trace so far against `reference`: a sentence if it has
    differed, or has gone `seconds` of the race alike; None to go on."""
    if subprocess.run(["adb", "pull", remote, local], capture_output=True, env=ENV).returncode:
        return None
    data = open(local, "rb").read()
    if len(data) < 16:
        return None
    cars = struct.unpack_from("<I", data, 12)[0]
    step = 4 + cars * 13 * 4
    steps = (min(len(data), len(reference)) - 16) // step
    length = 16 + steps * step
    if data[:length] != reference[:length]:
        return "the trace differs from the reference"
    clock = struct.unpack_from("<I", data, 16 + (steps - 1) * step)[0] if steps else 0
    if seconds is not None and clock / TICKS_PER_SECOND >= seconds:
        return "alike to the bit for %.1f s of the race, stopped there" % (clock / TICKS_PER_SECOND)
    return None


def main():
    sys.stdout.reconfigure(line_buffering=True)
    ap = argparse.ArgumentParser()
    ap.add_argument("out")
    ap.add_argument("--replay")
    ap.add_argument("--extra", action="append", default=[])
    ap.add_argument("--compare")
    ap.add_argument("--timeout", type=int, default=900)
    ap.add_argument("--seconds", type=float)
    args = ap.parse_args()

    adb("shell", "am", "force-stop", PACKAGE)
    if args.replay:
        adb("push", args.replay, FILES + "/replay.rp3")
    adb("shell", "rm", "-f", FILES + "/logs/trace-replay.bin")
    adb("shell", "input", "keyevent", "KEYCODE_WAKEUP")
    adb("logcat", "-c")
    start = ["shell", "am", "start", "-n", PACKAGE + "/.SplashActivity", "--ez", "debug", "true",
             "--ez", "replay_trace", "true", "--ez", "auto_replay", "true"]
    for name in args.extra:
        start += ["--ez", name, "true"]
    adb(*start)
    print("started; waiting for the replay to end (the screen must stay on)")

    reference = open(args.compare, "rb").read() if args.compare else None
    remote = FILES + "/logs/trace-replay.bin"
    began = time.time()
    done = None
    announced = False
    while time.time() - began < args.timeout:
        time.sleep(5)
        if reference is not None and announced:
            verdict = held_against(reference, remote, args.out, args.seconds)
            if verdict:
                print(verdict, "in %.0f s" % (time.time() - began))
                adb("shell", "am", "force-stop", PACKAGE)
                adb("pull", remote, args.out)
                here = os.path.dirname(os.path.abspath(__file__))
                result = subprocess.call([sys.executable, os.path.join(here, "replay_compare.py"), args.compare, args.out])
                return 1 if "differs" in verdict else result
        log = adb("logcat", "-d", "-s", "SDL/APP:I")
        lines = [l for l in log.splitlines() if "[TRACE]" in l]
        if lines and not announced:
            print(lines[0].split("[TRACE]")[1].strip())
            announced = True
        done = next((l for l in lines if "replay done" in l), None)
        if done:
            break
    if not done:
        adb("shell", "am", "force-stop", PACKAGE)
        sys.exit("no end of the replay within %d s" % args.timeout)
    print(done.split("[TRACE]")[1].strip(), "in %.0f s" % (time.time() - began))
    adb("shell", "am", "force-stop", PACKAGE)
    adb("pull", FILES + "/logs/trace-replay.bin", args.out)
    print("saved", args.out)
    if args.compare:
        here = os.path.dirname(os.path.abspath(__file__))
        return subprocess.call([sys.executable, os.path.join(here, "replay_compare.py"), args.compare, args.out])
    return 0


if __name__ == "__main__":
    sys.exit(main())
