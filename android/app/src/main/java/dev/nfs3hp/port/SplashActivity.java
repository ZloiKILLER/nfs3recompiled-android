package dev.nfs3hp.port;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;
import java.io.IOException;

/**
 * Launcher entry point.  Makes sure the game's data is where
 * SDL_GetAndroidExternalStoragePath() will point nfs3hp_main.cpp at, then
 * hands off to LauncherActivity -- see DataImporter for exactly what "the data"
 * means and why.
 */
public class SplashActivity extends Activity
{
    private static final String TAG = "NFS3Splash";
    private static final int REQUEST_PICK_FOLDER = 1;

    private TextView statusText;
    private ProgressBar progressBar;
    private Button pickFolderButton;
    private boolean importRunning = false;
    private boolean folderPickerRunning = false;
    private Thread importThread;

    /* Language of the launcher, chosen in the picker on the main screen.
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
        setContentView(R.layout.activity_splash);
        statusText = findViewById(R.id.status_text);
        progressBar = findViewById(R.id.progress_bar);
        pickFolderButton = findViewById(R.id.pick_folder_button);
        pickFolderButton.setOnClickListener(v -> launchFolderPicker());

        checkDataAndProceed();
    }

    private File dataRoot() throws IOException
    {
        File dir = getExternalFilesDir(null);
        if (dir == null)
        {
            // Only happens if external storage is unavailable, e.g. removed mid-run.
            throw new IOException(getString(R.string.error_external_storage));
        }
        return dir;
    }

    private void checkDataAndProceed()
    {
        try
        {
            File root = dataRoot();
            DataImporter.copyBundledAssets(this, root);

            if (DataImporter.isUserDataPresent(root))
            {
                startLauncher();
                return;
            }
        }
        catch (IOException e)
        {
            Log.e(TAG, "Failed to copy bundled game files", e);
            statusText.setText(getString(R.string.import_failed, e.getMessage()));
            return;
        }

        statusText.setText(R.string.import_hint);
        pickFolderButton.setVisibility(View.VISIBLE);
    }

    private void launchFolderPicker()
    {
        if (folderPickerRunning || importRunning)
            return;
        folderPickerRunning = true;
        pickFolderButton.setEnabled(false);

        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
        intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION
            | Intent.FLAG_GRANT_PERSISTABLE_URI_PERMISSION);
        startActivityForResult(intent, REQUEST_PICK_FOLDER);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data)
    {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode != REQUEST_PICK_FOLDER)
            return;

        folderPickerRunning = false;
        pickFolderButton.setEnabled(true);
        if (resultCode != Activity.RESULT_OK || data == null)
            return;

        Uri treeUri = data.getData();
        if (treeUri == null)
            return;

        try
        {
            getContentResolver().takePersistableUriPermission(treeUri,
                Intent.FLAG_GRANT_READ_URI_PERMISSION);
        }
        catch (SecurityException e)
        {
            // Not fatal -- the permission still holds for this session, it just
            // will not survive a process restart if the import is interrupted.
            Log.w(TAG, "Could not persist folder permission", e);
        }

        startImport(treeUri);
    }

    private void startImport(Uri treeUri)
    {
        if (importRunning)
            return;

        final File root;
        try
        {
            root = dataRoot();
        }
        catch (IOException e)
        {
            showImportFailure(e);
            return;
        }

        importRunning = true;

        pickFolderButton.setVisibility(View.GONE);
        progressBar.setVisibility(View.VISIBLE);
        statusText.setText(R.string.checking_game_data);

        Thread worker = new Thread(() -> {
            Thread runningThread = Thread.currentThread();
            try
            {
                DataImporter.importFromTree(getApplicationContext(), treeUri, root,
                    (bytesCopied, filesCopied, currentPath) ->
                    runOnUiThread(() -> {
                        if (importThread != runningThread || isDestroyed())
                            return;
                        int mb = (int) (bytesCopied / (1024 * 1024));
                        String name = currentPath.substring(currentPath.lastIndexOf('/') + 1);
                        statusText.setText(getResources().getQuantityString(
                            R.plurals.importing_progress, filesCopied, name, mb, filesCopied));
                    }));

                runOnUiThread(() -> {
                    if (importThread != runningThread || isDestroyed())
                        return;
                    importThread = null;
                    importRunning = false;
                    if (DataImporter.isUserDataPresent(root))
                    {
                        startLauncher();
                    }
                    else
                    {
                        // The picked folder had fedata/gamedata subfolders (importFromTree
                        // would have thrown otherwise) but the copy still doesn't match the
                        // expected layout -- surface that instead of silently retrying.
                        progressBar.setVisibility(View.GONE);
                        pickFolderButton.setVisibility(View.VISIBLE);
                        statusText.setText(getString(R.string.import_failed,
                            getString(R.string.error_import_incomplete)));
                    }
                });
            }
            catch (IOException e)
            {
                if (runningThread.isInterrupted())
                    return;
                Log.e(TAG, "Import failed", e);
                runOnUiThread(() -> {
                    if (importThread != runningThread || isDestroyed())
                        return;
                    importThread = null;
                    importRunning = false;
                    showImportFailure(e);
                });
            }
        }, "nfs3-import");
        importThread = worker;
        worker.start();
    }

    private void showImportFailure(Exception e)
    {
        progressBar.setVisibility(View.GONE);
        pickFolderButton.setVisibility(View.VISIBLE);
        String message = getString(R.string.import_failed, e.getMessage());
        statusText.setText(message);
        Toast.makeText(this, message, Toast.LENGTH_LONG).show();
    }

    @Override
    protected void onDestroy()
    {
        Thread worker = importThread;
        importThread = null;
        importRunning = false;
        if (worker != null)
            worker.interrupt();
        super.onDestroy();
    }

    private void startLauncher()
    {
        Intent intent = new Intent(this, LauncherActivity.class);
        // Preserve diagnostic extras so LauncherActivity can forward them to
        // the non-exported NFS3Activity when Play is tapped.
        if (getIntent() != null && getIntent().getExtras() != null)
            intent.putExtras(getIntent().getExtras());
        startActivity(intent);
        finish();
    }
}
