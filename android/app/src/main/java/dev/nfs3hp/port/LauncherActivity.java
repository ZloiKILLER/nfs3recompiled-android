package dev.nfs3hp.port;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Intent;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.graphics.Color;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.Bundle;
import android.provider.OpenableColumns;
import android.text.InputType;
import android.text.Spannable;
import android.text.SpannableStringBuilder;
import android.text.style.ImageSpan;
import android.util.Log;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import androidx.documentfile.provider.DocumentFile;

import java.io.File;
import java.io.IOException;
import java.util.List;

public class LauncherActivity extends Activity
{
    private static final String TAG = "NFS3Launcher";
    private static final int REQUEST_PICK_FOLDER = 10;
    private static final int REQUEST_PICK_ZIP = 11;
    private static final int REQUEST_IMPORT_SAVES = 12;
    private static final int REQUEST_EXPORT_SAVES = 13;
    private static final int REQUEST_IMPORT_LAUNCHER = 14;
    private static final int REQUEST_EXPORT_LAUNCHER = 15;

    private String currentScreen = "main";
    private Thread importThread;
    private DataSetManager dataSetManager;
    private TouchControlsOverlay touchPreview;

    @Override
    public void setContentView(int layout) {
        if (touchPreview != null) { touchPreview.releaseAll(); touchPreview = null; }
        super.setContentView(layout);
        android.view.ViewGroup content = findViewById(android.R.id.content);
        View root = content.getChildAt(0);
        root.setBackgroundColor(getColor(R.color.ui_background));
        final int l=root.getPaddingLeft(),t=root.getPaddingTop(),r=root.getPaddingRight(),b=root.getPaddingBottom();
        root.setOnApplyWindowInsetsListener((view,insets)->{
            android.graphics.Rect safe=AndroidWindowCompat.safeInsets(insets);
            view.setPadding(l+safe.left,t+safe.top,r+safe.right,b+safe.bottom);
            return insets;
        });
        root.requestApplyInsets();
    }

