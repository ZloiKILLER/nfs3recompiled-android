// game_checks.cpp's parts for the game's side of DirectInput (native_controls_input.cpp), included in
// each table with the macro naming the part.
//
// DirectInput is made up in the harness's 0x611000 to 0x616000: its object, 18 devices (16 for the
// joysticks' table, the mouse's, the keyboard's) sharing one table of IDirectInputDevice2A's methods,
// 30 effects sharing IDirectInputEffect's -- each method a stand-in, by a pseudo-slot of 0xfff100 to
// 0xfff1ff -- and the buffers the arguments point at.  The game's tables the code reads in its code
// segment (the effects' GUIDs, the hats' and axes' offsets, createEffect's jump table), which the
// harness does not load, are written where the game has them.
#if defined(CONTROLS_WINDOWS)
    {0x434000, 0x436000},  // the game's tables in its code, as the cases write them
    {0x5f1000, 0x5f3000},  // the joysticks' and the effects' records
#elif defined(CONTROLS_EFFECTS)
    kInputCreate,        // DirectInputCreateA: the made-up DirectInput where its third word points, or an error
    kInputResult,        // a DirectInput method's HRESULT: DI_OK mostly, S_FALSE, an error
    kInputDevice,        // CreateDevice and QueryInterface: a made-up device where the third word points
    kInputEffect,        // CreateEffect: a made-up effect where its fourth word points
    kInputCapabilities,  // GetCapabilities: a DIDEVCAPS's flags, axes, buttons and hats
    kInputState,         // GetDeviceState: its buffer's bytes
    kInputData,          // GetDeviceData: events into its buffer, how many
    kInputProperty,      // GetProperty: a DIPROPDWORD's value
#elif defined(CONTROLS_EFFECT_CASES)
        case kInputCreate:
        case kInputResult:
        case kInputDevice:
        case kInputEffect:
        case kInputCapabilities:
        case kInputState:
        case kInputData:
        case kInputProperty:
        {
            static const x86::reg32 errors[] = {1,          0x8007001e, 0x8007000c, 0x80070057,
                                                0x80070005, 0x80040207, 0x80004005, 0x80070000};
            result = r % 3 != 0 ? 0 : errors[(r >> 4) % 8];
            const x86::reg32 pick = r >> 8;
            if (s.effect == kInputCreate && result == 0)
                word(words[2]) = 0x611000;
            else if (s.effect == kInputDevice && result == 0)
                word(words[2]) = 0x611100 + 0x10 * (pick % 18);
            else if (s.effect == kInputEffect && result == 0)
                word(words[3]) = 0x611400 + 0x10 * (pick % 30);
            else if (s.effect == kInputCapabilities && result == 0)
            {
                // dwFlags (DIDC_FORCEFEEDBACK now and then), dwAxes, dwButtons, dwPOVs
                word(words[1] + 4) = (pick & 0x1ff) | (pick % 3 == 0 ? 0x100 : 0);
                word(words[1] + 0xc) = (pick >> 4) % 10;
                word(words[1] + 0x10) = (pick >> 8) % 40;
                word(words[1] + 0x14) = (pick >> 12) % 6;
            }
            else if (s.effect == kInputState && result == 0)
            {
                // A DIJOYSTATE: axes, sliders, hats -- centred, a direction or another -- buttons.
                static const x86::reg32 hats[] = {0xffffffff, 0, 0x1194, 0x2328, 0x34bc, 0x4650, 0x57e4,
                                                  0x6978, 0x7b0c, 0xffff, 0x1000, 0x10000};
                std::uint64_t bits = random64(id);
                for (x86::reg32 i = 0; i < 0x50; i += 4)
                {
                    bits = mix(bits);
                    word(words[2] + i) = i >= 0x20 && i < 0x30 ? hats[bits % 12] : x86::reg32(bits >> 8);
                }
            }
            else if (s.effect == kInputData && (result == 0 || result == 1) && words[2] != 0)
            {
                // DIDEVICEOBJECTDATA, 0x10 each: x, y, the wheel, a button or another offset; its data.
                static const x86::reg32 offsets[] = {0, 4, 8, 0xc, 0xd, 0xe, 0xf, 0x10, 0xc, 0xd};
                const x86::reg32 most = std::min<x86::reg32>(word(words[3]), 0x20);
                const x86::reg32 count = pick % (most + 1);
                std::uint64_t bits = random64(id);
                for (x86::reg32 i = 0; i < count; ++i)
                {
                    bits = mix(bits);
                    word(words[2] + 0x10 * i) = bits % 16 == 15 ? x86::reg32(bits >> 8) : offsets[bits % 10];
                    word(words[2] + 0x10 * i + 4) =
                        bits % 3 == 0 ? x86::reg32(bits >> 16) : x86::reg32(x86::sreg32((bits >> 24) % 80) - 40);
                }
                word(words[3]) = count;
            }
            else if (s.effect == kInputProperty && result == 0)
                word(words[2] + 0x10) = pick % 4 == 0 ? r : pick % 10001;
            break;
        }
