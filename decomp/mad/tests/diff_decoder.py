"""Differential test: decompiled MAD decoder (src/) against nfs3.exe's own code.

The original functions run in Unicorn, the decompiled ones natively, on the
same inputs from the same starting state; after every step the whole decoder
state (0x9f0c38..0x9f254c, the coefficient block and the IDCT scratch) and
every output byte are compared.

usage: diff_decoder.py [--seed N] [--scale K] [--cw 0x27f|0x37f] [--lib path]
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
import madstream  # noqa: E402

STATE_BASE = 0x9f0c38
STATE_LEN = 0x9f2548 - STATE_BASE     # up to the bit pointer
PTR_ADDR = 0x9f2548
COEF = 0x56cf2c
TMP = 0x56ce0c

F_BUILD, F_BEGIN, F_SKIP, F_SMALL = 0x4f30d0, 0x4f3560, 0x4f3350, 0x4f33b0
F_FILLDC, F_PLUMA, F_PCHROMA, F_MB = 0x4f33e0, 0x4f3420, 0x4f34f0, 0x4f3600
F_BLOCK, F_IDCT, F_PUT = 0x510e48, 0x510cbd, 0x510ff8


class MadDecoder(ctypes.Structure):
    _fields_ = [('qmat', ctypes.c_int32 * 64), ('clamp', ctypes.c_uint8 * 256),
                ('blk_a', ctypes.c_int32 * 64), ('blk_b', ctypes.c_int32 * 64),
                ('vlc_mv', ctypes.c_uint32 * 64), ('vlc_after9', ctypes.c_uint32 * 256),
                ('vlc_after6', ctypes.c_uint32 * 256), ('vlc_first9', ctypes.c_uint32 * 512),
                ('luma', ctypes.c_int32 * 256), ('tables_ready', ctypes.c_int32),
                ('bits', ctypes.c_uint32), ('inter', ctypes.c_int32), ('nbits', ctypes.c_int32),
                ('ptr', ctypes.c_void_p), ('coef', ctypes.c_int32 * 64),
                ('idct_tmp', ctypes.c_int32 * 72), ('overflow', ctypes.c_int32)]


assert MadDecoder.ptr.offset == STATE_LEN, MadDecoder.ptr.offset


class Pair:
    """A guest and a host copy of the same decoder, plus mirrored buffers."""

    def __init__(self, lib, cw):
        self.g = Guest(cw)
        self.lib = lib
        self.d = MadDecoder()
        self.bufs = {}       # name -> (guest address, host ctypes buffer)
        self.next = DATA

    def buf(self, name, size):
        host = ctypes.create_string_buffer(size)
        self.bufs[name] = (self.next, host)
        self.next += (size + 0xfff) & ~0xfff
        return self.bufs[name]

    def put(self, name, data, offset=0):
        ga, host = self.bufs[name]
        self.g.write(ga + offset, data)
        ctypes.memmove(ctypes.addressof(host) + offset, bytes(data), len(data))

    def haddr(self, name, offset=0):
        return ctypes.addressof(self.bufs[name][1]) + offset

    def gaddr(self, name, offset=0):
        return self.bufs[name][0] + offset

    def sync_from_guest(self):
        """Start the host copy from the guest's state, byte for byte."""
        raw = self.g.read(STATE_BASE, STATE_LEN)
        ctypes.memmove(ctypes.addressof(self.d), raw, STATE_LEN)
        ctypes.memmove(ctypes.addressof(self.d.coef), self.g.read(COEF, 256), 256)
        ctypes.memmove(ctypes.addressof(self.d.idct_tmp), self.g.read(TMP, 288), 288)
        gp = self.g.u32(PTR_ADDR)
        self.d.ptr = None
        for name, (ga, host) in self.bufs.items():
            if ga <= gp < ga + len(host):
                self.d.ptr = ctypes.addressof(host) + (gp - ga)

    def diff_state(self, what=''):
        problems = []
        hs = bytes(ctypes.string_at(ctypes.addressof(self.d), STATE_LEN))
        gs = self.g.read(STATE_BASE, STATE_LEN)
        if hs != gs:
            for name, ftype in MadDecoder._fields_:
                off = getattr(MadDecoder, name).offset
                size = ctypes.sizeof(ftype)
                if off + size <= STATE_LEN and hs[off:off + size] != gs[off:off + size]:
                    problems.append('state.%s' % name)
        if bytes(ctypes.string_at(ctypes.addressof(self.d.coef), 256)) != self.g.read(COEF, 256):
            problems.append('coef')
        if bytes(ctypes.string_at(ctypes.addressof(self.d.idct_tmp), 288)) != self.g.read(TMP, 288):
            problems.append('idct_tmp')
        gp = self.g.u32(PTR_ADDR)
        hp = self.d.ptr or 0
        gpos = hpos = None
        for name, (ga, host) in self.bufs.items():
            if ga <= gp < ga + len(host) + 64:
                gpos = (name, gp - ga)
            if ctypes.addressof(host) <= hp < ctypes.addressof(host) + len(host) + 64:
                hpos = (name, hp - ctypes.addressof(host))
        if gpos != hpos:
            problems.append('ptr %s vs %s' % (gpos, hpos))
        return problems

    def diff_buf(self, name):
        ga, host = self.bufs[name]
        hb = bytes(host.raw)
        gb = self.g.read(ga, len(hb))
        if hb == gb:
            return None
        first = next(i for i in range(len(hb)) if hb[i] != gb[i])
        count = sum(1 for i in range(len(hb)) if hb[i] != gb[i])
        return '%s: %d bytes differ, first at %d (guest %02x host %02x)' % (name, count, first, gb[first], hb[first])


