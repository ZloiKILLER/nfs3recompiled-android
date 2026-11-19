#include "native_frontend.h"
#include <nfs3hp.h>

#include "os_net.h"
#include <lib/app.h>
#include <SDL3/SDL.h>

#include <cstdio>
#include <cstring>

#ifdef __ANDROID__
#include <jni.h>
#endif

/* The connection screen's own additions: the TCP/IP port, switched by its button
 * between the Modern Patch's and the original's, and the address the others find
 * this phone at.  The items come from menu_data.cpp, without them nothing here
 * does anything.
 *
 * The game's handler of the screen stays as it is, held against its code by the
 * checks; this runs after it, put in its place as the application is built
 * (port_display.cpp does the same).  A text item draws the string its text field
 * points at, so the two values are written over the placeholders the menu file
 * gave them, which is the room they have. */
namespace nfs3hp
{

// nfs3hp_main.cpp.
nfs::u32 networkPort();
void setNetworkPort(nfs::u32 port);

namespace
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using nfs::u32;

constexpr u32 kPortModern = 9803, kPortOriginal = 1030;

// Addresses of the port's own, for the game to call: past anything it has.
constexpr u32 kPortClicked = 0xbff000, kScreenFrame = 0xbff010;

constexpr std::size_t kPortRoom = 20, kAddressRoom = 15;
constexpr Uint64 kAddressEveryMs = 2000;

u32 s_portText, s_addressText;  // where the two values are written
u32 s_gameFrame;                // the screen's own handler for each pass, if any
Uint64 s_nextAddress;

void writeText(App* app, u32 to, const char* text, std::size_t room)
{
    if (to == 0)
        return;
    char* place = &app->getMemory<char>(to);
    std::strncpy(place, text, room);
    place[room] = 0;
}

void showPort(App* app)
{
    char text[kPortRoom + 1];
    std::snprintf(text, sizeof text, "%u", unsigned(networkPort()));
    writeText(app, s_portText, text, kPortRoom);
}

void showAddress(App* app)
{
    const std::uint32_t address = os::net::localAddress();
    char text[kAddressRoom + 1];
    if (address == 0)
        std::snprintf(text, sizeof text, "No network");
    else
        std::snprintf(text, sizeof text, "%u.%u.%u.%u", address & 0xff, (address >> 8) & 0xff, (address >> 16) & 0xff,
                      address >> 24);
    writeText(app, s_addressText, text, kAddressRoom);
    s_nextAddress = SDL_GetTicks() + kAddressEveryMs;
}

/* Kept in the launcher's preferences, as the game starts from them. */
void rememberPort(u32 port)
{
#ifdef __ANDROID__
    JNIEnv* env = static_cast<JNIEnv*>(SDL_GetAndroidJNIEnv());
    jobject activity = static_cast<jobject>(SDL_GetAndroidActivity());
    if (!env || !activity)
        return;
    jclass cls = env->GetObjectClass(activity);
    jmethodID method = env->GetMethodID(cls, "onNetworkPortChosen", "(I)V");
    if (method)
        env->CallVoidMethod(activity, method, jint(port));
    if (env->ExceptionCheck())
        env->ExceptionClear();
    env->DeleteLocalRef(cls);
    env->DeleteLocalRef(activity);
#else
    (void)port;
#endif
}

u32 findItem(App* app, u32 menu, const char* codelink)
{
    const std::int32_t first = app->getMemory<std::int16_t>(menu + fe::menu::kFirstItem);
    for (std::int32_t i = 0;; ++i)
    {
        const u32 item = fe::item::kTable + fe::item::kSize * u32(first + i);
        if (app->getMemory<u32>(item + fe::item::kType) == 0)
            return 0;
        const u32 name = app->getMemory<u32>(item + fe::item::kCodelink);
        if (name != 0 && std::strcmp(&app->getMemory<char>(name), codelink) == 0)
            return item;
    }
}

/* A button's handler, called as the game's are: it takes its return address's
 * slot off the stack.  2: the key is spent. */
nfs::u32 portClicked(App* app, ThreadState& ts, nfs::Args& args)
{
    const u32 port = networkPort() == kPortModern ? kPortOriginal : kPortModern;
    setNetworkPort(port);
    rememberPort(port);
    showPort(app);
    args.eax = fe::kDialogHandled;
    ts.sp += 4;
    return args.eax;
}

/* Each pass of the menu loop: the address looked at again now and then, the
 * phone may join a network while the screen is up.  0 stays on the screen. */
nfs::u32 screenFrame(App* app, ThreadState& ts, nfs::Args& args)
{
    if (SDL_GetTicks() >= s_nextAddress)
        showAddress(app);
    if (s_gameFrame != 0)
    {
        // The game's own handler in its place: called with what this one was handed, its return slot
        // this one's (it takes it off, as this one would).
        const u32 esp = ts.sp;
        ts.sp = esp + 4;
        const std::optional<u32> left = guest::pointerCall(app, ts, s_gameFrame, args);
        ts.sp = esp + 4;
        args.eax = left.value_or(0);
        return args.eax;
    }
    args.eax = 0;
    ts.sp += 4;
    return args.eax;
}

nfs::u32 screenEnter(App* app, ThreadState& ts, nfs::Args& args)
{
    const u32 menu = args.eax;
    frontend::connectOnEnter(app, ts, args);
    const u32 button = findItem(app, menu, "NET_PORT");
    const u32 port = findItem(app, menu, "NET_PORT_VALUE");
    const u32 address = findItem(app, menu, "NET_ADDRESS_VALUE");
    if (button == 0 || port == 0 || address == 0)
        return args.eax;
    app->getMemory<u32>(button + fe::item::kOnClick) = kPortClicked;
    s_portText = app->getMemory<u32>(port + fe::item::kText);
    s_addressText = app->getMemory<u32>(address + fe::item::kText);
    showPort(app);
    showAddress(app);
    const u32 frame = app->getMemory<u32>(menu + fe::menu::kOnFrame);
    if (frame != kScreenFrame)
    {
        s_gameFrame = frame;
        app->getMemory<u32>(menu + fe::menu::kOnFrame) = kScreenFrame;
    }
    return args.eax;
}

}

void installConnectScreen(App* app)
{
    app->replaceMethod(0x4405c0, { "<nfs3hp>port::connect::screenEnter", &screenEnter });
    app->registerMethod(kPortClicked, { "<nfs3hp>port::connect::portClicked", &portClicked });
    app->registerMethod(kScreenFrame, { "<nfs3hp>port::connect::screenFrame", &screenFrame });
}

}
