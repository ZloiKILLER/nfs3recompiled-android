/*
 * MAD movie player of Need for Speed III, decompiled from nfs3.exe.  See
 * mad_player.h.  Divisions by two are Watcom's signed ones (round toward
 * zero), as the original computes them.
 */
#include "mad_player.h"

#include <stdio.h>
#include <string.h>

static uint32_t rd32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static int32_t rd16s(const uint8_t *p)
{
    return (int16_t)((uint32_t)p[0] | ((uint32_t)p[1] << 8));
}

/* ------------------------------------------------------------------ */
/* frame queue                                                         */
/* ------------------------------------------------------------------ */

/* sub_4957a0, EA's initrender: `buffers` frame buffers (at most 16) and the display. */
void mad_queue_open(MadPlayer *p, int32_t x, int32_t y, int32_t w, int32_t h, int32_t buffers,
                    int32_t unused, int32_t quality, int32_t software)
{
    MadPlayerState *s = &p->s;
    const MadPlayerHost *h_ = p->host;
    (void)unused;
    if (buffers > 16)
        h_->fatal(h_->user, "initrender - too many frame buffers specified\n"); /* 0x53be58 */
    s->disp_y = y;
    s->disp_w = w;
    s->disp_h = h;
    s->buffers = buffers;
    s->disp_x = x;
    s->queued = 0;
    s->frames_shown = 0;
    s->frames_dropped = 0;
    s->total_late_ms = 0;
    for (int32_t i = 0; i < s->buffers; i++)
        s->queue[i].frame = (uint8_t *)h_->alloc(h_->user, (uint32_t)(s->disp_w * 2 * s->disp_h));
    h_->display_open(h_->user, s->disp_x, s->disp_y, s->disp_w, s->disp_h, quality, software);
}

/* sub_495860 */
void mad_queue_close(MadPlayer *p)
{
    for (int32_t i = 0; i < p->s.buffers; i++)
        p->host->release_memory(p->host->user, p->s.queue[i].frame);
    p->host->display_close(p->host->user);
}

/* sub_495890: the next free buffer, NULL when all are queued. */
uint8_t *mad_queue_free_buffer(MadPlayer *p)
{
    if (p->s.queued == p->s.buffers)
        return NULL;
    return p->s.queue[p->s.queued].frame;
}

/* sub_4958c0: queue the buffer mad_queue_free_buffer gave, to be shown at
 * `time`; the waiting entries stay sorted (a later one with the same time
 * goes in front of the earlier). */
void mad_queue_put(MadPlayer *p, int32_t time)
{
    MadPlayerState *s = &p->s;
    int32_t i = s->queued;
    uint8_t *frame = s->queue[i].frame;
    while (i > 0 && time <= s->queue[i - 1].time)
    {
        s->queue[i] = s->queue[i - 1];
        i--;
    }
    s->queue[i].time = time;
    s->queued++;
    s->queue[i].frame = frame;
}

/* sub_495920: show the latest frame due at `now`; those due before it are
 * dropped.  Returns 1 if a frame was shown. */
int32_t mad_queue_show(MadPlayer *p, int32_t now)
{
    MadPlayerState *s = &p->s;
    if (s->queued == 0 || now < s->queue[0].time)
        return 0;
    int32_t n = 1;
    while (n < s->queued && now >= s->queue[n].time)
        n++;
    p->host->display_frame(p->host->user, s->queue[n - 1].frame);
    s->frames_shown++;
    s->total_late_ms += now - s->queue[n - 1].time;
    s->frames_dropped += n - 1;
    /* Move the rest down; the n buffers now free go to the end. */
    for (int32_t j = 0; j < s->buffers - n; j++)
    {
        uint8_t *spare = s->queue[j].frame;
        s->queue[j] = s->queue[j + n];
        s->queue[j + n].frame = spare;
    }
    s->queued -= n;
    return 1;
}

/* ------------------------------------------------------------------ */
/* clock                                                               */
/* ------------------------------------------------------------------ */

/* Watcom's x / 8, toward zero (sar edx,31 / shl edx,3 / sbb / sar 3). */
static int32_t div8(int32_t v)
{
    return v / 8;
}

/* sub_495a10 */
void mad_clock_start(MadPlayer *p, int32_t sound_handle)
{
    MadPlayerState *s = &p->s;
    s->sound_handle = sound_handle;
    s->has_sound = sound_handle >= 0 ? 1 : 0;
    s->clock_start = p->host->ticks(p->host->user);
    s->audio_seen = 0;
    s->clock_drift = 0;
}

/* sub_495a50: milliseconds since the clock started, pulled towards the sound:
 * the drift between the sound's position and the clock is filtered (7/8 of
 * the old plus the new), and once it is more than 264 ms the clock jumps by
 * an eighth of it. */
