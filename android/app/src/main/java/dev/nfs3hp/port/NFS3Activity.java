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
    /* A finger works the game's pointer as a touchpad rather than putting it
     * where it lands (GamePreferences.TOUCH_POINTER).  Read once, likewise. */
    private boolean touchpadPointer;

    /* The port tears its Application down and calls SDL_Quit before main
     * returns, so it can safely be launched again.  Without this opt-in SDL's
     * stock glue calls System.exit(0) on the next launch and takes the launcher
     * down with it. */
    @Override
    protected boolean allowActivityRecreation()
    {
        return true;
    }

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
        touchpadPointer = GamePreferences.TOUCH_POINTER_TOUCHPAD.equals(
            preferences.getString(GamePreferences.TOUCH_POINTER, GamePreferences.TOUCH_POINTER_TAP));
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
            {
                setEnv(GamepadButtons.environmentName(slot,GamepadButtons.BUTTON_IDS[button]),
                    GamepadButtons.environmentValue(slot,GamepadButtons.action(preferences,slot,button)));
                /* And what the same button sends while a menu or a replay is on
                 * the screen, where driving actions mean nothing. */
                setEnv(GamepadButtons.menuEnvironmentName(slot,GamepadButtons.BUTTON_IDS[button]),
                    GamepadButtons.menuEnvironmentValue(button));
            }
        /* How player one is about to drive, decided before the game reads its
         * settings.  With a pad in the first slot the game steers on that pad's
         * axes and the on-screen controls reach them through the port; with no
         * pad the on-screen controls are bound as the keyboard they are, and
         * the game steers them itself -- which is what makes a race driven on
         * them one the game can replay.  The native side is told which it is,
         * because in the second case it must keep out of the way: no folding
         * those keys into axes, and no holding the game's steering flag up. */
        boolean padForPlayerOne = GamepadSlots.resolve(preferences)[0] != null;
        java.io.File dataRoot = getExternalFilesDir(null);
        ControlProfile.Kind driving = dataRoot == null ? null
            : ControlProfile.driveWith(dataRoot, preferences, padForPlayerOne);
        setEnv("NFS_TOUCH_DRIVE", driving == ControlProfile.Kind.TOUCH ? "keys" : "axes");
        Log.i(TAG, "driving with " + (driving == null ? "the player's own controls" : driving)
            + (padForPlayerOne ? ", pad in slot 1" : ", no pad"));

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
        /* The phone's own output rate, for the game's sound (audio.cpp): a stream
         * at any other rate is resampled inside Android, off its low-latency
         * path, and on some phones that alone delays the sound by a quarter of a
         * second or more.  Logged with what the sound goes out through, since
         * Bluetooth adds a delay of its own whatever the rate. */
        android.media.AudioManager audio=(android.media.AudioManager)getSystemService(AUDIO_SERVICE);
        if(audio!=null) {
            String nativeRate=audio.getProperty(android.media.AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE);
            if(nativeRate!=null)setEnv("NFS_AUDIO_RATE",nativeRate);
            Log.i(TAG,"sound: the phone's own rate "+nativeRate+" Hz, a burst of "
                +audio.getProperty(android.media.AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER)
                +" frames, "+(bluetoothOutput(audio)?"a Bluetooth output connected":"no Bluetooth output"));
        }
        /* Debug builds log the car detail the game settles on, once a second
         * during a race ([CARDETAIL] in logcat): the level each car was drawn
         * at against the game's budget, the transform buffer and the atlas. */
        /* The traces that answer a question while it is being asked, and are
         * noise the rest of the time: the car detail the game settles on
         * ([CARDETAIL]), its clock against real time ([TICKS]) and what player
         * one's car is driven with ([INPUT]).  Off unless the launch asks for
         * one, which costs a line:
         *   adb shell am start -n dev.nfs3hp.port/.SplashActivity --ez trace_ticks true
         * The ones that speak only when something changes -- the layout, the
         * pads, the steering, text entry -- stay on: they are a handful of
         * lines a session and they are what makes a report readable. */
        if (getIntent() != null)
        {
            if (getIntent().getBooleanExtra("trace_car_detail", false))
                setEnv("NFS_CAR_DETAIL_TRACE", "1");
            if (getIntent().getBooleanExtra("trace_ticks", false))
                setEnv("NFS_TICK_TRACE", "1");
            if (getIntent().getBooleanExtra("trace_input", false))
                setEnv("NFS_INPUT_TRACE", "1");
        }

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
        desktopWindow();
        super.onCreate(savedInstanceState);
        running = true;
        attachTouchControls();
        pushGamepadSlots();
        ((InputManager) getSystemService(Context.INPUT_SERVICE))
            .registerInputDeviceListener(gamepadListener, null);
    }

    /* Whether the game is up, for its own process.  The launcher runs in
     * another one and asks the system instead (GameProcess.running): it writes
     * the game's settings file, which the game keeps in memory and writes back
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
        /* The game's process ends with the game.  SDL and the machine under it
         * are set up once per process; a second race in the same one died in
         * SDLThread before it drew anything.  Ending here means the next race
         * starts in a process of its own, as the first one did, and the
         * launcher -- another process since this was split off -- stays up. */
        if (isFinishing())
            System.exit(0);
    }

    /* Tells the native side which physical pad sits in each game slot.  It is
     * resolved here, by the launcher's rules, rather than natively, so the pad
     * the game reads and the pad GameHaptics vibrates are always the same one:
     * both come from GamepadSlots.  Sent again on every attach and detach, so a
     * pad switched on mid-race joins its slot on the next frame. */
    private void pushGamepadSlots()
    {
        GamepadSlots.Pad[] pads = GamepadSlots.resolve(GamePreferences.get(this));
        firstPlayersPad = pads[0] == null ? -1 : pads[0].id;
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

    /* Which way the window has to lie.  SDL asks Android for it once the game's
     * window exists, from that window's size and SDL_HINT_ORIENTATIONS, and the
     * manifest asks for landscape before that: right for a phone, where the
     * screen is the game's.
     *
     * On a desktop -- Samsung DeX -- the game is a window on a monitor, and an
     * activity that insists on a shape gets a window of that shape: the system
     * lays it inside the task's window and leaves the rest of it showing what
     * was there before, which is the launcher behind the game.  So there the
     * request is let go and the window is the player's: it opens across the
     * display (the manifest's <layout>) and is resized from the frame after
     * that.  What the game draws still fills whatever the window becomes. */
    private void desktopWindow()
    {
        if (!DesktopMode.active(this))
            return;
        Log.i(TAG, "desktop mode: the window's shape is the player's");
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED);
    }

    @Override
    public void setOrientationBis(int w, int h, boolean resizable, String hint)
    {
        if (DesktopMode.active(this))
        {
            desktopWindow();
            return;
        }
        super.setOrientationBis(w, h, resizable, hint);
    }

    @Override
    protected SDLSurface createSDLSurface(Context context)
    {
        gameSurface = new GameSurface(context);
        /* A pad's press takes Android out of touch mode, and the focused view --
         * this one, the whole window -- then wears the system's focus
         * highlight: a green line round the screen for as long as the pad is
         * used.  The game highlights its own items. */
        gameSurface.setDefaultFocusHighlightEnabled(false);
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
        if (gameSurface != null) gameSurface.cancelPointer();
        if(haptics!=null)haptics.pause();
        super.onPause();
    }

    private void attachTouchControls()
    {
        if (mLayout == null || !touchEnabled)
            return;

        if (touchControlsOverlay == null)
        {
            touchControlsOverlay = new TouchControlsOverlay(this);
            touchControlsOverlay.setRaceControls(raceGears, raceSpikes);
        }

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
    /* What the player last pressed with, for the phone's keyboard (onTextEntry):
     * a box opened from a pad brings it up at once, since a pad has nothing
     * else to type with.  Opened any other way the box waits for a finger on
     * the line being typed in (onTextFieldTap).  Presses only: a mouse resting
     * on the desk still reports its hovering. */
    private static final int INPUT_TOUCH=0,INPUT_PAD=1,INPUT_POINTER=2,INPUT_KEYBOARD=3;
    private int lastInput=INPUT_TOUCH;

    /* A keyboard with letters on it, plugged in or paired -- not the phone's own
     * volume keys, and not the on-screen keyboard's keys either. */
    private static boolean isRealKeyboard(android.view.InputDevice device) {
        return device!=null&&!device.isVirtual()&&!GamepadSlots.isGamepad(device)
            &&device.getKeyboardType()==android.view.InputDevice.KEYBOARD_TYPE_ALPHABETIC;
    }

    /* With a real keyboard at hand the phone's is never wanted: the name is
     * typed on the one there is. */
    private static boolean realKeyboardConnected() {
        for(int id:android.view.InputDevice.getDeviceIds())
            if(isRealKeyboard(android.view.InputDevice.getDevice(id)))return true;
        return false;
    }

    @Override public boolean dispatchTouchEvent(android.view.MotionEvent event) {
        if(event.getActionMasked()==android.view.MotionEvent.ACTION_DOWN)
            lastInput=isExternalPointer(event)?INPUT_POINTER:(isScreenTouch(event)?INPUT_TOUCH:lastInput);
        /* Only the screen presses the on-screen controls.  A pointer driven from
         * elsewhere -- a mouse, or the DualSense touchpad, which Android turns
         * into a mouse pointer but reports with a finger tool type -- goes
         * straight to the game, or it would press whichever control the pointer
         * happened to be over. */
        if(isExternalPointer(event))
            return forwardPointer(event);
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

    /* A real pointer -- a mouse, or a touchpad Android drives as one -- is the
     * game's pointer the way a finger is: where it is on the screen, and which
     * buttons are down (nativeMousePointer).  It used to go to SDL as a mouse
     * and reach the game as the distance it moved, which counts in pixels of the
     * phone's screen while the game's cursor counts in the 640 across a menu, so
     * the two parted company at once.  It is not the finger's pointer either:
     * its buttons are real, so a press is a press, and it keeps working in a
     * race, where a finger does not. */
    private boolean implicitPrimary;
    boolean forwardPointer(android.view.MotionEvent event) {
        tracePointer(event);
        int buttons=mouseButtons(event.getButtonState());
        switch(event.getActionMasked()) {
        case android.view.MotionEvent.ACTION_DOWN:
            // A tap on a touchpad is a click that reports no button; it means the primary one.
            implicitPrimary=buttons==0;
            break;
        case android.view.MotionEvent.ACTION_MOVE:
            break;
        case android.view.MotionEvent.ACTION_UP:
        case android.view.MotionEvent.ACTION_CANCEL:
            implicitPrimary=false;buttons=0;
            break;
        default:
            return true;
        }
        if(implicitPrimary)buttons|=1;
        nativeMousePointer(event.getX(),event.getY(),buttons);
        return true;
    }

    /* Android's button state as the native side counts buttons: bit 0 left,
     * 1 right, 2 middle. */
    private static int mouseButtons(int state) {
        int buttons=0;
        if((state&android.view.MotionEvent.BUTTON_PRIMARY)!=0)buttons|=1;
        if((state&android.view.MotionEvent.BUTTON_SECONDARY)!=0)buttons|=2;
        if((state&android.view.MotionEvent.BUTTON_TERTIARY)!=0)buttons|=4;
        return buttons;
    }

    /* The first player's pad, as GamepadSlots resolves the slots, kept from every
     * attach and detach; -1 for none.  Only that pad puts the on-screen controls
     * away: the second player drives split screen with a pad of their own while
     * the first drives with the screen, and a press of theirs must not take the
     * first player's controls out from under their thumbs.  The controls then
     * follow their own auto-hide setting, as they do with no pad at all. */
    private int firstPlayersPad = -1;

    private boolean fromFirstPlayersPad(android.view.InputDevice device) {
        return device != null && device.getId() == firstPlayersPad && GamepadSlots.isGamepad(device);
    }

    /* A gamepad in the player's hands: the on-screen controls get out of the way
     * at the first press, and come back with the next touch on the screen. */
    @Override public boolean dispatchKeyEvent(android.view.KeyEvent event) {
        if(event.getAction()==android.view.KeyEvent.ACTION_DOWN) {
            if(GamepadSlots.isGamepad(event.getDevice()))lastInput=INPUT_PAD;
            else if(isRealKeyboard(event.getDevice()))lastInput=INPUT_KEYBOARD;
        }
        if(touchControlsOverlay!=null&&event.getAction()==android.view.KeyEvent.ACTION_DOWN
           &&fromFirstPlayersPad(event.getDevice()))
            touchControlsOverlay.hideForGamepad();
        if(isSystemBack(event)) {
            systemBack(event);
            return true;
        }
        return super.dispatchKeyEvent(event);
    }

    /* The system's back -- the gesture, or the navigation bar's button -- is the
     * game's Esc, in its menus and in a race alike: it leaves a screen, and in a
     * race it opens the pause menu.  SDL used to take the key for a key of its
     * own, AC_BACK, which the game knows nothing of, so back did nothing at all.
     * A back button on a gamepad stays the pad's, which the player maps
     * themselves, and a mouse's stays out of it as SDL keeps it. */
    private static boolean isSystemBack(android.view.KeyEvent event) {
        return event.getKeyCode()==android.view.KeyEvent.KEYCODE_BACK
            &&!GamepadSlots.isGamepad(event.getDevice())
            &&!event.isFromSource(android.view.InputDevice.SOURCE_MOUSE);
    }

    /* With the keyboard up, back puts it away, as it does everywhere else, and
     * leaves the screen being typed into where it is.
     *
     * Esc stays down for ESCAPE_HOLD_MS at least.  The back gesture reports its
     * key going down and up together, once the gesture is over, and a race does
     * not wait for key messages: it looks at which keys are down once a frame,
     * so an Esc let go before the next frame never paused it, while the menus,
     * which read the messages, took it.  The overlay's pause button holds its
     * key as long for the same reason. */
    private static final long ESCAPE_HOLD_MS=100;
    private final android.os.Handler backHandler=new android.os.Handler(android.os.Looper.getMainLooper());
    private final Runnable escapeUp=()->SDLActivity.onNativeKeyUp(android.view.KeyEvent.KEYCODE_ESCAPE);
    private boolean backClosesKeyboard;
    private long escapeDownAt;
    private void systemBack(android.view.KeyEvent event) {
        switch(event.getAction()) {
        case android.view.KeyEvent.ACTION_DOWN:
            if(event.getRepeatCount()>0)return;
            backClosesKeyboard=nativeKeyboardActive();
            if(!backClosesKeyboard) {
                backHandler.removeCallbacks(escapeUp);
                escapeDownAt=android.os.SystemClock.uptimeMillis();
                SDLActivity.onNativeKeyDown(android.view.KeyEvent.KEYCODE_ESCAPE);
            }
            break;
        case android.view.KeyEvent.ACTION_UP:
            if(backClosesKeyboard) {
                if(nativeKeyboardActive())nativeToggleKeyboard();
            } else {
                long held=android.os.SystemClock.uptimeMillis()-escapeDownAt;
                backHandler.postDelayed(escapeUp,Math.max(0,ESCAPE_HOLD_MS-held));
            }
            backClosesKeyboard=false;
            break;
        default:
            break;
        }
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
        if(isGamepadMotion(event))lastInput=INPUT_PAD;
        else if(isExternalPointer(event)&&event.getActionMasked()==android.view.MotionEvent.ACTION_BUTTON_PRESS)
            lastInput=INPUT_POINTER;
        if(touchControlsOverlay!=null&&isGamepadMotion(event)&&fromFirstPlayersPad(event.getDevice()))touchControlsOverlay.hideForGamepad();
        /* The rest of a real pointer: moving with no button down, and buttons
         * pressed while another is held, which Android reports here rather than
         * as a touch.  The wheel still goes to SDL, which the game's DirectInput
         * mouse reads as its own. */
        if(isExternalPointer(event)) {
            int action=event.getActionMasked();
            switch(action) {
            case android.view.MotionEvent.ACTION_HOVER_ENTER:
            case android.view.MotionEvent.ACTION_HOVER_MOVE:
            case android.view.MotionEvent.ACTION_BUTTON_PRESS:
            case android.view.MotionEvent.ACTION_BUTTON_RELEASE:
                tracePointer(event);
                nativeMousePointer(event.getX(),event.getY(),
                    mouseButtons(event.getButtonState())|(implicitPrimary?1:0));
                return true;
            case android.view.MotionEvent.ACTION_SCROLL:
                tracePointer(event);
                SDLActivity.onNativeMouse(0,action,event.getAxisValue(android.view.MotionEvent.AXIS_HSCROLL),
                    event.getAxisValue(android.view.MotionEvent.AXIS_VSCROLL),false);
                return true;
            default:
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

    /** Whether the player is driving, from the game itself (nfs3hp_main.cpp):
     *  the on-screen controls carry the racing layout while a race is running
     *  with nothing over it, and the menu layout the rest of the time -- in the
     *  front end, and while a menu stands over a race. */
    public void onGameDriving(boolean driving) {
        runOnUiThread(()->{if(touchControlsOverlay!=null)touchControlsOverlay.setMenuMode(!driving);});
    }

    /** The game is taking a name (nfs3hp_main.cpp): the phone's keyboard comes
     *  up for as long as it is, and goes away when the typing is over, so a
     *  player never has to ask for it.  Raised through SDL, as the overlay's
     *  own keyboard button is -- a keyboard raised behind SDL's back deletes
     *  but types nothing -- and from the UI thread, which is where that has to
     *  happen.  At once only for a box opened from a pad (lastInput); a finger
     *  or a pointer opens the box and then the keyboard with a tap on the line
     *  being typed in (onTextFieldTap).  Never while a real keyboard is
     *  connected.  Going away is unconditional. */
    public void onTextEntry(boolean typing) {
        runOnUiThread(()->{
            if(typing&&(lastInput!=INPUT_PAD||realKeyboardConnected()))return;
            if(typing!=nativeKeyboardActive())nativeToggleKeyboard();
        });
    }

    /** A finger on the picture while the game's name box is up (nfs3hp_main.cpp):
     *  on the line being typed in, the keyboard comes up; anywhere else it goes
     *  away, as a tap beside a field does everywhere.  The tap still reaches the
     *  game, so one on Ok or Cancel does what it says as well. */
    public void onTextFieldTap(boolean inside) {
        runOnUiThread(()->{
            boolean wanted=inside&&!realKeyboardConnected();
            if(wanted!=nativeKeyboardActive())nativeToggleKeyboard();
        });
    }

    /* Which of the racing layout's buttons player one's car can use, from the
     * game (nfs3hp_main.cpp): the gears only with a manual gearbox, the spike
     * strip only in a police car.  Kept for an overlay made afterwards. */
    private boolean raceGears=true,raceSpikes=true;
    public void onRaceControls(boolean gears,boolean spikes) {
        runOnUiThread(()->{
            raceGears=gears;raceSpikes=spikes;
            if(touchControlsOverlay!=null)touchControlsOverlay.setRaceControls(gears,spikes);
        });
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

    /* A finger on the game's picture, in window pixels: TouchPointer.DOWN, MOVE
     * or UP.  The game reads it as its own pointer -- see nfs3hp_main.cpp, which
     * turns the place the finger reached into the movement the game's cursor
     * needs to get there. */
    private static native void nativeScreenTouch(int action, float x, float y);
    /* The finger on the touchpad instead: how far it slid, in window pixels,
     * and whether it holds the button (bit 0). */
    private static native void nativeTouchpad(float dx, float dy, int buttons);
    /* A real mouse, in window pixels, with its buttons: bit 0 left, 1 right,
     * 2 middle. */
    private static native void nativeMousePointer(float x, float y, int buttons);
    private static native void nativeToggleKeyboard();
    private static native boolean nativeKeyboardActive();
    private static native void nativeSetGamepadSlots(String first, String second);

    /* Whether sound can be going out over Bluetooth: Android sends media there
     * whenever such an output is connected. */
    private static boolean bluetoothOutput(android.media.AudioManager audio) {
        for(android.media.AudioDeviceInfo device:audio.getDevices(android.media.AudioManager.GET_DEVICES_OUTPUTS)) {
            int type=device.getType();
            if(type==android.media.AudioDeviceInfo.TYPE_BLUETOOTH_A2DP
               ||type==android.media.AudioDeviceInfo.TYPE_BLUETOOTH_SCO
               ||type==26/* TYPE_BLE_HEADSET, API 31 */||type==27/* TYPE_BLE_SPEAKER */)
                return true;
        }
        return false;
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
        final TouchPointer pointer;
        final TouchpadPointer touchpad;
        GameSurface(Context context)
        {
            super(context);
            pointer = new TouchPointer(this, NFS3Activity::nativeScreenTouch);
            touchpad = new TouchpadPointer(this, NFS3Activity::nativeTouchpad);
        }

        void cancelPointer() {
            pointer.cancel();
            touchpad.cancel();
        }

        /* No system arrow over the game.  The game draws its own cursor, and a
         * finger puts it exactly where it touched, so Android's pointer would be
         * a second cursor sitting somewhere else. */
        @Override public android.view.PointerIcon onResolvePointerIcon(android.view.MotionEvent event, int pointerIndex) {
            return android.view.PointerIcon.getSystemIcon(getContext(), android.view.PointerIcon.TYPE_NULL);
        }

        @Override public boolean onTouch(android.view.View v, android.view.MotionEvent event) {
            /* A real pointer never gets this far: dispatchTouchEvent hands it
             * to the game first.  Touch switched off: the finger is taken here,
             * so SDL does not turn it into a mouse of its own either. */
            if(!touchEnabled)
                return true;
            int tool=event.getToolType(0);
            if(tool==android.view.MotionEvent.TOOL_TYPE_FINGER||tool==android.view.MotionEvent.TOOL_TYPE_UNKNOWN)
                return touchpadPointer?touchpad.onTouch(event):pointer.onTouch(event);
            return super.onTouch(v,event);
        }

        @Override public void onWindowFocusChanged(boolean focus) {
            super.onWindowFocusChanged(focus);if(!focus)cancelPointer();
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
