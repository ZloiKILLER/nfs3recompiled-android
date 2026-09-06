package dev.nfs3hp.port;

import android.content.Context;
import android.content.SharedPreferences;
import android.graphics.*;
import android.view.MotionEvent;
import android.view.View;
import java.util.ArrayList;
import java.util.function.Consumer;

/** Resolution-independent DualSense diagram, with assignments read from the live preferences. */
final class ControllerDiagramView extends View {
    private final Paint paint=new Paint(Paint.ANTI_ALIAS_FLAG);
    private final ArrayList<Part> parts=new ArrayList<>();
    private final SharedPreferences prefs;
    private final Consumer<String> select;
    private float scale=1,offsetX,offsetY;
    private String pressed;
    private static final class Part {
        final String id,symbol;final float x,y;final RectF callout;
        Part(String id,String symbol,float x,float y,RectF box){this.id=id;this.symbol=symbol;this.x=x;this.y=y;callout=box;}
    }
    ControllerDiagramView(Context c,Consumer<String> select) {
        super(c);this.select=select;prefs=GamePreferences.get(c);
        setContentDescription("DualSense controller diagram. Tap a button or its assignment to remap it. Assignments are also available in the list below.");
        String[] left={"back","left_shoulder","dpad_up","dpad_left","dpad_right","dpad_down","left_stick"};
        String[] ls={"Create","L1","↑","←","→","↓","L3"};
        float[] lx={390,342,337,312,362,337,409},ly={114,88,157,182,182,207,248};
        String[] right={"start","right_shoulder","north","west","east","south","right_stick"};
        String[] rs={"Options","R1","△","□","○","×","R3"};
        float[] rx={570,618,621,596,646,621,551},ry={114,88,157,182,182,207,248};
        for(int i=0;i<left.length;i++)parts.add(new Part(left[i],ls[i],lx[i],ly[i],new RectF(8,58+i*42,250,96+i*42)));
        for(int i=0;i<right.length;i++)parts.add(new Part(right[i],rs[i],rx[i],ry[i],new RectF(710,58+i*42,952,96+i*42)));
    }
    @Override protected void onDraw(Canvas c) {
        scale=Math.min(getWidth()/960f,getHeight()/400f);offsetX=(getWidth()-960*scale)/2;offsetY=(getHeight()-400*scale)/2;
        c.save();c.translate(offsetX,offsetY);c.scale(scale,scale);
        text(c,"DUALSENSE · BUTTON ASSIGNMENTS",480,23,18,0xffeef2f8);
        // Shoulder silhouette and the two swept grips characteristic of DualSense.
        fill(0xff334155);c.drawRoundRect(new RectF(306,63,381,98),17,17,paint);c.drawRoundRect(new RectF(579,63,654,98),17,17,paint);
        Path shell=new Path();shell.moveTo(346,100);shell.cubicTo(300,95,294,132,279,207);
        shell.cubicTo(261,282,261,349,290,353);shell.cubicTo(314,358,334,312,361,283);
        shell.cubicTo(409,303,551,303,599,283);shell.cubicTo(626,312,646,358,670,353);
        shell.cubicTo(699,349,699,282,681,207);shell.cubicTo(666,132,660,95,614,100);shell.close();
        fill(0xff9baac0);c.drawPath(shell,paint);stroke(0xffcfdae8,2);c.drawPath(shell,paint);
        fill(0xff1a2330);c.drawRoundRect(new RectF(377,205,583,289),35,35,paint);
        fill(0xff263648);c.drawRoundRect(new RectF(414,111,546,186),18,18,paint);
        stroke(0xff50667f,2);c.drawRoundRect(new RectF(414,111,546,186),18,18,paint);
        text(c,"TOUCHPAD",480,153,11,0xff9baac0);
        for(float x:new float[]{409,551}){fill(0xff10151d);c.drawCircle(x,248,35,paint);stroke(0xff607187,2);c.drawCircle(x,248,28,paint);}
        for(Part p:parts) {
            boolean on=p.id.equals(pressed);fill(on?0xff694d2b:0xff1a2330);c.drawRoundRect(p.callout,9,9,paint);
            stroke(on?0xfff4b664:0xff334155,1);c.drawRoundRect(p.callout,9,9,paint);
            boolean left=p.callout.centerX()<480;
            float edge=left?p.callout.right:p.callout.left;
            stroke(on?0xfff4b664:0x665c718a,on?2:1);
            c.drawLine(edge,p.callout.centerY(),left?268:692,p.callout.centerY(),paint);
            c.drawLine(left?268:692,p.callout.centerY(),p.x,p.y,paint);
            fill(on?0xfff4b664:0xff10151d);c.drawCircle(p.x,p.y,p.id.endsWith("stick")?22:16,paint);
            text(c,p.symbol,p.x,p.y+5,p.id.equals("back")||p.id.equals("start")?8:15,on?0xff10151d:0xffeef2f8);
            text(c,p.symbol,p.callout.left+31,p.callout.centerY()+5,14,0xfff4b664);
            String binding=assignment(p.id);
            paint.setTextSize(14);float font=Math.min(14,14*174/Math.max(1,paint.measureText(binding)));
            text(c,binding,p.callout.left+150,p.callout.centerY()+5,font,0xffeef2f8);
        }
        text(c,"Tap a button to remap · Sticks / L2 / R2: configured by the game",480,388,13,0xff9baac0);
        c.restore();
    }
    private String assignment(String id) {
        for(int i=0;i<GamePreferences.ACTION_IDS.length;i++)if(id.equals(GamePreferences.getPhysicalButton(prefs,i)))return GamePreferences.ACTION_LABELS[i];
        return "Unassigned";
    }
    private void fill(int color){paint.setStyle(Paint.Style.FILL);paint.setColor(color);}
    private void stroke(int color,float width){paint.setStyle(Paint.Style.STROKE);paint.setColor(color);paint.setStrokeWidth(width);}
    private void text(Canvas c,String text,float x,float y,float size,int color){fill(color);paint.setTextAlign(Paint.Align.CENTER);paint.setTypeface(Typeface.create("sans-serif-medium",0));paint.setTextSize(size);c.drawText(text,x,y,paint);}
    private String hit(float x,float y){
        for(Part p:parts)if(p.callout.contains(x,y)||(x-p.x)*(x-p.x)+(y-p.y)*(y-p.y)<24*24)return p.id;
        return null;
    }
    @Override public boolean onTouchEvent(MotionEvent e){
        float x=(e.getX()-offsetX)/scale,y=(e.getY()-offsetY)/scale;
        if(e.getActionMasked()==MotionEvent.ACTION_DOWN){pressed=hit(x,y);invalidate();return pressed!=null;}
        if(e.getActionMasked()==MotionEvent.ACTION_UP){String id=pressed;pressed=null;invalidate();if(id!=null&&id.equals(hit(x,y)))select.accept(id);return true;}
        if(e.getActionMasked()==MotionEvent.ACTION_CANCEL){pressed=null;invalidate();return true;}
        return pressed!=null;
    }
}
