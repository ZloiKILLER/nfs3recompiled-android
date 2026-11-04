#include "native_frontend.h"
#include "dynamic_fullscreen.h"
#include <cstring>
#include <iterator>
#include <utility>

/* The options screens, decompiled: where the player sets the sound, the keys,
 * the controllers and the graphics, before a race or, most of them, from a
 * race's pause screens.
 *
 *   options    MENUS/OPTIONS.MNU, the screen the others are reached from;
 *   audio      AUDIO.MNU (psaudio in a race): the five volumes, sliders;
 *   audfeed    the audio feed: three keys, for the time, the position and
 *              the speed in a race;
 *   funkeys    the function keys, and hardkeys, the fixed ones: lines of
 *              text, placed in rows;
 *   control1   the controllers: a reset to their defaults, and the way to
 *              force feedback and the dead zones;
 *   deadzone   (psdeadz) a controller's axes, each a slider with two ends;
 *   force      (psforce) force feedback, six sliders;
 *   graphics   the screen mode and the car's shading; advgrph, the rest.
 *
 * Each is a row of the handler table at 0x555c48 -- as the screen opens, as it
 * closes, on every pass of the menu loop -- and an item can be handed a key
 * handler (+0x30), a drawing one (+0x2c), a click (+0x64) or, a slider, a
 * function its moves go to (+0x78).  Those are here too, and the few functions
 * only these screens call.
 *
 * The settings stay in guest memory where the rest of the game reads them.
 * A screen keeps a copy of them as it opens, and its Cancel button puts the
 * copy back as it closes.  The widgets -- buttons, lists, sliders -- stay the
 * generated code, called by address. */
