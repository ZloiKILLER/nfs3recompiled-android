package dev.nfs3hp.port

import android.content.Context
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.Typeface
import android.text.Spanned
import android.text.style.StyleSpan
import android.widget.FrameLayout
import androidx.compose.animation.AnimatedContent
import androidx.compose.animation.core.tween
import androidx.compose.animation.fadeIn
import androidx.compose.animation.fadeOut
import androidx.compose.animation.slideInHorizontally
import androidx.compose.animation.slideOutHorizontally
import androidx.compose.animation.togetherWith
import androidx.compose.foundation.BorderStroke
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.BoxWithConstraints
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxHeight
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.layout.widthIn
import androidx.compose.foundation.layout.windowInsetsPadding
import androidx.compose.foundation.pager.HorizontalPager
import androidx.compose.foundation.pager.rememberPagerState
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.text.BasicText
import androidx.compose.foundation.text.InlineTextContent
import androidx.compose.foundation.text.TextAutoSize
import androidx.compose.foundation.text.appendInlineContent
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Icon
import androidx.compose.material3.LinearProgressIndicator
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.CompositionLocalProvider
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.key
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.asImageBitmap
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.AnnotatedString
import androidx.compose.ui.text.Placeholder
import androidx.compose.ui.text.PlaceholderVerticalAlign
import androidx.compose.ui.text.SpanStyle
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.text.buildAnnotatedString
import androidx.compose.ui.text.font.FontStyle
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.Density
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.em
import androidx.compose.ui.unit.sp
import androidx.compose.ui.viewinterop.AndroidView
import kotlinx.coroutines.launch

/** Every screen of the launcher, by the one the activity says is up, moving
 *  as Material moves between the levels of an app: a screen further in comes
 *  from the right, going back returns from the left, each fading as it goes. */
@Composable
fun LauncherScreens(a: LauncherActivity) {
    AnimatedContent(
        targetState = a.screen,
        transitionSpec = {
            val forward = depth(targetState) >= depth(initialState)
            val shift = { width: Int -> width / 10 }
            (slideInHorizontally(tween(260)) { if (forward) shift(it) else -shift(it) } + fadeIn(tween(260))) togetherWith
                (slideOutHorizontally(tween(200)) { if (forward) -shift(it) else shift(it) } + fadeOut(tween(200)))
        },
        label = "screen",
    ) { screen -> Screen(a, screen) }
    a.dialog?.let { DialogFor(a, it) }
}

/* How far in a screen is: the first screen, the menus off it, the screens
 * those open, and the ones below those. */
private fun depth(screen: LauncherActivity.Screen): Int = when (screen) {
    LauncherActivity.Screen.Main -> 0
    LauncherActivity.Screen.Controls, LauncherActivity.Screen.Display, LauncherActivity.Screen.Data,
    LauncherActivity.Screen.Language, LauncherActivity.Screen.Faq -> 1
    LauncherActivity.Screen.TouchKeys, LauncherActivity.Screen.GamepadButtons,
    LauncherActivity.Screen.ControlsHelp, LauncherActivity.Screen.DataSets, LauncherActivity.Screen.Editor -> 3
    else -> 2
}

@Composable
private fun Screen(a: LauncherActivity, screen: LauncherActivity.Screen) {
    when (screen) {
        LauncherActivity.Screen.Main -> MainScreen(a)
        LauncherActivity.Screen.Language -> LanguageScreen(a)
        LauncherActivity.Screen.Data -> DataScreen(a)
        LauncherActivity.Screen.GameData -> GameDataScreen(a)
        LauncherActivity.Screen.DataSets -> DataSetsScreen(a)
        LauncherActivity.Screen.Saves -> SavesScreen(a)
        LauncherActivity.Screen.LauncherSettings -> LauncherSettingsScreen(a)
        LauncherActivity.Screen.Controls -> ControlsScreen(a)
        LauncherActivity.Screen.Touch -> TouchScreen(a)
        LauncherActivity.Screen.TouchKeys -> TouchKeysScreen(a)
        LauncherActivity.Screen.Editor -> Unit  // a View of its own (LauncherActivity.openEditor)
        LauncherActivity.Screen.Gamepads -> GamepadsScreen(a)
        LauncherActivity.Screen.GamepadButtons -> GamepadButtonsScreen(a)
        LauncherActivity.Screen.ControlsHelp -> ControlsHelpScreen(a)
        LauncherActivity.Screen.Faq -> FaqScreen(a)
        LauncherActivity.Screen.Display -> DisplayScreen(a)
        LauncherActivity.Screen.Adjustment -> AdjustmentScreen(a)
    }
}

@Composable
private fun DialogFor(a: LauncherActivity, d: LauncherActivity.Dialog) {
    val close: () -> Unit = {
        a.dialog = null
        d.onClose?.invoke()
    }
    AlertDialog(
        onDismissRequest = close,
        confirmButton = {
            TextButton(onClick = {
                a.dialog = null
                d.onConfirm?.invoke()
                d.onClose?.invoke()
            }) { Text(d.confirm, color = GameColors.Gold, fontWeight = FontWeight.Bold) }
        },
        dismissButton = if (d.onConfirm == null) null else {
            { TextButton(onClick = close) { Text(stringResource(android.R.string.cancel), color = GameColors.MenuBlue) } }
        },
        title = { Text(d.title, color = GameColors.Gold, fontWeight = FontWeight.Black) },
        text = {
            Text(d.message, color = GameColors.Silver, modifier = Modifier.verticalScroll(rememberScrollState()))
        },
        containerColor = MaterialTheme.colorScheme.surfaceContainerHigh,
    )
}

/* The width a column of menu buttons keeps on a wide screen. */
private val MENU_WIDTH = 440.dp

