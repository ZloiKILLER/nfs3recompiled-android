"""End-to-end differential test of the movie player.

nfs3.exe's sub_4960e0 / sub_495bc0 (showmad) and their helpers run in Unicorn
with the real decoder underneath; every function they call outside the
player -- EA's stream and sound libraries, memory, GetTickCount, the display,
the keyboard -- is replaced by a scripted stand-in.  src/mad_player.c runs
natively against the same stand-ins (through MadPlayerHost / MadMovieHost).
Both must make the same outside calls with the same arguments in the same
order, show the same pictures at the same moments, and end in the same state.

usage: diff_player.py [--seed N] [--movies K]
"""
import argparse
import ctypes
import hashlib
import os
import random
import struct
import sys
import time

from unicorn.x86_const import UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EAX, UC_X86_REG_EBX, \
    UC_X86_REG_ECX, UC_X86_REG_EDX
from unicorn import UC_HOOK_CODE

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import native  # noqa: E402
from guest import Guest, DATA  # noqa: E402
import madstream  # noqa: E402
from diff_decoder import MadDecoder  # noqa: E402

STREAM, SOUND_CHAN, FILE, SOUND_PLAYER, SOUND_HANDLE = 0x5000, 0x6000, 0x7000, 0x8000, 7
STATE_GUEST = 0x79f290

V = ctypes.c_void_p
I32, U32 = ctypes.c_int32, ctypes.c_uint32
F = ctypes.CFUNCTYPE


class Host(ctypes.Structure):
    _fields_ = [
        ('user', V),
        ('alloc', F(V, V, U32)), ('release_memory', F(None, V, V)),
        ('stream_create', F(V, V, V, U32)), ('stream_channel', F(V, V, V)), ('stream_setup', F(None, V, V)),
        ('stream_open', F(V, V, V, ctypes.c_char_p)), ('stream_next', F(V, V, V)),
        ('stream_release', F(None, V, V, V)), ('stream_ended', F(I32, V, V)), ('stream_pending', F(I32, V, V)),
        ('stream_close', F(None, V, V)),
        ('sound_defaults', F(None, V, V, I32)), ('sound_memory', F(U32, V, I32)),
        ('sound_create', F(V, V, V, V, I32, I32, V, U32)), ('sound_start', F(I32, V, V, V)),
        ('sound_status', F(None, V, I32, ctypes.POINTER(I32))), ('sound_destroy', F(None, V, V)),
        ('ticks', F(U32, V)), ('pump', F(None, V)), ('display_mode', F(I32, V)), ('key_pressed', F(I32, V)),
        ('display_open', F(None, V, I32, I32, I32, I32, I32, I32)), ('display_frame', F(None, V, V)),
        ('display_close', F(None, V)), ('fatal', F(None, V, ctypes.c_char_p)),
        ('screen_width', I32), ('screen_height', I32),
    ]


class MovieHost(ctypes.Structure):
    _fields_ = [('user', V), ('movies_off', I32), ('movie_dir', ctypes.c_char_p), ('cpu_hz', U32),
                ('file_exists', F(I32, V, ctypes.c_char_p)), ('flush_input', F(None, V)),
                ('sound_pause', F(None, V)), ('sound_resume', F(None, V)),
                ('music_stop', F(None, V)), ('music_start', F(None, V))]


class Entry(ctypes.Structure):
    _fields_ = [('time', I32), ('frame', V)]


class PlayerState(ctypes.Structure):
    _fields_ = [('queue', Entry * 16)] + [(n, I32) for n in (
        'frame_ms_q16', 'dropped_catching_up', 'clock_drift', 'height', 'audio_seen', 'width')] + [
        ('clock_start', U32)] + [(n, I32) for n in (
            'chunks_read', 'stop', 'extra_skipped', 'has_sound', 'sound_handle', 'frames_shown', 'disp_h',
            'disp_y', 'disp_x', 'disp_w', 'queued', 'buffers', 'total_late_ms', 'frames_dropped')]


