/* What the port's headers and native_loading.cpp take from SDL, for
 * loading_checks alone: the checks build without SDL. */
#ifndef LOADING_CHECKS_SDL_STUB_H
#define LOADING_CHECKS_SDL_STUB_H
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

typedef uint64_t Uint64;
typedef uint32_t Uint32;
typedef uint16_t Uint16;
typedef uint8_t Uint8;
typedef int32_t Sint32;

inline const char* SDL_getenv(const char* name) { return std::getenv(name); }
inline int SDL_strcmp(const char* a, const char* b) { return std::strcmp(a, b); }
inline size_t SDL_strlen(const char* s) { return std::strlen(s); }
inline unsigned long SDL_strtoul(const char* s, char** end, int base) { return std::strtoul(s, end, base); }
inline int SDL_snprintf(char* text, size_t size, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    const int length = std::vsnprintf(text, size, format, args);
    va_end(args);
    return length;
}
inline void SDL_Log(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    std::vfprintf(stdout, format, args);
    va_end(args);
    std::fputc('\n', stdout);
}
#endif