// ---- The first screen ----

/* The title card with Play on the left, the settings on the right, and the
 * build's version under them. */
@Composable
private fun MainScreen(a: LauncherActivity) {
    val active = a.activeSet
    BoxWithConstraints(
        Modifier
            .fillMaxSize()
            .windowInsetsPadding(LauncherInsets)
            .padding(24.dp)
    ) {
        val narrow = maxWidth < NARROW
        val card = @Composable { modifier: Modifier ->
            Column(
                modifier
                    .clip(RoundedCornerShape(24.dp))
                    .background(GameColors.PanelFill)
                    .border(1.dp, GameColors.PillEdge.copy(alpha = 0.5f), RoundedCornerShape(24.dp))
                    .padding(28.dp),
                verticalArrangement = Arrangement.Center,
            ) {
                GameTitle()
                Spacer(Modifier.height(24.dp))
                GoldButton(
                    stringResource(R.string.play),
                    onClick = { a.startGame() },
                    enabled = active != null,
                    modifier = Modifier.fillMaxWidth(),
                    height = 72.dp,
                )
                if (active == null) {
                    Spacer(Modifier.height(8.dp))
                    Hint(stringResource(R.string.no_active_data_set))
                }
            }
        }
        val menu = @Composable { each: Modifier ->
            MainMenuButton(stringResource(R.string.controls_hub), each) { a.go(LauncherActivity.Screen.Controls) }
            MainMenuButton(stringResource(R.string.display_settings), each) { a.go(LauncherActivity.Screen.Display) }
            MainMenuButton(stringResource(R.string.data_settings), each) { a.go(LauncherActivity.Screen.Data) }
            MainMenuButton(stringResource(R.string.language_label), each) { a.go(LauncherActivity.Screen.Language) }
            MainMenuButton(stringResource(R.string.faq_title), each) { a.go(LauncherActivity.Screen.Faq) }
        }
        val version = @Composable {
            Text(
                a.versionLabel(),
                modifier = Modifier.fillMaxWidth().padding(top = 8.dp),
                color = GameColors.Muted,
                fontSize = 12.sp,
                letterSpacing = 1.sp,
                textAlign = androidx.compose.ui.text.style.TextAlign.End,
            )
        }
        if (narrow) {
            Column(Modifier.fillMaxSize().verticalScroll(rememberScrollState()), verticalArrangement = Arrangement.spacedBy(10.dp)) {
                card(Modifier.fillMaxWidth())
                menu(Modifier.fillMaxWidth())
                version()
            }
        } else {
            /* One block, one height: the card fills it and the five buttons
             * share it, so the first button's top is the card's top and the
             * last one's bottom the card's bottom, whatever the window --
             * never measured from what the buttons happen to hold, which is
             * what let the two drift apart once the window settled. */
            val block = (maxHeight - 32.dp).coerceIn(260.dp, 440.dp)
            Column(Modifier.fillMaxSize(), verticalArrangement = Arrangement.Center) {
                Row(Modifier.fillMaxWidth().height(block), horizontalArrangement = Arrangement.spacedBy(24.dp)) {
                    card(Modifier.weight(1.25f).fillMaxHeight())
                    Column(Modifier.weight(1f).fillMaxHeight(), verticalArrangement = Arrangement.spacedBy(10.dp)) {
                        menu(Modifier.fillMaxWidth().weight(1f))
                    }
                }
                version()
            }
        }
    }
}

/* Below this width a window gets one column instead of two: a small DeX
 * window, a split screen. */
private val NARROW = 640.dp

@Composable
private fun MainMenuButton(text: String, modifier: Modifier = Modifier.fillMaxWidth(), onClick: () -> Unit) {
    MenuButton(text, onClick, modifier, height = 44.dp)
}

/* The name as the game's title card has it: two lines of heavy white
 * capitals, centred, each fitted to the card's width. */
@Composable
private fun GameTitle() {
    val style = TextStyle(
        fontWeight = FontWeight.Black,
        letterSpacing = 0.5.sp,
        color = GameColors.Silver,
        textAlign = androidx.compose.ui.text.style.TextAlign.Center,
    )
    BasicText(
        "NEED FOR SPEED III",
        modifier = Modifier.fillMaxWidth(),
        style = style,
        maxLines = 1,
        autoSize = TextAutoSize.StepBased(minFontSize = 16.sp, maxFontSize = 46.sp),
    )
    BasicText(
        "HOT PURSUIT",
        modifier = Modifier.fillMaxWidth(),
        style = style.copy(letterSpacing = 3.sp),
        maxLines = 1,
        autoSize = TextAutoSize.StepBased(minFontSize = 14.sp, maxFontSize = 34.sp),
    )
}

// ---- Language ----

/** Language of the launcher only: the game's own menus come from FEDATA and
 *  stay in the language of the disc. */
@Composable
private fun LanguageScreen(a: LauncherActivity) {
    val preferences = remember { GamePreferences.get(a) }
    val tags = remember { a.resources.getStringArray(R.array.language_tags) }
    val names = remember { a.resources.getStringArray(R.array.language_names) }
    val current = preferences.getString(GamePreferences.UI_LANGUAGE, GamePreferences.UI_LANGUAGE_SYSTEM)
    ScreenFrame(stringResource(R.string.language_label), onBack = { a.go(LauncherActivity.Screen.Main) }) {
        Hint(stringResource(R.string.language_note))
        Panel(Modifier.widthIn(max = MENU_WIDTH)) {
            ChoiceList(names.toList(), tags.indexOf(current).coerceAtLeast(0)) { index ->
                if (tags[index] != current) {
                    // commit, not apply: recreate() reads this back in attachBaseContext.
                    preferences.edit().putString(GamePreferences.UI_LANGUAGE, tags[index]).commit()
                    a.recreate()
                }
            }
        }
    }
}