class Player(ctypes.Structure):
    _fields_ = [('s', PlayerState), ('dec', MadDecoder), ('host', ctypes.POINTER(Host))]


GUEST_FIELDS = ['frame_ms_q16', 'dropped_catching_up', 'clock_drift', 'height', 'audio_seen', 'width',
                'clock_start', 'chunks_read', 'stop', 'extra_skipped', 'has_sound', 'sound_handle',
                'frames_shown', 'disp_h', 'disp_y', 'disp_x', 'disp_w', 'queued', 'buffers',
                'total_late_ms', 'frames_dropped']


# ---------------------------------------------------------------- the movie

def make_movie(rng, w, h, frames):
    """Chunks of the main channel (video and junk) and of the sound channel."""
    dur = rng.choice([0x42aaab, 0x3c0000, 0x215555, 0x7d0000])  # 15, 16.7, 30, 8 fps in ms 16.16
    video = []
    kinds = []
    for i in range(frames):
        if i == 0 or rng.random() < 0.12:
            kind = b'MADk'
        else:
            kind = rng.choice([b'MADm', b'MADe', b'MADm'])
        kinds.append(kind)
        wtr = madstream.Writer()
        for my in range(0, h, 16):
            for mx in range(0, w, 16):
                madstream.macroblock(wtr, rng, kind != b'MADk', mv_range=4, inside=(mx, my, w, h))
        data = wtr.data(rng, tail_words=16)
        head = bytearray(0x18)
        head[0:4] = kind
        struct.pack_into('<I', head, 4, 0x18 + len(data))
        struct.pack_into('<I', head, 0x0c, dur)
        struct.pack_into('<hh', head, 0x10, w, h)
        head[0x15] = rng.randint(1, 31)
        video.append(bytes(head) + data)
    main = []
    for c in video:
        if rng.random() < 0.15:
            main.append(b'ZZZZ' + bytes(28))      # not a MAD chunk: thrown away
        main.append(c)
    sound = [b'SCDl' + bytes(60) for _ in range(rng.randint(0, 6))]
    return main, sound, kinds


# ---------------------------------------------------------------- the world

class Script:
    """The stand-ins' behaviour, the same for both sides: everything follows
    from the order of the calls, which is what is being compared."""

    def __init__(self, rng, main, sound, variant):
        self.main, self.sound = list(main), list(sound)
        self.mi = self.si = 0
        self.log = []
        self.tick = 1000
        self.tick_steps = [rng.choice([0, 1, 3, 8, 20, 40, 70, 150, 300]) for _ in range(97)]
        self.tick_calls = 0
        self.played = 0
        self.status_calls = 0
        self.next_calls = 0
        self.key_calls = 0
        self.variant = variant
        self.sound_end = variant.get('sound_end', 1500)

    def ticks(self):
        self.tick += self.tick_steps[self.tick_calls % len(self.tick_steps)]
        self.tick_calls += 1
        return self.tick

    def stream_next(self, channel):
        self.next_calls += 1
        if channel == STREAM:
            if self.mi >= len(self.main):
                return None
            if self.variant.get('stall') and self.next_calls % 7 == 3:
                return None                     # not delivered yet: the player polls
            self.mi += 1
            return ('main', self.mi - 1)
        if self.si >= len(self.sound):
            return None
        self.si += 1
        return ('sound', self.si - 1)

    def stream_ended(self, channel):
        return 1 if channel == STREAM and self.mi >= len(self.main) else 0

    def stream_pending(self, channel):
        return len(self.sound) - self.si

    def sound_status(self):
        self.status_calls += 1
        self.played += self.variant.get('sound_step', 37) + (self.status_calls % 5) * 11
        done = 3 if self.played >= self.sound_end else 1
        return [done, self.played, 0, 0]

    def key(self):
        self.key_calls += 1
        k = self.variant.get('key_at')
        return 1 if k is not None and self.key_calls >= k else 0


