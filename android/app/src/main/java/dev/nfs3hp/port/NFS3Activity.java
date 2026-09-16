package dev.nfs3hp.port;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.pm.ActivityInfo;
import android.hardware.input.InputManager;
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
    /* Off, the screen does nothing in the game: no on-screen controls and no
     * touch mouse (GamePreferences.TOUCH_ENABLED).  Read once in onCreate --
     * only the launcher changes it. */
    private boolean touchEnabled = true;

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
        touchEnabled = preferences.getBoolean(GamePreferences.TOUCH_ENABLED, true);
        /* The four on-screen controls that steer and work the pedals.  The
         * control profile puts those on the first slot's axes rather than on
         * keys, so the native side recognises what the overlay is doing by the
         * key each of the four sends and folds it into the axes.  The overlay's
         * other controls are keys the profile binds as they are. */
        for(int i=0;i<4;i++) {
            String action=GamePreferences.ACTION_IDS[i];
            setEnv("NFS_TOUCH_"+action.toUpperCase(java.util.Locale.ROOT),
                GamePreferences.keyName(GamePreferences.getTouchKey(preferences,action)));
        }
        /* What every pad button sends, for both slots: the right player's key,
         * an axis, or nothing.  Resolved here, so the native side only ever
         * sees final values. */
        for(int slot=0;slot<GamepadSlots.COUNT;slot++)
            for(int button=0;button<GamepadButtons.BUTTON_IDS.length;button++)
                setEnv(GamepadButtons.environmentName(slot,GamepadButtons.BUTTON_IDS[button]),
                    GamepadButtons.environmentValue(slot,GamepadButtons.action(preferences,slot,button)));
        String orientation = preferences.getString(GamePreferences.ORIENTATION,
            GamePreferences.ORIENTATION_LANDSCAPE);
        int fpsCap = preferences.getInt(GamePreferences.FPS_CAP, 30);
        setEnv("NFS_ORIENTATION", orientation);
        setEnv("NFS_FPS_CAP", Integer.toString(fpsCap));
        /* Percent in the settings, a multiplier in the blit shader.  Formatted
         * with the root locale on purpose: a comma decimal separator would not
         * survive SDL_atof on the other side. */
        setEnv("NFS_GAMMA", String.format(java.util.Locale.ROOT, "%.3f",
            ScreenAdjustment.gamma(ScreenAdjustment.gammaPercent(preferences))));
        setEnv("NFS_BRIGHTNESS", String.format(java.util.Locale.ROOT, "%.3f",
            ScreenAdjustment.brightness(ScreenAdjustment.brightnessPercent(preferences))));
        setEnv("NFS_CONTRAST", String.format(java.util.Locale.ROOT, "%.3f",
            ScreenAdjustment.contrast(ScreenAdjustment.contrastPercent(preferences))));
        /* Debug builds log the car detail the game settles on, once a second
         * during a race ([CARDETAIL] in logcat): the level each car was drawn
         * at against the game's budget, the transform buffer and the atlas. */
        if ((getApplicationInfo().flags & android.content.pm.ApplicationInfo.FLAG_DEBUGGABLE) != 0)
            setEnv("NFS_CAR_DETAIL_TRACE", "1");

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
        running = true;
        attachTouchControls();
        pushGamepadSlots();
        ((InputManager) getSystemService(Context.INPUT_SERVICE))
            .registerInputDeviceListener(gamepadListener, null);
    }

    /* Whether the game is up.  The launcher asks before writing the game's
     * settings file: the game keeps that file in memory and writes it back
     * whole, so a change made underneath it would be lost. */
    private static volatile boolean running;

    static boolean isRunning()
    {
        return running;
    }

    @Override
    protected void onDestroy()
    {
        running = false;
        ((InputManager) getSystemService(Context.INPUT_SERVICE))
            .unregisterInputDeviceListener(gamepadListener);
        super.onDestroy();
    }

    /* Tells the native side which physical pad sits in each game slot.  It is
     * resolved here, by the launcher's rules, rather than natively, so the pad
     * the game reads and the pad GameHaptics vibrates are always the same one:
     * both come from GamepadSlots.  Sent again on every attach and detach, so a
     * pad switched on mid-race joins its slot on the next frame. */
    private void pushGamepadSlots()
    {
        GamepadSlots.Pad[] pads = GamepadSlots.resolve(GamePreferences.get(this));
        try
        {
            nativeSetGamepadSlots(pads[0] == null ? "" : pads[0].descriptor,
                                  pads[1] == null ? "" : pads[1].descriptor);
        }
        catch (UnsatisfiedLinkError notLoaded)
        {
            // SDLActivity could not load the game library, and says so itself.
        }
    }

    private final InputManager.InputDeviceListener gamepadListener =
        new InputManager.InputDeviceListener()
    {
        @Override public void onInputDeviceAdded(int deviceId) { pushGamepadSlots(); }
        @Override public void onInputDeviceRemoved(int deviceId) { pushGamepadSlots(); }
        @Override public void onInputDeviceChanged(int deviceId) { pushGamepadSlots(); }
    };

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
        if (mLayout == null || !touchEnabled)
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

    /* Which motor an effect reaches used to be guessed from whatever the player
     * touched last, and the guess could be wrong in a way nothing could
     * recover from: hold a pad whose vibration is switched off while the phone's
     * is on, and the effect was routed to the pad and then dropped there.  The
     * game now names the device slot the effect belongs to, so no guessing is
     * needed and these handlers no longer steer the output. */
    @Override public boolean dispatchTouchEvent(android.view.MotionEvent event) {
        /* Only the screen presses the on-screen controls.  A pointer driven from
         * elsewhere -- the DualSense touchpad, which Android turns into a mouse
         * pointer but reports with a finger tool type -- would otherwise press
         * whichever control the pointer happened to be over. */
        if(touchControlsOverlay!=null&&isScreenTouch(event))
            touchControlsOverlay.onScreenTouch(event);
        return super.dispatchTouchEvent(event);
    }

    /* Told apart by source, not by tool type: a touchpad pointer and a finger on
     * the screen both report TOOL_TYPE_FINGER. */
    static boolean isScreenTouch(android.view.MotionEvent event) {
        return event.isFromSource(android.view.InputDevice.SOURCE_TOUCHSCREEN);
    }

    static boolean isExternalPointer(android.view.MotionEvent event) {
        return !isScreenTouch(event)
            &&(event.isFromSource(android.view.InputDevice.SOURCE_MOUSE)
               ||event.isFromSource(android.view.InputDevice.SOURCE_TOUCHPAD));
    }

    /* A real pointer goes to SDL as a mouse, which the game's DirectInput mouse
     * then reads.  SDLSurface does this only for a mouse tool type, so a
     * touchpad pointer -- a finger tool type -- used to be taken for a touch on
     * the screen and drove the on-screen mouse emulation instead. */
    boolean forwardPointer(android.view.MotionEvent event) {
        tracePointer(event);
        int action=event.getActionMasked();
        int buttons=event.getButtonState();
        switch(action) {
        case android.view.MotionEvent.ACTION_DOWN:
            // A tap on a touchpad is a click that reports no button; it means the primary one.
            if(buttons==0)buttons=android.view.MotionEvent.BUTTON_PRIMARY;
            break;
        case android.view.MotionEvent.ACTION_CANCEL:
            action=android.view.MotionEvent.ACTION_UP;buttons=0;
            break;
        case android.view.MotionEvent.ACTION_UP:
        case android.view.MotionEvent.ACTION_MOVE:
            break;
        default:
            return false;
        }
        SDLActivity.onNativeMouse(buttons,action,event.getX(),event.getY(),false);
        return true;
    }

    /* A gamepad in the player's hands: the on-screen controls get out of the way
     * at the first press, and come back with the next touch on the screen. */
    @Override public boolean dispatchKeyEvent(android.view.KeyEvent event) {
        if(touchControlsOverlay!=null&&event.getAction()==android.view.KeyEvent.ACTION_DOWN
           &&GamepadSlots.isGamepad(event.getDevice()))
            touchControlsOverlay.hideForGamepad();
        return super.dispatchKeyEvent(event);
    }

    private static final int[] GAMEPAD_AXES={
        android.view.MotionEvent.AXIS_X,android.view.MotionEvent.AXIS_Y,
        android.view.MotionEvent.AXIS_Z,android.view.MotionEvent.AXIS_RZ,
        android.view.MotionEvent.AXIS_HAT_X,android.view.MotionEvent.AXIS_HAT_Y,
        android.view.MotionEvent.AXIS_LTRIGGER,android.view.MotionEvent.AXIS_RTRIGGER,
        android.view.MotionEvent.AXIS_GAS,android.view.MotionEvent.AXIS_BRAKE,
    };

    /* A stick, trigger or hat on a gamepad, pushed well past its resting noise. */
    static boolean isGamepadMotion(android.view.MotionEvent event) {
        if(!event.isFromSource(android.view.InputDevice.SOURCE_JOYSTICK)||!GamepadSlots.isGamepad(event.getDevice()))return false;
        for(int axis:GAMEPAD_AXES)if(Math.abs(event.getAxisValue(axis))>.5f)return true;
        return false;
    }

    @Override public boolean dispatchGenericMotionEvent(android.view.MotionEvent event) {
        if(touchControlsOverlay!=null&&isGamepadMotion(event))touchControlsOverlay.hideForGamepad();
        // Hover and scroll from a pointer SDL would ignore for its finger tool type.
        if(isExternalPointer(event)&&event.getToolType(0)!=android.view.MotionEvent.TOOL_TYPE_MOUSE) {
            int action=event.getActionMasked();
            if(action==android.view.MotionEvent.ACTION_HOVER_MOVE) {
                tracePointer(event);
                SDLActivity.onNativeMouse(0,action,event.getX(),event.getY(),false);
                return true;
            }
            if(action==android.view.MotionEvent.ACTION_SCROLL) {
                tracePointer(event);
                SDLActivity.onNativeMouse(0,action,event.getAxisValue(android.view.MotionEvent.AXIS_HSCROLL),
                    event.getAxisValue(android.view.MotionEvent.AXIS_VSCROLL),false);
                return true;
            }
        }
        return super.dispatchGenericMotionEvent(event);
    }

    /* Debug builds: which pointers actually arrive, once per kind, so a device
     * that routes differently from the DualSense shows up in the log. */
    private String lastPointerTrace;
    private void tracePointer(android.view.MotionEvent event) {
        if((getApplicationInfo().flags&android.content.pm.ApplicationInfo.FLAG_DEBUGGABLE)==0)return;
        android.view.InputDevice device=event.getDevice();
        String kind="source=0x"+Integer.toHexString(event.getSource())+" tool="+event.getToolType(0)
            +" device="+(device==null?"?":device.getName());
        if(kind.equals(lastPointerTrace))return;
        lastPointerTrace=kind;
        Log.i(TAG,"external pointer "+kind+" action="+event.getActionMasked()+" buttons="+event.getButtonState());
    }

    @Override public void onWindowFocusChanged(boolean focus) {
        super.onWindowFocusChanged(focus);
        if(haptics!=null){if(focus)haptics.resume();else haptics.pause();}
    }
    /** JNI entry point, invoked only for effects issued by the game.  `slot` is
     *  the DirectInput device the effect was created on: 0 is player one (the
     *  phone and the first pad), 1 the second pad. */
    public void onForceFeedback(int slot, float level, float impact, float road, float roadHz,
                                float engine, float engineHz) {
        long sent=android.os.SystemClock.uptimeMillis();
        /* `level` is everything mixed; the rest splits it into jolts and the
         * game's two textures with their rates (win32::Gamepad::RumbleDetail),
         * which is what a pad and the phone each play in their own way.  A level
         * that waited too long for the UI thread is dropped, but a stop never is:
         * an effect runs for seconds until it is told otherwise. */
        runOnUiThread(()->{if(haptics!=null&&(!(level>0)||android.os.SystemClock.uptimeMillis()-sent<100))
            haptics.setEffects(slot,level,impact,road,roadHz,engine,engineHz);});
    }

    /** JNI entry point, about every 40 ms while a race is drawn: how hard player
     *  one's car corners, 0 to 1, for the phone's own vibration.  A tick that
     *  waited too long for the UI thread is dropped; the next is on its way. */
    public void onPhoneTick(float turn) {
        long sent=android.os.SystemClock.uptimeMillis();
        runOnUiThread(()->{if(haptics!=null&&android.os.SystemClock.uptimeMillis()-sent<100)haptics.setTurn(turn);});
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

    /* The touch D-pad's up and down arms (TouchControlsOverlay.MENU_KEY): keys
     * for moving through menus, handed to the native side apart from SDL's
     * keyboard state, which is where it reads the pedals from.  They reach the
     * game on the next frame. */
    static void sendTouchMenuKey(int keyCode, boolean down) {
        String name=GamePreferences.keyName(keyCode);
        if(!running||"unknown".equals(name))return;
        try { nativeTouchMenuKey(name,down); }
        catch(UnsatisfiedLinkError notLoaded) { }
    }

    private static native void nativeToggleKeyboard();
    private static native void nativeSetGamepadSlots(String first, String second);
    private static native void nativeTouchMenuKey(String key, boolean down);

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

        /* No system arrow over the game.  The game draws its own cursor and
         * moves it from the mouse deltas SDL passes on, so Android's pointer
         * would be a second cursor that does not even agree with the first --
         * the two drift apart as soon as the game's own sensitivity or the
         * letterboxed 4:3 picture comes into it. */
        @Override public android.view.PointerIcon onResolvePointerIcon(android.view.MotionEvent event, int pointerIndex) {
            return android.view.PointerIcon.getSystemIcon(getContext(), android.view.PointerIcon.TYPE_NULL);
        }

        @Override public boolean onTouch(android.view.View v, android.view.MotionEvent event) {
            if(isExternalPointer(event))
                return forwardPointer(event);
            /* Touch switched off: the finger is taken here, so SDL does not
             * turn it into a mouse of its own either. */
            if(!touchEnabled)
                return true;
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
