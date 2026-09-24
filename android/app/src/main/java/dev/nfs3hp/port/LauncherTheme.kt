package dev.nfs3hp.port

import androidx.compose.foundation.BorderStroke
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.BoxWithConstraints
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.ColumnScope
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.RowScope
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.WindowInsets
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.heightIn
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.displayCutout
import androidx.compose.foundation.layout.navigationBars
import androidx.compose.foundation.layout.union
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.layout.windowInsetsPadding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.selection.selectable
import androidx.compose.foundation.selection.toggleable
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.ArrowBack
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.FilledTonalButton
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.RadioButton
import androidx.compose.material3.Slider
import androidx.compose.material3.SliderDefaults
import androidx.compose.material3.Switch
import androidx.compose.material3.SwitchDefaults
import androidx.compose.material3.Text
import androidx.compose.material3.darkColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.runtime.CompositionLocalProvider
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.ui.semantics.Role
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.Density
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp

/* The launcher in the colours of the game's own menus -- the royal blue of the
 * title and car screens, the indigo pills its buttons are, the gold of its
 * headings -- sampled from the game's screens and then calmed for a screen that
 * is read rather than raced past: the blue deeper and less loud, the pills
 * darker so their lettering stands out, the text near white. */
object GameColors {
    val Night = Color(0xFF06081A)
    val Deep = Color(0xFF0D1233)
    val Royal = Color(0xFF16205C)
    val Pill = Color(0xFF151B47)
    val PillEdge = Color(0xFF3A479C)
    val Gold = Color(0xFFE9CF4E)
    val GoldDim = Color(0xFFC8A040)
    val MenuBlue = Color(0xFFDCE6FF)
    val Silver = Color(0xFFEEF1F8)
    val Muted = Color(0xFF9FAACB)
    val PanelFill = Color(0xD910163F)
}

private val GameScheme = darkColorScheme(
    primary = GameColors.Gold,
    onPrimary = Color(0xFF1B1500),
    primaryContainer = Color(0xFF4A3F0C),
    onPrimaryContainer = Color(0xFFFFF0A8),
    secondary = GameColors.MenuBlue,
    onSecondary = Color(0xFF0A1440),
    secondaryContainer = GameColors.Pill,
    onSecondaryContainer = Color(0xFFD6DEFF),
    tertiary = GameColors.GoldDim,
    onTertiary = Color(0xFF1B1500),
    background = GameColors.Night,
    onBackground = GameColors.Silver,
    surface = Color(0xFF10163F),
    onSurface = GameColors.Silver,
    surfaceVariant = Color(0xFF1A2152),
    onSurfaceVariant = GameColors.Muted,
    surfaceContainerLowest = GameColors.Night,
    surfaceContainerLow = GameColors.Deep,
    surfaceContainer = Color(0xFF121A48),
    surfaceContainerHigh = Color(0xFF182052),
    surfaceContainerHighest = Color(0xFF1F285F),
    outline = GameColors.PillEdge,
    outlineVariant = Color(0xFF26306E),
)

/* The launcher is laid out for a landscape phone at least this tall.  A screen
 * shorter than that in dp -- a small phone, or a large "display size" setting
 * -- gets a proportionally smaller density rather than cut-off controls, and a
 * large system font is held to a scale the layouts still hold. */
/** What the launcher keeps clear of: the cutout and the navigation bar.  Not
 *  the status bar -- the launcher is full screen, and the first frame still
 *  reports the bar before it is gone, which moved every screen up a moment
 *  after it appeared. */
val LauncherInsets: WindowInsets
    @Composable get() = WindowInsets.displayCutout.union(WindowInsets.navigationBars)

private const val DESIGN_SHORT_SIDE_DP = 400f
private const val MAX_FONT_SCALE = 1.15f

@Composable
fun GameTheme(content: @Composable () -> Unit) {
    MaterialTheme(colorScheme = GameScheme) {
        BoxWithConstraints(Modifier.fillMaxSize()) {
            val system = LocalDensity.current
            val shortSide = minOf(maxWidth.value, maxHeight.value)
            val scale = if (shortSide > 0f && shortSide < DESIGN_SHORT_SIDE_DP) shortSide / DESIGN_SHORT_SIDE_DP else 1f
            val density = Density(system.density * scale, minOf(system.fontScale, MAX_FONT_SCALE))
            CompositionLocalProvider(LocalDensity provides density) {
                Box(
                    Modifier
                        .fillMaxSize()
                        .background(
                            Brush.verticalGradient(
                                0f to GameColors.Royal,
                                0.5f to GameColors.Deep,
                                1f to GameColors.Night,
                            )
                        )
                ) {
                    content()
                }
            }
        }
    }
}

