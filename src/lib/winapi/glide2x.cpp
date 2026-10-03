#include <winapi/glide2x.h>
#include <winapi/wrapper.h>
#include <lib/library.h>
#include <lib/renderer.h>
#include <lib/gliderenderer.h>
#include <lib/thrashrenderer.h>
#include <lib/window.h>
#include <SDL3/SDL.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <vector>


namespace win32 { namespace glide2x
{

static Renderer* s_renderer;
static GlideBackend* s_glideRenderer;
/* The native THRASH driver draws through its own renderer (useThrashRenderer). */
static bool s_thrashRenderer = false;
static SwapObserver s_swapObserver;
static x86::reg32 s_preferredAtlasSize = 2048;

/* NFS_NATIVE_CHECK: while set, every state call a native THRASH stand-in or
 * voodoo2a makes is appended here -- its entry's offset in voodoo2a's table of
 * Glide functions (0x60 grColorCombine, ...), its arguments, and a marker. */
static std::vector<x86::reg32>* s_callTrace = nullptr;

static void traceCall(x86::reg32 id, std::initializer_list<x86::reg32> args)
{
    if (!s_callTrace)
        return;
    s_callTrace->push_back(id);
    s_callTrace->insert(s_callTrace->end(), args.begin(), args.end());
    s_callTrace->push_back(0xffffffff);
}

static x86::reg32 floatBits(float value)
{
    x86::reg32 bits;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}

static void traceTexture(x86::reg32 id, x86::reg32 tmu, x86::reg32 start, x86::reg32 evenOdd, const void* info)
{
    if (!s_callTrace)
        return;
    // The record as the guest wrote it: four dwords and the guest address of the texels.
    x86::reg32 record[5];
    std::memcpy(record, info, sizeof record);
    traceCall(id, {tmu, start, evenOdd, record[0], record[1], record[2], record[3], record[4]});
}

static void traceFogTable(const x86::reg8* table)
{
    if (!s_callTrace)
        return;
    s_callTrace->push_back(0x94);
    for (int i = 0; i < 64; i += 4)
        s_callTrace->push_back(x86::reg32(table[i]) | x86::reg32(table[i + 1]) << 8
                               | x86::reg32(table[i + 2]) << 16 | x86::reg32(table[i + 3]) << 24);
    s_callTrace->push_back(0xffffffff);
}

void useThrashRenderer(bool on)
{
    s_thrashRenderer = on;
}

/* The ThrashRenderer, while there is one (direct::thrashRenderer). */
static ThrashRenderer* s_directRenderer = nullptr;

void setPreferredAtlasSize(x86::reg32 size)
{
    s_preferredAtlasSize = size;
}

bool fullColourTextures()
{
    static const bool on = []() {
        const char* value = SDL_getenv("NFS_TEXTURES32");
        return !value || SDL_strcmp(value, "0") != 0;
    }();
    return on;
}

/* fitFourThree: x becomes s_fitOffset + x * s_fitScale for everything drawn. */
static float s_fitScale = 1.0f, s_fitOffset = 0.0f;
/* The clip window as grClipWindow last set it, put back after clearPicture. */
static x86::reg32 s_clipWindow[4];
static bool s_clipWindowSet = false;

void fitFourThree(bool on)
{
    s_fitScale = 1.0f;
    s_fitOffset = 0.0f;
    if (!on || !s_renderer)
        return;
    const float width = float(s_renderer->getWidth()), height = float(s_renderer->getHeight());
    if (height <= 0 || width * 3 <= height * 4)
        return;
    s_fitScale = height * 4 / (width * 3);
    s_fitOffset = (width - height * 4 / 3) / 2;
}

void clearPicture(WinApplication* app, x86::CPU& cpu)
{
    if (!s_renderer || !s_glideRenderer)
        return;
    app->unlockContext(cpu);
    s_glideRenderer->setClipWindow(0, 0, s_renderer->getWidth(), s_renderer->getHeight());
    s_glideRenderer->clear(0);
    if (s_clipWindowSet)
        s_glideRenderer->setClipWindow(s_clipWindow[0], s_clipWindow[1], s_clipWindow[2], s_clipWindow[3]);
    app->lockContext(cpu);
}

static const GrVertex* fitted(const GrVertex* vertex, GrVertex& copy)
{
    if (s_fitScale == 1.0f && s_fitOffset == 0.0f)
        return vertex;
    copy = *vertex;
    copy.x = s_fitOffset + copy.x * s_fitScale;
    return &copy;
}

/* squeezeToLeft: each triangle narrowed by the 4:3 factor towards its own
 * leftmost corner, so a picture laid out for 640x480 and stretched over a wide
 * one -- the NFS emblem of a dialog box -- keeps its shape where it stands.  Both
 * triangles of a quad hold one of its left corners, so the quad narrows as one. */
static float s_squeezeScale = 1.0f;

void squeezeToLeft(bool on)
{
    s_squeezeScale = 1.0f;
    if (!on || !s_renderer)
        return;
    const float width = float(s_renderer->getWidth()), height = float(s_renderer->getHeight());
    if (height > 0 && width * 3 > height * 4)
        s_squeezeScale = height * 4 / (width * 3);
}

static void squeezed(const GrVertex*& a, const GrVertex*& b, const GrVertex*& c, GrVertex copies[3])
{
    if (s_squeezeScale == 1.0f)
        return;
    const float left = std::min(std::min(a->x, b->x), c->x);
    copies[0] = *a;
    copies[1] = *b;
    copies[2] = *c;
    for (int i = 0; i < 3; ++i)
        copies[i].x = left + (copies[i].x - left) * s_squeezeScale;
    a = &copies[0];
    b = &copies[1];
    c = &copies[2];
}

bool screenRect(int& x, int& y, int& w, int& h, int& pictureWidth, int& pictureHeight)
{
    if (!s_renderer)
        return false;
    s_renderer->getViewportRect(x, y, w, h);
    pictureWidth = int(s_renderer->getWidth());
    pictureHeight = int(s_renderer->getHeight());
    return w > 0 && h > 0 && pictureWidth > 0 && pictureHeight > 0;
}

void setSwapObserver(SwapObserver observer)
{
    s_swapObserver = observer;
}

bool atlasFreeSpace(x86::reg32& freeTexels, x86::reg32& freeTiles, x86::reg32& totalTexels)
{
    if (!s_glideRenderer)
        return false;
    s_glideRenderer->atlasFreeSpace(freeTexels, freeTiles, totalTexels);
    return true;
}

static const x86::reg32 GR_FOG_TABLE_SIZE   = 64;
static const x86::reg32 MAX_NUM_SST         = 4;
static const x86::reg32 GLIDE_NUM_TMU       = RENDERER_NUM_TMU;

typedef x86::reg32 GrScreenResolution_t;
static const GrScreenResolution_t GR_RESOLUTION_320x200 = 0x0;
static const GrScreenResolution_t GR_RESOLUTION_320x240 = 0x1;
static const GrScreenResolution_t GR_RESOLUTION_400x256 = 0x2;
static const GrScreenResolution_t GR_RESOLUTION_512x384 = 0x3;
static const GrScreenResolution_t GR_RESOLUTION_640x200 = 0x4;
static const GrScreenResolution_t GR_RESOLUTION_640x350 = 0x5;
static const GrScreenResolution_t GR_RESOLUTION_640x400 = 0x6;
static const GrScreenResolution_t GR_RESOLUTION_640x480 = 0x7;
static const GrScreenResolution_t GR_RESOLUTION_800x600 = 0x8;
static const GrScreenResolution_t GR_RESOLUTION_960x720 = 0x9;
static const GrScreenResolution_t GR_RESOLUTION_856x480 = 0xa;
static const GrScreenResolution_t GR_RESOLUTION_512x256 = 0xb;
static const GrScreenResolution_t GR_RESOLUTION_1024x768 = 0xc;
//static const GrScreenResolution_t GR_RESOLUTION_NONE = 0xff;
//static const GrScreenResolution_t GR_RESOLUTION_MIN = GR_RESOLUTION_320x200;
//static const GrScreenResolution_t GR_RESOLUTION_MAX = GR_RESOLUTION_1024x768;


typedef x86::reg32 GrScreenRefresh_t;
/*static const GrScreenRefresh_t GR_REFRESH_60Hz  = 0x0;
static const GrScreenRefresh_t GR_REFRESH_70Hz  = 0x1;
static const GrScreenRefresh_t GR_REFRESH_72Hz  = 0x2;
static const GrScreenRefresh_t GR_REFRESH_75Hz  = 0x3;
static const GrScreenRefresh_t GR_REFRESH_80Hz  = 0x4;
static const GrScreenRefresh_t GR_REFRESH_90Hz  = 0x5;
static const GrScreenRefresh_t GR_REFRESH_100Hz = 0x6;
static const GrScreenRefresh_t GR_REFRESH_85Hz  = 0x7;
static const GrScreenRefresh_t GR_REFRESH_120Hz = 0x8;
static const GrScreenRefresh_t GR_REFRESH_NONE  = 0xff;*/


typedef x86::reg32 GrColorFormat_t;
/*static const GrColorFormat_t GR_COLORFORMAT_ARGB = 0x0;
static const GrColorFormat_t GR_COLORFORMAT_ABGR = 0x1;*/


typedef x86::reg32 GrOriginLocation_t;
/*static const GrOriginLocation_t GR_ORIGIN_UPPER_LEFT = 0x0;
static const GrOriginLocation_t GR_ORIGIN_LOWER_LEFT = 0x1;
static const GrOriginLocation_t GR_ORIGIN_ANY = 0xff;*/


typedef x86::reg32 GrChipID_t;
//static const GrChipID_t GR_TMU0 = 0x0;
//static const GrChipID_t GR_TMU1 = 0x1;
//static const GrChipID_t GR_TMU2 = 0x2;
//static const GrChipID_t GR_FBI  = 0x3;

typedef x86::reg32 GrCmpFnc_t;
//static const GrCmpFnc_t GR_CMP_NEVER    = 0x0;
//static const GrCmpFnc_t GR_CMP_LESS     = 0x1;
//static const GrCmpFnc_t GR_CMP_EQUAL    = 0x2;
static const GrCmpFnc_t GR_CMP_LEQUAL   = 0x3;
static const GrCmpFnc_t GR_CMP_GREATER  = 0x4;
//static const GrCmpFnc_t GR_CMP_NOTEQUAL = 0x5;
//static const GrCmpFnc_t GR_CMP_GEQUAL   = 0x6;
static const GrCmpFnc_t GR_CMP_ALWAYS   = 0x7;

typedef x86::reg32 GrCombineFunction_t;
//static const GrCombineFunction_t GR_COMBINE_FUNCTION_ZERO                                       = 0x0;
//static const GrCombineFunction_t GR_COMBINE_FUNCTION_NONE                                       = GR_COMBINE_FUNCTION_ZERO;
static const GrCombineFunction_t GR_COMBINE_FUNCTION_LOCAL                                      = 0x1;
//static const GrCombineFunction_t GR_COMBINE_FUNCTION_LOCAL_ALPHA                                = 0x2;
static const GrCombineFunction_t GR_COMBINE_FUNCTION_SCALE_OTHER                                = 0x3;
static const GrCombineFunction_t GR_COMBINE_FUNCTION_BLEND_OTHER                                = GR_COMBINE_FUNCTION_SCALE_OTHER;
//static const GrCombineFunction_t GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL                      = 0x4;
//static const GrCombineFunction_t GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA                = 0x5;
//static const GrCombineFunction_t GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL                    = 0x6;
//static const GrCombineFunction_t GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL          = 0x7;
//static const GrCombineFunction_t GR_COMBINE_FUNCTION_BLEND                                      = GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL;
//static const GrCombineFunction_t GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA    = 0x8;
//static const GrCombineFunction_t GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL                = 0x9;
//static const GrCombineFunction_t GR_COMBINE_FUNCTION_BLEND_LOCAL                                = GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL;
//static const GrCombineFunction_t GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL_ALPHA          = 0x10;

typedef x86::reg32 GrCombineFactor_t;
static const GrCombineFactor_t GR_COMBINE_FACTOR_ZERO                       = 0x0;
//static const GrCombineFactor_t GR_COMBINE_FACTOR_NONE                       = GR_COMBINE_FACTOR_ZERO;
static const GrCombineFactor_t GR_COMBINE_FACTOR_LOCAL                      = 0x1;
//static const GrCombineFactor_t GR_COMBINE_FACTOR_OTHER_ALPHA                = 0x2;
//static const GrCombineFactor_t GR_COMBINE_FACTOR_LOCAL_ALPHA                = 0x3;
//static const GrCombineFactor_t GR_COMBINE_FACTOR_TEXTURE_ALPHA              = 0x4;
//static const GrCombineFactor_t GR_COMBINE_FACTOR_DETAIL_FACTOR              = GR_COMBINE_FACTOR_TEXTURE_ALPHA;
//static const GrCombineFactor_t GR_COMBINE_FACTOR_LOD_FRACTION               = 0x5;
//static const GrCombineFactor_t GR_COMBINE_FACTOR_ONE                        = 0x8;
//static const GrCombineFactor_t GR_COMBINE_FACTOR_ONE_MINUS_LOCAL            = 0x9;
//static const GrCombineFactor_t GR_COMBINE_FACTOR_ONE_MINUS_OTHER_ALPHA      = 0xa;
//static const GrCombineFactor_t GR_COMBINE_FACTOR_ONE_MINUS_LOCAL_ALPHA      = 0xb;
//static const GrCombineFactor_t GR_COMBINE_FACTOR_ONE_MINUS_TEXTURE_ALPHA    = 0xc;
//static const GrCombineFactor_t GR_COMBINE_FACTOR_ONE_MINUS_DETAIL_FACTOR    = GR_COMBINE_FACTOR_ONE_MINUS_TEXTURE_ALPHA;
//static const GrCombineFactor_t GR_COMBINE_FACTOR_ONE_MINUS_LOD_FRACTION     = 0xd;

typedef x86::reg32 GrCombineLocal_t;
static const GrCombineLocal_t GR_COMBINE_LOCAL_ITERATED = 0x0;
//static const GrCombineLocal_t GR_COMBINE_LOCAL_CONSTANT = 0x1;
//static const GrCombineLocal_t GR_COMBINE_LOCAL_NONE     = GR_COMBINE_LOCAL_CONSTANT;
//static const GrCombineLocal_t GR_COMBINE_LOCAL_DEPTH    = 0x2;

typedef x86::reg32 GrCombineOther_t;
//static const GrCombineOther_t GR_COMBINE_OTHER_ITERATED = 0x0;
static const GrCombineOther_t GR_COMBINE_OTHER_TEXTURE  = 0x1;
static const GrCombineOther_t GR_COMBINE_OTHER_CONSTANT = 0x2;
static const GrCombineOther_t GR_COMBINE_OTHER_NONE     = GR_COMBINE_OTHER_CONSTANT;

typedef x86::reg32 GrTextureCombineFnc_t;
//static const GrCombineFunction_t GR_TEXTURECOMBINE_ZERO             = 0x0;
//static const GrCombineFunction_t GR_TEXTURECOMBINE_DECAL            = 0x1;
//static const GrCombineFunction_t GR_TEXTURECOMBINE_OTHER            = 0x2;
//static const GrCombineFunction_t GR_TEXTURECOMBINE_ADD              = 0x3;
//static const GrCombineFunction_t GR_TEXTURECOMBINE_MULTIPLY         = 0x4;
//static const GrCombineFunction_t GR_TEXTURECOMBINE_SUBTRACT         = 0x5;
//static const GrCombineFunction_t GR_TEXTURECOMBINE_DETAIL           = 0x6;
//static const GrCombineFunction_t GR_TEXTURECOMBINE_DETAIL_OTHER     = 0x7;
//static const GrCombineFunction_t GR_TEXTURECOMBINE_TRILINEAR_ODD    = 0x8;
//static const GrCombineFunction_t GR_TEXTURECOMBINE_TRILINEAR_EVEN   = 0x9;
//static const GrCombineFunction_t GR_TEXTURECOMBINE_ONE              = 0xa;

typedef x86::reg32 GrAlphaBlendFnc_t;
static const GrAlphaBlendFnc_t GR_BLEND_ZERO                = 0x0;
static const GrAlphaBlendFnc_t GR_BLEND_SRC_ALPHA           = 0x1;
//static const GrAlphaBlendFnc_t GR_BLEND_SRC_COLOR           = 0x2;
//static const GrAlphaBlendFnc_t GR_BLEND_DST_COLOR           = GR_BLEND_SRC_COLOR;
//static const GrAlphaBlendFnc_t GR_BLEND_DST_ALPHA           = 0x3;
static const GrAlphaBlendFnc_t GR_BLEND_ONE                 = 0x4;
static const GrAlphaBlendFnc_t GR_BLEND_ONE_MINUS_SRC_ALPHA = 0x5;
//static const GrAlphaBlendFnc_t GR_BLEND_ONE_MINUS_SRC_COLOR = 0x6;
//static const GrAlphaBlendFnc_t GR_BLEND_ONE_MINUS_DST_COLOR = GR_BLEND_ONE_MINUS_SRC_COLOR;
//static const GrAlphaBlendFnc_t GR_BLEND_ONE_MINUS_DST_ALPHA = 0x7;
//static const GrAlphaBlendFnc_t GR_BLEND_RESERVED_8          = 0x8;
//static const GrAlphaBlendFnc_t GR_BLEND_RESERVED_9          = 0x9;
//static const GrAlphaBlendFnc_t GR_BLEND_RESERVED_A          = 0xa;
//static const GrAlphaBlendFnc_t GR_BLEND_RESERVED_B          = 0xb;
//static const GrAlphaBlendFnc_t GR_BLEND_RESERVED_C          = 0xc;
//static const GrAlphaBlendFnc_t GR_BLEND_RESERVED_D          = 0xd;
//static const GrAlphaBlendFnc_t GR_BLEND_RESERVED_E          = 0xe;
//static const GrAlphaBlendFnc_t GR_BLEND_ALPHA_SATURATE      = 0xf;
//static const GrAlphaBlendFnc_t GR_BLEND_PREFOG_COLOR        = GR_BLEND_ALPHA_SATURATE;

typedef x86::reg32 GrTexTable_t;
//static const GrTexTable_t GR_TEXTABLE_NCC0    = 0x0;
//static const GrTexTable_t GR_TEXTABLE_NCC1    = 0x1;
//static const GrTexTable_t GR_TEXTABLE_PALETTE = 0x2;

typedef int32_t GrTextureFilterMode_t;
//static const GrTextureFilterMode_t GR_TEXTUREFILTER_POINT_SAMPLED = 0x0;
static const GrTextureFilterMode_t GR_TEXTUREFILTER_BILINEAR      = 0x1;

typedef x86::reg32 GrTextureClampMode_t;
//static const GrTextureClampMode_t GR_TEXTURECLAMP_WRAP   = 0x0;
//static const GrTextureClampMode_t GR_TEXTURECLAMP_CLAMP  = 0x1;

typedef x86::reg32 GrColor_t;
typedef x86::reg8  GrAlpha_t;
typedef x86::reg32 GrMipMapId_t;
typedef x86::reg8  GrFog_t;

typedef x86::reg32 GrBuffer_t;
//static const GrBuffer_t GR_BUFFER_FRONTBUFFER  = 0x0;
//static const GrBuffer_t GR_BUFFER_BACKBUFFER   = 0x1;
//static const GrBuffer_t GR_BUFFER_AUXBUFFER    = 0x2;
//static const GrBuffer_t GR_BUFFER_DEPTHBUFFER  = 0x3;
//static const GrBuffer_t GR_BUFFER_ALPHABUFFER  = 0x4;
//static const GrBuffer_t GR_BUFFER_TRIPLEBUFFER = 0x5;

typedef x86::reg32 GrCullMode_t;
//static const GrCullMode_t GR_CULL_DISABLE      = 0x0;
//static const GrCullMode_t GR_CULL_NEGATIVE     = 0x1;
//static const GrCullMode_t GR_CULL_POSITIVE     = 0x2;

typedef x86::reg32 GrDitherMode_t;
//static const GrDitherMode_t GR_DITHER_DISABLE  = 0x0;
//static const GrDitherMode_t GR_DITHER_2x2      = 0x1;
//static const GrDitherMode_t GR_DITHER_4x4      = 0x2;

typedef x86::reg32 GrFogMode_t;
//static const GrFogMode_t GR_FOG_DISABLE             = 0x0;
//static const GrFogMode_t GR_FOG_WITH_ITERATED_ALPHA = 0x1;
//static const GrFogMode_t GR_FOG_WITH_TABLE          = 0x2;
//static const GrFogMode_t GR_FOG_MULT2               = 0x100;
//static const GrFogMode_t GR_FOG_ADD2                = 0x200;

typedef x86::reg32 GrDepthBufferMode_t;
//static const GrDepthBufferMode_t GR_DEPTHBUFFER_DISABLE                     = 0x0;
//static const GrDepthBufferMode_t GR_DEPTHBUFFER_ZBUFFER                     = 0x1;
//static const GrDepthBufferMode_t GR_DEPTHBUFFER_WBUFFER                     = 0x2;
//static const GrDepthBufferMode_t GR_DEPTHBUFFER_ZBUFFER_COMPARE_TO_BIAS     = 0x3;
//static const GrDepthBufferMode_t GR_DEPTHBUFFER_WBUFFER_COMPARE_TO_BIAS     = 0x4;

typedef x86::reg32 GrChromakeyMode_t;
//static const GrChromakeyMode_t GR_CHROMAKEY_DISABLE     = 0x0;
//static const GrChromakeyMode_t GR_CHROMAKEY_ENABLE      = 0x1;

typedef x86::reg32 GrLock_t;
//static const GrLock_t GR_LFB_READ_ONLY  = 0x00;
static const GrLock_t GR_LFB_WRITE_ONLY = 0x01;
//static const GrLock_t GR_LFB_IDLE       = 0x00;
//static const GrLock_t GR_LFB_NOIDLE     = 0x10;

typedef x86::reg32 GrLfbBypassMode_t;
//static const GrLfbBypassMode_t GR_LFBBYPASS_DISABLE = 0x0;
//static const GrLfbBypassMode_t GR_LFBBYPASS_ENABLE  = 0x1;

typedef x86::reg32 GrLfbWriteMode_t;
//static const GrLfbWriteMode_t GR_LFBWRITEMODE_565           = 0x0; /* RGB:RGB */
//static const GrLfbWriteMode_t GR_LFBWRITEMODE_555           = 0x1; /* RGB:RGB */
//static const GrLfbWriteMode_t GR_LFBWRITEMODE_1555          = 0x2; /* ARGB:ARGB */
//static const GrLfbWriteMode_t GR_LFBWRITEMODE_RESERVED1     = 0x3;
//static const GrLfbWriteMode_t GR_LFBWRITEMODE_888           = 0x4; /* RGB */
//static const GrLfbWriteMode_t GR_LFBWRITEMODE_8888          = 0x5; /* ARGB */
//static const GrLfbWriteMode_t GR_LFBWRITEMODE_RESERVED2     = 0x6;
//static const GrLfbWriteMode_t GR_LFBWRITEMODE_RESERVED3     = 0x7;
//static const GrLfbWriteMode_t GR_LFBWRITEMODE_RESERVED4     = 0x8;
//static const GrLfbWriteMode_t GR_LFBWRITEMODE_RESERVED5     = 0x9;
//static const GrLfbWriteMode_t GR_LFBWRITEMODE_RESERVED6     = 0xa;
//static const GrLfbWriteMode_t GR_LFBWRITEMODE_RESERVED7     = 0xb;
//static const GrLfbWriteMode_t GR_LFBWRITEMODE_565_DEPTH     = 0xc; /* RGB:DEPTH */
//static const GrLfbWriteMode_t GR_LFBWRITEMODE_555_DEPTH     = 0xd; /* RGB:DEPTH */
//static const GrLfbWriteMode_t GR_LFBWRITEMODE_1555_DEPTH    = 0xe; /* ARGB:DEPTH */
//static const GrLfbWriteMode_t GR_LFBWRITEMODE_ZA16          = 0xf; /* DEPTH:DEPTH */
//static const GrLfbWriteMode_t GR_LFBWRITEMODE_ANY           = 0xff;

typedef x86::reg32 GrLfbSrcFmt_t;
//static const GrLfbSrcFmt_t GR_LFB_SRC_FMT_565           = 0x00;
//static const GrLfbSrcFmt_t GR_LFB_SRC_FMT_555           = 0x01;
//static const GrLfbSrcFmt_t GR_LFB_SRC_FMT_1555          = 0x02;
//static const GrLfbSrcFmt_t GR_LFB_SRC_FMT_888           = 0x04;
//static const GrLfbSrcFmt_t GR_LFB_SRC_FMT_8888          = 0x05;
//static const GrLfbSrcFmt_t GR_LFB_SRC_FMT_565_DEPTH     = 0x0c;
//static const GrLfbSrcFmt_t GR_LFB_SRC_FMT_555_DEPTH     = 0x0d;
//static const GrLfbSrcFmt_t GR_LFB_SRC_FMT_1555_DEPTH    = 0x0e;
//static const GrLfbSrcFmt_t GR_LFB_SRC_FMT_ZA16          = 0x0f;
//static const GrLfbSrcFmt_t GR_LFB_SRC_FMT_RLE16         = 0x80;

typedef x86::reg32 GrLOD_t;
//static const GrLOD_t GR_LOD_256                 = 0x0;
//static const GrLOD_t GR_LOD_128                 = 0x1;
//static const GrLOD_t GR_LOD_64                  = 0x2;
//static const GrLOD_t GR_LOD_32                  = 0x3;
//static const GrLOD_t GR_LOD_16                  = 0x4;
//static const GrLOD_t GR_LOD_8                   = 0x5;
//static const GrLOD_t GR_LOD_4                   = 0x6;
//static const GrLOD_t GR_LOD_2                   = 0x7;
//static const GrLOD_t GR_LOD_1                   = 0x8;

struct GrLfbInfo_t
{
    x86::sreg32         size;
    Packed<void>        lfbPtr;
    x86::reg32          strideInBytes;
    GrLfbWriteMode_t    writeMode;
    GrOriginLocation_t  origin;
};

typedef x86::reg32 GrMipMapMode_t;
//static const GrMipMapMode_t GR_MIPMAP_DISABLE           = 0x0; /* no mip mapping  */
//static const GrMipMapMode_t GR_MIPMAP_NEAREST           = 0x1; /* use nearest mipmap */
//static const GrMipMapMode_t GR_MIPMAP_NEAREST_DITHER    = 0x2; /* GR_MIPMAP_NEAREST + LOD dith */


typedef x86::reg32 GrAspectRatio_t;
//static const GrAspectRatio_t GR_ASPECT_8x1  = 0x0;       /* 8W x 1H */
//static const GrAspectRatio_t GR_ASPECT_4x1  = 0x1;       /* 4W x 1H */
//static const GrAspectRatio_t GR_ASPECT_2x1  = 0x2;       /* 2W x 1H */
static const GrAspectRatio_t GR_ASPECT_1x1  = 0x3;       /* 1W x 1H */
//static const GrAspectRatio_t GR_ASPECT_1x2  = 0x4;       /* 1W x 2H */
//static const GrAspectRatio_t GR_ASPECT_1x4  = 0x5;       /* 1W x 4H */
//static const GrAspectRatio_t GR_ASPECT_1x8  = 0x6;       /* 1W x 8H */

typedef x86::reg32 GrTextureFormat_t;
//static const GrTextureFormat_t GR_TEXFMT_8BIT               = 0x0;
//static const GrTextureFormat_t GR_TEXFMT_RGB_332            = GR_TEXFMT_8BIT;
//static const GrTextureFormat_t GR_TEXFMT_YIQ_422            = 0x1;
//static const GrTextureFormat_t GR_TEXFMT_ALPHA_8            = 0x2; /* (0..0xFF) alpha     */
//static const GrTextureFormat_t GR_TEXFMT_INTENSITY_8        = 0x3; /* (0..0xFF) intensity */
//static const GrTextureFormat_t GR_TEXFMT_ALPHA_INTENSITY_44 = 0x4;
//static const GrTextureFormat_t GR_TEXFMT_P_8                = 0x5; /* 8-bit palette */
//static const GrTextureFormat_t GR_TEXFMT_RSVD0              = 0x6;
//static const GrTextureFormat_t GR_TEXFMT_RSVD1              = 0x7;
//static const GrTextureFormat_t GR_TEXFMT_16BIT              = 0x8;
//static const GrTextureFormat_t GR_TEXFMT_ARGB_8332          = GR_TEXFMT_16BIT;
//static const GrTextureFormat_t GR_TEXFMT_AYIQ_8422          = 0x9;
static const GrTextureFormat_t GR_TEXFMT_RGB_565            = 0xa;
static const GrTextureFormat_t GR_TEXFMT_ARGB_1555          = 0xb;
static const GrTextureFormat_t GR_TEXFMT_ARGB_4444          = 0xc;
//static const GrTextureFormat_t GR_TEXFMT_ALPHA_INTENSITY_88 = 0xd;
//static const GrTextureFormat_t GR_TEXFMT_AP_88              = 0xe; /* 8-bit alpha 8-bit palette */
//static const GrTextureFormat_t GR_TEXFMT_RSVD2              = 0xf;

struct GrTexInfo
{
    GrLOD_t           smallLod;
    GrLOD_t           largeLod;
    GrAspectRatio_t   aspectRatio;
    GrTextureFormat_t format;
    Packed<void>      data;
};

typedef x86::reg32 GrHints_t;

typedef x86::reg32 GrSstType;

//static const GrSstType GR_SSTTYPE_VOODOO = 0;
//static const GrSstType GR_SSTTYPE_SST96 = 1;
//static const GrSstType GR_SSTTYPE_AT3D = 2;
static const GrSstType GR_SSTTYPE_Voodoo2 = 3;
//static const GrSstType GR_SSTTYPE_Banshee = 4;
//static const GrSstType GR_SSTTYPE_Voodoo3 = 5;
//static const GrSstType GR_SSTTYPE_Voodoo4 = 6;
//static const GrSstType GR_SSTTYPE_Voodoo5 = 7;

struct GrTMUConfig_t
{
    x86::reg32 tmuRev;
    x86::reg32 tmuRam;
};

struct GrHwConfiguration
{
    x86::sreg32 num_sst;
    struct SstCard_St
    {
        GrSstType type;         /* Which hardware is it? */
        x86::reg32 fbRam;       /* 1, 2, or 4 MB */
        x86::reg32 fbiRev;      /* Rev of Pixelfx chip */
        x86::reg32 nTexelfx;    /* How many texelFX chips are there? */
        x86::reg32 sli;         /* SLI interleaved support */
        GrTMUConfig_t tmuConfig[GLIDE_NUM_TMU]; /* Configuration of the Texelfx chips */
    } SSTs[MAX_NUM_SST];
};

static void grGlideInit(WinApplication* app, x86::CPU& cpu)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    s_renderer = nullptr;
    s_glideRenderer = nullptr;
    s_directRenderer = nullptr;
}

static void grGlideShutdown(WinApplication* app, x86::CPU& cpu)
{
    NFS2_USE(cpu);
    if (s_glideRenderer)
    {
        app->freeResource(s_glideRenderer->getResourceIndex());
        s_glideRenderer = nullptr;
        s_directRenderer = nullptr;
    }
    if (s_renderer)
    {
        app->freeResource(s_renderer->getResourceIndex());
        s_renderer = nullptr;
    }
}

static x86::reg32 grSstQueryHardware(WinApplication* app, x86::CPU& cpu, GrHwConfiguration *hwconfig)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    hwconfig->num_sst = 1;
    hwconfig->SSTs[0].type = GR_SSTTYPE_Voodoo2;
    hwconfig->SSTs[0].fbRam = 16;
    hwconfig->SSTs[0].fbiRev = 222;
    hwconfig->SSTs[0].nTexelfx = 1;
    hwconfig->SSTs[0].sli = 0;
    hwconfig->SSTs[0].tmuConfig[0].tmuRev = 1;
    hwconfig->SSTs[0].tmuConfig[0].tmuRam = 16;
    return true;
}

