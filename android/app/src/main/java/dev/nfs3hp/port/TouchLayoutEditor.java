package dev.nfs3hp.port;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.SharedPreferences;
import android.view.View;
import android.widget.*;

final class TouchLayoutEditor {
    static TouchControlsOverlay attach(Activity activity,Runnable done) {
        TouchControlsOverlay overlay=new TouchControlsOverlay(activity,true);
        TouchPreviewFrame.attach(activity,activity.findViewById(R.id.editor_canvas),overlay);
        /* One size a control: what it covers is what it answers to, so the
         * button drawn in the preview is exactly what the game will show and
         * touch.  It used to carry a touch zone of its own, which left a button
         * made bigger still answering to its old, smaller square. */
        SeekBar visual=activity.findViewById(R.id.editor_visual);
        TextView label=activity.findViewById(R.id.editor_selection);
        visual.setMin(50);visual.setMax(180);
        Runnable sync=()->{
            label.setText(overlay.selectedLabel());
            visual.setEnabled(overlay.selectedAction()!=null);
            visual.setProgress(overlay.selectedVisualSize());
            ((TextView)activity.findViewById(R.id.editor_visual_label)).setText(activity.getString(R.string.editor_button_size,visual.getProgress()));
        };
        overlay.setEditing(true,name->{label.setText(name);sync.run();});sync.run();
        visual.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener(){
            public void onProgressChanged(SeekBar bar,int value,boolean user){
                if(user){overlay.resizeSelected(value);sync.run();}
            }
            public void onStartTrackingTouch(SeekBar bar){}
            public void onStopTrackingTouch(SeekBar bar){}
        });
        // Whether a dragged control lands on the grid the editor draws; on unless switched off.
        SharedPreferences preferences=GamePreferences.get(activity);
        CheckBox snap=activity.findViewById(R.id.editor_snap);
        snap.setChecked(preferences.getBoolean(GamePreferences.TOUCH_SNAP,true));
        snap.setOnCheckedChangeListener((box,on)->preferences.edit().putBoolean(GamePreferences.TOUCH_SNAP,on).apply());
        /* There is one layout to arrange now: the racing one.  A menu has no
         * controls of ours on it at all -- the screen itself is what it is
         * worked with, the system's back is Escape, and the keyboard comes up
         * by itself -- so the editor has nothing to show for it and no longer
         * asks which of the two is being edited. */
        overlay.setMenuMode(false);
        activity.findViewById(R.id.editor_done).setOnClickListener(v->done.run());
        activity.findViewById(R.id.editor_reset).setOnClickListener(v->new AlertDialog.Builder(activity)
            .setTitle(R.string.editor_reset_title).setMessage(R.string.editor_reset_message)
            .setNegativeButton(android.R.string.cancel,null)
            .setPositiveButton(android.R.string.ok,(d,w)->{overlay.resetLayout();sync.run();}).show());
        return overlay;
    }
}
