"""Does the differential test notice mistakes?  Each mutation is a plausible
decompilation error; the test must fail on every one of them."""
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, '..'))
SRC = os.path.join(ROOT, 'src', 'mad_decoder.c')
PY = sys.executable
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import native  # noqa: E402
from build_libs import compile_lib  # noqa: E402

MUTATIONS = [
    ('clamp writes index 127 too', 'for (int32_t v = -128; v < 127; v++)', 'for (int32_t v = -128; v < 128; v++)'),
    ('IDCT outputs 1 and 6 swapped', 'out[1] = e0 + h.ut;', 'out[1] = e0 - h.ut;'),
    ('IDCT rounding term changed', 'const double b = (yc0 - yzc2) + magic;', 'const double b = (yc0 - yzc2 * 1.0000001) + magic;'),
    ('fused product (x87 rounds twice)', 'const double a = (zc1 + yzc2) + magic;', 'const double a = fma(yz, c2, zc1) + magic;'),
    ('half-pixel luma source off by one', '{ 2, 7, 6, 11, 10, 15, 14, 19 }', '{ 2, 7, 6, 11, 10, 15, 14, 18 }'),
    ('refill threshold 15 instead of 16', '    if (left < 16)\n', '    if (left < 15)\n'),
    ('Q16 multiply without rounding', '(lo >> 16) + ((lo >> 15) & 1u)', '(lo >> 16)'),
    ('motion x halved with / instead of sar', '(uint32_t)(dx >> 1)', '(uint32_t)(dx / 2)'),
    ('chroma planes swapped in output', 'dst[i] = (left << 24) | (right << 16) | (a << 8) | b;', 'dst[i] = (left << 24) | (right << 16) | (b << 8) | a;'),
    ('DC dequantised like AC', '(uint32_t)MAD_QUANT_MATRIX[0] << 15', '(uint32_t)MAD_QUANT_MATRIX[0] << 14'),
    ('escape run/level fields swapped', 'e = (bits & 0xffff0000u) | 0x10;', 'e = (((bits >> 16) & 0x3fu) << 22) | (((bits >> 22) & 0x3fu) << 16) | 0x10;'),
]

# Equivalent rewrites: the test must NOT fail on these.
EQUIVALENT = [
    # With every AC coefficient zero the full row transform gives the DC in
    # all eight outputs too, so the shortcut's test may include it or not.
    ('row IDCT zero test includes DC', '(in[5] | in[3] | in[1] | in[7] | in[2] | in[6] | in[4]) == 0', '(in[0] | in[5] | in[3] | in[1] | in[7] | in[2] | in[6] | in[4]) == 0'),
    # Bits 6 and 7 of the "01" mask byte are never tested.
    ('mask byte masked to six bits', 'mask = d->bits >> 24;', 'mask = (d->bits >> 24) & 0x3f;'),
]


def run(original, build, i, old, new):
    mutated = original.replace(old, new, 1)
    if 'fma(' in new:
        mutated = '#include <math.h>\n' + mutated
    path = os.path.join(build, 'mut%d.c' % i)
    with open(path, 'w') as f:
        f.write(mutated)
    lib = os.path.join(build, 'libmut%d%s' % (i, native.EXT))
    compile_lib([path], lib)
    r = subprocess.run([PY, os.path.join(HERE, 'diff_decoder.py'), '--lib', lib, '--scale', '1'],
                       capture_output=True, text=True)
    os.remove(path)
    os.remove(lib)
    return r.returncode != 0


def main():
    original = open(SRC).read()
    build = native.BUILD
    os.makedirs(build, exist_ok=True)
    ok = 0
    for i, (name, old, new) in enumerate(EQUIVALENT):
        assert old in original, name
        failed = run(original, build, 100 + i, old, new)
        print('%-40s %s' % ('[equivalent] ' + name, 'FALSE ALARM' if failed else 'passes, as it should'))
        ok += not failed
    caught = 0
    for i, (name, old, new) in enumerate(MUTATIONS):
        assert old in original, name
        failed = run(original, build, i, old, new)
        print('%-40s %s' % (name, 'caught' if failed else 'MISSED'))
        caught += failed
    print('%d of %d mutations caught, %d of %d equivalent rewrites pass' % (caught, len(MUTATIONS), ok, len(EQUIVALENT)))
    return 0 if caught == len(MUTATIONS) and ok == len(EQUIVALENT) else 1


if __name__ == '__main__':
    sys.exit(main())
