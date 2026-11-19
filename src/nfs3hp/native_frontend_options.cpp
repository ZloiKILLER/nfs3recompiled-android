#include "native_frontend_options.h"
#include "native_aiplan.h"
#include "native_watcom.h"
#include "native_game.h"
#include "game_settings.h"
#include "native_controls_devices.h"
#include "native_controls_input.h"
#include "native_ea_memory.h"
#include "native_frontend.h"
#include "native_frontend_dialogs.h"
#include "native_frontend_glue.h"
#include "native_frontend_loop.h"
#include "native_frontend_menua.h"
#include "port_buttons.h"
#include "port_dialogs.h"
#include "port_fields.h"
#include "port_loop.h"
#include "port_menus.h"
#include "port_shapes.h"
#include "port_snd_stream_calls.h"
#include "port_snd_voices.h"
#include "port_sound_effects.h"
#include "port_sound_music.h"
#include "port_sound_racesetup.h"
#include "port_text.h"
#include "dynamic_fullscreen.h"
#include "native_graphics.h"
#include <cstring>
#include <iterator>
#include <utility>
#include "native_guest.h"

namespace nfs3hp
{
namespace
{

/* A call through a pointer or to a function of the game's that has no typed core: Watcom's register
 * arguments (eax, edx, ebx, ecx in turn) and the stack words as `arguments` have them, through
 * guest::pointerCall; its eax, and Terminated when the game stopped in it. */
nfs::u32 callAt(fe::Guest& guest, nfs::u32 target, const fe::Arguments& arguments = fe::Arguments())
{
    nfs::ThreadState& ts = guest.ts();
    nfs::Args registers;
    nfs::u32* const into[4] = { &registers.eax, &registers.edx, &registers.ebx, &registers.ecx };
    for (std::size_t i = 0; i < arguments.registerCount; ++i)
        *into[i] = arguments.registers[i];
    const nfs::u32 esp = ts.sp;
    for (std::size_t i = arguments.stackCount; i-- > 0;)
    {
        ts.sp -= 4;
        guest.setDword(ts.sp, arguments.stack[i]);
    }
    const std::optional<nfs::u32> result = guest::pointerCall(guest.app(), ts, target, registers);
    if (!result)
        throw fe::Terminated();
    ts.sp = esp;
    return *result;
}

}  // namespace
}  // namespace nfs3hp

namespace nfs3hp
{
namespace
{

/* The game's float operations on plain values: fild, the single-precision sum, difference and
 * product (fp::), what jb and ja read after fcom, and the truncation of sub_4dfd56 and fistp. */
struct FloatMath
{
    explicit FloatMath(fe::Guest& guest) : m_guest(guest) {}

    static nfs::Float integer(std::int32_t value) { return nfs::Float(nfs::s32(value)); }
    nfs::Float load(nfs::u32 address) const { return nfs::Float(m_guest.floatAt(address)); }
    nfs::Float add(const nfs::Float& a, const nfs::Float& b) { return fp::add(a, b); }
    nfs::Float sub(const nfs::Float& a, const nfs::Float& b) { return fp::sub(a, b); }
    nfs::Float mul(const nfs::Float& a, const nfs::Float& b) { return fp::mul(a, b); }
    bool above(const nfs::Float& a, const nfs::Float& b) { return a > b; }
    bool below(const nfs::Float& a, const nfs::Float& b) { return !(a >= b); }  // or unordered
    std::int32_t truncated(const nfs::Float& value)
    {
        const double chopped = fe::direct<0x4dfd56, watcom::clib::chop>(m_guest.app(), m_guest.ts(), double(value));
        return fp::toInteger<nfs::s32>(chopped);
    }

private:
    fe::Guest& m_guest;
};

}  // namespace
}  // namespace nfs3hp

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
 * game's code, called by address.
 *
 * Plan step 5: each function a typed core in frontend::options (native_frontend_options.h), in a
 * frame of its own where the game's function has one (fe::Frame: what it calls runs at the stack
 * addresses it did), and a thunk under the game's name for the calls by address.  A core of
 * this file calls another as C++ (guest::direct); the rest of the game still by address.  Where
 * several of the game's functions share a body, the body stays a helper and each has a core. */
