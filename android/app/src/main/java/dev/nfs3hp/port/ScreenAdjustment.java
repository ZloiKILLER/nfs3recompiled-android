package dev.nfs3hp.port;

import android.content.SharedPreferences;
import android.graphics.Bitmap;

/**
 * The picture adjustment the game applies to the finished frame, mirrored here
 * so the launcher's samples show exactly what a race will look like.
 *
 * Keep this in step with g_blitFragmentShader in src/lib/renderer.cpp -- same
 * order, same formula: gamma first, then contrast around mid grey, then
 * brightness as an offset. All three act on every channel identically, which is
 * why one 256-entry curve can stand in for the whole shader.
 *
 * Stored as percentages because a SeekBar deals in ints; 100 means "leave the
 * picture as the game drew it" for all three.
 */
final class ScreenAdjustment
{
    private ScreenAdjustment() {}

    static final int NEUTRAL = 100;
    static final int GAMMA_MIN = 100, GAMMA_MAX = 250;
    static final int BRIGHTNESS_MIN = 50, BRIGHTNESS_MAX = 150;
    static final int CONTRAST_MIN = 50, CONTRAST_MAX = 200;

    static int gammaPercent(SharedPreferences p) {
        return clamp(p.getInt(GamePreferences.GAMMA, NEUTRAL), GAMMA_MIN, GAMMA_MAX);
    }

    static int brightnessPercent(SharedPreferences p) {
        return clamp(p.getInt(GamePreferences.BRIGHTNESS, NEUTRAL), BRIGHTNESS_MIN, BRIGHTNESS_MAX);
    }

    static int contrastPercent(SharedPreferences p) {
        return clamp(p.getInt(GamePreferences.CONTRAST, NEUTRAL), CONTRAST_MIN, CONTRAST_MAX);
    }

    private static int clamp(int value, int low, int high) {
        return value < low ? low : value > high ? high : value;
    }

    /** Percent to what the shader's uniform is given. */
    static float gamma(int percent) { return percent / 100f; }
    static float contrast(int percent) { return percent / 100f; }

    /* Half the slider's travel either way is a quarter of the range added or
     * taken off, which is as far as an offset stays useful before the picture
     * turns into fog at one end or crushes to black at the other. */
    static float brightness(int percent) { return (percent - NEUTRAL) / 200f; }

    /**
     * The 256-entry transfer curve for one set of values. Building it costs a
     * few hundred pow() calls; applying it costs one array lookup per channel,
     * which is what makes a live preview affordable while a slider moves.
     */
    static int[] curve(int gammaPercent, int brightnessPercent, int contrastPercent)
    {
        final float inverse = 1f / Math.max(gamma(gammaPercent), 0.001f);
        final float contrast = contrast(contrastPercent);
        final float brightness = brightness(brightnessPercent);
        int[] lut = new int[256];
        for (int i = 0; i < 256; ++i)
        {
            double c = Math.pow(i / 255.0, inverse);
            c = (c - 0.5) * contrast + 0.5 + brightness;
            lut[i] = (int) Math.round(Math.max(0.0, Math.min(1.0, c)) * 255.0);
        }
        return lut;
    }

    /**
     * Writes `source` through `curve` into `target`. Both bitmaps must be the
     * same size; `scratch` must hold width*height ints and is reused between
     * calls so dragging a slider does not allocate a new buffer per frame.
     */
    static void apply(Bitmap source, Bitmap target, int[] curve, int[] scratch)
    {
        final int w = source.getWidth(), h = source.getHeight();
        source.getPixels(scratch, 0, w, 0, 0, w, h);
        for (int i = 0; i < scratch.length; ++i)
        {
            final int p = scratch[i];
            scratch[i] = (p & 0xff000000)
                | (curve[(p >> 16) & 0xff] << 16)
                | (curve[(p >> 8) & 0xff] << 8)
                | curve[p & 0xff];
        }
        target.setPixels(scratch, 0, w, 0, 0, w, h);
    }
}
