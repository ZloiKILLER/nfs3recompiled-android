#include "game_settings.h"
#include "native_load_carmodels.h"
#include "port_obj3d.h"
#include "native_frontend_loop.h"
#include "port_draw2d.h"
#include "port_fields.h"
#include "port_snd_stream_calls.h"
#include "native_collide.h"
#include "native_decompiled.h"
#include "native_ea_memory.h"
#include "native_ea_threads.h"
#include "native_frontend_rendc.h"
#include "native_frontend_rendd.h"
#include "native_frontend_rende.h"
#include "native_render.h"
#include "native_frontend_wata.h"
#include "native_frontend_glue.h"
#include "port_snd.h"
#include "native_frontend_simd.h"
#include "native_graphics.h"
#include "native_load_cardb.h"
#include "native_raceload.h"
#include "native_vertices.h"
#include "native_watcom.h"

#include <cstring>
#include "native_guest.h"

namespace nfs3hp
{
namespace port
{
namespace
{

/* A call into the game's code through guest::pointerCall: the register arguments of `a`, `stack` pushed, from the
 * game's stack at esp.  Its eax; false when the game stops. */
inline bool callPointer(App* app, ThreadState& ts, u32 esp, u32 target, Args a, std::initializer_list<u32> stack,
                        u32& result)
{
    ts.sp = esp;
    nfs::Args registers;
    registers.eax = a.eax;
    registers.edx = a.edx;
    registers.ebx = a.ebx;
    registers.ecx = a.ecx;
    const std::optional<u32> value = guest::pointerCall(app, ts, target, registers, stack);
    if (!value)
        return false;
    result = *value;
    return true;
}

inline bool callPointer(App* app, ThreadState& ts, u32 esp, u32 target, Args a = {},
                        std::initializer_list<u32> stack = {})
{
    u32 ignored;
    return callPointer(app, ts, esp, target, a, stack, ignored);
}

}  // namespace
}  // namespace port
}  // namespace nfs3hp

/* Each sum and product rounds as the x87 code rounds it: none fused. */
#if defined(__clang__)
#pragma clang fp contract(off)
#elif defined(_MSC_VER)
#pragma fp_contract(off)
#endif

/* The menus' 3D object, rewritten (port_obj3d.h): the obj3d widget's
 * handlers (0x466cf0 drawn, 0x4675b0 a key, 0x4675d0 passed over, 0x467650
 * the mouse let go, 0x467700 shown, 0x467930 hidden); its loader, which
 * reads the car's .viv a part at a time through EA's asynchronous reads while
 * the screen runs (0x467ac0-0x468410, 0x468860), the fade between two cars
 * (0x468580, 0x4685e0), the car's colour and texture (0x468da0, 0x468f00);
 * the object dragged round (0x4667d0), its rectangle (0x4668b0), its lights'
 * flares (0x465eb0-0x466740); a car's .viv path (0x439360); the logo's quad
 * (0x4d79b0).
 *
 * The registers the
 * game's code leaves otherwise than the caller had them, and which a caller
 * reads (lib_boundary's reading of each call site, the widget table's
 * callers -- the menus' loop -- included): edx after a key (0x4675b0 leaves
 * it 8, which the key's dispatch at 0x44a36a hands up), ebx, ecx and edx
 * after the logo's quad (0x4d79b0 leaves them as the clipped quad's draw
 * does, and render::loadingScreen hands edx on to 0x4ead90), esi after the
 * flares (0x466740 leaves it at its frame; its one caller, 0x466cf0, sets it
 * again first).  Those the thunks leave as the game's code does.
 *
 * What the game's code leaves in esi and edi on the way into a callee is
 * kept too where the callee reads it (the renderer's lists' functions take
 * their list in esi), and where it lands on the stack among what a logged
 * call reads (the saved registers above sprintf's arguments, 0x467c40).
 *
 * The x87 is the game's: a value the code loads and stores as a float goes
 * through nfs::Float as the code has it, a word it moves by mov is copied as
 * it is (a float's bits, NaN or not). */
