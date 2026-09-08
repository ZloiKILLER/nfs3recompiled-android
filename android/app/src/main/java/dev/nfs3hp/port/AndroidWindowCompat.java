package dev.nfs3hp.port;

import android.graphics.Rect;
import android.os.Build;
import android.view.WindowInsets;

/** Keeps the launcher usable on Android 10 while retaining cutout-aware insets on Android 11+. */
final class AndroidWindowCompat {
    private AndroidWindowCompat() {}

    static Rect safeInsets(WindowInsets insets) {
        if (insets == null) return new Rect();
        if (Build.VERSION.SDK_INT >= 30) {
            android.graphics.Insets safe = insets.getInsets(
                WindowInsets.Type.systemBars() | WindowInsets.Type.displayCutout());
            return new Rect(safe.left, safe.top, safe.right, safe.bottom);
        }
        return new Rect(insets.getSystemWindowInsetLeft(), insets.getSystemWindowInsetTop(),
            insets.getSystemWindowInsetRight(), insets.getSystemWindowInsetBottom());
    }
}
