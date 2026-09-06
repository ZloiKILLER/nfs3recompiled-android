package dev.nfs3hp.port;

import android.content.Context;
import android.content.SharedPreferences;
import android.graphics.*;
import android.os.Handler;
import android.os.Looper;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowInsets;
import org.libsdl.app.SDLActivity;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

/** Digital controls: movement changes held keys; touches starting outside controls pass to SDL. */
final class TouchControlsOverlay extends View {
    private final SharedPreferences preferences;
    private final Handler handler = new Handler(Looper.getMainLooper());
    private final Paint paint = new Paint(Paint.ANTI_ALIAS_FLAG);
    private final ArrayList<Control> controls = new ArrayList<>();
    private final Map<Integer,Contact> contacts = new HashMap<>();
    private final TouchKeyState keys;
    private final boolean preview;
    private final Runnable dim = this::fadeIdle;
    private float scale=1, knobX, knobY;
    private float referenceWidth,referenceHeight;
    private boolean menuMode;
    private int screenFingers;
    private final java.util.HashSet<Integer> pointers=new java.util.HashSet<>();
    private boolean editing;
    private Control selected;
    private float dragX,dragY;
    private int editorPointer=-1;
    private java.util.function.Consumer<String> selectionChanged;
    private static final class Control {
        final String action,label; final RectF box; final boolean pulse;
        final RectF hitBox=new RectF(); float visualSize=1,hitSize=1;
        float baseWidth,baseHeight;
        Control(String a,String l,RectF b,boolean p) { action=a;label=l;box=b;pulse=p;baseWidth=b.width();baseHeight=b.height(); }
    }
    private static final class Contact {
        final Control control; final ArrayList<Integer> held=new ArrayList<>(); boolean inside=true;
        Contact(Control c) { control=c; }
    }
    TouchControlsOverlay(Context c) { this(c,false); }
    TouchControlsOverlay(Context c,boolean preview) {
        this(c,preview,null);
    }
    TouchControlsOverlay(Context c,boolean preview,TouchKeyState.Sink output) {
        super(c);this.preview=preview;preferences=GamePreferences.get(c);
        menuMode=!preview; // The game starts in its menus; preview opens on the racing layout.
        keys=new TouchKeyState(output!=null?output:(key,down)->{
            if(!preview) { if(down) SDLActivity.onNativeKeyDown(key);else SDLActivity.onNativeKeyUp(key); }
        });
        setFocusable(false);setContentDescription(c.getString(R.string.touch_preview_description));
        setOnApplyWindowInsetsListener((v,insets)->{
            if(!preview) {
                Insets safe=insets.getInsets(WindowInsets.Type.systemBars()|WindowInsets.Type.displayCutout());
                setPadding(safe.left,safe.top,safe.right,safe.bottom);rebuild();
            }
            return insets;
        });
    }
    void refreshSettings() { rebuild();resumeIdleTimer(); }
    void setPreviewReference(float w,float h) { referenceWidth=w;referenceHeight=h;rebuild(); }
    void onScreenTouch(MotionEvent e) {
        int action=e.getActionMasked();
        screenFingers=(action==MotionEvent.ACTION_UP||action==MotionEvent.ACTION_CANCEL)?0:
            e.getPointerCount()-(action==MotionEvent.ACTION_POINTER_UP?1:0);
        if(screenFingers>0)reveal();else scheduleHide();
    }
    void setMenuMode(boolean menu) { menuMode=menu;rebuild(); }
    boolean isMenuMode() { return menuMode; }
    void setEditing(boolean value,java.util.function.Consumer<String> listener) {
        editing=value;selectionChanged=listener;selected=null;releaseAll();invalidate();
    }
    String selectedAction() { return selected==null?null:selected.action; }
    String selectedLabel() { return selected==null?"Select a control":selected.label; }
    int selectedVisualSize() { return selected==null?100:Math.round(selected.visualSize*100); }
    int selectedHitSize() { return selected==null?100:Math.round(selected.hitSize*100); }
    private String layoutPrefix() {
        return "touch_position_"+(preferences.getBoolean(GamePreferences.TOUCH_SEPARATE,false)?(menuMode?"menu_":"race_"):"shared_");
    }
    void resetLayout() {
        SharedPreferences.Editor edit=preferences.edit();
        for(String k:preferences.getAll().keySet()) if(k.startsWith(layoutPrefix())) edit.remove(k);
        edit.apply();selected=null;rebuild();if(selectionChanged!=null)selectionChanged.accept("Select a control");
    }
    void resizeSelected(int visual,int hit) {
        if(selected==null)return;
        selected.visualSize=visual/100f;selected.hitSize=hit/100f;
        float x=selected.box.centerX(),y=selected.box.centerY();
        float w=selected.baseWidth*selected.visualSize,h=selected.baseHeight*selected.visualSize;
        selected.box.set(x-w/2,y-h/2,x+w/2,y+h/2);
        constrain(selected);savePosition(selected);invalidate();
    }
    private float areaWidth() { return (getWidth()-getPaddingLeft()-getPaddingRight())/scale; }
    private float areaHeight() { return (getHeight()-getPaddingTop()-getPaddingBottom())/scale; }
    private void constrain(Control c) {
        float w=Math.min(c.box.width(),areaWidth()),h=Math.min(c.box.height(),areaHeight());
        float x=Math.max(w/2,Math.min(areaWidth()-w/2,c.box.centerX()));
        float y=Math.max(h/2,Math.min(areaHeight()-h/2,c.box.centerY()));
        c.box.set(x-w/2,y-h/2,x+w/2,y+h/2);
        // Hit size is independent of the visible size, relative to the default control.
        float hw=c.baseWidth*c.hitSize,hh=c.baseHeight*c.hitSize;
        c.hitBox.set(Math.max(0,x-hw/2),Math.max(0,y-hh/2),Math.min(areaWidth(),x+hw/2),Math.min(areaHeight(),y+hh/2));
    }
    private void savePosition(Control c) {
        String k=layoutPrefix()+c.action;
        preferences.edit().putFloat(k+"_x",c.box.centerX()/areaWidth()).putFloat(k+"_y",c.box.centerY()/areaHeight())
            .putFloat(k+"_size",c.visualSize).putFloat(k+"_hit",c.hitSize).apply();
    }
    @Override protected void onSizeChanged(int w,int h,int ow,int oh) { rebuild(); }
    private void add(String a,int label,float x,float y,float w,float h,boolean pulse) {
        controls.add(new Control(a,getContext().getString(label),new RectF(x,y,x+w,y+h),pulse));
    }
    private void rebuild() {
        String selection=selectedAction();releaseAll();controls.clear();selected=null;
        float width=getWidth()-getPaddingLeft()-getPaddingRight(),height=getHeight()-getPaddingTop()-getPaddingBottom();
        if(width<=0||height<=0) return;
        scale=Math.min(getResources().getDisplayMetrics().density,Math.min(width/640f,height/340f));
        float pixelScale=1;
        if(preview&&referenceWidth>0&&referenceHeight>0){
            pixelScale=Math.min(width/referenceWidth,height/referenceHeight);
            scale=Math.min(getResources().getDisplayMetrics().density,Math.min(referenceWidth/640f,referenceHeight/340f))*pixelScale;
        }
        float edge=preferences.getInt(GamePreferences.TOUCH_EDGE,0);
        float w=width/scale-2*edge,h=height/scale-preferences.getInt(GamePreferences.TOUCH_RAISE,0);
        float size=Math.min(preferences.getInt(GamePreferences.TOUCH_SIZE,100)/100f,1.15f);
        float pad=140*size,pedal=62*size;
        add("steering",menuMode?R.string.control_navigation:R.string.control_steering,20,h-20-pad,pad,pad,false);
        if(menuMode) {
            add("confirm",R.string.control_confirm,w-100*size,h-20-84*size,80*size,84*size,true);
            add("back",R.string.control_back,w-188*size,h-20-64*size,72*size,64*size,true);
        } else {
            add("accelerate",R.string.control_gas,w-20-pedal-5*pixelScale/scale,h-20-118*size,pedal,118*size,false);
            add("brake",R.string.control_brake,w-32-2*pedal,h-20-84*size,pedal,84*size,false);
            add("handbrake",R.string.control_handbrake,w-32-2*pedal,h-32-138*size,52*size,46*size,false);
            add("look_behind",R.string.control_look_back,w-86,h-244,50,48,false);
            add("camera",R.string.control_camera,w-146,h-244,50,48,true);
            add("horn",R.string.control_horn,24,h-244,48,48,false);
            if(preferences.getBoolean(GamePreferences.TOUCH_GEARS,false)) {
                add("gear_down",R.string.control_gear_down,w-208*size,h-20-52*size,48*size,52*size,true);
                add("gear_up",R.string.control_gear_up,w-208*size,h-84-52*size,48*size,52*size,true);
            }
        }
        add("pause",R.string.control_pause,20,18,48,48,true);
        add("mode",menuMode?R.string.control_race:R.string.control_menu,82,18,58,48,true);
        add("headlights",R.string.control_lights,w-136,18,48,48,true);
        add("recover",R.string.control_recover,w-74,18,54,48,true);
        if(GamePreferences.TOUCH_LAYOUT_MIRRORED.equals(preferences.getString(GamePreferences.TOUCH_LAYOUT,"standard")))
            for(Control c:controls) { float left=c.box.left;c.box.left=w-c.box.right;c.box.right=w-left; }
        for(Control c:controls) {
            c.box.offset(edge,0);
            String k=layoutPrefix()+c.action;
            float x=preferences.getFloat(k+"_x",c.box.centerX()/areaWidth())*areaWidth();
            float y=preferences.getFloat(k+"_y",c.box.centerY()/areaHeight())*areaHeight();
            c.visualSize=preferences.getFloat(k+"_size",1);c.hitSize=preferences.getFloat(k+"_hit",1);
            float cw=c.baseWidth*c.visualSize,ch=c.baseHeight*c.visualSize;
            c.box.set(x-cw/2,y-ch/2,x+cw/2,y+ch/2);constrain(c);
            if(c.action.equals(selection))selected=c;
        }
        invalidate();
    }
    @Override protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);canvas.save();canvas.translate(getPaddingLeft(),getPaddingTop());canvas.scale(scale,scale);
        if(preview) {
            canvas.drawColor(0xff0c1017);
            text(canvas,editing?"Drag controls · Select to resize":getContext().getString(R.string.touch_preview_hint),getWidth()/scale/2,100,12,0xff8491a4);
        }
        int opacity=preferences.getInt(GamePreferences.TOUCH_OPACITY,65);
        for(Control c:controls) {
            boolean down=false;
            for(Contact contact:contacts.values()) if(contact.control==c&&contact.inside) down=true;
            paint.setStyle(Paint.Style.FILL);paint.setColor(Color.argb(Math.round(opacity*2.1f),down?88:18,down?64:24,down?30:34));
            boolean stick=c.action.equals("steering");
            if(stick&&menuMode) canvas.drawPath(dpad(c.box),paint);
            else if(stick) canvas.drawOval(c.box,paint);else canvas.drawRoundRect(c.box,16,16,paint);
            paint.setStyle(Paint.Style.STROKE);paint.setStrokeWidth(down?2.2f:1.4f);
            paint.setColor(down?0xffffbf69:Color.argb(Math.round(opacity*2.55f),203,216,232));
            if(stick) {
                if(menuMode) canvas.drawPath(dpad(c.box),paint);else canvas.drawOval(c.box,paint);
                float x=c.box.centerX(),y=c.box.centerY(),r=c.box.width()/2;
                arrow(canvas,x-r*.68f,y,-1,0);arrow(canvas,x+r*.68f,y,1,0);
                if(menuMode) { arrow(canvas,x,y-r*.68f,0,-1);arrow(canvas,x,y+r*.68f,0,1); }
                paint.setStyle(Paint.Style.FILL);paint.setColor(down?0xffad8148:0xff3b4555);
                if(!menuMode) canvas.drawCircle(x+knobX*r*.38f,y+knobY*r*.38f,r*.35f,paint);
                else if(down) canvas.drawCircle(x+knobX*r*.65f,y+knobY*r*.65f,r*.12f,paint);
            } else {
                canvas.drawRoundRect(c.box,16,16,paint);
                if(c.action.equals("accelerate")||c.action.equals("brake"))
                    for(int i=0;i<4;i++) canvas.drawLine(c.box.left+13,c.box.top+15+i*9,c.box.right-13,c.box.top+15+i*9,paint);
                else icon(canvas,c.action,c.box.centerX(),c.box.top+18);
            }
            paint.setTextSize(9);
            float labelSize=Math.min(9,9*(c.box.width()-10)/Math.max(1,paint.measureText(c.label)));
            if(!stick||!menuMode) text(canvas,c.label,c.box.centerX(),c.box.bottom-9,labelSize,down?0xffffcf89:0xffeff3fa);
            if(editing) {
                paint.setStyle(Paint.Style.STROKE);paint.setStrokeWidth(c==selected?2:1);
                paint.setColor(c==selected?0xffffbf69:0xff668f9a);
                paint.setPathEffect(new DashPathEffect(new float[]{4,4},0));
                canvas.drawRect(c.hitBox,paint);paint.setPathEffect(null);
                if(c==selected)canvas.drawRoundRect(c.box,8,8,paint);
            }
        }
        canvas.restore();
    }
    private Path dpad(RectF b) {
        float x=b.centerX(),y=b.centerY(),t=b.width()*.17f;
        Path p=new Path();p.moveTo(x-t,b.top);p.lineTo(x+t,b.top);p.lineTo(x+t,y-t);
        p.lineTo(b.right,y-t);p.lineTo(b.right,y+t);p.lineTo(x+t,y+t);p.lineTo(x+t,b.bottom);
        p.lineTo(x-t,b.bottom);p.lineTo(x-t,y+t);p.lineTo(b.left,y+t);p.lineTo(b.left,y-t);
        p.lineTo(x-t,y-t);p.close();return p;
    }
    private void text(Canvas c,String s,float x,float y,float size,int color) {
        paint.setStyle(Paint.Style.FILL);paint.setColor(color);paint.setTextSize(size);
        paint.setTypeface(Typeface.create("sans-serif-medium",0));paint.setTextAlign(Paint.Align.CENTER);c.drawText(s,x,y,paint);
    }
    private void arrow(Canvas c,float x,float y,int dx,int dy) {
        c.drawLine(x-dx*4-dy*5,y-dy*4+dx*5,x+dx*3,y+dy*3,paint);
        c.drawLine(x-dx*4+dy*5,y-dy*4-dx*5,x+dx*3,y+dy*3,paint);
    }
    private void icon(Canvas c,String a,float x,float y) {
        paint.setStyle(Paint.Style.STROKE);paint.setStrokeWidth(2);paint.setStrokeCap(Paint.Cap.ROUND);
        switch(a) {
        case "gear_up": text(c,"+",x,y+7,24,0xffeff3fa);break;
        case "gear_down": text(c,"−",x,y+7,24,0xffeff3fa);break;
        case "back": arrow(c,x,y,-1,0);break;
        case "pause": c.drawLine(x-4,y-6,x-4,y+6,paint);c.drawLine(x+4,y-6,x+4,y+6,paint);break;
        case "headlights":
            c.drawArc(new RectF(x-10,y-7,x+3,y+7),90,180,false,paint);c.drawLine(x-4,y-7,x-4,y+7,paint);
            for(int i=-1;i<=1;i++) c.drawLine(x+1,y+i*5,x+10,y+i*5,paint);break;
        case "recover": case "look_behind":
            c.drawArc(new RectF(x-9,y-7,x+9,y+7),35,280,false,paint);arrow(c,x+7,y-5,0,1);break;
        case "camera":
            c.drawRoundRect(new RectF(x-10,y-6,x+10,y+7),3,3,paint);c.drawCircle(x,y,4,paint);c.drawLine(x-4,y-9,x+3,y-9,paint);break;
        case "confirm": c.drawLine(x-8,y,x-2,y+6,paint);c.drawLine(x-2,y+6,x+9,y-6,paint);break;
        case "mode": for(int i=-1;i<=1;i++) c.drawLine(x-9,y+i*5,x+9,y+i*5,paint);break;
        case "horn":
            Path p=new Path();p.moveTo(x-9,y-3);p.lineTo(x-3,y-3);p.lineTo(x+3,y-8);p.lineTo(x+3,y+8);p.lineTo(x-3,y+3);p.lineTo(x-9,y+3);p.close();c.drawPath(p,paint);
            c.drawArc(new RectF(x+1,y-7,x+13,y+7),-60,120,false,paint);break;
        case "handbrake": text(c,"P",x,y+6,18,0xffeff3fa);break;
        default: break;
        }
    }
    @Override public boolean onTouchEvent(MotionEvent e) {
        int action=e.getActionMasked(),index=e.getActionIndex(),id=e.getPointerId(index);
        float x=(e.getX(index)-getPaddingLeft())/scale,y=(e.getY(index)-getPaddingTop())/scale;
        if(editing) return editTouch(e,x,y);
        if(action==MotionEvent.ACTION_DOWN||action==MotionEvent.ACTION_POINTER_DOWN) {
            Control hit=hitTest(x,y);
            if(hit==null&&action==MotionEvent.ACTION_DOWN)return false;
            pointers.add(id);reveal();
            if(hit==null) return true;
            reveal();
            if(hit.action.equals("mode")) {
                java.util.HashSet<Integer> active=new java.util.HashSet<>(pointers);
                int fingers=screenFingers;
                menuMode=!menuMode;rebuild();pointers.addAll(active);screenFingers=fingers;return true;
            }
            if(hit.action.equals("steering")) for(Contact c:contacts.values()) if(c.control.action.equals("steering")) return true;
            Contact contact=new Contact(hit);contacts.put(id,contact);
            if(hit.pulse) { int key=key(hit.action);keys.press(key);handler.postDelayed(()->keys.release(key),90); }
            else update(contact,x,y);
            invalidate();return true;
        }
        if(action==MotionEvent.ACTION_MOVE) {
            for(int i=0;i<e.getPointerCount();i++) {
                Contact c=contacts.get(e.getPointerId(i));
                if(c!=null&&!c.control.pulse) update(c,(e.getX(i)-getPaddingLeft())/scale,(e.getY(i)-getPaddingTop())/scale);
            }
            invalidate();return true;
        }
        if(action==MotionEvent.ACTION_CANCEL) { releaseAll();scheduleHide();return true; }
        if(action==MotionEvent.ACTION_UP||action==MotionEvent.ACTION_POINTER_UP) {
            pointers.remove(id);
            Contact c=contacts.remove(id);if(c!=null) { release(c);if(c.control.action.equals("steering")) knobX=knobY=0; }
            scheduleHide();invalidate();return true;
        }
        return !contacts.isEmpty();
    }
    private Control hitTest(float x,float y) {
        Control best=null;float distance=Float.MAX_VALUE;
        for(Control c:controls) if(c.hitBox.contains(x,y)) {
            float dx=(x-c.box.centerX())/c.hitBox.width(),dy=(y-c.box.centerY())/c.hitBox.height();
            float d=dx*dx+dy*dy;if(d<distance){best=c;distance=d;}
        }
        return best;
    }
    private boolean editTouch(MotionEvent e,float x,float y) {
        int action=e.getActionMasked();
        if(action==MotionEvent.ACTION_DOWN) {
            selected=hitTest(x,y);editorPointer=e.getPointerId(0);
            if(selected!=null){dragX=x-selected.box.centerX();dragY=y-selected.box.centerY();}
            if(selectionChanged!=null)selectionChanged.accept(selected==null?"Select a control":selected.label);
            if(getParent()!=null)getParent().requestDisallowInterceptTouchEvent(true);invalidate();
        } else if(action==MotionEvent.ACTION_MOVE&&selected!=null) {
            int i=e.findPointerIndex(editorPointer);
            if(i>=0){x=(e.getX(i)-getPaddingLeft())/scale;y=(e.getY(i)-getPaddingTop())/scale;
                selected.box.offset(x-dragX-selected.box.centerX(),y-dragY-selected.box.centerY());constrain(selected);invalidate();}
        } else if(action==MotionEvent.ACTION_UP||action==MotionEvent.ACTION_CANCEL) {
            if(selected!=null)savePosition(selected);editorPointer=-1;
            if(getParent()!=null)getParent().requestDisallowInterceptTouchEvent(false);
        }
        return true;
    }
    private int key(String action) { return GamePreferences.getTouchKey(preferences,action.equals("back")?"pause":action); }
    private void update(Contact c,float x,float y) {
        ArrayList<Integer> wanted=new ArrayList<>();
        if(c.control.action.equals("steering")) {
            float r=c.control.box.width()/2;
            knobX=Math.max(-1,Math.min(1,(x-c.control.box.centerX())/r));
            knobY=menuMode?Math.max(-1,Math.min(1,(y-c.control.box.centerY())/r)):0;
            c.inside=c.control.hitBox.contains(x,y);
            if(!c.inside)knobX=knobY=0;
            if(menuMode) {
                if(Math.abs(knobX)>Math.abs(knobY))knobY=0;else knobX=0;
            }
            if(knobX<-.22f) wanted.add(key("steer_left"));if(knobX>.22f) wanted.add(key("steer_right"));
            if(menuMode&&knobY<-.22f) wanted.add(key("accelerate"));if(menuMode&&knobY>.22f) wanted.add(key("brake"));
        } else { c.inside=c.control.hitBox.contains(x,y);if(c.inside) wanted.add(key(c.control.action)); }
        for(Integer k:c.held) if(!wanted.contains(k)) keys.release(k);
        for(Integer k:wanted) if(!c.held.contains(k)) keys.press(k);
        c.held.clear();c.held.addAll(wanted);
    }
    private void release(Contact c) { for(Integer k:c.held) keys.release(k);c.held.clear(); }
    void releaseAll() { handler.removeCallbacksAndMessages(null);animate().cancel();keys.releaseAll();contacts.clear();pointers.clear();screenFingers=0;knobX=knobY=0;invalidate(); }
    private void fadeIdle() { if(screenFingers==0&&pointers.isEmpty()&&contacts.isEmpty())animate().alpha(preferences.getBoolean(GamePreferences.TOUCH_HIDE_FULL,false)?0f:.12f).setDuration(350).start(); }
    void resumeIdleTimer() { reveal();scheduleHide(); }
    private void reveal() { handler.removeCallbacks(dim);animate().cancel();setAlpha(1); }
    private void scheduleHide() {
        handler.removeCallbacks(dim);
        if(!preview&&screenFingers==0&&contacts.isEmpty()&&pointers.isEmpty()&&preferences.getBoolean(GamePreferences.TOUCH_AUTO_HIDE,false))
            handler.postDelayed(dim,Math.max(1,preferences.getInt(GamePreferences.TOUCH_HIDE_SECONDS,4))*1000L);
    }
    @Override public void onWindowFocusChanged(boolean focus) { super.onWindowFocusChanged(focus);if(!focus) releaseAll(); }
    @Override protected void onDetachedFromWindow() { releaseAll();super.onDetachedFromWindow(); }
}