int32_t mad_clock_now(MadPlayer *p)
{
    MadPlayerState *s = &p->s;
    const uint32_t now = p->host->ticks(p->host->user);
    int32_t elapsed = (int32_t)(now - s->clock_start);
    if (s->has_sound)
    {
        int32_t status[4];
        p->host->sound_status(p->host->user, s->sound_handle, status);
        const int32_t played = status[1];
        if (played > s->audio_seen)
        {
            s->audio_seen = played;
            int32_t drift = (int32_t)((uint32_t)s->clock_drift - (uint32_t)div8(s->clock_drift)
                                      + (uint32_t)(played - elapsed));
            const int32_t size = drift > 0 ? drift : -drift;
            if (size > 0x108)
            {
                s->clock_start -= (uint32_t)div8(drift);
                elapsed = (int32_t)(now - s->clock_start);
                drift = 0;
            }
            s->clock_drift = drift;
        }
    }
    return elapsed;
}

/* ------------------------------------------------------------------ */
/* stream helpers                                                      */
/* ------------------------------------------------------------------ */

/* sub_495b10: any key stops the movie. */
void mad_check_key(MadPlayer *p)
{
    if (p->host->key_pressed(p->host->user))
        p->s.stop = 1;
}

/* sub_495b30: throw away what is waiting on a channel (the sound's, when
 * there is no sound to play it). */
void mad_drain_channel(MadPlayer *p, void *channel)
{
    const uint8_t *chunk;
    while ((chunk = p->host->stream_next(p->host->user, channel)) != NULL)
        p->host->stream_release(p->host->user, channel, chunk);
}

/* sub_495b60: the next MADk/MADm/MADe chunk, others thrown away; NULL at the
 * end.  Waits (polls) while the stream has not delivered. */
const uint8_t *mad_next_video_chunk(MadPlayer *p, void *channel)
{
    const MadPlayerHost *h = p->host;
    for (;;)
    {
        const uint8_t *chunk = h->stream_next(h->user, channel);
        if (chunk)
        {
            const uint32_t tag = rd32(chunk);
            if (tag == MAD_PRED || tag == MAD_EXTRA || tag == MAD_KEY)
            {
                p->s.chunks_read++;
                return chunk;
            }
            h->stream_release(h->user, channel, chunk);
        }
        if (h->stream_ended(h->user, channel))
            return NULL;
    }
}

/* ------------------------------------------------------------------ */
/* sub_495bc0, EA's showmad: the player                              */
/* ------------------------------------------------------------------ */

/* One frame, macroblock by macroblock in raster order.  Between rows, the
 * main loop shows a frame that has come due (`show_between`). */
static void decode_frame(MadPlayer *p, const uint8_t *chunk, uint8_t *frame, uint8_t *ref, int32_t show_between)
{
    MadPlayerState *s = &p->s;
    const uint32_t tag = rd32(chunk);
    mad_begin_frame(&p->dec, chunk + 0x18, tag == MAD_KEY ? 0 : 1, chunk[0x15]);
    int32_t shown = 0;
    for (int32_t y = 0; y < s->height; y += 16)
    {
        for (int32_t x = 0; x < s->width; x += 16)
        {
            const int32_t off = ((y * s->width + x) / 2) * 4;
            /* The original reads the reference through a pointer it has not
             * set before the first frame; a stream that starts with a
             * predicted frame would read garbage.  Here: the frame itself. */
            const uint8_t *r = ref ? ref + off : frame + off;
            mad_decode_macroblock(&p->dec, r, frame + off, s->width);
        }
        if (show_between && !shown)
            shown = mad_queue_show(p, mad_clock_now(p));
    }
}

static void advance(MadPlayerState *s, uint32_t *fraction, int32_t *time)
{
    *fraction += (uint32_t)s->frame_ms_q16;
    *time += (int32_t)(*fraction >> 16);  /* sar on a value below 2^31 */
    *fraction &= 0xffff;
}

