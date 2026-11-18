#include "native_frontend.h"
#include <nfs3hp.h>

#include <lib/socket.h>
#include <lib/winapp.h>
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
x86::reg32 networkPort();
void setNetworkPort(x86::reg32 port);

namespace
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using x86::reg32;

constexpr reg32 kPortModern = 9803, kPortOriginal = 1030;

// Addresses of the port's own, for the game to call: past anything it has.
constexpr reg32 kPortClicked = 0xbff000, kScreenFrame = 0xbff010;

constexpr std::size_t kPortRoom = 20, kAddressRoom = 15;
constexpr Uint64 kAddressEveryMs = 2000;

reg32 s_portText, s_addressText;  // where the two values are written
reg32 s_gameFrame;                // the screen's own handler for each pass, if any
Uint64 s_nextAddress;

void writeText(App* app, reg32 to, const char* text, std::size_t room)
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
    const std::uint32_t address = win32::Socket::localAddress();
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
void rememberPort(reg32 port)
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

reg32 findItem(App* app, reg32 menu, const char* codelink)
{
    const std::int32_t first = app->getMemory<std::int16_t>(menu + fe::menu::kFirstItem);
    for (std::int32_t i = 0;; ++i)
    {
        const reg32 item = fe::item::kTable + fe::item::kSize * reg32(first + i);
        if (app->getMemory<reg32>(item + fe::item::kType) == 0)
            return 0;
        const reg32 name = app->getMemory<reg32>(item + fe::item::kCodelink);
        if (name != 0 && std::strcmp(&app->getMemory<char>(name), codelink) == 0)
            return item;
    }
}

/* A button's handler, called as the game's are: it takes its return address's
 * slot off the stack.  2: the key is spent. */
x86::reg32 portClicked(App* app, CPU& cpu, x86::Args& args)
{
    const reg32 port = networkPort() == kPortModern ? kPortOriginal : kPortModern;
    setNetworkPort(port);
    rememberPort(port);
    showPort(app);
    args.eax = fe::kDialogHandled;
    cpu.esp += 4;
    return args.eax;
}

/* Each pass of the menu loop: the address looked at again now and then, the
 * phone may join a network while the screen is up.  0 stays on the screen. */
x86::reg32 screenFrame(App* app, CPU& cpu, x86::Args& args)
{
    if (SDL_GetTicks() >= s_nextAddress)
        showAddress(app);
    if (s_gameFrame != 0)
    {
        // The game's own handler in its place: called with what this one was handed, its return slot
        // this one's (it takes it off, as this one would).
        const reg32 esp = cpu.esp;
        cpu.esp = esp + 4;
        const std::optional<reg32> left = guest::pointerCall(app, cpu, s_gameFrame, args);
        cpu.esp = esp + 4;
        args.eax = left.value_or(0);
        return args.eax;
    }
    args.eax = 0;
    cpu.esp += 4;
    return args.eax;
}

x86::reg32 screenEnter(App* app, CPU& cpu, x86::Args& args)
{
    const reg32 menu = args.eax;
    frontend::connectOnEnter(app, cpu, args);
    const reg32 button = findItem(app, menu, "NET_PORT");
    const reg32 port = findItem(app, menu, "NET_PORT_VALUE");
    const reg32 address = findItem(app, menu, "NET_ADDRESS_VALUE");
    if (button == 0 || port == 0 || address == 0)
        return args.eax;
    app->getMemory<reg32>(button + fe::item::kOnClick) = kPortClicked;
    s_portText = app->getMemory<reg32>(port + fe::item::kText);
    s_addressText = app->getMemory<reg32>(address + fe::item::kText);
    showPort(app);
    showAddress(app);
    const reg32 frame = app->getMemory<reg32>(menu + fe::menu::kOnFrame);
    if (frame != kScreenFrame)
    {
        s_gameFrame = frame;
        app->getMemory<reg32>(menu + fe::menu::kOnFrame) = kScreenFrame;
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
