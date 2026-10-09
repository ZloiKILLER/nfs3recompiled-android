/*
 * The MAD movie player of Need for Speed III, decompiled from nfs3.exe:
 * sub_4960e0 (play a movie by name), sub_495bc0 (the player) and its helpers.
 *
 * Everything the player calls outside itself -- EA's stream and sound
 * libraries, memory, the timer, the display -- goes through MadPlayerHost, one
 * function per original call, named after it.  The decoder (mad_decoder.c) is
 * called directly.
 *
 * Time is milliseconds (sub_4f2790 is GetTickCount).
 *
 * Chunks come from EA's stream library.  A video chunk:
 *   +0x00  tag 'MADk' key frame | 'MADm' predicted | 'MADe' predicted, not a
 *          reference (nothing is predicted from it; the player may drop it)
 *   +0x0c  frame duration, milliseconds in 16.16 (first chunk)
 *   +0x10  int16 width, +0x12 int16 height                (first chunk)
 *   +0x15  quantiser
 *   +0x18  bitstream (mad_begin_frame)
 * Audio chunks ('SC..', EA's SCHl/SCCl/SCDl/SCEl) go to a second channel
 * that EA's sound stream (SNDSTRM_*) plays.
 */
#ifndef MAD_PLAYER_H
#define MAD_PLAYER_H

#include <stdint.h>
#include "mad_decoder.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAD_TAG(a, b, c, d) ((uint32_t)(a) | ((uint32_t)(b) << 8) | ((uint32_t)(c) << 16) | ((uint32_t)(d) << 24))
#define MAD_KEY   MAD_TAG('M', 'A', 'D', 'k')
#define MAD_PRED  MAD_TAG('M', 'A', 'D', 'm')
#define MAD_EXTRA MAD_TAG('M', 'A', 'D', 'e')

typedef struct MadPlayerHost
{
    void *user;

    /* cmn\memstd.c */
    void *(*alloc)(void *user, uint32_t size);                       /* sub_4e1620 reservememadr */
    void (*release_memory)(void *user, void *block);                 /* sub_4e1890 purgememadr   */

    /* EA stream library */
    void *(*stream_create)(void *user, void *buffer, uint32_t size); /* sub_4cfa80(2, 2, 2, buffer, size) */
    void *(*stream_channel)(void *user, void *stream);               /* sub_4cfe70(stream, 2, 0xffff, 'SC') */
    void (*stream_setup)(void *user, void *stream);                  /* sub_4cfca0(stream, 1, 2)            */
    void *(*stream_open)(void *user, void *stream, const char *path);/* sub_4cfef0(stream, path, 0, 0)      */
    const uint8_t *(*stream_next)(void *user, void *channel);        /* sub_4d0300: next chunk or NULL      */
    void (*stream_release)(void *user, void *channel, const uint8_t *chunk); /* sub_4d03c0                  */
    int32_t (*stream_ended)(void *user, void *channel);              /* sub_4d04c0: 1 when nothing will come */
    int32_t (*stream_pending)(void *user, void *channel);            /* sub_4d0460: chunks waiting           */
    void (*stream_close)(void *user, void *stream);                  /* sub_4cfd30                          */

    /* EA sound stream */
    void (*sound_defaults)(void *user, void *params, int32_t count); /* sub_4e9b20 SNDplaysetdef(params, 0x1e) */
    uint32_t (*sound_memory)(void *user, int32_t streams);           /* sub_4f3a10(1)                       */
    void *(*sound_create)(void *user, void *channel, void *params, int32_t streams, int32_t count,
                          void *memory, uint32_t size);              /* sub_4f3a60(chan, params, 1, 0x1e, mem, size) */
    int32_t (*sound_start)(void *user, void *player, void *file);   /* sub_4f3a80(player, 0, file): handle or <0 */
    void (*sound_status)(void *user, int32_t handle, int32_t status[4]); /* sub_4f2f50 SNDSTRM_requeststatus:
                                                                        status[0] 3 = finished, status[1] ms played */
    void (*sound_destroy)(void *user, void *player);                 /* sub_4f43c0                          */

    /* time, input, display */
    uint32_t (*ticks)(void *user);                                   /* sub_4f2790 GetTickCount             */
    void (*pump)(void *user);                                        /* sub_4e7630(0)                       */
    int32_t (*display_mode)(void *user);                             /* sub_4df340: 2 = software (always, here) */
    int32_t (*key_pressed)(void *user);                              /* sub_451960(1)                       */
    void (*display_open)(void *user, int32_t x, int32_t y, int32_t w, int32_t h,
                         int32_t quality, int32_t software);         /* sub_4df1f0                          */
    void (*display_frame)(void *user, const uint8_t *frame);         /* sub_4df2b0                          */
    void (*display_close)(void *user);                               /* sub_4df310                          */
    void (*fatal)(void *user, const char *message);                  /* sub_401010 (does not return)        */

    int32_t screen_width, screen_height;                             /* [0x564384], [0x564388]              */
} MadPlayerHost;

