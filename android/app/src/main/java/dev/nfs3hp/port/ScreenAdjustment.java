package dev.nfs3hp.port;

import android.content.SharedPreferences;

/**
 * The picture adjustment the game applies to the finished frame, as kept in the
 * preferences: percentages, 100 meaning "leave the picture as the game drew it"
 * for all three.  The sliders are the game's own, on its graphics screens; this
 * is what the game is handed as it starts.
 *
 * Keep the ranges and the formulas in step with src/nfs3hp/port_display.cpp,
 * and what they mean with g_blitFragmentShader in src/lib/renderer.cpp.
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
}
