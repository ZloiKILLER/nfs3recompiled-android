package dev.nfs3hp.port;

import android.content.Context;
import android.content.SharedPreferences;
import android.graphics.*;
import android.os.Handler;
import android.os.Looper;
import android.os.SystemClock;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowInsets;
import org.libsdl.app.SDLActivity;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

/** Digital controls: movement changes held keys; touches starting outside controls pass to SDL. */
final class TouchControlsOverlay extends View {
    /* Marks the keys the D-pad's up and down arms send to move through menus.
     * They reach the game past SDL's keyboard state (NFS3Activity.sendTouchMenuKey),
     * which is where the native side reads the pedals from, so those arms never
     * accelerate or brake; the mark also keeps an arm and a pedal holding the
     * same key counted apart. */
    static final int MENU_KEY = 0x40000000;
    /* What the short presses of tap controls are released under, so releasing
     * everything drops those and nothing else.  It used to drop every pending
     * callback, the idle timer too, and a layout rebuilt for new window insets
     * right after the game started left the controls on screen for good. */
    private static final Object PULSES = new Object();
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
    /* The area controls are laid out in, in layout units, and where it starts
     * inside the view, in pixels: the whole view in the game, the real screen
     * fitted and centred in a preview (rebuild). */
    private float areaW=1,areaH=1,originX,originY;
    private boolean menuMode;
    private int screenFingers;
    private final java.util.HashSet<Integer> pointers=new java.util.HashSet<>();
    private boolean editing;
    /* Hidden by a press on a gamepad, until the screen is touched again. */
    private boolean gamepadHidden;
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
            if(preview) return;
            if((key&MENU_KEY)!=0) NFS3Activity.sendTouchMenuKey(key&~MENU_KEY,down);
            else if(down) SDLActivity.onNativeKeyDown(key);else SDLActivity.onNativeKeyUp(key);
        });
        setFocusable(false);setContentDescription(c.getString(R.string.touch_preview_description));
        setOnApplyWindowInsetsListener((v,insets)->{
            if(!preview) {
                android.graphics.Rect safe=AndroidWindowCompat.safeInsets(insets);
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
    String selectedLabel() { return selected==null?getContext().getString(R.string.editor_select_control):selected.label; }
    int selectedVisualSize() { return selected==null?100:Math.round(selected.visualSize*100); }
    int selectedHitSize() { return selected==null?100:Math.round(selected.hitSize*100); }
    private String layoutPrefix() {
        return "touch_position_"+(preferences.getBoolean(GamePreferences.TOUCH_SEPARATE,false)?(menuMode?"menu_":"race_"):"shared_");
    }
    void resetLayout() {
        SharedPreferences.Editor edit=preferences.edit();
        for(String k:preferences.getAll().keySet()) if(k.startsWith(layoutPrefix())) edit.remove(k);
        edit.apply();selected=null;rebuild();if(selectionChanged!=null)selectionChanged.accept(getContext().getString(R.string.editor_select_control));
    }
    void resizeSelected(int visual,int hit) {
        if(selected==null)return;
        selected.visualSize=visual/100f;selected.hitSize=hit/100f;
        float x=selected.box.centerX(),y=selected.box.centerY();
        float w=selected.baseWidth*selected.visualSize,h=selected.baseHeight*selected.visualSize;
        selected.box.set(x-w/2,y-h/2,x+w/2,y+h/2);
        constrain(selected);savePosition(selected);invalidate();
    }
    private float areaWidth() { return areaW; }
    private float areaHeight() { return areaH; }
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
    private void rebuild() {
        String selection=selectedAction();releaseAll();controls.clear();selected=null;
        float width=getWidth()-getPaddingLeft()-getPaddingRight(),height=getHeight()-getPaddingTop()-getPaddingBottom();
        if(width<=0||height<=0) return;
        scale=Math.min(getResources().getDisplayMetrics().density,Math.min(width/640f,height/340f));
        float pixelScale=1;
        areaW=width/scale;areaH=height/scale;originX=originY=0;
        if(preview&&referenceWidth>0&&referenceHeight>0){
            /* The real screen, fitted into the card and centred.  The layout
             * editor and the settings preview used to lay controls out across
             * their whole card instead, and two cards of different shapes put
             * the same control in two different places -- neither of them the
             * place the game puts it. */
            pixelScale=Math.min(width/referenceWidth,height/referenceHeight);
            float base=Math.min(getResources().getDisplayMetrics().density,Math.min(referenceWidth/640f,referenceHeight/340f));
            scale=base*pixelScale;
            areaW=referenceWidth/base;areaH=referenceHeight/base;
            originX=(width-areaW*scale)/2;originY=(height-areaH*scale)/2;
        }
        // Where everything goes by default, on the editor's grid: TouchLayout.
        float size=Math.min(preferences.getInt(GamePreferences.TOUCH_SIZE,100)/100f,1.15f);
        boolean mirrored=GamePreferences.TOUCH_LAYOUT_MIRRORED.equals(preferences.getString(GamePreferences.TOUCH_LAYOUT,"standard"));
        for(TouchLayout.Box b:TouchLayout.defaults(areaWidth(),areaHeight(),preferences.getInt(GamePreferences.TOUCH_EDGE,0),
                preferences.getInt(GamePreferences.TOUCH_RAISE,0),size,pixelScale/scale,
                preferences.getBoolean(GamePreferences.TOUCH_SEPARATE,false),menuMode,mirrored))
            controls.add(new Control(b.action,getContext().getString(label(b.action)),
                new RectF(b.left(),b.top(),b.right(),b.bottom()),pulse(b.action)));
        for(Control c:controls) {
            String k=layoutPrefix()+c.action;
            float x=preferences.getFloat(k+"_x",c.box.centerX()/areaWidth())*areaWidth();
            float y=preferences.getFloat(k+"_y",c.box.centerY()/areaHeight())*areaHeight();
            c.visualSize=preferences.getFloat(k+"_size",1);c.hitSize=preferences.getFloat(k+"_hit",1);
            float cw=c.baseWidth*c.visualSize,ch=c.baseHeight*c.visualSize;
            c.box.set(x-cw/2,y-ch/2,x+cw/2,y+ch/2);constrain(c);
            if(c.action.equals(selection))selected=c;
        }
        invalidate();
        // A rebuilt layout starts the idle countdown over rather than losing it.
        scheduleHide();
    }
    private int label(String action) {
        switch(action) {
        case "steering": return R.string.control_navigation;
        case "steer_left": return R.string.control_left;
        case "steer_right": return R.string.control_right;
        case "confirm": return R.string.control_confirm;
        case "back": return R.string.control_back;
        case "brake": return R.string.control_brake;
        case "accelerate": return R.string.control_gas;
        case "handbrake": return R.string.control_handbrake;
        case "look_behind": return R.string.control_look_back;
        case "camera": return R.string.control_camera;
        case "horn": return R.string.control_horn;
        case "spike_strip": return R.string.control_spikes;
        case "gear_down": return R.string.control_gear_down;
        case "gear_up": return R.string.control_gear_up;
        case "pause": return R.string.control_pause;
        case "mode": return menuMode?R.string.control_race:R.string.control_menu;
        case "keyboard": return R.string.control_keyboard;
        case "headlights": return R.string.control_lights;
        default: return R.string.control_recover;
        }
    }
    /* Held for as long as a finger stays on them; the rest press briefly on touch. */
    private static boolean pulse(String action) {
        switch(action) {
        case "steering": case "steer_left": case "steer_right": case "brake": case "accelerate":
        case "handbrake": case "look_behind": case "horn": return false;
        default: return true;
        }
    }
    @Override protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);canvas.save();canvas.translate(getPaddingLeft()+originX,getPaddingTop()+originY);canvas.scale(scale,scale);
        if(preview) {
            // Darker around the screen area, so the card shows where the game's edges are.
            canvas.drawColor(0xff06080c);
            paint.setStyle(Paint.Style.FILL);paint.setColor(0xff0c1017);canvas.drawRect(0,0,areaWidth(),areaHeight(),paint);
            if(editing) {
                drawGrid(canvas);
                text(canvas,getContext().getString(R.string.editor_drag_title),areaWidth()/2,94,24,0xffaab5c6);
                text(canvas,getContext().getString(R.string.editor_resize_hint),areaWidth()/2,124,20,0xff8491a4);
            } else text(canvas,getContext().getString(R.string.touch_preview_hint),areaWidth()/2,118,24,0xff8491a4);
        }
        int opacity=preferences.getInt(GamePreferences.TOUCH_OPACITY,65);
        for(Control c:controls) {
            boolean down=false;
            for(Contact contact:contacts.values()) if(contact.control==c&&contact.inside) down=true;
            paint.setStyle(Paint.Style.FILL);paint.setColor(Color.argb(Math.round(opacity*2.1f),down?88:18,down?64:24,down?30:34));
            // The steering control, wherever a layout has one, is the four-way D-pad.
            boolean stick=c.action.equals("steering");
            if(stick) canvas.drawPath(dpad(c.box),paint);else canvas.drawRoundRect(c.box,16,16,paint);
            paint.setStyle(Paint.Style.STROKE);paint.setStrokeWidth(down?2.2f:1.4f);
            paint.setColor(down?0xffffbf69:Color.argb(Math.round(opacity*2.55f),203,216,232));
            if(stick) {
                canvas.drawPath(dpad(c.box),paint);
                float x=c.box.centerX(),y=c.box.centerY(),r=c.box.width()/2;
                arrow(canvas,x-r*.68f,y,-1,0);arrow(canvas,x+r*.68f,y,1,0);
                arrow(canvas,x,y-r*.68f,0,-1);arrow(canvas,x,y+r*.68f,0,1);
                paint.setStyle(Paint.Style.FILL);paint.setColor(down?0xffad8148:0xff3b4555);
                if(down) canvas.drawCircle(x+knobX*r*.65f,y+knobY*r*.65f,r*.12f,paint);
            } else {
                canvas.drawRoundRect(c.box,16,16,paint);
                if(c.action.equals("accelerate")||c.action.equals("brake")) {
                    float firstLine=c.box.centerY()-13.5f;
                    for(int i=0;i<4;i++) canvas.drawLine(c.box.left+13,firstLine+i*9,c.box.right-13,firstLine+i*9,paint);
                } else icon(canvas,c.action,c.box.centerX(),c.box.centerY());
            }
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
    /* The editor's grid over the area controls can be placed in: a hairline every
     * cell and a brighter line every fourth.  Default positions stand on it, and
     * with Snap to grid a dragged control lands on it. */
    private void drawGrid(Canvas canvas) {
        float w=areaWidth(),h=areaHeight();
        paint.setStyle(Paint.Style.STROKE);paint.setPathEffect(null);
        for(int i=0;i*TouchLayout.GRID<=w;i++) { gridLine(i);canvas.drawLine(i*TouchLayout.GRID,0,i*TouchLayout.GRID,h,paint); }
        for(int i=0;i*TouchLayout.GRID<=h;i++) { gridLine(i);canvas.drawLine(0,i*TouchLayout.GRID,w,i*TouchLayout.GRID,paint); }
    }
    private void gridLine(int index) {
        boolean major=index%4==0;
        paint.setStrokeWidth(major?1.5f/scale:0);paint.setColor(major?0x40aab5c6:0x1caab5c6);
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
    private void centeredText(Canvas c,String s,float x,float y,float size,int color) {
        paint.setTextSize(size);Paint.FontMetrics metrics=paint.getFontMetrics();
        text(c,s,x,y-(metrics.ascent+metrics.descent)/2,size,color);
    }
    private void arrow(Canvas c,float x,float y,int dx,int dy) {
        c.drawLine(x-dx*4-dy*5,y-dy*4+dx*5,x+dx*3,y+dy*3,paint);
        c.drawLine(x-dx*4+dy*5,y-dy*4-dx*5,x+dx*3,y+dy*3,paint);
    }
    /* The caller has already put the opacity setting -- and the pressed
     * highlight -- into the paint, so the glyphs drawn as text below take their
     * colour from it rather than naming one.  Spelling out a literal here is
     * what left "+", "-" and "P" fully opaque whatever the slider said, while
     * every control drawn with the paint itself faded correctly. */
    private void icon(Canvas c,String a,float x,float y) {
        paint.setStyle(Paint.Style.STROKE);paint.setStrokeWidth(2);paint.setStrokeCap(Paint.Cap.ROUND);
        switch(a) {
        case "gear_up": centeredText(c,"+",x,y,24,paint.getColor());break;
        case "gear_down": centeredText(c,"−",x,y,24,paint.getColor());break;
        case "steer_left": arrow(c,x,y,-1,0);break;
        case "steer_right": arrow(c,x,y,1,0);break;
        case "back": arrow(c,x,y,-1,0);break;
        case "pause": c.drawLine(x-4,y-6,x-4,y+6,paint);c.drawLine(x+4,y-6,x+4,y+6,paint);break;
        case "headlights":
            c.drawArc(new RectF(x-10,y-7,x+3,y+7),90,180,false,paint);c.drawLine(x-4,y-7,x-4,y+7,paint);
            for(int i=-1;i<=1;i++) c.drawLine(x+1,y+i*5,x+10,y+i*5,paint);break;
        case "recover":
            c.drawArc(new RectF(x-9,y-7,x+9,y+7),35,280,false,paint);arrow(c,x+7,y-5,0,1);break;
        case "look_behind":
            RectF eye=new RectF(x-10,y-6,x+10,y+6);
            c.drawArc(eye,200,140,false,paint);c.drawArc(eye,20,140,false,paint);
            c.drawCircle(x,y,2.5f,paint);break;
        case "keyboard":
            c.drawRoundRect(new RectF(x-13,y-8,x+13,y+8),3,3,paint);
            for(int row=0;row<2;row++)for(int col=0;col<5;col++)
                c.drawPoint(x-9+col*4.5f,y-4+row*5,paint);
            c.drawLine(x-8,y+5,x+8,y+5,paint);break;
        case "camera":
            c.drawRoundRect(new RectF(x-10,y-6,x+10,y+7),3,3,paint);c.drawCircle(x,y,4,paint);c.drawLine(x-4,y-9,x+3,y-9,paint);break;
        case "confirm": c.drawLine(x-8,y,x-2,y+6,paint);c.drawLine(x-2,y+6,x+9,y-6,paint);break;
        case "mode": for(int i=-1;i<=1;i++) c.drawLine(x-9,y+i*5,x+9,y+i*5,paint);break;
        case "horn":
            Path p=new Path();p.moveTo(x-9,y-3);p.lineTo(x-3,y-3);p.lineTo(x+3,y-8);p.lineTo(x+3,y+8);p.lineTo(x-3,y+3);p.lineTo(x-9,y+3);p.close();c.drawPath(p,paint);
            c.drawArc(new RectF(x+1,y-7,x+13,y+7),-60,120,false,paint);break;
        case "spike_strip":
            for(int i=-1;i<=1;i++) {
                Path spike=new Path();float sx=x+i*7;
                spike.moveTo(sx-3,y+5);spike.lineTo(sx,y-6);spike.lineTo(sx+3,y+5);spike.close();c.drawPath(spike,paint);
            }
            c.drawLine(x-12,y+6,x+12,y+6,paint);break;
        case "handbrake": text(c,"P",x,y+6,18,paint.getColor());break;
        default: break;
        }
    }
    @Override public boolean onTouchEvent(MotionEvent e) {
        int action=e.getActionMasked(),index=e.getActionIndex(),id=e.getPointerId(index);
        float x=(e.getX(index)-getPaddingLeft()-originX)/scale,y=(e.getY(index)-getPaddingTop()-originY)/scale;
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
            if(hit.action.equals("keyboard")) {
                if(!preview&&getContext() instanceof NFS3Activity)
                    ((NFS3Activity)getContext()).showTouchKeyboard();
                return true;
            }
            if(hit.action.equals("steering")) for(Contact c:contacts.values()) if(c.control.action.equals("steering")) return true;
            Contact contact=new Contact(hit);contacts.put(id,contact);
            if(hit.pulse) { int key=key(hit.action);keys.press(key);handler.postAtTime(()->keys.release(key),PULSES,SystemClock.uptimeMillis()+90); }
            else update(contact,x,y);
            invalidate();return true;
        }
        if(action==MotionEvent.ACTION_MOVE) {
            for(int i=0;i<e.getPointerCount();i++) {
                Contact c=contacts.get(e.getPointerId(i));
                if(c!=null&&!c.control.pulse) update(c,(e.getX(i)-getPaddingLeft()-originX)/scale,(e.getY(i)-getPaddingTop()-originY)/scale);
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
            if(selectionChanged!=null)selectionChanged.accept(selected==null?getContext().getString(R.string.editor_select_control):selected.label);
            if(getParent()!=null)getParent().requestDisallowInterceptTouchEvent(true);invalidate();
        } else if(action==MotionEvent.ACTION_MOVE&&selected!=null) {
            int i=e.findPointerIndex(editorPointer);
            if(i>=0){x=(e.getX(i)-getPaddingLeft()-originX)/scale;y=(e.getY(i)-getPaddingTop()-originY)/scale;
                selected.box.offset(x-dragX-selected.box.centerX(),y-dragY-selected.box.centerY());
                // The centre onto the nearest grid point, while dragging, so it shows where the control lands.
                if(preferences.getBoolean(GamePreferences.TOUCH_SNAP,true))
                    selected.box.offset(TouchLayout.snap(selected.box.centerX())-selected.box.centerX(),
                                        TouchLayout.snap(selected.box.centerY())-selected.box.centerY());
                constrain(selected);invalidate();}
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
            /* One direction at a time, in every layout.  Left and right steer; up
             * and down move through menus -- on the pedals' keys, but past the
             * pedals themselves (MENU_KEY), so a race is driven with the pedals. */
            float r=c.control.box.width()/2;
            knobX=Math.max(-1,Math.min(1,(x-c.control.box.centerX())/r));
            knobY=Math.max(-1,Math.min(1,(y-c.control.box.centerY())/r));
            c.inside=c.control.hitBox.contains(x,y);
            if(!c.inside)knobX=knobY=0;
            if(Math.abs(knobX)>Math.abs(knobY))knobY=0;else knobX=0;
            if(knobX<-.22f) wanted.add(key("steer_left"));if(knobX>.22f) wanted.add(key("steer_right"));
            if(knobY<-.22f) menuKey(wanted,key("accelerate"));if(knobY>.22f) menuKey(wanted,key("brake"));
        } else { c.inside=c.control.hitBox.contains(x,y);if(c.inside) wanted.add(key(c.control.action)); }
        for(Integer k:c.held) if(!wanted.contains(k)) keys.release(k);
        for(Integer k:wanted) if(!c.held.contains(k)) keys.press(k);
        c.held.clear();c.held.addAll(wanted);
    }
    private static void menuKey(ArrayList<Integer> wanted,int key) { if(key!=0) wanted.add(MENU_KEY|key); }
    private void release(Contact c) { for(Integer k:c.held) keys.release(k);c.held.clear(); }
    void releaseAll() { handler.removeCallbacksAndMessages(PULSES);keys.releaseAll();contacts.clear();pointers.clear();screenFingers=0;knobX=knobY=0;invalidate(); }
    private void fadeIdle() { if(!gamepadHidden&&screenFingers==0&&pointers.isEmpty()&&contacts.isEmpty())animate().alpha(preferences.getBoolean(GamePreferences.TOUCH_HIDE_FULL,false)?0f:.12f).setDuration(350).start(); }
    void resumeIdleTimer() { if(gamepadHidden)return;reveal();scheduleHide(); }
    private void reveal() { gamepadHidden=false;handler.removeCallbacks(dim);animate().cancel();setAlpha(1); }
    private void scheduleHide() {
        handler.removeCallbacks(dim);
        if(!preview&&!gamepadHidden&&screenFingers==0&&contacts.isEmpty()&&pointers.isEmpty()&&preferences.getBoolean(GamePreferences.TOUCH_AUTO_HIDE,false))
            handler.postDelayed(dim,Math.max(1,preferences.getInt(GamePreferences.TOUCH_HIDE_SECONDS,4))*1000L);
    }
    /* A press on a gamepad: the controls get out of the way at once, whatever the
     * auto-hide setting says, and stay away until the screen is touched again. */
    void hideForGamepad() {
        if(preview||gamepadHidden||screenFingers>0||!contacts.isEmpty()||!pointers.isEmpty())return;
        gamepadHidden=true;handler.removeCallbacks(dim);animate().cancel();animate().alpha(0f).setDuration(120).start();
    }
    @Override public void onWindowFocusChanged(boolean focus) { super.onWindowFocusChanged(focus);if(!focus) releaseAll();else scheduleHide(); }
    @Override protected void onDetachedFromWindow() { releaseAll();handler.removeCallbacks(dim);super.onDetachedFromWindow(); }
}