// ---- Data ----

/** The Data menu: game data, saved games and the launcher's own settings. */
@Composable
private fun DataScreen(a: LauncherActivity) {
    ScreenFrame(stringResource(R.string.data_settings), onBack = { a.go(LauncherActivity.Screen.Main) }) {
        Column(Modifier.widthIn(max = MENU_WIDTH), verticalArrangement = Arrangement.spacedBy(10.dp)) {
            MainMenuButton(stringResource(R.string.game_data_title)) { a.go(LauncherActivity.Screen.GameData) }
            MainMenuButton(stringResource(R.string.saves_title)) { a.go(LauncherActivity.Screen.Saves) }
            MainMenuButton(stringResource(R.string.launcher_settings_title)) { a.go(LauncherActivity.Screen.LauncherSettings) }
        }
    }
}

/* How far a long operation is, under the buttons that started it. */
@Composable
private fun OperationStatus(a: LauncherActivity) {
    if (a.busy) {
        if (a.importPercent < 0)
            LinearProgressIndicator(Modifier.fillMaxWidth(), color = GameColors.Gold, trackColor = GameColors.Pill)
        else
            LinearProgressIndicator(
                progress = { a.importPercent / 100f },
                modifier = Modifier.fillMaxWidth(),
                color = GameColors.Gold,
                trackColor = GameColors.Pill,
            )
    }
    if (a.status.isNotEmpty())
        Hint(a.status, color = if (a.busy) GameColors.MenuBlue else GameColors.Gold)
}

/* The data set the game plays, a way to change it, and the two ways to bring
 * one in. */
@Composable
private fun GameDataScreen(a: LauncherActivity) {
    val active = a.activeSet
    ScreenFrame(stringResource(R.string.game_data_title), onBack = { a.go(LauncherActivity.Screen.Data) }, backEnabled = !a.busy) {
        Panel(Modifier.widthIn(max = 640.dp)) {
            Row(verticalAlignment = Alignment.CenterVertically) {
                Column(Modifier.weight(1f)) {
                    if (active != null) {
                        val line = stringResource(R.string.active_data_set, active.name)
                        val at = line.indexOf(active.name)
                        Text(
                            buildAnnotatedString {
                                append(line)
                                if (at >= 0)
                                    addStyle(SpanStyle(color = GameColors.Gold, fontWeight = FontWeight.Bold), at, at + active.name.length)
                            },
                            color = GameColors.Silver,
                            style = MaterialTheme.typography.titleMedium,
                        )
                    } else {
                        Hint(stringResource(R.string.no_active_data_set), color = GameColors.Silver)
                    }
                }
                Spacer(Modifier.width(12.dp))
                MenuButton(
                    stringResource(R.string.data_change),
                    onClick = { a.go(LauncherActivity.Screen.DataSets) },
                    enabled = !a.busy && a.dataSets.isNotEmpty(),
                    height = 44.dp,
                )
            }
        }
        Hint(stringResource(R.string.data_import_hint), Modifier.widthIn(max = 640.dp))
        Row(Modifier.widthIn(max = 640.dp), horizontalArrangement = Arrangement.spacedBy(12.dp)) {
            MenuButton(stringResource(R.string.import_folder), { a.launchFolderPicker() }, Modifier.weight(1f), enabled = !a.busy)
            MenuButton(stringResource(R.string.import_zip), { a.launchZipPicker() }, Modifier.weight(1f), enabled = !a.busy)
        }
        Column(Modifier.widthIn(max = 640.dp), verticalArrangement = Arrangement.spacedBy(6.dp)) {
            OperationStatus(a)
        }
    }
}

/* Every data set on the device: make one the game's, or delete one. */
@Composable
private fun DataSetsScreen(a: LauncherActivity) {
    ScreenFrame(stringResource(R.string.game_data_title), onBack = { a.go(LauncherActivity.Screen.GameData) }) {
        a.dataSets.forEach { set ->
            Panel(Modifier.widthIn(max = 640.dp)) {
                Row(verticalAlignment = Alignment.CenterVertically, horizontalArrangement = Arrangement.spacedBy(10.dp)) {
                    Text(
                        set.name,
                        Modifier.weight(1f),
                        color = if (set.active) GameColors.Gold else GameColors.Silver,
                        fontWeight = if (set.active) FontWeight.Bold else FontWeight.Normal,
                        style = MaterialTheme.typography.titleMedium,
                    )
                    if (set.active)
                        Text(stringResource(R.string.data_set_active_suffix).trim().trim('(', ')'),
                            color = GameColors.Gold, style = MaterialTheme.typography.labelLarge)
                    else
                        MenuButton(stringResource(R.string.switch_data), { a.switchDataSet(set) }, height = 40.dp)
                    QuietButton(stringResource(R.string.delete), { a.confirmDelete(set) })
                }
            }
        }
    }
}

@Composable
private fun SavesScreen(a: LauncherActivity) {
    val preferences = remember { GamePreferences.get(a) }
    var withSettings by remember { mutableStateOf(preferences.getBoolean(GamePreferences.SAVES_INCLUDE_SETTINGS, false)) }
    val available = !a.busy && a.activeSet != null
    ScreenFrame(stringResource(R.string.saves_title), onBack = { a.go(LauncherActivity.Screen.Data) }, backEnabled = !a.busy) {
        Hint(stringResource(R.string.saves_hint), Modifier.widthIn(max = 640.dp))
        Panel(Modifier.widthIn(max = 640.dp)) {
            SwitchRow(stringResource(R.string.saves_include_settings), withSettings, { on ->
                withSettings = on
                preferences.edit().putBoolean(GamePreferences.SAVES_INCLUDE_SETTINGS, on).apply()
            })
        }
        Row(Modifier.widthIn(max = 640.dp), horizontalArrangement = Arrangement.spacedBy(12.dp)) {
            MenuButton(stringResource(R.string.import_saves), { a.launchSaveImport() }, Modifier.weight(1f), enabled = available)
            MenuButton(stringResource(R.string.export_saves), { a.launchSaveExport() }, Modifier.weight(1f), enabled = available)
        }
        Column(Modifier.widthIn(max = 640.dp), verticalArrangement = Arrangement.spacedBy(6.dp)) {
            OperationStatus(a)
        }
    }
}