    /* Language of the launcher, chosen on its own screen.
     * LocaleHelper returns the context unchanged while the setting is
     * "system". */
    @Override
    protected void attachBaseContext(android.content.Context base)
    {
        super.attachBaseContext(LocaleHelper.wrap(base));
    }

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        try
        {
            dataSetManager = new DataSetManager(this, dataRoot());
        }
        catch (IOException e)
        {
            Log.e(TAG, "Could not initialize data sets", e);
            Toast.makeText(this, getString(R.string.data_error, e.getMessage()),
                Toast.LENGTH_LONG).show();
        }
        showMainScreen();
    }

    private File dataRoot() throws IOException
    {
        File root = getExternalFilesDir(null);
        if (root == null)
            throw new IOException(getString(R.string.error_external_storage));
        return root;
    }

    private void showMainScreen()
    {
        currentScreen = "main";
        setContentView(R.layout.activity_launcher_home);
        Button playButton = findViewById(R.id.play_button);
        DataSetManager.DataSet active = dataSetManager == null ? null : dataSetManager.active();
        if (active == null)
        {
            playButton.setEnabled(false);
        }
        else
        {
            playButton.setEnabled(true);
        }

        playButton.setOnClickListener(v -> startGame());
        findViewById(R.id.data_button).setOnClickListener(v -> showDataScreen());
        findViewById(R.id.controls_button).setOnClickListener(v -> showControlsHome());
        findViewById(R.id.screen_settings_button).setOnClickListener(v -> showScreenScreen());
        findViewById(R.id.language_button).setOnClickListener(v -> showLanguageScreen());
        findViewById(R.id.faq_button).setOnClickListener(v -> showFaq());
    }

    /** Language of the launcher only: the game's own menus come from FEDATA and
     *  stay in the language of the disc. */
    private void showLanguageScreen()
    {
        currentScreen = "language";
        setContentView(R.layout.activity_language);
        findViewById(R.id.back_button).setOnClickListener(v -> showMainScreen());

        SharedPreferences preferences = GamePreferences.get(this);
        String[] tags = getResources().getStringArray(R.array.language_tags);
        String[] names = getResources().getStringArray(R.array.language_names);
        String current = preferences.getString(GamePreferences.UI_LANGUAGE,
            GamePreferences.UI_LANGUAGE_SYSTEM);
        RadioGroup list = findViewById(R.id.language_list);
        RadioButton chosen = null;
        for (int i = 0; i < tags.length; ++i)
        {
            RadioButton row = new RadioButton(this);
            // RadioGroup clears the previous selection through findViewById, so a
            // row without an id would leave two languages ticked at once.
            row.setId(View.generateViewId());
            row.setText(names[i]);
            row.setTextColor(getColor(R.color.ui_text));
            row.setPadding(row.getPaddingLeft(), dp(10), row.getPaddingRight(), dp(10));
            if (tags[i].equals(current))
                chosen = row;
            final String tag = tags[i];
            row.setOnClickListener(v -> {
                if (tag.equals(preferences.getString(GamePreferences.UI_LANGUAGE,
                        GamePreferences.UI_LANGUAGE_SYSTEM)))
                    return;
                // commit, not apply: recreate() re-reads this from attachBaseContext
                // before an asynchronous write would necessarily have landed.
                preferences.edit().putString(GamePreferences.UI_LANGUAGE, tag).commit();
                recreate();
            });
            list.addView(row);
        }
        if (chosen != null)
        {
            list.check(chosen.getId());
            // The list is longer than the screen: bring the current language into
            // view rather than leaving the reader to hunt for the ticked row.
            final View target = chosen;
            target.post(() -> {
                View scroller = (View) list.getParent();
                if (scroller instanceof android.widget.ScrollView)
                    ((android.widget.ScrollView) scroller).scrollTo(0,
                        Math.max(0, target.getTop() - target.getHeight()));
            });
        }
    }

    /** The Data menu: game data, saved games and the launcher's own settings. */
    private void showDataScreen()
    {
        currentScreen = "data";
        setContentView(R.layout.activity_data_settings);
        findViewById(R.id.back_button).setOnClickListener(v -> showMainScreen());
        findViewById(R.id.game_data_button).setOnClickListener(v -> showGameDataScreen());
        findViewById(R.id.game_saves_button).setOnClickListener(v -> showGameSavesScreen());
        findViewById(R.id.launcher_settings_button).setOnClickListener(v -> showLauncherSettingsScreen());
    }

    private void showGameSavesScreen()
    {
        currentScreen = "game_saves";
        setContentView(R.layout.activity_game_saves);
        findViewById(R.id.back_button).setOnClickListener(v -> showDataScreen());
        findViewById(R.id.import_saves_button).setOnClickListener(v -> launchSaveImport());
        findViewById(R.id.export_saves_button).setOnClickListener(v -> launchSaveExport());

        android.widget.CheckBox settingsBox = findViewById(R.id.saves_include_settings);
        settingsBox.setChecked(includeSettings());
        settingsBox.setOnCheckedChangeListener((box, checked) ->
            GamePreferences.get(this).edit().putBoolean(GamePreferences.SAVES_INCLUDE_SETTINGS, checked).apply());

        boolean active = dataSetManager != null && dataSetManager.active() != null;
        findViewById(R.id.import_saves_button).setEnabled(active);
        findViewById(R.id.export_saves_button).setEnabled(active);
    }

    private void showLauncherSettingsScreen()
    {
        currentScreen = "launcher_settings";
        setContentView(R.layout.activity_launcher_settings);
        findViewById(R.id.back_button).setOnClickListener(v -> showDataScreen());
        findViewById(R.id.import_launcher_button).setOnClickListener(v -> launchLauncherImport());
        findViewById(R.id.export_launcher_button).setOnClickListener(v -> launchLauncherExport());
    }

    private void showGameDataScreen()
    {
        currentScreen = "game_data";
        setContentView(R.layout.activity_game_data);
        findViewById(R.id.back_button).setOnClickListener(v -> showDataScreen());
        findViewById(R.id.import_folder_button).setOnClickListener(v -> launchFolderPicker());
        findViewById(R.id.import_zip_button).setOnClickListener(v -> launchZipPicker());

        Spinner spinner = findViewById(R.id.data_set_spinner);
        List<DataSetManager.DataSet> dataSets = dataSetManager == null
            ? java.util.Collections.emptyList() : dataSetManager.list();
        ArrayAdapter<DataSetManager.DataSet> adapter = new ArrayAdapter<>(this,
            android.R.layout.simple_spinner_item, dataSets);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        spinner.setAdapter(adapter);

        DataSetManager.DataSet active = dataSetManager == null ? null : dataSetManager.active();
        TextView activeText = findViewById(R.id.data_active_text);
        activeText.setText(active == null ? getString(R.string.no_active_data_set)
            : getString(R.string.active_data_set, active.name));

        Button switchButton = findViewById(R.id.switch_data_button);
        Button deleteButton = findViewById(R.id.delete_data_button);
        switchButton.setEnabled(!dataSets.isEmpty());
        deleteButton.setEnabled(!dataSets.isEmpty());
        switchButton.setOnClickListener(v -> {
            DataSetManager.DataSet selected = (DataSetManager.DataSet) spinner.getSelectedItem();
            if (selected != null)
                switchDataSet(selected);
        });
        deleteButton.setOnClickListener(v -> {
            DataSetManager.DataSet selected = (DataSetManager.DataSet) spinner.getSelectedItem();
            if (selected != null)
                confirmDelete(selected);
        });
    }

    private void showControlsHome() {
        currentScreen="controls_home";
        setContentView(R.layout.activity_controls_home);
        findViewById(R.id.touch_controls_button).setOnClickListener(v->showTouchScreen());
        findViewById(R.id.gamepad_controls_button).setOnClickListener(v->showControlsScreen());
        findViewById(R.id.back_button).setOnClickListener(v->showMainScreen());
    }

    private void addVibrationStrength(LinearLayout parent,String key) {
        SharedPreferences prefs=GamePreferences.get(this);
        TextView label=new TextView(this);label.setTextColor(getColor(R.color.ui_text));
        SeekBar slider=new SeekBar(this);slider.setMax(100);slider.setProgress(prefs.getInt(key,100));
        label.setText(getString(R.string.vibration_strength,slider.getProgress()));
        parent.addView(label);parent.addView(slider,new LinearLayout.LayoutParams(-1,dp(48)));
        slider.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener(){
            public void onProgressChanged(SeekBar bar,int value,boolean user){
                label.setText(getString(R.string.vibration_strength,value));
                if(user)prefs.edit().putInt(key,value).apply();
            }
            public void onStartTrackingTouch(SeekBar bar){}
            public void onStopTrackingTouch(SeekBar bar){}
        });
    }

    private void showControlsScreen()
    {
        currentScreen = "controls";
        setContentView(R.layout.activity_controller_mapping);
        findViewById(R.id.back_button).setOnClickListener(v -> showControlsHome());
        findViewById(R.id.gamepad_settings_button).setOnClickListener(v -> showGamepadSettings());
        LinearLayout container = findViewById(R.id.mapping_container);
        SharedPreferences preferences = GamePreferences.get(this);
        String[] actionLabels=GamePreferences.actionLabels(this);
        String[] buttonLabels=GamePreferences.physicalButtonLabels(this);
        ControllerDiagramView diagram=new ControllerDiagramView(this,button->{
            String[] choices=new String[actionLabels.length+1];choices[0]=getString(R.string.label_unassigned);
            System.arraycopy(actionLabels,0,choices,1,actionLabels.length);
            new AlertDialog.Builder(this).setTitle(getString(R.string.assign_button_title,buttonLabels[GamePreferences.indexOf(GamePreferences.PHYSICAL_BUTTON_VALUES,button)]))
                .setItems(choices,(dialog,which)->{
                    SharedPreferences.Editor edit=preferences.edit();
                    for(int i=0;i<GamePreferences.ACTION_IDS.length;i++)if(button.equals(GamePreferences.getPhysicalButton(preferences,i)))
                        edit.putString(GamePreferences.physicalKey(GamePreferences.ACTION_IDS[i]),"none");
                    if(which>0)edit.putString(GamePreferences.physicalKey(GamePreferences.ACTION_IDS[which-1]),button);
                    edit.apply();showControlsScreen();
                }).show();
        });
        ((android.widget.FrameLayout)findViewById(R.id.controller_diagram)).addView(diagram,new android.widget.FrameLayout.LayoutParams(-1,-1));
        findViewById(R.id.mappings_toggle).setOnClickListener(v->{
            boolean list=findViewById(R.id.mapping_scroll).getVisibility()!=View.VISIBLE;
            findViewById(R.id.mapping_scroll).setVisibility(list?View.VISIBLE:View.GONE);
            findViewById(R.id.controller_diagram).setVisibility(list?View.GONE:View.VISIBLE);
            ((Button)v).setText(list?R.string.mapping_show_diagram:R.string.mapping_show_list);
        });

        for (int i = 0; i < GamePreferences.ACTION_IDS.length; ++i)
        {
            final int actionIndex = i;
            LinearLayout row = new LinearLayout(this);
            row.setOrientation(LinearLayout.VERTICAL);
            row.setPadding(0, dp(8), 0, dp(8));

            TextView label = new TextView(this);
            label.setText(actionLabels[i]);
            label.setTextColor(Color.WHITE);
            label.setTextSize(16);
            row.addView(label);

            Spinner physical = new Spinner(this);
            ArrayAdapter<String> physicalAdapter = new ArrayAdapter<>(this,
                android.R.layout.simple_spinner_item, buttonLabels);
            physicalAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            physical.setAdapter(physicalAdapter);
            physical.setSelection(GamePreferences.indexOf(GamePreferences.PHYSICAL_BUTTON_VALUES,
                GamePreferences.getPhysicalButton(preferences, i)));
            physical.setOnItemSelectedListener(new SimpleItemSelectedListener(position -> {
                String button=GamePreferences.PHYSICAL_BUTTON_VALUES[position];
                if(button.equals(GamePreferences.getPhysicalButton(preferences,actionIndex)))return;
                SharedPreferences.Editor edit=preferences.edit();boolean conflict=false;
                if(!"none".equals(button))for(int j=0;j<GamePreferences.ACTION_IDS.length;j++)
                    if(j!=actionIndex&&button.equals(GamePreferences.getPhysicalButton(preferences,j))){
                        edit.putString(GamePreferences.physicalKey(GamePreferences.ACTION_IDS[j]),"none");conflict=true;
                    }
                edit.putString(GamePreferences.physicalKey(GamePreferences.ACTION_IDS[actionIndex]),button).apply();
                diagram.invalidate();if(conflict)showControlsScreen();
            }));
            row.addView(physical);

            Spinner key = new Spinner(this);
            ArrayAdapter<String> keyAdapter = new ArrayAdapter<>(this,
                android.R.layout.simple_spinner_item, GamePreferences.keyLabels(this));
            keyAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            key.setAdapter(keyAdapter);
            key.setSelection(GamePreferences.indexOf(GamePreferences.KEY_VALUES,
                GamePreferences.getTouchKey(preferences, GamePreferences.ACTION_IDS[i])));
            key.setOnItemSelectedListener(new SimpleItemSelectedListener(position ->
                preferences.edit().putInt(GamePreferences.touchKey(
                    GamePreferences.ACTION_IDS[actionIndex]),
                    GamePreferences.KEY_VALUES[position]).apply()));
            row.addView(key);
            container.addView(row);
        }
    }

    private void showGamepadSettings()
    {
        currentScreen="gamepad_settings";
        setContentView(R.layout.activity_controller_settings);
        findViewById(R.id.back_button).setOnClickListener(v -> showControlsScreen());
        LinearLayout options=findViewById(R.id.controller_options);
        addPreferenceCheck(options,getString(R.string.pref_gamepad_vibration),GamePreferences.GAMEPAD_VIBRATION,false);
        addVibrationStrength(options,GamePreferences.GAMEPAD_VIBRATION_STRENGTH);
    }

    private void showTouchScreen()
    {
        currentScreen = "touch";
        setContentView(R.layout.activity_touch_controls);
        findViewById(R.id.back_button).setOnClickListener(v -> showControlsHome());
        SharedPreferences preferences = GamePreferences.get(this);

        // Replace the old wheel/tilt placeholders with an actual interactive preview.
        preferences.edit().putString(GamePreferences.TOUCH_MODE, GamePreferences.TOUCH_MODE_BUTTONS).apply();
        touchPreview = new TouchControlsOverlay(this, true);
        TouchPreviewFrame.attach(this,findViewById(R.id.touch_preview),touchPreview);

        String[] layoutValues = {
            GamePreferences.TOUCH_LAYOUT_STANDARD,
            GamePreferences.TOUCH_LAYOUT_MIRRORED,
        };
        Spinner layout = findViewById(R.id.touch_layout_spinner);
        layout.setAdapter(stringAdapter(new String[] { getString(R.string.layout_right_handed), getString(R.string.layout_left_handed) }));
        layout.setSelection(GamePreferences.indexOf(layoutValues,
            preferences.getString(GamePreferences.TOUCH_LAYOUT,
                GamePreferences.TOUCH_LAYOUT_STANDARD)));
        layout.setOnItemSelectedListener(new SimpleItemSelectedListener(position -> {
            GamePreferences.setTouchLayout(preferences,layoutValues[position]);
            if (touchPreview != null) touchPreview.refreshSettings();
        }));

        bindSeekBar(R.id.opacity_seek, R.id.opacity_value, GamePreferences.TOUCH_OPACITY,
            20, 100, 65, "%");
        bindSeekBar(R.id.size_seek, R.id.size_value, GamePreferences.TOUCH_SIZE,
            70, 115, 100, "%");
        bindSeekBar(R.id.edge_seek, R.id.edge_value, GamePreferences.TOUCH_EDGE,0,32,0," dp");
        bindSeekBar(R.id.raise_seek, R.id.raise_value, GamePreferences.TOUCH_RAISE,0,24,0," dp");

        CheckBox autoHide = findViewById(R.id.auto_hide_check);
        autoHide.setChecked(preferences.getBoolean(GamePreferences.TOUCH_AUTO_HIDE, false));
        autoHide.setOnCheckedChangeListener((button, checked) ->
            preferences.edit().putBoolean(GamePreferences.TOUCH_AUTO_HIDE, checked).apply());
        LinearLayout options=findViewById(R.id.touch_options);
        addPreferenceCheck(options,getString(R.string.pref_phone_vibration),GamePreferences.TOUCH_VIBRATION,false);
        addVibrationStrength(options,GamePreferences.TOUCH_VIBRATION_STRENGTH);
        addPreferenceCheck(options,getString(R.string.pref_separate_layouts),GamePreferences.TOUCH_SEPARATE,false);
        addPreferenceCheck(options,getString(R.string.pref_hide_full),GamePreferences.TOUCH_HIDE_FULL,false);
        refreshTouchLayoutNote();
        TextView delay=new TextView(this);delay.setTextColor(getColor(R.color.ui_text));options.addView(delay);
        SeekBar seconds=new SeekBar(this);seconds.setMin(1);seconds.setMax(30);
        seconds.setProgress(preferences.getInt(GamePreferences.TOUCH_HIDE_SECONDS,4));
        delay.setText(getString(R.string.auto_hide_delay,seconds.getProgress()));
        options.addView(seconds,new LinearLayout.LayoutParams(-1,dp(48)));
        seconds.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener(){
            public void onProgressChanged(SeekBar b,int value,boolean user){delay.setText(getString(R.string.auto_hide_delay,value));if(user)preferences.edit().putInt(GamePreferences.TOUCH_HIDE_SECONDS,value).apply();}
            public void onStartTrackingTouch(SeekBar b){}
            public void onStopTrackingTouch(SeekBar b){}
        });
        Button editor=findViewById(R.id.edit_touch_layout);
        editor.setOnClickListener(v->{currentScreen="editor";setContentView(R.layout.activity_touch_editor);touchPreview=TouchLayoutEditor.attach(this,this::showTouchScreen);});
    }

    private void addPreferenceCheck(LinearLayout parent,String label,String key,boolean defaultValue) {
        SharedPreferences prefs=GamePreferences.get(this);CheckBox check=new CheckBox(this);
        check.setText(label);check.setTextColor(getColor(R.color.ui_text));check.setChecked(prefs.getBoolean(key,defaultValue));
        check.setOnCheckedChangeListener((v,on)->{prefs.edit().putBoolean(key,on).apply();
            if(touchPreview!=null)touchPreview.refreshSettings();refreshTouchLayoutNote();});
        parent.addView(check);
    }

    /** The note names the overlay's mode button, which only exists while menu and
     *  race layouts are separate -- with one shared layout there is nothing to
     *  switch, so the note goes away rather than pointing at a missing button. */
    private void refreshTouchLayoutNote()
    {
        TextView note = findViewById(R.id.touch_layout_note);
        if (note == null)
            return;
        if (!GamePreferences.get(this).getBoolean(GamePreferences.TOUCH_SEPARATE, false))
        {
            note.setVisibility(View.GONE);
            return;
        }
        note.setText(withControlIcons(getText(R.string.touch_digital_note),
            note.getTextSize(), note.getCurrentTextColor()));
        note.setVisibility(View.VISIBLE);
    }

    /** The overlay buttons carry no captions, so help text cannot name them.
     *  Each resource string carries the matching symbol instead, and this swaps
     *  every one for the very glyph the overlay draws. Keeping the symbol in the
     *  resource means translators see readable text and cannot lose the mark. */
    private CharSequence withControlIcons(CharSequence text, float textSize, int tint)
    {
        SpannableStringBuilder out = new SpannableStringBuilder(text);
        int size = Math.round(textSize * 1.4f);
        for (int i = out.length() - 1; i >= 0; --i)
        {
            int icon = controlIcon(out.charAt(i));
            if (icon == 0)
                continue;
            Drawable glyph = getDrawable(icon);
            if (glyph == null)
                continue;
            glyph = glyph.mutate();
            glyph.setTint(tint);
            glyph.setBounds(0, 0, size, size);
            out.setSpan(new ImageSpan(glyph, ImageSpan.ALIGN_CENTER), i, i + 1,
                Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
        }
        return out;
    }

    private static int controlIcon(char marker)
    {
        switch (marker)
        {
        case '☰': return R.drawable.ic_touch_mode;     // MENU / DRIVE
        case '✓': return R.drawable.ic_faq_confirm;    // OK
        case '←': return R.drawable.ic_faq_back;       // BACK
        case '↻': return R.drawable.ic_faq_reset;      // RESET
        case '☼': return R.drawable.ic_faq_lights;     // LIGHTS
        default:  return 0;
        }
    }

    /** Order of the help page.  One resource per question so that editing one
     *  answer does not invalidate a whole translated page. */
    private static final int[] FAQ_SECTIONS = {
        R.string.faq_layout_modes, R.string.faq_mouse, R.string.faq_size,
        R.string.faq_shared_layouts, R.string.faq_edges, R.string.faq_hiding,
        R.string.faq_vibration, R.string.faq_mapping,
        R.string.faq_lights, R.string.faq_screen_data,
    };

    /** Blank line between two FAQ answers. */
    private static final String SECTION_GAP = "\n\n";

    private void showFaq() {
        currentScreen="faq";setContentView(R.layout.activity_faq);
        findViewById(R.id.back_button).setOnClickListener(v->showMainScreen());
        // getText, not getString: the <b> markup in each answer is a span.
        CharSequence[] parts=new CharSequence[FAQ_SECTIONS.length*2-1];
        for(int i=0;i<FAQ_SECTIONS.length;i++) {
            parts[2*i]=getText(FAQ_SECTIONS[i]);
            if(i+1<FAQ_SECTIONS.length)parts[2*i+1]=SECTION_GAP;
        }
        TextView body=findViewById(R.id.faq_body);
        body.setText(withControlIcons(android.text.TextUtils.concat(parts),
            body.getTextSize(), body.getCurrentTextColor()));
    }

    private void showScreenScreen()
    {
        currentScreen = "screen";
        setContentView(R.layout.activity_screen_settings);
        findViewById(R.id.back_button).setOnClickListener(v -> showMainScreen());
        SharedPreferences preferences = GamePreferences.get(this);
        RadioButton auto = findViewById(R.id.orientation_auto);
        RadioButton landscape = findViewById(R.id.orientation_landscape);
        RadioButton reversed = findViewById(R.id.orientation_landscape_reverse);
        String orientation = preferences.getString(GamePreferences.ORIENTATION,
            GamePreferences.ORIENTATION_LANDSCAPE);
        auto.setChecked(GamePreferences.ORIENTATION_AUTO.equals(orientation));
        reversed.setChecked(GamePreferences.ORIENTATION_LANDSCAPE_REVERSE.equals(orientation));
        landscape.setChecked(!auto.isChecked() && !reversed.isChecked());

        /* Offered rather than typed: the cap is applied by skipping whole
         * display refreshes, so only divisors of the refresh rate are actually
         * reachable and a free number would quietly round to one of them. */
        RadioButton fps30 = findViewById(R.id.fps_cap_30);
        RadioButton fps60 = findViewById(R.id.fps_cap_60);
        boolean sixty = preferences.getInt(GamePreferences.FPS_CAP, 30) >= 60;
        fps60.setChecked(sixty);
        fps30.setChecked(!sixty);

        /* Percent on the bar, a multiplier in the shader: 100 is the picture
         * exactly as the game drew it.  Saved with the rest on Save rather than
         * applied live, so the whole screen behaves the one way. */
        SeekBar gamma = findViewById(R.id.gamma_seek);
        TextView gammaValue = findViewById(R.id.gamma_value);
        gamma.setMax(150);
        gamma.setProgress(Math.max(0, Math.min(150,
            preferences.getInt(GamePreferences.GAMMA, 100) - 100)));
        gammaValue.setText((gamma.getProgress() + 100) + "%");
        gamma.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override public void onProgressChanged(SeekBar bar, int value, boolean byUser) {
                gammaValue.setText((value + 100) + "%");
            }
            @Override public void onStartTrackingTouch(SeekBar bar) {}
            @Override public void onStopTrackingTouch(SeekBar bar) {}
        });

        findViewById(R.id.save_screen_button).setOnClickListener(v -> {
            preferences.edit()
                .putInt(GamePreferences.GAMMA, gamma.getProgress() + 100)
                .putString(GamePreferences.ORIENTATION,
                    auto.isChecked() ? GamePreferences.ORIENTATION_AUTO
                    : reversed.isChecked() ? GamePreferences.ORIENTATION_LANDSCAPE_REVERSE
                    : GamePreferences.ORIENTATION_LANDSCAPE)
                .putInt(GamePreferences.FPS_CAP, fps60.isChecked() ? 60 : 30)
                .apply();
            Toast.makeText(this, R.string.settings_saved, Toast.LENGTH_SHORT).show();
        });
    }

    private ArrayAdapter<String> stringAdapter(String[] values)
    {
        ArrayAdapter<String> adapter = new ArrayAdapter<>(this,
            android.R.layout.simple_spinner_item, values);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        return adapter;
    }

    private void bindSeekBar(int seekId, int valueId, String key, int minimum,
                             int maximum, int defaultValue, String suffix)
    {
        SharedPreferences preferences = GamePreferences.get(this);
        SeekBar seekBar = findViewById(seekId);
        TextView value = findViewById(valueId);
        seekBar.setMin(minimum);
        seekBar.setMax(maximum);
        seekBar.setProgress(preferences.getInt(key, defaultValue));
        value.setText(seekBar.getProgress() + suffix);
        seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar bar, int progress, boolean fromUser)
            {
                value.setText(progress + suffix);
                if (fromUser) {
                    preferences.edit().putInt(key, progress).apply();
                    if (touchPreview != null) touchPreview.refreshSettings();
                }
            }

            @Override public void onStartTrackingTouch(SeekBar bar) {}
            @Override public void onStopTrackingTouch(SeekBar bar) {}
        });
    }

    private void launchFolderPicker()
    {
        if (importThread != null)
            return;
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
        intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION
            | Intent.FLAG_GRANT_PERSISTABLE_URI_PERMISSION);
        startActivityForResult(intent, REQUEST_PICK_FOLDER);
    }

    private void launchZipPicker()
    {
        if (importThread != null)
            return;
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        intent.setType("application/zip");
        intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
        startActivityForResult(intent, REQUEST_PICK_ZIP);
    }

    /* Read from the stored preference, not from the checkbox: the data screen
     * is rebuilt whenever the system file picker returns, so the view has
     * already been replaced by the time an import or export reports back. */
    private boolean includeSettings()
    {
        return GamePreferences.get(this).getBoolean(GamePreferences.SAVES_INCLUDE_SETTINGS, false);
    }

    private void launchSaveImport()
    {
        if(importThread!=null)return;
        Intent intent=new Intent(Intent.ACTION_OPEN_DOCUMENT).addCategory(Intent.CATEGORY_OPENABLE).setType("application/zip");
        intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);startActivityForResult(intent,REQUEST_IMPORT_SAVES);
    }

    private void launchSaveExport()
    {
        if(importThread!=null)return;
        String stamp=new java.text.SimpleDateFormat("yyyyMMdd-HHmm",java.util.Locale.ROOT).format(new java.util.Date());
        Intent intent=new Intent(Intent.ACTION_CREATE_DOCUMENT).addCategory(Intent.CATEGORY_OPENABLE).setType("application/zip")
            .putExtra(Intent.EXTRA_TITLE,"nfs3-saves-"+stamp+".zip");
        intent.addFlags(Intent.FLAG_GRANT_WRITE_URI_PERMISSION);startActivityForResult(intent,REQUEST_EXPORT_SAVES);
    }

    private void launchLauncherImport()
    {
        if(importThread!=null)return;
        Intent intent=new Intent(Intent.ACTION_OPEN_DOCUMENT).addCategory(Intent.CATEGORY_OPENABLE).setType("*/*");
        intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);startActivityForResult(intent,REQUEST_IMPORT_LAUNCHER);
    }

    private void launchLauncherExport()
    {
        if(importThread!=null)return;
        String stamp=new java.text.SimpleDateFormat("yyyyMMdd-HHmm",java.util.Locale.ROOT).format(new java.util.Date());
        Intent intent=new Intent(Intent.ACTION_CREATE_DOCUMENT).addCategory(Intent.CATEGORY_OPENABLE).setType("application/json")
            .putExtra(Intent.EXTRA_TITLE,"nfs3-launcher-"+stamp+".json");
        intent.addFlags(Intent.FLAG_GRANT_WRITE_URI_PERMISSION);startActivityForResult(intent,REQUEST_EXPORT_LAUNCHER);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data)
    {
        super.onActivityResult(requestCode, resultCode, data);
        if (resultCode != Activity.RESULT_OK || data == null || data.getData() == null)
            return;
        Uri uri = data.getData();
        if (requestCode == REQUEST_PICK_FOLDER)
        {
            try
            {
                getContentResolver().takePersistableUriPermission(uri,
                    Intent.FLAG_GRANT_READ_URI_PERMISSION);
            }
            catch (SecurityException e)
            {
                Log.w(TAG, "Could not persist folder permission", e);
            }
            DocumentFile folder = DocumentFile.fromTreeUri(this, uri);
            String name = folder == null ? getString(R.string.data_set_name_folder) : folder.getName();
            startImport(name, temporary -> DataImporter.importFromTree(
                getApplicationContext(), uri, temporary, progressListener()));
        }
        else if (requestCode == REQUEST_PICK_ZIP)
        {
            startImport(displayName(uri), temporary -> DataImporter.importFromZip(
                getApplicationContext(), uri, temporary, progressListener()));
        }
        else if(requestCode==REQUEST_IMPORT_SAVES)
        {
            /* Read on the UI thread: the operation itself runs off it. */
            final boolean withSettings=includeSettings();
            startSaveOperation(()->{
                String backup=SaveGameManager.importZip(getApplicationContext(),uri,dataRoot(),withSettings);
                return backup.isEmpty()?getString(R.string.saves_import_complete_no_backup)
                    :getString(R.string.saves_import_complete,backup);
            });
        }
        else if(requestCode==REQUEST_EXPORT_SAVES)
        {
            final boolean withSettings=includeSettings();
            startSaveOperation(()->getString(R.string.saves_export_complete,
                SaveGameManager.exportZip(getApplicationContext(),uri,dataRoot(),withSettings)));
        }
        else if(requestCode==REQUEST_EXPORT_LAUNCHER)
        {
            startSaveOperation(()->getString(R.string.launcher_export_complete,
                LauncherSettings.export(getApplicationContext(),uri)));
        }
        else if(requestCode==REQUEST_IMPORT_LAUNCHER)
        {
            /* No recreate() afterwards: every screen reads these settings when
             * it is shown, and the language is deliberately not carried, so
             * nothing already on screen can go stale. */
            startSaveOperation(()->{
                String backup=LauncherSettings.importFrom(getApplicationContext(),uri,dataRoot());
                return backup.isEmpty()?getString(R.string.launcher_import_complete_no_backup)
                    :getString(R.string.launcher_import_complete,backup);
            });
        }
    }

    /* An operation can finish on any of the Data sub-screens, so redraw
     * whichever one is showing rather than sending the user somewhere else. */
    private void reshowDataSubScreen()
    {
        if ("game_saves".equals(currentScreen)) showGameSavesScreen();
        else if ("launcher_settings".equals(currentScreen)) showLauncherSettingsScreen();
        else showGameDataScreen();
    }

    private interface SaveOperation { String run() throws IOException; }

    private void startSaveOperation(SaveOperation operation)
    {
        if(importThread!=null)return;setDataImportUi(true);
        Thread worker=new Thread(()->{
            Thread running=Thread.currentThread();
            try {
                String message=operation.run();
                runOnUiThread(()->{if(importThread!=running||isDestroyed())return;importThread=null;Toast.makeText(this,message,Toast.LENGTH_LONG).show();reshowDataSubScreen();});
            } catch(IOException failure) {
                Log.e(TAG,"Save operation failed",failure);
                runOnUiThread(()->{if(importThread!=running||isDestroyed())return;importThread=null;setDataImportUi(false);String message=getString(R.string.data_error,failure.getMessage());((TextView)findViewById(R.id.data_status_text)).setText(message);Toast.makeText(this,message,Toast.LENGTH_LONG).show();});
            }
        },"nfs3-save-operation");importThread=worker;worker.start();
    }

    private DataImporter.ProgressListener progressListener()
    {
        Thread runningThread = Thread.currentThread();
        return (bytesCopied, filesCopied, currentPath) -> runOnUiThread(() -> {
            if (importThread != runningThread || isDestroyed() || !"data".equals(currentScreen))
                return;
            TextView status = findViewById(R.id.data_status_text);
            int mb = (int) (bytesCopied / (1024 * 1024));
            String name = currentPath.substring(currentPath.lastIndexOf(File.separatorChar) + 1);
            status.setText(getResources().getQuantityString(R.plurals.importing_progress,
                filesCopied, name, mb, filesCopied));
        });
    }

    private void startImport(String displayName, DataSetManager.ImportOperation operation)
    {
        if (importThread != null || dataSetManager == null)
            return;
        setDataImportUi(true);
        Thread worker = new Thread(() -> {
            Thread runningThread = Thread.currentThread();
            try
            {
                dataSetManager.importDataSet(displayName, operation);
                runOnUiThread(() -> {
                    if (importThread != runningThread || isDestroyed())
                        return;
                    importThread = null;
                    Toast.makeText(this, R.string.import_complete, Toast.LENGTH_SHORT).show();
                    showGameDataScreen();
                });
            }
            catch (IOException e)
            {
                if (runningThread.isInterrupted())
                    return;
                Log.e(TAG, "Data import failed", e);
                runOnUiThread(() -> {
                    if (importThread != runningThread || isDestroyed())
                        return;
                    importThread = null;
                    setDataImportUi(false);
                    String message = getString(R.string.import_failed, e.getMessage());
                    ((TextView) findViewById(R.id.data_status_text)).setText(message);
                    Toast.makeText(this, message, Toast.LENGTH_LONG).show();
                });
            }
        }, "nfs3-data-import");
        importThread = worker;
        worker.start();
    }

    /* Long operations now run from three different screens, and each carries
     * only its own buttons -- so every control is disabled by id if it happens
     * to be there, rather than assuming a single layout holds them all. */
    private void setDataImportUi(boolean running)
    {
        View progress = findViewById(R.id.data_progress);
        if (progress == null)
            return;
        progress.setVisibility(running ? View.VISIBLE : View.GONE);
        boolean savesAvailable = !running && dataSetManager != null && dataSetManager.active() != null;
        enableIfPresent(R.id.import_folder_button, !running);
        enableIfPresent(R.id.import_zip_button, !running);
        enableIfPresent(R.id.import_saves_button, savesAvailable);
        enableIfPresent(R.id.export_saves_button, savesAvailable);
        enableIfPresent(R.id.import_launcher_button, !running);
        enableIfPresent(R.id.export_launcher_button, !running);
        enableIfPresent(R.id.switch_data_button, !running);
        enableIfPresent(R.id.delete_data_button, !running);
        enableIfPresent(R.id.back_button, !running);
        TextView status = findViewById(R.id.data_status_text);
        if (running && status != null)
            status.setText(R.string.checking_game_data);
    }

    private void enableIfPresent(int id, boolean enabled)
    {
        View view = findViewById(id);
        if (view != null)
            view.setEnabled(enabled);
    }

    private void switchDataSet(DataSetManager.DataSet dataSet)
    {
        try
        {
            dataSetManager.activate(dataSet.id);
            showGameDataScreen();
        }
        catch (IOException e)
        {
            showDataError(e);
        }
    }

    private void confirmDelete(DataSetManager.DataSet dataSet)
    {
        new AlertDialog.Builder(this)
            .setTitle(R.string.delete_data_title)
            .setMessage(getString(R.string.delete_data_message, dataSet.name))
            .setNegativeButton(android.R.string.cancel, null)
            .setPositiveButton(R.string.delete, (dialog, which) -> {
                try
                {
                    dataSetManager.delete(dataSet.id);
                    showGameDataScreen();
                }
                catch (IOException e)
                {
                    showDataError(e);
                }
            })
            .show();
    }

    private void showDataError(Exception e)
    {
        Log.e(TAG, "Data-set operation failed", e);
        Toast.makeText(this, getString(R.string.data_error, e.getMessage()),
            Toast.LENGTH_LONG).show();
    }

    private String displayName(Uri uri)
    {
        try (Cursor cursor = getContentResolver().query(uri,
            new String[] { OpenableColumns.DISPLAY_NAME }, null, null, null))
        {
            if (cursor != null && cursor.moveToFirst())
                return cursor.getString(0);
        }
        catch (Exception e)
        {
            Log.w(TAG, "Could not read document display name", e);
        }
        return getString(R.string.data_set_name_zip);
    }

    private void startGame()
    {
        if (dataSetManager == null || dataSetManager.active() == null)
            return;
        Intent intent = new Intent(this, NFS3Activity.class);
        if (getIntent() != null && getIntent().getExtras() != null)
            intent.putExtras(getIntent().getExtras());
        startActivity(intent);
    }

    @Override
    public void onBackPressed()
    {
        if (importThread != null)
            return;
        if ("editor".equals(currentScreen))
            showTouchScreen();
        else if ("gamepad_settings".equals(currentScreen))
            showControlsScreen();
        else if ("touch".equals(currentScreen)||"controls".equals(currentScreen))
            showControlsHome();
        // The Data screens are a level deeper than the rest, so back goes to
        // their menu rather than all the way out to the launcher.
        else if ("game_data".equals(currentScreen)||"game_saves".equals(currentScreen)
                 ||"launcher_settings".equals(currentScreen))
            showDataScreen();
        else if (!"main".equals(currentScreen))
            showMainScreen();
        else
            super.onBackPressed();
    }

    @Override
    protected void onDestroy()
    {
        Thread worker = importThread;
        importThread = null;
        if (worker != null)
            worker.interrupt();
        super.onDestroy();
    }

    private int dp(int value)
    {
        return Math.round(value * getResources().getDisplayMetrics().density);
    }

    private interface PositionConsumer
    {
        void accept(int position);
    }

    private static final class SimpleItemSelectedListener
        implements android.widget.AdapterView.OnItemSelectedListener
    {
        private final PositionConsumer consumer;

        SimpleItemSelectedListener(PositionConsumer consumer)
        {
            this.consumer = consumer;
        }

        @Override
        public void onItemSelected(android.widget.AdapterView<?> parent, View view,
                                   int position, long id)
        {
            consumer.accept(position);
        }

        @Override
        public void onNothingSelected(android.widget.AdapterView<?> parent) {}
    }
}