namespace nfs3hp
{

namespace
{

using fe::Guest;
using fe::GuestString;
using fe::Item;

/* ---------------------------------------------------------------------------
 * What the screens share
 * ------------------------------------------------------------------------- */

constexpr x86::reg32 kInRace = 0x559234;          // the menus are a race's pause screens
// 2 in a race over a network (sub_44be60), where some screens are closed.
constexpr x86::reg32 kSplitScreen = 0x6fd3b0;
// The rows a screen's items stand in, by their ypos: x and y, int16 every four
// bytes (see native_frontend_main.cpp).
constexpr x86::reg32 kRowX = 0x559c6c;
constexpr x86::reg32 kRowY = 0x559c94;

// The menu engine's and the widgets' functions.
constexpr x86::reg32 kCodelinkMatches = 0x4429c0;  // a codelink names `name`
constexpr x86::reg32 kShownNow = 0x44a660;         // an item is shown on this pass
constexpr x86::reg32 kCurrentMenu = 0x44aeb0;
constexpr x86::reg32 kDrawItems = 0x44a4f0;
constexpr x86::reg32 kDialogShown = 0x4438b0;
constexpr x86::reg32 kDialogPass = 0x445f50;
constexpr x86::reg32 kDialogFadedIn = 0x445f00;
constexpr x86::reg32 kDialogClose = 0x445f10;
constexpr x86::reg32 kButtonKey = 0x461270;        // a button's own key handler
constexpr x86::reg32 kCascadeKey = 0x4624a0;       // a list's that pops up in place
constexpr x86::reg32 kListKey = 0x463ff0;          // a list's that is always open
// A slider (widget 0x46c...): its value, 0 to 1, by the index of its end (two
// ends for a range); the function its moves go to; how it draws.
constexpr x86::reg32 kSliderSetValue = 0x46c950;   // eax the item; the value and the end on the stack
constexpr x86::reg32 kSliderValue = 0x46c9b0;      // eax the item, edx the end; the value on the x87
constexpr x86::reg32 kSliderSetOnMove = 0x46cac0;  // edx the function, 0 for none (+0x78)
constexpr x86::reg32 kSliderDraw = 0x46cae0;       // eax the item, edx 1 when selected
constexpr x86::reg32 kSliderSetColour = 0x46cca0;  // its bar's brightness, edx; ebx 1
constexpr x86::reg32 kSliderOnMove = 0x78;
constexpr x86::reg32 kOnDraw = 0x2c;
constexpr x86::reg32 kOnKey = 0x30;
constexpr std::uint32_t kButton = 5;               // the widget's type, the item's first dword
constexpr std::uint16_t kKeyEnter = 0x0d;

// The game's.
constexpr x86::reg32 kDimBackground = 0x451530;    // a float on the stack: a race's pause screen behind
constexpr std::uint32_t kFloat0_6 = 0x3f19999a;    // 0.6f
// Copies ebx bytes from eax to edx, the source first (sub_4ea4f0).
constexpr x86::reg32 kCopy = 0x4ea4f0;
// Zeroes edx bytes at eax.
constexpr x86::reg32 kFillZero = 0x4e070c;
constexpr x86::reg32 kSprintf = 0x4df690;

/* A screen's selected item: its index from the screen's first, in the table
 * of items. */
x86::reg32 selectedItem(const Guest& guest, x86::reg32 menu)
{
    const std::int32_t first = std::int16_t(guest.word(menu + fe::menu::kFirstItem));
    const std::int32_t selected = std::int16_t(guest.word(menu + fe::menu::kSelected));
    return fe::item::kTable + fe::item::kSize * x86::reg32(first + selected);
}

/* As a screen closes: whether its selected item is the button `codelink`, the
 * one that left it. */
bool leftBy(Guest& guest, x86::reg32 menu, const GuestString& codelink)
{
    const x86::reg32 item = selectedItem(guest, menu);
    return guest.dword(item) == kButton
           && guest.call(kCodelinkMatches, fe::args({ guest.dword(item + fe::item::kCodelink), codelink.address }))
                  != 0;
}

bool inRace(const Guest& guest)
{
    return guest.dword(kInRace) != 0;
}

bool networkRace(const Guest& guest)
{
    return std::int32_t(guest.dword(kSplitScreen)) > 1;
}

/* Greyed out and hidden, the state and flags bytes as one word. */
void greyAndHide(Guest& guest, Item item)
{
    guest.setWord(item.address() + fe::item::kState,
                  std::uint16_t(guest.word(item.address() + fe::item::kState) | 0x1001));
}

void showAgain(Guest& guest, Item item)
{
    guest.setWord(item.address() + fe::item::kState,
                  std::uint16_t(guest.word(item.address() + fe::item::kState) & 0xeffe));
}

/* `count` dwords from `from` to `to`, as rep movsd copies them. */
void copyDwords(Guest& guest, x86::reg32 to, x86::reg32 from, std::uint32_t count)
{
    for (std::uint32_t i = 0; i < count; ++i)
        guest.setDword(to + 4 * i, guest.dword(from + 4 * i));
}

/* A key for an item, handed on to the widget's own handler: eax the item, edx
 * the screen, ebx the key, as the menu loop hands them. */
x86::reg32 passKey(Guest& guest, x86::reg32 handler, x86::reg32 item, x86::reg32 menu, std::uint16_t key)
{
    return guest.call(handler, fe::args({ item, menu, x86::reg32(std::int32_t(std::int16_t(key))) }));
}

/* A slider's value, as kSliderValue leaves it on the x87. */
x86::Float sliderValue(Guest& guest, x86::reg32 item, x86::reg32 end)
{
    guest.call(kSliderValue, fe::args({ item, end }));
    return guest.fpu().pop();
}

/* A float as the dword pushed for it. */
x86::reg32 floatBits(float value)
{
    x86::reg32 bits;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}

/* The x87's compare and what je reads after it: equal, or unordered. */
bool same(Guest& guest, const x86::Float& a, const x86::Float& b)
{
    guest.fpu().compare(a, b);
    return guest.fpu().status.c3;
}

/* The credits button of a screen, greyed out in a race over a network. */
void greyCreditsInNetworkRace(Guest& guest, x86::reg32 menu, const GuestString& credits)
{
    if (Item item = fe::findItem(guest, menu, credits))
    {
        if (networkRace(guest))
            item.setDisabled(true);
    }
}

/* ---------------------------------------------------------------------------
 * options
 * ------------------------------------------------------------------------- */

constexpr GuestString kAudioButton{ 0x5389ec, "AudioB" };
constexpr GuestString kLoadButton{ 0x5389f4, "LOAD" };
constexpr GuestString kOptionsCredits{ 0x537d8c, "credits" };
// Nonzero when the game runs without sound (sub_4107c0 plays no music then).
constexpr x86::reg32 kNoSound = 0x6fbb54;

/* sub_442aa0, options as it opens: no sound screen without sound, and in a
 * race over a network neither loading nor the credits. */
x86::reg32 onOptionsEnter(Guest& guest, x86::reg32 menu)
{
    if (Item audio = fe::findItem(guest, menu, kAudioButton))
    {
        if (guest.dword(kNoSound) != 0)
            audio.setDisabled(true);
    }
    if (Item load = fe::findItem(guest, menu, kLoadButton))
    {
        if (networkRace(guest))
            greyAndHide(guest, load);
    }
    greyCreditsInNetworkRace(guest, menu, kOptionsCredits);
    return 0;
}

/* ---------------------------------------------------------------------------
 * audfeed: the audio feed's keys
 * ------------------------------------------------------------------------- */

/* Three keys, one of ten each (0 to 9), never two on the same: the time's,
 * the position's and the speed's.  A button and a list for each, B and C. */
constexpr x86::reg32 kFeedTime = 0x6fbc04;
constexpr x86::reg32 kFeedPosition = 0x6fbc08;
constexpr x86::reg32 kFeedSpeed = 0x6fbc0c;
constexpr std::int32_t kFeedKeys = 10;
// What they were as the screen opened, for its Cancel.
constexpr x86::reg32 kFeedTimeKept = 0x5f3914;
constexpr x86::reg32 kFeedPositionKept = 0x5f3910;
constexpr x86::reg32 kFeedSpeedKept = 0x5f3918;

constexpr GuestString kFeedCredits{ 0x537494, "credits" };
constexpr GuestString kFeedTimeButton{ 0x53749c, "AF_BFEEDTIME" };
constexpr GuestString kFeedTimeList{ 0x5374ac, "AF_CFEEDTIME" };
constexpr GuestString kFeedPositionButton{ 0x5374bc, "AF_BPOSITION" };
constexpr GuestString kFeedPositionList{ 0x5374cc, "AF_CPOSITION" };
constexpr GuestString kFeedSpeedButton{ 0x5374dc, "AF_BSPEED" };
constexpr GuestString kFeedSpeedList{ 0x5374e8, "AF_CSPEED" };
constexpr GuestString kFeedCancel{ 0x5374f4, "CANCEL_AUDFEED_OPTIONS" };

// The key handlers the screen hands its items.
constexpr x86::reg32 kFeedTimeButtonKey = 0x437300;
constexpr x86::reg32 kFeedTimeListKey = 0x437350;
constexpr x86::reg32 kFeedPositionButtonKey = 0x437390;
constexpr x86::reg32 kFeedPositionListKey = 0x4373e0;
constexpr x86::reg32 kFeedSpeedButtonKey = 0x437420;
constexpr x86::reg32 kFeedSpeedListKey = 0x437470;
constexpr x86::reg32 kSeparateFeeds = 0x437250;

/* A feed's key moved on, round from 9 to 0, until no other feed has it. */
void moveOff(Guest& guest, x86::reg32 feed, x86::reg32 other1, x86::reg32 other2)
{
    while (guest.dword(feed) == guest.dword(other1) || guest.dword(feed) == guest.dword(other2))
    {
        const std::int32_t next = std::int32_t(guest.dword(feed)) + 1;
        guest.setDword(feed, x86::reg32(next));
        if (next > kFeedKeys - 1)
            guest.setDword(feed, 0);
    }
}

/* sub_437250: the three feeds on keys of their own, the time first. */
x86::reg32 separateFeeds(Guest& guest)
{
    moveOff(guest, kFeedTime, kFeedPosition, kFeedSpeed);
    moveOff(guest, kFeedPosition, kFeedTime, kFeedSpeed);
    moveOff(guest, kFeedSpeed, kFeedTime, kFeedPosition);
    return guest.dword(kFeedSpeed);
}

/* sub_437300, sub_437390, sub_437420: a feed's button.  Enter moves it to the
 * next key, and on until no other feed has that one. */
x86::reg32 onFeedButtonKey(Guest& guest, x86::reg32 item, x86::reg32 menu, std::uint16_t key, x86::reg32 feed,
                           x86::reg32 other1, x86::reg32 other2)
{
    if (key != kKeyEnter)
        return passKey(guest, kButtonKey, item, menu, key);
    x86::reg32 result;
    do
        result = passKey(guest, kButtonKey, item, menu, key);
    while (guest.dword(feed) == guest.dword(other1) || guest.dword(feed) == guest.dword(other2));
    guest.leaveEbx(guest.dword(feed));
    return result;
}

/* sub_437350, sub_4373e0, sub_437470: a feed's list.  A key another feed has
 * is not taken: the feed stays where it was. */
x86::reg32 onFeedListKey(Guest& guest, x86::reg32 item, x86::reg32 menu, std::uint16_t key, x86::reg32 feed,
                         x86::reg32 other1, x86::reg32 other2)
{
    const x86::reg32 was = guest.dword(feed);
    const x86::reg32 result = passKey(guest, kCascadeKey, item, menu, key);
    const x86::reg32 now = guest.dword(feed);
    if (was != now && (now == guest.dword(other1) || now == guest.dword(other2)))
        guest.setDword(feed, was);
    guest.leaveEdx(now);
    return result;
}

/* sub_4374b0, audfeed as it opens: the feeds kept for Cancel, each item its
 * key handler. */
x86::reg32 onFeedEnter(Guest& guest, x86::reg32 menu)
{
    guest.setDword(kFeedTimeKept, guest.dword(kFeedTime));
    guest.setDword(kFeedSpeedKept, guest.dword(kFeedSpeed));
    guest.setDword(kFeedPositionKept, guest.dword(kFeedPosition));
    greyCreditsInNetworkRace(guest, menu, kFeedCredits);
    const struct
    {
        const GuestString& codelink;
        x86::reg32 handler;
    } items[] = {
        { kFeedTimeButton, kFeedTimeButtonKey },         { kFeedTimeList, kFeedTimeListKey },
        { kFeedPositionButton, kFeedPositionButtonKey }, { kFeedPositionList, kFeedPositionListKey },
        { kFeedSpeedButton, kFeedSpeedButtonKey },       { kFeedSpeedList, kFeedSpeedListKey },
    };
    for (const auto& entry : items)
    {
        if (Item item = fe::findItem(guest, menu, entry.codelink))
            guest.setDword(item.address() + kOnKey, entry.handler);
    }
    return 0;
}

/* sub_437590, audfeed as it closes: Cancel puts the feeds back; anything else
 * keeps them, on keys of their own. */
x86::reg32 onFeedExit(Guest& guest, x86::reg32 menu)
{
    if (leftBy(guest, menu, kFeedCancel))
    {
        guest.setDword(kFeedTime, guest.dword(kFeedTimeKept));
        guest.setDword(kFeedSpeed, guest.dword(kFeedSpeedKept));
        guest.setDword(kFeedPosition, guest.dword(kFeedPositionKept));
        return 0;
    }
    guest.call(kSeparateFeeds);
    return 0;
}

/* ---------------------------------------------------------------------------
 * funkeys and hardkeys
 * ------------------------------------------------------------------------- */

constexpr GuestString kKeysCredits{ 0x537494, "credits" };
/* Their lines of text, each placed by a row: the first by the ninth, a little
 * further right, the others by the first eight. */
constexpr GuestString kKeyTexts[] = {
    { 0x53750c, "FUNTXT_1" },  { 0x537518, "FUNTXT_2" }, { 0x537524, "FUNTXT_3" },
    { 0x537530, "FUNTXT_4" },  { 0x53753c, "FUNTXT_56" }, { 0x537548, "FUNTXT_7" },
    { 0x537554, "FUNTXT_8" },  { 0x537560, "FUNTXT_9" }, { 0x53756c, "FUNTXT_10" },
};
constexpr std::uint32_t kFirstKeyTextRow = 8;
constexpr std::uint16_t kFirstKeyTextIndent = 0x9c;
constexpr std::uint16_t kKeyTextIndent = 0x96;
// Set as these screens open (the menu loop's kFadeIn).
constexpr x86::reg32 kFadeIn = 0x559c68;

/* sub_437600 (funkeys) and sub_4377d0 (hardkeys), alike as they open: the
 * lines of text placed in rows. */
x86::reg32 onKeysEnter(Guest& guest, x86::reg32 menu)
{
    guest.setDword(kFadeIn, 1);
    greyCreditsInNetworkRace(guest, menu, kKeysCredits);
    for (std::uint32_t i = 0; i < std::size(kKeyTexts); ++i)
    {
        if (Item text = fe::findItem(guest, menu, kKeyTexts[i]))
        {
            const std::uint32_t row = i == 0 ? kFirstKeyTextRow : i - 1;
            const std::uint16_t indent = i == 0 ? kFirstKeyTextIndent : kKeyTextIndent;
            guest.setWord(text.address() + fe::item::kX, std::uint16_t(guest.word(kRowX + 4 * row) + indent));
            guest.setWord(text.address() + fe::item::kY, guest.word(kRowY + 4 * row));
        }
    }
    return 0;
}

/* ---------------------------------------------------------------------------
 * audio
 * ------------------------------------------------------------------------- */

/* The sound's settings: sixteen dwords from 0x6fbbd0, kept whole as the screen
 * opens.  The volumes are 0 to 127; a slider shows them from 0 to 1. */
constexpr x86::reg32 kSoundSettings = 0x6fbbd0;
constexpr std::uint32_t kSoundSettingsDwords = 16;
constexpr x86::reg32 kSoundOutput = 0x6fbbe4;      // above 2, the race's music in its other form (sub_4107c0)
constexpr x86::reg32 kMenuMusicVolume = 0x6fbbe8;
constexpr x86::reg32 kSoundSetting1ec = 0x6fbbec;  // handed to sub_4103a0 with edx 1
constexpr x86::reg32 kRaceMusicVolume = 0x6fbbf0;
constexpr x86::reg32 kEffectsVolume = 0x6fbbf4;
constexpr x86::reg32 kEngineVolume = 0x6fbbf8;
constexpr x86::reg32 kSpeechVolume = 0x6fbbfc;
// Changing it starts the sound over (sub_40fcf0), with a message first.
constexpr x86::reg32 kSoundSetup = 0x6fbc00;
// A race's own effects and engine volumes, what its pause screen moves.
constexpr x86::reg32 kRaceEffectsVolume = 0x552920;
constexpr x86::reg32 kRaceEngineVolume = 0x552924;
// Byte: the race's volumes go back to the settings as the screen closes.
constexpr x86::reg32 kRaceVolumesToSettings = 0x552909;
// Byte: the race's music is playing.
constexpr x86::reg32 kRaceMusicPlaying = 0x55e890;
constexpr x86::reg32 kRaceMusicTrack = 0x6fd4ac;
// Word: cleared when the sound has no 3D.
constexpr x86::reg32 kSound3d = 0x55636a;

// What the screen keeps as it opens.
constexpr x86::reg32 kSoundSettingsKept = 0x5f3920;
constexpr x86::reg32 kSoundSetupKept = 0x5f3970;
constexpr x86::reg32 kSoundOutputKept = 0x5f3968;
constexpr x86::reg32 kSetting1ecKept = 0x5f3964;
constexpr x86::reg32 kRaceEffectsKept = 0x5f3960;
constexpr x86::reg32 kRaceEngineKept = 0x5f396c;
// Set when the output changed, to apply kSoundSetting1ec as the screen closes.
constexpr x86::reg32 kOutputChanged = 0x554f34;

/* The five sliders, their items found as the screen opens, and for each
 * whether it is held selected: while one is, the music plays at the volume it
 * moves, or an effect does. */
constexpr x86::reg32 kMenuMusicSlider = 0x554f3c;
constexpr x86::reg32 kRaceMusicSlider = 0x554f40;
constexpr x86::reg32 kEffectsSlider = 0x554f44;
constexpr x86::reg32 kEngineSlider = 0x554f48;
constexpr x86::reg32 kSpeechSlider = 0x554f4c;
constexpr x86::reg32 kMenuMusicHeld = 0x554f24;
constexpr x86::reg32 kRaceMusicHeld = 0x554f20;
constexpr x86::reg32 kEffectsHeld = 0x554f28;
constexpr x86::reg32 kEngineHeld = 0x554f2c;
constexpr x86::reg32 kSpeechHeld = 0x554f30;
// The effect playing for a slider, 0 for none.
constexpr x86::reg32 kSample = 0x554f38;

// EA's SND library and the game's sound.
constexpr x86::reg32 kStreamVolume = 0x4e7470;     // SNDstreamautovol: eax the stream, edx the volume
constexpr x86::reg32 kSampleFade = 0x4e9420;       // SNDautovol: eax the sample, edx 5, ebx -1
constexpr x86::reg32 kSampleVolume = 0x4e95fc;     // SNDvol: eax the sample, edx the volume
constexpr x86::reg32 kPlayEffect = 0x4182b0;       // eax a bank, edx its effect, ecx the volume
constexpr x86::reg32 kEffectsBank = 0x571124;
constexpr std::uint32_t kSliderEffect = 7;
constexpr std::uint32_t kSliderEffectPan = 0x40;
constexpr std::uint32_t kMusicStream = 0xa;
constexpr std::uint32_t kMenuMusicStream = 1;
constexpr x86::reg32 kHas3dSound = 0x40faf0;
constexpr x86::reg32 kApplySound = 0x4103a0;       // eax a setting, edx which
constexpr x86::reg32 kStartRaceMusic = 0x4107c0;
constexpr x86::reg32 kRestartSound = 0x40fcf0;
constexpr std::uint32_t kSoundRestartedText = 0x1e6;

// What the sound's restart calls, in turn.
constexpr x86::reg32 kStopMusic = 0x4109d0;
constexpr x86::reg32 kStopSound2 = 0x410aa0;
constexpr x86::reg32 kStopSound3 = 0x410b20;
constexpr x86::reg32 kStopSound4 = 0x40feb0;
constexpr x86::reg32 kLoadFrontendSounds = 0x4136e0;
constexpr x86::reg32 kStartSound2 = 0x410330;
constexpr x86::reg32 kBeforeRestart = 0x4d9120;
constexpr x86::reg32 kAfterRestart = 0x4d9140;

// The sliders' own functions.
constexpr x86::reg32 kVolumeSliderDraw = 0x437a90;
constexpr x86::reg32 kMenuMusicMoved = 0x437990;
constexpr x86::reg32 kRaceMusicMoved = 0x4379b0;
constexpr x86::reg32 kEffectsMoved = 0x4379d0;
constexpr x86::reg32 kEngineMoved = 0x437a20;
constexpr x86::reg32 kSpeechMoved = 0x437a70;

constexpr double kVolumeToSlider = 1.0 / 127.0;    // the double at 0x537600
constexpr double kSliderToVolume = 127.0;          // at 0x537578 and after

constexpr GuestString kAudioCredits{ 0x5375a0, "credits" };
constexpr GuestString kMenuMusicCodelink{ 0x5375a8, "ASLD_MENUMUSIC" };
constexpr GuestString kRaceMusicCodelink{ 0x5375b8, "ASLD_RACEMUSIC" };
constexpr GuestString kEffectsCodelink{ 0x5375c8, "ASLD_SOUNDFX" };
constexpr GuestString kEngineCodelink{ 0x5375d8, "ASLD_ENGINEVOLUME" };
constexpr GuestString kSpeechCodelink{ 0x5375ec, "ASLD_SPEECHVOLUME" };
constexpr GuestString kAudioCancel{ 0x537608, "CANCEL_AUDIO_OPTIONS" };

/* A volume slider found, kept, drawn by the screen's own function, set to the
 * volume and told where its moves go. */
void setUpVolumeSlider(Guest& guest, x86::reg32 menu, const GuestString& codelink, x86::reg32 slot,
                       x86::reg32 volume, x86::reg32 moved)
{
    const x86::reg32 item = fe::findItem(guest, menu, codelink).address();
    guest.setDword(slot, item);
    if (item == 0)
        return;
    guest.setDword(item + kOnDraw, kVolumeSliderDraw);
    fe::X87 x87(guest);
    const float value =
        float(x87.mul(fe::X87::integer(std::int32_t(guest.dword(volume))), x86::Float(kVolumeToSlider)));
    guest.call(kSliderSetOnMove, fe::args({ item, moved }));
    guest.call(kSliderSetValue, fe::args({ guest.dword(slot) }, { floatBits(value), 0 }));
}

/* sub_437e00, audio as it opens (psaudio too): the settings kept, the sliders
 * set to the volumes. */
x86::reg32 onAudioEnter(Guest& guest, x86::reg32 menu)
{
    if (menu == 0)
        return 0;
    if (inRace(guest))
        guest.call(kDimBackground, fe::args({}, { kFloat0_6 }));
    if (guest.call(kHas3dSound) == 0)
        guest.setWord(kSound3d, 0);
    if (!inRace(guest))
        greyCreditsInNetworkRace(guest, menu, kAudioCredits);

    guest.setDword(kSoundSetupKept, guest.dword(kSoundSetup));
    guest.setDword(kRaceMusicHeld, 0);
    guest.setDword(kMenuMusicHeld, 0);
    guest.setDword(kEffectsHeld, 0);
    guest.setDword(kSoundOutputKept, guest.dword(kSoundOutput));
    guest.setDword(kEngineHeld, 0);
    guest.setDword(kSetting1ecKept, guest.dword(kSoundSetting1ec));
    guest.setDword(kSpeechHeld, 0);
    guest.setDword(kSample, 0);

    setUpVolumeSlider(guest, menu, kMenuMusicCodelink, kMenuMusicSlider, kMenuMusicVolume, kMenuMusicMoved);
    setUpVolumeSlider(guest, menu, kRaceMusicCodelink, kRaceMusicSlider, kRaceMusicVolume, kRaceMusicMoved);
    setUpVolumeSlider(guest, menu, kEffectsCodelink, kEffectsSlider,
                      inRace(guest) ? kRaceEffectsVolume : kEffectsVolume, kEffectsMoved);
    setUpVolumeSlider(guest, menu, kEngineCodelink, kEngineSlider,
                      inRace(guest) ? kRaceEngineVolume : kEngineVolume, kEngineMoved);
    setUpVolumeSlider(guest, menu, kSpeechCodelink, kSpeechSlider, kSpeechVolume, kSpeechMoved);

    if (!inRace(guest))
        guest.call(kStreamVolume, fe::args({ kMenuMusicStream, guest.dword(kMenuMusicVolume) }));
    guest.setDword(kRaceEngineKept, guest.dword(kRaceEngineVolume));
    copyDwords(guest, kSoundSettingsKept, kSoundSettings, kSoundSettingsDwords);
    guest.setDword(kRaceEffectsKept, guest.dword(kRaceEffectsVolume));
    return 0;
}

/* Fades the effect a slider plays, if any, and forgets it. */
void stopSample(Guest& guest)
{
    guest.call(kSampleFade, fe::args({ guest.dword(kSample), 5, 0xffffffff }));
    guest.setDword(kSample, 0);
}

/* The output's change applied, if there was one, outside a race. */
void applyOutputChange(Guest& guest)
{
    if (!inRace(guest) && guest.dword(kOutputChanged) != 0)
    {
        guest.setDword(kOutputChanged, 0);
        guest.call(kApplySound, fe::args({ guest.dword(kSoundSetting1ec), 1 }));
    }
}

/* sub_438070, audio as it closes.  In a race the race's own volumes go back
 * to the settings, and its music starts if it was turned up from nothing.
 * Cancel puts back what the screen kept. */
x86::reg32 onAudioExit(Guest& guest, x86::reg32 menu)
{
    if (!inRace(guest))
        guest.call(kStreamVolume, fe::args({ kMusicStream, guest.dword(kMenuMusicVolume) }));
    for (const x86::reg32 cleared : { kRaceMusicSlider, kEffectsSlider, kEngineSlider, kSpeechSlider, kRaceMusicHeld,
                                      kMenuMusicHeld, kEffectsHeld, kEngineHeld, kSpeechHeld, kMenuMusicSlider })
        guest.setDword(cleared, 0);

    if (inRace(guest))
    {
        if (guest.byte(kRaceVolumesToSettings) != 0)
        {
            guest.setDword(kEffectsVolume, guest.dword(kRaceEffectsVolume));
            guest.setDword(kEngineVolume, guest.dword(kRaceEngineVolume));
        }
        if (guest.byte(kRaceMusicPlaying) == 0)
        {
            const x86::reg32 volume = guest.dword(kRaceMusicVolume);
            if (volume != 0)
            {
                const x86::reg32 otherForm = std::int32_t(guest.dword(kSoundOutput)) > 2 ? 1 : 0;
                guest.call(kStartRaceMusic, fe::args({ guest.dword(kRaceMusicTrack), volume, otherForm }));
                guest.setByte(kRaceMusicPlaying, 1);
            }
        }
    }

    if (leftBy(guest, menu, kAudioCancel))
    {
        const std::uint8_t toSettings = guest.byte(kRaceVolumesToSettings);
        guest.setDword(kRaceEffectsVolume, guest.dword(kRaceEffectsKept));
        copyDwords(guest, kSoundSettings, kSoundSettingsKept, kSoundSettingsDwords);
        guest.setDword(kRaceEngineVolume, guest.dword(kRaceEngineKept));
        if (toSettings != 0)
        {
            guest.setDword(kEffectsVolume, guest.dword(kRaceEffectsVolume));
            guest.setDword(kEngineVolume, guest.dword(kRaceEngineVolume));
        }
        applyOutputChange(guest);
        stopSample(guest);
        return 0;
    }
    stopSample(guest);
    applyOutputChange(guest);
    return 0;
}

/* sub_438260, audio on every pass: a change of the setup starts the sound
 * over, a message up meanwhile; the output and the other setting are applied
 * as they change. */
x86::reg32 onAudioFrame(Guest& guest)
{
    if (guest.dword(kSoundSetupKept) != guest.dword(kSoundSetup))
    {
        guest.setDword(kSoundSetupKept, guest.dword(kSoundSetup));
        fe::messageBox(guest, kSoundRestartedText);
        // The message drawn once before the sound stops, the screen under it.
        guest.call(kDialogFadedIn);
        guest.call(kBeforeRestart);
        guest.call(kDrawItems, fe::args({ guest.call(kCurrentMenu) }));
        if ((guest.call(kDialogShown) & 0xff) != 0)
            guest.call(kDialogPass);
        guest.call(kAfterRestart);
        guest.call(kStopMusic);
        guest.call(kStopSound2);
        guest.call(kStopSound3);
        guest.call(kStopSound4);
        guest.call(kRestartSound, fe::args({ guest.dword(kSoundSetup), 1 }));
        guest.call(kLoadFrontendSounds);
        guest.call(kStartSound2);
        guest.call(kApplySound, fe::args({ guest.dword(kSoundSetting1ec), 1 }));
        guest.call(kDialogClose);
    }
    const x86::reg32 output = guest.dword(kSoundOutput);
    if (output != guest.dword(kSoundOutputKept))
    {
        guest.setDword(kSoundOutputKept, output);
        guest.call(kApplySound, fe::args({ output, 0 }));
        guest.setDword(kOutputChanged, 1);
    }
    const x86::reg32 setting = guest.dword(kSoundSetting1ec);
    if (setting != guest.dword(kSetting1ecKept))
    {
        guest.setDword(kSetting1ecKept, setting);
        guest.call(kApplySound, fe::args({ setting, 1 }));
    }
    return 0;
}

/* sub_437990 and the four after it: a volume slider moved, its value back to
 * 0..127, cut towards zero.  The effects' and the engine's are the race's own
 * in a race. */
x86::reg32 onVolumeMoved(Guest& guest, x86::reg32 item, x86::reg32 volume, x86::reg32 raceVolume)
{
    x86::Float value = sliderValue(guest, item, 0);
    if (raceVolume != 0)
    {
        value = x86::Float(float(value));   // kept as a float first: fstp dword, fld dword
        if (inRace(guest))
            volume = raceVolume;
    }
    fe::X87 x87(guest);
    guest.setDword(volume, x86::reg32(x87.truncated(x87.mul(value, x86::Float(kSliderToVolume)))));
    return guest.eax();
}

/* sub_437a90, a volume slider drawn.  Selected, it plays at its volume: the
 * music sliders the music, the others an effect, started the first time. */
x86::reg32 onVolumeSliderDraw(Guest& guest, x86::reg32 item, x86::reg32 selected)
{
    if (guest.call(kShownNow, fe::args({ item, 0xf })) == 0)
        return 0;
    if (selected != 0)
    {
        if (item == guest.dword(kMenuMusicSlider) || item == guest.dword(kRaceMusicSlider))
        {
            const bool menuMusic = item == guest.dword(kMenuMusicSlider);
            const x86::reg32 held = menuMusic ? kMenuMusicHeld : kRaceMusicHeld;
            if (guest.dword(held) == 0)
            {
                if (!inRace(guest))
                    stopSample(guest);
                guest.setDword(held, 1);
            }
            if (!inRace(guest))
                guest.call(kStreamVolume, fe::args({ kMusicStream,
                                                     guest.dword(menuMusic ? kMenuMusicVolume : kRaceMusicVolume) }));
        }
        else
        {
            struct Effect
            {
                x86::reg32 slider;
                x86::reg32 held;
                x86::reg32 volume;
            };
            const Effect effects[] = {
                { kEffectsSlider, kEffectsHeld, kEffectsVolume },
                { kEngineSlider, kEngineHeld, kEngineVolume },
                { kSpeechSlider, kSpeechHeld, kSpeechVolume },
            };
            for (const Effect& effect : effects)
            {
                if (item != guest.dword(effect.slider))
                    continue;
                if (guest.dword(effect.held) == 0)
                {
                    if (!inRace(guest))
                        guest.call(kStreamVolume, fe::args({ kMusicStream, guest.dword(kRaceMusicVolume) }));
                    guest.setDword(effect.held, 1);
                    if (!inRace(guest) && guest.dword(kSample) == 0)
                        guest.setDword(kSample, guest.call(kPlayEffect,
                                                           fe::args({ guest.dword(kEffectsBank), kSliderEffect, 0,
                                                                      guest.dword(effect.volume) },
                                                                    { kSliderEffectPan })));
                }
                if (!inRace(guest) && guest.dword(kSample) != 0)
                    guest.call(kSampleVolume, fe::args({ guest.dword(kSample), guest.dword(effect.volume) }));
                break;
            }
        }
    }
    else
    {
        for (const auto& slider : { std::pair<x86::reg32, x86::reg32>{ kMenuMusicSlider, kMenuMusicHeld },
                                    { kRaceMusicSlider, kRaceMusicHeld },
                                    { kEffectsSlider, kEffectsHeld },
                                    { kEngineSlider, kEngineHeld },
                                    { kSpeechSlider, kSpeechHeld } })
        {
            if (item == guest.dword(slider.first))
            {
                guest.setDword(slider.second, 0);
                break;
            }
        }
    }
    // None held: the menu's music at its own volume, the effect let go.
    if (guest.dword(kMenuMusicHeld) == 0 && guest.dword(kRaceMusicHeld) == 0 && guest.dword(kEffectsHeld) == 0
        && guest.dword(kEngineHeld) == 0 && guest.dword(kSpeechHeld) == 0 && !inRace(guest))
    {
        guest.call(kStreamVolume, fe::args({ kMusicStream, guest.dword(kMenuMusicVolume) }));
        stopSample(guest);
    }
    return guest.call(kSliderDraw, fe::args({ item, selected }));
}

/* ---------------------------------------------------------------------------
 * control1: the controllers
 * ------------------------------------------------------------------------- */

/* The controllers' records, 0x88 bytes each from 0x6fc970: the first dword
 * its kind (0 none), a bit for each of its eight axes, and each axis's two
 * ends, ints from 0 to 100. */
constexpr x86::reg32 kControllers = 0x6fc970;
constexpr x86::reg32 kControllerSize = 0x88;
constexpr x86::reg32 kControllerAxes = 0x40;
constexpr x86::reg32 kControllerRanges = 0x48;
constexpr std::uint32_t kAxes = 8;
constexpr std::uint8_t kHasForceFeedback = 0x08;   // in the first byte
constexpr x86::reg32 kControllerCount = 0x6fd204;
// The first controller's force feedback strength, 0 to 100.
constexpr x86::reg32 kStickStrength = 0x6fc9b4;

/* The players' controls (sub_4904d0): one player's 0x34 bytes at 0x6fd240,
 * or both players' from 0x6fd20c; as the screen opens 0x68 bytes from there
 * are kept. */
constexpr x86::reg32 kControlsOf = 0x4904d0;
constexpr x86::reg32 kControlsKept = 0x5f3cd0;
constexpr std::uint32_t kControlsBytes = 0x68;
constexpr std::uint32_t kOnePlayersControls = 0x34;
// Set by Cancel's Enter: the kept controls go back as the screen closes.
constexpr x86::reg32 kControlsCancelled = 0x5f3da4;
constexpr x86::reg32 kApplyControls = 0x43c040;
constexpr x86::reg32 kDefaultControls = 0x4904f0;  // eax 1: the one player's; 0: both

// The pictures of the controllers, joys.qfs, loaded into the frontend's art
// for as long as their twelve shapes are found by name, jy01 to jy12.
constexpr x86::reg32 kFrontendArt = 0x559228;
constexpr x86::reg32 kArtFolder = 0x7a2db4;
constexpr x86::reg32 kLoadShapes = 0x44ac90;
constexpr x86::reg32 kShapeByName = 0x4d5db0;
constexpr x86::reg32 kJoyShapes = 0x5f3d38;       // int16 each
constexpr std::uint32_t kJoyShapeCount = 12;

constexpr GuestString kControlsCancel{ 0x53778c, "cancel" };
constexpr GuestString kJoysFile{ 0x537794, "joys.qfs" };
constexpr GuestString kPathFormat{ 0x5377a0, "%s%s" };
constexpr GuestString kJoyShapeFormat{ 0x5377a8, "jy%2d" };
constexpr GuestString kForceFeedbackButton{ 0x5377b0, "CTRL1_FORCEFEED" };
constexpr GuestString kDeadZoneButton{ 0x5377c0, "CTRL1_DEADZONE" };
constexpr GuestString kResetButton{ 0x5377d0, "reset" };

constexpr x86::reg32 kControlsCancelKey = 0x43d3f0;
constexpr x86::reg32 kResetClicked = 0x43d460;
constexpr x86::reg32 kResetAnswered = 0x43d410;
// The question before the reset: its line, its two buttons.
constexpr x86::reg32 kResetLines = 0x5f3db8;
constexpr x86::reg32 kResetButtons = 0x5f3dbc;
constexpr std::uint32_t kResetQuestion = 0x1ab;
constexpr std::uint32_t kYes = 0xc4;
constexpr std::uint32_t kNo = 0xc5;

/* sub_43d4d0, control1 as it opens: the controls kept, the controllers'
 * pictures found, force feedback and the dead zones only with a controller
 * that has them. */
x86::reg32 onControlsEnter(Guest& guest, x86::reg32 menu)
{
    if (menu == 0)
        return 0;
    guest.setDword(kControlsCancelled, 0);
    if (Item cancel = fe::findItem(guest, menu, kControlsCancel))
        guest.setDword(cancel.address() + kOnKey, kControlsCancelKey);
    const x86::reg32 controls = guest.call(kControlsOf, fe::args({ guest.dword(kSplitScreen) }));
    guest.call(kCopy, fe::args({ controls, kControlsKept, kControlsBytes }));

    const x86::reg32 name = guest.frame();   // char[0x50]
    const x86::reg32 art = guest.dword(kFrontendArt);
    guest.call(kSprintf, fe::args({}, { name, kPathFormat.address, kArtFolder, kJoysFile.address }));
    guest.setDword(kFrontendArt, guest.call(kLoadShapes, fe::args({ name, 0 })));
    for (std::uint32_t i = 0; i < kJoyShapeCount; ++i)
    {
        guest.call(kSprintf, fe::args({}, { name, kJoyShapeFormat.address, i + 1 }));
        if (guest.byte(name + 2) == ' ')
            guest.setByte(name + 2, '0');
        guest.setWord(kJoyShapes + 2 * i, std::uint16_t(guest.call(kShapeByName, fe::args({ name }))));
    }
    guest.setDword(kFrontendArt, art);

    Item force = fe::findItem(guest, menu, kForceFeedbackButton);
    if ((guest.byte(kControllers) & kHasForceFeedback) == 0 && force)
        force.setDisabled(true);
    Item deadZone = fe::findItem(guest, menu, kDeadZoneButton);
    if (guest.dword(kControllers) == 0 && deadZone)
        deadZone.setDisabled(true);
    if (Item reset = fe::findItem(guest, menu, kResetButton))
        reset.setOnClick(kResetClicked);
    return 0;
}

/* sub_43d5f0, control1 as it closes: after Cancel the controls go back --
 * in split screen only the first player's half, as the original has it --
 * and otherwise, outside a race, they are applied. */
x86::reg32 onControlsExit(Guest& guest)
{
    const x86::reg32 controls = guest.call(kControlsOf, fe::args({ guest.dword(kSplitScreen) }));
    if (guest.dword(kControlsCancelled) != 0)
    {
        const std::uint32_t bytes = guest.dword(kSplitScreen) == 1 ? kControlsBytes : kOnePlayersControls;
        guest.call(kCopy, fe::args({ kControlsKept, controls, bytes }));
    }
    else if (!inRace(guest))
    {
        guest.call(kApplyControls);
    }
    return 0;
}

/* sub_43d3f0 (control1's Cancel), sub_442db0 (deadzone's), sub_447b40
 * (force's): Enter marks the screen cancelled, or not kept; the key goes on
 * to the button. */
x86::reg32 onCancelKey(Guest& guest, x86::reg32 item, x86::reg32 menu, std::uint16_t key, x86::reg32 flag,
                       x86::reg32 value)
{
    if (key == kKeyEnter)
        guest.setDword(flag, value);
    return passKey(guest, kButtonKey, item, menu, key);
}

/* sub_43d460, the reset button: a question first. */
x86::reg32 onResetClicked(Guest& guest)
{
    guest.setDword(kResetLines, fe::text(guest, kResetQuestion));
    guest.setDword(kResetButtons, fe::text(guest, kYes));
    guest.setDword(kResetButtons + 4, fe::text(guest, kNo));
    fe::Question question;
    question.lineCount = 1;
    question.lines = kResetLines;
    question.buttonCount = 2;
    question.buttons = kResetButtons;
    question.buttonHelp = 0;
    question.focus = 0;
    question.column = 0;
    question.callback = kResetAnswered;
    fe::openQuestion(guest, question);
    return fe::kDialogHandled;
}

/* sub_43d410, the question answered: Yes puts the controls back to their
 * defaults, the one player's or both. */
x86::reg32 onResetAnswered(Guest& guest, std::uint32_t answer, x86::reg32 close)
{
    guest.setByte(close, 0);
    if (answer == fe::kDialogEnter)
    {
        const std::uint32_t focus = fe::dialogFocus(guest);
        if (focus == 0)
        {
            guest.call(kDefaultControls, fe::args({ guest.dword(kSplitScreen) == 1 ? 1u : 0u }));
            guest.setByte(close, 1);
        }
        else if (focus == 1)
        {
            guest.setByte(close, 1);
        }
    }
    else if (answer == fe::kDialogEscape)
    {
        guest.setByte(close, 1);
    }
    return fe::kDialogHandled;
}

/* ---------------------------------------------------------------------------
 * deadzone: a controller's axes
 * ------------------------------------------------------------------------- */

/* The eight axes' sliders, each with two ends -- the axis's two ends, from
 * the top -- and a third, where the controller stands now.  The screen's data
 * from 0x558a10, 0xec bytes, cleared as it opens. */
constexpr x86::reg32 kDeadZoneData = 0x558a10;
constexpr std::uint32_t kDeadZoneDataBytes = 0xec;
constexpr x86::reg32 kAxisSliders = 0x558a10;     // eight items
constexpr x86::reg32 kAxisLastEnds = 0x558a30;    // two floats each, as last applied
constexpr x86::reg32 kControllerShown = 0x558a70;
constexpr x86::reg32 kControllerChosen = 0x558a74;
constexpr x86::reg32 kControllerState = 0x558a78; // what sub_4358f0 reads the controller into
constexpr x86::reg32 kAxisPositions = 0x558ac8;   // ints, 0 to 65535
constexpr x86::reg32 kReadController = 0x4358f0;
// An axis's end applied: eax the controller, edx the axis, ebx 1 for the
// first end (counted from the other side), 0 the second, ecx 0 to 100.
constexpr x86::reg32 kApplyRange = 0x435b40;
// The ends as the screen opened, 64 bytes for each controller; put back as
// it closes unless kDeadZoneKept, which Cancel's Enter clears.
constexpr x86::reg32 kRangesKept = 0x660bc0;
constexpr std::uint32_t kRangesKeptSize = 0x40;
constexpr x86::reg32 kDeadZoneKept = 0x660fc0;

/* The list of controllers DZ_JOYSEL shows, 16 entries: a line of text each
 * (int16, 0x855 the first controller's name), whether it is there (int16)
 * and its colour. */
constexpr x86::reg32 kControllerListText = 0x555648;
constexpr x86::reg32 kControllerListShown = 0x555668;
constexpr x86::reg32 kControllerListColour = 0x555688;
constexpr std::uint32_t kControllerListEntries = 16;
constexpr std::uint32_t kFirstControllerName = 0x855;
constexpr std::uint32_t kControllerListColourValue = 0xff649dfd;

// A slider laid out in a race's pause screen: x, and y by its row.
constexpr std::uint16_t kPauseSliderX = 0xf2;
constexpr x86::reg32 kSliderOrder = 0x40;         // int16: its row among the shown ones
constexpr x86::reg32 kSliderOutside = 0x3d;       // byte: 1 outside a race

constexpr float kHundredth = 0.01f;               // at 0x538a40 and 0x538d44
constexpr float kHundred = 100.0f;                // at 0x538a10, 0x538ca0 and after
constexpr float kMinimumGap = 0.1f;               // at 0x538a14
constexpr float kMinusMinimumGap = -0.1f;         // at 0x538a18
constexpr float kPositionScale = 1.0f / 65536.0f; // at 0x538a1c

constexpr GuestString kDeadZoneCredits{ 0x538a44, "credits" };
constexpr GuestString kDeadZoneCancel{ 0x538a4c, "DEAD_CANCEL" };
constexpr GuestString kAxisCodelinks[kAxes] = {
    { 0x538a58, "SLDER_XAXIS" },    { 0x538a64, "SLDER_YAXIS" },    { 0x538a70, "SLDER_ZAXIS" },
    { 0x538a7c, "SLDER_ROTXAXIS" }, { 0x538a8c, "SLDER_ROTYAXIS" }, { 0x538a9c, "SLDER_ROTZAXIS" },
    { 0x538aac, "SLDER_SLIDEA" },   { 0x538abc, "SLDER_SLIDEB" },
};
constexpr GuestString kControllerChoice{ 0x538acc, "DZ_JOYSEL" };
// Items the screen has and hides.
constexpr GuestString kHiddenItems[] = {
    { 0x538a20, "TEST" }, { 0x538a28, "WHAT_R0" }, { 0x538a30, "WHAT_R1" }, { 0x538a38, "WHAT_R2" },
};

constexpr x86::reg32 kFillControllerList = 0x43d9a0;
constexpr x86::reg32 kDeadZoneCancelKey = 0x442db0;
constexpr x86::reg32 kChooseController = 0x443260;
constexpr x86::reg32 kChooseControllerInRace = 0x443290;
constexpr x86::reg32 kShowController = 0x443070;
constexpr x86::reg32 kAxisMoved = 0x442ef0;
constexpr x86::reg32 kAxisDraw = 0x442ff0;
constexpr x86::reg32 kAxisIndex = 0x442e60;
constexpr x86::reg32 kApplyAxis = 0x442dd0;

x86::reg32 controllerRanges(x86::reg32 controller)
{
    return kControllers + controller * kControllerSize + kControllerRanges;
}

/* sub_43d9a0: the list, one entry for each controller there is. */
x86::reg32 fillControllerList(Guest& guest)
{
    for (std::uint32_t i = 0; i < kControllerListEntries; ++i)
    {
        if (std::int32_t(i) < std::int32_t(guest.dword(kControllerCount)))
        {
            guest.setWord(kControllerListText + 2 * i, std::uint16_t(i + kFirstControllerName));
            guest.setWord(kControllerListShown + 2 * i, 1);
            guest.setDword(kControllerListColour + 4 * i, kControllerListColourValue);
        }
        else
        {
            guest.setWord(kControllerListShown + 2 * i, 0);
        }
    }
    return kControllerListEntries;
}

/* sub_442e60: which axis a slider is, 0 for none. */
x86::reg32 axisIndex(const Guest& guest, x86::reg32 item)
{
    for (std::uint32_t axis = 0; axis < kAxes; ++axis)
    {
        if (item == guest.dword(kAxisSliders + 4 * axis))
            return axis;
    }
    return 0;
}

/* sub_442dd0: an axis's two ends, from a slider's (0 to 1, from the top),
 * to the controller's record in hundredths, each applied when it changed.
 * The slider's first end is the record's second. */
x86::reg32 applyAxis(Guest& guest, float first, float second, x86::reg32 controller, x86::reg32 axis)
{
    fe::X87 x87(guest);
    const x86::Float hundred = x86::Float(kHundred);
    const x86::Float firstScaled = x87.mul(x86::Float(first), hundred);
    const x86::Float secondScaled = x87.mul(hundred, x86::Float(second));
    const std::int32_t firstEnd = x87.truncated(firstScaled);
    const std::int32_t secondEnd = x87.truncated(secondScaled);
    const x86::reg32 ends = controllerRanges(controller) + 8 * axis;
    x86::reg32 result = guest.dword(ends + 4);
    if (guest.dword(ends + 4) != x86::reg32(firstEnd))
    {
        guest.setDword(ends + 4, x86::reg32(firstEnd));
        result = guest.call(kApplyRange, fe::args({ controller, axis, 1, x86::reg32(firstEnd) }));
    }
    if (x86::reg32(secondEnd) != guest.dword(ends))
    {
        guest.setDword(ends, x86::reg32(secondEnd));
        result = guest.call(kApplyRange, fe::args({ controller, axis, 0, x86::reg32(secondEnd) }));
    }
    return result;
}

/* sub_442ef0, an axis's slider moved: its two ends kept a tenth apart, the
 * one that did not move giving way, and applied. */
x86::reg32 onAxisMoved(Guest& guest, x86::reg32 item)
{
    const x86::reg32 onMove = guest.dword(item + kSliderOnMove);
    const x86::reg32 axis = guest.call(kAxisIndex, fe::args({ item }));
    const float first = float(sliderValue(guest, item, 0));
    const float second = float(sliderValue(guest, item, 1));
    x86::FPU& fpu = guest.fpu();
    const x86::Float firstFromBottom = fp::sub(x86::Float(1.0), x86::Float(first));
    const float gap = float(fp::abs(fp::sub(firstFromBottom, x86::Float(second))));
    fe::X87 x87(guest);
    if (x87.below(firstFromBottom, x86::Float(second)) || x87.below(x86::Float(gap), x86::Float(kMinimumGap)))
    {
        guest.call(kSliderSetOnMove, fe::args({ item, 0 }));
        const x86::reg32 last = kAxisLastEnds + 8 * axis;
        if (!same(guest, x86::Float(first), x86::Float(guest.floatAt(last))))
        {
            const float moved =
                float(fp::add(fp::sub(x86::Float(1.0), x86::Float(second)), x86::Float(kMinusMinimumGap)));
            guest.call(kSliderSetValue, fe::args({ item }, { floatBits(moved), 0 }));
        }
        else if (!same(guest, x86::Float(second), x86::Float(guest.floatAt(last + 4))))
        {
            const float moved =
                float(fp::add(fp::sub(x86::Float(1.0), x86::Float(first)), x86::Float(kMinusMinimumGap)));
            guest.call(kSliderSetValue, fe::args({ item }, { floatBits(moved), 1 }));
        }
        guest.call(kSliderSetOnMove, fe::args({ item, onMove }));
    }
    const x86::reg32 firstNow = floatBits(float(sliderValue(guest, item, 0)));
    const x86::reg32 secondNow = floatBits(float(sliderValue(guest, item, 1)));
    guest.setDword(kAxisLastEnds + 8 * axis, firstNow);
    guest.setDword(kAxisLastEnds + 8 * axis + 4, secondNow);
    return guest.call(kApplyAxis, fe::args({}, { firstNow, secondNow, guest.dword(kControllerChosen), axis }));
}

/* sub_442ff0, an axis's slider drawn: its third end where the controller
 * stands now. */
x86::reg32 onAxisDraw(Guest& guest, x86::reg32 item, x86::reg32 selected)
{
    if (guest.call(kShownNow, fe::args({ item, 0xf })) == 0)
        return 0;
    const x86::reg32 axis = guest.call(kAxisIndex, fe::args({ item }));
    const x86::reg32 onMove = guest.dword(item + kSliderOnMove);
    guest.call(kSliderSetOnMove, fe::args({ item, 0 }));
    guest.call(kReadController, fe::args({ guest.dword(kControllerChosen), kControllerState }));
    fe::X87 x87(guest);
    const x86::Float position = x87.mul(fe::X87::integer(std::int32_t(guest.dword(kAxisPositions + 4 * axis))),
                                        x86::Float(kPositionScale));
    const float value = float(x87.sub(x86::Float(1.0), position));
    guest.call(kSliderSetValue, fe::args({ item, onMove }, { floatBits(value), 2 }));
    guest.call(kSliderSetOnMove, fe::args({ item, onMove }));
    return guest.call(kSliderDraw, fe::args({ item, selected }));
}

/* sub_443070: a controller's axes shown, a slider for each it has, laid out
 * one under the other in a race's pause screen. */
x86::reg32 showController(Guest& guest, x86::reg32 menu, x86::reg32 controller)
{
    if (menu == 0)
        return 0;
    guest.setDword(kControllerShown, controller);
    guest.setDword(kControllerChosen, controller);
    const x86::reg32 record = kControllers + controller * kControllerSize;
    std::uint16_t order = 0;
    fe::X87 x87(guest);
    for (std::uint32_t axis = 0; axis < kAxes; ++axis)
    {
        const x86::reg32 slider = guest.dword(kAxisSliders + 4 * axis);
        if (slider == 0)
            continue;
        if (inRace(guest))
        {
            guest.setWord(slider + fe::item::kX, kPauseSliderX);
            guest.setWord(slider + fe::item::kY, guest.word(kRowY + 4 * x86::reg32(std::int32_t(std::int16_t(order)))));
            guest.setByte(slider + kSliderOutside, 0);
        }
        else
        {
            guest.setByte(slider + kSliderOutside, 1);
        }
        if (record != 0 && (guest.dword(record + kControllerAxes) & (1u << axis)) != 0)
        {
            const x86::reg32 ends = controllerRanges(guest.dword(kControllerChosen)) + 8 * axis;
            guest.call(kSliderSetOnMove, fe::args({ slider, kAxisMoved }));
            guest.setDword(slider + kOnDraw, kAxisDraw);
            Item(guest, slider).setHidden(false);
            guest.setWord(slider + kSliderOrder, order);
            // Not told of its own ends being set.
            guest.call(kSliderSetOnMove, fe::args({ slider, 0 }));
            const float first = float(
                x87.mul(fe::X87::integer(std::int32_t(guest.dword(ends + 4))), x86::Float(kHundredth)));
            guest.call(kSliderSetValue, fe::args({ slider }, { floatBits(first), 0 }));
            const float second =
                float(x87.mul(fe::X87::integer(std::int32_t(guest.dword(ends))), x86::Float(kHundredth)));
            guest.call(kSliderSetValue, fe::args({ slider, kAxisMoved }, { floatBits(second), 1 }));
            ++order;
            guest.call(kSliderSetOnMove, fe::args({ slider, kAxisMoved }));
        }
        else
        {
            guest.call(kSliderSetOnMove, fe::args({ slider, 0 }));
            guest.setDword(slider + kOnDraw, kSliderDraw);
            Item(guest, slider).setHidden(true);
        }
    }
    x86::reg32 found = 0;
    for (const GuestString& codelink : kHiddenItems)
    {
        Item item = fe::findItem(guest, menu, codelink);
        if (item)
            item.setHidden(true);
        found = item.address();
    }
    return found;
}

/* sub_443260 (the list in the menus) and sub_443290 (the button in a race),
 * DZ_JOYSEL's key: another controller chosen, its axes shown. */
x86::reg32 onChooseControllerKey(Guest& guest, x86::reg32 handler, x86::reg32 item, x86::reg32 menu,
                                 std::uint16_t key)
{
    const x86::reg32 result = passKey(guest, handler, item, menu, key);
    const x86::reg32 chosen = guest.dword(kControllerChosen);
    if (chosen != guest.dword(kControllerShown))
        guest.call(kShowController, fe::args({ menu, chosen }));
    else
        guest.leaveEdx(chosen);
    guest.leaveEbx(result);
    return result;
}

/* sub_4432c0, deadzone as it opens (psdeadz too): every controller's ends
 * kept, the sliders found, the first controller shown. */
x86::reg32 onDeadZoneEnter(Guest& guest, x86::reg32 menu)
{
    if (menu == 0)
        return 0;
    if (inRace(guest))
        guest.call(kDimBackground, fe::args({}, { kFloat0_6 }));
    guest.call(kFillControllerList);
    greyCreditsInNetworkRace(guest, menu, kDeadZoneCredits);
    guest.setDword(kDeadZoneKept, 1);
    if (Item cancel = fe::findItem(guest, menu, kDeadZoneCancel))
        guest.setDword(cancel.address() + kOnKey, kDeadZoneCancelKey);
    for (x86::reg32 controller = 0; std::int32_t(controller) < std::int32_t(guest.dword(kControllerCount));
         ++controller)
        guest.call(kCopy, fe::args({ controllerRanges(controller), kRangesKept + kRangesKeptSize * controller,
                                     kRangesKeptSize }));
    guest.call(kFillZero, fe::args({ kDeadZoneData, kDeadZoneDataBytes }));
    for (std::uint32_t axis = 0; axis < kAxes; ++axis)
        guest.setDword(kAxisSliders + 4 * axis, fe::findItem(guest, menu, kAxisCodelinks[axis]).address());
    guest.call(kShowController, fe::args({ menu, 0 }));
    if (Item choice = fe::findItem(guest, menu, kControllerChoice))
        guest.setDword(choice.address() + kOnKey, inRace(guest) ? kChooseControllerInRace : kChooseController);
    return 0;
}

/* sub_443430, deadzone as it closes: after Cancel every controller's ends go
 * back and are applied again. */
x86::reg32 onDeadZoneExit(Guest& guest)
{
    if (guest.dword(kDeadZoneKept) != 0)
        return 0;
    for (x86::reg32 controller = 0; std::int32_t(controller) < std::int32_t(guest.dword(kControllerCount));
         ++controller)
    {
        const x86::reg32 kept = kRangesKept + kRangesKeptSize * controller;
        guest.call(kCopy, fe::args({ kept, controllerRanges(controller), kRangesKeptSize }));
        for (std::uint32_t axis = 0; axis < kAxes; ++axis)
        {
            if ((guest.dword(kControllers + controller * kControllerSize + kControllerAxes) & (1u << axis)) == 0)
                continue;
            guest.call(kApplyRange, fe::args({ controller, axis, 1, guest.dword(kept + 8 * axis + 4) }));
            guest.call(kApplyRange, fe::args({ controller, axis, 0, guest.dword(kept + 8 * axis) }));
        }
    }
    return 0;
}

/* ---------------------------------------------------------------------------
 * force: force feedback
 * ------------------------------------------------------------------------- */

/* Six sliders, their items from 0x6611e0, cleared as the screen opens and
 * closes.  The strength is the first controller's; the five effects, 0 to
 * 100, are kept from 0x6fd1f0 -- body roll and road grip never both: one
 * moved from 0 puts the other to 0. */
constexpr x86::reg32 kForceSliders = 0x6611e0;
constexpr std::uint32_t kForceSliderBytes = 0x18;
constexpr x86::reg32 kBodyRollSlider = 0x6611f0;
constexpr x86::reg32 kRoadGripSlider = 0x6611f4;
constexpr x86::reg32 kForceEffects = 0x6fd1f0;
constexpr std::uint32_t kForceEffectsBytes = 0x14;
constexpr x86::reg32 kCollisionForce = 0x6fd1f0;
constexpr x86::reg32 kRoadEffectsForce = 0x6fd1f4;
constexpr x86::reg32 kEngineForce = 0x6fd1f8;
constexpr x86::reg32 kBodyRollForce = 0x6fd1fc;
constexpr x86::reg32 kRoadGripForce = 0x6fd200;
// What they were as the screen opened, and whether Cancel was pressed.
constexpr x86::reg32 kForceEffectsKept = 0x6611f8;
constexpr x86::reg32 kStrengthKept = 0x66120c;
constexpr x86::reg32 kForceCancelled = 0x661210;
constexpr x86::reg32 kApplyStrength = 0x435df0;   // eax the controller, edx the strength
// Byte: 0x10 hides road grip.
constexpr x86::reg32 kEdition = 0x55eb2e;
constexpr std::uint8_t kNoRoadGrip = 0x10;

// A slider laid out in a race's pause screen.
constexpr std::uint16_t kPauseForceSliderX = 0x140;
constexpr x86::reg32 kSliderLength = 0x64;        // int16
constexpr x86::reg32 kSliderBarY = 0x66;          // int16
constexpr std::uint16_t kPauseSliderLength = 0x40;
constexpr std::uint8_t kStrengthBrightness = 0x7f;
constexpr float kStrengthToBrightness = 128.0f;   // at 0x538ca4

constexpr GuestString kForceCredits{ 0x538cbc, "FORCE_CREDITS" };
constexpr GuestString kForceCodelinks[] = {
    { 0x538ccc, "FORCE_STICKVOLUME" }, { 0x538ce0, "FORCE_ROADEFFECTS" }, { 0x538cf4, "FORCE_COLLISION" },
    { 0x538d04, "FORCE_ENGINE" },      { 0x538d14, "FORCE_BODYROLL" },    { 0x538d24, "FORCE_ROADGRIP" },
};
constexpr GuestString kForceCancel{ 0x538d34, "FORCE_CANCEL" };

constexpr x86::reg32 kForceCancelKey = 0x447b40;
constexpr x86::reg32 kStrengthMoved = 0x447b60;
constexpr x86::reg32 kRoadEffectsMoved = 0x447c40;
constexpr x86::reg32 kCollisionMoved = 0x447c70;
constexpr x86::reg32 kEngineForceMoved = 0x447ca0;
constexpr x86::reg32 kBodyRollMoved = 0x447cd0;
constexpr x86::reg32 kRoadGripMoved = 0x447d50;

/* sub_447dd0, force as it opens (psforce too): the sliders set from the
 * settings, which are kept. */
x86::reg32 onForceEnter(Guest& guest, x86::reg32 menu)
{
    if (menu == 0)
        return 0;
    if (inRace(guest))
        guest.call(kDimBackground, fe::args({}, { kFloat0_6 }));
    guest.call(kFillZero, fe::args({ kForceSliders, kForceSliderBytes }));
    if (Item credits = fe::findItem(guest, menu, kForceCredits))
    {
        if (networkRace(guest))
            credits.setDisabled(true);
    }
    for (std::uint32_t i = 0; i < std::size(kForceCodelinks); ++i)
        guest.setDword(kForceSliders + 4 * i, fe::findItem(guest, menu, kForceCodelinks[i]).address());

    struct Slider
    {
        std::uint16_t row;
        x86::reg32 setting;
        x86::reg32 moved;
    };
    const Slider sliders[] = {
        { 0, kStickStrength, kStrengthMoved },        { 2, kRoadEffectsForce, kRoadEffectsMoved },
        { 3, kCollisionForce, kCollisionMoved },      { 4, kEngineForce, kEngineForceMoved },
        { 6, kBodyRollForce, kBodyRollMoved },        { 7, kRoadGripForce, kRoadGripMoved },
    };
    fe::X87 x87(guest);
    for (std::uint32_t i = 0; i < std::size(sliders); ++i)
    {
        const x86::reg32 slot = kForceSliders + 4 * i;
        const x86::reg32 slider = guest.dword(slot);
        if (slider == 0)
            continue;
        const Slider& s = sliders[i];
        if (inRace(guest))
        {
            const std::uint16_t y = guest.word(kRowY + 4 * s.row);
            guest.setByte(slider + kSliderOutside, 0);
            guest.setWord(slider + kSliderOrder, s.row);
            guest.setWord(slider + fe::item::kX, kPauseForceSliderX);
            guest.setWord(slider + kSliderLength, kPauseSliderLength);
            guest.setWord(slider + fe::item::kY, y);
            guest.setWord(slider + kSliderBarY, y);
        }
        else
        {
            guest.setByte(slider + kSliderOutside, 1);
            guest.setWord(slider + kSliderOrder, s.row);
        }
        // The strength is signed, the effects read as unsigned (fild qword).
        const x86::Float setting = i == 0 ? fe::X87::integer(std::int32_t(guest.dword(s.setting)))
                                          : x86::Float(double(guest.dword(s.setting)));
        const float value = float(x87.mul(setting, x86::Float(kHundredth)));
        if (s.moved == kRoadGripMoved)
        {
            guest.call(kSliderSetValue, fe::args({ guest.dword(slot) }, { floatBits(value), 0 }));
            if (guest.byte(kEdition) & kNoRoadGrip)
                Item(guest, guest.dword(slot)).setHidden(true);
            else
                guest.call(kSliderSetOnMove, fe::args({ guest.dword(slot), s.moved }));
        }
        else
        {
            guest.call(kSliderSetValue, fe::args({ guest.dword(slot), s.moved }, { floatBits(value), 0 }));
            guest.call(kSliderSetOnMove, fe::args({ guest.dword(slot), s.moved }));
        }
    }

    if (Item cancel = fe::findItem(guest, menu, kForceCancel))
        guest.setDword(cancel.address() + kOnKey, kForceCancelKey);
    guest.call(kCopy, fe::args({ kForceEffects, kForceEffectsKept, kForceEffectsBytes }));
    guest.setDword(kStrengthKept, guest.dword(kStickStrength));
    guest.setDword(kForceCancelled, 0);
    return 0;
}

/* sub_4481c0, force as it closes: Cancel puts the settings back; the strength
 * is applied either way. */
x86::reg32 onForceExit(Guest& guest)
{
    if (guest.dword(kForceCancelled) == 1)
    {
        guest.call(kCopy, fe::args({ kForceEffectsKept, kForceEffects, kForceEffectsBytes }));
        guest.setDword(kStickStrength, guest.dword(kStrengthKept));
    }
    guest.call(kFillZero, fe::args({ kForceSliders, kForceSliderBytes }));
    guest.call(kApplyStrength, fe::args({ 0, guest.dword(kStickStrength) }));
    return 0;
}

/* sub_447b60, the strength moved: kept in hundredths, and the other sliders'
 * bars drawn the brighter for it. */
x86::reg32 onStrengthMoved(Guest& guest, x86::reg32 item)
{
    const x86::Float value = sliderValue(guest, item, 0);
    fe::X87 x87(guest);
    const x86::Float hundredths = x87.mul(value, x86::Float(kHundred));
    const x86::Float scaled = x87.mul(value, x86::Float(kStrengthToBrightness));
    const std::int32_t strength = x87.truncated(hundredths);
    const std::int32_t brightness = x87.truncated(scaled);
    guest.setDword(kStickStrength, x86::reg32(strength));
    const std::uint8_t colour = std::uint8_t((x86::reg32(brightness) & 0xff) + kStrengthBrightness);
    x86::reg32 last = 0;
    for (std::uint32_t i = 1; i < std::size(kForceCodelinks); ++i)
    {
        last = guest.dword(kForceSliders + 4 * i);
        if (last != 0)
            guest.call(kSliderSetColour, fe::args({ last, colour, 1 }));
    }
    return last;
}

/* sub_447c40, sub_447c70, sub_447ca0: an effect moved, kept in hundredths. */
x86::reg32 onForceMoved(Guest& guest, x86::reg32 item, x86::reg32 setting)
{
    fe::X87 x87(guest);
    const std::int32_t value = x87.truncated(x87.mul(sliderValue(guest, item, 0), x86::Float(kHundred)));
    guest.setDword(setting, x86::reg32(value));
    return x86::reg32(value);
}

/* sub_447cd0 (body roll) and sub_447d50 (road grip): moved from 0, the other
 * goes to 0, its slider too. */
x86::reg32 onExclusiveForceMoved(Guest& guest, x86::reg32 item, x86::reg32 setting, x86::reg32 other,
                                 x86::reg32 otherSlider)
{
    fe::X87 x87(guest);
    const x86::Float scaled = x87.mul(sliderValue(guest, item, 0), x86::Float(kHundred));
    const x86::reg32 bodyRoll = guest.dword(kBodyRollSlider);
    const std::int32_t value = x87.truncated(scaled);
    guest.call(kSliderSetOnMove, fe::args({ bodyRoll, 0 }));
    guest.call(kSliderSetOnMove, fe::args({ guest.dword(kRoadGripSlider), 0 }));
    if (value != 0)
    {
        const x86::reg32 slider = guest.dword(otherSlider);
        guest.setDword(setting, x86::reg32(value));
        guest.setDword(other, 0);
        guest.call(kSliderSetValue, fe::args({ slider }, { 0, 0 }));
    }
    guest.call(kSliderSetOnMove, fe::args({ guest.dword(kRoadGripSlider), kRoadGripMoved }));
    return guest.call(kSliderSetOnMove, fe::args({ guest.dword(kBodyRollSlider), kBodyRollMoved }));
}

/* ---------------------------------------------------------------------------
 * graphics and advgrph
 * ------------------------------------------------------------------------- */

/* The graphics settings: sixteen dwords from 0x6fbc10, kept whole as either
 * screen opens. */
constexpr x86::reg32 kGraphicsSettings = 0x6fbc10;
constexpr x86::reg32 kGraphicsKept = 0x6627b0;
constexpr std::uint32_t kGraphicsSettingsDwords = 16;
constexpr x86::reg32 kScreenMode = 0x6fbc14;       // the mode's index in the list
constexpr x86::reg32 kScreenModeValue = 0x6fbc18;  // the chosen mode's second dword
constexpr x86::reg32 kProjectedText = 0x6fbc34;
constexpr x86::reg32 kEffectsIntensity = 0x6fbc38; // float
// The screen modes the driver offers, listed by sub_4482c0: 0x114 bytes each.
constexpr x86::reg32 kListModes = 0x4482c0;
constexpr x86::reg32 kModes = 0x661220;
constexpr x86::reg32 kModeSize = 0x114;
constexpr x86::reg32 kModeCount = 0x559218;
constexpr x86::reg32 kModeHasDepthBuffer = 0x10;
// The chosen mode copied to the buffer in esi.
constexpr x86::reg32 kCurrentMode = 0x448330;
// What the display can do (byte; 1 << the driver's type, sub_4b6020): 0x06
// the car's shading and the perspective.
constexpr x86::reg32 kDisplayFlags = 0x7a3a58;
constexpr std::uint8_t kDisplayShading = 0x06;
constexpr x86::reg32 kDisplayBuffering = 0x7a3a68;
// 1 shows the germanonly item.
constexpr x86::reg32 kLanguage = 0x6fbb4c;
// The z buffer's items, found as advgrph opens.
constexpr x86::reg32 kDepthBufferItem = 0x6627f0;
constexpr x86::reg32 kDepthBufferText = 0x6627f4;
constexpr x86::reg32 kSliderValueField = 0x54;      // float
constexpr x86::reg32 kEffectsIntensityMoved = 0x448570;

constexpr GuestString kCarShade{ 0x538d60, "CARSHADE" };
constexpr GuestString kCarShadeText{ 0x538d6c, "CARSHADE_TEXT" };
constexpr GuestString kGraphicsCredits{ 0x538d7c, "credits" };
constexpr GuestString kGraphicsCancel{ 0x538d84, "CANCEL_GRAPHICS_OPTIONS" };
constexpr GuestString kProjectedTextButton{ 0x538d9c, "PROJ_TEXT_BUTTON" };
constexpr GuestString kProjectedTextList{ 0x538db0, "PROJ_TEXT_CASCADE" };
constexpr GuestString kGermanOnly{ 0x538dc4, "germanonly" };
constexpr GuestString kPerspectiveList{ 0x538dd0, "GRAPH_PERSP_CASCADE" };
constexpr GuestString kPerspective{ 0x538de4, "GRAPH_PERSP" };
constexpr GuestString kTrackLightingList{ 0x538df0, "GRAPH_TRACKLIGHTING_CASCADE" };
constexpr GuestString kTrackLighting{ 0x538e0c, "GRAPH_TRACKLIGHTING" };
constexpr GuestString kDepthBuffer{ 0x538e20, "Z_BUFFER" };
constexpr GuestString kDepthBufferTextCodelink{ 0x538e2c, "Z_BUFFER_TEXT" };
constexpr GuestString kEffectsIntensityCodelink{ 0x538e3c, "SFX_INTENSITY" };
constexpr GuestString kAdvancedCancel{ 0x538e4c, "CANCEL_ADVGRAPHICS_OPTIONS" };

bool displayShading(const Guest& guest)
{
    return (guest.byte(kDisplayFlags) & kDisplayShading) != 0;
}

/* sub_448440, graphics as it opens: the modes listed, the settings kept, no
 * car shading where the display has none. */
x86::reg32 onGraphicsEnter(Guest& guest, x86::reg32 menu)
{
    guest.call(kListModes);
    if (std::int32_t(guest.dword(kScreenMode)) >= std::int32_t(guest.dword(kModeCount)))
        guest.setDword(kScreenMode, 0);
    copyDwords(guest, kGraphicsKept, kGraphicsSettings, kGraphicsSettingsDwords);
    if (Item shade = fe::findItem(guest, menu, kCarShade))
    {
        if (displayShading(guest))
            shade.setDisabled(true);
    }
    if (Item text = fe::findItem(guest, menu, kCarShadeText))
    {
        if (displayShading(guest))
            greyAndHide(guest, text);
    }
    greyCreditsInNetworkRace(guest, menu, kGraphicsCredits);
    return 0;
}

/* sub_4484e0, graphics as it closes: Cancel puts the settings back; anything
 * else takes the mode chosen. */
x86::reg32 onGraphicsExit(Guest& guest, x86::reg32 menu)
{
    if (leftBy(guest, menu, kGraphicsCancel))
    {
        copyDwords(guest, kGraphicsSettings, kGraphicsKept, kGraphicsSettingsDwords);
        return 0;
    }
    guest.call(kListModes);
    const x86::reg32 chosen = kModes + kModeSize * guest.dword(kScreenMode);
    // port: Dynamic Fullscreen kept as its marker, its width being the screen's
    guest.setDword(kScreenModeValue, dynamicFullscreenMode(guest.dword(chosen)) ? kDynamicFullscreenWidth
                                                                                : guest.dword(chosen + 4));
    return 0;
}

/* sub_448610, advgrph as it opens: what the display and the mode cannot do
 * greyed out. */
x86::reg32 onAdvancedEnter(Guest& guest, x86::reg32 menu)
{
    copyDwords(guest, kGraphicsKept, kGraphicsSettings, kGraphicsSettingsDwords);
    const x86::reg32 mode = guest.frame();   // a mode's 0x114 bytes
    guest.setEsi(mode);
    guest.call(kCurrentMode, fe::args({ menu, kGraphicsCredits.address }));
    greyCreditsInNetworkRace(guest, menu, kGraphicsCredits);
    if (Item german = fe::findItem(guest, menu, kGermanOnly))
        german.setHidden(guest.dword(kLanguage) != 1);
    if (!displayShading(guest))
    {
        if (Item item = fe::findItem(guest, menu, kPerspectiveList))
            greyAndHide(guest, item);
        if (Item item = fe::findItem(guest, menu, kPerspective))
            item.setDisabled(true);
        if (Item item = fe::findItem(guest, menu, kTrackLightingList))
            greyAndHide(guest, item);
        if (Item item = fe::findItem(guest, menu, kTrackLighting))
            item.setDisabled(true);
    }
    guest.setDword(kDepthBufferItem, fe::findItem(guest, menu, kDepthBuffer).address());
    Item text = fe::findItem(guest, menu, kDepthBufferTextCodelink);
    guest.setDword(kDepthBufferText, text.address());
    Item depth(guest, guest.dword(kDepthBufferItem));
    if (depth && text)
    {
        if (guest.dword(mode + kModeHasDepthBuffer) == 0)
        {
            depth.setDisabled(true);
            greyAndHide(guest, text);
        }
        else
        {
            depth.setDisabled(false);
            showAgain(guest, text);
        }
    }
    if (Item intensity = fe::findItem(guest, menu, kEffectsIntensityCodelink))
    {
        /* Port: the original greys the slider out unless kDisplayFlags has
         * 0x40 -- 1 << the driver's type, and 0x40 is Direct3D's.
         * The port's renderer scales the effects' colours by it on any
         * driver, so the slider is always there (tools/apply_menu_unlock.py
         * does the same to the generated code; apply_alpha_intensity.py makes
         * the value count). */
        guest.setFloat(intensity.address() + kSliderValueField,
                       float(x86::Float(guest.floatAt(kEffectsIntensity))));
        guest.call(kSliderSetOnMove, fe::args({ intensity.address(), kEffectsIntensityMoved }));
    }
    return 0;
}

/* sub_4485a0, advgrph on every pass: projected text only with the buffering
 * that allows it and the setting on. */
x86::reg32 onAdvancedFrame(Guest& guest, x86::reg32 menu)
{
    auto allowed = [&]() {
        return guest.dword(kDisplayBuffering) != 0 && guest.dword(kProjectedText) != 0;
    };
    if (Item button = fe::findItem(guest, menu, kProjectedTextButton))
        button.setDisabled(!allowed());
    if (Item list = fe::findItem(guest, menu, kProjectedTextList))
    {
        if (allowed())
            showAgain(guest, list);
        else
            greyAndHide(guest, list);
    }
    return 0;
}

/* sub_448770, advgrph as it closes: Cancel puts the settings back. */
x86::reg32 onAdvancedExit(Guest& guest, x86::reg32 menu)
{
    if (leftBy(guest, menu, kAdvancedCancel))
        copyDwords(guest, kGraphicsSettings, kGraphicsKept, kGraphicsSettingsDwords);
    return 0;
}

/* sub_448570, the effects' intensity moved, while it is shown. */
x86::reg32 onEffectsIntensityMoved(Guest& guest, x86::reg32 item)
{
    if (guest.call(kShownNow, fe::args({ item, 0xf })) == 0)
        return 0;
    guest.setFloat(kEffectsIntensity, float(sliderValue(guest, item, 0)));
    return guest.eax();
}

/* ---------------------------------------------------------------------------
 * The strings each stand-in hands to the game
 * ------------------------------------------------------------------------- */

const GuestString kOptionsStrings[] = { kAudioButton, kLoadButton, kOptionsCredits };
const GuestString kFeedEnterStrings[] = {
    kFeedCredits,  kFeedTimeButton, kFeedTimeList,  kFeedPositionButton,
    kFeedPositionList, kFeedSpeedButton, kFeedSpeedList,
};
const GuestString kFeedExitStrings[] = { kFeedCancel };
const GuestString kKeysStrings[] = {
    kKeysCredits,  kKeyTexts[0], kKeyTexts[1], kKeyTexts[2], kKeyTexts[3],
    kKeyTexts[4],  kKeyTexts[5], kKeyTexts[6], kKeyTexts[7], kKeyTexts[8],
};
const GuestString kAudioEnterStrings[] = {
    kAudioCredits, kMenuMusicCodelink, kRaceMusicCodelink, kEffectsCodelink, kEngineCodelink, kSpeechCodelink,
};
const GuestString kAudioExitStrings[] = { kAudioCancel };
const GuestString kControlsStrings[] = {
    kControlsCancel, kJoysFile, kPathFormat, kJoyShapeFormat, kForceFeedbackButton, kDeadZoneButton, kResetButton,
};
const GuestString kDeadZoneStrings[] = {
    kDeadZoneCredits,  kDeadZoneCancel,   kAxisCodelinks[0], kAxisCodelinks[1], kAxisCodelinks[2],
    kAxisCodelinks[3], kAxisCodelinks[4], kAxisCodelinks[5], kAxisCodelinks[6], kAxisCodelinks[7],
    kControllerChoice,
};
const GuestString kShowControllerStrings[] = { kHiddenItems[0], kHiddenItems[1], kHiddenItems[2], kHiddenItems[3] };
const GuestString kForceStrings[] = {
    kForceCredits,      kForceCodelinks[0], kForceCodelinks[1], kForceCodelinks[2],
    kForceCodelinks[3], kForceCodelinks[4], kForceCodelinks[5], kForceCancel,
};
const GuestString kGraphicsEnterStrings[] = { kCarShade, kCarShadeText, kGraphicsCredits };
const GuestString kGraphicsExitStrings[] = { kGraphicsCancel };
const GuestString kAdvancedEnterStrings[] = {
    kGraphicsCredits, kGermanOnly,  kPerspectiveList, kPerspective,     kTrackLightingList,
    kTrackLighting,   kDepthBuffer, kDepthBufferTextCodelink, kEffectsIntensityCodelink,
};
const GuestString kAdvancedFrameStrings[] = { kProjectedTextButton, kProjectedTextList };
const GuestString kAdvancedExitStrings[] = { kAdvancedCancel };

constexpr std::uint8_t kRestoreNone = 0;
constexpr std::uint8_t kEcxEdx = fe::kRestoreEcx | fe::kRestoreEdx;
constexpr std::uint8_t kAllButEbx = fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi;
constexpr std::uint8_t kAllButEdx = fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi;

/* A screen's handler: the screen in eax; an item's key handler: the item in eax,
 * the screen in edx, the key in bx; a slider's function: the item in eax, and for
 * a drawing one edx. */
template <typename Body>
void wholeForMenu(win32::WinApplication* app, x86::CPU& cpu, fe::Site& site, Body body)
{
    const x86::reg32 menu = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return body(guest, menu); });
}