@Composable
private fun LauncherSettingsScreen(a: LauncherActivity) {
    ScreenFrame(stringResource(R.string.launcher_settings_title), onBack = { a.go(LauncherActivity.Screen.Data) }, backEnabled = !a.busy) {
        Hint(stringResource(R.string.launcher_settings_hint), Modifier.widthIn(max = 640.dp))
        Row(Modifier.widthIn(max = 640.dp), horizontalArrangement = Arrangement.spacedBy(12.dp)) {
            MenuButton(stringResource(R.string.import_launcher), { a.launchLauncherImport() }, Modifier.weight(1f), enabled = !a.busy)
            MenuButton(stringResource(R.string.export_launcher), { a.launchLauncherExport() }, Modifier.weight(1f), enabled = !a.busy)
        }
        Column(Modifier.widthIn(max = 640.dp), verticalArrangement = Arrangement.spacedBy(6.dp)) {
            OperationStatus(a)
        }
    }
}

// ---- Controls ----

/* Split screen help lives on the Gamepads screen, beside the steps it walks
 * through. */
@Composable
private fun ControlsScreen(a: LauncherActivity) {
    ScreenFrame(stringResource(R.string.controls_hub), onBack = { a.go(LauncherActivity.Screen.Main) }) {
        Column(Modifier.widthIn(max = MENU_WIDTH), verticalArrangement = Arrangement.spacedBy(10.dp)) {
            MainMenuButton(stringResource(R.string.controls_touch)) { a.go(LauncherActivity.Screen.Touch) }
            MainMenuButton(stringResource(R.string.controls_gamepad)) { a.go(LauncherActivity.Screen.Gamepads) }
        }
    }
}

/* The on-screen controls: a live preview on the left -- the overlay itself,
 * at the size it will have in the game -- and what can be set about them on
 * the right, each setting written the moment it changes. */
@Composable
private fun TouchScreen(a: LauncherActivity) {
    val preferences = remember { GamePreferences.get(a) }
    fun refresh() = a.touchPreview?.refreshSettings()
    ScreenFrame(stringResource(R.string.touch_settings), onBack = { a.go(LauncherActivity.Screen.Controls) }, scroll = false) {
        TwoPanes(
            secondWidth = 300.dp,
            first = { modifier, narrow ->
                Column(if (narrow) modifier.height(240.dp) else modifier, verticalArrangement = Arrangement.spacedBy(10.dp)) {
                    AndroidView(
                        factory = { context ->
                            FrameLayout(context).also { host ->
                                val overlay = TouchControlsOverlay(a, true)
                                a.touchPreview = overlay
                                TouchPreviewFrame.attach(a, host, overlay)
                            }
                        },
                        onRelease = { host ->
                            val overlay = host.getChildAt(0) as? TouchControlsOverlay
                            overlay?.releaseAll()
                            if (a.touchPreview === overlay) a.touchPreview = null
                        },
                        modifier = Modifier
                            .fillMaxWidth()
                            .weight(1f)
                            .clip(RoundedCornerShape(20.dp))
                            .background(GameColors.Deep)
                            .border(1.dp, GameColors.PillEdge.copy(alpha = 0.5f), RoundedCornerShape(20.dp)),
                    )
                    Row(horizontalArrangement = Arrangement.spacedBy(10.dp)) {
                        MenuButton(stringResource(R.string.edit_touch_layout), { a.openEditor() }, Modifier.weight(1f), height = 48.dp)
                        MenuButton(stringResource(R.string.touch_keys_button), { a.go(LauncherActivity.Screen.TouchKeys) }, Modifier.weight(1f), height = 48.dp)
                    }
                }
            },
            second = { modifier, narrow ->
                Column(
                    if (narrow) modifier else modifier.verticalScroll(rememberScrollState()),
                    verticalArrangement = Arrangement.spacedBy(10.dp),
                ) {
                    BooleanSetting(preferences, GamePreferences.TOUCH_ENABLED, true,
                        stringResource(R.string.pref_touch_enabled), null) { refresh() }
                    Label(stringResource(R.string.touch_layout_label))
                    val layouts = arrayOf(GamePreferences.TOUCH_LAYOUT_STANDARD, GamePreferences.TOUCH_LAYOUT_MIRRORED)
                    var layout by remember {
                        mutableIntStateOf(GamePreferences.indexOf(layouts,
                            preferences.getString(GamePreferences.TOUCH_LAYOUT, GamePreferences.TOUCH_LAYOUT_STANDARD)))
                    }
                    DropdownPill(
                        listOf(stringResource(R.string.layout_right_handed), stringResource(R.string.layout_left_handed)),
                        layout, { index ->
                            layout = index
                            GamePreferences.setTouchLayout(preferences, layouts[index])
                            refresh()
                        }, Modifier.fillMaxWidth())
                    Label(stringResource(R.string.touch_pointer_label))
                    val pointers = arrayOf(GamePreferences.TOUCH_POINTER_TAP, GamePreferences.TOUCH_POINTER_TOUCHPAD)
                    var pointer by remember {
                        mutableIntStateOf(GamePreferences.indexOf(pointers,
                            preferences.getString(GamePreferences.TOUCH_POINTER, GamePreferences.TOUCH_POINTER_TAP)))
                    }
                    DropdownPill(
                        listOf(stringResource(R.string.touch_pointer_tap), stringResource(R.string.touch_pointer_touchpad)),
                        pointer, { index ->
                            pointer = index
                            preferences.edit().putString(GamePreferences.TOUCH_POINTER, pointers[index]).apply()
                        }, Modifier.fillMaxWidth())
                    Hint(stringResource(R.string.touch_pointer_note))
                    IntSetting(preferences, GamePreferences.TOUCH_OPACITY, 65, 20..100, stringResource(R.string.opacity_label), { "$it%" }) { refresh() }
                    IntSetting(preferences, GamePreferences.TOUCH_SIZE, 100, 70..115, stringResource(R.string.size_label), { "$it%" }) { refresh() }
                    IntSetting(preferences, GamePreferences.TOUCH_EDGE, 0, 0..32, stringResource(R.string.touch_edge_label), { "$it dp" }) { refresh() }
                    BooleanSetting(preferences, GamePreferences.TOUCH_AUTO_HIDE, false,
                        stringResource(R.string.touch_dim_label), stringResource(R.string.touch_dim_note)) { refresh() }
                    val delayLabel = stringResource(R.string.auto_hide_delay)
                    IntSetting(preferences, GamePreferences.TOUCH_HIDE_SECONDS, 4, 1..30, "", { "" }, { delayLabel.format(it) }) { refresh() }
                    BooleanSetting(preferences, GamePreferences.TOUCH_VIBRATION, false, stringResource(R.string.pref_phone_vibration), null) { refresh() }
                }
            },
        )
    }
}

