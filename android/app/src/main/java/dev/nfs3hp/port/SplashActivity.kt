package dev.nfs3hp.port

import android.app.Activity
import android.content.Context
import android.content.Intent
import android.net.Uri
import android.os.Bundle
import android.util.Log
import android.widget.Toast
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.widthIn
import androidx.compose.foundation.layout.windowInsetsPadding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.LinearProgressIndicator
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import java.io.File
import java.io.IOException

/**
 * Launcher entry point.  Makes sure the game's data is where
 * SDL_GetAndroidExternalStoragePath() will point nfs3hp_main.cpp at, then
 * hands off to LauncherActivity -- see DataImporter for exactly what "the data"
 * means and why.
 */
class SplashActivity : ComponentActivity() {

    private var status by mutableStateOf("")
    private var canPick by mutableStateOf(false)
    private var importing by mutableStateOf(false)
    private var importPercent by mutableIntStateOf(-1)
    private var missing by mutableStateOf<List<String>?>(null)
    private var pickerOpen = false
    private var worker: Thread? = null

    /* Language of the launcher, chosen in the picker on the main screen.
     * LocaleHelper returns the context unchanged while the setting is
     * "system". */
    override fun attachBaseContext(base: Context) {
        super.attachBaseContext(LocaleHelper.wrap(base))
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        /* The first screen of all, so the shape of the task's window on a
         * desktop is decided here: DeX gets a window the player sizes rather
         * than a phone's landscape (see DesktopMode). */
        requestedOrientation = LauncherActivity.orientationFor(this)
        enableEdgeToEdge()
        super.onCreate(savedInstanceState)
        LauncherActivity.hideStatusBar(this)
        status = getString(R.string.checking_game_data)
        setContent { GameTheme { SplashScreen() } }
        checkDataAndProceed()
    }

    @androidx.compose.runtime.Composable
    private fun SplashScreen() {
        Column(
            Modifier
                .fillMaxSize()
                .windowInsetsPadding(LauncherInsets)
                .padding(32.dp)
                .verticalScroll(rememberScrollState()),
            verticalArrangement = Arrangement.spacedBy(16.dp, Alignment.CenterVertically),
            horizontalAlignment = Alignment.CenterHorizontally,
        ) {
            Text(
                getString(R.string.app_name).uppercase(),
                color = GameColors.Gold,
                fontWeight = FontWeight.Black,
                style = androidx.compose.material3.MaterialTheme.typography.headlineMedium,
            )
            Text(status, color = GameColors.Silver, textAlign = TextAlign.Center, modifier = Modifier.widthIn(max = 560.dp))
            if (importing) {
                val bar = Modifier.widthIn(max = 560.dp).fillMaxWidth()
                if (importPercent < 0)
                    LinearProgressIndicator(bar, color = GameColors.Gold, trackColor = GameColors.Pill)
                else
                    LinearProgressIndicator({ importPercent / 100f }, bar, color = GameColors.Gold, trackColor = GameColors.Pill)
            }
            if (canPick)
                GoldButton(getString(R.string.pick_data_folder), { launchFolderPicker() }, Modifier.widthIn(min = 320.dp))
        }
        missing?.let { files ->
            AlertDialog(
                onDismissRequest = { closeMissing() },
                confirmButton = {
                    TextButton({ closeMissing() }) { Text(getString(android.R.string.ok), color = GameColors.Gold) }
                },
                title = { Text(getString(R.string.data_missing_title), color = GameColors.Gold, fontWeight = FontWeight.Bold) },
                text = {
                    Text(DataImporter.describeMissing(this, files), color = GameColors.Silver,
                        modifier = Modifier.verticalScroll(rememberScrollState()))
                },
            )
        }
    }

    private fun closeMissing() {
        missing = null
        startLauncher()
    }

    private fun dataRoot(): File =
        // Only null if external storage is unavailable, e.g. removed mid-run.
        getExternalFilesDir(null) ?: throw IOException(getString(R.string.error_external_storage))