template <typename Body>
void wholeForKey(win32::WinApplication* app, x86::CPU& cpu, fe::Site& site, Body body)
{
    const x86::reg32 item = cpu.eax;
    const x86::reg32 menu = cpu.edx;
    const std::uint16_t key = std::uint16_t(cpu.ebx);
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return body(guest, item, menu, key); });
}

template <typename Body>
void wholeForItem(win32::WinApplication* app, x86::CPU& cpu, fe::Site& site, Body body)
{
    const x86::reg32 item = cpu.eax;
    const x86::reg32 edx = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return body(guest, item, edx); });
}

}

/* Each stand-in: the generated function, what its ret pops, what its prologue
 * pushes, the strings it hands to the game and the registers it pushes. */

void frontend::optionsOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_442aa0", 0, 12, kOptionsStrings, std::size(kOptionsStrings), kEcxEdx };
    wholeForMenu(app, cpu, site, onOptionsEnter);
}

void frontend::feedOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4374b0", 0, 12, kFeedEnterStrings, std::size(kFeedEnterStrings), kEcxEdx };
    wholeForMenu(app, cpu, site, onFeedEnter);
}

void frontend::feedOnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_437590", 0, 8, kFeedExitStrings, std::size(kFeedExitStrings), fe::kRestoreEdx };
    wholeForMenu(app, cpu, site, onFeedExit);
}

