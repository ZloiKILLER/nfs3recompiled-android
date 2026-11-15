#include "native_frontend_tracks.h"
#include "native_watcom.h"
#include "game_settings.h"
#include "native_ea_memory.h"
#include "native_frontend.h"
#include "native_frontend_cars.h"
#include "native_frontend_eab.h"
#include "port_buttons.h"
#include "port_shapes.h"
#include "port_snd_play.h"
#include "port_sound_speech.h"
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
 * native_frontend_cars.cpp with the cars').
 *
 * Plan step 5: each function a typed core in frontend::tracks (native_frontend_tracks.h), in a
 * frame of its own where the game's function has one (fe::Frame: what it calls runs at the stack
 * addresses it did), and an adapter under the game's name for the calls by address.  A core of
 * this file calls another as C++ (guest::direct), and the typed cores of other files too
 * (fe::direct); the rest of the game still by address. */
namespace nfs3hp
{

namespace
{

using fe::Guest;
using fe::GuestString;
using fe::Item;
using App = win32::WinApplication;
using CPU = x86::CPU;
using x86::reg32;

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

}  // namespace

/* sub_4487e0, location as it opens: everything greyed out in a series. */
x86::reg32 frontend::tracks::locationEnter(App* app, CPU& cpu, reg32 menu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24);
    for (const GuestString& codelink : kLocationItems)
    {
        if (Item item = fe::findItem(guest, menu, codelink))
        {
            const std::uint32_t type = gameSettings(guest).raceType;
            item.setDisabled(type == 1 || type == 2);
        }
    }
    return 0;
}

/* sub_4487d0, location's every pass: the developer's recorder. */
x86::reg32 frontend::tracks::locationFrame(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    fe::direct<0x426b00, frontend::cars::recordTrack>(app, cpu);
    return 0;
}

/* ---------------------------------------------------------------------------
 * trkinfo
 * ------------------------------------------------------------------------- */

namespace
{

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
constexpr x86::reg32 kFindShapeFormat = 0x4ef270;  // eax the file, edx a name: the places'
// The description, a stream.
constexpr x86::reg32 kStartStream = 0x4c9240;      // eax a path
constexpr std::uint32_t kDescriptionVolume = 0x33;
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

}  // namespace

/* sub_456200: the track's places on its map -- each one's shape, where it
 * stands and its centre; none for the tracks past the first nine, nor for the
 * first in a network race. */
x86::reg32 frontend::tracks::placeTrackPlaces(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 0x50);
    const std::int32_t track = std::int32_t(gameSettings(guest).track);
    if (track >= kTracksWithPlaces || (guest.dword(kNetworkConnected) != 0 && track == 0))
    {
        for (const x86::reg32 at : { 0x677d74u, 0x677d72u, 0x677d70u, 0x677d76u })
            guest.setWord(at, 0);
        return guest.eax();
    }
    const x86::reg32 name = guest.frame();   // char[0x50]
    for (x86::reg32 i = 0;; ++i)
    {
        const x86::reg32 t = gameSettings(guest).track;
        if (std::int32_t(i) >= std::int32_t(guest.dword(kTrackPlaceCounts + 4 * t)))
            return t;
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.cpu(), name, guest.dword(kTrackPlaceNames + 16 * t + 4 * i), guest::Words({}));
        const x86::reg32 shape = guest.call(kFindShapeFormat, fe::args({ guest.dword(kInfoArt), name }));
        const x86::reg32 opaque = (guest.byte(kDisplayFlags) & kNoTranslucency) ? 0xffffffff : 0;
        const auto texture = fe::direct<0x4d6390, port::shapes::artTexturesOf>(app, cpu, cpu.esp - 4, shape,
                                                                               std::int32_t(opaque), cpu.ecx);
        guest.setWord(kPlaceShapes + 2 * i, std::uint16_t(*texture));
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
x86::reg32 frontend::tracks::infoEnter(App* app, CPU& cpu, reg32 menu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16 + 0x50);
    guest.setDword(kSpeakDescription, 1);
    guest.setDword(kInfoTrack, gameSettings(guest).track);
    guest.setDword(kInfo818, 0);
    guest.setDword(kInfo81c, 0);
    guest.setDword(kInfoD78, 0);
    guest.setWord(kFlagShape,
                  std::uint16_t(*fe::direct<0x4d5db0, port::shapes::shapeNamed>(app, cpu, cpu.esp - 4, kFlag.address)));
    const x86::reg32 name = guest.frame();   // char[0x50]
    fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.cpu(), name, kArtName.address, guest::Words{kArtFolder, guest.dword(kTrackArtNames + 4 * gameSettings(guest).track)});
    // The game's code hands it "trkinfomask" in edx too, which it does not read.
    guest.setDword(kInfoArt,
                   reg32(*fe::direct<0x44ac90, port::buttons::shapesLoaded>(app, cpu, cpu.esp - 4, name, cpu.ebx)));
    if (Item mask = fe::findItem(guest, menu, kInfoMask))
    {
        const x86::reg32 shape =
            fe::direct<0x4ef1e0, frontend::eab::shapeFileFind>(app, cpu, guest.dword(kInfoArt), kMask.address);
        const auto texture =
            fe::direct<0x4d6390, port::shapes::artTexturesOf>(app, cpu, cpu.esp - 4, shape, 0, cpu.ecx);
        guest.setWord(mask.address() + kMaskShape, std::uint16_t(*texture));
    }
    const auto mapShape = fe::direct<0x456800, port::buttons::mapShape>(app, cpu, cpu.esp - 4, 0, 0);
    guest.setWord(kInfoD90, std::uint16_t(*mapShape));
    guest::direct<0x456200, placeTrackPlaces>(app, cpu);
    return 0;
}