def load_lib(path):
    lib = ctypes.CDLL(path)
    P, I, U = ctypes.c_void_p, ctypes.c_int32, ctypes.c_uint32
    sig = {
        'mad_build_tables': (None, [P]),
        'mad_begin_frame': (None, [P, P, I, U]),
        'mad_decode_macroblock': (None, [P, P, P, I]),
        'mad_skip_bits': (None, [P, I]),
        'mad_read_small': (I, [P]),
        'mad_fill_dc': (None, [P, P, I]),
        'mad_predict_luma': (None, [P, I, I, P, I]),
        'mad_predict_chroma': (None, [P, I, I, P, I]),
        'mad_decode_block': (I, [P]),
        'mad_idct': (None, [P, P, I]),
        'mad_put_row': (None, [P, P, P, P, P]),
    }
    for name, (res, args) in sig.items():
        f = getattr(lib, name)
        f.restype = res
        f.argtypes = args
    return lib


class Results:
    def __init__(self):
        self.rows = []
        self.failures = []

    def record(self, name, cases, fails, note=''):
        self.rows.append((name, cases, fails, note))

    def fail(self, name, case, msg):
        if len(self.failures) < 40:
            self.failures.append('%s case %s: %s' % (name, case, msg))


# ---------------------------------------------------------------- tests

def test_tables(lib, cw, res):
    p = Pair(lib, cw)
    p.g.call(F_BUILD)
    lib.mad_build_tables(ctypes.byref(p.d))
    probs = p.diff_state()
    zero = [i for i in range(512) if p.d.vlc_first9[i] == 0]
    res.record('sub_4f30d0 build tables', 1, 1 if probs else 0,
               'empty first-9 entries: %d; clamp[127]=%d' % (len(zero), p.d.clamp[127]))
    if probs:
        res.fail('tables', 0, probs)


def fresh_pair(lib, cw):
    p = Pair(lib, cw)
    p.g.call(F_BUILD)
    p.sync_from_guest()
    return p


def begin(p, rng, data_name, inter, quant):
    p.g.call(F_BEGIN, eax=p.gaddr(data_name), edx=inter, ebx=quant)
    p.lib.mad_begin_frame(ctypes.byref(p.d), p.haddr(data_name), inter, quant)