void frontend::feedSeparate(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_437250", 0, 16, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEdx | fe::kRestoreEdi };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return separateFeeds(guest); });
}

namespace
{
constexpr std::uint8_t kButtonKeyRestores = fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi;
}

void frontend::feedTimeButtonKey(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_437300", 0, 16, nullptr, 0, kButtonKeyRestores };
    wholeForKey(app, cpu, site, [](Guest& g, x86::reg32 i, x86::reg32 m, std::uint16_t k) {
        return onFeedButtonKey(g, i, m, k, kFeedTime, kFeedPosition, kFeedSpeed);
    });
}

void frontend::feedPositionButtonKey(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_437390", 0, 16, nullptr, 0, kButtonKeyRestores };
    wholeForKey(app, cpu, site, [](Guest& g, x86::reg32 i, x86::reg32 m, std::uint16_t k) {
        return onFeedButtonKey(g, i, m, k, kFeedPosition, kFeedTime, kFeedSpeed);
    });
}

void frontend::feedSpeedButtonKey(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_437420", 0, 16, nullptr, 0, kButtonKeyRestores };
    wholeForKey(app, cpu, site, [](Guest& g, x86::reg32 i, x86::reg32 m, std::uint16_t k) {
        return onFeedButtonKey(g, i, m, k, kFeedSpeed, kFeedTime, kFeedPosition);
    });
}

