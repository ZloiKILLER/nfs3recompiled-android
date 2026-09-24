package dev.nfs3hp.port

import android.app.Activity
import android.content.Context
import android.content.Intent
import android.content.pm.ActivityInfo
import android.content.pm.ApplicationInfo
import android.content.pm.PackageManager
import android.hardware.input.InputManager
import android.net.Uri
import android.os.Bundle
import android.provider.OpenableColumns
import android.util.Log
import android.view.KeyEvent
import android.view.View
import android.view.ViewGroup
import android.widget.Toast
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.documentfile.provider.DocumentFile
import java.io.File
import java.io.IOException
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale

/**
 * The launcher: Play, and every setting the game cannot ask for itself.  The
 * screens are Compose (LauncherScreens.kt) in the game's own colours
 * (LauncherTheme.kt); what they do -- imports, saves, the game's settings file
 * -- stays with the classes that always did it.  One screen is still a View:
 * the touch layout editor, which is the live overlay itself.
 */
class LauncherActivity : ComponentActivity() {

    enum class Screen {
        Main, Language, Data, GameData, DataSets, Saves, LauncherSettings,
        Controls, Touch, TouchKeys, Editor, Gamepads, GamepadButtons, ControlsHelp,
        Faq, Display, Adjustment,
    }

    /** A question put to the player: a title, the question, and what the
     *  confirming button says and does.  Null `onConfirm`: only OK. */
    class Dialog(
        val title: String,
        val message: String,
        val confirm: String,
        val onConfirm: (() -> Unit)?,
        val onClose: (() -> Unit)? = null,
    )

    var screen by mutableStateOf(Screen.Main)
        private set
    /** Which pad's buttons the GamepadButtons screen shows. */
    var buttonsSlot by mutableIntStateOf(0)
        private set
    /** The Gamepads slot waiting for a button press, or -1.  While it waits,
     *  the next press on any gamepad answers it (dispatchKeyEvent). */
    var capturingSlot by mutableIntStateOf(-1)
    /** Counts pads coming and going, so the Gamepads screen reads them anew. */
    var padsChanged by mutableIntStateOf(0)
        private set
    /** An import, export or restore is running: the screens hold still. */
    var busy by mutableStateOf(false)
        private set
    /** How far it is, 0 to 100, or -1 while that is not known. */
    var importPercent by mutableIntStateOf(-1)
        private set
    var status by mutableStateOf("")
        private set
    var dataSets by mutableStateOf<List<DataSetManager.DataSet>>(emptyList())
        private set
    var activeSet by mutableStateOf<DataSetManager.DataSet?>(null)
        private set
    var dialog by mutableStateOf<Dialog?>(null)

    /* The press that answered a waiting slot is swallowed on its way back up
     * as well: otherwise its key-up would click whatever had focus. */
    private var swallowKeyUp = KeyEvent.KEYCODE_UNKNOWN
    private var worker: Thread? = null
    private var dataSetManager: DataSetManager? = null
    /** The overlay drawn on the Touch screen or in the editor, while one is. */
    var touchPreview: TouchControlsOverlay? = null

    private val gamepadListener = object : InputManager.InputDeviceListener {
        override fun onInputDeviceAdded(deviceId: Int) { padsChanged++ }
        override fun onInputDeviceRemoved(deviceId: Int) { padsChanged++ }
        override fun onInputDeviceChanged(deviceId: Int) { padsChanged++ }
    }

