#include <x86.h>
#include <SDL3/SDL.h>
#include <map>
#include <mutex>
#include <string>
#include <utility>

namespace x86
{

/* See the NFS2_ASSERT comment in x86.h: this is the release-safe path (no
 * NFS2_ASSERT_TRAP), reached only when a check fails, so it does not need to
 * be cheap -- just visible. */
void assertLog(const char* file, int line, const char* expr)
{
    /* Deduplicated per call site.  These fire on paths the game takes as a
     * matter of course (a mouse GetDeviceData in the wrong format used to log
     * thousands of times a second), and a log that drowns in one repeated site
     * hides the single-shot ones -- which are exactly the interesting ones when
     * the question is "which unsupported paths does this screen touch".  First
     * hit verbatim, then at every power of ten with the running count, so both
     * existence and magnitude survive at a bounded cost. */
    static std::mutex mutex;
    static std::map<std::pair<std::string, int>, unsigned long> hits;

    unsigned long count;
    {
        std::lock_guard<std::mutex> lock(mutex);
        count = ++hits[std::make_pair(std::string(file), line)];
    }

    unsigned long decade = 1;
    while (decade < count)
        decade *= 10;
    if (decade != count)
        return;

    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[ASSERT] %s:%d: %s (hit %lu)",
                 file, line, expr, count);
}

}