namespace nfs3hp
{

namespace
{

using fe::Guest;
using fe::GuestString;
using fe::Item;
using App = nfs::App;
using ThreadState = nfs::ThreadState;

/* ---------------------------------------------------------------------------
 * What the screens share
 * ------------------------------------------------------------------------- */

constexpr nfs::u32 kInRace = 0x559234;          // the menus are a race's pause screens
// The rows a screen's items stand in, by their ypos: x and y, int16 every four
// bytes (see native_frontend_main.cpp).
constexpr nfs::u32 kRowX = 0x559c6c;
constexpr nfs::u32 kRowY = 0x559c94;

// The menu engine's and the widgets' functions.
constexpr nfs::u32 kShownNow = 0x44a660;         // an item is shown on this pass
constexpr nfs::u32 kDialogShown = 0x4438b0;
constexpr nfs::u32 kButtonKey = 0x461270;        // a button's own key handler
constexpr nfs::u32 kCascadeKey = 0x4624a0;       // a list's that pops up in place
constexpr nfs::u32 kListKey = 0x463ff0;          // a list's that is always open
// A slider (widget 0x46c...): its value, 0 to 1, by the index of its end (two
// ends for a range); the function its moves go to; how it draws.
constexpr nfs::u32 kSliderDraw = 0x46cae0;       // eax the item, edx 1 when selected
constexpr nfs::u32 kSliderOnMove = 0x78;
constexpr nfs::u32 kOnDraw = 0x2c;
constexpr nfs::u32 kOnKey = 0x30;
constexpr std::uint32_t kButton = 5;               // the widget's type, the item's first dword
constexpr std::uint16_t kKeyEnter = 0x0d;

// The game's.
constexpr std::uint32_t kFloat0_6 = 0x3f19999a;    // 0.6f
// Zeroes edx bytes at eax.
constexpr nfs::u32 kFillZero = 0x4e070c;
constexpr nfs::u32 kSprintf = 0x4df690;

/* A screen's selected item: its index from the screen's first, in the table
 * of items. */
nfs::u32 selectedItem(const Guest& guest, nfs::u32 menu)
{
    const std::int32_t first = std::int16_t(guest.word(menu + fe::menu::kFirstItem));
    const std::int32_t selected = std::int16_t(guest.word(menu + fe::menu::kSelected));
    return fe::item::kTable + fe::item::kSize * nfs::u32(first + selected);
}

/* As a screen closes: whether its selected item is the button `codelink`, the
 * one that left it. */
bool leftBy(Guest& guest, ThreadState& ts, nfs::u32 menu, const GuestString& codelink)
{
    const nfs::u32 item = selectedItem(guest, menu);
    return guest.dword(item) == kButton
           && *fe::direct<0x4429c0, port::menus::codelinkMatched>(guest.app(), ts, ts.sp - 4,
                                                                  guest.dword(item + fe::item::kCodelink),
                                                                  codelink.address, 0)
                  != 0;
}

bool inRace(const Guest& guest)
{
    return guest.dword(kInRace) != 0;
}

bool networkRace(const Guest& guest)
{
    return std::int32_t(gameSettings(guest).playMode) > 1;
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
void copyDwords(Guest& guest, nfs::u32 to, nfs::u32 from, std::uint32_t count)
{
    for (std::uint32_t i = 0; i < count; ++i)
        guest.setDword(to + 4 * i, guest.dword(from + 4 * i));
}

/* A key for an item, handed on to the widget's own handler: eax the item, edx
 * the screen, ebx the key, as the menu loop hands them. */
nfs::u32 passKey(Guest& guest, nfs::u32 handler, nfs::u32 item, nfs::u32 menu, std::uint16_t key)
{
    return callAt(guest, handler, fe::args({ item, menu, nfs::u32(std::int32_t(std::int16_t(key))) }));
}

/* A slider's value, as 0x46c9b0 gives it. */
nfs::Float readSliderValue(Guest& guest, ThreadState& ts, nfs::u32 item, nfs::u32 end)
{
    return fe::direct<0x46c9b0, frontend::glue::sliderValue>(guest.app(), ts, item, end);
}

/* A float as the dword pushed for it. */
nfs::u32 floatBits(float value)
{
    nfs::u32 bits;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}

/* A slider set to the float `value` (a two-axis one's end `end`), which 0x46c950 takes pushed. */
void setSliderValue(Guest& guest, ThreadState& ts, nfs::u32 item, float value, nfs::u32 end)
{
    fe::direct<0x46c950, port::fields::sliderSetValueOf>(guest.app(), ts, ts.sp - 12, item, floatBits(value),
                                                         end);
}

/* The function a slider's moves go to, 0 for none. */
void setSliderOnMove(Guest& guest, ThreadState& ts, nfs::u32 item, nfs::u32 function)
{
    fe::direct<0x46cac0, frontend::glue::sliderSetOnMove>(guest.app(), ts, item, function);
}

/* The screen behind dimmed for a race's pause screen: the shade pushed where the game's code pushes
 * it, under the frame; eax handed on as it is. */
void dimBehind(Guest& guest, ThreadState& ts)
{
    const nfs::u32 slot = guest.frame() - 4;
    guest.setDword(slot, kFloat0_6);
    fe::direct<0x451530, port::text::dimmed>(guest.app(), ts, slot, 0);  // eax: its result is not read
}

/* Whether an item is shown (the 0xf the game's code hands it in edx it does not read). */
nfs::u32 itemShown(Guest& guest, ThreadState& ts, nfs::u32 item)
{
    return fe::direct<0x44a660, frontend::loop::shownNow>(guest.app(), ts, item);
}

/* `count` bytes from `from` to `to`, memmove's. */
void copyBytes(Guest& guest, ThreadState& ts, nfs::u32 to, nfs::u32 from, nfs::u32 count)
{
    fe::direct<0x4ea4f0, ea::moveMemory>(guest.app(), ts, to, from, count);
}

/* The x87's compare and what je reads after it: equal, or unordered. */
bool same(Guest& guest, const nfs::Float& a, const nfs::Float& b)
{
    return !(a < b) && !(a > b);
}

/* The credits button of a screen, greyed out in a race over a network. */
void greyCreditsInNetworkRace(Guest& guest, nfs::u32 menu, const GuestString& credits)
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

}  // namespace

/* sub_442aa0, options as it opens: no sound screen without sound, and in a
 * race over a network neither loading nor the credits. */
nfs::u32 frontend::options::onOptionsEnter(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    if (Item audio = fe::findItem(guest, menu, kAudioButton))
    {
        if (gameSettings(guest).installIsRemote != 0)
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

namespace
{

/* ---------------------------------------------------------------------------
 * audfeed: the audio feed's keys
 * ------------------------------------------------------------------------- */

/* Three keys, one of ten each (0 to 9), never two on the same: the time's,
 * the position's and the speed's.  A button and a list for each, B and C. */
constexpr std::int32_t kFeedKeys = 10;
// What they were as the screen opened, for its Cancel.
constexpr nfs::u32 kFeedTimeKept = 0x5f3914;
constexpr nfs::u32 kFeedPositionKept = 0x5f3910;
constexpr nfs::u32 kFeedSpeedKept = 0x5f3918;

constexpr GuestString kFeedCredits{ 0x537494, "credits" };
constexpr GuestString kFeedTimeButton{ 0x53749c, "AF_BFEEDTIME" };
constexpr GuestString kFeedTimeList{ 0x5374ac, "AF_CFEEDTIME" };
constexpr GuestString kFeedPositionButton{ 0x5374bc, "AF_BPOSITION" };
constexpr GuestString kFeedPositionList{ 0x5374cc, "AF_CPOSITION" };
constexpr GuestString kFeedSpeedButton{ 0x5374dc, "AF_BSPEED" };
constexpr GuestString kFeedSpeedList{ 0x5374e8, "AF_CSPEED" };
constexpr GuestString kFeedCancel{ 0x5374f4, "CANCEL_AUDFEED_OPTIONS" };

// The key handlers the screen hands its items.
constexpr nfs::u32 kFeedTimeButtonKey = 0x437300;
constexpr nfs::u32 kFeedTimeListKey = 0x437350;
constexpr nfs::u32 kFeedPositionButtonKey = 0x437390;
constexpr nfs::u32 kFeedPositionListKey = 0x4373e0;
constexpr nfs::u32 kFeedSpeedButtonKey = 0x437420;
constexpr nfs::u32 kFeedSpeedListKey = 0x437470;

/* A feed's key moved on, round from 9 to 0, until no other feed has it. */
void moveOff(nfs::u32& feed, const nfs::u32& other1, const nfs::u32& other2)
{
    while (feed == other1 || feed == other2)
    {
        const std::int32_t next = std::int32_t(feed) + 1;
        feed = nfs::u32(next);
        if (next > kFeedKeys - 1)
            feed = 0;
    }
}

}  // namespace

/* sub_437250: the three feeds on keys of their own, the time first. */
nfs::u32 frontend::options::separateFeeds(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    nfs::u32* const feeds = gameSettings(guest).feedKeys;
    moveOff(feeds[0], feeds[1], feeds[2]);
    moveOff(feeds[1], feeds[0], feeds[2]);
    moveOff(feeds[2], feeds[0], feeds[1]);
    return gameSettings(guest).feedKeys[2];
}

namespace
{

/* sub_437300, sub_437390, sub_437420: a feed's button.  Enter moves it to the
 * next key, and on until no other feed has that one. */
nfs::u32 onFeedButtonKey(Guest& guest, nfs::u32 item, nfs::u32 menu, std::uint16_t key, const nfs::u32& feed,
                           const nfs::u32& other1, const nfs::u32& other2)
{
    if (key != kKeyEnter)
        return passKey(guest, kButtonKey, item, menu, key);
    nfs::u32 result;
    do
        result = passKey(guest, kButtonKey, item, menu, key);
    while (feed == other1 || feed == other2);
    return result;
}

/* sub_437350, sub_4373e0, sub_437470: a feed's list.  A key another feed has
 * is not taken: the feed stays where it was. */
nfs::u32 onFeedListKey(Guest& guest, nfs::u32 item, nfs::u32 menu, std::uint16_t key, nfs::u32& feed,
                         const nfs::u32& other1, const nfs::u32& other2)
{
    const nfs::u32 was = feed;
    const nfs::u32 result = passKey(guest, kCascadeKey, item, menu, key);
    const nfs::u32 now = feed;
    if (was != now && (now == other1 || now == other2))
        feed = was;
    return result;
}

}  // namespace

/* sub_4374b0, audfeed as it opens: the feeds kept for Cancel, each item its
 * key handler. */
nfs::u32 frontend::options::onFeedEnter(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    guest.setDword(kFeedTimeKept, gameSettings(guest).feedKeys[0]);
    guest.setDword(kFeedSpeedKept, gameSettings(guest).feedKeys[2]);
    guest.setDword(kFeedPositionKept, gameSettings(guest).feedKeys[1]);
    greyCreditsInNetworkRace(guest, menu, kFeedCredits);
    const struct
    {
        const GuestString& codelink;
        nfs::u32 handler;
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
nfs::u32 frontend::options::onFeedExit(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    if (leftBy(guest, ts, menu, kFeedCancel))
    {
        gameSettings(guest).feedKeys[0] = guest.dword(kFeedTimeKept);
        gameSettings(guest).feedKeys[2] = guest.dword(kFeedSpeedKept);
        gameSettings(guest).feedKeys[1] = guest.dword(kFeedPositionKept);
        return 0;
    }
    guest::direct<0x437250, separateFeeds>(app, ts);
    return 0;
}

namespace
{

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
constexpr nfs::u32 kFadeIn = 0x559c68;

/* sub_437600 (funkeys) and sub_4377d0 (hardkeys), alike as they open: the
 * lines of text placed in rows. */
nfs::u32 onKeysEnter(Guest& guest, nfs::u32 menu)
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
constexpr std::uint32_t kSoundSettingsDwords = 16;
// A race's own effects and engine volumes, what its pause screen moves.
constexpr nfs::u32 kRaceEffectsVolume = 0x552920;
constexpr nfs::u32 kRaceEngineVolume = 0x552924;
// Byte: the race's volumes go back to the settings as the screen closes.
constexpr nfs::u32 kRaceVolumesToSettings = 0x552909;
// Byte: the race's music is playing.
constexpr nfs::u32 kRaceMusicPlaying = 0x55e890;
// Word: cleared when the sound has no 3D.
constexpr nfs::u32 kSound3d = 0x55636a;

// What the screen keeps as it opens.
constexpr nfs::u32 kSoundSettingsKept = 0x5f3920;
constexpr nfs::u32 kSoundSetupKept = 0x5f3970;
constexpr nfs::u32 kSoundOutputKept = 0x5f3968;
constexpr nfs::u32 kSetting1ecKept = 0x5f3964;
constexpr nfs::u32 kRaceEffectsKept = 0x5f3960;
constexpr nfs::u32 kRaceEngineKept = 0x5f396c;
// Set when the output changed, to apply kSoundSetting1ec as the screen closes.
constexpr nfs::u32 kOutputChanged = 0x554f34;

/* The five sliders, their items found as the screen opens, and for each
 * whether it is held selected: while one is, the music plays at the volume it
 * moves, or an effect does. */
constexpr nfs::u32 kMenuMusicSlider = 0x554f3c;
constexpr nfs::u32 kRaceMusicSlider = 0x554f40;
constexpr nfs::u32 kEffectsSlider = 0x554f44;
constexpr nfs::u32 kEngineSlider = 0x554f48;
constexpr nfs::u32 kSpeechSlider = 0x554f4c;
constexpr nfs::u32 kMenuMusicHeld = 0x554f24;
constexpr nfs::u32 kRaceMusicHeld = 0x554f20;
constexpr nfs::u32 kEffectsHeld = 0x554f28;
constexpr nfs::u32 kEngineHeld = 0x554f2c;
constexpr nfs::u32 kSpeechHeld = 0x554f30;
// The effect playing for a slider, 0 for none.
constexpr nfs::u32 kSample = 0x554f38;

// EA's SND library and the game's sound.
constexpr nfs::u32 kEffectsBank = 0x571124;
constexpr std::uint32_t kSliderEffect = 7;
constexpr std::uint8_t kSliderEffectPitch = 0x40;
constexpr std::uint32_t kMusicStream = 0xa;
constexpr std::uint32_t kMenuMusicStream = 1;
constexpr nfs::u32 kHas3dSound = 0x40faf0;
constexpr nfs::u32 kApplySound = 0x4103a0;       // eax a setting, edx which
constexpr nfs::u32 kStartRaceMusic = 0x4107c0;
constexpr nfs::u32 kRestartSound = 0x40fcf0;
constexpr std::uint32_t kSoundRestartedText = 0x1e6;

// Around the sound's restart: the window taken, the picture flipped (by address).

// The sliders' own functions.
constexpr nfs::u32 kVolumeSliderDraw = 0x437a90;
constexpr nfs::u32 kMenuMusicMoved = 0x437990;
constexpr nfs::u32 kRaceMusicMoved = 0x4379b0;
constexpr nfs::u32 kEffectsMoved = 0x4379d0;
constexpr nfs::u32 kEngineMoved = 0x437a20;
constexpr nfs::u32 kSpeechMoved = 0x437a70;

constexpr double kVolumeToSlider = 1.0 / 127.0;    // the double at 0x537600
constexpr double kSliderToVolume = 127.0;          // at 0x537578 and after

constexpr GuestString kAudioCredits{ 0x5375a0, "credits" };
constexpr GuestString kMenuMusicCodelink{ 0x5375a8, "ASLD_MENUMUSIC" };
constexpr GuestString kRaceMusicCodelink{ 0x5375b8, "ASLD_RACEMUSIC" };
constexpr GuestString kEffectsCodelink{ 0x5375c8, "ASLD_SOUNDFX" };
constexpr GuestString kEngineCodelink{ 0x5375d8, "ASLD_ENGINEVOLUME" };
constexpr GuestString kSpeechCodelink{ 0x5375ec, "ASLD_SPEECHVOLUME" };
constexpr GuestString kAudioCancel{ 0x537608, "CANCEL_AUDIO_OPTIONS" };

/* A dword of the guest's that is not in the settings, where a helper takes either. */
nfs::u32& guestDword(const Guest& guest, nfs::u32 address)
{
    return *&guest.app()->getMemory<nfs::u32>(address);
}

/* The music's stream (SNDstreamautovol): its voice to `volume` over `ticks`. */
void streamVolume(Guest& guest, ThreadState& ts, nfs::u32 ticks, nfs::u32 volume)
{
    fe::direct<0x4e7470, port::stream::fadeStream>(guest.app(), ts, ts.sp - 4, std::int32_t(ticks),
                                                   std::int32_t(volume));
}

/* A volume slider found, kept, drawn by the screen's own function, set to the
 * volume and told where its moves go. */
void setUpVolumeSlider(Guest& guest, ThreadState& ts, nfs::u32 menu, const GuestString& codelink, nfs::u32 slot,
                       const nfs::u32& volume, nfs::u32 moved)
{
    const nfs::u32 item = fe::findItem(guest, menu, codelink).address();
    guest.setDword(slot, item);
    if (item == 0)
        return;
    guest.setDword(item + kOnDraw, kVolumeSliderDraw);
    FloatMath x87(guest);
    const float value =
        float(x87.mul(FloatMath::integer(std::int32_t(volume)), nfs::Float(kVolumeToSlider)));
    setSliderOnMove(guest, ts, item, moved);
    setSliderValue(guest, ts, guest.dword(slot), value, 0);
}

}  // namespace

/* sub_437e00, audio as it opens (psaudio too): the settings kept, the sliders
 * set to the volumes. */
nfs::u32 frontend::options::onAudioEnter(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 36);
    if (menu == 0)
        return 0;
    if (inRace(guest))
        dimBehind(guest, ts);
    if (callAt(guest, kHas3dSound) == 0)
        guest.setWord(kSound3d, 0);
    if (!inRace(guest))
        greyCreditsInNetworkRace(guest, menu, kAudioCredits);

    guest.setDword(kSoundSetupKept, gameSettings(guest).soundSetup);
    guest.setDword(kRaceMusicHeld, 0);
    guest.setDword(kMenuMusicHeld, 0);
    guest.setDword(kEffectsHeld, 0);
    guest.setDword(kSoundOutputKept, gameSettings(guest).musicMode);
    guest.setDword(kEngineHeld, 0);
    guest.setDword(kSetting1ecKept, gameSettings(guest).menuMusicAsked);
    guest.setDword(kSpeechHeld, 0);
    guest.setDword(kSample, 0);

    setUpVolumeSlider(guest, ts, menu, kMenuMusicCodelink, kMenuMusicSlider, gameSettings(guest).menuMusicVolume, kMenuMusicMoved);
    setUpVolumeSlider(guest, ts, menu, kRaceMusicCodelink, kRaceMusicSlider, gameSettings(guest).raceMusicVolume, kRaceMusicMoved);
    setUpVolumeSlider(guest, ts, menu, kEffectsCodelink, kEffectsSlider,
                      inRace(guest) ? guestDword(guest, kRaceEffectsVolume) : gameSettings(guest).effectsVolume, kEffectsMoved);
    setUpVolumeSlider(guest, ts, menu, kEngineCodelink, kEngineSlider,
                      inRace(guest) ? guestDword(guest, kRaceEngineVolume) : gameSettings(guest).engineVolume, kEngineMoved);
    setUpVolumeSlider(guest, ts, menu, kSpeechCodelink, kSpeechSlider, gameSettings(guest).speechVolume, kSpeechMoved);

    if (!inRace(guest))
        streamVolume(guest, ts, kMenuMusicStream, gameSettings(guest).menuMusicVolume);
    guest.setDword(kRaceEngineKept, guest.dword(kRaceEngineVolume));
    copyDwords(guest, kSoundSettingsKept, SETTINGS_ADDRESS(unknown090), kSoundSettingsDwords);
    guest.setDword(kRaceEffectsKept, guest.dword(kRaceEffectsVolume));
    return 0;
}

namespace
{

/* Fades the effect a slider plays, if any, and forgets it. */
void stopSample(Guest& guest, ThreadState& ts)
{
    fe::direct<0x4e9420, port::fadeSound>(guest.app(), ts, ts.sp - 4, guest.dword(kSample), 5, -1);
    guest.setDword(kSample, 0);
}

/* The output's change applied, if there was one, outside a race. */
void applyOutputChange(Guest& guest)
{
    if (!inRace(guest) && guest.dword(kOutputChanged) != 0)
    {
        guest.setDword(kOutputChanged, 0);
        callAt(guest, kApplySound, fe::args({ gameSettings(guest).menuMusicAsked, 1 }));
    }
}

}  // namespace

/* sub_438070, audio as it closes.  In a race the race's own volumes go back
 * to the settings, and its music starts if it was turned up from nothing.
 * Cancel puts back what the screen kept. */
nfs::u32 frontend::options::onAudioExit(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    if (!inRace(guest))
        streamVolume(guest, ts, kMusicStream, gameSettings(guest).menuMusicVolume);
    for (const nfs::u32 cleared : { kRaceMusicSlider, kEffectsSlider, kEngineSlider, kSpeechSlider, kRaceMusicHeld,
                                      kMenuMusicHeld, kEffectsHeld, kEngineHeld, kSpeechHeld, kMenuMusicSlider })
        guest.setDword(cleared, 0);

    if (inRace(guest))
    {
        if (guest.byte(kRaceVolumesToSettings) != 0)
        {
            gameSettings(guest).effectsVolume = guest.dword(kRaceEffectsVolume);
            gameSettings(guest).engineVolume = guest.dword(kRaceEngineVolume);
        }
        if (guest.byte(kRaceMusicPlaying) == 0)
        {
            const nfs::u32 volume = gameSettings(guest).raceMusicVolume;
            if (volume != 0)
            {
                const nfs::u32 otherForm = std::int32_t(gameSettings(guest).musicMode) > 2 ? 1 : 0;
                callAt(guest, kStartRaceMusic, fe::args({ gameSettings(guest).track, volume, otherForm }));
                guest.setByte(kRaceMusicPlaying, 1);
            }
        }
    }

    if (leftBy(guest, ts, menu, kAudioCancel))
    {
        const std::uint8_t toSettings = guest.byte(kRaceVolumesToSettings);
        guest.setDword(kRaceEffectsVolume, guest.dword(kRaceEffectsKept));
        copyDwords(guest, SETTINGS_ADDRESS(unknown090), kSoundSettingsKept, kSoundSettingsDwords);
        guest.setDword(kRaceEngineVolume, guest.dword(kRaceEngineKept));
        if (toSettings != 0)
        {
            gameSettings(guest).effectsVolume = guest.dword(kRaceEffectsVolume);
            gameSettings(guest).engineVolume = guest.dword(kRaceEngineVolume);
        }
        applyOutputChange(guest);
        stopSample(guest, ts);
        return 0;
    }
    stopSample(guest, ts);
    applyOutputChange(guest);
    return 0;
}

/* sub_438260, audio on every pass: a change of the setup starts the sound
 * over, a message up meanwhile; the output and the other setting are applied
 * as they change. */
nfs::u32 frontend::options::onAudioFrame(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    if (guest.dword(kSoundSetupKept) != gameSettings(guest).soundSetup)
    {
        guest.setDword(kSoundSetupKept, gameSettings(guest).soundSetup);
        fe::messageBox(guest, kSoundRestartedText);
        // The message drawn once before the sound stops, the screen under it.  What eax holds for
        // each call that hands it on is what the call before left there, as the game's code has it.
        const nfs::u32 esp = ts.sp - 4;
        fe::direct<0x445f00, frontend::dialogs::fadedIn>(app, ts);
        fe::direct<0x4d9120, graphics::windowTakenAndCleared>(app, ts);
        const nfs::u32 menu = fe::direct<0x44aeb0, port::loop::currentMenuOf>(app, ts);
        const nfs::u32 drawn = nfs::u32(*fe::direct<0x44a4f0, port::loop::drawItemsOf>(app, ts, esp, menu));
                // What it leaves in eax goes on -- its al, the rest of eax (drawItems's) as it came.
        const nfs::u32 shown = (drawn & ~0xffu) | guest::direct<kDialogShown, port::dialogs::dialogShownOf>(app, ts);
        if ((shown & 0xff) != 0)
            fe::direct<0x445f50, port::dialogs::dialogPassOf>(app, ts, esp, shown);
        const nfs::u32 afterRestart = *fe::direct<0x4d9140, graphics::flipAndWait>(app, ts);
        const nfs::u32 stopped = nfs::u32(*fe::direct<0x4109d0, port::musicStopped>(app, ts, esp, afterRestart));
        fe::direct<0x410aa0, port::musicStreamEndOf>(app, ts, esp, stopped, 0, 0);
        const nfs::u32 removed = nfs::u32(*fe::direct<0x410b20, port::banksRemoveOf>(app, ts, esp));
        fe::direct<0x40feb0, port::soundExitOf>(app, ts, esp, port::Args{ removed, 0, 0, 0 });
        fe::direct<kRestartSound, aiplan::restartLibrary>(guest.app(), guest.ts(), gameSettings(guest).soundSetup, 1);
        const nfs::u32 bank = nfs::u32(*fe::direct<0x4136e0, port::raceBankLoadOf>(app, ts, esp));
        fe::direct<0x410330, port::musicStreamMade>(app, ts, esp, bank, 0);
        const nfs::u32 applied = callAt(guest, kApplySound, fe::args({ gameSettings(guest).menuMusicAsked, 1 }));
        fe::direct<0x445f10, port::dialogs::closeDialogOf>(app, ts, applied);
    }
    const nfs::u32 output = gameSettings(guest).musicMode;
    if (output != guest.dword(kSoundOutputKept))
    {
        guest.setDword(kSoundOutputKept, output);
        callAt(guest, kApplySound, fe::args({ output, 0 }));
        guest.setDword(kOutputChanged, 1);
    }
    const nfs::u32 setting = gameSettings(guest).menuMusicAsked;
    if (setting != guest.dword(kSetting1ecKept))
    {
        guest.setDword(kSetting1ecKept, setting);
        callAt(guest, kApplySound, fe::args({ setting, 1 }));
    }
    return 0;
}

namespace
{

/* sub_437990 and the four after it: a volume slider moved, its value back to
 * 0..127, cut towards zero.  The effects' and the engine's are the race's own
 * in a race. */
nfs::u32 onVolumeMoved(Guest& guest, ThreadState& ts, nfs::u32 item, nfs::u32* volume, nfs::u32* raceVolume)
{
    nfs::Float value = readSliderValue(guest, ts, item, 0);
    if (raceVolume != nullptr)
    {
        value = nfs::Float(float(value));   // kept as a float first: fstp dword, fld dword
        if (inRace(guest))
            volume = raceVolume;
    }
    FloatMath x87(guest);
    *volume = nfs::u32(x87.truncated(x87.mul(value, nfs::Float(kSliderToVolume))));
    // eax as the slider's value leaves it: the slider, which its code keeps there.
    return item;
}

}  // namespace

/* sub_437a90, a volume slider drawn.  Selected, it plays at its volume: the
 * music sliders the music, the others an effect, started the first time. */
nfs::u32 frontend::options::onVolumeSliderDraw(App* app, ThreadState& ts, u32 item, u32 selected)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    // By address, and the slider's drawing below: the function leaves edx as these calls hand it.
    if (fe::direct<kShownNow, frontend::loop::shownNow>(guest.app(), guest.ts(), item) == 0)
        return 0;
    if (selected != 0)
    {
        if (item == guest.dword(kMenuMusicSlider) || item == guest.dword(kRaceMusicSlider))
        {
            const bool menuMusic = item == guest.dword(kMenuMusicSlider);
            const nfs::u32 held = menuMusic ? kMenuMusicHeld : kRaceMusicHeld;
            if (guest.dword(held) == 0)
            {
                if (!inRace(guest))
                    stopSample(guest, ts);
                guest.setDword(held, 1);
            }
            if (!inRace(guest))
                streamVolume(guest, ts, kMusicStream,
                             menuMusic ? gameSettings(guest).menuMusicVolume : gameSettings(guest).raceMusicVolume);
        }
        else
        {
            struct Effect
            {
                nfs::u32 slider;
                nfs::u32 held;
                nfs::u32 GameSettings::* volume;
            };
            const Effect effects[] = {
                { kEffectsSlider, kEffectsHeld, &GameSettings::effectsVolume },
                { kEngineSlider, kEngineHeld, &GameSettings::engineVolume },
                { kSpeechSlider, kSpeechHeld, &GameSettings::speechVolume },
            };
            for (const Effect& effect : effects)
            {
                if (item != guest.dword(effect.slider))
                    continue;
                if (guest.dword(effect.held) == 0)
                {
                    if (!inRace(guest))
                        streamVolume(guest, ts, kMusicStream, gameSettings(guest).raceMusicVolume);
                    guest.setDword(effect.held, 1);
                    // The effect: bank, number, volume, pan 0 (ebx), pitch kSliderEffectPitch (pushed).
                    if (!inRace(guest) && guest.dword(kSample) == 0)
                        guest.setDword(kSample, nfs::u32(*fe::direct<0x4182b0, port::bankEffect>(
                                                    app, ts, ts.sp - 8, std::int32_t(guest.dword(kEffectsBank)),
                                                    std::int32_t(kSliderEffect),
                                                    std::int32_t(gameSettings(guest).*effect.volume), 0u,
                                                    kSliderEffectPitch)));
                }
                if (!inRace(guest) && guest.dword(kSample) != 0)
                    fe::direct<0x4e95fc, port::setVolume>(app, ts, ts.sp - 4, guest.dword(kSample),
                                                          std::int32_t(gameSettings(guest).*effect.volume));
                break;
            }
        }
    }
    else
    {
        for (const auto& slider : { std::pair<nfs::u32, nfs::u32>{ kMenuMusicSlider, kMenuMusicHeld },
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
        streamVolume(guest, ts, kMusicStream, gameSettings(guest).menuMusicVolume);
        stopSample(guest, ts);
    }
        return nfs::u32(*fe::direct<kSliderDraw, port::fields::sliderDrawOf>(guest.app(), guest.ts(), guest.ts().sp - 4,
                                                                          item, selected));
}

namespace
{

/* ---------------------------------------------------------------------------
 * control1: the controllers
 * ------------------------------------------------------------------------- */

/* The controllers' records, 0x88 bytes each from 0x6fc970: the first dword
 * its kind (0 none), a bit for each of its eight axes, and each axis's two
 * ends, ints from 0 to 100. */
constexpr nfs::u32 kControllerSize = 0x88;
constexpr std::uint32_t kAxes = 8;
constexpr std::uint8_t kHasForceFeedback = 0x08;   // in the first byte

/* The players' controls (sub_4904d0): one player's 0x34 bytes at 0x6fd240,
 * or both players' from 0x6fd20c; as the screen opens 0x68 bytes from there
 * are kept. */
constexpr nfs::u32 kControlsKept = 0x5f3cd0;
constexpr std::uint32_t kControlsBytes = 0x68;
constexpr std::uint32_t kOnePlayersControls = 0x34;
// Set by Cancel's Enter: the kept controls go back as the screen closes.
constexpr nfs::u32 kControlsCancelled = 0x5f3da4;

// The pictures of the controllers, joys.qfs, loaded into the frontend's art
// for as long as their twelve shapes are found by name, jy01 to jy12.
constexpr nfs::u32 kFrontendArt = 0x559228;
constexpr nfs::u32 kArtFolder = 0x7a2db4;
constexpr nfs::u32 kJoyShapes = 0x5f3d38;       // int16 each
constexpr std::uint32_t kJoyShapeCount = 12;

constexpr GuestString kControlsCancel{ 0x53778c, "cancel" };
constexpr GuestString kJoysFile{ 0x537794, "joys.qfs" };
constexpr GuestString kPathFormat{ 0x5377a0, "%s%s" };
constexpr GuestString kJoyShapeFormat{ 0x5377a8, "jy%2d" };
constexpr GuestString kForceFeedbackButton{ 0x5377b0, "CTRL1_FORCEFEED" };
constexpr GuestString kDeadZoneButton{ 0x5377c0, "CTRL1_DEADZONE" };
constexpr GuestString kResetButton{ 0x5377d0, "reset" };

constexpr nfs::u32 kControlsCancelKey = 0x43d3f0;
constexpr nfs::u32 kResetClicked = 0x43d460;
constexpr nfs::u32 kResetAnswered = 0x43d410;
// The question before the reset: its line, its two buttons.
constexpr nfs::u32 kResetLines = 0x5f3db8;
constexpr nfs::u32 kResetButtons = 0x5f3dbc;
constexpr std::uint32_t kResetQuestion = 0x1ab;
constexpr std::uint32_t kYes = 0xc4;
constexpr std::uint32_t kNo = 0xc5;

}  // namespace

/* sub_43d4d0, control1 as it opens: the controls kept, the controllers'
 * pictures found, force feedback and the dead zones only with a controller
 * that has them. */
nfs::u32 frontend::options::onControlsEnter(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 0x64);
    if (menu == 0)
        return 0;
    guest.setDword(kControlsCancelled, 0);
    if (Item cancel = fe::findItem(guest, menu, kControlsCancel))
        guest.setDword(cancel.address() + kOnKey, kControlsCancelKey);
    const nfs::u32 controls = fe::direct<0x4904d0, controls::controlSets>(app, ts, gameSettings(guest).playMode);
    copyBytes(guest, ts, kControlsKept, controls, kControlsBytes);

    const nfs::u32 name = guest.frame();   // char[0x50]
    const nfs::u32 art = guest.dword(kFrontendArt);
    fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), name, kPathFormat.address, guest::Words{kArtFolder, kJoysFile.address});
    // ebx handed on as the copy's count left it; the game's code clears edx here for its loop.
    guest.setDword(kFrontendArt,
                   nfs::u32(*fe::direct<0x44ac90, port::buttons::shapesLoaded>(app, ts, ts.sp - 4, name,
                                                                                 kControlsBytes)));
    for (std::uint32_t i = 0; i < kJoyShapeCount; ++i)
    {
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), name, kJoyShapeFormat.address, guest::Words{i + 1});
        if (guest.byte(name + 2) == ' ')
            guest.setByte(name + 2, '0');
        guest.setWord(kJoyShapes + 2 * i,
                      std::uint16_t(*fe::direct<0x4d5db0, port::shapes::shapeNamed>(app, ts, ts.sp - 4, name)));
    }
    guest.setDword(kFrontendArt, art);