/* sub_4c94e0: the track's description spoken, from the language's folder. */
x86::reg32 frontend::tracks::speak(App* app, CPU& cpu, reg32 language, reg32 track)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16 + 0x200);
    const x86::reg32 path = guest.frame();          // char[0x100]
    const x86::reg32 folder = guest.frame() + 0x100;
    const x86::reg32 shown = gameSettings(guest).language;
    if (shown < std::size(kLanguageFolders))
        guest.copyString(folder, kLanguageFolders[shown]);
    const x86::reg32 speaker = guest.dword(kLanguageSpeakers + 4 * language);
    fe::direct<kSprintf, watcom::clib::sprintf>(app, cpu, path, kDescriptionName.address,
                                                guest::Words{folder, speaker, track});
    guest.leaveEdx(speaker);
    const x86::reg32 started = guest.call(kStartStream, fe::args({ path }));
    if (started != 0)
        guest.setDword(kSpeaking, 1);
    return started;
}

/* sub_456890, trkinfo on every pass: the description, once. */
x86::reg32 frontend::tracks::infoFrame(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    if (guest.dword(kSpeakDescription) != 0)
    {
        guest.setDword(kSpeakDescription, 0);
        guest::direct<0x4c94e0, speak>(app, cpu, gameSettings(guest).language, gameSettings(guest).track);
        fe::direct<0x4c95f0, port::descriptionStarted>(app, cpu, cpu.esp - 4, 0, kDescriptionVolume,
                                                       gameSettings(guest).language);
    }
    return 0;
}

/* sub_4c9590: the description stopped, if it is spoken. */
x86::reg32 frontend::tracks::stopSpeaking(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    if (guest.dword(kSpeaking) == 0)
        return guest.eax();
    fe::direct<0x4c9490, port::descriptionStopped>(app, cpu, cpu.esp - 4, cpu.eax);
    // What the calls leave in eax, the last one's: the cores hand theirs back.
    x86::reg32 left =
        reg32(*fe::direct<0x4e8c1c, port::removeEveryBank>(app, cpu, cpu.esp - 4, guest.dword(kStreamHandle)));
    if (const x86::reg32 buffer = guest.dword(kStreamBuffer))
    {
        // The game's code hands it the edx the call before left, 0 in ebx and the buffer in ecx too,
        // which it does not read.
        left = *fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, buffer);
        guest.setDword(kStreamBuffer, 0);
    }
    guest.setDword(kSpeaking, 0);
    return left;
}

/* sub_456990, trkinfo as it closes: the description stopped. */
x86::reg32 frontend::tracks::infoExit(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    guest::direct<0x4c9590, stopSpeaking>(app, cpu);
    return 0;
}

namespace
{

const GuestString kLocationStrings[] = {
    kLocationItems[0], kLocationItems[1], kLocationItems[2], kLocationItems[3],
    kLocationItems[4], kLocationItems[5], kLocationItems[6], kLocationItems[7],
    kLocationItems[8], kLocationItems[9], kLocationItems[10],
};
const GuestString kInfoEnterStrings[] = { kFlag, kArtName, kInfoMask, kMask };
const GuestString kSpeakStrings[] = { kDescriptionName };

}  // namespace

void frontend::locationOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4487e0", 0, 0, kLocationStrings, std::size(kLocationStrings) };
    const x86::reg32 menu = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return tracks::locationEnter(app, cpu, menu); });
}

void frontend::locationOnFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4487d0", 0, 0, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return tracks::locationFrame(app, cpu); });
}

void frontend::locationOnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    // sub_4489e0: nothing to undo.
    static fe::Site site{ "sub_4489e0", 0, 4, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [](Guest&) { return x86::reg32(0); });
}

void frontend::infoPlaceTrackPlaces(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_456200", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return tracks::placeTrackPlaces(app, cpu); });
}

void frontend::infoOnEnter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4568d0", 0, 0, kInfoEnterStrings, std::size(kInfoEnterStrings),
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    const x86::reg32 menu = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return tracks::infoEnter(app, cpu, menu); });
}

void frontend::infoOnFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_456890", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return tracks::infoFrame(app, cpu); });
}

void frontend::infoOnExit(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_456990", 0, 0, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return tracks::infoExit(app, cpu); });
}

void frontend::infoSpeak(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4c94e0", 0, 0, kSpeakStrings, std::size(kSpeakStrings),
                          fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const x86::reg32 language = cpu.eax, track = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return tracks::speak(app, cpu, language, track); });
}

void frontend::infoStopSpeaking(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4c9590", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi };
    fe::runWhole(app, cpu, site, [&](Guest&) { return tracks::stopSpeaking(app, cpu); });
}
}
