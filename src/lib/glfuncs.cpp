#include <lib/glfuncs.h>
#include <x86.h>
#include <SDL3/SDL.h>

namespace win32
{

#if NFS_GL_NEEDS_LOADER

PFNGLBLENDFUNCSEPARATEPROC       glBlendFuncSeparate;
PFNGLATTACHSHADERPROC            glAttachShader;
PFNGLCOMPILESHADERPROC           glCompileShader;
PFNGLCREATEPROGRAMPROC           glCreateProgram;
PFNGLCREATESHADERPROC            glCreateShader;
PFNGLGETSHADERIVPROC             glGetShaderiv;
PFNGLGETPROGRAMIVPROC            glGetProgramiv;
PFNGLGETPROGRAMINFOLOGPROC       glGetProgramInfoLog;
PFNGLGETSHADERINFOLOGPROC        glGetShaderInfoLog;
PFNGLLINKPROGRAMPROC             glLinkProgram;
PFNGLSHADERSOURCEPROC            glShaderSource;
PFNGLUSEPROGRAMPROC              glUseProgram;
PFNGLGETATTRIBLOCATIONPROC       glGetAttribLocation;
PFNGLVERTEXATTRIBPOINTERPROC     glVertexAttribPointer;
PFNGLGENVERTEXARRAYSPROC         glGenVertexArrays;
PFNGLBINDVERTEXARRAYPROC         glBindVertexArray;
PFNGLBUFFERDATAPROC              glBufferData;
PFNGLGENBUFFERSPROC              glGenBuffers;
PFNGLBINDBUFFERPROC              glBindBuffer;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLGETUNIFORMLOCATIONPROC      glGetUniformLocation;
PFNGLUNIFORMMATRIX4FVPROC        glUniformMatrix4fv;
PFNGLUNIFORM1IPROC               glUniform1i;
PFNGLUNIFORM3FPROC               glUniform3f;
PFNGLUNIFORM1FPROC               glUniform1f;
NfsCullFaceProc                  glCullFace;
NfsFrontFaceProc                 glFrontFace;
NfsScissorProc                   glScissor;
PFNGLACTIVETEXTUREPROC           glActiveTexture;
PFNGLTEXSTORAGE2DPROC            glTexStorage2D;
PFNGLGENFRAMEBUFFERSPROC         glGenFramebuffers;
PFNGLDELETEFRAMEBUFFERSPROC      glDeleteFramebuffers;
PFNGLBINDFRAMEBUFFERPROC         glBindFramebuffer;
PFNGLFRAMEBUFFERTEXTURE2DPROC    glFramebufferTexture2D;
PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC glGetFramebufferAttachmentParameteriv;
PFNGLGENRENDERBUFFERSPROC        glGenRenderbuffers;
PFNGLDELETERENDERBUFFERSPROC     glDeleteRenderbuffers;
PFNGLBINDRENDERBUFFERPROC        glBindRenderbuffer;
PFNGLRENDERBUFFERSTORAGEPROC     glRenderbufferStorage;
PFNGLDRAWBUFFERSPROC             glDrawBuffers;

namespace
{

template< typename T >
void load(T& fn, const char* name)
{
    fn = reinterpret_cast<T>(SDL_GL_GetProcAddress(name));
    if (!fn)
    {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "GL entry point not available: %s", name);
    }
    NFS2_ASSERT(fn);
}

}

void loadGlFunctions()
{
    static bool s_loaded = false;
    if (s_loaded)
        return;

    load(glBlendFuncSeparate,       "glBlendFuncSeparate");
    load(glAttachShader,            "glAttachShader");
    load(glCompileShader,           "glCompileShader");
    load(glCreateProgram,           "glCreateProgram");
    load(glCreateShader,            "glCreateShader");
    load(glGetShaderiv,             "glGetShaderiv");
    load(glGetProgramiv,            "glGetProgramiv");
    load(glGetProgramInfoLog,       "glGetProgramInfoLog");
    load(glGetShaderInfoLog,        "glGetShaderInfoLog");
    load(glLinkProgram,             "glLinkProgram");
    load(glShaderSource,            "glShaderSource");
    load(glUseProgram,              "glUseProgram");
    load(glGetAttribLocation,       "glGetAttribLocation");
    load(glVertexAttribPointer,     "glVertexAttribPointer");
    load(glGenVertexArrays,         "glGenVertexArrays");
    load(glBindVertexArray,         "glBindVertexArray");
    load(glBufferData,              "glBufferData");
    load(glGenBuffers,              "glGenBuffers");
    load(glBindBuffer,              "glBindBuffer");
    load(glEnableVertexAttribArray, "glEnableVertexAttribArray");
    load(glGetUniformLocation,      "glGetUniformLocation");
    load(glUniformMatrix4fv,        "glUniformMatrix4fv");
    load(glUniform1i,               "glUniform1i");
    load(glUniform3f,               "glUniform3f");
    load(glUniform1f,               "glUniform1f");
    load(glCullFace,                "glCullFace");
    load(glFrontFace,               "glFrontFace");
    load(glScissor,                 "glScissor");
    load(glActiveTexture,           "glActiveTexture");
    load(glTexStorage2D,            "glTexStorage2D");
    load(glGenFramebuffers,         "glGenFramebuffers");
    load(glDeleteFramebuffers,      "glDeleteFramebuffers");
    load(glBindFramebuffer,         "glBindFramebuffer");
    load(glFramebufferTexture2D,    "glFramebufferTexture2D");
    load(glFramebufferRenderbuffer, "glFramebufferRenderbuffer");
    load(glGetFramebufferAttachmentParameteriv, "glGetFramebufferAttachmentParameteriv");
    load(glGenRenderbuffers,        "glGenRenderbuffers");
    load(glDeleteRenderbuffers,     "glDeleteRenderbuffers");
    load(glBindRenderbuffer,        "glBindRenderbuffer");
    load(glRenderbufferStorage,     "glRenderbufferStorage");
    load(glDrawBuffers,             "glDrawBuffers");

    s_loaded = true;
}

#else

void loadGlFunctions()
{
    /* GLES 3.0 links these directly. */
}

#endif

}