    Item force = fe::findItem(guest, menu, kForceFeedbackButton);
    if ((std::uint8_t(gameSettings(guest).devices[0].kind) & kHasForceFeedback) == 0 && force)
        force.setDisabled(true);
    Item deadZone = fe::findItem(guest, menu, kDeadZoneButton);
    if (gameSettings(guest).devices[0].kind == 0 && deadZone)
        deadZone.setDisabled(true);
    if (Item reset = fe::findItem(guest, menu, kResetButton))
        reset.setOnClick(kResetClicked);
    return 0;
}

/* sub_43d5f0, control1 as it closes: after Cancel the controls go back --
 * in split screen only the first player's half, as the original has it --
 * and otherwise, outside a race, they are applied. */
nfs::u32 frontend::options::onControlsExit(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    const nfs::u32 controls = fe::direct<0x4904d0, controls::controlSets>(app, ts, gameSettings(guest).playMode);
    if (guest.dword(kControlsCancelled) != 0)
    {
        const std::uint32_t bytes = gameSettings(guest).playMode == 1 ? kControlsBytes : kOnePlayersControls;
        copyBytes(guest, ts, controls, kControlsKept, bytes);
    }
    else if (!inRace(guest))
    {
        fe::direct<0x43c040, frontend::glue::noteDigitalControls>(app, ts);
    }
    return 0;
}

namespace
{

/* sub_43d3f0 (control1's Cancel), sub_442db0 (deadzone's), sub_447b40
 * (force's): Enter marks the screen cancelled, or not kept; the key goes on
 * to the button. */
nfs::u32 onCancelKey(Guest& guest, nfs::u32 item, nfs::u32 menu, std::uint16_t key, nfs::u32 flag,
                       nfs::u32 value)
{
    if (key == kKeyEnter)
        guest.setDword(flag, value);
    return passKey(guest, kButtonKey, item, menu, key);
}

}  // namespace

/* sub_43d460, the reset button: a question first. */
nfs::u32 frontend::options::onResetClicked(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
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
nfs::u32 frontend::options::onResetAnswered(App* app, ThreadState& ts, u32 answer, u32 close)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    guest.setByte(close, 0);
    if (answer == fe::kDialogEnter)
    {
        const std::uint32_t focus = fe::dialogFocus(guest);
        if (focus == 0)
        {
            fe::direct<0x4904f0, controls::defaultSets>(app, ts, gameSettings(guest).playMode == 1 ? 1u : 0u);
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

namespace
{

/* ---------------------------------------------------------------------------
 * deadzone: a controller's axes
 * ------------------------------------------------------------------------- */

/* The eight axes' sliders, each with two ends -- the axis's two ends, from
 * the top -- and a third, where the controller stands now.  The screen's data
 * from 0x558a10, 0xec bytes, cleared as it opens. */
constexpr nfs::u32 kDeadZoneData = 0x558a10;
constexpr std::uint32_t kDeadZoneDataBytes = 0xec;
constexpr nfs::u32 kAxisSliders = 0x558a10;     // eight items
constexpr nfs::u32 kAxisLastEnds = 0x558a30;    // two floats each, as last applied
constexpr nfs::u32 kControllerShown = 0x558a70;
constexpr nfs::u32 kControllerChosen = 0x558a74;
constexpr nfs::u32 kControllerState = 0x558a78; // what sub_4358f0 reads the controller into
constexpr nfs::u32 kAxisPositions = 0x558ac8;   // ints, 0 to 65535
// The ends as the screen opened, 64 bytes for each controller; put back as
// it closes unless kDeadZoneKept, which Cancel's Enter clears.
constexpr nfs::u32 kRangesKept = 0x660bc0;
constexpr std::uint32_t kRangesKeptSize = 0x40;
constexpr nfs::u32 kDeadZoneKept = 0x660fc0;

/* The list of controllers DZ_JOYSEL shows, 16 entries: a line of text each
 * (int16, 0x855 the first controller's name), whether it is there (int16)
 * and its colour. */
constexpr nfs::u32 kControllerListText = 0x555648;
constexpr nfs::u32 kControllerListShown = 0x555668;
constexpr nfs::u32 kControllerListColour = 0x555688;
constexpr std::uint32_t kControllerListEntries = 16;
constexpr std::uint32_t kFirstControllerName = 0x855;
constexpr std::uint32_t kControllerListColourValue = 0xff649dfd;

// A slider laid out in a race's pause screen: x, and y by its row.
constexpr std::uint16_t kPauseSliderX = 0xf2;
constexpr nfs::u32 kSliderOrder = 0x40;         // int16: its row among the shown ones
constexpr nfs::u32 kSliderOutside = 0x3d;       // byte: 1 outside a race

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

constexpr nfs::u32 kDeadZoneCancelKey = 0x442db0;
constexpr nfs::u32 kChooseController = 0x443260;
constexpr nfs::u32 kChooseControllerInRace = 0x443290;
constexpr nfs::u32 kAxisMoved = 0x442ef0;
constexpr nfs::u32 kAxisDraw = 0x442ff0;

nfs::u32 controllerRanges(nfs::u32 controller)
{
    return SETTINGS_ADDRESS(devices[0].axisRange) + controller * kControllerSize;
}

}  // namespace

/* sub_43d9a0: the list, one entry for each controller there is. */
nfs::u32 frontend::options::fillControllerList(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    for (std::uint32_t i = 0; i < kControllerListEntries; ++i)
    {
        if (std::int32_t(i) < std::int32_t(gameSettings(guest).deviceCount))
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
nfs::u32 frontend::options::axisIndex(App* app, ThreadState& ts, u32 item)
{
    const Guest guest(app, ts);
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
nfs::u32 frontend::options::applyAxis(App* app, ThreadState& ts, float first, float second, u32 controller, u32 axis)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 32);
    FloatMath x87(guest);
    const nfs::Float hundred = nfs::Float(kHundred);
    const nfs::Float firstScaled = x87.mul(nfs::Float(first), hundred);
    const nfs::Float secondScaled = x87.mul(hundred, nfs::Float(second));
    const std::int32_t firstEnd = x87.truncated(firstScaled);
    const std::int32_t secondEnd = x87.truncated(secondScaled);
    const nfs::u32 ends = controllerRanges(controller) + 8 * axis;
    nfs::u32 result = guest.dword(ends + 4);
    if (guest.dword(ends + 4) != nfs::u32(firstEnd))
    {
        guest.setDword(ends + 4, nfs::u32(firstEnd));
        result = *fe::direct<0x435b40, controls::zoneSet>(app, ts, controller, axis, nfs::u32(1),
                                                          nfs::u32(firstEnd));
    }
    if (nfs::u32(secondEnd) != guest.dword(ends))
    {
        guest.setDword(ends, nfs::u32(secondEnd));
        result = *fe::direct<0x435b40, controls::zoneSet>(app, ts, controller, axis, nfs::u32(0),
                                                          nfs::u32(secondEnd));
    }
    return result;
}

/* sub_442ef0, an axis's slider moved: its two ends kept a tenth apart, the
 * one that did not move giving way, and applied. */
nfs::u32 frontend::options::onAxisMoved(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 40);
    const nfs::u32 onMove = guest.dword(item + kSliderOnMove);
    const nfs::u32 axis = guest::direct<0x442e60, axisIndex>(app, ts, item);
    const float first = float(readSliderValue(guest, ts, item, 0));
    const float second = float(readSliderValue(guest, ts, item, 1));
    const nfs::Float firstFromBottom = fp::sub(nfs::Float(1.0), nfs::Float(first));
    const float gap = float(fp::abs(fp::sub(firstFromBottom, nfs::Float(second))));
    FloatMath x87(guest);
    if (x87.below(firstFromBottom, nfs::Float(second)) || x87.below(nfs::Float(gap), nfs::Float(kMinimumGap)))
    {
        setSliderOnMove(guest, ts, item, 0);
        const nfs::u32 last = kAxisLastEnds + 8 * axis;
        if (!same(guest, nfs::Float(first), nfs::Float(guest.floatAt(last))))
        {
            const float moved =
                float(fp::add(fp::sub(nfs::Float(1.0), nfs::Float(second)), nfs::Float(kMinusMinimumGap)));
            setSliderValue(guest, ts, item, moved, 0);
        }
        else if (!same(guest, nfs::Float(second), nfs::Float(guest.floatAt(last + 4))))
        {
            const float moved =
                float(fp::add(fp::sub(nfs::Float(1.0), nfs::Float(first)), nfs::Float(kMinusMinimumGap)));
            setSliderValue(guest, ts, item, moved, 1);
        }
        setSliderOnMove(guest, ts, item, onMove);
    }
    const float firstNow = float(readSliderValue(guest, ts, item, 0));
    const float secondNow = float(readSliderValue(guest, ts, item, 1));
    guest.setDword(kAxisLastEnds + 8 * axis, floatBits(firstNow));
    guest.setDword(kAxisLastEnds + 8 * axis + 4, floatBits(secondNow));
    return guest::direct<0x442dd0, applyAxis>(app, ts, firstNow, secondNow, guest.dword(kControllerChosen), axis);
}

/* sub_442ff0, an axis's slider drawn: its third end where the controller
 * stands now. */
nfs::u32 frontend::options::onAxisDraw(App* app, ThreadState& ts, u32 item, u32 selected)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    // By address, and the slider's drawing below: the function leaves edx as these calls hand it.
    if (fe::direct<kShownNow, frontend::loop::shownNow>(guest.app(), guest.ts(), item) == 0)
        return 0;
    const nfs::u32 axis = guest::direct<0x442e60, axisIndex>(app, ts, item);
    const nfs::u32 onMove = guest.dword(item + kSliderOnMove);
    setSliderOnMove(guest, ts, item, 0);
    fe::direct<0x4358f0, controls::readState>(app, ts, guest.dword(kControllerChosen), kControllerState);
    FloatMath x87(guest);
    const nfs::Float position = x87.mul(FloatMath::integer(std::int32_t(guest.dword(kAxisPositions + 4 * axis))),
                                        nfs::Float(kPositionScale));
    const float value = float(x87.sub(nfs::Float(1.0), position));
    setSliderValue(guest, ts, item, value, 2);   // edx, onMove here, is for the next call
    setSliderOnMove(guest, ts, item, onMove);
        return nfs::u32(*fe::direct<kSliderDraw, port::fields::sliderDrawOf>(guest.app(), guest.ts(), guest.ts().sp - 4,
                                                                          item, selected));
}

/* sub_443070: a controller's axes shown, a slider for each it has, laid out
 * one under the other in a race's pause screen. */
nfs::u32 frontend::options::showController(App* app, ThreadState& ts, u32 menu, u32 controller)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 28);
    if (menu == 0)
        return 0;
    guest.setDword(kControllerShown, controller);
    guest.setDword(kControllerChosen, controller);
    const nfs::u32 record = SETTINGS_ADDRESS(devices) + controller * kControllerSize;
    std::uint16_t order = 0;
    FloatMath x87(guest);
    for (std::uint32_t axis = 0; axis < kAxes; ++axis)
    {
        const nfs::u32 slider = guest.dword(kAxisSliders + 4 * axis);
        if (slider == 0)
            continue;
        if (inRace(guest))
        {
            guest.setWord(slider + fe::item::kX, kPauseSliderX);
            guest.setWord(slider + fe::item::kY, guest.word(kRowY + 4 * nfs::u32(std::int32_t(std::int16_t(order)))));
            guest.setByte(slider + kSliderOutside, 0);
        }
        else
        {
            guest.setByte(slider + kSliderOutside, 1);
        }
        if (record != 0 && (gameSettings(guest).deviceAt(controller * kControllerSize).axisMask & (1u << axis)) != 0)
        {
            const nfs::u32* const ends = gameSettings(guest).devices[guest.dword(kControllerChosen)].axisRange[axis];
            setSliderOnMove(guest, ts, slider, kAxisMoved);
            guest.setDword(slider + kOnDraw, kAxisDraw);
            Item(guest, slider).setHidden(false);
            guest.setWord(slider + kSliderOrder, order);
            // Not told of its own ends being set.
            setSliderOnMove(guest, ts, slider, 0);
            const float first = float(
                x87.mul(FloatMath::integer(std::int32_t(ends[1])), nfs::Float(kHundredth)));
            setSliderValue(guest, ts, slider, first, 0);
            const float second =
                float(x87.mul(FloatMath::integer(std::int32_t(ends[0])), nfs::Float(kHundredth)));
            setSliderValue(guest, ts, slider, second, 1);   // edx, kAxisMoved here, is for the next call
            ++order;
            setSliderOnMove(guest, ts, slider, kAxisMoved);
        }
        else
        {
            setSliderOnMove(guest, ts, slider, 0);
            guest.setDword(slider + kOnDraw, kSliderDraw);
            Item(guest, slider).setHidden(true);
        }
    }
    nfs::u32 found = 0;
    for (const GuestString& codelink : kHiddenItems)
    {
        Item item = fe::findItem(guest, menu, codelink);
        if (item)
            item.setHidden(true);
        found = item.address();
    }
    return found;
}

namespace
{

/* sub_443260 (the list in the menus) and sub_443290 (the button in a race),
 * DZ_JOYSEL's key: another controller chosen, its axes shown. */
nfs::u32 onChooseControllerKey(App* app, ThreadState& ts, Guest& guest, nfs::u32 handler, nfs::u32 item,
                                 nfs::u32 menu, std::uint16_t key)
{
    const nfs::u32 result = passKey(guest, handler, item, menu, key);
    const nfs::u32 chosen = guest.dword(kControllerChosen);
    if (chosen != guest.dword(kControllerShown))
        guest::direct<0x443070, frontend::options::showController>(app, ts, menu, chosen);
    else
        {}
    return result;
}

}  // namespace

/* sub_4432c0, deadzone as it opens (psdeadz too): every controller's ends
 * kept, the sliders found, the first controller shown. */
nfs::u32 frontend::options::onDeadZoneEnter(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    if (menu == 0)
        return 0;
    if (inRace(guest))
        dimBehind(guest, ts);
    guest::direct<0x43d9a0, fillControllerList>(app, ts);
    greyCreditsInNetworkRace(guest, menu, kDeadZoneCredits);
    guest.setDword(kDeadZoneKept, 1);
    if (Item cancel = fe::findItem(guest, menu, kDeadZoneCancel))
        guest.setDword(cancel.address() + kOnKey, kDeadZoneCancelKey);
    for (nfs::u32 controller = 0; std::int32_t(controller) < std::int32_t(gameSettings(guest).deviceCount);
         ++controller)
        copyBytes(guest, ts, kRangesKept + kRangesKeptSize * controller, controllerRanges(controller),
                  kRangesKeptSize);
    fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.ts(), kDeadZoneData, kDeadZoneDataBytes);
    for (std::uint32_t axis = 0; axis < kAxes; ++axis)
        guest.setDword(kAxisSliders + 4 * axis, fe::findItem(guest, menu, kAxisCodelinks[axis]).address());
    guest::direct<0x443070, showController>(app, ts, menu, nfs::u32(0));
    if (Item choice = fe::findItem(guest, menu, kControllerChoice))
        guest.setDword(choice.address() + kOnKey, inRace(guest) ? kChooseControllerInRace : kChooseController);
    return 0;
}