def test_bits(lib, cw, res, rng, n):
    p = fresh_pair(lib, cw)
    p.buf('bits', 4096)
    fails = 0
    for case in range(n):
        p.put('bits', bytes(rng.getrandbits(8) for _ in range(4096)))
        begin(p, rng, 'bits', rng.randint(0, 1), rng.randint(0, 255))
        probs = p.diff_state()
        for step in range(200):
            if probs:
                break
            if rng.random() < 0.5:
                k = rng.randint(0, 16)
                p.g.call(F_SKIP, eax=k)
                lib.mad_skip_bits(ctypes.byref(p.d), k)
            else:
                r = p.g.call(F_SMALL)
                v = lib.mad_read_small(ctypes.byref(p.d))
                if (r['eax'] & 0xffffffff) != (v & 0xffffffff):
                    probs = ['read_small %d vs %d' % (r['eax'], v)]
            probs = probs or p.diff_state()
        if probs:
            fails += 1
            res.fail('bits', case, probs)
    res.record('sub_4f3560 + sub_4f3350 + sub_4f33b0 bit reader', n, fails, '200 random reads each')


def test_blocks(lib, cw, res, rng, n, garbage=False):
    p = fresh_pair(lib, cw)
    p.buf('stream', 8192)
    fails = skipped = 0
    for case in range(n):
        if garbage:
            data = bytes(rng.getrandbits(8) for _ in range(8192))
        else:
            w = madstream.Writer()
            for _ in range(8):
                madstream.coded_block(w, rng)
            data = w.data(rng, tail_words=32)
            data = data + bytes(8192 - len(data))
        p.put('stream', data[:8192])
        quant = rng.choice([rng.randint(1, 31), rng.randint(0, 255)])
        begin(p, rng, 'stream', rng.randint(0, 1), quant)
        probs = p.diff_state()
        for blk in range(8):
            if probs:
                break
            p.d.overflow = 0
            # The host first: a corrupt stream must not reach the guest when
            # it would make the original write outside the block.
            hv = lib.mad_decode_block(ctypes.byref(p.d))
            if p.d.overflow & 1:
                # Not run in the guest; put the host back to the guest's state.
                skipped += 1
                p.sync_from_guest()
                break
            r = p.g.call(F_BLOCK)
            if (r['eax'] & 0xffffffff) != (hv & 0xffffffff):
                probs = ['count %d vs %d' % (r['eax'], hv)]
            probs = probs or p.diff_state()
            # the decoded block through both kinds of output
            if not probs:
                if hv == 1:
                    p.g.call(F_FILLDC, eax=STATE_BASE + 0x1500, edx=16)   # luma buffer
                    lib.mad_fill_dc(ctypes.byref(p.d), ctypes.addressof(p.d.luma), 16)
                else:
                    p.g.call(F_IDCT, eax=STATE_BASE + 0x1500, edx=16)
                    lib.mad_idct(ctypes.byref(p.d), ctypes.addressof(p.d.luma), 16)
                probs = p.diff_state()
        if probs:
            fails += 1
            res.fail('garbage blocks' if garbage else 'blocks', case, probs)
            p.sync_from_guest()
    name = 'sub_510e48 coefficients (+ sub_4f33e0 / sub_510cbd)'
    res.record(name + (' -- random bytes' if garbage else ''), n, fails,
               ('%d streams stopped where the original writes outside the block' % skipped) if garbage else
               '8 blocks a stream, every code of tables A and B, escapes')


def test_idct(lib, cw, res, rng, n):
    p = fresh_pair(lib, cw)
    fails = 0
    for case in range(n):
        kind = case % 4
        if kind == 0:     # realistic: few coefficients, dequantised sizes
            coef = [0] * 64
            for _ in range(rng.randint(1, 12)):
                coef[rng.randrange(64)] = rng.randint(-1023, 1023) * rng.randint(1, 1 << 17)
        elif kind == 1:   # dense, moderate
            coef = [rng.randint(-(1 << 22), 1 << 22) for _ in range(64)]
        elif kind == 2:   # any 32-bit values (wrap-around everywhere)
            coef = [rng.getrandbits(32) - (1 << 31) for _ in range(64)]
        else:             # rows with only a DC (the row pass's shortcut)
            coef = [0] * 64
            for r in range(8):
                coef[r * 8] = rng.randint(-(1 << 24), 1 << 24)
                if rng.random() < 0.5:
                    coef[r * 8 + rng.randint(1, 7)] = rng.randint(-(1 << 20), 1 << 20)
        raw = struct.pack('<64i', *coef)
        p.g.write(COEF, raw)
        ctypes.memmove(ctypes.addressof(p.d.coef), raw, 256)
        stride = rng.choice([8, 16])
        p.g.call(F_IDCT, eax=STATE_BASE + 0x1500, edx=stride)
        lib.mad_idct(ctypes.byref(p.d), ctypes.addressof(p.d.luma), stride)
        probs = p.diff_state()
        if probs:
            fails += 1
            res.fail('idct', case, probs)
            p.sync_from_guest()
    res.record('sub_510cbd / sub_510a50 / sub_510bb3 IDCT', n, fails, 'x87 control word 0x%03x' % cw)
    return fails


