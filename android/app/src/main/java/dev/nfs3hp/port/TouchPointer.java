package dev.nfs3hp.port;

import android.view.HapticFeedbackConstants;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewConfiguration;

/** A finger on the game's picture, as the game's own pointer.  Where the finger
 *  is, the pointer goes -- the game is told the difference from wherever it
 *  keeps its cursor (nfs3hp_main.cpp) -- but only a tap clicks:
 *
 *  - a finger that lands brings the cursor under it, which lights up whatever it
 *    is over, and presses nothing;
 *  - lifted again close to where it landed, before HOLD_MS, it clicks there;
 *  - resting that long, it presses, with a short buzz, and keeps the button down
 *    until it lifts, so a slider can be dragged and a list's arrow held to
 *    repeat;
 *  - moved further than a tap wanders before that, it only points, and lifting
 *    it presses nothing.  A swipe across the car list, or the system's back
 *    gesture starting at the screen's edge, used to end in a click wherever the
 *    finger left the glass.
 *
 *  The first finger owns the pointer until it lifts; the rest are ignored, so a
 *  second finger cannot take over what the first one is holding. */
final class TouchPointer {
    interface Sink { void touch(int action,float x,float y); }
    /** The same three the native side counts on: the button goes down, the
     *  cursor goes to a place, the button comes up. */
    static final int DOWN=0, MOVE=1, UP=2;
    /** How long a finger rests before it presses and holds. */
    static final long HOLD_MS=250;
    private final View view;
    private final Sink sink;
    private final float slop;
    private final Runnable holdTimer=this::hold;
    private int pointer=-1;
    private float downX,downY,lastX,lastY;
    private boolean pressed,wandered;
    TouchPointer(View view,Sink sink) {
        this.view=view;this.sink=sink;
        slop=ViewConfiguration.get(view.getContext()).getScaledTouchSlop();
    }
    boolean onTouch(MotionEvent e) {
        switch(e.getActionMasked()) {
        case MotionEvent.ACTION_DOWN:
            cancel();
            pointer=e.getPointerId(0);downX=e.getX();downY=e.getY();pressed=wandered=false;
            send(MOVE,downX,downY);
            view.postDelayed(holdTimer,HOLD_MS);
            break;
        case MotionEvent.ACTION_MOVE: {
            if(pointer<0)break;
            int i=e.findPointerIndex(pointer);
            if(i<0)break;
            float x=e.getX(i),y=e.getY(i);
            if(!pressed&&!wandered&&Math.hypot(x-downX,y-downY)>slop) { wandered=true;view.removeCallbacks(holdTimer); }
            send(MOVE,x,y);
            break;
        }
        case MotionEvent.ACTION_POINTER_UP:
            if(e.getPointerId(e.getActionIndex())==pointer)lift(e.getX(e.getActionIndex()),e.getY(e.getActionIndex()));
            break;
        case MotionEvent.ACTION_UP:
            if(pointer>=0)lift(e.getX(),e.getY());
            break;
        case MotionEvent.ACTION_CANCEL:
            cancel();break;
        default:
            break;
        }
        return true;
    }
    /** Lets go without clicking: the gesture was taken away (the system's back
     *  gesture, another view, the window losing focus, the game pausing).  A
     *  button already held comes up where the finger was last seen, or it would
     *  stay down for good. */
    void cancel() {
        view.removeCallbacks(holdTimer);
        if(pointer<0)return;
        pointer=-1;
        if(pressed){pressed=false;sink.touch(UP,lastX,lastY);}
    }
    private void lift(float x,float y) {
        view.removeCallbacks(holdTimer);
        pointer=-1;
        if(pressed) { pressed=false;send(UP,x,y); }
        else if(!wandered) { send(DOWN,x,y);send(UP,x,y); }
        else send(MOVE,x,y);
    }
    /** The finger has rested HOLD_MS: it presses, unless it has lifted or
     *  wandered since.  The on-device checks call it rather than wait. */
    void hold() {
        if(pointer<0||pressed||wandered)return;
        pressed=true;
        view.performHapticFeedback(HapticFeedbackConstants.LONG_PRESS);
        send(DOWN,lastX,lastY);
    }
    private void send(int action,float x,float y) { lastX=x;lastY=y;sink.touch(action,x,y); }
}
