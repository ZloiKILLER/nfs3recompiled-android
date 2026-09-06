package dev.nfs3hp.port;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.view.MotionEvent;
import android.view.ViewConfiguration;

/** A finger moves the mouse without holding a button. Only a short stationary tap clicks. */
final class TouchMouseInput {
    interface Sink { void mouse(int buttons,int action,float x,float y,boolean relative); }
    private final Sink sink;
    private final Handler handler=new Handler(Looper.getMainLooper());
    private final float slop;
    private int pointer=-1;
    private float startX,startY,lastX,lastY;
    private long started;
    private boolean tap,buttonDown;
    private final Runnable release=this::releaseButton;
    TouchMouseInput(Context context,Sink sink) {
        this.sink=sink;slop=ViewConfiguration.get(context).getScaledTouchSlop();
    }
    boolean onTouch(MotionEvent e) {
        int action=e.getActionMasked();
        if(action==MotionEvent.ACTION_DOWN) {
            releaseButton();pointer=e.getPointerId(0);startX=lastX=e.getX();startY=lastY=e.getY();
            started=e.getEventTime();tap=true;
        } else if(action==MotionEvent.ACTION_POINTER_DOWN) tap=false;
        else if(action==MotionEvent.ACTION_MOVE) {
            int i=e.findPointerIndex(pointer);if(i<0)return true;
            float x=e.getX(i),y=e.getY(i);
            // Check historical samples too: moving out and back is still a drag.
            for(int h=0;h<e.getHistorySize();h++)checkDistance(e.getHistoricalX(i,h),e.getHistoricalY(i,h));
            checkDistance(x,y);
            sink.mouse(0,MotionEvent.ACTION_MOVE,x-lastX,y-lastY,true);lastX=x;lastY=y;
        } else if(action==MotionEvent.ACTION_UP) {
            checkDistance(e.getX(),e.getY());
            if(pointer==e.getPointerId(0)&&tap&&e.getEventTime()-started<=250) {
                sink.mouse(MotionEvent.BUTTON_PRIMARY,MotionEvent.ACTION_DOWN,0,0,true);buttonDown=true;
                handler.postDelayed(release,70);
            }
            pointer=-1;tap=false;
        } else if(action==MotionEvent.ACTION_POINTER_UP) {
            tap=false;
            if(e.getPointerId(e.getActionIndex())==pointer) {
                int next=e.getActionIndex()==0?1:0;pointer=e.getPointerId(next);
                lastX=e.getX(next);lastY=e.getY(next);
            }
        } else if(action==MotionEvent.ACTION_CANCEL) cancel();
        return true;
    }
    private void checkDistance(float x,float y) {
        if((x-startX)*(x-startX)+(y-startY)*(y-startY)>slop*slop)tap=false;
    }
    private void releaseButton() {
        handler.removeCallbacks(release);
        if(buttonDown){sink.mouse(0,MotionEvent.ACTION_UP,0,0,true);buttonDown=false;}
    }
    void cancel(){pointer=-1;tap=false;releaseButton();}
}