/** The frame every screen but the first sits in: back, a gold title in heavy
 *  capitals set a little close, as the game letters its screens, an optional
 *  action on the right, then the screen's own content, which scrolls when it
 *  has to. */
@Composable
fun ScreenFrame(
    title: String,
    onBack: () -> Unit,
    backEnabled: Boolean = true,
    action: (@Composable RowScope.() -> Unit)? = null,
    scroll: Boolean = true,
    content: @Composable ColumnScope.() -> Unit,
) {
    Column(
        Modifier
            .fillMaxSize()
            .windowInsetsPadding(LauncherInsets)
            .padding(horizontal = 20.dp, vertical = 12.dp)
    ) {
        Row(verticalAlignment = Alignment.CenterVertically) {
            IconButton(onClick = onBack, enabled = backEnabled) {
                Icon(Icons.AutoMirrored.Filled.ArrowBack, contentDescription = null, tint = GameColors.MenuBlue)
            }
            Text(
                title.uppercase(),
                modifier = Modifier.weight(1f).padding(start = 4.dp),
                style = MaterialTheme.typography.headlineSmall,
                color = GameColors.Gold,
                fontWeight = FontWeight.Black,
                letterSpacing = (-0.5).sp,
                maxLines = 1,
                overflow = TextOverflow.Ellipsis,
            )
            if (action != null) action()
        }
        Spacer(Modifier.size(8.dp))
        val body = Modifier.fillMaxWidth().weight(1f)
        Column(
            if (scroll) body.verticalScroll(rememberScrollState()) else body,
            verticalArrangement = Arrangement.spacedBy(12.dp),
        ) {
            content()
        }
    }
}

private val PillShape = RoundedCornerShape(50)

/** A button as the game's menus draw theirs: an indigo pill with a lighter
 *  rim and pale blue lettering. */
@Composable
fun MenuButton(
    text: String,
    onClick: () -> Unit,
    modifier: Modifier = Modifier,
    enabled: Boolean = true,
    height: Dp = 52.dp,
) {
    FilledTonalButton(
        onClick = onClick,
        enabled = enabled,
        modifier = modifier.heightIn(min = height),
        shape = PillShape,
        border = BorderStroke(1.dp, if (enabled) GameColors.PillEdge else GameColors.PillEdge.copy(alpha = 0.3f)),
        colors = ButtonDefaults.filledTonalButtonColors(
            containerColor = GameColors.Pill,
            contentColor = GameColors.MenuBlue,
        ),
    ) {
        Text(text, fontWeight = FontWeight.SemiBold, maxLines = 1, overflow = TextOverflow.Ellipsis)
    }
}

/** The one gold button: Play, and whatever a screen is mainly for. */
@Composable
fun GoldButton(
    text: String,
    onClick: () -> Unit,
    modifier: Modifier = Modifier,
    enabled: Boolean = true,
    height: Dp = 56.dp,
) {
    Button(
        onClick = onClick,
        enabled = enabled,
        modifier = modifier.heightIn(min = height),
        shape = RoundedCornerShape(16.dp),
    ) {
        Text(
            text.uppercase(),
            fontWeight = FontWeight.Black,
            fontSize = 20.sp,
            letterSpacing = 1.sp,
            maxLines = 1,
            overflow = TextOverflow.Ellipsis,
        )
    }
}

/** A quieter button beside a pill: a rim and nothing inside. */
@Composable
fun QuietButton(text: String, onClick: () -> Unit, modifier: Modifier = Modifier, enabled: Boolean = true) {
    OutlinedButton(
        onClick = onClick,
        enabled = enabled,
        modifier = modifier.heightIn(min = 48.dp),
        shape = PillShape,
        border = BorderStroke(1.dp, GameColors.PillEdge),
    ) {
        Text(text, color = GameColors.MenuBlue, maxLines = 1, overflow = TextOverflow.Ellipsis)
    }
}

