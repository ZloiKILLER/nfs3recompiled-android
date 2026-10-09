"""Write MAD bitstreams for the tests, from the code tables in nfs3.exe.

Only the decoder's own code tables are used, read from the executable, so a
stream this writes is one the game's decoder reads as intended.  Layout
(reverse-engineered from sub_4f3600 / sub_510e48):

frame:      bits, as 16-bit little-endian words, most significant bit first
macroblock: in a predicted frame a type first --
              "00"            intra: six coded blocks
              "1"  mv mv      all six blocks predicted
              "01" bbbbbb mv mv   bit k (from the right) set: block k predicted
            then for each block k = 0..5 (4 luma, chroma A, chroma B):
              predicted: small(offset)            brightness offset
              coded:     dc:8 (signed)  { AC code }  EOB "10"
small:      "0" -> 0 | "10" xxxx -> xxxx+1 | "11" xxxx -> xxxx-16
AC code:    a code of table A or B, or escape "000001" level:10 run:6
"""
import os
import random
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'tools'))
from disx import read  # noqa: E402


def _recs(addr, n):
    v = struct.unpack('<%di' % (4 * n), read(addr, 16 * n))
    return [v[4 * k:4 * k + 4] for k in range(n)]


def _level10(v):
    v &= 0x3ff
    return v - 0x400 if v & 0x200 else v


SYMBOLS = []  # (bit string, run, level)
for rec in _recs(0x5652d4, 95)[1:]:
    length, rl, _, code = rec
    bits = format((code & 0xffff) >> (16 - length), '0%db' % length)
    SYMBOLS.append((bits, (rl >> 10) & 0x3f, _level10(rl)))
for rec in _recs(0x5658c4, 128):
    length, rl, _, code = rec
    bits = '0' * 8 + format((code & 0xffff) >> (16 - length), '0%db' % length)
    SYMBOLS.append((bits, (rl >> 10) & 0x3f, _level10(rl)))


def small(v):
    if v == 0:
        return '0'
    if 1 <= v <= 16:
        return '10' + format(v - 1, '04b')
    if -16 <= v <= -1:
        return '11' + format(v + 16, '04b')
    raise ValueError(v)


def escape(run, level):
    return '000001' + format(level & 0x3ff, '010b') + format(run & 0x3f, '06b')


class Writer:
    def __init__(self):
        self.bits = []

    def put(self, s):
        self.bits.append(s)

    def data(self, rng, tail_words=8):
        s = ''.join(self.bits)
        s += ''.join(rng.choice('01') for _ in range((-len(s)) % 16))
        out = bytearray()
        for i in range(0, len(s), 16):
            out += struct.pack('<H', int(s[i:i + 16], 2))
        for _ in range(tail_words):
            out += struct.pack('<H', rng.getrandbits(16))
        return bytes(out)


def coded_block(w, rng, max_symbols=None):
    """DC, AC codes with the scan position kept inside the block, EOB."""
    w.put(format(rng.getrandbits(8), '08b'))
    pos = 1
    budget = rng.choice([0, 1, 2, 4, 8, 20, 64]) if max_symbols is None else max_symbols
    count = 0
    while count < budget:
        kind = rng.random()
        if kind < 0.12:
            run = rng.randrange(0, 64)
            level = rng.randrange(-512, 512)
            if pos + run > 63:
                break
            w.put(escape(run, level))
        else:
            bits, run, level = rng.choice(SYMBOLS)
            if pos + run > 63:
                break
            w.put(bits)
        pos += run + 1
        count += 1
        if pos > 63:
            break
    w.put('10')


def macroblock(w, rng, inter, mv_range=16, inside=None):
    """`inside` = (x, y, width, height): keep the motion vector's reference
    block inside the picture (the original reads outside it otherwise)."""
    mask = 0
    if inter:
        kind = rng.random()
        if kind < 0.25:
            w.put('00')
        elif kind < 0.55:
            w.put('1')
            mask = 0x3f
        else:
            mask = rng.getrandbits(6)
            w.put('01' + format(mask, '06b'))
        if kind >= 0.25:
            if inside:
                x, y, width, height = inside
                dx = rng.randint(max(-mv_range, -x), min(mv_range, width - 16 - x))
                dy = rng.randint(max(-mv_range, -y), min(mv_range, height - 16 - y))
            else:
                dx = rng.randint(-mv_range, mv_range)
                dy = rng.randint(-mv_range, mv_range)
            w.put(small(dx))
            w.put(small(dy))
    for k in range(6):
        if mask & (1 << k):
            w.put(small(rng.randint(-16, 16)))
        else:
            coded_block(w, rng)
