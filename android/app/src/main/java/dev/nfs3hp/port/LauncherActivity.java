package dev.nfs3hp.port;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Intent;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.graphics.Color;
import android.net.Uri;
import android.os.Bundle;
import android.provider.OpenableColumns;
import android.text.InputType;
import android.util.Log;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.RadioButton;
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
            android.graphics.Insets safe=insets.getInsets(android.view.WindowInsets.Type.systemBars()
                | android.view.WindowInsets.Type.displayCutout());
            view.setPadding(l+safe.left,t+safe.top,r+safe.right,b+safe.bottom);
            return insets;
        });
        root.requestApplyInsets();
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
            throw new IOException("External storage is not available");
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
        findViewById(R.id.controls_button).setOnClickListener(v -> showControlsScreen());
        findViewById(R.id.touch_controls_button).setOnClickListener(v -> showTouchScreen());
        findViewById(R.id.screen_settings_button).setOnClickListener(v -> showScreenScreen());
        findViewById(R.id.faq_button).setOnClickListener(v -> showFaq());
    }

    private void showDataScreen()
    {
        currentScreen = "data";
        setContentView(R.layout.activity_data_settings);
        findViewById(R.id.back_button).setOnClickListener(v -> showMainScreen());
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

    private void showControlsScreen()
    {
        currentScreen = "controls";
        setContentView(R.layout.activity_controller_mapping);
        findViewById(R.id.back_button).setOnClickListener(v -> showMainScreen());
        LinearLayout container = findViewById(R.id.mapping_container);
        SharedPreferences preferences = GamePreferences.get(this);
        ControllerDiagramView diagram=new ControllerDiagramView(this,button->{
            String[] choices=new String[GamePreferences.ACTION_LABELS.length+1];choices[0]="Unassigned";
            System.arraycopy(GamePreferences.ACTION_LABELS,0,choices,1,GamePreferences.ACTION_LABELS.length);
            new AlertDialog.Builder(this).setTitle("Assign "+GamePreferences.PHYSICAL_BUTTON_LABELS[GamePreferences.indexOf(GamePreferences.PHYSICAL_BUTTON_VALUES,button)])
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
            ((Button)v).setText(list?"Controller diagram":"All actions & keyboard keys");
        });

        for (int i = 0; i < GamePreferences.ACTION_IDS.length; ++i)
        {
            final int actionIndex = i;
            LinearLayout row = new LinearLayout(this);
            row.setOrientation(LinearLayout.VERTICAL);
            row.setPadding(0, dp(8), 0, dp(8));

            TextView label = new TextView(this);
            label.setText(GamePreferences.ACTION_LABELS[i]);
            label.setTextColor(Color.WHITE);
            label.setTextSize(16);
            row.addView(label);

            Spinner physical = new Spinner(this);
            ArrayAdapter<String> physicalAdapter = new ArrayAdapter<>(this,
                android.R.layout.simple_spinner_item, GamePreferences.PHYSICAL_BUTTON_LABELS);
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
                android.R.layout.simple_spinner_item, GamePreferences.KEY_LABELS);
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

    private void showTouchScreen()
    {
        currentScreen = "touch";
        setContentView(R.layout.activity_touch_controls);
        findViewById(R.id.back_button).setOnClickListener(v -> showMainScreen());
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
            preferences.edit().putString(GamePreferences.TOUCH_LAYOUT, layoutValues[position]).apply();
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
        addPreferenceCheck(options,"Separate menu and race layouts",GamePreferences.TOUCH_SEPARATE,false);
        addPreferenceCheck(options,"Show gear − / + buttons",GamePreferences.TOUCH_GEARS,false);
        addPreferenceCheck(options,"Hide completely (otherwise faint silhouette)",GamePreferences.TOUCH_HIDE_FULL,false);
        TextView delay=new TextView(this);delay.setTextColor(getColor(R.color.ui_text));options.addView(delay);
        SeekBar seconds=new SeekBar(this);seconds.setMin(1);seconds.setMax(30);
        seconds.setProgress(preferences.getInt(GamePreferences.TOUCH_HIDE_SECONDS,4));
        delay.setText("Auto-hide delay: "+seconds.getProgress()+" s");
        options.addView(seconds,new LinearLayout.LayoutParams(-1,dp(48)));
        seconds.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener(){
            public void onProgressChanged(SeekBar b,int value,boolean user){delay.setText("Auto-hide delay: "+value+" s");if(user)preferences.edit().putInt(GamePreferences.TOUCH_HIDE_SECONDS,value).apply();}
            public void onStartTrackingTouch(SeekBar b){}
            public void onStopTrackingTouch(SeekBar b){}
        });
        Button editor=findViewById(R.id.edit_touch_layout);
        editor.setOnClickListener(v->{currentScreen="editor";setContentView(R.layout.activity_touch_editor);touchPreview=TouchLayoutEditor.attach(this,this::showTouchScreen);});
    }

    private void addPreferenceCheck(LinearLayout parent,String label,String key,boolean defaultValue) {
        SharedPreferences prefs=GamePreferences.get(this);CheckBox check=new CheckBox(this);
        check.setText(label);check.setTextColor(getColor(R.color.ui_text));check.setChecked(prefs.getBoolean(key,defaultValue));
        check.setOnCheckedChangeListener((v,on)->{prefs.edit().putBoolean(key,on).apply();if(touchPreview!=null)touchPreview.refreshSettings();});
        parent.addView(check);
    }

    private void showFaq() {
        currentScreen="faq";setContentView(R.layout.activity_faq);
        findViewById(R.id.back_button).setOnClickListener(v->showMainScreen());
    }

    private void showScreenScreen()
    {
        currentScreen = "screen";
        setContentView(R.layout.activity_screen_settings);
        findViewById(R.id.back_button).setOnClickListener(v -> showMainScreen());
        SharedPreferences preferences = GamePreferences.get(this);
        RadioButton auto = findViewById(R.id.orientation_auto);
        RadioButton landscape = findViewById(R.id.orientation_landscape);
        String orientation = preferences.getString(GamePreferences.ORIENTATION,
            GamePreferences.ORIENTATION_LANDSCAPE);
        auto.setChecked(GamePreferences.ORIENTATION_AUTO.equals(orientation));
        landscape.setChecked(!auto.isChecked());

        EditText fps = findViewById(R.id.fps_cap_edit);
        fps.setInputType(InputType.TYPE_CLASS_NUMBER);
        fps.setText(Integer.toString(preferences.getInt(GamePreferences.FPS_CAP, 30)));
        findViewById(R.id.save_screen_button).setOnClickListener(v -> {
            int cap;
            try
            {
                cap = Integer.parseInt(fps.getText().toString().trim());
            }
            catch (NumberFormatException e)
            {
                fps.setError(getString(R.string.fps_error));
                return;
            }
            if (cap < 1 || cap > 240)
            {
                fps.setError(getString(R.string.fps_error));
                return;
            }
            preferences.edit()
                .putString(GamePreferences.ORIENTATION, auto.isChecked()
                    ? GamePreferences.ORIENTATION_AUTO : GamePreferences.ORIENTATION_LANDSCAPE)
                .putInt(GamePreferences.FPS_CAP, cap)
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
            String name = folder == null ? "Imported folder" : folder.getName();
            startImport(name, temporary -> DataImporter.importFromTree(
                getApplicationContext(), uri, temporary, progressListener()));
        }
        else if (requestCode == REQUEST_PICK_ZIP)
        {
            startImport(displayName(uri), temporary -> DataImporter.importFromZip(
                getApplicationContext(), uri, temporary, progressListener()));
        }
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
            status.setText(getString(R.string.importing_progress, name, mb, filesCopied));
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
                    showDataScreen();
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

    private void setDataImportUi(boolean running)
    {
        if (!"data".equals(currentScreen))
            return;
        findViewById(R.id.data_progress).setVisibility(running ? View.VISIBLE : View.GONE);
        findViewById(R.id.import_folder_button).setEnabled(!running);
        findViewById(R.id.import_zip_button).setEnabled(!running);
        findViewById(R.id.switch_data_button).setEnabled(!running);
        findViewById(R.id.delete_data_button).setEnabled(!running);
        findViewById(R.id.back_button).setEnabled(!running);
        if (running)
            ((TextView) findViewById(R.id.data_status_text)).setText(R.string.checking_game_data);
    }

    private void switchDataSet(DataSetManager.DataSet dataSet)
    {
        try
        {
            dataSetManager.activate(dataSet.id);
            showDataScreen();
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
                    showDataScreen();
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
        return "Imported ZIP";
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
