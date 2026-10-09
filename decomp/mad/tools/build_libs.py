"""Build the decompiled code as shared libraries for the tests, into build/.

usage: python tools/build_libs.py [--ubsan]
The compiler is $CC (default: cc); gcc or clang, MinGW on Windows.  The
decoder must be built without FMA contraction (-ffp-contract=off): a fused
multiply-add rounds once where the original x87 code rounds twice.
"""
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, '..'))
SRC = os.path.join(ROOT, 'src')
sys.path.insert(0, os.path.join(ROOT, 'tests'))
import native  # noqa: E402

CC = os.environ.get('CC', 'cc')
FLAGS = ['-std=c99', '-O2', '-Wall', '-Wextra', '-ffp-contract=off', '-shared']
if sys.platform != 'win32':
    FLAGS.append('-fPIC')

LIBS = {
    'libmad': ['mad_decoder.c'],
    'libmaddisp': ['mad_display.c'],
    'libmadplay': ['mad_decoder.c', 'mad_player.c'],
}


def compile_lib(sources, out, extra=()):
    """Compile `sources` (paths, or names in src/) into the library `out`."""
    srcs = [s if os.path.isabs(s) else os.path.join(SRC, s) for s in sources]
    cmd = [CC] + FLAGS + list(extra) + ['-I', SRC, '-o', out] + srcs + ['-lm']
    subprocess.check_call(cmd)


def main():
    os.makedirs(native.BUILD, exist_ok=True)
    for name, sources in LIBS.items():
        compile_lib(sources, native.lib(name))
        print('built', native.lib(name))
    if '--ubsan' in sys.argv:
        out = native.lib('libmad_ubsan')
        compile_lib(['mad_decoder.c'], out, ['-O1', '-g', '-fsanitize=undefined',
                                              '-fno-sanitize-recover=undefined', '-static-libubsan'])
        print('built', out)


if __name__ == '__main__':
    main()
