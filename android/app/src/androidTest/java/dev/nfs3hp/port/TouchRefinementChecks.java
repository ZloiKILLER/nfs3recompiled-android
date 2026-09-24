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
    static void deleteTree(java.io.File file){java.io.File[] children=file.listFiles();if(children!=null)for(java.io.File c:children)deleteTree(c);file.delete();}
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
    static void finger(TouchPointer p,int action,float x,float y){
        MotionEvent e=MotionEvent.obtain(100,200,action,x,y,0);p.onTouch(e);e.recycle();
    }
    static void finger(TouchpadPointer p,int action,float x,float y){
        MotionEvent e=MotionEvent.obtain(100,200,action,x,y,0);p.onTouch(e);e.recycle();
    }
    /* Two fingers at once, the given one acting. */
    static void fingers(TouchPointer p,int action,int index,float[] xs,float[] ys){
        MotionEvent.PointerProperties[] props=new MotionEvent.PointerProperties[xs.length];
        MotionEvent.PointerCoords[] coords=new MotionEvent.PointerCoords[xs.length];
        for(int i=0;i<xs.length;++i){
            props[i]=new MotionEvent.PointerProperties();props[i].id=i;
            props[i].toolType=MotionEvent.TOOL_TYPE_FINGER;
            coords[i]=new MotionEvent.PointerCoords();coords[i].x=xs[i];coords[i].y=ys[i];
        }
        MotionEvent e=MotionEvent.obtain(100,200,action|(index<<MotionEvent.ACTION_POINTER_INDEX_SHIFT),
            xs.length,props,coords,0,0,1,1,100,0,0,0);
        p.onTouch(e);e.recycle();
    }
    static void run(Instrumentation test)throws Exception{
        SharedPreferences prefs=GamePreferences.get(test.getTargetContext());Map<String,?> saved=prefs.getAll();
        Throwable[] failure={null};TouchControlsOverlay[] idle={null};
        ArrayList<String> events=new ArrayList<>();
        try{
            test.runOnMainSync(()->{try{
                // Isolated test application; preserve its full settings after the checks.
                prefs.edit().clear().commit();
                /* The pointer follows the finger, but only a tap clicks: a finger that
                 * lands points, lifted where it landed it clicks, resting it presses
                 * and drags, and one that wanders first presses nothing. */
                TouchPointer pointer=new TouchPointer(new android.view.View(test.getTargetContext()),
                    (action,x,y)->events.add(action+":"+(int)x+","+(int)y));
                finger(pointer,MotionEvent.ACTION_DOWN,230,140);
                check(events.equals(Arrays.asList("1:230,140")),"a finger that lands only points");
                events.clear();
                finger(pointer,MotionEvent.ACTION_UP,231,141);
                check(events.equals(Arrays.asList("0:231,141","2:231,141")),"lifted where it landed, it clicks");
                events.clear();
                finger(pointer,MotionEvent.ACTION_DOWN,50,60);pointer.hold();
                check(events.equals(Arrays.asList("1:50,60","0:50,60")),"a finger at rest presses");
                events.clear();
                finger(pointer,MotionEvent.ACTION_MOVE,400,300);finger(pointer,MotionEvent.ACTION_UP,410,305);
                check(events.equals(Arrays.asList("1:400,300","2:410,305")),"a pressing finger drags and lifts");
                events.clear();
                /* A swipe through a list, or the system's back gesture from the edge. */
                finger(pointer,MotionEvent.ACTION_DOWN,70,80);finger(pointer,MotionEvent.ACTION_MOVE,370,80);
                pointer.hold();finger(pointer,MotionEvent.ACTION_UP,380,80);
                check(events.equals(Arrays.asList("1:70,80","1:370,80","1:380,80")),"a swipe presses nothing");
                events.clear();
                /* Taken away mid-gesture: nothing clicks, and a held button is lifted
                 * where it last was, once. */
                finger(pointer,MotionEvent.ACTION_DOWN,70,80);finger(pointer,MotionEvent.ACTION_CANCEL,70,80);
                check(events.equals(Arrays.asList("1:70,80")),"a cancelled tap clicks nothing");
                events.clear();
                finger(pointer,MotionEvent.ACTION_DOWN,70,80);pointer.hold();events.clear();
                finger(pointer,MotionEvent.ACTION_CANCEL,70,80);
                check(events.equals(Arrays.asList("2:70,80")),"a cancelled hold lifts the button");
                events.clear();pointer.cancel();
                check(events.isEmpty(),"nothing left to lift afterwards");
                /* A second finger neither moves the pointer nor lifts it. */
                finger(pointer,MotionEvent.ACTION_DOWN,100,100);pointer.hold();events.clear();
                fingers(pointer,MotionEvent.ACTION_POINTER_DOWN,1,new float[]{100,500},new float[]{100,500});
                fingers(pointer,MotionEvent.ACTION_MOVE,0,new float[]{120,900},new float[]{130,900});
                fingers(pointer,MotionEvent.ACTION_POINTER_UP,1,new float[]{120,900},new float[]{130,900});
                check(events.equals(Arrays.asList("1:120,130")),"only the first finger drives the pointer");
                events.clear();finger(pointer,MotionEvent.ACTION_UP,120,130);
                check(events.equals(Arrays.asList("2:120,130")),"lifting the first finger releases");
                events.clear();
                /* The touchpad instead: a slide moves the cursor by as far as it
                 * went and presses nothing, a tap in place clicks, a finger
                 * resting in place grabs and drags. */
                ArrayList<String> pad=new ArrayList<>();
                TouchpadPointer touchpad=new TouchpadPointer(new android.view.View(test.getTargetContext()),
                    (dx,dy,buttons)->pad.add((int)dx+","+(int)dy+","+buttons));
                finger(touchpad,MotionEvent.ACTION_DOWN,100,100);finger(touchpad,MotionEvent.ACTION_MOVE,130,90);
                finger(touchpad,MotionEvent.ACTION_UP,130,90);
                check(pad.equals(Arrays.asList("30,-10,0")),"a touchpad slide moves by as far and presses nothing");
                pad.clear();
                finger(touchpad,MotionEvent.ACTION_DOWN,200,200);finger(touchpad,MotionEvent.ACTION_UP,200,200);
                check(pad.equals(Arrays.asList("0,0,1")),"a touchpad tap clicks");
                touchpad.cancel();check(pad.equals(Arrays.asList("0,0,1","0,0,0")),"and lets go");
                pad.clear();
                finger(touchpad,MotionEvent.ACTION_DOWN,50,50);touchpad.hold();
                finger(touchpad,MotionEvent.ACTION_MOVE,60,70);finger(touchpad,MotionEvent.ACTION_UP,60,70);
                check(pad.equals(Arrays.asList("0,0,1","10,20,1","0,0,0")),"a resting finger grabs, drags and lets go");
                /* The cop's map as the game sets it up -- the whole track across the
                 * whole screen -- becomes a rotating minimap in the game's own small
                 * map place, zoomed like the racer's; once changed, it stays. */
                byte[] hudConfig=new byte[0x20F0];
                java.nio.ByteBuffer cfg=java.nio.ByteBuffer.wrap(hudConfig).order(java.nio.ByteOrder.LITTLE_ENDIAN);
                cfg.putInt(0,0x20F0);
                int racer=0x6fbc50-0x6fbb40,cop=racer+0x1a4,copMap=cop+0x24+22*16;
                cfg.putFloat(racer+0x10,4f);cfg.putFloat(cop+0x10,1f);cfg.putInt(cop+0x20,2);
                cfg.putFloat(copMap,0f).putFloat(copMap+4,0f).putFloat(copMap+8,1f).putFloat(copMap+12,1f);
                cfg.putFloat(cop+0x194,.648f).putFloat(cop+0x198,.848f).putFloat(cop+0x19c,.917f).putFloat(cop+0x1a0,.998f);
                check(ControlProfile.copMinimap(hudConfig),"the cop's big map is changed");
                check(cfg.getInt(cop+0x20)==1&&cfg.getFloat(cop+0x10)==4f&&cfg.getFloat(copMap)==.648f
                    &&cfg.getFloat(copMap+4)==.848f&&cfg.getFloat(copMap+8)==.917f&&cfg.getFloat(copMap+12)==.998f,
                    "into a rotating minimap in the corner, zoomed like the racer's");
                check(!ControlProfile.copMinimap(hudConfig),"and a map already changed is left alone");
                /* The HUD an import hands over: the standings and the maps out of
                 * the bottom, where the thumbs and the controls are, into the corners. */
                ControlProfile.hudDefaults(hudConfig);
                int list=racer+0x24+13*16,racerMap=racer+0x24+17*16,speeders=cop+0x24+20*16;
                check(cfg.getFloat(list)==.1f&&cfg.getFloat(list+12)<.2f
                    &&cfg.getFloat(racerMap)==.1f&&cfg.getFloat(racerMap+4)==.85f,
                    "the standings and the racer's map sit in the top corners");
                check(cfg.getFloat(speeders+4)==0f&&cfg.getFloat(speeders+12)<.4f
                    &&cfg.getInt(racer+0x20)==1&&cfg.getFloat(cop+0x10)==4f,
                    "the cop's speeders are a block on the left and both maps zoomed minimaps");
                /* The game centres an element's text in its box, so a box hangs off
                 * the screen by its empty half to put the text against the edge. */
                check(cfg.getFloat(racer+0x24+5*16+4)<0f,"the clock's box hangs off the edge, its text against it");
                /* Import progress: a share of the whole, or unknown while the
                 * folder is still being measured. */
                check(DataImporter.ProgressListener.percent(50,200)==25&&DataImporter.ProgressListener.percent(0,-1)==-1
                    &&DataImporter.ProgressListener.percent(300,200)==100,"import progress in percent");

                TouchControlsOverlay view=new TouchControlsOverlay(test.getTargetContext(),true);view.layout(0,0,1280,680);
                RectF gearUp=bounds(view,"gear_up","box"),gearDown=bounds(view,"gear_down","box");
                check(gearUp!=null&&gearDown!=null,"gears always visible");
                RectF gas=bounds(view,"accelerate","box"),brakePair=bounds(view,"brake","box");float scale=(float)field(view,"scale");
                /* Default positions stand on the layout editor's grid.  The pedals go
                 * onto it as one, so their hand-tuned shapes and gap stay exact. */
                float cell=TouchLayout.GRID*scale;
                check(onGrid(brakePair.centerX(),cell)&&onGrid(brakePair.centerY(),cell),"brake stands on the grid");
                check(Math.abs(gas.width()-(62*scale+8))<.1f,"gas is eight physical pixels wider");
                check(Math.abs((brakePair.left-gas.right)-(12*scale+19))<.1f,"pedal gap includes twelve more physical pixels");
                check(Math.abs(gas.bottom-brakePair.bottom)<.1f,"pedals stand level");
                /* The racing layout steers with two buttons.  The five small
                 * controls above it form the final left-edge column. */
                RectF steerLeft=bounds(view,"steer_left","box"),steerRight=bounds(view,"steer_right","box");
                check(steerLeft!=null&&steerRight!=null&&steerLeft.centerX()<steerRight.centerX(),"the racing layout steers with two buttons");
                RectF pauseColumn=bounds(view,"pause","box"),horn=bounds(view,"horn","box"),spikes=bounds(view,"spike_strip","box");
                check(Math.abs(pauseColumn.left-spikes.left)<.1f&&Math.abs(pauseColumn.left-horn.left)<.1f
                    &&Math.abs(pauseColumn.left-gearUp.left)<.1f&&Math.abs(pauseColumn.left-gearDown.left)<.1f,
                    "pause, spikes, horn and gears share one left edge");
                check(pauseColumn.bottom<=spikes.top&&spikes.bottom<=horn.top&&horn.bottom<=gearUp.top&&gearUp.bottom<=gearDown.top,
                    "pause, spikes, horn, plus and minus form one column");
                RectF camera=bounds(view,"camera","box"),lookBehind=bounds(view,"look_behind","box");
                /* Every control but the steering and the pedals has the one small size,
                 * stands on the grid and sits whole cells from its neighbours. */
                check(Math.abs(horn.width()-58*scale)<.1f&&Math.abs(horn.height()-48*scale)<.1f,"small buttons are 58 x 48 at 100%");
                for(String action:new String[]{"pause","headlights","recover","horn","spike_strip",
                                               "camera","look_behind","handbrake","gear_down","gear_up"}) {
                    RectF small=bounds(view,action,"box");
                    check(Math.abs(small.width()-horn.width())<.1f&&Math.abs(small.height()-horn.height())<.1f,action+" is the small button size");
                    check(onGrid(small.centerX(),cell)&&onGrid(small.centerY(),cell),action+" stands on the grid");
                }
                RectF recoverColumn=bounds(view,"recover","box"),lightsColumn=bounds(view,"headlights","box");
                check(Math.abs(recoverColumn.right-lightsColumn.right)<.1f&&Math.abs(recoverColumn.right-lookBehind.right)<.1f
                    &&Math.abs(recoverColumn.right-camera.right)<.1f&&Math.abs(recoverColumn.right-bounds(view,"handbrake","box").right)<.1f,
                    "right utility buttons share one right edge");
                check(recoverColumn.bottom<=lightsColumn.top&&lightsColumn.bottom<=lookBehind.top&&lookBehind.bottom<=camera.top
                    &&camera.bottom<=bounds(view,"handbrake","box").top,
                    "reset, lights, look-behind, camera and handbrake form one column");
                check(bounds(view,"headlights","box")!=null&&bounds(view,"recover","box")!=null,"the racing layout shows lights and recovery");
                prefs.edit().putInt(GamePreferences.TOUCH_SIZE,80).commit();view.refreshSettings();
                check(Math.abs(bounds(view,"camera","box").width()-58*.8f*scale)<.1f&&Math.abs(bounds(view,"gear_up","box").height()-48*.8f*scale)<.1f,
                    "small buttons follow the size setting");
                prefs.edit().remove(GamePreferences.TOUCH_SIZE).commit();view.refreshSettings();
                // Exact positions from here on; snapping a dragged control is checked on its own below.
                prefs.edit().putBoolean(GamePreferences.TOUCH_SNAP,false).commit();
                view.setEditing(true,null);RectF lights=bounds(view,"pause","box");
                touch(view,0,lights.centerX(),lights.centerY());touch(view,2,640,240);touch(view,1,640,240);
                check(view.selectedAction().equals("pause"),"editor selected control");
                /* The size slider resizes the control as it moves, and a control
                 * answers to touches exactly where it is drawn: there is no touch
                 * zone of its own any more. */
                float beforeResize=bounds(view,"pause","box").width();
                view.resizeSelected(150);
                RectF visible=bounds(view,"pause","box"),hit=bounds(view,"pause","hitBox");
                check(Math.abs(visible.width()-beforeResize*1.5f)<.5f,"the button takes the size the slider asks for");
                check(Math.abs(visible.width()-hit.width())<.1f&&Math.abs(visible.height()-hit.height())<.1f,
                    "and is touched exactly where it is drawn");
                view.refreshSettings();
                check(Math.abs(bounds(view,"pause","box").width()-visible.width())<.1f,
                    "the size it was given is kept when the layout is rebuilt");
                /* The menu layout has nothing on it: a menu is worked with the screen
                 * itself, the system's back is the game's Escape, and the keyboard
                 * comes up by itself while the game takes a name.  So what is dragged
                 * in the racing layout is all there is to keep. */
                view.setMenuMode(true);
                check(bounds(view,"pause","box")==null&&bounds(view,"back","box")==null
                    &&bounds(view,"keyboard","box")==null,"the menu layout is empty");
                view.setMenuMode(false);check(Math.abs(bounds(view,"pause","box").centerX()-640)<1,"a dragged control keeps its place");
                /* A placed control keeps its distance from the nearer edge, in layout
                 * units, on an area of another size: the controls along an edge keep
                 * their spacing where the game's area differs from the editor's. */
                float fromLeft=bounds(view,"pause","box").centerX()/(float)field(view,"scale");
                view.layout(0,0,1180,680);
                check(Math.abs(bounds(view,"pause","box").centerX()/(float)field(view,"scale")-fromLeft)<.5f,
                    "a placed control keeps its distance from the nearer edge");
                view.layout(0,0,1280,680);
                view.resetLayout();check(Math.abs(bounds(view,"pause","box").centerX()-640)>20,"reset restores layout");
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
                // Mirroring must also move custom positions, and be reversible.
                GamePreferences.setTouchLayout(prefs,GamePreferences.TOUCH_LAYOUT_MIRRORED);
                view.refreshSettings();
                check(bounds(view,"steer_left","box").centerX()>640,"steering moves right");
                RectF mirroredBrake=bounds(view,"brake","box"),mirroredGas=bounds(view,"accelerate","box");
                check(mirroredBrake.left<mirroredGas.left,"mirrored brake remains the outer pedal");
                GamePreferences.setTouchLayout(prefs,GamePreferences.TOUCH_LAYOUT_STANDARD);
                view.refreshSettings();
                check(bounds(view,"steer_left","box").centerX()<640,"steering moves left again");
                check(bounds(view,"steer_left","box").centerX()<bounds(view,"steer_right","box").centerX(),"left steers left of right");
                /* The menus take a finger on the screen itself and nothing else: every
                 * control stays behind, the two that used to be there among them. */
                view.setEditing(false,null);view.setMenuMode(true);
                Map<?,?> held=(Map<?,?>)field(field(view,"keys"),"held");
                for(String gone:new String[]{"back","keyboard","steer_left","steer_right","accelerate","brake",
                                             "handbrake","pause","headlights","recover","horn","spike_strip",
                                             "gear_up","gear_down","look_behind","camera"})
                    check(bounds(view,gone,"box")==null,"the menu layout leaves "+gone+" behind");
                touch(view,0,view.getWidth()/2f,view.getHeight()/2f);
                check(held.isEmpty(),"a finger in the menus presses no key of ours");
                touch(view,1,view.getWidth()/2f,view.getHeight()/2f);
                view.setMenuMode(false);
                check(bounds(view,"steer_left","box")!=null&&bounds(view,"accelerate","box")!=null,"the racing layout comes back");
                RectF left=bounds(view,"steer_left","box");
                touch(view,0,left.centerX(),left.centerY());
                check(held.size()==1&&held.containsKey(android.view.KeyEvent.KEYCODE_DPAD_LEFT),"the left button steers left");
                touch(view,1,left.centerX(),left.centerY());
                view.releaseAll();
                /* Positions kept the old way, as fractions of the area, become their
                 * distances from the nearer edges, once, where they stood. */
                prefs.edit().putFloat("touch_position_race_horn_x",.1f).putFloat("touch_position_race_horn_y",.75f)
                    .putFloat("touch_position_race_brake_x",.9f).putFloat("touch_position_race_brake_y",.2f).commit();
                TouchControlsOverlay.anchorLegacyPositions(prefs,800,400);
                check(!prefs.contains("touch_position_race_horn_x")&&!prefs.contains("touch_position_race_horn_y"),
                    "old positions are replaced");
                check(Math.abs(prefs.getFloat("touch_position_race_horn_dx",-1)-80)<.01f&&!prefs.getBoolean("touch_position_race_horn_from_right",true)
                    &&Math.abs(prefs.getFloat("touch_position_race_horn_dy",-1)-100)<.01f&&prefs.getBoolean("touch_position_race_horn_from_bottom",false),
                    "an old position becomes its distance from the left and the bottom");
                check(Math.abs(prefs.getFloat("touch_position_race_brake_dx",-1)-80)<.01f&&prefs.getBoolean("touch_position_race_brake_from_right",false)
                    &&Math.abs(prefs.getFloat("touch_position_race_brake_dy",-1)-80)<.01f&&!prefs.getBoolean("touch_position_race_brake_from_bottom",true),
                    "and from the right and the top");
                SharedPreferences.Editor unplace=prefs.edit();
                for(String k:prefs.getAll().keySet())if(k.startsWith("touch_position_"))unplace.remove(k);
                unplace.commit();
                /* In the game the race buttons a car has no use for are gone: the
                 * gears with an automatic gearbox, the spike strip outside a
                 * police car.  A hidden button presses nothing. */
                TouchControlsOverlay race=new TouchControlsOverlay(test.getTargetContext(),false,(k,down)->{});
                race.setMenuMode(false);race.layout(0,0,1280,680);
                Map<?,?> raceHeld=(Map<?,?>)field(field(race,"keys"),"held");
                RectF gear=bounds(race,"gear_up","box"),spike=bounds(race,"spike_strip","box");
                race.setRaceControls(false,false);
                touch(race,0,gear.centerX(),gear.centerY());touch(race,1,gear.centerX(),gear.centerY());
                touch(race,0,spike.centerX(),spike.centerY());touch(race,1,spike.centerX(),spike.centerY());
                check(raceHeld.isEmpty(),"hidden gear and spike buttons press nothing");
                race.setRaceControls(true,true);
                touch(race,0,gear.centerX(),gear.centerY());
                check(raceHeld.size()==1,"shown again, the gear button presses");
                touch(race,1,gear.centerX(),gear.centerY());race.releaseAll();
                /* An import short of game files is told which: every file of the
                 * disc's list but one, in capitals as the disc has them, leaves
                 * exactly that one missing. */
                java.io.File data=new java.io.File(test.getTargetContext().getCacheDir(),"manifest-check");
                java.util.List<String> all=DataImporter.missingFiles(test.getTargetContext(),data);
                check(all.contains("gamedata/render/pc/cop0.art")&&all.size()>900,"the disc's list names the game's files");
                for(String path:all) if(!path.equals("gamedata/render/pc/cop0.art")) {
                    java.io.File file=new java.io.File(data,path.toUpperCase(java.util.Locale.ROOT));
                    file.getParentFile().mkdirs();file.createNewFile();
                }
                java.util.List<String> short1=DataImporter.missingFiles(test.getTargetContext(),data);
                check(short1.size()==1&&short1.get(0).equals("gamedata/render/pc/cop0.art"),
                    "a copy short of one file is told which, whatever the case of the names");
                check(DataImporter.describeMissing(test.getTargetContext(),all).contains("…"),"a long list is cut short");
                deleteTree(data);
                prefs.edit().putBoolean(GamePreferences.TOUCH_AUTO_HIDE,true)
                    .putInt(GamePreferences.TOUCH_HIDE_SECONDS,1).apply();
                idle[0]=new TouchControlsOverlay(test.getTargetContext(),false,(k,down)->{});idle[0].setMenuMode(false);idle[0].layout(0,0,1280,680);
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
                idle[0].resumeIdleTimer();
            }catch(Throwable e){failure[0]=e;}});
            Thread.sleep(1500);
            test.runOnMainSync(()->{if(idle[0].getAlpha()>.01f)failure[0]=new AssertionError("hidden again, completely, after the delay");});
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