void frontend::feedTimeListKey(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_437350", 0, 8, nullptr, 0, fe::kRestoreEcx };
    wholeForKey(app, cpu, site, [](Guest& g, x86::reg32 i, x86::reg32 m, std::uint16_t k) {
        return onFeedListKey(g, i, m, k, kFeedTime, kFeedPosition, kFeedSpeed);
    });
}

void frontend::feedPositionListKey(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4373e0", 0, 8, nullptr, 0, fe::kRestoreEcx };
    wholeForKey(app, cpu, site, [](Guest& g, x86::reg32 i, x86::reg32 m, std::uint16_t k) {
        return onFeedListKey(g, i, m, k, kFeedPosition, kFeedTime, kFeedSpeed);
    });
}

void frontend::feedSpeedListKey(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_437470", 0, 8, nullptr, 0, fe::kRestoreEcx };
    wholeForKey(app, cpu, site, [](Guest& g, x86::reg32 i, x86::reg32 m, std::uint16_t k) {
        return onFeedListKey(g, i, m, k, kFeedSpeed, kFeedTime, kFeedPosition);
    });
}

void frontend::audioOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_437e00", 0, 36, kAudioEnterStrings, std::size(kAudioEnterStrings), kAllButEbx };
    wholeForMenu(app, cpu, site, onAudioEnter);
}

