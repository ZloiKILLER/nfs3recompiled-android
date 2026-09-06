#ifndef LIB_GLCOMPAT_H_
#define LIB_GLCOMPAT_H_

/* Single place where the GL headers are chosen.
 *
 * Desktop keeps exactly what the project used before: SDL's own GL 1.1 header,
 * which also pulls in SDL_opengl_glext.h and with it every PFNGL...PROC typedef.
 * Everything past GL 1.1 is resolved at runtime through SDL_GL_GetProcAddress
 * (see lib/glfuncs.h).
 *
 * On GLES the indirection is unnecessary: GLES 3.0 declares the whole API
 * directly and libGLESv3 exports it, so the function-pointer declarations and
 * their loader are compiled out.
 *
 * Include lib/glplatform.h instead if all you need is the NFS_GLES flavour flag
 * without dragging in the GL (and, on Windows, windows.h) headers.
 */

#include <lib/glplatform.h>
#include <SDL3/SDL.h>

#if NFS_GLES
#   include <GLES3/gl3.h>
#   include <GLES2/gl2ext.h>
    /* GLES has no double-precision depth clear. */
#   define glClearDepth(d) glClearDepthf(GLfloat(d))
#else
#   include <SDL3/SDL_opengl.h>
#endif

#endif /* !LIB_GLCOMPAT_H_ */