static x86::reg32 grSstQueryBoards(WinApplication* app, x86::CPU& cpu, GrHwConfiguration *hwconfig)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    hwconfig->num_sst = 1;
    return true;
}

static void grSstSelect(WinApplication* app, x86::CPU& cpu, x86::sreg32 sst)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_ASSERT(sst == 0);
}

static x86::reg32 grSstWinOpen(WinApplication* app, x86::CPU& cpu, HWND hWnd,
                               GrScreenResolution_t resolution, GrScreenRefresh_t refreshRate, GrColorFormat_t format,
                               GrOriginLocation_t origin, x86::sreg32 nColBuffers, x86::sreg32 nAuxBuffers)
{
    NFS2_USE(cpu);
    NFS2_USE(refreshRate);
    NFS2_USE(format);
    NFS2_USE(origin);
    NFS2_USE(nColBuffers);
    NFS2_USE(nAuxBuffers);

    x86::sreg32 width, height;
    switch(resolution)
    {
    case GR_RESOLUTION_320x200:
        width = 320;
        height = 200;
        break;
    case GR_RESOLUTION_320x240:
        width = 320;
        height = 240;
        break;
    case GR_RESOLUTION_400x256:
        width = 400;
        height = 256;
        break;
    case GR_RESOLUTION_512x384:
        width = 512;
        height = 384;
        break;
    case GR_RESOLUTION_640x200:
        width = 640;
        height = 200;
        break;
    case GR_RESOLUTION_640x350:
        width = 640;
        height = 350;
        break;
    case GR_RESOLUTION_640x400:
        width = 640;
        height = 400;
        break;
    case GR_RESOLUTION_640x480:
        width = 640;
        height = 480;
        break;
    case GR_RESOLUTION_800x600:
        width = 800;
        height = 600;
        break;
    case GR_RESOLUTION_960x720:
        width = 960;
        height = 720;
        break;
    case GR_RESOLUTION_856x480:
        width = 856;
        height = 480;
        break;
    case GR_RESOLUTION_512x256:
        width = 512;
        height = 256;
        break;
    case GR_RESOLUTION_1024x768:
        width = 1024;
        height = 768;
        break;
    case win32::glide2x::kResolution1280x720:
        width = 1280;
        height = 720;
        break;
    case win32::glide2x::kResolution1600x900:
        width = 1600;
        height = 900;
        break;
    case win32::glide2x::kResolution1920x1080:
        width = 1920;
        height = 1080;
        break;
    default:
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Unsupported resolution: %d", resolution);
        width = 640;
        height = 480;
        break;
    }
    s_renderer = new Renderer(app, dynamic_cast<Window*>(app->getResource(hWnd)));
    s_renderer->setVideoMode(width, height, 16);
    app->allocateResource(s_renderer);
    s_directRenderer = nullptr;
    if (s_thrashRenderer)
        s_glideRenderer = s_directRenderer = new ThrashRenderer(s_renderer);
    else
        s_glideRenderer = new GlideRenderer(s_renderer, s_preferredAtlasSize);
    app->allocateResource(s_glideRenderer);
    x86::reg16 data = 0xff;
    s_glideRenderer->setTextureData(0, 0, &data, 8, 8, TF_ARGB_4444);
    return true;
}