void frontend::audioOnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_438070", 0, 24, kAudioExitStrings, std::size(kAudioExitStrings) };
    wholeForMenu(app, cpu, site, onAudioExit);
}

void frontend::audioOnFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_438260", 0, 24, nullptr, 0 };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return onAudioFrame(guest); });
}

void frontend::audioVolumeDraw(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_437a90", 0, 20, nullptr, 0, kAllButEdx };
    wholeForItem(app, cpu, site, onVolumeSliderDraw);
}

void frontend::audioMenuMusicMoved(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_437990", 0, 8, nullptr, 0, fe::kRestoreEdx };
    wholeForItem(app, cpu, site, [](Guest& g, x86::reg32 item, x86::reg32) {
        return onVolumeMoved(g, item, kMenuMusicVolume, 0);
    });
}

void frontend::audioRaceMusicMoved(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4379b0", 0, 8, nullptr, 0, fe::kRestoreEdx };
    wholeForItem(app, cpu, site, [](Guest& g, x86::reg32 item, x86::reg32) {
        return onVolumeMoved(g, item, kRaceMusicVolume, 0);
    });
}

void frontend::audioEffectsMoved(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4379d0", 0, 12, nullptr, 0, fe::kRestoreEdx };
    wholeForItem(app, cpu, site, [](Guest& g, x86::reg32 item, x86::reg32) {
        return onVolumeMoved(g, item, kEffectsVolume, kRaceEffectsVolume);
    });
}