/* Two panes side by side -- the second `secondWidth` wide, or as wide as the
 * first -- or, on a narrow window, one above the other in a column that
 * scrolls.  Each pane is handed the modifier that sizes it and whether the
 * window is narrow. */
@Composable
private fun TwoPanes(
    secondWidth: androidx.compose.ui.unit.Dp? = null,
    scrollNarrow: Boolean = true,
    first: @Composable (Modifier, Boolean) -> Unit,
    second: @Composable (Modifier, Boolean) -> Unit,
) {
    BoxWithConstraints(Modifier.fillMaxWidth()) {
        val bounded = constraints.hasBoundedHeight
        if (maxWidth < NARROW) {
            Column(
                if (scrollNarrow && bounded) Modifier.fillMaxSize().verticalScroll(rememberScrollState()) else Modifier.fillMaxWidth(),
                verticalArrangement = Arrangement.spacedBy(16.dp),
            ) {
                first(Modifier.fillMaxWidth(), true)
                second(Modifier.fillMaxWidth(), true)
            }
        } else {
            Row(
                if (bounded) Modifier.fillMaxSize() else Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(20.dp),
            ) {
                val fill = if (bounded) Modifier.fillMaxHeight() else Modifier
                first(Modifier.weight(1f).then(fill), false)
                second((if (secondWidth != null) Modifier.width(secondWidth) else Modifier.weight(1f)).then(fill), false)
            }
        }
    }
}

@Composable
private fun BooleanSetting(
    preferences: android.content.SharedPreferences,
    key: String,
    default: Boolean,
    text: String,
    note: String?,
    changed: () -> Unit,
) {
    var on by remember { mutableStateOf(preferences.getBoolean(key, default)) }
    SwitchRow(text, on, { value ->
        on = value
        preferences.edit().putBoolean(key, value).apply()
        changed()
    }, note)
}

@Composable
private fun IntSetting(
    preferences: android.content.SharedPreferences,
    key: String,
    default: Int,
    range: IntRange,
    text: String,
    format: (Int) -> String,
    label: ((Int) -> String)? = null,
    changed: () -> Unit,
) {
    var value by remember { mutableIntStateOf(preferences.getInt(key, default).coerceIn(range)) }
    SliderRow(label?.invoke(value) ?: text, value, range, format) { next ->
        if (next != value) {
            value = next
            preferences.edit().putInt(key, next).apply()
            changed()
        }
    }
}

/* The key each on-screen control sends.  The overlay speaks to the game
 * through keys, so these have to match the game's settings -- which the
 * gamepad control set written from Controls -> Gamepads does out of the box. */
@Composable
private fun TouchKeysScreen(a: LauncherActivity) {
    val preferences = remember { GamePreferences.get(a) }
    val actions = remember { GamePreferences.actionLabels(a) }
    val keys = remember { GamePreferences.keyLabels(a).toList() }
    ScreenFrame(stringResource(R.string.touch_keys_title), onBack = { a.go(LauncherActivity.Screen.Touch) }) {
        Hint(stringResource(R.string.touch_keys_hint))
        Panel(Modifier.widthIn(max = 720.dp)) {
            GamePreferences.ACTION_IDS.forEachIndexed { i, action ->
                var key by remember {
                    mutableIntStateOf(GamePreferences.indexOf(GamePreferences.KEY_VALUES,
                        GamePreferences.getTouchKey(preferences, action)))
                }
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Text(actions[i], Modifier.weight(1f), color = GameColors.Silver, style = MaterialTheme.typography.bodyLarge)
                    DropdownPill(keys, key, { index ->
                        key = index
                        preferences.edit().putInt(GamePreferences.touchKey(action), GamePreferences.KEY_VALUES[index]).apply()
                    }, Modifier.width(240.dp))
                }
            }
        }
    }
}

/* Which physical pad is Gamepad 1 and which is Gamepad 2, what their buttons
 * do, and writing the port's controls into the game.  A pad is picked by
 * pressing a button on it rather than from a list, because two pads of the
 * same model share a name and a list could not tell them apart. */