/* sub_443430, deadzone as it closes: after Cancel every controller's ends go
 * back and are applied again. */
nfs::u32 frontend::options::onDeadZoneExit(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 28);
    if (guest.dword(kDeadZoneKept) != 0)
        return 0;
    for (nfs::u32 controller = 0; std::int32_t(controller) < std::int32_t(gameSettings(guest).deviceCount);
         ++controller)
    {
        const nfs::u32 kept = kRangesKept + kRangesKeptSize * controller;
        copyBytes(guest, ts, controllerRanges(controller), kept, kRangesKeptSize);
        for (std::uint32_t axis = 0; axis < kAxes; ++axis)
        {
            if ((gameSettings(guest).devices[controller].axisMask & (1u << axis)) == 0)
                continue;
            fe::direct<0x435b40, controls::zoneSet>(app, ts, controller, nfs::u32(axis), nfs::u32(1),
                                                    guest.dword(kept + 8 * axis + 4));
            fe::direct<0x435b40, controls::zoneSet>(app, ts, controller, nfs::u32(axis), nfs::u32(0),
                                                    guest.dword(kept + 8 * axis));
        }
    }
    return 0;
}

namespace
{

/* ---------------------------------------------------------------------------
 * force: force feedback
 * ------------------------------------------------------------------------- */

/* Six sliders, their items from 0x6611e0, cleared as the screen opens and
 * closes.  The strength is the first controller's; the five effects, 0 to
 * 100, are kept from 0x6fd1f0 -- body roll and road grip never both: one
 * moved from 0 puts the other to 0. */
constexpr nfs::u32 kForceSliders = 0x6611e0;
constexpr std::uint32_t kForceSliderBytes = 0x18;
constexpr nfs::u32 kBodyRollSlider = 0x6611f0;
constexpr nfs::u32 kRoadGripSlider = 0x6611f4;
constexpr std::uint32_t kForceEffectsBytes = 0x14;
// What they were as the screen opened, and whether Cancel was pressed.
constexpr nfs::u32 kForceEffectsKept = 0x6611f8;
constexpr nfs::u32 kStrengthKept = 0x66120c;
constexpr nfs::u32 kForceCancelled = 0x661210;
// Byte: 0x10 hides road grip.
constexpr nfs::u32 kEdition = 0x55eb2e;
constexpr std::uint8_t kNoRoadGrip = 0x10;

// A slider laid out in a race's pause screen.
constexpr std::uint16_t kPauseForceSliderX = 0x140;
constexpr nfs::u32 kSliderLength = 0x64;        // int16
constexpr nfs::u32 kSliderBarY = 0x66;          // int16
constexpr std::uint16_t kPauseSliderLength = 0x40;
constexpr std::uint8_t kStrengthBrightness = 0x7f;
constexpr float kStrengthToBrightness = 128.0f;   // at 0x538ca4

constexpr GuestString kForceCredits{ 0x538cbc, "FORCE_CREDITS" };
constexpr GuestString kForceCodelinks[] = {
    { 0x538ccc, "FORCE_STICKVOLUME" }, { 0x538ce0, "FORCE_ROADEFFECTS" }, { 0x538cf4, "FORCE_COLLISION" },
    { 0x538d04, "FORCE_ENGINE" },      { 0x538d14, "FORCE_BODYROLL" },    { 0x538d24, "FORCE_ROADGRIP" },
};
constexpr GuestString kForceCancel{ 0x538d34, "FORCE_CANCEL" };

constexpr nfs::u32 kForceCancelKey = 0x447b40;
constexpr nfs::u32 kStrengthMoved = 0x447b60;
constexpr nfs::u32 kRoadEffectsMoved = 0x447c40;
constexpr nfs::u32 kCollisionMoved = 0x447c70;
constexpr nfs::u32 kEngineForceMoved = 0x447ca0;
constexpr nfs::u32 kBodyRollMoved = 0x447cd0;
constexpr nfs::u32 kRoadGripMoved = 0x447d50;

}  // namespace