static void grSstWinClose(WinApplication* app, x86::CPU& cpu)
{
    NFS2_USE(cpu);
    if (s_glideRenderer)
    {
        app->freeResource(s_glideRenderer->getResourceIndex());
        s_glideRenderer = nullptr;
        s_directRenderer = nullptr;
    }
    if (s_renderer)
    {
        app->freeResource(s_renderer->getResourceIndex());
        s_renderer = nullptr;
    }
}

static x86::reg32 grSstStatus(WinApplication* app, x86::CPU& cpu)
{
    traceCall(0x4c, {});
    NFS2_USE(app);
    NFS2_USE(cpu);
    return 0x0fff03f;
}

static x86::reg32 grSstVRetraceOn(WinApplication* app, x86::CPU& cpu)
{
    traceCall(0x50, {});
    NFS2_USE(app);
    NFS2_USE(cpu);
    return 0;
}

static void grSstIdle(WinApplication* app, x86::CPU& cpu)
{
    traceCall(0x54, {});
    NFS2_USE(app);
    NFS2_USE(cpu);
    //NFS2_ASSERT(false);
}

static void grTexCombineFunction(WinApplication* app, x86::CPU& cpu, GrChipID_t tmu, GrTextureCombineFnc_t fnc)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(tmu);
    NFS2_USE(fnc);
    NFS2_ASSERT(false);
}

