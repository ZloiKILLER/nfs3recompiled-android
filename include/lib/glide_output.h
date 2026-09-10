#ifndef LIB_GLIDE_OUTPUT_H
#define LIB_GLIDE_OUTPUT_H
#include <cstdint>
namespace win32 {
// THRASH mode records: width, height, output depth, ...; 10 DWORDs.
// Keep mode IDs and availability unchanged. CPU LFB format is negotiated separately.
inline void setGlideOutputDepth(uint32_t* modes, uint32_t depth) {
    for (unsigned i = 0; i < 17; ++i) {
        auto* mode = modes + i * 10;
        if (mode[0] && mode[1] && (mode[2] == 16 || mode[2] == 32)) mode[2] = depth;
    }
}
}
#endif