def chunk_name(rchunks, sizes, a):
    """A pointer into a chunk as (chunk, offset), by exact chunk bounds."""
    for base, k in rchunks.items():
        if base <= a < base + sizes[k]:
            return '%s%d+%d' % (k[0], k[1], a - base)
    return hex(a)


def frame_hash(b):
    return hashlib.sha1(b).hexdigest()[:12]


class GuestWorld:
    def __init__(self, script, w, h, screen):
        self.s = script
        self.g = Guest(0x27f)
        self.uc = self.g.uc
        self.next_alloc = DATA + 0x100000
        self.allocs = {}
        self.chunks = {}
        self.w, self.h = w, h
        self.screen = screen
        addr = DATA + 0x10000
        for i, c in enumerate(script.main):
            self.g.write(addr, c)
            self.chunks[('main', i)] = addr
            addr += (len(c) + 0x1f) & ~0xf
        for i, c in enumerate(script.sound):
            self.g.write(addr, c)
            self.chunks[('sound', i)] = addr
            addr += (len(c) + 0x1f) & ~0xf
        self.rchunks = {v: k for k, v in self.chunks.items()}
        self.sizes = {('main', i): len(c) for i, c in enumerate(script.main)}
        self.sizes.update({('sound', i): len(c) for i, c in enumerate(script.sound)})
        self.g.put_u32(0x564384, screen[0])
        self.g.put_u32(0x564388, screen[1])
        self.stubs = {
            0x4e1620: (self.alloc, 0), 0x4e1890: (self.free, 0),
            0x4cfa80: (self.stream_create, 4), 0x4cfe70: (self.stream_channel, 0),
            0x4cfca0: (self.stream_setup, 4), 0x4cfef0: (self.stream_open, 0),
            0x4d0300: (self.stream_next, 0), 0x4d03c0: (self.stream_release, 0),
            0x4d04c0: (self.stream_ended, 0), 0x4d0460: (self.stream_pending, 0),
            0x4cfd30: (self.stream_close, 0),
            0x4e9b20: (self.sound_defaults, 0), 0x4f3a10: (self.sound_memory, 0),
            0x4f3a60: (self.sound_create, 8), 0x4f3a80: (self.sound_start, 0),
            0x4f2f50: (self.sound_status, 0), 0x4f43c0: (self.sound_destroy, 0),
            0x4f2790: (self.ticks, 0), 0x4e7630: (self.pump, 0), 0x4df340: (self.display_mode, 0),
            0x451960: (self.key_pressed, 0), 0x4df1f0: (self.display_open, 8),
            0x4df2b0: (self.display_frame, 0), 0x4df310: (self.display_close, 0),
            0x401010: (self.fatal, 0),
            # sub_4960e0's
            0x4e0e60: (self.file_exists, 0), 0x4f0240: (self.flush_input, 0),
            0x4d5d30: (self.sound_pause, 0), 0x4d5d70: (self.sound_resume, 0),
            0x410aa0: (self.music_stop, 0), 0x410330: (self.music_start, 0),
        }
        for addr in self.stubs:
            self.uc.hook_add(UC_HOOK_CODE, self._hook, begin=addr, end=addr)

    def _hook(self, uc, address, size, user):
        fn, pop = self.stubs[address]
        regs = {r: uc.reg_read(c) for r, c in (('eax', UC_X86_REG_EAX), ('ebx', UC_X86_REG_EBX),
                                                ('ecx', UC_X86_REG_ECX), ('edx', UC_X86_REG_EDX))}
        esp = uc.reg_read(UC_X86_REG_ESP)
        args = [self.g.u32(esp + 4 + 4 * i) for i in range(3)]
        res = fn(regs, args)
        if res is not None:
            uc.reg_write(UC_X86_REG_EAX, res & 0xffffffff)
        ret = self.g.u32(esp)
        uc.reg_write(UC_X86_REG_ESP, esp + 4 + pop)
        uc.reg_write(UC_X86_REG_EIP, ret)

    def name(self, a):
        if a in self.allocs:
            return 'A%d' % self.allocs[a]
        return chunk_name(self.rchunks, self.sizes, a)

    # stand-ins
    def alloc(self, r, a):
        size = r['edx']
        addr = self.next_alloc
        self.next_alloc += (size + 0xfff) & ~0xfff
        self.allocs[addr] = len(self.allocs)
        self.s.log.append(('alloc', size, 'A%d' % self.allocs[addr]))
        return addr

    def free(self, r, a):
        self.s.log.append(('free', self.name(r['eax'])))

    def stream_create(self, r, a):
        self.s.log.append(('stream_create', self.name(r['ecx']), a[0], r['eax'], r['edx'], r['ebx']))
        return STREAM

    def stream_channel(self, r, a):
        self.s.log.append(('stream_channel', r['eax'], r['edx'], r['ebx'], r['ecx']))
        return SOUND_CHAN

    def stream_setup(self, r, a):
        self.s.log.append(('stream_setup', r['eax'], r['edx'], a[0]))

    def stream_open(self, r, a):
        path = self.g.read(r['edx'], 64).split(b'\0')[0]
        self.s.log.append(('stream_open', r['eax'], path, r['ebx'], r['ecx']))
        return FILE

    def stream_next(self, r, a):
        k = self.s.stream_next(r['eax'])
        self.s.log.append(('stream_next', r['eax'], k))
        return self.chunks[k] if k else 0

    def stream_release(self, r, a):
        self.s.log.append(('stream_release', r['eax'], self.name(r['edx'])))

    def stream_ended(self, r, a):
        return self.s.stream_ended(r['eax'])

    def stream_pending(self, r, a):
        return self.s.stream_pending(r['eax'])

    def stream_close(self, r, a):
        self.s.log.append(('stream_close', r['eax']))

    def sound_defaults(self, r, a):
        self.s.log.append(('sound_defaults', r['edx']))

    def sound_memory(self, r, a):
        self.s.log.append(('sound_memory', r['eax']))
        return 0x2000

    def sound_create(self, r, a):
        self.s.log.append(('sound_create', r['eax'], r['ebx'], r['ecx'], self.name(a[0]), a[1]))
        return SOUND_PLAYER

    def sound_start(self, r, a):
        self.s.log.append(('sound_start', r['eax'], r['edx'], r['ebx']))
        return self.s.variant.get('sound_start', SOUND_HANDLE)

    def sound_status(self, r, a):
        st = self.s.sound_status()
        self.g.write(r['edx'], struct.pack('<4i', *st))
        self.s.log.append(('sound_status', r['eax'], st[0], st[1]))

    def sound_destroy(self, r, a):
        self.s.log.append(('sound_destroy', r['eax']))

    def ticks(self, r, a):
        return self.s.ticks()

    def pump(self, r, a):
        self.s.log.append(('pump', r['eax']))

    def display_mode(self, r, a):
        return self.s.variant.get('display_mode', 2)

    def key_pressed(self, r, a):
        self.s.log.append(('key', r['eax']))
        return self.s.key()

    def display_open(self, r, a):
        self.s.log.append(('display_open', r['eax'], r['edx'], r['ebx'], r['ecx'], a[0], a[1]))

    def display_frame(self, r, a):
        b = self.g.read(r['eax'], self.w * self.h * 2)
        self.s.log.append(('display_frame', self.name(r['eax']), frame_hash(b)))

    def display_close(self, r, a):
        self.s.log.append(('display_close',))

    def fatal(self, r, a):
        raise RuntimeError('guest fatal: %r' % self.g.read(a[0], 64).split(b'\0')[0])

    def file_exists(self, r, a):
        path = self.g.read(r['eax'], 64).split(b'\0')[0]
        self.s.log.append(('file_exists', path))
        return self.s.variant.get('file_exists', 1)

    def flush_input(self, r, a):
        self.s.log.append(('flush_input',))

    def sound_pause(self, r, a):
        self.s.log.append(('sound_pause',))

    def sound_resume(self, r, a):
        self.s.log.append(('sound_resume',))

    def music_stop(self, r, a):
        self.s.log.append(('music_stop',))

    def music_start(self, r, a):
        self.s.log.append(('music_start',))

    def state(self):
        raw = self.g.read(STATE_GUEST, 0x79f364 - STATE_GUEST)
        q = [struct.unpack_from('<iI', raw, 8 * i) for i in range(16)]
        vals = struct.unpack_from('<%di' % len(GUEST_FIELDS), raw, 128)
        st = {'queue': [(t, self.name(f) if f else 0) for t, f in q]}
        for n, v in zip(GUEST_FIELDS, vals):
            st[n] = v & 0xffffffff if n == 'clock_start' else v
        return st