void frontend::audioEngineMoved(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_437a20", 0, 12, nullptr, 0, fe::kRestoreEdx };
    wholeForItem(app, cpu, site, [](Guest& g, x86::reg32 item, x86::reg32) {
        return onVolumeMoved(g, item, kEngineVolume, kRaceEngineVolume);
    });
}

void frontend::audioSpeechMoved(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_437a70", 0, 8, nullptr, 0, fe::kRestoreEdx };
    wholeForItem(app, cpu, site, [](Guest& g, x86::reg32 item, x86::reg32) {
        return onVolumeMoved(g, item, kSpeechVolume, 0);
    });
}

void frontend::graphicsOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_448440", 0, 24, kGraphicsEnterStrings, std::size(kGraphicsEnterStrings) };
    wholeForMenu(app, cpu, site, onGraphicsEnter);
}

void frontend::graphicsOnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4484e0", 0, 20, kGraphicsExitStrings, std::size(kGraphicsExitStrings), kAllButEbx };
    wholeForMenu(app, cpu, site, onGraphicsExit);
}

void frontend::graphicsOnFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    // sub_448430: nothing.
    static fe::Site site{ "sub_448430", 0, 4, nullptr, 0, kRestoreNone };
    fe::runWhole(app, cpu, site, [](Guest&) { return x86::reg32(0); });
}

