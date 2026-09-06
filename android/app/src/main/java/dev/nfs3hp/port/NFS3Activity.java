package dev.nfs3hp.port;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.pm.ActivityInfo;
import android.os.Bundle;
import android.system.Os;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.ViewGroup;
import android.widget.RelativeLayout;

import org.libsdl.app.SDLActivity;
import org.libsdl.app.SDLSurface;

/**
 * The actual game.  A plain SDLActivity subclass -- the defaults
 * (getMainSharedObject/getLibraries -> {"SDL3","main"}, getMainFunction ->
 * "SDL_main") already match what the CMake build produces (see the ANDROID
 * branch of add_game_exe() in the root CMakeLists.txt), so nothing needs
 * overriding here.
 *
 * Only ever started by LauncherActivity, once DataImporter confirms the game
 * data and the bundled original binaries are both present under
 * getExternalFilesDir(null) -- see src/nfs3hp/nfs3hp_main.cpp, which reads
 * SDL_GetAndroidExternalStoragePath() for both the data and "CD" directory.
 *
 * Diagnostics: Android does not pass adb-shell environment variables down to
 * an app process, but Java and the native code it loads share one process
 * address space -- Os.setenv() here lands in the same environment table
 * SDL_getenv()/getenv() reads from native code, as long as it runs before the
 * native thread starts (SDLActivity only spins that up later, from
 * surfaceCreated()). This is how NFS_TRACE_API (winapp.h) and NFS_TRACE_MSG's
 * runtime cousins get toggled on device without a wrap.sh or a rebuild:
 *   adb shell am start -n dev.nfs3hp.port/.SplashActivity --ez trace_api true
 */
public class NFS3Activity extends SDLActivity
{
    private static final String TAG = "NFS3Activity";
    private TouchControlsOverlay touchControlsOverlay;
    private GameSurface gameSurface;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        SharedPreferences preferences = GamePreferences.get(this);
        String orientation = preferences.getString(GamePreferences.ORIENTATION,
            GamePreferences.ORIENTATION_LANDSCAPE);
        int fpsCap = preferences.getInt(GamePreferences.FPS_CAP, 30);
        setEnv("NFS_ORIENTATION", orientation);
        setEnv("NFS_FPS_CAP", Integer.toString(fpsCap));
        setEnv("NFS_GAMEPAD_MAPPING",
            GamePreferences.gamepadMappingEnvironment(preferences));

        setRequestedOrientation(GamePreferences.ORIENTATION_AUTO.equals(orientation)
            ? ActivityInfo.SCREEN_ORIENTATION_FULL_USER
            : ActivityInfo.SCREEN_ORIENTATION_SENSOR_LANDSCAPE);

        if (getIntent() != null && getIntent().getBooleanExtra("trace_api", false))
        {
            // Bundled together: NFS_TRACE_API covers every win32 call and
            // File::read's own byte counts, SDL_LOGGING=app,error surfaces
            // File::File's "opening file %s" (SDL_LogDebug, silent by
            // default) so a given File::read burst can be matched back to
            // which path it belongs to.
            setEnv("NFS_TRACE_API", "1");
            setEnv("SDL_LOGGING", "app=verbose,error=verbose");
        }
        super.onCreate(savedInstanceState);
        attachTouchControls();
    }

    @Override
    protected SDLSurface createSDLSurface(Context context)
    {
        gameSurface = new GameSurface(context);
        return gameSurface;
    }

    @Override
    protected void onResume()
    {
        super.onResume();
        attachTouchControls();
    }

    @Override
    protected void onPause()
    {
        if (touchControlsOverlay != null)
            touchControlsOverlay.releaseAll();
        if (gameSurface != null) gameSurface.touchMouse.cancel();
        super.onPause();
    }

    private void attachTouchControls()
    {
        // Legacy wheel/tilt choices were placeholders; always provide usable controls.
        if (mLayout == null)
            return;

        if (touchControlsOverlay == null)
            touchControlsOverlay = new TouchControlsOverlay(this);

        if (touchControlsOverlay.getParent() != mLayout)
        {
            if (touchControlsOverlay.getParent() instanceof ViewGroup)
                ((ViewGroup) touchControlsOverlay.getParent()).removeView(touchControlsOverlay);
            RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT);
            mLayout.addView(touchControlsOverlay, params);
        }
        touchControlsOverlay.bringToFront();
        touchControlsOverlay.requestApplyInsets();
        touchControlsOverlay.resumeIdleTimer();
    }

    @Override public boolean dispatchTouchEvent(android.view.MotionEvent event) {
        if(touchControlsOverlay!=null&&event.getToolType(0)==android.view.MotionEvent.TOOL_TYPE_FINGER)
            touchControlsOverlay.onScreenTouch(event);
        return super.dispatchTouchEvent(event);
    }

    private void setEnv(String name, String value)
    {
        try
        {
            Os.setenv(name, value, true);
            Log.i(TAG, "set " + name + "=" + value);
        }
        catch (Exception e)
        {
            Log.w(TAG, "Os.setenv(" + name + ") failed", e);
        }
    }

    private final class GameSurface extends SDLSurface
    {
        final TouchMouseInput touchMouse;
        GameSurface(Context context)
        {
            super(context);
            touchMouse = new TouchMouseInput(context, SDLActivity::onNativeMouse);
        }

        @Override public boolean onTouch(android.view.View v, android.view.MotionEvent event) {
            int tool=event.getToolType(0);
            if(tool==android.view.MotionEvent.TOOL_TYPE_FINGER||tool==android.view.MotionEvent.TOOL_TYPE_UNKNOWN)
                return touchMouse.onTouch(event);
            return super.onTouch(v,event);
        }

        @Override public void onWindowFocusChanged(boolean focus) {
            super.onWindowFocusChanged(focus);if(!focus)touchMouse.cancel();
        }

        @Override
        public void surfaceCreated(SurfaceHolder holder)
        {
            super.surfaceCreated(holder);
            post(NFS3Activity.this::attachTouchControls);
        }

        @Override
        public void surfaceChanged(SurfaceHolder holder, int format, int width, int height)
        {
            super.surfaceChanged(holder, format, width, height);
            post(NFS3Activity.this::attachTouchControls);
        }
    }
}