class HostWorld:
    def __init__(self, lib, script, w, h, screen):
        self.s = script
        self.lib = lib
        self.w, self.h = w, h
        self.bufs = []
        self.allocs = {}
        self.chunks = {}
        for i, c in enumerate(script.main):
            b = ctypes.create_string_buffer(c + bytes(256), len(c) + 256)
            self.bufs.append(b)
            self.chunks[('main', i)] = ctypes.addressof(b)
        for i, c in enumerate(script.sound):
            b = ctypes.create_string_buffer(c, len(c))
            self.bufs.append(b)
            self.chunks[('sound', i)] = ctypes.addressof(b)
        self.rchunks = {v: k for k, v in self.chunks.items()}
        self.sizes = {('main', i): len(c) for i, c in enumerate(script.main)}
        self.sizes.update({('sound', i): len(c) for i, c in enumerate(script.sound)})
        S = self
        h = Host()
        self.cbs = []

        def cb(name, fn):
            f = getattr(Host, name)
            proto = [t for n, t in Host._fields_ if n == name][0]
            c = proto(fn)
            self.cbs.append(c)
            setattr(h, name, c)

        def alloc(u, size):
            b = ctypes.create_string_buffer(size)
            S.bufs.append(b)
            a = ctypes.addressof(b)
            S.allocs[a] = len(S.allocs)
            S.s.log.append(('alloc', size, 'A%d' % S.allocs[a]))
            return a
        cb('alloc', alloc)
        cb('release_memory', lambda u, p: S.s.log.append(('free', S.name(p))))
        cb('stream_create', lambda u, b, size: (S.s.log.append(('stream_create', S.name(b), size, 2, 2, 2)), STREAM)[1])
        cb('stream_channel', lambda u, st: (S.s.log.append(('stream_channel', st, 2, 0xffff, 0x4353)), SOUND_CHAN)[1])
        cb('stream_setup', lambda u, st: S.s.log.append(('stream_setup', st, 1, 2)))
        cb('stream_open', lambda u, st, path: (S.s.log.append(('stream_open', st, path, 0, 0)), FILE)[1])

        def nxt(u, ch):
            k = S.s.stream_next(ch)
            S.s.log.append(('stream_next', ch, k))
            return S.chunks[k] if k else None
        cb('stream_next', nxt)
        cb('stream_release', lambda u, ch, c: S.s.log.append(('stream_release', ch, S.name(c))))
        cb('stream_ended', lambda u, ch: S.s.stream_ended(ch))
        cb('stream_pending', lambda u, ch: S.s.stream_pending(ch))
        cb('stream_close', lambda u, st: S.s.log.append(('stream_close', st)))
        cb('sound_defaults', lambda u, p, n: S.s.log.append(('sound_defaults', n)))
        cb('sound_memory', lambda u, n: (S.s.log.append(('sound_memory', n)), 0x2000)[1])
        cb('sound_create', lambda u, ch, p, a1, a2, mem, size: (
            S.s.log.append(('sound_create', ch, a1, a2, S.name(mem), size)), SOUND_PLAYER)[1])
        cb('sound_start', lambda u, pl, f: (S.s.log.append(('sound_start', pl, 0, f)),
                                            S.s.variant.get('sound_start', SOUND_HANDLE))[1])

        def status(u, handle, out):
            st = S.s.sound_status()
            for i in range(4):
                out[i] = st[i]
            S.s.log.append(('sound_status', handle, st[0], st[1]))
        cb('sound_status', status)
        cb('sound_destroy', lambda u, pl: S.s.log.append(('sound_destroy', pl)))
        cb('ticks', lambda u: S.s.ticks())
        cb('pump', lambda u: S.s.log.append(('pump', 0)))
        cb('display_mode', lambda u: S.s.variant.get('display_mode', 2))
        cb('key_pressed', lambda u: (S.s.log.append(('key', 1)), S.s.key())[1])
        cb('display_open', lambda u, x, y, w_, h_, q, sw: S.s.log.append(
            ('display_open', x & 0xffffffff, y & 0xffffffff, w_, h_, q, sw)))
        cb('display_frame', lambda u, f: S.s.log.append(
            ('display_frame', S.name(f), frame_hash(ctypes.string_at(f, S.w * S.h * 2)))))
        cb('display_close', lambda u: S.s.log.append(('display_close',)))

        def fatal(u, msg):
            raise RuntimeError('host fatal %r' % msg)
        cb('fatal', fatal)
        h.screen_width, h.screen_height = screen
        self.host = h
        self.player = Player()
        self.player.host = ctypes.pointer(h)

        mh = MovieHost()
        self.mcbs = []

        def mcb(name, fn):
            proto = [t for n, t in MovieHost._fields_ if n == name][0]
            c = proto(fn)
            self.mcbs.append(c)
            setattr(mh, name, c)

        def exists(u, path):
            S.s.log.append(('file_exists', path))
            return S.s.variant.get('file_exists', 1)
        mcb('file_exists', exists)
        mcb('flush_input', lambda u: S.s.log.append(('flush_input',)))
        mcb('sound_pause', lambda u: S.s.log.append(('sound_pause',)))
        mcb('sound_resume', lambda u: S.s.log.append(('sound_resume',)))
        mcb('music_stop', lambda u: S.s.log.append(('music_stop',)))
        mcb('music_start', lambda u: S.s.log.append(('music_start',)))
        self.movie_host = mh

    def name(self, a):
        if a in self.allocs:
            return 'A%d' % self.allocs[a]
        return chunk_name(self.rchunks, self.sizes, a or 0)

    def state(self):
        s = self.player.s
        st = {'queue': [(e.time, self.name(e.frame) if e.frame else 0) for e in s.queue]}
        for n in GUEST_FIELDS:
            st[n] = getattr(s, n)
        return st