#elif defined(CONTROLS_GAME_FUNCTIONS)
    // What the game's side of DirectInput calls by address (native_controls_input.cpp):
    // DirectInputCreateA through its thunk, the window's message handlers' table.
    {0x533ee0, "DirectInputCreateA", kStdcall, 0, 4, kScratch, -1, 0, kInputCreate},
    {0x4cc660, "a message's handler", kStdcall, 0, 1, kScratch, -1, 0, kNothing},
    {0x4cc6d0, "a message's handler set", kStdcall, 0, 2, kScratch, -1, 0, kNothing},
#elif defined(CONTROLS_POINTERS)
    {0x534780, "SetCursor", kStdcall, 0, 1, kScratch, -1, 0, kNothing},
    {0x53479c, "SwapMouseButton", kStdcall, 0, 1, kScratch, -1, 0, kNothing},
    // IDirectInputA's, by pseudo-slot 0xfff100 + the method's offset.
    {0xfff108, "IDirectInput::Release", kStdcall, 0, 1, kScratch, -1, 0, kInputResult},
    {0xfff10c, "IDirectInput::CreateDevice", kStdcall, 0, 4, kScratch, 5, 4, kInputDevice},  // the GUID
    {0xfff110, "IDirectInput::EnumDevices", kStdcall, 0, 5, kScratch, -1, 0, kInputResult},
    // IDirectInputDevice2A's: 0xfff120 + the offset.
    {0xfff120, "IDirectInputDevice::QueryInterface", kStdcall, 0, 3, kScratch, 5, 4, kInputDevice},
    {0xfff128, "IDirectInputDevice::Release", kStdcall, 0, 1, kScratch, -1, 0, kInputResult},
    {0xfff12c, "IDirectInputDevice::GetCapabilities", kStdcall, 0, 2, kScratch, 5, 1, kInputCapabilities},
    {0xfff134, "IDirectInputDevice::GetProperty", kStdcall, 0, 3, kScratch, 6, 4, kInputProperty},
    {0xfff138, "IDirectInputDevice::SetProperty", kStdcall, 0, 3, kScratch, 6, 5, kInputResult},
    {0xfff13c, "IDirectInputDevice::Acquire", kStdcall, 0, 1, kScratch, -1, 0, kInputResult},
    {0xfff140, "IDirectInputDevice::Unacquire", kStdcall, 0, 1, kScratch, -1, 0, kInputResult},
    {0xfff144, "IDirectInputDevice::GetDeviceState", kStdcall, 0, 3, kScratch, -1, 0, kInputState},
    {0xfff148, "IDirectInputDevice::GetDeviceData", kStdcall, 0, 5, kScratch, 7, 1, kInputData},
    {0xfff14c, "IDirectInputDevice::SetDataFormat", kStdcall, 0, 2, kScratch, 5, 6, kInputResult},
    {0xfff150, "IDirectInputDevice::SetEventNotification", kStdcall, 0, 2, kScratch, -1, 0, kInputResult},
    {0xfff154, "IDirectInputDevice::SetCooperativeLevel", kStdcall, 0, 3, kScratch, -1, 0, kInputResult},
    {0xfff158, "IDirectInputDevice::GetObjectInfo", kStdcall, 0, 4, kScratch, 5, 1, kInputResult},
    {0xfff168, "IDirectInputDevice::CreateEffect", kStdcall, 0, 5, kScratch, 6, 13, kInputEffect},  // the DIEFFECT
    {0xfff178, "IDirectInputDevice::SendForceFeedbackCommand", kStdcall, 0, 2, kScratch, -1, 0, kInputResult},
    {0xfff184, "IDirectInputDevice::Poll", kStdcall, 0, 1, kScratch, -1, 0, kInputResult},
    // IDirectInputEffect's: 0xfff190 + the offset.
    {0xfff1a8, "IDirectInputEffect::SetParameters", kStdcall, 0, 3, kScratch, 5, 13, kInputResult},
    {0xfff1ac, "IDirectInputEffect::Start", kStdcall, 0, 3, kScratch, -1, 0, kInputResult},
    {0xfff1b0, "IDirectInputEffect::Stop", kStdcall, 0, 1, kScratch, -1, 0, kInputResult},
    {0xfff1b8, "IDirectInputEffect::Download", kStdcall, 0, 1, kScratch, -1, 0, kInputResult},
    {0xfff1bc, "IDirectInputEffect::Unload", kStdcall, 0, 1, kScratch, -1, 0, kInputResult},
    // The game's handler of the mouse's buttons let go ([0x56446c]), the WM_SETCURSOR handler before
    // hideCursor ([0x554ef4]).
    {0xfff1c4, "the mouse's buttons' handler", kWatcom, 1, 0, kEax, -1, 0, kNothing},
    {0xfff1c8, "the WM_SETCURSOR handler before", kStdcall, 0, 6, kScratch, -1, 0, kNothing},
