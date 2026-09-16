package dev.nfs3hp.port;

import android.app.Instrumentation;
import android.content.SharedPreferences;
import android.graphics.RectF;
import android.view.MotionEvent;
import android.view.View;
import java.lang.reflect.Field;
import java.util.*;

final class TouchRefinementChecks {
    static Object field(Object o,String name)throws Exception{Field f=o.getClass().getDeclaredField(name);f.setAccessible(true);return f.get(o);}
    static void check(boolean ok,String message){if(!ok)throw new AssertionError(message);}
    /* Whether a length in pixels is a whole number of grid cells that many pixels wide. */
    static boolean onGrid(float value,float cell){return Math.abs(value/cell-Math.round(value/cell))<.01f;}
    static RectF bounds(TouchControlsOverlay v,String action,String which)throws Exception{
        float scale=(float)field(v,"scale");
        for(Object c:(Iterable<?>)field(v,"controls"))if(action.equals(field(c,"action"))){
            float ox=(float)field(v,"originX"),oy=(float)field(v,"originY");
            RectF b=new RectF((RectF)field(c,which));return new RectF(ox+b.left*scale,oy+b.top*scale,ox+b.right*scale,oy+b.bottom*scale);
        }
        return null;
    }
    static void touch(TouchControlsOverlay v,int action,float x,float y){
        MotionEvent e=MotionEvent.obtain(1,2,action,x,y,0);v.onTouchEvent(e);e.recycle();
    }
    static void mouse(TouchMouseInput m,int action,long time,float x,float y){
        MotionEvent e=MotionEvent.obtain(100,time,action,x,y,0);m.onTouch(e);e.recycle();
    }
    static void run(Instrumentation test)throws Exception{
        SharedPreferences prefs=GamePreferences.get(test.getTargetContext());Map<String,?> saved=prefs.getAll();
        Throwable[] failure={null};TouchControlsOverlay[] idle={null};TouchMouseInput[] cursor={null};
        ArrayList<String> events=new ArrayList<>();
        try{
            test.runOnMainSync(()->{try{
                // Isolated test application; preserve its full settings after the checks.
                prefs.edit().clear().commit();
                TouchMouseInput mouse=new TouchMouseInput(new View(test.getTargetContext()),(button,action,x,y,relative)->events.add(action+":"+button));
                cursor[0]=mouse;
                mouse(mouse,MotionEvent.ACTION_DOWN,100,100,100);mouse(mouse,MotionEvent.ACTION_MOVE,140,230,100);
                mouse(mouse,MotionEvent.ACTION_MOVE,180,100,100);mouse(mouse,MotionEvent.ACTION_UP,200,100,100);
                check(!events.contains("0:1"),"drag out and back must not click");events.clear();
                mouse(mouse,MotionEvent.ACTION_DOWN,100,100,100);mouse(mouse,MotionEvent.ACTION_UP,170,100,100);
                check(events.contains("0:1"),"short stationary tap clicks");mouse.cancel();
                check(events.contains("1:0"),"cancel releases pending mouse button");events.clear();
                mouse(mouse,MotionEvent.ACTION_DOWN,100,100,100);mouse(mouse,MotionEvent.ACTION_UP,600,100,100);
                check(events.isEmpty(),"a hold past the tap window must not click");
                mouse(mouse,MotionEvent.ACTION_DOWN,100,100,100);mouse(mouse,MotionEvent.ACTION_CANCEL,140,100,100);
                mouse(mouse,MotionEvent.ACTION_UP,160,100,100);check(events.isEmpty(),"cancelled gesture must not click");

                TouchControlsOverlay view=new TouchControlsOverlay(test.getTargetContext(),true);view.layout(0,0,1280,680);
                RectF gearUp=bounds(view,"gear_up","box"),gearDown=bounds(view,"gear_down","box");
                check(gearUp!=null&&gearDown!=null,"gears always visible");
                RectF gas=bounds(view,"accelerate","box"),brakePair=bounds(view,"brake","box");float scale=(float)field(view,"scale");
                /* Default positions stand on the layout editor's grid.  The pedals go
                 * onto it as one, so their hand-tuned shapes and gap stay exact and
                 * the brake stays within half a cell of its old place by the edge. */
                float cell=TouchLayout.GRID*scale;
                check(onGrid(brakePair.centerX(),cell)&&onGrid(brakePair.centerY(),cell),"brake stands on the grid");
                check(Math.abs(brakePair.right-(1280-20*scale-5))<=cell/2+.1f,"brake occupies the outer pedal position");
                check(Math.abs(gas.width()-(62*scale+8))<.1f,"gas is eight physical pixels wider");
                check(Math.abs((brakePair.left-gas.right)-(12*scale+19))<.1f,"pedal gap includes twelve more physical pixels");
                check(Math.abs(gas.bottom-brakePair.bottom)<.1f,"pedals stand level");
                /* The shared layout steers with the D-pad, stacks horn and spikes over
                 * it and the gears over those in the same two columns, keeps the
                 * keyboard between pause and OK, and shows lights and recovery
                 * whatever mode the overlay is in. */
                RectF dpad=bounds(view,"steering","box");
                check(dpad!=null&&bounds(view,"steer_left","box")==null&&bounds(view,"steer_right","box")==null,"shared layout steers with the D-pad");
                RectF horn=bounds(view,"horn","box"),spikes=bounds(view,"spike_strip","box");
                check(Math.abs(gearDown.centerX()-horn.centerX())<.1f&&Math.abs(gearUp.centerX()-spikes.centerX())<.1f,"gears stand over horn and spikes");
                check(gearDown.bottom<=horn.top&&horn.bottom<=dpad.top,"gears, then horn and spikes, then the D-pad");
                RectF pause=bounds(view,"pause","box"),keyboardKey=bounds(view,"keyboard","box"),confirm=bounds(view,"confirm","box");
                check(keyboardKey!=null&&keyboardKey.left>pause.right&&keyboardKey.right<confirm.left&&Math.abs(keyboardKey.top-pause.top)<.1f
                    &&Math.abs(keyboardKey.width()-confirm.width())<.1f&&Math.abs(keyboardKey.height()-confirm.height())<.1f,
                    "keyboard sits between pause and OK, sized like OK");
                RectF camera=bounds(view,"camera","box"),lookBehind=bounds(view,"look_behind","box");
                /* Every control but the D-pad and the pedals has the one small size,
                 * stands on the grid and sits whole cells from its neighbours. */
                check(Math.abs(horn.width()-58*scale)<.1f&&Math.abs(horn.height()-48*scale)<.1f,"small buttons are 58 x 48 at 100%");
                for(String action:new String[]{"pause","keyboard","confirm","back","headlights","recover","horn","spike_strip",
                                               "camera","look_behind","handbrake","gear_down","gear_up"}) {
                    RectF small=bounds(view,action,"box");
                    check(Math.abs(small.width()-horn.width())<.1f&&Math.abs(small.height()-horn.height())<.1f,action+" is the small button size");
                    check(onGrid(small.centerX(),cell)&&onGrid(small.centerY(),cell),action+" stands on the grid");
                }
                check(onGrid(spikes.left-horn.left,cell)&&Math.abs(spikes.top-horn.top)<.1f,"horn and spikes share a row, whole cells apart");
                check(onGrid(lookBehind.left-camera.left,cell)&&Math.abs(lookBehind.top-camera.top)<.1f,"camera and look-behind share a row, whole cells apart");
                check(bounds(view,"headlights","box")!=null&&bounds(view,"recover","box")!=null,"shared layout shows lights and recovery");
                prefs.edit().putInt(GamePreferences.TOUCH_SIZE,80).commit();view.refreshSettings();
                check(Math.abs(bounds(view,"camera","box").width()-58*.8f*scale)<.1f&&Math.abs(bounds(view,"gear_up","box").height()-48*.8f*scale)<.1f,
                    "small buttons follow the size setting");
                prefs.edit().remove(GamePreferences.TOUCH_SIZE).commit();view.refreshSettings();
                // Exact positions from here on; snapping a dragged control is checked on its own below.
                prefs.edit().putBoolean(GamePreferences.TOUCH_SNAP,false).commit();
                view.setEditing(true,null);RectF lights=bounds(view,"pause","box");
                touch(view,0,lights.centerX(),lights.centerY());touch(view,2,640,240);touch(view,1,640,240);
                check(view.selectedAction().equals("pause"),"editor selected control");
                view.resizeSelected(150,80);
                RectF visible=bounds(view,"pause","box"),hit=bounds(view,"pause","hitBox");
                check(visible.width()>hit.width(),"visual and hit sizes independent");
                view.setMenuMode(true);RectF shared=bounds(view,"pause","box");
                check(Math.abs(shared.centerX()-640)<1,"shared mode reuses saved position");
                prefs.edit().putBoolean(GamePreferences.TOUCH_SEPARATE,true).apply();view.refreshSettings();
                check(Math.abs(bounds(view,"pause","box").centerX()-640)>20,"separate mode has its own positions");
                view.setMenuMode(false);lights=bounds(view,"pause","box");
                touch(view,0,lights.centerX(),lights.centerY());touch(view,2,600,200);touch(view,1,600,200);
                view.setMenuMode(true);check(Math.abs(bounds(view,"pause","box").centerX()-600)>20,"race position does not modify menu");
                view.setMenuMode(false);check(Math.abs(bounds(view,"pause","box").centerX()-600)<1,"race position survives mode switch");
                view.resetLayout();check(Math.abs(bounds(view,"pause","box").centerX()-600)>20,"reset restores layout");
                /* With Snap to grid on, a dragged control's centre lands on a grid point
                 * while it is being dragged. */
                prefs.edit().putBoolean(GamePreferences.TOUCH_SNAP,true).commit();
                RectF recover=bounds(view,"recover","box");
                touch(view,0,recover.centerX(),recover.centerY());
                touch(view,2,recover.centerX()-3*scale,recover.centerY()+13*scale);
                RectF snapped=bounds(view,"recover","box");
                touch(view,1,recover.centerX()-3*scale,recover.centerY()+13*scale);
                check(onGrid(snapped.centerX(),cell)&&onGrid(snapped.centerY(),cell)
                    &&Math.abs(snapped.centerX()-recover.centerX())<.1f&&Math.abs(snapped.centerY()-recover.centerY()-16*scale)<.1f,
                    "a dragged control lands on the grid");
                view.resetLayout();
                prefs.edit().putBoolean(GamePreferences.TOUCH_SEPARATE,false).apply();view.refreshSettings();
                check(Math.abs(bounds(view,"pause","box").centerX()-640)<1,"reset separate layout preserves shared layout");
                // Mirroring must also move custom positions, and be reversible.
                GamePreferences.setTouchLayout(prefs,GamePreferences.TOUCH_LAYOUT_MIRRORED);
                view.refreshSettings();
                check(Math.abs(bounds(view,"pause","box").centerX()-640)<1,"center stays centered when mirrored");
                check(bounds(view,"steering","box").centerX()>640,"steering moves right");
                RectF mirroredBrake=bounds(view,"brake","box"),mirroredGas=bounds(view,"accelerate","box");
                check(mirroredBrake.left<mirroredGas.left,"mirrored brake remains the outer pedal");
                GamePreferences.setTouchLayout(prefs,GamePreferences.TOUCH_LAYOUT_STANDARD);
                view.refreshSettings();
                check(bounds(view,"steering","box").centerX()<640,"steering moves left again");
                check(bounds(view,"mode","box")==null,"shared layout has no mode switch");
                view.setEditing(false,null);view.setMenuMode(true);
                RectF sharedPad=bounds(view,"steering","box");
                check(bounds(view,"keyboard","box")!=null&&bounds(view,"headlights","box")!=null&&bounds(view,"recover","box")!=null,
                    "shared layout in the game's menus keeps lights and recovery beside the keyboard");
                view.setMenuMode(false);
                check(sharedPad.equals(bounds(view,"steering","box"))&&bounds(view,"confirm","box")!=null&&bounds(view,"keyboard","box")!=null,
                    "shared controls unchanged in race");
                Map<?,?> sharedHeld=(Map<?,?>)field(field(view,"keys"),"held");
                touch(view,0,sharedPad.left+sharedPad.width()*.1f,sharedPad.centerY());
                check(sharedHeld.size()==1&&sharedHeld.containsKey(android.view.KeyEvent.KEYCODE_DPAD_LEFT),"shared D-pad left steers left");
                /* Up and down move through menus on the pedals' keys, but past the
                 * pedals themselves: a D-pad arm never accelerates or brakes. */
                touch(view,2,sharedPad.centerX(),sharedPad.top+sharedPad.height()*.1f);
                check(sharedHeld.size()==1&&sharedHeld.containsKey(TouchControlsOverlay.MENU_KEY|android.view.KeyEvent.KEYCODE_DPAD_UP)
                    &&!sharedHeld.containsKey(android.view.KeyEvent.KEYCODE_DPAD_UP),"shared D-pad up moves through menus, never as the pedal");
                touch(view,1,sharedPad.centerX(),sharedPad.top+sharedPad.height()*.1f);
                view.releaseAll();
                prefs.edit().putBoolean(GamePreferences.TOUCH_SEPARATE,true).apply();
                view.setEditing(false,null);view.setMenuMode(true);RectF pad=bounds(view,"steering","box");
                check(bounds(view,"keyboard","box")!=null&&bounds(view,"headlights","box")==null&&bounds(view,"recover","box")==null,"menu replaces race utilities with keyboard");
                touch(view,0,pad.centerX(),pad.top+pad.height()*.1f);
                Map<?,?> held=(Map<?,?>)field(field(view,"keys"),"held");
                check(held.size()==1&&held.containsKey(TouchControlsOverlay.MENU_KEY|android.view.KeyEvent.KEYCODE_DPAD_UP),"D-pad up sends only the menu's up");
                touch(view,2,pad.right-pad.width()*.1f,pad.centerY());
                check(held.size()==1&&held.containsKey(android.view.KeyEvent.KEYCODE_DPAD_RIGHT),"D-pad direction change releases up");
                view.releaseAll();
                prefs.edit().putBoolean(GamePreferences.TOUCH_AUTO_HIDE,true).putBoolean(GamePreferences.TOUCH_HIDE_FULL,true)
                    .putInt(GamePreferences.TOUCH_HIDE_SECONDS,1).apply();
                idle[0]=new TouchControlsOverlay(test.getTargetContext(),false,(k,down)->{});idle[0].layout(0,0,1280,680);idle[0].setMenuMode(false);
                RectF brake=bounds(idle[0],"brake","box");touch(idle[0],0,brake.centerX(),brake.centerY());
                idle[0].resumeIdleTimer();
                /* Gamepad slots: pinned pads go first, automatic slots share the
                 * rest in connection order, and a pinned pad that is missing
                 * leaves its slot empty rather than taking someone else's. */
                java.util.List<GamepadSlots.Pad> pads=java.util.Arrays.asList(
                    new GamepadSlots.Pad(5,"alpha","Pad A"),new GamepadSlots.Pad(9,"beta","Pad B"));
                GamepadSlots.Pad[] auto=GamepadSlots.resolve(new String[]{"",""},pads);
                check(auto[0].descriptor.equals("alpha")&&auto[1].descriptor.equals("beta"),"automatic slots follow connection order");
                GamepadSlots.Pad[] swapped=GamepadSlots.resolve(new String[]{"beta",""},pads);
                check(swapped[0].descriptor.equals("beta")&&swapped[1].descriptor.equals("alpha"),"a pinned pad is placed before automatic slots fill");
                GamepadSlots.Pad[] missing=GamepadSlots.resolve(new String[]{"","gamma"},pads);
                check(missing[1]==null&&missing[0].descriptor.equals("alpha"),"a missing pinned pad leaves its slot empty");
                GamepadSlots.Pad[] twice=GamepadSlots.resolve(new String[]{"alpha","alpha"},pads);
                check(twice[0].descriptor.equals("alpha")&&twice[1]==null,"one pad never serves both slots");
                String[][] kept={{GamepadSlots.assigned(prefs,0),GamepadSlots.assignedName(prefs,0)},
                                 {GamepadSlots.assigned(prefs,1),GamepadSlots.assignedName(prefs,1)}};
                GamepadSlots.assign(prefs,1,"alpha","Pad A");
                GamepadSlots.assign(prefs,0,"alpha","Pad A");
                check(GamepadSlots.assigned(prefs,0).equals("alpha")&&GamepadSlots.assigned(prefs,1).isEmpty(),"pinning a pad to one slot frees it from the other");
                for(int slot=0;slot<GamepadSlots.COUNT;slot++){
                    if(kept[slot][0].isEmpty())GamepadSlots.useAutomatic(prefs,slot);
                    else GamepadSlots.assign(prefs,slot,kept[slot][0],kept[slot][1]);
                }
                /* Control profile: both slots described exactly as the game
                 * enumerates them, the three tables written, devices past the
                 * slots left alone, and what the player tuned in the game kept.
                 * The expected records are the bytes the game itself produced:
                 * its keyboard defaults, and the tables it generated for a
                 * two-axis pad in 0.72 and 0.73. */
                byte[] config=new byte[ControlProfile.FILE_SIZE];
                java.nio.ByteBuffer file=java.nio.ByteBuffer.wrap(config).order(java.nio.ByteOrder.LITTLE_ENDIAN);
                file.putInt(0,ControlProfile.FILE_SIZE);
                file.putInt(0xE30+2*0x88,0x5A5A5A5A);
                ControlProfile.apply(config,ControlProfile.Kind.GAMEPADS);
                check(file.getInt(0x16C4)==2&&file.getInt(0x16C8)==2,"profile describes two devices");
                for(int slot=0;slot<2;slot++){
                    int base=0xE30+slot*0x88;
                    check(file.getInt(base)==9&&file.getInt(base+0x38)==0&&file.getInt(base+0x3C)==0&&file.getInt(base+0x40)==3,
                        "slot record: joystick with force feedback, two axes, no buttons or hats");
                    check(new String(config,base+4,13,java.nio.charset.StandardCharsets.US_ASCII).equals("NFS Gamepad "+(slot+1))
                        &&config[base+4+13]==0,"slot record carries the enumerated name");
                    check(file.getInt(base+0x44)==75&&file.getInt(base+0x4C)==100&&file.getInt(base+0x54)==100,"a new record gets the game's defaults");
                }
                check(file.getInt(0xE30+2*0x88)==0x5A5A5A5A,"records past the slots are untouched");
                int[] one={0x0080FF01,0x007F0001,0x017F0001,0x0180FF01,0x20003904,0x41001E04,0x5A002C04,
                    0x43002E04,0x48002304,0x42003004,0x52001304,0x53001F04,0x4C002604};
                int[] two={0x1080FF01,0x107F0001,0x117F0001,0x1180FF01,0x44002004,0x46002104,0x47002204,
                    0x51001004,0x57001104,0x45001204,0x58002D04,0x50001904,0x59001504};
                int[][] gamepads={one,one,two};
                int[][] keyboard={
                    {0x00004D04,0x00004B04,0x00004804,0x00005004,0x20003904,0x41001E04,0x5A002C04,
                     0x43002E04,0x48002304,0x42003004,0x52001304,0x53001F04,0x4C002604},
                    {0x00004D04,0x00004B04,0x00004804,0x00005004,0x00005204,0x00004904,0x00005104,
                     0x4B002504,0x4D003204,0x00004704,0x00004F04,0x50001904,0x4C002604},
                    {0x47002204,0x44002004,0x52001304,0x46002104,0x20003904,0x41001E04,0x5A002C04,
                     0x51001004,0x57001104,0x45001204,0x58002D04,0x53001F04,0x59001504}};
                for(int t=0;t<3;t++)for(int f=0;f<ControlProfile.FUNCTIONS;f++)
                    check(file.getInt(ControlProfile.TABLES[t]+4*f)==gamepads[t][f],"gamepad table "+t+" function "+f);
                file.putInt(0xE30+0x44,40);file.putInt(0xE30+0x48,7);
                ControlProfile.apply(config,ControlProfile.Kind.KEYBOARD);
                check(file.getInt(0xE30+0x44)==40&&file.getInt(0xE30+0x48)==7,"force feedback strength and axis settings survive a rewrite");
                for(int t=0;t<3;t++)for(int f=0;f<ControlProfile.FUNCTIONS;f++)
                    check(file.getInt(ControlProfile.TABLES[t]+4*f)==keyboard[t][f],"keyboard table "+t+" function "+f);
                boolean refused=false;
                try{ControlProfile.apply(new byte[100],ControlProfile.Kind.GAMEPADS);}catch(IllegalArgumentException expected){refused=true;}
                check(refused,"a file that is not config.dat is refused");
                /* Pad buttons: both pads share the actions, each sends its own
                 * player's key -- the key the profile binds for that player. */
                for(int function=ControlProfile.HANDBRAKE;function<ControlProfile.FUNCTIONS;function++){
                    check(ControlProfile.playerKey(0,function).record()==one[function],"player 1 key matches its table");
                    check(ControlProfile.playerKey(1,function).record()==two[function],"player 2 key matches its table");
                }
                check(GamepadButtons.environmentValue(0,"handbrake").equals("Space")
                    &&GamepadButtons.environmentValue(1,"handbrake").equals("D"),"handbrake is each player's own key");
                check(GamepadButtons.environmentValue(1,"confirm").equals("Return")
                    &&GamepadButtons.environmentValue(1,"menu_up").equals("Up"),"menu keys are the same for both pads");
                check(GamepadButtons.environmentValue(0,"accelerate").equals("axis:accelerate")
                    &&GamepadButtons.environmentValue(0,GamepadButtons.NONE).isEmpty(),"axis actions and no action");
                check(GamepadButtons.DEFAULT_ACTIONS.length==GamepadButtons.BUTTON_IDS.length,"every button has a default");
                check(GamepadButtons.environmentName(1,"dpad_up").equals("NFS_GAMEPAD2_DPAD_UP"),"environment names match the native table");
            }catch(Throwable e){failure[0]=e;}});
            if(failure[0]!=null)throw new AssertionError(failure[0]);
            Thread.sleep(1500);
            test.runOnMainSync(()->{try{
                check(idle[0].getAlpha()==1,"holding a control prevents hiding");
                RectF brake=bounds(idle[0],"brake","box");touch(idle[0],1,brake.centerX(),brake.centerY());
            }catch(Throwable e){failure[0]=e;}});
            Thread.sleep(1500);
            test.runOnMainSync(()->{try{
                check(idle[0].getAlpha()<.01f,"fully hidden after configured delay");
                RectF brake=bounds(idle[0],"brake","box");touch(idle[0],0,brake.centerX(),brake.centerY());
                check(idle[0].getAlpha()==1&&!((Map<?,?>)field(field(idle[0],"keys"),"held")).isEmpty(),"first touch on hidden control activates it immediately");
                touch(idle[0],1,brake.centerX(),brake.centerY());
                prefs.edit().putBoolean(GamePreferences.TOUCH_HIDE_FULL,false).apply();idle[0].resumeIdleTimer();
            }catch(Throwable e){failure[0]=e;}});
            Thread.sleep(1500);
            test.runOnMainSync(()->{if(Math.abs(idle[0].getAlpha()-.12f)>.02f)failure[0]=new AssertionError("silhouette opacity");});
            test.runOnMainSync(()->{
                MotionEvent e=MotionEvent.obtain(1,2,MotionEvent.ACTION_DOWN,640,300,0);
                idle[0].onScreenTouch(e);e.recycle();idle[0].resumeIdleTimer();
            });
            Thread.sleep(1500);
            test.runOnMainSync(()->{
                if(idle[0].getAlpha()!=1)failure[0]=new AssertionError("mouse gesture also prevents hiding");
                MotionEvent e=MotionEvent.obtain(1,2,MotionEvent.ACTION_UP,640,300,0);
                idle[0].onScreenTouch(e);e.recycle();
            });
            /* A press on a gamepad hides the controls at once; a resume leaves them
             * hidden, and only the next touch brings them back. */
            test.runOnMainSync(()->idle[0].hideForGamepad());
            Thread.sleep(400);
            test.runOnMainSync(()->{try{
                check(idle[0].getAlpha()<.01f,"a gamepad press hides the controls at once");
                idle[0].resumeIdleTimer();
                check(idle[0].getAlpha()<.01f,"hidden for a gamepad until the screen is touched");
                MotionEvent e=MotionEvent.obtain(1,2,MotionEvent.ACTION_DOWN,640,300,0);
                idle[0].onScreenTouch(e);e.recycle();
                check(idle[0].getAlpha()==1,"a touch brings the controls back");
                e=MotionEvent.obtain(1,2,MotionEvent.ACTION_UP,640,300,0);
                idle[0].onScreenTouch(e);e.recycle();
                /* A layout rebuilt under the running countdown -- new window insets
                 * right after the game starts -- starts it over instead of losing it. */
                idle[0].resumeIdleTimer();idle[0].layout(0,0,1280,690);
            }catch(Throwable e){failure[0]=e;}});
            Thread.sleep(1500);
            test.runOnMainSync(()->{if(failure[0]==null&&Math.abs(idle[0].getAlpha()-.12f)>.02f)failure[0]=new AssertionError("a rebuilt layout keeps the idle timer");});
            // The grab runs on the looper, so these need real time to elapse.
            test.runOnMainSync(()->{events.clear();mouse(cursor[0],MotionEvent.ACTION_DOWN,100,100,100);});
            Thread.sleep(600);
            test.runOnMainSync(()->{try{
                check(events.contains("0:1"),"a stationary hold grabs the button");
                mouse(cursor[0],MotionEvent.ACTION_MOVE,700,300,100);
                check(!events.contains("1:0"),"the button stays down while the finger drags");
                mouse(cursor[0],MotionEvent.ACTION_UP,800,300,100);
                check(events.contains("1:0"),"lifting after a drag releases the button");
                events.clear();mouse(cursor[0],MotionEvent.ACTION_DOWN,100,100,100);
                mouse(cursor[0],MotionEvent.ACTION_MOVE,140,300,100);
            }catch(Throwable e){failure[0]=e;}});
            Thread.sleep(600);
            test.runOnMainSync(()->{try{
                check(!events.contains("0:1"),"a moving finger never grabs");
                mouse(cursor[0],MotionEvent.ACTION_UP,160,300,100);cursor[0].cancel();
            }catch(Throwable e){failure[0]=e;}});
            if(failure[0]!=null)throw new AssertionError(failure[0]);
        }finally{
            test.runOnMainSync(()->{if(idle[0]!=null)idle[0].releaseAll();});
            SharedPreferences.Editor edit=prefs.edit().clear();
            for(Map.Entry<String,?> e:saved.entrySet()){
                Object v=e.getValue();String k=e.getKey();
                if(v instanceof Boolean)edit.putBoolean(k,(Boolean)v);else if(v instanceof Integer)edit.putInt(k,(Integer)v);
                else if(v instanceof Float)edit.putFloat(k,(Float)v);else if(v instanceof Long)edit.putLong(k,(Long)v);
                else if(v instanceof String)edit.putString(k,(String)v);
            }edit.commit();
        }
    }
}
