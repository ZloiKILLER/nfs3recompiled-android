#ifndef LIB_GLFUNCS_H_
#define LIB_GLFUNCS_H_

/* Everything past GL 1.1 has to be fetched through SDL_GL_GetProcAddress on the
 * desktop.  Both renderer.cpp and gliderenderer.cpp need the same entry points,
 * so they live here instead of being duplicated as file statics.
 *
 * On GLES the whole indirection disappears: GLES 3.0 declares and exports these
 * directly, so the declarations are compiled out and calls bind to the real
 * functions from libGLESv3. */

#include <lib/glcompat.h>

namespace win32
{

#if NFS_GL_NEEDS_LOADER

extern PFNGLBLENDFUNCSEPARATEPROC       glBlendFuncSeparate;
extern PFNGLATTACHSHADERPROC            glAttachShader;
extern PFNGLCOMPILESHADERPROC           glCompileShader;
extern PFNGLCREATEPROGRAMPROC           glCreateProgram;
extern PFNGLCREATESHADERPROC            glCreateShader;
extern PFNGLGETSHADERIVPROC             glGetShaderiv;
extern PFNGLGETPROGRAMIVPROC            glGetProgramiv;
extern PFNGLGETPROGRAMINFOLOGPROC       glGetProgramInfoLog;
extern PFNGLGETSHADERINFOLOGPROC        glGetShaderInfoLog;
extern PFNGLLINKPROGRAMPROC             glLinkProgram;
extern PFNGLSHADERSOURCEPROC            glShaderSource;
extern PFNGLUSEPROGRAMPROC              glUseProgram;
extern PFNGLGETATTRIBLOCATIONPROC       glGetAttribLocation;
extern PFNGLVERTEXATTRIBPOINTERPROC     glVertexAttribPointer;
extern PFNGLGENVERTEXARRAYSPROC         glGenVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC         glBindVertexArray;
extern PFNGLBUFFERDATAPROC              glBufferData;
extern PFNGLGENBUFFERSPROC              glGenBuffers;
extern PFNGLBINDBUFFERPROC              glBindBuffer;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
extern PFNGLGETUNIFORMLOCATIONPROC      glGetUniformLocation;
extern PFNGLUNIFORMMATRIX4FVPROC        glUniformMatrix4fv;
extern PFNGLUNIFORM1IPROC               glUniform1i;
extern PFNGLUNIFORM3FPROC               glUniform3f;
extern PFNGLUNIFORM1FPROC               glUniform1f;
typedef void (APIENTRYP NfsCullFaceProc)(GLenum mode);
typedef void (APIENTRYP NfsFrontFaceProc)(GLenum mode);
typedef void (APIENTRYP NfsScissorProc)(GLint x, GLint y, GLsizei width, GLsizei height);
extern NfsCullFaceProc                  glCullFace;
extern NfsFrontFaceProc                 glFrontFace;
extern NfsScissorProc                   glScissor;
extern PFNGLACTIVETEXTUREPROC           glActiveTexture;
extern PFNGLTEXSTORAGE2DPROC            glTexStorage2D;
extern PFNGLGENFRAMEBUFFERSPROC         glGenFramebuffers;
extern PFNGLDELETEFRAMEBUFFERSPROC      glDeleteFramebuffers;
extern PFNGLBINDFRAMEBUFFERPROC         glBindFramebuffer;
extern PFNGLFRAMEBUFFERTEXTURE2DPROC    glFramebufferTexture2D;
extern PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;
extern PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC glGetFramebufferAttachmentParameteriv;
extern PFNGLGENRENDERBUFFERSPROC        glGenRenderbuffers;
extern PFNGLDELETERENDERBUFFERSPROC     glDeleteRenderbuffers;
extern PFNGLBINDRENDERBUFFERPROC        glBindRenderbuffer;
extern PFNGLRENDERBUFFERSTORAGEPROC     glRenderbufferStorage;
extern PFNGLDRAWBUFFERSPROC             glDrawBuffers;

#endif /* NFS_GL_NEEDS_LOADER */

/* Idempotent; must be called with a current GL context.  No-op on GLES. */
void loadGlFunctions();

}

#endif /* !LIB_GLFUNCS_H_ */