    private fun checkDataAndProceed() {
        try {
            val root = dataRoot()
            DataImporter.copyBundledAssets(this, root)
            if (DataImporter.isUserDataPresent(root)) {
                startLauncher()
                return
            }
        } catch (e: IOException) {
            Log.e(TAG, "Failed to copy bundled game files", e)
            status = getString(R.string.import_failed, e.message)
            return
        }
        status = getString(R.string.import_hint)
        canPick = true
    }

    private fun launchFolderPicker() {
        if (pickerOpen || importing) return
        pickerOpen = true
        val intent = Intent(Intent.ACTION_OPEN_DOCUMENT_TREE)
            .addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION or Intent.FLAG_GRANT_PERSISTABLE_URI_PERMISSION)
        startActivityForResult(intent, REQUEST_PICK_FOLDER)
    }

    @Deprecated("Deprecated in Java")
    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        if (requestCode != REQUEST_PICK_FOLDER) return
        pickerOpen = false
        val treeUri = data?.data
        if (resultCode != Activity.RESULT_OK || treeUri == null) return
        try {
            contentResolver.takePersistableUriPermission(treeUri, Intent.FLAG_GRANT_READ_URI_PERMISSION)
        } catch (e: SecurityException) {
            // Not fatal -- the permission still holds for this session, it just
            // will not survive a process restart if the import is interrupted.
            Log.w(TAG, "Could not persist folder permission", e)
        }
        startImport(treeUri)
    }

    private fun startImport(treeUri: Uri) {
        if (importing) return
        val root = try {
            dataRoot()
        } catch (e: IOException) {
            showImportFailure(e)
            return
        }
        importing = true
        canPick = false
        importPercent = -1
        status = getString(R.string.checking_game_data)
        val thread = Thread({
            val running = Thread.currentThread()
            try {
                DataImporter.importFromTree(applicationContext, treeUri, root) { done, total, currentPath ->
                    runOnUiThread {
                        if (worker !== running || isDestroyed) return@runOnUiThread
                        importPercent = DataImporter.ProgressListener.percent(done, total)
                        status = LauncherActivity.importStatus(this, importPercent, currentPath)
                    }
                }
                val lacking = LauncherActivity.missingGameFiles(applicationContext, root)
                runOnUiThread {
                    if (worker !== running || isDestroyed) return@runOnUiThread
                    worker = null
                    importing = false
                    if (DataImporter.isUserDataPresent(root)) {
                        // What the copy is short of, before the launcher takes over.
                        if (lacking.isEmpty()) startLauncher() else missing = lacking
                    } else {
                        // The picked folder had fedata/gamedata subfolders (importFromTree
                        // would have thrown otherwise) but the copy still doesn't match the
                        // expected layout -- surface that instead of silently retrying.
                        canPick = true
                        status = getString(R.string.import_failed, getString(R.string.error_import_incomplete))
                    }
                }
            } catch (e: IOException) {
                if (running.isInterrupted) return@Thread
                Log.e(TAG, "Import failed", e)
                runOnUiThread {
                    if (worker !== running || isDestroyed) return@runOnUiThread
                    worker = null
                    importing = false
                    showImportFailure(e)
                }
            }
        }, "nfs3-import")
        worker = thread
        thread.start()
    }

    private fun showImportFailure(e: Exception) {
        canPick = true
        status = getString(R.string.import_failed, e.message)
        Toast.makeText(this, status, Toast.LENGTH_LONG).show()
    }

    override fun onDestroy() {
        val running = worker
        worker = null
        importing = false
        running?.interrupt()
        super.onDestroy()
    }

    private fun startLauncher() {
        val intent = Intent(this, LauncherActivity::class.java)
        // Preserve diagnostic extras so LauncherActivity can forward them to
        // the non-exported NFS3Activity when Play is tapped.
        getIntent()?.extras?.let { intent.putExtras(it) }
        startActivity(intent)
        finish()
    }

    companion object {
        private const val TAG = "NFS3Splash"
        private const val REQUEST_PICK_FOLDER = 1
    }
}
