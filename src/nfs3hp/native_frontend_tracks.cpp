#include "native_frontend.h"
#include <iterator>

/* The screens a race's track is chosen on, decompiled:
 *
 *   location   MENUS/LOCATION.MNU: the track, its laps, its direction, mirror
 *              image, night and weather -- none of it in a series, which runs
 *              its own tracks;
 *   trkinfo    TRKINFO.MNU: the track's picture and map with its places named,
 *              and its description spoken once, in the game's language.
 *
 * location's every pass is the developer's recorder (sub_426b00, in
 * native_frontend_cars.cpp with the cars'). */
namespace nfs3hp
{

namespace
{

using fe::Guest;
using fe::GuestString;
using fe::Item;

constexpr x86::reg32 kRaceType = 0x6fd3b8;        // 0 a single race, 1 and 2 the series
constexpr x86::reg32 kTrack = 0x6fd4ac;            // the track chosen
constexpr x86::reg32 kLanguage = 0x6fbb4c;
constexpr x86::reg32 kNetworkConnected = 0x7a22e0;
constexpr x86::reg32 kSprintf = 0x4df690;

/* ---------------------------------------------------------------------------
 * location
 * ------------------------------------------------------------------------- */

constexpr GuestString kLocationItems[] = {
    { 0x538e68, "LOCATION_TRACKS_LIST" },    { 0x538e80, "LOCATION_LAPS_BUTTON" },
    { 0x538e98, "LOCATION_LAPS_CASCADE" },   { 0x538eb0, "LOCATION_DIR_BUTTON" },
    { 0x538ec4, "LOCATION_DIR_CASCADE" },    { 0x538edc, "LOCATION_MIR_BUTTON" },
    { 0x538ef0, "LOCATION_MIR_CASCADE" },    { 0x538f08, "LOCATION_NIGHT_BUTTON" },
    { 0x538f20, "LOCATION_NIGHT_CASCADE" },  { 0x538f38, "LOCATION_WEATHER_BUTTON" },
    { 0x538f50, "LOCATION_WEATHER_CASCADE" },
};
constexpr x86::reg32 kRecordTrack = 0x426b00;

/* sub_4487e0, location as it opens: everything greyed out in a series. */
x86::reg32 onLocationEnter(Guest& guest, x86::reg32 menu)
{
    for (const GuestString& codelink : kLocationItems)
    {
        if (Item item = fe::findItem(guest, menu, codelink))
        {
            const std::uint32_t type = guest.dword(kRaceType);
            item.setDisabled(type == 1 || type == 2);
        }
    }
    return 0;
}

/* ---------------------------------------------------------------------------
 * trkinfo
 * ------------------------------------------------------------------------- */

// The screen's state: shown, the description still to be spoken.
constexpr x86::reg32 kSpeakDescription = 0x559828;
constexpr x86::reg32 kInfoTrack = 0x677d8c;
constexpr x86::reg32 kInfo818 = 0x559818;
constexpr x86::reg32 kInfo81c = 0x55981c;
constexpr x86::reg32 kInfoD78 = 0x677d78;
constexpr x86::reg32 kFlagShape = 0x677d92;        // int16
constexpr x86::reg32 kInfoArt = 0x677d80;          // the track's .qfs, loaded
constexpr x86::reg32 kInfoD90 = 0x677d90;          // int16
// The tracks' art files and their map's places: a count for each track, and
// for each place the name of its shape (a format, with nothing to fill in).
constexpr x86::reg32 kTrackArtNames = 0x559620;
constexpr x86::reg32 kTrackPlaceCounts = 0x5597f4;
constexpr x86::reg32 kTrackPlaceNames = 0x5596d4;  // four for each track
constexpr std::int32_t kTracksWithPlaces = 9;
// The places' shapes and where they stand, int16 each.
constexpr x86::reg32 kPlaceShapes = 0x677d70;
constexpr x86::reg32 kPlaceX = 0x677d50;
constexpr x86::reg32 kPlaceY = 0x677d58;
constexpr x86::reg32 kPlaceCentreX = 0x677d68;
constexpr x86::reg32 kPlaceCentreY = 0x677d60;
constexpr x86::reg32 kArtFolder = 0x7a31b4;
constexpr x86::reg32 kDisplayFlags = 0x7a3a58;     // byte
constexpr std::uint8_t kNoTranslucency = 0x02;
constexpr x86::reg32 kMaskShape = 0x44;            // int16 of the mask's item: its texture

// The game's.
constexpr x86::reg32 kShapeByName = 0x4d5db0;
constexpr x86::reg32 kLoadShapes = 0x44ac90;
constexpr x86::reg32 kFindShape = 0x4ef1e0;        // eax the file, edx a name
constexpr x86::reg32 kFindShapeFormat = 0x4ef270;  // likewise, for the places
constexpr x86::reg32 kShapeTexture = 0x4d6390;     // eax a shape, edx -1 opaque
constexpr x86::reg32 kInfoD90Of = 0x456800;
constexpr x86::reg32 kPlaceTrackPlaces = 0x456200;
// The description, a stream: started, its volume, stopped.
constexpr x86::reg32 kSpeak = 0x4c94e0;
constexpr x86::reg32 kStartStream = 0x4c9240;      // eax a path
constexpr x86::reg32 kStreamVolume = 0x4c95f0;     // eax 0, edx the volume, ebx the language
constexpr std::uint32_t kDescriptionVolume = 0x33;
constexpr x86::reg32 kStopSpeaking = 0x4c9590;
constexpr x86::reg32 kStreamStop = 0x4c9490;
constexpr x86::reg32 kStreamClose = 0x4e8c1c;
constexpr x86::reg32 kFree = 0x4e1890;
constexpr x86::reg32 kSpeaking = 0x7dd838;
constexpr x86::reg32 kStreamHandle = 0x7dd844;
constexpr x86::reg32 kStreamBuffer = 0x7dd84c;
// The languages' folders (the 6th is the first's) and their speakers' names.
constexpr x86::reg32 kLanguageFolders[] = { 0x7a2b34, 0x7a2bb4, 0x7a2c34, 0x7a2cb4, 0x7a2d34, 0x7a2b34 };
constexpr x86::reg32 kLanguageSpeakers = 0x560b34;

constexpr GuestString kFlag{ 0x539a88, "flag" };
constexpr GuestString kArtName{ 0x539a90, "%s%s.qfs" };
constexpr GuestString kInfoMask{ 0x539a9c, "trkinfomask" };
constexpr GuestString kMask{ 0x539aa8, "mask" };
constexpr GuestString kDescriptionName{ 0x540348, "%s%s%02x" };

/* Half an int16, towards zero. */
std::int32_t half(std::int32_t value)
{
    return (value - (value >> 31)) >> 1;
}

/* sub_456200: the track's places on its map -- each one's shape, where it
 * stands and its centre; none for the tracks past the first nine, nor for the
 * first in a network race. */
x86::reg32 placeTrackPlaces(Guest& guest)
{
    const std::int32_t track = std::int32_t(guest.dword(kTrack));
    if (track >= kTracksWithPlaces || (guest.dword(kNetworkConnected) != 0 && track == 0))
    {
        for (const x86::reg32 at : { 0x677d74u, 0x677d72u, 0x677d70u, 0x677d76u })
            guest.setWord(at, 0);
        return guest.eax();
    }
    const x86::reg32 name = guest.frame();   // char[0x50]
    for (x86::reg32 i = 0;; ++i)
    {
        const x86::reg32 t = guest.dword(kTrack);
        if (std::int32_t(i) >= std::int32_t(guest.dword(kTrackPlaceCounts + 4 * t)))
            return t;
        guest.call(kSprintf, fe::args({}, { name, guest.dword(kTrackPlaceNames + 16 * t + 4 * i) }));
        const x86::reg32 shape = guest.call(kFindShapeFormat, fe::args({ guest.dword(kInfoArt), name }));
        const x86::reg32 opaque = (guest.byte(kDisplayFlags) & kNoTranslucency) ? 0xffffffff : 0;
        guest.setWord(kPlaceShapes + 2 * i, std::uint16_t(guest.call(kShapeTexture, fe::args({ shape, opaque }))));
        const x86::reg32 place = guest.dword(shape + 0xc);
        const std::int32_t x = std::int32_t(place << 20) >> 20;
        const std::int32_t y = std::int32_t(place << 4) >> 20;
        guest.setWord(kPlaceX + 2 * i, std::uint16_t(x));
        guest.setWord(kPlaceY + 2 * i, std::uint16_t(y));
        guest.setWord(kPlaceCentreX + 2 * i, std::uint16_t(half(std::int16_t(guest.word(shape + 4))) + x));
        guest.setWord(kPlaceCentreY + 2 * i, std::uint16_t(half(std::int16_t(guest.word(shape + 6))) + y));
    }
}

/* sub_4568d0, trkinfo as it opens: the track's art, its mask, its places. */
x86::reg32 onInfoEnter(Guest& guest, x86::reg32 menu)
{
    guest.setDword(kSpeakDescription, 1);
    guest.setDword(kInfoTrack, guest.dword(kTrack));
    guest.setDword(kInfo818, 0);
    guest.setDword(kInfo81c, 0);
    guest.setDword(kInfoD78, 0);
    guest.setWord(kFlagShape, std::uint16_t(guest.call(kShapeByName, fe::args({ kFlag.address }))));
    const x86::reg32 name = guest.frame();   // char[0x50]
    guest.call(kSprintf, fe::args({}, { name, kArtName.address, kArtFolder,
                                        guest.dword(kTrackArtNames + 4 * guest.dword(kTrack)) }));
    guest.setDword(kInfoArt, guest.call(kLoadShapes, fe::args({ name, kInfoMask.address })));
    if (Item mask = fe::findItem(guest, menu, kInfoMask))
    {
        const x86::reg32 shape = guest.call(kFindShape, fe::args({ guest.dword(kInfoArt), kMask.address }));
        guest.setWord(mask.address() + kMaskShape, std::uint16_t(guest.call(kShapeTexture, fe::args({ shape, 0 }))));
    }
    guest.setWord(kInfoD90, std::uint16_t(guest.call(kInfoD90Of, fe::args({ 0, 0 }))));
    guest.call(kPlaceTrackPlaces);
    return 0;
}

/* sub_4c94e0: the track's description spoken, from the language's folder. */
x86::reg32 speak(Guest& guest, x86::reg32 language, x86::reg32 track)
{
    const x86::reg32 path = guest.frame();          // char[0x100]
    const x86::reg32 folder = guest.frame() + 0x100;
    const x86::reg32 shown = guest.dword(kLanguage);
    if (shown < std::size(kLanguageFolders))
        guest.copyString(folder, kLanguageFolders[shown]);
    const x86::reg32 speaker = guest.dword(kLanguageSpeakers + 4 * language);
    guest.call(kSprintf, fe::args({}, { path, kDescriptionName.address, folder, speaker, track }));
    guest.leaveEdx(speaker);
    const x86::reg32 started = guest.call(kStartStream, fe::args({ path }));
    if (started != 0)
        guest.setDword(kSpeaking, 1);
    return started;
}

/* sub_456890, trkinfo on every pass: the description, once. */
x86::reg32 onInfoFrame(Guest& guest)
{
    if (guest.dword(kSpeakDescription) != 0)
    {
        guest.setDword(kSpeakDescription, 0);
        guest.call(kSpeak, fe::args({ guest.dword(kLanguage), guest.dword(kTrack) }));
        guest.call(kStreamVolume, fe::args({ 0, kDescriptionVolume, guest.dword(kLanguage) }));
    }
    return 0;
}

/* sub_4c9590: the description stopped, if it is spoken. */
x86::reg32 stopSpeaking(Guest& guest)
{
    if (guest.dword(kSpeaking) == 0)
        return guest.eax();
    guest.call(kStreamStop);
    guest.call(kStreamClose, fe::args({ guest.dword(kStreamHandle) }));
    if (const x86::reg32 buffer = guest.dword(kStreamBuffer))
    {
        guest.call(kFree, fe::args({ buffer, guest.edx(), 0, buffer }));
        guest.setDword(kStreamBuffer, 0);
    }
    guest.setDword(kSpeaking, 0);
    return guest.eax();
}

const GuestString kLocationStrings[] = {
    kLocationItems[0], kLocationItems[1], kLocationItems[2], kLocationItems[3],
    kLocationItems[4], kLocationItems[5], kLocationItems[6], kLocationItems[7],
    kLocationItems[8], kLocationItems[9], kLocationItems[10],
};
const GuestString kInfoEnterStrings[] = { kFlag, kArtName, kInfoMask, kMask };
const GuestString kSpeakStrings[] = { kDescriptionName };

}

void frontend::locationOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4487e0", 0, 24, kLocationStrings, std::size(kLocationStrings) };
    const x86::reg32 menu = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return onLocationEnter(guest, menu); });
}