#elif defined(CONTROLS_PROMISES)
    // The game's side of DirectInput (native_controls_input.cpp): every register compared.
    {"controls::startDirectInput", true, true, 0},
    {"controls::stopDirectInput", true, true, 0},
    {"controls::openMouse", true, true, 0},
    {"controls::setPointerBounds", true, true, 0},
    // ebx, ecx, edx: no caller reads what the code leaves there (its last event's, a callee's); the
    // game's callers (0x434b60, 0x442580, 0x451500 and its callers, 0x4bc530 and its callers, 0x4bc560,
    // 0x4bd4d0, 0x4c4e40) save them or write them first.  eax is compared: 0x4bc530 and 0x451500 give it back.
    {"controls::readMouse", true, true, 0, false, kNoHeap, false, "ebx ecx edx"},
    {"controls::findJoysticks", true, true, 0},
    {"controls::describeJoystick", true, true, 0},
    {"controls::readJoystick", true, true, 0},
    {"controls::readAxisZone", true, true, 0},
    {"controls::setAxisZone", true, true, 0},
    {"controls::resetForceFeedback", true, true, 0},
    {"controls::setAutocenter", true, true, 0},
    {"controls::pauseEffects", true, true, 0},
    {"controls::continueEffects", true, true, 0},
    {"controls::actuatorsOn", true, true, 0},
    {"controls::actuatorsOff", true, true, 0},
    {"controls::setGain", true, true, 0},
    {"controls::createEffect", true, true, 0},
    {"controls::startEffect", true, true, 0},
    {"controls::stopEffect", true, true, 0},
    {"controls::stopEffects", true, true, 0},
    {"controls::downloadEffect", true, true, 0},
    {"controls::unloadEffect", true, true, 0},
    // ebx, ecx, edx: no caller reads what the code leaves there (the last compare's, a callee's); the
    // game's callers (0x470b90, 0x4714f0, 0x4716e0, 0x471860, 0x4719f0, 0x471af0, 0x471fd0 and its
    // callers) save them or write them first.
    {"controls::updateEffect", true, true, 0, false, kNoHeap, false, "ebx ecx edx"},
    {"controls::setCooperativeLevel", true, true, 0},
    {"controls::acquireDevice", true, true, 0},
    {"controls::unacquireDevice", true, true, 0},
    {"controls::acquireResult", true, true, 0},
    {"controls::effectPlaying", true, true, 0},
    {"controls::hideCursor", true, true, 0},
    {"controls::joystickFound", true, true, 0},
    {"controls::axisProperty", true, true, 0},