namespace nfs3hp
{
namespace port
{

namespace obj3d
{

namespace
{

using nfs::Float;

#pragma pack(push, 1)
/* A corner of a quad as the clipper and the drivers take it. */
struct Vertex
{
    float x, y, z, w;
    uint32_t colour;
    uint8_t code;
    uint8_t unused[3];
    float u, v;
};
/* The obj3d widget: a menu item (its head the menus' own) and the object's
 * state after it. */
struct Object
{
    uint8_t head[5];
    uint8_t flags;              // 0x05: 1 its rectangle unknown, 4 dragged
    uint8_t unused06[0x14];
    int16_t rect[4];            // 0x1a: on the screen
    uint8_t unused22[0x1a];
    u32 model;                // 0x3c: the model drawn
    u32 shadow;               // 0x40: its copy, the shadow's
    float angles[3];            // 0x44: degrees
    float spins[3];             // 0x50: degrees a tick
    u32 position[3];          // 0x5c: floats' bits
    float turn[3];              // 0x68: turns
    float spin[3];              // 0x74: turns a millisecond
    int16_t kind;               // 0x80: whose car, or a track's, the generic one's
    int16_t shadowed;           // 0x82
    int16_t unused84;
    int16_t number;             // 0x86: the generic car's
    uint8_t dragging;           // 0x88
    uint8_t unused89;
    int16_t grabX;              // 0x8a: where the mouse took hold
    int32_t alpha;              // 0x8c: 0 to 0xff, faded
    int32_t lastClock;          // 0x90
    int32_t fadeClock;          // 0x94
    int32_t mouseClock;         // 0x98
    int16_t loadingId;          // 0x9c: the car loading
    int16_t shownId;            // 0x9e: the car shown
    int32_t checkClock;         // 0xa0
    u32 texture;              // 0xa4
    u32 modelData;            // 0xa8
    u32 textureData;          // 0xac
    uint8_t wanted;             // 0xb0: another car to load
    uint8_t loading;            // 0xb1
    uint8_t automatic;          // 0xb2
    uint8_t textureStale;       // 0xb3
    uint8_t logo;               // 0xb4
    uint8_t unusedB5[3];
    u32 loader;               // 0xb8
};
/* What the loader keeps while it reads the car's .viv (0x1a0 bytes). */
struct Loader
{
    char viv[0x100];
    char modelName[0x20];       // 0x100
    char textureName[0x20];     // 0x120
    char shapesName[0x20];      // 0x140
    u32 reads[5];             // 0x160: EA's reads, the .viv's first bytes to the shapes
    u32 head[4];              // 0x174: the .viv's first bytes, big-endian
    u32 modelSize;            // 0x184
    u32 vivHead;              // 0x188
    u32 modelData;            // 0x18c
    u32 textureData;          // 0x190
    u32 shapesData;           // 0x194
    int32_t state;              // 0x198
    int32_t waiting;            // 0x19c
};
#pragma pack(pop)
static_assert(sizeof(Vertex) == 0x20, "a vertex is 0x20 bytes");
static_assert(sizeof(Object) == 0xbc, "the object is 0xbc bytes");
static_assert(sizeof(Loader) == 0x1a0, "the loader is 0x1a0 bytes");

/* The clock; the mouse's x (a word of it read); no mouse. */
constexpr Var<int32_t> kClock{0x79c1a4}, kMouseX{0x55e5e4}, kNoMouse{0x555748};
/* The menus over a race (0x40 of the flags), the pause's states, its turn. */
constexpr Var<uint8_t> kMenuFlags{0x55eb2f};
constexpr Var<int32_t> kPaused{0x553030}, kPauseShown{0x553034};
constexpr Var<u32> kPauseTurn{0x55302c};
/* Whose cars and which track: players' (the cars' menu's 1 and 2), the
 * track, the generic car's count of players, the cars shown, the random car
 * last drawn. */
constexpr Var<int32_t> kCarShown9{0x664788}, kCarShown10{0x664784}, kRandomCar{0x55a4dc};
/* The projection: the scale and centre of x and y. */
constexpr Var<float> kScaleX{0x56009c}, kScaleY{0x5600a0}, kCentreX{0x5600a4}, kCentreY{0x5600a8};
/* The clip's bounds, compared as the floats' bits. */
constexpr Var<int32_t> kClipFar{0x7d34f8}, kClipLeft{0x7d34fc}, kClipBottom{0x7d3500}, kClipTop{0x7d3504};
constexpr Var<int32_t> kClipRight{0x7d350c};
/* The flares' texture and its record's tag. */
constexpr Var<u32> kFlareTexture{0x678a54};
constexpr u32 kFlareTag = 0x678a50;
/* The logo: its texture, palette, place; the 2D screens' quad, depth, w. */
constexpr Var<u32> kLogoTexture{0x55a4d4}, kLogoPalette{0x55a4d8};
constexpr Var<int32_t> kLogoX{0x678a80}, kLogoY{0x678a7c};
constexpr u32 kQuadAt = 0x563ab0;
constexpr Records<Vertex> kQuad{kQuadAt};
constexpr Var<float> kScreenDepth{0x563aa8}, kScreenW{0x563aac};
/* The shadow's pass, the lights' list and its tail; the driver's flags; the
 * mirror's state set after. */
constexpr u32 kShadowPass = 0x554e4c, kLights = 0x55a4c4;
constexpr Var<u32> kLightsTail{0x55a4d0};
constexpr Var<uint8_t> kDriverFlags{0x7a3a58};
constexpr Var<u32> kAfterState{0x7a3a60};
/* The code's tables: the light's direction, the identity, two of zeros. */
constexpr u32 kLightDirection = 0x465d10, kIdentity = 0x465d1c, kNoColour = 0x465d40, kNoColour2 = 0x465d50;
/* Folders and names: the cars', the generic car's .viv folder, the track's. */
constexpr u32 kCarsFolder = 0x7a28b4, kGenericFolder = 0x7a2db4, kTrackFolder = 0x7a31b4;
constexpr u32 kBackslash = 0x537698, kViv = 0x53769c, kNul = 0x53ac80, kEmpty = 0x53ac68;
constexpr u32 kGenericViv = 0x53ac84, kModelNumber = 0x53ac90, kTrackViv = 0x53ac98;
constexpr u32 kFce = 0x53aca4, kTwo = 0x53acac, kTga = 0x53acb4, kFsh = 0x53acbc, kLogo = 0x53acc8;
constexpr u32 kGenericName = 0x678a84, kTrackName = 0x678a94;
constexpr u32 kStartFailed = 0x53acd0, kPreheadFailed = 0x53acf4, kHeadFailed = 0x53ad18;
constexpr u32 kModelFailed = 0x53ad38, kTextureFailed = 0x53ad58;
/* The x87's constants. */
constexpr u32 kQuarterA = 0x53abc8, kQuarterB = 0x53abcc, kWhite = 0x53abf0, kHeadAlpha = 0x53abf4;
constexpr u32 kTailFacing = 0x53abf8, kQuarterC = 0x53ac20, kTailScale = 0x53ac38, kTailAlpha = 0x53ac3c;
constexpr u32 kDragTurn = 0x53ac40, kDragSpin = 0x53ac44, kPerDegree = 0x53ac50, kSlowing = 0x53ac54;
constexpr u32 kHeadA = 0x53abd0, kHeadB = 0x53abd8, kHeadNear = 0x53abe0, kHeadFour = 0x53abe8;
constexpr u32 kTailMinus = 0x53ac00, kTailOver = 0x53ac08, kTailNear = 0x53ac10, kTailFour = 0x53ac18;
constexpr u32 kTailA = 0x53ac28, kTailB = 0x53ac30, kDragDamp = 0x53ac48, kPerMillisecond = 0x53ac58;
constexpr u32 kTwice = 0x53ac60, kDegrees = 0x53ac70, kHalfBack = 0x53ac78;

/* What is not rewritten. */
constexpr u32 kMenuShownNow = 0x44a660, kMouseKey = 0x49c080, kTrace = 0x42d500;
constexpr u32 kCarRecord = 0x439a50, kCarFilePrefix = 0x439300;
constexpr u32 kCarColourIndex = 0x439790, kCarColourRgb = 0x439810;
constexpr u32 kSprintf = 0x4df690, kMemset = 0x4e0640, kMemmove = 0x4ea4f0;
constexpr u32 kAllocate = 0x4e1620, kFree = 0x4e1890, kRunTasks = 0x4e7630;
constexpr u32 kReadClose = 0x4f1370, kRead = 0x4f15d0, kReadDone = 0x4f1790, kVivEntry = 0x4679f0;
constexpr u32 kLocateShape = 0x4ef270, kShapeIntoTexture = 0x4d5ed0, kAllocTexture = 0x49c600;
constexpr u32 kModelReady = 0x49cfa0, kFreeModel = 0x49d5a0;
constexpr u32 kRoom = 0x4bbde0, kSine = 0x4ea930, kCosine = 0x4ea950;
constexpr u32 kSubtract = 0x4e0000, kUnit = 0x4e01c0, kRotate = 0x4e03b0, kTransform = 0x4e0430;
constexpr u32 kXRotation = 0x4dfd80, kYRotation = 0x4eaab0, kZRotation = 0x4eab80;
constexpr u32 kListEmpty = 0x4bbe70, kListOf = 0x4bbe90, kListJoined = 0x4bbec0, kListTaken = 0x49c470;
constexpr u32 kListSorted = 0x4c7380, kListStart = 0x49c410, kDrawRecords = 0x434950;
constexpr u32 kSetRenderState = 0x431900, kStatesForHud = 0x431b40, kStatesForScreens = 0x431bc0;
constexpr u32 kSunLighting = 0x49d640, kLightFrom = 0x49d750, kLightDirectionSet = 0x49d760;
constexpr u32 kAmbient = 0x49d8f0, kLightAlpha = 0x49d720, kGlareColours = 0x49d800, kDrawModel = 0x49de20;
constexpr u32 kFrameDone = 0x4bbd70, kClipQuad = 0x4c12e0;
constexpr u32 kRandom = 0x4e0d88;                              // EA's random number (port::nextRandom)
constexpr u32 kFilledRectangle = 0x4d8910;

/* The light types of a model's lights. */
constexpr uint8_t kHeadlight = 0x48, kTaillight = 0x54;
/* The kinds whose logo is drawn, whose texture is the cars'. */
constexpr int32_t kPlayer1 = 1, kPlayer2 = 2, kRandom3 = 3, kTrackKind = 5, kSplit = 8;

uint32_t bitsOf(float f)
{
    uint32_t b;
    std::memcpy(&b, &f, 4);
    return b;
}

float fromBits(uint32_t b)
{
    float f;
    std::memcpy(&f, &b, 4);
    return f;
}

u32 word(App* app, u32 at)
{
    return app->getMemory<u32>(at);
}

void setWord(App* app, u32 at, u32 value)
{
    app->getMemory<u32>(at) = value;
}

Float F(App* app, u32 at)
{
    return Float(app->getMemory<float>(at));
}

Float D(App* app, u32 at)
{
    return Float(app->getMemory<double>(at));
}

void putF(App* app, u32 at, const Float& value)
{
    app->getMemory<float>(at) = float(value);
}

Float bitsF(u32 bits)
{
    return Float(fromBits(bits));
}

/* A float through the x87's fld and fstp. */
float viaX87(float f)
{
    return float(Float(f));
}

void copyWords(App* app, u32 to, u32 from, u32 count)
{
    for (u32 i = 0; i < count; ++i)
        setWord(app, to + 4 * i, word(app, from + 4 * i));
}

/* Where a callee's esp is (its return address) when the game's code calls it
 * from `frame` with `words` on the stack. */
u32 callee(u32 frame, u32 words)
{
    return frame - 4 * words - 4;
}

/* A function with a typed core called as C++ (guest::direct) from the game's stack at `esp` (the caller's,
 * the stack words pushed under it counted off), the core given its own arguments; its result as the core
 * gives it.  A port core's `esp` is the frame base, callee(frame, words). */
template <u32 Address, auto Core, typename... Values>
auto callCore(App* app, ThreadState& ts, u32 esp, Values... values)
{
    ts.sp = esp;
    return guest::direct<Address, Core>(app, ts, values...);
}

/* Its result taken as callGame takes eax: false when the game stopped. */
inline bool taken(ThreadState& ts, const std::optional<u32>& value, u32& result)
{
    if (!value || ts.terminate)
        return false;
    result = *value;
    return true;
}

inline bool taken(ThreadState& ts, const Result& value, u32& result)
{
    if (!value || ts.terminate)
        return false;
    result = u32(*value);
    return true;
}

inline bool taken(ThreadState& ts, u32 value, u32& result)
{
    if (ts.terminate)
        return false;
    result = value;
    return true;
}

/* A call whose core gives nothing back, or nothing the code reads: false when the game stopped. */
template <typename T>
inline bool ran(ThreadState& ts, const T&)
{
    return !ts.terminate;
}

inline bool ran(ThreadState& ts)
{
    return !ts.terminate;
}

/* sub_4ea930 or sub_4ea950 as callGame calls them, the angle pushed: simd's sineValue or cosineValue,
 * which leave every register as it was.  nullopt when the game stopped. */
template <u32 Address, auto Core>
std::optional<double> turnValue(App* app, ThreadState& ts, u32 frame, const Args& a, u32 angle)
{
    const double value = callCore<Address, Core>(app, ts, frame, angle);
    if (ts.terminate)
        return std::nullopt;
    return value;
}

/* A rotation's angle pushed as the code pushes it, the core handed its slot (ea's rotateAbout* read
 * the angle there): the call's esp and the slot one word below `frame`. */
template <u32 Address, auto Core>
bool rotated(App* app, ThreadState& ts, u32 frame, u32 matrix, u32 angleBits)
{
    app->getMemory<u32>(frame - 4) = angleBits;
    const std::optional<u32> done = callCore<Address, Core>(app, ts, frame - 4, matrix, frame - 4);
    ts.sp = frame;
    if (!done || ts.terminate)
        return false;
    return true;
}

/* fcom and fnstsw: the x87's compare, CF (c0) and ZF (c3) as sahf leaves them. */
struct Compared
{
    bool cf, zf;
    nfs::u16 word;
    bool above() const { return !cf && !zf; }
    bool belowOrEqual() const { return cf || zf; }
};

Compared fcom(const Float& a, const Float& b)
{
    const nfs::u16 word = fp::compare(double(a), double(b));
    return {(word & 0x100) != 0, (word & 0x4000) != 0, word};
}

/* eax with the x87's status word in ax (fnstsw ax). */
u32 withStatus(const Compared& compared, u32 eax)
{
    return (eax & 0xffff0000u) | compared.word;
}

/* The clip code of a vertex: 0x10 behind the eye or past the far clip, else
 * 8 above the clip, 4 below, 1 left of it, 2 right of it. */
uint8_t clipCode(App* app, const Vertex& v)
{
    const int32_t w = int32_t(bitsOf(v.w));
    if (w < 0 || w >= kClipFar(app))
        return 0x10;
    uint8_t code = 0;
    const int32_t y = int32_t(bitsOf(v.y));
    if (y < 0 || y < kClipTop(app))
        code |= 8;
    else if (y > kClipBottom(app))
        code |= 4;
    const int32_t x = int32_t(bitsOf(v.x));
    if (x < 0 || x < kClipLeft(app))
        code |= 1;
    else if (x > kClipRight(app))
        code |= 2;
    return code;
}

void markCode(App* app, u32 at)
{
    record<Vertex>(app, at).code = clipCode(app, load<Vertex>(app, at));
}

/* A point (floats' bits) projected: its w (1 over its depth), x and y
 * scaled by it about the centre, its depth's bits as they are -- into `v`
 * (x, y, z, w only). */
void project(App* app, ThreadState& ts, u32 xBits, u32 yBits, u32 zBits, Vertex& v)
{
    const float w = float(fp::div(Float(1.0), bitsF(zBits)));
    const Float a = fp::mul(Float(kScaleX(app)), Float(w));
    const Float b = fp::mul(Float(kScaleY(app)), Float(w));
    const Float y = fp::mul(b, bitsF(yBits));
    const Float x = fp::mul(a, bitsF(xBits));
    v.w = w;
    v.x = float(fp::add(x, Float(kCentreX(app))));
    v.z = fromBits(zBits);
    v.y = float(fp::add(y, Float(kCentreY(app))));
}

/* A colour's channels scaled by `scale` of 0xff (the code's mul and its two
 * halves of the colour). */
uint32_t faded(uint32_t colour, uint32_t scale)
{
    const uint32_t low = (colour & 0xff00ffu) * scale;
    const uint32_t high = ((colour & 0xff00ff00u) >> 8) * scale;
    return (high & 0xff00ff00u) | ((low >> 8) & 0xff00ffu);
}

/* An alpha as the code takes it for that scale: shifted up a byte, past 0xff
 * held at 0xff. */
uint32_t fadeOf(int32_t alpha)
{
    const int32_t shifted = int32_t(uint32_t(alpha) << 8);
    const uint32_t s = shifted >= 0x10000 ? 0xffffu : uint32_t(shifted);
    return (s >> 8) & 0xff;
}

/* fistp of an alpha, as the flares round it: below 0 none, past 0xff all of
 * it, else the float kept at `kept` rounded again. */
int32_t flareAlpha(ThreadState& ts, const Float& value, float kept)
{
    if (fp::toInteger<int32_t>(value) < 0)
        return 0;
    if (fp::toInteger<int32_t>(Float(kept)) > 0xff)
        return 0xff;
    return fp::toInteger<int32_t>(Float(kept));
}

/* strcpy and strcat as the code has them inline. */
void copyString(App* app, u32 to, u32 from)
{
    for (;; ++to, ++from)
    {
        const uint8_t c = app->getMemory<uint8_t>(from);
        app->getMemory<uint8_t>(to) = c;
        if (c == 0)
            return;
    }
}

void appendString(App* app, u32 to, u32 from)
{
    while (app->getMemory<uint8_t>(to) != 0)
        ++to;
    copyString(app, to, from);
}

/* The model drawn: a part of it (`part`, `light`) through the renderer, its
 * matrix and place, the lights' list; the depth it gives back not wanted.
 * The call as callGame makes it -- the registers, the five words pushed, the
 * frame where the game's lies, the registers as it leaves them -- with
 * rendc's modelTransformPartCalled run as C++ in place of the jump by
 * address.  False when the game stopped in it. */
bool drawModel(App* app, ThreadState& ts, u32 frame, u32 model, u32 light, u32 lights, u32 part, u32 matrix,
               u32 place, u32 tail)
{
        // The five words the call pushes (the matrix and place, a zero, the lights, the tail) under `frame`.
    callCore<kDrawModel, frontend::rendc::modelTransformPart>(app, ts, frame - 20, model, light, lights, part, matrix,
                                                              place, 0u, u32(kLights), tail);
    if (ts.terminate)
        return false;
    ts.sp = frame;
    return true;
}

}  // namespace

/* ---- 0x439360 ---- */

Result carVivPathOf(App* app, ThreadState& ts, u32 esp, int32_t car, u32 buffer)
{
    const u32 frame = esp - 0x14;
    u32 record, prefix;
    if (!taken(ts, callCore<kCarRecord, load::recordAt>(app, ts, frame, u32(car)),
               record))
        return {};
    app->getMemory<uint8_t>(buffer) = 0;
    copyString(app, buffer, kCarsFolder);
    if (!callCore<0x438340, load::slashAdded>(app, ts, frame, buffer))
        return {};
    appendString(app, buffer, record + 0x110);
    appendString(app, buffer, kBackslash);
    if (!taken(ts, callCore<kCarFilePrefix, load::prefixOf>(app, ts, frame,
                                                             u32(car)),
               prefix))
        return {};
    appendString(app, buffer, prefix);
    appendString(app, buffer, kViv);
    return int32_t(prefix & 0xffffff00u);
}

/* 0x439360: car eax's .viv path into edx. */
nfs::u32 carVivPath(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, carVivPathOf(app, ts, e.esp, int32_t(e.eax), e.edx));
}

/* ---- 0x467bc0, 0x467be0, 0x467c00, 0x467c20 ---- */

namespace
{

/* sprintf from a function that pushes ebp alone. */
Result printed(App* app, ThreadState& ts, u32 esp, u32 eax, std::initializer_list<u32> words)
{
    // (the ebp the code pushes here is not written: nothing reads it)
    // sprintf's buffer and format, then the words after them.
    const u32* const word = words.begin();
    guest::Words rest({});
    for (const u32* w = word + 2; w != words.end(); ++w)
        rest.word[rest.count++] = *w;
    u32 r;
    if (!taken(ts, callCore<kSprintf, watcom::clib::sprintf>(app, ts, esp - 4, word[0], word[1], rest), r))
        return {};
    return int32_t(r);
}

}  // namespace

Result genericVivPathOf(App* app, ThreadState& ts, u32 esp, u32 buffer)
{
    return printed(app, ts, esp, buffer, {buffer, kGenericViv, kGenericFolder});
}

Result genericModelNameOf(App* app, ThreadState& ts, u32 esp, int32_t n)
{
    if (!printed(app, ts, esp, u32(n), {kGenericName, kModelNumber, u32(n)}))
        return {};
    return int32_t(kGenericName);
}

Result trackVivPathOf(App* app, ThreadState& ts, u32 esp, u32 buffer)
{
    return printed(app, ts, esp, buffer, {buffer, kTrackViv, kTrackFolder});
}

Result trackModelNameOf(App* app, ThreadState& ts, u32 esp, int32_t n)
{
    if (!printed(app, ts, esp, u32(n), {kTrackName, kModelNumber, u32(n)}))
        return {};
    return int32_t(kTrackName);
}

/* 0x467bc0: the generic car's .viv path into eax. */
nfs::u32 genericVivPath(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, genericVivPathOf(app, ts, e.esp, e.eax));
}

/* 0x467be0: the generic model's name of number eax. */
nfs::u32 genericModelName(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, genericModelNameOf(app, ts, e.esp, int32_t(e.eax)));
}

