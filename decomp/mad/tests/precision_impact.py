"""How much the x87 precision matters for the picture.

The decompiled decoder does the IDCT's floating-point part in double, which is
what the x87 gives at 53-bit precision (control word 0x27f) and what the
port's emulated FPU gives.  Here the original runs at 64-bit precision (0x37f,
the control word after fninit) and every macroblock starts from the guest's
own state, so differences do not accumulate; the output pixels are counted.
"""
import ctypes
import os
import random
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import diff_decoder as D  # noqa: E402
import native  # noqa: E402
import madstream  # noqa: E402


def main(frames=20, width=320, height=240, cw=0x37f, seed=7):
    lib = D.load_lib(native.lib('libmad'))
    rng = random.Random(seed)
    p = D.fresh_pair(lib, cw)
    pad = 40 * width * 2
    size = width * height * 2 + 2 * pad
    p.buf('f0', size)
    p.buf('f1', size)
    p.buf('stream', 256 * 1024)
    junk = bytes(rng.getrandbits(8) for _ in range(size))
    p.put('f0', junk)
    p.put('f1', junk)
    cur, ref = 'f0', 'f1'
    pixels = diff_luma = diff_chroma = worst = mbs_diff = mbs = 0
    for frame in range(frames):
        inter = 0 if frame == 0 else 1
        w = madstream.Writer()
        for _ in range((width // 16) * (height // 16)):
            madstream.macroblock(w, rng, inter)
        data = w.data(rng, tail_words=64)
        p.put('stream', data + bytes(256 * 1024 - len(data)))
        D.begin(p, rng, 'stream', inter, rng.randint(1, 31))
        for y in range(0, height, 16):
            for x in range(0, width, 16):
                off = pad + ((y * width + x) // 2) * 4
                p.sync_from_guest()
                # the host's frame buffers from the guest's, too
                for name in (cur, ref):
                    ga, host = p.bufs[name]
                    ctypes.memmove(ctypes.addressof(host), p.g.read(ga, len(host)), len(host))
                p.g.call(D.F_MB, eax=p.gaddr(ref, off), edx=p.gaddr(cur, off), ebx=width, ecx=p.gaddr(cur))
                lib.mad_decode_macroblock(ctypes.byref(p.d), p.haddr(ref, off), p.haddr(cur, off), width)
                mbs += 1
                changed = False
                for row in range(16):
                    a = p.gaddr(cur, off + row * width * 2)
                    g = p.g.read(a, 32)
                    h = ctypes.string_at(p.haddr(cur, off + row * width * 2), 32)
                    for i in range(32):
                        if i % 4 >= 2:
                            pixels += 1
                        if g[i] != h[i]:
                            changed = True
                            worst = max(worst, abs(g[i] - h[i]))
                            if i % 4 >= 2:
                                diff_luma += 1
                            else:
                                diff_chroma += 1
                mbs_diff += changed
        cur, ref = ref, cur
    print('x87 control word 0x%03x vs decompiled (double), %d frames %dx%d' % (cw, frames, width, height))
    print('macroblocks with any difference: %d of %d' % (mbs_diff, mbs))
    print('luma samples different: %d of %d (%.4f%%)' % (diff_luma, pixels, 100.0 * diff_luma / pixels))
    print('chroma bytes different: %d' % diff_chroma)
    print('largest difference: %d (of 0..127)' % worst)


if __name__ == '__main__':
    main(*[int(a, 0) for a in sys.argv[1:]])