static void grChromakeyValue(WinApplication* app, x86::CPU& cpu, GrColor_t  color)
{
    traceCall(0x34, {color});
    NFS2_USE(app);
    NFS2_USE(cpu);
    s_glideRenderer->setChromakeyValue(color);
}

static void grAlphaTestReferenceValue(WinApplication* app, x86::CPU& cpu, GrAlpha_t value)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    s_glideRenderer->setAlphaTestRef(value);
}

static void grTexDownloadTable(WinApplication* app, x86::CPU& cpu, GrChipID_t tmu, GrTexTable_t type, void *data)
{
    traceCall(0x3c, {tmu, type});
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(tmu);
    NFS2_USE(type);
    NFS2_USE(data);
    NFS2_ASSERT(false);
}

static void grRenderBuffer(WinApplication* app, x86::CPU& cpu, GrBuffer_t buffer)
{    
    traceCall(0x40, {buffer});
    NFS2_USE(app);
    NFS2_USE(cpu);
    /* The GL path renders into one FBO texture and ignores this entirely --
     * only the software/LFB addressing in Renderer uses the buffer index.  If
     * the game switches to an off-screen buffer to build the rear-view image,
     * that render lands in the visible frame instead.  Twice a frame in a
     * race, so traced only with the rest of the Glide layer. */
#ifdef NFS_TRACE_MSG
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[MIRROR] grRenderBuffer(%u)", (unsigned)buffer);
#endif
    s_glideRenderer->render(buffer);
}

static void grBufferClear(WinApplication* app, x86::CPU& cpu, GrColor_t color, GrAlpha_t alpha, x86::reg16 depth)
{
    traceCall(0x44, {color, alpha, depth});
    //NFS2_ASSERT(color == 0);
    NFS2_ASSERT(alpha == 0);
    NFS2_ASSERT(depth == 65535);
    app->unlockContext(cpu);
    s_glideRenderer->clear(color);
    app->lockContext(cpu);
}

static void grBufferSwap(WinApplication* app, x86::CPU& cpu, x86::reg32 swapInterval)
{
    traceCall(0x48, {swapInterval});
    NFS2_ASSERT(swapInterval == 1);
    app->unlockContext(cpu);
    s_glideRenderer->swap();
    app->lockContext(cpu);
    if (s_swapObserver)
        s_swapObserver(app);
}

static void grSstIsBusy(WinApplication* app, x86::CPU& cpu)
{
    traceCall(0x58, {});
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_ASSERT(false);
}

static void grClipWindow(WinApplication* app, x86::CPU& cpu,
                         x86::reg32 minX, x86::reg32 minY, x86::reg32 maxX, x86::reg32 maxY)
{
    traceCall(0x5c, {minX, minY, maxX, maxY});
    NFS2_USE(app);
    NFS2_USE(cpu);
    s_clipWindow[0] = minX;
    s_clipWindow[1] = minY;
    s_clipWindow[2] = maxX;
    s_clipWindow[3] = maxY;
    s_clipWindowSet = true;
    s_glideRenderer->setClipWindow(minX, minY, maxX, maxY);
}

static void grColorCombine(WinApplication* app, x86::CPU& cpu,
                           GrCombineFunction_t function, GrCombineFactor_t factor,
                           GrCombineLocal_t local, GrCombineOther_t other, BOOL invert)
{
    traceCall(0x60, {function, factor, local, other, invert});
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_ASSERT(function == GR_COMBINE_FUNCTION_LOCAL || function == GR_COMBINE_FUNCTION_BLEND_OTHER);
    NFS2_ASSERT(factor == GR_COMBINE_FACTOR_ZERO || factor == GR_COMBINE_FACTOR_LOCAL);
    NFS2_ASSERT(local == GR_COMBINE_LOCAL_ITERATED);
    NFS2_ASSERT(other == GR_COMBINE_OTHER_NONE || other == GR_COMBINE_OTHER_TEXTURE);
    NFS2_ASSERT(!invert);
    if (factor == GR_COMBINE_FACTOR_ZERO)
    {
        s_glideRenderer->setColorFactor(0.f);
    }
    else
    {
        s_glideRenderer->setColorFactor(1.f);
    }
    if (other == GR_COMBINE_OTHER_NONE)
    {
        s_glideRenderer->setTexture(0, 0);
    }
}

