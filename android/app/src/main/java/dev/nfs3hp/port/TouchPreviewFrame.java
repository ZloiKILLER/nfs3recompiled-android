package dev.nfs3hp.port;

import android.app.Activity;
import android.graphics.Insets;
import android.graphics.Rect;
import android.view.Gravity;
import android.view.WindowInsets;
import android.view.WindowMetrics;
import android.widget.FrameLayout;

/** Preview and editor retain the playable screen proportions and control sizes. */
final class TouchPreviewFrame {
    static void attach(Activity activity,FrameLayout host,TouchControlsOverlay overlay) {
        WindowMetrics metrics=activity.getWindowManager().getCurrentWindowMetrics();
        Rect bounds=metrics.getBounds();Insets safe=metrics.getWindowInsets().getInsets(WindowInsets.Type.systemBars()|WindowInsets.Type.displayCutout());
        float width=bounds.width()-safe.left-safe.right,height=bounds.height()-safe.top-safe.bottom;
        overlay.setPreviewReference(width,height);
        host.addView(overlay,new FrameLayout.LayoutParams(1,1,Gravity.CENTER));
        host.addOnLayoutChangeListener((v,l,t,r,b,ol,ot,or,ob)->{
            float factor=Math.min(host.getWidth()/width,host.getHeight()/height);
            FrameLayout.LayoutParams params=(FrameLayout.LayoutParams)overlay.getLayoutParams();
            int w=Math.max(1,Math.round(width*factor)),h=Math.max(1,Math.round(height*factor));
            if(params.width!=w||params.height!=h){params.width=w;params.height=h;overlay.setLayoutParams(params);}
        });
    }
}