@Composable
private fun GamepadsScreen(a: LauncherActivity) {
    val preferences = remember { GamePreferences.get(a) }
    var edits by remember { mutableIntStateOf(0) }  // Automatic chosen here
    val changes = a.padsChanged + edits  // read, so pads coming and going redraw this
    val pads = remember(changes, a.capturingSlot) { GamepadSlots.resolve(preferences) }
    val keyboard = remember(changes) { a.keyboardControlsInstalled() }
    var vibration by remember { mutableStateOf(preferences.getBoolean(GamePreferences.GAMEPAD_VIBRATION, false)) }
    ScreenFrame(
        stringResource(R.string.gamepads_title),
        onBack = { a.go(LauncherActivity.Screen.Controls) },
        action = { QuietButton(stringResource(R.string.controls_help), { a.go(LauncherActivity.Screen.ControlsHelp) }) },
    ) {
        Hint(stringResource(R.string.gamepads_hint))
        if (keyboard)
            Hint(stringResource(R.string.gamepads_keyboard_note), color = GameColors.Gold)
        val slots = @Composable { modifier: Modifier ->
            for (slot in 0 until GamepadSlots.COUNT) {
                val pinned = GamepadSlots.assigned(preferences, slot).isNotEmpty()
                val pad = pads[slot]
                val status = when {
                    a.capturingSlot == slot -> stringResource(R.string.gamepad_assign_prompt, slot + 1)
                    pinned && pad != null -> pad.name
                    pinned -> stringResource(R.string.gamepad_slot_absent, GamepadSlots.assignedName(preferences, slot))
                    pad != null -> stringResource(R.string.gamepad_slot_auto_now, pad.name)
                    else -> stringResource(R.string.gamepad_slot_auto_none)
                }
                Panel(modifier) {
                    Text(stringResource(R.string.gamepad_slot_label, slot + 1), color = GameColors.Silver,
                        style = MaterialTheme.typography.titleLarge, fontWeight = FontWeight.Bold)
                    Text(status, color = if (a.capturingSlot == slot) GameColors.Gold else GameColors.MenuBlue,
                        minLines = 2, style = MaterialTheme.typography.bodyMedium)
                    Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                        MenuButton(stringResource(R.string.gamepad_assign), { a.capturingSlot = slot }, Modifier.weight(1f), height = 44.dp)
                        QuietButton(stringResource(R.string.gamepad_use_auto), {
                            a.capturingSlot = -1
                            GamepadSlots.useAutomatic(preferences, slot)
                            edits++
                        }, Modifier.weight(1f))
                    }
                    QuietButton(stringResource(R.string.gamepad_buttons), { a.openGamepadButtons(slot) }, Modifier.fillMaxWidth())
                }
            }
        }
        BoxWithConstraints(Modifier.fillMaxWidth()) {
            if (maxWidth < NARROW)
                Column(verticalArrangement = Arrangement.spacedBy(12.dp)) { slots(Modifier.fillMaxWidth()) }
            else
                Row(horizontalArrangement = Arrangement.spacedBy(16.dp)) { slots(Modifier.weight(1f)) }
        }
        Panel {
            SwitchRow(stringResource(R.string.gamepad_vibration), vibration, { on ->
                vibration = on
                preferences.edit().putBoolean(GamePreferences.GAMEPAD_VIBRATION, on).apply()
            }, stringResource(R.string.gamepad_vibration_note))
        }
        Row(horizontalArrangement = Arrangement.spacedBy(12.dp)) {
            MenuButton(stringResource(R.string.controls_write_gamepads), { a.confirmWriteControls(ControlProfile.Kind.GAMEPADS) }, Modifier.weight(1f))
            MenuButton(stringResource(R.string.controls_write_keyboard), { a.confirmWriteControls(ControlProfile.Kind.KEYBOARD) }, Modifier.weight(1f))
        }
    }
}

/* What each button of one pad does.  Stored as it is chosen, like the touch
 * keys; NFS3Activity hands it all to the native side when the game starts. */
@Composable
private fun GamepadButtonsScreen(a: LauncherActivity) {
    val slot = a.buttonsSlot
    val preferences = remember { GamePreferences.get(a) }
    val buttons = remember { GamepadButtons.buttonLabels(a) }
    val actions = remember { GamepadButtons.actionLabels(a).toList() }
    var generation by remember { mutableIntStateOf(0) }  // Defaults redraws every row
    ScreenFrame(
        stringResource(R.string.gamepad_buttons_title, slot + 1),
        onBack = { a.go(LauncherActivity.Screen.Gamepads) },
        action = {
            QuietButton(stringResource(R.string.gamepad_buttons_reset), {
                GamepadButtons.reset(preferences, slot)
                generation++
            })
        },
    ) {
        Hint(stringResource(R.string.gamepad_buttons_hint))
        key(generation) {
            Panel(Modifier.widthIn(max = 720.dp)) {
                GamepadButtons.BUTTON_IDS.indices.forEach { button ->
                    var action by remember {
                        mutableIntStateOf(GamePreferences.indexOf(GamepadButtons.ACTION_IDS,
                            GamepadButtons.action(preferences, slot, button)))
                    }
                    Row(verticalAlignment = Alignment.CenterVertically) {
                        Text(buttons[button], Modifier.weight(1f), color = GameColors.Silver, style = MaterialTheme.typography.bodyLarge)
                        DropdownPill(actions, action, { index ->
                            action = index
                            GamepadButtons.setAction(preferences, slot, button, GamepadButtons.ACTION_IDS[index])
                        }, Modifier.width(240.dp))
                    }
                }
            }
        }
    }
}