static void grAlphaCombine(WinApplication* app, x86::CPU& cpu,
                           GrCombineFunction_t function, GrCombineFactor_t factor,
                           GrCombineLocal_t local, GrCombineOther_t other, BOOL invert)
{
    traceCall(0x64, {function, factor, local, other, invert});
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_ASSERT(function == GR_COMBINE_FUNCTION_LOCAL || function == GR_COMBINE_FUNCTION_BLEND_OTHER);
    NFS2_ASSERT(factor == GR_COMBINE_FACTOR_ZERO || factor == GR_COMBINE_FACTOR_LOCAL);
    NFS2_ASSERT(local == GR_COMBINE_LOCAL_ITERATED);
    NFS2_ASSERT(other == GR_COMBINE_OTHER_NONE || other == GR_COMBINE_OTHER_TEXTURE);
    NFS2_ASSERT(!invert);
    if (factor == GR_COMBINE_FACTOR_ZERO)
    {
        s_glideRenderer->setAlphaFactor(0.f);
    }
    else
    {
        s_glideRenderer->setAlphaFactor(1.f);
    }
    // rely on Alpha combine to be done as Color combine
}

static void grAlphaBlendFunction(WinApplication* app, x86::CPU& cpu,
                                 GrAlphaBlendFnc_t rgb_sf, GrAlphaBlendFnc_t rgb_df,
                                 GrAlphaBlendFnc_t alpha_sf, GrAlphaBlendFnc_t alpha_df)
{
    traceCall(0x68, {rgb_sf, rgb_df, alpha_sf, alpha_df});
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_ASSERT(rgb_sf == GR_BLEND_SRC_ALPHA);
    NFS2_ASSERT(alpha_sf == GR_BLEND_ONE);
    NFS2_ASSERT(alpha_df == GR_BLEND_ZERO);
    switch(rgb_df)
    {
    case GR_BLEND_ONE_MINUS_SRC_ALPHA:
        s_glideRenderer->setAlphaBlendDst(AB_1mSrcAlpha);
        break;
    case GR_BLEND_ONE:
        s_glideRenderer->setAlphaBlendDst(AB_1);
        break;
    default:
        NFS2_ASSERT(false);
    }
}

static void grCullMode(WinApplication* app, x86::CPU& cpu, GrCullMode_t mode)
{
    traceCall(0x6c, {mode});
    NFS2_USE(app);
    NFS2_USE(cpu);
    /* Read once: this runs for every state change of a race, and a getenv
     * each time showed in the profile (2026-10-03). */
    static const int overrideMode = []() {
        const char* value = std::getenv("NFS_CULL");
        if (!value)
            return -1;
        char* end = nullptr;
        const unsigned long chosen = std::strtoul(value, &end, 0);
        return end != value && *end == '\0' && chosen <= 2 ? int(chosen) : -1;
    }();
    if (overrideMode >= 0)
        mode = x86::reg32(overrideMode);
    /* Glide's negative winding is clockwise in the game's screen-space
     * coordinates.  NFS_CULL can select 0/1/2 at runtime for device A/B tests
     * if a backend reports the opposite front-face convention. */
    s_glideRenderer->setCullMode(mode);
}

static void grTexFilterMode(WinApplication* app, x86::CPU& cpu, GrChipID_t tmu,
                            GrTextureFilterMode_t minfilter_mode, GrTextureFilterMode_t magfilter_mode)
{
    traceCall(0x70, {tmu, x86::reg32(minfilter_mode), x86::reg32(magfilter_mode)});
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(tmu);
    NFS2_USE(minfilter_mode);
    NFS2_USE(magfilter_mode);
    NFS2_ASSERT(minfilter_mode == GR_TEXTUREFILTER_BILINEAR);
    NFS2_ASSERT(magfilter_mode == GR_TEXTUREFILTER_BILINEAR);
}

static void grDitherMode(WinApplication* app, x86::CPU& cpu, GrDitherMode_t mode)
{
    traceCall(0x74, {mode});
    NFS2_USE(app);
    NFS2_USE(cpu);
    /* The RGB565 target quantises the shader result just like the Voodoo FBI;
     * ordered shader dithering restores the missing 2x2/4x4 colour spread. */
    s_glideRenderer->setDither(mode != 0);
}

static void grChromakeyMode(WinApplication* app, x86::CPU& cpu, GrChromakeyMode_t mode)
{
    traceCall(0x78, {mode});
    NFS2_USE(app);
    NFS2_USE(cpu);
    s_glideRenderer->setChromakeyMode(mode != 0);
}

static void grAlphaTestFunction(WinApplication* app, x86::CPU& cpu, GrCmpFnc_t function)
{
    traceCall(0x7c, {function});
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_ASSERT(function == GR_CMP_GREATER);
}

static void grDepthBufferMode(WinApplication* app, x86::CPU& cpu, GrDepthBufferMode_t mode)
{
    traceCall(0x80, {mode});
    NFS2_USE(app);
    NFS2_USE(cpu);
#ifdef NFS_TRACE_MSG
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[MIRROR] grDepthBufferMode(%u)", (unsigned)mode);
#endif
    if (mode == 0)
    {
        s_glideRenderer->setZTest(false);
    }
    else if (mode == 1)
    {
        s_glideRenderer->setZTest(true);
    }
    else
    {
        /* The game selects ZBUFFER.  WBUFFER and compare-to-bias need a
         * different depth interpolation path; retaining the ZBUFFER path is
         * safer than trapping on an unreachable hardware-only mode. */
        s_glideRenderer->setZTest(true);
    }
}

static void grDepthBufferFunction(WinApplication* app, x86::CPU& cpu, GrCmpFnc_t function)
{
    traceCall(0x84, {function});
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_ASSERT(function == GR_CMP_LEQUAL || function == GR_CMP_ALWAYS);
#ifdef NFS_TRACE_MSG
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "[MIRROR] grDepthBufferFunction(%u)", (unsigned)function);
#endif
    /* GR_CMP_ALWAYS is a comparison function, not grDepthBufferMode's
     * GR_DEPTHBUFFER_DISABLE: the depth test stays active (so a write still
     * happens whenever grDepthMask is on), it just always passes.  A common
     * technique -- this game's rear-view mirror included -- clips to a small
     * rectangle, draws a background/fill pass with ALWAYS + depth write on
     * to reset that rectangle's depth in place (no real grBufferClear
     * needed), then draws the real content with LEQUAL against that fresh
     * depth.  Collapsing ALWAYS into setZTest(false) used to disable the
     * depth test outright, which in GL also suppresses the write regardless
     * of the depth mask -- so that "reset" pass did nothing, and the
     * mirror's actual content was left depth-testing against whatever the
     * main scene had already drawn at those pixels. */
    s_glideRenderer->setZTest(true);
    s_glideRenderer->setDepthAlways(function == GR_CMP_ALWAYS);
}

static void grDepthMask(WinApplication* app, x86::CPU& cpu, x86::reg32 mask)
{
    traceCall(0x88, {mask});
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_ASSERT(mask == 0 || mask == 1);
    s_glideRenderer->setZWrite(mask == 1);
}

static void grFogColorValue(WinApplication* app, x86::CPU& cpu, GrColor_t fogcolor)
{
    traceCall(0x8c, {fogcolor});
    NFS2_USE(app);
    NFS2_USE(cpu);
    s_glideRenderer->setFogColor(fogcolor);
}

static void grFogMode(WinApplication* app, x86::CPU& cpu, GrFogMode_t mode)
{
    traceCall(0x90, {mode});
    NFS2_USE(app);
    NFS2_USE(cpu);
    s_glideRenderer->setFogMode(mode);
}

static void grFogTable(WinApplication* app, x86::CPU& cpu, const GrFog_t ft[GR_FOG_TABLE_SIZE])
{
    traceFogTable(ft);
    NFS2_USE(app);
    NFS2_USE(cpu);
    s_glideRenderer->setFogTable(ft);
}

static x86::reg32 grLfbLock(WinApplication* app, x86::CPU& cpu, GrLock_t type, GrBuffer_t buffer,
                            GrLfbWriteMode_t writeMode, GrOriginLocation_t origin, BOOL pixelPipeline, GrLfbInfo_t *info)
{
    traceCall(0x98, {type, buffer, writeMode, origin, pixelPipeline});
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(pixelPipeline);
    /* A read lock is refused (returns 0 below).  If the game asks for one it
     * is trying to read the rendered frame back -- the obvious way to build a
     * rear-view mirror image -- and a refusal makes it abandon that path
     * without ever reaching grLfbReadRegion, which is why the stub assert
     * there never fires. */
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                "[MIRROR] grLfbLock type=%u buffer=%u -> %s",
                (unsigned)type, (unsigned)buffer,
                (type == GR_LFB_WRITE_ONLY) ? "ptr" : "REFUSED");
    if (type == GR_LFB_WRITE_ONLY)
    {
        info->strideInBytes = s_renderer->getWidth() * 2;
        //info->size = s_renderer->getWidth() * s_renderer->getHeight() * 2;
        info->lfbPtr = Packed<void>(s_renderer->lock(buffer));
        info->origin = origin;
        info->writeMode = writeMode;
        return 1;
    }
    else
    {
        return 0;
    }
}

static x86::reg32 grLfbUnlock(WinApplication* app, x86::CPU& cpu, GrLock_t type, GrBuffer_t buffer)
{
    traceCall(0x9c, {type, buffer});
    NFS2_USE(app);
    NFS2_USE(cpu);
    if (type == GR_LFB_WRITE_ONLY)
    {
        s_renderer->unlock(buffer);
        return 1;
    }
    else
    {
        return 0;
    }
}