/* 0x467c00: the track's .viv path into eax. */
nfs::u32 trackVivPath(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, trackVivPathOf(app, ts, e.esp, e.eax));
}

/* 0x467c20: the track model's name of number eax. */
nfs::u32 trackModelName(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, trackModelNameOf(app, ts, e.esp, int32_t(e.eax)));
}

/* ---- 0x465eb0 ---- */

Result flare(App* app, ThreadState& ts, u32 esp, u32 x, u32 y, u32 z, uint32_t colour, u32 size, u32 zSlot)
{
    const u32 ebp = esp - 0x18, frame = ebp - 0x48;
    const u32 roomAt = ebp - 0x28, vertexAt = ebp - 0x48;
    setWord(app, roomAt, 0);
    if (kFlareTexture(app) == 0)
        return 0;
    if (!fcom(Float(0.0), bitsF(size)).cf)
        return 0;
    if (!fcom(Float(0.0), bitsF(z)).cf)
        return 0;
    if ((z & 0x7fffffff) == 0)
    {
        z = 0x37800080;
        if (zSlot != 0)
            setWord(app, zSlot, z);
    }
    // The point projected, on the stack as the code has it (its texture's
    // place there as the stack holds it, which the quads copy).
    {
        Vertex v = load<Vertex>(app, vertexAt);
        project(app, ts, x, y, z, v);
        store<Vertex>(app, vertexAt, v);
    }
    Vertex& v = record<Vertex>(app, vertexAt);
    v.code = clipCode(app, v);
    if (v.code != 0)
        return 0;
    v.colour = colour;
    u32 r;
    if (!taken(ts, callCore<kRoom, frontend::rendd::arenaTake>(app, ts, frame, roomAt, 0xa0u), r))
        return {};
    if (r == 0)
        return 0;
    const u32 room = word(app, roomAt);
    const Float w = Float(record<Vertex>(app, vertexAt).w);
    const Float sx = fp::mul(Float(kScaleX(app)), w);
    const Float sy = fp::mul(Float(kScaleY(app)), w);
    const u32 q0 = room + 0x20, q1 = room + 0x40, q2 = room + 0x60, q3 = room + 0x80;
    const Float distance = fp::div(bitsF(x), bitsF(z));
    app->getMemory<uint16_t>(room + 4) = 0;
    setWord(app, room, 0);
    app->getMemory<uint16_t>(room + 6) = 1;
    setWord(app, room + 0x18, kFlareTag);
    setWord(app, room + 8, q0);
    setWord(app, room + 0xc, q1);
    setWord(app, room + 0x10, q2);
    setWord(app, room + 0x14, q3);
    setWord(app, room + 0x1c, bitsOf(record<Vertex>(app, vertexAt).z));
    const Float size_ = bitsF(size);
    const Float across = fp::mul(sx, size_), down = fp::mul(size_, sy);
    const float d = float(distance);
    copyWords(app, q3, vertexAt, 8);
    const Float turned = fp::mul(fp::add(Float(1.0), Float(d)), F(app, kQuarterA));
    copyWords(app, q2, q3, 8);
    const float acrossF = float(across), downF = float(down);
    copyWords(app, q1, q2, 8);
    const u32 angle = bitsOf(float(turned));
    copyWords(app, q0, q1, 8);
    const std::optional<double> sine = turnValue<kSine, frontend::simd::sineValue>(
        app, ts, frame, {bitsOf(record<Vertex>(app, vertexAt).z), q0}, angle);
    if (!sine)
        return {};
    const float s = float(fp::mul(Float(*sine), Float(acrossF)));
    const std::optional<double> cosine = turnValue<kCosine, frontend::simd::cosineValue>(
        app, ts, frame, {bitsOf(record<Vertex>(app, vertexAt).z), q0}, angle);
    if (!cosine)
        return {};
    const Float c = fp::mul(Float(*cosine), Float(downF));
    {
        const Float x0 = F(app, q0), y0 = F(app, q0 + 4);
        putF(app, q0, fp::sub(x0, Float(s)));
        putF(app, q0 + 4, fp::add(y0, c));
    }
    {
        const Float y1 = F(app, q1 + 4), x1 = F(app, q1);
        const Float nx = fp::add(x1, c), ny = fp::add(y1, Float(s));
        putF(app, q1, nx);
        putF(app, q1 + 4, ny);
    }
    {
        const Float y2 = F(app, q2 + 4), x2 = F(app, q2);
        const Float nx = fp::add(x2, Float(s)), ny = fp::sub(y2, c);
        putF(app, q2, nx);
        putF(app, q2 + 4, ny);
    }
    {
        const Float y3 = F(app, q3 + 4), x3 = F(app, q3);
        const Float nx = fp::sub(x3, c), ny = fp::sub(y3, Float(s));
        putF(app, q3, nx);
        putF(app, q3 + 4, ny);
    }
    for (const u32 q : {q0, q1, q2, q3})
        markCode(app, q);
    return int32_t(room);
}

/* 0x465eb0 (stdcall 0x14): a flare at the point of the stack's three floats,
 * its colour and size after them. */
nfs::u32 flareQuad(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args,
             flare(app, ts, e.esp, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2), e.arg(app, 3), e.arg(app, 4),
                   e.esp + 0xc),
             0x14);
}

/* ---- 0x466330 ---- */

Result lightPlace(App* app, ThreadState& ts, u32 esp, u32 model, int32_t index, u32 out, u32 matrix,
                  const u32 offset[3])
{
    const u32 ebp = esp - 0xc, frame = ebp - 0x1c;
    if (index >= int32_t(word(app, model + 0x34)))
        return 0;
    const u32 from = ebp - 0x10, turned = ebp - 0x1c;
    copyWords(app, from, model + 0x38 + 12 * u32(index), 3);
    callCore<kRotate, vertices::turnVectors>(app, ts, frame, 1, from, matrix, turned);
    if (!ran(ts))
        return {};
    putF(app, out, fp::add(F(app, turned), bitsF(offset[0])));
    putF(app, out + 4, fp::add(F(app, turned + 4), bitsF(offset[1])));
    putF(app, out + 8, fp::add(F(app, turned + 8), bitsF(offset[2])));
    return 1;
}

/* 0x466330 (stdcall 0xc): light edx of model eax turned by the matrix at
 * ecx, moved by the stack's three floats, into ebx. */
nfs::u32 lightPlaced(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    const u32 offset[3] = {e.arg(app, 0), e.arg(app, 1), e.arg(app, 2)};
    return e.finish(ts, args, lightPlace(app, ts, e.esp, e.eax, int32_t(e.edx), e.ebx, e.ecx, offset), 0xc);
}

/* ---- 0x4663a0, 0x466580 ---- */

namespace
{

/* What both flares do once the light is placed and its facing known: the
 * flare at `place` in `colour`, `size` across, joined to the list at `pair`;
 * the list's pair kept at `list`, the joined one at `joined`. */
Result flareJoined(App* app, ThreadState& ts, u32 frame, u32 pair, u32 place, uint32_t colour, u32 size,
                   u32 list, u32 joined)
{
    const Result room = flare(app, ts, callee(frame, 5), word(app, place), word(app, place + 4),
                              word(app, place + 8), colour, size, frame - 0xc);
    if (!room || *room == 0)
        return room;
    if (!ran(ts, callCore<kListOf, frontend::rendd::listOfOne>(app, ts, frame, u32(*room), list)))
        return {};
    // The list joined into `joined`: the pair's two words first, the list's after them, on the stack.
    u32 r;
    if (!taken(ts,
               callCore<kListJoined, frontend::rendd::joinLists>(app, ts, frame - 4 * 4, joined, word(app, pair),
                                                                 word(app, pair + 4), word(app, list),
                                                                 word(app, list + 4)),
               r))
        return {};
    copyWords(app, pair, joined, 2);
    return int32_t(r);
}

/* The light placed and its direction from the eye made a unit long: false
 * when it is not there (`placed` 0) or the game stopped.  `afterUnit` what
 * the unit's call leaves in eax. */
Result lightFacing(App* app, ThreadState& ts, u32 frame, u32 ebp, u32 model, u32 matrix, u32 position,
                   int32_t index, u32& afterUnit)
{
    const u32 offset[3] = {word(app, position), word(app, position + 4), word(app, position + 8)};
    const u32 place = ebp - 0x54, toward = ebp - 0x48;
    const Result placed = lightPlace(app, ts, callee(frame, 3), model, index, place, matrix, offset);
    if (!placed || *placed == 0)
        return placed;
    callCore<kSubtract, vertices::offsetVectorList<false>>(app, ts, frame, 1, place, position, toward);
    if (!ran(ts))
        return {};
    callCore<kUnit, vertices::makeUnitVector>(app, ts, frame, toward, toward);
    if (!ran(ts))
        return {};
    afterUnit = toward;  // eax as the unit vector's code leaves it: the vector's address it was given
    return 1;
}

}  // namespace

Result headlightFlare(App* app, ThreadState& ts, u32 esp, u32 pair, u32 model, u32 matrix, u32 position,
                      int32_t index, u32 alpha)
{
    const u32 ebp = esp - 0xc, frame = ebp - 0x54;
    u32 afterUnit = 0;
    const Result facing = lightFacing(app, ts, frame, ebp, model, matrix, position, index, afterUnit);
    if (!facing || *facing == 0)
        return facing;
    // How far the light faces the eye, against the lights' threshold (a
    // brighter one for models of more than 12 parts).
    const float faced = float(-F(app, ebp - 0x40));
    u32 threshold = 0xbe4ccccd, scale = 0x3f555555;
    if (int32_t(word(app, model + 0xf8)) > 0xc)
    {
        scale = 0x3fa00000;
        threshold = 0x3e4ccccd;
    }
    const Compared facedCompared = fcom(Float(faced), bitsF(threshold));
    if (facedCompared.belowOrEqual())
        return int32_t(withStatus(facedCompared, model));
    const float strength = float(fp::mul(fp::sub(Float(faced), bitsF(threshold)), bitsF(scale)));
    const u32 least = fcom(F(app, kQuarterB), Float(strength)).belowOrEqual() ? bitsOf(strength) : 0x3e800000;
    const Float size = fp::mul(fp::mul(bitsF(least), D(app, kHeadA)), D(app, kHeadB));
    const double strong = double(Float(strength));
    const float sizeF = float(size);
    double brightness = 1.0;
    if (!fcom(Float(strength), D(app, kHeadNear)).above())
        brightness = double(fp::mul(Float(strong), D(app, kHeadFour)));
    const Float a = fp::mul(fp::mul(Float(brightness), F(app, kWhite)), F(app, kHeadAlpha));
    const int32_t opacity = flareAlpha(ts, a, float(a));
    const uint32_t colour = faded(uint32_t(opacity & 0xff) << 24 | 0xffffff, fadeOf(int32_t(alpha & 0xff)));
    return flareJoined(app, ts, frame, pair, ebp - 0x54, colour, bitsOf(sizeF), ebp - 0x3c, ebp - 0x24);
}

