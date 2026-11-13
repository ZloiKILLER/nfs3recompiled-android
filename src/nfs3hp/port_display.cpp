#include "native_frontend.h"
#include <nfs3hp.h>

#include <lib/renderer.h>
#include <lib/winapp.h>
#include <SDL3/SDL.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <iterator>

#ifdef __ANDROID__
#include <jni.h>
#endif

/* Gamma, brightness and contrast as sliders of the game's own Advanced Graphics
 * screen, advgrph; the sliders come from menu_data.cpp.  The pause menu's
 * graphics, psgraph, had them too until the user took them off (2026-11-13).
 *
 * The game's handlers of that screen are left as they are, held against
 * its code by the checks.  What these sliders need runs after each of them, put
 * in their place in the table of methods as the application is built: as the
 * screen opens the sliders are found and set, on each pass of the menu loop what
 * they show goes to the final blit, so the picture follows the finger, and as it
 * closes Cancel puts back what was there and anything else keeps it.
 *
 * The launcher's preferences stay where these values live: it hands them to the
 * game as it starts (NFS3Activity) and carries them in its settings file, and
 * the game gives them back when they change. */
namespace nfs3hp
{

namespace
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using x86::reg32;

/* Percentages, as in the launcher's preferences; in step with ScreenAdjustment.java.
 * Each slider's step in menu_data.cpp lands on 100. */
struct Range
{
    const char* codelink;
    int least, most;
};
constexpr int kAdjustments = 3;
constexpr Range kRanges[kAdjustments] = {
    { "DISPLAY_GAMMA", 100, 250 },
    { "DISPLAY_BRIGHTNESS", 50, 150 },
    { "DISPLAY_CONTRAST", 50, 200 },
};

constexpr reg32 kSliderValue = 0x54;  // float, 0 to 1
constexpr reg32 kButton = 5;

reg32 s_sliders[kAdjustments];   // of the screen open, 0 where there is none
int s_percent[kAdjustments];     // as the sliders show them
int s_kept[kAdjustments];        // as the screen found them

void currentPercents(int (&percent)[kAdjustments])
{
    const win32::DisplayAdjustment now = win32::displayAdjustment();
    percent[0] = int(std::lround(now.gamma * 100.f));
    percent[1] = int(std::lround(now.brightness * 200.f)) + 100;
    percent[2] = int(std::lround(now.contrast * 100.f));
}

/* What the launcher's ScreenAdjustment hands the shader for these percentages. */
void applyPercents(const int (&percent)[kAdjustments])
{
    win32::setDisplayAdjustment({ percent[0] / 100.f, (percent[1] - 100) / 200.f, percent[2] / 100.f });
}

bool holds(App* app, reg32 string, const char* text)
{
    return string != 0 && std::strcmp(&app->getMemory<char>(string), text) == 0;
}

reg32 itemAt(App* app, reg32 menu, std::int32_t index)
{
    const std::int32_t first = app->getMemory<std::int16_t>(menu + fe::menu::kFirstItem);
    return fe::item::kTable + fe::item::kSize * reg32(first + index);
}

/* The screen's item with this codelink, 0 without one.  Read directly and not
 * through the game's lookup, so nothing here touches the guest's registers. */
reg32 findSlider(App* app, reg32 menu, const char* codelink)
{
    for (std::int32_t i = 0;; ++i)
    {
        const reg32 item = itemAt(app, menu, i);
        if (app->getMemory<reg32>(item + fe::item::kType) == 0)
            return 0;
        if (holds(app, app->getMemory<reg32>(item + fe::item::kCodelink), codelink))
            return item;
    }
}

void opened(App* app, reg32 menu)
{
    currentPercents(s_kept);
    for (int i = 0; i < kAdjustments; ++i)
    {
        const Range& range = kRanges[i];
        s_percent[i] = s_kept[i];
        s_sliders[i] = findSlider(app, menu, range.codelink);
        if (s_sliders[i] != 0)
            app->getMemory<float>(s_sliders[i] + kSliderValue) =
                float(std::clamp(s_kept[i], range.least, range.most) - range.least) / float(range.most - range.least);
    }
}

void followSliders(App* app)
{
    bool changed = false;
    for (int i = 0; i < kAdjustments; ++i)
    {
        if (s_sliders[i] == 0)
            continue;
        const Range& range = kRanges[i];
        const float value = app->getMemory<float>(s_sliders[i] + kSliderValue);
        const int percent = range.least + int(std::lround(value * float(range.most - range.least)));
        changed = changed || percent != s_percent[i];
        s_percent[i] = percent;
    }
    if (changed)
        applyPercents(s_percent);
}

/* Handed to the launcher's preferences, on the thread it is called from (SDL
 * gives each its JNIEnv); NFS3Activity puts them on the disk by itself. */
void remember(const int (&percent)[kAdjustments])
{
#ifdef __ANDROID__
    JNIEnv* env = static_cast<JNIEnv*>(SDL_GetAndroidJNIEnv());
    jobject activity = static_cast<jobject>(SDL_GetAndroidActivity());
    if (!env || !activity)
        return;
    jclass cls = env->GetObjectClass(activity);
    jmethodID method = env->GetMethodID(cls, "onDisplayAdjusted", "(III)V");
    if (method)
        env->CallVoidMethod(activity, method, jint(percent[0]), jint(percent[1]), jint(percent[2]));
    if (env->ExceptionCheck())
        env->ExceptionClear();
    env->DeleteLocalRef(cls);
    env->DeleteLocalRef(activity);
#else
    (void)percent;
#endif
}

/* The button the screen is left by is the selected item, as for the game's own
 * handlers of Cancel. */
bool leftByCancel(App* app, reg32 menu, const char* cancel)
{
    const reg32 item = itemAt(app, menu, app->getMemory<std::int16_t>(menu + fe::menu::kSelected));
    return app->getMemory<reg32>(item + fe::item::kType) == kButton
        && holds(app, app->getMemory<reg32>(item + fe::item::kCodelink), cancel);
}

void closed(App* app, reg32 menu, const char* cancel)
{
    if (s_sliders[0] == 0 && s_sliders[1] == 0 && s_sliders[2] == 0)
        return;
    if (leftByCancel(app, menu, cancel))
        applyPercents(s_kept);
    else if (!std::equal(std::begin(s_percent), std::end(s_percent), std::begin(s_kept)))
        remember(s_percent);
    std::fill(std::begin(s_sliders), std::end(s_sliders), 0);
}

void advancedEnter(App* app, CPU& cpu)
{
    const reg32 menu = cpu.eax;
    frontend::advancedOnEnter(app, cpu);
    opened(app, menu);
}

void advancedFrame(App* app, CPU& cpu)
{
    frontend::advancedOnFrame(app, cpu);
    followSliders(app);
}

void advancedExit(App* app, CPU& cpu)
{
    const reg32 menu = cpu.eax;
    frontend::advancedOnExit(app, cpu);
    closed(app, menu, "CANCEL_ADVGRAPHICS_OPTIONS");
}

}

void installDisplaySliders(App* app)
{
    app->replaceMethod(0x448610, { "<nfs3hp>port::display::advancedEnter", &advancedEnter });
    app->replaceMethod(0x4485a0, { "<nfs3hp>port::display::advancedFrame", &advancedFrame });
    app->replaceMethod(0x448770, { "<nfs3hp>port::display::advancedExit", &advancedExit });
}

}
