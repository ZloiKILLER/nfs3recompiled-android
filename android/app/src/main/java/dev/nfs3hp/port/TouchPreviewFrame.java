package dev.nfs3hp.port;

import android.app.Activity;
import android.graphics.Rect;
import android.os.Build;
import android.view.WindowInsets;
import android.view.WindowMetrics;
import android.widget.FrameLayout;

/** Preview and editor fill their cards while retaining real-device control sizes. */
final class TouchPreviewFrame {
    static void attach(Activity activity,FrameLayout host,TouchControlsOverlay overlay) {
        float[] window=windowArea(activity);
        float width=window[0],height=window[1];
        /* Once the game has run, the area its controls really had: this
         * window keeps the navigation bar and stays clear of the cutout, the
         * game's does neither, and a preview of another size shows controls
         * that fit where in the game they overlap. */
        android.content.SharedPreferences preferences=GamePreferences.get(activity);
        int gameWidth=preferences.getInt(GamePreferences.TOUCH_GAME_WIDTH,0),gameHeight=preferences.getInt(GamePreferences.TOUCH_GAME_HEIGHT,0);
        if(gameWidth>0&&gameHeight>0&&gameWidth>=gameHeight) { width=gameWidth;height=gameHeight; }
        overlay.setPreviewReference(width,height);
        // The whole card is the normalized touch canvas. This exposes the side space
        // to the layout editor instead of leaving untouchable letterbox pillars.
        host.addView(overlay,new FrameLayout.LayoutParams(
            FrameLayout.LayoutParams.MATCH_PARENT,FrameLayout.LayoutParams.MATCH_PARENT));
    }

    /** The area the launcher's own window leaves for content, in pixels, the
     *  long side first: what every preview laid controls out in before the
     *  game's own area was known. */
    static float[] windowArea(Activity activity) {
        float width,height;
        if(Build.VERSION.SDK_INT>=30) {
            WindowMetrics metrics=activity.getWindowManager().getCurrentWindowMetrics();
            Rect bounds=metrics.getBounds();Rect safe=AndroidWindowCompat.safeInsets(metrics.getWindowInsets());
            width=bounds.width()-safe.left-safe.right;height=bounds.height()-safe.top-safe.bottom;
        } else {
            android.util.DisplayMetrics metrics=activity.getResources().getDisplayMetrics();
            width=metrics.widthPixels;height=metrics.heightPixels;
        }
        // WindowMetrics can briefly expose the previous portrait bounds while
        // this landscape-only activity starts. Normalise them before deriving
        // the preview scale, otherwise 1080x2400 shrinks every control ~4x.
        if(height>width) { float portraitWidth=width;width=height;height=portraitWidth; }
        return new float[]{width,height};
    }
}