#elif defined(CONTROLS_ARGUMENTS)
constexpr x86::reg32 kInputObject = 0x611000;    // DirectInput; its methods at +0x10
constexpr x86::reg32 kInputDevices = 0x611100;   // 18 devices of 0x10: 16 joysticks', the mouse's, the keyboard's
constexpr x86::reg32 kDeviceMethods = 0x611300;  // IDirectInputDevice2A's 27
constexpr x86::reg32 kInputEffects = 0x611400;   // 30 effects of 0x10
constexpr x86::reg32 kEffectMethods = 0x611600;  // IDirectInputEffect's 13
constexpr x86::reg32 kInputBuffers = 0x612000;   // what the arguments point at

/* DirectInput's objects and methods, the game's tables and the state of its
 * devices and effects, made up. */
void makeInputWorld(CheckApp& g)
{
    // The methods there are stand-ins for; the others none.
    static const x86::reg32 inputMethods[] = {8, 0xc, 0x10};
    static const x86::reg32 deviceMethods[] = {0, 8, 0xc, 0x14, 0x18, 0x1c, 0x20, 0x24, 0x28,
                                               0x2c, 0x30, 0x34, 0x38, 0x48, 0x58, 0x64};
    static const x86::reg32 effectMethods[] = {0x18, 0x1c, 0x20, 0x28, 0x2c};
    for (x86::reg32 at = kInputObject; at < kInputBuffers; at += 4)
        g.word(at) = 0;
    g.word(kInputObject) = kInputObject + 0x10;
    for (const x86::reg32 m : inputMethods)
        g.word(kInputObject + 0x10 + m) = standInFor(0xfff100 + m);
    for (x86::reg32 i = 0; i < 18; ++i)
        g.word(kInputDevices + 0x10 * i) = kDeviceMethods;
    for (const x86::reg32 m : deviceMethods)
        g.word(kDeviceMethods + m) = standInFor(0xfff120 + m);
    for (x86::reg32 i = 0; i < 30; ++i)
        g.word(kInputEffects + 0x10 * i) = kEffectMethods;
    for (const x86::reg32 m : effectMethods)
        g.word(kEffectMethods + m) = standInFor(0xfff190 + m);
    for (x86::reg32 at = kInputBuffers; at < 0x616000; at += 4)
        g.word(at) = random32();

    // The game's tables in its code: the effects' GUIDs by type, the hats'
    // and the axes' offsets, createEffect's jump table.
    static const x86::reg32 guids[] = {0x56b848, 0x56b858, 0x56b868, 0x56b878, 0x56b888, 0x56b898, 0x56b8a8,
                                       0x56b8b8, 0x56b8c8, 0x56b8c8, 0x56b8d8, 0x56b8e8, 0x56b8f8};
    static const x86::reg32 jumps[] = {0x436103, 0x43611d, 0x436129, 0x436135, 0x436135, 0x436135, 0x436135,
                                       0x436135, 0x43614f, 0x436169, 0x43614f, 0x43614f, 0x43614f};
    for (x86::reg32 i = 0; i < 13; ++i)
    {
        g.word(0x434ea0 + 4 * i) = guids[i];
        g.word(0x435ed8 + 4 * i) = jumps[i];
    }
    for (x86::reg32 i = 0; i < 8; ++i)
    {
        g.word(0x434ed4 + 4 * i) = 4 * i;
        g.word(0x434ef4 + 4 * i) = 4 * i;
    }

    // DirectInput, the mouse, the keyboard, there or not.
    g.word(0x554ee4) = chance(80) ? kInputObject : 0;
    g.word(0x554ee8) = chance(70) ? kInputDevices + 0x10 * 16 : 0;
    g.word(0x554eec) = chance(50) ? kInputDevices + 0x10 * 17 : 0;
    g.word(0x554ef0) = chance(50) ? 0x7000 + 4 * below(0x100) : 0;
    g.word(0x554ef4) = chance(60) ? standInFor(0xfff1c8) : 0;
    g.word(0x554ef8) = chance(25) ? 0 : below(17);
    g.word(0x554efc) = chance(10) ? 0 : below(31);
    g.word(0x554f00) = chance(50) ? 0 : chance(80) ? 1 : random32();
    // The joysticks: devices or none; their records, force feedback or not;
    // the effects' records, their parameters' sizes as createEffect has them.
    for (x86::reg32 i = 0; i < 16; ++i)
        g.word(0x5f1fb8 + 4 * i) = chance(70) ? kInputDevices + 0x10 * below(16) : 0;
    for (x86::reg32 at = 0x5f1fb0; at < 0x5f1fb8; at += 4)
        g.word(at) = random32();
    for (x86::reg32 at = 0x5f1ff8; at < 0x5f38e0; at += 4)
        g.word(at) = chance(20) ? 0 : chance(50) ? below(0x40) : random32();
    for (x86::reg32 i = 0; i < 17; ++i)
    {
        g.getMemory<x86::reg8>(0x5f1ff8 + 0x88 * i) = x86::reg8(below(3) << 3 | below(8));
        for (x86::reg32 at = 0x5f2030 + 0x88 * i; at < 0x5f203c + 0x88 * i; at += 4)
            g.word(at) = chance(50) ? 0 : random32();
    }
    static const x86::reg32 sizes[] = {4, 8, 0x10, 0x18, 0x30, 0};
    for (x86::reg32 i = 0; i < 30; ++i)
    {
        g.word(0x5f2878 + 4 * i) = kInputEffects + 0x10 * i;
        g.word(0x5f28f0 + 0x88 * i + 0x2c) = sizes[below(6)];
        g.word(0x5f28f0 + 0x88 * i + 0x80) = chance(50) ? below(2) : random32();
        g.word(0x5f28f0 + 0x88 * i + 0x84) = chance(30) ? 0 : below(13);
    }
    // The pointer, the buttons, the wheel; the buttons' handler.
    g.word(0x5f38e0) = chance(90) ? below(100) : random32();
    g.word(0x5f38e4) = chance(90) ? below(100) : random32();
    g.word(0x5f38e8) = chance(90) ? 400 + below(700) : random32();
    g.word(0x5f38ec) = chance(90) ? 300 + below(500) : random32();
    g.word(0x5f38f0) = chance(80) ? below(1000) : random32();
    g.word(0x5f38f4) = chance(80) ? below(800) : random32();
    g.word(0x5f38f8) = random32();
    g.word(0x5f38fc) = random32();
    g.word(0x5f3900) = chance(50) ? 0 : chance(80) ? 1 : random32();
    g.word(0x56446c) = chance(60) ? standInFor(0xfff1c4) : 0;
}