Result taillightFlare(App* app, ThreadState& ts, u32 esp, u32 pair, u32 model, u32 matrix, u32 position,
                      int32_t index, u32 alpha)
{
    const u32 ebp = esp - 0xc, frame = ebp - 0x54;
    u32 afterUnit = 0;
    const Result facing = lightFacing(app, ts, frame, ebp, model, matrix, position, index, afterUnit);
    if (!facing || *facing == 0)
        return facing;
    const Float faced = -F(app, ebp - 0x40);
    const float facedF = float(faced);
    const Compared facedCompared = fcom(faced, F(app, kTailFacing));
    if (facedCompared.belowOrEqual())
        return int32_t(withStatus(facedCompared, afterUnit));
    const Float over = fp::mul(fp::add(Float(facedF), D(app, kTailMinus)), D(app, kTailOver));
    const float overF = float(over);
    const double overD = double(over);
    double brightness = 1.0;
    if (!fcom(over, D(app, kTailNear)).above())
        brightness = double(fp::mul(Float(overD), D(app, kTailFour)));
    const float brightF = float(Float(brightness));
    const u32 least = fcom(F(app, kQuarterC), Float(overF)).belowOrEqual() ? bitsOf(overF) : 0x3e800000;
    const Float p = fp::mul(bitsF(least), D(app, kTailA));
    const Float q = fp::mul(Float(brightF), F(app, kTailScale));
    const Float size = fp::mul(p, D(app, kTailB));
    const Float a = fp::mul(q, F(app, kTailAlpha));
    const float sizeF = float(size);
    const int32_t opacity = flareAlpha(ts, a, float(a));
    const uint32_t colour = faded(uint32_t(opacity & 0xff) << 24 | 0xff0000, fadeOf(int32_t(alpha & 0xff)));
    return flareJoined(app, ts, frame, pair, ebp - 0x54, colour, bitsOf(sizeF), ebp - 0x34, ebp - 0x2c);
}

/* 0x4663a0 (stdcall 8): a headlight's flare -- the list's pair at eax, the
 * model at edx, the matrix at ebx, the place at ecx, the light's number and
 * the alpha on the stack. */
nfs::u32 headlightGlare(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args,
             headlightFlare(app, ts, e.esp, e.eax, e.edx, e.ebx, e.ecx, int32_t(e.arg(app, 0)), e.arg(app, 1)),
             8);
}

/* 0x466580 (stdcall 8): a taillight's, so. */
nfs::u32 taillightGlare(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args,
             taillightFlare(app, ts, e.esp, e.eax, e.edx, e.ebx, e.ecx, int32_t(e.arg(app, 0)), e.arg(app, 1)),
             8);
}

/* ---- 0x466740 ---- */

Result lightFlares(App* app, ThreadState& ts, u32 esp, u32 model, u32 matrix, u32 position, u32 alpha,
                   u32 out)
{
    const u32 ebp = esp - 8, frame = ebp - 0x18;
    const u32 pair = ebp - 0x18;
    if (!ran(ts, callCore<kListEmpty, frontend::rendd::emptyList>(app, ts, frame, pair)))
        return {};
    for (int32_t i = 0; i < int32_t(word(app, model + 0x34)); ++i)
    {
        if (app->getMemory<uint8_t>(model + 0xa04 + 64 * u32(i)) == kHeadlight
            && !headlightFlare(app, ts, callee(frame, 2), pair, model, matrix, position, i, alpha & 0xff))
            return {};
        if (app->getMemory<uint8_t>(model + 0xa04 + 64 * u32(i)) == kTaillight
            && !taillightFlare(app, ts, callee(frame, 2), pair, model, matrix, position, i, alpha & 0xff))
            return {};
    }
    copyWords(app, out, pair, 2);
    return int32_t(out);
}

/* 0x466740: the flares of model eax's lights (matrix edx, place ebx, alpha
 * cl) into the list's pair at esi.  esi is left at the frame, as the game's
 * code leaves it (its caller sets it again before it reads it). */
nfs::u32 objectLights(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    const Result r = lightFlares(app, ts, e.esp, e.eax, e.edx, e.ebx, e.ecx & 0xff, e.esi);
    const u32 result = e.finish(ts, args, r);
    if (r)
        args.esi = e.esp - 0x18;
    return result;
}

/* ---- 0x4667d0 ---- */

Result dragSpin(App* app, ThreadState& ts, u32 esp, u32 item, u32 edx)
{
    const u32 ebp = esp - 0xc, frame = ebp - 8;
    Object& o = record<Object>(app, item);
    const int32_t clock = kClock(app);
    if (clock < int32_t(uint32_t(o.mouseClock) + 4))
        return clock;
    o.mouseClock = clock;
    u32 key;
    if (!taken(ts, callCore<kMouseKey, frontend::glue::mouseKey>(app, ts, frame), key))
        return {};
    if (key != 0xd || o.loading != 0)
    {
        const uint8_t flags = o.flags;
        o.dragging = 0;
        o.flags = uint8_t(flags & 0xfb);
        return int32_t(key);
    }
    u32 eax = key;
    if (edx != 0 && o.dragging == 0)
    {
        o.dragging = 1;
        const uint16_t mouse = app->getMemory<uint16_t>(kMouseX.at);
        eax = (eax & 0xffff0000u) | mouse;
        const uint8_t flags = o.flags;
        o.spin[1] = fromBits(0);
        o.grabX = int16_t(mouse);
        o.flags = uint8_t(flags | 4);
    }
    if (o.dragging == 0)
        return int32_t(eax);
    const int32_t grabbed = o.grabX;
    const int32_t moved = int32_t(uint32_t(kMouseX(app)) - uint32_t(grabbed));
    const Float turn = fp::mul(Float(moved), F(app, kDragTurn));
    const Float spin = fp::mul(turn, F(app, kDragSpin));
    const uint16_t mouse = app->getMemory<uint16_t>(kMouseX.at);
    eax = (uint32_t(grabbed) & 0xffff0000u) | mouse;
    const Float was = Float(o.turn[1]);
    o.grabX = int16_t(mouse);
    const Float now = fp::add(turn, was);
    const Float damped = fp::mul(spin, D(app, kDragDamp));
    o.turn[1] = float(now);
    o.spin[1] = float(damped);
    return int32_t(eax);
}

/* 0x4667d0: the object at eax dragged round by the mouse (edx: it may take
 * hold now). */
nfs::u32 objectDragged(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, dragSpin(app, ts, e.esp, e.eax, e.edx));
}

/* ---- 0x4668b0 ---- */

Result screenBounds(App* app, ThreadState& ts, u32 esp, u32 item, u32 rect)
{
    const u32 B = esp - 0x14 - 0x82, frame = esp - 0x14 - 0x240;
    Object& o = record<Object>(app, item);
    const u32 model = o.model;
    const u32 centre = B + 0x46, matrix = B + 2, corners = B - 0x5e, turned = B - 0xbe;
    copyWords(app, centre, item + 0x5c, 3);
    if ((word(app, centre + 8) & 0x7fffffff) == 0)
        setWord(app, centre + 8, 0x37800080);
    Vertex v{};
    project(app, ts, word(app, centre), word(app, centre + 4), word(app, centre + 8), v);
    v.code = clipCode(app, v);
    uint32_t maxX = bitsOf(v.x), minX = maxX, maxY = bitsOf(v.y), minY = maxY;
    if (!rotated<kXRotation, ea::rotateAboutX>(app, ts, frame, matrix, bitsOf(o.turn[0]))
        || (callCore<kYRotation, frontend::simd::rotateY>(app, ts, frame, matrix, bitsOf(o.turn[1]), matrix), !ran(ts))
        || (callCore<kZRotation, frontend::simd::rotateZ>(app, ts, frame, matrix, bitsOf(o.turn[2]), matrix), !ran(ts)))
        return {};
    // The model's box: its half sizes, each corner's signs.
    const u32 hx = word(app, model + 0x28), hy = word(app, model + 0x2c), hz = word(app, model + 0x30);
    const u32 nx = bitsOf(float(-Float(fromBits(hx)))), ny = bitsOf(float(-Float(fromBits(hy))));
    const u32 nz = bitsOf(float(-Float(fromBits(hz))));
    const u32 box[24] = {nx, hy, hz, hx, hy, hz, hx, hy, nz, nx, hy, nz,
                           nx, ny, hz, hx, ny, hz, hx, ny, nz, nx, ny, nz};
    for (u32 i = 0; i < 24; ++i)
        setWord(app, corners + 4 * i, box[i]);
    callCore<kTransform, vertices::transformPointList>(app, ts, frame, 8, corners, matrix,
                                                       centre, turned);
    if (!ran(ts))
        return {};
    Vertex corner[8];
    for (u32 i = 0; i < 8; ++i)
    {
        const u32 p = turned + 12 * i;
        if ((word(app, p + 8) & 0x7fffffff) == 0)
            setWord(app, p + 8, 0x37800080);
        corner[i] = Vertex{};
        project(app, ts, word(app, p), word(app, p + 4), word(app, p + 8), corner[i]);
        corner[i].code = clipCode(app, corner[i]);
    }
    for (u32 i = 0; i < 8; ++i)
    {
        const float x = corner[i].x, y = corner[i].y;
        if (fcom(Float(x), Float(fromBits(maxX))).above())
            maxX = bitsOf(x);
        if (fcom(Float(x), Float(fromBits(minX))).cf)
            minX = bitsOf(x);
        if (fcom(Float(y), Float(fromBits(maxY))).above())
            maxY = bitsOf(y);
        if (fcom(Float(y), Float(fromBits(minY))).cf)
            minY = bitsOf(y);
    }
    const Float top = Float(fromBits(minY)), left = Float(fromBits(minX));
    const Float right = Float(fromBits(maxX)), bottom = Float(fromBits(maxY));
    const int32_t y0 = chopped<int32_t>(ts, top), x0 = chopped<int32_t>(ts, left);
    const int32_t width = chopped<int32_t>(ts, fp::sub(right, left));
    const int32_t height = chopped<int32_t>(ts, fp::sub(bottom, top));
    app->getMemory<uint16_t>(rect) = uint16_t(y0);
    app->getMemory<uint16_t>(rect + 2) = uint16_t(x0);
    app->getMemory<uint16_t>(rect + 4) = uint16_t(width);
    app->getMemory<uint16_t>(rect + 6) = uint16_t(height);
    return height;
}

/* 0x4668b0: the object at eax's rectangle on the screen into edx. */
nfs::u32 objectBounds(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, screenBounds(app, ts, e.esp, e.eax, e.edx));
}

/* ---- 0x4d79b0 ---- */

Result drawLogoOf(App* app, ThreadState& ts, u32 esp, u32 texture, int32_t x, int32_t y, int32_t width, int32_t height,
                  uint32_t colour)
{
    const u32 ebp = esp - 0xc, frame = ebp - 0xc;
    const float depth = kScreenDepth(app), w = kScreenW(app);
    for (int32_t corner = 0; corner < 4; ++corner)
    {
        kQuad(app, corner).z = viaX87(depth);
        kQuad(app, corner).w = viaX87(w);
    }
    for (int32_t corner = 0; corner < 4; ++corner)
        kQuad(app, corner).colour = colour;
    kQuad(app, 3).x = float(Float(x));
    kQuad(app, 1).y = float(Float(y));
    const Float left = Float(kQuad(app, 3).x), top = Float(kQuad(app, 1).y);
    kQuad(app, 0).x = float(left);
    kQuad(app, 0).y = float(top);
    kQuad(app, 2).x = float(fp::add(left, Float(width)));
    const Float bottom = fp::add(Float(height), top);
    const float right = viaX87(kQuad(app, 2).x);
    kQuad(app, 3).y = float(bottom);
    kQuad(app, 1).x = right;
    kQuad(app, 2).y = float(bottom);
    callCore<kSetRenderState, graphics::sendState>(app, ts, frame, 1u, texture);
    if (!ran(ts))
        return {};
    for (u32 corner = 0; corner < 4; ++corner)
        markCode(app, kQuadAt + 0x20 * corner);
    u32 r;
    if (!callPointer(app, ts, frame, kClipQuad, {kQuadAt, kQuadAt + 0x20, kQuadAt + 0x40, kQuadAt + 0x60}, {}, r))
        return {};
    return int32_t(r);
}

/* 0x4d79b0 (stdcall 8): the logo's texture eax over the rectangle at edx,
 * ebx, ecx wide, the stack's height high, in the stack's colour.  ebx, ecx
 * and edx are left as the clipped quad's draw leaves them, as the game's
 * code does: render::loadingScreen hands edx on. */
