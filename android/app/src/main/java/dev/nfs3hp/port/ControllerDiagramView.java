package dev.nfs3hp.port;

import android.content.Context;
import android.content.SharedPreferences;
import android.graphics.*;
import android.view.MotionEvent;
import android.view.View;
import java.util.ArrayList;
import java.util.function.Consumer;

/** Resolution-independent DualSense diagram with remappable axes and triggers. */
final class ControllerDiagramView extends View {
    private final Paint paint=new Paint(Paint.ANTI_ALIAS_FLAG);
    private final ArrayList<Part> parts=new ArrayList<>();
    private final SharedPreferences prefs;
    private final Consumer<String> select;
    private final String[] actionLabels;
    private final String unassigned;
    private final Bitmap controllerImage;
    private float scale=1,offsetX,offsetY;
    private String pressed;
    private static final class Part {
        final String id,symbol;final float x,y;final RectF callout;
        Part(String id,String symbol,float x,float y,RectF box){this.id=id;this.symbol=symbol;this.x=x;this.y=y;callout=box;}
    }
    ControllerDiagramView(Context c,Consumer<String> select) {
        super(c);this.select=select;prefs=GamePreferences.get(c);
        actionLabels=GamePreferences.actionLabels(c);unassigned=c.getString(R.string.label_unassigned);
        controllerImage=BitmapFactory.decodeResource(getResources(),R.drawable.controller_dualsense_transparent);
        setContentDescription(c.getString(R.string.diagram_description));
        String[] left={"left_trigger","left_shoulder","back","dpad_up","dpad_left","dpad_right","dpad_down",
            "left_stick_up","left_stick_left","left_stick_right","left_stick_down","left_stick"};
        String[] ls={"LT","LB","VIEW","↑","←","→","↓","LS↑","LS←","LS→","LS↓","L3"};
        float[] lx={371,371,392,362,340,384,362,418,404,432,418,418};
        float[] ly={78,90,107,125,147,147,168,188,202,202,216,202};
        String[] right={"right_trigger","right_shoulder","start","north","west","east","south",
            "right_stick_up","right_stick_left","right_stick_right","right_stick_down","right_stick"};
        String[] rs={"RT","RB","MENU","Y","X","B","A","RS↑","RS←","RS→","RS↓","R3"};
        float[] rx={589,589,569,600,570,628,600,541,527,555,541,541};
        float[] ry={78,90,107,122,147,147,177,188,202,202,216,202};
        for(int i=0;i<left.length;i++)parts.add(new Part(left[i],ls[i],lx[i],ly[i],new RectF(6,32+i*29,258,57+i*29)));
        for(int i=0;i<right.length;i++)parts.add(new Part(right[i],rs[i],rx[i],ry[i],new RectF(702,32+i*29,954,57+i*29)));
    }
    @Override protected void onDraw(Canvas c) {
        scale=Math.min(getWidth()/960f,getHeight()/400f);offsetX=(getWidth()-960*scale)/2;offsetY=(getHeight()-400*scale)/2;
        c.save();c.translate(offsetX,offsetY);c.scale(scale,scale);
        text(c,getContext().getString(R.string.diagram_title),480,20,18,0xffeef2f8);
        if(controllerImage!=null) {
            int iw=controllerImage.getWidth(),ih=controllerImage.getHeight();
            Rect source=new Rect(Math.round(iw*.14f),Math.round(ih*.29f),Math.round(iw*.865f),Math.round(ih*.80f));
            paint.setStyle(Paint.Style.FILL);paint.setAlpha(255);paint.setFilterBitmap(true);
            c.drawBitmap(controllerImage,source,new RectF(279,55,681,347),paint);
        }
        for(Part p:parts) {
            boolean on=p.id.equals(pressed);fill(on?0xff694d2b:0xff1a2330);c.drawRoundRect(p.callout,7,7,paint);
            stroke(on?0xfff4b664:0xff334155,1);c.drawRoundRect(p.callout,7,7,paint);
            boolean left=p.callout.centerX()<480;float edge=left?p.callout.right:p.callout.left;
            stroke(on?0xfff4b664:0x665c718a,on?2:1);c.drawLine(edge,p.callout.centerY(),left?276:684,p.callout.centerY(),paint);c.drawLine(left?276:684,p.callout.centerY(),p.x,p.y,paint);
            fill(on?0xfff4b664:0xff263648);c.drawCircle(p.x,p.y,7,paint);
            text(c,p.symbol,p.callout.left+29,p.callout.centerY()+4,10,0xfff4b664);
            String binding=assignment(p.id);paint.setTextSize(12);float font=Math.min(12,12*184/Math.max(1,paint.measureText(binding)));
            text(c,binding,p.callout.left+151,p.callout.centerY()+4,font,0xffeef2f8);
        }
        text(c,getContext().getString(R.string.diagram_hint),480,391,13,0xff9baac0);c.restore();
    }
    private String assignment(String id){for(int i=0;i<GamePreferences.ACTION_IDS.length;i++)if(id.equals(GamePreferences.getPhysicalButton(prefs,i)))return actionLabels[i];return unassigned;}
    private void fill(int color){paint.setStyle(Paint.Style.FILL);paint.setColor(color);}
    private void stroke(int color,float width){paint.setStyle(Paint.Style.STROKE);paint.setColor(color);paint.setStrokeWidth(width);}
    private void text(Canvas c,String text,float x,float y,float size,int color){fill(color);paint.setTextAlign(Paint.Align.CENTER);paint.setTypeface(Typeface.create("sans-serif-medium",0));paint.setTextSize(size);c.drawText(text,x,y,paint);}
    private String hit(float x,float y){for(Part p:parts)if(p.callout.contains(x,y)||(x-p.x)*(x-p.x)+(y-p.y)*(y-p.y)<9*9)return p.id;return null;}
    @Override public boolean onTouchEvent(MotionEvent e){float x=(e.getX()-offsetX)/scale,y=(e.getY()-offsetY)/scale;if(e.getActionMasked()==MotionEvent.ACTION_DOWN){pressed=hit(x,y);invalidate();return pressed!=null;}if(e.getActionMasked()==MotionEvent.ACTION_UP){String id=pressed;pressed=null;invalidate();if(id!=null&&id.equals(hit(x,y)))select.accept(id);return true;}if(e.getActionMasked()==MotionEvent.ACTION_CANCEL){pressed=null;invalidate();return true;}return pressed!=null;}
}