/* A device: a joystick mostly, all of them, the mouse, the keyboard,
 * every device, now and then past them. */
x86::reg32 aDevice(bool negative)
{
    if (chance(10))
        return 0x10;
    if (chance(8))
        return 0x13;
    if (chance(8))
        return 0x11 + below(2);
    if (chance(85))
        return below(16);
    return negative && chance(50) ? x86::reg32(-1 - x86::sreg32(below(100))) : 0x14 + below(100);
}

/* An effect's number in ax, the rest of eax anything: one of the 30 mostly. */
x86::reg32 anEffect()
{
    return (random32() & 0xffff0000) | (chance(10) ? 0x8000 | below(0x8000) : below(30));
}

bool makeInputArguments(CheckApp& g, const std::string& name, x86::CPU& cpu, x86::reg32* stack)
{
    static const char* const names[] = {
        "startDirectInput", "stopDirectInput", "openMouse", "setPointerBounds", "readMouse", "findJoysticks",
        "describeJoystick", "readJoystick", "readAxisZone", "setAxisZone", "resetForceFeedback", "setAutocenter",
        "pauseEffects", "continueEffects", "actuatorsOn", "actuatorsOff", "setGain", "createEffect",
        "startEffect", "stopEffect", "stopEffects", "downloadEffect", "unloadEffect", "updateEffect",
        "setCooperativeLevel", "acquireDevice", "unacquireDevice", "acquireResult", "effectPlaying",
        "hideCursor", "joystickFound", "axisProperty"};
    if (std::find_if(std::begin(names), std::end(names), [&](const char* n) { return name == n; })
        == std::end(names))
        return false;
    makeInputWorld(g);
    const x86::reg32 buffers = kInputBuffers;
    if (name == "setPointerBounds")
    {
        cpu.eax = chance(90) ? below(100) : random32();
        cpu.edx = chance(90) ? below(100) : random32();
        cpu.ebx = chance(90) ? 400 + below(700) : random32();
        cpu.ecx = chance(90) ? 300 + below(500) : random32();
    }
    else if (name == "readMouse")
    {
        cpu.eax = chance(20) ? 0 : buffers;
        cpu.edx = chance(20) ? 0 : buffers + 4;
        cpu.ebx = chance(20) ? 0 : buffers + 8;
        cpu.ecx = chance(20) ? 0 : buffers + 0xc;
    }
    else if (name == "describeJoystick")
        cpu.eax = chance(10) ? 0x10 : chance(85) ? below(16) : random32();
    else if (name == "readJoystick")
    {
        cpu.eax = chance(90) ? below(16) : random32();
        cpu.edx = chance(8) ? 0 : buffers + 0x100;
    }
    else if (name == "readAxisZone")
    {
        cpu.eax = below(16);
        cpu.edx = below(9);
        cpu.ebx = chance(50) ? 0 : chance(80) ? 1 : random32();
    }
    else if (name == "setAxisZone" || name == "axisProperty")
    {
        cpu.eax = chance(90) ? below(16) : random32();
        cpu.edx = chance(90) ? below(9) : x86::reg32(x86::sreg32(below(30)) - 8);
        cpu.ebx = chance(40) ? 0 : chance(70) ? 1 : random32();
        cpu.ecx = chance(40) ? 0 : chance(50) ? below(120) : random32();
        stack[0] = chance(80) ? x86::reg32(x86::sreg32(below(130)) - 10) : random32();
    }
    else if (name == "setAutocenter" || name == "pauseEffects" || name == "continueEffects"
             || name == "actuatorsOn" || name == "actuatorsOff" || name == "stopEffects")
    {
        cpu.eax = aDevice(true);
        cpu.edx = chance(30) ? 0 : chance(50) ? 1 : random32();
    }
    else if (name == "setGain")
    {
        cpu.eax = aDevice(true);
        cpu.edx = chance(80) ? x86::reg32(x86::sreg32(below(130)) - 10) : random32();
    }
    else if (name == "createEffect")
    {
        cpu.eax = chance(90) ? below(16) : random32();
        cpu.edx = buffers + 0x200;
        if (chance(10))
            g.word(0x554efc) = 0x1e + below(3);
        g.word(buffers + 0x200) = chance(90) ? below(13) : random32();
        g.word(buffers + 0x234) = chance(20) ? 0xffffff : chance(80) ? below(5000) : random32();
        g.word(buffers + 0x238) = below(10001);
        g.word(buffers + 0x23c) = chance(80) ? below(3) : random32();
        g.word(buffers + 0x240) = chance(30) ? 0xffffffff : chance(80) ? below(32) : random32();
        g.word(buffers + 0x248) = chance(80) ? below(361) : random32();
    }
    else if (name == "startEffect" || name == "stopEffect" || name == "downloadEffect" || name == "unloadEffect"
             || name == "effectPlaying")
        cpu.eax = anEffect();
    else if (name == "updateEffect")
    {
        cpu.eax = anEffect();
        const x86::reg32 record = 0x5f28f0 + 0x88 * (cpu.eax & 0xffff);
        const bool known = (cpu.eax & 0x8000) == 0;
        cpu.edx = chance(90) ? below(4) : random32();
        // Its direction, duration, gain, trigger: each as the record has it now and then.
        cpu.ebx = chance(25) ? 0 : buffers + 0x300;
        g.word(buffers + 0x300) = chance(80) ? x86::reg32(x86::sreg32(below(400)) - 20) : random32();
        if (known && chance(30))
            g.word(record + 0x78) = std::min<x86::sreg32>(std::max<x86::sreg32>(g.word(buffers + 0x300), 0), 360) * 100;
        cpu.ecx = chance(25) ? 0 : buffers + 0x304;
        g.word(buffers + 0x304) = chance(20) ? 0xffffff : chance(80) ? below(1600) : random32();
        if (known && chance(30))
            g.word(record + 8) = g.word(buffers + 0x304) == 0xffffff
                                     ? 0xffffffff
                                     : std::min<x86::reg32>(g.word(buffers + 0x304), 1500) * 1000;
        stack[0] = chance(25) ? 0 : buffers + 0x308;
        g.word(buffers + 0x308) = chance(80) ? below(120) : random32();
        if (known && chance(30))
            g.word(record + 0x10) = std::min<x86::reg32>(g.word(buffers + 0x308) * 100, 10000);
        // Its parameters: the record's, or others.
        stack[1] = chance(25) ? 0 : buffers + 0x340;
        for (x86::reg32 i = 0; i < 0x30; i += 4)
            g.word(buffers + 0x340 + i) = known && chance(50) ? g.word(record + 0x34 + i) : random32();
        if (chance(30))
            g.getMemory<x86::reg8>(buffers + 0x340 + below(0x30)) ^= x86::reg8(1 + below(0xff));
        stack[2] = chance(25) ? 0 : buffers + 0x30c;
        g.word(buffers + 0x30c) = chance(30) ? 0xffffffff : chance(80) ? below(40) : random32();
        // Its envelope: the record's, or another.
        stack[3] = chance(25) ? 0 : buffers + 0x310;
        for (x86::reg32 i = 0; i < 0x10; i += 4)
            g.word(buffers + 0x310 + i) = known && chance(70) ? g.word(record + 0x68 + i) : random32();
    }
    else if (name == "setCooperativeLevel")
    {
        cpu.eax = aDevice(false);  // a joystick's below 0 is read far outside the table
        cpu.edx = chance(70) ? 1 + below(16) : random32();
        // Its device there: the code reads through it otherwise.
        if (cpu.eax == 0x11 && g.word(0x554ee8) == 0)
            g.word(0x554ee8) = kInputDevices + 0x10 * 16;
        if (cpu.eax == 0x12 && g.word(0x554eec) == 0)
            g.word(0x554eec) = kInputDevices + 0x10 * 17;
    }
    else if (name == "acquireDevice")
        cpu.eax = aDevice(false);
    else if (name == "unacquireDevice")
        cpu.eax = aDevice(true);
    else if (name == "acquireResult")
    {
        static const x86::reg32 results[] = {0, 1, 0x80070057, 0x80070005, 0x8007001e, 2};
        cpu.eax = chance(90) ? results[below(6)] : random32();
    }
    else if (name == "joystickFound")
    {
        // A DIDEVICEINSTANCE: its subtype a stick, a gamepad, a wheel...; its
        // product's name of any length.
        const x86::reg32 instance = buffers + 0x400;
        g.word(instance + 0x24) = (random32() & 0xffff0000) | (chance(80) ? 2 + below(6) : below(0x100)) << 8 | 4;
        const x86::reg32 length = chance(10) ? 0 : below(0x50);
        g.text(instance + 0x12c, length, random32());
        stack[0] = instance;
        stack[1] = kInputObject;
    }
    return true;
}
#endif