typedef struct MadQueueEntry
{
    int32_t time;     /* when to show it, ms since the clock started */
    uint8_t *frame;   /* a w*h*2 buffer */
} MadQueueEntry;

/* The player's globals, 0x79f290..0x79f364 in the game, in this order. */
typedef struct MadPlayerState
{
    MadQueueEntry queue[16];   /* 0x79f290  [0, queued): waiting, by time; the rest: free buffers */
    int32_t frame_ms_q16;      /* 0x79f310  frame duration, 16.16 ms */
    int32_t dropped_catching_up; /* 0x79f314 frames skipped to reach a key frame */
    int32_t clock_drift;       /* 0x79f318  filtered (audio - clock), ms */
    int32_t height;            /* 0x79f31c */
    int32_t audio_seen;        /* 0x79f320  last audio position used */
    int32_t width;             /* 0x79f324 */
    uint32_t clock_start;      /* 0x79f328  tick count at time 0 */
    int32_t chunks_read;       /* 0x79f32c */
    int32_t stop;              /* 0x79f330  a key was pressed */
    int32_t extra_skipped;     /* 0x79f334  MADe frames not decoded */
    int32_t has_sound;         /* 0x79f338 */
    int32_t sound_handle;      /* 0x79f33c */
    int32_t frames_shown;      /* 0x79f340 */
    int32_t disp_h;            /* 0x79f344 */
    int32_t disp_y;            /* 0x79f348 */
    int32_t disp_x;            /* 0x79f34c */
    int32_t disp_w;            /* 0x79f350 */
    int32_t queued;            /* 0x79f354 */
    int32_t buffers;           /* 0x79f358 */
    int32_t total_late_ms;     /* 0x79f35c */
    int32_t frames_dropped;    /* 0x79f360  decoded but never shown */
} MadPlayerState;

typedef struct MadPlayer
{
    MadPlayerState s;
    MadDecoder dec;
    const MadPlayerHost *host;
} MadPlayer;

/* sub_495bc0: play the MAD file at `path`.  `skip_extra` points to a flag
 * that, once set, makes the player drop every MADe frame; it sets it itself
 * as soon as the display is in software mode.  `unused` is the original's
 * ebx (0), `quality` 1..3 picks the display mode, `software` 1 keeps the
 * display in software mode (the game always passes 1). */
void mad_player_play(MadPlayer *p, const char *path, int32_t *skip_extra, int32_t unused,
                     int32_t quality, int32_t software);

/* What sub_4960e0 needs besides the player. */
typedef struct MadMovieHost
{
    void *user;
    int32_t movies_off;             /* [0x7a1fdc], set by -nomovie */
    const char *movie_dir;          /* 0x7a32b4, "<cd>\fedata\movies\" from install.win */
    uint32_t cpu_hz;                /* [0x5652a4], measured at start-up */
    int32_t (*file_exists)(void *user, const char *path);   /* sub_4e0e60 */
    void (*flush_input)(void *user);                         /* sub_4f0240 */
    void (*sound_pause)(void *user);                         /* sub_4d5d30 */
    void (*sound_resume)(void *user);                        /* sub_4d5d70 */
    void (*music_stop)(void *user);                          /* sub_410aa0 */
    void (*music_start)(void *user);                         /* sub_410330 */
} MadMovieHost;

/* sub_4960e0: play fedata\movies\<name> (titleav.mad, demoav1..3.mad).
 * Always returns 1. */
int32_t mad_play_movie(MadPlayer *p, const MadMovieHost *m, const char *name);

/* Helpers, exported for the tests. */
void mad_queue_open(MadPlayer *p, int32_t x, int32_t y, int32_t w, int32_t h, int32_t buffers,
                    int32_t unused, int32_t quality, int32_t software);       /* sub_4957a0 */
void mad_queue_close(MadPlayer *p);                                           /* sub_495860 */
uint8_t *mad_queue_free_buffer(MadPlayer *p);                                 /* sub_495890 */
void mad_queue_put(MadPlayer *p, int32_t time);                               /* sub_4958c0 */
int32_t mad_queue_show(MadPlayer *p, int32_t now);                            /* sub_495920 */
void mad_clock_start(MadPlayer *p, int32_t sound_handle);                     /* sub_495a10 */
int32_t mad_clock_now(MadPlayer *p);                                          /* sub_495a50 */
void mad_check_key(MadPlayer *p);                                             /* sub_495b10 */
void mad_drain_channel(MadPlayer *p, void *channel);                          /* sub_495b30 */
const uint8_t *mad_next_video_chunk(MadPlayer *p, void *channel);             /* sub_495b60 */

#ifdef __cplusplus
}
#endif

#endif