/** Order of the split-screen help page: one resource per step, like the FAQ. */
private val CONTROLS_HELP_SECTIONS = intArrayOf(
    R.string.help_split_setup, R.string.help_split_write, R.string.help_split_race,
    R.string.help_split_driving, R.string.help_split_notes,
)

/** Order of the help page.  One resource per question so that editing one
 *  answer does not invalidate a whole translated page. */
private val FAQ_SECTIONS = intArrayOf(
    R.string.faq_layout_modes, R.string.faq_mouse, R.string.faq_size,
    R.string.faq_edges, R.string.faq_hiding,
    R.string.faq_vibration, R.string.faq_mapping,
    R.string.faq_lights, R.string.faq_screen_data,
)

@Composable
private fun ControlsHelpScreen(a: LauncherActivity) {
    val text = remember { helpText(a, CONTROLS_HELP_SECTIONS) }
    ScreenFrame(stringResource(R.string.controls_help_title), onBack = { a.go(LauncherActivity.Screen.Gamepads) }) {
        Panel { Text(text, color = GameColors.Silver, style = MaterialTheme.typography.bodyLarge, lineHeight = 24.sp) }
    }
}

/* The overlay buttons carry no captions, so help text cannot name them.  Each
 * resource string carries the matching symbol instead, and it is shown as the
 * very glyph the overlay draws.  Keeping the symbol in the resource means
 * translators see readable text and cannot lose the mark. */
private val CONTROL_ICONS = mapOf(
    '✓' to R.drawable.ic_faq_confirm,
    '←' to R.drawable.ic_faq_back,
    '↻' to R.drawable.ic_faq_reset,
    '☼' to R.drawable.ic_faq_lights,
)

@Composable
private fun FaqScreen(a: LauncherActivity) {
    val text = remember { withControlIcons(helpText(a, FAQ_SECTIONS)) }
    val icons = CONTROL_ICONS.map { (marker, drawable) ->
        marker.toString() to InlineTextContent(Placeholder(1.4.em, 1.4.em, PlaceholderVerticalAlign.Center)) {
            Icon(painterResource(drawable), contentDescription = null, tint = GameColors.Gold, modifier = Modifier.fillMaxSize())
        }
    }.toMap()
    ScreenFrame(stringResource(R.string.faq_title), onBack = { a.go(LauncherActivity.Screen.Main) }) {
        Panel {
            Text(text, color = GameColors.Silver, style = MaterialTheme.typography.bodyLarge,
                lineHeight = 24.sp, inlineContent = icons)
        }
    }
}

/* The sections one after another with a blank line between, their <b>
 * markup kept as bold. */
private fun helpText(context: Context, sections: IntArray): AnnotatedString = buildAnnotatedString {
    sections.forEachIndexed { i, id ->
        if (i > 0) append("\n\n")
        appendSpanned(context.getText(id))
    }
}

private fun AnnotatedString.Builder.appendSpanned(text: CharSequence) {
    val start = length
    append(text.toString())
    if (text is Spanned) {
        for (span in text.getSpans(0, text.length, StyleSpan::class.java)) {
            val style = when (span.style) {
                Typeface.BOLD -> SpanStyle(fontWeight = FontWeight.Bold, color = GameColors.Gold)
                Typeface.ITALIC -> SpanStyle(fontStyle = FontStyle.Italic)
                Typeface.BOLD_ITALIC -> SpanStyle(fontWeight = FontWeight.Bold, fontStyle = FontStyle.Italic)
                else -> null
            } ?: continue
            addStyle(style, start + text.getSpanStart(span), start + text.getSpanEnd(span))
        }
    }
}

private fun withControlIcons(text: AnnotatedString): AnnotatedString = buildAnnotatedString {
    var from = 0
    text.forEachIndexed { i, c ->
        if (c in CONTROL_ICONS) {
            append(text.subSequence(from, i))
            appendInlineContent(c.toString(), c.toString())
            from = i + 1
        }
    }
    append(text.subSequence(from, text.length))
}

// ---- Display ----

@Composable
private fun DisplayScreen(a: LauncherActivity) {
    val preferences = remember { GamePreferences.get(a) }
    val orientations = arrayOf(GamePreferences.ORIENTATION_AUTO, GamePreferences.ORIENTATION_LANDSCAPE,
        GamePreferences.ORIENTATION_LANDSCAPE_REVERSE)
    var orientation by remember {
        mutableIntStateOf(GamePreferences.indexOf(orientations,
            preferences.getString(GamePreferences.ORIENTATION, GamePreferences.ORIENTATION_LANDSCAPE)).coerceAtLeast(1))
    }
    var fps by remember { mutableIntStateOf(if (preferences.getInt(GamePreferences.FPS_CAP, 30) >= 60) 1 else 0) }
    ScreenFrame(stringResource(R.string.display_settings), onBack = { a.go(LauncherActivity.Screen.Main) }) {
        TwoPanes(
            scrollNarrow = false,
            first = { modifier, _ ->
                Panel(modifier) {
                    Label(stringResource(R.string.orientation_label))
                    ChoiceList(
                        listOf(stringResource(R.string.orientation_auto), stringResource(R.string.orientation_landscape),
                            stringResource(R.string.orientation_landscape_reverse)),
                        orientation,
                    ) { index ->
                        orientation = index
                        // Written the moment it is chosen; the game picks it up when it next starts.
                        preferences.edit().putString(GamePreferences.ORIENTATION, orientations[index]).apply()
                    }
                }
            },
            second = { modifier, _ ->
                Panel(modifier) {
                    Label(stringResource(R.string.fps_cap_label))
                    /* Offered rather than typed: only divisors of the refresh
                     * rate are reachable, and a free number would quietly round
                     * to one of them. */
                    ChoiceList(listOf(stringResource(R.string.fps_30), stringResource(R.string.fps_60)), fps) { index ->
                        fps = index
                        preferences.edit().putInt(GamePreferences.FPS_CAP, if (index == 1) 60 else 30).apply()
                    }
                    MenuButton(stringResource(R.string.screen_adjustment), { a.go(LauncherActivity.Screen.Adjustment) }, Modifier.fillMaxWidth())
                }
            },
        )
    }
}

