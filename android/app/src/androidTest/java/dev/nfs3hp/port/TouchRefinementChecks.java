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
                /* The racing layout steers with two buttons, stacks horn and spikes over
                 * them and the gears over those in the same two columns. */
                RectF steerLeft=bounds(view,"steer_left","box"),steerRight=bounds(view,"steer_right","box");
                check(steerLeft!=null&&steerRight!=null&&steerLeft.centerX()<steerRight.centerX(),"the racing layout steers with two buttons");
                RectF horn=bounds(view,"horn","box"),spikes=bounds(view,"spike_strip","box");
                check(Math.abs(gearDown.centerX()-horn.centerX())<.1f&&Math.abs(gearUp.centerX()-spikes.centerX())<.1f,"gears stand over horn and spikes");
                check(gearDown.bottom<=horn.top&&horn.bottom<=steerLeft.top,"gears, then horn and spikes, then the steering");
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
                check(onGrid(spikes.left-horn.left,cell)&&Math.abs(spikes.top-horn.top)<.1f,"horn and spikes share a row, whole cells apart");
                check(onGrid(lookBehind.left-camera.left,cell)&&Math.abs(lookBehind.top-camera.top)<.1f,"camera and look-behind share a row, whole cells apart");
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
                view.resizeSelected(150,80);
                RectF visible=bounds(view,"pause","box"),hit=bounds(view,"pause","hitBox");
                check(visible.width()>hit.width(),"visual and hit sizes independent");
                /* The two layouts keep their positions apart: pause exists only in the
                 * racing one, so dragging it there leaves the menu layout alone. */
                view.setMenuMode(true);check(bounds(view,"pause","box")==null,"the menu layout has no pause");
                view.setMenuMode(false);check(Math.abs(bounds(view,"pause","box").centerX()-640)<1,"a dragged control keeps its place");
                RectF menuBack;
                view.setMenuMode(true);menuBack=bounds(view,"back","box");
                touch(view,0,menuBack.centerX(),menuBack.centerY());touch(view,2,300,200);touch(view,1,300,200);
                check(Math.abs(bounds(view,"back","box").centerX()-300)<1,"the menu layout saves its own position");
                /* A placed control keeps its distance from the nearer edge, in layout
                 * units, on an area of another size: the controls along an edge keep
                 * their spacing where the game's area differs from the editor's. */
                float fromLeft=bounds(view,"back","box").centerX()/(float)field(view,"scale");
                view.layout(0,0,1180,680);
                check(Math.abs(bounds(view,"back","box").centerX()/(float)field(view,"scale")-fromLeft)<.5f,
                    "a placed control keeps its distance from the nearer edge");
                view.layout(0,0,1280,680);
                // Reset works on the layout shown; the menu one goes back to its defaults for the checks below.
                view.resetLayout();check(Math.abs(bounds(view,"back","box").centerX()-300)>20,"reset restores the menu layout");
                view.setMenuMode(false);check(Math.abs(bounds(view,"pause","box").centerX()-640)<1,"the racing layout is untouched by it");
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
                /* The menus take a finger on the screen itself, so the menu layout is
                 * down to the two keys a screen cannot offer. */
                view.setEditing(false,null);view.setMenuMode(true);
                check(bounds(view,"back","box")!=null&&bounds(view,"keyboard","box")!=null,"the menu layout keeps back and the keyboard");
                for(String gone:new String[]{"steer_left","steer_right","accelerate","brake","handbrake","pause",
                                             "headlights","recover","horn","spike_strip","gear_up","gear_down",
                                             "look_behind","camera"})
                    check(bounds(view,gone,"box")==null,"the menu layout leaves "+gone+" behind");
                Map<?,?> held=(Map<?,?>)field(field(view,"keys"),"held");
                RectF back=bounds(view,"back","box"),menuKeyboard=bounds(view,"keyboard","box");
                check(Math.abs(back.width()-menuKeyboard.width())<.5f&&Math.abs(back.height()-menuKeyboard.height())<.5f,
                    "back is the keyboard's size");
                check(Math.abs(back.centerX()-menuKeyboard.centerX())<.5f&&back.bottom<=menuKeyboard.top
                    &&menuKeyboard.bottom<view.getHeight()/2f,"back takes the top corner and the keyboard stands under it");
                touch(view,0,back.centerX(),back.centerY());
                check(held.size()==1&&held.containsKey(GamePreferences.getTouchKey(prefs,"pause")),"back sends the pause key");
                touch(view,1,back.centerX(),back.centerY());
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
                prefs.edit().putBoolean(GamePreferences.TOUCH_AUTO_HIDE,true).putBoolean(GamePreferences.TOUCH_HIDE_FULL,true)
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