static x86::reg32 grLfbWriteRegion(WinApplication* app, x86::CPU& cpu,
                                   GrBuffer_t dst_buffer, x86::reg32 dst_x, x86::reg32 dst_y, GrLfbSrcFmt_t src_format,
                                   x86::reg32 src_width, x86::reg32 src_height, x86::reg32 pixelPipeline, x86::reg32 src_stride, void* src_data)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(dst_buffer);
    NFS2_USE(dst_x);
    NFS2_USE(dst_y);
    NFS2_USE(src_format);
    NFS2_USE(src_width);
    NFS2_USE(src_height);
    NFS2_USE(pixelPipeline);
    NFS2_USE(src_stride);
    NFS2_USE(src_data);
    NFS2_ASSERT(false);
    return 0;
}

static x86::reg32 grLfbReadRegion(WinApplication* app, x86::CPU& cpu,
                                  GrBuffer_t src_buffer, x86::reg32 src_x, x86::reg32 src_y,
                                  x86::reg32 src_width, x86::reg32 src_height, x86::reg32 dst_stride, void *dst_data)
{
    traceCall(0xa4, {src_buffer, src_x, src_y, src_width, src_height, dst_stride});
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(src_buffer);
    NFS2_USE(src_x);
    NFS2_USE(src_y);
    NFS2_USE(src_width);
    NFS2_USE(src_height);
    NFS2_USE(dst_stride);
    NFS2_USE(dst_data);
    NFS2_ASSERT(false);
    return 0;
}

static std::vector<GrVertex>* s_triangleTrace = nullptr;

void traceTriangles(std::vector<GrVertex>* trace)
{
    s_triangleTrace = trace;
}

bool tracingTriangles()
{
    return s_triangleTrace != nullptr;
}

void drawTriangle(const GrVertex* a, const GrVertex* b, const GrVertex* c)
{
    if (s_triangleTrace)
    {
        s_triangleTrace->push_back(*a);
        s_triangleTrace->push_back(*b);
        s_triangleTrace->push_back(*c);
    }
    if (!s_glideRenderer)
        return;
    GrVertex narrow[3];
    squeezed(a, b, c, narrow);
    GrVertex fitA, fitB, fitC;
    s_glideRenderer->drawTriangle(fitted(a, fitA), fitted(b, fitB), fitted(c, fitC));
}

static void grDrawTriangle(WinApplication* app, x86::CPU& cpu, const GrVertex* a, const GrVertex* b, const GrVertex* c)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    drawTriangle(a, b, c);
}

void drawLine(const GrVertex* a, const GrVertex* b)
{
    if (s_triangleTrace)
    {
        s_triangleTrace->push_back(*a);
        s_triangleTrace->push_back(*b);
        s_triangleTrace->push_back(*b);
    }
    if (!s_glideRenderer)
        return;
    GrVertex fitA, fitB;
    a = fitted(a, fitA);
    b = fitted(b, fitB);
    float orthX = a->y - b->y;
    float orthY = b->x - a->x;
    float len = ::sqrt(orthX*orthX + orthY * orthY);
    orthX /= len / 2;
    orthY /= len / 2;
    GrVertex a1 = *a;
    a1.a = 0;
    a1.x -= orthX;
    a1.y -= orthY;
    GrVertex a2 = *a;
    GrVertex a3 = *a;
    a3.a = 0;
    a3.x += orthX;
    a3.y += orthY;
    GrVertex b1 = *b;
    b1.a = 0;
    b1.x -= orthX;
    b1.y -= orthY;
    GrVertex b2 = *b;
    GrVertex b3 = *b;
    b3.a = 0;
    b3.x += orthX;
    b3.y += orthY;
    s_glideRenderer->drawTriangle(&a1, &b1, &a2);
    s_glideRenderer->drawTriangle(&b1, &b2, &a2);
    s_glideRenderer->drawTriangle(&a2, &b2, &a3);
    s_glideRenderer->drawTriangle(&b2, &b3, &a3);
}

static void grDrawLine(WinApplication* app, x86::CPU& cpu, const GrVertex *a, const GrVertex *b)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    drawLine(a, b);
}

static void grDrawPoint(WinApplication* app, x86::CPU& cpu, const GrVertex *a)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(a);
    NFS2_ASSERT(false);
}

static void grGammaCorrectionValue(WinApplication* app, x86::CPU& cpu, float correction)
{
    traceCall(0xb4, {floatBits(correction)});
    NFS2_USE(app);
    NFS2_USE(cpu);
    /* Apply Glide's display-space correction to 3D output.  The value is
     * batched with triangles, so a mid-frame change has ordered semantics. */
    s_glideRenderer->setGamma(correction);
}

static void grDepthBiasLevel(WinApplication* app, x86::CPU& cpu, x86::sreg16 bias)
{
    traceCall(0xb8, {x86::reg32(x86::sreg32(bias))});
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(bias);
    NFS2_ASSERT(bias == 0);
}

static void grTexMipMapMode(WinApplication* app, x86::CPU& cpu, GrChipID_t tmu, GrMipMapMode_t mode, BOOL lodBlend)
{
    traceCall(0xbc, {tmu, mode, lodBlend});
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(tmu);
    NFS2_USE(mode);
    NFS2_USE(lodBlend);
    NFS2_ASSERT(tmu == 0);
    //NFS2_ASSERT(false);
}

static void grTexCombine(WinApplication* app, x86::CPU& cpu, GrChipID_t tmu,
                         GrCombineFunction_t rgb_function, GrCombineFactor_t rgb_factor,
                         GrCombineFunction_t alpha_function, GrCombineFactor_t alpha_factor,
                         BOOL rgb_invert, BOOL alpha_invert)
{
    traceCall(0xc0, {tmu, rgb_function, rgb_factor, alpha_function, alpha_factor, rgb_invert, alpha_invert});
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_ASSERT(tmu == 0);
    NFS2_ASSERT(rgb_function == GR_COMBINE_FUNCTION_LOCAL);
    NFS2_ASSERT(rgb_factor == GR_COMBINE_FACTOR_ZERO);
    NFS2_ASSERT(alpha_function == GR_COMBINE_FUNCTION_LOCAL);
    NFS2_ASSERT(alpha_factor == GR_COMBINE_FACTOR_ZERO);
    NFS2_ASSERT(!rgb_invert);
    NFS2_ASSERT(!alpha_invert);
    //NFS2_ASSERT(false);
}

/* What a texture takes of the Voodoo's texture memory, whatever its size:
 * one slot of the renderer's table (GlideRenderer::getTextureMemSize, and
 * ThrashRenderer's the same), so each address is one texture's.  A constant
 * here, as THRASH_about asks before any window -- any renderer -- is open. */
static const x86::reg32 kTextureMemSize = sizeof(void*);

static x86::reg32 grTexCalcMemRequired(WinApplication* app, x86::CPU& cpu,
                                       GrLOD_t lodmin, GrLOD_t lodmax, GrAspectRatio_t aspect, GrTextureFormat_t fmt)
{
    traceCall(0xc4, {lodmin, lodmax, aspect, fmt});
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(fmt);
    NFS2_ASSERT(aspect == GR_ASPECT_1x1);
    return kTextureMemSize;
}

static void grTexDownloadMipMap(WinApplication* app, x86::CPU& cpu, GrChipID_t tmu,
                                x86::reg32 startAddress, x86::reg32 evenOdd, GrTexInfo* info)
{
    traceTexture(0xc8, tmu, startAddress, evenOdd, info);
    /* Rate limited: this is how an image would get from system memory into a
     * texture, i.e. the second half of any "read the frame back and show it in
     * the mirror" scheme. */
#ifdef NFS_TRACE_MSG
    {
        static unsigned hits = 0;
        if (++hits <= 8u || (hits % 256u) == 0u)
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                        "[MIRROR] grTexDownloadMipMap (call %u)", hits);
    }
#endif
    NFS2_USE(evenOdd);
    NFS2_ASSERT(tmu == 0);
    NFS2_ASSERT(info->aspectRatio == GR_ASPECT_1x1);
    app->unlockContext(cpu);
    if (info->format == GR_TEXFMT_RGB_565)
    {
        s_glideRenderer->setTextureData(tmu, startAddress, &app->getMemory<const void>(info ->data),
                                        info->largeLod, info->smallLod, TF_RGB_565);
    }
    else if (info->format == GR_TEXFMT_ARGB_1555)
    {
        s_glideRenderer->setTextureData(tmu, startAddress, &app->getMemory<const void>(info ->data),
                                        info->largeLod, info->smallLod, TF_ARGB_1555);
    }
    else if (info->format == GR_TEXFMT_ARGB_4444)
    {
        s_glideRenderer->setTextureData(tmu, startAddress, &app->getMemory<const void>(info ->data),
                                        info->largeLod, info->smallLod, TF_ARGB_4444);
    }
    else if (info->format == kTexFmtArgb8888)
    {
        s_glideRenderer->setTextureData(tmu, startAddress, &app->getMemory<const void>(info ->data),
                                        info->largeLod, info->smallLod, TF_ARGB_8888);
    }
    else
    {
        NFS2_ASSERT(false);
    }
    app->lockContext(cpu);
}

static x86::reg32 grTexMinAddress(WinApplication* app, x86::CPU& cpu, GrChipID_t tmu)
{
    traceCall(0xcc, {tmu});
    NFS2_USE(app);
    NFS2_USE(cpu);
    return 16*1024*1024 * tmu + kTextureMemSize;
}

static x86::reg32 grTexMaxAddress(WinApplication* app, x86::CPU& cpu, GrChipID_t tmu)
{
    traceCall(0xd0, {tmu});
    NFS2_USE(app);
    NFS2_USE(cpu);
    return 16*1024*1024 * (tmu+1);
}

static void grTexSource(WinApplication* app, x86::CPU& cpu, GrChipID_t tmu, uint32_t startAddress, uint32_t evenOdd, GrTexInfo *info)
{
    traceTexture(0xd4, tmu, startAddress, evenOdd, info);
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(tmu);
    NFS2_USE(startAddress);
    NFS2_USE(evenOdd);
    NFS2_USE(info);
    NFS2_ASSERT(tmu == 0);
    NFS2_ASSERT(info->aspectRatio == GR_ASPECT_1x1);
    s_glideRenderer->setTexture(tmu, startAddress);
}

