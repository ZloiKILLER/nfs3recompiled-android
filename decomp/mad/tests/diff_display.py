"""Differential test: decompiled display path (src/mad_display.c) against
nfs3.exe's sub_4fda20 / sub_4fd61c / sub_4fd6ef / sub_4fd7b6 / sub_4fd892 /
sub_4fd8ee / sub_4fdc90, run in Unicorn.

usage: diff_display.py [--seed N] [--scale K] [--cw 0x27f|0x37f]
"""
import argparse
import ctypes
import os
import random
import struct
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import native  # noqa: E402
from guest import Guest, DATA  # noqa: E402

TABLES = 0x9f4fb0
TABLES_LEN = 0x9f641c - TABLES
F_BUILD, F_1X, F_2X, F_2XB, F_WIDEN, F_WIDENB, F_SHOW = (0x4fda20, 0x4fd61c, 0x4fd6ef, 0x4fd7b6,
                                                         0x4fd892, 0x4fd8ee, 0x4fdc90)
CB = ctypes.CFUNCTYPE(ctypes.c_int32, ctypes.c_void_p)


class MadRgbTables(ctypes.Structure):
    _fields_ = [('blue', ctypes.c_uint32 * 354), ('green', ctypes.c_uint32 * 262),
                ('red', ctypes.c_uint32 * 306), ('luma', ctypes.c_uint32 * 128),
                ('cb', ctypes.c_uint32 * 128), ('cr', ctypes.c_uint32 * 128), ('depth', ctypes.c_int32)]


assert ctypes.sizeof(MadRgbTables) == TABLES_LEN


class MadSurface(ctypes.Structure):
    _fields_ = [('clip_left', ctypes.c_int32), ('clip_top', ctypes.c_int32),
                ('clip_right', ctypes.c_int32), ('clip_bottom', ctypes.c_int32),
                ('depth', ctypes.c_uint8), ('base', ctypes.c_void_p),
                ('row_offset', ctypes.c_void_p), ('x_offset', ctypes.c_void_p),
                ('large_picture_ok', CB), ('user', ctypes.c_void_p)]


def load(path):
    lib = ctypes.CDLL(path)
    P, I = ctypes.c_void_p, ctypes.c_int32
    for name, res, args in [('mad_build_rgb_tables', None, [P, I]),
                            ('mad_show_frame', I, [P, P, I, I, P, I, I, I]),
                            ('mad_row_1x', None, [P, P, P, I]), ('mad_row_2x', None, [P, P, P, I]),
                            ('mad_row_2x_between', None, [P, P, P, P, I]),
                            ('mad_row_widen', None, [P, P, I]), ('mad_row_widen_between', None, [P, P, P, I])]:
        f = getattr(lib, name)
        f.restype, f.argtypes = res, args
    return lib