nfs::u32 drawLogo(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    const Result r = drawLogoOf(app, ts, e.esp, e.eax, int32_t(e.edx), int32_t(e.ebx), int32_t(e.ecx),
                                int32_t(e.arg(app, 0)), e.arg(app, 1));
    const u32 ebx = args.ebx, ecx = args.ecx, edx = args.edx;
    const u32 result = e.finish(ts, args, r, 8);
    if (r)
    {
        args.ebx = ebx;
        args.ecx = ecx;
        args.edx = edx;
    }
    return result;
}

/* ---- 0x467ac0 ---- */

Result resetLoaderOf(App* app, ThreadState& ts, u32 esp, u32 loader)
{
    const u32 frame = esp - 0x18;
    if (loader == 0)
        return 0;
    u32 r;
    if (!taken(ts, callCore<kReadClose, frontend::wata::asyncSetFile>(app, ts, frame, kNul), r))
        return {};
    Loader& l = record<Loader>(app, loader);
    for (int i = 0; i < 4; ++i)
        l.reads[i] = 0;
    l.viv[0] = 0;
    l.modelName[0] = 0;
    l.textureName[0] = 0;
    l.shapesName[0] = 0;
    l.state = 0;
    l.waiting = 0;
    if (!taken(ts, callCore<kMemset, watcom::clib::memset>(app, ts, frame,
                                                            loader + 0x174, 0u, 0x10u),
               r))
        return {};
    for (const u32 at : {0x188u, 0x18cu, 0x190u, 0x194u})
    {
        const u32 block = word(app, loader + at);
        if (block == 0)
            continue;
        if (!taken(ts, callCore<kFree, ea::purgeBlock>(app, ts, frame, block), r))
            return {};
        setWord(app, loader + at, 0);
    }
    return int32_t(r);
}

/* 0x467ac0: the loader at eax started afresh: its reads stopped, its names
 * and state cleared, its blocks let go. */
nfs::u32 resetLoader(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, resetLoaderOf(app, ts, e.esp, e.eax));
}

/* ---- 0x467c40 ---- */

Result chooseFilesOf(App* app, ThreadState& ts, u32 esp, u32 item)
{
    const u32 frame = esp - 0x18;
    // (the six registers the code pushes here are not written: nothing reads them)
    if (item == 0)
        return 0;
    Object& o = record<Object>(app, item);
    if (o.loader == 0)
        return 0;
    if (!resetLoaderOf(app, ts, callee(frame, 0), o.loader))
        return {};
    const int32_t kind = o.kind;
    u32 prefix = 0;
    bool identified = false;
    int16_t id = 0;
    // A car's: its .viv path and its files' prefix.
    auto car = [&](int32_t number) -> bool {
        u32 r;
        if (!carVivPathOf(app, ts, callee(frame, 0), number, o.loader)
            || !taken(ts, callCore<kCarFilePrefix, load::prefixOf>(app, ts, frame,
                                                                    u32(number)),
                      r))
            return false;
        prefix = r;
        return true;
    };
    if (kind == 9 || kind == 10 || kind == kPlayer1 || kind == kPlayer2)
    {
        const u32& global = kind == 9 ? *&app->getMemory<u32>(kCarShown9.at) : kind == 10 ? *&app->getMemory<u32>(kCarShown10.at)
                            : gameSettings(app).players[kind == kPlayer1 ? 0 : 1].car;
        if (!car(int32_t(global)))
            return {};
        identified = true;
        id = int16_t(global);
    }
    else if (kind == kTrackKind)
    {
        if (!trackVivPathOf(app, ts, callee(frame, 0), o.loader))
            return {};
        const Result name = trackModelNameOf(app, ts, callee(frame, 0), std::int32_t(gameSettings(app).track));
        if (!name)
            return {};
        prefix = u32(*name);
        identified = true;
        id = int16_t(gameSettings(app).track);
    }
    else if (kind == 4 || kind == 6)
    {
        if (!genericVivPathOf(app, ts, callee(frame, 0), o.loader))
            return {};
        const Result name = genericModelNameOf(app, ts, callee(frame, 0), o.number);
        if (!name)
            return {};
        prefix = u32(*name);
    }
    else if (kind == kRandom3)
    {
        // A car at random, not the one drawn last if there are more than two.
        u32 count;
        if (!taken(ts, callCore<0x438bd0, load::carsListed>(app, ts, frame), count))
            return {};
        u32 chosen = 0;
        if (count != 0)
        {
            u32 r;
            if (!taken(ts, callCore<kRandom, port::nextRandom>(app, ts, frame), r))
                return {};
            chosen = r % count;
            if (int32_t(count) > 2)
                do
                {
                    if (!taken(ts, callCore<kRandom, port::nextRandom>(app, ts, frame), r))
                        return {};
                    chosen = r % count;
                } while (chosen == u32(kRandomCar(app)));
        }
        kRandomCar.set(app, int32_t(chosen));
        if (!car(int32_t(chosen)))
            return {};
        if (prefix == 0)
            return 0;
    }
    else if (kind == kSplit)
    {
        if (std::int32_t(gameSettings(app).opponentClass) < 4)
            return 0;
        if (!car(std::int32_t(gameSettings(app).opponentClass) - 4))
            return {};
        identified = true;
        id = int16_t(gameSettings(app).opponentClass);
    }
    else
    {
        return 0;
    }
    if (identified)
        o.loadingId = id;
    if (prefix == 0)
        return 0;
    u32 printedCount;
    if (!taken(ts, callCore<kSprintf, watcom::clib::sprintf>(app, ts, frame, o.loader + 0x100, kTwo,
                                                              guest::Words{prefix, kFce}),
               printedCount)
        || !taken(ts, callCore<kSprintf, watcom::clib::sprintf>(app, ts, frame, o.loader + 0x120,
                                                                 kTwo, guest::Words{prefix, kTga}),
                  printedCount)
        || !taken(ts, callCore<kSprintf, watcom::clib::sprintf>(app, ts, frame, o.loader + 0x140,
                                                                 kFsh, guest::Words({})),
                  printedCount))
        return {};
    return 1;
}

/* 0x467c40: the files of the car (or track, or generic car) the object at
 * eax shows: the .viv's path, the model's, texture's and shapes' names. */
nfs::u32 chooseFiles(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, chooseFilesOf(app, ts, e.esp, e.eax));
}

/* ---- 0x467ea0, 0x467ef0 ---- */

namespace
{

/* The .viv's header's three words byte-swapped where they lie. */
void swapHead(App* app, u32 at)
{
    for (const u32 off : {0u, 0xcu, 8u})
    {
        const u32 v = word(app, at + off);
        setWord(app, at + off, (v << 24) | ((v & 0xff00) << 8) | ((v & 0xff0000) >> 8) | (v >> 24));
    }
}

}  // namespace

Result readPreheadOf(App* app, ThreadState& ts, u32 esp, u32 loader)
{
    const u32 frame = esp - 0x14;
    if (loader == 0)
        return 0;
        u32 r;
    if (!taken(ts, callCore<kMemset, watcom::clib::memset>(app, ts, frame, loader + 0x174, 0u, 0x10u),
               r)
        || !taken(ts, callCore<kReadClose, frontend::wata::asyncSetFile>(app, ts, frame, loader), r)
        || !taken(ts, callCore<kRead, frontend::wata::asyncRead>(app, ts, frame, 0u, loader + 0x174, 0x10u), r))
        return {};
    record<Loader>(app, loader).reads[0] = r;
    return r != 0 ? 1 : 0;
}

/* 0x467ea0: the .viv's first 16 bytes asked for. */
nfs::u32 readPrehead(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, readPreheadOf(app, ts, e.esp, e.eax));
}

Result readHeadOf(App* app, ThreadState& ts, u32 esp, u32 loader)
{
    const u32 frame = esp - 0x14;
    if (loader == 0)
        return 0;
    Loader& l = record<Loader>(app, loader);
    if (l.reads[0] == 0)
        return 0;
    swapHead(app, loader + 0x174);
        u32 r;
    if (!taken(ts, callCore<kAllocate, ea::reserved>(app, ts, frame, kEmpty, l.head[3] + 0x20, 0x10u),
               r))
        return {};
    l.vivHead = r;
    if (r == 0)
        return 0;
        if (!taken(ts, callCore<kReadClose, frontend::wata::asyncSetFile>(app, ts, frame, loader), r)
        || !taken(ts, callCore<kRead, frontend::wata::asyncRead>(app, ts, frame, 0u, l.vivHead, l.head[3] + 0x20),
                  r))
        return {};
    l.reads[1] = r;
    return r != 0 ? 1 : 0;
}

/* 0x467ef0: the .viv's header asked for, as long as its first bytes say. */
nfs::u32 readHead(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, readHeadOf(app, ts, e.esp, e.eax));
}

/* ---- 0x468010, 0x468170, 0x4682c0 ---- */

Result readPartOf(App* app, ThreadState& ts, u32 esp, u32 loader, int part)
{
    // The model's code pushes edi too.
    const u32 ebp = esp - (part == 0 ? 0x18 : 0x14), frame = ebp - 8;
    if (loader == 0)
        return 0;
    Loader& l = record<Loader>(app, loader);
    if (l.vivHead == 0)
        return 0;
    swapHead(app, l.vivHead);
    const u32 offsetAt = ebp - 8, sizeAt = ebp - 4;
        if (!callPointer(app, ts, frame, kVivEntry, {l.vivHead, loader + 0x100 + 0x20 * u32(part), offsetAt, sizeAt}))
        return {};
    if (word(app, offsetAt) == 0 || word(app, sizeAt) == 0)
        return 0;
    u32 r;
    if (!taken(ts, callCore<kAllocate, ea::reserved>(app, ts, frame, kEmpty,
                                                      word(app, sizeAt), 0x10u),
               r))
        return {};
    setWord(app, loader + 0x18c + 4 * u32(part), r);
    if (part == 0)
        l.modelSize = word(app, sizeAt);
    if (r == 0)
        return 0;
        if (!taken(ts, callCore<kReadClose, frontend::wata::asyncSetFile>(app, ts, frame, loader), r)
        || !taken(ts,
                  callCore<kRead, frontend::wata::asyncRead>(app, ts, frame, word(app, offsetAt),
                                                             word(app, loader + 0x18c + 4 * u32(part)),
                                                             word(app, sizeAt)),
                  r))
        return {};
    l.reads[2 + part] = r;
    return r != 0 ? 1 : 0;
}

/* 0x468010, 0x468170, 0x4682c0: the model's, texture's and shapes' parts of
 * the .viv found in its header and asked for. */
nfs::u32 readModel(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, readPartOf(app, ts, e.esp, e.eax, 0));
}

nfs::u32 readTexture(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, readPartOf(app, ts, e.esp, e.eax, 1));
}

nfs::u32 readShapes(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, readPartOf(app, ts, e.esp, e.eax, 2));
}

/* ---- 0x468410 ---- */

Result takeModelOf(App* app, ThreadState& ts, u32 esp, u32 item)
{
    const u32 frame = esp - 0x18;
    if (item == 0)
        return 0;
    Object& o = record<Object>(app, item);
    if (o.loader == 0)
        return 0;
    u32 r;
    // The logo among the shapes read, made the logo's texture.
    const u32 shapes = record<Loader>(app, o.loader).shapesData;
    if (shapes != 0 && kLogoTexture(app) != 0)
    {
        u32 shape;
        if (!callPointer(app, ts, frame, kLocateShape, {shapes, kLogo}, {}, shape))
            return {};
        if (shape != 0)
        {
            const u32 place = word(app, shape + 0xc);
            kLogoX.set(app, int32_t(place << 20) >> 20);
                        kLogoY.set(app, int32_t(place << 4) >> 20);
            callCore<kShapeIntoTexture, graphics::shapeIntoTextureOf>(app, ts, frame, u32(kLogoTexture(app)), shape,
                                                                      0x100u, u32(kLogoPalette(app)));
            if (ts.terminate)
                return {};
            o.logo = 1;
        }
        else
            o.logo = 0;
    }
    else
        o.logo = 0;
    Loader& l = record<Loader>(app, o.loader);
    if (l.modelData == 0 || l.textureData == 0)
        return 0;
    if (o.modelData != 0)
    {
                if (!taken(ts, callCore<kFreeModel, frontend::rendc::modelFree>(app, ts, frame, o.modelData), r))
            return {};
        o.modelData = 0;
        o.model = 0;
    }
    if (o.shadow != 0)
    {
        if (!taken(ts, callCore<kFreeModel, frontend::rendc::modelFree>(app, ts, frame, o.shadow), r))
            return {};
        o.shadow = 0;
    }
    if (o.textureData != 0)
    {
        if (!taken(ts, callCore<kFree, ea::purgeBlock>(app, ts, frame, o.textureData), r))
            return {};
        o.textureData = 0;
    }
    o.modelData = record<Loader>(app, o.loader).modelData;
    o.textureData = record<Loader>(app, o.loader).textureData;
    record<Loader>(app, o.loader).modelData = 0;
    record<Loader>(app, o.loader).textureData = 0;
    return 1;
}