static void grTexClampMode(WinApplication* app, x86::CPU& cpu, GrChipID_t tmu, GrTextureClampMode_t s_clampmode, GrTextureClampMode_t t_clampmode)
{
    traceCall(0xd8, {tmu, s_clampmode, t_clampmode});
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(tmu);
    /* GR_TEXTURECLAMP_WRAP is 0, GR_TEXTURECLAMP_CLAMP is 1.  This used
     * to ignore both arguments while the vertex attribute was hardcoded
     * to "wrap", so a texture the game asked to clamp had its opposite
     * edge folded back in along the border -- the commented-out asserts
     * below are the previous author noticing the game does ask for
     * clamping. */
    s_glideRenderer->setTexClampMode(s_clampmode == 0, t_clampmode == 0);
}

static void grGlideGetVersion(WinApplication* app, x86::CPU& cpu, char* version)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(version);
    NFS2_ASSERT(false);
}

static void grAADrawTriangle(WinApplication* app, x86::CPU& cpu, const GrVertex *a, const GrVertex *b, const GrVertex *c,
                             BOOL ab_antialias, BOOL bc_antialias, BOOL ca_antialias)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(a);
    NFS2_USE(b);
    NFS2_USE(c);
    NFS2_USE(ab_antialias);
    NFS2_USE(bc_antialias);
    NFS2_USE(ca_antialias);
    NFS2_ASSERT(false);
}

static void grAADrawLine(WinApplication* app, x86::CPU& cpu, const GrVertex *a, const GrVertex *b)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(a);
    NFS2_USE(b);
    NFS2_ASSERT(false);
}

static void grHints(WinApplication* app, x86::CPU& cpu, GrHints_t type, x86::reg32 hintMask)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(type);
    NFS2_USE(hintMask);
    /* Glide hints only tune hardware scheduling/LOD.  This renderer has no
     * equivalent hint state, and the game does not rely on them for pixels. */
}

static void grTexLodBiasValue(WinApplication* app, x86::CPU& cpu, GrChipID_t tmu, float bias)
{
    traceCall(0xec, {tmu, floatBits(bias)});
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(tmu);
    NFS2_USE(bias);
    /* Mip levels are explicitly selected by grTexSource in this port, so a
     * hardware LOD bias cannot change sampling and is safe to ignore. */
}

static x86::reg32 grSstScreenWidth(WinApplication* app, x86::CPU& cpu)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    return s_renderer->getWidth();
}

static x86::reg32 grSstScreenHeight(WinApplication* app, x86::CPU& cpu)
{
    NFS2_USE(app);
    NFS2_USE(cpu);
    return s_renderer->getHeight();
}

static x86::sreg32 grBufferNumPending(WinApplication* app, x86::CPU& cpu)
{
    traceCall(0xf8, {});
    NFS2_USE(app);
    NFS2_USE(cpu);
    return 0;
}

static BOOL grSstControl(WinApplication* app, x86::CPU& cpu, x86::reg32 code)
{
    traceCall(0xfc, {code});
    NFS2_USE(app);
    NFS2_USE(cpu);
    NFS2_USE(code);
    NFS2_ASSERT(false);
    return 0;
}

void traceCalls(std::vector<x86::reg32>* trace)
{
    s_callTrace = trace;
}

namespace direct
{

void chromakeyValue(WinApplication* app, x86::CPU& cpu, x86::reg32 color)
{
    grChromakeyValue(app, cpu, color);
}

void colorCombine(WinApplication* app, x86::CPU& cpu, x86::reg32 function, x86::reg32 factor, x86::reg32 local,
                  x86::reg32 other, x86::reg32 invert)
{
    grColorCombine(app, cpu, function, factor, local, other, invert);
}

void alphaCombine(WinApplication* app, x86::CPU& cpu, x86::reg32 function, x86::reg32 factor, x86::reg32 local,
                  x86::reg32 other, x86::reg32 invert)
{
    grAlphaCombine(app, cpu, function, factor, local, other, invert);
}

void alphaBlendFunction(WinApplication* app, x86::CPU& cpu, x86::reg32 rgbSource, x86::reg32 rgbDestination,
                        x86::reg32 alphaSource, x86::reg32 alphaDestination)
{
    grAlphaBlendFunction(app, cpu, rgbSource, rgbDestination, alphaSource, alphaDestination);
}

void cullMode(WinApplication* app, x86::CPU& cpu, x86::reg32 mode)
{
    grCullMode(app, cpu, mode);
}

void texFilterMode(WinApplication* app, x86::CPU& cpu, x86::reg32 tmu, x86::reg32 minification,
                   x86::reg32 magnification)
{
    grTexFilterMode(app, cpu, tmu, GrTextureFilterMode_t(minification), GrTextureFilterMode_t(magnification));
}

void ditherMode(WinApplication* app, x86::CPU& cpu, x86::reg32 mode)
{
    grDitherMode(app, cpu, mode);
}

void chromakeyMode(WinApplication* app, x86::CPU& cpu, x86::reg32 mode)
{
    grChromakeyMode(app, cpu, mode);
}

void alphaTestFunction(WinApplication* app, x86::CPU& cpu, x86::reg32 function)
{
    grAlphaTestFunction(app, cpu, function);
}

void depthBufferMode(WinApplication* app, x86::CPU& cpu, x86::reg32 mode)
{
    grDepthBufferMode(app, cpu, mode);
}

void depthBufferFunction(WinApplication* app, x86::CPU& cpu, x86::reg32 function)
{
    grDepthBufferFunction(app, cpu, function);
}

void depthMask(WinApplication* app, x86::CPU& cpu, x86::reg32 mask)
{
    grDepthMask(app, cpu, mask);
}

void fogColorValue(WinApplication* app, x86::CPU& cpu, x86::reg32 colour)
{
    grFogColorValue(app, cpu, colour);
}

void fogMode(WinApplication* app, x86::CPU& cpu, x86::reg32 mode)
{
    grFogMode(app, cpu, mode);
}

void fogTable(WinApplication* app, x86::CPU& cpu, const x86::reg8* table)
{
    grFogTable(app, cpu, table);
}

void gammaCorrectionValue(WinApplication* app, x86::CPU& cpu, float gamma)
{
    grGammaCorrectionValue(app, cpu, gamma);
}

void depthBiasLevel(WinApplication* app, x86::CPU& cpu, x86::sreg16 bias)
{
    grDepthBiasLevel(app, cpu, bias);
}

void texMipMapMode(WinApplication* app, x86::CPU& cpu, x86::reg32 tmu, x86::reg32 mode, x86::reg32 lodBlend)
{
    grTexMipMapMode(app, cpu, tmu, mode, lodBlend);
}

void texCombine(WinApplication* app, x86::CPU& cpu, x86::reg32 tmu, x86::reg32 rgbFunction, x86::reg32 rgbFactor,
                x86::reg32 alphaFunction, x86::reg32 alphaFactor, x86::reg32 rgbInvert, x86::reg32 alphaInvert)
{
    grTexCombine(app, cpu, tmu, rgbFunction, rgbFactor, alphaFunction, alphaFactor, rgbInvert, alphaInvert);
}

void texClampMode(WinApplication* app, x86::CPU& cpu, x86::reg32 tmu, x86::reg32 s, x86::reg32 t)
{
    grTexClampMode(app, cpu, tmu, s, t);
}

void texLodBiasValue(WinApplication* app, x86::CPU& cpu, x86::reg32 tmu, float bias)
{
    grTexLodBiasValue(app, cpu, tmu, bias);
}

x86::reg32 texCalcMemRequired(WinApplication* app, x86::CPU& cpu, x86::reg32 smallLod, x86::reg32 largeLod,
                              x86::reg32 aspect, x86::reg32 format)
{
    return grTexCalcMemRequired(app, cpu, smallLod, largeLod, aspect, format);
}

x86::reg32 texMinAddress(WinApplication* app, x86::CPU& cpu, x86::reg32 tmu)
{
    return grTexMinAddress(app, cpu, tmu);
}

x86::reg32 texMaxAddress(WinApplication* app, x86::CPU& cpu, x86::reg32 tmu)
{
    return grTexMaxAddress(app, cpu, tmu);
}

void texSource(WinApplication* app, x86::CPU& cpu, x86::reg32 tmu, x86::reg32 start, x86::reg32 evenOdd, x86::reg32 info)
{
    grTexSource(app, cpu, tmu, start, evenOdd, &app->getMemory<GrTexInfo>(info));
}

void texDownloadMipMap(WinApplication* app, x86::CPU& cpu, x86::reg32 tmu, x86::reg32 start, x86::reg32 evenOdd,
                       x86::reg32 info)
{
    grTexDownloadMipMap(app, cpu, tmu, start, evenOdd, &app->getMemory<GrTexInfo>(info));
}

void texDownloadTable(WinApplication* app, x86::CPU& cpu, x86::reg32 tmu, x86::reg32 type, x86::reg32 data)
{
    grTexDownloadTable(app, cpu, tmu, type, &app->getMemory<void>(data));
}

void renderBuffer(WinApplication* app, x86::CPU& cpu, x86::reg32 buffer)
{
    grRenderBuffer(app, cpu, buffer);
}

void bufferClear(WinApplication* app, x86::CPU& cpu, x86::reg32 colour, x86::reg8 alpha, x86::reg16 depth)
{
    grBufferClear(app, cpu, colour, alpha, depth);
}

void bufferSwap(WinApplication* app, x86::CPU& cpu, x86::reg32 interval)
{
    grBufferSwap(app, cpu, interval);
}

x86::sreg32 bufferNumPending(WinApplication* app, x86::CPU& cpu)
{
    return grBufferNumPending(app, cpu);
}

x86::reg32 sstStatus(WinApplication* app, x86::CPU& cpu)
{
    return grSstStatus(app, cpu);
}

x86::reg32 sstVRetraceOn(WinApplication* app, x86::CPU& cpu)
{
    return grSstVRetraceOn(app, cpu);
}

void sstIdle(WinApplication* app, x86::CPU& cpu)
{
    grSstIdle(app, cpu);
}

void sstIsBusy(WinApplication* app, x86::CPU& cpu)
{
    grSstIsBusy(app, cpu);
}

void clipWindow(WinApplication* app, x86::CPU& cpu, x86::reg32 minX, x86::reg32 minY, x86::reg32 maxX, x86::reg32 maxY)
{
    grClipWindow(app, cpu, minX, minY, maxX, maxY);
}

x86::reg32 lfbLock(WinApplication* app, x86::CPU& cpu, x86::reg32 type, x86::reg32 buffer, x86::reg32 writeMode,
                   x86::reg32 origin, x86::reg32 pixelPipeline, x86::reg32 info)
{
    return grLfbLock(app, cpu, type, buffer, writeMode, origin, pixelPipeline, &app->getMemory<GrLfbInfo_t>(info));
}

x86::reg32 lfbUnlock(WinApplication* app, x86::CPU& cpu, x86::reg32 type, x86::reg32 buffer)
{
    return grLfbUnlock(app, cpu, type, buffer);
}

x86::reg32 lfbReadRegion(WinApplication* app, x86::CPU& cpu, x86::reg32 buffer, x86::reg32 x, x86::reg32 y,
                         x86::reg32 width, x86::reg32 height, x86::reg32 stride, x86::reg32 data)
{
    return grLfbReadRegion(app, cpu, buffer, x, y, width, height, stride, &app->getMemory<void>(data));
}

void resetTextures()
{
    if (s_glideRenderer)
        s_glideRenderer->resetTextures();
}

x86::reg32 sstControl(WinApplication* app, x86::CPU& cpu, x86::reg32 code)
{
    return grSstControl(app, cpu, code);
}

ThrashRenderer* thrashRenderer()
{
    if (!s_directRenderer || s_triangleTrace || s_squeezeScale != 1.0f || s_fitScale != 1.0f || s_fitOffset != 0.0f)
        return nullptr;
    return s_directRenderer;
}

}


