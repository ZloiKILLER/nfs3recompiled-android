package dev.nfs3hp.port;

import android.os.Handler;
import android.os.Looper;
import android.view.HapticFeedbackConstants;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewConfiguration;

/** A finger moves the mouse without holding a button. A short stationary tap clicks;
 *  holding still grabs the button so the same finger can drag. */
final class TouchMouseInput {
    interface Sink { void mouse(int buttons,int action,float x,float y,boolean relative); }
    /** Stationary hold that grabs the button, just past the 250 ms tap window so
     *  a press either clicks or grabs, never both. */
    private static final long GRAB=275;
    private final Sink sink;
    private final View host;
    private final Handler handler=new Handler(Looper.getMainLooper());
    private final float slop;
    private int pointer=-1;
    private float startX,startY,lastX,lastY;
    private long started;
    private boolean tap,buttonDown,dragging;
    private final Runnable release=this::releaseButton;
    private final Runnable grab=this::grabButton;
    TouchMouseInput(View host,Sink sink) {
        this.host=host;this.sink=sink;slop=ViewConfiguration.get(host.getContext()).getScaledTouchSlop();
    }
    boolean onTouch(MotionEvent e) {
        int action=e.getActionMasked();
        if(action==MotionEvent.ACTION_DOWN) {
            cancel();pointer=e.getPointerId(0);startX=lastX=e.getX();startY=lastY=e.getY();
            started=e.getEventTime();tap=true;handler.postDelayed(grab,GRAB);
        } else if(action==MotionEvent.ACTION_POINTER_DOWN) {tap=false;handler.removeCallbacks(grab);}
        else if(action==MotionEvent.ACTION_MOVE) {
            int i=e.findPointerIndex(pointer);if(i<0)return true;
            float x=e.getX(i),y=e.getY(i);
            // Check historical samples too: moving out and back is still a drag.
            for(int h=0;h<e.getHistorySize();h++)checkDistance(e.getHistoricalX(i,h),e.getHistoricalY(i,h));
            checkDistance(x,y);
            sink.mouse(dragging?MotionEvent.BUTTON_PRIMARY:0,MotionEvent.ACTION_MOVE,x-lastX,y-lastY,true);
            lastX=x;lastY=y;
        } else if(action==MotionEvent.ACTION_UP) {
            checkDistance(e.getX(),e.getY());handler.removeCallbacks(grab);
            if(dragging)releaseButton();
            else if(pointer==e.getPointerId(0)&&tap&&e.getEventTime()-started<=250) {
                sink.mouse(MotionEvent.BUTTON_PRIMARY,MotionEvent.ACTION_DOWN,0,0,true);buttonDown=true;
                handler.postDelayed(release,70);
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
    /** Presses the button and keeps it down so the next movement drags. A silent grab
     *  would be indistinguishable from a resting finger, so the phone confirms it. */
    private void grabButton() {
        if(pointer==-1||buttonDown)return;
        tap=false;dragging=true;
        sink.mouse(MotionEvent.BUTTON_PRIMARY,MotionEvent.ACTION_DOWN,0,0,true);buttonDown=true;
        host.performHapticFeedback(HapticFeedbackConstants.LONG_PRESS);
    }
    private void releaseButton() {
        handler.removeCallbacks(release);dragging=false;
        if(buttonDown){sink.mouse(0,MotionEvent.ACTION_UP,0,0,true);buttonDown=false;}
    }
    void cancel(){pointer=-1;tap=false;handler.removeCallbacks(grab);releaseButton();}
}
