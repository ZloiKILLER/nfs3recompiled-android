package dev.nfs3hp.port;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Intent;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.graphics.drawable.Drawable;
import android.hardware.input.InputManager;
import android.net.Uri;
import android.os.Bundle;
import android.provider.OpenableColumns;
import android.text.InputType;
import android.text.Spannable;
import android.text.SpannableStringBuilder;
import android.text.style.ImageSpan;
import android.util.Log;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.ImageView;
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
    /* The Gamepads slot waiting for a button press, or -1.  While it waits, the
     * next press on any gamepad answers it (dispatchKeyEvent). */
    private int capturingSlot = -1;
    /* The press that answered is swallowed on its way back up as well:
     * otherwise its key-up would click whatever button had focus -- typically
     * Assign itself, which would start waiting all over again. */
    private int swallowKeyUp = KeyEvent.KEYCODE_UNKNOWN;
    /* Keeps the Gamepads screen honest while it is open: a pad switched on, or
     * one whose battery runs out, shows at once rather than on the next visit. */
    private final InputManager.InputDeviceListener gamepadListener =
        new InputManager.InputDeviceListener()
    {
        @Override public void onInputDeviceAdded(int deviceId) { refreshGamepadsScreen(); }
        @Override public void onInputDeviceRemoved(int deviceId) { refreshGamepadsScreen(); }
        @Override public void onInputDeviceChanged(int deviceId) { refreshGamepadsScreen(); }
    };
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
        /* On a desktop -- Samsung DeX -- the launcher is a window the player
         * sizes, so the manifest's landscape lock is let go: the task's window
         * is made to its shape, and the game that follows into it inherits it
         * (NFS3Activity.desktopWindow). */
        if (DesktopMode.active(this))
            setRequestedOrientation(android.content.pm.ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED);
        super.onCreate(savedInstanceState);
        // Removed in 0.75: fixed/editable positions supersede this global offset.
        GamePreferences.get(this).edit().remove("touch_raise_controls").apply();
        // 0.75: the pads' buttons mean one thing in a race and another in the
        // menus, and their racing set moved with it.
        GamepadButtons.migrateDefaults(GamePreferences.get(this));
        TouchControlsOverlay.anchorLegacyPositions(this);
        ((InputManager) getSystemService(INPUT_SERVICE))
            .registerInputDeviceListener(gamepadListener, null);
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
        ((TextView) findViewById(R.id.version_text)).setText(versionLabel());
    }

    /* Which build this is, under the menu: v0.73, or v0.73 DEBUG for a debug
     * build, which installs beside a release one under a package of its own. */
    private String versionLabel()
    {
        String name = "";
        try
        {
            name = getPackageManager().getPackageInfo(getPackageName(), 0).versionName;
        }
        catch (android.content.pm.PackageManager.NameNotFoundException ignored)
        {
        }
        boolean debug = (getApplicationInfo().flags & android.content.pm.ApplicationInfo.FLAG_DEBUGGABLE) != 0;
        return "v" + name + (debug ? " DEBUG" : "");
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
        findViewById(R.id.gamepad_controls_button).setOnClickListener(v->showGamepadsScreen());
        findViewById(R.id.back_button).setOnClickListener(v->showMainScreen());
    }

    /** Order of the split-screen help page: one resource per step, like the FAQ. */
    private static final int[] CONTROLS_HELP_SECTIONS = {
        R.string.help_split_setup, R.string.help_split_write, R.string.help_split_race,
        R.string.help_split_driving, R.string.help_split_notes,
    };

    /* Split screen in a few steps, found where a player looks for it: on the
     * Gamepads screen the steps walk through. */
    private void showControlsHelp() {
        currentScreen="controls_help";
        setContentView(R.layout.activity_controls_help);
        findViewById(R.id.back_button).setOnClickListener(v->showGamepadsScreen());
        // getText, not getString: the <b> markup in each section is a span.
        CharSequence[] parts=new CharSequence[CONTROLS_HELP_SECTIONS.length*2-1];
        for(int i=0;i<CONTROLS_HELP_SECTIONS.length;i++) {
            parts[2*i]=getText(CONTROLS_HELP_SECTIONS[i]);
            if(i+1<CONTROLS_HELP_SECTIONS.length)parts[2*i+1]=SECTION_GAP;
        }
        ((TextView)findViewById(R.id.controls_help_body)).setText(android.text.TextUtils.concat(parts));
    }

    /* Which physical pad is Gamepad 1 and which is Gamepad 2, what their
     * buttons do, and writing the port's controls into the game.  A pad is
     * picked by pressing a button on it rather than from a list, because two
     * pads of the same model share a name and a list could not tell them
     * apart. */
    private void showGamepadsScreen()
    {
        currentScreen = "gamepads";
        capturingSlot = -1;
        setContentView(R.layout.activity_gamepads);
        findViewById(R.id.back_button).setOnClickListener(v -> showControlsHome());
        findViewById(R.id.controls_help_button).setOnClickListener(v -> showControlsHelp());
        SharedPreferences preferences = GamePreferences.get(this);
        CheckBox vibration = findViewById(R.id.gamepad_vibration_check);
        vibration.setChecked(preferences.getBoolean(GamePreferences.GAMEPAD_VIBRATION, false));
        vibration.setOnCheckedChangeListener((box, on) ->
            preferences.edit().putBoolean(GamePreferences.GAMEPAD_VIBRATION, on).apply());
        int[][] ids = {
            { R.id.gamepad1_label, R.id.gamepad1_assign, R.id.gamepad1_auto, R.id.gamepad1_buttons },
            { R.id.gamepad2_label, R.id.gamepad2_assign, R.id.gamepad2_auto, R.id.gamepad2_buttons },
        };
        for (int slot = 0; slot < GamepadSlots.COUNT; ++slot)
        {
            final int index = slot;
            ((TextView) findViewById(ids[slot][0])).setText(getString(R.string.gamepad_slot_label, slot + 1));
            findViewById(ids[slot][1]).setOnClickListener(v -> {
                capturingSlot = index;
                refreshGamepadsScreen();
            });
            findViewById(ids[slot][2]).setOnClickListener(v -> {
                capturingSlot = -1;
                GamepadSlots.useAutomatic(preferences, index);
                refreshGamepadsScreen();
            });
            findViewById(ids[slot][3]).setOnClickListener(v -> showGamepadButtonsScreen(index));
        }
        findViewById(R.id.write_gamepad_controls)
            .setOnClickListener(v -> confirmWriteControls(ControlProfile.Kind.GAMEPADS));
        findViewById(R.id.write_keyboard_controls)
            .setOnClickListener(v -> confirmWriteControls(ControlProfile.Kind.KEYBOARD));
        refreshGamepadsScreen();
    }

    /* What each button of one pad does.  Stored as it is chosen, like the touch
     * keys; NFS3Activity hands it all to the native side when the game starts. */
    private void showGamepadButtonsScreen(int slot)
    {
        currentScreen = "gamepad_buttons";
        capturingSlot = -1;
        setContentView(R.layout.activity_gamepad_buttons);
        ((TextView) findViewById(R.id.gamepad_buttons_title))
            .setText(getString(R.string.gamepad_buttons_title, slot + 1));
        findViewById(R.id.back_button).setOnClickListener(v -> showGamepadsScreen());
        SharedPreferences preferences = GamePreferences.get(this);
        LinearLayout container = findViewById(R.id.gamepad_buttons_container);
        String[] buttonLabels = GamepadButtons.buttonLabels(this);
        String[] actionLabels = GamepadButtons.actionLabels(this);
        Spinner[] choices = new Spinner[GamepadButtons.BUTTON_IDS.length];
        for (int i = 0; i < GamepadButtons.BUTTON_IDS.length; ++i)
        {
            final int button = i;
            LinearLayout row = new LinearLayout(this);
            row.setOrientation(LinearLayout.HORIZONTAL);
            row.setGravity(android.view.Gravity.CENTER_VERTICAL);
            row.setPadding(0, dp(2), 0, dp(2));

            TextView label = new TextView(this);
            label.setText(buttonLabels[i]);
            label.setTextColor(getColor(R.color.ui_text));
            label.setTextSize(16);
            row.addView(label, new LinearLayout.LayoutParams(0, LinearLayout.LayoutParams.WRAP_CONTENT, 1f));

            Spinner action = new Spinner(this);
            action.setAdapter(stringAdapter(actionLabels));
            action.setSelection(GamePreferences.indexOf(GamepadButtons.ACTION_IDS,
                GamepadButtons.action(preferences, slot, i)));
            action.setOnItemSelectedListener(new SimpleItemSelectedListener(position ->
                GamepadButtons.setAction(preferences, slot, button, GamepadButtons.ACTION_IDS[position])));
            row.addView(action, new LinearLayout.LayoutParams(dp(240), dp(48)));
            container.addView(row);
            choices[i] = action;
        }
        findViewById(R.id.gamepad_buttons_reset).setOnClickListener(v -> {
            GamepadButtons.reset(preferences, slot);
            for (int button = 0; button < choices.length; ++button)
                choices[button].setSelection(GamePreferences.indexOf(GamepadButtons.ACTION_IDS,
                    GamepadButtons.DEFAULT_ACTIONS[button]));
        });
    }

    private void confirmWriteControls(ControlProfile.Kind kind)
    {
        boolean gamepads = kind == ControlProfile.Kind.GAMEPADS;
        new AlertDialog.Builder(this)
            .setTitle(gamepads ? R.string.controls_write_gamepads : R.string.controls_write_keyboard)
            .setMessage(gamepads ? R.string.controls_write_gamepads_message
                                 : R.string.controls_write_keyboard_message)
            .setNegativeButton(android.R.string.cancel, null)
            .setPositiveButton(android.R.string.ok, (dialog, which) -> writeControls(kind))
            .show();
    }

    /* Only while the game is not running: the game keeps config.dat in memory
     * and writes the whole file back, which would quietly undo this. */
    private void writeControls(ControlProfile.Kind kind)
    {
        if (GameProcess.running(this))
        {
            Toast.makeText(this, R.string.controls_write_game_running, Toast.LENGTH_LONG).show();
            return;
        }
        try
        {
            String backup = ControlProfile.writeToGame(dataRoot(), kind);
            Toast.makeText(this, getString(R.string.controls_write_done, backup), Toast.LENGTH_LONG).show();
            refreshGamepadsScreen();
        }
        catch (ControlProfile.NoSettingsException missing)
        {
            Toast.makeText(this, R.string.controls_write_no_config, Toast.LENGTH_LONG).show();
        }
        catch (IOException e)
        {
            Log.e(TAG, "Writing controls failed", e);
            Toast.makeText(this, getString(R.string.controls_write_failed, e.getMessage()),
                Toast.LENGTH_LONG).show();
        }
    }

    private void refreshGamepadsScreen()
    {
        if (!"gamepads".equals(currentScreen))
            return;
        SharedPreferences preferences = GamePreferences.get(this);
        GamepadSlots.Pad[] pads = GamepadSlots.resolve(preferences);
        int[] statusIds = { R.id.gamepad1_status, R.id.gamepad2_status };
        for (int slot = 0; slot < GamepadSlots.COUNT; ++slot)
        {
            boolean pinned = !GamepadSlots.assigned(preferences, slot).isEmpty();
            String text;
            if (capturingSlot == slot)
                text = getString(R.string.gamepad_assign_prompt, slot + 1);
            else if (pinned && pads[slot] != null)
                text = pads[slot].name;
            else if (pinned)
                text = getString(R.string.gamepad_slot_absent, GamepadSlots.assignedName(preferences, slot));
            else if (pads[slot] != null)
                text = getString(R.string.gamepad_slot_auto_now, pads[slot].name);
            else
                text = getString(R.string.gamepad_slot_auto_none);
            ((TextView) findViewById(statusIds[slot])).setText(text);
        }
        /* The game's own keyboard controls keep the pads quiet in a split-screen
         * race, where their keys would work the other player's car
         * (nfs3hp_main.cpp) -- said here, where Gamepad ON is. */
        File root = null;
        try { root = dataRoot(); } catch (IOException ignored) { }
        findViewById(R.id.gamepads_keyboard_note).setVisibility(
            root != null && ControlProfile.installedKind(root) == ControlProfile.Kind.KEYBOARD
                ? View.VISIBLE : View.GONE);
    }

    /* The key each on-screen control sends.  The overlay speaks to the game
     * through keys, so these have to match the game's settings -- which the
     * gamepad control set written from Controls -> Gamepads does out of the box. */
    private void showTouchKeysScreen()
    {
        currentScreen = "touch_keys";
        setContentView(R.layout.activity_touch_keys);
        findViewById(R.id.back_button).setOnClickListener(v -> showTouchScreen());
        LinearLayout container = findViewById(R.id.touch_keys_container);
        SharedPreferences preferences = GamePreferences.get(this);
        String[] actionLabels = GamePreferences.actionLabels(this);
        for (int i = 0; i < GamePreferences.ACTION_IDS.length; ++i)
        {
            final int actionIndex = i;
            LinearLayout row = new LinearLayout(this);
            row.setOrientation(LinearLayout.HORIZONTAL);
            row.setGravity(android.view.Gravity.CENTER_VERTICAL);
            row.setPadding(0, dp(2), 0, dp(2));

            TextView label = new TextView(this);
            label.setText(actionLabels[i]);
            label.setTextColor(getColor(R.color.ui_text));
            label.setTextSize(16);
            row.addView(label, new LinearLayout.LayoutParams(0, LinearLayout.LayoutParams.WRAP_CONTENT, 1f));

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
            row.addView(key, new LinearLayout.LayoutParams(dp(240), dp(48)));
            container.addView(row);
        }
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

        // How a finger works the game's pointer in the menus.
        final String[] pointerValues = {
            GamePreferences.TOUCH_POINTER_TAP,
            GamePreferences.TOUCH_POINTER_TOUCHPAD,
        };
        Spinner pointer = findViewById(R.id.touch_pointer_spinner);
        pointer.setAdapter(stringAdapter(new String[] { getString(R.string.touch_pointer_tap), getString(R.string.touch_pointer_touchpad) }));
        pointer.setSelection(GamePreferences.indexOf(pointerValues,
            preferences.getString(GamePreferences.TOUCH_POINTER, GamePreferences.TOUCH_POINTER_TAP)));
        pointer.setOnItemSelectedListener(new SimpleItemSelectedListener(position ->
            preferences.edit().putString(GamePreferences.TOUCH_POINTER, pointerValues[position]).apply()));

        bindSeekBar(R.id.opacity_seek, R.id.opacity_value, GamePreferences.TOUCH_OPACITY,
            20, 100, 65, "%");
        bindSeekBar(R.id.size_seek, R.id.size_value, GamePreferences.TOUCH_SIZE,
            70, 115, 100, "%");
        bindSeekBar(R.id.edge_seek, R.id.edge_value, GamePreferences.TOUCH_EDGE,0,32,0," dp");
        CheckBox enabled = findViewById(R.id.touch_enabled_check);
        enabled.setChecked(preferences.getBoolean(GamePreferences.TOUCH_ENABLED, true));
        enabled.setOnCheckedChangeListener((button, checked) ->
            preferences.edit().putBoolean(GamePreferences.TOUCH_ENABLED, checked).apply());

        CheckBox autoHide = findViewById(R.id.auto_hide_check);
        autoHide.setChecked(preferences.getBoolean(GamePreferences.TOUCH_AUTO_HIDE, false));
        autoHide.setOnCheckedChangeListener((button, checked) ->
            preferences.edit().putBoolean(GamePreferences.TOUCH_AUTO_HIDE, checked).apply());
        LinearLayout options=findViewById(R.id.touch_options);
        addPreferenceCheck(options,getString(R.string.pref_phone_vibration),GamePreferences.TOUCH_VIBRATION,false);
        addPreferenceCheck(options,getString(R.string.pref_hide_full),GamePreferences.TOUCH_HIDE_FULL,false);
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
        findViewById(R.id.touch_keys_button).setOnClickListener(v->showTouchKeysScreen());
    }

    private void addPreferenceCheck(LinearLayout parent,String label,String key,boolean defaultValue) {
        SharedPreferences prefs=GamePreferences.get(this);CheckBox check=new CheckBox(this);
        check.setText(label);check.setTextColor(getColor(R.color.ui_text));check.setChecked(prefs.getBoolean(key,defaultValue));
        check.setOnCheckedChangeListener((v,on)->{prefs.edit().putBoolean(key,on).apply();
            if(touchPreview!=null)touchPreview.refreshSettings();});
        parent.addView(check);
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
            /* Centring a span on the line came in Android 10; before it the
             * constant means nothing and the glyph would sit on the baseline
             * anyway, so ask for the baseline there and say so. */
            int align = android.os.Build.VERSION.SDK_INT >= 29
                ? ImageSpan.ALIGN_CENTER : ImageSpan.ALIGN_BASELINE;
            out.setSpan(new ImageSpan(glyph, align), i, i + 1,
                Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
        }
        return out;
    }

    private static int controlIcon(char marker)
    {
        switch (marker)
        {
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
        R.string.faq_edges, R.string.faq_hiding,
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

        findViewById(R.id.screen_adjustment_button)
            .setOnClickListener(v -> showScreenAdjustmentScreen());

        /* Written the moment a choice is made, like every other launcher
         * setting; the game picks both up when it next starts. */
        ((RadioGroup) findViewById(R.id.orientation_group)).setOnCheckedChangeListener((group, id) ->
            preferences.edit().putString(GamePreferences.ORIENTATION,
                id == R.id.orientation_auto ? GamePreferences.ORIENTATION_AUTO
                : id == R.id.orientation_landscape_reverse ? GamePreferences.ORIENTATION_LANDSCAPE_REVERSE
                : GamePreferences.ORIENTATION_LANDSCAPE).apply());
        ((RadioGroup) findViewById(R.id.fps_cap_group)).setOnCheckedChangeListener((group, id) ->
            preferences.edit().putInt(GamePreferences.FPS_CAP, id == R.id.fps_cap_60 ? 60 : 30).apply());
    }

    /* Gamma, brightness and contrast over two sample frames -- one night, one
     * daylight.  Two samples because the three pull the ends of the range in
     * opposite directions: what rescues a night track washes out a daylit one,
     * and only seeing both at once shows the trade.  The samples run through
     * the same curve the game's final blit uses (ScreenAdjustment mirrors the
     * shader), so what moves here is what a race will look like.  Written as
     * it changes, like every other launcher setting; Reset goes back to the
     * picture as the game drew it. */
    private void showScreenAdjustmentScreen()
    {
        currentScreen = "screen_adjustment";
        setContentView(R.layout.activity_screen_adjustment);
        findViewById(R.id.back_button).setOnClickListener(v -> showScreenScreen());
        SharedPreferences preferences = GamePreferences.get(this);

        /* Day first, then night: the second is a swipe away, and the dots under
         * the picture say which of the two is up. */
        ImageView sampleView = findViewById(R.id.sample_image);
        TextView sampleCaption = findViewById(R.id.sample_caption);
        View[] dots = { findViewById(R.id.sample_dot_day), findViewById(R.id.sample_dot_night) };
        int[] captions = { R.string.sample_bright, R.string.sample_dark };
        Bitmap[] sources = {
            BitmapFactory.decodeResource(getResources(), R.drawable.adjust_sample_bright),
            BitmapFactory.decodeResource(getResources(), R.drawable.adjust_sample_dark),
        };
        Bitmap[] shown = {
            sources[0].copy(Bitmap.Config.ARGB_8888, true),
            sources[1].copy(Bitmap.Config.ARGB_8888, true),
        };
        int[] current = { 0 };
        Runnable showSample = () -> {
            int index = current[0];
            sampleView.setImageBitmap(shown[index]);
            sampleCaption.setText(captions[index]);
            sampleView.setContentDescription(getString(captions[index]));
            for (int i = 0; i < dots.length; ++i)
            {
                android.graphics.drawable.GradientDrawable dot = new android.graphics.drawable.GradientDrawable();
                dot.setShape(android.graphics.drawable.GradientDrawable.OVAL);
                dot.setColor(getColor(i == index ? R.color.ui_accent : R.color.ui_border));
                dots[i].setBackground(dot);
            }
        };
        showSample.run();
        attachSampleSwipe(sampleView, dots, current, showSample);
        /* One buffer for both, reused on every slider step: a preview that
         * allocated a megabyte per frame would stutter exactly while being
         * judged. */
        int[] scratch = new int[Math.max(sources[0].getWidth() * sources[0].getHeight(),
                                         sources[1].getWidth() * sources[1].getHeight())];

        SeekBar gamma = findViewById(R.id.gamma_seek);
        SeekBar brightness = findViewById(R.id.brightness_seek);
        SeekBar contrast = findViewById(R.id.contrast_seek);
        TextView gammaValue = findViewById(R.id.gamma_value);
        TextView brightnessValue = findViewById(R.id.brightness_value);
        TextView contrastValue = findViewById(R.id.contrast_value);

        gamma.setMax(ScreenAdjustment.GAMMA_MAX - ScreenAdjustment.GAMMA_MIN);
        brightness.setMax(ScreenAdjustment.BRIGHTNESS_MAX - ScreenAdjustment.BRIGHTNESS_MIN);
        contrast.setMax(ScreenAdjustment.CONTRAST_MAX - ScreenAdjustment.CONTRAST_MIN);

        Runnable refresh = () -> {
            int g = gamma.getProgress() + ScreenAdjustment.GAMMA_MIN;
            int b = brightness.getProgress() + ScreenAdjustment.BRIGHTNESS_MIN;
            int c = contrast.getProgress() + ScreenAdjustment.CONTRAST_MIN;
            gammaValue.setText(g + "%");
            brightnessValue.setText(b + "%");
            contrastValue.setText(c + "%");
            int[] curve = ScreenAdjustment.curve(g, b, c);
            for (int i = 0; i < sources.length; ++i)
                ScreenAdjustment.apply(sources[i], shown[i], curve, scratch);
            sampleView.invalidate();
        };
        Runnable save = () -> preferences.edit()
            .putInt(GamePreferences.GAMMA, gamma.getProgress() + ScreenAdjustment.GAMMA_MIN)
            .putInt(GamePreferences.BRIGHTNESS, brightness.getProgress() + ScreenAdjustment.BRIGHTNESS_MIN)
            .putInt(GamePreferences.CONTRAST, contrast.getProgress() + ScreenAdjustment.CONTRAST_MIN)
            .apply();
        SeekBar.OnSeekBarChangeListener listener = new SeekBar.OnSeekBarChangeListener() {
            @Override public void onProgressChanged(SeekBar bar, int value, boolean byUser) { refresh.run(); if (byUser) save.run(); }
            @Override public void onStartTrackingTouch(SeekBar bar) {}
            @Override public void onStopTrackingTouch(SeekBar bar) {}
        };
        gamma.setOnSeekBarChangeListener(listener);
        brightness.setOnSeekBarChangeListener(listener);
        contrast.setOnSeekBarChangeListener(listener);

        gamma.setProgress(ScreenAdjustment.gammaPercent(preferences) - ScreenAdjustment.GAMMA_MIN);
        brightness.setProgress(ScreenAdjustment.brightnessPercent(preferences) - ScreenAdjustment.BRIGHTNESS_MIN);
        contrast.setProgress(ScreenAdjustment.contrastPercent(preferences) - ScreenAdjustment.CONTRAST_MIN);
        refresh.run();

        findViewById(R.id.reset_adjust_button).setOnClickListener(v -> {
            gamma.setProgress(ScreenAdjustment.NEUTRAL - ScreenAdjustment.GAMMA_MIN);
            brightness.setProgress(ScreenAdjustment.NEUTRAL - ScreenAdjustment.BRIGHTNESS_MIN);
            contrast.setProgress(ScreenAdjustment.NEUTRAL - ScreenAdjustment.CONTRAST_MIN);
            refresh.run();
            save.run();
        });
    }

    /* The sample follows a finger sideways and, let go past a fifth of its width,
     * slides out and comes back as the other one; a shorter move springs back.
     * A tap on a dot turns to its sample as well. */
    private void attachSampleSwipe(ImageView sampleView, View[] dots, int[] current, Runnable showSample)
    {
        /* Out to the left for the next sample, to the right for the one before,
         * and in again from the other side. */
        java.util.function.BiConsumer<Integer, Boolean> slideTo = (index, leftwards) -> {
            float out = Math.max(1, sampleView.getWidth()) * (leftwards ? -1 : 1);
            sampleView.animate().cancel();
            sampleView.animate().translationX(out).alpha(0).setDuration(120).withEndAction(() -> {
                current[0] = index;
                showSample.run();
                sampleView.setTranslationX(-out);
                sampleView.animate().translationX(0).alpha(1).setDuration(160).start();
            }).start();
        };
        for (int i = 0; i < dots.length; ++i)
        {
            final int index = i;
            dots[i].setOnClickListener(v -> {
                if (index != current[0])
                    slideTo.accept(index, index > current[0]);
            });
        }
        float[] downX = { 0 };
        sampleView.setOnTouchListener((v, event) -> {
            switch (event.getActionMasked())
            {
            case android.view.MotionEvent.ACTION_DOWN:
                downX[0] = event.getRawX();
                v.animate().cancel();
                v.setAlpha(1);
                return true;
            case android.view.MotionEvent.ACTION_MOVE:
                v.setTranslationX(event.getRawX() - downX[0]);
                return true;
            case android.view.MotionEvent.ACTION_UP:
            {
                float moved = event.getRawX() - downX[0];
                if (Math.abs(moved) > v.getWidth() / 5f)
                    slideTo.accept((current[0] + (moved < 0 ? 1 : dots.length - 1)) % dots.length, moved < 0);
                else
                {
                    v.animate().translationX(0).setDuration(150).start();
                    v.performClick();
                }
                return true;
            }
            case android.view.MotionEvent.ACTION_CANCEL:
                v.animate().translationX(0).alpha(1).setDuration(150).start();
                return true;
            default:
                return false;
            }
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
                // A file from before may carry control positions the old way.
                TouchControlsOverlay.anchorLegacyPositions(this);
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

    /* How far along an import is, on whichever data screen shows it.  It used
     * to ask for the Data screen by name, and imports run from Game data, so
     * nothing but an endless bar ever showed there. */
    private DataImporter.ProgressListener progressListener()
    {
        Thread runningThread = Thread.currentThread();
        return (done, total, currentPath) -> runOnUiThread(() -> {
            if (importThread != runningThread || isDestroyed())
                return;
            showImportProgress(this, findViewById(R.id.data_progress), findViewById(R.id.data_status_text),
                DataImporter.ProgressListener.percent(done, total), currentPath);
        });
    }

    /** One step of an import, the same on the first run's screen and on Game
     *  data: the bar fills to the percentage and the text names the file and
     *  says it; while the folder is still being measured the bar runs on its
     *  own and the text says so. */
    static void showImportProgress(Activity activity, ProgressBar bar, TextView status, int percent, String path)
    {
        if (bar == null || status == null)
            return;
        if (percent < 0)
        {
            bar.setIndeterminate(true);
            status.setText(R.string.checking_game_data);
            return;
        }
        bar.setIndeterminate(false);
        bar.setMax(100);
        bar.setProgress(percent);
        String name = path.substring(path.lastIndexOf(File.separatorChar) + 1);
        status.setText(activity.getString(R.string.importing_percent, name, percent));
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
                List<String> missing = new java.util.ArrayList<>();
                dataSetManager.importDataSet(displayName, temporary -> {
                    operation.run(temporary);
                    missing.addAll(missingGameFiles(getApplicationContext(), temporary));
                });
                runOnUiThread(() -> {
                    if (importThread != runningThread || isDestroyed())
                        return;
                    importThread = null;
                    Toast.makeText(this, R.string.import_complete, Toast.LENGTH_SHORT).show();
                    showGameDataScreen();
                    showMissingGameFiles(this, missing, null);
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

    /* What an import copied without: the game would only find out half way
     * into loading a race (DataImporter.missingFiles).  A failed check is no
     * reason to fail the import, so it only goes to the log. */
    static List<String> missingGameFiles(android.content.Context context, File root)
    {
        try
        {
            return DataImporter.missingFiles(context, root);
        }
        catch (IOException e)
        {
            Log.w(TAG, "Could not check the imported game data", e);
            return new java.util.ArrayList<>();
        }
    }

    /** Tells the player, once the import is through, which of the game's files
     *  it did not bring; `then` runs when the message is closed, or at once if
     *  nothing is missing. */
    static void showMissingGameFiles(Activity activity, List<String> missing, Runnable then)
    {
        if (missing.isEmpty())
        {
            if (then != null) then.run();
            return;
        }
        new AlertDialog.Builder(activity)
            .setTitle(R.string.data_missing_title)
            .setMessage(DataImporter.describeMissing(activity, missing))
            .setPositiveButton(android.R.string.ok, null)
            .setOnDismissListener(d -> { if (then != null) then.run(); })
            .show();
    }

    /* Long operations now run from three different screens, and each carries
     * only its own buttons -- so every control is disabled by id if it happens
     * to be there, rather than assuming a single layout holds them all. */
    private void setDataImportUi(boolean running)
    {
        ProgressBar progress = findViewById(R.id.data_progress);
        if (progress == null)
            return;
        progress.setVisibility(running ? View.VISIBLE : View.GONE);
        // Endless until an import says how far it is; saves never do.
        progress.setIndeterminate(true);
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
        /* Game data imported before the cop's map came as a minimap gets it
         * now, once -- while the game is not running to save over it. */
        if (!GameProcess.running(this))
        {
            try { ControlProfile.copMinimapOnce(dataRoot()); }
            catch (IOException e) { Log.w(TAG, "No data root for the cop's minimap", e); }
        }
        Intent intent = new Intent(this, NFS3Activity.class);
        if (getIntent() != null && getIntent().getExtras() != null)
            intent.putExtras(getIntent().getExtras());
        startActivity(intent);
    }

    /* While a Gamepads slot waits for its pad, the next button pressed on any
     * gamepad answers it.  Only gamepad presses count, and only then; the rest
     * of the time the launcher stays navigable with a pad as usual. */
    @Override
    public boolean dispatchKeyEvent(KeyEvent event)
    {
        if (event.getAction() == KeyEvent.ACTION_UP && event.getKeyCode() == swallowKeyUp)
        {
            swallowKeyUp = KeyEvent.KEYCODE_UNKNOWN;
            return true;
        }
        if (capturingSlot >= 0 && event.getAction() == KeyEvent.ACTION_DOWN)
        {
            InputDevice device = event.getDevice();
            if (GamepadSlots.isGamepad(device))
            {
                GamepadSlots.assign(GamePreferences.get(this), capturingSlot,
                    device.getDescriptor(), device.getName());
                capturingSlot = -1;
                swallowKeyUp = event.getKeyCode();
                refreshGamepadsScreen();
                return true;
            }
        }
        return super.dispatchKeyEvent(event);
    }

    @Override
    public void onBackPressed()
    {
        if (importThread != null)
            return;
        // Waiting for a pad is cancelled, not the screen it waits on.
        if (capturingSlot >= 0)
        {
            capturingSlot = -1;
            refreshGamepadsScreen();
            return;
        }
        if ("editor".equals(currentScreen))
            showTouchScreen();
        else if ("touch_keys".equals(currentScreen))
            showTouchScreen();
        else if ("touch".equals(currentScreen)||"gamepads".equals(currentScreen))
            showControlsHome();
        // Split screen help and a pad's buttons both open from the Gamepads screen.
        else if ("gamepad_buttons".equals(currentScreen)||"controls_help".equals(currentScreen))
            showGamepadsScreen();
        // The Data screens are a level deeper than the rest, so back goes to
        // their menu rather than all the way out to the launcher.
        else if ("game_data".equals(currentScreen)||"game_saves".equals(currentScreen)
                 ||"launcher_settings".equals(currentScreen))
            showDataScreen();
        // Screen adjustment sits under Display, so back returns there.
        else if ("screen_adjustment".equals(currentScreen))
            showScreenScreen();
        else if (!"main".equals(currentScreen))
            showMainScreen();
        else
            super.onBackPressed();
    }

    @Override
    protected void onDestroy()
    {
        ((InputManager) getSystemService(INPUT_SERVICE)).unregisterInputDeviceListener(gamepadListener);
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