/** A panel on the blue, as the game frames its lists. */
@Composable
fun Panel(modifier: Modifier = Modifier, content: @Composable ColumnScope.() -> Unit) {
    Card(
        modifier = modifier.fillMaxWidth(),
        shape = RoundedCornerShape(20.dp),
        colors = CardDefaults.cardColors(containerColor = GameColors.PanelFill),
        border = BorderStroke(1.dp, GameColors.PillEdge.copy(alpha = 0.5f)),
    ) {
        Column(Modifier.padding(16.dp), verticalArrangement = Arrangement.spacedBy(10.dp), content = content)
    }
}

@Composable
fun Hint(text: String, modifier: Modifier = Modifier, color: Color = GameColors.Muted) {
    Text(text, modifier = modifier, style = MaterialTheme.typography.bodyMedium, color = color)
}

@Composable
fun Label(text: String, modifier: Modifier = Modifier) {
    Text(text, modifier = modifier, style = MaterialTheme.typography.titleMedium, color = GameColors.Silver)
}

/** A setting that is on or off: the whole row is the switch. */
@Composable
fun SwitchRow(text: String, checked: Boolean, onChange: (Boolean) -> Unit, note: String? = null) {
    Column {
        Row(
            Modifier
                .fillMaxWidth()
                .toggleable(value = checked, role = Role.Switch, onValueChange = onChange)
                .padding(vertical = 4.dp),
            verticalAlignment = Alignment.CenterVertically,
        ) {
            Text(text, Modifier.weight(1f), style = MaterialTheme.typography.bodyLarge, color = GameColors.Silver)
            Switch(
                checked = checked,
                onCheckedChange = null,
                colors = SwitchDefaults.colors(
                    checkedThumbColor = GameColors.Night,
                    checkedTrackColor = GameColors.Gold,
                    uncheckedTrackColor = GameColors.Deep,
                    uncheckedBorderColor = GameColors.PillEdge,
                    uncheckedThumbColor = GameColors.MenuBlue,
                ),
            )
        }
        if (note != null) Hint(note)
    }
}

/** A number chosen on a slider, written as it moves; the value in gold. */
@Composable
fun SliderRow(
    text: String,
    value: Int,
    range: IntRange,
    format: (Int) -> String,
    onChange: (Int) -> Unit,
) {
    Column {
        Row(verticalAlignment = Alignment.CenterVertically) {
            Text(text, Modifier.weight(1f), style = MaterialTheme.typography.bodyLarge, color = GameColors.Silver)
            Text(format(value), color = GameColors.Gold, fontWeight = FontWeight.Bold)
        }
        Slider(
            value = value.toFloat(),
            onValueChange = { onChange(it.toInt().coerceIn(range)) },
            valueRange = range.first.toFloat()..range.last.toFloat(),
            steps = 0,
            colors = SliderDefaults.colors(
                thumbColor = GameColors.Gold,
                activeTrackColor = GameColors.Gold,
                inactiveTrackColor = GameColors.Pill,
            ),
        )
    }
}

/** One of a few choices, each a row with its round mark. */
@Composable
fun ChoiceList(options: List<String>, selected: Int, onSelect: (Int) -> Unit) {
    Column {
        options.forEachIndexed { index, option ->
            Row(
                Modifier
                    .fillMaxWidth()
                    .selectable(selected = index == selected, role = Role.RadioButton, onClick = { onSelect(index) })
                    .padding(vertical = 2.dp),
                verticalAlignment = Alignment.CenterVertically,
            ) {
                RadioButton(selected = index == selected, onClick = null)
                Spacer(Modifier.width(8.dp))
                Text(option, style = MaterialTheme.typography.bodyLarge, color = GameColors.Silver)
            }
        }
    }
}

/** One of many choices, behind a pill that opens the list. */
@Composable
fun DropdownPill(options: List<String>, selected: Int, onSelect: (Int) -> Unit, modifier: Modifier = Modifier) {
    var open by remember { mutableStateOf(false) }
    Box(modifier) {
        MenuButton(
            text = options.getOrElse(selected) { "" } + "  ▾",
            onClick = { open = true },
            modifier = Modifier.fillMaxWidth(),
            height = 44.dp,
        )
        DropdownMenu(expanded = open, onDismissRequest = { open = false }) {
            options.forEachIndexed { index, option ->
                DropdownMenuItem(
                    text = { Text(option, color = if (index == selected) GameColors.Gold else GameColors.Silver) },
                    onClick = {
                        open = false
                        onSelect(index)
                    },
                )
            }
        }
    }
}
