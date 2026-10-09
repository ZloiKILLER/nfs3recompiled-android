"""Mutation check for the display path: each plausible mistake must fail."""
import os, subprocess, sys
HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, '..'))
SRC = os.path.join(ROOT, 'src', 'mad_display.c')
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import native  # noqa: E402
from build_libs import compile_lib  # noqa: E402
MUTATIONS = [
    ('rounded and truncated halves swapped', '*dst++ = (right & 0xffff0000u) | (left & 0xffffu);', '*dst++ = (left & 0xffff0000u) | (right & 0xffffu);'),
    ('chroma bias +0.5 dropped', 'const double c = (double)(k - 64) + half;', 'const double c = (double)(k - 64);'),
    ('green rounding +2 -> +4 at 16-bit', 'g = (v + 2) >> 2;', 'g = (v + 4) >> 2;'),
    ('widen: next pixel from wrong byte', 'const uint32_t next_left = (n >> 8) & 0x007f0000u;', 'const uint32_t next_left = (n << 8) & 0x007f0000u;'),
    ('average without mask', 'return ((a + b) >> 1) & 0x7f7f7f7fu;', 'return ((a + b) >> 1);'),
    ('clip-left rounds down', 'const int32_t n = idiv(s->clip_left - x + step - 1, step);', 'const int32_t n = idiv(s->clip_left - x, step);'),
    ('mode 3 odd rows start one row late', 'for (int32_t row = y + 1; row < y_end - 1; row += 2)\n        {\n            const uint8_t *next = p + row_bytes;\n            mad_row_widen_between', 'for (int32_t row = y + 3; row < y_end - 1; row += 2)\n        {\n            const uint8_t *next = p + row_bytes;\n            mad_row_widen_between'),
    ('alignment ignores origin', 'x = (int32_t)(((uint32_t)x & ~1u) | (misalign >> 1));', 'x = (int32_t)((uint32_t)x & ~1u);'),
]
def main():
    original = open(SRC).read(); build = native.BUILD; os.makedirs(build, exist_ok=True); caught = 0
    for i, (name, old, new) in enumerate(MUTATIONS):
        assert old in original, name
        path = os.path.join(build, 'dmut%d.c' % i); lib = os.path.join(build, 'libdmut%d%s' % (i, native.EXT))
        open(path, 'w').write(original.replace(old, new, 1))
        compile_lib([path], lib)
        r = subprocess.run([sys.executable, os.path.join(HERE, 'diff_display.py'), '--lib', lib], capture_output=True, text=True)
        print('%-40s %s' % (name, 'caught' if r.returncode else 'MISSED')); caught += r.returncode != 0
        os.remove(path); os.remove(lib)
    print('%d of %d mutations caught' % (caught, len(MUTATIONS)))
    return 0 if caught == len(MUTATIONS) else 1
if __name__ == '__main__':
    sys.exit(main())