def load(path):
    lib = ctypes.CDLL(path)
    lib.mad_player_play.argtypes = [V, ctypes.c_char_p, ctypes.POINTER(I32), I32, I32, I32]
    lib.mad_player_play.restype = None
    lib.mad_play_movie.argtypes = [V, V, ctypes.c_char_p]
    lib.mad_play_movie.restype = I32
    return lib


def run_case(lib, rng, case, variant, through_wrapper):
    w, h = rng.choice([(64, 48), (96, 64), (160, 112)])
    frames = rng.randint(3, 18)
    main, sound, kinds = make_movie(rng, w, h, frames)
    screen = rng.choice([(640, 480), (512, 384)])
    seed = rng.getrandbits(32)

    gs = Script(random.Random(seed), main, sound, variant)
    gw = GuestWorld(gs, w, h, screen)
    hs = Script(random.Random(seed), main, sound, variant)
    hw = HostWorld(lib, hs, w, h, screen)

    if through_wrapper:
        cpu = variant['cpu_hz']
        movie_dir = b'D:\\FEDATA\\MOVIES\\'
        gw.g.write(0x7a32b4, movie_dir + b'\0')
        gw.g.put_u32(0x7a1fdc, variant.get('movies_off', 0))
        gw.g.put_u32(0x5652a4, cpu)
        name = b'titleav.mad'
        gw.g.write(DATA + 0x500, name + b'\0')
        gw.g.call(0x4960e0, eax=DATA + 0x500)
        mh = hw.movie_host
        mh.movies_off = variant.get('movies_off', 0)
        mh.movie_dir = movie_dir
        mh.cpu_hz = cpu
        lib.mad_play_movie(ctypes.byref(hw.player), ctypes.byref(mh), name)
    else:
        path = b'D:\\FEDATA\\MOVIES\\TEST.MAD'
        gw.g.write(DATA + 0x500, path + b'\0')
        gw.g.put_u32(DATA + 0x600, variant.get('skip', 0))
        quality = variant.get('quality', 3)
        gw.g.call(0x495bc0, [1], eax=DATA + 0x500, edx=DATA + 0x600, ebx=0, ecx=quality)
        flag = I32(variant.get('skip', 0))
        lib.mad_player_play(ctypes.byref(hw.player), path, ctypes.byref(flag), 0, quality, 1)

    problems = []
    if gs.log != hs.log:
        n = next((i for i in range(min(len(gs.log), len(hs.log))) if gs.log[i] != hs.log[i]),
                 min(len(gs.log), len(hs.log)))
        problems.append('call %d: guest %s / host %s (of %d / %d calls)' % (
            n, gs.log[n] if n < len(gs.log) else None, hs.log[n] if n < len(hs.log) else None,
            len(gs.log), len(hs.log)))
    gst, hst = gw.state(), hw.state()
    for k in gst:
        if gst[k] != hst[k]:
            problems.append('state.%s %s vs %s' % (k, gst[k], hst[k]))
    shown = sum(1 for e in gs.log if e[0] == 'display_frame')
    return problems, shown, len(gs.log), kinds


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--seed', type=int, default=1998)
    ap.add_argument('--movies', type=int, default=40)
    ap.add_argument('--lib', default=native.lib('libmadplay'))
    a = ap.parse_args()
    lib = load(a.lib)
    rng = random.Random(a.seed)
    variants = [
        ('software display, sound', {}, False),
        ('software display, stalls', {'stall': 1}, False),
        ('no sound (channel drained)', {'sound_start': -1}, False),
        ('overlay display: MADe decoded unless late', {'display_mode': 0}, False),
        ('key pressed mid-movie', {'key_at': 3}, False),
        ('quality 0 placement', {'quality': 0, 'display_mode': 0}, False),
        ('sub_4960e0, 2 GHz', {'cpu_hz': 2000000000}, True),
        ('sub_4960e0, 150 MHz', {'cpu_hz': 150000000}, True),
        ('sub_4960e0, 100 MHz (starts skipping)', {'cpu_hz': 100000000, 'display_mode': 0}, True),
        ('sub_4960e0, -nomovie', {'cpu_hz': 2000000000, 'movies_off': 1}, True),
        ('sub_4960e0, file missing', {'cpu_hz': 2000000000, 'file_exists': 0}, True),
    ]
    t0 = time.time()
    print('%-44s %6s %6s %8s %8s' % ('variant', 'movies', 'fails', 'shown', 'calls'))
    total_fail = 0
    for name, variant, wrap in variants:
        fails = shown = calls = 0
        n = a.movies if not variant.get('movies_off') and not variant.get('file_exists') == 0 else 3
        first = None
        for case in range(n):
            problems, s_, c_, kinds = run_case(lib, rng, case, dict(variant), wrap)
            shown += s_
            calls += c_
            if problems:
                fails += 1
                first = first or (case, problems, b''.join(k[-1:] for k in kinds))
        total_fail += fails
        print('%-44s %6d %6d %8d %8d' % (name, n, fails, shown, calls))
        if first:
            print('   first failure: case %d frames %s: %s' % (first[0], first[2], first[1][:4]))
    print('%.0f s' % (time.time() - t0))
    return 1 if total_fail else 0


if __name__ == '__main__':
    sys.exit(main())