def test_idct_critical(lib, cw, res, rng):
    """Rows on which rounding each product (x87 at 53 bits) and fusing it with
    the add (an FMA) round differently -- found by tests/fma_search.c."""
    path = os.path.join(HERE, 'idct_critical_rows.txt')
    rows = [list(map(int, l.split())) for l in open(path) if l.strip()]
    p = fresh_pair(lib, cw)
    fails = 0
    for case, (i1, i3, i5, i7) in enumerate(rows):
        for r in range(8):
            coef = [0] * 64
            coef[r * 8 + 1], coef[r * 8 + 3], coef[r * 8 + 5], coef[r * 8 + 7] = i1, i3, i5, i7
            coef[r * 8] = rng.randint(-(1 << 20), 1 << 20)
            raw = struct.pack('<64i', *coef)
            p.g.write(COEF, raw)
            ctypes.memmove(ctypes.addressof(p.d.coef), raw, 256)
            p.g.call(F_IDCT, eax=STATE_BASE + 0x1500, edx=16)
            lib.mad_idct(ctypes.byref(p.d), ctypes.addressof(p.d.luma), 16)
            probs = p.diff_state()
            if probs:
                fails += 1
                res.fail('idct critical', case, probs)
                p.sync_from_guest()
                break
    res.record('IDCT rows where an FMA would round differently', len(rows), fails,
               'x87 control word 0x%03x' % cw)


def test_put_and_predict(lib, cw, res, rng, n):
    p = fresh_pair(lib, cw)
    p.buf('ref', 64 * 1024)
    p.buf('out', 64)
    fails = 0
    for case in range(n):
        # random block buffers in both
        raw = bytes(rng.getrandbits(8) for _ in range(256 * 4 + 128 * 4))
        p.g.write(STATE_BASE + 0x1500, raw[:1024])
        p.g.write(STATE_BASE + 0x200, raw[1024:])
        ctypes.memmove(ctypes.addressof(p.d.luma), raw[:1024], 1024)
        ctypes.memmove(ctypes.addressof(p.d.blk_a), raw[1024:], 512)
        row = rng.randrange(16)
        p.put('out', bytes(64))
        p.g.call(F_PUT, eax=STATE_BASE + 0x1500 + row * 64, edx=STATE_BASE + 0x200 + (row >> 1) * 32,
                 ebx=p.gaddr('out'))
        lib.mad_put_row(ctypes.byref(p.d), ctypes.addressof(p.d.luma) + row * 64,
                        ctypes.addressof(p.d.blk_a) + (row >> 1) * 32,
                        ctypes.addressof(p.d.blk_b) + (row >> 1) * 32, p.haddr('out'))
        probs = []
        d = p.diff_buf('out')
        if d:
            probs.append(d)
        # prediction
        p.put('ref', bytes(rng.getrandbits(8) for _ in range(64 * 1024)))
        wdw = rng.choice([8, 16, 24, 160])
        at = rng.randrange(0, 64 * 1024 - wdw * 4 * 16 - 64)
        off = rng.randint(-16, 16) - 0x40
        if rng.random() < 0.5:
            odd = rng.randint(0, 1)
            p.g.call(F_PLUMA, [off], eax=p.gaddr('ref', at), edx=odd, ebx=wdw, ecx=STATE_BASE + 0x1500)
            lib.mad_predict_luma(p.haddr('ref', at), odd, wdw, ctypes.addressof(p.d.luma), off)
        else:
            plane = rng.randint(0, 1)
            target = STATE_BASE + (0x200 if plane else 0x300)
            host = ctypes.addressof(p.d.blk_a) if plane else ctypes.addressof(p.d.blk_b)
            p.g.call(F_PCHROMA, [off], eax=p.gaddr('ref', at), edx=plane, ebx=wdw, ecx=target)
            lib.mad_predict_chroma(p.haddr('ref', at), plane, wdw, host, off)
        probs += p.diff_state()
        if probs:
            fails += 1
            res.fail('put/predict', case, probs)
    res.record('sub_510ff8 output row, sub_4f3420 / sub_4f34f0 prediction', n, fails, '')