void frontend::locationOnFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4487d0", 0, 4, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [](Guest& guest) {
        guest.call(kRecordTrack);
        return x86::reg32(0);
    });
}

void frontend::locationOnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    // sub_4489e0: nothing to undo.
    static fe::Site site{ "sub_4489e0", 0, 4, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [](Guest&) { return x86::reg32(0); });
}

void frontend::infoPlaceTrackPlaces(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_456200", 0, 24 + 0x50, nullptr, 0 };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return placeTrackPlaces(guest); });
}

void frontend::infoOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4568d0", 0, 16 + 0x50, kInfoEnterStrings, std::size(kInfoEnterStrings),
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    const x86::reg32 menu = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return onInfoEnter(guest, menu); });
}

void frontend::infoOnFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_456890", 0, 16, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return onInfoFrame(guest); });
}

void frontend::infoOnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_456990", 0, 4, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [](Guest& guest) {
        guest.call(kStopSpeaking);
        return x86::reg32(0);
    });
}

void frontend::infoSpeak(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4c94e0", 0, 16 + 0x200, kSpeakStrings, std::size(kSpeakStrings),
                          fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const x86::reg32 language = cpu.eax, track = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return speak(guest, language, track); });
}

void frontend::infoStopSpeaking(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4c9590", 0, 16, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return stopSpeaking(guest); });
}

}
