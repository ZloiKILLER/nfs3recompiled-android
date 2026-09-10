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
    static RectF bounds(TouchControlsOverlay v,String action,String which)throws Exception{
        float scale=(float)field(v,"scale");
        for(Object c:(Iterable<?>)field(v,"controls"))if(action.equals(field(c,"action"))){
            RectF b=new RectF((RectF)field(c,which));return new RectF(b.left*scale,b.top*scale,b.right*scale,b.bottom*scale);
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
                check(Math.abs(brakePair.right-(1280-20*scale-5))<.1f,"brake occupies the outer pedal position");
                check(Math.abs(gas.width()-(62*scale+8))<.1f,"gas is eight physical pixels wider");
                check(Math.abs((brakePair.left-gas.right)-(12*scale+19))<.1f,"pedal gap includes twelve more physical pixels");
                RectF leftPair=bounds(view,"steer_left","box"),rightPair=bounds(view,"steer_right","box");
                check(Math.abs((rightPair.left-leftPair.right)-(8*scale+26))<.1f,"steering gap includes twelve more physical pixels");
                check(Math.abs((gearUp.left-gearDown.right)-10*scale)<.1f,"gear buttons use the horn/spikes gap");
                check(Math.abs((gearDown.left+gearUp.right)/2-(leftPair.left+rightPair.right)/2)<.1f,"gear buttons centered over steering");
                RectF horn=bounds(view,"horn","box"),spikes=bounds(view,"spike_strip","box");
                RectF camera=bounds(view,"camera","box"),lookBehind=bounds(view,"look_behind","box");
                check(Math.abs(horn.width()-58*scale)<.1f&&Math.abs(horn.height()-48*scale)<.1f,"utility buttons use the layout-toggle size");
                for(String action:new String[]{"pause","headlights","recover","spike_strip","camera","look_behind"}) {
                    RectF utility=bounds(view,action,"box");
                    check(Math.abs(utility.width()-horn.width())<.1f&&Math.abs(utility.height()-horn.height())<.1f,action+" matches the utility button size");
                }
                check(Math.abs(bounds(view,"recover","box").left-bounds(view,"headlights","box").right-18*scale)<.1f,"lights and recover use the auxiliary gap");
                check(Math.abs((spikes.left-horn.right)-18*scale)<.1f,"horn and spikes use the wider gap");
                check(Math.abs((lookBehind.left-camera.right)-18*scale)<.1f,"camera and look-behind use the wider gap");
                check(Math.abs(gearDown.top-254)<.1f,"gear row moves down thirty physical pixels");
                check(horn.bottom<=gearDown.top&&gearDown.bottom<leftPair.top,"raised actions, gears and steering form separate rows");
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
                prefs.edit().putBoolean(GamePreferences.TOUCH_SEPARATE,false).apply();view.refreshSettings();
                check(Math.abs(bounds(view,"pause","box").centerX()-640)<1,"reset separate layout preserves shared layout");
                // Mirroring must also move custom positions, and be reversible.
                GamePreferences.setTouchLayout(prefs,GamePreferences.TOUCH_LAYOUT_MIRRORED);
                view.refreshSettings();
                check(Math.abs(bounds(view,"pause","box").centerX()-640)<1,"center stays centered when mirrored");
                check(bounds(view,"steer_left","box").centerX()>640,"steering moves right");
                RectF mirroredBrake=bounds(view,"brake","box"),mirroredGas=bounds(view,"accelerate","box");
                check(mirroredBrake.left<mirroredGas.left,"mirrored brake remains the outer pedal");
                GamePreferences.setTouchLayout(prefs,GamePreferences.TOUCH_LAYOUT_STANDARD);
                view.refreshSettings();
                check(bounds(view,"steer_left","box").centerX()<640,"steering moves left again");
                check(bounds(view,"steering","box")==null&&bounds(view,"mode","box")==null,"shared layout has no stick or mode switch");
                RectF sharedLeft=bounds(view,"steer_left","box");
                view.setMenuMode(false);
                check(sharedLeft.equals(bounds(view,"steer_left","box"))&&bounds(view,"confirm","box")!=null,"shared controls unchanged in race");
                prefs.edit().putBoolean(GamePreferences.TOUCH_SEPARATE,true).apply();
                view.setEditing(false,null);view.setMenuMode(true);RectF pad=bounds(view,"steering","box");
                check(bounds(view,"keyboard","box")!=null&&bounds(view,"headlights","box")==null&&bounds(view,"recover","box")==null,"menu replaces race utilities with keyboard");
                touch(view,0,pad.centerX(),pad.top+pad.height()*.1f);
                Map<?,?> held=(Map<?,?>)field(field(view,"keys"),"held");
                check(held.size()==1&&held.containsKey(android.view.KeyEvent.KEYCODE_DPAD_UP),"D-pad up sends only up");
                touch(view,2,pad.right-pad.width()*.1f,pad.centerY());
                check(held.size()==1&&held.containsKey(android.view.KeyEvent.KEYCODE_DPAD_RIGHT),"D-pad direction change releases up");
                view.releaseAll();
                prefs.edit().putBoolean(GamePreferences.TOUCH_AUTO_HIDE,true).putBoolean(GamePreferences.TOUCH_HIDE_FULL,true)
                    .putInt(GamePreferences.TOUCH_HIDE_SECONDS,1).apply();
                idle[0]=new TouchControlsOverlay(test.getTargetContext(),false,(k,down)->{});idle[0].layout(0,0,1280,680);idle[0].setMenuMode(false);
                RectF brake=bounds(idle[0],"brake","box");touch(idle[0],0,brake.centerX(),brake.centerY());
                idle[0].resumeIdleTimer();
                check(GamePreferences.gamepadMappingEnvironment(prefs).contains("north=unknown"),"unassigned button overrides native fallback");
                prefs.edit().putString(GamePreferences.physicalKey("gear_up"),"north").apply();
                check(GamePreferences.gamepadMappingEnvironment(prefs).contains("north=a"),"gear mapping exported");
                prefs.edit().putString(GamePreferences.physicalKey("spike_strip"),"right_stick").apply();
                check(GamePreferences.gamepadMappingEnvironment(prefs).contains("right_stick=s"),"spike strip mapping exported");
                String defaults=GamePreferences.gamepadMappingEnvironment(prefs);
                check(defaults.contains("left_stick_left=left")&&defaults.contains("right_trigger=up")&&defaults.contains("left_trigger=down"),"sticks and triggers exported");
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