/* 0x468410: the model read taken over by the object at eax, the old one let
 * go; the logo found among the shapes. */
nfs::u32 takeModel(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, takeModelOf(app, ts, e.esp, e.eax));
}

/* ---- 0x468580, 0x4685e0 ---- */

namespace
{

/* The object's alpha a step on (16 a tick of the clock), held to 0..0xff. */
void fadeStep(App* app, Object& o, int32_t step)
{
    const int32_t clock = kClock(app);
    if (clock == o.fadeClock)
        return;
    int32_t a = int32_t(uint32_t(o.alpha) + uint32_t(step));
    if (a < 0)
        a = 0;
    else if (a > 0xff)
        a = 0xff;
    o.fadeClock = clock;
    o.alpha = a;
}

}  // namespace

Result fadeInOf(App* app, ThreadState&, u32, u32 item)
{
    Object& o = record<Object>(app, item);
    fadeStep(app, o, 0x10);
    return o.alpha == 0xff ? 1 : 0;
}

Result fadeOutOf(App* app, ThreadState&, u32, u32 item)
{
    Object& o = record<Object>(app, item);
    fadeStep(app, o, -0x10);
    return o.alpha == 0 ? 1 : 0;
}

/* 0x468580, 0x4685e0: the object at eax faded in or out a step: 1 once it is
 * all there or gone. */
nfs::u32 fadeIn(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, fadeInOf(app, ts, e.esp, e.eax));
}

nfs::u32 fadeOut(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, fadeOutOf(app, ts, e.esp, e.eax));
}

/* ---- 0x468670 ---- */

Result wantedChangedOf(App* app, ThreadState&, u32, u32 item)
{
    Object& o = record<Object>(app, item);
    u32 eax = u32(kClock(app));
    if (int32_t(uint32_t(o.checkClock) + 0x80) < int32_t(eax))
    {
        o.checkClock = int32_t(eax);
        eax = ((eax & 0xffff0000u) | uint16_t(o.kind)) - 1;
        if ((eax & 0xffff) <= 9)
        {
            eax &= 0xffff;
            // Each kind's car (or track) against the one shown: another one
            // wanted, unless one is loading already.
            GameSettings& settings = gameSettings(app);
            const u32* const globals[10] = {&settings.players[0].car, &settings.players[1].car, nullptr, nullptr, &settings.track, nullptr, nullptr,
                                              &settings.opponentClass, &app->getMemory<u32>(0x664788), &app->getMemory<u32>(0x664784)};
            const u32* const global = globals[eax];
            if (global != nullptr)
            {
                eax = u32(int32_t(o.shownId));
                if (int32_t(eax) != int32_t(*global) && o.loading == 0)
                    o.wanted = 1;
            }
            else if (eax == 2 && o.loading == 0 && o.wanted == 0)
                o.wanted = 1;
        }
    }
    if (o.automatic != 0 && o.wanted != 0 && o.loading == 0)
    {
        o.loading = 1;
        eax = o.loader;
        o.wanted = 0;
        record<Loader>(app, o.loader).state = 1;
    }
    return int32_t(eax);
}

/* 0x468670: whether the object at eax wants another car loaded (its kind's
 * car changed), checked every 0x80 ticks; the load started if it does. */
nfs::u32 wantedChanged(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, wantedChangedOf(app, ts, e.esp, e.eax));
}

/* ---- 0x468860 ---- */

Result runLoaderOf(App* app, ThreadState& ts, u32 esp, u32 item)
{
    const u32 frame = esp - 0x14;
    u32 r;
    if (!taken(ts, callCore<0x44a660, frontend::loop::shownNow>(app, ts, frame, item), r))
        return {};
    if (r == 0)
        return 0;
    if (item == 0)
        return 0;
    Object& o = record<Object>(app, item);
    if (o.loader == 0)
        return 0;
    if (!wantedChangedOf(app, ts, callee(frame, 0), item))
        return {};
    if (o.loading == 0)
        return 1;
    if (!callCore<kRunTasks, stream::runDueTasks>(app, ts, frame, callee(frame, 0), 0u)
        || ts.terminate)
        return {};
    int32_t result = 1;
    const u32 loader = o.loader;
    const int32_t state = record<Loader>(app, loader).state;
        // A step failed: traced, the loader started afresh, the load given up.
    auto failed = [&](u32 text) -> bool {
        // The hook takes nothing: the text the code pushes for it is not read.
        callCore<kTrace, raceload::noHook>(app, ts, frame);
        if (!ran(ts))
            return false;
        result = 0;
        if (!resetLoaderOf(app, ts, callee(frame, 0), o.loader))
            return false;
        o.loading = 0;
        return true;
    };
    // A step asked for: its read waited on next.
    auto asked = [&](int32_t waiting) {
        record<Loader>(app, o.loader).state = 0;
        record<Loader>(app, o.loader).waiting = waiting;
    };
    switch (state)
    {
    case 1:
    {
        const Result chosen = chooseFilesOf(app, ts, callee(frame, 0), item);
        if (!chosen)
            return {};
        if (*chosen != 0)
            record<Loader>(app, o.loader).state = 2;
        else if (!failed(kStartFailed))
            return {};
        break;
    }
    case 2:
    case 3:
    case 4:
    case 5:
    {
        static const u32 texts[4] = {kPreheadFailed, kHeadFailed, kModelFailed, kTextureFailed};
        const Result read = state == 2 ? readPreheadOf(app, ts, callee(frame, 0), loader)
                          : state == 3 ? readHeadOf(app, ts, callee(frame, 0), loader)
                                       : readPartOf(app, ts, callee(frame, 0), loader, state - 4);
        if (!read)
            return {};
        if (*read != 0)
            asked(state - 1);
        else if (!failed(texts[state - 2]))
            return {};
        break;
    }
    case 6:
    {
        const Result read = readPartOf(app, ts, callee(frame, 0), loader, 2);
        if (!read)
            return {};
        if (*read != 0)
            asked(5);
        else
        {
            record<Loader>(app, o.loader).waiting = 0;
            record<Loader>(app, o.loader).state = 7;
        }
        break;
    }
    case 7:
    case 8:
    case 9:
        record<Loader>(app, loader).state = 0;
        record<Loader>(app, o.loader).waiting = state - 1;
        break;
    default:
        break;
    }
    const int32_t waiting = record<Loader>(app, o.loader).waiting;
    switch (waiting)
    {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    {
        // A read waited on: once it is done, the step after it.
        if (!callCore<kRunTasks, stream::runDueTasks>(app, ts, frame, callee(frame, 0),
                                                      0u)
            || ts.terminate)
            return {};
        u32 done;
                if (!taken(ts,
                   callCore<kReadDone, frontend::wata::asyncReadStatus>(app, ts, frame,
                                                                        record<Loader>(app, o.loader).reads[waiting - 1]),
                   done))
            return {};
        if (done == 0)
            break;
        record<Loader>(app, o.loader).waiting = 0;
        int32_t next = waiting + 2;
        if (waiting == 4)
            next = o.kind == kPlayer1 || o.kind == kPlayer2 || o.kind == kSplit ? 6 : 7;
        else if (waiting == 5)
            next = 7;
        record<Loader>(app, o.loader).state = next;
        break;
    }
    case 6:
    {
        const Result gone = fadeOutOf(app, ts, callee(frame, 0), item);
        if (*gone != 0)
        {
            record<Loader>(app, o.loader).waiting = 0;
            record<Loader>(app, o.loader).state = 8;
        }
        break;
    }
    case 7:
    {
        const Result taken = takeModelOf(app, ts, callee(frame, 0), item);
        if (!taken)
            return {};
        if (*taken != 0)
        {
            record<Loader>(app, o.loader).waiting = 0;
            record<Loader>(app, o.loader).state = 9;
            if (!makeTextureOf(app, ts, callee(frame, 0), item))
                return {};
        }
        break;
    }
    case 8:
    {
        const Result there = fadeInOf(app, ts, callee(frame, 0), item);
        if (*there != 0)
        {
            record<Loader>(app, o.loader).waiting = 0;
            record<Loader>(app, o.loader).state = 0;
            o.shownId = o.loadingId;
            if (!objectUp(app, ts, callee(frame, 0), item))
                return {};
            o.loading = 0;
        }
        break;
    }
    default:
        break;
    }
    return result;
}

/* 0x468860: the loader of the object at eax run a step: 1, or 0 when the
 * screen is not the object's or a step failed. */
nfs::u32 runLoader(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, runLoaderOf(app, ts, e.esp, e.eax));
}

/* ---- 0x468da0 ---- */

Result carColourOf(App* app, ThreadState& ts, u32 esp, u32 item, u32 out)
{
    const u32 ebp = esp - 0x14, frame = ebp - 0x14;
    const u32 rgb = ebp - 0x14;
    copyWords(app, rgb, kNoColour, 4);
    Object& o = record<Object>(app, item);
    const u32 index = u32(uint16_t(o.kind) - 1) & 0xffff;
    if (index > 9)
        return 0;
    switch (index)
    {
    case 8:
    case 9:
        // The generic car's and the cars shown: the model's first colour.
        for (u32 i = 0; i < 4; ++i)
            setWord(app, out + 4 * i, word(app, record<Object>(app, item).model + 0x800 + 4 * i));
        return 1;
    case 2:
    case 7:
    {
        // A random car's: one of the model's colours at random.
        const u32 model = o.model;
        if (word(app, model + 0x7fc) == 0)
            return 0;
        u32 r;
                if (!taken(ts, callCore<kRandom, port::nextRandom>(app, ts, frame), r))
            return {};
        const u32 chosen = (r % word(app, model + 0x7fc)) << 4;
        for (u32 i = 0; i < 4; ++i)
            setWord(app, out + 4 * i, word(app, record<Object>(app, item).model + chosen + 0x800 + 4 * i));
        return 1;
    }
    case 0:
    case 1:
    {
        // The players': the colour they chose, as hue, saturation and value.
        const auto car = [&] { return int32_t(gameSettings(app).players[index == 0 ? 0 : 1].car); };
        u32 r;
        if (!taken(ts, callCore<kCarColourIndex, fields::chosenColour>(app, ts, frame,
                                                                        callee(frame, 0), u32(car()), int32_t(index)),
                   r))
            return {};
                if (!callPointer(app, ts, frame, kCarColourRgb, {u32(car()), r, index, rgb},
                      {ebp - 0x10, ebp - 0xc, ebp - 8})
            || !taken(ts, callCore<0x49c880, load::colourToHsv>(app, ts, frame, rgb, out), r))
            return {};
        return 1;
    }
    default:
        return 0;
    }
}

/* 0x468da0: the colour of the object at eax's car into edx (4 words). */
nfs::u32 carColour(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, carColourOf(app, ts, e.esp, e.eax, e.edx));
}

/* ---- 0x468f00 ---- */