/* sub_447dd0, force as it opens (psforce too): the sliders set from the
 * settings, which are kept. */
nfs::u32 frontend::options::onForceEnter(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 32);
    if (menu == 0)
        return 0;
    if (inRace(guest))
        dimBehind(guest, ts);
    fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.ts(), kForceSliders, kForceSliderBytes);
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
        nfs::u32* setting;
        nfs::u32 moved;
    };
    const Slider sliders[] = {
        { 0, &gameSettings(guest).devices[0].gain, kStrengthMoved },  { 2, &gameSettings(guest).forceRoad, kRoadEffectsMoved },
        { 3, &gameSettings(guest).forceCollision, kCollisionMoved },  { 4, &gameSettings(guest).forceEngine, kEngineForceMoved },
        { 6, &gameSettings(guest).forceBodyRoll, kBodyRollMoved },    { 7, &gameSettings(guest).forceRoadGrip, kRoadGripMoved },
    };
    FloatMath x87(guest);
    for (std::uint32_t i = 0; i < std::size(sliders); ++i)
    {
        const nfs::u32 slot = kForceSliders + 4 * i;
        const nfs::u32 slider = guest.dword(slot);
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
        const nfs::Float setting = i == 0 ? FloatMath::integer(std::int32_t(*s.setting))
                                          : nfs::Float(double(*s.setting));
        const float value = float(x87.mul(setting, nfs::Float(kHundredth)));
        if (s.moved == kRoadGripMoved)
        {
            setSliderValue(guest, ts, guest.dword(slot), value, 0);
            if (guest.byte(kEdition) & kNoRoadGrip)
                Item(guest, guest.dword(slot)).setHidden(true);
            else
                setSliderOnMove(guest, ts, guest.dword(slot), s.moved);
        }
        else
        {
            setSliderValue(guest, ts, guest.dword(slot), value, 0);   // edx, s.moved, is for the next call
            setSliderOnMove(guest, ts, guest.dword(slot), s.moved);
        }
    }

    if (Item cancel = fe::findItem(guest, menu, kForceCancel))
        guest.setDword(cancel.address() + kOnKey, kForceCancelKey);
    copyBytes(guest, ts, kForceEffectsKept, SETTINGS_ADDRESS(forceCollision), kForceEffectsBytes);
    guest.setDword(kStrengthKept, gameSettings(guest).devices[0].gain);
    guest.setDword(kForceCancelled, 0);
    return 0;
}

