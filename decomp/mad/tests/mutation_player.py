"""Mutation check for the player: each plausible mistake must fail diff_player.py."""
import os, subprocess, sys
HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, '..'))
SRC = os.path.join(ROOT, 'src', 'mad_player.c')
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(ROOT, 'tools'))
import native  # noqa: E402
from build_libs import compile_lib  # noqa: E402
MUTATIONS = [
    ('MADe skip threshold 133 -> 134 ms', 'time - now < 0x85', 'time - now < 0x86'),
    ('catch-up threshold 67 -> 66 ms', 'if (time - now < 0x43)', 'if (time - now < 0x42)'),
    ('MADe used as reference', 'if (rd32(chunk) != MAD_EXTRA)\n            ref = frame;\n        h->stream_release(h->user, stream, chunk);\n        mad_queue_put(p, time);\n        if (sound_handle', 'ref = frame;\n        h->stream_release(h->user, stream, chunk);\n        mad_queue_put(p, time);\n        if (sound_handle'),
    ('queue insert: equal times keep order', 'while (i > 0 && time <= s->queue[i - 1].time)', 'while (i > 0 && time < s->queue[i - 1].time)'),
    ('show: frame due exactly now not shown', 'while (n < s->queued && now >= s->queue[n].time)', 'while (n < s->queued && now > s->queue[n].time)'),
    ('drift /8 as arithmetic shift', 'return v / 8;', 'return v >> 3;'),
    ('drift threshold 264 -> 263 ms', 'if (size > 0x108)', 'if (size > 0x107)'),
    ('no show between macroblock rows', 'if (show_between && !shown)', 'if (0 && show_between && !shown)'),
    ('centering uses >> 1', 'x = (h->screen_width - s->width) / 2;', 'x = (h->screen_width - s->width) >> 1;'),
    ('software display does not force skipping', '*skip_extra = 1;  /* software display', '(void)0;  /* software display'),
    ('end loop: key not checked', '            mad_check_key(p);\n            if (status[0] == 3 || s->stop)', '            if (status[0] == 3 || s->stop)'),
    ('quality thresholds 133 MHz -> 134 MHz', 'if (m->cpu_hz < 133000000u)\n        quality = 1;', 'if (m->cpu_hz < 134000000u)\n        quality = 1;'),
]
def main():
    original = open(SRC).read(); build = native.BUILD; os.makedirs(build, exist_ok=True); caught = 0
    for i, (name, old, new) in enumerate(MUTATIONS):
        assert old in original, name
        path = os.path.join(build, 'pmut%d.c' % i); lib = os.path.join(build, 'libpmut%d%s' % (i, native.EXT))
        open(path, 'w').write(original.replace(old, new, 1))
        compile_lib(['mad_decoder.c', path], lib)
        r = subprocess.run([sys.executable, os.path.join(HERE, 'diff_player.py'), '--lib', lib, '--movies', '8'], capture_output=True, text=True)
        print('%-44s %s' % (name, 'caught' if r.returncode else 'MISSED')); caught += r.returncode != 0
        os.remove(path); os.remove(lib)
    print('%d of %d mutations caught' % (caught, len(MUTATIONS)))
    return 0 if caught == len(MUTATIONS) else 1
if __name__ == '__main__':
    sys.exit(main())