Result makeTextureOf(App* app, ThreadState& ts, u32 esp, u32 item)
{
    const u32 ebp = esp - 0x18, frame = ebp - 0x18;
    const u32 colour = ebp - 0x18;
    copyWords(app, colour, kNoColour2, 4);
    if (item == 0)
        return 0;
    Object& o = record<Object>(app, item);
    if (o.modelData == 0 || o.textureData == 0 || o.texture == 0)
        return int32_t(item);
    u32 r;
    if (o.model == 0)
    {
        if (!callPointer(app, ts, frame, kModelReady, {o.modelData, item}, {}, r))
            return {};
        o.model = r;
    }
    if (o.model == 0)
        return int32_t(item);
    // The object pushed back far enough for the model to be in front of the eye.
    const float back = float(fp::add(Float(1.0), F(app, o.model + 0x30)));
    if (fcom(Float(fromBits(o.position[2])), Float(back)).cf)
        o.position[2] = bitsOf(back);
    const Result coloured = carColourOf(app, ts, callee(frame, 0), item, colour);
    if (!coloured)
        return {};
    u32 loadedTga;
    if (*coloured != 0)
    {
        if (!taken(ts, callCore<0x49d320, load::tgaLoaded>(app, ts, frame - 12,
                                                         o.model, o.textureData, 0x100, o.texture, colour, 1, 0),
                   loadedTga))
            return {};
    }
    else if (!taken(ts, callCore<0x49d320, load::tgaLoaded>(app, ts, frame - 12,
                                                             o.model, o.textureData, 0x100, o.texture, 0, 1, 0),
                    loadedTga))
        return {};
    if (o.shadow != 0)
        return int32_t(item);
    // The model's copy for the shadow.
    if (!taken(ts, callCore<kAllocate, ea::reserved>(app, ts, frame, kEmpty,
                                                      record<Loader>(app, o.loader).modelSize, 0x10u),
               r))
        return {};
    o.shadow = r;
    if (!callCore<kMemmove, ea::moveMemory>(app, ts, frame, o.shadow,
                                            o.model, record<Loader>(app, o.loader).modelSize)
        || ts.terminate
        || !callPointer(app, ts, frame, kModelReady, {o.shadow, item}, {}, r))
        return {};
    o.shadow = r;
    return int32_t(r);
}

/* 0x468f00: the object at eax's model made ready, its texture made in its
 * car's colour, its shadow's copy made. */
nfs::u32 makeTexture(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, makeTextureOf(app, ts, e.esp, e.eax));
}

/* ---- 0x466cf0 ---- */

Result drawObject(App* app, ThreadState& ts, u32 esp, u32 item, u32 edx)
{
    const u32 P = esp - 0x14, B = P - 0x82, frame = P - 0xd0;
    u32 r;
    const u32 light = B + 0x1e, matrix = B - 0x2a, identity = B - 0x4e, mirrored = B - 6, place = B + 0x2a;
    const u32 drawn = B + 0x36, shadows = B + 0x3e, flares = B + 0x46;
    copyWords(app, light, kLightDirection, 3);
        for (const u32 list : {drawn, shadows, flares})
    {
        if (!ran(ts, callCore<kListEmpty, frontend::rendd::emptyList>(app, ts, frame, list)))
            return {};
    }
    if (!taken(ts, callCore<0x44a660, frontend::loop::shownNow>(app, ts, frame, item), r))
        return {};
    if (r == 0)
        return 0;
    Object& o = record<Object>(app, item);
    if (o.kind == kSplit && std::int32_t(gameSettings(app).opponentClass) < 4)
    {
        // The split screen's object with fewer than 4 players: no car.
        o.shownId = -1;
        if (o.modelData != 0)
        {
                        if (!ran(ts, callCore<kFreeModel, frontend::rendc::modelFree>(app, ts, frame, o.modelData)))
                return {};
            o.modelData = 0;
            o.model = 0;
        }
        return 0;
    }
    if (!runLoaderOf(app, ts, callee(frame, 0), item))
        return {};
    if (o.model == 0)
        return 0;
    if (o.textureStale != 0)
    {
        o.textureStale = 0;
        if (!makeTextureOf(app, ts, callee(frame, 0), item))
            return {};
    }
    if (o.logo != 0 && (o.kind == kPlayer1 || o.kind == kPlayer2 || o.kind == kSplit))
    {
                if (kLogoTexture(app) != 0
            && !drawLogoOf(app, ts, callee(frame, 2), kLogoTexture(app), kLogoX(app), kLogoY(app), 0x100, 0x100,
                           uint32_t(o.alpha) << 24 | 0xffffff))
            return {};
    }
    // The clock: the time since the object was last drawn.
    const int32_t clock = kClock(app);
    if (o.lastClock == 0)
        o.lastClock = clock;
    const int32_t elapsed = int32_t(uint32_t(clock) - uint32_t(o.lastClock));
    o.lastClock = clock;
    const float dt = float(Float(elapsed));
        if (kNoMouse(app) == 0 && !dragSpin(app, ts, callee(frame, 0), item, edx))
        return {};
    // A spin faster than the object's own turn slowed, a step a tick.
    const float own = float(fp::mul(F(app, item + 0x54), F(app, kPerDegree)));
    const float ownSize = fcom(Float(0.0), Float(own)).belowOrEqual() ? own : float(-Float(own));
    const float spinSize = fcom(Float(0.0), Float(o.spin[1])).belowOrEqual() ? o.spin[1] : float(-Float(o.spin[1]));
    if (fcom(Float(spinSize), Float(ownSize)).above())
        for (int32_t n = 0; n < chopped<int32_t>(ts, Float(dt)); ++n)
            o.spin[1] = float(fp::mul(Float(o.spin[1]), F(app, kSlowing)));
    if ((kMenuFlags(app) & 0x40) != 0)
    {
        // Over a race: the screen cleared behind it, its turn the pause's.
        const u32 colour = kPaused(app) != 0 && kPauseShown(app) == 0 ? 0xff00ff00 : 0xff000000;
                u32 filled;
        if (!taken(ts, callCore<kFilledRectangle, draw2d::filled>(app, ts, frame - 4, callee(frame, 1), 0, 0, 0x27f, 0x1df,
                                                                  colour),
                   filled))
            return {};
        o.turn[1] = fromBits(kPauseTurn(app));
    }
    else if (o.dragging == 0)
    {
        const Float t = Float(dt);
        const Float a = fp::mul(t, Float(o.spin[0]));
        const Float b = fp::mul(t, Float(o.spin[1]));
        const Float c = fp::mul(t, Float(o.spin[2]));
        const Float k = D(app, kPerMillisecond);
        const Float x = fp::add(fp::mul(a, k), Float(o.turn[0]));
        const Float y = fp::add(fp::mul(b, k), Float(o.turn[1]));
        const Float z = fp::add(fp::mul(k, c), Float(o.turn[2]));
        o.turn[0] = float(x);
        o.turn[1] = float(y);
        o.turn[2] = float(z);
    }
    // The object's matrix, the renderer's states, the light.
    if (!rotated<kXRotation, ea::rotateAboutX>(app, ts, frame, matrix, bitsOf(o.turn[0]))
        || (callCore<kYRotation, frontend::simd::rotateY>(app, ts, frame, matrix, bitsOf(o.turn[1]), matrix), !ran(ts))
        || (callCore<kZRotation, frontend::simd::rotateZ>(app, ts, frame, matrix, bitsOf(o.turn[2]), matrix), !ran(ts))
        || !ran(ts, callCore<kStatesForHud, collide::hudStates>(app, ts, frame))
        || !ran(ts, callCore<kSetRenderState, graphics::sendState>(app, ts, frame, 0x15u, 0u))
        || (callCore<kSunLighting, decompiled::sunLightingReset>(app, ts, frame), !ran(ts))
                || !ran(ts, callCore<kLightFrom, frontend::rendc::modelSetShading>(app, ts, frame, 1u))
        || (callCore<kUnit, vertices::makeUnitVector>(app, ts, frame, light, light), !ran(ts))
        || !ran(ts, callCore<kLightDirectionSet, frontend::rendc::modelSetLight>(app, ts, frame, light))
        || !ran(ts, callCore<kAmbient, frontend::rendc::modelSetColour>(app, ts, frame, 0xffafafafu)))
        return {};
    {
        const uint32_t white = faded(0xffffffff, fadeOf(o.alpha));
        const uint32_t grey = faded(0x7f7f7f7f, fadeOf(o.alpha));
                callCore<kGlareColours, render::glareColoursSet>(app, ts, frame, grey, white);
        if (!ran(ts))
            return {};
    }
    const int32_t kind = o.kind;
    setWord(app, kShadowPass, 0);
    if ((kind >= 1 && kind <= 3) || (kind >= kSplit && kind <= 10))
    {
        copyWords(app, identity, kIdentity, 9);
        copyWords(app, place, item + 0x5c, 3);
        copyWords(app, mirrored, matrix, 9);
        for (const u32 sign : {B + 9, B + 0x11, B + 0xd})
            app->getMemory<uint8_t>(sign) ^= 0x80;
        copyWords(app, place, item + 0x5c, 3);
        // The shadow's place: the object's, below it by the model's height.
        putF(app, place + 4, fp::sub(F(app, place + 4), fp::mul(F(app, o.model + 0x2c), D(app, kTwice))));
                if (!ran(ts, callCore<kListStart, frontend::rendc::objectsBegin>(app, ts, frame))
            || !ran(ts, callCore<kLightAlpha, frontend::rendc::modelSetAlphas>(app, ts, frame, uint32_t(o.alpha) & 0xff,
                                                                               uint32_t(o.alpha >> 1) & 0xff)))
            return {};
        if ((kMenuFlags(app) & 0x40) == 0 || (kPaused(app) != 0 && kPauseShown(app) == 0))
        {
            // The car: its body, its glass, its four wheels.
            if (!drawModel(app, ts, frame, o.model, 0, 0, o.model + 0xfc, matrix, item + 0x5c, kLightsTail(app)))
                return {};
            if (!drawModel(app, ts, frame, o.model, 0xc, 0, o.model + 0x18c, matrix, item + 0x5c, kLightsTail(app)))
                return {};
            if (!drawModel(app, ts, frame, o.model, 1, identity, o.model + 0x108, matrix, item + 0x5c, 0))
                return {};
            if (!drawModel(app, ts, frame, o.model, 2, identity, o.model + 0x114, matrix, item + 0x5c, 0))
                return {};
            if (!drawModel(app, ts, frame, o.model, 3, identity, o.model + 0x120, matrix, item + 0x5c, 0))
                return {};
            if (!drawModel(app, ts, frame, o.model, 4, identity, o.model + 0x12c, matrix, item + 0x5c, 0))
                return {};
        }
                if (!ran(ts, callCore<kListTaken, frontend::rendc::objectsEnd>(app, ts, frame, drawn))
            || !ran(ts, callCore<kListStart, frontend::rendc::objectsBegin>(app, ts, frame)))
            return {};
        if (((kMenuFlags(app) & 0x40) == 0 || kPauseShown(app) != 0) && o.shadowed != 0
            && (kDriverFlags(app) & 6) == 0)
        {
            // The shadow: the copy of the model mirrored below the car, dim.
            const int32_t scaled = int32_t(uint32_t(o.alpha) * 155u);
            const int32_t dim = scaled < 0 ? int32_t(uint32_t(scaled) + 0xff) >> 8 : scaled >> 8;
                        if (!ran(ts, callCore<kLightAlpha, frontend::rendc::modelSetAlphas>(app, ts, frame, uint32_t(dim) & 0xff,
                                                                                 uint32_t(dim >> 2) & 0xff))
                || !ran(ts, callCore<kAmbient, frontend::rendc::modelSetColour>(app, ts, frame, 0xff4f5f7fu)))
                return {};
            app->getMemory<uint8_t>(B + 0x25) ^= 0x80;
            if (!ran(ts, callCore<kLightDirectionSet, frontend::rendc::modelSetLight>(app, ts, frame, light)))
                return {};
            const u32 tail = kLightsTail(app);
            app->getMemory<uint8_t>(kShadowPass) ^= 1;
            if (!drawModel(app, ts, frame, o.shadow, 0, 0, o.model + 0xfc, mirrored, place, tail))
                return {};
            if (!drawModel(app, ts, frame, o.shadow, 1, identity, o.model + 0x108, mirrored, place, 0))
                return {};
            if (!drawModel(app, ts, frame, o.shadow, 2, identity, o.model + 0x114, mirrored, place, 0))
                return {};
            if (!drawModel(app, ts, frame, o.shadow, 3, identity, o.model + 0x120, mirrored, place, 0))
                return {};
                        if (!drawModel(app, ts, frame, o.shadow, 4, identity, o.model + 0x12c, mirrored, place, 0))
                return {};
            app->getMemory<uint8_t>(kShadowPass) ^= 1;
            if (!ran(ts, callCore<kLightAlpha, frontend::rendc::modelSetAlphas>(app, ts, frame, 0xffu, 0x7fu))
                || !ran(ts, callCore<kListTaken, frontend::rendc::objectsEnd>(app, ts, frame, shadows)))
                return {};
        }
        if ((kMenuFlags(app) & 0x40) == 0 || (kPaused(app) == 0 && kPauseShown(app) == 0))
        {
                        if (!lightFlares(app, ts, callee(frame, 0), o.model, matrix, item + 0x5c, uint32_t(o.alpha) & 0xff,
                             flares))
                return {};
        }
        if (kAfterState(app) == 0)
        {
                        const u32 s0 = word(app, shadows), s1 = word(app, shadows + 4);
            if (!ran(ts, callCore<kListSorted, frontend::rende::polygonsSorted>(app, ts, frame - 8, shadows, s0, s1)))
                return {};
            const u32 d0 = word(app, drawn), d1 = word(app, drawn + 4);
            if (!ran(ts, callCore<kListSorted, frontend::rende::polygonsSorted>(app, ts, frame - 8, drawn, d0, d1)))
                return {};
        }
        if (!ran(ts, callCore<kListJoined, frontend::rendd::joinLists>(app, ts, frame - 4 * 4, drawn, word(app, shadows),
                                                                        word(app, shadows + 4), word(app, drawn),
                                                                        word(app, drawn + 4))))
            return {};
    }
    else
    {
        // The generic car, the track: the model alone, its glass after it.
                if (!ran(ts, callCore<kLightAlpha, frontend::rendc::modelSetAlphas>(app, ts, frame, uint32_t(o.alpha) & 0xff,
                                                                             uint32_t(o.alpha >> 1) & 0xff))
            || !ran(ts, callCore<kListStart, frontend::rendc::objectsBegin>(app, ts, frame)))
            return {};
        if (!drawModel(app, ts, frame, o.model, 1, 0, o.model + 0x108, matrix, item + 0x5c, 0))
            return {};
        if (!ran(ts, callCore<kListTaken, frontend::rendc::objectsEnd>(app, ts, frame, shadows))
            || !ran(ts, callCore<kListStart, frontend::rendc::objectsBegin>(app, ts, frame)))
            return {};
        if (!drawModel(app, ts, frame, o.model, 0, 0, o.model + 0xfc, matrix, item + 0x5c, 0))
            return {};
        if (!ran(ts, callCore<kListTaken, frontend::rendc::objectsEnd>(app, ts, frame, drawn)))
            return {};
        if (!ran(ts, callCore<kListJoined, frontend::rendd::joinLists>(app, ts, frame - 4 * 4, drawn, word(app, shadows),
                                                                        word(app, shadows + 4), word(app, drawn),
                                                                        word(app, drawn + 4))))
            return {};
    }
    if (!callPointer(app, ts, frame, kDrawRecords, {word(app, drawn)})
        || !ran(ts, callCore<kSetRenderState, graphics::sendState>(app, ts, frame, 4u, 0u))
        || !callPointer(app, ts, frame, kDrawRecords, {word(app, flares)})
        || !ran(ts, callCore<kSetRenderState, graphics::sendState>(app, ts, frame, 4u,
                                                                    kAfterState(app)))
                || !ran(ts, callCore<kFrameDone, frontend::rendd::arenaRewind>(app, ts, frame))
        || !ran(ts, callCore<kStatesForScreens, collide::screenStates>(app, ts, frame)))
        return {};
    return 0;
}