    /* Language of the launcher, chosen on its own screen.  LocaleHelper
     * returns the context unchanged while the setting is "system". */
    override fun attachBaseContext(base: Context) {
        super.attachBaseContext(LocaleHelper.wrap(base))
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        /* On a desktop -- Samsung DeX -- the launcher is a window the player
         * sizes, so the orientation lock is let go: the task's window is made
         * to its shape, and the game that follows into it inherits it
         * (NFS3Activity.desktopWindow).  On a phone it turns over with the
         * phone, in landscape. */
        requestedOrientation = launcherOrientation()
        enableEdgeToEdge()
        super.onCreate(savedInstanceState)
        hideStatusBar(this)
        val preferences = GamePreferences.get(this)
        // Removed in 0.75: fixed/editable positions supersede this global offset.
        preferences.edit().remove("touch_raise_controls").apply()
        // 0.75: the pads' buttons mean one thing in a race and another in the
        // menus, and their racing set moved with it.
        GamepadButtons.migrateDefaults(preferences)
        TouchControlsOverlay.anchorLegacyPositions(this)
        (getSystemService(INPUT_SERVICE) as InputManager).registerInputDeviceListener(gamepadListener, null)
        try {
            dataSetManager = DataSetManager(this, dataRoot())
        } catch (e: IOException) {
            Log.e(TAG, "Could not initialize data sets", e)
            Toast.makeText(this, getString(R.string.data_error, e.message), Toast.LENGTH_LONG).show()
        }
        refreshDataSets()
        showScreens()
    }

    fun launcherOrientation(): Int = orientationFor(this)

    private fun showScreens() {
        setContent { GameTheme { LauncherScreens(this) } }
    }

    fun dataRoot(): File =
        getExternalFilesDir(null) ?: throw IOException(getString(R.string.error_external_storage))

    fun refreshDataSets() {
        val manager = dataSetManager
        dataSets = manager?.list() ?: emptyList()
        activeSet = manager?.active()
    }

    /** Which build this is, under the menu: v0.75, or v0.75 DEBUG for a debug
     *  build, which installs beside a release one under a package of its own. */
    fun versionLabel(): String {
        val name = try {
            packageManager.getPackageInfo(packageName, 0).versionName ?: ""
        } catch (ignored: PackageManager.NameNotFoundException) {
            ""
        }
        val debug = applicationInfo.flags and ApplicationInfo.FLAG_DEBUGGABLE != 0
        return "v" + name + if (debug) " DEBUG" else ""
    }

    fun go(next: Screen) {
        if (next == Screen.Gamepads || next == Screen.GamepadButtons)
            capturingSlot = -1
        if (next == Screen.GameData || next == Screen.DataSets || next == Screen.Main)
            refreshDataSets()
        if (next != Screen.GameData && next != Screen.Saves && next != Screen.LauncherSettings && next != Screen.DataSets)
            status = ""
        if (next == Screen.Touch)
            // The old wheel and tilt modes are gone: the buttons are what there is.
            GamePreferences.get(this).edit()
                .putString(GamePreferences.TOUCH_MODE, GamePreferences.TOUCH_MODE_BUTTONS).apply()
        screen = next
    }

    fun openGamepadButtons(slot: Int) {
        buttonsSlot = slot
        go(Screen.GamepadButtons)
    }

    /* The editor is the overlay itself, laid out in a View: the screens step
     * aside for it and come back when it is done. */
    fun openEditor() {
        releasePreview()
        screen = Screen.Editor
        setContentView(R.layout.activity_touch_editor)
        val root = (findViewById<ViewGroup>(android.R.id.content)).getChildAt(0)
        root.setBackgroundColor(getColor(R.color.ui_background))
        val l = root.paddingLeft; val t = root.paddingTop; val r = root.paddingRight; val b = root.paddingBottom
        root.setOnApplyWindowInsetsListener { view, insets ->
            val safe = AndroidWindowCompat.safeInsets(insets)
            view.setPadding(l + safe.left, t + safe.top, r + safe.right, b + safe.bottom)
            insets
        }
        root.requestApplyInsets()
        touchPreview = TouchLayoutEditor.attach(this) { closeEditor() }
    }

    private fun closeEditor() {
        releasePreview()
        screen = Screen.Touch
        showScreens()
    }

    fun releasePreview() {
        touchPreview?.releaseAll()
        touchPreview = null
    }

