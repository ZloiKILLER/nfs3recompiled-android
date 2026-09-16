#include <lib/glidetmu.h>
#include <SDL3/SDL.h>
#include <cstdio>
#include <stdexcept>
#include <vector>
using win32::GlideTMU;
using win32::GlTextureSlot;
static void check(bool ok, const char* what) { if (!ok) throw std::runtime_error(what); }
static x86::reg32 wholeTiles(const GlideTMU& tmu) {
    x86::reg32 texels = 0, tiles = 0;
    tmu.freeSpace(texels, tiles);
    return tiles;
}
int main() {
    try {
        {
            GlideTMU tmu(512);
            check(wholeTiles(tmu) == 4, "a fresh 512 atlas is four whole tiles");
            std::vector<GlTextureSlot*> small;
            for (int i = 0; i < 16; ++i) small.push_back(tmu.reserveTextureSlot(2));
            for (auto* slot : small) check(slot != nullptr, "sixteen 64x64 textures fit");
            check(wholeTiles(tmu) == 3, "and take exactly one tile");
            for (auto* slot : small) tmu.returnTextureSlot(slot);
            check(wholeTiles(tmu) == 4, "freed quadrants merge back into a whole tile");
            std::vector<GlTextureSlot*> full;
            for (int i = 0; i < 4; ++i) full.push_back(tmu.reserveTextureSlot(0));
            for (auto* slot : full) check(slot != nullptr, "four 256x256 textures fit");
            check(tmu.reserveTextureSlot(0) == nullptr, "a full atlas says so instead of crashing");
            check(tmu.reserveTextureSlot(3) == nullptr, "with nothing left to split either");
            tmu.returnTextureSlot(full[0]);
            check(tmu.reserveTextureSlot(1) != nullptr, "a returned tile can be split again");
        }
        {
            // Splitting every tile into 16x16 needs more slot records than one chunk holds.
            GlideTMU tmu(1024);
            std::vector<GlTextureSlot*> tiny;
            for (int i = 0; i < 16 * 256; ++i) {
                GlTextureSlot* slot = tmu.reserveTextureSlot(4);
                check(slot != nullptr, "slot records are added on demand");
                tiny.push_back(slot);
            }
            check(tmu.reserveTextureSlot(4) == nullptr, "and the atlas still ends");
            for (auto* slot : tiny) tmu.returnTextureSlot(slot);
            check(wholeTiles(tmu) == 16, "a fully fragmented atlas comes back whole");
        }
        std::puts("PASS: glide texture atlas -- exhaustion without a crash, quadrant merge, slot records on demand");
        return 0;
    } catch (const std::exception& e) {
        std::fprintf(stderr, "FAIL: %s\n", e.what());
        return 1;
    }
}
