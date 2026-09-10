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
    private GameHaptics haptics;

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
        haptics=new GameHaptics(this);
        SharedPreferences preferences = GamePreferences.get(this);
        setEnv("NFS_TOUCH_VIBRATION",preferences.getBoolean(GamePreferences.TOUCH_VIBRATION,false)?"1":"0");
        setEnv("NFS_GAMEPAD_VIBRATION",preferences.getBoolean(GamePreferences.GAMEPAD_VIBRATION,false)?"1":"0");
        /* Whether the phone can vibrate at all is a hardware question, and the
         * native side has no way to ask it.  Without this, a phone with no motor
         * still reports force feedback to the game, which then creates effects
         * that go nowhere. */
        android.os.Vibrator motor=(android.os.Vibrator)getSystemService(Context.VIBRATOR_SERVICE);
        setEnv("NFS_HAS_VIBRATOR",motor!=null&&motor.hasVibrator()?"1":"0");
        for(String action:new String[]{"steer_left","steer_right","accelerate","brake"})
            setEnv("NFS_TOUCH_"+action.toUpperCase(java.util.Locale.ROOT),
                GamePreferences.keyName(GamePreferences.getTouchKey(preferences,action)));
        String orientation = preferences.getString(GamePreferences.ORIENTATION,
            GamePreferences.ORIENTATION_LANDSCAPE);
        int fpsCap = preferences.getInt(GamePreferences.FPS_CAP, 30);
        setEnv("NFS_ORIENTATION", orientation);
        setEnv("NFS_FPS_CAP", Integer.toString(fpsCap));
        /* Percent in the settings, a multiplier in the blit shader.  Formatted
         * with the root locale on purpose: a comma decimal separator would not
         * survive SDL_atof on the other side. */
        setEnv("NFS_GAMMA", String.format(java.util.Locale.ROOT, "%.2f",
            preferences.getInt(GamePreferences.GAMMA, 100) / 100f));
        setEnv("NFS_GAMEPAD_MAPPING",
            GamePreferences.gamepadMappingEnvironment(preferences));

        /* Landscape whatever happens -- portrait is not offered, the game
         * cannot use it.  "auto" turns over with the phone; the two fixed
         * choices pin one direction each, and which of them is the right way up
         * depends only on how the player holds the device.
         *
         * This has to agree with the SDL_HINT_ORIENTATIONS set natively in
         * nfs3hp_main.cpp: SDL applies the hint through setOrientationBis()
         * afterwards, so a disagreement means the hint wins and the choice
         * here is silently undone. */
        setRequestedOrientation(
            GamePreferences.ORIENTATION_AUTO.equals(orientation)
                ? ActivityInfo.SCREEN_ORIENTATION_SENSOR_LANDSCAPE
            : GamePreferences.ORIENTATION_LANDSCAPE_REVERSE.equals(orientation)
                ? ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE
                : ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);

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
        if(haptics!=null)haptics.resume();
        attachTouchControls();
    }

    @Override
    protected void onPause()
    {
        if (touchControlsOverlay != null)
            touchControlsOverlay.releaseAll();
        if (gameSurface != null) gameSurface.touchMouse.cancel();
        if(haptics!=null)haptics.pause();
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
        if(haptics!=null&&event.getActionMasked()==android.view.MotionEvent.ACTION_DOWN)haptics.usePhone();
        if(touchControlsOverlay!=null&&event.getToolType(0)==android.view.MotionEvent.TOOL_TYPE_FINGER)
            touchControlsOverlay.onScreenTouch(event);
        return super.dispatchTouchEvent(event);
    }

    @Override public boolean dispatchKeyEvent(android.view.KeyEvent event) {
        if(haptics!=null&&event.getAction()==android.view.KeyEvent.ACTION_DOWN)haptics.useController(event.getDevice());
        return super.dispatchKeyEvent(event);
    }
    @Override public boolean dispatchGenericMotionEvent(android.view.MotionEvent event) {
        if(haptics!=null&&event.isFromSource(android.view.InputDevice.SOURCE_JOYSTICK)
            &&(Math.abs(event.getAxisValue(android.view.MotionEvent.AXIS_X))>.25f
                ||Math.abs(event.getAxisValue(android.view.MotionEvent.AXIS_Y))>.25f
                ||event.getAxisValue(android.view.MotionEvent.AXIS_RTRIGGER)>.25f
                ||event.getAxisValue(android.view.MotionEvent.AXIS_LTRIGGER)>.25f))haptics.useController(event.getDevice());
        return super.dispatchGenericMotionEvent(event);
    }
    @Override public void onWindowFocusChanged(boolean focus) {
        super.onWindowFocusChanged(focus);
        if(haptics!=null){if(focus)haptics.resume();else haptics.pause();}
    }
    /** JNI entry point, invoked only for effects issued by the game. */
    public void onForceFeedback(float level) {
        long sent=android.os.SystemClock.uptimeMillis();
        runOnUiThread(()->{if(haptics!=null&&android.os.SystemClock.uptimeMillis()-sent<100)haptics.setLevel(level);});
    }

    /** Invoked by the menu overlay. SDL's hidden edit view forwards committed
     * text as SDL_TEXT_INPUT, which the Win32 bridge exposes as WM_CHAR. */
    /* The same button opens and closes, and both ways go through SDL rather
     * than SDLActivity.showTextInput(): raising the Android keyboard on its own
     * leaves SDL's text input inactive, and SDL then drops every character while
     * still passing backspace through as a key event.  Which way to go is asked
     * of SDL too, not remembered here -- the system back key closes the keyboard
     * as well, and a flag kept privately in Java would go stale exactly then,
     * leaving a button that does nothing. */
    void showTouchKeyboard() {
        nativeToggleKeyboard();
    }

    private static native void nativeToggleKeyboard();

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
            touchMouse = new TouchMouseInput(this, SDLActivity::onNativeMouse);
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