/* sub_4481c0, force as it closes: Cancel puts the settings back; the strength
 * is applied either way. */
nfs::u32 frontend::options::onForceExit(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    if (guest.dword(kForceCancelled) == 1)
    {
        copyBytes(guest, ts, SETTINGS_ADDRESS(forceCollision), kForceEffectsKept, kForceEffectsBytes);
        gameSettings(guest).devices[0].gain = guest.dword(kStrengthKept);
    }
    fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.ts(), kForceSliders, kForceSliderBytes);
    fe::direct<0x435df0, controls::gain>(app, ts, nfs::u32(0), gameSettings(guest).devices[0].gain);
    return 0;
}

/* sub_447b60, the strength moved: kept in hundredths, and the other sliders'
 * bars drawn the brighter for it. */
nfs::u32 frontend::options::onStrengthMoved(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 36);
    const nfs::Float value = readSliderValue(guest, ts, item, 0);
    FloatMath x87(guest);
    const nfs::Float hundredths = x87.mul(value, nfs::Float(kHundred));
    const nfs::Float scaled = x87.mul(value, nfs::Float(kStrengthToBrightness));
    const std::int32_t strength = x87.truncated(hundredths);
    const std::int32_t brightness = x87.truncated(scaled);
    gameSettings(guest).devices[0].gain = nfs::u32(strength);
    const std::uint8_t colour = std::uint8_t((nfs::u32(brightness) & 0xff) + kStrengthBrightness);
    nfs::u32 last = 0;
    for (std::uint32_t i = 1; i < std::size(kForceCodelinks); ++i)
    {
        last = guest.dword(kForceSliders + 4 * i);
        if (last != 0)
            fe::direct<0x46cca0, frontend::glue::sliderSetColour>(app, ts, last, nfs::u32(colour), nfs::u32(1));
    }
    return last;
}

namespace
{

/* sub_447c40, sub_447c70, sub_447ca0: an effect moved, kept in hundredths. */
nfs::u32 onForceMoved(Guest& guest, ThreadState& ts, nfs::u32 item, nfs::u32& setting)
{
    FloatMath x87(guest);
    const std::int32_t value = x87.truncated(x87.mul(readSliderValue(guest, ts, item, 0), nfs::Float(kHundred)));
    setting = nfs::u32(value);
    return nfs::u32(value);
}

/* sub_447cd0 (body roll) and sub_447d50 (road grip): moved from 0, the other
 * goes to 0, its slider too. */
nfs::u32 onExclusiveForceMoved(Guest& guest, ThreadState& ts, nfs::u32 item, nfs::u32& setting, nfs::u32& other,
                                 nfs::u32 otherSlider)
{
    FloatMath x87(guest);
    const nfs::Float scaled = x87.mul(readSliderValue(guest, ts, item, 0), nfs::Float(kHundred));
    const nfs::u32 bodyRoll = guest.dword(kBodyRollSlider);
    const std::int32_t value = x87.truncated(scaled);
    setSliderOnMove(guest, ts, bodyRoll, 0);
    setSliderOnMove(guest, ts, guest.dword(kRoadGripSlider), 0);
    if (value != 0)
    {
        const nfs::u32 slider = guest.dword(otherSlider);
        setting = nfs::u32(value);
        other = 0;
        setSliderValue(guest, ts, slider, 0.0f, 0);
    }
    setSliderOnMove(guest, ts, guest.dword(kRoadGripSlider), kRoadGripMoved);
    const nfs::u32 last = guest.dword(kBodyRollSlider);
    setSliderOnMove(guest, ts, last, kBodyRollMoved);
    return last;   // eax as the last one's code leaves it: its slider
}

/* ---------------------------------------------------------------------------
 * graphics and advgrph
 * ------------------------------------------------------------------------- */

/* The graphics settings: sixteen dwords from 0x6fbc10, kept whole as either
 * screen opens. */
constexpr nfs::u32 kGraphicsKept = 0x6627b0;
constexpr std::uint32_t kGraphicsSettingsDwords = 16;
// The screen modes the driver offers, listed by sub_4482c0: 0x114 bytes each.
constexpr nfs::u32 kModes = 0x661220;
constexpr nfs::u32 kModeSize = 0x114;
constexpr nfs::u32 kModeCount = 0x559218;
constexpr nfs::u32 kModeHasDepthBuffer = 0x10;
// What the display can do (byte; 1 << the driver's type, sub_4b6020): 0x06
// the car's shading and the perspective.
constexpr nfs::u32 kDisplayFlags = 0x7a3a58;
constexpr std::uint8_t kDisplayShading = 0x06;
constexpr nfs::u32 kDisplayBuffering = 0x7a3a68;
// The z buffer's items, found as advgrph opens.
constexpr nfs::u32 kDepthBufferItem = 0x6627f0;
constexpr nfs::u32 kDepthBufferText = 0x6627f4;
constexpr nfs::u32 kSliderValueField = 0x54;      // float
constexpr nfs::u32 kEffectsIntensityMoved = 0x448570;

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

}  // namespace

/* sub_448440, graphics as it opens: the modes listed, the settings kept, no
 * car shading where the display has none. */