    /* Only while the game is not running: the game keeps config.dat in memory
     * and writes the whole file back, which would quietly undo this. */
    fun confirmWriteControls(kind: ControlProfile.Kind) {
        val gamepads = kind == ControlProfile.Kind.GAMEPADS
        dialog = Dialog(
            getString(if (gamepads) R.string.controls_write_gamepads else R.string.controls_write_keyboard),
            getString(if (gamepads) R.string.controls_write_gamepads_message else R.string.controls_write_keyboard_message),
            getString(android.R.string.ok),
            { writeControls(kind) },
        )
    }

    private fun writeControls(kind: ControlProfile.Kind) {
        if (GameProcess.running(this)) {
            Toast.makeText(this, R.string.controls_write_game_running, Toast.LENGTH_LONG).show()
            return
        }
        try {
            val backup = ControlProfile.writeToGame(dataRoot(), kind)
            Toast.makeText(this, getString(R.string.controls_write_done, backup), Toast.LENGTH_LONG).show()
            padsChanged++
        } catch (missing: ControlProfile.NoSettingsException) {
            Toast.makeText(this, R.string.controls_write_no_config, Toast.LENGTH_LONG).show()
        } catch (e: IOException) {
            Log.e(TAG, "Writing controls failed", e)
            Toast.makeText(this, getString(R.string.controls_write_failed, e.message), Toast.LENGTH_LONG).show()
        }
    }

    /** The game's own keyboard controls keep the pads quiet in a split-screen
     *  race, where their keys would work the other player's car -- said on the
     *  Gamepads screen, where Gamepad ON is. */
    fun keyboardControlsInstalled(): Boolean = try {
        ControlProfile.installedKind(dataRoot()) == ControlProfile.Kind.KEYBOARD
    } catch (e: IOException) {
        false
    }

    fun switchDataSet(set: DataSetManager.DataSet) {
        try {
            dataSetManager?.activate(set.id)
            refreshDataSets()
        } catch (e: IOException) {
            showDataError(e)
        }
    }

    fun confirmDelete(set: DataSetManager.DataSet) {
        dialog = Dialog(
            getString(R.string.delete_data_title),
            getString(R.string.delete_data_message, set.name),
            getString(R.string.delete),
            {
                try {
                    dataSetManager?.delete(set.id)
                    refreshDataSets()
                } catch (e: IOException) {
                    showDataError(e)
                }
            },
        )
    }

    private fun showDataError(e: Exception) {
        Log.e(TAG, "Data-set operation failed", e)
        Toast.makeText(this, getString(R.string.data_error, e.message), Toast.LENGTH_LONG).show()
    }

    // ---- The system's file pickers, and what comes back from them ----