def test_frames(lib, cw, res, rng, n_frames, width, height):
    """Whole frames through sub_4f3560 + sub_4f3600, macroblock by macroblock
    in the player's order, the previous output as the reference."""
    p = fresh_pair(lib, cw)
    pad = 40 * width * 2          # forty rows above and below: motion up to 16
    size = width * height * 2 + 2 * pad
    p.buf('f0', size)
    p.buf('f1', size)
    p.buf('stream', 256 * 1024)
    junk = bytes(rng.getrandbits(8) for _ in range(size))
    p.put('f0', junk)
    p.put('f1', bytes(reversed(junk)))
    fails = mbs = 0
    cur, ref = 'f0', 'f1'
    for frame in range(n_frames):
        inter = 0 if frame == 0 or rng.random() < 0.15 else 1
        w = madstream.Writer()
        for _ in range((width // 16) * (height // 16)):
            madstream.macroblock(w, rng, inter)
        data = w.data(rng, tail_words=64)
        p.put('stream', data + bytes(256 * 1024 - len(data)))
        quant = rng.randint(1, 31)
        begin(p, rng, 'stream', inter, quant)
        probs = p.diff_state()
        for y in range(0, height, 16):
            for x in range(0, width, 16):
                if probs:
                    break
                off = pad + ((y * width + x) // 2) * 4
                p.g.call(F_MB, eax=p.gaddr(ref, off), edx=p.gaddr(cur, off), ebx=width, ecx=p.gaddr(cur))
                lib.mad_decode_macroblock(ctypes.byref(p.d), p.haddr(ref, off), p.haddr(cur, off), width)
                mbs += 1
                probs = p.diff_state()
                d = p.diff_buf(cur)
                if d:
                    probs.append(d)
                if probs:
                    probs.append('frame %d mb (%d,%d) inter %d' % (frame, x, y, inter))
        if probs:
            fails += 1
            res.fail('frames %dx%d' % (width, height), frame, probs)
            break
        cur, ref = ref, cur
    res.record('sub_4f3600 macroblocks, whole %dx%d frames' % (width, height), n_frames, fails,
               '%d macroblocks, key and predicted frames' % mbs)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--seed', type=int, default=1998)
    ap.add_argument('--scale', type=int, default=1)
    ap.add_argument('--cw', type=lambda s: int(s, 0), default=0x27f)
    ap.add_argument('--lib', default=native.lib('libmad'))
    a = ap.parse_args()
    lib = load_lib(a.lib)
    rng = random.Random(a.seed)
    res = Results()
    t0 = time.time()
    test_tables(lib, a.cw, res)
    test_bits(lib, a.cw, res, rng, 20 * a.scale)
    test_blocks(lib, a.cw, res, rng, 150 * a.scale)
    test_blocks(lib, a.cw, res, rng, 60 * a.scale, garbage=True)
    test_idct(lib, a.cw, res, rng, 2000 * a.scale)
    test_idct_critical(lib, a.cw, res, rng)
    test_put_and_predict(lib, a.cw, res, rng, 400 * a.scale)
    test_frames(lib, a.cw, res, rng, 6 * a.scale, 64, 48)
    test_frames(lib, a.cw, res, rng, 3 * a.scale, 320, 240)
    print('x87 control word 0x%03x, seed %d, %.0f s' % (a.cw, a.seed, time.time() - t0))
    print('%-62s %8s %6s  %s' % ('test', 'cases', 'fails', 'notes'))
    for name, cases, fails, note in res.rows:
        print('%-62s %8d %6d  %s' % (name, cases, fails, note))
    for f in res.failures:
        print('FAIL', f)
    return 1 if any(r[2] for r in res.rows) else 0


if __name__ == '__main__':
    sys.exit(main())