nfs::u32 frontend::options::onGraphicsEnter(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    fe::direct<0x4482c0, frontend::menua::listModes>(app, ts);
    if (std::int32_t(gameSettings(guest).screenMode) >= std::int32_t(guest.dword(kModeCount)))
        gameSettings(guest).screenMode = 0;
    copyDwords(guest, kGraphicsKept, SETTINGS_ADDRESS(configVersion), kGraphicsSettingsDwords);
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
nfs::u32 frontend::options::onGraphicsExit(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    if (leftBy(guest, ts, menu, kGraphicsCancel))
    {
        copyDwords(guest, SETTINGS_ADDRESS(configVersion), kGraphicsKept, kGraphicsSettingsDwords);
        return 0;
    }
    fe::direct<0x4482c0, frontend::menua::listModes>(app, ts);
    const nfs::u32 chosen = kModes + kModeSize * gameSettings(guest).screenMode;
    // port: Dynamic Fullscreen kept as its marker, its width being the screen's
    gameSettings(guest).screenWidth = dynamicFullscreenMode(guest.dword(chosen)) ? kDynamicFullscreenWidth
                                                                                 : guest.dword(chosen + 4);
    return 0;
}

/* sub_448610, advgrph as it opens: what the display and the mode cannot do
 * greyed out. */
nfs::u32 frontend::options::onAdvancedEnter(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 0x12c);
    copyDwords(guest, kGraphicsKept, SETTINGS_ADDRESS(configVersion), kGraphicsSettingsDwords);
    const nfs::u32 mode = guest.frame();   // a mode's 0x114 bytes
    // The game's code has the screen in eax and the credits' codelink in edx here, for its next call.
    fe::direct<0x448330, frontend::menua::copyChosenMode>(app, ts, mode);
    greyCreditsInNetworkRace(guest, menu, kGraphicsCredits);
    if (Item german = fe::findItem(guest, menu, kGermanOnly))
        german.setHidden(gameSettings(guest).language != 1);
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
         * driver, so the slider is always there (the port's menu unlock change
         * does the same to the game's code; the port's alpha intensity change makes
         * the value count). */
        guest.setFloat(intensity.address() + kSliderValueField,
                       float(nfs::Float(gameSettings(guest).alphaIntensity)));
        setSliderOnMove(guest, ts, intensity.address(), kEffectsIntensityMoved);
    }
    return 0;
}

/* sub_4485a0, advgrph on every pass: projected text only with the buffering
 * that allows it and the setting on. */
nfs::u32 frontend::options::onAdvancedFrame(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    auto allowed = [&]() {
        return guest.dword(kDisplayBuffering) != 0 && gameSettings(guest).projectedText != 0;
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
nfs::u32 frontend::options::onAdvancedExit(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    if (leftBy(guest, ts, menu, kAdvancedCancel))
        copyDwords(guest, SETTINGS_ADDRESS(configVersion), kGraphicsKept, kGraphicsSettingsDwords);
    return 0;
}

/* sub_448570, the effects' intensity moved, while it is shown. */
nfs::u32 frontend::options::onEffectsIntensityMoved(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    if (itemShown(guest, ts, item) == 0)
        return 0;
    gameSettings(guest).alphaIntensity = float(readSliderValue(guest, ts, item, 0));
    // eax as the slider's value leaves it: the slider, which its code keeps there.
    return item;
}

namespace
{

/* ---------------------------------------------------------------------------
 * The strings each function hands to the game
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

/* A core run under the game's name, its arguments read as the game's code hands them: a screen's
 * handler the screen in eax; an item's key handler the item in eax, the screen in edx, the key in
 * bx; a slider's function the item in eax, and for a drawing one whether it is selected in edx. */
template <typename Core>
nfs::u32 wholeForMenu(App* app, ThreadState& ts, nfs::Args& args, fe::Site& site, Core core, nfs::u32 menu)
{
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return core(app, ts, menu); });
}

template <typename Core>
nfs::u32 wholeForKey(App* app, ThreadState& ts, nfs::Args& args, fe::Site& site, Core core, nfs::u32 item, nfs::u32 menu, nfs::u32 ebx)
{
    const std::uint16_t key = std::uint16_t(ebx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return core(app, ts, item, menu, key); });
}

template <typename Core>
nfs::u32 wholeForItem(App* app, ThreadState& ts, nfs::Args& args, fe::Site& site, Core core, nfs::u32 item)
{
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return core(app, ts, item); });
}

template <typename Core>
nfs::u32 wholeForDraw(App* app, ThreadState& ts, nfs::Args& args, fe::Site& site, Core core, nfs::u32 item, nfs::u32 selected)
{
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return core(app, ts, item, selected); });
}

template <typename Core>
nfs::u32 wholeForNone(App* app, ThreadState& ts, nfs::Args& args, fe::Site& site, Core core)
{
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return core(app, ts); });
}

}

/* Each function: the game's function, what its ret pops, what its prologue
 * pushes, the strings it hands to the game and the registers it pushes. */

nfs::u32 frontend::optionsOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_442aa0", 0, 0, kOptionsStrings, std::size(kOptionsStrings), kEcxEdx };
    return wholeForMenu(app, ts, args, site, options::onOptionsEnter, args.eax);
}

nfs::u32 frontend::feedOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4374b0", 0, 0, kFeedEnterStrings, std::size(kFeedEnterStrings), kEcxEdx };
    return wholeForMenu(app, ts, args, site, options::onFeedEnter, args.eax);
}

nfs::u32 frontend::feedOnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_437590", 0, 0, kFeedExitStrings, std::size(kFeedExitStrings), fe::kRestoreEdx };
    return wholeForMenu(app, ts, args, site, options::onFeedExit, args.eax);
}

nfs::u32 frontend::feedSeparate(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_437250", 0, 0, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEdx | fe::kRestoreEdi };
    return wholeForNone(app, ts, args, site, options::separateFeeds);
}

/* The feeds' keys: a button's and a list's for each, one body each (onFeedButtonKey,
 * onFeedListKey) with the feed and the other two. */
nfs::u32 frontend::options::feedTimeButtonKey(App* app, ThreadState& ts, u32 item, u32 menu, std::uint16_t key)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    return onFeedButtonKey(guest, item, menu, key, gameSettings(guest).feedKeys[0], gameSettings(guest).feedKeys[1],
                           gameSettings(guest).feedKeys[2]);
}

nfs::u32 frontend::options::feedPositionButtonKey(App* app, ThreadState& ts, u32 item, u32 menu, std::uint16_t key)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    return onFeedButtonKey(guest, item, menu, key, gameSettings(guest).feedKeys[1], gameSettings(guest).feedKeys[0],
                           gameSettings(guest).feedKeys[2]);
}

nfs::u32 frontend::options::feedSpeedButtonKey(App* app, ThreadState& ts, u32 item, u32 menu, std::uint16_t key)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    return onFeedButtonKey(guest, item, menu, key, gameSettings(guest).feedKeys[2], gameSettings(guest).feedKeys[0],
                           gameSettings(guest).feedKeys[1]);
}

nfs::u32 frontend::options::feedTimeListKey(App* app, ThreadState& ts, u32 item, u32 menu, std::uint16_t key)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    return onFeedListKey(guest, item, menu, key, gameSettings(guest).feedKeys[0], gameSettings(guest).feedKeys[1],
                         gameSettings(guest).feedKeys[2]);
}

nfs::u32 frontend::options::feedPositionListKey(App* app, ThreadState& ts, u32 item, u32 menu, std::uint16_t key)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    return onFeedListKey(guest, item, menu, key, gameSettings(guest).feedKeys[1], gameSettings(guest).feedKeys[0],
                         gameSettings(guest).feedKeys[2]);
}

nfs::u32 frontend::options::feedSpeedListKey(App* app, ThreadState& ts, u32 item, u32 menu, std::uint16_t key)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    return onFeedListKey(guest, item, menu, key, gameSettings(guest).feedKeys[2], gameSettings(guest).feedKeys[0],
                         gameSettings(guest).feedKeys[1]);
}

namespace
{
constexpr std::uint8_t kButtonKeyRestores = fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi;
}

nfs::u32 frontend::feedTimeButtonKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_437300", 0, 0, nullptr, 0, kButtonKeyRestores };
    return wholeForKey(app, ts, args, site, options::feedTimeButtonKey, args.eax, args.edx, args.ebx);
}

nfs::u32 frontend::feedPositionButtonKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_437390", 0, 0, nullptr, 0, kButtonKeyRestores };
    return wholeForKey(app, ts, args, site, options::feedPositionButtonKey, args.eax, args.edx, args.ebx);
}

nfs::u32 frontend::feedSpeedButtonKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_437420", 0, 0, nullptr, 0, kButtonKeyRestores };
    return wholeForKey(app, ts, args, site, options::feedSpeedButtonKey, args.eax, args.edx, args.ebx);
}

nfs::u32 frontend::feedTimeListKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_437350", 0, 0, nullptr, 0, fe::kRestoreEcx };
    return wholeForKey(app, ts, args, site, options::feedTimeListKey, args.eax, args.edx, args.ebx);
}

nfs::u32 frontend::feedPositionListKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4373e0", 0, 0, nullptr, 0, fe::kRestoreEcx };
    return wholeForKey(app, ts, args, site, options::feedPositionListKey, args.eax, args.edx, args.ebx);
}

nfs::u32 frontend::feedSpeedListKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_437470", 0, 0, nullptr, 0, fe::kRestoreEcx };
    return wholeForKey(app, ts, args, site, options::feedSpeedListKey, args.eax, args.edx, args.ebx);
}

nfs::u32 frontend::audioOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_437e00", 0, 0, kAudioEnterStrings, std::size(kAudioEnterStrings), kAllButEbx };
    return wholeForMenu(app, ts, args, site, options::onAudioEnter, args.eax);
}

nfs::u32 frontend::audioOnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_438070", 0, 0, kAudioExitStrings, std::size(kAudioExitStrings) };
    return wholeForMenu(app, ts, args, site, options::onAudioExit, args.eax);
}

nfs::u32 frontend::audioOnFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_438260", 0, 0, nullptr, 0 };
    return wholeForNone(app, ts, args, site, options::onAudioFrame);
}

nfs::u32 frontend::audioVolumeDraw(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_437a90", 0, 0, nullptr, 0, kAllButEdx };
    return wholeForDraw(app, ts, args, site, options::onVolumeSliderDraw, args.eax, args.edx);
}

/* The volume sliders' moves: one body (onVolumeMoved) with the volume each moves, and for the
 * effects and the engine the race's own. */
nfs::u32 frontend::options::audioMenuMusicMoved(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    return onVolumeMoved(guest, ts, item, &gameSettings(guest).menuMusicVolume, nullptr);
}

nfs::u32 frontend::options::audioRaceMusicMoved(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    return onVolumeMoved(guest, ts, item, &gameSettings(guest).raceMusicVolume, nullptr);
}

nfs::u32 frontend::options::audioEffectsMoved(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return onVolumeMoved(guest, ts, item, &gameSettings(guest).effectsVolume, &guestDword(guest, kRaceEffectsVolume));
}

nfs::u32 frontend::options::audioEngineMoved(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return onVolumeMoved(guest, ts, item, &gameSettings(guest).engineVolume, &guestDword(guest, kRaceEngineVolume));
}

nfs::u32 frontend::options::audioSpeechMoved(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    return onVolumeMoved(guest, ts, item, &gameSettings(guest).speechVolume, nullptr);
}

nfs::u32 frontend::audioMenuMusicMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_437990", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return wholeForItem(app, ts, args, site, options::audioMenuMusicMoved, args.eax);
}

nfs::u32 frontend::audioRaceMusicMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4379b0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return wholeForItem(app, ts, args, site, options::audioRaceMusicMoved, args.eax);
}

nfs::u32 frontend::audioEffectsMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4379d0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return wholeForItem(app, ts, args, site, options::audioEffectsMoved, args.eax);
}

