#ifndef NFS3HP_THRASH_H
#define NFS3HP_THRASH_H

/* THRASH: the interface between nfs3.exe and its renderer DLLs -- voodoo2a,
 * softtria and the Direct3D one -- 34 stdcall exports the game finds with
 * GetProcAddress (THRASH_opendll, native_game.cpp).  The port's driver is
 * voodoo2a, every export of it the port's own, in port_thrash.cpp.
 *
 * Each name is checked against what voodoo2a's generated code does with the
 * number; only the numbers voodoo2a knows are here, and where a number means
 * something different to voodoo2a than to the Direct3D driver, voodoo2a's
 * meaning is the one given.
 *
 * The exports, voodoo2a's address and the argument bytes:
 *
 *   THRASH_about         @0  0xa83c60  native   the descriptor (below)
 *   THRASH_clearwindow   @0  0xa847b0  native
 *   THRASH_clip          @16 0xa848b0  native   x0, y0, x1, y1
 *   THRASH_drawline      @8  0xa86240  native   a, b
 *   THRASH_drawlinemesh  @12 0xa86350  native   count, vertices, indices
 *   THRASH_drawlinestrip @8  0xa86490  native   count, vertices
 *   THRASH_drawpoint     @4  0xa86650  native   vertex
 *   THRASH_drawpointmesh @12 0xa866e0  native   count, vertices, indices
 *   THRASH_drawpointstrip @8 0xa86790  native   count, vertices
 *   THRASH_drawquad      @16 0xa85300  native   a, b, c, d
 *   THRASH_drawquadmesh  @12 0xa85520  native   count, vertices, indices
 *   THRASH_drawtri       @12 0xa85770  native   a, b, c
 *   THRASH_drawtrifan    @8  0xa86000  native   count, vertices
 *   THRASH_drawtrimesh   @12 0xa85900  native   count, vertices, indices
 *   THRASH_drawtristrip  @8  0xa85ac0  native   count, vertices
 *   THRASH_flushwindow   @0  0xa847d0  native
 *   THRASH_idle          @0  0xa84810  native
 *   THRASH_init          @0  0xa83f50  native
 *   THRASH_is            @0  0xa83df0  native
 *   THRASH_lockwindow    @0  0xa850f0  native   the lock (below)
 *   THRASH_pageflip      @0  0xa847e0  native
 *   THRASH_readrect      @20 0xa85220  native   x, y, width, height, pixels
 *   THRASH_restore       @0  0xa84100  native
 *   THRASH_selectdisplay @4  0xa83f10  native   board
 *   THRASH_setstate      @8  0xa84990  native   state, value (below)
 *   THRASH_settexture    @4  0xa846d0  native   texture
 *   THRASH_setvideomode  @12 0xa843d0  native   mode, colour buffers, aux buffers
 *   THRASH_sync          @4  0xa84830  native   type
 *   THRASH_talloc        @20 0xa844b0  native   width, height, format, palette, state
 *   THRASH_treset        @0  0xa84690  native
 *   THRASH_tupdate       @12 0xa84630  native   texture, pixels, palette
 *   THRASH_unlockwindow  @4  0xa851d0  native   the lock
 *   THRASH_window        @4  0xa84780  native   buffer, from 1
 *   THRASH_writerect     @20 0xa85290  native   x, y, width, height, pixels
 *
 * Seven of them the game does not call: drawlinestrip, the three drawpoint
 * ones, drawtristrip, is and writerect.
 *
 * The game keeps a pointer to each in its data, 0x9ef900 on (kThrash*,
 * kExports below); native code calls them through it, guest::driver
 * (thrash_calls.h). */

#include <x86.h>