Library s_glide2xLibrary = win32::Library("glide2x.dll");
Library* s_glide2xRegistry = &(s_glide2xLibrary
    .registerSymbol("_grGlideInit@0", &Wrapper<decltype(&grGlideInit), &grGlideInit>::stdcall)
    .registerSymbol("_grGlideShutdown@0", &Wrapper<decltype(&grGlideShutdown), &grGlideShutdown>::stdcall)
    .registerSymbol("_grSstQueryHardware@4", &Wrapper<decltype(&grSstQueryHardware), &grSstQueryHardware>::stdcall)
    .registerSymbol("_grSstQueryBoards@4", &Wrapper<decltype(&grSstQueryBoards), &grSstQueryBoards>::stdcall)
    .registerSymbol("_grSstSelect@4", &Wrapper<decltype(&grSstSelect), &grSstSelect>::stdcall)
    .registerSymbol("_grSstWinOpen@28", &Wrapper<decltype(&grSstWinOpen), &grSstWinOpen>::stdcall)
    .registerSymbol("_grSstWinClose@0", &Wrapper<decltype(&grSstWinClose), &grSstWinClose>::stdcall)
    .registerSymbol("_grSstStatus@0", &Wrapper<decltype(&grSstStatus), &grSstStatus>::stdcall)
    .registerSymbol("_grSstVRetraceOn@0", &Wrapper<decltype(&grSstVRetraceOn), &grSstVRetraceOn>::stdcall)
    .registerSymbol("_grSstIdle@0", &Wrapper<decltype(&grSstIdle), &grSstIdle>::stdcall)
    .registerSymbol("_grTexCombineFunction@8", &Wrapper<decltype(&grTexCombineFunction), &grTexCombineFunction>::stdcall)
    .registerSymbol("_grChromakeyValue@4", &Wrapper<decltype(&grChromakeyValue), &grChromakeyValue>::stdcall)
    .registerSymbol("_grAlphaTestReferenceValue@4", &Wrapper<decltype(&grAlphaTestReferenceValue), &grAlphaTestReferenceValue>::stdcall)
    .registerSymbol("_grTexDownloadTable@12", &Wrapper<decltype(&grTexDownloadTable), &grTexDownloadTable>::stdcall)
    .registerSymbol("_grRenderBuffer@4", &Wrapper<decltype(&grRenderBuffer), &grRenderBuffer>::stdcall)
    .registerSymbol("_grBufferClear@12", &Wrapper<decltype(&grBufferClear), &grBufferClear>::stdcall)
    .registerSymbol("_grBufferSwap@4", &Wrapper<decltype(&grBufferSwap), &grBufferSwap>::stdcall)
    .registerSymbol("_grSstIsBusy@0", &Wrapper<decltype(&grSstIsBusy), &grSstIsBusy>::stdcall)
    .registerSymbol("_grClipWindow@16", &Wrapper<decltype(&grClipWindow), &grClipWindow>::stdcall)
    .registerSymbol("_grColorCombine@20", &Wrapper<decltype(&grColorCombine), &grColorCombine>::stdcall)
    .registerSymbol("_grAlphaCombine@20", &Wrapper<decltype(&grAlphaCombine), &grAlphaCombine>::stdcall)
    .registerSymbol("_grAlphaBlendFunction@16", &Wrapper<decltype(&grAlphaBlendFunction), &grAlphaBlendFunction>::stdcall)
    .registerSymbol("_grCullMode@4", &Wrapper<decltype(&grCullMode), &grCullMode>::stdcall)
    .registerSymbol("_grTexFilterMode@12", &Wrapper<decltype(&grTexFilterMode), &grTexFilterMode>::stdcall)
    .registerSymbol("_grDitherMode@4", &Wrapper<decltype(&grDitherMode), &grDitherMode>::stdcall)
    .registerSymbol("_grChromakeyMode@4", &Wrapper<decltype(&grChromakeyMode), &grChromakeyMode>::stdcall)
    .registerSymbol("_grAlphaTestFunction@4", &Wrapper<decltype(&grAlphaTestFunction), &grAlphaTestFunction>::stdcall)
    .registerSymbol("_grDepthBufferMode@4", &Wrapper<decltype(&grDepthBufferMode), &grDepthBufferMode>::stdcall)
    .registerSymbol("_grDepthBufferFunction@4", &Wrapper<decltype(&grDepthBufferFunction), &grDepthBufferFunction>::stdcall)
    .registerSymbol("_grDepthMask@4", &Wrapper<decltype(&grDepthMask), &grDepthMask>::stdcall)
    .registerSymbol("_grFogColorValue@4", &Wrapper<decltype(&grFogColorValue), &grFogColorValue>::stdcall)
    .registerSymbol("_grFogMode@4", &Wrapper<decltype(&grFogMode), &grFogMode>::stdcall)
    .registerSymbol("_grFogTable@4", &Wrapper<decltype(&grFogTable), &grFogTable>::stdcall)
    .registerSymbol("_grLfbLock@24", &Wrapper<decltype(&grLfbLock), &grLfbLock>::stdcall)
    .registerSymbol("_grLfbUnlock@8", &Wrapper<decltype(&grLfbUnlock), &grLfbUnlock>::stdcall)
    .registerSymbol("_grLfbWriteRegion@32", &Wrapper<decltype(&grLfbWriteRegion), &grLfbWriteRegion>::stdcall)
    .registerSymbol("_grLfbReadRegion@28", &Wrapper<decltype(&grLfbReadRegion), &grLfbReadRegion>::stdcall)
    .registerSymbol("_grDrawTriangle@12", &Wrapper<decltype(&grDrawTriangle), &grDrawTriangle>::stdcall)
    .registerSymbol("_grDrawLine@8", &Wrapper<decltype(&grDrawLine), &grDrawLine>::stdcall)
    .registerSymbol("_grDrawPoint@4", &Wrapper<decltype(&grDrawPoint), &grDrawPoint>::stdcall)
    .registerSymbol("_grGammaCorrectionValue@4", &Wrapper<decltype(&grGammaCorrectionValue), &grGammaCorrectionValue>::stdcall)
    .registerSymbol("_grDepthBiasLevel@4", &Wrapper<decltype(&grDepthBiasLevel), &grDepthBiasLevel>::stdcall)
    .registerSymbol("_grTexMipMapMode@12", &Wrapper<decltype(&grTexMipMapMode), &grTexMipMapMode>::stdcall)
    .registerSymbol("_grTexCombine@28", &Wrapper<decltype(&grTexCombine), &grTexCombine>::stdcall)
    .registerSymbol("_grTexCalcMemRequired@16", &Wrapper<decltype(&grTexCalcMemRequired), &grTexCalcMemRequired>::stdcall)
    .registerSymbol("_grTexDownloadMipMap@16", &Wrapper<decltype(&grTexDownloadMipMap), &grTexDownloadMipMap>::stdcall)
    .registerSymbol("_grTexMinAddress@4", &Wrapper<decltype(&grTexMinAddress), &grTexMinAddress>::stdcall)
    .registerSymbol("_grTexMaxAddress@4", &Wrapper<decltype(&grTexMaxAddress), &grTexMaxAddress>::stdcall)
    .registerSymbol("_grTexSource@16", &Wrapper<decltype(&grTexSource), &grTexSource>::stdcall)
    .registerSymbol("_grTexClampMode@12", &Wrapper<decltype(&grTexClampMode), &grTexClampMode>::stdcall)
    .registerSymbol("_grGlideGetVersion@4", &Wrapper<decltype(&grGlideGetVersion), &grGlideGetVersion>::stdcall)
    .registerSymbol("_grAADrawTriangle@24", &Wrapper<decltype(&grAADrawTriangle), &grAADrawTriangle>::stdcall)
    .registerSymbol("_grAADrawLine@8", &Wrapper<decltype(&grAADrawLine), &grAADrawLine>::stdcall)
    .registerSymbol("_grHints@8", &Wrapper<decltype(&grHints), &grHints>::stdcall)
    .registerSymbol("_grTexLodBiasValue@8", &Wrapper<decltype(&grTexLodBiasValue), &grTexLodBiasValue>::stdcall)
    .registerSymbol("_grSstScreenWidth@0", &Wrapper<decltype(&grSstScreenWidth), &grSstScreenWidth>::stdcall)
    .registerSymbol("_grSstScreenHeight@0", &Wrapper<decltype(&grSstScreenHeight), &grSstScreenHeight>::stdcall)
    .registerSymbol("_grBufferNumPending@0", &Wrapper<decltype(&grBufferNumPending), &grBufferNumPending>::stdcall)
    .registerSymbol("_grSstControl@4", &Wrapper<decltype(&grSstControl), &grSstControl>::stdcall)
);

}}
