package dev.nfs3hp.port;

import android.app.Instrumentation;
import android.content.SharedPreferences;
import android.graphics.RectF;
import android.view.MotionEvent;
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
        Throwable[] failure={null};TouchControlsOverlay[] idle={null};
        try{
            test.runOnMainSync(()->{try{
                // Isolated test application; preserve its full settings after the checks.
                prefs.edit().clear().commit();
                ArrayList<String> events=new ArrayList<>();
                TouchMouseInput mouse=new TouchMouseInput(test.getTargetContext(),(button,action,x,y,relative)->events.add(action+":"+button));
                mouse(mouse,MotionEvent.ACTION_DOWN,100,100,100);mouse(mouse,MotionEvent.ACTION_MOVE,140,230,100);
                mouse(mouse,MotionEvent.ACTION_MOVE,180,100,100);mouse(mouse,MotionEvent.ACTION_UP,200,100,100);
                check(!events.contains("0:1"),"drag out and back must not click");events.clear();
                mouse(mouse,MotionEvent.ACTION_DOWN,100,100,100);mouse(mouse,MotionEvent.ACTION_UP,170,100,100);
                check(events.contains("0:1"),"short stationary tap clicks");mouse.cancel();
                check(events.contains("1:0"),"cancel releases pending mouse button");events.clear();
                mouse(mouse,MotionEvent.ACTION_DOWN,100,100,100);mouse(mouse,MotionEvent.ACTION_UP,600,100,100);
                check(events.isEmpty(),"long hold must not click");
                mouse(mouse,MotionEvent.ACTION_DOWN,100,100,100);mouse(mouse,MotionEvent.ACTION_CANCEL,140,100,100);
                mouse(mouse,MotionEvent.ACTION_UP,160,100,100);check(events.isEmpty(),"cancelled gesture must not click");

                TouchControlsOverlay view=new TouchControlsOverlay(test.getTargetContext(),true);view.layout(0,0,1280,680);
                check(bounds(view,"gear_up","box")==null,"gears default hidden");
                prefs.edit().putBoolean(GamePreferences.TOUCH_GEARS,true).apply();view.refreshSettings();
                check(bounds(view,"gear_up","box")!=null&&bounds(view,"gear_down","box")!=null,"optional gears visible");
                RectF gas=bounds(view,"accelerate","box");float scale=(float)field(view,"scale");
                check(Math.abs(gas.right-(1280-20*scale-5))<.1f,"gas shifted exactly five physical pixels left");
                RectF brakePair=bounds(view,"brake","box");
                check(Math.abs((gas.left-brakePair.right)-(12*scale+7))<.1f,"pedal gap includes twelve extra physical pixels");
                RectF leftPair=bounds(view,"steer_left","box"),rightPair=bounds(view,"steer_right","box");
                check(Math.abs((rightPair.left-leftPair.right)-(8*scale+14))<.1f,"steering gap includes fourteen extra physical pixels");
                view.setEditing(true,null);RectF lights=bounds(view,"headlights","box");
                touch(view,0,lights.centerX(),lights.centerY());touch(view,2,640,240);touch(view,1,640,240);
                check(view.selectedAction().equals("headlights"),"editor selected control");
                view.resizeSelected(150,80);
                RectF visible=bounds(view,"headlights","box"),hit=bounds(view,"headlights","hitBox");
                check(visible.width()>hit.width(),"visual and hit sizes independent");
                view.setMenuMode(true);RectF shared=bounds(view,"headlights","box");
                check(Math.abs(shared.centerX()-640)<1,"shared mode reuses saved position");
                prefs.edit().putBoolean(GamePreferences.TOUCH_SEPARATE,true).apply();view.refreshSettings();
                check(Math.abs(bounds(view,"headlights","box").centerX()-640)>20,"separate mode has its own positions");
                view.setMenuMode(false);lights=bounds(view,"headlights","box");
                touch(view,0,lights.centerX(),lights.centerY());touch(view,2,600,200);touch(view,1,600,200);
                view.setMenuMode(true);check(Math.abs(bounds(view,"headlights","box").centerX()-600)>20,"race position does not modify menu");
                view.setMenuMode(false);check(Math.abs(bounds(view,"headlights","box").centerX()-600)<1,"race position survives mode switch");
                view.resetLayout();check(Math.abs(bounds(view,"headlights","box").centerX()-600)>20,"reset restores layout");
                prefs.edit().putBoolean(GamePreferences.TOUCH_SEPARATE,false).apply();view.refreshSettings();
                check(Math.abs(bounds(view,"headlights","box").centerX()-640)<1,"reset separate layout preserves shared layout");
                // Mirroring must also move custom positions, and be reversible.
                GamePreferences.setTouchLayout(prefs,GamePreferences.TOUCH_LAYOUT_MIRRORED);
                view.refreshSettings();
                check(Math.abs(bounds(view,"headlights","box").centerX()-640)<1,"center stays centered when mirrored");
                check(bounds(view,"steer_left","box").centerX()>640,"steering moves right");
                GamePreferences.setTouchLayout(prefs,GamePreferences.TOUCH_LAYOUT_STANDARD);
                view.refreshSettings();
                check(bounds(view,"steer_left","box").centerX()<640,"steering moves left again");
                check(bounds(view,"steering","box")==null&&bounds(view,"mode","box")==null,"shared layout has no stick or mode switch");
                RectF sharedLeft=bounds(view,"steer_left","box");
                view.setMenuMode(false);
                check(sharedLeft.equals(bounds(view,"steer_left","box"))&&bounds(view,"confirm","box")!=null,"shared controls unchanged in race");
                prefs.edit().putBoolean(GamePreferences.TOUCH_SEPARATE,true).apply();
                view.setEditing(false,null);view.setMenuMode(true);RectF pad=bounds(view,"steering","box");
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
