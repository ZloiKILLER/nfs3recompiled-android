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
                    /* The menu layout is what the game asks for when it is not racing:
                     * back and the keyboard, and nothing a race needs. */
                    view.setMenuMode(true);
                    bounds(view,"back");bounds(view,"keyboard");
                    require(TouchRefinementChecks.bounds(view,"headlights","box")==null
                        &&TouchRefinementChecks.bounds(view,"recover","box")==null
                        &&TouchRefinementChecks.bounds(view,"steer_left","box")==null
                        &&TouchRefinementChecks.bounds(view,"accelerate","box")==null,"the menu layout leaves the race behind");
                    require((boolean)field(view,"menuMode"),"the game decides which layout is up");
                    view.setMenuMode(false);
                    view.releaseAll();require(held(view).isEmpty(),"pause clears key state");
                    android.content.SharedPreferences prefs=GamePreferences.get(getTargetContext());
                    Map<String,?> saved=prefs.getAll();
                    ArrayList<String> changed=new ArrayList<>(Arrays.asList(GamePreferences.TOUCH_SIZE,GamePreferences.TOUCH_EDGE,GamePreferences.TOUCH_RAISE,GamePreferences.TOUCH_LAYOUT));
                    /* What is checked here is the default arrangement: controls a tester
                     * dragged in the layout editor are set aside for the check and put
                     * back afterwards exactly as they were. */
                    android.content.SharedPreferences.Editor setAside=prefs.edit();
                    for(String k:saved.keySet()) if(k.startsWith("touch_position_")) { changed.add(k);setAside.remove(k); }
                    setAside.commit();
                    try {
                        prefs.edit().putInt(GamePreferences.TOUCH_SIZE,115).putInt(GamePreferences.TOUCH_EDGE,32)
                            .putInt(GamePreferences.TOUCH_RAISE,24).apply();
                        // Both layouts, both ways round.
                        for(boolean menu:new boolean[]{false,true})
                        for(String layout:new String[]{GamePreferences.TOUCH_LAYOUT_STANDARD,GamePreferences.TOUCH_LAYOUT_MIRRORED}) {
                            prefs.edit().putString(GamePreferences.TOUCH_LAYOUT,layout).apply();
                            view.setMenuMode(menu);
                            view.layout(0,0,640,340);view.refreshSettings();
                            ArrayList<RectF> boxes=new ArrayList<>();
                            for(Object control:(Iterable<?>)field(view,"controls")) {
                                RectF box=(RectF)field(control,"box");
                                require(box.left>=0&&box.top>=0&&box.right<=640&&box.bottom<=340,"controls remain on screen");
                                for(RectF other:boxes) require(!RectF.intersects(box,other),"controls do not overlap");
                                boxes.add(box);
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
            android.app.Activity activity=startActivitySync(new android.content.Intent(getTargetContext(),LauncherActivity.class)
                .addFlags(android.content.Intent.FLAG_ACTIVITY_NEW_TASK));
            waitForIdleSync();
            HapticsChecks.run(this);
            capture("ui-launcher.png");
            /* The main screen: the build's version under the settings column, and
             * that column spanning exactly the height of the title card. */
            String version=((android.widget.TextView)activity.findViewById(R.id.version_text)).getText().toString();
            require(version.startsWith("v0.")&&version.endsWith(" DEBUG"),"the launcher names a debug build's version: "+version);
            runOnMainSync(()->{
                android.view.View card=(android.view.View)activity.findViewById(R.id.play_button).getParent();
                android.view.View first=activity.findViewById(R.id.controls_button),last=activity.findViewById(R.id.faq_button);
                int[] c=new int[2],f=new int[2],l=new int[2];
                card.getLocationOnScreen(c);first.getLocationOnScreen(f);last.getLocationOnScreen(l);
                require(Math.abs(c[1]-f[1])<=1&&Math.abs(c[1]+card.getHeight()-l[1]-last.getHeight())<=1,
                    "the settings column spans the title card, top to bottom");
            });
            runOnMainSync(()->activity.findViewById(R.id.controls_button).performClick());
            waitForIdleSync();capture("ui-controls-home.png");
            require(activity.findViewById(R.id.controls_help_button)==null,"split screen help lives on the gamepads screen");
            runOnMainSync(()->activity.findViewById(R.id.touch_controls_button).performClick());
            waitForIdleSync();Thread.sleep(500);waitForIdleSync();
            runOnMainSync(()->{
                android.widget.FrameLayout host=activity.findViewById(R.id.touch_preview);
                require(host.getChildCount()==1,"preview has one touch canvas");
                TouchControlsOverlay canvas=(TouchControlsOverlay)host.getChildAt(0);
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
            runOnMainSync(()->activity.findViewById(R.id.touch_keys_button).performClick());waitForIdleSync();
            require(((android.view.ViewGroup)activity.findViewById(R.id.touch_keys_container)).getChildCount()==GamePreferences.ACTION_IDS.length,"every touch control has a key to choose");
            capture("ui-touch-keys.png");
            runOnMainSync(()->activity.onBackPressed());waitForIdleSync();
            require(activity.findViewById(R.id.edit_touch_layout)!=null,"back from touch keys returns to touch settings");
            runOnMainSync(()->activity.findViewById(R.id.edit_touch_layout).performClick());waitForIdleSync();
            capture("ui-touch-editor-race.png");
            runOnMainSync(()->{
                android.widget.Spinner mode=activity.findViewById(R.id.editor_mode);
                if(mode.getAdapter().getCount()>1)mode.setSelection(1);
            });waitForIdleSync();
            capture("ui-touch-editor-menu.png");
            runOnMainSync(()->activity.findViewById(R.id.editor_done).performClick());waitForIdleSync();
            runOnMainSync(()->activity.onBackPressed());waitForIdleSync();
            runOnMainSync(()->activity.findViewById(R.id.gamepad_controls_button).performClick());waitForIdleSync();
            require(activity.findViewById(R.id.gamepad1_assign)!=null&&activity.findViewById(R.id.gamepad2_assign)!=null,"gamepads screen offers both slots");
            runOnMainSync(()->activity.findViewById(R.id.gamepad1_assign).performClick());waitForIdleSync();
            require(((android.widget.TextView)activity.findViewById(R.id.gamepad1_status)).getText().toString()
                .equals(activity.getString(R.string.gamepad_assign_prompt,1)),"assigning waits for a button press");
            runOnMainSync(()->activity.onBackPressed());waitForIdleSync();
            require(activity.findViewById(R.id.gamepad1_assign)!=null,"back cancels the wait without leaving the screen");
            capture("ui-gamepads.png");
            require(activity.findViewById(R.id.write_gamepad_controls)!=null
                &&activity.findViewById(R.id.write_keyboard_controls)!=null,"both control sets can be written to the game");
            require(((android.widget.CheckBox)activity.findViewById(R.id.gamepad_vibration_check)).isChecked()
                ==GamePreferences.get(getTargetContext()).getBoolean(GamePreferences.GAMEPAD_VIBRATION,false),
                "the gamepad vibration switch shows its setting");
            runOnMainSync(()->activity.findViewById(R.id.controls_help_button).performClick());waitForIdleSync();
            require(((android.widget.TextView)activity.findViewById(R.id.controls_help_body)).getText().length()>200,
                "controls help explains split screen");
            capture("ui-controls-help.png");
            runOnMainSync(()->activity.onBackPressed());waitForIdleSync();
            require(activity.findViewById(R.id.gamepad1_assign)!=null,"back from help returns to the gamepads screen");
            runOnMainSync(()->activity.findViewById(R.id.gamepad2_buttons).performClick());waitForIdleSync();
            require(((android.view.ViewGroup)activity.findViewById(R.id.gamepad_buttons_container)).getChildCount()
                ==GamepadButtons.BUTTON_IDS.length,"every pad button has an action to choose");
            capture("ui-gamepad-buttons.png");
            runOnMainSync(()->activity.onBackPressed());waitForIdleSync();
            require(activity.findViewById(R.id.gamepad2_buttons)!=null,"back from the buttons returns to the gamepads screen");
            runOnMainSync(()->activity.onBackPressed());waitForIdleSync();
            runOnMainSync(()->activity.onBackPressed());waitForIdleSync();
            /* Display, saved as it is chosen with no Save button, and Screen
             * adjustment underneath it. */
            runOnMainSync(()->activity.findViewById(R.id.screen_settings_button).performClick());waitForIdleSync();
            require(activity.findViewById(R.id.orientation_group)!=null&&activity.findViewById(R.id.fps_cap_group)!=null,
                "display settings list their choices");
            capture("ui-display.png");
            runOnMainSync(()->activity.findViewById(R.id.screen_adjustment_button).performClick());waitForIdleSync();
            require(activity.findViewById(R.id.reset_adjust_button)!=null,"screen adjustment opens from display settings");
            capture("ui-screen-adjustment.png");
            runOnMainSync(()->activity.onBackPressed());waitForIdleSync();
            runOnMainSync(()->activity.onBackPressed());waitForIdleSync();
            /* Data is a menu now, so saves live one level down -- and back has
             * to return to that menu rather than all the way out. */
            runOnMainSync(()->activity.findViewById(R.id.data_button).performClick());waitForIdleSync();
            require(activity.findViewById(R.id.game_data_button)!=null
                &&activity.findViewById(R.id.game_saves_button)!=null
                &&activity.findViewById(R.id.launcher_settings_button)!=null,"data menu lists its screens");
            runOnMainSync(()->activity.findViewById(R.id.launcher_settings_button).performClick());waitForIdleSync();
            require(activity.findViewById(R.id.export_launcher_button)!=null,"launcher settings can be exported");
            runOnMainSync(()->activity.onBackPressed());waitForIdleSync();
            require(activity.findViewById(R.id.game_saves_button)!=null,"back from a sub-screen returns to the data menu");
            runOnMainSync(()->activity.findViewById(R.id.game_saves_button).performClick());waitForIdleSync();
            require(activity.findViewById(R.id.import_saves_button)!=null&&activity.findViewById(R.id.export_saves_button)!=null,"save management actions visible");
            capture("ui-data-saves.png");
            runOnMainSync(()->activity.onBackPressed());waitForIdleSync();
            runOnMainSync(()->activity.onBackPressed());waitForIdleSync();
            runOnMainSync(()->activity.findViewById(R.id.faq_button).performClick());waitForIdleSync();
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
