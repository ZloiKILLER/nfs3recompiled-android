package dev.nfs3hp.port;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Typeface;
import android.util.AttributeSet;
import android.view.View;

/** Both title lines share one typeface and size; fit the longest line without wrapping. */
public final class GameTitleView extends View {
    private final Paint paint = new Paint(Paint.ANTI_ALIAS_FLAG);
    public GameTitleView(Context context, AttributeSet attrs) { super(context, attrs); }
    @Override protected void onDraw(Canvas canvas) {
        paint.setTypeface(Typeface.create("sans-serif-black", Typeface.BOLD));
        paint.setColor(0xffeef2f8);
        paint.setTextSize(100);
        float size = Math.min(getHeight() / 2.8f, 100 * getWidth() / paint.measureText("NEED FOR SPEED III"));
        paint.setTextSize(size);
        float top = (getHeight() - size * 2.35f) / 2;
        canvas.drawText("NEED FOR SPEED III", 0, top + size, paint);
        canvas.drawText("HOT PURSUIT", 0, top + size * 2.25f, paint);
    }
}