void mad_player_play(MadPlayer *p, const char *path, int32_t *skip_extra, int32_t unused,
                     int32_t quality, int32_t software)
{
    MadPlayerState *s = &p->s;
    const MadPlayerHost *h = p->host;

    void *stream_buffer = h->alloc(h->user, 0x100000);
    void *stream = h->stream_create(h->user, stream_buffer, 0x100000);
    void *sound_channel = h->stream_channel(h->user, stream);
    h->stream_setup(h->user, stream);
    void *file = h->stream_open(h->user, stream, path);

    const uint8_t *chunk = mad_next_video_chunk(p, stream);
    if (!chunk)
        h->fatal(h->user, "showmad - no MAD chunks found\n"); /* 0x53be8c */

    /* The first chunk's header. */
    s->frame_ms_q16 = (int32_t)rd32(chunk + 0x0c);
    s->width = rd16s(chunk + 0x10);
    s->chunks_read = 1;
    s->height = rd16s(chunk + 0x12);
    s->dropped_catching_up = 0;
    s->extra_skipped = 0;

    /* Centred; at 2x (quality != 0) the picture is twice the size. */
    int32_t x, y;
    if (quality == 0)
    {
        x = (h->screen_width - s->width) / 2;
        y = (h->screen_height - s->height) / 2;
    }
    else
    {
        x = h->screen_width / 2 - s->width;
        y = h->screen_height / 2 - s->height;
    }
    mad_queue_open(p, x, y, s->width, s->height, 6, unused, quality, software);

    /* Fill the queue before the sound starts: every frame decoded. */
    uint8_t *frame = mad_queue_free_buffer(p);
    uint8_t *ref = NULL;
    uint32_t fraction = 0;
    int32_t time = 0;
    while (chunk && frame)
    {
        decode_frame(p, chunk, frame, ref, 0);
        if (rd32(chunk) != MAD_EXTRA)
            ref = frame;
        h->stream_release(h->user, stream, chunk);
        mad_queue_put(p, time);
        advance(s, &fraction, &time);
        frame = mad_queue_free_buffer(p);
        chunk = mad_next_video_chunk(p, stream);
    }

    /* The sound. */
    uint8_t params[0x20];  /* SNDplaysetdef's block, 0x20 bytes on the original's stack; 0x1e is passed with it */
    h->sound_defaults(h->user, params, 0x1e);
    const uint32_t sound_size = h->sound_memory(h->user, 1);
    void *sound_memory = h->alloc(h->user, sound_size);
    void *sound = h->sound_create(h->user, sound_channel, params, 1, 0x1e, sound_memory, sound_size);
    int32_t sound_handle = -1;
    if (h->stream_pending(h->user, sound_channel))
        sound_handle = h->sound_start(h->user, sound, file);
    mad_clock_start(p, sound_handle);
    s->stop = 0;

    while (chunk && !s->stop)
    {
        h->pump(h->user);
        if (h->display_mode(h->user) == 2)
            *skip_extra = 1;  /* software display: MADe frames are never decoded */
        int32_t now = mad_clock_now(p);

        /* A MADe frame is dropped when asked to, or when it is due in less
         * than 133 ms. */
        if (rd32(chunk) == MAD_EXTRA && (*skip_extra != 0 || time - now < 0x85))
        {
            s->extra_skipped++;
            advance(s, &fraction, &time);
            h->stream_release(h->user, stream, chunk);
            chunk = mad_next_video_chunk(p, stream);
            if (!chunk)
                break;
        }

        /* Less than 67 ms ahead: skip to the next key frame. */
        if (time - now < 0x43)
        {
            do
            {
                if (rd32(chunk) != MAD_EXTRA)
                    s->dropped_catching_up++;
                else
                    s->extra_skipped++;
                advance(s, &fraction, &time);
                h->stream_release(h->user, stream, chunk);
                chunk = mad_next_video_chunk(p, stream);
            } while (chunk && rd32(chunk) != MAD_KEY);
            if (!chunk)
                break;
        }

        /* A free buffer, showing frames until one is. */
        while ((frame = mad_queue_free_buffer(p)) == NULL)
            mad_queue_show(p, mad_clock_now(p));

        decode_frame(p, chunk, frame, ref, 1);
        if (rd32(chunk) != MAD_EXTRA)
            ref = frame;
        h->stream_release(h->user, stream, chunk);
        mad_queue_put(p, time);
        if (sound_handle < 0)
            mad_drain_channel(p, sound_channel);
        advance(s, &fraction, &time);
        chunk = mad_next_video_chunk(p, stream);
        mad_check_key(p);
    }

    /* Show what is left, then wait for the sound to finish. */
    for (;;)
    {
        const int32_t now = mad_clock_now(p);
        if (now >= time)
            break;
        mad_queue_show(p, now);
        h->pump(h->user);
    }
    if (sound_handle >= 0)
    {
        for (;;)
        {
            int32_t status[4];
            h->sound_status(h->user, sound_handle, status);
            h->pump(h->user);
            mad_check_key(p);
            if (status[0] == 3 || s->stop)
                break;
        }
    }
    h->sound_destroy(h->user, sound);
    h->stream_close(h->user, stream);
    h->release_memory(h->user, stream_buffer);
    h->release_memory(h->user, sound_memory);
    mad_queue_close(p);
}

/* ------------------------------------------------------------------ */
/* sub_4960e0                                                          */
/* ------------------------------------------------------------------ */

int32_t mad_play_movie(MadPlayer *p, const MadMovieHost *m, const char *name)
{
    if (m->movies_off)
        return 1;
    char path[0x7e];
    snprintf(path, sizeof path, "%s%s", m->movie_dir, name);  /* inline strcpy + strcat in the original */
    if (!m->file_exists(m->user, path))
        return 1;
    m->flush_input(m->user);
    m->sound_pause(m->user);
    m->music_stop(m->user);

    /* The display mode by the speed measured at start-up. */
    int32_t quality;
    if (m->cpu_hz < 133000000u)
        quality = 1;
    else if (m->cpu_hz < 200000000u)
        quality = 2;
    else
        quality = 3;
    /* edx still holds the -nomovie flag here, i.e. 0; a slow machine starts
     * out dropping MADe frames. */
    int32_t skip_extra = 0;
    if (m->cpu_hz < 133000000u)
        skip_extra = 1;

    mad_player_play(p, path, &skip_extra, 0, quality, 1);

    m->sound_resume(m->user);
    m->flush_input(m->user);
    m->music_start(m->user);
    return 1;
}