/* 0x466cf0: the object at eax drawn -- its car loaded and faded, turned by
 * the clock or the mouse, lit, its shadow and its lights' flares (edx: the
 * mouse may take hold of it). */
nfs::u32 objectDraw(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, drawObject(app, ts, e.esp, e.eax, e.edx));
}

/* ---- 0x4675b0 ---- */

Result objectKey(App* app, ThreadState& ts, u32 esp, u32 item)
{
    // (the ebp the code pushes here is not written: nothing reads it)
    u32 r;
    if (!taken(ts, callCore<0x44a660, frontend::loop::shownNow>(app, ts, esp - 4, item), r))
        return {};
    return 0;
}

/* 0x4675b0: a key on the object at eax: never taken.  edx is left 8, as the
 * game's code leaves it (the key's dispatch at 0x44a36a hands it up). */
nfs::u32 objectKeyed(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    const Result r = objectKey(app, ts, e.esp, e.eax);
    const u32 result = e.finish(ts, args, r);
    if (r)
        args.edx = 8;
    return result;
}

/* ---- 0x4675d0 ---- */

Result objectPass(App* app, ThreadState& ts, u32 esp, u32 item)
{
    const u32 frame = esp - 0xc;
    u32 r;
    if (!taken(ts, callCore<0x44a660, frontend::loop::shownNow>(app, ts, frame, item), r))
        return {};
    if (r == 0)
        return 0;
    if (kNoMouse(app) != 0)
        return 0;
    const Object& o = record<Object>(app, item);
    if (o.kind == kSplit && std::int32_t(gameSettings(app).opponentClass) < 4)
        return 0;
    if (o.loading != 0 || o.wanted != 0)
        return 4;
    if (o.dragging != 0)
        return 2;
    return 1;
}

/* 0x4675d0: what the object at eax is to the mouse passing over it: 4
 * loading, 2 held, 1 there, 0 not. */
nfs::u32 objectPassed(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, objectPass(app, ts, e.esp, e.eax));
}

/* ---- 0x467650 ---- */

Result objectUp(App* app, ThreadState& ts, u32 esp, u32 item)
{
    const u32 frame = esp - 0x10;
    u32 r;
    if (!taken(ts, callCore<0x44a660, frontend::loop::shownNow>(app, ts, frame, item), r))
        return {};
    if (r == 0)
        return 0;
    Object& o = record<Object>(app, item);
    if (o.kind == kSplit && std::int32_t(gameSettings(app).opponentClass) < 4)
    {
        o.rect[1] = 0;
        o.rect[2] = 0;
        o.rect[3] = 0;
        const uint8_t flags = uint8_t(o.flags | 1);
        o.rect[0] = 0;
        o.flags = flags;
        return int32_t((r & 0xffff00ffu) | u32(flags) << 8);
    }
    if (o.model != 0 && o.unused84 != 0)
    {
        const Result bounds = screenBounds(app, ts, callee(frame, 0), item, item + 0x1a);
        if (!bounds)
            return {};
        o.flags &= 0xfe;
        return bounds;
    }
    o.rect[1] = 0;
    o.rect[2] = 0;
    o.rect[3] = 0;
    const uint8_t flags = uint8_t(o.flags | 1);
    o.rect[0] = 0;
    o.flags = flags;
    return int32_t(r);
}

/* 0x467650: the object at eax's rectangle on the screen made afresh. */
nfs::u32 objectReleased(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, objectUp(app, ts, e.esp, e.eax));
}

/* ---- 0x467700, 0x467930 ---- */

namespace
{

/* What showing and hiding both let go: the loader, the models, the texture. */
Result letGo(App* app, ThreadState& ts, u32 frame, u32 item, u32& r, bool hidden)
{
    Object& o = record<Object>(app, item);
    if (o.loader != 0)
    {
        if (!resetLoaderOf(app, ts, callee(frame, 0), o.loader))
            return {};
        if (!taken(ts, callCore<kFree, ea::purgeBlock>(app, ts, frame, o.loader), r))
            return {};
        o.loader = 0;
    }
    if (o.modelData != 0)
    {
                if (!taken(ts, callCore<kFreeModel, frontend::rendc::modelFree>(app, ts, frame, o.modelData), r))
            return {};
        o.modelData = 0;
        o.model = 0;
    }
    if (o.shadow != 0)
    {
        if (!taken(ts, callCore<kFreeModel, frontend::rendc::modelFree>(app, ts, frame, o.shadow), r))
            return {};
        o.shadow = 0;
    }
    r = o.textureData;
    if (o.textureData != 0)
    {
        if (!taken(ts, callCore<kFree, ea::purgeBlock>(app, ts, frame, o.textureData), r))
            return {};
        o.textureData = 0;
    }
    return 1;
}

}  // namespace

Result objectShown(App* app, ThreadState& ts, u32 esp, u32 item)
{
    const u32 frame = esp - 0x18;
        u32 r;
    if (!taken(ts, callCore<0x44a660, frontend::loop::shownNow>(app, ts, frame, item), r))
        return {};
    if (r == 0)
        return 0;
    Object& o = record<Object>(app, item);
    o.texture = 0;
    o.alpha = 0xff;
    o.textureStale = 0;
    o.loading = 0;
    o.wanted = 0;
    o.automatic = 1;
    o.dragging = 0;
    o.rect[0] = -1;
    o.rect[1] = -1;
    o.rect[2] = 0;
    o.rect[3] = 0;
    o.shownId = -1;
    o.checkClock = 0;
    o.loadingId = -1;
    const uint8_t flags = uint8_t(o.flags | 1);
    r = (r & 0xffff00ffu) | u32(flags) << 8;
    o.logo = 0;
    o.flags = flags;
    if (!letGo(app, ts, frame, item, r, false))
        return {};
    // A loader afresh, a texture of the object's own, the first car loaded
    // as far as the screen lets it.
    if (!taken(ts, callCore<kAllocate, ea::reserved>(app, ts, frame, kEmpty, 0x1a0u,
                                                      0x10u),
               r))
        return {};
    o.loader = r;
    if (r != 0)
    {
        if (!taken(ts, callCore<kAllocTexture, graphics::allocatedTexture>(app, ts, frame,
                                                                            0x100u, 0x100u),
                   r))
            return {};
        o.texture = r;
        if (r != 0)
        {
            if (!taken(ts, callCore<kMemset, watcom::clib::memset>(app, ts, frame, o.loader,
                                                                    0u, 0x1a0u),
                       r))
                return {};
            o.wanted = 1;
            for (;;)
            {
                const Result ran = runLoaderOf(app, ts, callee(frame, 0), item);
                if (!ran)
                    return {};
                r = u32(*ran);
                if (o.loading == 0 || r == 0)
                    break;
                r = o.loader;
                if (record<Loader>(app, o.loader).waiting == 8)
                    break;
            }
        }
    }
    // Its turn from its angles in degrees, less half a turn; its spins.
    const Float k = D(app, kDegrees);
    const Float a = fp::mul(Float(o.angles[0]), k), b = fp::mul(Float(o.angles[1]), k);
    const Float c = fp::mul(Float(o.angles[2]), k), d = fp::mul(Float(o.spins[0]), k);
    const Float e = fp::mul(Float(o.spins[1]), k), f = fp::mul(k, Float(o.spins[2]));
    const uint8_t menuFlags = kMenuFlags(app);
    o.spin[0] = float(d);
    o.spin[1] = float(e);
    o.spin[2] = float(f);
    const Float half = D(app, kHalfBack);
    const Float x = fp::add(a, half), y = fp::add(b, half), z = fp::add(half, c);
    o.turn[1] = float(y);
    o.turn[2] = float(z);
    o.turn[0] = float(x);
    if ((menuFlags & 0x40) != 0)
    {
        // Over a race: still, and up where the pause's screens want it.
        if (o.kind != kTrackKind)
            o.position[1] = 0x3f3d70a4;
        o.spin[1] = fromBits(0);
        o.spin[2] = fromBits(0);
        o.spin[0] = fromBits(0);
    }
    o.mouseClock = 0;
    o.fadeClock = 0;
    o.lastClock = 0;
    return int32_t(r);
}

/* 0x467700: the object at eax shown: its state afresh, its loader and
 * texture made, its first car loaded as far as it goes. */
nfs::u32 objectOpened(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, objectShown(app, ts, e.esp, e.eax));
}

Result objectHidden(App* app, ThreadState& ts, u32 esp, u32 item)
{
    const u32 frame = esp - 0x18;
    u32 r;
    if (!taken(ts, callCore<0x44a660, frontend::loop::shownNow>(app, ts, frame, item), r))
        return {};
    if (r == 0)
        return 0;
    Object& o = record<Object>(app, item);
    const uint8_t flags = uint8_t(o.flags & 0xfb);
    o.dragging = 0;
    r = (r & 0xffff00ffu) | u32(flags) << 8;
    o.flags = flags;
    if (!letGo(app, ts, frame, item, r, true))
        return {};
    o.model = 0;
    o.texture = 0;
    return int32_t(r);
}

/* 0x467930: the object at eax hidden: what it holds let go. */
nfs::u32 objectClosed(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, objectHidden(app, ts, e.esp, e.eax));
}

}  // namespace obj3d
}  // namespace port
}  // namespace nfs3hp