namespace nfs3hp
{
namespace thrash
{

/* The game's pointers to the exports, THRASH_opendll's (native_game.cpp). */
constexpr x86::reg32 kThrashDrawPointMesh = 0x9ef900;
constexpr x86::reg32 kThrashDrawPointStrip = 0x9ef904;
constexpr x86::reg32 kThrashDrawLineStrip = 0x9ef908;
constexpr x86::reg32 kThrashDrawLineMesh = 0x9ef90c;
constexpr x86::reg32 kThrashDrawPoint = 0x9ef910;
constexpr x86::reg32 kThrashWriteRect = 0x9ef914;
constexpr x86::reg32 kThrashTReset = 0x9ef918;
constexpr x86::reg32 kThrashDrawTriStrip = 0x9ef91c;
constexpr x86::reg32 kThrashReadRect = 0x9ef920;
constexpr x86::reg32 kThrashDrawTriMesh = 0x9ef924;
constexpr x86::reg32 kThrashDrawLine = 0x9ef928;
constexpr x86::reg32 kThrashClip = 0x9ef92c;
constexpr x86::reg32 kThrashDrawQuadMesh = 0x9ef930;
constexpr x86::reg32 kThrashSetTexture = 0x9ef934;
constexpr x86::reg32 kThrashWindow = 0x9ef938;
constexpr x86::reg32 kThrashIdle = 0x9ef93c;
constexpr x86::reg32 kThrashPageFlip = 0x9ef940;
constexpr x86::reg32 kThrashSelectDisplay = 0x9ef944;
constexpr x86::reg32 kThrashSync = 0x9ef948;
constexpr x86::reg32 kThrashDrawQuad = 0x9ef94c;
constexpr x86::reg32 kThrashFlushWindow = 0x9ef950;
constexpr x86::reg32 kThrashLockWindow = 0x9ef954;
constexpr x86::reg32 kThrashSetVideoMode = 0x9ef958;
constexpr x86::reg32 kThrashUnlockWindow = 0x9ef95c;
constexpr x86::reg32 kThrashDrawTriFan = 0x9ef960;
constexpr x86::reg32 kThrashTUpdate = 0x9ef964;
constexpr x86::reg32 kThrashTAlloc = 0x9ef968;
constexpr x86::reg32 kThrashSetState = 0x9ef96c;
constexpr x86::reg32 kThrashClearWindow = 0x9ef970;
constexpr x86::reg32 kThrashDrawTri = 0x9ef974;
constexpr x86::reg32 kThrashInit = 0x9ef978;
constexpr x86::reg32 kThrashRestore = 0x9ef97c;
constexpr x86::reg32 kThrashAbout = 0x9ef980;
constexpr x86::reg32 kThrashIs = 0x9ef984;

/* Each pointer with its export: voodoo2a's address of it (the list above)
 * and the bytes of its words. */
struct Export
{
    x86::reg32 slot, voodoo2a, bytes;
};

constexpr Export kExports[] = {
    {kThrashDrawPointMesh, 0xa866e0, 12}, {kThrashDrawPointStrip, 0xa86790, 8}, {kThrashDrawLineStrip, 0xa86490, 8},
    {kThrashDrawLineMesh, 0xa86350, 12},  {kThrashDrawPoint, 0xa86650, 4},       {kThrashWriteRect, 0xa85290, 20},
    {kThrashTReset, 0xa84690, 0},         {kThrashDrawTriStrip, 0xa85ac0, 8},    {kThrashReadRect, 0xa85220, 20},
    {kThrashDrawTriMesh, 0xa85900, 12},   {kThrashDrawLine, 0xa86240, 8},        {kThrashClip, 0xa848b0, 16},
    {kThrashDrawQuadMesh, 0xa85520, 12},  {kThrashSetTexture, 0xa846d0, 4},      {kThrashWindow, 0xa84780, 4},
    {kThrashIdle, 0xa84810, 0},           {kThrashPageFlip, 0xa847e0, 0},        {kThrashSelectDisplay, 0xa83f10, 4},
    {kThrashSync, 0xa84830, 4},           {kThrashDrawQuad, 0xa85300, 16},       {kThrashFlushWindow, 0xa847d0, 0},
    {kThrashLockWindow, 0xa850f0, 0},     {kThrashSetVideoMode, 0xa843d0, 12},   {kThrashUnlockWindow, 0xa851d0, 4},
    {kThrashDrawTriFan, 0xa86000, 8},     {kThrashTUpdate, 0xa84630, 12},        {kThrashTAlloc, 0xa844b0, 20},
    {kThrashSetState, 0xa84990, 8},       {kThrashClearWindow, 0xa847b0, 0},     {kThrashDrawTri, 0xa85770, 12},
    {kThrashInit, 0xa83f50, 0},           {kThrashRestore, 0xa84100, 0},         {kThrashAbout, 0xa83c60, 0},
    {kThrashIs, 0xa83df0, 0},
};

/* The export whose pointer is at `slot`; one of no address for another
 * slot. */
constexpr Export exportAt(x86::reg32 slot)
{
    for (const Export& e : kExports)
        if (e.slot == slot)
            return e;
    return {slot, 0, 0};
}

/* THRASH_setstate(state, value): 1 done, 0 a state or value the driver does
 * not take.  The state's name, then what voodoo2a does with it. */
constexpr x86::reg32 kStateTexture = 0x01;                // SELECT_TEXTURE: a texture record, or 0
constexpr x86::reg32 kStateCull = 0x02;                   // SELECT_CULL_STATE: kCull*
constexpr x86::reg32 kStateClearColour = 0x03;            // SELECT_MATERIAL, "background colour": what clearwindow clears to
constexpr x86::reg32 kStateDepth = 0x04;                  // SELECT_DEPTH_STATE: kDepth*
constexpr x86::reg32 kStateDither = 0x05;                 // SELECT_DITHER_STATE
constexpr x86::reg32 kStateShade = 0x06;                  // SELECT_SHADE_STATE: kShadeFlat, or the vertex colours
constexpr x86::reg32 kStateTextureFilter = 0x07;          // SELECT_TEXTURE_FILTER_STATE: kFilter*
constexpr x86::reg32 kStateLodBias = 0x08;                // SELECT_MIP_MAP_LOD_BIAS_STATE: a float's bits
constexpr x86::reg32 kStateAntiAlias = 0x09;              // SELECT_ANTI_ALIAS_STATE
constexpr x86::reg32 kStateTransparency = 0x0a;           // SELECT_ALPHA_BLEND_STATE, "transparency": kTransparency*
constexpr x86::reg32 kStateMipFilter = 0x0b;              // SELECT_TEXTURE_MIP_FILTER_STATE: kMip*
constexpr x86::reg32 kStateChromaKey = 0x0c;              // SELECT_CHROMATIC_COLOR: the colour keyed out
constexpr x86::reg32 kStateTextureAddress = 0x0d;         // SELECT_TEXTURE_ADDRESS_STATE: kAddress*
constexpr x86::reg32 kStateFogDensity = 0x0e;             // SELECT_FOG_DENSITY: fog by 1 / value, or off
constexpr x86::reg32 kStateFogColour = 0x0f;              // SELECT_FOG_COLOR
constexpr x86::reg32 kStateWindowMode = 0x12;             // SELECT_WINDOW_MODE_STATE: kept, [0xa91f44]
constexpr x86::reg32 kStateLambdas = 0x13;                // SELECT_LAMBDAS: the game's callbacks, kLambda* of a record, or none
constexpr x86::reg32 kStateFog = 0x15;                    // SELECT_FOG_STATE: kFog*
constexpr x86::reg32 kStateDepthBias = 0x18;              // SELECT_DEPTH_BIAS_STATE
constexpr x86::reg32 kStateWindow = 0x19;                 // SELECT_WINDOW: the game's window, [0xa91f08]
constexpr x86::reg32 kStateLogLambda = 0x1a;              // SELECT_LOG_LAMBDA, [0xa91f34]
constexpr x86::reg32 kStateExecuteLambda = 0x1b;          // SELECT_EXECUTE_LAMBDA, [0xa91f3c]
constexpr x86::reg32 kStateLockWindowLambda = 0x1c;       // SELECT_LOCK_WINDOW_LAMBDA, [0xa91f38]
constexpr x86::reg32 kStateGamma = 0x65;                  // SELECT_GAMMA_CONTROL_STATE_ALTERNATIVE: a float's bits
constexpr x86::reg32 kStateDepthBiasAlternative = 0x66;   // SELECT_DEPTH_BIAS_STATE_ALTERNATIVE: as kStateDepthBias
constexpr x86::reg32 kStateFlipRate = 0x67;               // SELECT_TOGGLE_STATE_ALTERNATIVE, "flip rate": the swap interval
constexpr x86::reg32 kStateBlend = 0x68;                  // SELECT_BLEND_STATE_ALTERNATIVE: kBlend*
constexpr x86::reg32 kStateFogAlphas = 0x69;              // SELECT_FOG_ALPHAS_ALTERNATIVE: the game's 64-entry fog table, or off
constexpr x86::reg32 kStateDepthWrite = 0x6a;             // SELECT_DEPTH_BUFFER_WRITE_STATE_ALTERNATIVE
constexpr x86::reg32 kState107 = 0x6b;                    // unnamed: kept, [0xa91f10]
constexpr x86::reg32 kState108 = 0x6c;                    // unnamed: Glide's sstControl, not on a Voodoo2

/* The values some of them take. */
constexpr x86::reg32 kCullNone = 0;                       // grCullMode's GR_CULL_DISABLE
constexpr x86::reg32 kCullCounterClockwise = 1;           // GR_CULL_NEGATIVE
constexpr x86::reg32 kCullClockwise = 2;                  // GR_CULL_POSITIVE

constexpr x86::reg32 kDepthOff = 0;
constexpr x86::reg32 kDepthZ = 1;                         // Glide's z buffer, compared less-or-equal
constexpr x86::reg32 kDepthW = 2;                         // Glide's w buffer, the same

constexpr x86::reg32 kShadeFlat = 0;                      // the constant colour; anything else, the vertices'

constexpr x86::reg32 kFilterPoint = 0;
constexpr x86::reg32 kFilterLinear = 1;

/* voodoo2a's meaning: the Direct3D driver takes 2 as blending on. */
constexpr x86::reg32 kTransparencyNone = 0;               // no chroma key, no alpha test
constexpr x86::reg32 kTransparencyChromaKey = 1;          // chroma key, and alpha test greater
constexpr x86::reg32 kTransparencyAlpha = 2;              // alpha test greater alone

constexpr x86::reg32 kMipNone = 0;
constexpr x86::reg32 kMipPoint = 1;
constexpr x86::reg32 kMipPointAdvanced = 2;
constexpr x86::reg32 kMipLinear = 3;                      // trilinear, voodoo2a's two passes of kMipPoint

constexpr x86::reg32 kAddressClamp = 0;                   // anything else wraps
constexpr x86::reg32 kAddressWrap = 1;

constexpr x86::reg32 kFogOff = 0;                         // the rest all turn on the table fog
constexpr x86::reg32 kFogOn = 1;
constexpr x86::reg32 kFogLinear = 2;
constexpr x86::reg32 kFogExp = 4;
constexpr x86::reg32 kFogExp2 = 8;

constexpr x86::reg32 kBlendSourceAlpha = 0;               // source alpha, one minus source alpha
constexpr x86::reg32 kBlendSourceAlphaOne = 1;            // source alpha, one: added
constexpr x86::reg32 kBlendZeroInverseSourceAlpha = 2;    // zero, one minus source alpha
constexpr x86::reg32 kBlendDestinationColourZero = 3;     // destination colour, zero: multiplied

/* kStateLambdas' record, the game's
 * stdcall functions; voodoo2a keeps four of them. */
constexpr x86::reg32 kLambdaAcquireWindow = 0x04;         // [0xa91f30]
constexpr x86::reg32 kLambdaExecute = 0x08;               // [0xa91f3c]; also kStateExecuteLambda
constexpr x86::reg32 kLambdaSelectInstance = 0x10;
constexpr x86::reg32 kLambdaLockWindow = 0x14;            // [0xa91f38], told 1 before a lock and 0 after
constexpr x86::reg32 kLambdaLog = 0x18;                   // [0xa91f34]; also kStateLogLambda
constexpr x86::reg32 kLambdasBytes = 0x20;

/* The game's vertex, kVertexBytes. */
constexpr x86::reg32 kVertexX = 0x00;                     // floats: screen x, y, z, then 1/w
constexpr x86::reg32 kVertexY = 0x04;
constexpr x86::reg32 kVertexZ = 0x08;
constexpr x86::reg32 kVertexRhw = 0x0c;
constexpr x86::reg32 kVertexColour = 0x10;                // bytes b, g, r, a
constexpr x86::reg32 kVertexSpecular = 0x14;              // voodoo2a does not read it
constexpr x86::reg32 kVertexU = 0x18;                     // floats
constexpr x86::reg32 kVertexV = 0x1c;
constexpr x86::reg32 kVertexBytes = 0x20;

/* THRASH_about's descriptor, 0x80 bytes;
 * voodoo2a's is at 0xa91f54. */
constexpr x86::reg32 kAboutSignature = 0x00;              // kSignature3dfx, "XFD3"
constexpr x86::reg32 kAboutSize = 0x04;                   // kAboutBytes
constexpr x86::reg32 kAboutVersion = 0x08;                // 0x68, 104
constexpr x86::reg32 kAboutCaps = 0x0c;                   // kCaps*
constexpr x86::reg32 kAboutMinTextureWidth = 0x10;
constexpr x86::reg32 kAboutMaxTextureWidth = 0x14;
constexpr x86::reg32 kAboutTextureWidthMultiple = 0x18;
constexpr x86::reg32 kAboutMinTextureHeight = 0x1c;
constexpr x86::reg32 kAboutMaxTextureHeight = 0x20;
constexpr x86::reg32 kAboutTextureHeightMultiple = 0x24;
constexpr x86::reg32 kAboutClipAlign = 0x28;
constexpr x86::reg32 kAboutTextureFormatCount = 0x2c;     // voodoo2a: 9
constexpr x86::reg32 kAboutTextureFormats = 0x30;         // a dword each, non-zero where the format is taken (0xa91fd4)
constexpr x86::reg32 kAboutUnknownCount = 0x34;           // voodoo2a: 4; what they are is not known
constexpr x86::reg32 kAboutUnknown = 0x38;                // voodoo2a: 0xa92000
constexpr x86::reg32 kAboutModeCount = 0x3c;              // voodoo2a: 16
constexpr x86::reg32 kAboutModes = 0x40;                  // kMode* records (0xa92018)
constexpr x86::reg32 kAboutMaxTextures = 0x44;            // at once
constexpr x86::reg32 kAboutName = 0x4c;                   // kNameLength chars
constexpr x86::reg32 kAboutSubType = 0x6c;                // voodoo2a: the card's type from Glide
constexpr x86::reg32 kAboutVideoMemory = 0x70;            // voodoo2a: the texture memory, a multiple of 16
constexpr x86::reg32 kAboutTotalMemory = 0x74;            // voodoo2a puts 1 here
constexpr x86::reg32 kAboutAuthor = 0x78;
constexpr x86::reg32 kAboutDirectXVersion = 0x7c;
constexpr x86::reg32 kAboutBytes = 0x80;
constexpr x86::reg32 kNameLength = 32;

constexpr x86::reg32 kSignature3dfx = 0x33444658;         // "XFD3" in memory
constexpr x86::reg32 kSignatureSoftware = 0x53545249;     // "STRI", softtria's

constexpr x86::reg32 kCapsLineWidth = 0x01;
constexpr x86::reg32 kCapsTextureSquare = 0x02;
constexpr x86::reg32 kCapsTextureWidthPow2 = 0x04;
constexpr x86::reg32 kCapsTextureHeightPow2 = 0x08;
constexpr x86::reg32 kCapsSoftware = 0x10;
constexpr x86::reg32 kCapsWindowed = 0x20;
constexpr x86::reg32 kCapsGlobalCut = 0x40;
constexpr x86::reg32 kCapsTrilinearPass = 0x80;

/* A mode of the descriptor's table, kModeBytes. */
constexpr x86::reg32 kModeWidth = 0x00;
constexpr x86::reg32 kModeHeight = 0x04;
constexpr x86::reg32 kModeBits = 0x08;
constexpr x86::reg32 kModeFormat = 0x0c;                  // the frame buffer's pixels
constexpr x86::reg32 kModeAvailable = 0x10;
constexpr x86::reg32 kModeColourBuffers = 0x14;           // voodoo2a: as many as the card's memory holds
constexpr x86::reg32 kModeAuxBuffers = 0x18;              // voodoo2a: one less, at most 3
constexpr x86::reg32 kModeBytes = 0x28;

/* THRASH_lockwindow's answer, a record
 * voodoo2a allocates, kLockBytes; unlockwindow takes it back. */
constexpr x86::reg32 kLockData = 0x00;                    // the pixels
constexpr x86::reg32 kLockStride = 0x04;                  // bytes a line
constexpr x86::reg32 kLockFormat = 0x08;                  // voodoo2a: 4, R5G6B5
constexpr x86::reg32 kLockWidth = 0x0c;
constexpr x86::reg32 kLockHeight = 0x10;
constexpr x86::reg32 kLockBuffer = 0x14;                  // voodoo2a's own: the buffer locked
constexpr x86::reg32 kLockBytes = 0x18;

}  // namespace thrash
}  // namespace nfs3hp

#endif
