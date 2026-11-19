#ifndef LIB_FORCEFEEDBACK_H_
#define LIB_FORCEFEEDBACK_H_

#include <lib/gamepad.h>

/* The game's force-feedback effects, played as rumble.  The game makes them
 * as DirectInput's -- a constant force for its jolts, a sine for the engine,
 * a square for the road, a spring for the wheel -- and its side of
 * DirectInput (src/nfs3hp/native_controls_input.cpp) hands them here as they
 * come: an effect made for a pad from DirectInput's description of it, its
 * parameters changed, started, stopped, unloaded; the device's gain and its
 * commands.  Each pad's effects are mixed into its motors' level. */
namespace nfs
{

/* A GUID as the game's memory holds one: what names DirectInput's devices
 * and effects. */
struct Guid
{
    u32 data1;
    u16 data2;
    u16 data3;
    u8  data4[8];

    bool operator==(const Guid& other) const
    {
        return data1 == other.data1
            && data2 == other.data2
            && data3 == other.data3
            && data4[0] == other.data4[0]
            && data4[1] == other.data4[1]
            && data4[2] == other.data4[2]
            && data4[3] == other.data4[3]
            && data4[4] == other.data4[4]
            && data4[5] == other.data4[5]
            && data4[6] == other.data4[6]
            && data4[7] == other.data4[7];
    }
};

namespace forcefeedback
{

/* The errors DirectInput gives the game: invalid parameters, not supported. */
constexpr u32 kInvalid = 0x80070057, kUnsupported = 0x80004001;

/* DirectInput's DIENVELOPE and DIEFFECT, as the game's memory holds them:
 * its pointers are the game's addresses. */
struct Envelope
{
    u32 dwSize;
    u32 dwAttackLevel;
    u32 dwAttackTime;
    u32 dwFadeLevel;
    u32 dwFadeTime;
};

struct Parameters
{
    u32 dwSize;
    u32 dwFlags;
    u32 dwDuration;
    u32 dwSamplePeriod;
    u32 dwGain;
    u32 dwTriggerButton;
    u32 dwTriggerRepeatInterval;
    u32 cAxes;
    nfs::u32 rgdwAxes;
    nfs::u32 rglDirection;
    nfs::u32 lpEnvelope;
    u32 cbTypeSpecificParams;
    nfs::u32 lpvTypeSpecificParams;
    u32 dwStartDelay;
};

class Effect;

/* Whether the effect DirectInput's GUID names is one this plays. */
bool supported(const Guid& guid);
/* An effect for `pad` of the kind `guid` names, with `parameters` (or none
 * yet), to `*effect`; 0, or the error DirectInput would give. */
u32 create(App* app, Gamepad* pad, const Guid& guid, const Parameters* parameters, Effect** effect);
void destroy(App* app, Effect* effect);
/* DirectInput's SetParameters, its DIEP_ flags. */
u32 setParameters(App* app, Effect* effect, const Parameters& parameters, u32 flags);
u32 start(Effect* effect, u32 iterations, u32 flags);
u32 stop(Effect* effect);
u32 download(Effect* effect);
u32 unload(Effect* effect);

/* Every frame: the effects a button fires, and the motors' levels. */
void update();
/* DirectInput's SendForceFeedbackCommand, its DISFFC_ flags. */
void command(Gamepad* pad, u32 flags);
/* GetForceFeedbackState's DIGFFS_ flags. */
u32 state(Gamepad* pad);
/* The device's gain, DIPROP_FFGAIN, 0 to 10000. */
void gain(Gamepad* pad, u32 value);
/* A pad gone: its effects silent and let go of. */
void detach(Gamepad* pad);

}}

#endif
