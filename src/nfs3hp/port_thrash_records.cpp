#include "port_thrash.h"

#include <cstring>

/* The THRASH driver's texture records as the game's memory has them (see
 * port_thrash.h): apart from port_thrash.cpp, which draws through the
 * renderer. */
namespace nfs3hp
{
namespace port
{
namespace driver
{

using guest::at;
using guest::put;
using guest::byte;

int32_t exponentOf(reg32 value)
{
    const float converted = float(double(int32_t(value)));
    reg32 bits;
    std::memcpy(&bits, &converted, sizeof bits);
    return int32_t(bits - 0x3f800000) >> 23;
}

reg32 lodOf(App* app, int32_t log)
{
    return log > 8 ? reg32(8 - log) : reg32(byte(app, kLods + reg32(log)));
}

void resizeTexture(App* app, reg32 record, reg32 side)
{
    if (record == 0 || at(app, record + kRecordLargeLod) != at(app, record + kRecordSmallLod))
        return;
    const reg32 lod = lodOf(app, exponentOf(side));
    put(app, record + kRecordLargeLod, lod);
    put(app, record + kRecordSmallLod, lod);
    put(app, record + kRecordAspect, byte(app, kAspects));
}

}  // namespace driver
}  // namespace port
}  // namespace nfs3hp
