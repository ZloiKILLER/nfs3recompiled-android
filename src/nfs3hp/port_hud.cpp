#include "port_hud.h"
#include "port_text.h"

#include <algorithm>
#include <cstdint>
#include <cstring>

/* The race HUD, rewritten (port.h says how; port_hud.h says what it holds).
 * The arithmetic is double, rounded to a float where the game stores a float
 * -- no cpu.fpu.  Each function is a typed core and an adapter under the
 * game's name; a function that keeps working on the game's stack keeps its
 * values there as the code does. */
#if defined(__clang__)
#pragma clang fp contract(off)
#else
#pragma fp_contract(off)
#endif

namespace nfs3hp
{

// nfs3hp_main.cpp: the size a 4:3 screen fitted to the real one would have (tools/apply_hud_scale.py).
x86::reg32 hudReferenceWidth(win32::WinApplication* app);
x86::reg32 hudReferenceHeight(win32::WinApplication* app);

namespace port
{
namespace hud
{

namespace
{

float bitsToFloat(reg32 bits)
{
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

/* ---- 0x484040 ---- */

constexpr reg32 kTextBegin = 0x4d0d50;               // the text started
constexpr reg32 kSetTextScale = 0x4d1390;            // (float) stdcall: the scale before, on the x87's stack
constexpr reg32 kTextPrepared = 0x4a33d0;
constexpr reg32 kTextEnd = 0x4d1310;
constexpr Var<reg32> kTextColour{0x561b34};

/* The loading screen's text set up: grey, at the scale 1.0 (the old scale
 * the game puts on the x87's stack is dropped). */
bool loadingText(App* app, CPU& cpu, const Entry& e)
{
    const reg32 ebp = e.frame(2);
    const HeldEbp held(cpu, ebp);
    if (!callGame(app, cpu, ebp, kTextBegin))
        return false;
    kTextColour.set(app, 0xffa0a0a0);
    if (!callGame(app, cpu, ebp, kSetTextScale, {}, {0x3f800000}))
        return false;
    cpu.fpu.pop();
    if (!callGame(app, cpu, ebp, kTextPrepared))
        return false;
    return callGame(app, cpu, ebp, kTextEnd);
}

/* ---- 0x485c20 ---- */

/* The float at `b` into `a`, rounded through the x87 as a float load and store
 * does, and a's old bits into b. */
void swapFloats(App* app, reg32 a, reg32 b)
{
    const float second = float(double(load<float>(app, b)));
    const reg32 first = load<reg32>(app, a);
    app->getMemory<float>(a) = second;
    app->getMemory<reg32>(b) = first;
}

/* ---- 0x487350 ---- */

constexpr Table<reg32> kRecordCounts{0x55d694};      // by group: how many records it has
constexpr reg32 kRecords = 0x791b68;                 // by group (80 bytes) and record (16 bytes): four floats

/* Four integers added to group `group`'s records as four floats, its count up
 * by one.  The code keeps them on its stack. */
void fourAppended(App* app, const Entry& e, reg32 group, int32_t first, int32_t second, int32_t third, int32_t fourth)
{
    const reg32 ebp = e.frame(3);
    app->getMemory<reg32>(ebp - 0xc) = reg32(first);
    app->getMemory<reg32>(ebp - 8) = reg32(second);
    app->getMemory<reg32>(ebp - 4) = reg32(third);
    app->getMemory<reg32>(ebp - 4) = reg32(fourth);
    const reg32 count = kRecordCounts(app, int32_t(group));
    const reg32 record = kRecords + (count << 4) + 80 * group;
    app->getMemory<float>(record + 4) = float(double(second));
    app->getMemory<float>(record + 8) = float(double(third));
    app->getMemory<float>(record + 0xc) = float(double(fourth));
    app->getMemory<float>(record + 0) = float(double(first));
    kRecordCounts.set(app, int32_t(group), count + 1);
}

/* ---- 0x48cb50 ---- */

constexpr Var<int32_t> kHudRow{0x79c10c}, kHudColumn{0x79c110};
constexpr Var<float> kHudScale{0x79c114}, kHudOffsetX{0x79c11c}, kHudOffsetY{0x79c120};
constexpr reg32 kHudModes = 0x6fbc70;                // by column (0x348) and row (0x1a4): 1 turned
constexpr Var<float> kTurnCosine{0x7930fc}, kTurnSine{0x793100};
constexpr Var<reg32> kMirrored{0x6fd4c4};
constexpr Table<float> kHudScaleX{0x792878}, kHudCentreY{0x792888}, kHudCentreX{0x792890};

/* "indefinite" -- the NaN the x87 makes -- as a float's bits. */
bool isIndefinite(reg32 bits)
{
    return (bits | 0x80000000) == 0xffc00000;
}

/* The point (x, y) -- the stack's two floats -- moved by the HUD's offset,
 * scaled, turned if the HUD's mode is 1 (an indefinite result taken for 0),
 * and put through the gauge's own centre and scale into the two floats at
 * `out`.  The scaled point is written back to the stack as the code does. */
void pointMoved(App* app, const Entry& e, reg32 gauge, reg32 xAt, reg32 yAt, reg32 out)
{
    const reg32 ebp = e.frame(5);
    const double scale = double(kHudScale(app));
    const float x = float(scale * (double(load<float>(app, xAt)) + double(kHudOffsetX(app))));
    const float y = float((double(load<float>(app, yAt)) + double(kHudOffsetY(app))) * scale);
    app->getMemory<float>(yAt) = y;
    app->getMemory<float>(xAt) = x;
    float turnedX, turnedY;
    const reg32 mode = app->getMemory<reg32>(kHudModes + reg32(kHudColumn(app)) * 0x348 + reg32(kHudRow(app)) * 0x1a4);
    if (mode == 1)
    {
        const double cosine = double(kTurnCosine(app)), sine = double(kTurnSine(app));
        turnedX = float(double(x) * cosine - double(y) * sine);
        app->getMemory<float>(ebp - 4) = turnedX;
        if (isIndefinite(app->getMemory<reg32>(ebp - 4)))
            app->getMemory<reg32>(ebp - 4) = 0;
        turnedY = float(sine * double(x) + cosine * double(y));
        app->getMemory<float>(ebp - 8) = turnedY;
        if (isIndefinite(app->getMemory<reg32>(ebp - 8)))
            app->getMemory<reg32>(ebp - 8) = 0;
    }
    else
    {
        app->getMemory<float>(ebp - 4) = x;
        app->getMemory<float>(ebp - 8) = y;
    }
    app->getMemory<float>(ebp - 0xc) = kMirrored(app) != 0 ? -1.0f : 1.0f;
    const double sign = double(app->getMemory<float>(ebp - 0xc));
    const double gaugeScale = double(kHudScaleX(app, int32_t(gauge)));
    app->getMemory<float>(out) = float(((sign * gaugeScale) * double(app->getMemory<float>(ebp - 4))) +
                                       double(kHudCentreX(app, int32_t(gauge))));
    app->getMemory<float>(out + 4) =
        float(double(kHudCentreY(app, int32_t(gauge))) - gaugeScale * double(app->getMemory<float>(ebp - 8)));
}

/* ---- 0x48cc60 ---- */

constexpr Var<float> kBoxHalf{0x53b9ac};
constexpr Table<reg32> kBoxColours{0x792880};
constexpr reg32 kBoxQuad = 0x487620;                 // (gauge, colour...; ten words) stdcall
constexpr reg32 kBoxText = 0x76da48;

/* A box round (x, y), `a2` and `a3` away from it on each axis (times the HUD's
 * constant), put to the quad drawer with the corners' texture coordinates and
 * the gauge's colour.  False when the game stops in the call. */
bool boxDrawnOf(App* app, CPU& cpu, const Entry& e, reg32& result)
{
    const reg32 gauge = e.eax;
    const reg32 ebp = e.frame(5);
    const HeldEbp held(cpu, ebp);
    const double half = double(kBoxHalf(app));
    const float a0 = bitsToFloat(e.arg(app, 0)), a1 = bitsToFloat(e.arg(app, 1));
    const float a2 = bitsToFloat(e.arg(app, 2)), a3 = bitsToFloat(e.arg(app, 3));
    const float low = float(double(a0) - half * double(a2));
    const float high = float(half * double(a2) + double(a0));
    const float far = float(double(a3) * half + double(a1));
    const float near = float(double(a1) - double(a3) * half);
    const reg32 edgeA = 0x3c800000, edgeB = 0x3f7c0000;
    app->getMemory<float>(ebp - 0x10) = low;
    app->getMemory<float>(ebp - 8) = high;
    app->getMemory<float>(ebp - 0xc) = far;
    app->getMemory<float>(ebp - 0x14) = near;
    app->getMemory<reg32>(ebp - 0x24) = edgeA;
    app->getMemory<reg32>(ebp - 0x20) = edgeA;
    app->getMemory<reg32>(ebp - 0x1c) = edgeB;
    app->getMemory<reg32>(ebp - 0x18) = edgeB;
    const reg32 base = e.frame(5, 0x24);
    const reg32 colour = kBoxColours(app, int32_t(gauge));
    return callGame(app, cpu, base, kBoxQuad, {gauge, colour, load<reg32>(app, ebp - 0x14), load<reg32>(app, ebp - 0x10)},
                    {load<reg32>(app, ebp - 0x14), load<reg32>(app, ebp - 0x10), load<reg32>(app, ebp - 0xc),
                     load<reg32>(app, ebp - 8), edgeA, edgeA, edgeB, edgeB, e.edx, kBoxText}, result);
}

/* ---- Shared by the text-drawing ones ---- */

reg32 bitsOfFloat(float value)
{
    reg32 bits;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}

constexpr guest::Native kLanguageText = text::languageText;     // 0x4d1850: line eax of the language's text
constexpr reg32 kTextWidth = 0x4d1480;               // (text): its width, on the x87's stack
constexpr reg32 kTextDrawn = 0x4d1000;               // (text, x, y): drawn, a float back on the x87's stack
constexpr reg32 kTextCentred = 0x4d10e0;             // (text): drawn at the middle
constexpr reg32 kFontHeight = 0x4d1540;              // (font): its height
constexpr reg32 kFontWidth = 0x4d14f0;               // (font)
constexpr reg32 kAlignedText = 0x4897f0;             // this file's own, stdcall 6 words
constexpr reg32 kSprintf = 0x4df690;

/* fistp after Watcom's __CHP: towards zero, or the x87's "integer indefinite". */
int32_t chopped(double value)
{
    if (!(value > -2147483649.0 && value < 2147483648.0))
        return int32_t(0x80000000);
    return int32_t(value);
}

/* ---- 0x48a220 ---- */

constexpr Var<reg32> kIndicators{0x791b38};          // the indicators' records, 256 bytes each
constexpr Var<float> kIndicatorSize{0x791b14};
constexpr Var<float> kScaleA{0x53b87c}, kScaleB{0x53b880}, kScaleC{0x53b884}, kScaleD{0x53b888}, kScaleE{0x53b88c};
constexpr reg32 kSineCosine = 0x4ea970;              // (&angle, &out1, &out2)
constexpr reg32 kArcTangent = 0x497590;              // (1.0, q) stdcall: on the x87's stack

/* An indicator's corners worked out from its angle: the record's colours set,
 * the angle (times a constant, plus another) put through the sine and cosine
 * game function, two points of the pointer out of them, and the needle's two
 * others at the slope the arc tangent gives.  The code keeps its working on
 * its stack.  False when the game stops in a call. */
bool indicatorGeometry(App* app, CPU& cpu, const Entry& e, reg32& result)
{
    const reg32 ebp = e.frame(6);
    const reg32 base = e.frame(6, 0x1c);
    const HeldEbp held(cpu, ebp);
    const reg32 index = e.eax;
    const float angle = bitsToFloat(e.arg(app, 0));
    const reg32 colour = e.arg(app, 1);
    const reg32 record = kIndicators(app) + (index << 8);
    const reg32 centre = record + 0xc0, upper = record + 0x80, left = record + 0xa0, right = record + 0xe0;
    app->getMemory<reg32>(ebp - 4) = right;
    for (const reg32 at : {record + 0x70, record + 0x50, record + 0x30, record + 0x10})
        app->getMemory<reg32>(at) = colour;
    app->getMemory<float>(ebp - 0x1c) = float(double(angle) * double(kScaleA(app)) + double(kScaleB(app)));
    reg32 ignored;
    if (!callGame(app, cpu, base, kSineCosine, {ebp - 0x1c, ebp - 0x18, ebp - 0xc, centre}, {}, ignored))
        return false;
    const double size = double(kIndicatorSize(app));
    const double reach = size * double(kScaleC(app));
    app->getMemory<float>(upper) = float(reach * double(app->getMemory<float>(ebp - 0xc)) + double(load<float>(app, centre)));
    app->getMemory<float>(upper + 4) =
        float(reach * double(app->getMemory<float>(ebp - 0x18)) + double(load<float>(app, centre + 4)));
    if ((app->getMemory<reg32>(ebp - 0x18) & 0x7ffffff) == 0)
        app->getMemory<reg32>(ebp - 0x18) = 0x800000;
    const double slope = double(app->getMemory<float>(ebp - 0xc)) / double(app->getMemory<float>(ebp - 0x18));
    if (!callGame(app, cpu, base, kArcTangent, {}, {0x3f800000, bitsOfFloat(float(slope))}, ignored))
        return false;
    const double arc = double(cpu.fpu.st(0));
    cpu.fpu.pop();
    const double inverse = 1.0 / arc;
    const double slope2 = double(app->getMemory<float>(ebp - 0xc)) / double(app->getMemory<float>(ebp - 0x18));
    const double wide = (size * double(kScaleD(app))) * double(kScaleE(app));
    const double along = inverse * wide;
    const double across = (-slope2) * along;
    app->getMemory<float>(left) = float(double(load<float>(app, centre)) + along);
    app->getMemory<float>(left + 4) = float(double(load<float>(app, centre + 4)) + across);
    app->getMemory<float>(app->getMemory<reg32>(ebp - 4)) = float(double(load<float>(app, centre)) - along);
    app->getMemory<float>(app->getMemory<reg32>(ebp - 4) + 4) = float(double(load<float>(app, centre + 4)) - across);
    result = app->getMemory<reg32>(ebp - 4);
    return true;
}

/* ---- 0x4897f0 ---- */

constexpr Var<reg32> kScreenRight{0x7cdac8};
constexpr Var<float> kAlignPad{0x53b7dc};
constexpr Var<double> kCentreFactor{0x53b7e4};
constexpr reg32 kTextScale = 0x4d1390;               // (float) stdcall: the scale before, on the x87's stack

/* A text drawn at the scale and in the colour given: at x as it is (mode 0),
 * pushed left to end at the screen's edge (mode 1) or centred round x + dx
 * (any other, mode 2 aside, which is drawn centred by the game's own).  Its
 * width is the x87 value the text function leaves on the stack.  The caller's
 * x is written back, as the code does.  False when the game stops. */
bool textAligned(App* app, CPU& cpu, const Entry& e, reg32& result)
{
    const reg32 text = e.eax;
    const reg32 ebp = e.frame(5);
    const reg32 base = e.frame(5, 0xc);
    const HeldEbp held(cpu, ebp);
    const reg32 xAt = e.esp + 4 + 8;
    const reg32 scale = e.arg(app, 0), colour = e.arg(app, 1);
    const reg32 dx = e.arg(app, 3), y = e.arg(app, 4), mode = e.arg(app, 5);
    kTextColour.set(app, colour);
    if (!callGame(app, cpu, base, kTextScale, {text, dx, y, 0}, {scale}, result))
        return false;
    cpu.fpu.pop();
    reg32 x;
    if (mode == 1)
    {
        if (!callGame(app, cpu, base, kTextWidth, {text, dx, y, text}, {}, result))
            return false;
        app->getMemory<reg32>(ebp - 4) = app->getMemory<reg32>(xAt);
        const float width = float(cpu.fpu.st(0));
        cpu.fpu.pop();
        app->getMemory<float>(ebp - 8) = width;
        const double sum = double(int32_t(app->getMemory<reg32>(ebp - 4))) + double(width);
        const float limit = float(double(int32_t(kScreenRight(app))));
        app->getMemory<float>(ebp - 0xc) = limit;
        if (sum >= double(limit))
            app->getMemory<int32_t>(xAt) =
                chopped(double(app->getMemory<float>(ebp - 0xc)) - double(app->getMemory<float>(ebp - 8)) + double(kAlignPad(app)));
        x = app->getMemory<reg32>(xAt);
    }
    else if (mode == 2)
        return callGame(app, cpu, base, kTextCentred, {text, dx, y, text}, {}, result);
    else
    {
        if (!callGame(app, cpu, base, kTextWidth, {text, dx, y, text}, {}, result))
            return false;
        const reg32 sum = app->getMemory<reg32>(xAt) + cpu.edx;
        app->getMemory<reg32>(ebp - 4) = sum;
        const double width = double(cpu.fpu.st(0));
        cpu.fpu.pop();
        app->getMemory<int32_t>(ebp - 4) = chopped((double(int32_t(sum)) - width) * kCentreFactor(app));
        x = app->getMemory<reg32>(ebp - 4);
    }
    if (!callGame(app, cpu, base, kTextDrawn, {text, x, y, text}, {}, result))
        return false;
    cpu.fpu.pop();
    return true;
}

/* ---- 0x489d70 ---- */

constexpr Var<float> kPercentA{0x53b83c}, kPercentB{0x53b840};
constexpr reg32 kPercentFormat = 0x53b7f4;

/* A percentage -- the float given times a constant, chopped -- written with the
 * language's line, and drawn aligned.  False when the game stops. */
bool percentDrawn(App* app, CPU& cpu, const Entry& e, reg32& result)
{
    const reg32 ebp = e.frame(5);
    const reg32 base = e.frame(5, 0x10);
    const HeldEbp held(cpu, ebp);
    const reg32 buffer = ebp - 0x10;
    const bool first = e.arg(app, 1) != 0;
    const reg32 y = e.arg(app, 4);          // (the code takes it before the calls)
    reg32 line;
    if (!callGame(app, cpu, base, kLanguageText, {first ? 0x156u : 0x157u, 0, 0, 0}, {}, line))
        return false;
    const double scaled = double(bitsToFloat(e.arg(app, 0))) * double(first ? kPercentA(app) : kPercentB(app));
    const int32_t value = chopped(scaled);
    app->getMemory<int32_t>(ebp - 4) = value;
    if (!callGame(app, cpu, base, kSprintf, {buffer, 0, 0, 0}, {buffer, kPercentFormat, reg32(value), line}, result))
        return false;
    return callGame(app, cpu, base, kAlignedText, {buffer, y, 0, 0},
                    {e.arg(app, 2), e.arg(app, 3), y, y, e.arg(app, 5), 2}, result);
}

/* ---- 0x489b30 ---- */

constexpr reg32 kRowFormat = 0x53b820;

/* A list's rows down the screen: a title line, a numbered line for each of
 * `count` rows, with the chosen one and the last in colours of their own, and
 * a footer.  The y of each is a float stepped by the box's height over the
 * rows and two. */
bool rowsDrawn(App* app, CPU& cpu, const Entry& e, reg32& result)
{
    const reg32 ebp = e.frame(4);
    const reg32 base = e.frame(4, 0x1c);
    const HeldEbp held(cpu, ebp);
    const reg32 buffer = ebp - 4;
    const int32_t count = int32_t(e.eax);
    const int32_t chosen = int32_t(e.edx) - 1;
    const reg32 total = e.ebx;
    const reg32 scale = e.arg(app, 0), colourRow = e.arg(app, 1), colourChosen = e.arg(app, 2), colourLast = e.arg(app, 3);
    const int32_t top = int32_t(e.arg(app, 4)), bottom = int32_t(e.arg(app, 5));
    const reg32 x = e.arg(app, 6);
    app->getMemory<reg32>(ebp - 0x18) = e.eax;
    app->getMemory<reg32>(ebp - 0x1c) = e.ebx;
    const int32_t span = bottom - top;
    app->getMemory<int32_t>(ebp - 0xc) = span;
    app->getMemory<int32_t>(ebp - 8) = int32_t(e.eax) + 2;
    const double step = double(span) / double(int32_t(e.eax) + 2);
    app->getMemory<int32_t>(ebp - 8) = top;
    app->getMemory<float>(ebp - 0x10) = float(double(top));
    app->getMemory<float>(ebp - 0x14) = float(step);
    reg32 ignored;
    if (!callGame(app, cpu, base, kTextScale, {e.eax, 0, 0, 0}, {scale}, ignored))
        return false;
    cpu.fpu.pop();
    kTextColour.set(app, chosen == 0 ? colourChosen : colourRow);
    const auto advance = [&]() {
        app->getMemory<float>(ebp - 0x10) =
            float(double(app->getMemory<float>(ebp - 0x10)) + double(app->getMemory<float>(ebp - 0x14)));
    };
    const auto lineAt = [&](reg32 text, reg32 ecx) {
        const int32_t y = chopped(double(app->getMemory<float>(ebp - 0x10)));
        app->getMemory<int32_t>(ebp - 0xc) = y;
        if (!callGame(app, cpu, base, kTextDrawn, {text, reg32(y), x, ecx}, {}, ignored))
            return false;
        cpu.fpu.pop();
        advance();
        return true;
    };
    // The title: the language's line 0x15a, at the first y.
    reg32 text;
    {
        const int32_t y = chopped(double(app->getMemory<float>(ebp - 0x10)));
        app->getMemory<int32_t>(ebp - 0xc) = y;
        if (!callGame(app, cpu, base, kLanguageText, {0x15a, reg32(y), x, 1}, {}, text))
            return false;
    }
    if (!callGame(app, cpu, base, kTextDrawn, {text, reg32(chopped(double(app->getMemory<float>(ebp - 0x10)))), x, 1}, {}, ignored))
        return false;
    cpu.fpu.pop();
    advance();
    for (int32_t row = 1; row <= count; ++row)
    {
        const reg32 colour = row == chosen ? colourChosen : (row == int32_t(total) - 1 ? colourLast : colourRow);
        kTextColour.set(app, colour);
        if (!callGame(app, cpu, base, kSprintf, {buffer, 0, x, reg32(row)}, {buffer, kRowFormat, reg32(row)}, ignored))
            return false;
        if (!lineAt(buffer, reg32(row)))
            return false;
    }
    kTextColour.set(app, chosen == -1 ? colourChosen : colourRow);
    const int32_t y = chopped(double(app->getMemory<float>(ebp - 0x10)));
    app->getMemory<int32_t>(ebp - 0xc) = y;
    if (!callGame(app, cpu, base, kLanguageText, {0x15b, reg32(y), x, reg32(count + 1)}, {}, text))
        return false;
    if (!callGame(app, cpu, base, kTextDrawn, {text, reg32(y), x, reg32(count + 1)}, {}, result))
        return false;
    cpu.fpu.pop();
    return true;
}

/* ---- 0x489c40 ---- */

constexpr Var<reg32> kLanguage{0x6fbb4c};
constexpr reg32 kTimeFormatA = 0x53b824, kTimeFormatB = 0x53b830;

/* A time in milliseconds written as seconds and tenths with the language's
 * line, drawn aligned, and under it the language's next line, a font's height
 * down.  False when the game stops. */
bool timeDrawn(App* app, CPU& cpu, const Entry& e, reg32& result)
{
    const reg32 ebp = e.frame(6);
    const reg32 base = e.frame(6, 0x108);
    const HeldEbp held(cpu, ebp);
    const reg32 buffer = ebp - 0x108;
    const int32_t time = int32_t(e.eax);
    const reg32 colour = e.arg(app, 0), colourSecond = e.arg(app, 1), x = e.arg(app, 2), y = e.arg(app, 3), scale = e.arg(app, 4);
    const reg32 language = kLanguage(app);
    const reg32 format = (language == 1 || language == 2 || language == 5) ? kTimeFormatA : kTimeFormatB;
    reg32 line;
    if (!callGame(app, cpu, base, kLanguageText, {0x15c, 0, 0, 0}, {}, line))
        return false;
    const reg32 tenths = reg32((time / 100) % 10), seconds = reg32(time / 1000);
    if (!callGame(app, cpu, base, kSprintf, {buffer, 0, 0, 0}, {buffer, format, seconds, tenths, line}, result))
        return false;
    if (!callGame(app, cpu, base, kAlignedText, {buffer, x, 0, 0}, {colour, x, y, y, scale, 2}, result))
        return false;
    reg32 height, width;
    if (!callGame(app, cpu, base, kFontHeight, {0x39, 0, 0, 0}, {}, height))
        return false;
    const reg32 down = scale + height;
    if (!callGame(app, cpu, base, kFontWidth, {0x39, down, 0, 0}, {}, width))
        return false;
    const reg32 sum = width + down;
    app->getMemory<reg32>(ebp - 4) = sum;
    const float before = float(double(int32_t(sum)));
    app->getMemory<float>(ebp - 8) = before;
    reg32 second;
    if (!callGame(app, cpu, base - 4, kFontWidth, {0x39, down, 0, 0}, {}, second))
        return false;
    app->getMemory<reg32>(ebp - 4) = second;
    const int32_t middle = chopped(double(app->getMemory<float>(ebp - 8)) - double(int32_t(second)));
    app->getMemory<int32_t>(ebp - 4) = middle;
    if (!callGame(app, cpu, base - 4, kLanguageText, {0x15d, 0, 0, 0}, {}, line))
        return false;
    return callGame(app, cpu, base, kAlignedText, {line, x, 0, 0}, {colourSecond, x, y, y, reg32(middle), 2}, result);
}

/* ---- 0x48ea20 ---- */

constexpr Table<reg32> kScaleSlots{0x7925d8};        // by view (16 bytes): four floats' bits
constexpr reg32 kCameraTarget = 0x422bd0;            // (view): its record
constexpr Var<reg32> kScreens{0x6fd3b0};
constexpr reg32 kViewStates = 0x6fbc50;              // by view (0x348), side (0x1a4): the states
constexpr Var<float> kStepUp{0x53b9f4}, kStepDown{0x53b9f8}, kScaleFactor{0x53b9fc};
constexpr Table<reg32> kViewScaleOut{0x792878}, kViewCentreXOut{0x792888}, kViewCentreYOut{0x792890};
constexpr reg32 kViewRefresh = 0x48ccf0;             // (view)

/* The view's zoom stepped up (mode 0, to at most 4.0) or down (mode 1, to at
 * least 1.0), and the scale and the centre it makes written for the view.
 * False when the game stops in a call. */
bool zoomStepped(App* app, CPU& cpu, const Entry& e, reg32& result)
{
    const reg32 view = e.eax, mode = e.edx;
    const reg32 ebp = e.frame(4);
    const reg32 base = e.frame(4, 0x1c);
    const HeldEbp held(cpu, ebp);
    const reg32 slots = kScaleSlots.at + (view << 4);
    app->getMemory<reg32>(ebp - 0x18) = app->getMemory<reg32>(slots + 4);
    app->getMemory<reg32>(ebp - 4) = app->getMemory<reg32>(slots + 0xc);
    app->getMemory<reg32>(ebp - 0x14) = app->getMemory<reg32>(slots);
    app->getMemory<reg32>(ebp - 0x1c) = app->getMemory<reg32>(slots + 8);
    reg32 record;
    if (!callGame(app, cpu, base, kCameraTarget, {view, view << 4, mode, view}, {}, record))
        return false;
    const reg32 second = (app->getMemory<uint8_t>(record + 0x200) & 0x20) != 0 ? 1 : 0;
    const reg32 screens = kScreens(app);
    const reg32 first = screens == 1 ? 1 : 0;
    const reg32 state = kViewStates + 0x348 * (view + first) + 0x1a4 * second + 0x10;
    if (mode == 0)
    {
        app->getMemory<float>(state) = float(double(app->getMemory<float>(state)) + double(kStepUp(app)));
        if (app->getMemory<int32_t>(state) > 0x40800000)
            app->getMemory<reg32>(state) = 0x40800000;
    }
    else if (mode == 1)
    {
        app->getMemory<float>(state) = float(double(app->getMemory<float>(state)) + double(kStepDown(app)));
        if (app->getMemory<int32_t>(state) < 0x3f800000)
            app->getMemory<reg32>(state) = 0x3f800000;
    }
    const float a = float(double(app->getMemory<float>(ebp - 0x1c)) - double(app->getMemory<float>(ebp - 0x14)));
    app->getMemory<float>(ebp - 8) = a;
    const float b = float(double(app->getMemory<float>(ebp - 4)) - double(app->getMemory<float>(ebp - 0x18)));
    app->getMemory<float>(ebp - 0x10) = b;
    app->getMemory<float>(ebp - 0xc) = double(a) >= double(b) ? b : a;
    const double factor = double(kScaleFactor(app));
    app->getMemory<float>(kViewScaleOut.at + 4 * view) =
        float((double(app->getMemory<float>(ebp - 0xc)) * factor) * double(app->getMemory<float>(state)));
    if (app->getMemory<reg32>(state) == 0x3f800000)
    {
        app->getMemory<float>(kViewCentreXOut.at + 4 * view) =
            float((double(app->getMemory<float>(ebp - 0x14)) + double(app->getMemory<float>(ebp - 0x1c))) * factor);
        app->getMemory<float>(kViewCentreYOut.at + 4 * view) =
            float(factor * (double(app->getMemory<float>(ebp - 0x18)) + double(app->getMemory<float>(ebp - 4))));
    }
    return callGame(app, cpu, base, kViewRefresh, {view, 0, 0, 0}, {}, result);
}

/* ---- 0x48e790 ---- */

constexpr reg32 kPaneAt = 0x481f90, kPaneSize = 0x4853c0, kPaneDrawn = 0x4844c0;
constexpr Table<reg32> kPaneResults{0x7925d0};

/* A view's pane state moved on one step: off to 2 (a fresh pane, its rectangle
 * set by the view's number), to 1 (opening), and back to 2 (settled, from the
 * saved rectangle) or 0; then the pane refreshed.  False when the game stops. */
bool paneStepped(App* app, CPU& cpu, const Entry& e, reg32& result)
{
    const reg32 view = e.eax;
    const reg32 ebp = e.frame(6);
    const reg32 base = e.frame(6, 8);
    const HeldEbp held(cpu, ebp);
    reg32 record;
    if (!callGame(app, cpu, base, kCameraTarget, {view, e.edx, e.ebx, view}, {}, record))
        return false;
    const reg32 second = (app->getMemory<uint8_t>(record + 0x200) & 0x20) != 0 ? 1 : 0;
    const reg32 first = kScreens(app) == 1 ? 1 : 0;
    const reg32 index = view + first;
    const reg32 height = second != 0 ? 0x16 : 0x11;
    app->getMemory<reg32>(ebp - 4) = index * 0x348;
    app->getMemory<reg32>(ebp - 8) = 0x6fbc50 + index * 0x348;
    app->getMemory<reg32>(ebp - 4) = second * 0x1a4;
    const reg32 states = 0x6fbc50 + index * 0x348 + second * 0x1a4;
    app->getMemory<reg32>(ebp - 8) = height << 4;
    app->getMemory<reg32>(ebp - 4) = states;
    const reg32 rect = states + 0x24 + (height << 4), word = states + 0x20;
    if (view != 0 && kScreens(app) != 1)
    {
        result = rect;
        return true;
    }
    const auto get32 = [&](reg32 at) -> reg32 { return app->getMemory<reg32>(at); };
    const auto put32 = [&](reg32 at, reg32 value) { app->getMemory<reg32>(at) = value; };
    if (get32(word) == 0)
    {
        put32(word, 2);
        put32(rect, 0);
        put32(rect + 8, 0x3f800000);
        put32(rect + 4, get32(rect));
        app->getMemory<float>(rect + 0xc) = float(double(app->getMemory<float>(rect + 8)));
        if (index == 1)
            put32(rect + 8, 0x3f000000);
        else if (index == 2)
        {
            put32(rect + 8, 0x3f800000);
            put32(rect, 0x3f000000);
        }
        put32(states + 0x10, 0x3f800000);
    }
    else if (get32(word) != 1)
        put32(word, 1);
    else if ((get32(rect + 4) & 0x7fffffff) == 0 && get32(rect + 0xc) == 0x3f800000)
    {
        put32(word, 2);
        put32(rect + 4, get32(states + 0x198));
        put32(rect, get32(states + 0x194));
        put32(rect + 0xc, get32(states + 0x1a0));
        put32(rect + 8, get32(states + 0x19c));
        put32(states + 0x10, 0x40800000);
    }
    else
        put32(word, 0);
    reg32 pane;
    if (!callGame(app, cpu, base, kPaneAt, {view, 0, 0, 0}, {}, pane))
        return false;
    kPaneResults.set(app, int32_t(view), pane);
    if (!callGame(app, cpu, base, kPaneSize, {view, height, 0, 0}, {}, result))
        return false;
    if (!callGame(app, cpu, base, kPaneDrawn, {view, height, 0, 0}, {}, result))
        return false;
    return callGame(app, cpu, base, kViewRefresh, {view, height, 0, 0}, {}, result);
}

/* ---- 0x484b70 ---- */

constexpr Var<int32_t> kScreenWidth{0x7cdac8}, kScreenHeight{0x7cdacc};
constexpr Var<float> kWidthScaleA{0x53b620}, kWidthScaleB{0x53b624}, kWidthScaleC{0x53b628}, kWidthScaleD{0x53b62c};

/* The HUD's four scales set from the screen's width: 640 and wider scale a
 * constant each by the width, narrower keep the constants. */
reg32 scalesSet(App* app, const Entry& e)
{
    const reg32 ebp = e.frame(5);
    const int32_t width = int32_t(hudReferenceWidth(app));   // (tools/apply_hud_scale.py: was [0x7cdac8])
    if (width >= 0x280)
    {
        app->getMemory<int32_t>(ebp - 4) = width;
        const double w = double(width), base = double(kWidthScaleA(app));
        app->getMemory<float>(0x791b50) = float(w * base);
        app->getMemory<float>(0x791b3c) = float((w * double(kWidthScaleB(app))) * base);
        app->getMemory<float>(0x791b54) = float((w * double(kWidthScaleC(app))) * base);
        app->getMemory<float>(0x791b4c) = float(base * (w * double(kWidthScaleD(app))));
    }
    else
    {
        app->getMemory<reg32>(0x791b3c) = 0x3f4ccccd;
        app->getMemory<reg32>(0x791b54) = 0x3f19999a;
        app->getMemory<reg32>(0x791b4c) = 0x3f000000;
        app->getMemory<reg32>(0x791b50) = 0x3f800000;
    }
    return reg32(width);
}

/* ---- 0x4870c0, 0x486f70 ---- */

constexpr reg32 kVertexPair = 0x49bcd0;              // (surface, &vertex, &vertex): two vertices of a quad's side
constexpr reg32 kQuadStart = 0x49bd30;               // (surface, a, b, c) stdcall: d
constexpr Var<reg32> kFlipped{0x6fd4c8};

/* A quad's corners p[0] to p[3] given their colour (word by word from the
 * last to the first) and the two floats the code copies down the same way. */
void cornerStyle(App* app, const reg32 (&p)[4], reg32 colour)
{
    const auto word = [&](reg32 at) { return app->getMemory<reg32>(at); };
    const auto set = [&](reg32 at, reg32 value) { app->getMemory<reg32>(at) = value; };
    const auto copyFloat = [&](reg32 from, reg32 to) { app->getMemory<float>(to) = float(double(app->getMemory<float>(from))); };
    set(p[3] + 0x10, colour);
    set(p[2] + 0x10, word(p[3] + 0x10));
    set(p[1] + 0x10, word(p[2] + 0x10));
    set(p[0] + 0x10, word(p[1] + 0x10));
    set(p[3] + 0x18, kFlipped(app) != 0 ? 0x3ebf0000 : 0x3e020000);
    copyFloat(p[3] + 0x18, p[2] + 0x18);
    copyFloat(p[2] + 0x18, p[1] + 0x18);
    copyFloat(p[1] + 0x18, p[0] + 0x18);
    set(p[3] + 0x1c, 0x3d080000);
    copyFloat(p[3] + 0x1c, p[2] + 0x1c);
    copyFloat(p[2] + 0x1c, p[1] + 0x1c);
    copyFloat(p[1] + 0x1c, p[0] + 0x1c);
}

/* A quad from a rectangle's six words (x, y at +0, +4 for the first corner,
 * +8, +0xc for the second, +0x10, +0x14 for the third and, as a float copy,
 * the fourth) on the surface `surface`, its colour `colour`.  False when the
 * game stops in a call. */
bool cornersOfRectangle(App* app, CPU& cpu, const Entry& e, reg32& result)
{
    const reg32 rect = e.eax, colour = e.edx, surface = e.ebx;
    const reg32 ebp = e.frame(4);
    const reg32 base = e.frame(4, 0x24);
    const HeldEbp held(cpu, ebp);
    const auto L = [&](int32_t offset) { return ebp + reg32(offset); };
    app->getMemory<reg32>(L(-4)) = colour;
    const int32_t high[4] = {-0x24, -0x20, -0x1c, -0x18}, low[4] = {-0x14, -0x10, -0xc, -8};
    for (int i = 0; i < 4; ++i)
        if (!callGame(app, cpu, base, kVertexPair, {surface, L(high[i]), L(low[i]), e.ecx}))
            return false;
    if (!callGame(app, cpu, base, kQuadStart,
                  {surface, app->getMemory<reg32>(L(-0x14)), app->getMemory<reg32>(L(-0x10)), app->getMemory<reg32>(L(-0xc))},
                  {app->getMemory<reg32>(L(-8))}, result))
        return false;
    const reg32 p[4] = {app->getMemory<reg32>(L(-0x24)), app->getMemory<reg32>(L(-0x20)), app->getMemory<reg32>(L(-0x1c)),
                        app->getMemory<reg32>(L(-0x18))};
    cornerStyle(app, p, colour);
    app->getMemory<reg32>(p[0]) = app->getMemory<reg32>(rect);
    app->getMemory<reg32>(p[1]) = app->getMemory<reg32>(rect + 8);
    app->getMemory<reg32>(p[3]) = app->getMemory<reg32>(rect + 0x10);
    app->getMemory<float>(p[2]) = float(double(app->getMemory<float>(p[3])));
    app->getMemory<reg32>(p[0] + 4) = app->getMemory<reg32>(rect + 4);
    app->getMemory<reg32>(p[1] + 4) = app->getMemory<reg32>(rect + 0xc);
    app->getMemory<reg32>(p[3] + 4) = app->getMemory<reg32>(rect + 0x14);
    result = app->getMemory<reg32>(rect + 0x14);
    app->getMemory<float>(p[2] + 4) = float(double(app->getMemory<float>(p[3] + 4)));
    return true;
}

/* The same quad with its eight places on the stack (x, y of each corner in
 * turn), the colour and the surface after them; the corners' words as they
 * are. */
bool cornersFromStack(App* app, CPU& cpu, const Entry& e, reg32& result)
{
    const reg32 ebp = e.frame(5);
    const reg32 base = e.frame(5, 0x20);
    const HeldEbp held(cpu, ebp);
    const auto L = [&](int32_t offset) { return ebp + reg32(offset); };
    const reg32 colour = e.arg(app, 8), surface = e.arg(app, 9);
    const int32_t high[4] = {-0x10, -0xc, -8, -4}, low[4] = {-0x20, -0x1c, -0x18, -0x14};
    for (int i = 0; i < 4; ++i)
        if (!callGame(app, cpu, base, kVertexPair, {surface, L(high[i]), L(low[i]), e.ecx}))
            return false;
    if (!callGame(app, cpu, base, kQuadStart,
                  {surface, app->getMemory<reg32>(L(-0x20)), app->getMemory<reg32>(L(-0x1c)), app->getMemory<reg32>(L(-0x18))},
                  {app->getMemory<reg32>(L(-0x14))}, result))
        return false;
    const reg32 p[4] = {app->getMemory<reg32>(L(-0x10)), app->getMemory<reg32>(L(-0xc)), app->getMemory<reg32>(L(-8)),
                        app->getMemory<reg32>(L(-4))};
    cornerStyle(app, p, colour);
    for (int i = 0; i < 4; ++i)
        app->getMemory<reg32>(p[i]) = e.arg(app, 2 * i);
    for (int i = 0; i < 4; ++i)
        app->getMemory<reg32>(p[i] + 4) = e.arg(app, 2 * i + 1);
    result = e.arg(app, 7);
    return true;
}

/* ---- 0x485180 ---- */

constexpr reg32 kLabels = 0x725278, kLabelTargets = 0x725238, kLabelIds = 0x725258;
constexpr reg32 kLabelFormat = 0x53b644, kLabelFormatShort = 0x53b658;

/* One of the HUD's labels (group A, slot B, shown on C) for the record D: its
 * target, its id, and its text -- the language's lines with the record's two
 * floats times a constant -- written with sprintf into the label.  False when
 * the game stops in a call. */
bool labelSet(App* app, CPU& cpu, const Entry& e, reg32& result)
{
    const reg32 group = e.eax, shown = e.ebx, record = e.ecx;
    int32_t slot = int32_t(e.edx);
    const reg32 ebp = e.frame(3);
    const reg32 base = e.frame(3, 8);
    const HeldEbp held(cpu, ebp);
    result = group;
    if (slot >= 8)
        slot = 7;
    const reg32 slotWord = reg32(slot) * 4;
    if (record == 0)
    {
        app->getMemory<reg32>(slotWord + kLabelTargets) = 0;
        result = slotWord;
        return true;
    }
    app->getMemory<reg32>(slotWord + kLabelTargets) = app->getMemory<reg32>(record + 0x220) + 0x34;
    app->getMemory<reg32>(slotWord + kLabelIds) = app->getMemory<reg32>(record + 0x2c8);
    app->getMemory<reg32>(ebp - 8) = group * 320;
    const reg32 label = reg32(slot) * 40 + group * 320;
    app->getMemory<reg32>(label + kLabels) = shown;
    app->getMemory<reg32>(label + kLabels + 0x24) = record;
    const reg32 screens = kScreens(app);
    const reg32 gate = (screens == 1 ? 1 : 0) + group;
    const reg32 mode = app->getMemory<reg32>(gate * 0x348 + 0x6fbe0c);
    reg32 factor;
    bool wide;
    if (mode == 1 || mode == 2)
    {
        factor = 0x400f295f;
        wide = true;
    }
    else
    {
        factor = 0x40666666;
        wide = false;
    }
    app->getMemory<reg32>(ebp - 4) = factor;
    const reg32 dest = kLabels + group * 320 + reg32(slot) * 40 + 4;
    const double scale = double(bitsToFloat(factor));
    reg32 line;
    if (app->getMemory<int32_t>(record + 0x6f0) > 0x40000000)
    {
        reg32 first, second, third;
        if (!callGame(app, cpu, base, kLanguageText, {0x15e, reg32(slot), 0, record}, {}, first))
            return false;
        if (!callGame(app, cpu, base, kLanguageText, {wide ? 0x156u : 0x157u, reg32(slot), 0, record}, {}, second))
            return false;
        const int32_t one = chopped(double(app->getMemory<float>(record + 0x6f0)) * scale);
        app->getMemory<int32_t>(ebp - 8) = one;
        if (!callGame(app, cpu, base, kLanguageText, {wide ? 0x156u : 0x157u, reg32(slot), 0, record}, {}, third))
            return false;
        const int32_t two = chopped(double(app->getMemory<float>(record + 0x7c4)) * scale);
        app->getMemory<int32_t>(ebp - 8) = two;
        // (tools/apply_hud_scale.py: the compact table's short format, the arguments left on the stack.)
        return callGame(app, cpu, base, kSprintf, {dest, reg32(slot), 0, reg32(two)},
                        {dest, kLabelFormatShort, reg32(two), third, reg32(one), second, first}, result);
    }
    if (!callGame(app, cpu, base, kLanguageText, {wide ? 0x156u : 0x157u, reg32(slot), 0, record}, {}, line))
        return false;
    const int32_t value = chopped(double(app->getMemory<float>(record + 0x7c4)) * scale);
    app->getMemory<int32_t>(ebp - 8) = value;
    return callGame(app, cpu, base, kSprintf, {dest, reg32(slot), 0, reg32(value)},
                    {dest, kLabelFormatShort, reg32(value), line}, result);
}

/* ---- 0x485c30, 0x485f20, 0x4860b0, 0x4873e0: lines clipped to rectangles ---- */

constexpr reg32 kClipFound = 0x791c08;               // the point a clip finds: x, then y (two floats)

/* Whether `a` is below `b` for the x87's compare and a jb: also when unordered. */
bool jumpBelow(double a, double b)
{
    return !(a >= b);
}

/* The point where the line of the segment (its four floats at `segment`: x0,
 * y0, x1, y1) enters the rectangle (left, top, right, bottom at `rect`) from
 * the point `point` outside it, as the sides of the rectangle it starts past
 * are tried one after the other (left, right, then below and above when the
 * line is vertical or runs level).  Found, the point is at 0x791c08, the side
 * it is on set in the flags byte at `flagsAt`; not found, the result is 0 --
 * but for `keep` clear, when it is the point anyway.  The line is y = m x + c
 * through the segment's end. */
reg32 clipPointOf(App* app, reg32 point, reg32 segment, reg32 rect, reg32 keep, reg32 flagsAt)
{
    const auto f = [&](reg32 at) { return double(app->getMemory<float>(at)); };
    const auto setFloat = [&](reg32 at, double value) { app->getMemory<float>(at) = float(value); };
    const auto addFlag = [&](uint8_t bit) { app->getMemory<uint8_t>(flagsAt) = uint8_t(app->getMemory<uint8_t>(flagsAt) | bit); };
    constexpr reg32 g0 = kClipFound, g1 = kClipFound + 4;
    const float d8 = float(f(point) - f(rect + 4));
    const float d18 = float(f(rect + 0xc) - f(point));
    const float d14 = float(f(segment) - f(segment + 8));
    const float d1c = float(f(point + 4) - f(rect));
    float m = 0.0f;
    reg32 d14Bits;
    std::memcpy(&d14Bits, &d14, sizeof d14Bits);
    if ((d14Bits & 0x7fffffff) != 0)
        m = float((f(segment + 4) - f(segment + 0xc)) / double(d14));
    const float c = float(f(segment + 0xc) - double(m) * f(segment + 8));
    reg32 mBits;
    std::memcpy(&mBits, &m, sizeof mBits);
    const bool level = (mBits & 0x7fffffff) == 0;
    bool found = true;
    const double cc = double(c), mm = double(m);
    if (0.0 > double(d8))
    {
        const double t = mm * f(rect + 4) + cc;
        app->getMemory<float>(g1) = float(t);
        if (!(t >= f(rect)))
        {
            const double u = (f(rect) - cc) / mm;
            app->getMemory<float>(g0) = float(u);
            if (jumpBelow(u, f(rect + 4)))
                found = false;
            else if (double(app->getMemory<float>(g0)) > f(rect + 0xc))
                found = false;
            else
            {
                const uint8_t flags = app->getMemory<uint8_t>(flagsAt);
                app->getMemory<float>(g1) = float(f(rect));
                app->getMemory<uint8_t>(flagsAt) = uint8_t(flags | 1);
            }
        }
        else if (!(double(app->getMemory<float>(g1)) > f(rect + 8)))
        {
            const uint8_t flags = app->getMemory<uint8_t>(flagsAt);
            app->getMemory<float>(g0) = float(f(rect + 4));
            app->getMemory<uint8_t>(flagsAt) = uint8_t(flags | 4);
        }
        else
        {
            const double u = (f(rect + 8) - cc) / mm;
            app->getMemory<float>(g0) = float(u);
            if (jumpBelow(u, f(rect + 4)))
                found = false;
            else if (double(app->getMemory<float>(g0)) > f(rect + 0xc))
                found = false;
            else
            {
                const uint8_t flags = app->getMemory<uint8_t>(flagsAt);
                app->getMemory<float>(g1) = float(f(rect + 8));
                app->getMemory<uint8_t>(flagsAt) = uint8_t(flags | 2);
            }
        }
    }
    else if (0.0 > double(d18))
    {
        const double t = mm * f(rect + 0xc) + cc;
        app->getMemory<float>(g1) = float(t);
        if (!(t >= f(rect)))
        {
            const double u = (f(rect) - cc) / mm;
            app->getMemory<float>(g0) = float(u);
            if (jumpBelow(u, f(rect + 4)))
                found = false;
            else if (double(app->getMemory<float>(g0)) > f(rect + 0xc))
                found = false;
            else
            {
                const uint8_t flags = app->getMemory<uint8_t>(flagsAt);
                app->getMemory<float>(g1) = float(f(rect));
                app->getMemory<uint8_t>(flagsAt) = uint8_t(flags | 1);
            }
        }
        else if (!(double(app->getMemory<float>(g1)) > f(rect + 8)))
        {
            const uint8_t flags = app->getMemory<uint8_t>(flagsAt);
            app->getMemory<float>(g0) = float(f(rect + 0xc));
            app->getMemory<uint8_t>(flagsAt) = uint8_t(flags | 8);
        }
        else
        {
            const double u = (f(rect + 8) - cc) / mm;
            app->getMemory<float>(g0) = float(u);
            if (jumpBelow(u, f(rect + 4)))
                found = false;
            else if (double(app->getMemory<float>(g0)) > f(rect + 0xc))
                found = false;
            else
            {
                const uint8_t flags = app->getMemory<uint8_t>(flagsAt);
                app->getMemory<float>(g1) = float(f(rect + 8));
                app->getMemory<uint8_t>(flagsAt) = uint8_t(flags | 2);
            }
        }
    }
    else if (0.0 > double(d1c))
    {
        if (level)
        {
            const float start = float(f(segment)), left = float(f(rect));
            const uint8_t flags = app->getMemory<uint8_t>(flagsAt);
            app->getMemory<float>(g1) = left;
            app->getMemory<float>(g0) = start;
            app->getMemory<uint8_t>(flagsAt) = uint8_t(flags | 1);
        }
        else
        {
            const double u = (f(rect) - cc) / mm;
            app->getMemory<reg32>(g1) = app->getMemory<reg32>(rect);
            app->getMemory<float>(g0) = float(u);
            found = !(jumpBelow(u, f(rect + 4)) || double(app->getMemory<float>(g0)) > f(rect + 0xc));
            if (found)
                addFlag(1);
        }
    }
    else
    {
        if (level)
        {
            const float start = float(f(segment)), right = float(f(rect + 8));
            const uint8_t flags = app->getMemory<uint8_t>(flagsAt);
            app->getMemory<float>(g1) = right;
            app->getMemory<float>(g0) = start;
            app->getMemory<uint8_t>(flagsAt) = uint8_t(flags | 2);
        }
        else
        {
            const double u = (f(rect + 8) - cc) / mm;
            app->getMemory<reg32>(g1) = app->getMemory<reg32>(rect + 8);
            app->getMemory<float>(g0) = float(u);
            found = !(jumpBelow(u, f(rect + 4)) || double(app->getMemory<float>(g0)) > f(rect + 0xc));
            if (found)
                addFlag(2);
        }
    }
    if (found || keep == 0)
        return g0;
    return 0;
}

/* ---- 0x485f20 ---- */

/* The segment (x0, y0, x1, y1 at `segment`) clipped to the rectangle: its
 * two ends into the four floats at `out` (the clipped one the clip finds when
 * an end is outside), their sides in the flags at `flagsAt`; 1 for a segment
 * wholly outside, else 0.  The code keeps the two points it clips on its
 * stack, at `frame`. */
reg32 segmentClipped(App* app, reg32 frame, reg32 segment, reg32 rect, reg32 out, reg32 flagsAt)
{
    const auto f = [&](reg32 at) { return double(app->getMemory<float>(at)); };
    const auto s = [&](int i) { return f(segment + 4 * reg32(i)); };
    const auto r = [&](int i) { return f(rect + 4 * reg32(i)); };
    const auto below = [](double a, double b) { return !(a >= b); };
    app->getMemory<reg32>(flagsAt) = 0;
    reg32 failed = 0;
    // Both ends past one side of the rectangle: nothing of it is in.
    if (!(s(0) >= r(1)) && below(s(2), r(1)))
        return app->getMemory<reg32>(frame - 8) = 1;
    if (s(0) > r(3) && s(2) > r(3))
        return app->getMemory<reg32>(frame - 8) = 1;
    if (!(s(1) >= r(0)) && below(s(3), r(0)))
        return app->getMemory<reg32>(frame - 8) = 1;
    if (s(1) > r(2) && s(3) > r(2))
        return app->getMemory<reg32>(frame - 8) = 1;
    const auto inside = [&](int x, int y) {
        return !below(s(x), r(1)) && !(s(x) > r(3)) && !below(s(y), r(0)) && !(s(y) > r(2));
    };
    app->getMemory<reg32>(frame - 8) = 0;
    if (inside(0, 1))
    {
        app->getMemory<reg32>(out) = app->getMemory<reg32>(segment);
        app->getMemory<reg32>(out + 4) = app->getMemory<reg32>(segment + 4);
    }
    else
    {
        app->getMemory<reg32>(frame - 0x1c) = app->getMemory<reg32>(segment);
        app->getMemory<reg32>(frame - 0x18) = app->getMemory<reg32>(segment + 4);
        const reg32 found = clipPointOf(app, frame - 0x1c, segment, rect, 1, flagsAt);
        if (found != 0)
        {
            app->getMemory<reg32>(out) = app->getMemory<reg32>(found);
            app->getMemory<reg32>(out + 4) = app->getMemory<reg32>(found + 4);
        }
        else
            failed = 1;
        if (failed)
            app->getMemory<reg32>(frame - 8) = 1;
    }
    if (app->getMemory<reg32>(frame - 8) != 0)
        return app->getMemory<reg32>(frame - 8);
    if (inside(2, 3))
    {
        app->getMemory<reg32>(out + 8) = app->getMemory<reg32>(segment + 8);
        app->getMemory<reg32>(out + 12) = app->getMemory<reg32>(segment + 12);
        return app->getMemory<reg32>(frame - 8);
    }
    app->getMemory<reg32>(frame - 0x14) = app->getMemory<reg32>(segment + 8);
    app->getMemory<reg32>(frame - 0x10) = app->getMemory<reg32>(segment + 12);
    const reg32 found = clipPointOf(app, frame - 0x14, segment, rect, 1, flagsAt);
    if (found == 0)
        return app->getMemory<reg32>(frame - 8) = 1;
    app->getMemory<reg32>(out + 8) = app->getMemory<reg32>(found);
    app->getMemory<reg32>(out + 12) = app->getMemory<reg32>(found + 4);
    return app->getMemory<reg32>(frame - 8);
}

/* ---- 0x4860b0 ---- */

/* The segment clipped to the rectangle into up to two pieces: how many pieces
 * (0, 1 or 2) to `countAt`, their ends (four floats each) to `out`; 0 for a
 * segment that is wholly outside, 1 otherwise.  An end that is inside stays
 * as it is; an outside one is the clip's point.  The code keeps its working
 * on its stack, at `frame`. */
reg32 segmentPieces(App* app, reg32 frame, reg32 segment, reg32 rect, reg32 countAt, reg32 out)
{
    const auto f = [&](reg32 at) { return double(app->getMemory<float>(at)); };
    const auto s = [&](int i) { return f(segment + 4 * reg32(i)); };
    const auto r = [&](int i) { return f(rect + 4 * reg32(i)); };
    const auto below = [](double a, double b) { return !(a >= b); };
    const auto word = [&](reg32 at) -> reg32 { return app->getMemory<reg32>(at); };
    const auto set = [&](reg32 at, reg32 value) { app->getMemory<reg32>(at) = value; };
    set(frame - 0x10, countAt);
    set(frame - 0xc, out);
    set(frame - 8, 1);
    if ((below(s(0), r(1)) && below(s(2), r(1))) || (s(0) > r(3) && s(2) > r(3)) ||
        (below(s(1), r(0)) && below(s(3), r(0))) || (s(1) > r(2) && s(3) > r(2)))
    {
        set(frame - 8, 0);
        return word(frame - 8);
    }
    const auto inside = [&](int x, int y) {
        return !below(s(x), r(1)) && !(s(x) > r(3)) && !below(s(y), r(0)) && !(s(y) > r(2));
    };
    const auto clipFrom = [&](reg32 x, reg32 y, reg32 keep) {
        set(frame - 0x18, x);
        set(frame - 0x14, y);
        return clipPointOf(app, frame - 0x18, segment, rect, keep, frame - 4);
    };
    const bool first = inside(0, 1), second = inside(2, 3);
    if (first && second)
    {
        set(countAt, 0);
        return word(frame - 8);
    }
    if (first)
    {
        set(countAt, 1);
        set(out + 8, word(segment + 8));
        set(frame - 0x18, word(out + 8));
        set(out + 0xc, word(segment + 0xc));
        app->getMemory<float>(frame - 0x14) = float(double(app->getMemory<float>(out + 0xc)));
        const reg32 found = clipPointOf(app, frame - 0x18, segment, rect, 0, frame - 4);
        set(out, word(found));
        set(out + 4, word(found + 4));
        return word(frame - 8);
    }
    if (second)
    {
        set(countAt, 1);
        set(out, word(segment));
        set(frame - 0x18, word(out));
        set(out + 4, word(segment + 4));
        app->getMemory<float>(frame - 0x14) = float(double(app->getMemory<float>(out + 4)));
        const reg32 found = clipPointOf(app, frame - 0x18, segment, rect, 0, frame - 4);
        set(out + 8, word(found));
        set(out + 0xc, word(found + 4));
        return word(frame - 8);
    }
    const reg32 found = clipFrom(word(segment), word(segment + 4), 1);
    if (found == 0)
    {
        set(frame - 8, found);
        return word(frame - 8);
    }
    set(countAt, 2);
    set(out, word(segment));
    set(out + 4, word(segment + 4));
    set(out + 8, word(found));
    set(out + 0xc, word(found + 4));
    const reg32 other = clipFrom(word(segment + 8), word(segment + 12), 0);
    set(out + 0x10, word(segment + 8));
    set(out + 0x14, word(segment + 12));
    set(out + 0x18, word(other));
    set(out + 0x1c, word(other + 4));
    return word(frame - 8);
}

/* ---- 0x4873e0 ---- */

constexpr Table<reg32> kGroupRects{0x791b60};        // by group: the rectangle that clips its strokes
constexpr reg32 kGroupPieces = 0x791b68;             // by group (80 bytes) and piece (16 bytes): the rectangles
constexpr reg32 kTakeVertices = 0x49bf40;            // (surface, count, &first): vertices on a surface

/* A stroke (a segment) of group `group` drawn in colour `colour` on `surface`:
 * mode 0 as it is, mode 1 clipped to the group's rectangle, mode 2 clipped and
 * then cut by each of the group's rectangles in turn into the pieces that are
 * left (a piece wholly cut away goes, one cut in two makes two) -- each piece
 * two vertices, 32 bytes each.  The working is on the code's stack, which it
 * holds 0x7e below its frame.  False when the game stops in a call. */
bool strokeDrawn(App* app, CPU& cpu, const Entry& e)
{
    const reg32 group = e.eax, mode = e.edx, segment = e.ebx, colour = e.ecx;
    const reg32 surface = e.arg(app, 0);
    const reg32 ebp0 = e.frame(3);
    const reg32 base = e.frame(3, 0xf0);
    const reg32 frame = ebp0 - 0x7e;
    const HeldEbp held(cpu, frame);
    const auto L = [&](int32_t offset) { return frame + reg32(offset); };
    const auto word = [&](reg32 at) -> reg32 { return app->getMemory<reg32>(at); };
    const auto set = [&](reg32 at, reg32 value) { app->getMemory<reg32>(at) = value; };
    const auto copyFour = [&](reg32 to, reg32 from) {
        for (reg32 i = 0; i < 4; ++i)
            set(to + 4 * i, word(from + 4 * i));
    };
    set(L(0x6a), group);
    set(L(0x6e), colour);
    if (mode == 0)
        copyFour(L(0x4e), segment);
    else if (segmentClipped(app, base - 16, segment, kGroupRects(app, int32_t(group)), L(0x4e), L(0x62)) != 0)
        return true;
    if (mode != 2)
    {
        reg32 first;
        if (!callGame(app, cpu, base, kTakeVertices, {surface, 1, L(0x5e), 0}, {}, first))
            return false;
        const reg32 vertex = word(L(0x5e));
        set(vertex, word(L(0x4e)));
        set(vertex + 4, word(L(0x52)));
        set(vertex + 0x20, word(L(0x56)));
        set(vertex + 0x24, word(L(0x5a)));
        set(vertex + 0x10, colour);
        set(L(0x5e), vertex + 0x20);
        set(vertex + 0x20 + 0x10, colour);
        return true;
    }
    const reg32 array = L(-0x72);
    set(L(0x7a), 1);
    set(L(0x72), 0);
    copyFour(array, L(0x4e));
    const auto pieces = [&]() { return int32_t(word(L(0x7a))); };
    while (int32_t(word(L(0x72))) < pieces())
    {
        set(L(0x76), 0);
        const reg32 rectCount = word(0x55d694 + group * 4);
        while (int32_t(word(L(0x76))) < int32_t(rectCount))
        {
            const reg32 rect = kGroupPieces + 80 * group + (word(L(0x76)) << 4);
            const reg32 piece = array + (word(L(0x72)) << 4);
            if (segmentPieces(app, base - 16, piece, rect, L(0x66), L(0x2e)) != 0)
            {
                const reg32 count = word(L(0x66));
                if (count == 0)
                {
                    const reg32 last = word(L(0x7a)) - 1;
                    set(L(0x7a), last);
                    if (int32_t(word(L(0x72))) < int32_t(last))
                    {
                        copyFour(piece, array + (last << 4));
                        set(L(0x76), 0xffffffff);
                    }
                    else
                        set(L(0x76), rectCount);
                }
                else if (count == 1)
                    copyFour(piece, L(0x2e));
                else if (count == 2)
                {
                    copyFour(piece, L(0x2e));
                    copyFour(array + (word(L(0x7a)) << 4), L(0x3e));
                    set(L(0x7a), word(L(0x7a)) + 1);
                }
            }
            set(L(0x76), word(L(0x76)) + 1);
        }
        set(L(0x72), word(L(0x72)) + 1);
    }
    if (pieces() == 0)
        return true;
    reg32 first;
    if (!callGame(app, cpu, base, kTakeVertices, {surface, word(L(0x7a)), L(0x5e), word(L(0x7a))}, {}, first))
        return false;
    for (reg32 k = 0; int32_t(k) < pieces(); ++k)
    {
        const reg32 piece = array + (k << 4);
        const reg32 vertex = word(L(0x5e));
        set(vertex, word(piece));
        set(vertex + 4, word(piece + 4));
        set(vertex + 0x20, word(piece + 8));
        app->getMemory<float>(vertex + 0x24) = float(double(app->getMemory<float>(piece + 12)));
        set(vertex + 0x10, word(L(0x6e)));
        set(L(0x5e), vertex + 0x20);
        set(vertex + 0x30, word(L(0x6e)));
        set(L(0x5e), word(L(0x5e)) + 0x20);
    }
    return true;
}

/* ---- 0x485a60 ---- */

constexpr Var<int32_t> kStandingsWidth{0x7d3684};
constexpr Var<float> kStandingsFactorA{0x53b67c};
constexpr Var<double> kStandingsFactorB{0x53b680}, kStandingsLimit{0x53b688};

/* How far apart two racers' rows in the standings are, for the order they go
 * in: when both are shown in the big form (flag and kind 3) the difference of
 * their places, else a distance of the rows' heights divided by the larger of
 * the two ratios of a row to the box, chopped to an integer -- 0 when there is
 * no ratio to speak of. */
int32_t standingsGap(App* app, reg32 first, reg32 second)
{
    const auto word = [&](reg32 at) -> reg32 { return app->getMemory<reg32>(at); };
    if (kStandingsWidth(app) <= 0x200)
        return 0;
    const int32_t high = int32_t(word(second + 8)), low = int32_t(word(first + 8));
    if (high == 0 || low == 0)
        return 0;
    const int32_t integer = chopped(double(app->getMemory<float>(second + 0x7c0)));
    const float size = float((double(integer) * double(kStandingsFactorA(app))) * kStandingsFactorB(app));
    const int32_t gap = low - high;
    const bool bigSecond = (app->getMemory<uint8_t>(second + 0x200) & 1) != 0 && word(second + 0x2c4) == 3;
    const bool bigFirst = (app->getMemory<uint8_t>(first + 0x200) & 1) != 0 && word(first + 0x2c4) == 3;
    if (bigSecond && bigFirst)
        return int32_t(word(first + 0x228) - word(second + 0x228));
    const double width = double(int32_t(kStandingsWidth(app) - 0x200));
    const float ratioSecond = float(bigSecond ? double(int32_t(word(second + 0x224))) / double(int32_t(word(second + 0x228) - 0x200))
                                              : double(high) / width);
    const float ratioFirst = float(bigFirst ? double(int32_t(word(first + 0x224))) / double(int32_t(word(first + 0x228) - 0x200))
                                             : double(low) / width);
    if (jumpBelow(1.0, double(size)))
    {
        if (double(size) >= double(ratioSecond))
            return chopped(double(gap) / double(size));
        return chopped(double(gap) / double(ratioSecond));
    }
    if (double(ratioFirst) > kStandingsLimit(app))
        return chopped(double(gap) / double(ratioFirst));
    if (double(ratioSecond) > kStandingsLimit(app))
        return chopped(double(gap) / double(ratioSecond));
    return 0;
}

/* ---- 0x486df0 ---- */

/* A quad of four corners given as eight floats (x, y of the box's sides in
 * the order the code has them) on the surface `surface` with a colour and two
 * more floats for each corner, each pair written to the corner that has it and
 * copied through the x87 to its neighbour.  Nothing is drawn for a box less
 * than a pixel each way (the sides chopped to integers); the result is then
 * the last side less one.  False when the game stops in a call. */
bool quadOfBox(App* app, CPU& cpu, const Entry& e, reg32& result)
{
    const reg32 ebp = e.frame(5);
    const reg32 base = e.frame(5, 0x28);
    const HeldEbp held(cpu, ebp);
    const auto L = [&](int32_t offset) { return ebp + reg32(offset); };
    const auto word = [&](reg32 at) -> reg32 { return app->getMemory<reg32>(at); };
    const auto set = [&](reg32 at, reg32 value) { app->getMemory<reg32>(at) = value; };
    const auto copyFloat = [&](reg32 from, reg32 to) { app->getMemory<float>(to) = float(double(app->getMemory<float>(from))); };
    const auto arg = [&](int i) { return e.arg(app, i); };
    const reg32 surface = arg(9);
    const int32_t n3 = chopped(double(bitsToFloat(arg(3)))), n1 = chopped(double(bitsToFloat(arg(1))));
    set(L(-4), reg32(n3));
    set(L(-8), reg32(n1));
    result = reg32(n3 - 1);
    if (n3 - 1 <= n1)
        return true;
    const int32_t n2 = chopped(double(bitsToFloat(arg(2)))), n0 = chopped(double(bitsToFloat(arg(0))));
    set(L(-8), reg32(n2));
    set(L(-4), reg32(n0));
    result = reg32(n2 - 1);
    if (n2 - 1 <= n0)
        return true;
    const int32_t high[4] = {-0x28, -0x24, -0x20, -0x1c}, low[4] = {-0x18, -0x14, -0x10, -0xc};
    for (int i = 0; i < 4; ++i)
        if (!callGame(app, cpu, base, kVertexPair, {surface, L(high[i]), L(low[i]), reg32(n0)}))
            return false;
    if (!callGame(app, cpu, base, kQuadStart, {surface, word(L(-0x18)), word(L(-0x14)), word(L(-0x10))}, {word(L(-0xc))}, result))
        return false;
    const reg32 p[4] = {word(L(-0x28)), word(L(-0x24)), word(L(-0x20)), word(L(-0x1c))};
    set(p[3] + 0x10, arg(8));
    set(p[2] + 0x10, word(p[3] + 0x10));
    set(p[1] + 0x10, word(p[2] + 0x10));
    set(p[0] + 0x10, word(p[1] + 0x10));
    set(p[3], arg(1));
    copyFloat(p[3], p[0]);
    set(p[2], arg(3));
    copyFloat(p[2], p[1]);
    set(p[1] + 4, arg(0));
    copyFloat(p[1] + 4, p[0] + 4);
    set(p[3] + 4, arg(2));
    copyFloat(p[3] + 4, p[2] + 4);
    set(p[3] + 0x18, arg(5));
    copyFloat(p[3] + 0x18, p[0] + 0x18);
    set(p[2] + 0x18, arg(7));
    copyFloat(p[2] + 0x18, p[1] + 0x18);
    set(p[1] + 0x1c, arg(4));
    copyFloat(p[1] + 0x1c, p[0] + 0x1c);
    set(p[3] + 0x1c, arg(6));
    copyFloat(p[3] + 0x1c, p[2] + 0x1c);
    result = p[2];
    return true;
}

/* ---- 0x484070 ---- */

constexpr reg32 kHudCountersReset = 0x47f590, kBlockSet = 0x4e01c0;
constexpr reg32 kSlotAdded = 0x4771c0, kSlotFieldSet = 0x477470, kSlotBaseSet = 0x477070, kPictureTaken = 0x49bc80;
constexpr reg32 kHudGeometryPrepared = 0x48e780, kHudBatchesInitialised = 0x47bc40, kTrackMapInitialised = 0x48cdb0;
constexpr reg32 kHudTablesStarted = 0x484c10, kHudColumnsStarted = 0x48aa40, kHudScrolled = 0x480a00;

/* The race HUD set up as a race starts: its counters cleared, its colour slots
 * and pictures put, its batches, geometry, map, tables and columns started
 * (the columns given the screen's size as two floats), each pane drawn and
 * scrolled once.  Between the calls the code leaves the registers as the
 * callee left them, but where it sets one.  False when the game stops in a
 * call. */
bool startHud(App* app, CPU& cpu, const Entry& e, reg32& result)
{
    const reg32 ebp = e.frame(6);
    const HeldEbp held(cpu, ebp);
    const auto set = [&](reg32 at, reg32 value) { app->getMemory<reg32>(at) = value; };
    const auto word = [&](reg32 at) -> reg32 { return app->getMemory<reg32>(at); };
    Args r{e.eax, 0, e.ebx, 0xffffffff};
    const auto call = [&](reg32 target, std::initializer_list<reg32> stack = {}) {
        if (!callGame(app, cpu, ebp, target, r, stack, r.eax))
            return false;
        r.edx = cpu.edx;
        r.ebx = cpu.ebx;
        r.ecx = cpu.ecx;
        return true;
    };
    for (const reg32 at : {0x700234u, 0x70023cu, 0x700230u, 0x700238u, 0x791b1cu, 0x725228u, 0x725210u})
        set(at, 0);
    if (!call(kTextPrepared) || !call(kHudCountersReset) || !call(kTextPrepared))
        return false;
    r.ebx = r.ecx;
    r.edx = 0x55d448;
    set(0x55d338, r.ecx);
    set(0x55d33c, r.ecx);
    r.eax = r.edx;
    set(0x55b088, 4);
    if (!call(kBlockSet))
        return false;
    r.ecx = 0;
    r.edx = 0x55d448;
    r.eax = 0xff99aaaa;
    if (!call(kSlotAdded))
        return false;
    r.eax = word(0x55d2f0 + 4 * word(0x55d2d8)) != 0 ? 0x67901c : 0;
    if (!call(kSlotFieldSet))
        return false;
    r.eax = 3;
    r.edx = 0x55d448;
    r.ebx = 0xff808080;
    r.ecx = 0;
    set(0x55b088, r.eax);
    set(0x791b44, r.edx);
    r.eax = 0;
    r.edx = 0xffa0a0a0;
    if (!call(kSlotFieldSet))
        return false;
    r.eax = r.edx;
    set(0x55b088, r.ecx);
    if (!call(kSlotBaseSet))
        return false;
    r.eax = 0;
    if (!call(kSlotFieldSet))
        return false;
    r.edx = 0;
    r.eax = 0x725748;
    if (!call(kPictureTaken))
        return false;
    r.eax = 0x749a38;
    r.edx = word(0x791aec);
    if (!call(kPictureTaken))
        return false;
    r.eax = 0x700248;
    r.edx = word(0x791aec);
    if (!call(kPictureTaken))
        return false;
    r.eax = 0x76da48;
    r.edx = word(0x791a98);
    if (!call(kPictureTaken))
        return false;
    for (const reg32 target : {kHudGeometryPrepared, kHudBatchesInitialised, kTrackMapInitialised, kHudTablesStarted})
        if (!call(target) || !call(kTextPrepared))
            return false;
    // Two floats pushed, the screen's height first.
    const float width = float(double(int32_t(word(0x7cdac8)))), height = float(double(int32_t(word(0x7cdacc))));
    if (!call(kHudColumnsStarted, {bitsOfFloat(width), bitsOfFloat(height)}))
        return false;
    r.eax = 0;
    if (!call(kPaneDrawn) || !call(kTextPrepared))
        return false;
    r.eax = 1;
    if (!call(kPaneDrawn) || !call(kTextPrepared))
        return false;
    r.eax = 0;
    if (!call(kHudScrolled) || !call(kTextPrepared))
        return false;
    r.eax = 1;
    r.ebx = 1;
    if (!call(kHudScrolled) || !call(kTextPrepared))
        return false;
    set(0x55d440, r.ebx);
    result = r.eax;
    return true;
}

/* The registers as the game's code has them between its calls: it sets the
 * ones its instructions set (`r`), and after a call has what the callee left. */
struct Walk
{
    App* app;
    CPU& cpu;
    reg32 esp;
    Args r;
    template <class Target>
    bool call(Target target, std::initializer_list<reg32> stack = {})
    {
        if (!callGame(app, cpu, esp, target, r, stack, r.eax))
            return false;
        r.edx = cpu.edx;
        r.ebx = cpu.ebx;
        r.ecx = cpu.ecx;
        return true;
    }
};

/* ---- 0x48a3a0 ---- */

constexpr reg32 kTextWidthPx = 0x4d1430;             // (text): its width in whole pixels, on the x87's stack
constexpr reg32 kOrdinalSuffix = 0x4896f0;           // (number): "st", "nd", "rd" or "th"
constexpr reg32 kRankFormat = 0x53b820, kRankOfFormat = 0x53b8b0;   // "%d", " of %d"
constexpr Var<float> kRankPadRight{0x53b8b8}, kRankPadLeft{0x53b8bc};

/* A place "3rd of 8" drawn: the number (the scale `scaleMain`) and its ordinal
 * suffix (the scale `scaleSuffix`) and " of 8", one after the other from x, or
 * (mode 2) right to left to end at x2; kept inside the screen on the left to
 * right way -- moved left to end at its edge -- and from the other side kept
 * past its own length.  The text's widths are the x87 values the width
 * functions leave.  False when the game stops in a call. */
bool rankDrawn(App* app, CPU& cpu, const Entry& e, reg32& result)
{
    const reg32 ebp = e.frame(5);
    const reg32 base = e.frame(5, 0x10c);
    const HeldEbp held(cpu, ebp);
    const reg32 buffer = ebp - 0x10c;
    const reg32 number = e.eax, count = e.edx;
    const reg32 scaleSuffix = e.arg(app, 0), scaleMain = e.arg(app, 1), colour = e.arg(app, 2);
    const reg32 y = e.arg(app, 5), mode = e.arg(app, 6);
    const auto slot = [&](int i) { return app->getMemory<int32_t>(e.esp + 4 + 4 * reg32(i)); };
    Walk w{app, cpu, base, {colour, e.edx, e.ebx, number}};
    kTextColour.set(app, colour);
    if (!w.call(kTextScale, {scaleMain}))
        return false;
    cpu.fpu.pop();
    w.r.eax = buffer;
    if (!w.call(kSprintf, {buffer, kRankFormat, w.r.ecx}))
        return false;
    w.r.eax = buffer;
    if (!w.call(kTextWidthPx))
        return false;
    const float widthNumber = float(double(cpu.fpu.st(0)));
    cpu.fpu.pop();
    if (!w.call(kTextScale, {scaleSuffix}))
        return false;
    cpu.fpu.pop();
    w.r.eax = w.r.ecx;
    if (!w.call(kOrdinalSuffix) || !w.call(kTextWidthPx))
        return false;
    const float widths = float(double(cpu.fpu.st(0)) + double(widthNumber));
    cpu.fpu.pop();
    if (!w.call(kTextScale, {scaleMain}))
        return false;
    cpu.fpu.pop();
    w.r.eax = buffer;
    if (!w.call(kSprintf, {buffer, kRankOfFormat, w.r.edx}))
        return false;
    w.r.eax = buffer;
    if (!w.call(kTextWidth))
        return false;
    const float length = float(double(cpu.fpu.st(0)) + double(widths));
    cpu.fpu.pop();
    w.r.edx = mode;
    if (mode != 2)
    {
        int32_t x = slot(3);
        w.r.eax = reg32(x);
        const float limit = float(double(int32_t(kScreenRight(app))));
        if (!jumpBelow(double(x) + double(length), double(limit)))
            slot(3) = x = chopped((double(limit) - double(length)) + double(kRankPadRight(app)));
        if (!w.call(kTextScale, {scaleMain}))
            return false;
        cpu.fpu.pop();
        w.r.edx = reg32(x);
        w.r.eax = buffer;
        if (!w.call(kSprintf, {buffer, kRankFormat, w.r.ecx}))
            return false;
        w.r.ebx = y;
        w.r.eax = buffer;
        if (!w.call(kTextDrawn))
            return false;
        w.r.eax = reg32(x);
        w.r.ebx = y;
        slot(3) = x = chopped(double(cpu.fpu.st(0)) + double(x));
        cpu.fpu.pop();
        if (!w.call(kTextScale, {scaleSuffix}))
            return false;
        cpu.fpu.pop();
        w.r.eax = w.r.ecx;
        w.r.edx = reg32(x);
        if (!w.call(kOrdinalSuffix) || !w.call(kTextDrawn))
            return false;
        w.r.eax = reg32(x);
        slot(3) = x = chopped(double(cpu.fpu.st(0)) + double(x));
        cpu.fpu.pop();
        if (!w.call(kTextScale, {scaleMain}))
            return false;
        cpu.fpu.pop();
        w.r.ebx = y;
        w.r.eax = buffer;
        if (!w.call(kSprintf, {buffer, kRankOfFormat, count}))
            return false;
        w.r.edx = reg32(x);
        w.r.eax = buffer;
        if (!w.call(kTextDrawn))
            return false;
        cpu.fpu.pop();
        result = w.r.eax;
        return true;
    }
    int32_t x = slot(4);
    w.r.eax = reg32(x);
    if (jumpBelow(double(x), double(length)))
        slot(4) = x = chopped(double(length) + double(kRankPadLeft(app)));
    if (!w.call(kTextScale, {scaleMain}))
        return false;
    cpu.fpu.pop();
    w.r.eax = buffer;
    if (!w.call(kSprintf, {buffer, kRankOfFormat, count}))
        return false;
    w.r.eax = buffer;
    if (!w.call(kTextWidth))
        return false;
    w.r.ebx = y;
    w.r.eax = buffer;
    slot(4) = x = chopped(double(x) - double(cpu.fpu.st(0)));
    cpu.fpu.pop();
    w.r.edx = reg32(x);
    if (!w.call(kTextDrawn))
        return false;
    cpu.fpu.pop();
    if (!w.call(kTextScale, {scaleSuffix}))
        return false;
    cpu.fpu.pop();
    w.r.eax = w.r.ecx;
    if (!w.call(kOrdinalSuffix) || !w.call(kTextWidth))
        return false;
    w.r.ebx = y;
    w.r.eax = w.r.ecx;
    slot(4) = x = chopped(double(x) - double(cpu.fpu.st(0)));
    cpu.fpu.pop();
    w.r.edx = reg32(x);
    if (!w.call(kOrdinalSuffix) || !w.call(kTextDrawn))
        return false;
    cpu.fpu.pop();
    if (!w.call(kTextScale, {scaleMain}))
        return false;
    cpu.fpu.pop();
    w.r.eax = buffer;
    if (!w.call(kSprintf, {buffer, kRankFormat, w.r.ecx}))
        return false;
    w.r.eax = buffer;
    if (!w.call(kTextWidth))
        return false;
    w.r.ebx = y;
    w.r.eax = buffer;
    slot(4) = x = chopped(double(x) - double(cpu.fpu.st(0)));
    cpu.fpu.pop();
    w.r.edx = reg32(x);
    if (!w.call(kTextDrawn))
        return false;
    cpu.fpu.pop();
    result = w.r.eax;
    return true;
}

/* ---- 0x48a660 ---- */

constexpr Var<reg32> kLabelsOn{0x55fe3c}, kMenuOpen{0x7a3d10}, kCursorX{0x55d6b0}, kCursorY{0x55d6b4};
constexpr Var<int32_t> kPaneRight{0x7d367c}, kPaneBottom{0x7d3678}, kRowGap{0x55d6c0}, kColumnGap{0x7925c0};
constexpr Var<int32_t> kLabelCount{0x5efda0}, kSplitScreens{0x6fd3a0}, kReplayOn{0x7cd948};
constexpr Var<float> kRowsFactorA{0x53b8c0}, kRowsFactorB{0x53b8c4};
constexpr Var<double> kRowsFactorC{0x53b8d4};
constexpr reg32 kCursorGet = 0x4dab10, kRacerPicked = 0x4215c0, kRacerShown = 0x485640;
constexpr reg32 kRaceCars = 0x5efac8, kPaneRects = 0x7497d8, kLabelRecords = 0x72529c;
constexpr reg32 kHoverFlags = 0x55e5ec, kPickedRacer = 0x6fd4f8, kHoverReset = 0x563d3c, kMouseButtons = 0x55e5ed;
// The line the text of a kind of row is drawn in the colour of: kinds 1 to 7, anything else the last.
constexpr reg32 kRowColours[7] = {0xff505050, 0xff505050, 0xff505050, 0xffa0a0a0, 0xffc0c0c0, 0x9ffafa00, 0x9ffafa00};

/* fistp as the code does it without __CHP: `value` rounded the way the x87's
 * control word says. */
int32_t stored(CPU& cpu, double value)
{
    return fp::toInteger<int32_t>(x86::Float(value));
}

/* The HUD's table of labelled rows -- the racers on the screen `group` --
 * drawn from the top y (fourth argument) down, a row at a time: the name in
 * the colour of the row's kind, then the language's line of it ("%d %s" with
 * the label's number), and for the later kinds the text of the label itself in
 * the next column; the rows a text line and a half apart (the Modern Patch's
 * 12.414/13), the first x past the left edge (second argument) by the width
 * 4/640 of the HUD's reference.  While the mouse is on a row in the columns
 * (left and right edges, second and third arguments) and no pointer is
 * on the racer already, the racer is picked (its label's record) and shown.
 * False when the game stops in a call. */
bool drawLabelRows(App* app, CPU& cpu, const Entry& e, reg32& result)
{
    const reg32 ebp = e.frame(6);
    const reg32 base = e.frame(6, 0x3c);
    const HeldEbp held(cpu, ebp);
    const reg32 buffer = ebp - 0x3c;
    const reg32 group = e.eax;
    const reg32 scale = e.arg(app, 0);
    const int32_t left = int32_t(e.arg(app, 1)), right = int32_t(e.arg(app, 2));
    Walk w{app, cpu, base, {e.eax, e.edx, e.ebx, group}};
    const double width = double(int32_t(hudReferenceWidth(app)));   // (tools/apply_hud_scale.py: was [0x7cdac8])
    const double inset = (width * double(kRowsFactorA(app))) * double(kRowsFactorB(app));
    const int32_t textX = stored(cpu, inset) + left + 2;
    int32_t mouseX = 0, mouseY = 0;
    if (kLabelsOn(app) != 0 && kMenuOpen(app) == 0)
    {
        app->getMemory<reg32>(kHoverReset) = 0;
        w.r.eax = 0x55d6b0;
        w.r.edx = 0x55d6b4;
        w.r.ebx = kLabelsOn(app);
        if (!w.call(kCursorGet))
            return false;
        const reg32 x = kCursorX(app) - 8;
        mouseX = kPaneRight(app) - int32_t(x);
        mouseY = kPaneBottom(app) - int32_t(kCursorY(app));
        kCursorX.set(app, x);
    }
    if (!w.call(kTextScale, {scale}))
        return false;
    cpu.fpu.pop();
    w.r.eax = 0x30;
    if (!w.call(kFontWidth))
        return false;
    w.r.edx = w.r.eax;
    w.r.eax = 0x30;
    if (!w.call(kFontHeight))
        return false;
    const int32_t lineHeight = int32_t(w.r.eax + w.r.edx);   // the font's height and its width
    const int32_t rowStep = chopped((double(lineHeight) * 12.414) * kRowsFactorC(app));   // (tools/apply_hud_scale.py: was [0x53b8cc] = 20)
    int32_t y = int32_t(e.arg(app, 3)) + stored(cpu, inset) + 2;
    const int32_t nameX = textX;
    const int32_t numberX = textX + kRowGap(app);
    const int32_t labelX = numberX + kColumnGap(app);
    for (int32_t i = 0; i < kLabelCount(app); ++i)
    {
        const reg32 slotWord = reg32(i) * 4;
        const reg32 target = app->getMemory<reg32>(slotWord + kLabelTargets);
        if (target == 0)
            continue;
        const reg32 label = group * 320 + reg32(i) * 40 + kLabels;
        const reg32 kind = app->getMemory<reg32>(label);
        kTextColour.set(app, kind - 1 > 6 ? 0xff505050 : kRowColours[kind - 1]);
        w.r.edx = reg32(nameX);
        w.r.ebx = reg32(y);
        w.r.eax = target;
        if (!w.call(kTextDrawn))
            return false;
        cpu.fpu.pop();
        w.r.eax = kind;
        if (kind == 1 || kind == 2 || app->getMemory<reg32>(slotWord + kLabelIds) == 1)
        {
            const reg32 id = kind == 1 ? 0x14a : kind == 2 ? 0x14b : 0x14c;
            w.r.eax = id;
            if (!w.call(kLanguageText))
                return false;
            if (!w.call(kSprintf, {buffer, w.r.eax}))
                return false;
        }
        else
        {
            w.r.eax = 0x14d;
            if (!w.call(kLanguageText))
                return false;
            const reg32 line = w.r.eax;
            if (!w.call(kSprintf, {buffer, 0x53b7f4, app->getMemory<reg32>(slotWord + kLabelIds), line}))
                return false;
        }
        w.r.edx = reg32(numberX);
        w.r.eax = buffer;
        w.r.ebx = reg32(y);
        if (!w.call(kTextDrawn))
            return false;
        cpu.fpu.pop();
        if (int32_t(kind) >= 4)
        {
            w.r.eax = label + 4;
            w.r.edx = reg32(labelX);
            w.r.ebx = reg32(y);
            if (!w.call(kTextDrawn))
                return false;
            cpu.fpu.pop();
        }
        y += rowStep;
        if (kLabelsOn(app) == 0 || kMenuOpen(app) != 0)
            continue;
        if (mouseX <= left || mouseX >= right)
            continue;
        if (y - rowStep >= mouseY || y <= mouseY)
            continue;
        if ((app->getMemory<uint8_t>(kMouseButtons) & 1) == 0)
            continue;
        const reg32 record = app->getMemory<reg32>(group * 320 + reg32(i) * 40 + kLabelRecords);
        const reg32 racer = app->getMemory<reg32>(kRaceCars + 4 * app->getMemory<reg32>(record + 0x1f0));
        if ((app->getMemory<uint8_t>(racer + 0x200) & 1) != 0 && app->getMemory<reg32>(racer + 0x2c4) == 3)
            continue;
        if ((app->getMemory<uint8_t>(racer + 0x200) & 1) != 0 && app->getMemory<reg32>(racer + 0x2c4) == 4)
            continue;
        if (kSplitScreens(app) >= 2)
        {
            const reg32 rect = kPaneRects + group * 368;
            if (mouseX >= app->getMemory<int32_t>(rect) && mouseX <= app->getMemory<int32_t>(rect + 8) &&
                mouseY >= app->getMemory<int32_t>(rect + 4) && mouseY <= app->getMemory<int32_t>(rect + 0xc))
                continue;
        }
        if (kSplitScreens(app) == 2 && kReplayOn(app) != 0)
            continue;
        app->getMemory<reg32>(kHoverFlags) = 0;
        const reg32 index = app->getMemory<reg32>(app->getMemory<reg32>(group * 320 + reg32(i) * 40 + kLabelRecords) + 0x1f0);
        app->getMemory<reg32>(kPickedRacer) = index;
        w.r.ebx = group * 320;
        w.r.edx = index;
        w.r.eax = group;
        if (!w.call(kRacerPicked))
            return false;
        w.r.eax = group;
        if (!w.call(kRacerShown))
            return false;
    }
    result = w.r.eax;
    return true;
}

/* ---- 0x48ae70 ---- */

constexpr Var<int32_t> kStandingRows{0x7254f8};      // how many rows the standings have
constexpr reg32 kStandingRow = 0x7254fc;             // by row, 24 bytes: name, colour, value, colour, flag, racer
constexpr Var<float> kStandingsInsetA{0x53b924}, kStandingsInsetB{0x53b928};
constexpr Var<double> kStandingsHeightA{0x53b92c}, kStandingsHeightB{0x53b934};
constexpr Var<int32_t> kNameWidth{0x55d6c0}, kValueWidth{0x55d6c4};
constexpr reg32 kRowCounts = 0x55d6a8;               // by group: how many rows are drawn
constexpr reg32 kRowRects = 0x792340;                // by group (320) and row (20): left, top, right, bottom, racer
constexpr reg32 kBlankText = 0x792320;
constexpr reg32 kTimeText = 0x489750;                // (buffer, number): the number as text
constexpr reg32 kPlusText = 0x53b8dc, kMinusText = 0x53b8e0;

/* The racer's standings drawn at x (second argument), y (third) for group
 * `group` (fourth), the text at the scale given: a row of the table at a
 * time -- its name, then the line "Busted" or the value (a time, signed with
 * a sign column) -- each row's rectangle noted for the mouse (left, top,
 * right, bottom and the racer's number); then the mouse on one of the
 * rectangles, no pointer on a racer already and no player's own, picks the
 * racer and shows him.  Everything is the width and height of the HUD's
 * reference.  False when the game stops in a call. */
bool drawStandings(App* app, CPU& cpu, const Entry& e, reg32& result)
{
    const reg32 ebp = e.frame(6);
    const reg32 base = e.frame(6, 0x128);
    const HeldEbp held(cpu, ebp);
    const reg32 buffer = ebp - 0x128;
    const reg32 scale = e.arg(app, 0), group = e.arg(app, 3);
    Walk w{app, cpu, base, {e.eax, e.edx, e.ebx, e.arg(app, 2)}};
    const auto word = [&](reg32 at) -> reg32 { return app->getMemory<reg32>(at); };
    const auto set = [&](reg32 at, reg32 value) { app->getMemory<reg32>(at) = value; };
    const double inset = (double(int32_t(hudReferenceWidth(app))) * double(kStandingsInsetA(app))) * double(kStandingsInsetB(app));
    const int32_t pad = stored(cpu, inset) + 2;
    const reg32 x = e.arg(app, 1) + reg32(pad);
    w.r.ecx = e.arg(app, 2) + reg32(pad);
    if (!w.call(kTextScale, {scale}))
        return false;
    cpu.fpu.pop();
    const double high = (double(int32_t(hudReferenceHeight(app))) * kStandingsHeightA(app)) * kStandingsHeightB(app);
    w.r.eax = 0x49;
    w.r.edx = reg32(chopped(high));
    if (!w.call(kFontHeight))
        return false;
    const int32_t rowHeight = int32_t(w.r.edx + w.r.eax);
    const reg32 countAt = kRowCounts + 4 * group;
    set(countAt, 0);
    int32_t count = 0;
    w.r.edx = 0;
    w.r.eax = 0;
    for (; count < kStandingRows(app); ++count)
    {
        const reg32 row = kStandingRow + 24 * reg32(count);
        kTextColour.set(app, word(row + 4));
        w.r.ebx = w.r.ecx;
        w.r.edx = x;
        w.r.eax = word(row);
        if (!w.call(kTextDrawn))
            return false;
        cpu.fpu.pop();
        kTextColour.set(app, word(row + 0xc));
        const reg32 nameEnd = reg32(kNameWidth(app)) + x;
        w.r.edx = nameEnd;
        const reg32 value = word(row + 8);
        w.r.ebx = word(row + 0x10);
        if (w.r.ebx != 0)
        {
            w.r.eax = 0x14a;
            w.r.ebx = w.r.ecx;
            if (!w.call(kLanguageText))
                return false;
        }
        else if (value == 0)
        {
            w.r.ebx = w.r.ecx;
            w.r.edx += reg32(kValueWidth(app));
            w.r.eax = kBlankText;
        }
        else
        {
            w.r.eax = buffer;
            w.r.edx = value;
            w.r.ebx = value;
            if (!w.call(kTimeText))
                return false;
            if (int32_t(word(row + 8)) > 0)
            {
                w.r.eax = kMinusText;
                w.r.edx = reg32(kNameWidth(app)) + x;
                w.r.ebx = w.r.ecx;
                if (!w.call(kTextDrawn))
                    return false;
                cpu.fpu.pop();
            }
            if (int32_t(word(row + 8)) < 0)
            {
                w.r.eax = kPlusText;
                w.r.edx = reg32(kNameWidth(app)) + x;
                w.r.ebx = w.r.ecx;
                if (!w.call(kTextDrawn))
                    return false;
                cpu.fpu.pop();
            }
            w.r.eax = reg32(kNameWidth(app)) + x;
            w.r.edx = reg32(kValueWidth(app));
            w.r.ebx = w.r.ecx;
            w.r.edx += w.r.eax;
            w.r.eax = buffer;
        }
        if (!w.call(kTextDrawn))
            return false;
        cpu.fpu.pop();
        const reg32 used = word(countAt);
        const reg32 rect = kRowRects + 320 * group + 20 * used;
        const int32_t top = int32_t(w.r.ecx);
        w.r.ecx += reg32(rowHeight);
        app->getMemory<float>(rect) = float(int32_t(x));
        app->getMemory<float>(rect + 4) = float(top);
        app->getMemory<float>(rect + 8) = float(int32_t(reg32(kNameWidth(app)) + x));
        set(rect + 0x10, word(row + 0x14));
        app->getMemory<float>(rect + 0xc) = float(int32_t(w.r.ecx));
        w.r.ebx = reg32(count) + 1;
        w.r.edx = used + 1;
        set(countAt, used + 1);
    }
    w.r.eax = reg32(count);
    if (count > 0)
    {
        w.r.ebx = reg32(count);
        w.r.edx = word(countAt);
    }
    result = w.r.eax;
    if (kLabelsOn(app) == 0 || kMenuOpen(app) != 0)
        return true;
    set(kHoverReset, 0);
    w.r.edx = 0x55d6b4;
    w.r.eax = 0x55d6b0;
    if (!w.call(kCursorGet))
        return false;
    const reg32 cursor = kCursorX(app) - 8;
    const int32_t mouseY = kPaneBottom(app) - int32_t(kCursorY(app));
    kCursorX.set(app, cursor);
    const int32_t mouseX = kPaneRight(app) - int32_t(cursor);
    int32_t hit = -1;
    for (int32_t j = 0; j < int32_t(word(countAt)); ++j)
    {
        const reg32 rect = kRowRects + 320 * group + 20 * reg32(j);
        const float across = float(mouseX);
        if (jumpBelow(double(across), double(app->getMemory<float>(rect))))
            continue;
        if (jumpBelow(double(mouseY), double(app->getMemory<float>(rect + 4))))
            continue;
        if (double(across) > double(app->getMemory<float>(rect + 8)))
            continue;
        if (double(float(mouseY)) > double(app->getMemory<float>(rect + 0xc)))
            continue;
        hit = j;
        break;
    }
    if (hit == -1 || (app->getMemory<uint8_t>(kMouseButtons) & 1) == 0)
        return true;
    const reg32 index = word(kRowRects + 320 * group + 20 * reg32(hit) + 0x10);
    const reg32 racer = word(kRaceCars + 4 * index);
    if ((app->getMemory<uint8_t>(racer + 0x200) & 1) != 0 && word(racer + 0x2c4) == 3)
        return true;
    if ((app->getMemory<uint8_t>(racer + 0x200) & 1) != 0 && word(racer + 0x2c4) == 4)
        return true;
    if (kSplitScreens(app) >= 2)
    {
        const reg32 rect = kPaneRects + group * 368;
        if (mouseX >= app->getMemory<int32_t>(rect) && mouseX <= app->getMemory<int32_t>(rect + 8) &&
            mouseY >= app->getMemory<int32_t>(rect + 4) && mouseY <= app->getMemory<int32_t>(rect + 0xc))
            return true;
    }
    if (kSplitScreens(app) == 2 && kReplayOn(app) != 0)
        return true;
    set(kHoverFlags, 0);
    set(kPickedRacer, index);
    w.r.ebx = reg32(mouseX);
    w.r.ecx = group * 320;
    w.r.edx = index;
    w.r.eax = group;
    if (!w.call(kRacerPicked))
        return false;
    w.r.eax = group;
    if (!w.call(kRacerShown))
        return false;
    result = w.r.eax;
    return true;
}

/* ---- 0x48aa40 ---- */

constexpr Var<reg32> kColumnsWidth{0x55d6b8}, kColumnsHeight{0x55d6bc}, kTextSize{0x791b4c};
constexpr Var<reg32> kTimeWidth{0x55d6c8};
constexpr Var<int32_t> kTableRight{0x7925c0}, kTableBottom{0x7925c4}, kTableWidth{0x7925c8}, kTableHeight{0x7925cc};
constexpr reg32 kRacerRecords = 0x5efc48;            // by racer: the record
constexpr Var<double> kColumnsA{0x53b8fc}, kColumnsB{0x53b904}, kRowsA{0x53b914}, kRowsB{0x53b91c};
constexpr Var<float> kColumnsC{0x53b90c}, kColumnsD{0x53b910};
constexpr reg32 kColumnsFormat = 0x53b7f4, kCompactFormat = 0x53b658;   // "%d %s", twice
constexpr reg32 kBarsText = 0x792320;

/* A string of the game's copied: bytes up to and with the NUL. */
void stringCopied(App* app, reg32 to, reg32 from)
{
    for (;; ++to, ++from)
    {
        const uint8_t c = app->getMemory<uint8_t>(from);
        app->getMemory<uint8_t>(to) = c;
        if (c == 0)
            break;
    }
}

reg32 stringEnd(App* app, reg32 at)
{
    while (app->getMemory<uint8_t>(at) != 0)
        ++at;
    return at;
}

/* The HUD's tables started for a screen of the size given (two floats, the
 * width first; the sizes the HUD fits itself to are the reference's): the
 * widths of the table's columns -- of a sign, of a time at the current
 * size -- as the widest of the texts that go in them, the name column as wide
 * as the widest racer's name, then the table's width, its height (a line of
 * the font and the row spacing for each racer) and the right edge from the
 * widest of the language's lines for the columns.  False when the game
 * stops in a call. */
bool startColumns(App* app, CPU& cpu, const Entry& e, reg32& result)
{
    const reg32 ebp = e.frame(6);
    const reg32 base = e.frame(6, 0x228);
    const HeldEbp held(cpu, ebp);
    const reg32 first = ebp - 0x128, second = ebp - 0x228;
    Walk w{app, cpu, base, {e.eax, e.edx, e.ebx, e.ecx}};
    const auto word = [&](reg32 at) -> reg32 { return app->getMemory<reg32>(at); };
    // The width of the text at `text`, as the x87 value the width function leaves.
    const auto widthOf = [&](reg32 text, double& width) {
        w.r.eax = text;
        if (!w.call(kTextWidth))
            return false;
        width = double(cpu.fpu.st(0));
        cpu.fpu.pop();
        return true;
    };
    // The greater of two lines' widths as a float, the second's measured again as the code does.
    const auto widest = [&](reg32 one, reg32 other, float& wide) {
        double a, b;
        if (!widthOf(one, a))
            return false;
        const float stored = float(a);
        if (!widthOf(other, b))
            return false;
        double c;
        if (!widthOf(jumpBelow(b, double(stored)) ? one : other, c))
            return false;
        wide = float(c);
        return true;
    };
    kColumnsWidth.set(app, e.arg(app, 0));
    kColumnsHeight.set(app, e.arg(app, 1));
    if (!w.call(kTextScale, {kTextSize(app)}))
        return false;
    cpu.fpu.pop();
    float wide;
    if (!widest(kPlusText, kMinusText, wide))
        return false;
    kValueWidth.set(app, chopped(double(wide)));
    w.r.eax = first;
    w.r.edx = 0;
    if (!w.call(kTimeText))
        return false;
    double length;
    if (!widthOf(first, length))
        return false;
    kTimeWidth.set(app, reg32(chopped(length)));
    stringCopied(app, kBarsText, kMinusText);
    for (;;)
    {
        double bars;
        if (!widthOf(kBarsText, bars))
            return false;
        if (!(double(int32_t(kTimeWidth(app))) > bars))
            break;
        stringCopied(app, stringEnd(app, kBarsText), kMinusText);
    }
    app->getMemory<uint8_t>(stringEnd(app, kBarsText) - 1) = 0;
    const double columns = (double(int32_t(hudReferenceWidth(app))) * kColumnsA(app)) * kColumnsB(app);
    kTimeWidth.set(app, kTimeWidth(app) + reg32(chopped(columns)));
    kNameWidth.set(app, 0);
    for (int32_t racer = 0; racer < kLabelCount(app); ++racer)
    {
        const reg32 record = word(kRacerRecords + 4 * reg32(racer));
        if (record == 0 || word(record + 0x220) == 0)
            continue;
        double name;
        if (!widthOf(word(record + 0x220) + 0x34, name))
            return false;
        const float have = float(double(kNameWidth(app)));
        float taller = have;
        if (!jumpBelow(name, double(have)))
        {
            double again;
            if (!widthOf(word(word(kRacerRecords + 4 * reg32(racer)) + 0x220) + 0x34, again))
                return false;
            taller = float(again);
        }
        kNameWidth.set(app, chopped(double(taller)));
    }
    const double wide2 = double(int32_t(hudReferenceWidth(app)));
    const int32_t nameStep = chopped((wide2 * kColumnsA(app)) * kColumnsB(app));
    const int32_t gap = stored(cpu, (wide2 * double(kColumnsC(app))) * double(kColumnsD(app)));
    kNameWidth.set(app, kNameWidth(app) + nameStep);
    const int32_t sum = kNameWidth(app) + kValueWidth(app);
    kTableWidth.set(app, int32_t(kTimeWidth(app)) + sum + gap + 2);
    const double tall = (double(int32_t(hudReferenceHeight(app))) * kRowsA(app)) * kRowsB(app);
    const int32_t extra = chopped(tall);
    w.r.eax = 0x49;
    if (!w.call(kFontHeight))
        return false;
    kTableBottom.set(app, (kLabelCount(app) + 1) * (int32_t(w.r.eax) + extra));
    // The language's lines for the columns, the widest of each pair.
    reg32 line;
    double a, b, c;
    w.r.eax = 0x14a;
    if (!w.call(kLanguageText))
        return false;
    if (!widthOf(w.r.eax, a))
        return false;
    kTableRight.set(app, chopped(a));
    w.r.eax = 0x14b;
    if (!w.call(kLanguageText))
        return false;
    if (!widthOf(w.r.eax, b))
        return false;
    {
        const float kept = float(double(int32_t(kTableRight(app))));
        float more = kept;
        if (!jumpBelow(b, double(kept)))
        {
            w.r.eax = 0x14b;
            if (!w.call(kLanguageText))
                return false;
            if (!widthOf(w.r.eax, c))
                return false;
            more = float(c);
        }
        kTableRight.set(app, chopped(double(more)));
    }
    w.r.eax = 0x14d;
    if (!w.call(kLanguageText))
        return false;
    line = w.r.eax;
    if (!w.call(kSprintf, {first, kColumnsFormat, 0x63, line}))
        return false;
    {
        double d;
        if (!widthOf(first, d))
            return false;
        const float kept = float(double(int32_t(kTableRight(app))));
        float more = kept;
        if (!jumpBelow(d, double(kept)))
        {
            if (!widthOf(first, c))
                return false;
            more = float(c);
        }
        kTableRight.set(app, chopped(double(more)));
    }
    const auto text = [&](reg32 id, reg32& into) {
        w.r.eax = id;
        if (!w.call(kLanguageText))
            return false;
        into = w.r.eax;
        return true;
    };
    // The top speed lines: the compact format (tools/apply_hud_scale.py: was "%d %s - %d %s %s") with the rest as arguments.
    reg32 l15e, l156a, l156b;
    if (!text(0x15e, l15e) || !text(0x156, l156a) || !text(0x156, l156b))
        return false;
    if (!w.call(kSprintf, {first, kCompactFormat, 0x3e7, l156b, 0x3e7, l156a, l15e}))
        return false;
    reg32 m15e, m157a, m157b;
    if (!text(0x15e, m15e) || !text(0x157, m157a) || !text(0x157, m157b))
        return false;
    if (!w.call(kSprintf, {second, kCompactFormat, 0x3e7, m157b, 0x3e7, m157a, m15e}))
        return false;
    double widthFirst, widthSecond, widest2;
    if (!widthOf(first, widthFirst))
        return false;
    const float storedFirst = float(widthFirst);
    if (!widthOf(second, widthSecond))
        return false;
    if (!widthOf(jumpBelow(widthSecond, double(storedFirst)) ? first : second, widest2))
        return false;
    const float longest = float(widest2);
    const double screen = double(int32_t(hudReferenceWidth(app)));
    const int32_t sumNow = int32_t(kNameWidth(app)) + int32_t(kTableRight(app));
    const double with = double(sumNow) + double(longest);
    const double scaled = (screen * double(kColumnsC(app))) * double(kColumnsD(app));
    const float kept = float(with);
    const int32_t doubled = stored(cpu, scaled) * 2 + 4;
    kTableHeight.set(app, chopped(double(doubled) + double(kept)));
    result = reg32(doubled);
    return true;
}

/* ---- 0x48cdb0 ---- */

constexpr Var<int32_t> kOutlineSamples{0x5dd940};
constexpr Var<reg32> kOutlineBase{0x5dd948};
constexpr Var<double> kOutlineScale{0x53b9b4};
constexpr Var<float> kOutlineHalf{0x53b9bc};
constexpr Var<int32_t> kOutlineLength{0x79c118};
constexpr reg32 kOutlinePoints = 0x792898, kOutlineCentre = 0x79c11c, kOutlineInverse = 0x79c114, kMapColours = 0x793098;
constexpr reg32 kCarSlotRecord = 0x422bd0, kSquareRoot = 0x4ea920, kRgbToHsv = 0x49c880, kHsvToRgb = 0x49ca50, kViewRefreshed = 0x48ccf0;
constexpr reg32 kStatesA = 0x6fbc50, kStatesB = 0x6fbf98, kStatesC = 0x6fc2e0, kStateSize = 0x1a4;
constexpr reg32 kPlayerColours = 0x6fd54c;

/* A float of a state's, as bits, held between 1.0 and 4.0 (the integer
 * comparison of a float's bits the code makes, signed). */
void zoomHeld(App* app, reg32 at)
{
    const int32_t bits = int32_t(app->getMemory<reg32>(at));
    if (bits < 0x3f800000)
        app->getMemory<reg32>(at) = 0x3f800000;
    else if (bits > 0x40800000)
        app->getMemory<reg32>(at) = 0x40800000;
}

/* The track map's outline set up as a race starts, and its colours: the
 * views' zooms held in range, the outline's points -- one in every 128th of
 * the track's samples, scaled by 2^-16 -- centred (their bounding box's
 * middle taken away) and then divided by the farthest one's distance,
 * the first point repeated at the end, and the sixteen players' colours
 * brightened (through the HSV the load code has) into the map's colour
 * table.  False when the game stops in a call. */
bool startMapOutline(App* app, CPU& cpu, const Entry& e, reg32& result)
{
    const reg32 ebp = e.frame(6);
    const reg32 base = e.frame(6, 0x40);
    const HeldEbp held(cpu, ebp);
    Walk w{app, cpu, base, {0, 0x7f7fffff, e.ebx, 0x800000}};
    const auto word = [&](reg32 at) -> reg32 { return app->getMemory<reg32>(at); };
    const auto set = [&](reg32 at, reg32 value) { app->getMemory<reg32>(at) = value; };
    if (!w.call(kCarSlotRecord))
        return false;
    reg32 minY = w.r.edx, minX = w.r.edx, maxX = w.r.ecx, maxY = w.r.ecx, maxSquare = w.r.ecx;
    const reg32 slot = (app->getMemory<uint8_t>(w.r.eax + 0x200) & 0x20) != 0 ? 1 : 0;
    w.r.edx = slot;
    const reg32 first = kStatesA + slot * kStateSize + 0x10;
    w.r.ecx = word(first);
    zoomHeld(app, first);
    if (kScreens(app) == 1)
    {
        zoomHeld(app, kStatesB + slot * kStateSize + 0x10);
        w.r.eax = 1;
        if (!w.call(kCarSlotRecord))
            return false;
        zoomHeld(app, kStatesC + (app->getMemory<reg32>(w.r.eax + 0x200) & 0x20) * kStateSize + 0x10);
    }
    set(kOutlineLength.at, 0);
    for (int32_t i = 0; i < kOutlineSamples(app);)
    {
        const reg32 sample = 36 * reg32(i) + kOutlineBase(app);
        if (sample != 0)
        {
            const reg32 n = reg32(kOutlineLength(app));
            const float x = float(double(app->getMemory<int32_t>(sample)) * kOutlineScale(app));
            const float y = float(double(app->getMemory<int32_t>(sample + 8)) * kOutlineScale(app));
            app->getMemory<float>(kOutlinePoints + 8 * n) = x;
            app->getMemory<float>(kOutlinePoints + 8 * n + 4) = y;
            const double dx = double(x), dy = double(y);
            if (jumpBelow(dx, double(bitsToFloat(minX))))
                minX = bitsOfFloat(x);
            if (dx > double(bitsToFloat(maxX)))
                maxX = bitsOfFloat(x);
            if (jumpBelow(dy, double(bitsToFloat(minY))))
                minY = bitsOfFloat(y);
            if (dy > double(bitsToFloat(maxY)))
                maxY = bitsOfFloat(y);
            const double square = dx * dx + dy * dy;
            if (square > double(bitsToFloat(maxSquare)))
                maxSquare = bitsOfFloat(float(square));
        }
        i += kOutlineSamples(app) / 128;
        kOutlineLength.set(app, kOutlineLength(app) + 1);
    }
    const double centreX = -(double(bitsToFloat(maxX)) + double(bitsToFloat(minX)));
    const double centreY = -(double(bitsToFloat(maxY)) + double(bitsToFloat(minY)));
    const float half = kOutlineHalf(app);
    const int32_t count = kOutlineLength(app) + 1;
    kOutlineLength.set(app, count);
    app->getMemory<float>(kOutlineCentre) = float(centreX * double(half));
    app->getMemory<float>(kOutlineCentre + 4) = float(double(half) * centreY);
    const reg32 last = kOutlinePoints + 8 * reg32(count - 1);
    set(last, word(kOutlinePoints));
    set(last + 4, word(kOutlinePoints + 4));
    for (int32_t i = 0; i < count; ++i)
    {
        const reg32 point = kOutlinePoints + 8 * reg32(i);
        const double cx = double(app->getMemory<float>(kOutlineCentre)), cy = double(app->getMemory<float>(kOutlineCentre + 4));
        app->getMemory<float>(point) = float(cx + double(app->getMemory<float>(point)));
        app->getMemory<float>(point + 4) = float(cy + double(app->getMemory<float>(point + 4)));
    }
    if (!w.call(kSquareRoot, {maxSquare}))
        return false;
    const double root = double(cpu.fpu.st(0));
    cpu.fpu.pop();
    app->getMemory<float>(kOutlineInverse) = float(1.0 / root);
    for (int32_t i = 0; i < count; ++i)
    {
        const reg32 point = kOutlinePoints + 8 * reg32(i);
        const double scale = double(app->getMemory<float>(kOutlineInverse));
        app->getMemory<float>(point) = float(scale * double(app->getMemory<float>(point)));
        app->getMemory<float>(point + 4) = float(scale * double(app->getMemory<float>(point + 4)));
    }
    const reg32 rgb = ebp - 0x30, hsv = ebp - 0x40;
    for (reg32 c = 0; c < 16; ++c)
    {
        const reg32 player = kPlayerColours + 108 * c;
        for (reg32 k = 0; k < 4; ++k)
            set(rgb + 4 * k, app->getMemory<uint8_t>(player + k));
        w.r.eax = rgb;
        w.r.edx = hsv;
        if (!w.call(kRgbToHsv))
            return false;
        int32_t value = int32_t(word(ebp - 0x38) << 2);
        if (value > 0xff)
            value = 0xff;
        else if (value < 0x18)
            value = 0x18;
        set(ebp - 0x38, reg32(value));
        w.r.edx = rgb;
        w.r.eax = hsv;
        if (!w.call(kHsvToRgb))
            return false;
        set(kMapColours + 4 * c, (word(rgb) << 16 | 0xff000000) | (word(rgb + 4) << 8) | word(rgb + 8));
    }
    w.r.eax = 0;
    if (!w.call(kViewRefreshed))
        return false;
    w.r.eax = 1;
    if (!w.call(kViewRefreshed))
        return false;
    result = w.r.eax;
    return true;
}

/* ---- 0x48d110 ---- */

constexpr Var<float> kViewHalf{0x53b9c0}, kRectIn{0x53b9c4}, kRectOut{0x53b9c8}, kMapUnit{0x53b9d0};
constexpr Var<float> kMapSizeA{0x53b9cc}, kMapSizeB{0x53b9d4}, kMapSizeC{0x53b9d8}, kMapSizeD{0x53b9dc};
constexpr Var<reg32> kRectsA{0x791b2c}, kRectsB{0x791b38};
constexpr reg32 kPaneInts = 0x749758;                // by group (368) and shape (16): four ints
constexpr reg32 kPaneFloats = 0x7925d8;              // by group (16): four floats
constexpr reg32 kPaneMiddles = 0x792888;             // by group (4): middle y, then (0x8 further) middle x
constexpr reg32 kPaneSizes = 0x792878;               // by group (4): the size of the smaller side, zoomed
constexpr Var<reg32> kMirrorKind{0x6fd3b8};
constexpr reg32 kIntegersAppended = 0x487350, kHudInsetDrawn = 0x487210, kHudBandDrawn = 0x487340, kCockpitShown = 0x422aa0;

/* The HUD's views fitted to the screen for group `group` (the first player's
 * record decides which of two shapes the panes take): their rectangles from
 * the pane table (ints, kept as floats), their middles and the size of the
 * smaller side zoomed by the view's own zoom; then the HUD's inset and band
 * drawn, up to four rectangles (the cockpit's, the mirrors', the map's) put
 * on the integers' list as chopped ints, and the sizes of the map's points
 * (the width of the HUD's reference by constants).  False when the game stops
 * in a call. */
bool setHudViews(App* app, CPU& cpu, const Entry& e, reg32& result)
{
    const reg32 ebp = e.frame(4);
    const reg32 base = e.frame(4, 0x34);
    const HeldEbp held(cpu, ebp);
    const reg32 group = e.eax;
    Walk w{app, cpu, base, {e.eax, e.edx, e.ebx, e.ecx}};
    const auto word = [&](reg32 at) -> reg32 { return app->getMemory<reg32>(at); };
    const auto set = [&](reg32 at, reg32 value) { app->getMemory<reg32>(at) = value; };
    const auto floatAt = [&](reg32 at) { return double(app->getMemory<float>(at)); };
    const auto integersAppended = [&](reg32 index, reg32 d, reg32 b, reg32 c, reg32 pushed) {
        w.r.eax = index;
        w.r.edx = d;
        w.r.ebx = b;
        w.r.ecx = c;
        return w.call(kIntegersAppended, {pushed});
    };
    if (!w.call(kCarSlotRecord))
        return false;
    const reg32 slot = (app->getMemory<uint8_t>(w.r.eax + 0x200) & 0x20) != 0 ? 1 : 0;
    const reg32 shape = slot != 0 ? 0x16 : 0x11;
    const reg32 view = group + (kScreens(app) == 1 ? 1u : 0u);
    const reg32 viewAt = view * 0x348 + slot * kStateSize;
    const reg32 zoom = word(viewAt + 0x6fbc60);
    const reg32 ints = kPaneInts + (shape << 4) + 368 * group;
    const float a = float(app->getMemory<int32_t>(ints)), b = float(app->getMemory<int32_t>(ints + 4));
    const float c = float(app->getMemory<int32_t>(ints + 8)), d = float(app->getMemory<int32_t>(ints + 0xc));
    const reg32 rect = kPaneFloats + (group << 4);
    app->getMemory<float>(rect + 4) = a;
    app->getMemory<float>(rect + 0xc) = c;
    app->getMemory<float>(rect) = b;
    app->getMemory<float>(rect + 8) = d;
    const double half = double(kViewHalf(app));
    const float across = float(double(c) - double(a));
    app->getMemory<float>(kPaneMiddles + 8 + 4 * group) = float((double(a) + double(c)) * half);
    const float down = float(double(d) - double(b));
    app->getMemory<float>(kPaneMiddles + 4 * group) = float(half * (double(b) + double(d)));
    const float smaller = jumpBelow(double(d) - double(b), double(across)) ? down : across;
    app->getMemory<float>(kPaneSizes + 4 * group) = float((double(smaller) * half) * double(bitsToFloat(zoom)));
    w.r.eax = group;
    w.r.edx = rect;
    w.r.ebx = shape;
    if (!w.call(kViewRefreshed))
        return false;
    w.r.eax = group;
    if (!w.call(kHudInsetDrawn))
        return false;
    w.r.eax = group;
    if (!w.call(kHudBandDrawn))
        return false;
    if (group == 0 && word(slot * kStateSize + 0x6fbc5c) != 0)
    {
        w.r.eax = 0;
        if (!w.call(kCockpitShown))
            return false;
        if (w.r.eax != 0 && !integersAppended(0, word(0x7497ec), word(0x7497e8), word(0x7497f4), word(0x7497f0)))
            return false;
    }
    const reg32 kinds = viewAt;
    if (word(kinds + 0x6fbc6c) == 2 || word(kinds + 0x6fbc6c) == 3)
    {
        const reg32 at = kRectsA(app) + 224 * group;
        const int32_t third = chopped(floatAt(at + 0x44) + double(kRectIn(app)));
        const int32_t last = chopped(floatAt(at + 0x40) + double(kRectIn(app)));
        const int32_t first = chopped(floatAt(at) + double(kRectOut(app)));
        const int32_t second = chopped(floatAt(at + 4) + double(kRectOut(app)));
        if (!integersAppended(group, reg32(second), reg32(first), reg32(third), reg32(last)))
            return false;
    }
    if (word(kinds + 0x6fbc68) == 4 || word(kinds + 0x6fbc68) == 2)
    {
        const reg32 at = kRectsB(app) + 256 * group;
        const int32_t third = chopped(floatAt(at + 0x44));
        const int32_t last = chopped(floatAt(at + 0x40));
        const int32_t first = chopped(floatAt(at));
        const int32_t second = chopped(floatAt(at + 4));
        if (!integersAppended(group, reg32(second), reg32(first), reg32(third), reg32(last)))
            return false;
    }
    if (slot == 0 && kMirrorKind(app) == 3)
    {
        const reg32 at = 368 * group;
        if (!integersAppended(group, word(at + 0x74985c), word(at + 0x749858), word(at + 0x749864), word(at + 0x749860)))
            return false;
    }
    const double screen = double(int32_t(hudReferenceWidth(app)));   // (tools/apply_hud_scale.py: was [0x7cdac8])
    const double unit = double(kMapUnit(app));
    const float one = float((screen * double(kMapSizeA(app))) * unit);
    const float two = float((screen * double(kMapSizeB(app))) * unit);
    const float three = float((screen * double(kMapSizeC(app))) * unit);
    const float four = float(unit * (screen * double(kMapSizeD(app))));
    app->getMemory<float>(0x7930e4) = one;
    app->getMemory<float>(0x7930f8) = two;
    app->getMemory<float>(0x793104) = three;
    app->getMemory<float>(0x7930ec) = four;
    app->getMemory<float>(0x7930dc) = app->getMemory<float>(0x7930e4);
    app->getMemory<float>(0x7930f0) = app->getMemory<float>(0x7930f8);
    app->getMemory<float>(0x7930f4) = app->getMemory<float>(0x793104);
    app->getMemory<float>(0x7930e8) = app->getMemory<float>(0x7930ec);
    set(0x7930e0, word(0x7930f4));
    set(0x7930d8, word(0x7930e0));
    result = word(0x7930e0);
    return true;
}

/* ---- 0x4844c0 ---- */

constexpr reg32 kHudQuads = 0x749a38, kPictureQuads = 0x725748;
constexpr reg32 kQuadQueued = 0x49bd30, kQuadUnqueued = 0x49bdc0;
constexpr reg32 kPaneHidden = 0x47fcb0, kPaneShown = 0x47fbe0, kPaneBandA = 0x480fc0, kPaneBandB = 0x4812e0, kPaneBandC = 0x4800a0;
constexpr reg32 kPaneMapped = 0x481170, kPanePlace = 0x481f90, kPaneRedrawn = 0x4808b0, kPaneInset = 0x481aa0, kPaneSurface = 0x481c50;
constexpr reg32 kPaneCameras = 0x47ee80, kPaneScreen = 0x48d110;
constexpr Var<reg32> kQuadsA{0x791acc}, kQuadsB{0x791ac8}, kQuadsC{0x791b30}, kQuadsD{0x791b24}, kQuadsE{0x791ac4}, kQuadsF{0x791ad0};
constexpr Var<reg32> kQuadsG{0x791b20}, kQuadsH{0x791b48}, kQuadsI{0x791adc}, kQuadsJ{0x791ad8}, kQuadsK{0x791b38};
constexpr Var<reg32> kPaneUp{0x791b1c}, kFrameFlags{0x7a3a58};
constexpr Var<uint16_t> kSizeA{0x791ae8}, kSizeB{0x791aea};
constexpr Var<float> kSplitA{0x53b5f8}, kSplitB{0x53b5fc};
constexpr Var<double> kInsetA{0x53b600}, kInsetB{0x53b608};
constexpr Var<float> kBoxHalf2{0x53b610}, kBoxFour{0x53b614}, kBoxThird{0x53b618}, kBoxThree{0x53b61c};
constexpr reg32 kPaneScale = 0x724760, kPaneMids = 0x724768, kPaneBoxA = 0x724778, kPaneBoxB = 0x724758;

/* The HUD's pane of group `group` set up for a frame: its quads taken off
 * the lists (or put on them), its bands and the mirrors' and the map's panes
 * drawn when the player's record allows it, then the quads' numbers it
 * keeps spaced out (the cockpit's two sizes by 64 over each size, the rest
 * moved in by a fraction), and the pane's own box worked out from its scale.
 * False when the game stops in a call. */
bool setHudPane(App* app, CPU& cpu, const Entry& e, reg32& result)
{
    const reg32 ebp = e.frame(6);
    const reg32 base = e.frame(6, 0x38);
    const HeldEbp held(cpu, ebp);
    const reg32 s = e.eax;
    Walk w{app, cpu, base, {e.eax, e.edx, e.ebx, e.ecx}};
    const auto word = [&](reg32 at) -> reg32 { return app->getMemory<reg32>(at); };
    const auto set = [&](reg32 at, reg32 value) { app->getMemory<reg32>(at) = value; };
    const auto floatAt = [&](reg32 at) { return double(app->getMemory<float>(at)); };
    const auto call = [&](reg32 target, std::initializer_list<reg32> stack = {}) { return w.call(target, stack); };
    // A quad (four vertex numbers from `at`) on or off the list at `list`.
    const auto quad = [&](reg32 target, reg32 list, reg32 at) {
        w.r.eax = list;
        w.r.edx = at;
        w.r.ebx = at + 1;
        w.r.ecx = at + 2;
        return w.call(target, {at + 3});
    };
    w.r.eax = s;
    w.r.edx = (kScreens(app) == 1 ? 1u : 0u) + s;
    if (!call(kCarSlotRecord))
        return false;
    const reg32 view = w.r.edx;
    const reg32 record = w.r.eax;
    w.r.edx = (w.r.edx & 0xffffff00) | app->getMemory<uint8_t>(record + 0x200);
    const reg32 slot = (app->getMemory<uint8_t>(record + 0x200) & 0x20) != 0 ? 1 : 0;
    w.r.eax = slot;
    w.r.ecx = view;
    if (view != 0 && kScreens(app) != 1)
    {
        result = w.r.eax;
        return true;
    }
    const reg32 viewAt = view * 0x348 + slot * kStateSize;
    quad(kQuadUnqueued, kHudQuads, kQuadsA(app) + 4 * s);
    w.r.eax = (s << 5) + kQuadsB(app);
    if (!call(kPaneHidden))
        return false;
    w.r.edx = view * 0x348;
    w.r.eax = slot * kStateSize;
    w.r.ebx = word(viewAt + 0x6fbc50);
    const bool drawn = w.r.ebx != 0 && (kPaneUp(app) != 0 || kSplitScreens(app) != 2);
    if (drawn)
    {
        w.r.eax = s;
        if (!call(kPaneBandA))
            return false;
        w.r.eax = s;
        if (!call(kPaneBandB))
            return false;
        w.r.eax = s;
        w.r.edx = view;
        if (!call(kPaneBandC))
            return false;
        w.r.eax = w.r.edx * 0x348;
        w.r.edx = slot * kStateSize;
        w.r.eax += w.r.edx;
        w.r.ebx = word(viewAt + 0x6fbc6c);
        const bool flagged = (app->getMemory<uint8_t>(record + 0x200) & 4) != 0;
        if ((w.r.ebx == 2 || w.r.ebx == 3) && flagged)
        {
            w.r.edx = kQuadsC(app);
            w.r.eax = (s << 5) + w.r.edx;
            if (!call(kPaneShown))
                return false;
            quad(kQuadQueued, kHudQuads, kQuadsD(app) + 7 * s);
        }
        else
        {
            w.r.ecx = kQuadsC(app);
            w.r.eax = (s << 5) + w.r.ecx;
            if (!call(kPaneHidden))
                return false;
            quad(kQuadUnqueued, kHudQuads, kQuadsD(app) + 7 * s);
        }
        w.r.edx = slot * kStateSize;
        w.r.eax = view * 0x348 + w.r.edx;
        w.r.edx = word(viewAt + 0x6fbc68);
        reg32 pair;
        if ((w.r.edx == 4 || w.r.edx == 2) && flagged)
        {
            w.r.ebx = kQuadsE(app);
            w.r.eax = (s << 5) + w.r.ebx;
            if (!call(kPaneShown))
                return false;
            pair = kQuadsF(app) + 8 * s;
            if (!quad(kQuadQueued, kHudQuads, pair) || !quad(kQuadQueued, kHudQuads, pair + 4))
                return false;
            w.r.ebx = (s << 8) + kQuadsK(app);
            w.r.edx = slot;
            w.r.eax = view;
            if (!call(kPaneMapped))
                return false;
        }
        else
        {
            w.r.eax = (s << 5) + kQuadsE(app);
            if (!call(kPaneHidden))
                return false;
            pair = kQuadsF(app) + 8 * s;
            if (!quad(kQuadUnqueued, kHudQuads, pair) || !quad(kQuadUnqueued, kHudQuads, pair + 4))
                return false;
        }
        w.r.eax = s;
        if (!call(kPanePlace))
            return false;
        w.r.edx = 4 * s;
        if (w.r.eax != 0)
        {
            if ((kFrameFlags(app) & 2) == 0)
            {
                w.r.edx += kQuadsG(app);
                if (!quad(kQuadQueued, kPictureQuads, w.r.edx))
                    return false;
            }
            w.r.edx = kQuadsH(app);
            w.r.eax = (s << 5) + w.r.edx;
            if (!call(kPaneShown))
                return false;
        }
        else
        {
            w.r.ecx = kQuadsH(app);
            w.r.eax = (s << 5) + w.r.ecx;
            if (!call(kPaneHidden))
                return false;
            w.r.edx += kQuadsG(app);
            if (!quad(kQuadUnqueued, kPictureQuads, w.r.edx))
                return false;
        }
        w.r.ecx = slot;
        w.r.eax = slot * kStateSize;
        w.r.edx = view * 0x348;
        if (word(viewAt + 0x6fbc58) != 0 && (kLabelCount(app) > 1 || slot != 0))
        {
            if ((kFrameFlags(app) & 2) == 0)
            {
                w.r.edx = kQuadsI(app) + 4 * s;
                if (!quad(kQuadQueued, kPictureQuads, w.r.edx))
                    return false;
                w.r.edx = kQuadsJ(app);
                w.r.eax = (s << 5) + w.r.edx;
                if (!call(kPaneShown))
                    return false;
            }
        }
        else
        {
            w.r.ecx = kQuadsJ(app);
            w.r.eax = (s << 5) + w.r.ecx;
            if (!call(kPaneHidden))
                return false;
            if ((kFrameFlags(app) & 2) == 0)
            {
                w.r.edx = kQuadsI(app) + 4 * s;
                if (!quad(kQuadUnqueued, kPictureQuads, w.r.edx))
                    return false;
            }
        }
    }
    else
    {
        const reg32 shifted = s << 5;
        w.r.ebx = shifted;
        w.r.eax = kQuadsC(app) + shifted;
        if (!call(kPaneHidden))
            return false;
        if (!quad(kQuadUnqueued, kHudQuads, kQuadsD(app) + 7 * s))
            return false;
        w.r.edx = shifted;
        w.r.eax = kQuadsE(app) + shifted;
        if (!call(kPaneHidden))
            return false;
        const reg32 pair = kQuadsF(app) + 8 * s;
        if (!quad(kQuadUnqueued, kHudQuads, pair) || !quad(kQuadUnqueued, kHudQuads, pair + 4))
            return false;
        w.r.edx = shifted;
        w.r.eax = kQuadsH(app) + shifted;
        if (!call(kPaneHidden))
            return false;
        const reg32 own = 4 * s;
        if (!quad(kQuadUnqueued, kPictureQuads, kQuadsG(app) + own))
            return false;
        w.r.ebx = shifted;
        w.r.eax = kQuadsJ(app) + shifted;
        if (!call(kPaneHidden))
            return false;
        if ((kFrameFlags(app) & 2) == 0 && !quad(kQuadUnqueued, kPictureQuads, kQuadsI(app) + own))
            return false;
        w.r.eax = s;
        if (!call(kPaneRedrawn))
            return false;
    }
    // The pane's quads spaced out by the kind of view.
    const reg32 kind = word(viewAt + 0x6fbc6c);
    w.r.ecx = view * 0x348 + slot * kStateSize;
    w.r.ebx = kind;
    if (kind != 0)
    {
        const reg32 at = kRectsA(app) + 224 * s;
        const double scale = floatAt(kPaneScale + 4 * s);
        if (kind == 3)
        {
            const double first = double(kSizeA(app)), second = double(kSizeB(app));
            const float a = float(1.0 / (first * double(kSplitA(app))));
            const float b = float(double(kSplitB(app)) / first - double(a));
            app->getMemory<float>(at + 0x78) = a;
            app->getMemory<float>(at + 0x58) = b;
            app->getMemory<float>(at + 0x18) = a;
            const float c = float(1.0 / (double(kSplitA(app)) * second));
            set(at + 0x3c, bitsOfFloat(c));
            app->getMemory<float>(at + 0x38) = b;
            const float d = float(double(kSplitB(app)) / second - double(c));
            app->getMemory<float>(at + 0x7c) = d;
            set(at + 0x1c, bitsOfFloat(c));
            app->getMemory<float>(at + 0x5c) = d;
            const double shift = s != 0 ? kInsetA(app) : kInsetB(app);
            for (const reg32 field : {0x18u, 0x38u, 0x58u, 0x78u})
                app->getMemory<float>(at + field) = float(floatAt(at + field) + shift);
            app->getMemory<float>(kPaneMids + 8 * s) = float((floatAt(at) + floatAt(at + 0x40)) * double(kBoxHalf2(app)));
            app->getMemory<float>(kPaneBoxA + 4 * s) = float(scale * double(kBoxFour(app)));
            app->getMemory<float>(kPaneBoxB + 4 * s) = float(scale * double(kBoxThird(app)));
            app->getMemory<float>(kPaneMids + 4 + 8 * s) = float(double(kBoxHalf2(app)) * (floatAt(at + 4) + floatAt(at + 0x44)));
        }
        else
        {
            const double three = scale * double(kBoxThree(app));
            app->getMemory<float>(kPaneBoxA + 4 * s) = float(three * double(kBoxFour(app)));
            app->getMemory<float>(kPaneBoxB + 4 * s) = float(three * double(kBoxThird(app)));
        }
    }
    w.r.ebx = kScreens(app) == 1 ? word(0x7cdb14) : word(0x7cdb94);
    w.r.edx = kScreens(app) == 1 ? word(0x7cdb10) : word(0x7cdb90);
    w.r.eax = s;
    if (!call(kPaneScreen))
        return false;
    w.r.eax = s;
    w.r.edx = slot;
    if (!call(kPaneInset))
        return false;
    if (w.r.edx == 0)
    {
        w.r.edx = record;
        w.r.eax = s;
        if (!call(kPaneCameras))
            return false;
    }
    w.r.eax = s;
    if (!call(kPaneSurface))
        return false;
    result = w.r.eax;
    return true;
}

/* ---- 0x484c10 ---- */

/* A tiny x87 stack of doubles for the tables' float copies: the code moves
 * floats from place to place through the x87 (a store rounds to float). */
struct X87
{
    double v[8];
    int n = 0;
    void push(double d) { v[n++] = d; }
    double pop() { return v[--n]; }
    double& st(int i) { return v[n - 1 - i]; }
    void fxch(int i) { std::swap(st(0), st(i)); }
    void fst_st(int i) { st(i) = st(0); }
    void fstp_st(int i)
    {
        const double t = st(0);
        --n;
        st(i - 1) = t;
    }
};

constexpr reg32 kLayoutPlaced = 0x480910, kHudBegun = 0x47fd80, kHudSurfaces = 0x481290, kHudBars = 0x4815d0, kHudBarsB = 0x4818b0;
constexpr reg32 kHudDials = 0x481510, kHudDialsB = 0x481570, kHudScalesSet = 0x484b70;
constexpr reg32 kHudGaugeA = 0x4812e0, kHudGaugeB = 0x480fc0;

/* The HUD's layout started as a race starts: the 23 places of each of two
 * screens laid out, the text sizes set, the panes' gauges, surfaces, bars and
 * dials started, the map pane's box (its width times a constant) kept, the
 * panes fitted and their insets drawn; then the wider of the two titles of
 * the standings measured (in the language's text, at the HUD's text size) and
 * the table of the places' boxes filled in -- the screen's size and the text's
 * height and width as floats, copied from one place to the next through the
 * x87.  False when the game stops in a call. */
bool startHudTables(App* app, CPU& cpu, const Entry& e, reg32& result)
{
    const reg32 ebp = e.frame(6);
    const reg32 base = e.frame(6, 0x38);
    const HeldEbp held(cpu, ebp);
    Walk w{app, cpu, base, {e.eax, e.edx, e.ebx, e.ecx}};
    const auto W = [&](reg32 a) { return app->getMemory<reg32>(a); };
    const auto F = [&](reg32 a) { return double(app->getMemory<float>(a)); };
    const auto SF = [&](reg32 a, double v) { app->getMemory<float>(a) = float(v); };
    const reg32 height = reg32(kScreenHeight(app)), width = reg32(kScreenWidth(app));
    for (reg32 a = 0; a < 2; ++a)
        for (reg32 b = 0; b < 23; ++b)
        {
            w.r.eax = a;
            w.r.edx = b;
            w.r.ecx = height;
            w.r.ebx = width;
            if (!w.call(kLayoutPlaced))
                return false;
        }
    if (!w.call(kHudScalesSet) || !w.call(kHudBegun))
        return false;
    for (reg32 c = 0; c < 2; ++c)
    {
        w.r.ecx = c;
        w.r.eax = c;
        if (!w.call(kHudGaugeA))
            return false;
    }
    for (reg32 c = 0; c < 2; ++c)
    {
        w.r.ecx = c;
        w.r.edx = width;
        w.r.eax = c;
        if (!w.call(kHudSurfaces))
            return false;
    }
    w.r.ecx = 2;
    w.r.eax = 0;
    if (!w.call(kHudBars))
        return false;
    w.r.eax = 0;
    if (!w.call(kHudBarsB))
        return false;
    w.r.eax = 1;
    w.r.ecx = 0;
    if (!w.call(kHudBarsB))
        return false;
    for (reg32 c = 0; c < 2; ++c)
    {
        w.r.ecx = c;
        w.r.eax = c;
        if (!w.call(kHudGaugeB))
            return false;
    }
    X87 x;
    reg32 eax = 0, ebx = 0, ecx = 0, edx = 0, esi = width, edi = height;
    eax = w.r.eax;
    ebx = w.r.ebx;
    edx = w.r.edx;
    // The map pane's box: its width (the right side past the left) times a constant.
    eax = W(0x791b38u);  // 484cce mov eax, dword ptr [0x791b38]
    x.push(F(eax + 0x40u));  // 484cd3 fld dword ptr [eax + 0x40]
    x.st(0) = (x.st(0)) - (F(eax));  // 484cd6 fsub dword ptr [eax]
    x.st(0) = (x.st(0)) * (F(0x53b630u));  // 484cd8 fmul dword ptr [0x53b630]
    ecx = 0;  // 484cde xor ecx, ecx
    SF((0x791b14u), x.st(0));  // 484ce0 fstp dword ptr [0x791b14]
    x.pop();  // 484ce0 fstp dword ptr [0x791b14]
    for (reg32 c = 0; c < 2; ++c)
    {
        w.r.ebx = height;
        w.r.edx = width;
        w.r.eax = c;
        w.r.ecx = c;
        if (!w.call(kPaneScreen))
            return false;
        w.r.eax = c;
        if (!w.call(kPaneInset))
            return false;
    }
    for (const reg32 target : {kPaneSurface, kHudDials, kHudDialsB})
        for (reg32 c = 0; c < 2; ++c)
        {
            w.r.eax = c;
            if (!w.call(target))
                return false;
        }
    // The two titles measured: the wider of them, at the HUD's text size.
    if (!w.call(kTextScale, {app->getMemory<reg32>(0x791b50)}))
        return false;
    cpu.fpu.pop();
    const auto measured = [&](reg32 line, double& width2) {
        w.r.eax = line;
        if (!w.call(kLanguageText) || !w.call(kTextWidth))
            return false;
        width2 = double(cpu.fpu.st(0));
        cpu.fpu.pop();
        return true;
    };
    double first, second, wide;
    if (!measured(0x1a7, first))
        return false;
    const float firstFloat = float(first);
    if (!measured(0x1a6, second))
        return false;
    if (!measured(jumpBelow(second, double(firstFloat)) ? 0x1a7u : 0x1a6u, wide))
        return false;
    SF(ebp - 0x38u, wide);
    eax = w.r.eax;
    ebx = w.r.ebx;
    ecx = w.r.ecx;
    edx = w.r.edx;
    x.push(F(ebp - 0x38u));  // 484d84 fld dword ptr [ebp - 0x38]
    x.st(0) = (x.st(0)) * (F(0x53b634u));  // 484d87 fmul dword ptr [0x53b634]
    eax = 33u;  // 484d8d mov eax, 0x21
    edx = esi;  // 484d92 mov edx, esi
    x.st(0) = std::trunc(x.st(0));  // __CHP  // 484d94 call 0x4dfd56
    W(ebp - 0x2cu) = stored(cpu, x.pop());  // 484d99 fistp dword ptr [ebp - 0x2c]
    w.r.eax = eax;
    w.r.edx = edx;
    w.r.ebx = ebx;
    w.r.ecx = ecx;
    if (!w.call(kFontHeight))
        return false;
    eax = w.r.eax;
    edx = w.r.edx;
    ebx = w.r.ebx;
    ecx = w.r.ecx;
    ecx = (eax + eax);  // 484da1 lea ecx, [eax + eax]
    edx = reg32(int32_t(edx) >> 31);  // 484da4 sar edx, 0x1f
    eax = esi;  // 484da7 mov eax, esi
    eax = eax - edx;  // 484da9 sub eax, edx
    eax = reg32(int32_t(eax) >> 1);  // 484dab sar eax, 1
    W(ebp - 0x18u) = ecx;  // 484dad mov dword ptr [ebp - 0x18], ecx
    x.push(double(int32_t(W(ebp - 0x18u))));  // 484db0 fild dword ptr [ebp - 0x18]
    SF((ebp - 0x1cu), x.st(0));  // 484db3 fst dword ptr [ebp - 0x1c]
    x.st(0) = (x.st(0)) * (F(0x53b638u));  // 484db6 fmul dword ptr [0x53b638]
    x.st(0) = (x.st(0)) * (F(0x53b63cu));  // 484dbc fmul dword ptr [0x53b63c]
    x.st(0) = (x.st(0)) * (F(0x53b640u));  // 484dc2 fmul dword ptr [0x53b640]
    ebx = W(ebp - 0x2cu);  // 484dc8 mov ebx, dword ptr [ebp - 0x2c]
    x.st(0) = std::trunc(x.st(0));  // __CHP  // 484dcb call 0x4dfd56
    esi = eax;  // 484dd0 mov esi, eax
    W(ebp - 0x28u) = stored(cpu, x.pop());  // 484dd2 fistp dword ptr [ebp - 0x28]
    esi = esi - ebx;  // 484dd5 sub esi, ebx
    edx = W(ebp - 0x28u);  // 484dd7 mov edx, dword ptr [ebp - 0x28]
    ebx = esi;  // 484dda mov ebx, esi
    ebx = ebx - edx;  // 484ddc sub ebx, edx
    W(ebp - 0x18u) = ebx;  // 484dde mov dword ptr [ebp - 0x18], ebx
    ebx = W(ebp - 0x1cu);  // 484de1 mov ebx, dword ptr [ebp - 0x1c]
    W(0x72427cu) = ebx;  // 484de4 mov dword ptr [0x72427c], ebx
    W(0x72425cu) = ebx;  // 484dea mov dword ptr [0x72425c], ebx
    ebx = (ecx + ecx);  // 484df0 lea ebx, [ecx + ecx]
    W(ebp - 0x34u) = ebx;  // 484df3 mov dword ptr [ebp - 0x34], ebx
    W(ebp - 0x10u) = ebx;  // 484df6 mov dword ptr [ebp - 0x10], ebx
    ebx = W(ebp - 0x2cu);  // 484df9 mov ebx, dword ptr [ebp - 0x2c]
    ebx = ebx + eax;  // 484dfc add ebx, eax
    eax = W(ebp - 0x1cu);  // 484dfe mov eax, dword ptr [ebp - 0x1c]
    W(ebp - 0x30u) = ebx;  // 484e01 mov dword ptr [ebp - 0x30], ebx
    W(ebp - 0xcu) = ebx;  // 484e04 mov dword ptr [ebp - 0xc], ebx
    W(0x7242fcu) = eax;  // 484e07 mov dword ptr [0x7242fc], eax
    W(0x7242dcu) = eax;  // 484e0c mov dword ptr [0x7242dc], eax
    eax = (ebx + edx);  // 484e11 lea eax, [ebx + edx]
    W(ebp - 8u) = eax;  // 484e14 mov dword ptr [ebp - 8], eax
    eax = W(ebp - 0x1cu);  // 484e17 mov eax, dword ptr [ebp - 0x1c]
    W(0x72437cu) = eax;  // 484e1a mov dword ptr [0x72437c], eax
    W(0x72435cu) = eax;  // 484e1f mov dword ptr [0x72435c], eax
    eax = edx;  // 484e24 mov eax, edx
    edx = reg32(int32_t(edx) >> 31);  // 484e26 sar edx, 0x1f
    eax = eax - edx;  // 484e29 sub eax, edx
    eax = reg32(int32_t(eax) >> 1);  // 484e2b sar eax, 1
    ebx = eax;  // 484e2d mov ebx, eax
    edx = ecx;  // 484e2f mov edx, ecx
    eax = eax + esi;  // 484e31 add eax, esi
    edx = reg32(int32_t(edx) >> 31);  // 484e33 sar edx, 0x1f
    W(ebp - 4u) = eax;  // 484e36 mov dword ptr [ebp - 4], eax
    eax = ecx;  // 484e39 mov eax, ecx
    eax = eax - edx;  // 484e3b sub eax, edx
    eax = reg32(int32_t(eax) >> 1);  // 484e3d sar eax, 1
    W(ebp - 0x14u) = esi;  // 484e3f mov dword ptr [ebp - 0x14], esi
    x.push(double(int32_t(W(ebp - 0x14u))));  // 484e42 fild dword ptr [ebp - 0x14]
    SF((0x724298u), x.st(0));  // 484e45 fst dword ptr [0x724298]
    SF((0x724278u), x.st(0));  // 484e4b fst dword ptr [0x724278]
    SF((0x724338u), x.st(0));  // 484e51 fst dword ptr [0x724338]
    x.push(double(int32_t(W(ebp - 0x18u))));  // 484e57 fild dword ptr [ebp - 0x18]
    SF((0x7242b8u), x.st(0));  // 484e5a fstp dword ptr [0x7242b8]
    x.pop();  // 484e5a fstp dword ptr [0x7242b8]
    SF((0x7242d8u), x.st(0));  // 484e60 fstp dword ptr [0x7242d8]
    x.pop();  // 484e60 fstp dword ptr [0x7242d8]
    x.push(F(0x7242b8u));  // 484e66 fld dword ptr [0x7242b8]
    SF((0x724258u), x.st(0));  // 484e6c fst dword ptr [0x724258]
    SF((0x724438u), x.st(0));  // 484e72 fst dword ptr [0x724438]
    SF((0x7243d8u), x.st(0));  // 484e78 fstp dword ptr [0x7243d8]
    x.pop();  // 484e78 fstp dword ptr [0x7243d8]
    x.push(double(int32_t(W(ebp - 0x10u))));  // 484e7e fild dword ptr [ebp - 0x10]
    SF((0x7242bcu), x.st(0));  // 484e81 fst dword ptr [0x7242bc]
    SF((0x72429cu), x.st(0));  // 484e87 fst dword ptr [0x72429c]
    SF((0x72433cu), x.st(0));  // 484e8d fst dword ptr [0x72433c]
    SF((0x72431cu), x.st(0));  // 484e93 fst dword ptr [0x72431c]
    SF((0x7243bcu), x.st(0));  // 484e99 fst dword ptr [0x7243bc]
    x.push(double(int32_t(W(ebp - 0xcu))));  // 484e9f fild dword ptr [ebp - 0xc]
    SF((0x724318u), x.st(0));  // 484ea2 fstp dword ptr [0x724318]
    x.pop();  // 484ea2 fstp dword ptr [0x724318]
    SF((0x72439cu), x.st(0));  // 484ea8 fstp dword ptr [0x72439c]
    x.pop();  // 484ea8 fstp dword ptr [0x72439c]
    x.push(F(0x724318u));  // 484eae fld dword ptr [0x724318]
    SF((0x7242f8u), x.st(0));  // 484eb4 fst dword ptr [0x7242f8]
    SF((0x7243b8u), x.st(0));  // 484eba fst dword ptr [0x7243b8]
    SF((0x724358u), x.st(0));  // 484ec0 fstp dword ptr [0x724358]
    x.pop();  // 484ec0 fstp dword ptr [0x724358]
    x.push(double(int32_t(W(ebp - 8u))));  // 484ec6 fild dword ptr [ebp - 8]
    SF((0x724398u), x.st(0));  // 484ec9 fst dword ptr [0x724398]
    SF((0x724378u), x.st(0));  // 484ecf fstp dword ptr [0x724378]
    x.pop();  // 484ecf fstp dword ptr [0x724378]
    esi = W(ebp - 0x34u);  // 484ed5 mov esi, dword ptr [ebp - 0x34]
    x.push(double(int32_t(W(ebp - 4u))));  // 484ed8 fild dword ptr [ebp - 4]
    SF((0x724418u), x.st(0));  // 484edb fst dword ptr [0x724418]
    ecx = ecx - eax;  // 484ee1 sub ecx, eax
    SF((0x7243f8u), x.st(0));  // 484ee3 fstp dword ptr [0x7243f8]
    x.pop();  // 484ee3 fstp dword ptr [0x7243f8]
    W(ebp - 4u) = ecx;  // 484ee9 mov dword ptr [ebp - 4], ecx
    eax = eax + esi;  // 484eec add eax, esi
    x.push(double(int32_t(W(ebp - 4u))));  // 484eee fild dword ptr [ebp - 4]
    W(ebp - 4u) = eax;  // 484ef1 mov dword ptr [ebp - 4], eax
    SF((0x7243fcu), x.st(0));  // 484ef4 fstp dword ptr [0x7243fc]
    x.pop();  // 484ef4 fstp dword ptr [0x7243fc]
    x.push(double(int32_t(W(ebp - 4u))));  // 484efa fild dword ptr [ebp - 4]
    x.push(F(0x7243fcu));  // 484efd fld dword ptr [0x7243fc]
    eax = W(ebp - 0x30u);  // 484f03 mov eax, dword ptr [ebp - 0x30]
    SF((0x7243dcu), x.st(0));  // 484f06 fstp dword ptr [0x7243dc]
    x.pop();  // 484f06 fstp dword ptr [0x7243dc]
    SF((0x72443cu), x.st(0));  // 484f0c fstp dword ptr [0x72443c]
    x.pop();  // 484f0c fstp dword ptr [0x72443c]
    eax = eax - ebx;  // 484f12 sub eax, ebx
    x.push(F(0x72443cu));  // 484f14 fld dword ptr [0x72443c]
    W(ebp - 4u) = eax;  // 484f1a mov dword ptr [ebp - 4], eax
    SF((0x72441cu), x.st(0));  // 484f1d fstp dword ptr [0x72441c]
    x.pop();  // 484f1d fstp dword ptr [0x72441c]
    x.push(double(int32_t(W(ebp - 4u))));  // 484f23 fild dword ptr [ebp - 4]
    SF((0x7244b8u), x.st(0));  // 484f26 fst dword ptr [0x7244b8]
    SF((0x724458u), x.st(0));  // 484f2c fstp dword ptr [0x724458]
    x.pop();  // 484f2c fstp dword ptr [0x724458]
    edx = edi;  // 484f32 mov edx, edi
    eax = edi;  // 484f34 mov eax, edi
    edx = reg32(int32_t(edx) >> 31);  // 484f36 sar edx, 0x1f
    eax = eax - edx;  // 484f39 sub eax, edx
    eax = reg32(int32_t(eax) >> 1);  // 484f3b sar eax, 1
    x.push(F(ebp - 0x1cu));  // 484f3d fld dword ptr [ebp - 0x1c]
    x.push(F(0x724398u));  // 484f40 fld dword ptr [0x724398]
    x.push(F(0x7243fcu));  // 484f46 fld dword ptr [0x7243fc]
    x.push(F(0x72443cu));  // 484f4c fld dword ptr [0x72443c]
    x.push(F(0x724258u));  // 484f52 fld dword ptr [0x724258]
    x.push(F(0x724278u));  // 484f58 fld dword ptr [0x724278]
    x.push(F(0x72429cu));  // 484f5e fld dword ptr [0x72429c]
    x.fxch(5);  // 484f64 fxch st(5)
    SF((0x724498u), x.st(0));  // 484f66 fst dword ptr [0x724498]
    SF((0x724478u), x.st(0));  // 484f6c fstp dword ptr [0x724478]
    x.pop();  // 484f6c fstp dword ptr [0x724478]
    x.push(F(0x7242d8u));  // 484f72 fld dword ptr [0x7242d8]
    x.fxch(4);  // 484f78 fxch st(4)
    SF((0x72447cu), x.st(0));  // 484f7a fst dword ptr [0x72447c]
    SF((0x72445cu), x.st(0));  // 484f80 fstp dword ptr [0x72445c]
    x.pop();  // 484f80 fstp dword ptr [0x72445c]
    x.push(F(0x7242f8u));  // 484f86 fld dword ptr [0x7242f8]
    x.fxch(3);  // 484f8c fxch st(3)
    SF((0x7244bcu), x.st(0));  // 484f8e fst dword ptr [0x7244bc]
    SF((0x72449cu), x.st(0));  // 484f94 fstp dword ptr [0x72449c]
    x.pop();  // 484f94 fstp dword ptr [0x72449c]
    x.push(F(0x72431cu));  // 484f9a fld dword ptr [0x72431c]
    x.fxch(2);  // 484fa0 fxch st(2)
    SF((0x724538u), x.st(0));  // 484fa2 fst dword ptr [0x724538]
    SF((0x7244d8u), x.st(0));  // 484fa8 fstp dword ptr [0x7244d8]
    x.pop();  // 484fa8 fstp dword ptr [0x7244d8]
    x.push(F(0x724358u));  // 484fae fld dword ptr [0x724358]
    x.fxch(1);  // 484fb4 fxch st(1)
    SF((0x724518u), x.st(0));  // 484fb6 fst dword ptr [0x724518]
    W(ebp - 4u) = eax;  // 484fbc mov dword ptr [ebp - 4], eax
    SF((0x7244f8u), x.st(0));  // 484fbf fstp dword ptr [0x7244f8]
    x.pop();  // 484fbf fstp dword ptr [0x7244f8]
    x.push(double(int32_t(W(ebp - 4u))));  // 484fc5 fild dword ptr [ebp - 4]
    SF((ebp - 0x18u), x.st(0));  // 484fc8 fstp dword ptr [ebp - 0x18]
    x.pop();  // 484fc8 fstp dword ptr [ebp - 0x18]
    x.push(F(0x724378u));  // 484fcb fld dword ptr [0x724378]
    x.fxch(4);  // 484fd1 fxch st(4)
    SF((0x7245b8u), x.st(0));  // 484fd3 fst dword ptr [0x7245b8]
    SF((0x724558u), x.st(0));  // 484fd9 fstp dword ptr [0x724558]
    x.pop();  // 484fd9 fstp dword ptr [0x724558]
    x.push(F(0x72439cu));  // 484fdf fld dword ptr [0x72439c]
    x.fxch(3);  // 484fe5 fxch st(3)
    SF((0x724598u), x.st(0));  // 484fe7 fst dword ptr [0x724598]
    SF((0x724578u), x.st(0));  // 484fed fstp dword ptr [0x724578]
    x.pop();  // 484fed fstp dword ptr [0x724578]
    x.push(F(0x7243d8u));  // 484ff3 fld dword ptr [0x7243d8]
    x.fxch(1);  // 484ff9 fxch st(1)
    SF((0x724638u), x.st(0));  // 484ffb fst dword ptr [0x724638]
    SF((0x7245d8u), x.st(0));  // 485001 fstp dword ptr [0x7245d8]
    x.pop();  // 485001 fstp dword ptr [0x7245d8]
    x.push(F(0x7243f8u));  // 485007 fld dword ptr [0x7243f8]
    x.fxch(4);  // 48500d fxch st(4)
    SF((0x724618u), x.st(0));  // 48500f fst dword ptr [0x724618]
    SF((0x7245f8u), x.st(0));  // 485015 fstp dword ptr [0x7245f8]
    x.pop();  // 485015 fstp dword ptr [0x7245f8]
    x.push(F(0x7243dcu));  // 48501b fld dword ptr [0x7243dc]
    x.fxch(1);  // 485021 fxch st(1)
    SF((0x7246b8u), x.st(0));  // 485023 fst dword ptr [0x7246b8]
    SF((0x724658u), x.st(0));  // 485029 fstp dword ptr [0x724658]
    x.pop();  // 485029 fstp dword ptr [0x724658]
    x.push(F(0x72441cu));  // 48502f fld dword ptr [0x72441c]
    x.fxch(4);  // 485035 fxch st(4)
    SF((0x724698u), x.st(0));  // 485037 fst dword ptr [0x724698]
    SF((0x724678u), x.st(0));  // 48503d fstp dword ptr [0x724678]
    x.pop();  // 48503d fstp dword ptr [0x724678]
    eax = W(0x724458u);  // 485043 mov eax, dword ptr [0x724458]
    W(0x724738u) = eax;  // 485048 mov dword ptr [0x724738], eax
    eax = W(0x724738u);  // 48504d mov eax, dword ptr [0x724738]
    W(0x7246d8u) = eax;  // 485052 mov dword ptr [0x7246d8], eax
    x.fxch(5);  // 485057 fxch st(5)
    x.st(0) = (x.st(0)) + (F(ebp - 0x18u));  // 485059 fadd dword ptr [ebp - 0x18]
    x.fxch(4);  // 48505c fxch st(4)
    x.st(0) = (x.st(0)) + (F(ebp - 0x18u));  // 48505e fadd dword ptr [ebp - 0x18]
    x.fxch(4);  // 485061 fxch st(4)
    SF((0x7244fcu), x.st(0));  // 485063 fstp dword ptr [0x7244fc]
    x.pop();  // 485063 fstp dword ptr [0x7244fc]
    x.push(F(0x724478u));  // 485069 fld dword ptr [0x724478]
    x.fxch(4);  // 48506f fxch st(4)
    SF((0x72453cu), x.st(0));  // 485071 fstp dword ptr [0x72453c]
    x.pop();  // 485071 fstp dword ptr [0x72453c]
    x.push(F(0x72445cu));  // 485077 fld dword ptr [0x72445c]
    x.push(F(0x72449cu));  // 48507d fld dword ptr [0x72449c]
    x.fxch(5);  // 485083 fxch st(5)
    SF((0x724718u), x.st(0));  // 485085 fst dword ptr [0x724718]
    SF((0x7246f8u), x.st(0));  // 48508b fstp dword ptr [0x7246f8]
    x.pop();  // 48508b fstp dword ptr [0x7246f8]
    x.fxch(1);  // 485091 fxch st(1)
    x.st(0) = (x.st(0)) + (F(ebp - 0x18u));  // 485093 fadd dword ptr [ebp - 0x18]
    x.fxch(2);  // 485096 fxch st(2)
    x.st(0) = (x.st(0)) + (F(ebp - 0x18u));  // 485098 fadd dword ptr [ebp - 0x18]
    x.fxch(5);  // 48509b fxch st(5)
    x.st(0) = (x.st(0)) + (F(ebp - 0x18u));  // 48509d fadd dword ptr [ebp - 0x18]
    x.fxch(3);  // 4850a0 fxch st(3)
    x.st(0) = (x.st(0)) + (F(ebp - 0x18u));  // 4850a2 fadd dword ptr [ebp - 0x18]
    x.fxch(1);  // 4850a5 fxch st(1)
    x.st(0) = (x.st(0)) + (F(ebp - 0x18u));  // 4850a7 fadd dword ptr [ebp - 0x18]
    x.fxch(4);  // 4850aa fxch st(4)
    x.st(0) = (x.st(0)) + (F(ebp - 0x18u));  // 4850ac fadd dword ptr [ebp - 0x18]
    x.fxch(2);  // 4850af fxch st(2)
    SF((0x7245bcu), x.st(0));  // 4850b1 fstp dword ptr [0x7245bc]
    x.pop();  // 4850b1 fstp dword ptr [0x7245bc]
    x.fxch(4);  // 4850b7 fxch st(4)
    SF((0x72463cu), x.st(0));  // 4850b9 fstp dword ptr [0x72463c]
    x.pop();  // 4850b9 fstp dword ptr [0x72463c]
    x.fxch(1);  // 4850bf fxch st(1)
    SF((0x72467cu), x.st(0));  // 4850c1 fstp dword ptr [0x72467c]
    x.pop();  // 4850c1 fstp dword ptr [0x72467c]
    x.fxch(2);  // 4850c7 fxch st(2)
    SF((0x7246bcu), x.st(0));  // 4850c9 fstp dword ptr [0x7246bc]
    x.pop();  // 4850c9 fstp dword ptr [0x7246bc]
    SF((0x7246fcu), x.st(0));  // 4850cf fstp dword ptr [0x7246fc]
    x.pop();  // 4850cf fstp dword ptr [0x7246fc]
    SF((0x72473cu), x.st(0));  // 4850d5 fstp dword ptr [0x72473c]
    x.pop();  // 4850d5 fstp dword ptr [0x72473c]
    x.push(F(0x7244fcu));  // 4850db fld dword ptr [0x7244fc]
    x.push(F(0x72453cu));  // 4850e1 fld dword ptr [0x72453c]
    x.push(F(0x7244fcu));  // 4850e7 fld dword ptr [0x7244fc]
    x.push(F(0x7245bcu));  // 4850ed fld dword ptr [0x7245bc]
    x.push(F(0x72463cu));  // 4850f3 fld dword ptr [0x72463c]
    x.push(F(0x72467cu));  // 4850f9 fld dword ptr [0x72467c]
    x.fxch(5);  // 4850ff fxch st(5)
    SF((0x7244dcu), x.st(0));  // 485101 fstp dword ptr [0x7244dc]
    x.pop();  // 485101 fstp dword ptr [0x7244dc]
    x.push(F(0x7246bcu));  // 485107 fld dword ptr [0x7246bc]
    x.fxch(4);  // 48510d fxch st(4)
    SF((0x72451cu), x.st(0));  // 48510f fstp dword ptr [0x72451c]
    x.pop();  // 48510f fstp dword ptr [0x72451c]
    x.push(F(0x7246fcu));  // 485115 fld dword ptr [0x7246fc]
    x.fxch(3);  // 48511b fxch st(3)
    SF((0x72457cu), x.st(0));  // 48511d fstp dword ptr [0x72457c]
    x.pop();  // 48511d fstp dword ptr [0x72457c]
    x.push(F(0x72473cu));  // 485123 fld dword ptr [0x72473c]
    x.fxch(2);  // 485129 fxch st(2)
    SF((0x72459cu), x.st(0));  // 48512b fstp dword ptr [0x72459c]
    x.pop();  // 48512b fstp dword ptr [0x72459c]
    SF((0x72461cu), x.st(0));  // 485131 fstp dword ptr [0x72461c]
    x.pop();  // 485131 fstp dword ptr [0x72461c]
    x.fxch(3);  // 485137 fxch st(3)
    SF((0x72465cu), x.st(0));  // 485139 fstp dword ptr [0x72465c]
    x.pop();  // 485139 fstp dword ptr [0x72465c]
    x.fxch(1);  // 48513f fxch st(1)
    SF((0x72469cu), x.st(0));  // 485141 fstp dword ptr [0x72469c]
    x.pop();  // 485141 fstp dword ptr [0x72469c]
    SF((0x7246dcu), x.st(0));  // 485147 fstp dword ptr [0x7246dc]
    x.pop();  // 485147 fstp dword ptr [0x7246dc]
    SF((0x72471cu), x.st(0));  // 48514d fstp dword ptr [0x72471c]
    x.pop();  // 48514d fstp dword ptr [0x72471c]
    x.push(F(0x72457cu));  // 485153 fld dword ptr [0x72457c]
    SF((0x72455cu), x.st(0));  // 485159 fst dword ptr [0x72455c]
    SF((0x7245fcu), x.st(0));  // 48515f fst dword ptr [0x7245fc]
    SF((0x7245dcu), x.st(0));  // 485165 fstp dword ptr [0x7245dc]
    x.pop();  // 485165 fstp dword ptr [0x7245dc]
    result = eax;
    return true;
}

/* ---- 0x489e00 ---- */

constexpr reg32 kSineCosineOf = 0x4ea970, kArcTangentOf = 0x497590, kBoxQuadOf = 0x487a30;

/* An indicator's geometry for group `group` (eax) from its angle (edx), the
 * colour of the 32 entries of its table (the first blue when the fraction
 * edx over ... has run past, else grey), and the rectangle of its pane: the
 * rectangle's four corners moved, the pane's middle and its size, the
 * indicator's two quads' corners worked out through the x87 -- with the
 * x87 stack kept in `x` -- and its box quad drawn.  Everything is registers
 * and the frame the code keeps 0x82 below its ebp.  False when the game
 * stops in a call. */
bool setIndicator(App* app, CPU& cpu, const Entry& e, reg32& result)
{
    const reg32 ebp0 = e.frame(3);
    const reg32 base = e.frame(3, 0x98);
    const reg32 ebp = ebp0 - 0x82;
    const HeldEbp held(cpu, ebp);
    Walk w{app, cpu, base, {e.eax, e.edx, e.ebx, e.ecx}};
    const auto W = [&](reg32 a) { return app->getMemory<reg32>(a); };
    const auto H = [&](reg32 a) { return app->getMemory<uint16_t>(a); };
    const auto F = [&](reg32 a) { return double(app->getMemory<float>(a)); };
    const auto D = [&](reg32 a) { return app->getMemory<double>(a); };
    const auto SF = [&](reg32 a, double v) { app->getMemory<float>(a) = float(v); };
    const auto SD = [&](reg32 a, double v) { app->getMemory<double>(a) = v; };
    X87 x;
    reg32 eax = e.eax, ebx = e.ebx, ecx = e.ecx, edx = e.edx, esi = 0, edi = 0;
    bool fcLess = false, fcEqual = false, fcUnordered = false;
    (void)fcLess, (void)fcEqual, (void)fcUnordered, (void)D, (void)SD, (void)H;
    esi = eax;  // 489e11 mov esi, eax
    W(ebp + 0x76u) = edx;  // 489e13 mov dword ptr [ebp + 0x76], edx
    W(ebp + 0x6au) = ecx;  // 489e16 mov dword ptr [ebp + 0x6a], ecx
    eax = eax << 5;  // 489e19 shl eax, 5
    ecx = (eax * 8);  // 489e1c lea ecx, [eax*8]
    ecx = ecx - eax;  // 489e23 sub ecx, eax
    eax = W(0x791b2cu);  // 489e25 mov eax, dword ptr [0x791b2c]
    ecx = ecx + eax;  // 489e2a add ecx, eax
    eax = (ecx + 0x80u);  // 489e2c lea eax, [ecx + 0x80]
    W(ebp + 0x7au) = eax;  // 489e32 mov dword ptr [ebp + 0x7a], eax
    eax = (ecx + 0xa0u);  // 489e35 lea eax, [ecx + 0xa0]
    edx = W(0x6fd3b0u);  // 489e3b mov edx, dword ptr [0x6fd3b0]
    W(ebp + 0x7eu) = eax;  // 489e41 mov dword ptr [ebp + 0x7e], eax
    edi = (ecx + 0xc0u);  // 489e44 lea edi, [ecx + 0xc0]
    eax = edx == 1 ? edx : 0;
    edx = esi + eax;
    eax = esi;
    w.r = {eax, edx, ebx, ecx};
    if (!w.call(kCarSlotRecord))
        return false;
    eax = w.r.eax;
    edx = w.r.edx;
    ebx = w.r.ebx;
    ecx = w.r.ecx;
    W(ebp + 0x66u) = edx;
    eax = (app->getMemory<uint8_t>(eax + 0x200) & 0x20) != 0 ? 1 : 0;
    edx = (ebx * 4);  // 489e74 lea edx, [ebx*4]
    W(ebp + 0x62u) = eax;  // 489e7b mov dword ptr [ebp + 0x62], eax
    edx = edx + ebx;  // 489e7e add edx, ebx
    ebx = 6u;  // 489e80 mov ebx, 6
    eax = edx;  // 489e85 mov eax, edx
    edx = reg32(int32_t(edx) >> 31);  // 489e87 sar edx, 0x1f
    {
        const int32_t dividend = int32_t(eax);
        eax = reg32(dividend / int32_t(ebx));
        edx = reg32(dividend % int32_t(ebx));
    }
    edx = esi;  // 489e8c mov edx, esi
    ebx = W(ebp + 0x76u);  // 489e8e mov ebx, dword ptr [ebp + 0x76]
    edx = edx << 10;  // 489e91 shl edx, 0xa
    if (int32_t(eax) > int32_t(ebx))
    {
        eax = W(0x791b34) + edx;
        edx = 0;
        do
        {
            eax += 0x20;
            ++edx;
            W(eax - 0x10) = 0xffffffff;
        } while (int32_t(edx) < 0x20);
    }
    else
    {
        eax = W(0x791b34);
        edx += eax;
        eax = 0;
        do
        {
            edx += 0x20;
            ++eax;
            W(edx - 0x10) = 0xffff7777;
        } while (int32_t(eax) < 0x20);
    }
    eax = W(ebp + 0x6au);  // 489ed2 mov eax, dword ptr [ebp + 0x6a]
    W(ecx + 0x70u) = eax;  // 489ed5 mov dword ptr [ecx + 0x70], eax
    W(ecx + 0x50u) = eax;  // 489ed8 mov dword ptr [ecx + 0x50], eax
    W(ecx + 0x30u) = eax;  // 489edb mov dword ptr [ecx + 0x30], eax
    W(ecx + 0x10u) = eax;  // 489ede mov dword ptr [ecx + 0x10], eax
    eax = W(ebp + 0x76u);  // 489ee1 mov eax, dword ptr [ebp + 0x76]
    W(ebp + 0x5eu) = eax;  // 489ee4 mov dword ptr [ebp + 0x5e], eax
    x.push(double(int32_t(W(ebp + 0x5eu))));  // 489ee7 fild dword ptr [ebp + 0x5e]
    x.st(0) = (x.st(0)) * (F(0x53b844u));  // 489eea fmul dword ptr [0x53b844]
    ebx = (ebp + 0x3au);  // 489ef0 lea ebx, [ebp + 0x3a]
    edx = (ebp + 0x46u);  // 489ef3 lea edx, [ebp + 0x46]
    x.st(0) = (x.st(0)) + (F(0x53b848u));  // 489ef6 fadd dword ptr [0x53b848]
    eax = (ebp + 0x36u);  // 489efc lea eax, [ebp + 0x36]
    SF((ebp + 0x36u), x.st(0));  // 489eff fstp dword ptr [ebp + 0x36]
    x.pop();  // 489eff fstp dword ptr [ebp + 0x36]
    w.r = {eax, edx, ebx, ecx};
    if (!w.call(kSineCosineOf))
        return false;
    eax = w.r.eax;
    edx = w.r.edx;
    ebx = w.r.ebx;
    ecx = w.r.ecx;
    eax = reg32(int32_t(W(ebp + 0x66u)) * int32_t(840));  // 489f07 imul eax, dword ptr [ebp + 0x66], 0x348
    edx = reg32(int32_t(W(ebp + 0x62u)) * int32_t(420));  // 489f0e imul edx, dword ptr [ebp + 0x62], 0x1a4
    if (W(edx + eax + 0x6fbc6cu) == 2)
    {
    x.push(F(ebp + 0x46u));  // 489f23 fld dword ptr [ebp + 0x46]
    x.st(0) = (x.st(0)) * (F(0x53b84cu));  // 489f26 fmul dword ptr [0x53b84c]
    eax = 0;  // 489f2c xor eax, eax
    x.st(0) = (x.st(0)) + (F(0x53b84cu));  // 489f2e fadd dword ptr [0x53b84c]
    eax = (eax & 0xffff0000u) | H(0x791ae8u);  // 489f34 mov ax, word ptr [0x791ae8]
    x.push(F(0x53b850u));  // 489f3a fld dword ptr [0x53b850]
    x.fxch(1);  // 489f40 fxch st(1)
    x.st(0) = (x.st(0)) * (x.st(1));  // 489f42 fmul st(1)
    W(ebp + 0x5eu) = eax;  // 489f44 mov dword ptr [ebp + 0x5e], eax
    x.push(double(int32_t(W(ebp + 0x5eu))));  // 489f47 fild dword ptr [ebp + 0x5e]
    SF((ebp + 0x3eu), x.st(0));  // 489f4a fstp dword ptr [ebp + 0x3e]
    x.pop();  // 489f4a fstp dword ptr [ebp + 0x3e]
    x.push(F(0x53b854u));  // 489f4d fld dword ptr [0x53b854]
    x.st(0) = (x.st(0)) / (F(ebp + 0x3eu));  // 489f53 fdiv dword ptr [ebp + 0x3e]
    x.push(F(ebp + 0x3au));  // 489f56 fld dword ptr [ebp + 0x3a]
    x.st(0) = (x.st(0)) * (F(0x53b84cu));  // 489f59 fmul dword ptr [0x53b84c]
    eax = 0;  // 489f5f xor eax, eax
    x.st(0) = (x.st(0)) + (F(0x53b84cu));  // 489f61 fadd dword ptr [0x53b84c]
    eax = (eax & 0xffff0000u) | H(0x791aeau);  // 489f67 mov ax, word ptr [0x791aea]
    x.st(3) = (x.st(3)) * (x.st(0));  // 489f6d fmulp st(3)
    x.pop();  // 489f6d fmulp st(3)
    W(ebp + 0x5eu) = eax;  // 489f6f mov dword ptr [ebp + 0x5e], eax
    x.push(double(int32_t(W(ebp + 0x5eu))));  // 489f72 fild dword ptr [ebp + 0x5e]
    x.push(F(0x53b854u));  // 489f75 fld dword ptr [0x53b854]
    x.st(0) = (x.st(0)) / (x.st(1));  // 489f7b fdiv st(1)
    x.push(F(ebp + 0x3eu));  // 489f7d fld dword ptr [ebp + 0x3e]
    x.st(0) = (x.st(0)) * (F(0x53b858u));  // 489f80 fmul dword ptr [0x53b858]
    x.push(1.0);  // 489f86 fld1
    x.st(1) = (x.st(0)) / (x.st(1));  // 489f88 fdivrp st(1)
    x.pop();  // 489f88 fdivrp st(1)
    x.fxch(2);  // 489f8a fxch st(2)
    x.st(0) = (x.st(0)) * (F(0x53b858u));  // 489f8c fmul dword ptr [0x53b858]
    x.push(1.0);  // 489f92 fld1
    x.st(1) = (x.st(0)) / (x.st(1));  // 489f94 fdivrp st(1)
    x.pop();  // 489f94 fdivrp st(1)
    x.fxch(4);  // 489f96 fxch st(4)
    x.st(0) = (x.st(0)) * (x.st(3));  // 489f98 fmul st(3)
    x.fxch(1);  // 489f9a fxch st(1)
    x.st(5) = (x.st(5)) * (x.st(0));  // 489f9c fmulp st(5)
    x.pop();  // 489f9c fmulp st(5)
    x.push(x.st(2));  // 489f9e fld st(2)
    x.st(0) = (x.st(0)) * (F(0x53b84cu));  // 489fa0 fmul dword ptr [0x53b84c]
    x.push(x.st(5));  // 489fa6 fld st(5)
    x.fxch(3);  // 489fa8 fxch st(3)
    SF((ebp + 0x5eu), x.st(0));  // 489faa fstp dword ptr [ebp + 0x5e]
    x.pop();  // 489faa fstp dword ptr [ebp + 0x5e]
    x.fxch(2);  // 489fad fxch st(2)
    x.st(0) = (x.st(0)) + (F(ebp + 0x5eu));  // 489faf fadd dword ptr [ebp + 0x5e]
    SF((ebp + 0x52u), x.st(0));  // 489fb2 fstp dword ptr [ebp + 0x52]
    x.pop();  // 489fb2 fstp dword ptr [ebp + 0x52]
    eax = W(ebp + 0x52u);  // 489fb5 mov eax, dword ptr [ebp + 0x52]
    x.fxch(1);  // 489fb8 fxch st(1)
    SF((ebp + 0x6eu), x.st(0));  // 489fba fstp dword ptr [ebp + 0x6e]
    x.pop();  // 489fba fstp dword ptr [ebp + 0x6e]
    W(ecx + 0x18u) = eax;  // 489fbd mov dword ptr [ecx + 0x18], eax
    x.push(F(ebp + 0x6eu));  // 489fc0 fld dword ptr [ebp + 0x6e]
    x.st(0) = (x.st(0)) - (F(ecx + 0x18u));  // 489fc3 fsub dword ptr [ecx + 0x18]
    x.fxch(2);  // 489fc6 fxch st(2)
    x.st(0) = (x.st(0)) * (F(0x53b854u));  // 489fc8 fmul dword ptr [0x53b854]
    x.push(F(ebp + 0x3eu));  // 489fce fld dword ptr [ebp + 0x3e]
    x.st(0) = (x.st(0)) * (F(0x53b860u));  // 489fd1 fmul dword ptr [0x53b860]
    x.fxch(1);  // 489fd7 fxch st(1)
    x.st(0) = (x.st(0)) * (F(0x53b85cu));  // 489fd9 fmul dword ptr [0x53b85c]
    x.fxch(1);  // 489fdf fxch st(1)
    SF((ebp + 0x72u), x.st(0));  // 489fe1 fstp dword ptr [ebp + 0x72]
    x.pop();  // 489fe1 fstp dword ptr [ebp + 0x72]
    x.fxch(2);  // 489fe4 fxch st(2)
    x.st(0) = (x.st(0)) * (F(ebp + 0x72u));  // 489fe6 fmul dword ptr [ebp + 0x72]
    x.push(x.st(1));  // 489fe9 fld st(1)
    x.st(0) = (x.st(0)) + (x.st(4));  // 489feb fadd st(4)
    x.fxch(5);  // 489fed fxch st(5)
    x.st(0) = (x.st(0)) + (x.st(3));  // 489fef fadd st(3)
    x.fxch(5);  // 489ff1 fxch st(5)
    SF((ebp + 0x56u), x.st(0));  // 489ff3 fstp dword ptr [ebp + 0x56]
    x.pop();  // 489ff3 fstp dword ptr [ebp + 0x56]
    x.fxch(4);  // 489ff6 fxch st(4)
    x.st(0) = (x.st(0)) - (F(ebp + 0x5eu));  // 489ff8 fsub dword ptr [ebp + 0x5e]
    eax = W(ebp + 0x56u);  // 489ffb mov eax, dword ptr [ebp + 0x56]
    SF((ebp + 0x5eu), x.st(0));  // 489ffe fstp dword ptr [ebp + 0x5e]
    x.pop();  // 489ffe fstp dword ptr [ebp + 0x5e]
    W(ecx + 0x1cu) = eax;  // 48a001 mov dword ptr [ecx + 0x1c], eax
    eax = W(ebp + 0x5eu);  // 48a004 mov eax, dword ptr [ebp + 0x5e]
    x.st(1) = (x.st(1)) + (x.st(0));  // 48a007 faddp st(1)
    x.pop();  // 48a007 faddp st(1)
    W(ecx + 0x38u) = eax;  // 48a009 mov dword ptr [ecx + 0x38], eax
    eax = W(ebp + 0x56u);  // 48a00c mov eax, dword ptr [ebp + 0x56]
    x.st(1) = (x.st(0)) - (x.st(1));  // 48a00f fsubrp st(1)
    x.pop();  // 48a00f fsubrp st(1)
    W(ecx + 0x3cu) = eax;  // 48a011 mov dword ptr [ecx + 0x3c], eax
    eax = W(ebp + 0x5eu);  // 48a014 mov eax, dword ptr [ebp + 0x5e]
    SF((ebp + 0x5eu), x.st(0));  // 48a017 fstp dword ptr [ebp + 0x5e]
    x.pop();  // 48a017 fstp dword ptr [ebp + 0x5e]
    W(ecx + 0x58u) = eax;  // 48a01a mov dword ptr [ecx + 0x58], eax
    eax = W(ebp + 0x5eu);  // 48a01d mov eax, dword ptr [ebp + 0x5e]
    W(ecx + 0x5cu) = eax;  // 48a020 mov dword ptr [ecx + 0x5c], eax
    eax = W(ebp + 0x52u);  // 48a023 mov eax, dword ptr [ebp + 0x52]
    W(ecx + 0x78u) = eax;  // 48a026 mov dword ptr [ecx + 0x78], eax
    eax = W(ebp + 0x5eu);  // 48a029 mov eax, dword ptr [ebp + 0x5e]
    W(ecx + 0x7cu) = eax;  // 48a02c mov dword ptr [ecx + 0x7c], eax
    eax = (esi * 4);  // 48a02f lea eax, [esi*4]
    x.st(0) = (x.st(0)) * (F(eax + 0x724760u));  // 48a036 fmul dword ptr [eax + 0x724760]
    x.st(0) = (x.st(0)) * (F(0x53b854u));  // 48a03c fmul dword ptr [0x53b854]
    x.st(0) = (x.st(0)) * (F(0x53b864u));  // 48a042 fmul dword ptr [0x53b864]
    x.st(0) = (x.st(0)) + (F(ecx));  // 48a048 fadd dword ptr [ecx]
    edx = (esi * 8);  // 48a04a lea edx, [esi*8]
    SF((edx + 0x724768u), x.st(0));  // 48a051 fstp dword ptr [edx + 0x724768]
    x.pop();  // 48a051 fstp dword ptr [edx + 0x724768]
    x.push(F(ebp + 0x6eu));  // 48a057 fld dword ptr [ebp + 0x6e]
    x.st(0) = (x.st(0)) - (F(ecx + 0x1cu));  // 48a05a fsub dword ptr [ecx + 0x1c]
    x.st(0) = (x.st(0)) * (F(ebp + 0x72u));  // 48a05d fmul dword ptr [ebp + 0x72]
    x.st(0) = (x.st(0)) * (F(eax + 0x724760u));  // 48a060 fmul dword ptr [eax + 0x724760]
    x.st(0) = (x.st(0)) * (F(0x53b854u));  // 48a066 fmul dword ptr [0x53b854]
    x.st(0) = (x.st(0)) * (F(0x53b864u));  // 48a06c fmul dword ptr [0x53b864]
    x.st(0) = (x.st(0)) + (F(ecx + 4u));  // 48a072 fadd dword ptr [ecx + 4]
    SF((edx + 0x72476cu), x.st(0));  // 48a075 fstp dword ptr [edx + 0x72476c]
    x.pop();  // 48a075 fstp dword ptr [edx + 0x72476c]
        if (esi != 0)
        {
    x.push(F(ecx + 0x18u));  // 48a07f fld dword ptr [ecx + 0x18]
    x.push(F(ecx + 0x38u));  // 48a082 fld dword ptr [ecx + 0x38]
    x.push(F(ecx + 0x58u));  // 48a085 fld dword ptr [ecx + 0x58]
    x.push(F(ecx + 0x78u));  // 48a088 fld dword ptr [ecx + 0x78]
    x.fxch(3);  // 48a08b fxch st(3)
    x.fxch(2);  // 48a08d fxch st(2)
    x.push(x.st(1));  // 48a08f fld st(1)
    x.push(x.st(4));  // 48a091 fld st(4)
    x.fxch(4);  // 48a093 fxch st(4)
    x.push(D(0x53b86cu));  // 48a095 fld qword ptr [0x53b86c]
    x.fxch(1);  // 48a09b fxch st(1)
    x.st(0) = (x.st(0)) + (x.st(1));  // 48a09d fadd st(1)
    x.fxch(3);  // 48a09f fxch st(3)
    x.st(0) = (x.st(0)) + (x.st(1));  // 48a0a1 fadd st(1)
    x.fxch(2);  // 48a0a3 fxch st(2)
    x.st(0) = (x.st(0)) + (x.st(1));  // 48a0a5 fadd st(1)
    x.fxch(5);  // 48a0a7 fxch st(5)
    x.st(1) = (x.st(1)) + (x.st(0));  // 48a0a9 faddp st(1)
    x.pop();  // 48a0a9 faddp st(1)
    x.fxch(2);  // 48a0ab fxch st(2)
    SD((ebp + 0x22u), x.st(0));  // 48a0ad fstp qword ptr [ebp + 0x22]
    x.pop();  // 48a0ad fstp qword ptr [ebp + 0x22]
    SD((ebp + 0x2au), x.st(0));  // 48a0b0 fstp qword ptr [ebp + 0x2a]
    x.pop();  // 48a0b0 fstp qword ptr [ebp + 0x2a]
    x.fxch(2);  // 48a0b3 fxch st(2)
    x.fstp_st(1);  // 48a0b5 fstp st(1)
    x.fxch(1);  // 48a0b7 fxch st(1)
    x.fstp_st(2);  // 48a0b9 fstp st(2)
    x.push(D(ebp + 0x22u));  // 48a0bb fld qword ptr [ebp + 0x22]
    SF((ecx + 0x18u), x.st(0));  // 48a0be fstp dword ptr [ecx + 0x18]
    x.pop();  // 48a0be fstp dword ptr [ecx + 0x18]
    x.push(D(ebp + 0x2au));  // 48a0c1 fld qword ptr [ebp + 0x2a]
        }
        else
        {
    x.push(F(ecx + 0x18u));  // 48a0c6 fld dword ptr [ecx + 0x18]
    x.push(F(ecx + 0x38u));  // 48a0c9 fld dword ptr [ecx + 0x38]
    x.push(F(ecx + 0x58u));  // 48a0cc fld dword ptr [ecx + 0x58]
    x.push(F(ecx + 0x78u));  // 48a0cf fld dword ptr [ecx + 0x78]
    x.fxch(3);  // 48a0d2 fxch st(3)
    x.fxch(2);  // 48a0d4 fxch st(2)
    x.push(x.st(1));  // 48a0d6 fld st(1)
    x.push(x.st(4));  // 48a0d8 fld st(4)
    x.fxch(4);  // 48a0da fxch st(4)
    x.push(D(0x53b874u));  // 48a0dc fld qword ptr [0x53b874]
    x.fxch(1);  // 48a0e2 fxch st(1)
    x.st(0) = (x.st(0)) + (x.st(1));  // 48a0e4 fadd st(1)
    x.fxch(3);  // 48a0e6 fxch st(3)
    x.st(0) = (x.st(0)) + (x.st(1));  // 48a0e8 fadd st(1)
    x.fxch(2);  // 48a0ea fxch st(2)
    x.st(0) = (x.st(0)) + (x.st(1));  // 48a0ec fadd st(1)
    x.fxch(5);  // 48a0ee fxch st(5)
    x.st(1) = (x.st(1)) + (x.st(0));  // 48a0f0 faddp st(1)
    x.pop();  // 48a0f0 faddp st(1)
    x.fxch(2);  // 48a0f2 fxch st(2)
    SD((ebp + 0x2au), x.st(0));  // 48a0f4 fstp qword ptr [ebp + 0x2a]
    x.pop();  // 48a0f4 fstp qword ptr [ebp + 0x2a]
    SD((ebp + 0x12u), x.st(0));  // 48a0f7 fstp qword ptr [ebp + 0x12]
    x.pop();  // 48a0f7 fstp qword ptr [ebp + 0x12]
    x.fxch(2);  // 48a0fa fxch st(2)
    x.fstp_st(1);  // 48a0fc fstp st(1)
    x.fxch(1);  // 48a0fe fxch st(1)
    x.fstp_st(2);  // 48a100 fstp st(2)
    x.push(D(ebp + 0x2au));  // 48a102 fld qword ptr [ebp + 0x2a]
    SF((ecx + 0x18u), x.st(0));  // 48a105 fstp dword ptr [ecx + 0x18]
    x.pop();  // 48a105 fstp dword ptr [ecx + 0x18]
    x.push(D(ebp + 0x12u));  // 48a108 fld qword ptr [ebp + 0x12]
        }
    SF((ecx + 0x38u), x.st(0));  // 48a10b fstp dword ptr [ecx + 0x38]
    x.pop();  // 48a10b fstp dword ptr [ecx + 0x38]
    SF((ecx + 0x58u), x.st(0));  // 48a10e fstp dword ptr [ecx + 0x58]
    x.pop();  // 48a10e fstp dword ptr [ecx + 0x58]
    SF((ecx + 0x78u), x.st(0));  // 48a111 fstp dword ptr [ecx + 0x78]
    x.pop();  // 48a111 fstp dword ptr [ecx + 0x78]
    }
    x.push(F(esi * 4 + 0x724758u));  // 48a114 fld dword ptr [esi*4 + 0x724758]
    x.st(0) = (x.st(0)) * (F(ebp + 0x3au));  // 48a11b fmul dword ptr [ebp + 0x3a]
    x.st(0) = (x.st(0)) + (F(esi * 8 + 0x724768u));  // 48a11e fadd dword ptr [esi*8 + 0x724768]
    ebx = W(ebp + 0x7au);  // 48a125 mov ebx, dword ptr [ebp + 0x7a]
    SF((ebx), x.st(0));  // 48a128 fstp dword ptr [ebx]
    x.pop();  // 48a128 fstp dword ptr [ebx]
    x.push(F(esi * 4 + 0x724758u));  // 48a12a fld dword ptr [esi*4 + 0x724758]
    x.st(0) = (x.st(0)) * (F(ebp + 0x46u));  // 48a131 fmul dword ptr [ebp + 0x46]
    x.st(0) = (x.st(0)) + (F(esi * 8 + 0x72476cu));  // 48a134 fadd dword ptr [esi*8 + 0x72476c]
    SF((ebx + 4u), x.st(0));  // 48a13b fstp dword ptr [ebx + 4]
    x.pop();  // 48a13b fstp dword ptr [ebx + 4]
    if ((W(ebp + 0x46u) & 0x7fffffffu) == 0)
        W(ebp + 0x46u) = 0x800000;
    x.push(F(ebp + 0x3au));  // 48a14e fld dword ptr [ebp + 0x3a]
    x.st(0) = (x.st(0)) / (F(ebp + 0x46u));  // 48a151 fdiv dword ptr [ebp + 0x46]
    {
        const float pushed = float(x.pop());
        if (!w.call(kArcTangentOf, {0x3f800000, bitsOfFloat(pushed)}))
            return false;
        x.push(double(cpu.fpu.st(0)));
        cpu.fpu.pop();
    }
    x.push(1.0);  // 48a164 fld1
    x.st(1) = (x.st(0)) / (x.st(1));  // 48a166 fdivrp st(1)
    x.pop();  // 48a166 fdivrp st(1)
    x.push(F(ebp + 0x3au));  // 48a168 fld dword ptr [ebp + 0x3a]
    x.st(0) = (x.st(0)) / (F(ebp + 0x46u));  // 48a16b fdiv dword ptr [ebp + 0x46]
    x.push(F(esi * 4 + 0x724778u));  // 48a16e fld dword ptr [esi*4 + 0x724778]
    x.st(0) = (x.st(0)) * (F(0x53b84cu));  // 48a175 fmul dword ptr [0x53b84c]
    x.st(2) = (x.st(2)) * (x.st(0));  // 48a17b fmulp st(2)
    x.pop();  // 48a17b fmulp st(2)
    x.st(0) = -x.st(0);  // 48a17d fchs
    x.push(x.st(0));  // 48a17f fld st(0)
    x.st(0) = (x.st(0)) * (x.st(2));  // 48a181 fmul st(2)
    edx = W(ebp + 0x7eu);  // 48a183 mov edx, dword ptr [ebp + 0x7e]
    x.push(F(esi * 8 + 0x724768u));  // 48a186 fld dword ptr [esi*8 + 0x724768]
    x.st(0) = (x.st(0)) + (x.st(3));  // 48a18d fadd st(3)
    x.fxch(1);  // 48a18f fxch st(1)
    x.fstp_st(2);  // 48a191 fstp st(2)
    SF((edx), x.st(0));  // 48a193 fstp dword ptr [edx]
    x.pop();  // 48a193 fstp dword ptr [edx]
    x.push(F(esi * 8 + 0x72476cu));  // 48a195 fld dword ptr [esi*8 + 0x72476c]
    x.st(0) = (x.st(0)) + (x.st(1));  // 48a19c fadd st(1)
    SF((edx + 4u), x.st(0));  // 48a19e fstp dword ptr [edx + 4]
    x.pop();  // 48a19e fstp dword ptr [edx + 4]
    x.push(F(esi * 8 + 0x724768u));  // 48a1a1 fld dword ptr [esi*8 + 0x724768]
    x.st(2) = (x.st(0)) - (x.st(2));  // 48a1a8 fsubrp st(2)
    x.pop();  // 48a1a8 fsubrp st(2)
    x.fxch(1);  // 48a1aa fxch st(1)
    SF((edi), x.st(0));  // 48a1ac fstp dword ptr [edi]
    x.pop();  // 48a1ac fstp dword ptr [edi]
    x.st(0) = (F(esi * 8 + 0x72476cu)) - (x.st(0));  // 48a1ae fsubr dword ptr [esi*8 + 0x72476c]
    SF((edi + 4u), x.st(0));  // 48a1b5 fstp dword ptr [edi + 4]
    x.pop();  // 48a1b5 fstp dword ptr [edi + 4]
    eax = W(ecx + 4u);  // 48a1b8 mov eax, dword ptr [ecx + 4]
    W(ebp + 2u) = eax;  // 48a1bb mov dword ptr [ebp + 2], eax
    eax = W(ecx + 0x44u);  // 48a1be mov eax, dword ptr [ecx + 0x44]
    W(ebp + 0xau) = eax;  // 48a1c1 mov dword ptr [ebp + 0xa], eax
    eax = W(ecx);  // 48a1c4 mov eax, dword ptr [ecx]
    W(ebp + 6u) = eax;  // 48a1c6 mov dword ptr [ebp + 6], eax
    eax = W(ecx + 0x40u);  // 48a1c9 mov eax, dword ptr [ecx + 0x40]
    W(ebp + 0xeu) = eax;  // 48a1cc mov dword ptr [ebp + 0xe], eax
    eax = W(ebp + 0x7au);  // 48a1cf mov eax, dword ptr [ebp + 0x7a]
    eax = W(eax);  // 48a1d2 mov eax, dword ptr [eax]
    W(ebp - 0x16u) = eax;  // 48a1d4 mov dword ptr [ebp - 0x16], eax
    eax = W(ebp + 0x7au);  // 48a1d7 mov eax, dword ptr [ebp + 0x7a]
    eax = W(eax + 4u);  // 48a1da mov eax, dword ptr [eax + 4]
    W(ebp - 0x12u) = eax;  // 48a1dd mov dword ptr [ebp - 0x12], eax
    eax = W(edi);  // 48a1e0 mov eax, dword ptr [edi]
    W(ebp - 0xeu) = eax;  // 48a1e2 mov dword ptr [ebp - 0xe], eax
    eax = W(edi + 4u);  // 48a1e5 mov eax, dword ptr [edi + 4]
    W(ebp - 0xau) = eax;  // 48a1e8 mov dword ptr [ebp - 0xa], eax
    eax = W(edx);  // 48a1eb mov eax, dword ptr [edx]
    ebx = 4294967295u;  // 48a1ed mov ebx, 0xffffffff
    W(ebp - 6u) = eax;  // 48a1f2 mov dword ptr [ebp - 6], eax
    ecx = 7641656u;  // 48a1f5 mov ecx, 0x749a38
    eax = W(edx + 4u);  // 48a1fa mov eax, dword ptr [edx + 4]
    edx = (ebp + 2u);  // 48a1fd lea edx, [ebp + 2]
    W(ebp - 2u) = eax;  // 48a200 mov dword ptr [ebp - 2], eax
    eax = (ebp - 0x16u);  // 48a203 lea eax, [ebp - 0x16]
    w.r = {eax, edx, ebx, ecx};
    if (!w.call(kBoxQuadOf))
        return false;
    result = w.r.eax;
    return true;
}

/* ---- Functions rewritten from their code by tools (the registers, flags and stack as locals) ---- */

/* The flags the last instruction left, for the conditional jump after it. */
struct Flags
{
    enum Kind { Logic, Sub, Arith, Fpu } k = Logic;
    reg32 fa = 0, fb = 0, fr = 0;
    bool c = false, lt = false, eq = false, un = false;
    void sub(reg32 x, reg32 y) { k = Sub; fa = x; fb = y; fr = x - y; }
    void add(reg32 x, reg32 y) { k = Arith; fr = x + y; c = fr < x; }
    void arith(reg32 v) { k = Arith; fr = v; }
    void logic(reg32 v) { k = Logic; fr = v; }
    void fcmp(double x, double y) { k = Fpu; lt = x < y; eq = x == y; un = !(lt || eq || x > y); }
    bool e() const { return k == Fpu ? (eq || un) : k == Sub ? fa == fb : fr == 0; }
    bool ne() const { return !e(); }
    bool l() const { return k == Sub ? int32_t(fa) < int32_t(fb) : int32_t(fr) < 0; }
    bool le() const { return k == Sub ? int32_t(fa) <= int32_t(fb) : (fr == 0 || int32_t(fr) < 0); }
    bool g() const { return !le(); }
    bool ge() const { return !l(); }
    bool b() const { return k == Fpu ? (lt || un) : k == Sub ? fa < fb : k == Arith ? c : false; }
    bool be() const { return k == Fpu ? (lt || eq || un) : k == Sub ? fa <= fb : k == Arith ? (c || fr == 0) : fr == 0; }
    bool a() const { return !be(); }
    bool ae() const { return !b(); }
    bool s() const { return k == Sub ? int32_t(fa - fb) < 0 : int32_t(fr) < 0; }
    bool ns() const { return !s(); }
};

/* A call from such a function: the arguments already on the guest's stack at
 * `esp`, the registers in and out. */
bool rawCall(App* app, CPU& cpu, reg32 target, reg32 esp, int words, reg32& eax, reg32& edx, reg32& ebx, reg32& ecx)
{
    (void)words;
    cpu.eax = eax;
    cpu.edx = edx;
    cpu.ebx = ebx;
    cpu.ecx = ecx;
    cpu.esp = esp - 4;
    app->getMemory<reg32>(esp - 4) = 0;
    app->dynamic_call(target, cpu);
    if (cpu.terminate)
        return false;
    cpu.esp = esp;
    eax = cpu.eax;
    edx = cpu.edx;
    ebx = cpu.ebx;
    ecx = cpu.ecx;
    return true;
}

/* What a callee left on the x87's stack (a float returned) taken into the function's own. */
void pullX87(CPU& cpu, int before, X87& x)
{
    const int k = cpu.fpu.count - before;
    if (k <= 0)
        return;
    double v[8];
    for (int i = 0; i < k && i < 8; ++i)
    {
        v[i] = double(cpu.fpu.st(0));
        cpu.fpu.pop();
    }
    for (int i = std::min(k, 8) - 1; i >= 0; --i)
        x.push(v[i]);
}


bool decompGaugeQuad(App* app, CPU& cpu, const Entry& e, reg32& result, reg32& popped_)
{
    reg32 eax = e.eax, ebx = e.ebx, ecx = e.ecx, edx = e.edx, esi = e.esi, edi = e.edi, ebp = e.ebp, esp = e.esp;
    Flags fl;
    X87 x;
    const auto rd32 = [&](reg32 a) { return app->getMemory<reg32>(a); };
    const auto rd16 = [&](reg32 a) { return reg32(app->getMemory<uint16_t>(a)); };
    const auto rd8 = [&](reg32 a) { return reg32(app->getMemory<uint8_t>(a)); };
    const auto wr32 = [&](reg32 a, reg32 v) { app->getMemory<reg32>(a) = v; };
    const auto wr16 = [&](reg32 a, reg32 v) { app->getMemory<uint16_t>(a) = uint16_t(v); };
    const auto wr8 = [&](reg32 a, reg32 v) { app->getMemory<uint8_t>(a) = uint8_t(v); };
    const auto F = [&](reg32 a) { return double(app->getMemory<float>(a)); };
    const auto D = [&](reg32 a) { return app->getMemory<double>(a); };
    const auto SF = [&](reg32 a, double v) { app->getMemory<float>(a) = float(v); };
    const auto SD = [&](reg32 a, double v) { app->getMemory<double>(a) = v; };
    esp -= 4; wr32(esp, ebx);  // 487620 push ebx
    esp -= 4; wr32(esp, ecx);  // 487621 push ecx
    esp -= 4; wr32(esp, esi);  // 487622 push esi
    esp -= 4; wr32(esp, edi);  // 487623 push edi
    esp -= 4; wr32(esp, ebp);  // 487624 push ebp
    ebp = esp;  // 487625 mov ebp, esp
    esp -= 692u;  // 487627 sub esp, 0x2b4
    { const reg32 a_ = ebp, b_ = 90u; ebp = a_ - b_; fl.sub(a_, b_); }  // 48762d sub ebp, 0x5a
    wr32((ebp + 0x4eu), eax);  // 487630 mov dword ptr [ebp + 0x4e], eax
    ecx = edx;  // 487633 mov ecx, edx
    wr32((ebp + 0x42u), 1u);  // 487635 mov dword ptr [ebp + 0x42], 1
    edx = rd32((eax * 4u + 0x791b60u));  // 48763c mov edx, dword ptr [eax*4 + 0x791b60]
    x.push(F(ebp + 0x72u));  // 487643 fld dword ptr [ebp + 0x72]
    fl.fcmp(x.st(0), F(edx));  // 487646 fcomp dword ptr [edx]
    x.pop();  // 487646 fcomp dword ptr [edx]
    // fnstsw ax  // 487648 fnstsw ax
    // sahf  // 48764a sahf
    if (fl.be()) goto L_487652;  // 48764b jbe 0x487652
    eax = rd32((ebp + 0x72u));  // 48764d mov eax, dword ptr [ebp + 0x72]
    goto L_487654;  // 487650 jmp 0x487654
L_487652:
    eax = rd32((edx));  // 487652 mov eax, dword ptr [edx]
L_487654:
    wr32((ebp + 0x36u), eax);  // 487654 mov dword ptr [ebp + 0x36], eax
    eax = rd32((ebp + 0x36u));  // 487657 mov eax, dword ptr [ebp + 0x36]
    wr32((ebp + 6u), eax);  // 48765a mov dword ptr [ebp + 6], eax
    eax = rd32((ebp + 0x4eu));  // 48765d mov eax, dword ptr [ebp + 0x4e]
    eax = rd32((eax * 4u + 0x791b60u));  // 487660 mov eax, dword ptr [eax*4 + 0x791b60]
    x.push(F(eax + 8u));  // 487667 fld dword ptr [eax + 8]
    x.st(0) = (x.st(0)) + (F(0x53b698u));  // 48766a fadd dword ptr [0x53b698]
    x.push(F(ebp + 0x7au));  // 487670 fld dword ptr [ebp + 0x7a]
    x.fxch(1);  // 487673 fxch st(1)
    SF((ebp + 0x32u), x.st(0));  // 487675 fstp dword ptr [ebp + 0x32]
    x.pop();  // 487675 fstp dword ptr [ebp + 0x32]
    fl.fcmp(x.st(0), F(ebp + 0x32u));  // 487678 fcomp dword ptr [ebp + 0x32]
    x.pop();  // 487678 fcomp dword ptr [ebp + 0x32]
    // fnstsw ax  // 48767b fnstsw ax
    // sahf  // 48767d sahf
    if (fl.ae()) goto L_487685;  // 48767e jae 0x487685
    eax = rd32((ebp + 0x7au));  // 487680 mov eax, dword ptr [ebp + 0x7a]
    goto L_487688;  // 487683 jmp 0x487688
L_487685:
    eax = rd32((ebp + 0x32u));  // 487685 mov eax, dword ptr [ebp + 0x32]
L_487688:
    wr32((ebp + 0x3eu), eax);  // 487688 mov dword ptr [ebp + 0x3e], eax
    edx = rd32((ebp + 0x4eu));  // 48768b mov edx, dword ptr [ebp + 0x4e]
    eax = rd32((ebp + 0x3eu));  // 48768e mov eax, dword ptr [ebp + 0x3e]
    edx = rd32((edx * 4u + 0x791b60u));  // 487691 mov edx, dword ptr [edx*4 + 0x791b60]
    x.push(F(ebp + 0x76u));  // 487698 fld dword ptr [ebp + 0x76]
    wr32((ebp + 0xeu), eax);  // 48769b mov dword ptr [ebp + 0xe], eax
    fl.fcmp(x.st(0), F(edx + 4u));  // 48769e fcomp dword ptr [edx + 4]
    x.pop();  // 48769e fcomp dword ptr [edx + 4]
    // fnstsw ax  // 4876a1 fnstsw ax
    // sahf  // 4876a3 sahf
    if (fl.be()) goto L_4876ab;  // 4876a4 jbe 0x4876ab
    eax = rd32((ebp + 0x76u));  // 4876a6 mov eax, dword ptr [ebp + 0x76]
    goto L_4876ae;  // 4876a9 jmp 0x4876ae
L_4876ab:
    eax = rd32((edx + 4u));  // 4876ab mov eax, dword ptr [edx + 4]
L_4876ae:
    wr32((ebp + 0x2eu), eax);  // 4876ae mov dword ptr [ebp + 0x2e], eax
    eax = rd32((ebp + 0x2eu));  // 4876b1 mov eax, dword ptr [ebp + 0x2e]
    wr32((ebp + 0xau), eax);  // 4876b4 mov dword ptr [ebp + 0xa], eax
    eax = rd32((ebp + 0x4eu));  // 4876b7 mov eax, dword ptr [ebp + 0x4e]
    eax = rd32((eax * 4u + 0x791b60u));  // 4876ba mov eax, dword ptr [eax*4 + 0x791b60]
    x.push(F(eax + 0xcu));  // 4876c1 fld dword ptr [eax + 0xc]
    x.st(0) = (x.st(0)) + (F(0x53b698u));  // 4876c4 fadd dword ptr [0x53b698]
    x.push(F(ebp + 0x7eu));  // 4876ca fld dword ptr [ebp + 0x7e]
    x.fxch(1);  // 4876cd fxch st(1)
    SF((ebp + 0x3au), x.st(0));  // 4876cf fstp dword ptr [ebp + 0x3a]
    x.pop();  // 4876cf fstp dword ptr [ebp + 0x3a]
    fl.fcmp(x.st(0), F(ebp + 0x3au));  // 4876d2 fcomp dword ptr [ebp + 0x3a]
    x.pop();  // 4876d2 fcomp dword ptr [ebp + 0x3a]
    // fnstsw ax  // 4876d5 fnstsw ax
    // sahf  // 4876d7 sahf
    if (fl.ae()) goto L_4876df;  // 4876d8 jae 0x4876df
    eax = rd32((ebp + 0x7eu));  // 4876da mov eax, dword ptr [ebp + 0x7e]
    goto L_4876e2;  // 4876dd jmp 0x4876e2
L_4876df:
    eax = rd32((ebp + 0x3au));  // 4876df mov eax, dword ptr [ebp + 0x3a]
L_4876e2:
    wr32((ebp + 0x46u), eax);  // 4876e2 mov dword ptr [ebp + 0x46], eax
    eax = rd32((ebp + 0x46u));  // 4876e5 mov eax, dword ptr [ebp + 0x46]
    x.push(F(ebp + 6u));  // 4876e8 fld dword ptr [ebp + 6]
    wr32((ebp + 0x12u), eax);  // 4876eb mov dword ptr [ebp + 0x12], eax
    fl.fcmp(x.st(0), F(ebp + 0xeu));  // 4876ee fcomp dword ptr [ebp + 0xe]
    x.pop();  // 4876ee fcomp dword ptr [ebp + 0xe]
    // fnstsw ax  // 4876f1 fnstsw ax
    // sahf  // 4876f3 sahf
    if (fl.a()) goto L_487a13;  // 4876f4 ja 0x487a13
    x.push(F(ebp + 0xau));  // 4876fa fld dword ptr [ebp + 0xa]
    fl.fcmp(x.st(0), F(ebp + 0x46u));  // 4876fd fcomp dword ptr [ebp + 0x46]
    x.pop();  // 4876fd fcomp dword ptr [ebp + 0x46]
    // fnstsw ax  // 487700 fnstsw ax
    // sahf  // 487702 sahf
    if (fl.a()) goto L_487a13;  // 487703 ja 0x487a13
    x.push(F(ebp + 0x8au));  // 487709 fld dword ptr [ebp + 0x8a]
    x.push(F(ebp + 6u));  // 48770f fld dword ptr [ebp + 6]
    x.st(0) = (x.st(0)) - (F(ebp + 0x72u));  // 487712 fsub dword ptr [ebp + 0x72]
    x.fxch(1);  // 487715 fxch st(1)
    x.st(0) = (x.st(0)) - (F(ebp + 0x82u));  // 487717 fsub dword ptr [ebp + 0x82]
    x.fxch(1);  // 48771d fxch st(1)
    x.st(0) = (x.st(0)) * (x.st(1));  // 48771f fmul st(1)
    x.push(F(ebp + 0x7au));  // 487721 fld dword ptr [ebp + 0x7a]
    x.st(0) = (x.st(0)) - (F(ebp + 0x72u));  // 487724 fsub dword ptr [ebp + 0x72]
    x.push(1.0);  // 487727 fld1
    x.st(1) = (x.st(0)) / (x.st(1));  // 487729 fdivrp st(1)
    x.pop();  // 487729 fdivrp st(1)
    x.push(F(ebp + 0x8eu));  // 48772b fld dword ptr [ebp + 0x8e]
    x.push(F(ebp + 0xau));  // 487731 fld dword ptr [ebp + 0xa]
    x.st(0) = (x.st(0)) - (F(ebp + 0x76u));  // 487734 fsub dword ptr [ebp + 0x76]
    x.fxch(1);  // 487737 fxch st(1)
    x.st(0) = (x.st(0)) - (F(ebp + 0x86u));  // 487739 fsub dword ptr [ebp + 0x86]
    SF((ebp + 0x16u), x.st(0));  // 48773f fstp dword ptr [ebp + 0x16]
    x.pop();  // 48773f fstp dword ptr [ebp + 0x16]
    x.st(0) = (x.st(0)) * (F(ebp + 0x16u));  // 487742 fmul dword ptr [ebp + 0x16]
    x.push(F(ebp + 0x7eu));  // 487745 fld dword ptr [ebp + 0x7e]
    x.st(0) = (x.st(0)) - (F(ebp + 0x76u));  // 487748 fsub dword ptr [ebp + 0x76]
    x.push(1.0);  // 48774b fld1
    x.st(1) = (x.st(0)) / (x.st(1));  // 48774d fdivrp st(1)
    x.pop();  // 48774d fdivrp st(1)
    x.push(F(ebp + 0x46u));  // 48774f fld dword ptr [ebp + 0x46]
    x.push(F(ebp + 0xeu));  // 487752 fld dword ptr [ebp + 0xe]
    x.st(0) = (x.st(0)) - (F(ebp + 0x7au));  // 487755 fsub dword ptr [ebp + 0x7a]
    x.fxch(1);  // 487758 fxch st(1)
    x.st(0) = (x.st(0)) - (F(ebp + 0x7eu));  // 48775a fsub dword ptr [ebp + 0x7e]
    x.fxch(1);  // 48775d fxch st(1)
    x.st(6) = (x.st(6)) * (x.st(0));  // 48775f fmulp st(6)
    x.pop();  // 48775f fmulp st(6)
    x.st(0) = (x.st(0)) * (F(ebp + 0x16u));  // 487761 fmul dword ptr [ebp + 0x16]
    x.fxch(4);  // 487764 fxch st(4)
    x.st(0) = (x.st(0)) * (x.st(3));  // 487766 fmul st(3)
    x.fxch(5);  // 487768 fxch st(5)
    x.st(3) = (x.st(3)) * (x.st(0));  // 48776a fmulp st(3)
    x.pop();  // 48776a fmulp st(3)
    x.fxch(1);  // 48776c fxch st(1)
    x.st(0) = (x.st(0)) * (x.st(1));  // 48776e fmul st(1)
    x.fxch(3);  // 487770 fxch st(3)
    x.st(1) = (x.st(1)) * (x.st(0));  // 487772 fmulp st(1)
    x.pop();  // 487772 fmulp st(1)
    x.fxch(3);  // 487774 fxch st(3)
    x.st(0) = (x.st(0)) + (F(ebp + 0x82u));  // 487776 fadd dword ptr [ebp + 0x82]
    x.fxch(1);  // 48777c fxch st(1)
    x.st(0) = (x.st(0)) + (F(ebp + 0x8au));  // 48777e fadd dword ptr [ebp + 0x8a]
    x.fxch(2);  // 487784 fxch st(2)
    x.st(0) = (x.st(0)) + (F(ebp + 0x86u));  // 487786 fadd dword ptr [ebp + 0x86]
    x.fxch(3);  // 48778c fxch st(3)
    x.st(0) = (x.st(0)) + (F(ebp + 0x8eu));  // 48778e fadd dword ptr [ebp + 0x8e]
    x.fxch(1);  // 487794 fxch st(1)
    SF((ebp + 0x82u), x.st(0));  // 487796 fstp dword ptr [ebp + 0x82]
    x.pop();  // 487796 fstp dword ptr [ebp + 0x82]
    x.fxch(1);  // 48779c fxch st(1)
    SF((ebp + 0x8au), x.st(0));  // 48779e fstp dword ptr [ebp + 0x8a]
    x.pop();  // 48779e fstp dword ptr [ebp + 0x8a]
    x.fxch(1);  // 4877a4 fxch st(1)
    SF((ebp + 0x86u), x.st(0));  // 4877a6 fstp dword ptr [ebp + 0x86]
    x.pop();  // 4877a6 fstp dword ptr [ebp + 0x86]
    SF((ebp + 0x8eu), x.st(0));  // 4877ac fstp dword ptr [ebp + 0x8e]
    x.pop();  // 4877ac fstp dword ptr [ebp + 0x8e]
    fl.sub(ecx, 2u);  // 4877b2 cmp ecx, 2
    if (fl.e()) goto L_487800;  // 4877b5 je 0x487800
    ebx = rd32((ebp + 0x96u));  // 4877b7 mov ebx, dword ptr [ebp + 0x96]
    esp -= 4; wr32(esp, ebx);  // 4877bd push ebx
    esi = rd32((ebp + 0x92u));  // 4877be mov esi, dword ptr [ebp + 0x92]
    esp -= 4; wr32(esp, esi);  // 4877c4 push esi
    edi = rd32((ebp + 0x8eu));  // 4877c5 mov edi, dword ptr [ebp + 0x8e]
    esp -= 4; wr32(esp, edi);  // 4877cb push edi
    eax = rd32((ebp + 0x8au));  // 4877cc mov eax, dword ptr [ebp + 0x8a]
    esp -= 4; wr32(esp, eax);  // 4877d2 push eax
    edx = rd32((ebp + 0x86u));  // 4877d3 mov edx, dword ptr [ebp + 0x86]
    ecx = 0u;  // 4877d9 xor ecx, ecx
    fl.logic(0u);  // 4877d9 xor ecx, ecx
    esp -= 4; wr32(esp, edx);  // 4877db push edx
    wr32((ebp + 0x42u), ecx);  // 4877dc mov dword ptr [ebp + 0x42], ecx
    ecx = rd32((ebp + 0x82u));  // 4877df mov ecx, dword ptr [ebp + 0x82]
    esp -= 4; wr32(esp, ecx);  // 4877e5 push ecx
    ebx = rd32((ebp + 0x12u));  // 4877e6 mov ebx, dword ptr [ebp + 0x12]
    esp -= 4; wr32(esp, ebx);  // 4877e9 push ebx
    esi = rd32((ebp + 0xeu));  // 4877ea mov esi, dword ptr [ebp + 0xe]
    esp -= 4; wr32(esp, esi);  // 4877ed push esi
    edi = rd32((ebp + 0xau));  // 4877ee mov edi, dword ptr [ebp + 0xa]
    esp -= 4; wr32(esp, edi);  // 4877f1 push edi
    eax = rd32((ebp + 6u));  // 4877f2 mov eax, dword ptr [ebp + 6]
    esp -= 4; wr32(esp, eax);  // 4877f5 push eax
    cpu.ebp = ebp; if (!rawCall(app, cpu, 0x486df0, esp, 10, eax, edx, ebx, ecx)) return false;  // 4877f6 call 0x486df0
    esp += 40u;  // 4877f6 call 0x486df0
    goto L_487a13;  // 4877fb jmp 0x487a13
L_487800:
    edi = (ebp - 0x25au);  // 487800 lea edi, [ebp - 0x25a]
    esi = (ebp + 6u);  // 487806 lea esi, [ebp + 6]
    edx = 1u;  // 487809 mov edx, 1
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 48780e movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 48780f movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 487810 movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 487811 movsd dword ptr es:[edi], dword ptr [esi]
    ecx = 0u;  // 487812 xor ecx, ecx
    fl.logic(0u);  // 487812 xor ecx, ecx
    edi = (ebp - 0x16au);  // 487814 lea edi, [ebp - 0x16a]
    esi = (ebp + 0x82u);  // 48781a lea esi, [ebp + 0x82]
    wr32((ebp + 0x56u), edx);  // 487820 mov dword ptr [ebp + 0x56], edx
    wr32((ebp + 0x52u), ecx);  // 487823 mov dword ptr [ebp + 0x52], ecx
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 487826 movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 487827 movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 487828 movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 487829 movsd dword ptr es:[edi], dword ptr [esi]
L_48782a:
    eax = rd32((ebp + 0x52u));  // 48782a mov eax, dword ptr [ebp + 0x52]
    fl.sub(eax, rd32((ebp + 0x56u)));  // 48782d cmp eax, dword ptr [ebp + 0x56]
    if (fl.ge()) goto L_4879ac;  // 487830 jge 0x4879ac
    esi = 0u;  // 487836 xor esi, esi
    fl.logic(0u);  // 487836 xor esi, esi
    wr32((ebp + 0x4au), esi);  // 487838 mov dword ptr [ebp + 0x4a], esi
L_48783b:
    eax = rd32((ebp + 0x4eu));  // 48783b mov eax, dword ptr [ebp + 0x4e]
    edx = rd32((ebp + 0x4au));  // 48783e mov edx, dword ptr [ebp + 0x4a]
    fl.sub(edx, rd32((eax * 4u + 0x55d694u)));  // 487841 cmp edx, dword ptr [eax*4 + 0x55d694]
    if (fl.ge()) goto L_4879a4;  // 487848 jge 0x4879a4
    eax = (ebp - 0x7au);  // 48784e lea eax, [ebp - 0x7a]
    esp -= 4; wr32(esp, eax);  // 487851 push eax
    eax = (ebp - 0x3au);  // 487852 lea eax, [ebp - 0x3a]
    edx = rd32((ebp + 0x4eu));  // 487855 mov edx, dword ptr [ebp + 0x4e]
    esp -= 4; wr32(esp, eax);  // 487858 push eax
    eax = (edx * 4u);  // 487859 lea eax, [edx*4]
    { const reg32 a_ = eax, b_ = edx; eax = a_ + b_; fl.add(a_, b_); }  // 487860 add eax, edx
    { const reg32 r_ = eax << 4; eax = r_; fl.logic(r_); }  // 487862 shl eax, 4
    edx = rd32((ebp + 0x4au));  // 487865 mov edx, dword ptr [ebp + 0x4a]
    { const reg32 a_ = eax, b_ = 7936872u; eax = a_ + b_; fl.add(a_, b_); }  // 487868 add eax, 0x791b68
    { const reg32 r_ = edx << 4; edx = r_; fl.logic(r_); }  // 48786d shl edx, 4
    ebx = (eax + edx);  // 487870 lea ebx, [eax + edx]
    eax = rd32((ebp + 0x52u));  // 487873 mov eax, dword ptr [ebp + 0x52]
    edx = (ebp - 0x16au);  // 487876 lea edx, [ebp - 0x16a]
    { const reg32 r_ = eax << 4; eax = r_; fl.logic(r_); }  // 48787c shl eax, 4
    ecx = (ebp - 0x25au);  // 48787f lea ecx, [ebp - 0x25a]
    { const reg32 a_ = edx, b_ = eax; edx = a_ + b_; fl.add(a_, b_); }  // 487885 add edx, eax
    { const reg32 a_ = eax, b_ = ecx; eax = a_ + b_; fl.add(a_, b_); }  // 487887 add eax, ecx
    ecx = (ebp + 0x26u);  // 487889 lea ecx, [ebp + 0x26]
    cpu.ebp = ebp; if (!rawCall(app, cpu, 0x486340, esp, 2, eax, edx, ebx, ecx)) return false;  // 48788c call 0x486340
    esp += 8u;  // 48788c call 0x486340
    fl.logic(eax & eax);  // 487891 test eax, eax
    if (fl.e()) goto L_48799c;  // 487893 je 0x48799c
    eax = rd32((ebp + 0x26u));  // 487899 mov eax, dword ptr [ebp + 0x26]
    fl.sub(eax, 4u);  // 48789c cmp eax, 4
    if (fl.a()) goto L_48799c;  // 48789f ja 0x48799c
    switch (eax) {  // 4878a5 jmp dword ptr [eax*4 + 0x487600]
    case 0: goto L_4878ac;  // 4878a5 jmp dword ptr [eax*4 + 0x487600]
    case 1: goto L_48797a;  // 4878a5 jmp dword ptr [eax*4 + 0x487600]
    case 2: goto L_487951;  // 4878a5 jmp dword ptr [eax*4 + 0x487600]
    case 3: goto L_48792c;  // 4878a5 jmp dword ptr [eax*4 + 0x487600]
    case 4: goto L_487907;  // 4878a5 jmp dword ptr [eax*4 + 0x487600]
    default: break; }  // 4878a5 jmp dword ptr [eax*4 + 0x487600]
    return false;  // (no entry)  // 4878a5 jmp dword ptr [eax*4 + 0x487600]
L_4878ac:
    edx = rd32((ebp + 0x56u));  // 4878ac mov edx, dword ptr [ebp + 0x56]
    { const reg32 r_ = edx - 1u; edx = r_; fl.arith(r_); }  // 4878af dec edx
    eax = rd32((ebp + 0x52u));  // 4878b0 mov eax, dword ptr [ebp + 0x52]
    wr32((ebp + 0x56u), edx);  // 4878b3 mov dword ptr [ebp + 0x56], edx
    fl.sub(eax, edx);  // 4878b6 cmp eax, edx
    if (fl.ge()) goto L_4878f2;  // 4878b8 jge 0x4878f2
    eax = edx;  // 4878ba mov eax, edx
    edx = rd32((ebp + 0x52u));  // 4878bc mov edx, dword ptr [ebp + 0x52]
    { const reg32 r_ = eax << 4; eax = r_; fl.logic(r_); }  // 4878bf shl eax, 4
    { const reg32 r_ = edx << 4; edx = r_; fl.logic(r_); }  // 4878c2 shl edx, 4
    esi = (eax + ebp - 0x25au);  // 4878c5 lea esi, [eax + ebp - 0x25a]
    edi = (edx + ebp - 0x25au);  // 4878cc lea edi, [edx + ebp - 0x25a]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 4878d3 movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 4878d4 movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 4878d5 movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 4878d6 movsd dword ptr es:[edi], dword ptr [esi]
    ebx = 4294967295u;  // 4878d7 mov ebx, 0xffffffff
    edi = (edx + ebp - 0x16au);  // 4878dc lea edi, [edx + ebp - 0x16a]
    esi = (eax + ebp - 0x16au);  // 4878e3 lea esi, [eax + ebp - 0x16a]
    wr32((ebp + 0x4au), ebx);  // 4878ea mov dword ptr [ebp + 0x4a], ebx
    goto L_487998;  // 4878ed jmp 0x487998
L_4878f2:
    eax = rd32((ebp + 0x4eu));  // 4878f2 mov eax, dword ptr [ebp + 0x4e]
    eax = rd32((eax * 4u + 0x55d694u));  // 4878f5 mov eax, dword ptr [eax*4 + 0x55d694]
    wr32((ebp + 0x4au), eax);  // 4878fc mov dword ptr [ebp + 0x4a], eax
    { const reg32 r_ = rd32((ebp + 0x4au)) + 1u; wr32((ebp + 0x4au), r_); fl.arith(r_); }  // 4878ff inc dword ptr [ebp + 0x4a]
    goto L_48783b;  // 487902 jmp 0x48783b
L_487907:
    eax = rd32((ebp + 0x56u));  // 487907 mov eax, dword ptr [ebp + 0x56]
    { const reg32 r_ = eax << 4; eax = r_; fl.logic(r_); }  // 48790a shl eax, 4
    esi = (ebp - 0xau);  // 48790d lea esi, [ebp - 0xa]
    edi = (eax + ebp - 0x25au);  // 487910 lea edi, [eax + ebp - 0x25a]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 487917 movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 487918 movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 487919 movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 48791a movsd dword ptr es:[edi], dword ptr [esi]
    esi = (ebp - 0x4au);  // 48791b lea esi, [ebp - 0x4a]
    edi = (eax + ebp - 0x16au);  // 48791e lea edi, [eax + ebp - 0x16a]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 487925 movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 487926 movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 487927 movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 487928 movsd dword ptr es:[edi], dword ptr [esi]
    { const reg32 r_ = rd32((ebp + 0x56u)) + 1u; wr32((ebp + 0x56u), r_); fl.arith(r_); }  // 487929 inc dword ptr [ebp + 0x56]
L_48792c:
    eax = rd32((ebp + 0x56u));  // 48792c mov eax, dword ptr [ebp + 0x56]
    { const reg32 r_ = eax << 4; eax = r_; fl.logic(r_); }  // 48792f shl eax, 4
    esi = (ebp - 0x1au);  // 487932 lea esi, [ebp - 0x1a]
    edi = (eax + ebp - 0x25au);  // 487935 lea edi, [eax + ebp - 0x25a]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 48793c movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 48793d movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 48793e movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 48793f movsd dword ptr es:[edi], dword ptr [esi]
    esi = (ebp - 0x5au);  // 487940 lea esi, [ebp - 0x5a]
    edi = (eax + ebp - 0x16au);  // 487943 lea edi, [eax + ebp - 0x16a]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 48794a movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 48794b movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 48794c movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 48794d movsd dword ptr es:[edi], dword ptr [esi]
    { const reg32 r_ = rd32((ebp + 0x56u)) + 1u; wr32((ebp + 0x56u), r_); fl.arith(r_); }  // 48794e inc dword ptr [ebp + 0x56]
L_487951:
    eax = rd32((ebp + 0x56u));  // 487951 mov eax, dword ptr [ebp + 0x56]
    { const reg32 r_ = eax << 4; eax = r_; fl.logic(r_); }  // 487954 shl eax, 4
    esi = (ebp - 0x2au);  // 487957 lea esi, [ebp - 0x2a]
    edi = (eax + ebp - 0x25au);  // 48795a lea edi, [eax + ebp - 0x25a]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 487961 movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 487962 movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 487963 movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 487964 movsd dword ptr es:[edi], dword ptr [esi]
    edi = (eax + ebp - 0x16au);  // 487965 lea edi, [eax + ebp - 0x16a]
    eax = rd32((ebp + 0x56u));  // 48796c mov eax, dword ptr [ebp + 0x56]
    esi = (ebp - 0x6au);  // 48796f lea esi, [ebp - 0x6a]
    { const reg32 r_ = eax + 1u; eax = r_; fl.arith(r_); }  // 487972 inc eax
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 487973 movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 487974 movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 487975 movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 487976 movsd dword ptr es:[edi], dword ptr [esi]
    wr32((ebp + 0x56u), eax);  // 487977 mov dword ptr [ebp + 0x56], eax
L_48797a:
    eax = rd32((ebp + 0x52u));  // 48797a mov eax, dword ptr [ebp + 0x52]
    { const reg32 r_ = eax << 4; eax = r_; fl.logic(r_); }  // 48797d shl eax, 4
    esi = (ebp - 0x3au);  // 487980 lea esi, [ebp - 0x3a]
    edi = (eax + ebp - 0x25au);  // 487983 lea edi, [eax + ebp - 0x25a]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 48798a movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 48798b movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 48798c movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 48798d movsd dword ptr es:[edi], dword ptr [esi]
    esi = (ebp - 0x7au);  // 48798e lea esi, [ebp - 0x7a]
    edi = (eax + ebp - 0x16au);  // 487991 lea edi, [eax + ebp - 0x16a]
L_487998:
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 487998 movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 487999 movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 48799a movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 48799b movsd dword ptr es:[edi], dword ptr [esi]
L_48799c:
    { const reg32 r_ = rd32((ebp + 0x4au)) + 1u; wr32((ebp + 0x4au), r_); fl.arith(r_); }  // 48799c inc dword ptr [ebp + 0x4a]
    goto L_48783b;  // 48799f jmp 0x48783b
L_4879a4:
    { const reg32 r_ = rd32((ebp + 0x52u)) + 1u; wr32((ebp + 0x52u), r_); fl.arith(r_); }  // 4879a4 inc dword ptr [ebp + 0x52]
    goto L_48782a;  // 4879a7 jmp 0x48782a
L_4879ac:
    edx = 0u;  // 4879ac xor edx, edx
    fl.logic(0u);  // 4879ac xor edx, edx
L_4879ae:
    fl.sub(edx, rd32((ebp + 0x56u)));  // 4879ae cmp edx, dword ptr [ebp + 0x56]
    if (fl.ge()) goto L_487a13;  // 4879b1 jge 0x487a13
    esi = rd32((ebp + 0x96u));  // 4879b3 mov esi, dword ptr [ebp + 0x96]
    edi = rd32((ebp + 0x92u));  // 4879b9 mov edi, dword ptr [ebp + 0x92]
    eax = edx;  // 4879bf mov eax, edx
    esp -= 4; wr32(esp, esi);  // 4879c1 push esi
    { const reg32 r_ = eax << 4; eax = r_; fl.logic(r_); }  // 4879c2 shl eax, 4
    esp -= 4; wr32(esp, edi);  // 4879c5 push edi
    ecx = rd32((eax + ebp - 0x15eu));  // 4879c6 mov ecx, dword ptr [eax + ebp - 0x15e]
    esp -= 4; wr32(esp, ecx);  // 4879cd push ecx
    ebx = rd32((eax + ebp - 0x162u));  // 4879ce mov ebx, dword ptr [eax + ebp - 0x162]
    esp -= 4; wr32(esp, ebx);  // 4879d5 push ebx
    esi = rd32((eax + ebp - 0x166u));  // 4879d6 mov esi, dword ptr [eax + ebp - 0x166]
    esp -= 4; wr32(esp, esi);  // 4879dd push esi
    edi = rd32((eax + ebp - 0x16au));  // 4879de mov edi, dword ptr [eax + ebp - 0x16a]
    esp -= 4; wr32(esp, edi);  // 4879e5 push edi
    ecx = rd32((eax + ebp - 0x24eu));  // 4879e6 mov ecx, dword ptr [eax + ebp - 0x24e]
    esp -= 4; wr32(esp, ecx);  // 4879ed push ecx
    ebx = rd32((eax + ebp - 0x252u));  // 4879ee mov ebx, dword ptr [eax + ebp - 0x252]
    esp -= 4; wr32(esp, ebx);  // 4879f5 push ebx
    esi = rd32((eax + ebp - 0x256u));  // 4879f6 mov esi, dword ptr [eax + ebp - 0x256]
    esp -= 4; wr32(esp, esi);  // 4879fd push esi
    edi = rd32((eax + ebp - 0x25au));  // 4879fe mov edi, dword ptr [eax + ebp - 0x25a]
    esp -= 4; wr32(esp, edi);  // 487a05 push edi
    cpu.ebp = ebp; if (!rawCall(app, cpu, 0x486df0, esp, 10, eax, edx, ebx, ecx)) return false;  // 487a06 call 0x486df0
    esp += 40u;  // 487a06 call 0x486df0
    eax = 0u;  // 487a0b xor eax, eax
    fl.logic(0u);  // 487a0b xor eax, eax
    { const reg32 r_ = edx + 1u; edx = r_; fl.arith(r_); }  // 487a0d inc edx
    wr32((ebp + 0x42u), eax);  // 487a0e mov dword ptr [ebp + 0x42], eax
    goto L_4879ae;  // 487a11 jmp 0x4879ae
L_487a13:
    eax = rd32((ebp + 0x42u));  // 487a13 mov eax, dword ptr [ebp + 0x42]
    esp = (ebp + 0x5au);  // 487a16 lea esp, [ebp + 0x5a]
    ebp = rd32(esp); esp += 4;  // 487a19 pop ebp
    edi = rd32(esp); esp += 4;  // 487a1a pop edi
    esi = rd32(esp); esp += 4;  // 487a1b pop esi
    ecx = rd32(esp); esp += 4;  // 487a1c pop ecx
    ebx = rd32(esp); esp += 4;  // 487a1d pop ebx
    result = eax; popped_ = 40u; return true;  // 487a1e ret 0x28
    return false;
}
// (popped = 40
bool decompSegmentsMet(App* app, CPU& cpu, const Entry& e, reg32& result, reg32& popped_)
{
    reg32 eax = e.eax, ebx = e.ebx, ecx = e.ecx, edx = e.edx, esi = e.esi, edi = e.edi, ebp = e.ebp, esp = e.esp;
    Flags fl;
    X87 x;
    const auto rd32 = [&](reg32 a) { return app->getMemory<reg32>(a); };
    const auto rd16 = [&](reg32 a) { return reg32(app->getMemory<uint16_t>(a)); };
    const auto rd8 = [&](reg32 a) { return reg32(app->getMemory<uint8_t>(a)); };
    const auto wr32 = [&](reg32 a, reg32 v) { app->getMemory<reg32>(a) = v; };
    const auto wr16 = [&](reg32 a, reg32 v) { app->getMemory<uint16_t>(a) = uint16_t(v); };
    const auto wr8 = [&](reg32 a, reg32 v) { app->getMemory<uint8_t>(a) = uint8_t(v); };
    const auto F = [&](reg32 a) { return double(app->getMemory<float>(a)); };
    const auto D = [&](reg32 a) { return app->getMemory<double>(a); };
    const auto SF = [&](reg32 a, double v) { app->getMemory<float>(a) = float(v); };
    const auto SD = [&](reg32 a, double v) { app->getMemory<double>(a) = v; };
    esp -= 4; wr32(esp, esi);  // 486340 push esi
    esp -= 4; wr32(esp, edi);  // 486341 push edi
    esp -= 4; wr32(esp, ebp);  // 486342 push ebp
    ebp = esp;  // 486343 mov ebp, esp
    esp -= 72u;  // 486345 sub esp, 0x48
    esi = eax;  // 486348 mov esi, eax
    edi = ebx;  // 48634a mov edi, ebx
    wr32((ebp - 0x18u), ecx);  // 48634c mov dword ptr [ebp - 0x18], ecx
    ebx = rd32((ebp + 0x10u));  // 48634f mov ebx, dword ptr [ebp + 0x10]
    ecx = rd32((ebp + 0x14u));  // 486352 mov ecx, dword ptr [ebp + 0x14]
    eax = 1u;  // 486355 mov eax, 1
    x.push(F(esi));  // 48635a fld dword ptr [esi]
    wr32((ebp - 0x2cu), eax);  // 48635c mov dword ptr [ebp - 0x2c], eax
    fl.fcmp(x.st(0), F(edi));  // 48635f fcomp dword ptr [edi]
    x.pop();  // 48635f fcomp dword ptr [edi]
    // fnstsw ax  // 486361 fnstsw ax
    // sahf  // 486363 sahf
    if (fl.be()) goto L_48636a;  // 486364 jbe 0x48636a
    eax = rd32((esi));  // 486366 mov eax, dword ptr [esi]
    goto L_48636c;  // 486368 jmp 0x48636c
L_48636a:
    eax = rd32((edi));  // 48636a mov eax, dword ptr [edi]
L_48636c:
    wr32((ebp - 0x28u), eax);  // 48636c mov dword ptr [ebp - 0x28], eax
    eax = rd32((ebp - 0x28u));  // 48636f mov eax, dword ptr [ebp - 0x28]
    x.push(F(esi + 8u));  // 486372 fld dword ptr [esi + 8]
    wr32((ebp - 0x14u), eax);  // 486375 mov dword ptr [ebp - 0x14], eax
    fl.fcmp(x.st(0), F(edi + 8u));  // 486378 fcomp dword ptr [edi + 8]
    x.pop();  // 486378 fcomp dword ptr [edi + 8]
    // fnstsw ax  // 48637b fnstsw ax
    // sahf  // 48637d sahf
    if (fl.ae()) goto L_486385;  // 48637e jae 0x486385
    eax = rd32((esi + 8u));  // 486380 mov eax, dword ptr [esi + 8]
    goto L_486388;  // 486383 jmp 0x486388
L_486385:
    eax = rd32((edi + 8u));  // 486385 mov eax, dword ptr [edi + 8]
L_486388:
    wr32((ebp - 0x24u), eax);  // 486388 mov dword ptr [ebp - 0x24], eax
    eax = rd32((ebp - 0x24u));  // 48638b mov eax, dword ptr [ebp - 0x24]
    x.push(F(esi + 4u));  // 48638e fld dword ptr [esi + 4]
    wr32((ebp - 0x38u), eax);  // 486391 mov dword ptr [ebp - 0x38], eax
    fl.fcmp(x.st(0), F(edi + 4u));  // 486394 fcomp dword ptr [edi + 4]
    x.pop();  // 486394 fcomp dword ptr [edi + 4]
    // fnstsw ax  // 486397 fnstsw ax
    // sahf  // 486399 sahf
    if (fl.be()) goto L_4863a1;  // 48639a jbe 0x4863a1
    eax = rd32((esi + 4u));  // 48639c mov eax, dword ptr [esi + 4]
    goto L_4863a4;  // 48639f jmp 0x4863a4
L_4863a1:
    eax = rd32((edi + 4u));  // 4863a1 mov eax, dword ptr [edi + 4]
L_4863a4:
    wr32((ebp - 0x20u), eax);  // 4863a4 mov dword ptr [ebp - 0x20], eax
    eax = rd32((ebp - 0x20u));  // 4863a7 mov eax, dword ptr [ebp - 0x20]
    x.push(F(esi + 0xcu));  // 4863aa fld dword ptr [esi + 0xc]
    wr32((ebp - 0xcu), eax);  // 4863ad mov dword ptr [ebp - 0xc], eax
    fl.fcmp(x.st(0), F(edi + 0xcu));  // 4863b0 fcomp dword ptr [edi + 0xc]
    x.pop();  // 4863b0 fcomp dword ptr [edi + 0xc]
    // fnstsw ax  // 4863b3 fnstsw ax
    // sahf  // 4863b5 sahf
    if (fl.ae()) goto L_4863bd;  // 4863b6 jae 0x4863bd
    eax = rd32((esi + 0xcu));  // 4863b8 mov eax, dword ptr [esi + 0xc]
    goto L_4863c0;  // 4863bb jmp 0x4863c0
L_4863bd:
    eax = rd32((edi + 0xcu));  // 4863bd mov eax, dword ptr [edi + 0xc]
L_4863c0:
    wr32((ebp - 0x1cu), eax);  // 4863c0 mov dword ptr [ebp - 0x1c], eax
    eax = rd32((ebp - 0x1cu));  // 4863c3 mov eax, dword ptr [ebp - 0x1c]
    x.push(F(ebp - 0x14u));  // 4863c6 fld dword ptr [ebp - 0x14]
    wr32((ebp - 0x10u), eax);  // 4863c9 mov dword ptr [ebp - 0x10], eax
    fl.fcmp(x.st(0), F(esi));  // 4863cc fcomp dword ptr [esi]
    x.pop();  // 4863cc fcomp dword ptr [esi]
    // fnstsw ax  // 4863ce fnstsw ax
    // sahf  // 4863d0 sahf
    if (fl.e()) goto L_4863da;  // 4863d1 je 0x4863da
    eax = 1u;  // 4863d3 mov eax, 1
    goto L_4863dc;  // 4863d8 jmp 0x4863dc
L_4863da:
    eax = 0u;  // 4863da xor eax, eax
    fl.logic(0u);  // 4863da xor eax, eax
L_4863dc:
    x.push(F(ebp - 0x38u));  // 4863dc fld dword ptr [ebp - 0x38]
    wr32((ebp - 0x30u), eax);  // 4863df mov dword ptr [ebp - 0x30], eax
    fl.fcmp(x.st(0), F(esi + 8u));  // 4863e2 fcomp dword ptr [esi + 8]
    x.pop();  // 4863e2 fcomp dword ptr [esi + 8]
    // fnstsw ax  // 4863e5 fnstsw ax
    // sahf  // 4863e7 sahf
    if (fl.e()) goto L_4863f1;  // 4863e8 je 0x4863f1
    eax = 1u;  // 4863ea mov eax, 1
    goto L_4863f3;  // 4863ef jmp 0x4863f3
L_4863f1:
    eax = 0u;  // 4863f1 xor eax, eax
    fl.logic(0u);  // 4863f1 xor eax, eax
L_4863f3:
    x.push(F(ebp - 0xcu));  // 4863f3 fld dword ptr [ebp - 0xc]
    wr32((ebp - 0x3cu), eax);  // 4863f6 mov dword ptr [ebp - 0x3c], eax
    fl.fcmp(x.st(0), F(esi + 4u));  // 4863f9 fcomp dword ptr [esi + 4]
    x.pop();  // 4863f9 fcomp dword ptr [esi + 4]
    // fnstsw ax  // 4863fc fnstsw ax
    // sahf  // 4863fe sahf
    if (fl.e()) goto L_486408;  // 4863ff je 0x486408
    eax = 1u;  // 486401 mov eax, 1
    goto L_48640a;  // 486406 jmp 0x48640a
L_486408:
    eax = 0u;  // 486408 xor eax, eax
    fl.logic(0u);  // 486408 xor eax, eax
L_48640a:
    x.push(F(ebp - 0x10u));  // 48640a fld dword ptr [ebp - 0x10]
    wr32((ebp - 0x34u), eax);  // 48640d mov dword ptr [ebp - 0x34], eax
    fl.fcmp(x.st(0), F(esi + 0xcu));  // 486410 fcomp dword ptr [esi + 0xc]
    x.pop();  // 486410 fcomp dword ptr [esi + 0xc]
    // fnstsw ax  // 486413 fnstsw ax
    // sahf  // 486415 sahf
    if (fl.e()) goto L_48641f;  // 486416 je 0x48641f
    eax = 1u;  // 486418 mov eax, 1
    goto L_486421;  // 48641d jmp 0x486421
L_48641f:
    eax = 0u;  // 48641f xor eax, eax
    fl.logic(0u);  // 48641f xor eax, eax
L_486421:
    wr32((ebp - 0x48u), eax);  // 486421 mov dword ptr [ebp - 0x48], eax
    eax = (eax & 0xffffff00u) | ((rd8((ebp - 0x30u))) & 0xffu);  // 486424 mov al, byte ptr [ebp - 0x30]
    { const reg32 r_ = (eax & 0xffu) << 3; eax = (eax & 0xffffff00u) | ((r_) & 0xffu); fl.logic(r_); }  // 486427 shl al, 3
    wr8((ebp - 4u), (eax & 0xffu));  // 48642a mov byte ptr [ebp - 4], al
    eax = (eax & 0xffffff00u) | ((rd8((ebp - 0x3cu))) & 0xffu);  // 48642d mov al, byte ptr [ebp - 0x3c]
    { const reg32 r_ = (eax & 0xffu) << 2; eax = (eax & 0xffffff00u) | ((r_) & 0xffu); fl.logic(r_); }  // 486430 shl al, 2
    eax = (eax & 0xffff00ffu) | (((rd8((ebp - 4u))) & 0xffu) << 8);  // 486433 mov ah, byte ptr [ebp - 4]
    { const reg32 r_ = ((eax >> 8) & 0xffu) | (eax & 0xffu); eax = (eax & 0xffff00ffu) | (((r_) & 0xffu) << 8); fl.logic(r_); }  // 486436 or ah, al
    eax = (eax & 0xffffff00u) | ((rd8((ebp - 0x34u))) & 0xffu);  // 486438 mov al, byte ptr [ebp - 0x34]
    { const reg32 a_ = (eax & 0xffu), b_ = (eax & 0xffu); eax = (eax & 0xffffff00u) | ((a_ + b_) & 0xffu); fl.add(a_, b_); }  // 48643b add al, al
    { const reg32 r_ = (eax & 0xffu) | ((eax >> 8) & 0xffu); eax = (eax & 0xffffff00u) | ((r_) & 0xffu); fl.logic(r_); }  // 48643d or al, ah
    eax = (eax & 0xffff00ffu) | (((rd8((ebp - 0x48u))) & 0xffu) << 8);  // 48643f mov ah, byte ptr [ebp - 0x48]
    { const reg32 r_ = ((eax >> 8) & 0xffu) | (eax & 0xffu); eax = (eax & 0xffff00ffu) | (((r_) & 0xffu) << 8); fl.logic(r_); }  // 486442 or ah, al
    x.push(F(ebp - 0x14u));  // 486444 fld dword ptr [ebp - 0x14]
    wr8((ebp - 8u), ((eax >> 8) & 0xffu));  // 486447 mov byte ptr [ebp - 8], ah
    fl.fcmp(x.st(0), F(ebp - 0x38u));  // 48644a fcomp dword ptr [ebp - 0x38]
    x.pop();  // 48644a fcomp dword ptr [ebp - 0x38]
    // fnstsw ax  // 48644d fnstsw ax
    // sahf  // 48644f sahf
    if (fl.ae()) goto L_48645d;  // 486450 jae 0x48645d
    x.push(F(ebp - 0xcu));  // 486452 fld dword ptr [ebp - 0xc]
    fl.fcmp(x.st(0), F(ebp - 0x10u));  // 486455 fcomp dword ptr [ebp - 0x10]
    x.pop();  // 486455 fcomp dword ptr [ebp - 0x10]
    // fnstsw ax  // 486458 fnstsw ax
    // sahf  // 48645a sahf
    if (fl.b()) goto L_486467;  // 48645b jb 0x486467
L_48645d:
    edx = 0u;  // 48645d xor edx, edx
    fl.logic(0u);  // 48645d xor edx, edx
    wr32((ebp - 0x2cu), edx);  // 48645f mov dword ptr [ebp - 0x2c], edx
    goto L_486dda;  // 486462 jmp 0x486dda
L_486467:
    fl.sub(rd8((ebp - 8u)), 15u);  // 486467 cmp byte ptr [ebp - 8], 0xf
    if (fl.a()) goto L_486dda;  // 48646b ja 0x486dda
    eax = 0u;  // 486471 xor eax, eax
    fl.logic(0u);  // 486471 xor eax, eax
    eax = (eax & 0xffffff00u) | ((rd8((ebp - 8u))) & 0xffu);  // 486473 mov al, byte ptr [ebp - 8]
    switch (eax) {  // 486476 jmp dword ptr [eax*4 + 0x4862f8]
    case 0: goto L_48647d;  // 486476 jmp dword ptr [eax*4 + 0x4862f8]
    case 1: goto L_486491;  // 486476 jmp dword ptr [eax*4 + 0x4862f8]
    case 2: goto L_4864e7;  // 486476 jmp dword ptr [eax*4 + 0x4862f8]
    case 3: goto L_48653d;  // 486476 jmp dword ptr [eax*4 + 0x4862f8]
    case 4: goto L_4865d3;  // 486476 jmp dword ptr [eax*4 + 0x4862f8]
    case 5: goto L_486627;  // 486476 jmp dword ptr [eax*4 + 0x4862f8]
    case 6: goto L_4866bf;  // 486476 jmp dword ptr [eax*4 + 0x4862f8]
    case 7: goto L_486757;  // 486476 jmp dword ptr [eax*4 + 0x4862f8]
    case 8: goto L_486843;  // 486476 jmp dword ptr [eax*4 + 0x4862f8]
    case 9: goto L_486895;  // 486476 jmp dword ptr [eax*4 + 0x4862f8]
    case 10: goto L_48692a;  // 486476 jmp dword ptr [eax*4 + 0x4862f8]
    case 11: goto L_4869bf;  // 486476 jmp dword ptr [eax*4 + 0x4862f8]
    case 12: goto L_486a94;  // 486476 jmp dword ptr [eax*4 + 0x4862f8]
    case 13: goto L_486b26;  // 486476 jmp dword ptr [eax*4 + 0x4862f8]
    case 14: goto L_486bf5;  // 486476 jmp dword ptr [eax*4 + 0x4862f8]
    case 15: goto L_486cd6;  // 486476 jmp dword ptr [eax*4 + 0x4862f8]
    default: break; }  // 486476 jmp dword ptr [eax*4 + 0x4862f8]
    return false;  // (no entry)  // 486476 jmp dword ptr [eax*4 + 0x4862f8]
L_48647d:
    eax = rd32((ebp - 0x18u));  // 48647d mov eax, dword ptr [ebp - 0x18]
    wr32((eax), 0u);  // 486480 mov dword ptr [eax], 0
    eax = rd32((ebp - 0x2cu));  // 486486 mov eax, dword ptr [ebp - 0x2c]
    esp = ebp;  // 486489 mov esp, ebp
    ebp = rd32(esp); esp += 4;  // 48648b pop ebp
    edi = rd32(esp); esp += 4;  // 48648c pop edi
    esi = rd32(esp); esp += 4;  // 48648d pop esi
    result = eax; popped_ = 8u; return true;  // 48648e ret 8
L_486491:
    eax = rd32((ebp - 0x18u));  // 486491 mov eax, dword ptr [ebp - 0x18]
    wr32((eax), 1u);  // 486494 mov dword ptr [eax], 1
    x.push(F(esi));  // 48649a fld dword ptr [esi]
    eax = rd32((ebp - 0x10u));  // 48649c mov eax, dword ptr [ebp - 0x10]
    SF((ebx), x.st(0));  // 48649f fstp dword ptr [ebx]
    x.pop();  // 48649f fstp dword ptr [ebx]
    edi = rd32((esi + 8u));  // 4864a1 mov edi, dword ptr [esi + 8]
    wr32((ebx + 8u), edi);  // 4864a4 mov dword ptr [ebx + 8], edi
    x.push(F(esi + 0xcu));  // 4864a7 fld dword ptr [esi + 0xc]
    wr32((ebx + 4u), eax);  // 4864aa mov dword ptr [ebx + 4], eax
    SF((ebx + 0xcu), x.st(0));  // 4864ad fstp dword ptr [ebx + 0xc]
    x.pop();  // 4864ad fstp dword ptr [ebx + 0xc]
    eax = rd32((edx));  // 4864b0 mov eax, dword ptr [edx]
    wr32((ecx), eax);  // 4864b2 mov dword ptr [ecx], eax
    eax = rd32((edx + 8u));  // 4864b4 mov eax, dword ptr [edx + 8]
    wr32((ecx + 8u), eax);  // 4864b7 mov dword ptr [ecx + 8], eax
    eax = rd32((edx + 0xcu));  // 4864ba mov eax, dword ptr [edx + 0xc]
    wr32((ecx + 0xcu), eax);  // 4864bd mov dword ptr [ecx + 0xc], eax
    x.push(F(edx + 0xcu));  // 4864c0 fld dword ptr [edx + 0xc]
    x.st(0) = (x.st(0)) - (F(edx + 4u));  // 4864c3 fsub dword ptr [edx + 4]
    x.push(F(ebp - 0x10u));  // 4864c6 fld dword ptr [ebp - 0x10]
    x.st(0) = (x.st(0)) - (F(esi + 4u));  // 4864c9 fsub dword ptr [esi + 4]
    x.st(1) = (x.st(1)) * (x.st(0));  // 4864cc fmulp st(1)
    x.pop();  // 4864cc fmulp st(1)
    x.push(F(esi + 0xcu));  // 4864ce fld dword ptr [esi + 0xc]
    x.st(0) = (x.st(0)) - (F(esi + 4u));  // 4864d1 fsub dword ptr [esi + 4]
    x.st(1) = (x.st(1)) / (x.st(0));  // 4864d4 fdivp st(1)
    x.pop();  // 4864d4 fdivp st(1)
    x.st(0) = (x.st(0)) + (F(edx + 4u));  // 4864d6 fadd dword ptr [edx + 4]
    SF((ecx + 4u), x.st(0));  // 4864d9 fstp dword ptr [ecx + 4]
    x.pop();  // 4864d9 fstp dword ptr [ecx + 4]
    eax = rd32((ebp - 0x2cu));  // 4864dc mov eax, dword ptr [ebp - 0x2c]
    esp = ebp;  // 4864df mov esp, ebp
    ebp = rd32(esp); esp += 4;  // 4864e1 pop ebp
    edi = rd32(esp); esp += 4;  // 4864e2 pop edi
    esi = rd32(esp); esp += 4;  // 4864e3 pop esi
    result = eax; popped_ = 8u; return true;  // 4864e4 ret 8
L_4864e7:
    eax = rd32((ebp - 0x18u));  // 4864e7 mov eax, dword ptr [ebp - 0x18]
    wr32((eax), 1u);  // 4864ea mov dword ptr [eax], 1
    x.push(F(esi));  // 4864f0 fld dword ptr [esi]
    eax = rd32((ebp - 0xcu));  // 4864f2 mov eax, dword ptr [ebp - 0xc]
    SF((ebx), x.st(0));  // 4864f5 fstp dword ptr [ebx]
    x.pop();  // 4864f5 fstp dword ptr [ebx]
    edi = rd32((esi + 8u));  // 4864f7 mov edi, dword ptr [esi + 8]
    wr32((ebx + 8u), edi);  // 4864fa mov dword ptr [ebx + 8], edi
    x.push(F(esi + 4u));  // 4864fd fld dword ptr [esi + 4]
    wr32((ebx + 0xcu), eax);  // 486500 mov dword ptr [ebx + 0xc], eax
    SF((ebx + 4u), x.st(0));  // 486503 fstp dword ptr [ebx + 4]
    x.pop();  // 486503 fstp dword ptr [ebx + 4]
    eax = rd32((edx));  // 486506 mov eax, dword ptr [edx]
    wr32((ecx), eax);  // 486508 mov dword ptr [ecx], eax
    eax = rd32((edx + 8u));  // 48650a mov eax, dword ptr [edx + 8]
    wr32((ecx + 8u), eax);  // 48650d mov dword ptr [ecx + 8], eax
    eax = rd32((edx + 4u));  // 486510 mov eax, dword ptr [edx + 4]
    wr32((ecx + 4u), eax);  // 486513 mov dword ptr [ecx + 4], eax
    x.push(F(edx + 0xcu));  // 486516 fld dword ptr [edx + 0xc]
    x.st(0) = (x.st(0)) - (F(edx + 4u));  // 486519 fsub dword ptr [edx + 4]
    x.push(F(ebp - 0xcu));  // 48651c fld dword ptr [ebp - 0xc]
    x.st(0) = (x.st(0)) - (F(esi + 4u));  // 48651f fsub dword ptr [esi + 4]
    x.st(1) = (x.st(1)) * (x.st(0));  // 486522 fmulp st(1)
    x.pop();  // 486522 fmulp st(1)
    x.push(F(esi + 0xcu));  // 486524 fld dword ptr [esi + 0xc]
    x.st(0) = (x.st(0)) - (F(esi + 4u));  // 486527 fsub dword ptr [esi + 4]
    x.st(1) = (x.st(1)) / (x.st(0));  // 48652a fdivp st(1)
    x.pop();  // 48652a fdivp st(1)
    x.st(0) = (x.st(0)) + (F(edx + 4u));  // 48652c fadd dword ptr [edx + 4]
    SF((ecx + 0xcu), x.st(0));  // 48652f fstp dword ptr [ecx + 0xc]
    x.pop();  // 48652f fstp dword ptr [ecx + 0xc]
    eax = rd32((ebp - 0x2cu));  // 486532 mov eax, dword ptr [ebp - 0x2c]
    esp = ebp;  // 486535 mov esp, ebp
    ebp = rd32(esp); esp += 4;  // 486537 pop ebp
    edi = rd32(esp); esp += 4;  // 486538 pop edi
    esi = rd32(esp); esp += 4;  // 486539 pop esi
    result = eax; popped_ = 8u; return true;  // 48653a ret 8
L_48653d:
    eax = rd32((ebp - 0x18u));  // 48653d mov eax, dword ptr [ebp - 0x18]
    wr32((eax), 2u);  // 486540 mov dword ptr [eax], 2
    x.push(F(esi));  // 486546 fld dword ptr [esi]
    x.push(F(edx + 0xcu));  // 486548 fld dword ptr [edx + 0xc]
    x.st(0) = (x.st(0)) - (F(edx + 4u));  // 48654b fsub dword ptr [edx + 4]
    eax = rd32((ebp - 0xcu));  // 48654e mov eax, dword ptr [ebp - 0xc]
    x.push(F(esi + 0xcu));  // 486551 fld dword ptr [esi + 0xc]
    x.st(0) = (x.st(0)) - (F(esi + 4u));  // 486554 fsub dword ptr [esi + 4]
    x.fxch(2);  // 486557 fxch st(2)
    SF((ebx), x.st(0));  // 486559 fstp dword ptr [ebx]
    x.pop();  // 486559 fstp dword ptr [ebx]
    edi = rd32((esi + 8u));  // 48655b mov edi, dword ptr [esi + 8]
    wr32((ebx + 8u), edi);  // 48655e mov dword ptr [ebx + 8], edi
    x.push(F(esi + 4u));  // 486561 fld dword ptr [esi + 4]
    wr32((ebx + 0xcu), eax);  // 486564 mov dword ptr [ebx + 0xc], eax
    SF((ebx + 4u), x.st(0));  // 486567 fstp dword ptr [ebx + 4]
    x.pop();  // 486567 fstp dword ptr [ebx + 4]
    x.push(F(esi));  // 48656a fld dword ptr [esi]
    eax = rd32((ebp - 0x10u));  // 48656c mov eax, dword ptr [ebp - 0x10]
    SF((ebx + 0x10u), x.st(0));  // 48656f fstp dword ptr [ebx + 0x10]
    x.pop();  // 48656f fstp dword ptr [ebx + 0x10]
    edi = rd32((esi + 8u));  // 486572 mov edi, dword ptr [esi + 8]
    wr32((ebx + 0x18u), edi);  // 486575 mov dword ptr [ebx + 0x18], edi
    x.push(F(esi + 0xcu));  // 486578 fld dword ptr [esi + 0xc]
    wr32((ebx + 0x14u), eax);  // 48657b mov dword ptr [ebx + 0x14], eax
    x.push(F(ebp - 0xcu));  // 48657e fld dword ptr [ebp - 0xc]
    x.fxch(1);  // 486581 fxch st(1)
    SF((ebx + 0x1cu), x.st(0));  // 486583 fstp dword ptr [ebx + 0x1c]
    x.pop();  // 486583 fstp dword ptr [ebx + 0x1c]
    eax = rd32((edx));  // 486586 mov eax, dword ptr [edx]
    wr32((ecx), eax);  // 486588 mov dword ptr [ecx], eax
    eax = rd32((edx + 8u));  // 48658a mov eax, dword ptr [edx + 8]
    wr32((ecx + 8u), eax);  // 48658d mov dword ptr [ecx + 8], eax
    eax = rd32((edx + 4u));  // 486590 mov eax, dword ptr [edx + 4]
    wr32((ecx + 4u), eax);  // 486593 mov dword ptr [ecx + 4], eax
    x.st(0) = (x.st(0)) - (F(esi + 4u));  // 486596 fsub dword ptr [esi + 4]
    x.fxch(2);  // 486599 fxch st(2)
    x.st(1) = (x.st(1)) / (x.st(0));  // 48659b fdivp st(1)
    x.pop();  // 48659b fdivp st(1)
    x.fxch(1);  // 48659d fxch st(1)
    x.st(0) = (x.st(0)) * (x.st(1));  // 48659f fmul st(1)
    x.st(0) = (x.st(0)) + (F(edx + 4u));  // 4865a1 fadd dword ptr [edx + 4]
    x.push(F(ebp - 0x10u));  // 4865a4 fld dword ptr [ebp - 0x10]
    x.fxch(1);  // 4865a7 fxch st(1)
    SF((ecx + 0xcu), x.st(0));  // 4865a9 fstp dword ptr [ecx + 0xc]
    x.pop();  // 4865a9 fstp dword ptr [ecx + 0xc]
    eax = rd32((edx));  // 4865ac mov eax, dword ptr [edx]
    wr32((ecx + 0x10u), eax);  // 4865ae mov dword ptr [ecx + 0x10], eax
    eax = rd32((edx + 8u));  // 4865b1 mov eax, dword ptr [edx + 8]
    wr32((ecx + 0x18u), eax);  // 4865b4 mov dword ptr [ecx + 0x18], eax
    eax = rd32((edx + 0xcu));  // 4865b7 mov eax, dword ptr [edx + 0xc]
    wr32((ecx + 0x1cu), eax);  // 4865ba mov dword ptr [ecx + 0x1c], eax
    x.st(0) = (x.st(0)) - (F(esi + 4u));  // 4865bd fsub dword ptr [esi + 4]
    x.st(1) = (x.st(1)) * (x.st(0));  // 4865c0 fmulp st(1)
    x.pop();  // 4865c0 fmulp st(1)
    x.st(0) = (x.st(0)) + (F(edx + 4u));  // 4865c2 fadd dword ptr [edx + 4]
    SF((ecx + 0x14u), x.st(0));  // 4865c5 fstp dword ptr [ecx + 0x14]
    x.pop();  // 4865c5 fstp dword ptr [ecx + 0x14]
    eax = rd32((ebp - 0x2cu));  // 4865c8 mov eax, dword ptr [ebp - 0x2c]
    esp = ebp;  // 4865cb mov esp, ebp
    ebp = rd32(esp); esp += 4;  // 4865cd pop ebp
    edi = rd32(esp); esp += 4;  // 4865ce pop edi
    esi = rd32(esp); esp += 4;  // 4865cf pop esi
    result = eax; popped_ = 8u; return true;  // 4865d0 ret 8
L_4865d3:
    eax = rd32((ebp - 0x18u));  // 4865d3 mov eax, dword ptr [ebp - 0x18]
    wr32((eax), 1u);  // 4865d6 mov dword ptr [eax], 1
    x.push(F(esi + 8u));  // 4865dc fld dword ptr [esi + 8]
    eax = rd32((ebp - 0x38u));  // 4865df mov eax, dword ptr [ebp - 0x38]
    SF((ebx + 8u), x.st(0));  // 4865e2 fstp dword ptr [ebx + 8]
    x.pop();  // 4865e2 fstp dword ptr [ebx + 8]
    edi = rd32((esi + 4u));  // 4865e5 mov edi, dword ptr [esi + 4]
    wr32((ebx + 4u), edi);  // 4865e8 mov dword ptr [ebx + 4], edi
    x.push(F(esi + 0xcu));  // 4865eb fld dword ptr [esi + 0xc]
    wr32((ebx), eax);  // 4865ee mov dword ptr [ebx], eax
    SF((ebx + 0xcu), x.st(0));  // 4865f0 fstp dword ptr [ebx + 0xc]
    x.pop();  // 4865f0 fstp dword ptr [ebx + 0xc]
    eax = rd32((edx + 8u));  // 4865f3 mov eax, dword ptr [edx + 8]
    wr32((ecx + 8u), eax);  // 4865f6 mov dword ptr [ecx + 8], eax
    eax = rd32((edx + 4u));  // 4865f9 mov eax, dword ptr [edx + 4]
    wr32((ecx + 4u), eax);  // 4865fc mov dword ptr [ecx + 4], eax
    eax = rd32((edx + 0xcu));  // 4865ff mov eax, dword ptr [edx + 0xc]
    wr32((ecx + 0xcu), eax);  // 486602 mov dword ptr [ecx + 0xc], eax
    x.push(F(edx + 8u));  // 486605 fld dword ptr [edx + 8]
    x.st(0) = (x.st(0)) - (F(edx));  // 486608 fsub dword ptr [edx]
    x.push(F(ebp - 0x38u));  // 48660a fld dword ptr [ebp - 0x38]
    x.st(0) = (x.st(0)) - (F(esi));  // 48660d fsub dword ptr [esi]
    x.st(1) = (x.st(1)) * (x.st(0));  // 48660f fmulp st(1)
    x.pop();  // 48660f fmulp st(1)
    x.push(F(esi + 8u));  // 486611 fld dword ptr [esi + 8]
    x.st(0) = (x.st(0)) - (F(esi));  // 486614 fsub dword ptr [esi]
    x.st(1) = (x.st(1)) / (x.st(0));  // 486616 fdivp st(1)
    x.pop();  // 486616 fdivp st(1)
    x.st(0) = (x.st(0)) + (F(edx));  // 486618 fadd dword ptr [edx]
    SF((ecx), x.st(0));  // 48661a fstp dword ptr [ecx]
    x.pop();  // 48661a fstp dword ptr [ecx]
    eax = rd32((ebp - 0x2cu));  // 48661c mov eax, dword ptr [ebp - 0x2c]
    esp = ebp;  // 48661f mov esp, ebp
    ebp = rd32(esp); esp += 4;  // 486621 pop ebp
    edi = rd32(esp); esp += 4;  // 486622 pop edi
    esi = rd32(esp); esp += 4;  // 486623 pop esi
    result = eax; popped_ = 8u; return true;  // 486624 ret 8
L_486627:
    eax = rd32((ebp - 0x18u));  // 486627 mov eax, dword ptr [ebp - 0x18]
    wr32((eax), 2u);  // 48662a mov dword ptr [eax], 2
    x.push(F(esi));  // 486630 fld dword ptr [esi]
    eax = rd32((ebp - 0x38u));  // 486632 mov eax, dword ptr [ebp - 0x38]
    SF((ebx), x.st(0));  // 486635 fstp dword ptr [ebx]
    x.pop();  // 486635 fstp dword ptr [ebx]
    x.push(F(esi + 0xcu));  // 486637 fld dword ptr [esi + 0xc]
    wr32((ebx + 8u), eax);  // 48663a mov dword ptr [ebx + 8], eax
    eax = rd32((ebp - 0x10u));  // 48663d mov eax, dword ptr [ebp - 0x10]
    wr32((ebx + 4u), eax);  // 486640 mov dword ptr [ebx + 4], eax
    SF((ebx + 0xcu), x.st(0));  // 486643 fstp dword ptr [ebx + 0xc]
    x.pop();  // 486643 fstp dword ptr [ebx + 0xc]
    eax = rd32((edx));  // 486646 mov eax, dword ptr [edx]
    wr32((ecx), eax);  // 486648 mov dword ptr [ecx], eax
    eax = rd32((edx + 0xcu));  // 48664a mov eax, dword ptr [edx + 0xc]
    wr32((ecx + 0xcu), eax);  // 48664d mov dword ptr [ecx + 0xc], eax
    x.push(F(edx + 8u));  // 486650 fld dword ptr [edx + 8]
    x.st(0) = (x.st(0)) - (F(edx));  // 486653 fsub dword ptr [edx]
    x.push(F(ebp - 0x38u));  // 486655 fld dword ptr [ebp - 0x38]
    x.st(0) = (x.st(0)) - (F(esi));  // 486658 fsub dword ptr [esi]
    x.st(1) = (x.st(1)) * (x.st(0));  // 48665a fmulp st(1)
    x.pop();  // 48665a fmulp st(1)
    x.push(F(esi + 8u));  // 48665c fld dword ptr [esi + 8]
    x.st(0) = (x.st(0)) - (F(esi));  // 48665f fsub dword ptr [esi]
    x.st(1) = (x.st(1)) / (x.st(0));  // 486661 fdivp st(1)
    x.pop();  // 486661 fdivp st(1)
    x.st(0) = (x.st(0)) + (F(edx));  // 486663 fadd dword ptr [edx]
    SF((ecx + 8u), x.st(0));  // 486665 fstp dword ptr [ecx + 8]
    x.pop();  // 486665 fstp dword ptr [ecx + 8]
    x.push(F(edx + 0xcu));  // 486668 fld dword ptr [edx + 0xc]
    x.st(0) = (x.st(0)) - (F(edx + 4u));  // 48666b fsub dword ptr [edx + 4]
    x.push(F(ebp - 0x10u));  // 48666e fld dword ptr [ebp - 0x10]
    x.st(0) = (x.st(0)) - (F(esi + 4u));  // 486671 fsub dword ptr [esi + 4]
    x.st(1) = (x.st(1)) * (x.st(0));  // 486674 fmulp st(1)
    x.pop();  // 486674 fmulp st(1)
    x.push(F(esi + 0xcu));  // 486676 fld dword ptr [esi + 0xc]
    x.st(0) = (x.st(0)) - (F(esi + 4u));  // 486679 fsub dword ptr [esi + 4]
    x.st(1) = (x.st(1)) / (x.st(0));  // 48667c fdivp st(1)
    x.pop();  // 48667c fdivp st(1)
    x.st(0) = (x.st(0)) + (F(edx + 4u));  // 48667e fadd dword ptr [edx + 4]
    eax = rd32((ebp - 0x38u));  // 486681 mov eax, dword ptr [ebp - 0x38]
    SF((ecx + 4u), x.st(0));  // 486684 fstp dword ptr [ecx + 4]
    x.pop();  // 486684 fstp dword ptr [ecx + 4]
    edi = rd32((esi + 8u));  // 486687 mov edi, dword ptr [esi + 8]
    wr32((ebx + 0x18u), edi);  // 48668a mov dword ptr [ebx + 0x18], edi
    edi = rd32((esi + 4u));  // 48668d mov edi, dword ptr [esi + 4]
    wr32((ebx + 0x14u), edi);  // 486690 mov dword ptr [ebx + 0x14], edi
    x.push(F(esi + 0xcu));  // 486693 fld dword ptr [esi + 0xc]
    wr32((ebx + 0x10u), eax);  // 486696 mov dword ptr [ebx + 0x10], eax
    SF((ebx + 0x1cu), x.st(0));  // 486699 fstp dword ptr [ebx + 0x1c]
    x.pop();  // 486699 fstp dword ptr [ebx + 0x1c]
    x.push(F(ecx + 8u));  // 48669c fld dword ptr [ecx + 8]
    eax = rd32((edx + 8u));  // 48669f mov eax, dword ptr [edx + 8]
    wr32((ecx + 0x18u), eax);  // 4866a2 mov dword ptr [ecx + 0x18], eax
    eax = rd32((edx + 4u));  // 4866a5 mov eax, dword ptr [edx + 4]
    wr32((ecx + 0x14u), eax);  // 4866a8 mov dword ptr [ecx + 0x14], eax
    eax = rd32((edx + 0xcu));  // 4866ab mov eax, dword ptr [edx + 0xc]
    wr32((ecx + 0x1cu), eax);  // 4866ae mov dword ptr [ecx + 0x1c], eax
    SF((ecx + 0x10u), x.st(0));  // 4866b1 fstp dword ptr [ecx + 0x10]
    x.pop();  // 4866b1 fstp dword ptr [ecx + 0x10]
    eax = rd32((ebp - 0x2cu));  // 4866b4 mov eax, dword ptr [ebp - 0x2c]
    esp = ebp;  // 4866b7 mov esp, ebp
    ebp = rd32(esp); esp += 4;  // 4866b9 pop ebp
    edi = rd32(esp); esp += 4;  // 4866ba pop edi
    esi = rd32(esp); esp += 4;  // 4866bb pop esi
    result = eax; popped_ = 8u; return true;  // 4866bc ret 8
L_4866bf:
    eax = rd32((ebp - 0x18u));  // 4866bf mov eax, dword ptr [ebp - 0x18]
    wr32((eax), 2u);  // 4866c2 mov dword ptr [eax], 2
    x.push(F(esi));  // 4866c8 fld dword ptr [esi]
    eax = rd32((ebp - 0x38u));  // 4866ca mov eax, dword ptr [ebp - 0x38]
    SF((ebx), x.st(0));  // 4866cd fstp dword ptr [ebx]
    x.pop();  // 4866cd fstp dword ptr [ebx]
    x.push(F(esi + 4u));  // 4866cf fld dword ptr [esi + 4]
    wr32((ebx + 8u), eax);  // 4866d2 mov dword ptr [ebx + 8], eax
    eax = rd32((ebp - 0xcu));  // 4866d5 mov eax, dword ptr [ebp - 0xc]
    wr32((ebx + 0xcu), eax);  // 4866d8 mov dword ptr [ebx + 0xc], eax
    SF((ebx + 4u), x.st(0));  // 4866db fstp dword ptr [ebx + 4]
    x.pop();  // 4866db fstp dword ptr [ebx + 4]
    eax = rd32((edx));  // 4866de mov eax, dword ptr [edx]
    wr32((ecx), eax);  // 4866e0 mov dword ptr [ecx], eax
    eax = rd32((edx + 4u));  // 4866e2 mov eax, dword ptr [edx + 4]
    wr32((ecx + 4u), eax);  // 4866e5 mov dword ptr [ecx + 4], eax
    x.push(F(edx + 8u));  // 4866e8 fld dword ptr [edx + 8]
    x.st(0) = (x.st(0)) - (F(edx));  // 4866eb fsub dword ptr [edx]
    x.push(F(ebp - 0x38u));  // 4866ed fld dword ptr [ebp - 0x38]
    x.st(0) = (x.st(0)) - (F(esi));  // 4866f0 fsub dword ptr [esi]
    x.st(1) = (x.st(1)) * (x.st(0));  // 4866f2 fmulp st(1)
    x.pop();  // 4866f2 fmulp st(1)
    x.push(F(esi + 8u));  // 4866f4 fld dword ptr [esi + 8]
    x.st(0) = (x.st(0)) - (F(esi));  // 4866f7 fsub dword ptr [esi]
    x.st(1) = (x.st(1)) / (x.st(0));  // 4866f9 fdivp st(1)
    x.pop();  // 4866f9 fdivp st(1)
    x.st(0) = (x.st(0)) + (F(edx));  // 4866fb fadd dword ptr [edx]
    SF((ecx + 8u), x.st(0));  // 4866fd fstp dword ptr [ecx + 8]
    x.pop();  // 4866fd fstp dword ptr [ecx + 8]
    x.push(F(edx + 0xcu));  // 486700 fld dword ptr [edx + 0xc]
    x.st(0) = (x.st(0)) - (F(edx + 4u));  // 486703 fsub dword ptr [edx + 4]
    x.push(F(ebp - 0xcu));  // 486706 fld dword ptr [ebp - 0xc]
    x.st(0) = (x.st(0)) - (F(esi + 4u));  // 486709 fsub dword ptr [esi + 4]
    x.st(1) = (x.st(1)) * (x.st(0));  // 48670c fmulp st(1)
    x.pop();  // 48670c fmulp st(1)
    x.push(F(esi + 0xcu));  // 48670e fld dword ptr [esi + 0xc]
    x.st(0) = (x.st(0)) - (F(esi + 4u));  // 486711 fsub dword ptr [esi + 4]
    x.st(1) = (x.st(1)) / (x.st(0));  // 486714 fdivp st(1)
    x.pop();  // 486714 fdivp st(1)
    x.st(0) = (x.st(0)) + (F(edx + 4u));  // 486716 fadd dword ptr [edx + 4]
    eax = rd32((ebp - 0x38u));  // 486719 mov eax, dword ptr [ebp - 0x38]
    SF((ecx + 0xcu), x.st(0));  // 48671c fstp dword ptr [ecx + 0xc]
    x.pop();  // 48671c fstp dword ptr [ecx + 0xc]
    edi = rd32((esi + 8u));  // 48671f mov edi, dword ptr [esi + 8]
    wr32((ebx + 0x18u), edi);  // 486722 mov dword ptr [ebx + 0x18], edi
    edi = rd32((esi + 4u));  // 486725 mov edi, dword ptr [esi + 4]
    wr32((ebx + 0x14u), edi);  // 486728 mov dword ptr [ebx + 0x14], edi
    x.push(F(esi + 0xcu));  // 48672b fld dword ptr [esi + 0xc]
    wr32((ebx + 0x10u), eax);  // 48672e mov dword ptr [ebx + 0x10], eax
    SF((ebx + 0x1cu), x.st(0));  // 486731 fstp dword ptr [ebx + 0x1c]
    x.pop();  // 486731 fstp dword ptr [ebx + 0x1c]
    x.push(F(ecx + 8u));  // 486734 fld dword ptr [ecx + 8]
    eax = rd32((edx + 8u));  // 486737 mov eax, dword ptr [edx + 8]
    wr32((ecx + 0x18u), eax);  // 48673a mov dword ptr [ecx + 0x18], eax
    eax = rd32((edx + 4u));  // 48673d mov eax, dword ptr [edx + 4]
    wr32((ecx + 0x14u), eax);  // 486740 mov dword ptr [ecx + 0x14], eax
    eax = rd32((edx + 0xcu));  // 486743 mov eax, dword ptr [edx + 0xc]
    wr32((ecx + 0x1cu), eax);  // 486746 mov dword ptr [ecx + 0x1c], eax
    SF((ecx + 0x10u), x.st(0));  // 486749 fstp dword ptr [ecx + 0x10]
    x.pop();  // 486749 fstp dword ptr [ecx + 0x10]
    eax = rd32((ebp - 0x2cu));  // 48674c mov eax, dword ptr [ebp - 0x2c]
    esp = ebp;  // 48674f mov esp, ebp
    ebp = rd32(esp); esp += 4;  // 486751 pop ebp
    edi = rd32(esp); esp += 4;  // 486752 pop edi
    esi = rd32(esp); esp += 4;  // 486753 pop esi
    result = eax; popped_ = 8u; return true;  // 486754 ret 8
L_486757:
    eax = rd32((ebp - 0x18u));  // 486757 mov eax, dword ptr [ebp - 0x18]
    wr32((eax), 3u);  // 48675a mov dword ptr [eax], 3
    x.push(F(esi));  // 486760 fld dword ptr [esi]
    x.push(F(edx + 0xcu));  // 486762 fld dword ptr [edx + 0xc]
    x.st(0) = (x.st(0)) - (F(edx + 4u));  // 486765 fsub dword ptr [edx + 4]
    eax = rd32((ebp - 0x38u));  // 486768 mov eax, dword ptr [ebp - 0x38]
    x.push(F(esi + 0xcu));  // 48676b fld dword ptr [esi + 0xc]
    x.st(0) = (x.st(0)) - (F(esi + 4u));  // 48676e fsub dword ptr [esi + 4]
    x.fxch(2);  // 486771 fxch st(2)
    SF((ebx), x.st(0));  // 486773 fstp dword ptr [ebx]
    x.pop();  // 486773 fstp dword ptr [ebx]
    x.push(F(esi + 4u));  // 486775 fld dword ptr [esi + 4]
    wr32((ebx + 8u), eax);  // 486778 mov dword ptr [ebx + 8], eax
    eax = rd32((ebp - 0xcu));  // 48677b mov eax, dword ptr [ebp - 0xc]
    wr32((ebx + 0xcu), eax);  // 48677e mov dword ptr [ebx + 0xc], eax
    SF((ebx + 4u), x.st(0));  // 486781 fstp dword ptr [ebx + 4]
    x.pop();  // 486781 fstp dword ptr [ebx + 4]
    eax = rd32((edx));  // 486784 mov eax, dword ptr [edx]
    wr32((ecx), eax);  // 486786 mov dword ptr [ecx], eax
    eax = rd32((edx + 4u));  // 486788 mov eax, dword ptr [edx + 4]
    wr32((ecx + 4u), eax);  // 48678b mov dword ptr [ecx + 4], eax
    x.push(F(edx + 8u));  // 48678e fld dword ptr [edx + 8]
    x.st(0) = (x.st(0)) - (F(edx));  // 486791 fsub dword ptr [edx]
    x.push(F(ebp - 0x38u));  // 486793 fld dword ptr [ebp - 0x38]
    x.st(0) = (x.st(0)) - (F(esi));  // 486796 fsub dword ptr [esi]
    x.st(1) = (x.st(1)) * (x.st(0));  // 486798 fmulp st(1)
    x.pop();  // 486798 fmulp st(1)
    x.push(F(esi + 8u));  // 48679a fld dword ptr [esi + 8]
    x.st(0) = (x.st(0)) - (F(esi));  // 48679d fsub dword ptr [esi]
    x.st(1) = (x.st(1)) / (x.st(0));  // 48679f fdivp st(1)
    x.pop();  // 48679f fdivp st(1)
    x.fxch(2);  // 4867a1 fxch st(2)
    x.st(1) = (x.st(1)) / (x.st(0));  // 4867a3 fdivp st(1)
    x.pop();  // 4867a3 fdivp st(1)
    x.push(F(edx));  // 4867a5 fld dword ptr [edx]
    x.st(2) = (x.st(2)) + (x.st(0));  // 4867a7 faddp st(2)
    x.pop();  // 4867a7 faddp st(2)
    x.push(F(ebp - 0xcu));  // 4867a9 fld dword ptr [ebp - 0xc]
    x.fxch(2);  // 4867ac fxch st(2)
    SF((ecx + 8u), x.st(0));  // 4867ae fstp dword ptr [ecx + 8]
    x.pop();  // 4867ae fstp dword ptr [ecx + 8]
    x.fxch(1);  // 4867b1 fxch st(1)
    x.st(0) = (x.st(0)) - (F(esi + 4u));  // 4867b3 fsub dword ptr [esi + 4]
    x.st(0) = (x.st(0)) * (x.st(1));  // 4867b6 fmul st(1)
    x.st(0) = (x.st(0)) + (F(edx + 4u));  // 4867b8 fadd dword ptr [edx + 4]
    eax = rd32((ebp - 0x38u));  // 4867bb mov eax, dword ptr [ebp - 0x38]
    SF((ecx + 0xcu), x.st(0));  // 4867be fstp dword ptr [ecx + 0xc]
    x.pop();  // 4867be fstp dword ptr [ecx + 0xc]
    edi = rd32((esi));  // 4867c1 mov edi, dword ptr [esi]
    wr32((ebx + 0x10u), edi);  // 4867c3 mov dword ptr [ebx + 0x10], edi
    x.push(F(esi + 0xcu));  // 4867c6 fld dword ptr [esi + 0xc]
    wr32((ebx + 0x18u), eax);  // 4867c9 mov dword ptr [ebx + 0x18], eax
    eax = rd32((ebp - 0x10u));  // 4867cc mov eax, dword ptr [ebp - 0x10]
    wr32((ebx + 0x14u), eax);  // 4867cf mov dword ptr [ebx + 0x14], eax
    SF((ebx + 0x1cu), x.st(0));  // 4867d2 fstp dword ptr [ebx + 0x1c]
    x.pop();  // 4867d2 fstp dword ptr [ebx + 0x1c]
    eax = rd32((edx));  // 4867d5 mov eax, dword ptr [edx]
    wr32((ecx + 0x10u), eax);  // 4867d7 mov dword ptr [ecx + 0x10], eax
    eax = rd32((edx + 0xcu));  // 4867da mov eax, dword ptr [edx + 0xc]
    wr32((ecx + 0x1cu), eax);  // 4867dd mov dword ptr [ecx + 0x1c], eax
    x.push(F(edx + 8u));  // 4867e0 fld dword ptr [edx + 8]
    x.st(0) = (x.st(0)) - (F(edx));  // 4867e3 fsub dword ptr [edx]
    x.push(F(ebp - 0x38u));  // 4867e5 fld dword ptr [ebp - 0x38]
    x.st(0) = (x.st(0)) - (F(esi));  // 4867e8 fsub dword ptr [esi]
    x.st(1) = (x.st(1)) * (x.st(0));  // 4867ea fmulp st(1)
    x.pop();  // 4867ea fmulp st(1)
    x.push(F(esi + 8u));  // 4867ec fld dword ptr [esi + 8]
    x.st(0) = (x.st(0)) - (F(esi));  // 4867ef fsub dword ptr [esi]
    x.st(1) = (x.st(1)) / (x.st(0));  // 4867f1 fdivp st(1)
    x.pop();  // 4867f1 fdivp st(1)
    x.st(0) = (x.st(0)) + (F(edx));  // 4867f3 fadd dword ptr [edx]
    x.push(F(ebp - 0x10u));  // 4867f5 fld dword ptr [ebp - 0x10]
    x.fxch(1);  // 4867f8 fxch st(1)
    SF((ecx + 0x18u), x.st(0));  // 4867fa fstp dword ptr [ecx + 0x18]
    x.pop();  // 4867fa fstp dword ptr [ecx + 0x18]
    x.st(0) = (x.st(0)) - (F(esi + 4u));  // 4867fd fsub dword ptr [esi + 4]
    x.st(1) = (x.st(1)) * (x.st(0));  // 486800 fmulp st(1)
    x.pop();  // 486800 fmulp st(1)
    x.st(0) = (x.st(0)) + (F(edx + 4u));  // 486802 fadd dword ptr [edx + 4]
    eax = rd32((ebp - 0x38u));  // 486805 mov eax, dword ptr [ebp - 0x38]
    SF((ecx + 0x14u), x.st(0));  // 486808 fstp dword ptr [ecx + 0x14]
    x.pop();  // 486808 fstp dword ptr [ecx + 0x14]
    edi = rd32((esi + 8u));  // 48680b mov edi, dword ptr [esi + 8]
    wr32((ebx + 0x28u), edi);  // 48680e mov dword ptr [ebx + 0x28], edi
    edi = rd32((esi + 4u));  // 486811 mov edi, dword ptr [esi + 4]
    wr32((ebx + 0x24u), edi);  // 486814 mov dword ptr [ebx + 0x24], edi
    x.push(F(esi + 0xcu));  // 486817 fld dword ptr [esi + 0xc]
    wr32((ebx + 0x20u), eax);  // 48681a mov dword ptr [ebx + 0x20], eax
    SF((ebx + 0x2cu), x.st(0));  // 48681d fstp dword ptr [ebx + 0x2c]
    x.pop();  // 48681d fstp dword ptr [ebx + 0x2c]
    x.push(F(ecx + 8u));  // 486820 fld dword ptr [ecx + 8]
    eax = rd32((edx + 8u));  // 486823 mov eax, dword ptr [edx + 8]
    wr32((ecx + 0x28u), eax);  // 486826 mov dword ptr [ecx + 0x28], eax
    eax = rd32((edx + 4u));  // 486829 mov eax, dword ptr [edx + 4]
    wr32((ecx + 0x24u), eax);  // 48682c mov dword ptr [ecx + 0x24], eax
    eax = rd32((edx + 0xcu));  // 48682f mov eax, dword ptr [edx + 0xc]
    wr32((ecx + 0x2cu), eax);  // 486832 mov dword ptr [ecx + 0x2c], eax
    SF((ecx + 0x20u), x.st(0));  // 486835 fstp dword ptr [ecx + 0x20]
    x.pop();  // 486835 fstp dword ptr [ecx + 0x20]
    eax = rd32((ebp - 0x2cu));  // 486838 mov eax, dword ptr [ebp - 0x2c]
    esp = ebp;  // 48683b mov esp, ebp
    ebp = rd32(esp); esp += 4;  // 48683d pop ebp
    edi = rd32(esp); esp += 4;  // 48683e pop edi
    esi = rd32(esp); esp += 4;  // 48683f pop esi
    result = eax; popped_ = 8u; return true;  // 486840 ret 8
L_486843:
    eax = rd32((ebp - 0x18u));  // 486843 mov eax, dword ptr [ebp - 0x18]
    wr32((eax), 1u);  // 486846 mov dword ptr [eax], 1
    x.push(F(esi));  // 48684c fld dword ptr [esi]
    eax = rd32((ebp - 0x14u));  // 48684e mov eax, dword ptr [ebp - 0x14]
    SF((ebx), x.st(0));  // 486851 fstp dword ptr [ebx]
    x.pop();  // 486851 fstp dword ptr [ebx]
    edi = rd32((esi + 4u));  // 486853 mov edi, dword ptr [esi + 4]
    wr32((ebx + 4u), edi);  // 486856 mov dword ptr [ebx + 4], edi
    x.push(F(esi + 0xcu));  // 486859 fld dword ptr [esi + 0xc]
    wr32((ebx + 8u), eax);  // 48685c mov dword ptr [ebx + 8], eax
    SF((ebx + 0xcu), x.st(0));  // 48685f fstp dword ptr [ebx + 0xc]
    x.pop();  // 48685f fstp dword ptr [ebx + 0xc]
    eax = rd32((edx));  // 486862 mov eax, dword ptr [edx]
    wr32((ecx), eax);  // 486864 mov dword ptr [ecx], eax
    eax = rd32((edx + 4u));  // 486866 mov eax, dword ptr [edx + 4]
    wr32((ecx + 4u), eax);  // 486869 mov dword ptr [ecx + 4], eax
    eax = rd32((edx + 0xcu));  // 48686c mov eax, dword ptr [edx + 0xc]
    wr32((ecx + 0xcu), eax);  // 48686f mov dword ptr [ecx + 0xc], eax
    x.push(F(edx + 8u));  // 486872 fld dword ptr [edx + 8]
    x.st(0) = (x.st(0)) - (F(edx));  // 486875 fsub dword ptr [edx]
    x.push(F(ebp - 0x14u));  // 486877 fld dword ptr [ebp - 0x14]
    x.st(0) = (x.st(0)) - (F(esi));  // 48687a fsub dword ptr [esi]
    x.st(1) = (x.st(1)) * (x.st(0));  // 48687c fmulp st(1)
    x.pop();  // 48687c fmulp st(1)
    x.push(F(esi + 8u));  // 48687e fld dword ptr [esi + 8]
    x.st(0) = (x.st(0)) - (F(esi));  // 486881 fsub dword ptr [esi]
    x.st(1) = (x.st(1)) / (x.st(0));  // 486883 fdivp st(1)
    x.pop();  // 486883 fdivp st(1)
    x.st(0) = (x.st(0)) + (F(edx));  // 486885 fadd dword ptr [edx]
    SF((ecx + 8u), x.st(0));  // 486887 fstp dword ptr [ecx + 8]
    x.pop();  // 486887 fstp dword ptr [ecx + 8]
    eax = rd32((ebp - 0x2cu));  // 48688a mov eax, dword ptr [ebp - 0x2c]
    esp = ebp;  // 48688d mov esp, ebp
    ebp = rd32(esp); esp += 4;  // 48688f pop ebp
    edi = rd32(esp); esp += 4;  // 486890 pop edi
    esi = rd32(esp); esp += 4;  // 486891 pop esi
    result = eax; popped_ = 8u; return true;  // 486892 ret 8
L_486895:
    eax = rd32((ebp - 0x18u));  // 486895 mov eax, dword ptr [ebp - 0x18]
    wr32((eax), 2u);  // 486898 mov dword ptr [eax], 2
    x.push(F(esi));  // 48689e fld dword ptr [esi]
    eax = rd32((ebp - 0x14u));  // 4868a0 mov eax, dword ptr [ebp - 0x14]
    SF((ebx), x.st(0));  // 4868a3 fstp dword ptr [ebx]
    x.pop();  // 4868a3 fstp dword ptr [ebx]
    edi = rd32((esi + 4u));  // 4868a5 mov edi, dword ptr [esi + 4]
    wr32((ebx + 4u), edi);  // 4868a8 mov dword ptr [ebx + 4], edi
    x.push(F(esi + 0xcu));  // 4868ab fld dword ptr [esi + 0xc]
    wr32((ebx + 8u), eax);  // 4868ae mov dword ptr [ebx + 8], eax
    SF((ebx + 0xcu), x.st(0));  // 4868b1 fstp dword ptr [ebx + 0xc]
    x.pop();  // 4868b1 fstp dword ptr [ebx + 0xc]
    edi = rd32((edx));  // 4868b4 mov edi, dword ptr [edx]
    wr32((ecx), edi);  // 4868b6 mov dword ptr [ecx], edi
    edi = rd32((edx + 4u));  // 4868b8 mov edi, dword ptr [edx + 4]
    wr32((ecx + 4u), edi);  // 4868bb mov dword ptr [ecx + 4], edi
    edi = rd32((edx + 0xcu));  // 4868be mov edi, dword ptr [edx + 0xc]
    wr32((ecx + 0xcu), edi);  // 4868c1 mov dword ptr [ecx + 0xc], edi
    x.push(F(edx + 8u));  // 4868c4 fld dword ptr [edx + 8]
    x.st(0) = (x.st(0)) - (F(edx));  // 4868c7 fsub dword ptr [edx]
    x.push(F(ebp - 0x14u));  // 4868c9 fld dword ptr [ebp - 0x14]
    x.st(0) = (x.st(0)) - (F(esi));  // 4868cc fsub dword ptr [esi]
    x.st(1) = (x.st(1)) * (x.st(0));  // 4868ce fmulp st(1)
    x.pop();  // 4868ce fmulp st(1)
    x.push(F(esi + 8u));  // 4868d0 fld dword ptr [esi + 8]
    x.st(0) = (x.st(0)) - (F(esi));  // 4868d3 fsub dword ptr [esi]
    x.st(1) = (x.st(1)) / (x.st(0));  // 4868d5 fdivp st(1)
    x.pop();  // 4868d5 fdivp st(1)
    x.st(0) = (x.st(0)) + (F(edx));  // 4868d7 fadd dword ptr [edx]
    SF((ecx + 8u), x.st(0));  // 4868d9 fstp dword ptr [ecx + 8]
    x.pop();  // 4868d9 fstp dword ptr [ecx + 8]
    wr32((ebx + 0x10u), eax);  // 4868dc mov dword ptr [ebx + 0x10], eax
    eax = rd32((ebp - 0x10u));  // 4868df mov eax, dword ptr [ebp - 0x10]
    wr32((ebx + 0x14u), eax);  // 4868e2 mov dword ptr [ebx + 0x14], eax
    eax = rd32((esi + 8u));  // 4868e5 mov eax, dword ptr [esi + 8]
    wr32((ebx + 0x18u), eax);  // 4868e8 mov dword ptr [ebx + 0x18], eax
    eax = rd32((esi + 0xcu));  // 4868eb mov eax, dword ptr [esi + 0xc]
    wr32((ebx + 0x1cu), eax);  // 4868ee mov dword ptr [ebx + 0x1c], eax
    eax = rd32((ecx + 8u));  // 4868f1 mov eax, dword ptr [ecx + 8]
    wr32((ecx + 0x10u), eax);  // 4868f4 mov dword ptr [ecx + 0x10], eax
    x.push(F(edx + 0xcu));  // 4868f7 fld dword ptr [edx + 0xc]
    x.st(0) = (x.st(0)) - (F(edx + 4u));  // 4868fa fsub dword ptr [edx + 4]
    x.push(F(ebp - 0x10u));  // 4868fd fld dword ptr [ebp - 0x10]
    x.st(0) = (x.st(0)) - (F(esi + 4u));  // 486900 fsub dword ptr [esi + 4]
    x.st(1) = (x.st(1)) * (x.st(0));  // 486903 fmulp st(1)
    x.pop();  // 486903 fmulp st(1)
    x.push(F(esi + 0xcu));  // 486905 fld dword ptr [esi + 0xc]
    x.st(0) = (x.st(0)) - (F(esi + 4u));  // 486908 fsub dword ptr [esi + 4]
    x.st(1) = (x.st(1)) / (x.st(0));  // 48690b fdivp st(1)
    x.pop();  // 48690b fdivp st(1)
    x.st(0) = (x.st(0)) + (F(edx + 4u));  // 48690d fadd dword ptr [edx + 4]
    SF((ecx + 0x14u), x.st(0));  // 486910 fstp dword ptr [ecx + 0x14]
    x.pop();  // 486910 fstp dword ptr [ecx + 0x14]
    eax = rd32((edx + 8u));  // 486913 mov eax, dword ptr [edx + 8]
    wr32((ecx + 0x18u), eax);  // 486916 mov dword ptr [ecx + 0x18], eax
    eax = rd32((edx + 0xcu));  // 486919 mov eax, dword ptr [edx + 0xc]
    wr32((ecx + 0x1cu), eax);  // 48691c mov dword ptr [ecx + 0x1c], eax
    eax = rd32((ebp - 0x2cu));  // 48691f mov eax, dword ptr [ebp - 0x2c]
    esp = ebp;  // 486922 mov esp, ebp
    ebp = rd32(esp); esp += 4;  // 486924 pop ebp
    edi = rd32(esp); esp += 4;  // 486925 pop edi
    esi = rd32(esp); esp += 4;  // 486926 pop esi
    result = eax; popped_ = 8u; return true;  // 486927 ret 8
L_48692a:
    eax = rd32((ebp - 0x18u));  // 48692a mov eax, dword ptr [ebp - 0x18]
    wr32((eax), 2u);  // 48692d mov dword ptr [eax], 2
    x.push(F(esi));  // 486933 fld dword ptr [esi]
    eax = rd32((ebp - 0x14u));  // 486935 mov eax, dword ptr [ebp - 0x14]
    SF((ebx), x.st(0));  // 486938 fstp dword ptr [ebx]
    x.pop();  // 486938 fstp dword ptr [ebx]
    edi = rd32((esi + 4u));  // 48693a mov edi, dword ptr [esi + 4]
    wr32((ebx + 4u), edi);  // 48693d mov dword ptr [ebx + 4], edi
    x.push(F(esi + 0xcu));  // 486940 fld dword ptr [esi + 0xc]
    wr32((ebx + 8u), eax);  // 486943 mov dword ptr [ebx + 8], eax
    SF((ebx + 0xcu), x.st(0));  // 486946 fstp dword ptr [ebx + 0xc]
    x.pop();  // 486946 fstp dword ptr [ebx + 0xc]
    edi = rd32((edx));  // 486949 mov edi, dword ptr [edx]
    wr32((ecx), edi);  // 48694b mov dword ptr [ecx], edi
    edi = rd32((edx + 4u));  // 48694d mov edi, dword ptr [edx + 4]
    wr32((ecx + 4u), edi);  // 486950 mov dword ptr [ecx + 4], edi
    edi = rd32((edx + 0xcu));  // 486953 mov edi, dword ptr [edx + 0xc]
    wr32((ecx + 0xcu), edi);  // 486956 mov dword ptr [ecx + 0xc], edi
    x.push(F(edx + 8u));  // 486959 fld dword ptr [edx + 8]
    x.st(0) = (x.st(0)) - (F(edx));  // 48695c fsub dword ptr [edx]
    x.push(F(ebp - 0x14u));  // 48695e fld dword ptr [ebp - 0x14]
    x.st(0) = (x.st(0)) - (F(esi));  // 486961 fsub dword ptr [esi]
    x.st(1) = (x.st(1)) * (x.st(0));  // 486963 fmulp st(1)
    x.pop();  // 486963 fmulp st(1)
    x.push(F(esi + 8u));  // 486965 fld dword ptr [esi + 8]
    x.st(0) = (x.st(0)) - (F(esi));  // 486968 fsub dword ptr [esi]
    x.st(1) = (x.st(1)) / (x.st(0));  // 48696a fdivp st(1)
    x.pop();  // 48696a fdivp st(1)
    x.st(0) = (x.st(0)) + (F(edx));  // 48696c fadd dword ptr [edx]
    SF((ecx + 8u), x.st(0));  // 48696e fstp dword ptr [ecx + 8]
    x.pop();  // 48696e fstp dword ptr [ecx + 8]
    wr32((ebx + 0x10u), eax);  // 486971 mov dword ptr [ebx + 0x10], eax
    x.push(F(esi + 4u));  // 486974 fld dword ptr [esi + 4]
    eax = rd32((ebp - 0xcu));  // 486977 mov eax, dword ptr [ebp - 0xc]
    SF((ebx + 0x14u), x.st(0));  // 48697a fstp dword ptr [ebx + 0x14]
    x.pop();  // 48697a fstp dword ptr [ebx + 0x14]
    x.push(F(esi + 8u));  // 48697d fld dword ptr [esi + 8]
    wr32((ebx + 0x1cu), eax);  // 486980 mov dword ptr [ebx + 0x1c], eax
    SF((ebx + 0x18u), x.st(0));  // 486983 fstp dword ptr [ebx + 0x18]
    x.pop();  // 486983 fstp dword ptr [ebx + 0x18]
    eax = rd32((ecx + 8u));  // 486986 mov eax, dword ptr [ecx + 8]
    wr32((ecx + 0x10u), eax);  // 486989 mov dword ptr [ecx + 0x10], eax
    eax = rd32((edx + 4u));  // 48698c mov eax, dword ptr [edx + 4]
    wr32((ecx + 0x14u), eax);  // 48698f mov dword ptr [ecx + 0x14], eax
    eax = rd32((edx + 8u));  // 486992 mov eax, dword ptr [edx + 8]
    wr32((ecx + 0x18u), eax);  // 486995 mov dword ptr [ecx + 0x18], eax
    x.push(F(edx + 0xcu));  // 486998 fld dword ptr [edx + 0xc]
    x.st(0) = (x.st(0)) - (F(edx + 4u));  // 48699b fsub dword ptr [edx + 4]
    x.push(F(ebp - 0xcu));  // 48699e fld dword ptr [ebp - 0xc]
    x.st(0) = (x.st(0)) - (F(esi + 4u));  // 4869a1 fsub dword ptr [esi + 4]
    x.st(1) = (x.st(1)) * (x.st(0));  // 4869a4 fmulp st(1)
    x.pop();  // 4869a4 fmulp st(1)
    x.push(F(esi + 0xcu));  // 4869a6 fld dword ptr [esi + 0xc]
    x.st(0) = (x.st(0)) - (F(esi + 4u));  // 4869a9 fsub dword ptr [esi + 4]
    x.st(1) = (x.st(1)) / (x.st(0));  // 4869ac fdivp st(1)
    x.pop();  // 4869ac fdivp st(1)
    x.st(0) = (x.st(0)) + (F(edx + 4u));  // 4869ae fadd dword ptr [edx + 4]
    SF((ecx + 0x1cu), x.st(0));  // 4869b1 fstp dword ptr [ecx + 0x1c]
    x.pop();  // 4869b1 fstp dword ptr [ecx + 0x1c]
    eax = rd32((ebp - 0x2cu));  // 4869b4 mov eax, dword ptr [ebp - 0x2c]
    esp = ebp;  // 4869b7 mov esp, ebp
    ebp = rd32(esp); esp += 4;  // 4869b9 pop ebp
    edi = rd32(esp); esp += 4;  // 4869ba pop edi
    esi = rd32(esp); esp += 4;  // 4869bb pop esi
    result = eax; popped_ = 8u; return true;  // 4869bc ret 8
L_4869bf:
    eax = rd32((ebp - 0x18u));  // 4869bf mov eax, dword ptr [ebp - 0x18]
    wr32((eax), 3u);  // 4869c2 mov dword ptr [eax], 3
    x.push(F(esi));  // 4869c8 fld dword ptr [esi]
    x.push(F(edx + 0xcu));  // 4869ca fld dword ptr [edx + 0xc]
    x.st(0) = (x.st(0)) - (F(edx + 4u));  // 4869cd fsub dword ptr [edx + 4]
    eax = rd32((ebp - 0x14u));  // 4869d0 mov eax, dword ptr [ebp - 0x14]
    x.push(F(esi + 0xcu));  // 4869d3 fld dword ptr [esi + 0xc]
    x.st(0) = (x.st(0)) - (F(esi + 4u));  // 4869d6 fsub dword ptr [esi + 4]
    x.fxch(2);  // 4869d9 fxch st(2)
    SF((ebx), x.st(0));  // 4869db fstp dword ptr [ebx]
    x.pop();  // 4869db fstp dword ptr [ebx]
    edi = rd32((esi + 4u));  // 4869dd mov edi, dword ptr [esi + 4]
    wr32((ebx + 4u), edi);  // 4869e0 mov dword ptr [ebx + 4], edi
    x.push(F(esi + 0xcu));  // 4869e3 fld dword ptr [esi + 0xc]
    wr32((ebx + 8u), eax);  // 4869e6 mov dword ptr [ebx + 8], eax
    SF((ebx + 0xcu), x.st(0));  // 4869e9 fstp dword ptr [ebx + 0xc]
    x.pop();  // 4869e9 fstp dword ptr [ebx + 0xc]
    edi = rd32((edx));  // 4869ec mov edi, dword ptr [edx]
    wr32((ecx), edi);  // 4869ee mov dword ptr [ecx], edi
    edi = rd32((edx + 4u));  // 4869f0 mov edi, dword ptr [edx + 4]
    wr32((ecx + 4u), edi);  // 4869f3 mov dword ptr [ecx + 4], edi
    edi = rd32((edx + 0xcu));  // 4869f6 mov edi, dword ptr [edx + 0xc]
    wr32((ecx + 0xcu), edi);  // 4869f9 mov dword ptr [ecx + 0xc], edi
    x.push(F(edx + 8u));  // 4869fc fld dword ptr [edx + 8]
    x.st(0) = (x.st(0)) - (F(edx));  // 4869ff fsub dword ptr [edx]
    x.push(F(ebp - 0x14u));  // 486a01 fld dword ptr [ebp - 0x14]
    x.st(0) = (x.st(0)) - (F(esi));  // 486a04 fsub dword ptr [esi]
    x.st(1) = (x.st(1)) * (x.st(0));  // 486a06 fmulp st(1)
    x.pop();  // 486a06 fmulp st(1)
    x.push(F(esi + 8u));  // 486a08 fld dword ptr [esi + 8]
    x.st(0) = (x.st(0)) - (F(esi));  // 486a0b fsub dword ptr [esi]
    x.st(1) = (x.st(1)) / (x.st(0));  // 486a0d fdivp st(1)
    x.pop();  // 486a0d fdivp st(1)
    x.st(0) = (x.st(0)) + (F(edx));  // 486a0f fadd dword ptr [edx]
    SF((ecx + 8u), x.st(0));  // 486a11 fstp dword ptr [ecx + 8]
    x.pop();  // 486a11 fstp dword ptr [ecx + 8]
    wr32((ebx + 0x10u), eax);  // 486a14 mov dword ptr [ebx + 0x10], eax
    x.push(F(esi + 4u));  // 486a17 fld dword ptr [esi + 4]
    eax = rd32((ebp - 0xcu));  // 486a1a mov eax, dword ptr [ebp - 0xc]
    SF((ebx + 0x14u), x.st(0));  // 486a1d fstp dword ptr [ebx + 0x14]
    x.pop();  // 486a1d fstp dword ptr [ebx + 0x14]
    x.push(F(esi + 8u));  // 486a20 fld dword ptr [esi + 8]
    wr32((ebx + 0x1cu), eax);  // 486a23 mov dword ptr [ebx + 0x1c], eax
    x.push(F(ebp - 0xcu));  // 486a26 fld dword ptr [ebp - 0xc]
    x.fxch(1);  // 486a29 fxch st(1)
    SF((ebx + 0x18u), x.st(0));  // 486a2b fstp dword ptr [ebx + 0x18]
    x.pop();  // 486a2b fstp dword ptr [ebx + 0x18]
    eax = rd32((ecx + 8u));  // 486a2e mov eax, dword ptr [ecx + 8]
    wr32((ecx + 0x10u), eax);  // 486a31 mov dword ptr [ecx + 0x10], eax
    eax = rd32((edx + 4u));  // 486a34 mov eax, dword ptr [edx + 4]
    wr32((ecx + 0x14u), eax);  // 486a37 mov dword ptr [ecx + 0x14], eax
    eax = rd32((edx + 8u));  // 486a3a mov eax, dword ptr [edx + 8]
    wr32((ecx + 0x18u), eax);  // 486a3d mov dword ptr [ecx + 0x18], eax
    x.st(0) = (x.st(0)) - (F(esi + 4u));  // 486a40 fsub dword ptr [esi + 4]
    x.fxch(2);  // 486a43 fxch st(2)
    x.st(1) = (x.st(1)) / (x.st(0));  // 486a45 fdivp st(1)
    x.pop();  // 486a45 fdivp st(1)
    x.fxch(1);  // 486a47 fxch st(1)
    x.st(0) = (x.st(0)) * (x.st(1));  // 486a49 fmul st(1)
    x.st(0) = (x.st(0)) + (F(edx + 4u));  // 486a4b fadd dword ptr [edx + 4]
    eax = rd32((ebp - 0x14u));  // 486a4e mov eax, dword ptr [ebp - 0x14]
    SF((ecx + 0x1cu), x.st(0));  // 486a51 fstp dword ptr [ecx + 0x1c]
    x.pop();  // 486a51 fstp dword ptr [ecx + 0x1c]
    wr32((ebx + 0x20u), eax);  // 486a54 mov dword ptr [ebx + 0x20], eax
    eax = rd32((ebp - 0x10u));  // 486a57 mov eax, dword ptr [ebp - 0x10]
    wr32((ebx + 0x24u), eax);  // 486a5a mov dword ptr [ebx + 0x24], eax
    x.push(F(ebp - 0x10u));  // 486a5d fld dword ptr [ebp - 0x10]
    eax = rd32((esi + 8u));  // 486a60 mov eax, dword ptr [esi + 8]
    wr32((ebx + 0x28u), eax);  // 486a63 mov dword ptr [ebx + 0x28], eax
    eax = rd32((esi + 0xcu));  // 486a66 mov eax, dword ptr [esi + 0xc]
    wr32((ebx + 0x2cu), eax);  // 486a69 mov dword ptr [ebx + 0x2c], eax
    eax = rd32((ecx + 8u));  // 486a6c mov eax, dword ptr [ecx + 8]
    wr32((ecx + 0x20u), eax);  // 486a6f mov dword ptr [ecx + 0x20], eax
    x.st(0) = (x.st(0)) - (F(esi + 4u));  // 486a72 fsub dword ptr [esi + 4]
    x.st(1) = (x.st(1)) * (x.st(0));  // 486a75 fmulp st(1)
    x.pop();  // 486a75 fmulp st(1)
    x.st(0) = (x.st(0)) + (F(edx + 4u));  // 486a77 fadd dword ptr [edx + 4]
    SF((ecx + 0x24u), x.st(0));  // 486a7a fstp dword ptr [ecx + 0x24]
    x.pop();  // 486a7a fstp dword ptr [ecx + 0x24]
    eax = rd32((edx + 8u));  // 486a7d mov eax, dword ptr [edx + 8]
    wr32((ecx + 0x28u), eax);  // 486a80 mov dword ptr [ecx + 0x28], eax
    eax = rd32((edx + 0xcu));  // 486a83 mov eax, dword ptr [edx + 0xc]
    wr32((ecx + 0x2cu), eax);  // 486a86 mov dword ptr [ecx + 0x2c], eax
    eax = rd32((ebp - 0x2cu));  // 486a89 mov eax, dword ptr [ebp - 0x2c]
    esp = ebp;  // 486a8c mov esp, ebp
    ebp = rd32(esp); esp += 4;  // 486a8e pop ebp
    edi = rd32(esp); esp += 4;  // 486a8f pop edi
    esi = rd32(esp); esp += 4;  // 486a90 pop esi
    result = eax; popped_ = 8u; return true;  // 486a91 ret 8
L_486a94:
    eax = rd32((ebp - 0x18u));  // 486a94 mov eax, dword ptr [ebp - 0x18]
    wr32((eax), 2u);  // 486a97 mov dword ptr [eax], 2
    x.push(F(esi));  // 486a9d fld dword ptr [esi]
    x.push(F(edx + 8u));  // 486a9f fld dword ptr [edx + 8]
    x.st(0) = (x.st(0)) - (F(edx));  // 486aa2 fsub dword ptr [edx]
    eax = rd32((ebp - 0x14u));  // 486aa4 mov eax, dword ptr [ebp - 0x14]
    x.push(F(esi + 8u));  // 486aa7 fld dword ptr [esi + 8]
    x.st(0) = (x.st(0)) - (F(esi));  // 486aaa fsub dword ptr [esi]
    x.fxch(2);  // 486aac fxch st(2)
    SF((ebx), x.st(0));  // 486aae fstp dword ptr [ebx]
    x.pop();  // 486aae fstp dword ptr [ebx]
    edi = rd32((esi + 4u));  // 486ab0 mov edi, dword ptr [esi + 4]
    wr32((ebx + 4u), edi);  // 486ab3 mov dword ptr [ebx + 4], edi
    x.push(F(esi + 0xcu));  // 486ab6 fld dword ptr [esi + 0xc]
    wr32((ebx + 8u), eax);  // 486ab9 mov dword ptr [ebx + 8], eax
    x.push(F(ebp - 0x14u));  // 486abc fld dword ptr [ebp - 0x14]
    x.fxch(1);  // 486abf fxch st(1)
    SF((ebx + 0xcu), x.st(0));  // 486ac1 fstp dword ptr [ebx + 0xc]
    x.pop();  // 486ac1 fstp dword ptr [ebx + 0xc]
    eax = rd32((edx));  // 486ac4 mov eax, dword ptr [edx]
    wr32((ecx), eax);  // 486ac6 mov dword ptr [ecx], eax
    eax = rd32((edx + 4u));  // 486ac8 mov eax, dword ptr [edx + 4]
    wr32((ecx + 4u), eax);  // 486acb mov dword ptr [ecx + 4], eax
    eax = rd32((edx + 0xcu));  // 486ace mov eax, dword ptr [edx + 0xc]
    wr32((ecx + 0xcu), eax);  // 486ad1 mov dword ptr [ecx + 0xc], eax
    x.st(0) = (x.st(0)) - (F(esi));  // 486ad4 fsub dword ptr [esi]
    x.fxch(2);  // 486ad6 fxch st(2)
    x.st(1) = (x.st(1)) / (x.st(0));  // 486ad8 fdivp st(1)
    x.pop();  // 486ad8 fdivp st(1)
    x.fxch(1);  // 486ada fxch st(1)
    x.st(0) = (x.st(0)) * (x.st(1));  // 486adc fmul st(1)
    x.st(0) = (x.st(0)) + (F(edx));  // 486ade fadd dword ptr [edx]
    eax = rd32((ebp - 0x38u));  // 486ae0 mov eax, dword ptr [ebp - 0x38]
    SF((ecx + 8u), x.st(0));  // 486ae3 fstp dword ptr [ecx + 8]
    x.pop();  // 486ae3 fstp dword ptr [ecx + 8]
    edi = rd32((esi + 8u));  // 486ae6 mov edi, dword ptr [esi + 8]
    wr32((ebx + 0x18u), edi);  // 486ae9 mov dword ptr [ebx + 0x18], edi
    edi = rd32((esi + 4u));  // 486aec mov edi, dword ptr [esi + 4]
    wr32((ebx + 0x14u), edi);  // 486aef mov dword ptr [ebx + 0x14], edi
    x.push(F(esi + 0xcu));  // 486af2 fld dword ptr [esi + 0xc]
    wr32((ebx + 0x10u), eax);  // 486af5 mov dword ptr [ebx + 0x10], eax
    x.push(F(ebp - 0x38u));  // 486af8 fld dword ptr [ebp - 0x38]
    x.fxch(1);  // 486afb fxch st(1)
    SF((ebx + 0x1cu), x.st(0));  // 486afd fstp dword ptr [ebx + 0x1c]
    x.pop();  // 486afd fstp dword ptr [ebx + 0x1c]
    eax = rd32((edx + 8u));  // 486b00 mov eax, dword ptr [edx + 8]
    wr32((ecx + 0x18u), eax);  // 486b03 mov dword ptr [ecx + 0x18], eax
    eax = rd32((edx + 4u));  // 486b06 mov eax, dword ptr [edx + 4]
    wr32((ecx + 0x14u), eax);  // 486b09 mov dword ptr [ecx + 0x14], eax
    eax = rd32((edx + 0xcu));  // 486b0c mov eax, dword ptr [edx + 0xc]
    wr32((ecx + 0x1cu), eax);  // 486b0f mov dword ptr [ecx + 0x1c], eax
    x.st(0) = (x.st(0)) - (F(esi));  // 486b12 fsub dword ptr [esi]
    x.st(1) = (x.st(1)) * (x.st(0));  // 486b14 fmulp st(1)
    x.pop();  // 486b14 fmulp st(1)
    x.st(0) = (x.st(0)) + (F(edx));  // 486b16 fadd dword ptr [edx]
    SF((ecx + 0x10u), x.st(0));  // 486b18 fstp dword ptr [ecx + 0x10]
    x.pop();  // 486b18 fstp dword ptr [ecx + 0x10]
    eax = rd32((ebp - 0x2cu));  // 486b1b mov eax, dword ptr [ebp - 0x2c]
    esp = ebp;  // 486b1e mov esp, ebp
    ebp = rd32(esp); esp += 4;  // 486b20 pop ebp
    edi = rd32(esp); esp += 4;  // 486b21 pop edi
    esi = rd32(esp); esp += 4;  // 486b22 pop esi
    result = eax; popped_ = 8u; return true;  // 486b23 ret 8
L_486b26:
    eax = rd32((ebp - 0x18u));  // 486b26 mov eax, dword ptr [ebp - 0x18]
    wr32((eax), 3u);  // 486b29 mov dword ptr [eax], 3
    x.push(F(edx + 8u));  // 486b2f fld dword ptr [edx + 8]
    x.st(0) = (x.st(0)) - (F(edx));  // 486b32 fsub dword ptr [edx]
    x.push(F(esi + 8u));  // 486b34 fld dword ptr [esi + 8]
    x.st(0) = (x.st(0)) - (F(esi));  // 486b37 fsub dword ptr [esi]
    x.st(1) = (x.st(1)) / (x.st(0));  // 486b39 fdivp st(1)
    x.pop();  // 486b39 fdivp st(1)
    x.push(F(esi));  // 486b3b fld dword ptr [esi]
    eax = rd32((ebp - 0x14u));  // 486b3d mov eax, dword ptr [ebp - 0x14]
    SF((ebx), x.st(0));  // 486b40 fstp dword ptr [ebx]
    x.pop();  // 486b40 fstp dword ptr [ebx]
    edi = rd32((esi + 4u));  // 486b42 mov edi, dword ptr [esi + 4]
    wr32((ebx + 4u), edi);  // 486b45 mov dword ptr [ebx + 4], edi
    x.push(F(esi + 0xcu));  // 486b48 fld dword ptr [esi + 0xc]
    wr32((ebx + 8u), eax);  // 486b4b mov dword ptr [ebx + 8], eax
    x.push(F(ebp - 0x14u));  // 486b4e fld dword ptr [ebp - 0x14]
    x.fxch(1);  // 486b51 fxch st(1)
    SF((ebx + 0xcu), x.st(0));  // 486b53 fstp dword ptr [ebx + 0xc]
    x.pop();  // 486b53 fstp dword ptr [ebx + 0xc]
    edi = rd32((edx));  // 486b56 mov edi, dword ptr [edx]
    wr32((ecx), edi);  // 486b58 mov dword ptr [ecx], edi
    edi = rd32((edx + 4u));  // 486b5a mov edi, dword ptr [edx + 4]
    wr32((ecx + 4u), edi);  // 486b5d mov dword ptr [ecx + 4], edi
    edi = rd32((edx + 0xcu));  // 486b60 mov edi, dword ptr [edx + 0xc]
    wr32((ecx + 0xcu), edi);  // 486b63 mov dword ptr [ecx + 0xc], edi
    x.st(0) = (x.st(0)) - (F(esi));  // 486b66 fsub dword ptr [esi]
    x.st(0) = (x.st(0)) * (x.st(1));  // 486b68 fmul st(1)
    x.st(0) = (x.st(0)) + (F(edx));  // 486b6a fadd dword ptr [edx]
    SF((ecx + 8u), x.st(0));  // 486b6c fstp dword ptr [ecx + 8]
    x.pop();  // 486b6c fstp dword ptr [ecx + 8]
    wr32((ebx + 0x10u), eax);  // 486b6f mov dword ptr [ebx + 0x10], eax
    eax = rd32((ebp - 0x10u));  // 486b72 mov eax, dword ptr [ebp - 0x10]
    wr32((ebx + 0x14u), eax);  // 486b75 mov dword ptr [ebx + 0x14], eax
    eax = rd32((esi + 8u));  // 486b78 mov eax, dword ptr [esi + 8]
    wr32((ebx + 0x18u), eax);  // 486b7b mov dword ptr [ebx + 0x18], eax
    eax = rd32((esi + 0xcu));  // 486b7e mov eax, dword ptr [esi + 0xc]
    wr32((ebx + 0x1cu), eax);  // 486b81 mov dword ptr [ebx + 0x1c], eax
    eax = rd32((ecx + 8u));  // 486b84 mov eax, dword ptr [ecx + 8]
    wr32((ecx + 0x10u), eax);  // 486b87 mov dword ptr [ecx + 0x10], eax
    x.push(F(edx + 0xcu));  // 486b8a fld dword ptr [edx + 0xc]
    x.st(0) = (x.st(0)) - (F(edx + 4u));  // 486b8d fsub dword ptr [edx + 4]
    x.push(F(ebp - 0x10u));  // 486b90 fld dword ptr [ebp - 0x10]
    x.st(0) = (x.st(0)) - (F(esi + 4u));  // 486b93 fsub dword ptr [esi + 4]
    x.st(1) = (x.st(1)) * (x.st(0));  // 486b96 fmulp st(1)
    x.pop();  // 486b96 fmulp st(1)
    x.push(F(esi + 0xcu));  // 486b98 fld dword ptr [esi + 0xc]
    x.st(0) = (x.st(0)) - (F(esi + 4u));  // 486b9b fsub dword ptr [esi + 4]
    x.st(1) = (x.st(1)) / (x.st(0));  // 486b9e fdivp st(1)
    x.pop();  // 486b9e fdivp st(1)
    x.st(0) = (x.st(0)) + (F(edx + 4u));  // 486ba0 fadd dword ptr [edx + 4]
    eax = rd32((ebp - 0x38u));  // 486ba3 mov eax, dword ptr [ebp - 0x38]
    SF((ecx + 0x14u), x.st(0));  // 486ba6 fstp dword ptr [ecx + 0x14]
    x.pop();  // 486ba6 fstp dword ptr [ecx + 0x14]
    edi = rd32((edx + 8u));  // 486ba9 mov edi, dword ptr [edx + 8]
    wr32((ecx + 0x18u), edi);  // 486bac mov dword ptr [ecx + 0x18], edi
    edi = rd32((edx + 0xcu));  // 486baf mov edi, dword ptr [edx + 0xc]
    wr32((ecx + 0x1cu), edi);  // 486bb2 mov dword ptr [ecx + 0x1c], edi
    edi = rd32((esi + 8u));  // 486bb5 mov edi, dword ptr [esi + 8]
    wr32((ebx + 0x28u), edi);  // 486bb8 mov dword ptr [ebx + 0x28], edi
    edi = rd32((esi + 4u));  // 486bbb mov edi, dword ptr [esi + 4]
    wr32((ebx + 0x24u), edi);  // 486bbe mov dword ptr [ebx + 0x24], edi
    x.push(F(esi + 0xcu));  // 486bc1 fld dword ptr [esi + 0xc]
    wr32((ebx + 0x20u), eax);  // 486bc4 mov dword ptr [ebx + 0x20], eax
    x.push(F(ebp - 0x38u));  // 486bc7 fld dword ptr [ebp - 0x38]
    x.fxch(1);  // 486bca fxch st(1)
    SF((ebx + 0x2cu), x.st(0));  // 486bcc fstp dword ptr [ebx + 0x2c]
    x.pop();  // 486bcc fstp dword ptr [ebx + 0x2c]
    eax = rd32((edx + 8u));  // 486bcf mov eax, dword ptr [edx + 8]
    wr32((ecx + 0x28u), eax);  // 486bd2 mov dword ptr [ecx + 0x28], eax
    eax = rd32((edx + 4u));  // 486bd5 mov eax, dword ptr [edx + 4]
    wr32((ecx + 0x24u), eax);  // 486bd8 mov dword ptr [ecx + 0x24], eax
    eax = rd32((edx + 0xcu));  // 486bdb mov eax, dword ptr [edx + 0xc]
    wr32((ecx + 0x2cu), eax);  // 486bde mov dword ptr [ecx + 0x2c], eax
    x.st(0) = (x.st(0)) - (F(esi));  // 486be1 fsub dword ptr [esi]
    x.st(1) = (x.st(1)) * (x.st(0));  // 486be3 fmulp st(1)
    x.pop();  // 486be3 fmulp st(1)
    x.st(0) = (x.st(0)) + (F(edx));  // 486be5 fadd dword ptr [edx]
    SF((ecx + 0x20u), x.st(0));  // 486be7 fstp dword ptr [ecx + 0x20]
    x.pop();  // 486be7 fstp dword ptr [ecx + 0x20]
    eax = rd32((ebp - 0x2cu));  // 486bea mov eax, dword ptr [ebp - 0x2c]
    esp = ebp;  // 486bed mov esp, ebp
    ebp = rd32(esp); esp += 4;  // 486bef pop ebp
    edi = rd32(esp); esp += 4;  // 486bf0 pop edi
    esi = rd32(esp); esp += 4;  // 486bf1 pop esi
    result = eax; popped_ = 8u; return true;  // 486bf2 ret 8
L_486bf5:
    eax = rd32((ebp - 0x18u));  // 486bf5 mov eax, dword ptr [ebp - 0x18]
    wr32((eax), 3u);  // 486bf8 mov dword ptr [eax], 3
    x.push(F(edx + 8u));  // 486bfe fld dword ptr [edx + 8]
    x.st(0) = (x.st(0)) - (F(edx));  // 486c01 fsub dword ptr [edx]
    x.push(F(esi + 8u));  // 486c03 fld dword ptr [esi + 8]
    x.st(0) = (x.st(0)) - (F(esi));  // 486c06 fsub dword ptr [esi]
    x.st(1) = (x.st(1)) / (x.st(0));  // 486c08 fdivp st(1)
    x.pop();  // 486c08 fdivp st(1)
    x.push(F(esi));  // 486c0a fld dword ptr [esi]
    eax = rd32((ebp - 0x14u));  // 486c0c mov eax, dword ptr [ebp - 0x14]
    SF((ebx), x.st(0));  // 486c0f fstp dword ptr [ebx]
    x.pop();  // 486c0f fstp dword ptr [ebx]
    edi = rd32((esi + 4u));  // 486c11 mov edi, dword ptr [esi + 4]
    wr32((ebx + 4u), edi);  // 486c14 mov dword ptr [ebx + 4], edi
    x.push(F(esi + 0xcu));  // 486c17 fld dword ptr [esi + 0xc]
    wr32((ebx + 8u), eax);  // 486c1a mov dword ptr [ebx + 8], eax
    x.push(F(ebp - 0x14u));  // 486c1d fld dword ptr [ebp - 0x14]
    x.fxch(1);  // 486c20 fxch st(1)
    SF((ebx + 0xcu), x.st(0));  // 486c22 fstp dword ptr [ebx + 0xc]
    x.pop();  // 486c22 fstp dword ptr [ebx + 0xc]
    edi = rd32((edx));  // 486c25 mov edi, dword ptr [edx]
    wr32((ecx), edi);  // 486c27 mov dword ptr [ecx], edi
    edi = rd32((edx + 4u));  // 486c29 mov edi, dword ptr [edx + 4]
    wr32((ecx + 4u), edi);  // 486c2c mov dword ptr [ecx + 4], edi
    edi = rd32((edx + 0xcu));  // 486c2f mov edi, dword ptr [edx + 0xc]
    wr32((ecx + 0xcu), edi);  // 486c32 mov dword ptr [ecx + 0xc], edi
    x.st(0) = (x.st(0)) - (F(esi));  // 486c35 fsub dword ptr [esi]
    x.st(0) = (x.st(0)) * (x.st(1));  // 486c37 fmul st(1)
    x.st(0) = (x.st(0)) + (F(edx));  // 486c39 fadd dword ptr [edx]
    SF((ecx + 8u), x.st(0));  // 486c3b fstp dword ptr [ecx + 8]
    x.pop();  // 486c3b fstp dword ptr [ecx + 8]
    wr32((ebx + 0x10u), eax);  // 486c3e mov dword ptr [ebx + 0x10], eax
    x.push(F(esi + 4u));  // 486c41 fld dword ptr [esi + 4]
    eax = rd32((ebp - 0xcu));  // 486c44 mov eax, dword ptr [ebp - 0xc]
    SF((ebx + 0x14u), x.st(0));  // 486c47 fstp dword ptr [ebx + 0x14]
    x.pop();  // 486c47 fstp dword ptr [ebx + 0x14]
    x.push(F(esi + 8u));  // 486c4a fld dword ptr [esi + 8]
    wr32((ebx + 0x1cu), eax);  // 486c4d mov dword ptr [ebx + 0x1c], eax
    SF((ebx + 0x18u), x.st(0));  // 486c50 fstp dword ptr [ebx + 0x18]
    x.pop();  // 486c50 fstp dword ptr [ebx + 0x18]
    x.push(F(edx + 8u));  // 486c53 fld dword ptr [edx + 8]
    x.st(0) = (x.st(0)) - (F(edx));  // 486c56 fsub dword ptr [edx]
    x.push(F(ebp - 0x14u));  // 486c58 fld dword ptr [ebp - 0x14]
    x.st(0) = (x.st(0)) - (F(esi));  // 486c5b fsub dword ptr [esi]
    x.st(1) = (x.st(1)) * (x.st(0));  // 486c5d fmulp st(1)
    x.pop();  // 486c5d fmulp st(1)
    x.push(F(esi + 8u));  // 486c5f fld dword ptr [esi + 8]
    x.st(0) = (x.st(0)) - (F(esi));  // 486c62 fsub dword ptr [esi]
    x.st(1) = (x.st(1)) / (x.st(0));  // 486c64 fdivp st(1)
    x.pop();  // 486c64 fdivp st(1)
    x.st(0) = (x.st(0)) + (F(edx));  // 486c66 fadd dword ptr [edx]
    SF((ecx + 0x10u), x.st(0));  // 486c68 fstp dword ptr [ecx + 0x10]
    x.pop();  // 486c68 fstp dword ptr [ecx + 0x10]
    eax = rd32((edx + 4u));  // 486c6b mov eax, dword ptr [edx + 4]
    wr32((ecx + 0x14u), eax);  // 486c6e mov dword ptr [ecx + 0x14], eax
    eax = rd32((edx + 8u));  // 486c71 mov eax, dword ptr [edx + 8]
    wr32((ecx + 0x18u), eax);  // 486c74 mov dword ptr [ecx + 0x18], eax
    x.push(F(edx + 0xcu));  // 486c77 fld dword ptr [edx + 0xc]
    x.st(0) = (x.st(0)) - (F(edx + 4u));  // 486c7a fsub dword ptr [edx + 4]
    x.push(F(ebp - 0xcu));  // 486c7d fld dword ptr [ebp - 0xc]
    x.st(0) = (x.st(0)) - (F(esi + 4u));  // 486c80 fsub dword ptr [esi + 4]
    x.st(1) = (x.st(1)) * (x.st(0));  // 486c83 fmulp st(1)
    x.pop();  // 486c83 fmulp st(1)
    x.push(F(esi + 0xcu));  // 486c85 fld dword ptr [esi + 0xc]
    x.st(0) = (x.st(0)) - (F(esi + 4u));  // 486c88 fsub dword ptr [esi + 4]
    x.st(1) = (x.st(1)) / (x.st(0));  // 486c8b fdivp st(1)
    x.pop();  // 486c8b fdivp st(1)
    x.st(0) = (x.st(0)) + (F(edx + 4u));  // 486c8d fadd dword ptr [edx + 4]
    eax = rd32((ebp - 0x38u));  // 486c90 mov eax, dword ptr [ebp - 0x38]
    SF((ecx + 0x1cu), x.st(0));  // 486c93 fstp dword ptr [ecx + 0x1c]
    x.pop();  // 486c93 fstp dword ptr [ecx + 0x1c]
    edi = rd32((esi + 8u));  // 486c96 mov edi, dword ptr [esi + 8]
    wr32((ebx + 0x28u), edi);  // 486c99 mov dword ptr [ebx + 0x28], edi
    edi = rd32((esi + 4u));  // 486c9c mov edi, dword ptr [esi + 4]
    wr32((ebx + 0x24u), edi);  // 486c9f mov dword ptr [ebx + 0x24], edi
    x.push(F(esi + 0xcu));  // 486ca2 fld dword ptr [esi + 0xc]
    wr32((ebx + 0x20u), eax);  // 486ca5 mov dword ptr [ebx + 0x20], eax
    x.push(F(ebp - 0x38u));  // 486ca8 fld dword ptr [ebp - 0x38]
    x.fxch(1);  // 486cab fxch st(1)
    SF((ebx + 0x2cu), x.st(0));  // 486cad fstp dword ptr [ebx + 0x2c]
    x.pop();  // 486cad fstp dword ptr [ebx + 0x2c]
    eax = rd32((edx + 8u));  // 486cb0 mov eax, dword ptr [edx + 8]
    wr32((ecx + 0x28u), eax);  // 486cb3 mov dword ptr [ecx + 0x28], eax
    eax = rd32((edx + 4u));  // 486cb6 mov eax, dword ptr [edx + 4]
    wr32((ecx + 0x24u), eax);  // 486cb9 mov dword ptr [ecx + 0x24], eax
    eax = rd32((edx + 0xcu));  // 486cbc mov eax, dword ptr [edx + 0xc]
    wr32((ecx + 0x2cu), eax);  // 486cbf mov dword ptr [ecx + 0x2c], eax
    x.st(0) = (x.st(0)) - (F(esi));  // 486cc2 fsub dword ptr [esi]
    x.st(1) = (x.st(1)) * (x.st(0));  // 486cc4 fmulp st(1)
    x.pop();  // 486cc4 fmulp st(1)
    x.st(0) = (x.st(0)) + (F(edx));  // 486cc6 fadd dword ptr [edx]
    SF((ecx + 0x20u), x.st(0));  // 486cc8 fstp dword ptr [ecx + 0x20]
    x.pop();  // 486cc8 fstp dword ptr [ecx + 0x20]
    eax = rd32((ebp - 0x2cu));  // 486ccb mov eax, dword ptr [ebp - 0x2c]
    esp = ebp;  // 486cce mov esp, ebp
    ebp = rd32(esp); esp += 4;  // 486cd0 pop ebp
    edi = rd32(esp); esp += 4;  // 486cd1 pop edi
    esi = rd32(esp); esp += 4;  // 486cd2 pop esi
    result = eax; popped_ = 8u; return true;  // 486cd3 ret 8
L_486cd6:
    eax = rd32((ebp - 0x18u));  // 486cd6 mov eax, dword ptr [ebp - 0x18]
    wr32((eax), 4u);  // 486cd9 mov dword ptr [eax], 4
    x.push(F(edx + 8u));  // 486cdf fld dword ptr [edx + 8]
    x.st(0) = (x.st(0)) - (F(edx));  // 486ce2 fsub dword ptr [edx]
    x.push(F(esi + 8u));  // 486ce4 fld dword ptr [esi + 8]
    x.st(0) = (x.st(0)) - (F(esi));  // 486ce7 fsub dword ptr [esi]
    x.st(1) = (x.st(1)) / (x.st(0));  // 486ce9 fdivp st(1)
    x.pop();  // 486ce9 fdivp st(1)
    x.push(F(esi));  // 486ceb fld dword ptr [esi]
    x.push(F(edx + 0xcu));  // 486ced fld dword ptr [edx + 0xc]
    x.st(0) = (x.st(0)) - (F(edx + 4u));  // 486cf0 fsub dword ptr [edx + 4]
    eax = rd32((ebp - 0x14u));  // 486cf3 mov eax, dword ptr [ebp - 0x14]
    x.push(F(esi + 0xcu));  // 486cf6 fld dword ptr [esi + 0xc]
    x.st(0) = (x.st(0)) - (F(esi + 4u));  // 486cf9 fsub dword ptr [esi + 4]
    x.fxch(2);  // 486cfc fxch st(2)
    SF((ebx), x.st(0));  // 486cfe fstp dword ptr [ebx]
    x.pop();  // 486cfe fstp dword ptr [ebx]
    edi = rd32((esi + 4u));  // 486d00 mov edi, dword ptr [esi + 4]
    wr32((ebx + 4u), edi);  // 486d03 mov dword ptr [ebx + 4], edi
    x.push(F(esi + 0xcu));  // 486d06 fld dword ptr [esi + 0xc]
    wr32((ebx + 8u), eax);  // 486d09 mov dword ptr [ebx + 8], eax
    x.push(F(ebp - 0x14u));  // 486d0c fld dword ptr [ebp - 0x14]
    x.fxch(1);  // 486d0f fxch st(1)
    SF((ebx + 0xcu), x.st(0));  // 486d11 fstp dword ptr [ebx + 0xc]
    x.pop();  // 486d11 fstp dword ptr [ebx + 0xc]
    edi = rd32((edx));  // 486d14 mov edi, dword ptr [edx]
    wr32((ecx), edi);  // 486d16 mov dword ptr [ecx], edi
    edi = rd32((edx + 4u));  // 486d18 mov edi, dword ptr [edx + 4]
    wr32((ecx + 4u), edi);  // 486d1b mov dword ptr [ecx + 4], edi
    edi = rd32((edx + 0xcu));  // 486d1e mov edi, dword ptr [edx + 0xc]
    wr32((ecx + 0xcu), edi);  // 486d21 mov dword ptr [ecx + 0xc], edi
    x.st(0) = (x.st(0)) - (F(esi));  // 486d24 fsub dword ptr [esi]
    x.st(0) = (x.st(0)) * (x.st(3));  // 486d26 fmul st(3)
    x.st(0) = (x.st(0)) + (F(edx));  // 486d28 fadd dword ptr [edx]
    SF((ecx + 8u), x.st(0));  // 486d2a fstp dword ptr [ecx + 8]
    x.pop();  // 486d2a fstp dword ptr [ecx + 8]
    wr32((ebx + 0x10u), eax);  // 486d2d mov dword ptr [ebx + 0x10], eax
    x.push(F(esi + 4u));  // 486d30 fld dword ptr [esi + 4]
    eax = rd32((ebp - 0xcu));  // 486d33 mov eax, dword ptr [ebp - 0xc]
    SF((ebx + 0x14u), x.st(0));  // 486d36 fstp dword ptr [ebx + 0x14]
    x.pop();  // 486d36 fstp dword ptr [ebx + 0x14]
    x.push(F(esi + 8u));  // 486d39 fld dword ptr [esi + 8]
    wr32((ebx + 0x1cu), eax);  // 486d3c mov dword ptr [ebx + 0x1c], eax
    x.push(F(ebp - 0xcu));  // 486d3f fld dword ptr [ebp - 0xc]
    x.fxch(1);  // 486d42 fxch st(1)
    SF((ebx + 0x18u), x.st(0));  // 486d44 fstp dword ptr [ebx + 0x18]
    x.pop();  // 486d44 fstp dword ptr [ebx + 0x18]
    eax = rd32((ecx + 8u));  // 486d47 mov eax, dword ptr [ecx + 8]
    wr32((ecx + 0x10u), eax);  // 486d4a mov dword ptr [ecx + 0x10], eax
    eax = rd32((edx + 4u));  // 486d4d mov eax, dword ptr [edx + 4]
    wr32((ecx + 0x14u), eax);  // 486d50 mov dword ptr [ecx + 0x14], eax
    eax = rd32((edx + 8u));  // 486d53 mov eax, dword ptr [edx + 8]
    wr32((ecx + 0x18u), eax);  // 486d56 mov dword ptr [ecx + 0x18], eax
    x.st(0) = (x.st(0)) - (F(esi + 4u));  // 486d59 fsub dword ptr [esi + 4]
    x.fxch(2);  // 486d5c fxch st(2)
    x.st(1) = (x.st(1)) / (x.st(0));  // 486d5e fdivp st(1)
    x.pop();  // 486d5e fdivp st(1)
    x.fxch(1);  // 486d60 fxch st(1)
    x.st(0) = (x.st(0)) * (x.st(1));  // 486d62 fmul st(1)
    x.st(0) = (x.st(0)) + (F(edx + 4u));  // 486d64 fadd dword ptr [edx + 4]
    eax = rd32((ebp - 0x14u));  // 486d67 mov eax, dword ptr [ebp - 0x14]
    SF((ecx + 0x1cu), x.st(0));  // 486d6a fstp dword ptr [ecx + 0x1c]
    x.pop();  // 486d6a fstp dword ptr [ecx + 0x1c]
    wr32((ebx + 0x20u), eax);  // 486d6d mov dword ptr [ebx + 0x20], eax
    eax = rd32((ebp - 0x10u));  // 486d70 mov eax, dword ptr [ebp - 0x10]
    wr32((ebx + 0x24u), eax);  // 486d73 mov dword ptr [ebx + 0x24], eax
    x.push(F(ebp - 0x10u));  // 486d76 fld dword ptr [ebp - 0x10]
    eax = rd32((esi + 8u));  // 486d79 mov eax, dword ptr [esi + 8]
    wr32((ebx + 0x28u), eax);  // 486d7c mov dword ptr [ebx + 0x28], eax
    eax = rd32((esi + 0xcu));  // 486d7f mov eax, dword ptr [esi + 0xc]
    wr32((ebx + 0x2cu), eax);  // 486d82 mov dword ptr [ebx + 0x2c], eax
    eax = rd32((ecx + 8u));  // 486d85 mov eax, dword ptr [ecx + 8]
    wr32((ecx + 0x20u), eax);  // 486d88 mov dword ptr [ecx + 0x20], eax
    x.st(0) = (x.st(0)) - (F(esi + 4u));  // 486d8b fsub dword ptr [esi + 4]
    x.st(1) = (x.st(1)) * (x.st(0));  // 486d8e fmulp st(1)
    x.pop();  // 486d8e fmulp st(1)
    x.st(0) = (x.st(0)) + (F(edx + 4u));  // 486d90 fadd dword ptr [edx + 4]
    eax = rd32((ebp - 0x38u));  // 486d93 mov eax, dword ptr [ebp - 0x38]
    SF((ecx + 0x24u), x.st(0));  // 486d96 fstp dword ptr [ecx + 0x24]
    x.pop();  // 486d96 fstp dword ptr [ecx + 0x24]
    edi = rd32((edx + 8u));  // 486d99 mov edi, dword ptr [edx + 8]
    wr32((ecx + 0x28u), edi);  // 486d9c mov dword ptr [ecx + 0x28], edi
    edi = rd32((edx + 0xcu));  // 486d9f mov edi, dword ptr [edx + 0xc]
    wr32((ecx + 0x2cu), edi);  // 486da2 mov dword ptr [ecx + 0x2c], edi
    edi = rd32((esi + 8u));  // 486da5 mov edi, dword ptr [esi + 8]
    wr32((ebx + 0x38u), edi);  // 486da8 mov dword ptr [ebx + 0x38], edi
    edi = rd32((esi + 4u));  // 486dab mov edi, dword ptr [esi + 4]
    wr32((ebx + 0x34u), edi);  // 486dae mov dword ptr [ebx + 0x34], edi
    x.push(F(esi + 0xcu));  // 486db1 fld dword ptr [esi + 0xc]
    wr32((ebx + 0x30u), eax);  // 486db4 mov dword ptr [ebx + 0x30], eax
    x.push(F(ebp - 0x38u));  // 486db7 fld dword ptr [ebp - 0x38]
    x.fxch(1);  // 486dba fxch st(1)
    SF((ebx + 0x3cu), x.st(0));  // 486dbc fstp dword ptr [ebx + 0x3c]
    x.pop();  // 486dbc fstp dword ptr [ebx + 0x3c]
    eax = rd32((edx + 8u));  // 486dbf mov eax, dword ptr [edx + 8]
    wr32((ecx + 0x38u), eax);  // 486dc2 mov dword ptr [ecx + 0x38], eax
    eax = rd32((edx + 4u));  // 486dc5 mov eax, dword ptr [edx + 4]
    wr32((ecx + 0x34u), eax);  // 486dc8 mov dword ptr [ecx + 0x34], eax
    eax = rd32((edx + 0xcu));  // 486dcb mov eax, dword ptr [edx + 0xc]
    wr32((ecx + 0x3cu), eax);  // 486dce mov dword ptr [ecx + 0x3c], eax
    x.st(0) = (x.st(0)) - (F(esi));  // 486dd1 fsub dword ptr [esi]
    x.st(1) = (x.st(1)) * (x.st(0));  // 486dd3 fmulp st(1)
    x.pop();  // 486dd3 fmulp st(1)
    x.st(0) = (x.st(0)) + (F(edx));  // 486dd5 fadd dword ptr [edx]
    SF((ecx + 0x30u), x.st(0));  // 486dd7 fstp dword ptr [ecx + 0x30]
    x.pop();  // 486dd7 fstp dword ptr [ecx + 0x30]
L_486dda:
    eax = rd32((ebp - 0x2cu));  // 486dda mov eax, dword ptr [ebp - 0x2c]
    esp = ebp;  // 486ddd mov esp, ebp
    ebp = rd32(esp); esp += 4;  // 486ddf pop ebp
    edi = rd32(esp); esp += 4;  // 486de0 pop edi
    esi = rd32(esp); esp += 4;  // 486de1 pop esi
    result = eax; popped_ = 8u; return true;  // 486de2 ret 8
    return false;
}
// (popped = 8
bool decompBoxQuads(App* app, CPU& cpu, const Entry& e, reg32& result, reg32& popped_)
{
    reg32 eax = e.eax, ebx = e.ebx, ecx = e.ecx, edx = e.edx, esi = e.esi, edi = e.edi, ebp = e.ebp, esp = e.esp;
    Flags fl;
    X87 x;
    const auto rd32 = [&](reg32 a) { return app->getMemory<reg32>(a); };
    const auto rd16 = [&](reg32 a) { return reg32(app->getMemory<uint16_t>(a)); };
    const auto rd8 = [&](reg32 a) { return reg32(app->getMemory<uint8_t>(a)); };
    const auto wr32 = [&](reg32 a, reg32 v) { app->getMemory<reg32>(a) = v; };
    const auto wr16 = [&](reg32 a, reg32 v) { app->getMemory<uint16_t>(a) = uint16_t(v); };
    const auto wr8 = [&](reg32 a, reg32 v) { app->getMemory<uint8_t>(a) = uint8_t(v); };
    const auto F = [&](reg32 a) { return double(app->getMemory<float>(a)); };
    const auto D = [&](reg32 a) { return app->getMemory<double>(a); };
    const auto SF = [&](reg32 a, double v) { app->getMemory<float>(a) = float(v); };
    const auto SD = [&](reg32 a, double v) { app->getMemory<double>(a) = v; };
    esp -= 4; wr32(esp, esi);  // 487a30 push esi
    esp -= 4; wr32(esp, edi);  // 487a31 push edi
    esp -= 4; wr32(esp, ebp);  // 487a32 push ebp
    ebp = esp;  // 487a33 mov ebp, esp
    esp -= 200u;  // 487a35 sub esp, 0xc8
    { const reg32 a_ = ebp, b_ = 130u; ebp = a_ - b_; fl.sub(a_, b_); }  // 487a3b sub ebp, 0x82
    wr32((ebp + 0x7eu), eax);  // 487a41 mov dword ptr [ebp + 0x7e], eax
    esi = edx;  // 487a44 mov esi, edx
    wr32((ebp + 0x72u), ebx);  // 487a46 mov dword ptr [ebp + 0x72], ebx
    wr32((ebp + 0x6eu), ecx);  // 487a49 mov dword ptr [ebp + 0x6e], ecx
    x.push(F(eax));  // 487a4c fld dword ptr [eax]
    fl.fcmp(x.st(0), F(edx + 4u));  // 487a4e fcomp dword ptr [edx + 4]
    x.pop();  // 487a4e fcomp dword ptr [edx + 4]
    // fnstsw ax  // 487a51 fnstsw ax
    // sahf  // 487a53 sahf
    if (fl.b()) goto L_487a85;  // 487a54 jb 0x487a85
    eax = rd32((ebp + 0x7eu));  // 487a56 mov eax, dword ptr [ebp + 0x7e]
    x.push(F(eax));  // 487a59 fld dword ptr [eax]
    fl.fcmp(x.st(0), F(edx + 0xcu));  // 487a5b fcomp dword ptr [edx + 0xc]
    x.pop();  // 487a5b fcomp dword ptr [edx + 0xc]
    // fnstsw ax  // 487a5e fnstsw ax
    // sahf  // 487a60 sahf
    if (fl.a()) goto L_487a85;  // 487a61 ja 0x487a85
    eax = rd32((ebp + 0x7eu));  // 487a63 mov eax, dword ptr [ebp + 0x7e]
    x.push(F(eax + 4u));  // 487a66 fld dword ptr [eax + 4]
    fl.fcmp(x.st(0), F(edx));  // 487a69 fcomp dword ptr [edx]
    x.pop();  // 487a69 fcomp dword ptr [edx]
    // fnstsw ax  // 487a6b fnstsw ax
    // sahf  // 487a6d sahf
    if (fl.b()) goto L_487a85;  // 487a6e jb 0x487a85
    eax = rd32((ebp + 0x7eu));  // 487a70 mov eax, dword ptr [ebp + 0x7e]
    x.push(F(eax + 4u));  // 487a73 fld dword ptr [eax + 4]
    fl.fcmp(x.st(0), F(edx + 8u));  // 487a76 fcomp dword ptr [edx + 8]
    x.pop();  // 487a76 fcomp dword ptr [edx + 8]
    // fnstsw ax  // 487a79 fnstsw ax
    // sahf  // 487a7b sahf
    if (fl.a()) goto L_487a85;  // 487a7c ja 0x487a85
    eax = 1u;  // 487a7e mov eax, 1
    goto L_487a87;  // 487a83 jmp 0x487a87
L_487a85:
    eax = 0u;  // 487a85 xor eax, eax
    fl.logic(0u);  // 487a85 xor eax, eax
L_487a87:
    wr32((ebp + 0x52u), eax);  // 487a87 mov dword ptr [ebp + 0x52], eax
    eax = rd32((ebp + 0x7eu));  // 487a8a mov eax, dword ptr [ebp + 0x7e]
    x.push(F(eax + 8u));  // 487a8d fld dword ptr [eax + 8]
    fl.fcmp(x.st(0), F(esi + 4u));  // 487a90 fcomp dword ptr [esi + 4]
    x.pop();  // 487a90 fcomp dword ptr [esi + 4]
    // fnstsw ax  // 487a93 fnstsw ax
    // sahf  // 487a95 sahf
    if (fl.b()) goto L_487ac8;  // 487a96 jb 0x487ac8
    eax = rd32((ebp + 0x7eu));  // 487a98 mov eax, dword ptr [ebp + 0x7e]
    x.push(F(eax + 8u));  // 487a9b fld dword ptr [eax + 8]
    fl.fcmp(x.st(0), F(esi + 0xcu));  // 487a9e fcomp dword ptr [esi + 0xc]
    x.pop();  // 487a9e fcomp dword ptr [esi + 0xc]
    // fnstsw ax  // 487aa1 fnstsw ax
    // sahf  // 487aa3 sahf
    if (fl.a()) goto L_487ac8;  // 487aa4 ja 0x487ac8
    eax = rd32((ebp + 0x7eu));  // 487aa6 mov eax, dword ptr [ebp + 0x7e]
    x.push(F(eax + 0xcu));  // 487aa9 fld dword ptr [eax + 0xc]
    fl.fcmp(x.st(0), F(esi));  // 487aac fcomp dword ptr [esi]
    x.pop();  // 487aac fcomp dword ptr [esi]
    // fnstsw ax  // 487aae fnstsw ax
    // sahf  // 487ab0 sahf
    if (fl.b()) goto L_487ac8;  // 487ab1 jb 0x487ac8
    eax = rd32((ebp + 0x7eu));  // 487ab3 mov eax, dword ptr [ebp + 0x7e]
    x.push(F(eax + 0xcu));  // 487ab6 fld dword ptr [eax + 0xc]
    fl.fcmp(x.st(0), F(esi + 8u));  // 487ab9 fcomp dword ptr [esi + 8]
    x.pop();  // 487ab9 fcomp dword ptr [esi + 8]
    // fnstsw ax  // 487abc fnstsw ax
    // sahf  // 487abe sahf
    if (fl.a()) goto L_487ac8;  // 487abf ja 0x487ac8
    eax = 1u;  // 487ac1 mov eax, 1
    goto L_487aca;  // 487ac6 jmp 0x487aca
L_487ac8:
    eax = 0u;  // 487ac8 xor eax, eax
    fl.logic(0u);  // 487ac8 xor eax, eax
L_487aca:
    wr32((ebp + 0x56u), eax);  // 487aca mov dword ptr [ebp + 0x56], eax
    eax = rd32((ebp + 0x7eu));  // 487acd mov eax, dword ptr [ebp + 0x7e]
    x.push(F(eax + 0x10u));  // 487ad0 fld dword ptr [eax + 0x10]
    fl.fcmp(x.st(0), F(esi + 4u));  // 487ad3 fcomp dword ptr [esi + 4]
    x.pop();  // 487ad3 fcomp dword ptr [esi + 4]
    // fnstsw ax  // 487ad6 fnstsw ax
    // sahf  // 487ad8 sahf
    if (fl.b()) goto L_487b0b;  // 487ad9 jb 0x487b0b
    eax = rd32((ebp + 0x7eu));  // 487adb mov eax, dword ptr [ebp + 0x7e]
    x.push(F(eax + 0x10u));  // 487ade fld dword ptr [eax + 0x10]
    fl.fcmp(x.st(0), F(esi + 0xcu));  // 487ae1 fcomp dword ptr [esi + 0xc]
    x.pop();  // 487ae1 fcomp dword ptr [esi + 0xc]
    // fnstsw ax  // 487ae4 fnstsw ax
    // sahf  // 487ae6 sahf
    if (fl.a()) goto L_487b0b;  // 487ae7 ja 0x487b0b
    eax = rd32((ebp + 0x7eu));  // 487ae9 mov eax, dword ptr [ebp + 0x7e]
    x.push(F(eax + 0x14u));  // 487aec fld dword ptr [eax + 0x14]
    fl.fcmp(x.st(0), F(esi));  // 487aef fcomp dword ptr [esi]
    x.pop();  // 487aef fcomp dword ptr [esi]
    // fnstsw ax  // 487af1 fnstsw ax
    // sahf  // 487af3 sahf
    if (fl.b()) goto L_487b0b;  // 487af4 jb 0x487b0b
    eax = rd32((ebp + 0x7eu));  // 487af6 mov eax, dword ptr [ebp + 0x7e]
    x.push(F(eax + 0x14u));  // 487af9 fld dword ptr [eax + 0x14]
    fl.fcmp(x.st(0), F(esi + 8u));  // 487afc fcomp dword ptr [esi + 8]
    x.pop();  // 487afc fcomp dword ptr [esi + 8]
    // fnstsw ax  // 487aff fnstsw ax
    // sahf  // 487b01 sahf
    if (fl.a()) goto L_487b0b;  // 487b02 ja 0x487b0b
    eax = 1u;  // 487b04 mov eax, 1
    goto L_487b0d;  // 487b09 jmp 0x487b0d
L_487b0b:
    eax = 0u;  // 487b0b xor eax, eax
    fl.logic(0u);  // 487b0b xor eax, eax
L_487b0d:
    edx = rd32((ebp + 0x52u));  // 487b0d mov edx, dword ptr [ebp + 0x52]
    wr32((ebp + 0x5au), eax);  // 487b10 mov dword ptr [ebp + 0x5a], eax
    fl.logic(edx & edx);  // 487b13 test edx, edx
    if (fl.e()) goto L_487b34;  // 487b15 je 0x487b34
    fl.sub(rd32((ebp + 0x56u)), 0u);  // 487b17 cmp dword ptr [ebp + 0x56], 0
    if (fl.e()) goto L_487b34;  // 487b1b je 0x487b34
    fl.logic(eax & eax);  // 487b1d test eax, eax
    if (fl.e()) goto L_487b34;  // 487b1f je 0x487b34
    ebx = rd32((ebp + 0x6eu));  // 487b21 mov ebx, dword ptr [ebp + 0x6e]
    edx = rd32((ebp + 0x72u));  // 487b24 mov edx, dword ptr [ebp + 0x72]
    eax = rd32((ebp + 0x7eu));  // 487b27 mov eax, dword ptr [ebp + 0x7e]
    cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4870c0, esp, 0, eax, edx, ebx, ecx)) return false;  // 487b2a call 0x4870c0
    esp += 0u;  // 487b2a call 0x4870c0
    goto L_48801c;  // 487b2f jmp 0x48801c
L_487b34:
    fl.sub(rd32((ebp + 0x52u)), 0u);  // 487b34 cmp dword ptr [ebp + 0x52], 0
    if (fl.ne()) goto L_487b5f;  // 487b38 jne 0x487b5f
    fl.sub(rd32((ebp + 0x56u)), 0u);  // 487b3a cmp dword ptr [ebp + 0x56], 0
    if (fl.e()) goto L_487b5f;  // 487b3e je 0x487b5f
    edx = rd32((ebp + 0x7eu));  // 487b40 mov edx, dword ptr [ebp + 0x7e]
    eax = rd32((ebp + 0x7eu));  // 487b43 mov eax, dword ptr [ebp + 0x7e]
    { const reg32 a_ = edx, b_ = 8u; edx = a_ + b_; fl.add(a_, b_); }  // 487b46 add edx, 8
    cpu.ebp = ebp; if (!rawCall(app, cpu, 0x485c20, esp, 0, eax, edx, ebx, ecx)) return false;  // 487b49 call 0x485c20
    esp += 0u;  // 487b49 call 0x485c20
    edx = rd32((ebp + 0x7eu));  // 487b4e mov edx, dword ptr [ebp + 0x7e]
    eax = rd32((ebp + 0x7eu));  // 487b51 mov eax, dword ptr [ebp + 0x7e]
    { const reg32 a_ = edx, b_ = 12u; edx = a_ + b_; fl.add(a_, b_); }  // 487b54 add edx, 0xc
    { const reg32 a_ = eax, b_ = 4u; eax = a_ + b_; fl.add(a_, b_); }  // 487b57 add eax, 4
    cpu.ebp = ebp; if (!rawCall(app, cpu, 0x485c20, esp, 0, eax, edx, ebx, ecx)) return false;  // 487b5a call 0x485c20
    esp += 0u;  // 487b5a call 0x485c20
L_487b5f:
    fl.sub(rd32((ebp + 0x56u)), 0u);  // 487b5f cmp dword ptr [ebp + 0x56], 0
    if (fl.ne()) goto L_487b8d;  // 487b63 jne 0x487b8d
    fl.sub(rd32((ebp + 0x5au)), 0u);  // 487b65 cmp dword ptr [ebp + 0x5a], 0
    if (fl.e()) goto L_487b8d;  // 487b69 je 0x487b8d
    edx = rd32((ebp + 0x7eu));  // 487b6b mov edx, dword ptr [ebp + 0x7e]
    eax = rd32((ebp + 0x7eu));  // 487b6e mov eax, dword ptr [ebp + 0x7e]
    { const reg32 a_ = edx, b_ = 8u; edx = a_ + b_; fl.add(a_, b_); }  // 487b71 add edx, 8
    { const reg32 a_ = eax, b_ = 16u; eax = a_ + b_; fl.add(a_, b_); }  // 487b74 add eax, 0x10
    cpu.ebp = ebp; if (!rawCall(app, cpu, 0x485c20, esp, 0, eax, edx, ebx, ecx)) return false;  // 487b77 call 0x485c20
    esp += 0u;  // 487b77 call 0x485c20
    edx = rd32((ebp + 0x7eu));  // 487b7c mov edx, dword ptr [ebp + 0x7e]
    eax = rd32((ebp + 0x7eu));  // 487b7f mov eax, dword ptr [ebp + 0x7e]
    { const reg32 a_ = edx, b_ = 12u; edx = a_ + b_; fl.add(a_, b_); }  // 487b82 add edx, 0xc
    { const reg32 a_ = eax, b_ = 20u; eax = a_ + b_; fl.add(a_, b_); }  // 487b85 add eax, 0x14
    cpu.ebp = ebp; if (!rawCall(app, cpu, 0x485c20, esp, 0, eax, edx, ebx, ecx)) return false;  // 487b88 call 0x485c20
    esp += 0u;  // 487b88 call 0x485c20
L_487b8d:
    fl.sub(rd32((ebp + 0x52u)), 0u);  // 487b8d cmp dword ptr [ebp + 0x52], 0
    if (fl.ne()) goto L_487bb8;  // 487b91 jne 0x487bb8
    fl.sub(rd32((ebp + 0x56u)), 0u);  // 487b93 cmp dword ptr [ebp + 0x56], 0
    if (fl.e()) goto L_487bb8;  // 487b97 je 0x487bb8
    edx = rd32((ebp + 0x7eu));  // 487b99 mov edx, dword ptr [ebp + 0x7e]
    eax = rd32((ebp + 0x7eu));  // 487b9c mov eax, dword ptr [ebp + 0x7e]
    { const reg32 a_ = edx, b_ = 8u; edx = a_ + b_; fl.add(a_, b_); }  // 487b9f add edx, 8
    cpu.ebp = ebp; if (!rawCall(app, cpu, 0x485c20, esp, 0, eax, edx, ebx, ecx)) return false;  // 487ba2 call 0x485c20
    esp += 0u;  // 487ba2 call 0x485c20
    edx = rd32((ebp + 0x7eu));  // 487ba7 mov edx, dword ptr [ebp + 0x7e]
    eax = rd32((ebp + 0x7eu));  // 487baa mov eax, dword ptr [ebp + 0x7e]
    { const reg32 a_ = edx, b_ = 12u; edx = a_ + b_; fl.add(a_, b_); }  // 487bad add edx, 0xc
    { const reg32 a_ = eax, b_ = 4u; eax = a_ + b_; fl.add(a_, b_); }  // 487bb0 add eax, 4
    cpu.ebp = ebp; if (!rawCall(app, cpu, 0x485c20, esp, 0, eax, edx, ebx, ecx)) return false;  // 487bb3 call 0x485c20
    esp += 0u;  // 487bb3 call 0x485c20
L_487bb8:
    eax = rd32((ebp + 0x7eu));  // 487bb8 mov eax, dword ptr [ebp + 0x7e]
    eax = rd32((eax));  // 487bbb mov eax, dword ptr [eax]
    wr32((ebp - 0x16u), eax);  // 487bbd mov dword ptr [ebp - 0x16], eax
    eax = rd32((ebp + 0x7eu));  // 487bc0 mov eax, dword ptr [ebp + 0x7e]
    eax = rd32((eax + 4u));  // 487bc3 mov eax, dword ptr [eax + 4]
    wr32((ebp - 0x12u), eax);  // 487bc6 mov dword ptr [ebp - 0x12], eax
    eax = rd32((ebp + 0x7eu));  // 487bc9 mov eax, dword ptr [ebp + 0x7e]
    eax = rd32((eax + 8u));  // 487bcc mov eax, dword ptr [eax + 8]
    wr32((ebp - 0xeu), eax);  // 487bcf mov dword ptr [ebp - 0xe], eax
    eax = rd32((ebp + 0x7eu));  // 487bd2 mov eax, dword ptr [ebp + 0x7e]
    eax = rd32((eax + 0xcu));  // 487bd5 mov eax, dword ptr [eax + 0xc]
    wr32((ebp - 0xau), eax);  // 487bd8 mov dword ptr [ebp - 0xa], eax
    eax = rd32((ebp + 0x7eu));  // 487bdb mov eax, dword ptr [ebp + 0x7e]
    eax = rd32((eax + 8u));  // 487bde mov eax, dword ptr [eax + 8]
    wr32((ebp - 6u), eax);  // 487be1 mov dword ptr [ebp - 6], eax
    eax = rd32((ebp + 0x7eu));  // 487be4 mov eax, dword ptr [ebp + 0x7e]
    eax = rd32((eax + 0xcu));  // 487be7 mov eax, dword ptr [eax + 0xc]
    wr32((ebp - 2u), eax);  // 487bea mov dword ptr [ebp - 2], eax
    eax = rd32((ebp + 0x7eu));  // 487bed mov eax, dword ptr [ebp + 0x7e]
    eax = rd32((eax + 0x10u));  // 487bf0 mov eax, dword ptr [eax + 0x10]
    wr32((ebp + 2u), eax);  // 487bf3 mov dword ptr [ebp + 2], eax
    eax = rd32((ebp + 0x7eu));  // 487bf6 mov eax, dword ptr [ebp + 0x7e]
    eax = rd32((eax + 0x14u));  // 487bf9 mov eax, dword ptr [eax + 0x14]
    wr32((ebp + 6u), eax);  // 487bfc mov dword ptr [ebp + 6], eax
    eax = rd32((ebp + 0x7eu));  // 487bff mov eax, dword ptr [ebp + 0x7e]
    eax = rd32((eax + 0x10u));  // 487c02 mov eax, dword ptr [eax + 0x10]
    wr32((ebp + 0xau), eax);  // 487c05 mov dword ptr [ebp + 0xa], eax
    eax = rd32((ebp + 0x7eu));  // 487c08 mov eax, dword ptr [ebp + 0x7e]
    eax = rd32((eax + 0x14u));  // 487c0b mov eax, dword ptr [eax + 0x14]
    wr32((ebp + 0xeu), eax);  // 487c0e mov dword ptr [ebp + 0xe], eax
    eax = rd32((ebp + 0x7eu));  // 487c11 mov eax, dword ptr [ebp + 0x7e]
    eax = rd32((eax));  // 487c14 mov eax, dword ptr [eax]
    wr32((ebp + 0x12u), eax);  // 487c16 mov dword ptr [ebp + 0x12], eax
    eax = rd32((ebp + 0x7eu));  // 487c19 mov eax, dword ptr [ebp + 0x7e]
    ecx = (ebp + 0x5eu);  // 487c1c lea ecx, [ebp + 0x5e]
    ebx = (ebp - 0x46u);  // 487c1f lea ebx, [ebp - 0x46]
    eax = rd32((eax + 4u));  // 487c22 mov eax, dword ptr [eax + 4]
    edx = esi;  // 487c25 mov edx, esi
    wr32((ebp + 0x16u), eax);  // 487c27 mov dword ptr [ebp + 0x16], eax
    eax = (ebp - 0x16u);  // 487c2a lea eax, [ebp - 0x16]
    cpu.ebp = ebp; if (!rawCall(app, cpu, 0x485f20, esp, 0, eax, edx, ebx, ecx)) return false;  // 487c2d call 0x485f20
    esp += 0u;  // 487c2d call 0x485f20
    ecx = (ebp + 0x62u);  // 487c32 lea ecx, [ebp + 0x62]
    ebx = (ebp - 0x36u);  // 487c35 lea ebx, [ebp - 0x36]
    wr32((ebp + 0x6au), eax);  // 487c38 mov dword ptr [ebp + 0x6a], eax
    edx = esi;  // 487c3b mov edx, esi
    eax = (ebp - 6u);  // 487c3d lea eax, [ebp - 6]
    cpu.ebp = ebp; if (!rawCall(app, cpu, 0x485f20, esp, 0, eax, edx, ebx, ecx)) return false;  // 487c40 call 0x485f20
    esp += 0u;  // 487c40 call 0x485f20
    ecx = (ebp + 0x66u);  // 487c45 lea ecx, [ebp + 0x66]
    ebx = (ebp - 0x26u);  // 487c48 lea ebx, [ebp - 0x26]
    edi = eax;  // 487c4b mov edi, eax
    edx = esi;  // 487c4d mov edx, esi
    eax = (ebp + 0xau);  // 487c4f lea eax, [ebp + 0xa]
    cpu.ebp = ebp; if (!rawCall(app, cpu, 0x485f20, esp, 0, eax, edx, ebx, ecx)) return false;  // 487c52 call 0x485f20
    esp += 0u;  // 487c52 call 0x485f20
    fl.sub(rd32((ebp + 0x6au)), 0u);  // 487c57 cmp dword ptr [ebp + 0x6a], 0
    if (fl.e()) goto L_487c69;  // 487c5b je 0x487c69
    fl.logic(edi & edi);  // 487c5d test edi, edi
    if (fl.e()) goto L_487c69;  // 487c5f je 0x487c69
    fl.logic(eax & eax);  // 487c61 test eax, eax
    if (fl.ne()) goto L_48801c;  // 487c63 jne 0x48801c
L_487c69:
    fl.sub(rd32((ebp + 0x52u)), 0u);  // 487c69 cmp dword ptr [ebp + 0x52], 0
    if (fl.e()) goto L_487fc1;  // 487c6d je 0x487fc1
    fl.sub(rd32((ebp + 0x56u)), 0u);  // 487c73 cmp dword ptr [ebp + 0x56], 0
    if (fl.e()) goto L_487dfa;  // 487c77 je 0x487dfa
    eax = rd32((ebp + 0x62u));  // 487c7d mov eax, dword ptr [ebp + 0x62]
    fl.sub(eax, rd32((ebp + 0x66u)));  // 487c80 cmp eax, dword ptr [ebp + 0x66]
    if (fl.ne()) goto L_487cda;  // 487c83 jne 0x487cda
    eax = rd32((ebp + 0x7eu));  // 487c85 mov eax, dword ptr [ebp + 0x7e]
    eax = rd32((eax));  // 487c88 mov eax, dword ptr [eax]
    wr32((ebp + 0x1au), eax);  // 487c8a mov dword ptr [ebp + 0x1a], eax
    eax = rd32((ebp + 0x7eu));  // 487c8d mov eax, dword ptr [ebp + 0x7e]
    eax = rd32((eax + 4u));  // 487c90 mov eax, dword ptr [eax + 4]
    wr32((ebp + 0x1eu), eax);  // 487c93 mov dword ptr [ebp + 0x1e], eax
    eax = rd32((ebp + 0x7eu));  // 487c96 mov eax, dword ptr [ebp + 0x7e]
    eax = rd32((eax + 8u));  // 487c99 mov eax, dword ptr [eax + 8]
    wr32((ebp + 0x22u), eax);  // 487c9c mov dword ptr [ebp + 0x22], eax
    eax = rd32((ebp + 0x7eu));  // 487c9f mov eax, dword ptr [ebp + 0x7e]
    eax = rd32((eax + 0xcu));  // 487ca2 mov eax, dword ptr [eax + 0xc]
    wr32((ebp + 0x26u), eax);  // 487ca5 mov dword ptr [ebp + 0x26], eax
    eax = rd32((ebp - 0x2eu));  // 487ca8 mov eax, dword ptr [ebp - 0x2e]
    wr32((ebp + 0x2au), eax);  // 487cab mov dword ptr [ebp + 0x2a], eax
    eax = rd32((ebp - 0x2au));  // 487cae mov eax, dword ptr [ebp - 0x2a]
    wr32((ebp + 0x2eu), eax);  // 487cb1 mov dword ptr [ebp + 0x2e], eax
    eax = rd32((ebp - 0x26u));  // 487cb4 mov eax, dword ptr [ebp - 0x26]
    wr32((ebp + 0x32u), eax);  // 487cb7 mov dword ptr [ebp + 0x32], eax
    eax = rd32((ebp - 0x22u));  // 487cba mov eax, dword ptr [ebp - 0x22]
    wr32((ebp + 0x36u), eax);  // 487cbd mov dword ptr [ebp + 0x36], eax
    eax = rd32((ebp + 0x6eu));  // 487cc0 mov eax, dword ptr [ebp + 0x6e]
    esp -= 4; wr32(esp, eax);  // 487cc3 push eax
    edx = rd32((ebp + 0x72u));  // 487cc4 mov edx, dword ptr [ebp + 0x72]
    esp -= 4; wr32(esp, edx);  // 487cc7 push edx
    ecx = 8u;  // 487cc8 mov ecx, 8
    esp -= 32u;  // 487ccd sub esp, 0x20
    esi = (ebp + 0x1au);  // 487cd0 lea esi, [ebp + 0x1a]
    edi = esp;  // 487cd3 mov edi, esp
    goto L_487fb0;  // 487cd5 jmp 0x487fb0
L_487cda:
    fl.logic(rd8((ebp + 0x62u)) & 1u);  // 487cda test byte ptr [ebp + 0x62], 1
    if (fl.e()) goto L_487ce6;  // 487cde je 0x487ce6
    fl.logic(rd8((ebp + 0x66u)) & 4u);  // 487ce0 test byte ptr [ebp + 0x66], 4
    if (fl.ne()) goto L_487cf2;  // 487ce4 jne 0x487cf2
L_487ce6:
    fl.logic(rd8((ebp + 0x66u)) & 1u);  // 487ce6 test byte ptr [ebp + 0x66], 1
    if (fl.e()) goto L_487cff;  // 487cea je 0x487cff
    fl.logic(rd8((ebp + 0x62u)) & 4u);  // 487cec test byte ptr [ebp + 0x62], 4
    if (fl.e()) goto L_487cff;  // 487cf0 je 0x487cff
L_487cf2:
    eax = rd32((esi + 4u));  // 487cf2 mov eax, dword ptr [esi + 4]
    wr32((ebp + 0x7au), eax);  // 487cf5 mov dword ptr [ebp + 0x7a], eax
    eax = rd32((esi));  // 487cf8 mov eax, dword ptr [esi]
    goto L_487d5f;  // 487cfa jmp 0x487d5f
L_487cff:
    fl.logic(rd8((ebp + 0x62u)) & 2u);  // 487cff test byte ptr [ebp + 0x62], 2
    if (fl.e()) goto L_487d0b;  // 487d03 je 0x487d0b
    fl.logic(rd8((ebp + 0x66u)) & 4u);  // 487d05 test byte ptr [ebp + 0x66], 4
    if (fl.ne()) goto L_487d17;  // 487d09 jne 0x487d17
L_487d0b:
    fl.logic(rd8((ebp + 0x66u)) & 2u);  // 487d0b test byte ptr [ebp + 0x66], 2
    if (fl.e()) goto L_487d1c;  // 487d0f je 0x487d1c
    fl.logic(rd8((ebp + 0x62u)) & 4u);  // 487d11 test byte ptr [ebp + 0x62], 4
    if (fl.e()) goto L_487d1c;  // 487d15 je 0x487d1c
L_487d17:
    eax = rd32((esi + 4u));  // 487d17 mov eax, dword ptr [esi + 4]
    goto L_487d59;  // 487d1a jmp 0x487d59
L_487d1c:
    fl.logic(rd8((ebp + 0x62u)) & 1u);  // 487d1c test byte ptr [ebp + 0x62], 1
    if (fl.e()) goto L_487d28;  // 487d20 je 0x487d28
    fl.logic(rd8((ebp + 0x66u)) & 8u);  // 487d22 test byte ptr [ebp + 0x66], 8
    if (fl.ne()) goto L_487d34;  // 487d26 jne 0x487d34
L_487d28:
    fl.logic(rd8((ebp + 0x66u)) & 1u);  // 487d28 test byte ptr [ebp + 0x66], 1
    if (fl.e()) goto L_487d3e;  // 487d2c je 0x487d3e
    fl.logic(rd8((ebp + 0x62u)) & 8u);  // 487d2e test byte ptr [ebp + 0x62], 8
    if (fl.e()) goto L_487d3e;  // 487d32 je 0x487d3e
L_487d34:
    eax = rd32((esi + 0xcu));  // 487d34 mov eax, dword ptr [esi + 0xc]
    wr32((ebp + 0x7au), eax);  // 487d37 mov dword ptr [ebp + 0x7a], eax
    eax = rd32((esi));  // 487d3a mov eax, dword ptr [esi]
    goto L_487d5f;  // 487d3c jmp 0x487d5f
L_487d3e:
    fl.logic(rd8((ebp + 0x62u)) & 2u);  // 487d3e test byte ptr [ebp + 0x62], 2
    if (fl.e()) goto L_487d4a;  // 487d42 je 0x487d4a
    fl.logic(rd8((ebp + 0x66u)) & 8u);  // 487d44 test byte ptr [ebp + 0x66], 8
    if (fl.ne()) goto L_487d56;  // 487d48 jne 0x487d56
L_487d4a:
    fl.logic(rd8((ebp + 0x66u)) & 2u);  // 487d4a test byte ptr [ebp + 0x66], 2
    if (fl.e()) goto L_487d62;  // 487d4e je 0x487d62
    fl.logic(rd8((ebp + 0x62u)) & 8u);  // 487d50 test byte ptr [ebp + 0x62], 8
    if (fl.e()) goto L_487d62;  // 487d54 je 0x487d62
L_487d56:
    eax = rd32((esi + 0xcu));  // 487d56 mov eax, dword ptr [esi + 0xc]
L_487d59:
    wr32((ebp + 0x7au), eax);  // 487d59 mov dword ptr [ebp + 0x7a], eax
    eax = rd32((esi + 8u));  // 487d5c mov eax, dword ptr [esi + 8]
L_487d5f:
    wr32((ebp + 0x76u), eax);  // 487d5f mov dword ptr [ebp + 0x76], eax
L_487d62:
    eax = rd32((ebp + 0x7eu));  // 487d62 mov eax, dword ptr [ebp + 0x7e]
    eax = rd32((eax));  // 487d65 mov eax, dword ptr [eax]
    wr32((ebp + 0x1au), eax);  // 487d67 mov dword ptr [ebp + 0x1a], eax
    eax = rd32((ebp + 0x7eu));  // 487d6a mov eax, dword ptr [ebp + 0x7e]
    eax = rd32((eax + 4u));  // 487d6d mov eax, dword ptr [eax + 4]
    wr32((ebp + 0x1eu), eax);  // 487d70 mov dword ptr [ebp + 0x1e], eax
    eax = rd32((ebp + 0x7eu));  // 487d73 mov eax, dword ptr [ebp + 0x7e]
    eax = rd32((eax + 8u));  // 487d76 mov eax, dword ptr [eax + 8]
    ecx = rd32((ebp + 0x6eu));  // 487d79 mov ecx, dword ptr [ebp + 0x6e]
    wr32((ebp + 0x22u), eax);  // 487d7c mov dword ptr [ebp + 0x22], eax
    eax = rd32((ebp + 0x7eu));  // 487d7f mov eax, dword ptr [ebp + 0x7e]
    ebx = rd32((ebp + 0x72u));  // 487d82 mov ebx, dword ptr [ebp + 0x72]
    esi = (ebp + 0x1au);  // 487d85 lea esi, [ebp + 0x1a]
    eax = rd32((eax + 0xcu));  // 487d88 mov eax, dword ptr [eax + 0xc]
    esp -= 4; wr32(esp, ecx);  // 487d8b push ecx
    wr32((ebp + 0x26u), eax);  // 487d8c mov dword ptr [ebp + 0x26], eax
    eax = rd32((ebp - 0x2eu));  // 487d8f mov eax, dword ptr [ebp - 0x2e]
    ecx = 8u;  // 487d92 mov ecx, 8
    wr32((ebp + 0x2au), eax);  // 487d97 mov dword ptr [ebp + 0x2a], eax
    eax = rd32((ebp - 0x2au));  // 487d9a mov eax, dword ptr [ebp - 0x2a]
    esp -= 4; wr32(esp, ebx);  // 487d9d push ebx
    wr32((ebp + 0x2eu), eax);  // 487d9e mov dword ptr [ebp + 0x2e], eax
    eax = rd32((ebp + 0x7au));  // 487da1 mov eax, dword ptr [ebp + 0x7a]
    esp -= 32u;  // 487da4 sub esp, 0x20
    wr32((ebp + 0x32u), eax);  // 487da7 mov dword ptr [ebp + 0x32], eax
    eax = rd32((ebp + 0x76u));  // 487daa mov eax, dword ptr [ebp + 0x76]
    edi = esp;  // 487dad mov edi, esp
    wr32((ebp + 0x36u), eax);  // 487daf mov dword ptr [ebp + 0x36], eax
    for (; ecx != 0; --ecx) { wr32(edi, rd32(esi)); esi += 4; edi += 4; }  // 487db2 rep movsd dword ptr es:[edi], dword ptr [esi]
    cpu.ebp = ebp; if (!rawCall(app, cpu, 0x486f70, esp, 20, eax, edx, ebx, ecx)) return false;  // 487db4 call 0x486f70
    esp += 80u;  // 487db4 call 0x486f70
    eax = rd32((ebp + 0x7eu));  // 487db9 mov eax, dword ptr [ebp + 0x7e]
    eax = rd32((eax));  // 487dbc mov eax, dword ptr [eax]
    wr32((ebp + 0x3au), eax);  // 487dbe mov dword ptr [ebp + 0x3a], eax
    eax = rd32((ebp + 0x7eu));  // 487dc1 mov eax, dword ptr [ebp + 0x7e]
    eax = rd32((eax + 4u));  // 487dc4 mov eax, dword ptr [eax + 4]
    wr32((ebp + 0x3eu), eax);  // 487dc7 mov dword ptr [ebp + 0x3e], eax
    eax = rd32((ebp + 0x7au));  // 487dca mov eax, dword ptr [ebp + 0x7a]
    wr32((ebp + 0x42u), eax);  // 487dcd mov dword ptr [ebp + 0x42], eax
    eax = rd32((ebp + 0x76u));  // 487dd0 mov eax, dword ptr [ebp + 0x76]
    wr32((ebp + 0x46u), eax);  // 487dd3 mov dword ptr [ebp + 0x46], eax
    eax = rd32((ebp - 0x26u));  // 487dd6 mov eax, dword ptr [ebp - 0x26]
    edx = rd32((ebp + 0x72u));  // 487dd9 mov edx, dword ptr [ebp + 0x72]
    wr32((ebp + 0x4au), eax);  // 487ddc mov dword ptr [ebp + 0x4a], eax
    eax = rd32((ebp - 0x22u));  // 487ddf mov eax, dword ptr [ebp - 0x22]
    ebx = rd32((ebp + 0x6eu));  // 487de2 mov ebx, dword ptr [ebp + 0x6e]
    wr32((ebp + 0x4eu), eax);  // 487de5 mov dword ptr [ebp + 0x4e], eax
    eax = (ebp + 0x3au);  // 487de8 lea eax, [ebp + 0x3a]
    cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4870c0, esp, 0, eax, edx, ebx, ecx)) return false;  // 487deb call 0x4870c0
    esp += 0u;  // 487deb call 0x4870c0
    esp = (ebp + 0x82u);  // 487df0 lea esp, [ebp + 0x82]
    ebp = rd32(esp); esp += 4;  // 487df6 pop ebp
    edi = rd32(esp); esp += 4;  // 487df7 pop edi
    esi = rd32(esp); esp += 4;  // 487df8 pop esi
    result = eax; popped_ = 0u; return true;  // 487df9 ret
L_487dfa:
    eax = rd32((ebp + 0x5eu));  // 487dfa mov eax, dword ptr [ebp + 0x5e]
    fl.sub(eax, rd32((ebp + 0x66u)));  // 487dfd cmp eax, dword ptr [ebp + 0x66]
    if (fl.ne()) goto L_487e43;  // 487e00 jne 0x487e43
    eax = rd32((ebp + 0x7eu));  // 487e02 mov eax, dword ptr [ebp + 0x7e]
    eax = rd32((eax));  // 487e05 mov eax, dword ptr [eax]
    wr32((ebp + 0x3au), eax);  // 487e07 mov dword ptr [ebp + 0x3a], eax
    eax = rd32((ebp + 0x7eu));  // 487e0a mov eax, dword ptr [ebp + 0x7e]
    eax = rd32((eax + 4u));  // 487e0d mov eax, dword ptr [eax + 4]
    wr32((ebp + 0x3eu), eax);  // 487e10 mov dword ptr [ebp + 0x3e], eax
    eax = rd32((ebp - 0x3eu));  // 487e13 mov eax, dword ptr [ebp - 0x3e]
    wr32((ebp + 0x42u), eax);  // 487e16 mov dword ptr [ebp + 0x42], eax
    eax = rd32((ebp - 0x3au));  // 487e19 mov eax, dword ptr [ebp - 0x3a]
    wr32((ebp + 0x46u), eax);  // 487e1c mov dword ptr [ebp + 0x46], eax
    eax = rd32((ebp - 0x26u));  // 487e1f mov eax, dword ptr [ebp - 0x26]
    ebx = rd32((ebp + 0x6eu));  // 487e22 mov ebx, dword ptr [ebp + 0x6e]
    wr32((ebp + 0x4au), eax);  // 487e25 mov dword ptr [ebp + 0x4a], eax
    eax = rd32((ebp - 0x22u));  // 487e28 mov eax, dword ptr [ebp - 0x22]
    edx = rd32((ebp + 0x72u));  // 487e2b mov edx, dword ptr [ebp + 0x72]
    wr32((ebp + 0x4eu), eax);  // 487e2e mov dword ptr [ebp + 0x4e], eax
    eax = (ebp + 0x3au);  // 487e31 lea eax, [ebp + 0x3a]
    cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4870c0, esp, -3, eax, edx, ebx, ecx)) return false;  // 487e34 call 0x4870c0
    esp += -12u;  // 487e34 call 0x4870c0
    esp = (ebp + 0x82u);  // 487e39 lea esp, [ebp + 0x82]
    ebp = rd32(esp); esp += 4;  // 487e3f pop ebp
    edi = rd32(esp); esp += 4;  // 487e40 pop edi
    esi = rd32(esp); esp += 4;  // 487e41 pop esi
    result = eax; popped_ = 0u; return true;  // 487e42 ret
L_487e43:
    fl.sub(rd32((ebp + 0x62u)), 0u);  // 487e43 cmp dword ptr [ebp + 0x62], 0
    if (fl.e()) goto L_487ede;  // 487e47 je 0x487ede
    eax = rd32((ebp + 0x7eu));  // 487e4d mov eax, dword ptr [ebp + 0x7e]
    eax = rd32((eax));  // 487e50 mov eax, dword ptr [eax]
    wr32((ebp + 0x1au), eax);  // 487e52 mov dword ptr [ebp + 0x1a], eax
    eax = rd32((ebp + 0x7eu));  // 487e55 mov eax, dword ptr [ebp + 0x7e]
    edx = rd32((ebp + 0x6eu));  // 487e58 mov edx, dword ptr [ebp + 0x6e]
    eax = rd32((eax + 4u));  // 487e5b mov eax, dword ptr [eax + 4]
    ecx = rd32((ebp + 0x72u));  // 487e5e mov ecx, dword ptr [ebp + 0x72]
    wr32((ebp + 0x1eu), eax);  // 487e61 mov dword ptr [ebp + 0x1e], eax
    eax = rd32((ebp - 0x3eu));  // 487e64 mov eax, dword ptr [ebp - 0x3e]
    esi = (ebp + 0x1au);  // 487e67 lea esi, [ebp + 0x1a]
    wr32((ebp + 0x22u), eax);  // 487e6a mov dword ptr [ebp + 0x22], eax
    eax = rd32((ebp - 0x3au));  // 487e6d mov eax, dword ptr [ebp - 0x3a]
    esp -= 4; wr32(esp, edx);  // 487e70 push edx
    wr32((ebp + 0x26u), eax);  // 487e71 mov dword ptr [ebp + 0x26], eax
    eax = rd32((ebp - 0x36u));  // 487e74 mov eax, dword ptr [ebp - 0x36]
    esp -= 4; wr32(esp, ecx);  // 487e77 push ecx
    wr32((ebp + 0x2au), eax);  // 487e78 mov dword ptr [ebp + 0x2a], eax
    eax = rd32((ebp - 0x32u));  // 487e7b mov eax, dword ptr [ebp - 0x32]
    ecx = 8u;  // 487e7e mov ecx, 8
    wr32((ebp + 0x2eu), eax);  // 487e83 mov dword ptr [ebp + 0x2e], eax
    eax = rd32((ebp - 0x2eu));  // 487e86 mov eax, dword ptr [ebp - 0x2e]
    esp -= 32u;  // 487e89 sub esp, 0x20
    wr32((ebp + 0x32u), eax);  // 487e8c mov dword ptr [ebp + 0x32], eax
    eax = rd32((ebp - 0x2au));  // 487e8f mov eax, dword ptr [ebp - 0x2a]
    edi = esp;  // 487e92 mov edi, esp
    wr32((ebp + 0x36u), eax);  // 487e94 mov dword ptr [ebp + 0x36], eax
    for (; ecx != 0; --ecx) { wr32(edi, rd32(esi)); esi += 4; edi += 4; }  // 487e97 rep movsd dword ptr es:[edi], dword ptr [esi]
    cpu.ebp = ebp; if (!rawCall(app, cpu, 0x486f70, esp, 7, eax, edx, ebx, ecx)) return false;  // 487e99 call 0x486f70
    esp += 28u;  // 487e99 call 0x486f70
    eax = rd32((ebp + 0x7eu));  // 487e9e mov eax, dword ptr [ebp + 0x7e]
    eax = rd32((eax));  // 487ea1 mov eax, dword ptr [eax]
    wr32((ebp + 0x3au), eax);  // 487ea3 mov dword ptr [ebp + 0x3a], eax
    eax = rd32((ebp + 0x7eu));  // 487ea6 mov eax, dword ptr [ebp + 0x7e]
    eax = rd32((eax + 4u));  // 487ea9 mov eax, dword ptr [eax + 4]
    wr32((ebp + 0x3eu), eax);  // 487eac mov dword ptr [ebp + 0x3e], eax
    eax = rd32((ebp - 0x2eu));  // 487eaf mov eax, dword ptr [ebp - 0x2e]
    wr32((ebp + 0x42u), eax);  // 487eb2 mov dword ptr [ebp + 0x42], eax
    eax = rd32((ebp - 0x2au));  // 487eb5 mov eax, dword ptr [ebp - 0x2a]
    wr32((ebp + 0x46u), eax);  // 487eb8 mov dword ptr [ebp + 0x46], eax
    eax = rd32((ebp - 0x26u));  // 487ebb mov eax, dword ptr [ebp - 0x26]
    ebx = edx;  // 487ebe mov ebx, edx
    wr32((ebp + 0x4au), eax);  // 487ec0 mov dword ptr [ebp + 0x4a], eax
    eax = rd32((ebp - 0x22u));  // 487ec3 mov eax, dword ptr [ebp - 0x22]
    edx = rd32((ebp + 0x72u));  // 487ec6 mov edx, dword ptr [ebp + 0x72]
    wr32((ebp + 0x4eu), eax);  // 487ec9 mov dword ptr [ebp + 0x4e], eax
    eax = (ebp + 0x3au);  // 487ecc lea eax, [ebp + 0x3a]
    cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4870c0, esp, 0, eax, edx, ebx, ecx)) return false;  // 487ecf call 0x4870c0
    esp += 0u;  // 487ecf call 0x4870c0
    esp = (ebp + 0x82u);  // 487ed4 lea esp, [ebp + 0x82]
    ebp = rd32(esp); esp += 4;  // 487eda pop ebp
    edi = rd32(esp); esp += 4;  // 487edb pop edi
    esi = rd32(esp); esp += 4;  // 487edc pop esi
    result = eax; popped_ = 0u; return true;  // 487edd ret
L_487ede:
    fl.logic(rd8((ebp + 0x5eu)) & 1u);  // 487ede test byte ptr [ebp + 0x5e], 1
    if (fl.e()) goto L_487eea;  // 487ee2 je 0x487eea
    fl.logic(rd8((ebp + 0x66u)) & 4u);  // 487ee4 test byte ptr [ebp + 0x66], 4
    if (fl.ne()) goto L_487ef6;  // 487ee8 jne 0x487ef6
L_487eea:
    fl.logic(rd8((ebp + 0x66u)) & 1u);  // 487eea test byte ptr [ebp + 0x66], 1
    if (fl.e()) goto L_487f03;  // 487eee je 0x487f03
    fl.logic(rd8((ebp + 0x5eu)) & 4u);  // 487ef0 test byte ptr [ebp + 0x5e], 4
    if (fl.e()) goto L_487f03;  // 487ef4 je 0x487f03
L_487ef6:
    eax = rd32((esi + 4u));  // 487ef6 mov eax, dword ptr [esi + 4]
    wr32((ebp + 0x7au), eax);  // 487ef9 mov dword ptr [ebp + 0x7a], eax
    eax = rd32((esi));  // 487efc mov eax, dword ptr [esi]
    goto L_487f63;  // 487efe jmp 0x487f63
L_487f03:
    fl.logic(rd8((ebp + 0x5eu)) & 2u);  // 487f03 test byte ptr [ebp + 0x5e], 2
    if (fl.e()) goto L_487f0f;  // 487f07 je 0x487f0f
    fl.logic(rd8((ebp + 0x66u)) & 4u);  // 487f09 test byte ptr [ebp + 0x66], 4
    if (fl.ne()) goto L_487f1b;  // 487f0d jne 0x487f1b
L_487f0f:
    fl.logic(rd8((ebp + 0x66u)) & 2u);  // 487f0f test byte ptr [ebp + 0x66], 2
    if (fl.e()) goto L_487f20;  // 487f13 je 0x487f20
    fl.logic(rd8((ebp + 0x5eu)) & 4u);  // 487f15 test byte ptr [ebp + 0x5e], 4
    if (fl.e()) goto L_487f20;  // 487f19 je 0x487f20
L_487f1b:
    eax = rd32((esi + 4u));  // 487f1b mov eax, dword ptr [esi + 4]
    goto L_487f5d;  // 487f1e jmp 0x487f5d
L_487f20:
    fl.logic(rd8((ebp + 0x5eu)) & 1u);  // 487f20 test byte ptr [ebp + 0x5e], 1
    if (fl.e()) goto L_487f2c;  // 487f24 je 0x487f2c
    fl.logic(rd8((ebp + 0x66u)) & 8u);  // 487f26 test byte ptr [ebp + 0x66], 8
    if (fl.ne()) goto L_487f38;  // 487f2a jne 0x487f38
L_487f2c:
    fl.logic(rd8((ebp + 0x66u)) & 1u);  // 487f2c test byte ptr [ebp + 0x66], 1
    if (fl.e()) goto L_487f42;  // 487f30 je 0x487f42
    fl.logic(rd8((ebp + 0x5eu)) & 8u);  // 487f32 test byte ptr [ebp + 0x5e], 8
    if (fl.e()) goto L_487f42;  // 487f36 je 0x487f42
L_487f38:
    eax = rd32((esi + 0xcu));  // 487f38 mov eax, dword ptr [esi + 0xc]
    wr32((ebp + 0x7au), eax);  // 487f3b mov dword ptr [ebp + 0x7a], eax
    eax = rd32((esi));  // 487f3e mov eax, dword ptr [esi]
    goto L_487f63;  // 487f40 jmp 0x487f63
L_487f42:
    fl.logic(rd8((ebp + 0x5eu)) & 2u);  // 487f42 test byte ptr [ebp + 0x5e], 2
    if (fl.e()) goto L_487f4e;  // 487f46 je 0x487f4e
    fl.logic(rd8((ebp + 0x66u)) & 8u);  // 487f48 test byte ptr [ebp + 0x66], 8
    if (fl.ne()) goto L_487f5a;  // 487f4c jne 0x487f5a
L_487f4e:
    fl.logic(rd8((ebp + 0x66u)) & 2u);  // 487f4e test byte ptr [ebp + 0x66], 2
    if (fl.e()) goto L_487f66;  // 487f52 je 0x487f66
    fl.logic(rd8((ebp + 0x5eu)) & 8u);  // 487f54 test byte ptr [ebp + 0x5e], 8
    if (fl.e()) goto L_487f66;  // 487f58 je 0x487f66
L_487f5a:
    eax = rd32((esi + 0xcu));  // 487f5a mov eax, dword ptr [esi + 0xc]
L_487f5d:
    wr32((ebp + 0x7au), eax);  // 487f5d mov dword ptr [ebp + 0x7a], eax
    eax = rd32((esi + 8u));  // 487f60 mov eax, dword ptr [esi + 8]
L_487f63:
    wr32((ebp + 0x76u), eax);  // 487f63 mov dword ptr [ebp + 0x76], eax
L_487f66:
    eax = rd32((ebp + 0x7eu));  // 487f66 mov eax, dword ptr [ebp + 0x7e]
    eax = rd32((eax));  // 487f69 mov eax, dword ptr [eax]
    wr32((ebp + 0x1au), eax);  // 487f6b mov dword ptr [ebp + 0x1a], eax
    eax = rd32((ebp + 0x7eu));  // 487f6e mov eax, dword ptr [ebp + 0x7e]
    ebx = rd32((ebp + 0x6eu));  // 487f71 mov ebx, dword ptr [ebp + 0x6e]
    eax = rd32((eax + 4u));  // 487f74 mov eax, dword ptr [eax + 4]
    esi = rd32((ebp + 0x72u));  // 487f77 mov esi, dword ptr [ebp + 0x72]
    wr32((ebp + 0x1eu), eax);  // 487f7a mov dword ptr [ebp + 0x1e], eax
    eax = rd32((ebp - 0x3eu));  // 487f7d mov eax, dword ptr [ebp - 0x3e]
    ecx = 8u;  // 487f80 mov ecx, 8
    wr32((ebp + 0x22u), eax);  // 487f85 mov dword ptr [ebp + 0x22], eax
    eax = rd32((ebp - 0x3au));  // 487f88 mov eax, dword ptr [ebp - 0x3a]
    esp -= 4; wr32(esp, ebx);  // 487f8b push ebx
    wr32((ebp + 0x26u), eax);  // 487f8c mov dword ptr [ebp + 0x26], eax
    eax = rd32((ebp + 0x7au));  // 487f8f mov eax, dword ptr [ebp + 0x7a]
    esp -= 4; wr32(esp, esi);  // 487f92 push esi
    wr32((ebp + 0x2au), eax);  // 487f93 mov dword ptr [ebp + 0x2a], eax
    eax = rd32((ebp + 0x76u));  // 487f96 mov eax, dword ptr [ebp + 0x76]
    esi = (ebp + 0x1au);  // 487f99 lea esi, [ebp + 0x1a]
    wr32((ebp + 0x2eu), eax);  // 487f9c mov dword ptr [ebp + 0x2e], eax
    eax = rd32((ebp - 0x26u));  // 487f9f mov eax, dword ptr [ebp - 0x26]
    esp -= 32u;  // 487fa2 sub esp, 0x20
    wr32((ebp + 0x32u), eax);  // 487fa5 mov dword ptr [ebp + 0x32], eax
    eax = rd32((ebp - 0x22u));  // 487fa8 mov eax, dword ptr [ebp - 0x22]
    edi = esp;  // 487fab mov edi, esp
    wr32((ebp + 0x36u), eax);  // 487fad mov dword ptr [ebp + 0x36], eax
L_487fb0:
    for (; ecx != 0; --ecx) { wr32(edi, rd32(esi)); esi += 4; edi += 4; }  // 487fb0 rep movsd dword ptr es:[edi], dword ptr [esi]
    cpu.ebp = ebp; if (!rawCall(app, cpu, 0x486f70, esp, 7, eax, edx, ebx, ecx)) return false;  // 487fb2 call 0x486f70
    esp += 28u;  // 487fb2 call 0x486f70
    esp = (ebp + 0x82u);  // 487fb7 lea esp, [ebp + 0x82]
    ebp = rd32(esp); esp += 4;  // 487fbd pop ebp
    edi = rd32(esp); esp += 4;  // 487fbe pop edi
    esi = rd32(esp); esp += 4;  // 487fbf pop esi
    result = eax; popped_ = 0u; return true;  // 487fc0 ret
L_487fc1:
    fl.sub(rd32((ebp + 0x5eu)), 0u);  // 487fc1 cmp dword ptr [ebp + 0x5e], 0
    if (fl.ne()) goto L_48801c;  // 487fc5 jne 0x48801c
    fl.sub(rd32((ebp + 0x62u)), 0u);  // 487fc7 cmp dword ptr [ebp + 0x62], 0
    if (fl.e()) goto L_487fd7;  // 487fcb je 0x487fd7
    esp = (ebp + 0x82u);  // 487fcd lea esp, [ebp + 0x82]
    ebp = rd32(esp); esp += 4;  // 487fd3 pop ebp
    edi = rd32(esp); esp += 4;  // 487fd4 pop edi
    esi = rd32(esp); esp += 4;  // 487fd5 pop esi
    result = eax; popped_ = 0u; return true;  // 487fd6 ret
L_487fd7:
    fl.sub(rd32((ebp + 0x66u)), 0u);  // 487fd7 cmp dword ptr [ebp + 0x66], 0
    if (fl.ne()) goto L_48801c;  // 487fdb jne 0x48801c
    ecx = rd32((ebp + 0x6eu));  // 487fdd mov ecx, dword ptr [ebp + 0x6e]
    eax = rd32((esi + 4u));  // 487fe0 mov eax, dword ptr [esi + 4]
    ebx = rd32((ebp + 0x72u));  // 487fe3 mov ebx, dword ptr [ebp + 0x72]
    wr32((ebp + 0x32u), eax);  // 487fe6 mov dword ptr [ebp + 0x32], eax
    wr32((ebp + 0x1au), eax);  // 487fe9 mov dword ptr [ebp + 0x1a], eax
    esp -= 4; wr32(esp, ecx);  // 487fec push ecx
    ecx = 8u;  // 487fed mov ecx, 8
    eax = rd32((esi + 0xcu));  // 487ff2 mov eax, dword ptr [esi + 0xc]
    esp -= 4; wr32(esp, ebx);  // 487ff5 push ebx
    wr32((ebp + 0x2au), eax);  // 487ff6 mov dword ptr [ebp + 0x2a], eax
    wr32((ebp + 0x22u), eax);  // 487ff9 mov dword ptr [ebp + 0x22], eax
    esp -= 32u;  // 487ffc sub esp, 0x20
    eax = rd32((esi));  // 487fff mov eax, dword ptr [esi]
    edi = esp;  // 488001 mov edi, esp
    wr32((ebp + 0x26u), eax);  // 488003 mov dword ptr [ebp + 0x26], eax
    wr32((ebp + 0x1eu), eax);  // 488006 mov dword ptr [ebp + 0x1e], eax
    eax = rd32((esi + 8u));  // 488009 mov eax, dword ptr [esi + 8]
    esi = (ebp + 0x1au);  // 48800c lea esi, [ebp + 0x1a]
    wr32((ebp + 0x36u), eax);  // 48800f mov dword ptr [ebp + 0x36], eax
    wr32((ebp + 0x2eu), eax);  // 488012 mov dword ptr [ebp + 0x2e], eax
    for (; ecx != 0; --ecx) { wr32(edi, rd32(esi)); esi += 4; edi += 4; }  // 488015 rep movsd dword ptr es:[edi], dword ptr [esi]
    cpu.ebp = ebp; if (!rawCall(app, cpu, 0x486f70, esp, 4, eax, edx, ebx, ecx)) return false;  // 488017 call 0x486f70
    esp += 16u;  // 488017 call 0x486f70
L_48801c:
    esp = (ebp + 0x82u);  // 48801c lea esp, [ebp + 0x82]
    ebp = rd32(esp); esp += 4;  // 488022 pop ebp
    edi = rd32(esp); esp += 4;  // 488023 pop edi
    esi = rd32(esp); esp += 4;  // 488024 pop esi
    result = eax; popped_ = 0u; return true;  // 488025 ret
    return false;
}
// (popped = 0
bool decompMarkers(App* app, CPU& cpu, const Entry& e, reg32& result, reg32& popped_)
{
    reg32 eax = e.eax, ebx = e.ebx, ecx = e.ecx, edx = e.edx, esi = e.esi, edi = e.edi, ebp = e.ebp, esp = e.esp;
    Flags fl;
    X87 x;
    const auto rd32 = [&](reg32 a) { return app->getMemory<reg32>(a); };
    const auto rd16 = [&](reg32 a) { return reg32(app->getMemory<uint16_t>(a)); };
    const auto rd8 = [&](reg32 a) { return reg32(app->getMemory<uint8_t>(a)); };
    const auto wr32 = [&](reg32 a, reg32 v) { app->getMemory<reg32>(a) = v; };
    const auto wr16 = [&](reg32 a, reg32 v) { app->getMemory<uint16_t>(a) = uint16_t(v); };
    const auto wr8 = [&](reg32 a, reg32 v) { app->getMemory<uint8_t>(a) = uint8_t(v); };
    const auto F = [&](reg32 a) { return double(app->getMemory<float>(a)); };
    const auto D = [&](reg32 a) { return app->getMemory<double>(a); };
    const auto SF = [&](reg32 a, double v) { app->getMemory<float>(a) = float(v); };
    const auto SD = [&](reg32 a, double v) { app->getMemory<double>(a) = v; };
    esp -= 4; wr32(esp, esi);  // 48b5f0 push esi
    esp -= 4; wr32(esp, edi);  // 48b5f1 push edi
    esp -= 4; wr32(esp, ebp);  // 48b5f2 push ebp
    ebp = esp;  // 48b5f3 mov ebp, esp
    esp -= 300u;  // 48b5f5 sub esp, 0x12c
    { const reg32 a_ = ebp, b_ = 130u; ebp = a_ - b_; fl.sub(a_, b_); }  // 48b5fb sub ebp, 0x82
    wr32((ebp + 0x4au), eax);  // 48b601 mov dword ptr [ebp + 0x4a], eax
    esi = edx;  // 48b604 mov esi, edx
    edi = ebx;  // 48b606 mov edi, ebx
    wr32((ebp + 0xeu), ecx);  // 48b608 mov dword ptr [ebp + 0xe], ecx
    fl.sub(rd32((0x6fd4c4u)), 0u);  // 48b60b cmp dword ptr [0x6fd4c4], 0
    if (fl.e()) goto L_48b61d;  // 48b612 je 0x48b61d
    wr32((ebp + 0x1au), 3212836864u);  // 48b614 mov dword ptr [ebp + 0x1a], 0xbf800000
    goto L_48b624;  // 48b61b jmp 0x48b624
L_48b61d:
    wr32((ebp + 0x1au), 1065353216u);  // 48b61d mov dword ptr [ebp + 0x1a], 0x3f800000
L_48b624:
    eax = rd32((ebp + 0x1au));  // 48b624 mov eax, dword ptr [ebp + 0x1a]
    ecx = (ebp - 0x1eu);  // 48b627 lea ecx, [ebp - 0x1e]
    ebx = (ebp - 0xau);  // 48b62a lea ebx, [ebp - 0xa]
    edx = (ebp - 0x2au);  // 48b62d lea edx, [ebp - 0x2a]
    wr32((ebp + 0x52u), eax);  // 48b630 mov dword ptr [ebp + 0x52], eax
    eax = (ebp - 0x22u);  // 48b633 lea eax, [ebp - 0x22]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4beb80, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48b636 call 0x4beb80
    esp += 0u;  // 48b636 call 0x4beb80
    eax = rd32((ebp + 0x4au));  // 48b63b mov eax, dword ptr [ebp + 0x4a]
    edx = rd32((0x791ac8u));  // 48b63e mov edx, dword ptr [0x791ac8]
    { const reg32 r_ = eax << 5; eax = r_; fl.logic(r_); }  // 48b644 shl eax, 5
    { const reg32 a_ = eax, b_ = edx; eax = a_ + b_; fl.add(a_, b_); }  // 48b647 add eax, edx
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x47fbe0, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48b649 call 0x47fbe0
    esp += 0u;  // 48b649 call 0x47fbe0
    x.push(F(edi + 0x98u));  // 48b64e fld dword ptr [edi + 0x98]
    edx = rd32((ebp + 0x4au));  // 48b654 mov edx, dword ptr [ebp + 0x4a]
    x.st(0) = (x.st(0)) - (F(esi + 0x98u));  // 48b657 fsub dword ptr [esi + 0x98]
    eax = (edx * 4u);  // 48b65d lea eax, [edx*4]
    SF((ebp - 0x9eu), x.st(0));  // 48b664 fstp dword ptr [ebp - 0x9e]
    x.pop();  // 48b664 fstp dword ptr [ebp - 0x9e]
    { const reg32 a_ = eax, b_ = edx; eax = a_ - b_; fl.sub(a_, b_); }  // 48b66a sub eax, edx
    x.push(F(edi + 0x9cu));  // 48b66c fld dword ptr [edi + 0x9c]
    { const reg32 r_ = eax << 3; eax = r_; fl.logic(r_); }  // 48b672 shl eax, 3
    x.st(0) = (x.st(0)) - (F(esi + 0x9cu));  // 48b675 fsub dword ptr [esi + 0x9c]
    { const reg32 a_ = eax, b_ = edx; eax = a_ - b_; fl.sub(a_, b_); }  // 48b67b sub eax, edx
    SF((ebp - 0x9au), x.st(0));  // 48b67d fstp dword ptr [ebp - 0x9a]
    x.pop();  // 48b67d fstp dword ptr [ebp - 0x9a]
    { const reg32 r_ = eax << 4; eax = r_; fl.logic(r_); }  // 48b683 shl eax, 4
    x.push(F(edi + 0xa0u));  // 48b686 fld dword ptr [edi + 0xa0]
    x.push(double(int32_t(rd32((eax + 0x74987cu)))));  // 48b68c fild dword ptr [eax + 0x74987c]
    x.push(double(int32_t(rd32((eax + 0x749878u)))));  // 48b692 fild dword ptr [eax + 0x749878]
    SF((ebp + 0x6au), x.st(0));  // 48b698 fstp dword ptr [ebp + 0x6a]
    x.pop();  // 48b698 fstp dword ptr [ebp + 0x6a]
    SF((ebp + 0x7au), x.st(0));  // 48b69b fstp dword ptr [ebp + 0x7a]
    x.pop();  // 48b69b fstp dword ptr [ebp + 0x7a]
    edx = rd32((eax + 0x749884u));  // 48b69e mov edx, dword ptr [eax + 0x749884]
    ecx = rd32((eax + 0x74987cu));  // 48b6a4 mov ecx, dword ptr [eax + 0x74987c]
    ebx = rd32((eax + 0x749878u));  // 48b6aa mov ebx, dword ptr [eax + 0x749878]
    x.st(0) = (x.st(0)) - (F(esi + 0xa0u));  // 48b6b0 fsub dword ptr [esi + 0xa0]
    { const reg32 a_ = edx, b_ = ecx; edx = a_ - b_; fl.sub(a_, b_); }  // 48b6b6 sub edx, ecx
    SF((ebp - 0x96u), x.st(0));  // 48b6b8 fstp dword ptr [ebp - 0x96]
    x.pop();  // 48b6b8 fstp dword ptr [ebp - 0x96]
    wr32((ebp + 0x7eu), edx);  // 48b6be mov dword ptr [ebp + 0x7e], edx
    edx = rd32((eax + 0x749880u));  // 48b6c1 mov edx, dword ptr [eax + 0x749880]
    x.push(double(int32_t(rd32((ebp + 0x7eu)))));  // 48b6c7 fild dword ptr [ebp + 0x7e]
    { const reg32 a_ = edx, b_ = ebx; edx = a_ - b_; fl.sub(a_, b_); }  // 48b6ca sub edx, ebx
    SF((ebp + 0x76u), x.st(0));  // 48b6cc fstp dword ptr [ebp + 0x76]
    x.pop();  // 48b6cc fstp dword ptr [ebp + 0x76]
    wr32((ebp + 0x7eu), edx);  // 48b6cf mov dword ptr [ebp + 0x7e], edx
    eax = (esi + 0x190u);  // 48b6d2 lea eax, [esi + 0x190]
    x.push(double(int32_t(rd32((ebp + 0x7eu)))));  // 48b6d8 fild dword ptr [ebp + 0x7e]
    edx = (ebp - 0x9eu);  // 48b6db lea edx, [ebp - 0x9e]
    SF((ebp + 0x6eu), x.st(0));  // 48b6e1 fstp dword ptr [ebp + 0x6e]
    x.pop();  // 48b6e1 fstp dword ptr [ebp + 0x6e]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4e01f0, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48b6e4 call 0x4e01f0
    esp += 0u;  // 48b6e4 call 0x4e01f0
    edx = (ebp - 0x9eu);  // 48b6e9 lea edx, [ebp - 0x9e]
    eax = (esi + 0x1a8u);  // 48b6ef lea eax, [esi + 0x1a8]
    SF((ebp + 0x5au), x.st(0));  // 48b6f5 fstp dword ptr [ebp + 0x5a]
    x.pop();  // 48b6f5 fstp dword ptr [ebp + 0x5a]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4e01f0, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48b6f8 call 0x4e01f0
    esp += 0u;  // 48b6f8 call 0x4e01f0
    x.push(0.0);  // 48b6fd fldz
    x.fxch(1);  // 48b6ff fxch st(1)
    SF((ebp + 0x5eu), x.st(0));  // 48b701 fstp dword ptr [ebp + 0x5e]
    x.pop();  // 48b701 fstp dword ptr [ebp + 0x5e]
    fl.fcmp(x.st(0), F(ebp + 0x5au));  // 48b704 fcomp dword ptr [ebp + 0x5a]
    x.pop();  // 48b704 fcomp dword ptr [ebp + 0x5a]
    // fnstsw ax  // 48b707 fnstsw ax
    // sahf  // 48b709 sahf
    if (fl.be()) goto L_48b716;  // 48b70a jbe 0x48b716
    x.push(F(ebp + 0x5au));  // 48b70c fld dword ptr [ebp + 0x5a]
    x.st(0) = -x.st(0);  // 48b70f fchs
    SF((ebp + 0x2au), x.st(0));  // 48b711 fstp dword ptr [ebp + 0x2a]
    x.pop();  // 48b711 fstp dword ptr [ebp + 0x2a]
    goto L_48b71c;  // 48b714 jmp 0x48b71c
L_48b716:
    eax = rd32((ebp + 0x5au));  // 48b716 mov eax, dword ptr [ebp + 0x5a]
    wr32((ebp + 0x2au), eax);  // 48b719 mov dword ptr [ebp + 0x2a], eax
L_48b71c:
    x.push(F(ebp + 0x2au));  // 48b71c fld dword ptr [ebp + 0x2a]
    fl.fcmp(x.st(0), D(0x53b93cu));  // 48b71f fcomp qword ptr [0x53b93c]
    x.pop();  // 48b71f fcomp qword ptr [0x53b93c]
    // fnstsw ax  // 48b725 fnstsw ax
    // sahf  // 48b727 sahf
    if (fl.a()) goto L_48b752;  // 48b728 ja 0x48b752
    x.push(0.0);  // 48b72a fldz
    fl.fcmp(x.st(0), F(ebp + 0x5eu));  // 48b72c fcomp dword ptr [ebp + 0x5e]
    x.pop();  // 48b72c fcomp dword ptr [ebp + 0x5e]
    // fnstsw ax  // 48b72f fnstsw ax
    // sahf  // 48b731 sahf
    if (fl.be()) goto L_48b73e;  // 48b732 jbe 0x48b73e
    x.push(F(ebp + 0x5eu));  // 48b734 fld dword ptr [ebp + 0x5e]
    x.st(0) = -x.st(0);  // 48b737 fchs
    SF((ebp + 0x26u), x.st(0));  // 48b739 fstp dword ptr [ebp + 0x26]
    x.pop();  // 48b739 fstp dword ptr [ebp + 0x26]
    goto L_48b744;  // 48b73c jmp 0x48b744
L_48b73e:
    eax = rd32((ebp + 0x5eu));  // 48b73e mov eax, dword ptr [ebp + 0x5e]
    wr32((ebp + 0x26u), eax);  // 48b741 mov dword ptr [ebp + 0x26], eax
L_48b744:
    x.push(F(ebp + 0x26u));  // 48b744 fld dword ptr [ebp + 0x26]
    fl.fcmp(x.st(0), D(0x53b93cu));  // 48b747 fcomp qword ptr [0x53b93c]
    x.pop();  // 48b747 fcomp qword ptr [0x53b93c]
    // fnstsw ax  // 48b74d fnstsw ax
    // sahf  // 48b74f sahf
    if (fl.be()) goto L_48b76c;  // 48b750 jbe 0x48b76c
L_48b752:
    eax = rd32((ebp + 0x4au));  // 48b752 mov eax, dword ptr [ebp + 0x4a]
    esi = rd32((0x791ac8u));  // 48b755 mov esi, dword ptr [0x791ac8]
    { const reg32 r_ = eax << 5; eax = r_; fl.logic(r_); }  // 48b75b shl eax, 5
    { const reg32 a_ = eax, b_ = esi; eax = a_ + b_; fl.add(a_, b_); }  // 48b75e add eax, esi
    ebx = 0u;  // 48b760 xor ebx, ebx
    fl.logic(0u);  // 48b760 xor ebx, ebx
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x47fcb0, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48b762 call 0x47fcb0
    esp += 0u;  // 48b762 call 0x47fcb0
    goto L_48c0d2;  // 48b767 jmp 0x48c0d2
L_48b76c:
    x.push(F(ebp + 0x5au));  // 48b76c fld dword ptr [ebp + 0x5a]
    x.st(0) = (x.st(0)) * (x.st(0));  // 48b76f fmul st(0), st(0)
    x.push(F(ebp + 0x5eu));  // 48b771 fld dword ptr [ebp + 0x5e]
    x.st(0) = (x.st(0)) * (x.st(0));  // 48b774 fmul st(0), st(0)
    x.st(1) = (x.st(1)) + (x.st(0));  // 48b776 faddp st(1)
    x.pop();  // 48b776 faddp st(1)
    esp -= 4u;  // 48b778 sub esp, 4
    SF((esp), x.st(0));  // 48b77b fstp dword ptr [esp]
    x.pop();  // 48b77b fstp dword ptr [esp]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4ea920, esp, 1, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48b77e call 0x4ea920
    esp += 4u;  // 48b77e call 0x4ea920
    x.st(0) = (x.st(0)) * (F(0x53b944u));  // 48b783 fmul dword ptr [0x53b944]
    x.st(0) = (x.st(0)) * (D(0x53b94cu));  // 48b789 fmul qword ptr [0x53b94c]
    x.push(1.0);  // 48b78f fld1
    x.fxch(1);  // 48b791 fxch st(1)
    SD((ebp - 0x3eu), x.st(0));  // 48b793 fstp qword ptr [ebp - 0x3e]
    x.pop();  // 48b793 fstp qword ptr [ebp - 0x3e]
    fl.fcmp(x.st(0), D(ebp - 0x3eu));  // 48b796 fcomp qword ptr [ebp - 0x3e]
    x.pop();  // 48b796 fcomp qword ptr [ebp - 0x3e]
    // fnstsw ax  // 48b799 fnstsw ax
    // sahf  // 48b79b sahf
    if (fl.be()) goto L_48b7ac;  // 48b79c jbe 0x48b7ac
    eax = rd32((ebp - 0x3eu));  // 48b79e mov eax, dword ptr [ebp - 0x3e]
    wr32((ebp - 0x46u), eax);  // 48b7a1 mov dword ptr [ebp - 0x46], eax
    eax = rd32((ebp - 0x3au));  // 48b7a4 mov eax, dword ptr [ebp - 0x3a]
    wr32((ebp - 0x42u), eax);  // 48b7a7 mov dword ptr [ebp - 0x42], eax
    goto L_48b7b9;  // 48b7aa jmp 0x48b7b9
L_48b7ac:
    eax = 0u;  // 48b7ac xor eax, eax
    fl.logic(0u);  // 48b7ac xor eax, eax
    edx = 1072693248u;  // 48b7ae mov edx, 0x3ff00000
    wr32((ebp - 0x46u), eax);  // 48b7b3 mov dword ptr [ebp - 0x46], eax
    wr32((ebp - 0x42u), edx);  // 48b7b6 mov dword ptr [ebp - 0x42], edx
L_48b7b9:
    x.push(1.0);  // 48b7b9 fld1
    x.st(0) = (x.st(0)) - (D(ebp - 0x46u));  // 48b7bb fsub qword ptr [ebp - 0x46]
    x.st(0) = (x.st(0)) * (F(0x53b954u));  // 48b7be fmul dword ptr [0x53b954]
    SF((ebp + 0x42u), x.st(0));  // 48b7c4 fst dword ptr [ebp + 0x42]
    x.push(1.0);  // 48b7c7 fld1
    fl.fcmp(x.st(0), x.st(1));  // 48b7c9 fcompp
    x.pop(); x.pop();  // 48b7c9 fcompp
    // fnstsw ax  // 48b7cb fnstsw ax
    // sahf  // 48b7cd sahf
    if (fl.be()) goto L_48b7df;  // 48b7ce jbe 0x48b7df
    ecx = 0u;  // 48b7d0 xor ecx, ecx
    fl.logic(0u);  // 48b7d0 xor ecx, ecx
    ebx = 1072693248u;  // 48b7d2 mov ebx, 0x3ff00000
    wr32((ebp - 0x36u), ecx);  // 48b7d7 mov dword ptr [ebp - 0x36], ecx
    wr32((ebp - 0x32u), ebx);  // 48b7da mov dword ptr [ebp - 0x32], ebx
    goto L_48b802;  // 48b7dd jmp 0x48b802
L_48b7df:
    x.push(F(ebp + 0x42u));  // 48b7df fld dword ptr [ebp + 0x42]
    fl.fcmp(x.st(0), F(0x53b954u));  // 48b7e2 fcomp dword ptr [0x53b954]
    x.pop();  // 48b7e2 fcomp dword ptr [0x53b954]
    // fnstsw ax  // 48b7e8 fnstsw ax
    // sahf  // 48b7ea sahf
    if (fl.be()) goto L_48b7f6;  // 48b7eb jbe 0x48b7f6
    wr32((ebp + 0x2eu), 1084227584u);  // 48b7ed mov dword ptr [ebp + 0x2e], 0x40a00000
    goto L_48b7fc;  // 48b7f4 jmp 0x48b7fc
L_48b7f6:
    eax = rd32((ebp + 0x42u));  // 48b7f6 mov eax, dword ptr [ebp + 0x42]
    wr32((ebp + 0x2eu), eax);  // 48b7f9 mov dword ptr [ebp + 0x2e], eax
L_48b7fc:
    x.push(F(ebp + 0x2eu));  // 48b7fc fld dword ptr [ebp + 0x2e]
    SD((ebp - 0x36u), x.st(0));  // 48b7ff fstp qword ptr [ebp - 0x36]
    x.pop();  // 48b7ff fstp qword ptr [ebp - 0x36]
L_48b802:
    x.push(F(ebp + 0x6au));  // 48b802 fld dword ptr [ebp + 0x6a]
    x.push(F(ebp + 0x7au));  // 48b805 fld dword ptr [ebp + 0x7a]
    eax = rd32((ebp + 0x6au));  // 48b808 mov eax, dword ptr [ebp + 0x6a]
    edx = rd32((ebp + 0xeu));  // 48b80b mov edx, dword ptr [ebp + 0xe]
    wr32((ebp - 0x6eu), eax);  // 48b80e mov dword ptr [ebp - 0x6e], eax
    x.fxch(1);  // 48b811 fxch st(1)
    x.st(0) = (x.st(0)) + (F(ebp + 0x6eu));  // 48b813 fadd dword ptr [ebp + 0x6e]
    x.fxch(1);  // 48b816 fxch st(1)
    x.st(0) = (x.st(0)) + (F(ebp + 0x76u));  // 48b818 fadd dword ptr [ebp + 0x76]
    esp -= 4; wr32(esp, edx);  // 48b81b push edx
    eax = rd32((ebp + 0x7au));  // 48b81c mov eax, dword ptr [ebp + 0x7a]
    x.fxch(1);  // 48b81f fxch st(1)
    SF((ebp - 0x66u), x.st(0));  // 48b821 fstp dword ptr [ebp - 0x66]
    x.pop();  // 48b821 fstp dword ptr [ebp - 0x66]
    wr32((ebp - 0x6au), eax);  // 48b824 mov dword ptr [ebp - 0x6a], eax
    wr32((ebp - 0x62u), eax);  // 48b827 mov dword ptr [ebp - 0x62], eax
    eax = rd32((ebp - 0x66u));  // 48b82a mov eax, dword ptr [ebp - 0x66]
    SF((ebp - 0x5au), x.st(0));  // 48b82d fstp dword ptr [ebp - 0x5a]
    x.pop();  // 48b82d fstp dword ptr [ebp - 0x5a]
    wr32((ebp - 0x5eu), eax);  // 48b830 mov dword ptr [ebp - 0x5e], eax
    eax = rd32((ebp + 0x6au));  // 48b833 mov eax, dword ptr [ebp + 0x6a]
    esp -= 4; wr32(esp, 1u);  // 48b836 push 1
    wr32((ebp - 0x56u), eax);  // 48b838 mov dword ptr [ebp - 0x56], eax
    eax = rd32((ebp - 0x5au));  // 48b83b mov eax, dword ptr [ebp - 0x5a]
    esp -= 4; wr32(esp, eax);  // 48b83e push eax
    ebx = rd32((ebp - 0x56u));  // 48b83f mov ebx, dword ptr [ebp - 0x56]
    esp -= 4; wr32(esp, ebx);  // 48b842 push ebx
    esp -= 4; wr32(esp, eax);  // 48b843 push eax
    edx = rd32((ebp - 0x5eu));  // 48b844 mov edx, dword ptr [ebp - 0x5e]
    esp -= 4; wr32(esp, edx);  // 48b847 push edx
    ecx = rd32((ebp - 0x62u));  // 48b848 mov ecx, dword ptr [ebp - 0x62]
    esp -= 4; wr32(esp, ecx);  // 48b84b push ecx
    ebx = rd32((ebp - 0x66u));  // 48b84c mov ebx, dword ptr [ebp - 0x66]
    wr32((ebp - 0x52u), eax);  // 48b84f mov dword ptr [ebp - 0x52], eax
    esp -= 4; wr32(esp, ebx);  // 48b852 push ebx
    eax = rd32((ebp - 0x6au));  // 48b853 mov eax, dword ptr [ebp - 0x6a]
    edx = rd32((ebp - 0x6eu));  // 48b856 mov edx, dword ptr [ebp - 0x6e]
    esp -= 4; wr32(esp, eax);  // 48b859 push eax
    x.push(D(ebp - 0x36u));  // 48b85a fld qword ptr [ebp - 0x36]
    esp -= 4; wr32(esp, edx);  // 48b85d push edx
    SF((ebp + 0x62u), x.st(0));  // 48b85e fstp dword ptr [ebp + 0x62]
    x.pop();  // 48b85e fstp dword ptr [ebp + 0x62]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x47d1d0, esp, 10, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48b861 call 0x47d1d0
    esp += 40u;  // 48b861 call 0x47d1d0
    x.push(F(ebp + 0x76u));  // 48b866 fld dword ptr [ebp + 0x76]
    x.push(F(ebp + 0x6eu));  // 48b869 fld dword ptr [ebp + 0x6e]
    x.push(F(ebp + 0x7au));  // 48b86c fld dword ptr [ebp + 0x7a]
    x.push(F(ebp + 0x6au));  // 48b86f fld dword ptr [ebp + 0x6a]
    x.fxch(3);  // 48b872 fxch st(3)
    x.st(0) = std::trunc(x.st(0));  // __CHP  // 48b874 call 0x4dfd56
    x.fxch(2);  // 48b879 fxch st(2)
    x.st(0) = std::trunc(x.st(0));  // __CHP  // 48b87b call 0x4dfd56
    x.fxch(1);  // 48b880 fxch st(1)
    x.st(0) = std::trunc(x.st(0));  // __CHP  // 48b882 call 0x4dfd56
    x.fxch(3);  // 48b887 fxch st(3)
    x.st(0) = std::trunc(x.st(0));  // __CHP  // 48b889 call 0x4dfd56
    x.fxch(2);  // 48b88e fxch st(2)
    wr32((ebp - 0x1au), stored(cpu, x.pop()));  // 48b890 fistp dword ptr [ebp - 0x1a]
    wr32((ebp - 0x16u), stored(cpu, x.pop()));  // 48b893 fistp dword ptr [ebp - 0x16]
    x.fxch(1);  // 48b896 fxch st(1)
    wr32((ebp - 0xeu), stored(cpu, x.pop()));  // 48b898 fistp dword ptr [ebp - 0xe]
    wr32((ebp - 0x12u), stored(cpu, x.pop()));  // 48b89b fistp dword ptr [ebp - 0x12]
    ecx = rd32((ebp - 0x1au));  // 48b89e mov ecx, dword ptr [ebp - 0x1a]
    ebx = rd32((ebp - 0x16u));  // 48b8a1 mov ebx, dword ptr [ebp - 0x16]
    edx = rd32((ebp - 0xeu));  // 48b8a4 mov edx, dword ptr [ebp - 0xe]
    eax = rd32((ebp - 0x12u));  // 48b8a7 mov eax, dword ptr [ebp - 0x12]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4bebe0, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48b8aa call 0x4bebe0
    esp += 0u;  // 48b8aa call 0x4bebe0
    edx = 1u;  // 48b8af mov edx, 1
    eax = 104u;  // 48b8b4 mov eax, 0x68
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x431900, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48b8b9 call 0x431900
    esp += 0u;  // 48b8b9 call 0x431900
    x.push(F(0x53b958u));  // 48b8be fld dword ptr [0x53b958]
    x.push(F(ebp + 0x6eu));  // 48b8c4 fld dword ptr [ebp + 0x6e]
    x.st(0) = (x.st(0)) * (x.st(1));  // 48b8c7 fmul st(1)
    x.push(F(ebp + 0x76u));  // 48b8c9 fld dword ptr [ebp + 0x76]
    esp -= 4; wr32(esp, 788561664u);  // 48b8cc push 0x2f007f00
    x.st(2) = (x.st(2)) * (x.st(0));  // 48b8d1 fmulp st(2)
    x.pop();  // 48b8d1 fmulp st(2)
    esp -= 4; wr32(esp, 1u);  // 48b8d3 push 1
    x.fxch(1);  // 48b8d5 fxch st(1)
    x.st(0) = (x.st(0)) + (F(ebp + 0x7au));  // 48b8d7 fadd dword ptr [ebp + 0x7a]
    x.fxch(1);  // 48b8da fxch st(1)
    x.st(0) = (x.st(0)) + (F(ebp + 0x6au));  // 48b8dc fadd dword ptr [ebp + 0x6a]
    x.fxch(1);  // 48b8df fxch st(1)
    SF((ebp - 0x5au), x.st(0));  // 48b8e1 fstp dword ptr [ebp - 0x5a]
    x.pop();  // 48b8e1 fstp dword ptr [ebp - 0x5a]
    ecx = rd32((ebp - 0x5au));  // 48b8e4 mov ecx, dword ptr [ebp - 0x5a]
    SF((ebp - 0x5eu), x.st(0));  // 48b8e7 fstp dword ptr [ebp - 0x5e]
    x.pop();  // 48b8e7 fstp dword ptr [ebp - 0x5e]
    esp -= 4; wr32(esp, ecx);  // 48b8ea push ecx
    ebx = rd32((ebp - 0x5eu));  // 48b8eb mov ebx, dword ptr [ebp - 0x5e]
    eax = rd32((ebp + 0x6au));  // 48b8ee mov eax, dword ptr [ebp + 0x6a]
    esp -= 4; wr32(esp, ebx);  // 48b8f1 push ebx
    wr32((ebp - 0x6eu), eax);  // 48b8f2 mov dword ptr [ebp - 0x6e], eax
    eax = rd32((ebp + 0x7au));  // 48b8f5 mov eax, dword ptr [ebp + 0x7a]
    esp -= 4; wr32(esp, eax);  // 48b8f8 push eax
    edx = rd32((ebp - 0x66u));  // 48b8f9 mov edx, dword ptr [ebp - 0x66]
    esp -= 4; wr32(esp, edx);  // 48b8fc push edx
    esp -= 4; wr32(esp, eax);  // 48b8fd push eax
    ebx = rd32((ebp - 0x6eu));  // 48b8fe mov ebx, dword ptr [ebp - 0x6e]
    esp -= 4; wr32(esp, ebx);  // 48b901 push ebx
    wr32((ebp - 0x6au), eax);  // 48b902 mov dword ptr [ebp - 0x6a], eax
    wr32((ebp - 0x62u), eax);  // 48b905 mov dword ptr [ebp - 0x62], eax
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x48b2a0, esp, 8, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48b908 call 0x48b2a0
    esp += 32u;  // 48b908 call 0x48b2a0
    eax = rd32((esi + 0x1cu));  // 48b90d mov eax, dword ptr [esi + 0x1c]
    { const reg32 a_ = eax, b_ = 200u; eax = a_ - b_; fl.sub(a_, b_); }  // 48b910 sub eax, 0xc8
    fl.logic(eax & eax);  // 48b915 test eax, eax
    if (fl.ge()) goto L_48b91f;  // 48b917 jge 0x48b91f
    { const reg32 a_ = eax, b_ = rd32((0x5dd940u)); eax = a_ + b_; fl.add(a_, b_); }  // 48b919 add eax, dword ptr [0x5dd940]
L_48b91f:
    x.push(F(0x53b954u));  // 48b91f fld dword ptr [0x53b954]
    ebx = 4294966796u;  // 48b925 mov ebx, 0xfffffe0c
    ecx = eax;  // 48b92a mov ecx, eax
    esp -= 4u;  // 48b92c sub esp, 4
    wr32((esp), stored(cpu, x.pop()));  // 48b92f fistp dword ptr [esp]
    eax = rd32(esp); esp += 4;  // 48b932 pop eax
    x.push(F(ebp + 0x62u));  // 48b933 fld dword ptr [ebp + 0x62]
    edx = eax;  // 48b936 mov edx, eax
    esp -= 4u;  // 48b938 sub esp, 4
    wr32((esp), stored(cpu, x.pop()));  // 48b93b fistp dword ptr [esp]
    eax = rd32(esp); esp += 4;  // 48b93e pop eax
    { const reg32 r_ = edx + 1u; edx = r_; fl.arith(r_); }  // 48b93f inc edx
    wr32((ebp + 0x46u), ebx);  // 48b940 mov dword ptr [ebp + 0x46], ebx
    { const reg32 a_ = edx, b_ = eax; edx = a_ - b_; fl.sub(a_, b_); }  // 48b943 sub edx, eax
    ebx = 1u;  // 48b945 mov ebx, 1
    wr32((ebp + 0x3eu), edx);  // 48b94a mov dword ptr [ebp + 0x3e], edx
    goto L_48bd17;  // 48b94d jmp 0x48bd17
L_48b952:
    eax = rd32((ebp + 0x1eu));  // 48b952 mov eax, dword ptr [ebp + 0x1e]
    edx = rd32((0x5dd948u));  // 48b955 mov edx, dword ptr [0x5dd948]
    { const reg32 a_ = edx, b_ = eax; edx = a_ + b_; fl.add(a_, b_); }  // 48b95b add edx, eax
    eax = rd32((edx + 0x15u));  // 48b95d mov eax, dword ptr [edx + 0x15]
    { const reg32 r_ = reg32(int32_t(eax) >> 24); eax = r_; fl.logic(r_); }  // 48b960 sar eax, 0x18
    { const reg32 r_ = eax << 9; eax = r_; fl.logic(r_); }  // 48b963 shl eax, 9
    wr32((ebp + 0x7eu), eax);  // 48b966 mov dword ptr [ebp + 0x7e], eax
    x.push(D(0x53b95cu));  // 48b969 fld qword ptr [0x53b95c]
    x.push(double(int32_t(rd32((ebp + 0x7eu)))));  // 48b96f fild dword ptr [ebp + 0x7e]
    x.st(0) = (x.st(0)) * (x.st(1));  // 48b972 fmul st(1)
    SF((ebp - 0xaau), x.st(0));  // 48b974 fstp dword ptr [ebp - 0xaa]
    x.pop();  // 48b974 fstp dword ptr [ebp - 0xaa]
    eax = rd32((ebp - 0xaau));  // 48b97a mov eax, dword ptr [ebp - 0xaa]
    wr32((ebp - 0x86u), eax);  // 48b980 mov dword ptr [ebp - 0x86], eax
    eax = rd32((edx + 0x16u));  // 48b986 mov eax, dword ptr [edx + 0x16]
    { const reg32 r_ = reg32(int32_t(eax) >> 24); eax = r_; fl.logic(r_); }  // 48b989 sar eax, 0x18
    { const reg32 r_ = eax << 9; eax = r_; fl.logic(r_); }  // 48b98c shl eax, 9
    wr32((ebp + 0x7eu), eax);  // 48b98f mov dword ptr [ebp + 0x7e], eax
    x.push(double(int32_t(rd32((ebp + 0x7eu)))));  // 48b992 fild dword ptr [ebp + 0x7e]
    x.st(0) = (x.st(0)) * (x.st(1));  // 48b995 fmul st(1)
    SF((ebp - 0xa6u), x.st(0));  // 48b997 fstp dword ptr [ebp - 0xa6]
    x.pop();  // 48b997 fstp dword ptr [ebp - 0xa6]
    eax = rd32((ebp - 0xa6u));  // 48b99d mov eax, dword ptr [ebp - 0xa6]
    wr32((ebp - 0x82u), eax);  // 48b9a3 mov dword ptr [ebp - 0x82], eax
    eax = rd32((edx + 0x17u));  // 48b9a9 mov eax, dword ptr [edx + 0x17]
    { const reg32 r_ = reg32(int32_t(eax) >> 24); eax = r_; fl.logic(r_); }  // 48b9ac sar eax, 0x18
    { const reg32 r_ = eax << 9; eax = r_; fl.logic(r_); }  // 48b9af shl eax, 9
    wr32((ebp + 0x7eu), eax);  // 48b9b2 mov dword ptr [ebp + 0x7e], eax
    x.push(double(int32_t(rd32((ebp + 0x7eu)))));  // 48b9b5 fild dword ptr [ebp + 0x7e]
    x.st(0) = (x.st(0)) * (x.st(1));  // 48b9b8 fmul st(1)
    SF((ebp - 0xa2u), x.st(0));  // 48b9ba fstp dword ptr [ebp - 0xa2]
    x.pop();  // 48b9ba fstp dword ptr [ebp - 0xa2]
    eax = rd32((ebp - 0xa2u));  // 48b9c0 mov eax, dword ptr [ebp - 0xa2]
    wr32((ebp - 0x7eu), eax);  // 48b9c6 mov dword ptr [ebp - 0x7e], eax
    x.push(double(int32_t(rd32((edx + 0x20u)))));  // 48b9c9 fild dword ptr [edx + 0x20]
    eax = (ebp - 0xaau);  // 48b9cc lea eax, [ebp - 0xaa]
    x.st(1) = (x.st(1)) * (x.st(0));  // 48b9d2 fmulp st(1)
    x.pop();  // 48b9d2 fmulp st(1)
    esp -= 4; wr32(esp, eax);  // 48b9d4 push eax
    edx = (ebp - 0xaau);  // 48b9d5 lea edx, [ebp - 0xaa]
    esp -= 4u;  // 48b9db sub esp, 4
    x.st(0) = (x.st(0)) + (D(0x53b96cu));  // 48b9de fadd qword ptr [0x53b96c]
    eax = 1u;  // 48b9e4 mov eax, 1
    SF((esp), x.st(0));  // 48b9e9 fstp dword ptr [esp]
    x.pop();  // 48b9e9 fstp dword ptr [esp]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4e0050, esp, 2, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48b9ec call 0x4e0050
    esp += 8u;  // 48b9ec call 0x4e0050
    x.push(F(ebp - 0x7au));  // 48b9f1 fld dword ptr [ebp - 0x7a]
    x.push(F(ebp - 0x76u));  // 48b9f4 fld dword ptr [ebp - 0x76]
    x.push(F(ebp - 0x72u));  // 48b9f7 fld dword ptr [ebp - 0x72]
    eax = (ebp - 0xaau);  // 48b9fa lea eax, [ebp - 0xaa]
    x.fxch(2);  // 48ba00 fxch st(2)
    x.st(0) = (x.st(0)) + (F(ebp - 0xaau));  // 48ba02 fadd dword ptr [ebp - 0xaa]
    x.fxch(1);  // 48ba08 fxch st(1)
    x.st(0) = (x.st(0)) + (F(ebp - 0xa6u));  // 48ba0a fadd dword ptr [ebp - 0xa6]
    x.fxch(2);  // 48ba10 fxch st(2)
    x.st(0) = (x.st(0)) + (F(ebp - 0xa2u));  // 48ba12 fadd dword ptr [ebp - 0xa2]
    x.fxch(1);  // 48ba18 fxch st(1)
    SF((ebp - 0xaau), x.st(0));  // 48ba1a fstp dword ptr [ebp - 0xaa]
    x.pop();  // 48ba1a fstp dword ptr [ebp - 0xaa]
    x.fxch(1);  // 48ba20 fxch st(1)
    SF((ebp - 0xa6u), x.st(0));  // 48ba22 fstp dword ptr [ebp - 0xa6]
    x.pop();  // 48ba22 fstp dword ptr [ebp - 0xa6]
    SF((ebp - 0xa2u), x.st(0));  // 48ba28 fstp dword ptr [ebp - 0xa2]
    x.pop();  // 48ba28 fstp dword ptr [ebp - 0xa2]
    x.push(F(ebp - 0xaau));  // 48ba2e fld dword ptr [ebp - 0xaa]
    x.st(0) = (x.st(0)) - (F(esi + 0x98u));  // 48ba34 fsub dword ptr [esi + 0x98]
    x.push(F(ebp - 0xa6u));  // 48ba3a fld dword ptr [ebp - 0xa6]
    x.fxch(1);  // 48ba40 fxch st(1)
    SF((ebp - 0xaau), x.st(0));  // 48ba42 fstp dword ptr [ebp - 0xaa]
    x.pop();  // 48ba42 fstp dword ptr [ebp - 0xaa]
    x.st(0) = (x.st(0)) - (F(esi + 0x9cu));  // 48ba48 fsub dword ptr [esi + 0x9c]
    x.push(F(ebp - 0xa2u));  // 48ba4e fld dword ptr [ebp - 0xa2]
    x.fxch(1);  // 48ba54 fxch st(1)
    SF((ebp - 0xa6u), x.st(0));  // 48ba56 fstp dword ptr [ebp - 0xa6]
    x.pop();  // 48ba56 fstp dword ptr [ebp - 0xa6]
    x.st(0) = (x.st(0)) - (F(esi + 0xa0u));  // 48ba5c fsub dword ptr [esi + 0xa0]
    edx = (esi + 0x190u);  // 48ba62 lea edx, [esi + 0x190]
    SF((ebp - 0xa2u), x.st(0));  // 48ba68 fstp dword ptr [ebp - 0xa2]
    x.pop();  // 48ba68 fstp dword ptr [ebp - 0xa2]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4e01f0, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48ba6e call 0x4e01f0
    esp += 0u;  // 48ba6e call 0x4e01f0
    eax = (ebp - 0xaau);  // 48ba73 lea eax, [ebp - 0xaa]
    edx = (esi + 0x19cu);  // 48ba79 lea edx, [esi + 0x19c]
    SF((ebp - 0x92u), x.st(0));  // 48ba7f fstp dword ptr [ebp - 0x92]
    x.pop();  // 48ba7f fstp dword ptr [ebp - 0x92]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4e01f0, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48ba85 call 0x4e01f0
    esp += 0u;  // 48ba85 call 0x4e01f0
    eax = (ebp - 0xaau);  // 48ba8a lea eax, [ebp - 0xaa]
    edx = (esi + 0x1a8u);  // 48ba90 lea edx, [esi + 0x1a8]
    SF((ebp - 0x8eu), x.st(0));  // 48ba96 fstp dword ptr [ebp - 0x8e]
    x.pop();  // 48ba96 fstp dword ptr [ebp - 0x8e]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4e01f0, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48ba9c call 0x4e01f0
    esp += 0u;  // 48ba9c call 0x4e01f0
    SF((ebp - 0x8au), x.st(0));  // 48baa1 fstp dword ptr [ebp - 0x8a]
    x.pop();  // 48baa1 fstp dword ptr [ebp - 0x8a]
    x.push(F(ebp + 0x62u));  // 48baa7 fld dword ptr [ebp + 0x62]
    x.push(F(ebp - 0x92u));  // 48baaa fld dword ptr [ebp - 0x92]
    x.st(0) = (x.st(0)) * (x.st(1));  // 48bab0 fmul st(1)
    x.push(F(ebp - 0x8au));  // 48bab2 fld dword ptr [ebp - 0x8a]
    x.st(0) = -x.st(0);  // 48bab8 fchs
    x.st(2) = (x.st(2)) * (x.st(0));  // 48baba fmulp st(2)
    x.pop();  // 48baba fmulp st(2)
    x.push(F(ebp + 0x6eu));  // 48babc fld dword ptr [ebp + 0x6e]
    x.fxch(1);  // 48babf fxch st(1)
    x.st(0) = (x.st(0)) * (x.st(1));  // 48bac1 fmul st(1)
    x.fxch(2);  // 48bac3 fxch st(2)
    x.push(F(ebp + 0x76u));  // 48bac5 fld dword ptr [ebp + 0x76]
    x.fxch(1);  // 48bac8 fxch st(1)
    x.st(0) = (x.st(0)) * (x.st(1));  // 48baca fmul st(1)
    x.fxch(3);  // 48bacc fxch st(3)
    x.push(D(0x53b974u));  // 48bace fld qword ptr [0x53b974]
    x.fxch(1);  // 48bad4 fxch st(1)
    x.st(0) = (x.st(0)) * (x.st(1));  // 48bad6 fmul st(1)
    x.fxch(4);  // 48bad8 fxch st(4)
    x.st(1) = (x.st(1)) * (x.st(0));  // 48bada fmulp st(1)
    x.pop();  // 48bada fmulp st(1)
    x.push(F(ebp + 0x52u));  // 48badc fld dword ptr [ebp + 0x52]
    x.st(4) = (x.st(4)) * (x.st(0));  // 48badf fmulp st(4)
    x.pop();  // 48badf fmulp st(4)
    x.push(D(0x53b94cu));  // 48bae1 fld qword ptr [0x53b94c]
    x.fxch(1);  // 48bae7 fxch st(1)
    x.st(0) = (x.st(0)) * (x.st(1));  // 48bae9 fmul st(1)
    x.fxch(4);  // 48baeb fxch st(4)
    x.st(1) = (x.st(1)) * (x.st(0));  // 48baed fmulp st(1)
    x.pop();  // 48baed fmulp st(1)
    x.fxch(1);  // 48baef fxch st(1)
    x.push(F(0x53b958u));  // 48baf1 fld dword ptr [0x53b958]
    x.fxch(1);  // 48baf7 fxch st(1)
    x.st(0) = (x.st(0)) * (x.st(1));  // 48baf9 fmul st(1)
    x.fxch(3);  // 48bafb fxch st(3)
    x.st(1) = (x.st(1)) * (x.st(0));  // 48bafd fmulp st(1)
    x.pop();  // 48bafd fmulp st(1)
    x.fxch(2);  // 48baff fxch st(2)
    x.st(0) = (x.st(0)) + (F(ebp + 0x7au));  // 48bb01 fadd dword ptr [ebp + 0x7a]
    x.fxch(2);  // 48bb04 fxch st(2)
    x.st(0) = (x.st(0)) + (F(ebp + 0x6au));  // 48bb06 fadd dword ptr [ebp + 0x6a]
    x.fxch(2);  // 48bb09 fxch st(2)
    x.st(3) = (x.st(3)) + (x.st(0));  // 48bb0b faddp st(3)
    x.pop();  // 48bb0b faddp st(3)
    x.st(1) = (x.st(1)) + (x.st(0));  // 48bb0d faddp st(1)
    x.pop();  // 48bb0d faddp st(1)
    x.fxch(1);  // 48bb0f fxch st(1)
    SF((ebp + 0x3au), x.st(0));  // 48bb11 fstp dword ptr [ebp + 0x3a]
    x.pop();  // 48bb11 fstp dword ptr [ebp + 0x3a]
    SF((ebp + 0x36u), x.st(0));  // 48bb14 fstp dword ptr [ebp + 0x36]
    x.pop();  // 48bb14 fstp dword ptr [ebp + 0x36]
    fl.logic(ebx & ebx);  // 48bb17 test ebx, ebx
    if (fl.ne()) goto L_48bb4a;  // 48bb19 jne 0x48bb4a
    eax = rd32((ebp + 0xau));  // 48bb1b mov eax, dword ptr [ebp + 0xa]
    wr32((ebp - 0x6eu), eax);  // 48bb1e mov dword ptr [ebp - 0x6e], eax
    eax = rd32((ebp + 0x16u));  // 48bb21 mov eax, dword ptr [ebp + 0x16]
    wr32((ebp - 0x6au), eax);  // 48bb24 mov dword ptr [ebp - 0x6a], eax
    eax = rd32((ebp + 0x36u));  // 48bb27 mov eax, dword ptr [ebp + 0x36]
    esp -= 4; wr32(esp, 2399207168u);  // 48bb2a push 0x8f00ff00
    wr32((ebp - 0x66u), eax);  // 48bb2f mov dword ptr [ebp - 0x66], eax
    eax = rd32((ebp + 0x3au));  // 48bb32 mov eax, dword ptr [ebp + 0x3a]
    esp -= 4; wr32(esp, eax);  // 48bb35 push eax
    wr32((ebp - 0x62u), eax);  // 48bb36 mov dword ptr [ebp - 0x62], eax
    eax = rd32((ebp - 0x66u));  // 48bb39 mov eax, dword ptr [ebp - 0x66]
    esp -= 4; wr32(esp, eax);  // 48bb3c push eax
    edx = rd32((ebp - 0x6au));  // 48bb3d mov edx, dword ptr [ebp - 0x6a]
    esp -= 4; wr32(esp, edx);  // 48bb40 push edx
    eax = rd32((ebp - 0x6eu));  // 48bb41 mov eax, dword ptr [ebp - 0x6e]
    esp -= 4; wr32(esp, eax);  // 48bb44 push eax
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x48b4a0, esp, 5, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48bb45 call 0x48b4a0
    esp += 20u;  // 48bb45 call 0x48b4a0
L_48bb4a:
    eax = (ebp - 0x86u);  // 48bb4a lea eax, [ebp - 0x86]
    esp -= 4; wr32(esp, eax);  // 48bb50 push eax
    eax = (ecx * 8u);  // 48bb51 lea eax, [ecx*8]
    { const reg32 a_ = eax, b_ = ecx; eax = a_ + b_; fl.add(a_, b_); }  // 48bb58 add eax, ecx
    { const reg32 r_ = eax << 2; eax = r_; fl.logic(r_); }  // 48bb5a shl eax, 2
    wr32((ebp - 0x1au), eax);  // 48bb5d mov dword ptr [ebp - 0x1a], eax
    edx = rd32((ebp - 0x1au));  // 48bb60 mov edx, dword ptr [ebp - 0x1a]
    eax = rd32((0x5dd948u));  // 48bb63 mov eax, dword ptr [0x5dd948]
    { const reg32 a_ = eax, b_ = edx; eax = a_ + b_; fl.add(a_, b_); }  // 48bb68 add eax, edx
    x.push(double(int32_t(rd32((eax + 0x1cu)))));  // 48bb6a fild dword ptr [eax + 0x1c]
    x.st(0) = (x.st(0)) * (D(0x53b95cu));  // 48bb6d fmul qword ptr [0x53b95c]
    esp -= 4u;  // 48bb73 sub esp, 4
    edx = (ebp - 0x86u);  // 48bb76 lea edx, [ebp - 0x86]
    x.st(0) = (x.st(0)) + (D(0x53b96cu));  // 48bb7c fadd qword ptr [0x53b96c]
    eax = 1u;  // 48bb82 mov eax, 1
    x.st(0) = -x.st(0);  // 48bb87 fchs
    SF((esp), x.st(0));  // 48bb89 fstp dword ptr [esp]
    x.pop();  // 48bb89 fstp dword ptr [esp]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4e0050, esp, 2, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48bb8c call 0x4e0050
    esp += 8u;  // 48bb8c call 0x4e0050
    x.push(F(ebp - 0x7au));  // 48bb91 fld dword ptr [ebp - 0x7a]
    x.push(F(ebp - 0x76u));  // 48bb94 fld dword ptr [ebp - 0x76]
    x.push(F(ebp - 0x72u));  // 48bb97 fld dword ptr [ebp - 0x72]
    eax = (ebp - 0x86u);  // 48bb9a lea eax, [ebp - 0x86]
    x.fxch(2);  // 48bba0 fxch st(2)
    x.st(0) = (x.st(0)) + (F(ebp - 0x86u));  // 48bba2 fadd dword ptr [ebp - 0x86]
    x.fxch(1);  // 48bba8 fxch st(1)
    x.st(0) = (x.st(0)) + (F(ebp - 0x82u));  // 48bbaa fadd dword ptr [ebp - 0x82]
    x.fxch(2);  // 48bbb0 fxch st(2)
    x.st(0) = (x.st(0)) + (F(ebp - 0x7eu));  // 48bbb2 fadd dword ptr [ebp - 0x7e]
    x.fxch(1);  // 48bbb5 fxch st(1)
    SF((ebp - 0x86u), x.st(0));  // 48bbb7 fstp dword ptr [ebp - 0x86]
    x.pop();  // 48bbb7 fstp dword ptr [ebp - 0x86]
    x.fxch(1);  // 48bbbd fxch st(1)
    SF((ebp - 0x82u), x.st(0));  // 48bbbf fstp dword ptr [ebp - 0x82]
    x.pop();  // 48bbbf fstp dword ptr [ebp - 0x82]
    SF((ebp - 0x7eu), x.st(0));  // 48bbc5 fstp dword ptr [ebp - 0x7e]
    x.pop();  // 48bbc5 fstp dword ptr [ebp - 0x7e]
    x.push(F(ebp - 0x86u));  // 48bbc8 fld dword ptr [ebp - 0x86]
    x.st(0) = (x.st(0)) - (F(esi + 0x98u));  // 48bbce fsub dword ptr [esi + 0x98]
    x.push(F(ebp - 0x82u));  // 48bbd4 fld dword ptr [ebp - 0x82]
    x.fxch(1);  // 48bbda fxch st(1)
    SF((ebp - 0x86u), x.st(0));  // 48bbdc fstp dword ptr [ebp - 0x86]
    x.pop();  // 48bbdc fstp dword ptr [ebp - 0x86]
    x.st(0) = (x.st(0)) - (F(esi + 0x9cu));  // 48bbe2 fsub dword ptr [esi + 0x9c]
    x.push(F(ebp - 0x7eu));  // 48bbe8 fld dword ptr [ebp - 0x7e]
    x.fxch(1);  // 48bbeb fxch st(1)
    SF((ebp - 0x82u), x.st(0));  // 48bbed fstp dword ptr [ebp - 0x82]
    x.pop();  // 48bbed fstp dword ptr [ebp - 0x82]
    x.st(0) = (x.st(0)) - (F(esi + 0xa0u));  // 48bbf3 fsub dword ptr [esi + 0xa0]
    edx = (esi + 0x190u);  // 48bbf9 lea edx, [esi + 0x190]
    SF((ebp - 0x7eu), x.st(0));  // 48bbff fstp dword ptr [ebp - 0x7e]
    x.pop();  // 48bbff fstp dword ptr [ebp - 0x7e]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4e01f0, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48bc02 call 0x4e01f0
    esp += 0u;  // 48bc02 call 0x4e01f0
    eax = (ebp - 0x86u);  // 48bc07 lea eax, [ebp - 0x86]
    edx = (esi + 0x19cu);  // 48bc0d lea edx, [esi + 0x19c]
    SF((ebp - 0x92u), x.st(0));  // 48bc13 fstp dword ptr [ebp - 0x92]
    x.pop();  // 48bc13 fstp dword ptr [ebp - 0x92]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4e01f0, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48bc19 call 0x4e01f0
    esp += 0u;  // 48bc19 call 0x4e01f0
    eax = (ebp - 0x86u);  // 48bc1e lea eax, [ebp - 0x86]
    edx = (esi + 0x1a8u);  // 48bc24 lea edx, [esi + 0x1a8]
    SF((ebp - 0x8eu), x.st(0));  // 48bc2a fstp dword ptr [ebp - 0x8e]
    x.pop();  // 48bc2a fstp dword ptr [ebp - 0x8e]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4e01f0, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48bc30 call 0x4e01f0
    esp += 0u;  // 48bc30 call 0x4e01f0
    x.push(F(ebp + 0x62u));  // 48bc35 fld dword ptr [ebp + 0x62]
    x.push(F(ebp - 0x92u));  // 48bc38 fld dword ptr [ebp - 0x92]
    x.st(0) = (x.st(0)) * (x.st(1));  // 48bc3e fmul st(1)
    x.fxch(2);  // 48bc40 fxch st(2)
    SF((ebp - 0x8au), x.st(0));  // 48bc42 fst dword ptr [ebp - 0x8a]
    x.st(0) = -x.st(0);  // 48bc48 fchs
    x.st(1) = (x.st(1)) * (x.st(0));  // 48bc4a fmulp st(1)
    x.pop();  // 48bc4a fmulp st(1)
    x.fxch(1);  // 48bc4c fxch st(1)
    x.push(F(ebp + 0x6eu));  // 48bc4e fld dword ptr [ebp + 0x6e]
    x.fxch(1);  // 48bc51 fxch st(1)
    x.st(0) = (x.st(0)) * (x.st(1));  // 48bc53 fmul st(1)
    x.fxch(2);  // 48bc55 fxch st(2)
    x.push(F(ebp + 0x76u));  // 48bc57 fld dword ptr [ebp + 0x76]
    x.fxch(1);  // 48bc5a fxch st(1)
    x.st(0) = (x.st(0)) * (x.st(1));  // 48bc5c fmul st(1)
    x.fxch(3);  // 48bc5e fxch st(3)
    x.push(D(0x53b974u));  // 48bc60 fld qword ptr [0x53b974]
    x.fxch(1);  // 48bc66 fxch st(1)
    x.st(0) = (x.st(0)) * (x.st(1));  // 48bc68 fmul st(1)
    x.fxch(4);  // 48bc6a fxch st(4)
    x.st(1) = (x.st(1)) * (x.st(0));  // 48bc6c fmulp st(1)
    x.pop();  // 48bc6c fmulp st(1)
    x.push(F(ebp + 0x52u));  // 48bc6e fld dword ptr [ebp + 0x52]
    x.st(4) = (x.st(4)) * (x.st(0));  // 48bc71 fmulp st(4)
    x.pop();  // 48bc71 fmulp st(4)
    x.push(D(0x53b94cu));  // 48bc73 fld qword ptr [0x53b94c]
    x.fxch(1);  // 48bc79 fxch st(1)
    x.st(0) = (x.st(0)) * (x.st(1));  // 48bc7b fmul st(1)
    x.fxch(4);  // 48bc7d fxch st(4)
    x.st(1) = (x.st(1)) * (x.st(0));  // 48bc7f fmulp st(1)
    x.pop();  // 48bc7f fmulp st(1)
    x.fxch(1);  // 48bc81 fxch st(1)
    x.push(F(0x53b958u));  // 48bc83 fld dword ptr [0x53b958]
    x.fxch(1);  // 48bc89 fxch st(1)
    x.st(0) = (x.st(0)) * (x.st(1));  // 48bc8b fmul st(1)
    x.fxch(3);  // 48bc8d fxch st(3)
    x.st(1) = (x.st(1)) * (x.st(0));  // 48bc8f fmulp st(1)
    x.pop();  // 48bc8f fmulp st(1)
    x.fxch(2);  // 48bc91 fxch st(2)
    x.st(0) = (x.st(0)) + (F(ebp + 0x7au));  // 48bc93 fadd dword ptr [ebp + 0x7a]
    x.fxch(2);  // 48bc96 fxch st(2)
    x.st(0) = (x.st(0)) + (F(ebp + 0x6au));  // 48bc98 fadd dword ptr [ebp + 0x6a]
    x.fxch(2);  // 48bc9b fxch st(2)
    x.st(3) = (x.st(3)) + (x.st(0));  // 48bc9d faddp st(3)
    x.pop();  // 48bc9d faddp st(3)
    x.st(1) = (x.st(1)) + (x.st(0));  // 48bc9f faddp st(1)
    x.pop();  // 48bc9f faddp st(1)
    x.fxch(1);  // 48bca1 fxch st(1)
    SF((ebp + 0x22u), x.st(0));  // 48bca3 fstp dword ptr [ebp + 0x22]
    x.pop();  // 48bca3 fstp dword ptr [ebp + 0x22]
    SF((ebp + 0x32u), x.st(0));  // 48bca6 fstp dword ptr [ebp + 0x32]
    x.pop();  // 48bca6 fstp dword ptr [ebp + 0x32]
    fl.logic(ebx & ebx);  // 48bca9 test ebx, ebx
    if (fl.ne()) goto L_48bcdc;  // 48bcab jne 0x48bcdc
    eax = rd32((ebp + 6u));  // 48bcad mov eax, dword ptr [ebp + 6]
    wr32((ebp - 0x6eu), eax);  // 48bcb0 mov dword ptr [ebp - 0x6e], eax
    eax = rd32((ebp + 0x12u));  // 48bcb3 mov eax, dword ptr [ebp + 0x12]
    wr32((ebp - 0x6au), eax);  // 48bcb6 mov dword ptr [ebp - 0x6a], eax
    eax = rd32((ebp + 0x32u));  // 48bcb9 mov eax, dword ptr [ebp + 0x32]
    esp -= 4; wr32(esp, 2399207168u);  // 48bcbc push 0x8f00ff00
    wr32((ebp - 0x66u), eax);  // 48bcc1 mov dword ptr [ebp - 0x66], eax
    eax = rd32((ebp + 0x22u));  // 48bcc4 mov eax, dword ptr [ebp + 0x22]
    esp -= 4; wr32(esp, eax);  // 48bcc7 push eax
    wr32((ebp - 0x62u), eax);  // 48bcc8 mov dword ptr [ebp - 0x62], eax
    eax = rd32((ebp - 0x66u));  // 48bccb mov eax, dword ptr [ebp - 0x66]
    esp -= 4; wr32(esp, eax);  // 48bcce push eax
    edx = rd32((ebp - 0x6au));  // 48bccf mov edx, dword ptr [ebp - 0x6a]
    esp -= 4; wr32(esp, edx);  // 48bcd2 push edx
    ebx = rd32((ebp - 0x6eu));  // 48bcd3 mov ebx, dword ptr [ebp - 0x6e]
    esp -= 4; wr32(esp, ebx);  // 48bcd6 push ebx
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x48b4a0, esp, 5, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48bcd7 call 0x48b4a0
    esp += 20u;  // 48bcd7 call 0x48b4a0
L_48bcdc:
    eax = rd32((ebp + 0x3eu));  // 48bcdc mov eax, dword ptr [ebp + 0x3e]
    edx = rd32((ebp + 0x3eu));  // 48bcdf mov edx, dword ptr [ebp + 0x3e]
    ebx = 0u;  // 48bce2 xor ebx, ebx
    fl.logic(0u);  // 48bce2 xor ebx, ebx
    { const reg32 a_ = eax, b_ = ecx; eax = a_ + b_; fl.add(a_, b_); }  // 48bce4 add eax, ecx
    fl.logic(edx & edx);  // 48bce6 test edx, edx
    if (fl.l()) goto L_48bcf8;  // 48bce8 jl 0x48bcf8
    ecx = rd32((0x5dd940u));  // 48bcea mov ecx, dword ptr [0x5dd940]
    fl.sub(eax, ecx);  // 48bcf0 cmp eax, ecx
    if (fl.l()) goto L_48bd02;  // 48bcf2 jl 0x48bd02
    { const reg32 a_ = eax, b_ = ecx; eax = a_ - b_; fl.sub(a_, b_); }  // 48bcf4 sub eax, ecx
    goto L_48bd02;  // 48bcf6 jmp 0x48bd02
L_48bcf8:
    fl.logic(eax & eax);  // 48bcf8 test eax, eax
    if (fl.ge()) goto L_48bd02;  // 48bcfa jge 0x48bd02
    { const reg32 a_ = eax, b_ = rd32((0x5dd940u)); eax = a_ + b_; fl.add(a_, b_); }  // 48bcfc add eax, dword ptr [0x5dd940]
L_48bd02:
    ecx = eax;  // 48bd02 mov ecx, eax
L_48bd04:
    eax = rd32((ebp + 0x3eu));  // 48bd04 mov eax, dword ptr [ebp + 0x3e]
    { const reg32 a_ = rd32((ebp + 0x46u)), b_ = eax; wr32((ebp + 0x46u), a_ + b_); fl.add(a_, b_); }  // 48bd07 add dword ptr [ebp + 0x46], eax
    fl.sub(rd32((ebp + 0x46u)), 500u);  // 48bd0a cmp dword ptr [ebp + 0x46], 0x1f4
    if (fl.ge()) goto L_48bd9d;  // 48bd11 jge 0x48bd9d
L_48bd17:
    eax = rd32((ebp + 0x36u));  // 48bd17 mov eax, dword ptr [ebp + 0x36]
    wr32((ebp + 0xau), eax);  // 48bd1a mov dword ptr [ebp + 0xa], eax
    eax = rd32((ebp + 0x3au));  // 48bd1d mov eax, dword ptr [ebp + 0x3a]
    wr32((ebp + 0x16u), eax);  // 48bd20 mov dword ptr [ebp + 0x16], eax
    eax = rd32((ebp + 0x32u));  // 48bd23 mov eax, dword ptr [ebp + 0x32]
    wr32((ebp + 6u), eax);  // 48bd26 mov dword ptr [ebp + 6], eax
    eax = rd32((ebp + 0x22u));  // 48bd29 mov eax, dword ptr [ebp + 0x22]
    wr32((ebp + 0x12u), eax);  // 48bd2c mov dword ptr [ebp + 0x12], eax
    eax = (ecx * 8u);  // 48bd2f lea eax, [ecx*8]
    { const reg32 a_ = eax, b_ = ecx; eax = a_ + b_; fl.add(a_, b_); }  // 48bd36 add eax, ecx
    { const reg32 r_ = eax << 2; eax = r_; fl.logic(r_); }  // 48bd38 shl eax, 2
    wr32((ebp + 0x1eu), eax);  // 48bd3b mov dword ptr [ebp + 0x1e], eax
    edx = rd32((ebp + 0x1eu));  // 48bd3e mov edx, dword ptr [ebp + 0x1e]
    eax = rd32((0x5dd948u));  // 48bd41 mov eax, dword ptr [0x5dd948]
    { const reg32 a_ = eax, b_ = edx; eax = a_ + b_; fl.add(a_, b_); }  // 48bd46 add eax, edx
    x.push(D(0x53b95cu));  // 48bd48 fld qword ptr [0x53b95c]
    x.push(double(int32_t(rd32((eax)))));  // 48bd4e fild dword ptr [eax]
    x.st(0) = (x.st(0)) * (x.st(1));  // 48bd50 fmul st(1)
    SF((ebp - 0x7au), x.st(0));  // 48bd52 fstp dword ptr [ebp - 0x7a]
    x.pop();  // 48bd52 fstp dword ptr [ebp - 0x7a]
    x.push(double(int32_t(rd32((eax + 4u)))));  // 48bd55 fild dword ptr [eax + 4]
    x.st(0) = (x.st(0)) * (x.st(1));  // 48bd58 fmul st(1)
    SF((ebp - 0x76u), x.st(0));  // 48bd5a fstp dword ptr [ebp - 0x76]
    x.pop();  // 48bd5a fstp dword ptr [ebp - 0x76]
    x.push(double(int32_t(rd32((eax + 8u)))));  // 48bd5d fild dword ptr [eax + 8]
    x.st(1) = (x.st(1)) * (x.st(0));  // 48bd60 fmulp st(1)
    x.pop();  // 48bd60 fmulp st(1)
    edx = (esi + 0x98u);  // 48bd62 lea edx, [esi + 0x98]
    eax = (ebp - 0x7au);  // 48bd68 lea eax, [ebp - 0x7a]
    SF((ebp - 0x72u), x.st(0));  // 48bd6b fstp dword ptr [ebp - 0x72]
    x.pop();  // 48bd6b fstp dword ptr [ebp - 0x72]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4965b0, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48bd6e call 0x4965b0
    esp += 0u;  // 48bd6e call 0x4965b0
    fl.fcmp(x.st(0), D(0x53b964u));  // 48bd73 fcomp qword ptr [0x53b964]
    x.pop();  // 48bd73 fcomp qword ptr [0x53b964]
    // fnstsw ax  // 48bd79 fnstsw ax
    // sahf  // 48bd7b sahf
    if (fl.be()) goto L_48b952;  // 48bd7c jbe 0x48b952
    ebx = 1u;  // 48bd82 mov ebx, 1
    eax = rd32((0x5dd940u));  // 48bd87 mov eax, dword ptr [0x5dd940]
    { const reg32 a_ = ecx, b_ = ebx; ecx = a_ + b_; fl.add(a_, b_); }  // 48bd8c add ecx, ebx
    fl.sub(ecx, eax);  // 48bd8e cmp ecx, eax
    if (fl.l()) goto L_48bd04;  // 48bd90 jl 0x48bd04
    { const reg32 a_ = ecx, b_ = eax; ecx = a_ - b_; fl.sub(a_, b_); }  // 48bd96 sub ecx, eax
    goto L_48bd04;  // 48bd98 jmp 0x48bd04
L_48bd9d:
    x.push(F(ebp + 0x62u));  // 48bd9d fld dword ptr [ebp + 0x62]
    x.push(F(ebp + 0x5au));  // 48bda0 fld dword ptr [ebp + 0x5a]
    x.st(0) = (x.st(0)) * (x.st(1));  // 48bda3 fmul st(1)
    x.push(D(0x53b974u));  // 48bda5 fld qword ptr [0x53b974]
    x.push(F(edi + 0x114u));  // 48bdab fld dword ptr [edi + 0x114]
    x.st(0) = (x.st(0)) * (x.st(1));  // 48bdb1 fmul st(1)
    x.push(F(edi + 0x11cu));  // 48bdb3 fld dword ptr [edi + 0x11c]
    x.st(0) = (x.st(0)) * (x.st(2));  // 48bdb9 fmul st(2)
    x.push(F(ebp + 0x5eu));  // 48bdbb fld dword ptr [ebp + 0x5e]
    x.st(0) = -x.st(0);  // 48bdbe fchs
    x.st(0) = (x.st(0)) * (x.st(5));  // 48bdc0 fmul st(5)
    eax = rd32((ebp - 0xau));  // 48bdc2 mov eax, dword ptr [ebp - 0xa]
    x.fxch(4);  // 48bdc5 fxch st(4)
    x.st(0) = (x.st(0)) * (F(ebp + 0x6eu));  // 48bdc7 fmul dword ptr [ebp + 0x6e]
    wr32((ebp + 0x7eu), eax);  // 48bdca mov dword ptr [ebp + 0x7e], eax
    x.fxch(4);  // 48bdcd fxch st(4)
    x.st(0) = (x.st(0)) * (F(ebp + 0x76u));  // 48bdcf fmul dword ptr [ebp + 0x76]
    x.push(double(int32_t(rd32((ebp + 0x7eu)))));  // 48bdd2 fild dword ptr [ebp + 0x7e]
    x.st(3) = (x.st(3)) * (x.st(0));  // 48bdd5 fmulp st(3)
    x.pop();  // 48bdd5 fmulp st(3)
    x.fxch(4);  // 48bdd7 fxch st(4)
    x.st(0) = (x.st(0)) * (x.st(3));  // 48bdd9 fmul st(3)
    eax = rd32((ebp - 0x1eu));  // 48bddb mov eax, dword ptr [ebp - 0x1e]
    x.fxch(4);  // 48bdde fxch st(4)
    x.st(3) = (x.st(3)) * (x.st(0));  // 48bde0 fmulp st(3)
    x.pop();  // 48bde0 fmulp st(3)
    wr32((ebp + 0x7eu), eax);  // 48bde2 mov dword ptr [ebp + 0x7e], eax
    x.fxch(1);  // 48bde5 fxch st(1)
    x.st(0) = (x.st(0)) * (D(0x53b97cu));  // 48bde7 fmul qword ptr [0x53b97c]
    x.push(double(int32_t(rd32((ebp + 0x7eu)))));  // 48bded fild dword ptr [ebp + 0x7e]
    x.st(2) = (x.st(2)) * (x.st(0));  // 48bdf0 fmulp st(2)
    x.pop();  // 48bdf0 fmulp st(2)
    x.fxch(2);  // 48bdf2 fxch st(2)
    x.push(D(0x53b94cu));  // 48bdf4 fld qword ptr [0x53b94c]
    x.fxch(1);  // 48bdfa fxch st(1)
    x.st(0) = (x.st(0)) * (x.st(1));  // 48bdfc fmul st(1)
    x.push(F(ebp + 0x52u));  // 48bdfe fld dword ptr [ebp + 0x52]
    x.st(5) = (x.st(5)) * (x.st(0));  // 48be01 fmulp st(5)
    x.pop();  // 48be01 fmulp st(5)
    x.fxch(2);  // 48be03 fxch st(2)
    x.st(0) = (x.st(0)) * (D(0x53b984u));  // 48be05 fmul qword ptr [0x53b984]
    x.fxch(3);  // 48be0b fxch st(3)
    x.st(0) = (x.st(0)) * (x.st(5));  // 48be0d fmul st(5)
    x.fxch(4);  // 48be0f fxch st(4)
    x.st(1) = (x.st(1)) * (x.st(0));  // 48be11 fmulp st(1)
    x.pop();  // 48be11 fmulp st(1)
    x.fxch(2);  // 48be13 fxch st(2)
    x.st(4) = (x.st(4)) * (x.st(0));  // 48be15 fmulp st(4)
    x.pop();  // 48be15 fmulp st(4)
    x.push(F(0x53b958u));  // 48be17 fld dword ptr [0x53b958]
    x.push(F(ebp + 0x76u));  // 48be1d fld dword ptr [ebp + 0x76]
    x.st(0) = (x.st(0)) * (x.st(1));  // 48be20 fmul st(1)
    x.fxch(4);  // 48be22 fxch st(4)
    SF((ebp + 0x72u), x.st(0));  // 48be24 fstp dword ptr [ebp + 0x72]
    x.pop();  // 48be24 fstp dword ptr [ebp + 0x72]
    x.push(F(ebp + 0x6eu));  // 48be27 fld dword ptr [ebp + 0x6e]
    x.st(0) = (x.st(0)) * (x.st(1));  // 48be2a fmul st(1)
    x.push(F(ebp + 0x72u));  // 48be2c fld dword ptr [ebp + 0x72]
    x.fxch(6);  // 48be2f fxch st(6)
    SF((ebp + 0x66u), x.st(0));  // 48be31 fstp dword ptr [ebp + 0x66]
    x.pop();  // 48be31 fstp dword ptr [ebp + 0x66]
    x.fxch(4);  // 48be34 fxch st(4)
    x.st(0) = (x.st(0)) + (F(ebp + 0x7au));  // 48be36 fadd dword ptr [ebp + 0x7a]
    x.fxch(4);  // 48be39 fxch st(4)
    x.st(0) = (x.st(0)) + (F(ebp + 0x6au));  // 48be3b fadd dword ptr [ebp + 0x6a]
    x.fxch(4);  // 48be3e fxch st(4)
    x.st(2) = (x.st(2)) + (x.st(0));  // 48be40 faddp st(2)
    x.pop();  // 48be40 faddp st(2)
    x.fxch(3);  // 48be42 fxch st(3)
    x.st(2) = (x.st(2)) + (x.st(0));  // 48be44 faddp st(2)
    x.pop();  // 48be44 faddp st(2)
    SF((ebp + 0x4eu), x.st(0));  // 48be46 fstp dword ptr [ebp + 0x4e]
    x.pop();  // 48be46 fstp dword ptr [ebp + 0x4e]
    SF((ebp + 0x56u), x.st(0));  // 48be49 fstp dword ptr [ebp + 0x56]
    x.pop();  // 48be49 fstp dword ptr [ebp + 0x56]
    x.fxch(1);  // 48be4c fxch st(1)
    fl.fcmp(x.st(0), x.st(1));  // 48be4e fcompp
    x.pop(); x.pop();  // 48be4e fcompp
    // fnstsw ax  // 48be50 fnstsw ax
    // sahf  // 48be52 sahf
    if (fl.ae()) goto L_48be5c;  // 48be53 jae 0x48be5c
    wr32((ebp + 0x72u), 1056964608u);  // 48be55 mov dword ptr [ebp + 0x72], 0x3f000000
L_48be5c:
    x.push(F(ebp + 0x66u));  // 48be5c fld dword ptr [ebp + 0x66]
    fl.fcmp(x.st(0), F(0x53b958u));  // 48be5f fcomp dword ptr [0x53b958]
    x.pop();  // 48be5f fcomp dword ptr [0x53b958]
    // fnstsw ax  // 48be65 fnstsw ax
    // sahf  // 48be67 sahf
    if (fl.ae()) goto L_48be71;  // 48be68 jae 0x48be71
    wr32((ebp + 0x66u), 1056964608u);  // 48be6a mov dword ptr [ebp + 0x66], 0x3f000000
L_48be71:
    x.push(F(ebp + 0x52u));  // 48be71 fld dword ptr [ebp + 0x52]
    x.st(0) = (x.st(0)) * (F(edi + 0x1a8u));  // 48be74 fmul dword ptr [edi + 0x1a8]
    esp -= 4; wr32(esp, rd32((edi + 0x1b0u)));  // 48be7a push dword ptr [edi + 0x1b0]
    esp -= 4u;  // 48be80 sub esp, 4
    SF((esp), x.st(0));  // 48be83 fstp dword ptr [esp]
    x.pop();  // 48be83 fstp dword ptr [esp]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4ea9c0, esp, 2, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48be86 call 0x4ea9c0
    esp += 8u;  // 48be86 call 0x4ea9c0
    x.push(F(ebp + 0x52u));  // 48be8b fld dword ptr [ebp + 0x52]
    x.st(0) = (x.st(0)) * (F(esi + 0x1a8u));  // 48be8e fmul dword ptr [esi + 0x1a8]
    esp -= 4; wr32(esp, rd32((esi + 0x1b0u)));  // 48be94 push dword ptr [esi + 0x1b0]
    esp -= 4u;  // 48be9a sub esp, 4
    SF((esp), x.st(0));  // 48be9d fstp dword ptr [esp]
    x.pop();  // 48be9d fstp dword ptr [esp]
    SF((ebp - 0x1au), x.st(0));  // 48bea0 fstp dword ptr [ebp - 0x1a]
    x.pop();  // 48bea0 fstp dword ptr [ebp - 0x1a]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4ea9c0, esp, 2, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48bea3 call 0x4ea9c0
    esp += 8u;  // 48bea3 call 0x4ea9c0
    x.st(0) = (F(ebp - 0x1au)) - (x.st(0));  // 48bea8 fsubr dword ptr [ebp - 0x1a]
    SF((ebp + 2u), x.st(0));  // 48beab fstp dword ptr [ebp + 2]
    x.pop();  // 48beab fstp dword ptr [ebp + 2]
    esp -= 4; wr32(esp, rd32((ebp + 2u)));  // 48beae push dword ptr [ebp + 2]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4ea930, esp, 1, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48beb1 call 0x4ea930
    esp += 4u;  // 48beb1 call 0x4ea930
    esp -= 4; wr32(esp, rd32((ebp + 2u)));  // 48beb6 push dword ptr [ebp + 2]
    SF((ebp - 0x2eu), x.st(0));  // 48beb9 fstp dword ptr [ebp - 0x2e]
    x.pop();  // 48beb9 fstp dword ptr [ebp - 0x2e]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4ea950, esp, 1, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48bebc call 0x4ea950
    esp += 4u;  // 48bebc call 0x4ea950
    x.push(F(ebp + 0x72u));  // 48bec1 fld dword ptr [ebp + 0x72]
    x.fxch(1);  // 48bec4 fxch st(1)
    SF((ebp - 0x26u), x.st(0));  // 48bec6 fstp dword ptr [ebp - 0x26]
    x.pop();  // 48bec6 fstp dword ptr [ebp - 0x26]
    x.st(0) = (x.st(0)) * (F(ebp - 0x26u));  // 48bec9 fmul dword ptr [ebp - 0x26]
    x.push(F(ebp + 0x72u));  // 48becc fld dword ptr [ebp + 0x72]
    x.st(0) = (x.st(0)) * (F(ebp - 0x2eu));  // 48becf fmul dword ptr [ebp - 0x2e]
    x.push(F(ebp + 0x66u));  // 48bed2 fld dword ptr [ebp + 0x66]
    x.st(0) = (x.st(0)) * (F(ebp - 0x2eu));  // 48bed5 fmul dword ptr [ebp - 0x2e]
    x.push(F(ebp + 0x66u));  // 48bed8 fld dword ptr [ebp + 0x66]
    x.st(0) = (x.st(0)) * (F(ebp - 0x26u));  // 48bedb fmul dword ptr [ebp - 0x26]
    x.push(F(ebp + 0x72u));  // 48bede fld dword ptr [ebp + 0x72]
    x.st(0) = -x.st(0);  // 48bee1 fchs
    x.push(x.st(0));  // 48bee3 fld st(0)
    x.st(0) = (x.st(0)) * (F(ebp - 0x26u));  // 48bee5 fmul dword ptr [ebp - 0x26]
    x.push(F(ebp + 0x66u));  // 48bee8 fld dword ptr [ebp + 0x66]
    x.st(0) = -x.st(0);  // 48beeb fchs
    SF((ebp - 0x1au), x.st(0));  // 48beed fst dword ptr [ebp - 0x1a]
    x.st(0) = (x.st(0)) * (F(ebp - 0x2eu));  // 48bef0 fmul dword ptr [ebp - 0x2e]
    x.fxch(2);  // 48bef3 fxch st(2)
    x.st(0) = (x.st(0)) * (F(ebp - 0x2eu));  // 48bef5 fmul dword ptr [ebp - 0x2e]
    esp -= 4; wr32(esp, 2399207168u);  // 48bef8 push 0x8f00ff00
    x.fxch(2);  // 48befd fxch st(2)
    SF((ebp - 0x16u), x.st(0));  // 48beff fstp dword ptr [ebp - 0x16]
    x.pop();  // 48beff fstp dword ptr [ebp - 0x16]
    x.push(F(ebp - 0x1au));  // 48bf02 fld dword ptr [ebp - 0x1a]
    x.st(0) = (x.st(0)) * (F(ebp - 0x26u));  // 48bf05 fmul dword ptr [ebp - 0x26]
    esp -= 4; wr32(esp, 1u);  // 48bf08 push 1
    x.fxch(6);  // 48bf0a fxch st(6)
    x.st(0) = (x.st(0)) + (F(ebp + 0x56u));  // 48bf0c fadd dword ptr [ebp + 0x56]
    x.fxch(5);  // 48bf0f fxch st(5)
    x.st(0) = (x.st(0)) + (F(ebp + 0x4eu));  // 48bf11 fadd dword ptr [ebp + 0x4e]
    x.fxch(6);  // 48bf14 fxch st(6)
    SF((ebp - 0xeu), x.st(0));  // 48bf16 fstp dword ptr [ebp - 0xe]
    x.pop();  // 48bf16 fstp dword ptr [ebp - 0xe]
    x.fxch(4);  // 48bf19 fxch st(4)
    SF((ebp - 6u), x.st(0));  // 48bf1b fst dword ptr [ebp - 6]
    x.fxch(5);  // 48bf1e fxch st(5)
    SF((ebp - 2u), x.st(0));  // 48bf20 fst dword ptr [ebp - 2]
    x.fxch(4);  // 48bf23 fxch st(4)
    x.st(0) = (x.st(0)) + (F(ebp + 0x56u));  // 48bf25 fadd dword ptr [ebp + 0x56]
    x.fxch(1);  // 48bf28 fxch st(1)
    x.st(0) = (x.st(0)) + (F(ebp + 0x4eu));  // 48bf2a fadd dword ptr [ebp + 0x4e]
    x.fxch(1);  // 48bf2d fxch st(1)
    SF((ebp - 0x1au), x.st(0));  // 48bf2f fst dword ptr [ebp - 0x1a]
    x.fxch(1);  // 48bf32 fxch st(1)
    SF((ebp - 0x12u), x.st(0));  // 48bf34 fst dword ptr [ebp - 0x12]
    x.fxch(5);  // 48bf37 fxch st(5)
    x.st(0) = (x.st(0)) - (F(ebp - 0x16u));  // 48bf39 fsub dword ptr [ebp - 0x16]
    x.fxch(4);  // 48bf3c fxch st(4)
    x.st(0) = (x.st(0)) + (F(ebp - 0xeu));  // 48bf3e fadd dword ptr [ebp - 0xe]
    x.push(F(ebp - 6u));  // 48bf41 fld dword ptr [ebp - 6]
    x.st(0) = (x.st(0)) - (x.st(4));  // 48bf44 fsub st(4)
    x.fxch(3);  // 48bf46 fxch st(3)
    SF((ebp + 0x7eu), x.st(0));  // 48bf48 fstp dword ptr [ebp + 0x7e]
    x.pop();  // 48bf48 fstp dword ptr [ebp + 0x7e]
    x.push(F(ebp - 2u));  // 48bf4b fld dword ptr [ebp - 2]
    x.st(0) = (x.st(0)) + (F(ebp + 0x7eu));  // 48bf4e fadd dword ptr [ebp + 0x7e]
    x.fxch(5);  // 48bf51 fxch st(5)
    SF((ebp - 0x66u), x.st(0));  // 48bf53 fstp dword ptr [ebp - 0x66]
    x.pop();  // 48bf53 fstp dword ptr [ebp - 0x66]
    SF((ebp - 0x62u), x.st(0));  // 48bf56 fstp dword ptr [ebp - 0x62]
    x.pop();  // 48bf56 fstp dword ptr [ebp - 0x62]
    x.fxch(1);  // 48bf59 fxch st(1)
    SF((ebp - 0x5eu), x.st(0));  // 48bf5b fstp dword ptr [ebp - 0x5e]
    x.pop();  // 48bf5b fstp dword ptr [ebp - 0x5e]
    x.fxch(2);  // 48bf5e fxch st(2)
    SF((ebp - 0x5au), x.st(0));  // 48bf60 fstp dword ptr [ebp - 0x5a]
    x.pop();  // 48bf60 fstp dword ptr [ebp - 0x5a]
    x.push(F(ebp - 0x12u));  // 48bf63 fld dword ptr [ebp - 0x12]
    x.st(0) = (x.st(0)) + (F(ebp + 0x7eu));  // 48bf66 fadd dword ptr [ebp + 0x7e]
    x.push(F(ebp - 0x1au));  // 48bf69 fld dword ptr [ebp - 0x1a]
    x.st(2) = (x.st(0)) - (x.st(2));  // 48bf6c fsubrp st(2)
    x.pop();  // 48bf6c fsubrp st(2)
    SF((ebp - 0x52u), x.st(0));  // 48bf6e fstp dword ptr [ebp - 0x52]
    x.pop();  // 48bf6e fstp dword ptr [ebp - 0x52]
    edi = rd32((ebp - 0x52u));  // 48bf71 mov edi, dword ptr [ebp - 0x52]
    SF((ebp - 0x56u), x.st(0));  // 48bf74 fstp dword ptr [ebp - 0x56]
    x.pop();  // 48bf74 fstp dword ptr [ebp - 0x56]
    esp -= 4; wr32(esp, edi);  // 48bf77 push edi
    eax = rd32((ebp - 0x56u));  // 48bf78 mov eax, dword ptr [ebp - 0x56]
    edx = rd32((ebp - 0x5au));  // 48bf7b mov edx, dword ptr [ebp - 0x5a]
    esp -= 4; wr32(esp, eax);  // 48bf7e push eax
    ecx = rd32((ebp - 0x5eu));  // 48bf7f mov ecx, dword ptr [ebp - 0x5e]
    ebx = rd32((ebp - 0x62u));  // 48bf82 mov ebx, dword ptr [ebp - 0x62]
    esp -= 4; wr32(esp, edx);  // 48bf85 push edx
    x.fxch(1);  // 48bf86 fxch st(1)
    x.st(0) = (x.st(0)) + (F(ebp - 0xeu));  // 48bf88 fadd dword ptr [ebp - 0xe]
    esp -= 4; wr32(esp, ecx);  // 48bf8b push ecx
    SF((ebp - 0x6au), x.st(0));  // 48bf8c fstp dword ptr [ebp - 0x6a]
    x.pop();  // 48bf8c fstp dword ptr [ebp - 0x6a]
    esp -= 4; wr32(esp, ebx);  // 48bf8f push ebx
    edi = rd32((ebp - 0x66u));  // 48bf90 mov edi, dword ptr [ebp - 0x66]
    eax = rd32((ebp - 0x6au));  // 48bf93 mov eax, dword ptr [ebp - 0x6a]
    esp -= 4; wr32(esp, edi);  // 48bf96 push edi
    x.st(0) = (x.st(0)) - (F(ebp - 0x16u));  // 48bf97 fsub dword ptr [ebp - 0x16]
    esp -= 4; wr32(esp, eax);  // 48bf9a push eax
    SF((ebp - 0x6eu), x.st(0));  // 48bf9b fstp dword ptr [ebp - 0x6e]
    x.pop();  // 48bf9b fstp dword ptr [ebp - 0x6e]
    edx = rd32((ebp - 0x6eu));  // 48bf9e mov edx, dword ptr [ebp - 0x6e]
    esp -= 4; wr32(esp, edx);  // 48bfa1 push edx
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x47d1d0, esp, 10, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48bfa2 call 0x47d1d0
    esp += 40u;  // 48bfa2 call 0x47d1d0
    x.push(F(0x53b958u));  // 48bfa7 fld dword ptr [0x53b958]
    x.push(F(ebp + 0x6eu));  // 48bfad fld dword ptr [ebp + 0x6e]
    x.st(0) = (x.st(0)) * (x.st(1));  // 48bfb0 fmul st(1)
    x.push(F(ebp + 0x76u));  // 48bfb2 fld dword ptr [ebp + 0x76]
    x.st(2) = (x.st(2)) * (x.st(0));  // 48bfb5 fmulp st(2)
    x.pop();  // 48bfb5 fmulp st(2)
    x.push(D(0x53b974u));  // 48bfb7 fld qword ptr [0x53b974]
    x.push(F(esi + 0x114u));  // 48bfbd fld dword ptr [esi + 0x114]
    x.st(0) = (x.st(0)) * (x.st(1));  // 48bfc3 fmul st(1)
    x.push(F(esi + 0x11cu));  // 48bfc5 fld dword ptr [esi + 0x11c]
    eax = rd32((ebp - 0xau));  // 48bfcb mov eax, dword ptr [ebp - 0xa]
    x.st(2) = (x.st(2)) * (x.st(0));  // 48bfce fmulp st(2)
    x.pop();  // 48bfce fmulp st(2)
    wr32((ebp + 0x7eu), eax);  // 48bfd0 mov dword ptr [ebp + 0x7e], eax
    eax = rd32((ebp - 0x1eu));  // 48bfd3 mov eax, dword ptr [ebp - 0x1e]
    x.push(double(int32_t(rd32((ebp + 0x7eu)))));  // 48bfd6 fild dword ptr [ebp + 0x7e]
    wr32((ebp + 0x7eu), eax);  // 48bfd9 mov dword ptr [ebp + 0x7e], eax
    x.st(1) = (x.st(1)) * (x.st(0));  // 48bfdc fmulp st(1)
    x.pop();  // 48bfdc fmulp st(1)
    x.push(double(int32_t(rd32((ebp + 0x7eu)))));  // 48bfde fild dword ptr [ebp + 0x7e]
    x.st(2) = (x.st(2)) * (x.st(0));  // 48bfe1 fmulp st(2)
    x.pop();  // 48bfe1 fmulp st(2)
    x.st(0) = (x.st(0)) * (D(0x53b97cu));  // 48bfe3 fmul qword ptr [0x53b97c]
    x.fxch(1);  // 48bfe9 fxch st(1)
    x.st(0) = (x.st(0)) * (D(0x53b984u));  // 48bfeb fmul qword ptr [0x53b984]
    x.push(F(ebp + 0x62u));  // 48bff1 fld dword ptr [ebp + 0x62]
    x.fxch(2);  // 48bff4 fxch st(2)
    x.st(0) = (x.st(0)) * (x.st(2));  // 48bff6 fmul st(2)
    x.fxch(1);  // 48bff8 fxch st(1)
    x.st(2) = (x.st(2)) * (x.st(0));  // 48bffa fmulp st(2)
    x.pop();  // 48bffa fmulp st(2)
    x.fxch(2);  // 48bffc fxch st(2)
    x.st(0) = (x.st(0)) + (F(ebp + 0x6au));  // 48bffe fadd dword ptr [ebp + 0x6a]
    x.fxch(3);  // 48c001 fxch st(3)
    x.st(0) = (x.st(0)) + (F(ebp + 0x7au));  // 48c003 fadd dword ptr [ebp + 0x7a]
    x.fxch(3);  // 48c006 fxch st(3)
    SF((ebp + 0x56u), x.st(0));  // 48c008 fstp dword ptr [ebp + 0x56]
    x.pop();  // 48c008 fstp dword ptr [ebp + 0x56]
    x.fxch(2);  // 48c00b fxch st(2)
    SF((ebp + 0x4eu), x.st(0));  // 48c00d fstp dword ptr [ebp + 0x4e]
    x.pop();  // 48c00d fstp dword ptr [ebp + 0x4e]
    SF((ebp + 0x72u), x.st(0));  // 48c010 fstp dword ptr [ebp + 0x72]
    x.pop();  // 48c010 fstp dword ptr [ebp + 0x72]
    SF((ebp + 0x66u), x.st(0));  // 48c013 fstp dword ptr [ebp + 0x66]
    x.pop();  // 48c013 fstp dword ptr [ebp + 0x66]
    x.push(F(ebp + 0x72u));  // 48c016 fld dword ptr [ebp + 0x72]
    fl.fcmp(x.st(0), F(0x53b958u));  // 48c019 fcomp dword ptr [0x53b958]
    x.pop();  // 48c019 fcomp dword ptr [0x53b958]
    // fnstsw ax  // 48c01f fnstsw ax
    // sahf  // 48c021 sahf
    if (fl.ae()) goto L_48c02b;  // 48c022 jae 0x48c02b
    wr32((ebp + 0x72u), 1056964608u);  // 48c024 mov dword ptr [ebp + 0x72], 0x3f000000
L_48c02b:
    x.push(F(ebp + 0x66u));  // 48c02b fld dword ptr [ebp + 0x66]
    fl.fcmp(x.st(0), F(0x53b958u));  // 48c02e fcomp dword ptr [0x53b958]
    x.pop();  // 48c02e fcomp dword ptr [0x53b958]
    // fnstsw ax  // 48c034 fnstsw ax
    // sahf  // 48c036 sahf
    if (fl.ae()) goto L_48c040;  // 48c037 jae 0x48c040
    wr32((ebp + 0x66u), 1056964608u);  // 48c039 mov dword ptr [ebp + 0x66], 0x3f000000
L_48c040:
    x.push(F(ebp + 0x56u));  // 48c040 fld dword ptr [ebp + 0x56]
    x.push(x.st(0));  // 48c043 fld st(0)
    esp -= 4; wr32(esp, 2399207168u);  // 48c045 push 0x8f00ff00
    x.push(F(ebp + 0x4eu));  // 48c04a fld dword ptr [ebp + 0x4e]
    x.push(x.st(0));  // 48c04d fld st(0)
    esp -= 4; wr32(esp, 1u);  // 48c04f push 1
    x.fxch(2);  // 48c051 fxch st(2)
    x.push(F(ebp + 0x72u));  // 48c053 fld dword ptr [ebp + 0x72]
    x.fxch(1);  // 48c056 fxch st(1)
    x.st(0) = (x.st(0)) - (x.st(1));  // 48c058 fsub st(1)
    x.fxch(3);  // 48c05a fxch st(3)
    x.push(F(ebp + 0x66u));  // 48c05c fld dword ptr [ebp + 0x66]
    x.fxch(1);  // 48c05f fxch st(1)
    x.st(0) = (x.st(0)) - (x.st(1));  // 48c061 fsub st(1)
    x.fxch(5);  // 48c063 fxch st(5)
    x.st(2) = (x.st(2)) + (x.st(0));  // 48c065 faddp st(2)
    x.pop();  // 48c065 faddp st(2)
    x.st(2) = (x.st(2)) + (x.st(0));  // 48c067 faddp st(2)
    x.pop();  // 48c067 faddp st(2)
    x.fxch(3);  // 48c069 fxch st(3)
    SF((ebp - 0x6au), x.st(0));  // 48c06b fstp dword ptr [ebp - 0x6a]
    x.pop();  // 48c06b fstp dword ptr [ebp - 0x6a]
    eax = rd32((ebp - 0x6au));  // 48c06e mov eax, dword ptr [ebp - 0x6a]
    x.fxch(2);  // 48c071 fxch st(2)
    SF((ebp - 0x66u), x.st(0));  // 48c073 fstp dword ptr [ebp - 0x66]
    x.pop();  // 48c073 fstp dword ptr [ebp - 0x66]
    wr32((ebp - 0x62u), eax);  // 48c076 mov dword ptr [ebp - 0x62], eax
    eax = rd32((ebp - 0x66u));  // 48c079 mov eax, dword ptr [ebp - 0x66]
    SF((ebp - 0x6eu), x.st(0));  // 48c07c fstp dword ptr [ebp - 0x6e]
    x.pop();  // 48c07c fstp dword ptr [ebp - 0x6e]
    wr32((ebp - 0x5eu), eax);  // 48c07f mov dword ptr [ebp - 0x5e], eax
    eax = rd32((ebp - 0x6eu));  // 48c082 mov eax, dword ptr [ebp - 0x6e]
    SF((ebp - 0x5au), x.st(0));  // 48c085 fstp dword ptr [ebp - 0x5a]
    x.pop();  // 48c085 fstp dword ptr [ebp - 0x5a]
    wr32((ebp - 0x56u), eax);  // 48c088 mov dword ptr [ebp - 0x56], eax
    eax = rd32((ebp - 0x5au));  // 48c08b mov eax, dword ptr [ebp - 0x5a]
    esp -= 4; wr32(esp, eax);  // 48c08e push eax
    edi = rd32((ebp - 0x56u));  // 48c08f mov edi, dword ptr [ebp - 0x56]
    esp -= 4; wr32(esp, edi);  // 48c092 push edi
    esp -= 4; wr32(esp, eax);  // 48c093 push eax
    edx = rd32((ebp - 0x5eu));  // 48c094 mov edx, dword ptr [ebp - 0x5e]
    esp -= 4; wr32(esp, edx);  // 48c097 push edx
    ecx = rd32((ebp - 0x62u));  // 48c098 mov ecx, dword ptr [ebp - 0x62]
    esp -= 4; wr32(esp, ecx);  // 48c09b push ecx
    ebx = rd32((ebp - 0x66u));  // 48c09c mov ebx, dword ptr [ebp - 0x66]
    esp -= 4; wr32(esp, ebx);  // 48c09f push ebx
    esi = rd32((ebp - 0x6au));  // 48c0a0 mov esi, dword ptr [ebp - 0x6a]
    esp -= 4; wr32(esp, esi);  // 48c0a3 push esi
    edi = rd32((ebp - 0x6eu));  // 48c0a4 mov edi, dword ptr [ebp - 0x6e]
    esp -= 4; wr32(esp, edi);  // 48c0a7 push edi
    wr32((ebp - 0x52u), eax);  // 48c0a8 mov dword ptr [ebp - 0x52], eax
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x47d1d0, esp, 10, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48c0ab call 0x47d1d0
    esp += 40u;  // 48c0ab call 0x47d1d0
    ecx = rd32((ebp - 0x1eu));  // 48c0b0 mov ecx, dword ptr [ebp - 0x1e]
    ebx = rd32((ebp - 0xau));  // 48c0b3 mov ebx, dword ptr [ebp - 0xa]
    edx = rd32((ebp - 0x2au));  // 48c0b6 mov edx, dword ptr [ebp - 0x2a]
    eax = rd32((ebp - 0x22u));  // 48c0b9 mov eax, dword ptr [ebp - 0x22]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4bebe0, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48c0bc call 0x4bebe0
    esp += 0u;  // 48c0bc call 0x4bebe0
    eax = 104u;  // 48c0c1 mov eax, 0x68
    edx = 0u;  // 48c0c6 xor edx, edx
    fl.logic(0u);  // 48c0c6 xor edx, edx
    ebx = 1u;  // 48c0c8 mov ebx, 1
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x431900, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48c0cd call 0x431900
    esp += 0u;  // 48c0cd call 0x431900
L_48c0d2:
    eax = ebx;  // 48c0d2 mov eax, ebx
    esp = (ebp + 0x82u);  // 48c0d4 lea esp, [ebp + 0x82]
    ebp = rd32(esp); esp += 4;  // 48c0da pop ebp
    edi = rd32(esp); esp += 4;  // 48c0db pop edi
    esi = rd32(esp); esp += 4;  // 48c0dc pop esi
    result = eax; popped_ = 0u; return true;  // 48c0dd ret
    return false;
}
// (popped = 0
bool decompMapPoints(App* app, CPU& cpu, const Entry& e, reg32& result, reg32& popped_)
{
    reg32 eax = e.eax, ebx = e.ebx, ecx = e.ecx, edx = e.edx, esi = e.esi, edi = e.edi, ebp = e.ebp, esp = e.esp;
    Flags fl;
    X87 x;
    reg32 carry_ = 0;
    (void)carry_;
    const auto rd32 = [&](reg32 a) { return app->getMemory<reg32>(a); };
    const auto rd16 = [&](reg32 a) { return reg32(app->getMemory<uint16_t>(a)); };
    const auto rd8 = [&](reg32 a) { return reg32(app->getMemory<uint8_t>(a)); };
    const auto wr32 = [&](reg32 a, reg32 v) { app->getMemory<reg32>(a) = v; };
    const auto wr16 = [&](reg32 a, reg32 v) { app->getMemory<uint16_t>(a) = uint16_t(v); };
    const auto wr8 = [&](reg32 a, reg32 v) { app->getMemory<uint8_t>(a) = uint8_t(v); };
    const auto F = [&](reg32 a) { return double(app->getMemory<float>(a)); };
    const auto D = [&](reg32 a) { return app->getMemory<double>(a); };
    const auto SF = [&](reg32 a, double v) { app->getMemory<float>(a) = float(v); };
    const auto SD = [&](reg32 a, double v) { app->getMemory<double>(a) = v; };
    esp -= 4; wr32(esp, ebx);  // 48d4c0 push ebx
    esp -= 4; wr32(esp, ecx);  // 48d4c1 push ecx
    esp -= 4; wr32(esp, esi);  // 48d4c2 push esi
    esp -= 4; wr32(esp, edi);  // 48d4c3 push edi
    esp -= 4; wr32(esp, ebp);  // 48d4c4 push ebp
    ebp = esp;  // 48d4c5 mov ebp, esp
    esp -= 304u;  // 48d4c7 sub esp, 0x130
    { const reg32 a_ = ebp, b_ = 130u; ebp = a_ - b_; fl.sub(a_, b_); }  // 48d4cd sub ebp, 0x82
    wr32((ebp + 0x76u), eax);  // 48d4d3 mov dword ptr [ebp + 0x76], eax
    wr32((ebp + 0x72u), edx);  // 48d4d6 mov dword ptr [ebp + 0x72], edx
    carry_ = (eax >> 28) & 1u;  // 48d4d9 shl eax, 4
    { const reg32 r_ = eax << 4; eax = r_; fl.logic(r_); }  // 48d4d9 shl eax, 4
    edx = rd32((eax + 0x7925dcu));  // 48d4dc mov edx, dword ptr [eax + 0x7925dc]
    wr32((ebp - 0xeu), edx);  // 48d4e2 mov dword ptr [ebp - 0xe], edx
    edx = rd32((eax + 0x7925e4u));  // 48d4e5 mov edx, dword ptr [eax + 0x7925e4]
    wr32((ebp - 0x16u), edx);  // 48d4eb mov dword ptr [ebp - 0x16], edx
    edx = rd32((eax + 0x7925d8u));  // 48d4ee mov edx, dword ptr [eax + 0x7925d8]
    wr32((ebp - 0x12u), edx);  // 48d4f4 mov dword ptr [ebp - 0x12], edx
    eax = rd32((eax + 0x7925e0u));  // 48d4f7 mov eax, dword ptr [eax + 0x7925e0]
    edx = rd32((0x6fd3b0u));  // 48d4fd mov edx, dword ptr [0x6fd3b0]
    wr32((ebp - 6u), eax);  // 48d503 mov dword ptr [ebp - 6], eax
    fl.sub(edx, 1u);  // 48d506 cmp edx, 1
    if (fl.ne()) goto L_48d510;  // 48d509 jne 0x48d510
    eax = rd32((ebp + 0x76u));  // 48d50b mov eax, dword ptr [ebp + 0x76]
    goto L_48d515;  // 48d50e jmp 0x48d515
L_48d510:
    eax = rd32((0x6fd4f4u));  // 48d510 mov eax, dword ptr [0x6fd4f4]
L_48d515:
    ecx = rd32((0x6fd3b0u));  // 48d515 mov ecx, dword ptr [0x6fd3b0]
    wr32((ebp + 0x32u), eax);  // 48d51b mov dword ptr [ebp + 0x32], eax
    fl.sub(ecx, 1u);  // 48d51e cmp ecx, 1
    if (fl.ne()) goto L_48d527;  // 48d521 jne 0x48d527
    eax = ecx;  // 48d523 mov eax, ecx
    goto L_48d529;  // 48d525 jmp 0x48d529
L_48d527:
    eax = 0u;  // 48d527 xor eax, eax
    fl.logic(0u);  // 48d527 xor eax, eax
L_48d529:
    edx = rd32((ebp + 0x76u));  // 48d529 mov edx, dword ptr [ebp + 0x76]
    { const reg32 a_ = edx, b_ = eax; edx = a_ + b_; fl.add(a_, b_); }  // 48d52c add edx, eax
    eax = rd32((ebp + 0x72u));  // 48d52e mov eax, dword ptr [ebp + 0x72]
    wr32((0x79c110u), edx);  // 48d531 mov dword ptr [0x79c110], edx
    fl.logic(rd8((eax + 0x200u)) & 32u);  // 48d537 test byte ptr [eax + 0x200], 0x20
    if (fl.e()) goto L_48d547;  // 48d53e je 0x48d547
    eax = 1u;  // 48d540 mov eax, 1
    goto L_48d549;  // 48d545 jmp 0x48d549
L_48d547:
    eax = 0u;  // 48d547 xor eax, eax
    fl.logic(0u);  // 48d547 xor eax, eax
L_48d549:
    edx = reg32(int32_t(rd32((0x79c110u))) * int32_t(840u));  // 48d549 imul edx, dword ptr [0x79c110], 0x348
    wr32((0x79c10cu), eax);  // 48d553 mov dword ptr [0x79c10c], eax
    eax = reg32(int32_t(eax) * int32_t(420u));  // 48d558 imul eax, eax, 0x1a4
    eax = rd32((edx + eax + 0x6fbc60u));  // 48d55e mov eax, dword ptr [edx + eax + 0x6fbc60]
    edx = rd32((ebp + 0x76u));  // 48d565 mov edx, dword ptr [ebp + 0x76]
    esi = 0u;  // 48d568 xor esi, esi
    fl.logic(0u);  // 48d568 xor esi, esi
    carry_ = (edx >> 30) & 1u;  // 48d56a shl edx, 2
    { const reg32 r_ = edx << 2; edx = r_; fl.logic(r_); }  // 48d56a shl edx, 2
    wr32((ebp + 0x26u), eax);  // 48d56d mov dword ptr [ebp + 0x26], eax
    wr32((edx + 0x55e2b4u), esi);  // 48d570 mov dword ptr [edx + 0x55e2b4], esi
    fl.sub(eax, 1065353216u);  // 48d576 cmp eax, 0x3f800000
    if (fl.le()) goto L_48d5a9;  // 48d57b jle 0x48d5a9
    x.push(F(ebp - 0xeu));  // 48d57d fld dword ptr [ebp - 0xe]
    x.push(F(ebp - 0x12u));  // 48d580 fld dword ptr [ebp - 0x12]
    x.st(0) = (x.st(0)) + (F(ebp - 6u));  // 48d583 fadd dword ptr [ebp - 6]
    x.fxch(1);  // 48d586 fxch st(1)
    x.st(0) = (x.st(0)) + (F(ebp - 0x16u));  // 48d588 fadd dword ptr [ebp - 0x16]
    x.fxch(1);  // 48d58b fxch st(1)
    x.push(F(0x53b9e0u));  // 48d58d fld dword ptr [0x53b9e0]
    x.fxch(1);  // 48d593 fxch st(1)
    x.st(0) = (x.st(0)) * (x.st(1));  // 48d595 fmul st(1)
    x.fxch(2);  // 48d597 fxch st(2)
    x.st(1) = (x.st(1)) * (x.st(0));  // 48d599 fmulp st(1)
    x.pop();  // 48d599 fmulp st(1)
    x.fxch(1);  // 48d59b fxch st(1)
    SF((edx + 0x792888u), x.st(0));  // 48d59d fstp dword ptr [edx + 0x792888]
    x.pop();  // 48d59d fstp dword ptr [edx + 0x792888]
    SF((edx + 0x792890u), x.st(0));  // 48d5a3 fstp dword ptr [edx + 0x792890]
    x.pop();  // 48d5a3 fstp dword ptr [edx + 0x792890]
L_48d5a9:
    edx = reg32(int32_t(rd32((0x79c110u))) * int32_t(840u));  // 48d5a9 imul edx, dword ptr [0x79c110], 0x348
    eax = reg32(int32_t(rd32((0x79c10cu))) * int32_t(420u));  // 48d5b3 imul eax, dword ptr [0x79c10c], 0x1a4
    fl.sub(rd32((edx + eax + 0x6fbc70u)), 1u);  // 48d5bd cmp dword ptr [edx + eax + 0x6fbc70], 1
    if (fl.ne()) goto L_48d717;  // 48d5c5 jne 0x48d717
    eax = rd32((ebp + 0x72u));  // 48d5cb mov eax, dword ptr [ebp + 0x72]
    edx = 0u;  // 48d5ce xor edx, edx
    fl.logic(0u);  // 48d5ce xor edx, edx
    ebx = rd32((0x5dd940u));  // 48d5d0 mov ebx, dword ptr [0x5dd940]
    wr32((ebp + 0x6eu), edx);  // 48d5d6 mov dword ptr [ebp + 0x6e], edx
    wr32((ebp + 0x6au), edx);  // 48d5d9 mov dword ptr [ebp + 0x6a], edx
    ecx = rd32((eax + 0x1cu));  // 48d5dc mov ecx, dword ptr [eax + 0x1c]
    goto L_48d5e9;  // 48d5df jmp 0x48d5e9
L_48d5e1:
    ecx = 0u;  // 48d5e1 xor ecx, ecx
    fl.logic(0u);  // 48d5e1 xor ecx, ecx
L_48d5e3:
    { const reg32 r_ = edx + 1u; edx = r_; fl.arith(r_); }  // 48d5e3 inc edx
    fl.sub(edx, 12u);  // 48d5e4 cmp edx, 0xc
    if (fl.ge()) goto L_48d626;  // 48d5e7 jge 0x48d626
L_48d5e9:
    eax = (ecx * 8u);  // 48d5e9 lea eax, [ecx*8]
    { const reg32 a_ = eax, b_ = ecx; eax = a_ + b_; fl.add(a_, b_); }  // 48d5f0 add eax, ecx
    esi = rd32((0x5dd948u));  // 48d5f2 mov esi, dword ptr [0x5dd948]
    carry_ = (eax >> 30) & 1u;  // 48d5f8 shl eax, 2
    { const reg32 r_ = eax << 2; eax = r_; fl.logic(r_); }  // 48d5f8 shl eax, 2
    { const reg32 a_ = eax, b_ = esi; eax = a_ + b_; fl.add(a_, b_); }  // 48d5fb add eax, esi
    edi = rd32((ebp + 0x6eu));  // 48d5fd mov edi, dword ptr [ebp + 0x6e]
    esi = rd32((eax + 0x11u));  // 48d600 mov esi, dword ptr [eax + 0x11]
    eax = rd32((eax + 0x13u));  // 48d603 mov eax, dword ptr [eax + 0x13]
    carry_ = (esi >> 23) & 1u;  // 48d606 sar esi, 0x18
    { const reg32 r_ = reg32(int32_t(esi) >> 24); esi = r_; fl.logic(r_); }  // 48d606 sar esi, 0x18
    carry_ = (eax >> 23) & 1u;  // 48d609 sar eax, 0x18
    { const reg32 r_ = reg32(int32_t(eax) >> 24); eax = r_; fl.logic(r_); }  // 48d609 sar eax, 0x18
    carry_ = (esi >> 23) & 1u;  // 48d60c shl esi, 9
    { const reg32 r_ = esi << 9; esi = r_; fl.logic(r_); }  // 48d60c shl esi, 9
    carry_ = (eax >> 23) & 1u;  // 48d60f shl eax, 9
    { const reg32 r_ = eax << 9; eax = r_; fl.logic(r_); }  // 48d60f shl eax, 9
    { const reg32 a_ = edi, b_ = esi; edi = a_ + b_; fl.add(a_, b_); }  // 48d612 add edi, esi
    esi = rd32((ebp + 0x6au));  // 48d614 mov esi, dword ptr [ebp + 0x6a]
    { const reg32 r_ = ecx + 1u; ecx = r_; fl.arith(r_); }  // 48d617 inc ecx
    { const reg32 a_ = esi, b_ = eax; esi = a_ + b_; fl.add(a_, b_); }  // 48d618 add esi, eax
    wr32((ebp + 0x6eu), edi);  // 48d61a mov dword ptr [ebp + 0x6e], edi
    wr32((ebp + 0x6au), esi);  // 48d61d mov dword ptr [ebp + 0x6a], esi
    fl.sub(ecx, ebx);  // 48d620 cmp ecx, ebx
    if (fl.ge()) goto L_48d5e1;  // 48d622 jge 0x48d5e1
    goto L_48d5e3;  // 48d624 jmp 0x48d5e3
L_48d626:
    edx = rd32((ebp + 0x72u));  // 48d626 mov edx, dword ptr [ebp + 0x72]
    ecx = 0u;  // 48d629 xor ecx, ecx
    fl.logic(0u);  // 48d629 xor ecx, ecx
    edx = rd32((edx + 0x1cu));  // 48d62b mov edx, dword ptr [edx + 0x1c]
    goto L_48d638;  // 48d62e jmp 0x48d638
L_48d630:
    edx = ebx;  // 48d630 mov edx, ebx
L_48d632:
    { const reg32 r_ = ecx + 1u; ecx = r_; fl.arith(r_); }  // 48d632 inc ecx
    fl.sub(ecx, 12u);  // 48d633 cmp ecx, 0xc
    if (fl.ge()) goto L_48d6ad;  // 48d636 jge 0x48d6ad
L_48d638:
    eax = (edx * 8u);  // 48d638 lea eax, [edx*8]
    { const reg32 a_ = eax, b_ = edx; eax = a_ + b_; fl.add(a_, b_); }  // 48d63f add eax, edx
    esi = rd32((0x5dd948u));  // 48d641 mov esi, dword ptr [0x5dd948]
    carry_ = (eax >> 30) & 1u;  // 48d647 shl eax, 2
    { const reg32 r_ = eax << 2; eax = r_; fl.logic(r_); }  // 48d647 shl eax, 2
    { const reg32 a_ = esi, b_ = eax; esi = a_ + b_; fl.add(a_, b_); }  // 48d64a add esi, eax
    eax = rd32((esi + 0x11u));  // 48d64c mov eax, dword ptr [esi + 0x11]
    carry_ = (eax >> 23) & 1u;  // 48d64f sar eax, 0x18
    { const reg32 r_ = reg32(int32_t(eax) >> 24); eax = r_; fl.logic(r_); }  // 48d64f sar eax, 0x18
    carry_ = (eax >> 23) & 1u;  // 48d652 shl eax, 9
    { const reg32 r_ = eax << 9; eax = r_; fl.logic(r_); }  // 48d652 shl eax, 9
    wr32((ebp + 0x7au), eax);  // 48d655 mov dword ptr [ebp + 0x7a], eax
    x.push(double(int32_t(rd32((ebp + 0x7au)))));  // 48d658 fild dword ptr [ebp + 0x7a]
    eax = rd32((ebp + 0x6eu));  // 48d65b mov eax, dword ptr [ebp + 0x6e]
    x.push(D(0x53b9e4u));  // 48d65e fld qword ptr [0x53b9e4]
    x.fxch(1);  // 48d664 fxch st(1)
    x.st(0) = (x.st(0)) * (x.st(1));  // 48d666 fmul st(1)
    wr32((ebp + 0x7au), eax);  // 48d668 mov dword ptr [ebp + 0x7a], eax
    eax = rd32((esi + 0x13u));  // 48d66b mov eax, dword ptr [esi + 0x13]
    carry_ = (eax >> 23) & 1u;  // 48d66e sar eax, 0x18
    { const reg32 r_ = reg32(int32_t(eax) >> 24); eax = r_; fl.logic(r_); }  // 48d66e sar eax, 0x18
    carry_ = (eax >> 23) & 1u;  // 48d671 shl eax, 9
    { const reg32 r_ = eax << 9; eax = r_; fl.logic(r_); }  // 48d671 shl eax, 9
    wr32((ebp + 0x7eu), eax);  // 48d674 mov dword ptr [ebp + 0x7e], eax
    x.push(double(int32_t(rd32((ebp + 0x7eu)))));  // 48d677 fild dword ptr [ebp + 0x7e]
    x.st(2) = (x.st(2)) * (x.st(0));  // 48d67a fmulp st(2)
    x.pop();  // 48d67a fmulp st(2)
    { const reg32 r_ = edx - 1u; edx = r_; fl.arith(r_); }  // 48d67c dec edx
    x.push(double(int32_t(rd32((ebp + 0x7au)))));  // 48d67d fild dword ptr [ebp + 0x7a]
    x.fxch(1);  // 48d680 fxch st(1)
    x.st(0) = (x.st(0)) + (x.st(1));  // 48d682 fadd st(1)
    eax = rd32((ebp + 0x6au));  // 48d684 mov eax, dword ptr [ebp + 0x6a]
    x.fstp_st(1);  // 48d687 fstp st(1)
    wr32((ebp + 0x7eu), eax);  // 48d689 mov dword ptr [ebp + 0x7e], eax
    x.st(0) = std::trunc(x.st(0));  // __CHP  // 48d68c call 0x4dfd56
    x.push(double(int32_t(rd32((ebp + 0x7eu)))));  // 48d691 fild dword ptr [ebp + 0x7e]
    x.fxch(2);  // 48d694 fxch st(2)
    x.st(0) = (x.st(0)) + (x.st(2));  // 48d696 fadd st(2)
    x.fxch(1);  // 48d698 fxch st(1)
    wr32((ebp + 0x6eu), stored(cpu, x.pop()));  // 48d69a fistp dword ptr [ebp + 0x6e]
    x.fstp_st(1);  // 48d69d fstp st(1)
    x.st(0) = std::trunc(x.st(0));  // __CHP  // 48d69f call 0x4dfd56
    wr32((ebp + 0x6au), stored(cpu, x.pop()));  // 48d6a4 fistp dword ptr [ebp + 0x6a]
    fl.logic(edx & edx);  // 48d6a7 test edx, edx
    if (fl.l()) goto L_48d630;  // 48d6a9 jl 0x48d630
    goto L_48d632;  // 48d6ab jmp 0x48d632
L_48d6ad:
    edx = rd32((ebp + 0x6au));  // 48d6ad mov edx, dword ptr [ebp + 0x6a]
    eax = rd32((ebp + 0x6eu));  // 48d6b0 mov eax, dword ptr [ebp + 0x6e]
    ecx = rd32((ebp + 0x72u));  // 48d6b3 mov ecx, dword ptr [ebp + 0x72]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4e8f28, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48d6b6 call 0x4e8f28
    esp += 0u;  // 48d6b6 call 0x4e8f28
    edi = rd32((ecx + 0x998u));  // 48d6bb mov edi, dword ptr [ecx + 0x998]
    edx = eax;  // 48d6c1 mov edx, eax
    fl.sub(edi, 148u);  // 48d6c3 cmp edi, 0x94
    if (fl.l()) goto L_48d6dc;  // 48d6c9 jl 0x48d6dc
    { const reg32 a_ = eax, b_ = 512u; eax = a_ + b_; fl.add(a_, b_); }  // 48d6cb add eax, 0x200
    fl.sub(eax, 512u);  // 48d6d0 cmp eax, 0x200
    if (fl.l()) goto L_48d6dc;  // 48d6d5 jl 0x48d6dc
    { const reg32 a_ = eax, b_ = 1024u; eax = a_ - b_; fl.sub(a_, b_); }  // 48d6d7 sub eax, 0x400
L_48d6dc:
    fl.sub(rd32((0x6fd4c0u)), 0u);  // 48d6dc cmp dword ptr [0x6fd4c0], 0
    if (fl.e()) goto L_48d6f6;  // 48d6e3 je 0x48d6f6
    { const reg32 a_ = eax, b_ = 512u; eax = a_ + b_; fl.add(a_, b_); }  // 48d6e5 add eax, 0x200
    fl.sub(eax, 512u);  // 48d6ea cmp eax, 0x200
    if (fl.l()) goto L_48d6f6;  // 48d6ef jl 0x48d6f6
    { const reg32 a_ = eax, b_ = 1024u; eax = a_ - b_; fl.sub(a_, b_); }  // 48d6f1 sub eax, 0x400
L_48d6f6:
    wr32((ebp + 0x7eu), eax);  // 48d6f6 mov dword ptr [ebp + 0x7e], eax
    x.push(double(int32_t(rd32((ebp + 0x7eu)))));  // 48d6f9 fild dword ptr [ebp + 0x7e]
    x.st(0) = (x.st(0)) * (F(0x53b9ecu));  // 48d6fc fmul dword ptr [0x53b9ec]
    ebx = 7942396u;  // 48d702 mov ebx, 0x7930fc
    edx = 7942400u;  // 48d707 mov edx, 0x793100
    eax = (ebp - 0x1eu);  // 48d70c lea eax, [ebp - 0x1e]
    SF((ebp - 0x1eu), x.st(0));  // 48d70f fstp dword ptr [ebp - 0x1e]
    x.pop();  // 48d70f fstp dword ptr [ebp - 0x1e]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4ea970, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48d712 call 0x4ea970
    esp += 0u;  // 48d712 call 0x4ea970
L_48d717:
    eax = (ebp - 0x4eu);  // 48d717 lea eax, [ebp - 0x4e]
    esp -= 4; wr32(esp, eax);  // 48d71a push eax
    eax = rd32((ebp + 0x72u));  // 48d71b mov eax, dword ptr [ebp + 0x72]
    esp -= 4; wr32(esp, rd32((eax + 0xa0u)));  // 48d71e push dword ptr [eax + 0xa0]
    esp -= 4; wr32(esp, rd32((eax + 0x98u)));  // 48d724 push dword ptr [eax + 0x98]
    eax = rd32((ebp + 0x76u));  // 48d72a mov eax, dword ptr [ebp + 0x76]
    ecx = rd32((ebp + 0x26u));  // 48d72d mov ecx, dword ptr [ebp + 0x26]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x48cb50, esp, 3, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48d730 call 0x48cb50
    esp += 12u;  // 48d730 call 0x48cb50
    fl.sub(ecx, 1065353216u);  // 48d735 cmp ecx, 0x3f800000
    if (fl.le()) goto L_48d7ac;  // 48d73b jle 0x48d7ac
    x.push(F(ebp - 0x4eu));  // 48d73d fld dword ptr [ebp - 0x4e]
    eax = rd32((ebp + 0x76u));  // 48d740 mov eax, dword ptr [ebp + 0x76]
    x.push(F(ebp - 0x4au));  // 48d743 fld dword ptr [ebp - 0x4a]
    x.push(F(eax * 4u + 0x792890u));  // 48d746 fld dword ptr [eax*4 + 0x792890]
    x.push(F(eax * 4u + 0x792888u));  // 48d74d fld dword ptr [eax*4 + 0x792888]
    x.fxch(3);  // 48d754 fxch st(3)
    x.st(0) = (x.st(0)) - (F(eax * 4u + 0x792890u));  // 48d756 fsub dword ptr [eax*4 + 0x792890]
    x.fxch(2);  // 48d75d fxch st(2)
    x.st(0) = (x.st(0)) - (F(eax * 4u + 0x792888u));  // 48d75f fsub dword ptr [eax*4 + 0x792888]
    x.fxch(2);  // 48d766 fxch st(2)
    SF((ebp - 0x4eu), x.st(0));  // 48d768 fstp dword ptr [ebp - 0x4e]
    x.pop();  // 48d768 fstp dword ptr [ebp - 0x4e]
    x.fxch(1);  // 48d76b fxch st(1)
    SF((ebp - 0x4au), x.st(0));  // 48d76d fstp dword ptr [ebp - 0x4a]
    x.pop();  // 48d76d fstp dword ptr [ebp - 0x4a]
    x.push(F(ebp - 0x4eu));  // 48d770 fld dword ptr [ebp - 0x4e]
    x.push(F(ebp - 0x4au));  // 48d773 fld dword ptr [ebp - 0x4a]
    x.fxch(2);  // 48d776 fxch st(2)
    x.st(0) = (x.st(0)) - (F(ebp - 0x4eu));  // 48d778 fsub dword ptr [ebp - 0x4e]
    x.fxch(3);  // 48d77b fxch st(3)
    x.st(0) = (x.st(0)) - (F(ebp - 0x4au));  // 48d77d fsub dword ptr [ebp - 0x4a]
    x.fxch(3);  // 48d780 fxch st(3)
    SF((eax * 4u + 0x792890u), x.st(0));  // 48d782 fstp dword ptr [eax*4 + 0x792890]
    x.pop();  // 48d782 fstp dword ptr [eax*4 + 0x792890]
    x.fxch(2);  // 48d789 fxch st(2)
    SF((eax * 4u + 0x792888u), x.st(0));  // 48d78b fstp dword ptr [eax*4 + 0x792888]
    x.pop();  // 48d78b fstp dword ptr [eax*4 + 0x792888]
    x.fxch(1);  // 48d792 fxch st(1)
    x.st(0) = (x.st(0)) + (F(eax * 4u + 0x792890u));  // 48d794 fadd dword ptr [eax*4 + 0x792890]
    x.fxch(1);  // 48d79b fxch st(1)
    x.st(0) = (x.st(0)) + (F(eax * 4u + 0x792888u));  // 48d79d fadd dword ptr [eax*4 + 0x792888]
    x.fxch(1);  // 48d7a4 fxch st(1)
    SF((ebp - 0x4eu), x.st(0));  // 48d7a6 fstp dword ptr [ebp - 0x4e]
    x.pop();  // 48d7a6 fstp dword ptr [ebp - 0x4e]
    SF((ebp - 0x4au), x.st(0));  // 48d7a9 fstp dword ptr [ebp - 0x4a]
    x.pop();  // 48d7a9 fstp dword ptr [ebp - 0x4a]
L_48d7ac:
    fl.sub(rd32((0x55fe3cu)), 0u);  // 48d7ac cmp dword ptr [0x55fe3c], 0
    if (fl.e()) goto L_48d848;  // 48d7b3 je 0x48d848
    x.push(F(0x53b9e0u));  // 48d7b9 fld dword ptr [0x53b9e0]
    x.push(F(0x793104u));  // 48d7bf fld dword ptr [0x793104]
    x.st(0) = (x.st(0)) * (x.st(1));  // 48d7c5 fmul st(1)
    x.push(F(0x7930f4u));  // 48d7c7 fld dword ptr [0x7930f4]
    x.st(2) = (x.st(2)) * (x.st(0));  // 48d7cd fmulp st(2)
    x.pop();  // 48d7cd fmulp st(2)
    x.push(F(ebp - 0x4eu));  // 48d7cf fld dword ptr [ebp - 0x4e]
    x.push(F(ebp - 0x4au));  // 48d7d2 fld dword ptr [ebp - 0x4a]
    x.push(F(ebp - 0x4eu));  // 48d7d5 fld dword ptr [ebp - 0x4e]
    edx = rd32((ebp + 0x76u));  // 48d7d8 mov edx, dword ptr [ebp + 0x76]
    x.push(F(ebp - 0x4au));  // 48d7db fld dword ptr [ebp - 0x4a]
    eax = (edx * 4u);  // 48d7de lea eax, [edx*4]
    ecx = rd32((edx * 4u + 0x55e2b4u));  // 48d7e5 mov ecx, dword ptr [edx*4 + 0x55e2b4]
    esi = rd32((edx * 4u + 0x55e2b4u));  // 48d7ec mov esi, dword ptr [edx*4 + 0x55e2b4]
    ebx = (ecx * 4u);  // 48d7f3 lea ebx, [ecx*4]
    { const reg32 a_ = eax, b_ = edx; eax = a_ + b_; fl.add(a_, b_); }  // 48d7fa add eax, edx
    x.fxch(3);  // 48d7fc fxch st(3)
    x.st(0) = (x.st(0)) - (x.st(4));  // 48d7fe fsub st(4)
    carry_ = (eax >> 26) & 1u;  // 48d800 shl eax, 6
    { const reg32 r_ = eax << 6; eax = r_; fl.logic(r_); }  // 48d800 shl eax, 6
    x.fxch(2);  // 48d803 fxch st(2)
    x.st(0) = (x.st(0)) - (x.st(5));  // 48d805 fsub st(5)
    x.fxch(1);  // 48d807 fxch st(1)
    x.st(4) = (x.st(4)) + (x.st(0));  // 48d809 faddp st(4)
    x.pop();  // 48d809 faddp st(4)
    { const reg32 a_ = ebx, b_ = ecx; ebx = a_ + b_; fl.add(a_, b_); }  // 48d80b add ebx, ecx
    x.fxch(2);  // 48d80d fxch st(2)
    x.st(4) = (x.st(4)) + (x.st(0));  // 48d80f faddp st(4)
    x.pop();  // 48d80f faddp st(4)
    carry_ = (ebx >> 30) & 1u;  // 48d811 shl ebx, 2
    { const reg32 r_ = ebx << 2; ebx = r_; fl.logic(r_); }  // 48d811 shl ebx, 2
    ecx = rd32((ebp + 0x72u));  // 48d814 mov ecx, dword ptr [ebp + 0x72]
    SF((ebx + eax + 0x7925f8u), x.st(0));  // 48d817 fstp dword ptr [ebx + eax + 0x7925f8]
    x.pop();  // 48d817 fstp dword ptr [ebx + eax + 0x7925f8]
    SF((ebx + eax + 0x7925fcu), x.st(0));  // 48d81e fstp dword ptr [ebx + eax + 0x7925fc]
    x.pop();  // 48d81e fstp dword ptr [ebx + eax + 0x7925fc]
    SF((ebx + eax + 0x792600u), x.st(0));  // 48d825 fstp dword ptr [ebx + eax + 0x792600]
    x.pop();  // 48d825 fstp dword ptr [ebx + eax + 0x792600]
    SF((ebx + eax + 0x792604u), x.st(0));  // 48d82c fstp dword ptr [ebx + eax + 0x792604]
    x.pop();  // 48d82c fstp dword ptr [ebx + eax + 0x792604]
    ecx = rd32((ecx + 0x1f0u));  // 48d833 mov ecx, dword ptr [ecx + 0x1f0]
    { const reg32 r_ = esi + 1u; esi = r_; fl.arith(r_); }  // 48d839 inc esi
    wr32((ebx + eax + 0x792608u), ecx);  // 48d83a mov dword ptr [ebx + eax + 0x792608], ecx
    wr32((edx * 4u + 0x55e2b4u), esi);  // 48d841 mov dword ptr [edx*4 + 0x55e2b4], esi
L_48d848:
    eax = rd32((ebp + 0x76u));  // 48d848 mov eax, dword ptr [ebp + 0x76]
    fl.sub(rd32((eax * 4u + 0x7925d0u)), 0u);  // 48d84b cmp dword ptr [eax*4 + 0x7925d0], 0
    if (fl.e()) goto L_48db5a;  // 48d853 je 0x48db5a
    fl.sub(rd32((0x7cdac8u)), 640u);  // 48d859 cmp dword ptr [0x7cdac8], 0x280
    if (fl.ge()) goto L_48db5a;  // 48d863 jge 0x48db5a
    edx = reg32(int32_t(rd32((0x79c110u))) * int32_t(840u));  // 48d869 imul edx, dword ptr [0x79c110], 0x348
    eax = reg32(int32_t(rd32((0x79c10cu))) * int32_t(420u));  // 48d873 imul eax, dword ptr [0x79c10c], 0x1a4
    ecx = 0u;  // 48d87d xor ecx, ecx
    fl.logic(0u);  // 48d87d xor ecx, ecx
    edi = rd32((edx + eax + 0x6fbc70u));  // 48d87f mov edi, dword ptr [edx + eax + 0x6fbc70]
    fl.sub(edi, 1u);  // 48d886 cmp edi, 1
    if (fl.ne()) goto L_48d8f1;  // 48d889 jne 0x48d8f1
    x.push(F(0x79289cu));  // 48d88b fld dword ptr [0x79289c]
    x.st(0) = (x.st(0)) * (F(0x793100u));  // 48d891 fmul dword ptr [0x793100]
    x.push(F(0x79289cu));  // 48d897 fld dword ptr [0x79289c]
    x.st(0) = (x.st(0)) * (F(0x7930fcu));  // 48d89d fmul dword ptr [0x7930fc]
    x.push(F(0x792898u));  // 48d8a3 fld dword ptr [0x792898]
    x.st(0) = (x.st(0)) * (F(0x7930fcu));  // 48d8a9 fmul dword ptr [0x7930fc]
    x.push(F(0x792898u));  // 48d8af fld dword ptr [0x792898]
    x.fxch(1);  // 48d8b5 fxch st(1)
    x.st(3) = (x.st(0)) - (x.st(3));  // 48d8b7 fsubrp st(3)
    x.pop();  // 48d8b7 fsubrp st(3)
    x.st(0) = (x.st(0)) * (F(0x793100u));  // 48d8b9 fmul dword ptr [0x793100]
    x.fxch(2);  // 48d8bf fxch st(2)
    SF((ebp + 0x46u), x.st(0));  // 48d8c1 fstp dword ptr [ebp + 0x46]
    x.pop();  // 48d8c1 fstp dword ptr [ebp + 0x46]
    eax = rd32((ebp + 0x46u));  // 48d8c4 mov eax, dword ptr [ebp + 0x46]
    x.st(1) = (x.st(1)) + (x.st(0));  // 48d8c7 faddp st(1)
    x.pop();  // 48d8c7 faddp st(1)
    { const reg32 r_ = eax | 2147483648u; eax = r_; fl.logic(r_); }  // 48d8c9 or eax, 0x80000000
    SF((ebp + 0x42u), x.st(0));  // 48d8ce fstp dword ptr [ebp + 0x42]
    x.pop();  // 48d8ce fstp dword ptr [ebp + 0x42]
    fl.sub(eax, 4290772992u);  // 48d8d1 cmp eax, 0xffc00000
    if (fl.ne()) goto L_48d8db;  // 48d8d6 jne 0x48d8db
    wr32((ebp + 0x46u), ecx);  // 48d8d8 mov dword ptr [ebp + 0x46], ecx
L_48d8db:
    eax = rd32((ebp + 0x42u));  // 48d8db mov eax, dword ptr [ebp + 0x42]
    { const reg32 r_ = eax | 2147483648u; eax = r_; fl.logic(r_); }  // 48d8de or eax, 0x80000000
    fl.sub(eax, 4290772992u);  // 48d8e3 cmp eax, 0xffc00000
    if (fl.ne()) goto L_48d901;  // 48d8e8 jne 0x48d901
    edx = 0u;  // 48d8ea xor edx, edx
    fl.logic(0u);  // 48d8ea xor edx, edx
    wr32((ebp + 0x42u), edx);  // 48d8ec mov dword ptr [ebp + 0x42], edx
    goto L_48d901;  // 48d8ef jmp 0x48d901
L_48d8f1:
    eax = rd32((0x792898u));  // 48d8f1 mov eax, dword ptr [0x792898]
    wr32((ebp + 0x46u), eax);  // 48d8f6 mov dword ptr [ebp + 0x46], eax
    eax = rd32((0x79289cu));  // 48d8f9 mov eax, dword ptr [0x79289c]
    wr32((ebp + 0x42u), eax);  // 48d8fe mov dword ptr [ebp + 0x42], eax
L_48d901:
    fl.sub(rd32((0x6fd4c4u)), 0u);  // 48d901 cmp dword ptr [0x6fd4c4], 0
    if (fl.e()) goto L_48d913;  // 48d908 je 0x48d913
    wr32((ebp + 0xau), 3212836864u);  // 48d90a mov dword ptr [ebp + 0xa], 0xbf800000
    goto L_48d91a;  // 48d911 jmp 0x48d91a
L_48d913:
    wr32((ebp + 0xau), 1065353216u);  // 48d913 mov dword ptr [ebp + 0xa], 0x3f800000
L_48d91a:
    eax = rd32((ebp + 0x76u));  // 48d91a mov eax, dword ptr [ebp + 0x76]
    x.push(F(ebp + 0xau));  // 48d91d fld dword ptr [ebp + 0xa]
    x.st(0) = (x.st(0)) * (F(eax * 4u + 0x792878u));  // 48d920 fmul dword ptr [eax*4 + 0x792878]
    x.push(F(eax * 4u + 0x792878u));  // 48d927 fld dword ptr [eax*4 + 0x792878]
    x.st(0) = (x.st(0)) * (F(ebp + 0x42u));  // 48d92e fmul dword ptr [ebp + 0x42]
    edx = eax;  // 48d931 mov edx, eax
    x.fxch(1);  // 48d933 fxch st(1)
    x.st(0) = (x.st(0)) * (F(ebp + 0x46u));  // 48d935 fmul dword ptr [ebp + 0x46]
    carry_ = (edx >> 28) & 1u;  // 48d938 shl edx, 4
    { const reg32 r_ = edx << 4; edx = r_; fl.logic(r_); }  // 48d938 shl edx, 4
    x.fxch(1);  // 48d93b fxch st(1)
    x.st(0) = (F(eax * 4u + 0x792888u)) - (x.st(0));  // 48d93d fsubr dword ptr [eax*4 + 0x792888]
    x.fxch(1);  // 48d944 fxch st(1)
    x.st(0) = (x.st(0)) + (F(eax * 4u + 0x792890u));  // 48d946 fadd dword ptr [eax*4 + 0x792890]
    x.fxch(1);  // 48d94d fxch st(1)
    SF((ebp - 0x3au), x.st(0));  // 48d94f fstp dword ptr [ebp - 0x3a]
    x.pop();  // 48d94f fstp dword ptr [ebp - 0x3a]
    SF((ebp - 0x3eu), x.st(0));  // 48d952 fst dword ptr [ebp - 0x3e]
    fl.fcmp(x.st(0), F(edx + 0x7925dcu));  // 48d955 fcomp dword ptr [edx + 0x7925dc]
    x.pop();  // 48d955 fcomp dword ptr [edx + 0x7925dc]
    // fnstsw ax  // 48d95b fnstsw ax
    // sahf  // 48d95d sahf
    if (fl.ae()) goto L_48d965;  // 48d95e jae 0x48d965
    { const reg32 r_ = (ecx & 0xffu) | 1u; ecx = (ecx & 0xffffff00u) | ((r_) & 0xffu); fl.logic(r_); }  // 48d960 or cl, 1
    goto L_48d976;  // 48d963 jmp 0x48d976
L_48d965:
    x.push(F(ebp - 0x3eu));  // 48d965 fld dword ptr [ebp - 0x3e]
    fl.fcmp(x.st(0), F(edx + 0x7925e4u));  // 48d968 fcomp dword ptr [edx + 0x7925e4]
    x.pop();  // 48d968 fcomp dword ptr [edx + 0x7925e4]
    // fnstsw ax  // 48d96e fnstsw ax
    // sahf  // 48d970 sahf
    if (fl.b()) goto L_48d976;  // 48d971 jb 0x48d976
    { const reg32 r_ = (ecx & 0xffu) | 2u; ecx = (ecx & 0xffffff00u) | ((r_) & 0xffu); fl.logic(r_); }  // 48d973 or cl, 2
L_48d976:
    edx = rd32((ebp + 0x76u));  // 48d976 mov edx, dword ptr [ebp + 0x76]
    carry_ = (edx >> 28) & 1u;  // 48d979 shl edx, 4
    { const reg32 r_ = edx << 4; edx = r_; fl.logic(r_); }  // 48d979 shl edx, 4
    x.push(F(ebp - 0x3au));  // 48d97c fld dword ptr [ebp - 0x3a]
    fl.fcmp(x.st(0), F(edx + 0x7925d8u));  // 48d97f fcomp dword ptr [edx + 0x7925d8]
    x.pop();  // 48d97f fcomp dword ptr [edx + 0x7925d8]
    // fnstsw ax  // 48d985 fnstsw ax
    // sahf  // 48d987 sahf
    if (fl.ae()) goto L_48d98f;  // 48d988 jae 0x48d98f
    { const reg32 r_ = (ecx & 0xffu) | 8u; ecx = (ecx & 0xffffff00u) | ((r_) & 0xffu); fl.logic(r_); }  // 48d98a or cl, 8
    goto L_48d9a0;  // 48d98d jmp 0x48d9a0
L_48d98f:
    x.push(F(ebp - 0x3au));  // 48d98f fld dword ptr [ebp - 0x3a]
    fl.fcmp(x.st(0), F(edx + 0x7925e0u));  // 48d992 fcomp dword ptr [edx + 0x7925e0]
    x.pop();  // 48d992 fcomp dword ptr [edx + 0x7925e0]
    // fnstsw ax  // 48d998 fnstsw ax
    // sahf  // 48d99a sahf
    if (fl.b()) goto L_48d9a0;  // 48d99b jb 0x48d9a0
    { const reg32 r_ = (ecx & 0xffu) | 4u; ecx = (ecx & 0xffffff00u) | ((r_) & 0xffu); fl.logic(r_); }  // 48d99d or cl, 4
L_48d9a0:
    wr32((ebp + 0x5au), 1u);  // 48d9a0 mov dword ptr [ebp + 0x5a], 1
L_48d9a7:
    eax = rd32((ebp + 0x5au));  // 48d9a7 mov eax, dword ptr [ebp + 0x5a]
    fl.sub(eax, rd32((0x79c118u)));  // 48d9aa cmp eax, dword ptr [0x79c118]
    if (fl.ge()) goto L_48dd8e;  // 48d9b0 jge 0x48dd8e
    edi = (ebp - 0x26u);  // 48d9b6 lea edi, [ebp - 0x26]
    esi = (ebp - 0x3eu);  // 48d9b9 lea esi, [ebp - 0x3e]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 48d9bc movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 48d9bd movsd dword ptr es:[edi], dword ptr [esi]
    eax = reg32(int32_t(rd32((0x79c110u))) * int32_t(840u));  // 48d9be imul eax, dword ptr [0x79c110], 0x348
    edx = reg32(int32_t(rd32((0x79c10cu))) * int32_t(420u));  // 48d9c8 imul edx, dword ptr [0x79c10c], 0x1a4
    ebx = ecx;  // 48d9d2 mov ebx, ecx
    { const reg32 a_ = edx, b_ = eax; edx = a_ + b_; fl.add(a_, b_); }  // 48d9d4 add edx, eax
    ecx = 0u;  // 48d9d6 xor ecx, ecx
    fl.logic(0u);  // 48d9d6 xor ecx, ecx
    eax = rd32((ebp + 0x5au));  // 48d9d8 mov eax, dword ptr [ebp + 0x5a]
    esi = rd32((edx + 0x6fbc70u));  // 48d9db mov esi, dword ptr [edx + 0x6fbc70]
    carry_ = (eax >> 29) & 1u;  // 48d9e1 shl eax, 3
    { const reg32 r_ = eax << 3; eax = r_; fl.logic(r_); }  // 48d9e1 shl eax, 3
    fl.sub(esi, 1u);  // 48d9e4 cmp esi, 1
    if (fl.ne()) goto L_48da4f;  // 48d9e7 jne 0x48da4f
    x.push(F(eax + 0x79289cu));  // 48d9e9 fld dword ptr [eax + 0x79289c]
    x.st(0) = (x.st(0)) * (F(0x793100u));  // 48d9ef fmul dword ptr [0x793100]
    x.push(F(eax + 0x79289cu));  // 48d9f5 fld dword ptr [eax + 0x79289c]
    x.st(0) = (x.st(0)) * (F(0x7930fcu));  // 48d9fb fmul dword ptr [0x7930fc]
    x.push(F(eax + 0x792898u));  // 48da01 fld dword ptr [eax + 0x792898]
    x.st(0) = (x.st(0)) * (F(0x7930fcu));  // 48da07 fmul dword ptr [0x7930fc]
    x.push(F(eax + 0x792898u));  // 48da0d fld dword ptr [eax + 0x792898]
    x.fxch(1);  // 48da13 fxch st(1)
    x.st(3) = (x.st(0)) - (x.st(3));  // 48da15 fsubrp st(3)
    x.pop();  // 48da15 fsubrp st(3)
    x.st(0) = (x.st(0)) * (F(0x793100u));  // 48da17 fmul dword ptr [0x793100]
    x.fxch(2);  // 48da1d fxch st(2)
    SF((ebp + 0x4au), x.st(0));  // 48da1f fstp dword ptr [ebp + 0x4a]
    x.pop();  // 48da1f fstp dword ptr [ebp + 0x4a]
    eax = rd32((ebp + 0x4au));  // 48da22 mov eax, dword ptr [ebp + 0x4a]
    x.st(1) = (x.st(1)) + (x.st(0));  // 48da25 faddp st(1)
    x.pop();  // 48da25 faddp st(1)
    { const reg32 r_ = eax | 2147483648u; eax = r_; fl.logic(r_); }  // 48da27 or eax, 0x80000000
    SF((ebp + 0x56u), x.st(0));  // 48da2c fstp dword ptr [ebp + 0x56]
    x.pop();  // 48da2c fstp dword ptr [ebp + 0x56]
    fl.sub(eax, 4290772992u);  // 48da2f cmp eax, 0xffc00000
    if (fl.ne()) goto L_48da39;  // 48da34 jne 0x48da39
    wr32((ebp + 0x4au), ecx);  // 48da36 mov dword ptr [ebp + 0x4a], ecx
L_48da39:
    eax = rd32((ebp + 0x56u));  // 48da39 mov eax, dword ptr [ebp + 0x56]
    { const reg32 r_ = eax | 2147483648u; eax = r_; fl.logic(r_); }  // 48da3c or eax, 0x80000000
    fl.sub(eax, 4290772992u);  // 48da41 cmp eax, 0xffc00000
    if (fl.ne()) goto L_48da61;  // 48da46 jne 0x48da61
    eax = 0u;  // 48da48 xor eax, eax
    fl.logic(0u);  // 48da48 xor eax, eax
    wr32((ebp + 0x56u), eax);  // 48da4a mov dword ptr [ebp + 0x56], eax
    goto L_48da61;  // 48da4d jmp 0x48da61
L_48da4f:
    edx = rd32((eax + 0x792898u));  // 48da4f mov edx, dword ptr [eax + 0x792898]
    eax = rd32((eax + 0x79289cu));  // 48da55 mov eax, dword ptr [eax + 0x79289c]
    wr32((ebp + 0x4au), edx);  // 48da5b mov dword ptr [ebp + 0x4a], edx
    wr32((ebp + 0x56u), eax);  // 48da5e mov dword ptr [ebp + 0x56], eax
L_48da61:
    fl.sub(rd32((0x6fd4c4u)), 0u);  // 48da61 cmp dword ptr [0x6fd4c4], 0
    if (fl.e()) goto L_48da73;  // 48da68 je 0x48da73
    wr32((ebp + 6u), 3212836864u);  // 48da6a mov dword ptr [ebp + 6], 0xbf800000
    goto L_48da7a;  // 48da71 jmp 0x48da7a
L_48da73:
    wr32((ebp + 6u), 1065353216u);  // 48da73 mov dword ptr [ebp + 6], 0x3f800000
L_48da7a:
    eax = rd32((ebp + 0x76u));  // 48da7a mov eax, dword ptr [ebp + 0x76]
    x.push(F(ebp + 6u));  // 48da7d fld dword ptr [ebp + 6]
    x.st(0) = (x.st(0)) * (F(eax * 4u + 0x792878u));  // 48da80 fmul dword ptr [eax*4 + 0x792878]
    x.push(F(eax * 4u + 0x792878u));  // 48da87 fld dword ptr [eax*4 + 0x792878]
    x.st(0) = (x.st(0)) * (F(ebp + 0x56u));  // 48da8e fmul dword ptr [ebp + 0x56]
    edx = eax;  // 48da91 mov edx, eax
    x.fxch(1);  // 48da93 fxch st(1)
    x.st(0) = (x.st(0)) * (F(ebp + 0x4au));  // 48da95 fmul dword ptr [ebp + 0x4a]
    carry_ = (edx >> 28) & 1u;  // 48da98 shl edx, 4
    { const reg32 r_ = edx << 4; edx = r_; fl.logic(r_); }  // 48da98 shl edx, 4
    x.fxch(1);  // 48da9b fxch st(1)
    x.st(0) = (F(eax * 4u + 0x792888u)) - (x.st(0));  // 48da9d fsubr dword ptr [eax*4 + 0x792888]
    x.fxch(1);  // 48daa4 fxch st(1)
    x.st(0) = (x.st(0)) + (F(eax * 4u + 0x792890u));  // 48daa6 fadd dword ptr [eax*4 + 0x792890]
    x.fxch(1);  // 48daad fxch st(1)
    SF((ebp - 0x3au), x.st(0));  // 48daaf fstp dword ptr [ebp - 0x3a]
    x.pop();  // 48daaf fstp dword ptr [ebp - 0x3a]
    SF((ebp - 0x3eu), x.st(0));  // 48dab2 fst dword ptr [ebp - 0x3e]
    fl.fcmp(x.st(0), F(edx + 0x7925dcu));  // 48dab5 fcomp dword ptr [edx + 0x7925dc]
    x.pop();  // 48dab5 fcomp dword ptr [edx + 0x7925dc]
    // fnstsw ax  // 48dabb fnstsw ax
    // sahf  // 48dabd sahf
    if (fl.ae()) goto L_48dac5;  // 48dabe jae 0x48dac5
    { const reg32 r_ = (ecx & 0xffu) | 1u; ecx = (ecx & 0xffffff00u) | ((r_) & 0xffu); fl.logic(r_); }  // 48dac0 or cl, 1
    goto L_48dad6;  // 48dac3 jmp 0x48dad6
L_48dac5:
    x.push(F(ebp - 0x3eu));  // 48dac5 fld dword ptr [ebp - 0x3e]
    fl.fcmp(x.st(0), F(edx + 0x7925e4u));  // 48dac8 fcomp dword ptr [edx + 0x7925e4]
    x.pop();  // 48dac8 fcomp dword ptr [edx + 0x7925e4]
    // fnstsw ax  // 48dace fnstsw ax
    // sahf  // 48dad0 sahf
    if (fl.b()) goto L_48dad6;  // 48dad1 jb 0x48dad6
    { const reg32 r_ = (ecx & 0xffu) | 2u; ecx = (ecx & 0xffffff00u) | ((r_) & 0xffu); fl.logic(r_); }  // 48dad3 or cl, 2
L_48dad6:
    edx = rd32((ebp + 0x76u));  // 48dad6 mov edx, dword ptr [ebp + 0x76]
    carry_ = (edx >> 28) & 1u;  // 48dad9 shl edx, 4
    { const reg32 r_ = edx << 4; edx = r_; fl.logic(r_); }  // 48dad9 shl edx, 4
    x.push(F(ebp - 0x3au));  // 48dadc fld dword ptr [ebp - 0x3a]
    fl.fcmp(x.st(0), F(edx + 0x7925d8u));  // 48dadf fcomp dword ptr [edx + 0x7925d8]
    x.pop();  // 48dadf fcomp dword ptr [edx + 0x7925d8]
    // fnstsw ax  // 48dae5 fnstsw ax
    // sahf  // 48dae7 sahf
    if (fl.ae()) goto L_48daef;  // 48dae8 jae 0x48daef
    { const reg32 r_ = (ecx & 0xffu) | 8u; ecx = (ecx & 0xffffff00u) | ((r_) & 0xffu); fl.logic(r_); }  // 48daea or cl, 8
    goto L_48db00;  // 48daed jmp 0x48db00
L_48daef:
    x.push(F(ebp - 0x3au));  // 48daef fld dword ptr [ebp - 0x3a]
    fl.fcmp(x.st(0), F(edx + 0x7925e0u));  // 48daf2 fcomp dword ptr [edx + 0x7925e0]
    x.pop();  // 48daf2 fcomp dword ptr [edx + 0x7925e0]
    // fnstsw ax  // 48daf8 fnstsw ax
    // sahf  // 48dafa sahf
    if (fl.b()) goto L_48db00;  // 48dafb jb 0x48db00
    { const reg32 r_ = (ecx & 0xffu) | 4u; ecx = (ecx & 0xffffff00u) | ((r_) & 0xffu); fl.logic(r_); }  // 48dafd or cl, 4
L_48db00:
    eax = ecx;  // 48db00 mov eax, ecx
    { const reg32 r_ = eax | ebx; eax = r_; fl.logic(r_); }  // 48db02 or eax, ebx
    if (fl.ne()) goto L_48db52;  // 48db04 jne 0x48db52
    ebx = (ebp - 0x1au);  // 48db06 lea ebx, [ebp - 0x1a]
    edx = 1u;  // 48db09 mov edx, 1
    eax = 7942408u;  // 48db0e mov eax, 0x793108
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x49bf40, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48db13 call 0x49bf40
    esp += 0u;  // 48db13 call 0x49bf40
    edx = rd32((ebp - 0x1au));  // 48db18 mov edx, dword ptr [ebp - 0x1a]
    eax = rd32((ebp - 0x26u));  // 48db1b mov eax, dword ptr [ebp - 0x26]
    wr32((edx), eax);  // 48db1e mov dword ptr [edx], eax
    edx = rd32((ebp - 0x1au));  // 48db20 mov edx, dword ptr [ebp - 0x1a]
    eax = rd32((ebp - 0x22u));  // 48db23 mov eax, dword ptr [ebp - 0x22]
    wr32((edx + 4u), eax);  // 48db26 mov dword ptr [edx + 4], eax
    eax = rd32((ebp - 0x1au));  // 48db29 mov eax, dword ptr [ebp - 0x1a]
    wr32((eax + 0x10u), 4294967295u);  // 48db2c mov dword ptr [eax + 0x10], 0xffffffff
    { const reg32 a_ = rd32((ebp - 0x1au)), b_ = 32u; wr32((ebp - 0x1au), a_ + b_); fl.add(a_, b_); }  // 48db33 add dword ptr [ebp - 0x1a], 0x20
    edx = rd32((ebp - 0x1au));  // 48db37 mov edx, dword ptr [ebp - 0x1a]
    eax = rd32((ebp - 0x3eu));  // 48db3a mov eax, dword ptr [ebp - 0x3e]
    wr32((edx), eax);  // 48db3d mov dword ptr [edx], eax
    edx = rd32((ebp - 0x1au));  // 48db3f mov edx, dword ptr [ebp - 0x1a]
    eax = rd32((ebp - 0x3au));  // 48db42 mov eax, dword ptr [ebp - 0x3a]
    wr32((edx + 4u), eax);  // 48db45 mov dword ptr [edx + 4], eax
    eax = rd32((ebp - 0x1au));  // 48db48 mov eax, dword ptr [ebp - 0x1a]
    wr32((eax + 0x10u), 4294967295u);  // 48db4b mov dword ptr [eax + 0x10], 0xffffffff
L_48db52:
    { const reg32 r_ = rd32((ebp + 0x5au)) + 1u; wr32((ebp + 0x5au), r_); fl.arith(r_); }  // 48db52 inc dword ptr [ebp + 0x5a]
    goto L_48d9a7;  // 48db55 jmp 0x48d9a7
L_48db5a:
    eax = reg32(int32_t(rd32((0x79c110u))) * int32_t(840u));  // 48db5a imul eax, dword ptr [0x79c110], 0x348
    edx = reg32(int32_t(rd32((0x79c10cu))) * int32_t(420u));  // 48db64 imul edx, dword ptr [0x79c10c], 0x1a4
    esi = rd32((edx + eax + 0x6fbc70u));  // 48db6e mov esi, dword ptr [edx + eax + 0x6fbc70]
    fl.sub(esi, 1u);  // 48db75 cmp esi, 1
    if (fl.ne()) goto L_48dbe2;  // 48db78 jne 0x48dbe2
    x.push(F(0x79289cu));  // 48db7a fld dword ptr [0x79289c]
    x.st(0) = (x.st(0)) * (F(0x793100u));  // 48db80 fmul dword ptr [0x793100]
    x.push(F(0x79289cu));  // 48db86 fld dword ptr [0x79289c]
    x.st(0) = (x.st(0)) * (F(0x7930fcu));  // 48db8c fmul dword ptr [0x7930fc]
    x.push(F(0x792898u));  // 48db92 fld dword ptr [0x792898]
    x.st(0) = (x.st(0)) * (F(0x7930fcu));  // 48db98 fmul dword ptr [0x7930fc]
    x.push(F(0x792898u));  // 48db9e fld dword ptr [0x792898]
    x.fxch(1);  // 48dba4 fxch st(1)
    x.st(3) = (x.st(0)) - (x.st(3));  // 48dba6 fsubrp st(3)
    x.pop();  // 48dba6 fsubrp st(3)
    x.st(0) = (x.st(0)) * (F(0x793100u));  // 48dba8 fmul dword ptr [0x793100]
    x.fxch(2);  // 48dbae fxch st(2)
    SF((ebp + 0x4eu), x.st(0));  // 48dbb0 fstp dword ptr [ebp + 0x4e]
    x.pop();  // 48dbb0 fstp dword ptr [ebp + 0x4e]
    eax = rd32((ebp + 0x4eu));  // 48dbb3 mov eax, dword ptr [ebp + 0x4e]
    x.st(1) = (x.st(1)) + (x.st(0));  // 48dbb6 faddp st(1)
    x.pop();  // 48dbb6 faddp st(1)
    { const reg32 r_ = eax | 2147483648u; eax = r_; fl.logic(r_); }  // 48dbb8 or eax, 0x80000000
    SF((ebp + 0x66u), x.st(0));  // 48dbbd fstp dword ptr [ebp + 0x66]
    x.pop();  // 48dbbd fstp dword ptr [ebp + 0x66]
    fl.sub(eax, 4290772992u);  // 48dbc0 cmp eax, 0xffc00000
    if (fl.ne()) goto L_48dbcc;  // 48dbc5 jne 0x48dbcc
    edi = 0u;  // 48dbc7 xor edi, edi
    fl.logic(0u);  // 48dbc7 xor edi, edi
    wr32((ebp + 0x4eu), edi);  // 48dbc9 mov dword ptr [ebp + 0x4e], edi
L_48dbcc:
    eax = rd32((ebp + 0x66u));  // 48dbcc mov eax, dword ptr [ebp + 0x66]
    { const reg32 r_ = eax | 2147483648u; eax = r_; fl.logic(r_); }  // 48dbcf or eax, 0x80000000
    fl.sub(eax, 4290772992u);  // 48dbd4 cmp eax, 0xffc00000
    if (fl.ne()) goto L_48dbf2;  // 48dbd9 jne 0x48dbf2
    eax = 0u;  // 48dbdb xor eax, eax
    fl.logic(0u);  // 48dbdb xor eax, eax
    wr32((ebp + 0x66u), eax);  // 48dbdd mov dword ptr [ebp + 0x66], eax
    goto L_48dbf2;  // 48dbe0 jmp 0x48dbf2
L_48dbe2:
    eax = rd32((0x792898u));  // 48dbe2 mov eax, dword ptr [0x792898]
    wr32((ebp + 0x4eu), eax);  // 48dbe7 mov dword ptr [ebp + 0x4e], eax
    eax = rd32((0x79289cu));  // 48dbea mov eax, dword ptr [0x79289c]
    wr32((ebp + 0x66u), eax);  // 48dbef mov dword ptr [ebp + 0x66], eax
L_48dbf2:
    fl.sub(rd32((0x6fd4c4u)), 0u);  // 48dbf2 cmp dword ptr [0x6fd4c4], 0
    if (fl.e()) goto L_48dc04;  // 48dbf9 je 0x48dc04
    wr32((ebp + 0x22u), 3212836864u);  // 48dbfb mov dword ptr [ebp + 0x22], 0xbf800000
    goto L_48dc0b;  // 48dc02 jmp 0x48dc0b
L_48dc04:
    wr32((ebp + 0x22u), 1065353216u);  // 48dc04 mov dword ptr [ebp + 0x22], 0x3f800000
L_48dc0b:
    eax = rd32((ebp + 0x76u));  // 48dc0b mov eax, dword ptr [ebp + 0x76]
    x.push(F(ebp + 0x22u));  // 48dc0e fld dword ptr [ebp + 0x22]
    x.st(0) = (x.st(0)) * (F(eax * 4u + 0x792878u));  // 48dc11 fmul dword ptr [eax*4 + 0x792878]
    x.push(F(eax * 4u + 0x792878u));  // 48dc18 fld dword ptr [eax*4 + 0x792878]
    x.st(0) = (x.st(0)) * (F(ebp + 0x66u));  // 48dc1f fmul dword ptr [ebp + 0x66]
    esi = 1u;  // 48dc22 mov esi, 1
    x.fxch(1);  // 48dc27 fxch st(1)
    x.st(0) = (x.st(0)) * (F(ebp + 0x4eu));  // 48dc29 fmul dword ptr [ebp + 0x4e]
    wr32((ebp + 0x5eu), esi);  // 48dc2c mov dword ptr [ebp + 0x5e], esi
    x.fxch(1);  // 48dc2f fxch st(1)
    x.st(0) = (F(eax * 4u + 0x792888u)) - (x.st(0));  // 48dc31 fsubr dword ptr [eax*4 + 0x792888]
    x.fxch(1);  // 48dc38 fxch st(1)
    x.st(0) = (x.st(0)) + (F(eax * 4u + 0x792890u));  // 48dc3a fadd dword ptr [eax*4 + 0x792890]
    x.fxch(1);  // 48dc41 fxch st(1)
    SF((ebp - 0x22u), x.st(0));  // 48dc43 fstp dword ptr [ebp - 0x22]
    x.pop();  // 48dc43 fstp dword ptr [ebp - 0x22]
    SF((ebp - 0x26u), x.st(0));  // 48dc46 fstp dword ptr [ebp - 0x26]
    x.pop();  // 48dc46 fstp dword ptr [ebp - 0x26]
L_48dc49:
    eax = rd32((ebp + 0x5eu));  // 48dc49 mov eax, dword ptr [ebp + 0x5e]
    fl.sub(eax, rd32((0x79c118u)));  // 48dc4c cmp eax, dword ptr [0x79c118]
    if (fl.ge()) goto L_48dd8e;  // 48dc52 jge 0x48dd8e
    eax = reg32(int32_t(rd32((0x79c110u))) * int32_t(840u));  // 48dc58 imul eax, dword ptr [0x79c110], 0x348
    edx = reg32(int32_t(rd32((0x79c10cu))) * int32_t(420u));  // 48dc62 imul edx, dword ptr [0x79c10c], 0x1a4
    { const reg32 a_ = edx, b_ = eax; edx = a_ + b_; fl.add(a_, b_); }  // 48dc6c add edx, eax
    eax = rd32((ebp + 0x5eu));  // 48dc6e mov eax, dword ptr [ebp + 0x5e]
    ebx = rd32((edx + 0x6fbc70u));  // 48dc71 mov ebx, dword ptr [edx + 0x6fbc70]
    carry_ = (eax >> 29) & 1u;  // 48dc77 shl eax, 3
    { const reg32 r_ = eax << 3; eax = r_; fl.logic(r_); }  // 48dc77 shl eax, 3
    fl.sub(ebx, 1u);  // 48dc7a cmp ebx, 1
    if (fl.ne()) goto L_48dce7;  // 48dc7d jne 0x48dce7
    x.push(F(eax + 0x79289cu));  // 48dc7f fld dword ptr [eax + 0x79289c]
    x.st(0) = (x.st(0)) * (F(0x793100u));  // 48dc85 fmul dword ptr [0x793100]
    x.push(F(eax + 0x79289cu));  // 48dc8b fld dword ptr [eax + 0x79289c]
    x.st(0) = (x.st(0)) * (F(0x7930fcu));  // 48dc91 fmul dword ptr [0x7930fc]
    x.push(F(eax + 0x792898u));  // 48dc97 fld dword ptr [eax + 0x792898]
    x.st(0) = (x.st(0)) * (F(0x7930fcu));  // 48dc9d fmul dword ptr [0x7930fc]
    x.push(F(eax + 0x792898u));  // 48dca3 fld dword ptr [eax + 0x792898]
    x.fxch(1);  // 48dca9 fxch st(1)
    x.st(3) = (x.st(0)) - (x.st(3));  // 48dcab fsubrp st(3)
    x.pop();  // 48dcab fsubrp st(3)
    x.st(0) = (x.st(0)) * (F(0x793100u));  // 48dcad fmul dword ptr [0x793100]
    x.fxch(2);  // 48dcb3 fxch st(2)
    SF((ebp + 0x52u), x.st(0));  // 48dcb5 fstp dword ptr [ebp + 0x52]
    x.pop();  // 48dcb5 fstp dword ptr [ebp + 0x52]
    eax = rd32((ebp + 0x52u));  // 48dcb8 mov eax, dword ptr [ebp + 0x52]
    x.st(1) = (x.st(1)) + (x.st(0));  // 48dcbb faddp st(1)
    x.pop();  // 48dcbb faddp st(1)
    { const reg32 r_ = eax | 2147483648u; eax = r_; fl.logic(r_); }  // 48dcbd or eax, 0x80000000
    SF((ebp + 0x62u), x.st(0));  // 48dcc2 fstp dword ptr [ebp + 0x62]
    x.pop();  // 48dcc2 fstp dword ptr [ebp + 0x62]
    fl.sub(eax, 4290772992u);  // 48dcc5 cmp eax, 0xffc00000
    if (fl.ne()) goto L_48dcd1;  // 48dcca jne 0x48dcd1
    esi = 0u;  // 48dccc xor esi, esi
    fl.logic(0u);  // 48dccc xor esi, esi
    wr32((ebp + 0x52u), esi);  // 48dcce mov dword ptr [ebp + 0x52], esi
L_48dcd1:
    eax = rd32((ebp + 0x62u));  // 48dcd1 mov eax, dword ptr [ebp + 0x62]
    { const reg32 r_ = eax | 2147483648u; eax = r_; fl.logic(r_); }  // 48dcd4 or eax, 0x80000000
    fl.sub(eax, 4290772992u);  // 48dcd9 cmp eax, 0xffc00000
    if (fl.ne()) goto L_48dcf9;  // 48dcde jne 0x48dcf9
    edi = 0u;  // 48dce0 xor edi, edi
    fl.logic(0u);  // 48dce0 xor edi, edi
    wr32((ebp + 0x62u), edi);  // 48dce2 mov dword ptr [ebp + 0x62], edi
    goto L_48dcf9;  // 48dce5 jmp 0x48dcf9
L_48dce7:
    edx = rd32((eax + 0x792898u));  // 48dce7 mov edx, dword ptr [eax + 0x792898]
    eax = rd32((eax + 0x79289cu));  // 48dced mov eax, dword ptr [eax + 0x79289c]
    wr32((ebp + 0x52u), edx);  // 48dcf3 mov dword ptr [ebp + 0x52], edx
    wr32((ebp + 0x62u), eax);  // 48dcf6 mov dword ptr [ebp + 0x62], eax
L_48dcf9:
    fl.sub(rd32((0x6fd4c4u)), 0u);  // 48dcf9 cmp dword ptr [0x6fd4c4], 0
    if (fl.e()) goto L_48dd0b;  // 48dd00 je 0x48dd0b
    wr32((ebp + 0xeu), 3212836864u);  // 48dd02 mov dword ptr [ebp + 0xe], 0xbf800000
    goto L_48dd12;  // 48dd09 jmp 0x48dd12
L_48dd0b:
    wr32((ebp + 0xeu), 1065353216u);  // 48dd0b mov dword ptr [ebp + 0xe], 0x3f800000
L_48dd12:
    edx = rd32((ebp + 0x76u));  // 48dd12 mov edx, dword ptr [ebp + 0x76]
    x.push(F(ebp + 0xeu));  // 48dd15 fld dword ptr [ebp + 0xe]
    x.st(0) = (x.st(0)) * (F(edx * 4u + 0x792878u));  // 48dd18 fmul dword ptr [edx*4 + 0x792878]
    x.push(F(edx * 4u + 0x792878u));  // 48dd1f fld dword ptr [edx*4 + 0x792878]
    x.st(0) = (x.st(0)) * (F(ebp + 0x62u));  // 48dd26 fmul dword ptr [ebp + 0x62]
    x.fxch(1);  // 48dd29 fxch st(1)
    x.st(0) = (x.st(0)) * (F(ebp + 0x52u));  // 48dd2b fmul dword ptr [ebp + 0x52]
    eax = rd32((ebp - 0x26u));  // 48dd2e mov eax, dword ptr [ebp - 0x26]
    esp -= 4; wr32(esp, 7942408u);  // 48dd31 push 0x793108
    ecx = 4294967295u;  // 48dd36 mov ecx, 0xffffffff
    wr32((ebp - 0x6au), eax);  // 48dd3b mov dword ptr [ebp - 0x6a], eax
    eax = rd32((ebp - 0x22u));  // 48dd3e mov eax, dword ptr [ebp - 0x22]
    ebx = (ebp - 0x6au);  // 48dd41 lea ebx, [ebp - 0x6a]
    wr32((ebp - 0x66u), eax);  // 48dd44 mov dword ptr [ebp - 0x66], eax
    x.st(0) = (x.st(0)) + (F(edx * 4u + 0x792890u));  // 48dd47 fadd dword ptr [edx*4 + 0x792890]
    x.fxch(1);  // 48dd4e fxch st(1)
    x.st(0) = (F(edx * 4u + 0x792888u)) - (x.st(0));  // 48dd50 fsubr dword ptr [edx*4 + 0x792888]
    x.fxch(1);  // 48dd57 fxch st(1)
    SF((ebp - 0x3eu), x.st(0));  // 48dd59 fstp dword ptr [ebp - 0x3e]
    x.pop();  // 48dd59 fstp dword ptr [ebp - 0x3e]
    eax = rd32((ebp - 0x3eu));  // 48dd5c mov eax, dword ptr [ebp - 0x3e]
    SF((ebp - 0x3au), x.st(0));  // 48dd5f fstp dword ptr [ebp - 0x3a]
    x.pop();  // 48dd5f fstp dword ptr [ebp - 0x3a]
    wr32((ebp - 0x62u), eax);  // 48dd62 mov dword ptr [ebp - 0x62], eax
    eax = rd32((ebp - 0x3au));  // 48dd65 mov eax, dword ptr [ebp - 0x3a]
    edx = rd32((edx * 4u + 0x792880u));  // 48dd68 mov edx, dword ptr [edx*4 + 0x792880]
    wr32((ebp - 0x5eu), eax);  // 48dd6f mov dword ptr [ebp - 0x5e], eax
    eax = rd32((ebp + 0x76u));  // 48dd72 mov eax, dword ptr [ebp + 0x76]
    edi = (ebp - 0x26u);  // 48dd75 lea edi, [ebp - 0x26]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4873e0, esp, 1, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48dd78 call 0x4873e0
    esp += 4u;  // 48dd78 call 0x4873e0
    ebx = rd32((ebp + 0x5eu));  // 48dd7d mov ebx, dword ptr [ebp + 0x5e]
    esi = (ebp - 0x3eu);  // 48dd80 lea esi, [ebp - 0x3e]
    { const reg32 r_ = ebx + 1u; ebx = r_; fl.arith(r_); }  // 48dd83 inc ebx
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 48dd84 movsd dword ptr es:[edi], dword ptr [esi]
    wr32(edi, rd32(esi)); esi += 4; edi += 4;  // 48dd85 movsd dword ptr es:[edi], dword ptr [esi]
    wr32((ebp + 0x5eu), ebx);  // 48dd86 mov dword ptr [ebp + 0x5e], ebx
    goto L_48dc49;  // 48dd89 jmp 0x48dc49
L_48dd8e:
    eax = (ebp - 0x3eu);  // 48dd8e lea eax, [ebp - 0x3e]
    esp -= 4; wr32(esp, eax);  // 48dd91 push eax
    eax = rd32((0x5dd948u));  // 48dd92 mov eax, dword ptr [0x5dd948]
    x.push(D(0x53b9e4u));  // 48dd97 fld qword ptr [0x53b9e4]
    x.push(double(int32_t(rd32((eax + 8u)))));  // 48dd9d fild dword ptr [eax + 8]
    x.st(0) = (x.st(0)) * (x.st(1));  // 48dda0 fmul st(1)
    esp -= 4u;  // 48dda2 sub esp, 4
    SF((esp), x.st(0));  // 48dda5 fstp dword ptr [esp]
    x.pop();  // 48dda5 fstp dword ptr [esp]
    x.push(double(int32_t(rd32((eax)))));  // 48dda8 fild dword ptr [eax]
    x.st(1) = (x.st(1)) * (x.st(0));  // 48ddaa fmulp st(1)
    x.pop();  // 48ddaa fmulp st(1)
    esp -= 4u;  // 48ddac sub esp, 4
    eax = rd32((ebp + 0x76u));  // 48ddaf mov eax, dword ptr [ebp + 0x76]
    SF((esp), x.st(0));  // 48ddb2 fstp dword ptr [esp]
    x.pop();  // 48ddb2 fstp dword ptr [esp]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x48cb50, esp, 3, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48ddb5 call 0x48cb50
    esp += 12u;  // 48ddb5 call 0x48cb50
    edx = reg32(int32_t(rd32((0x79c110u))) * int32_t(840u));  // 48ddba imul edx, dword ptr [0x79c110], 0x348
    eax = reg32(int32_t(rd32((0x79c10cu))) * int32_t(420u));  // 48ddc4 imul eax, dword ptr [0x79c10c], 0x1a4
    fl.sub(rd32((edx + eax + 0x6fbc70u)), 1u);  // 48ddce cmp dword ptr [edx + eax + 0x6fbc70], 1
    if (fl.ne()) goto L_48de27;  // 48ddd6 jne 0x48de27
    eax = rd32((0x5dd948u));  // 48ddd8 mov eax, dword ptr [0x5dd948]
    edx = rd32((eax + 0x15u));  // 48dddd mov edx, dword ptr [eax + 0x15]
    carry_ = (edx >> 23) & 1u;  // 48dde0 sar edx, 0x18
    { const reg32 r_ = reg32(int32_t(edx) >> 24); edx = r_; fl.logic(r_); }  // 48dde0 sar edx, 0x18
    carry_ = (edx >> 23) & 1u;  // 48dde3 shl edx, 9
    { const reg32 r_ = edx << 9; edx = r_; fl.logic(r_); }  // 48dde3 shl edx, 9
    wr32((ebp + 0x7eu), edx);  // 48dde6 mov dword ptr [ebp + 0x7e], edx
    eax = rd32((eax + 0x17u));  // 48dde9 mov eax, dword ptr [eax + 0x17]
    x.push(D(0x53b9e4u));  // 48ddec fld qword ptr [0x53b9e4]
    x.push(double(int32_t(rd32((ebp + 0x7eu)))));  // 48ddf2 fild dword ptr [ebp + 0x7e]
    x.st(0) = (x.st(0)) * (x.st(1));  // 48ddf5 fmul st(1)
    carry_ = (eax >> 23) & 1u;  // 48ddf7 sar eax, 0x18
    { const reg32 r_ = reg32(int32_t(eax) >> 24); eax = r_; fl.logic(r_); }  // 48ddf7 sar eax, 0x18
    x.push(F(0x7930fcu));  // 48ddfa fld dword ptr [0x7930fc]
    carry_ = (eax >> 23) & 1u;  // 48de00 shl eax, 9
    { const reg32 r_ = eax << 9; eax = r_; fl.logic(r_); }  // 48de00 shl eax, 9
    x.push(x.st(1));  // 48de03 fld st(1)
    x.st(0) = (x.st(0)) * (x.st(1));  // 48de05 fmul st(1)
    wr32((ebp + 0x7eu), eax);  // 48de07 mov dword ptr [ebp + 0x7e], eax
    x.push(double(int32_t(rd32((ebp + 0x7eu)))));  // 48de0a fild dword ptr [ebp + 0x7e]
    x.st(4) = (x.st(4)) * (x.st(0));  // 48de0d fmulp st(4)
    x.pop();  // 48de0d fmulp st(4)
    x.push(F(0x793100u));  // 48de0f fld dword ptr [0x793100]
    x.fxch(3);  // 48de15 fxch st(3)
    x.st(0) = (x.st(0)) * (x.st(3));  // 48de17 fmul st(3)
    x.push(x.st(4));  // 48de19 fld st(4)
    x.st(4) = (x.st(4)) * (x.st(0));  // 48de1b fmulp st(4)
    x.pop();  // 48de1b fmulp st(4)
    x.fxch(4);  // 48de1d fxch st(4)
    x.st(2) = (x.st(2)) * (x.st(0));  // 48de1f fmulp st(2)
    x.pop();  // 48de1f fmulp st(2)
    x.st(2) = (x.st(0)) - (x.st(2));  // 48de21 fsubrp st(2)
    x.pop();  // 48de21 fsubrp st(2)
    x.st(2) = (x.st(2)) + (x.st(0));  // 48de23 faddp st(2)
    x.pop();  // 48de23 faddp st(2)
    goto L_48de56;  // 48de25 jmp 0x48de56
L_48de27:
    eax = rd32((0x5dd948u));  // 48de27 mov eax, dword ptr [0x5dd948]
    edx = rd32((eax + 0x15u));  // 48de2c mov edx, dword ptr [eax + 0x15]
    carry_ = (edx >> 23) & 1u;  // 48de2f sar edx, 0x18
    { const reg32 r_ = reg32(int32_t(edx) >> 24); edx = r_; fl.logic(r_); }  // 48de2f sar edx, 0x18
    eax = rd32((eax + 0x17u));  // 48de32 mov eax, dword ptr [eax + 0x17]
    carry_ = (edx >> 23) & 1u;  // 48de35 shl edx, 9
    { const reg32 r_ = edx << 9; edx = r_; fl.logic(r_); }  // 48de35 shl edx, 9
    carry_ = (eax >> 23) & 1u;  // 48de38 sar eax, 0x18
    { const reg32 r_ = reg32(int32_t(eax) >> 24); eax = r_; fl.logic(r_); }  // 48de38 sar eax, 0x18
    wr32((ebp + 0x7eu), edx);  // 48de3b mov dword ptr [ebp + 0x7e], edx
    carry_ = (eax >> 23) & 1u;  // 48de3e shl eax, 9
    { const reg32 r_ = eax << 9; eax = r_; fl.logic(r_); }  // 48de3e shl eax, 9
    x.push(double(int32_t(rd32((ebp + 0x7eu)))));  // 48de41 fild dword ptr [ebp + 0x7e]
    wr32((ebp + 0x7eu), eax);  // 48de44 mov dword ptr [ebp + 0x7e], eax
    x.push(D(0x53b9e4u));  // 48de47 fld qword ptr [0x53b9e4]
    x.fxch(1);  // 48de4d fxch st(1)
    x.st(0) = (x.st(0)) * (x.st(1));  // 48de4f fmul st(1)
    x.push(double(int32_t(rd32((ebp + 0x7eu)))));  // 48de51 fild dword ptr [ebp + 0x7e]
    x.st(2) = (x.st(2)) * (x.st(0));  // 48de54 fmulp st(2)
    x.pop();  // 48de54 fmulp st(2)
L_48de56:
    SF((ebp + 0x3au), x.st(0));  // 48de56 fstp dword ptr [ebp + 0x3a]
    x.pop();  // 48de56 fstp dword ptr [ebp + 0x3a]
    SF((ebp + 0x36u), x.st(0));  // 48de59 fstp dword ptr [ebp + 0x36]
    x.pop();  // 48de59 fstp dword ptr [ebp + 0x36]
    fl.sub(rd32((0x6fd4c4u)), 0u);  // 48de5c cmp dword ptr [0x6fd4c4], 0
    if (fl.e()) goto L_48de6e;  // 48de63 je 0x48de6e
    wr32((ebp - 2u), 3212836864u);  // 48de65 mov dword ptr [ebp - 2], 0xbf800000
    goto L_48de75;  // 48de6c jmp 0x48de75
L_48de6e:
    wr32((ebp - 2u), 1065353216u);  // 48de6e mov dword ptr [ebp - 2], 0x3f800000
L_48de75:
    x.push(F(0x53b9f0u));  // 48de75 fld dword ptr [0x53b9f0]
    x.push(F(ebp - 2u));  // 48de7b fld dword ptr [ebp - 2]
    x.st(0) = (x.st(0)) * (x.st(1));  // 48de7e fmul st(1)
    x.push(F(ebp + 0x36u));  // 48de80 fld dword ptr [ebp + 0x36]
    x.st(2) = (x.st(2)) * (x.st(0));  // 48de83 fmulp st(2)
    x.pop();  // 48de83 fmulp st(2)
    x.st(0) = (x.st(0)) * (F(ebp + 0x3au));  // 48de85 fmul dword ptr [ebp + 0x3a]
    eax = rd32((0x6fd4c4u));  // 48de88 mov eax, dword ptr [0x6fd4c4]
    x.fxch(1);  // 48de8d fxch st(1)
    x.st(0) = (F(ebp - 0x3au)) - (x.st(0));  // 48de8f fsubr dword ptr [ebp - 0x3a]
    x.fxch(1);  // 48de92 fxch st(1)
    x.st(0) = (x.st(0)) + (F(ebp - 0x3eu));  // 48de94 fadd dword ptr [ebp - 0x3e]
    x.fxch(1);  // 48de97 fxch st(1)
    SF((ebp - 0x76u), x.st(0));  // 48de99 fstp dword ptr [ebp - 0x76]
    x.pop();  // 48de99 fstp dword ptr [ebp - 0x76]
    SF((ebp - 0x7au), x.st(0));  // 48de9c fstp dword ptr [ebp - 0x7a]
    x.pop();  // 48de9c fstp dword ptr [ebp - 0x7a]
    fl.logic(eax & eax);  // 48de9f test eax, eax
    if (fl.e()) goto L_48deac;  // 48dea1 je 0x48deac
    wr32((ebp + 0x12u), 3212836864u);  // 48dea3 mov dword ptr [ebp + 0x12], 0xbf800000
    goto L_48deb3;  // 48deaa jmp 0x48deb3
L_48deac:
    wr32((ebp + 0x12u), 1065353216u);  // 48deac mov dword ptr [ebp + 0x12], 0x3f800000
L_48deb3:
    x.push(F(0x53b9f0u));  // 48deb3 fld dword ptr [0x53b9f0]
    x.push(F(ebp + 0x12u));  // 48deb9 fld dword ptr [ebp + 0x12]
    x.st(0) = (x.st(0)) * (x.st(1));  // 48debc fmul st(1)
    x.push(F(ebp + 0x36u));  // 48debe fld dword ptr [ebp + 0x36]
    x.st(2) = (x.st(2)) * (x.st(0));  // 48dec1 fmulp st(2)
    x.pop();  // 48dec1 fmulp st(2)
    x.st(0) = (x.st(0)) * (F(ebp + 0x3au));  // 48dec3 fmul dword ptr [ebp + 0x3a]
    esp -= 4; wr32(esp, 7942408u);  // 48dec6 push 0x793108
    edx = rd32((ebp + 0x76u));  // 48decb mov edx, dword ptr [ebp + 0x76]
    ecx = 4294967295u;  // 48dece mov ecx, 0xffffffff
    ebx = (ebp - 0x7au);  // 48ded3 lea ebx, [ebp - 0x7a]
    eax = rd32((ebp + 0x76u));  // 48ded6 mov eax, dword ptr [ebp + 0x76]
    edx = rd32((edx * 4u + 0x792880u));  // 48ded9 mov edx, dword ptr [edx*4 + 0x792880]
    x.fxch(1);  // 48dee0 fxch st(1)
    x.st(0) = (x.st(0)) + (F(ebp - 0x3au));  // 48dee2 fadd dword ptr [ebp - 0x3a]
    x.fxch(1);  // 48dee5 fxch st(1)
    x.st(0) = (F(ebp - 0x3eu)) - (x.st(0));  // 48dee7 fsubr dword ptr [ebp - 0x3e]
    x.fxch(1);  // 48deea fxch st(1)
    SF((ebp - 0x6eu), x.st(0));  // 48deec fstp dword ptr [ebp - 0x6e]
    x.pop();  // 48deec fstp dword ptr [ebp - 0x6e]
    SF((ebp - 0x72u), x.st(0));  // 48deef fstp dword ptr [ebp - 0x72]
    x.pop();  // 48deef fstp dword ptr [ebp - 0x72]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4873e0, esp, 1, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48def2 call 0x4873e0
    esp += 4u;  // 48def2 call 0x4873e0
    fl.sub(rd32((0x79c10cu)), 0u);  // 48def7 cmp dword ptr [0x79c10c], 0
    if (fl.e()) goto L_48e0ef;  // 48defe je 0x48e0ef
    edx = rd32((ebp + 0x72u));  // 48df04 mov edx, dword ptr [ebp + 0x72]
    edx = rd32((edx + 0x714u));  // 48df07 mov edx, dword ptr [edx + 0x714]
    eax = (edx * 4u);  // 48df0d lea eax, [edx*4]
    { const reg32 a_ = eax, b_ = edx; eax = a_ + b_; fl.add(a_, b_); }  // 48df14 add eax, edx
    carry_ = (eax >> 28) & 1u;  // 48df16 shl eax, 4
    { const reg32 r_ = eax << 4; eax = r_; fl.logic(r_); }  // 48df16 shl eax, 4
    fl.sub(rd32((eax + 0x7dd850u)), 0u);  // 48df19 cmp dword ptr [eax + 0x7dd850], 0
    if (fl.e()) goto L_48e0ef;  // 48df20 je 0x48e0ef
    edx = rd32((ebp + 0x72u));  // 48df26 mov edx, dword ptr [ebp + 0x72]
    eax = (ebp - 0x3eu);  // 48df29 lea eax, [ebp - 0x3e]
    edx = rd32((edx + 0x714u));  // 48df2c mov edx, dword ptr [edx + 0x714]
    esp -= 4; wr32(esp, eax);  // 48df32 push eax
    eax = (edx * 4u);  // 48df33 lea eax, [edx*4]
    { const reg32 a_ = edx, b_ = eax; edx = a_ + b_; fl.add(a_, b_); }  // 48df3a add edx, eax
    carry_ = (edx >> 28) & 1u;  // 48df3c shl edx, 4
    { const reg32 r_ = edx << 4; edx = r_; fl.logic(r_); }  // 48df3c shl edx, 4
    ecx = rd32((edx + 0x7dd884u));  // 48df3f mov ecx, dword ptr [edx + 0x7dd884]
    eax = (ecx * 8u);  // 48df45 lea eax, [ecx*8]
    { const reg32 a_ = eax, b_ = ecx; eax = a_ + b_; fl.add(a_, b_); }  // 48df4c add eax, ecx
    ecx = rd32((0x5dd948u));  // 48df4e mov ecx, dword ptr [0x5dd948]
    x.push(D(0x53b9e4u));  // 48df54 fld qword ptr [0x53b9e4]
    x.push(double(int32_t(rd32((ecx + eax * 4u + 8u)))));  // 48df5a fild dword ptr [ecx + eax*4 + 8]
    x.st(0) = (x.st(0)) * (x.st(1));  // 48df5e fmul st(1)
    esp -= 4u;  // 48df60 sub esp, 4
    SF((esp), x.st(0));  // 48df63 fstp dword ptr [esp]
    x.pop();  // 48df63 fstp dword ptr [esp]
    x.push(double(int32_t(rd32((ecx + eax * 4u)))));  // 48df66 fild dword ptr [ecx + eax*4]
    x.st(1) = (x.st(1)) * (x.st(0));  // 48df69 fmulp st(1)
    x.pop();  // 48df69 fmulp st(1)
    esp -= 4u;  // 48df6b sub esp, 4
    eax = rd32((ebp + 0x76u));  // 48df6e mov eax, dword ptr [ebp + 0x76]
    SF((esp), x.st(0));  // 48df71 fstp dword ptr [esp]
    x.pop();  // 48df71 fstp dword ptr [esp]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x48cb50, esp, 3, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48df74 call 0x48cb50
    esp += 12u;  // 48df74 call 0x48cb50
    ecx = reg32(int32_t(rd32((0x79c110u))) * int32_t(840u));  // 48df79 imul ecx, dword ptr [0x79c110], 0x348
    eax = reg32(int32_t(rd32((0x79c10cu))) * int32_t(420u));  // 48df83 imul eax, dword ptr [0x79c10c], 0x1a4
    esi = rd32((ecx + eax + 0x6fbc70u));  // 48df8d mov esi, dword ptr [ecx + eax + 0x6fbc70]
    fl.sub(esi, 1u);  // 48df94 cmp esi, 1
    if (fl.ne()) goto L_48dffd;  // 48df97 jne 0x48dffd
    edx = rd32((edx + 0x7dd884u));  // 48df99 mov edx, dword ptr [edx + 0x7dd884]
    eax = (edx * 8u);  // 48df9f lea eax, [edx*8]
    { const reg32 a_ = eax, b_ = edx; eax = a_ + b_; fl.add(a_, b_); }  // 48dfa6 add eax, edx
    edx = rd32((0x5dd948u));  // 48dfa8 mov edx, dword ptr [0x5dd948]
    carry_ = (eax >> 30) & 1u;  // 48dfae shl eax, 2
    { const reg32 r_ = eax << 2; eax = r_; fl.logic(r_); }  // 48dfae shl eax, 2
    { const reg32 a_ = eax, b_ = edx; eax = a_ + b_; fl.add(a_, b_); }  // 48dfb1 add eax, edx
    edx = rd32((eax + 0x15u));  // 48dfb3 mov edx, dword ptr [eax + 0x15]
    carry_ = (edx >> 23) & 1u;  // 48dfb6 sar edx, 0x18
    { const reg32 r_ = reg32(int32_t(edx) >> 24); edx = r_; fl.logic(r_); }  // 48dfb6 sar edx, 0x18
    carry_ = (edx >> 23) & 1u;  // 48dfb9 shl edx, 9
    { const reg32 r_ = edx << 9; edx = r_; fl.logic(r_); }  // 48dfb9 shl edx, 9
    wr32((ebp + 0x7eu), edx);  // 48dfbc mov dword ptr [ebp + 0x7e], edx
    eax = rd32((eax + 0x17u));  // 48dfbf mov eax, dword ptr [eax + 0x17]
    x.push(D(0x53b9e4u));  // 48dfc2 fld qword ptr [0x53b9e4]
    x.push(double(int32_t(rd32((ebp + 0x7eu)))));  // 48dfc8 fild dword ptr [ebp + 0x7e]
    x.st(0) = (x.st(0)) * (x.st(1));  // 48dfcb fmul st(1)
    carry_ = (eax >> 23) & 1u;  // 48dfcd sar eax, 0x18
    { const reg32 r_ = reg32(int32_t(eax) >> 24); eax = r_; fl.logic(r_); }  // 48dfcd sar eax, 0x18
    x.push(F(0x7930fcu));  // 48dfd0 fld dword ptr [0x7930fc]
    carry_ = (eax >> 23) & 1u;  // 48dfd6 shl eax, 9
    { const reg32 r_ = eax << 9; eax = r_; fl.logic(r_); }  // 48dfd6 shl eax, 9
    x.push(x.st(1));  // 48dfd9 fld st(1)
    x.st(0) = (x.st(0)) * (x.st(1));  // 48dfdb fmul st(1)
    wr32((ebp + 0x7eu), eax);  // 48dfdd mov dword ptr [ebp + 0x7e], eax
    x.push(double(int32_t(rd32((ebp + 0x7eu)))));  // 48dfe0 fild dword ptr [ebp + 0x7e]
    x.st(4) = (x.st(4)) * (x.st(0));  // 48dfe3 fmulp st(4)
    x.pop();  // 48dfe3 fmulp st(4)
    x.push(F(0x793100u));  // 48dfe5 fld dword ptr [0x793100]
    x.fxch(3);  // 48dfeb fxch st(3)
    x.st(0) = (x.st(0)) * (x.st(3));  // 48dfed fmul st(3)
    x.push(x.st(4));  // 48dfef fld st(4)
    x.st(4) = (x.st(4)) * (x.st(0));  // 48dff1 fmulp st(4)
    x.pop();  // 48dff1 fmulp st(4)
    x.fxch(4);  // 48dff3 fxch st(4)
    x.st(2) = (x.st(2)) * (x.st(0));  // 48dff5 fmulp st(2)
    x.pop();  // 48dff5 fmulp st(2)
    x.st(2) = (x.st(0)) - (x.st(2));  // 48dff7 fsubrp st(2)
    x.pop();  // 48dff7 fsubrp st(2)
    x.st(2) = (x.st(2)) + (x.st(0));  // 48dff9 faddp st(2)
    x.pop();  // 48dff9 faddp st(2)
    goto L_48e041;  // 48dffb jmp 0x48e041
L_48dffd:
    edx = rd32((edx + 0x7dd884u));  // 48dffd mov edx, dword ptr [edx + 0x7dd884]
    eax = (edx * 8u);  // 48e003 lea eax, [edx*8]
    { const reg32 a_ = eax, b_ = edx; eax = a_ + b_; fl.add(a_, b_); }  // 48e00a add eax, edx
    edx = rd32((0x5dd948u));  // 48e00c mov edx, dword ptr [0x5dd948]
    carry_ = (eax >> 30) & 1u;  // 48e012 shl eax, 2
    { const reg32 r_ = eax << 2; eax = r_; fl.logic(r_); }  // 48e012 shl eax, 2
    { const reg32 a_ = eax, b_ = edx; eax = a_ + b_; fl.add(a_, b_); }  // 48e015 add eax, edx
    edx = rd32((eax + 0x15u));  // 48e017 mov edx, dword ptr [eax + 0x15]
    carry_ = (edx >> 23) & 1u;  // 48e01a sar edx, 0x18
    { const reg32 r_ = reg32(int32_t(edx) >> 24); edx = r_; fl.logic(r_); }  // 48e01a sar edx, 0x18
    eax = rd32((eax + 0x17u));  // 48e01d mov eax, dword ptr [eax + 0x17]
    carry_ = (edx >> 23) & 1u;  // 48e020 shl edx, 9
    { const reg32 r_ = edx << 9; edx = r_; fl.logic(r_); }  // 48e020 shl edx, 9
    carry_ = (eax >> 23) & 1u;  // 48e023 sar eax, 0x18
    { const reg32 r_ = reg32(int32_t(eax) >> 24); eax = r_; fl.logic(r_); }  // 48e023 sar eax, 0x18
    wr32((ebp + 0x7eu), edx);  // 48e026 mov dword ptr [ebp + 0x7e], edx
    carry_ = (eax >> 23) & 1u;  // 48e029 shl eax, 9
    { const reg32 r_ = eax << 9; eax = r_; fl.logic(r_); }  // 48e029 shl eax, 9
    x.push(double(int32_t(rd32((ebp + 0x7eu)))));  // 48e02c fild dword ptr [ebp + 0x7e]
    wr32((ebp + 0x7eu), eax);  // 48e02f mov dword ptr [ebp + 0x7e], eax
    x.push(D(0x53b9e4u));  // 48e032 fld qword ptr [0x53b9e4]
    x.fxch(1);  // 48e038 fxch st(1)
    x.st(0) = (x.st(0)) * (x.st(1));  // 48e03a fmul st(1)
    x.push(double(int32_t(rd32((ebp + 0x7eu)))));  // 48e03c fild dword ptr [ebp + 0x7e]
    x.st(2) = (x.st(2)) * (x.st(0));  // 48e03f fmulp st(2)
    x.pop();  // 48e03f fmulp st(2)
L_48e041:
    SF((ebp + 0x2eu), x.st(0));  // 48e041 fstp dword ptr [ebp + 0x2e]
    x.pop();  // 48e041 fstp dword ptr [ebp + 0x2e]
    SF((ebp + 0x3eu), x.st(0));  // 48e044 fstp dword ptr [ebp + 0x3e]
    x.pop();  // 48e044 fstp dword ptr [ebp + 0x3e]
    fl.sub(rd32((0x6fd4c4u)), 0u);  // 48e047 cmp dword ptr [0x6fd4c4], 0
    if (fl.e()) goto L_48e059;  // 48e04e je 0x48e059
    wr32((ebp + 0x2au), 3212836864u);  // 48e050 mov dword ptr [ebp + 0x2a], 0xbf800000
    goto L_48e060;  // 48e057 jmp 0x48e060
L_48e059:
    wr32((ebp + 0x2au), 1065353216u);  // 48e059 mov dword ptr [ebp + 0x2a], 0x3f800000
L_48e060:
    x.push(F(0x53b9f0u));  // 48e060 fld dword ptr [0x53b9f0]
    x.push(F(ebp + 0x2au));  // 48e066 fld dword ptr [ebp + 0x2a]
    x.st(0) = (x.st(0)) * (x.st(1));  // 48e069 fmul st(1)
    x.push(F(ebp + 0x3eu));  // 48e06b fld dword ptr [ebp + 0x3e]
    x.st(2) = (x.st(2)) * (x.st(0));  // 48e06e fmulp st(2)
    x.pop();  // 48e06e fmulp st(2)
    x.st(0) = (x.st(0)) * (F(ebp + 0x2eu));  // 48e070 fmul dword ptr [ebp + 0x2e]
    ecx = rd32((0x6fd4c4u));  // 48e073 mov ecx, dword ptr [0x6fd4c4]
    x.fxch(1);  // 48e079 fxch st(1)
    x.st(0) = (F(ebp - 0x3au)) - (x.st(0));  // 48e07b fsubr dword ptr [ebp - 0x3a]
    x.fxch(1);  // 48e07e fxch st(1)
    x.st(0) = (x.st(0)) + (F(ebp - 0x3eu));  // 48e080 fadd dword ptr [ebp - 0x3e]
    x.fxch(1);  // 48e083 fxch st(1)
    SF((ebp - 0x86u), x.st(0));  // 48e085 fstp dword ptr [ebp - 0x86]
    x.pop();  // 48e085 fstp dword ptr [ebp - 0x86]
    SF((ebp - 0x8au), x.st(0));  // 48e08b fstp dword ptr [ebp - 0x8a]
    x.pop();  // 48e08b fstp dword ptr [ebp - 0x8a]
    fl.logic(ecx & ecx);  // 48e091 test ecx, ecx
    if (fl.e()) goto L_48e09e;  // 48e093 je 0x48e09e
    wr32((ebp + 0x16u), 3212836864u);  // 48e095 mov dword ptr [ebp + 0x16], 0xbf800000
    goto L_48e0a5;  // 48e09c jmp 0x48e0a5
L_48e09e:
    wr32((ebp + 0x16u), 1065353216u);  // 48e09e mov dword ptr [ebp + 0x16], 0x3f800000
L_48e0a5:
    x.push(F(0x53b9f0u));  // 48e0a5 fld dword ptr [0x53b9f0]
    x.push(F(ebp + 0x16u));  // 48e0ab fld dword ptr [ebp + 0x16]
    x.st(0) = (x.st(0)) * (x.st(1));  // 48e0ae fmul st(1)
    x.push(F(ebp + 0x3eu));  // 48e0b0 fld dword ptr [ebp + 0x3e]
    x.st(2) = (x.st(2)) * (x.st(0));  // 48e0b3 fmulp st(2)
    x.pop();  // 48e0b3 fmulp st(2)
    x.st(0) = (x.st(0)) * (F(ebp + 0x2eu));  // 48e0b5 fmul dword ptr [ebp + 0x2e]
    esp -= 4; wr32(esp, 7942408u);  // 48e0b8 push 0x793108
    edx = rd32((ebp + 0x76u));  // 48e0bd mov edx, dword ptr [ebp + 0x76]
    ecx = 4281571669u;  // 48e0c0 mov ecx, 0xff339955
    ebx = (ebp - 0x8au);  // 48e0c5 lea ebx, [ebp - 0x8a]
    eax = rd32((ebp + 0x76u));  // 48e0cb mov eax, dword ptr [ebp + 0x76]
    edx = rd32((edx * 4u + 0x792880u));  // 48e0ce mov edx, dword ptr [edx*4 + 0x792880]
    x.fxch(1);  // 48e0d5 fxch st(1)
    x.st(0) = (x.st(0)) + (F(ebp - 0x3au));  // 48e0d7 fadd dword ptr [ebp - 0x3a]
    x.fxch(1);  // 48e0da fxch st(1)
    x.st(0) = (F(ebp - 0x3eu)) - (x.st(0));  // 48e0dc fsubr dword ptr [ebp - 0x3e]
    x.fxch(1);  // 48e0df fxch st(1)
    SF((ebp - 0x7eu), x.st(0));  // 48e0e1 fstp dword ptr [ebp - 0x7e]
    x.pop();  // 48e0e1 fstp dword ptr [ebp - 0x7e]
    SF((ebp - 0x82u), x.st(0));  // 48e0e4 fstp dword ptr [ebp - 0x82]
    x.pop();  // 48e0e4 fstp dword ptr [ebp - 0x82]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4873e0, esp, 1, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48e0ea call 0x4873e0
    esp += 4u;  // 48e0ea call 0x4873e0
L_48e0ef:
    ecx = 0u;  // 48e0ef xor ecx, ecx
    fl.logic(0u);  // 48e0ef xor ecx, ecx
L_48e0f1:
    fl.sub(ecx, rd32((0x5efd88u)));  // 48e0f1 cmp ecx, dword ptr [0x5efd88]
    if (fl.ge()) goto L_48e193;  // 48e0f7 jge 0x48e193
    eax = (ecx * 4u);  // 48e0fd lea eax, [ecx*4]
    edx = rd32((eax + 0x5efa88u));  // 48e104 mov edx, dword ptr [eax + 0x5efa88]
    fl.logic(edx & edx);  // 48e10a test edx, edx
    if (fl.e()) goto L_48e18d;  // 48e10c je 0x48e18d
    esi = rd32((edx + 0x624u));  // 48e112 mov esi, dword ptr [edx + 0x624]
    ebx = edx;  // 48e118 mov ebx, edx
    fl.logic(esi & esi);  // 48e11a test esi, esi
    if (fl.e()) goto L_48e18d;  // 48e11c je 0x48e18d
    eax = rd32((0x7d3684u));  // 48e11e mov eax, dword ptr [0x7d3684]
    edx = eax;  // 48e123 mov edx, eax
    carry_ = (edx >> 30) & 1u;  // 48e125 sar edx, 0x1f
    { const reg32 r_ = reg32(int32_t(edx) >> 31); edx = r_; fl.logic(r_); }  // 48e125 sar edx, 0x1f
    carry_ = (edx >> 27) & 1u;  // 48e128 shl edx, 5
    { const reg32 r_ = edx << 5; edx = r_; fl.logic(r_); }  // 48e128 shl edx, 5
    { const reg32 r_ = eax - edx - carry_; eax = r_; fl.logic(r_); }  // 48e12b sbb eax, edx
    carry_ = (eax >> 4) & 1u;  // 48e12d sar eax, 5
    { const reg32 r_ = reg32(int32_t(eax) >> 5); eax = r_; fl.logic(r_); }  // 48e12d sar eax, 5
    edx = rd32((ebx + 0x1f0u));  // 48e130 mov edx, dword ptr [ebx + 0x1f0]
    { const reg32 a_ = edx, b_ = eax; edx = a_ + b_; fl.add(a_, b_); }  // 48e136 add edx, eax
    ebx = 2u;  // 48e138 mov ebx, 2
    eax = edx;  // 48e13d mov eax, edx
    carry_ = (edx >> 30) & 1u;  // 48e13f sar edx, 0x1f
    { const reg32 r_ = reg32(int32_t(edx) >> 31); edx = r_; fl.logic(r_); }  // 48e13f sar edx, 0x1f
    { const int32_t n_ = int32_t(eax), d_ = int32_t(ebx); eax = reg32(n_ / d_); edx = reg32(n_ % d_); }  // 48e142 idiv ebx
    fl.logic(edx & edx);  // 48e144 test edx, edx
    if (fl.ne()) goto L_48e14f;  // 48e146 jne 0x48e14f
    edx = 2146959360u;  // 48e148 mov edx, 0x7ff80000
    goto L_48e154;  // 48e14d jmp 0x48e154
L_48e14f:
    edx = 2130706680u;  // 48e14f mov edx, 0x7f0000f8
L_48e154:
    eax = (ebp - 0x46u);  // 48e154 lea eax, [ebp - 0x46]
    esp -= 4; wr32(esp, eax);  // 48e157 push eax
    eax = rd32((ecx * 4u + 0x5efa88u));  // 48e158 mov eax, dword ptr [ecx*4 + 0x5efa88]
    esp -= 4; wr32(esp, rd32((eax + 0xa0u)));  // 48e15f push dword ptr [eax + 0xa0]
    esp -= 4; wr32(esp, rd32((eax + 0x98u)));  // 48e165 push dword ptr [eax + 0x98]
    eax = rd32((ebp + 0x76u));  // 48e16b mov eax, dword ptr [ebp + 0x76]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x48cb50, esp, 3, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48e16e call 0x48cb50
    esp += 12u;  // 48e16e call 0x48cb50
    esp -= 4; wr32(esp, rd32((0x7930f0u)));  // 48e173 push dword ptr [0x7930f0]
    esp -= 4; wr32(esp, rd32((0x7930f8u)));  // 48e179 push dword ptr [0x7930f8]
    esp -= 4; wr32(esp, rd32((ebp - 0x42u)));  // 48e17f push dword ptr [ebp - 0x42]
    eax = rd32((ebp + 0x76u));  // 48e182 mov eax, dword ptr [ebp + 0x76]
    esp -= 4; wr32(esp, rd32((ebp - 0x46u)));  // 48e185 push dword ptr [ebp - 0x46]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x48cc60, esp, 4, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48e188 call 0x48cc60
    esp += 16u;  // 48e188 call 0x48cc60
L_48e18d:
    { const reg32 r_ = ecx + 1u; ecx = r_; fl.arith(r_); }  // 48e18d inc ecx
    goto L_48e0f1;  // 48e18e jmp 0x48e0f1
L_48e193:
    ecx = 0u;  // 48e193 xor ecx, ecx
    fl.logic(0u);  // 48e193 xor ecx, ecx
L_48e195:
    fl.sub(ecx, rd32((0x5efda0u)));  // 48e195 cmp ecx, dword ptr [0x5efda0]
    if (fl.ge()) goto L_48e31b;  // 48e19b jge 0x48e31b
    eax = (ecx * 4u);  // 48e1a1 lea eax, [ecx*4]
    ebx = rd32((ebp + 0x72u));  // 48e1a8 mov ebx, dword ptr [ebp + 0x72]
    edx = rd32((eax + 0x5efc48u));  // 48e1ab mov edx, dword ptr [eax + 0x5efc48]
    fl.sub(edx, ebx);  // 48e1b1 cmp edx, ebx
    if (fl.e()) goto L_48e315;  // 48e1b3 je 0x48e315
    eax = rd32((edx + 0x220u));  // 48e1b9 mov eax, dword ptr [edx + 0x220]
    esi = rd32((0x6fdc30u));  // 48e1bf mov esi, dword ptr [0x6fdc30]
    fl.sub(esi, rd32((eax + 0x14u)));  // 48e1c5 cmp esi, dword ptr [eax + 0x14]
    if (fl.ne()) goto L_48e1e3;  // 48e1c8 jne 0x48e1e3
    eax = rd32((ebp + 0x32u));  // 48e1ca mov eax, dword ptr [ebp + 0x32]
    eax = rd32((eax * 4u + 0x5efac8u));  // 48e1cd mov eax, dword ptr [eax*4 + 0x5efac8]
    eax = rd32((eax + 0x220u));  // 48e1d4 mov eax, dword ptr [eax + 0x220]
    fl.sub(esi, rd32((eax + 0x14u)));  // 48e1da cmp esi, dword ptr [eax + 0x14]
    if (fl.ne()) goto L_48e315;  // 48e1dd jne 0x48e315
L_48e1e3:
    eax = rd32((ecx * 4u + 0x5efc48u));  // 48e1e3 mov eax, dword ptr [ecx*4 + 0x5efc48]
    fl.logic(rd8((eax + 0x200u)) & 1u);  // 48e1ea test byte ptr [eax + 0x200], 1
    if (fl.e()) goto L_48e221;  // 48e1f1 je 0x48e221
    edx = rd32((eax + 0x2c4u));  // 48e1f3 mov edx, dword ptr [eax + 0x2c4]
    fl.sub(edx, 4u);  // 48e1f9 cmp edx, 4
    if (fl.ne()) goto L_48e221;  // 48e1fc jne 0x48e221
    eax = rd32((ebp + 0x32u));  // 48e1fe mov eax, dword ptr [ebp + 0x32]
    eax = rd32((eax * 4u + 0x5efac8u));  // 48e201 mov eax, dword ptr [eax*4 + 0x5efac8]
    fl.logic(rd8((eax + 0x200u)) & 1u);  // 48e208 test byte ptr [eax + 0x200], 1
    if (fl.e()) goto L_48e315;  // 48e20f je 0x48e315
    fl.sub(edx, rd32((eax + 0x2c4u)));  // 48e215 cmp edx, dword ptr [eax + 0x2c4]
    if (fl.ne()) goto L_48e315;  // 48e21b jne 0x48e315
L_48e221:
    eax = (ebp - 0x46u);  // 48e221 lea eax, [ebp - 0x46]
    ebx = (ecx * 4u);  // 48e224 lea ebx, [ecx*4]
    esp -= 4; wr32(esp, eax);  // 48e22b push eax
    eax = rd32((ebx + 0x5efc48u));  // 48e22c mov eax, dword ptr [ebx + 0x5efc48]
    esp -= 4; wr32(esp, rd32((eax + 0xa0u)));  // 48e232 push dword ptr [eax + 0xa0]
    esp -= 4; wr32(esp, rd32((eax + 0x98u)));  // 48e238 push dword ptr [eax + 0x98]
    eax = rd32((ebp + 0x76u));  // 48e23e mov eax, dword ptr [ebp + 0x76]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x48cb50, esp, 3, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48e241 call 0x48cb50
    esp += 12u;  // 48e241 call 0x48cb50
    esp -= 4; wr32(esp, rd32((0x7930e8u)));  // 48e246 push dword ptr [0x7930e8]
    eax = rd32((ebx + 0x5efc48u));  // 48e24c mov eax, dword ptr [ebx + 0x5efc48]
    esp -= 4; wr32(esp, rd32((0x7930ecu)));  // 48e252 push dword ptr [0x7930ec]
    edx = rd32((eax + 0x1f0u));  // 48e258 mov edx, dword ptr [eax + 0x1f0]
    esp -= 4; wr32(esp, rd32((ebp - 0x42u)));  // 48e25e push dword ptr [ebp - 0x42]
    eax = rd32((ebp + 0x76u));  // 48e261 mov eax, dword ptr [ebp + 0x76]
    edx = rd32((edx * 4u + 0x793098u));  // 48e264 mov edx, dword ptr [edx*4 + 0x793098]
    esp -= 4; wr32(esp, rd32((ebp - 0x46u)));  // 48e26b push dword ptr [ebp - 0x46]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x48cc60, esp, 4, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48e26e call 0x48cc60
    esp += 16u;  // 48e26e call 0x48cc60
    fl.sub(rd32((0x55fe3cu)), 0u);  // 48e273 cmp dword ptr [0x55fe3c], 0
    if (fl.e()) goto L_48e315;  // 48e27a je 0x48e315
    fl.logic(eax & eax);  // 48e280 test eax, eax
    if (fl.ne()) goto L_48e315;  // 48e282 jne 0x48e315
    x.push(F(0x53b9e0u));  // 48e288 fld dword ptr [0x53b9e0]
    x.push(F(0x7930ecu));  // 48e28e fld dword ptr [0x7930ec]
    x.st(0) = (x.st(0)) * (x.st(1));  // 48e294 fmul st(1)
    x.push(F(0x7930e8u));  // 48e296 fld dword ptr [0x7930e8]
    x.st(2) = (x.st(2)) * (x.st(0));  // 48e29c fmulp st(2)
    x.pop();  // 48e29c fmulp st(2)
    x.push(F(ebp - 0x46u));  // 48e29e fld dword ptr [ebp - 0x46]
    x.push(F(ebp - 0x42u));  // 48e2a1 fld dword ptr [ebp - 0x42]
    x.push(F(ebp - 0x46u));  // 48e2a4 fld dword ptr [ebp - 0x46]
    edx = rd32((ebp + 0x76u));  // 48e2a7 mov edx, dword ptr [ebp + 0x76]
    x.push(F(ebp - 0x42u));  // 48e2aa fld dword ptr [ebp - 0x42]
    ebx = rd32((ebx + 0x5efc48u));  // 48e2ad mov ebx, dword ptr [ebx + 0x5efc48]
    esi = rd32((edx * 4u + 0x55e2b4u));  // 48e2b3 mov esi, dword ptr [edx*4 + 0x55e2b4]
    eax = (edx * 4u);  // 48e2ba lea eax, [edx*4]
    edi = (esi * 4u);  // 48e2c1 lea edi, [esi*4]
    { const reg32 a_ = eax, b_ = edx; eax = a_ + b_; fl.add(a_, b_); }  // 48e2c8 add eax, edx
    x.fxch(3);  // 48e2ca fxch st(3)
    x.st(0) = (x.st(0)) - (x.st(4));  // 48e2cc fsub st(4)
    carry_ = (eax >> 26) & 1u;  // 48e2ce shl eax, 6
    { const reg32 r_ = eax << 6; eax = r_; fl.logic(r_); }  // 48e2ce shl eax, 6
    x.fxch(2);  // 48e2d1 fxch st(2)
    x.st(0) = (x.st(0)) - (x.st(5));  // 48e2d3 fsub st(5)
    { const reg32 a_ = edi, b_ = esi; edi = a_ + b_; fl.add(a_, b_); }  // 48e2d5 add edi, esi
    x.fxch(1);  // 48e2d7 fxch st(1)
    x.st(4) = (x.st(4)) + (x.st(0));  // 48e2d9 faddp st(4)
    x.pop();  // 48e2d9 faddp st(4)
    carry_ = (edi >> 30) & 1u;  // 48e2db shl edi, 2
    { const reg32 r_ = edi << 2; edi = r_; fl.logic(r_); }  // 48e2db shl edi, 2
    x.fxch(2);  // 48e2de fxch st(2)
    x.st(4) = (x.st(4)) + (x.st(0));  // 48e2e0 faddp st(4)
    x.pop();  // 48e2e0 faddp st(4)
    SF((edi + eax + 0x7925f8u), x.st(0));  // 48e2e2 fstp dword ptr [edi + eax + 0x7925f8]
    x.pop();  // 48e2e2 fstp dword ptr [edi + eax + 0x7925f8]
    SF((edi + eax + 0x7925fcu), x.st(0));  // 48e2e9 fstp dword ptr [edi + eax + 0x7925fc]
    x.pop();  // 48e2e9 fstp dword ptr [edi + eax + 0x7925fc]
    SF((edi + eax + 0x792600u), x.st(0));  // 48e2f0 fstp dword ptr [edi + eax + 0x792600]
    x.pop();  // 48e2f0 fstp dword ptr [edi + eax + 0x792600]
    SF((edi + eax + 0x792604u), x.st(0));  // 48e2f7 fstp dword ptr [edi + eax + 0x792604]
    x.pop();  // 48e2f7 fstp dword ptr [edi + eax + 0x792604]
    ebx = rd32((ebx + 0x1f0u));  // 48e2fe mov ebx, dword ptr [ebx + 0x1f0]
    wr32((edi + eax + 0x792608u), ebx);  // 48e304 mov dword ptr [edi + eax + 0x792608], ebx
    edi = (esi + 1u);  // 48e30b lea edi, [esi + 1]
    wr32((edx * 4u + 0x55e2b4u), edi);  // 48e30e mov dword ptr [edx*4 + 0x55e2b4], edi
L_48e315:
    { const reg32 r_ = ecx + 1u; ecx = r_; fl.arith(r_); }  // 48e315 inc ecx
    goto L_48e195;  // 48e316 jmp 0x48e195
L_48e31b:
    fl.sub(rd32((0x55d000u)), 1u);  // 48e31b cmp dword ptr [0x55d000], 1
    if (fl.ne()) goto L_48e394;  // 48e322 jne 0x48e394
    edx = (ebp - 0xaeu);  // 48e324 lea edx, [ebp - 0xae]
    eax = (ebp - 0x5au);  // 48e32a lea eax, [ebp - 0x5a]
    ecx = rd32((0x793098u));  // 48e32d mov ecx, dword ptr [0x793098]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x477d90, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48e333 call 0x477d90
    esp += 0u;  // 48e333 call 0x477d90
    eax = (ebp - 0x46u);  // 48e338 lea eax, [ebp - 0x46]
    esp -= 4; wr32(esp, eax);  // 48e33b push eax
    eax = rd32((ebp - 0x52u));  // 48e33c mov eax, dword ptr [ebp - 0x52]
    wr32((ebp + 0x7eu), eax);  // 48e33f mov dword ptr [ebp + 0x7e], eax
    x.push(D(0x53b9e4u));  // 48e342 fld qword ptr [0x53b9e4]
    x.push(double(int32_t(rd32((ebp + 0x7eu)))));  // 48e348 fild dword ptr [ebp + 0x7e]
    x.st(0) = (x.st(0)) * (x.st(1));  // 48e34b fmul st(1)
    esp -= 4u;  // 48e34d sub esp, 4
    SF((esp), x.st(0));  // 48e350 fstp dword ptr [esp]
    x.pop();  // 48e350 fstp dword ptr [esp]
    eax = rd32((ebp - 0x5au));  // 48e353 mov eax, dword ptr [ebp - 0x5a]
    wr32((ebp + 0x7eu), eax);  // 48e356 mov dword ptr [ebp + 0x7e], eax
    x.push(double(int32_t(rd32((ebp + 0x7eu)))));  // 48e359 fild dword ptr [ebp + 0x7e]
    x.st(1) = (x.st(1)) * (x.st(0));  // 48e35c fmulp st(1)
    x.pop();  // 48e35c fmulp st(1)
    esp -= 4u;  // 48e35e sub esp, 4
    eax = rd32((ebp + 0x76u));  // 48e361 mov eax, dword ptr [ebp + 0x76]
    SF((esp), x.st(0));  // 48e364 fstp dword ptr [esp]
    x.pop();  // 48e364 fstp dword ptr [esp]
    { const reg32 r_ = ecx & 16777215u; ecx = r_; fl.logic(r_); }  // 48e367 and ecx, 0xffffff
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x48cb50, esp, 3, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48e36d call 0x48cb50
    esp += 12u;  // 48e36d call 0x48cb50
    esp -= 4; wr32(esp, rd32((0x7930d8u)));  // 48e372 push dword ptr [0x7930d8]
    { const reg32 r_ = ecx | 2130706432u; ecx = r_; fl.logic(r_); }  // 48e378 or ecx, 0x7f000000
    esp -= 4; wr32(esp, rd32((0x7930e0u)));  // 48e37e push dword ptr [0x7930e0]
    edx = ecx;  // 48e384 mov edx, ecx
    esp -= 4; wr32(esp, rd32((ebp - 0x42u)));  // 48e386 push dword ptr [ebp - 0x42]
    eax = rd32((ebp + 0x76u));  // 48e389 mov eax, dword ptr [ebp + 0x76]
    esp -= 4; wr32(esp, rd32((ebp - 0x46u)));  // 48e38c push dword ptr [ebp - 0x46]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x48cc60, esp, 4, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48e38f call 0x48cc60
    esp += 16u;  // 48e38f call 0x48cc60
L_48e394:
    esp -= 4; wr32(esp, rd32((0x7930f4u)));  // 48e394 push dword ptr [0x7930f4]
    edx = rd32((ebp + 0x72u));  // 48e39a mov edx, dword ptr [ebp + 0x72]
    eax = rd32((ebp + 0x76u));  // 48e39d mov eax, dword ptr [ebp + 0x76]
    esp -= 4; wr32(esp, rd32((0x793104u)));  // 48e3a0 push dword ptr [0x793104]
    ecx = rd32((ebp + 0x32u));  // 48e3a6 mov ecx, dword ptr [ebp + 0x32]
    esp -= 4; wr32(esp, rd32((ebp - 0x4au)));  // 48e3a9 push dword ptr [ebp - 0x4a]
    edx = rd32((edx + 0x1f0u));  // 48e3ac mov edx, dword ptr [edx + 0x1f0]
    esp -= 4; wr32(esp, rd32((ebp - 0x4eu)));  // 48e3b2 push dword ptr [ebp - 0x4e]
    edx = rd32((edx * 4u + 0x793098u));  // 48e3b5 mov edx, dword ptr [edx*4 + 0x793098]
    carry_ = (ecx >> 30) & 1u;  // 48e3bc shl ecx, 2
    { const reg32 r_ = ecx << 2; ecx = r_; fl.logic(r_); }  // 48e3bc shl ecx, 2
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x48cc60, esp, 4, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48e3bf call 0x48cc60
    esp += 16u;  // 48e3bf call 0x48cc60
    eax = rd32((ecx + 0x5efac8u));  // 48e3c4 mov eax, dword ptr [ecx + 0x5efac8]
    fl.logic(rd8((eax + 0x200u)) & 32u);  // 48e3ca test byte ptr [eax + 0x200], 0x20
    if (fl.e()) goto L_48e4ce;  // 48e3d1 je 0x48e4ce
    edx = rd32((ebp + 0x72u));  // 48e3d7 mov edx, dword ptr [ebp + 0x72]
    fl.logic(rd8((edx + 0x200u)) & 32u);  // 48e3da test byte ptr [edx + 0x200], 0x20
    if (fl.ne()) goto L_48e4ce;  // 48e3e1 jne 0x48e4ce
    edx = (ebp - 0x46u);  // 48e3e7 lea edx, [ebp - 0x46]
    esp -= 4; wr32(esp, edx);  // 48e3ea push edx
    esp -= 4; wr32(esp, rd32((eax + 0xa0u)));  // 48e3eb push dword ptr [eax + 0xa0]
    esp -= 4; wr32(esp, rd32((eax + 0x98u)));  // 48e3f1 push dword ptr [eax + 0x98]
    eax = rd32((ebp + 0x76u));  // 48e3f7 mov eax, dword ptr [ebp + 0x76]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x48cb50, esp, 3, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48e3fa call 0x48cb50
    esp += 12u;  // 48e3fa call 0x48cb50
    esp -= 4; wr32(esp, rd32((0x7930e8u)));  // 48e3ff push dword ptr [0x7930e8]
    eax = rd32((ecx + 0x5efac8u));  // 48e405 mov eax, dword ptr [ecx + 0x5efac8]
    esp -= 4; wr32(esp, rd32((0x7930ecu)));  // 48e40b push dword ptr [0x7930ec]
    edx = rd32((eax + 0x1f0u));  // 48e411 mov edx, dword ptr [eax + 0x1f0]
    esp -= 4; wr32(esp, rd32((ebp - 0x42u)));  // 48e417 push dword ptr [ebp - 0x42]
    eax = rd32((ebp + 0x76u));  // 48e41a mov eax, dword ptr [ebp + 0x76]
    edx = rd32((edx * 4u + 0x793098u));  // 48e41d mov edx, dword ptr [edx*4 + 0x793098]
    esp -= 4; wr32(esp, rd32((ebp - 0x46u)));  // 48e424 push dword ptr [ebp - 0x46]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x48cc60, esp, 4, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48e427 call 0x48cc60
    esp += 16u;  // 48e427 call 0x48cc60
    fl.sub(rd32((0x55fe3cu)), 0u);  // 48e42c cmp dword ptr [0x55fe3c], 0
    if (fl.e()) goto L_48e4ce;  // 48e433 je 0x48e4ce
    fl.logic(eax & eax);  // 48e439 test eax, eax
    if (fl.ne()) goto L_48e4ce;  // 48e43b jne 0x48e4ce
    x.push(F(0x53b9e0u));  // 48e441 fld dword ptr [0x53b9e0]
    x.push(F(0x7930ecu));  // 48e447 fld dword ptr [0x7930ec]
    x.st(0) = (x.st(0)) * (x.st(1));  // 48e44d fmul st(1)
    x.push(F(0x7930e8u));  // 48e44f fld dword ptr [0x7930e8]
    x.st(2) = (x.st(2)) * (x.st(0));  // 48e455 fmulp st(2)
    x.pop();  // 48e455 fmulp st(2)
    x.push(F(ebp - 0x46u));  // 48e457 fld dword ptr [ebp - 0x46]
    x.push(F(ebp - 0x42u));  // 48e45a fld dword ptr [ebp - 0x42]
    x.push(F(ebp - 0x46u));  // 48e45d fld dword ptr [ebp - 0x46]
    edx = rd32((ebp + 0x76u));  // 48e460 mov edx, dword ptr [ebp + 0x76]
    x.push(F(ebp - 0x42u));  // 48e463 fld dword ptr [ebp - 0x42]
    ecx = rd32((ecx + 0x5efac8u));  // 48e466 mov ecx, dword ptr [ecx + 0x5efac8]
    ebx = rd32((edx * 4u + 0x55e2b4u));  // 48e46c mov ebx, dword ptr [edx*4 + 0x55e2b4]
    eax = (edx * 4u);  // 48e473 lea eax, [edx*4]
    esi = (ebx * 4u);  // 48e47a lea esi, [ebx*4]
    { const reg32 a_ = eax, b_ = edx; eax = a_ + b_; fl.add(a_, b_); }  // 48e481 add eax, edx
    x.fxch(3);  // 48e483 fxch st(3)
    x.st(0) = (x.st(0)) - (x.st(4));  // 48e485 fsub st(4)
    carry_ = (eax >> 26) & 1u;  // 48e487 shl eax, 6
    { const reg32 r_ = eax << 6; eax = r_; fl.logic(r_); }  // 48e487 shl eax, 6
    x.fxch(2);  // 48e48a fxch st(2)
    x.st(0) = (x.st(0)) - (x.st(5));  // 48e48c fsub st(5)
    { const reg32 a_ = esi, b_ = ebx; esi = a_ + b_; fl.add(a_, b_); }  // 48e48e add esi, ebx
    x.fxch(1);  // 48e490 fxch st(1)
    x.st(4) = (x.st(4)) + (x.st(0));  // 48e492 faddp st(4)
    x.pop();  // 48e492 faddp st(4)
    carry_ = (esi >> 30) & 1u;  // 48e494 shl esi, 2
    { const reg32 r_ = esi << 2; esi = r_; fl.logic(r_); }  // 48e494 shl esi, 2
    x.fxch(2);  // 48e497 fxch st(2)
    x.st(4) = (x.st(4)) + (x.st(0));  // 48e499 faddp st(4)
    x.pop();  // 48e499 faddp st(4)
    SF((esi + eax + 0x7925f8u), x.st(0));  // 48e49b fstp dword ptr [esi + eax + 0x7925f8]
    x.pop();  // 48e49b fstp dword ptr [esi + eax + 0x7925f8]
    SF((esi + eax + 0x7925fcu), x.st(0));  // 48e4a2 fstp dword ptr [esi + eax + 0x7925fc]
    x.pop();  // 48e4a2 fstp dword ptr [esi + eax + 0x7925fc]
    SF((esi + eax + 0x792600u), x.st(0));  // 48e4a9 fstp dword ptr [esi + eax + 0x792600]
    x.pop();  // 48e4a9 fstp dword ptr [esi + eax + 0x792600]
    SF((esi + eax + 0x792604u), x.st(0));  // 48e4b0 fstp dword ptr [esi + eax + 0x792604]
    x.pop();  // 48e4b0 fstp dword ptr [esi + eax + 0x792604]
    ecx = rd32((ecx + 0x1f0u));  // 48e4b7 mov ecx, dword ptr [ecx + 0x1f0]
    wr32((esi + eax + 0x792608u), ecx);  // 48e4bd mov dword ptr [esi + eax + 0x792608], ecx
    ecx = (ebx + 1u);  // 48e4c4 lea ecx, [ebx + 1]
    wr32((edx * 4u + 0x55e2b4u), ecx);  // 48e4c7 mov dword ptr [edx*4 + 0x55e2b4], ecx
L_48e4ce:
    fl.sub(rd32((0x55fe3cu)), 0u);  // 48e4ce cmp dword ptr [0x55fe3c], 0
    if (fl.e()) goto L_48e761;  // 48e4d5 je 0x48e761
    esi = rd32((0x7a3d10u));  // 48e4db mov esi, dword ptr [0x7a3d10]
    fl.logic(esi & esi);  // 48e4e1 test esi, esi
    if (fl.ne()) goto L_48e761;  // 48e4e3 jne 0x48e761
    ecx = 4294967295u;  // 48e4e9 mov ecx, 0xffffffff
    edx = 5628608u;  // 48e4ee mov edx, 0x55e2c0
    eax = 5628604u;  // 48e4f3 mov eax, 0x55e2bc
    wr32((0x563d3cu), esi);  // 48e4f8 mov dword ptr [0x563d3c], esi
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4dab10, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48e4fe call 0x4dab10
    esp += 0u;  // 48e4fe call 0x4dab10
    eax = rd32((0x55e2bcu));  // 48e503 mov eax, dword ptr [0x55e2bc]
    esi = rd32((0x7d367cu));  // 48e508 mov esi, dword ptr [0x7d367c]
    ebx = rd32((0x7d3678u));  // 48e50e mov ebx, dword ptr [0x7d3678]
    edi = rd32((0x55e2c0u));  // 48e514 mov edi, dword ptr [0x55e2c0]
    { const reg32 a_ = eax, b_ = 8u; eax = a_ - b_; fl.sub(a_, b_); }  // 48e51a sub eax, 8
    { const reg32 a_ = ebx, b_ = edi; ebx = a_ - b_; fl.sub(a_, b_); }  // 48e51d sub ebx, edi
    wr32((0x55e2bcu), eax);  // 48e51f mov dword ptr [0x55e2bc], eax
    edx = eax;  // 48e524 mov edx, eax
    { const reg32 a_ = esi, b_ = eax; esi = a_ - b_; fl.sub(a_, b_); }  // 48e526 sub esi, eax
    { const reg32 r_ = edx ^ eax; edx = r_; fl.logic(r_); }  // 48e528 xor edx, eax
L_48e52a:
    eax = rd32((ebp + 0x76u));  // 48e52a mov eax, dword ptr [ebp + 0x76]
    fl.sub(edx, rd32((eax * 4u + 0x55e2b4u)));  // 48e52d cmp edx, dword ptr [eax*4 + 0x55e2b4]
    if (fl.ge()) goto L_48e5aa;  // 48e534 jge 0x48e5aa
    wr32((ebp + 0x7au), eax);  // 48e53a mov dword ptr [ebp + 0x7a], eax
    edi = rd32((ebp + 0x7au));  // 48e53d mov edi, dword ptr [ebp + 0x7a]
    carry_ = (eax >> 30) & 1u;  // 48e540 shl eax, 2
    { const reg32 r_ = eax << 2; eax = r_; fl.logic(r_); }  // 48e540 shl eax, 2
    { const reg32 a_ = eax, b_ = edi; eax = a_ + b_; fl.add(a_, b_); }  // 48e543 add eax, edi
    carry_ = (eax >> 26) & 1u;  // 48e545 shl eax, 6
    { const reg32 r_ = eax << 6; eax = r_; fl.logic(r_); }  // 48e545 shl eax, 6
    wr32((ebp + 0x7eu), esi);  // 48e548 mov dword ptr [ebp + 0x7e], esi
    wr32((ebp - 0xau), eax);  // 48e54b mov dword ptr [ebp - 0xa], eax
    eax = (edx * 4u);  // 48e54e lea eax, [edx*4]
    x.push(double(int32_t(rd32((ebp + 0x7eu)))));  // 48e555 fild dword ptr [ebp + 0x7e]
    { const reg32 a_ = eax, b_ = edx; eax = a_ + b_; fl.add(a_, b_); }  // 48e558 add eax, edx
    edi = rd32((ebp - 0xau));  // 48e55a mov edi, dword ptr [ebp - 0xa]
    carry_ = (eax >> 30) & 1u;  // 48e55d shl eax, 2
    { const reg32 r_ = eax << 2; eax = r_; fl.logic(r_); }  // 48e55d shl eax, 2
    SF((ebp + 0x1au), x.st(0));  // 48e560 fstp dword ptr [ebp + 0x1a]
    x.pop();  // 48e560 fstp dword ptr [ebp + 0x1a]
    { const reg32 a_ = edi, b_ = eax; edi = a_ + b_; fl.add(a_, b_); }  // 48e563 add edi, eax
    x.push(F(ebp + 0x1au));  // 48e565 fld dword ptr [ebp + 0x1a]
    fl.fcmp(x.st(0), F(edi + 0x7925f8u));  // 48e568 fcomp dword ptr [edi + 0x7925f8]
    x.pop();  // 48e568 fcomp dword ptr [edi + 0x7925f8]
    // fnstsw ax  // 48e56e fnstsw ax
    // sahf  // 48e570 sahf
    if (fl.b()) goto L_48e5a7;  // 48e571 jb 0x48e5a7
    wr32((ebp + 0x7eu), ebx);  // 48e573 mov dword ptr [ebp + 0x7e], ebx
    x.push(double(int32_t(rd32((ebp + 0x7eu)))));  // 48e576 fild dword ptr [ebp + 0x7e]
    SF((ebp + 2u), x.st(0));  // 48e579 fst dword ptr [ebp + 2]
    fl.fcmp(x.st(0), F(edi + 0x7925fcu));  // 48e57c fcomp dword ptr [edi + 0x7925fc]
    x.pop();  // 48e57c fcomp dword ptr [edi + 0x7925fc]
    // fnstsw ax  // 48e582 fnstsw ax
    // sahf  // 48e584 sahf
    if (fl.b()) goto L_48e5a7;  // 48e585 jb 0x48e5a7
    x.push(F(ebp + 0x1au));  // 48e587 fld dword ptr [ebp + 0x1a]
    fl.fcmp(x.st(0), F(edi + 0x792600u));  // 48e58a fcomp dword ptr [edi + 0x792600]
    x.pop();  // 48e58a fcomp dword ptr [edi + 0x792600]
    // fnstsw ax  // 48e590 fnstsw ax
    // sahf  // 48e592 sahf
    if (fl.a()) goto L_48e5a7;  // 48e593 ja 0x48e5a7
    x.push(F(ebp + 2u));  // 48e595 fld dword ptr [ebp + 2]
    fl.fcmp(x.st(0), F(edi + 0x792604u));  // 48e598 fcomp dword ptr [edi + 0x792604]
    x.pop();  // 48e598 fcomp dword ptr [edi + 0x792604]
    // fnstsw ax  // 48e59e fnstsw ax
    // sahf  // 48e5a0 sahf
    if (fl.a()) goto L_48e5a7;  // 48e5a1 ja 0x48e5a7
    ecx = edx;  // 48e5a3 mov ecx, edx
    goto L_48e5aa;  // 48e5a5 jmp 0x48e5aa
L_48e5a7:
    { const reg32 r_ = edx + 1u; edx = r_; fl.arith(r_); }  // 48e5a7 inc edx
    goto L_48e52a;  // 48e5a8 jmp 0x48e52a
L_48e5aa:
    fl.sub(ecx, reg32(-1));  // 48e5aa cmp ecx, -1
    if (fl.e()) goto L_48e761;  // 48e5ad je 0x48e761
    fl.logic(rd8((0x55e5edu)) & 1u);  // 48e5b3 test byte ptr [0x55e5ed], 1
    if (fl.e()) goto L_48e6c5;  // 48e5ba je 0x48e6c5
    edx = rd32((ebp + 0x76u));  // 48e5c0 mov edx, dword ptr [ebp + 0x76]
    edi = (edx * 4u);  // 48e5c3 lea edi, [edx*4]
    eax = (ecx * 4u);  // 48e5ca lea eax, [ecx*4]
    { const reg32 a_ = edi, b_ = edx; edi = a_ + b_; fl.add(a_, b_); }  // 48e5d1 add edi, edx
    { const reg32 a_ = eax, b_ = ecx; eax = a_ + b_; fl.add(a_, b_); }  // 48e5d3 add eax, ecx
    carry_ = (edi >> 26) & 1u;  // 48e5d5 shl edi, 6
    { const reg32 r_ = edi << 6; edi = r_; fl.logic(r_); }  // 48e5d5 shl edi, 6
    eax = rd32((edi + eax * 4u + 0x792608u));  // 48e5d8 mov eax, dword ptr [edi + eax*4 + 0x792608]
    eax = rd32((eax * 4u + 0x5efac8u));  // 48e5df mov eax, dword ptr [eax*4 + 0x5efac8]
    fl.logic(rd8((eax + 0x200u)) & 1u);  // 48e5e6 test byte ptr [eax + 0x200], 1
    if (fl.e()) goto L_48e5fc;  // 48e5ed je 0x48e5fc
    fl.sub(rd32((eax + 0x2c4u)), 3u);  // 48e5ef cmp dword ptr [eax + 0x2c4], 3
    if (fl.e()) goto L_48e6c5;  // 48e5f6 je 0x48e6c5
L_48e5fc:
    edx = rd32((ebp + 0x76u));  // 48e5fc mov edx, dword ptr [ebp + 0x76]
    edi = (edx * 4u);  // 48e5ff lea edi, [edx*4]
    eax = (ecx * 4u);  // 48e606 lea eax, [ecx*4]
    { const reg32 a_ = edi, b_ = edx; edi = a_ + b_; fl.add(a_, b_); }  // 48e60d add edi, edx
    { const reg32 a_ = eax, b_ = ecx; eax = a_ + b_; fl.add(a_, b_); }  // 48e60f add eax, ecx
    carry_ = (edi >> 26) & 1u;  // 48e611 shl edi, 6
    { const reg32 r_ = edi << 6; edi = r_; fl.logic(r_); }  // 48e611 shl edi, 6
    eax = rd32((edi + eax * 4u + 0x792608u));  // 48e614 mov eax, dword ptr [edi + eax*4 + 0x792608]
    eax = rd32((eax * 4u + 0x5efac8u));  // 48e61b mov eax, dword ptr [eax*4 + 0x5efac8]
    fl.logic(rd8((eax + 0x200u)) & 1u);  // 48e622 test byte ptr [eax + 0x200], 1
    if (fl.e()) goto L_48e638;  // 48e629 je 0x48e638
    fl.sub(rd32((eax + 0x2c4u)), 4u);  // 48e62b cmp dword ptr [eax + 0x2c4], 4
    if (fl.e()) goto L_48e6c5;  // 48e632 je 0x48e6c5
L_48e638:
    fl.sub(rd32((0x6fd3a0u)), 2u);  // 48e638 cmp dword ptr [0x6fd3a0], 2
    if (fl.l()) goto L_48e675;  // 48e63f jl 0x48e675
    edx = rd32((ebp + 0x76u));  // 48e641 mov edx, dword ptr [ebp + 0x76]
    eax = (edx * 4u);  // 48e644 lea eax, [edx*4]
    { const reg32 a_ = eax, b_ = edx; eax = a_ - b_; fl.sub(a_, b_); }  // 48e64b sub eax, edx
    carry_ = (eax >> 29) & 1u;  // 48e64d shl eax, 3
    { const reg32 r_ = eax << 3; eax = r_; fl.logic(r_); }  // 48e64d shl eax, 3
    { const reg32 a_ = eax, b_ = edx; eax = a_ - b_; fl.sub(a_, b_); }  // 48e650 sub eax, edx
    carry_ = (eax >> 28) & 1u;  // 48e652 shl eax, 4
    { const reg32 r_ = eax << 4; eax = r_; fl.logic(r_); }  // 48e652 shl eax, 4
    fl.sub(esi, rd32((eax + 0x7497d8u)));  // 48e655 cmp esi, dword ptr [eax + 0x7497d8]
    if (fl.l()) goto L_48e675;  // 48e65b jl 0x48e675
    fl.sub(esi, rd32((eax + 0x7497e0u)));  // 48e65d cmp esi, dword ptr [eax + 0x7497e0]
    if (fl.g()) goto L_48e675;  // 48e663 jg 0x48e675
    fl.sub(ebx, rd32((eax + 0x7497dcu)));  // 48e665 cmp ebx, dword ptr [eax + 0x7497dc]
    if (fl.l()) goto L_48e675;  // 48e66b jl 0x48e675
    fl.sub(ebx, rd32((eax + 0x7497e4u)));  // 48e66d cmp ebx, dword ptr [eax + 0x7497e4]
    if (fl.le()) goto L_48e6c5;  // 48e673 jle 0x48e6c5
L_48e675:
    fl.sub(rd32((0x6fd3a0u)), 2u);  // 48e675 cmp dword ptr [0x6fd3a0], 2
    if (fl.ne()) goto L_48e687;  // 48e67c jne 0x48e687
    fl.sub(rd32((0x7cd948u)), 0u);  // 48e67e cmp dword ptr [0x7cd948], 0
    if (fl.ne()) goto L_48e6c5;  // 48e685 jne 0x48e6c5
L_48e687:
    edi = 0u;  // 48e687 xor edi, edi
    fl.logic(0u);  // 48e687 xor edi, edi
    edx = rd32((ebp + 0x76u));  // 48e689 mov edx, dword ptr [ebp + 0x76]
    wr32((0x55e5ecu), edi);  // 48e68c mov dword ptr [0x55e5ec], edi
    edi = (edx * 4u);  // 48e692 lea edi, [edx*4]
    eax = (ecx * 4u);  // 48e699 lea eax, [ecx*4]
    { const reg32 a_ = edi, b_ = edx; edi = a_ + b_; fl.add(a_, b_); }  // 48e6a0 add edi, edx
    { const reg32 a_ = eax, b_ = ecx; eax = a_ + b_; fl.add(a_, b_); }  // 48e6a2 add eax, ecx
    carry_ = (edi >> 26) & 1u;  // 48e6a4 shl edi, 6
    { const reg32 r_ = edi << 6; edi = r_; fl.logic(r_); }  // 48e6a4 shl edi, 6
    eax = rd32((edi + eax * 4u + 0x792608u));  // 48e6a7 mov eax, dword ptr [edi + eax*4 + 0x792608]
    wr32((0x6fd4f8u), eax);  // 48e6ae mov dword ptr [0x6fd4f8], eax
    edx = eax;  // 48e6b3 mov edx, eax
    eax = rd32((ebp + 0x76u));  // 48e6b5 mov eax, dword ptr [ebp + 0x76]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4215c0, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48e6b8 call 0x4215c0
    esp += 0u;  // 48e6b8 call 0x4215c0
    eax = rd32((ebp + 0x76u));  // 48e6bd mov eax, dword ptr [ebp + 0x76]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x485640, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48e6c0 call 0x485640
    esp += 0u;  // 48e6c0 call 0x485640
L_48e6c5:
    esp -= 4; wr32(esp, 1058642330u);  // 48e6c5 push 0x3f19999a
    edx = rd32((ebp + 0x76u));  // 48e6ca mov edx, dword ptr [ebp + 0x76]
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4d1390, esp, 1, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48e6cd call 0x4d1390
    esp += 4u;  // 48e6cd call 0x4d1390
    edi = (edx * 4u);  // 48e6d2 lea edi, [edx*4]
    eax = (ecx * 4u);  // 48e6d9 lea eax, [ecx*4]
    { const reg32 a_ = edi, b_ = edx; edi = a_ + b_; fl.add(a_, b_); }  // 48e6e0 add edi, edx
    { const reg32 a_ = eax, b_ = ecx; eax = a_ + b_; fl.add(a_, b_); }  // 48e6e2 add eax, ecx
    carry_ = (edi >> 26) & 1u;  // 48e6e4 shl edi, 6
    { const reg32 r_ = edi << 6; edi = r_; fl.logic(r_); }  // 48e6e4 shl edi, 6
    eax = rd32((edi + eax * 4u + 0x792608u));  // 48e6e7 mov eax, dword ptr [edi + eax*4 + 0x792608]
    eax = rd32((eax * 4u + 0x5efac8u));  // 48e6ee mov eax, dword ptr [eax*4 + 0x5efac8]
    eax = rd32((eax + 0x220u));  // 48e6f5 mov eax, dword ptr [eax + 0x220]
    { const reg32 a_ = eax, b_ = 52u; eax = a_ + b_; fl.add(a_, b_); }  // 48e6fb add eax, 0x34
    x.fstp_st(0);  // 48e6fe fstp st(0)
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4d1480, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48e700 call 0x4d1480
    esp += 0u;  // 48e700 call 0x4d1480
    x.st(0) = std::trunc(x.st(0));  // __CHP  // 48e705 call 0x4dfd56
    wr32((ebp + 0x1eu), stored(cpu, x.pop()));  // 48e70a fistp dword ptr [ebp + 0x1e]
    eax = rd32((ebp + 0x1eu));  // 48e70d mov eax, dword ptr [ebp + 0x1e]
    edx = rd32((0x7cdac8u));  // 48e710 mov edx, dword ptr [0x7cdac8]
    { const reg32 a_ = eax, b_ = esi; eax = a_ + b_; fl.add(a_, b_); }  // 48e716 add eax, esi
    fl.sub(eax, edx);  // 48e718 cmp eax, edx
    if (fl.l()) goto L_48e726;  // 48e71a jl 0x48e726
    esi = rd32((ebp + 0x1eu));  // 48e71c mov esi, dword ptr [ebp + 0x1e]
    eax = (edx - 8u);  // 48e71f lea eax, [edx - 8]
    { const reg32 a_ = eax, b_ = esi; eax = a_ - b_; fl.sub(a_, b_); }  // 48e722 sub eax, esi
    esi = eax;  // 48e724 mov esi, eax
L_48e726:
    edx = rd32((ebp + 0x76u));  // 48e726 mov edx, dword ptr [ebp + 0x76]
    eax = (edx * 4u);  // 48e729 lea eax, [edx*4]
    { const reg32 a_ = edx, b_ = eax; edx = a_ + b_; fl.add(a_, b_); }  // 48e730 add edx, eax
    eax = (ecx * 4u);  // 48e732 lea eax, [ecx*4]
    carry_ = (edx >> 26) & 1u;  // 48e739 shl edx, 6
    { const reg32 r_ = edx << 6; edx = r_; fl.logic(r_); }  // 48e739 shl edx, 6
    { const reg32 a_ = eax, b_ = ecx; eax = a_ + b_; fl.add(a_, b_); }  // 48e73c add eax, ecx
    eax = rd32((edx + eax * 4u + 0x792608u));  // 48e73e mov eax, dword ptr [edx + eax*4 + 0x792608]
    eax = rd32((eax * 4u + 0x5efac8u));  // 48e745 mov eax, dword ptr [eax*4 + 0x5efac8]
    { const reg32 a_ = ebx, b_ = 16u; ebx = a_ - b_; fl.sub(a_, b_); }  // 48e74c sub ebx, 0x10
    eax = rd32((eax + 0x220u));  // 48e74f mov eax, dword ptr [eax + 0x220]
    edx = esi;  // 48e755 mov edx, esi
    { const reg32 a_ = eax, b_ = 52u; eax = a_ + b_; fl.add(a_, b_); }  // 48e757 add eax, 0x34
    { const int n0_ = cpu.fpu.count; cpu.ebp = ebp; if (!rawCall(app, cpu, 0x4d1000, esp, 0, eax, edx, ebx, ecx)) return false; pullX87(cpu, n0_, x); }  // 48e75a call 0x4d1000
    esp += 0u;  // 48e75a call 0x4d1000
    x.fstp_st(0);  // 48e75f fstp st(0)
L_48e761:
    esp = (ebp + 0x82u);  // 48e761 lea esp, [ebp + 0x82]
    ebp = rd32(esp); esp += 4;  // 48e767 pop ebp
    edi = rd32(esp); esp += 4;  // 48e768 pop edi
    esi = rd32(esp); esp += 4;  // 48e769 pop esi
    ecx = rd32(esp); esp += 4;  // 48e76a pop ecx
    ebx = rd32(esp); esp += 4;  // 48e76b pop ebx
    result = eax; popped_ = 0u; return true;  // 48e76c ret
    return false;
}
// (popped = 0
}  // namespace

/* ---- The adapters ---- */

void loadingTextPrepared(App* app, CPU& cpu)
{
    const Entry e(cpu);
    if (loadingText(app, cpu, e))
        e.back(cpu, e.eax);
}

void floatsSwapped(App* app, CPU& cpu)
{
    const Entry e(cpu);
    swapFloats(app, e.eax, e.edx);
    e.back(cpu, e.eax);
}

void fourIntegersAppended(App* app, CPU& cpu)
{
    const Entry e(cpu);
    fourAppended(app, e, e.eax, int32_t(e.edx), int32_t(e.ebx), int32_t(e.ecx), int32_t(e.arg(app, 0)));
    e.back(cpu, 80 * e.eax, 4);
}

void pointTransformed(App* app, CPU& cpu)
{
    const Entry e(cpu);
    pointMoved(app, e, e.eax, e.esp + 4, e.esp + 8, e.arg(app, 2));
    e.back(cpu, e.eax, 0xc);
}

void boxDrawn(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 result;
    if (boxDrawnOf(app, cpu, e, result))
        e.back(cpu, result, 0x10);
}

void indicatorGeometryUpdated(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 result;
    if (indicatorGeometry(app, cpu, e, result))
        e.back(cpu, result, 8);
}

void textAlignedDrawn(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 result;
    if (textAligned(app, cpu, e, result))
        e.back(cpu, result, 0x18);
}

void percentTextDrawn(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 result;
    if (percentDrawn(app, cpu, e, result))
        e.back(cpu, result, 0x18);
}

void listRowsDrawn(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 result;
    if (rowsDrawn(app, cpu, e, result))
        e.back(cpu, result, 0x1c);
}

void timeTextDrawn(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 result;
    if (timeDrawn(app, cpu, e, result))
        e.back(cpu, result, 0x14);
}

void viewZoomStepped(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 result;
    if (zoomStepped(app, cpu, e, result))
        e.back(cpu, result);
}

void viewPaneStepped(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 result;
    if (paneStepped(app, cpu, e, result))
        e.back(cpu, result);
}

void screenScalesSet(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, scalesSet(app, e));
}

void rectangleQuadSet(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 result;
    if (cornersOfRectangle(app, cpu, e, result))
        e.back(cpu, result);
}

void stackQuadSet(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 result;
    if (cornersFromStack(app, cpu, e, result))
        e.back(cpu, result, 0x28);
}

void labelTextSet(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 result;
    if (labelSet(app, cpu, e, result))
        e.back(cpu, result);
}

void pointClipped(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, clipPointOf(app, e.eax, e.edx, e.ebx, e.ecx, e.arg(app, 0)), 4);
}

void segmentClippedToRect(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, segmentClipped(app, e.frame(3), e.eax, e.edx, e.ebx, e.ecx));
}

void segmentCutIntoPieces(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, segmentPieces(app, e.frame(3), e.eax, e.edx, e.ebx, e.ecx));
}

void strokeClippedDrawn(App* app, CPU& cpu)
{
    const Entry e(cpu);
    if (strokeDrawn(app, cpu, e))
        e.back(cpu, e.eax, 4);
}

void standingsGapMeasured(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, reg32(standingsGap(app, e.eax, e.edx)));
}

void floatsQuadSet(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 result;
    if (quadOfBox(app, cpu, e, result))
        e.back(cpu, result, 0x28);
}

void hudStarted(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 result;
    if (startHud(app, cpu, e, result))
        e.back(cpu, result);
}

void rankTextDrawn(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 result;
    if (rankDrawn(app, cpu, e, result))
        e.back(cpu, result, 0x1c);
}

void labelRowsDrawn(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 result;
    if (drawLabelRows(app, cpu, e, result))
        e.back(cpu, result, 0x10);
}

void standingsDrawn(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 result;
    if (drawStandings(app, cpu, e, result))
        e.back(cpu, result, 0x10);
}

void columnsStarted(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 result;
    if (startColumns(app, cpu, e, result))
        e.back(cpu, result, 8);
}

void mapOutlineStarted(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 result;
    if (startMapOutline(app, cpu, e, result))
        e.back(cpu, result);
}

void hudViewsSet(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 result;
    if (setHudViews(app, cpu, e, result))
        e.back(cpu, result);
}

void hudPaneSet(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 result;
    if (setHudPane(app, cpu, e, result))
        e.back(cpu, result);
}

void hudTablesStarted(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 result;
    if (startHudTables(app, cpu, e, result))
        e.back(cpu, result);
}

void indicatorQuadSet(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 result;
    if (setIndicator(app, cpu, e, result))
        e.back(cpu, result);
}

void gaugeQuadClipped(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 result, popped = 0;
    if (decompGaugeQuad(app, cpu, e, result, popped))
        e.back(cpu, result, popped);
}

void segmentsMet(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 result, popped = 0;
    if (decompSegmentsMet(app, cpu, e, result, popped))
        e.back(cpu, result, popped);
}

void boxQuadsSet(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 result, popped = 0;
    if (decompBoxQuads(app, cpu, e, result, popped))
        e.back(cpu, result, popped);
}

void markersSet(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 result, popped = 0;
    if (decompMarkers(app, cpu, e, result, popped))
        e.back(cpu, result, popped);
}

void mapPointsDrawn(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 result, popped = 0;
    if (decompMapPoints(app, cpu, e, result, popped))
        e.back(cpu, result, popped);
}

}  // namespace hud
}  // namespace port
}  // namespace nfs3hp