void frontend::advancedOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_448610", 0, 0x12c, kAdvancedEnterStrings, std::size(kAdvancedEnterStrings) };
    wholeForMenu(app, cpu, site, onAdvancedEnter);
}

void frontend::advancedOnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_448770", 0, 20, kAdvancedExitStrings, std::size(kAdvancedExitStrings), kAllButEbx };
    wholeForMenu(app, cpu, site, onAdvancedExit);
}

void frontend::advancedOnFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4485a0", 0, 12, kAdvancedFrameStrings, std::size(kAdvancedFrameStrings), kEcxEdx };
    wholeForMenu(app, cpu, site, onAdvancedFrame);
}

void frontend::advancedIntensityMoved(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_448570", 0, 12, nullptr, 0, kEcxEdx };
    wholeForItem(app, cpu, site,
                      [](Guest& g, x86::reg32 item, x86::reg32) { return onEffectsIntensityMoved(g, item); });
}

/* The controls' screens, native whole, no generated code behind them
 * (disassemble_nfs3hp.py's MENU_NATIVES): the function keys' and the
 * hardware keys', the controllers' (control1), their dead zones and their
 * force feedback. */
namespace frontend
{

void funkeysOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_437600", 0, 12, kKeysStrings, std::size(kKeysStrings), kEcxEdx };
    wholeForMenu(app, cpu, site, onKeysEnter);
}

void funkeysOnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    // sub_4377c0: nothing to undo.
    static fe::Site site{ "sub_4377c0", 0, 4, nullptr, 0, kRestoreNone };
    fe::runWhole(app, cpu, site, [](Guest&) { return x86::reg32(0); });
}

void hardkeysOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    // sub_4377d0: funkeys', instruction for instruction.
    static fe::Site site{ "sub_4377d0", 0, 12, kKeysStrings, std::size(kKeysStrings), kEcxEdx };
    wholeForMenu(app, cpu, site, onKeysEnter);
}

void controlsOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_43d4d0", 0, 0x64, kControlsStrings, std::size(kControlsStrings),
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    wholeForMenu(app, cpu, site, onControlsEnter);
}

void controlsOnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_43d5f0", 0, 16, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return onControlsExit(guest); });
}

void controlsCancelKey(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_43d3f0", 0, 4, nullptr, 0, kRestoreNone };
    wholeForKey(app, cpu, site, [](Guest& g, x86::reg32 i, x86::reg32 m, std::uint16_t k) {
        return onCancelKey(g, i, m, k, kControlsCancelled, 1);
    });
}

void controlsResetClicked(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_43d460", 0, 16, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return onResetClicked(guest); });
}

void controlsResetAnswered(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_43d410", 0, 12, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx };
    const std::uint32_t answer = cpu.eax;
    const x86::reg32 close = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return onResetAnswered(guest, answer, close); });
}

void deadZoneFillList(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_43d9a0", 0, 16, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return fillControllerList(guest); });
}

void deadZoneOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4432c0", 0, 20, kDeadZoneStrings, std::size(kDeadZoneStrings),
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    wholeForMenu(app, cpu, site, onDeadZoneEnter);
}

void deadZoneOnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_443430", 0, 28, nullptr, 0 };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return onDeadZoneExit(guest); });
}

void deadZoneCancelKey(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_442db0", 0, 8, nullptr, 0, fe::kRestoreEcx };
    wholeForKey(app, cpu, site, [](Guest& g, x86::reg32 i, x86::reg32 m, std::uint16_t k) {
        return onCancelKey(g, i, m, k, kDeadZoneKept, 0);
    });
}

void deadZoneChooseKey(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_443260", 0, 8, nullptr, 0, fe::kRestoreEcx };
    wholeForKey(app, cpu, site, [](Guest& g, x86::reg32 i, x86::reg32 m, std::uint16_t k) {
        return onChooseControllerKey(g, kListKey, i, m, k);
    });
}

void deadZoneChooseKeyInRace(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_443290", 0, 8, nullptr, 0, fe::kRestoreEcx };
    wholeForKey(app, cpu, site, [](Guest& g, x86::reg32 i, x86::reg32 m, std::uint16_t k) {
        return onChooseControllerKey(g, kButtonKey, i, m, k);
    });
}

void deadZoneShowController(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_443070", 0, 28, kShowControllerStrings, std::size(kShowControllerStrings),
                          kAllButEdx };
    const x86::reg32 menu = cpu.eax;
    const x86::reg32 controller = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return showController(guest, menu, controller); });
}

void deadZoneAxisIndex(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_442e60", 0, 8, nullptr, 0, fe::kRestoreEdx };
    wholeForItem(app, cpu, site, [](Guest& g, x86::reg32 item, x86::reg32) { return axisIndex(g, item); });
}

void deadZoneApplyAxis(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_442dd0", 16, 32, nullptr, 0 };
    Guest arguments(app, cpu);
    const float first = arguments.floatAt(cpu.esp + 4);
    const float second = arguments.floatAt(cpu.esp + 8);
    const x86::reg32 controller = arguments.dword(cpu.esp + 12);
    const x86::reg32 axis = arguments.dword(cpu.esp + 16);
    fe::runWhole(app, cpu, site,
                   [&](Guest& guest) { return applyAxis(guest, first, second, controller, axis); });
}

void deadZoneAxisMoved(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_442ef0", 0, 40, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    wholeForItem(app, cpu, site, [](Guest& g, x86::reg32 item, x86::reg32) { return onAxisMoved(g, item); });
}

void deadZoneAxisDraw(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_442ff0", 0, 24, nullptr, 0, kAllButEdx };
    wholeForItem(app, cpu, site, onAxisDraw);
}

void forceOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_447dd0", 0, 32, kForceStrings, std::size(kForceStrings) };
    wholeForMenu(app, cpu, site, onForceEnter);
}

void forceOnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4481c0", 0, 12, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return onForceExit(guest); });
}

void forceCancelKey(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_447b40", 0, 4, nullptr, 0, kRestoreNone };
    wholeForKey(app, cpu, site, [](Guest& g, x86::reg32 i, x86::reg32 m, std::uint16_t k) {
        return onCancelKey(g, i, m, k, kForceCancelled, 1);
    });
}

void forceStrengthMoved(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_447b60", 0, 36, nullptr, 0 };
    wholeForItem(app, cpu, site, [](Guest& g, x86::reg32 item, x86::reg32) { return onStrengthMoved(g, item); });
}

void forceRoadEffectsMoved(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_447c40", 0, 12, nullptr, 0, fe::kRestoreEdx };
    wholeForItem(app, cpu, site, [](Guest& g, x86::reg32 item, x86::reg32) {
        return onForceMoved(g, item, kRoadEffectsForce);
    });
}

void forceCollisionMoved(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_447c70", 0, 12, nullptr, 0, fe::kRestoreEdx };
    wholeForItem(app, cpu, site, [](Guest& g, x86::reg32 item, x86::reg32) {
        return onForceMoved(g, item, kCollisionForce);
    });
}

void forceEngineMoved(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_447ca0", 0, 12, nullptr, 0, fe::kRestoreEdx };
    wholeForItem(app, cpu, site, [](Guest& g, x86::reg32 item, x86::reg32) {
        return onForceMoved(g, item, kEngineForce);
    });
}

void forceBodyRollMoved(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_447cd0", 0, 16, nullptr, 0, kEcxEdx };
    wholeForItem(app, cpu, site, [](Guest& g, x86::reg32 item, x86::reg32) {
        return onExclusiveForceMoved(g, item, kBodyRollForce, kRoadGripForce, kRoadGripSlider);
    });
}

void forceRoadGripMoved(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_447d50", 0, 16, nullptr, 0, kEcxEdx };
    wholeForItem(app, cpu, site, [](Guest& g, x86::reg32 item, x86::reg32) {
        return onExclusiveForceMoved(g, item, kRoadGripForce, kBodyRollForce, kBodyRollSlider);
    });
}

}  // namespace frontend

}
