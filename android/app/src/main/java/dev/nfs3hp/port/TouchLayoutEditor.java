package dev.nfs3hp.port;

import android.app.Activity;
import android.app.AlertDialog;
import android.view.View;
import android.widget.*;

final class TouchLayoutEditor {
    static TouchControlsOverlay attach(Activity activity,Runnable done) {
        TouchControlsOverlay overlay=new TouchControlsOverlay(activity,true);
        TouchPreviewFrame.attach(activity,activity.findViewById(R.id.editor_canvas),overlay);
        SeekBar visual=activity.findViewById(R.id.editor_visual),hit=activity.findViewById(R.id.editor_hit);
        TextView label=activity.findViewById(R.id.editor_selection);
        visual.setMin(50);visual.setMax(180);hit.setMin(50);hit.setMax(200);
        Runnable sync=()->{
            label.setText(overlay.selectedLabel());
            visual.setEnabled(overlay.selectedAction()!=null);hit.setEnabled(overlay.selectedAction()!=null);
            visual.setProgress(overlay.selectedVisualSize());hit.setProgress(overlay.selectedHitSize());
            ((TextView)activity.findViewById(R.id.editor_visual_label)).setText("Button: "+visual.getProgress()+"%");
            ((TextView)activity.findViewById(R.id.editor_hit_label)).setText("Touch zone: "+hit.getProgress()+"%");
        };
        overlay.setEditing(true,name->{label.setText(name);sync.run();});sync.run();
        SeekBar.OnSeekBarChangeListener listener=new SeekBar.OnSeekBarChangeListener(){
            public void onProgressChanged(SeekBar bar,int value,boolean user){
                if(user){overlay.resizeSelected(visual.getProgress(),hit.getProgress());sync.run();}
            }
            public void onStartTrackingTouch(SeekBar bar){}
            public void onStopTrackingTouch(SeekBar bar){}
        };
        visual.setOnSeekBarChangeListener(listener);hit.setOnSeekBarChangeListener(listener);
        Spinner mode=activity.findViewById(R.id.editor_mode);
        ArrayAdapter<String> adapter=new ArrayAdapter<>(activity,android.R.layout.simple_spinner_dropdown_item,new String[]{"Race layout","Menu layout"});
        mode.setAdapter(adapter);
        mode.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener(){
            public void onItemSelected(AdapterView<?> p,View v,int index,long id){overlay.setMenuMode(index==1);sync.run();}
            public void onNothingSelected(AdapterView<?> p){}
        });
        activity.findViewById(R.id.editor_done).setOnClickListener(v->done.run());
        activity.findViewById(R.id.editor_reset).setOnClickListener(v->new AlertDialog.Builder(activity)
            .setTitle("Reset this layout?").setMessage("Restore default positions, button sizes and touch zones for this layout.")
            .setNegativeButton("Cancel",null).setPositiveButton("Reset",(d,w)->{overlay.resetLayout();sync.run();}).show());
        return overlay;
    }
}