    fun launchFolderPicker() {
        if (busy) return
        val intent = Intent(Intent.ACTION_OPEN_DOCUMENT_TREE)
            .addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION or Intent.FLAG_GRANT_PERSISTABLE_URI_PERMISSION)
        startActivityForResult(intent, REQUEST_PICK_FOLDER)
    }

    fun launchZipPicker() {
        if (busy) return
        val intent = Intent(Intent.ACTION_OPEN_DOCUMENT).addCategory(Intent.CATEGORY_OPENABLE)
            .setType("application/zip").addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION)
        startActivityForResult(intent, REQUEST_PICK_ZIP)
    }

    fun launchSaveImport() {
        if (busy) return
        val intent = Intent(Intent.ACTION_OPEN_DOCUMENT).addCategory(Intent.CATEGORY_OPENABLE)
            .setType("application/zip").addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION)
        startActivityForResult(intent, REQUEST_IMPORT_SAVES)
    }

    fun launchSaveExport() {
        if (busy) return
        val intent = Intent(Intent.ACTION_CREATE_DOCUMENT).addCategory(Intent.CATEGORY_OPENABLE)
            .setType("application/zip").putExtra(Intent.EXTRA_TITLE, "nfs3-saves-" + stamp() + ".zip")
            .addFlags(Intent.FLAG_GRANT_WRITE_URI_PERMISSION)
        startActivityForResult(intent, REQUEST_EXPORT_SAVES)
    }

    fun launchLauncherImport() {
        if (busy) return
        val intent = Intent(Intent.ACTION_OPEN_DOCUMENT).addCategory(Intent.CATEGORY_OPENABLE)
            .setType("*/*").addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION)
        startActivityForResult(intent, REQUEST_IMPORT_LAUNCHER)
    }

    fun launchLauncherExport() {
        if (busy) return
        val intent = Intent(Intent.ACTION_CREATE_DOCUMENT).addCategory(Intent.CATEGORY_OPENABLE)
            .setType("application/json").putExtra(Intent.EXTRA_TITLE, "nfs3-launcher-" + stamp() + ".json")
            .addFlags(Intent.FLAG_GRANT_WRITE_URI_PERMISSION)
        startActivityForResult(intent, REQUEST_EXPORT_LAUNCHER)
    }

    private fun stamp(): String = SimpleDateFormat("yyyyMMdd-HHmm", Locale.ROOT).format(Date())

    /* Read from the stored preference: the operation reports back after the
     * picker, whatever the screen shows by then. */
    private fun includeSettings(): Boolean =
        GamePreferences.get(this).getBoolean(GamePreferences.SAVES_INCLUDE_SETTINGS, false)

    @Deprecated("Deprecated in Java")
    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        val uri = data?.data
        if (resultCode != Activity.RESULT_OK || uri == null)
            return
        when (requestCode) {
            REQUEST_PICK_FOLDER -> {
                try {
                    contentResolver.takePersistableUriPermission(uri, Intent.FLAG_GRANT_READ_URI_PERMISSION)
                } catch (e: SecurityException) {
                    Log.w(TAG, "Could not persist folder permission", e)
                }
                val name = DocumentFile.fromTreeUri(this, uri)?.name ?: getString(R.string.data_set_name_folder)
                startImport(name) { temporary ->
                    DataImporter.importFromTree(applicationContext, uri, temporary, progressListener())
                }
            }
            REQUEST_PICK_ZIP -> startImport(displayName(uri)) { temporary ->
                DataImporter.importFromZip(applicationContext, uri, temporary, progressListener())
            }
            REQUEST_IMPORT_SAVES -> {
                val withSettings = includeSettings()
                startOperation {
                    val backup = SaveGameManager.importZip(applicationContext, uri, dataRoot(), withSettings)
                    if (backup.isEmpty()) getString(R.string.saves_import_complete_no_backup)
                    else getString(R.string.saves_import_complete, backup)
                }
            }
            REQUEST_EXPORT_SAVES -> {
                val withSettings = includeSettings()
                startOperation {
                    getString(R.string.saves_export_complete,
                        SaveGameManager.exportZip(applicationContext, uri, dataRoot(), withSettings))
                }
            }
            REQUEST_EXPORT_LAUNCHER -> startOperation {
                getString(R.string.launcher_export_complete, LauncherSettings.export(applicationContext, uri))
            }
            REQUEST_IMPORT_LAUNCHER -> startOperation {
                /* No recreate() afterwards: every screen reads these settings
                 * when it is shown, and the language is deliberately not
                 * carried, so nothing already on screen can go stale. */
                val backup = LauncherSettings.importFrom(applicationContext, uri, dataRoot())
                // A file from before may carry control positions the old way.
                TouchControlsOverlay.anchorLegacyPositions(this)
                if (backup.isEmpty()) getString(R.string.launcher_import_complete_no_backup)
                else getString(R.string.launcher_import_complete, backup)
            }
        }
    }

    private fun displayName(uri: Uri): String {
        try {
            contentResolver.query(uri, arrayOf(OpenableColumns.DISPLAY_NAME), null, null, null)?.use { cursor ->
                if (cursor.moveToFirst())
                    return cursor.getString(0)
            }
        } catch (e: Exception) {
            Log.w(TAG, "Could not read document display name", e)
        }
        return getString(R.string.data_set_name_zip)
    }

    private fun begin() {
        busy = true
        importPercent = -1
        status = getString(R.string.checking_game_data)
    }

    /** How far along an import is: the bar fills to the percentage and the
     *  text names the file; while the folder is still being measured the bar
     *  runs on its own and the text says so. */
    private fun progressListener(): DataImporter.ProgressListener {
        val running = Thread.currentThread()
        return DataImporter.ProgressListener { done, total, currentPath ->
            runOnUiThread {
                if (worker !== running || isDestroyed) return@runOnUiThread
                val percent = DataImporter.ProgressListener.percent(done, total)
                importPercent = percent
                status = importStatus(this, percent, currentPath)
            }
        }
    }

    private fun startOperation(operation: () -> String) {
        if (busy) return
        begin()
        val thread = Thread({
            val running = Thread.currentThread()
            try {
                val message = operation()
                runOnUiThread {
                    if (worker !== running || isDestroyed) return@runOnUiThread
                    worker = null
                    busy = false
                    status = ""
                    Toast.makeText(this, message, Toast.LENGTH_LONG).show()
                }
            } catch (failure: IOException) {
                Log.e(TAG, "Save operation failed", failure)
                runOnUiThread {
                    if (worker !== running || isDestroyed) return@runOnUiThread
                    worker = null
                    busy = false
                    status = getString(R.string.data_error, failure.message)
                    Toast.makeText(this, status, Toast.LENGTH_LONG).show()
                }
            }
        }, "nfs3-save-operation")
        worker = thread
        thread.start()
    }

    private fun startImport(displayName: String, operation: (File) -> Unit) {
        val manager = dataSetManager ?: return
        if (busy) return
        begin()
        val thread = Thread({
            val running = Thread.currentThread()
            try {
                val missing = ArrayList<String>()
                manager.importDataSet(displayName) { temporary ->
                    operation(temporary)
                    missing.addAll(missingGameFiles(applicationContext, temporary))
                }
                runOnUiThread {
                    if (worker !== running || isDestroyed) return@runOnUiThread
                    worker = null
                    busy = false
                    status = ""
                    refreshDataSets()
                    Toast.makeText(this, R.string.import_complete, Toast.LENGTH_SHORT).show()
                    if (missing.isNotEmpty())
                        dialog = missingFilesDialog(this, missing, null)
                }
            } catch (e: IOException) {
                if (running.isInterrupted) return@Thread
                Log.e(TAG, "Data import failed", e)
                runOnUiThread {
                    if (worker !== running || isDestroyed) return@runOnUiThread
                    worker = null
                    busy = false
                    status = getString(R.string.import_failed, e.message)
                    Toast.makeText(this, status, Toast.LENGTH_LONG).show()
                }
            }
        }, "nfs3-data-import")
        worker = thread
        thread.start()
    }

    fun startGame() {
        if (dataSetManager?.active() == null)
            return
        /* Game data imported before the cop's map came as a minimap gets it
         * now, once -- while the game is not running to save over it. */
        if (!GameProcess.running(this)) {
            try {
                ControlProfile.copMinimapOnce(dataRoot())
            } catch (e: IOException) {
                Log.w(TAG, "No data root for the cop's minimap", e)
            }
        }
        val intent = Intent(this, NFS3Activity::class.java)
        getIntent()?.extras?.let { intent.putExtras(it) }
        startActivity(intent)
    }

    /* While a Gamepads slot waits for its pad, the next button pressed on any
     * gamepad answers it.  Only gamepad presses count, and only then; the rest
     * of the time the launcher stays navigable with a pad as usual. */
    override fun dispatchKeyEvent(event: KeyEvent): Boolean {
        if (event.action == KeyEvent.ACTION_UP && event.keyCode == swallowKeyUp) {
            swallowKeyUp = KeyEvent.KEYCODE_UNKNOWN
            return true
        }
        if (capturingSlot >= 0 && event.action == KeyEvent.ACTION_DOWN) {
            val device = event.device
            if (GamepadSlots.isGamepad(device)) {
                GamepadSlots.assign(GamePreferences.get(this), capturingSlot, device.descriptor, device.name)
                capturingSlot = -1
                swallowKeyUp = event.keyCode
                padsChanged++
                return true
            }
        }
        return super.dispatchKeyEvent(event)
    }

    @Deprecated("Deprecated in Java")
    override fun onBackPressed() {
        if (busy) return
        if (dialog != null) {
            dialog = null
            return
        }
        // Waiting for a pad is cancelled, not the screen it waits on.
        if (capturingSlot >= 0) {
            capturingSlot = -1
            return
        }
        when (screen) {
            Screen.Main -> {
                @Suppress("DEPRECATION")
                super.onBackPressed()
            }
            Screen.Editor -> closeEditor()
            Screen.TouchKeys -> go(Screen.Touch)
            Screen.Touch, Screen.Gamepads -> go(Screen.Controls)
            // Split screen help and a pad's buttons both open from the Gamepads screen.
            Screen.GamepadButtons, Screen.ControlsHelp -> go(Screen.Gamepads)
            // The Data screens are a level deeper than the rest.
            Screen.GameData, Screen.Saves, Screen.LauncherSettings -> go(Screen.Data)
            Screen.DataSets -> go(Screen.GameData)
            // Screen adjustment sits under Display.
            Screen.Adjustment -> go(Screen.Display)
            else -> go(Screen.Main)
        }
    }

    override fun onDestroy() {
        (getSystemService(INPUT_SERVICE) as InputManager).unregisterInputDeviceListener(gamepadListener)
        val running = worker
        worker = null
        running?.interrupt()
        releasePreview()
        super.onDestroy()
    }

    companion object {
        private const val TAG = "NFS3Launcher"
        private const val REQUEST_PICK_FOLDER = 10
        private const val REQUEST_PICK_ZIP = 11
        private const val REQUEST_IMPORT_SAVES = 12
        private const val REQUEST_EXPORT_SAVES = 13
        private const val REQUEST_IMPORT_LAUNCHER = 14
        private const val REQUEST_EXPORT_LAUNCHER = 15

        /** Full screen, as the theme asks, said to the window as well: with the
         *  status bar gone from the start, nothing arrives later to move the
         *  screen that was already drawn. */
        fun hideStatusBar(activity: Activity) {
            val controller = androidx.core.view.WindowCompat.getInsetsController(activity.window, activity.window.decorView)
            controller.hide(androidx.core.view.WindowInsetsCompat.Type.statusBars())
        }

        /** Landscape either way up, turning with the phone as any app does --
         *  the game has an orientation setting of its own, the launcher does
         *  not need one; portrait never, the game cannot use it.  A desktop
         *  window (DeX) takes whatever shape the player gives it. */
        fun orientationFor(context: Context): Int =
            if (DesktopMode.active(context)) ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED
            else ActivityInfo.SCREEN_ORIENTATION_SENSOR_LANDSCAPE

        /** One step of an import, the same on the first run's screen and on
         *  Game data: the file it is on and how far, or that the folder is
         *  still being measured. */
        fun importStatus(context: Context, percent: Int, path: String): String {
            if (percent < 0)
                return context.getString(R.string.checking_game_data)
            val name = path.substring(path.lastIndexOf(File.separatorChar) + 1)
            return context.getString(R.string.importing_percent, name, percent)
        }

        /* What an import copied without: the game would only find out half way
         * into loading a race (DataImporter.missingFiles).  A failed check is
         * no reason to fail the import, so it only goes to the log. */
        fun missingGameFiles(context: Context, root: File): List<String> = try {
            DataImporter.missingFiles(context, root)
        } catch (e: IOException) {
            Log.w(TAG, "Could not check the imported game data", e)
            emptyList()
        }

        /** Which of the game's files an import did not bring; `then` runs when
         *  the message is closed. */
        fun missingFilesDialog(context: Context, missing: List<String>, then: (() -> Unit)?): Dialog =
            Dialog(
                context.getString(R.string.data_missing_title),
                DataImporter.describeMissing(context, missing),
                context.getString(android.R.string.ok),
                null,
                then,
            )
    }
}
