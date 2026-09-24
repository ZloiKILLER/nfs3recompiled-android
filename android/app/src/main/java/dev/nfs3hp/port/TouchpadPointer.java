package dev.nfs3hp.port;

import android.os.Handler;
import android.os.Looper;
import android.view.HapticFeedbackConstants;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewConfiguration;

/** The screen as a touchpad for the game's pointer -- the way touch worked it
 *  before 0.74, and the choice of players who like it better than the pointer
 *  going where the finger lands (TouchPointer), picked under Controls -> Touch:
 *
 *  - a finger sliding anywhere on the screen moves the cursor by as far as it
 *    slides, and presses nothing;
 *  - a short tap in place clicks;
 *  - resting in place for GRAB_MS grabs the button, with a short buzz, so the
 *    same finger drags until it lifts.
 *
 *  A second finger takes over the movement when the first one lifts.  What it
 *  sends is how far, not where: nfs3hp_main.cpp moves the game's cursor that many
 *  of its pixels. */
final class TouchpadPointer {
    interface Sink { void touchpad(float dx,float dy,int buttons); }
    /** A stationary hold that grabs the button, just past the tap window, so a
     *  press either clicks or grabs, never both. */
    private static final long TAP_MS=250,GRAB_MS=275,CLICK_MS=70;
    private final Sink sink;
    private final View host;
    private final Handler handler=new Handler(Looper.getMainLooper());
    private final float slop;
    private int pointer=-1;
    private float startX,startY,lastX,lastY;
    private long started;
    private boolean tap,buttonDown,dragging;
    private final Runnable release=this::releaseButton;
    private final Runnable grab=this::hold;
    TouchpadPointer(View host,Sink sink) {
        this.host=host;this.sink=sink;slop=ViewConfiguration.get(host.getContext()).getScaledTouchSlop();
    }
    boolean onTouch(MotionEvent e) {
        int action=e.getActionMasked();
        if(action==MotionEvent.ACTION_DOWN) {
            cancel();pointer=e.getPointerId(0);startX=lastX=e.getX();startY=lastY=e.getY();
            started=e.getEventTime();tap=true;handler.postDelayed(grab,GRAB_MS);
        } else if(action==MotionEvent.ACTION_POINTER_DOWN) {tap=false;handler.removeCallbacks(grab);}
        else if(action==MotionEvent.ACTION_MOVE) {
            int i=e.findPointerIndex(pointer);if(i<0)return true;
            float x=e.getX(i),y=e.getY(i);
            // Historical samples count too: out and back again is still a slide.
            for(int h=0;h<e.getHistorySize();h++)checkDistance(e.getHistoricalX(i,h),e.getHistoricalY(i,h));
            checkDistance(x,y);
            sink.touchpad(x-lastX,y-lastY,dragging?1:0);
            lastX=x;lastY=y;
        } else if(action==MotionEvent.ACTION_UP) {
            checkDistance(e.getX(),e.getY());handler.removeCallbacks(grab);
            if(dragging)releaseButton();
            else if(pointer==e.getPointerId(0)&&tap&&e.getEventTime()-started<=TAP_MS) {
                sink.touchpad(0,0,1);buttonDown=true;
                handler.postDelayed(release,CLICK_MS);
            }
            pointer=-1;tap=false;
        } else if(action==MotionEvent.ACTION_POINTER_UP) {
            tap=false;handler.removeCallbacks(grab);
            if(e.getPointerId(e.getActionIndex())==pointer) {
                int next=e.getActionIndex()==0?1:0;pointer=e.getPointerId(next);
                lastX=e.getX(next);lastY=e.getY(next);
            }
        } else if(action==MotionEvent.ACTION_CANCEL) cancel();
        return true;
    }
    private void checkDistance(float x,float y) {
        if((x-startX)*(x-startX)+(y-startY)*(y-startY)<=slop*slop)return;
        tap=false;
        // Moving before the hold completes means the finger is aiming, not grabbing.
        if(!dragging)handler.removeCallbacks(grab);
    }
    /** The finger has rested GRAB_MS: the button goes down and stays down, so
     *  the next movement drags.  A silent grab would look like a resting finger,
     *  so the phone confirms it.  The on-device checks call it rather than wait. */
    void hold() {
        if(pointer==-1||buttonDown)return;
        tap=false;dragging=true;
        sink.touchpad(0,0,1);buttonDown=true;
        host.performHapticFeedback(HapticFeedbackConstants.LONG_PRESS);
    }
    private void releaseButton() {
        handler.removeCallbacks(release);dragging=false;
        if(buttonDown){sink.touchpad(0,0,0);buttonDown=false;}
    }
    /** Lets go: the gesture was taken away, or the window lost focus. */
    void cancel(){pointer=-1;tap=false;handler.removeCallbacks(grab);releaseButton();}
}
