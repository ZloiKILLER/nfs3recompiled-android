/* Included first into every SDL source of a release (android/app/jni/
 * CMakeLists.txt, NFS_RELEASE): SDL's Android code writes to the system log
 * directly, around SDL's own log priorities, and a release of the NFS III port
 * writes nothing to the log.  The header is read here, so its declarations
 * stand, and the calls after it come to nothing. */
#ifndef NFS3HP_SDL_QUIET_H
#define NFS3HP_SDL_QUIET_H

#include <android/log.h>

#define __android_log_print(...) (0)
#define __android_log_write(...) (0)

#endif