/* Gamma, brightness and contrast over two sample frames -- one daylight, one
 * night -- because the three pull the ends of the range in opposite
 * directions: what rescues a night track washes out a daylit one.  The
 * samples run through the same curve the game's final blit uses
 * (ScreenAdjustment mirrors the shader), so what moves here is what a race
 * will look like.  Reset goes back to the picture as the game drew it. */
@Composable
private fun AdjustmentScreen(a: LauncherActivity) {
    val preferences = remember { GamePreferences.get(a) }
    var gamma by remember { mutableIntStateOf(ScreenAdjustment.gammaPercent(preferences)) }
    var brightness by remember { mutableIntStateOf(ScreenAdjustment.brightnessPercent(preferences)) }
    var contrast by remember { mutableIntStateOf(ScreenAdjustment.contrastPercent(preferences)) }
    val sources = remember {
        arrayOf(
            BitmapFactory.decodeResource(a.resources, R.drawable.adjust_sample_bright),
            BitmapFactory.decodeResource(a.resources, R.drawable.adjust_sample_dark),
        )
    }
    val shown = remember { sources.map { it.copy(Bitmap.Config.ARGB_8888, true) } }
    /* One buffer for both, reused on every step: a preview that allocated a
     * megabyte per frame would stutter exactly while being judged. */
    val scratch = remember { IntArray(sources.maxOf { it.width * it.height }) }
    var drawn by remember { mutableIntStateOf(0) }
    LaunchedEffect(gamma, brightness, contrast) {
        val curve = ScreenAdjustment.curve(gamma, brightness, contrast)
        for (i in sources.indices)
            ScreenAdjustment.apply(sources[i], shown[i], curve, scratch)
        drawn++
    }
    fun save() = preferences.edit()
        .putInt(GamePreferences.GAMMA, gamma)
        .putInt(GamePreferences.BRIGHTNESS, brightness)
        .putInt(GamePreferences.CONTRAST, contrast)
        .apply()
    val pager = rememberPagerState { 2 }
    val scope = rememberCoroutineScope()
    val captions = listOf(stringResource(R.string.sample_bright), stringResource(R.string.sample_dark))
    /* Three sliders, a hint and two pictures on one screen: drawn a size
     * smaller than the rest of the launcher so that all of it fits at once,
     * with Reset up beside the title. */
    val system = LocalDensity.current
    CompositionLocalProvider(LocalDensity provides Density(system.density * 0.85f, system.fontScale)) {
        ScreenFrame(
            stringResource(R.string.screen_adjustment),
            onBack = { a.go(LauncherActivity.Screen.Display) },
            scroll = false,
            action = {
                QuietButton(stringResource(R.string.adjust_reset), {
                    gamma = ScreenAdjustment.NEUTRAL
                    brightness = ScreenAdjustment.NEUTRAL
                    contrast = ScreenAdjustment.NEUTRAL
                    save()
                })
            },
        ) {
            TwoPanes(
                first = { modifier, narrow ->
                    Column(
                        if (narrow) modifier else modifier.verticalScroll(rememberScrollState()),
                        verticalArrangement = Arrangement.spacedBy(6.dp),
                    ) {
                        Hint(stringResource(R.string.adjust_hint))
                        SliderRow(stringResource(R.string.gamma_label), gamma, ScreenAdjustment.GAMMA_MIN..ScreenAdjustment.GAMMA_MAX, { "$it%" }) {
                            gamma = it; save()
                        }
                        SliderRow(stringResource(R.string.brightness_label), brightness, ScreenAdjustment.BRIGHTNESS_MIN..ScreenAdjustment.BRIGHTNESS_MAX, { "$it%" }) {
                            brightness = it; save()
                        }
                        SliderRow(stringResource(R.string.contrast_label), contrast, ScreenAdjustment.CONTRAST_MIN..ScreenAdjustment.CONTRAST_MAX, { "$it%" }) {
                            contrast = it; save()
                        }
                    }
                },
                second = { modifier, narrow ->
                    Column(if (narrow) modifier.height(260.dp) else modifier, horizontalAlignment = Alignment.CenterHorizontally) {
                        Text(captions[pager.currentPage], color = GameColors.Gold, fontWeight = FontWeight.Bold,
                            style = MaterialTheme.typography.titleMedium)
                        Spacer(Modifier.height(6.dp))
                        HorizontalPager(pager, Modifier.fillMaxWidth().weight(1f)) { page ->
                            key(drawn) {
                                Image(
                                    shown[page].asImageBitmap(),
                                    contentDescription = captions[page],
                                    modifier = Modifier.fillMaxSize().clip(RoundedCornerShape(12.dp)),
                                    contentScale = ContentScale.Fit,
                                )
                            }
                        }
                        Row(Modifier.padding(top = 8.dp), horizontalArrangement = Arrangement.spacedBy(12.dp)) {
                            for (page in 0 until 2) {
                                Box(
                                    Modifier
                                        .size(12.dp)
                                        .clip(CircleShape)
                                        .background(if (pager.currentPage == page) GameColors.Gold else GameColors.PillEdge)
                                        .clickable { scope.launch { pager.animateScrollToPage(page) } }
                                )
                            }
                        }
                    }
                },
            )
        }
    }
}