def words7(rng, n):
    return struct.pack('<%dI' % n, *[rng.getrandbits(32) & 0x7f7f7f7f for _ in range(n)])


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--seed', type=int, default=1998)
    ap.add_argument('--scale', type=int, default=1)
    ap.add_argument('--cw', type=lambda s: int(s, 0), default=0x27f)
    ap.add_argument('--lib', default=native.lib('libmaddisp'))
    a = ap.parse_args()
    lib = load(a.lib)
    rng = random.Random(a.seed)
    rows, failures = [], []
    t0 = time.time()

    # tables
    for depth in (15, 16):
        g = Guest(a.cw)
        g.call(F_BUILD, eax=depth)
        t = MadRgbTables()
        lib.mad_build_rgb_tables(ctypes.byref(t), depth)
        gb = g.read(TABLES, TABLES_LEN - 4)          # without the depth word (the caller sets it)
        hb = bytes(ctypes.string_at(ctypes.addressof(t), TABLES_LEN - 4))
        bad = [n for n, ft in MadRgbTables._fields_[:-1]
               if gb[getattr(MadRgbTables, n).offset:getattr(MadRgbTables, n).offset + ctypes.sizeof(ft)]
               != hb[getattr(MadRgbTables, n).offset:getattr(MadRgbTables, n).offset + ctypes.sizeof(ft)]]
        rows.append(('sub_4fda20 colour tables, %d-bit' % depth, 1, 1 if bad else 0, ''))
        if bad:
            failures.append('tables %d: %s' % (depth, bad))

    # row converters
    fails = 0
    n = 300 * a.scale
    for case in range(n):
        depth = rng.choice((15, 16))
        g = Guest(a.cw)
        g.call(F_BUILD, eax=depth)
        t = MadRgbTables()
        ctypes.memmove(ctypes.addressof(t), g.read(TABLES, TABLES_LEN), TABLES_LEN)
        words = rng.randint(1, 200)
        A, B = words7(rng, words + 1), words7(rng, words + 1)
        ga, gb, gd = DATA, DATA + 0x2000, DATA + 0x4000
        g.write(ga, A)
        g.write(gb, B)
        g.write(gd, bytes(0x2000))
        ha = ctypes.create_string_buffer(A, len(A))
        hb = ctypes.create_string_buffer(B, len(B))
        hd = ctypes.create_string_buffer(0x2000)
        which = case % 5
        if which == 0:
            g.call(F_1X, eax=ga, edx=gd, ebx=words)
            lib.mad_row_1x(ctypes.byref(t), ha, hd, words)
        elif which == 1:
            g.call(F_2X, eax=ga, edx=gd, ebx=words)
            lib.mad_row_2x(ctypes.byref(t), ha, hd, words)
        elif which == 2:
            g.call(F_2XB, eax=ga, edx=gb, ebx=gd, ecx=words)
            lib.mad_row_2x_between(ctypes.byref(t), ha, hb, hd, words)
        elif which == 3:
            g.call(F_WIDEN, eax=ga, edx=gd, ebx=words)
            lib.mad_row_widen(ha, hd, words)
        else:
            g.call(F_WIDENB, eax=ga, edx=gb, ebx=gd, ecx=words)
            lib.mad_row_widen_between(ha, hb, hd, words)
        if g.read(gd, 0x2000) != hd.raw:
            fails += 1
            failures.append('row converter %d case %d words %d' % (which, case, words))
    rows.append(('sub_4fd61c/6ef/7b6/892/8ee row converters', n, fails, '1x, 2x, 2x between rows, widen, widen between'))

    # whole frames through sub_4fdc90
    fails = 0
    n = 120 * a.scale
    keep = []  # keep ctypes objects alive
    for case in range(n):
        g = Guest(a.cw)
        depth = rng.choice((15, 16))
        sw, sh = 640, 480
        pitch = sw * 2 + rng.choice((0, 64))
        base_off = rng.choice((0, 2))           # an origin two bytes off a dword
        surf_size = pitch * sh + 8
        g_surf, g_rows, g_xs, g_frame = DATA, DATA + 0x100000, DATA + 0x102000, DATA + 0x110000
        mode = rng.choice((0, 1, 2, 3, 3, 3))
        fw = rng.choice((16, 64, 160, 320))
        fh = rng.choice((16, 48, 120, 240))
        if mode == 0 and rng.random() < 0.5:
            fw, fh = 640, 480
        frame = words7(rng, fw * fh // 2 + 64)
        clip = [rng.choice((0, 0, 8, 33)), rng.choice((0, 0, 5, 40)),
                sw - rng.choice((0, 0, 16, 31)), sh - rng.choice((0, 0, 4, 37))]
        scale = 1 if mode == 0 else 2
        x = rng.randint(-40, sw - fw * scale // 2)
        y = rng.randint(-40, sh - fh * scale // 2)
        junk = bytes(rng.getrandbits(8) for _ in range(surf_size))
        g.write(g_surf, junk)
        g.write(g_rows, struct.pack('<%di' % sh, *[r * pitch for r in range(sh)]))
        g.write(g_xs, struct.pack('<%di' % sw, *[c * 2 for c in range(sw)]))
        g.write(g_frame, frame)
        for addr, val in ((0x565000, clip[0]), (0x565004, clip[1]), (0x565008, clip[2]), (0x56500c, clip[3]),
                          (0x565014, g_surf + base_off), (0x565020, g_rows), (0x565024, g_xs), (0x565028, 0)):
            g.put_u32(addr, val)
        g.write(0x565010, bytes([depth]))
        r = g.call(F_SHOW, [fh, mode], eax=x, edx=y, ebx=g_frame, ecx=fw)

        t = MadRgbTables()
        surf = ctypes.create_string_buffer(junk, surf_size)
        rowt = (ctypes.c_int32 * sh)(*[r_ * pitch for r_ in range(sh)])
        xt = (ctypes.c_int32 * sw)(*[c * 2 for c in range(sw)])
        hf = ctypes.create_string_buffer(frame, len(frame))
        s = MadSurface(clip[0], clip[1], clip[2], clip[3], depth, ctypes.addressof(surf) + base_off,
                       ctypes.addressof(rowt), ctypes.addressof(xt), CB(), None)
        keep.append((surf, rowt, xt, hf, s))
        hv = lib.mad_show_frame(ctypes.byref(t), ctypes.byref(s), x, y, hf, fw, fh, mode)
        problems = []
        if (r['eax'] & 0xffffffff) != (hv & 0xffffffff):
            problems.append('return %d vs %d' % (r['eax'], hv))
        gs = g.read(g_surf, surf_size)
        if gs != surf.raw:
            diff = sum(1 for i in range(surf_size) if gs[i] != surf.raw[i])
            problems.append('%d surface bytes differ' % diff)
        if problems:
            fails += 1
            failures.append('show case %d mode %d %dx%d at (%d,%d) clip %s depth %d: %s'
                            % (case, mode, fw, fh, x, y, clip, depth, problems))
        keep = keep[-4:]
    rows.append(('sub_4fdc90 whole frames to a 640x480 surface', n, fails,
                 'modes 0-3, clipping, 15/16-bit, misaligned origin'))

    print('x87 control word 0x%03x, seed %d, %.0f s' % (a.cw, a.seed, time.time() - t0))
    print('%-50s %8s %6s  %s' % ('test', 'cases', 'fails', 'notes'))
    for name, cases, f, note in rows:
        print('%-50s %8d %6d  %s' % (name, cases, f, note))
    for f in failures[:30]:
        print('FAIL', f)
    return 1 if any(r_[2] for r_ in rows) else 0


if __name__ == '__main__':
    sys.exit(main())