nfs::u32 frontend::audioEngineMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_437a20", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return wholeForItem(app, ts, args, site, options::audioEngineMoved, args.eax);
}

nfs::u32 frontend::audioSpeechMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_437a70", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return wholeForItem(app, ts, args, site, options::audioSpeechMoved, args.eax);
}

nfs::u32 frontend::graphicsOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_448440", 0, 0, kGraphicsEnterStrings, std::size(kGraphicsEnterStrings) };
    return wholeForMenu(app, ts, args, site, options::onGraphicsEnter, args.eax);
}

nfs::u32 frontend::graphicsOnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4484e0", 0, 0, kGraphicsExitStrings, std::size(kGraphicsExitStrings), kAllButEbx };
    return wholeForMenu(app, ts, args, site, options::onGraphicsExit, args.eax);
}

nfs::u32 frontend::graphicsOnFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    // sub_448430: nothing.
    static fe::Site site{ "sub_448430", 0, 4, nullptr, 0, kRestoreNone };
    return fe::runThunk(app, ts, args, site, [](Guest&) { return nfs::u32(0); });
}

nfs::u32 frontend::advancedOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_448610", 0, 0, kAdvancedEnterStrings, std::size(kAdvancedEnterStrings) };
    return wholeForMenu(app, ts, args, site, options::onAdvancedEnter, args.eax);
}

nfs::u32 frontend::advancedOnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_448770", 0, 0, kAdvancedExitStrings, std::size(kAdvancedExitStrings), kAllButEbx };
    return wholeForMenu(app, ts, args, site, options::onAdvancedExit, args.eax);
}

nfs::u32 frontend::advancedOnFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4485a0", 0, 0, kAdvancedFrameStrings, std::size(kAdvancedFrameStrings), kEcxEdx };
    return wholeForMenu(app, ts, args, site, options::onAdvancedFrame, args.eax);
}

nfs::u32 frontend::advancedIntensityMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_448570", 0, 0, nullptr, 0, kEcxEdx };
    return wholeForItem(app, ts, args, site, options::onEffectsIntensityMoved, args.eax);
}

/* funkeys and hardkeys as they open: one body (onKeysEnter), instruction for instruction. */
nfs::u32 frontend::options::funkeysEnter(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return onKeysEnter(guest, menu);
}

nfs::u32 frontend::options::hardkeysEnter(App* app, ThreadState& ts, u32 menu)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return onKeysEnter(guest, menu);
}

/* The Cancel buttons' keys of control1, deadzone and force: one body (onCancelKey) with what Enter
 * marks. */
nfs::u32 frontend::options::controlsCancelKey(App* app, ThreadState& ts, u32 item, u32 menu, std::uint16_t key)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    return onCancelKey(guest, item, menu, key, kControlsCancelled, 1);
}

nfs::u32 frontend::options::deadZoneCancelKey(App* app, ThreadState& ts, u32 item, u32 menu, std::uint16_t key)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    return onCancelKey(guest, item, menu, key, kDeadZoneKept, 0);
}

nfs::u32 frontend::options::forceCancelKey(App* app, ThreadState& ts, u32 item, u32 menu, std::uint16_t key)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    return onCancelKey(guest, item, menu, key, kForceCancelled, 1);
}

/* DZ_JOYSEL's keys: one body (onChooseControllerKey) with the widget's own handler, the list's in
 * the menus, the button's in a race. */
nfs::u32 frontend::options::deadZoneChooseKey(App* app, ThreadState& ts, u32 item, u32 menu, std::uint16_t key)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    return onChooseControllerKey(app, ts, guest, kListKey, item, menu, key);
}

nfs::u32 frontend::options::deadZoneChooseKeyInRace(App* app, ThreadState& ts, u32 item, u32 menu,
                                                      std::uint16_t key)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    return onChooseControllerKey(app, ts, guest, kButtonKey, item, menu, key);
}

/* The force effects' sliders moved: one body (onForceMoved) with the setting each moves, and for
 * body roll and road grip one (onExclusiveForceMoved) with the other's too. */
nfs::u32 frontend::options::forceRoadEffectsMoved(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return onForceMoved(guest, ts, item, gameSettings(guest).forceRoad);
}

nfs::u32 frontend::options::forceCollisionMoved(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return onForceMoved(guest, ts, item, gameSettings(guest).forceCollision);
}

nfs::u32 frontend::options::forceEngineMoved(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    return onForceMoved(guest, ts, item, gameSettings(guest).forceEngine);
}

nfs::u32 frontend::options::forceBodyRollMoved(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    return onExclusiveForceMoved(guest, ts, item, gameSettings(guest).forceBodyRoll, gameSettings(guest).forceRoadGrip,
                                 kRoadGripSlider);
}

nfs::u32 frontend::options::forceRoadGripMoved(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    return onExclusiveForceMoved(guest, ts, item, gameSettings(guest).forceRoadGrip, gameSettings(guest).forceBodyRoll,
                                 kBodyRollSlider);
}

/* The controls' screens, native whole, no the game's code behind them
 * (MENU_NATIVES): the function keys' and the
 * hardware keys', the controllers' (control1), their dead zones and their
 * force feedback. */
namespace frontend
{

nfs::u32 funkeysOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_437600", 0, 0, kKeysStrings, std::size(kKeysStrings), kEcxEdx };
    return wholeForMenu(app, ts, args, site, options::funkeysEnter, args.eax);
}

nfs::u32 funkeysOnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    // sub_4377c0: nothing to undo.
    static fe::Site site{ "sub_4377c0", 0, 4, nullptr, 0, kRestoreNone };
    return fe::runThunk(app, ts, args, site, [](Guest&) { return nfs::u32(0); });
}

nfs::u32 hardkeysOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    // sub_4377d0: funkeys', instruction for instruction.
    static fe::Site site{ "sub_4377d0", 0, 0, kKeysStrings, std::size(kKeysStrings), kEcxEdx };
    return wholeForMenu(app, ts, args, site, options::hardkeysEnter, args.eax);
}

nfs::u32 controlsOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43d4d0", 0, 0, kControlsStrings, std::size(kControlsStrings),
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    return wholeForMenu(app, ts, args, site, options::onControlsEnter, args.eax);
}

nfs::u32 controlsOnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43d5f0", 0, 0, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    return wholeForNone(app, ts, args, site, options::onControlsExit);
}

nfs::u32 controlsCancelKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43d3f0", 0, 0, nullptr, 0, kRestoreNone };
    return wholeForKey(app, ts, args, site, options::controlsCancelKey, args.eax, args.edx, args.ebx);
}

nfs::u32 controlsResetClicked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43d460", 0, 0, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    return wholeForNone(app, ts, args, site, options::onResetClicked);
}

nfs::u32 controlsResetAnswered(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43d410", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx };
    const nfs::u32 answer = args.eax;
    const nfs::u32 close = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return options::onResetAnswered(app, ts, answer, close); });
}

nfs::u32 deadZoneFillList(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43d9a0", 0, 0, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    return wholeForNone(app, ts, args, site, options::fillControllerList);
}

nfs::u32 deadZoneOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4432c0", 0, 0, kDeadZoneStrings, std::size(kDeadZoneStrings),
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    return wholeForMenu(app, ts, args, site, options::onDeadZoneEnter, args.eax);
}

nfs::u32 deadZoneOnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_443430", 0, 0, nullptr, 0 };
    return wholeForNone(app, ts, args, site, options::onDeadZoneExit);
}

nfs::u32 deadZoneCancelKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_442db0", 0, 0, nullptr, 0, fe::kRestoreEcx };
    return wholeForKey(app, ts, args, site, options::deadZoneCancelKey, args.eax, args.edx, args.ebx);
}

nfs::u32 deadZoneChooseKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_443260", 0, 0, nullptr, 0, fe::kRestoreEcx };
    return wholeForKey(app, ts, args, site, options::deadZoneChooseKey, args.eax, args.edx, args.ebx);
}

nfs::u32 deadZoneChooseKeyInRace(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_443290", 0, 0, nullptr, 0, fe::kRestoreEcx };
    return wholeForKey(app, ts, args, site, options::deadZoneChooseKeyInRace, args.eax, args.edx, args.ebx);
}

nfs::u32 deadZoneShowController(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_443070", 0, 0, kShowControllerStrings, std::size(kShowControllerStrings),
                          kAllButEdx };
    const nfs::u32 menu = args.eax;
    const nfs::u32 controller = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return options::showController(app, ts, menu, controller); });
}

nfs::u32 deadZoneAxisIndex(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_442e60", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return wholeForItem(app, ts, args, site, options::axisIndex, args.eax);
}

nfs::u32 deadZoneApplyAxis(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_442dd0", 16, 0, nullptr, 0 };
    Guest arguments(app, ts);
    const float first = arguments.floatAt(ts.sp + 4);
    const float second = arguments.floatAt(ts.sp + 8);
    const nfs::u32 controller = arguments.dword(ts.sp + 12);
    const nfs::u32 axis = arguments.dword(ts.sp + 16);
    return fe::runThunk(app, ts, args, site,
                 [&](Guest&) { return options::applyAxis(app, ts, first, second, controller, axis); });
}

nfs::u32 deadZoneAxisMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_442ef0", 0, 0, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    return wholeForItem(app, ts, args, site, options::onAxisMoved, args.eax);
}

nfs::u32 deadZoneAxisDraw(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_442ff0", 0, 0, nullptr, 0, kAllButEdx };
    return wholeForDraw(app, ts, args, site, options::onAxisDraw, args.eax, args.edx);
}

nfs::u32 forceOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_447dd0", 0, 0, kForceStrings, std::size(kForceStrings) };
    return wholeForMenu(app, ts, args, site, options::onForceEnter, args.eax);
}

nfs::u32 forceOnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4481c0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEdx };
    return wholeForNone(app, ts, args, site, options::onForceExit);
}

nfs::u32 forceCancelKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_447b40", 0, 0, nullptr, 0, kRestoreNone };
    return wholeForKey(app, ts, args, site, options::forceCancelKey, args.eax, args.edx, args.ebx);
}

nfs::u32 forceStrengthMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_447b60", 0, 0, nullptr, 0 };
    return wholeForItem(app, ts, args, site, options::onStrengthMoved, args.eax);
}

nfs::u32 forceRoadEffectsMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_447c40", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return wholeForItem(app, ts, args, site, options::forceRoadEffectsMoved, args.eax);
}

nfs::u32 forceCollisionMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_447c70", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return wholeForItem(app, ts, args, site, options::forceCollisionMoved, args.eax);
}

nfs::u32 forceEngineMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_447ca0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return wholeForItem(app, ts, args, site, options::forceEngineMoved, args.eax);
}

nfs::u32 forceBodyRollMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_447cd0", 0, 0, nullptr, 0, kEcxEdx };
    return wholeForItem(app, ts, args, site, options::forceBodyRollMoved, args.eax);
}

nfs::u32 forceRoadGripMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_447d50", 0, 0, nullptr, 0, kEcxEdx };
    return wholeForItem(app, ts, args, site, options::forceRoadGripMoved, args.eax);
}

}  // namespace frontend

}
