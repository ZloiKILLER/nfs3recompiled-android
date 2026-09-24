package dev.nfs3hp.port;

import android.app.Instrumentation;
import android.os.Bundle;
import android.view.MotionEvent;
import android.graphics.RectF;
import java.lang.reflect.Field;
import java.util.*;

/** Device checks using real MotionEvents; preview mode never sends keys to the game. */
public final class TouchOverlayInstrumentation extends Instrumentation {
    /* -e probe pad_vibration runs PadVibrationProbe instead of the checks. */
    private String probe;
    @Override public void onCreate(Bundle arguments) { super.onCreate(arguments);probe=arguments==null?null:arguments.getString("probe");PadVibrationProbe.pattern=arguments==null?null:arguments.getString("pattern");PadVibrationProbe.context=getTargetContext();start(); }
    private static Object field(Object object,String name) throws Exception {
        Field f=object.getClass().getDeclaredField(name);f.setAccessible(true);return f.get(object);
    }
    private static void require(boolean value,String message) { if(!value) throw new AssertionError(message); }
    @SuppressWarnings("unchecked")
    private static Map<Integer,Integer> held(TouchControlsOverlay view) throws Exception {
        return (Map<Integer,Integer>)field(field(view,"keys"),"held");
    }
    private static RectF bounds(TouchControlsOverlay view,String action) throws Exception {
        for(Object c:(Iterable<?>)field(view,"controls")) if(action.equals(field(c,"action"))) {
            RectF b=new RectF((RectF)field(c,"box"));float s=(float)field(view,"scale");
            float ox=(float)field(view,"originX"),oy=(float)field(view,"originY");
            return new RectF(ox+b.left*s,oy+b.top*s,ox+b.right*s,oy+b.bottom*s);
        }
        throw new AssertionError("Missing control: "+action);
    }
    private static boolean touch(TouchControlsOverlay view,int action,int[] ids,float... xy) {
        MotionEvent.PointerProperties[] pp=new MotionEvent.PointerProperties[ids.length];
        MotionEvent.PointerCoords[] pc=new MotionEvent.PointerCoords[ids.length];
        for(int i=0;i<ids.length;i++) {
            pp[i]=new MotionEvent.PointerProperties();pp[i].id=ids[i];pp[i].toolType=MotionEvent.TOOL_TYPE_FINGER;
            pc[i]=new MotionEvent.PointerCoords();pc[i].x=xy[i*2];pc[i].y=xy[i*2+1];pc[i].pressure=1;pc[i].size=1;
        }
        MotionEvent e=MotionEvent.obtain(1,2,action,ids.length,pp,pc,0,0,1,1,0,0,0,0);
        boolean result=view.onTouchEvent(e);e.recycle();return result;
    }
    @Override public void onStart() {
        Bundle result=new Bundle();
        if("pad_vibration".equals(probe)) {
            try { result.putString("stream",PadVibrationProbe.run());finish(-1,result); }
            catch(Throwable e) { result.putString("stream","FAIL: "+android.util.Log.getStackTraceString(e));finish(0,result); }
            return;
        }
        try {
            ArrayList<String> events=new ArrayList<>();
            TouchKeyState state=new TouchKeyState((key,down)->events.add(key+":"+down));
            state.press(21);state.press(21);state.release(21);
            require(events.size()==1,"second finger must keep key held");
            state.release(21);state.release(21);
            require(events.equals(Arrays.asList("21:true","21:false")),"one key up per key down");
            state.press(0);require(events.size()==2,"unassigned key ignored");
            final Throwable[] failure={null};
            runOnMainSync(()->{
                TouchControlsOverlay view=null;
                try {
                    view=new TouchControlsOverlay(getTargetContext(),true);view.layout(0,0,1280,720);
                    android.graphics.Bitmap shot=android.graphics.Bitmap.createBitmap(1280,720,android.graphics.Bitmap.Config.ARGB_8888);
                    view.draw(new android.graphics.Canvas(shot));
                    try(java.io.FileOutputStream out=new java.io.FileOutputStream(new java.io.File(getTargetContext().getExternalFilesDir(null),"ui-overlay-landscape.png"))) {
                        shot.compress(android.graphics.Bitmap.CompressFormat.PNG,100,out);
                    }
                    shot.recycle();
                    RectF gas=bounds(view,"accelerate"),spikes=bounds(view,"spike_strip");
                    RectF stick=bounds(view,"steer_left");
                    float sx=stick.left+stick.width()*.18f;
                    float sy=stick.centerY(),gx=gas.centerX(),gy=gas.centerY();
                    int left=GamePreferences.getTouchKey(GamePreferences.get(getTargetContext()),"steer_left");
                    int up=GamePreferences.getTouchKey(GamePreferences.get(getTargetContext()),"accelerate");
                    require(!touch(view,MotionEvent.ACTION_DOWN,new int[]{7},640,360),"blank area must pass through");
                    touch(view,MotionEvent.ACTION_DOWN,new int[]{7},sx,sy);
                    touch(view,MotionEvent.ACTION_POINTER_DOWN|(1<<8),new int[]{7,42},sx,sy,gx,gy);
                    require(held(view).containsKey(left)&&held(view).containsKey(up),"steering plus throttle");
                    touch(view,MotionEvent.ACTION_POINTER_UP,new int[]{7,42},sx,sy,gx,gy);
                    require(!held(view).containsKey(left)&&held(view).containsKey(up),"pointer ID survives index change");
                    touch(view,MotionEvent.ACTION_MOVE,new int[]{42},640,360);
                    require(held(view).isEmpty(),"leaving pedal releases key");
                    touch(view,MotionEvent.ACTION_MOVE,new int[]{42},gx,gy);
                    require(held(view).containsKey(up),"sliding back re-engages pedal");
                    touch(view,MotionEvent.ACTION_CANCEL,new int[]{42},gx,gy);
                    require(held(view).isEmpty(),"cancel releases everything");
                    int spikeKey=GamePreferences.getTouchKey(GamePreferences.get(getTargetContext()),"spike_strip");
                    require(spikeKey==android.view.KeyEvent.KEYCODE_S,"spike strip defaults to S");
                    touch(view,MotionEvent.ACTION_DOWN,new int[]{51},spikes.centerX(),spikes.centerY());
                    require(held(view).containsKey(spikeKey),"spike strip emits its configured key");
                    view.releaseAll();
                    /* The menu layout is what the game asks for when it is not
                     * racing, and it has nothing on it: a menu is worked with the
                     * screen itself, the system's back is the game's Escape, and
                     * the keyboard comes up by itself while the game takes a name. */
                    view.setMenuMode(true);
                    require(TouchRefinementChecks.bounds(view,"back","box")==null
                        &&TouchRefinementChecks.bounds(view,"keyboard","box")==null
                        &&TouchRefinementChecks.bounds(view,"headlights","box")==null
                        &&TouchRefinementChecks.bounds(view,"recover","box")==null
                        &&TouchRefinementChecks.bounds(view,"steer_left","box")==null
                        &&TouchRefinementChecks.bounds(view,"accelerate","box")==null,
                        "the menu layout is the menu, with nothing of ours on it");
                    require((boolean)field(view,"menuMode"),"the game decides which layout is up");
                    view.setMenuMode(false);
                    view.releaseAll();require(held(view).isEmpty(),"pause clears key state");
                    android.content.SharedPreferences prefs=GamePreferences.get(getTargetContext());
                    Map<String,?> saved=prefs.getAll();
                    ArrayList<String> changed=new ArrayList<>(Arrays.asList(GamePreferences.TOUCH_SIZE,GamePreferences.TOUCH_EDGE,GamePreferences.TOUCH_LAYOUT));
                    /* What is checked here is the default arrangement: controls a tester
                     * dragged in the layout editor are set aside for the check and put
                     * back afterwards exactly as they were. */
                    android.content.SharedPreferences.Editor setAside=prefs.edit();
                    for(String k:saved.keySet()) if(k.startsWith("touch_position_")) { changed.add(k);setAside.remove(k); }
                    setAside.commit();
                    try {
                        prefs.edit().putInt(GamePreferences.TOUCH_SIZE,115).putInt(GamePreferences.TOUCH_EDGE,32).apply();
                        // Both layouts, both ways round.
                        for(boolean menu:new boolean[]{false,true})
                        for(String layout:new String[]{GamePreferences.TOUCH_LAYOUT_STANDARD,GamePreferences.TOUCH_LAYOUT_MIRRORED}) {
                            prefs.edit().putString(GamePreferences.TOUCH_LAYOUT,layout).apply();
                            view.setMenuMode(menu);
                            view.layout(0,0,640,340);view.refreshSettings();
                            ArrayList<RectF> boxes=new ArrayList<>();
                            ArrayList<String> names=new ArrayList<>();
                            for(Object control:(Iterable<?>)field(view,"controls")) {
                                String name=(String)field(control,"action");
                                RectF box=(RectF)field(control,"box");
                                require(box.left>=0&&box.top>=0&&box.right<=640&&box.bottom<=340,name+" remains on screen");
                                for(int i=0;i<boxes.size();++i) require(!RectF.intersects(box,boxes.get(i)),
                                    layout+" "+(menu?"menu":"race")+": "+name+" "+box+" does not overlap "+names.get(i)+" "+boxes.get(i));
                                boxes.add(box);names.add(name);
                            }
                        }
                    } finally {
                        android.content.SharedPreferences.Editor edit=prefs.edit();
                        for(String k:changed) {
                            Object value=saved.get(k);
                            if(value instanceof Integer) edit.putInt(k,(Integer)value);
                            else if(value instanceof String) edit.putString(k,(String)value);
                            else if(value instanceof Boolean) edit.putBoolean(k,(Boolean)value);
                            else if(value instanceof Float) edit.putFloat(k,(Float)value);
                            else edit.remove(k);
                        }
                        edit.commit();
                    }
                } catch(Throwable e) { failure[0]=e; }
                finally { if(view!=null)view.releaseAll(); }
            });
            if(failure[0]!=null) throw new AssertionError(failure[0]);
            TouchRefinementChecks.run(this);
            SaveGameChecks.run(this);
            LauncherActivity activity=(LauncherActivity)startActivitySync(new android.content.Intent(getTargetContext(),LauncherActivity.class)
                .addFlags(android.content.Intent.FLAG_ACTIVITY_NEW_TASK));
            waitForIdleSync();
            HapticsChecks.run(this);
            capture("ui-launcher.png");
            /* The launcher is Compose now: the screens are walked through the
             * activity's own navigation, and each is checked for what it is
             * there to do. */
            String version=activity.versionLabel();
            require(version.startsWith("v0.")&&version.endsWith(" DEBUG"),"the launcher names a debug build's version: "+version);
            runOnMainSync(()->activity.go(LauncherActivity.Screen.Controls));
            waitForIdleSync();capture("ui-controls-home.png");
            runOnMainSync(()->activity.go(LauncherActivity.Screen.Touch));
            waitForIdleSync();Thread.sleep(500);waitForIdleSync();
            runOnMainSync(()->{
                TouchControlsOverlay canvas=activity.getTouchPreview();
                require(canvas!=null,"the touch screen shows the overlay itself");
                android.view.View host=(android.view.View)canvas.getParent();
                require(canvas.getWidth()==host.getWidth()&&canvas.getHeight()==host.getHeight(),"the touch canvas fills its card");
                try {
                    /* Controls are laid out on the real screen's shape, fitted into the
                     * card, so the preview, the editor and the game agree. */
                    float areaW=(float)field(canvas,"areaW"),areaH=(float)field(canvas,"areaH");
                    float refW=(float)field(canvas,"referenceWidth"),refH=(float)field(canvas,"referenceHeight");
                    require(Math.abs(areaW/areaH-refW/refH)<.01f,"the preview lays controls out on the screen's own shape: area "+areaW+"x"+areaH+" ref "+refW+"x"+refH+" view "+canvas.getWidth()+"x"+canvas.getHeight());
                } catch(Exception e) { throw new RuntimeException(e); }
                try {
                    RectF leftButton=bounds(canvas,"steer_left");
                    touch(canvas,MotionEvent.ACTION_DOWN,new int[]{61},leftButton.centerX(),leftButton.centerY());
                    int leftKey=GamePreferences.getTouchKey(GamePreferences.get(getTargetContext()),"steer_left");
                    require(held(canvas).containsKey(leftKey),"a control in the fitted preview is touchable");
                    canvas.releaseAll();
                } catch(Exception e) { throw new RuntimeException(e); }
            });
            capture("ui-touch-settings.png");
            runOnMainSync(()->activity.go(LauncherActivity.Screen.TouchKeys));waitForIdleSync();
            capture("ui-touch-keys.png");
            runOnMainSync(()->activity.onBackPressed());waitForIdleSync();
            require(activity.getScreen()==LauncherActivity.Screen.Touch,"back from touch keys returns to touch settings");
            runOnMainSync(()->activity.openEditor());waitForIdleSync();
            /* One layout to arrange: the racing one.  The menus carry nothing of
             * ours any more, so the editor no longer asks which is being edited. */
            require(activity.findViewById(R.id.editor_done)!=null,"the layout editor opens from touch settings");
            capture("ui-touch-editor-race.png");
            runOnMainSync(()->activity.findViewById(R.id.editor_done).performClick());waitForIdleSync();
            require(activity.getScreen()==LauncherActivity.Screen.Touch,"done in the editor returns to touch settings");
            runOnMainSync(()->activity.onBackPressed());waitForIdleSync();
            require(activity.getScreen()==LauncherActivity.Screen.Controls,"back from touch settings returns to controls");
            runOnMainSync(()->activity.go(LauncherActivity.Screen.Gamepads));waitForIdleSync();
            runOnMainSync(()->activity.setCapturingSlot(0));waitForIdleSync();
            runOnMainSync(()->activity.onBackPressed());waitForIdleSync();
            require(activity.getCapturingSlot()==-1&&activity.getScreen()==LauncherActivity.Screen.Gamepads,
                "back cancels the wait for a pad without leaving the screen");
            capture("ui-gamepads.png");
            runOnMainSync(()->activity.go(LauncherActivity.Screen.ControlsHelp));waitForIdleSync();
            capture("ui-controls-help.png");
            runOnMainSync(()->activity.onBackPressed());waitForIdleSync();
            require(activity.getScreen()==LauncherActivity.Screen.Gamepads,"back from help returns to the gamepads screen");
            runOnMainSync(()->activity.openGamepadButtons(1));waitForIdleSync();
            capture("ui-gamepad-buttons.png");
            runOnMainSync(()->activity.onBackPressed());waitForIdleSync();
            require(activity.getScreen()==LauncherActivity.Screen.Gamepads,"back from the buttons returns to the gamepads screen");
            runOnMainSync(()->activity.onBackPressed());waitForIdleSync();
            runOnMainSync(()->activity.onBackPressed());waitForIdleSync();
            require(activity.getScreen()==LauncherActivity.Screen.Main,"back walks up to the first screen");
            /* Display, saved as it is chosen with no Save button, and Screen
             * adjustment underneath it. */
            runOnMainSync(()->activity.go(LauncherActivity.Screen.Display));waitForIdleSync();
            capture("ui-display.png");
            runOnMainSync(()->activity.go(LauncherActivity.Screen.Adjustment));waitForIdleSync();
            capture("ui-screen-adjustment.png");
            runOnMainSync(()->activity.onBackPressed());waitForIdleSync();
            require(activity.getScreen()==LauncherActivity.Screen.Display,"back from screen adjustment returns to display");
            runOnMainSync(()->activity.onBackPressed());waitForIdleSync();
            /* Data is a menu, so saves live one level down -- and back has to
             * return to that menu rather than all the way out. */
            runOnMainSync(()->activity.go(LauncherActivity.Screen.Data));waitForIdleSync();
            runOnMainSync(()->activity.go(LauncherActivity.Screen.LauncherSettings));waitForIdleSync();
            runOnMainSync(()->activity.onBackPressed());waitForIdleSync();
            require(activity.getScreen()==LauncherActivity.Screen.Data,"back from a sub-screen returns to the data menu");
            runOnMainSync(()->activity.go(LauncherActivity.Screen.Saves));waitForIdleSync();
            capture("ui-data-saves.png");
            runOnMainSync(()->activity.go(LauncherActivity.Screen.GameData));waitForIdleSync();
            capture("ui-game-data.png");
            runOnMainSync(()->activity.onBackPressed());waitForIdleSync();
            runOnMainSync(()->activity.onBackPressed());waitForIdleSync();
            runOnMainSync(()->activity.go(LauncherActivity.Screen.Faq));waitForIdleSync();
            capture("ui-faq.png");
            result.putString("stream","PASS: touch geometry/keyboard, multitouch, vibration cap, gamepad slots, touch keys, save ZIP backup/import/export, layout editor and auto-hide\n");
            finish(-1,result);
        } catch(Throwable e) { result.putString("stream","FAIL: "+android.util.Log.getStackTraceString(e));finish(0,result); }
    }
    private void capture(String name) throws Exception {
        // Wait for the compositor, not just the Activity's message queue.
        Thread.sleep(300);
        android.graphics.Bitmap shot=getUiAutomation().takeScreenshot();
        require(shot!=null,"screenshot available");
        try(java.io.FileOutputStream out=new java.io.FileOutputStream(new java.io.File(getTargetContext().getExternalFilesDir(null),name))) {
            shot.compress(android.graphics.Bitmap.CompressFormat.PNG,100,out);
        }
        shot.recycle();
    }
}
