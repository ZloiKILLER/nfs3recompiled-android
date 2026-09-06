#ifndef LIB_GLPLATFORM_H_
#define LIB_GLPLATFORM_H_

/* Which GL flavour this build targets.  Deliberately free of #includes: files
 * that only need to branch on the flavour (window.cpp, for one) must not be
 * forced to pull in the GL headers, which on Windows drag in windows.h and its
 * macros -- WM_USER among them, which collides with a constant in window.cpp. */

#if defined(__ANDROID__) || defined(NFS_FORCE_GLES)
#   define NFS_GLES 1
#   define NFS_GL_NEEDS_LOADER 0
    /* Only the preamble differs between the two GLSL dialects; every shader
     * body in the project compiles under both once this is prepended.  It is
     * passed to glShaderSource as a separate string. */
#   define NFS_GLSL_PREAMBLE                \
        "#version 300 es\n"                 \
        "precision highp float;\n"          \
        "precision highp int;\n"            \
        "precision highp sampler2D;\n"
#else
#   define NFS_GLES 0
#   define NFS_GL_NEEDS_LOADER 1
#   define NFS_GLSL_PREAMBLE "#version 400\n"
#endif

#endif /* !LIB_GLPLATFORM_H_ */
